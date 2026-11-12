/* SPDX-License-Identifier: MIT */
/* PSG device state (plan 05-03, D-16, D-18, A-12): internal. Not installed and not an ABI: it
 * lives in core/src, which only the core and the tests (ivs_core_internal) see.
 *
 * The core keeps O(1) PSG state: counters, output flags, the noise register and the envelope
 * state of the chip, and the index of the next chip step. It never samples or records (D-16).
 * The 16 guest-visible registers and the two port pull-down masks are not here: they live in the
 * memory system (ivs_memsys psg_reg and port_pulldown, D-04, MEM digest rows). Plan 05-03 only
 * stores this state: nothing advances it, so every member is 0 and stays 0 until plan 05-06.
 *
 * Plan 05-06 (D-09, D-10, D-15, D-16, D-18) makes the PSG a time-driven device: one chip step every
 * sample_period_ticks master ticks (16 on the NTSC profile), a step index starting at 1 after a
 * power-on reset (next_sample), tone counters that count down and reload, and one 4-byte sample
 * (levels A, B, C, 0) per step into a caller sink. The previous-sample levels, the emitted count and
 * the level-change chain live in the AUDIO observation row (ivs_psg_obs), updated by every step
 * whether or not a buffer is present.
 *
 * Integer state only: every counter is a uint32_t (the envelope counter reaches 131,072, which
 * does not fit 16 bits) and each operation masks explicitly. The struct is one flat block with
 * no implicit padding: the 64-bit index leads (8-aligned), the 13 state words follow, and 3
 * reserved words (PAD rows of the digest table, always 0) complete 72 bytes, which is what makes
 * the instance a multiple of 16 bytes (machine.h states the derivation). The previous-sample
 * levels and the emitted-sample count are observation rows (obs.audio), not state of this
 * struct. */
#ifndef IVS_PSG_STATE_H
#define IVS_PSG_STATE_H

#include <stdint.h>

#include "psg/psg_params.h"

typedef struct ivs_psg_state {
  uint64_t next_sample;     /* index of the next chip step (sample) to produce */
  uint32_t tone_counter[3]; /* down-reload counters of channels A, B, C */
  uint32_t tone_out[3];     /* tone output flags, 0 or 1 */
  uint32_t noise_counter;   /* noise step counter */
  uint32_t noise_lfsr;      /* noise shift register */
  uint32_t env_counter;     /* envelope step counter */
  uint32_t env_level;       /* envelope level 0..15 */
  uint32_t env_dir;         /* envelope direction flag, 1 rising */
  uint32_t env_held;        /* envelope held flag (1 after a reset until the first shape write) */
  uint32_t env_cycles;      /* index 0..15 of the envelope state inside its 16-state cycle */
  uint32_t reserved[3];     /* 0 */
} ivs_psg_state;

_Static_assert(sizeof(ivs_psg_state) == 72u, "ivs_psg_state must be 72 bytes (machine.h layout)");

/* Sets s to the power-on state (psg.reset, C77): every member 0 except next_sample, which is 1 (the first
 * chip step is step 1, stamped sample_period_ticks), noise_lfsr, which is the seed of psg.noise.lfsr
 * (plan 05-07), and env_held, which is 1 with level 0 (the envelope holds level 0 until the first shape
 * write). A NULL p leaves the noise register 0. */
void ivs_psg_state_reset(const ivs_psg_params *p, ivs_psg_state *s);

/* The register file (plan 05-04, D-08): the 16 guest-visible words regs[0..15] at $01F0..$01FF
 * (the storage lives in ivs_memsys psg_reg). A store keeps value & the write mask of the register;
 * a load gives the stored word & the read mask (never above bit 7); neither has a side effect and
 * there is no mirror. idx at or above IVS_PSG_REG_COUNT stores nothing and loads 0. */
void ivs_psg_reg_store(const ivs_psg_params *p, uint16_t *regs, uint32_t idx, uint32_t value);
uint16_t ivs_psg_reg_load(const ivs_psg_params *p, const uint16_t *regs, uint32_t idx);

/* ---- audio observation row and sink (plan 05-06, D-15, D-16, D-18) ----
 * ivs_psg_obs is the AUDIO row of the instance (digest domain 17), 56 bytes with no implicit
 * padding: chain (SHA-256 over the level-change records, 32 zero bytes after a power-on reset),
 * count (level-change records chained), emitted (samples produced so far, with or without a buffer;
 * the sample index k of a step is its 1-based chip step number, so emitted == next_sample - 1),
 * prev_levels (the levels of the last sample, channel A in bits 7..0, B in 15..8, C in 23..16) and a
 * reserved word (0). The chain is
 *   chain' = SHA-256('I' 'V' 'S' 'R' || 17 || chain || be64(count) || be32(12) || record)
 * with the 12-byte record be64(k) || A || B || C || 0 for the sample k whose three levels differ from
 * the previous sample's (the first sample compares with all-zero levels): it depends on the level
 * changes only, never on how a run is sliced or whether a buffer is present.
 * ivs_psg_sink is the per-call delivery target, a stack struct of the caller: buf NULL is the
 * counting sink (nothing is written, the same code runs), cap the bytes buf holds, written the bytes
 * delivered, samples the samples produced by the call, overflow 1 when a sample did not fit (never
 * expected: the capacity rule of the run call proves it fits). */
typedef struct ivs_psg_obs {
  uint8_t chain[32];
  uint64_t count;
  uint64_t emitted;
  uint32_t prev_levels;
  uint32_t reserved;
} ivs_psg_obs;

_Static_assert(sizeof(ivs_psg_obs) == 56u, "ivs_psg_obs must be 56 bytes (machine.h layout)");

typedef struct ivs_psg_sink {
  uint8_t *buf;
  uint64_t cap;
  uint64_t written;
  uint64_t samples;
  uint32_t overflow;
  uint32_t fresh; /* 1 until the first catch-up of the call (named fault PSG_SAMPLE_PHASE_RESET_PER_CALL) */
} ivs_psg_sink;

/* Bytes of one audio sample record: A, B, C, 0 (IVS_OUTPUT_FORMAT_PSG_LEVELS_V1). */
#define IVS_PSG_SAMPLE_BYTES 4u

/* Sets k to a sink over buf (NULL for the counting sink) holding cap bytes. */
void ivs_psg_sink_init(ivs_psg_sink *k, uint8_t *buf, uint64_t cap);

/* Mixer gate of channel ch (0..2): chan = (tone | tone_disabled) & (noise | noise_disabled) with the
 * disable bits of the mixer register value, so a channel with both generators disabled is a
 * constant 1 (psg.mixer.bits). tone and noise are the 0 or 1 generator outputs. Returns 0 or 1. */
uint32_t ivs_psg_mix_channel(const ivs_psg_params *p, uint32_t tone, uint32_t noise, uint32_t mixer, uint32_t ch);

/* Effective 4-bit level index of an amplitude register value: the fixed level (select 0) or the
 * envelope level shifted right by the select's shift (psg.amp.env_select). env_level is the level of
 * the envelope generator (plan 05-07). */
uint32_t ivs_psg_amp_index(const ivs_psg_params *p, uint32_t amp_reg, uint32_t env_level);

/* The effective tone period of channel ch (0..2): the 12-bit value of its coarse and fine
 * registers, or psg.period_zero.tone for 0. */
uint32_t ivs_psg_tone_period(const ivs_psg_params *p, const uint16_t *regs, uint32_t ch);

/* One chip step of the three tone counters: an empty counter reloads from the effective period,
 * counts down by one and toggles its output when it reaches zero (a period write takes effect at the
 * next reload; the counter runs whether or not the tone is enabled). */
void ivs_psg_tone_step(const ivs_psg_params *p, ivs_psg_state *s, const uint16_t *regs);

/* ---- noise (plan 05-07, D-10, rules PSG-R09, PSG-R10) ----
 * ivs_psg_lfsr_next is the pure right-shifting Galois step of the profile register: if the low bit is set
 * the value shifts right and is XORed with psg.noise.lfsr galois_mask, else it only shifts right (a value
 * within the width stays within it). ivs_psg_noise_out is the noise bit the mixer reads: bit out_bit of
 * the register, XORed with invert. ivs_psg_noise_period is the effective 5-bit noise period (a written 0
 * acts as psg.period_zero.noise). ivs_psg_noise_step is one chip step: an empty counter reloads from
 * steps_per_np x the effective period, counts down by one and shifts the register when it reaches zero
 * (policy POL-PSG-02: the first shift of a period P is at step 2 P). */
uint32_t ivs_psg_lfsr_next(const ivs_psg_params *p, uint32_t value);
uint32_t ivs_psg_noise_out(const ivs_psg_params *p, uint32_t lfsr);
uint32_t ivs_psg_noise_period(const ivs_psg_params *p, const uint16_t *regs);
void ivs_psg_noise_step(const ivs_psg_params *p, ivs_psg_state *s, const uint16_t *regs);

/* ---- envelope (plan 05-07, D-10, rules PSG-R11..R13) ----
 * ivs_psg_env_period is the effective 16-bit envelope period (coarse at $01F7 over fine at $01F3; a
 * written 0 acts as psg.period_zero.envelope). ivs_psg_env_restart restarts the generator for the shape
 * value just written: level = attack ? 0 : 15 with the matching direction, state 0 of 16, held flag
 * cleared, step counter empty (the first state lasts steps_per_ep x EP steps counted from the next step).
 * ivs_psg_env_step is one chip step of the 16-state machine (see psg_env.c); it does nothing while the
 * generator is held. ivs_psg_after_write is called after a register store of slot idx: a write of the
 * shape register restarts the generator when psg.envelope.shape_bits restart_on_write is 1, and no other
 * write has an effect on the generator (a period write never restarts it). */
uint32_t ivs_psg_env_period(const ivs_psg_params *p, const uint16_t *regs);
void ivs_psg_env_restart(const ivs_psg_params *p, ivs_psg_state *s, uint32_t shape);
void ivs_psg_env_step(const ivs_psg_params *p, ivs_psg_state *s, const uint16_t *regs);
void ivs_psg_after_write(const ivs_psg_params *p, ivs_psg_state *s, const uint16_t *regs, uint32_t idx);

/* Performs every chip step k with k * sample_period_ticks <= to_tick that is not yet performed:
 * tones, mixer, amplitude, the sample into the sink and the AUDIO row. The state, the registers
 * and the row change the same whatever the sink is. */
void ivs_psg_catch_up(const ivs_psg_params *p, ivs_psg_state *s, const uint16_t *regs, ivs_psg_obs *obs,
                      uint64_t to_tick, ivs_psg_sink *sink);

/* A register write stamped stamp: catches up to the stamp (a sample at k * sample_period_ticks sees
 * writes stamped below it: step first, then write, C78), then applies the masked store of register
 * idx and ivs_psg_after_write (the envelope restart of plan 05-07). The machine's bus wrapper catches
 * up, lets the memory system store and calls ivs_psg_after_write itself, so a stamp-0 vector write and
 * a guest write cannot diverge. */
void ivs_psg_write(const ivs_psg_params *p, ivs_psg_state *s, uint16_t *regs, ivs_psg_obs *obs, ivs_psg_sink *sink,
                   uint64_t stamp, uint32_t idx, uint32_t value);

#endif
