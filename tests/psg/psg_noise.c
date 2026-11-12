/* SPDX-License-Identifier: MIT */
/* Suite psg, family noise (AUD-01, plan 05-07, D-10, rules PSG-R09, PSG-R10, PSG-R07): the noise generator of
 * the PSG, a right-shifting Galois register whose mask, seed, output bit and inversion are the profile field
 * psg.noise.lfsr (tag unknown-default-chosen, C71) shifting every steps_per_np x NP chip steps.
 *
 * Expected values come from the sourced vector file data/psg/vectors/noise.txt (ids psg.noise.*, 12 rows),
 * turned into a C include by cmake/IvsPsgVectors.cmake (which reads neither the profile nor the core), never
 * from the engine. The judge is ivs_pe_eval (psg_tone.c): the shift spacing is read off the register of a run
 * (shifts), the period and the output bits of the register are pure functions of the engine's lfsr step from
 * the profile seed. psg.noise.lfsr-maximal-period is the property guard of the default mask (131,071 shifts
 * back to the seed, proved without any golden) and psg.noise.lfsr-alt-period states that the community
 * candidate mask is a one-field edit with the non-maximal period 114,681.
 * Convention of psg.noise.lfsr-first-bits (second read, docs/psg/second-read-record-noise.md): the committed
 * vector lists the register output AFTER each shift (13 zeros, then the first 1 at shift 14); a reader who
 * lists the output from the seed state sees the seed's 1 first. The committed convention stands.
 *
 * Fixed cases (hand-written ids):
 *   psg.noise.mixer-follows-register  noise only on channel A at fixed level 15 and noise period 1: the sample
 *                                     of step k is 15 times the register output after floor(k / 2) shifts (the
 *                                     seed's own bit before the first shift), the sequence taken from the
 *                                     first-bits vector, so the stream reads the noise bit through the mixer
 *   psg.judge.noise-row-changed       the predicate rejects a shifts vector and a period vector whose expected
 *                                     number was changed (and accepts the originals) */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_test.h"
#include "machine_cases.h"
#include "memmap/memsys.h"
#include "psg/psg.h"
#include "psg_cases.h"

#include "ivs_psg_vectors_noise.inc"

static const ivs_pe *pn_find(const char *id)
{
  size_t i;
  for (i = 0u; i < (size_t)IVS_PV_noise_COUNT; i++) {
    if (strcmp(ivs_pe_noise[i].id, id) == 0) {
      return &ivs_pe_noise[i];
    }
  }
  return NULL;
}

static int pn_mixer_follows_register(void)
{
  enum { STEPS = 80 };
  const ivs_pe *bits = pn_find("psg.noise.lfsr-first-bits");
  const ivs_psg_params *p = NULL;
  ivs_psg_state s;
  ivs_psg_obs obs;
  ivs_psg_sink sink;
  uint16_t regs[IVS_PSG_REG_COUNT];
  uint8_t buf[4 * STEPS];
  uint32_t k;
  uint32_t i;
  if (bits == NULL || bits->n_seq != 40u) {
    return IVS_TEST_FAIL;
  }
  for (i = 0u; i < ivs_profile_count(); i++) {
    if (strcmp(ivs_profile_get(i)->id, IVS_MS_PROFILE) == 0) {
      p = ivs_psg_params_get(i);
    }
  }
  if (p == NULL) {
    return IVS_TEST_FAIL;
  }
  memset(regs, 0, sizeof regs);
  memset(&obs, 0, sizeof obs);
  ivs_psg_state_reset(p, &s);
  ivs_psg_sink_init(&sink, buf, sizeof buf);
  ivs_psg_write(p, &s, regs, &obs, &sink, 0u, 8u, 0x37u);  /* mixer: noise A only (tone off, noise B and C off) */
  ivs_psg_write(p, &s, regs, &obs, &sink, 0u, 9u, 0x01u);  /* noise period 1 */
  ivs_psg_write(p, &s, regs, &obs, &sink, 0u, 11u, 0x0Fu); /* amplitude A fixed 15 */
  ivs_psg_catch_up(p, &s, regs, &obs, (uint64_t)STEPS * 16u, &sink);
  if (sink.samples != (uint64_t)STEPS || sink.written != (uint64_t)sizeof buf) {
    return IVS_TEST_FAIL;
  }
  for (k = 1u; k <= (uint32_t)STEPS; k++) {
    uint32_t shifts = k / 2u;
    uint32_t bit = shifts == 0u ? 1u : (uint32_t)bits->seq[shifts - 1u];
    uint32_t want = bit != 0u ? 15u : 0u;
    if ((uint32_t)buf[4u * (k - 1u)] != want || buf[4u * (k - 1u) + 1u] != 0u || buf[4u * (k - 1u) + 2u] != 0u) {
      ivs_test_diag("step %u: levels %u %u %u, expected %u 0 0", (unsigned)k, (unsigned)buf[4u * (k - 1u)],
                    (unsigned)buf[4u * (k - 1u) + 1u], (unsigned)buf[4u * (k - 1u) + 2u], (unsigned)want);
      return IVS_TEST_FAIL;
    }
  }
  return IVS_TEST_PASS;
}

static int pn_judge_row_changed(void)
{
  const ivs_pe *shifts = pn_find("psg.noise.rate-np1");
  const ivs_pe *period = pn_find("psg.noise.lfsr-maximal-period");
  ivs_pe a;
  ivs_pe b;
  if (shifts == NULL || period == NULL || shifts->kind != IVS_PE_SHIFTS || period->kind != IVS_PE_PERIOD) {
    return IVS_TEST_FAIL;
  }
  if (!ivs_pe_eval(shifts, 0) || !ivs_pe_eval(period, 0)) {
    ivs_test_diag("the predicate rejects a real vector");
    return IVS_TEST_FAIL;
  }
  a = *shifts;
  a.n += 1u;
  b = *period;
  b.n += 1u;
  if (ivs_pe_eval(&a, 1) || ivs_pe_eval(&b, 1)) {
    ivs_test_diag("the predicate accepts a vector with its expected number changed");
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_psg_noise_cases[] = {
    {"psg.noise.mixer-follows-register", pn_mixer_follows_register, NULL},
    {"psg.judge.noise-row-changed", pn_judge_row_changed, NULL},
};
const size_t ivs_psg_noise_count = sizeof ivs_psg_noise_cases / sizeof ivs_psg_noise_cases[0];

size_t ivs_psg_noise_vector_count(void)
{
  return (size_t)IVS_PV_noise_COUNT;
}

const char *ivs_psg_noise_vector_id(size_t i)
{
  return ivs_pe_noise[i].id;
}

int ivs_psg_noise_vector_run(size_t i)
{
  return ivs_pe_eval(&ivs_pe_noise[i], 0) ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_source *ivs_psg_noise_sources(size_t *n)
{
  *n = 1u;
  return &ivs_pv_sources_noise[0];
}
