/* SPDX-License-Identifier: MIT */
/* Suite input, family port (INP-01, INP-03, plan 05-05, D-04): the PSG ports $01FE and $01FF as the
 * guest sees them, and the machine state that carries the pins.
 *   input.port.direction-input-reads-pins   input mode (R7 bits 6 and 7 clear): the read follows the pins
 *   input.port.direction-output-reads-latch output mode: the read returns the written latch, the pins are
 *                                           ignored (policy POL-PSG-07, C65, tag unknown-default-chosen)
 *   input.port.per-port-direction           bit 6 selects port 0 and bit 7 port 1, 1 = output (policy
 *                                           POL-PSG-09, C73), independently
 *   input.port.r7-other-bits-ignored        the other bits of R7 (the mixer enables) change no port read
 *   input.port.upper-byte                   the upper byte of a port read is $00 in both modes (C64)
 *   input.port.write-latch-in-input-mode    a write in input mode stores the latch and leaves the read on
 *                                           the pins; the latch shows when the port turns to output
 *   input.port.peek-equals-cpu-read         ivs_machine_peek equals a CPU read at every PSG address
 *   input.port.unplugged-is-idle            a word of an unconnected controller is refused and no port
 *                                           changes; ports read idle
 *   input.port.reads-are-pure               peeks and CPU reads leave the whole instance storage unchanged
 *   input.port.independence                 changing one port's word never changes the other's read
 *   input.digest.mem-covers-pins            two instances that differ only in a controller word differ in
 *                                           the MEM and composite digests (the pull-down masks are state)
 *   input.takeback.restores-pins            a refused step takes the applied event back, pins included
 *   input.reset.clears-pulldown             a power-on reset clears the pull-down masks
 *   input.invariant.pulldown-derives-from-words  port_pulldown equals the OR of the literal's masks of the
 *                                           words in force, after every call
 *   input.derive.pure-equals-rederive       (plan 08-10, D-15) ivs_input_derive, the pure form of the derivation (no instance
 *                                           argument), equals the OR of the literal's masks per port, equals the masks an
 *                                           instance holds after ivs_input_rederive, and an out-of-range profile index gives
 *                                           zeros: over the zero word, every single control, every pair of controls (465), the
 *                                           word with every control closed, the reserved-bit words and a pseudo-random sample
 *   input.judge.direction-sense-flipped     the direction predicate rejects the opposite R7 sense
 * Expected bytes come from the controller literal records and the wiki anchors, never from the
 * profile or the core's table. The latch and direction rules are policy rows (tags above), not hardware
 * evidence. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "input_cases.h"

#define IP_R7 0x01F8u
#define IP_P0 0x01FEu
#define IP_P1 0x01FFu
/* The literal's controls used by name: indices are bits of data/expected/input-bits.ids. */
#define IP_KEY_1 0u
#define IP_KEY_2 1u
#define IP_KEY_5 4u
#define IP_KEY_ENTER 11u

static uint32_t ip_byte(uint32_t bit)
{
  return ivs_ip_ctls[bit].byte;
}

static uint32_t ip_word(uint32_t bit)
{
  return 1u << bit;
}

/* The pins mask the literal gives for a word: OR of the cleared masks of every set bit. */
static uint32_t ip_derive(uint32_t word)
{
  uint32_t mask = 0u;
  size_t k;
  for (k = 0u; k < ivs_ip_count; k++) {
    if ((word & (1u << ivs_ip_ctls[k].bit)) != 0u) {
      mask |= ivs_ip_ctls[k].cleared;
    }
  }
  return mask;
}

/* ---- direction and latch ---- */

static int ip_direction_input_reads_pins(void)
{
  ii_rig r;
  int rc = IVS_TEST_FAIL;
  if (!ii_rig_open(&r, IVS_MS_PROG_A)) {
    return IVS_TEST_FAIL;
  }
  II_CHECK(ii_rig_set(&r, ip_word(IP_KEY_1), ip_word(IP_KEY_ENTER)));
  II_CHECK(ii_bus_read(&r, IP_P0) == ip_byte(IP_KEY_1) && ii_bus_read(&r, IP_P1) == ip_byte(IP_KEY_ENTER));
  ii_bus_write(&r, IP_R7, 0x0000u); /* both ports input, written explicitly */
  II_CHECK(ii_bus_read(&r, IP_P0) == ip_byte(IP_KEY_1) && ii_bus_read(&r, IP_P1) == ip_byte(IP_KEY_ENTER));
  rc = IVS_TEST_PASS;
done:
  ii_rig_close(&r);
  return rc;
}

static int ip_direction_output_reads_latch(void)
{
  ii_rig r;
  int rc = IVS_TEST_FAIL;
  if (!ii_rig_open(&r, IVS_MS_PROG_A)) {
    return IVS_TEST_FAIL;
  }
  II_CHECK(ii_rig_set(&r, ip_word(IP_KEY_1), ip_word(IP_KEY_ENTER)));
  /* port 0 as output: the latch read back, from power-on 0 and then the written value */
  ii_bus_write(&r, IP_R7, 0x0040u);
  II_CHECK(ii_bus_read(&r, IP_P0) == 0x0000u);
  ii_bus_write(&r, IP_P0, 0x00A5u);
  II_CHECK(ii_bus_read(&r, IP_P0) == 0x00A5u);
  II_CHECK(ii_rig_set(&r, ip_word(IP_KEY_2), ip_word(IP_KEY_ENTER))); /* the pins change, the read does not */
  II_CHECK(ii_bus_read(&r, IP_P0) == 0x00A5u);
  /* port 1 as output */
  ii_bus_write(&r, IP_R7, 0x0080u);
  ii_bus_write(&r, IP_P1, 0x005Au);
  II_CHECK(ii_bus_read(&r, IP_P1) == 0x005Au);
  rc = IVS_TEST_PASS;
done:
  ii_rig_close(&r);
  return rc;
}

static int ip_per_port_direction(void)
{
  ii_rig r;
  int rc = IVS_TEST_FAIL;
  if (!ii_rig_open(&r, IVS_MS_PROG_A)) {
    return IVS_TEST_FAIL;
  }
  II_CHECK(ii_rig_set(&r, ip_word(IP_KEY_1), ip_word(IP_KEY_ENTER)));
  ii_bus_write(&r, IP_P0, 0x00A5u);
  ii_bus_write(&r, IP_P1, 0x005Au);
  ii_bus_write(&r, IP_R7, 0x0040u); /* port 0 output, port 1 input */
  II_CHECK(ii_bus_read(&r, IP_P0) == 0x00A5u && ii_bus_read(&r, IP_P1) == ip_byte(IP_KEY_ENTER));
  ii_bus_write(&r, IP_R7, 0x0080u); /* port 1 output, port 0 input */
  II_CHECK(ii_bus_read(&r, IP_P0) == ip_byte(IP_KEY_1) && ii_bus_read(&r, IP_P1) == 0x005Au);
  ii_bus_write(&r, IP_R7, 0x00C0u); /* both output */
  II_CHECK(ii_bus_read(&r, IP_P0) == 0x00A5u && ii_bus_read(&r, IP_P1) == 0x005Au);
  ii_bus_write(&r, IP_R7, 0x0000u); /* both input again: the pins follow */
  II_CHECK(ii_bus_read(&r, IP_P0) == ip_byte(IP_KEY_1) && ii_bus_read(&r, IP_P1) == ip_byte(IP_KEY_ENTER));
  rc = IVS_TEST_PASS;
done:
  ii_rig_close(&r);
  return rc;
}

static int ip_r7_other_bits_ignored(void)
{
  ii_rig r;
  uint32_t v;
  int rc = IVS_TEST_FAIL;
  if (!ii_rig_open(&r, IVS_MS_PROG_A)) {
    return IVS_TEST_FAIL;
  }
  II_CHECK(ii_rig_set(&r, ip_word(IP_KEY_1), ip_word(IP_KEY_ENTER)));
  for (v = 0u; v < 0x40u; v++) { /* every setting of bits 5..0, bits 7 and 6 clear */
    ii_bus_write(&r, IP_R7, (uint16_t)v);
    II_CHECK(ii_bus_read(&r, IP_P0) == ip_byte(IP_KEY_1) && ii_bus_read(&r, IP_P1) == ip_byte(IP_KEY_ENTER));
  }
  rc = IVS_TEST_PASS;
done:
  ii_rig_close(&r);
  return rc;
}

static int ip_upper_byte(void)
{
  ii_rig r;
  uint32_t all = 0u;
  size_t k;
  uint32_t union_mask = 0u;
  int rc = IVS_TEST_FAIL;
  if (!ii_rig_open(&r, IVS_MS_PROG_A)) {
    return IVS_TEST_FAIL;
  }
  for (k = 0u; k < ivs_ip_count; k++) {
    all |= 1u << ivs_ip_ctls[k].bit;
    union_mask |= ivs_ip_ctls[k].cleared;
  }
  II_CHECK(ii_rig_set(&r, all, all)); /* every switch closed: the pins mask is the union of all masks */
  II_CHECK(ii_bus_read(&r, IP_P0) == (0xFFu ^ union_mask) && ii_bus_read(&r, IP_P1) == (0xFFu ^ union_mask));
  II_CHECK((ii_bus_read(&r, IP_P0) & 0xFF00u) == 0u && (ii_bus_read(&r, IP_P1) & 0xFF00u) == 0u);
  ii_bus_write(&r, IP_R7, 0x00C0u); /* output mode: a wide store reads back with the upper byte 0 */
  ii_bus_write(&r, IP_P0, 0xFFFFu);
  ii_bus_write(&r, IP_P1, 0xABCDu);
  II_CHECK(ii_bus_read(&r, IP_P0) == 0x00FFu && ii_bus_read(&r, IP_P1) == 0x00CDu);
  II_CHECK(ii_peek(r.x.m, IP_P0) == 0x00FFu && ii_peek(r.x.m, IP_P1) == 0x00CDu);
  rc = IVS_TEST_PASS;
done:
  ii_rig_close(&r);
  return rc;
}

static int ip_write_latch_in_input_mode(void)
{
  ii_rig r;
  int rc = IVS_TEST_FAIL;
  if (!ii_rig_open(&r, IVS_MS_PROG_A)) {
    return IVS_TEST_FAIL;
  }
  II_CHECK(ii_rig_set(&r, ip_word(IP_KEY_1), 0u));
  ii_bus_write(&r, IP_P0, 0x00A5u);
  II_CHECK(ii_bus_read(&r, IP_P0) == ip_byte(IP_KEY_1));                       /* the pins still win */
  II_CHECK(ivs_memsys_psg_reg_internal(&ii_im(&r)->mem, 14u) == 0x00A5u);       /* the latch holds the write */
  ii_bus_write(&r, IP_R7, 0x0040u);
  II_CHECK(ii_bus_read(&r, IP_P0) == 0x00A5u);                                 /* now it shows */
  ii_bus_write(&r, IP_R7, 0x0000u);
  II_CHECK(ii_bus_read(&r, IP_P0) == ip_byte(IP_KEY_1));
  rc = IVS_TEST_PASS;
done:
  ii_rig_close(&r);
  return rc;
}

/* ---- peek, purity, independence ---- */

static int ip_peek_equals_cpu_read(void)
{
  ii_rig r;
  size_t k;
  uint32_t a;
  int rc = IVS_TEST_FAIL;
  if (!ii_rig_open(&r, IVS_MS_PROG_A)) {
    return IVS_TEST_FAIL;
  }
  for (k = 0u; k < ivs_ip_count; k++) {
    II_CHECK(ii_rig_set(&r, ip_word(ivs_ip_ctls[k].bit), ip_word(ivs_ip_ctls[(k + 7u) % ivs_ip_count].bit)));
    for (a = 0x01F0u; a <= 0x01FFu; a++) {
      II_CHECK(ii_peek(r.x.m, (uint16_t)a) == ii_bus_read(&r, (uint16_t)a));
    }
  }
  /* output mode too, with latches written, and a bank of ordinary addresses around the block */
  ii_bus_write(&r, IP_R7, 0x00C0u);
  ii_bus_write(&r, IP_P0, 0x0033u);
  ii_bus_write(&r, IP_P1, 0x00CCu);
  for (a = 0x01E8u; a <= 0x0208u; a++) {
    II_CHECK(ii_peek(r.x.m, (uint16_t)a) == ii_bus_read(&r, (uint16_t)a));
  }
  rc = IVS_TEST_PASS;
done:
  ii_rig_close(&r);
  return rc;
}

static int ip_unplugged_is_idle(void)
{
  ii_rig r;
  ivs_input_event ev;
  ivs_run_report rep;
  ivs_status st;
  static uint8_t before[IVS_MACHINE_BYTES];
  int rc = IVS_TEST_FAIL;
  if (!ii_rig_open(&r, IVS_MS_PROG_A)) {
    return IVS_TEST_FAIL;
  }
  II_CHECK(ii_rig_set(&r, 0u, 0u));
  ii_event(&ev, r.now, 0u, 0u);
  ev.controller[2] = ip_word(IP_KEY_1); /* a third controller: the profile connects two */
  memcpy(before, r.x.storage, sizeof before);
  st = ii_run(r.x.m, r.now + 100u, &ev, 1u, &rep);
  II_CHECK(st == IVS_E_UNSUPPORTED && rep.refusal_reason == IVS_REASON_INPUT_EXTRA_CONTROLLER);
  II_CHECK(memcmp(before, r.x.storage, sizeof before) == 0); /* a refusal changes nothing */
  II_CHECK(ii_peek(r.x.m, IP_P0) == 0x00FFu && ii_peek(r.x.m, IP_P1) == 0x00FFu);
  II_CHECK(ii_im(&r)->mem.port_pulldown[0] == 0u && ii_im(&r)->mem.port_pulldown[1] == 0u);
  rc = IVS_TEST_PASS;
done:
  ii_rig_close(&r);
  return rc;
}

static int ip_reads_are_pure(void)
{
  ii_rig r;
  static uint8_t before[IVS_MACHINE_BYTES];
  uint8_t d0[32];
  uint8_t d1[32];
  uint32_t a;
  int rc = IVS_TEST_FAIL;
  if (!ii_rig_open(&r, IVS_MS_PROG_A)) {
    return IVS_TEST_FAIL;
  }
  II_CHECK(ii_rig_set(&r, ip_word(IP_KEY_1) | ip_word(IP_KEY_5), ip_word(IP_KEY_ENTER)));
  II_CHECK(ivs_machine_state_digest(r.x.m, IVS_DIGEST_COMPOSITE, d0) == IVS_OK);
  memcpy(before, r.x.storage, sizeof before);
  for (a = 0x01F0u; a <= 0x01FFu; a++) {
    (void)ii_peek(r.x.m, (uint16_t)a);
    (void)ii_bus_read(&r, (uint16_t)a);
    (void)ii_bus_read(&r, (uint16_t)a);
  }
  II_CHECK(memcmp(before, r.x.storage, sizeof before) == 0);
  II_CHECK(ivs_machine_state_digest(r.x.m, IVS_DIGEST_COMPOSITE, d1) == IVS_OK && memcmp(d0, d1, 32u) == 0);
  rc = IVS_TEST_PASS;
done:
  ii_rig_close(&r);
  return rc;
}

static int ip_independence(void)
{
  ii_rig r;
  size_t k;
  int rc = IVS_TEST_FAIL;
  if (!ii_rig_open(&r, IVS_MS_PROG_A)) {
    return IVS_TEST_FAIL;
  }
  /* port 1 holds KEY_5 while port 0 walks through every control; then the reverse */
  for (k = 0u; k < ivs_ip_count; k++) {
    II_CHECK(ii_rig_set(&r, ip_word(ivs_ip_ctls[k].bit), ip_word(IP_KEY_5)));
    II_CHECK(ii_bus_read(&r, IP_P0) == ivs_ip_ctls[k].byte && ii_bus_read(&r, IP_P1) == ip_byte(IP_KEY_5));
  }
  for (k = 0u; k < ivs_ip_count; k++) {
    II_CHECK(ii_rig_set(&r, ip_word(IP_KEY_5), ip_word(ivs_ip_ctls[k].bit)));
    II_CHECK(ii_bus_read(&r, IP_P0) == ip_byte(IP_KEY_5) && ii_bus_read(&r, IP_P1) == ivs_ip_ctls[k].byte);
  }
  rc = IVS_TEST_PASS;
done:
  ii_rig_close(&r);
  return rc;
}

/* ---- digest, take-back, reset, invariant ---- */

static int ip_digest_mem_covers_pins(void)
{
  ii_rig a;
  ii_rig b;
  ii_rig c;
  uint8_t da[32];
  uint8_t db[32];
  uint8_t dc[32];
  int open_a = 0;
  int open_b = 0;
  int open_c = 0;
  int rc = IVS_TEST_FAIL;
  /* program A never reads the ports, so only the controller word (and with it the pull-down mask)
   * differs between a and b; c repeats a's schedule */
  open_a = ii_rig_open(&a, IVS_MS_PROG_A);
  open_b = ii_rig_open(&b, IVS_MS_PROG_A);
  open_c = ii_rig_open(&c, IVS_MS_PROG_A);
  II_CHECK(open_a && open_b && open_c);
  II_CHECK(ii_rig_set(&a, ip_word(IP_KEY_1), 0u) && ii_rig_set(&b, 0u, 0u) && ii_rig_set(&c, ip_word(IP_KEY_1), 0u));
  II_CHECK(ivs_machine_state_digest(a.x.m, IVS_DIGEST_MEM, da) == IVS_OK);
  II_CHECK(ivs_machine_state_digest(b.x.m, IVS_DIGEST_MEM, db) == IVS_OK);
  II_CHECK(ivs_machine_state_digest(c.x.m, IVS_DIGEST_MEM, dc) == IVS_OK);
  II_CHECK(memcmp(da, db, 32u) != 0 && memcmp(da, dc, 32u) == 0);
  II_CHECK(ivs_machine_state_digest(a.x.m, IVS_DIGEST_COMPOSITE, da) == IVS_OK);
  II_CHECK(ivs_machine_state_digest(b.x.m, IVS_DIGEST_COMPOSITE, db) == IVS_OK);
  II_CHECK(ivs_machine_state_digest(c.x.m, IVS_DIGEST_COMPOSITE, dc) == IVS_OK);
  II_CHECK(memcmp(da, db, 32u) != 0 && memcmp(da, dc, 32u) == 0);
  /* the CPU domain does not see the pins when the guest does not read them */
  II_CHECK(ivs_machine_state_digest(a.x.m, IVS_DIGEST_CPU, da) == IVS_OK);
  II_CHECK(ivs_machine_state_digest(b.x.m, IVS_DIGEST_CPU, db) == IVS_OK);
  II_CHECK(memcmp(da, db, 32u) == 0);
  rc = IVS_TEST_PASS;
done:
  if (open_a) {
    ii_rig_close(&a);
  }
  if (open_b) {
    ii_rig_close(&b);
  }
  if (open_c) {
    ii_rig_close(&c);
  }
  return rc;
}

/* A refused step (the CPU state damaged through the test seam, as run.stop.internal-input-restored
 * does) takes back the events applied for it: words and pull-down masks are those of the last whole
 * boundary. */
static int ip_takeback_restores_pins(void)
{
  ii_rig r;
  ivs_input_event ev;
  ivs_run_report rep;
  ivs_status st;
  int rc = IVS_TEST_FAIL;
  if (!ii_rig_open(&r, IVS_MS_PROG_A)) {
    return IVS_TEST_FAIL;
  }
  II_CHECK(ii_rig_set(&r, ip_word(IP_KEY_1), 0u));
  ii_im(&r)->cpu.run_state = 9u; /* the seam refuses the next unit */
  ii_event(&ev, r.now, ip_word(IP_KEY_2), ip_word(IP_KEY_ENTER));
  st = ii_run(r.x.m, r.now + 100u, &ev, 1u, &rep);
  II_CHECK(st == IVS_OK && rep.stop_reason == IVS_STOP_INTERNAL && rep.units_executed == 0u);
  II_CHECK(rep.input_consumed == 0u && rep.controller_now[0] == ip_word(IP_KEY_1) && rep.controller_now[1] == 0u);
  II_CHECK(ii_im(&r)->mem.port_pulldown[0] == ivs_ip_ctls[IP_KEY_1].cleared);
  II_CHECK(ii_im(&r)->mem.port_pulldown[1] == 0u);
  II_CHECK(ii_peek(r.x.m, IP_P0) == ip_byte(IP_KEY_1) && ii_peek(r.x.m, IP_P1) == 0x00FFu);
  rc = IVS_TEST_PASS;
done:
  ii_rig_close(&r);
  return rc;
}

static int ip_reset_clears_pulldown(void)
{
  ii_rig r;
  int rc = IVS_TEST_FAIL;
  if (!ii_rig_open(&r, IVS_MS_PROG_A)) {
    return IVS_TEST_FAIL;
  }
  II_CHECK(ii_rig_set(&r, ip_word(IP_KEY_1), ip_word(IP_KEY_ENTER)));
  II_CHECK(ii_peek(r.x.m, IP_P0) == ip_byte(IP_KEY_1));
  II_CHECK(ivs_machine_reset(r.x.m, IVS_RESET_POWER_ON) == IVS_OK);
  II_CHECK(ii_im(&r)->mem.port_pulldown[0] == 0u && ii_im(&r)->mem.port_pulldown[1] == 0u);
  II_CHECK(ii_peek(r.x.m, IP_P0) == 0x00FFu && ii_peek(r.x.m, IP_P1) == 0x00FFu);
  rc = IVS_TEST_PASS;
done:
  ii_rig_close(&r);
  return rc;
}

/* port_pulldown[p] equals the OR of the literal's masks of the words in force, for both ports. */
static int ip_invariant_holds(ii_rig *r, const char *when)
{
  const struct ivs_machine *im = ii_im(r);
  uint32_t p;
  for (p = 0u; p < 2u; p++) {
    if ((uint32_t)im->mem.port_pulldown[p] != ip_derive(im->obs.input.words[p])) {
      ivs_test_diag("%s: port_pulldown[%u] is %02X, the words in force derive %02X", when, (unsigned)p,
                    (unsigned)im->mem.port_pulldown[p], (unsigned)ip_derive(im->obs.input.words[p]));
      return 0;
    }
  }
  return 1;
}

static int ip_invariant_pulldown_derives_from_words(void)
{
  ii_rig r;
  ivs_input_event ev[4];
  ivs_run_report rep;
  size_t k;
  int rc = IVS_TEST_FAIL;
  if (!ii_rig_open(&r, IVS_MS_PROG_A)) {
    return IVS_TEST_FAIL;
  }
  II_CHECK(ip_invariant_holds(&r, "after power-on"));
  for (k = 0u; k < ivs_ip_count; k++) {
    uint32_t w0 = ip_word(ivs_ip_ctls[k].bit) | ip_word(ivs_ip_ctls[(3u * k + 5u) % ivs_ip_count].bit);
    uint32_t w1 = ip_word(ivs_ip_ctls[(k + 11u) % ivs_ip_count].bit);
    II_CHECK(ii_rig_set(&r, w0, w1));
    II_CHECK(ip_invariant_holds(&r, "after a one-unit call"));
  }
  /* one call with several events inside it: the words in force are the last applied */
  ii_event(&ev[0], r.now, ip_word(IP_KEY_1), 0u);
  ii_event(&ev[1], r.now + 30u, 0u, ip_word(IP_KEY_2));
  ii_event(&ev[2], r.now + 60u, ip_word(IP_KEY_5) | ip_word(IP_KEY_ENTER), 0u);
  ii_event(&ev[3], r.now + 1000000u, ip_word(IP_KEY_1), ip_word(IP_KEY_1)); /* never reached */
  II_CHECK(ii_run(r.x.m, r.now + 200u, ev, 4u, &rep) == IVS_OK && rep.input_consumed == 3u);
  II_CHECK(ip_invariant_holds(&r, "after a call with three events"));
  II_CHECK(ivs_machine_reset(r.x.m, IVS_RESET_POWER_ON) == IVS_OK);
  II_CHECK(ip_invariant_holds(&r, "after a reset"));
  rc = IVS_TEST_PASS;
done:
  ii_rig_close(&r);
  return rc;
}

/* ---- judge ---- */

/* 1 when the machine behaves as if the bit value output_value of R7 bit 6 makes port 0 an output: with
 * KEY_1 held and the latch written, that setting reads the latch and the opposite setting reads the
 * pins. The case judge below runs it with the sense the plan assumes (1) and with the opposite (0). */
static int ip_direction_predicate(uint32_t output_value)
{
  ii_rig r;
  uint16_t out_bits = output_value != 0u ? 0x0040u : 0x0000u;
  uint16_t in_bits = output_value != 0u ? 0x0000u : 0x0040u;
  int ok = 0;
  if (!ii_rig_open(&r, IVS_MS_PROG_A)) {
    return 0;
  }
  II_CHECK(ii_rig_set(&r, ip_word(IP_KEY_1), 0u));
  ii_bus_write(&r, IP_P0, 0x00A5u);
  ii_bus_write(&r, IP_R7, out_bits);
  II_CHECK(ii_bus_read(&r, IP_P0) == 0x00A5u);
  ii_bus_write(&r, IP_R7, in_bits);
  II_CHECK(ii_bus_read(&r, IP_P0) == ip_byte(IP_KEY_1));
  ok = 1;
done:
  ii_rig_close(&r);
  return ok;
}

/* ---- the pure derivation (plan 08-10, D-15) ---- */

/* A pseudo-random word (a 32-bit LCG; the constants of the usual numerical recipe, so no value is chosen by hand). */
static uint32_t ip_next(uint32_t *state)
{
  *state = *state * 1664525u + 1013904223u;
  return *state;
}

/* One word set for the sweep: 0, each control alone, each pair of controls, every control closed, the reserved bit alone and with
 * a control, all ones, and n pseudo-random words. The set is built into out (capacity cap); returns its size (0 when cap is short). */
static size_t ip_derive_words(uint32_t *out, size_t cap)
{
  size_t n = 0u;
  size_t a;
  size_t b;
  uint32_t seed = 0x1D872B41u;
  uint32_t all = 0u;
  for (a = 0u; a < ivs_ip_count; a++) {
    all |= ip_word(ivs_ip_ctls[a].bit);
  }
  if (cap < 1u + ivs_ip_count + ivs_ip_count * (ivs_ip_count - 1u) / 2u + 4u + 600u) {
    return 0u;
  }
  out[n++] = 0u;
  for (a = 0u; a < ivs_ip_count; a++) {
    out[n++] = ip_word(ivs_ip_ctls[a].bit);
  }
  for (a = 0u; a < ivs_ip_count; a++) {
    for (b = a + 1u; b < ivs_ip_count; b++) {
      out[n++] = ip_word(ivs_ip_ctls[a].bit) | ip_word(ivs_ip_ctls[b].bit);
    }
  }
  out[n++] = all;
  out[n++] = 0x80000000u;                                 /* the reserved bit alone: no control, no pin */
  out[n++] = 0x80000000u | ip_word(ivs_ip_ctls[0].bit);   /* with one control */
  out[n++] = 0xFFFFFFFFu;
  for (a = 0u; a < 600u; a++) {
    out[n++] = ip_next(&seed);
  }
  return n;
}

#define IP_DERIVE_MAX 1200u

static int ip_derive_pure_equals_rederive(void)
{
  uint32_t *set = malloc(IP_DERIVE_MAX * sizeof set[0]);
  uint8_t *before = malloc(IVS_MACHINE_BYTES);
  ii_rig r;
  size_t n = 0u;
  size_t i;
  uint32_t profile = 0u;
  int open = 0;
  int rc = IVS_TEST_FAIL;
  memset(&r, 0, sizeof r);
  if (set == NULL || before == NULL) {
    goto done;
  }
  n = ip_derive_words(set, IP_DERIVE_MAX);
  II_CHECK(n > 0u);
  II_CHECK(ii_rig_open(&r, IVS_MS_PROG_A));
  open = 1;
  profile = ii_im(&r)->profile;
  for (i = 0u; i < n; i++) {
    struct ivs_machine *im = ii_im(&r);
    uint32_t w[4];
    uint8_t out[2];
    uint32_t p;
    w[0] = set[i];
    w[1] = set[(i * 7u + 3u) % n]; /* the two ports get different words, in every combination over the sweep */
    w[2] = 0xFFFFFFFFu;            /* words past the profile's controllers are never read */
    w[3] = set[(i + 1u) % n];
    /* the instance is brought to the state rederive gives for the words in force, and a copy is kept */
    memcpy(im->obs.input.words, w, sizeof w);
    ivs_input_rederive(im);
    memcpy(before, im, IVS_MACHINE_BYTES);
    out[0] = 0xEEu;
    out[1] = 0xEEu;
    ivs_input_derive(profile, w, out);
    II_CHECK(memcmp(before, im, IVS_MACHINE_BYTES) == 0); /* the instance is not written by the pure form */
    for (p = 0u; p < 2u; p++) {
      /* the literal's OR of the masks, and what rederive stored in the instance */
      if ((uint32_t)out[p] != ip_derive(w[p]) || out[p] != im->mem.port_pulldown[p]) {
        ivs_test_diag("word %08X on port %u: derive %02X, the literal %02X, the instance %02X", (unsigned)w[p], (unsigned)p,
                      (unsigned)out[p], (unsigned)ip_derive(w[p]), (unsigned)im->mem.port_pulldown[p]);
        goto done;
      }
    }
  }
  /* a profile index outside the generated table: two zero masks, whatever the words */
  {
    uint32_t w[4] = {0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu};
    uint32_t bad[3];
    uint32_t k;
    bad[0] = ivs_input_profile_count();
    bad[1] = ivs_input_profile_count() + 1u;
    bad[2] = 0xFFFFFFFFu;
    for (k = 0u; k < 3u; k++) {
      uint8_t out[2] = {0xEEu, 0xEEu};
      ivs_input_derive(bad[k], w, out);
      II_CHECK(out[0] == 0u && out[1] == 0u);
    }
    {
      uint8_t out[2] = {0xEEu, 0xEEu};
      ivs_input_derive(profile, w, out); /* and the real profile with every control closed is not zero */
      II_CHECK(out[0] != 0u && out[1] != 0u);
    }
  }
  rc = IVS_TEST_PASS;
done:
  if (open) {
    ii_rig_close(&r);
  }
  free(set);
  free(before);
  return rc;
}

static int ip_judge_direction_sense_flipped(void)
{
  if (!ip_direction_predicate(1u)) {
    ivs_test_diag("the predicate rejects the sense the profile carries");
    return IVS_TEST_FAIL;
  }
  if (ip_direction_predicate(0u)) {
    ivs_test_diag("the predicate accepts the inverted R7 sense");
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

static const ivs_test_case ip_cases[] = {
    {"input.port.direction-input-reads-pins", ip_direction_input_reads_pins, NULL},
    {"input.port.direction-output-reads-latch", ip_direction_output_reads_latch, NULL},
    {"input.port.per-port-direction", ip_per_port_direction, NULL},
    {"input.port.r7-other-bits-ignored", ip_r7_other_bits_ignored, NULL},
    {"input.port.upper-byte", ip_upper_byte, NULL},
    {"input.port.write-latch-in-input-mode", ip_write_latch_in_input_mode, NULL},
    {"input.port.peek-equals-cpu-read", ip_peek_equals_cpu_read, NULL},
    {"input.port.unplugged-is-idle", ip_unplugged_is_idle, NULL},
    {"input.port.reads-are-pure", ip_reads_are_pure, NULL},
    {"input.port.independence", ip_independence, NULL},
    {"input.digest.mem-covers-pins", ip_digest_mem_covers_pins, NULL},
    {"input.takeback.restores-pins", ip_takeback_restores_pins, NULL},
    {"input.reset.clears-pulldown", ip_reset_clears_pulldown, NULL},
    {"input.invariant.pulldown-derives-from-words", ip_invariant_pulldown_derives_from_words, NULL},
    {"input.derive.pure-equals-rederive", ip_derive_pure_equals_rederive, NULL},
    {"input.judge.direction-sense-flipped", ip_judge_direction_sense_flipped, NULL},
};

II_TABLE_FAMILY(port, ip_cases);
