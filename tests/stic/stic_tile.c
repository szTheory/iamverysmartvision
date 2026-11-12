/* SPDX-License-Identifier: MIT */
/* Suite stic, family tile and steal (plan 06-08; D-09, D-10, A-09): the units of the machine tile machine time exactly, and the stolen
 * time is an accounting identity. Nothing here is a policy number: the window table, the hold and the release latency come from the
 * parameters and the profile as relations (D-02).
 *   stic.tile.units-equal-elapsed     over three display-on frames the recorded units are contiguous (each starts where the previous one
 *                                     ended), the sum of their cycles is elapsed machine time exactly, and the run holds instruction,
 *                                     interrupt-entry, grant and hold, and release units
 *   stic.steal.accounting             a constant-cost loop in a display-on frame 0: per window resume - w = 4 (H + 1 + L); the cycles of all
 *                                     BUS_GRANT units equal the sum over the windows of (resume - grant) / 4; and the loop retires
 *                                     floor((total cycles - stolen - lead) / cost) instructions
 *   stic.steal.usable-within-community-tolerance  executed instruction cycles of a steady display-on frame of the 12-cycle interruptible
 *                                     loop lie within 1.0 percent of the community aggregate 13,518: a labelled plausibility check
 *                                     under C95, an output of the guest's mix and never an expected constant */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "machine/machine.h"
#include "machine_cases.h"
#include "stic_cases.h"

#define ST_CAP 40000u

static int st_tile(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  const struct ivs_machine *im;
  ivs_ms_inst x;
  sc_unit *u = malloc(ST_CAP * sizeof *u);
  uint16_t w[SC_PROG_MAX];
  uint64_t sum = 0u;
  uint32_t instr = 0u;
  uint32_t entry = 0u;
  uint32_t hold = 0u;
  uint32_t release = 0u;
  size_t n;
  size_t k;
  int ok = 0;
  memset(&x, 0, sizeof x);
  if (u == NULL || !sc_guest(&x, w, sc_prog_vbl(w, SC_BODY_INCR), 0u)) {
    free(u);
    return IVS_TEST_FAIL;
  }
  im = ivs_machine_from_handle(x.m);
  n = sc_trace(&x, 3u * (uint64_t)p->frame_ticks, u, ST_CAP);
  if (n == (size_t)-1) {
    goto done;
  }
  for (k = 0u; k < n; k++) {
    if (u[k].start != (k == 0u ? 0u : u[k - 1u].start + (uint64_t)u[k - 1u].cycles * p->ticks_per_cpu_cycle)) {
      ivs_test_diag("unit %lu starts at %llu, the previous unit ended elsewhere", (unsigned long)k, (unsigned long long)u[k].start);
      goto done;
    }
    sum += u[k].cycles;
    instr += u[k].kind == IVS_CP1610_UNIT_INSTR ? 1u : 0u;
    entry += u[k].kind == IVS_CP1610_UNIT_INT_ENTRY ? 1u : 0u;
    if (u[k].kind == IVS_CP1610_UNIT_BUS_GRANT) {
      if ((u[k].events & IVS_CP1610_EV_BUS_RELEASED) != 0u) {
        release++;
      } else {
        hold++;
      }
    }
  }
  if (sum * p->ticks_per_cpu_cycle != im->time_ticks || instr == 0u || entry == 0u || hold == 0u || release == 0u) {
    ivs_test_diag("sum of unit cycles x ticks = %llu, machine time %llu; instr %u entry %u hold %u release %u",
                  (unsigned long long)(sum * p->ticks_per_cpu_cycle), (unsigned long long)im->time_ticks, (unsigned)instr, (unsigned)entry,
                  (unsigned)hold, (unsigned)release);
    goto done;
  }
  ok = 1;
done:
  free(u);
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int st_accounting(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint32_t lat = ivs_profile_get(SC_PROFILE)->release_latency_cpu_cycles;
  ivs_ms_inst x;
  sc_unit *u = malloc(ST_CAP * sizeof *u);
  uint16_t w[SC_PROG_MAX];
  uint64_t total = 0u;
  uint64_t stolen_units = 0u;
  uint64_t stolen_runs = 0u;
  uint64_t lead = 0u;
  uint64_t loop_cycles;
  uint32_t loop_units = 0u;
  uint32_t cost = 0u;
  uint32_t windows = 0u;
  size_t n;
  size_t k = 0u;
  int ok = 0;
  memset(&x, 0, sizeof x);
  if (u == NULL || !sc_guest(&x, w, sc_prog_strobe_loop(w), 0u)) {
    free(u);
    return IVS_TEST_FAIL;
  }
  n = sc_trace(&x, (uint64_t)p->frame_phase_ticks + (uint64_t)p->frame_ticks, u, ST_CAP); /* the units that start inside frame 0 */
  if (n == (size_t)-1 || n < 100u) {
    goto done;
  }
  while (k < n) {
    if (u[k].kind == IVS_CP1610_UNIT_BUS_GRANT) {
      size_t j = k;
      uint64_t ws = (uint64_t)p->frame_phase_ticks + (uint64_t)p->bus_first_start_ticks + (uint64_t)windows * (uint64_t)p->bus_pitch_ticks;
      uint64_t resume;
      while (j < n && (u[j].events & IVS_CP1610_EV_BUS_RELEASED) == 0u) {
        j++;
      }
      if (j + 1u >= n) {
        goto done;
      }
      resume = u[j + 1u].start;
      if (resume - ws != 4u * ((uint64_t)p->bus_hold_cycles + 1u + (uint64_t)lat)) {
        ivs_test_diag("window %u: resume - w = %llu ticks", (unsigned)windows, (unsigned long long)(resume - ws));
        goto done;
      }
      stolen_runs += (resume - u[k].start) / p->ticks_per_cpu_cycle;
      windows++;
      k = j + 1u;
    } else {
      k++;
    }
  }
  for (k = 0u; k < n; k++) {
    total += u[k].cycles;
    if (u[k].kind == IVS_CP1610_UNIT_BUS_GRANT) {
      stolen_units += u[k].cycles;
    }
  }
  lead = (uint64_t)u[0].cycles + (uint64_t)u[1].cycles; /* the reset entry and the strobe store */
  cost = u[2].cycles;
  for (k = 2u; k < n; k++) {
    loop_units += u[k].kind == IVS_CP1610_UNIT_INSTR ? 1u : 0u;
  }
  loop_cycles = total - stolen_units - lead;
  if (windows != p->bus_count || stolen_units != stolen_runs || cost == 0u || loop_units != loop_cycles / cost || loop_cycles % cost != 0u) {
    ivs_test_diag("windows %u, stolen by units %llu by windows %llu, loop units %u of cost %u in %llu cycles", (unsigned)windows,
                  (unsigned long long)stolen_units, (unsigned long long)stolen_runs, (unsigned)loop_units, (unsigned)cost,
                  (unsigned long long)loop_cycles);
    goto done;
  }
  ok = 1;
done:
  free(u);
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int st_usable(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  ivs_ms_inst x;
  sc_unit *u = malloc(ST_CAP * sizeof *u);
  uint16_t w[SC_PROG_MAX];
  uint64_t executed = 0u;
  size_t n;
  size_t k;
  int ok = 0;
  memset(&x, 0, sizeof x);
  if (u == NULL || !sc_guest(&x, w, sc_prog_vbl(w, SC_BODY_J), 0u)) {
    free(u);
    return IVS_TEST_FAIL;
  }
  n = sc_trace(&x, 2u * (uint64_t)p->frame_ticks, u, ST_CAP);
  if (n == (size_t)-1) {
    goto done;
  }
  for (k = 0u; k < n; k++) { /* frame 1, a steady frame: the display is strobed by the service routine of the frame */
    if (u[k].start >= (uint64_t)p->frame_ticks && u[k].kind == IVS_CP1610_UNIT_INSTR) {
      executed += u[k].cycles;
    }
  }
  /* C95 plausibility: within 1.0 percent of 13,518 (a tolerance of 135 cycles), never an expected constant */
  if (executed + 135u < 13518u || executed > 13518u + 135u) {
    ivs_test_diag("executed instruction cycles in frame 1: %llu (community aggregate 13,518, tolerance 1.0 percent)", (unsigned long long)executed);
    goto done;
  }
  ok = 1;
done:
  free(u);
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_case ivs_stic_tile_cases[] = {
    {"stic.tile.units-equal-elapsed", st_tile, NULL},
    {"stic.steal.accounting", st_accounting, NULL},
    {"stic.steal.usable-within-community-tolerance", st_usable, NULL},
};
const size_t ivs_stic_tile_count = sizeof ivs_stic_tile_cases / sizeof ivs_stic_tile_cases[0];
