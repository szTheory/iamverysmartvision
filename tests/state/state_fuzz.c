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
  uint8_t row_known[FZ_MAXROWS];
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

/* The rows no validator or invariant constrains although the specification does not call them free: a known acceptance, pinned here so that a
 * later validator is a conscious change. The STIC register words are stored masked by the write path of the memory system, so a word with a bit
 * outside its write mask is a state no step makes; nothing refuses it today (plan 08-15, summary: "Accepted by design, and the one known
 * acceptance"). */
static const char *const fz_known_rows[] = {"mem.stic_reg", NULL};

static int fz_is_known(const char *row)
{
  size_t i;
  for (i = 0u; fz_known_rows[i] != NULL; i++) {
    if (strcmp(fz_known_rows[i], row) == 0) {
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
      fz.row_known[row] = (uint8_t)fz_is_known(f->name);
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
static void fz_call(fz_ctx *c, const fz_lab *lab, uint8_t *h, size_t readable, uint64_t claimed, uint8_t pbyte, int subset, fz_res *r)
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
    /* an acceptance writes the whole v1 report exactly as describe does (describe hashes the snapshot again, so the comparison is made on the
     * bounded subset of the accepted cases) */
    if (subset) {
      su_rep want;
      memset(&want, pbyte, sizeof want);
      want.r.struct_size = SU_CALLER;
      if (ivs_snapshot_describe(h, claimed, &want.r) != IVS_OK || memcmp(rep.b, want.b, sizeof rep.b) != 0) {
        fz_fail(c, lab, "the report of an acceptance is not the report describe writes");
      }
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
  /* an oversized length is refused before any byte is read, so no header fact is written either */
  su_expect_report(exp, h, r->reason == 60u ? 0u : readable, r->reason, r->off, r->sec, r->kind);
  if (memcmp(rep.b, exp, sizeof exp) != 0) {
    size_t i = 0u;
    while (rep.b[i] == exp[i]) {
      i++;
    }
    fz_fail(c, lab, "report byte %u is %02X, the contract says %02X (poison %02X)", (unsigned)i, (unsigned)rep.b[i], (unsigned)exp[i], (unsigned)pbyte);
  }
}

/* The checks of an accepted case, made on the restored target. The instance outside the STATE and PAD rows, the PAD rows and the re-encode of every
 * section that differs from the valid snapshot's are checked on every accepted case; on the bounded subset (the first 16 accepted cases of an id and
 * every 97th, `subset`) all seven sections, a full save, the report against describe and a short run of the restored machine are checked as well. */
static void fz_accepted_checks(fz_ctx *c, const fz_lab *lab, const uint8_t *h, size_t len, int subset)
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
    size_t got;
    if (!subset && len == fz.n && memcmp(h + fz.sec_off[i], fz.snap + fz.sec_off[i], fz.sec_len[i]) == 0) {
      continue; /* a section equal to the valid snapshot's: every accepted case of the subset re-encodes all seven */
    }
    got = ivs_machine_state_preimage(fz.tgt.m, st_section_domain[i], pre, sizeof pre);
    if (c->tamper == FZ_T_PREIMAGE) {
      pre[0] ^= 0x01u;
    }
    if (got != fz.sec_len[i] || memcmp(pre, h + fz.sec_off[i], got) != 0) {
      fz_fail(c, lab, "the re-encode of section %u (%u bytes) is not the input's section", (unsigned)i, (unsigned)got);
    }
  }
  if (subset) {
    uint32_t consumed = 0u;
    const struct ivs_machine *im = fz.tgt.m;
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
    /* a restored machine runs: a state that restore accepted must not make the core fail (the sanitizer sees any bad access) */
    if (im->time_ticks < ((uint64_t)1u << 62) && !su_run_to(fz.tgt.m, im->time_ticks + 6000u, NULL, 0u, &consumed)) {
      fz_fail(c, lab, "the restored machine refuses to run");
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
  int subset = c->accepted + 1u <= 16u || (c->accepted + 1u) % 97u == 0u;
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
    fz_call(c, &lab, h, readable, claimed, pbyte, subset, &r[k]);
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
      fz_accepted_checks(c, &lab, h, readable, subset);
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

/* A sweep must have met every reason it exists to meet, and some acceptance when it expects any (a normal build; a fault run takes a thinned
 * point set). Returns IVS_TEST_PASS or IVS_TEST_FAIL. */
static int fz_seen(const fz_ctx *c, uint32_t want, int expect_accept)
{
  int ok = IVS_TEST_PASS;
  if (ST_FAULT_STRIDE == 1u && (c->seen & want) != want) {
    ivs_test_diag("%s: reasons never produced: %06X", c->name, (unsigned)(want & ~c->seen));
    ok = IVS_TEST_FAIL;
  }
  if (ST_FAULT_STRIDE == 1u && expect_accept && c->accepted == 0u) {
    ivs_test_diag("%s: nothing was accepted, the sweep is vacuous", c->name);
    ok = IVS_TEST_FAIL;
  }
  return ok;
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
    if (fz.row_free[fz.pk_row[k]] || fz.row_known[fz.pk_row[k]]) {
      e->accept = 1u; /* no validator or invariant constrains the row: any value is accepted */
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


/* state.fuzz.byte-flips-resealed: every offset flipped (XOR 0xFF and XOR 0x01) with the trailer RECOMPUTED, so the flip is judged by the
 * checks after the trailer: the header fields by their own reasons as before, the profile id by its form (71) or the registry (72), the
 * parameter digest (74), the slot records (76, 78), the informational digests (accepted), the frame of a section (78), a count prefix (77) and
 * the values (accepted, or 78 or 79). A flip of the trailer is repaired by the reseal and is the valid snapshot. An accepted case must
 * re-encode canonically (fz_accepted_checks) and, for a row the specification calls free, every flip must be accepted. */
static void fz_print_rows(const fz_ctx *c)
{
  char line[360];
  size_t at = 0u;
  size_t i;
  for (i = 0u; i < fz.nrows; i++) {
    if (c->row_ref[i] + c->row_acc[i] == 0u) {
      continue;
    }
    if (at + 64u >= sizeof line) {
      ivs_test_diag("%s: refused/accepted by row: %s", c->name, line);
      at = 0u;
    }
    {
      int w = snprintf(line + at, sizeof line - at, "%s%s %u/%u; ", fz.row_free[i] ? "free " : (fz.row_known[i] ? "known " : ""), fz.row_name[i], (unsigned)c->row_ref[i],
                       (unsigned)c->row_acc[i]);
      if (w > 0) {
        at += (size_t)w;
      }
    }
  }
  if (at > 0u) {
    ivs_test_diag("%s: refused/accepted by row: %s", c->name, line);
  }
}

static int fz_byte_flips_resealed(void)
{
  static const uint8_t masks[2] = {0xFFu, 0x01u};
  fz_ctx c;
  uint32_t k;
  int ok;
  ST_CHECK(fz_get());
  fz_ctx_init(&c, "state.fuzz.byte-flips-resealed");
  for (k = 0u; k < (uint32_t)fz.n; k++) {
    uint32_t j;
    if (!fz_keep(k)) {
      continue;
    }
    for (j = 0u; j < 2u; j++) {
      /* the second value of a byte of a free row adds nothing to the first: XOR 0xFF flips every bit of the byte, so with the valid snapshot's
       * own round trip every bit of it is shown restored as 0 and as 1; the constrained rows, the frames, the counts and the header take both
       * (plan 08-15, cost stop rule) */
      if (j == 1u && k >= FZ_HDR && k < (uint32_t)fz.n - IVS_SNAP_TRAILER_BYTES && fz.pk_kind[k] == FZ_K_VALUE && fz.row_free[fz.pk_row[k]]) {
        continue;
      }
      fz_flip_case(&c, k, masks[j], 1, k + j, 0);
    }
  }
  fz_print_rows(&c);
  ok = fz_finish(&c);
  ok |= fz_seen(&c, FZ_BIT(61u) | FZ_BIT(62u) | FZ_BIT(63u) | FZ_BIT(64u) | FZ_BIT(65u) | FZ_BIT(66u) | FZ_BIT(67u) | FZ_BIT(68u) | FZ_BIT(69u) |
                        FZ_BIT(71u) | FZ_BIT(72u) | FZ_BIT(74u) | FZ_BIT(76u) | FZ_BIT(77u) | FZ_BIT(78u) | FZ_BIT(79u),
                1);
  return ok;
}

/* 1 when byte k of the header lies in a 32 byte digest field. */
static int fz_digest_byte(uint32_t k)
{
  uint32_t s;
  if ((k >= IVS_SNAP_OFF_SCHEMA_DIGEST && k < IVS_SNAP_OFF_PROFILE_ID) || (k >= IVS_SNAP_OFF_PARAMS && k < IVS_SNAP_OFF_SLOTS) ||
      (k >= IVS_SNAP_OFF_COMPOSITE && k < IVS_SNAP_OFF_TABLE)) {
    return 1;
  }
  for (s = 0u; s < 3u; s++) {
    uint32_t rec = IVS_SNAP_OFF_SLOTS + s * IVS_SNAP_SLOT_BYTES;
    if (k >= rec + 4u && k < rec + 36u) {
      return 1;
    }
  }
  return k >= IVS_SNAP_OFF_TABLE && k < FZ_HDR && (k - IVS_SNAP_OFF_TABLE) % IVS_SNAP_ENTRY_BYTES >= 12u;
}

/* state.fuzz.header-bit-flips: every single bit of the 612 byte header that is not a bit of a digest (the digests take one bit per byte), with the
 * trailer left as it was and with it recomputed, through the same classifier. */
static int fz_header_bit_flips(void)
{
  fz_ctx c;
  uint32_t k;
  int ok;
  ST_CHECK(fz_get());
  fz_ctx_init(&c, "state.fuzz.header-bit-flips");
  for (k = 0u; k < FZ_HDR; k++) {
    uint32_t b;
    if (!fz_keep(k)) {
      continue;
    }
    for (b = 0u; b < 8u; b++) {
      /* a byte of a 32 byte digest (the schema digest, the parameter digest, the three slot SHA-256, the composite and the seven section
       * digests) is compared or ignored as a whole, so every bit of it ends alike: one bit per byte, rotating with the offset, so that all
       * eight bit positions occur in every digest (every byte of every digest takes XOR 0xFF and XOR 0x01 in the byte sweeps) */
      if (fz_digest_byte(k) && b != k % 8u) {
        continue;
      }
      fz_flip_case(&c, k, (uint8_t)(1u << b), 0, k + b, 0);
      fz_flip_case(&c, k, (uint8_t)(1u << b), 1, k + b + 1u, 0);
    }
  }
  ok = fz_finish(&c);
  ok |= fz_seen(&c, FZ_BIT(61u) | FZ_BIT(62u) | FZ_BIT(63u) | FZ_BIT(64u) | FZ_BIT(65u) | FZ_BIT(66u) | FZ_BIT(67u) | FZ_BIT(68u) | FZ_BIT(69u) |
                        FZ_BIT(70u) | FZ_BIT(71u) | FZ_BIT(72u) | FZ_BIT(74u) | FZ_BIT(76u) | FZ_BIT(78u),
                1);
  return ok;
}

/* ---- the rows of the structured lies (data, as in the matrices of plan 08-13) ---- */

#define FZ_TAB_CAP 512u
static uint8_t fz_lax[FZ_TAB_CAP]; /* 1: the row's expectation names the reason only (the reader's choice of entry or offset is ambiguous) */

static void fz_lax_row(const su_tab *t, const su_row *r)
{
  size_t i = (size_t)(r - t->row);
  if (i < FZ_TAB_CAP) {
    fz_lax[i] = 1u;
  }
}

/* A table row: one lie, the reason and the exact place the specification gives it. */
static su_row *fz_lie(su_tab *t, uint32_t reason, uint64_t off, uint32_t sec, uint32_t kind, const char *fmt, ...)
{
  char name[80];
  su_row *r;
  va_list ap;
  va_start(ap, fmt);
  (void)vsnprintf(name, sizeof name, fmt, ap);
  va_end(ap);
  r = su_add(t, "%s", name);
  su_expect(r, reason, off, sec, kind);
  return r;
}

/* Runs the rows: each is a copy of the valid snapshot with the row's patches, resealed when the row says so, handed to restore as a heap buffer of
 * its supplied length with the length it claims, and judged against the row's own (status, reason, offset, section, kind). */
static void fz_run_tab(fz_ctx *c, const su_tab *t, uint32_t psel0, int twice)
{
  static uint8_t work[IVS_SNAPSHOT_MAX_BYTES + 64u];
  size_t r;
  if (t->bad) {
    fz_fail(c, &(fz_lab){"a row table overflowed", 0u, 0u}, "raise its capacity");
    c->bad++;
    return;
  }
  for (r = 0u; r < t->n; r++) {
    const su_row *w = &t->row[r];
    fz_exp e;
    size_t i;
    size_t readable;
    uint64_t claimed;
    memset(work, 0, sizeof work);
    memcpy(work, fz.snap, fz.n);
    for (i = 0u; i < w->np; i++) {
      uint8_t *b = &work[w->p[i].off];
      *b = w->p[i].op == 1u ? (uint8_t)(*b ^ w->p[i].v) : w->p[i].v;
    }
    if (w->reseal != 0u) {
      su_reseal(work, fz.n);
    }
    if (w->big_len != 0u) {
      readable = 1u;
      claimed = w->big_len;
    } else {
      readable = w->trunc != 0u ? (size_t)w->trunc : fz.n;
      claimed = readable;
    }
    memset(&e, 0, sizeof e);
    e.reasons = w->reason != 0u ? FZ_BIT(w->reason) : 0u;
    e.accept = (uint8_t)(w->reason == 0u);
    e.xmask = (r < FZ_TAB_CAP && fz_lax[r] != 0u) ? 0u : (uint8_t)(FZ_X_OFF | FZ_X_SEC | FZ_X_KIND);
    e.off = w->off;
    e.sec = w->sec;
    e.kind = w->kind;
    (void)fz_check(c, w->name, r, 0u, work, readable, claimed, &e, -1, -1, psel0 + (uint32_t)r, twice);
  }
}

/* The lies of the length, the header, the section table, the profile id and the identities. Every row's outcome is written from the Layout, the
 * Check order and the Refusal reasons of the specification and the "As built" notes of the reader (the offset a refusal reports). */
static void fz_build_lies(su_tab *t)
{
  const uint32_t n = (uint32_t)fz.n;
  const uint32_t trailer = n - IVS_SNAP_TRAILER_BYTES;
  uint32_t i;
  uint32_t s;
  su_row *r;
  memset(fz_lax, 0, sizeof fz_lax);
  /* total_bytes: 0, 1, below the smallest container, the smallest container, one below, one above, far above, the bound, 0xFFFFFFFF. The total is
   * judged before the trailer, so a recomputed trailer changes nothing. A total above the supplied length is a truncation (61, offset the
   * length); a total below it, or any other total that is not the size this build derives, is the total length reason (67, offset 12). */
  {
    static const uint32_t totals[] = {0u, 1u, 643u, 644u, 4298u, 4300u, 8598u, 16384u, 16385u, 0x00010000u + 4299u, 0x80000000u, 0xFFFFFFFFu};
    for (i = 0u; i < sizeof totals / sizeof totals[0]; i++) {
      uint32_t v = totals[i];
      r = fz_lie(t, v > n ? 61u : 67u, v > n ? n : IVS_SNAP_OFF_TOTAL, FZ_NONE, 0u, "total_bytes %u", (unsigned)v);
      su_setn(t, r, IVS_SNAP_OFF_TOTAL, 4u, v);
      r->reseal = (uint8_t)(i & 1u);
    }
  }
  /* the supplied length against the total the snapshot states: one byte short (the total is above the buffer), one, eight, 4,096 and every byte up
   * to the bound more (a total below the length), and above the bound (refused before any byte is read, offset the bound) */
  r = fz_lie(t, 61u, n - 1u, FZ_NONE, 0u, "a buffer one byte short");
  r->trunc = n - 1u;
  {
    static const uint32_t extra[] = {1u, 8u, 64u, 4096u, 16384u - 4299u};
    for (i = 0u; i < sizeof extra / sizeof extra[0]; i++) {
      r = fz_lie(t, 67u, IVS_SNAP_OFF_TOTAL, FZ_NONE, 0u, "%u extra bytes after the trailer", (unsigned)extra[i]);
      r->trunc = n + extra[i];
    }
  }
  r = fz_lie(t, 60u, IVS_SNAPSHOT_MAX_BYTES, FZ_NONE, 0u, "16,385 bytes supplied");
  r->trunc = IVS_SNAPSHOT_MAX_BYTES + 1u;
  r = fz_lie(t, 60u, IVS_SNAPSHOT_MAX_BYTES, FZ_NONE, 0u, "a length of 16,385 claimed for one byte");
  r->big_len = IVS_SNAPSHOT_MAX_BYTES + 1u;
  r = fz_lie(t, 60u, IVS_SNAPSHOT_MAX_BYTES, FZ_NONE, 0u, "a length of 2^32 claimed for one byte");
  r->big_len = (uint64_t)1u << 32;
  r = fz_lie(t, 60u, IVS_SNAPSHOT_MAX_BYTES, FZ_NONE, 0u, "a length of 2^64 - 1 claimed for one byte");
  r->big_len = UINT64_MAX;
  /* the versions, the flags, the schema version, the section count and the magic */
  {
    static const uint32_t versions[] = {0u, 2u, 0xFFFFu};
    static const uint32_t schemas[] = {0u, 2u, 0x00010000u, 0xFFFFFFFFu};
    static const uint32_t counts[] = {0u, 1u, 6u, 8u, 0xFFFFu, 0x00010007u, 0xFFFFFFFFu};
    for (i = 0u; i < sizeof versions / sizeof versions[0]; i++) {
      r = fz_lie(t, 63u, IVS_SNAP_OFF_FORMAT, FZ_NONE, 0u, "format version %u", (unsigned)versions[i]);
      su_setn(t, r, IVS_SNAP_OFF_FORMAT, 2u, versions[i]);
      r->reseal = (uint8_t)(i & 1u);
    }
    for (i = 0u; i < 16u; i++) {
      r = fz_lie(t, 64u, IVS_SNAP_OFF_FLAGS, FZ_NONE, 0u, "flag bit %u", (unsigned)i);
      su_setn(t, r, IVS_SNAP_OFF_FLAGS, 2u, 1u << i);
    }
    r = fz_lie(t, 64u, IVS_SNAP_OFF_FLAGS, FZ_NONE, 0u, "all 16 flag bits");
    su_setn(t, r, IVS_SNAP_OFF_FLAGS, 2u, 0xFFFFu);
    for (i = 0u; i < sizeof schemas / sizeof schemas[0]; i++) {
      r = fz_lie(t, 65u, IVS_SNAP_OFF_SCHEMA, FZ_NONE, 0u, "schema version %u", (unsigned)schemas[i]);
      su_setn(t, r, IVS_SNAP_OFF_SCHEMA, 4u, schemas[i]);
    }
    for (i = 0u; i < sizeof counts / sizeof counts[0]; i++) {
      r = fz_lie(t, 68u, IVS_SNAP_OFF_COUNT, FZ_NONE, 0u, "section count %u", (unsigned)counts[i]);
      su_setn(t, r, IVS_SNAP_OFF_COUNT, 4u, counts[i]);
      r->reseal = (uint8_t)(i & 1u);
    }
    r = fz_lie(t, 62u, 0u, FZ_NONE, 0u, "magic all zero");
    for (i = 0u; i < 8u; i++) {
      su_set(t, r, i, 0u);
    }
    r = fz_lie(t, 62u, 0u, FZ_NONE, 0u, "magic with the high bit of the first byte cleared (a 7-bit transfer)");
    su_set(t, r, 0u, 0x09u);
    r = fz_lie(t, 62u, 5u, FZ_NONE, 0u, "magic with the CR changed to LF (a text-mode transfer)");
    su_set(t, r, 5u, 0x0Au);
    r = fz_lie(t, 66u, IVS_SNAP_OFF_SCHEMA_DIGEST, FZ_NONE, 0u, "schema digest all zero");
    for (i = 0u; i < 32u; i++) {
      su_set(t, r, IVS_SNAP_OFF_SCHEMA_DIGEST + i, 0u);
    }
    r = fz_lie(t, 66u, IVS_SNAP_OFF_SCHEMA_DIGEST, FZ_NONE, 0u, "schema digest, last byte");
    su_xor(t, r, IVS_SNAP_OFF_SCHEMA_DIGEST + 31u, 0x80u);
  }
  /* the section table: wraparound, overlap, gap, unknown and duplicate domains, the reserved bytes, a length that is one too many or too few.
   * The entry the reader names is the first, in order, whose rule fails: the domain, the three zero bytes, the offset against the end of the entry
   * before, the length against the end of the payload; when every entry holds and the last section does not end at the trailer, the last entry's
   * length. */
  {
    uint32_t e1 = IVS_SNAP_OFF_TABLE + 1u * IVS_SNAP_ENTRY_BYTES;
    uint32_t e2 = IVS_SNAP_OFF_TABLE + 2u * IVS_SNAP_ENTRY_BYTES;
    uint32_t e3 = IVS_SNAP_OFF_TABLE + 3u * IVS_SNAP_ENTRY_BYTES;
    uint32_t e0 = IVS_SNAP_OFF_TABLE;
    uint32_t e6 = IVS_SNAP_OFF_TABLE + 6u * IVS_SNAP_ENTRY_BYTES;
    r = fz_lie(t, 69u, e0 + 4u, 0u, 0u, "entry 0 offset 0xFFFFFFFF");
    su_setn(t, r, e0 + 4u, 4u, 0xFFFFFFFFu);
    r = fz_lie(t, 69u, e0 + 8u, 0u, 0u, "entry 0 length 0xFFFFFFFF (offset plus length wraps 32 bits)");
    su_setn(t, r, e0 + 8u, 4u, 0xFFFFFFFFu);
    r = fz_lie(t, 69u, e3 + 8u, 3u, 0u, "entry 3 length 0xFFFFFFF0");
    su_setn(t, r, e3 + 8u, 4u, 0xFFFFFFF0u);
    r = fz_lie(t, 69u, e3 + 8u, 3u, 0u, "entry 3 length 0x80000000");
    su_setn(t, r, e3 + 8u, 4u, 0x80000000u);
    r = fz_lie(t, 69u, e2 + 4u, 2u, 0u, "entry 2 offset equals entry 1 offset (overlap)");
    su_setn(t, r, e2 + 4u, 4u, fz.sec_off[1]);
    r = fz_lie(t, 69u, e2 + 4u, 2u, 0u, "entry 2 offset one above (a gap)");
    su_setn(t, r, e2 + 4u, 4u, fz.sec_off[2] + 1u);
    r = fz_lie(t, 69u, e2 + 4u, 2u, 0u, "entry 2 offset one below (an overlap of one byte)");
    su_setn(t, r, e2 + 4u, 4u, fz.sec_off[2] - 1u);
    r = fz_lie(t, 69u, e2 + 4u, 2u, 0u, "entry 2 offset 0");
    su_setn(t, r, e2 + 4u, 4u, 0u);
    r = fz_lie(t, 69u, e2 + 4u, 2u, 0u, "entry 2 offset 0xFFFFFFFF");
    su_setn(t, r, e2 + 4u, 4u, 0xFFFFFFFFu);
    r = fz_lie(t, 69u, e1 + 4u, 1u, 0u, "entry 1 offset 0xFFFFFFFC");
    su_setn(t, r, e1 + 4u, 4u, 0xFFFFFFFCu);
    r = fz_lie(t, 69u, e1 + 4u, 1u, 0u, "section 0 one byte long: the next entry no longer begins where it ends");
    su_setn(t, r, e0 + 8u, 4u, fz.sec_len[0] + 1u);
    r = fz_lie(t, 69u, e1 + 4u, 1u, 0u, "section 0 one byte short");
    su_setn(t, r, e0 + 8u, 4u, fz.sec_len[0] - 1u);
    r = fz_lie(t, 69u, e6 + 8u, 6u, 0u, "the last section one byte long: past the payload");
    su_setn(t, r, e6 + 8u, 4u, fz.sec_len[6] + 1u);
    r = fz_lie(t, 69u, e6 + 8u, 6u, 0u, "the last section one byte short: it does not end at the trailer");
    su_setn(t, r, e6 + 8u, 4u, fz.sec_len[6] - 1u);
    for (i = 1u; i < 4u; i++) {
      r = fz_lie(t, 69u, e2 + i, 2u, 0u, "entry 2 reserved byte %u not zero", (unsigned)i);
      su_set(t, r, e2 + i, 0x01u);
    }
    /* the domains. The reader judges each entry's domain as known and strictly ascending, so which entry of a repeated or out-of-order pair it
     * names is its own choice; the reason is the specification's */
    r = fz_lie(t, 69u, e3, 3u, 0u, "entry 3 domain 0x20 (unknown)");
    su_set(t, r, e3, 0x20u);
    fz_lax_row(t, r);
    r = fz_lie(t, 69u, e0, 0u, 0u, "entry 0 domain 0");
    su_set(t, r, e0, 0u);
    fz_lax_row(t, r);
    r = fz_lie(t, 69u, e0, 0u, 0u, "entry 0 domain 3 (IMAGES holds no section)");
    su_set(t, r, e0, 3u);
    r = fz_lie(t, 69u, e6, 6u, 0u, "entry 6 domain 0xFF");
    su_set(t, r, e6, 0xFFu);
    fz_lax_row(t, r);
    r = fz_lie(t, 69u, e1, 1u, 0u, "entry 1 domain 1 (a duplicate of entry 0)");
    su_set(t, r, e1, 1u);
    fz_lax_row(t, r);
    r = fz_lie(t, 69u, e3 + IVS_SNAP_ENTRY_BYTES, 4u, 0u, "entry 4 domain 16 (a duplicate of entry 3)");
    su_set(t, r, e3 + IVS_SNAP_ENTRY_BYTES, 16u);
    fz_lax_row(t, r);
    r = fz_lie(t, 69u, e1, 1u, 0u, "entries 1 and 2 swapped (out of order)");
    su_set(t, r, e1, 4u);
    su_set(t, r, e2, 2u);
    fz_lax_row(t, r);
    /* the table is judged before the trailer: the same lies with a recomputed trailer */
    r = fz_lie(t, 69u, e2 + 4u, 2u, 0u, "entry 2 offset one above, resealed");
    su_setn(t, r, e2 + 4u, 4u, fz.sec_off[2] + 1u);
    r->reseal = 1u;
    r = fz_lie(t, 69u, e6 + 8u, 6u, 0u, "the last section one byte long, resealed");
    su_setn(t, r, e6 + 8u, 4u, fz.sec_len[6] + 1u);
    r->reseal = 1u;
  }
  /* the profile id forms with one byte changed (the forms that need the whole field are fz_id_forms): judged after the trailer, so each row is
   * resealed; the id is looked up only when its form is good */
  r = fz_lie(t, 71u, IVS_SNAP_OFF_PROFILE_ID + 40u, FZ_NONE, 0u, "a printable byte at index 40 after the NUL");
  su_set(t, r, IVS_SNAP_OFF_PROFILE_ID + 40u, (uint8_t)'Z');
  r->reseal = 1u;
  r = fz_lie(t, 71u, IVS_SNAP_OFF_PROFILE_ID + 5u + 1u, FZ_NONE, 0u, "an embedded NUL at index 5 with printable bytes after it");
  su_set(t, r, IVS_SNAP_OFF_PROFILE_ID + 5u, 0u);
  r->reseal = 1u;
  r = fz_lie(t, 71u, IVS_SNAP_OFF_PROFILE_ID + 3u, FZ_NONE, 0u, "a non-ASCII byte 0x80 at index 3");
  su_set(t, r, IVS_SNAP_OFF_PROFILE_ID + 3u, 0x80u);
  r->reseal = 1u;
  r = fz_lie(t, 71u, IVS_SNAP_OFF_PROFILE_ID + 5u, FZ_NONE, 0u, "a control byte 0x01 at index 5");
  su_set(t, r, IVS_SNAP_OFF_PROFILE_ID + 5u, 0x01u);
  r->reseal = 1u;
  r = fz_lie(t, 71u, IVS_SNAP_OFF_PROFILE_ID + 7u, FZ_NONE, 0u, "DEL 0x7F at index 7");
  su_set(t, r, IVS_SNAP_OFF_PROFILE_ID + 7u, 0x7Fu);
  r->reseal = 1u;
  r = fz_lie(t, 70u, trailer, FZ_NONE, 0u, "an unregistered id with a stale trailer is the trailer");
  su_set(t, r, IVS_SNAP_OFF_PROFILE_ID, (uint8_t)'z');
  /* the identities: the parameter digest and the three slot records */
  r = fz_lie(t, 74u, IVS_SNAP_OFF_PARAMS, FZ_NONE, 0u, "parameter digest, first byte");
  su_xor(t, r, IVS_SNAP_OFF_PARAMS, 0x01u);
  r->reseal = 1u;
  r = fz_lie(t, 74u, IVS_SNAP_OFF_PARAMS, FZ_NONE, 0u, "parameter digest, last byte");
  su_xor(t, r, IVS_SNAP_OFF_PARAMS + 31u, 0x80u);
  r->reseal = 1u;
  r = fz_lie(t, 74u, IVS_SNAP_OFF_PARAMS, FZ_NONE, 0u, "parameter digest all zero");
  for (i = 0u; i < 32u; i++) {
    su_set(t, r, IVS_SNAP_OFF_PARAMS + i, 0u);
  }
  r->reseal = 1u;
  for (s = 0u; s < 3u; s++) {
    uint32_t rec = IVS_SNAP_OFF_SLOTS + s * IVS_SNAP_SLOT_BYTES;
    r = fz_lie(t, 76u, rec + 4u, FZ_NONE, s + 1u, "slot %u digest, first byte", (unsigned)s);
    su_xor(t, r, rec + 4u, 0x01u);
    r->reseal = 1u;
    r = fz_lie(t, 76u, rec + 4u, FZ_NONE, s + 1u, "slot %u digest, last byte", (unsigned)s);
    su_xor(t, r, rec + 4u + 31u, 0x80u);
    r->reseal = 1u;
    r = fz_lie(t, 76u, rec + 36u, FZ_NONE, s + 1u, "slot %u length one more", (unsigned)s);
    su_xor(t, r, rec + 39u, 0x01u);
    r->reseal = 1u;
    r = fz_lie(t, 78u, rec, FZ_NONE, s + 1u, "slot %u loaded flag 2", (unsigned)s);
    su_setn(t, r, rec, 4u, 2u);
    r->reseal = 1u;
    r = fz_lie(t, 78u, rec, FZ_NONE, s + 1u, "slot %u loaded flag 0xFFFFFFFF", (unsigned)s);
    su_setn(t, r, rec, 4u, 0xFFFFFFFFu);
    r->reseal = 1u;
    r = fz_lie(t, 78u, rec + 36u, FZ_NONE, s + 1u, "slot %u loaded flag 0 over a full record", (unsigned)s);
    su_setn(t, r, rec, 4u, 0u);
    r->reseal = 1u;
    r = fz_lie(t, 76u, rec, FZ_NONE, s + 1u, "slot %u empty in the snapshot, loaded in the instance", (unsigned)s);
    su_setn(t, r, rec, 4u, 0u);
    su_setn(t, r, rec + 36u, 4u, 0u);
    for (i = 0u; i < 32u; i++) {
      su_set(t, r, rec + 4u + i, 0u);
    }
    r->reseal = 1u;
    r = fz_lie(t, 70u, trailer, FZ_NONE, 0u, "slot %u digest flipped with a stale trailer is the trailer", (unsigned)s);
    su_xor(t, r, rec + 4u, 0x01u);
  }
}


/* The profile id forms that change the whole field: the id bytes from index 0 (the rest of the field NUL), the trailer recomputed or not, and the
 * outcome the specification gives: 71 at the first offending byte (the last byte of the field when there is no NUL), 72 for a well formed id that
 * is not the registered one, 70 when the trailer is stale. */
static void fz_id_forms(fz_ctx *c)
{
  static const struct {
    const char *label;
    const char *text;
    int fill; /* >0: that many 'a' bytes instead of text */
    int sealed;
    uint32_t reason;
    uint32_t at; /* the index of the byte reported (71) or 0 */
  } rows[] = {
      {"id of 63 printable bytes and the NUL", "", 63, 1, 72u, 0u},
      {"id empty", "", 0, 1, 72u, 0u},
      {"id in capitals", "INTV-2609-NTSC-FAMILY-P1", 0, 1, 72u, 0u},
      {"id one byte short", "intv-2609-ntsc-family-p", 0, 1, 72u, 0u},
      {"id one byte long", "intv-2609-ntsc-family-p1x", 0, 1, 72u, 0u},
      {"id of spaces, a tilde and a hyphen", " ~- ", 0, 1, 72u, 0u},
      {"id of 64 printable bytes (no NUL)", "", 64, 1, 71u, 63u},
      {"id of 64 printable bytes with a stale trailer is the trailer", "", 64, 0, 70u, 0u},
  };
  uint32_t i;
  for (i = 0u; i < sizeof rows / sizeof rows[0]; i++) {
    static uint8_t work[IVS_SNAPSHOT_MAX_BYTES];
    fz_exp e;
    uint32_t j;
    size_t want = strlen(rows[i].text);
    memcpy(work, fz.snap, fz.n);
    for (j = 0u; j < 64u; j++) {
      work[IVS_SNAP_OFF_PROFILE_ID + j] = rows[i].fill > 0 ? (j < (uint32_t)rows[i].fill ? (uint8_t)'a' : 0u) : (j < want ? (uint8_t)rows[i].text[j] : 0u);
    }
    if (rows[i].sealed) {
      su_reseal(work, fz.n);
    }
    if (rows[i].reason == 70u) {
      fz_exp_one(&e, 70u, FZ_X_OFF | FZ_X_SEC | FZ_X_KIND, fz.n - IVS_SNAP_TRAILER_BYTES, FZ_NONE, 0u);
    } else if (rows[i].reason == 71u) {
      fz_exp_one(&e, 71u, FZ_X_OFF | FZ_X_SEC | FZ_X_KIND, IVS_SNAP_OFF_PROFILE_ID + rows[i].at, FZ_NONE, 0u);
    } else {
      fz_exp_one(&e, 72u, FZ_X_OFF | FZ_X_SEC | FZ_X_KIND, IVS_SNAP_OFF_PROFILE_ID, FZ_NONE, 0u);
    }
    (void)fz_check(c, rows[i].label, i, 0u, work, fz.n, fz.n, &e, -1, -1, i, 0);
  }
}

static int fz_run_lies(const char *name, void (*build)(su_tab *), size_t cap, int twice, int expect_all_refused, void (*extra)(fz_ctx *))
{
  fz_ctx c;
  su_tab t;
  int ok;
  ST_CHECK(fz_get());
  fz_ctx_init(&c, name);
  ST_CHECK(su_tab_init(&t, cap));
  build(&t);
  fz_run_tab(&c, &t, 0u, twice);
  if (extra != NULL) {
    extra(&c);
  }
  ok = fz_finish(&c);
  if (expect_all_refused && c.accepted != 0u) {
    ivs_test_diag("%s: a lie was accepted", name);
    ok = IVS_TEST_FAIL;
  }
  ivs_test_diag("%s: %zu rows, reasons seen %06X", name, t.n, (unsigned)c.seen);
  su_tab_free(&t);
  return ok;
}

static int fz_length_and_table_lies(void)
{
  return fz_run_lies("state.fuzz.length-and-table-lies", fz_build_lies, FZ_TAB_CAP, 0, 1, fz_id_forms);
}

/* ---- the semantic lies: one derived invariant (or device validator) violated at a time, with a valid trailer ---- */

static int fz_where(su_tab *t, const char *row, uint32_t *sec, uint32_t *off)
{
  uint32_t cnt = 0u;
  if (!su_row_pos(row, sec, off, &cnt)) {
    ivs_test_diag("no STATE row '%s' in the field table", row);
    t->bad = 1;
    *sec = 0u;
    *off = 0u;
    return 0;
  }
  return 1;
}

static uint64_t fz_base_val(su_tab *t, const char *row, uint32_t elem, uint32_t width)
{
  uint32_t sec = 0u;
  uint32_t off = 0u;
  if (!fz_where(t, row, &sec, &off)) {
    return 0u;
  }
  return su_get(fz.snap + off + elem * width, width);
}

/* One member to change: row name, element index, width and the value to put there. */
typedef struct fz_mem {
  const char *row;
  uint32_t elem;
  uint32_t width;
  uint64_t v;
} fz_mem;

/* A resealed row that changes up to three members and must be refused as `reason` at the first element of the row `at` (the struct or scalar
 * the rule is about: the CPU, the phase, machine time, the PSG state, the STIC state, the input row or the video count). */
static void fz_member_row(su_tab *t, uint32_t reason, const char *at, const char *label, const fz_mem *a, const fz_mem *b, const fz_mem *c)
{
  const fz_mem *m[3];
  uint32_t k;
  uint32_t sec = 0u;
  uint32_t off = 0u;
  su_row *r;
  m[0] = a;
  m[1] = b;
  m[2] = c;
  (void)fz_where(t, at, &sec, &off);
  r = fz_lie(t, reason, off, sec, 0u, "%u %s", (unsigned)reason, label);
  for (k = 0u; k < 3u; k++) {
    uint32_t msec;
    uint32_t moff;
    if (m[k] == NULL || m[k]->row == NULL) {
      continue;
    }
    if (fz_where(t, m[k]->row, &msec, &moff)) {
      su_setn(t, r, moff + m[k]->elem * m[k]->width, m[k]->width, m[k]->v);
    }
  }
  r->reseal = 1u;
}

static void fz_build_semantic(su_tab *t)
{
  uint64_t time_ticks = fz_base_val(t, "time_ticks", 0u, 8u);
  uint64_t ei = fz_base_val(t, "stic.event_index", 0u, 4u);
  uint64_t ns = fz_base_val(t, "psg.next_sample", 0u, 8u);
  uint64_t em = fz_base_val(t, "obs.audio.emitted", 0u, 8u);
  uint64_t vc = fz_base_val(t, "obs.video.count", 0u, 8u);
  uint64_t gate1 = fz_base_val(t, "mem.gate_open", 1u, 1u);
  uint64_t pull0 = fz_base_val(t, "mem.port_pulldown", 0u, 1u);
  uint64_t reg0 = fz_base_val(t, "mem.psg_reg", 0u, 2u);
  memset(fz_lax, 0, sizeof fz_lax);
  /* INV-2: the STIC timeline and the gate flags */
  {
    fz_mem g1 = {"mem.gate_open", 1u, 1u, gate1 ^ 1u};
    fz_mem g0 = {"mem.gate_open", 0u, 1u, 0u};
    fz_mem g3 = {"mem.gate_open", 3u, 1u, 1u};
    fz_mem e1 = {"stic.event_index", 0u, 4u, ei + 1u};
    fz_mem e0 = {"stic.event_index", 0u, 4u, ei - 1u};
    fz_member_row(t, 79u, "stic.frame_index", "INV-2 the register gate flag against the cursor", &g1, NULL, NULL);
    fz_member_row(t, 79u, "stic.frame_index", "INV-2 gate 0 is always open", &g0, NULL, NULL);
    fz_member_row(t, 79u, "stic.frame_index", "INV-2 gate 3 is always closed", &g3, NULL, NULL);
    if (ei + 1u < 16u) {
      fz_member_row(t, 79u, "stic.frame_index", "INV-2 the cursor one event ahead", &e1, NULL, NULL);
    }
    if (ei >= 1u) {
      fz_member_row(t, 79u, "stic.frame_index", "INV-2 the cursor one event behind", &e0, NULL, NULL);
    }
  }
  /* INV-3: the sample index, the emitted count, the previous levels, the register masks */
  {
    fz_mem nhi = {"psg.next_sample", 0u, 8u, ns + 1u};
    fz_mem nlo = {"psg.next_sample", 0u, 8u, ns - 1u};
    fz_mem ehi = {"obs.audio.emitted", 0u, 8u, em + 1u};
    fz_mem pl = {"obs.audio.prev_levels", 0u, 4u, 0x10u};
    fz_mem pt = {"obs.audio.prev_levels", 0u, 4u, 0x01000000u};
    fz_mem r0 = {"mem.psg_reg", 0u, 2u, reg0 | 0x0100u};
    fz_member_row(t, 79u, "psg.next_sample", "INV-3 the next sample index one more than the clock says", &nhi, NULL, NULL);
    fz_member_row(t, 79u, "psg.next_sample", "INV-3 the next sample index one less", &nlo, NULL, NULL);
    fz_member_row(t, 79u, "psg.next_sample", "INV-3 the emitted count is not the next index minus one", &ehi, NULL, NULL);
    fz_member_row(t, 79u, "psg.next_sample", "INV-3 a previous level of 16", &pl, NULL, NULL);
    fz_member_row(t, 79u, "psg.next_sample", "INV-3 the fourth byte of the previous levels is not 0", &pt, NULL, NULL);
    fz_member_row(t, 79u, "psg.next_sample", "INV-3 register 0 holds a bit its mask drops", &r0, NULL, NULL);
  }
  /* INV-4: the pull-downs against the words, the words of controllers the profile does not connect, the input floor */
  {
    fz_mem p0 = {"mem.port_pulldown", 0u, 1u, pull0 ^ 0x01u};
    fz_mem w0 = {"obs.input.words", 0u, 4u, 0x80000000u};
    fz_mem w2 = {"obs.input.words", 2u, 4u, 1u};
    fz_mem fl = {"obs.input.floor_tick", 0u, 8u, time_ticks + 1u};
    fz_member_row(t, 79u, "obs.input.words", "INV-4 the pull-downs against the words", &p0, NULL, NULL);
    fz_member_row(t, 79u, "obs.input.words", "INV-4 the reserved bit of a controller word", &w0, NULL, NULL);
    fz_member_row(t, 79u, "obs.input.words", "INV-4 a word of a controller the profile does not connect", &w2, NULL, NULL);
    fz_member_row(t, 79u, "obs.input.words", "INV-4 the input floor above machine time", &fl, NULL, NULL);
  }
  /* INV-6: the video count is the frames the STIC completed */
  {
    fz_mem vhi = {"obs.video.count", 0u, 8u, vc + 1u};
    fz_mem vbig = {"obs.video.count", 0u, 8u, UINT64_MAX};
    fz_member_row(t, 79u, "obs.video.count", "INV-6 a video count that differs from the frame index", &vhi, NULL, NULL);
    fz_member_row(t, 79u, "obs.video.count", "INV-6 a video count of 2^64 - 1", &vbig, NULL, NULL);
  }
  /* the validators and the scalars (INV-1, INV-5) */
  {
    fz_mem ev16 = {"stic.event_index", 0u, 4u, 16u};
    fz_mem dbd = {"cpu.dbd", 0u, 1u, 1u};
    fz_mem pint = {"cpu.prev_interruptible", 0u, 1u, 1u};
    fz_mem faulted = {"cpu.run_state", 0u, 1u, 4u};
    fz_mem rs9 = {"cpu.run_state", 0u, 1u, 9u};
    fz_mem st = {"cpu.status", 0u, 1u, 0x01u};
    fz_mem ph0 = {"phase", 0u, 4u, 0u};
    fz_mem ph2 = {"phase", 0u, 4u, 2u};
    fz_mem thi = {"time_ticks", 0u, 8u, 0x8000000000000001ull};
    fz_mem tout = {"psg.tone_out", 0u, 4u, 2u};
    fz_mem lvl = {"psg.env_level", 0u, 4u, 16u};
    fz_mem cs = {"stic.cs_ptr", 0u, 4u, 4u};
    fz_member_row(t, 78u, "stic.frame_index", "INV-1 the STIC event index at the table size", &ev16, NULL, NULL);
    fz_member_row(t, 78u, "cpu.r", "INV-1 a pending SDBD with the interruptible flag set", &dbd, &pint, NULL);
    fz_member_row(t, 78u, "cpu.r", "INV-1 a faulted run state with fault reason 0", &faulted, NULL, NULL);
    fz_member_row(t, 78u, "cpu.r", "INV-1 a run state of 9", &rs9, NULL, NULL);
    fz_member_row(t, 78u, "cpu.r", "INV-1 a status bit outside the mask", &st, NULL, NULL);
    fz_member_row(t, 78u, "phase", "INV-5 a phase other than POWERED (0)", &ph0, NULL, NULL);
    fz_member_row(t, 78u, "phase", "INV-5 a phase other than POWERED (2)", &ph2, NULL, NULL);
    fz_member_row(t, 78u, "time_ticks", "INV-5 machine time above 2^63", &thi, NULL, NULL);
    fz_member_row(t, 78u, "psg.next_sample", "INV-1 a PSG tone output flag of 2", &tout, NULL, NULL);
    fz_member_row(t, 78u, "psg.next_sample", "INV-1 an envelope level of 16", &lvl, NULL, NULL);
    fz_member_row(t, 78u, "stic.frame_index", "INV-1 the color stack pointer 4", &cs, NULL, NULL);
  }
}

static int fz_semantic_lies(void)
{
  return fz_run_lies("state.fuzz.semantic-lies", fz_build_semantic, 128u, 0, 1, NULL);
}

const ivs_test_case ivs_state_fuzz_cases[] = {
    {"state.fuzz.truncations", fz_truncations, NULL},
    {"state.fuzz.byte-flips", fz_byte_flips, NULL},
    {"state.fuzz.byte-flips-resealed", fz_byte_flips_resealed, NULL},
    {"state.fuzz.header-bit-flips", fz_header_bit_flips, NULL},
    {"state.fuzz.length-and-table-lies", fz_length_and_table_lies, NULL},
    {"state.fuzz.semantic-lies", fz_semantic_lies, NULL},
};
const size_t ivs_state_fuzz_count = sizeof ivs_state_fuzz_cases / sizeof ivs_state_fuzz_cases[0];
