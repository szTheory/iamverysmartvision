/* SPDX-License-Identifier: MIT */
/* CP-1610 decode table. Rows are transcribed from the instruction tables of the GI manual
 * (pdf 50-58) as handlers land; the Phase 1 family table is an inventory, never a value
 * source. Decode looks only at bits 9..0 of a word (D-07). A decle without an implemented
 * row decodes to NULL and the stepper reports the temporary UNIMPLEMENTED fault. */
#include "common/ivs_prelude.h"

#include "cpu/cp1610.h"

/* No rows yet: the first rows arrive with their handlers. A one-element array with a zero
 * mask would match every decle, so the table is empty and the count is zero. */
static const ivs_cp1610_row *const ivs_cp1610_rows[1] = {NULL};
static const size_t ivs_cp1610_row_count = 0u;

const ivs_cp1610_row *ivs_cp1610_decode(uint16_t decle)
{
  uint32_t d = (uint32_t)decle & 0x3FFu;
  size_t i;
  for (i = 0u; i < ivs_cp1610_row_count; i++) {
    const ivs_cp1610_row *row = ivs_cp1610_rows[i];
    if (row != NULL && (d & row->mask) == row->value) {
      return row;
    }
  }
  return NULL;
}
