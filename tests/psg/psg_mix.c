/* SPDX-License-Identifier: MIT */
/* Suite psg, families mix and amp (AUD-01, plan 05-06, D-10, rules PSG-R14..R17): the mixer gate and the
 * effective amplitude index, both pure functions of the engine (ivs_psg_mix_channel, ivs_psg_amp_index).
 * Expected values come from the sourced vector files data/psg/vectors/mix.txt (ids psg.mix.*, 10 rows) and
 * amp.txt (ids psg.amp.*, 6 rows), turned into C includes by cmake/IvsPsgVectors.cmake (which reads neither the
 * profile nor the core), never from the engine. The judge is ivs_pe_eval (psg_tone.c). The kind source rows
 * cite the data sheet (the Enable cells and the 4-bit field, tier 1) or the wiki (the amplitude select, tier 4,
 * C12); the 1980 catalog pages print no amplitude select sense, so those rows keep dependent claims provisional.
 *   psg.judge.mix-row-changed  the predicate rejects a gate vector and an index vector whose expected output
 *                              was changed (and accepts the originals) */
#include <stdint.h>
#include <string.h>

#include "ivs_test.h"
#include "psg_cases.h"

#include "ivs_psg_vectors_amp.inc"
#include "ivs_psg_vectors_mix.inc"

static int pm_judge_row_changed(void)
{
  ivs_pe g;
  ivs_pe a;
  if (ivs_pe_mix[0].kind != IVS_PE_GATE || ivs_pe_amp[0].kind != IVS_PE_INDEX) {
    return IVS_TEST_FAIL;
  }
  if (!ivs_pe_eval(&ivs_pe_mix[0], 0) || !ivs_pe_eval(&ivs_pe_amp[0], 0)) {
    ivs_test_diag("the predicate rejects a real vector");
    return IVS_TEST_FAIL;
  }
  g = ivs_pe_mix[0];
  g.n ^= 1u;
  a = ivs_pe_amp[0];
  a.n ^= 1u;
  if (ivs_pe_eval(&g, 1) || ivs_pe_eval(&a, 1)) {
    ivs_test_diag("the predicate accepts a vector with its expected output changed");
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_psg_mix_cases[] = {
    {"psg.judge.mix-row-changed", pm_judge_row_changed, NULL},
};
const size_t ivs_psg_mix_count = sizeof ivs_psg_mix_cases / sizeof ivs_psg_mix_cases[0];

size_t ivs_psg_mix_vector_count(void)
{
  return (size_t)IVS_PV_mix_COUNT;
}

const char *ivs_psg_mix_vector_id(size_t i)
{
  return ivs_pe_mix[i].id;
}

int ivs_psg_mix_vector_run(size_t i)
{
  return ivs_pe_eval(&ivs_pe_mix[i], 0) ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_source *ivs_psg_mix_sources(size_t *n)
{
  *n = 1u;
  return &ivs_pv_sources_mix[0];
}

/* Family amp: only vectors (its judge self-control is the mixer's, above). */
const ivs_test_case ivs_psg_amp_cases[] = {{"", NULL, NULL}};
const size_t ivs_psg_amp_count = 0u;

size_t ivs_psg_amp_vector_count(void)
{
  return (size_t)IVS_PV_amp_COUNT;
}

const char *ivs_psg_amp_vector_id(size_t i)
{
  return ivs_pe_amp[i].id;
}

int ivs_psg_amp_vector_run(size_t i)
{
  return ivs_pe_eval(&ivs_pe_amp[i], 0) ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_source *ivs_psg_amp_sources(size_t *n)
{
  *n = 1u;
  return &ivs_pv_sources_amp[0];
}
