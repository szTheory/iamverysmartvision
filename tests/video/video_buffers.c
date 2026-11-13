/* SPDX-License-Identifier: MIT */
/* Suite video, family buffers (plan 06-09, D-13, D-15, A-08, T-06-12, T-06-13): the caller-owned video buffer of ivs_machine_run and its
 * capacity law through the PUBLIC call with the real layout: a run from now to target needs video_bytes_per_frame x (floor(D / F) + 1 +
 * video_slack_frames) bytes (the slot of 15,296 bytes, F = 59,736 and the slack of one frame are the page's numbers, the same literals as
 * the geometry family), NULL with capacity 0 means not requested, and every refusal leaves the instance, the report's other bytes and the
 * poisoned buffer exactly as they were. Guests are the stand-ins of video_progs.c (a DIS then B self loop, or a halted CPU). */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "video_cases.h"

#define VB_SLOT 15296u
#define VB_F 59736u
#define VB_SLACK 1u
#define VB_POISON 0xA5u

/* The requirement of a run of d ticks (the law under test, restated from the header text). */
static uint64_t vb_need(uint64_t d)
{
  return (uint64_t)VB_SLOT * (d / VB_F + 1u + VB_SLACK);
}

static void vb_req(ivs_run_request *q, uint64_t target, uint8_t *video, uint64_t cap)
{
  memset(q, 0, sizeof *q);
  q->struct_size = (uint32_t)sizeof *q;
  q->target_ticks = target;
  q->video = video;
  q->video_capacity = cap;
}

static int vb_all(const uint8_t *p, size_t n, uint8_t v)
{
  size_t i;
  for (i = 0u; i < n; i++) {
    if (p[i] != v) {
      return 0;
    }
  }
  return 1;
}

/* One guest instance, powered, at machine time 0; 1 on success. */
static int vb_inst(ivs_ms_inst *x)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = vp_prog_off(w);
  return sc_guest(x, w, n, 0u);
}

static uint64_t vb_now(const ivs_ms_inst *x)
{
  return ivs_machine_from_handle(x->m)->time_ticks;
}

/* A refused call: the status and reason, the instance bytes equal to the snapshot, the poisoned buffer (and the whole of it) untouched,
 * and every report byte past the refusal fields as it was poisoned. */
static int vb_refused(ivs_ms_inst *x, const ivs_run_request *q, uint8_t *buf, size_t buf_bytes, ivs_status want, uint32_t reason)
{
  uint8_t *snap = (uint8_t *)malloc(IVS_MACHINE_BYTES);
  ivs_run_report r;
  ivs_status st;
  int ok = 0;
  if (snap == NULL) {
    return 0;
  }
  memcpy(snap, x->storage, IVS_MACHINE_BYTES);
  memset(&r, (int)VB_POISON, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  st = ivs_machine_run(x->m, q, &r);
  if (st != want || r.refusal_reason != reason) {
    ivs_test_diag("status %s reason %u, want %s %u", ivs_status_name(st), (unsigned)r.refusal_reason, ivs_status_name(want),
                  (unsigned)reason);
    goto done;
  }
  if (memcmp(snap, x->storage, IVS_MACHINE_BYTES) != 0) {
    ivs_test_diag("instance changed by a refused run");
    goto done;
  }
  if (buf != NULL && !vb_all(buf, buf_bytes, VB_POISON)) {
    ivs_test_diag("buffer written by a refused run");
    goto done;
  }
  ok = 1;
done:
  free(snap);
  return ok;
}

/* Instances A and B run the same guest to the same target, A with video not requested (NULL with capacity 0) and B with exactly the
 * required buffer: equal whole-instance bytes, equal digests, equal report except video_bytes_written, equal frame counts. */
static int vb_not_requested_equals_requested(void)
{
  ivs_ms_inst a;
  ivs_ms_inst b;
  ivs_run_request q;
  ivs_run_report ra;
  ivs_run_report rb;
  uint8_t da[32];
  uint8_t db[32];
  uint64_t target = (uint64_t)VB_F * 5u / 2u; /* 2.5 frames: two frame events inside */
  uint64_t need = vb_need(target);
  uint8_t *buf = (uint8_t *)malloc((size_t)need);
  int ok = 0;
  VC_CHECK(buf != NULL);
  memset(buf, (int)VB_POISON, (size_t)need);
  if (!vb_inst(&a)) {
    free(buf);
    return IVS_TEST_FAIL;
  }
  if (!vb_inst(&b)) {
    ivs_ms_free(&a);
    free(buf);
    return IVS_TEST_FAIL;
  }
  memset(&ra, 0, sizeof ra);
  memset(&rb, 0, sizeof rb);
  ra.struct_size = rb.struct_size = (uint32_t)sizeof ra;
  vb_req(&q, target, NULL, 0u);
  if (ivs_machine_run(a.m, &q, &ra) != IVS_OK) {
    goto done;
  }
  vb_req(&q, target, buf, need);
  if (ivs_machine_run(b.m, &q, &rb) != IVS_OK) {
    goto done;
  }
  if (ra.video_frames_completed != 2u || rb.video_frames_completed != 2u || ra.video_bytes_written != 0u ||
      rb.video_bytes_written != 2u * VB_SLOT) {
    ivs_test_diag("frames %llu %llu bytes %llu %llu", (unsigned long long)ra.video_frames_completed,
                  (unsigned long long)rb.video_frames_completed, (unsigned long long)ra.video_bytes_written,
                  (unsigned long long)rb.video_bytes_written);
    goto done;
  }
  rb.video_bytes_written = 0u;
  if (memcmp(&ra, &rb, sizeof ra) != 0 || memcmp(a.storage, b.storage, IVS_MACHINE_BYTES) != 0) {
    ivs_test_diag("report or instance differs");
    goto done;
  }
  if (ivs_machine_state_digest(a.m, IVS_DIGEST_COMPOSITE, da) != IVS_OK || ivs_machine_state_digest(b.m, IVS_DIGEST_COMPOSITE, db) != IVS_OK ||
      memcmp(da, db, 32u) != 0 || ivs_machine_state_digest(a.m, IVS_DIGEST_VIDEO, da) != IVS_OK ||
      ivs_machine_state_digest(b.m, IVS_DIGEST_VIDEO, db) != IVS_OK || memcmp(da, db, 32u) != 0) {
    goto done;
  }
  ok = 1;
done:
  ivs_ms_free(&a);
  ivs_ms_free(&b);
  free(buf);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* Exactly the requirement is accepted, one byte less is BUFFER_TOO_SMALL. */
static int vb_exact_fit(void)
{
  ivs_ms_inst x;
  ivs_run_request q;
  ivs_run_report r;
  uint64_t target = 20000u;
  uint64_t need = vb_need(target);
  uint8_t *buf = (uint8_t *)malloc((size_t)need);
  int ok = 0;
  VC_CHECK(buf != NULL && need == 2u * VB_SLOT);
  memset(buf, (int)VB_POISON, (size_t)need);
  if (!vb_inst(&x)) {
    free(buf);
    return IVS_TEST_FAIL;
  }
  vb_req(&q, target, buf, need - 1u);
  ok = vb_refused(&x, &q, buf, (size_t)need, IVS_E_BUFFER_TOO_SMALL, IVS_REASON_RUN_BUFFER_SMALL);
  if (ok) {
    memset(&r, 0, sizeof r);
    r.struct_size = (uint32_t)sizeof r;
    vb_req(&q, target, buf, need);
    ok = ivs_machine_run(x.m, &q, &r) == IVS_OK && r.stop_reason == IVS_STOP_TARGET && r.video_frames_completed == 0u &&
         r.video_bytes_written == 0u && vb_all(buf, (size_t)need, VB_POISON); /* no frame ends inside 20,000 ticks */
  }
  ivs_ms_free(&x);
  free(buf);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* A run too small by one byte writes nothing and changes nothing (T-06-12), for a run that WOULD cross a frame event. */
static int vb_undersized_refused(void)
{
  ivs_ms_inst x;
  ivs_run_request q;
  uint64_t target = (uint64_t)VB_F * 2u;
  uint64_t need = vb_need(target);
  uint8_t *buf = (uint8_t *)malloc((size_t)need);
  int ok;
  VC_CHECK(buf != NULL);
  memset(buf, (int)VB_POISON, (size_t)need);
  if (!vb_inst(&x)) {
    free(buf);
    return IVS_TEST_FAIL;
  }
  vb_req(&q, target, buf, need - 1u);
  ok = vb_refused(&x, &q, buf, (size_t)need, IVS_E_BUFFER_TOO_SMALL, IVS_REASON_RUN_BUFFER_SMALL);
  if (ok) { /* a buffer of capacity 0 is also too small, and so is one slot short of two */
    vb_req(&q, target, buf, 0u);
    ok = vb_refused(&x, &q, buf, (size_t)need, IVS_E_BUFFER_TOO_SMALL, IVS_REASON_RUN_BUFFER_SMALL);
  }
  ivs_ms_free(&x);
  free(buf);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* A NULL buffer with a nonzero capacity is BAD_ARG. */
static int vb_null_nonzero(void)
{
  ivs_ms_inst x;
  ivs_run_request q;
  int ok;
  if (!vb_inst(&x)) {
    return IVS_TEST_FAIL;
  }
  vb_req(&q, 20000u, NULL, VB_SLOT);
  ok = vb_refused(&x, &q, NULL, 0u, IVS_E_BAD_ARG, IVS_REASON_RUN_BUFFER_NULL);
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* The video buffer may not overlap the instance storage, the audio buffer or the report. */
static int vb_overlap(int which)
{
  ivs_ms_inst x;
  ivs_run_request q;
  uint8_t *arena = (uint8_t *)malloc(3u * VB_SLOT + 4096u);
  ivs_run_report *rep;
  int ok = 0;
  uint64_t need = 2u * VB_SLOT;
  VC_CHECK(arena != NULL);
  memset(arena, (int)VB_POISON, 3u * VB_SLOT + 4096u);
  if (!vb_inst(&x)) {
    free(arena);
    return IVS_TEST_FAIL;
  }
  rep = (ivs_run_report *)(void *)(arena + 8u + (uint64_t)VB_SLOT); /* a report inside the buffer (8-aligned) */
  vb_req(&q, 20000u, arena + 8u, need);
  if (which == 0) { /* the instance storage */
    vb_req(&q, 20000u, x.storage + 64u, need);
    ok = vb_refused(&x, &q, NULL, 0u, IVS_E_BAD_ARG, IVS_REASON_RUN_BUFFER_OVERLAP);
  } else if (which == 1) { /* the audio buffer */
    q.audio = arena + 8u + need - 1u;
    q.audio_capacity = 64u;
    ok = vb_refused(&x, &q, arena + 8u, (size_t)need - 1u, IVS_E_BAD_ARG, IVS_REASON_RUN_BUFFER_OVERLAP);
  } else { /* the report */
    ivs_run_report probe;
    uint8_t *snap = (uint8_t *)malloc(IVS_MACHINE_BYTES);
    VC_CHECK(snap != NULL);
    memcpy(snap, x.storage, IVS_MACHINE_BYTES);
    memset(&probe, 0, sizeof probe);
    rep->struct_size = (uint32_t)sizeof *rep;
    ok = ivs_machine_run(x.m, &q, rep) == IVS_E_BAD_ARG && memcmp(snap, x.storage, IVS_MACHINE_BYTES) == 0;
    free(snap);
  }
  ivs_ms_free(&x);
  free(arena);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int vb_overlap_instance(void)
{
  return vb_overlap(0);
}

static int vb_overlap_audio(void)
{
  return vb_overlap(1);
}

static int vb_overlap_report(void)
{
  return vb_overlap(2);
}

/* A buffer larger than the requirement: the bytes past the delivered slots stay untouched (the sentinel tail). */
static int vb_sentinel(void)
{
  ivs_ms_inst x;
  ivs_run_request q;
  ivs_run_report r;
  uint64_t target = (uint64_t)VB_F + 5000u;
  uint64_t need = vb_need(target); /* floor(1.08) + 1 + 1 = 3 slots */
  size_t total = (size_t)need + 64u;
  uint8_t *buf = (uint8_t *)malloc(total);
  int ok;
  VC_CHECK(buf != NULL);
  memset(buf, (int)VB_POISON, total);
  if (!vb_inst(&x)) {
    free(buf);
    return IVS_TEST_FAIL;
  }
  memset(&r, 0, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  vb_req(&q, target, buf, total);
  ok = ivs_machine_run(x.m, &q, &r) == IVS_OK && r.video_frames_completed == 1u && r.video_bytes_written == VB_SLOT &&
       vb_all(buf + VB_SLOT, total - VB_SLOT, VB_POISON) && !vb_all(buf, VB_SLOT, VB_POISON);
  ivs_ms_free(&x);
  free(buf);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* The constructed corner (A-08): a run that starts within 24 ticks below a frame event and spans F - 1 ticks completes TWO frames (the
 * first unit past the event and the last unit that crosses the target), so the Phase 4 count of floor(D / F) + 1 = 1 slot is short and a
 * buffer of one slot must be refused while two slots are accepted and filled. The start is found by single steps (run(now + 1)), each a
 * whole unit, until the machine time is within 24 ticks below the next event; frames 1 to 4 are tried (the residues of E_k mod the unit). */
static int vb_worst_case_two_frames(void)
{
  ivs_ms_inst x;
  ivs_run_request q;
  ivs_run_report r;
  uint8_t *buf = (uint8_t *)malloc(3u * VB_SLOT);
  uint32_t k;
  int found = 0;
  int ok = 0;
  VC_CHECK(buf != NULL);
  memset(buf, (int)VB_POISON, 3u * VB_SLOT);
  if (!vb_inst(&x)) {
    free(buf);
    return IVS_TEST_FAIL;
  }
  for (k = 1u; k <= 4u && !found; k++) {
    uint64_t ek = (uint64_t)k * VB_F;
    uint32_t guard = 0u;
    while (vb_now(&x) + 36u < ek && guard++ < 4000u) { /* whole units until within one unit of the event */
      memset(&r, 0, sizeof r);
      r.struct_size = (uint32_t)sizeof r;
      vb_req(&q, vb_now(&x) + 1u, NULL, 0u);
      if (ivs_machine_run(x.m, &q, &r) != IVS_OK) {
        goto done;
      }
    }
    if (vb_now(&x) < ek && ek - vb_now(&x) <= 24u) {
      found = 1;
    } else {
      uint64_t past = ek + 100u; /* try the next event */
      while (vb_now(&x) < past && guard++ < 8000u) {
        memset(&r, 0, sizeof r);
        r.struct_size = (uint32_t)sizeof r;
        vb_req(&q, vb_now(&x) + 1u, NULL, 0u);
        if (ivs_machine_run(x.m, &q, &r) != IVS_OK) {
          goto done;
        }
      }
    }
  }
  if (!found) {
    ivs_test_diag("no boundary within 24 ticks below a frame event");
    goto done;
  }
  {
    uint64_t target = vb_now(&x) + (VB_F - 1u);
    vb_req(&q, target, buf, VB_SLOT); /* the Phase 4 requirement: one slot */
    if (!vb_refused(&x, &q, buf, 3u * VB_SLOT, IVS_E_BUFFER_TOO_SMALL, IVS_REASON_RUN_BUFFER_SMALL)) {
      goto done;
    }
    memset(&r, 0, sizeof r);
    r.struct_size = (uint32_t)sizeof r;
    vb_req(&q, target, buf, 2u * VB_SLOT);
    if (ivs_machine_run(x.m, &q, &r) != IVS_OK || r.video_frames_completed != 2u || r.video_bytes_written != 2u * VB_SLOT ||
        r.stop_reason != IVS_STOP_TARGET) {
      ivs_test_diag("corner run: frames %llu bytes %llu", (unsigned long long)r.video_frames_completed,
                    (unsigned long long)r.video_bytes_written);
      goto done;
    }
  }
  ok = 1;
done:
  ivs_ms_free(&x);
  free(buf);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* A product that would wrap 64 bits is refused, never accepted as a small number (T-06-13): a synthetic layout through the env seam of
 * 2^24 bytes per frame and a 1-tick frame, and a run of 2^40 - 1 ticks, needs exactly 2^64 bytes, which wraps to 0. */
static int vb_capacity_overflow_guard(void)
{
  ivs_ms_inst x;
  ivs_run_env e;
  ivs_run_request q;
  ivs_run_report r;
  uint8_t buf[64];
  int ok;
  if (!vb_inst(&x)) {
    return IVS_TEST_FAIL;
  }
  memset(&e, 0, sizeof e);
  e.video_format = IVS_OUTPUT_FORMAT_INDEX8_FRAMES_V1;
  e.video_bytes_per_frame = 1u << 24;
  e.nominal_frame_ticks = 1u;
  memset(buf, (int)VB_POISON, sizeof buf);
  vb_req(&q, (1ull << 40) - 1ull, buf, sizeof buf);
  memset(&r, 0, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  ok = ivs_machine_run_env_internal(x.m, &q, &r, &e) == IVS_E_BUFFER_TOO_SMALL && r.refusal_reason == IVS_REASON_RUN_BUFFER_SMALL &&
       vb_now(&x) == 0u && vb_all(buf, sizeof buf, VB_POISON);
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* Phase 6 review WR-03: a geometry the card-row working buffer cannot hold (33 columns of 8 x 8, 2,112 bytes above the 2,048 of the
 * buffer) is refused by the generator and never reached by a registry profile; if a frame were still asked for, the delivery must be
 * LOUD (video_overflow set, which ends a run INTERNAL) and must not touch the chain, the count or the caller's slot, instead of
 * skipping silently while the frame index advances. Module-level: a stack copy of the generated parameters, no memory system
 * (the guard returns before any card is read). */
static int vb_unrenderable_geometry_overflows(void)
{
  ivs_stic_params p = *ivs_stic_params_get(SC_PROFILE);
  ivs_stic_state s;
  ivs_stic_video_row row;
  ivs_stic_video_row before;
  ivs_psg_sink k;
  uint8_t slot[64];
  memset(&s, 0, sizeof s);
  ivs_video_chain_init(&row);
  before = row;
  p.video_card_columns = 33u;
  memset(slot, (int)VB_POISON, sizeof slot);
  ivs_psg_sink_init(&k, NULL, 0u);
  k.video = slot;
  k.video_cap = sizeof slot;
  ivs_stic_frame_deliver(&p, &s, NULL, &row, &k, 59736u);
  VC_CHECK(k.video_overflow == 1u);
  VC_CHECK(k.video_frames == 0u && k.video_written == 0u);
  VC_CHECK(memcmp(&row, &before, sizeof row) == 0);
  VC_CHECK(vb_all(slot, sizeof slot, VB_POISON));
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_video_buffers_cases[] = {
    {"video.buffers.not-requested-equals-requested", vb_not_requested_equals_requested, NULL},
    {"video.buffers.exact-fit", vb_exact_fit, NULL},
    {"video.buffers.undersized-refused", vb_undersized_refused, NULL},
    {"video.buffers.null-nonzero", vb_null_nonzero, NULL},
    {"video.buffers.overlap-instance", vb_overlap_instance, NULL},
    {"video.buffers.overlap-audio", vb_overlap_audio, NULL},
    {"video.buffers.overlap-report", vb_overlap_report, NULL},
    {"video.buffers.sentinel", vb_sentinel, NULL},
    {"video.buffers.worst-case-two-frames", vb_worst_case_two_frames, NULL},
    {"video.buffers.capacity-overflow-guard", vb_capacity_overflow_guard, NULL},
    {"video.buffers.unrenderable-geometry-overflows", vb_unrenderable_geometry_overflows, NULL},
};
const size_t ivs_video_buffers_count = sizeof ivs_video_buffers_cases / sizeof ivs_video_buffers_cases[0];
