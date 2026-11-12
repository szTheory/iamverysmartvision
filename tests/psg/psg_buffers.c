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
 *                                             with nothing written or changed
 * Plan 05-08 (D-14, T-05-20): the write-log buffer of the same call. W_log = 2,152 records = 34,432 bytes per
 * frame slot; the guests are the project programs of psg_util.h (LOGMIX: two writes; WRITER 3800: straight-line
 * MVO@ writes 36 ticks apart from tick 100, the first MVO@ unit starting at tick 84 = 21 CPU cycles).
 *   psg.buffers.worst-case-writer             the writer's records are exactly 100 + 36 i, 3,609 of them over a run
 *                                             to tick 130,000, any window of one frame slot holds at most 1,660,
 *                                             the run ends at its target and never INTERNAL
 *   psg.buffers.log-too-small                 capacity one byte below the requirement: BUFFER_TOO_SMALL
 *                                             (RUN_PSGLOG_SMALL), nothing written or changed; the exact capacity runs
 *   psg.buffers.log-not-requested             NULL with capacity 0: no log byte, the machine and every digest equal
 *                                             the same run with a buffer, and psg_log_first_index still counts
 *   psg.buffers.log-sentinel-tail             bytes past psg_log_bytes_written inside the capacity are untouched
 *   psg.buffers.log-safety-net                a synthetic layout whose W_log is halved: the writer ends INTERNAL at
 *                                             the last whole boundary, state equal to a twin run to that boundary */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs/ivs_run.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "machine_cases.h"
#include "psg_cases.h"
#include "psg_util.h"

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

/* ---- plan 05-08: the write-log buffer ---- */

#define PBF_WRITES 3800u
#define PBF_WRITER_TARGET 130000u
#define PBF_WRITER_FIRST 100u /* the first MVO@ unit starts at 21 CPU cycles = 84 ticks, its write is at cycle 4 */
#define PBF_WRITER_START 84u

/* An instance running program words w (n words), poisoned. 1 on success. */
static int pbf_writer(ivs_ms_inst *x)
{
  uint16_t *w = malloc((PBF_WRITES + 8u) * sizeof w[0]);
  size_t n;
  int ok;
  if (w == NULL) {
    return 0;
  }
  n = pu_prog_writer(w, PBF_WRITES);
  ok = ivs_ms_powered_words(x, w, n, PBF_SENTINEL);
  free(w);
  return ok;
}

static int pbf_worst_case_writer(void)
{
  ivs_ms_inst x;
  ivs_run_report rep;
  uint8_t *log = NULL;
  uint64_t cap;
  uint64_t records;
  uint64_t i;
  uint64_t j;
  uint64_t densest = 0u;
  uint64_t expect;
  int rc = IVS_TEST_FAIL;
  x.storage = NULL;
  x.m = NULL;
  PBF_CHECK(pbf_writer(&x));
  cap = pu_log_need(x.m, 0u, PBF_WRITER_TARGET);
  log = malloc((size_t)cap);
  PBF_CHECK(log != NULL);
  memset(log, PBF_SENTINEL, (size_t)cap);
  PBF_CHECK(pu_run_to_log(x.m, PBF_WRITER_TARGET, NULL, log, &rep));
  PBF_CHECK(rep.stop_reason == IVS_STOP_TARGET); /* never INTERNAL: the safety net did not fire */
  PBF_CHECK(rep.psg_log_first_index == 0u && rep.psg_log_bytes_written % 16u == 0u);
  records = rep.psg_log_bytes_written / 16u;
  /* Hand-derived: unit j of the writer starts at 84 + 36 j and is a started unit while that is below the target,
   * so j runs 0 .. 3608: 3,609 records (the plan's floor(T / 36) - 1 bound reads 3,610 because it ignores the
   * 84-tick prefix: floor((130,000 - 84 - 1) / 36) + 1). */
  expect = (PBF_WRITER_TARGET - PBF_WRITER_START - 1u) / 36u + 1u;
  PBF_CHECK(expect == 3609u && records == expect);
  for (i = 0u; i < records; i++) {
    const uint8_t *r = log + 16u * i;
    uint8_t want[16];
    pu_be64(want, PBF_WRITER_FIRST + 36u * i);
    want[8] = 0x00u;
    want[9] = 0x0Fu;
    want[10] = 0x0Bu;
    want[11] = 0x01u;
    memset(want + 12, 0, 4u);
    if (memcmp(r, want, 16u) != 0) {
      ivs_test_diag("record %llu differs from the hand-derived one", (unsigned long long)i);
      goto done;
    }
  }
  /* the densest window of one frame slot (F = 59,736 ticks) holds ceil(F / 36) = 1,660 writes, below W_log */
  for (i = 0u, j = 0u; i < records; i++) {
    uint64_t ti = pu_get_be64(log + 16u * i);
    while (j < records && pu_get_be64(log + 16u * j) < ti + 59736u) {
      j++;
    }
    if (j - i > densest) {
      densest = j - i;
    }
  }
  PBF_CHECK(densest == 1660u && densest <= 2152u);
  rc = IVS_TEST_PASS;
done:
  free(log);
  ivs_ms_free(&x);
  return rc;
}

/* LOGMIX at tick 1000 from a fresh instance: the log buffer capacity needed is exactly 34,432 (one frame slot). */
static int pbf_log_run(ivs_machine *m, uint64_t target, uint8_t *log, uint64_t cap, ivs_run_report *rep,
                       ivs_status *st)
{
  ivs_run_request q;
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.target_ticks = target;
  q.psg_log = log;
  q.psg_log_capacity = cap;
  memset(rep, PBF_SENTINEL, sizeof *rep);
  rep->struct_size = (uint32_t)sizeof *rep;
  *st = ivs_machine_run(m, &q, rep);
  return 1;
}

static int pbf_logmix(ivs_ms_inst *x)
{
  uint16_t w[16];
  size_t n = pu_prog_logmix(w);
  return ivs_ms_powered_words(x, w, n, PBF_SENTINEL);
}

static int pbf_log_too_small(void)
{
  ivs_ms_inst x;
  ivs_ms_inst ref;
  ivs_run_report rep;
  ivs_status st = IVS_OK;
  uint8_t *log = NULL;
  uint64_t need;
  uint64_t i;
  int rc = IVS_TEST_FAIL;
  x.storage = ref.storage = NULL;
  x.m = ref.m = NULL;
  PBF_CHECK(pbf_logmix(&x) && pbf_logmix(&ref));
  need = pu_log_need(x.m, 0u, 1000u);
  PBF_CHECK(need == 34432u); /* one frame slot of 2,152 records x 16 bytes */
  log = malloc((size_t)need);
  PBF_CHECK(log != NULL);
  memset(log, PBF_SENTINEL, (size_t)need);
  (void)pbf_log_run(x.m, 1000u, log, need - 1u, &rep, &st);
  PBF_CHECK(st == IVS_E_BUFFER_TOO_SMALL && rep.refusal_reason == IVS_REASON_RUN_PSGLOG_SMALL);
  for (i = 0u; i < need; i++) {
    PBF_CHECK(log[i] == PBF_SENTINEL);
  }
  PBF_CHECK(memcmp(x.storage, ref.storage, IVS_MACHINE_BYTES) == 0);
  (void)pbf_log_run(x.m, 1000u, log, need, &rep, &st);
  PBF_CHECK(st == IVS_OK && rep.stop_reason == IVS_STOP_TARGET);
  PBF_CHECK(rep.psg_log_bytes_written == 32u && rep.psg_log_first_index == 0u); /* the two writes of LOGMIX */
  rc = IVS_TEST_PASS;
done:
  free(log);
  if (ref.storage != NULL) {
    ivs_ms_free(&ref);
  }
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  return rc;
}

static int pbf_log_not_requested(void)
{
  ivs_ms_inst a;
  ivs_ms_inst b;
  ivs_run_report ra;
  ivs_run_report rb;
  ivs_status st = IVS_OK;
  uint8_t *log = NULL;
  uint64_t need;
  int rc = IVS_TEST_FAIL;
  a.storage = b.storage = NULL;
  a.m = b.m = NULL;
  PBF_CHECK(pbf_logmix(&a) && pbf_logmix(&b));
  need = pu_log_need(a.m, 0u, 1000u);
  log = malloc((size_t)need);
  PBF_CHECK(log != NULL);
  memset(log, PBF_SENTINEL, (size_t)need);
  /* a requests the log, b does not: NULL with capacity 0 */
  (void)pbf_log_run(a.m, 1000u, log, need, &ra, &st);
  PBF_CHECK(st == IVS_OK && ra.psg_log_bytes_written == 32u);
  (void)pbf_log_run(b.m, 1000u, NULL, 0u, &rb, &st);
  PBF_CHECK(st == IVS_OK && rb.psg_log_bytes_written == 0u && rb.psg_log_first_index == 0u);
  PBF_CHECK(pbf_same(&a, &b)); /* whole storage and every digest, domain 19 included */
  PBF_CHECK(((struct ivs_machine *)b.m)->obs.psglog.count == 2u); /* the records were counted and chained */
  /* the next call of the unrequested instance: records before the call are 2 */
  (void)pbf_log_run(b.m, 2000u, NULL, 0u, &rb, &st);
  PBF_CHECK(st == IVS_OK && rb.psg_log_bytes_written == 0u && rb.psg_log_first_index == 2u);
  rc = IVS_TEST_PASS;
done:
  free(log);
  if (a.storage != NULL) {
    ivs_ms_free(&a);
  }
  if (b.storage != NULL) {
    ivs_ms_free(&b);
  }
  return rc;
}

static int pbf_log_sentinel_tail(void)
{
  ivs_ms_inst x;
  ivs_run_report rep;
  ivs_status st = IVS_OK;
  uint8_t *log = NULL;
  uint64_t need;
  uint64_t i;
  int rc = IVS_TEST_FAIL;
  x.storage = NULL;
  x.m = NULL;
  PBF_CHECK(pbf_logmix(&x));
  need = pu_log_need(x.m, 0u, 1000u);
  log = malloc((size_t)need + 64u);
  PBF_CHECK(log != NULL);
  memset(log, PBF_SENTINEL, (size_t)need + 64u);
  (void)pbf_log_run(x.m, 1000u, log, need, &rep, &st);
  PBF_CHECK(st == IVS_OK && rep.psg_log_bytes_written == 32u);
  for (i = 32u; i < need + 64u; i++) {
    PBF_CHECK(log[i] == PBF_SENTINEL); /* the 32 delivered bytes are the only ones written, even past the capacity */
  }
  PBF_CHECK(log[0] != PBF_SENTINEL || log[1] != PBF_SENTINEL || log[7] != PBF_SENTINEL);
  rc = IVS_TEST_PASS;
done:
  free(log);
  ivs_ms_free(&x);
  return rc;
}

static int pbf_log_safety_net(void)
{
  ivs_ms_inst x;
  ivs_ms_inst twin;
  ivs_run_env env;
  ivs_run_request q;
  ivs_run_report rep;
  const ivs_profile_desc *prof = NULL;
  uint8_t *log = NULL;
  uint64_t cap;
  uint64_t records;
  uint32_t p;
  int rc = IVS_TEST_FAIL;
  x.storage = twin.storage = NULL;
  x.m = twin.m = NULL;
  for (p = 0u; p < ivs_profile_count(); p++) {
    if (strcmp(ivs_profile_get(p)->id, IVS_MS_PROFILE) == 0) {
      prof = ivs_profile_get(p);
    }
  }
  PBF_CHECK(prof != NULL && pbf_writer(&x) && pbf_writer(&twin));
  ivs_machine_profile_env(prof, &env);
  /* the synthetic layout: W_log halved, 1,076 records per frame slot, below the instruction-only density of 1,660 */
  PBF_CHECK(env.psg_log_bytes_per_frame == 34432u || env.psg_log_bytes_per_frame == 17216u);
  env.psg_log_bytes_per_frame = 17216u;
  cap = (uint64_t)env.psg_log_bytes_per_frame * (PBF_WRITER_TARGET / env.nominal_frame_ticks + 1u);
  log = malloc((size_t)cap);
  PBF_CHECK(log != NULL);
  memset(log, PBF_SENTINEL, (size_t)cap);
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.target_ticks = PBF_WRITER_TARGET;
  q.psg_log = log;
  q.psg_log_capacity = cap;
  memset(&rep, 0, sizeof rep);
  rep.struct_size = (uint32_t)sizeof rep;
  PBF_CHECK(ivs_machine_run_env_internal(x.m, &q, &rep, &env) == IVS_OK);
  PBF_CHECK(rep.stop_reason == IVS_STOP_INTERNAL);
  records = rep.psg_log_bytes_written / 16u;
  PBF_CHECK(records == cap / 16u && records < 3609u);
  PBF_CHECK(rep.now_ticks == PBF_WRITER_START + 36u * records); /* the last whole boundary: records units have run */
  /* the state equals a twin that simply ran to that boundary with enough room */
  {
    ivs_run_report r2;
    uint8_t *big = malloc((size_t)pu_log_need(twin.m, 0u, rep.now_ticks));
    PBF_CHECK(big != NULL);
    PBF_CHECK(pu_now(twin.m) <= rep.now_ticks);
    PBF_CHECK(pu_run_to_log(twin.m, rep.now_ticks, NULL, big, &r2));
    free(big);
    PBF_CHECK(r2.stop_reason == IVS_STOP_TARGET && r2.now_ticks == rep.now_ticks);
  }
  PBF_CHECK(pbf_same(&x, &twin));
  rc = IVS_TEST_PASS;
done:
  free(log);
  if (twin.storage != NULL) {
    ivs_ms_free(&twin);
  }
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  return rc;
}

const ivs_test_case ivs_psg_buffers_cases[] = {
    {"psg.buffers.too-small-no-write", pbf_too_small_no_write, NULL},
    {"psg.buffers.exact-fit", pbf_exact_fit, NULL},
    {"psg.buffers.not-requested", pbf_not_requested, NULL},
    {"psg.buffers.state-independent-of-buffer", pbf_state_independent, NULL},
    {"psg.buffers.sentinel-tail", pbf_sentinel_tail, NULL},
    {"psg.buffers.overlap", pbf_overlap, NULL},
    {"psg.buffers.worst-case-writer", pbf_worst_case_writer, NULL},
    {"psg.buffers.log-too-small", pbf_log_too_small, NULL},
    {"psg.buffers.log-not-requested", pbf_log_not_requested, NULL},
    {"psg.buffers.log-sentinel-tail", pbf_log_sentinel_tail, NULL},
    {"psg.buffers.log-safety-net", pbf_log_safety_net, NULL},
};
const size_t ivs_psg_buffers_count = sizeof ivs_psg_buffers_cases / sizeof ivs_psg_buffers_cases[0];
