/* SPDX-License-Identifier: MIT */
/* Suite psg, family buffers (AUD-02, plan 05-06, D-16, D-17, T-05-15, T-05-16): the audio buffer rules of the
 * public run call with the real layout and a guest that makes sound (stand-in program TONE: channel A silent
 * until step 4096, then level 15 toggling every step, see machine_standin.c). The audio requirement of a run
 * from machine time 0 to T is audio_bytes_per_frame x (T / nominal_frame_ticks + 1) with both factors read from
 * ivs_info; the layout itself is judged by the run suite (run.info.audio-fields).
 *   psg.buffers.too-small-no-write            capacity one byte below the requirement: BUFFER_TOO_SMALL, the whole
 *                                             buffer still holds its sentinel, storage and digests unchanged
 *   psg.buffers.exact-fit                     capacity exactly the requirement: delivered, 4 bytes per chip step
 *                                             stamped on the 16-tick grid, levels as the guest programs them
 *   psg.buffers.not-requested                 NULL with capacity 0: no bytes, the first index still advances
 *   psg.buffers.state-independent-of-buffer   with and without a buffer, over slices, whole storage and every
 *                                             digest are equal after every call (the fault
 *                                             AUDIO_STATE_DEPENDS_ON_BUFFER breaks this)
 *   psg.buffers.sentinel-tail                 bytes past audio_bytes_written, inside the capacity, are untouched
 *   psg.buffers.overlap                       an audio buffer that overlaps the instance, the report, the video
 *                                             buffer, the event array or the write log is refused as in Phase 4
 *                                             with nothing written or changed */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs/ivs_run.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "machine_cases.h"
#include "psg_cases.h"

#define PBF_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      goto done;                                                                                       \
    }                                                                                                  \
  } while (0)

#define PBF_SENTINEL 0xA5u
#define PBF_TARGET 70000u /* 65,536 + 4,464: past the first edge at step 4096 (tick 65,536), two frames */

typedef struct pbf_layout {
  uint64_t frame;
  uint64_t per_frame;
  uint32_t sample_bytes;
} pbf_layout;

static int pbf_layout_of(const ivs_machine *m, pbf_layout *l)
{
  ivs_info info;
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info(m, &info) != IVS_OK || info.audio_format != IVS_OUTPUT_FORMAT_PSG_LEVELS_V1) {
    return 0;
  }
  l->frame = info.nominal_frame_ticks;
  l->per_frame = info.audio_bytes_per_frame;
  l->sample_bytes = info.audio_sample_bytes;
  return 1;
}

/* The audio bytes a run from now to target needs. */
static uint64_t pbf_need(const pbf_layout *l, uint64_t now, uint64_t target)
{
  return l->per_frame * ((target > now ? target - now : 0u) / l->frame + 1u);
}

static uint64_t pbf_now(const ivs_machine *m)
{
  ivs_info info;
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  return ivs_machine_info(m, &info) == IVS_OK ? info.time_ticks : UINT64_MAX;
}

static void pbf_req(ivs_run_request *q, uint64_t target, uint8_t *audio, uint64_t cap)
{
  memset(q, 0, sizeof *q);
  q->struct_size = (uint32_t)sizeof *q;
  q->target_ticks = target;
  q->audio = audio;
  q->audio_capacity = cap;
}

static void pbf_rep(ivs_run_report *r)
{
  memset(r, PBF_SENTINEL, sizeof *r);
  r->struct_size = (uint32_t)sizeof *r;
}

/* Digest of domain d of m into out; 1 on success. */
static int pbf_digest(const ivs_machine *m, uint32_t d, uint8_t out[32])
{
  return ivs_machine_state_digest(m, d, out) == IVS_OK;
}

/* 1 when the whole storage and the composite and AUDIO digests of two instances are equal. */
static int pbf_same(const ivs_ms_inst *a, const ivs_ms_inst *b)
{
  uint8_t da[32];
  uint8_t db[32];
  uint32_t d;
  if (memcmp(a->storage, b->storage, IVS_MACHINE_BYTES) != 0) {
    return 0;
  }
  for (d = 0u; d <= IVS_DIGEST_PSGLOG; d++) {
    if (d > IVS_DIGEST_INSTANCE && d < IVS_DIGEST_VIDEO) {
      continue;
    }
    if (!pbf_digest(a->m, d, da) || !pbf_digest(b->m, d, db) || memcmp(da, db, 32u) != 0) {
      return 0;
    }
  }
  return 1;
}

static int pbf_too_small_no_write(void)
{
  ivs_ms_inst x;
  ivs_ms_inst ref;
  pbf_layout l;
  ivs_run_request q;
  ivs_run_report r;
  uint8_t *buf = NULL;
  uint64_t need;
  uint64_t i;
  int rc = IVS_TEST_FAIL;
  ref.storage = NULL;
  ref.m = NULL;
  if (!ivs_ms_powered_prog(&x, IVS_MS_PROG_TONE, PBF_SENTINEL)) {
    return IVS_TEST_FAIL;
  }
  PBF_CHECK(ivs_ms_powered_prog(&ref, IVS_MS_PROG_TONE, PBF_SENTINEL));
  PBF_CHECK(pbf_layout_of(x.m, &l));
  need = pbf_need(&l, 0u, PBF_TARGET);
  buf = malloc((size_t)need + 64u);
  PBF_CHECK(buf != NULL);
  memset(buf, PBF_SENTINEL, (size_t)need + 64u);
  pbf_req(&q, PBF_TARGET, buf, need - 1u);
  pbf_rep(&r);
  PBF_CHECK(ivs_machine_run(x.m, &q, &r) == IVS_E_BUFFER_TOO_SMALL);
  PBF_CHECK(r.refusal_reason == IVS_REASON_RUN_BUFFER_SMALL);
  for (i = 0u; i < need + 64u; i++) {
    if (buf[i] != PBF_SENTINEL) {
      ivs_test_diag("buffer byte %llu written by a refused run", (unsigned long long)i);
      goto done;
    }
  }
  PBF_CHECK(pbf_same(&x, &ref));
  PBF_CHECK(pbf_now(x.m) == 0u);
  rc = IVS_TEST_PASS;
done:
  free(buf);
  ivs_ms_free(&ref);
  ivs_ms_free(&x);
  return rc;
}

static int pbf_exact_fit(void)
{
  ivs_ms_inst x;
  pbf_layout l;
  ivs_run_request q;
  ivs_run_report r;
  uint8_t *buf = NULL;
  uint64_t need;
  uint64_t n;
  uint64_t k;
  int rc = IVS_TEST_FAIL;
  if (!ivs_ms_powered_prog(&x, IVS_MS_PROG_TONE, PBF_SENTINEL)) {
    return IVS_TEST_FAIL;
  }
  PBF_CHECK(pbf_layout_of(x.m, &l));
  need = pbf_need(&l, 0u, PBF_TARGET);
  buf = malloc((size_t)need);
  PBF_CHECK(buf != NULL);
  memset(buf, PBF_SENTINEL, (size_t)need);
  pbf_req(&q, PBF_TARGET, buf, need);
  pbf_rep(&r);
  PBF_CHECK(ivs_machine_run(x.m, &q, &r) == IVS_OK && r.stop_reason == IVS_STOP_TARGET);
  PBF_CHECK(r.audio_first_index == 0u);
  n = r.now_ticks / 16u; /* one sample per 16 ticks: the samples due up to the boundary the run ended at */
  PBF_CHECK(n > 4098u);
  PBF_CHECK(r.audio_bytes_written == 4u * n && r.audio_bytes_written <= need);
  /* The guest's tone A: 0 until step 4095, 15 at step 4096, then every step toggles (the literals of the
   * tracer psg.tone.guest-late-period); channels B, C and the reserved byte stay 0. */
  for (k = 1u; k <= n; k++) {
    uint64_t at = 4u * (k - 1u);
    uint8_t want = k < 4096u ? 0u : (uint8_t)(((k - 4096u) % 2u == 0u) ? 15u : 0u);
    PBF_CHECK(buf[at] == want && buf[at + 1u] == 0u && buf[at + 2u] == 0u && buf[at + 3u] == 0u);
  }
  rc = IVS_TEST_PASS;
done:
  free(buf);
  ivs_ms_free(&x);
  return rc;
}

static int pbf_not_requested(void)
{
  ivs_ms_inst x;
  ivs_run_request q;
  ivs_run_report r;
  uint64_t first;
  int rc = IVS_TEST_FAIL;
  if (!ivs_ms_powered_prog(&x, IVS_MS_PROG_TONE, PBF_SENTINEL)) {
    return IVS_TEST_FAIL;
  }
  pbf_req(&q, PBF_TARGET, NULL, 0u);
  pbf_rep(&r);
  PBF_CHECK(ivs_machine_run(x.m, &q, &r) == IVS_OK && r.stop_reason == IVS_STOP_TARGET);
  PBF_CHECK(r.audio_bytes_written == 0u && r.audio_first_index == 0u);
  first = r.now_ticks / 16u;
  pbf_req(&q, r.now_ticks + 5000u, NULL, 0u);
  pbf_rep(&r);
  PBF_CHECK(ivs_machine_run(x.m, &q, &r) == IVS_OK);
  PBF_CHECK(r.audio_bytes_written == 0u && r.audio_first_index == first);
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

static int pbf_state_independent(void)
{
  static const uint64_t slices[6] = {1u, 13u, 700u, 4096u, 30000u, 40000u};
  ivs_ms_inst a;
  ivs_ms_inst b;
  pbf_layout l;
  uint8_t *buf = NULL;
  uint32_t i;
  int rc = IVS_TEST_FAIL;
  b.storage = NULL;
  b.m = NULL;
  if (!ivs_ms_powered_prog(&a, IVS_MS_PROG_TONE, PBF_SENTINEL)) {
    return IVS_TEST_FAIL;
  }
  PBF_CHECK(ivs_ms_powered_prog(&b, IVS_MS_PROG_TONE, PBF_SENTINEL));
  PBF_CHECK(pbf_layout_of(a.m, &l));
  buf = malloc((size_t)l.per_frame * 4u);
  PBF_CHECK(buf != NULL);
  for (i = 0u; i < 6u; i++) {
    ivs_run_request q;
    ivs_run_report ra;
    ivs_run_report rb;
    uint64_t now = pbf_now(a.m);
    uint64_t target = now + slices[i];
    memset(buf, PBF_SENTINEL, (size_t)l.per_frame * 4u);
    pbf_req(&q, target, buf, pbf_need(&l, now, target));
    pbf_rep(&ra);
    PBF_CHECK(ivs_machine_run(a.m, &q, &ra) == IVS_OK);
    pbf_req(&q, target, NULL, 0u);
    pbf_rep(&rb);
    PBF_CHECK(ivs_machine_run(b.m, &q, &rb) == IVS_OK);
    PBF_CHECK(pbf_same(&a, &b));
    PBF_CHECK(ra.now_ticks == rb.now_ticks && ra.audio_first_index == rb.audio_first_index);
    PBF_CHECK(rb.audio_bytes_written == 0u);
  }
  rc = IVS_TEST_PASS;
done:
  free(buf);
  ivs_ms_free(&b);
  ivs_ms_free(&a);
  return rc;
}

static int pbf_sentinel_tail(void)
{
  ivs_ms_inst x;
  pbf_layout l;
  ivs_run_request q;
  ivs_run_report r;
  uint8_t *buf = NULL;
  uint64_t cap;
  uint64_t i;
  int rc = IVS_TEST_FAIL;
  if (!ivs_ms_powered_prog(&x, IVS_MS_PROG_TONE, PBF_SENTINEL)) {
    return IVS_TEST_FAIL;
  }
  PBF_CHECK(pbf_layout_of(x.m, &l));
  cap = pbf_need(&l, 0u, 5000u) + 777u; /* more than the run needs */
  buf = malloc((size_t)cap);
  PBF_CHECK(buf != NULL);
  memset(buf, PBF_SENTINEL, (size_t)cap);
  pbf_req(&q, 5000u, buf, cap);
  pbf_rep(&r);
  PBF_CHECK(ivs_machine_run(x.m, &q, &r) == IVS_OK);
  PBF_CHECK(r.audio_bytes_written > 0u && r.audio_bytes_written < cap);
  for (i = 0u; i < r.audio_bytes_written; i++) {
    PBF_CHECK(buf[i] == 0u); /* silent before the first edge */
  }
  for (i = r.audio_bytes_written; i < cap; i++) {
    if (buf[i] != PBF_SENTINEL) {
      ivs_test_diag("byte %llu past the delivered count written", (unsigned long long)i);
      goto done;
    }
  }
  rc = IVS_TEST_PASS;
done:
  free(buf);
  ivs_ms_free(&x);
  return rc;
}

/* One refusal: nothing written into the arena, the instance unchanged. */
static int pbf_refused(const ivs_ms_inst *x, const ivs_ms_inst *ref, const ivs_run_request *q, uint8_t *arena,
                       size_t arena_bytes, uint32_t reason)
{
  ivs_run_report r;
  uint8_t *keep = malloc(arena_bytes);
  int ok = 0;
  if (keep == NULL) {
    return 0;
  }
  memcpy(keep, arena, arena_bytes);
  pbf_rep(&r);
  if (ivs_machine_run(x->m, q, &r) != IVS_E_BAD_ARG) {
    ivs_test_diag("overlap was not refused BAD_ARG");
    goto done;
  }
  if (r.refusal_reason != reason) {
    ivs_test_diag("refusal reason %u, want %u", (unsigned)r.refusal_reason, (unsigned)reason);
    goto done;
  }
  ok = memcmp(keep, arena, arena_bytes) == 0 && pbf_same(x, ref);
done:
  free(keep);
  return ok;
}

static int pbf_overlap(void)
{
  enum { ARENA = 16384 };
  ivs_ms_inst x;
  ivs_ms_inst ref;
  ivs_run_request q;
  uint8_t *arena = NULL;
  int rc = IVS_TEST_FAIL;
  ref.storage = NULL;
  ref.m = NULL;
  if (!ivs_ms_powered_prog(&x, IVS_MS_PROG_TONE, PBF_SENTINEL)) {
    return IVS_TEST_FAIL;
  }
  PBF_CHECK(ivs_ms_powered_prog(&ref, IVS_MS_PROG_TONE, PBF_SENTINEL));
  arena = aligned_alloc(16u, ARENA);
  PBF_CHECK(arena != NULL);
  memset(arena, PBF_SENTINEL, ARENA);
  /* the audio buffer overlaps the instance storage (an address inside it) */
  pbf_req(&q, 1000u, x.storage + 64u, 4096u);
  PBF_CHECK(pbf_refused(&x, &ref, &q, arena, ARENA, IVS_REASON_RUN_BUFFER_OVERLAP));
  /* the audio buffer overlaps the video buffer */
  pbf_req(&q, 1000u, arena, 4096u);
  q.video = arena + 2048u;
  q.video_capacity = 4096u;
  PBF_CHECK(pbf_refused(&x, &ref, &q, arena, ARENA, IVS_REASON_RUN_BUFFER_OVERLAP));
  /* the audio buffer overlaps the event array (the array is read after the slot refuses: sentinel events) */
  pbf_req(&q, 1000u, arena, 4096u);
  q.events = (const ivs_input_event *)(const void *)(arena + 4096u - 32u);
  q.event_count = 2u;
  PBF_CHECK(pbf_refused(&x, &ref, &q, arena, ARENA, IVS_REASON_INPUT_EVENTS_OVERLAP));
  /* the audio buffer overlaps the write log */
  pbf_req(&q, 1000u, arena, 4096u);
  q.psg_log = arena + 4000u;
  q.psg_log_capacity = 1024u;
  PBF_CHECK(pbf_refused(&x, &ref, &q, arena, ARENA, IVS_REASON_RUN_PSGLOG_OVERLAP));
  rc = IVS_TEST_PASS;
done:
  free(arena);
  ivs_ms_free(&ref);
  ivs_ms_free(&x);
  return rc;
}

const ivs_test_case ivs_psg_buffers_cases[] = {
    {"psg.buffers.too-small-no-write", pbf_too_small_no_write, NULL},
    {"psg.buffers.exact-fit", pbf_exact_fit, NULL},
    {"psg.buffers.not-requested", pbf_not_requested, NULL},
    {"psg.buffers.state-independent-of-buffer", pbf_state_independent, NULL},
    {"psg.buffers.sentinel-tail", pbf_sentinel_tail, NULL},
    {"psg.buffers.overlap", pbf_overlap, NULL},
};
const size_t ivs_psg_buffers_count = sizeof ivs_psg_buffers_cases / sizeof ivs_psg_buffers_cases[0];
