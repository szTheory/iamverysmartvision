/* SPDX-License-Identifier: MIT */
/* ivs-inspect's `dis` limb (Phase 18, plan 18-03, TOOL-02): turn one of the project's own listings into
 * a source that ivs_asm reassembles into the same words. The frozen contract is docs/inspection.md and
 * its `## Disassembly` section is the whole rule; where this comment and that page disagree, the page
 * wins.
 *
 * What this file knows, and what it deliberately does not. The mnemonic is the listing's own `ROW`
 * column, verbatim, and the operand text is the listing's own `ASM` column cut by three fixed rules, in
 * this order: the trailing note at the first run of two or more spaces, the branch or jump target
 * annotation at ` -> `, and the leading token only when it repeats the row field (where it does not, that
 * column is a note and the mnemonic stands alone). Nothing here maps a word to a mnemonic or a mnemonic
 * to a word: there is no decoder table, and the encoding stays the assembler's and the listing's. The
 * only other knowledge in the file is which `#` header keys ivs_asm accepts, and when an `ORG` directive
 * has to be emitted so a gap in the listing is preserved rather than silently packed. The round trip
 * through ivs_asm, compared by the `ivs_pack --dump-words` word dump of both listings, is what proves
 * those rules: a listing with an annotation this file mis-cuts fails loudly there instead of
 * disassembling wrongly, and a row it has never seen is refused by name and address before the caller
 * creates any output.
 *
 * The known set of the row refusal is the assembler's accepted mnemonic vocabulary - the spellings
 * tools/asm/asm.c accepts as an opcode - which is a vocabulary and not a table: it holds no encoding, no
 * operand shape and no word, so it cannot become a second truth about the instruction set. A data row is
 * outside it on purpose: ivs_asm writes `DATA` for a DECLE word, and that row's mnemonic is a directive
 * rather than an opcode, so a listing carrying one is refused here (fail-closed) instead of being emitted
 * as a guess. Every row the boot-stub corpus uses is in the vocabulary, which is the corpus the frozen
 * page names.
 *
 * The header rule is the assembler's own, not a copy of it by accident: ivs_asm's header block ends at
 * the first line that is not `# key: value` with a key it accepts, and a bare `#` or a prose line inside
 * the block is refused as `asm-header`. So only the listing's leading run of accepted-key header lines is
 * copied, verbatim and in order, and every other `#` line is dropped. Copying every `#` line would make
 * the reassembly fail for a reason that has nothing to do with the disassembler.
 *
 * The interface is declared at both ends - here and in inspect.c - because the tool has no header file.
 * Its contract: 0 the emitted source is in *out_text (malloc'd, *out_len bytes, the caller owns it); 1 a
 * row was refused (detail carries the refusal's detail and the caller prints the token); 2 a bound,
 * allocation or listing-grammar error (detail carries the message, which carries no token). */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The tool's own bounds. A listing is a few hundred lines at the listing grammar's caps, and the emitted
 * source is one line per instruction, so these are refusals rather than expectations. */
#define IX_DIS_MAX_INS 4096u
#define IX_DIS_HDR_MAX 4096u
#define IX_DIS_INDENT "        "

/* The eight keys ivs_asm's header grammar accepts (tools/asm/asm.c, as_hkeys), in its order. */
static const char *const ix_dis_hkeys[] = {"id",     "kind", "contract", "profile",
                                           "window", "label", "entry",    "stack"};
#define IX_DIS_NKEYS (sizeof ix_dis_hkeys / sizeof ix_dis_hkeys[0])

/* The assembler's accepted mnemonic vocabulary: names only. This is not a decoder table - no encoding,
 * no operand shape, no word appears here - and it exists for one question, whether a listing's row is a
 * mnemonic ivs_asm can assemble. */
static const char *const ix_dis_rows[] = {
  "HLT",  "SDBD", "EIS",  "DIS",  "TCI",  "CLRC", "SETC", "INCR", "DECR", "COMR", "NEGR", "ADCR", "GSWD",
  "RSWD", "NOP",  "SIN",  "SWAP", "SLL",  "RLC",  "SLLC", "SLR",  "SAR",  "RRC",  "SARC", "TSTR", "JR",
  "MOVR", "ADDR", "SUBR", "CMPR", "ANDR", "XORR", "CLRR", "B",    "BC",   "BOV",  "BPL",  "BZE",  "BLT",
  "BLE",  "BUSC", "NOPP", "BNC",  "BNOV", "BMI",  "BNZE", "BGE",  "BGT",  "BESC", "BEXT", "MVO",  "MVI",
  "ADD",  "SUB",  "CMP",  "AND",  "XOR",  "MVO@", "MVI@", "ADD@", "SUB@", "CMP@", "AND@", "XOR@", "PSHR",
  "PULR", "MVII", "ADDI", "SUBI", "CMPI", "ANDI", "XORI", "J",    "JE",   "JD",   "JSR",  "JSRE", "JSRD"};
#define IX_DIS_NROWS (sizeof ix_dis_rows / sizeof ix_dis_rows[0])

/* The interface inspect.c calls and declares at its own end (see the file header for the contract). */
int ivs_inspect_dis_emit(const uint8_t *bytes, size_t len, char **out_text, size_t *out_len, char *detail,
                         size_t detail_cap);

/* One instruction of the listing: its address, its word count (itself plus the following `-` rows), and
 * the two slices of the listing's own text this file emits. The slices point into the caller's buffer. */
typedef struct ix_dis_insn {
  uint32_t addr;
  uint32_t words;
  size_t row_off;
  size_t row_len;
  size_t op_off;
  size_t op_len;
} ix_dis_insn;

static int ix_dis_ws(char c)
{
  return c == ' ' || c == '\t';
}

/* Four hex digits, upper or lower case. */
static int ix_dis_hex4(const char *a, uint32_t *out)
{
  uint32_t v = 0u;
  int i;
  for (i = 0; i < 4; i++) {
    char c = a[i];
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
  *out = v;
  return 1;
}

/* A `BITS` group: one or more characters, every one of them `0` or `1`. The groups are the only fields
 * between the word and the row, so a field that is not one of them is the row. */
static int ix_dis_bits(const char *s, size_t n)
{
  size_t i;
  if (n == 0u) {
    return 0;
  }
  for (i = 0u; i < n; i++) {
    if (s[i] != '0' && s[i] != '1') {
      return 0;
    }
  }
  return 1;
}

/* The next whitespace-separated field of [at, end): 0 when the rest of the line is blank. */
static int ix_dis_next(const char *s, size_t end, size_t *at, size_t *off, size_t *flen)
{
  size_t i = *at;
  while (i < end && ix_dis_ws(s[i])) {
    i++;
  }
  if (i >= end) {
    *at = i;
    return 0;
  }
  *off = i;
  while (i < end && !ix_dis_ws(s[i])) {
    i++;
  }
  *flen = i - *off;
  *at = i;
  return 1;
}

/* The index of the first run of two or more spaces in [off, off + n), or n. This is the cut between the
 * listing's instruction text and its trailing note: an operand's own separators are single spaces. */
static size_t ix_dis_note_cut(const char *s, size_t off, size_t n)
{
  size_t i;
  for (i = 0u; i + 1u < n; i++) {
    if (s[off + i] == ' ' && s[off + i + 1u] == ' ') {
      return i;
    }
  }
  return n;
}

/* The index of ` -> ` in [off, off + n), or n: the branch and jump target annotation the listing writes
 * with single spaces on both sides, so it survives the note cut and would otherwise be emitted as
 * operand text. */
static size_t ix_dis_target_cut(const char *s, size_t off, size_t n)
{
  size_t i;
  for (i = 0u; i + 4u <= n; i++) {
    if (s[off + i] == ' ' && s[off + i + 1u] == '-' && s[off + i + 2u] == '>' && s[off + i + 3u] == ' ') {
      return i;
    }
  }
  return n;
}

/* A line that holds nothing but blanks. */
static int ix_dis_blank(const char *s, size_t n)
{
  size_t i;
  for (i = 0u; i < n; i++) {
    if (!ix_dis_ws(s[i])) {
      return 0;
    }
  }
  return 1;
}

/* n without its trailing spaces. */
static size_t ix_dis_rtrim(const char *s, size_t off, size_t n)
{
  while (n > 0u && s[off + n - 1u] == ' ') {
    n--;
  }
  return n;
}

static int ix_dis_row_known(const char *s, size_t n)
{
  size_t i;
  for (i = 0u; i < IX_DIS_NROWS; i++) {
    if (strlen(ix_dis_rows[i]) == n && memcmp(s, ix_dis_rows[i], n) == 0) {
      return 1;
    }
  }
  return 0;
}

/* A header line of ivs_asm's grammar - `# key: value`, key of lower-case letters, value not empty - with
 * a key of the eight it accepts. */
static int ix_dis_header(const char *s, size_t n)
{
  size_t k = 2u;
  size_t i;
  if (n < 5u || s[0] != '#' || s[1] != ' ') {
    return 0;
  }
  while (k < n && s[k] >= 'a' && s[k] <= 'z') {
    k++;
  }
  if (k == 2u || k + 2u >= n || s[k] != ':' || s[k + 1u] != ' ') {
    return 0;
  }
  for (i = 0u; i < IX_DIS_NKEYS; i++) {
    if (strlen(ix_dis_hkeys[i]) == k - 2u && memcmp(s + 2, ix_dis_hkeys[i], k - 2u) == 0) {
      return 1;
    }
  }
  return 0;
}

/* One word line: fills *ins for an instruction row and returns 1, or advances the current instruction's
 * word count for a `-` row and returns 0. -1 is a listing-grammar error and -2 a refused row, both with
 * detail filled. */
static int ix_dis_word_line(const char *bytes, size_t base, size_t end, unsigned long lineno,
                            ix_dis_insn *ins, ix_dis_insn *cur, char *detail, size_t cap)
{
  const char *line = bytes + base;
  size_t at = 0u;
  size_t aoff = 0u;
  size_t alen = 0u;
  size_t woff = 0u;
  size_t wlen = 0u;
  size_t goff = 0u;
  size_t glen = 0u;
  size_t roff = 0u;
  size_t rlen = 0u;
  size_t groups = 0u;
  size_t asm_off;
  size_t asm_len;
  size_t n;
  uint32_t addr = 0u;
  uint32_t word = 0u;

  if (!ix_dis_next(line, end, &at, &aoff, &alen) || alen != 4u || !ix_dis_hex4(line + aoff, &addr)) {
    (void)snprintf(detail, cap, "line %lu is not ADDR HEX4 BITS ROW ASM", lineno);
    return -1;
  }
  if (!ix_dis_next(line, end, &at, &woff, &wlen) || wlen != 4u || !ix_dis_hex4(line + woff, &word)) {
    (void)snprintf(detail, cap, "line %lu is not ADDR HEX4 BITS ROW ASM", lineno);
    return -1;
  }
  if (word > 0x03FFu) {
    (void)snprintf(detail, cap, "line %lu holds the word %04X, above the 10-bit decle 03FF", lineno,
                   (unsigned)word);
    return -1;
  }
  for (;;) {
    if (!ix_dis_next(line, end, &at, &goff, &glen)) {
      (void)snprintf(detail, cap, "line %lu is not ADDR HEX4 BITS ROW ASM", lineno);
      return -1;
    }
    if (!ix_dis_bits(line + goff, glen)) {
      roff = goff;
      rlen = glen;
      break;
    }
    groups++;
  }
  if (groups == 0u) {
    (void)snprintf(detail, cap, "line %lu has no BITS group between the word and the row", lineno);
    return -1;
  }

  if (rlen == 1u && line[roff] == '-') {
    if (cur == NULL) {
      (void)snprintf(detail, cap, "line %lu is an operand word with no instruction above it", lineno);
      return -1;
    }
    cur->words++;
    return 0;
  }
  if (!ix_dis_row_known(line + roff, rlen)) {
    (void)snprintf(detail, cap, "the row %.*s at %04X is not one of the mnemonics ivs_asm accepts",
                   (int)rlen, line + roff, (unsigned)addr);
    return -2;
  }

  /* The `ASM` column: the rest of the line after the row field, its leading separator dropped. */
  asm_off = at;
  while (asm_off < end && ix_dis_ws(line[asm_off])) {
    asm_off++;
  }
  asm_len = ix_dis_rtrim(line, asm_off, end - asm_off);
  asm_len = ix_dis_note_cut(line, asm_off, asm_len);
  asm_len = ix_dis_rtrim(line, asm_off, asm_len);
  asm_len = ix_dis_target_cut(line, asm_off, asm_len);
  asm_len = ix_dis_rtrim(line, asm_off, asm_len);

  /* Cut 3: the leading token, dropped only when it repeats the row field. Where it does not, the column
   * is a note - the listing's zero-fill rows carry one - and the mnemonic is emitted with no operand. */
  n = 0u;
  while (n < asm_len && !ix_dis_ws(line[asm_off + n])) {
    n++;
  }
  if (n == rlen && memcmp(line + asm_off, line + roff, n) == 0) {
    size_t op = asm_off + n;
    while (op < asm_off + asm_len && ix_dis_ws(line[op])) {
      op++;
    }
    ins->op_off = base + op;
    ins->op_len = asm_off + asm_len - op;
  } else {
    ins->op_off = base + asm_off;
    ins->op_len = 0u;
  }
  ins->addr = addr;
  ins->words = 1u;
  ins->row_off = base + roff;
  ins->row_len = rlen;
  return 1;
}

/* The emitted source: the copied header, then one `ORG` before the first instruction and at every gap,
 * then one indented line per instruction. The address of the next instruction is the previous one's
 * address plus its word count, so a gap or a jump in the listing is preserved rather than packed. */
static int ix_dis_build(const char *bytes, const char *hdr, size_t hdr_len, const ix_dis_insn *ins,
                        size_t nins, char **out_text, size_t *out_len, char *detail, size_t cap)
{
  size_t total = hdr_len;
  size_t i;
  size_t at = 0u;
  char *out;

  for (i = 0u; i < nins; i++) {
    int gap = i == 0u || ins[i].addr != ins[i - 1u].addr + ins[i - 1u].words;
    if (gap) {
      total += strlen(IX_DIS_INDENT "ORG   $0000\n");
    }
    total += strlen(IX_DIS_INDENT) + ins[i].row_len + 1u;
    if (ins[i].op_len > 0u) {
      total += 1u + ins[i].op_len;
    }
  }
  out = malloc(total + 1u);
  if (out == NULL) {
    (void)snprintf(detail, cap, "cannot allocate the emitted source buffer");
    return 0;
  }
  if (hdr_len > 0u) {
    (void)memcpy(out, hdr, hdr_len);
    at = hdr_len;
  }
  for (i = 0u; i < nins; i++) {
    int gap = i == 0u || ins[i].addr != ins[i - 1u].addr + ins[i - 1u].words;
    if (gap) {
      char org[sizeof IX_DIS_INDENT "ORG   $0000\n"];
      int n = snprintf(org, sizeof org, IX_DIS_INDENT "ORG   $%04X\n", (unsigned)ins[i].addr);
      (void)memcpy(out + at, org, (size_t)n);
      at += (size_t)n;
    }
    (void)memcpy(out + at, IX_DIS_INDENT, strlen(IX_DIS_INDENT));
    at += strlen(IX_DIS_INDENT);
    (void)memcpy(out + at, bytes + ins[i].row_off, ins[i].row_len);
    at += ins[i].row_len;
    if (ins[i].op_len > 0u) {
      out[at++] = ' ';
      (void)memcpy(out + at, bytes + ins[i].op_off, ins[i].op_len);
      at += ins[i].op_len;
    }
    out[at++] = '\n';
  }
  out[at] = '\0';
  *out_text = out;
  *out_len = at;
  return 1;
}

int ivs_inspect_dis_emit(const uint8_t *bytes, size_t len, char **out_text, size_t *out_len, char *detail,
                         size_t detail_cap)
{
  const char *text = (const char *)bytes;
  ix_dis_insn *ins;
  ix_dis_insn *cur = NULL;
  size_t nins = 0u;
  size_t at = 0u;
  size_t hdr_cap = 0u;
  char *hdr = NULL;
  unsigned long lineno = 0ul;
  int in_header = 1;
  int rc = 2;

  *out_text = NULL;
  *out_len = 0u;
  detail[0] = '\0';
  ins = malloc(IX_DIS_MAX_INS * sizeof *ins);
  hdr = malloc(IX_DIS_HDR_MAX);
  if (ins == NULL || hdr == NULL) {
    (void)snprintf(detail, detail_cap, "cannot allocate the disassembler's own buffers");
    goto done;
  }
  while (at < len) {
    size_t end = at;
    size_t n;
    while (end < len && text[end] != '\n') {
      end++;
    }
    n = end - at;
    lineno++;
    if (in_header && ix_dis_header(text + at, n)) {
      if (hdr_cap + n + 1u > IX_DIS_HDR_MAX) {
        (void)snprintf(detail, detail_cap, "the listing's header block is above %u bytes",
                       (unsigned)IX_DIS_HDR_MAX);
        goto done;
      }
      (void)memcpy(hdr + hdr_cap, text + at, n);
      hdr_cap += n;
      hdr[hdr_cap++] = '\n';
      at = end + 1u;
      continue;
    }
    in_header = 0;
    if (n > 0u && text[at] != '#' && !ix_dis_blank(text + at, n)) {
      ix_dis_insn one;
      int r;
      memset(&one, 0, sizeof one);
      r = ix_dis_word_line(text, at, n, lineno, &one, cur, detail, detail_cap);
      if (r < 0) {
        rc = r == -2 ? 1 : 2;
        goto done;
      }
      if (r == 1) {
        if (nins == IX_DIS_MAX_INS) {
          (void)snprintf(detail, detail_cap, "the listing holds more than %u instructions",
                         (unsigned)IX_DIS_MAX_INS);
          goto done;
        }
        ins[nins] = one;
        cur = &ins[nins];
        nins++;
      }
    }
    at = end + 1u;
  }
  if (!ix_dis_build(text, hdr, hdr_cap, ins, nins, out_text, out_len, detail, detail_cap)) {
    goto done;
  }
  rc = 0;

 done:
  free(hdr);
  free(ins);
  if (rc != 0) {
    free(*out_text);
    *out_text = NULL;
    *out_len = 0u;
  }
  return rc;
}
