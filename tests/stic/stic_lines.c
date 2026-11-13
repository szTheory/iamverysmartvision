/* SPDX-License-Identifier: MIT */
/* Suite stic, family lines (plan 06-08; D-08, D-09, A-09): the pure line function ivs_stic_lines and the machine seam that samples it.
 * Every expectation is a relation to the generated parameters (the INTRM level is the first irq_low ticks of a frame; a window i is
 * [first + i x pitch, first + i x pitch + hold) from the frame start), formulated by an enumeration of the windows, never by the
 * modulo arithmetic the function uses, and never a policy number (D-02).
 *   stic.lines.pure-of-start-tick   the levels are a function of (parameters, state, tick) and of the tick relative to the frame
 *                                   start; nothing in the state changes; a tick before the frame start is in the previous frame
 *   stic.lines.sampling-lag         a perturbed lag moves the first boundary that sees a window by exactly the perturbation (pure
 *                                   function with the seam's rule), and the real machine with a halted CPU (a boundary every cycle)
 *                                   grants the bus at the window start plus the profile's lag in every one of the 13 windows */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "machine/machine.h"
#include "machine_cases.h"
#include "stic_cases.h"

/* The expected levels of the tick t of the frame that starts at s0, from the parameters by enumeration. */
static void sl_expect(const ivs_stic_params *p, uint64_t s0, uint32_t windows, uint64_t t, uint32_t *intrm, uint32_t *busrq)
{
  uint32_t i;
  *intrm = (t >= s0 && t - s0 < (uint64_t)p->irq_low_ticks) ? 1u : 0u;
  *busrq = 0u;
  for (i = 0u; i < p->bus_count && windows != 0u; i++) {
    uint64_t w = s0 + (uint64_t)p->bus_first_start_ticks + (uint64_t)i * (uint64_t)p->bus_pitch_ticks;
    if (t >= w && t < w + (uint64_t)p->bus_hold_ticks) {
      *busrq = 1u;
    }
  }
}

static int sl_pure(void)
{
  ivs_stic_params p = *ivs_stic_params_get(SC_PROFILE);
  ivs_stic_state s;
  ivs_stic_state before;
  ivs_stic_state off;
  uint64_t s3;
  uint64_t t;
  uint32_t seen_intrm = 0u;
  uint32_t seen_busrq = 0u;
  ivs_stic_state_reset(&s, &p);
  s.frame_index = 3u;
  s.windows_active = 1u;
  before = s;
  off = s;
  off.windows_active = 0u;
  s3 = (uint64_t)p.frame_phase_ticks + 3u * (uint64_t)p.frame_ticks;
  for (t = s3 - 2u * (uint64_t)p.frame_ticks; t < s3 + (uint64_t)p.frame_ticks + 8u; t++) {
    uint32_t xi;
    uint32_t xb;
    ivs_stic_line_levels a = ivs_stic_lines(&p, &s, t);
    ivs_stic_line_levels b = ivs_stic_lines(&p, &s, t);
    ivs_stic_line_levels z = ivs_stic_lines(&p, &off, t);
    ivs_stic_line_levels r;
    ivs_stic_state zero;
    sl_expect(&p, s3, 1u, t, &xi, &xb);
    if (a.intrm != xi || a.busrq != xb || b.intrm != a.intrm || b.busrq != a.busrq) {
      ivs_test_diag("tick %llu: levels %u/%u expected %u/%u", (unsigned long long)t, (unsigned)a.intrm, (unsigned)a.busrq,
                    (unsigned)xi, (unsigned)xb);
      return IVS_TEST_FAIL;
    }
    if (z.intrm != xi || z.busrq != 0u) { /* windows_active 0: INTRM is independent of the display latch, no bus request */
      ivs_test_diag("tick %llu: with the windows off the levels are %u/%u", (unsigned long long)t, (unsigned)z.intrm,
                    (unsigned)z.busrq);
      return IVS_TEST_FAIL;
    }
    if (t < s3 && (a.intrm != 0u || a.busrq != 0u)) {
      ivs_test_diag("tick %llu before the frame start: a line is up", (unsigned long long)t);
      return IVS_TEST_FAIL;
    }
    /* the same relative tick in frame 0: the same levels (the levels depend on tick minus frame start only) */
    ivs_stic_state_reset(&zero, &p);
    zero.windows_active = 1u;
    if (t >= s3 && t - s3 < (uint64_t)p.frame_ticks) {
      r = ivs_stic_lines(&p, &zero, (uint64_t)p.frame_phase_ticks + (t - s3));
      if (r.intrm != a.intrm || r.busrq != a.busrq) {
        ivs_test_diag("tick %llu: frame 0 and frame 3 differ at the same relative tick", (unsigned long long)t);
        return IVS_TEST_FAIL;
      }
    }
    seen_intrm |= a.intrm;
    seen_busrq |= a.busrq;
  }
  if (memcmp(&s, &before, sizeof s) != 0) {
    ivs_test_diag("the line function changed the state");
    return IVS_TEST_FAIL;
  }
  if (seen_intrm == 0u || seen_busrq == 0u) {
    ivs_test_diag("vacuous sweep: intrm seen %u, busrq seen %u", (unsigned)seen_intrm, (unsigned)seen_busrq);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

/* The first boundary (a multiple of one CPU cycle from 0) whose sample (boundary minus lag) sees window 0 of frame 0. */
static uint64_t sl_first_seen(const ivs_stic_params *p, const ivs_stic_state *s)
{
  uint64_t b;
  for (b = 0u; b < (uint64_t)p->frame_ticks; b += p->ticks_per_cpu_cycle) {
    if (b >= (uint64_t)p->line_sample_lag_ticks && ivs_stic_lines(p, s, b - (uint64_t)p->line_sample_lag_ticks).busrq != 0u) {
      return b;
    }
  }
  return UINT64_MAX;
}

static int sl_sampling_lag(void)
{
  ivs_stic_params p = *ivs_stic_params_get(SC_PROFILE);
  ivs_ms_inst x;
  sc_unit *u = NULL;
  uint16_t w[SC_PROG_MAX];
  ivs_stic_state s;
  uint32_t lag_cycles;
  uint32_t i;
  size_t n;
  size_t k;
  int ok = 0;
  /* (1) the pure rule: a lag moved by one cycle moves the first seen boundary by one cycle */
  ivs_stic_state_reset(&s, &p);
  s.windows_active = 1u;
  for (lag_cycles = 0u; lag_cycles < 4u; lag_cycles++) {
    uint64_t first;
    uint64_t seen;
    p.line_sample_lag_cycles = lag_cycles;
    p.line_sample_lag_ticks = lag_cycles * p.ticks_per_cpu_cycle;
    first = (uint64_t)p.frame_phase_ticks + (uint64_t)p.bus_first_start_ticks;
    seen = sl_first_seen(&p, &s);
    if (seen != first + (uint64_t)p.line_sample_lag_ticks) {
      ivs_test_diag("lag %u cycles: first seen boundary %llu, window start %llu", (unsigned)lag_cycles, (unsigned long long)seen,
                    (unsigned long long)first);
      return IVS_TEST_FAIL;
    }
  }
  memset(&x, 0, sizeof x);
  /* (2) the real machine: a halted CPU takes a unit every cycle, so the first bus grant of a window starts exactly one lag after it */
  p = *ivs_stic_params_get(SC_PROFILE);
  u = malloc(40000u * sizeof *u);
  if (u == NULL || !sc_guest(&x, w, sc_prog_halt(w), 0u)) {
    free(u);
    return IVS_TEST_FAIL;
  }
  n = sc_trace(&x, 2u * (uint64_t)p.frame_ticks, u, 40000u);
  if (n == (size_t)-1) {
    ivs_test_diag("trace refused or overflowed");
    goto done;
  }
  for (i = 0u; i < p.bus_count; i++) {
    uint64_t ws = (uint64_t)p.frame_phase_ticks + (uint64_t)p.bus_first_start_ticks + (uint64_t)i * (uint64_t)p.bus_pitch_ticks;
    uint64_t first_grant = UINT64_MAX;
    for (k = 0u; k < n; k++) {
      if (u[k].kind == IVS_CP1610_UNIT_BUS_GRANT && u[k].start >= ws && u[k].start < ws + (uint64_t)p.bus_pitch_ticks) {
        first_grant = u[k].start;
        break;
      }
    }
    if (first_grant != ws + (uint64_t)p.line_sample_lag_ticks) {
      ivs_test_diag("window %u starts at %llu, first grant at %llu (lag %u ticks)", (unsigned)i, (unsigned long long)ws,
                    (unsigned long long)first_grant, (unsigned)p.line_sample_lag_ticks);
      goto done;
    }
  }
  ok = 1;
done:
  free(u);
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_case ivs_stic_lines_cases[] = {
    {"stic.lines.pure-of-start-tick", sl_pure, NULL},
    {"stic.lines.sampling-lag", sl_sampling_lag, NULL},
};
const size_t ivs_stic_lines_count = sizeof ivs_stic_lines_cases / sizeof ivs_stic_lines_cases[0];
