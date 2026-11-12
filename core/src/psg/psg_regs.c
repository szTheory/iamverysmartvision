/* SPDX-License-Identifier: MIT */
/* PSG register file (plan 05-04, D-08, D-12, A-06): the masks of the 16 bus words $01F0..$01FF come
 * from ivs_psg_params.inc, which cmake/IvsPsgTable.cmake generates from the profile fields psg.regmap
 * and psg.mirrors (this file is its only includer); a store keeps value & the write mask and a load
 * gives the stored word & the read mask, with bits 15..8 zero. There is no side effect and no
 * mirror (psg.mirrors is "none"): the CPU sees a memory-mapped model, one logical access per READ,
 * WRITE or DW (BDIR and BC1 are not state).
 * The file holds no static or global state beyond the generated const table (D-07).
 * Named faults (plan 05-04, suite psg): PSG_TONE_HIGH_NIBBLE_UNMASKED, PSG_REG_OCTAL_AS_HEX,
 * PSG_SHAPE_READBACK_STORED. */
#include "common/ivs_prelude.h"

#include "cpu/cp1610.h"
#include "psg/psg.h"

#include "ivs_psg_params.inc"

const ivs_psg_params *ivs_psg_params_get(uint32_t profile)
{
  if (profile >= IVS_PSG_PROFILE_COUNT) {
    return NULL;
  }
  return &ivs_psg_params_table[profile];
}

/* The slot a bus word idx reaches: the identity. */
static uint32_t ivs_psg_slot(uint32_t idx)
{
  uint32_t slot = idx;
#ifdef IVS_FAULT_PSG_REG_OCTAL_AS_HEX
  /* named fault PSG_REG_OCTAL_AS_HEX: the amplitude registers (data sheet R10..R12, octal) are placed
   * as if the number were read in the wrong base, one slot low: R10 lands at $01FA, not $01FB */
  if (idx >= 11u && idx <= 13u) {
    slot = idx - 1u;
  }
#endif
  return slot;
}

void ivs_psg_reg_store(const ivs_psg_params *p, uint16_t *regs, uint32_t idx, uint32_t value)
{
  uint32_t slot;
  uint32_t wmask;
  if (idx >= IVS_PSG_REG_COUNT) {
    return;
  }
  slot = ivs_psg_slot(idx);
  wmask = p->reg_wmask[slot];
#ifdef IVS_FAULT_PSG_TONE_HIGH_NIBBLE_UNMASKED
  if (idx >= 4u && idx <= 6u) {
    wmask = 0x00FFu; /* named fault: the coarse tone period keeps the whole low byte */
  }
#endif
  regs[slot] = IVS_U16(value & wmask);
}

uint16_t ivs_psg_reg_load(const ivs_psg_params *p, const uint16_t *regs, uint32_t idx)
{
  uint32_t slot;
  uint32_t rmask;
  if (idx >= IVS_PSG_REG_COUNT) {
    return 0u;
  }
  slot = ivs_psg_slot(idx);
  rmask = p->reg_rmask[slot];
#ifdef IVS_FAULT_PSG_TONE_HIGH_NIBBLE_UNMASKED
  if (idx >= 4u && idx <= 6u) {
    rmask = 0x00FFu;
  }
#endif
#ifdef IVS_FAULT_PSG_SHAPE_READBACK_STORED
  if (idx == 10u) {
    rmask = 0x00FFu; /* named fault: the shape register reads back what was stored */
  }
#endif
  return IVS_U16((uint32_t)regs[slot] & rmask & 0x00FFu);
}
