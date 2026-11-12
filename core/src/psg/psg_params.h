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
 * implemented). Every mask is at most $00FF: only the low byte of a bus word is driven (C64). */
#ifndef IVS_PSG_PARAMS_H
#define IVS_PSG_PARAMS_H

#include <stdint.h>

#define IVS_PSG_REG_COUNT 16u

typedef struct ivs_psg_params {
  uint16_t reg_wmask[IVS_PSG_REG_COUNT]; /* a store keeps value & wmask (index = address - $01F0) */
  uint16_t reg_rmask[IVS_PSG_REG_COUNT]; /* a read gives the store & rmask */
} ivs_psg_params;

/* The parameters of registry profile index profile, or NULL for an index at or above the
 * profile count (the registry of ivs_profile_get and this table are generated over the same sorted
 * profile files). */
const ivs_psg_params *ivs_psg_params_get(uint32_t profile);

#endif
