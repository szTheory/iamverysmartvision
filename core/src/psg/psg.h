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
  uint32_t env_dir;         /* envelope direction flag */
  uint32_t env_held;        /* envelope held flag */
  uint32_t env_cycles;      /* envelope cycle count */
  uint32_t reserved[3];     /* 0 */
} ivs_psg_state;

_Static_assert(sizeof(ivs_psg_state) == 72u, "ivs_psg_state must be 72 bytes (machine.h layout)");

/* Sets s to the power-on state: every member 0 in plan 05-03 (a later plan gives the noise
 * register its seed). */
void ivs_psg_state_reset(ivs_psg_state *s);

/* The register file (plan 05-04, D-08): the 16 guest-visible words regs[0..15] at $01F0..$01FF
 * (the storage lives in ivs_memsys psg_reg). A store keeps value & the write mask of the register;
 * a load gives the stored word & the read mask (never above bit 7); neither has a side effect and
 * there is no mirror. idx at or above IVS_PSG_REG_COUNT stores nothing and loads 0. */
void ivs_psg_reg_store(const ivs_psg_params *p, uint16_t *regs, uint32_t idx, uint32_t value);
uint16_t ivs_psg_reg_load(const ivs_psg_params *p, const uint16_t *regs, uint32_t idx);

#endif
