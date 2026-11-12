/* SPDX-License-Identifier: MIT */
/* Suite stic, family profile (plan 06-06; D-02, D-05): the windows are profile data. The machine's bus wrapper runs with a
 * hand-built context holding a STACK COPY of the generated parameters, so a perturbed copy moves the observed edge and nothing
 * else: moving the table tick of an event by one CPU cycle moves the first blocked (or first served) access stamp by exactly
 * that cycle. No policy number is an expected value; the edge is found by bisection over the access stamp, each probe on a
 * fresh power-on state.
 *   stic.profile.window-close   the first blocked register access of frame 0 moves with the REG_CLOSE tick
 *   stic.profile.window-open    the first served register access after the close moves with the FRAME_END tick (the next frame
 *                               start reopens the window) */
#include <stdint.h>
#include <string.h>

#include "ivs/ivs.h"
#include "machine/machine.h"
#include "stic_cases.h"

static ivs_ms_inst sp_x;
static int sp_have;

static int sp_machine(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = sc_w_loop(w, 0u);
  if (!sp_have) {
    if (!sc_guest(&sp_x, w, n, 0u)) {
      return 0;
    }
    sp_have = 1;
  }
  return 1;
}

static void sp_free(void)
{
  if (sp_have) {
    ivs_ms_free(&sp_x);
    sp_have = 0;
  }
}

/* 1 when a READ of the collision register $0018 stamped at stamp is blocked (counted) under the parameters q. */
static int sp_blocked_at(const ivs_stic_params *q, uint64_t stamp)
{
  struct ivs_machine *im = ivs_machine_from_handle_mut(sp_x.m);
  ivs_machine_bus_ctx c;
  ivs_cp1610_access a;
  (void)ivs_machine_reset(sp_x.m, IVS_RESET_POWER_ON);
  memset(&c, 0, sizeof c);
  c.m = im;
  c.unit_start_tick = stamp - q->access_strobe_ticks;
  c.ticks_per_cpu_cycle = q->ticks_per_cpu_cycle;
  c.psg = ivs_psg_params_get(im->profile);
  c.stic = q;
  c.sink = NULL;
  memset(&a, 0, sizeof a);
  a.kind = IVS_ACC_READ;
  a.addr = 0x0018u;
  (void)ivs_machine_bus(&c, &a);
  return im->stic.blocked_reads == 1u;
}

/* The first blocked stamp in [0, hi): blocked accesses stay blocked up to the frame end. hi when none is. */
static uint64_t sp_first_blocked(const ivs_stic_params *q, uint64_t hi)
{
  uint64_t lo = 0u;
  while (lo < hi) {
    uint64_t mid = lo + (hi - lo) / 2u;
    if (sp_blocked_at(q, mid)) {
      hi = mid;
    } else {
      lo = mid + 1u;
    }
  }
  return lo;
}

/* The first served stamp in [from, hi): served accesses stay served up to the next close. hi when none is. */
static uint64_t sp_first_served(const ivs_stic_params *q, uint64_t from, uint64_t hi)
{
  uint64_t lo = from;
  while (lo < hi) {
    uint64_t mid = lo + (hi - lo) / 2u;
    if (!sp_blocked_at(q, mid)) {
      hi = mid;
    } else {
      lo = mid + 1u;
    }
  }
  return lo;
}

static uint32_t sp_index(const ivs_stic_params *p, uint32_t kind)
{
  uint32_t i;
  for (i = 0u; i < p->event_count; i++) {
    if (p->event_kind[i] == kind) {
      return i;
    }
  }
  return p->event_count;
}

uint64_t sp_register_edge(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint64_t edge = 0u;
  if (sp_machine()) {
    edge = sp_first_blocked(p, p->frame_ticks);
  }
  return edge;
}

int sp_edge_matches(uint64_t expected)
{
  return sp_register_edge() == expected;
}

static int sp_window_close(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  ivs_stic_params q;
  uint32_t idx = sp_index(p, IVS_STIC_EV_REG_CLOSE);
  uint64_t edge;
  uint64_t moved;
  int ok = 0;
  if (!sp_machine() || idx >= p->event_count) {
    sp_free();
    return IVS_TEST_FAIL;
  }
  edge = sp_first_blocked(p, p->frame_ticks);
  q = *p;
  q.event_tick[idx] += q.ticks_per_cpu_cycle;
  q.reg_window_ticks += q.ticks_per_cpu_cycle;
  moved = sp_first_blocked(&q, q.frame_ticks);
  ok = edge == p->event_tick[idx] && moved == edge + q.ticks_per_cpu_cycle;
  q = *p;
  q.event_tick[idx] -= q.ticks_per_cpu_cycle;
  moved = sp_first_blocked(&q, q.frame_ticks);
  ok = ok && moved + q.ticks_per_cpu_cycle == edge;
  if (!ok) {
    ivs_test_diag("first blocked stamp %llu, REG_CLOSE tick %llu", (unsigned long long)edge,
                  (unsigned long long)p->event_tick[idx]);
  }
  sp_free();
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int sp_window_open(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  ivs_stic_params q;
  uint64_t close;
  uint64_t open;
  uint64_t moved;
  int ok = 0;
  if (!sp_machine()) {
    sp_free();
    return IVS_TEST_FAIL;
  }
  close = sp_first_blocked(p, p->frame_ticks);
  open = sp_first_served(p, close, (uint64_t)p->frame_ticks + close);
  q = *p;
  q.frame_ticks += q.ticks_per_cpu_cycle;
  q.event_tick[q.event_count - 1u] += q.ticks_per_cpu_cycle; /* FRAME_END */
  moved = sp_first_served(&q, close, (uint64_t)q.frame_ticks + close);
  ok = open == p->frame_ticks && moved == open + q.ticks_per_cpu_cycle;
  if (!ok) {
    ivs_test_diag("first served stamp after the close %llu, frame ticks %llu", (unsigned long long)open,
                  (unsigned long long)p->frame_ticks);
  }
  sp_free();
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_case ivs_stic_profile_cases[] = {
    {"stic.profile.window-close", sp_window_close, NULL},
    {"stic.profile.window-open", sp_window_open, NULL},
};
const size_t ivs_stic_profile_count = sizeof ivs_stic_profile_cases / sizeof ivs_stic_profile_cases[0];
