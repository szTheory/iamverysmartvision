/* SPDX-License-Identifier: MIT */
/* Reference PCM converter, "reference conversion v1, converter repeatability only" (D7 decided as
 * D-21, plan 05-10, AUD-02).
 *
 * WHAT THIS IS. A simple aliasing reference that turns the core's dense PSG level stream into signed
 * 16-bit mono PCM at 44,100 or 48,000 Hz. It is a pure integer module: no I/O, no allocation, no real-number types,
 * no globals, includes only <stdint.h> and <stddef.h>. It is NOT chip truth, NOT a high-fidelity
 * renderer and NOT part of the raw-audio evidence path: no digest, claim or gate row for AUD-02 depends
 * on it, and its tests and its `.refpcm` golden are tier deterministic-repeatability (the converter
 * reproduces itself, nothing more). The levels it turns are post-mixer amplitude indices; the analogue
 * constants are the DAC table below, which only this tool and the tests read (D-11): the core never does.
 *
 * WHERE IT LIVES. tools/pcm is an internal static target, linked only by the runner's operator-side
 * `--audio-wav` and by suite runner, with no install rule. No PCM, resampling, filtering, DAC-table use
 * or WAV code exists in ivs_core or in any installed artifact (the install inventory and the
 * defined-symbol rule of core-symbols.sh prove it). Reversal trigger: an adapter that must produce
 * bit-identical PCM across hosts promotes this module to an installed ivs::refpcm target, still outside
 * the core.
 *
 * ALGORITHM (D-21).
 *   input   the dense stream, 4-byte samples A, B, C, 0 (each level 0..15), P = audio_sample_ticks of
 *           ivs_info. The 0-based sample i is stamped (i + 1) * P (the core's absolute grid, the first
 *           sample is the state after the first chip step) and is held from its stamp: cell i covers master
 *           ticks (i + 1) * P to (i + 2) * P. The first P ticks, before the first stamp, hold level 0 (the
 *           reset state): the converter treats them as one implicit silent cell before sample 0.
 *   mix     x_i = dac[A_i] + dac[B_i] + dac[C_i], unsigned, at most 300,000,000 (units of 1e-8 of full
 *           scale, the profile field psg.dac.levels)
 *   filter  exact rational box filter. The crystal is num / den Hz and the output rate is R, so output
 *           sample j covers master ticks [j * num / (den * R), (j + 1) * num / (den * R)). Time is
 *           counted in units of 1 / (den * R) tick: every window is exactly num units wide, every input
 *           cell is P * den * R units wide and every overlap is an integer, so there is no rounding of time, no
 *           drift and no per-sample accumulator error.
 *             s_j   = sum over the cells i of overlap(i, j) * x_i
 *             out_j = round_half_up(s_j / (num * 10000))    (integer divide: the maximum sum
 *                     300,000,000 maps to 30,000, so nothing clips)
 *   DC      one-pole high-pass y_j = out_j - out_{j-1} + floor(y_{j-1} * 32763 / 32768), the state
 *           (previous out and previous y) carried in the CALLER-OWNED ivs_refpcm_state, so slicing the
 *           conversion at any sample count never changes the output. The division floors toward minus
 *           infinity through a helper (no right shift of a negative value); one consequence is that a
 *           downward step leaves a residual of -1 that the floor never lifts, an accepted artifact.
 *   output  signed 16-bit, saturating (never expected: |y| stays below 30,000 by construction).
 *
 * The DAC table is a `static const` copy of the profile field psg.dac.levels (units of 1e-8 of full
 * scale, level 15 = 100,000,000, level 0 = 0). The test runner.refpcm.dac-table-matches-profile compares
 * it with the profile field read through the plan 05-07 probe generator, so the copy cannot drift. */
#ifndef IVS_REFPCM_H
#define IVS_REFPCM_H

#include <stddef.h>
#include <stdint.h>

#define IVS_REFPCM_RATE_44100 44100u
#define IVS_REFPCM_RATE_48000 48000u
#define IVS_REFPCM_WAV_HEADER_BYTES 44u
#define IVS_REFPCM_DAC_LEVELS 16u
#define IVS_REFPCM_LABEL "reference conversion v1, converter repeatability only"

#define IVS_REFPCM_OK 0
#define IVS_REFPCM_E_ARG (-1)   /* a rate that is not 44100 or 48000, or a parameter outside its range */
#define IVS_REFPCM_E_SPACE (-2) /* out is smaller than ivs_refpcm_out_bound; nothing was consumed */
#define IVS_REFPCM_E_LEVEL (-3) /* a level byte above 15 or a nonzero fourth byte; nothing was consumed */

/* The state of one conversion. Plain data, owned by the caller; fields are internal. */
typedef struct ivs_refpcm_state {
  uint64_t cell_units; /* P * den * R: units of one input cell */
  uint64_t win_units;  /* num: units of one output window */
  uint64_t win_rem;    /* units of the current window not yet covered */
  uint64_t acc;        /* sum of overlap * x over the covered part of the current window */
  uint64_t frames;     /* output frames produced so far */
  uint64_t samples;    /* input samples consumed so far */
  uint64_t lead_units; /* units of the implicit silent cell before sample 0 not yet consumed (P * den * R, then 0) */
  int32_t prev_x;      /* DC block: the previous box output (0..30000) */
  int32_t prev_y;      /* DC block: the previous high-pass output, before saturation */
  uint32_t rate;       /* R */
  uint32_t reserved;   /* always 0 */
} ivs_refpcm_state;

/* Starts a conversion of a stream whose master crystal is crystal_num / crystal_den Hz, one sample every
 * sample_period_ticks master ticks, to rate Hz (44100 or 48000). Ranges: 0 < num <= 2^32, 0 < den <=
 * 2^20, 0 < sample_period_ticks <= 2^16 (so every product below fits 64 bits). Every field of st is
 * written. Returns IVS_REFPCM_OK or IVS_REFPCM_E_ARG (st is then unusable). */
int ivs_refpcm_init(ivs_refpcm_state *st, uint64_t crystal_num, uint64_t crystal_den, uint32_t sample_period_ticks,
                    uint32_t rate);

/* An upper bound of the frames that feeding nsamples input samples can produce from the current state:
 * floor((nsamples + 1) * P * den * R / num) + 1 (the +1 cell is the implicit leading one, counted until it
 * has been consumed). */
size_t ivs_refpcm_out_bound(const ivs_refpcm_state *st, size_t nsamples);

/* Feeds nsamples 4-byte samples (levels A, B, C, 0, so 4 * nsamples bytes) and writes the frames that
 * completed to out as signed 16-bit values; *produced gets their count. out_capacity (in frames) must be
 * at least ivs_refpcm_out_bound(st, nsamples), else IVS_REFPCM_E_SPACE before anything is consumed; a
 * level above 15 or a nonzero fourth byte is IVS_REFPCM_E_LEVEL before anything is consumed. levels may
 * be NULL only when nsamples is 0. */
int ivs_refpcm_feed(ivs_refpcm_state *st, const uint8_t *levels, size_t nsamples, int16_t *out, size_t out_capacity,
                    size_t *produced);

/* Floor division of a by b (b > 0), toward minus infinity, without a right shift of a negative value. */
int64_t ivs_refpcm_floor_div(int64_t a, int64_t b);

/* The DAC level of amplitude index i (0..15), units of 1e-8 of full scale; 0 for i above 15. */
uint32_t ivs_refpcm_dac_level(uint32_t i);

/* The 44-byte RIFF/WAVE header of a mono 16-bit PCM file of nframes frames at rate Hz, written to out:
 * RIFF, 36 + data bytes, WAVE, "fmt " chunk of 16 bytes (format 1, one channel, rate, byte rate 2 * rate,
 * block align 2, 16 bits), "data", 2 * nframes. Returns IVS_REFPCM_OK, or IVS_REFPCM_E_ARG when the data
 * size would not fit the 32-bit field (2 * nframes > 4,294,967,259) or the rate is 0. */
int ivs_refpcm_wav_header(uint8_t out[44], uint32_t rate, uint64_t nframes);

#endif
