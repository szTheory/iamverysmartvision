/* SPDX-License-Identifier: MIT */
/* Suite stic, family peek (plan 06-07; D-04, A-07, VID-06, T-06-09): ivs_machine_peek keeps its Phase 4 contract (what a CPU read
 * returns now, gates honoured, no log, no time, no state) because every read side effect of the STIC lives in the bus wrapper, which
 * the const peek path never reaches. The roadmap example is a named test: a peek of $0021 inside the register window leaves the mode
 * unchanged. Stand-in guests are in stic_progs.c (decles derived in stic_cases.h); the snapshot oracle (storage, nine digests and
 * machine time) is ss_snap in stic_state.c.
 *   stic.peek.mode-unchanged-0021   a peek of $0021 and of its three read aliases inside the window leaves the mode as it was
 *   stic.peek.sweep-bytes-equal-after-render  (plan 06-11) the same sweep with a delivered Color Stack frame present
 *   stic.peek.sweep-bytes-equal     whole-instance and digest equality around peeking every STIC-decoded address, the alias
 *                                   windows and the glyph range, in five states
 *   stic.peek.cpu-read-switches     the POSITIVE CONTROL: a CPU read of $0021 inside the window does switch to Color Stack and
 *                                   one outside the window does not, so the sweep above cannot pass because nothing ever switches
 *   stic.peek.value-equals-cpu-value  the peek value equals the value a CPU read returned, for a collision register, a GRAM word
 *                                   and a blocked address (values only) */
#include <stdint.h>
#include <string.h>

#include "ivs/ivs.h"
#include "machine/machine.h"
#include "stic_cases.h"

#define SK_FILL 0xFFFFu /* memmap.undriven_read of the registry profile (the independent memmap literal's undriven rows) */

#define SK_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      goto done;                                                                                       \
    }                                                                                                  \
  } while (0)

static const struct ivs_machine *sk_im(const ivs_ms_inst *x)
{
  return ivs_machine_from_handle(x->m);
}

/* The report of x, or zero on a refusal (the caller checks the fields it needs). */
static int sk_report(const ivs_ms_inst *x, ivs_stic_report *r)
{
  memset(r, 0, sizeof *r);
  r->struct_size = IVS_STIC_REPORT_V1_SIZE;
  return ivs_machine_stic_state(x->m, r) == IVS_OK;
}

/* The five states of the sweep. Each builds a guest, runs it and checks that the state really holds (the precondition of the
 * sweep) through the public report. */
#define SK_STATES 5u
static const char *const sk_state_name[SK_STATES] = {"display off", "display on in the window", "display on outside the window",
                                                     "Color Stack", "Foreground/Background"};

static int sk_build(ivs_ms_inst *x, uint32_t which)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = 0u;
  ivs_stic_report r;
  uint64_t t;
  int ok = 0;
  switch (which) {
  case 0u: /* the display stays off: no strobe */
    n = sc_w_loop(w, n);
    break;
  case 1u: /* strobe the display enable and stay in the window */
  case 2u: /* strobe, then run past the register window */
    n = sc_w_mvii(w, n, 0u, 0u);
    n = sc_w_mvo(w, n, 0u, 0x0020u);
    if (which == 2u) {
      n = sc_w_delay(w, n, SC_DELAY_PAST_REG);
    }
    n = sc_w_loop(w, n);
    break;
  case 3u: /* a CPU read of $0021 inside the window selects Color Stack */
    n = sc_w_mvi(w, n, 0x0021u, 1u);
    n = sc_w_loop(w, n);
    break;
  default: /* the read, then a write of $0021 selects Foreground/Background again */
    n = sc_w_mvi(w, n, 0x0021u, 1u);
    n = sc_w_mvo(w, n, 0u, 0x0021u);
    n = sc_w_loop(w, n);
    break;
  }
  if (n > SC_PROG_MAX || !sc_guest(x, w, n, 0u)) {
    return 0;
  }
  if (which == 4u) {
    /* step until the read has switched to Color Stack, so the later Foreground/Background write is a real change */
    for (t = 20u; t <= 600u && sk_im(x)->stic.mode == 0u; t += 20u) {
      if (!sc_run_until(x, t)) {
        goto done;
      }
    }
    if (sk_im(x)->stic.mode != 1u) {
      ivs_test_diag("state %s: the read of $0021 never selected Color Stack", sk_state_name[which]);
      goto done;
    }
  }
  if (!sc_run_until(x, which == 2u ? 40000u : 600u) || !sk_report(x, &r)) {
    goto done;
  }
  switch (which) {
  case 0u:
    ok = r.display_latch == 0u && r.regs_window_open == 1u && r.mode == 0u;
    break;
  case 1u:
    ok = r.display_latch == 1u && r.regs_window_open == 1u && r.mode == 0u;
    break;
  case 2u:
    ok = r.display_latch == 1u && r.regs_window_open == 0u && r.mode == 0u;
    break;
  case 3u:
    ok = r.mode == 1u && r.regs_window_open == 1u;
    break;
  default:
    ok = r.mode == 0u && r.regs_window_open == 1u;
    break;
  }
  if (!ok) {
    ivs_test_diag("state %s does not hold: mode %u latch %u regs window %u", sk_state_name[which], (unsigned)r.mode,
                  (unsigned)r.display_latch, (unsigned)r.regs_window_open);
  }
done:
  if (!ok) {
    ivs_ms_free(x);
  }
  return ok;
}

/* ---- the roadmap example ---- */

static int sk_mode_unchanged_0021(void)
{
  static const uint16_t addrs[4] = {0x0021u, 0x4021u, 0x8021u, 0xC021u};
  ivs_ms_inst x;
  ivs_stic_report r;
  uint32_t which;
  uint32_t k;
  int rc = IVS_TEST_FAIL;
  x.storage = NULL;
  for (which = 0u; which < SK_STATES; which++) {
    uint32_t before;
    uint16_t v = 0u;
    if (!sk_build(&x, which)) {
      return IVS_TEST_FAIL;
    }
    SK_CHECK(sk_report(&x, &r));
    before = r.mode;
    for (k = 0u; k < 4u; k++) {
      SK_CHECK(ivs_machine_peek(x.m, addrs[k], 1u, &v) == IVS_OK);
      SK_CHECK(sk_report(&x, &r));
      if (r.mode != before) {
        ivs_test_diag("state %s: a peek of %04X changed the mode from %u to %u", sk_state_name[which], (unsigned)addrs[k],
                      (unsigned)before, (unsigned)r.mode);
        goto done;
      }
    }
    ivs_ms_free(&x);
  }
  rc = IVS_TEST_PASS;
done:
  if (rc != IVS_TEST_PASS && x.storage != NULL) {
    ivs_ms_free(&x);
  }
  return rc;
}

/* ---- the whole-instance sweep ---- */

static uint16_t sk_buf[0x1000];

typedef struct sk_range {
  uint16_t lo;
  uint16_t count;
  uint16_t one_by_one; /* 1: peek each address on its own, else one bulk peek */
} sk_range;

static const sk_range sk_ranges[5] = {
    {0x0000u, 0x0080u, 1u}, /* the STIC-decoded addresses: the register file, its read alias and the first words past it */
    {0x4000u, 0x0040u, 1u}, /* the alias windows */
    {0x8000u, 0x0040u, 1u},
    {0xC000u, 0x0040u, 1u},
    {0x3000u, 0x1000u, 0u}, /* the glyph range: GROM and GRAM */
};

/* Peeks every address of the sweep ranges (the STIC-decoded addresses, the alias windows and the glyph range) of the running instance x and checks
 * after each range that the whole instance, the digests and the machine time are unchanged against the snapshot s taken before. 1 when all hold. */
static int sk_sweep_instance(ivs_ms_inst *x, const ss_snap *s, const char *name)
{
  uint32_t k;
  uint32_t i;
  for (k = 0u; k < 5u; k++) {
    const sk_range *g = &sk_ranges[k];
    if (g->one_by_one != 0u) {
      for (i = 0u; i < g->count; i++) {
        if (ivs_machine_peek(x->m, (uint16_t)(g->lo + i), 1u, sk_buf) != IVS_OK) {
          return 0;
        }
      }
    } else if (ivs_machine_peek(x->m, g->lo, g->count, sk_buf) != IVS_OK) {
      return 0;
    }
    if (!ss_snap_same(s, x)) {
      ivs_test_diag("state %s: peeking %04X..%04X changed the machine", name, (unsigned)g->lo, (unsigned)(g->lo + g->count - 1u));
      return 0;
    }
  }
  return 1;
}

static int sk_sweep_bytes_equal(void)
{
  ivs_ms_inst x;
  ss_snap s;
  uint32_t which;
  int rc = IVS_TEST_FAIL;
  s.copy = NULL;
  x.storage = NULL;
  for (which = 0u; which < SK_STATES; which++) {
    if (!sk_build(&x, which)) {
      return IVS_TEST_FAIL;
    }
    SK_CHECK(ss_snap_take(&s, &x));
    SK_CHECK(sk_sweep_instance(&x, &s, sk_state_name[which]));
    ss_snap_free(&s);
    ivs_ms_free(&x);
  }
  rc = IVS_TEST_PASS;
done:
  ss_snap_free(&s);
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  return rc;
}

/* Plan 06-11 (VID-06): the same sweep with a DELIVERED Color Stack frame present. A guest reads $0021 (Color Stack), strobes the display and loops; the
 * BACKTAB holds three advance cards and the four stack registers are set, and the machine is run past the first frame event (59,736 ticks), so a
 * frame has been rendered (the chain count is 1, the report's pointer is 3, the row latches hold the BACKTAB). The sweep of every STIC-decoded, alias
 * and glyph address must leave the whole instance (storage, so the row latches and the chain, the digests and the time) byte-identical. */
static int sk_sweep_after_render(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = sc_w_mvi(w, 0u, 0x0021u, 1u);
  ivs_ms_inst x;
  ss_snap s;
  ivs_stic_report r;
  struct ivs_machine *im;
  uint32_t k;
  int rc = IVS_TEST_FAIL;
  s.copy = NULL;
  x.storage = NULL;
  n = sc_w_mvo(w, n, 0u, 0x0020u);
  n = sc_w_loop(w, n);
  SK_CHECK(sc_guest(&x, w, n, 0u));
  im = ivs_machine_from_handle_mut(x.m);
  ivs_memsys_poke_raw_internal(&im->mem, 0x0200u + 21u, 0x2000u);
  ivs_memsys_poke_raw_internal(&im->mem, 0x0200u + 105u, 0x2000u);
  ivs_memsys_poke_raw_internal(&im->mem, 0x0200u + 189u, 0x2000u);
  for (k = 0u; k < 4u; k++) {
    im->mem.stic_reg[0x28u + k] = (uint16_t)(3u * k + 1u);
  }
  SK_CHECK(sc_run_until(&x, 60000u));
  SK_CHECK(sk_report(&x, &r));
  SK_CHECK(r.mode == 1u && r.cs_ptr == 3u && sk_im(&x)->obs.video.count == 1u); /* a Color Stack frame was rendered and delivered */
  SK_CHECK(sk_im(&x)->stic.row_latch[1][1] == 0x2000u);
  SK_CHECK(ss_snap_take(&s, &x));
  SK_CHECK(sk_sweep_instance(&x, &s, "Color Stack with a rendered frame"));
  rc = IVS_TEST_PASS;
done:
  ss_snap_free(&s);
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  return rc;
}

/* ---- the positive control ---- */

/* A guest that reads $0021 into R1 after delay iterations of a wait (0 for none) and stops; the machine is run to ticks. */
static int sk_read_0021(ivs_ms_inst *x, uint32_t delay, uint64_t ticks)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = 0u;
  if (delay != 0u) {
    n = sc_w_delay(w, n, delay);
  }
  n = sc_w_mvi(w, n, 0x0021u, 1u);
  n = sc_w_loop(w, n);
  if (n > SC_PROG_MAX || !sc_guest(x, w, n, 0u)) {
    return 0;
  }
  if (!sc_run_until(x, ticks)) {
    ivs_ms_free(x);
    return 0;
  }
  return 1;
}

static int sk_cpu_read_switches(void)
{
  ivs_ms_inst x;
  ivs_stic_report r;
  ss_snap s;
  uint16_t v = 0u;
  int rc = IVS_TEST_FAIL;
  s.copy = NULL;
  x.storage = NULL;
  /* inside the register window the CPU read selects Color Stack ... */
  SK_CHECK(sk_read_0021(&x, 0u, 600u));
  SK_CHECK(sk_report(&x, &r));
  SK_CHECK(r.regs_window_open == 1u && r.mode == 1u && r.blocked_reads == 0u);
  ivs_ms_free(&x);
  /* ... and the same read after the window has closed does not (it is a blocked read), and a peek of the same address does
   * not count either: the sweep's equality has something to be equal to */
  SK_CHECK(sk_read_0021(&x, SC_DELAY_PAST_REG, 40000u));
  SK_CHECK(sk_report(&x, &r));
  SK_CHECK(r.regs_window_open == 0u && r.mode == 0u && r.blocked_reads == 1u);
  SK_CHECK(ss_snap_take(&s, &x));
  SK_CHECK(ivs_machine_peek(x.m, 0x0021u, 1u, &v) == IVS_OK);
  SK_CHECK(ss_snap_same(&s, &x));
  SK_CHECK(sk_report(&x, &r));
  SK_CHECK(r.mode == 0u && r.blocked_reads == 1u);
  rc = IVS_TEST_PASS;
done:
  ss_snap_free(&s);
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  return rc;
}

/* ---- peek value equals the CPU value ---- */

/* Inside both windows: a collision register stored by the guest and a GRAM word, each read back by the CPU into system RAM. 24 words. */
static int sk_value_open(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = 0u;
  ivs_ms_inst x;
  ivs_stic_report r;
  uint16_t pk[2];
  int rc = IVS_TEST_FAIL;
  x.storage = NULL;
  n = sc_w_mvii(w, n, 0u, 0x00AAu);
  n = sc_w_mvo(w, n, 0u, 0x0018u);
  n = sc_w_mvi(w, n, 0x0018u, 1u);
  n = sc_w_mvo(w, n, 1u, 0x0200u);
  n = sc_w_mvii(w, n, 0u, 0x00ABu);
  n = sc_w_mvo(w, n, 0u, 0x3801u);
  n = sc_w_mvi(w, n, 0x3801u, 2u);
  n = sc_w_mvo(w, n, 2u, 0x0201u);
  n = sc_w_loop(w, n);
  SK_CHECK(n <= SC_PROG_MAX);
  SK_CHECK(sc_guest(&x, w, n, 0u));
  SK_CHECK(sc_run_until(&x, 1500u));
  SK_CHECK(sk_report(&x, &r) && r.regs_window_open == 1u && r.gmem_window_open == 1u && r.blocked_reads == 0u);
  SK_CHECK(ivs_machine_peek(x.m, 0x0018u, 1u, &pk[0]) == IVS_OK);
  SK_CHECK(ivs_machine_peek(x.m, 0x3801u, 1u, &pk[1]) == IVS_OK);
  if (sk_im(&x)->mem.sysram[0] != pk[0] || sk_im(&x)->mem.sysram[1] != pk[1]) {
    ivs_test_diag("the CPU read %04X and %04X, the peek returns %04X and %04X", (unsigned)sk_im(&x)->mem.sysram[0],
                  (unsigned)sk_im(&x)->mem.sysram[1], (unsigned)pk[0], (unsigned)pk[1]);
    goto done;
  }
  SK_CHECK(pk[0] == 0xFCAAu && pk[0] != SK_FILL && pk[1] != SK_FILL); /* the collision register reads FC00 or its stored bits */
  rc = IVS_TEST_PASS;
done:
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  return rc;
}

/* Past both windows: the blocked read of the register and of a GRAM word return the undriven fill to the CPU and to a peek. */
static int sk_value_blocked(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = 0u;
  ivs_ms_inst x;
  ivs_stic_report r;
  uint16_t pk[2];
  int rc = IVS_TEST_FAIL;
  x.storage = NULL;
  n = sc_w_delay(w, n, SC_DELAY_PAST_GMEM);
  n = sc_w_mvi(w, n, 0x0018u, 1u);
  n = sc_w_mvo(w, n, 1u, 0x0200u);
  n = sc_w_mvi(w, n, 0x3801u, 2u);
  n = sc_w_mvo(w, n, 2u, 0x0201u);
  n = sc_w_loop(w, n);
  SK_CHECK(n <= SC_PROG_MAX);
  SK_CHECK(sc_guest(&x, w, n, 0u));
  SK_CHECK(sc_run_until(&x, 55000u));
  SK_CHECK(sk_report(&x, &r) && r.regs_window_open == 0u && r.gmem_window_open == 0u && r.blocked_reads == 2u);
  SK_CHECK(ivs_machine_peek(x.m, 0x0018u, 1u, &pk[0]) == IVS_OK);
  SK_CHECK(ivs_machine_peek(x.m, 0x3801u, 1u, &pk[1]) == IVS_OK);
  SK_CHECK(sk_im(&x)->mem.sysram[0] == pk[0] && sk_im(&x)->mem.sysram[1] == pk[1]);
  SK_CHECK(pk[0] == SK_FILL && pk[1] == SK_FILL);
  rc = IVS_TEST_PASS;
done:
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  return rc;
}

static int sk_value_equals_cpu_value(void)
{
  int a = sk_value_open();
  int b = sk_value_blocked();
  return (a == IVS_TEST_PASS && b == IVS_TEST_PASS) ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_case ivs_stic_peek_cases[] = {
    {"stic.peek.mode-unchanged-0021", sk_mode_unchanged_0021, NULL},
    {"stic.peek.sweep-bytes-equal", sk_sweep_bytes_equal, NULL},
    {"stic.peek.sweep-bytes-equal-after-render", sk_sweep_after_render, NULL},
    {"stic.peek.cpu-read-switches", sk_cpu_read_switches, NULL},
    {"stic.peek.value-equals-cpu-value", sk_value_equals_cpu_value, NULL},
};
const size_t ivs_stic_peek_count = sizeof ivs_stic_peek_cases / sizeof ivs_stic_peek_cases[0];
