/* SPDX-License-Identifier: MIT */
/* PSG tone generators (plan 05-06, D-09, D-10, AUD-01): three down-reload counters, one chip step
 * each (sample_period_ticks master ticks, 8 PSG clocks on the NTSC profile). The convention is
 * policy POL-PSG-02 (C12): a counter is empty (0) at reset and is reloaded from the effective period
 * on the step that finds it empty; per step it counts down by one and, when it reaches zero, its
 * output toggles. Tone half-period is therefore exactly P steps for a period P of at least 1, the
 * first edge falls at step P, and a period write takes effect at the next reload. The counters keep
 * running whether or not a tone is enabled in the mixer; the mixer decides what is heard.
 * The effective period of the value 0 is the profile field psg.period_zero.tone (4096, C12).
 * No static or global state; every value comes from the parameters or the state.
 * Named fault (plan 05-06, suite psg): PSG_TONE_PERIOD_OFF_BY_ONE. */
#include "common/ivs_prelude.h"

#include "psg/psg.h"

uint32_t ivs_psg_tone_period(const ivs_psg_params *p, const uint16_t *regs, uint32_t ch)
{
  uint32_t fine;
  uint32_t coarse;
  uint32_t v;
  if (ch >= 3u) {
    return p->period_zero_tone;
  }
  fine = (uint32_t)regs[IVS_PSG_SLOT_TONE_FINE + ch] & 0xFFu;
  coarse = (uint32_t)regs[IVS_PSG_SLOT_TONE_COARSE + ch] & 0x0Fu;
  v = (coarse << 8) | fine;
  return v == 0u ? p->period_zero_tone : v;
}

void ivs_psg_tone_step(const ivs_psg_params *p, ivs_psg_state *s, const uint16_t *regs)
{
  uint32_t ch;
  for (ch = 0u; ch < 3u; ch++) {
    if (s->tone_counter[ch] == 0u) {
      uint32_t period = ivs_psg_tone_period(p, regs, ch);
#ifdef IVS_FAULT_PSG_TONE_PERIOD_OFF_BY_ONE
      period += 1u; /* named fault: the counter reloads with P + 1, so every half-period is one step long */
#endif
      s->tone_counter[ch] = period;
    }
    s->tone_counter[ch] -= 1u;
    if (s->tone_counter[ch] == 0u) {
      s->tone_out[ch] ^= 1u;
    }
  }
}
