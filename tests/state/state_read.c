/* SPDX-License-Identifier: MIT */
/* Suite state, family read (plan 08-12, D-13, D-14, D-16): the instance-free reader of the IVSN v1 container and the public call
 * ivs_snapshot_describe (ivs/ivs_state.h). The refusal matrix of this file is written from the specification (docs/formats/ivsn-v1.md:
 * Layout, Check order, Refusal reasons) and from the contract text of the header, never from the reader: every row names the mutation
 * of a valid snapshot, the reason it must give (the status follows from the reason by the table of the specification) and the byte
 * offset and section index it must report. The layout constants below are typed from the Layout table of the specification, not taken
 * from the core. A snapshot is built once by the writer (verified against the literals by the other cases of the suite) and each row
 * changes bytes of a copy of it.
 *
 * What every row checks (r_call_check): the status; the whole report, 400 bytes of a caller struct larger than the v1 size, filled with
 * two different poison bytes in turn, against the report the contract of the header allows (a refusal writes only reason, byte_offset,
 * section_index and the header facts the supplied length let it read, struct_size and every other member keep the poison; a success
 * writes the 360 bytes of the v1 report in full and keeps the tail), so a byte written that the contract does not list, or a listed byte
 * not written, fails; and that the supplied bytes are unchanged. The input is a heap copy of exactly the supplied length (the dev
 * preset's sanitizer finds any read past it); a fault build (IVS_STATE_FAULT_RUN) adds a zeroed tail, so that a reader that trusts a
 * claimed length reads defined bytes and fails by its status, not by a crash. */
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/ivs_sha256.h"
#include "ivs/ivs.h"
#include "machine/machine.h"
#include "state_cases.h"

/* ---- the layout of docs/formats/ivsn-v1.md, typed from its Layout table ---- */
#define R_NONE 0xFFFFFFFFu /* section_index of a refusal that concerns no section */
#define R_MAX 16384u       /* IVS_SNAPSHOT_MAX_BYTES */
#define R_MIN 644u         /* the smallest container: the header 612 and the trailer 32 */
#define R_REP 360u         /* the v1 size of the report */
#define R_O_FORMAT 8u
#define R_O_FLAGS 10u
#define R_O_TOTAL 12u
#define R_O_SCHEMA 16u
#define R_O_COUNT 20u
#define R_O_SDIG 24u
#define R_O_ID 56u
#define R_O_PARAMS 120u
#define R_O_SLOTS 152u
#define R_O_COMP 272u
#define R_O_TABLE 304u
#define R_ENTRY(i) (304u + 44u * (uint32_t)(i))

/* The input of a fault build gets a zeroed tail (see above); a normal build gets exactly the bytes it supplies. */
#ifdef IVS_STATE_FAULT_RUN
#define R_TAIL 4400u
#else
#define R_TAIL 0u
#endif

static uint32_t r_be16(const uint8_t *p)
{
  return ((uint32_t)p[0] << 8) | (uint32_t)p[1];
}

static uint32_t r_be32(const uint8_t *p)
{
  return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

/* The status each reason travels with: the table of "Refusal reasons" of the specification (58 to 71 are the reasons of this plan). */
static ivs_status r_status_of(uint32_t reason)
{
  switch (reason) {
  case 58u: /* SNAP_BUF_NULL */
  case 59u: /* SNAP_REPORT_OVERLAP */
    return IVS_E_BAD_ARG;
  case 60u: /* SNAP_OVERSIZED */
    return IVS_E_OVERSIZED;
  case 61u: /* SNAP_TRUNCATED */
  case 62u: /* SNAP_MAGIC */
  case 67u: /* SNAP_TOTAL_LENGTH */
  case 68u: /* SNAP_SECTION_COUNT */
  case 69u: /* SNAP_SECTION_TABLE */
  case 71u: /* SNAP_PROFILE_ID_FORM */
    return IVS_E_MALFORMED;
  case 63u: /* SNAP_FORMAT_VERSION */
  case 64u: /* SNAP_FLAGS */
  case 65u: /* SNAP_SCHEMA_VERSION */
  case 66u: /* SNAP_SCHEMA_DIGEST */
    return IVS_E_UNSUPPORTED;
  case 70u: /* SNAP_TRAILER */
    return IVS_E_DIGEST_MISMATCH;
  default:
    return IVS_OK;
  }
}

/* ---- the base snapshot ---- */

static uint8_t r_base[IVS_SNAPSHOT_MAX_BYTES];
static size_t r_base_n;

/* A valid snapshot of a stepped instance of content A, built once: 1 when it exists and has the length of the literal. */
static int r_base_get(void)
{
  ivs_ms_inst x;
  uint64_t w = 0u;
  uint32_t total = 0u;
  if (r_base_n != 0u) {
    return 1;
  }
  if (!ivsn_lit_u32("total-bytes", &total) || !ivs_ms_powered(&x, IVS_MS_CONTENT_A, 0x5Au)) {
    return 0;
  }
  if (!ivs_ms_step(x.m, 3333u, NULL) || ivs_machine_snapshot_save(x.m, r_base, sizeof r_base, &w) != IVS_OK || w != total) {
    ivs_test_diag("the base snapshot could not be built (%u bytes, the literal says %u)", (unsigned)w, (unsigned)total);
    ivs_ms_free(&x);
    return 0;
  }
  ivs_ms_free(&x);
  r_base_n = (size_t)w;
  return 1;
}

/* A heap copy of exactly len bytes of src (a fault build adds a zeroed tail); at least one byte is allocated, so the pointer is never NULL. */
static uint8_t *r_heap_copy(const uint8_t *src, size_t len)
{
  uint8_t *p = (uint8_t *)malloc((len > 0u ? len : 1u) + R_TAIL);
  if (p == NULL) {
    return NULL;
  }
  if (len > 0u) {
    memcpy(p, src, len);
  }
  if (R_TAIL > 0u) {
    memset(p + len, 0, R_TAIL);
  }
  return p;
}

/* ---- the report the contract allows ---- */

/* The judge case runs deliberately corrupted expectations: it silences the diagnostics of the runner meanwhile. r_seen counts the calls
 * checked per reason (58 to 71, index reason - 58) for the record of the reasons-covered case. */
static int r_quiet;
static uint32_t r_seen[14];

/* A caller struct larger than the v1 size, so that the tail past 360 bytes shows a write past the v1 report. */
typedef union r_rep {
  ivs_snapshot_report r;
  uint64_t align;
  uint8_t b[400];
} r_rep;

static void r_put32(uint8_t *exp, size_t at, uint32_t v)
{
  memcpy(exp + at, &v, sizeof v);
}

static void r_put64(uint8_t *exp, size_t at, uint64_t v)
{
  memcpy(exp + at, &v, sizeof v);
}

/* Edits exp, the report as the caller gave it, into the report the contract of ivs/ivs_state.h says the call leaves. src holds the
 * supplied bytes (readable of them were read: 0 for an argument refusal and for OVERSIZED). A refusal (reason not 0) writes reason,
 * byte_offset and section_index, and the three header facts the supplied length let it read; a success writes the 360 bytes in full. */
static void r_expect_report(uint8_t *exp, const uint8_t *src, size_t readable, uint32_t reason, uint64_t off, uint32_t sec)
{
  uint32_t i;
  if (reason != 0u) {
    r_put32(exp, offsetof(ivs_snapshot_report, reason), reason);
    r_put64(exp, offsetof(ivs_snapshot_report, byte_offset), off);
    r_put32(exp, offsetof(ivs_snapshot_report, section_index), sec);
    if (readable >= 12u) {
      r_put32(exp, offsetof(ivs_snapshot_report, format_version), r_be16(src + R_O_FORMAT));
    }
    if (readable >= 16u) {
      r_put64(exp, offsetof(ivs_snapshot_report, total_bytes), (uint64_t)r_be32(src + R_O_TOTAL));
    }
    if (readable >= 24u) {
      r_put32(exp, offsetof(ivs_snapshot_report, schema_version), r_be32(src + R_O_SCHEMA));
    }
    return;
  }
  memset(exp, 0, R_REP);
  r_put32(exp, offsetof(ivs_snapshot_report, struct_size), R_REP);
  r_put32(exp, offsetof(ivs_snapshot_report, format_version), r_be16(src + R_O_FORMAT));
  r_put32(exp, offsetof(ivs_snapshot_report, schema_version), r_be32(src + R_O_SCHEMA));
  r_put64(exp, offsetof(ivs_snapshot_report, total_bytes), (uint64_t)r_be32(src + R_O_TOTAL));
  r_put32(exp, offsetof(ivs_snapshot_report, section_index), R_NONE);
  memcpy(exp + offsetof(ivs_snapshot_report, profile_id), src + R_O_ID, 64u);
  memcpy(exp + offsetof(ivs_snapshot_report, profile_params_sha256), src + R_O_PARAMS, 32u);
  memcpy(exp + offsetof(ivs_snapshot_report, composite_sha256), src + R_O_COMP, 32u);
  for (i = 0u; i < 3u; i++) {
    size_t at = offsetof(ivs_snapshot_report, slots) + (size_t)i * sizeof(ivs_image_report);
    const uint8_t *rec = src + R_O_SLOTS + 40u * i;
    r_put32(exp, at + offsetof(ivs_image_report, struct_size), (uint32_t)sizeof(ivs_image_report));
    r_put32(exp, at + offsetof(ivs_image_report, kind), i + 1u);
    r_put32(exp, at + offsetof(ivs_image_report, loaded), r_be32(rec));
    r_put32(exp, at + offsetof(ivs_image_report, length_bytes), r_be32(rec + 36u));
    memcpy(exp + at + offsetof(ivs_image_report, sha256), rec + 4u, 32u);
  }
}

/* One call of ivs_snapshot_describe on (buf, claimed), twice, into a report of 400 bytes poisoned with 0xA5 and then with 0x5A: the status
 * must be the one of the reason, the report must be exactly the one the contract allows for each poison, and the readable supplied bytes
 * (the bytes of src) must be unchanged. reason 0 expects a success. *produced gets the bit of the reason when everything held. */
static int r_call_check(const char *name, const uint8_t *buf, uint64_t claimed, size_t readable, const uint8_t *src, uint32_t reason,
                        uint64_t off, uint32_t sec, uint32_t *produced)
{
  static const uint8_t poison[2] = {0xA5u, 0x5Au};
  ivs_status want = reason == 0u ? IVS_OK : r_status_of(reason);
  int ok = 1;
  uint32_t k;
  for (k = 0u; k < 2u; k++) {
    r_rep rep;
    uint8_t exp[sizeof rep.b];
    ivs_status st;
    size_t i;
    memset(rep.b, poison[k], sizeof rep.b);
    rep.r.struct_size = (uint32_t)sizeof rep.b;
    memcpy(exp, rep.b, sizeof exp);
    r_expect_report(exp, src, readable, reason, off, sec);
    st = ivs_snapshot_describe(buf, claimed, &rep.r);
    if (st != want) {
      if (!r_quiet) {
        ivs_test_diag("row '%s': status %s (want %s)", name, ivs_status_name(st), ivs_status_name(want));
      }
      ok = 0;
    }
    for (i = 0u; i < sizeof exp; i++) {
      if (rep.b[i] != exp[i]) {
        if (!r_quiet) {
          ivs_test_diag("row '%s' (poison %02X): report byte %u is %02X, the contract says %02X", name, (unsigned)poison[k], (unsigned)i,
                        (unsigned)rep.b[i], (unsigned)exp[i]);
        }
        ok = 0;
        break;
      }
    }
  }
  if (readable > 0u && memcmp(buf, src, readable) != 0) {
    if (!r_quiet) {
      ivs_test_diag("row '%s': the supplied bytes were changed", name);
    }
    ok = 0;
  }
  if (reason >= 58u && reason <= 71u) {
    r_seen[reason - 58u]++;
    if (ok) {
      *produced |= 1u << (reason - 58u);
    }
  }
  return ok;
}

/* ---- the rows ---- */

/* A row: a mutation of a copy of the base snapshot and the outcome it must give. Patches apply in order after the fill; op 1 flips the
 * bits of v, op 2 sets the byte to v. reseal recomputes the trailer of the mutated copy (over the base length) so that the row reaches
 * the structure check it is about. has_len supplies len bytes (zero filled past the base length) instead of the whole snapshot; big_len
 * claims that length for a one-byte real buffer (the reader must refuse before it reads). reason 0 is a success. */
#define R_PATCHES 40u
typedef struct r_patch {
  uint16_t off;
  uint8_t op;
  uint8_t v;
} r_patch;

typedef struct r_row {
  char name[64];
  uint8_t reseal;
  uint8_t has_len;
  uint8_t fill_val;
  uint8_t np;
  uint16_t fill_off;
  uint16_t fill_len;
  uint32_t len;
  uint64_t big_len;
  uint32_t reason;
  uint64_t off;
  uint32_t sec;
  r_patch p[R_PATCHES];
} r_row;

typedef struct r_tab {
  r_row *row;
  size_t n;
  size_t cap;
  int bad;
} r_tab;

static int r_tab_init(r_tab *t, size_t cap)
{
  memset(t, 0, sizeof *t);
  t->row = (r_row *)calloc(cap, sizeof *t->row);
  t->cap = cap;
  return t->row != NULL;
}

static void r_tab_free(r_tab *t)
{
  free(t->row);
  memset(t, 0, sizeof *t);
}

static r_row *r_addv(r_tab *t, const char *fmt, va_list ap)
{
  static r_row spare;
  r_row *r;
  if (t->n >= t->cap) {
    t->bad = 1;
    memset(&spare, 0, sizeof spare);
    return &spare;
  }
  r = &t->row[t->n++];
  memset(r, 0, sizeof *r);
  (void)vsnprintf(r->name, sizeof r->name, fmt, ap);
  r->sec = R_NONE;
  return r;
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 2, 3)))
#endif
static r_row *r_add(r_tab *t, const char *fmt, ...)
{
  r_row *r;
  va_list ap;
  va_start(ap, fmt);
  r = r_addv(t, fmt, ap);
  va_end(ap);
  return r;
}

static void r_patch_add(r_tab *t, r_row *r, uint32_t off, uint8_t op, uint8_t v)
{
  if (r->np >= R_PATCHES || off > 0xFFFFu) {
    t->bad = 1;
    return;
  }
  r->p[r->np].off = (uint16_t)off;
  r->p[r->np].op = op;
  r->p[r->np].v = v;
  r->np++;
}

static void r_xor(r_tab *t, r_row *r, uint32_t off, uint8_t m)
{
  r_patch_add(t, r, off, 1u, m);
}

static void r_set(r_tab *t, r_row *r, uint32_t off, uint8_t v)
{
  r_patch_add(t, r, off, 2u, v);
}

static void r_set16(r_tab *t, r_row *r, uint32_t off, uint32_t v)
{
  r_set(t, r, off, (uint8_t)(v >> 8));
  r_set(t, r, off + 1u, (uint8_t)v);
}

static void r_set32(r_tab *t, r_row *r, uint32_t off, uint32_t v)
{
  r_set16(t, r, off, v >> 16);
  r_set16(t, r, off + 2u, v & 0xFFFFu);
}

static void r_fill(r_row *r, uint32_t off, uint32_t len, uint8_t v)
{
  r->fill_off = (uint16_t)off;
  r->fill_len = (uint16_t)len;
  r->fill_val = v;
}

/* The outcome of the row: reason 0 for a success, else the reason, the byte offset and the section index it must report. */
static void r_expect(r_row *r, uint32_t reason, uint64_t off, uint32_t sec)
{
  r->reason = reason;
  r->off = off;
  r->sec = sec;
}

/* Builds the input of a row: work is the mutated working copy (zero filled past the base length), *h a heap copy of exactly the bytes the
 * row supplies (NULL when malloc fails), *claimed the length the call is given and *readable the number of real bytes behind it. */
static void r_row_make(const r_row *w, uint8_t *work, uint8_t **h, uint64_t *claimed, size_t *readable)
{
  size_t i;
  memset(work, 0, IVS_SNAPSHOT_MAX_BYTES + 64u);
  memcpy(work, r_base, r_base_n);
  if (w->fill_len > 0u) {
    memset(work + w->fill_off, w->fill_val, w->fill_len);
  }
  for (i = 0u; i < w->np; i++) {
    uint8_t *b = &work[w->p[i].off];
    *b = w->p[i].op == 1u ? (uint8_t)(*b ^ w->p[i].v) : w->p[i].v;
  }
  if (w->reseal != 0u) {
    ivs_sha256_digest(work, r_base_n - 32u, work + r_base_n - 32u);
  }
  if (w->big_len != 0u) {
    *h = r_heap_copy(work, 1u);
    *claimed = w->big_len;
    *readable = 0u;
  } else {
    *readable = w->has_len != 0u ? (size_t)w->len : r_base_n;
    *h = r_heap_copy(work, *readable);
    *claimed = *readable;
  }
}

/* Runs one row. */
static int r_run_row(const r_row *w, uint32_t *produced)
{
  static uint8_t work[IVS_SNAPSHOT_MAX_BYTES + 64u];
  uint64_t claimed;
  size_t len;
  uint8_t *h;
  int ok;
  r_row_make(w, work, &h, &claimed, &len);
  if (h == NULL) {
    return 0;
  }
  ok = r_call_check(w->name, h, claimed, len, work, w->reason, w->off, w->sec, produced);
  free(h);
  return ok;
}

/* Runs every row of a table; the number of rows that failed. */
static uint32_t r_run_rows(const r_tab *t, uint32_t *produced)
{
  uint32_t bad = 0u;
  size_t i;
  for (i = 0u; i < t->n; i++) {
    if (!r_run_row(&t->row[i], produced)) {
      bad++;
    }
  }
  return bad;
}

/* ---- family: the header (magic, versions, flags, schema, count) and the check order among them ---- */

static void r_build_header(r_tab *t)
{
  static const uint32_t fv[] = {0u, 2u, 0xFFFFu, 0x0100u, 0x8001u, 0x0101u};
  static const uint32_t sv[] = {0u, 2u, 0xFFFFFFFFu, 0x01000000u, 0x00010000u, 0x00000100u, 0x80000001u};
  static const uint32_t sc[] = {0u, 1u, 6u, 8u, 0xFFFFFFFFu, 0x01000007u, 0x00010007u, 0x00000107u, 0x80000007u};
  uint32_t k;
  r_row *r;
  for (k = 0u; k < 8u; k++) {
    r = r_add(t, "magic byte %u flipped", (unsigned)k);
    r_xor(t, r, k, 0x01u);
    r_expect(r, 62u, k, R_NONE);
  }
  r = r_add(t, "magic bytes 5 and 2 flipped: the first difference is reported");
  r_xor(t, r, 5u, 0x80u);
  r_xor(t, r, 2u, 0x01u);
  r_expect(r, 62u, 2u, R_NONE);
  r = r_add(t, "magic all zero");
  r_fill(r, 0u, 8u, 0u);
  r_expect(r, 62u, 0u, R_NONE);
  r = r_add(t, "magic first byte 7-bit (high bit cleared)");
  r_set(t, r, 0u, 0x09u);
  r_expect(r, 62u, 0u, R_NONE);
  for (k = 0u; k < sizeof fv / sizeof fv[0]; k++) {
    r = r_add(t, "format version %04X", (unsigned)fv[k]);
    r_set16(t, r, R_O_FORMAT, fv[k]);
    r_expect(r, 63u, R_O_FORMAT, R_NONE);
  }
  for (k = 0u; k < 16u; k++) {
    r = r_add(t, "flags bit %u", (unsigned)k);
    r_set16(t, r, R_O_FLAGS, 1u << k);
    r_expect(r, 64u, R_O_FLAGS, R_NONE);
  }
  r = r_add(t, "flags all bits");
  r_set16(t, r, R_O_FLAGS, 0xFFFFu);
  r_expect(r, 64u, R_O_FLAGS, R_NONE);
  for (k = 0u; k < sizeof sv / sizeof sv[0]; k++) {
    r = r_add(t, "schema version %08X", (unsigned)sv[k]);
    r_set32(t, r, R_O_SCHEMA, sv[k]);
    r_expect(r, 65u, R_O_SCHEMA, R_NONE);
  }
  for (k = 0u; k < 4u; k++) {
    r = r_add(t, "schema version byte %u flipped", (unsigned)k);
    r_xor(t, r, R_O_SCHEMA + k, 0x01u);
    r_expect(r, 65u, R_O_SCHEMA, R_NONE);
  }
  for (k = 0u; k < 32u; k++) {
    r = r_add(t, "schema digest byte %u flipped", (unsigned)k);
    r_xor(t, r, R_O_SDIG + k, 0x01u);
    r_expect(r, 66u, R_O_SDIG, R_NONE);
  }
  r = r_add(t, "schema digest all zero");
  r_fill(r, R_O_SDIG, 32u, 0u);
  r_expect(r, 66u, R_O_SDIG, R_NONE);
  for (k = 0u; k < sizeof sc / sizeof sc[0]; k++) {
    r = r_add(t, "section count %08X", (unsigned)sc[k]);
    r_set32(t, r, R_O_COUNT, sc[k]);
    r_expect(r, 68u, R_O_COUNT, R_NONE);
  }

  /* the order: the first failing class wins */
  r = r_add(t, "order: magic and format version");
  r_xor(t, r, 0u, 0x01u);
  r_set16(t, r, R_O_FORMAT, 2u);
  r_expect(r, 62u, 0u, R_NONE);
  r = r_add(t, "order: format version and flags");
  r_set16(t, r, R_O_FORMAT, 2u);
  r_set16(t, r, R_O_FLAGS, 1u);
  r_expect(r, 63u, R_O_FORMAT, R_NONE);
  r = r_add(t, "order: flags and schema version");
  r_set16(t, r, R_O_FLAGS, 1u);
  r_set32(t, r, R_O_SCHEMA, 2u);
  r_expect(r, 64u, R_O_FLAGS, R_NONE);
  r = r_add(t, "order: schema version and schema digest");
  r_set32(t, r, R_O_SCHEMA, 2u);
  r_xor(t, r, R_O_SDIG, 0x01u);
  r_expect(r, 65u, R_O_SCHEMA, R_NONE);
  r = r_add(t, "order: schema digest and a total below the length");
  r_xor(t, r, R_O_SDIG, 0x01u);
  r_set32(t, r, R_O_TOTAL, 4000u);
  r_expect(r, 66u, R_O_SDIG, R_NONE);
  r = r_add(t, "order: a total above the length and a wrong section count");
  r_set32(t, r, R_O_TOTAL, 4300u);
  r_set32(t, r, R_O_COUNT, 8u);
  r_expect(r, 61u, r_base_n, R_NONE);
  r = r_add(t, "order: a total below the length and a wrong section count");
  r_set32(t, r, R_O_TOTAL, 4000u);
  r_set32(t, r, R_O_COUNT, 8u);
  r_expect(r, 67u, R_O_TOTAL, R_NONE);
  r = r_add(t, "order: a wrong section count and a section table defect");
  r_set32(t, r, R_O_COUNT, 6u);
  r_set(t, r, R_ENTRY(0), 0u);
  r_expect(r, 68u, R_O_COUNT, R_NONE);
  r = r_add(t, "order: a wrong section count and a wrong trailer");
  r_set32(t, r, R_O_COUNT, 6u);
  r_xor(t, r, (uint32_t)(r_base_n - 1u), 0x01u);
  r_expect(r, 68u, R_O_COUNT, R_NONE);
  r = r_add(t, "order: every header field wrong at once");
  r_xor(t, r, 7u, 0x01u);
  r_set16(t, r, R_O_FORMAT, 9u);
  r_set16(t, r, R_O_FLAGS, 9u);
  r_set32(t, r, R_O_SCHEMA, 9u);
  r_set32(t, r, R_O_COUNT, 9u);
  r_expect(r, 62u, 7u, R_NONE);
}

/* ---- family: the length, and the total the snapshot states ---- */

static void r_build_length(r_tab *t)
{
  static const uint32_t cut[] = {0u, 1u, 7u, 8u, 11u, 12u, 15u, 16u, 23u, 24u, 100u, 303u, 304u, 611u, 612u, 643u, 644u, 645u, 700u, 2000u, 4000u, 4266u, 4267u};
  static const uint64_t big[] = {16385u, 16386u, 20000u, 65536u, 0xFFFFFFFFu, 0x100000000ull, 0x1000010CBull, 0x7FFFFFFFFFFFFFFFull,
                                 0x8000000000000000ull, 0xFFFFFFFFFFFFFFFFull};
  uint32_t k;
  r_row *r;
  for (k = 0u; k < sizeof big / sizeof big[0]; k++) {
    r = r_add(t, "length %llx claimed for one real byte", (unsigned long long)big[k]);
    r->big_len = big[k];
    r_expect(r, 60u, R_MAX, R_NONE);
  }
  for (k = 0u; k < sizeof cut / sizeof cut[0]; k++) {
    r = r_add(t, "length %u (the snapshot cut)", (unsigned)cut[k]);
    r->has_len = 1u;
    r->len = cut[k];
    r_expect(r, 61u, cut[k], R_NONE);
  }
  /* the length class comes before everything the bytes say: a cut snapshot with a damaged header is still reported as cut */
  r = r_add(t, "length 643 with the magic flipped: the length is judged first");
  r->has_len = 1u;
  r->len = 643u;
  r_xor(t, r, 0u, 0x01u);
  r_expect(r, 61u, 643u, R_NONE);
  r = r_add(t, "length 12 with the magic flipped");
  r->has_len = 1u;
  r->len = 12u;
  r_xor(t, r, 3u, 0x01u);
  r_expect(r, 61u, 12u, R_NONE);
  r = r_add(t, "length 640 with the format version 2");
  r->has_len = 1u;
  r->len = 640u;
  r_set16(t, r, R_O_FORMAT, 2u);
  r_expect(r, 61u, 640u, R_NONE);
  r = r_add(t, "length 400 with the schema digest flipped");
  r->has_len = 1u;
  r->len = 400u;
  r_xor(t, r, R_O_SDIG, 0x01u);
  r_expect(r, 61u, 400u, R_NONE);
  r = r_add(t, "length 4298 (one byte short)");
  r->has_len = 1u;
  r->len = (uint32_t)r_base_n - 1u;
  r_expect(r, 61u, r_base_n - 1u, R_NONE);
  /* a claimed total above the supplied length: truncated, whatever the claim */
  r = r_add(t, "total 4300 claimed, 4299 supplied");
  r_set32(t, r, R_O_TOTAL, 4300u);
  r_expect(r, 61u, r_base_n, R_NONE);
  r = r_add(t, "total 16385 claimed, 4299 supplied");
  r_set32(t, r, R_O_TOTAL, 16385u);
  r_expect(r, 61u, r_base_n, R_NONE);
  r = r_add(t, "total FFFFFFFF claimed, 4299 supplied");
  r_set32(t, r, R_O_TOTAL, 0xFFFFFFFFu);
  r_expect(r, 61u, r_base_n, R_NONE);
  r = r_add(t, "total 80000000 claimed, 4299 supplied");
  r_set32(t, r, R_O_TOTAL, 0x80000000u);
  r_expect(r, 61u, r_base_n, R_NONE);
  r = r_add(t, "total 5000 claimed, 700 supplied");
  r_set32(t, r, R_O_TOTAL, 5000u);
  r->has_len = 1u;
  r->len = 700u;
  r_expect(r, 61u, 700u, R_NONE);
  /* a claimed total below the supplied length, or a total that is not the size this build derives: total length */
  r = r_add(t, "total 4298 claimed, 4299 supplied");
  r_set32(t, r, R_O_TOTAL, 4298u);
  r_expect(r, 67u, R_O_TOTAL, R_NONE);
  r = r_add(t, "total 0 claimed");
  r_set32(t, r, R_O_TOTAL, 0u);
  r_expect(r, 67u, R_O_TOTAL, R_NONE);
  r = r_add(t, "total 643 claimed");
  r_set32(t, r, R_O_TOTAL, 643u);
  r_expect(r, 67u, R_O_TOTAL, R_NONE);
  r = r_add(t, "total 4000 claimed");
  r_set32(t, r, R_O_TOTAL, 4000u);
  r_expect(r, 67u, R_O_TOTAL, R_NONE);
  r = r_add(t, "4300 supplied with the total 4299 (a byte after the container)");
  r->has_len = 1u;
  r->len = 4300u;
  r_expect(r, 67u, R_O_TOTAL, R_NONE);
  r = r_add(t, "4299 + 32 supplied with the total 4299");
  r->has_len = 1u;
  r->len = 4331u;
  r_expect(r, 67u, R_O_TOTAL, R_NONE);
  r = r_add(t, "16384 supplied with the total 4299");
  r->has_len = 1u;
  r->len = R_MAX;
  r_expect(r, 67u, R_O_TOTAL, R_NONE);
  r = r_add(t, "4300 supplied and 4300 claimed: not the size this build derives");
  r->has_len = 1u;
  r->len = 4300u;
  r_set32(t, r, R_O_TOTAL, 4300u);
  r_expect(r, 67u, R_O_TOTAL, R_NONE);
  r = r_add(t, "4298 supplied and 4298 claimed");
  r->has_len = 1u;
  r->len = 4298u;
  r_set32(t, r, R_O_TOTAL, 4298u);
  r_expect(r, 67u, R_O_TOTAL, R_NONE);
  r = r_add(t, "644 supplied and 644 claimed");
  r->has_len = 1u;
  r->len = 644u;
  r_set32(t, r, R_O_TOTAL, 644u);
  r_expect(r, 67u, R_O_TOTAL, R_NONE);
  r = r_add(t, "16384 supplied and 16384 claimed");
  r->has_len = 1u;
  r->len = R_MAX;
  r_set32(t, r, R_O_TOTAL, R_MAX);
  r_expect(r, 67u, R_O_TOTAL, R_NONE);
}

/* ---- family: the trailer, after the structure ---- */

static void r_build_trailer(r_tab *t)
{
  static const uint32_t payload[] = {612u, 613u, 646u, 647u, 1000u, 3407u, 3408u, 4022u, 4023u, 4215u, 4216u, 4266u};
  static const uint32_t header[] = {R_O_PARAMS, R_O_PARAMS + 31u, R_O_SLOTS, R_O_SLOTS + 3u, R_O_SLOTS + 4u, R_O_SLOTS + 39u, R_O_SLOTS + 40u,
                                    R_O_SLOTS + 119u, R_O_COMP, R_O_COMP + 31u, R_O_TABLE + 12u, R_O_TABLE + 43u, R_O_TABLE + 44u * 6u + 12u,
                                    R_O_TABLE + 44u * 6u + 43u, R_O_ID, R_O_ID + 23u};
  const uint32_t at = (uint32_t)r_base_n - 32u;
  uint32_t k;
  r_row *r;
  for (k = 0u; k < 32u; k++) {
    r = r_add(t, "trailer byte %u flipped", (unsigned)k);
    r_xor(t, r, at + k, 0x01u);
    r_expect(r, 70u, at, R_NONE);
  }
  for (k = 0u; k < 32u; k++) {
    r = r_add(t, "trailer byte %u inverted", (unsigned)k);
    r_xor(t, r, at + k, 0xFFu);
    r_expect(r, 70u, at, R_NONE);
  }
  r = r_add(t, "trailer all zero");
  r_fill(r, at, 32u, 0u);
  r_expect(r, 70u, at, R_NONE);
  for (k = 0u; k < sizeof payload / sizeof payload[0]; k++) {
    r = r_add(t, "payload byte %u flipped", (unsigned)payload[k]);
    r_xor(t, r, payload[k], 0x01u);
    r_expect(r, 70u, at, R_NONE);
  }
  /* the fields that only the trailer covers: informational digests, the slot records, a valid character of the profile id */
  for (k = 0u; k < sizeof header / sizeof header[0]; k++) {
    r = r_add(t, "header byte %u flipped (only the trailer covers it)", (unsigned)header[k]);
    r_xor(t, r, header[k], 0x01u);
    r_expect(r, 70u, at, R_NONE);
  }
  /* the structure is checked before the trailer: a structural defect with a stale trailer is the structural reason */
  r = r_add(t, "a section domain wrong, trailer stale");
  r_xor(t, r, R_ENTRY(2), 0x01u);
  r_expect(r, 69u, R_ENTRY(2), 2u);
  r = r_add(t, "a section offset wrong, trailer stale");
  r_xor(t, r, R_ENTRY(4) + 7u, 0x01u);
  r_expect(r, 69u, R_ENTRY(4) + 4u, 4u);
  r = r_add(t, "the section count wrong, trailer stale");
  r_set32(t, r, R_O_COUNT, 6u);
  r_expect(r, 68u, R_O_COUNT, R_NONE);
  r = r_add(t, "the total wrong, trailer stale");
  r_set32(t, r, R_O_TOTAL, 4298u);
  r_expect(r, 67u, R_O_TOTAL, R_NONE);
  r = r_add(t, "the schema digest wrong, trailer stale");
  r_xor(t, r, R_O_SDIG + 5u, 0x01u);
  r_expect(r, 66u, R_O_SDIG, R_NONE);
  /* the form of the profile id is checked after the trailer (step 6 follows step 5): a bad id with a stale trailer is the trailer */
  r = r_add(t, "profile id with a control byte, trailer stale");
  r_set(t, r, R_O_ID + 3u, 0x01u);
  r_expect(r, 70u, at, R_NONE);
  r = r_add(t, "profile id without a NUL, trailer stale");
  r_fill(r, R_O_ID, 64u, 'a');
  r_expect(r, 70u, at, R_NONE);
}

/* ---- family: what describe accepts (and echoes) ---- */

/* Rows that change only fields the reader does not judge (the parameter digest, the three slot records, the composite and section digests
 * are informational for describe), with a valid trailer: describe accepts them and the report echoes the bytes. */
static void r_build_ok(r_tab *t)
{
  r_row *r = r_add(t, "the snapshot as written");
  r_expect(r, 0u, 0u, R_NONE);
  r = r_add(t, "parameter digest changed, resealed");
  r_xor(t, r, R_O_PARAMS, 0xFFu);
  r_xor(t, r, R_O_PARAMS + 31u, 0x01u);
  r->reseal = 1u;
  r_expect(r, 0u, 0u, R_NONE);
  r = r_add(t, "composite digest changed, resealed");
  r_xor(t, r, R_O_COMP, 0x80u);
  r_xor(t, r, R_O_COMP + 31u, 0x01u);
  r->reseal = 1u;
  r_expect(r, 0u, 0u, R_NONE);
  r = r_add(t, "slot records changed (loaded 2, length FFFFFFFF, a digest byte), resealed");
  r_set32(t, r, R_O_SLOTS, 2u);
  r_set32(t, r, R_O_SLOTS + 40u + 36u, 0xFFFFFFFFu);
  r_xor(t, r, R_O_SLOTS + 80u + 4u, 0x01u);
  r->reseal = 1u;
  r_expect(r, 0u, 0u, R_NONE);
  r = r_add(t, "section digests changed, resealed");
  r_xor(t, r, R_ENTRY(0) + 12u, 0x01u);
  r_xor(t, r, R_ENTRY(6) + 43u, 0x80u);
  r->reseal = 1u;
  r_expect(r, 0u, 0u, R_NONE);
  r = r_add(t, "parameter digest, slot records and composite all zero, resealed");
  r_fill(r, R_O_PARAMS, R_O_TABLE - R_O_PARAMS, 0u);
  r->reseal = 1u;
  r_expect(r, 0u, 0u, R_NONE);
  r = r_add(t, "parameter digest, slot records and composite all FF, resealed");
  r_fill(r, R_O_PARAMS, R_O_TABLE - R_O_PARAMS, 0xFFu);
  r->reseal = 1u;
  r_expect(r, 0u, 0u, R_NONE);
  r = r_add(t, "section bodies changed, resealed (the content is not judged by describe)");
  r_xor(t, r, 700u, 0xFFu);
  r_xor(t, r, 3500u, 0x01u);
  r->reseal = 1u;
  r_expect(r, 0u, 0u, R_NONE);
}

/* ---- family: the section table (MALFORMED 69) ---- */

/* The seven entries of the table of the specification (Layout and Size derivation): the domains, the absolute offsets and the lengths of the
 * sections. The test state.read.describe-refuse-section-table first holds these typed numbers to the literals of
 * data/expected/ivsn-literals.ids and to the table of the base snapshot, so a row below always mutates the entry it names. */
static const uint8_t r_dom[7] = {1u, 2u, 4u, 16u, 17u, 18u, 19u};
static const uint32_t r_soff[7] = {612u, 647u, 3408u, 4023u, 4074u, 4137u, 4216u};
static const uint32_t r_slen[7] = {35u, 2761u, 615u, 51u, 63u, 79u, 51u};

/* A row of the section table family: resealed, so that it is the table that is refused and never the trailer. The convention for the
 * report of a table defect (the reader and the header of ivs/ivs_state.h): section_index is the entry whose rule failed first, entry by
 * entry in order (domain, the three zero bytes, the offset against the end of the entry before, the length against the end of the
 * payload); byte_offset is the offset of the field of that entry that failed (the domain at the entry start, the first nonzero zero byte,
 * the offset field at +4, the length field at +8); and when every entry passed but the last section does not end where the trailer
 * begins, the entry is the last one and the field its length. */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 4, 5)))
#endif
static r_row *r_tab_row(r_tab *t, uint32_t entry, uint32_t field, const char *fmt, ...)
{
  r_row *r;
  va_list ap;
  va_start(ap, fmt);
  r = r_addv(t, fmt, ap);
  va_end(ap);
  r->reseal = 1u;
  r_expect(r, 69u, R_ENTRY(entry) + field, entry);
  return r;
}

static void r_build_table(r_tab *t)
{
  static const uint8_t odd[] = {5u, 0xFFu, 20u, 3u, 0x80u, 0x11u};
  static const uint32_t off0[] = {611u, 613u, 0u, 0xFFFFFFFFu, 0x100u, 0x00010264u, 0x80000264u};
  static const uint32_t off3[] = {4024u, 4022u, 0u, 0xFFFFFFFFu, 0xFFFFFFFCu, 0x00010FB7u, 0x80000FB7u};
  static const uint32_t big[] = {0xFFFFFFFFu, 0xFFFFFFFEu, 0x80000000u, 0x7FFFFFFFu, 0x00100000u, 0x00010000u};
  uint32_t i;
  uint32_t k;
  r_row *r;
  r = r_add(t, "the table as written, resealed");
  r->reseal = 1u;
  r_expect(r, 0u, 0u, R_NONE);
  /* unknown domains */
  for (i = 0u; i < 7u; i++) {
    r = r_tab_row(t, i, 0u, "entry %u: domain 0", i);
    r_set(t, r, R_ENTRY(i), 0u);
  }
  for (k = 0u; k < sizeof odd; k++) {
    r = r_tab_row(t, 3u, 0u, "entry 3: unknown domain %u", odd[k]);
    r_set(t, r, R_ENTRY(3), odd[k]);
  }
  r = r_tab_row(t, 6u, 0u, "entry 6: domain %u (one above the last)", 20u);
  r_set(t, r, R_ENTRY(6), 20u);
  r = r_tab_row(t, 0u, 0u, "entry 0: domain %u (images, which holds no section)", 3u);
  r_set(t, r, R_ENTRY(0), 3u);
  r = r_tab_row(t, 2u, 0u, "entry 2: domain %u (images, which holds no section)", 3u);
  r_set(t, r, R_ENTRY(2), 3u);
  /* duplicated domains */
  r = r_tab_row(t, 1u, 0u, "entry %u repeats the domain of entry %u", 1u, 0u);
  r_set(t, r, R_ENTRY(1), 1u);
  r = r_tab_row(t, 2u, 0u, "entry %u repeats the domain of entry %u", 2u, 1u);
  r_set(t, r, R_ENTRY(2), 2u);
  r = r_tab_row(t, 4u, 0u, "entry %u repeats the domain of entry %u", 4u, 3u);
  r_set(t, r, R_ENTRY(4), 16u);
  r = r_tab_row(t, 6u, 0u, "entry %u repeats the domain of entry %u", 6u, 5u);
  r_set(t, r, R_ENTRY(6), 18u);
  /* descending order */
  r = r_tab_row(t, 1u, 0u, "entries %u and %u swapped", 1u, 2u);
  r_set(t, r, R_ENTRY(1), 4u);
  r_set(t, r, R_ENTRY(2), 2u);
  r = r_tab_row(t, 5u, 0u, "entries %u and %u swapped", 5u, 6u);
  r_set(t, r, R_ENTRY(5), 19u);
  r_set(t, r, R_ENTRY(6), 18u);
  r = r_tab_row(t, 0u, 0u, "the domains in descending order");
  for (i = 0u; i < 7u; i++) {
    r_set(t, r, R_ENTRY(i), r_dom[6u - i]);
  }
  /* a skipped domain */
  r = r_tab_row(t, 2u, 0u, "entry %u skips a domain (%u)", 2u, 16u);
  r_set(t, r, R_ENTRY(2), 16u);
  r = r_tab_row(t, 5u, 0u, "entry %u skips a domain (%u)", 5u, 19u);
  r_set(t, r, R_ENTRY(5), 19u);
  r = r_tab_row(t, 1u, 0u, "entry %u skips a domain (%u)", 1u, 4u);
  r_set(t, r, R_ENTRY(1), 4u);
  /* the three zero bytes of an entry */
  for (i = 0u; i < 7u; i += 3u) {
    for (k = 1u; k < 4u; k++) {
      r = r_tab_row(t, i, k, "entry %u: zero byte %u not zero", i, k);
      r_set(t, r, R_ENTRY(i) + k, 0x01u);
    }
  }
  r = r_tab_row(t, 2u, 1u, "entry %u: zero byte %u is FF", 2u, 1u);
  r_set(t, r, R_ENTRY(2) + 1u, 0xFFu);
  r = r_tab_row(t, 4u, 3u, "entry %u: zero byte %u is 80", 4u, 3u);
  r_set(t, r, R_ENTRY(4) + 3u, 0x80u);
  /* the first offset, a gap, an overlap, a section that begins before the end of the one before */
  for (k = 0u; k < sizeof off0 / sizeof off0[0]; k++) {
    r = r_tab_row(t, 0u, 4u, "entry 0: offset %u (the first is 612)", off0[k]);
    r_set32(t, r, R_ENTRY(0) + 4u, off0[k]);
  }
  for (k = 0u; k < sizeof off3 / sizeof off3[0]; k++) {
    r = r_tab_row(t, 3u, 4u, "entry 3: offset %u (a gap or an overlap)", off3[k]);
    r_set32(t, r, R_ENTRY(3) + 4u, off3[k]);
  }
  r = r_tab_row(t, 2u, 4u, "entry 2 begins where entry 1 begins");
  r_set32(t, r, R_ENTRY(2) + 4u, r_soff[1]);
  r = r_tab_row(t, 0u, 4u, "every offset one higher");
  for (i = 0u; i < 7u; i++) {
    r_set32(t, r, R_ENTRY(i) + 4u, r_soff[i] + 1u);
  }
  /* a length one more and one less than derived: the next entry no longer begins at the end of this one, and the last section no longer
   * ends where the trailer begins */
  for (k = 0u; k < 2u; k++) {
    uint32_t d = k == 0u ? 1u : 0xFFFFFFFFu; /* +1 and -1 */
    r = r_tab_row(t, 1u, 4u, "entry 0: length %u (derived plus or minus one: entry 1 no longer adjacent)", r_slen[0] + d);
    r_set32(t, r, R_ENTRY(0) + 8u, r_slen[0] + d);
    r = r_tab_row(t, 3u, 4u, "entry 2: length %u (derived plus or minus one: entry 3 no longer adjacent)", r_slen[2] + d);
    r_set32(t, r, R_ENTRY(2) + 8u, r_slen[2] + d);
    r = r_tab_row(t, 6u, 4u, "entry 5: length %u (derived plus or minus one: entry 6 no longer adjacent)", r_slen[5] + d);
    r_set32(t, r, R_ENTRY(5) + 8u, r_slen[5] + d);
    r = r_tab_row(t, 6u, 8u, "entry 6: length %u (the last end one past or before the trailer)", r_slen[6] + d);
    r_set32(t, r, R_ENTRY(6) + 8u, r_slen[6] + d);
  }
  r = r_tab_row(t, 4u, 4u, "entry 3: length 0 (a zero length leaves entry 4 not adjacent)");
  r_set32(t, r, R_ENTRY(3) + 8u, 0u);
  r = r_tab_row(t, 6u, 8u, "entry 6: length 0 (the last end is the trailer start minus 51)");
  r_set32(t, r, R_ENTRY(6) + 8u, 0u);
  /* lengths and offsets near 2^32, whose 32-bit sum wraps */
  for (k = 0u; k < sizeof big / sizeof big[0]; k++) {
    r = r_tab_row(t, 2u, 8u, "entry 2: length %u (does not fit in the payload)", big[k]);
    r_set32(t, r, R_ENTRY(2) + 8u, big[k]);
    r = r_tab_row(t, 0u, 8u, "entry 0: length %u (does not fit in the payload)", big[k]);
    r_set32(t, r, R_ENTRY(0) + 8u, big[k]);
    r = r_tab_row(t, 6u, 8u, "entry 6: length %u (does not fit in the payload)", big[k]);
    r_set32(t, r, R_ENTRY(6) + 8u, big[k]);
  }
  /* the 32-bit sum of the offset and the length wraps onto the next offset, but the offset is not the end of the entry before */
  r = r_tab_row(t, 2u, 4u, "entry 2: offset %u with length %u wrapping to the next offset", 0xFFFFFFF0u, 4039u);
  r_set32(t, r, R_ENTRY(2) + 4u, 0xFFFFFFF0u);
  r_set32(t, r, R_ENTRY(2) + 8u, 4039u);
  r = r_tab_row(t, 6u, 4u, "entry 6: offset %u with length %u wrapping to the trailer start", 0xFFFFFF00u, 4523u);
  r_set32(t, r, R_ENTRY(6) + 4u, 0xFFFFFF00u);
  r_set32(t, r, R_ENTRY(6) + 8u, 4523u);
  r = r_tab_row(t, 0u, 0u, "the whole table zero");
  r_fill(r, R_O_TABLE, R_ENTRY(7) - R_O_TABLE, 0u);
  r = r_tab_row(t, 0u, 0u, "the whole table FF");
  r_fill(r, R_O_TABLE, R_ENTRY(7) - R_O_TABLE, 0xFFu);
  /* the order inside the table: entry by entry, and field by field inside an entry */
  r = r_tab_row(t, 3u, 0u, "entry 3: domain, zero byte and offset all wrong");
  r_set(t, r, R_ENTRY(3), 0u);
  r_set(t, r, R_ENTRY(3) + 2u, 1u);
  r_set32(t, r, R_ENTRY(3) + 4u, 1u);
  r = r_tab_row(t, 3u, 2u, "entry 3: zero byte and offset wrong");
  r_set(t, r, R_ENTRY(3) + 2u, 1u);
  r_set32(t, r, R_ENTRY(3) + 4u, 1u);
  r = r_tab_row(t, 3u, 4u, "entry 3: offset and length wrong");
  r_set32(t, r, R_ENTRY(3) + 4u, 1u);
  r_set32(t, r, R_ENTRY(3) + 8u, 0xFFFFFFFFu);
  r = r_tab_row(t, 0u, 4u, "entry 0 offset wrong, entry 1 domain wrong");
  r_set32(t, r, R_ENTRY(0) + 4u, 1u);
  r_set(t, r, R_ENTRY(1), 0u);
  /* what describe does not judge: lengths that differ from the derived lengths but chain and sum to the same payload are accepted; the
   * lengths are compared with the table of the build by restore (the reason 80), not by the reader */
  r = r_add(t, "entries 1 and 2 lengths %u and %u (derived plus and minus one, chained): not judged by describe", r_slen[1] + 1u, r_slen[2] - 1u);
  r->reseal = 1u;
  r_set32(t, r, R_ENTRY(1) + 8u, r_slen[1] + 1u);
  r_set32(t, r, R_ENTRY(2) + 4u, r_soff[2] + 1u);
  r_set32(t, r, R_ENTRY(2) + 8u, r_slen[2] - 1u);
  r_expect(r, 0u, 0u, R_NONE);
  r = r_add(t, "entry 3 of length %u, entry 4 of length %u (chained): not judged by describe", 0u, r_slen[3] + r_slen[4]);
  r->reseal = 1u;
  r_set32(t, r, R_ENTRY(3) + 8u, 0u);
  r_set32(t, r, R_ENTRY(4) + 4u, r_soff[3]);
  r_set32(t, r, R_ENTRY(4) + 8u, r_slen[3] + r_slen[4]);
  r_expect(r, 0u, 0u, R_NONE);
  r = r_add(t, "the section digests all changed, resealed: informational");
  for (i = 0u; i < 7u; i++) {
    r_xor(t, r, R_ENTRY(i) + 12u + i, 0x01u);
  }
  r->reseal = 1u;
  r_expect(r, 0u, 0u, R_NONE);
}

/* ---- family: the form of the profile id (MALFORMED 71 after the trailer) ---- */

/* Sets the 64 bytes of the profile id field to the NUL padded text s. */
static void r_set_id(r_tab *t, r_row *r, const char *s)
{
  size_t i;
  r_fill(r, R_O_ID, 64u, 0u);
  for (i = 0u; s[i] != '\0'; i++) {
    r_set(t, r, R_O_ID + (uint32_t)i, (uint8_t)s[i]);
  }
}

/* A resealed row about the id: reason 71 at the offset of the first offending byte, or an acceptance. */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 4, 5)))
#endif
static r_row *r_id_row(r_tab *t, uint32_t reason, uint32_t byte_at, const char *fmt, ...)
{
  r_row *r;
  va_list ap;
  va_start(ap, fmt);
  r = r_addv(t, fmt, ap);
  va_end(ap);
  r->reseal = 1u;
  r_expect(r, reason, reason == 0u ? 0u : R_O_ID + byte_at, R_NONE);
  return r;
}

static void r_build_id(r_tab *t)
{
  static const uint8_t ctl[] = {0x01u, 0x09u, 0x0Au, 0x0Du, 0x1Fu, 0x7Fu};
  static const uint8_t high[] = {0x80u, 0xA0u, 0xC3u, 0xFFu};
  static const uint8_t okc[] = {0x20u, 0x21u, 0x2Du, 0x5Fu, 0x7Eu};
  uint32_t k;
  r_row *r;
  r = r_id_row(t, 0u, 0u, "the id as written");
  r = r_id_row(t, 0u, 0u, "empty id (the field all zero)");
  r_fill(r, R_O_ID, 64u, 0u);
  r = r_id_row(t, 0u, 0u, "a well formed id that no profile has");
  r_set_id(t, r, "zzz-not-a-registered-profile");
  r = r_id_row(t, 0u, 0u, "a one character id");
  r_set_id(t, r, "a");
  r = r_id_row(t, 0u, 0u, "63 printable bytes and the NUL");
  r_fill(r, R_O_ID, 63u, 'a');
  r = r_id_row(t, 0u, 0u, "62 printable bytes");
  r_fill(r, R_O_ID, 62u, 'b');
  for (k = 0u; k < sizeof okc; k++) {
    r = r_id_row(t, 0u, 0u, "printable byte %u at index 4 (accepted)", okc[k]);
    r_set(t, r, R_O_ID + 4u, okc[k]);
  }
  /* no NUL in the 64 bytes: the last byte should have been one */
  r = r_id_row(t, 71u, 63u, "64 printable bytes (no NUL)");
  r_fill(r, R_O_ID, 64u, 'a');
  r = r_id_row(t, 71u, 63u, "63 printable bytes and a printable byte 63");
  r_fill(r, R_O_ID, 63u, 'a');
  r_set(t, r, R_O_ID + 63u, 'b');
  r = r_id_row(t, 71u, 63u, "64 spaces (printable, no NUL)");
  r_fill(r, R_O_ID, 64u, 0x20u);
  /* a byte after the first NUL */
  r = r_id_row(t, 71u, 5u, "an embedded NUL at index %u, the rest of the id follows", 4u);
  r_set(t, r, R_O_ID + 4u, 0u);
  r = r_id_row(t, 71u, 1u, "the first byte NUL, the rest of the id follows");
  r_set(t, r, R_O_ID, 0u);
  r = r_id_row(t, 71u, 40u, "a printable byte at index %u after the NUL", 40u);
  r_set(t, r, R_O_ID + 40u, 'x');
  r = r_id_row(t, 71u, 63u, "byte 63 is 01 after the NUL");
  r_set(t, r, R_O_ID + 63u, 0x01u);
  r = r_id_row(t, 71u, 30u, "byte 30 is 80 after the NUL");
  r_set(t, r, R_O_ID + 30u, 0x80u);
  /* a byte that is not printable ASCII before the NUL */
  for (k = 0u; k < sizeof high; k++) {
    r = r_id_row(t, 71u, 2u + k, "non-ASCII byte %u at the index shown", high[k]);
    r_set(t, r, R_O_ID + 2u + k, high[k]);
  }
  r = r_id_row(t, 71u, 0u, "the first byte FF");
  r_set(t, r, R_O_ID, 0xFFu);
  r = r_id_row(t, 71u, 23u, "the last byte of the id 80");
  r_set(t, r, R_O_ID + 23u, 0x80u);
  for (k = 0u; k < sizeof ctl; k++) {
    r = r_id_row(t, 71u, 3u + k, "control byte %u at the index shown", ctl[k]);
    r_set(t, r, R_O_ID + 3u + k, ctl[k]);
  }
  r = r_id_row(t, 71u, 0u, "the first byte 1F");
  r_set(t, r, R_O_ID, 0x1Fu);
  r = r_id_row(t, 71u, 0u, "the whole field FF");
  r_fill(r, R_O_ID, 64u, 0xFFu);
  /* the first offending byte in scan order */
  r = r_id_row(t, 71u, 3u, "a control byte at 3 and a byte after the NUL");
  r_set(t, r, R_O_ID + 3u, 0x01u);
  r_set(t, r, R_O_ID + 40u, 'x');
  r = r_id_row(t, 71u, 50u, "64 printable bytes with a control byte at 50");
  r_fill(r, R_O_ID, 64u, 'a');
  r_set(t, r, R_O_ID + 50u, 0x01u);
  /* the order: the section table before the id */
  r = r_add(t, "a section table defect and a bad id: the table first");
  r->reseal = 1u;
  r_set(t, r, R_ENTRY(2), 0u);
  r_set(t, r, R_O_ID + 3u, 0x01u);
  r_expect(r, 69u, R_ENTRY(2), 2u);
}

/* ---- the argument classes ---- */

/* One call with the report and the buffer placed inside one object, so that they can lie against each other. The buffer starts at
 * buf_at and holds the base snapshot (then 0xC3), the report of rep_at (360 bytes, struct_size 360, poisoned 0xA5) is placed after the
 * snapshot is copied. The whole object must be as the contract says afterwards: the report fields of a refusal, or the full report of a
 * success, and not one byte more changed. */
static int r_overlap_row(const char *name, size_t rep_at, size_t buf_at, uint64_t claimed, uint32_t reason, uint64_t off, uint32_t *produced)
{
  enum { R_OBJ = 21504 };
  uint8_t *region = (uint8_t *)aligned_alloc(16u, R_OBJ);
  uint8_t *exp = (uint8_t *)malloc(R_OBJ);
  ivs_status st;
  ivs_status want = reason == 0u ? IVS_OK : r_status_of(reason);
  size_t readable = 0u;
  int ok = 1;
  if (region == NULL || exp == NULL) {
    free(region);
    free(exp);
    return 0;
  }
  memset(region, 0xC3, R_OBJ);
  memcpy(region + buf_at, r_base, r_base_n);
  memset(region + rep_at, 0xA5, R_REP);
  r_put32(region, rep_at, R_REP);
  memcpy(exp, region, R_OBJ);
  if (reason == 61u || reason == 67u || reason == 0u) {
    readable = (size_t)claimed;
  }
  r_expect_report(exp + rep_at, exp + buf_at, readable, reason, off, R_NONE);
  st = ivs_snapshot_describe(region + buf_at, claimed, (ivs_snapshot_report *)(void *)(region + rep_at));
  if (st != want) {
    ivs_test_diag("row '%s': status %s (want %s)", name, ivs_status_name(st), ivs_status_name(want));
    ok = 0;
  }
  if (memcmp(region, exp, R_OBJ) != 0) {
    size_t i = 0u;
    while (region[i] == exp[i]) {
      i++;
    }
    ivs_test_diag("row '%s': byte %u of the object is %02X, the contract says %02X", name, (unsigned)i, (unsigned)region[i], (unsigned)exp[i]);
    ok = 0;
  }
  if (reason >= 58u && reason <= 71u) {
    r_seen[reason - 58u]++;
    if (ok) {
      *produced |= 1u << (reason - 58u);
    }
  }
  free(region);
  free(exp);
  return ok;
}

/* The refusals by an argument (BAD_ARG 58 and 59), the unusable report (nothing written) and the cases that look like arguments but
 * are not (length 0 with a NULL buffer is a truncation). *produced gets the reasons the rows produced. */
static int r_args(uint32_t *produced)
{
  static const uint32_t bad_size[] = {0u, 1u, 4u, 8u, 359u, 4097u, 4098u, 8192u, 65536u, 0x7FFFFFFFu, 0x80000000u, 0xFFFFFFFFu};
  static const uint64_t nulllen[] = {1u, 100u, 4299u, 16384u, 16385u, 0x100000000ull, 0xFFFFFFFFFFFFFFFFull};
  uint64_t raw[600]; /* 4800 bytes: room for the largest struct_size the call accepts */
  uint64_t pre[600];
  uint8_t *snap = r_heap_copy(r_base, r_base_n);
  ivs_status st;
  uint32_t k;
  int ok = 1;
  if (snap == NULL) {
    return 0;
  }
  /* a NULL report */
  st = ivs_snapshot_describe(snap, r_base_n, NULL);
  if (st != IVS_E_BAD_ARG) {
    ivs_test_diag("NULL report: status %s (want IVS_E_BAD_ARG)", ivs_status_name(st));
    ok = 0;
  }
  st = ivs_snapshot_describe(NULL, 100u, NULL);
  if (st != IVS_E_BAD_ARG) {
    ivs_test_diag("NULL report and NULL buffer: status %s (want IVS_E_BAD_ARG)", ivs_status_name(st));
    ok = 0;
  }
  /* an unusable struct_size: BAD_ARG with nothing written, whatever else is wrong with the call */
  for (k = 0u; k < sizeof bad_size / sizeof bad_size[0]; k++) {
    uint32_t pass;
    for (pass = 0u; pass < 3u; pass++) {
      const uint8_t *b = pass == 0u ? snap : NULL;
      uint64_t len = pass == 0u ? r_base_n : (pass == 1u ? 100u : 0x100000000ull);
      memset(raw, 0xA5, sizeof raw);
      memcpy(raw, &bad_size[k], sizeof bad_size[k]);
      memcpy(pre, raw, sizeof pre);
      st = ivs_snapshot_describe(b, len, (ivs_snapshot_report *)(void *)raw);
      if (st != IVS_E_BAD_ARG || memcmp(raw, pre, sizeof raw) != 0) {
        ivs_test_diag("struct_size %u (case %u): status %s, report %s", (unsigned)bad_size[k], (unsigned)pass, ivs_status_name(st),
                      memcmp(raw, pre, sizeof raw) == 0 ? "untouched" : "WRITTEN");
        ok = 0;
      }
    }
  }
  /* a NULL buffer with a length above 0: reason 58, before the length bound (so also for a length above it) */
  for (k = 0u; k < sizeof nulllen / sizeof nulllen[0]; k++) {
    char name[48];
    (void)snprintf(name, sizeof name, "NULL buffer, length %llx", (unsigned long long)nulllen[k]);
    ok = r_call_check(name, NULL, nulllen[k], 0u, NULL, 58u, 0u, R_NONE, produced) && ok;
  }
  /* length 0 with a NULL buffer is a truncation (no byte is supplied), not an argument error; a non-NULL buffer of length 0 too */
  ok = r_call_check("NULL buffer, length 0", NULL, 0u, 0u, NULL, 61u, 0u, R_NONE, produced) && ok;
  ok = r_call_check("buffer of one byte, length 0", snap, 0u, 0u, snap, 61u, 0u, R_NONE, produced) && ok;
  /* the report overlaps the buffer (59): inside it, across its first and last byte, and just outside (accepted) */
  ok = r_overlap_row("report at the buffer start", 0u, 0u, r_base_n, 59u, 0u, produced) && ok;
  ok = r_overlap_row("report inside the buffer", 2048u, 0u, r_base_n, 59u, 0u, produced) && ok;
  ok = r_overlap_row("report across the buffer end", 4296u, 0u, r_base_n, 59u, 0u, produced) && ok;
  ok = r_overlap_row("buffer starts on the last report byte", 0u, 359u, r_base_n, 59u, 0u, produced) && ok;
  ok = r_overlap_row("buffer starts right after the report", 0u, 360u, r_base_n, 0u, 0u, produced) && ok;
  ok = r_overlap_row("report starts right after the buffer", 4304u, 0u, r_base_n, 0u, 0u, produced) && ok;
  /* a length above the bound counts as the bound for the overlap rule: the report just inside the first 16,384 bytes overlaps; just past them
   * it does not, and the length is then refused as oversized */
  ok = r_overlap_row("report inside the bound, length 20000", 16376u, 0u, 20000u, 59u, 0u, produced) && ok;
  ok = r_overlap_row("report past the bound, length 20000", 16384u, 0u, 20000u, 60u, R_MAX, produced) && ok;
  ok = r_overlap_row("report past a buffer of exactly 16384 bytes", 16384u, 0u, R_MAX, 67u, R_O_TOTAL, produced) && ok;
  free(snap);
  return ok;
}

/* ---- the cases ---- */

/* The live values of the snapshot of x, against the report of describe: versions and total from the literals, the profile id, the parameter
 * digest from its literal, the composite and the slot identities from the live instance. */
static int r_check_live(ivs_ms_inst *x, const char *label, uint32_t units)
{
  uint8_t snap[IVS_SNAPSHOT_MAX_BYTES];
  uint8_t prefix[24];
  uint8_t params[32];
  uint8_t comp[32];
  uint8_t id[64];
  uint8_t *copy;
  r_rep rep;
  ivs_info info;
  uint64_t w = 0u;
  uint32_t total_lit = 0u;
  uint32_t i;
  int ok = 1;
  ST_REQ(ok, ivsn_lit_hex("header-prefix-hex", prefix, sizeof prefix));
  ST_REQ(ok, ivsn_lit_hex("params-digest-sha256", params, sizeof params));
  ST_REQ(ok, ivsn_lit_u32("total-bytes", &total_lit));
  if (!ok || (units > 0u && !ivs_ms_step(x->m, units, NULL))) {
    return 0;
  }
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  ST_REQ(ok, ivs_machine_snapshot_save(x->m, snap, sizeof snap, &w) == IVS_OK && w == total_lit);
  ST_REQ(ok, ivs_machine_info(x->m, &info) == IVS_OK);
  ST_REQ(ok, ivs_machine_state_digest(x->m, IVS_DIGEST_COMPOSITE, comp) == IVS_OK);
  if (!ok) {
    return 0;
  }
  memset(id, 0, sizeof id);
  memcpy(id, IVS_MS_PROFILE, strlen(IVS_MS_PROFILE));
  copy = r_heap_copy(snap, (size_t)w);
  if (copy == NULL) {
    return 0;
  }
  memset(rep.b, 0xA5, sizeof rep.b);
  rep.r.struct_size = (uint32_t)R_REP;
  ST_REQ(ok, ivs_snapshot_describe(copy, w, &rep.r) == IVS_OK);
  if (!ok) {
    ivs_test_diag("%s: describe did not accept the snapshot of the writer", label);
  }
  ST_REQ(ok, rep.r.struct_size == R_REP && rep.r.reason == 0u);
  ST_REQ(ok, rep.r.format_version == r_be16(prefix + R_O_FORMAT) && rep.r.format_version == 1u);
  ST_REQ(ok, rep.r.schema_version == r_be32(prefix + R_O_SCHEMA) && rep.r.schema_version == 1u);
  ST_REQ(ok, rep.r.total_bytes == total_lit && rep.r.byte_offset == 0u && rep.r.section_index == R_NONE && rep.r.kind == 0u);
  ST_REQ(ok, memcmp(rep.r.profile_id, id, sizeof id) == 0);
  ST_REQ(ok, memcmp(rep.r.profile_params_sha256, params, sizeof params) == 0);
  ST_REQ(ok, memcmp(rep.r.composite_sha256, comp, sizeof comp) == 0);
  for (i = 0u; i < 3u; i++) {
    const ivs_image_report *s = &rep.r.slots[i];
    ST_REQ(ok, s->struct_size == sizeof(ivs_image_report) && s->kind == i + 1u && s->kind == info.slots[i].kind);
    ST_REQ(ok, s->loaded == 1u && s->loaded == info.slots[i].loaded);
    ST_REQ(ok, s->length_bytes == info.slots[i].length_bytes && s->length_bytes != 0u);
    ST_REQ(ok, memcmp(s->sha256, info.slots[i].sha256, 32u) == 0);
    ST_REQ(ok, s->length_units == 0u && s->unit_bits == 0u && s->base_addr == 0u && s->reserved == 0u);
  }
  for (i = (uint32_t)R_REP; i < (uint32_t)sizeof rep.b; i++) {
    ST_REQ(ok, rep.b[i] == 0xA5u);
  }
  ST_REQ(ok, memcmp(copy, snap, (size_t)w) == 0);
  /* a second call is the same */
  {
    r_rep again;
    memset(again.b, 0xA5, sizeof again.b);
    again.r.struct_size = (uint32_t)R_REP;
    ST_REQ(ok, ivs_snapshot_describe(copy, w, &again.r) == IVS_OK && memcmp(again.b, rep.b, sizeof rep.b) == 0);
  }
  free(copy);
  return ok;
}

/* state.read.describe-ok (Task 1, D-14, D-16): a snapshot the writer made is accepted, and the report says what the specification and the live
 * instance say (the versions and total from the hand-derived literals, the profile id, the parameter digest from its literal, the composite
 * digest and the three slot identities from the live instance, every other member of a slot record 0), for a reset instance, a stepped
 * one and the second content; the section lengths the reader derives from the table (no instance) equal the literals, the live counting
 * pass of the writer and sum to the total; the rows that change only fields describe does not judge are accepted and echoed. */
static int r_describe_ok(void)
{
  static const uint32_t units[3] = {0u, 3333u, 777u};
  ivs_ms_inst x;
  r_tab t;
  uint32_t produced = 0u;
  uint32_t total_lit = 0u;
  uint64_t sum = 0u;
  uint32_t i;
  int ok = 1;
  ST_CHECK(r_base_get());
  ST_CHECK(ivsn_lit_u32("total-bytes", &total_lit));
  for (i = 0u; i < 3u; i++) {
    int up = i == 2u ? ivs_ms_powered(&x, IVS_MS_CONTENT_B, 0x5Au) : ivs_ms_powered(&x, IVS_MS_CONTENT_A, 0x5Au);
    ST_CHECK(up);
    ST_REQ(ok, r_check_live(&x, i == 0u ? "reset, content A" : (i == 1u ? "stepped, content A" : "stepped, content B"), units[i]));
    if (i == 0u) {
      /* the reader's own derivation of the lengths, instance-free, against the literals and the counting pass of the writer */
      uint32_t k;
      for (k = 0u; k < ST_SECTIONS; k++) {
        char key[48];
        uint32_t lit = 0u;
        uint32_t d = ivs_snapshot_section_derived_length(k);
        (void)snprintf(key, sizeof key, "section-bytes.%s", st_section_name[k]);
        ST_REQ(ok, ivsn_lit_u32(key, &lit));
        ST_REQ(ok, d == lit);
        ST_REQ(ok, ivs_machine_state_preimage(x.m, st_section_domain[k], NULL, 0u) == d);
        ST_REQ(ok, ivs_snap_domain[k] == st_section_domain[k]);
        sum += d;
      }
      ST_REQ(ok, ivs_snapshot_section_derived_length(ST_SECTIONS) == 0u && ivs_snapshot_section_derived_length(0xFFFFFFFFu) == 0u);
      ST_REQ(ok, ivs_snapshot_derived_total() == total_lit && sum + 612u + 32u == total_lit);
    }
    ivs_ms_free(&x);
  }
  ST_CHECK(r_tab_init(&t, 16u));
  r_build_ok(&t);
  ST_REQ(ok, !t.bad);
  ST_REQ(ok, r_run_rows(&t, &produced) == 0u);
  ST_REQ(ok, produced == 0u);
  r_tab_free(&t);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* state.read.describe-refuse-args (Task 1, D-14): the report that cannot be used (NULL, struct_size below the v1 size or above the maximum)
 * is BAD_ARG with nothing written; a NULL buffer with a length above 0 is reason 58 (also for a length above the bound); the report
 * overlapping the buffer is reason 59, judged over at most the first 16,384 bytes of the buffer; length 0 with a NULL buffer is the
 * truncated class. Each row states the exact reason, the report footprint and that the supplied bytes are unchanged. */
static int r_describe_refuse_args(void)
{
  uint32_t produced = 0u;
  ST_CHECK(r_base_get());
  if (!r_args(&produced)) {
    return IVS_TEST_FAIL;
  }
  ST_CHECK((produced & 0x7u) == 0x7u); /* the bits of the reasons 58, 59 and 60 */
  return IVS_TEST_PASS;
}

/* Builds and runs one family of rows; 1 when every row held and the table was not overfull. */
static int r_family(void (*build)(r_tab *), size_t cap, uint32_t *produced)
{
  r_tab t;
  int ok;
  if (!r_base_get() || !r_tab_init(&t, cap)) {
    return 0;
  }
  build(&t);
  ok = !t.bad && t.n > 0u;
  if (t.bad) {
    ivs_test_diag("the row table is full (%u rows) or a row has too many patches", (unsigned)t.cap);
  }
  ok = r_run_rows(&t, produced) == 0u && ok;
  r_tab_free(&t);
  return ok;
}

/* state.read.describe-refuse-length (Task 1, D-13, D-16): a length above the bound is OVERSIZED before any byte is read (a one-byte real
 * buffer under the sanitizer; lengths that wrap a 32-bit value included); every length below the smallest container, and every cut of the
 * snapshot, is truncated and reports the length as the offset; a total above the supplied length is truncated, a total below it or not the
 * size this build derives is the total-length reason. The buffers of a fault build carry a zeroed tail, so the fault that trusts a claimed
 * length fails by its status. */
static int r_describe_refuse_length(void)
{
  uint32_t produced = 0u;
  return r_family(r_build_length, 96u, &produced) ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* state.read.describe-refuse-header (Task 1, D-13, D-16): each magic byte flipped is reason 62 with the offset of the first difference;
 * format version 0, 2, 0xFFFF and other values, each flag bit, a flipped schema version and schema digest are UNSUPPORTED with their own
 * reasons and the versions reported; the section count 0, 6, 8 and 0xFFFFFFFF is reason 68; and where a snapshot has several defects the
 * first class of the order of the specification wins. */
static int r_describe_refuse_header(void)
{
  uint32_t produced = 0u;
  return r_family(r_build_header, 160u, &produced) ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* state.read.describe-refuse-trailer (Task 1, D-13, D-16): every one of the 32 trailer bytes flipped (and inverted), a byte of every region
 * that only the trailer covers, is DIGEST_MISMATCH 70 at the trailer; a structural defect with a stale trailer reports the structure, and
 * the form of the profile id is judged after the trailer, as the specification orders them. */
static int r_describe_refuse_trailer(void)
{
  uint32_t produced = 0u;
  return r_family(r_build_trailer, 140u, &produced) ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- the families as one list (the reasons-covered case and the judge run them all) ---- */

typedef struct r_fam {
  void (*build)(r_tab *);
  size_t cap;
} r_fam;

static const r_fam r_families[] = {
    {r_build_header, 160u}, {r_build_length, 96u}, {r_build_trailer, 140u}, {r_build_table, 160u}, {r_build_id, 96u}, {r_build_ok, 16u},
};
#define R_FAMILIES (sizeof r_families / sizeof r_families[0])

/* The bit of a reason in a mask of the reasons 58 to 71. */
#define R_BIT(reason) (1u << ((reason)-58u))
#define R_ALL 0x3FFFu /* the fourteen reasons of this plan */

/* The reasons that the rows of r_args expect (58, 59, 60 and the truncation 61 of a zero length): the argument rows are code, not table
 * rows, so their expectation is stated here. */
#define R_ARGS_EXPECT (R_BIT(58u) | R_BIT(59u) | R_BIT(60u) | R_BIT(61u))

/* ---- state.read.describe-refuse-section-table ---- */

/* The typed numbers of the section table rows against the literals of the oracle and against the table of the base snapshot. */
static int r_table_matches_spec(void)
{
  uint32_t i;
  int ok = 1;
  for (i = 0u; i < 7u; i++) {
    char key[48];
    uint32_t lo = 0u;
    uint32_t ln = 0u;
    const uint8_t *e = r_base + R_ENTRY(i);
    (void)snprintf(key, sizeof key, "section-offset.%s", st_section_name[i]);
    ST_REQ(ok, ivsn_lit_u32(key, &lo));
    (void)snprintf(key, sizeof key, "section-bytes.%s", st_section_name[i]);
    ST_REQ(ok, ivsn_lit_u32(key, &ln));
    ST_REQ(ok, r_soff[i] == lo && r_slen[i] == ln && r_dom[i] == st_section_domain[i]);
    ST_REQ(ok, e[0] == r_dom[i] && e[1] == 0u && e[2] == 0u && e[3] == 0u);
    ST_REQ(ok, r_be32(e + 4u) == r_soff[i] && r_be32(e + 8u) == r_slen[i]);
    ST_REQ(ok, i == 6u || r_soff[i] + r_slen[i] == r_soff[i + 1u]);
  }
  ST_REQ(ok, r_soff[6] + r_slen[6] == r_base_n - 32u);
  return ok;
}

/* state.read.describe-refuse-section-table (Task 2, D-13, D-16): the section table is refused with reason 69 and the index of the entry
 * (and the offset of the field) for an unknown, repeated, descending or skipped domain, a nonzero byte among the three zero bytes, a first
 * offset of 611 or 613, a gap, an overlap, a length one more or one less than derived, offsets and lengths near 2^32 whose 32-bit sum
 * wraps, and a last section that ends one byte before or after the trailer. Every refusal row carries a valid trailer (resealed), so it
 * is the table that is refused and never the trailer; the rows that change only what describe does not judge are accepted. */
static int r_describe_refuse_section_table(void)
{
  uint32_t produced = 0u;
  ST_CHECK(r_base_get());
  ST_CHECK(r_table_matches_spec());
  if (!r_family(r_build_table, 160u, &produced)) {
    return IVS_TEST_FAIL;
  }
  ST_CHECK((produced & R_BIT(69u)) != 0u);
  return IVS_TEST_PASS;
}

/* state.read.describe-profile-id-forms (Task 2, D-13, D-16): the form of the 64 byte profile id field, judged after the trailer (the rows
 * are resealed): an embedded NUL followed by non-zero bytes, no NUL in the 64 bytes, a non-ASCII byte, a control byte and a byte after
 * the NUL are reason 71 at the first offending byte (the last byte of the field when there is no NUL); an empty id, 63 printable bytes,
 * the printable edge characters and an unregistered but well formed id are accepted, because only restore consults the registry. */
static int r_describe_profile_id_forms(void)
{
  uint32_t produced = 0u;
  ST_CHECK(r_base_get());
  if (!r_family(r_build_id, 96u, &produced)) {
    return IVS_TEST_FAIL;
  }
  ST_CHECK((produced & R_BIT(71u)) != 0u);
  return IVS_TEST_PASS;
}

/* ---- state.read.describe-heap-exact ---- */

/* state.read.describe-heap-exact (Task 2, D-13, D-16, T-08-30): the snapshot cut to every length from 0 to the total plus one is described
 * from a heap buffer of exactly that many bytes, so the sanitizer of the dev preset finds any read past the supplied length. The result is
 * pinned by length class: below 644 and up to the total minus one it is truncated with the length as the offset (the total the bytes state
 * is above the length), the total itself is accepted, and one byte more is the total-length reason (the total stated is below the length).
 * A fault run takes every 16th length and always the lengths around the smallest container and the last four. */
static int r_describe_heap_exact(void)
{
  static uint8_t work[IVS_SNAPSHOT_MAX_BYTES];
  uint32_t produced = 0u;
  uint32_t bad = 0u;
  size_t n;
  size_t len;
  ST_CHECK(r_base_get());
  n = r_base_n;
  memset(work, 0, sizeof work);
  memcpy(work, r_base, n);
  for (len = 0u; len <= n + 1u && bad < 8u; len++) {
    uint8_t *h;
    char name[32];
    uint32_t reason = 61u;
    uint64_t off = len;
    if (ST_FAULT_STRIDE > 1u && len % ST_FAULT_STRIDE != 0u && (len < 640u || len > 650u) && len + 4u < n) {
      continue;
    }
    if (len == n) {
      reason = 0u;
      off = 0u;
    } else if (len == n + 1u) {
      reason = 67u;
      off = R_O_TOTAL;
    }
    h = r_heap_copy(work, len);
    if (h == NULL) {
      return IVS_TEST_FAIL;
    }
    (void)snprintf(name, sizeof name, "length %u", (unsigned)len);
    if (!r_call_check(name, h, len, len, work, reason, off, R_NONE, &produced)) {
      bad++;
    }
    free(h);
  }
  return bad == 0u ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- state.read.describe-writes-only-report ---- */

/* The bytes of a 400 byte report that a call wrote, found by running it into two reports that carry different poison bytes: a byte was
 * written when it differs from what the report held before the call in either run (a written byte that happens to keep its value, as the
 * high bytes of struct_size do, cannot be seen this way, so struct_size is returned as well). written[i] is 1 for a written byte. Returns 0 when the
 * two runs gave different statuses (then the footprint is meaningless) and sets *st to the status. */
static int r_footprint(const uint8_t *buf, uint64_t claimed, uint8_t written[400], ivs_status *st, uint32_t ssz[2])
{
  static const uint8_t poison[2] = {0xA5u, 0x5Au};
  r_rep rep[2];
  uint8_t pre[2][sizeof rep[0].b];
  ivs_status s[2];
  size_t i;
  uint32_t k;
  for (k = 0u; k < 2u; k++) {
    memset(rep[k].b, poison[k], sizeof rep[k].b);
    rep[k].r.struct_size = (uint32_t)sizeof rep[k].b;
    memcpy(pre[k], rep[k].b, sizeof pre[k]);
    s[k] = ivs_snapshot_describe(buf, claimed, &rep[k].r);
  }
  for (i = 0u; i < 400u; i++) {
    written[i] = (uint8_t)(rep[0].b[i] != pre[0][i] || rep[1].b[i] != pre[1][i]);
  }
  ssz[0] = rep[0].r.struct_size;
  ssz[1] = rep[1].r.struct_size;
  *st = s[0];
  return s[0] == s[1];
}

/* The bytes the contract says a call writes: a refusal writes reason (4 to 7), byte_offset (24 to 31), section_index (32 to 35) and the
 * header facts the supplied length let it read, format_version (8 to 11) from 12 bytes, total_bytes (16 to 23) from 16 and schema_version
 * (12 to 15) from 24; a success writes the 360 bytes of the v1 report. Nothing else, and not struct_size of a refusal. */
static void r_contract_footprint(uint8_t want[400], size_t readable, int success)
{
  size_t i;
  memset(want, 0, 400u);
  if (success) {
    memset(want, 1, R_REP);
    return;
  }
  for (i = 4u; i < 8u; i++) {
    want[i] = 1u;
  }
  for (i = 24u; i < 36u; i++) {
    want[i] = 1u;
  }
  if (readable >= 12u) {
    for (i = 8u; i < 12u; i++) {
      want[i] = 1u;
    }
  }
  if (readable >= 16u) {
    for (i = 16u; i < 24u; i++) {
      want[i] = 1u;
    }
  }
  if (readable >= 24u) {
    for (i = 12u; i < 16u; i++) {
      want[i] = 1u;
    }
  }
}

/* 1 when the footprint of the call is the one of the contract; a diagnostic names the first byte that differs. */
static int r_footprint_check(const char *name, const uint8_t *buf, uint64_t claimed, size_t readable, int success)
{
  uint8_t got[400];
  uint8_t want[400];
  ivs_status st = IVS_OK;
  uint32_t ssz[2] = {0u, 0u};
  size_t i;
  if (!r_footprint(buf, claimed, got, &st, ssz)) {
    ivs_test_diag("row '%s': two calls on the same bytes gave different statuses", name);
    return 0;
  }
  if ((st == IVS_OK) != (success != 0)) {
    ivs_test_diag("row '%s': status %s does not match the expected %s", name, ivs_status_name(st), success ? "success" : "refusal");
    return 0;
  }
  r_contract_footprint(want, readable, success);
  /* struct_size (bytes 0 to 3): a success sets it to the v1 size, a refusal keeps the caller's 400 */
  if (ssz[0] != (success ? (uint32_t)R_REP : 400u) || ssz[1] != ssz[0]) {
    ivs_test_diag("row '%s': struct_size is %u after the call, expected %u", name, (unsigned)ssz[0], success ? (unsigned)R_REP : 400u);
    return 0;
  }
  for (i = 4u; i < 400u; i++) {
    if (got[i] != want[i]) {
      ivs_test_diag("row '%s': report byte %u is %s, the contract says it is %s", name, (unsigned)i, got[i] ? "written" : "untouched",
                    want[i] ? "written" : "untouched");
      return 0;
    }
  }
  return 1;
}

/* state.read.describe-writes-only-report (Task 2, D-14, D-16): for every row of every family (and a NULL buffer), the bytes of the report
 * that the call writes are exactly those of the contract of ivs/ivs_state.h, found by running the call into two reports poisoned
 * differently: a refusal writes reason, byte_offset, section_index and the header facts that the supplied length let it read and not
 * struct_size or any other member; a success writes the 360 bytes of the v1 report in full; and no run writes a byte past the v1 size of
 * a larger caller struct. The bytes of the buffer are compared by every row of the other cases. */
static int r_describe_writes_only_report(void)
{
  static uint8_t work[IVS_SNAPSHOT_MAX_BYTES + 64u];
  uint32_t bad = 0u;
  size_t f;
  size_t i;
  ST_CHECK(r_base_get());
  for (f = 0u; f < R_FAMILIES; f++) {
    r_tab t;
    ST_CHECK(r_tab_init(&t, r_families[f].cap));
    r_families[f].build(&t);
    for (i = 0u; i < t.n && bad < 8u; i++) {
      const r_row *w = &t.row[i];
      uint64_t claimed = 0u;
      size_t readable = 0u;
      uint8_t *h = NULL;
      r_row_make(w, work, &h, &claimed, &readable);
      if (h == NULL) {
        r_tab_free(&t);
        return IVS_TEST_FAIL;
      }
      if (!r_footprint_check(w->name, h, claimed, readable, w->reason == 0u)) {
        bad++;
      }
      free(h);
    }
    r_tab_free(&t);
  }
  if (!r_footprint_check("NULL buffer, length 7", NULL, 7u, 0u, 0)) {
    bad++;
  }
  return bad == 0u ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- state.read.describe-report-v1-size-caller ---- */

/* The contract of the report struct: an object of exactly the v1 size (360 bytes, from the heap) is filled by a success and by a refusal
 * without a write past it; a larger caller struct (struct_size 361 to 4096) gets exactly the v1 bytes of a success with struct_size set to 360
 * and keeps every byte past them, and a refusal writes only its fields and keeps struct_size; a struct_size below the v1 size (every
 * value from 0 to 359) or above the maximum is BAD_ARG with the whole object untouched. */
static int r_describe_report_v1_size_caller(void)
{
  static const uint32_t good[] = {360u, 361u, 367u, 368u, 369u, 400u, 1000u, 4095u, 4096u};
  static const uint32_t bad_big[] = {4097u, 4098u, 4104u, 8192u, 65536u, 0x7FFFFFFFu, 0x80000000u, 0xFFFFFFFFu};
  uint64_t obj[600];
  uint64_t pre[600];
  uint8_t flipped[IVS_SNAPSHOT_MAX_BYTES];
  uint8_t *snap;
  uint8_t *bad_snap;
  uint8_t *rep;
  uint8_t exp[sizeof obj];
  uint32_t k;
  uint32_t sz;
  int ok = 1;
  ST_CHECK(r_base_get());
  memcpy(flipped, r_base, r_base_n);
  flipped[r_base_n - 1u] ^= 0x01u;
  snap = r_heap_copy(r_base, r_base_n);
  bad_snap = r_heap_copy(flipped, r_base_n);
  if (snap == NULL || bad_snap == NULL) {
    free(snap);
    free(bad_snap);
    return IVS_TEST_FAIL;
  }
  /* an object of exactly the v1 size: a success and a refusal stay inside it (the sanitizer of the dev preset finds a write past it) */
  rep = (uint8_t *)malloc(R_REP);
  if (rep == NULL) {
    free(snap);
    free(bad_snap);
    return IVS_TEST_FAIL;
  }
  for (k = 0u; k < 2u; k++) {
    uint8_t want[R_REP];
    memset(rep, 0xA5, R_REP);
    sz = (uint32_t)R_REP;
    memcpy(rep, &sz, sizeof sz);
    memcpy(want, rep, R_REP);
    r_expect_report(want, k == 0u ? r_base : flipped, r_base_n, k == 0u ? 0u : 70u, r_base_n - 32u, R_NONE);
    ST_REQ(ok, ivs_snapshot_describe(k == 0u ? snap : bad_snap, r_base_n, (ivs_snapshot_report *)(void *)rep) ==
                   (k == 0u ? IVS_OK : IVS_E_DIGEST_MISMATCH));
    ST_REQ(ok, memcmp(rep, want, R_REP) == 0);
  }
  free(rep);
  /* larger caller structs: the tail is kept, and a success sets struct_size to the v1 size, a refusal keeps the caller's */
  for (k = 0u; k < sizeof good / sizeof good[0]; k++) {
    uint32_t pass;
    for (pass = 0u; pass < 2u; pass++) {
      memset(obj, 0xA5, sizeof obj);
      memcpy(obj, &good[k], sizeof good[k]);
      memcpy(pre, obj, sizeof pre);
      memcpy(exp, obj, sizeof exp);
      r_expect_report(exp, pass == 0u ? r_base : flipped, r_base_n, pass == 0u ? 0u : 70u, r_base_n - 32u, R_NONE);
      ST_REQ(ok, ivs_snapshot_describe(pass == 0u ? snap : bad_snap, r_base_n, (ivs_snapshot_report *)(void *)obj) ==
                     (pass == 0u ? IVS_OK : IVS_E_DIGEST_MISMATCH));
      if (memcmp(obj, exp, sizeof exp) != 0) {
        ivs_test_diag("struct_size %u (%s): the object differs from the contract", (unsigned)good[k], pass == 0u ? "success" : "refusal");
        ok = 0;
      }
    }
  }
  /* an unusable struct_size: every value below the v1 size and a few above the maximum; nothing is written, whatever the snapshot is */
  for (sz = 0u; sz < (uint32_t)R_REP + (uint32_t)(sizeof bad_big / sizeof bad_big[0]); sz++) {
    uint32_t pass;
    for (pass = 0u; pass < 2u; pass++) {
      uint32_t v = sz < (uint32_t)R_REP ? sz : bad_big[sz - (uint32_t)R_REP];
      memset(obj, 0xA5, sizeof obj);
      memcpy(obj, &v, sizeof v);
      memcpy(pre, obj, sizeof pre);
      if (ivs_snapshot_describe(pass == 0u ? snap : bad_snap, r_base_n, (ivs_snapshot_report *)(void *)obj) != IVS_E_BAD_ARG ||
          memcmp(obj, pre, sizeof obj) != 0) {
        ivs_test_diag("struct_size %u: not refused BAD_ARG with nothing written", (unsigned)v);
        ok = 0;
      }
    }
  }
  free(snap);
  free(bad_snap);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- state.read.reasons-covered ---- */

/* Runs every family and the argument rows; *produced gets the bits of the reasons some row produced (the call returned them and the row
 * held), *expected the bits of the reasons some row expects, and the return value is the number of rows that failed. The rows per reason
 * are written to the diagnostics (stdout only) for the record. */
static uint32_t r_collect_all(uint32_t *produced, uint32_t *expected, uint32_t per_reason[14])
{
  uint32_t bad = 0u;
  size_t f;
  size_t i;
  uint32_t k;
  *produced = 0u;
  *expected = R_ARGS_EXPECT;
  memset(r_seen, 0, sizeof r_seen);
  for (f = 0u; f < R_FAMILIES; f++) {
    r_tab t;
    if (!r_tab_init(&t, r_families[f].cap)) {
      return 1000u;
    }
    r_families[f].build(&t);
    if (t.bad) {
      bad++;
    }
    for (i = 0u; i < t.n; i++) {
      uint32_t r = t.row[i].reason;
      if (r >= 58u && r <= 71u) {
        *expected |= R_BIT(r);
      }
    }
    bad += r_run_rows(&t, produced);
    r_tab_free(&t);
  }
  if (!r_args(produced)) {
    bad++;
  }
  for (k = 0u; k < 14u; k++) {
    per_reason[k] = r_seen[k];
  }
  return bad;
}

/* state.read.reasons-covered (Task 2, D-16): every one of the fourteen reasons 58 to 71 is expected by some row of the matrix and produced
 * by the call for some row, so no reason is dead; and the statuses of the table of the specification map each reason to one status (the
 * five statuses of the plan: BAD_ARG, OVERSIZED, MALFORMED, UNSUPPORTED, DIGEST_MISMATCH). */
static int r_reasons_covered(void)
{
  uint32_t produced = 0u;
  uint32_t expected = 0u;
  uint32_t per[14];
  uint32_t bad;
  uint32_t k;
  int ok = 1;
  ST_CHECK(r_base_get());
  bad = r_collect_all(&produced, &expected, per);
  ST_REQ(ok, bad == 0u);
  for (k = 0u; k < 14u; k++) {
    ivs_test_diag("reason %u: %u rows", (unsigned)(58u + k), (unsigned)per[k]);
    if ((produced & (1u << k)) == 0u) {
      ivs_test_diag("reason %u is produced by no row", (unsigned)(58u + k));
      ok = 0;
    }
    if ((expected & (1u << k)) == 0u) {
      ivs_test_diag("reason %u is expected by no row", (unsigned)(58u + k));
      ok = 0;
    }
  }
  ST_REQ(ok, produced == R_ALL && expected == R_ALL);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- state.judge.matrix-expectation-corrupt ---- */

/* state.judge.matrix-expectation-corrupt (Task 2, D-16): the matrix runner can fail. Each family table is copied and one expectation of
 * the copy is corrupted in turn (the reason, the byte offset, the section index, a success turned into a refusal, a refusal turned into a
 * success); the runner must report exactly that one row, while the uncorrupted copy reports none. */
static int r_judge_matrix(void)
{
  size_t f;
  int ok = 1;
  ST_CHECK(r_base_get());
  for (f = 0u; f < R_FAMILIES; f++) {
    r_tab t;
    r_tab c;
    uint32_t produced = 0u;
    uint32_t corrupt;
    size_t first_refusal = 0u;
    size_t last_refusal = 0u;
    size_t first_ok = 0u;
    size_t i;
    int have_ok = 0;
    int have_ref = 0;
    ST_CHECK(r_tab_init(&t, r_families[f].cap));
    r_families[f].build(&t);
    for (i = 0u; i < t.n; i++) {
      if (t.row[i].reason == 0u && !have_ok) {
        first_ok = i;
        have_ok = 1;
      }
      if (t.row[i].reason != 0u && !have_ref) {
        first_refusal = i;
        have_ref = 1;
      }
      if (t.row[i].reason != 0u) {
        last_refusal = i;
      }
    }
    for (corrupt = 0u; corrupt < 6u; corrupt++) {
      size_t at = corrupt == 5u ? last_refusal : (corrupt == 4u ? first_ok : first_refusal);
      r_row *row;
      if ((corrupt == 4u && !have_ok) || (corrupt != 4u && !have_ref)) {
        continue; /* the family has no such row */
      }
      if (!r_tab_init(&c, t.cap)) {
        r_tab_free(&t);
        return IVS_TEST_FAIL;
      }
      memcpy(c.row, t.row, t.n * sizeof *t.row);
      c.n = t.n;
      row = &c.row[at];
      switch (corrupt) {
      case 0u: /* the uncorrupted copy */
        break;
      case 1u: /* another reason that travels with another status */
        row->reason = row->reason == 62u ? 63u : 62u;
        break;
      case 2u: /* the byte offset one off */
        row->off += 1u;
        break;
      case 3u: /* the section index changed */
        row->sec = row->sec == R_NONE ? 0u : R_NONE;
        break;
      case 4u: /* a success that the row now says is refused */
        row->reason = 62u;
        row->off = 0u;
        break;
      default: /* the last refusal row, its offset changed */
        row->off ^= 0x10u;
        break;
      }
      {
        uint32_t failed;
        uint32_t want = corrupt == 0u ? 0u : 1u;
        r_quiet = 1; /* the corrupted rows fail on purpose */
        failed = r_run_rows(&c, &produced);
        r_quiet = 0;
        if (failed != want) {
          ivs_test_diag("family %u, corruption %u: the runner reported %u failing rows, expected %u", (unsigned)f, (unsigned)corrupt,
                        (unsigned)failed, (unsigned)want);
          ok = 0;
        }
      }
      r_tab_free(&c);
    }
    r_tab_free(&t);
  }
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_case ivs_state_read_cases[] = {
    {"state.read.describe-ok", r_describe_ok, NULL},
    {"state.read.describe-refuse-args", r_describe_refuse_args, NULL},
    {"state.read.describe-refuse-length", r_describe_refuse_length, NULL},
    {"state.read.describe-refuse-header", r_describe_refuse_header, NULL},
    {"state.read.describe-refuse-trailer", r_describe_refuse_trailer, NULL},
    {"state.read.describe-refuse-section-table", r_describe_refuse_section_table, NULL},
    {"state.read.describe-profile-id-forms", r_describe_profile_id_forms, NULL},
    {"state.read.describe-heap-exact", r_describe_heap_exact, NULL},
    {"state.read.describe-writes-only-report", r_describe_writes_only_report, NULL},
    {"state.read.describe-report-v1-size-caller", r_describe_report_v1_size_caller, NULL},
    {"state.read.reasons-covered", r_reasons_covered, NULL},
    {"state.judge.matrix-expectation-corrupt", r_judge_matrix, NULL},
};
const size_t ivs_state_read_count = sizeof ivs_state_read_cases / sizeof ivs_state_read_cases[0];
