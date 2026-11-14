/* SPDX-License-Identifier: MIT */
/* Suite video, family collision (plan 08-05, VID-05, D-02, D-06, D-07, D-08, D-10): the collision registers $0018 to $001F held to the frozen
 * oracle of plan 08-03. The expected values are the words of the truth table of docs/video/diag-stic-2-derivation.md section (d) (rows T1 to
 * T16), typed here by hand from that section and from the scene table (A7) and the per-event table of section (c); no word is read from a core
 * table or from a run. The scenes are the three collision scenes of the program of the oracle (scene 7, 8 and 9, selector BD): eight MOBs of
 * solid cards, three graphics RAM cards (derivation A2: solid, staircase, 5 x 5 block) and the BACKTAB words of A3, programmed on a fresh memory
 * system and delivered through ivs_stic_frame_deliver, the function that ORs the collision of a frame into the register file at the frame end.
 *
 * A pinned case (X coordinate 0, the border bit, positions outside the plane, a frame with the display off, a read that clears, a nonzero CPU
 * write) appears in the oracle as a `pinned: not asserted` row (P1 to P6) and nothing here asserts it except the default-pinned ids, which assert
 * the default and then require a one-field flip of a stack copy of ivs_stic_params to change the result. Guests mask collision reads with 03FF (D-08);
 * the reads of this file do the same. */
#include <stdint.h>
#include <string.h>

#include "video_cases.h"

#define VK_F 59736u
#define VK_COLL 0x18u /* the first collision register of the register file */
#define VK_MOBS 8u
#define VK_SLOT_MAX 16384u

static ivs_memsys vk_mem;
static ivs_stic_state vk_state;
static ivs_stic_video_row vk_row;

/* The three program cards of derivation A2 (byte y is pixel row y from the top, bit 7 the leftmost pixel). */
static const uint8_t vk_card_solid[8] = {0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu};
static const uint8_t vk_card_stairs[8] = {0x80u, 0xC0u, 0xE0u, 0xF0u, 0xF8u, 0xFCu, 0xFEu, 0xFFu};
static const uint8_t vk_card_block[8] = {0xF8u, 0xF8u, 0xF8u, 0xF8u, 0xF8u, 0x00u, 0x00u, 0x00u};

/* The three words of a MOB (X, Y and attribute) of a scene of derivation A7. */
typedef struct vk_mobw {
  uint16_t x;
  uint16_t y;
  uint16_t a;
} vk_mobw;

/* Derivation A7, selector BD, scene 7 (frame 1). Section (b) "Collision geometry" gives the rectangles these words place. */
static const vk_mobw vk_scene7[VK_MOBS] = {{0x0726u, 0x011Cu, 0x0804u}, {0x072Eu, 0x0120u, 0x0805u}, {0x0558u, 0x011Cu, 0x0806u},
                                           {0x0760u, 0x0120u, 0x0807u}, {0x0780u, 0x011Cu, 0x1801u}, {0x0688u, 0x0120u, 0x1802u},
                                           {0x0750u, 0x0134u, 0x1803u}, {0x0720u, 0x0144u, 0x1804u}};

/* A fresh scene: the three graphics RAM cards at the card numbers 0, 1 and 2, Color Stack mode with the stack 1 2 3 4 and the border 8, every
 * MOB register and every collision register zero, and the BACKTAB of derivation A3 in the row latches: the fill word 0000 (graphics ROM card 0,
 * which is all zero in this memory system, so every cell is background), the word 0812 at cell (6, 10) (the 5 x 5 block), and, when the
 * Colored Squares cells are asked for, 16DB at (6, 13) (color 3), 1000 at (6, 16) (color 0) and 37FF at (9, 10) (color 7). The parameters are returned. */
static const ivs_stic_params *vk_scene(int with_squares)
{
  uint32_t i;
  uint32_t k;
  (void)ivs_memsys_init(&vk_mem, SC_PROFILE);
  vf_poke_gram_card(&vk_mem, 0u, vk_card_solid);
  vf_poke_gram_card(&vk_mem, 1u, vk_card_stairs);
  vf_poke_gram_card(&vk_mem, 2u, vk_card_block);
  memset(&vk_state, 0, sizeof vk_state);
  vk_state.mode = 1u; /* Color Stack */
  vk_state.windows_active = 1u;
  for (i = 0u; i < 12u * 20u; i++) {
    vk_state.row_latch[i / 20u][i % 20u] = 0x0000u;
  }
  vk_state.row_latch[6][10] = 0x0812u;
  if (with_squares != 0) {
    vk_state.row_latch[6][13] = 0x16DBu;
    vk_state.row_latch[6][16] = 0x1000u;
    vk_state.row_latch[9][10] = 0x37FFu;
  }
  for (k = 0u; k < 4u; k++) {
    vk_mem.stic_reg[0x28u + k] = (uint16_t)(1u + k);
  }
  vk_mem.stic_reg[0x2Cu] = 8u;
  ivs_video_chain_init(&vk_row);
  return ivs_stic_params_get(SC_PROFILE);
}

/* Programs MOB i with the three words of w. */
static void vk_put(uint32_t i, const vk_mobw *w)
{
  vk_mem.stic_reg[0x00u + i] = w->x;
  vk_mem.stic_reg[0x08u + i] = w->y;
  vk_mem.stic_reg[0x10u + i] = w->a;
}

/* Completes one frame: the state is displayed (or not) and the frame is delivered with no sink at all, which ORs the collision of the frame into
 * the register file. */
static void vk_frame(const ivs_stic_params *p, uint32_t displayed)
{
  vk_state.windows_active = displayed;
  ivs_stic_frame_deliver(p, &vk_state, &vk_mem, &vk_row, NULL, (uint64_t)VK_F * (vk_row.count + 1u));
}

/* Register k as a guest masks it (D-08): the value of the register file and 03FF. */
static uint32_t vk_reg(uint32_t k)
{
  return (uint32_t)vk_mem.stic_reg[VK_COLL + k] & 0x03FFu;
}

/* ---- video.collision.mob-mob-bits: two overlapping MOBs of solid cards set the bit of the other MOB and never their own (R44, rows T1 and T2) ---- */

static int vk_mob_mob_bits(void)
{
  const ivs_stic_params *p;
  vk_mobw apart;
  uint32_t k;
  /* Scene 7, MOB0 0726 011C 0804 and MOB1 072E 0120 0805: two solid 16 x 8 rectangles at x 30..45, y 20..27 and x 38..53, y 24..31, both with the
   * interaction bit (X word bit 8): they share x 38..45, y 24..27. Derivation (d): bit 1 of C0 (word 0002, T1) and bit 0 of C1 (word 0001, T2);
   * the own bits stay clear and no other register changes. */
  p = vk_scene(0);
  VC_CHECK(p != NULL);
  vk_put(0u, &vk_scene7[0]);
  vk_put(1u, &vk_scene7[1]);
  vk_frame(p, 1u);
  VC_CHECK(vk_reg(0u) == 0x0002u && vk_reg(1u) == 0x0001u);
  VC_CHECK((vk_reg(0u) & 0x0001u) == 0u && (vk_reg(1u) & 0x0002u) == 0u); /* the own bit of MOB 0 and of MOB 1 */
  for (k = 2u; k < VK_MOBS; k++) {
    VC_CHECK(vk_reg(k) == 0u);
  }
  /* The control that the registers move with the geometry (the mutant STIC2-COLLIDE-APART of derivation (g)): MOB1 with the X word 0780 is at
   * x 120..135, 66 positions right of MOB0's last column, so nothing overlaps and no bit is set. */
  p = vk_scene(0);
  apart = vk_scene7[1];
  apart.x = 0x0780u;
  vk_put(0u, &vk_scene7[0]);
  vk_put(1u, &apart);
  vk_frame(p, 1u);
  for (k = 0u; k < VK_MOBS; k++) {
    VC_CHECK(vk_reg(k) == 0u);
  }
  return IVS_TEST_PASS;
}

/* ---- video.collision.or-at-frame-end: the bits reach the register file at the frame end, never inside the frame they describe (STIC-R34, C104) ---- */

/* The guest, from derivation A6 and A9 in miniature: frame 0 writes the solid card, the registers of MOB0 and MOB1 of scene 7, strobes the display
 * and reads C0 and C1 into $0300 and $0301 (inside the frame the overlap belongs to); a counted delay of 900 iterations (15 cycles each; a unit trace
 * of the same guest with 1,000 iterations put the strobe of frame 1 at cycle 1,941 of the frame, so 900 puts it near cycle 440, far inside the register
 * window that closes at cycle 2,000) carries it to the register window of frame 1, where it strobes again, reads C0 and C1 into $0302 and $0303, writes 0
 * to the X word of MOB1 (no overlap in frame 1), writes 0 to C0 (a CPU write clears) and reads C0 again into $0304, then idles. */
static size_t vk_guest(uint16_t *w)
{
  size_t n = 0u;
  uint32_t y;
  for (y = 0u; y < 8u; y++) {
    n = sc_w_mvii(w, n, 0u, 0xFFu);
    n = sc_w_mvo(w, n, 0u, 0x3800u + y);
  }
  n = sc_w_mvii16(w, n, 0u, 0x0726u);
  n = sc_w_mvo(w, n, 0u, 0x0000u);
  n = sc_w_mvii(w, n, 0u, 0x011Cu);
  n = sc_w_mvo(w, n, 0u, 0x0008u);
  n = sc_w_mvii16(w, n, 0u, 0x0804u);
  n = sc_w_mvo(w, n, 0u, 0x0010u);
  n = sc_w_mvii16(w, n, 0u, 0x072Eu);
  n = sc_w_mvo(w, n, 0u, 0x0001u);
  n = sc_w_mvii(w, n, 0u, 0x0120u);
  n = sc_w_mvo(w, n, 0u, 0x0009u);
  n = sc_w_mvii16(w, n, 0u, 0x0805u);
  n = sc_w_mvo(w, n, 0u, 0x0011u);
  n = sc_w_mvo(w, n, 0u, 0x0020u); /* the display strobe of frame 0 (the value is ignored) */
  n = sc_w_mvi(w, n, 0x0018u, 1u);
  n = sc_w_mvo(w, n, 1u, 0x0300u);
  n = sc_w_mvi(w, n, 0x0019u, 1u);
  n = sc_w_mvo(w, n, 1u, 0x0301u);
  n = sc_w_delay(w, n, 900u);
  n = sc_w_mvo(w, n, 0u, 0x0020u); /* the display strobe of frame 1 */
  n = sc_w_mvi(w, n, 0x0018u, 1u);
  n = sc_w_mvo(w, n, 1u, 0x0302u);
  n = sc_w_mvi(w, n, 0x0019u, 1u);
  n = sc_w_mvo(w, n, 1u, 0x0303u);
  n = sc_w_mvii(w, n, 0u, 0x0000u);
  n = sc_w_mvo(w, n, 0u, 0x0001u); /* MOB1 disabled: frame 1 shows no overlap */
  n = sc_w_mvo(w, n, 0u, 0x0018u); /* the CPU writes 0 into C0 */
  n = sc_w_mvi(w, n, 0x0018u, 1u);
  n = sc_w_mvo(w, n, 1u, 0x0304u);
  return sc_w_loop(w, n);
}

static const struct ivs_machine *vk_im(const ivs_ms_inst *x)
{
  return ivs_machine_from_handle(x->m);
}

static int vk_or_at_frame_end(void)
{
  static uint16_t w[600];
  ivs_ms_inst x;
  size_t n = vk_guest(w);
  int ok = 0;
  const struct ivs_machine *im;
  VC_CHECK(n < sizeof w / sizeof w[0]);
  memset(&x, 0, sizeof x);
  if (!sc_guest_n(&x, w, n)) {
    return IVS_TEST_FAIL;
  }
  im = vk_im(&x);
  /* Inside frame 0 (tick 30,000, the guest is in its delay): the two reads of the register window returned the cleared registers (FC00 OR 0), and the
   * register file holds no bit yet: the frame the overlap belongs to has not ended. */
  if (!sc_run_until(&x, 30000u)) {
    goto done;
  }
  if ((im->mem.sysram[0x100] & 0x03FFu) != 0u || (im->mem.sysram[0x101] & 0x03FFu) != 0u || im->mem.stic_reg[VK_COLL] != 0u ||
      im->mem.stic_reg[VK_COLL + 1u] != 0u) {
    ivs_test_diag("a collision bit is visible inside the frame it describes: reads %04X %04X, registers %04X %04X", (unsigned)im->mem.sysram[0x100],
                  (unsigned)im->mem.sysram[0x101], (unsigned)im->mem.stic_reg[VK_COLL], (unsigned)im->mem.stic_reg[VK_COLL + 1u]);
    goto done;
  }
  if (im->mem.sysram[0x100] != 0xFC00u || im->mem.sysram[0x101] != 0xFC00u) { /* non-vacuity: the reads happened and returned the cleared register (fill FC00) */
    ivs_test_diag("the in-frame reads did not run or were blocked: %04X %04X", (unsigned)im->mem.sysram[0x100], (unsigned)im->mem.sysram[0x101]);
    goto done;
  }
  /* Just after the end of frame 0 (tick 59,736 + 400, before the guest's next window read): the register file now holds the bits of frame 0. */
  if (!sc_run_until(&x, (uint64_t)VK_F + 400u)) {
    goto done;
  }
  if ((im->mem.stic_reg[VK_COLL] & 0x03FFu) != 0x0002u || (im->mem.stic_reg[VK_COLL + 1u] & 0x03FFu) != 0x0001u) {
    ivs_test_diag("the frame end did not put the collision into the register file: %04X %04X", (unsigned)im->mem.stic_reg[VK_COLL],
                  (unsigned)im->mem.stic_reg[VK_COLL + 1u]);
    goto done;
  }
  /* After the routine of frame 1 and its end (tick 2 x 59,736 + 400): the reads in the window of frame 1 saw the bits (FC00 OR 0002, FC00 OR 0001), the
   * read after the CPU write of 0 saw zero, and the register file holds no overlap of frame 1: C0 is clear (the write cleared it and frame 1 had no
   * overlap), C1 is still set (sticky until the CPU clears it). */
  if (!sc_run_until(&x, 2u * (uint64_t)VK_F + 400u)) {
    goto done;
  }
  if (im->mem.sysram[0x102] != 0xFC02u || im->mem.sysram[0x103] != 0xFC01u || im->mem.sysram[0x104] != 0xFC00u) {
    ivs_test_diag("the window reads of frame 1 are %04X %04X %04X, expected FC02 FC01 FC00", (unsigned)im->mem.sysram[0x102],
                  (unsigned)im->mem.sysram[0x103], (unsigned)im->mem.sysram[0x104]);
    goto done;
  }
  if ((im->mem.stic_reg[VK_COLL] & 0x03FFu) != 0u || (im->mem.stic_reg[VK_COLL + 1u] & 0x03FFu) != 0x0001u) {
    ivs_test_diag("after frame 1 the registers are %04X %04X, expected 0000 0001", (unsigned)im->mem.stic_reg[VK_COLL],
                  (unsigned)im->mem.stic_reg[VK_COLL + 1u]);
    goto done;
  }
  ok = 1;
done:
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- video.collision.buffer-independent: the register values and the chain do not depend on a video buffer (D-06, T-08-16) ---- */

/* The whole-instance storage of the guest run to `ticks` with the video channel in one of three forms: 0 the counting sink of ivs_ms_step, 1 an explicit
 * counting sink (no buffer), 2 a poisoned video buffer that takes every slot. The storage is copied into out (IVS_MACHINE_BYTES); 1 on success. */
static int vk_machine_run(const uint16_t *w, size_t n, uint32_t form, uint64_t ticks, uint8_t *out)
{
  static uint8_t buf[VK_SLOT_MAX * 4u];
  ivs_ms_inst x;
  ivs_psg_sink sink;
  uint32_t units = 0u;
  int ok = 0;
  memset(&x, 0, sizeof x);
  if (!sc_guest_n(&x, w, n)) {
    return 0;
  }
  memset(buf, (int)VP_POISON, sizeof buf);
  ivs_psg_sink_init(&sink, NULL, 0u);
  if (form == 2u) {
    ivs_psg_sink_video(&sink, buf, sizeof buf);
  }
  while (vk_im(&x)->time_ticks < ticks) {
    ivs_cp1610_result res;
    ivs_status st = form == 0u ? (ivs_ms_step(x.m, 1u, NULL) != 0 ? IVS_OK : IVS_E_BAD_STATE)
                               : ivs_machine_step_unit_sink_internal(x.m, &res, &sink);
    if (units++ >= 200000u || st != IVS_OK) {
      goto done;
    }
  }
  memcpy(out, x.storage, IVS_MACHINE_BYTES);
  ok = 1;
done:
  ivs_ms_free(&x);
  return ok;
}

static int vk_buffer_independent(void)
{
  static uint16_t w[600];
  static uint8_t slot[VK_SLOT_MAX];
  static uint8_t img[3][IVS_MACHINE_BYTES];
  static ivs_memsys mem_b;
  static ivs_memsys mem_c;
  const ivs_stic_params *p;
  ivs_stic_state sb;
  ivs_stic_state sc;
  ivs_stic_video_row rb;
  ivs_stic_video_row rc;
  ivs_psg_sink ka;
  ivs_psg_sink kc;
  size_t n = vk_guest(w);
  uint32_t k;
  /* The frame writer: the scene 7 overlap delivered with a video buffer, with the counting sink and with no sink at all. The register values, the chain, the
   * count and the stic state are the same in the three, and the registers hold the bits of the overlap (non-vacuity). */
  p = vk_scene(0);
  VC_CHECK(p != NULL && p->video_slot_bytes <= VK_SLOT_MAX);
  vk_put(0u, &vk_scene7[0]);
  vk_put(1u, &vk_scene7[1]);
  memcpy(&mem_b, &vk_mem, sizeof mem_b);
  memcpy(&mem_c, &vk_mem, sizeof mem_c);
  sb = vk_state;
  sc = vk_state;
  rb = vk_row;
  rc = vk_row;
  memset(slot, (int)VP_POISON, sizeof slot);
  ivs_psg_sink_init(&ka, NULL, 0u);
  ivs_psg_sink_video(&ka, slot, sizeof slot);
  ivs_stic_frame_deliver(p, &vk_state, &vk_mem, &vk_row, &ka, VK_F); /* with a buffer */
  ivs_psg_sink_init(&kc, NULL, 0u);
  ivs_stic_frame_deliver(p, &sb, &mem_b, &rb, &kc, VK_F); /* with the counting sink */
  ivs_stic_frame_deliver(p, &sc, &mem_c, &rc, NULL, VK_F); /* with no sink */
  VC_CHECK(ka.video_frames == 1u && ka.video_written == p->video_slot_bytes && kc.video_frames == 1u && kc.video_written == 0u);
  VC_CHECK(vk_reg(0u) == 0x0002u && vk_reg(1u) == 0x0001u);
  for (k = 0u; k < VK_MOBS; k++) {
    VC_CHECK(vk_mem.stic_reg[VK_COLL + k] == mem_b.stic_reg[VK_COLL + k] && vk_mem.stic_reg[VK_COLL + k] == mem_c.stic_reg[VK_COLL + k]);
  }
  VC_CHECK(memcmp(vk_row.chain, rb.chain, 32u) == 0 && memcmp(vk_row.chain, rc.chain, 32u) == 0);
  VC_CHECK(vk_row.count == 1u && rb.count == 1u && rc.count == 1u);
  VC_CHECK(memcmp(&vk_state, &sb, sizeof sb) == 0 && memcmp(&vk_state, &sc, sizeof sc) == 0);
  VC_CHECK(memcmp(&vk_mem, &mem_b, sizeof vk_mem) == 0 && memcmp(&vk_mem, &mem_c, sizeof vk_mem) == 0); /* the whole memory system, not only the registers */
  /* The machine: the guest of the previous id run to the end of frame 1 with the three forms of the video channel leaves the same instance storage. */
  VC_CHECK(vk_machine_run(w, n, 0u, 2u * (uint64_t)VK_F + 400u, img[0]));
  VC_CHECK(vk_machine_run(w, n, 1u, 2u * (uint64_t)VK_F + 400u, img[1]));
  VC_CHECK(vk_machine_run(w, n, 2u, 2u * (uint64_t)VK_F + 400u, img[2]));
  VC_CHECK(memcmp(img[0], img[1], IVS_MACHINE_BYTES) == 0 && memcmp(img[0], img[2], IVS_MACHINE_BYTES) == 0);
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_video_collision_cases[] = {
    {"video.collision.mob-mob-bits", vk_mob_mob_bits, NULL},
    {"video.collision.or-at-frame-end", vk_or_at_frame_end, NULL},
    {"video.collision.buffer-independent", vk_buffer_independent, NULL},
};
const size_t ivs_video_collision_count = sizeof ivs_video_collision_cases / sizeof ivs_video_collision_cases[0];
