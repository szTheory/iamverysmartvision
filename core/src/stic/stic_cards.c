/* SPDX-License-Identifier: MIT */
/* STIC card renderer (plan 06-10; D-18, D-19, D-11, D-12): a pure function of the card inputs, the 20 BACKTAB words of one card row and
 * the graphics memory. It writes the 20-cards-wide working plane (160 x 96 index bytes); the frame writer crops it to the published
 * 159 columns. The file holds no static or global writable data, allocates nothing and keeps no state between calls.
 *
 * Foreground/Background word (data/video/cards.literal.txt, variant fgbg; rules CR-01 to CR-08, CR-12, CR-14): W = word and $3FFF (bits
 * 15 and 14 are not used). Bit 11 selects the graphics memory (0 graphics ROM, 1 graphics RAM), bits 8..3 are the card number, bits
 * 2..0 the foreground color (0..7), the background color is 8 x bit 12 + 4 x bit 13 + 2 x bit 10 + bit 9 (0..15). The color stack is
 * never read and bit 13 never advances anything in this mode. Pixel x (0 left) of graphics byte y (0 top) is the foreground color when
 * bit 7 - x of the byte is set, else the background color. In card column 19 the pixel x = 7 is IVS_PIXEL_BLANK: the 7-pixel last
 * column cuts it off (clip side right, video.last_card_column_drop).
 *
 * The graphics bytes come ONLY from ivs_memsys_stic_gfx_byte_internal (the pure backing accessor: no gate, no undriven fill, no access
 * record), never the CPU view, so a closed graphics window cannot leak the undriven fill into a pixel and rendering has no read side
 * effect. The graphics ROM is the zero image in every CI run (D-19: no GROM in CI or in any fixture), so a graphics ROM card renders as
 * its background only.
 *
 * Named faults (plan 06-10): VIDEO_WRONG_CARD_MODE (the mode sense is inverted, a Foreground/Background frame is decoded as Color
 * Stack), VIDEO_CARD_BIT_ORDER_REVERSED (bit 0 of a graphics byte is the leftmost pixel), VIDEO_GRAM_GROM_SELECT_SWAPPED (the sense of
 * bit 11 is inverted), VIDEO_20TH_COLUMN_WIDTH (column 19 shows its 8th pixel). */
#include "common/ivs_prelude.h"

#include "stic/stic.h"

#define IVS_CARD_COLS 20u
#define IVS_CARD_ROWS 12u
#define IVS_CARD_W 8u
#define IVS_CARD_H 8u
#define IVS_CARD_PLANE_W (IVS_CARD_COLS * IVS_CARD_W)
#define IVS_CARD_ROW_BYTES (IVS_CARD_PLANE_W * IVS_CARD_H)

uint32_t ivs_stic_render_cards_row(const ivs_stic_cardin *in, const uint16_t *row_words, const ivs_memsys *mem, uint8_t *out)
{
  uint32_t c;
  uint32_t y;
  uint32_t x;
  uint32_t fg_mode_ok = in->mode == IVS_STIC_MODE_FGBG ? 1u : 0u;
#ifdef IVS_FAULT_VIDEO_WRONG_CARD_MODE
  fg_mode_ok = fg_mode_ok == 0u ? 1u : 0u; /* named fault: the mode sense is inverted */
#endif
  if (fg_mode_ok == 0u) {
    memset(out, (int)IVS_PIXEL_BLANK, IVS_CARD_ROW_BYTES); /* Color Stack: not decoded here (plan 06-11) */
    return IVS_CARD_ROW_BYTES;
  }
  for (c = 0u; c < IVS_CARD_COLS; c++) {
    uint32_t w = (uint32_t)row_words[c] & 0x3FFFu;
    uint32_t sel = (w >> 11) & 1u;
    uint32_t card = (w >> 3) & 0x3Fu;
    uint32_t fg = w & 7u;
    uint32_t bg = 8u * ((w >> 12) & 1u) + 4u * ((w >> 13) & 1u) + 2u * ((w >> 10) & 1u) + ((w >> 9) & 1u);
#ifdef IVS_FAULT_VIDEO_GRAM_GROM_SELECT_SWAPPED
    sel = sel ^ 1u; /* named fault: the sense of bit 11 is inverted */
#endif
    for (y = 0u; y < IVS_CARD_H; y++) {
      uint32_t byte = (uint32_t)ivs_memsys_stic_gfx_byte_internal(mem, sel, card, y);
      uint8_t *dst = out + (size_t)y * IVS_CARD_PLANE_W + (size_t)c * IVS_CARD_W;
      for (x = 0u; x < IVS_CARD_W; x++) {
#ifdef IVS_FAULT_VIDEO_CARD_BIT_ORDER_REVERSED
        uint32_t bit = (byte >> x) & 1u; /* named fault: bit 0 is the leftmost pixel */
#else
        uint32_t bit = (byte >> (7u - x)) & 1u;
#endif
        dst[x] = (uint8_t)(bit != 0u ? fg : bg);
      }
#ifdef IVS_FAULT_VIDEO_20TH_COLUMN_WIDTH
      /* named fault: column 19 shows its 8th pixel (the 7-pixel cut-off is not applied) */
#else
      if (c == IVS_CARD_COLS - 1u) {
        dst[7] = (uint8_t)IVS_PIXEL_BLANK; /* the cut-off pixel of the 7-pixel last column */
      }
#endif
    }
  }
  return IVS_CARD_ROW_BYTES;
}

uint32_t ivs_stic_render_cards(const ivs_stic_cardin *in, const uint16_t *words, const ivs_memsys *mem, uint8_t *plane)
{
  uint32_t r;
  for (r = 0u; r < IVS_CARD_ROWS; r++) {
    (void)ivs_stic_render_cards_row(in, words + (size_t)r * IVS_CARD_COLS, mem, plane + (size_t)r * IVS_CARD_ROW_BYTES);
  }
  return IVS_CARD_ROWS * IVS_CARD_ROW_BYTES;
}
