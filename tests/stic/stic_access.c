/* SPDX-License-Identifier: MIT */
/* Suite stic, family access (plan 06-06; D-04, D-05, D-06, A-07): the register file, the write and read aliases, the access
 * windows, the blocked policy and its counters, the display-enable handshake and the mode select, seen from stand-in guests
 * (stic_progs.c; decles derived in stic_cases.h) and from synthesized accesses through the machine's bus wrapper.
 * Blocked-policy rows are POLICY (the register and graphics-memory windows are unknown-default-chosen, discrepancies C5 and
 * C94): they assert the outcome of an access after a window edge, never a number as its own expected value (D-02).
 * Expected values are literals of the independent register file (the register literal of data/stic: $0000 keeps 07FF, $0018 keeps
 * and reads 03FF with FC00 above, the strobes keep nothing) or relations to the parameters.
 *   stic.alias.*   the write aliases at $4000, $8000 and $C000 reach the register file; the read alias at $0040 mirrors it;
 *                  a read of $4021 (decoded to the cartridge in the read view) selects Color Stack when the register window
 *                  is open, counts as a blocked read when it is closed, and returns exactly what the read view returns
 *   stic.window.*  windows apply whether or not the display latch is set; the graphics-memory edge at the table tick
 *   stic.blocked.* a closed window drops a write and returns the undriven fill for a read, and counts each
 *   stic.enable.*  the display-enable strobe and its latch
 *   stic.mode.*    the Foreground/Background write and the Color Stack read */
#include <stdint.h>
#include <string.h>

#include "ivs/ivs.h"
#include "machine/machine.h"
#include "stic_cases.h"

#define SA_FILL 0xFFFFu /* memmap.undriven_read of the registry profile (the independent memmap literal's undriven rows) */

static struct ivs_machine *sa_im(ivs_ms_inst *x)
{
  return ivs_machine_from_handle_mut(x->m);
}

/* Builds the guest, runs it until ticks (machine time) and leaves the instance for the checks; 0 on failure. */
static int sa_run(ivs_ms_inst *x, const uint16_t *w, size_t n, uint32_t cart_addr, uint64_t ticks)
{
  if (!sc_guest(x, w, n, cart_addr)) {
    return 0;
  }
  if (!sc_run_until(x, ticks)) {
    ivs_ms_free(x);
    return 0;
  }
  return 1;
}

/* An access through the machine's bus wrapper at a chosen stamp, without a guest: the unit starts at unit_start and the
 * access is offset 0 of it, so the stamp is unit_start + the profile's access strobe ticks. */
static ivs_bus_reply sa_bus(ivs_ms_inst *x, uint64_t unit_start, uint8_t kind, uint16_t addr, uint16_t data)
{
  struct ivs_machine *im = sa_im(x);
  ivs_machine_bus_ctx c;
  ivs_cp1610_access a;
  memset(&c, 0, sizeof c);
  c.m = im;
  c.unit_start_tick = unit_start;
  c.ticks_per_cpu_cycle = ivs_profile_get(im->profile)->ticks_per_cpu_cycle;
  c.psg = ivs_psg_params_get(im->profile);
  c.stic = ivs_stic_params_get(im->profile);
  c.sink = NULL;
  memset(&a, 0, sizeof a);
  a.kind = kind;
  a.addr = addr;
  a.data = data;
  return ivs_machine_bus(&c, &a);
}

/* ---- aliases ---- */

/* The guest stores $355 (a 10-bit decle holds it) through a write alias; a register that keeps 07FF holds 0355 (the independent
 * register literal) and nothing is blocked inside the window. */
static int sa_alias_write(uint32_t addr, uint32_t reg)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = 0u;
  ivs_ms_inst x;
  int ok;
  n = sc_w_mvii(w, n, 0u, 0x0355u);
  n = sc_w_mvo(w, n, 0u, addr);
  n = sc_w_loop(w, n);
  SC_CHECK(sa_run(&x, w, n, 0u, 400u));
  ok = sa_im(&x)->mem.stic_reg[reg] == 0x0355u && sa_im(&x)->stic.blocked_writes == 0u;
  ivs_ms_free(&x);
  if (!ok) {
    ivs_test_diag("write of 0355 at %04X did not reach register %02X as 0355", (unsigned)addr, (unsigned)reg);
  }
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int sa_alias_w4000(void)
{
  return sa_alias_write(0x4000u, 0u);
}

static int sa_alias_w8000(void)
{
  return sa_alias_write(0x8001u, 1u); /* register $01, mob1-x, keeps 07FF */
}

static int sa_alias_wc000(void)
{
  return sa_alias_write(0xC002u, 2u);
}

/* The read alias mirrors the register file: a collision register written in the window reads back FC00 OR its stored bits
 * at $0058 as at $0018; a write-only register reads FFFF at $0040. */
static int sa_alias_read_0040(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = 0u;
  ivs_ms_inst x;
  int ok;
  n = sc_w_mvii(w, n, 0u, 0x00A5u);
  n = sc_w_mvo(w, n, 0u, 0x0018u); /* collision register 0: keeps 03FF */
  n = sc_w_mvi(w, n, 0x0058u, 1u);
  n = sc_w_mvo(w, n, 1u, 0x0200u);
  n = sc_w_mvi(w, n, 0x0040u, 2u);
  n = sc_w_mvo(w, n, 2u, 0x0201u);
  n = sc_w_loop(w, n);
  SC_CHECK(n <= SC_PROG_MAX);
  SC_CHECK(sa_run(&x, w, n, 0u, 600u));
  ok = sa_im(&x)->mem.sysram[0] == 0xFCA5u && sa_im(&x)->mem.sysram[1] == SA_FILL && sa_im(&x)->stic.mode == 0u;
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int sa_alias_read_4021_switches(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = 0u;
  ivs_ms_inst x;
  int ok;
  n = sc_w_mvi(w, n, 0x4021u, 1u);
  n = sc_w_mvo(w, n, 1u, 0x0200u);
  n = sc_w_loop(w, n);
  SC_CHECK(sa_run(&x, w, n, 0u, 400u));
  ok = sa_im(&x)->stic.mode == 1u && sa_im(&x)->stic.blocked_reads == 0u;
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* The CPU receives what the read view returns for $4021, never STIC data: the undriven fill without a cartridge word there
 * and the cartridge word with one (a segment at $4020-$402F). The wrapper never alters a reply. */
static int sa_alias_read_4021_value(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = 0u;
  ivs_ms_inst x;
  int ok;
  n = sc_w_mvi(w, n, 0x4021u, 1u);
  n = sc_w_mvo(w, n, 1u, 0x0200u);
  n = sc_w_loop(w, n);
  SC_CHECK(sa_run(&x, w, n, 0u, 400u));
  ok = sa_im(&x)->mem.sysram[0] == SA_FILL && sa_im(&x)->stic.mode == 1u;
  ivs_ms_free(&x);
  SC_CHECK(ok);
  SC_CHECK(sa_run(&x, w, n, 0x4020u, 400u));
  {
    const struct ivs_machine *im = sa_im(&x);
    uint16_t want = ivs_memsys_read(&im->mem, 0x4021u);
    ok = want != SA_FILL && im->mem.sysram[0] == want && im->stic.mode == 1u;
    if (!ok) {
      ivs_test_diag("the CPU received %04X, the read view gives %04X", (unsigned)im->mem.sysram[0], (unsigned)want);
    }
  }
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int sa_alias_read_4021_closed(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = 0u;
  ivs_ms_inst x;
  int ok;
  n = sc_w_delay(w, n, SC_DELAY_PAST_REG);
  n = sc_w_mvi(w, n, 0x4021u, 1u);
  n = sc_w_mvo(w, n, 1u, 0x0200u);
  n = sc_w_loop(w, n);
  SC_CHECK(sa_run(&x, w, n, 0u, 40000u));
  ok = sa_im(&x)->stic.mode == 0u && sa_im(&x)->stic.blocked_reads == 1u && sa_im(&x)->mem.sysram[0] == SA_FILL;
  if (!ok) {
    ivs_test_diag("mode %u blocked_reads %u", (unsigned)sa_im(&x)->stic.mode, (unsigned)sa_im(&x)->stic.blocked_reads);
  }
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* $4020 and $402F share the 16-word granule of $4021 but are not mode-select addresses: no effect and no count, in the
 * window and after it. */
static int sa_alias_read_neighbour(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = 0u;
  ivs_ms_inst x;
  int ok;
  n = sc_w_mvi(w, n, 0x4020u, 1u);
  n = sc_w_delay(w, n, SC_DELAY_PAST_REG);
  n = sc_w_mvi(w, n, 0x4020u, 1u);
  n = sc_w_mvi(w, n, 0x402Fu, 2u);
  n = sc_w_loop(w, n);
  SC_CHECK(n <= SC_PROG_MAX);
  SC_CHECK(sa_run(&x, w, n, 0u, 40000u));
  ok = sa_im(&x)->stic.mode == 0u && sa_im(&x)->stic.blocked_reads == 0u && sa_im(&x)->stic.blocked_writes == 0u;
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- windows and the blocked policy ---- */

/* The windows apply whether or not the display latch is set (D-05, stic.access_gating_when_off apply): the display is off
 * (no strobe) and a read after the register window is still blocked. */
static int sa_window_applies_when_off(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = 0u;
  ivs_ms_inst x;
  int ok;
  n = sc_w_delay(w, n, SC_DELAY_PAST_REG);
  n = sc_w_mvi(w, n, 0x0018u, 1u);
  n = sc_w_loop(w, n);
  SC_CHECK(sa_run(&x, w, n, 0u, 40000u));
  ok = sa_im(&x)->stic.display_latch == 0u && sa_im(&x)->stic.blocked_reads == 1u;
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* The graphics-memory edge: an access stamped one tick below the table tick is served, one stamped at it is blocked (the
 * tie rule A-06), for a GRAM write and a GROM read. */
static int sa_window_gmem_edges(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = 0u;
  ivs_ms_inst x;
  const ivs_stic_params *p;
  struct ivs_machine *im;
  uint32_t k;
  uint16_t want_grom;
  n = sc_w_loop(w, n);
  SC_CHECK(sc_guest(&x, w, n, 0u));
  im = sa_im(&x);
  p = ivs_stic_params_get(im->profile);
  want_grom = ivs_memsys_read(&im->mem, 0x3000u);
  for (k = 0u; k < 2u; k++) {
    uint64_t at = (uint64_t)p->gmem_window_ticks - (k == 0u ? 1u : 0u);
    uint64_t start = at - p->access_strobe_ticks;
    uint16_t got;
    SC_CHECK(ivs_machine_reset(x.m, IVS_RESET_POWER_ON) == IVS_OK);
    (void)sa_bus(&x, start, IVS_ACC_WRITE, 0x3801u, 0x00ABu);
    got = sa_bus(&x, start, IVS_ACC_READ, 0x3000u, 0u).data;
    if (k == 0u) {
      SC_CHECK(im->mem.gram[1] == 0x00ABu && got == want_grom && im->stic.blocked_writes == 0u && im->stic.blocked_reads == 0u);
    } else {
      SC_CHECK(im->mem.gram[1] == 0u && got == SA_FILL && im->stic.blocked_writes == 1u && im->stic.blocked_reads == 1u);
    }
  }
  ivs_ms_free(&x);
  return IVS_TEST_PASS;
}

/* A guest that stores a collision register in the window and then touches the closed regions after the edges. */
static int sa_blocked_reg_read(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = 0u;
  ivs_ms_inst x;
  int ok;
  n = sc_w_mvii(w, n, 0u, 0x00AAu);
  n = sc_w_mvo(w, n, 0u, 0x0018u);
  n = sc_w_delay(w, n, SC_DELAY_PAST_REG);
  n = sc_w_mvi(w, n, 0x0018u, 1u);
  n = sc_w_mvo(w, n, 1u, 0x0200u);
  n = sc_w_loop(w, n);
  SC_CHECK(n <= SC_PROG_MAX);
  SC_CHECK(sa_run(&x, w, n, 0u, 40000u));
  /* open it would read FCAA; closed the CPU receives the undriven fill, and the stored word is untouched */
  ok = sa_im(&x)->mem.sysram[0] == SA_FILL && sa_im(&x)->stic.blocked_reads == 1u && sa_im(&x)->mem.stic_reg[0x18] == 0x00AAu;
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int sa_blocked_reg_write(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = 0u;
  ivs_ms_inst x;
  int ok;
  n = sc_w_mvii(w, n, 0u, 0x0055u);
  n = sc_w_delay(w, n, SC_DELAY_PAST_REG);
  n = sc_w_mvo(w, n, 0u, 0x0018u);
  n = sc_w_loop(w, n);
  SC_CHECK(sa_run(&x, w, n, 0u, 40000u));
  ok = sa_im(&x)->mem.stic_reg[0x18] == 0u && sa_im(&x)->stic.blocked_writes == 1u && sa_im(&x)->stic.blocked_reads == 0u;
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* One guest, three closed graphics-memory accesses after both windows: a GRAM read, a GRAM write and a GROM read. */
static int sa_blocked_gmem(uint32_t which)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = 0u;
  ivs_ms_inst x;
  int ok = 0;
  n = sc_w_mvii(w, n, 0u, 0x0055u);
  n = sc_w_delay(w, n, SC_DELAY_PAST_GMEM);
  if (which == 0u) {
    n = sc_w_mvi(w, n, 0x3800u, 1u); /* GRAM read */
  } else if (which == 1u) {
    n = sc_w_mvo(w, n, 0u, 0x3800u); /* GRAM write */
  } else {
    n = sc_w_mvi(w, n, 0x3000u, 1u); /* GROM read */
  }
  n = sc_w_mvo(w, n, 1u, 0x0200u);
  n = sc_w_loop(w, n);
  SC_CHECK(n <= SC_PROG_MAX);
  SC_CHECK(sa_run(&x, w, n, 0u, 55000u));
  if (which == 1u) {
    ok = sa_im(&x)->mem.gram[0] == 0u && sa_im(&x)->stic.blocked_writes == 1u && sa_im(&x)->stic.blocked_reads == 0u;
  } else {
    ok = sa_im(&x)->mem.sysram[0] == SA_FILL && sa_im(&x)->stic.blocked_reads == 1u && sa_im(&x)->stic.blocked_writes == 0u;
  }
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int sa_blocked_gram_read(void)
{
  return sa_blocked_gmem(0u);
}

static int sa_blocked_gram_write(void)
{
  return sa_blocked_gmem(1u);
}

static int sa_blocked_grom_read(void)
{
  return sa_blocked_gmem(2u);
}

/* The counters count each refused access of their kind and saturate at 0xFFFFFFFF; GROM and GRAM count with the registers. */
static int sa_blocked_counters(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = 0u;
  ivs_ms_inst x;
  const ivs_stic_params *p;
  struct ivs_machine *im;
  uint64_t after;
  n = sc_w_loop(w, n);
  SC_CHECK(sc_guest(&x, w, n, 0u));
  im = sa_im(&x);
  p = ivs_stic_params_get(im->profile);
  after = (uint64_t)p->gmem_window_ticks + 100u; /* past both windows */
  (void)sa_bus(&x, after, IVS_ACC_READ, 0x0018u, 0u);
  (void)sa_bus(&x, after, IVS_ACC_READ, 0x3800u, 0u);
  (void)sa_bus(&x, after, IVS_ACC_WRITE, 0x0018u, 1u);
  (void)sa_bus(&x, after, IVS_ACC_WRITE, 0x3800u, 1u);
  (void)sa_bus(&x, after, IVS_ACC_WRITE, 0x4018u, 1u);
  SC_CHECK(im->stic.blocked_reads == 2u && im->stic.blocked_writes == 3u);
  im->stic.blocked_reads = 0xFFFFFFFEu;
  im->stic.blocked_writes = 0xFFFFFFFFu;
  (void)sa_bus(&x, after, IVS_ACC_READ, 0x0018u, 0u);
  (void)sa_bus(&x, after, IVS_ACC_READ, 0x0018u, 0u);
  (void)sa_bus(&x, after, IVS_ACC_WRITE, 0x0018u, 1u);
  SC_CHECK(im->stic.blocked_reads == 0xFFFFFFFFu && im->stic.blocked_writes == 0xFFFFFFFFu);
  ivs_ms_free(&x);
  return IVS_TEST_PASS;
}

/* ---- the display-enable strobe ---- */

/* One strobe guest: optional delay, then count strobes of $0020 (or the alias $4020), then a loop; runs to ticks. */
static int sa_strobe(uint32_t delay, uint32_t count, uint32_t addr, uint64_t ticks, uint32_t want_latch,
                     uint32_t want_blocked_writes)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = 0u;
  uint32_t i;
  ivs_ms_inst x;
  int ok;
  n = sc_w_mvii(w, n, 0u, 0x0000u);
  if (delay != 0u) {
    n = sc_w_delay(w, n, delay);
  }
  for (i = 0u; i < count; i++) {
    n = sc_w_mvo(w, n, 0u, addr);
  }
  n = sc_w_loop(w, n);
  SC_CHECK(n <= SC_PROG_MAX);
  SC_CHECK(sa_run(&x, w, n, 0u, ticks));
  ok = sa_im(&x)->stic.display_latch == want_latch && sa_im(&x)->stic.blocked_writes == want_blocked_writes &&
       sa_im(&x)->mem.stic_reg[addr & 0x3Fu] == 0u;
  if (!ok) {
    ivs_test_diag("latch %u blocked_writes %u", (unsigned)sa_im(&x)->stic.display_latch,
                  (unsigned)sa_im(&x)->stic.blocked_writes);
  }
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int sa_enable_in_window(void)
{
  return sa_strobe(0u, 1u, 0x0020u, 600u, 1u, 0u);
}

static int sa_enable_outside(void)
{
  return sa_strobe(SC_DELAY_PAST_REG, 1u, 0x0020u, 40000u, 0u, 1u);
}

static int sa_enable_second(void)
{
  return sa_strobe(0u, 2u, 0x0020u, 600u, 1u, 0u);
}

static int sa_enable_via_alias(void)
{
  return sa_strobe(0u, 1u, 0x4020u, 600u, 1u, 0u);
}

/* The latch is cleared at the next frame start: a strobe in frame 0 is gone once machine time passes the frame end. */
static int sa_enable_latch_cleared(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = 0u;
  ivs_ms_inst x;
  const ivs_stic_params *p;
  int ok;
  n = sc_w_mvii(w, n, 0u, 0x0000u);
  n = sc_w_mvo(w, n, 0u, 0x0020u);
  n = sc_w_loop(w, n);
  SC_CHECK(sc_guest(&x, w, n, 0u));
  p = ivs_stic_params_get(sa_im(&x)->profile);
  if (!sc_run_until(&x, 600u) || sa_im(&x)->stic.display_latch != 1u || !sc_run_until(&x, (uint64_t)p->frame_ticks + 100u)) {
    ivs_ms_free(&x);
    return IVS_TEST_FAIL;
  }
  ok = sa_im(&x)->stic.display_latch == 0u && sa_im(&x)->stic.frame_index == 1u;
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int sa_enable_reset_clears(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = 0u;
  ivs_ms_inst x;
  int ok;
  n = sc_w_mvii(w, n, 0u, 0x0000u);
  n = sc_w_mvo(w, n, 0u, 0x0020u);
  n = sc_w_loop(w, n);
  SC_CHECK(sa_run(&x, w, n, 0u, 600u));
  ok = sa_im(&x)->stic.display_latch == 1u && ivs_machine_reset(x.m, IVS_RESET_POWER_ON) == IVS_OK &&
       sa_im(&x)->stic.display_latch == 0u && sa_im(&x)->stic.blocked_writes == 0u;
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- the mode select ---- */

static int sa_mode_write_fgbg(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = 0u;
  ivs_ms_inst x;
  int ok;
  n = sc_w_mvii(w, n, 0u, 0x0000u);
  n = sc_w_mvi(w, n, 0x0021u, 1u); /* mode becomes Color Stack */
  n = sc_w_loop(w, n);
  SC_CHECK(sa_run(&x, w, n, 0u, 600u));
  ok = sa_im(&x)->stic.mode == 1u;
  ivs_ms_free(&x);
  SC_CHECK(ok);
  n = 0u;
  n = sc_w_mvii(w, n, 0u, 0x0000u);
  n = sc_w_mvi(w, n, 0x0021u, 1u);
  n = sc_w_mvo(w, n, 0u, 0x0021u); /* a write selects Foreground/Background again */
  n = sc_w_loop(w, n);
  SC_CHECK(sa_run(&x, w, n, 0u, 800u));
  ok = sa_im(&x)->stic.mode == 0u && sa_im(&x)->mem.stic_reg[0x21] == 0u;
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int sa_mode_read_cs(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = 0u;
  ivs_ms_inst x;
  int ok;
  n = sc_w_mvi(w, n, 0x0021u, 1u);
  n = sc_w_mvo(w, n, 1u, 0x0200u);
  n = sc_w_loop(w, n);
  SC_CHECK(sa_run(&x, w, n, 0u, 600u));
  /* a strobe register has read mask 0: the CPU receives the undriven fill, and the read still selects Color Stack */
  ok = sa_im(&x)->stic.mode == 1u && sa_im(&x)->mem.sysram[0] == SA_FILL && sa_im(&x)->stic.blocked_reads == 0u;
    ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int sa_mode_read_outside(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = 0u;
  ivs_ms_inst x;
  int ok;
  n = sc_w_delay(w, n, SC_DELAY_PAST_REG);
  n = sc_w_mvi(w, n, 0x0021u, 1u);
  n = sc_w_loop(w, n);
  SC_CHECK(sa_run(&x, w, n, 0u, 40000u));
  ok = sa_im(&x)->stic.mode == 0u && sa_im(&x)->stic.blocked_reads == 1u;
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_case ivs_stic_access_cases[] = {
    {"stic.alias.write-4000", sa_alias_w4000, NULL},
    {"stic.alias.write-8000", sa_alias_w8000, NULL},
    {"stic.alias.write-C000", sa_alias_wc000, NULL},
    {"stic.alias.read-0040", sa_alias_read_0040, NULL},
    {"stic.alias.read-4021-switches-mode", sa_alias_read_4021_switches, NULL},
    {"stic.alias.read-4021-returns-read-view-value", sa_alias_read_4021_value, NULL},
    {"stic.alias.read-4021-closed-gate-counts-blocked", sa_alias_read_4021_closed, NULL},
    {"stic.alias.read-non-member-granule-neighbour-no-effect", sa_alias_read_neighbour, NULL},
    {"stic.window.applies-when-display-off", sa_window_applies_when_off, NULL},
    {"stic.window.gmem-edges", sa_window_gmem_edges, NULL},
    {"stic.blocked.reg-read", sa_blocked_reg_read, NULL},
    {"stic.blocked.reg-write", sa_blocked_reg_write, NULL},
    {"stic.blocked.gram-read", sa_blocked_gram_read, NULL},
    {"stic.blocked.gram-write", sa_blocked_gram_write, NULL},
    {"stic.blocked.grom-read", sa_blocked_grom_read, NULL},
    {"stic.blocked.counters", sa_blocked_counters, NULL},
    {"stic.enable.seq.strobe-in-window", sa_enable_in_window, NULL},
    {"stic.enable.seq.strobe-outside-window-dropped", sa_enable_outside, NULL},
    {"stic.enable.seq.second-strobe", sa_enable_second, NULL},
    {"stic.enable.seq.latch-cleared-at-frame-start", sa_enable_latch_cleared, NULL},
    {"stic.enable.seq.reset-clears", sa_enable_reset_clears, NULL},
    {"stic.enable.seq.strobe-via-write-alias", sa_enable_via_alias, NULL},
    {"stic.mode.write-selects-fgbg", sa_mode_write_fgbg, NULL},
    {"stic.mode.read-selects-cs", sa_mode_read_cs, NULL},
    {"stic.mode.read-outside-window-no-effect", sa_mode_read_outside, NULL},
};
const size_t ivs_stic_access_count = sizeof ivs_stic_access_cases / sizeof ivs_stic_access_cases[0];
