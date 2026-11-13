/* SPDX-License-Identifier: MIT */
/* Suite video, families slice, fuzz and iso (plan 06-09, D-16, A-06, T-06-12): slicing invariance of the slot stream, the chain, the count
 * and the whole instance (the chain is per frame, not per call), fixed-seed fuzz over random flags, buffer presence and slicing, and the
 * isolation of two instances stepped alternately. Total run length is three frames plus a little (the third frame ends at 179,208). The
 * reference is ONE call to the same final machine time: every sliced run ends at the first unit boundary at or after the total, so both
 * end at the same boundary. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "video_cases.h"

#define VS_SLOT 15296u
#define VS_F 59736u
#define VS_TOTAL (3u * VS_F + 4000u)
#define VS_CAP(d) ((uint64_t)VS_SLOT * ((uint64_t)(d) / VS_F + 2u + 1u))

static uint32_t vs_xs(uint32_t *s)
{
  uint32_t x = *s;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  *s = x;
  return x;
}

static ivs_status vs_call(ivs_ms_inst *x, uint32_t flags, uint64_t target, uint8_t *video, uint64_t cap, ivs_run_report *r)
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

static int vs_guest(ivs_ms_inst *x)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = vp_prog_strobe_once(w);
  return sc_guest(x, w, n, 0u);
}

static uint64_t vs_now(const ivs_ms_inst *x)
{
  return ivs_machine_from_handle(x->m)->time_ticks;
}

/* Slices the run: every call runs to min(now + step, total), step from slice (fixed) or from the xorshift seed (1 + x % 2999); in windows
 * of 300 ticks around each frame end the step is 1 when fine is set. The delivered slots are concatenated into out (cap bytes); returns the
 * delivered bytes or (uint64_t)-1 on failure. */
static uint64_t vs_sliced(ivs_ms_inst *x, uint64_t slice, uint32_t seed, int fine, uint64_t total, uint8_t *out, uint64_t cap)
{
  uint64_t got = 0u;
  uint32_t s = seed;
  uint8_t *tmp = (uint8_t *)malloc((size_t)VS_CAP(total));
  if (tmp == NULL) {
    return (uint64_t)-1;
  }
  while (vs_now(x) < total) {
    ivs_run_report r;
    uint64_t now = vs_now(x);
    uint64_t step = slice != 0u ? slice : (seed != 0u ? 1u + (uint64_t)(vs_xs(&s) % 2999u) : total);
    uint64_t target;
    if (fine) {
      uint64_t k = now / VS_F;
      uint64_t to_end = (k + 1u) * VS_F - now;
      uint64_t since = now - k * VS_F;
      if (to_end <= 300u || since <= 300u) {
        step = 1u;
      }
    }
    target = now + step < total ? now + step : total;
    memset(tmp, (int)VP_POISON, (size_t)VS_CAP(total));
    if (vs_call(x, 0u, target, tmp, VS_CAP(target - now), &r) != IVS_OK || got + r.video_bytes_written > cap) {
      free(tmp);
      return (uint64_t)-1;
    }
    memcpy(out + got, tmp, (size_t)r.video_bytes_written);
    got += r.video_bytes_written;
  }
  free(tmp);
  return got;
}

/* One call versus the sliced run: the concatenated slot stream, the chain, the count and the whole instance equal. */
static int vs_compare(uint64_t slice, uint32_t seed, int fine)
{
  ivs_ms_inst a;
  ivs_ms_inst b;
  ivs_run_report r;
  uint8_t *sa = (uint8_t *)malloc((size_t)VS_CAP(VS_TOTAL));
  uint8_t *sb = (uint8_t *)malloc((size_t)VS_CAP(VS_TOTAL));
  uint64_t ga;
  int ok = 0;
  VC_CHECK(sa != NULL && sb != NULL);
  memset(sa, (int)VP_POISON, (size_t)VS_CAP(VS_TOTAL));
  memset(sb, (int)VP_POISON, (size_t)VS_CAP(VS_TOTAL));
  if (!vs_guest(&a) || !vs_guest(&b)) {
    goto done;
  }
  if (vs_call(&a, 0u, VS_TOTAL, sa, VS_CAP(VS_TOTAL), &r) != IVS_OK) {
    goto done;
  }
  ga = vs_sliced(&b, slice, seed, fine, VS_TOTAL, sb, VS_CAP(VS_TOTAL));
  if (ga != r.video_bytes_written || r.video_frames_completed != 3u || memcmp(sa, sb, (size_t)ga) != 0 ||
      memcmp(a.storage, b.storage, IVS_MACHINE_BYTES) != 0 || ivs_machine_from_handle(a.m)->obs.video.count != 3u) {
    ivs_test_diag("sliced run differs (slice %llu seed %u): bytes %llu vs %llu", (unsigned long long)slice, (unsigned)seed,
                  (unsigned long long)ga, (unsigned long long)r.video_bytes_written);
    goto done;
  }
  ok = 1;
done:
  ivs_ms_free(&a);
  ivs_ms_free(&b);
  free(sa);
  free(sb);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int vs_slice_1_near_events(void)
{
  return vs_compare(468u, 0u, 1);
}

static int vs_slice_7(void)
{
  return vs_compare(7u, 0u, 0);
}

static int vs_slice_468(void)
{
  return vs_compare(468u, 0u, 0);
}

static int vs_slice_4096(void)
{
  return vs_compare(4096u, 0u, 0);
}

static int vs_slice_seeded(void)
{
  return vs_compare(0u, 0x9E3779B9u, 0) == IVS_TEST_PASS && vs_compare(0u, 0x1234567u, 1) == IVS_TEST_PASS ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int vs_slice_2_22(void)
{
  return vs_compare(1ull << 22, 0u, 0);
}

/* A sequence of frame-stop calls equals the same run in one call: three flagged calls to the frame ends reach the state and the chain of a
 * single call that ends at the same boundary. */
static int vs_slice_frame_stops(void)
{
  ivs_ms_inst a;
  ivs_ms_inst b;
  ivs_run_report r;
  uint8_t *buf = (uint8_t *)malloc(2u * VS_SLOT);
  uint8_t *ref = (uint8_t *)malloc((size_t)VS_CAP(3u * VS_F));
  uint32_t k;
  int ok = 0;
  VC_CHECK(buf != NULL && ref != NULL);
  memset(ref, (int)VP_POISON, (size_t)VS_CAP(3u * VS_F));
  if (!vs_guest(&a) || !vs_guest(&b)) {
    free(buf);
    free(ref);
    return IVS_TEST_FAIL;
  }
  for (k = 0u; k < 3u; k++) {
    memset(buf, (int)VP_POISON, 2u * VS_SLOT);
    if (vs_call(&a, IVS_RUN_F_STOP_ON_FRAME, IVS_RUN_TARGET_MAX, buf, VS_SLOT, &r) != IVS_OK || r.video_bytes_written != VS_SLOT ||
        r.stop_reason != IVS_STOP_FRAME) {
      goto done;
    }
    /* each flagged slot equals the matching slot of the unflagged one-call stream, compared below */
    memcpy(ref + (size_t)k * VS_SLOT, buf, VS_SLOT);
  }
  {
    uint8_t *one = (uint8_t *)malloc((size_t)VS_CAP(vs_now(&a)));
    int same;
    if (one == NULL) {
      goto done;
    }
    memset(one, (int)VP_POISON, (size_t)VS_CAP(vs_now(&a)));
    same = vs_call(&b, 0u, vs_now(&a), one, VS_CAP(vs_now(&a)), &r) == IVS_OK && r.video_bytes_written == 3u * VS_SLOT &&
           memcmp(one, ref, 3u * VS_SLOT) == 0 && memcmp(a.storage, b.storage, IVS_MACHINE_BYTES) == 0;
    free(one);
    if (!same) {
      goto done;
    }
  }
  ok = 1;
done:
  ivs_ms_free(&a);
  ivs_ms_free(&b);
  free(buf);
  free(ref);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* Fuzz: a fixed xorshift32 seed drives 100,000 ticks of random calls (random step, a flag one time in three, a buffer one time in two); the
 * final instance and chain equal one call to the same time, and every slot a call delivered equals the slot of the same index of the reference
 * stream. The buffer of a flagged call is one slot, of a plain call the full law. */
static int vs_fuzz(uint32_t seed)
{
  ivs_ms_inst a;
  ivs_ms_inst b;
  ivs_run_report r;
  uint32_t s = seed;
  uint8_t *tmp = (uint8_t *)malloc((size_t)VS_CAP(4000u));
  uint8_t *ref = (uint8_t *)malloc((size_t)VS_CAP(100000u));
  uint64_t refn = 0u;
  int ok = 0;
  VC_CHECK(tmp != NULL && ref != NULL);
  if (!vs_guest(&a) || !vs_guest(&b)) {
    free(tmp);
    free(ref);
    return IVS_TEST_FAIL;
  }
  while (vs_now(&a) < 100000u) {
    uint32_t flag = vs_xs(&s) % 3u == 0u ? IVS_RUN_F_STOP_ON_FRAME : 0u;
    uint32_t buffered = vs_xs(&s) % 2u;
    uint64_t step = 1u + (uint64_t)(vs_xs(&s) % 3999u);
    uint64_t target = vs_now(&a) + step;
    uint64_t cap = flag != 0u ? VS_SLOT : VS_CAP(step);
    memset(tmp, (int)VP_POISON, (size_t)cap);
    if (vs_call(&a, flag, target, buffered != 0u ? tmp : NULL, buffered != 0u ? cap : 0u, &r) != IVS_OK) {
      goto done;
    }
    if (buffered != 0u && r.video_bytes_written != r.video_frames_completed * VS_SLOT) {
      goto done;
    }
    if (buffered != 0u && r.video_bytes_written != 0u) {
      uint64_t idx = r.video_first_index;
      if (idx + r.video_frames_completed > VS_CAP(100000u) / VS_SLOT) {
        goto done;
      }
      memcpy(ref + idx * VS_SLOT, tmp, (size_t)r.video_bytes_written);
      refn = refn > (idx + r.video_frames_completed) * VS_SLOT ? refn : (idx + r.video_frames_completed) * VS_SLOT;
    }
  }
  {
    uint8_t *one = (uint8_t *)malloc((size_t)VS_CAP(vs_now(&a)));
    uint64_t i;
    int same;
    if (one == NULL) {
      goto done;
    }
    memset(one, (int)VP_POISON, (size_t)VS_CAP(vs_now(&a)));
    same = vs_call(&b, 0u, vs_now(&a), one, VS_CAP(vs_now(&a)), &r) == IVS_OK && memcmp(a.storage, b.storage, IVS_MACHINE_BYTES) == 0;
    /* every slot a buffered call delivered equals the reference stream's slot (the unwritten ones stay poison in ref and are skipped) */
    for (i = 0u; same && i < refn / VS_SLOT; i++) {
      const uint8_t *got = ref + i * VS_SLOT;
      int untouched = got[0] == VP_POISON && got[1] == VP_POISON && got[VS_SLOT - 1u] == VP_POISON;
      if (!untouched && memcmp(got, one + i * VS_SLOT, VS_SLOT) != 0) {
        same = 0;
      }
    }
    free(one);
    if (!same) {
      goto done;
    }
  }
  ok = 1;
done:
  ivs_ms_free(&a);
  ivs_ms_free(&b);
  free(tmp);
  free(ref);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

#define VS_FUZZ(n, seed) \
  static int vs_fuzz_##n(void) { return vs_fuzz(seed); }
VS_FUZZ(1, 0x00000001u)
VS_FUZZ(2, 0x2545F491u)
VS_FUZZ(3, 0x9E3779B9u)
VS_FUZZ(4, 0xDEADBEEFu)
VS_FUZZ(5, 0x0BADC0DEu)
VS_FUZZ(6, 0x13579BDFu)
VS_FUZZ(7, 0xCAFEF00Du)
VS_FUZZ(8, 0x7F4A7C15u)

/* Isolation: one instance stepped alone and the same guest stepped in alternate calls with a second instance running a different guest (a
 * display strobe against a border store) end byte for byte equal: nothing is shared between instances (no static state). */
static int vs_iso_interleaved(void)
{
  ivs_ms_inst a;
  ivs_ms_inst a2;
  ivs_ms_inst o;
  uint16_t w[SC_PROG_MAX];
  size_t n = vp_prog_border(w, 7u);
  ivs_run_report r;
  uint32_t k;
  int ok = 0;
  if (!vs_guest(&a) || !vs_guest(&a2)) {
    return IVS_TEST_FAIL;
  }
  if (!sc_guest(&o, w, n, 0u)) {
    ivs_ms_free(&a);
    ivs_ms_free(&a2);
    return IVS_TEST_FAIL;
  }
  if (vs_call(&a, 0u, VS_TOTAL, NULL, 0u, &r) != IVS_OK) {
    goto done;
  }
  for (k = 1u; vs_now(&a2) < VS_TOTAL && k < 5000u; k++) {
    if (vs_call(&a2, 0u, vs_now(&a2) + 4000u, NULL, 0u, &r) != IVS_OK || vs_call(&o, 0u, vs_now(&o) + 3000u, NULL, 0u, &r) != IVS_OK) {
      goto done;
    }
  }
  if (vs_now(&a) != vs_now(&a2) && vs_call(&a, 0u, vs_now(&a2), NULL, 0u, &r) != IVS_OK) {
    goto done;
  }
  ok = vs_now(&a) == vs_now(&a2) && memcmp(a.storage, a2.storage, IVS_MACHINE_BYTES) == 0;
done:
  ivs_ms_free(&a);
  ivs_ms_free(&a2);
  ivs_ms_free(&o);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_case ivs_video_slice_cases[] = {
    {"video.slice.one-call-equals-slices-near-events", vs_slice_1_near_events, NULL},
    {"video.slice.7", vs_slice_7, NULL},
    {"video.slice.468", vs_slice_468, NULL},
    {"video.slice.4096", vs_slice_4096, NULL},
    {"video.slice.seeded", vs_slice_seeded, NULL},
    {"video.slice.2-22", vs_slice_2_22, NULL},
    {"video.slice.frame-stop-sequence", vs_slice_frame_stops, NULL},
    {"video.fuzz.seed01", vs_fuzz_1, NULL},
    {"video.fuzz.seed02", vs_fuzz_2, NULL},
    {"video.fuzz.seed03", vs_fuzz_3, NULL},
    {"video.fuzz.seed04", vs_fuzz_4, NULL},
    {"video.fuzz.seed05", vs_fuzz_5, NULL},
    {"video.fuzz.seed06", vs_fuzz_6, NULL},
    {"video.fuzz.seed07", vs_fuzz_7, NULL},
    {"video.fuzz.seed08", vs_fuzz_8, NULL},
    {"video.iso.interleaved-instances", vs_iso_interleaved, NULL},
};
const size_t ivs_video_slice_count = sizeof ivs_video_slice_cases / sizeof ivs_video_slice_cases[0];
