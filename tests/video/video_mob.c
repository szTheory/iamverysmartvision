/* SPDX-License-Identifier: MIT */
/* Suite video, family mob (plan 08-04, VID-05, D-01, D-02, D-03, D-05): the moving-object renderer of core/src/stic/stic_mob.c held to the
 * frozen oracle of plan 08-03. The expected pictures are the ten hand-derived frame literals F-M0..F-M9 (data/video/frames, derived from the
 * rules before any renderer existed, frozen by the blind second read, never produced by a run), generated into ivs_mob_frames.inc by
 * cmake/IvsVideoFrames.cmake with FRAMESET=mob and kept apart from the card frames of video_fgbg.c. The register words of every scene are
 * typed here, by hand, from the scene table (A7) and the per-event table of docs/video/diag-stic-2-derivation.md (sections (a) and (c)); no
 * word and no expected pixel is read from a core table or from a run. The three graphics RAM cards of the program (derivation A2: solid,
 * staircase, 5 x 5 block) are typed here too (the frame literals list only the block, card 2, which the card cell uses).
 *
 * A scene is built on a fresh memory system (vm_scene): the graphics RAM cards, the 240 BACKTAB words of the frame literal into the row
 * latches of a STIC state, the color stack and the border registers, then the MOB registers of the scene; the plane is rendered by
 * ivs_stic_render_plane (the whole-plane entry of the one composition path) and compared position by position with the literal, a `-`
 * position (the cut-off pixel, or a position inside an unknown rectangle) being skipped. The hash of the 159 published columns of a hashed
 * literal is compared too. The generated include also carries the three profile values of the moving-object pins, read by the generator's own
 * code (video.mob.params-match-profile). */
#include <stdint.h>
#include <string.h>

#include "video_cases.h"
#include "ivs_mob_frames.inc"

#define VM_FIRST_MOB_REG 0u
#define VM_REG_Y 8u
#define VM_REG_A 16u

static ivs_memsys vm_mem;
static ivs_stic_state vm_state;
static uint8_t vm_plane[VF_PLANE_BYTES];
static uint8_t vm_exp[VF_PLANE_BYTES];

/* The shared scene of the families that hold the moving-object frames (plan 08-06: video_border.c and video_delay.c use the same scene, the
 * same renderer entry and the same comparison against F-M4 to F-M7): the frame literal i, and the static memory system, STIC state, rendered
 * plane and expected plane. */
const vm_frame *vm_frame_get(uint32_t i)
{
  return i < VM_FRAMES ? &vm_frames[i] : NULL;
}
ivs_memsys *vm_mem_get(void)
{
  return &vm_mem;
}
ivs_stic_state *vm_state_get(void)
{
  return &vm_state;
}
uint8_t *vm_plane_buf(void)
{
  return vm_plane;
}
uint8_t *vm_exp_buf(void)
{
  return vm_exp;
}

/* The three program cards of derivation A2 (a card is 8 pattern bytes, byte y is pixel row y from the top, bit 7 the leftmost pixel). */
static const uint8_t vm_card_solid[8] = {0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu};
static const uint8_t vm_card_stairs[8] = {0x80u, 0xC0u, 0xE0u, 0xF0u, 0xF8u, 0xFCu, 0xFEu, 0xFFu};
/* The pair of the tall MOB (derivation section (e), F-M9): card 4 is the staircase, card 5 its right-aligned mirror (row r has its r + 1
 * rightmost pixels set). They are poked into every scene; no background card uses them. */
static const uint8_t vm_card_mirror[8] = {0x01u, 0x03u, 0x07u, 0x0Fu, 0x1Fu, 0x3Fu, 0x7Fu, 0xFFu};

static uint8_t vm_symbol(char ch)
{
  if (ch == '-') {
    return 0xFFu;
  }
  if (ch >= '0' && ch <= '9') {
    return (uint8_t)(ch - '0');
  }
  return (uint8_t)(10 + (ch - 'A'));
}

/* The expected 160 x 96 plane of a literal (the wildcard `-` is 0xFF). */
void vm_expected(const vm_frame *f, uint8_t *exp)
{
  uint32_t y;
  uint32_t x;
  for (y = 0u; y < VF_PLANE_H; y++) {
    for (x = 0u; x < VF_PLANE_W; x++) {
      exp[y * VF_PLANE_W + x] = vm_symbol(f->plane[y][x]);
    }
  }
}

/* The scene of literal f with every MOB register at its reset value: graphics RAM cards 0 and 1 of the program and the cards of the
 * literal, the BACKTAB words into the row latches, the mode, the color stack and the border. The parameters of the profile are returned. */
const ivs_stic_params *vm_scene(const vm_frame *f)
{
  uint32_t i;
  uint32_t k;
  (void)ivs_memsys_init(&vm_mem, SC_PROFILE);
  vf_poke_gram_card(&vm_mem, 0u, vm_card_solid);
  vf_poke_gram_card(&vm_mem, 1u, vm_card_stairs);
  vf_poke_gram_card(&vm_mem, 4u, vm_card_stairs);
  vf_poke_gram_card(&vm_mem, 5u, vm_card_mirror);
  for (i = 0u; i < f->n_gram; i++) {
    vf_poke_gram_card(&vm_mem, f->gram[i].card, f->gram[i].b);
  }
  memset(&vm_state, 0, sizeof vm_state);
  vm_state.mode = f->mode;
  for (i = 0u; i < 12u * 20u; i++) {
    vm_state.row_latch[i / 20u][i % 20u] = (uint16_t)f->fill;
  }
  for (i = 0u; i < f->n_cells; i++) {
    vm_state.row_latch[f->cells[i].r][f->cells[i].c] = (uint16_t)f->cells[i].word;
  }
  for (k = 0u; k < 4u; k++) {
    vm_mem.stic_reg[0x28u + k] = (uint16_t)f->stack[k];
  }
  vm_mem.stic_reg[0x2Cu] = (uint16_t)f->border;
  return ivs_stic_params_get(SC_PROFILE);
}

/* Programs MOB i (X word, Y word, attribute word of the derivation's scene table). */
void vm_put(uint32_t i, uint32_t xw, uint32_t yw, uint32_t aw)
{
  vm_mem.stic_reg[VM_FIRST_MOB_REG + i] = (uint16_t)xw;
  vm_mem.stic_reg[VM_REG_Y + i] = (uint16_t)yw;
  vm_mem.stic_reg[VM_REG_A + i] = (uint16_t)aw;
}

uint32_t vm_render(const ivs_stic_params *p, uint32_t displayed, uint8_t *plane)
{
  return ivs_stic_render_plane(p, &vm_state, &vm_mem, displayed, plane);
}

/* The index of the first published position (x below 159) at which got differs from the literal plane exp, a `-` position being skipped,
 * or -1; *skipped counts the skipped positions of the published columns. */
long vm_first_diff(const uint8_t *got, const uint8_t *exp, uint32_t *skipped)
{
  uint32_t y;
  uint32_t x;
  *skipped = 0u;
  for (y = 0u; y < VF_PLANE_H; y++) {
    for (x = 0u; x < VF_PLANE_W - 1u; x++) {
      uint8_t e = exp[y * VF_PLANE_W + x];
      if (e == 0xFFu) {
        *skipped += 1u;
        continue;
      }
      if (got[y * VF_PLANE_W + x] != e) {
        return (long)(y * (VF_PLANE_W - 1u) + x);
      }
    }
  }
  return -1;
}

static void vm_report(const vm_frame *f, const uint8_t *got, const uint8_t *exp, long idx)
{
  uint32_t y = (uint32_t)idx / (VF_PLANE_W - 1u);
  uint32_t x = (uint32_t)idx % (VF_PLANE_W - 1u);
  ivs_test_diag("%s (literal plane at line %u): first difference at position (%u,%u): got %u, expected %u", f->name, (unsigned)f->plane_line,
                (unsigned)x, (unsigned)y, (unsigned)got[y * VF_PLANE_W + x], (unsigned)exp[y * VF_PLANE_W + x]);
}

/* The fixture checks of a literal before it judges anything: a hashed literal has no wildcard in the published columns and its plane hashes to
 * its own sha256-text; a literal with `unknown` rectangles has a wildcard and no hash. 1 when consistent (a failure is FIXTURE, not a renderer). */
int vm_fixture_ok(const vm_frame *f)
{
  char hex[65];
  uint32_t y;
  uint32_t x;
  uint32_t wild = 0u;
  vm_expected(f, vm_exp);
  for (y = 0u; y < VF_PLANE_H; y++) {
    for (x = 0u; x < VF_PLANE_W - 1u; x++) {
      wild += vm_exp[y * VF_PLANE_W + x] == 0xFFu ? 1u : 0u;
    }
  }
  if ((f->n_unknown == 0u) != (wild == 0u) || (f->sha_text[0] == '\0') != (f->n_unknown != 0u)) {
    ivs_test_diag("FIXTURE: %s: %u unknown rectangles, %u wildcard positions, hash text '%s'", f->name, (unsigned)f->n_unknown, (unsigned)wild,
                  f->sha_text);
    return 0;
  }
  if (f->sha_text[0] != '\0') {
    vf_text_sha(vm_exp, VF_PLANE_W, VF_PLANE_W - 1u, hex);
    if (strcmp(hex, f->sha_text) != 0) {
      ivs_test_diag("FIXTURE: %s: the literal plane hashes to %s, not its sha256-text %s", f->name, hex, f->sha_text);
      return 0;
    }
  }
  return 1;
}

/* Compares the rendered plane in vm_plane with literal f (positions, then the hash of the published columns when the literal has one). */
int vm_matches(const vm_frame *f)
{
  uint32_t skipped = 0u;
  long d = vm_first_diff(vm_plane, vm_exp, &skipped);
  if (d >= 0) {
    vm_report(f, vm_plane, vm_exp, d);
    return 0;
  }
  if (f->sha_text[0] != '\0') {
    char hex[65];
    vf_text_sha(vm_plane, VF_PLANE_W, VF_PLANE_W - 1u, hex);
    if (strcmp(hex, f->sha_text) != 0) {
      ivs_test_diag("%s: the rendered plane hashes to %s, the literal says %s", f->name, hex, f->sha_text);
      return 0;
    }
  }
  return 1;
}

/* ---- video.mob.base-neutral: with every MOB register at its reset value the composition is the card renderer and nothing else ---- */

/* The card frame f of the card families (F-A, F-B) rendered by the card renderer alone, then by the composition path with the registers of the
 * frame and every MOB register zero: equal byte for byte over the whole 160 x 96 working plane. */
static int vm_neutral_card_frame(uint32_t i)
{
  const vf_frame *f = vf_frame_get(i);
  ivs_stic_state st;
  uint8_t *want = vm_exp;
  uint32_t r;
  uint32_t c;
  uint32_t k;
  uint16_t words[240];
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  VC_CHECK(f != NULL && p != NULL);
  vf_render_frame(f, want); /* the card renderer alone: vf_mem() now holds the cards of the frame */
  memset(&st, 0, sizeof st);
  st.mode = f->mode;
  vf_words(f, words);
  for (r = 0u; r < 12u; r++) {
    for (c = 0u; c < 20u; c++) {
      st.row_latch[r][c] = words[20u * r + c];
    }
  }
  for (k = 0u; k < 4u; k++) {
    vf_mem()->stic_reg[0x28u + k] = (uint16_t)f->stack[k];
  }
  for (k = 0u; k < 24u; k++) {
    VC_CHECK(vf_mem()->stic_reg[k] == 0u); /* every MOB register is at its reset value */
  }
  VC_CHECK(ivs_stic_render_plane(p, &st, vf_mem(), 1u, vm_plane) == VF_PLANE_BYTES);
  if (memcmp(vm_plane, want, VF_PLANE_BYTES) != 0) {
    ivs_test_diag("%s: the composition with every MOB register zero differs from the card renderer", f->name);
    return 0;
  }
  return 1;
}

static int vm_base_neutral(void)
{
  uint32_t i;
  uint32_t y;
  uint32_t x;
  uint32_t displayed;
  const vm_frame *f0 = &vm_frames[0];
  const ivs_stic_params *p;
  VC_CHECK(vf_frame_count() == 8u && sizeof vm_frames / sizeof vm_frames[0] == VM_FRAMES);
  for (i = 0u; i < vf_frame_count(); i++) {
    if (!vm_neutral_card_frame(i)) {
      return IVS_TEST_FAIL;
    }
  }
  /* F-M0 (the cards of the program, display on): the literal itself, with every MOB register zero. */
  VC_CHECK(vm_fixture_ok(f0));
  p = vm_scene(f0);
  VC_CHECK(p != NULL && vm_render(p, 1u, vm_plane) == VF_PLANE_BYTES);
  VC_CHECK(vm_matches(f0));
  /* A display-off frame is the border fill (the border register, 8 here) with every MOB register zero, and also with a MOB programmed: the
   * overlay never runs for a frame that is not displayed. */
  for (displayed = 0u; displayed < 2u; displayed++) {
    p = vm_scene(f0);
    if (displayed == 1u) {
      vm_put(0u, 0x069Eu, 0x0162u, 0x080Cu); /* the MOB of scene 1 (derivation A7) */
    }
    VC_CHECK(vm_render(p, 0u, vm_plane) == VF_PLANE_BYTES);
    for (y = 0u; y < VF_PLANE_H; y++) {
      for (x = 0u; x < VF_PLANE_W; x++) {
        if (vm_plane[y * VF_PLANE_W + x] != 8u) {
          ivs_test_diag("display off, MOB %s: position (%u,%u) is %u, the border fill is 8", displayed == 1u ? "programmed" : "reset", (unsigned)x,
                        (unsigned)y, (unsigned)vm_plane[y * VF_PLANE_W + x]);
          return IVS_TEST_FAIL;
        }
      }
    }
  }
  return IVS_TEST_PASS;
}

/* ---- video.mob.frame-placement-clip: one MOB at 2x clipped at the right and bottom edges (F-M1) ---- */

static int vm_frame_placement_clip(void)
{
  const vm_frame *f = &vm_frames[1];
  const ivs_stic_params *p;
  VC_CHECK(vm_fixture_ok(f));
  p = vm_scene(f);
  VC_CHECK(p != NULL);
  /* derivation (a) A7, scene 1: MOB0 X 069E (visible, double width, X coordinate 158), Y 0162 (Y size code 01, Y coordinate 98), attribute 080C
   * (graphics RAM card 1, color 4); section (c): this is the picture of frames 1 to 3 of the `mob` scenario. */
  vm_put(0u, 0x069Eu, 0x0162u, 0x080Cu);
  VC_CHECK(vm_render(p, 1u, vm_plane) == VF_PLANE_BYTES);
  VC_CHECK(vm_matches(f));
  return IVS_TEST_PASS;
}

/* ---- video.mob.field-origin-8-8: the object field starts 8 positions left of and above the plane (R37) ---- */

/* One MOB of the solid card at zoom 1 (X word 0200 plus X, Y word 0100 plus Y: Y size code 01, one plane row per pattern row), color 4, over the
 * cards of F-M0 (whose colors are 1 and 2): the positions of color 4 are counted and bounded. The rows are derived by hand from rule R37: pattern
 * pixel (c, r) at plane position (X - 8 + c, Y - 8 + r), clipped to x 0..158 and y 0..95, a MOB with X coordinate 0 disabled. */
typedef struct vm_origin_row {
  uint32_t x;
  uint32_t y;
  uint32_t count; /* positions of the MOB color; 0 means none */
  uint32_t x0;
  uint32_t y0;
  uint32_t x1;
  uint32_t y1;
  const char *why;
} vm_origin_row;

static const vm_origin_row vm_origin_rows[] = {
    {8u, 8u, 64u, 0u, 0u, 7u, 7u, "X 8 and Y 8 put the first pixel at the plane corner"},
    {9u, 9u, 64u, 1u, 1u, 8u, 8u, "one position right and down of the corner"},
    {1u, 1u, 1u, 0u, 0u, 0u, 0u, "X 1 and Y 1: only the last pixel of the last pattern row is on the plane (the corner case of R37)"},
    {0u, 8u, 0u, 0u, 0u, 0u, 0u, "X coordinate 0 disables the MOB"},
    {0u, 0u, 0u, 0u, 0u, 0u, 0u, "X coordinate 0 disables the MOB whatever the Y coordinate"},
    {8u, 1u, 8u, 0u, 0u, 7u, 0u, "Y 1: the last pattern row is the plane row 0"},
    {8u, 0u, 0u, 0u, 0u, 0u, 0u, "Y 0 puts the whole MOB above the plane (rows -8 to -1)"},
    {158u, 8u, 64u, 150u, 0u, 157u, 7u, "the MOB of the oracle: x 150 to 157, inside the plane"},
    {159u, 8u, 64u, 151u, 0u, 158u, 7u, "the last column of the plane is x 158: still whole"},
    {160u, 8u, 56u, 152u, 0u, 158u, 7u, "x 152 to 159, clipped at 158: seven columns"},
    {166u, 8u, 8u, 158u, 0u, 158u, 7u, "one column left on the plane"},
    {167u, 8u, 0u, 0u, 0u, 0u, 0u, "x 159 and beyond is not published"},
    {255u, 8u, 0u, 0u, 0u, 0u, 0u, "the largest X coordinate puts the MOB far right of the plane"},
    {8u, 103u, 8u, 0u, 95u, 7u, 95u, "Y 103: the first pattern row is the last plane row (y 95)"},
    {8u, 104u, 0u, 0u, 0u, 0u, 0u, "Y 104: below the plane"},
};

static int vm_field_origin(void)
{
  const vm_frame *f0 = &vm_frames[0];
  uint32_t t;
  for (t = 0u; t < sizeof vm_origin_rows / sizeof vm_origin_rows[0]; t++) {
    const vm_origin_row *r = &vm_origin_rows[t];
    const ivs_stic_params *p = vm_scene(f0);
    uint32_t count = 0u;
    uint32_t x0 = 999u;
    uint32_t y0 = 999u;
    uint32_t x1 = 0u;
    uint32_t y1 = 0u;
    uint32_t y;
    uint32_t x;
    VC_CHECK(p != NULL);
    vm_put(0u, 0x0200u + r->x, 0x0100u + r->y, 0x0804u); /* visible, size code 01, solid card 0 in graphics RAM, color 4 */
    VC_CHECK(vm_render(p, 1u, vm_plane) == VF_PLANE_BYTES);
    for (y = 0u; y < VF_PLANE_H; y++) {
      for (x = 0u; x < VF_PLANE_W - 1u; x++) {
        if (vm_plane[y * VF_PLANE_W + x] == 4u) {
          count += 1u;
          x0 = x < x0 ? x : x0;
          y0 = y < y0 ? y : y0;
          x1 = x > x1 ? x : x1;
          y1 = y > y1 ? y : y1;
        }
      }
    }
    if (count != r->count || (count != 0u && (x0 != r->x0 || y0 != r->y0 || x1 != r->x1 || y1 != r->y1))) {
      ivs_test_diag("X %u Y %u (%s): %u positions in (%u,%u)-(%u,%u), expected %u in (%u,%u)-(%u,%u)", (unsigned)r->x, (unsigned)r->y, r->why,
                    (unsigned)count, (unsigned)x0, (unsigned)y0, (unsigned)x1, (unsigned)y1, (unsigned)r->count, (unsigned)r->x0, (unsigned)r->y0,
                    (unsigned)r->x1, (unsigned)r->y1);
      return IVS_TEST_FAIL;
    }
  }
  return IVS_TEST_PASS;
}

/* ---- video.mob.params-match-profile: the three generated members equal the profile values read by the generator's own code ---- */

static int vm_params_match_profile(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  VC_CHECK(p != NULL);
  VC_CHECK(p->video_mob_origin_x == VM_PROFILE_ORIGIN_X);
  VC_CHECK(p->video_mob_origin_y == VM_PROFILE_ORIGIN_Y);
  VC_CHECK(p->video_mob_line_sample == VM_PROFILE_LINE_SAMPLE);
  VC_CHECK(p->video_mob_prio_hides == VM_PROFILE_PRIO_HIDES);
  /* the ranges the generator enforces */
  VC_CHECK(p->video_mob_origin_x <= 15u && p->video_mob_origin_y <= 15u);
  VC_CHECK(p->video_mob_line_sample < p->video_row_lines);
  VC_CHECK(p->video_mob_prio_hides <= 1u);
  return IVS_TEST_PASS;
}

/* ---- helpers of the shape tests ---- */

/* The positions of the published columns that hold color col in the rendered plane. */
uint32_t vm_count(uint32_t col)
{
  uint32_t y;
  uint32_t x;
  uint32_t n = 0u;
  for (y = 0u; y < VF_PLANE_H; y++) {
    for (x = 0u; x < VF_PLANE_W - 1u; x++) {
      n += vm_plane[y * VF_PLANE_W + x] == col ? 1u : 0u;
    }
  }
  return n;
}

/* 1 when the w x h block at (x0, y0) of the rendered plane reads as the text rows: `#` the MOB color col, `.` the background color 1, a hex digit
 * that color. */
static int vm_region_is(uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, const char *const *rows, uint32_t col, const char *what)
{
  uint32_t y;
  uint32_t x;
  for (y = 0u; y < h; y++) {
    for (x = 0u; x < w; x++) {
      char ch = rows[y][x];
      uint32_t want = ch == '#' ? col : (ch == '.' ? 1u : (uint32_t)vm_symbol(ch));
      uint32_t got = vm_plane[(y0 + y) * VF_PLANE_W + x0 + x];
      if (got != want) {
        ivs_test_diag("%s: position (%u,%u): got %u, expected %u", what, (unsigned)(x0 + x), (unsigned)(y0 + y), (unsigned)got, (unsigned)want);
        return 0;
      }
    }
  }
  return 1;
}

/* The scene of F-M0 (the cards: color 1 everywhere, the 5 x 5 block of color 2 at x 80..84, y 48..52) with MOB 0 programmed and rendered. */
static int vm_render_mob0(uint32_t xw, uint32_t yw, uint32_t aw)
{
  const ivs_stic_params *p = vm_scene(&vm_frames[0]);
  vm_put(0u, xw, yw, aw);
  return p != NULL && vm_render(p, 1u, vm_plane) == VF_PLANE_BYTES;
}

/* Color 4, graphics RAM card 1 (the staircase), card 4 (the staircase of the tall pair) and card 0 (solid): the attribute words of the scenes. */
#define VM_A_SOLID 0x0804u  /* graphics RAM, card 0, color 4 */
#define VM_A_STAIRS 0x080Cu /* graphics RAM, card 1, color 4 */
#define VM_A_TALL4 0x0824u  /* graphics RAM, card 4, color 4 */
#define VM_A_TALL5 0x082Cu  /* graphics RAM, card 5, color 4: in tall mode the card number bit 0 is ignored */

/* ---- video.mob.zoom-x2: the X size bit doubles the width (R38) ---- */

static int vm_zoom_x2(void)
{
  /* X 40 and Y 40 put the corner at (32, 32); Y word 0128 is the Y size code 01 (one plane row per pattern row). The staircase row r has its
   * r + 1 leftmost pattern pixels set; at 1x each is one column, at 2x two columns. */
  static const char *const x1[8] = {"#.......", "##......", "###.....", "####....", "#####...", "######..", "#######.", "########"};
  static const char *const x2[8] = {"##..............", "####............", "######..........", "########........",
                                    "##########......", "############....", "##############..", "################"};
  VC_CHECK(vm_render_mob0(0x0200u + 40u, 0x0100u + 40u, VM_A_STAIRS));
  VC_CHECK(vm_region_is(32u, 32u, 8u, 8u, x1, 4u, "1x"));
  VC_CHECK(vm_count(4u) == 36u);
  VC_CHECK(vm_render_mob0(0x0600u + 40u, 0x0100u + 40u, VM_A_STAIRS));
  VC_CHECK(vm_region_is(32u, 32u, 16u, 8u, x2, 4u, "2x"));
  VC_CHECK(vm_count(4u) == 72u);
  return IVS_TEST_PASS;
}

/* ---- video.mob.zoom-y2-y4-y8: the Y size codes 01, 10 and 11 give 1, 2 and 4 plane rows per pattern row (R38) ---- */

static int vm_zoom_y(void)
{
  static const char *const y4[16] = {"#.......", "#.......", "##......", "##......", "###.....", "###.....", "####....", "####....",
                                     "#####...", "#####...", "######..", "######..", "#######.", "#######.", "########", "########"};
  uint32_t code;
  uint32_t k;
  /* Y 16 puts the corner at (32, 8); the three codes cover 8, 16 and 32 plane rows. Row offset k shows pattern row k / p, which has k / p + 1
   * pixels. */
  for (code = 1u; code <= 3u; code++) {
    uint32_t p = code == 1u ? 1u : (code == 2u ? 2u : 4u);
    VC_CHECK(vm_render_mob0(0x0200u + 40u, 0x0100u * code + 16u, VM_A_STAIRS));
    for (k = 0u; k < 8u * p; k++) {
      uint32_t run = k / p + 1u;
      uint32_t x;
      for (x = 0u; x < 8u; x++) {
        uint32_t want = x < run ? 4u : 1u;
        if (vm_plane[(8u + k) * VF_PLANE_W + 32u + x] != want) {
          ivs_test_diag("code %u: position (%u,%u): got %u, expected %u", (unsigned)code, (unsigned)(32u + x), (unsigned)(8u + k),
                        (unsigned)vm_plane[(8u + k) * VF_PLANE_W + 32u + x], (unsigned)want);
          return IVS_TEST_FAIL;
        }
      }
    }
    VC_CHECK(vm_count(4u) == 36u * p); /* the zoom multiplies the set pixels and adds no stray one */
    VC_CHECK(vm_plane[(8u + 8u * p) * VF_PLANE_W + 32u] == 1u && vm_plane[(7u) * VF_PLANE_W + 32u] == 1u); /* the rows just outside */
  }
  /* an explicit table for the 4x code, so the loop above is not the only statement of the rule */
  VC_CHECK(vm_render_mob0(0x0200u + 40u, 0x0200u + 16u, VM_A_STAIRS));
  VC_CHECK(vm_region_is(32u, 8u, 8u, 16u, y4, 4u, "4x"));
  /* Y coordinate 0 is not special: with the code 11 and X 8 the MOB covers plane rows 0 to 23 (the field rows -8 to 23, rows 2 to 7 of the
   * pattern on the plane: 4 x (3 + 4 + 5 + 6 + 7 + 8) = 132 set positions). */
  VC_CHECK(vm_render_mob0(0x0200u + 8u, 0x0300u, VM_A_STAIRS));
  VC_CHECK(vm_count(4u) == 132u);
  VC_CHECK(vm_plane[0u] == 4u && vm_plane[2u * VF_PLANE_W] == 4u && vm_plane[2u * VF_PLANE_W + 2u] == 4u && vm_plane[2u * VF_PLANE_W + 3u] == 1u);
  VC_CHECK(vm_plane[23u * VF_PLANE_W + 7u] == 4u && vm_plane[24u * VF_PLANE_W + 7u] == 1u);
  return IVS_TEST_PASS;
}

/* ---- video.mob.size-8x16: tall mode, two stacked cards, card number bit 0 ignored (R40) ---- */

static int vm_size_8x16(void)
{
  static const char *const tall[16] = {"#.......", "##......", "###.....", "####....", "#####...", "######..", "#######.", "########",
                                       ".......#", "......##", ".....###", "....####", "...#####", "..######", ".#######", "########"};
  static const char *const tall_y[16] = {"########", ".#######", "..######", "...#####", "....####", ".....###", "......##", ".......#",
                                         "########", "#######.", "######..", "#####...", "####....", "###.....", "##......", "#......."};
  uint32_t card_bit;
  /* Y word 01A8: tall bit, size code 01, Y 40 (the corner at (32, 32)). The upper eight rows are the card 4 (staircase), the lower eight the card 5
   * (mirror); naming the odd card 5 in the attribute word must give the same picture (bit 0 of the card number is ignored in tall mode). */
  for (card_bit = 0u; card_bit < 2u; card_bit++) {
    VC_CHECK(vm_render_mob0(0x0200u + 40u, 0x0180u + 0x28u, card_bit == 0u ? VM_A_TALL4 : VM_A_TALL5));
    VC_CHECK(vm_region_is(32u, 32u, 8u, 16u, tall, 4u, card_bit == 0u ? "tall, card 4" : "tall, card 5"));
    VC_CHECK(vm_count(4u) == 72u);
  }
  /* The Y flip reverses all sixteen rows of the pair: the lower card comes first, upside down. */
  VC_CHECK(vm_render_mob0(0x0200u + 40u, 0x0980u + 0x28u, VM_A_TALL4));
  VC_CHECK(vm_region_is(32u, 32u, 8u, 16u, tall_y, 4u, "tall, Y flip"));
  /* at the Y size code 10 the sixteen rows cover 32 plane rows and 2 x 72 set positions */
  VC_CHECK(vm_render_mob0(0x0200u + 40u, 0x0280u + 0x10u, VM_A_TALL4));
  VC_CHECK(vm_count(4u) == 144u);
  VC_CHECK(vm_plane[8u * VF_PLANE_W + 32u] == 4u && vm_plane[39u * VF_PLANE_W + 39u] == 4u && vm_plane[40u * VF_PLANE_W + 39u] == 1u);
  return IVS_TEST_PASS;
}

/* ---- video.mob.frame-mirror: X flip, Y flip and both on the asymmetric card, with Y size 4x (F-M2) ---- */

static int vm_frame_mirror(void)
{
  const vm_frame *f = &vm_frames[2];
  const ivs_stic_params *p;
  uint32_t i;
  VC_CHECK(vm_fixture_ok(f));
  p = vm_scene(f);
  VC_CHECK(p != NULL);
  /* derivation (a) A7, scene 2 (section (c): frames 4 to 6 of the `mob` scenario): MOB0 0620 0620 080C (X flip), MOB1 0648 0A20 080D (Y flip),
   * MOB2 0670 0E20 080E (both flips), the Y size code 10 (2 plane rows per pattern row), colors 4, 5 and 6, the staircase card 1. */
  vm_put(0u, 0x0620u, 0x0620u, 0x080Cu);
  vm_put(1u, 0x0648u, 0x0A20u, 0x080Du);
  vm_put(2u, 0x0670u, 0x0E20u, 0x080Eu);
  VC_CHECK(vm_render(p, 1u, vm_plane) == VF_PLANE_BYTES);
  VC_CHECK(vm_matches(f));
  for (i = 4u; i <= 6u; i++) {
    VC_CHECK(vm_count(i) == 144u); /* a flip changes where the 36 x 4 set positions are, never how many */
  }
  return IVS_TEST_PASS;
}

/* ---- video.mob.colour-bit12: the attribute bit 12 is the fourth color bit (R40) ---- */

static int vm_colour_bit12(void)
{
  uint32_t k;
  uint32_t pass;
  for (pass = 0u; pass < 2u; pass++) {
    for (k = 0u; k < 16u; k++) {
      uint32_t aw = 0x0800u + (k & 7u) + (k >= 8u ? 0x1000u : 0u); /* solid card 0 in graphics RAM */
      const ivs_stic_params *p = vm_scene(&vm_frames[0]);
      uint32_t y;
      uint32_t x;
      VC_CHECK(p != NULL);
      vm_mem.stic_reg[0x28u] = (uint16_t)(pass == 0u ? 1u : 2u); /* the background of the fill cells: color 1, then color 2 (so that a MOB of either color shows) */
      vm_put(0u, 0x0200u + 40u, 0x0100u + 40u, aw);
      VC_CHECK(vm_render(p, 1u, vm_plane) == VF_PLANE_BYTES);
      for (y = 32u; y < 40u; y++) {
        for (x = 32u; x < 40u; x++) {
          if (vm_plane[y * VF_PLANE_W + x] != k) {
            ivs_test_diag("attribute %04X: position (%u,%u) is %u, expected the color %u", (unsigned)aw, (unsigned)x, (unsigned)y,
                          (unsigned)vm_plane[y * VF_PLANE_W + x], (unsigned)k);
            return IVS_TEST_FAIL;
          }
        }
      }
      VC_CHECK(vm_plane[32u * VF_PLANE_W + 40u] == (pass == 0u ? 1u : 2u)); /* the position right of the MOB is the background */
    }
  }
  return IVS_TEST_PASS;
}

/* ---- video.mob.frame-priority-colour: MOB over MOB by the lower index, MOB over a card, color bit 12, Y size 8x (F-M3) ---- */

static int vm_frame_priority_colour(void)
{
  const vm_frame *f = &vm_frames[3];
  const ivs_stic_params *p;
  VC_CHECK(vm_fixture_ok(f));
  p = vm_scene(f);
  VC_CHECK(p != NULL);
  /* derivation (a) A7, scene 3 (section (c): frames 7 to 9 of the `mob` scenario): MOB0 021C 031C 080C (1x, code 11, color 4), MOB1 0620 0224 0805
   * (2x, code 10, solid card 0, color 5), MOB2 0654 0138 1809 (2x, code 01, staircase, color 8 + 1 = 9, over the card cell (6, 10)). */
  vm_put(0u, 0x021Cu, 0x031Cu, 0x080Cu);
  vm_put(1u, 0x0620u, 0x0224u, 0x0805u);
  vm_put(2u, 0x0654u, 0x0138u, 0x1809u);
  VC_CHECK(vm_render(p, 1u, vm_plane) == VF_PLANE_BYTES);
  VC_CHECK(vm_matches(f));
  VC_CHECK(vm_count(4u) == 144u && vm_count(5u) == 244u && vm_count(9u) == 72u); /* 256 - 12 positions of MOB 1 are hidden behind MOB 0 */
  return IVS_TEST_PASS;
}

/* ---- video.mob.front-lower-index: where two MOBs coincide the lower index is shown, whatever the order of programming (R41) ---- */

static int vm_front_lower_index(void)
{
  /* A staircase MOB (color 4, corner (32, 32)) and a solid MOB (color 5, corner (36, 36)) overlap on x 36..39, y 36..39. The staircase rows 4 to 7
   * have the pixels c <= r, so in the overlap row y = 36 + (r - 4) the columns x = 36 .. 36 + (r - 4) are set. */
  static const char *const stairs_first[4] = {"4555", "4455", "4445", "4444"};
  static const char *const solid_first[4] = {"5555", "5555", "5555", "5555"};
  uint32_t order;
  for (order = 0u; order < 2u; order++) {
    const ivs_stic_params *p = vm_scene(&vm_frames[0]);
    uint32_t stairs = order == 0u ? 0u : 1u;
    uint32_t solid = 1u - stairs;
    VC_CHECK(p != NULL);
    vm_put(stairs, 0x0200u + 40u, 0x0100u + 40u, VM_A_STAIRS);
    vm_put(solid, 0x0200u + 44u, 0x0100u + 44u, 0x0805u); /* solid card 0, color 5 */
    VC_CHECK(vm_render(p, 1u, vm_plane) == VF_PLANE_BYTES);
    VC_CHECK(vm_region_is(36u, 36u, 4u, 4u, order == 0u ? stairs_first : solid_first, 0u, order == 0u ? "staircase at index 0" : "staircase at index 1"));
    /* the counts: the staircase has 36 set positions of which 10 lie in the overlap; the solid MOB has 64 */
    VC_CHECK(order == 0u ? (vm_count(4u) == 36u && vm_count(5u) == 54u) : (vm_count(4u) == 26u && vm_count(5u) == 64u));
  }
  return IVS_TEST_PASS;
}

/* ---- video.mob.prio-clear-front: a MOB with the priority bit clear is in front of every card pixel (R42) ---- */

static int vm_prio_clear_front(void)
{
  /* The block of the card cell (6, 10) is color 2 at x 80..84, y 48..52 over the background 1. A solid MOB (corner (80, 48)) covers it: every
   * position of the block shows the MOB color. A staircase MOB there shows its color where its pattern is set and lets the card show where it is
   * clear: row y = 48 + r has the pattern pixels c <= r, so the block rows read 42222, 44222, 44422, 44442, 44444. */
  static const char *const block[5] = {"42222", "44222", "44422", "44442", "44444"};
  VC_CHECK(vm_render_mob0(0x0200u + 88u, 0x0100u + 56u, VM_A_SOLID));
  VC_CHECK(vm_count(2u) == 0u && vm_count(4u) == 64u);
  VC_CHECK(vm_render_mob0(0x0200u + 88u, 0x0100u + 56u, VM_A_STAIRS));
  VC_CHECK(vm_region_is(80u, 48u, 5u, 5u, block, 0u, "staircase over the block"));
  VC_CHECK(vm_count(2u) == 10u && vm_count(4u) == 36u); /* 25 - (1 + 2 + 3 + 4 + 5) card positions remain */
  return IVS_TEST_PASS;
}

/* ---- video.mob.register-bit-roles: every bit of the 48 rows of the MOB literal, one at a time (R37 to R43) ---- */

/* The base states of the sweep. Each is chosen so that the role of the bit can show; the first four are MOB 0 over the cards of F-M0 at the corner
 * (32, 32), the fifth uses a generated graphics ROM, the sixth lays the MOB over the card block. */
#define VM_BASE_SOLID 0u   /* 1x, size code 01, solid card 0, color 4 */
#define VM_BASE_STAIRS 1u  /* the same with the staircase card 1: flips and sizes change a staircase */
#define VM_BASE_TALL5 2u   /* non-tall, card 4 (staircase) whose odd card 5 is the mirror: the tall bit adds the lower eight rows */
#define VM_BASE_TALLSET 3u /* tall mode on, cards 4 and 5: the card number bit 0 must change nothing */
#define VM_BASE_ROM 4u     /* graphics ROM, a generated pattern in which every card differs from every other, card number 0x55 */
#define VM_BASE_PRIO 5u    /* a solid MOB over the card block, priority bit clear */
#define VM_BASE_TWO 6u     /* two overlapping solid MOBs: MOB 0 without the interaction bit, MOB 1 with it (the collision registers can show the bit of MOB 0) */

static void vm_base_scene(uint32_t base)
{
  const ivs_stic_params *p = vm_scene(&vm_frames[0]);
  uint32_t xw = 0x0200u + 40u;
  uint32_t yw = 0x0100u + 40u;
  uint32_t aw = VM_A_SOLID;
  (void)p;
  switch (base) {
    case VM_BASE_STAIRS:
      aw = VM_A_STAIRS;
      break;
    case VM_BASE_TALL5:
      aw = VM_A_TALL4;
      break;
    case VM_BASE_TALLSET:
      yw = 0x0180u + 40u;
      aw = VM_A_TALL4;
      break;
    case VM_BASE_ROM: {
      uint8_t grom[IVS_MEMSYS_GROM_BYTES];
      uint32_t i;
      for (i = 0u; i < IVS_MEMSYS_GROM_BYTES; i++) {
        grom[i] = (uint8_t)((i >> 3) + 1u + 37u * (i & 7u)); /* byte y of card c is c + 1 + 37 y: injective in c for every y, so no two cards are equal */
      }
      ivs_memsys_install_grom(&vm_mem, grom);
      aw = (0x55u << 3) + 4u; /* graphics ROM, card 0x55, color 4 */
      break;
    }
    case VM_BASE_PRIO:
      xw = 0x0200u + 88u;
      yw = 0x0100u + 56u;
      break;
    case VM_BASE_TWO:
      /* MOB 1: visible, interaction enable set (X word bit 8), X 44 and Y 44, solid card 0, color 5: it overlaps MOB 0 (X 40, Y 40) on the plane
       * positions x 36..39, y 36..39. MOB 0 keeps its interaction bit clear, so no collision bit is set until a flip sets that bit. */
      vm_put(1u, 0x0300u + 44u, 0x0100u + 44u, 0x0805u);
      break;
    default:
      break;
  }
  vm_put(0u, xw, yw, aw);
}

/* The base state and the observability a role asks for. A bit is OBSERVABLE when flipping that single bit from the base state changes the plane or
 * changes any of the collision registers $0018 to $001F after the frame (plan 08-05: the registers are the second observation channel); the sweep
 * requires an observable change exactly when the literal's role is not ign (in the row's own base state). The interaction-enable bit of the X
 * register (rule R44) acts only through the collision registers, so its row asks for a change of the collision registers and none of the plane, in
 * its two-MOB base state (the exemption of plan 08-04 is gone). *want is 1 when an observable change is required, 0 when none may occur, 3 when a
 * change of the collision registers only is required; 2 is an unknown role (a fixture failure). */
static uint32_t vm_base_for(const vm_role *r, uint32_t *want, uint32_t *second_base)
{
  const char *role = r->role;
  *want = 1u;
  *second_base = 99u;
  if (strcmp(role, "ign") == 0) {
    *want = 0u;
    *second_base = VM_BASE_PRIO; /* an ignored bit must change nothing in either base: the staircase and the MOB over the card block */
    return VM_BASE_STAIRS;
  }
  if (strcmp(r->reg, "mobx") == 0 && strcmp(role, "intr") == 0) {
    *want = 3u; /* the collision registers change and the plane does not (R44) */
    return VM_BASE_TWO;
  }
  if (strncmp(role, "pos", 3u) == 0 || strcmp(role, "visb") == 0 || strcmp(role, "xsize") == 0 || strncmp(role, "color", 5u) == 0 ||
      strcmp(role, "sel") == 0) {
    return VM_BASE_SOLID;
  }
  if (strcmp(role, "yres") == 0) {
    return VM_BASE_TALL5;
  }
  if (strcmp(role, "ysiz2") == 0 || strcmp(role, "ysiz4") == 0 || strcmp(role, "xflip") == 0 || strcmp(role, "yflip") == 0) {
    return VM_BASE_STAIRS;
  }
  if (strcmp(role, "prio") == 0) {
    return VM_BASE_PRIO;
  }
  if (strcmp(role, "card6") == 0 || strcmp(role, "card7") == 0) {
    return VM_BASE_ROM; /* ignored for a graphics RAM card: the bit shows only for a graphics ROM card */
  }
  if (strcmp(role, "card0") == 0) {
    *second_base = VM_BASE_TALLSET; /* and in tall mode the bit must change nothing */
    return VM_BASE_SOLID;
  }
  if (role[0] == 'c' && role[1] == 'a' && role[2] == 'r' && role[3] == 'd' && role[4] >= '1' && role[4] <= '5' && role[5] == '\0') {
    return VM_BASE_SOLID;
  }
  *want = 2u;
  return VM_BASE_SOLID;
}

static uint32_t vm_reg_index(const char *reg)
{
  return strcmp(reg, "mobx") == 0 ? 0u : (strcmp(reg, "moby") == 0 ? VM_REG_Y : VM_REG_A);
}

/* The eight collision registers after one displayed frame of the scene in vm_mem and vm_state (plan 08-05): they are cleared first, the frame is
 * delivered through ivs_stic_frame_deliver (which ORs the collision of the frame into the register file) and read back masked with 03FF, then cleared
 * again, so the scene is left as it was. */
static void vm_collision_regs(const ivs_stic_params *p, uint16_t out[8])
{
  ivs_stic_state st = vm_state;
  ivs_stic_video_row row;
  uint32_t k;
  for (k = 0u; k < 8u; k++) {
    vm_mem.stic_reg[0x18u + k] = 0u;
  }
  st.windows_active = 1u;
  ivs_video_chain_init(&row);
  ivs_stic_frame_deliver(p, &st, &vm_mem, &row, NULL, 59736u);
  for (k = 0u; k < 8u; k++) {
    out[k] = (uint16_t)(vm_mem.stic_reg[0x18u + k] & 0x03FFu);
    vm_mem.stic_reg[0x18u + k] = 0u;
  }
}

#define VM_EFFECT_PLANE 1 /* the plane changed */
#define VM_EFFECT_COLL 2  /* a collision register changed */

/* The effect of flipping bit `bit` of register index idx from base state `base`: a bit mask of VM_EFFECT_PLANE and VM_EFFECT_COLL (0 none), or -1 when
 * a render fails. */
static int vm_flip_effect(uint32_t base, uint32_t idx, uint32_t bit)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint16_t before[8];
  uint16_t after[8];
  int effect = 0;
  vm_base_scene(base);
  if (p == NULL || vm_render(p, 1u, vm_exp) != VF_PLANE_BYTES) {
    return -1;
  }
  vm_collision_regs(p, before);
  vm_mem.stic_reg[idx] = (uint16_t)(vm_mem.stic_reg[idx] ^ (1u << bit));
  if (vm_render(p, 1u, vm_plane) != VF_PLANE_BYTES) {
    return -1;
  }
  vm_collision_regs(p, after);
  effect |= memcmp(vm_plane, vm_exp, VF_PLANE_BYTES) != 0 ? VM_EFFECT_PLANE : 0;
  effect |= memcmp(before, after, sizeof before) != 0 ? VM_EFFECT_COLL : 0;
  return effect;
}

/* The sweep over the n rows of a role table; the number of rows whose observed effect disagrees with the role (0 is a pass). quiet suppresses the
 * diagnostics (the judge of plan 08-04 task 3 runs it on a copy with two roles swapped). */
static uint32_t vm_sweep(const vm_role *t, size_t n, int quiet)
{
  uint32_t bad = 0u;
  size_t i;
  for (i = 0u; i < n; i++) {
    uint32_t want;
    uint32_t second;
    uint32_t base = vm_base_for(&t[i], &want, &second);
    int seen;
    if (want == 2u) {
      if (!quiet) {
        ivs_test_diag("FIXTURE: no base state for the role %s of %s bit %u", t[i].role, t[i].reg, (unsigned)t[i].bit);
      }
      bad += 1u;
      continue;
    }
    seen = vm_flip_effect(base, vm_reg_index(t[i].reg), t[i].bit);
    if (seen < 0 || (want == 0u && seen != 0) || (want == 1u && seen == 0) || (want == 3u && seen != VM_EFFECT_COLL)) {
      if (!quiet) {
        ivs_test_diag("%s bit %u (role %s): flipping it changes %s, the literal says it %s", t[i].reg, (unsigned)t[i].bit, t[i].role,
                      seen == 0 ? "nothing" : (seen == VM_EFFECT_PLANE ? "the plane" : (seen == VM_EFFECT_COLL ? "the collision registers" : "both")),
                      want == 0u ? "changes nothing" : (want == 3u ? "changes the collision registers only" : "changes the picture or the collision registers"));
      }
      bad += 1u;
      continue;
    }
    if (second != 99u) {
      int again = vm_flip_effect(second, vm_reg_index(t[i].reg), t[i].bit);
      if (again != 0) { /* card number bit 0 in tall mode, and every ignored bit in the second base */
        if (!quiet) {
          ivs_test_diag("%s bit %u (role %s): flipping it must change nothing in its second base state", t[i].reg, (unsigned)t[i].bit, t[i].role);
        }
        bad += 1u;
      }
    }
  }
  return bad;
}

static int vm_register_bit_roles(void)
{
  uint32_t i;
  uint32_t ign = 0u;
  uint32_t seen[3] = {0u, 0u, 0u};
  VC_CHECK(sizeof vm_roles / sizeof vm_roles[0] == 48u);
  for (i = 0u; i < 48u; i++) {
    ign += strcmp(vm_roles[i].role, "ign") == 0 ? 1u : 0u;
    seen[vm_reg_index(vm_roles[i].reg) / 8u] += 1u;
  }
  VC_CHECK(seen[0] == 16u && seen[1] == 16u && seen[2] == 16u && ign == 5u + 4u + 2u); /* the literal: X 11..15, Y 12..15, attribute 14 and 15 */
  VC_CHECK(vm_sweep(vm_roles, 48u, 0) == 0u);
  return IVS_TEST_PASS;
}

/* ---- video.mob.stream-equals-plane: the streamed slot of ivs_stic_frame_deliver is the whole plane of ivs_stic_render_plane ---- */


/* The F-M3 scene programmed on a fresh memory system (the registers of vm_frame_priority_colour, from derivation A7). */
static const ivs_stic_params *vm_scene_f3(void)
{
  const ivs_stic_params *p = vm_scene(&vm_frames[3]);
  vm_put(0u, 0x021Cu, 0x031Cu, 0x080Cu);
  vm_put(1u, 0x0620u, 0x0224u, 0x0805u);
  vm_put(2u, 0x0654u, 0x0138u, 0x1809u);
  return p;
}

static int vm_stream_equals_plane(void)
{
  static uint8_t slot[VM_SLOT_BYTES + 64u];
  const ivs_stic_params *p = vm_scene_f3();
  ivs_stic_state sa;
  ivs_stic_state sb;
  ivs_stic_state sc;
  ivs_stic_video_row ra;
  ivs_stic_video_row rb;
  ivs_stic_video_row rc;
  ivs_psg_sink ka;
  ivs_psg_sink kc;
  uint32_t y;
  VC_CHECK(p != NULL && vm_render(p, 1u, vm_plane) == VF_PLANE_BYTES);
  VC_CHECK(p->video_slot_bytes == VM_SLOT_BYTES && p->video_width == 159u && p->video_height == 96u);
  vm_state.windows_active = 1u; /* the frame is displayed */
  sa = vm_state;
  sb = vm_state;
  sc = vm_state;
  ivs_video_chain_init(&ra);
  ivs_video_chain_init(&rb);
  ivs_video_chain_init(&rc);
  memset(slot, (int)VP_POISON, sizeof slot);
  ivs_psg_sink_init(&ka, NULL, 0u);
  ivs_psg_sink_video(&ka, slot, VM_SLOT_BYTES);
  ivs_stic_frame_deliver(p, &sa, &vm_mem, &ra, &ka, VM_F); /* with a video buffer */
  VC_CHECK(ka.video_frames == 1u && ka.video_written == VM_SLOT_BYTES && ka.video_overflow == 0u && ra.count == 1u);
  VC_CHECK(slot[VM_SLOT_BYTES] == (uint8_t)VP_POISON); /* nothing written past the slot */
  VC_CHECK(slot[27] == 1u); /* the header flag word: bit 0 displayed */
  for (y = 0u; y < VF_PLANE_H; y++) {
    if (memcmp(slot + 32u + (size_t)y * 159u, vm_plane + (size_t)y * VF_PLANE_W, 159u) != 0) {
      ivs_test_diag("the streamed slot differs from the whole plane in plane row %u", (unsigned)y);
      return IVS_TEST_FAIL;
    }
  }
  ivs_stic_frame_deliver(p, &sb, &vm_mem, &rb, NULL, VM_F); /* with no sink at all */
  ivs_psg_sink_init(&kc, NULL, 0u);
  ivs_stic_frame_deliver(p, &sc, &vm_mem, &rc, &kc, VM_F); /* with the counting sink: no buffer */
  VC_CHECK(rb.count == 1u && rc.count == 1u && kc.video_frames == 1u && kc.video_written == 0u);
  VC_CHECK(memcmp(ra.chain, rb.chain, 32u) == 0 && memcmp(ra.chain, rc.chain, 32u) == 0); /* the chain does not depend on a buffer */
  VC_CHECK(memcmp(&sa, &sb, sizeof sa) == 0 && memcmp(&sa, &sc, sizeof sa) == 0);
  return IVS_TEST_PASS;
}

/* ---- video.mob.slice-invariant: the host's call sizes change no byte of a run that draws MOBs ---- */


/* The stand-in guest (decles derived in tests/stic/stic_cases.h): the three graphics RAM cards of derivation A2 written byte by byte, the nine MOB
 * registers of the F-M3 scene (derivation A7, scene 3; none when with_mobs is 0), a read of $0021 (Color Stack), then the display strobe in an
 * endless loop, so every frame from the first strobe on is displayed. Frame 0 holds the initialization, as in the program of the oracle. */
static size_t vm_guest(uint16_t *w, int with_mobs)
{
  static const uint8_t block[8] = {0xF8u, 0xF8u, 0xF8u, 0xF8u, 0xF8u, 0x00u, 0x00u, 0x00u};
  static const uint16_t regs[9][2] = {{0x0000u, 0x021Cu}, {0x0001u, 0x0620u}, {0x0002u, 0x0654u}, {0x0008u, 0x031Cu}, {0x0009u, 0x0224u},
                                      {0x000Au, 0x0138u}, {0x0010u, 0x080Cu}, {0x0011u, 0x0805u}, {0x0012u, 0x1809u}};
  size_t n = 0u;
  size_t start;
  uint32_t c;
  uint32_t y;
  for (c = 0u; c < 3u; c++) {
    for (y = 0u; y < 8u; y++) {
      uint32_t b = c == 0u ? 0xFFu : (c == 1u ? vm_card_stairs[y] : block[y]);
      n = sc_w_mvii(w, n, 0u, b);
      n = sc_w_mvo(w, n, 0u, 0x3800u + 8u * c + y);
    }
  }
  for (c = 0u; with_mobs != 0 && c < 9u; c++) {
    n = sc_w_mvii16(w, n, 0u, regs[c][1]);
    n = sc_w_mvo(w, n, 0u, regs[c][0]);
  }
  n = sc_w_mvi(w, n, 0x0021u, 1u);
  start = n;
  n = sc_w_mvo(w, n, 0u, 0x0020u);
  w[n] = 0x0220u; /* B back to the strobe */
  w[n + 1u] = (uint16_t)(n + 1u - start);
  return n + 2u;
}

static ivs_status vm_call(ivs_ms_inst *x, uint64_t target, uint8_t *video, uint64_t cap, ivs_run_report *r)
{
  ivs_run_request q;
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.target_ticks = target;
  q.video = video;
  q.video_capacity = cap;
  memset(r, 0, sizeof *r);
  r->struct_size = (uint32_t)sizeof *r;
  return ivs_machine_run(x->m, &q, r);
}

static uint64_t vm_now(const ivs_ms_inst *x)
{
  return ivs_machine_from_handle(x->m)->time_ticks;
}

/* Runs the guest of w[0..n) to VM_TOTAL in calls of `slice` ticks (0: one call), appending every delivered slot to out; the instance is left in x
 * for the whole-storage comparison. The delivered byte count, or (uint64_t)-1 on a failure. */
uint64_t vm_run_sliced(ivs_ms_inst *x, const uint16_t *w, size_t n, uint64_t slice, uint8_t *out)
{
  static uint8_t tmp[VM_SLOT_BYTES * 5u];
  uint64_t got = 0u;
  if (!sc_guest_n(x, w, n)) {
    return (uint64_t)-1;
  }
  while (vm_now(x) < VM_TOTAL) {
    ivs_run_report r;
    uint64_t now = vm_now(x);
    uint64_t target = slice == 0u ? VM_TOTAL : (now + slice < VM_TOTAL ? now + slice : VM_TOTAL);
    if (vm_call(x, target, tmp, VM_CAP, &r) != IVS_OK || got + r.video_bytes_written > VM_CAP) {
      return (uint64_t)-1;
    }
    memcpy(out + got, tmp, (size_t)r.video_bytes_written);
    got += r.video_bytes_written;
  }
  return got;
}

int vm_slice_one(uint64_t slice, uint8_t *ref, uint64_t ref_bytes, const ivs_ms_inst *a, const uint16_t *w, size_t n)
{
  ivs_ms_inst b;
  static uint8_t out[VM_SLOT_BYTES * 5u];
  uint64_t got;
  int ok;
  memset(&b, 0, sizeof b);
  got = vm_run_sliced(&b, w, n, slice, out);
  ok = got == ref_bytes && memcmp(out, ref, (size_t)ref_bytes) == 0 && memcmp(a->storage, b.storage, IVS_MACHINE_BYTES) == 0 &&
       memcmp(ivs_machine_from_handle(a->m)->obs.video.chain, ivs_machine_from_handle(b.m)->obs.video.chain, 32u) == 0;
  if (!ok) {
    ivs_test_diag("calls of %llu ticks differ from one call: %llu bytes against %llu", (unsigned long long)slice, (unsigned long long)got,
                  (unsigned long long)ref_bytes);
  }
  if (b.storage != NULL) {
    ivs_ms_free(&b);
  }
  return ok;
}

static int vm_slice_invariant(void)
{
  static uint16_t w[600];
  static uint8_t ref[VM_SLOT_BYTES * 5u];
  static const uint64_t sizes[4] = {1u, 7u, 468u, 97u};
  ivs_ms_inst a;
  uint64_t ref_bytes;
  uint32_t k;
  int ok = 0;
  size_t n = vm_guest(w, 1);
  const uint8_t *last;
  VC_CHECK(n < sizeof w / sizeof w[0]);
  memset(&a, 0, sizeof a);
  ref_bytes = vm_run_sliced(&a, w, n, 0u, ref); /* the reference: one call to the same end */
  if (ref_bytes != 3u * VM_SLOT_BYTES) {
    ivs_test_diag("the reference run delivered %llu bytes, expected %u", (unsigned long long)ref_bytes, (unsigned)(3u * VM_SLOT_BYTES));
    goto done;
  }
  /* non-vacuity: the registers were written by the guest, and the last frame shows the MOB pixels of F-M3 at positions the literal gives
   * (20,20) color 4, (24,28) color 5, (24,36) color 4, (84,52) color 9 (derivation (e), the pixel probes of F-M3) */
  if (ivs_machine_from_handle(a.m)->mem.stic_reg[0] != 0x021Cu || ivs_machine_from_handle(a.m)->mem.stic_reg[0x12u] != 0x1809u) {
    ivs_test_diag("the guest did not leave the F-M3 registers in the register file");
    goto done;
  }
  last = ref + 2u * VM_SLOT_BYTES + 32u;
  if (last[20u * 159u + 20u] != 4u || last[28u * 159u + 24u] != 5u || last[36u * 159u + 24u] != 4u || last[52u * 159u + 84u] != 9u) {
    ivs_test_diag("the last delivered frame does not show the MOBs of the scene");
    goto done;
  }
  for (k = 0u; k < 4u; k++) {
    if (!vm_slice_one(sizes[k], ref, ref_bytes, &a, w, n)) {
      goto done;
    }
  }
  ok = 1;
done:
  ivs_ms_free(&a);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- the pinned defaults (D-02, T-08-14): each test asserts the default and then changes ONE field of a stack copy of ivs_stic_params and
 * requires a different plane, so no default can pass as a confirmed behavior. A field value other than the two selectors is refused by the
 * generator, so only these two values are exercised. ---- */

/* 1 when planes a and b differ in at least one published position. */
int vm_planes_differ(const uint8_t *a, const uint8_t *b)
{
  uint32_t y;
  uint32_t x;
  for (y = 0u; y < VF_PLANE_H; y++) {
    for (x = 0u; x < VF_PLANE_W - 1u; x++) {
      if (a[y * VF_PLANE_W + x] != b[y * VF_PLANE_W + x]) {
        return 1;
      }
    }
  }
  return 0;
}

/* ---- video.mob.default-pinned.prio-set: a MOB with the priority bit set (R43, C107, field video.mob_prio_hides) ---- */

static int vm_pin_prio_set(void)
{
  const ivs_stic_params *p = vm_scene(&vm_frames[0]);
  ivs_stic_params q;
  /* A solid MOB of color 4 with the priority bit set (attribute 2804) covers x 80..87, y 48..55, which holds the 5 x 5 on pixels of the card cell
   * (6, 10) (color 2) over the background color 1 of every other position. Default (0): the on pixels hide the MOB and the 39 other positions
   * show it through the card background; flip (1): every card pixel hides it, so the MOB is never shown. */
  VC_CHECK(p != NULL && p->video_mob_prio_hides == 0u);
  vm_put(0u, 0x0200u + 88u, 0x0100u + 56u, VM_A_SOLID + 0x2000u);
  VC_CHECK(vm_render(p, 1u, vm_plane) == VF_PLANE_BYTES);
  VC_CHECK(vm_count(4u) == 39u && vm_count(2u) == 25u);
  VC_CHECK(vm_plane[48u * VF_PLANE_W + 80u] == 2u && vm_plane[52u * VF_PLANE_W + 84u] == 2u); /* the on pixels hide the MOB */
  VC_CHECK(vm_plane[48u * VF_PLANE_W + 85u] == 4u && vm_plane[53u * VF_PLANE_W + 80u] == 4u && vm_plane[55u * VF_PLANE_W + 87u] == 4u); /* through the background */
  q = *p;
  q.video_mob_prio_hides = 1u; /* the one-field flip */
  memcpy(vm_exp, vm_plane, VF_PLANE_BYTES);
  VC_CHECK(vm_render(&q, 1u, vm_plane) == VF_PLANE_BYTES);
  VC_CHECK(vm_planes_differ(vm_plane, vm_exp));
  VC_CHECK(vm_count(4u) == 0u && vm_count(2u) == 25u);
  return IVS_TEST_PASS;
}

/* ---- the two sampling pins (R50, C108, field video.mob_line_sample): a Y size code that puts two pattern rows into one plane row ---- */

/* The literal f rendered under the default and compared position by position (and by hash), then rendered again with the line sample flipped to 1. */
static int vm_pin_sampling(const vm_frame *f, uint32_t xw, uint32_t yw, uint32_t aw)
{
  const ivs_stic_params *p;
  ivs_stic_params q;
  VC_CHECK(vm_fixture_ok(f));
  p = vm_scene(f);
  VC_CHECK(p != NULL && p->video_mob_line_sample == 0u);
  vm_put(0u, xw, yw, aw);
  VC_CHECK(vm_render(p, 1u, vm_plane) == VF_PLANE_BYTES);
  VC_CHECK(vm_matches(f)); /* the even line: the published plane row y shows the television line 2 y */
  q = *p;
  q.video_mob_line_sample = 1u; /* the one-field flip: the odd line */
  memcpy(vm_exp, vm_plane, VF_PLANE_BYTES);
  VC_CHECK(vm_render(&q, 1u, vm_plane) == VF_PLANE_BYTES);
  VC_CHECK(vm_planes_differ(vm_plane, vm_exp));
  return IVS_TEST_PASS;
}

static int vm_pin_sampling_y1x(void)
{
  /* F-M8: X 32 (2x) and Y 32 (size code 00, one television line per pattern row), the staircase card 1, color 4 */
  return vm_pin_sampling(&vm_frames[8], 0x0600u + 32u, 32u, VM_A_STAIRS);
}

static int vm_pin_sampling_8x16(void)
{
  /* F-M9: X 48 (1x), Y 32 with the tall bit (size code 00), the cards 4 and 5, color 4 */
  return vm_pin_sampling(&vm_frames[9], 0x0200u + 48u, 0x0080u + 32u, VM_A_TALL4);
}

/* ---- the judges: a comparison that cannot fail proves nothing ---- */

/* video.judge.mob-frame-corrupt: a copy of the literal F-M1 with one expected position changed makes the comparison report a mismatch at exactly
 * that position (and the hash comparison fail), and the uncorrupted copy passes. */
static int vm_judge_frame_corrupt(void)
{
  const vm_frame *f = &vm_frames[1];
  const ivs_stic_params *p;
  static const uint32_t at[3][2] = {{150u, 90u}, {0u, 0u}, {158u, 95u}}; /* a MOB position, the first and the last published position */
  char hex[65];
  uint32_t k;
  uint32_t skipped = 0u;
  VC_CHECK(vm_fixture_ok(f));
  p = vm_scene(f);
  VC_CHECK(p != NULL);
  vm_put(0u, 0x069Eu, 0x0162u, 0x080Cu);
  VC_CHECK(vm_render(p, 1u, vm_plane) == VF_PLANE_BYTES);
  vm_expected(f, vm_exp);
  VC_CHECK(vm_first_diff(vm_plane, vm_exp, &skipped) == -1); /* the control passes */
  for (k = 0u; k < 3u; k++) {
    uint32_t i = at[k][1] * VF_PLANE_W + at[k][0];
    uint8_t saved = vm_exp[i];
    vm_exp[i] = (uint8_t)(saved ^ 1u);
    VC_CHECK(vm_first_diff(vm_plane, vm_exp, &skipped) == (long)(at[k][1] * (VF_PLANE_W - 1u) + at[k][0])); /* reported at the corrupted position */
    vf_text_sha(vm_exp, VF_PLANE_W, VF_PLANE_W - 1u, hex);
    VC_CHECK(strcmp(hex, f->sha_text) != 0); /* and the hash of the corrupted copy is not the literal's */
    vm_exp[i] = saved;
  }
  vm_exp[150u * 0u + 5u * VF_PLANE_W + 5u] = 0xFFu; /* a wildcard position is skipped, never compared */
  VC_CHECK(vm_first_diff(vm_plane, vm_exp, &skipped) == -1 && skipped == 1u);
  return IVS_TEST_PASS;
}

/* video.judge.mob-role-swapped: a copy of the role table with the roles of a used bit and an ignored bit exchanged makes the register-bit sweep
 * fail (and the true table passes): the sweep reads its roles from the table, so a wrong literal is caught. */
static int vm_judge_role_swapped(void)
{
  static vm_role copy[48];
  static const uint32_t pairs[3][2] = {{10u, 11u}, {16u + 10u, 16u + 12u}, {32u + 13u, 32u + 14u}}; /* xsize and ign, xflip and ign, prio and ign */
  uint32_t k;
  VC_CHECK(vm_sweep(vm_roles, 48u, 1) == 0u); /* the control passes */
  for (k = 0u; k < 3u; k++) {
    const char *tmp;
    memcpy(copy, vm_roles, sizeof copy);
    VC_CHECK(strcmp(copy[pairs[k][0]].role, "ign") != 0 && strcmp(copy[pairs[k][1]].role, "ign") == 0);
    tmp = copy[pairs[k][0]].role;
    copy[pairs[k][0]].role = copy[pairs[k][1]].role;
    copy[pairs[k][1]].role = tmp;
    VC_CHECK(vm_sweep(copy, 48u, 1) >= 2u); /* both rows now disagree with the plane */
  }
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_video_mob_cases[] = {
    {"video.mob.base-neutral", vm_base_neutral, NULL},
    {"video.mob.frame-placement-clip", vm_frame_placement_clip, NULL},
    {"video.mob.field-origin-8-8", vm_field_origin, NULL},
    {"video.mob.params-match-profile", vm_params_match_profile, NULL},
    {"video.mob.zoom-x2", vm_zoom_x2, NULL},
    {"video.mob.zoom-y2-y4-y8", vm_zoom_y, NULL},
    {"video.mob.size-8x16", vm_size_8x16, NULL},
    {"video.mob.frame-mirror", vm_frame_mirror, NULL},
    {"video.mob.colour-bit12", vm_colour_bit12, NULL},
    {"video.mob.frame-priority-colour", vm_frame_priority_colour, NULL},
    {"video.mob.front-lower-index", vm_front_lower_index, NULL},
    {"video.mob.prio-clear-front", vm_prio_clear_front, NULL},
    {"video.mob.register-bit-roles", vm_register_bit_roles, NULL},
    {"video.mob.stream-equals-plane", vm_stream_equals_plane, NULL},
    {"video.mob.slice-invariant", vm_slice_invariant, NULL},
    {"video.mob.default-pinned.prio-set", vm_pin_prio_set, NULL},
    {"video.mob.default-pinned.sampling-y1x", vm_pin_sampling_y1x, NULL},
    {"video.mob.default-pinned.sampling-8x16", vm_pin_sampling_8x16, NULL},
    {"video.judge.mob-frame-corrupt", vm_judge_frame_corrupt, NULL},
    {"video.judge.mob-role-swapped", vm_judge_role_swapped, NULL},
};
const size_t ivs_video_mob_count = sizeof ivs_video_mob_cases / sizeof ivs_video_mob_cases[0];

/* The sources of the generated include (the ten literals and the profile), for the TAP header of the suite. */
const ivs_test_source *vm_source_table(size_t *n)
{
  *n = sizeof vm_sources / sizeof vm_sources[0];
  return vm_sources;
}
