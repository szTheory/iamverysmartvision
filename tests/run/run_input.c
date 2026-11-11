/* SPDX-License-Identifier: MIT */
/* Suite run, family input (D-12..D-16, A-04, A-08, plan 04-11): the input schedule of
 * ivs_machine_run (ivs/ivs_run.h) and the INPUT log (digest domain 18).
 *   run.input.event-size      ivs_input_event is 32 bytes with the documented offsets.
 *   run.input.one-event       one event with at_tick inside the third unit of stand-in content A
 *                             takes effect at the start of the fourth unit, never earlier, and the
 *                             INPUT row then holds the words, count 1, the floor and the literal
 *                             chain h1 below.
 * Stand-in content A of tests/machine/machine_standin.c (no rights-bearing byte); its unit
 * boundaries come from a seam-stepped twin, and the cases check that the literal tick lies where
 * they need it.
 *
 * Chain oracle (A-08, D-16). The literals below were produced at authoring time, outside the code
 * under test, with POSIX printf octal escapes and cmake -E sha256sum (cmake string(SHA256) and
 * file(WRITE) cannot carry NUL or raw hash bytes). Recipe, POSIX sh or dash, in a mktemp -d dir
 * (nothing is committed):
 *   oct() { h=$1; while [ -n "$h" ]; do b=${h%"${h#??}"}; h=${h#??}; printf '\\%03o' "0x$b"; done; }
 *   sha() { cmake -E sha256sum "$1" | cut -c1-64; }
 *   printf 'IVS-INPUT-LOG-V1' > h0.in; H0=$(sha h0.in)
 *   printf "$(oct "${H0}0000000000000046000020044000080000000000""00000000")" > h1.in; H1=$(sha h1.in)
 *   printf "$(oct "${H1}0000000000000046100000010000000000000000""00000000")" > h2.in; H2=$(sha h2.in)
 *   printf "$(oct "${H0}0000000000000046100000010000000000000000""00000000")" > g1.in; G1=$(sha g1.in)
 *   printf "$(oct "${G1}0000000000000046000020044000080000000000""00000000")" > g2.in; G2=$(sha g2.in)
 * Each 56-byte preimage is h(n) || be64(at_tick) || be32 of the four controller words: at_tick
 * 0x46 = 70; event A = {0x00002004, 0x40000800, 0, 0}, event B = {0x10000001, 0, 0, 0}.
 *   h0 = SHA-256("IVS-INPUT-LOG-V1")                  8a922f4b...da3a79dc
 *   h1 = after A at 70                                 304446b6...8e98267e
 *   h2 = after A then B, both at 70                    eb3b457e...cb91e229
 *   g2 = after B then A, both at 70                    bc0052b5...7444bcf6
 * The same h0 was also checked with shasum -a 256 over the 16 ASCII bytes. These literals check
 * the chain's framing and order, not hardware behaviour. */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common/ivs_sha256.h"
#include "ivs/ivs.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "run_cases.h"

#define IVS_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

#define RI_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      goto done;                                                                                       \
    }                                                                                                  \
  } while (0)

#define RI_POISON 0xA5u
#define RI_UNITS 12u
/* The literal tick (inside unit 3 of content A) and the two literal events. */
#define RI_TICK 70u

static const uint8_t ri_h0[32] = {0x8au, 0x92u, 0x2fu, 0x4bu, 0x64u, 0x8bu, 0x28u, 0x06u, 0x56u, 0x17u, 0x58u,
                                  0xcau, 0xf4u, 0xcfu, 0x26u, 0x14u, 0xf0u, 0x98u, 0x62u, 0x67u, 0x34u, 0x95u,
                                  0x46u, 0x86u, 0x0au, 0x23u, 0xcau, 0xc3u, 0xdau, 0x3au, 0x79u, 0xdcu};
static const uint8_t ri_h1[32] = {0x30u, 0x44u, 0x46u, 0xb6u, 0x14u, 0x98u, 0x8fu, 0x7cu, 0x29u, 0x07u, 0xa2u,
                                  0x71u, 0xcfu, 0xaau, 0xf2u, 0x74u, 0x55u, 0xafu, 0x48u, 0x0bu, 0xadu, 0xe2u,
                                  0x40u, 0x56u, 0xadu, 0x1bu, 0x68u, 0xd4u, 0x8eu, 0x98u, 0x26u, 0x7eu};
static const uint32_t ri_words_a[4] = {0x00002004u, 0x40000800u, 0u, 0u};

/* ---- helpers ---- */

static void ri_event(ivs_input_event *e, uint64_t at, const uint32_t w[4])
{
  memset(e, 0, sizeof *e);
  e->at_tick = at;
  memcpy(e->controller, w, sizeof e->controller);
}

/* One run call with events; the report is poisoned first (separate statements). */
static ivs_status ri_run(ivs_machine *m, uint32_t flags, uint64_t target, const ivs_input_event *ev, uint32_t n,
                         ivs_run_report *rep)
{
  ivs_run_request q;
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.flags = flags;
  q.target_ticks = target;
  q.events = ev;
  q.event_count = n;
  memset(rep, RI_POISON, sizeof *rep);
  rep->struct_size = (uint32_t)sizeof *rep;
  return ivs_machine_run(m, &q, rep);
}

/* Machine time of m (info), or UINT64_MAX when info refuses. */
static uint64_t ri_now(const ivs_machine *m)
{
  ivs_info info;
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info(m, &info) != IVS_OK) {
    ivs_test_diag("info refused");
    return UINT64_MAX;
  }
  return info.time_ticks;
}

/* The boundaries of content A from a seam-stepped twin: b[0] = 0, b[i] after i units. */
static int ri_bounds(uint64_t b[RI_UNITS + 1u])
{
  ivs_ms_inst y;
  uint32_t i;
  int ok = 0;
  if (!ivs_ms_powered(&y, IVS_MS_CONTENT_A, RI_POISON)) {
    return 0;
  }
  b[0] = ri_now(y.m);
  for (i = 1u; i <= RI_UNITS; i++) {
    if (!ivs_ms_step(y.m, 1u, NULL)) {
      goto done;
    }
    b[i] = ri_now(y.m);
    if (b[i] <= b[i - 1u] || b[i] == UINT64_MAX) {
      goto done;
    }
  }
  ok = b[0] == 0u;
done:
  ivs_ms_free(&y);
  return ok;
}

/* 1 when an accepted report carries the input fields given (and IVS_INPUT_INDEX_NONE). */
static int ri_input_report(const ivs_run_report *r, uint32_t consumed, uint64_t floor, const uint32_t words[4])
{
  if (r->refusal_reason != IVS_REASON_NONE || r->input_consumed != consumed ||
      r->input_failing_index != IVS_INPUT_INDEX_NONE || r->input_floor_tick != floor ||
      memcmp(r->controller_now, words, sizeof r->controller_now) != 0) {
    ivs_test_diag("report input consumed %u index %u floor %llu words %08x %08x %08x %08x; want %u floor %llu words "
                  "%08x %08x %08x %08x",
                  (unsigned)r->input_consumed, (unsigned)r->input_failing_index,
                  (unsigned long long)r->input_floor_tick, (unsigned)r->controller_now[0],
                  (unsigned)r->controller_now[1], (unsigned)r->controller_now[2], (unsigned)r->controller_now[3],
                  (unsigned)consumed, (unsigned long long)floor, (unsigned)words[0], (unsigned)words[1],
                  (unsigned)words[2], (unsigned)words[3]);
    return 0;
  }
  return 1;
}

/* Appends n bytes of v big-endian at p[*k]. */
static void ri_be(uint8_t *p, size_t *k, uint64_t v, uint32_t n)
{
  uint32_t i;
  for (i = 0u; i < n; i++) {
    p[*k + i] = (uint8_t)(v >> (8u * (n - 1u - i)));
  }
  *k += n;
}

/* 1 when the INPUT row of m serializes to exactly the preimage built here from the expected
 * words, count, floor and literal chain (machine.h framing: "IVSD" || u8 18 || u16 1 || u32 4 ||
 * four u32 words || u64 count || u64 floor || u32 32 || chain), and the public INPUT digest is the
 * SHA-256 of that preimage. */
static int ri_input_row(const ivs_machine *m, const uint32_t words[4], uint64_t count, uint64_t floor,
                        const uint8_t chain[32])
{
  uint8_t want[96];
  uint8_t got[96];
  uint8_t dw[32];
  uint8_t dg[32];
  size_t k = 0u;
  size_t len;
  uint32_t i;
  want[k++] = 'I';
  want[k++] = 'V';
  want[k++] = 'S';
  want[k++] = 'D';
  ri_be(want, &k, IVS_DIGEST_INPUT, 1u);
  ri_be(want, &k, 1u, 2u);
  ri_be(want, &k, 4u, 4u);
  for (i = 0u; i < 4u; i++) {
    ri_be(want, &k, words[i], 4u);
  }
  ri_be(want, &k, count, 8u);
  ri_be(want, &k, floor, 8u);
  ri_be(want, &k, 32u, 4u);
  memcpy(want + k, chain, 32u);
  k += 32u;
  len = ivs_machine_digest_preimage((const struct ivs_machine *)m, IVS_DIGEST_INPUT, got, sizeof got);
  if (len != k || memcmp(got, want, k) != 0) {
    ivs_test_diag("INPUT row preimage differs from the expected row (length %u, want %u)", (unsigned)len,
                  (unsigned)k);
    return 0;
  }
  ivs_sha256_digest(want, k, dw);
  if (ivs_machine_state_digest(m, IVS_DIGEST_INPUT, dg) != IVS_OK || memcmp(dg, dw, 32u) != 0) {
    ivs_test_diag("INPUT digest is not the SHA-256 of the expected row");
    return 0;
  }
  return 1;
}

/* ---- cases ---- */

/* The event record is 32 bytes: at_tick at 0, flags 4 bytes at 8, reserved at 12, controller[4]
 * at 16. */
static int ri_event_size(void)
{
  IVS_CHECK(sizeof(ivs_input_event) == 32u);
  IVS_CHECK(offsetof(ivs_input_event, at_tick) == 0u);
  IVS_CHECK(offsetof(ivs_input_event, flags) == 8u);
  IVS_CHECK(offsetof(ivs_input_event, reserved) == 12u);
  IVS_CHECK(offsetof(ivs_input_event, controller) == 16u);
  IVS_CHECK(IVS_INPUT_CONTROLLER_WORDS == 4u);
  return IVS_TEST_PASS;
}

/* Event A at tick 70, strictly inside unit 3 (b[2] < 70 < b[3]), resubmitted until consumed:
 * single steps over units 1 and 2 and a call whose target 71 also lies inside unit 3 leave it
 * unconsumed (words 0, chain h0); the call that begins unit 4 at b[3] applies it first (consumed 1,
 * words A, floor b[3] + 1, chain h1); a later call without events keeps the words in force. */
static int ri_one_event(void)
{
  static const uint32_t zero[4] = {0u, 0u, 0u, 0u};
  uint64_t b[RI_UNITS + 1u];
  ivs_input_event ev;
  ivs_ms_inst x;
  ivs_run_report r;
  ivs_status st;
  uint32_t k;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ri_bounds(b));
  IVS_CHECK(b[2] < RI_TICK && RI_TICK + 1u < b[3]);
  ri_event(&ev, RI_TICK, ri_words_a);
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
  /* After reset: the row holds h0 only. */
  RI_CHECK(ri_input_row(x.m, zero, 0u, 0u, ri_h0));
  /* Units 1 and 2, one call each (run(now + 1)). */
  for (k = 0u; k < 2u; k++) {
    st = ri_run(x.m, 0u, b[k] + 1u, &ev, 1u, &r);
    RI_CHECK(st == IVS_OK && r.now_ticks == b[k + 1u] && r.units_executed == 1u);
    RI_CHECK(ri_input_report(&r, 0u, b[k] + 1u, zero));
  }
  /* A target inside unit 3 (after the event's tick): unit 3 begins at b[2] < 70, so nothing is
   * applied, and nothing is applied at exit either. */
  st = ri_run(x.m, 0u, RI_TICK + 1u, &ev, 1u, &r);
  RI_CHECK(st == IVS_OK && r.now_ticks == b[3] && r.units_executed == 1u);
  RI_CHECK(ri_input_report(&r, 0u, b[2] + 1u, zero));
  RI_CHECK(ri_input_row(x.m, zero, 0u, b[2] + 1u, ri_h0));
  /* Unit 4 begins at b[3] >= 70: the event takes effect there. */
  st = ri_run(x.m, 0u, b[3] + 1u, &ev, 1u, &r);
  RI_CHECK(st == IVS_OK && r.now_ticks == b[4] && r.units_executed == 1u);
  RI_CHECK(ri_input_report(&r, 1u, b[3] + 1u, ri_words_a));
  RI_CHECK(ri_input_row(x.m, ri_words_a, 1u, b[3] + 1u, ri_h1));
  /* The suffix is empty now; the words stay in force. */
  st = ri_run(x.m, 0u, b[4] + 1u, NULL, 0u, &r);
  RI_CHECK(st == IVS_OK && r.now_ticks == b[5]);
  RI_CHECK(ri_input_report(&r, 0u, b[4] + 1u, ri_words_a));
  RI_CHECK(ri_input_row(x.m, ri_words_a, 1u, b[4] + 1u, ri_h1));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

const ivs_test_case ivs_run_input_cases[] = {
    {"run.input.event-size", ri_event_size, NULL},
    {"run.input.one-event", ri_one_event, NULL},
};

const size_t ivs_run_input_count = sizeof ivs_run_input_cases / sizeof ivs_run_input_cases[0];
