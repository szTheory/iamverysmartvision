/* SPDX-License-Identifier: MIT */
/* Suite run, family buffers (D-02, D-03, D-05, D-06, plan 04-10): the caller-owned video and audio
 * buffers of ivs_machine_run and their capacity contract (API-04).
 * The real Phase 4 layout is zero-length (formats NONE, 0 bytes per frame), so the capacity rule
 * is proven through the internal env seam ivs_machine_run_env_internal (machine/machine.h) with a
 * synthetic layout: BF_BYTES bytes per frame and a BF_FRAME-tick nominal frame. The rule under
 * test (ivs/ivs_run.h): a run from now to target needs, per channel,
 *   bytes_per_frame * (max(target - now, 0) / nominal_frame_ticks + 1)
 * bytes; one byte less is BUFFER_TOO_SMALL before any unit runs.
 * Every refusal is judged with the SP4 oracle: status, refusal_reason, every other report byte
 * kept, whole instance storage memcmp-equal, the composite and seven domain digests and machine
 * time equal, and the poisoned buffer arena (both buffers and the bytes around them) unchanged.
 * Stand-in content A of tests/machine/machine_standin.c (no rights-bearing byte). */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "run_cases.h"

#define IVS_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

#define BF_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      goto done;                                                                                       \
    }                                                                                                  \
  } while (0)

#define BF_POISON 0xA5u
/* The synthetic layout: 16 bytes per frame, a 1000-tick nominal frame. */
#define BF_BYTES 16u
#define BF_FRAME 1000u
/* A run from now to now + BF_SPAN needs 2500 / 1000 + 1 = 3 frames: 48 bytes per channel. */
#define BF_SPAN 2500u
#define BF_NEED 48u
/* The poisoned caller memory the buffers are cut from (offsets below), with guard bytes. */
#define BF_ARENA 512u
#define BF_VIDEO_OFF 16u
#define BF_AUDIO_OFF 128u
#define BF_REP_BYTES 192u

typedef union bf_rep {
  ivs_run_report r;
  uint8_t b[BF_REP_BYTES];
} bf_rep;

/* Plan 05-02 (D-14, D-17, A-11): the third channel, the write log (psg_log, psg_log_capacity of
 * request v2; the env's psg_log_bytes_per_frame), its reasons RUN_PSGLOG_NULL, RUN_PSGLOG_OVERLAP and
 * RUN_PSGLOG_SMALL, and the rule that a NULL buffer with capacity 0 means "channel not requested".
 * The Phase 4 case run.prec.capacity-over-event-order (run_input.c) relied on NULL with capacity 0
 * against a nonzero layout being BUFFER_TOO_SMALL; its old expectation is named there. */

/* ---- helpers ---- */

/* Machine time of m (info), or UINT64_MAX when info refuses. */
static uint64_t bf_now(const ivs_machine *m)
{
  ivs_info info;
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info(m, &info) != IVS_OK) {
    ivs_test_diag("info refused");
    return UINT64_MAX;
  }
  return info.time_ticks;
}

static void bf_env(ivs_run_env *e, uint32_t video_bytes, uint32_t audio_bytes, uint64_t frame)
{
  memset(e, 0, sizeof *e);
  e->video_bytes_per_frame = video_bytes;
  e->audio_bytes_per_frame = audio_bytes;
  e->nominal_frame_ticks = frame;
}

/* The write-log channel of a synthetic layout. */
static void bf_log_env(ivs_run_env *e, uint32_t log_bytes)
{
  e->psg_log_bytes_per_frame = log_bytes;
}

/* The write-log buffer of a request built by bf_req. */
static void bf_log(ivs_run_request *q, uint8_t *log, uint64_t cap)
{
  q->psg_log = log;
  q->psg_log_capacity = cap;
}

static void bf_req(ivs_run_request *q, uint32_t flags, uint64_t target, uint8_t *video, uint64_t video_cap,
                   uint8_t *audio, uint64_t audio_cap)
{
  memset(q, 0, sizeof *q);
  q->struct_size = (uint32_t)sizeof *q;
  q->flags = flags;
  q->target_ticks = target;
  q->video = video;
  q->video_capacity = video_cap;
  q->audio = audio;
  q->audio_capacity = audio_cap;
}

/* One run: through the env seam when env is not NULL, else through the public call. */
static ivs_status bf_call(ivs_machine *m, const ivs_run_request *q, ivs_run_report *rep, const ivs_run_env *env)
{
  if (env != NULL) {
    return ivs_machine_run_env_internal(m, q, rep, env);
  }
  return ivs_machine_run(m, q, rep);
}

/* A poisoned arena (the caller frees). */
static uint8_t *bf_arena(void)
{
  uint8_t *a = malloc(BF_ARENA);
  if (a != NULL) {
    memset(a, BF_POISON, BF_ARENA);
  }
  return a;
}

/* 1 when every arena byte is still poison. */
static int bf_arena_kept(const uint8_t *a)
{
  size_t i;
  for (i = 0u; i < BF_ARENA; i++) {
    if (a[i] != BF_POISON) {
      ivs_test_diag("arena byte %u written (0x%02x)", (unsigned)i, (unsigned)a[i]);
      return 0;
    }
  }
  return 1;
}

/* One refused call with the SP4 oracle (header comment). The report is poisoned with struct_size
 * the full size; on return it must hold struct_size, refusal_reason reason and poison elsewhere. */
static int bf_refused(const ivs_ms_inst *x, ivs_machine *m, const ivs_run_request *q, const ivs_run_env *env,
                      const uint8_t *arena, ivs_status want, uint32_t reason)
{
  ivs_run_snap s;
  bf_rep u;
  ivs_status st;
  size_t i;
  int ok = 0;
  memset(&u, BF_POISON, sizeof u);
  u.r.struct_size = (uint32_t)sizeof(ivs_run_report);
  if (!ivs_run_snap_take(&s, x, 1)) {
    ivs_run_snap_free(&s);
    return 0;
  }
  st = bf_call(m, q, &u.r, env);
  if (st != want) {
    ivs_test_diag("status %s, want %s", ivs_status_name(st), ivs_status_name(want));
    goto done;
  }
  if (u.r.struct_size != (uint32_t)sizeof(ivs_run_report) || u.r.refusal_reason != reason) {
    ivs_test_diag("report struct_size %u refusal_reason %u, want %u %u", (unsigned)u.r.struct_size,
                  (unsigned)u.r.refusal_reason, (unsigned)sizeof(ivs_run_report), (unsigned)reason);
    goto done;
  }
  for (i = 4u; i < BF_REP_BYTES; i++) {
    if (i >= offsetof(ivs_run_report, refusal_reason) && i < offsetof(ivs_run_report, refusal_reason) + 4u) {
      continue;
    }
    if (u.b[i] != BF_POISON) {
      ivs_test_diag("report byte %u written by a refused call", (unsigned)i);
      goto done;
    }
  }
  ok = ivs_run_snap_same(&s, x) && bf_arena_kept(arena);
done:
  ivs_run_snap_free(&s);
  return ok;
}

/* ---- BUFFER_TOO_SMALL (D-05, D-06) ---- */

/* A video capacity one byte below the requirement: BUFFER_TOO_SMALL (RUN_BUFFER_SMALL) before any
 * unit, at machine time 0 and again after three seam-stepped units (the requirement is measured
 * from now). The audio channel needs nothing (0 bytes per frame). */
static int bf_too_small_video(void)
{
  ivs_ms_inst x;
  ivs_run_env e;
  ivs_run_request q;
  uint8_t *a = NULL;
  uint64_t now;
  uint32_t pass;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, BF_POISON));
  a = bf_arena();
  BF_CHECK(a != NULL);
  bf_env(&e, BF_BYTES, 0u, BF_FRAME);
  for (pass = 0u; pass < 2u; pass++) {
    if (pass == 1u) {
      BF_CHECK(ivs_ms_step(x.m, 3u, NULL));
    }
    now = bf_now(x.m);
    BF_CHECK(now != UINT64_MAX && (pass == 0u) == (now == 0u));
    bf_req(&q, 0u, now + BF_SPAN, a + BF_VIDEO_OFF, BF_NEED - 1u, NULL, 0u);
    BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BUFFER_TOO_SMALL, IVS_REASON_RUN_BUFFER_SMALL));
  }
  rc = IVS_TEST_PASS;
done:
  free(a);
  ivs_ms_free(&x);
  return rc;
}

/* The audio channel likewise (video needs nothing). */
static int bf_too_small_audio(void)
{
  ivs_ms_inst x;
  ivs_run_env e;
  ivs_run_request q;
  uint8_t *a = NULL;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, BF_POISON));
  a = bf_arena();
  BF_CHECK(a != NULL);
  bf_env(&e, 0u, BF_BYTES, BF_FRAME);
  bf_req(&q, 0u, BF_SPAN, NULL, 0u, a + BF_AUDIO_OFF, BF_NEED - 1u);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BUFFER_TOO_SMALL, IVS_REASON_RUN_BUFFER_SMALL));
  /* Both channels with a layout; video fits, audio is one byte short. */
  bf_env(&e, BF_BYTES, BF_BYTES, BF_FRAME);
  bf_req(&q, 0u, BF_SPAN, a + BF_VIDEO_OFF, BF_NEED, a + BF_AUDIO_OFF, BF_NEED - 1u);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BUFFER_TOO_SMALL, IVS_REASON_RUN_BUFFER_SMALL));
  rc = IVS_TEST_PASS;
done:
  free(a);
  ivs_ms_free(&x);
  return rc;
}

/* An accepted run through the seam with buffers of exactly the requirement: IVS_OK, stop TARGET
 * at the first boundary at or after the target, zero output counts (no frame is fabricated), the
 * arena untouched, and the storage equal to a twin run through the public call. which: 0 video,
 * 1 audio, 2 both. A target crossing a frame edge (now + 3000: 4 frames, 64 bytes) as well. */
static int bf_exact_fit(uint32_t which)
{
  static const uint64_t spans[2] = {BF_SPAN, 3u * BF_FRAME};
  ivs_ms_inst x;
  ivs_ms_inst y;
  ivs_run_env e;
  ivs_run_request q;
  ivs_run_request qy;
  bf_rep u;
  ivs_run_report ry;
  uint8_t *a = NULL;
  uint32_t i;
  int rc = IVS_TEST_FAIL;
  y.storage = NULL;
  y.m = NULL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, BF_POISON));
  BF_CHECK(ivs_ms_powered(&y, IVS_MS_CONTENT_A, BF_POISON));
  a = bf_arena();
  BF_CHECK(a != NULL);
  bf_env(&e, which != 1u ? BF_BYTES : 0u, which != 0u ? BF_BYTES : 0u, BF_FRAME);
  for (i = 0u; i < 2u; i++) {
    uint64_t now = bf_now(x.m);
    uint64_t need = BF_BYTES * (spans[i] / BF_FRAME + 1u);
    ivs_status st;
    BF_CHECK(now != UINT64_MAX && now == bf_now(y.m));
    bf_req(&q, 0u, now + spans[i], which != 1u ? a + BF_VIDEO_OFF : NULL, which != 1u ? need : 0u,
           which != 0u ? a + BF_AUDIO_OFF : NULL, which != 0u ? need : 0u);
    memset(&u, BF_POISON, sizeof u);
    u.r.struct_size = (uint32_t)sizeof(ivs_run_report);
    st = bf_call(x.m, &q, &u.r, &e);
    BF_CHECK(st == IVS_OK);
    BF_CHECK(u.r.struct_size == (uint32_t)sizeof(ivs_run_report) && u.r.stop_reason == IVS_STOP_TARGET);
    BF_CHECK(u.r.refusal_reason == IVS_REASON_NONE && u.r.now_ticks >= now + spans[i] && u.r.units_executed > 0u);
    BF_CHECK(u.r.video_frames_completed == 0u && u.r.video_bytes_written == 0u && u.r.audio_bytes_written == 0u);
    BF_CHECK(bf_arena_kept(a));
    bf_req(&qy, 0u, now + spans[i], NULL, 0u, NULL, 0u);
    memset(&ry, 0, sizeof ry);
    ry.struct_size = (uint32_t)sizeof ry;
    st = ivs_machine_run(y.m, &qy, &ry);
    BF_CHECK(st == IVS_OK && memcmp(&ry, &u.r, sizeof ry) == 0);
    BF_CHECK(memcmp(x.storage, y.storage, IVS_MACHINE_BYTES) == 0);
  }
  rc = IVS_TEST_PASS;
done:
  free(a);
  ivs_ms_free(&y);
  ivs_ms_free(&x);
  return rc;
}

static int bf_exact_fit_video(void)
{
  return bf_exact_fit(0u);
}

static int bf_exact_fit_audio(void)
{
  return bf_exact_fit(1u);
}

static int bf_exact_fit_both(void)
{
  return bf_exact_fit(2u);
}

/* A zero-size layout (0 bytes per frame on both channels, synthetic 1000-tick frame) needs
 * nothing: NULL buffers with capacity 0 run, and so does a 0 frame length (never divided by). */
static int bf_zero_layout(void)
{
  ivs_ms_inst x;
  ivs_run_env e;
  ivs_run_request q;
  ivs_run_report r;
  ivs_status st;
  uint32_t pass;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, BF_POISON));
  for (pass = 0u; pass < 2u; pass++) {
    uint64_t now = bf_now(x.m);
    bf_env(&e, 0u, 0u, pass == 0u ? BF_FRAME : 0u);
    bf_req(&q, 0u, now + BF_SPAN, NULL, 0u, NULL, 0u);
    memset(&r, BF_POISON, sizeof r);
    r.struct_size = (uint32_t)sizeof r;
    st = bf_call(x.m, &q, &r, &e);
    BF_CHECK(st == IVS_OK && r.stop_reason == IVS_STOP_TARGET && r.now_ticks >= now + BF_SPAN);
    BF_CHECK(r.video_frames_completed == 0u && r.video_bytes_written == 0u && r.audio_bytes_written == 0u);
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* ---- buffer arguments (D-05 slot after UNSUPPORTED, before BAD_STATE) ---- */

/* BAD_ARG (RUN_BUFFER_NULL): a NULL video or audio buffer with a nonzero capacity, through the
 * public call (real layout: the capacity would suffice) and through the seam. */
static int bf_null_with_capacity(void)
{
  ivs_ms_inst x;
  ivs_run_env e;
  ivs_run_request q;
  uint8_t *a = NULL;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, BF_POISON));
  a = bf_arena();
  BF_CHECK(a != NULL);
  bf_env(&e, BF_BYTES, BF_BYTES, BF_FRAME);
  bf_req(&q, 0u, BF_SPAN, NULL, 1u, NULL, 0u);
  BF_CHECK(bf_refused(&x, x.m, &q, NULL, a, IVS_E_BAD_ARG, IVS_REASON_RUN_BUFFER_NULL));
  bf_req(&q, 0u, BF_SPAN, a + BF_VIDEO_OFF, BF_NEED, NULL, BF_NEED);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BAD_ARG, IVS_REASON_RUN_BUFFER_NULL));
  bf_req(&q, 0u, BF_SPAN, NULL, UINT64_MAX, a + BF_AUDIO_OFF, BF_NEED);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BAD_ARG, IVS_REASON_RUN_BUFFER_NULL));
  rc = IVS_TEST_PASS;
done:
  free(a);
  ivs_ms_free(&x);
  return rc;
}

/* An offset inside the instance, 16-aligned, in cartridge backing words that no stand-in segment
 * uses (content A writes $5000..$500F), as in run_bound.c. */
static size_t bf_inside_off(void)
{
  size_t off = offsetof(struct ivs_machine, mem) + offsetof(ivs_memsys, cart) + 2u * 0x6000u;
  return (off + 15u) & ~(size_t)15u;
}

/* BAD_ARG (RUN_BUFFER_OVERLAP): a video buffer inside the instance storage, an audio buffer
 * straddling its last byte, and a buffer before the instance whose capacity reaches into it.
 * Positive control: a buffer that ends exactly where the instance begins, and one that starts
 * just after its last byte, both inside a longer host allocation, are accepted. */
static int bf_inside_instance(void)
{
  ivs_ms_inst x;
  ivs_run_env e;
  ivs_run_request q;
  ivs_run_report r;
  ivs_create_options o;
  uint8_t *a = NULL;
  uint8_t *big = NULL;
  ivs_machine *bm = NULL;
  ivs_status st;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, BF_POISON));
  a = bf_arena();
  BF_CHECK(a != NULL);
  bf_env(&e, BF_BYTES, BF_BYTES, BF_FRAME);
  bf_req(&q, 0u, BF_SPAN, x.storage + bf_inside_off(), BF_NEED, NULL, 0u);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BAD_ARG, IVS_REASON_RUN_BUFFER_OVERLAP));
  BF_CHECK(bf_refused(&x, x.m, &q, NULL, a, IVS_E_BAD_ARG, IVS_REASON_RUN_BUFFER_OVERLAP));
  bf_req(&q, 0u, BF_SPAN, NULL, 0u, x.storage + IVS_MACHINE_BYTES - 1u, BF_NEED);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BAD_ARG, IVS_REASON_RUN_BUFFER_OVERLAP));
  /* Host allocation: 64 bytes, the instance, 128 bytes. */
  memset(&o, 0, sizeof o);
  o.struct_size = (uint32_t)sizeof o;
  o.profile_id = IVS_MS_PROFILE;
  big = aligned_alloc(IVS_MACHINE_ALIGN, 64u + IVS_MACHINE_BYTES + 128u);
  BF_CHECK(big != NULL);
  memset(big, BF_POISON, 64u + IVS_MACHINE_BYTES + 128u);
  BF_CHECK(ivs_machine_create(&o, big + 64u, IVS_MACHINE_BYTES, &bm) == IVS_OK);
  BF_CHECK(ivs_ms_content(bm, IVS_MS_CONTENT_A) && ivs_machine_reset(bm, IVS_RESET_POWER_ON) == IVS_OK);
  bf_req(&q, 0u, BF_SPAN, big + 64u - BF_NEED, BF_NEED, big + 64u + IVS_MACHINE_BYTES, BF_NEED);
  memset(&r, BF_POISON, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  st = bf_call(bm, &q, &r, &e);
  BF_CHECK(st == IVS_OK && r.stop_reason == IVS_STOP_TARGET);
  /* One byte more on the video buffer reaches the instance's first byte. */
  q.video_capacity = BF_NEED + 1u;
  memset(&r, BF_POISON, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  st = bf_call(bm, &q, &r, &e);
  BF_CHECK(st == IVS_E_BAD_ARG && r.refusal_reason == IVS_REASON_RUN_BUFFER_OVERLAP);
  rc = IVS_TEST_PASS;
done:
  free(big);
  free(a);
  ivs_ms_free(&x);
  return rc;
}

/* BAD_ARG (RUN_BUFFER_OVERLAP): video and audio overlapping (the same buffer, one byte shared,
 * audio inside video). Positive control: adjacent buffers (video ends where audio begins) run. */
static int bf_video_audio_overlap(void)
{
  ivs_ms_inst x;
  ivs_run_env e;
  ivs_run_request q;
  ivs_run_report r;
  uint8_t *a = NULL;
  ivs_status st;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, BF_POISON));
  a = bf_arena();
  BF_CHECK(a != NULL);
  bf_env(&e, BF_BYTES, BF_BYTES, BF_FRAME);
  bf_req(&q, 0u, BF_SPAN, a + BF_VIDEO_OFF, BF_NEED, a + BF_VIDEO_OFF, BF_NEED);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BAD_ARG, IVS_REASON_RUN_BUFFER_OVERLAP));
  bf_req(&q, 0u, BF_SPAN, a + BF_VIDEO_OFF, BF_NEED, a + BF_VIDEO_OFF + BF_NEED - 1u, BF_NEED);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BAD_ARG, IVS_REASON_RUN_BUFFER_OVERLAP));
  bf_req(&q, 0u, BF_SPAN, a + BF_VIDEO_OFF, 4u * BF_NEED, a + BF_VIDEO_OFF + BF_NEED, BF_NEED);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BAD_ARG, IVS_REASON_RUN_BUFFER_OVERLAP));
  /* The real layout too: the overlap rule does not depend on the requirement. */
  bf_req(&q, 0u, BF_SPAN, a + BF_VIDEO_OFF, 8u, a + BF_VIDEO_OFF + 4u, 8u);
  BF_CHECK(bf_refused(&x, x.m, &q, NULL, a, IVS_E_BAD_ARG, IVS_REASON_RUN_BUFFER_OVERLAP));
  bf_req(&q, 0u, BF_SPAN, a + BF_VIDEO_OFF, BF_NEED, a + BF_VIDEO_OFF + BF_NEED, BF_NEED);
  memset(&r, BF_POISON, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  st = bf_call(x.m, &q, &r, &e);
  BF_CHECK(st == IVS_OK && r.stop_reason == IVS_STOP_TARGET && bf_arena_kept(a));
  rc = IVS_TEST_PASS;
done:
  free(a);
  ivs_ms_free(&x);
  return rc;
}

/* BAD_ARG (RUN_BUFFER_OVERLAP): a buffer overlapping the report. The report is then part of the
 * cause, so nothing is written into it either: the whole arena (report included) is unchanged.
 * Positive control: the report right after the video buffer runs (a video-only layout). */
static int bf_report_overlap(void)
{
  ivs_ms_inst x;
  ivs_run_snap s;
  ivs_run_env e;
  ivs_run_request q;
  uint8_t before[BF_ARENA];
  uint8_t *a = NULL;
  ivs_run_report *rep;
  ivs_status st;
  uint32_t pass;
  int rc = IVS_TEST_FAIL;
  s.copy = NULL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, BF_POISON));
  a = bf_arena();
  BF_CHECK(a != NULL);
  bf_env(&e, BF_BYTES, BF_BYTES, BF_FRAME);
  /* The report sits at BF_AUDIO_OFF (8-aligned); pass 0: video covers its last byte; pass 1:
   * audio starts at its first byte. */
  rep = (ivs_run_report *)(void *)(a + BF_AUDIO_OFF);
  rep->struct_size = (uint32_t)sizeof(ivs_run_report);
  for (pass = 0u; pass < 2u; pass++) {
    if (pass == 0u) {
      bf_req(&q, 0u, BF_SPAN, a + BF_AUDIO_OFF + sizeof(ivs_run_report) - 1u, BF_NEED, NULL, 0u);
    } else {
      bf_req(&q, 0u, BF_SPAN, a + BF_VIDEO_OFF, BF_NEED, a + BF_AUDIO_OFF, BF_NEED);
    }
    memcpy(before, a, BF_ARENA);
    BF_CHECK(ivs_run_snap_take(&s, &x, 1));
    st = bf_call(x.m, &q, rep, &e);
    BF_CHECK(st == IVS_E_BAD_ARG);
    BF_CHECK(memcmp(before, a, BF_ARENA) == 0);
    BF_CHECK(ivs_run_snap_same(&s, &x));
    ivs_run_snap_free(&s);
  }
  bf_env(&e, BF_BYTES, 0u, BF_FRAME);
  bf_req(&q, 0u, BF_SPAN, a + BF_AUDIO_OFF - BF_NEED, BF_NEED, NULL, 0u);
  st = bf_call(x.m, &q, rep, &e);
  if (st != IVS_OK) {
    ivs_test_diag("positive control: status %s reason %u", ivs_status_name(st), (unsigned)rep->refusal_reason);
  }
  BF_CHECK(st == IVS_OK && rep->stop_reason == IVS_STOP_TARGET);
  rc = IVS_TEST_PASS;
done:
  ivs_run_snap_free(&s);
  free(a);
  ivs_ms_free(&x);
  return rc;
}

/* The real (Phase 4) layout through the public call: every capacity suffices, NULL with 0
 * included, no byte is written and every output count is 0, on a running and a halted CPU. */
static int bf_real_layout_zero(void)
{
  static const uint64_t caps[4] = {0u, 1u, 7u, BF_ARENA - BF_AUDIO_OFF};
  ivs_ms_inst x;
  ivs_run_request q;
  ivs_run_report r;
  uint8_t *a = NULL;
  uint32_t i;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, BF_POISON));
  a = bf_arena();
  BF_CHECK(a != NULL);
  for (i = 0u; i < 4u; i++) {
    uint64_t now = bf_now(x.m);
    ivs_status st;
    bf_req(&q, 0u, now + 20000u, caps[i] != 0u ? a + BF_VIDEO_OFF : NULL, i == 3u ? 64u : caps[i],
           caps[i] != 0u ? a + BF_AUDIO_OFF : NULL, caps[i]);
    memset(&r, BF_POISON, sizeof r);
    r.struct_size = (uint32_t)sizeof r;
    st = ivs_machine_run(x.m, &q, &r);
    BF_CHECK(st == IVS_OK && r.stop_reason == IVS_STOP_TARGET && r.units_executed > 0u);
    BF_CHECK(r.video_frames_completed == 0u && r.video_bytes_written == 0u && r.audio_bytes_written == 0u);
    BF_CHECK(bf_arena_kept(a));
  }
  /* A non-NULL buffer with capacity 0 is accepted (it names no byte). */
  bf_req(&q, 0u, bf_now(x.m) + 1000u, a + BF_VIDEO_OFF, 0u, a + BF_VIDEO_OFF, 0u);
  memset(&r, BF_POISON, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  BF_CHECK(ivs_machine_run(x.m, &q, &r) == IVS_OK && r.video_bytes_written == 0u && bf_arena_kept(a));
  rc = IVS_TEST_PASS;
done:
  free(a);
  ivs_ms_free(&x);
  return rc;
}

/* The public call and the env seam with an equal env (built here from the public ivs_info layout
 * fields, not from the core's helper) give byte-equal reports and storage, call by call, over a
 * sequence of targets including one at or below now and the stop-on-halt flag. */
static int bf_entry_points_equal(void)
{
  static const uint64_t steps[5] = {1u, 1000u, 0u, 59736u, 120001u};
  ivs_ms_inst x;
  ivs_ms_inst y;
  ivs_info info;
  ivs_run_env e;
  ivs_run_request q;
  ivs_run_report rx;
  ivs_run_report ry;
  uint8_t *a = NULL;
  uint32_t i;
  int rc = IVS_TEST_FAIL;
  y.storage = NULL;
  y.m = NULL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, BF_POISON));
  BF_CHECK(ivs_ms_powered(&y, IVS_MS_CONTENT_A, BF_POISON));
  a = bf_arena();
  BF_CHECK(a != NULL);
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  BF_CHECK(ivs_machine_info(x.m, &info) == IVS_OK);
  memset(&e, 0, sizeof e);
  e.video_format = info.video_format;
  e.video_bytes_per_frame = info.video_bytes_per_frame;
  e.audio_format = info.audio_format;
  e.audio_bytes_per_frame = info.audio_bytes_per_frame;
  e.nominal_frame_ticks = info.nominal_frame_ticks;
  for (i = 0u; i < 5u; i++) {
    uint64_t now = bf_now(x.m);
    ivs_status sx;
    ivs_status sy;
    bf_req(&q, i == 4u ? IVS_RUN_F_STOP_ON_HALT : 0u, now + steps[i], a + BF_VIDEO_OFF, 32u, a + BF_AUDIO_OFF, 32u);
    memset(&rx, BF_POISON, sizeof rx);
    rx.struct_size = (uint32_t)sizeof rx;
    memset(&ry, BF_POISON, sizeof ry);
    ry.struct_size = (uint32_t)sizeof ry;
    sx = ivs_machine_run(x.m, &q, &rx);
    sy = ivs_machine_run_env_internal(y.m, &q, &ry, &e);
    BF_CHECK(sx == IVS_OK && sy == IVS_OK);
    BF_CHECK(memcmp(&rx, &ry, sizeof rx) == 0);
    BF_CHECK(memcmp(x.storage, y.storage, IVS_MACHINE_BYTES) == 0);
  }
  BF_CHECK(bf_arena_kept(a));
  rc = IVS_TEST_PASS;
done:
  free(a);
  ivs_ms_free(&y);
  ivs_ms_free(&x);
  return rc;
}

/* ---- precedence: the neighbours of the two new slots (D-05) ---- */

/* BAD_HANDLE over the buffer slot: a NULL handle with a NULL-with-capacity buffer, and a
 * destroyed instance with a buffer inside its storage (the overlap is tested only against a
 * valid instance). */
static int bf_prec_handle_over_buffer(void)
{
  ivs_ms_inst x;
  ivs_run_env e;
  ivs_run_request q;
  uint8_t *a = NULL;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, BF_POISON));
  a = bf_arena();
  BF_CHECK(a != NULL);
  bf_env(&e, BF_BYTES, BF_BYTES, BF_FRAME);
  bf_req(&q, 0u, BF_SPAN, NULL, 8u, NULL, 0u);
  BF_CHECK(bf_refused(&x, NULL, &q, &e, a, IVS_E_BAD_HANDLE, IVS_REASON_HANDLE));
  BF_CHECK(bf_refused(&x, NULL, &q, NULL, a, IVS_E_BAD_HANDLE, IVS_REASON_HANDLE));
  BF_CHECK(ivs_machine_destroy(x.m) == IVS_OK);
  bf_req(&q, 0u, BF_SPAN, x.storage + bf_inside_off(), BF_NEED, NULL, 0u);
  {
    ivs_run_snap s;
    bf_rep u;
    ivs_status st;
    memset(&u, BF_POISON, sizeof u);
    u.r.struct_size = (uint32_t)sizeof(ivs_run_report);
    BF_CHECK(ivs_run_snap_take(&s, &x, 0));
    st = ivs_machine_run(x.m, &q, &u.r);
    if (st != IVS_E_BAD_HANDLE || u.r.refusal_reason != IVS_REASON_HANDLE || !ivs_run_snap_same(&s, &x)) {
      ivs_test_diag("destroyed handle with a buffer inside: status %s reason %u", ivs_status_name(st),
                    (unsigned)u.r.refusal_reason);
      ivs_run_snap_free(&s);
      goto done;
    }
    ivs_run_snap_free(&s);
  }
  rc = IVS_TEST_PASS;
done:
  free(a);
  ivs_ms_free(&x);
  return rc;
}

/* UNSUPPORTED over the buffer slot: an unknown flag, and the reserved frame flag, with a NULL
 * buffer with capacity is UNSUPPORTED (RUN_FLAGS); a nonzero request tail with a video buffer
 * inside the instance is UNSUPPORTED (RUN_TAIL). */
static int bf_prec_unsupported_over_buffer(void)
{
  union {
    ivs_run_request q;
    uint8_t b[sizeof(ivs_run_request) + 8u];
  } big;
  ivs_ms_inst x;
  ivs_run_request q;
  uint8_t *a = NULL;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, BF_POISON));
  a = bf_arena();
  BF_CHECK(a != NULL);
  bf_req(&q, 4u, BF_SPAN, NULL, 8u, NULL, 0u);
  BF_CHECK(bf_refused(&x, x.m, &q, NULL, a, IVS_E_UNSUPPORTED, IVS_REASON_RUN_FLAGS));
  bf_req(&q, IVS_RUN_F_RESERVED_FRAME, BF_SPAN, NULL, 0u, NULL, 8u);
  BF_CHECK(bf_refused(&x, x.m, &q, NULL, a, IVS_E_UNSUPPORTED, IVS_REASON_RUN_FLAGS));
  memset(&big, 0, sizeof big);
  bf_req(&big.q, 0u, BF_SPAN, x.storage + bf_inside_off(), 8u, NULL, 0u);
  big.q.struct_size = (uint32_t)sizeof big;
  big.b[sizeof(ivs_run_request)] = 1u;
  BF_CHECK(bf_refused(&x, x.m, &big.q, NULL, a, IVS_E_UNSUPPORTED, IVS_REASON_RUN_TAIL));
  rc = IVS_TEST_PASS;
done:
  free(a);
  ivs_ms_free(&x);
  return rc;
}

/* The buffer slot over BAD_STATE: an OFF machine with a NULL-with-capacity buffer, and with
 * overlapping buffers, is BAD_ARG. */
static int bf_prec_buffer_over_state(void)
{
  ivs_ms_inst x;
  ivs_run_request q;
  uint8_t *a = NULL;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_new(&x, BF_POISON));
  a = bf_arena();
  BF_CHECK(a != NULL);
  bf_req(&q, 0u, BF_SPAN, NULL, 8u, NULL, 0u);
  BF_CHECK(bf_refused(&x, x.m, &q, NULL, a, IVS_E_BAD_ARG, IVS_REASON_RUN_BUFFER_NULL));
  bf_req(&q, 0u, BF_SPAN, a + BF_VIDEO_OFF, 8u, a + BF_VIDEO_OFF, 8u);
  BF_CHECK(bf_refused(&x, x.m, &q, NULL, a, IVS_E_BAD_ARG, IVS_REASON_RUN_BUFFER_OVERLAP));
  rc = IVS_TEST_PASS;
done:
  free(a);
  ivs_ms_free(&x);
  return rc;
}

/* OUT_OF_RANGE over BUFFER_TOO_SMALL: a target above 2^63 with an undersized buffer under the
 * synthetic layout is OUT_OF_RANGE (RUN_TARGET_RANGE). */
static int bf_prec_range_over_capacity(void)
{
  ivs_ms_inst x;
  ivs_run_env e;
  ivs_run_request q;
  uint8_t *a = NULL;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, BF_POISON));
  a = bf_arena();
  BF_CHECK(a != NULL);
  bf_env(&e, BF_BYTES, BF_BYTES, BF_FRAME);
  bf_req(&q, 0u, IVS_RUN_TARGET_MAX + 1u, a + BF_VIDEO_OFF, 1u, NULL, 0u);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_OUT_OF_RANGE, IVS_REASON_RUN_TARGET_RANGE));
  rc = IVS_TEST_PASS;
done:
  free(a);
  ivs_ms_free(&x);
  return rc;
}

/* ---- the write log and the not-requested rule (plan 05-02, D-14, D-17) ---- */

/* BF_LOG_OFF: a log buffer in the arena beside video (16..) and audio (128..); the events array of
 * the overlap case sits at BF_EV_OFF, clear of the report cases (report at BF_AUDIO_OFF, 136 bytes). */
#define BF_LOG_OFF 192u
#define BF_EV_OFF 320u

/* NULL with capacity 0 on every channel means "channel not requested": against a synthetic layout
 * that needs 48 bytes per channel the call runs, writes no byte, and equals (reports, whole storage,
 * composite and CPU digests) the same call under a layout that needs nothing. Old Phase 4
 * expectation of the video/audio half: BUFFER_TOO_SMALL (RUN_BUFFER_SMALL). */
static int bf_null_zero_not_requested(void)
{
  ivs_ms_inst x;
  ivs_ms_inst y;
  ivs_run_env e;
  ivs_run_env e0;
  ivs_run_request q;
  bf_rep ux;
  bf_rep uy;
  uint8_t cx[32];
  uint8_t px[32];
  uint8_t cy[32];
  uint8_t py[32];
  uint32_t i;
  int rc = IVS_TEST_FAIL;
  y.storage = NULL;
  y.m = NULL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, BF_POISON));
  BF_CHECK(ivs_ms_powered(&y, IVS_MS_CONTENT_A, BF_POISON));
  bf_env(&e, BF_BYTES, BF_BYTES, BF_FRAME);
  bf_log_env(&e, BF_BYTES);
  bf_env(&e0, 0u, 0u, BF_FRAME);
  for (i = 0u; i < 3u; i++) {
    uint64_t now = bf_now(x.m);
    ivs_status sx;
    ivs_status sy;
    BF_CHECK(now != UINT64_MAX && now == bf_now(y.m));
    bf_req(&q, 0u, now + BF_SPAN, NULL, 0u, NULL, 0u);
    memset(&ux, BF_POISON, sizeof ux);
    memset(&uy, BF_POISON, sizeof uy);
    ux.r.struct_size = (uint32_t)sizeof(ivs_run_report);
    uy.r.struct_size = (uint32_t)sizeof(ivs_run_report);
    sx = bf_call(x.m, &q, &ux.r, &e);
    sy = bf_call(y.m, &q, &uy.r, &e0);
    BF_CHECK(sx == IVS_OK && sy == IVS_OK);
    BF_CHECK(ux.r.stop_reason == IVS_STOP_TARGET && ux.r.units_executed > 0u);
    BF_CHECK(ux.r.video_bytes_written == 0u && ux.r.audio_bytes_written == 0u && ux.r.psg_log_bytes_written == 0u);
    BF_CHECK(memcmp(&ux.r, &uy.r, sizeof ux.r) == 0);
    BF_CHECK(memcmp(x.storage, y.storage, IVS_MACHINE_BYTES) == 0);
    BF_CHECK(ivs_ms_digests(x.m, cx, px) && ivs_ms_digests(y.m, cy, py));
    BF_CHECK(memcmp(cx, cy, 32u) == 0 && memcmp(px, py, 32u) == 0);
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&y);
  ivs_ms_free(&x);
  return rc;
}

/* A nonzero capacity below the requirement, and a non-NULL buffer with capacity 0 against a layout
 * that needs bytes, are still refused before any state change, per channel (video, audio, log). */
static int bf_nonzero_too_small_refused(void)
{
  ivs_ms_inst x;
  ivs_run_env e;
  ivs_run_request q;
  uint8_t *a = NULL;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, BF_POISON));
  a = bf_arena();
  BF_CHECK(a != NULL);
  bf_env(&e, BF_BYTES, BF_BYTES, BF_FRAME);
  bf_log_env(&e, BF_BYTES);
  /* One byte short on each channel in turn, the others at their exact requirement. */
  bf_req(&q, 0u, BF_SPAN, a + BF_VIDEO_OFF, BF_NEED - 1u, a + BF_AUDIO_OFF, BF_NEED);
  bf_log(&q, a + BF_LOG_OFF, BF_NEED);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BUFFER_TOO_SMALL, IVS_REASON_RUN_BUFFER_SMALL));
  bf_req(&q, 0u, BF_SPAN, a + BF_VIDEO_OFF, BF_NEED, a + BF_AUDIO_OFF, BF_NEED - 1u);
  bf_log(&q, a + BF_LOG_OFF, BF_NEED);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BUFFER_TOO_SMALL, IVS_REASON_RUN_BUFFER_SMALL));
  bf_req(&q, 0u, BF_SPAN, a + BF_VIDEO_OFF, BF_NEED, a + BF_AUDIO_OFF, BF_NEED);
  bf_log(&q, a + BF_LOG_OFF, BF_NEED - 1u);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BUFFER_TOO_SMALL, IVS_REASON_RUN_PSGLOG_SMALL));
  /* A non-NULL buffer with capacity 0 names a buffer: refused as in Phase 4 (log likewise). */
  bf_req(&q, 0u, BF_SPAN, a + BF_VIDEO_OFF, 0u, NULL, 0u);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BUFFER_TOO_SMALL, IVS_REASON_RUN_BUFFER_SMALL));
  bf_req(&q, 0u, BF_SPAN, NULL, 0u, NULL, 0u);
  bf_log(&q, a + BF_LOG_OFF, 0u);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BUFFER_TOO_SMALL, IVS_REASON_RUN_PSGLOG_SMALL));
  rc = IVS_TEST_PASS;
done:
  free(a);
  ivs_ms_free(&x);
  return rc;
}

/* BAD_ARG (RUN_PSGLOG_NULL): psg_log NULL with a nonzero capacity, through the public call (real
 * layout) and the seam, with a 0 capacity of the other two channels and with a large capacity. */
static int bf_psglog_null(void)
{
  ivs_ms_inst x;
  ivs_run_env e;
  ivs_run_request q;
  uint8_t *a = NULL;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, BF_POISON));
  a = bf_arena();
  BF_CHECK(a != NULL);
  bf_env(&e, BF_BYTES, BF_BYTES, BF_FRAME);
  bf_log_env(&e, BF_BYTES);
  bf_req(&q, 0u, BF_SPAN, NULL, 0u, NULL, 0u);
  bf_log(&q, NULL, 1u);
  BF_CHECK(bf_refused(&x, x.m, &q, NULL, a, IVS_E_BAD_ARG, IVS_REASON_RUN_PSGLOG_NULL));
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BAD_ARG, IVS_REASON_RUN_PSGLOG_NULL));
  bf_req(&q, 0u, BF_SPAN, a + BF_VIDEO_OFF, BF_NEED, a + BF_AUDIO_OFF, BF_NEED);
  bf_log(&q, NULL, UINT64_MAX);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BAD_ARG, IVS_REASON_RUN_PSGLOG_NULL));
  /* The OFF machine: the buffer slot precedes BAD_STATE. */
  rc = IVS_TEST_PASS;
done:
  free(a);
  ivs_ms_free(&x);
  return rc;
}

/* BAD_ARG (RUN_PSGLOG_OVERLAP): psg_log overlapping the instance (inside, and straddling its last
 * byte), video, audio, the event array and a span running past the top of the address space. The refusal changes nothing: the poisoned arena is
 * untouched and the instance snapshot is equal (bf_refused). Positive controls: disjoint adjacent
 * buffers run; a v1-sized report (112 bytes) with the log right after it runs and the log bytes
 * stay poison, while the log one byte lower overlaps the bytes the core would write. */
static int bf_psglog_overlap(void)
{
  ivs_ms_inst x;
  ivs_run_snap s;
  ivs_run_env e;
  ivs_run_request q;
  uint8_t before[BF_ARENA];
  uint8_t *a = NULL;
  ivs_run_report *rep;
  ivs_status st;
  uint32_t pass;
  int rc = IVS_TEST_FAIL;
  s.copy = NULL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, BF_POISON));
  a = bf_arena();
  BF_CHECK(a != NULL);
  bf_env(&e, BF_BYTES, BF_BYTES, BF_FRAME);
  bf_log_env(&e, BF_BYTES);
  /* The instance. */
  bf_req(&q, 0u, BF_SPAN, NULL, 0u, NULL, 0u);
  bf_log(&q, x.storage + bf_inside_off(), BF_NEED);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BAD_ARG, IVS_REASON_RUN_PSGLOG_OVERLAP));
  BF_CHECK(bf_refused(&x, x.m, &q, NULL, a, IVS_E_BAD_ARG, IVS_REASON_RUN_PSGLOG_OVERLAP));
  bf_log(&q, x.storage + IVS_MACHINE_BYTES - 1u, BF_NEED);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BAD_ARG, IVS_REASON_RUN_PSGLOG_OVERLAP));
  /* Video, audio: the same buffer, one byte shared, inside. */
  bf_req(&q, 0u, BF_SPAN, a + BF_VIDEO_OFF, BF_NEED, a + BF_AUDIO_OFF, BF_NEED);
  bf_log(&q, a + BF_VIDEO_OFF, BF_NEED);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BAD_ARG, IVS_REASON_RUN_PSGLOG_OVERLAP));
  bf_log(&q, a + BF_VIDEO_OFF + BF_NEED - 1u, BF_NEED);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BAD_ARG, IVS_REASON_RUN_PSGLOG_OVERLAP));
  bf_log(&q, a + BF_AUDIO_OFF, BF_NEED);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BAD_ARG, IVS_REASON_RUN_PSGLOG_OVERLAP));
  bf_log(&q, a + BF_AUDIO_OFF - 4u, 8u); /* ends inside audio; the real layout too */
  BF_CHECK(bf_refused(&x, x.m, &q, NULL, a, IVS_E_BAD_ARG, IVS_REASON_RUN_PSGLOG_OVERLAP));
  bf_log(&q, a + BF_VIDEO_OFF - 8u, 8u + 4u);
  BF_CHECK(bf_refused(&x, x.m, &q, NULL, a, IVS_E_BAD_ARG, IVS_REASON_RUN_PSGLOG_OVERLAP));
  /* The event array (2 events, 64 bytes at BF_EV_OFF; never read by the refusal). */
  bf_req(&q, 0u, BF_SPAN, NULL, 0u, NULL, 0u);
  q.events = (const ivs_input_event *)(const void *)(a + BF_EV_OFF);
  q.event_count = 2u;
  bf_log(&q, a + BF_EV_OFF + 32u, BF_NEED);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BAD_ARG, IVS_REASON_RUN_PSGLOG_OVERLAP));
  bf_log(&q, a + BF_EV_OFF - BF_NEED + 1u, BF_NEED);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BAD_ARG, IVS_REASON_RUN_PSGLOG_OVERLAP));
  /* A span past the top of the address space: a pointer near the top (never dereferenced). */
  bf_req(&q, 0u, BF_SPAN, NULL, 0u, NULL, 0u);
  bf_log(&q, (uint8_t *)(uintptr_t)(UINTPTR_MAX - 7u), 16u);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BAD_ARG, IVS_REASON_RUN_PSGLOG_OVERLAP));
  /* (A capacity that wraps from an ordinary pointer would also cover the caller's report, which is
   * then part of the cause and is not written; that stays in the report case below.) */
  /* The report: it is part of the cause, so nothing is written into it either (whole arena kept). */
  rep = (ivs_run_report *)(void *)(a + BF_AUDIO_OFF);
  for (pass = 0u; pass < 2u; pass++) {
    rep->struct_size = (uint32_t)sizeof(ivs_run_report);
    bf_req(&q, 0u, BF_SPAN, NULL, 0u, NULL, 0u);
    bf_log(&q, pass == 0u ? a + BF_AUDIO_OFF + sizeof(ivs_run_report) - 1u : a + BF_AUDIO_OFF, BF_NEED);
    memcpy(before, a, BF_ARENA);
    BF_CHECK(ivs_run_snap_take(&s, &x, 1));
    st = bf_call(x.m, &q, rep, &e);
    BF_CHECK(st == IVS_E_BAD_ARG);
    BF_CHECK(memcmp(before, a, BF_ARENA) == 0);
    BF_CHECK(ivs_run_snap_same(&s, &x));
    ivs_run_snap_free(&s);
  }
  /* Positive: a v1-sized report (112 bytes) and the log right after it runs; the log is untouched. */
  rep->struct_size = IVS_RUN_REPORT_V1_SIZE;
  bf_req(&q, 0u, BF_SPAN, NULL, 0u, NULL, 0u);
  bf_log(&q, a + BF_AUDIO_OFF + IVS_RUN_REPORT_V1_SIZE, BF_NEED);
  st = bf_call(x.m, &q, rep, &e);
  if (st != IVS_OK) {
    ivs_test_diag("positive control: status %s reason %u", ivs_status_name(st), (unsigned)rep->refusal_reason);
  }
  BF_CHECK(st == IVS_OK && rep->stop_reason == IVS_STOP_TARGET && rep->struct_size == IVS_RUN_REPORT_V1_SIZE);
  {
    size_t i;
    for (i = 0u; i < BF_NEED; i++) {
      BF_CHECK(a[BF_AUDIO_OFF + IVS_RUN_REPORT_V1_SIZE + i] == BF_POISON);
    }
  }
  /* One byte lower the log reaches the last byte of the 112 the core writes. */
  rep->struct_size = IVS_RUN_REPORT_V1_SIZE;
  bf_log(&q, a + BF_AUDIO_OFF + IVS_RUN_REPORT_V1_SIZE - 1u, BF_NEED);
  memcpy(before, a, BF_ARENA);
  st = bf_call(x.m, &q, rep, &e);
  BF_CHECK(st == IVS_E_BAD_ARG && memcmp(before, a, BF_ARENA) == 0);
  /* Disjoint adjacent buffers: video, audio, log back to back run. */
  bf_req(&q, 0u, BF_SPAN, a + BF_VIDEO_OFF, BF_NEED, a + BF_VIDEO_OFF + BF_NEED, BF_NEED);
  bf_log(&q, a + BF_VIDEO_OFF + 2u * BF_NEED, BF_NEED);
  {
    bf_rep u;
    memset(&u, BF_POISON, sizeof u);
    u.r.struct_size = (uint32_t)sizeof(ivs_run_report);
    st = bf_call(x.m, &q, &u.r, &e);
    BF_CHECK(st == IVS_OK && u.r.stop_reason == IVS_STOP_TARGET);
  }
  rc = IVS_TEST_PASS;
done:
  ivs_run_snap_free(&s);
  free(a);
  ivs_ms_free(&x);
  return rc;
}

/* BUFFER_TOO_SMALL (RUN_PSGLOG_SMALL): the log capacity one byte below the requirement is refused
 * before any unit, at machine time 0 and after three seam-stepped units; the exact capacity is
 * accepted, writes no byte (nothing produces log bytes yet) and leaves the arena untouched. The
 * synthetic env needs 16 bytes of log per 1000-tick frame, 48 for a 2500-tick run. */
static int bf_psglog_small(void)
{
  ivs_ms_inst x;
  ivs_run_env e;
  ivs_run_request q;
  bf_rep u;
  uint8_t *a = NULL;
  uint64_t now;
  uint32_t pass;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, BF_POISON));
  a = bf_arena();
  BF_CHECK(a != NULL);
  bf_env(&e, 0u, 0u, BF_FRAME);
  bf_log_env(&e, BF_BYTES);
  for (pass = 0u; pass < 2u; pass++) {
    ivs_status st;
    if (pass == 1u) {
      BF_CHECK(ivs_ms_step(x.m, 3u, NULL));
    }
    now = bf_now(x.m);
    BF_CHECK(now != UINT64_MAX && (pass == 0u) == (now == 0u));
    bf_req(&q, 0u, now + BF_SPAN, NULL, 0u, NULL, 0u);
    bf_log(&q, a + BF_LOG_OFF, BF_NEED - 1u);
    BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BUFFER_TOO_SMALL, IVS_REASON_RUN_PSGLOG_SMALL));
    bf_log(&q, a + BF_LOG_OFF, BF_NEED);
    memset(&u, BF_POISON, sizeof u);
    u.r.struct_size = (uint32_t)sizeof(ivs_run_report);
    st = bf_call(x.m, &q, &u.r, &e);
    BF_CHECK(st == IVS_OK && u.r.stop_reason == IVS_STOP_TARGET && u.r.now_ticks >= now + BF_SPAN);
    BF_CHECK(u.r.psg_log_bytes_written == 0u && bf_arena_kept(a));
  }
  /* A frame count that overflows the product refuses and does not wrap: 2^63 ticks of 1-tick frames
   * at 16 bytes per frame is above UINT64_MAX bytes. */
  e.nominal_frame_ticks = 1u;
  bf_req(&q, 0u, IVS_RUN_TARGET_MAX, NULL, 0u, NULL, 0u);
  bf_log(&q, a + BF_LOG_OFF, BF_NEED);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BUFFER_TOO_SMALL, IVS_REASON_RUN_PSGLOG_SMALL));
  rc = IVS_TEST_PASS;
done:
  free(a);
  ivs_ms_free(&x);
  return rc;
}

/* Precedence inside the buffer slot (D-14): any video or audio problem is reported before any
 * write-log problem, and the log stage precedes BAD_STATE and OUT_OF_RANGE (the slots around it
 * are unchanged). */
static int bf_prec_psglog_after_audio(void)
{
  ivs_ms_inst x;
  ivs_ms_inst off;
  ivs_run_env e;
  ivs_run_request q;
  uint8_t *a = NULL;
  int rc = IVS_TEST_FAIL;
  off.storage = NULL;
  off.m = NULL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, BF_POISON));
  BF_CHECK(ivs_ms_new(&off, BF_POISON));
  a = bf_arena();
  BF_CHECK(a != NULL);
  bf_env(&e, BF_BYTES, BF_BYTES, BF_FRAME);
  bf_log_env(&e, BF_BYTES);
  /* Audio NULL with a capacity, log NULL with a capacity: the audio reason. */
  bf_req(&q, 0u, BF_SPAN, NULL, 0u, NULL, 8u);
  bf_log(&q, NULL, 8u);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BAD_ARG, IVS_REASON_RUN_BUFFER_NULL));
  /* Audio NULL, log overlapping the instance: still the audio reason. */
  bf_log(&q, x.storage + bf_inside_off(), 8u);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BAD_ARG, IVS_REASON_RUN_BUFFER_NULL));
  /* Audio overlapping video, log NULL with a capacity: the audio reason. */
  bf_req(&q, 0u, BF_SPAN, a + BF_VIDEO_OFF, BF_NEED, a + BF_VIDEO_OFF, BF_NEED);
  bf_log(&q, NULL, 8u);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BAD_ARG, IVS_REASON_RUN_BUFFER_OVERLAP));
  /* Audio overlapping video and the log overlapping the instance: the audio reason. */
  bf_log(&q, x.storage + bf_inside_off(), 8u);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BAD_ARG, IVS_REASON_RUN_BUFFER_OVERLAP));
  /* Audio short, log short: the capacity stage names video and audio first. */
  bf_req(&q, 0u, BF_SPAN, a + BF_VIDEO_OFF, BF_NEED, a + BF_AUDIO_OFF, BF_NEED - 1u);
  bf_log(&q, a + BF_LOG_OFF, BF_NEED - 1u);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BUFFER_TOO_SMALL, IVS_REASON_RUN_BUFFER_SMALL));
  /* Good video and audio with a NULL log: the log reason. */
  bf_req(&q, 0u, BF_SPAN, a + BF_VIDEO_OFF, BF_NEED, a + BF_AUDIO_OFF, BF_NEED);
  bf_log(&q, NULL, 8u);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_BAD_ARG, IVS_REASON_RUN_PSGLOG_NULL));
  /* The log stage precedes OUT_OF_RANGE and BAD_STATE; BUFFER_TOO_SMALL follows OUT_OF_RANGE. */
  bf_req(&q, 0u, BF_SPAN, NULL, 0u, NULL, 0u);
  bf_log(&q, NULL, 8u);
  BF_CHECK(bf_refused(&off, off.m, &q, NULL, a, IVS_E_BAD_ARG, IVS_REASON_RUN_PSGLOG_NULL));
  bf_req(&q, 0u, IVS_RUN_TARGET_MAX + 1u, NULL, 0u, NULL, 0u);
  bf_log(&q, a + BF_LOG_OFF, 1u);
  BF_CHECK(bf_refused(&x, x.m, &q, &e, a, IVS_E_OUT_OF_RANGE, IVS_REASON_RUN_TARGET_RANGE));
  bf_req(&q, 0u, BF_SPAN, NULL, 0u, NULL, 0u);
  bf_log(&q, a + BF_LOG_OFF, 1u);
  BF_CHECK(bf_refused(&off, off.m, &q, &e, a, IVS_E_BAD_STATE, IVS_REASON_RUN_STATE_OFF));
  rc = IVS_TEST_PASS;
done:
  free(a);
  ivs_ms_free(&off);
  ivs_ms_free(&x);
  return rc;
}

const ivs_test_case ivs_run_buffers_cases[] = {
    {"run.buffers.too-small-video", bf_too_small_video, NULL},
    {"run.buffers.too-small-audio", bf_too_small_audio, NULL},
    {"run.buffers.exact-fit-video", bf_exact_fit_video, NULL},
    {"run.buffers.exact-fit-audio", bf_exact_fit_audio, NULL},
    {"run.buffers.exact-fit-both", bf_exact_fit_both, NULL},
    {"run.buffers.zero-layout", bf_zero_layout, NULL},
    {"run.buffers.null-with-capacity", bf_null_with_capacity, NULL},
    {"run.buffers.inside-instance", bf_inside_instance, NULL},
    {"run.buffers.video-audio-overlap", bf_video_audio_overlap, NULL},
    {"run.buffers.report-overlap", bf_report_overlap, NULL},
    {"run.buffers.real-layout-zero", bf_real_layout_zero, NULL},
    {"run.buffers.entry-points-equal", bf_entry_points_equal, NULL},
    {"run.prec.handle-over-buffer", bf_prec_handle_over_buffer, NULL},
    {"run.prec.unsupported-over-buffer", bf_prec_unsupported_over_buffer, NULL},
    {"run.prec.buffer-over-state", bf_prec_buffer_over_state, NULL},
    {"run.prec.range-over-capacity", bf_prec_range_over_capacity, NULL},
    {"run.buffers.null-zero-not-requested", bf_null_zero_not_requested, NULL},
    {"run.buffers.nonzero-too-small-refused", bf_nonzero_too_small_refused, NULL},
    {"run.buffers.psglog-null", bf_psglog_null, NULL},
    {"run.buffers.psglog-overlap", bf_psglog_overlap, NULL},
    {"run.buffers.psglog-small", bf_psglog_small, NULL},
    {"run.prec.psglog-after-audio", bf_prec_psglog_after_audio, NULL},
};
const size_t ivs_run_buffers_count = sizeof ivs_run_buffers_cases / sizeof ivs_run_buffers_cases[0];
