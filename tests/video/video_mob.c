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

#define VM_FRAMES 10u
#define VM_FIRST_MOB_REG 0u
#define VM_REG_Y 8u
#define VM_REG_A 16u

static ivs_memsys vm_mem;
static ivs_stic_state vm_state;
static uint8_t vm_plane[VF_PLANE_BYTES];
static uint8_t vm_exp[VF_PLANE_BYTES];

/* The three program cards of derivation A2 (a card is 8 pattern bytes, byte y is pixel row y from the top, bit 7 the leftmost pixel). */
static const uint8_t vm_card_solid[8] = {0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu};
static const uint8_t vm_card_stairs[8] = {0x80u, 0xC0u, 0xE0u, 0xF0u, 0xF8u, 0xFCu, 0xFEu, 0xFFu};

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
static void vm_expected(const vm_frame *f, uint8_t *exp)
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
static const ivs_stic_params *vm_scene(const vm_frame *f)
{
  uint32_t i;
  uint32_t k;
  (void)ivs_memsys_init(&vm_mem, SC_PROFILE);
  vf_poke_gram_card(&vm_mem, 0u, vm_card_solid);
  vf_poke_gram_card(&vm_mem, 1u, vm_card_stairs);
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
static void vm_put(uint32_t i, uint32_t xw, uint32_t yw, uint32_t aw)
{
  vm_mem.stic_reg[VM_FIRST_MOB_REG + i] = (uint16_t)xw;
  vm_mem.stic_reg[VM_REG_Y + i] = (uint16_t)yw;
  vm_mem.stic_reg[VM_REG_A + i] = (uint16_t)aw;
}

static uint32_t vm_render(const ivs_stic_params *p, uint32_t displayed, uint8_t *plane)
{
  return ivs_stic_render_plane(p, &vm_state, &vm_mem, displayed, plane);
}

/* The index of the first published position (x below 159) at which got differs from the literal plane exp, a `-` position being skipped,
 * or -1; *skipped counts the skipped positions of the published columns. */
static long vm_first_diff(const uint8_t *got, const uint8_t *exp, uint32_t *skipped)
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
static int vm_fixture_ok(const vm_frame *f)
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
static int vm_matches(const vm_frame *f)
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

const ivs_test_case ivs_video_mob_cases[] = {
    {"video.mob.base-neutral", vm_base_neutral, NULL},
    {"video.mob.frame-placement-clip", vm_frame_placement_clip, NULL},
    {"video.mob.field-origin-8-8", vm_field_origin, NULL},
    {"video.mob.params-match-profile", vm_params_match_profile, NULL},
};
const size_t ivs_video_mob_count = sizeof ivs_video_mob_cases / sizeof ivs_video_mob_cases[0];

/* The sources of the generated include (the ten literals and the profile), for the TAP header of the suite. */
const ivs_test_source *vm_source_table(size_t *n)
{
  *n = sizeof vm_sources / sizeof vm_sources[0];
  return vm_sources;
}
