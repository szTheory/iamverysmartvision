/* SPDX-License-Identifier: MIT */
/* Suite stic, family events, reset and slice (plan 06-06; D-05, D-07, D-10, A-06): the 16-entry frame timeline and its
 * cursor. Module-level cases take a stack copy of the generated parameters of the registry profile and perturb it, so
 * every expectation is a RELATION to the parameters (an edge equals the table tick of its event, a perturbed table
 * moves the observed edge by exactly the delta, the frame index is a count of FRAME_END events), never a policy
 * number (D-02). The machine-path cases step a stand-in program (program A, a loop of MVII, MVO, INCR, B decles
 * derived in tests/machine/machine_standin.c) through the internal step seam.
 *   stic.events.table-order                 kinds and ticks of the table, applied in order by the catch-up
 *   stic.events.edges                       the gates close at exactly the table tick and are open one tick before;
 *                                           a table moved by one CPU cycle moves the observed edge by that cycle
 *   stic.events.frame-index-is-state        a nonzero frame phase: the index is a count of FRAME_END events
 *   stic.events.idempotent                  the same or an earlier target changes nothing
 *   stic.events.latch-cleared-at-frame-start  the latch, windows_active and the gates at FRAME_START, cs_ptr kept
 *   stic.reset.cursor                       a power-on reset returns the cursor, the gates and the state to power-on
 *   stic.slice.state-equal-at-every-unit-boundary  the machine's STIC state equals a copy caught up directly to
 *                                           each unit end, and one big run call equals the unit-by-unit run */
#include <stdint.h>
#include <string.h>

#include "ivs/ivs.h"
#include "machine/machine.h"
#include "machine_cases.h"
#include "stic_cases.h"

static ivs_memsys sc_mem; /* 113 KB: static, the cases run one at a time */

static int sc_init(ivs_stic_params *p, ivs_stic_state *s)
{
  const ivs_stic_params *g = ivs_stic_params_get(SC_PROFILE);
  if (g == NULL || ivs_memsys_init(&sc_mem, SC_PROFILE) != IVS_OK) {
    return 0;
  }
  *p = *g;
  ivs_stic_state_reset(s, p);
  return 1;
}

/* Index of the first table entry of kind k, or the event count when there is none. */
static uint32_t sc_kind_index(const ivs_stic_params *p, uint32_t k)
{
  uint32_t i;
  for (i = 0u; i < p->event_count; i++) {
    if (p->event_kind[i] == k) {
      return i;
    }
  }
  return p->event_count;
}

/* 1 when the gate is open in sc_mem. */
static uint32_t sc_open(uint32_t gate)
{
  return sc_mem.gate_open[gate];
}

/* The first tick of [0, F) at which the gate is closed after a fresh catch-up from tick 0 to it (bisection: within the
 * frame a closed gate stays closed). F when it is never closed. */
static uint64_t sc_edge(const ivs_stic_params *p, uint32_t gate)
{
  uint64_t lo = 0u;
  uint64_t hi = p->frame_ticks;
  while (lo < hi) {
    uint64_t mid = lo + (hi - lo) / 2u;
    ivs_stic_state s;
    ivs_stic_state_reset(&s, p);
    (void)ivs_memsys_init(&sc_mem, SC_PROFILE);
    ivs_stic_catch_up(p, &s, &sc_mem, mid, NULL, NULL);
    if (sc_open(gate) == 0u) {
      hi = mid;
    } else {
      lo = mid + 1u;
    }
  }
  return lo;
}

/* The table is applied in order: after a catch-up to the tick of entry i (every entry at or below it applied) the
 * cursor is the number of entries at or below the tick, and one tick below the first entry nothing has moved. */
static int sc_table_order(void)
{
  ivs_stic_params p;
  ivs_stic_state s;
  uint32_t i;
  SC_CHECK(sc_init(&p, &s));
  SC_CHECK(p.event_count >= 4u && p.event_count <= IVS_STIC_EVENT_MAX);
  SC_CHECK(p.event_kind[0] == IVS_STIC_EV_FRAME_START && p.event_kind[1] == IVS_STIC_EV_REG_CLOSE &&
           p.event_kind[2] == IVS_STIC_EV_GMEM_CLOSE && p.event_kind[p.event_count - 1u] == IVS_STIC_EV_FRAME_END);
  SC_CHECK(p.event_tick[0] == 0u && p.event_tick[p.event_count - 1u] == p.frame_ticks);
  for (i = 1u; i < p.event_count; i++) {
    SC_CHECK(p.event_tick[i] >= p.event_tick[i - 1u]);
  }
  for (i = 0u; i + 1u < p.event_count; i++) {
    uint32_t want = 0u;
    uint32_t j;
    for (j = 0u; j < p.event_count; j++) {
      if (p.event_tick[j] <= p.event_tick[i]) {
        want = j + 1u;
      }
    }
    SC_CHECK(sc_init(&p, &s));
    ivs_stic_catch_up(&p, &s, &sc_mem, p.event_tick[i], NULL, NULL);
    SC_CHECK(s.event_index == want && s.frame_index == 0u);
    if (p.event_tick[i] > 0u) {
      SC_CHECK(sc_init(&p, &s));
      ivs_stic_catch_up(&p, &s, &sc_mem, p.event_tick[i] - 1u, NULL, NULL);
      SC_CHECK(s.event_index < want || p.event_tick[i - 1u] == p.event_tick[i]);
    }
  }
  /* the frame end wraps the cursor and advances the frame index; the next FRAME_START is applied at the same tick */
  SC_CHECK(sc_init(&p, &s));
  ivs_stic_catch_up(&p, &s, &sc_mem, p.frame_ticks, NULL, NULL);
  SC_CHECK(s.frame_index == 1u && s.event_index == 1u);
  return IVS_TEST_PASS;
}

/* The gates close at exactly the table tick, are open one tick before, and a table moved by one CPU cycle moves the
 * observed edge by exactly that cycle (the profile field is a relation, not a literal). */
static int sc_edges(void)
{
  static const uint32_t gates[2] = {IVS_MEMSYS_GATE_STIC_REGS, IVS_MEMSYS_GATE_GMEM};
  static const uint32_t kinds[2] = {IVS_STIC_EV_REG_CLOSE, IVS_STIC_EV_GMEM_CLOSE};
  uint32_t g;
  for (g = 0u; g < 2u; g++) {
    ivs_stic_params p;
    ivs_stic_params q;
    ivs_stic_state s;
    uint32_t idx;
    uint64_t at;
    uint64_t edge;
    uint64_t moved;
    SC_CHECK(sc_init(&p, &s));
    idx = sc_kind_index(&p, kinds[g]);
    SC_CHECK(idx < p.event_count);
    at = p.event_tick[idx];
    SC_CHECK(at > 0u);
    /* open one tick before, closed at the tick */
    ivs_stic_catch_up(&p, &s, &sc_mem, at - 1u, NULL, NULL);
    SC_CHECK(sc_open(gates[g]) == 1u);
    SC_CHECK(sc_init(&p, &s));
    ivs_stic_catch_up(&p, &s, &sc_mem, at, NULL, NULL);
    SC_CHECK(sc_open(gates[g]) == 0u);
    edge = sc_edge(&p, gates[g]);
    SC_CHECK(edge == at);
    /* the profile field of the window equals the table tick (the generator's own relation, pinned here) */
    SC_CHECK(at == (g == 0u ? p.reg_window_ticks : p.gmem_window_ticks));
    /* a table moved by one CPU cycle moves the observed edge by exactly one cycle */
    q = p;
    q.event_tick[idx] += q.ticks_per_cpu_cycle;
    moved = sc_edge(&q, gates[g]);
    SC_CHECK(moved == edge + q.ticks_per_cpu_cycle);
    q = p;
    q.event_tick[idx] -= q.ticks_per_cpu_cycle;
    moved = sc_edge(&q, gates[g]);
    SC_CHECK(moved + q.ticks_per_cpu_cycle == edge);
  }
  return IVS_TEST_PASS;
}

/* A nonzero frame phase: after a catch-up to phase + k x F + x the frame index is k and the cursor is the number of
 * table entries at or below x, so the pair is a COUNT of applied events (never to_tick / F: with this phase and x the
 * division gives k + 1). */
static int sc_frame_index_is_state(void)
{
  ivs_stic_params p;
  ivs_stic_state s;
  uint32_t k;
  uint32_t n = 0u;
  uint32_t i;
  uint64_t x;
  uint64_t to;
  SC_CHECK(sc_init(&p, &s));
  p.frame_phase_ticks = (uint32_t)((2u * (uint64_t)p.frame_ticks) / 3u);
  x = (uint64_t)p.frame_ticks / 2u; /* phase + x is at least F: a division would see the next frame */
  for (i = 0u; i < p.event_count; i++) {
    if (p.event_tick[i] <= x) {
      n = i + 1u;
    }
  }
  SC_CHECK(n > 0u && n < p.event_count);
  for (k = 0u; k < 3u; k++) {
    to = (uint64_t)p.frame_phase_ticks + (uint64_t)k * p.frame_ticks + x;
    SC_CHECK(to / p.frame_ticks != k); /* the premise: a modulo derivation would be wrong here */
    SC_CHECK(sc_init(&p, &s));
    p.frame_phase_ticks = (uint32_t)((2u * (uint64_t)p.frame_ticks) / 3u);
    ivs_stic_catch_up(&p, &s, &sc_mem, to, NULL, NULL);
    SC_CHECK(s.frame_index == k);
    SC_CHECK(s.event_index == n);
    SC_CHECK(s.frame_index * p.event_count + s.event_index == (uint64_t)k * p.event_count + n);
  }
  /* the interval before the first frame start schedules nothing: both gates stay open and the cursor does not move */
  SC_CHECK(sc_init(&p, &s));
  p.frame_phase_ticks = (uint32_t)((2u * (uint64_t)p.frame_ticks) / 3u);
  ivs_stic_catch_up(&p, &s, &sc_mem, (uint64_t)p.frame_phase_ticks - 1u, NULL, NULL);
  SC_CHECK(s.frame_index == 0u && s.event_index == 0u);
  SC_CHECK(sc_open(IVS_MEMSYS_GATE_STIC_REGS) == 1u && sc_open(IVS_MEMSYS_GATE_GMEM) == 1u);
  return IVS_TEST_PASS;
}

/* The same target twice, or an earlier target afterwards, changes nothing. */
static int sc_idempotent(void)
{
  ivs_stic_params p;
  ivs_stic_state s;
  ivs_stic_state before;
  uint8_t gates[IVS_MEMSYS_GATES];
  uint64_t to;
  SC_CHECK(sc_init(&p, &s));
  to = (uint64_t)p.frame_ticks + (uint64_t)p.reg_window_ticks + 7u;
  ivs_stic_catch_up(&p, &s, &sc_mem, to, NULL, NULL);
  before = s;
  memcpy(gates, sc_mem.gate_open, sizeof gates);
  ivs_stic_catch_up(&p, &s, &sc_mem, to, NULL, NULL);
  SC_CHECK(memcmp(&before, &s, sizeof s) == 0 && memcmp(gates, sc_mem.gate_open, sizeof gates) == 0);
  ivs_stic_catch_up(&p, &s, &sc_mem, to - 1000u, NULL, NULL);
  ivs_stic_catch_up(&p, &s, &sc_mem, 0u, NULL, NULL);
  SC_CHECK(memcmp(&before, &s, sizeof s) == 0 && memcmp(gates, sc_mem.gate_open, sizeof gates) == 0);
  return IVS_TEST_PASS;
}

/* The latch can change inside the register window; the graphics-memory close decides windows_active from it; the next
 * FRAME_START clears the latch, windows_active and the per-frame grants and opens both gates; cs_ptr is not cleared. */
static int sc_latch_cleared_at_frame_start(void)
{
  ivs_stic_params p;
  ivs_stic_state s;
  uint32_t gm;
  SC_CHECK(sc_init(&p, &s));
  gm = sc_kind_index(&p, IVS_STIC_EV_GMEM_CLOSE);
  SC_CHECK(gm < p.event_count);
  ivs_stic_catch_up(&p, &s, &sc_mem, (uint64_t)p.event_tick[gm] - 1u, NULL, NULL);
  SC_CHECK(s.windows_active == 0u && sc_open(IVS_MEMSYS_GATE_STIC_REGS) == 0u && sc_open(IVS_MEMSYS_GATE_GMEM) == 1u);
  s.display_latch = 1u; /* what the strobe does inside the register window (machine_bus.c) */
  s.cs_ptr = 3u;
  s.grants_in_frame = 5u;
  ivs_stic_catch_up(&p, &s, &sc_mem, (uint64_t)p.event_tick[gm], NULL, NULL);
  SC_CHECK(s.windows_active == 1u && s.display_latch == 1u && sc_open(IVS_MEMSYS_GATE_GMEM) == 0u);
  ivs_stic_catch_up(&p, &s, &sc_mem, (uint64_t)p.frame_ticks - 1u, NULL, NULL);
  SC_CHECK(s.display_latch == 1u && s.windows_active == 1u && s.frame_index == 0u);
  ivs_stic_catch_up(&p, &s, &sc_mem, p.frame_ticks, NULL, NULL);
  SC_CHECK(s.frame_index == 1u && s.display_latch == 0u && s.windows_active == 0u && s.grants_in_frame == 0u);
  SC_CHECK(s.cs_ptr == 3u);
  SC_CHECK(sc_open(IVS_MEMSYS_GATE_STIC_REGS) == 1u && sc_open(IVS_MEMSYS_GATE_GMEM) == 1u);
  /* a frame without a strobe leaves windows_active 0 */
  s.display_latch = 0u;
  ivs_stic_catch_up(&p, &s, &sc_mem, (uint64_t)p.frame_ticks + (uint64_t)p.event_tick[gm], NULL, NULL);
  SC_CHECK(s.windows_active == 0u);
  return IVS_TEST_PASS;
}

/* A power-on reset returns the cursor and the gates of a machine that ran past both windows. */
static int sc_reset_cursor(void)
{
  ivs_ms_inst x;
  struct ivs_machine *im;
  const ivs_stic_params *p;
  ivs_stic_state fresh;
  uint32_t n = 0u;
  int ok = 0;
  SC_CHECK(ivs_ms_powered_prog(&x, IVS_MS_PROG_A, 0x5Au));
  im = ivs_machine_from_handle_mut(x.m);
  p = ivs_stic_params_get(im->profile);
  while (n < 20000u && im->time_ticks <= (uint64_t)p->gmem_window_ticks) {
    if (!ivs_ms_step(x.m, 1u, NULL)) {
      goto done;
    }
    n++;
  }
  if (im->stic.event_index == 0u || im->mem.gate_open[IVS_MEMSYS_GATE_STIC_REGS] != 0u ||
      im->mem.gate_open[IVS_MEMSYS_GATE_GMEM] != 0u) {
    ivs_test_diag("the run did not close both gates (event_index %u)", (unsigned)im->stic.event_index);
    goto done;
  }
  if (ivs_machine_reset(x.m, IVS_RESET_POWER_ON) != IVS_OK) {
    goto done;
  }
  ivs_stic_state_reset(&fresh, p);
  if (memcmp(&fresh, &im->stic, sizeof fresh) != 0 || im->stic.event_index != 0u || im->stic.frame_index != 0u ||
      im->mem.gate_open[IVS_MEMSYS_GATE_STIC_REGS] != 1u || im->mem.gate_open[IVS_MEMSYS_GATE_GMEM] != 1u ||
      im->time_ticks != 0u) {
    ivs_test_diag("reset left the cursor or a gate");
    goto done;
  }
  ok = 1;
done:
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* After every unit boundary the machine's STIC state and gates equal a reference caught up DIRECTLY to that boundary
 * (so the seam catches the STIC up to the unit END, never to its start), over more than two frames; and one big run
 * call to the same boundary leaves the same composite digest as the unit-by-unit run. */
static int sc_slice_state_equal(void)
{
  static ivs_memsys ref_mem;
  ivs_ms_inst x;
  ivs_ms_inst y;
  struct ivs_machine *im;
  const ivs_stic_params *p;
  ivs_stic_state ref;
  uint64_t end;
  uint32_t n = 0u;
  uint32_t closed_seen = 0u;
  uint32_t reopened = 0u;
  int ok = 0;
  uint8_t cx[32];
  uint8_t cy[32];
  uint8_t cpu[32];
  ivs_run_request q;
  ivs_run_report rep;
  x.storage = NULL;
  y.storage = NULL;
  if (!ivs_ms_powered_prog(&x, IVS_MS_PROG_A, 0x5Au)) {
    return IVS_TEST_FAIL;
  }
  if (!ivs_ms_powered_prog(&y, IVS_MS_PROG_A, 0x5Au)) {
    ivs_ms_free(&x);
    return IVS_TEST_FAIL;
  }
  im = ivs_machine_from_handle_mut(x.m);
  p = ivs_stic_params_get(im->profile);
  if (ivs_memsys_init(&ref_mem, im->profile) != IVS_OK) {
    goto done;
  }
  ivs_stic_state_reset(&ref, p);
  end = 2u * (uint64_t)p->frame_ticks + (uint64_t)p->reg_window_ticks;
  while (n < 40000u && im->time_ticks <= end) {
    uint32_t reg_before = im->mem.gate_open[IVS_MEMSYS_GATE_STIC_REGS];
    if (!ivs_ms_step(x.m, 1u, NULL)) {
      goto done;
    }
    n++;
    ivs_stic_catch_up(p, &ref, &ref_mem, im->time_ticks, NULL, NULL);
    if (memcmp(&ref, &im->stic, sizeof ref) != 0 ||
        ref_mem.gate_open[IVS_MEMSYS_GATE_STIC_REGS] != im->mem.gate_open[IVS_MEMSYS_GATE_STIC_REGS] ||
        ref_mem.gate_open[IVS_MEMSYS_GATE_GMEM] != im->mem.gate_open[IVS_MEMSYS_GATE_GMEM]) {
      ivs_test_diag("unit %u (time %llu): the machine's STIC state differs from the state caught up to the unit end",
                    (unsigned)n, (unsigned long long)im->time_ticks);
      goto done;
    }
    if (im->mem.gate_open[IVS_MEMSYS_GATE_STIC_REGS] == 0u) {
      closed_seen = 1u;
    } else if (reg_before == 0u) {
      reopened = 1u;
    }
  }
  if (closed_seen == 0u || reopened == 0u) {
    ivs_test_diag("the run did not close and reopen the register gate (units %u)", (unsigned)n);
    goto done;
  }
  /* one big call to the boundary the unit-by-unit run ended on */
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.target_ticks = im->time_ticks;
  memset(&rep, 0, sizeof rep);
  rep.struct_size = (uint32_t)sizeof rep;
  if (ivs_machine_run(y.m, &q, &rep) != IVS_OK || rep.now_ticks != im->time_ticks) {
    ivs_test_diag("the big call ended at %llu, the unit-by-unit run at %llu", (unsigned long long)rep.now_ticks,
                  (unsigned long long)im->time_ticks);
    goto done;
  }
  if (!ivs_ms_digests(x.m, cx, cpu) || !ivs_ms_digests(y.m, cy, cpu) || memcmp(cx, cy, 32u) != 0) {
    ivs_test_diag("composite digest of the big call differs from the unit-by-unit run");
    goto done;
  }
  ok = 1;
done:
  ivs_ms_free(&x);
  ivs_ms_free(&y);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_case ivs_stic_events_cases[] = {
    {"stic.events.table-order", sc_table_order, NULL},
    {"stic.events.edges", sc_edges, NULL},
    {"stic.events.frame-index-is-state", sc_frame_index_is_state, NULL},
    {"stic.events.idempotent", sc_idempotent, NULL},
    {"stic.events.latch-cleared-at-frame-start", sc_latch_cleared_at_frame_start, NULL},
    {"stic.reset.cursor", sc_reset_cursor, NULL},
    {"stic.slice.state-equal-at-every-unit-boundary", sc_slice_state_equal, NULL},
};
const size_t ivs_stic_events_count = sizeof ivs_stic_events_cases / sizeof ivs_stic_events_cases[0];
