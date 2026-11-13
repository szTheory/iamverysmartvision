/* SPDX-License-Identifier: MIT */
/* Suite video, family card (plan 06-10, VID-04, D-18, D-19): the card-level pixel rules of the Foreground/Background renderer, each on a small
 * constructed frame whose expected 8 x 8 block is written here as text from the card rules (never read from a run): the bit order (bit 7
 * is the leftmost pixel, 1 is foreground), the row order (graphics byte 0 is the top pixel row), the 16 colors, the cut-off 20th column,
 * the zero graphics ROM, the display-off border fill and the color range. Graphics RAM is poked through the raw seam and the unit-path
 * memory system of video_fgbg.c is used; the graphics ROM is the zero image (D-19: no GROM in CI, no third-party glyph anywhere). */
#include <stdint.h>
#include <string.h>

#include "video_cases.h"

static uint8_t vc_plane[VF_PLANE_BYTES];
static uint16_t vc_words[240];
static ivs_stic_state vc_state;

static void vc_clear_words(void)
{
  memset(vc_words, 0, sizeof vc_words);
}

/* 1 when the 8 x 8 block of cell (r, c) of the plane reads as the 8 text rows (digits and letters are colors, '#' the sentinel). */
static int vc_block_is(uint32_t r, uint32_t c, const char *const rows[8])
{
  uint32_t y;
  uint32_t x;
  for (y = 0u; y < 8u; y++) {
    for (x = 0u; x < 8u; x++) {
      char ch = rows[y][x];
      uint8_t want;
      uint8_t got = vc_plane[(8u * r + y) * VF_PLANE_W + 8u * c + x];
      if (ch == '#') {
        want = 0xFFu;
      } else if (ch >= '0' && ch <= '9') {
        want = (uint8_t)(ch - '0');
      } else {
        want = (uint8_t)(10 + (ch - 'A'));
      }
      if (got != want) {
        ivs_test_diag("cell (%u,%u) pixel (%u,%u): got %u, expected %u", (unsigned)r, (unsigned)c, (unsigned)x, (unsigned)y, (unsigned)got,
                      (unsigned)want);
        return 0;
      }
    }
  }
  return 1;
}

/* Bit 7 is the leftmost pixel and a set bit is the foreground: card 5 = 80 01 00 .. 00, foreground 1, background 2. */
static int vc_bit_order(void)
{
  static const char *const blk[8] = {"12222222", "22222221", "22222222", "22222222", "22222222", "22222222", "22222222", "22222222"};
  static const uint8_t pat[8] = {0x80u, 0x01u, 0u, 0u, 0u, 0u, 0u, 0u};
  vf_mem_clear();
  vf_poke_gram_card(vf_mem(), 5u, pat);
  vc_clear_words();
  vc_words[20u * 4u + 4u] = (uint16_t)vid_encode_fgbg(1u, 2u, 1u, 5u);
  vf_render_words(vc_words, 0u, vc_plane);
  VC_CHECK(vc_block_is(4u, 4u, blk));
  return IVS_TEST_PASS;
}

/* Graphics byte y is pixel row y from the top: card 6 = FF 00 00 F0 00 00 00 0F. */
static int vc_row_order(void)
{
  static const char *const blk[8] = {"11111111", "22222222", "22222222", "11112222", "22222222", "22222222", "22222222", "22221111"};
  static const uint8_t pat[8] = {0xFFu, 0u, 0u, 0xF0u, 0u, 0u, 0u, 0x0Fu};
  vf_mem_clear();
  vf_poke_gram_card(vf_mem(), 6u, pat);
  vc_clear_words();
  vc_words[20u * 7u + 10u] = (uint16_t)vid_encode_fgbg(1u, 2u, 1u, 6u);
  vf_render_words(vc_words, 0u, vc_plane);
  VC_CHECK(vc_block_is(7u, 10u, blk));
  return IVS_TEST_PASS;
}

/* Sixteen cells with background k and foreground (k + 3) and 7 (never equal to k): pixel (0,0) is the foreground, pixel (1,1) the
 * background, so every one of the 16 colors is read back as a background and the foreground range 0..7 as a foreground. */
static int vc_colors_0_15(void)
{
  static const uint8_t pat[8] = {0x80u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
  uint32_t k;
  vf_mem_clear();
  vf_poke_gram_card(vf_mem(), 1u, pat);
  vc_clear_words();
  for (k = 0u; k < 16u; k++) {
    vc_words[20u * 3u + k] = (uint16_t)vid_encode_fgbg((k + 3u) & 7u, k, 1u, 1u);
  }
  vf_render_words(vc_words, 0u, vc_plane);
  for (k = 0u; k < 16u; k++) {
    uint8_t fg = vc_plane[(8u * 3u) * VF_PLANE_W + 8u * k];
    uint8_t bg = vc_plane[(8u * 3u + 1u) * VF_PLANE_W + 8u * k + 1u];
    if (fg != ((k + 3u) & 7u) || bg != k) {
      ivs_test_diag("color %u: foreground read %u (expected %u), background read %u", (unsigned)k, (unsigned)fg, (unsigned)((k + 3u) & 7u),
                    (unsigned)bg);
      return IVS_TEST_FAIL;
    }
  }
  return IVS_TEST_PASS;
}

/* Card 7 = 03 00 .. 00 has a marker in pixel 6 and pixel 7: in column 18 both show, in column 19 pixel 6 shows and pixel 7 is the
 * sentinel; every pixel of x = 159 is the sentinel and no other position is. */
static int vc_last_column_clip(void)
{
  static const char *const full[8] = {"22222211", "22222222", "22222222", "22222222", "22222222", "22222222", "22222222", "22222222"};
  static const char *const cut[8] = {"2222221#", "2222222#", "2222222#", "2222222#", "2222222#", "2222222#", "2222222#", "2222222#"};
  static const uint8_t pat[8] = {0x03u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
  uint32_t y;
  uint32_t x;
  vf_mem_clear();
  vf_poke_gram_card(vf_mem(), 7u, pat);
  vc_clear_words();
  vc_words[20u * 2u + 18u] = (uint16_t)vid_encode_fgbg(1u, 2u, 1u, 7u);
  vc_words[20u * 2u + 19u] = (uint16_t)vid_encode_fgbg(1u, 2u, 1u, 7u);
  vf_render_words(vc_words, 0u, vc_plane);
  VC_CHECK(vc_block_is(2u, 18u, full));
  VC_CHECK(vc_block_is(2u, 19u, cut));
  for (y = 0u; y < VF_PLANE_H; y++) {
    for (x = 0u; x < VF_PLANE_W; x++) {
      int blank = vc_plane[y * VF_PLANE_W + x] == 0xFFu;
      if (blank != (x == VF_PLANE_W - 1u)) {
        ivs_test_diag("pixel (%u,%u): sentinel %d, expected %d", (unsigned)x, (unsigned)y, blank, x == VF_PLANE_W - 1u);
        return IVS_TEST_FAIL;
      }
    }
  }
  return IVS_TEST_PASS;
}

/* The graphics ROM is the zero image in CI: a graphics ROM card of any number shows only its background, even with every graphics RAM
 * card all ones (so a swapped select would show foreground). Graphics RAM cards in the same frame do show it. */
static int vc_grom_zero_stub(void)
{
  static const uint8_t ones[8] = {0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu};
  uint32_t card;
  uint32_t y;
  uint32_t x;
  vf_mem_clear();
  for (card = 0u; card < 64u; card++) {
    vf_poke_gram_card(vf_mem(), card, ones);
  }
  for (card = 0u; card < 64u; card++) {
    for (y = 0u; y < 8u; y++) {
      VC_CHECK(ivs_memsys_stic_gfx_byte_internal(vf_mem(), 0u, card, y) == 0u); /* the graphics ROM image is zero */
      VC_CHECK(ivs_memsys_stic_gfx_byte_internal(vf_mem(), 1u, card, y) == 0xFFu);
    }
  }
  vc_clear_words();
  for (card = 0u; card < 20u; card++) {
    vc_words[20u * 4u + card] = (uint16_t)vid_encode_fgbg(7u, 3u, 0u, card); /* graphics ROM cards 0..19, foreground 7, background 3 */
    vc_words[20u * 5u + card] = (uint16_t)vid_encode_fgbg(7u, 3u, 1u, card); /* the same fields over graphics RAM */
  }
  vf_render_words(vc_words, 0u, vc_plane);
  for (card = 0u; card < 19u; card++) { /* the cut-off column is checked by the clip test */
    for (y = 0u; y < 8u; y++) {
      for (x = 0u; x < 8u; x++) {
        VC_CHECK(vc_plane[(32u + y) * VF_PLANE_W + 8u * card + x] == 3u);
        VC_CHECK(vc_plane[(40u + y) * VF_PLANE_W + 8u * card + x] == 7u);
      }
    }
  }
  return IVS_TEST_PASS;
}

/* A display-off frame is the border color index everywhere and ignores BACKTAB (default-pin, C99): a non-trivial set of row latches and a
 * border register holding more than four bits (only the low four bits are the color). */
static int vc_display_off_fill_border(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint32_t border;
  uint32_t i;
  uint32_t r;
  uint32_t c;
  VC_CHECK(p != NULL);
  ivs_stic_state_reset(&vc_state, p);
  for (r = 0u; r < IVS_STIC_LATCH_ROWS; r++) {
    for (c = 0u; c < IVS_STIC_ROW_WORDS; c++) {
      vc_state.row_latch[r][c] = (uint16_t)(0x2A55u + 37u * (r * 20u + c));
    }
  }
  for (border = 5u; border < 16u; border += 7u) { /* 5 and 12 */
    vf_mem_clear();
    vf_mem()->stic_reg[0x2Cu] = (uint16_t)(0x00F0u | border);
    VC_CHECK(ivs_stic_render_plane(p, &vc_state, vf_mem(), 0u, vc_plane) == VF_PLANE_BYTES);
    for (i = 0u; i < VF_PLANE_BYTES; i++) {
      if (vc_plane[i] != border) {
        ivs_test_diag("border %u: byte %u is %u", (unsigned)border, (unsigned)i, (unsigned)vc_plane[i]);
        return IVS_TEST_FAIL;
      }
    }
  }
  return IVS_TEST_PASS;
}

/* Every non-sentinel pixel is a color 0..15 and the sentinel is only the cut-off column, over seeded random words and graphics. */
static int vc_color_range(void)
{
  uint32_t s = 0x2545F491u;
  uint32_t i;
  uint32_t card;
  uint32_t y;
  vf_mem_clear();
  for (card = 0u; card < 64u; card++) {
    uint8_t b[8];
    for (y = 0u; y < 8u; y++) {
      s ^= s << 13;
      s ^= s >> 17;
      s ^= s << 5;
      b[y] = (uint8_t)(s & 0xFFu);
    }
    vf_poke_gram_card(vf_mem(), card, b);
  }
  for (i = 0u; i < 240u; i++) {
    s ^= s << 13;
    s ^= s >> 17;
    s ^= s << 5;
    vc_words[i] = (uint16_t)(s & 0xFFFFu);
  }
  vf_render_words(vc_words, 0u, vc_plane);
  for (i = 0u; i < VF_PLANE_BYTES; i++) {
    int cut = (i % VF_PLANE_W) == VF_PLANE_W - 1u;
    if (cut ? vc_plane[i] != 0xFFu : vc_plane[i] > 15u) {
      ivs_test_diag("byte %u is %u", (unsigned)i, (unsigned)vc_plane[i]);
      return IVS_TEST_FAIL;
    }
  }
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_video_card_cases[] = {
    {"video.card.bit-order", vc_bit_order, NULL},
    {"video.card.row-order", vc_row_order, NULL},
    {"video.card.colors-0-15", vc_colors_0_15, NULL},
    {"video.card.last-column-clip", vc_last_column_clip, NULL},
    {"video.grom.zero-stub", vc_grom_zero_stub, NULL},
    {"video.display-off.fill-border", vc_display_off_fill_border, NULL},
    {"video.color.range-0-15", vc_color_range, NULL},
};
const size_t ivs_video_card_count = sizeof ivs_video_card_cases / sizeof ivs_video_card_cases[0];
