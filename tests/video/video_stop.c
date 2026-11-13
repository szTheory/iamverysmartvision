/* SPDX-License-Identifier: MIT */
/* Suite video, families stop and boundary (plan 06-09, D-14, A-02, A-06): IVS_RUN_F_STOP_ON_FRAME through the public call (the exit-state
 * order, the effective target T' = min(target, E_next), a flagged run equal to run(T'), the pending input event), the frame boundary observed
 * at the unit starts (single steps run(now + 1), never predicted from the cycle table) and the border-register write swept across E_k.
 * Frames end at E_k = (k + 1) x 59,736 ticks (the chain literal's first tick, phase 0). Guests: the stand-ins of video_progs.c; a halted CPU
 * (HLT then NOP then B self: one cycle per unit, boundaries every 4 ticks) and the Phase 4 fault program are built here. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "video_cases.h"

#define VT_SLOT 15296u
#define VT_E 59736u
#define VT_POISON 0xA5u

static ivs_status vt_run(ivs_ms_inst *x, uint32_t flags, uint64_t target, uint8_t *video, uint64_t cap, ivs_run_report *r)
{
  ivs_run_request q;
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.flags = flags;
  q.target_ticks = target;
  q.video = video;
  q.video_capacity = cap;
  memset(r, 0, sizeof *r);
  r->struct_size = (uint32_t)sizeof *r;
  return ivs_machine_run(x->m, &q, r);
}

static size_t vt_prog_halt(uint16_t *w)
{
  w[0] = 0x0000u; /* HLT */
  w[1] = 0x0034u; /* NOP */
  return sc_w_loop(w, 2u);
}

static size_t vt_prog_fault(uint16_t *w)
{
  w[0] = 0x02B9u; /* MVII #$0123, R1 */
  w[1] = 0x0123u;
  w[2] = 0x0264u; /* MVO@ R4, R4: a sticky guest fault (the Phase 4 program F) */
  return 3u;
}

static int vt_inst(ivs_ms_inst *x, size_t (*prog)(uint16_t *))
{
  uint16_t w[SC_PROG_MAX];
  size_t n = prog(w);
  return sc_guest(x, w, n, 0u);
}

/* The exit-state order: INTERNAL (the pre-unit guard, through a synthetic layout whose frame is declared four times too long so the
 * requirement is one slot while two frames complete), GUEST_FAULT, CPU_HALTED (only with its flag), FRAME (flag set, a frame completed; a target
 * equal to the frame end reports FRAME) and TARGET. */
static int vt_frame_reason_order(void)
{
  ivs_ms_inst x;
  ivs_run_report r;
  ivs_run_env env;
  ivs_run_request q;
  uint8_t *buf = (uint8_t *)malloc(VT_SLOT);
  int ok = 0;
  VC_CHECK(buf != NULL);
  /* FRAME, and TARGET below the frame end, and a target equal to the frame end */
  if (!vt_inst(&x, vp_prog_off)) {
    free(buf);
    return IVS_TEST_FAIL;
  }
  if (vt_run(&x, IVS_RUN_F_STOP_ON_FRAME, 1000u, NULL, 0u, &r) != IVS_OK || r.stop_reason != IVS_STOP_TARGET) {
    goto done;
  }
  if (vt_run(&x, IVS_RUN_F_STOP_ON_FRAME, VT_E, NULL, 0u, &r) != IVS_OK || r.stop_reason != IVS_STOP_FRAME) {
    goto done;
  }
  ivs_ms_free(&x);
  /* GUEST_FAULT before the frame, with the flag */
  if (!vt_inst(&x, vt_prog_fault) || vt_run(&x, IVS_RUN_F_STOP_ON_FRAME, IVS_RUN_TARGET_MAX, NULL, 0u, &r) != IVS_OK ||
      r.stop_reason != IVS_STOP_GUEST_FAULT || r.video_frames_completed != 0u) {
    goto fail_inst;
  }
  ivs_ms_free(&x);
  /* CPU_HALTED needs its own flag; the frame flag alone runs on to the frame end */
  if (!vt_inst(&x, vt_prog_halt) || vt_run(&x, IVS_RUN_F_STOP_ON_FRAME | IVS_RUN_F_STOP_ON_HALT, IVS_RUN_TARGET_MAX, NULL, 0u, &r) != IVS_OK ||
      r.stop_reason != IVS_STOP_CPU_HALTED || r.video_frames_completed != 0u) {
    goto fail_inst;
  }
  if (vt_run(&x, IVS_RUN_F_STOP_ON_FRAME, IVS_RUN_TARGET_MAX, NULL, 0u, &r) != IVS_OK || r.stop_reason != IVS_STOP_FRAME) {
    goto fail_inst;
  }
  ivs_ms_free(&x);
  /* INTERNAL: a synthetic layout whose frame is four times the real one (requirement floor(3F / 4F) + 1 = 1 slot, slack 0) while the run
   * completes two frames: the second would not fit, so the guard ends the run at a whole boundary with INTERNAL, after one slot */
  if (!vt_inst(&x, vp_prog_off)) {
    goto fail_inst;
  }
  ivs_machine_profile_env(ivs_profile_get(SC_PROFILE), &env);
  env.nominal_frame_ticks = 4u * (uint64_t)VT_E;
  env.video_slack_frames = 0u;
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.target_ticks = 3u * (uint64_t)VT_E;
  q.video = buf;
  q.video_capacity = VT_SLOT;
  memset(&r, 0, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  if (ivs_machine_run_env_internal(x.m, &q, &r, &env) != IVS_OK || r.stop_reason != IVS_STOP_INTERNAL || r.video_frames_completed != 1u) {
    ivs_test_diag("internal guard: stop %u frames %llu", (unsigned)r.stop_reason, (unsigned long long)r.video_frames_completed);
    goto fail_inst;
  }
  ok = 1;
fail_inst:
done:
  ivs_ms_free(&x);
  free(buf);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* A call that starts exactly at a frame end does not stop at once: the halted CPU gives a unit every 4 ticks, so the first flagged call ends
 * exactly at E_0 and the second runs a whole frame to E_1. */
static int vt_no_stuck_loop(void)
{
  ivs_ms_inst x;
  ivs_run_report r;
  int ok = 0;
  if (!vt_inst(&x, vt_prog_halt)) {
    return IVS_TEST_FAIL;
  }
  if (vt_run(&x, IVS_RUN_F_STOP_ON_FRAME, IVS_RUN_TARGET_MAX, NULL, 0u, &r) != IVS_OK || r.now_ticks != VT_E || r.stop_reason != IVS_STOP_FRAME) {
    goto done;
  }
  if (vt_run(&x, IVS_RUN_F_STOP_ON_FRAME, IVS_RUN_TARGET_MAX, NULL, 0u, &r) != IVS_OK || r.now_ticks != 2u * VT_E ||
      r.stop_reason != IVS_STOP_FRAME || r.elapsed_ticks != VT_E || r.units_executed != VT_E / 4u) {
    ivs_test_diag("second call: now %llu units %llu", (unsigned long long)r.now_ticks, (unsigned long long)r.units_executed);
    goto done;
  }
  ok = 1;
done:
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* With the stop-on-halt bit as well, a halted CPU stops the run before the frame (CPU_HALTED outranks FRAME). */
static int vt_frame_flag_with_halt(void)
{
  ivs_ms_inst x;
  ivs_run_report r;
  int ok;
  if (!vt_inst(&x, vt_prog_halt)) {
    return IVS_TEST_FAIL;
  }
  ok = vt_run(&x, IVS_RUN_F_STOP_ON_FRAME | IVS_RUN_F_STOP_ON_HALT, IVS_RUN_TARGET_MAX, NULL, 0u, &r) == IVS_OK &&
       r.stop_reason == IVS_STOP_CPU_HALTED && r.video_frames_completed == 0u && r.now_ticks < VT_E;
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* A flagged run is byte-identical to run(T'), T' the end of the frame in progress: the instance, the delivered slot and the digests agree;
 * only the stop reason (FRAME against TARGET) and the overshoot (against the caller's target) may differ. */
static int vt_flagged_equals_run_t_prime(void)
{
  ivs_ms_inst a;
  ivs_ms_inst b;
  ivs_run_report ra;
  ivs_run_report rb;
  uint8_t *ba = (uint8_t *)malloc(VT_SLOT);
  uint8_t *bb = (uint8_t *)malloc(3u * VT_SLOT);
  uint8_t da[32];
  uint8_t db[32];
  int ok = 0;
  VC_CHECK(ba != NULL && bb != NULL);
  memset(ba, (int)VT_POISON, VT_SLOT);
  memset(bb, (int)VT_POISON, 3u * VT_SLOT);
  if (!vt_inst(&a, vp_prog_strobe_once)) {
    free(ba);
    free(bb);
    return IVS_TEST_FAIL;
  }
  if (!vt_inst(&b, vp_prog_strobe_once)) {
    ivs_ms_free(&a);
    free(ba);
    free(bb);
    return IVS_TEST_FAIL;
  }
  if (vt_run(&a, IVS_RUN_F_STOP_ON_FRAME, IVS_RUN_TARGET_MAX, ba, VT_SLOT, &ra) != IVS_OK ||
      vt_run(&b, 0u, VT_E, bb, 3u * VT_SLOT, &rb) != IVS_OK) {
    goto done;
  }
  if (ra.stop_reason != IVS_STOP_FRAME || rb.stop_reason != IVS_STOP_TARGET || ra.video_bytes_written != VT_SLOT ||
      rb.video_bytes_written != VT_SLOT) {
    goto done;
  }
  ra.stop_reason = rb.stop_reason = 0u;
  ra.overshoot_ticks = rb.overshoot_ticks = 0u;
  if (memcmp(&ra, &rb, sizeof ra) != 0 || memcmp(a.storage, b.storage, IVS_MACHINE_BYTES) != 0 || memcmp(ba, bb, VT_SLOT) != 0 ||
      ivs_machine_state_digest(a.m, IVS_DIGEST_VIDEO, da) != IVS_OK || ivs_machine_state_digest(b.m, IVS_DIGEST_VIDEO, db) != IVS_OK ||
      memcmp(da, db, 32u) != 0) {
    ivs_test_diag("flagged run differs from run(T')");
    goto done;
  }
  ok = 1;
done:
  ivs_ms_free(&a);
  ivs_ms_free(&b);
  free(ba);
  free(bb);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* A target below the frame end is a plain run to the target: reason TARGET, no frame, the next end reported. */
static int vt_target_below_next_end(void)
{
  ivs_ms_inst x;
  ivs_run_report r;
  int ok;
  if (!vt_inst(&x, vp_prog_off)) {
    return IVS_TEST_FAIL;
  }
  ok = vt_run(&x, IVS_RUN_F_STOP_ON_FRAME, VT_E - 1000u, NULL, 0u, &r) == IVS_OK && r.stop_reason == IVS_STOP_TARGET &&
       r.video_frames_completed == 0u && r.next_frame_end_ticks == VT_E && r.last_frame_end_ticks == 0u && r.now_ticks >= VT_E - 1000u &&
       r.now_ticks < VT_E;
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* A flagged call applies the input events that fall before the effective target and leaves later ones for the next call: the event at tick
 * 100 is consumed in the first call (stop FRAME at E_0), the event at tick 100,000 (after E_0, before E_1) only in the second. */
static int vt_flag_with_pending_input_event(void)
{
  ivs_ms_inst x;
  ivs_run_request q;
  ivs_run_report r;
  ivs_input_event ev[2];
  int ok = 0;
  if (!vt_inst(&x, vp_prog_off)) {
    return IVS_TEST_FAIL;
  }
  memset(ev, 0, sizeof ev);
  ev[0].at_tick = 100u;
  ev[0].controller[0] = IVS_CTRL_KEY_1;
  ev[1].at_tick = 100000u;
  ev[1].controller[0] = 0u;
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.flags = IVS_RUN_F_STOP_ON_FRAME;
  q.target_ticks = IVS_RUN_TARGET_MAX;
  q.events = ev;
  q.event_count = 2u;
  memset(&r, 0, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  if (ivs_machine_run(x.m, &q, &r) != IVS_OK || r.stop_reason != IVS_STOP_FRAME || r.input_consumed != 1u) {
    ivs_test_diag("first call: stop %u consumed %u", (unsigned)r.stop_reason, (unsigned)r.input_consumed);
    goto done;
  }
  q.events = &ev[1];
  q.event_count = 1u;
  memset(&r, 0, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  if (ivs_machine_run(x.m, &q, &r) != IVS_OK || r.stop_reason != IVS_STOP_FRAME || r.input_consumed != 1u) {
    ivs_test_diag("second call: stop %u consumed %u", (unsigned)r.stop_reason, (unsigned)r.input_consumed);
    goto done;
  }
  ok = 1;
done:
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- boundary: the unit that ends at or after E_0 completes the frame ---- */

/* Runs a fresh guest to target (public call, video not requested) and checks the frame count against the observed time: the frame is complete
 * exactly when the call ended at or after E_0, whichever unit boundary that was, and a single-stepped twin agrees at the same time. */
static int vt_boundary(uint64_t target)
{
  ivs_ms_inst a;
  ivs_ms_inst b;
  ivs_run_report r;
  const struct ivs_machine *im;
  int ok = 0;
  if (!vt_inst(&a, vp_prog_off)) {
    return IVS_TEST_FAIL;
  }
  if (!vt_inst(&b, vp_prog_off)) {
    ivs_ms_free(&a);
    return IVS_TEST_FAIL;
  }
  if (vt_run(&a, 0u, target, NULL, 0u, &r) != IVS_OK) {
    goto done;
  }
  if (r.video_frames_completed != (r.now_ticks >= VT_E ? 1u : 0u)) {
    ivs_test_diag("target %llu now %llu frames %llu", (unsigned long long)target, (unsigned long long)r.now_ticks,
                  (unsigned long long)r.video_frames_completed);
    goto done;
  }
  im = ivs_machine_from_handle(b.m);
  while (im->time_ticks < r.now_ticks) { /* the twin: whole units, one call each */
    ivs_run_report t;
    if (vt_run(&b, 0u, im->time_ticks + 1u, NULL, 0u, &t) != IVS_OK) {
      goto done;
    }
  }
  if (im->time_ticks != r.now_ticks || im->obs.video.count != r.video_frames_completed ||
      memcmp(ivs_machine_from_handle(a.m)->obs.video.chain, im->obs.video.chain, 32u) != 0 ||
      memcmp(a.storage, b.storage, IVS_MACHINE_BYTES) != 0) {
    goto done;
  }
  ok = 1;
done:
  ivs_ms_free(&a);
  ivs_ms_free(&b);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int vt_boundary_event_minus_1(void)
{
  return vt_boundary(VT_E - 1u);
}

static int vt_boundary_event(void)
{
  return vt_boundary(VT_E);
}

static int vt_boundary_event_plus_1(void)
{
  return vt_boundary(VT_E + 1u);
}

/* A border-register write swept across E_0 (T-06-07, A-06). The guest stores 5 to $002C after a counted delay (the loop of N iterations of
 * about 60 ticks) and a prefix of a INCR (6 cycles) and b TSTR R6 (7 cycles), so the store stamp moves by single cycles near E_0. A store
 * stamped below E_0 hits the closed register window and is dropped; one stamped at or after E_0 lands in the NEXT frame. Either way the
 * delivered fill of frame 0 is the OLD border (0), and the fill of frame 1 is 0 (dropped) or 5 (landed), monotone in the stamp; a sweep that
 * shows both sides and a frame 0 that never shows 5 is the observation. */
static size_t vt_prog_write(uint16_t *w, uint32_t n_iter, uint32_t a, uint32_t b)
{
  size_t n = sc_w_mvii(w, 0u, 0u, 5u);
  uint32_t i;
  n = sc_w_delay(w, n, n_iter);
  for (i = 0u; i < a; i++) {
    w[n++] = 0x000Du; /* INCR R5 (6 cycles) */
  }
  for (i = 0u; i < b; i++) {
    w[n++] = 0x00B6u; /* TSTR R6 (7 cycles) */
  }
  n = sc_w_mvo(w, n, 0u, 0x002Cu);
  w[n++] = 0x0003u; /* DIS */
  return sc_w_loop(w, n);
}

/* Runs the write guest to tick 125,000 (past E_1) with a four-slot buffer; *f0 and *f1 get the fill bytes of frames 0 and 1; 1 on success. */
static int vt_write_run(uint32_t n_iter, uint32_t a, uint32_t b, uint8_t *f0, uint8_t *f1)
{
  ivs_ms_inst x;
  ivs_run_report r;
  uint16_t w[SC_PROG_MAX * 2u];
  size_t n = vt_prog_write(w, n_iter, a, b);
  uint8_t *buf = (uint8_t *)malloc(4u * VT_SLOT);
  int ok = 0;
  if (buf == NULL) {
    return 0;
  }
  memset(buf, (int)VT_POISON, 4u * VT_SLOT);
  if (sc_guest_n(&x, w, n)) {
    ok = vt_run(&x, 0u, 125000u, buf, 4u * VT_SLOT, &r) == IVS_OK && r.video_frames_completed == 2u;
    if (ok) {
      *f0 = buf[IVS_FRAME_HEADER_BYTES];
      *f1 = buf[VT_SLOT + IVS_FRAME_HEADER_BYTES];
    }
    ivs_ms_free(&x);
  }
  free(buf);
  return ok;
}

static int vt_write_straddling(void)
{
  uint32_t n_iter;
  uint32_t found = 0u;
  uint32_t a;
  uint32_t b;
  uint8_t f0;
  uint8_t f1;
  uint32_t dropped = 0u;
  uint32_t landed = 0u;
  uint32_t seen_landed = 0u;
  /* the first iteration count whose store lands (stamp at or after E_0) */
  for (n_iter = 900u; n_iter < 1100u && found == 0u; n_iter++) {
    VC_CHECK(vt_write_run(n_iter, 0u, 0u, &f0, &f1));
    VC_CHECK(f0 == 0u);
    if (f1 == 5u) {
      found = n_iter;
    }
  }
  VC_CHECK(found != 0u);
  /* one iteration earlier plus 0..3 INCR and 0..3 TSTR: stamps from below to above E_0 in steps of single cycles */
  for (a = 0u; a < 4u; a++) {
    for (b = 0u; b < 4u; b++) {
      VC_CHECK(vt_write_run(found - 2u, a, b, &f0, &f1));
      VC_CHECK(f0 == 0u); /* the frame that ended before the store stamp keeps the old border */
      VC_CHECK(f1 == 0u || f1 == 5u);
      if (f1 == 5u) {
        landed++;
        seen_landed = 1u;
      } else {
        dropped++;
        VC_CHECK(seen_landed == 0u || (a * 6u + b * 7u) > 0u);
      }
    }
  }
  VC_CHECK(dropped > 0u && landed > 0u);
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_video_stop_cases[] = {
    {"video.stop.frame-reason-order", vt_frame_reason_order, NULL},
    {"video.stop.no-stuck-loop-at-boundary", vt_no_stuck_loop, NULL},
    {"video.stop.frame-flag-with-halt", vt_frame_flag_with_halt, NULL},
    {"video.stop.flagged-equals-run-T-prime", vt_flagged_equals_run_t_prime, NULL},
    {"video.stop.target-below-next-end", vt_target_below_next_end, NULL},
    {"video.stop.flag-with-pending-input-event", vt_flag_with_pending_input_event, NULL},
    {"video.boundary.event-minus-1", vt_boundary_event_minus_1, NULL},
    {"video.boundary.event", vt_boundary_event, NULL},
    {"video.boundary.event-plus-1", vt_boundary_event_plus_1, NULL},
    {"video.boundary.write-straddling", vt_write_straddling, NULL},
};
const size_t ivs_video_stop_count = sizeof ivs_video_stop_cases / sizeof ivs_video_stop_cases[0];
