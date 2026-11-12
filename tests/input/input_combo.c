/* SPDX-License-Identifier: MIT */
/* Suite input, family combo (INP-02, plan 05-05, D-03, P5-C2): several controls closed at once on one
 * port, and the algebraic properties of the combination rule over all pairs of the 31 controls.
 *
 * The rule (D-03): the byte a read returns is FF XOR the OR of the cleared masks of every closed
 * control; the core never resolves a combination and accepts any subset of the 31 bits. The expected
 * byte of every row is computed in the test from the literal records (cmake/IvsInputProbes.cmake) as
 * 0xFF XOR OR(cleared masks of the named controls), except the wiki anchors written as literals
 * (KEY_1 with KEY_9 is $5A). Each row names its class of data/expected/input-policies.ids and that
 * class's tag; a row of a class tagged unknown-default-chosen is a policy row (C63): its result is the
 * model's definition, never evidence of what hardware reads. The rows read the port through ivs_machine_peek
 * and a CPU read of the memory system (the guest read path is covered by input.single.*).
 *
 *   input.combo.<row>                 about 24 named rows, class in the comment of each
 *   input.pairs.union                 all 465 unordered pairs: the byte equals FF XOR (m_a OR m_b)
 *   input.pairs.commutative           all 930 ordered pairs: A then A+B equals B then A+B on both ports (the
 *                                     read depends only on the words in force, never on the history)
 *   input.pairs.monotone-clear        every pair's byte has the bits of each single's mask cleared, and
 *                                     closing a third control never sets a bit (all 4,495 triples)
 *   input.pairs.decodable-claims      the three wiki statements as properties of the literal: no collision
 *                                     among {idle, single key, single disc, single side, disc + side}; every
 *                                     key + side pair, every side + side pair and most key + disc pairs equal
 *                                     the byte of a different state (counts derived, tag source-expectation) */
#include <stdint.h>
#include <string.h>

#include "input_cases.h"

#define IC_P0 0x01FEu
#define IC_P1 0x01FFu

/* The index (= bit number) of a control by its lower case literal name; II_MAX_CTL when absent. */
static size_t ic_index(const char *name)
{
  size_t k;
  for (k = 0u; k < ivs_ip_count; k++) {
    if (strcmp(ivs_ip_ctls[k].name, name) == 0) {
      return k;
    }
  }
  return II_MAX_CTL;
}

/* One row: the named controls closed together on port 0, then on port 1 (the other port idle): the
 * peek and the CPU read of the closed port equal want, the other port reads $00FF. want is the literal
 * byte when lit is 0..255, else computed from the literal records. */
static int ic_row(const char *const *names, size_t n, int lit)
{
  ii_rig r;
  uint32_t word = 0u;
  uint32_t mask = 0u;
  uint32_t want;
  uint32_t port;
  size_t i;
  int rc = IVS_TEST_FAIL;
  for (i = 0u; i < n; i++) {
    size_t k = ic_index(names[i]);
    if (k >= ivs_ip_count) {
      ivs_test_diag("no control named %s in the literal", names[i]);
      return IVS_TEST_FAIL;
    }
    word |= 1u << ivs_ip_ctls[k].bit;
    mask |= ivs_ip_ctls[k].cleared;
  }
  want = lit >= 0 ? (uint32_t)lit : (0xFFu ^ mask);
  if (!ii_rig_open(&r, IVS_MS_PROG_A)) {
    return IVS_TEST_FAIL;
  }
  for (port = 0u; port < 2u; port++) {
    uint16_t closed = port == 0u ? IC_P0 : IC_P1;
    uint16_t other = port == 0u ? IC_P1 : IC_P0;
    II_CHECK(ii_rig_set(&r, port == 0u ? word : 0u, port == 1u ? word : 0u));
    if (ii_peek(r.x.m, closed) != want || ii_bus_read(&r, closed) != want || ii_peek(r.x.m, other) != 0x00FFu) {
      ivs_test_diag("%s... on port %u reads %04X, want %04X", names[0], (unsigned)port,
                    (unsigned)ii_peek(r.x.m, closed), (unsigned)want);
      goto done;
    }
  }
  rc = IVS_TEST_PASS;
done:
  ii_rig_close(&r);
  return rc;
}

#define IC_ROW(fn, lit, ...)                                                                           \
  static int fn(void)                                                                                  \
  {                                                                                                    \
    static const char *const n[] = {__VA_ARGS__};                                                      \
    return ic_row(n, sizeof n / sizeof n[0], (lit));                                                   \
  }

/* class key-key (tier4-community): the wiki anchor, written as a literal */
IC_ROW(ic_key1_key9, 0x5A, "key_1", "key_9")
/* class key-key */
IC_ROW(ic_key2_key8, -1, "key_2", "key_8")
IC_ROW(ic_clear_enter, -1, "key_clear", "key_enter")
/* class disc-side (tier4-community) */
IC_ROW(ic_discn_sidetop, -1, "disc_n", "side_top")
IC_ROW(ic_disce_sidell, -1, "disc_e", "side_lower_left")
IC_ROW(ic_discsw_sidelr, -1, "disc_sw", "side_lower_right")
/* class disc-disc-adjacent (unknown-default-chosen, C63, policy: not evidence) */
IC_ROW(ic_discn_discnne, -1, "disc_n", "disc_nne")
IC_ROW(ic_disce_discese, -1, "disc_e", "disc_ese")
/* class disc-disc-other (unknown-default-chosen, C63, policy: not evidence) */
IC_ROW(ic_discn_disce, -1, "disc_n", "disc_e")
IC_ROW(ic_discnne_discese, -1, "disc_nne", "disc_ese")
/* class opposite-directions (unknown-default-chosen, C63, policy: not evidence) */
IC_ROW(ic_discn_discs, -1, "disc_n", "disc_s")
IC_ROW(ic_disce_discw2, -1, "disc_e", "disc_w")
IC_ROW(ic_discne_discsw, -1, "disc_ne", "disc_sw")
IC_ROW(ic_discnw_discse, -1, "disc_nw", "disc_se")
/* class side-side (inference, guest-ambiguous) */
IC_ROW(ic_sidetop_sidell, -1, "side_top", "side_lower_left")
IC_ROW(ic_sides_all, -1, "side_top", "side_lower_left", "side_lower_right")
/* class key-disc (inference, guest-ambiguous) */
IC_ROW(ic_key5_discn, -1, "key_5", "disc_n")
IC_ROW(ic_key0_discw, -1, "key_0", "disc_w")
/* class key-side (inference, guest-ambiguous) */
IC_ROW(ic_key5_sidetop, -1, "key_5", "side_top")
IC_ROW(ic_keyenter_sidelr, -1, "key_enter", "side_lower_right")
/* class three-or-more (unknown-default-chosen, C63, policy: not evidence) */
IC_ROW(ic_key1_key9_discn, -1, "key_1", "key_9", "disc_n")
IC_ROW(ic_discn_disce_discs, -1, "disc_n", "disc_e", "disc_s")

/* class three-or-more: all 31 controls closed on one port */
static int ic_all_closed(void)
{
  const char *names[II_MAX_CTL];
  size_t k;
  if (ivs_ip_count > II_MAX_CTL) {
    return IVS_TEST_FAIL;
  }
  for (k = 0u; k < ivs_ip_count; k++) {
    names[k] = ivs_ip_ctls[k].name;
  }
  return ic_row(names, ivs_ip_count, -1);
}

/* Different combinations on both ports at once (class independence: each port keeps its own rule). */
static int ic_ports_independent(void)
{
  ii_rig r;
  uint32_t w0 = (1u << ic_index("key_1")) | (1u << ic_index("disc_n"));
  uint32_t w1 = (1u << ic_index("key_9")) | (1u << ic_index("side_top"));
  uint32_t m0 = ivs_ip_ctls[ic_index("key_1")].cleared | ivs_ip_ctls[ic_index("disc_n")].cleared;
  uint32_t m1 = ivs_ip_ctls[ic_index("key_9")].cleared | ivs_ip_ctls[ic_index("side_top")].cleared;
  int rc = IVS_TEST_FAIL;
  if (!ii_rig_open(&r, IVS_MS_PROG_A)) {
    return IVS_TEST_FAIL;
  }
  II_CHECK(ii_rig_set(&r, w0, w1));
  II_CHECK(ii_peek(r.x.m, IC_P0) == (0xFFu ^ m0) && ii_peek(r.x.m, IC_P1) == (0xFFu ^ m1));
  II_CHECK(ii_bus_read(&r, IC_P0) == (0xFFu ^ m0) && ii_bus_read(&r, IC_P1) == (0xFFu ^ m1));
  rc = IVS_TEST_PASS;
done:
  ii_rig_close(&r);
  return rc;
}

/* ---- pair properties ---- */

static uint32_t ic_cleared(size_t k)
{
  return ivs_ip_ctls[k].cleared;
}

static uint32_t ic_word(size_t k)
{
  return 1u << ivs_ip_ctls[k].bit;
}

/* Every unordered pair {a, b}: the same word on both ports reads FF XOR (m_a OR m_b) on both. */
static int ic_pairs_union(void)
{
  ii_rig r;
  size_t a;
  size_t b;
  int rc = IVS_TEST_FAIL;
  if (!ii_rig_open(&r, IVS_MS_PROG_A)) {
    return IVS_TEST_FAIL;
  }
  for (a = 0u; a < ivs_ip_count; a++) {
    for (b = a + 1u; b < ivs_ip_count; b++) {
      uint32_t w = ic_word(a) | ic_word(b);
      uint32_t want = 0xFFu ^ (ic_cleared(a) | ic_cleared(b));
      II_CHECK(ii_rig_set(&r, w, w));
      if (ii_peek(r.x.m, IC_P0) != want || ii_peek(r.x.m, IC_P1) != want) {
        ivs_test_diag("%s + %s reads %04X and %04X, want %04X", ivs_ip_ctls[a].name, ivs_ip_ctls[b].name,
                      (unsigned)ii_peek(r.x.m, IC_P0), (unsigned)ii_peek(r.x.m, IC_P1), (unsigned)want);
        goto done;
      }
    }
  }
  rc = IVS_TEST_PASS;
done:
  ii_rig_close(&r);
  return rc;
}

/* Every ordered pair (a, b): A then A+B, and B then A+B, read the same byte on both ports (the read
 * follows the words in force, not the history), and it is the union. */
static int ic_pairs_commutative(void)
{
  ii_rig r;
  size_t a;
  size_t b;
  int rc = IVS_TEST_FAIL;
  if (!ii_rig_open(&r, IVS_MS_PROG_A)) {
    return IVS_TEST_FAIL;
  }
  for (a = 0u; a < ivs_ip_count; a++) {
    for (b = 0u; b < ivs_ip_count; b++) {
      uint32_t w = ic_word(a) | ic_word(b);
      uint32_t first0;
      uint32_t first1;
      uint32_t second0;
      uint32_t second1;
      if (a == b) {
        continue;
      }
      II_CHECK(ii_rig_set(&r, ic_word(a), ic_word(b))); /* A on port 0 and B on port 1 first */
      II_CHECK(ii_rig_set(&r, w, w));
      first0 = ii_peek(r.x.m, IC_P0);
      first1 = ii_peek(r.x.m, IC_P1);
      II_CHECK(ii_rig_set(&r, ic_word(b), ic_word(a))); /* then the other way round */
      II_CHECK(ii_rig_set(&r, w, w));
      second0 = ii_peek(r.x.m, IC_P0);
      second1 = ii_peek(r.x.m, IC_P1);
      if (first0 != second0 || first1 != second1 || first0 != first1
          || first0 != (0xFFu ^ (ic_cleared(a) | ic_cleared(b)))) {
        ivs_test_diag("%s then %s: %04X %04X, other order %04X %04X", ivs_ip_ctls[a].name, ivs_ip_ctls[b].name,
                      (unsigned)first0, (unsigned)first1, (unsigned)second0, (unsigned)second1);
        goto done;
      }
    }
  }
  rc = IVS_TEST_PASS;
done:
  ii_rig_close(&r);
  return rc;
}

/* Every pair's byte has the bits of both singles' masks cleared, and closing a third control to a pair
 * never sets a bit of its byte (4,495 triples). */
static int ic_pairs_monotone_clear(void)
{
  static uint8_t pair_byte[II_MAX_CTL][II_MAX_CTL];
  ii_rig r;
  size_t a;
  size_t b;
  size_t c;
  int rc = IVS_TEST_FAIL;
  if (ivs_ip_count > II_MAX_CTL || !ii_rig_open(&r, IVS_MS_PROG_A)) {
    return IVS_TEST_FAIL;
  }
  for (a = 0u; a < ivs_ip_count; a++) {
    for (b = a + 1u; b < ivs_ip_count; b++) {
      uint32_t got;
      II_CHECK(ii_rig_set(&r, ic_word(a) | ic_word(b), 0u));
      got = ii_peek(r.x.m, IC_P0);
      II_CHECK((got & ic_cleared(a)) == 0u && (got & ic_cleared(b)) == 0u);
      pair_byte[a][b] = (uint8_t)got;
    }
  }
  for (a = 0u; a < ivs_ip_count; a++) {
    for (b = a + 1u; b < ivs_ip_count; b++) {
      for (c = b + 1u; c < ivs_ip_count; c++) {
        uint32_t got;
        II_CHECK(ii_rig_set(&r, ic_word(a) | ic_word(b) | ic_word(c), 0u));
        got = ii_peek(r.x.m, IC_P0);
        if ((got & ~(uint32_t)pair_byte[a][b]) != 0u || (got & ~(uint32_t)pair_byte[a][c]) != 0u
            || (got & ~(uint32_t)pair_byte[b][c]) != 0u) {
          ivs_test_diag("closing %s, %s and %s set a bit that a pair of them had cleared (%02X)",
                        ivs_ip_ctls[a].name, ivs_ip_ctls[b].name, ivs_ip_ctls[c].name, (unsigned)got);
          goto done;
        }
      }
    }
  }
  rc = IVS_TEST_PASS;
done:
  ii_rig_close(&r);
  return rc;
}

static int ic_is(size_t k, const char *prefix)
{
  return strncmp(ivs_ip_ctls[k].name, prefix, strlen(prefix)) == 0;
}

/* 1 when the union of the cleared masks of the pair state {a, b} equals the union of some other state of
 * the universe: a single control, or a pair of two different controls other than {a, b}. */
static int ic_collides(size_t a, size_t b)
{
  uint32_t m = ic_cleared(a) | ic_cleared(b);
  size_t x;
  size_t y;
  for (x = 0u; x < ivs_ip_count; x++) {
    if (ic_cleared(x) == m) {
      return 1;
    }
    for (y = x + 1u; y < ivs_ip_count; y++) {
      int same_state = (x == a && y == b) || (x == b && y == a);
      if (!same_state && (ic_cleared(x) | ic_cleared(y)) == m) {
        return 1;
      }
    }
  }
  return 0;
}

/* The wiki statements about decodability as properties of the literal under the union rule. The set W
 * of states: idle, every single control, every disc + side pair; their bytes read through the machine
 * are pairwise different. The other classes are counted with ic_collides. */
static int ic_pairs_decodable_claims(void)
{
  uint16_t seen[256];
  size_t n_states = 0u;
  size_t a;
  size_t b;
  size_t key_side = 0u;
  size_t key_side_amb = 0u;
  size_t key_disc = 0u;
  size_t key_disc_amb = 0u;
  size_t side_side = 0u;
  size_t side_side_amb = 0u;
  ii_rig r;
  int rc = IVS_TEST_FAIL;
  memset(seen, 0, sizeof seen);
  if (!ii_rig_open(&r, IVS_MS_PROG_A)) {
    return IVS_TEST_FAIL;
  }
  /* W through the machine: every state's byte is new */
  {
    uint32_t words[256];
    size_t w;
    words[n_states++] = 0u; /* idle */
    for (a = 0u; a < ivs_ip_count; a++) {
      words[n_states++] = ic_word(a);
    }
    for (a = 0u; a < ivs_ip_count; a++) {
      for (b = 0u; b < ivs_ip_count; b++) {
        if (ic_is(a, "disc_") && ic_is(b, "side_")) {
          words[n_states++] = ic_word(a) | ic_word(b);
        }
      }
    }
    II_CHECK(n_states == 1u + 31u + 16u * 3u);
    for (w = 0u; w < n_states; w++) {
      uint32_t got;
      II_CHECK(ii_rig_set(&r, words[w], 0u));
      got = ii_peek(r.x.m, IC_P0);
      II_CHECK(got < 256u);
      if (seen[got] != 0u) {
        ivs_test_diag("two states of {idle, single, disc + side} read the same byte %02X", (unsigned)got);
        goto done;
      }
      seen[got] = 1u;
    }
  }
  for (a = 0u; a < ivs_ip_count; a++) {
    for (b = 0u; b < ivs_ip_count; b++) {
      if (ic_is(a, "key_") && ic_is(b, "side_")) {
        key_side++;
        key_side_amb += (size_t)ic_collides(a, b);
      } else if (ic_is(a, "key_") && ic_is(b, "disc_")) {
        key_disc++;
        key_disc_amb += (size_t)ic_collides(a, b);
      } else if (ic_is(a, "side_") && ic_is(b, "side_") && a < b) {
        side_side++;
        side_side_amb += (size_t)ic_collides(a, b);
      }
    }
  }
  /* The wiki's three negative statements, with the derived counts (36 of 36, 3 of 3, 180 of 192 at
   * plan 05-01; asserted as computed from the literal, tag source-expectation). */
  ivs_test_diag("derived: key+side %u of %u, side+side %u of %u, key+disc %u of %u ambiguous", (unsigned)key_side_amb,
                (unsigned)key_side, (unsigned)side_side_amb, (unsigned)side_side, (unsigned)key_disc_amb,
                (unsigned)key_disc);
  II_CHECK(key_side == 36u && side_side == 3u && key_disc == 192u);
  II_CHECK(key_side_amb == key_side && side_side_amb == side_side);
  II_CHECK(key_disc_amb == 180u);
  rc = IVS_TEST_PASS;
done:
  ii_rig_close(&r);
  return rc;
}

static const ivs_test_case ic_cases[] = {
    {"input.combo.key1-key9", ic_key1_key9, NULL},
    {"input.combo.key2-key8", ic_key2_key8, NULL},
    {"input.combo.clear-enter", ic_clear_enter, NULL},
    {"input.combo.discn-sidetop", ic_discn_sidetop, NULL},
    {"input.combo.disce-sidelowerleft", ic_disce_sidell, NULL},
    {"input.combo.discsw-sidelowerright", ic_discsw_sidelr, NULL},
    {"input.combo.discn-discnne", ic_discn_discnne, NULL},
    {"input.combo.disce-discese", ic_disce_discese, NULL},
    {"input.combo.discn-disce", ic_discn_disce, NULL},
    {"input.combo.discnne-discese", ic_discnne_discese, NULL},
    {"input.combo.opposite-n-s", ic_discn_discs, NULL},
    {"input.combo.opposite-e-w", ic_disce_discw2, NULL},
    {"input.combo.opposite-ne-sw", ic_discne_discsw, NULL},
    {"input.combo.opposite-nw-se", ic_discnw_discse, NULL},
    {"input.combo.sidetop-sidelowerleft", ic_sidetop_sidell, NULL},
    {"input.combo.sides-all", ic_sides_all, NULL},
    {"input.combo.key5-discn", ic_key5_discn, NULL},
    {"input.combo.key0-discw", ic_key0_discw, NULL},
    {"input.combo.key5-sidetop", ic_key5_sidetop, NULL},
    {"input.combo.keyenter-sidelowerright", ic_keyenter_sidelr, NULL},
    {"input.combo.three-key1-key9-discn", ic_key1_key9_discn, NULL},
    {"input.combo.three-discn-disce-discs", ic_discn_disce_discs, NULL},
    {"input.combo.all-closed", ic_all_closed, NULL},
    {"input.combo.ports-independent", ic_ports_independent, NULL},
    {"input.pairs.union", ic_pairs_union, NULL},
    {"input.pairs.commutative", ic_pairs_commutative, NULL},
    {"input.pairs.monotone-clear", ic_pairs_monotone_clear, NULL},
    {"input.pairs.decodable-claims", ic_pairs_decodable_claims, NULL},
};

II_TABLE_FAMILY(combo, ic_cases);
