/* SPDX-License-Identifier: MIT */
/* Suite stic, family busrq (plan 06-08; D-09, A-09): the 13 bus windows per display-on frame as the CPU stepper's own BUS_GRANT unit
 * sequence. The closed form is the oracle: a window starting at tick w resumes the CPU at w + 4 x (H + 1 + L) ticks (H the hold, one
 * grant cycle, L the release latency of the profile), whichever boundary granted; the numbers H, L and the window table come from the
 * parameters and the profile as relations (D-02), never as expected constants.
 *   stic.busrq.closed-form-resume   six loop bodies of different instruction lengths over three display-on frames: every window is
 *                                   granted at an open boundary inside it, the grant units are 1 cycle, the release unit is L cycles,
 *                                   and the next unit starts at w + 4 (H + 1 + L); several distinct grant phases are covered
 *   stic.busrq.count-per-frame      13 releases per display-on frame (within the printed 13 or 14), none in a frame without strobe
 *   stic.busrq.hold-from-profile    a hold or a release latency changed by one cycle moves the resume by exactly one cycle
 *                                   (stepper plus line function with a stack copy of the parameters)
 *   stic.busrq.none-when-display-off  a loop that never strobes the display is never stalled
 *   stic.busrq.mvo-stream-never-stalled  a stream of non-interruptible stores is neither stalled nor interrupted
 *   stic.busrq.halted-grants        a halted CPU grants every window, 13 releases, and stays halted
 *   stic.busrq.late-grant-counter   a stream of non-interruptible stores that covers every window: late_grants grows by bus_count in the
 *                                   display-on frame and no grant occurs; an interruptible loop: late_grants stays 0 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "machine/machine.h"
#include "machine_cases.h"
#include "stic_cases.h"

#define SB_CAP 40000u

static uint32_t sb_latency(void)
{
  return ivs_profile_get(SC_PROFILE)->release_latency_cpu_cycles;
}

/* Checks every bus-grant run of the trace against the closed form; returns the number of windows verified (frames fully inside the
 * trace), or (uint32_t)-1 on a violation. phases[] collects the distinct grant phases (grant start minus window start minus lag, in cycles). */
static uint32_t sb_check_runs(const ivs_stic_params *p, const sc_unit *u, size_t n, uint64_t frames, uint32_t *phases)
{
  uint32_t verified = 0u;
  uint32_t lat = sb_latency();
  size_t k = 0u;
  while (k < n) {
    size_t j;
    uint64_t rel;
    uint64_t frame;
    uint64_t sk;
    uint64_t idx;
    uint64_t ws;
    uint64_t resume;
    if (u[k].kind != IVS_CP1610_UNIT_BUS_GRANT) {
      k++;
      continue;
    }
    frame = (u[k].start - p->frame_phase_ticks) / p->frame_ticks;
    sk = (uint64_t)p->frame_phase_ticks + frame * (uint64_t)p->frame_ticks;
    rel = u[k].start - sk;
    if (rel < p->bus_first_start_ticks) {
      ivs_test_diag("grant at %llu is before the first window", (unsigned long long)u[k].start);
      return (uint32_t)-1;
    }
    idx = (rel - p->bus_first_start_ticks) / p->bus_pitch_ticks;
    ws = sk + (uint64_t)p->bus_first_start_ticks + idx * (uint64_t)p->bus_pitch_ticks;
    if (idx >= p->bus_count || u[k].start < ws + p->line_sample_lag_ticks || u[k].start >= ws + p->bus_hold_ticks + p->line_sample_lag_ticks) {
      ivs_test_diag("grant at %llu is outside every window", (unsigned long long)u[k].start);
      return (uint32_t)-1;
    }
    /* the run: 1-cycle units until the release unit (event BUS_RELEASED, L cycles) */
    for (j = k; j < n && (u[j].events & IVS_CP1610_EV_BUS_RELEASED) == 0u; j++) {
      if (u[j].kind != IVS_CP1610_UNIT_BUS_GRANT || u[j].cycles != 1u) {
        ivs_test_diag("grant run at %llu: unit %lu is not a 1-cycle grant unit", (unsigned long long)u[k].start, (unsigned long)j);
        return (uint32_t)-1;
      }
    }
    if (j + 1u >= n || u[j].kind != IVS_CP1610_UNIT_BUS_GRANT || u[j].cycles != lat) {
      ivs_test_diag("grant run at %llu has no release unit of %u cycles", (unsigned long long)u[k].start, (unsigned)lat);
      return (uint32_t)-1;
    }
    resume = ws + 4u * ((uint64_t)p->bus_hold_cycles + 1u + (uint64_t)lat);
    if (u[j + 1u].start != resume || u[j + 1u].kind == IVS_CP1610_UNIT_BUS_GRANT) {
      ivs_test_diag("window at %llu: the CPU resumes at %llu, the closed form says %llu", (unsigned long long)ws,
                    (unsigned long long)u[j + 1u].start, (unsigned long long)resume);
      return (uint32_t)-1;
    }
    if (frame < frames) {
      verified++;
    }
    if (phases != NULL) {
      uint64_t ph = (u[k].start - ws - p->line_sample_lag_ticks) / p->ticks_per_cpu_cycle;
      if (ph < 32u) {
        *phases |= 1u << ph;
      }
    }
    k = j + 1u;
  }
  return verified;
}

static int sb_closed_form(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  ivs_ms_inst x;
  sc_unit *u = malloc(SB_CAP * sizeof *u);
  uint16_t w[SC_PROG_MAX];
  uint32_t phases = 0u;
  uint32_t body;
  uint32_t bits = 0u;
  uint32_t v;
  int ok = 0;
  memset(&x, 0, sizeof x);
  if (u == NULL) {
    return IVS_TEST_FAIL;
  }
  for (body = 0u; body < SC_BODIES; body++) {
    size_t n;
    if (!sc_guest(&x, w, sc_prog_vbl(w, body), 0u)) {
      goto done;
    }
    n = sc_trace(&x, 3u * (uint64_t)p->frame_ticks, u, SB_CAP);
    if (n == (size_t)-1) {
      ivs_test_diag("body %u: trace refused or overflowed", (unsigned)body);
      goto done;
    }
    v = sb_check_runs(p, u, n, 3u, &phases);
    if (v != 3u * p->bus_count && v != 3u * p->bus_count - 1u) { /* the last frame's final window may lie beyond the trace end */
      ivs_test_diag("body %u: %u windows verified, %u expected", (unsigned)body, (unsigned)v, (unsigned)(3u * p->bus_count));
      goto done;
    }
    ivs_ms_free(&x);
    memset(&x, 0, sizeof x);
  }
  while (phases != 0u) {
    bits += phases & 1u;
    phases >>= 1u;
  }
  if (bits < 5u) {
    ivs_test_diag("only %u distinct grant phases were covered", (unsigned)bits);
    goto done;
  }
  ok = 1;
done:
  free(u);
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int sb_count_per_frame(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  ivs_ms_inst x;
  sc_unit *u = malloc(SB_CAP * sizeof *u);
  uint16_t w[SC_PROG_MAX];
  uint32_t per_frame[4] = {0u, 0u, 0u, 0u};
  size_t n;
  size_t k;
  uint32_t f;
  int ok = 0;
  memset(&x, 0, sizeof x);
  if (u == NULL || !sc_guest(&x, w, sc_prog_vbl(w, SC_BODY_INCR), 0u)) {
    free(u);
    return IVS_TEST_FAIL;
  }
  n = sc_trace(&x, 4u * (uint64_t)p->frame_ticks, u, SB_CAP);
  if (n == (size_t)-1) {
    goto done;
  }
  for (k = 0u; k < n; k++) {
    if ((u[k].events & IVS_CP1610_EV_BUS_RELEASED) != 0u) {
      uint64_t frame = (u[k].start - p->frame_phase_ticks) / p->frame_ticks;
      if (frame < 4u) {
        per_frame[frame]++;
      }
    }
  }
  for (f = 0u; f < 4u; f++) {
    if (per_frame[f] != p->bus_count || per_frame[f] < 13u || per_frame[f] > 14u) {
      ivs_test_diag("frame %u has %u releases (bus_count %u, printed bound 13 or 14)", (unsigned)f, (unsigned)per_frame[f],
                    (unsigned)p->bus_count);
      goto done;
    }
  }
  ok = 1;
done:
  free(u);
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* A flat bus for the stepper-level rig: every fetch returns a NOP (0034), every other access is ignored. */
static ivs_bus_reply sb_flat_bus(void *ctx, const ivs_cp1610_access *a)
{
  ivs_bus_reply r;
  (void)ctx;
  (void)a;
  r.data = 0x0034u;
  r.wait_cpu_cycles = 0u;
  return r;
}

/* The tick at which the CPU resumes after window 0 of frame 0, driving the real stepper with the line function of the (perturbed)
 * parameters and the cfg latency; UINT64_MAX when no grant happened. */
static uint64_t sb_rig_resume(const ivs_stic_params *p, uint32_t latency)
{
  ivs_cp1610 cpu;
  ivs_cp1610_config cfg;
  ivs_cp1610_lines lines = {0u, 0u, 0u, 0u};
  ivs_cp1610_port port;
  ivs_stic_state s;
  uint64_t now = 0u;
  uint32_t units = 0u;
  int granted = 0;
  memset(&cfg, 0, sizeof cfg);
  cfg.release_latency_cpu_cycles = (uint8_t)latency;
  cfg.wait_limit_cpu_cycles = 35u;
  memset(&port, 0, sizeof port);
  port.bus = sb_flat_bus;
  (void)ivs_cp1610_reset(&cpu);
  ivs_stic_state_reset(&s, p);
  s.windows_active = 1u;
  while (now < (uint64_t)p->frame_ticks && units++ < 8000u) {
    ivs_cp1610_result r;
    ivs_stic_line_levels l = {0u, 0u};
    if (now >= p->line_sample_lag_ticks) {
      l = ivs_stic_lines(p, &s, now - p->line_sample_lag_ticks);
    }
    lines.busrq = (uint8_t)l.busrq;
    if (ivs_cp1610_step(&cpu, &cfg, &lines, &port, NULL, 0u, &r) != IVS_OK) {
      return UINT64_MAX;
    }
    if (r.unit_kind == IVS_CP1610_UNIT_BUS_GRANT) {
      granted = 1;
    } else if (granted != 0) {
      return now;
    }
    now += (uint64_t)r.elapsed_cpu_cycles * (uint64_t)p->ticks_per_cpu_cycle;
  }
  return UINT64_MAX;
}

static int sb_hold_from_profile(void)
{
  ivs_stic_params p = *ivs_stic_params_get(SC_PROFILE);
  uint32_t lat = sb_latency();
  uint64_t w0 = (uint64_t)p.frame_phase_ticks + (uint64_t)p.bus_first_start_ticks;
  uint64_t base = sb_rig_resume(&p, lat);
  uint64_t more_hold;
  uint64_t more_latency;
  if (base != w0 + 4u * ((uint64_t)p.bus_hold_cycles + 1u + (uint64_t)lat)) {
    ivs_test_diag("rig resume %llu is not the closed form %llu", (unsigned long long)base,
                  (unsigned long long)(w0 + 4u * ((uint64_t)p.bus_hold_cycles + 1u + (uint64_t)lat)));
    return IVS_TEST_FAIL;
  }
  p.bus_hold_cycles += 1u;
  p.bus_hold_ticks += p.ticks_per_cpu_cycle;
  more_hold = sb_rig_resume(&p, lat);
  p.bus_hold_cycles -= 1u;
  p.bus_hold_ticks -= p.ticks_per_cpu_cycle;
  more_latency = sb_rig_resume(&p, lat + 1u);
  if (more_hold != base + p.ticks_per_cpu_cycle || more_latency != base + p.ticks_per_cpu_cycle) {
    ivs_test_diag("hold + 1 cycle resumes at %llu, latency + 1 cycle at %llu, base %llu (one cycle is %u ticks)",
                  (unsigned long long)more_hold, (unsigned long long)more_latency, (unsigned long long)base, (unsigned)p.ticks_per_cpu_cycle);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

static int sb_none_when_display_off(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  const struct ivs_machine *im;
  ivs_ms_inst x;
  sc_unit *u = malloc(SB_CAP * sizeof *u);
  uint16_t w[SC_PROG_MAX];
  size_t n;
  size_t k;
  int ok = 0;
  memset(&x, 0, sizeof x);
  if (u == NULL || !sc_guest(&x, w, sc_prog_loop(w, 0u), 0u)) {
    free(u);
    return IVS_TEST_FAIL;
  }
  im = ivs_machine_from_handle(x.m);
  n = sc_trace(&x, 3u * (uint64_t)p->frame_ticks, u, SB_CAP);
  if (n == (size_t)-1) {
    goto done;
  }
  for (k = 0u; k < n; k++) {
    if (u[k].kind != IVS_CP1610_UNIT_INSTR && u[k].kind != IVS_CP1610_UNIT_RESET_ENTRY) {
      ivs_test_diag("unit %lu at %llu has kind %u in a display-off loop", (unsigned long)k, (unsigned long long)u[k].start,
                    (unsigned)u[k].kind);
      goto done;
    }
  }
  if (im->stic.windows_active != 0u || im->stic.display_latch != 0u || n < 1000u) {
    ivs_test_diag("windows_active %u, latch %u, units %lu", (unsigned)im->stic.windows_active, (unsigned)im->stic.display_latch,
                  (unsigned long)n);
    goto done;
  }
  ok = 1;
done:
  free(u);
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int sb_mvo_stream(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  const struct ivs_machine *im;
  ivs_ms_inst x;
  sc_unit *u = malloc(SB_CAP * sizeof *u);
  uint16_t *w = malloc(4096u * sizeof *w);
  size_t n;
  size_t k;
  uint64_t last_window;
  int ok = 0;
  memset(&x, 0, sizeof x);
  /* 1,300 stores of 11 cycles after the EIS end at cycle 14,304, before the frame end at 14,934 */
  if (u == NULL || w == NULL || !sc_guest_n(&x, w, sc_prog_stream(w, 1300u))) {
    free(u);
    free(w);
    return IVS_TEST_FAIL;
  }
  im = ivs_machine_from_handle(x.m);
  last_window = (uint64_t)p->frame_phase_ticks + (uint64_t)p->bus_first_start_ticks + 11u * (uint64_t)p->bus_pitch_ticks;
  n = sc_trace(&x, last_window + (uint64_t)p->bus_hold_ticks + 4u, u, SB_CAP);
  if (n == (size_t)-1) {
    goto done;
  }
  for (k = 0u; k < n; k++) {
    if (u[k].kind != IVS_CP1610_UNIT_INSTR && u[k].kind != IVS_CP1610_UNIT_RESET_ENTRY) {
      ivs_test_diag("unit %lu at %llu has kind %u in a store stream", (unsigned long)k, (unsigned long long)u[k].start,
                    (unsigned)u[k].kind);
      goto done;
    }
  }
  if (im->stic.windows_active != 1u || n < 1000u) {
    ivs_test_diag("windows_active %u (the stream strobes the display), units %lu", (unsigned)im->stic.windows_active, (unsigned long)n);
    goto done;
  }
  ok = 1;
done:
  free(u);
  free(w);
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int sb_halted_grants(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  ivs_ms_inst x;
  sc_unit *u = malloc(SB_CAP * sizeof *u);
  uint16_t w[SC_PROG_MAX];
  uint32_t released = 0u;
  uint32_t released_frame1 = 0u;
  uint32_t v;
  size_t n;
  size_t k;
  int ok = 0;
  memset(&x, 0, sizeof x);
  if (u == NULL || !sc_guest(&x, w, sc_prog_halt(w), 0u)) {
    free(u);
    return IVS_TEST_FAIL;
  }
  n = sc_trace(&x, 2u * (uint64_t)p->frame_ticks, u, SB_CAP);
  if (n == (size_t)-1) {
    goto done;
  }
  for (k = 0u; k < n; k++) {
    if ((u[k].events & IVS_CP1610_EV_BUS_RELEASED) != 0u) {
      if (u[k].start < (uint64_t)p->frame_ticks) {
        released++;
      } else {
        released_frame1++;
      }
    }
    if (k > 20u && u[k].kind != IVS_CP1610_UNIT_BUS_GRANT && u[k].kind != IVS_CP1610_UNIT_HALTED) {
      ivs_test_diag("unit %lu at %llu has kind %u: the CPU did not stay halted", (unsigned long)k, (unsigned long long)u[k].start,
                    (unsigned)u[k].kind);
      goto done;
    }
  }
  v = sb_check_runs(p, u, n, 1u, NULL);
  if (released != p->bus_count || released_frame1 != 0u || v != p->bus_count) {
    ivs_test_diag("frame 0: %u releases (%u expected), frame 1: %u (0 expected: no strobe), %u windows verified", (unsigned)released,
                  (unsigned)p->bus_count, (unsigned)released_frame1, (unsigned)v);
    goto done;
  }
  ok = 1;
done:
  free(u);
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* The checker of the late-grant observations (also run on corrupted observations by stic.judge.late-grant-latched). */
int sb_late_counter_ok(uint32_t late_after_stream, uint32_t late_after_loop, uint32_t bus_count)
{
  return late_after_stream == bus_count && late_after_loop == 0u;
}

static int sb_late_grant_counter(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  const struct ivs_machine *im;
  ivs_ms_inst x;
  uint16_t *w = malloc(4096u * sizeof *w);
  uint16_t v[SC_PROG_MAX];
  uint32_t late_stream = 0u;
  uint32_t late_loop = 0u;
  uint32_t grants = 0u;
  int ok = 0;
  memset(&x, 0, sizeof x);
  if (w == NULL) {
    return IVS_TEST_FAIL;
  }
  /* 1,355 stores of 11 cycles after the EIS end at cycle 14,909, after the last window (it ends at 14,849) and before the frame end */
  if (!sc_guest_n(&x, w, sc_prog_stream(w, 1355u))) {
    free(w);
    return IVS_TEST_FAIL;
  }
  im = ivs_machine_from_handle(x.m);
  if (!sc_run_until(&x, (uint64_t)p->frame_phase_ticks + (uint64_t)p->frame_ticks - 100u)) { /* every window is over, the frame is not */
    goto done;
  }
  grants = im->stic.grants_in_frame;
  if (!sc_run_until(&x, (uint64_t)p->frame_phase_ticks + (uint64_t)p->frame_ticks + 4u)) {
    goto done;
  }
  late_stream = im->stic.late_grants;
  ivs_ms_free(&x);
  memset(&x, 0, sizeof x);
  if (!sc_guest(&x, v, sc_prog_vbl(v, SC_BODY_INCR), 0u)) {
    goto done;
  }
  im = ivs_machine_from_handle(x.m);
  if (!sc_run_until(&x, 3u * (uint64_t)p->frame_ticks + 4u)) {
    goto done;
  }
  late_loop = im->stic.late_grants;
  if (grants != 0u || sb_late_counter_ok(late_stream, late_loop, p->bus_count) == 0) {
    ivs_test_diag("stream: %u late grants after frame 0, %u grants; loop: %u late grants (bus_count %u)",
                  (unsigned)late_stream, (unsigned)grants, (unsigned)late_loop, (unsigned)p->bus_count);
    goto done;
  }
  ok = 1;
done:
  free(w);
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_case ivs_stic_busrq_cases[] = {
    {"stic.busrq.closed-form-resume", sb_closed_form, NULL},
    {"stic.busrq.count-per-frame", sb_count_per_frame, NULL},
    {"stic.busrq.hold-from-profile", sb_hold_from_profile, NULL},
    {"stic.busrq.none-when-display-off", sb_none_when_display_off, NULL},
    {"stic.busrq.mvo-stream-never-stalled", sb_mvo_stream, NULL},
    {"stic.busrq.halted-grants", sb_halted_grants, NULL},
    {"stic.busrq.late-grant-counter", sb_late_grant_counter, NULL},
};
const size_t ivs_stic_busrq_count = sizeof ivs_stic_busrq_cases / sizeof ivs_stic_busrq_cases[0];
