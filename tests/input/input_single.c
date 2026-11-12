/* SPDX-License-Identifier: MIT */
/* Suite input, family single (INP-01, plan 05-05, D-02, D-04, D-06): the 31 single controls on both
 * ports, the wiki anchors, the structure of the literal and the generated table against it.
 * Claim tier source-expectation (single-control rows, tag tier4-community, C16) and original
 * diagnostic of the guest read path (a stand-in program, no rights-bearing byte).
 *
 *   input.single.p0.<name> and input.single.p1.<name> (31 names each, 62 ids): the control pressed
 *                            alone on that port; the guest read (stand-in program PORTS: MVI of the port
 *                            stored into RAM) and ivs_machine_peek both equal $00 FF XOR cleared (the
 *                            literal's byte), the other port reads $00FF
 *   input.idle.p0/p1         no event, and events then a power-on reset: the port reads $00FF, by the guest
 *                            and by peek
 *   input.anchor.wiki-idle   an idle port reads $00FF (written here from the wiki prose)
 *   input.anchor.wiki-key1   KEY_1 alone reads $007E (written here from the wiki prose)
 *   input.anchor.wiki-key1-9 KEY_1 and KEY_9 together read $005A (written here from the wiki prose)
 *   input.struct.*           the property facts of the literal rows: counts, distinct masks, the pins of
 *                            keys, discs and side buttons, the table identical for both ports
 *   input.table.matches-literal  the generated table the core was built with gives, for all 31 controls on
 *                            both ports, the literal's byte (observed through the machine)
 *   input.judge.table-row-changed  the predicate of the previous case rejects a literal with one byte changed
 * Expected bytes come from the literal records (cmake/IvsInputProbes.cmake) or the literals written in
 * this file, never from the profile or the core's table. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "input_cases.h"

#include "ivs_input_probes.inc"

/* The pin-to-bit map of the literal header (ConsoleMods): pin 4 = b0, 3 = b1, 2 = b2, 1 = b3, 9 = b4,
 * 8 = b5, 7 = b6, 6 = b7; pin 5 is ground. Written here from the header, not read from a table. */
static const int is_pin_bit[10] = {-1, 3, 2, 1, 0, -1, 7, 6, 5, 4};

const ivs_test_source *ivs_input_sources(size_t *n)
{
  *n = sizeof ivs_ip_sources / sizeof ivs_ip_sources[0];
  return ivs_ip_sources;
}

/* One powered instance running the PORTS program, one event of word w0 on port 0 and w1 on port 1 at
 * tick 0, run for II_RUN_TICKS. The guest read of port 0 is left in *g0 and of port 1 in *g1, the
 * peeks of $01FE and $01FF in *p0 and *p1. */
static int is_run_pair(uint32_t w0, uint32_t w1, uint32_t *g0, uint32_t *g1, uint32_t *p0, uint32_t *p1)
{
  ivs_ms_inst x;
  ivs_input_event ev;
  ivs_run_report rep;
  int ok = 0;
  if (!ivs_ms_powered_prog(&x, IVS_MS_PROG_PORTS, II_POISON)) {
    return 0;
  }
  ii_event(&ev, 0u, w0, w1);
  II_CHECK(ii_run(x.m, II_RUN_TICKS, &ev, 1u, &rep) == IVS_OK);
  II_CHECK(rep.input_consumed == 1u && rep.stop_reason == IVS_STOP_TARGET);
  *g0 = ii_peek(x.m, 0x0200u);
  *g1 = ii_peek(x.m, 0x0201u);
  *p0 = ii_peek(x.m, 0x01FEu);
  *p1 = ii_peek(x.m, 0x01FFu);
  ok = 1;
done:
  ivs_ms_free(&x);
  return ok;
}

/* Both read paths of both ports against the wanted bytes (low byte, upper byte $00). */
static int is_expect_pair(uint32_t w0, uint32_t w1, uint32_t want0, uint32_t want1, const char *what)
{
  uint32_t g0;
  uint32_t g1;
  uint32_t p0;
  uint32_t p1;
  if (!is_run_pair(w0, w1, &g0, &g1, &p0, &p1)) {
    return IVS_TEST_FAIL;
  }
  if (g0 != want0 || p0 != want0 || g1 != want1 || p1 != want1) {
    ivs_test_diag("%s: guest %04X %04X, peek %04X %04X, want %04X %04X", what, (unsigned)g0, (unsigned)g1,
                  (unsigned)p0, (unsigned)p1, (unsigned)want0, (unsigned)want1);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

/* ---- single controls ---- */

static int is_single(size_t k, uint32_t port)
{
  const ivs_ip_ctl *c = &ivs_ip_ctls[k];
  uint32_t word = 1u << c->bit;
  uint32_t want0 = port == 0u ? c->byte : 0x00FFu;
  uint32_t want1 = port == 1u ? c->byte : 0x00FFu;
  return is_expect_pair(port == 0u ? word : 0u, port == 1u ? word : 0u, want0, want1, c->name);
}

/* ---- idle ---- */

/* The port reads $00FF with no event, and after events then a power-on reset (words and pins clear). */
static int is_idle(uint32_t port)
{
  ivs_ms_inst x;
  ivs_input_event ev;
  ivs_run_report rep;
  uint16_t addr = port == 0u ? 0x01FEu : 0x01FFu;
  uint16_t ram = port == 0u ? 0x0200u : 0x0201u;
  int rc = IVS_TEST_FAIL;
  if (!ivs_ms_powered_prog(&x, IVS_MS_PROG_PORTS, II_POISON)) {
    return IVS_TEST_FAIL;
  }
  II_CHECK(ii_peek(x.m, addr) == 0x00FFu); /* straight after power-on */
  II_CHECK(ii_run(x.m, II_RUN_TICKS, NULL, 0u, &rep) == IVS_OK);
  II_CHECK(ii_peek(x.m, ram) == 0x00FFu && ii_peek(x.m, addr) == 0x00FFu);
  ii_event(&ev, rep.now_ticks, IVS_CTRL_KEY_1 | IVS_CTRL_DISC_N, IVS_CTRL_KEY_ENTER | IVS_CTRL_SIDE_TOP);
  II_CHECK(ii_run(x.m, 2u * II_RUN_TICKS, &ev, 1u, &rep) == IVS_OK);
  II_CHECK(ii_peek(x.m, addr) != 0x00FFu); /* the event took effect */
  II_CHECK(ivs_machine_reset(x.m, IVS_RESET_POWER_ON) == IVS_OK);
  II_CHECK(ii_peek(x.m, addr) == 0x00FFu);
  II_CHECK(ii_run(x.m, II_RUN_TICKS, NULL, 0u, &rep) == IVS_OK);
  II_CHECK(ii_peek(x.m, ram) == 0x00FFu && ii_peek(x.m, addr) == 0x00FFu);
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

static int is_idle_p0(void)
{
  return is_idle(0u);
}

static int is_idle_p1(void)
{
  return is_idle(1u);
}

/* ---- the wiki anchors (written here from the wiki prose of the literal header) ---- */

static int is_anchor_wiki_idle(void)
{
  return is_expect_pair(0u, 0u, 0x00FFu, 0x00FFu, "idle");
}

static int is_anchor_wiki_key1(void)
{
  return is_expect_pair(IVS_CTRL_KEY_1, 0u, 0x007Eu, 0x00FFu, "KEY_1 on port 0");
}

static int is_anchor_wiki_key1_9(void)
{
  return is_expect_pair(IVS_CTRL_KEY_1 | IVS_CTRL_KEY_9, 0u, 0x005Au, 0x00FFu, "KEY_1 and KEY_9 on port 0");
}

/* ---- the structure of the literal ---- */

/* The mask the pins of record c give through the pin-to-bit map. */
static uint32_t is_pins_to_mask(const ivs_ip_ctl *c)
{
  uint32_t mask = 0u;
  uint32_t pin;
  for (pin = 1u; pin < 10u; pin++) {
    if ((c->pins & (1u << pin)) != 0u && is_pin_bit[pin] >= 0) {
      mask |= 1u << (uint32_t)is_pin_bit[pin];
    }
  }
  return mask;
}

/* Every record: the pins mapped through the header's pin-to-bit map give cleared (pin 5 and pin 0 never
 * occur), and byte is FF XOR cleared. */
static int is_records_consistent(const ivs_ip_ctl *recs, size_t n)
{
  size_t k;
  for (k = 0u; k < n; k++) {
    if ((recs[k].pins & ((1u << 5) | 1u)) != 0u || is_pins_to_mask(&recs[k]) != recs[k].cleared
        || recs[k].byte != (uint8_t)(0xFFu ^ recs[k].cleared) || recs[k].bit != k) {
      ivs_test_diag("record %s is inconsistent (pins %03X cleared %02X byte %02X)", recs[k].name,
                    (unsigned)recs[k].pins, (unsigned)recs[k].cleared, (unsigned)recs[k].byte);
      return 0;
    }
  }
  return 1;
}

static int is_has_prefix(const char *s, const char *prefix)
{
  return strncmp(s, prefix, strlen(prefix)) == 0;
}

static int is_struct_counts(void)
{
  size_t keys = 0u;
  size_t discs = 0u;
  size_t sides = 0u;
  size_t k;
  if (!is_records_consistent(ivs_ip_ctls, ivs_ip_count)) {
    return IVS_TEST_FAIL;
  }
  for (k = 0u; k < ivs_ip_count; k++) {
    keys += (size_t)is_has_prefix(ivs_ip_ctls[k].name, "key_");
    discs += (size_t)is_has_prefix(ivs_ip_ctls[k].name, "disc_");
    sides += (size_t)is_has_prefix(ivs_ip_ctls[k].name, "side_");
  }
  if (keys != 12u || discs != 16u || sides != 3u || keys + discs + sides != ivs_ip_count || ivs_ip_count != 31u) {
    ivs_test_diag("counts: %u keys, %u discs, %u sides, %u controls", (unsigned)keys, (unsigned)discs,
                  (unsigned)sides, (unsigned)ivs_ip_count);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

static int is_struct_single_masks_distinct(void)
{
  size_t a;
  size_t b;
  for (a = 0u; a < ivs_ip_count; a++) {
    for (b = a + 1u; b < ivs_ip_count; b++) {
      if (ivs_ip_ctls[a].cleared == ivs_ip_ctls[b].cleared) {
        ivs_test_diag("%s and %s have the same mask %02X", ivs_ip_ctls[a].name, ivs_ip_ctls[b].name,
                      (unsigned)ivs_ip_ctls[a].cleared);
        return IVS_TEST_FAIL;
      }
    }
  }
  return IVS_TEST_PASS;
}

/* Pin sets of the families (the facts of the matrix): the keypad rows are pins {4,3,2,1}, the keypad
 * columns and the side buttons pins {6,7,8}, the disc uses pins {1,2,3,4,9}. */
#define IS_ROWS ((1u << 4) | (1u << 3) | (1u << 2) | (1u << 1))
#define IS_COLS ((1u << 6) | (1u << 7) | (1u << 8))
#define IS_DISC (IS_ROWS | (1u << 9))

static unsigned is_popcount(unsigned v)
{
  unsigned n = 0u;
  while (v != 0u) {
    n += v & 1u;
    v >>= 1;
  }
  return n;
}

static int is_struct_key_masks_two_pins(void)
{
  size_t k;
  size_t seen = 0u;
  if (!is_records_consistent(ivs_ip_ctls, ivs_ip_count)) {
    return IVS_TEST_FAIL;
  }
  for (k = 0u; k < ivs_ip_count; k++) {
    const ivs_ip_ctl *c = &ivs_ip_ctls[k];
    if (is_has_prefix(c->name, "key_")) {
      seen++;
      if (is_popcount(c->pins) != 2u || is_popcount(c->pins & IS_ROWS) != 1u || is_popcount(c->pins & IS_COLS) != 1u) {
        ivs_test_diag("%s: pins %03X are not one of {4,3,2,1} and one of {6,7,8}", c->name, (unsigned)c->pins);
        return IVS_TEST_FAIL;
      }
    }
  }
  return seen == 12u ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int is_struct_disc_masks_within_disc_pins(void)
{
  size_t k;
  size_t seen = 0u;
  if (!is_records_consistent(ivs_ip_ctls, ivs_ip_count)) {
    return IVS_TEST_FAIL;
  }
  for (k = 0u; k < ivs_ip_count; k++) {
    const ivs_ip_ctl *c = &ivs_ip_ctls[k];
    if (is_has_prefix(c->name, "disc_")) {
      seen++;
      if ((c->pins & ~(unsigned)IS_DISC) != 0u || is_popcount(c->pins) < 1u || is_popcount(c->pins) > 3u) {
        ivs_test_diag("%s: pins %03X are not within {1,2,3,4,9}", c->name, (unsigned)c->pins);
        return IVS_TEST_FAIL;
      }
    }
  }
  return seen == 16u ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int is_struct_side_masks_two_pins(void)
{
  size_t k;
  size_t seen = 0u;
  if (!is_records_consistent(ivs_ip_ctls, ivs_ip_count)) {
    return IVS_TEST_FAIL;
  }
  for (k = 0u; k < ivs_ip_count; k++) {
    const ivs_ip_ctl *c = &ivs_ip_ctls[k];
    if (is_has_prefix(c->name, "side_")) {
      seen++;
      if (is_popcount(c->pins) != 2u || (c->pins & ~(unsigned)IS_COLS) != 0u) {
        ivs_test_diag("%s: pins %03X are not two of {6,7,8}", c->name, (unsigned)c->pins);
        return IVS_TEST_FAIL;
      }
    }
  }
  return seen == 3u ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- the machine against the literal ---- */

/* For every control, pressed alone on port 0 and then on port 1 of one rig, the peek of the pressed port
 * equals recs[k].byte and the other port reads $00FF. Returns 1 when all 62 observations agree. The
 * records are a parameter so the judge case can run the predicate against an altered copy. */
static int is_table_matches(const ivs_ip_ctl *recs, size_t n)
{
  ii_rig r;
  size_t k;
  uint32_t port;
  int ok = 0;
  if (!ii_rig_open(&r, IVS_MS_PROG_A)) {
    return 0;
  }
  for (k = 0u; k < n; k++) {
    for (port = 0u; port < 2u; port++) {
      uint32_t word = 1u << recs[k].bit;
      uint32_t pressed;
      uint32_t other;
      II_CHECK(ii_rig_set(&r, port == 0u ? word : 0u, port == 1u ? word : 0u));
      pressed = ii_peek(r.x.m, port == 0u ? 0x01FEu : 0x01FFu);
      other = ii_peek(r.x.m, port == 0u ? 0x01FFu : 0x01FEu);
      if (pressed != recs[k].byte || other != 0x00FFu) {
        ivs_test_diag("%s on port %u reads %04X (other port %04X), the literal gives %02X", recs[k].name,
                      (unsigned)port, (unsigned)pressed, (unsigned)other, (unsigned)recs[k].byte);
        goto done;
      }
    }
  }
  ok = 1;
done:
  ii_rig_close(&r);
  return ok;
}

static int is_table_matches_literal(void)
{
  return is_table_matches(ivs_ip_ctls, ivs_ip_count) ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* The table is identical for both ports: the byte of every control read from port 0 equals the byte
 * read from port 1. */
static int is_struct_port_tables_identical(void)
{
  ii_rig r;
  size_t k;
  int rc = IVS_TEST_FAIL;
  if (!ii_rig_open(&r, IVS_MS_PROG_A)) {
    return IVS_TEST_FAIL;
  }
  for (k = 0u; k < ivs_ip_count; k++) {
    uint32_t word = 1u << ivs_ip_ctls[k].bit;
    uint32_t b0;
    uint32_t b1;
    II_CHECK(ii_rig_set(&r, word, 0u));
    b0 = ii_peek(r.x.m, 0x01FEu);
    II_CHECK(ii_rig_set(&r, 0u, word));
    b1 = ii_peek(r.x.m, 0x01FFu);
    if (b0 != b1) {
      ivs_test_diag("%s reads %04X on port 0 and %04X on port 1", ivs_ip_ctls[k].name, (unsigned)b0, (unsigned)b1);
      goto done;
    }
  }
  rc = IVS_TEST_PASS;
done:
  ii_rig_close(&r);
  return rc;
}

/* The predicate of input.table.matches-literal must reject a literal with one byte changed (and accept
 * the real one): a predicate that always passed would not be a control. */
static int is_judge_table_row_changed(void)
{
  ivs_ip_ctl copy[II_MAX_CTL];
  size_t k;
  if (ivs_ip_count > II_MAX_CTL) {
    return IVS_TEST_FAIL;
  }
  for (k = 0u; k < ivs_ip_count; k++) {
    copy[k] = ivs_ip_ctls[k];
  }
  if (!is_table_matches(copy, ivs_ip_count)) {
    ivs_test_diag("the predicate rejects the real literal");
    return IVS_TEST_FAIL;
  }
  copy[0].byte = (uint8_t)(copy[0].byte ^ 0x01u);
  if (is_table_matches(copy, ivs_ip_count)) {
    ivs_test_diag("the predicate accepts a literal with the byte of KEY_1 changed");
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

/* ---- the family ---- */

static const ivs_test_case is_fixed[] = {
    {"input.idle.p0", is_idle_p0, NULL},
    {"input.idle.p1", is_idle_p1, NULL},
    {"input.anchor.wiki-idle", is_anchor_wiki_idle, NULL},
    {"input.anchor.wiki-key1", is_anchor_wiki_key1, NULL},
    {"input.anchor.wiki-key1-9", is_anchor_wiki_key1_9, NULL},
    {"input.struct.counts", is_struct_counts, NULL},
    {"input.struct.single-masks-distinct", is_struct_single_masks_distinct, NULL},
    {"input.struct.key-masks-two-pins", is_struct_key_masks_two_pins, NULL},
    {"input.struct.disc-masks-within-disc-pins", is_struct_disc_masks_within_disc_pins, NULL},
    {"input.struct.side-masks-two-pins", is_struct_side_masks_two_pins, NULL},
    {"input.struct.port-tables-identical", is_struct_port_tables_identical, NULL},
    {"input.table.matches-literal", is_table_matches_literal, NULL},
    {"input.judge.table-row-changed", is_judge_table_row_changed, NULL},
};
#define IS_FIXED (sizeof is_fixed / sizeof is_fixed[0])

/* The 62 single-control ids are built from the literal's names at first use. */
#define IS_SINGLE_MAX (2u * II_MAX_CTL)
static char is_single_ids[IS_SINGLE_MAX][64];
static size_t is_single_built;

static void is_build_ids(void)
{
  size_t k;
  if (is_single_built != 0u) {
    return;
  }
  for (k = 0u; k < ivs_ip_count && k < II_MAX_CTL; k++) {
    (void)snprintf(is_single_ids[2u * k], sizeof is_single_ids[0], "input.single.p0.%s", ivs_ip_ctls[k].name);
    (void)snprintf(is_single_ids[2u * k + 1u], sizeof is_single_ids[0], "input.single.p1.%s", ivs_ip_ctls[k].name);
  }
  is_single_built = 1u;
}

static size_t is_count(void)
{
  return IS_FIXED + 2u * ivs_ip_count;
}

static const char *is_id(size_t i)
{
  if (i < IS_FIXED) {
    return is_fixed[i].id;
  }
  is_build_ids();
  return is_single_ids[i - IS_FIXED];
}

static int is_run(size_t i)
{
  size_t j;
  if (i < IS_FIXED) {
    return is_fixed[i].fn();
  }
  j = i - IS_FIXED;
  return is_single(j / 2u, (uint32_t)(j % 2u));
}

const ii_family ivs_input_fam_single = {is_count, is_id, is_run};
