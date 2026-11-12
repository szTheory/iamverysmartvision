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

int sc_guest(ivs_ms_inst *x, const uint16_t *w, size_t n, uint32_t cart_addr)
{
  size_t cn = 0u;
  uint8_t *cart;
  uint8_t *exec;
  ivs_status st;
  if (n > SC_PROG_MAX || !ivs_ms_new(x, 0x5Au)) {
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
