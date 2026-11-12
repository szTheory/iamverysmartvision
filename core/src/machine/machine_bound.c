/* SPDX-License-Identifier: MIT */
/* Bounded execution (D-01..D-07, D-09, plans 04-07, 04-10): ivs_machine_run and ivs_run_stop_name
 * through the public header ivs/ivs_run.h, and the internal env seam ivs_machine_run_env_internal
 * (machine/machine.h) that the public call wraps with the profile's own output layout.
 *
 * Validate-then-commit (analog: machine_load.c). ivs_bound_validate reads only const inputs and
 * fills a small stack plan (the request fields it needs); the ladder is one stage per class, the
 * first failing stage wins (D-05):
 *   1. BAD_ARG       req NULL or its struct_size below IVS_RUN_REQUEST_V1_SIZE or above
 *                    IVS_STRUCT_SIZE_MAX (which bounds the stage 4 tail scan); rep NULL or its
 *                    struct_size below IVS_RUN_REPORT_V1_SIZE or above IVS_STRUCT_SIZE_MAX (nothing
 *                    written); env NULL (the seam). The minimums are the pinned v1 sizes, never
 *                    sizeof of the grown structs (plan 05-02, D-19, A-04);
 *   2. BAD_HANDLE    NULL or invalid handle;
 *   3. BAD_ARG       rep overlaps the storage of the (valid) instance, uintptr_t differences only
 *                    (ivs_machine_span_overlaps; nothing written);
 *   4. UNSUPPORTED   req: an unknown flag (IVS_RUN_F_RESERVED_FRAME included), then a nonzero
 *                    byte past the core's own size of a larger struct (the tail rule is rebased on
 *                    sizeof, plan 05-02), then a nonzero reserved_in (INPUT_RESERVED_IN, plan 04-11);
 *   4b. BAD_ARG      output buffers (D-05 slot, plan 04-10): a NULL buffer with a nonzero
 *                    capacity (RUN_BUFFER_NULL; video, then audio), then a buffer of capacity
 *                    bytes overlapping the instance storage, the other buffer or the report
 *                    (RUN_BUFFER_OVERLAP; uintptr_t differences, 64-bit lengths, nothing wraps);
 *                    then the write log (plan 05-02, D-14; read only when struct_size covers it):
 *                    psg_log NULL with a nonzero capacity (RUN_PSGLOG_NULL), then psg_log running
 *                    past the top of the address space or overlapping the instance, the report,
 *                    video, audio or the event array (RUN_PSGLOG_OVERLAP);
 *   4c. BAD_ARG      event arguments (D-05 slot, plan 04-11): events NULL with event_count above 0
 *                    (INPUT_EVENTS_NULL), event_count above IVS_INPUT_EVENTS_MAX (INPUT_EVENT_COUNT),
 *                    then the event_count * 32 bytes at events overlapping the instance storage,
 *                    the report, the video buffer or the audio buffer (INPUT_EVENTS_OVERLAP;
 *                    uintptr_t differences, 64-bit lengths); no event is read here;
 *   5. BAD_STATE     the machine is OFF;
 *   6. OUT_OF_RANGE  target_ticks above IVS_RUN_TARGET_MAX (2^63), before any unit, so now plus
 *                    one unit can never wrap;
 *   7. BUFFER_TOO_SMALL  a video, audio (RUN_BUFFER_SMALL) or write-log (RUN_PSGLOG_SMALL)
 *                    capacity below the env's requirement for this run, bytes_per_frame *
 *                    (max(target - now, 0) / nominal_frame_ticks + 1) (D-06), before the first
 *                    unit. A NULL buffer with capacity 0 means the channel is not requested and
 *                    needs nothing (D-17, plan 05-02);
 *   8. BAD_ARG       the events, whole array (machine_input.c, ivs_input_check): an at_tick at or
 *                    above 2^63 (INPUT_TICK_RANGE), below its predecessor (INPUT_UNSORTED) or below
 *                    the input floor (INPUT_BEFORE_FLOOR), lowest failing index;
 *   9. UNSUPPORTED   the events, whole array: nonzero flags (INPUT_EVENT_FLAGS), a nonzero reserved
 *                    word (INPUT_EVENT_RESERVED), a controller word with IVS_CTRL_RESERVED_MASK set
 *                    (INPUT_CONTROL_RESERVED), a nonzero word at an index at or above the profile's
 *                    input.controllers (INPUT_EXTRA_CONTROLLER, plan 04-12), lowest failing index;
 *                    still before the first unit.
 *                    Stages 8 and 9 also write the failing index (input_failing_index).
 * A refusal writes struct_size and the IVS_REASON_* only into a report that is not the cause, does
 * not overlap the bytes the handle names and does not overlap an output buffer or the event array
 * of a well-sized request (ivs_bound_report_ok), whatever stage refused, and never a byte into
 * either buffer or the array.
 *
 * Commit: while machine time is below the target, step one unit through the internal seam
 * ivs_machine_step_unit_internal (machine_run.c; the seam stays internal). The exit tests come
 * first; only when a unit will begin are the due input events applied (every not yet applied
 * event with at_tick <= the unit's start, in array order; machine_input.c), and the seam then
 * sets the input floor to that start + 1 (A-04, D-14). Nothing is applied when the call ends, so
 * an event with at_tick inside the overshoot window waits for the next call. A FAULTED CPU is never
 * stepped; with IVS_RUN_F_STOP_ON_HALT a HALTED CPU is not stepped; a unit the seam refuses, or a
 * unit that does not advance machine time, ends the loop (INTERNAL unless the CPU is FAULTED), so
 * the loop always terminates. The stop reason is then a pure function of the exit state: INTERNAL,
 * GUEST_FAULT, CPU_HALTED (flag), TARGET. The report is filled last; overshoot is derived. Nothing
 * outlives the call (no budget, deficit or pointer): slicing invariance holds by construction.
 * Phase 4 layouts produce no bytes, so the loop writes nothing into the output buffers and the
 * report's output counts are 0 (no frame is fabricated).
 * The file holds no static or global state.
 * Named faults (plan 04-07, D-07): RUN_TARGET_UNCHECKED, RUN_STOP_BEFORE_OVERSHOOT,
 * RUN_FAULT_STEPS; (plan 04-09, D-07): RUN_BUDGET_CARRY, the only static, inside its fault block;
 * (plan 04-10, D-06, D-07): RUN_CAPACITY_AFTER_STEP, OUTBUF_PARTIAL_WRITE; (plan 04-11, D-05,
 * D-16): INPUT_APPLY_AT_RUN_START, RUN_EVENTS_UNCHECKED; (plan 05-02, A-02, D-14):
 * RUN_REPORT_TAIL_UNGUARDED, RUN_REQUEST_V2_READ_UNGUARDED, RUN_PSGLOG_OVERLAP_UNCHECKED. */
#include "common/ivs_prelude.h"

#include "ivs/ivs.h"
#include "machine/machine.h"

/* The request carries pointers, so only its pointer-free prefix has a fixed offset on every data
 * model (Pitfall 13); its size is bounded by the struct cap. The report is pointer-free. */
_Static_assert(offsetof(ivs_run_request, video) == 16u, "ivs_run_request: the buffers follow the 16-byte prefix");
_Static_assert(sizeof(ivs_run_request) <= IVS_STRUCT_SIZE_MAX, "ivs_run_request within the struct cap");
#if UINTPTR_MAX == 0xFFFFFFFFFFFFFFFFu
_Static_assert(offsetof(ivs_run_request, psg_log) == IVS_RUN_REQUEST_V1_SIZE, "request v2 starts at the v1 size");
_Static_assert(offsetof(ivs_run_request, psg_log_capacity) == 72u, "psg_log_capacity at offset 72");
_Static_assert(sizeof(ivs_run_request) == 80u, "ivs_run_request is 80 bytes (v2, 64-bit pointers)");
#endif
_Static_assert(IVS_RUN_REPORT_V1_SIZE == 112u, "the pinned v1 report size is 112");
_Static_assert(offsetof(ivs_run_report, audio_first_index) == IVS_RUN_REPORT_V1_SIZE, "report v2 starts at the v1 size");
_Static_assert(offsetof(ivs_run_report, psg_log_bytes_written) == 120u, "psg_log_bytes_written at offset 120");
_Static_assert(offsetof(ivs_run_report, psg_log_first_index) == 128u, "psg_log_first_index at offset 128");
_Static_assert(sizeof(ivs_run_report) == 136u, "ivs_run_report is 136 bytes (v2)");
_Static_assert(offsetof(ivs_run_report, now_ticks) % 8u == 0u, "64-bit report members are 8-aligned");

/* One byte past the last v2 request field: a request whose struct_size reaches it carries
 * psg_log and psg_log_capacity (plan 05-02). A smaller (v1-sized) caller never has them read. */
#define IVS_BOUND_REQ_LOG_END (offsetof(ivs_run_request, psg_log_capacity) + sizeof(uint64_t))

#ifdef IVS_FAULT_RUN_BUDGET_CARRY
/* Named fault RUN_BUDGET_CARRY (plan 04-09, D-07): a hidden carry between calls. The overshoot of
 * the last call that ran a unit is kept at file scope and, when the next call continues the same
 * handle from the boundary that call ended at, taken off its target ("those ticks already ran"),
 * so a sliced run ends at a different boundary than one call. The slicing-invariance family
 * (run.slice.*) must catch it. */
static const void *ivs_bound_carry_handle;
static uint64_t ivs_bound_carry_end;
static uint64_t ivs_bound_carry_ticks;
#endif

/* What the commit loop needs from a validated request. */
typedef struct ivs_bound_plan {
  uint64_t target;
  const ivs_input_event *events;
  uint32_t event_count;
  uint32_t flags;
} ivs_bound_plan;

/* 1 when req's v1 buffer fields may be read: present and at least the pinned v1 size (stage 1). */
static uint32_t ivs_bound_req_sized(const ivs_run_request *req)
{
  return req != NULL && req->struct_size >= IVS_RUN_REQUEST_V1_SIZE && req->struct_size <= IVS_STRUCT_SIZE_MAX ? 1u
                                                                                                             : 0u;
}

/* The write-log buffer of a request and its capacity: read only when struct_size covers the v2
 * fields (plan 05-02, A-02), else NULL and 0 (the channel is not requested). req is sized. */
static uint8_t *ivs_bound_log_ptr(const ivs_run_request *req)
{
#ifdef IVS_FAULT_RUN_REQUEST_V2_READ_UNGUARDED
  /* Named fault RUN_REQUEST_V2_READ_UNGUARDED (plan 05-02, A-02): the appended request fields are
   * read even when struct_size is the v1 size, past a v1-sized caller's struct. */
  return req->psg_log;
#else
  return req->struct_size >= IVS_BOUND_REQ_LOG_END ? req->psg_log : NULL;
#endif
}

static uint64_t ivs_bound_log_cap(const ivs_run_request *req)
{
#ifdef IVS_FAULT_RUN_REQUEST_V2_READ_UNGUARDED
  return req->psg_log_capacity;
#else
  return req->struct_size >= IVS_BOUND_REQ_LOG_END ? req->psg_log_capacity : 0u;
#endif
}

/* The bytes the core writes into rep on success or refusal: min(caller struct_size, own size)
 * (A-02, plan 05-02), the same rule as ivs_machine_info. rep is sized (stage 1). */
static uint32_t ivs_bound_rep_bytes(const ivs_run_report *rep)
{
#ifdef IVS_FAULT_RUN_REPORT_TAIL_UNGUARDED
  /* Named fault RUN_REPORT_TAIL_UNGUARDED (plan 05-02, A-02): the report is written (and its
   * span tested) at the core's full size whatever the caller's struct_size, past a v1-sized
   * caller's struct. */
  (void)rep;
  return (uint32_t)sizeof(ivs_run_report);
#else
  return rep->struct_size < (uint32_t)sizeof(ivs_run_report) ? rep->struct_size : (uint32_t)sizeof(ivs_run_report);
#endif
}

/* 1 when capacity holds the bytes one channel needs for a run from now to target (D-06):
 * bytes_per_frame * (max(target - now, 0) / frame + 1). Nothing is needed at 0 bytes per frame;
 * a frame of 0 ticks, or a product above UINT64_MAX, can never be met. target is at most 2^63
 * here (OUT_OF_RANGE ran first); the frame count is still guarded against wrapping.
 * D-17 (plan 05-02): a NULL buffer with capacity 0 means the channel is not requested, so it needs
 * nothing whatever the layout says; a nonzero capacity below the requirement is still refused, and
 * so is a non-NULL buffer of capacity 0 against a layout that needs bytes (Phase 4 behaviour). */
static uint32_t ivs_bound_fits(const void *buf, uint64_t capacity, uint32_t bytes_per_frame, uint64_t frame,
                               uint64_t target, uint64_t now)
{
  uint64_t frames;
  if (buf == NULL && capacity == 0u) {
    return 1u;
  }
  if (bytes_per_frame == 0u) {
    return 1u;
  }
  if (frame == 0u) {
    return 0u;
  }
  frames = (target > now ? target - now : 0u) / frame;
  if (frames == UINT64_MAX) {
    return 0u; /* unreachable below 2^63, kept so the + 1 can never wrap */
  }
  frames += 1u;
  if (frames > UINT64_MAX / bytes_per_frame) {
    return 0u;
  }
  return capacity >= frames * bytes_per_frame ? 1u : 0u;
}

/* IVS_REASON_NONE when the three channels hold what the run from now to target needs under env
 * (video, audio, psg_log in that order), else the first short channel's reason: RUN_BUFFER_SMALL
 * for video or audio, RUN_PSGLOG_SMALL for the write log. */
static uint32_t ivs_bound_capacity_reason(const ivs_run_request *req, const ivs_run_env *env, uint64_t target,
                                          uint64_t now)
{
  if (ivs_bound_fits(req->video, req->video_capacity, env->video_bytes_per_frame, env->nominal_frame_ticks, target,
                     now) == 0u ||
      ivs_bound_fits(req->audio, req->audio_capacity, env->audio_bytes_per_frame, env->nominal_frame_ticks, target,
                     now) == 0u) {
    return IVS_REASON_RUN_BUFFER_SMALL;
  }
  if (ivs_bound_fits(ivs_bound_log_ptr(req), ivs_bound_log_cap(req), env->psg_log_bytes_per_frame,
                     env->nominal_frame_ticks, target, now) == 0u) {
    return IVS_REASON_RUN_PSGLOG_SMALL;
  }
  return IVS_REASON_NONE;
}

/* 1 when every byte of p[from, to) is zero. */
static uint32_t ivs_bound_tail_zero(const uint8_t *p, size_t from, size_t to)
{
  size_t i;
  for (i = from; i < to; i++) {
    if (p[i] != 0u) {
      return 0u;
    }
  }
  return 1u;
}

/* 1 when the na bytes at a and the nb bytes at b share a byte: uintptr_t differences compared
 * with 64-bit lengths, so a capacity above SIZE_MAX or one that runs past the end of the address
 * space cannot wrap. A NULL pointer or a length 0 names no byte (a NULL buffer with a capacity is
 * refused by its own check first). */
static uint32_t ivs_bound_spans_meet(const void *a, uint64_t na, const void *b, uint64_t nb)
{
  uintptr_t x = (uintptr_t)a;
  uintptr_t y = (uintptr_t)b;
  if (a == NULL || b == NULL || na == 0u || nb == 0u) {
    return 0u;
  }
  if (x >= y) {
    return (uint64_t)(x - y) < nb ? 1u : 0u;
  }
  return (uint64_t)(y - x) < na ? 1u : 0u;
}

/* 1 when the n bytes at p run past the top of the address space (review IN-02): such a span
 * names no real object, so a buffer or event array that does is refused as overlapping before
 * the pairwise tests (whose arithmetic does not wrap but would miss the wrapped part). */
static uint32_t ivs_bound_span_wraps(const void *p, uint64_t n)
{
  if (p == NULL || n == 0u) {
    return 0u;
  }
  return n - 1u > (uint64_t)(UINTPTR_MAX - (uintptr_t)p) ? 1u : 0u;
}

/* The bytes the event array of req names: event_count * 32 in 64 bits, or 0 when event_count is
 * above IVS_INPUT_EVENTS_MAX (such a count names no array the core honours: it is refused
 * INPUT_EVENT_COUNT before any overlap test, and the report rule must not depend on where a span
 * of up to 2^37 bytes happens to reach in the caller's address space). */
static uint64_t ivs_bound_events_bytes(const ivs_run_request *req)
{
  if (req->event_count > IVS_INPUT_EVENTS_MAX) {
    return 0u;
  }
  return (uint64_t)req->event_count * (uint64_t)sizeof(ivs_input_event);
}

/* 1 when the caller's report may receive refusal detail: present, a size the core accepts, not
 * overlapping the instance bytes the handle names and not overlapping an output buffer of a
 * well-sized request (the report is then part of the cause, and a buffer gets no byte). The
 * handle test uses only the handle's address (uintptr_t differences, nothing read through it), so
 * a report inside the storage of an invalid or destroyed instance gets nothing either. */
static uint32_t ivs_bound_report_ok(const ivs_machine *m, const ivs_run_request *req, const ivs_run_report *rep)
{
  uint32_t n;
  if (rep == NULL || rep->struct_size < IVS_RUN_REPORT_V1_SIZE || rep->struct_size > IVS_STRUCT_SIZE_MAX) {
    return 0u;
  }
  n = ivs_bound_rep_bytes(rep); /* the bytes a write would cover, not the caller's whole struct */
  if (m != NULL && ivs_machine_span_overlaps(rep, n, m, sizeof(struct ivs_machine)) != 0u) {
    return 0u;
  }
  if (ivs_bound_req_sized(req) != 0u &&
      (ivs_bound_spans_meet(rep, n, req->video, req->video_capacity) != 0u ||
       ivs_bound_spans_meet(rep, n, req->audio, req->audio_capacity) != 0u ||
       ivs_bound_spans_meet(rep, n, ivs_bound_log_ptr(req), ivs_bound_log_cap(req)) != 0u ||
       ivs_bound_spans_meet(rep, n, req->events, ivs_bound_events_bytes(req)) != 0u)) {
    return 0u;
  }
  return 1u;
}

/* The D-05 buffer slot (stage 4b): IVS_REASON_NONE, or the reason the buffers are refused. Video
 * and audio first (a NULL buffer with a capacity, then the overlap tests), then the write log in
 * the same two steps (plan 05-02, D-14), so any video or audio problem is reported before any
 * psg_log problem. The log's overlap list also holds the event array: it is tested here, whatever
 * the events stage would say (an array the core honours is at most IVS_INPUT_EVENTS_MAX events;
 * a larger count names no span). Only the bytes the core would write into the report count. */
static uint32_t ivs_bound_buffers(const struct ivs_machine *im, const ivs_run_request *req, const ivs_run_report *rep)
{
  const uint8_t *log = ivs_bound_log_ptr(req);
  uint64_t logn = ivs_bound_log_cap(req);
  uint32_t repn = ivs_bound_rep_bytes(rep);
  if ((req->video == NULL && req->video_capacity != 0u) || (req->audio == NULL && req->audio_capacity != 0u)) {
    return IVS_REASON_RUN_BUFFER_NULL;
  }
  if (ivs_bound_span_wraps(req->video, req->video_capacity) != 0u ||
      ivs_bound_span_wraps(req->audio, req->audio_capacity) != 0u ||
      ivs_bound_spans_meet(req->video, req->video_capacity, im, sizeof *im) != 0u ||
      ivs_bound_spans_meet(req->audio, req->audio_capacity, im, sizeof *im) != 0u ||
      ivs_bound_spans_meet(req->video, req->video_capacity, req->audio, req->audio_capacity) != 0u ||
      ivs_bound_spans_meet(req->video, req->video_capacity, rep, repn) != 0u ||
      ivs_bound_spans_meet(req->audio, req->audio_capacity, rep, repn) != 0u) {
    return IVS_REASON_RUN_BUFFER_OVERLAP;
  }
  if (log == NULL && logn != 0u) {
    return IVS_REASON_RUN_PSGLOG_NULL;
  }
#ifndef IVS_FAULT_RUN_PSGLOG_OVERLAP_UNCHECKED /* named fault RUN_PSGLOG_OVERLAP_UNCHECKED: the tests are skipped */
  if (ivs_bound_span_wraps(log, logn) != 0u || ivs_bound_spans_meet(log, logn, im, sizeof *im) != 0u ||
      ivs_bound_spans_meet(log, logn, rep, repn) != 0u ||
      ivs_bound_spans_meet(log, logn, req->video, req->video_capacity) != 0u ||
      ivs_bound_spans_meet(log, logn, req->audio, req->audio_capacity) != 0u ||
      ivs_bound_spans_meet(log, logn, req->events, ivs_bound_events_bytes(req)) != 0u) {
    return IVS_REASON_RUN_PSGLOG_OVERLAP;
  }
#else
  (void)im;
  (void)rep;
  (void)repn;
#endif
  return IVS_REASON_NONE;
}

/* The D-05 event argument slot (stage 4c): IVS_REASON_NONE, or the reason the event array is
 * refused. Nothing of the array is read here. */
static uint32_t ivs_bound_events(const struct ivs_machine *im, const ivs_run_request *req, const ivs_run_report *rep)
{
  uint64_t n = ivs_bound_events_bytes(req);
  if (req->events == NULL && req->event_count != 0u) {
    return IVS_REASON_INPUT_EVENTS_NULL;
  }
  if (req->event_count > IVS_INPUT_EVENTS_MAX) {
    return IVS_REASON_INPUT_EVENT_COUNT;
  }
#ifndef IVS_FAULT_RUN_EVENTS_UNCHECKED /* named fault RUN_EVENTS_UNCHECKED: the overlap test is skipped */
  if (ivs_bound_span_wraps(req->events, n) != 0u || ivs_bound_spans_meet(req->events, n, im, sizeof *im) != 0u ||
      ivs_bound_spans_meet(req->events, n, rep, ivs_bound_rep_bytes(rep)) != 0u ||
      ivs_bound_spans_meet(req->events, n, req->video, req->video_capacity) != 0u ||
      ivs_bound_spans_meet(req->events, n, req->audio, req->audio_capacity) != 0u) {
    return IVS_REASON_INPUT_EVENTS_OVERLAP;
  }
#else
  (void)n;
  (void)im;
  (void)rep;
#endif
  return IVS_REASON_NONE;
}

/* The ladder (header comment). Reads only const inputs; on IVS_OK the plan is filled. *index is
 * the failing event of a per-event refusal, else IVS_INPUT_INDEX_NONE. */
static ivs_status ivs_bound_validate(const ivs_machine *m, const ivs_run_request *req, const ivs_run_report *rep,
                                     const ivs_run_env *env, ivs_bound_plan *plan, uint32_t *reason,
                                     uint32_t *index)
{
  ivs_status st;
  uint32_t controllers;
  const struct ivs_machine *im;
  /* 1. BAD_ARG: NULL arguments and struct sizes. */
  if (rep == NULL || rep->struct_size < IVS_RUN_REPORT_V1_SIZE || rep->struct_size > IVS_STRUCT_SIZE_MAX) {
    *reason = IVS_REASON_RUN_STRUCT_SIZE;
    return IVS_E_BAD_ARG;
  }
  if (req == NULL || env == NULL) {
    *reason = IVS_REASON_RUN_NULL_ARG;
    return IVS_E_BAD_ARG;
  }
  if (req->struct_size < IVS_RUN_REQUEST_V1_SIZE || req->struct_size > IVS_STRUCT_SIZE_MAX) {
    *reason = IVS_REASON_RUN_STRUCT_SIZE;
    return IVS_E_BAD_ARG;
  }
  /* 2. BAD_HANDLE. */
  im = ivs_machine_from_handle(m);
  if (im == NULL) {
    *reason = IVS_REASON_HANDLE;
    return IVS_E_BAD_HANDLE;
  }
  /* 3. BAD_ARG: the report overlaps the instance storage (the bytes the core would write). */
  if (ivs_machine_span_overlaps(rep, ivs_bound_rep_bytes(rep), im, sizeof *im) != 0u) {
    *reason = IVS_REASON_RUN_REPORT_OVERLAP;
    return IVS_E_BAD_ARG;
  }
  /* 4. UNSUPPORTED: unknown flags, then a nonzero tail of a larger request. */
  if ((req->flags & ~IVS_RUN_F_STOP_ON_HALT) != 0u) {
    *reason = IVS_REASON_RUN_FLAGS;
    return IVS_E_UNSUPPORTED;
  }
  /* The tail rule is rebased on the core's own size (A-02, plan 05-02): a caller between the v1
   * size and sizeof is valid; one above sizeof must be zero past it. */
  if (ivs_bound_tail_zero((const uint8_t *)req, sizeof(ivs_run_request), (size_t)req->struct_size) == 0u) {
    *reason = IVS_REASON_RUN_TAIL;
    return IVS_E_UNSUPPORTED;
  }
  if (req->reserved_in != 0u) {
    *reason = IVS_REASON_INPUT_RESERVED_IN;
    return IVS_E_UNSUPPORTED;
  }
  /* 4b. BAD_ARG: the output buffers (D-05 slot). */
  *reason = ivs_bound_buffers(im, req, rep);
  if (*reason != IVS_REASON_NONE) {
    return IVS_E_BAD_ARG;
  }
  /* 4c. BAD_ARG: the event arguments (D-05 slot, plan 04-11). */
  *reason = ivs_bound_events(im, req, rep);
  if (*reason != IVS_REASON_NONE) {
    return IVS_E_BAD_ARG;
  }
  /* 5. BAD_STATE: the machine is OFF. */
  if (im->phase != IVS_PHASE_POWERED) {
    *reason = IVS_REASON_RUN_STATE_OFF;
    return IVS_E_BAD_STATE;
  }
  /* 6. OUT_OF_RANGE: the target. */
#ifndef IVS_FAULT_RUN_TARGET_UNCHECKED /* named fault RUN_TARGET_UNCHECKED: the stage is skipped */
  if (req->target_ticks > IVS_RUN_TARGET_MAX) {
    *reason = IVS_REASON_RUN_TARGET_RANGE;
    return IVS_E_OUT_OF_RANGE;
  }
#endif
  /* 7. BUFFER_TOO_SMALL: the output capacities against the env's requirement, before any unit. */
#ifndef IVS_FAULT_RUN_CAPACITY_AFTER_STEP /* named fault: the check moves after the first unit */
  *reason = ivs_bound_capacity_reason(req, env, req->target_ticks, im->time_ticks);
  if (*reason != IVS_REASON_NONE) {
    return IVS_E_BUFFER_TOO_SMALL;
  }
#endif
  /* 8, 9. The events one by one (D-14): order, range and floor (BAD_ARG) over the whole array,
   * then flags, reserved word, control bits and words of unconnected controllers (UNSUPPORTED;
   * the profile's input.controllers, plan 04-12); every event is read before any write. The
   * profile index is registered: the handle check covers it (WR-03). */
  controllers = ivs_profile_get(im->profile)->input_controllers;
  *reason = ivs_input_check(&im->obs.input, req->events, req->event_count, controllers, &st, index);
  if (*reason != IVS_REASON_NONE) {
    return st;
  }
  plan->target = req->target_ticks;
  plan->events = req->events;
  plan->event_count = req->event_count;
  plan->flags = req->flags;
  return IVS_OK;
}

/* The stop reason of an exit state (pure: fault, then halt with the flag, then target). */
static uint32_t ivs_bound_stop(const struct ivs_machine *im, uint32_t flags, uint64_t target)
{
#ifdef IVS_FAULT_RUN_FAULT_STEPS /* and the stop order tests the target first */
  if (im->time_ticks >= target) {
    return IVS_STOP_TARGET; /* the target is checked before the guest state */
  }
#endif
  if (im->cpu.run_state == IVS_CP1610_RS_FAULTED) {
    return IVS_STOP_GUEST_FAULT;
  }
  if ((flags & IVS_RUN_F_STOP_ON_HALT) != 0u && im->cpu.run_state == IVS_CP1610_RS_HALTED) {
    return IVS_STOP_CPU_HALTED;
  }
  (void)target;
  return IVS_STOP_TARGET;
}

/* Refusal detail only into a report that is not itself the cause; no byte into a buffer. index is
 * written only for a refusal of one event (not IVS_INPUT_INDEX_NONE). */
static void ivs_bound_refuse(const ivs_machine *m, const ivs_run_request *req, ivs_run_report *rep, uint32_t reason,
                             uint32_t index)
{
#ifdef IVS_FAULT_OUTBUF_PARTIAL_WRITE
  /* Named fault OUTBUF_PARTIAL_WRITE (plan 04-10, D-06, D-07): a refused run writes one byte into
   * the caller's video buffer (the poison checks of every buffer refusal must see it). */
  if (ivs_bound_req_sized(req) != 0u && req->video != NULL && req->video_capacity != 0u) {
    req->video[0] = 0u;
  }
#endif
  if (ivs_bound_report_ok(m, req, rep) != 0u) {
    rep->struct_size = ivs_bound_rep_bytes(rep); /* inside the v1 prefix: struct_size, reason, index */
    rep->refusal_reason = reason;
    if (index != IVS_INPUT_INDEX_NONE) {
      rep->input_failing_index = index;
    }
  }
}

ivs_status ivs_machine_run_env_internal(ivs_machine *m, const ivs_run_request *req, ivs_run_report *rep,
                                        const ivs_run_env *env)
{
  ivs_bound_plan plan;
  ivs_run_report r;
  struct ivs_machine *im;
  ivs_status st;
  uint32_t reason = IVS_REASON_NONE;
  uint32_t index = IVS_INPUT_INDEX_NONE;
  uint32_t internal = 0u;
  uint64_t entry;
  uint64_t units = 0u;
  uint32_t consumed = 0u;
  uint32_t n;
  memset(&plan, 0, sizeof plan);
  st = ivs_bound_validate(m, req, rep, env, &plan, &reason, &index);
  if (st != IVS_OK) {
    ivs_bound_refuse(m, req, rep, reason, index);
    return st;
  }
  im = ivs_machine_from_handle_mut(m);
  entry = im->time_ticks;
#ifdef IVS_FAULT_RUN_BUDGET_CARRY
  if (ivs_bound_carry_handle == (const void *)m && ivs_bound_carry_end == entry) {
    plan.target = plan.target > ivs_bound_carry_ticks ? plan.target - ivs_bound_carry_ticks : 0u;
  }
#endif
#ifdef IVS_FAULT_INPUT_APPLY_AT_RUN_START
  /* Named fault INPUT_APPLY_AT_RUN_START (plan 04-11, D-16): every event the call's range reaches
   * (at_tick below the target) is applied once at call start instead of before each unit. */
  if (plan.target > 0u) {
    consumed = ivs_input_apply_due(im, plan.events, plan.event_count, 0u, plan.target - 1u, entry);
  }
#endif
  while (im->time_ticks < plan.target) {
    ivs_cp1610_result unit;
    uint64_t before = im->time_ticks;
    ivs_machine_obs_input input_before = im->obs.input; /* restored if the seam refuses (IN-01) */
    uint32_t consumed_before = consumed;
#ifdef IVS_FAULT_RUN_STOP_BEFORE_OVERSHOOT
    /* Named fault RUN_STOP_BEFORE_OVERSHOOT: a unit that would end past the target is taken back
     * (CPU and time; its memory writes stay), so the run stops at the last boundary below it. */
    ivs_cp1610 cpu_before = im->cpu;
#endif
#ifndef IVS_FAULT_RUN_FAULT_STEPS /* named fault RUN_FAULT_STEPS: a faulted CPU is stepped */
    if (im->cpu.run_state == IVS_CP1610_RS_FAULTED) {
      break; /* a faulted CPU is never stepped */
    }
#endif
    if ((plan.flags & IVS_RUN_F_STOP_ON_HALT) != 0u && im->cpu.run_state == IVS_CP1610_RS_HALTED) {
      break;
    }
    /* A unit will begin at before: apply every due event first (A-04, D-14); the step seam then
     * sets the input floor to before + 1. */
#ifndef IVS_FAULT_INPUT_APPLY_AT_RUN_START
    consumed = ivs_input_apply_due(im, plan.events, plan.event_count, consumed, before, entry);
#endif
    memset(&unit, 0, sizeof unit);
    if (ivs_machine_step_unit_internal(m, &unit) != IVS_OK) {
      /* The seam refused before any write: the events applied for this unit are taken back, so
       * nothing changed and the state is the last whole boundary (review IN-01). */
      im->obs.input = input_before;
      ivs_input_rederive(im); /* the masks follow the restored words (plan 05-05, T-05-12) */
      consumed = consumed_before;
      internal = 1u;
      break;
    }
#ifdef IVS_FAULT_RUN_STOP_BEFORE_OVERSHOOT
    if (im->time_ticks > plan.target) {
      im->cpu = cpu_before;
      im->time_ticks = before;
      break;
    }
#endif
    units++;
#ifdef IVS_FAULT_RUN_CAPACITY_AFTER_STEP
    /* Named fault RUN_CAPACITY_AFTER_STEP (plan 04-10, D-06, D-07): the capacity check runs after
     * the first unit, so an undersized buffer is refused with that unit already executed. */
    if (units == 1u) {
      uint32_t small = ivs_bound_capacity_reason(req, env, plan.target, entry);
      if (small != IVS_REASON_NONE) {
        ivs_bound_refuse(m, req, rep, small, IVS_INPUT_INDEX_NONE);
        return IVS_E_BUFFER_TOO_SMALL;
      }
    }
#endif
    if (im->time_ticks == before) {
      /* No progress: never loop on it. A FAULTED CPU (a zero-cycle fault unit) is a guest stop. */
      internal = im->cpu.run_state == IVS_CP1610_RS_FAULTED ? 0u : 1u;
      break;
    }
  }
  n = ivs_bound_rep_bytes(rep); /* before rep is written: the caller's size, capped at our own */
  memset(&r, 0, sizeof r);
  r.struct_size = n;
  r.stop_reason = internal != 0u ? IVS_STOP_INTERNAL : ivs_bound_stop(im, plan.flags, plan.target);
  r.refusal_reason = IVS_REASON_NONE;
  r.cpu_run_state = im->cpu.run_state;
  r.cpu_fault_reason = im->cpu.fault_reason;
  r.now_ticks = im->time_ticks;
  r.overshoot_ticks = im->time_ticks > plan.target ? im->time_ticks - plan.target : 0u;
  r.elapsed_ticks = im->time_ticks - entry;
  r.units_executed = units;
  /* Output counts (D-03): Phase 4 layouts produce no bytes, so they stay 0 (memset above). */
  r.input_consumed = consumed;
  r.input_failing_index = IVS_INPUT_INDEX_NONE;
  r.input_floor_tick = im->obs.input.floor_tick;
  memcpy(r.controller_now, im->obs.input.words, sizeof r.controller_now);
#ifdef IVS_FAULT_RUN_BUDGET_CARRY
  if (units > 0u) {
    ivs_bound_carry_ticks = r.overshoot_ticks;
  }
  ivs_bound_carry_handle = (const void *)m;
  ivs_bound_carry_end = im->time_ticks;
#endif
  memcpy(rep, &r, n); /* never past the caller's struct (A-02): a v1-sized caller gets 112 bytes */
  return IVS_OK;
}

ivs_status ivs_machine_run(ivs_machine *m, const ivs_run_request *req, ivs_run_report *rep)
{
  ivs_run_env env;
  const struct ivs_machine *im = ivs_machine_from_handle(m);
  const ivs_profile_desc *p = im != NULL ? ivs_profile_get(im->profile) : NULL;
  /* The profile's own layout; an invalid handle gets an empty env and is refused BAD_HANDLE by
   * the ladder (its earlier stages first) exactly as before. */
  memset(&env, 0, sizeof env);
  if (p != NULL) {
    ivs_machine_profile_env(p, &env);
  }
  return ivs_machine_run_env_internal(m, req, rep, &env);
}

const char *ivs_run_stop_name(uint32_t stop)
{
  switch (stop) {
  case IVS_STOP_TARGET:
    return "IVS_STOP_TARGET";
  case IVS_STOP_CPU_HALTED:
    return "IVS_STOP_CPU_HALTED";
  case IVS_STOP_GUEST_FAULT:
    return "IVS_STOP_GUEST_FAULT";
  case IVS_STOP_INTERNAL:
    return "IVS_STOP_INTERNAL";
  default:
    return "IVS_STOP_UNKNOWN";
  }
}
