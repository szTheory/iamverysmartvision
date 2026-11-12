/* SPDX-License-Identifier: MIT */
/* Suite psg, family dac (AUD-01, plan 05-07, D-11, rule PSG-R19): the DAC level table of the profile
 * (psg.dac.levels, 16 unsigned integers in units of 1e-8 of full scale, tag inference, C12) as DATA THAT THE CORE
 * NEVER READS. The table is not a member of ivs_psg_params and is not in libivs_core.a: the test-side generator
 * cmake/IvsPsgProfileProbe.cmake reads the profile field into a test include and these tests compare it with the
 * vectors of data/psg/vectors/dac.txt (ids psg.dac.*, 8 rows: levels 15..9 are the printed Fig. 9 labels, levels
 * 8..1 the halving policy, level 0 from the text; all kind policy because the reprint pages were not obtained, so
 * they are not an independent source) and with the properties a table must have. A wrong table therefore cannot
 * invalidate a raw audio golden: raw audio is the level indices (D-15).
 *
 * Fixed cases (hand-written ids):
 *   psg.dac.monotonic       the table is non-decreasing in the level, level 0 is its minimum, and three channels at
 *                           the full level are at most 300,000,000 (and so fit uint32_t)
 *   psg.dac.not-in-core     structural: no 32-bit pattern of a nonzero table entry occurs in any row of the core's
 *                           ivs_psg_params (the only profile-to-PSG channel), so the table never enters the core;
 *                           the structural proofs are the core-symbols step and the byte-identical generated
 *                           parameters of psg.profile.dac-levels-not-read-by-core
 *   psg.judge.dac-row-changed the predicate rejects a table row whose expected value was changed (and accepts the
 *                           original) */
#include <stdint.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_test.h"
#include "memmap/memsys.h"
#include "psg/psg.h"
#include "psg_cases.h"

#include "ivs_psg_probe.inc"
#include "ivs_psg_vectors_dac.inc"

#define PD_KEY_COUNT 16u
#define PD_KEY_SUM3 17u

static int pd_row_holds(const ivs_pe *v, int quiet)
{
  uint32_t i;
  if (v->kind != IVS_PE_TABLE || v->n_tab == 0u || v->tkeys == NULL || v->tvals == NULL) {
    return 0;
  }
  for (i = 0u; i < v->n_tab; i++) {
    uint32_t key = v->tkeys[i];
    uint32_t want = v->tvals[i];
    uint32_t got;
    if (key < 16u) {
      got = ivs_probe_dac_levels[key];
    } else if (key == PD_KEY_COUNT) {
      got = (uint32_t)(sizeof ivs_probe_dac_levels / sizeof ivs_probe_dac_levels[0]);
    } else if (key == PD_KEY_SUM3) {
      got = 3u * ivs_probe_dac_levels[15];
    } else {
      return 0;
    }
    if (got != want) {
      if (!quiet) {
        ivs_test_diag("%s: key %u is %u, vector expects %u", v->id, (unsigned)key, (unsigned)got, (unsigned)want);
      }
      return 0;
    }
  }
  return 1;
}

static int pd_monotonic(void)
{
  uint32_t i;
  uint64_t sum3;
  for (i = 1u; i < 16u; i++) {
    if (ivs_probe_dac_levels[i] < ivs_probe_dac_levels[i - 1u]) {
      ivs_test_diag("level %u (%u) is below level %u (%u)", (unsigned)i, (unsigned)ivs_probe_dac_levels[i],
                    (unsigned)(i - 1u), (unsigned)ivs_probe_dac_levels[i - 1u]);
      return IVS_TEST_FAIL;
    }
  }
  for (i = 1u; i < 16u; i++) {
    if (ivs_probe_dac_levels[0] > ivs_probe_dac_levels[i]) {
      return IVS_TEST_FAIL; /* level 0 is the minimum */
    }
  }
  sum3 = 3u * (uint64_t)ivs_probe_dac_levels[15];
  return sum3 <= 300000000u && sum3 <= 0xFFFFFFFFu ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int pd_not_in_core(void)
{
  uint32_t row;
  uint32_t lv;
  for (row = 0u; row < ivs_profile_count(); row++) {
    const ivs_psg_params *p = ivs_psg_params_get(row);
    const uint8_t *b = (const uint8_t *)p;
    size_t at;
    if (p == NULL) {
      return IVS_TEST_FAIL;
    }
    for (lv = 1u; lv < 16u; lv++) {
      uint32_t pat = ivs_probe_dac_levels[lv];
      if (pat == 0u) {
        continue;
      }
      for (at = 0u; at + sizeof pat <= sizeof *p; at++) {
        if (memcmp(b + at, &pat, sizeof pat) == 0) {
          ivs_test_diag("the DAC level %u (%u) occurs in the core's PSG parameters at byte %u", (unsigned)lv,
                        (unsigned)pat, (unsigned)at);
          return IVS_TEST_FAIL;
        }
      }
    }
  }
  return IVS_TEST_PASS;
}

static int pd_judge_row_changed(void)
{
  ivs_pe v;
  uint32_t vals[16];
  if (IVS_PV_dac_COUNT == 0u || ivs_pe_dac[0].n_tab == 0u || ivs_pe_dac[0].n_tab > 16u) {
    return IVS_TEST_FAIL;
  }
  if (!pd_row_holds(&ivs_pe_dac[0], 0)) {
    ivs_test_diag("the predicate rejects the real vector %s", ivs_pe_dac[0].id);
    return IVS_TEST_FAIL;
  }
  memcpy(vals, ivs_pe_dac[0].tvals, ivs_pe_dac[0].n_tab * sizeof vals[0]);
  vals[0] += 1u;
  v = ivs_pe_dac[0];
  v.tvals = vals;
  if (pd_row_holds(&v, 1)) {
    ivs_test_diag("the predicate accepts %s with a changed value", v.id);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_psg_dac_cases[] = {
    {"psg.dac.monotonic", pd_monotonic, NULL},
    {"psg.dac.not-in-core", pd_not_in_core, NULL},
    {"psg.judge.dac-row-changed", pd_judge_row_changed, NULL},
};
const size_t ivs_psg_dac_count = sizeof ivs_psg_dac_cases / sizeof ivs_psg_dac_cases[0];

size_t ivs_psg_dac_vector_count(void)
{
  return (size_t)IVS_PV_dac_COUNT;
}

const char *ivs_psg_dac_vector_id(size_t i)
{
  return ivs_pe_dac[i].id;
}

int ivs_psg_dac_vector_run(size_t i)
{
  return pd_row_holds(&ivs_pe_dac[i], 0) ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_source *ivs_psg_dac_sources(size_t *n)
{
  *n = 1u;
  return &ivs_pv_sources_dac[0];
}
