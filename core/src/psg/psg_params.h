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
 * ports holds no profile constant. */
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
} ivs_psg_params;

/* Register slots (index = address - $01F0) the engine addresses by role: the mixer register that
 * holds the port direction bits and the two port registers (data sheet R7, R16, R17). */
#define IVS_PSG_SLOT_MIXER 8u
#define IVS_PSG_SLOT_PORT0 14u
#define IVS_PSG_SLOT_PORT1 15u

/* The parameters of registry profile index profile, or NULL for an index at or above the
 * profile count (the registry of ivs_profile_get and this table are generated over the same sorted
 * profile files). */
const ivs_psg_params *ivs_psg_params_get(uint32_t profile);

#endif
