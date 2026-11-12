/* SPDX-License-Identifier: MIT */
/* PSG chip step scheduler, catch-up, sample record and sink (plan 05-06, D-09, D-15, D-16, D-18,
 * A-01, A-02): the machine keeps O(1) PSG state and never samples or records on its own; the
 * generator runs lazily and exactly as far as it is asked.
 *
 * Chip step k (k = 1, 2, ...) is due at master tick k * sample_period_ticks (16 on the NTSC profile):
 * an absolute grid from reset. ivs_psg_catch_up performs every step whose tick is at or below the
 * target and has not been performed; the result depends on the ticks and the register writes only,
 * never on how often catch-up is called (slicing invariance). Each step runs the tone counters, the
 * mixer and the amplitude index of each channel, makes the 4-byte sample A, B, C, 0 (the level index
 * of each channel, IVS_OUTPUT_FORMAT_PSG_LEVELS_V1), updates the AUDIO row (previous levels, emitted
 * count and, when the three levels differ from the previous sample's, the level-change chain, see
 * psg.h) and then delivers the sample into the sink if it has a buffer. Everything but the delivery
 * is sink independent: a NULL buffer (the counting sink) changes no machine state.
 * A register write at stamp s first catches up to s: a sample at tick k * 16 sees the writes stamped
 * below k * 16 (step first, then write, C78).
 * Plan 05-07: every step also runs the noise generator (psg_noise.c) and the envelope generator
 * (psg_env.c); the mixer reads the noise bit of the register and the amplitude index the envelope level.
 * No static or global state.
 * Named faults (plan 05-06, suite psg): PSG_STEP_IS_16_CLOCKS, PSG_SAMPLE_PHASE_RESET_PER_CALL. */
#include "common/ivs_prelude.h"

#include "common/ivs_sha256.h"
#include "psg/psg.h"

static void ivs_psg_put_be64(uint8_t *d, uint64_t v)
{
  uint32_t i;
  for (i = 0u; i < 8u; i++) {
    d[i] = (uint8_t)(v >> (56u - 8u * i));
  }
}

/* Appends the level-change record of sample k to the AUDIO chain: the preimage is
 * 'I' 'V' 'S' 'R' || 17 || chain || be64(count) || be32(12) || be64(k) || A || B || C || 0. */
static void ivs_psg_chain_change(ivs_psg_obs *obs, uint64_t k, uint32_t levels)
{
  uint8_t pre[4u + 1u + 32u + 8u + 4u + 12u];
  uint32_t n = 0u;
  pre[n++] = (uint8_t)'I';
  pre[n++] = (uint8_t)'V';
  pre[n++] = (uint8_t)'S';
  pre[n++] = (uint8_t)'R';
  pre[n++] = 17u;
  memcpy(pre + n, obs->chain, 32u);
  n += 32u;
  ivs_psg_put_be64(pre + n, obs->count);
  n += 8u;
  pre[n++] = 0u;
  pre[n++] = 0u;
  pre[n++] = 0u;
  pre[n++] = 12u;
  ivs_psg_put_be64(pre + n, k);
  n += 8u;
  pre[n++] = (uint8_t)(levels & 0xFFu);
  pre[n++] = (uint8_t)((levels >> 8) & 0xFFu);
  pre[n++] = (uint8_t)((levels >> 16) & 0xFFu);
  pre[n++] = 0u;
  ivs_sha256_digest(pre, n, obs->chain);
  obs->count += 1u;
}

/* One chip step: state, AUDIO row, then delivery. */
static void ivs_psg_step(const ivs_psg_params *p, ivs_psg_state *s, const uint16_t *regs, ivs_psg_obs *obs,
                         ivs_psg_sink *sink)
{
  uint64_t k = s->next_sample;
  uint32_t mixer = (uint32_t)regs[IVS_PSG_SLOT_MIXER];
  uint32_t levels = 0u;
  uint32_t ch;
  uint32_t noise;
  ivs_psg_tone_step(p, s, regs);
  ivs_psg_noise_step(p, s, regs);
  ivs_psg_env_step(p, s, regs);
  noise = ivs_psg_noise_out(p, s->noise_lfsr);
  for (ch = 0u; ch < 3u; ch++) {
    uint32_t gate = ivs_psg_mix_channel(p, s->tone_out[ch], noise, mixer, ch);
    uint32_t idx = ivs_psg_amp_index(p, (uint32_t)regs[IVS_PSG_SLOT_AMP + ch], s->env_level);
    uint32_t level = gate != 0u ? idx : 0u;
    levels |= (level & 0xFFu) << (8u * ch);
  }
  if (levels != obs->prev_levels) {
    ivs_psg_chain_change(obs, k, levels);
  }
  obs->prev_levels = levels;
  obs->emitted = k;
  s->next_sample = k + 1u;
  sink->samples += 1u;
  if (sink->buf != NULL) {
    if (sink->cap - sink->written >= IVS_PSG_SAMPLE_BYTES) {
      uint8_t *d = sink->buf + sink->written;
      d[0] = (uint8_t)(levels & 0xFFu);
      d[1] = (uint8_t)((levels >> 8) & 0xFFu);
      d[2] = (uint8_t)((levels >> 16) & 0xFFu);
      d[3] = 0u;
      sink->written += IVS_PSG_SAMPLE_BYTES;
    } else {
      sink->overflow = 1u; /* never expected: the capacity rule of the run call proves the room */
    }
  }
}

void ivs_psg_catch_up(const ivs_psg_params *p, ivs_psg_state *s, const uint16_t *regs, ivs_psg_obs *obs,
                      uint64_t to_tick, ivs_psg_sink *sink)
{
  uint64_t period = p->sample_period_ticks;
#ifdef IVS_FAULT_PSG_STEP_IS_16_CLOCKS
  period *= 2u; /* named fault: a chip step of 16 chip clocks (32 ticks), the glossary mistake of plan 05-01 */
#endif
  if (period == 0u) {
    return;
  }
#ifdef IVS_FAULT_PSG_SAMPLE_PHASE_RESET_PER_CALL
  if (sink->fresh != 0u) {
    /* named fault: the sample grid restarts at each call's first catch-up, skipping the steps due before it */
    sink->fresh = 0u;
    s->next_sample = to_tick / period + 1u;
  }
#endif
  while (s->next_sample <= to_tick / period) {
    ivs_psg_step(p, s, regs, obs, sink);
  }
}

void ivs_psg_write(const ivs_psg_params *p, ivs_psg_state *s, uint16_t *regs, ivs_psg_obs *obs, ivs_psg_sink *sink,
                   uint64_t stamp, uint32_t idx, uint32_t value)
{
  ivs_psg_catch_up(p, s, regs, obs, stamp, sink);
  ivs_psg_reg_store(p, regs, idx, value);
  ivs_psg_after_write(p, s, regs, idx);
}
