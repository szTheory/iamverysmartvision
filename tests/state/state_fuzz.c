/* SPDX-License-Identifier: MIT */
/* Suite state, family fuzz (plan 08-15, D-16, D-20, D-41, docs/formats/ivsn-v1.md "Fuzzing"): a seeded, deterministic family of hostile
 * snapshots. The input is one valid snapshot of about 4.3 KB (a Lamp Tone instance with one input event applied, stopped at a unit boundary
 * in the middle of its third frame, so that counters, RAM, the latched card rows, the pull-downs and the observation rows are not default),
 * so the sweeps are exhaustive and cheap instead of sampled. Nothing here reads a host source of randomness or time: the one generator is
 * an in-repo xorshift with a fixed seed (the random family of plan 08-15 task 3).
 *
 * Every case runs through one checker (fz_check) that enforces the invariants of 08-DISCUSS-state 6.1 on the call it makes:
 *   - the status is one of the documented set and matches its reason by the table of the specification (su_status_of);
 *   - the outcome is one the case's expectation allows: the position classifier (fz_classify) says, for a single flipped byte, which reason
 *     the first failing check must report with the trailer unresealed and which set of outcomes a resealed flip may have; it is written from
 *     the layout table and the check order of the specification, never from the reader or the restore code;
 *   - a refusal leaves the WHOLE target instance (every one of its bytes) equal to the baseline copy and writes only the report: every byte
 *     of a 400 byte caller struct not named by the contract keeps its poison (the poison alternates between 0xA5 and 0x5A from case to
 *     case, and a case may run twice, once with each, to show two attempts agree);
 *   - an acceptance writes no byte outside the STATE and PAD rows of the digest field table, zeroes every PAD row, leaves the state-only
 *     preimage of each of the seven domains equal to the input's section (no payload byte is ignored: a restore never silently drops a
 *     byte), writes the report exactly as ivs_snapshot_describe does, and on a bounded subset (the first 16 accepted cases of an id and
 *     every 97th) a full save of the restored instance equals the input bytes except the eight informational digest fields (the composite
 *     and the seven section digests) and the trailer, which a re-save recomputes (the one named exception of D-41, taken from the input and
 *     resealed before the comparison);
 *   - the target is put back from the baseline copy after every accepted case.
 * The checker has been shown able to fail (state.judge.fuzz-invariant-detects). Under IVS_STATE_FAULT_RUN (plan 08-11, D-40, D-37) every id
 * runs a reduced point set (every 16th offset or row), never a different check; the full set runs in the normal builds. */
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/ivs_sha256.h"
#include "ivs/ivs.h"
#include "machine/machine.h"
#include "state_cases.h"
#include "state_util.h"

#define FZ_NONE 0xFFFFFFFFu
#define FZ_BIT(reason) (1u << ((reason)-58u))
#define FZ_HDR IVS_SNAP_HEADER_BYTES /* 612 */
#define FZ_MAXROWS 64u
#define FZ_MAX_RANGES 256u
#define FZ_SOURCE_TICKS 156400u /* the machine time the source stops at (the first unit boundary at or after it): frame 2, cycle 9,229 */

/* ---- the classes a flipped byte can fall in (the rows of the report of an id) ---- */
#define FZ_C_MAGIC 0u
#define FZ_C_FORMAT 1u
#define FZ_C_FLAGS 2u
#define FZ_C_TOTAL 3u
#define FZ_C_SCHEMAVER 4u
#define FZ_C_COUNT 5u
#define FZ_C_SCHEMADIG 6u
#define FZ_C_ID 7u
#define FZ_C_PARAMS 8u
#define FZ_C_SLOTFLAG 9u
#define FZ_C_SLOTSHA 10u
#define FZ_C_SLOTLEN 11u
#define FZ_C_COMPOSITE 12u
#define FZ_C_TABLE 13u
#define FZ_C_TABLEDIG 14u
#define FZ_C_FRAME 15u
#define FZ_C_COUNTPFX 16u
#define FZ_C_VALUE 17u
#define FZ_C_TRAILER 18u
#define FZ_NCLASS 19u
static const char *const fz_class_name[FZ_NCLASS] = {"magic",      "format",      "flags",     "total",      "schema-version", "section-count",
                                                     "schema-digest", "profile-id", "params",    "slot-loaded", "slot-sha",       "slot-length",
                                                     "composite",  "table",       "table-digest", "frame",     "row-count",       "value",
                                                     "trailer"};

/* ---- the expectation of a case ---- */
#define FZ_X_OFF 1u  /* the reported byte offset is exact */
#define FZ_X_SEC 2u  /* the reported section index is exact */
#define FZ_X_KIND 4u /* the reported slot kind is exact (0: none reported) */
typedef struct fz_exp {
  uint32_t reasons; /* bit (reason - 58) of every refusal reason the case may end in */
  uint8_t accept;   /* an acceptance is allowed */
  uint8_t xmask;    /* FZ_X_*: what of (off, sec, kind) is checked, when the outcome is a refusal */
  uint64_t off;
  uint32_t sec;
  uint32_t kind;
} fz_exp;

static void fz_exp_one(fz_exp *e, uint32_t reason, uint8_t xmask, uint64_t off, uint32_t sec, uint32_t kind)
{
  e->reasons = FZ_BIT(reason);
  e->accept = 0u;
  e->xmask = xmask;
  e->off = off;
  e->sec = sec;
  e->kind = kind;
}

/* ---- the fixture: the valid snapshot, the target and its baseline, the layout of the payload ---- */
typedef struct fz_range {
  uint32_t off;
  uint32_t len;
} fz_range;

#define FZ_K_FRAME 1u
#define FZ_K_COUNT 2u
#define FZ_K_VALUE 3u

typedef struct fz_fixture {
  int state; /* 0 not built, 1 ready, -1 the build failed */
  uint8_t snap[IVS_SNAPSHOT_MAX_BYTES];
  size_t n;
  ivs_ms_inst tgt;
  uint8_t baseline[IVS_MACHINE_BYTES]; /* the target before any case: STATE and PAD rows poisoned */
  uint8_t after[IVS_MACHINE_BYTES];    /* the target after the first attempt of a case that runs twice */
  fz_range other[FZ_MAX_RANGES];       /* the runs of instance bytes that no STATE or PAD row covers */
  size_t nother;
  fz_range pad[FZ_MAX_RANGES]; /* the runs of PAD rows */
  size_t npad;
  uint32_t state_byte; /* the offset of one STATE byte, for the self-control */
  uint32_t sec_off[ST_SECTIONS];
  uint32_t sec_len[ST_SECTIONS];
  uint8_t pk_kind[IVS_SNAPSHOT_MAX_BYTES]; /* per container offset of the payload: FZ_K_* */
  uint8_t pk_row[IVS_SNAPSHOT_MAX_BYTES];  /* the row index of a count or value byte */
  size_t nrows;
  const char *row_name[FZ_MAXROWS];
  uint8_t row_free[FZ_MAXROWS];
  uint32_t row_sec[FZ_MAXROWS];
  uint32_t row_count_off[FZ_MAXROWS]; /* the container offset of the count prefix of an array row, else 0 */
} fz_fixture;

static fz_fixture fz;

/* The rows the specification calls free values ("Derived invariants": counters, chains, RAM words and the latched card rows): no validator
 * and no invariant constrains them, so the trailer is the only check and a resealed flip in them is always accepted. */
static const char *const fz_free_rows[] = {"cpu.r",        "mem.sysram",          "mem.scratchpad",    "mem.gram",
                                           "stic.row_latch", "stic.blocked_reads", "stic.blocked_writes", "stic.late_grants",
                                           "stic.grants_in_frame", "obs.video.chain", "obs.audio.chain",  "obs.input.chain",
                                           "obs.psglog.chain", "obs.audio.count",   "obs.input.count",   "obs.psglog.count",
                                           NULL};

static int fz_is_free(const char *row)
{
  size_t i;
  for (i = 0u; fz_free_rows[i] != NULL; i++) {
    if (strcmp(fz_free_rows[i], row) == 0) {
      return 1;
    }
  }
  return 0;
}

static void fz_add_range(fz_range *r, size_t *n, uint32_t off, uint32_t len)
{
  if (*n > 0u && r[*n - 1u].off + r[*n - 1u].len == off) {
    r[*n - 1u].len += len;
    return;
  }
  if (*n < FZ_MAX_RANGES) {
    r[*n].off = off;
    r[*n].len = len;
    (*n)++;
  }
}

static int fz_layout(void)
{
  static uint8_t cover[IVS_MACHINE_BYTES];
  size_t rows = ivs_machine_digest_field_count();
  size_t k;
  uint32_t i;
  uint32_t o;
  memset(cover, 0, sizeof cover);
  for (k = 0u; k < rows; k++) {
    const ivs_digest_field *f = ivs_machine_digest_field(k);
    if (f == NULL) {
      return 0;
    }
    if (f->snap == IVS_SNAP_STATE || f->snap == IVS_SNAP_PAD) {
      uint32_t len = f->width * f->count;
      if (f->offset + len > IVS_MACHINE_BYTES) {
        return 0;
      }
      memset(cover + f->offset, f->snap == IVS_SNAP_STATE ? 1 : 2, len);
      if (strcmp(f->name, "time_ticks") == 0) {
        fz.state_byte = f->offset;
      }
    }
  }
  fz.nother = 0u;
  fz.npad = 0u;
  for (o = 0u; o < IVS_MACHINE_BYTES; o++) {
    if (cover[o] == 0u) {
      fz_add_range(fz.other, &fz.nother, o, 1u);
    } else if (cover[o] == 2u) {
      fz_add_range(fz.pad, &fz.npad, o, 1u);
    }
  }
  /* the payload: per section the frame, then per STATE row in table order the count prefix (array rows) and the values */
  fz.nrows = 0u;
  memset(fz.pk_kind, 0, sizeof fz.pk_kind);
  for (i = 0u; i < ST_SECTIONS; i++) {
    char key[48];
    uint32_t pos;
    uint32_t j;
    (void)snprintf(key, sizeof key, "section-bytes.%s", st_section_name[i]);
    if (!ivsn_lit_u32(key, &fz.sec_len[i])) {
      return 0;
    }
    fz.sec_off[i] = su_section_offset(i);
    pos = fz.sec_off[i];
    if (pos == 0u || pos + fz.sec_len[i] > IVS_SNAPSHOT_MAX_BYTES) {
      return 0;
    }
    for (j = 0u; j < IVS_SNAP_FRAME_BYTES; j++) {
      fz.pk_kind[pos + j] = FZ_K_FRAME;
    }
    pos += IVS_SNAP_FRAME_BYTES;
    for (k = 0u; k < rows; k++) {
      const ivs_digest_field *f = ivs_machine_digest_field(k);
      uint32_t row;
      uint32_t vlen;
      if (f->domain != st_section_domain[i] || f->snap != IVS_SNAP_STATE || f->kind != IVS_DIGEST_KIND_UINT) {
        continue;
      }
      if (fz.nrows >= FZ_MAXROWS) {
        return 0;
      }
      row = (uint32_t)fz.nrows++;
      fz.row_name[row] = f->name;
      fz.row_free[row] = (uint8_t)fz_is_free(f->name);
      fz.row_sec[row] = i;
      fz.row_count_off[row] = 0u;
      if (f->count > 1u) {
        fz.row_count_off[row] = pos;
        for (j = 0u; j < 4u; j++) {
          fz.pk_kind[pos + j] = FZ_K_COUNT;
          fz.pk_row[pos + j] = (uint8_t)row;
        }
        pos += 4u;
      }
      vlen = f->width * f->count;
      for (j = 0u; j < vlen; j++) {
        fz.pk_kind[pos + j] = FZ_K_VALUE;
        fz.pk_row[pos + j] = (uint8_t)row;
      }
      pos += vlen;
    }
    if (pos != fz.sec_off[i] + fz.sec_len[i]) {
      ivs_test_diag("the walk of section %u ends at %u, the literals say %u", (unsigned)i, (unsigned)pos, (unsigned)(fz.sec_off[i] + fz.sec_len[i]));
      return 0;
    }
  }
  return fz.nother > 0u && fz.npad > 0u && fz.state_byte != 0u;
}

static int fz_get(void)
{
  ivs_ms_inst src;
  uint32_t total = 0u;
  uint32_t consumed = 0u;
  uint32_t nev = 0u;
  const ivs_input_event *ev;
  int ok;
  if (fz.state != 0) {
    return fz.state > 0;
  }
  fz.state = -1;
  if (!ivsn_lit_u32("total-bytes", &total)) {
    return 0;
  }
  ev = su_scenario_events(SU_SC_LAMP_PASS, &nev);
  if (!su_scenario_new(&src, SU_SC_LAMP_PASS, 1)) {
    return 0;
  }
  ok = su_run_to(src.m, FZ_SOURCE_TICKS, ev, nev, &consumed) && su_snapshot(src.m, fz.snap, sizeof fz.snap, &fz.n) && fz.n == total;
  ivs_ms_free(&src);
  if (!ok) {
    ivs_test_diag("the valid snapshot could not be built (%u bytes, %u expected)", (unsigned)fz.n, (unsigned)total);
    return 0;
  }
  consumed = 0u;
  if (!su_scenario_new(&fz.tgt, SU_SC_LAMP_PASS, 1) || !su_run_to(fz.tgt.m, 20000u, ev, nev, &consumed)) {
    return 0;
  }
  state_poison(fz.tgt.m);
  memcpy(fz.baseline, fz.tgt.storage, IVS_MACHINE_BYTES);
  if (!fz_layout()) {
    ivs_test_diag("the layout of the payload could not be derived from the field table");
    return 0;
  }
  fz.state = 1;
  return 1;
}

/* ---- the context of one id ---- */
#define FZ_T_NONE 0u
#define FZ_T_ACCEPT 1u   /* a byte of the restored target is changed before the acceptance checks */
#define FZ_T_REFUSAL 2u  /* a byte of the target is changed after a refusal */
#define FZ_T_REPORT 3u   /* a byte of the report is changed after the call */
#define FZ_T_PREIMAGE 4u /* a byte of the re-encoded section is changed before the comparison */

typedef struct fz_ctx {
  const char *name;
  int quiet;
  uint32_t tamper;
  uint64_t attempts;
  uint64_t refused;
  uint64_t accepted;
  uint64_t bad;
  uint32_t seen; /* bit (reason - 58) of every refusal reason observed */
  uint32_t shown;
  int case_bad;
  uint64_t cls_ref[FZ_NCLASS];
  uint64_t cls_acc[FZ_NCLASS];
  uint32_t row_ref[FZ_MAXROWS];
  uint32_t row_acc[FZ_MAXROWS];
  uint32_t full_saves;
} fz_ctx;

static void fz_ctx_init(fz_ctx *c, const char *name)
{
  memset(c, 0, sizeof *c);
  c->name = name;
}

typedef struct fz_lab {
  const char *what;
  uint64_t a;
  uint32_t m;
} fz_lab;

#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 3, 4)))
#endif
static void fz_fail(fz_ctx *c, const fz_lab *lab, const char *fmt, ...)
{
  char msg[200];
  va_list ap;
  c->case_bad = 1;
  if (c->quiet || c->shown >= 8u) {
    return;
  }
  c->shown++;
  va_start(ap, fmt);
  (void)vsnprintf(msg, sizeof msg, fmt, ap);
  va_end(ap);
  ivs_test_diag("%s: %s %llu mask %02X: %s", c->name, lab->what, (unsigned long long)lab->a, (unsigned)lab->m, msg);
}

/* ---- one call of restore ---- */
typedef struct fz_res {
  ivs_status st;
  uint32_t reason; /* 0 for an acceptance */
  uint64_t off;
  uint32_t sec;
  uint32_t kind;
} fz_res;

/* The call into a report poisoned with pbyte, and what the contract says about it. The target is the fixture's. */
static void fz_call(fz_ctx *c, const fz_lab *lab, uint8_t *h, size_t readable, uint64_t claimed, uint8_t pbyte, fz_res *r)
{
  su_rep rep;
  uint8_t exp[SU_CALLER];
  uint32_t pword = (uint32_t)pbyte * 0x01010101u;
  memset(&rep, pbyte, sizeof rep);
  rep.r.struct_size = SU_CALLER;
  memcpy(exp, rep.b, sizeof exp);
  r->st = ivs_machine_snapshot_restore(fz.tgt.m, h, claimed, &rep.r);
  if (c->tamper == FZ_T_REPORT) {
    rep.b[offsetof(ivs_snapshot_report, profile_id)] ^= 0x01u;
  }
  r->reason = 0u;
  r->off = 0u;
  r->sec = FZ_NONE;
  r->kind = 0u;
  if (r->st == IVS_OK) {
    /* an acceptance writes the whole v1 report exactly as describe does */
    su_rep want;
    memset(&want, pbyte, sizeof want);
    want.r.struct_size = SU_CALLER;
    if (ivs_snapshot_describe(h, claimed, &want.r) != IVS_OK || memcmp(rep.b, want.b, sizeof rep.b) != 0) {
      fz_fail(c, lab, "the report of an acceptance is not the report describe writes");
    }
    return;
  }
  r->reason = rep.r.reason;
  if (r->reason < 58u || r->reason > 81u || su_status_of(r->reason) != r->st) {
    fz_fail(c, lab, "status %s with reason %u is no documented pair", ivs_status_name(r->st), (unsigned)r->reason);
    return;
  }
  r->off = rep.r.byte_offset;
  r->sec = rep.r.section_index;
  r->kind = rep.r.kind == pword ? 0u : rep.r.kind;
  if (r->off > IVS_SNAPSHOT_MAX_BYTES || (r->sec != FZ_NONE && r->sec >= ST_SECTIONS) || r->kind > 3u) {
    fz_fail(c, lab, "a reported position out of range: offset %llu, section %u, kind %u", (unsigned long long)r->off, (unsigned)r->sec, (unsigned)r->kind);
  }
  su_expect_report(exp, h, readable, r->reason, r->off, r->sec, r->kind);
  if (memcmp(rep.b, exp, sizeof exp) != 0) {
    size_t i = 0u;
    while (rep.b[i] == exp[i]) {
      i++;
    }
    fz_fail(c, lab, "report byte %u is %02X, the contract says %02X (poison %02X)", (unsigned)i, (unsigned)rep.b[i], (unsigned)exp[i], (unsigned)pbyte);
  }
}

/* The checks of an accepted case, made on the restored target. */
static void fz_accepted_checks(fz_ctx *c, const fz_lab *lab, const uint8_t *h, size_t len)
{
  static uint8_t pre[4096];
  static uint8_t out[IVS_SNAPSHOT_MAX_BYTES];
  size_t i;
  size_t n = 0u;
  if (c->tamper == FZ_T_ACCEPT) {
    fz.tgt.storage[fz.state_byte] ^= 0x01u;
  }
  for (i = 0u; i < fz.nother; i++) {
    if (memcmp(fz.tgt.storage + fz.other[i].off, fz.baseline + fz.other[i].off, fz.other[i].len) != 0) {
      fz_fail(c, lab, "bytes outside every STATE and PAD row changed at %u", (unsigned)fz.other[i].off);
      break;
    }
  }
  for (i = 0u; i < fz.npad; i++) {
    uint32_t j;
    for (j = 0u; j < fz.pad[i].len; j++) {
      if (fz.tgt.storage[fz.pad[i].off + j] != 0u) {
        fz_fail(c, lab, "a PAD row is not zero at %u", (unsigned)(fz.pad[i].off + j));
        break;
      }
    }
  }
  for (i = 0u; i < ST_SECTIONS; i++) {
    size_t got = ivs_machine_state_preimage(fz.tgt.m, st_section_domain[i], pre, sizeof pre);
    if (c->tamper == FZ_T_PREIMAGE) {
      pre[0] ^= 0x01u;
    }
    if (got != fz.sec_len[i] || memcmp(pre, h + fz.sec_off[i], got) != 0) {
      fz_fail(c, lab, "the re-encode of section %u (%u bytes) is not the input's section", (unsigned)i, (unsigned)got);
    }
  }
  if (c->accepted <= 16u || c->accepted % 97u == 0u) {
    c->full_saves++;
    if (!su_snapshot(fz.tgt.m, out, sizeof out, &n) || n != len) {
      fz_fail(c, lab, "a full save of the restored instance gives %u bytes, not %u", (unsigned)n, (unsigned)len);
    } else {
      memcpy(out + IVS_SNAP_OFF_COMPOSITE, h + IVS_SNAP_OFF_COMPOSITE, 32u);
      for (i = 0u; i < ST_SECTIONS; i++) {
        size_t at = IVS_SNAP_OFF_TABLE + i * IVS_SNAP_ENTRY_BYTES + 12u;
        memcpy(out + at, h + at, 32u);
      }
      su_reseal(out, n);
      if (memcmp(out, h, len) != 0) {
        size_t d = 0u;
        while (out[d] == h[d]) {
          d++;
        }
        fz_fail(c, lab, "a full save of the restored instance differs from the input at byte %u", (unsigned)d);
      }
    }
  }
}

/* One case. The bytes are copied to a heap buffer of exactly readable bytes (a fault build adds a zeroed tail, su_heap_copy), restore is
 * told claimed (the supplied length unless the case lies about it), and the outcome is checked against the expectation. psel picks the poison
 * of the report; with twice the case runs a second time with the other poison on an identically reset target and the two must agree.
 * cls and row (or -1) say where the byte lies, for the report of the id. Returns 1 when the case held. */
static int fz_check(fz_ctx *c, const char *what, uint64_t a, uint32_t m, const uint8_t *bytes, size_t readable, uint64_t claimed,
                    const fz_exp *e, int cls, int row, uint32_t psel, int twice)
{
  fz_lab lab;
  fz_res r[2];
  uint8_t *h;
  uint32_t k;
  uint32_t attempts = twice ? 2u : 1u;
  lab.what = what;
  lab.a = a;
  lab.m = m;
  c->case_bad = 0;
  c->attempts++;
  h = su_heap_copy(bytes, readable);
  if (h == NULL) {
    fz_fail(c, &lab, "out of memory");
    c->bad++;
    return 0;
  }
  for (k = 0u; k < attempts; k++) {
    uint8_t pbyte = ((psel ^ k) & 1u) != 0u ? 0x5Au : 0xA5u;
    fz_call(c, &lab, h, readable, claimed, pbyte, &r[k]);
    if (readable > 0u && memcmp(h, bytes, readable) != 0) {
      fz_fail(c, &lab, "the supplied bytes were changed");
      memcpy(h, bytes, readable);
    }
    if (r[k].st == IVS_OK) {
      if (k == 0u) {
        c->accepted++;
        if (cls >= 0) {
          c->cls_acc[cls]++;
        }
        if (row >= 0) {
          c->row_acc[row]++;
        }
        if (!e->accept) {
          fz_fail(c, &lab, "accepted, but the specification says it is refused");
        }
      }
      fz_accepted_checks(c, &lab, h, readable);
      if (attempts == 2u && k == 0u) {
        memcpy(fz.after, fz.tgt.storage, IVS_MACHINE_BYTES);
      }
      if (attempts == 2u && k == 1u && memcmp(fz.after, fz.tgt.storage, IVS_MACHINE_BYTES) != 0) {
        fz_fail(c, &lab, "the second attempt left another instance than the first");
      }
      memcpy(fz.tgt.storage, fz.baseline, IVS_MACHINE_BYTES);
    } else {
      if (k == 0u) {
        c->refused++;
        if (r[k].reason >= 58u && r[k].reason <= 81u) {
          c->seen |= FZ_BIT(r[k].reason);
        }
        if (cls >= 0) {
          c->cls_ref[cls]++;
        }
        if (row >= 0) {
          c->row_ref[row]++;
        }
        if (r[k].reason >= 58u && r[k].reason <= 81u && (e->reasons & FZ_BIT(r[k].reason)) == 0u) {
          fz_fail(c, &lab, "refused with reason %u, which the specification does not allow here (allowed %06X, accept %u)", (unsigned)r[k].reason,
                  (unsigned)e->reasons, (unsigned)e->accept);
        } else if (r[k].reason >= 58u && r[k].reason <= 81u) {
          if ((e->xmask & FZ_X_OFF) != 0u && r[k].off != e->off) {
            fz_fail(c, &lab, "reason %u at offset %llu, the specification says %llu", (unsigned)r[k].reason, (unsigned long long)r[k].off,
                    (unsigned long long)e->off);
          }
          if ((e->xmask & FZ_X_SEC) != 0u && r[k].sec != e->sec) {
            fz_fail(c, &lab, "reason %u in section %u, the specification says %u", (unsigned)r[k].reason, (unsigned)r[k].sec, (unsigned)e->sec);
          }
          if ((e->xmask & FZ_X_KIND) != 0u && r[k].kind != e->kind) {
            fz_fail(c, &lab, "reason %u with slot kind %u, the specification says %u", (unsigned)r[k].reason, (unsigned)r[k].kind, (unsigned)e->kind);
          }
        }
      }
      if (c->tamper == FZ_T_REFUSAL) {
        fz.tgt.storage[0] ^= 0x01u;
      }
      if (memcmp(fz.tgt.storage, fz.baseline, IVS_MACHINE_BYTES) != 0) {
        fz_fail(c, &lab, "a refusal changed the instance");
        memcpy(fz.tgt.storage, fz.baseline, IVS_MACHINE_BYTES);
      }
    }
  }
  if (attempts == 2u) {
    if (r[0].st != r[1].st || r[0].reason != r[1].reason || r[0].off != r[1].off || r[0].sec != r[1].sec || r[0].kind != r[1].kind) {
      fz_fail(c, &lab, "two attempts differ: %s/%u/%llu against %s/%u/%llu", ivs_status_name(r[0].st), (unsigned)r[0].reason,
              (unsigned long long)r[0].off, ivs_status_name(r[1].st), (unsigned)r[1].reason, (unsigned long long)r[1].off);
    }
  }
  free(h);
  if (c->case_bad) {
    c->bad++;
  }
  return !c->case_bad;
}

/* The line of an id: attempts, outcomes and, per class, the refused and accepted counts. Returns the verdict of the case. */
static int fz_finish(const fz_ctx *c)
{
  char line[400];
  size_t at = 0u;
  uint32_t i;
  ivs_test_diag("%s: %llu attempts, %llu refused, %llu accepted, %llu failed; %u full saves", c->name, (unsigned long long)c->attempts,
                (unsigned long long)c->refused, (unsigned long long)c->accepted, (unsigned long long)c->bad, (unsigned)c->full_saves);
  for (i = 0u; i < FZ_NCLASS; i++) {
    if (c->cls_ref[i] + c->cls_acc[i] > 0u && at + 48u < sizeof line) {
      int w = snprintf(line + at, sizeof line - at, "%s %llu/%llu; ", fz_class_name[i], (unsigned long long)c->cls_ref[i],
                       (unsigned long long)c->cls_acc[i]);
      if (w > 0) {
        at += (size_t)w;
      }
    }
  }
  if (at > 0u) {
    ivs_test_diag("%s: refused/accepted by class: %s", c->name, line);
  }
  return c->bad == 0u && c->attempts > 0u ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- the position classifier (written from the Layout table and the Check order of the specification) ---- */

/* The profile id field after one byte of it changed: the reason of its form rule and the offset of the first offending byte, or 0 when the form
 * is good (the id is then looked up: it is not registered, because it differs from the one registered id, 72). */
static uint32_t fz_id_form(uint32_t idx, uint8_t mask, uint32_t *off)
{
  uint8_t id[64];
  uint32_t nul = 64u;
  uint32_t j;
  memcpy(id, fz.snap + IVS_SNAP_OFF_PROFILE_ID, sizeof id);
  id[idx] = (uint8_t)(id[idx] ^ mask);
  for (j = 0u; j < 64u; j++) {
    if (id[j] == 0u) {
      nul = j;
      break;
    }
  }
  if (nul == 64u) {
    *off = IVS_SNAP_OFF_PROFILE_ID + 63u;
    return 71u;
  }
  for (j = 0u; j < 64u; j++) {
    if ((j < nul && (id[j] < 0x20u || id[j] > 0x7Eu)) || (j > nul && id[j] != 0u)) {
      *off = IVS_SNAP_OFF_PROFILE_ID + j;
      return 71u;
    }
  }
  return 0u;
}

/* What a single flipped byte at offset k (the byte XOR mask) must end in, with the trailer left as it was (sealed 0) or recomputed (sealed 1);
 * returns the class of the byte. The reasons are those of the check order: magic 62, format 63, flags 64, schema version 65, schema digest 66,
 * total 61 or 67, section count 68, section table 69, trailer 70, profile id form 71, unknown profile 72, parameter digest 74, slot records 76
 * and 78, row counts 77, frames and values 78 and 79. The total says 61 when the flipped total is above the length and 67 when it is below. */
static uint32_t fz_classify(uint32_t k, uint8_t mask, int sealed, fz_exp *e)
{
  const uint32_t n = (uint32_t)fz.n;
  const uint32_t trailer = n - IVS_SNAP_TRAILER_BYTES;
  const uint8_t xall = FZ_X_OFF | FZ_X_SEC | FZ_X_KIND;
  memset(e, 0, sizeof *e);
  if (k < 8u) {
    fz_exp_one(e, 62u, xall, k, FZ_NONE, 0u);
    return FZ_C_MAGIC;
  }
  if (k < 10u) {
    fz_exp_one(e, 63u, xall, IVS_SNAP_OFF_FORMAT, FZ_NONE, 0u);
    return FZ_C_FORMAT;
  }
  if (k < 12u) {
    fz_exp_one(e, 64u, xall, IVS_SNAP_OFF_FLAGS, FZ_NONE, 0u);
    return FZ_C_FLAGS;
  }
  if (k < 16u) {
    uint32_t total = n ^ ((uint32_t)mask << (8u * (15u - k)));
    if (total > n) {
      fz_exp_one(e, 61u, xall, n, FZ_NONE, 0u);
    } else {
      fz_exp_one(e, 67u, xall, IVS_SNAP_OFF_TOTAL, FZ_NONE, 0u);
    }
    return FZ_C_TOTAL;
  }
  if (k < 20u) {
    fz_exp_one(e, 65u, xall, IVS_SNAP_OFF_SCHEMA, FZ_NONE, 0u);
    return FZ_C_SCHEMAVER;
  }
  if (k < 24u) {
    fz_exp_one(e, 68u, xall, IVS_SNAP_OFF_COUNT, FZ_NONE, 0u);
    return FZ_C_COUNT;
  }
  if (k < IVS_SNAP_OFF_PROFILE_ID) {
    fz_exp_one(e, 66u, xall, IVS_SNAP_OFF_SCHEMA_DIGEST, FZ_NONE, 0u);
    return FZ_C_SCHEMADIG;
  }
  if (k < IVS_SNAP_OFF_PARAMS) {
    if (!sealed) {
      fz_exp_one(e, 70u, xall, trailer, FZ_NONE, 0u);
    } else {
      uint32_t off = 0u;
      uint32_t reason = fz_id_form(k - IVS_SNAP_OFF_PROFILE_ID, mask, &off);
      if (reason == 71u) {
        fz_exp_one(e, 71u, xall, off, FZ_NONE, 0u);
      } else {
        fz_exp_one(e, 72u, xall, IVS_SNAP_OFF_PROFILE_ID, FZ_NONE, 0u);
      }
    }
    return FZ_C_ID;
  }
  if (k < IVS_SNAP_OFF_SLOTS) {
    if (!sealed) {
      fz_exp_one(e, 70u, xall, trailer, FZ_NONE, 0u);
    } else {
      fz_exp_one(e, 74u, xall, IVS_SNAP_OFF_PARAMS, FZ_NONE, 0u);
    }
    return FZ_C_PARAMS;
  }
  if (k < IVS_SNAP_OFF_COMPOSITE) {
    uint32_t s = (k - IVS_SNAP_OFF_SLOTS) / IVS_SNAP_SLOT_BYTES;
    uint32_t r = (k - IVS_SNAP_OFF_SLOTS) % IVS_SNAP_SLOT_BYTES;
    uint32_t rec = IVS_SNAP_OFF_SLOTS + s * IVS_SNAP_SLOT_BYTES;
    if (!sealed) {
      fz_exp_one(e, 70u, xall, trailer, FZ_NONE, 0u);
    } else if (r < 4u) {
      /* the loaded flag: 1 becomes 0 (a record that holds a length, reported at the length) or a value above 1 (reported at the flag) */
      uint32_t flag = 1u ^ ((uint32_t)mask << (8u * (3u - r)));
      fz_exp_one(e, 78u, xall, flag == 0u ? rec + 36u : rec, FZ_NONE, s + 1u);
    } else if (r < 36u) {
      fz_exp_one(e, 76u, xall, rec + 4u, FZ_NONE, s + 1u);
    } else {
      fz_exp_one(e, 76u, xall, rec + 36u, FZ_NONE, s + 1u);
    }
    return r < 4u ? FZ_C_SLOTFLAG : (r < 36u ? FZ_C_SLOTSHA : FZ_C_SLOTLEN);
  }
  if (k < IVS_SNAP_OFF_TABLE) {
    if (!sealed) {
      fz_exp_one(e, 70u, xall, trailer, FZ_NONE, 0u);
    } else {
      e->accept = 1u; /* an informational digest (D-41) */
    }
    return FZ_C_COMPOSITE;
  }
  if (k < FZ_HDR) {
    uint32_t r = (k - IVS_SNAP_OFF_TABLE) % IVS_SNAP_ENTRY_BYTES;
    if (r < 12u) {
      fz_exp_one(e, 69u, 0u, 0u, FZ_NONE, 0u); /* which entry and field the reader names is for the matrix of plan 08-12 */
      return FZ_C_TABLE;
    }
    if (!sealed) {
      fz_exp_one(e, 70u, xall, trailer, FZ_NONE, 0u);
    } else {
      e->accept = 1u;
    }
    return FZ_C_TABLEDIG;
  }
  if (k < trailer) {
    if (!sealed) {
      fz_exp_one(e, 70u, xall, trailer, FZ_NONE, 0u);
      return fz.pk_kind[k] == FZ_K_FRAME ? FZ_C_FRAME : (fz.pk_kind[k] == FZ_K_COUNT ? FZ_C_COUNTPFX : FZ_C_VALUE);
    }
    if (fz.pk_kind[k] == FZ_K_FRAME) {
      uint32_t i = 0u;
      while (i < ST_SECTIONS - 1u && k >= fz.sec_off[i + 1u]) {
        i++;
      }
      fz_exp_one(e, 78u, xall, k, i, 0u);
      return FZ_C_FRAME;
    }
    if (fz.pk_kind[k] == FZ_K_COUNT) {
      uint32_t row = fz.pk_row[k];
      fz_exp_one(e, 77u, xall, fz.row_count_off[row], fz.row_sec[row], 0u);
      return FZ_C_COUNTPFX;
    }
    if (fz.row_free[fz.pk_row[k]]) {
      e->accept = 1u;
    } else {
      e->accept = 1u;
      e->reasons = FZ_BIT(78u) | FZ_BIT(79u);
    }
    return FZ_C_VALUE;
  }
  /* the trailer: a flipped trailer is refused; with the trailer recomputed the flip is repaired and the input is the valid snapshot */
  if (!sealed) {
    fz_exp_one(e, 70u, xall, trailer, FZ_NONE, 0u);
  } else {
    e->accept = 1u;
  }
  return FZ_C_TRAILER;
}

/* ---- the sweeps ---- */

/* The point set of a sweep: every one in a normal build, every 16th in a fault run. */
static int fz_keep(uint32_t i)
{
  return ST_FAULT_STRIDE == 1u || i % ST_FAULT_STRIDE == 0u;
}

/* state.fuzz.truncations: every length from 0 to the size minus one, through restore on a heap buffer of exactly that length. The check order
 * gives a defined refusal for each: below the smallest container and above it alike the reader says the snapshot is truncated (61) with the
 * supplied length as the offset, because the total the bytes state is above the length. */
static int fz_truncations(void)
{
  fz_ctx c;
  size_t len;
  int ok;
  ST_CHECK(fz_get());
  fz_ctx_init(&c, "state.fuzz.truncations");
  for (len = 0u; len < fz.n; len++) {
    fz_exp e;
    if (!fz_keep((uint32_t)len)) {
      continue;
    }
    fz_exp_one(&e, 61u, FZ_X_OFF | FZ_X_SEC | FZ_X_KIND, len, FZ_NONE, 0u);
    (void)fz_check(&c, "length", len, 0u, fz.snap, len, len, &e, -1, -1, (uint32_t)len, 0);
  }
  ok = fz_finish(&c);
  if (c.accepted != 0u || c.seen != FZ_BIT(61u)) {
    ivs_test_diag("%s: a truncation was accepted or ended in another reason (seen %06X)", c.name, (unsigned)c.seen);
    ok = IVS_TEST_FAIL;
  }
  return ok;
}

/* A flip of one byte of the valid snapshot, resealed or not, checked against the classifier. */
static void fz_flip_case(fz_ctx *c, uint32_t k, uint8_t mask, int sealed, uint32_t psel, int twice)
{
  static uint8_t work[IVS_SNAPSHOT_MAX_BYTES];
  fz_exp e;
  uint32_t cls;
  int row = -1;
  memcpy(work, fz.snap, fz.n);
  work[k] = (uint8_t)(work[k] ^ mask);
  if (sealed) {
    su_reseal(work, fz.n);
  }
  cls = fz_classify(k, mask, sealed, &e);
  if (k >= FZ_HDR && k < fz.n - IVS_SNAP_TRAILER_BYTES && fz.pk_kind[k] == FZ_K_VALUE) {
    row = (int)fz.pk_row[k];
  }
  (void)fz_check(c, sealed ? "resealed byte" : "byte", k, mask, work, fz.n, fz.n, &e, (int)cls, row, psel, twice);
}

/* state.fuzz.byte-flips: every offset flipped (XOR 0xFF and XOR 0x01) with the trailer NOT resealed is refused, with the reason of the first
 * check that fails: the header fields by their own reasons, everything the structure does not look at by the trailer (70). */
static int fz_byte_flips(void)
{
  static const uint8_t masks[2] = {0xFFu, 0x01u};
  fz_ctx c;
  uint32_t k;
  int ok;
  ST_CHECK(fz_get());
  fz_ctx_init(&c, "state.fuzz.byte-flips");
  for (k = 0u; k < (uint32_t)fz.n; k++) {
    uint32_t j;
    if (!fz_keep(k)) {
      continue;
    }
    for (j = 0u; j < 2u; j++) {
      fz_flip_case(&c, k, masks[j], 0, k + j, 0);
    }
  }
  ok = fz_finish(&c);
  if (c.accepted != 0u) {
    ivs_test_diag("%s: a flip with the trailer left as it was was accepted", c.name);
    ok = IVS_TEST_FAIL;
  }
  return ok;
}

const ivs_test_case ivs_state_fuzz_cases[] = {
    {"state.fuzz.truncations", fz_truncations, NULL},
    {"state.fuzz.byte-flips", fz_byte_flips, NULL},
};
const size_t ivs_state_fuzz_count = sizeof ivs_state_fuzz_cases / sizeof ivs_state_fuzz_cases[0];
