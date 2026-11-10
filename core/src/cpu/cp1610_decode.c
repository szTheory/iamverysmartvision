/* SPDX-License-Identifier: MIT */
/* CP-1610 decode table. Rows are transcribed from the instruction tables of the GI manual
 * (pdf 50-58) as handlers land; the Phase 1 family table is an inventory, never a value
 * source. Decode looks only at bits 9..0 of a word (D-07). A decle without an implemented
 * row decodes to NULL and the stepper reports the temporary UNIMPLEMENTED fault. */
#include "common/ivs_prelude.h"

#include "cpu/cp1610.h"

/* Columns: mask, value, words, base cycles, interruptible, handler.
 * Control group (pdf 56; SDBD on pdf 57), 4 cycles each: HLT 0000 000 000, SDBD 0000 000 001,
 * EIS 0000 000 010, DIS 0000 000 011, TCI 0000 000 101, CLRC 0000 000 110, SETC 0000 000 111.
 * EIS, DIS, TCI, CLRC and SETC are printed "Not Interruptable"; HLT and SDBD are
 * manual-silent and non-interruptible by default (C56, POL-INT-02).
 * Single-register forms (pdf 56, Fig 3.3.3 and footnote on pdf 51), 6 cycles each, no
 * interruptibility note, so interruptible: GSWD 0000 110 0DD (R0-R3), NOP 0000 110 10N,
 * SIN 0000 110 11N (N is ignored), RSWD 0000 111 SSS. */
static const ivs_cp1610_row ivs_cp1610_rows[] = {
    {0x3FFu, 0x000u, 1u, 4u, 0u, IVS_CP1610_OP_HLT},
    {0x3FFu, 0x001u, 1u, 4u, 0u, IVS_CP1610_OP_SDBD},
    {0x3FFu, 0x002u, 1u, 4u, 0u, IVS_CP1610_OP_EIS},
    {0x3FFu, 0x003u, 1u, 4u, 0u, IVS_CP1610_OP_DIS},
    {0x3FFu, 0x005u, 1u, 4u, 0u, IVS_CP1610_OP_TCI},
    {0x3FFu, 0x006u, 1u, 4u, 0u, IVS_CP1610_OP_CLRC},
    {0x3FFu, 0x007u, 1u, 4u, 0u, IVS_CP1610_OP_SETC},
    {0x3FCu, 0x030u, 1u, 6u, 1u, IVS_CP1610_OP_GSWD},
    {0x3FEu, 0x034u, 1u, 6u, 1u, IVS_CP1610_OP_NOP},
    {0x3FEu, 0x036u, 1u, 6u, 1u, IVS_CP1610_OP_SIN},
    {0x3F8u, 0x038u, 1u, 6u, 1u, IVS_CP1610_OP_RSWD},
};

const ivs_cp1610_row *ivs_cp1610_decode(uint16_t decle)
{
  uint32_t d = (uint32_t)decle & 0x3FFu;
  size_t i;
  for (i = 0u; i < sizeof ivs_cp1610_rows / sizeof ivs_cp1610_rows[0]; i++) {
    const ivs_cp1610_row *row = &ivs_cp1610_rows[i];
    if ((d & row->mask) == row->value) {
      return row;
    }
  }
  return NULL;
}
