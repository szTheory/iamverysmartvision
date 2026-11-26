/* SPDX-License-Identifier: MIT */
/* ivs_flatwrap: wrap one flat cartridge image, at a base address the operator states, into the
 * project's own IVSC v1 container (Phase 14, plan 14-01, D-47, TOOL-07). It is a development tool:
 * build-only, no install rule, never shipped, never part of the core and never a gate step.
 *
 * It states bytes and detects nothing. The operator supplies the three facts the container needs:
 * the byte length of the operator's flat file, the base address its words belong at, and the order
 * of the two bytes of a word. Nothing is sniffed and nothing is guessed. The tool parses no other
 * container: its input is a byte count and an order, so a file of any name is the same input, and
 * no extension means anything here.
 *
 * The layout is the core's, not this file's: the sizes, the version, the width, the word maximum
 * and the address space come from content/ivsc.h, and the digest comes from common/ivs_sha256.h,
 * both reached through the ivs_core_internal include path that tools/pack already links. The eight
 * magic bytes are restated below, because they are a static array inside core/src/content/ivsc.c
 * rather than a header constant (tools/pack/pack.c restates the same eight); declaring them here
 * instead of exporting a new core symbol keeps the core's defined and public symbol sets unchanged
 * for a build-only tool. The container is specified in docs/formats/ivsc-v1.md, and the load is the
 * drift detector: the core's parser refuses a container whose magic differs from its own.
 *
 * Usage: ivs_flatwrap <flat> <base> <out> [--word-order hi-first|lo-first]
 *   <base> is four hex digits without a prefix, $4000 to $FFFF: the operator's address, stated.
 *   --word-order hi-first pairs the first byte of the file with the high half of the first word
 *   (the project's one wire order) and is the default; lo-first swaps the two bytes of each word.
 *
 * Check order, part of the published contract (the container's own specification states its order
 * and that the first failing check decides the refusal):
 *   1 empty          no words at all (a zero-byte input)          flatwrap-empty
 *   2 length         an odd byte count                            flatwrap-length
 *   3 words          more than 65,536 words                       flatwrap-words
 *   4 base           below $4000                                  flatwrap-base
 *   5 crossing       base + words above $10000                    flatwrap-cross
 *   6 output tree    the output's physical parent directory is inside the work tree and not under
 *                    refs/                                       flatwrap-output-tree
 *   7 output exists  a path that already exists                   flatwrap-output-exists
 *   8 word range     a word above $03FF                           flatwrap-word-range
 * A refusal is one stderr line `ivs_flatwrap: <token>: <detail>`, exit status 1, and no output
 * file. Any other failure (an input that cannot be read, an output directory that cannot be
 * resolved, a write error) is a file or usage error: one stderr line without a token, exit status
 * 2. The eight tokens are the whole typed surface and are pinned in data/expected/flat-refusals.ids.
 *
 * Both paths this tool touches are operator-supplied, so neither probe blocks on one: the input is
 * opened without blocking and refused unless fstat reports a regular file, and the output's
 * existence is a stat, not an open (review WR-01). The technique is runner/hostio.c's
 * ivs_hostio_read, which exists for the same reason: fopen("rb") on a FIFO waits for a writer that
 * never comes, and a refusal that never returns is worse than a wrong token. The exclusive create
 * (`wbx`) is still what enforces rule 7; the stat is only the early, non-blocking answer.
 *
 * Rule 1 is the lower bound the container imposes and the tool used to lack: a segment holds at
 * least one word, so an input of no bytes has no legal container at all. The container's own
 * specification states len_words is 1 to 65,535, IVS_IVSC_MIN_BYTES is 66 (prefix 24 + entry 8 +
 * one word 2 + trailer 32), and the core's parser refuses len_words == 0 outright
 * (IVS_IVSC_R_SEGMENT_LENGTH, content/ivsc.c). Writing a container the project's own loader refuses
 * is the one thing this tool must never do, so the empty input is refused by its own token before
 * any byte count arithmetic, and its boundary positive is the smallest legal container: one word.
 *
 * The output is derived material of content the operator holds. It is written only to the path
 * named, refused inside the work tree (refs/ is the one exception, as it is for tools/owner-run.sh),
 * never a tracked path (the ignore file and the binary extension list already carry the extension),
 * never a fixture, never a golden, never a claim artifact, and no digest of it is recorded anywhere
 * but the one line this tool prints.
 *
 * On success stdout carries exactly one line: the 64 lower-case hex digits of the SHA-256 of the
 * whole container including its trailer, which is the digest the loader publishes as
 * slot.cart.sha256 (core/src/content/ivsc.h). Nothing else is printed on success.
 *
 * This tool names no other implementation and describes no other format. */
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common/ivs_sha256.h"
#include "content/ivsc.h"

#ifndef IVS_FLATWRAP_TREE_ROOT
#error "IVS_FLATWRAP_TREE_ROOT must be defined by tools/flatwrap/CMakeLists.txt"
#endif

/* The container's own magic, restated (see the header comment). */
static const uint8_t fw_magic[IVS_IVSC_MAGIC_BYTES] = {0x89u, 0x49u, 0x56u, 0x53u, 0x43u, 0x0Du, 0x0Au, 0x1Au};

/* The largest input this tool reads: 65,537 words is the first count that rule 2 refuses, so a
 * longer file is refused from its byte count and never read into the buffer at all. */
#define FW_INPUT_MAX 131074u
/* The most words an accepted input can hold, and the size of the pairing buffer: the container's
 * own total word limit, which also covers the 65,536-word boundary that rule 4 refuses. */
#define FW_WORDS_MAX IVS_IVSC_ADDRESS_SPACE_WORDS
/* realpath writes at most PATH_MAX bytes; 4096 is that bound on every platform this tree builds on. */
#define FW_PATH_MAX 4096u

static uint8_t fw_in[FW_INPUT_MAX];
static uint16_t fw_word[FW_WORDS_MAX];
static uint8_t fw_img[IVS_IVSC_MAX_BYTES];

static void fw_put16(uint8_t *p, uint32_t v)
{
  p[0] = (uint8_t)((v >> 8) & 0xFFu);
  p[1] = (uint8_t)(v & 0xFFu);
}

static void fw_put32(uint8_t *p, uint32_t v)
{
  fw_put16(p, (v >> 16) & 0xFFFFu);
  fw_put16(p + 2, v & 0xFFFFu);
}

/* One stderr line carrying a token of the pinned set, and the refusal exit status. */
static int fw_refuse(const char *token, const char *detail)
{
  fprintf(stderr, "ivs_flatwrap: %s: %s\n", token, detail);
  return 1;
}

/* A failure that is not one of the eight rules: a file or usage error, with no token. */
static int fw_fail(const char *detail)
{
  fprintf(stderr, "ivs_flatwrap: %s\n", detail);
  return 2;
}

static int fw_usage(void)
{
  return fw_fail("usage: ivs_flatwrap <flat> <base> <out> [--word-order hi-first|lo-first]");
}

/* 1 when s is four hex digits and *base is their value, else 0. */
static int fw_parse_base(const char *s, uint32_t *base)
{
  uint32_t v = 0u;
  size_t i;
  if (strlen(s) != 4u) {
    return 0;
  }
  for (i = 0u; i < 4u; i++) {
    char c = s[i];
    uint32_t d;
    if (c >= '0' && c <= '9') {
      d = (uint32_t)(c - '0');
    } else if (c >= 'a' && c <= 'f') {
      d = (uint32_t)(c - 'a') + 10u;
    } else if (c >= 'A' && c <= 'F') {
      d = (uint32_t)(c - 'A') + 10u;
    } else {
      return 0;
    }
    v = (v << 4) | d;
  }
  *base = v;
  return 1;
}

/* 1 when the physical parent directory of out lies inside the work tree and is not under refs/,
 * 0 when it is outside or under refs/, -1 when the question cannot be answered (the caller then
 * refuses with a file error rather than write blind). The parent is resolved physically, so a
 * spelling through `..` or through a symlink to a directory inside the tree is the same answer as
 * the literal path: the technique tools/owner-run.sh uses for its own image paths. */
static int fw_inside_tree(const char *out)
{
  char root[FW_PATH_MAX];
  char dir[FW_PATH_MAX];
  char phys[FW_PATH_MAX];
  char *slash;
  size_t rootlen;

  if (realpath(IVS_FLATWRAP_TREE_ROOT, root) == NULL) {
    fprintf(stderr, "ivs_flatwrap: cannot resolve the work-tree root %s\n", IVS_FLATWRAP_TREE_ROOT);
    return -1;
  }
  if (strlen(out) >= sizeof dir) {
    fprintf(stderr, "ivs_flatwrap: the output path is too long: %s\n", out);
    return -1;
  }
  memcpy(dir, out, strlen(out) + 1u);
  slash = strrchr(dir, '/');
  if (slash == NULL) {
    memcpy(dir, ".", 2u);
  } else if (slash == dir) {
    dir[1] = '\0';
  } else {
    *slash = '\0';
  }
  if (realpath(dir, phys) == NULL) {
    fprintf(stderr, "ivs_flatwrap: cannot resolve the output directory %s\n", dir);
    return -1;
  }
  rootlen = strlen(root);
  if (strcmp(phys, root) != 0 && !(strncmp(phys, root, rootlen) == 0 && phys[rootlen] == '/')) {
    return 0;
  }
  if (strcmp(phys + rootlen, "/refs") == 0 || strncmp(phys + rootlen, "/refs/", 6u) == 0) {
    return 0;
  }
  return 1;
}

/* Write the one-segment container, then print the digest of the whole file. */
static int fw_write(const char *out, uint32_t base, uint32_t n_words)
{
  size_t total = (size_t)IVS_IVSC_PREFIX_BYTES + (size_t)IVS_IVSC_ENTRY_BYTES + 2u * (size_t)n_words
                 + (size_t)IVS_IVSC_TRAILER_BYTES;
  size_t at = (size_t)IVS_IVSC_PREFIX_BYTES + (size_t)IVS_IVSC_ENTRY_BYTES;
  uint8_t dig[IVS_SHA256_DIGEST_BYTES];
  FILE *f;
  int ok;
  uint32_t i;

  memset(fw_img, 0, total);
  memcpy(fw_img, fw_magic, IVS_IVSC_MAGIC_BYTES);
  fw_put16(fw_img + 8u, IVS_IVSC_VERSION);          /* flags at 10 and required_features at 12 stay 0 */
  fw_put32(fw_img + 16u, (uint32_t)total);          /* total_bytes, trailer included */
  fw_put16(fw_img + 20u, 1u);                       /* segment_count; reserved at 22 stays 0 */
  fw_put16(fw_img + 24u, base);                     /* entry: addr */
  fw_put16(fw_img + 26u, n_words);                  /* entry: len_words */
  fw_img[28] = (uint8_t)IVS_IVSC_WIDTH_BITS;        /* entry: width; attr at 29 and reserved stay 0 */
  for (i = 0u; i < n_words; i++) {
    fw_put16(fw_img + at, fw_word[i]);
    at += 2u;
  }
  ivs_sha256_digest(fw_img, total - (size_t)IVS_IVSC_TRAILER_BYTES, fw_img + total - (size_t)IVS_IVSC_TRAILER_BYTES);

  /* Exclusive create: a truncating open would destroy the file rule 6 exists to refuse. */
  f = fopen(out, "wbx");
  if (f == NULL) {
    if (errno == EEXIST) {
      return fw_refuse("flatwrap-output-exists", out);
    }
    fprintf(stderr, "ivs_flatwrap: cannot create %s\n", out);
    return 2;
  }
  ok = fwrite(fw_img, 1u, total, f) == total;
  if (fclose(f) != 0) {
    ok = 0;
  }
  if (!ok) {
    remove(out);
    fprintf(stderr, "ivs_flatwrap: write error on %s\n", out);
    return 2;
  }

  ivs_sha256_digest(fw_img, total, dig);
  for (i = 0u; i < IVS_SHA256_DIGEST_BYTES; i++) {
    printf("%02x", (unsigned int)dig[i]);
  }
  printf("\n");
  return 0;
}

int main(int argc, char **argv)
{
  const char *flat;
  const char *out;
  uint32_t base = 0u;
  uint32_t n_words;
  int lo_first = 0;
  int i;
  long size;
  FILE *f;
  struct stat sb;
  int fd;
  size_t got;
  uint32_t k;
  int inside;
  char detail[160];

  if (argc < 4) {
    return fw_usage();
  }
  flat = argv[1];
  out = argv[3];
  for (i = 4; i < argc; i += 2) {
    if (strcmp(argv[i], "--word-order") != 0 || i + 1 >= argc) {
      return fw_usage();
    }
    if (strcmp(argv[i + 1], "hi-first") == 0) {
      lo_first = 0;
    } else if (strcmp(argv[i + 1], "lo-first") == 0) {
      lo_first = 1;
    } else {
      return fw_usage();
    }
  }

  /* Rule 1 (empty), rule 2 (length) and rule 3 (words) are decided from the byte count, before
   * anything is read, so an oversized file never reaches the buffer and the word count is checked
   * before any multiplication by two. */
  fd = open(flat, O_RDONLY | O_NOCTTY | O_NONBLOCK);
  if (fd < 0) {
    fprintf(stderr, "ivs_flatwrap: cannot read %s\n", flat);
    return 2;
  }
  if (fstat(fd, &sb) != 0 || !S_ISREG(sb.st_mode)) {
    (void)close(fd);
    fprintf(stderr, "ivs_flatwrap: %s is not a regular file\n", flat);
    return 2;
  }
  f = fdopen(fd, "rb");
  if (f == NULL) {
    (void)close(fd);
    fprintf(stderr, "ivs_flatwrap: cannot read %s\n", flat);
    return 2;
  }
  if (fseek(f, 0L, SEEK_END) != 0 || (size = ftell(f)) < 0 || fseek(f, 0L, SEEK_SET) != 0) {
    fclose(f);
    fprintf(stderr, "ivs_flatwrap: cannot read %s\n", flat);
    return 2;
  }
  if (size == 0L) {
    fclose(f);
    return fw_refuse("flatwrap-empty", "the input is empty, and a container segment holds at least one word");
  }
  if ((size % 2L) != 0L) {
    fclose(f);
    snprintf(detail, sizeof detail, "the input is %ld bytes, and a word is two bytes", size);
    return fw_refuse("flatwrap-length", detail);
  }
  if ((unsigned long)size / 2UL > (unsigned long)FW_WORDS_MAX) {
    fclose(f);
    snprintf(detail, sizeof detail, "the input is %ld bytes, more than 65536 words", size);
    return fw_refuse("flatwrap-words", detail);
  }
  n_words = (uint32_t)((unsigned long)size / 2UL);

  /* Rule 4 (base) and rule 5 (crossing). */
  if (!fw_parse_base(argv[2], &base) || base < 0x4000u) {
    fclose(f);
    snprintf(detail, sizeof detail, "the base %s is not four hex digits in 4000 to FFFF", argv[2]);
    return fw_refuse("flatwrap-base", detail);
  }
  if (base + n_words > IVS_IVSC_ADDRESS_SPACE_WORDS) {
    fclose(f);
    snprintf(detail, sizeof detail, "%u words at %04X end above 10000", n_words, base);
    return fw_refuse("flatwrap-cross", detail);
  }

  /* Rule 6 (output tree) and rule 7 (output exists). */
  inside = fw_inside_tree(out);
  if (inside < 0) {
    fclose(f);
    return 2;
  }
  if (inside != 0) {
    fclose(f);
    snprintf(detail, sizeof detail, "the output %s is inside the work tree outside refs/", out);
    return fw_refuse("flatwrap-output-tree", detail);
  }
  if (stat(out, &sb) == 0) {
    fclose(f);
    snprintf(detail, sizeof detail, "the output %s already exists", out);
    return fw_refuse("flatwrap-output-exists", detail);
  }

  /* Read the input and pair its bytes into words, then rule 8 (word range). */
  got = fread(fw_in, 1u, (size_t)size, f);
  if (got != (size_t)size) {
    fclose(f);
    fprintf(stderr, "ivs_flatwrap: cannot read %s\n", flat);
    return 2;
  }
  fclose(f);
  for (k = 0u; k < n_words; k++) {
    uint8_t a = fw_in[2u * (size_t)k];
    uint8_t b = fw_in[2u * (size_t)k + 1u];
    uint8_t hi = lo_first != 0 ? b : a;
    uint8_t lo = lo_first != 0 ? a : b;
    fw_word[k] = (uint16_t)(((uint16_t)hi << 8) | (uint16_t)lo);
  }
  for (k = 0u; k < n_words; k++) {
    if (fw_word[k] > IVS_IVSC_WORD_MAX) {
      snprintf(detail, sizeof detail, "word %u is %04X, above 03FF", k, (unsigned int)fw_word[k]);
      return fw_refuse("flatwrap-word-range", detail);
    }
  }

  return fw_write(out, base, n_words);
}
