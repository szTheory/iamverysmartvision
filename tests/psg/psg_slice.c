/* SPDX-License-Identifier: MIT */
/* Suite psg, family slice (AUD-02, API-03 for audio, plan 05-06, D-16, A-02, T-05-17): the audio stream does not
 * depend on how a run is sliced. The PSG is caught up to every unit's boundary in the step seam and in the run call
 * alike, never to the call target; a seam-stepped instance equals a run-call instance byte for byte. The guest is
 * stand-in program TONE (silent until step 4096 = tick 65,536, then level changes every step), so a run to 70,000
 * ticks delivers about 4,375 samples with several hundred changes after the first edge.
 *   psg.slice.seam-equals-run          300 seam-stepped units and one run call to the same time: whole storage and
 *                                      delivered bytes equal
 *   psg.slice.samples-split            one call to 70,000 equals calls of 1, 7 and 13 ticks and the cycle 1, 7, 13:
 *                                      concatenated bytes, whole storage, every digest and the audio_first_index chain
 *   psg.slice.samples-slice-1          run(now + 1) is one unit; 200 of them equal 200 seam units, bytes and storage
 *   psg.slice.tone-write-mid-slice     slices of 1 tick through the first 400 ticks (the guest's register writes land
 *                                      inside them), then one call: equal to one call
 *   psg.slice.overshoot                a call whose target lies inside a unit delivers the samples due up to the
 *                                      unit's end (a unit end that crosses a sample tick)
 *   psg.slice.no-catch-up-to-target    after a call the PSG is exactly at floor(now / 16) samples, whatever the target
 *   psg.slice.interleaved-instances    two instances run in turn with calls of 700 ticks equal each run alone
 *   psg.slice.reset-between-calls      a power-on reset between calls gives audio_first_index 0 and the first run's
 *                                      bytes again */
#include "psg_util.h"

#include "psg_cases.h"

#define SL_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      goto done;                                                                                       \
    }                                                                                                  \
  } while (0)

#define SL_TARGET 70000u
#define SL_BYTES (4u * 4500u)

/* 1 when the storage and every digest domain of two instances are equal. */
static int sl_same(const ivs_ms_inst *a, const ivs_ms_inst *b)
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
    if (ivs_machine_state_digest(a->m, d, da) != IVS_OK || ivs_machine_state_digest(b->m, d, db) != IVS_OK ||
        memcmp(da, db, 32u) != 0) {
      return 0;
    }
  }
  return 1;
}

static int sl_seam_equals_run(void)
{
  enum { UNITS = 300, CAP = 4 * 1200 };
  ivs_ms_inst a;
  ivs_ms_inst b;
  ivs_psg_sink k;
  ivs_run_report rep;
  uint8_t *ba = malloc(CAP);
  uint8_t *bb = malloc(15060u);
  uint32_t i;
  int rc = IVS_TEST_FAIL;
  a.storage = NULL;
  b.storage = NULL;
  a.m = NULL;
  b.m = NULL;
  SL_CHECK(ba != NULL && bb != NULL);
  SL_CHECK(ivs_ms_powered_prog(&a, IVS_MS_PROG_TONE, 0x5Au));
  SL_CHECK(ivs_ms_powered_prog(&b, IVS_MS_PROG_TONE, 0x5Au));
  ivs_psg_sink_init(&k, ba, CAP);
  for (i = 0u; i < UNITS; i++) {
    ivs_cp1610_result r;
    SL_CHECK(ivs_machine_step_unit_sink_internal(a.m, &r, &k) == IVS_OK);
  }
  SL_CHECK(pu_run_to(b.m, pu_now(a.m), bb, &rep));
  SL_CHECK(pu_now(b.m) == pu_now(a.m));
  SL_CHECK(rep.audio_bytes_written == k.written && memcmp(ba, bb, (size_t)k.written) == 0);
  SL_CHECK(sl_same(&a, &b));
  rc = IVS_TEST_PASS;
done:
  free(ba);
  free(bb);
  if (a.storage != NULL) {
    ivs_ms_free(&a);
  }
  if (b.storage != NULL) {
    ivs_ms_free(&b);
  }
  return rc;
}

static int sl_samples_split(void)
{
  static const uint64_t pats[4][3] = {{1u, 0u, 0u}, {7u, 0u, 0u}, {13u, 0u, 0u}, {1u, 7u, 13u}};
  static const uint32_t npat[4] = {1u, 1u, 1u, 3u};
  ivs_ms_inst one;
  uint8_t *ref = malloc(SL_BYTES);
  uint64_t ref_n = 0u;
  uint32_t p;
  int rc = IVS_TEST_FAIL;
  one.storage = NULL;
  one.m = NULL;
  SL_CHECK(ref != NULL);
  SL_CHECK(ivs_ms_powered_prog(&one, IVS_MS_PROG_TONE, 0x5Au));
  SL_CHECK(pu_run_sliced(one.m, SL_TARGET, NULL, 0u, ref, SL_BYTES, &ref_n));
  SL_CHECK(ref_n > 4u * 4300u);
  for (p = 0u; p < 4u; p++) {
    ivs_ms_inst x;
    uint8_t *out = malloc(SL_BYTES);
    uint64_t n = 0u;
    int ok;
    x.storage = NULL;
    x.m = NULL;
    SL_CHECK(out != NULL);
    SL_CHECK(ivs_ms_powered_prog(&x, IVS_MS_PROG_TONE, 0x5Au));
    ok = pu_run_sliced(x.m, SL_TARGET, pats[p], npat[p], out, SL_BYTES, &n);
    if (!ok || n != ref_n || memcmp(out, ref, (size_t)n) != 0 || !sl_same(&x, &one)) {
      ivs_test_diag("slice pattern %u differs from one call (ok %d, %llu bytes, want %llu)", (unsigned)p, ok,
                    (unsigned long long)n, (unsigned long long)ref_n);
      free(out);
      ivs_ms_free(&x);
      goto done;
    }
    free(out);
    ivs_ms_free(&x);
  }
  rc = IVS_TEST_PASS;
done:
  free(ref);
  if (one.storage != NULL) {
    ivs_ms_free(&one);
  }
  return rc;
}

static int sl_samples_slice_1(void)
{
  enum { UNITS = 200, CAP = 4 * 700 };
  ivs_ms_inst a;
  ivs_ms_inst b;
  ivs_psg_sink k;
  uint8_t *ba = malloc(CAP);
  uint8_t *bb = malloc(15060u);
  uint64_t got = 0u;
  uint32_t i;
  int rc = IVS_TEST_FAIL;
  a.storage = NULL;
  b.storage = NULL;
  a.m = NULL;
  b.m = NULL;
  SL_CHECK(ba != NULL && bb != NULL);
  SL_CHECK(ivs_ms_powered_prog(&a, IVS_MS_PROG_TONE, 0x5Au));
  SL_CHECK(ivs_ms_powered_prog(&b, IVS_MS_PROG_TONE, 0x5Au));
  ivs_psg_sink_init(&k, ba, CAP);
  for (i = 0u; i < UNITS; i++) {
    ivs_cp1610_result r;
    ivs_run_report rep;
    SL_CHECK(ivs_machine_step_unit_sink_internal(a.m, &r, &k) == IVS_OK);
    SL_CHECK(pu_run_to(b.m, pu_now(b.m) + 1u, bb, &rep));
    SL_CHECK(rep.units_executed == 1u && rep.audio_first_index == got / 4u);
    SL_CHECK(got + rep.audio_bytes_written == k.written);
    SL_CHECK(memcmp(ba + got, bb, (size_t)rep.audio_bytes_written) == 0);
    got += rep.audio_bytes_written;
    SL_CHECK(memcmp(a.storage, b.storage, IVS_MACHINE_BYTES) == 0);
  }
  rc = IVS_TEST_PASS;
done:
  free(ba);
  free(bb);
  if (a.storage != NULL) {
    ivs_ms_free(&a);
  }
  if (b.storage != NULL) {
    ivs_ms_free(&b);
  }
  return rc;
}

static int sl_tone_write_mid_slice(void)
{
  static const uint64_t one[1] = {1u};
  ivs_ms_inst a;
  ivs_ms_inst b;
  uint8_t *ra = malloc(SL_BYTES);
  uint8_t *rb = malloc(SL_BYTES);
  uint64_t na = 0u;
  uint64_t nb = 0u;
  uint64_t head = 0u;
  int rc = IVS_TEST_FAIL;
  a.storage = NULL;
  b.storage = NULL;
  a.m = NULL;
  b.m = NULL;
  SL_CHECK(ra != NULL && rb != NULL);
  SL_CHECK(ivs_ms_powered_prog(&a, IVS_MS_PROG_TONE, 0x5Au));
  SL_CHECK(ivs_ms_powered_prog(&b, IVS_MS_PROG_TONE, 0x5Au));
  SL_CHECK(pu_run_sliced(a.m, SL_TARGET, NULL, 0u, ra, SL_BYTES, &na));
  /* single-tick calls through the first 400 ticks, then one call: the register writes of the guest land inside */
  SL_CHECK(pu_run_sliced(b.m, 400u, one, 1u, rb, SL_BYTES, &head));
  {
    ivs_run_report rep;
    uint8_t *scratch = malloc(15060u * 3u);
    SL_CHECK(scratch != NULL);
    {
      ivs_run_request q;
      memset(&q, 0, sizeof q);
      q.struct_size = (uint32_t)sizeof q;
      q.target_ticks = SL_TARGET;
      q.audio = scratch;
      q.audio_capacity = pu_need(b.m, pu_now(b.m), SL_TARGET);
      memset(&rep, 0, sizeof rep);
      rep.struct_size = (uint32_t)sizeof rep;
      if (ivs_machine_run(b.m, &q, &rep) != IVS_OK || rep.audio_first_index != head / 4u) {
        free(scratch);
        goto done;
      }
      memcpy(rb + head, scratch, (size_t)rep.audio_bytes_written);
      nb = head + rep.audio_bytes_written;
    }
    free(scratch);
  }
  SL_CHECK(na == nb && memcmp(ra, rb, (size_t)na) == 0 && sl_same(&a, &b));
  rc = IVS_TEST_PASS;
done:
  free(ra);
  free(rb);
  if (a.storage != NULL) {
    ivs_ms_free(&a);
  }
  if (b.storage != NULL) {
    ivs_ms_free(&b);
  }
  return rc;
}

/* Boundaries of a reference instance's first units (machine time after each seam unit). */
static int sl_boundaries(uint64_t *b, uint32_t n)
{
  ivs_ms_inst x;
  uint32_t i;
  int ok = 1;
  if (!ivs_ms_powered_prog(&x, IVS_MS_PROG_TONE, 0x5Au)) {
    return 0;
  }
  b[0] = 0u;
  for (i = 1u; i < n; i++) {
    if (!ivs_ms_step(x.m, 1u, NULL)) {
      ok = 0;
      break;
    }
    b[i] = pu_now(x.m);
  }
  ivs_ms_free(&x);
  return ok;
}

static int sl_overshoot(void)
{
  enum { N = 80 };
  uint64_t b[N];
  ivs_ms_inst x;
  uint8_t *buf = malloc(15060u);
  uint32_t i;
  uint32_t pick = 0u;
  int rc = IVS_TEST_FAIL;
  x.storage = NULL;
  x.m = NULL;
  SL_CHECK(buf != NULL && sl_boundaries(b, N));
  /* a unit whose end is a multiple of 16: a call aimed one tick before it still delivers the sample at its end */
  for (i = 2u; i < N; i++) {
    if (b[i] % 16u == 0u && b[i] > b[i - 1u] + 1u) {
      pick = i;
      break;
    }
  }
  SL_CHECK(pick != 0u);
  SL_CHECK(ivs_ms_powered_prog(&x, IVS_MS_PROG_TONE, 0x5Au));
  SL_CHECK(ivs_ms_step(x.m, pick - 1u, NULL));
  SL_CHECK(pu_now(x.m) == b[pick - 1u]);
  {
    ivs_run_report rep;
    SL_CHECK(pu_run_to(x.m, b[pick] - 1u, buf, &rep));
    SL_CHECK(pu_now(x.m) == b[pick] && rep.overshoot_ticks == 1u && rep.units_executed == 1u);
    /* delivered: the samples due up to the end of the unit, not up to the target one tick short of it */
    SL_CHECK(rep.audio_bytes_written == 4u * (b[pick] / 16u - b[pick - 1u] / 16u));
    SL_CHECK(rep.audio_bytes_written > 4u * ((b[pick] - 1u) / 16u - b[pick - 1u] / 16u));
  }
  rc = IVS_TEST_PASS;
done:
  free(buf);
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  return rc;
}

static int sl_no_catch_up_to_target(void)
{
  static const uint64_t targets[6] = {1u, 5u, 29u, 47u, 100u, 333u};
  ivs_ms_inst x;
  uint8_t *buf = malloc(15060u);
  uint32_t i;
  uint32_t crossed = 0u;
  int rc = IVS_TEST_FAIL;
  x.storage = NULL;
  x.m = NULL;
  SL_CHECK(buf != NULL && ivs_ms_powered_prog(&x, IVS_MS_PROG_TONE, 0x5Au));
  for (i = 0u; i < 6u; i++) {
    ivs_run_report rep;
    uint64_t target = pu_now(x.m) + targets[i];
    uint64_t now;
    SL_CHECK(pu_run_to(x.m, target, buf, &rep));
    now = pu_now(x.m);
    SL_CHECK(((struct ivs_machine *)x.m)->obs.audio.emitted == now / 16u);
    SL_CHECK(((struct ivs_machine *)x.m)->psg.next_sample == now / 16u + 1u);
    if (now / 16u > target / 16u) {
      crossed++;
    }
  }
  SL_CHECK(crossed > 0u); /* at least one call ended past a sample tick that its target had not reached */
  rc = IVS_TEST_PASS;
done:
  free(buf);
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  return rc;
}

static int sl_interleaved(void)
{
  static const uint64_t s700[1] = {700u};
  ivs_ms_inst a;
  ivs_ms_inst b;
  ivs_ms_inst sa;
  ivs_ms_inst sb;
  uint8_t *oa = malloc(SL_BYTES);
  uint8_t *ob = malloc(SL_BYTES);
  uint8_t *ra = malloc(SL_BYTES);
  uint8_t *rb = malloc(SL_BYTES);
  uint8_t *scratch = malloc(15060u * 3u);
  uint64_t ga = 0u;
  uint64_t gb = 0u;
  uint64_t na = 0u;
  uint64_t nb = 0u;
  int rc = IVS_TEST_FAIL;
  a.storage = b.storage = sa.storage = sb.storage = NULL;
  a.m = b.m = sa.m = sb.m = NULL;
  SL_CHECK(oa != NULL && ob != NULL && ra != NULL && rb != NULL && scratch != NULL);
  SL_CHECK(ivs_ms_powered_prog(&a, IVS_MS_PROG_TONE, 0x5Au));
  SL_CHECK(ivs_ms_powered_prog(&b, IVS_MS_PROG_CONST, 0x5Au));
  SL_CHECK(ivs_ms_powered_prog(&sa, IVS_MS_PROG_TONE, 0x5Au));
  SL_CHECK(ivs_ms_powered_prog(&sb, IVS_MS_PROG_CONST, 0x5Au));
  SL_CHECK(pu_run_sliced(sa.m, SL_TARGET, NULL, 0u, ra, SL_BYTES, &na));
  SL_CHECK(pu_run_sliced(sb.m, SL_TARGET, NULL, 0u, rb, SL_BYTES, &nb));
  while (pu_now(a.m) < SL_TARGET || pu_now(b.m) < SL_TARGET) {
    ivs_machine *ms[2];
    uint8_t *outs[2];
    uint64_t *gots[2];
    uint32_t w;
    ms[0] = a.m;
    ms[1] = b.m;
    outs[0] = oa;
    outs[1] = ob;
    gots[0] = &ga;
    gots[1] = &gb;
    for (w = 0u; w < 2u; w++) {
      uint64_t now = pu_now(ms[w]);
      if (now < SL_TARGET) {
        ivs_run_request q;
        ivs_run_report rep;
        uint64_t t = SL_TARGET - now > s700[0] ? now + s700[0] : SL_TARGET;
        memset(&q, 0, sizeof q);
        q.struct_size = (uint32_t)sizeof q;
        q.target_ticks = t;
        q.audio = scratch;
        q.audio_capacity = pu_need(ms[w], now, t);
        memset(&rep, 0, sizeof rep);
        rep.struct_size = (uint32_t)sizeof rep;
        SL_CHECK(ivs_machine_run(ms[w], &q, &rep) == IVS_OK && rep.audio_first_index == *gots[w] / 4u);
        memcpy(outs[w] + *gots[w], scratch, (size_t)rep.audio_bytes_written);
        *gots[w] += rep.audio_bytes_written;
      }
    }
  }
  SL_CHECK(ga == na && gb == nb && memcmp(oa, ra, (size_t)na) == 0 && memcmp(ob, rb, (size_t)nb) == 0);
  SL_CHECK(sl_same(&a, &sa) && sl_same(&b, &sb));
  rc = IVS_TEST_PASS;
done:
  free(oa);
  free(ob);
  free(ra);
  free(rb);
  free(scratch);
  if (a.storage != NULL) {
    ivs_ms_free(&a);
  }
  if (b.storage != NULL) {
    ivs_ms_free(&b);
  }
  if (sa.storage != NULL) {
    ivs_ms_free(&sa);
  }
  if (sb.storage != NULL) {
    ivs_ms_free(&sb);
  }
  return rc;
}

static int sl_reset_between_calls(void)
{
  ivs_ms_inst x;
  ivs_ms_inst fresh;
  uint8_t *r1 = malloc(SL_BYTES);
  uint8_t *r2 = malloc(SL_BYTES);
  uint64_t n1 = 0u;
  uint64_t n2 = 0u;
  int rc = IVS_TEST_FAIL;
  x.storage = fresh.storage = NULL;
  x.m = fresh.m = NULL;
  SL_CHECK(r1 != NULL && r2 != NULL);
  SL_CHECK(ivs_ms_powered_prog(&x, IVS_MS_PROG_TONE, 0x5Au));
  SL_CHECK(ivs_ms_powered_prog(&fresh, IVS_MS_PROG_TONE, 0x5Au));
  SL_CHECK(pu_run_sliced(x.m, SL_TARGET, NULL, 0u, r1, SL_BYTES, &n1));
  SL_CHECK(ivs_machine_reset(x.m, IVS_RESET_POWER_ON) == IVS_OK);
  SL_CHECK(pu_now(x.m) == 0u && ((struct ivs_machine *)x.m)->obs.audio.emitted == 0u);
  /* pu_run_sliced checks audio_first_index == 0 for the first call after the reset */
  SL_CHECK(pu_run_sliced(x.m, SL_TARGET, NULL, 0u, r2, SL_BYTES, &n2));
  SL_CHECK(n1 == n2 && memcmp(r1, r2, (size_t)n1) == 0);
  SL_CHECK(ivs_machine_reset(x.m, IVS_RESET_POWER_ON) == IVS_OK);
  SL_CHECK(sl_same(&x, &fresh));
  rc = IVS_TEST_PASS;
done:
  free(r1);
  free(r2);
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  if (fresh.storage != NULL) {
    ivs_ms_free(&fresh);
  }
  return rc;
}

const ivs_test_case ivs_psg_slice_cases[] = {
    {"psg.slice.seam-equals-run", sl_seam_equals_run, NULL},
    {"psg.slice.samples-split", sl_samples_split, NULL},
    {"psg.slice.samples-slice-1", sl_samples_slice_1, NULL},
    {"psg.slice.tone-write-mid-slice", sl_tone_write_mid_slice, NULL},
    {"psg.slice.overshoot", sl_overshoot, NULL},
    {"psg.slice.no-catch-up-to-target", sl_no_catch_up_to_target, NULL},
    {"psg.slice.interleaved-instances", sl_interleaved, NULL},
    {"psg.slice.reset-between-calls", sl_reset_between_calls, NULL},
};
const size_t ivs_psg_slice_count = sizeof ivs_psg_slice_cases / sizeof ivs_psg_slice_cases[0];
