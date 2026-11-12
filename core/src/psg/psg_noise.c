/* SPDX-License-Identifier: MIT */
/* PSG noise generator (plan 05-07, D-10, AUD-01, rules PSG-R09, PSG-R10, PSG-R07): a right-shifting
 * Galois register whose width-bound parameters (mask, seed, output bit, inversion) are the profile field
 * psg.noise.lfsr (tag unknown-default-chosen, C71: the data sheet prints a noise frequency but no register).
 * The engine form is one line, `if (s & 1) s = (s >> 1) ^ mask; else s >>= 1`, so the community candidate
 * mask is a one-field edit of the profile and the default has the maximal period 131,071 (a property of the
 * polynomial, proved by psg.noise.lfsr-maximal-period without any golden).
 * The register shifts every psg.noise.steps_per_np x NP chip steps (2 NP on the NTSC profile, C69), NP the
 * 5-bit noise period register with a written 0 acting as psg.period_zero.noise (a default by analogy,
 * C12). The convention is policy POL-PSG-02 like the tone: the step counter is empty (0) at reset and is
 * reloaded from the effective period on the step that finds it empty, so the first shift is at step 2 NP.
 * The noise bit gates the noise side of the mixer (psg_mix.c). Everything comes from the parameters or
 * the state; there is no static or global state.
 * Named faults (plan 05-07, suite psg): PSG_NOISE_WRONG_TAP, PSG_NOISE_RATE_USES_TONE_DIVISOR,
 * PSG_NOISE_TAP_HARDCODED. */
#include "common/ivs_prelude.h"

#include "psg/psg.h"

uint32_t ivs_psg_lfsr_next(const ivs_psg_params *p, uint32_t value)
{
  uint32_t mask = p->noise_galois_mask;
#ifdef IVS_FAULT_PSG_NOISE_WRONG_TAP
  mask = 0x14000u; /* named fault: the other candidate mask (period 114,681) while the profile says $12000 */
#endif
#ifdef IVS_FAULT_PSG_NOISE_TAP_HARDCODED
  mask = 0x12000u; /* named fault: the default mask is a constant of the engine, the profile field is ignored */
#endif
  if ((value & 1u) != 0u) {
    return (value >> 1) ^ mask;
  }
  return value >> 1;
}

uint32_t ivs_psg_noise_out(const ivs_psg_params *p, uint32_t lfsr)
{
  return ((lfsr >> p->noise_out_bit) & 1u) ^ ((uint32_t)p->noise_invert & 1u);
}

uint32_t ivs_psg_noise_period(const ivs_psg_params *p, const uint16_t *regs)
{
  uint32_t np = (uint32_t)regs[IVS_PSG_SLOT_NOISE] & 0x1Fu;
  return np == 0u ? p->period_zero_noise : np;
}

void ivs_psg_noise_step(const ivs_psg_params *p, ivs_psg_state *s, const uint16_t *regs)
{
  if (s->noise_counter == 0u) {
    uint32_t reload = p->noise_steps_per_np;
#ifdef IVS_FAULT_PSG_NOISE_RATE_USES_TONE_DIVISOR
    reload = 1u; /* named fault: the shift spacing is NP steps, the tone divisor, instead of 2 NP */
#endif
    s->noise_counter = reload * ivs_psg_noise_period(p, regs);
  }
  s->noise_counter -= 1u;
  if (s->noise_counter == 0u) {
    s->noise_lfsr = ivs_psg_lfsr_next(p, s->noise_lfsr);
  }
}
