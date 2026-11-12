/* SPDX-License-Identifier: MIT */
/* Suite stic, family judge (plan 06-06; V6 judge self-controls): each case corrupts a copy of its oracle input and requires
 * the checking function to FAIL on it (and, as a positive control, to pass on the unchanged input), so a checker that
 * accepts anything is caught.
 *   stic.judge.register-row-changed  the literal-table check on a copy with one write mask flipped
 *   stic.judge.window-edge-moved     the edge check against an expected edge moved by one tick
 *   stic.judge.blocked-write-applied the blocked-write check on a stand-in that applies a write the window blocked
 *   stic.judge.late-grant-latched    the late-grant check on a stand-in that latches a late grant (counts it as granted, or carries it over) */
#include <stdint.h>
#include <string.h>

#include "ivs/ivs.h"
#include "machine/machine.h"
#include "stic_cases.h"

static int sj_register_row_changed(void)
{
  SC_CHECK(sr_table_diff_copy(5u, 0x0000u) == 0u); /* the unchanged copy matches */
  SC_CHECK(sr_table_diff_copy(5u, 0x0001u) == 6u); /* one flipped bit of one write mask is found, at its row */
  SC_CHECK(sr_table_diff_copy(0x18u, 0x8000u) == 0x19u);
  return IVS_TEST_PASS;
}

static int sj_window_edge_moved(void)
{
  uint64_t edge = sp_register_edge();
  SC_CHECK(edge != 0u);
  SC_CHECK(sp_edge_matches(edge) == 1);
  SC_CHECK(sp_edge_matches(edge + 1u) == 0);
  SC_CHECK(sp_edge_matches(edge - 1u) == 0);
  return IVS_TEST_PASS;
}

/* The checker: the registers and graphics RAM equal their state before the access and exactly one blocked write was counted. */
static int sj_blocked_write_ok(const struct ivs_machine *before, const struct ivs_machine *after)
{
  return memcmp(before->mem.stic_reg, after->mem.stic_reg, sizeof before->mem.stic_reg) == 0 &&
         memcmp(before->mem.gram, after->mem.gram, sizeof before->mem.gram) == 0 &&
         after->stic.blocked_writes == before->stic.blocked_writes + 1u;
}

static int sj_blocked_write_applied(void)
{
  static struct ivs_machine before_copy;
  static struct ivs_machine after_copy;
  uint16_t w[SC_PROG_MAX];
  size_t n = sc_w_loop(w, 0u);
  ivs_ms_inst x;
  struct ivs_machine *im;
  ivs_machine_bus_ctx c;
  ivs_cp1610_access a;
  int ok = 0;
  SC_CHECK(sc_guest(&x, w, n, 0u));
  im = ivs_machine_from_handle_mut(x.m);
  memcpy(&before_copy, im, sizeof before_copy);
  memset(&c, 0, sizeof c);
  c.m = im;
  c.unit_start_tick = (uint64_t)ivs_stic_params_get(im->profile)->gmem_window_ticks + 100u; /* past both windows */
  c.ticks_per_cpu_cycle = ivs_profile_get(im->profile)->ticks_per_cpu_cycle;
  c.psg = ivs_psg_params_get(im->profile);
  c.stic = ivs_stic_params_get(im->profile);
  memset(&a, 0, sizeof a);
  a.kind = IVS_ACC_WRITE;
  a.addr = 0x0018u;
  a.data = 0x0055u;
  (void)ivs_machine_bus(&c, &a);
  memcpy(&after_copy, im, sizeof after_copy);
  /* the real wrapper: the write was blocked, the checker accepts */
  if (sj_blocked_write_ok(&before_copy, &after_copy) == 1) {
    /* a stand-in that applies the blocked write: the checker rejects */
    after_copy.mem.stic_reg[0x18] = 0x0055u;
    if (sj_blocked_write_ok(&before_copy, &after_copy) == 0) {
      /* and one that forgets to count it */
      memcpy(&after_copy, im, sizeof after_copy);
      after_copy.stic.blocked_writes = before_copy.stic.blocked_writes;
      ok = sj_blocked_write_ok(&before_copy, &after_copy) == 0;
    }
  }
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int sj_late_grant_latched(void)
{
  uint32_t n = ivs_stic_params_get(SC_PROFILE)->bus_count;
  SC_CHECK(sb_late_counter_ok(n, 0u, n) == 1);      /* the expected observation is accepted */
  SC_CHECK(sb_late_counter_ok(0u, 0u, n) == 0);     /* a late grant counted as granted: nothing counted late */
  SC_CHECK(sb_late_counter_ok(n - 1u, 0u, n) == 0); /* one window latched and fetched late */
  SC_CHECK(sb_late_counter_ok(n, 1u, n) == 0);      /* a late grant carried into an interruptible frame */
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_stic_judge_cases[] = {
    {"stic.judge.register-row-changed", sj_register_row_changed, NULL},
    {"stic.judge.window-edge-moved", sj_window_edge_moved, NULL},
    {"stic.judge.blocked-write-applied", sj_blocked_write_applied, NULL},
    {"stic.judge.late-grant-latched", sj_late_grant_latched, NULL},
};
const size_t ivs_stic_judge_count = sizeof ivs_stic_judge_cases / sizeof ivs_stic_judge_cases[0];
