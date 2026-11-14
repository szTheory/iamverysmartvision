/* SPDX-License-Identifier: MIT */
/* Suite video, families hdelay, vdelay and delay (plan 08-06, VID-05, D-02, D-03, D-07, D-10): the horizontal delay register $0030 and the vertical delay
 * register $0031 of core/src/stic/stic_frame.c held to the frozen oracle of plan 08-03. The expected pictures are the hand-derived frame literals F-M5
 * (horizontal delay 3, both strips), F-M6 (vertical delay 3, both strips) and F-M7 (both delays, no strips, the exposed positions unknown) of
 * data/video/frames, derived from the rules before any renderer existed, frozen by the blind second read and never produced by a run, plus counts and
 * positions derived by hand in the comments below from rule STIC-R46 (derivation section (b)): every card pixel and every MOB pixel is drawn h plane
 * columns to the right and v plane rows below its undelayed position, and what the shift moves past the right or the bottom edge of the plane is not
 * published. The register words are typed here from the scene table (A7) of docs/video/diag-stic-2-derivation.md (scene 5 and scene 6 write the MOB
 * `0644 0130 080C`), never read from a core table or from a run.
 *
 * The pinned remainder (the content of the area a delay exposes PD-10, a MOB in it PD-11, the delay range PD-12, no dependence of the bus-window schedule
 * on the vertical delay PD-13) is asserted by the default-pinned ids at the end of this file and by video.vdelay.no-bus-window-change: each pin asserts
 * the default and then flips ONE field of a stack copy of ivs_stic_params, or one existing field for the schedule, and requires a different result. */
#include <stdint.h>
#include <string.h>

#include "video_cases.h"

#define VD_H_REG 0x30u
#define VD_V_REG 0x31u
#define VD_EXT_REG 0x32u
#define VD_BLANK 0xFFu

static uint8_t vd_at(uint32_t x, uint32_t y)
{
  return vm_plane_buf()[y * VF_PLANE_W + x];
}

/* Writes the three registers (a raw poke of the register file: delays in plane positions, the extension bits) into the scene just built. */
static void vd_regs(uint32_t h, uint32_t v, uint32_t ext)
{
  ivs_memsys *m = vm_mem_get();
  m->stic_reg[VD_H_REG] = (uint16_t)h;
  m->stic_reg[VD_V_REG] = (uint16_t)v;
  m->stic_reg[VD_EXT_REG] = (uint16_t)ext;
}

/* The scene of F-M0 (the cards: color 1 everywhere, the 5 x 5 block of color 2 at x 80..84, y 48..52, border 8) with the delays h and v, no strip, no MOB. */
static const ivs_stic_params *vd_scene0(uint32_t h, uint32_t v)
{
  const ivs_stic_params *p = vm_scene(vm_frame_get(0u));
  vd_regs(h, v, 0u);
  return p;
}

/* The scene of a delay frame of the oracle (derivation (a) A7): 5 is scene 5 (the `hdelay` selector: horizontal delay 3, vertical 0, extension 3), 6 is
 * scene 6 (the `vdelay` selector: horizontal 0, vertical 3, extension 3) and 7 is the in-process frame F-M7 (horizontal 2, vertical 3, extension 0).
 * MOB0 is `0644 0130 080C` in all three: visible, X coordinate 68, Y size code 01, Y coordinate 48, graphics RAM card 1 (the staircase), color 4. */
static const ivs_stic_params *vd_scene_fm(uint32_t k)
{
  const ivs_stic_params *p = vm_scene(vm_frame_get(k));
  vm_put(0u, 0x0644u, 0x0130u, 0x080Cu);
  if (k == 5u) {
    vd_regs(3u, 0u, 3u);
  } else if (k == 6u) {
    vd_regs(0u, 3u, 3u);
  } else {
    vd_regs(2u, 3u, 0u);
  }
  return p;
}

/* ---- video.hdelay.shift-right-cards-and-mobs: scene 5 of the program is the frozen literal F-M5, and every step moves the card and the MOB one column ---- */

static int vd_hdelay_shift(void)
{
  const vm_frame *f = vm_frame_get(5u);
  const ivs_stic_params *p;
  uint32_t h;
  uint32_t y;
  uint32_t x;
  VC_CHECK(f != NULL && vm_fixture_ok(f));
  p = vd_scene_fm(5u);
  VC_CHECK(p != NULL && vm_render(p, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
  VC_CHECK(vm_matches(f)); /* every published position and the hash of the literal */
  /* the pixel probes of derivation (e) for F-M5, typed here: the MOB (x0 60 moved to 63) and the card (x 80..84 moved to 83..87) */
  VC_CHECK(vd_at(63u, 40u) == 4u && vd_at(65u, 40u) == 1u && vd_at(83u, 48u) == 2u && vd_at(155u, 50u) == 1u && vd_at(100u, 60u) == 1u);
  /* the shift of one marked position per step (derivation (e), item 5 of the request): the top left set pixel of card 2 at (80, 48) moves to
   * (80 + h, 48) under a horizontal delay h, the block of 25 set pixels stays whole (x 80 + h .. 84 + h, inside its own cell for h at most 3), and
   * the exposed columns 0 .. h - 1 of every row (96 x h published positions, nothing else) show the not-displayed value */
  for (h = 1u; h <= 3u; h++) {
    p = vd_scene0(h, 0u);
    VC_CHECK(p != NULL && vm_render(p, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
    VC_CHECK(vd_at(79u + h, 48u) == 1u && vd_at(80u + h, 48u) == 2u && vd_at(84u + h, 52u) == 2u && vd_at(85u + h, 52u) == 1u);
    VC_CHECK(vm_count(2u) == 25u && vm_count(VD_BLANK) == 96u * h);
    for (y = 0u; y < VF_PLANE_H; y++) {
      for (x = 0u; x < h; x++) {
        VC_CHECK(vd_at(x, y) == VD_BLANK);
      }
      VC_CHECK(vd_at(h, y) != VD_BLANK && vd_at(158u, y) != VD_BLANK);
    }
  }
  return IVS_TEST_PASS;
}

/* ---- video.hdelay.clip-right: what the shift moves past the right edge of the plane is not published, and the cut-off column stays blank ---- */

/* A solid color 4 MOB (X coordinate 158, Y coordinate 64 with the size code 01: pattern pixels at x 150..157, y 56..63 undelayed) and, in the last card
 * column, a solid card of foreground color 5 at the card cell (5, 19) (x 152..159, y 40..47, of which the published plane holds x 152..158: 7 columns
 * because the 20th card column is cut off; the two do not overlap). Under a delay of h columns the MOB covers x 150 + h .. 157 + h and the card
 * x 152 + h .. 159 + h, and only the positions up to x 158 are published. Derived by hand, 8 rows each:
 *   h   MOB columns published   MOB positions    card columns published   card positions
 *   0   150..157 (8)            64               152..158 (7)             56
 *   1   151..158 (8)            64               153..158 (6)             48
 *   2   152..158 (7)            56               154..158 (5)             40
 *   3   153..158 (6)            48               155..158 (4)             32
 *   7   157..158 (2)            16               159.. (0)                0 */
typedef struct vd_clip_row {
  uint32_t h;
  uint32_t mob;
  uint32_t card;
} vd_clip_row;

static const vd_clip_row vd_clip_rows[] = {{0u, 64u, 56u}, {1u, 64u, 48u}, {2u, 56u, 40u}, {3u, 48u, 32u}, {7u, 16u, 0u}};

static int vd_hdelay_clip_right(void)
{
  uint32_t t;
  uint32_t y;
  for (t = 0u; t < sizeof vd_clip_rows / sizeof vd_clip_rows[0]; t++) {
    const vd_clip_row *r = &vd_clip_rows[t];
    const ivs_stic_params *p = vd_scene0(r->h, 0u);
    VC_CHECK(p != NULL);
    vm_state_get()->row_latch[5][19] = 0x0805u; /* Color Stack, graphics RAM card 0 (solid), foreground 5 */
    vm_put(0u, 0x0200u + 158u, 0x0100u + 64u, 0x0804u);
    VC_CHECK(vm_render(p, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
    if (vm_count(4u) != r->mob || vm_count(5u) != r->card) {
      ivs_test_diag("horizontal delay %u: %u positions of the MOB and %u of the card, expected %u and %u", (unsigned)r->h, (unsigned)vm_count(4u),
                    (unsigned)vm_count(5u), (unsigned)r->mob, (unsigned)r->card);
      return IVS_TEST_FAIL;
    }
    if (r->h >= 1u) {
      VC_CHECK(vd_at(158u, 56u) == 4u && vd_at(158u, 63u) == 4u); /* the last published column holds the MOB */
    }
    for (y = 0u; y < VF_PLANE_H; y++) {
      VC_CHECK(vd_at(159u, y) == VD_BLANK); /* the cut-off column of the working plane is never a delayed pixel */
    }
  }
  return IVS_TEST_PASS;
}

/* ---- video.vdelay.shift-down-cards-and-mobs: scene 6 of the program is the frozen literal F-M6, every step moves the card and the MOB one row, and the bottom clips ---- */

/* The same shapes at the bottom edge: a solid color 4 MOB (X coordinate 40, Y coordinate 98: pattern pixels at x 32..39, y 90..97 undelayed, of which
 * y 90..95 are published) and a solid card of foreground color 5 at the card cell (11, 5) (x 40..47, y 88..95). Under a delay of v rows the MOB covers
 * y 90 + v .. 97 + v and the card y 88 + v .. 95 + v, published up to y 95. Derived by hand, 8 columns each:
 *   v   MOB rows published   MOB positions   card rows published   card positions
 *   0   90..95 (6)           48              88..95 (8)             64
 *   1   91..95 (5)           40              89..95 (7)             56
 *   3   93..95 (3)           24              91..95 (5)             40
 *   7   97.. (0)             0               95..95 (1)             8 */
typedef struct vd_down_row {
  uint32_t v;
  uint32_t mob;
  uint32_t card;
} vd_down_row;

static const vd_down_row vd_down_rows[] = {{0u, 48u, 64u}, {1u, 40u, 56u}, {3u, 24u, 40u}, {7u, 0u, 8u}};

static int vd_vdelay_shift(void)
{
  const vm_frame *f = vm_frame_get(6u);
  const ivs_stic_params *p;
  uint32_t v;
  uint32_t t;
  uint32_t x;
  VC_CHECK(f != NULL && vm_fixture_ok(f));
  p = vd_scene_fm(6u);
  VC_CHECK(p != NULL && vm_render(p, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
  VC_CHECK(vm_matches(f)); /* every published position and the hash of the literal */
  /* the pixel probes of derivation (e) for F-M6, typed here: the MOB (y0 40 moved to 43) and the card (y 48..52 moved to 51..55) */
  VC_CHECK(vd_at(60u, 43u) == 4u && vd_at(62u, 43u) == 1u && vd_at(80u, 48u) == 1u && vd_at(80u, 51u) == 2u && vd_at(150u, 90u) == 1u);
  /* the shift of one marked position per step: (80, 48) moves to (80, 48 + v); the exposed rows 0 .. v - 1 (159 x v published positions) are blank */
  for (v = 1u; v <= 3u; v++) {
    p = vd_scene0(0u, v);
    VC_CHECK(p != NULL && vm_render(p, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
    VC_CHECK(vd_at(80u, 47u + v) == 1u && vd_at(80u, 48u + v) == 2u && vd_at(84u, 52u + v) == 2u && vd_at(84u, 53u + v) == 1u);
    VC_CHECK(vm_count(2u) == 25u && vm_count(VD_BLANK) == 159u * v);
    for (x = 0u; x < VF_PLANE_W - 1u; x++) {
      VC_CHECK(vd_at(x, v - 1u) == VD_BLANK && vd_at(x, v) != VD_BLANK);
    }
  }
  /* what the shift moves past the bottom edge is not published */
  for (t = 0u; t < sizeof vd_down_rows / sizeof vd_down_rows[0]; t++) {
    const vd_down_row *r = &vd_down_rows[t];
    p = vd_scene0(0u, r->v);
    VC_CHECK(p != NULL);
    vm_state_get()->row_latch[11][5] = 0x0805u;
    vm_put(0u, 0x0200u + 40u, 0x0100u + 98u, 0x0804u);
    VC_CHECK(vm_render(p, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
    if (vm_count(4u) != r->mob || vm_count(5u) != r->card) {
      ivs_test_diag("vertical delay %u: %u positions of the MOB and %u of the card, expected %u and %u", (unsigned)r->v, (unsigned)vm_count(4u),
                    (unsigned)vm_count(5u), (unsigned)r->mob, (unsigned)r->card);
      return IVS_TEST_FAIL;
    }
  }
  return IVS_TEST_PASS;
}

/* ---- video.vdelay.straddle-card-rows: the band of rows a vertical delay moves across a card row boundary is carried to the next card row ---- */

/* A Foreground/Background scene whose card row r (0 .. 11) is one color, r + 1 (every cell the word of a graphics ROM card 0 over the zero graphics ROM,
 * so every pixel is the background color), under a vertical delay v. Output row y shows the source row y - v, that is the color of card row
 * (y - v) div 8 for y at least v, and the not-displayed value above. So the band of color k + 1 covers the output rows 8 k + v .. 8 k + v + 7
 * (the last band, color 12, only 8 - v rows), and every card row boundary of the source is crossed inside the band of the card row above it. The probes
 * below were derived by hand for each v: the last exposed row (v - 1), the first row of color 1 (v) and its last (v + 7), the first of color 2 (v + 8)
 * and its last (v + 15), the first of color 3 (v + 16), the last row of color 11 (87 + v) and the first of color 12 (88 + v), and the last plane
 * row 95 (color 12). */
typedef struct vd_straddle_row {
  uint32_t v;
  uint32_t y[9];
  uint32_t colour[9];
} vd_straddle_row;

static const vd_straddle_row vd_straddle_rows[] = {
    {1u, {0u, 1u, 8u, 9u, 16u, 17u, 88u, 89u, 95u}, {255u, 1u, 1u, 2u, 2u, 3u, 11u, 12u, 12u}},
    {2u, {1u, 2u, 9u, 10u, 17u, 18u, 89u, 90u, 95u}, {255u, 1u, 1u, 2u, 2u, 3u, 11u, 12u, 12u}},
    {3u, {2u, 3u, 10u, 11u, 18u, 19u, 90u, 91u, 95u}, {255u, 1u, 1u, 2u, 2u, 3u, 11u, 12u, 12u}},
    {4u, {3u, 4u, 11u, 12u, 19u, 20u, 91u, 92u, 95u}, {255u, 1u, 1u, 2u, 2u, 3u, 11u, 12u, 12u}},
    {5u, {4u, 5u, 12u, 13u, 20u, 21u, 92u, 93u, 95u}, {255u, 1u, 1u, 2u, 2u, 3u, 11u, 12u, 12u}},
    {6u, {5u, 6u, 13u, 14u, 21u, 22u, 93u, 94u, 95u}, {255u, 1u, 1u, 2u, 2u, 3u, 11u, 12u, 12u}},
    {7u, {6u, 7u, 14u, 15u, 22u, 23u, 94u, 95u, 95u}, {255u, 1u, 1u, 2u, 2u, 3u, 11u, 12u, 12u}},
};

static int vd_vdelay_straddle(void)
{
  uint32_t t;
  uint32_t k;
  uint32_t r;
  uint32_t c;
  for (t = 0u; t < sizeof vd_straddle_rows / sizeof vd_straddle_rows[0]; t++) {
    const vd_straddle_row *row = &vd_straddle_rows[t];
    const ivs_stic_params *p = vd_scene0(0u, row->v);
    ivs_stic_state *s = vm_state_get();
    VC_CHECK(p != NULL);
    s->mode = 0u; /* Foreground/Background */
    for (r = 0u; r < 12u; r++) {
      for (c = 0u; c < 20u; c++) {
        s->row_latch[r][c] = (uint16_t)vid_encode_fgbg(0u, r + 1u, 0u, 0u);
      }
    }
    VC_CHECK(vm_render(p, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
    for (k = 0u; k < 9u; k++) {
      uint32_t x;
      for (x = 0u; x < VF_PLANE_W - 1u; x++) {
        if (vd_at(x, row->y[k]) != row->colour[k]) {
          ivs_test_diag("vertical delay %u: position (%u,%u) is %u, expected %u", (unsigned)row->v, (unsigned)x, (unsigned)row->y[k],
                        (unsigned)vd_at(x, row->y[k]), (unsigned)row->colour[k]);
          return IVS_TEST_FAIL;
        }
      }
    }
    /* the rows of every color: 159 x 8 positions of each of the colors 1 to 11, 159 x (8 - v) of the color 12, 159 x v blank */
    for (k = 1u; k <= 11u; k++) {
      VC_CHECK(vm_count(k) == 159u * 8u);
    }
    VC_CHECK(vm_count(12u) == 159u * (8u - row->v) && vm_count(VD_BLANK) == 159u * row->v);
  }
  return IVS_TEST_PASS;
}

/* ---- video.vdelay.no-bus-window-change: the bus-window schedule and the interrupt level do not depend on the vertical delay (D-07, C92, PD-13) ---- */

#define VD_SWEEP (14934u + 1u) /* one sample per CPU cycle (4 ticks) of one frame, and the frame end */

/* One frame of timeline with the vertical delay register at v: the cursor, the latches and the two line levels sampled every CPU cycle. The display latch
 * is set a cycle into the frame (a strobe inside the register window), so the 13 bus windows are scheduled. */
static uint32_t vd_trace(const ivs_stic_params *p, uint32_t v, uint32_t *rec)
{
  ivs_stic_state s;
  ivs_memsys *m = vm_mem_get();
  uint32_t k;
  uint32_t busy = 0u;
  (void)vm_scene(vm_frame_get(0u));
  m->stic_reg[VD_V_REG] = (uint16_t)v;
  memset(&s, 0, sizeof s);
  for (k = 0u; k < VD_SWEEP; k++) {
    uint64_t tick = 4u * (uint64_t)k;
    ivs_stic_line_levels l;
    ivs_stic_catch_up(p, &s, m, tick, NULL, NULL);
    if (k == 1u) {
      s.display_latch = 1u; /* the strobe of the guest, inside the register window of the frame: the latch is read at the end of the graphics window */
    }
    l = ivs_stic_lines(p, &s, tick);
    rec[k] = l.intrm | (l.busrq << 1) | (s.event_index << 2) | (s.windows_active << 8) | (s.display_latch << 9) | (uint32_t)((s.frame_index & 0xFFu) << 10);
    busy += l.busrq;
  }
  return busy;
}

/* The machine form: a guest writes the vertical delay (0 or 7), clears the register it used and strobes the display in an endless loop, so the frames
 * are displayed and the bus windows steal cycles from the CPU. The extra word of the control adds one instruction ahead of the loop. */
static size_t vd_machine_guest(uint16_t *w, uint32_t v, int extra)
{
  size_t n = 0u;
  size_t start;
  n = sc_w_mvii(w, n, 1u, v);
  n = sc_w_mvo(w, n, 1u, VD_V_REG);
  n = sc_w_mvii(w, n, 1u, 0u); /* R1 is 0 again in both runs */
  if (extra != 0) {
    n = sc_w_mvii(w, n, 2u, 0u); /* the control: one more instruction, so the loop is out of phase with the windows */
  }
  start = n;
  n = sc_w_mvo(w, n, 0u, 0x0020u);
  w[n] = 0x0220u; /* B back to the strobe */
  w[n + 1u] = (uint16_t)(n + 1u - start);
  return n + 2u;
}

/* The state a machine run leaves that depends on time and on the bus schedule: machine time, the CPU, the STIC state, the PSG state and the data memory
 * (system RAM, scratchpad, graphics RAM, the PSG registers and the STIC register file but for the one word the two runs are allowed to differ in, the
 * vertical delay register). The program image itself differs by one immediate (the delay value the guest writes), so the system-image slots are not
 * compared. 1 when two instances agree on all of it. */
static int vd_machine_equal(const ivs_ms_inst *a, const ivs_ms_inst *b)
{
  const struct ivs_machine *ma = ivs_machine_from_handle(a->m);
  const struct ivs_machine *mb = ivs_machine_from_handle(b->m);
  const size_t before = sizeof(uint16_t) * VD_V_REG;
  return ma->time_ticks == mb->time_ticks && memcmp(&ma->cpu, &mb->cpu, sizeof ma->cpu) == 0 && memcmp(&ma->stic, &mb->stic, sizeof ma->stic) == 0 &&
         memcmp(&ma->psg, &mb->psg, sizeof ma->psg) == 0 && memcmp(ma->mem.sysram, mb->mem.sysram, sizeof ma->mem.sysram) == 0 &&
         memcmp(ma->mem.scratchpad, mb->mem.scratchpad, sizeof ma->mem.scratchpad) == 0 && memcmp(ma->mem.gram, mb->mem.gram, sizeof ma->mem.gram) == 0 &&
         memcmp(ma->mem.psg_reg, mb->mem.psg_reg, sizeof ma->mem.psg_reg) == 0 && memcmp(ma->mem.stic_reg, mb->mem.stic_reg, before) == 0 &&
         memcmp(ma->mem.stic_reg + VD_V_REG + 1u, mb->mem.stic_reg + VD_V_REG + 1u, sizeof ma->mem.stic_reg - before - sizeof(uint16_t)) == 0;
}

static int vd_machine_run(ivs_ms_inst *x, uint32_t v, int extra)
{
  uint16_t w[40];
  size_t n = vd_machine_guest(w, v, extra);
  memset(x, 0, sizeof *x);
  return sc_guest_n(x, w, n) && sc_run_until(x, 3u * VM_F + 4000u);
}

static int vd_no_bus_window_change(void)
{
  static uint32_t r0[VD_SWEEP];
  static uint32_t r7[VD_SWEEP];
  static uint32_t rq[VD_SWEEP];
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  ivs_stic_params q;
  uint32_t k;
  uint32_t busy0;
  uint32_t busy7;
  uint32_t differ = 0u;
  ivs_ms_inst a;
  ivs_ms_inst b;
  ivs_ms_inst c;
  int ok = 0;
  VC_CHECK(p != NULL && p->bus_count == 13u && p->frame_ticks == 59736u);
  /* the timeline of one frame with the vertical delay register at 0 and at 7: the cursor, the latches and the levels agree at every sample, and the
   * windows are really there (13 windows of the hold length, in samples of one CPU cycle each) */
  busy0 = vd_trace(p, 0u, r0);
  busy7 = vd_trace(p, 7u, r7);
  VC_CHECK(busy0 == 13u * (p->bus_hold_ticks / 4u) && busy7 == busy0);
  VC_CHECK(memcmp(r0, r7, sizeof r0) == 0);
  /* the one-field control that shows the sweep can fail: the existing field stic.bus.first_start_cpu_cycles (here its tick form, 40 cycles later) changed
   * in a stack copy of the parameters moves every window, and the same sweep sees it */
  q = *p;
  q.bus_first_start_cycles += 40u;
  q.bus_first_start_ticks += 160u;
  (void)vd_trace(&q, 0u, rq);
  for (k = 0u; k < VD_SWEEP; k++) {
    differ += r0[k] != rq[k] ? 1u : 0u;
  }
  VC_CHECK(differ != 0u);
  /* the machine form: the guest that writes 0 and the guest that writes 7 have the same time, CPU, STIC and PSG state and memory (but for the delay word)
   * after three frames; a guest with one more instruction does not (the comparison can fail) */
  memset(&b, 0, sizeof b);
  memset(&c, 0, sizeof c);
  if (!vd_machine_run(&a, 0u, 0) || !vd_machine_run(&b, 7u, 0) || !vd_machine_run(&c, 0u, 1)) {
    ivs_test_diag("a machine run did not complete");
    goto done;
  }
  if (!vd_machine_equal(&a, &b)) {
    ivs_test_diag("the runs with vertical delay 0 and 7 differ in machine time, CPU, STIC, PSG or memory state");
    goto done;
  }
  if (vd_machine_equal(&a, &c)) {
    ivs_test_diag("the control run with one more instruction equals the reference: the comparison cannot fail");
    goto done;
  }
  ok = 1;
done:
  if (a.storage != NULL) {
    ivs_ms_free(&a);
  }
  if (b.storage != NULL) {
    ivs_ms_free(&b);
  }
  if (c.storage != NULL) {
    ivs_ms_free(&c);
  }
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- video.delay.stream-equals-plane: the streamed slot of ivs_stic_frame_deliver is the whole plane of ivs_stic_render_plane, under delay and strips ---- */

static int vd_stream_one(uint32_t k)
{
  static uint8_t slot[VM_SLOT_BYTES + 64u];
  const ivs_stic_params *p = vd_scene_fm(k);
  ivs_stic_state sa;
  ivs_stic_state sb;
  ivs_stic_state sc;
  ivs_stic_video_row ra;
  ivs_stic_video_row rb;
  ivs_stic_video_row rc;
  ivs_psg_sink ka;
  ivs_psg_sink kc;
  uint32_t y;
  VC_CHECK(p != NULL && vm_render(p, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
  VC_CHECK(p->video_slot_bytes == VM_SLOT_BYTES && p->video_width == 159u && p->video_height == 96u);
  vm_state_get()->windows_active = 1u; /* the frame is displayed */
  sa = *vm_state_get();
  sb = *vm_state_get();
  sc = *vm_state_get();
  ivs_video_chain_init(&ra);
  ivs_video_chain_init(&rb);
  ivs_video_chain_init(&rc);
  memset(slot, (int)VP_POISON, sizeof slot);
  ivs_psg_sink_init(&ka, NULL, 0u);
  ivs_psg_sink_video(&ka, slot, VM_SLOT_BYTES);
  ivs_stic_frame_deliver(p, &sa, vm_mem_get(), &ra, &ka, VM_F); /* with a video buffer */
  VC_CHECK(ka.video_frames == 1u && ka.video_written == VM_SLOT_BYTES && ka.video_overflow == 0u && ra.count == 1u);
  VC_CHECK(slot[VM_SLOT_BYTES] == (uint8_t)VP_POISON && slot[27] == 1u);
  for (y = 0u; y < VF_PLANE_H; y++) {
    if (memcmp(slot + 32u + (size_t)y * 159u, vm_plane_buf() + (size_t)y * VF_PLANE_W, 159u) != 0) {
      ivs_test_diag("scene F-M%u: the streamed slot differs from the whole plane in plane row %u", (unsigned)k, (unsigned)y);
      return IVS_TEST_FAIL;
    }
  }
  ivs_stic_frame_deliver(p, &sb, vm_mem_get(), &rb, NULL, VM_F); /* with no sink at all */
  ivs_psg_sink_init(&kc, NULL, 0u);
  ivs_stic_frame_deliver(p, &sc, vm_mem_get(), &rc, &kc, VM_F); /* with the counting sink: no buffer */
  VC_CHECK(rb.count == 1u && rc.count == 1u && kc.video_frames == 1u && kc.video_written == 0u);
  VC_CHECK(memcmp(ra.chain, rb.chain, 32u) == 0 && memcmp(ra.chain, rc.chain, 32u) == 0); /* the chain does not depend on a buffer */
  VC_CHECK(memcmp(&sa, &sb, sizeof sa) == 0 && memcmp(&sa, &sc, sizeof sa) == 0);
  return IVS_TEST_PASS;
}

static int vd_stream_equals_plane(void)
{
  /* F-M5 and F-M6 (delay with strips) and F-M7 (both delays and the exposed area, no strips): non-vacuity, each plane has blank positions or a strip */
  VC_CHECK(vd_stream_one(5u) == IVS_TEST_PASS);
  VC_CHECK(vd_at(0u, 0u) == 8u && vd_at(100u, 3u) == 8u);
  VC_CHECK(vd_stream_one(6u) == IVS_TEST_PASS);
  VC_CHECK(vd_at(0u, 0u) == 8u);
  VC_CHECK(vd_stream_one(7u) == IVS_TEST_PASS);
  VC_CHECK(vd_at(0u, 0u) == VD_BLANK && vd_at(1u, 50u) == VD_BLANK && vd_at(50u, 2u) == VD_BLANK && vd_at(2u, 3u) == 1u);
  return IVS_TEST_PASS;
}

/* ---- video.delay.slice-invariant: the host's call sizes change no byte of a run that draws with both delays ---- */

/* The stand-in guest (decles derived in tests/stic/stic_cases.h): the staircase card 1 written byte by byte, the registers of the in-process frame
 * F-M7 (the MOB `0644 0130 080C`, horizontal delay 2, vertical delay 3, no extension bit), a read of $0021 (Color Stack), then the display strobe in an
 * endless loop, so every frame from the first strobe on is displayed. */
static size_t vd_slice_guest(uint16_t *w)
{
  static const uint8_t stairs[8] = {0x80u, 0xC0u, 0xE0u, 0xF0u, 0xF8u, 0xFCu, 0xFEu, 0xFFu};
  static const uint16_t regs[6][2] = {{0x0000u, 0x0644u}, {0x0008u, 0x0130u}, {0x0010u, 0x080Cu}, {0x0030u, 0x0002u}, {0x0031u, 0x0003u}, {0x0032u, 0x0000u}};
  size_t n = 0u;
  size_t start;
  uint32_t c;
  for (c = 0u; c < 8u; c++) {
    n = sc_w_mvii(w, n, 0u, stairs[c]);
    n = sc_w_mvo(w, n, 0u, 0x3808u + c); /* graphics RAM card 1 */
  }
  for (c = 0u; c < 6u; c++) {
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

static int vd_slice_invariant(void)
{
  static uint16_t w[400];
  static uint8_t ref[VM_SLOT_BYTES * 5u];
  static const uint64_t sizes[4] = {1u, 7u, 468u, 97u};
  ivs_ms_inst a;
  uint64_t ref_bytes;
  uint32_t k;
  int ok = 0;
  size_t n = vd_slice_guest(w);
  const uint8_t *last;
  VC_CHECK(n < sizeof w / sizeof w[0]);
  memset(&a, 0, sizeof a);
  ref_bytes = vm_run_sliced(&a, w, n, 0u, ref); /* the reference: one call to the same end */
  if (ref_bytes != 3u * VM_SLOT_BYTES) {
    ivs_test_diag("the reference run delivered %llu bytes, expected %u", (unsigned long long)ref_bytes, (unsigned)(3u * VM_SLOT_BYTES));
    goto done;
  }
  /* non-vacuity: the guest left the delay registers, and the last frame shows the delay: the MOB (x0 60, y0 40) moved to (62, 43) draws color 4 at its
   * first row (62, 43) and (63, 43) and not at the undelayed (60, 40), and the exposed columns 0 and 1 and rows 0 to 2 are the not-displayed value */
  if (ivs_machine_from_handle(a.m)->mem.stic_reg[VD_H_REG] != 2u || ivs_machine_from_handle(a.m)->mem.stic_reg[VD_V_REG] != 3u) {
    ivs_test_diag("the guest did not leave the delay registers in the register file");
    goto done;
  }
  last = ref + 2u * VM_SLOT_BYTES + 32u;
  if (last[43u * 159u + 62u] != 4u || last[43u * 159u + 63u] != 4u || last[40u * 159u + 60u] == 4u || last[10u * 159u + 0u] != VD_BLANK ||
      last[10u * 159u + 1u] != VD_BLANK || last[1u * 159u + 50u] != VD_BLANK || last[3u * 159u + 2u] == VD_BLANK) {
    ivs_test_diag("the last delivered frame does not show the delayed picture");
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

/* ---- the pinned defaults (D-02, T-08-17): each test asserts the default and then changes ONE field of a stack copy of ivs_stic_params (or, for the range,
 * holds the width of the register) and requires a different plane. ---- */

/* video.hdelay.default-pinned.range: the delay range is the three-bit register width (R47, PD-12, C110, resolver D10-VID-12). There is no field for it: the
 * width is a row of the register literal (write mask 0007 of $0030 and $0031), so the one-field controls are the existing self-control
 * stic.judge.register-row-changed (a changed register row makes the comparison fail), stic.reg.masks (the write masks of the 64 registers) and
 * stic.table.matches-literal (the generated table equals the literal). This test asserts the masked behavior through the renderer: a CPU store keeps only
 * the low three bits (a store of 8 reads and applies as 0, of 11 as 3, of 15 as 7), a word poked into the register file past the write mask applies
 * under the same three bits, and the largest delay (7) is covered by the strips of 8 (no blank position is published with both strips). Each applied delay
 * d moves the marked pixel (80, 48) of card 2 to (80 + d, 48) under the horizontal register and to (80, 48 + d) under the vertical one and exposes d columns
 * (96 x d published positions) or d rows (159 x d). */
static int vd_pin_range(void)
{
  static const uint32_t words[] = {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 11u, 15u, 0x13u, 0xFFFFu};
  uint32_t reg;
  uint32_t t;
  uint32_t via;
  for (reg = VD_H_REG; reg <= VD_V_REG; reg++) {
    for (t = 0u; t < sizeof words / sizeof words[0]; t++) {
      for (via = 0u; via < 2u; via++) {
        uint32_t d = words[t] & 7u;
        const ivs_stic_params *p = vd_scene0(0u, 0u);
        VC_CHECK(p != NULL);
        if (via == 0u) {
          ivs_memsys_write(vm_mem_get(), (uint16_t)reg, (uint16_t)words[t]); /* the CPU store, under the write mask 0007 */
          VC_CHECK(vm_mem_get()->stic_reg[reg] == (uint16_t)d);
        } else {
          vm_mem_get()->stic_reg[reg] = (uint16_t)words[t]; /* a raw word past the write mask: the renderer applies the low three bits only */
        }
        VC_CHECK(vm_render(p, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
        VC_CHECK(vm_count(2u) == 25u);
        if (reg == VD_H_REG) {
          VC_CHECK(vd_at(80u + d, 48u) == 2u && vm_count(VD_BLANK) == 96u * d);
        } else {
          VC_CHECK(vd_at(80u, 48u + d) == 2u && vm_count(VD_BLANK) == 159u * d);
        }
      }
    }
  }
  /* the largest delay in both registers with both strips: the exposed columns 0..6 and rows 0..6 lie under the strips (the stated purpose of the strips) */
  {
    const ivs_stic_params *p = vd_scene0(7u, 7u);
    VC_CHECK(p != NULL);
    vm_mem_get()->stic_reg[VD_EXT_REG] = 3u;
    VC_CHECK(vm_render(p, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
    VC_CHECK(vm_count(VD_BLANK) == 0u && vd_at(0u, 0u) == 8u && vd_at(7u, 7u) == 8u && vd_at(8u, 8u) == 1u);
  }
  return IVS_TEST_PASS;
}

/* The in-process frame F-M7 (both delays, no strip) rendered with the parameters p; the literal compared on its known positions, which must hold under
 * the default and under the flip alike (the flip changes only the unknown positions). */
static int vd_render_fm7(const ivs_stic_params *p, uint32_t *skipped)
{
  const vm_frame *f = vm_frame_get(7u);
  (void)vd_scene_fm(7u);
  vm_expected(f, vm_exp_buf());
  return vm_render(p, 1u, vm_plane_buf()) == VF_PLANE_BYTES && vm_first_diff(vm_plane_buf(), vm_exp_buf(), skipped) == -1;
}

/* The exposed area pinned by F-M7 (R47, PD-10, C110, field video.delay_exposed_fill, resolver D10-VID-12): the area a delay exposes is the not-displayed
 * value by default and the border color with the field flipped. focus 0 is the exposed columns (`unknown 0 0 2 96`: x 0 and 1 of every row, 192
 * positions), focus 1 the exposed rows (`unknown 2 0 157 3`: y 0 to 2 of x 2 to 158, 471 positions); the known positions next to them are the literal's. */
static int vd_exposed_pin(uint32_t focus)
{
  const vm_frame *f = vm_frame_get(7u);
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  ivs_stic_params q;
  uint8_t *base = vm_exp_buf();
  uint32_t skipped = 0u;
  uint32_t x;
  uint32_t y;
  uint32_t diff = 0u;
  VC_CHECK(f != NULL && p != NULL && p->video_delay_exposed_fill == 0u && vm_fixture_ok(f));
  VC_CHECK(vd_render_fm7(p, &skipped));
  VC_CHECK(skipped == 192u + 471u); /* the two unknown rectangles of the literal */
  /* the default: the not-displayed value in every unknown position and nowhere else */
  VC_CHECK(vm_count(VD_BLANK) == 663u);
  if (focus == 0u) {
    for (y = 0u; y < VF_PLANE_H; y++) {
      VC_CHECK(vd_at(0u, y) == VD_BLANK && vd_at(1u, y) == VD_BLANK);
    }
    for (y = 3u; y < VF_PLANE_H; y++) {
      VC_CHECK(vd_at(2u, y) == 1u); /* the first known column: the card color, shifted in */
    }
  } else {
    for (x = 2u; x < VF_PLANE_W - 1u; x++) {
      VC_CHECK(vd_at(x, 0u) == VD_BLANK && vd_at(x, 1u) == VD_BLANK && vd_at(x, 2u) == VD_BLANK && vd_at(x, 3u) == 1u);
    }
  }
  memcpy(base, vm_plane_buf(), VF_PLANE_BYTES);
  q = *p;
  q.video_delay_exposed_fill = 1u; /* the one-field flip: the border color (8) instead of the not-displayed value */
  VC_CHECK(vd_render_fm7(&q, &skipped)); /* the known positions are unchanged */
  VC_CHECK(vm_planes_differ(vm_plane_buf(), base));
  VC_CHECK(vm_count(8u) == 663u && vm_count(VD_BLANK) == 0u);
  for (y = 0u; y < VF_PLANE_H; y++) {
    for (x = 0u; x < VF_PLANE_W - 1u; x++) {
      diff += vd_at(x, y) != base[y * VF_PLANE_W + x] ? 1u : 0u;
    }
  }
  VC_CHECK(diff == 663u); /* the flip changes exactly the exposed positions */
  if (focus == 0u) {
    for (y = 0u; y < VF_PLANE_H; y++) {
      VC_CHECK(vd_at(0u, y) == 8u && vd_at(1u, y) == 8u && vd_at(2u, y >= 3u ? y : 3u) == 1u);
    }
  } else {
    for (x = 2u; x < VF_PLANE_W - 1u; x++) {
      VC_CHECK(vd_at(x, 0u) == 8u && vd_at(x, 2u) == 8u && vd_at(x, 3u) == 1u);
    }
  }
  return IVS_TEST_PASS;
}

static int vd_pin_hdelay_exposed(void)
{
  return vd_exposed_pin(0u);
}

static int vd_pin_vdelay_exposed(void)
{
  return vd_exposed_pin(1u);
}

/* video.vdelay.default-pinned.mob-in-exposed: a MOB in the exposed area (R47, PD-11, C110, field video.delay_mob_in_exposed, resolver D10-VID-12). A solid
 * color 4 MOB whose pattern lies partly in the margin of the object field (8 positions above and left of the plane) is shifted into the exposed area by the
 * delay: the pixels that come from inside the plane are drawn, the pixels from the margin are not drawn by default (the exposed positions stay the
 * not-displayed value) and are drawn with the field flipped. Derived by hand (R37, R46), 8 x 8 positions of the pattern:
 *   vertical: X 20, Y 5 (x 12..19, source rows -3..4), delay 3 rows: output rows 0..7; rows 0..2 are exposed: default 5 x 8 = 40 positions, flip 64
 *   horizontal: X 5, Y 40 (source columns -3..4, rows 32..39), delay 3 columns: output columns 0..7; columns 0..2 are exposed: default 40, flip 64 */
static int vd_pin_mob_in_exposed(void)
{
  const ivs_stic_params *p;
  ivs_stic_params q;
  uint32_t x;
  uint32_t y;
  uint32_t part;
  for (part = 0u; part < 2u; part++) {
    p = vd_scene0(part == 0u ? 0u : 3u, part == 0u ? 3u : 0u);
    VC_CHECK(p != NULL && p->video_delay_mob_in_exposed == 0u);
    if (part == 0u) {
      vm_put(0u, 0x0200u + 20u, 0x0100u + 5u, 0x0804u);
    } else {
      vm_put(0u, 0x0200u + 5u, 0x0100u + 40u, 0x0804u);
    }
    VC_CHECK(vm_render(p, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
    VC_CHECK(vm_count(4u) == 40u); /* the default: only the pixels from inside the plane */
    if (part == 0u) {
      for (x = 12u; x <= 19u; x++) {
        VC_CHECK(vd_at(x, 0u) == VD_BLANK && vd_at(x, 2u) == VD_BLANK && vd_at(x, 3u) == 4u && vd_at(x, 7u) == 4u && vd_at(x, 8u) == 1u);
      }
    } else {
      for (y = 32u; y <= 39u; y++) {
        VC_CHECK(vd_at(0u, y) == VD_BLANK && vd_at(2u, y) == VD_BLANK && vd_at(3u, y) == 4u && vd_at(7u, y) == 4u && vd_at(8u, y) == 1u);
      }
    }
    memcpy(vm_exp_buf(), vm_plane_buf(), VF_PLANE_BYTES);
    q = *p;
    q.video_delay_mob_in_exposed = 1u; /* the one-field flip */
    VC_CHECK(vm_render(&q, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
    VC_CHECK(vm_planes_differ(vm_plane_buf(), vm_exp_buf()));
    VC_CHECK(vm_count(4u) == 64u);
    if (part == 0u) {
      for (x = 12u; x <= 19u; x++) {
        VC_CHECK(vd_at(x, 0u) == 4u && vd_at(x, 2u) == 4u && vd_at(x, 3u) == 4u && vd_at(x, 7u) == 4u && vd_at(x, 8u) == 1u);
      }
      VC_CHECK(vd_at(11u, 0u) == VD_BLANK && vd_at(20u, 1u) == VD_BLANK); /* the rest of the exposed rows stays the not-displayed value */
    } else {
      for (y = 32u; y <= 39u; y++) {
        VC_CHECK(vd_at(0u, y) == 4u && vd_at(2u, y) == 4u && vd_at(3u, y) == 4u && vd_at(7u, y) == 4u && vd_at(8u, y) == 1u);
      }
      VC_CHECK(vd_at(0u, 31u) == VD_BLANK && vd_at(1u, 40u) == VD_BLANK);
    }
  }
  return IVS_TEST_PASS;
}

/* ---- the judge: a comparison that cannot fail proves nothing ---- */

/* video.judge.delay-frame-corrupt: a copy of the literal F-M5 with one expected position changed makes the comparison report a mismatch at exactly that
 * position (and the hash comparison fail), and the uncorrupted copy passes: the comparison of video.hdelay.shift-right-cards-and-mobs can fail. The
 * positions are a MOB pixel moved by the delay, a strip position, a card pixel moved by the delay and the last published position. */
static int vd_judge_frame_corrupt(void)
{
  const vm_frame *f = vm_frame_get(5u);
  const ivs_stic_params *p;
  static const uint32_t at[4][2] = {{63u, 40u}, {0u, 0u}, {83u, 48u}, {158u, 95u}};
  uint8_t *exp = vm_exp_buf();
  char hex[65];
  uint32_t k;
  uint32_t skipped = 0u;
  VC_CHECK(f != NULL && vm_fixture_ok(f));
  p = vd_scene_fm(5u);
  VC_CHECK(p != NULL && vm_render(p, 1u, vm_plane_buf()) == VF_PLANE_BYTES);
  vm_expected(f, exp);
  VC_CHECK(vm_first_diff(vm_plane_buf(), exp, &skipped) == -1); /* the control passes */
  for (k = 0u; k < 4u; k++) {
    uint32_t i = at[k][1] * VF_PLANE_W + at[k][0];
    uint8_t saved = exp[i];
    exp[i] = (uint8_t)(saved ^ 1u);
    VC_CHECK(vm_first_diff(vm_plane_buf(), exp, &skipped) == (long)(at[k][1] * (VF_PLANE_W - 1u) + at[k][0])); /* reported at the corrupted position */
    vf_text_sha(exp, VF_PLANE_W, VF_PLANE_W - 1u, hex);
    VC_CHECK(strcmp(hex, f->sha_text) != 0); /* and the hash of the corrupted copy is not the literal's */
    exp[i] = saved;
  }
  exp[5u * VF_PLANE_W + 5u] = 0xFFu; /* a wildcard position is skipped, never compared */
  VC_CHECK(vm_first_diff(vm_plane_buf(), exp, &skipped) == -1 && skipped == 1u);
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_video_delay_cases[] = {
    {"video.hdelay.shift-right-cards-and-mobs", vd_hdelay_shift, NULL},
    {"video.hdelay.clip-right", vd_hdelay_clip_right, NULL},
    {"video.vdelay.shift-down-cards-and-mobs", vd_vdelay_shift, NULL},
    {"video.vdelay.straddle-card-rows", vd_vdelay_straddle, NULL},
    {"video.vdelay.no-bus-window-change", vd_no_bus_window_change, NULL},
    {"video.delay.stream-equals-plane", vd_stream_equals_plane, NULL},
    {"video.delay.slice-invariant", vd_slice_invariant, NULL},
    {"video.hdelay.default-pinned.range", vd_pin_range, NULL},
    {"video.hdelay.default-pinned.exposed-area", vd_pin_hdelay_exposed, NULL},
    {"video.vdelay.default-pinned.exposed-area", vd_pin_vdelay_exposed, NULL},
    {"video.vdelay.default-pinned.mob-in-exposed", vd_pin_mob_in_exposed, NULL},
    {"video.judge.delay-frame-corrupt", vd_judge_frame_corrupt, NULL},
};
const size_t ivs_video_delay_count = sizeof ivs_video_delay_cases / sizeof ivs_video_delay_cases[0];
