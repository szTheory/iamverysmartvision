/* SPDX-License-Identifier: MIT */
/* Suite stic, family disc (VID-07, plan 19-01; D-54, GUARDRAILS-v0.4 2.6 and 2.7): the rival display-path rules, each as one
 * authored guest whose **one registered observable is machine state**, read at the machine-state seam
 * (`ivs_machine_stic_state`), never a pixel and never a hash of a frame (D-51).
 *
 * The rivals, and the discrepancy row that carries each:
 *   G1  the register-access window length, 2000 CPU cycles against 2900 (C1, field stic.reg_window_cpu_cycles).
 *       Observable: the machine's blocked-write counter for a write to $0020 stamped inside the 2000-to-2900 gap.
 *   G2  whether the access windows apply while the display latch is clear (C94, field stic.access_gating_when_off).
 *       Observable: the blocked-write counter for a write stamped past both window candidates with the latch clear.
 *   G3  whether a READ of $0020 strobes the display (C82, the $0020 READ row of stic.access_effects; C84 bounds it).
 *       Observable: the frame-0 displayed word after a read of $0020 with no write.
 *   G4  whether one strobe sustains the display for more than one frame (C82, the frame-start latch clear).
 *       Observable: the displayed word of frames 0, 1 and 2 after one write that is never re-strobed.
 *
 * Each guest is driven twice, and the two readings are what makes the case a *guard* rather than a fit: the same guest's
 * observable is read again with its own field at the alternate setting, on a STACK COPY of the generated row (the
 * `stic.profile.window-close` precedent: the shipped row is never edited, because the machine rebuilds its bus context
 * from the profile index on every unit and a global edit would make the matrix unreproducible). A case fails unless its
 * observable takes both values, so a guest whose observable cannot move is red rather than vacuously green.
 *
 * The guests are this project's own content: every word is built by the suite's decle builders (`sc_w_*`, stic_progs.c)
 * from the project's own Phase 2 opcode vectors, exactly as `stic_cases.h` derives them, and no byte here is
 * rights-bearing - no EXEC, GROM, cartridge or diagnostic image is read, and nothing third-party is named.
 *
 * The report's key table is closed and carries no STIC counter (`docs/runner.md` `## Report`), so the blocked-write
 * counter has no report line and is read from machine state only. The displayed word is the opposite case: the report's
 * `frame N displayed` clause prints the delivered slot header's IVS_FRAME_F_DISPLAYED bit, which is written from the same
 * `windows_active` word this family reads. `stic.disc.displayed-equivalence` pins that they are one word.
 *
 * The stamp of every access is MEASURED at run time, never derived: `sd_measure` steps the guest unit by unit through the
 * machine's per-unit access trace seam and composes each stamp the way the bus wrapper composes it
 * (`unit_start + offset_cpu_cycles x ticks_per_cpu_cycle + access_strobe_ticks`), which is also the composition the
 * frozen page states for `ivs-inspect trace`'s own `tick=` field (docs/inspection.md). A guest supplies the stamp; the
 * case re-drives that same stamp under a perturbed row. */

#include <stdint.h>
#include <string.h>

#include "ivs/ivs.h"
#include "machine/machine.h"
#include "stic_cases.h"

/* The two register-window candidates the criterion names. The shipped close is the profile's own
 * stic.reg_window_cpu_cycles (2000 CPU cycles, C1); the alternate is the figure its cite names ("SRC-intv-wiki gives
 * about 2900 (the alternate)"), so a case compares the measured stamp against p->reg_window_ticks and against this
 * converted alternate, never against a bare tick literal. */
#define SD_WINDOW_WIDE_CYCLES 2900u

/* G1's delay count. The gap write's stamp is measured at run time; this comment derives only the search start.
 * `sc_w_delay` is 15 x count + 6 cycles (stic_progs.c:65-72); the leading MVII #0, R0 is 8 and the MVO R0, $0020 that
 * carries the write is 11 with the access a couple of cycles into it, so count 149 puts the write near CPU cycle 2251,
 * i.e. tick 9004 at the profile's 4 ticks per CPU cycle - inside the 8000-to-11600 tick gap between the two candidates.
 * The case asserts the relation, so a profile whose time base moved fails rather than silently re-fitting this number. */
#define SD_G1_DELAY 149u

/* A stack copy of the shipped row with the register window at the alternate candidate: the REG_CLOSE event tick and the
 * two recorded forms of the same number move together, and nothing else in the row does. */
static int sd_row_wide(const ivs_stic_params *p, ivs_stic_params *q)
{
  uint32_t i;
  uint32_t idx = p->event_count;
  for (i = 0u; i < p->event_count; i++) {
    if (p->event_kind[i] == IVS_STIC_EV_REG_CLOSE) {
      idx = i;
      break;
    }
  }
  if (idx >= p->event_count) {
    return 0;
  }
  *q = *p;
  q->event_tick[idx] = (uint32_t)SD_WINDOW_WIDE_CYCLES * q->ticks_per_cpu_cycle;
  q->reg_window_cycles = SD_WINDOW_WIDE_CYCLES;
  q->reg_window_ticks = q->event_tick[idx];
  return 1;
}

/* ---- the guests (decle derivations in the style of stic_cases.h) ----
 *   MVII #imm, Rd   02B8 + d, then imm                                    8 cycles
 *   MVO  Rs, addr   0240 + s, then addr (MVO R0, $0020 is 0240 0020)     11 cycles, the write inside it
 *   MVI  addr, Rd   0280 + d, then addr (MVI $0020, R1 is 0281 0020)     10 cycles, the read inside it
 *   DECR R1 / BNZE back  0011 / 022C 0002       the delay loop, 15 x count + 6 cycles over the whole sc_w_delay
 *   B self          0220, 0001                                            9 cycles taken
 */

/* G1, the criterion's own guest: one strobe, at the gap, with the display still off. */
static size_t sd_prog_g1_criterion(uint16_t *w)
{
  size_t n = 0u;
  n = sc_w_mvii(w, n, 0u, 0x0000u); /* $1000 MVII #0, R0        8 cycles */
  n = sc_w_delay(w, n, SD_G1_DELAY); /* $1002 MVII #149,R1; DECR; BNZE   15 x 149 + 6 = 2241 cycles */
  n = sc_w_mvo(w, n, 0u, 0x0020u);   /* $1007 MVO R0, $0020      11 cycles, the gap write */
  return sc_w_loop(w, n);            /* $1009 B self             9 cycles taken */
}

/* G1, the witness: the same delay and the same gap write, with one strobe in front of it, so the display is already on
 * when the gap write lands. The gap write's only remaining trace is then the blocked-write counter, which is the
 * registered observable: with the latch already set the gating alternate's condition is false and the window schedule
 * alone decides the write. */
static size_t sd_prog_g1_witness(uint16_t *w)
{
  size_t n = 0u;
  n = sc_w_mvii(w, n, 0u, 0x0000u); /* $1000 MVII #0, R0        8 cycles */
  n = sc_w_mvo(w, n, 0u, 0x0020u);  /* $1002 MVO R0, $0020      11 cycles, the early strobe */
  n = sc_w_delay(w, n, SD_G1_DELAY); /* $1004 MVII #149,R1; DECR; BNZE   2241 cycles */
  n = sc_w_mvo(w, n, 0u, 0x0020u);   /* $1009 MVO R0, $0020      11 cycles, the gap write */
  return sc_w_loop(w, n);            /* $100B B self */
}

/* G1's committed stand-in: the witness shape with the delay removed, so the "gap" write lands inside the window under
 * both candidates, the blocked count is 0 in both limbs and the guard fails. It is committed code, re-run by
 * `stic.judge.disc-g1-dead` on every gate run, never built and deleted once. */
static size_t sd_prog_g1_dead(uint16_t *w)
{
  size_t n = 0u;
  n = sc_w_mvii(w, n, 0u, 0x0000u); /* $1000 MVII #0, R0        8 cycles */
  n = sc_w_mvo(w, n, 0u, 0x0020u);  /* $1002 MVO R0, $0020      11 cycles, inside the window */
  n = sc_w_mvo(w, n, 0u, 0x0020u);  /* $1004 MVO R0, $0020      11 cycles, inside the window too */
  return sc_w_loop(w, n);           /* $1006 B self */
}

/* ---- the measurement: the guest's own access stamps, composed the way the bus wrapper composes them ---- */

#define SD_HIT_MAX 8u

typedef struct sd_hits {
  size_t n;
  uint64_t stamp[SD_HIT_MAX];
  uint32_t cycle[SD_HIT_MAX];
} sd_hits;

/* Runs the guest unit by unit to machine time `until` and records the composed stamp of every access of the wanted kind
 * at the wanted address. 1 when the guest reached `until`; the caller checks the count, because a guest that never
 * strobes is the dead guest this family must not accept. */
static int sd_measure(const uint16_t *w, size_t n, uint32_t is_write, uint16_t addr, uint64_t until, sd_hits *h)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  ivs_ms_inst x;
  const struct ivs_machine *im;
  ivs_psg_sink sink;
  uint32_t units = 0u;
  int ok = 0;
  memset(h, 0, sizeof *h);
  if (!sc_guest(&x, w, n, 0u)) {
    return 0;
  }
  im = ivs_machine_from_handle(x.m);
  ivs_psg_sink_init(&sink, NULL, 0u);
  while (im->time_ticks < until && units++ < 40000u) {
    ivs_cp1610_access acc[8];
    ivs_cp1610_result r;
    uint64_t start = im->time_ticks;
    uint32_t j;
    if (ivs_machine_step_unit_trace_internal(x.m, &r, &sink, acc, 8u) != IVS_OK) {
      goto done;
    }
    for (j = 0u; j < r.n_accesses && j < 8u; j++) {
      uint32_t write = acc[j].kind == IVS_ACC_WRITE || acc[j].kind == IVS_ACC_DW;
      if (write != is_write || acc[j].addr != addr || h->n >= SD_HIT_MAX) {
        continue;
      }
      h->stamp[h->n] = start + (uint64_t)acc[j].offset_cpu_cycles * (uint64_t)p->ticks_per_cpu_cycle +
                       (uint64_t)p->access_strobe_ticks;
      h->cycle[h->n] = (uint32_t)(start / (uint64_t)p->ticks_per_cpu_cycle) + acc[j].offset_cpu_cycles;
      h->n += 1u;
    }
  }
  ok = im->time_ticks >= until;
done:
  ivs_ms_free(&x);
  return ok;
}

/* ---- the observation: the same stamps re-driven through the bus wrapper under a chosen row ---- */

/* Builds a machine whose own program never touches $0020 (EIS; B self), so the only accesses to the display-enable
 * strobe are the ones replayed here. */
static int sd_blank(ivs_ms_inst *x)
{
  uint16_t w[SC_PROG_MAX];
  size_t n;
  memset(w, 0, sizeof w);
  n = sc_prog_loop(w, 1u); /* $1000 EIS; $1001 B self - no access to $0020 at all */
  return sc_guest(x, w, n, 0u);
}

/* Replays the recorded accesses at their measured stamps through the machine's bus wrapper with the row q (the cell's
 * row, not the profile's), then catches the STIC up past frame 0's graphics-memory close and reads the stored view.
 * 1 on success. */
static int sd_observe(const ivs_stic_params *q, const sd_hits *h, uint32_t is_write, uint16_t addr,
                      ivs_stic_report *rep)
{
  ivs_ms_inst x;
  struct ivs_machine *im;
  size_t i;
  if (!sd_blank(&x)) {
    return 0;
  }
  im = ivs_machine_from_handle_mut(x.m);
  for (i = 0u; i < h->n; i++) {
    ivs_machine_bus_ctx c;
    ivs_cp1610_access a;
    memset(&c, 0, sizeof c);
    c.m = im;
    c.unit_start_tick = h->stamp[i] - q->access_strobe_ticks; /* the access is offset 0 of its unit */
    c.ticks_per_cpu_cycle = q->ticks_per_cpu_cycle;
    c.psg = ivs_psg_params_get(im->profile);
    c.stic = q;
    c.sink = NULL;
    memset(&a, 0, sizeof a);
    a.kind = is_write != 0u ? IVS_ACC_WRITE : IVS_ACC_READ;
    a.addr = addr;
    (void)ivs_machine_bus(&c, &a);
  }
  ivs_stic_catch_up(q, &im->stic, &im->mem, (uint64_t)q->gmem_window_ticks + 1u, NULL, NULL);
  memset(rep, 0, sizeof *rep);
  rep->struct_size = IVS_STIC_REPORT_V1_SIZE;
  if (ivs_machine_stic_state(x.m, rep) != IVS_OK) {
    ivs_ms_free(&x);
    return 0;
  }
  ivs_ms_free(&x);
  return 1;
}

/* ---- G1's guard: the witness's blocked counter takes both values under the window field alone ---- */

/* A limb of the guard that failed, one bit each, so the judge can say whether the stand-in failed for the intended
 * reason rather than merely failing. */
#define SD_G1_W_SHIPPED 0x01u /* the witness's blocked count is not 1 at the shipped window */
#define SD_G1_W_WIDE 0x02u    /* the witness's blocked count is not 0 at the alternate window */
#define SD_G1_C_SHIPPED 0x04u /* the criterion guest's displayed word is not 0 at the shipped window */
#define SD_G1_C_WIDE 0x08u    /* the criterion guest's displayed word is not 1 at the alternate window */
#define SD_G1_BROKEN 0x10u    /* a guest did not strobe $0020 the number of times its shape implies */

/* Runs G1's two-limb guard on `witness` (the real witness, or the committed stand-in) and returns the failing limbs.
 * 0 means the guard passed. */
static uint32_t sd_guard_g1(const uint16_t *witness, size_t witness_n, uint32_t *w_blocked, uint32_t *c_displayed)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint16_t crit[SC_PROG_MAX];
  size_t crit_n;
  ivs_stic_params wide;
  sd_hits wh;
  sd_hits ch;
  ivs_stic_report wr;
  ivs_stic_report cr;
  uint32_t bad = 0u;
  crit_n = sd_prog_g1_criterion(crit);
  if (!sd_row_wide(p, &wide)) {
    return SD_G1_BROKEN;
  }
  /* the criterion's guest strobes once; the witness (and its stand-in) strobes twice */
  if (!sd_measure(crit, crit_n, 1u, 0x0020u, p->frame_ticks, &ch) || ch.n != 1u) {
    return SD_G1_BROKEN;
  }
  if (!sd_measure(witness, witness_n, 1u, 0x0020u, p->frame_ticks, &wh) || wh.n != 2u) {
    return SD_G1_BROKEN;
  }
  /* the criterion's gap: strictly inside the 2000-to-2900 window, computed from the profile's own parameters */
  if (!(ch.stamp[0] > (uint64_t)p->reg_window_ticks &&
        ch.stamp[0] < (uint64_t)SD_WINDOW_WIDE_CYCLES * (uint64_t)p->ticks_per_cpu_cycle)) {
    ivs_test_diag("criterion stamp %llu is not strictly inside (%llu, %llu)", (unsigned long long)ch.stamp[0],
                  (unsigned long long)p->reg_window_ticks,
                  (unsigned long long)((uint64_t)SD_WINDOW_WIDE_CYCLES * p->ticks_per_cpu_cycle));
    return SD_G1_BROKEN;
  }
  /* the two limbs, each read twice: the shipped row first, then the alternate candidate */
  if (!sd_observe(p, &wh, 1u, 0x0020u, &wr)) {
    return SD_G1_BROKEN;
  }
  *w_blocked = wr.blocked_writes;
  if (wr.blocked_writes != 1u) {
    bad |= SD_G1_W_SHIPPED;
  }
  if (!sd_observe(&wide, &wh, 1u, 0x0020u, &wr)) {
    return SD_G1_BROKEN;
  }
  if (wr.blocked_writes != 0u) {
    bad |= SD_G1_W_WIDE;
  }
  if (!sd_observe(p, &ch, 1u, 0x0020u, &cr)) {
    return SD_G1_BROKEN;
  }
  *c_displayed = cr.windows_active;
  if (cr.windows_active != 0u) {
    bad |= SD_G1_C_SHIPPED;
  }
  if (!sd_observe(&wide, &ch, 1u, 0x0020u, &cr)) {
    return SD_G1_BROKEN;
  }
  if (cr.windows_active != 1u) {
    bad |= SD_G1_C_WIDE;
  }
  return bad;
}

/* G1's case. The criterion's own guest gives frame 0 displayed = no at the shipped window and yes at the alternate, which
 * is the criterion's stated observation; the witness gives a blocked count of 1 at the shipped window and 0 at the
 * alternate, which is the registered one. The two readings are the coupling, measured: the displayed word moves under the
 * window field, so it can never be a witness for one field, while the counter does not move with the pre-latch. */
static int sd_g1_window(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint16_t wit[SC_PROG_MAX];
  uint16_t crit[SC_PROG_MAX];
  size_t wit_n;
  size_t crit_n;
  uint32_t blocked = 0u;
  uint32_t displayed = 0u;
  uint32_t bad;
  ivs_stic_params open;
  sd_hits ch;
  sd_hits wh;
  ivs_stic_report r;
  wit_n = sd_prog_g1_witness(wit);
  crit_n = sd_prog_g1_criterion(crit);
  bad = sd_guard_g1(wit, wit_n, &blocked, &displayed);
  if (bad != 0u) {
    ivs_test_diag("G1 guard failed: limbs 0x%02X (witness blocked %u, criterion displayed %u)", (unsigned)bad,
                  (unsigned)blocked, (unsigned)displayed);
    return IVS_TEST_FAIL;
  }
  /* The coupling, measured in the case rather than asserted in a comment: a latch-clear access inside the gap is exactly
   * what the gating alternate unblocks, so the criterion guest's displayed word moves under the gating field as well as
   * under the window field - it can never be a witness for one field. The witness's counter does not move, because with
   * the display already on the gating alternate's condition ("the latch is clear") is false and the window schedule alone
   * decides the write. That is why the counter is the registered observable and the displayed word is not. */
  open = *p;
  open.access_gating_when_off = 0u;
  SC_CHECK(sd_measure(crit, crit_n, 1u, 0x0020u, p->frame_ticks, &ch) && ch.n == 1u);
  SC_CHECK(sd_measure(wit, wit_n, 1u, 0x0020u, p->frame_ticks, &wh) && wh.n == 2u);
  SC_CHECK(sd_observe(&open, &ch, 1u, 0x0020u, &r));
  if (r.windows_active != 1u) {
    ivs_test_diag("gating alternate: the criterion guest's displayed word is %u, not 1 (blocked %u)",
                  (unsigned)r.windows_active, (unsigned)r.blocked_writes);
    return IVS_TEST_FAIL;
  }
  SC_CHECK(sd_observe(&open, &wh, 1u, 0x0020u, &r));
  if (r.blocked_writes != 1u) {
    ivs_test_diag("gating alternate: the witness's blocked count is %u, not 1 (displayed %u)",
                  (unsigned)r.blocked_writes, (unsigned)r.windows_active);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

/* G1's judge: the same guard, run on the committed degenerate guest. It passes only when the guard fails, and only when
 * it fails on the witness limbs - a guard that also failed the criterion limbs would be failing for a reason the
 * stand-in does not name. */
static int sd_judge_g1_dead(void)
{
  uint16_t dead[SC_PROG_MAX];
  size_t dead_n;
  uint32_t blocked = 0u;
  uint32_t displayed = 0u;
  uint32_t bad;
  dead_n = sd_prog_g1_dead(dead);
  bad = sd_guard_g1(dead, dead_n, &blocked, &displayed);
  if (bad == 0u) {
    ivs_test_diag("judge-vacuous: the guard passed on the stand-in (blocked %u, displayed %u)", (unsigned)blocked,
                  (unsigned)displayed);
    return IVS_TEST_FAIL;
  }
  if ((bad & (SD_G1_W_SHIPPED | SD_G1_W_WIDE)) == 0u || (bad & (SD_G1_C_SHIPPED | SD_G1_C_WIDE | SD_G1_BROKEN)) != 0u) {
    ivs_test_diag("judge-wrong-reason: limbs 0x%02X (blocked %u, displayed %u)", (unsigned)bad, (unsigned)blocked,
                  (unsigned)displayed);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

/* ---- the displayed-word equivalence (research O2, settled by pinning instead of by choosing) ---- */

/* The two readings of one word, each with the machine time it was read at. The equivalence is an *instant* equality: at
 * one instant `windows_active` equals "the display latch is non-zero". Two readings taken at different instants are not
 * the same claim even when their values agree, which is the mistake this pair of ticks exists to catch. */
typedef struct sd_words {
  uint32_t displayed;
  uint64_t displayed_tick;
  uint32_t latch;
  uint64_t latch_tick;
} sd_words;

#define SD_EQ_OK 0u
#define SD_EQ_INSTANT 1u /* the two words were not read at the same machine time */
#define SD_EQ_VALUE 2u   /* at the same instant the two words disagree */

/* The guard: 0 when the pair is the equivalence, else the reason it is not. */
static uint32_t sd_equiv_guard(const sd_words *w)
{
  if (w->displayed_tick != w->latch_tick) {
    return SD_EQ_INSTANT;
  }
  if (w->displayed != (w->latch != 0u ? 1u : 0u)) {
    return SD_EQ_VALUE;
  }
  return SD_EQ_OK;
}

/* Re-drives the write at its measured stamp under the row q, catches the STIC up to tick `at` and reads both words
 * there, so both carry the same instant. */
static int sd_words_at(const ivs_stic_params *q, const sd_hits *h, uint64_t at, sd_words *w)
{
  ivs_ms_inst x;
  struct ivs_machine *im;
  ivs_stic_report rep;
  size_t i;
  if (!sd_blank(&x)) {
    return 0;
  }
  im = ivs_machine_from_handle_mut(x.m);
  for (i = 0u; i < h->n; i++) {
    ivs_machine_bus_ctx c;
    ivs_cp1610_access a;
    memset(&c, 0, sizeof c);
    c.m = im;
    c.unit_start_tick = h->stamp[i] - q->access_strobe_ticks;
    c.ticks_per_cpu_cycle = q->ticks_per_cpu_cycle;
    c.psg = ivs_psg_params_get(im->profile);
    c.stic = q;
    c.sink = NULL;
    memset(&a, 0, sizeof a);
    a.kind = IVS_ACC_WRITE;
    a.addr = 0x0020u;
    (void)ivs_machine_bus(&c, &a);
  }
  ivs_stic_catch_up(q, &im->stic, &im->mem, at, NULL, NULL);
  memset(&rep, 0, sizeof rep);
  rep.struct_size = IVS_STIC_REPORT_V1_SIZE;
  if (ivs_machine_stic_state(x.m, &rep) != IVS_OK) {
    ivs_ms_free(&x);
    return 0;
  }
  w->displayed = rep.windows_active;
  w->displayed_tick = at;
  w->latch = rep.display_latch;
  w->latch_tick = at;
  ivs_ms_free(&x);
  return 1;
}

/* The committed stand-in: the same two words, from two different instants. `windows_active` is read at the
 * graphics-memory window close of frame 0 and the display latch after the next frame start's clear, which is the
 * plausible mistake - on the shipped row both values are 0, so a guard that compared values would accept the pair. */
static int sd_equiv_dead(const ivs_stic_params *q, const sd_hits *h, sd_words *w)
{
  ivs_ms_inst x;
  struct ivs_machine *im;
  ivs_stic_report rep;
  size_t i;
  if (!sd_blank(&x)) {
    return 0;
  }
  im = ivs_machine_from_handle_mut(x.m);
  for (i = 0u; i < h->n; i++) {
    ivs_machine_bus_ctx c;
    ivs_cp1610_access a;
    memset(&c, 0, sizeof c);
    c.m = im;
    c.unit_start_tick = h->stamp[i] - q->access_strobe_ticks;
    c.ticks_per_cpu_cycle = q->ticks_per_cpu_cycle;
    c.psg = ivs_psg_params_get(im->profile);
    c.stic = q;
    c.sink = NULL;
    memset(&a, 0, sizeof a);
    a.kind = IVS_ACC_WRITE;
    a.addr = 0x0020u;
    (void)ivs_machine_bus(&c, &a);
  }
  /* first instant: the graphics-memory window close of frame 0 */
  ivs_stic_catch_up(q, &im->stic, &im->mem, (uint64_t)q->gmem_window_ticks, NULL, NULL);
  memset(&rep, 0, sizeof rep);
  rep.struct_size = IVS_STIC_REPORT_V1_SIZE;
  if (ivs_machine_stic_state(x.m, &rep) != IVS_OK) {
    ivs_ms_free(&x);
    return 0;
  }
  w->displayed = rep.windows_active;
  w->displayed_tick = (uint64_t)q->gmem_window_ticks;
  /* second instant: after the next frame start has cleared the latch */
  ivs_stic_catch_up(q, &im->stic, &im->mem, (uint64_t)q->frame_ticks, NULL, NULL);
  memset(&rep, 0, sizeof rep);
  rep.struct_size = IVS_STIC_REPORT_V1_SIZE;
  if (ivs_machine_stic_state(x.m, &rep) != IVS_OK) {
    ivs_ms_free(&x);
    return 0;
  }
  w->latch = rep.display_latch;
  w->latch_tick = (uint64_t)q->frame_ticks;
  ivs_ms_free(&x);
  return 1;
}

/* `stic.disc.displayed-equivalence`: `windows_active` at the graphics-memory window close equals whether the display
 * latch is non-zero at that same instant, in both polarities - both zero on the shipped row (the gap write is blocked),
 * both one on the alternate window (the same write lands). The other two readings of the same word are the delivered
 * slot header's IVS_FRAME_F_DISPLAYED bit, already held by `video.slot.displayed-flag-follows-latch`, and the runner's
 * `frame N displayed` line, which prints that bit (runner/report.c). The report's key table is closed and carries no
 * blocked-write counter, so this family reads machine state and the report line is a rendering of one of them. */
static int sd_displayed_equivalence(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint16_t crit[SC_PROG_MAX];
  size_t crit_n;
  ivs_stic_params wide;
  sd_hits ch;
  sd_words w;
  crit_n = sd_prog_g1_criterion(crit);
  SC_CHECK(sd_row_wide(p, &wide));
  SC_CHECK(sd_measure(crit, crit_n, 1u, 0x0020u, p->frame_ticks, &ch));
  SC_CHECK(ch.n == 1u);
  /* polarity 1: the shipped row, where the write is blocked and both words are zero */
  SC_CHECK(sd_words_at(p, &ch, (uint64_t)p->gmem_window_ticks, &w));
  if (sd_equiv_guard(&w) != SD_EQ_OK) {
    ivs_test_diag("shipped: displayed %u at %llu, latch %u at %llu (reason %u)", (unsigned)w.displayed,
                  (unsigned long long)w.displayed_tick, (unsigned)w.latch, (unsigned long long)w.latch_tick,
                  (unsigned)sd_equiv_guard(&w));
    return IVS_TEST_FAIL;
  }
  if (w.displayed != 0u || w.latch != 0u) {
    ivs_test_diag("shipped: the write was not blocked (displayed %u, latch %u)", (unsigned)w.displayed,
                  (unsigned)w.latch);
    return IVS_TEST_FAIL;
  }
  /* polarity 2: the alternate window, where the same write lands and both words are one */
  SC_CHECK(sd_words_at(&wide, &ch, (uint64_t)wide.gmem_window_ticks, &w));
  if (sd_equiv_guard(&w) != SD_EQ_OK) {
    ivs_test_diag("alternate: displayed %u at %llu, latch %u at %llu (reason %u)", (unsigned)w.displayed,
                  (unsigned long long)w.displayed_tick, (unsigned)w.latch, (unsigned long long)w.latch_tick,
                  (unsigned)sd_equiv_guard(&w));
    return IVS_TEST_FAIL;
  }
  if (w.displayed != 1u || w.latch != 1u) {
    ivs_test_diag("alternate: the write did not land (displayed %u, latch %u)", (unsigned)w.displayed,
                  (unsigned)w.latch);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

/* The equivalence's judge: the same guard on the committed stand-in, which supplies its two words from two different
 * instants. It passes only when the guard rejects the pair, and only when it rejects it for the instant - a guard that
 * rejected the values would not be testing the claim. */
static int sd_judge_equivalence_dead(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint16_t crit[SC_PROG_MAX];
  size_t crit_n;
  sd_hits ch;
  sd_words w;
  uint32_t reason;
  crit_n = sd_prog_g1_criterion(crit);
  SC_CHECK(sd_measure(crit, crit_n, 1u, 0x0020u, p->frame_ticks, &ch));
  SC_CHECK(ch.n == 1u);
  SC_CHECK(sd_equiv_dead(p, &ch, &w));
  reason = sd_equiv_guard(&w);
  if (reason == SD_EQ_OK) {
    ivs_test_diag("judge-vacuous: the guard accepted a pair read at %llu and %llu (displayed %u, latch %u)",
                  (unsigned long long)w.displayed_tick, (unsigned long long)w.latch_tick, (unsigned)w.displayed,
                  (unsigned)w.latch);
    return IVS_TEST_FAIL;
  }
  if (reason != SD_EQ_INSTANT) {
    ivs_test_diag("judge-wrong-reason: reason %u (displayed %u at %llu, latch %u at %llu)", (unsigned)reason,
                  (unsigned)w.displayed, (unsigned long long)w.displayed_tick, (unsigned)w.latch,
                  (unsigned long long)w.latch_tick);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

/* ---- the family table ---- */

const ivs_test_case ivs_stic_disc_cases[] = {
    {"stic.disc.g1-window", sd_g1_window, NULL},
    {"stic.judge.disc-g1-dead", sd_judge_g1_dead, NULL},
    {"stic.disc.displayed-equivalence", sd_displayed_equivalence, NULL},
    {"stic.judge.disc-equivalence-dead", sd_judge_equivalence_dead, NULL},
};

const size_t ivs_stic_disc_count = sizeof ivs_stic_disc_cases / sizeof ivs_stic_disc_cases[0];
