/* SPDX-License-Identifier: MIT */
/* ivs_pack: the build-tree image tool of the project's hand-assembled listings (D-25, D-26, D-30,
 * plan 04-14). It is the only decoder of a listing into image bytes. It is a development tool:
 * never installed, never shipped, never part of the core. It writes only the files named on its
 * command line (CMake points them into the build tree), and its output holds no time, path or
 * host-dependent byte.
 *
 * Listing grammar (D-26; docs/diagnostics.md): '#' lines are comments, '# window: AAAA-BBBB' is a
 * required header, blank lines are ignored, and every other line is a word line
 *   ADDR HEX4 BITS... ROW ASM...
 * ADDR and HEX4 are 4 hex digits, HEX4 is at most 03FF, the BITS groups are binary digits, 10 in
 * total, whose value equals HEX4, ROW is a row id or '-', and ASM is at least one more token.
 * Word lines are contiguous from the first one and stay inside the window. Row and operand
 * semantics are checked by the gate step `listing` (plan 04-15), not here.
 *
 * Modes:
 *   ivs_pack --exec <listing> <out> [--patch ADDR=HEX]...
 *       the 8192-byte raw EXEC image: the word at address 1000 + i at offset 2i, high byte first,
 *       bits 15..10 zero, zero tail (the listing must lie in 1000-1FFF)
 *   ivs_pack --ivsc <listing> <out> [--patch ADDR=HEX]...
 *       one-segment IVSC v1 container (docs/formats/ivsc-v1.md), trailer SHA-256 of all bytes
 *       before it
 *   ivs_pack --dump-words <listing> [--patch ADDR=HEX]...
 *       one 'ADDR WORD' line per word (4 upper-case hex digits each) on stdout
 *   ivs_pack --c-include <out.inc> <name>=<file>...
 *       a C include with one 'static const uint8_t ivs_img_<name>[]' array and one
 *       IVS_IMG_<NAME>_BYTES define per file (the includer includes <stdint.h> first)
 * A patch replaces the word at an address the listing defines; it is applied before any output.
 * Exit status: 0 on success, 1 on any refusal (one 'ivs_pack: ...' line on stderr, line-numbered
 * for a listing error). */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/ivs_sha256.h"

#define PK_MAX_WORDS 4096u
#define PK_MAX_LINE 1024u
#define PK_EXEC_BASE 0x1000u
#define PK_EXEC_BYTES 8192u
#define PK_WORD_MAX 0x03FFu
#define PK_MAX_INCLUDE_BYTES 262144u

typedef struct pk_listing {
  uint32_t first;  /* address of the first word */
  uint32_t count;  /* words */
  uint32_t win_lo; /* window from '# window:' */
  uint32_t win_hi;
  uint16_t word[PK_MAX_WORDS];
} pk_listing;

static pk_listing pk_list;

static int pk_hexval(int c)
{
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }
  return -1;
}

/* Exactly four hex digits; 1 on success. */
static int pk_hex4(const char *s, uint32_t *out)
{
  uint32_t v = 0u;
  int i;
  for (i = 0; i < 4; i++) {
    int d = pk_hexval((unsigned char)s[i]);
    if (d < 0) {
      return 0;
    }
    v = (v << 4) | (uint32_t)d;
  }
  if (s[4] != '\0') {
    return 0;
  }
  *out = v;
  return 1;
}

static int pk_is_binary(const char *s)
{
  if (*s == '\0') {
    return 0;
  }
  for (; *s != '\0'; s++) {
    if (*s != '0' && *s != '1') {
      return 0;
    }
  }
  return 1;
}

/* Splits line in place at spaces and tabs; returns the token count (at most max). */
static size_t pk_tokens(char *line, char **tok, size_t max)
{
  size_t n = 0u;
  char *p = line;
  while (*p != '\0') {
    while (*p == ' ' || *p == '\t') {
      *p++ = '\0';
    }
    if (*p == '\0') {
      break;
    }
    if (n == max) {
      return max + 1u;
    }
    tok[n++] = p;
    while (*p != '\0' && *p != ' ' && *p != '\t') {
      p++;
    }
  }
  return n;
}

static int pk_lerr(const char *path, unsigned long line, const char *what)
{
  fprintf(stderr, "ivs_pack: %s:%lu: %s\n", path, line, what);
  return 0;
}

/* '# window: AAAA-BBBB' (exact form). Returns 1 when line is that header (then *ok says whether it
 * parsed), else 0. */
static int pk_window(const char *line, int *ok)
{
  static const char key[] = "# window: ";
  char lo[5];
  char hi[5];
  if (strncmp(line, key, sizeof key - 1u) != 0) {
    return 0;
  }
  line += sizeof key - 1u;
  *ok = 0;
  if (strlen(line) != 9u || line[4] != '-') {
    return 1;
  }
  memcpy(lo, line, 4u);
  lo[4] = '\0';
  memcpy(hi, line + 5, 4u);
  hi[4] = '\0';
  if (pk_hex4(lo, &pk_list.win_lo) && pk_hex4(hi, &pk_list.win_hi) && pk_list.win_lo <= pk_list.win_hi) {
    *ok = 1;
  }
  return 1;
}

/* Reads and checks the listing into pk_list; 1 on success. */
static int pk_read(const char *path)
{
  char line[PK_MAX_LINE + 2u];
  char *tok[64];
  unsigned long lineno = 0ul;
  int have_window = 0;
  FILE *f = fopen(path, "rb");
  memset(&pk_list, 0, sizeof pk_list);
  if (f == NULL) {
    fprintf(stderr, "ivs_pack: cannot open %s\n", path);
    return 0;
  }
  while (fgets(line, (int)sizeof line, f) != NULL) {
    size_t len = strlen(line);
    size_t n;
    size_t i;
    size_t bits = 0u;
    uint32_t addr;
    uint32_t word;
    uint32_t value = 0u;
    int ok = 0;
    lineno++;
    if (len == 0u || line[len - 1u] != '\n') {
      fclose(f);
      return pk_lerr(path, lineno, "line too long or no final newline");
    }
    line[--len] = '\0';
    for (i = 0u; i < len; i++) {
      unsigned char c = (unsigned char)line[i];
      if (c != '\t' && (c < 0x20u || c > 0x7Eu)) {
        fclose(f);
        return pk_lerr(path, lineno, "byte outside printable ASCII");
      }
    }
    if (line[0] == '#') {
      if (pk_window(line, &ok)) {
        if (!ok || have_window || pk_list.count != 0u) {
          fclose(f);
          return pk_lerr(path, lineno, "bad, repeated or late '# window: AAAA-BBBB' header");
        }
        have_window = 1;
      }
      continue;
    }
    n = pk_tokens(line, tok, sizeof tok / sizeof tok[0]);
    if (n == 0u) {
      continue;
    }
    if (!have_window) {
      fclose(f);
      return pk_lerr(path, lineno, "word line before the '# window:' header");
    }
    if (n > sizeof tok / sizeof tok[0] || n < 5u) {
      fclose(f);
      return pk_lerr(path, lineno, "a word line is ADDR HEX4 BITS ROW ASM");
    }
    if (!pk_hex4(tok[0], &addr) || !pk_hex4(tok[1], &word)) {
      fclose(f);
      return pk_lerr(path, lineno, "ADDR and HEX4 must be 4 hex digits");
    }
    if (word > PK_WORD_MAX) {
      fclose(f);
      return pk_lerr(path, lineno, "word above 03FF");
    }
    for (i = 2u; i < n && pk_is_binary(tok[i]); i++) {
      const char *b;
      for (b = tok[i]; *b != '\0'; b++) {
        value = (value << 1) | (uint32_t)(*b - '0');
        bits++;
        if (bits > 10u) {
          break;
        }
      }
    }
    if (i == 2u || bits != 10u) {
      fclose(f);
      return pk_lerr(path, lineno, "bit groups must hold exactly 10 binary digits");
    }
    if (value != word) {
      fclose(f);
      return pk_lerr(path, lineno, "bit groups do not equal HEX4");
    }
    if (n < i + 2u) {
      fclose(f);
      return pk_lerr(path, lineno, "missing ROW or ASM after the bit groups");
    }
    if (pk_list.count == 0u) {
      pk_list.first = addr;
    } else if (addr != pk_list.first + pk_list.count) {
      fclose(f);
      return pk_lerr(path, lineno, "address not contiguous");
    }
    if (addr < pk_list.win_lo || addr > pk_list.win_hi) {
      fclose(f);
      return pk_lerr(path, lineno, "address outside the window");
    }
    if (pk_list.count == PK_MAX_WORDS) {
      fclose(f);
      return pk_lerr(path, lineno, "more than 4096 words");
    }
    pk_list.word[pk_list.count++] = (uint16_t)word;
  }
  if (ferror(f)) {
    fclose(f);
    fprintf(stderr, "ivs_pack: read error on %s\n", path);
    return 0;
  }
  fclose(f);
  if (pk_list.count == 0u) {
    fprintf(stderr, "ivs_pack: %s: no word line\n", path);
    return 0;
  }
  return 1;
}

/* Applies one --patch ADDR=HEX to pk_list; 1 on success. */
static int pk_patch(const char *arg)
{
  char a[5];
  char w[5];
  uint32_t addr;
  uint32_t word;
  if (strlen(arg) != 9u || arg[4] != '=') {
    fprintf(stderr, "ivs_pack: --patch wants ADDR=HEX (4 hex digits each): %s\n", arg);
    return 0;
  }
  memcpy(a, arg, 4u);
  a[4] = '\0';
  memcpy(w, arg + 5, 4u);
  w[4] = '\0';
  if (!pk_hex4(a, &addr) || !pk_hex4(w, &word) || word > PK_WORD_MAX) {
    fprintf(stderr, "ivs_pack: --patch wants ADDR=HEX with HEX at most 03FF: %s\n", arg);
    return 0;
  }
  if (addr < pk_list.first || addr - pk_list.first >= pk_list.count) {
    fprintf(stderr, "ivs_pack: --patch address %s is not a word of the listing\n", a);
    return 0;
  }
  pk_list.word[addr - pk_list.first] = (uint16_t)word;
  return 1;
}

static void pk_put16(uint8_t *p, uint32_t v)
{
  p[0] = (uint8_t)((v >> 8) & 0xFFu);
  p[1] = (uint8_t)(v & 0xFFu);
}

static int pk_write(const char *path, const uint8_t *b, size_t n)
{
  FILE *f = fopen(path, "wb");
  int ok;
  if (f == NULL) {
    fprintf(stderr, "ivs_pack: cannot create %s\n", path);
    return 0;
  }
  ok = fwrite(b, 1u, n, f) == n;
  if (fclose(f) != 0) {
    ok = 0;
  }
  if (!ok) {
    fprintf(stderr, "ivs_pack: write error on %s\n", path);
    remove(path);
  }
  return ok;
}

static int pk_exec(const char *out)
{
  static uint8_t img[PK_EXEC_BYTES];
  uint32_t i;
  if (pk_list.first < PK_EXEC_BASE || pk_list.first - PK_EXEC_BASE + pk_list.count > PK_EXEC_BYTES / 2u) {
    fprintf(stderr, "ivs_pack: --exec needs every word in 1000-1FFF\n");
    return 0;
  }
  memset(img, 0, sizeof img);
  for (i = 0u; i < pk_list.count; i++) {
    pk_put16(img + 2u * (pk_list.first - PK_EXEC_BASE + i), pk_list.word[i]);
  }
  return pk_write(out, img, sizeof img);
}

static int pk_ivsc(const char *out)
{
  static const uint8_t magic[8] = {0x89u, 0x49u, 0x56u, 0x53u, 0x43u, 0x0Du, 0x0Au, 0x1Au};
  static uint8_t img[24u + 8u + 2u * PK_MAX_WORDS + 32u];
  size_t total = 24u + 8u + 2u * (size_t)pk_list.count + 32u;
  size_t at = 32u;
  uint32_t i;
  memset(img, 0, sizeof img);
  memcpy(img, magic, sizeof magic);
  pk_put16(img + 8, 1u);                          /* version */
  pk_put16(img + 16, (uint32_t)(total >> 16));    /* total_bytes, high half */
  pk_put16(img + 18, (uint32_t)(total & 0xFFFFu)); /* total_bytes, low half */
  pk_put16(img + 20, 1u);                         /* segment_count */
  pk_put16(img + 24, pk_list.first);              /* entry: addr */
  pk_put16(img + 26, pk_list.count);              /* len_words */
  img[28] = 10u;                                  /* width; attr and reserved stay 0 */
  for (i = 0u; i < pk_list.count; i++) {
    pk_put16(img + at, pk_list.word[i]);
    at += 2u;
  }
  ivs_sha256_digest(img, total - 32u, img + total - 32u);
  return pk_write(out, img, total);
}

static int pk_dump(void)
{
  uint32_t i;
  for (i = 0u; i < pk_list.count; i++) {
    if (printf("%04X %04X\n", (unsigned)(pk_list.first + i), (unsigned)pk_list.word[i]) < 0) {
      return 0;
    }
  }
  return fflush(stdout) == 0;
}

/* name=file: name is [a-z_][a-z0-9_]* of at most 48 characters. */
static int pk_name_ok(const char *s, size_t n)
{
  size_t i;
  if (n == 0u || n > 48u || (s[0] >= '0' && s[0] <= '9')) {
    return 0;
  }
  for (i = 0u; i < n; i++) {
    char c = s[i];
    if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_')) {
      return 0;
    }
  }
  return 1;
}

static int pk_include(const char *out, int argc, char **argv)
{
  static uint8_t buf[PK_MAX_INCLUDE_BYTES];
  FILE *o;
  int k;
  int ok = 1;
  o = fopen(out, "wb");
  if (o == NULL) {
    fprintf(stderr, "ivs_pack: cannot create %s\n", out);
    return 0;
  }
  fprintf(o, "/* Generated by tools/pack (ivs_pack --c-include): build output only, never committed. */\n");
  for (k = 0; k < argc && ok; k++) {
    const char *eq = strchr(argv[k], '=');
    size_t nlen = eq == NULL ? 0u : (size_t)(eq - argv[k]);
    size_t n;
    size_t i;
    FILE *f;
    if (eq == NULL || !pk_name_ok(argv[k], nlen)) {
      fprintf(stderr, "ivs_pack: --c-include wants name=file with name [a-z_][a-z0-9_]*: %s\n", argv[k]);
      ok = 0;
      break;
    }
    f = fopen(eq + 1, "rb");
    if (f == NULL) {
      fprintf(stderr, "ivs_pack: cannot open %s\n", eq + 1);
      ok = 0;
      break;
    }
    n = fread(buf, 1u, sizeof buf, f);
    if (ferror(f) || fgetc(f) != EOF || n == 0u) {
      fprintf(stderr, "ivs_pack: %s is empty, unreadable or above %u bytes\n", eq + 1, PK_MAX_INCLUDE_BYTES);
      ok = 0;
    }
    fclose(f);
    if (!ok) {
      break;
    }
    fprintf(o, "#define IVS_IMG_");
    for (i = 0u; i < nlen; i++) {
      char c = argv[k][i];
      fputc(c >= 'a' && c <= 'z' ? c - 'a' + 'A' : c, o);
    }
    fprintf(o, "_BYTES %luu\nstatic const uint8_t ivs_img_%.*s[%lu] = {", (unsigned long)n, (int)nlen, argv[k],
            (unsigned long)n);
    for (i = 0u; i < n; i++) {
      fprintf(o, "%s%s0x%02Xu", i == 0u ? "" : ",", i % 16u == 0u ? "\n  " : " ", (unsigned)buf[i]);
    }
    fprintf(o, "};\n");
  }
  if (fclose(o) != 0) {
    ok = 0;
  }
  if (!ok) {
    remove(out);
  }
  return ok;
}

static int pk_usage(void)
{
  fprintf(stderr, "usage: ivs_pack --exec <listing> <out> [--patch ADDR=HEX]...\n"
                  "       ivs_pack --ivsc <listing> <out> [--patch ADDR=HEX]...\n"
                  "       ivs_pack --dump-words <listing> [--patch ADDR=HEX]...\n"
                  "       ivs_pack --c-include <out.inc> <name>=<file>...\n");
  return 1;
}

int main(int argc, char **argv)
{
  const char *mode;
  int pos;
  int i;
  if (argc < 3) {
    return pk_usage();
  }
  mode = argv[1];
  if (strcmp(mode, "--c-include") == 0) {
    return argc >= 4 && pk_include(argv[2], argc - 3, argv + 3) ? 0 : 1;
  }
  if (strcmp(mode, "--dump-words") == 0) {
    pos = 1;
  } else if (strcmp(mode, "--exec") == 0 || strcmp(mode, "--ivsc") == 0) {
    pos = 2;
  } else {
    return pk_usage();
  }
  if (argc < 2 + pos) {
    return pk_usage();
  }
  if (!pk_read(argv[2])) {
    return 1;
  }
  for (i = 2 + pos; i < argc; i += 2) {
    if (strcmp(argv[i], "--patch") != 0 || i + 1 >= argc) {
      return pk_usage();
    }
    if (!pk_patch(argv[i + 1])) {
      return 1;
    }
  }
  if (strcmp(mode, "--dump-words") == 0) {
    return pk_dump() ? 0 : 1;
  }
  if (strcmp(mode, "--exec") == 0) {
    return pk_exec(argv[3]) ? 0 : 1;
  }
  return pk_ivsc(argv[3]) ? 0 : 1;
}
