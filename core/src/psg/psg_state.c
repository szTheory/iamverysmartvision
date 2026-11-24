/* SPDX-License-Identifier: MIT */
/* PSG device state storage (plan 05-03) and the power-on reset of the state struct (plan 05-06: the
 * first chip step is step 1, so next_sample starts at 1; plan 05-07, psg.reset: the noise register holds
 * its seed and the envelope is held at level 0 until the first shape write). */
#include "common/ivs_prelude.h"

#include "psg/psg.h"

void ivs_psg_state_reset(const ivs_psg_params *p, ivs_psg_state *s)
{
  memset(s, 0, sizeof *s);
  s->next_sample = 1u;
  if (p != NULL) {
    s->noise_lfsr = p->noise_seed;
  }
  s->env_held = 1u; /* level 0 held until a shape write */
}

/* Number of bits needed to write v (0 for 0). */
static uint32_t ivs_psg_bit_length(uint32_t v)
{
  uint32_t n = 0u;
  while (v != 0u) {
    n++;
    v >>= 1;
  }
  return n;
}

static uint64_t ivs_psg_max_u64(uint64_t a, uint64_t b)
{
  return a > b ? a : b;
}

ivs_status ivs_psg_state_validate(const ivs_psg_params *p, const ivs_psg_state *s)
{
  uint64_t tone_reload_max;
  uint64_t noise_reload_max;
  uint64_t env_reload_max;
  uint32_t mask_bits;
  uint32_t reg_bits;
  uint32_t i;
  if (p == NULL || s == NULL) {
    return IVS_E_BAD_ARG;
  }
  /* The largest reload each counter can take: the effective period is the register value (12, 5 and 16 bits) or the
   * profile's period-zero value for a written 0, and a noise or envelope unit lasts steps-per-unit x period steps. */
  tone_reload_max = ivs_psg_max_u64(0xFFFu, p->period_zero_tone);
  noise_reload_max = (uint64_t)p->noise_steps_per_np * ivs_psg_max_u64(0x1Fu, p->period_zero_noise);
  env_reload_max = (uint64_t)p->env_steps_per_ep * ivs_psg_max_u64(0xFFFFu, p->period_zero_env);
  if (s->next_sample < 1u) {
    return IVS_E_BAD_STATE;
  }
  for (i = 0u; i < 3u; i++) {
    if (s->tone_counter[i] >= tone_reload_max || s->tone_out[i] > 1u || s->reserved[i] != 0u) {
      return IVS_E_BAD_STATE;
    }
  }
  if (s->noise_counter >= noise_reload_max) {
    return IVS_E_BAD_STATE;
  }
  /* The register stays below 2^b for b the bit length of mask | seed (b is at most 32, where every value fits). */
  mask_bits = ivs_psg_bit_length(p->noise_galois_mask);
  reg_bits = ivs_psg_bit_length(p->noise_galois_mask | p->noise_seed);
  if (reg_bits < 32u && (s->noise_lfsr >> reg_bits) != 0u) {
    return IVS_E_BAD_STATE;
  }
  if (p->noise_seed != 0u && ivs_psg_bit_length(p->noise_seed) <= mask_bits && s->noise_lfsr == 0u) {
    return IVS_E_BAD_STATE; /* the lock-up value is unreachable from a nonzero seed inside the mask's width */
  }
  if (s->env_counter >= env_reload_max || s->env_level > 15u || s->env_cycles > 15u || s->env_dir > 1u ||
      s->env_held > 1u) {
    return IVS_E_BAD_STATE;
  }
  if (s->env_held == 0u) {
    if (s->env_level != (s->env_dir != 0u ? s->env_cycles : 15u - s->env_cycles)) {
      return IVS_E_BAD_STATE;
    }
  } else if (s->env_counter != 0u) {
    return IVS_E_BAD_STATE;
  }
  return IVS_OK;
}

void ivs_psg_sink_init(ivs_psg_sink *k, uint8_t *buf, uint64_t cap)
{
  memset(k, 0, sizeof *k);
  k->buf = buf;
  k->cap = buf != NULL ? cap : 0u;
  k->fresh = 1u;
}

void ivs_psg_sink_log(ivs_psg_sink *k, uint8_t *buf, uint64_t cap)
{
  k->log = buf;
  k->log_cap = buf != NULL ? cap : 0u;
}

void ivs_psg_sink_video(ivs_psg_sink *k, uint8_t *buf, uint64_t cap)
{
  k->video = buf;
  k->video_cap = buf != NULL ? cap : 0u;
}
