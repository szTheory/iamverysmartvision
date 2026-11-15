/* SPDX-License-Identifier: MIT */
/* Suite state, family sweep (plan 08-14, D-16, 08-DISCUSS-state 5.4): the runtime row-omission sweep over the STATE rows of the digest field
 * table at the witness cut of each row (data/expected/state-checklist.ids). The cut tests of state_cut.c show that a snapshot restores into a
 * poisoned target and continues like the uninterrupted run; the sweep asks the converse question of every row: does it matter? A restore that
 * forgot the row would leave some other value in it, so for each STATE row the sweep restores the snapshot of the row's witness cut correctly
 * into a poisoned target, replaces the bytes of that row with a sweep value, continues to the end tick of the cut and requires the result to
 * differ from the uninterrupted reference. The sweep never calls the restore or snapshot code to learn what it should do: the offset, width and
 * count of a row come from the field table read as data, and the reference is the uninterrupted run.
 *   (1) zero-coincidence guard: the bytes of the row at the cut differ from the bytes of the row in the power-on state of the same program (at
 *       the cut of tick 0, which is the power-on state, from an all-zero row: the state of a fresh target), so a witness at which the row is
 *       default is no witness;
 *   (2) omission: the continuation of the instance with the replaced row differs from the reference outside the row's own bytes: the final
 *       whole instance compared with the row's byte range masked, the video, audio and PSG log streams, the first-index fields and the other
 *       fields of the run report (time, units, stop reason, events applied), the status of the run call, and the public reports (CPU state,
 *       STIC state, info), of which a few echo the row itself; the sweep says which kind of divergence it saw.
 * The sweep value is the byte-wise complement of the row's bytes (the first candidate) and, for a row that the validators restrict to a few
 * values (a flag, a run state), the other members of the set: every candidate is first judged by the validators and invariants of the restore
 * (ivs_snapshot_invariants on a probe of the modified instance), because a value no run reaches may be outside what the core can safely step
 * (an index out of its table); candidates that pass all six rules are tried first, then those that pass the range rules and break only a derived
 * rule. A row for which no candidate differs is INERT. The checklist column says what is expected: ROW (must diverge, and must be non-default at
 * its cut) or EXEMPT:<reason> (must be inert; the argument is in docs/state/hidden-state-checklist.md).
 *   state.sweep.row-omission                     the sweep over all STATE rows
 *   state.judge.row-sweep-detects-omission       the sweep can fail both ways: a row known to be read (cpu.r at HP-SDBD) is reported as divergent
 *                                                and an exempt row as inert */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "machine/machine.h"
#include "state_cases.h"
#include "state_util.h"

/* The STATE rows of the checklist: the row, its witness cut and the sweep column. */
typedef struct sw_listed {
  const char *row;
  const char *witness;
  const char *sweep;
} sw_listed;

#define IVSN_SOURCE(p, h)
#define STATE_ROW(r, c, s) {r, c, s},
static const sw_listed sw_rows[] = {
#include "state_checklist.inc"
};
#undef STATE_ROW
#undef IVSN_SOURCE

#define SW_N (sizeof sw_rows / sizeof sw_rows[0])

/* The appended notes of data/expected/state-sweep-notes.ids: a line replaces the witness cut and the sweep column of the row it names (the
 * checklist itself is frozen oracle data and is never edited). */
typedef struct sw_note {
  const char *row;
  const char *cut;
  const char *sweep;
} sw_note;

#define IVSN_SOURCE(p, h)
#define STATE_NOTE(r, c, s) {r, c, s},
static const sw_note sw_notes[] = {
#include "state_notes.inc"
};
#undef STATE_NOTE
#undef IVSN_SOURCE

/* The effective rows: the checklist with the notes applied. sw_merge also judges the notes (a note must name a STATE row of the checklist and
 * a cut of the cut file, an exemption needs a reason of at least three words, and after the amendment every cut is still the witness of at
 * least one row): 1 when they are sound. */
static sw_listed sw_eff[SW_N];
static int sw_merged;

static int sw_merge(void)
{
  size_t i;
  size_t k;
  int ok = 1;
  if (sw_merged) {
    return 1;
  }
  for (i = 0u; i < SW_N; i++) {
    sw_eff[i] = sw_rows[i];
  }
  for (k = 0u; k < sizeof sw_notes / sizeof sw_notes[0]; k++) {
    int found = 0;
    for (i = 0u; i < SW_N; i++) {
      if (strcmp(sw_rows[i].row, sw_notes[k].row) == 0) {
        found = 1;
        sw_eff[i].witness = sw_notes[k].cut;
        sw_eff[i].sweep = sw_notes[k].sweep;
      }
    }
    if (!found || sc_cut_find(sw_notes[k].cut) == NULL) {
      ivs_test_diag("sweep note for %s names no STATE row of the checklist or no cut (%s)", sw_notes[k].row, sw_notes[k].cut);
      ok = 0;
    }
    if (strncmp(sw_notes[k].sweep, "EXEMPT:", 7) == 0) {
      const char *r = sw_notes[k].sweep + 7;
      size_t words = 1u;
      size_t j;
      for (j = 0u; r[j] != '\0'; j++) {
        words += (size_t)(r[j] == '_' && j > 0u && r[j + 1u] != '\0' && r[j + 1u] != '_');
      }
      if (r[0] == '\0' || r[0] == '_' || words < 3u) {
        ivs_test_diag("sweep note for %s: the exemption needs a reason of at least three words", sw_notes[k].row);
        ok = 0;
      }
    }
  }
  for (k = 0u; k < sc_cut_count(); k++) {
    int used = 0;
    for (i = 0u; i < SW_N; i++) {
      used |= strcmp(sw_eff[i].witness, sc_cut_at(k)->id) == 0;
    }
    if (!used) {
      ivs_test_diag("the cut %s is the witness of no row after the sweep notes", sc_cut_at(k)->id);
      ok = 0;
    }
  }
  sw_merged = ok;
  return ok;
}

/* What differed (the divergence kinds). BEHAVIOUR kinds are outputs and state that are not the row's own bytes; SW_D_REPORT is the public reports. */
#define SW_D_STATUS 0x01u /* the run call refused or reported another status */
#define SW_D_RUN 0x02u    /* time, units, stop reason or events applied */
#define SW_D_STREAM 0x04u /* a video, audio or PSG log byte */
#define SW_D_FIRST 0x08u  /* a first-index field of the run report */
#define SW_D_FINAL 0x10u  /* the final instance outside the row's bytes */
#define SW_D_REPORT 0x20u /* a public report (CPU state, STIC state, info) */

#define SW_BEHAVIOUR (SW_D_STATUS | SW_D_RUN | SW_D_STREAM | SW_D_FIRST | SW_D_FINAL)

/* One cut of the sweep: its program, the source at the cut, the snapshot, the target restored from it and the power-on instance. */
typedef struct sw_cut {
  const sc_cut *cut;
  const sc_run *run;
  uint64_t tick;
  uint64_t end;
  const ivs_input_event *ev;
  uint32_t nev;
  ivs_ms_inst src;
  ivs_ms_inst po;
  ivs_ms_inst t0;
  sc_leg l1;
  uint8_t *scratch;
  int ready;
} sw_cut;

#define SW_CUT_MAX 24u
static sw_cut sw_cuts[SW_CUT_MAX];
static size_t sw_ncuts;

static void sw_cut_release(sw_cut *c)
{
  if (!c->ready) {
    return;
  }
  sc_leg_free(&c->l1);
  ivs_ms_free(&c->src);
  ivs_ms_free(&c->po);
  ivs_ms_free(&c->t0);
  free(c->scratch);
  memset(c, 0, sizeof *c);
}

static void sw_free_all(void)
{
  size_t i;
  for (i = 0u; i < sw_ncuts; i++) {
    sw_cut_release(&sw_cuts[i]);
  }
  sw_ncuts = 0u;
  sc_ref_free();
}

/* The context of a cut, built on first use. full: the whole window (a normal build, or the must-fail id of a fault). */
static sw_cut *sw_cut_get(const sc_cut *cut, int full)
{
  size_t i;
  sw_cut *c;
  uint8_t *snap;
  size_t n = 0u;
  for (i = 0u; i < sw_ncuts; i++) {
    if (sw_cuts[i].cut == cut) {
      return &sw_cuts[i];
    }
  }
  if (sw_ncuts >= SW_CUT_MAX) {
    return NULL;
  }
  c = &sw_cuts[sw_ncuts];
  memset(c, 0, sizeof *c);
  c->cut = cut;
  c->run = sc_run_find(cut->program);
  if (c->run == NULL || cut->tick == SC_PENDING) {
    return NULL;
  }
  c->tick = cut->tick;
  c->end = sc_window_end(cut->tick, sc_cut_end(cut), full);
  c->ev = sc_events(c->run, &c->nev);
  snap = (uint8_t *)malloc(IVS_SNAPSHOT_MAX_BYTES);
  c->scratch = (uint8_t *)aligned_alloc(IVS_MACHINE_ALIGN, IVS_MACHINE_BYTES);
  if (snap == NULL || c->scratch == NULL || !sc_new(&c->src, c->run, 1) || !sc_new(&c->po, c->run, 1)) {
    free(snap);
    free(c->scratch);
    return NULL;
  }
  if (c->tick > 0u && (!sc_run_leg(c->src.m, c->tick, c->ev, c->nev, 0u, &c->l1) || ((const struct ivs_machine *)c->src.m)->time_ticks != c->tick)) {
    ivs_test_diag("sweep: the run to the cut %s failed", cut->id);
    free(snap);
    free(c->scratch);
    ivs_ms_free(&c->src);
    ivs_ms_free(&c->po);
    return NULL;
  }
  if (!su_snapshot(c->src.m, snap, IVS_SNAPSHOT_MAX_BYTES, &n) || !sc_new(&c->t0, c->run, 1)) {
    free(snap);
    free(c->scratch);
    ivs_ms_free(&c->src);
    ivs_ms_free(&c->po);
    return NULL;
  }
  state_poison(c->t0.m);
  if (ivs_machine_snapshot_restore(c->t0.m, snap, n, NULL) != IVS_OK || memcmp(c->t0.storage, c->src.storage, IVS_MACHINE_BYTES) != 0) {
    ivs_test_diag("sweep: the correct restore at the cut %s does not equal the source", cut->id);
    free(snap);
    free(c->scratch);
    ivs_ms_free(&c->src);
    ivs_ms_free(&c->po);
    ivs_ms_free(&c->t0);
    return NULL;
  }
  free(snap);
  c->ready = 1;
  sw_ncuts++;
  return c;
}

/* ---- the sweep values ---- */

/* The value of element e of a row (native byte order) and its replacement. */
static uint64_t sw_get(const uint8_t *p, uint32_t w)
{
  uint64_t v = 0u;
  memcpy(&v, p, w); /* little-endian hosts only matter for which byte is the low one: the numeric value is what the candidates use */
  return v;
}

static void sw_put(uint8_t *p, uint64_t v, uint32_t w)
{
  memcpy(p, &v, w);
}

static uint64_t sw_mask(uint32_t w)
{
  return w >= 8u ? ~UINT64_C(0) : ((UINT64_C(1) << (8u * w)) - 1u);
}

#define SW_ROW_MAX 1100u /* the largest row: mem.sysram, 512 words */
#define SW_K_SINGLE 8u   /* candidates 0 to 7 apply to every row; the next ones to a single-element row */

/* Candidate k of a row of w-byte elements, n elements, whose current bytes are cur: written to out (w * n bytes). Returns 1 when produced,
 * 0 when this k does not apply (skip it) and -1 when there are no more. */
static int sw_cand(uint32_t w, uint32_t n, const uint8_t *cur, uint32_t k, uint8_t *out)
{
  uint32_t len = w * n;
  uint32_t e;
  uint64_t m = sw_mask(w);
  memcpy(out, cur, len);
  switch (k) {
  case 0u: /* the byte-wise complement of the whole row: the sweep value of the narrative */
    for (e = 0u; e < len; e++) {
      out[e] = (uint8_t)~cur[e];
    }
    return 1;
  case 1u: /* the lowest bit of every element flipped: the other member of a 0/1 flag, a small step for a counter */
    for (e = 0u; e < n; e++) {
      sw_put(out + e * w, sw_get(cur + e * w, w) ^ 1u, w);
    }
    return 1;
  case 2u: /* the constant fill 0xA5 of the poisoned target: what a restore that skipped the row would leave */
    memset(out, 0xA5, len);
    return 1;
  case 3u:
    memset(out, 0, len);
    return 1;
  case 4u: /* the first element complemented, the others as they are */
    sw_put(out, ~sw_get(cur, w) & m, w);
    return 1;
  case 5u: /* the last element complemented */
    sw_put(out + (n - 1u) * w, ~sw_get(cur + (n - 1u) * w, w) & m, w);
    return 1;
  case 6u:
    sw_put(out, sw_get(cur, w) ^ 1u, w);
    return 1;
  case 7u:
    sw_put(out + (n - 1u) * w, sw_get(cur + (n - 1u) * w, w) ^ 1u, w);
    return 1;
  default:
    break;
  }
  if (n != 1u) {
    return -1;
  }
  if (k < SW_K_SINGLE + 256u) { /* every small value, for a scalar whose valid values are few (a state number) */
    sw_put(out, (uint64_t)(k - SW_K_SINGLE) & m, w);
    return 1;
  }
  if (k == SW_K_SINGLE + 256u) {
    sw_put(out, (sw_get(cur, w) + 1u) & m, w);
    return 1;
  }
  if (k == SW_K_SINGLE + 257u) {
    sw_put(out, (sw_get(cur, w) - 1u) & m, w);
    return 1;
  }
  return -1;
}

/* ---- the comparison of one continuation with the reference ---- */

static int sw_stream_equal(const uint8_t *a, uint64_t n1, const uint8_t *b, uint64_t n2, const uint8_t *ref, uint64_t nref)
{
  if (n1 + n2 != nref) {
    return 0;
  }
  return (n1 == 0u || memcmp(a, ref, (size_t)n1) == 0) && (n2 == 0u || memcmp(b, ref + n1, (size_t)n2) == 0);
}

/* What differs between the continuation on m (its leg l2 after the cut) and the uninterrupted reference; the row's bytes (off, len) are masked in the
 * final instance compare. */
static uint32_t sw_diverge(const sw_cut *cx, const ivs_machine *m, const sc_leg *l2, uint32_t off, uint32_t len)
{
  const sc_ref *ref = sc_ref_get(cx->run, cx->end);
  const sc_leg *l1 = &cx->l1;
  const uint8_t *fin = (const uint8_t *)m;
  ivs_cpu_state_report cpu;
  ivs_stic_report stic;
  ivs_info info;
  uint32_t d = 0u;
  if (ref == NULL) {
    return SW_D_STATUS;
  }
  if (l2->st != IVS_OK) {
    return SW_D_STATUS;
  }
  if (l2->now != ref->leg.now || l1->units + l2->units != ref->leg.units || l2->stop != ref->leg.stop || l1->consumed + l2->consumed != ref->leg.consumed) {
    d |= SW_D_RUN;
  }
  if (!sw_stream_equal(l1->video, l1->vbytes, l2->video, l2->vbytes, ref->leg.video, ref->leg.vbytes) ||
      !sw_stream_equal(l1->audio, l1->abytes, l2->audio, l2->abytes, ref->leg.audio, ref->leg.abytes) ||
      !sw_stream_equal(l1->log, l1->lbytes, l2->log, l2->lbytes, ref->leg.log, ref->leg.lbytes)) {
    d |= SW_D_STREAM;
  }
  if (l2->vfirst != l1->vfirst + l1->frames || l2->afirst != l1->afirst + l1->abytes / ref->info.audio_sample_bytes ||
      l2->lfirst != l1->lfirst + l1->lbytes / ref->info.psg_log_record_bytes) {
    d |= SW_D_FIRST;
  }
  if (memcmp(fin, ref->x.storage, off) != 0 || memcmp(fin + off + len, ref->x.storage + off + len, IVS_MACHINE_BYTES - off - len) != 0) {
    d |= SW_D_FINAL;
  }
  if (!sc_reports(m, &cpu, &stic, &info) || memcmp(&cpu, &ref->cpu, sizeof cpu) != 0 || memcmp(&stic, &ref->stic, sizeof stic) != 0 ||
      memcmp(&info, &ref->info, sizeof info) != 0) {
    d |= SW_D_REPORT;
  }
  /* the run report echoes the controller words in force and the input floor: public report members like the three reports above */
  if (l2->floor != ref->leg.floor || memcmp(l2->ctrl, ref->leg.ctrl, sizeof l2->ctrl) != 0) {
    d |= SW_D_REPORT;
  }
  return d;
}

/* ---- one row ---- */

typedef struct sw_out {
  int guard;       /* 1: the row at the cut differs from the power-on state (see sw_guard) */
  int equals_po;   /* 1: the row at the cut equals the row of the power-on state */
  int behaviour;   /* 1: some sweep value gave a continuation that differs from the reference in behaviour (SW_BEHAVIOUR) */
  int report;      /* 1: some sweep value differed in a public report only (an echo of the row) */
  uint32_t kinds;  /* the SW_D_* of the first diverging continuation (the behaviour one if there is one) */
  uint32_t tried;  /* continuations run */
  uint32_t valid;  /* candidates that were safe to step */
  uint32_t full;   /* of which also passed the derived invariants */
  uint32_t cand;   /* the index of the candidate that diverged */
} sw_out;

#define SW_RUNS_PER_PASS 6u

/* The rows of a validated group whose RANGE keeps the core inside its tables but whose RELATION to a neighbour row the validator also enforces: a
 * running envelope's level is a function of its cycle index and direction, so no single-row replacement passes the validator, yet a level, cycle
 * or direction inside its range cannot index outside a table. For these rows the second pass accepts a value inside the range (the largest
 * value of the row) instead of one that merely breaks a derived rule. */
typedef struct sw_range {
  const char *row;
  uint32_t max;
} sw_range;
static const sw_range sw_ranges[] = {{"psg.env_level", 15u}, {"psg.env_cycles", 15u}, {"psg.env_dir", 1u}, {"psg.env_held", 1u}};

static int sw_in_range(const char *row, const uint8_t *cand, uint32_t w)
{
  size_t i;
  for (i = 0u; i < sizeof sw_ranges / sizeof sw_ranges[0]; i++) {
    if (strcmp(sw_ranges[i].row, row) == 0) {
      return sw_get(cand, w) <= sw_ranges[i].max;
    }
  }
  return 0;
}

/* The zero-coincidence guard (1): the bytes of the row at the cut differ from the row in the power-on state of the program (a witness at which the row
 * is default is no witness). Two cases cannot differ from the power-on state by nature and are held to the zero row of a fresh target instead: phase
 * (POWERED at every cut) and every row at the cut of tick 0, which is the power-on state (HP-RESETPEND); for those the row must be non-zero. */
static int sw_guard(const ivs_digest_field *f, const sw_cut *cx, const uint8_t *cur, uint32_t len, int *equals_po)
{
  const uint8_t *po = (const uint8_t *)cx->po.m + f->offset;
  uint8_t zero[SW_ROW_MAX];
  memset(zero, 0, len);
  *equals_po = memcmp(cur, po, len) == 0;
  if (!*equals_po) {
    return 1;
  }
  return memcmp(cur, zero, len) != 0 && (cx->tick == 0u || strcmp(f->name, "phase") == 0);
}

static int sw_row(const ivs_digest_field *f, sw_cut *cx, sw_out *o)
{
  uint8_t cur[SW_ROW_MAX];
  uint8_t cand[SW_ROW_MAX];
  uint8_t seen[24][SW_ROW_MAX];
  uint32_t nseen = 0u;
  uint32_t len = f->width * f->count;
  uint32_t pass;
  const struct ivs_machine *t0 = cx->t0.m;
  memset(o, 0, sizeof *o);
  if (len > SW_ROW_MAX) {
    return 0;
  }
  memcpy(cur, (const uint8_t *)cx->src.m + f->offset, len);
  o->guard = sw_guard(f, cx, cur, len, &o->equals_po);
  /* (2) the sweep values: first those that pass every rule of a restore, then those that break only a derived rule (or lie inside the range
   * of a row the validator relates to a neighbour); a behaviour divergence ends the search, a report-only one is noted and the search goes on,
   * because the report may only echo the row while another sweep value shows that something reads it */
  for (pass = 0u; pass < 2u; pass++) {
    uint32_t runs = 0u;
    uint32_t k;
    for (k = 0u;; k++) {
      int g = sw_cand(f->width, f->count, cur, k, cand);
      ivs_snap_probe pr;
      uint32_t inv;
      uint32_t i;
      sc_leg l2;
      uint32_t d;
      int dup = 0;
      int safe;
      if (g < 0) {
        break;
      }
      if (g == 0 || memcmp(cand, cur, len) == 0) {
        continue;
      }
      for (i = 0u; i < nseen && !dup; i++) {
        dup = memcmp(seen[i], cand, len) == 0;
      }
      if (dup) {
        continue;
      }
      memcpy(cx->scratch, t0, IVS_MACHINE_BYTES);
      memcpy(cx->scratch + f->offset, cand, len);
      ivs_snapshot_probe_from_instance((const struct ivs_machine *)(const void *)cx->scratch, &pr);
      inv = ivs_snapshot_invariants(&pr, t0->profile);
      safe = pass == 0u ? inv == 0u : (inv >= 2u || (f->count == 1u && sw_in_range(f->name, cand, f->width)));
      if (!safe) {
        continue;
      }
      if (nseen < 24u) {
        memcpy(seen[nseen++], cand, len);
      }
      o->valid++;
      o->full += (uint32_t)(inv == 0u);
      /* continue to the end tick on a copy of the target with the row replaced; the host resubmits its events from the count of the first leg */
      memset(&l2, 0, sizeof l2);
      (void)sc_run_leg((ivs_machine *)(void *)cx->scratch, cx->end, cx->ev, cx->nev, cx->l1.consumed, &l2);
      d = sw_diverge(cx, (const ivs_machine *)(const void *)cx->scratch, &l2, f->offset, len);
      sc_leg_free(&l2);
      o->tried++;
      if ((d & SW_BEHAVIOUR) != 0u) {
        o->behaviour = 1;
        o->kinds = d;
        o->cand = k;
        return 1;
      }
      if (d != 0u && !o->report) {
        o->report = 1;
        o->kinds = d;
        o->cand = k;
      }
      if (++runs >= SW_RUNS_PER_PASS) {
        break;
      }
    }
  }
  return 1;
}

/* ---- the rows against the checklist ---- */

static const ivs_digest_field *sw_field(const char *name)
{
  size_t i;
  size_t n = ivs_machine_digest_field_count();
  for (i = 0u; i < n; i++) {
    const ivs_digest_field *f = ivs_machine_digest_field(i);
    if (f != NULL && f->snap == IVS_SNAP_STATE && strcmp(f->name, name) == 0) {
      return f;
    }
  }
  return NULL;
}

/* The outcome of one listed row against its expectation; 1 on agreement. ROW: some sweep value changes behaviour or a public report, and the row is
 * not default at its cut. EXEMPT: no sweep value changes behaviour (a public report that only echoes the row is allowed: the argument is in the
 * checklist narrative); the guard is not asked of an exempt row. */
static int sw_check_row(const sw_listed *l, int full, int verbose)
{
  const ivs_digest_field *f = sw_field(l->row);
  const sc_cut *cut = sc_cut_find(l->witness);
  sw_cut *cx;
  sw_out o;
  int exempt = strncmp(l->sweep, "EXEMPT:", 7) == 0;
  int ok = 1;
  if (f == NULL || cut == NULL) {
    ivs_test_diag("row %s: no such STATE row of the field table or no cut %s", l->row, l->witness);
    return 0;
  }
  cx = sw_cut_get(cut, full);
  if (cx == NULL || !sw_row(f, cx, &o)) {
    ivs_test_diag("row %s: the sweep could not be set up at %s", l->row, l->witness);
    return 0;
  }
  if (verbose) {
    ivs_test_diag("row %s at %s (%s): %s, %u runs of %u safe sweep values (%u pass every rule), candidate %u, end tick %llu%s%s", l->row, l->witness,
                  exempt ? "EXEMPT" : "ROW", o.behaviour ? "diverges in behaviour" : (o.report ? "differs in a public report only" : "inert"),
                  (unsigned)o.tried, (unsigned)o.valid, (unsigned)o.full, (unsigned)o.cand, (unsigned long long)cx->end,
                  o.equals_po ? ", equals the power-on row" : "", o.guard ? "" : ", default at the cut");
  }
  if (o.valid == 0u) {
    ivs_test_diag("row %s has no safe sweep value at %s", l->row, l->witness);
    return 0;
  }
  if (exempt) {
    if (o.behaviour) {
      ivs_test_diag("row %s is exempt but diverged at %s", l->row, l->witness);
      ok = 0;
    }
  } else {
    if (!o.behaviour && !o.report) {
      ivs_test_diag("row %s survived omission at %s", l->row, l->witness);
      ok = 0;
    }
    if (!o.guard) {
      ivs_test_diag("row %s is default at %s: the witness is no witness", l->row, l->witness);
      ok = 0;
    }
  }
  return ok;
}

/* state.sweep.row-omission: every STATE row, at its witness cut. A fault run takes every 16th row and a one-frame window (the id is the
 * must-fail id of no fault, D-40); a normal run takes them all with the whole window. */
static int sw_sweep_all(void)
{
  size_t i;
  size_t n = 0u;
  size_t bad = 0u;
  size_t rows_run = 0u;
  size_t exempt = 0u;
  if (!sw_merge()) {
    return IVS_TEST_FAIL;
  }
  for (i = 0u; i < SW_N; i++) {
    const sw_listed *l = &sw_eff[i];
    if (ST_FAULT_STRIDE > 1u && n++ % 16u != 0u) {
      continue;
    }
    rows_run++;
    exempt += (size_t)(strncmp(l->sweep, "EXEMPT:", 7) == 0);
    if (!sw_check_row(l, 0, 1)) {
      bad++;
    }
  }
  ivs_test_diag("%u STATE rows swept (%u exempt), %u disagreements with the checklist", (unsigned)rows_run, (unsigned)exempt, (unsigned)bad);
  sw_free_all();
  return bad == 0u && rows_run > 0u ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static const sw_listed *sw_find(const char *row)
{
  size_t i;
  for (i = 0u; i < SW_N; i++) {
    if (strcmp(sw_eff[i].row, row) == 0) {
      return &sw_eff[i];
    }
  }
  return NULL;
}

/* The result of a row for the judge: 1 diverged, 0 inert, -1 not run. */
static int sw_result(const char *row, const char *cut_id)
{
  const ivs_digest_field *f = sw_field(row);
  const sc_cut *cut = sc_cut_find(cut_id);
  sw_cut *cx = cut != NULL ? sw_cut_get(cut, 0) : NULL;
  sw_out o;
  if (f == NULL || cx == NULL || !sw_row(f, cx, &o) || o.valid == 0u) {
    return -1;
  }
  return o.behaviour ? 1 : 0;
}

/* state.judge.row-sweep-detects-omission (D-16): the sweep is run on a row known to be read, cpu.r at HP-SDBD (the program loads a value into R1 and
 * adds R3 to R2, so a wrong register changes the final state), and on a row of the checklist that is exempt: the first must be reported as
 * divergent and the second as inert, so the sweep neither calls every row divergent nor misses a read one. */
static int sw_judge(void)
{
  size_t i;
  size_t judged = 0u;
  int ok = 1;
  int r;
  ST_CHECK(sw_merge());
  r = sw_result("cpu.r", "HP-SDBD");
  ST_REQ(ok, r == 1);
  for (i = 0u; i < SW_N && judged < 1u; i++) {
    if (strncmp(sw_eff[i].sweep, "EXEMPT:", 7) == 0) {
      const sw_listed *l = sw_find(sw_eff[i].row);
      r = l != NULL ? sw_result(l->row, l->witness) : -1;
      ivs_test_diag("the exempt row %s at %s is reported %s", l->row, l->witness, r == 0 ? "inert" : "divergent");
      ST_REQ(ok, r == 0);
      judged++;
    }
  }
  ST_REQ(ok, judged == 1u); /* a checklist with no exempt row cannot show that the sweep can report one */
  sw_free_all();
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_case ivs_state_sweep_cases[] = {
    {"state.sweep.row-omission", sw_sweep_all, NULL},
    {"state.judge.row-sweep-detects-omission", sw_judge, NULL},
};
const size_t ivs_state_sweep_count = sizeof ivs_state_sweep_cases / sizeof ivs_state_sweep_cases[0];
