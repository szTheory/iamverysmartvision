/* SPDX-License-Identifier: MIT */
/* Suite stic, stand-in guests (plan 06-06): the decle builders and the machine builder of the access families. See the
 * derivation of every decle in stic_cases.h. No rights-bearing byte: the EXEC image is the generated stand-in of
 * tests/machine/machine_standin.c with the guest words at $1000, the GROM and the cartridge are generated patterns. */
#include <stdint.h>
#include <string.h>

#include "ivs/ivs.h"
#include "machine/machine.h"
#include "stic_cases.h"

size_t sc_w_mvii(uint16_t *w, size_t n, uint32_t r, uint32_t imm)
{
  w[n++] = (uint16_t)(0x02B8u + r);
  w[n++] = (uint16_t)imm;
  return n;
}

/* A direct address is a 10-bit decle, so an address above $3FF goes through R3: SDBD; MVII #addr, R3 (the two data decles
 * are the low byte then the high byte, sdbd.json cpu.MVII.sdbd-low-high), then MVO@ Rs, R3 or MVI@ R3, Rd (1001 mmm sss
 * and 1010 mmm ddd with mmm = 3, a non-incrementing pointer; MVO@ R1, R2 is 0251 in indirect.json). The register r of
 * the access must not be R3. */
static size_t sc_w_addr(uint16_t *w, size_t n, uint32_t addr)
{
  w[n++] = 0x0001u;                   /* SDBD */
  w[n++] = 0x02BBu;                   /* MVII ..., R3 */
  w[n++] = (uint16_t)(addr & 0xFFu);  /* low byte */
  w[n++] = (uint16_t)((addr >> 8) & 0xFFu);
  return n;
}

size_t sc_w_mvo(uint16_t *w, size_t n, uint32_t r, uint32_t addr)
{
  if (addr > 0x03FFu) {
    n = sc_w_addr(w, n, addr);
    w[n++] = (uint16_t)(0x0240u + 3u * 8u + r); /* MVO@ Rr, R3 */
    return n;
  }
  w[n++] = (uint16_t)(0x0240u + r);
  w[n++] = (uint16_t)addr;
  return n;
}

size_t sc_w_mvi(uint16_t *w, size_t n, uint32_t addr, uint32_t r)
{
  if (addr > 0x03FFu) {
    n = sc_w_addr(w, n, addr);
    w[n++] = (uint16_t)(0x0280u + 3u * 8u + r); /* MVI@ R3, Rr */
    return n;
  }
  w[n++] = (uint16_t)(0x0280u + r);
  w[n++] = (uint16_t)addr;
  return n;
}

size_t sc_w_delay(uint16_t *w, size_t n, uint32_t count)
{
  n = sc_w_mvii(w, n, 1u, count); /* MVII #count, R1 */
  w[n++] = 0x0011u;               /* DECR R1 */
  w[n++] = 0x022Cu;               /* BNZE back to the DECR */
  w[n++] = 0x0002u;
  return n;
}

size_t sc_w_loop(uint16_t *w, size_t n)
{
  w[n++] = 0x0220u; /* B self */
  w[n++] = 0x0001u;
  return n;
}

static int sc_guest_any(ivs_ms_inst *x, const uint16_t *w, size_t n, uint32_t cart_addr, size_t limit)
{
  size_t cn = 0u;
  uint8_t *cart;
  uint8_t *exec;
  ivs_status st;
  if (n > limit || !ivs_ms_new(x, 0x5Au)) {
    return 0;
  }
  /* the cartridge and the EXEC image are built in their own statements: ivs_ms_cart sets cn (an out-parameter) and
   * an argument list that set and read it would have no defined order (gcc reads it first) */
  cart = ivs_ms_cart(cart_addr != 0u ? cart_addr : 0x5000u, 16u, 0u, &cn);
  exec = ivs_ms_exec_words(w, n);
  if (!ivs_ms_load(x->m, IVS_KIND_EXEC, exec, IVS_MS_EXEC_BYTES) ||
      !ivs_ms_load(x->m, IVS_KIND_GROM, ivs_ms_grom(0u), IVS_MS_GROM_BYTES) ||
      !ivs_ms_load(x->m, IVS_KIND_CARTRIDGE, cart, cn)) {
    ivs_ms_free(x);
    return 0;
  }
  st = ivs_machine_reset(x->m, IVS_RESET_POWER_ON);
  if (st != IVS_OK) {
    ivs_ms_free(x);
    return 0;
  }
  return 1;
}

int sc_guest(ivs_ms_inst *x, const uint16_t *w, size_t n, uint32_t cart_addr)
{
  return sc_guest_any(x, w, n, cart_addr, SC_PROG_MAX);
}

int sc_guest_n(ivs_ms_inst *x, const uint16_t *w, size_t n)
{
  return sc_guest_any(x, w, n, 0u, 4096u);
}

/* A branch back to word index start from the branch word at index n: backward, target = addr + 2 - d - 1, so d = n + 1 - start. */
static size_t sc_w_bback(uint16_t *w, size_t n, size_t start)
{
  w[n] = 0x0220u;
  w[n + 1u] = (uint16_t)(n + 1u - start);
  return n + 2u;
}

size_t sc_prog_vbl(uint16_t *w, uint32_t body_flags)
{
  uint32_t body = body_flags & 0xFFu;
  uint32_t backtab = body_flags & SC_BODY_BACKTAB;
  size_t n = 0u;
  size_t start;
  size_t k;
  w[n++] = 0x0200u; /* B forward to $100F, or $1011 when the ISR also stores to BACKTAB */
  w[n++] = backtab != 0u ? 0x000Fu : 0x000Du;
  w[n++] = 0x0000u;
  w[n++] = 0x0000u;
  w[n++] = 0x0003u; /* $1004 DIS */
  n = sc_w_mvo(w, n, 0u, 0x0020u);
  w[n++] = 0x000Au; /* INCR R2 */
  if (backtab != 0u) {
    n = sc_w_mvo(w, n, 2u, 0x0200u); /* MVO R2, $0200: the counter into the first BACKTAB word (a store to background memory) */
  }
  n = sc_w_mvii(w, n, 1u, SC_ISR_DELAY);
  w[n++] = 0x0011u; /* DECR R1 */
  w[n++] = 0x022Cu; /* BNZE back */
  w[n++] = 0x0002u;
  w[n++] = 0x0002u; /* EIS */
  w[n++] = 0x02B7u; /* PULR R7 */
  n = sc_w_mvii(w, n, 6u, 0x0300u);
  w[n++] = 0x0002u; /* EIS */
  start = n;
  switch (body) {
  case SC_BODY_INCR:
    for (k = 0u; k < 3u; k++) {
      w[n++] = 0x000Du;
    }
    return sc_w_bback(w, n, start);
  case SC_BODY_TSTR6:
    for (k = 0u; k < 3u; k++) {
      w[n++] = 0x00B6u;
    }
    return sc_w_bback(w, n, start);
  case SC_BODY_MVII:
    n = sc_w_mvii(w, n, 5u, 0u);
    n = sc_w_mvii(w, n, 5u, 0u);
    return sc_w_bback(w, n, start);
  case SC_BODY_MVI:
    w[n++] = 0x0285u;
    w[n++] = 0x0100u;
    return sc_w_bback(w, n, start);
  case SC_BODY_J:
    w[n++] = 0x0004u;
    w[n++] = (uint16_t)(0x0300u | ((((0x1000u + (uint32_t)start) >> 10) & 0x3Fu) << 2));
    w[n++] = (uint16_t)((0x1000u + (uint32_t)start) & 0x3FFu);
    return n;
  default: /* SC_BODY_BSELF */
    return sc_w_bback(w, n, start);
  }
}

/* An interrupt program for the C41 consequences: B $1006 or $1007 over the ISR at $1004 (INCR R2; PULR R7, or DIS first), then
 * MVII #$300, R6; EIS; B self. The ISR never re-enables, so with INTFF held (C41 = 0) a service routine without DIS is re-entered. */
size_t sc_prog_isr(uint16_t *w, uint32_t dis_first)
{
  size_t n = 0u;
  w[n++] = 0x0200u;
  w[n++] = dis_first != 0u ? 0x0005u : 0x0004u; /* target $1002 + d: the main part right after the ISR */
  w[n++] = 0x0000u;
  w[n++] = 0x0000u;
  if (dis_first != 0u) {
    w[n++] = 0x0003u; /* DIS */
  }
  w[n++] = 0x000Au; /* INCR R2 */
  w[n++] = 0x02B7u; /* PULR R7 */
  n = sc_w_mvii(w, n, 6u, 0x0300u);
  w[n++] = 0x0002u; /* EIS */
  return sc_w_bback(w, n, n);
}

/* MVO R0, $0020 (the strobe once) then B self (4 words): a constant-cost loop in a display-on frame 0. */
size_t sc_prog_strobe_loop(uint16_t *w)
{
  size_t n = sc_w_mvo(w, 0u, 0u, 0x0020u);
  return sc_w_bback(w, n, n);
}

/* The interrupt program of sc_prog_vbl up to the main part: words 0..14 (the forward branch, the pad and the ISR), returns 15. */
static size_t sc_vbl_prefix(uint16_t *w)
{
  size_t n = 0u;
  w[n++] = 0x0200u;
  w[n++] = 0x000Du;
  w[n++] = 0x0000u;
  w[n++] = 0x0000u;
  w[n++] = 0x0003u; /* DIS */
  n = sc_w_mvo(w, n, 0u, 0x0020u);
  w[n++] = 0x000Au; /* INCR R2 */
  n = sc_w_mvii(w, n, 1u, SC_ISR_DELAY);
  w[n++] = 0x0011u;
  w[n++] = 0x022Cu;
  w[n++] = 0x0002u;
  w[n++] = 0x0002u; /* EIS */
  w[n++] = 0x02B7u; /* PULR R7 */
  return n;
}

/* The adversarial PSG writer: the VBL interrupt program whose main loop is `pad` x INCR R5 and k x MVO@ R0, R3 with R3 = $01F8 (a PSG
 * register: 0258 is MVO@ R0, R3, 9 cycles, not interruptible) and a branch back, so PSG writes meet the bus windows and the VBLANK
 * entries at every phase. */
size_t sc_prog_psg(uint16_t *w, uint32_t k, uint32_t pad)
{
  size_t n = sc_vbl_prefix(w);
  size_t start;
  uint32_t i;
  n = sc_w_mvii(w, n, 6u, 0x0300u);
  n = sc_w_mvii(w, n, 3u, 0x01F8u);
  w[n++] = 0x0002u; /* EIS */
  start = n;
  for (i = 0u; i < pad; i++) {
    w[n++] = 0x000Du;
  }
  for (i = 0u; i < k; i++) {
    w[n++] = 0x0258u;
  }
  return sc_w_bback(w, n, start);
}

/* The densest reachable PSG writer: [the display strobe], R3 = $01F8, count x MVO@ R0, R3 back to back, then B self. */
size_t sc_prog_psg_stream(uint16_t *w, size_t count, uint32_t strobe)
{
  size_t n = 0u;
  size_t i;
  if (strobe != 0u) {
    n = sc_w_mvo(w, n, 0u, 0x0020u);
  }
  n = sc_w_mvii(w, n, 3u, 0x01F8u);
  for (i = 0u; i < count; i++) {
    w[n++] = 0x0258u;
  }
  return sc_w_bback(w, n, n);
}

size_t sc_prog_loop(uint16_t *w, uint32_t enable)
{
  w[0] = enable != 0u ? 0x0002u : 0x0003u;
  return sc_w_bback(w, 1u, 1u);
}

size_t sc_prog_halt(uint16_t *w)
{
  size_t n = sc_w_mvo(w, 0u, 0u, 0x0020u);
  w[n++] = 0x0000u; /* HLT */
  w[n++] = 0x0034u; /* NOP */
  return sc_w_bback(w, n, n);
}

size_t sc_prog_stream(uint16_t *w, size_t count)
{
  size_t n = 0u;
  size_t k;
  w[n++] = 0x0002u; /* EIS */
  for (k = 0u; k < count; k++) {
    n = sc_w_mvo(w, n, 0u, 0x0020u);
  }
  return sc_w_bback(w, n, n);
}

size_t sc_trace(ivs_ms_inst *x, uint64_t until, sc_unit *u, size_t cap)
{
  const struct ivs_machine *im = ivs_machine_from_handle(x->m);
  size_t n = 0u;
  while (im->time_ticks < until) {
    ivs_cp1610_result r;
    uint64_t start = im->time_ticks;
    if (n >= cap || ivs_machine_step_unit_internal(x->m, &r) != IVS_OK) {
      return (size_t)-1;
    }
    u[n].start = start;
    u[n].cycles = r.elapsed_cpu_cycles;
    u[n].kind = r.unit_kind;
    u[n].events = r.events;
    u[n].opcode = r.opcode;
    n++;
  }
  return n;
}

int sc_run_until(ivs_ms_inst *x, uint64_t ticks)
{
  const struct ivs_machine *im = ivs_machine_from_handle(x->m);
  uint32_t n = 0u;
  while (im->time_ticks < ticks) {
    if (n++ >= 40000u || !ivs_ms_step(x->m, 1u, NULL)) {
      return 0;
    }
  }
  return 1;
}
