/* SPDX-License-Identifier: MIT */
/* CP-1610 decode table. Rows are transcribed from the instruction tables of the GI manual
 * (pdf 50-58) as handlers land; the Phase 1 family table is an inventory, never a value
 * source. Decode looks only at bits 9..0 of a word (D-07). A decle without an implemented
 * row decodes to NULL and the stepper reports the temporary UNIMPLEMENTED fault. */
#include "common/ivs_prelude.h"

#include "cpu/cp1610.h"

/* Columns: mask, value, words, base cycles, interruptible, handler.
 * Control group (pdf 56): EIS 0000 000 010, DIS 0000 000 011, CLRC 0000 000 110 and
 * SETC 0000 000 111, 4 cycles each, all marked "Not Interruptable"; status change C for
 * CLRC and SETC only. */
static const ivs_cp1610_row ivs_cp1610_rows[] = {
    {0x3FFu, 0x002u, 1u, 4u, 0u, IVS_CP1610_OP_EIS},
    {0x3FFu, 0x003u, 1u, 4u, 0u, IVS_CP1610_OP_DIS},
    {0x3FFu, 0x006u, 1u, 4u, 0u, IVS_CP1610_OP_CLRC},
    {0x3FFu, 0x007u, 1u, 4u, 0u, IVS_CP1610_OP_SETC},
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
