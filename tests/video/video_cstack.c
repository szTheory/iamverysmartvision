/* SPDX-License-Identifier: MIT */
/* Suite video, families cstack and mode (plan 06-11, VID-04, VID-06, D-06, D-18, D-19): the Color Stack renderer against the hand-derived frame
 * literals F-B1..F-B3 of plan 06-01 (data/video/frames, written from the card rules before any renderer existed, never produced by a run), in
 * the unit path (ivs_stic_render_cards) and through the whole machine (a stand-in guest reads $0021 inside the register window, which selects
 * Color Stack, strobes $0020, and the DELIVERED slot plane is compared). The frame helpers are those of video_fgbg.c; the graphics ROM is
 * the zero image and graphics RAM holds project-authored geometric shapes.
 *
 * The rule tests (family cstack) each build one small constructed frame and compare pixels with values written here from the card rules
 * (CR-09 to CR-13), never read from a run: stack {2, 5, 9, 15}, graphics RAM card 1 = 80 00 .. 00 (pixel (0,0) is the foreground, every
 * other pixel of the card the background), so the pixel (1,1) of a card cell reads the background that card was drawn with.
 * Family mode: the mode switch end to end through stand-in guests. */
#include <stdint.h>
#include <string.h>

#include "video_cases.h"

#define VCS_FB1 5u /* the index of F-B1 in the generated frame table (F-A1..F-A5 come first) */
#define VCS_FB2 6u
#define VCS_FB3 7u


#define VCS_SLOT 15296u /* 32-byte header and 159 x 96 index bytes */
#define VCS_E0 59736u

static uint8_t vcs_plane[VF_PLANE_BYTES];
static uint8_t vcs_plane2[VF_PLANE_BYTES];
static uint16_t vcs_words[240];
static const uint32_t vcs_stack[4] = {2u, 5u, 9u, 15u};

static int vcs_unit_b1(void)
{
  return vf_unit(VCS_FB1, 1u);
}

static int vcs_unit_b2(void)
{
  return vf_unit(VCS_FB2, 1u);
}

static int vcs_unit_b3(void)
{
  return vf_unit(VCS_FB3, 1u);
}

static int vcs_machine_b1(void)
{
  return vf_machine(VCS_FB1, 1u);
}

static int vcs_machine_b3(void)
{
  return vf_machine(VCS_FB3, 1u);
}

/* ---- constructed frames ---- */

/* The unit-path memory with graphics RAM card 1 = 80 00 00 00 00 00 00 00 and a zero word frame. */
static void vcs_begin(void)
{
  static const uint8_t pat[8] = {0x80u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
  vf_mem_clear();
  vf_poke_gram_card(vf_mem(), 1u, pat);
  memset(vcs_words, 0, sizeof vcs_words);
}

/* The Color Stack word of graphics RAM card 1, foreground 1, with or without the advance bit (bits 13, 11, 3 and 0: $2809 or $0809). */
static uint16_t vcs_card1(uint32_t adv)
{
  return (uint16_t)vid_encode_cs(adv, 1u, 1u, 1u);
}

/* The background of the cell (r, c) as the plane shows it (pixel (1,1) of the block, a background pixel of card 1 and of every blank card). */
static uint32_t vcs_bg(uint32_t r, uint32_t c)
{
  return vcs_plane[(8u * r + 1u) * VF_PLANE_W + 8u * c + 1u];
}

static uint32_t vcs_render(void)
{
  uint32_t pos = 99u;
  vf_render_cs(vcs_words, 1u, vcs_stack, vcs_plane, &pos);
  return pos;
}

/* The advance happens BEFORE the card is drawn: a card with bit 13 shows stack entry 1, not entry 0; the pointer persists to the next cell. */
static int vcs_advance_before_draw(void)
{
  uint32_t pos;
  vcs_begin();
  vcs_words[20u * 2u + 0u] = vcs_card1(0u); /* bg stack[0] = 2 */
  vcs_words[20u * 2u + 1u] = vcs_card1(1u); /* advance: stack[1] = 5 */
  vcs_words[20u * 2u + 2u] = vcs_card1(0u); /* no advance: the pointer stays at 1, 5 */
  vcs_words[20u * 2u + 3u] = vcs_card1(1u); /* advance: stack[2] = 9 */
  pos = vcs_render();
  VC_CHECK(vcs_bg(2u, 0u) == 2u && vcs_bg(2u, 1u) == 5u && vcs_bg(2u, 2u) == 5u && vcs_bg(2u, 3u) == 9u);
  VC_CHECK(vcs_bg(2u, 4u) == 9u); /* the fill word after it keeps the last entry */
  VC_CHECK(vcs_plane[(8u * 2u) * VF_PLANE_W + 8u * 1u] == 1u); /* pixel (0,0) of card 1 is the foreground 1 */
  VC_CHECK(pos == 2u);
  return IVS_TEST_PASS;
}

/* Five advances in a row: 1, 2, 3, then the wrap 3 to 0, then 1 again: backgrounds 5, 9, 15, 2, 5; the pointer ends at 1. */
static int vcs_advance_wrap(void)
{
  uint32_t c;
  uint32_t pos;
  static const uint32_t want[5] = {5u, 9u, 15u, 2u, 5u};
  vcs_begin();
  for (c = 0u; c < 5u; c++) {
    vcs_words[20u * 4u + c] = vcs_card1(1u);
  }
  pos = vcs_render();
  for (c = 0u; c < 5u; c++) {
    if (vcs_bg(4u, c) != want[c]) {
      ivs_test_diag("advance %u: background %u, expected %u", (unsigned)(c + 1u), (unsigned)vcs_bg(4u, c), (unsigned)want[c]);
      return IVS_TEST_FAIL;
    }
  }
  VC_CHECK(vcs_bg(4u, 5u) == 5u && pos == 1u);
  return IVS_TEST_PASS;
}

/* One stack across all rows: one advance at cell (3, 10) and every later cell in raster order, in every following row, reads entry 1; every
 * earlier cell reads entry 0 (no reset at a row start). */
static int vcs_one_stack_across_rows(void)
{
  uint32_t r;
  uint32_t c;
  uint32_t pos;
  vcs_begin();
  vcs_words[20u * 3u + 10u] = vcs_card1(1u);
  pos = vcs_render();
  for (r = 0u; r < 12u; r++) {
    for (c = 0u; c < 20u; c++) {
      uint32_t idx = 20u * r + c;
      uint32_t want = idx >= 20u * 3u + 10u ? 5u : 2u;
      if (c == 19u && idx < 20u * 3u + 10u) {
        want = 2u;
      }
      if (vcs_bg(r, c) != want) {
        ivs_test_diag("cell (%u,%u): background %u, expected %u", (unsigned)r, (unsigned)c, (unsigned)vcs_bg(r, c), (unsigned)want);
        return IVS_TEST_FAIL;
      }
    }
  }
  VC_CHECK(pos == 1u);
  return IVS_TEST_PASS;
}

/* The cut-off 20th-column card advances the pointer like any other (inference C102): the cell (5, 19) shows entry 1 in its 7 visible
 * pixels, its 8th pixel is the sentinel, the cell (5, 18) before it shows entry 0, and the cell (6, 0) after it shows entry 1. */
static int vcs_cutoff_column_advances(void)
{
  uint32_t pos;
  vcs_begin();
  vcs_words[20u * 5u + 19u] = vcs_card1(1u);
  pos = vcs_render();
  VC_CHECK(vcs_bg(5u, 18u) == 2u);
  VC_CHECK(vcs_bg(5u, 19u) == 5u);
  VC_CHECK(vcs_plane[(8u * 5u + 1u) * VF_PLANE_W + 159u] == 0xFFu); /* the cut-off pixel */
  VC_CHECK(vcs_bg(6u, 0u) == 5u);
  VC_CHECK(pos == 1u);
  return IVS_TEST_PASS;
}

/* Machine path, three advances in the BACKTAB and a strobe every frame: frames 0 and 1 are equal to each other and to the unit render of the
 * same words, and the stored pointer is 3, not 6 mod 4: the pointer restarts at 0 for every frame (D-06), it is not persistent state. */
static int vcs_run_loop(vp_run *r, uint32_t advances)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = vp_prog_cs_strobe_loop(w);
  struct ivs_machine *im;
  uint32_t k;
  static const uint32_t cell[4] = {20u * 1u + 1u, 20u * 5u + 5u, 20u * 9u + 9u, 20u * 11u + 18u};
  VC_CHECK(vp_new(r, w, n, 3u * VCS_SLOT));
  im = ivs_machine_from_handle_mut(r->x.m);
  memset(im->mem.grom, 0, sizeof im->mem.grom);
  vcs_begin();
  for (k = 0u; k < advances; k++) {
    vcs_words[cell[k]] = vcs_card1(1u);
  }
  for (k = 0u; k < 240u; k++) {
    ivs_memsys_poke_raw_internal(&im->mem, (uint16_t)(VF_BACKTAB_BASE + k), vcs_words[k]);
  }
  {
    static const uint8_t pat[8] = {0x80u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
    vf_poke_gram_card(&im->mem, 1u, pat);
  }
  for (k = 0u; k < 4u; k++) {
    im->mem.stic_reg[0x28u + k] = (uint16_t)vcs_stack[k];
  }
  return IVS_TEST_PASS;
}

/* 1 when the two delivered planes (159 columns, in two slots of the buffer) are equal; shared by the test and its judge. */
static int vcs_planes_equal(const uint8_t *a, const uint8_t *b, uint32_t stride)
{
  return vf_first_diff(a, stride, b, stride, 159u, VF_PLANE_H) == -1;
}

static int vcs_reset_per_frame(void)
{
  vp_run r;
  uint32_t pos = 0u;
  VC_CHECK(vcs_run_loop(&r, 3u) == IVS_TEST_PASS);
  if (!vp_until(&r, 2u * VCS_E0 + 200u)) {
    vp_free(&r);
    return IVS_TEST_FAIL;
  }
  if (r.sink.video_frames != 2u || r.sink.video_written != 2u * VCS_SLOT) {
    ivs_test_diag("frames %u written %u, expected 2 and %u", (unsigned)r.sink.video_frames, (unsigned)r.sink.video_written,
                  (unsigned)(2u * VCS_SLOT));
    vp_free(&r);
    return IVS_TEST_FAIL;
  }
  VC_CHECK(vp_im(&r)->stic.mode == 1u && (vp_im(&r)->stic.cs_ptr == 3u));
  VC_CHECK(r.buf[24u + 3u] == (uint8_t)IVS_FRAME_F_DISPLAYED && r.buf[VCS_SLOT + 24u + 3u] == (uint8_t)IVS_FRAME_F_DISPLAYED);
  vf_render_cs(vcs_words, 1u, vcs_stack, vcs_plane2, &pos);
  VC_CHECK(pos == 3u);
  VC_CHECK(vcs_planes_equal(r.buf + 32u, r.buf + VCS_SLOT + 32u, 159u));
  VC_CHECK(vf_first_diff(r.buf + 32u, 159u, vcs_plane2, VF_PLANE_W, 159u, VF_PLANE_H) == -1);
  vp_free(&r);
  return IVS_TEST_PASS;
}

/* The judge: a stand-in renderer that keeps the pointer across frames (frame 1 starts where frame 0 ended) produces a different plane, and the
 * comparison of the test above reports it; the reset variant is reported equal (the control passes). */
static void vcs_render_from(uint32_t start, uint8_t *plane, uint32_t *final_pos)
{
  ivs_stic_cardin in;
  uint32_t pos = start;
  uint32_t k;
  uint32_t r;
  memset(&in, 0, sizeof in);
  in.mode = 1u;
  for (k = 0u; k < 4u; k++) {
    in.stack[k] = vcs_stack[k];
  }
  for (r = 0u; r < 12u; r++) {
    (void)ivs_stic_render_cards_row(&in, vcs_words + 20u * r, vf_mem(), &pos, plane + (size_t)r * 1280u);
  }
  *final_pos = pos;
}

static int vcs_judge_persistent_pointer(void)
{
  static uint8_t a[VF_PLANE_BYTES];
  static uint8_t b[VF_PLANE_BYTES];
  static uint8_t c[VF_PLANE_BYTES];
  uint32_t f0;
  uint32_t f1;
  uint32_t f2;
  vcs_begin();
  vcs_words[20u * 1u + 1u] = vcs_card1(1u);
  vcs_words[20u * 5u + 5u] = vcs_card1(1u);
  vcs_words[20u * 9u + 9u] = vcs_card1(1u);
  vcs_render_from(0u, a, &f0);  /* frame 0 */
  vcs_render_from(0u, b, &f1);  /* frame 1 with the reset every frame */
  vcs_render_from(f0, c, &f2);  /* frame 1 of the stand-in that keeps the pointer */
  VC_CHECK(f0 == 3u && f1 == 3u && f2 == 2u);
  VC_CHECK(vcs_planes_equal(a, b, VF_PLANE_W));  /* the control passes */
  VC_CHECK(!vcs_planes_equal(a, c, VF_PLANE_W)); /* the persistent pointer is detected */
  return IVS_TEST_PASS;
}

/* ---- Colored Squares ---- */

/* The block of a cell as 8 rows of text is compared to the plane; digits and letters are colors. */
static int vcs_block_is(uint32_t r, uint32_t c, const char *const rows[8])
{
  uint32_t y;
  uint32_t x;
  for (y = 0u; y < 8u; y++) {
    for (x = 0u; x < 8u; x++) {
      char ch = rows[y][x];
      uint8_t want = (uint8_t)(ch <= '9' ? ch - '0' : 10 + (ch - 'A'));
      uint8_t got = vcs_plane[(8u * r + y) * VF_PLANE_W + 8u * c + x];
      if (got != want) {
        ivs_test_diag("cell (%u,%u) pixel (%u,%u): got %u, expected %u", (unsigned)r, (unsigned)c, (unsigned)x, (unsigned)y, (unsigned)got,
                      (unsigned)want);
        return 0;
      }
    }
  }
  return 1;
}

/* The word 30D1: bit 12 set and bit 11 clear is Colored Squares; c0 = bits 2..0 = 1, c1 = bits 5..3 = 2, c2 = bits 8..6 = 3, and c3 = 4 x b13 + 2 x b10 +
 * b9 = 4 (bit 13 is a color bit here). Quadrants: top left 1, top right 2, bottom left 3, bottom right 4, each 4 x 4. The word 35A8: c0 = 0, c1 = 5,
 * c2 = 6 and c3 = 4 + 2 + 0 = 6 (bits 13 and 10 set, bit 9 clear): quadrants 0, 5, 6, 6. */
static int vcs_squares_quadrants(void)
{
  static const char *const blk_a[8] = {"11112222", "11112222", "11112222", "11112222", "33334444", "33334444", "33334444", "33334444"};
  static const char *const blk_b[8] = {"00005555", "00005555", "00005555", "00005555", "66666666", "66666666", "66666666", "66666666"};
  uint32_t pos;
  vcs_begin();
  vcs_words[20u * 3u + 3u] = 0x30D1u;
  vcs_words[20u * 3u + 4u] = 0x35A8u;
  pos = vcs_render();
  VC_CHECK(vcs_block_is(3u, 3u, blk_a));
  VC_CHECK(vcs_block_is(3u, 4u, blk_b));
  VC_CHECK(pos == 0u); /* bit 13 is a color bit in a squares card: nothing advanced */
  return IVS_TEST_PASS;
}

/* A squares card never advances, even with bit 13 set: the card after it still reads entry 0 (an advance would show 5). */
static int vcs_squares_no_advance(void)
{
  uint32_t pos;
  vcs_begin();
  vcs_words[20u * 6u + 6u] = 0x3000u; /* squares, bit 13 set (c3 = 4) */
  vcs_words[20u * 6u + 7u] = vcs_card1(0u);
  pos = vcs_render();
  VC_CHECK(vcs_bg(6u, 7u) == 2u);
  VC_CHECK(pos == 0u);
  return IVS_TEST_PASS;
}

/* Color 7 in a square draws the current stack entry and does not advance: after one advance (entry 1 = 5) the word 37FF, all four colors 7,
 * is 64 pixels of 5; a second advance and the same word shows 9, and the pointer ends at 2. */
static int vcs_squares_stack7(void)
{
  uint32_t y;
  uint32_t x;
  uint32_t pos;
  vcs_begin();
  vcs_words[20u * 7u + 0u] = vcs_card1(1u); /* the pointer is 1 */
  vcs_words[20u * 7u + 1u] = 0x37FFu;      /* 1000 | 2000 | 400 | 200 | 1FF: all four colors 7 */
  vcs_words[20u * 7u + 2u] = vcs_card1(1u); /* the pointer is 2 */
  vcs_words[20u * 7u + 3u] = 0x37FFu;
  pos = vcs_render();
  for (y = 0u; y < 8u; y++) {
    for (x = 0u; x < 8u; x++) {
      VC_CHECK(vcs_plane[(8u * 7u + y) * VF_PLANE_W + 8u * 1u + x] == 5u);
      VC_CHECK(vcs_plane[(8u * 7u + y) * VF_PLANE_W + 8u * 3u + x] == 9u);
    }
  }
  VC_CHECK(pos == 2u);
  return IVS_TEST_PASS;
}

/* ---- the mode switch end to end (family video.mode) ---- */

/* The F-B3 words and graphics rendered by the unit path in both modes: the expected delivered planes of the guests below. */
static uint8_t vcs_exp_cs[VF_PLANE_BYTES];
static uint8_t vcs_exp_fg[VF_PLANE_BYTES];

static int vcs_mode_expectations(const vf_frame **fo)
{
  const vf_frame *f = vf_frame_get(VCS_FB3);
  uint16_t words[240];
  VC_CHECK(f != NULL && f->mode == 1u);
  vf_mem_clear();
  vf_poke_gram(vf_mem(), f);
  vf_words(f, words);
  vf_render_cs(words, 1u, f->stack, vcs_exp_cs, NULL);
  vf_render_cs(words, 0u, f->stack, vcs_exp_fg, NULL);
  VC_CHECK(vf_first_diff(vcs_exp_cs, VF_PLANE_W, vcs_exp_fg, VF_PLANE_W, 159u, VF_PLANE_H) >= 0); /* the two modes differ on these words */
  *fo = f;
  return IVS_TEST_PASS;
}

/* A powered instance of w with the F-B3 BACKTAB, graphics and stack, run until ticks. */
static int vcs_mode_run(vp_run *r, const uint16_t *w, size_t n, const vf_frame *f, uint64_t ticks)
{
  struct ivs_machine *im;
  uint16_t words[240];
  uint32_t k;
  VC_CHECK(vp_new(r, w, n, 3u * VCS_SLOT));
  im = ivs_machine_from_handle_mut(r->x.m);
  memset(im->mem.grom, 0, sizeof im->mem.grom);
  vf_words(f, words);
  for (k = 0u; k < 240u; k++) {
    ivs_memsys_poke_raw_internal(&im->mem, (uint16_t)(VF_BACKTAB_BASE + k), words[k]);
  }
  vf_poke_gram(&im->mem, f);
  for (k = 0u; k < 4u; k++) {
    im->mem.stic_reg[0x28u + k] = (uint16_t)f->stack[k];
  }
  if (!vp_until(r, ticks)) {
    vp_free(r);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

/* 1 when slot s of the run holds the displayed frame equal to the 159 columns of exp. */
static int vcs_slot_is(const vp_run *r, uint32_t s, const uint8_t *exp)
{
  const uint8_t *slot = r->buf + (size_t)s * VCS_SLOT;
  if ((slot[27] & IVS_FRAME_F_DISPLAYED) == 0u) {
    ivs_test_diag("slot %u is not a displayed frame", (unsigned)s);
    return 0;
  }
  return vf_first_diff(slot + 32u, 159u, exp, VF_PLANE_W, 159u, VF_PLANE_H) == -1;
}

static int vcs_mode_switch_end_to_end(void)
{
  const vf_frame *f;
  vp_run r;
  uint16_t w[SC_PROG_MAX];
  size_t n;
  int ok;
  VC_CHECK(vcs_mode_expectations(&f) == IVS_TEST_PASS);
  /* a: a read of $0021 inside the register window selects Color Stack: frame 0 is the Color Stack render of the same words */
  n = vp_prog_cs_strobe_once(w);
  VC_CHECK(vcs_mode_run(&r, w, n, f, VCS_E0 + 200u) == IVS_TEST_PASS);
  ok = r.sink.video_frames == 1u && vp_im(&r)->stic.mode == 1u && vcs_slot_is(&r, 0u, vcs_exp_cs);
  vp_free(&r);
  VC_CHECK(ok);
  /* b: the read and then a write of $0021 selects Foreground/Background again: frame 0 is the Foreground/Background render */
  n = sc_w_mvi(w, 0u, 0x0021u, 1u);
  n = sc_w_mvo(w, n, 0u, 0x0021u);
  n = sc_w_mvo(w, n, 0u, 0x0020u);
  w[n++] = 0x0003u; /* DIS */
  n = sc_w_loop(w, n);
  VC_CHECK(vcs_mode_run(&r, w, n, f, VCS_E0 + 200u) == IVS_TEST_PASS);
  ok = r.sink.video_frames == 1u && vp_im(&r)->stic.mode == 0u && vcs_slot_is(&r, 0u, vcs_exp_fg);
  vp_free(&r);
  VC_CHECK(ok);
  /* c: a read of $0021 after the register window has closed changes nothing: the read is counted as blocked and the frame is Foreground/Background */
  n = sc_w_mvo(w, 0u, 0u, 0x0020u);
  n = sc_w_delay(w, n, SC_DELAY_PAST_REG);
  n = sc_w_mvi(w, n, 0x0021u, 1u);
  w[n++] = 0x0003u; /* DIS */
  n = sc_w_loop(w, n);
  VC_CHECK(vcs_mode_run(&r, w, n, f, VCS_E0 + 200u) == IVS_TEST_PASS);
  ok = r.sink.video_frames == 1u && vp_im(&r)->stic.mode == 0u && vp_im(&r)->stic.blocked_reads == 1u && vcs_slot_is(&r, 0u, vcs_exp_fg);
  vp_free(&r);
  VC_CHECK(ok);
  /* d: the mode persists across frames with no further access: two displayed frames, both Color Stack */
  n = vp_prog_cs_strobe_loop(w);
  VC_CHECK(vcs_mode_run(&r, w, n, f, 2u * VCS_E0 + 200u) == IVS_TEST_PASS);
  ok = r.sink.video_frames == 2u && vp_im(&r)->stic.mode == 1u && vcs_slot_is(&r, 0u, vcs_exp_cs) && vcs_slot_is(&r, 1u, vcs_exp_cs);
  vp_free(&r);
  VC_CHECK(ok);
  return IVS_TEST_PASS;
}

/* e: a write of $0021 in the register window of frame 1 selects Foreground/Background, and frame 1 follows it: frame 0 is Color Stack (the read
 * and the strobe) and frame 1 is Foreground/Background. The guest waits VCS_WAIT iterations of DECR and BNZE between the strobe of frame 0 and
 * the write, derived below. */
#define VCS_WAIT 960u
static int vcs_mode_write_next_frame(void)
{
  const vf_frame *f;
  vp_run r;
  uint16_t w[SC_PROG_MAX];
  size_t n;
  int ok;
  VC_CHECK(vcs_mode_expectations(&f) == IVS_TEST_PASS);
  n = sc_w_mvi(w, 0u, 0x0021u, 1u); /* 2 words: Color Stack */
  n = sc_w_mvo(w, n, 0u, 0x0020u);  /* 2 words: the strobe of frame 0 */
  n = sc_w_delay(w, n, VCS_WAIT);   /* 5 words: wait into the register window of frame 1 */
  n = sc_w_mvo(w, n, 0u, 0x0021u);  /* 2 words: Foreground/Background again */
  n = sc_w_mvo(w, n, 0u, 0x0020u);  /* 2 words: the strobe of frame 1 */
  w[n++] = 0x0003u;                 /* DIS */
  n = sc_w_loop(w, n);              /* 14 words in all */
  VC_CHECK(vcs_mode_run(&r, w, n, f, 2u * VCS_E0 + 200u) == IVS_TEST_PASS);
  ok = r.sink.video_frames == 2u && vp_im(&r)->stic.mode == 0u && vp_im(&r)->stic.blocked_writes == 0u && vcs_slot_is(&r, 0u, vcs_exp_cs) &&
       vcs_slot_is(&r, 1u, vcs_exp_fg);
  vp_free(&r);
  VC_CHECK(ok);
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_video_cstack_cases[] = {
    {"video.cstack.F-B1", vcs_unit_b1, NULL},
    {"video.cstack.F-B2", vcs_unit_b2, NULL},
    {"video.cstack.F-B3", vcs_unit_b3, NULL},
    {"video.cstack.machine-F-B1", vcs_machine_b1, NULL},
    {"video.cstack.machine-F-B3", vcs_machine_b3, NULL},
    {"video.cstack.advance-before-draw", vcs_advance_before_draw, NULL},
    {"video.cstack.advance-wrap", vcs_advance_wrap, NULL},
    {"video.cstack.one-stack-across-rows", vcs_one_stack_across_rows, NULL},
    {"video.cstack.cutoff-column-advances", vcs_cutoff_column_advances, NULL},
    {"video.cstack.reset-per-frame", vcs_reset_per_frame, NULL},
    {"video.cstack.squares-quadrants", vcs_squares_quadrants, NULL},
    {"video.cstack.squares-no-advance", vcs_squares_no_advance, NULL},
    {"video.cstack.squares-stack7", vcs_squares_stack7, NULL},
    {"video.mode.switch-end-to-end", vcs_mode_switch_end_to_end, NULL},
    {"video.mode.write-selects-next-frame", vcs_mode_write_next_frame, NULL},
    {"video.judge.cstack-persistent-pointer", vcs_judge_persistent_pointer, NULL},
};
const size_t ivs_video_cstack_count = sizeof ivs_video_cstack_cases / sizeof ivs_video_cstack_cases[0];
