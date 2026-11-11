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
#define BF_ARENA 256u
#define BF_VIDEO_OFF 16u
#define BF_AUDIO_OFF 128u
#define BF_REP_BYTES 128u

typedef union bf_rep {
  ivs_run_report r;
  uint8_t b[BF_REP_BYTES];
} bf_rep;

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

const ivs_test_case ivs_run_buffers_cases[] = {
    {"run.buffers.too-small-video", bf_too_small_video, NULL},
};
const size_t ivs_run_buffers_count = sizeof ivs_run_buffers_cases / sizeof ivs_run_buffers_cases[0];
