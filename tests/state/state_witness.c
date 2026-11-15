/* SPDX-License-Identifier: MIT */
/* Suite state, family witness (plan 08-14, D-17, 08-DISCUSS-state 5.1, hazard H17): the witness scan and the ids that re-prove every pinned
 * tick of data/expected/state-cuts.ids.
 * A cut is a unit boundary (the instance state after a unit, the only place the run call can stop). For each cut the scan steps a powered
 * instance of the cut's program one unit at a time and records the first boundary at or after the occurrence rule at which the predicate of
 * docs/state/hard-points.md holds over the instance (and, for a few cuts, over the unit just run: its kind and events, which only the internal
 * step seam reports, so a program with input events is scanned through the public run call with its events resubmitted and its predicates are
 * instance-only). The tick found is pinned in data/expected/state-cuts.ids (self-recorded, repeatability only); the ids below re-prove each pin:
 * the scan's first occurrence equals the pin, and a fresh instance run to the pin through the public run call ends exactly on that boundary,
 * with the same bytes as the scan's instance, at a boundary where the instance part of the predicate holds. A literal that drifts fails there.
 *   state.witness.cpu-points     the eleven cpu cuts
 *   state.witness.stic-points    the three stic cuts
 *   state.witness.device-points  the four device cuts */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "machine/machine.h"
#include "state_cases.h"
#include "state_util.h"
#include "stic/stic.h"
#include "stic/stic_params.h"

/* What the scan knows about the unit it ran last (valid only when the instance is stepped through the internal seam). */
typedef struct sc_ctx {
  ivs_ms_inst x;
  const sc_run *run;
  const ivs_input_event *ev;
  uint32_t nev;
  uint32_t consumed;
  int seam;                 /* 1: stepped through the internal step seam, the unit result is known */
  int have_last;            /* a unit has run */
  ivs_cp1610_result last;   /* the unit just run (seam only) */
  uint8_t prev_kind;        /* the kind of the unit before it (seam only) */
  uint8_t prev_busak;       /* busak before the unit just run */
  uint16_t prev_r6;         /* R6 before the unit just run */
  uint64_t prev_log_count;  /* obs.psglog.count before the unit just run */
  uint64_t units;
  uint8_t *probe;           /* the copy of the instance a lookahead steps (the next unit), IVS_MACHINE_BYTES, 16 aligned */
} sc_ctx;

static const struct ivs_machine *sc_im(const sc_ctx *c)
{
  return c->x.m;
}

static int sc_ctx_open(sc_ctx *c, const sc_run *run)
{
  memset(c, 0, sizeof *c);
  c->run = run;
  c->ev = sc_events(run, &c->nev);
  c->seam = c->nev == 0u;
  if (!sc_new(&c->x, run, 1)) {
    return 0;
  }
  return 1;
}

static void sc_ctx_close(sc_ctx *c)
{
  free(c->probe);
  ivs_ms_free(&c->x);
  memset(c, 0, sizeof *c);
}

/* One unit: through the seam (the result kept) or through the public call (one unit, the events resubmitted). 1 on success. */
static int sc_ctx_step(sc_ctx *c)
{
  const struct ivs_machine *im = sc_im(c);
  c->prev_busak = im->cpu.busak;
  c->prev_r6 = im->cpu.r[6];
  c->prev_log_count = im->obs.psglog.count;
  c->prev_kind = c->have_last ? c->last.unit_kind : 0u;
  if (c->seam) {
    ivs_cp1610_result r;
    memset(&r, 0, sizeof r);
    if (ivs_machine_step_unit_internal(c->x.m, &r) != IVS_OK) {
      return 0;
    }
    c->last = r;
  } else if (!su_run_unit(c->x.m, c->ev, c->nev, &c->consumed)) {
    return 0;
  }
  c->have_last = 1;
  c->units++;
  return 1;
}

/* The line levels the next unit samples (the lines are sampled one profile lag before the unit start), or zeros below the lag. */
static ivs_stic_line_levels sc_next_lines(const struct ivs_machine *m)
{
  const ivs_stic_params *sp = ivs_stic_params_get(m->profile);
  ivs_stic_line_levels l;
  l.intrm = 0u;
  l.busrq = 0u;
  if (sp != NULL && m->time_ticks >= (uint64_t)sp->line_sample_lag_ticks) {
    l = ivs_stic_lines(sp, &m->stic, m->time_ticks - (uint64_t)sp->line_sample_lag_ticks);
  }
  return l;
}

/* The unit that would run next from this instance, found by stepping a copy of it through the seam (1 when it stepped; *r is the unit result). */
static int sc_next_result(sc_ctx *c, ivs_cp1610_result *r)
{
  ivs_machine *pm;
  if (c->probe == NULL) {
    c->probe = (uint8_t *)aligned_alloc(IVS_MACHINE_ALIGN, IVS_MACHINE_BYTES);
    if (c->probe == NULL) {
      return 0;
    }
  }
  memcpy(c->probe, c->x.storage, IVS_MACHINE_BYTES);
  pm = (ivs_machine *)(void *)c->probe;
  memset(r, 0, sizeof *r);
  return ivs_machine_step_unit_internal(pm, r) == IVS_OK;
}

static uint32_t sc_next_unit_kind(sc_ctx *c)
{
  ivs_cp1610_result r;
  return sc_next_result(c, &r) ? (uint32_t)r.unit_kind : 0u;
}

/* ---- the predicates of docs/state/hard-points.md ----
 * Each cut has a state part (a function of the instance, which may lean on a copy stepped once) and a unit part (a function of the unit just
 * run: kind, events, the previous unit; seam only). ctx->have_last is 0 for the instance of a run that ended at the pin through the public
 * call, where only the state part can be evaluated. */

typedef struct sc_def {
  const char *id;
  const char *after;  /* the cut whose first occurrence the scan starts after, or NULL */
  int (*state)(sc_ctx *c);
  int (*unit)(const sc_ctx *c); /* NULL when the cut needs nothing from the unit just run */
  int seam_only;                /* 1: the unit part reads the unit result, known only when stepped through the seam */
  const char *rule;
} sc_def;

static int sc_st_sdbd(sc_ctx *c)
{
  const struct ivs_machine *m = sc_im(c);
  return m->cpu.dbd == 1u && m->cpu.run_state == IVS_CP1610_RS_RUNNING;
}

/* HP-STEAL-A: an open boundary (prev_interruptible 1, BUSAK down) in a displayed frame at which BUSRQ is asserted for the next unit, and the next unit (a
 * copy stepped through the seam) is a bus grant: the boundary just before the first grant of the run. Plan 08-14 moved this cut from the unit after the grant
 * to the boundary before it: the sweep showed that a lost BUSAK is harmless after the grant (BUSRQ is still asserted, so the grant is taken again) and that a lost
 * prev_interruptible matters exactly here (a closed boundary does not test BUSRQ, so the grant comes late and the run report counts other units). */
static int sc_st_steal_a(sc_ctx *c)
{
  const struct ivs_machine *m = sc_im(c);
  if (m->cpu.run_state != IVS_CP1610_RS_RUNNING || m->cpu.busak != 0u || m->cpu.prev_interruptible != 1u || m->stic.frame_index < 1u ||
      sc_next_lines(m).busrq == 0u) {
    return 0;
  }
  return sc_next_unit_kind(c) == IVS_CP1610_UNIT_BUS_GRANT;
}

/* HP-STEAL-B: mid-hold with BUSAK up, at least one window of the frame already granted, BUSRQ low for the next unit and the next unit (a copy stepped) the
 * release: the boundary just before a release. Plan 08-14 moved this cut from the first mid-hold unit to the boundary before the release: a lost BUSAK matters
 * exactly here (the release unit would not run and the window would not be counted as granted). */
static int sc_st_steal_b(sc_ctx *c)
{
  const struct ivs_machine *m = sc_im(c);
  ivs_cp1610_result r;
  if (m->cpu.busak != 1u || m->stic.grants_in_frame < 1u || sc_next_lines(m).busrq != 0u) {
    return 0;
  }
  return sc_next_result(c, &r) && r.unit_kind == IVS_CP1610_UNIT_BUS_GRANT && (r.events & IVS_CP1610_EV_BUS_RELEASED) != 0u;
}

/* HP-STEAL-C: the release unit (BUS_RELEASED), BUSAK down, prev_interruptible still 1 (a grant never changes it), a window granted. */
static int sc_st_steal_c(sc_ctx *c)
{
  const struct ivs_machine *m = sc_im(c);
  return m->cpu.busak == 0u && m->cpu.prev_interruptible == 1u && m->stic.grants_in_frame >= 1u;
}
static int sc_un_steal_c(const sc_ctx *c)
{
  return (c->last.events & IVS_CP1610_EV_BUS_RELEASED) != 0u;
}

/* HP-OPEN: an open boundary with the interrupt enable set at which the unit stepped next (on a copy) is an interrupt entry. */
static int sc_st_open(sc_ctx *c)
{
  const struct ivs_machine *m = sc_im(c);
  if (m->cpu.prev_interruptible != 1u || m->cpu.intff != 1u || m->cpu.run_state != IVS_CP1610_RS_RUNNING) {
    return 0;
  }
  if (sc_next_lines(m).intrm == 0u) {
    return 0; /* no interrupt can be due: nothing to step */
  }
  return sc_next_unit_kind(c) == IVS_CP1610_UNIT_INT_ENTRY;
}

/* HP-CLOSED: a closed boundary (prev_interruptible 0) after an instruction, INTRM asserted for the next unit, the display latch set by the
 * strobe of the interrupt routine and the window decision still ahead. */
static int sc_st_closed(sc_ctx *c)
{
  const struct ivs_machine *m = sc_im(c);
  return m->cpu.prev_interruptible == 0u && m->cpu.run_state == IVS_CP1610_RS_RUNNING && m->stic.display_latch == 1u &&
         m->stic.windows_active == 0u && sc_next_lines(m).intrm != 0u;
}
static int sc_un_closed(const sc_ctx *c)
{
  return c->last.unit_kind == IVS_CP1610_UNIT_INSTR;
}

/* HP-INT-ENTRY: the unit just run was an interrupt entry: R6 one above where it was, R7 the vector, a closed boundary. */
static int sc_st_int_entry(sc_ctx *c)
{
  const struct ivs_machine *m = sc_im(c);
  return m->cpu.r[7] == 0x1004u && m->cpu.prev_interruptible == 0u;
}
static int sc_un_int_entry(const sc_ctx *c)
{
  const struct ivs_machine *m = sc_im(c);
  return c->last.unit_kind == IVS_CP1610_UNIT_INT_ENTRY && m->cpu.r[6] == (uint16_t)(c->prev_r6 + 1u);
}

static int sc_st_hlt(sc_ctx *c)
{
  const struct ivs_machine *m = sc_im(c);
  return m->cpu.halt_pending == 1u && m->cpu.run_state == IVS_CP1610_RS_RUNNING;
}

/* HP-HALTED: halted, and BUSRQ asserted for the next unit, so that unit is a bus grant. */
static int sc_st_halted(sc_ctx *c)
{
  const struct ivs_machine *m = sc_im(c);
  return m->cpu.run_state == IVS_CP1610_RS_HALTED && sc_next_lines(m).busrq != 0u;
}

static int sc_st_faulted(sc_ctx *c)
{
  const struct ivs_machine *m = sc_im(c);
  return m->cpu.run_state == IVS_CP1610_RS_FAULTED && m->cpu.fault_reason != 0u;
}

static int sc_st_resetpend(sc_ctx *c)
{
  const struct ivs_machine *m = sc_im(c);
  return m->cpu.run_state == IVS_CP1610_RS_RESET_PENDING && m->time_ticks == 0u;
}


/* ---- the stic and device cuts ---- */

/* The event cursor of a frame in progress: after the fetch of row 5 (index 8 applied, the cursor at 9) and before FRAME_END (index 15). */
#define SC_CURSOR_MID(m) ((m)->stic.event_index >= 9u && (m)->stic.event_index <= 15u)

/* HP-FRAME-MID: a displayed frame at least 1, windows decided, the fetches of rows 0 to 5 applied and FRAME_END not, the interrupt enable set. */
static int sc_st_frame_mid(sc_ctx *c)
{
  const struct ivs_machine *m = sc_im(c);
  return m->stic.frame_index >= 1u && m->stic.display_latch == 1u && m->stic.windows_active == 1u && SC_CURSOR_MID(m) && m->cpu.intff == 1u;
}

/* HP-LATCH: a Color Stack frame (mode 1, pointer nonzero) with row 0 fetched, whose latched first word is no longer the BACKTAB word. */
static int sc_st_latch(sc_ctx *c)
{
  const struct ivs_machine *m = sc_im(c);
  return m->stic.frame_index >= 1u && m->stic.mode == IVS_STIC_MODE_COLOR_STACK && m->stic.cs_ptr != 0u && m->stic.display_latch == 1u &&
         m->stic.windows_active == 1u && m->stic.event_index >= 4u && m->stic.row_latch[0][0] != m->mem.sysram[0];
}

/* HP-MOB: mid-frame with a MOB, border or delay register (words 0x00 to 0x17) and a collision register (words 0x18 to 0x1F) nonzero. */
static int sc_st_mob(sc_ctx *c)
{
  const struct ivs_machine *m = sc_im(c);
  uint32_t i;
  uint16_t mob = 0u;
  uint16_t coll = 0u;
  for (i = 0u; i < 0x18u; i++) {
    mob = (uint16_t)(mob | m->mem.stic_reg[i]);
  }
  for (i = 0x18u; i < 0x20u; i++) {
    coll = (uint16_t)(coll | m->mem.stic_reg[i]);
  }
  return m->stic.frame_index >= 1u && m->stic.windows_active == 1u && SC_CURSOR_MID(m) && mob != 0u && coll != 0u;
}

/* HP-PSG-MID: both tone periods rewritten, the noise and tone A counters nonzero and the tone outputs of A and B different. */
static int sc_st_psg_mid(sc_ctx *c)
{
  const struct ivs_machine *m = sc_im(c);
  return m->mem.psg_reg[0] == 0x31u && m->mem.psg_reg[1] == 0x57u && m->psg.noise_counter != 0u && m->psg.tone_counter[0] != 0u &&
         m->psg.tone_out[0] != m->psg.tone_out[1];
}

/* HP-ENV-MID: a repeating envelope in progress on a rising edge (the direction flag 1; plan 08-14 changed the shape of P-PSG to a triangle so that the row is
 * non-default): not held, a cycle index and a level between 1 and 14. */
static int sc_st_env_mid(sc_ctx *c)
{
  const struct ivs_machine *m = sc_im(c);
  return m->psg.env_held == 0u && m->psg.env_dir == 1u && m->psg.env_cycles >= 1u && m->psg.env_cycles <= 14u && m->psg.env_level >= 1u &&
         m->psg.env_level <= 14u;
}

/* HP-INPUT: one event applied, a pull-down in force, the floor above 0 and one event of the host's array still to be submitted. */
static int sc_st_input(sc_ctx *c)
{
  const struct ivs_machine *m = sc_im(c);
  return m->obs.input.count == 1u && (m->mem.port_pulldown[0] | m->mem.port_pulldown[1]) != 0u && m->obs.input.floor_tick > 0u &&
         c->consumed < c->nev;
}

/* HP-LOG: the log holds records and the unit just run wrote one (the count moved during it): a write whose stamp lies in the last unit. */
static int sc_st_log(sc_ctx *c)
{
  return sc_im(c)->obs.psglog.count > 0u;
}
static int sc_un_log(const sc_ctx *c)
{
  return sc_im(c)->obs.psglog.count > c->prev_log_count;
}

static const sc_def sc_defs[] = {
    {"HP-SDBD", NULL, sc_st_sdbd, NULL, 0,
     "the first boundary with cpu.dbd 1 and the CPU RUNNING (the SDBD unit has just run); arithmetic: reset entry 5 + MVII 8 + MVO 11 + SDBD 4 = 28 cycles, tick 112"},
    {"HP-STEAL-A", NULL, sc_st_steal_a, NULL, 0,
     "the first open boundary in a displayed frame (frame at least 1) at which BUSRQ is asserted and the next unit is a bus grant (the boundary before the first grant)"},
    {"HP-STEAL-B", "HP-STEAL-A", sc_st_steal_b, NULL, 0,
     "the first boundary after HP-STEAL-A with BUSAK up, a window of the frame already granted, BUSRQ low and the next unit the release (the boundary before a release)"},
    {"HP-STEAL-C", "HP-STEAL-B", sc_st_steal_c, sc_un_steal_c, 1,
     "the first release unit (BUS_RELEASED) after HP-STEAL-B, BUSAK down, prev_interruptible 1"},
    {"HP-OPEN", NULL, sc_st_open, NULL, 0,
     "the first open boundary with the interrupt enable set at which the next unit is an interrupt entry"},
    {"HP-CLOSED", "HP-INT-ENTRY", sc_st_closed, sc_un_closed, 1,
     "the first closed boundary after HP-INT-ENTRY that follows an instruction with INTRM asserted, the display latch set and the windows not yet decided"},
    {"HP-INT-ENTRY", NULL, sc_st_int_entry, sc_un_int_entry, 1,
     "the first interrupt entry unit of the run: R6 one above its earlier value, R7 $1004, a closed boundary"},
    {"HP-HLT", NULL, sc_st_hlt, NULL, 0,
     "the first boundary with the stop latch set and the CPU RUNNING (the HLT unit has just run); arithmetic: 93 cycles, tick 372"},
    {"HP-HALTED", NULL, sc_st_halted, NULL, 0,
     "the first boundary with the CPU HALTED and BUSRQ asserted for the next unit (a bus window due while halted)"},
    {"HP-FAULTED", NULL, sc_st_faulted, NULL, 0,
     "the first boundary with the CPU FAULTED and a nonzero fault reason; arithmetic: 135 cycles, tick 540"},
    {"HP-RESETPEND", NULL, sc_st_resetpend, NULL, 0, "the boundary at tick 0 right after the power-on reset: the CPU RESET_PENDING, no unit run"},
    {"HP-FRAME-MID", NULL, sc_st_frame_mid, NULL, 0,
     "the first boundary of a displayed frame at least 1 with the windows decided, the event cursor between 9 and 15 (rows 0 to 5 fetched, FRAME_END not) and the interrupt enable set"},
    {"HP-LATCH", NULL, sc_st_latch, NULL, 0,
     "the first boundary of a Color Stack frame at least 1 after the fetch of row 0 at which the latched first word differs from the BACKTAB word"},
    {"HP-MOB", NULL, sc_st_mob, NULL, 0,
     "the first mid-frame boundary of a frame at least 1 with a MOB, border or delay register and a collision register nonzero"},
    {"HP-PSG-MID", NULL, sc_st_psg_mid, NULL, 0,
     "the first boundary after both tone periods were rewritten with the noise and tone A counters nonzero and the tone outputs of A and B different"},
    {"HP-ENV-MID", NULL, sc_st_env_mid, NULL, 0,
     "the first boundary with the envelope not held, rising (direction 1), a cycle index and a level between 1 and 14"},
    {"HP-INPUT", NULL, sc_st_input, NULL, 0,
     "the first boundary with one event applied, a pull-down in force, the floor above 0 and one event still to be submitted"},
    {"HP-LOG", NULL, sc_st_log, sc_un_log, 0,
     "the first boundary at which the PSG log holds records and the unit just run appended one"},
};

static const sc_def *sc_def_find(const char *id)
{
  size_t i;
  for (i = 0u; i < sizeof sc_defs / sizeof sc_defs[0]; i++) {
    if (strcmp(sc_defs[i].id, id) == 0) {
      return &sc_defs[i];
    }
  }
  return NULL;
}

const char *sc_cut_rule(const sc_cut *c)
{
  const sc_def *d = sc_def_find(c->id);
  return d != NULL ? d->rule : "(no predicate)";
}

/* ---- the scan ---- */

/* The whole predicate at the boundary the context is at. */
static int sc_holds(sc_ctx *c, const sc_def *d)
{
  if (d->unit != NULL && (!c->have_last || (d->seam_only && !c->seam) || !d->unit(c))) {
    return 0;
  }
  return d->state(c);
}

static int sc_find_from(const sc_def *d, const sc_cut *cut, uint64_t after_tick, int has_after, uint64_t *tick, uint64_t *units, uint8_t *storage)
{
  const sc_run *run = sc_run_find(cut->program);
  uint64_t end = sc_cut_end(cut);
  sc_ctx c;
  int found = 0;
  if (run == NULL || !sc_ctx_open(&c, run)) {
    return 0;
  }
  for (;;) {
    const struct ivs_machine *im = sc_im(&c);
    if ((!has_after || im->time_ticks > after_tick) && sc_holds(&c, d)) {
      found = 1;
      break;
    }
    if (im->time_ticks >= end || (im->cpu.run_state == IVS_CP1610_RS_FAULTED && c.have_last)) {
      break;
    }
    if (!sc_ctx_step(&c)) {
      break;
    }
  }
  if (found) {
    *tick = sc_im(&c)->time_ticks;
    *units = c.units;
    if (storage != NULL) {
      memcpy(storage, c.x.storage, IVS_MACHINE_BYTES);
    }
  }
  sc_ctx_close(&c);
  return found;
}

int sc_witness_find(const sc_cut *cut, uint64_t *tick, uint64_t *units, uint8_t *storage)
{
  const sc_def *d = sc_def_find(cut->id);
  uint64_t after = 0u;
  uint64_t after_units = 0u;
  if (d == NULL) {
    return 0;
  }
  if (d->after != NULL) {
    const sc_cut *a = sc_cut_find(d->after);
    if (a == NULL || !sc_witness_find(a, &after, &after_units, NULL)) {
      return 0;
    }
    return sc_find_from(d, cut, after, 1, tick, units, storage);
  }
  return sc_find_from(d, cut, 0u, 0, tick, units, storage);
}

static uint64_t m_count(const ivs_machine *m)
{
  return ((const struct ivs_machine *)m)->obs.input.count;
}

int sc_witness_state_holds(const sc_cut *cut, const ivs_machine *m)
{
  const sc_def *d = sc_def_find(cut->id);
  sc_ctx c;
  int r;
  if (d == NULL) {
    return 0;
  }
  /* A context around the given instance: the state parts read c->x.m and may step a copy of it, never the instance itself. */
  memset(&c, 0, sizeof c);
  c.x.m = (ivs_machine *)(uintptr_t)m;
  c.x.storage = (uint8_t *)(uintptr_t)m;
  c.run = sc_run_find(cut->program);
  if (c.run != NULL) {
    c.ev = sc_events(c.run, &c.nev); /* a host that resubmits its events from the applied count: the count is the instance's */
  }
  c.consumed = (uint32_t)m_count(m);
  r = d->state(&c);
  free(c.probe);
  return r;
}

/* ---- the ids ---- */

/* One cut: the scan's first occurrence equals the pin, a fresh instance run to the pin through the public call ends exactly there with the
 * scan's bytes, and the state part of the predicate holds on the instance the run call made. 1 on success. */
static int sc_witness_cut(const sc_cut *cut, uint8_t *scan_storage)
{
  const sc_run *run = sc_run_find(cut->program);
  uint64_t tick = 0u;
  uint64_t units = 0u;
  uint64_t end = sc_cut_end(cut);
  ivs_ms_inst x;
  const ivs_input_event *ev;
  uint32_t nev = 0u;
  uint32_t consumed = 0u;
  int ok = 1;
  if (run == NULL) {
    ivs_test_diag("cut %s: no program %s", cut->id, cut->program);
    return 0;
  }
  if (!sc_witness_find(cut, &tick, &units, scan_storage)) {
    ivs_test_diag("cut %s: the predicate never holds before tick %llu (%s)", cut->id, (unsigned long long)end, sc_cut_rule(cut));
    return 0;
  }
  ivs_test_diag("cut %s: first occurrence at tick %llu after %llu units, pinned %llu, end %llu (%s)", cut->id, (unsigned long long)tick,
                (unsigned long long)units, (unsigned long long)cut->tick, (unsigned long long)end, sc_cut_rule(cut));
  if (cut->tick == SC_PENDING) {
    ivs_test_diag("cut %s: the tick is not pinned yet", cut->id);
    return 0;
  }
  if (tick != cut->tick) {
    ivs_test_diag("cut %s: the pinned tick %llu is not the first occurrence %llu: %s", cut->id, (unsigned long long)cut->tick,
                  (unsigned long long)tick, sc_cut_rule(cut));
    ok = 0;
  }
  if (cut->tick >= end) {
    ivs_test_diag("cut %s: the end tick %llu is not past the pinned tick %llu", cut->id, (unsigned long long)end, (unsigned long long)cut->tick);
    ok = 0;
  }
  if (!sc_new(&x, run, 1)) {
    return 0;
  }
  ev = sc_events(run, &nev);
  if (cut->tick > 0u && !su_run_to(x.m, cut->tick, ev, nev, &consumed)) {
    ivs_test_diag("cut %s: the run call refused", cut->id);
    ivs_ms_free(&x);
    return 0;
  }
  {
    const struct ivs_machine *im = x.m;
    if (im->time_ticks != cut->tick) {
      ivs_test_diag("cut %s: the run to the pin ended at %llu, not on the boundary %llu", cut->id, (unsigned long long)im->time_ticks,
                    (unsigned long long)cut->tick);
      ok = 0;
    }
  }
  if (memcmp(x.storage, scan_storage, IVS_MACHINE_BYTES) != 0) {
    ivs_test_diag("cut %s: the instance the run call made at the pin differs from the instance of the scan", cut->id);
    ok = 0;
  }
  if (!sc_witness_state_holds(cut, x.m)) {
    ivs_test_diag("cut %s: the state part of the predicate does not hold at the pin (%s)", cut->id, sc_cut_rule(cut));
    ok = 0;
  }
  ivs_ms_free(&x);
  return ok;
}

static int sc_witness_family(const char *family)
{
  uint8_t *storage = (uint8_t *)malloc(IVS_MACHINE_BYTES);
  size_t i;
  size_t n = 0u;
  int ok = 1;
  if (storage == NULL) {
    return IVS_TEST_FAIL;
  }
  for (i = 0u; i < sc_cut_count(); i++) {
    const sc_cut *c = sc_cut_at(i);
    if (strcmp(c->family, family) != 0) {
      continue;
    }
    n++;
    /* the fault-run reduced point set: every fourth cut (the must-fail id of no fault, D-40) */
    if (ST_FAULT_STRIDE > 1u && (n - 1u) % 4u != 0u) {
      continue;
    }
    ST_REQ(ok, sc_witness_cut(c, storage));
  }
  free(storage);
  ST_REQ(ok, n > 0u);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int sc_witness_cpu(void)
{
  return sc_witness_family("cpu-points");
}

static int sc_witness_stic(void)
{
  return sc_witness_family("stic-points");
}

static int sc_witness_device(void)
{
  return sc_witness_family("device-points");
}

const ivs_test_case ivs_state_witness_cases[] = {
    {"state.witness.cpu-points", sc_witness_cpu, NULL},
    {"state.witness.stic-points", sc_witness_stic, NULL},
    {"state.witness.device-points", sc_witness_device, NULL},
};
const size_t ivs_state_witness_count = sizeof ivs_state_witness_cases / sizeof ivs_state_witness_cases[0];
