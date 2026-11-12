/* SPDX-License-Identifier: MIT */
/* Suite stic, family fuzz (plan 06-08; D-10): eight fixed xorshift32 seeds, each a random slicing (calls of 1 to 3,000 ticks) of 100,000
 * ticks of one of the six display-on interrupt loops, compared with the unit-by-unit reference (run(now + 1) per call): the whole
 * instance storage near every frame event, bus grant and interrupt entry, at every 64th call elsewhere, and at the end. The seeds are
 * literals; no clock, no sleep.
 *   stic.fuzz.seed01 .. seed08   one seed each; the program body is (seed - 1) mod 6
 *   stic.fuzz.non-vacuity        the first seed's span holds a bus grant and an interrupt entry, so the family cannot pass vacuously */
#include <stdint.h>
#include <string.h>

#include "ivs/ivs.h"
#include "machine/machine.h"
#include "machine_cases.h"
#include "stic_cases.h"

#define SZ_TICKS 100000u

static uint32_t sz_events(uint32_t k)
{
  uint16_t w[SC_PROG_MAX + 8u];
  uint32_t ev = 0u;
  static const uint32_t seeds[8] = {0x1D872B41u, 0x2545F491u, 0x3C6EF372u, 0x4A7C15F3u, 0x5BD1E995u, 0x6C078965u, 0x7F4A7C15u, 0x8CB92BA7u};
  size_t n = sc_prog_vbl(w, (k % SC_BODIES) | SC_BODY_BACKTAB);
  if (!ss_compare(w, n, 0u, seeds[k], SZ_TICKS, 64u, 0, &ev)) {
    return 0xFFFFFFFFu;
  }
  return ev;
}

#define SZ_SEED(k) static int sz_seed##k(void) { return sz_events(k - 1u) == 0xFFFFFFFFu ? IVS_TEST_FAIL : IVS_TEST_PASS; }
SZ_SEED(1)
SZ_SEED(2)
SZ_SEED(3)
SZ_SEED(4)
SZ_SEED(5)
SZ_SEED(6)
SZ_SEED(7)
SZ_SEED(8)

static int sz_non_vacuity(void)
{
  uint32_t ev = sz_events(0u);
  if (ev == 0xFFFFFFFFu || (ev & 3u) != 3u) {
    ivs_test_diag("seed 1 saw events %u (a grant and an interrupt entry expected)", (unsigned)ev);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_stic_fuzz_cases[] = {
    {"stic.fuzz.seed01", sz_seed1, NULL}, {"stic.fuzz.seed02", sz_seed2, NULL}, {"stic.fuzz.seed03", sz_seed3, NULL},
    {"stic.fuzz.seed04", sz_seed4, NULL}, {"stic.fuzz.seed05", sz_seed5, NULL}, {"stic.fuzz.seed06", sz_seed6, NULL},
    {"stic.fuzz.seed07", sz_seed7, NULL}, {"stic.fuzz.seed08", sz_seed8, NULL}, {"stic.fuzz.non-vacuity", sz_non_vacuity, NULL},
};
const size_t ivs_stic_fuzz_count = sizeof ivs_stic_fuzz_cases / sizeof ivs_stic_fuzz_cases[0];
