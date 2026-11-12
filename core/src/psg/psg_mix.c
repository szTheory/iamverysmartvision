/* SPDX-License-Identifier: MIT */
/* PSG mixer and amplitude (plan 05-06, D-10, AUD-01): two pure functions with no state.
 *
 * Mixer (psg.mixer.bits, polarity 0 = a 0 bit enables the generator): for channel ch the gate is
 *   chan = (tone | tone_disabled) & (noise | noise_disabled)
 * with the disable bits of the mixer register, so a channel with both generators disabled is a
 * constant 1 and a channel with both enabled follows tone AND noise.
 * Amplitude (psg.amp.env_select, community statement, C12): bits 3..0 are the fixed level and the
 * select in bits 5..4 chooses it (0) or the envelope level shifted right by 2, 1 or 0 (select 1, 2,
 * 3); the result is the effective 4-bit level index, with no DAC and no analog constant (D-15).
 * Named faults (plan 05-06, suite psg): PSG_MIXER_SENSE_INVERTED, PSG_AMP_ENV_BIT_IGNORED. */
#include "common/ivs_prelude.h"

#include "psg/psg.h"

uint32_t ivs_psg_mix_channel(const ivs_psg_params *p, uint32_t tone, uint32_t noise, uint32_t mixer, uint32_t ch)
{
  uint32_t tone_dis;
  uint32_t noise_dis;
  if (ch >= 3u) {
    return 0u;
  }
  tone_dis = (mixer >> p->mixer_tone_bit[ch]) & 1u;
  noise_dis = (mixer >> p->mixer_noise_bit[ch]) & 1u;
#ifdef IVS_FAULT_PSG_MIXER_SENSE_INVERTED
  tone_dis ^= 1u; /* named fault: the enable bits are read with the opposite polarity */
  noise_dis ^= 1u;
#endif
  return ((tone & 1u) | tone_dis) & ((noise & 1u) | noise_dis) & 1u;
}

uint32_t ivs_psg_amp_index(const ivs_psg_params *p, uint32_t amp_reg, uint32_t env_level)
{
  uint32_t fixed = amp_reg & (uint32_t)p->amp_fixed_mask;
  uint32_t select = (amp_reg >> p->amp_select_shift) & (uint32_t)p->amp_select_mask;
#ifdef IVS_FAULT_PSG_AMP_ENV_BIT_IGNORED
  select = 0u; /* named fault: the select bits are ignored, the fixed level is always used */
#endif
  if (select == 0u || select > 3u) {
    return fixed & 0x0Fu;
  }
  return (env_level >> p->amp_env_shift[select - 1u]) & 0x0Fu;
}
