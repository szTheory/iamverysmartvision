/* SPDX-License-Identifier: MIT */
/* Suite runner, family refpcm (AUD-02, D7 decided as D-21, plan 05-10): the reference PCM converter of
 * tools/pcm, "reference conversion v1, converter repeatability only". Tier deterministic-repeatability:
 * these cases show that the converter reproduces itself and equals hand-derived numbers; they say nothing
 * about the chip. Test code only.
 *
 * Fixed cases:
 *   runner.refpcm.box-exact-vector  the tracer: a four-sample stream with toy rational parameters
 *                                   (crystal 384,000 Hz over 1, P = 12 ticks, 48,000 Hz output) whose box
 *                                   filter and DC block are worked by hand below
 *
 * Hand derivation of box-exact-vector. The crystal is 384,000 / 1 Hz and the output rate 48,000, so an
 * output window is 384,000 / 48,000 = 8 master ticks (384,000 units of 1 / (den * R) tick, one unit is
 * 1 / 48,000 tick) and an input cell P = 12 ticks is 576,000 units. Levels (A, B, C), mix x in units of
 * 1e-8 of full scale from the DAC table (level 15 = 100,000,000, level 5 = 3,125,000):
 *   cell 0 (15, 0, 0)  x0 = 100,000,000    cell 1 (0, 0, 0)   x1 = 0
 *   cell 2 (15, 15, 0) x2 = 200,000,000    cell 3 (5, 0, 0)   x3 = 3,125,000
 * Six windows cover the four cells exactly (4 x 576,000 = 6 x 384,000). out_j = round_half_up(s_j /
 * (num * 10000)) with num * 10000 = 3,840,000,000 and s_j = sum of overlap * x:
 *   W0 [0, 384k)      cell 0 whole:           s/D = 100,000,000 / 10,000          = 10000
 *   W1 [384k, 768k)   192k of cell 0, 192k of cell 1:  (1e8 + 0) / 2 / 10,000    =  5000
 *   W2 [768k, 1152k)  cell 1 whole:           0                                   =     0
 *   W3 [1152k, 1536k) cell 2 whole:           200,000,000 / 10,000                = 20000
 *   W4 [1536k, 1920k) 192k of cell 2, 192k of cell 3: (2e8 + 3,125,000) / 2 / 10,000 = 10156.25 -> 10156
 *   W5 [1920k, 2304k) cell 3 whole:           3,125,000 / 10,000 = 312.5 -> 313 (half up; a floor gives 312)
 * DC block y_j = out_j - out_{j-1} + floor(y_{j-1} * 32763 / 32768), y_{-1} = out_{-1} = 0:
 *   y0 = 10000
 *   y1 = 5000 - 10000 + floor(10000 * 32763 / 32768 = 9998.47) = -5000 + 9998 =  4998
 *   y2 = 0 - 5000 + floor(4998 * 32763 / 32768 = 4997.24)      = -5000 + 4997 =    -3
 *   y3 = 20000 - 0 + floor(-3 * 32763 / 32768 = -2.9995)       = 20000 - 3    = 19997  (a truncation gives 19998)
 *   y4 = 10156 - 20000 + floor(19997 * 32763 / 32768 = 19993.95) = -9844 + 19993 = 10149
 *   y5 = 313 - 10156 + floor(10149 * 32763 / 32768 = 10147.45)   = -9843 + 10147 =   304 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs_refpcm.h"
#include "ivs_test.h"
#include "runner_cases.h"

/* The toy stream: four samples of levels A, B, C, 0. */
static const uint8_t rp_toy_levels[16] = {15, 0, 0, 0, 0, 0, 0, 0, 15, 15, 0, 0, 5, 0, 0, 0};
static const int16_t rp_toy_expect[6] = {10000, 4998, -3, 19997, 10149, 304};

static int rp_box_exact_vector(void)
{
  ivs_refpcm_state st;
  int16_t out[8];
  size_t produced = 0u;
  IVS_CHECK(ivs_refpcm_init(&st, 384000u, 1u, 12u, IVS_REFPCM_RATE_48000) == IVS_REFPCM_OK);
  IVS_CHECK(ivs_refpcm_out_bound(&st, 4u) == 7u); /* 4 * 576,000 / 384,000 = 6, plus 1 */
  IVS_CHECK(ivs_refpcm_feed(&st, rp_toy_levels, 4u, out, 8u, &produced) == IVS_REFPCM_OK);
  IVS_CHECK(produced == 6u && st.frames == 6u && st.samples == 4u);
  for (size_t j = 0u; j < 6u; j++) {
    if (out[j] != rp_toy_expect[j]) {
      ivs_test_diag("frame %zu: %d, want %d", j, (int)out[j], (int)rp_toy_expect[j]);
      return IVS_TEST_FAIL;
    }
  }
  return IVS_TEST_PASS;
}

static const ivs_test_case refpcm_cases[] = {
    {"runner.refpcm.box-exact-vector", rp_box_exact_vector, NULL},
};

#define REFPCM_CASES (sizeof refpcm_cases / sizeof refpcm_cases[0])

size_t runner_refpcm_count(void) { return REFPCM_CASES; }

const char *runner_refpcm_id(size_t i) { return i < REFPCM_CASES ? refpcm_cases[i].id : NULL; }

int runner_refpcm_run(size_t i) { return i < REFPCM_CASES ? refpcm_cases[i].fn() : IVS_TEST_FAIL; }
