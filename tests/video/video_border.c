/* SPDX-License-Identifier: MIT */
/* Suite video, family border (plan 08-06, VID-05, D-02, D-03, D-10): the border-extension strips of core/src/stic/stic_frame.c held to the
 * frozen oracle of plan 08-03. The expected pictures are the hand-derived frame literal F-M4 (data/video/frames, derived from the rules before any
 * renderer existed, frozen by the blind second read, never produced by a run) and counts and rectangles derived by hand in the comments below from
 * rule STIC-R48 (derivation section (b)): extension register bit 0 paints the plane columns 0 to 7 of every row, bit 1 the plane rows 0 to 7 of
 * every column, with the border color of register $002C, after cards and MOBs, so everything under a strip is hidden; the cut-off column of the
 * working plane is never painted. The scenes are built by the shared moving-object scene of video_mob.c (vm_scene, vm_put, vm_render); the register
 * words are typed here from the scene table (A7) of docs/video/diag-stic-2-derivation.md, never read from a core table or from a run.
 *
 * The pinned remainder (the top strip unit PD-8, whether the strips follow a delay PD-9) is asserted by the default-pinned ids at the end of this
 * file: each asserts the default and then flips ONE field of a stack copy of ivs_stic_params and requires a different plane. */
#include <stdint.h>
#include <string.h>

#include "video_cases.h"

#define VB_EXT_REG 0x32u
#define VB_BORDER_REG 0x2Cu

/* The scene of F-M0 (the cards: color 1 everywhere, the 5 x 5 block of color 2 at x 80..84, y 48..52; border 8, stack 1 2 3 4, Color Stack mode)
 * with the extension register ext and the border register bord, no MOB. The parameters are returned. */
static const ivs_stic_params *vb_scene0(uint32_t ext, uint32_t bord)
{
  const ivs_stic_params *p = vm_scene(vm_frame_get(0u));
  ivs_memsys *m = vm_mem_get();
  m->stic_reg[VB_EXT_REG] = (uint16_t)ext;
  m->stic_reg[VB_BORDER_REG] = (uint16_t)bord;
  return p;
}

static uint8_t vb_at(uint32_t x, uint32_t y)
{
  return vm_plane_buf()[y * VF_PLANE_W + x];
}

/* The positions of the published columns (x below 159) of the rendered plane that hold color col. */
static uint32_t vb_count(uint32_t col)
{
  return vm_count(col);
}

/* ---- video.border.strips-left-top: scene 4 of the program (both strips, one MOB under the corner) is the frozen literal F-M4 ---- */

static int vb_strips_left_top(void)
{
  const vm_frame *f = vm_frame_get(4u);
  const ivs_stic_params *p;
  uint8_t *base = vm_exp_buf();
  uint32_t x;
  uint32_t y;
  uint32_t diff = 0u;
  VC_CHECK(f != NULL && vm_fixture_ok(f));
  p = vm_scene(f);
  VC_CHECK(p != NULL);
  /* derivation (a) A7, scene 4: MOB0 X 060C (visible, X coordinate 12), Y 010C (Y size code 01, Y coordinate 12), attribute 080C (graphics RAM
   * card 1, the staircase, color 4); the extension register $0032 is 0003 (both strips); the border register is 8 (A3). */
  vm_put(0u, 0x060Cu, 0x010Cu, 0x080Cu);
  vm_mem_get()->stic_reg[VB_EXT_REG] = 0x0003u;
  VC_CHECK(vm_render(p, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
  VC_CHECK(vm_matches(f)); /* every published position and the hash of the literal */
  /* the pixel probes of derivation (e) for F-M4, typed here: the strips are 8, the MOB shows only outside them, the cards are color 1 */
  VC_CHECK(vb_at(0u, 0u) == 8u && vb_at(100u, 3u) == 8u && vb_at(7u, 50u) == 8u);
  VC_CHECK(vb_at(8u, 8u) == 4u && vb_at(12u, 9u) == 4u && vb_at(8u, 12u) == 1u && vb_at(18u, 10u) == 1u && vb_at(8u, 50u) == 1u);
  /* the declared pair F-M0 F-M4 (derivation (e)): the top strip changes 159 x 8 = 1,272 positions, the left strip 8 x 88 = 704 and the MOB adds its
   * 36 visible positions outside both strips, 2,012 in all; the same scene without the MOB and the strips is F-M0 */
  memcpy(base, vm_plane_buf(), VF_PLANE_BYTES);
  p = vb_scene0(0u, 8u);
  VC_CHECK(p != NULL && vm_render(p, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
  for (y = 0u; y < VF_PLANE_H; y++) {
    for (x = 0u; x < VF_PLANE_W - 1u; x++) {
      diff += vm_plane_buf()[y * VF_PLANE_W + x] != base[y * VF_PLANE_W + x] ? 1u : 0u;
    }
  }
  VC_CHECK(diff == 1272u + 704u + 36u);
  return IVS_TEST_PASS;
}

/* ---- video.border.over-cards-and-mobs: the strips hide card pixels and MOB pixels alike (R48), and only the strips change ---- */

/* The scene: four cells of the solid graphics RAM card 0 in foreground color 5 (Color Stack word 0805: graphics RAM, card 0, foreground 5) at the
 * card cells (0, 0), (3, 0), (0, 5) and (3, 5) (the blocks x 0..7 y 0..7, x 0..7 y 24..31, x 40..47 y 0..7 and x 40..47 y 24..31, 64 positions each);
 * MOB0, a solid color 4 MOB (X 12, Y 40: pattern pixels at x 4..11, y 32..39) and MOB1, a solid color 6 MOB (X 60, Y 12: x 52..59, y 4..11). */
static const ivs_stic_params *vb_scene_cards_mobs(uint32_t ext)
{
  const ivs_stic_params *p = vb_scene0(ext, 8u);
  ivs_stic_state *s = vm_state_get();
  s->row_latch[0][0] = 0x0805u;
  s->row_latch[3][0] = 0x0805u;
  s->row_latch[0][5] = 0x0805u;
  s->row_latch[3][5] = 0x0805u;
  vm_put(0u, 0x0200u + 12u, 0x0100u + 40u, 0x0804u);
  vm_put(1u, 0x0200u + 60u, 0x0100u + 12u, 0x0806u);
  return p;
}

static int vb_over_cards_and_mobs(void)
{
  const ivs_stic_params *p;
  uint8_t *none = vm_exp_buf();
  uint32_t x;
  uint32_t y;
  /* without strips every block and every MOB shows: color 5 over 4 blocks = 256, color 4 over 8 x 8 = 64, color 6 over 64, the cell of F-M0 (25) */
  p = vb_scene_cards_mobs(0u);
  VC_CHECK(p != NULL && vm_render(p, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
  VC_CHECK(vb_count(5u) == 256u && vb_count(4u) == 64u && vb_count(6u) == 64u && vb_count(2u) == 25u && vb_count(8u) == 0u);
  memcpy(none, vm_plane_buf(), VF_PLANE_BYTES);
  /* with both strips: the left strip takes x 0..7 of every row and the top strip y 0..7 of every column, so of the card blocks only the one at
   * x 40..47, y 24..31 stays (64 positions of color 5; the other three lie under a strip), of MOB0 only x 8..11 of y 32..39 stays (4 x 8 = 32
   * positions of color 4) and of MOB1 only y 8..11 of x 52..59 stays (8 x 4 = 32 positions of color 6); the cell of F-M0 is outside both strips */
  p = vb_scene_cards_mobs(3u);
  VC_CHECK(p != NULL && vm_render(p, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
  VC_CHECK(vb_count(5u) == 64u && vb_count(4u) == 32u && vb_count(6u) == 32u && vb_count(2u) == 25u);
  VC_CHECK(vb_at(40u, 24u) == 5u && vb_at(47u, 31u) == 5u && vb_at(8u, 32u) == 4u && vb_at(11u, 39u) == 4u && vb_at(7u, 32u) == 8u && vb_at(52u, 8u) == 6u &&
           vb_at(59u, 11u) == 6u && vb_at(52u, 7u) == 8u && vb_at(0u, 24u) == 8u && vb_at(40u, 0u) == 8u);
  /* and nothing else moved: every position outside the strips equals the picture without strips, every position inside is the border color */
  for (y = 0u; y < VF_PLANE_H; y++) {
    for (x = 0u; x < VF_PLANE_W - 1u; x++) {
      uint8_t want = (x < 8u || y < 8u) ? 8u : none[y * VF_PLANE_W + x];
      if (vb_at(x, y) != want) {
        ivs_test_diag("position (%u,%u) is %u, expected %u", (unsigned)x, (unsigned)y, (unsigned)vb_at(x, y), (unsigned)want);
        return IVS_TEST_FAIL;
      }
    }
  }
  return IVS_TEST_PASS;
}

/* ---- video.border.colour-from-reg-2c: the strip color is the border color register masked to 4 bits ---- */

/* Register $002C as the guest wrote it (the stored word) and the color index of the strips: the low 4 bits (the register file keeps more bits than
 * the color; every other color source of the scene is a different register, the color stack 1 2 3 4). */
typedef struct vb_colour_row {
  uint32_t reg;
  uint32_t colour;
} vb_colour_row;

static const vb_colour_row vb_colours[] = {
    {0x0000u, 0u},  {0x0001u, 1u},  {0x0002u, 2u},  {0x0003u, 3u},  {0x0004u, 4u},  {0x0005u, 5u}, {0x0006u, 6u}, {0x0007u, 7u},
    {0x0008u, 8u},  {0x0009u, 9u},  {0x000Au, 10u}, {0x000Bu, 11u}, {0x000Cu, 12u}, {0x000Du, 13u}, {0x000Eu, 14u}, {0x000Fu, 15u},
    {0x00F5u, 5u},  {0x0010u, 0u},  {0x1234u, 4u},  {0xFFF3u, 3u},
};

static int vb_colour_from_reg(void)
{
  uint32_t t;
  uint32_t x;
  uint32_t y;
  for (t = 0u; t < sizeof vb_colours / sizeof vb_colours[0]; t++) {
    const vb_colour_row *r = &vb_colours[t];
    const ivs_stic_params *p = vb_scene0(3u, r->reg);
    VC_CHECK(p != NULL && vm_render(p, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
    for (y = 0u; y < VF_PLANE_H; y++) {
      for (x = 0u; x < VF_PLANE_W - 1u; x++) {
        if ((x < 8u || y < 8u) && vb_at(x, y) != r->colour) {
          ivs_test_diag("border register %04X: strip position (%u,%u) is %u, expected %u", (unsigned)r->reg, (unsigned)x, (unsigned)y,
                        (unsigned)vb_at(x, y), (unsigned)r->colour);
          return IVS_TEST_FAIL;
        }
      }
    }
    /* the picture outside the strips is the cards of F-M0 whatever the border color */
    VC_CHECK(vb_at(8u, 8u) == 1u && vb_at(80u, 48u) == 2u && vb_at(158u, 95u) == 1u);
  }
  /* the strips take the same color in Foreground/Background mode (the strips are not a card feature) */
  {
    const ivs_stic_params *p = vb_scene0(3u, 0x000Au);
    vm_state_get()->mode = 0u;
    VC_CHECK(p != NULL && vm_render(p, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
    VC_CHECK(vb_at(0u, 0u) == 10u && vb_at(158u, 0u) == 10u && vb_at(0u, 95u) == 10u && vb_at(7u, 50u) == 10u && vb_at(100u, 7u) == 10u);
  }
  /* a frame with the display off is the border fill, with or without the extension bits: no strip is a different color */
  {
    const ivs_stic_params *p = vb_scene0(3u, 0x000Au);
    VC_CHECK(p != NULL && vm_render(p, 0u, vm_plane_buf()) == VF_PLANE_BYTES);
    VC_CHECK(vb_count(10u) == 159u * 96u);
  }
  return IVS_TEST_PASS;
}

/* ---- video.border.ext-bits-independent: bit 0 paints the left strip, bit 1 the top strip, each alone, and no other bit of the register acts ---- */

/* The register value (a raw poke of the register file, so the bits above the write mask are exercised too), the number of strip positions in the
 * published 159 x 96 plane, and which strips are expected. Derived by hand from R48: the left strip is 8 columns of every row (8 x 96 = 768
 * positions), the top strip 8 rows of every column (159 x 8 = 1,272), both together 768 + 1,272 - 8 x 8 = 1,976 (the corner is counted once). */
typedef struct vb_ext_row {
  uint32_t reg;
  uint32_t left;
  uint32_t top;
  uint32_t count;
} vb_ext_row;

static const vb_ext_row vb_ext_rows[] = {
    {0x0000u, 0u, 0u, 0u},    {0x0001u, 1u, 0u, 768u},  {0x0002u, 0u, 1u, 1272u}, {0x0003u, 1u, 1u, 1976u}, {0x0004u, 0u, 0u, 0u},
    {0x0005u, 1u, 0u, 768u},  {0x0006u, 0u, 1u, 1272u}, {0xFFFCu, 0u, 0u, 0u},      {0xFFFFu, 1u, 1u, 1976u},
};

static int vb_ext_bits_independent(void)
{
  uint32_t t;
  uint32_t x;
  uint32_t y;
  for (t = 0u; t < sizeof vb_ext_rows / sizeof vb_ext_rows[0]; t++) {
    const vb_ext_row *r = &vb_ext_rows[t];
    const ivs_stic_params *p = vb_scene0(r->reg, 8u); /* the cards of F-M0 use the colors 1 and 2, so every position of color 8 is a strip position */
    VC_CHECK(p != NULL && vm_render(p, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
    VC_CHECK(vb_count(8u) == r->count);
    for (y = 0u; y < VF_PLANE_H; y++) {
      for (x = 0u; x < VF_PLANE_W - 1u; x++) {
        uint32_t in_strip = ((r->left != 0u && x < 8u) || (r->top != 0u && y < 8u)) ? 1u : 0u;
        if ((vb_at(x, y) == 8u) != (in_strip != 0u)) {
          ivs_test_diag("extension register %04X: position (%u,%u) is %u, a strip position is %s", (unsigned)r->reg, (unsigned)x, (unsigned)y,
                        (unsigned)vb_at(x, y), in_strip != 0u ? "painted" : "not painted");
          return IVS_TEST_FAIL;
        }
      }
    }
    /* the cut-off column is never part of a strip: the working plane keeps its not-displayed marker in column 159 of every row */
    for (y = 0u; y < VF_PLANE_H; y++) {
      VC_CHECK(vb_at(159u, y) == 0xFFu);
    }
  }
  return IVS_TEST_PASS;
}

/* ---- the pinned defaults (D-02, T-08-17): each test asserts the default and then changes ONE field of a stack copy of ivs_stic_params and requires a
 * different plane, so no default can pass as a confirmed behavior. The generator refuses a value other than the ones exercised here. ---- */

/* video.border.default-pinned.top-strip-unit: the top strip is 8 plane rows (R49, C109, field video.border_ext_strip, resolver D10-VID-13). The default
 * reproduces F-M4 (both strips, rows 0 to 7 painted). With the unit changed to 4 rows in a copy, the rows 4 to 7 of every column are no longer painted:
 * of the 159 x 4 = 636 positions of those rows, the ones outside the left strip (x 8 to 158, 151 columns) show the card layer, 151 x 4 = 604 positions
 * of color 1 (the MOB of F-M4 shows only from the row 8 down), and the left strip still holds its 8 columns. */
static int vb_pin_top_strip_unit(void)
{
  const vm_frame *f = vm_frame_get(4u);
  const ivs_stic_params *p;
  ivs_stic_params q;
  uint8_t *base = vm_exp_buf();
  uint32_t x;
  uint32_t y;
  VC_CHECK(f != NULL && vm_fixture_ok(f));
  p = vm_scene(f);
  VC_CHECK(p != NULL && p->video_border_ext_rows == 8u && p->video_border_ext_cols == 8u);
  vm_put(0u, 0x060Cu, 0x010Cu, 0x080Cu);
  vm_mem_get()->stic_reg[VB_EXT_REG] = 0x0003u;
  VC_CHECK(vm_render(p, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
  VC_CHECK(vm_matches(f)); /* the default: the top strip is 8 rows, the literal F-M4 */
  memcpy(base, vm_plane_buf(), VF_PLANE_BYTES);
  q = *p;
  q.video_border_ext_rows = 4u; /* the one-field flip: a top strip of 4 plane rows (8 television lines) */
  VC_CHECK(vm_render(&q, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
  VC_CHECK(vm_planes_differ(vm_plane_buf(), base));
  for (y = 0u; y < VF_PLANE_H; y++) {
    for (x = 0u; x < VF_PLANE_W - 1u; x++) {
      uint32_t strip = (x < 8u || y < 4u) ? 1u : 0u;
      if (strip != 0u && vb_at(x, y) != 8u) {
        ivs_test_diag("top strip of 4 rows: strip position (%u,%u) is %u, expected 8", (unsigned)x, (unsigned)y, (unsigned)vb_at(x, y));
        return IVS_TEST_FAIL;
      }
    }
  }
  VC_CHECK(vb_at(8u, 4u) == 1u && vb_at(100u, 7u) == 1u && vb_at(158u, 4u) == 1u && vb_at(100u, 3u) == 8u); /* rows 4 to 7 show the card layer now */
  return IVS_TEST_PASS;
}

/* video.border.default-pinned.follows-delay: the strips are fixed to the display (R49, C109, field video.border_follows_delay, resolver D10-VID-13). The
 * default reproduces F-M5 (horizontal delay 3: the strip is the columns 0 to 7) and F-M6 (vertical delay 3: the strip is the rows 0 to 7). With the field
 * flipped in a copy the strips are painted on the undelayed picture and move with it: under the horizontal delay 3 the left strip covers the columns 3
 * to 10 and the columns 0 to 2 are exposed (the not-displayed value, in every row, also the rows of the top strip); under the vertical delay 3 the top
 * strip covers the rows 3 to 10 and the rows 0 to 2 are exposed. The probes were derived by hand from R46 and R49. */
static int vb_pin_follows_delay(void)
{
  uint32_t k;
  for (k = 5u; k <= 6u; k++) {
    const vm_frame *f = vm_frame_get(k);
    const ivs_stic_params *p;
    ivs_stic_params q;
    uint8_t *base = vm_exp_buf();
    VC_CHECK(f != NULL && vm_fixture_ok(f));
    p = vm_scene(f);
    VC_CHECK(p != NULL && p->video_border_follows_delay == 0u);
    vm_put(0u, 0x0644u, 0x0130u, 0x080Cu); /* scene 5 and scene 6 of the program: MOB0 0644 0130 080C */
    vm_mem_get()->stic_reg[0x30u] = (uint16_t)(k == 5u ? 3u : 0u);
    vm_mem_get()->stic_reg[0x31u] = (uint16_t)(k == 6u ? 3u : 0u);
    vm_mem_get()->stic_reg[VB_EXT_REG] = 0x0003u;
    VC_CHECK(vm_render(p, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
    VC_CHECK(vm_matches(f)); /* the default: the literal, strips on the display */
    memcpy(base, vm_plane_buf(), VF_PLANE_BYTES);
    q = *p;
    q.video_border_follows_delay = 1u; /* the one-field flip */
    VC_CHECK(vm_render(&q, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
    VC_CHECK(vm_planes_differ(vm_plane_buf(), base));
    if (k == 5u) {
      /* the left strip is x 3..10 now, rows 0..95; the top strip rows 0..7 is shifted too, so x 0..2 of every row is exposed */
      VC_CHECK(vb_at(0u, 50u) == 0xFFu && vb_at(2u, 50u) == 0xFFu && vb_at(3u, 50u) == 8u && vb_at(10u, 50u) == 8u && vb_at(11u, 50u) == 1u);
      VC_CHECK(vb_at(0u, 3u) == 0xFFu && vb_at(3u, 3u) == 8u && vb_at(100u, 3u) == 8u && vb_at(100u, 8u) == 1u);
      VC_CHECK(base[0] == 8u && vb_at(0u, 0u) == 0xFFu); /* the corner (0, 0) is the strip under the default and exposed under the flip */
    } else {
      /* the top strip is y 3..10 now, every column; the left strip rows 0..7 is shifted too, so y 0..2 of every column is exposed */
      VC_CHECK(vb_at(50u, 0u) == 0xFFu && vb_at(50u, 2u) == 0xFFu && vb_at(50u, 3u) == 8u && vb_at(50u, 10u) == 8u && vb_at(50u, 11u) == 1u);
      VC_CHECK(vb_at(0u, 0u) == 0xFFu && vb_at(0u, 20u) == 8u && vb_at(7u, 20u) == 8u && vb_at(8u, 20u) == 1u);
    }
  }
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_video_border_cases[] = {
    {"video.border.strips-left-top", vb_strips_left_top, NULL},
    {"video.border.over-cards-and-mobs", vb_over_cards_and_mobs, NULL},
    {"video.border.colour-from-reg-2c", vb_colour_from_reg, NULL},
    {"video.border.ext-bits-independent", vb_ext_bits_independent, NULL},
    {"video.border.default-pinned.top-strip-unit", vb_pin_top_strip_unit, NULL},
    {"video.border.default-pinned.follows-delay", vb_pin_follows_delay, NULL},
};
const size_t ivs_video_border_count = sizeof ivs_video_border_cases / sizeof ivs_video_border_cases[0];
