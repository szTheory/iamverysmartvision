/* SPDX-License-Identifier: MIT */
/* Suite input, family slice (API-03, plan 05-05, D-05, C67): the guest sees input at unit starts, and
 * slicing and interleaving change nothing the guest can see.
 *
 * C67 resolver (the sentence also in docs/runner.md): an event takes effect at the first CPU-unit
 * boundary whose start tick is at or after its at_tick, so it is visible to every guest read of the
 * unit that begins at or after that tick; a read inside the unit that began earlier sees the old state.
 * Hardware may differ by up to one instruction: this is a contract with a profile-flag reversal path,
 * not a hardware claim (unknown-default-chosen).
 *   input.boundary.start-minus-1 | start | start-plus-1
 *                                the start tick s of the unit that performs the first poll read of $01FE
 *                                is OBSERVED by single-stepping (run(now + 1)), never predicted from the
 *                                cycle table; an event at s - 1 and at s is seen by that read, an event at
 *                                s + 1 only by the next poll
 *   input.slice.guest-visible    the same schedule in one call and in many slices (fixed, prime, one unit)
 *                                gives equal guest RAM, equal digests and byte-equal instance storage,
 *                                for a constant-cycle poller and a data-dependent poller
 *   input.iso.interleaved-instances  two instances with different schedules, advanced alternately call by
 *                                call, equal their sequential runs byte for byte
 * Stand-in programs of tests/machine/machine_standin.c (no rights-bearing byte). */
#include <stdint.h>
#include <string.h>

#include "input_cases.h"

#define IL_POLL_PC 0x1002u /* the MVI $01FE of both poller programs */
#define IL_LOG 0x0300u

/* Words from the pinned control macros (no table). */
#define IL_W_A (IVS_CTRL_KEY_1)
#define IL_W_B (IVS_CTRL_KEY_5 | IVS_CTRL_DISC_N)
#define IL_W_C (IVS_CTRL_SIDE_TOP)

static uint32_t il_pc(const ivs_machine *m)
{
  ivs_cpu_state_report c;
  memset(&c, 0, sizeof c);
  c.struct_size = (uint32_t)sizeof c;
  if (ivs_machine_cpu_state(m, &c) != IVS_OK) {
    return 0xFFFFFFFFu;
  }
  return c.r[7];
}

/* ---- the C67 boundary ---- */

/* The start tick of the unit that performs the first poll read (the first unit whose instruction
 * address is $1002), found by single-stepping a rig with no event; 0 when not found in 16 units. */
static uint64_t il_observe_poll_start(uint32_t program)
{
  ii_rig r;
  uint64_t s = 0u;
  uint32_t k;
  if (!ii_rig_open(&r, program)) {
    return 0u;
  }
  for (k = 0u; k < 16u; k++) {
    ivs_run_report rep;
    if (il_pc(r.x.m) == IL_POLL_PC) {
      s = r.now;
      break;
    }
    if (ii_run(r.x.m, r.now + 1u, NULL, 0u, &rep) != IVS_OK || rep.units_executed != 1u) {
      break;
    }
    r.now = rep.now_ticks;
  }
  ii_rig_close(&r);
  return s;
}

/* An event of KEY_1 on port 0 at tick at, then enough ticks for two polls: the first poll's byte is
 * log word 0 and the second poll's byte log word 2 of the constant-cycle poller. */
static int il_boundary(int64_t delta, uint32_t first, uint32_t second)
{
  uint64_t s = il_observe_poll_start(IVS_MS_PROG_POLL_CONST);
  ivs_ms_inst x;
  ivs_input_event ev;
  ivs_run_report rep;
  uint32_t got0;
  uint32_t got1;
  int rc = IVS_TEST_FAIL;
  if (s < 2u) {
    ivs_test_diag("no poll unit observed (start %llu)", (unsigned long long)s);
    return IVS_TEST_FAIL;
  }
  if (!ivs_ms_powered_prog(&x, IVS_MS_PROG_POLL_CONST, II_POISON)) {
    return IVS_TEST_FAIL;
  }
  ii_event(&ev, (uint64_t)((int64_t)s + delta), IL_W_A, 0u);
  II_CHECK(ii_run(x.m, 1000u, &ev, 1u, &rep) == IVS_OK && rep.input_consumed == 1u);
  got0 = ii_peek(x.m, IL_LOG);
  got1 = ii_peek(x.m, IL_LOG + 2u);
  if (got0 != first || got1 != second) {
    ivs_test_diag("event at s%+lld (s = %llu): first poll %04X second poll %04X, want %04X %04X", (long long)delta,
                  (unsigned long long)s, (unsigned)got0, (unsigned)got1, (unsigned)first, (unsigned)second);
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* KEY_1 reads $7E (the wiki anchor, written here); an idle port reads $FF. */
static int il_start_minus_1(void)
{
  return il_boundary(-1, 0x7Eu, 0x7Eu);
}

static int il_start(void)
{
  return il_boundary(0, 0x7Eu, 0x7Eu);
}

static int il_start_plus_1(void)
{
  return il_boundary(1, 0xFFu, 0x7Eu);
}

/* ---- slicing and interleaving ---- */

#define IL_EVENTS 8u
#define IL_TARGET 1500u

static void il_schedule(ivs_input_event *ev, uint32_t variant)
{
  static const uint32_t words[2][IL_EVENTS][2] = {
      {{IL_W_A, 0u}, {0u, 0u}, {IL_W_B, IL_W_C}, {IL_W_B, 0u}, {0u, IL_W_A}, {IL_W_C, IL_W_C}, {0u, 0u}, {IL_W_A, IL_W_B}},
      {{IL_W_C, IL_W_A}, {IL_W_C, 0u}, {0u, 0u}, {IL_W_A, IL_W_B}, {IL_W_B, IL_W_B}, {0u, IL_W_C}, {IL_W_A, 0u}, {0u, 0u}}};
  uint32_t i;
  for (i = 0u; i < IL_EVENTS; i++) {
    /* ticks: some on a pollers' unit start, some inside a unit, two equal */
    uint64_t at = 25u + 190u * i + (i == 3u ? 1u : 0u) + (variant == 1u ? 13u : 0u);
    if (i == 6u) {
      at = 25u + 190u * 5u + (variant == 1u ? 13u : 0u); /* equal to event 5 */
    }
    ii_event(&ev[i], at, words[variant][i][0], words[variant][i][1]);
  }
}

/* Runs m to target in calls of slice ticks (0: one call), resubmitting the unconsumed suffix; 1 when every
 * call was accepted and all events were consumed or left pending beyond the end. */
static int il_run_sliced(ivs_machine *m, const ivs_input_event *ev, uint32_t n, uint64_t target, uint64_t slice,
                         uint32_t *consumed)
{
  uint32_t next = 0u;
  uint64_t now = 0u;
  uint32_t calls = 0u;
  while (now < target && calls < 100000u) {
    ivs_run_report rep;
    uint64_t t = slice == 0u ? target : (now + slice < target ? now + slice : target);
    if (ii_run(m, t, ev + next, n - next, &rep) != IVS_OK) {
      return 0;
    }
    next += rep.input_consumed;
    if (rep.now_ticks <= now && now < target) {
      return 0; /* no progress */
    }
    now = rep.now_ticks;
    calls++;
  }
  *consumed = next;
  return 1;
}

/* One program, one schedule: the one-call run is the reference; the slicings equal it. */
static int il_slice_program(uint32_t program, uint32_t variant)
{
  static const uint64_t slices[4] = {0u, 7u, 13u, 1u}; /* one call, fixed 7, prime 13, one tick (one unit) per call */
  static uint8_t ref_storage[IVS_MACHINE_BYTES];
  ivs_input_event ev[IL_EVENTS];
  uint8_t ref_dig[4][32];
  uint32_t ref_consumed = 0u;
  uint32_t k;
  static const uint32_t doms[4] = {IVS_DIGEST_CPU, IVS_DIGEST_MEM, IVS_DIGEST_COMPOSITE, IVS_DIGEST_INPUT};
  int rc = IVS_TEST_FAIL;
  il_schedule(ev, variant);
  for (k = 0u; k < 4u; k++) {
    ivs_ms_inst x;
    uint8_t d[32];
    uint32_t consumed = 0u;
    uint32_t j;
    uint16_t log[8];
    if (!ivs_ms_powered_prog(&x, program, II_POISON)) {
      return IVS_TEST_FAIL;
    }
    if (!il_run_sliced(x.m, ev, IL_EVENTS, IL_TARGET, slices[k], &consumed)) {
      ivs_test_diag("slice %llu: a call was refused or made no progress", (unsigned long long)slices[k]);
      ivs_ms_free(&x);
      return IVS_TEST_FAIL;
    }
    if (k == 0u) {
      memcpy(ref_storage, x.storage, sizeof ref_storage);
      ref_consumed = consumed;
      for (j = 0u; j < 4u; j++) {
        if (ivs_machine_state_digest(x.m, doms[j], ref_dig[j]) != IVS_OK) {
          ivs_ms_free(&x);
          return IVS_TEST_FAIL;
        }
      }
      /* the guest did see the events: the log holds a byte that is not idle */
      if (ivs_machine_peek(x.m, IL_LOG, 8u, log) != IVS_OK) {
        ivs_ms_free(&x);
        return IVS_TEST_FAIL;
      }
      {
        int seen = 0;
        for (j = 0u; j < 8u; j++) {
          seen |= log[j] != 0x00FFu && log[j] != 0x0000u && log[j] != 0x0300u;
        }
        if (!seen && program == IVS_MS_PROG_POLL_CONST) {
          ivs_test_diag("the one-call run logged only idle bytes: the schedule never reached a poll");
          ivs_ms_free(&x);
          return IVS_TEST_FAIL;
        }
      }
    } else {
      int ok = consumed == ref_consumed && memcmp(x.storage, ref_storage, sizeof ref_storage) == 0;
      for (j = 0u; ok && j < 4u; j++) {
        ok = ivs_machine_state_digest(x.m, doms[j], d) == IVS_OK && memcmp(d, ref_dig[j], 32u) == 0;
      }
      if (!ok) {
        ivs_test_diag("program %u variant %u: slicing %llu differs from the one-call run", (unsigned)program,
                      (unsigned)variant, (unsigned long long)slices[k]);
        ivs_ms_free(&x);
        return IVS_TEST_FAIL;
      }
    }
    ivs_ms_free(&x);
  }
  rc = IVS_TEST_PASS;
  return rc;
}

static int il_slice_guest_visible(void)
{
  return il_slice_program(IVS_MS_PROG_POLL_CONST, 0u) == IVS_TEST_PASS &&
                 il_slice_program(IVS_MS_PROG_POLL_BRANCH, 0u) == IVS_TEST_PASS &&
                 il_slice_program(IVS_MS_PROG_POLL_BRANCH, 1u) == IVS_TEST_PASS
             ? IVS_TEST_PASS
             : IVS_TEST_FAIL;
}

/* Two instances with different schedules, one call each in turn, equal their own sequential runs. */
static int il_iso_interleaved(void)
{
  static uint8_t seq[2][IVS_MACHINE_BYTES];
  ivs_input_event ev[2][IL_EVENTS];
  ivs_ms_inst x[2];
  uint32_t next[2] = {0u, 0u};
  uint64_t now[2] = {0u, 0u};
  uint32_t v;
  uint32_t calls;
  int open_ok[2] = {0, 0};
  int rc = IVS_TEST_FAIL;
  for (v = 0u; v < 2u; v++) {
    uint32_t consumed = 0u;
    ivs_ms_inst y;
    il_schedule(ev[v], v);
    if (!ivs_ms_powered_prog(&y, IVS_MS_PROG_POLL_BRANCH, II_POISON)) {
      return IVS_TEST_FAIL;
    }
    if (!il_run_sliced(y.m, ev[v], IL_EVENTS, IL_TARGET, 0u, &consumed)) {
      ivs_ms_free(&y);
      return IVS_TEST_FAIL;
    }
    memcpy(seq[v], y.storage, sizeof seq[v]);
    ivs_ms_free(&y);
  }
  for (v = 0u; v < 2u; v++) {
    open_ok[v] = ivs_ms_powered_prog(&x[v], IVS_MS_PROG_POLL_BRANCH, II_POISON);
  }
  II_CHECK(open_ok[0] && open_ok[1]);
  for (calls = 0u; calls < 100000u && (now[0] < IL_TARGET || now[1] < IL_TARGET); calls++) {
    v = calls % 2u;
    if (now[v] < IL_TARGET) {
      ivs_run_report rep;
      uint64_t t = now[v] + 5u + 4u * (uint64_t)(calls % 7u);
      II_CHECK(ii_run(x[v].m, t < IL_TARGET ? t : IL_TARGET, ev[v] + next[v], IL_EVENTS - next[v], &rep) == IVS_OK);
      next[v] += rep.input_consumed;
      now[v] = rep.now_ticks;
    }
  }
  II_CHECK(now[0] >= IL_TARGET && now[1] >= IL_TARGET);
  II_CHECK(memcmp(x[0].storage, seq[0], IVS_MACHINE_BYTES) == 0);
  II_CHECK(memcmp(x[1].storage, seq[1], IVS_MACHINE_BYTES) == 0);
  II_CHECK(memcmp(x[0].storage, x[1].storage, IVS_MACHINE_BYTES) != 0); /* the two schedules are different */
  rc = IVS_TEST_PASS;
done:
  for (v = 0u; v < 2u; v++) {
    if (open_ok[v]) {
      ivs_ms_free(&x[v]);
    }
  }
  return rc;
}

static const ivs_test_case il_cases[] = {
    {"input.boundary.start-minus-1", il_start_minus_1, NULL},
    {"input.boundary.start", il_start, NULL},
    {"input.boundary.start-plus-1", il_start_plus_1, NULL},
    {"input.slice.guest-visible", il_slice_guest_visible, NULL},
    {"input.iso.interleaved-instances", il_iso_interleaved, NULL},
};

II_TABLE_FAMILY(slice, il_cases);
