/* SPDX-License-Identifier: MIT */
/* Suite runner, family refpcm (AUD-02, D7 decided as D-21, plan 05-10): the reference PCM converter of
 * tools/pcm, "reference conversion v1, converter repeatability only". Tier deterministic-repeatability:
 * these cases show that the converter reproduces itself and equals hand-derived numbers; they say nothing
 * about the chip. Test code only.
 *
 * Cases (ids runner.refpcm.* and runner.judge.refpcm-*):
 *   box-exact-vector          the tracer: a four-sample stream with toy rational parameters (crystal
 *                             384,000 Hz over 1, P = 12 ticks, 48,000 Hz output) worked by hand below
 *   silence-zeros             an all-zero stream gives all-zero frames
 *   constant-settles-zero     a constant level 15 on one channel (box output 10,000 once the stream has
 *                             begun) decays through the DC block: frame 0 is 7,854 (the implicit silent
 *                             first cell fills 16 of its 74.57 ticks: 58.57 / 74.57 of 10,000), frame 1 is
 *                             9,998, and the decay (y = floor(y * 32763 / 32768), iterated) reaches 0 at
 *                             frame 8,276 and stays there
 *   floor-division-negative   the helper floors toward minus infinity (a table of hand values)
 *   dc-state-split-invariance two feed calls with the state carried equal one call
 *   frame-split-invariance    every chunk size 1..9 and the cut inside a window equal one call
 *   rate-44100, rate-48000    the crystal 3,579,545 / 1, P = 16, one implicit leading cell: 223,722 samples
 *                             are 223,723 cells = 3,579,568 ticks, so floor(3,579,568 * R / 3,579,545) = R
 *                             frames (the excess is 23 ticks: 0.31 of a frame at 48,000 and 0.28 at
 *                             44,100), and 223,720 samples (223,721 cells = 3,579,536 ticks, 9 short) give
 *                             R - 1
 *   no-clip-max-input         three channels at level 15 give box 30,000 (3e8 / 1e4) once the stream has
 *                             begun (frame 0 is 23,563 and frame 1 is 29,996), never saturated; an
 *                             alternating full-scale stream keeps |y| <= 30,000
 *   dac-table-matches-profile the converter's copy equals the profile field psg.dac.levels (plan 05-07
 *                             probe include)
 *   wav-header                the 44-byte RIFF/WAVE literal of two known (rate, frames) pairs
 *   refusals-consume-nothing  a bad rate, a too small out and a bad level byte refuse before consuming
 *   pitch-within-1pct         a core run of the project stand-in below, converted: the zero-crossing
 *                             frequency is within 1 percent of crystal / (32 * TP)
 *   runner.judge.refpcm-floor-rounding, -dc-state-carry, -rate-drift
 *                             the predicates of box-exact-vector, dc-state-split-invariance and rate-48000
 *                             applied to a deliberately perturbed result must report a failure (a
 *                             self-control passes only when the judge fails): the fault framework hosts
 *                             only the core and runner modules (A-07), so the failure shapes of the
 *                             tools/pcm TU are shown here instead of as named faults
 *
 * Hand derivation of box-exact-vector. The crystal is 384,000 / 1 Hz and the output rate 48,000, so an
 * output window is 384,000 / 48,000 = 8 master ticks (384,000 units of 1 / (den * R) tick, one unit is
 * 1 / 48,000 tick) and an input cell P = 12 ticks is 576,000 units. The stream holds an implicit silent
 * cell before sample 0 (the first P ticks precede the first stamp, D-15: the 0-based sample i is stamped
 * (i + 1) * P and held from there), so the cells are: cell L (silent), then the four samples. Levels
 * (A, B, C), mix x in units of 1e-8 of full scale from the DAC table (level 15 = 100,000,000, level 5 =
 * 3,125,000):
 *   cell L   x = 0                          cell 0 (15, 0, 0)  x0 = 100,000,000   cell 1 (0, 0, 0)  x1 = 0
 *   cell 2 (15, 15, 0) x2 = 200,000,000     cell 3 (5, 0, 0)   x3 = 3,125,000
 * Five cells are 2,880,000 units = 7.5 windows, so seven windows complete (the half window stays pending).
 * out_j = round_half_up(s_j / (num * 10000)) with num * 10000 = 3,840,000,000 and s_j = sum of overlap * x;
 * the cells span (k = thousand units) L [0, 576k), 0 [576k, 1152k), 1 [1152k, 1728k), 2 [1728k, 2304k) and
 * 3 [2304k, 2880k):
 *   W0 [0, 384k)       cell L whole:                               0
 *   W1 [384k, 768k)    192k of cell L, 192k of cell 0: (0 + 1e8) / 2 / 10,000 = 5000
 *   W2 [768k, 1152k)   cell 0 whole:                               100,000,000 / 10,000 = 10000
 *   W3 [1152k, 1536k)  cell 1 whole:                               0
 *   W4 [1536k, 1920k)  192k of cell 1, 192k of cell 2: (0 + 2e8) / 2 / 10,000 = 10000
 *   W5 [1920k, 2304k)  cell 2 whole:                               200,000,000 / 10,000 = 20000
 *   W6 [2304k, 2688k)  cell 3 whole:                               3,125,000 / 10,000 = 312.5 -> 313 (half up; a floor gives 312)
 * DC block y_j = out_j - out_{j-1} + floor(y_{j-1} * 32763 / 32768), y_{-1} = out_{-1} = 0:
 *   y0 = 0
 *   y1 = 5000 - 0 + 0                                                          = 5000
 *   y2 = 10000 - 5000 + floor(5000 * 32763 / 32768 = 4999.25)                  = 5000 + 4999   = 9999
 *   y3 = 0 - 10000 + floor(9999 * 32763 / 32768 = 9997.47)                     = -10000 + 9997 =   -3
 *   y4 = 10000 - 0 + floor(-3 * 32763 / 32768 = -2.9995)                       = 10000 - 3     = 9997  (a truncation gives 9998)
 *   y5 = 20000 - 10000 + floor(9997 * 32763 / 32768 = 9995.46)                 = 10000 + 9995  = 19995
 *   y6 = 313 - 20000 + floor(19995 * 32763 / 32768 = 19991.95)                 = -19687 + 19991 =  304
 * (with a flooring box division W6 is 312 and y6 is 303). */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs/ivs_machine.h"
#include "ivs_refpcm.h"
#include "ivs_test.h"
#include "runner_cases.h"

#include "ivs_psg_probe.inc"

#define RP_CRYSTAL_NUM 3579545u /* the NTSC profile's master crystal in Hz, over 1 */
#define RP_P 16u                /* master ticks per audio sample */
#define RP_CHUNK 512u           /* samples fed per call by the helpers; bound(512) = 110 frames at 48,000 */
#define RP_OUT_CAP 2048u        /* frames of the helper output chunk */

/* The toy stream: four samples of levels A, B, C, 0. */
static const uint8_t rp_toy_levels[16] = {15, 0, 0, 0, 0, 0, 0, 0, 15, 15, 0, 0, 5, 0, 0, 0};
static const int16_t rp_toy_expect[7] = {0, 5000, 9999, -3, 9997, 19995, 304};

/* The predicate of box-exact-vector: the six frames are the hand-derived ones. */
static int rp_vec_ok(const int16_t *got, size_t n)
{
  if (n != 7u) {
    return 0;
  }
  for (size_t j = 0u; j < 7u; j++) {
    if (got[j] != rp_toy_expect[j]) {
      return 0;
    }
  }
  return 1;
}

/* Converts n samples with the toy parameters (or the given ones) in chunks of `chunk` samples, appending
 * every frame to out (cap frames); returns the frame count, or SIZE_MAX on an error. split_break nonzero
 * zeroes the DC block state after the first chunk (the shape of the dc-state-carry control). */
static size_t rp_convert(uint64_t num, uint64_t den, uint32_t p, uint32_t rate, const uint8_t *levels, size_t n,
                         size_t chunk, int split_break, int16_t *out, size_t cap)
{
  ivs_refpcm_state st;
  size_t total = 0u;
  size_t done = 0u;
  int first = 1;
  if (ivs_refpcm_init(&st, num, den, p, rate) != IVS_REFPCM_OK) {
    return SIZE_MAX;
  }
  while (done < n) {
    size_t m = n - done < chunk ? n - done : chunk;
    size_t got = 0u;
    if (cap - total < ivs_refpcm_out_bound(&st, m) ||
        ivs_refpcm_feed(&st, levels + 4u * done, m, out + total, cap - total, &got) != IVS_REFPCM_OK) {
      return SIZE_MAX;
    }
    total += got;
    done += m;
    if (first && split_break) {
      st.prev_x = 0;
      st.prev_y = 0;
    }
    first = 0;
  }
  return total;
}

/* A test-side model of the whole algorithm written another way (one window at a time, the overlaps found
 * by interval arithmetic, no streaming state): round_floor nonzero floors the box division instead of
 * rounding half up. Used to build perturbed results and as a cross-check of the converter. Returns the
 * frames of the complete windows. */
static size_t rp_model(uint64_t num, uint64_t den, uint32_t p, uint32_t rate, const uint8_t *levels, size_t n,
                       int round_floor, int16_t *out, size_t cap)
{
  uint64_t cell = (uint64_t)p * den * rate;
  uint64_t d = num * 10000u;
  uint64_t total = (uint64_t)(n + 1u) * cell; /* the implicit silent cell before sample 0, then n samples */
  int64_t px = 0;
  int64_t py = 0;
  size_t j = 0u;
  while ((uint64_t)(j + 1u) * num <= total && j < cap) {
    uint64_t a = (uint64_t)j * num;
    uint64_t b = a + num;
    uint64_t s = 0u;
    int64_t x;
    int64_t y;
    for (uint64_t i = a / cell; i * cell < b; i++) { /* cell i: 0 is the silent one, i >= 1 is sample i - 1 */
      uint64_t lo = a > i * cell ? a : i * cell;
      uint64_t hi = b < (i + 1u) * cell ? b : (i + 1u) * cell;
      uint64_t mix = 0u;
      if (i >= 1u) {
        mix = (uint64_t)ivs_refpcm_dac_level(levels[4u * (i - 1u)]) + ivs_refpcm_dac_level(levels[4u * (i - 1u) + 1u]) +
              ivs_refpcm_dac_level(levels[4u * (i - 1u) + 2u]);
      }
      s += (hi - lo) * mix;
    }
    x = round_floor ? (int64_t)(s / d) : (int64_t)((2u * s + d) / (2u * d));
    y = x - px + ivs_refpcm_floor_div(py * 32763, 32768);
    px = x;
    py = y;
    out[j++] = (int16_t)y;
  }
  return j;
}

/* A repeatable pseudo-random level stream (a linear congruential generator, levels 0..15 per channel). */
static uint8_t *rp_stream(size_t n, uint32_t seed)
{
  uint8_t *b = malloc(4u * n + 4u);
  uint32_t v = seed;
  if (b == NULL) {
    return NULL;
  }
  for (size_t i = 0u; i < n; i++) {
    for (size_t c = 0u; c < 3u; c++) {
      v = v * 1664525u + 1013904223u;
      b[4u * i + c] = (uint8_t)((v >> 24) & 15u);
    }
    b[4u * i + 3u] = 0u;
  }
  return b;
}

/* ---- the tracer ---- */

static int rp_box_exact_vector(void)
{
  ivs_refpcm_state st;
  int16_t out[8];
  size_t produced = 0u;
  int16_t model[8];
  IVS_CHECK(ivs_refpcm_init(&st, 384000u, 1u, 12u, IVS_REFPCM_RATE_48000) == IVS_REFPCM_OK);
  IVS_CHECK(ivs_refpcm_out_bound(&st, 4u) == 8u); /* (4 + 1) * 576,000 / 384,000 = 7, plus 1 */
  IVS_CHECK(ivs_refpcm_feed(&st, rp_toy_levels, 4u, out, 8u, &produced) == IVS_REFPCM_OK);
  IVS_CHECK(produced == 7u && st.frames == 7u && st.samples == 4u);
  for (size_t j = 0u; j < 7u; j++) {
    if (out[j] != rp_toy_expect[j]) {
      ivs_test_diag("frame %zu: %d, want %d", j, (int)out[j], (int)rp_toy_expect[j]);
      return IVS_TEST_FAIL;
    }
  }
  IVS_CHECK(rp_vec_ok(out, produced));
  /* the second implementation agrees with the hand values (and so with the converter) */
  IVS_CHECK(rp_model(384000u, 1u, 12u, IVS_REFPCM_RATE_48000, rp_toy_levels, 4u, 0, model, 8u) == 7u);
  IVS_CHECK(rp_vec_ok(model, 7u));
  return IVS_TEST_PASS;
}

/* ---- silence, settling, floor division ---- */

static int rp_silence_zeros(void)
{
  enum { N = 8192 };
  uint8_t *z = calloc((size_t)N, 4u);
  int16_t *out = malloc(sizeof(int16_t) * (size_t)N);
  size_t n;
  int ok = 1;
  IVS_CHECK(z != NULL && out != NULL);
  n = rp_convert(RP_CRYSTAL_NUM, 1u, RP_P, IVS_REFPCM_RATE_48000, z, (size_t)N, RP_CHUNK, 0, out, (size_t)N);
  /* 8192 samples = 131,072 ticks = floor(131072 * 48000 / 3579545) = 1757 frames (1757.6) */
  ok = n == 1757u;
  for (size_t j = 0u; ok && j < n; j++) {
    ok = out[j] == 0;
  }
  free(z);
  free(out);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

static int rp_constant_settles_zero(void)
{
  enum { N = 42000 };
  uint8_t *l = calloc((size_t)N, 4u);
  int16_t *out = malloc(sizeof(int16_t) * (size_t)N);
  size_t n;
  int ok;
  IVS_CHECK(l != NULL && out != NULL);
  for (size_t i = 0u; i < (size_t)N; i++) {
    l[4u * i] = 15u;
  }
  n = rp_convert(RP_CRYSTAL_NUM, 1u, RP_P, IVS_REFPCM_RATE_48000, l, (size_t)N, RP_CHUNK, 0, out, (size_t)N);
  /* 42000 samples + the silent first cell = 672,016 ticks = 9011 frames (9011.4); frame 0 is 58.57 / 74.57
   * of the box output 1e8 / 1e4 = 10000 (7854), frame 1 is 10000 - 7854 + floor(7854 * 32763 / 32768) =
   * 2146 + 7852 = 9998 */
  ok = n == 9011u && out[0] == 7854 && out[1] == 9998;
  for (size_t j = 2u; ok && j < n; j++) {
    ok = out[j] <= out[j - 1u] && out[j] >= 0 && (j < 8276u ? out[j] > 0 : out[j] == 0);
  }
  free(l);
  free(out);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

static int rp_floor_division_negative(void)
{
  static const struct {
    int64_t a;
    int64_t b;
    int64_t q;
  } t[] = {
      {7, 2, 3},         {-7, 2, -4},          {1, 2, 0},       {-1, 2, -1},      {0, 5, 0},
      {-6, 3, -2},       {6, 3, 2},            {-1, 32768, -1}, {-32768, 32768, -1}, {-32769, 32768, -2},
      {-98289, 32768, -3}, /* -3 * 32763: the y3 step of the tracer vector */
      {327630000, 32768, 9998}, {32767, 32768, 0}, {-32767, 32768, -1},
  };
  for (size_t i = 0u; i < sizeof t / sizeof t[0]; i++) {
    if (ivs_refpcm_floor_div(t[i].a, t[i].b) != t[i].q) {
      ivs_test_diag("floor(%lld / %lld) = %lld, want %lld", (long long)t[i].a, (long long)t[i].b,
                    (long long)ivs_refpcm_floor_div(t[i].a, t[i].b), (long long)t[i].q);
      return IVS_TEST_FAIL;
    }
  }
  return IVS_TEST_PASS;
}

/* ---- split invariance ---- */

/* The predicate of dc-state-split-invariance: the conversion of n samples cut after `split` samples (two feed
 * calls) equals the conversion in one call. */
static int rp_split_equal(const uint8_t *l, size_t n, size_t split, int break_dc)
{
  int16_t *whole = malloc(sizeof(int16_t) * 4096u);
  int16_t *cut = malloc(sizeof(int16_t) * 4096u);
  size_t nw;
  size_t a;
  size_t b;
  size_t pw;
  int ok = 0;
  ivs_refpcm_state st;
  if (whole == NULL || cut == NULL) {
    free(whole);
    free(cut);
    return 0;
  }
  nw = rp_convert(RP_CRYSTAL_NUM, 1u, RP_P, IVS_REFPCM_RATE_48000, l, n, n, 0, whole, 4096u);
  if (ivs_refpcm_init(&st, RP_CRYSTAL_NUM, 1u, RP_P, IVS_REFPCM_RATE_48000) != IVS_REFPCM_OK) {
    goto done;
  }
  pw = 0u;
  if (ivs_refpcm_feed(&st, l, split, cut, 4096u, &a) != IVS_REFPCM_OK) {
    goto done;
  }
  pw += a;
  if (break_dc) {
    st.prev_x = 0;
    st.prev_y = 0;
  }
  if (ivs_refpcm_feed(&st, l + 4u * split, n - split, cut + pw, 4096u - pw, &b) != IVS_REFPCM_OK) {
    goto done;
  }
  pw += b;
  ok = nw != SIZE_MAX && nw == pw && memcmp(whole, cut, sizeof(int16_t) * nw) == 0;
done:
  free(whole);
  free(cut);
  return ok;
}

static int rp_dc_state_split_invariance(void)
{
  uint8_t *l = rp_stream(3000u, 7u);
  int ok;
  IVS_CHECK(l != NULL);
  ok = rp_split_equal(l, 3000u, 1u, 0) && rp_split_equal(l, 3000u, 1499u, 0) && rp_split_equal(l, 3000u, 2999u, 0);
  free(l);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

static int rp_frame_split_invariance(void)
{
  enum { N = 4000 };
  uint8_t *l = rp_stream((size_t)N, 11u);
  int16_t *whole = malloc(sizeof(int16_t) * 4096u);
  int16_t *cut = malloc(sizeof(int16_t) * 4096u);
  size_t nw;
  int ok = 1;
  IVS_CHECK(l != NULL && whole != NULL && cut != NULL);
  nw = rp_convert(RP_CRYSTAL_NUM, 1u, RP_P, IVS_REFPCM_RATE_48000, l, (size_t)N, (size_t)N, 0, whole, 4096u);
  ok = nw != SIZE_MAX && nw > 0u;
  /* chunk sizes 1..9: most cuts fall inside a window (a window is 4.66 samples wide) */
  for (size_t chunk = 1u; ok && chunk <= 9u; chunk++) {
    size_t nc = rp_convert(RP_CRYSTAL_NUM, 1u, RP_P, IVS_REFPCM_RATE_48000, l, (size_t)N, chunk, 0, cut, 4096u);
    ok = nc == nw && memcmp(whole, cut, sizeof(int16_t) * nw) == 0;
  }
  /* the independent model agrees with the converter on the same stream (so the cuts did not agree on a wrong value) */
  if (ok) {
    size_t nm = rp_model(RP_CRYSTAL_NUM, 1u, RP_P, IVS_REFPCM_RATE_48000, l, (size_t)N, 0, cut, 4096u);
    ok = nm == nw && memcmp(whole, cut, sizeof(int16_t) * nw) == 0;
  }
  free(l);
  free(whole);
  free(cut);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

/* ---- rates ---- */

/* Frames of n silent samples at rate. */
static size_t rp_silent_frames(size_t n, uint32_t rate)
{
  uint8_t *z = calloc(n, 4u);
  int16_t *out = malloc(sizeof(int16_t) * 49152u);
  size_t f = SIZE_MAX;
  if (z != NULL && out != NULL) {
    f = rp_convert(RP_CRYSTAL_NUM, 1u, RP_P, rate, z, n, 1024u, 0, out, 49152u);
  }
  free(z);
  free(out);
  return f;
}

/* The predicate of rate-48000 (and -44100): the frame count of the stream equals the hand-derived one. */
static int rp_rate_ok(size_t frames, size_t want) { return frames == want; }

static int rp_rate_44100(void)
{
  IVS_CHECK(rp_rate_ok(rp_silent_frames(223722u, IVS_REFPCM_RATE_44100), 44100u));
  IVS_CHECK(rp_rate_ok(rp_silent_frames(223720u, IVS_REFPCM_RATE_44100), 44099u));
  return IVS_TEST_PASS;
}

static int rp_rate_48000(void)
{
  IVS_CHECK(rp_rate_ok(rp_silent_frames(223722u, IVS_REFPCM_RATE_48000), 48000u));
  IVS_CHECK(rp_rate_ok(rp_silent_frames(223720u, IVS_REFPCM_RATE_48000), 47999u));
  return IVS_TEST_PASS;
}

/* ---- range ---- */

static int rp_no_clip_max_input(void)
{
  enum { N = 6000 };
  uint8_t *l = calloc((size_t)N, 4u);
  int16_t *out = malloc(sizeof(int16_t) * ((size_t)N + 512u));
  size_t n;
  int ok;
  IVS_CHECK(l != NULL && out != NULL);
  for (size_t i = 0u; i < (size_t)N; i++) {
    l[4u * i] = 15u;
    l[4u * i + 1u] = 15u;
    l[4u * i + 2u] = 15u;
  }
  n = rp_convert(RP_CRYSTAL_NUM, 1u, RP_P, IVS_REFPCM_RATE_48000, l, (size_t)N, RP_CHUNK, 0, out, (size_t)N + 512u);
  /* frame 0 is 58.57 / 74.57 of 30000 = 23563 (the silent first cell), frame 1 is 30000 - 23563 +
   * floor(23563 * 32763 / 32768 = 23559.6) = 29996 */
  ok = n > 2u && out[0] == 23563 && out[1] == 29996;
  for (size_t j = 0u; ok && j < n; j++) {
    ok = out[j] <= 30000 && out[j] >= 0;
  }
  /* full-scale alternation with window = cell (num = P * den * R, so no filtering): every frame 3e8 / 1e4 or 0 */
  for (size_t i = 0u; ok && i < (size_t)N; i++) {
    uint8_t v = (i & 1u) == 0u ? 15u : 0u;
    l[4u * i] = v;
    l[4u * i + 1u] = v;
    l[4u * i + 2u] = v;
  }
  if (ok) {
    n = rp_convert(768000u, 1u, RP_P, IVS_REFPCM_RATE_48000, l, (size_t)N, RP_CHUNK, 0, out, (size_t)N + 512u);
    /* the silent cell, then 15, 0, 15, ...: frame 0 is 0, frame 1 is 30000, frame 2 is 0 - 30000 + 29995 = -5 */
    ok = n == (size_t)N + 1u && out[0] == 0 && out[1] == 30000 && out[2] == -5;
    for (size_t j = 0u; ok && j < n; j++) {
      ok = out[j] <= 30000 && out[j] >= -30000;
    }
  }
  free(l);
  free(out);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

/* ---- the DAC table copy ---- */

static int rp_dac_table_matches_profile(void)
{
  for (uint32_t i = 0u; i < 16u; i++) {
    if (ivs_refpcm_dac_level(i) != ivs_probe_dac_levels[i]) {
      ivs_test_diag("level %u: converter %u, profile %s %u", (unsigned)i, (unsigned)ivs_refpcm_dac_level(i),
                    IVS_PROBE_PROFILE_ID, (unsigned)ivs_probe_dac_levels[i]);
      return IVS_TEST_FAIL;
    }
  }
  IVS_CHECK(ivs_refpcm_dac_level(16u) == 0u);
  IVS_CHECK(ivs_refpcm_dac_level(15u) == 100000000u && ivs_refpcm_dac_level(0u) == 0u);
  return IVS_TEST_PASS;
}

/* ---- the WAV header ---- */

static int rp_wav_header(void)
{
  /* 48,000 frames at 48,000 Hz: data 96,000 = 0x00017700, RIFF size 96,036 = 0x00017724, byte rate 96,000;
   * 48,000 = 0xBB80 */
  static const uint8_t a[44] = {'R', 'I', 'F', 'F', 0x24, 0x77, 0x01, 0x00, 'W', 'A', 'V', 'E', 'f', 'm', 't',
                                ' ', 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x80, 0xBB, 0x00, 0x00,
                                0x00, 0x77, 0x01, 0x00, 0x02, 0x00, 0x10, 0x00, 'd', 'a', 't', 'a', 0x00, 0x77,
                                0x01, 0x00};
  /* 3 frames at 44,100 Hz: data 6, RIFF size 42 = 0x2A, 44,100 = 0xAC44, byte rate 88,200 = 0x00015888 */
  static const uint8_t b[44] = {'R', 'I', 'F', 'F', 0x2A, 0x00, 0x00, 0x00, 'W', 'A', 'V', 'E', 'f', 'm', 't',
                                ' ', 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x44, 0xAC, 0x00, 0x00,
                                0x88, 0x58, 0x01, 0x00, 0x02, 0x00, 0x10, 0x00, 'd', 'a', 't', 'a', 0x06, 0x00,
                                0x00, 0x00};
  uint8_t h[44];
  memset(h, 0xEE, sizeof h);
  IVS_CHECK(ivs_refpcm_wav_header(h, 48000u, 48000u) == IVS_REFPCM_OK && memcmp(h, a, 44u) == 0);
  memset(h, 0xEE, sizeof h);
  IVS_CHECK(ivs_refpcm_wav_header(h, 44100u, 3u) == IVS_REFPCM_OK && memcmp(h, b, 44u) == 0);
  /* a data size that does not fit the 32-bit field, and a zero rate, are refused */
  IVS_CHECK(ivs_refpcm_wav_header(h, 48000u, UINT64_C(2147483648)) == IVS_REFPCM_E_ARG);
  IVS_CHECK(ivs_refpcm_wav_header(h, 0u, 1u) == IVS_REFPCM_E_ARG);
  return IVS_TEST_PASS;
}

/* ---- refusals ---- */

static int rp_refusals_consume_nothing(void)
{
  ivs_refpcm_state st;
  ivs_refpcm_state before;
  uint8_t bad[8] = {1, 2, 3, 0, 16, 0, 0, 0};
  uint8_t bad4[4] = {1, 2, 3, 1};
  int16_t out[8];
  size_t produced = 99u;
  IVS_CHECK(ivs_refpcm_init(&st, RP_CRYSTAL_NUM, 1u, RP_P, 8000u) == IVS_REFPCM_E_ARG);
  IVS_CHECK(ivs_refpcm_init(&st, 0u, 1u, RP_P, 48000u) == IVS_REFPCM_E_ARG);
  IVS_CHECK(ivs_refpcm_init(&st, RP_CRYSTAL_NUM, 0u, RP_P, 48000u) == IVS_REFPCM_E_ARG);
  IVS_CHECK(ivs_refpcm_init(&st, RP_CRYSTAL_NUM, 1u, 0u, 48000u) == IVS_REFPCM_E_ARG);
  IVS_CHECK(ivs_refpcm_init(&st, RP_CRYSTAL_NUM, 1u, RP_P, 48000u) == IVS_REFPCM_OK);
  before = st;
  IVS_CHECK(ivs_refpcm_feed(&st, bad, 2u, out, 8u, &produced) == IVS_REFPCM_E_LEVEL && produced == 0u);
  IVS_CHECK(memcmp(&st, &before, sizeof st) == 0);
  IVS_CHECK(ivs_refpcm_feed(&st, bad4, 1u, out, 8u, &produced) == IVS_REFPCM_E_LEVEL);
  IVS_CHECK(memcmp(&st, &before, sizeof st) == 0);
  /* 2 samples bound to 1 frame; 1000 samples bound to 215: a 100-frame out is too small */
  IVS_CHECK(ivs_refpcm_feed(&st, rp_toy_levels, 1000u, out, 8u, &produced) == IVS_REFPCM_E_SPACE && produced == 0u);
  IVS_CHECK(memcmp(&st, &before, sizeof st) == 0);
  IVS_CHECK(ivs_refpcm_feed(&st, NULL, 0u, out, 0u, &produced) == IVS_REFPCM_OK && produced == 0u);
  /* Review IN-02: at the widest accepted parameters a cell is 2^16 * 2^20 * 48,000 = 375 * 2^43 units, so
   * (nsamples + 1) * cell wraps 64 bits from 5,594 samples on; at nsamples = 2^21 - 1 it wraps to exactly 0
   * (375 * 2^21 * 2^43 = 375 * 2^64), which made the bound 1 frame. The bound must saturate instead, and
   * feed must then refuse before it reads a level byte or writes a frame (levels is too small on purpose). */
  {
    ivs_refpcm_state wide;
    uint8_t one[4] = {0, 0, 0, 0};
    IVS_CHECK(ivs_refpcm_init(&wide, UINT64_C(0x100000000), UINT64_C(0x100000), 0x10000u, 48000u) == IVS_REFPCM_OK);
    IVS_CHECK(ivs_refpcm_out_bound(&wide, 1000u) > 1000u);
    IVS_CHECK(ivs_refpcm_out_bound(&wide, 6000u) == SIZE_MAX);
    IVS_CHECK(ivs_refpcm_out_bound(&wide, ((size_t)1 << 21) - 1u) == SIZE_MAX);
    IVS_CHECK(ivs_refpcm_out_bound(&wide, SIZE_MAX) == SIZE_MAX);
    before = wide;
    produced = 99u;
    IVS_CHECK(ivs_refpcm_feed(&wide, one, ((size_t)1 << 21) - 1u, out, 8u, &produced) == IVS_REFPCM_E_SPACE &&
              produced == 0u);
    IVS_CHECK(memcmp(&wide, &before, sizeof wide) == 0);
  }
  return IVS_TEST_PASS;
}

/* ---- the pitch check through a core run ---- */

/* Project-authored EXEC stand-in (plan 05-10; the encodings of plan 05-06: MVII is 02B8 + destination, MVO
 * Rs, a is 0240 + source, the branch to itself is 0220 0001; the registers are the project's own map): mixer
 * R7 = $3E (tone A enabled, noise A and channels B and C disabled), amplitude A = $0F (fixed level 15), tone A
 * period TP = 291 = $123 (fine $23 at $01F0, coarse $01 at $01F4), then a branch to itself. The period is
 * written late (about tick 300, after the first chip step reloaded the unprogrammed period 0 as 4096), so the
 * first edge arrives only after the 4096-step first reload (65,536 master ticks) and the edges then follow
 * every TP steps: the square wave has the period 2 * TP steps of 16 ticks, a frequency crystal / (32 * TP).
 * The program is a copy of the shape of plan 05-06's TONE program (tests/machine/machine_standin.c); the
 * runner suite links only the public core, so it carries its own. */
static const uint16_t rp_tone_words[] = {0x02B8u, 0x003Eu, 0x0240u, 0x01F8u, 0x02B8u, 0x000Fu, 0x0240u, 0x01FBu,
                                         0x02B8u, 0x0023u, 0x0240u, 0x01F0u, 0x02B8u, 0x0001u, 0x0240u, 0x01F4u,
                                         0x0220u, 0x0001u};
#define RP_TONE_TP 291u
#define RP_PITCH_TICKS UINT64_C(4000000) /* 1.117 emulated seconds: 1.099 s after the first edge */
#define RP_PITCH_FRAMES 60000u

/* Runs the stand-in to RP_PITCH_TICKS in one-frame slices through the public API, converts the dense stream
 * at 48,000 Hz and stores the frames in pcm (RP_PITCH_FRAMES); *frames gets the count. 1 on success. */
static int rp_pitch_pcm(int16_t *pcm, size_t *frames)
{
  ivs_storage_requirements req;
  ivs_create_options co;
  ivs_machine *m = NULL;
  void *raw = NULL;
  uint8_t *base;
  uint8_t *exec = calloc(8192u, 1u);
  uint8_t *grom = calloc(2048u, 1u);
  uint8_t *aud = NULL;
  ivs_info info;
  ivs_refpcm_state st;
  size_t total = 0u;
  int ok = 0;
  if (exec == NULL || grom == NULL) {
    goto done;
  }
  for (size_t i = 0u; i < sizeof rp_tone_words / sizeof rp_tone_words[0]; i++) {
    exec[2u * i] = (uint8_t)(rp_tone_words[i] >> 8);
    exec[2u * i + 1u] = (uint8_t)(rp_tone_words[i] & 0xFFu);
  }
  memset(&req, 0, sizeof req);
  req.struct_size = (uint32_t)sizeof req;
  if (ivs_machine_storage_requirements("intv-2609-ntsc-family-p1", &req) != IVS_OK) {
    goto done;
  }
  raw = malloc((size_t)req.size_bytes + (size_t)req.align_bytes);
  if (raw == NULL) {
    goto done;
  }
  base = (uint8_t *)(((uintptr_t)raw + (uintptr_t)(req.align_bytes - 1u)) & ~(uintptr_t)(req.align_bytes - 1u));
  memset(&co, 0, sizeof co);
  co.struct_size = (uint32_t)sizeof co;
  co.profile_id = "intv-2609-ntsc-family-p1";
  if (ivs_machine_create(&co, base, (size_t)req.size_bytes, &m) != IVS_OK ||
      ivs_machine_load(m, IVS_KIND_EXEC, exec, 8192u, NULL, NULL) != IVS_OK ||
      ivs_machine_load(m, IVS_KIND_GROM, grom, 2048u, NULL, NULL) != IVS_OK ||
      ivs_machine_reset(m, IVS_RESET_POWER_ON) != IVS_OK) {
    goto done;
  }
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info(m, &info) != IVS_OK || info.nominal_frame_ticks == 0u || info.audio_sample_bytes != 4u ||
      ivs_refpcm_init(&st, info.crystal_hz_num, info.crystal_hz_den, info.audio_sample_ticks,
                      IVS_REFPCM_RATE_48000) != IVS_REFPCM_OK) {
    goto done;
  }
  aud = malloc((size_t)info.audio_bytes_per_frame * 2u);
  if (aud == NULL) {
    goto done;
  }
  for (uint64_t now = info.time_ticks; now < RP_PITCH_TICKS;) {
    ivs_run_request q;
    ivs_run_report rep;
    size_t got;
    size_t n;
    size_t off = 0u;
    memset(&q, 0, sizeof q);
    q.struct_size = (uint32_t)sizeof q;
    q.target_ticks = RP_PITCH_TICKS - now > info.nominal_frame_ticks ? now + info.nominal_frame_ticks : RP_PITCH_TICKS;
    q.audio = aud;
    q.audio_capacity = (size_t)info.audio_bytes_per_frame * ((q.target_ticks - now) / info.nominal_frame_ticks + 1u);
    memset(&rep, 0, sizeof rep);
    rep.struct_size = (uint32_t)sizeof rep;
    if (ivs_machine_run(m, &q, &rep) != IVS_OK || rep.stop_reason != IVS_STOP_TARGET) {
      goto done;
    }
    now = rep.now_ticks;
    n = (size_t)rep.audio_bytes_written / 4u;
    while (off < n) {
      size_t m2 = n - off < RP_CHUNK ? n - off : RP_CHUNK;
      if (RP_PITCH_FRAMES - total < ivs_refpcm_out_bound(&st, m2) ||
          ivs_refpcm_feed(&st, aud + 4u * off, m2, pcm + total, RP_PITCH_FRAMES - total, &got) != IVS_REFPCM_OK) {
        goto done;
      }
      total += got;
      off += m2;
    }
  }
  *frames = total;
  ok = 1;
done:
  if (m != NULL) {
    (void)ivs_machine_destroy(m);
  }
  free(raw);
  free(exec);
  free(grom);
  free(aud);
  return ok;
}

/* The predicate of the pitch check: rising zero crossings (y <= 0 then y > 0) between frame f0 and f1 span
 * r - 1 periods; the measured frequency is (r - 1) * R / (f1 - f0) and the target crystal / (32 * TP). Within
 * 1 percent is the integer inequality |(r - 1) * R * 32 * TP - crystal * (f1 - f0)| * 100 <= crystal * (f1 - f0).
 * *err_ppm gets the signed relative error in parts per million (diagnostic only). */
static int rp_pitch_ok(const int16_t *pcm, size_t n, uint64_t crystal, uint64_t tp, int64_t *err_ppm)
{
  size_t first = 0u;
  size_t last = 0u;
  uint64_t r = 0u;
  int64_t lhs;
  int64_t rhs;
  int64_t diff;
  for (size_t j = 1u; j < n; j++) {
    if (pcm[j - 1u] <= 0 && pcm[j] > 0) {
      if (r == 0u) {
        first = j;
      }
      last = j;
      r++;
    }
  }
  if (r < 2u || last <= first || (uint64_t)(last - first) < 48000u) {
    return 0; /* at least one emulated second between the first and the last crossing */
  }
  lhs = (int64_t)((r - 1u) * 48000u * 32u * tp);
  rhs = (int64_t)(crystal * (uint64_t)(last - first));
  diff = lhs > rhs ? lhs - rhs : rhs - lhs;
  *err_ppm = ((lhs - rhs) * 1000000) / rhs;
  return diff * 100 <= rhs;
}

static int rp_pitch_within_1pct(void)
{
  int16_t *pcm = malloc(sizeof(int16_t) * RP_PITCH_FRAMES);
  size_t frames = 0u;
  int64_t err = 0;
  int ok;
  IVS_CHECK(pcm != NULL);
  ok = rp_pitch_pcm(pcm, &frames);
  if (!ok) {
    free(pcm);
    ivs_test_diag("the core run or the conversion failed");
    return IVS_TEST_FAIL;
  }
  ok = rp_pitch_ok(pcm, frames, RP_CRYSTAL_NUM, RP_TONE_TP, &err);
  ivs_test_diag("pitch: %zu frames, error %lld ppm of %u Hz / (32 * %u)", frames, (long long)err, RP_CRYSTAL_NUM,
                (unsigned)RP_TONE_TP);
  free(pcm);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

/* ---- judge self-controls: the same predicates, a deliberately perturbed result, the judge must fail ---- */

static int rp_judge_floor_rounding(void)
{
  int16_t good[8];
  int16_t bad[8];
  /* the model with the rounding the converter uses passes the predicate; the model that floors fails it */
  IVS_CHECK(rp_model(384000u, 1u, 12u, IVS_REFPCM_RATE_48000, rp_toy_levels, 4u, 0, good, 8u) == 7u);
  IVS_CHECK(rp_vec_ok(good, 7u));
  IVS_CHECK(rp_model(384000u, 1u, 12u, IVS_REFPCM_RATE_48000, rp_toy_levels, 4u, 1, bad, 8u) == 7u);
  IVS_CHECK(!rp_vec_ok(bad, 7u));
  return IVS_TEST_PASS;
}

static int rp_judge_dc_state_carry(void)
{
  uint8_t *l = rp_stream(3000u, 7u);
  int a;
  int b;
  IVS_CHECK(l != NULL);
  a = rp_split_equal(l, 3000u, 1499u, 0);
  b = rp_split_equal(l, 3000u, 1499u, 1); /* the DC state is zeroed at the split: the predicate must say unequal */
  free(l);
  IVS_CHECK(a == 1 && b == 0);
  return IVS_TEST_PASS;
}

static int rp_judge_rate_drift(void)
{
  /* a crystal numerator off by 75 Hz (about one output frame per emulated second at 48,000 Hz) */
  uint8_t *z = calloc(223722u, 4u);
  int16_t *out = malloc(sizeof(int16_t) * 49152u);
  size_t drift;
  size_t good;
  IVS_CHECK(z != NULL && out != NULL);
  good = rp_convert(RP_CRYSTAL_NUM, 1u, RP_P, IVS_REFPCM_RATE_48000, z, 223722u, 1024u, 0, out, 49152u);
  drift = rp_convert(RP_CRYSTAL_NUM + 75u, 1u, RP_P, IVS_REFPCM_RATE_48000, z, 223722u, 1024u, 0, out, 49152u);
  free(z);
  free(out);
  IVS_CHECK(rp_rate_ok(good, 48000u) && !rp_rate_ok(drift, 48000u));
  return IVS_TEST_PASS;
}

static const ivs_test_case refpcm_cases[] = {
    {"runner.refpcm.box-exact-vector", rp_box_exact_vector, NULL},
    {"runner.refpcm.silence-zeros", rp_silence_zeros, NULL},
    {"runner.refpcm.constant-settles-zero", rp_constant_settles_zero, NULL},
    {"runner.refpcm.floor-division-negative", rp_floor_division_negative, NULL},
    {"runner.refpcm.dc-state-split-invariance", rp_dc_state_split_invariance, NULL},
    {"runner.refpcm.frame-split-invariance", rp_frame_split_invariance, NULL},
    {"runner.refpcm.rate-44100", rp_rate_44100, NULL},
    {"runner.refpcm.rate-48000", rp_rate_48000, NULL},
    {"runner.refpcm.no-clip-max-input", rp_no_clip_max_input, NULL},
    {"runner.refpcm.dac-table-matches-profile", rp_dac_table_matches_profile, NULL},
    {"runner.refpcm.wav-header", rp_wav_header, NULL},
    {"runner.refpcm.refusals-consume-nothing", rp_refusals_consume_nothing, NULL},
    {"runner.refpcm.pitch-within-1pct", rp_pitch_within_1pct, NULL},
    {"runner.judge.refpcm-floor-rounding", rp_judge_floor_rounding, NULL},
    {"runner.judge.refpcm-dc-state-carry", rp_judge_dc_state_carry, NULL},
    {"runner.judge.refpcm-rate-drift", rp_judge_rate_drift, NULL},
};

#define REFPCM_CASES (sizeof refpcm_cases / sizeof refpcm_cases[0])

size_t runner_refpcm_count(void) { return REFPCM_CASES; }

const char *runner_refpcm_id(size_t i) { return i < REFPCM_CASES ? refpcm_cases[i].id : NULL; }

int runner_refpcm_run(size_t i) { return i < REFPCM_CASES ? refpcm_cases[i].fn() : IVS_TEST_FAIL; }
