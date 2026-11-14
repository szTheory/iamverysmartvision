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

/* Derivation A7, selector BD: scene 7 (frame 1), scene 8 (frame 2) and scene 9 (frame 3 and later). Section (b) "Collision geometry" gives the
 * rectangles these words place. */
static const vk_mobw vk_scene7[VK_MOBS] = {{0x0726u, 0x011Cu, 0x0804u}, {0x072Eu, 0x0120u, 0x0805u}, {0x0558u, 0x011Cu, 0x0806u},
                                           {0x0760u, 0x0120u, 0x0807u}, {0x0780u, 0x011Cu, 0x1801u}, {0x0688u, 0x0120u, 0x1802u},
                                           {0x0750u, 0x0134u, 0x1803u}, {0x0720u, 0x0144u, 0x1804u}};
static const vk_mobw vk_scene8[VK_MOBS] = {{0x070Au, 0x015Cu, 0x0804u}, {0x071Cu, 0x015Cu, 0x0805u}, {0x072Eu, 0x015Cu, 0x0806u},
                                           {0x0740u, 0x015Cu, 0x0807u}, {0x0752u, 0x015Cu, 0x1801u}, {0x0764u, 0x015Cu, 0x1802u},
                                           {0x0776u, 0x015Cu, 0x1803u}, {0x0788u, 0x015Cu, 0x1804u}};
static const vk_mobw vk_scene9[VK_MOBS] = {{0x0312u, 0x015Cu, 0x0804u}, {0x0370u, 0x0138u, 0x0805u}, {0x0388u, 0x0138u, 0x0806u},
                                           {0x0358u, 0x0150u, 0x0807u}, {0x0326u, 0x015Cu, 0x1801u}, {0x0258u, 0x0138u, 0x1802u},
                                           {0x023Au, 0x015Cu, 0x1803u}, {0x0380u, 0x015Cu, 0x1804u}};

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

static void vk_put_scene(const vk_mobw *scene)
{
  uint32_t i;
  for (i = 0u; i < VK_MOBS; i++) {
    vk_put(i, &scene[i]);
  }
}

/* Completes one frame: the state is displayed (or not) and the frame is delivered with no sink at all, which ORs the collision of the frame into
 * the register file. */
static void vk_frame(const ivs_stic_params *p, uint32_t displayed)
{
  vk_state.windows_active = displayed;
  ivs_stic_frame_deliver(p, &vk_state, &vk_mem, &vk_row, NULL, (uint64_t)VK_F * (vk_row.count + 1u));
}

/* The CPU's write of 0 into every collision register (the write view of the memory system, the gate is open in this unit scene). */
static void vk_cpu_clear(void)
{
  uint32_t k;
  for (k = 0u; k < VK_MOBS; k++) {
    ivs_memsys_write(&vk_mem, (uint16_t)(VK_COLL + k), 0x0000u);
  }
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
  /* The stand-in graphics ROM of the machine harness holds a seeded pattern, and the BACKTAB is all zero (graphics ROM card 0): the card layer would
   * have on pixels under the MOBs and set bit 8. This test is about the MOB against MOB bits and about time, so the graphics ROM is zeroed in the
   * instance (the raw backing, as the harness pokes graphics RAM): every card is then background and the registers are exactly the oracle's. */
  memset(ivs_machine_from_handle_mut(x.m)->mem.grom, 0, sizeof im->mem.grom);
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

/* ---- the truth table of derivation section (d): rows T1 to T16 ---- */

/* One row of the table: the register (C0 to C7) whose word the program stores, whether it is read after the CPU cleared the registers and scene 9 was
 * shown (rows T9 to T16, the words at $0308 to $030F) or after scenes 7 and 8 (rows T1 to T8, the words at $0300 to $0307), and the expected word,
 * already masked with 03FF. */
typedef struct vk_trow {
  const char *id;
  uint32_t reg;
  uint32_t after9;
  uint32_t expect;
} vk_trow;

/* Derivation (d), transcribed row by row (the Expected column). The six `pinned: not asserted` rows P1 to P6 of that section have no entry here: the table
 * of the test source has no row for a pinned case. */
static const vk_trow vk_table[16] = {
    {"T1", 0u, 0u, 0x0002u},  /* MOB0 over MOB1, both with the interaction bit; the own bit stays clear; sticky across scene 8 */
    {"T2", 1u, 0u, 0x0001u},  /* the same overlap seen from MOB1 */
    {"T3", 2u, 0u, 0x0008u},  /* MOB2 (interaction bit set, visible bit clear) over MOB3: an invisible MOB still interacts */
    {"T4", 3u, 0u, 0x0004u},  /* the same overlap from MOB3 */
    {"T5", 4u, 0u, 0x0000u},  /* MOB4 (interaction bit) over MOB5 (no interaction bit): both need it */
    {"T6", 5u, 0u, 0x0000u},  /* MOB5 over MOB4 */
    {"T7", 6u, 0u, 0x0100u},  /* MOB6 over the set pixels of card cell (6, 10): the background bit (bit 8) */
    {"T8", 7u, 0u, 0x0000u},  /* MOB7 over fill cells only */
    {"T9", 0u, 1u, 0x0000u},  /* scene 9: MOB0 alone over fill; the bits of scene 7 were cleared by the CPU */
    {"T10", 1u, 1u, 0x0100u}, /* MOB1 over the solid color 3 block: Colored Squares colors 0 to 6 are set pixels */
    {"T11", 2u, 1u, 0x0100u}, /* MOB2 over the solid color 0 block: color 0 is set */
    {"T12", 3u, 1u, 0x0000u}, /* MOB3 over the solid color 7 block: color 7 is not a set pixel */
    {"T13", 4u, 1u, 0x0000u}, /* MOB4 alone over fill */
    {"T14", 5u, 1u, 0x0000u}, /* MOB5 over the set pixels of cell (6, 10) with its interaction bit clear */
    {"T15", 6u, 1u, 0x0000u}, /* MOB6 alone over fill, interaction bit clear */
    {"T16", 7u, 1u, 0x0000u}, /* MOB7 alone over fill */
};

/* The number of rows of t whose word, read as the routines of frames 3 and 4 of the program read it, is not the expected one. Each row is run from a fresh
 * scene: the registers cleared by the CPU (frame 1), scene 7, scene 8, and for rows T9 to T16 the CPU's write of zero and scene 9 (frame 3). quiet
 * suppresses the diagnostics (the judge runs it on a corrupted copy). */
static uint32_t vk_table_diff(const vk_trow *t, size_t n, int quiet)
{
  uint32_t bad = 0u;
  size_t i;
  for (i = 0u; i < n; i++) {
    const ivs_stic_params *p = vk_scene(1);
    uint32_t got;
    if (p == NULL) {
      return 0xFFFFu;
    }
    vk_cpu_clear();
    vk_put_scene(vk_scene7);
    vk_frame(p, 1u);
    vk_put_scene(vk_scene8);
    vk_frame(p, 1u);
    if (t[i].after9 != 0u) {
      vk_cpu_clear();
      vk_put_scene(vk_scene9);
      vk_frame(p, 1u);
    }
    got = vk_reg(t[i].reg);
    if (got != t[i].expect) {
      if (!quiet) {
        ivs_test_diag("row %s (C%u after scene %u): got %04X, the derivation says %04X", t[i].id, (unsigned)t[i].reg, t[i].after9 != 0u ? 9u : 8u,
                      (unsigned)got, (unsigned)t[i].expect);
      }
      bad += 1u;
    }
  }
  return bad;
}

static int vk_truth_table(void)
{
  uint32_t set = 0u;
  uint32_t clear = 0u;
  size_t i;
  for (i = 0u; i < 16u; i++) {
    set += vk_table[i].expect != 0u ? 1u : 0u;
    clear += vk_table[i].expect == 0u ? 1u : 0u;
  }
  VC_CHECK(set == 7u && clear == 9u); /* derivation (d): seven rows show a set bit (T1 to T4, T7, T10, T11) and nine stay clear */
  VC_CHECK(vk_table_diff(vk_table, 16u, 0) == 0u);
  return IVS_TEST_PASS;
}

/* ---- one-MOB and two-MOB helpers of the rule tests ---- */

#define VK_A_SOLID 0x0804u /* graphics RAM card 0 (solid), color 4 */

/* MOB i of the solid card at the plane position (x0, y0), Y size code 01 (8 plane rows), double width when z2, interaction enable when intr, visible
 * when vis: the words are the register bits of data/video/mob.literal.txt (X word: bits 7..0 the X coordinate = x0 + 8, bit 8 interaction, bit 9 visible,
 * bit 10 double width; Y word: bits 6..0 the Y coordinate = y0 + 8, bits 9 and 8 the size code 01). */
static void vk_solid(uint32_t i, uint32_t x0, uint32_t y0, uint32_t z2, uint32_t intr, uint32_t vis)
{
  vk_mobw w;
  w.x = (uint16_t)((vis != 0u ? 0x0200u : 0u) | (intr != 0u ? 0x0100u : 0u) | (z2 != 0u ? 0x0400u : 0u) | (x0 + 8u));
  w.y = (uint16_t)(0x0100u | (y0 + 8u));
  w.a = VK_A_SOLID;
  vk_put(i, &w);
}

/* The register C0 after one displayed frame of a scene (the Colored Squares cells present) whose only MOB is MOB 0 at (x0, y0). */
static uint32_t vk_one(uint32_t x0, uint32_t y0, uint32_t z2, uint32_t intr, uint32_t vis)
{
  const ivs_stic_params *p = vk_scene(1);
  if (p == NULL) {
    return 0xFFFFu;
  }
  vk_solid(0u, x0, y0, z2, intr, vis);
  vk_frame(p, 1u);
  return vk_reg(0u);
}

/* ---- video.collision.mob-background-bit8: a set pixel of an interacting MOB on a set background pixel sets bit 8 (R44, rows T7, T10 to T12, T14) ---- */

static int vk_mob_background_bit8(void)
{
  typedef struct bgrow {
    uint32_t x0;
    uint32_t y0;
    uint32_t z2;
    uint32_t intr;
    uint32_t expect;
    const char *why;
  } bgrow;
  /* The scene: the 5 x 5 block of card 2 in cell (6, 10) is set at x 80..84, y 48..52 (card rows 0 to 4 and card columns 0 to 4, pattern F8); the other
   * pixels of that card are background. The Colored Squares cells are solid 8 x 8 blocks: color 3 at x 104..111, y 48..55, color 0 at x 128..135, y 48..55,
   * color 7 at x 80..87, y 72..79. The MOB is the solid 8 x 8 card (or 16 wide at double width). */
  static const bgrow rows[] = {
      {80u, 48u, 0u, 1u, 0x0100u, "over the 5 x 5 block of set pixels"},
      {84u, 52u, 0u, 1u, 0x0100u, "one MOB pixel on the last set pixel of the block (84, 52)"},
      {85u, 52u, 0u, 1u, 0x0000u, "one position right: (85, 52) is a background pixel of the same card"},
      {80u, 53u, 0u, 1u, 0x0000u, "below the block: rows 5 to 7 of the card are background"},
      {85u, 53u, 0u, 1u, 0x0000u, "over only the background pixels of the card cell"},
      {72u, 48u, 1u, 1u, 0x0100u, "at double width the MOB reaches the block from the left"},
      {104u, 48u, 0u, 1u, 0x0100u, "over the solid color 3 Colored Squares block (colors 0 to 6 are set pixels)"},
      {128u, 48u, 0u, 1u, 0x0100u, "over the solid color 0 block (color 0 is set)"},
      {80u, 72u, 0u, 1u, 0x0000u, "over the solid color 7 block (color 7 is not a set pixel)"},
      {20u, 20u, 0u, 1u, 0x0000u, "over fill cells only"},
      {80u, 48u, 0u, 0u, 0x0000u, "over the block but without the interaction bit"},
  };
  size_t i;
  for (i = 0u; i < sizeof rows / sizeof rows[0]; i++) {
    uint32_t got = vk_one(rows[i].x0, rows[i].y0, rows[i].z2, rows[i].intr, 1u);
    if (got != rows[i].expect) {
      ivs_test_diag("MOB at (%u,%u): %s: C0 is %04X, expected %04X", (unsigned)rows[i].x0, (unsigned)rows[i].y0, rows[i].why, (unsigned)got,
                    (unsigned)rows[i].expect);
      return IVS_TEST_FAIL;
    }
  }
  return IVS_TEST_PASS;
}

/* ---- video.collision.own-bit-never-set: the frame writer never sets the bit of a MOB in its own register (R44) ---- */

static int vk_own_bit_never_set(void)
{
  const ivs_stic_params *p;
  uint32_t n;
  /* Eight solid MOBs on the same 8 x 8 positions (80..87, 48..55), every one with the interaction bit, over the block of set pixels: each MOB overlaps the
   * seven others and the background, so every register is the seven other MOB bits and bit 8, and the one bit that would be its own is clear. */
  p = vk_scene(1);
  VC_CHECK(p != NULL);
  for (n = 0u; n < VK_MOBS; n++) {
    vk_solid(n, 80u, 48u, 0u, 1u, 1u);
  }
  vk_frame(p, 1u);
  for (n = 0u; n < VK_MOBS; n++) {
    uint32_t want = 0x01FFu & ~(1u << n);
    if (vk_reg(n) != want || (vk_reg(n) & (1u << n)) != 0u) {
      ivs_test_diag("C%u is %04X, expected %04X: the own bit is bit %u", (unsigned)n, (unsigned)vk_reg(n), (unsigned)want, (unsigned)n);
      return IVS_TEST_FAIL;
    }
  }
  /* A lone MOB never sets its own bit either: C0 holds the background bit only, and over fill cells it holds nothing. */
  p = vk_scene(1);
  vk_solid(0u, 80u, 48u, 0u, 1u, 1u);
  vk_frame(p, 1u);
  VC_CHECK(vk_reg(0u) == 0x0100u);
  p = vk_scene(1);
  vk_solid(0u, 20u, 20u, 0u, 1u, 1u);
  vk_frame(p, 1u);
  VC_CHECK(vk_reg(0u) == 0u);
  return IVS_TEST_PASS;
}

/* ---- video.collision.intr-required: both MOBs need the interaction bit for MOB against MOB, the MOB needs it for MOB against background (R44) ---- */

static int vk_intr_required(void)
{
  static const uint32_t flags[4][2] = {{1u, 1u}, {1u, 0u}, {0u, 1u}, {0u, 0u}};
  uint32_t t;
  for (t = 0u; t < 4u; t++) {
    const ivs_stic_params *p = vk_scene(1);
    uint32_t want = (flags[t][0] != 0u && flags[t][1] != 0u) ? 1u : 0u;
    VC_CHECK(p != NULL);
    /* the two solid rectangles of scene 7's MOB0 and MOB1 (x 30..45, y 20..27 and x 38..53, y 24..31), which share x 38..45, y 24..27 */
    vk_solid(0u, 30u, 20u, 1u, flags[t][0], 1u);
    vk_solid(1u, 38u, 24u, 1u, flags[t][1], 1u);
    vk_frame(p, 1u);
    if (vk_reg(0u) != (want != 0u ? 0x0002u : 0u) || vk_reg(1u) != (want != 0u ? 0x0001u : 0u)) {
      ivs_test_diag("interaction bits %u and %u: C0 %04X C1 %04X, expected %s", (unsigned)flags[t][0], (unsigned)flags[t][1], (unsigned)vk_reg(0u),
                    (unsigned)vk_reg(1u), want != 0u ? "0002 0001" : "0000 0000");
      return IVS_TEST_FAIL;
    }
  }
  /* MOB against background: the same MOB over the block of set pixels, with and without its interaction bit */
  VC_CHECK(vk_one(80u, 48u, 0u, 1u, 1u) == 0x0100u);
  VC_CHECK(vk_one(80u, 48u, 0u, 0u, 1u) == 0u);
  return IVS_TEST_PASS;
}

/* ---- video.collision.visibility-not-required: an invisible MOB with the interaction bit still collides, and still draws nothing (R44, rows T3 and T4) ---- */

static int vk_visibility_not_required(void)
{
  static uint8_t pic[VF_PLANE_BYTES];
  static uint8_t ref[VF_PLANE_BYTES];
  const ivs_stic_params *p;
  uint32_t vis0;
  uint32_t vis1;
  for (vis0 = 0u; vis0 < 2u; vis0++) {
    for (vis1 = 0u; vis1 < 2u; vis1++) {
      p = vk_scene(1);
      VC_CHECK(p != NULL);
      vk_solid(0u, 30u, 20u, 1u, 1u, vis0);
      vk_solid(1u, 38u, 24u, 1u, 1u, vis1);
      vk_frame(p, 1u);
      if (vk_reg(0u) != 0x0002u || vk_reg(1u) != 0x0001u) {
        ivs_test_diag("visible bits %u and %u: C0 %04X C1 %04X, expected 0002 0001", (unsigned)vis0, (unsigned)vis1, (unsigned)vk_reg(0u),
                      (unsigned)vk_reg(1u));
        return IVS_TEST_FAIL;
      }
    }
  }
  /* an invisible MOB over the set pixels of the background */
  VC_CHECK(vk_one(80u, 48u, 0u, 1u, 0u) == 0x0100u);
  /* and it draws nothing: with only an invisible MOB the plane is the plane of no MOB at all */
  p = vk_scene(1);
  VC_CHECK(p != NULL && ivs_stic_render_plane(p, &vk_state, &vk_mem, 1u, ref) == VF_PLANE_BYTES);
  vk_solid(0u, 80u, 48u, 0u, 1u, 0u);
  VC_CHECK(ivs_stic_render_plane(p, &vk_state, &vk_mem, 1u, pic) == VF_PLANE_BYTES);
  VC_CHECK(memcmp(pic, ref, VF_PLANE_BYTES) == 0);
  return IVS_TEST_PASS;
}

/* ---- video.collision.sticky-across-frames: the bits survive frames in which nothing overlaps, even a cleared interaction bit, accumulate across frames,
 * and go only when the CPU writes (R44, rows T1 to T8 read after scene 8) ---- */

static int vk_sticky_across_frames(void)
{
  const ivs_stic_params *p = vk_scene(0);
  uint32_t k;
  VC_CHECK(p != NULL);
  /* frame A: MOB0 and MOB1 overlap */
  vk_solid(0u, 30u, 20u, 1u, 1u, 1u);
  vk_solid(1u, 38u, 24u, 1u, 1u, 1u);
  vk_frame(p, 1u);
  VC_CHECK(vk_reg(0u) == 0x0002u && vk_reg(1u) == 0x0001u);
  /* frame B: MOB1 moved far away (x 120..135): no overlap in the frame, the bits stay */
  vk_solid(1u, 120u, 24u, 1u, 1u, 1u);
  vk_frame(p, 1u);
  if (vk_reg(0u) != 0x0002u || vk_reg(1u) != 0x0001u) {
    ivs_test_diag("after a frame with no overlap C0 is %04X and C1 %04X, expected 0002 and 0001", (unsigned)vk_reg(0u), (unsigned)vk_reg(1u));
    return IVS_TEST_FAIL;
  }
  /* frame C: the interaction bits cleared as well: the bits still stay */
  vk_solid(0u, 30u, 20u, 1u, 0u, 1u);
  vk_solid(1u, 120u, 24u, 1u, 0u, 1u);
  vk_frame(p, 1u);
  VC_CHECK(vk_reg(0u) == 0x0002u && vk_reg(1u) == 0x0001u);
  /* frame D: MOB0 overlaps a third MOB (x 34..49, y 22..29) in a new frame: the new bit is ORed with the old one (an assignment would lose bit 1) */
  vk_solid(0u, 30u, 20u, 1u, 1u, 1u);
  vk_solid(2u, 34u, 22u, 1u, 1u, 1u);
  vk_frame(p, 1u);
  if (vk_reg(0u) != 0x0006u || vk_reg(2u) != 0x0001u || vk_reg(1u) != 0x0001u) {
    ivs_test_diag("after a second overlap C0 is %04X, C1 %04X, C2 %04X, expected 0006 0001 0001", (unsigned)vk_reg(0u), (unsigned)vk_reg(1u),
                  (unsigned)vk_reg(2u));
    return IVS_TEST_FAIL;
  }
  /* the CPU's write of 0 clears them, and a frame with no overlap leaves them clear */
  vk_cpu_clear();
  vk_solid(2u, 120u, 60u, 1u, 1u, 1u);
  vk_frame(p, 1u);
  for (k = 0u; k < VK_MOBS; k++) {
    VC_CHECK(vk_reg(k) == 0u);
  }
  return IVS_TEST_PASS;
}

/* ---- video.collision.cpu-write-replaces: a CPU write replaces the register under the 10-bit mask, a write of 0 clears, the frame end ORs onto what is there (D-06, PD-7) ---- */

static int vk_cpu_write_replaces(void)
{
  const ivs_stic_params *p = vk_scene(0);
  VC_CHECK(p != NULL);
  vk_solid(0u, 30u, 20u, 1u, 1u, 1u);
  vk_solid(1u, 38u, 24u, 1u, 1u, 1u);
  vk_frame(p, 1u);
  VC_CHECK(vk_reg(0u) == 0x0002u);
  /* a nonzero write replaces the register: F1A0 under the mask 03FF is 01A0, with nothing left of the old 0002 (an OR would give 01A2); the bits 0 and 1
   * of F1A0 are clear, so the own-bit rule of the lineage does not enter (it stays outside this plan, see the SUMMARY) */
  ivs_memsys_write(&vk_mem, (uint16_t)VK_COLL, 0xF1A0u);
  VC_CHECK(vk_reg(0u) == 0x01A0u);
  VC_CHECK(ivs_memsys_read(&vk_mem, (uint16_t)VK_COLL) == 0xFDA0u); /* the read gives FC00 or the stored bits */
  /* the frame end ORs onto the written value */
  vk_frame(p, 1u);
  VC_CHECK(vk_reg(0u) == 0x01A2u);
  /* a write of 0 clears; a write of FFFC keeps the 10 bits under the mask */
  ivs_memsys_write(&vk_mem, (uint16_t)VK_COLL, 0x0000u);
  VC_CHECK(vk_reg(0u) == 0u && ivs_memsys_read(&vk_mem, (uint16_t)VK_COLL) == 0xFC00u);
  ivs_memsys_write(&vk_mem, (uint16_t)VK_COLL, 0xFFFCu);
  VC_CHECK(vk_reg(0u) == 0x03FCu);
  return IVS_TEST_PASS;
}

/* ---- video.collision.read-no-effect: a CPU read of a collision register changes nothing, and the test can fail (D-06, PD-7) ---- */

/* One CPU read of addr through the machine bus under the parameters q at the machine tick 400 (inside the register window); the word read. */
static uint32_t vk_bus_read(struct ivs_machine *im, const ivs_stic_params *q, uint16_t addr)
{
  ivs_machine_bus_ctx c;
  ivs_cp1610_access a;
  ivs_bus_reply r;
  memset(&c, 0, sizeof c);
  c.m = im;
  c.unit_start_tick = 400u;
  c.ticks_per_cpu_cycle = q->ticks_per_cpu_cycle;
  c.psg = ivs_psg_params_get(im->profile);
  c.stic = q;
  c.sink = NULL;
  memset(&a, 0, sizeof a);
  a.kind = IVS_ACC_READ;
  a.addr = addr;
  r = ivs_machine_bus(&c, &a);
  return (uint32_t)r.data;
}

static int vk_read_no_effect(void)
{
  static ivs_memsys mem0;
  static uint16_t w[8];
  ivs_ms_inst x;
  struct ivs_machine *im;
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  ivs_stic_params q;
  ivs_stic_state st0;
  uint32_t k;
  uint32_t rep;
  uint16_t v;
  int ok = 0;
  size_t n = sc_prog_loop(w, 0u);
  memset(&x, 0, sizeof x);
  VC_CHECK(p != NULL && sc_guest(&x, w, n, 0u));
  im = ivs_machine_from_handle_mut(x.m);
  (void)ivs_machine_reset(x.m, IVS_RESET_POWER_ON);
  for (k = 0u; k < VK_MOBS; k++) {
    im->mem.stic_reg[VK_COLL + k] = (uint16_t)(0x0301u + 0x10u * k); /* eight distinct nonzero words, bit 8 and bit 9 set */
  }
  (void)vk_bus_read(im, p, 0x0018u); /* the first access catches the STIC up to the stamp: the events up to it are applied here */
  st0 = im->stic;
  memcpy(&mem0, &im->mem, sizeof mem0);
  if (st0.mode != 0u) {
    ivs_test_diag("the power-on mode is not Foreground/Background: %u", (unsigned)st0.mode);
    goto done;
  }
  /* sixteen reads of the eight registers and of their read aliases at $0058, each twice: every read gives FC00 or the stored bits, the two reads of a
   * register agree, and no register, no state of the STIC and nothing else of the memory system changes */
  for (k = 0u; k < VK_MOBS; k++) {
    uint32_t want = 0xFC00u | (uint32_t)(0x0301u + 0x10u * k);
    for (rep = 0u; rep < 2u; rep++) {
      if (vk_bus_read(im, p, (uint16_t)(VK_COLL + k)) != want || vk_bus_read(im, p, (uint16_t)(0x0058u - 0x18u + VK_COLL + k)) != want) {
        ivs_test_diag("read %u of C%u gave another word than %04X", (unsigned)rep, (unsigned)k, (unsigned)want);
        goto done;
      }
    }
    if (ivs_machine_peek(x.m, (uint16_t)(VK_COLL + k), 1u, &v) != IVS_OK || (uint32_t)v != want) { /* the peek equals the CPU value */
      ivs_test_diag("the peek of C%u is %04X, the CPU read %04X", (unsigned)k, (unsigned)v, (unsigned)want);
      goto done;
    }
  }
  if (memcmp(&st0, &im->stic, sizeof st0) != 0 || memcmp(&mem0, &im->mem, sizeof mem0) != 0) {
    ivs_test_diag("a read of a collision register changed the state of the STIC or the memory system");
    goto done;
  }
  /* The flip that shows the check can fail with no new field: in a stack copy of the parameters the existing list stic.mode_select_addrs gets the address
   * $0018 and the existing rows stic.access_effects get a read row of $0018 that selects Color Stack (a read effect needs the address in the list and a
   * row for the register). The same read of the same register under that copy switches the mode, which the state comparison above would catch. */
  q = *p;
  VC_CHECK(q.mode_addr_count < 8u && q.effect_count < 8u);
  q.mode_addr[q.mode_addr_count++] = 0x0018u;
  q.effect_addr[q.effect_count] = 0x0018u;
  q.effect_access[q.effect_count] = IVS_STIC_ACCESS_READ;
  q.effect_code[q.effect_count] = IVS_STIC_EFFECT_MODE_CS;
  q.effect_count++;
  (void)vk_bus_read(im, &q, 0x0018u);
  if (im->stic.mode != 1u || memcmp(&st0, &im->stic, sizeof st0) == 0) {
    ivs_test_diag("with $0018 a read-sensitive address the read did not switch the mode (mode %u): the check could not see an effect", (unsigned)im->stic.mode);
    goto done;
  }
  ok = 1;
done:
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- video.collision.slice-invariant: the host's call sizes change no byte of a run that collides (T-08-16) ---- */

#define VK_TOTAL (3u * VK_F + 4000u)
#define VK_CAP ((uint64_t)15296u * 5u)

/* The stand-in guest: the solid card 0, the six registers of MOB0 and MOB1 of scene 7, a read of $0021 (Color Stack), then the display strobe in an endless
 * loop, so every frame from the first strobe on is displayed and the same overlap is seen in each. */
static size_t vk_slice_guest(uint16_t *w)
{
  size_t n = 0u;
  size_t start;
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
  n = sc_w_mvi(w, n, 0x0021u, 1u);
  start = n;
  n = sc_w_mvo(w, n, 0u, 0x0020u);
  w[n] = 0x0220u; /* B back to the strobe */
  w[n + 1u] = (uint16_t)(n + 1u - start);
  return n + 2u;
}

static ivs_status vk_call(ivs_ms_inst *x, uint64_t target, uint8_t *video, uint64_t cap, ivs_run_report *r)
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

/* Runs the guest to VK_TOTAL in calls of `slice` ticks (0: one call), appending every delivered slot to out; the instance stays in x. The delivered byte
 * count, or (uint64_t)-1 on a failure. The graphics ROM is zeroed in the instance first (see vk_or_at_frame_end). */
static uint64_t vk_run_sliced(ivs_ms_inst *x, const uint16_t *w, size_t n, uint64_t slice, uint8_t *out)
{
  static uint8_t tmp[15296u * 5u];
  uint64_t got = 0u;
  if (!sc_guest_n(x, w, n)) {
    return (uint64_t)-1;
  }
  memset(ivs_machine_from_handle_mut(x->m)->mem.grom, 0, sizeof ivs_machine_from_handle_mut(x->m)->mem.grom);
  while (vk_im(x)->time_ticks < VK_TOTAL) {
    ivs_run_report r;
    uint64_t now = vk_im(x)->time_ticks;
    uint64_t target = slice == 0u ? VK_TOTAL : (now + slice < VK_TOTAL ? now + slice : VK_TOTAL);
    if (vk_call(x, target, tmp, VK_CAP, &r) != IVS_OK || got + r.video_bytes_written > VK_CAP) {
      return (uint64_t)-1;
    }
    memcpy(out + got, tmp, (size_t)r.video_bytes_written);
    got += r.video_bytes_written;
  }
  return got;
}

static int vk_slice_one(uint64_t slice, uint8_t *ref, uint64_t ref_bytes, const ivs_ms_inst *a, const uint16_t *w, size_t n)
{
  static uint8_t out[15296u * 5u];
  ivs_ms_inst b;
  uint64_t got;
  int ok;
  memset(&b, 0, sizeof b);
  got = vk_run_sliced(&b, w, n, slice, out);
  ok = got == ref_bytes && memcmp(out, ref, (size_t)ref_bytes) == 0 && memcmp(a->storage, b.storage, IVS_MACHINE_BYTES) == 0 &&
       memcmp(vk_im(a)->obs.video.chain, vk_im(&b)->obs.video.chain, 32u) == 0;
  if (!ok) {
    ivs_test_diag("calls of %llu ticks differ from one call: %llu bytes against %llu", (unsigned long long)slice, (unsigned long long)got,
                  (unsigned long long)ref_bytes);
  }
  if (b.storage != NULL) {
    ivs_ms_free(&b);
  }
  return ok;
}

static int vk_slice_invariant(void)
{
  static uint16_t w[600];
  static uint8_t ref[15296u * 5u];
  static const uint64_t sizes[4] = {1u, 7u, 468u, 97u};
  ivs_ms_inst a;
  uint64_t ref_bytes;
  uint32_t k;
  int ok = 0;
  size_t n = vk_slice_guest(w);
  VC_CHECK(n < sizeof w / sizeof w[0]);
  memset(&a, 0, sizeof a);
  ref_bytes = vk_run_sliced(&a, w, n, 0u, ref); /* the reference: one call to the same end */
  if (ref_bytes != 3u * 15296u) {
    ivs_test_diag("the reference run delivered %llu bytes, expected %u", (unsigned long long)ref_bytes, (unsigned)(3u * 15296u));
    goto done;
  }
  /* non-vacuity: the guest left the overlap bits in the register file (C0 bit 1 and C1 bit 0, with the zero graphics ROM no background bit) */
  if ((vk_im(&a)->mem.stic_reg[VK_COLL] & 0x03FFu) != 0x0002u || (vk_im(&a)->mem.stic_reg[VK_COLL + 1u] & 0x03FFu) != 0x0001u) {
    ivs_test_diag("the colliding run left C0 %04X and C1 %04X, expected 0002 and 0001", (unsigned)vk_im(&a)->mem.stic_reg[VK_COLL],
                  (unsigned)vk_im(&a)->mem.stic_reg[VK_COLL + 1u]);
    goto done;
  }
  for (k = 0u; k < 4u; k++) {
    if (!vk_slice_one(sizes[k], ref, ref_bytes, &a, w, n)) {
      goto done;
    }
  }
  ok = 1;
done:
  ivs_ms_free(&a);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_case ivs_video_collision_cases[] = {
    {"video.collision.mob-mob-bits", vk_mob_mob_bits, NULL},
    {"video.collision.or-at-frame-end", vk_or_at_frame_end, NULL},
    {"video.collision.buffer-independent", vk_buffer_independent, NULL},
    {"video.collision.truth-table", vk_truth_table, NULL},
    {"video.collision.mob-background-bit8", vk_mob_background_bit8, NULL},
    {"video.collision.own-bit-never-set", vk_own_bit_never_set, NULL},
    {"video.collision.intr-required", vk_intr_required, NULL},
    {"video.collision.visibility-not-required", vk_visibility_not_required, NULL},
    {"video.collision.sticky-across-frames", vk_sticky_across_frames, NULL},
    {"video.collision.cpu-write-replaces", vk_cpu_write_replaces, NULL},
    {"video.collision.read-no-effect", vk_read_no_effect, NULL},
    {"video.collision.slice-invariant", vk_slice_invariant, NULL},
};
const size_t ivs_video_collision_count = sizeof ivs_video_collision_cases / sizeof ivs_video_collision_cases[0];
