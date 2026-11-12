/* SPDX-License-Identifier: MIT */
/* PSG parameters (plan 05-04, D-08, D-12, A-06, Pattern 3): the explicit parameter struct every PSG
 * function takes. Internal; it lives in core/src, which only the core and the tests
 * (ivs_core_internal) see.
 *
 * The values are generated from the profile by cmake/IvsPsgTable.cmake (fail closed) into
 * ivs_psg_params.inc, which only psg_regs.c includes; the core holds no profile constant (D-02).
 * Later plans append their psg.* fields to this struct and to the generator; plan 05-04 carries the
 * register file: the write and read mask of each of the 16 bus words at $01F0..$01FF, from the
 * profile field psg.regmap (psg.mirrors is validated and carries no struct member: only "none" is
 * implemented). Every mask is at most $00FF: only the low byte of a bus word is driven (C64).
 * Plan 05-05 (D-02, D-04) appends the port parameters: the idle byte of a port in input mode
 * (input.line_idle), whether an output port reads its latch (input.port_output_read, "latch" is the
 * only value implemented) and the bit numbers of the mixer register that select the direction of the
 * two ports and the bit value that means output (psg.mixer.port_dir_bits), so the read path of the
 * ports holds no profile constant.
 * Plan 05-06 (D-09, D-10, D-15, D-17) appends the time chain and the tone, mixer and amplitude
 * parameters: the sample period (master ticks per chip step), the tone period-zero value, the write
 * strobe offset, the published maximum unit length, the R7 bit of each channel's tone and noise
 * enable, and the amplitude register fields. Every value is generated from the profile; the
 * engine holds none.
 * Plan 05-07 (D-10, D-11, D-12) appends the noise and envelope parameters: the noise period-zero value,
 * the PSG steps per noise period unit and the Galois register (mask, seed, output bit, inversion), the
 * envelope period-zero value, the steps per envelope period unit, the four bit numbers of the shape
 * register and the restart-on-write flag. The DAC level table (psg.dac.levels) is NOT a member: the core
 * never reads it (D-11, D-21); a test-side generator reads it from the profile. */
#ifndef IVS_PSG_PARAMS_H
#define IVS_PSG_PARAMS_H

#include <stdint.h>

#define IVS_PSG_REG_COUNT 16u

typedef struct ivs_psg_params {
  uint16_t reg_wmask[IVS_PSG_REG_COUNT]; /* a store keeps value & wmask (index = address - $01F0) */
  uint16_t reg_rmask[IVS_PSG_REG_COUNT]; /* a read gives the store & rmask */
  uint8_t port_idle;                     /* low byte of a port in input mode with no pin held low */
  uint8_t port_output_read_latch;        /* 1: a port in output mode reads its latch */
  uint8_t port_dir_bit[2];               /* mixer register bit selecting the direction of port 0, 1 */
  uint8_t port_dir_output;               /* the bit value that makes a port an output */
  uint32_t sample_period_ticks;          /* master ticks per chip step = per audio sample (psg.sample_period_ticks) */
  uint32_t period_zero_tone;             /* the effective tone period of the value 0 (psg.period_zero.tone) */
  uint32_t write_strobe_ticks;           /* master ticks added to a write's bus cycle start (psg.write_strobe_ticks) */
  uint32_t max_unit_ticks;               /* the proven maximum ticks of one CPU unit (psg.max_unit_ticks) */
  uint8_t mixer_tone_bit[3];             /* R7 bit disabling tone of channel A, B, C (psg.mixer.bits) */
  uint8_t mixer_noise_bit[3];            /* R7 bit disabling noise of channel A, B, C */
  uint8_t amp_fixed_mask;                /* mask of the fixed level at bit 0 of an amplitude register: 15 */
  uint8_t amp_select_shift;              /* bit number of the low bit of the 2-bit envelope select */
  uint8_t amp_select_mask;               /* mask of the select field after the shift: 3 */
  uint8_t amp_env_shift[3];              /* right shift of the envelope level for select 1, 2, 3 */
  uint32_t period_zero_noise;            /* the effective noise period of the value 0 (psg.period_zero.noise) */
  uint32_t noise_steps_per_np;           /* PSG steps per noise period unit (psg.noise.steps_per_np) */
  uint32_t noise_galois_mask;            /* right-shifting Galois mask (psg.noise.lfsr), nonzero, within the width */
  uint32_t noise_seed;                   /* the register after a reset (psg.noise.lfsr), nonzero, within the width */
  uint8_t noise_out_bit;                 /* the register bit that is the noise output */
  uint8_t noise_invert;                  /* 1: the noise output is inverted */
  uint32_t period_zero_env;              /* the effective envelope period of the value 0 (psg.period_zero.envelope) */
  uint32_t env_steps_per_ep;             /* PSG steps per envelope period unit (psg.envelope.steps_per_ep) */
  uint8_t env_bit_continue;              /* shape register bit numbers (psg.envelope.shape_bits) */
  uint8_t env_bit_attack;
  uint8_t env_bit_alternate;
  uint8_t env_bit_hold;
  uint8_t env_restart_on_write;          /* 1: a write to the shape register restarts the generator */
} ivs_psg_params;

/* Register slots (index = address - $01F0) the engine addresses by role: the mixer register that
 * holds the port direction bits and the two port registers (data sheet R7, R16, R17). */
#define IVS_PSG_SLOT_TONE_FINE 0u   /* tone A, B, C fine period at slots 0, 1, 2 */
#define IVS_PSG_SLOT_TONE_COARSE 4u /* tone A, B, C coarse period (4 bits) at slots 4, 5, 6 */
#define IVS_PSG_SLOT_AMP 11u        /* amplitude A, B, C at slots 11, 12, 13 */
#define IVS_PSG_SLOT_ENV_FINE 3u    /* envelope period fine and coarse (16 bits) at slots 3 and 7 */
#define IVS_PSG_SLOT_ENV_COARSE 7u
#define IVS_PSG_SLOT_MIXER 8u
#define IVS_PSG_SLOT_NOISE 9u       /* noise period (5 bits) */
#define IVS_PSG_SLOT_ENV_SHAPE 10u  /* envelope shape (4 bits, reads 0) */
#define IVS_PSG_SLOT_PORT0 14u
#define IVS_PSG_SLOT_PORT1 15u

/* The parameters of registry profile index profile, or NULL for an index at or above the
 * profile count (the registry of ivs_profile_get and this table are generated over the same sorted
 * profile files). */
const ivs_psg_params *ivs_psg_params_get(uint32_t profile);

#endif
