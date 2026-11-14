/* SPDX-License-Identifier: MIT */
/* STIC moving objects (plan 08-04; D-01, D-02, D-03; rules STIC-R37 to R43 and R50 of data/stic/rules.json, bit roles of
 * data/video/mob.literal.txt): a pure decode of the eight MOBs from their three registers each and a pure per-plane-row overlay onto the
 * row of card colors. The file holds no static or global writable data, allocates nothing and keeps no state between calls; its only
 * imports are memset and the graphics accessor of the memory system. Graphics bytes come ONLY from ivs_memsys_stic_gfx_byte_internal (the
 * backing accessor: no gate, no undriven fill, no access record), so rendering has no read side effect. Every value is a tier 4 reading of
 * one community description (D-01): nothing here is a statement about a console.
 *
 * Registers (the register-address literal of data/stic): MOB i has the X word at 0x00 + i, the Y word at 0x08 + i and the attribute word at 0x10 + i.
 *   X word: bits 7..0 the X coordinate (0 disables the MOB, R37), bit 8 the interaction enable (collision, plan 08-05, no effect here),
 *           bit 9 visible, bit 10 double width (R38).
 *   Y word: bits 6..0 the Y coordinate (0 is not special, R37), bit 7 tall mode, bits 9 and 8 the Y size code, bit 10 X flip, bit 11 Y flip.
 *   Attribute word: bits 2..0 color, bits 10..3 card number, bit 11 graphics RAM (1) or ROM (0), bit 12 color bit 3, bit 13 priority.
 *
 * Model (rules R37, R38, D-03): line-resolved. A MOB is an image of 8 (tall mode: 16) pattern rows of 8 pattern columns. Pattern row r
 * covers lpr television lines, lpr = 1 << (Y size code), and its first line is top + lpr r with top = video_row_lines x (Y - origin_y);
 * pattern column c covers zx plane columns from left + zx c with left = X - origin_x, zx 1 or 2. The published plane takes, for plane row
 * y, the television line video_row_lines x y + video_mob_line_sample (the even line by default, C108): with the default of 2 lines per
 * plane row every code of at least 2 lines per pattern row (the zooms 2x, 4x and 8x) is sampling-invariant and the code with 1 line per
 * pattern row is the pinned half-height (R50). A position at or beyond video_width is never written (the published plane is 159 wide,
 * the 160th working column is the cut-off pixel of the card layer).
 *
 * Order (R41, R42, R43): among the MOBs that have a set pattern pixel on a position the one with the lower index owns it; the priority
 * bit does not change that. An owner with the priority bit clear is drawn in front of the card layer; an owner with the priority bit set
 * is hidden by the card layer where the card has an on pixel (video_mob_prio_hides 0, the default) or on every card pixel (1).
 *
 * Named faults (plan 08-04, one #ifdef site each): MOB_PRIORITY_SWAP (the higher index owns an overlap), MOB_FLIP_SWAPPED (the X and Y
 * flip bits are exchanged). */
#include "common/ivs_prelude.h"

#include "stic/stic.h"

/* The register file words of MOB i: X at 0x00 + i, Y at 0x08 + i, attribute at 0x10 + i. */
#define IVS_STIC_MOB_REG_X 0x00u
#define IVS_STIC_MOB_REG_Y 0x08u
#define IVS_STIC_MOB_REG_A 0x10u

uint32_t ivs_stic_mob_decode(const ivs_stic_params *p, const ivs_memsys *mem, ivs_stic_mob *out)
{
  uint32_t i;
  uint32_t n = 0u;
  for (i = 0u; i < IVS_STIC_MOBS; i++) {
    uint32_t xw = (uint32_t)mem->stic_reg[IVS_STIC_MOB_REG_X + i];
    uint32_t yw = (uint32_t)mem->stic_reg[IVS_STIC_MOB_REG_Y + i];
    uint32_t aw = (uint32_t)mem->stic_reg[IVS_STIC_MOB_REG_A + i];
    uint32_t xc = xw & 0xFFu;
    uint32_t yc = yw & 0x7Fu;
    ivs_stic_mob *m = &out[i];
    memset(m, 0, sizeof *m);
    m->on = (uint8_t)((xc != 0u && ((xw >> 9) & 1u) != 0u) ? 1u : 0u);
    m->left = (int16_t)((int32_t)xc - (int32_t)p->video_mob_origin_x);
    m->top = (int16_t)((int32_t)p->video_row_lines * ((int32_t)yc - (int32_t)p->video_mob_origin_y));
    m->zx = (uint8_t)(((xw >> 10) & 1u) != 0u ? 2u : 1u);
    m->lpr = (uint8_t)(((yw >> 8) & 1u) != 0u ? 2u : 1u);
    m->rows = 8u;
    m->sel = (uint8_t)((aw >> 11) & 1u);
    m->card = (uint8_t)((aw >> 3) & (m->sel != 0u ? 0x3Fu : 0xFFu));
    m->color = (uint8_t)(aw & 7u);
    n += m->on;
  }
  return n;
}

void ivs_stic_overlay_mobs_row(const ivs_stic_params *p, const ivs_memsys *mem, const ivs_stic_mob *mobs, uint32_t plane_y, const uint8_t *on_row,
                               uint8_t *row)
{
  uint8_t claimed[IVS_STIC_ROW_WORDS]; /* one bit per plane column: a MOB with a lower index already owns the position */
  int32_t line = (int32_t)(p->video_row_lines * plane_y + p->video_mob_line_sample);
  int32_t width = (int32_t)p->video_width;
  uint32_t i;
  (void)on_row; /* the priority bit is decoded by a later task of plan 08-04 */
  memset(claimed, 0, sizeof claimed);
  for (i = 0u; i < IVS_STIC_MOBS; i++) {
    const ivs_stic_mob *m = &mobs[i];
    int32_t rel;
    uint32_t r;
    uint32_t byte;
    uint32_t c;
    if (m->on == 0u) {
      continue;
    }
    rel = line - (int32_t)m->top;
    if (rel < 0) {
      continue;
    }
    r = (uint32_t)rel / (uint32_t)m->lpr;
    if (r >= (uint32_t)m->rows) {
      continue;
    }
    byte = (uint32_t)ivs_memsys_stic_gfx_byte_internal(mem, m->sel, m->card, r);
    for (c = 0u; c < 8u; c++) {
      uint32_t k;
      if (((byte >> (7u - c)) & 1u) == 0u) {
        continue;
      }
      for (k = 0u; k < (uint32_t)m->zx; k++) {
        int32_t x = (int32_t)m->left + (int32_t)((uint32_t)m->zx * c + k);
        if (x < 0 || x >= width) {
          continue;
        }
        if (((claimed[(uint32_t)x >> 3] >> (7u - ((uint32_t)x & 7u))) & 1u) != 0u) {
          continue;
        }
        claimed[(uint32_t)x >> 3] = (uint8_t)(claimed[(uint32_t)x >> 3] | (0x80u >> ((uint32_t)x & 7u)));
        row[x] = m->color;
      }
    }
  }
}
