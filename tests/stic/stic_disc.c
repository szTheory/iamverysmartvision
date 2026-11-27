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
 * case re-drives that same stamp under a perturbed row.
 *
 * Plan 19-03 adds the sweep that the four guests exist for: `stic.disc.sweep-diagonal` builds all sixteen cells of the
 * four rival fields as stack copies of the generated row and asserts ONE sentence - each guest's registered observable
 * takes its alternate value if and only if the cell sets that guest's own field, whatever the other three bits are -
 * so a guest that moves under two fields is caught by the same comparison that proves the other three diagonal, and
 * `stic.judge.disc-diagonal-two-fields` runs that comparison on G1's observable read the entangled way and is red if it
 * is accepted. The sixteen-cell matrix is one id and not sixteen: written as sixteen expected vectors it would be
 * sixteen fits, and the phase's cost line prices one in-process id. */

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
  uint8_t write[SD_HIT_MAX]; /* 1 for WRITE/DW, 0 for READ/FETCH: the kind the guest actually made */
} sd_hits;

/* The kind filter of sd_measure: a guest is measured for the accesses its shape implies. SD_ANY measures either kind,
 * which is how G2's and G3's guards run the SAME guard on a stand-in that makes the other kind. */
#define SD_READS 0u
#define SD_WRITES 1u
#define SD_ANY 2u

/* Runs the guest unit by unit to machine time `until` and records the composed stamp of every access of the wanted kind
 * at the wanted address. 1 when the guest reached `until`; the caller checks the count, because a guest that never
 * strobes is the dead guest this family must not accept. */
static int sd_measure(const uint16_t *w, size_t n, uint32_t kind_filter, uint16_t addr, uint64_t until,
                      sd_hits *h)
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
      if ((kind_filter != SD_ANY && write != kind_filter) || acc[j].addr != addr || h->n >= SD_HIT_MAX) {
        continue;
      }
      h->stamp[h->n] = start + (uint64_t)acc[j].offset_cpu_cycles * (uint64_t)p->ticks_per_cpu_cycle +
                       (uint64_t)p->access_strobe_ticks;
      h->cycle[h->n] = (uint32_t)(start / (uint64_t)p->ticks_per_cpu_cycle) + acc[j].offset_cpu_cycles;
      h->write[h->n] = (uint8_t)(write != 0u ? 1u : 0u);
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

/* Replays ONE recorded access through the bus wrapper at the stamp the guest itself measured, with the row q (the
 * cell's row, not the profile's) and the kind the guest actually made. This is the one place the stamp is composed:
 * h->stamp[i] is the instant of the access, so its unit started that many ticks less the strobe offset (the access is
 * offset 0 of its unit). The family's whole claim rests on that composition, so a second copy of it is how a future
 * guest gets mis-stamped while the suite stays green (PLAN-LESSONS lesson 13, applied to the stamp). */
static void sd_replay_one(struct ivs_machine *im, const ivs_stic_params *q, const sd_hits *h, size_t i, uint16_t addr)
{
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
  a.kind = h->write[i] != 0u ? IVS_ACC_WRITE : IVS_ACC_READ; /* the kind the guest made, never a hardcoded one */
  a.addr = addr;
  (void)ivs_machine_bus(&c, &a);
}

/* Replays the first n recorded hits through the bus wrapper with the row q, keeping only those of `filter` kind
 * (SD_READS, SD_WRITES, or SD_ANY for each hit's own kind). SD_ANY is how one guard runs unchanged on a stand-in whose
 * guest makes the other kind of access; the kind always comes from the hit and the address from the caller, so a guest
 * whose observable is read at another address replays the access it measured rather than a plausible substitute. */
static void sd_replay(struct ivs_machine *im, const ivs_stic_params *q, const sd_hits *h, size_t n, uint32_t filter,
                      uint16_t addr)
{
  size_t i;
  for (i = 0u; i < n && i < h->n; i++) {
    if (filter != SD_ANY && (uint32_t)h->write[i] != filter) {
      continue;
    }
    sd_replay_one(im, q, h, i, addr);
  }
}

/* Replays the recorded accesses at their measured stamps under the row q, then catches the STIC up past frame 0's
 * graphics-memory close and reads the stored view. `filter` is the kind of hit to replay (SD_ANY takes each hit's own
 * kind). 1 on success. */
static int sd_observe_x(const ivs_stic_params *q, const sd_hits *h, uint32_t filter, uint16_t addr,
                        ivs_stic_report *rep, uint16_t *regs)
{
  ivs_ms_inst x;
  struct ivs_machine *im;
  if (!sd_blank(&x)) {
    return 0;
  }
  im = ivs_machine_from_handle_mut(x.m);
  sd_replay(im, q, h, h->n, filter, addr);
  ivs_stic_catch_up(q, &im->stic, &im->mem, (uint64_t)q->gmem_window_ticks + 1u, NULL, NULL);
  memset(rep, 0, sizeof *rep);
  rep->struct_size = IVS_STIC_REPORT_V1_SIZE;
  if (ivs_machine_stic_state(x.m, rep) != IVS_OK) {
    ivs_ms_free(&x);
    return 0;
  }
  if (regs != NULL) {
    memcpy(regs, rep->regs, IVS_STIC_REG_COUNT * sizeof regs[0]);
  }
  ivs_ms_free(&x);
  return 1;
}

static int sd_observe(const ivs_stic_params *q, const sd_hits *h, uint32_t filter, uint16_t addr,
                      ivs_stic_report *rep)
{
  return sd_observe_x(q, h, filter, addr, rep, NULL);
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
  if (!sd_measure(crit, crit_n, SD_WRITES, 0x0020u, p->frame_ticks, &ch) || ch.n != 1u) {
    return SD_G1_BROKEN;
  }
  if (!sd_measure(witness, witness_n, SD_WRITES, 0x0020u, p->frame_ticks, &wh) || wh.n != 2u) {
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
  SC_CHECK(sd_measure(crit, crit_n, SD_WRITES, 0x0020u, p->frame_ticks, &ch) && ch.n == 1u);
  SC_CHECK(sd_measure(wit, wit_n, SD_WRITES, 0x0020u, p->frame_ticks, &wh) && wh.n == 2u);
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
  if (!sd_blank(&x)) {
    return 0;
  }
  im = ivs_machine_from_handle_mut(x.m);
  sd_replay(im, q, h, h->n, SD_WRITES, 0x0020u);
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
  if (!sd_blank(&x)) {
    return 0;
  }
  im = ivs_machine_from_handle_mut(x.m);
  sd_replay(im, q, h, h->n, SD_WRITES, 0x0020u);
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
  SC_CHECK(sd_measure(crit, crit_n, SD_WRITES, 0x0020u, p->frame_ticks, &ch));
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
  SC_CHECK(sd_measure(crit, crit_n, SD_WRITES, 0x0020u, p->frame_ticks, &ch));
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

/* ---- G2: whether the access windows apply while the display latch is clear (C94) ---- */

/* G2's delay: `sc_w_delay(200)` is 3006 cycles, so with the leading MVII and the MVO's own offset the write is stamped
 * past 2900 CPU cycles and still short of the graphics-memory close at 3780 - the criterion's "outside both window
 * candidates", asserted as a relation in the case and never written as a tick literal. */
#define SD_G2_DELAY 200u

/* G2, the criterion's guest: one write to $0020 stamped past both window candidates, with the display latch clear. */
static size_t sd_prog_g2(uint16_t *w)
{
  size_t n = 0u;
  n = sc_w_mvii(w, n, 0u, 0x0000u); /* $1000 MVII #0, R0        8 cycles */
  n = sc_w_delay(w, n, SD_G2_DELAY); /* $1002 MVII #200,R1; DECR; BNZE   3006 cycles */
  n = sc_w_mvo(w, n, 0u, 0x0020u);   /* $1007 MVO R0, $0020      11 cycles, past both candidates */
  return sc_w_loop(w, n);            /* $1009 B self */
}

/* G2's committed stand-in: the write lands inside the window, so it is served under both settings and the guard fails. */
static size_t sd_prog_g2_dead(uint16_t *w)
{
  size_t n = 0u;
  n = sc_w_mvii(w, n, 0u, 0x0000u); /* $1000 MVII #0, R0        8 cycles */
  n = sc_w_mvo(w, n, 0u, 0x0020u);  /* $1002 MVO R0, $0020      11 cycles, inside the window */
  return sc_w_loop(w, n);           /* $1004 B self */
}

/* A stack copy with the gating member at 0: the windows are open while the display is off. */
static void sd_row_gating_open(const ivs_stic_params *p, ivs_stic_params *q)
{
  *q = *p;
  q->access_gating_when_off = 0u;
}

#define SD_G2_SHIPPED 0x01u /* the blocked count is not 1 at the shipped row */
#define SD_G2_OPEN 0x02u    /* the blocked count is not 0 under the gating alternate */
#define SD_G2_BROKEN 0x04u  /* the guest did not strobe $0020 exactly once */

/* G2's guard: the blocked-write counter takes both values under the gating field alone. 0 means it passed. */
static uint32_t sd_guard_g2(const uint16_t *guest, size_t guest_n, uint32_t *shipped, uint32_t *open_count)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  ivs_stic_params open;
  sd_hits h;
  ivs_stic_report r;
  uint32_t bad = 0u;
  sd_row_gating_open(p, &open);
  if (!sd_measure(guest, guest_n, SD_WRITES, 0x0020u, p->frame_ticks, &h) || h.n != 1u) {
    return SD_G2_BROKEN;
  }
  if (!sd_observe(p, &h, 1u, 0x0020u, &r)) {
    return SD_G2_BROKEN;
  }
  *shipped = r.blocked_writes;
  if (r.blocked_writes != 1u) {
    bad |= SD_G2_SHIPPED;
  }
  if (!sd_observe(&open, &h, 1u, 0x0020u, &r)) {
    return SD_G2_BROKEN;
  }
  *open_count = r.blocked_writes;
  if (r.blocked_writes != 0u) {
    bad |= SD_G2_OPEN;
  }
  return bad;
}

/* G2's case. The report's key table is closed and carries no STIC counter, so this observable has no report line at all -
 * it is read from `ivs_machine_stic_state` and nowhere else. The default is one blocked write: the write is outside the
 * window and the closed register gate drops it. */
static int sd_g2_gating(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = sd_prog_g2(w);
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint32_t shipped = 0u;
  uint32_t open_count = 0u;
  uint32_t bad;
  sd_hits h;
  /* the criterion's "outside both window candidates": past the alternate and short of the graphics-memory close, both
   * converted from the profile's own parameters */
  SC_CHECK(sd_measure(w, n, SD_WRITES, 0x0020u, p->frame_ticks, &h) && h.n == 1u);
  if (!(h.stamp[0] > (uint64_t)SD_WINDOW_WIDE_CYCLES * (uint64_t)p->ticks_per_cpu_cycle &&
        h.stamp[0] < (uint64_t)p->gmem_window_ticks)) {
    ivs_test_diag("G2 stamp %llu is not strictly inside (%llu, %llu)", (unsigned long long)h.stamp[0],
                  (unsigned long long)((uint64_t)SD_WINDOW_WIDE_CYCLES * p->ticks_per_cpu_cycle),
                  (unsigned long long)p->gmem_window_ticks);
    return IVS_TEST_FAIL;
  }
  bad = sd_guard_g2(w, n, &shipped, &open_count);
  if (bad != 0u) {
    ivs_test_diag("G2 guard failed: limbs 0x%02X (shipped %u, gating alternate %u)", (unsigned)bad, (unsigned)shipped,
                  (unsigned)open_count);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

static int sd_judge_g2_dead(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = sd_prog_g2_dead(w);
  uint32_t shipped = 0u;
  uint32_t open_count = 0u;
  uint32_t bad = sd_guard_g2(w, n, &shipped, &open_count);
  if (bad == 0u) {
    ivs_test_diag("judge-vacuous: the guard passed on the stand-in (shipped %u, gating alternate %u)",
                  (unsigned)shipped, (unsigned)open_count);
    return IVS_TEST_FAIL;
  }
  if ((bad & SD_G2_SHIPPED) == 0u || (bad & (SD_G2_OPEN | SD_G2_BROKEN)) != 0u) {
    ivs_test_diag("judge-wrong-reason: limbs 0x%02X (shipped %u, gating alternate %u)", (unsigned)bad,
                  (unsigned)shipped, (unsigned)open_count);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

/* ---- G3: whether a READ of $0020 strobes the display (C82, bounded by C84) ---- */

/* G3, the criterion's guest: one READ of $0020, stamped inside both window candidates, and no write at all. */
static size_t sd_prog_g3(uint16_t *w)
{
  size_t n = 0u;
  n = sc_w_mvii(w, n, 0u, 0x0000u); /* $1000 MVII #0, R0        8 cycles */
  n = sc_w_mvi(w, n, 0x0020u, 1u);  /* $1002 MVI $0020, R1      10 cycles, the read */
  return sc_w_loop(w, n);           /* $1004 B self */
}

/* G3's committed stand-in: a WRITE of $0020 instead of a read, so both settings display and the guard fails. */
static size_t sd_prog_g3_dead(uint16_t *w)
{
  size_t n = 0u;
  n = sc_w_mvii(w, n, 0u, 0x0000u); /* $1000 MVII #0, R0        8 cycles */
  n = sc_w_mvo(w, n, 0u, 0x0020u);  /* $1002 MVO R0, $0020      11 cycles, a write where the guest reads */
  return sc_w_loop(w, n);           /* $1004 B self */
}

/* A stack copy with the $0020 READ row of stic.access_effects carrying the arm effect: the third rival field's parameter
 * is that row, not a member, because the row already exists in the profile with the effect `none`. */
static int sd_row_read_arms(const ivs_stic_params *p, ivs_stic_params *q)
{
  uint32_t i;
  uint32_t found = 0u;
  *q = *p;
  for (i = 0u; i < q->effect_count; i++) {
    if ((uint32_t)q->effect_access[i] == IVS_STIC_ACCESS_READ && (uint32_t)q->effect_addr[i] == 0x0020u) {
      q->effect_code[i] = IVS_STIC_EFFECT_ARM;
      found = 1u;
    }
  }
  return (int)found;
}

#define SD_G3_SHIPPED 0x01u /* the read displayed frame 0 at the shipped row */
#define SD_G3_ARMED 0x02u   /* the read did not display frame 0 under the arming row */
#define SD_G3_BROKEN 0x04u  /* the guest did not touch $0020 the way its shape implies */

/* G3's guard: the frame-0 displayed word takes both values under the $0020 READ row alone. 0 means it passed. */
static uint32_t sd_guard_g3(const uint16_t *guest, size_t guest_n, uint32_t *shipped, uint32_t *armed)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  ivs_stic_params arms;
  sd_hits h;
  ivs_stic_report r;
  uint32_t bad = 0u;
  if (!sd_row_read_arms(p, &arms)) {
    return SD_G3_BROKEN;
  }
  if (!sd_measure(guest, guest_n, SD_ANY, 0x0020u, p->frame_ticks, &h) || h.n != 1u) {
    return SD_G3_BROKEN;
  }
  if (!sd_observe_x(p, &h, SD_ANY, 0x0020u, &r, NULL)) {
    return SD_G3_BROKEN;
  }
  *shipped = r.windows_active;
  if (r.windows_active != 0u) {
    bad |= SD_G3_SHIPPED;
  }
  if (!sd_observe_x(&arms, &h, SD_ANY, 0x0020u, &r, NULL)) {
    return SD_G3_BROKEN;
  }
  *armed = r.windows_active;
  if (r.windows_active != 1u) {
    bad |= SD_G3_ARMED;
  }
  return bad;
}

/* G3's case, and C84's three limbs in the same comparison. The default is "not displayed": a read has no effect, because
 * the profile's $0020 READ row carries `none` (rule STIC-R15). The guard's second limb sets that row's effect on a stack
 * copy, which is the whole of the third rival field. The three further limbs are what the exact-address READ match must
 * not have swallowed: the read leaves the register file unchanged; a read of $0040 has no effect even on the arming row,
 * because a match that also armed on a mirrored address is a defect the diagonal would never catch; and a read of $4021
 * still switches Color Stack mode, because the mode-select limb is what the exact-address match keeps. */
static int sd_g3_read(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint16_t w[SC_PROG_MAX];
  size_t n = sd_prog_g3(w);
  uint32_t shipped = 0u;
  uint32_t armed = 0u;
  uint32_t bad = sd_guard_g3(w, n, &shipped, &armed);
  ivs_stic_params arms;
  sd_hits h;
  sd_hits none;
  ivs_stic_report r;
  uint16_t before[IVS_STIC_REG_COUNT];
  uint16_t after[IVS_STIC_REG_COUNT];
  if (bad != 0u) {
    ivs_test_diag("G3 guard failed: limbs 0x%02X (shipped %u, arming row %u)", (unsigned)bad, (unsigned)shipped,
                  (unsigned)armed);
    return IVS_TEST_FAIL;
  }
  SC_CHECK(sd_row_read_arms(p, &arms));
  SC_CHECK(sd_measure(w, n, SD_READS, 0x0020u, p->frame_ticks, &h) && h.n == 1u);
  memset(&none, 0, sizeof none);
  /* limb 1: the read leaves the register file exactly as power-on left it */
  SC_CHECK(sd_observe_x(p, &none, 0u, 0x0020u, &r, before));
  SC_CHECK(sd_observe_x(p, &h, 0u, 0x0020u, &r, after));
  if (memcmp(before, after, sizeof before) != 0) {
    ivs_test_diag("the read of $0020 changed the register file");
    return IVS_TEST_FAIL;
  }
  /* limb 2: a read of $0040 has no effect, even with the $0020 READ row arming - C84's stated default */
  SC_CHECK(sd_observe(&arms, &h, 0u, 0x0040u, &r));
  if (r.windows_active != 0u || r.mode != 0u) {
    ivs_test_diag("a read of $0040 had an effect (displayed %u, mode %u)", (unsigned)r.windows_active,
                  (unsigned)r.mode);
    return IVS_TEST_FAIL;
  }
  /* limb 3: a read of $4021 still selects Color Stack mode - the register-index limb the exact match must not swallow */
  SC_CHECK(sd_observe(p, &h, 0u, 0x4021u, &r));
  if (r.mode != IVS_STIC_MODE_COLOR_STACK) {
    ivs_test_diag("a read of $4021 did not select Color Stack mode (mode %u)", (unsigned)r.mode);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

static int sd_judge_g3_dead(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = sd_prog_g3_dead(w);
  uint32_t shipped = 0u;
  uint32_t armed = 0u;
  uint32_t bad = sd_guard_g3(w, n, &shipped, &armed);
  if (bad == 0u) {
    ivs_test_diag("judge-vacuous: the guard passed on the stand-in (shipped %u, arming row %u)", (unsigned)shipped,
                  (unsigned)armed);
    return IVS_TEST_FAIL;
  }
  if ((bad & SD_G3_SHIPPED) == 0u || (bad & (SD_G3_ARMED | SD_G3_BROKEN)) != 0u) {
    ivs_test_diag("judge-wrong-reason: limbs 0x%02X (shipped %u, arming row %u)", (unsigned)bad, (unsigned)shipped,
                  (unsigned)armed);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

/* ---- G4: whether one strobe sustains the display for more than one frame (C82) ---- */

/* G4, the criterion's guest: one write to $0020 inside both windows, never re-strobed. */
static size_t sd_prog_g4(uint16_t *w)
{
  size_t n = 0u;
  n = sc_w_mvii(w, n, 0u, 0x0000u); /* $1000 MVII #0, R0        8 cycles */
  n = sc_w_mvo(w, n, 0u, 0x0020u);  /* $1002 MVO R0, $0020      11 cycles, inside the window */
  return sc_w_loop(w, n);           /* $1004 B self, forever */
}

/* G4's committed stand-in: re-strobes every frame, so both rules give (yes, yes, yes) and the guard fails. The delay is
 * 15 x 995 + 6 = 14931 cycles, just under the 14934-cycle frame, so the three strobes land in frames 0, 1 and 2. */
static size_t sd_prog_g4_dead(uint16_t *w)
{
  size_t n = 0u;
  n = sc_w_mvii(w, n, 0u, 0x0000u); /* $1000 MVII #0, R0        8 cycles */
  n = sc_w_mvo(w, n, 0u, 0x0020u);  /* $1002 MVO R0, $0020      frame 0 */
  n = sc_w_delay(w, n, 995u);       /* $1004 14931 cycles */
  n = sc_w_mvo(w, n, 0u, 0x0020u);  /* $1009 MVO R0, $0020      frame 1 */
  n = sc_w_delay(w, n, 995u);       /* $100B 14931 cycles */
  n = sc_w_mvo(w, n, 0u, 0x0020u);  /* $1010 MVO R0, $0020      frame 2 */
  return sc_w_loop(w, n);           /* $1012 B self */
}

/* A stack copy with the latch member at 0: the latch persists across frame starts. */
static void sd_row_latch_persists(const ivs_stic_params *p, ivs_stic_params *q)
{
  *q = *p;
  q->display_latch_cleared_each_frame = 0u;
}

/* The displayed word at each of the first three graphics-memory window closes, so the observable is a three-element
 * vector and not a scalar. 1 on success. */
static int sd_g4_vector(const ivs_stic_params *q, const sd_hits *h, uint32_t v[3])
{
  ivs_ms_inst x;
  struct ivs_machine *im;
  ivs_stic_report rep;
  size_t i = 0u;
  uint32_t k;
  if (!sd_blank(&x)) {
    return 0;
  }
  im = ivs_machine_from_handle_mut(x.m);
  for (k = 0u; k < 3u; k++) {
    uint64_t at = (uint64_t)k * (uint64_t)q->frame_ticks + (uint64_t)q->gmem_window_ticks;
    /* every write stamped at or before this frame's close is replayed first, then the close is read: one ascending pass,
     * so a guest that re-strobes cannot move the cursor past an earlier close before that close has been read */
    for (; i < h->n && h->stamp[i] <= at; i++) {
      sd_replay_one(im, q, h, i, 0x0020u);
    }
    ivs_stic_catch_up(q, &im->stic, &im->mem, at, NULL, NULL);
    memset(&rep, 0, sizeof rep);
    rep.struct_size = IVS_STIC_REPORT_V1_SIZE;
    if (ivs_machine_stic_state(x.m, &rep) != IVS_OK) {
      ivs_ms_free(&x);
      return 0;
    }
    v[k] = rep.windows_active;
  }
  ivs_ms_free(&x);
  return 1;
}

#define SD_G4_SHIPPED 0x01u /* the shipped vector is not (yes, no, no) */
#define SD_G4_PERSIST 0x02u /* the persist vector is not (yes, yes, yes) */
#define SD_G4_BROKEN 0x04u  /* the guest did not strobe, or the vector could not be read */

/* G4's guard: the three-element displayed vector takes both values under the latch field alone. 0 means it passed. */
static uint32_t sd_guard_g4(const uint16_t *guest, size_t guest_n, uint32_t shipped[3], uint32_t persist[3])
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  ivs_stic_params keep;
  sd_hits h;
  sd_row_latch_persists(p, &keep);
  if (!sd_measure(guest, guest_n, SD_WRITES, 0x0020u, 3u * (uint64_t)p->frame_ticks, &h) || h.n == 0u) {
    return SD_G4_BROKEN;
  }
  if (!sd_g4_vector(p, &h, shipped) || !sd_g4_vector(&keep, &h, persist)) {
    return SD_G4_BROKEN;
  }
  /* a guest that stops inside frame 0 cannot distinguish the two rules: fail with a diagnostic, never pass on a partial
   * vector. One strobe in frame 0 is enough for the shipped rule, so the requirement is on the reading, not on the count. */
  if (shipped[0] != 1u && persist[0] != 1u) {
    ivs_test_diag("G4 could not read a displayed frame 0 at all (shipped %u %u %u, persist %u %u %u)",
                  (unsigned)shipped[0], (unsigned)shipped[1], (unsigned)shipped[2], (unsigned)persist[0],
                  (unsigned)persist[1], (unsigned)persist[2]);
    return SD_G4_BROKEN;
  }
  {
    uint32_t bad = 0u;
    if (shipped[0] != 1u || shipped[1] != 0u || shipped[2] != 0u) {
      bad |= SD_G4_SHIPPED;
    }
    if (persist[0] != 1u || persist[1] != 1u || persist[2] != 1u) {
      bad |= SD_G4_PERSIST;
    }
    return bad;
  }
}

/* G4's case. The default is (yes, no, no): one strobe displays the frame it lands in and the frame start clears the
 * latch. The alternate is (yes, yes, yes): the latch persists until something else clears it. */
static int sd_g4_latch(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = sd_prog_g4(w);
  uint32_t shipped[3] = {0u, 0u, 0u};
  uint32_t persist[3] = {0u, 0u, 0u};
  uint32_t bad = sd_guard_g4(w, n, shipped, persist);
  if (bad != 0u) {
    ivs_test_diag("G4 guard failed: limbs 0x%02X (shipped %u %u %u, persist %u %u %u)", (unsigned)bad,
                  (unsigned)shipped[0], (unsigned)shipped[1], (unsigned)shipped[2], (unsigned)persist[0],
                  (unsigned)persist[1], (unsigned)persist[2]);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

static int sd_judge_g4_dead(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = sd_prog_g4_dead(w);
  uint32_t shipped[3] = {0u, 0u, 0u};
  uint32_t persist[3] = {0u, 0u, 0u};
  uint32_t bad = sd_guard_g4(w, n, shipped, persist);
  if (bad == 0u) {
    ivs_test_diag("judge-vacuous: the guard passed on the stand-in (shipped %u %u %u, persist %u %u %u)",
                  (unsigned)shipped[0], (unsigned)shipped[1], (unsigned)shipped[2], (unsigned)persist[0],
                  (unsigned)persist[1], (unsigned)persist[2]);
    return IVS_TEST_FAIL;
  }
  if ((bad & SD_G4_SHIPPED) == 0u || (bad & (SD_G4_PERSIST | SD_G4_BROKEN)) != 0u) {
    ivs_test_diag("judge-wrong-reason: limbs 0x%02X (shipped %u %u %u, persist %u %u %u)", (unsigned)bad,
                  (unsigned)shipped[0], (unsigned)shipped[1], (unsigned)shipped[2], (unsigned)persist[0],
                  (unsigned)persist[1], (unsigned)persist[2]);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

/* ---- the sixteen-cell sweep (plan 19-03, VID-07 criterion 2) ---- */

/* The four rival fields, one bit each. A cell is a four-bit number: bit 0 the register-window length, bit 1 the gating
 * rule, bit 2 the $0020 READ row's effect, bit 3 the latch sustain rule; a bit SET means "the alternate setting". */
#define SD_FIELD_WINDOW 0u
#define SD_FIELD_GATING 1u
#define SD_FIELD_READ 2u
#define SD_FIELD_LATCH 3u
#define SD_CELLS 16u

/* The index of the first event of `kind`, or p->event_count when the row carries none. */
static uint32_t sd_event_index(const ivs_stic_params *p, uint8_t kind)
{
  uint32_t i;
  for (i = 0u; i < p->event_count; i++) {
    if (p->event_kind[i] == kind) {
      return i;
    }
  }
  return p->event_count;
}

/* Builds cell `cell` as a STACK COPY of the shipped row and nothing else - the `stic.profile.window-close` mechanism,
 * generalised to four bits. It returns 0 when a row it must perturb has no such member, event or effect row, so a field
 * that stops being reachable is a red case rather than a silent no-op (research Pitfall 1: sweeping a field that
 * produces no parameter passes trivially). Nothing here writes to the profile index, to a global or to a const row: the
 * machine rebuilds its bus context from the profile index on every unit, so a cell that edited the profile would leave
 * every later cell comparing against its own predecessor and the matrix would be unreproducible. */
static int sd_cell(const ivs_stic_params *p, ivs_stic_params *out, uint32_t cell)
{
  uint32_t idx;
  uint32_t i;
  uint32_t found = 0u;
  *out = *p;
  if ((cell & (1u << SD_FIELD_WINDOW)) != 0u) {
    idx = sd_event_index(p, IVS_STIC_EV_REG_CLOSE);
    if (idx >= p->event_count) {
      return 0;
    }
    out->event_tick[idx] = (uint32_t)SD_WINDOW_WIDE_CYCLES * out->ticks_per_cpu_cycle;
    out->reg_window_cycles = SD_WINDOW_WIDE_CYCLES;
    out->reg_window_ticks = out->event_tick[idx];
  }
  if ((cell & (1u << SD_FIELD_GATING)) != 0u) {
    if (out->access_gating_when_off == 0u) {
      return 0; /* already at the alternate: the field is not reachable as a parameter */
    }
    out->access_gating_when_off = 0u;
  }
  if ((cell & (1u << SD_FIELD_READ)) != 0u) {
    for (i = 0u; i < out->effect_count; i++) {
      if ((uint32_t)out->effect_access[i] == IVS_STIC_ACCESS_READ && (uint32_t)out->effect_addr[i] == 0x0020u) {
        out->effect_code[i] = IVS_STIC_EFFECT_ARM;
        found = 1u;
      }
    }
    if (found == 0u) {
      return 0;
    }
  }
  if ((cell & (1u << SD_FIELD_LATCH)) != 0u) {
    if (out->display_latch_cleared_each_frame == 0u) {
      return 0;
    }
    out->display_latch_cleared_each_frame = 0u;
  }
  return 1;
}

/* How one guest's registered observable is read. G4's observable is a three-element vector and is encoded as
 * (v0 << 2) | (v1 << 1) | v2, so every guest's reading is one number and the matrix is one comparison. */
#define SD_OBS_BLOCKED 0u    /* ivs_stic_report.blocked_writes */
#define SD_OBS_DISPLAYED 1u  /* ivs_stic_report.windows_active, the frame-0 displayed word */
#define SD_OBS_DISPLAYED3 2u /* the displayed word at the first three graphics-memory closes */

/* One guest of the sweep: its own field, the program, the kind and count of its $0020 accesses, the horizon its
 * observation needs, and the two values 19-01's observable table recorded for it. `default_v` and `alternate_v` are the
 * values that guest's own guard already asserts on the shipped row and on its one-field reversal; the sweep does not
 * invent them, it requires every cell to agree with the rule "alternate if and only if this guest's own bit is set". */
typedef struct sd_guest {
  const char *name;
  uint32_t field;
  size_t (*prog)(uint16_t *w);
  uint8_t is_write;
  uint32_t frames;
  size_t accesses;
  uint8_t obs;
  uint32_t default_v;
  uint32_t alternate_v;
} sd_guest;

static const sd_guest sd_guests[] = {
    {"G1", SD_FIELD_WINDOW, sd_prog_g1_witness, 1u, 1u, 2u, SD_OBS_BLOCKED, 1u, 0u},
    {"G2", SD_FIELD_GATING, sd_prog_g2, 1u, 1u, 1u, SD_OBS_BLOCKED, 1u, 0u},
    {"G3", SD_FIELD_READ, sd_prog_g3, 0u, 1u, 1u, SD_OBS_DISPLAYED, 0u, 1u},
    {"G4", SD_FIELD_LATCH, sd_prog_g4, 1u, 3u, 1u, SD_OBS_DISPLAYED3, 4u, 7u},
};

/* Reads one guest's registered observable under the row q, encoded as one number. 0 on success. */
static int sd_observe_guest(const ivs_stic_params *q, const sd_guest *g, const sd_hits *h, uint32_t *v)
{
  ivs_stic_report r;
  uint32_t vec[3];
  switch (g->obs) {
    case SD_OBS_BLOCKED:
      if (!sd_observe(q, h, g->is_write != 0u ? 1u : 0u, 0x0020u, &r)) {
        return 0;
      }
      *v = r.blocked_writes;
      return 1;
    case SD_OBS_DISPLAYED:
      if (!sd_observe(q, h, g->is_write != 0u ? 1u : 0u, 0x0020u, &r)) {
        return 0;
      }
      *v = r.windows_active;
      return 1;
    default:
      if (!sd_g4_vector(q, h, vec)) {
        return 0;
      }
      *v = (vec[0] << 2) | (vec[1] << 1) | vec[2];
      return 1;
  }
}

/* Why the diagonal did not hold. FOREIGN is the one the stand-in judge must produce: a guest took its alternate value in
 * a cell that does not set its own field, so it moves under a field that is not its own and must be re-derived. */
#define SD_DIAG_OK 0u
#define SD_DIAG_FOREIGN 1u /* a guest moved under a field that is not its own */
#define SD_DIAG_MASKED 2u  /* a guest did not move under its own field */
#define SD_DIAG_NEITHER 3u /* a guest's observable is neither its default nor its alternate */
#define SD_DIAG_BROKEN 4u  /* a cell could not be built, or a guest did not make the accesses its shape implies */

typedef struct sd_diag {
  uint32_t reason;
  uint32_t cell;
  uint32_t bits;
  const char *guest;
  uint32_t observed;
  uint32_t expected;
} sd_diag;

/* The diagonal, as ONE comparison over sixteen cells: each guest's registered observable takes its alternate value if and
 * only if the cell sets that guest's own field, whatever the other three bits are. Comparing against a literal table of
 * sixteen expected vectors would be sixteen fits; this is one rule, so a guest that moves under two fields is caught by
 * the same sentence that proves the other three diagonal. `masks`, when not NULL, receives per guest the sixteen-bit mask
 * of the cells in which its alternate value was observed (the readable form of the matrix). 0 when the diagonal holds. */
static uint32_t sd_diagonal(const sd_guest *guests, size_t n_guests, uint16_t *masks, sd_diag *d)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  size_t g;
  uint32_t cell;
  memset(d, 0, sizeof *d);
  for (g = 0u; g < n_guests; g++) {
    const sd_guest *gs = &guests[g];
    uint16_t prog[SC_PROG_MAX];
    size_t n = gs->prog(prog);
    sd_hits h;
    if (masks != NULL) {
      masks[g] = 0u;
    }
    if (!sd_measure(prog, n, gs->is_write != 0u ? SD_WRITES : SD_READS, 0x0020u,
                    (uint64_t)gs->frames * (uint64_t)p->frame_ticks, &h) ||
        h.n != gs->accesses) {
      d->reason = SD_DIAG_BROKEN;
      d->guest = gs->name;
      return d->reason;
    }
    for (cell = 0u; cell < SD_CELLS; cell++) {
      ivs_stic_params q;
      uint32_t own = (cell >> gs->field) & 1u;
      uint32_t expect = own != 0u ? gs->alternate_v : gs->default_v;
      uint32_t v = 0u;
      if (!sd_cell(p, &q, cell)) {
        d->reason = SD_DIAG_BROKEN;
        d->cell = cell;
        d->bits = cell & 0x0Fu;
        d->guest = gs->name;
        return d->reason;
      }
      if (!sd_observe_guest(&q, gs, &h, &v)) {
        d->reason = SD_DIAG_BROKEN;
        d->cell = cell;
        d->bits = cell & 0x0Fu;
        d->guest = gs->name;
        return d->reason;
      }
      if (masks != NULL && v == gs->alternate_v) {
        masks[g] |= (uint16_t)(1u << cell);
      }
      if (v == expect) {
        continue;
      }
      d->cell = cell;
      d->bits = cell & 0x0Fu;
      d->guest = gs->name;
      d->observed = v;
      d->expected = expect;
      if (own == 0u && v == gs->alternate_v) {
        d->reason = SD_DIAG_FOREIGN;
      } else if (own != 0u && v == gs->default_v) {
        d->reason = SD_DIAG_MASKED;
      } else {
        d->reason = SD_DIAG_NEITHER;
      }
      return d->reason;
    }
  }
  return SD_DIAG_OK;
}

/* `stic.disc.sweep-diagonal`: one id, sixteen cells, one comparison. Each guest's decisive stamps are MEASURED from the
 * guest itself (the same `sd_measure` the guest cases use), then every cell re-drives that access sequence through the
 * hand-built bus context with `c.stic` set to the cell's stack-copied row, and the guest's registered observable is read.
 * The diagnostic on a break names the cell, the four bits, the guest and the observed value, so a break is one line. On
 * success one line per guest carries the sixteen-bit mask of the cells in which its alternate value was observed: for a
 * diagonal guest that mask is exactly the cells with its own bit set, which is the matrix in one number. */
static int sd_sweep_diagonal(void)
{
  sd_diag d;
  uint16_t mask[sizeof sd_guests / sizeof sd_guests[0]];
  size_t g;
  uint32_t reason = sd_diagonal(sd_guests, sizeof sd_guests / sizeof sd_guests[0], mask, &d);
  if (reason != SD_DIAG_OK) {
    ivs_test_diag("sweep-diagonal: reason %u at cell %u (bits 0x%X) guest %s observed %u expected %u",
                  (unsigned)reason, (unsigned)d.cell, (unsigned)d.bits, d.guest != NULL ? d.guest : "?",
                  (unsigned)d.observed, (unsigned)d.expected);
    return IVS_TEST_FAIL;
  }
  for (g = 0u; g < sizeof sd_guests / sizeof sd_guests[0]; g++) {
    ivs_test_diag("sweep-diagonal: %s field %u mask 0x%04X default %u alternate %u", sd_guests[g].name,
                  (unsigned)sd_guests[g].field, (unsigned)mask[g], (unsigned)sd_guests[g].default_v,
                  (unsigned)sd_guests[g].alternate_v);
  }
  return IVS_TEST_PASS;
}

/* `stic.judge.disc-diagonal-two-fields`: the committed stand-in that makes the diagonal falsifiable. It runs the SAME
 * comparison with G1's observable read the entangled way - the criterion's single-strobe guest, whose frame-0 displayed
 * word also moves when the gating field flips, which is the 19-01 measurement that made the blocked counter G1's
 * registered witness - and passes only when the comparison rejects it for that reason. `judge-vacuous` when the
 * comparison accepts a guest that moves under two fields; `judge-wrong-reason` when it fails for any other reason. */
static int sd_judge_diagonal_two_fields(void)
{
  static const sd_guest entangled = {"G1-criterion", SD_FIELD_WINDOW, sd_prog_g1_criterion, 1u, 1u, 1u,
                                     SD_OBS_DISPLAYED, 0u, 1u};
  sd_diag d;
  uint32_t reason = sd_diagonal(&entangled, 1u, NULL, &d);
  if (reason == SD_DIAG_OK) {
    ivs_test_diag("judge-vacuous: the diagonal accepted a guest that moves under two fields");
    return IVS_TEST_FAIL;
  }
  if (reason != SD_DIAG_FOREIGN) {
    ivs_test_diag("judge-wrong-reason: reason %u at cell %u (bits 0x%X) guest %s observed %u expected %u",
                  (unsigned)reason, (unsigned)d.cell, (unsigned)d.bits, d.guest != NULL ? d.guest : "?",
                  (unsigned)d.observed, (unsigned)d.expected);
    return IVS_TEST_FAIL;
  }
  ivs_test_diag("judge: the diagonal rejected the entangled guest at cell %u (bits 0x%X), observed %u",
                (unsigned)d.cell, (unsigned)d.bits, (unsigned)d.observed);
  return IVS_TEST_PASS;
}

/* ---- the family table ---- */

const ivs_test_case ivs_stic_disc_cases[] = {
    {"stic.disc.g1-window", sd_g1_window, NULL},
    {"stic.judge.disc-g1-dead", sd_judge_g1_dead, NULL},
    {"stic.disc.displayed-equivalence", sd_displayed_equivalence, NULL},
    {"stic.judge.disc-equivalence-dead", sd_judge_equivalence_dead, NULL},
    {"stic.disc.g2-gating", sd_g2_gating, NULL},
    {"stic.disc.g3-read", sd_g3_read, NULL},
    {"stic.disc.g4-latch", sd_g4_latch, NULL},
    {"stic.judge.disc-g2-dead", sd_judge_g2_dead, NULL},
    {"stic.judge.disc-g3-dead", sd_judge_g3_dead, NULL},
    {"stic.judge.disc-g4-dead", sd_judge_g4_dead, NULL},
    {"stic.disc.sweep-diagonal", sd_sweep_diagonal, NULL},
    {"stic.judge.disc-diagonal-two-fields", sd_judge_diagonal_two_fields, NULL},
};

const size_t ivs_stic_disc_count = sizeof ivs_stic_disc_cases / sizeof ivs_stic_disc_cases[0];
