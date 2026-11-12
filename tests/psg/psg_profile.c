/* SPDX-License-Identifier: MIT */
/* Suite psg, family profile (AUD-01, plan 05-07, D-07, D-12, V-04, threat T-05-18): every PSG rule that has no
 * primary source is tier-tagged profile data, and each of them is proven to be DATA, not a constant of the engine,
 * by perturbation. A test builds a perturbed copy of the ivs_psg_params of the real profile on the stack (there is
 * no registry seam: ivs_profile_get() returns const static data), runs the engine on the real and on the perturbed
 * copy and requires the predicted change. A fault that hard-codes a value in the engine therefore fails exactly the
 * test of that value (PSG_NOISE_TAP_HARDCODED fails psg.profile.noise-taps).
 *
 * The hand-written expected numbers are derived from the stated rules (shift or state every steps_per x period steps,
 * the first at that step by the policy POL-PSG-02 convention) and from the vector notes (the alternative noise mask
 * $14000 has the period 114,681, noise.txt psg.noise.lfsr-alt-period); none is read from the engine.
 *
 * Ids (the first eight exactly as the plan names them):
 *   psg.profile.noise-taps                     the mask is data: the other candidate gives the period 114,681 (the
 *                                              default 131,071) and a different noise stream
 *   psg.profile.period-zero-tone               period-zero tone 8 instead of 4096: the first edge of a written 0
 *                                              moves from step 4096 to step 8
 *   psg.profile.period-zero-noise              period-zero noise 4 instead of 32: the first shift moves from step 64
 *                                              to step 8
 *   psg.profile.period-zero-envelope           period-zero envelope 3 instead of 65536: the first level change moves
 *                                              from step 131,072 to step 6
 *   psg.profile.clocks-per-step                the sample period doubled (psg.clocks_per_step reaches the engine only
 *                                              through psg.sample_period_ticks, their product with the ticks per PSG
 *                                              clock): the first edge of a tone of period 3 moves from tick 48 to 96
 *   psg.profile.mixer-bits                     tone and noise bit numbers of channel A swapped: the gate of a mixer
 *                                              value changes
 *   psg.profile.env-select                     the envelope shift of select 1 set to 0: index 3 becomes 15
 *   psg.profile.dac-levels-not-read-by-core    two profiles that differ only in psg.dac.levels generate byte-identical
 *                                              core parameters (build-time proof, cmake/IvsPsgDacIndependence.cmake)
 * Added by this plan (the same proof for the remaining tagged values):
 *   psg.profile.steps-per-np-ep                noise steps_per_np 4 and envelope steps_per_ep 1 move the first shift
 *                                              of period 1 from step 2 to step 4 and the first state change from
 *                                              step 2 to step 1
 *   psg.profile.env-shape-bits                 the attack bit moved to bit 0: a shape write of 4 starts at level 15
 *                                              instead of 0 */
#include <stdint.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_test.h"
#include "machine_cases.h"
#include "memmap/memsys.h"
#include "psg/psg.h"
#include "psg_cases.h"

#include "ivs_psg_dac_independence.inc"

#define PP_STEP 16u

typedef struct pp_rig {
  ivs_psg_state s;
  ivs_psg_obs obs;
  ivs_psg_sink sink;
  uint16_t regs[IVS_PSG_REG_COUNT];
  uint8_t buf[4u * 256u];
} pp_rig;

static const ivs_psg_params *pp_params(void)
{
  uint32_t p;
  for (p = 0u; p < ivs_profile_count(); p++) {
    if (strcmp(ivs_profile_get(p)->id, IVS_MS_PROFILE) == 0) {
      return ivs_psg_params_get(p);
    }
  }
  return NULL;
}

/* A fresh PSG on p; with with_buf the sink keeps up to 256 samples, else it only counts. */
static void pp_init(pp_rig *r, const ivs_psg_params *p, int with_buf)
{
  memset(r, 0, sizeof *r);
  ivs_psg_state_reset(p, &r->s);
  if (with_buf != 0) {
    ivs_psg_sink_init(&r->sink, r->buf, sizeof r->buf);
  } else {
    ivs_psg_sink_init(&r->sink, NULL, 0u);
  }
}

static void pp_write(pp_rig *r, const ivs_psg_params *p, uint32_t reg, uint32_t value)
{
  ivs_psg_write(p, &r->s, r->regs, &r->obs, &r->sink, 0u, reg, value);
}

/* The first step at which the named state word changes from its value after the writes, or 0 within max steps. */
static uint32_t pp_first_state_change(pp_rig *r, const ivs_psg_params *p, const uint32_t *word, uint32_t max)
{
  uint32_t before = *word;
  uint32_t k;
  for (k = 1u; k <= max; k++) {
    ivs_psg_catch_up(p, &r->s, r->regs, &r->obs, (uint64_t)k * PP_STEP, &r->sink);
    if (*word != before) {
      return k;
    }
  }
  return 0u;
}

/* The period of the register from the profile seed: shifts until the seed returns (0 if above limit). */
static uint32_t pp_period(const ivs_psg_params *p, uint32_t limit)
{
  uint32_t x = p->noise_seed;
  uint32_t i;
  for (i = 1u; i <= limit; i++) {
    x = ivs_psg_lfsr_next(p, x);
    if (x == p->noise_seed) {
      return i;
    }
  }
  return 0u;
}

/* Noise-only stream of channel A (mixer $37, amplitude $0F, noise period 1) of 100 steps into buf (400 bytes). */
static void pp_noise_stream(const ivs_psg_params *p, uint8_t *out)
{
  pp_rig r;
  pp_init(&r, p, 1);
  pp_write(&r, p, 8u, 0x37u);
  pp_write(&r, p, 11u, 0x0Fu);
  pp_write(&r, p, 9u, 0x01u);
  ivs_psg_catch_up(p, &r.s, r.regs, &r.obs, 100u * PP_STEP, &r.sink);
  memcpy(out, r.buf, 400u);
}

static int pp_noise_taps(void)
{
  const ivs_psg_params *p = pp_params();
  ivs_psg_params q;
  uint8_t a[400];
  uint8_t b[400];
  if (p == NULL) {
    return IVS_TEST_FAIL;
  }
  q = *p;
  q.noise_galois_mask = 0x14000u; /* the other candidate of psg.noise.lfsr (noise.txt psg.noise.lfsr-alt-period) */
  if (pp_period(p, 200000u) != 131071u || pp_period(&q, 200000u) != 114681u) {
    ivs_test_diag("periods %u (profile) and %u (other candidate)", (unsigned)pp_period(p, 200000u),
                  (unsigned)pp_period(&q, 200000u));
    return IVS_TEST_FAIL;
  }
  pp_noise_stream(p, a);
  pp_noise_stream(&q, b);
  return memcmp(a, b, sizeof a) != 0 ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* First step at which channel A (tone enabled, amplitude 15, both periods 0) changes level, within max steps. */
static uint32_t pp_tone_first_edge(const ivs_psg_params *p, uint32_t period_fine, uint32_t max, uint64_t *tick)
{
  pp_rig r;
  uint32_t k;
  pp_init(&r, p, 0);
  pp_write(&r, p, 8u, 0x3Eu);
  pp_write(&r, p, 11u, 0x0Fu);
  pp_write(&r, p, 0u, period_fine);
  for (k = 1u; k <= max; k++) {
    ivs_psg_catch_up(p, &r.s, r.regs, &r.obs, (uint64_t)k * p->sample_period_ticks, &r.sink);
    if (r.s.tone_out[0] != 0u) {
      *tick = (uint64_t)k * p->sample_period_ticks;
      return k;
    }
  }
  return 0u;
}

static int pp_period_zero_tone(void)
{
  const ivs_psg_params *p = pp_params();
  ivs_psg_params q;
  uint64_t t = 0u;
  if (p == NULL) {
    return IVS_TEST_FAIL;
  }
  q = *p;
  q.period_zero_tone = 8u;
  if (pp_tone_first_edge(p, 0u, 5000u, &t) != 4096u || pp_tone_first_edge(&q, 0u, 5000u, &t) != 8u) {
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

static int pp_period_zero_noise(void)
{
  const ivs_psg_params *p = pp_params();
  ivs_psg_params q;
  pp_rig a;
  pp_rig b;
  if (p == NULL) {
    return IVS_TEST_FAIL;
  }
  q = *p;
  q.period_zero_noise = 4u;
  pp_init(&a, p, 0);
  pp_init(&b, &q, 0);
  pp_write(&a, p, 9u, 0u);
  pp_write(&b, &q, 9u, 0u);
  if (pp_first_state_change(&a, p, &a.s.noise_lfsr, 200u) != 64u ||
      pp_first_state_change(&b, &q, &b.s.noise_lfsr, 200u) != 8u) {
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

static int pp_period_zero_envelope(void)
{
  const ivs_psg_params *p = pp_params();
  ivs_psg_params q;
  pp_rig a;
  pp_rig b;
  if (p == NULL) {
    return IVS_TEST_FAIL;
  }
  q = *p;
  q.period_zero_env = 3u;
  pp_init(&a, p, 0);
  pp_init(&b, &q, 0);
  pp_write(&a, p, 10u, 0x08u); /* decay: level 15, a state change is a level change */
  pp_write(&b, &q, 10u, 0x08u);
  if (pp_first_state_change(&a, p, &a.s.env_level, 140000u) != 131072u ||
      pp_first_state_change(&b, &q, &b.s.env_level, 200u) != 6u) {
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

static int pp_clocks_per_step(void)
{
  const ivs_psg_params *p = pp_params();
  ivs_psg_params q;
  uint64_t t_real = 0u;
  uint64_t t_pert = 0u;
  if (p == NULL) {
    return IVS_TEST_FAIL;
  }
  q = *p;
  q.sample_period_ticks = 2u * p->sample_period_ticks;
  if (pp_tone_first_edge(p, 3u, 100u, &t_real) != 3u || pp_tone_first_edge(&q, 3u, 100u, &t_pert) != 3u) {
    return IVS_TEST_FAIL;
  }
  /* 3 x 16 = 48 ticks on the profile, 3 x 32 = 96 with the doubled period (the edge spacing in ticks doubles) */
  return t_real == 3u * (uint64_t)p->sample_period_ticks && t_pert == 2u * t_real ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int pp_mixer_bits(void)
{
  const ivs_psg_params *p = pp_params();
  ivs_psg_params q;
  uint32_t real;
  uint32_t pert;
  if (p == NULL) {
    return IVS_TEST_FAIL;
  }
  q = *p;
  q.mixer_tone_bit[0] = p->mixer_noise_bit[0]; /* tone and noise enable bits of channel A swapped */
  q.mixer_noise_bit[0] = p->mixer_tone_bit[0];
  /* mixer value with only the tone-A bit position of the profile clear: tone A enabled, noise A disabled. A tone
   * of 0 with the noise at 1 gates to 0 on the profile; with the bits swapped noise A is enabled and tone A
   * disabled, so the same inputs gate to 1 */
  real = ivs_psg_mix_channel(p, 0u, 1u, 0x3Eu, 0u);
  pert = ivs_psg_mix_channel(&q, 0u, 1u, 0x3Eu, 0u);
  return real == 0u && pert == 1u ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int pp_env_select(void)
{
  const ivs_psg_params *p = pp_params();
  ivs_psg_params q;
  if (p == NULL) {
    return IVS_TEST_FAIL;
  }
  q = *p;
  q.amp_env_shift[0] = 0u; /* select 1 shifts by 0 instead of 2 */
  /* amplitude $10 is select 1: the envelope level 15 shifted right by the select's shift */
  return ivs_psg_amp_index(p, 0x10u, 15u) == 3u && ivs_psg_amp_index(&q, 0x10u, 15u) == 15u ? IVS_TEST_PASS
                                                                                            : IVS_TEST_FAIL;
}

static int pp_dac_levels_not_read(void)
{
  if (ivs_dac_independent != 1) {
    ivs_test_diag("the core's generated PSG parameters change with psg.dac.levels");
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

static int pp_steps_per_np_ep(void)
{
  const ivs_psg_params *p = pp_params();
  ivs_psg_params q;
  pp_rig a;
  pp_rig b;
  pp_rig c;
  pp_rig d;
  if (p == NULL) {
    return IVS_TEST_FAIL;
  }
  q = *p;
  q.noise_steps_per_np = 4u;
  q.env_steps_per_ep = 1u;
  pp_init(&a, p, 0);
  pp_init(&b, &q, 0);
  pp_write(&a, p, 9u, 1u);
  pp_write(&b, &q, 9u, 1u);
  if (pp_first_state_change(&a, p, &a.s.noise_lfsr, 50u) != 2u ||
      pp_first_state_change(&b, &q, &b.s.noise_lfsr, 50u) != 4u) {
    return IVS_TEST_FAIL;
  }
  pp_init(&c, p, 0);
  pp_init(&d, &q, 0);
  pp_write(&c, p, 3u, 1u);
  pp_write(&d, &q, 3u, 1u);
  pp_write(&c, p, 10u, 0x08u);
  pp_write(&d, &q, 10u, 0x08u);
  if (pp_first_state_change(&c, p, &c.s.env_level, 50u) != 2u ||
      pp_first_state_change(&d, &q, &d.s.env_level, 50u) != 1u) {
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

static int pp_env_shape_bits(void)
{
  const ivs_psg_params *p = pp_params();
  ivs_psg_params q;
  pp_rig a;
  pp_rig b;
  if (p == NULL) {
    return IVS_TEST_FAIL;
  }
  q = *p;
  q.env_bit_attack = p->env_bit_hold; /* the attack role moves to the bit that the profile gives to hold */
  q.env_bit_hold = p->env_bit_attack;
  pp_init(&a, p, 0);
  pp_init(&b, &q, 0);
  pp_write(&a, p, 10u, 0x04u); /* attack bit set on the profile: rising shape, level 0 */
  pp_write(&b, &q, 10u, 0x04u); /* with the roles swapped the same value is a falling shape, level 15 */
  return a.s.env_level == 0u && b.s.env_level == 15u ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_case ivs_psg_profile_cases[] = {
    {"psg.profile.noise-taps", pp_noise_taps, NULL},
    {"psg.profile.period-zero-tone", pp_period_zero_tone, NULL},
    {"psg.profile.period-zero-noise", pp_period_zero_noise, NULL},
    {"psg.profile.period-zero-envelope", pp_period_zero_envelope, NULL},
    {"psg.profile.clocks-per-step", pp_clocks_per_step, NULL},
    {"psg.profile.mixer-bits", pp_mixer_bits, NULL},
    {"psg.profile.env-select", pp_env_select, NULL},
    {"psg.profile.dac-levels-not-read-by-core", pp_dac_levels_not_read, NULL},
    {"psg.profile.steps-per-np-ep", pp_steps_per_np_ep, NULL},
    {"psg.profile.env-shape-bits", pp_env_shape_bits, NULL},
};
const size_t ivs_psg_profile_count = sizeof ivs_psg_profile_cases / sizeof ivs_psg_profile_cases[0];
