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
static const uint8_t ri_h2[32] = {0xebu, 0x3bu, 0x45u, 0x7eu, 0x4du, 0xa0u, 0x45u, 0x83u, 0xdfu, 0x12u, 0x91u,
                                  0x61u, 0xa3u, 0xbdu, 0x41u, 0x3eu, 0x96u, 0x9eu, 0x7cu, 0xefu, 0x82u, 0xe2u,
                                  0xedu, 0x83u, 0x82u, 0x9fu, 0x42u, 0x23u, 0xcbu, 0x91u, 0xe2u, 0x29u};
static const uint8_t ri_g2[32] = {0xbcu, 0x00u, 0x52u, 0xb5u, 0x3cu, 0xb3u, 0x20u, 0xd6u, 0x13u, 0xc6u, 0xd1u,
                                  0x4du, 0x33u, 0xe7u, 0x85u, 0xc6u, 0xa1u, 0x13u, 0x6du, 0x3cu, 0xa9u, 0xd1u,
                                  0x80u, 0xe5u, 0x80u, 0xeau, 0xf8u, 0x47u, 0x74u, 0x44u, 0xbcu, 0xf6u};
static const uint32_t ri_words_a[4] = {0x00002004u, 0x40000800u, 0u, 0u};
static const uint32_t ri_words_b[4] = {0x10000001u, 0u, 0u, 0u};
static const uint32_t ri_zero[4] = {0u, 0u, 0u, 0u};

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

/* ---- refusals (D-05, D-13, D-14) ---- */

#define RI_REP_BYTES 128u

typedef union ri_rep {
  ivs_run_report r;
  uint8_t b[RI_REP_BYTES];
} ri_rep;

static void ri_req(ivs_run_request *q, uint32_t flags, uint64_t target, const ivs_input_event *ev, uint32_t n)
{
  memset(q, 0, sizeof *q);
  q->struct_size = (uint32_t)sizeof *q;
  q->flags = flags;
  q->target_ticks = target;
  q->events = ev;
  q->event_count = n;
}

/* 1 when byte i of the report lies in the field at off of width w. */
static int ri_in(size_t i, size_t off, size_t w)
{
  return i >= off && i < off + w;
}

/* One refused call with the SP4 oracle: status want; the report holds struct_size, refusal_reason
 * reason and, when index is not IVS_INPUT_INDEX_NONE, input_failing_index index, with every other
 * byte (controller_now included) still poison. The instance storage, its digests and time are unchanged (snapshot of x, live). The call goes
 * through the env seam when env is not NULL. */
static int ri_refused(const ivs_ms_inst *x, ivs_machine *m, const ivs_run_request *q, const ivs_run_env *env,
                      ivs_status want, uint32_t reason, uint32_t index)
{
  ivs_run_snap s;
  ri_rep u;
  ivs_status st;
  size_t i;
  int ok = 0;
  memset(&u, RI_POISON, sizeof u);
  u.r.struct_size = (uint32_t)sizeof(ivs_run_report);
  if (!ivs_run_snap_take(&s, x, 1)) {
    ivs_run_snap_free(&s);
    return 0;
  }
  st = env != NULL ? ivs_machine_run_env_internal(m, q, &u.r, env) : ivs_machine_run(m, q, &u.r);
  if (st != want) {
    ivs_test_diag("status %s, want %s (reason %u)", ivs_status_name(st), ivs_status_name(want),
                  (unsigned)u.r.refusal_reason);
    goto done;
  }
  if (u.r.struct_size != (uint32_t)sizeof(ivs_run_report) || u.r.refusal_reason != reason) {
    ivs_test_diag("report struct_size %u refusal_reason %u, want %u", (unsigned)u.r.struct_size,
                  (unsigned)u.r.refusal_reason, (unsigned)reason);
    goto done;
  }
  if (index != IVS_INPUT_INDEX_NONE && u.r.input_failing_index != index) {
    ivs_test_diag("input_failing_index %u, want %u", (unsigned)u.r.input_failing_index, (unsigned)index);
    goto done;
  }
  for (i = 4u; i < RI_REP_BYTES; i++) {
    if (ri_in(i, offsetof(ivs_run_report, refusal_reason), 4u) ||
        (index != IVS_INPUT_INDEX_NONE && ri_in(i, offsetof(ivs_run_report, input_failing_index), 4u))) {
      continue;
    }
    if (u.b[i] != RI_POISON) {
      ivs_test_diag("report byte %u written by a refused call", (unsigned)i);
      goto done;
    }
  }
  ok = ivs_run_snap_same(&s, x);
done:
  ivs_run_snap_free(&s);
  return ok;
}

/* A powered instance of content A run to b[2] (two units; floor b[1] + 1) with no event. */
static int ri_powered_two(ivs_ms_inst *x, uint64_t b[RI_UNITS + 1u])
{
  ivs_run_report r;
  if (!ri_bounds(b) || !ivs_ms_powered(x, IVS_MS_CONTENT_A, RI_POISON)) {
    return 0;
  }
  if (ri_run(x->m, 0u, b[2], NULL, 0u, &r) != IVS_OK || r.now_ticks != b[2] || r.input_floor_tick != b[1] + 1u) {
    ivs_test_diag("setup run to b[2] failed");
    ivs_ms_free(x);
    return 0;
  }
  return 1;
}

/* An offset inside the instance, 16-aligned, in cartridge backing words no stand-in segment uses. */
static size_t ri_inside_off(void)
{
  size_t off = offsetof(struct ivs_machine, mem) + offsetof(ivs_memsys, cart) + 2u * 0x6000u;
  return (off + 15u) & ~(size_t)15u;
}

/* BAD_ARG (INPUT_EVENTS_NULL): events NULL with event_count 1, 65537 and UINT32_MAX (the NULL
 * check comes first). */
static int ri_refuse_events_null(void)
{
  static const uint32_t counts[3] = {1u, IVS_INPUT_EVENTS_MAX + 1u, UINT32_MAX};
  ivs_ms_inst x;
  ivs_run_request q;
  uint32_t k;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
  for (k = 0u; k < 3u; k++) {
    ri_req(&q, 0u, 1000u, NULL, counts[k]);
    RI_CHECK(ri_refused(&x, x.m, &q, NULL, IVS_E_BAD_ARG, IVS_REASON_INPUT_EVENTS_NULL, IVS_INPUT_INDEX_NONE));
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* BAD_ARG (INPUT_EVENT_COUNT): event_count 65537 and UINT32_MAX with a non-NULL array (never read).
 * The report gets the detail wherever it lies relative to the array: a count above the cap names
 * no span (found on gcc/Linux, where the stack report lay inside 2^32 x 32 bytes from &ev). */
static int ri_refuse_count_cap(void)
{
  ivs_input_event ev;
  ivs_ms_inst x;
  ivs_run_request q;
  int rc = IVS_TEST_FAIL;
  ri_event(&ev, 0u, ri_zero);
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
  ri_req(&q, 0u, 1000u, &ev, IVS_INPUT_EVENTS_MAX + 1u);
  RI_CHECK(ri_refused(&x, x.m, &q, NULL, IVS_E_BAD_ARG, IVS_REASON_INPUT_EVENT_COUNT, IVS_INPUT_INDEX_NONE));
  ri_req(&q, 0u, 1000u, &ev, UINT32_MAX);
  RI_CHECK(ri_refused(&x, x.m, &q, NULL, IVS_E_BAD_ARG, IVS_REASON_INPUT_EVENT_COUNT, IVS_INPUT_INDEX_NONE));
  /* The report placed right after the array, inside what 65537 events would cover: detail still
   * written, on every data layout. */
  {
    struct {
      ivs_input_event ev;
      ivs_run_report rep;
    } pair;
    memset(&pair, 0, sizeof pair);
    pair.rep.struct_size = (uint32_t)sizeof pair.rep;
    ri_req(&q, 0u, 1000u, &pair.ev, IVS_INPUT_EVENTS_MAX + 1u);
    RI_CHECK(ivs_machine_run(x.m, &q, &pair.rep) == IVS_E_BAD_ARG);
    RI_CHECK(pair.rep.refusal_reason == IVS_REASON_INPUT_EVENT_COUNT);
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* 65536 events (the cap) with valid ticks 0..65535 are accepted: a run to b[4] applies exactly the
 * events with at_tick <= b[3] (the start of the last begun unit), in order. */
static int ri_count_cap_ok(void)
{
  uint64_t b[RI_UNITS + 1u];
  ivs_input_event *ev = NULL;
  ivs_ms_inst x;
  ivs_run_report r;
  ivs_status st;
  uint32_t i;
  uint32_t last[4];
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ri_bounds(b));
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
  ev = malloc((size_t)IVS_INPUT_EVENTS_MAX * sizeof *ev);
  RI_CHECK(ev != NULL);
  for (i = 0u; i < IVS_INPUT_EVENTS_MAX; i++) {
    uint32_t w[4] = {i, i ^ 0x7FFFFFFFu, 0u, 0u};
    ri_event(&ev[i], i, w);
  }
  st = ri_run(x.m, 0u, b[4], ev, IVS_INPUT_EVENTS_MAX, &r);
  RI_CHECK(st == IVS_OK && r.now_ticks == b[4]);
  memcpy(last, ev[b[3]].controller, sizeof last);
  RI_CHECK(ri_input_report(&r, (uint32_t)b[3] + 1u, b[3] + 1u, last));
  rc = IVS_TEST_PASS;
done:
  free(ev);
  ivs_ms_free(&x);
  return rc;
}

/* BAD_ARG (INPUT_EVENTS_OVERLAP): an event array inside the instance storage, and one straddling
 * its last bytes; with a host allocation [64 bytes | instance | 64 bytes], an array ending exactly
 * where the instance begins and one starting exactly where it ends run, and one 16 bytes further
 * in (either side) is refused. */
static int ri_refuse_inside_instance(void)
{
  ivs_input_event ev[2];
  ivs_ms_inst x;
  ivs_run_request q;
  ivs_run_report r;
  ivs_create_options o;
  uint8_t *big = NULL;
  ivs_machine *bm = NULL;
  const ivs_input_event *at;
  ivs_status st;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
  at = (const ivs_input_event *)(const void *)(x.storage + ri_inside_off());
  ri_req(&q, 0u, 1000u, at, 1u);
  RI_CHECK(ri_refused(&x, x.m, &q, NULL, IVS_E_BAD_ARG, IVS_REASON_INPUT_EVENTS_OVERLAP, IVS_INPUT_INDEX_NONE));
  at = (const ivs_input_event *)(const void *)(x.storage + IVS_MACHINE_BYTES - 16u);
  ri_req(&q, 0u, 1000u, at, 1u);
  RI_CHECK(ri_refused(&x, x.m, &q, NULL, IVS_E_BAD_ARG, IVS_REASON_INPUT_EVENTS_OVERLAP, IVS_INPUT_INDEX_NONE));
  memset(&o, 0, sizeof o);
  o.struct_size = (uint32_t)sizeof o;
  o.profile_id = IVS_MS_PROFILE;
  big = aligned_alloc(IVS_MACHINE_ALIGN, 64u + IVS_MACHINE_BYTES + 64u);
  RI_CHECK(big != NULL);
  memset(big, 0, 64u + IVS_MACHINE_BYTES + 64u);
  RI_CHECK(ivs_machine_create(&o, big + 64u, IVS_MACHINE_BYTES, &bm) == IVS_OK);
  RI_CHECK(ivs_ms_content(bm, IVS_MS_CONTENT_A) && ivs_machine_reset(bm, IVS_RESET_POWER_ON) == IVS_OK);
  /* Zero-filled events (tick 0, all words 0) before and after the instance: accepted. */
  ri_req(&q, 0u, 100u, (const ivs_input_event *)(const void *)big, 2u);
  memset(&r, RI_POISON, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  st = ivs_machine_run(bm, &q, &r);
  RI_CHECK(st == IVS_OK && r.input_consumed == 2u);
  ri_req(&q, 0u, 200u, (const ivs_input_event *)(const void *)(big + 64u + IVS_MACHINE_BYTES), 2u);
  /* The floor is above 0 now; tick 0 events would be BEFORE_FLOOR, so give them a later tick. */
  memset(ev, 0, sizeof ev);
  ev[0].at_tick = 150u;
  ev[1].at_tick = 150u;
  memcpy(big + 64u + IVS_MACHINE_BYTES, ev, sizeof ev);
  memset(&r, RI_POISON, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  st = ivs_machine_run(bm, &q, &r);
  RI_CHECK(st == IVS_OK && r.input_consumed == 2u);
  /* 16 bytes further in, either side. */
  ri_req(&q, 0u, 300u, (const ivs_input_event *)(const void *)(big + 16u), 2u);
  memset(&r, RI_POISON, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  st = ivs_machine_run(bm, &q, &r);
  RI_CHECK(st == IVS_E_BAD_ARG && r.refusal_reason == IVS_REASON_INPUT_EVENTS_OVERLAP);
  ri_req(&q, 0u, 300u, (const ivs_input_event *)(const void *)(big + 64u + IVS_MACHINE_BYTES - 16u), 2u);
  memset(&r, RI_POISON, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  st = ivs_machine_run(bm, &q, &r);
  RI_CHECK(st == IVS_E_BAD_ARG && r.refusal_reason == IVS_REASON_INPUT_EVENTS_OVERLAP);
  rc = IVS_TEST_PASS;
done:
  free(big);
  ivs_ms_free(&x);
  return rc;
}

/* BAD_ARG (INPUT_EVENTS_OVERLAP): the event array shares bytes with the report (the report starts
 * inside the array, and the array starts inside the report). The report is part of the cause, so
 * the call writes no byte of it (and so none of the array); the instance is unchanged. Positive
 * control: the report right after the array runs. */
static int ri_refuse_overlap_report(void)
{
  union {
    ivs_input_event ev[8];
    uint8_t b[256];
  } mem;
  uint8_t keep[256];
  ivs_ms_inst x;
  ivs_run_snap s;
  ivs_run_request q;
  ivs_run_report *rep;
  ivs_status st;
  uint32_t k;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
  s.copy = NULL;
  for (k = 0u; k < 2u; k++) {
    memset(&mem, 0, sizeof mem);
    /* k 0: three events at 0..95, the report at byte 64 (inside event 2); k 1: the report at 0,
     * one event at byte 96 (inside the report's 112 bytes). */
    rep = (ivs_run_report *)(void *)(mem.b + (k == 0u ? 64u : 0u));
    rep->struct_size = (uint32_t)sizeof *rep;
    ri_req(&q, 0u, 1000u, k == 0u ? &mem.ev[0] : &mem.ev[3], k == 0u ? 3u : 1u);
    memcpy(keep, mem.b, sizeof keep);
    RI_CHECK(ivs_run_snap_take(&s, &x, 1));
    st = ivs_machine_run(x.m, &q, rep);
    RI_CHECK(st == IVS_E_BAD_ARG);
    RI_CHECK(memcmp(keep, mem.b, sizeof keep) == 0);
    RI_CHECK(ivs_run_snap_same(&s, &x));
    ivs_run_snap_free(&s);
  }
  /* Adjacent: three events at 0..95, the report at 96. */
  memset(&mem, 0, sizeof mem);
  rep = (ivs_run_report *)(void *)(mem.b + 96u);
  rep->struct_size = (uint32_t)sizeof *rep;
  ri_req(&q, 0u, 1000u, &mem.ev[0], 3u);
  st = ivs_machine_run(x.m, &q, rep);
  RI_CHECK(st == IVS_OK && rep->input_consumed == 3u);
  rc = IVS_TEST_PASS;
done:
  ivs_run_snap_free(&s);
  ivs_ms_free(&x);
  return rc;
}

/* BAD_ARG (INPUT_EVENTS_OVERLAP): the event array shares bytes with the video buffer (video, or
 * audio when audio is 1). The buffers come from one poisoned arena whose bytes no refusal may
 * change; the array sits in it too (a 0xA5 event is never read: the slot refuses first). Positive
 * control: the array right after the buffer runs (zeroed events). Plan 05-06: the real audio layout
 * needs 15,060 bytes per frame, so the audio case uses a 15,060-byte buffer that ends where the
 * array begins (the old 48-byte buffer is too small against the real layout: its positive control
 * was BUFFER_TOO_SMALL); the video layout is NONE and the old geometry stands. */
#define RI_AUDIO_NEED 15060u
static int ri_refuse_overlap_buffer(int audio)
{
  ivs_ms_inst x;
  ivs_run_request q;
  ivs_run_report r;
  uint8_t *a = NULL;
  uint8_t *keep = NULL;
  size_t arena = audio ? (size_t)RI_AUDIO_NEED + 128u : 256u;
  size_t arr = audio ? (size_t)RI_AUDIO_NEED + 4u : 64u; /* 8-aligned: the array's first byte */
  ivs_status st;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
  a = aligned_alloc(16u, (arena + 15u) & ~(size_t)15u);
  keep = malloc(arena);
  RI_CHECK(a != NULL && keep != NULL);
  memset(a, RI_POISON, arena);
  memcpy(keep, a, arena);
  ri_req(&q, 0u, 1000u, (const ivs_input_event *)(const void *)(a + arr), 2u);
  if (audio) {
    q.audio = a + 4u;
    q.audio_capacity = (uint64_t)RI_AUDIO_NEED + 1u; /* its last byte is the array's first */
  } else {
    q.video = a + 112u;
    q.video_capacity = 16u; /* inside the array's 64 bytes */
  }
  RI_CHECK(ri_refused(&x, x.m, &q, NULL, IVS_E_BAD_ARG, IVS_REASON_INPUT_EVENTS_OVERLAP, IVS_INPUT_INDEX_NONE));
  RI_CHECK(memcmp(keep, a, arena) == 0);
  /* Adjacent: the buffer ends where the array begins. */
  memset(a + arr, 0, 64u);
  if (audio) {
    q.audio_capacity = RI_AUDIO_NEED;
  } else {
    q.video = a + 128u;
  }
  memset(&r, RI_POISON, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  st = ivs_machine_run(x.m, &q, &r);
  RI_CHECK(st == IVS_OK && r.input_consumed == 2u);
  rc = IVS_TEST_PASS;
done:
  free(keep);
  free(a);
  ivs_ms_free(&x);
  return rc;
}

static int ri_refuse_overlap_video(void)
{
  return ri_refuse_overlap_buffer(0);
}

static int ri_refuse_overlap_audio(void)
{
  return ri_refuse_overlap_buffer(1);
}

/* UNSUPPORTED (INPUT_RESERVED_IN): reserved_in 1 and 0x80000000, with and without events. */
static int ri_refuse_reserved_in(void)
{
  static const uint32_t bad[2] = {1u, 0x80000000u};
  ivs_input_event ev;
  ivs_ms_inst x;
  ivs_run_request q;
  uint32_t k;
  int rc = IVS_TEST_FAIL;
  ri_event(&ev, 0u, ri_words_a);
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
  for (k = 0u; k < 2u; k++) {
    ri_req(&q, 0u, 1000u, NULL, 0u);
    q.reserved_in = bad[k];
    RI_CHECK(ri_refused(&x, x.m, &q, NULL, IVS_E_UNSUPPORTED, IVS_REASON_INPUT_RESERVED_IN, IVS_INPUT_INDEX_NONE));
    ri_req(&q, 0u, 1000u, &ev, 1u);
    q.reserved_in = bad[k];
    RI_CHECK(ri_refused(&x, x.m, &q, NULL, IVS_E_UNSUPPORTED, IVS_REASON_INPUT_RESERVED_IN, IVS_INPUT_INDEX_NONE));
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* One schedule refused at index want_index with status want and reason, the bad event placed by
 * the case; the instance (after two units, floor b[1] + 1) is unchanged. */
static int ri_refuse_schedule(const ivs_input_event *ev, uint32_t n, ivs_status want, uint32_t reason,
                              uint32_t want_index)
{
  uint64_t b[RI_UNITS + 1u];
  ivs_ms_inst x;
  ivs_run_request q;
  int ok;
  if (!ri_powered_two(&x, b)) {
    return 0;
  }
  ri_req(&q, 0u, b[6], ev, n);
  ok = ri_refused(&x, x.m, &q, NULL, want, reason, want_index);
  ivs_ms_free(&x);
  return ok;
}

/* Ticks used by the schedule refusals: all at or above the floor b[1] + 1 = 21 after two units. */
#define RI_T0 60u

/* BAD_ARG (INPUT_UNSORTED): a tick below its predecessor, at index 1 of two and as the last of
 * five events. */
static int ri_refuse_unsorted(void)
{
  ivs_input_event ev[5];
  uint32_t i;
  for (i = 0u; i < 5u; i++) {
    ri_event(&ev[i], RI_T0 + 10u * i, ri_words_a);
  }
  ev[1].at_tick = RI_T0 - 1u;
  IVS_CHECK(ri_refuse_schedule(ev, 2u, IVS_E_BAD_ARG, IVS_REASON_INPUT_UNSORTED, 1u));
  ev[1].at_tick = RI_T0 + 10u;
  ev[4].at_tick = ev[3].at_tick - 1u;
  IVS_CHECK(ri_refuse_schedule(ev, 5u, IVS_E_BAD_ARG, IVS_REASON_INPUT_UNSORTED, 4u));
  return IVS_TEST_PASS;
}

/* BAD_ARG (INPUT_BEFORE_FLOOR): after a call that began units 1 and 2 (floor b[1] + 1), an event at
 * b[1] (one below the floor) and at 0; an event exactly at the floor is accepted and applied at
 * the next unit. */
static int ri_refuse_below_floor(void)
{
  uint64_t b[RI_UNITS + 1u];
  ivs_input_event ev[2];
  ivs_ms_inst x;
  ivs_run_request q;
  ivs_run_report r;
  ivs_status st;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ri_powered_two(&x, b));
  ri_event(&ev[0], b[1], ri_words_a);
  ri_req(&q, 0u, b[6], ev, 1u);
  RI_CHECK(ri_refused(&x, x.m, &q, NULL, IVS_E_BAD_ARG, IVS_REASON_INPUT_BEFORE_FLOOR, 0u));
  ri_event(&ev[0], 0u, ri_words_a);
  ri_event(&ev[1], b[5], ri_words_b);
  ri_req(&q, 0u, b[6], ev, 2u);
  RI_CHECK(ri_refused(&x, x.m, &q, NULL, IVS_E_BAD_ARG, IVS_REASON_INPUT_BEFORE_FLOOR, 0u));
  ri_event(&ev[0], b[1] + 1u, ri_words_a);
  st = ri_run(x.m, 0u, b[3], ev, 1u, &r);
  RI_CHECK(st == IVS_OK && ri_input_report(&r, 1u, b[2] + 1u, ri_words_a));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* BAD_ARG (INPUT_TICK_RANGE): at_tick 2^63 and UINT64_MAX, alone and as the last of three sorted
 * events; 2^63 - 1 is accepted (never reached here, so it stays unconsumed). */
static int ri_refuse_tick_range(void)
{
  uint64_t b[RI_UNITS + 1u];
  ivs_input_event ev[3];
  ivs_ms_inst x;
  ivs_run_report r;
  ivs_status st;
  int rc = IVS_TEST_FAIL;
  ri_event(&ev[0], RI_T0, ri_words_a);
  ri_event(&ev[1], RI_T0 + 1u, ri_words_b);
  ri_event(&ev[2], IVS_RUN_TARGET_MAX, ri_words_a);
  IVS_CHECK(ri_refuse_schedule(&ev[2], 1u, IVS_E_BAD_ARG, IVS_REASON_INPUT_TICK_RANGE, 0u));
  IVS_CHECK(ri_refuse_schedule(ev, 3u, IVS_E_BAD_ARG, IVS_REASON_INPUT_TICK_RANGE, 2u));
  ev[2].at_tick = UINT64_MAX;
  IVS_CHECK(ri_refuse_schedule(ev, 3u, IVS_E_BAD_ARG, IVS_REASON_INPUT_TICK_RANGE, 2u));
  ev[2].at_tick = IVS_RUN_TARGET_MAX - 1u;
  IVS_CHECK(ri_powered_two(&x, b));
  st = ri_run(x.m, 0u, b[6], ev, 3u, &r);
  RI_CHECK(st == IVS_OK && ri_input_report(&r, 2u, b[5] + 1u, ri_words_b));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* UNSUPPORTED (INPUT_CONTROL_RESERVED): bit 31 in word 0 of the only event, and in word 3 of the
 * last of three. */
static int ri_refuse_control_reserved(void)
{
  ivs_input_event ev[3];
  uint32_t i;
  for (i = 0u; i < 3u; i++) {
    ri_event(&ev[i], RI_T0 + i, ri_words_a);
  }
  ev[0].controller[0] |= IVS_CTRL_RESERVED_MASK;
  IVS_CHECK(ri_refuse_schedule(ev, 1u, IVS_E_UNSUPPORTED, IVS_REASON_INPUT_CONTROL_RESERVED, 0u));
  ev[0].controller[0] = ri_words_a[0];
  ev[2].controller[3] = IVS_CTRL_RESERVED_MASK;
  IVS_CHECK(ri_refuse_schedule(ev, 3u, IVS_E_UNSUPPORTED, IVS_REASON_INPUT_CONTROL_RESERVED, 2u));
  return IVS_TEST_PASS;
}

/* UNSUPPORTED (INPUT_EVENT_FLAGS): flags 1 and 0x80000000 on the last of three events. */
static int ri_refuse_event_flags(void)
{
  ivs_input_event ev[3];
  uint32_t i;
  for (i = 0u; i < 3u; i++) {
    ri_event(&ev[i], RI_T0 + i, ri_words_a);
  }
  ev[2].flags = 1u;
  IVS_CHECK(ri_refuse_schedule(ev, 3u, IVS_E_UNSUPPORTED, IVS_REASON_INPUT_EVENT_FLAGS, 2u));
  ev[2].flags = 0x80000000u;
  IVS_CHECK(ri_refuse_schedule(ev, 3u, IVS_E_UNSUPPORTED, IVS_REASON_INPUT_EVENT_FLAGS, 2u));
  return IVS_TEST_PASS;
}

/* UNSUPPORTED (INPUT_EVENT_RESERVED): a nonzero reserved word on the only event and on the last of
 * three. */
static int ri_refuse_event_reserved(void)
{
  ivs_input_event ev[3];
  uint32_t i;
  for (i = 0u; i < 3u; i++) {
    ri_event(&ev[i], RI_T0 + i, ri_words_a);
  }
  ev[0].reserved = 1u;
  IVS_CHECK(ri_refuse_schedule(ev, 1u, IVS_E_UNSUPPORTED, IVS_REASON_INPUT_EVENT_RESERVED, 0u));
  ev[0].reserved = 0u;
  ev[2].reserved = 0xFFFFFFFFu;
  IVS_CHECK(ri_refuse_schedule(ev, 3u, IVS_E_UNSUPPORTED, IVS_REASON_INPUT_EVENT_RESERVED, 2u));
  return IVS_TEST_PASS;
}

/* UNSUPPORTED (INPUT_EXTRA_CONTROLLER, plan 04-12, D-13, D-15): the profile connects
 * input.controllers = 2 controllers, so a nonzero word 2 or 3 is refused before any write with the
 * failing index: word 2 on the only event, word 3 on the last of three, and word 2 with a later
 * nonzero flags (lowest index wins). A word with bit 31 set stays INPUT_CONTROL_RESERVED (that
 * check comes first per event); an earlier BAD_ARG event wins over the UNSUPPORTED pass. */
static int ri_refuse_extra_controller(void)
{
  ivs_input_event ev[3];
  uint32_t i;
  for (i = 0u; i < 3u; i++) {
    ri_event(&ev[i], RI_T0 + i, ri_words_a);
  }
  ev[0].controller[2] = 1u;
  IVS_CHECK(ri_refuse_schedule(ev, 1u, IVS_E_UNSUPPORTED, IVS_REASON_INPUT_EXTRA_CONTROLLER, 0u));
  ev[1].flags = 1u;
  IVS_CHECK(ri_refuse_schedule(ev, 3u, IVS_E_UNSUPPORTED, IVS_REASON_INPUT_EXTRA_CONTROLLER, 0u));
  ev[0].controller[2] = 0u;
  ev[1].flags = 0u;
  ev[2].controller[3] = 0x00000100u;
  IVS_CHECK(ri_refuse_schedule(ev, 3u, IVS_E_UNSUPPORTED, IVS_REASON_INPUT_EXTRA_CONTROLLER, 2u));
  ev[2].controller[0] |= IVS_CTRL_RESERVED_MASK;
  IVS_CHECK(ri_refuse_schedule(ev, 3u, IVS_E_UNSUPPORTED, IVS_REASON_INPUT_CONTROL_RESERVED, 2u));
  ev[2].controller[0] = ri_words_a[0];
  ev[1].at_tick = RI_T0 - 1u;
  IVS_CHECK(ri_refuse_schedule(ev, 3u, IVS_E_BAD_ARG, IVS_REASON_INPUT_UNSORTED, 1u));
  return IVS_TEST_PASS;
}

/* The count comes from the profile (plan 04-12): the registry of the instance's profile holds
 * input.controllers = 2; a schedule whose words 2 and 3 are zero runs and is applied; and the
 * per-event check follows the count it is given (word 3 accepted with 4 controllers, word 1
 * refused with 1), so no index is fixed in the core. */
static int ri_extra_word_zero_ok(void)
{
  uint64_t b[RI_UNITS + 1u];
  ivs_input_event ev[2];
  ivs_machine_obs_input in;
  ivs_ms_inst x;
  ivs_run_report r;
  ivs_status st;
  uint32_t idx;
  uint32_t reason;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ri_bounds(b));
  ri_event(&ev[0], RI_TICK, ri_words_a);
  ri_event(&ev[1], RI_TICK + 1u, ri_words_b);
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
  RI_CHECK(ivs_profile_get(x.m->profile) != NULL && ivs_profile_get(x.m->profile)->input_controllers == 2u);
  st = ri_run(x.m, 0u, b[6], ev, 2u, &r);
  RI_CHECK(st == IVS_OK && ri_input_report(&r, 2u, b[5] + 1u, ri_words_b));
  ivs_input_init(&in);
  ev[1].controller[3] = 1u;
  reason = ivs_input_check(&in, ev, 2u, 4u, &st, &idx);
  RI_CHECK(reason == IVS_REASON_NONE && st == IVS_OK && idx == IVS_INPUT_INDEX_NONE);
  reason = ivs_input_check(&in, ev, 2u, 1u, &st, &idx);
  RI_CHECK(reason == IVS_REASON_INPUT_EXTRA_CONTROLLER && st == IVS_E_UNSUPPORTED && idx == 0u);
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* ---- precedence (D-05): one case per neighbour of the new slots ---- */

/* UNSUPPORTED over the event arguments: an unknown run flag, and reserved_in, each with events
 * NULL and a count. */
static int ri_prec_unsupported_over_events(void)
{
  ivs_ms_inst x;
  ivs_run_request q;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
  ri_req(&q, IVS_RUN_F_RESERVED_FRAME, 1000u, NULL, 1u);
  RI_CHECK(ri_refused(&x, x.m, &q, NULL, IVS_E_UNSUPPORTED, IVS_REASON_RUN_FLAGS, IVS_INPUT_INDEX_NONE));
  ri_req(&q, 0u, 1000u, NULL, 1u);
  q.reserved_in = 1u;
  RI_CHECK(ri_refused(&x, x.m, &q, NULL, IVS_E_UNSUPPORTED, IVS_REASON_INPUT_RESERVED_IN, IVS_INPUT_INDEX_NONE));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* The buffer slot over the event slot (both BAD_ARG): a NULL video buffer with a capacity and NULL
 * events with a count report RUN_BUFFER_NULL. */
static int ri_prec_buffer_over_events(void)
{
  ivs_ms_inst x;
  ivs_run_request q;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
  ri_req(&q, 0u, 1000u, NULL, 1u);
  q.video_capacity = 16u;
  RI_CHECK(ri_refused(&x, x.m, &q, NULL, IVS_E_BAD_ARG, IVS_REASON_RUN_BUFFER_NULL, IVS_INPUT_INDEX_NONE));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* The event arguments over BAD_STATE: an OFF machine (content loaded, never reset) with events NULL
 * and a count, and with an over-cap count. */
static int ri_prec_events_over_state(void)
{
  ivs_input_event ev;
  ivs_ms_inst x;
  ivs_run_request q;
  int rc = IVS_TEST_FAIL;
  ri_event(&ev, 0u, ri_zero);
  IVS_CHECK(ivs_ms_new(&x, RI_POISON));
  RI_CHECK(ivs_ms_content(x.m, IVS_MS_CONTENT_A));
  ri_req(&q, 0u, 1000u, NULL, 1u);
  RI_CHECK(ri_refused(&x, x.m, &q, NULL, IVS_E_BAD_ARG, IVS_REASON_INPUT_EVENTS_NULL, IVS_INPUT_INDEX_NONE));
  ri_req(&q, 0u, 1000u, &ev, IVS_INPUT_EVENTS_MAX + 1u);
  RI_CHECK(ri_refused(&x, x.m, &q, NULL, IVS_E_BAD_ARG, IVS_REASON_INPUT_EVENT_COUNT, IVS_INPUT_INDEX_NONE));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* BUFFER_TOO_SMALL over the event order checks (env seam, 16 bytes per 1000-tick frame, video
 * capacity one byte short): an unsorted schedule is refused BUFFER_TOO_SMALL, and with the capacity
 * met it is refused INPUT_UNSORTED at index 1. Plan 05-02 (D-17): the first call used a NULL video
 * buffer with capacity 0 and expected BUFFER_TOO_SMALL; that pair now means "channel not
 * requested", so the capacity is a nonzero 31 and the case still tests the stage order. */
static int ri_prec_capacity_over_event_order(void)
{
  ivs_input_event ev[2];
  ivs_ms_inst x;
  ivs_run_request q;
  ivs_run_env e;
  uint8_t buf[64];
  int rc = IVS_TEST_FAIL;
  ri_event(&ev[0], RI_T0, ri_words_a);
  ri_event(&ev[1], RI_T0 - 1u, ri_words_b);
  memset(&e, 0, sizeof e);
  e.video_bytes_per_frame = 16u;
  e.nominal_frame_ticks = 1000u;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
  ri_req(&q, 0u, 1000u, ev, 2u);
  q.video = buf;
  q.video_capacity = 31u; /* one byte below the requirement */
  RI_CHECK(ri_refused(&x, x.m, &q, &e, IVS_E_BUFFER_TOO_SMALL, IVS_REASON_RUN_BUFFER_SMALL, IVS_INPUT_INDEX_NONE));
  q.video_capacity = 32u; /* (1000 - 0) / 1000 + 1 = 2 frames */
  RI_CHECK(ri_refused(&x, x.m, &q, &e, IVS_E_BAD_ARG, IVS_REASON_INPUT_UNSORTED, 1u));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* The event order (BAD_ARG) over the event content (UNSUPPORTED): the whole array is checked for
 * order first, so reserved flags at index 0 and an unsorted tick at index 2 report INPUT_UNSORTED
 * at index 2. */
static int ri_prec_event_order_over_content(void)
{
  ivs_input_event ev[3];
  ri_event(&ev[0], RI_T0, ri_words_a);
  ri_event(&ev[1], RI_T0 + 5u, ri_words_a);
  ri_event(&ev[2], RI_T0 + 4u, ri_words_a);
  ev[0].flags = 1u;
  IVS_CHECK(ri_refuse_schedule(ev, 3u, IVS_E_BAD_ARG, IVS_REASON_INPUT_UNSORTED, 2u));
  return IVS_TEST_PASS;
}

/* ---- timing (D-14, A-04) ---- */

/* Equal ticks keep array order: A then B, both at 70, give words B and the literal chain h2; B then
 * A give words A and g2. Both apply at the start of unit 4 (b[3]). */
static int ri_equal_ticks(void)
{
  uint64_t b[RI_UNITS + 1u];
  ivs_input_event ev[2];
  ivs_ms_inst x;
  ivs_run_report r;
  ivs_status st;
  uint32_t k;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ri_bounds(b));
  for (k = 0u; k < 2u; k++) {
    ri_event(&ev[0], RI_TICK, k == 0u ? ri_words_a : ri_words_b);
    ri_event(&ev[1], RI_TICK, k == 0u ? ri_words_b : ri_words_a);
    IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
    st = ri_run(x.m, 0u, b[3], ev, 2u, &r);
    RI_CHECK(st == IVS_OK && ri_input_report(&r, 0u, b[2] + 1u, ri_zero));
    st = ri_run(x.m, 0u, b[4], ev, 2u, &r);
    RI_CHECK(st == IVS_OK && ri_input_report(&r, 2u, b[3] + 1u, ev[1].controller));
    RI_CHECK(ri_input_row(x.m, ev[1].controller, 2u, b[3] + 1u, k == 0u ? ri_h2 : ri_g2));
    ivs_ms_free(&x);
  }
  return IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* An event exactly at a slice target that is a unit boundary (b[3]) is not applied by the call that
 * ends there (no unit begins at b[3] in it); the next call applies it at the unit that starts at
 * b[3]. The same with the target and the tick inside unit 3 (70): the call ends at b[3], the next
 * call applies it at b[3], and the row then holds the literal chain h1. */
static int ri_at_target(void)
{
  uint64_t b[RI_UNITS + 1u];
  uint64_t t[2];
  ivs_input_event ev;
  ivs_ms_inst x;
  ivs_run_report r;
  ivs_status st;
  uint32_t k;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ri_bounds(b));
  t[0] = b[3];
  t[1] = RI_TICK;
  for (k = 0u; k < 2u; k++) {
    ri_event(&ev, t[k], ri_words_a);
    IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
    st = ri_run(x.m, 0u, t[k], &ev, 1u, &r);
    RI_CHECK(st == IVS_OK && r.now_ticks == b[3] && ri_input_report(&r, 0u, b[2] + 1u, ri_zero));
    st = ri_run(x.m, 0u, b[3] + 1u, &ev, 1u, &r);
    RI_CHECK(st == IVS_OK && r.now_ticks == b[4] && ri_input_report(&r, 1u, b[3] + 1u, ri_words_a));
    if (t[k] == RI_TICK) {
      RI_CHECK(ri_input_row(x.m, ri_words_a, 1u, b[3] + 1u, ri_h1)); /* the literal is for tick 70 */
    }
    ivs_ms_free(&x);
  }
  return IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* A schedule of eight events, several inside overshoot windows, run to b[10] under four slicings
 * (one call; run(now + 1) single steps; targets one tick after each event's tick; targets exactly
 * at each tick), the host resubmitting the suffix from input_consumed. After every call the applied
 * events are exactly those with at_tick below the floor; every slicing ends with storage equal to
 * the one-call run (words, count, floor and chain included) and consumes all events due. */
#define RI_OW_EVENTS 8u

static const uint64_t ri_ow_ticks[RI_OW_EVENTS] = {21u, 53u, 60u, 60u, 97u, 130u, 157u, 199u};

static int ri_ow_run(ivs_ms_inst *x, const ivs_input_event *ev, uint32_t pattern, uint64_t end, uint32_t *total)
{
  uint32_t next = 0u;
  uint32_t step = 0u;
  for (;;) {
    ivs_run_report r;
    ivs_status st;
    uint64_t now = ri_now(x->m);
    uint64_t target = end;
    uint32_t i;
    if (now >= end) {
      break;
    }
    if (pattern == 1u) {
      target = now + 1u;
    } else if (pattern >= 2u && step < RI_OW_EVENTS) {
      target = ri_ow_ticks[step] + (pattern == 2u ? 1u : 0u);
    }
    step++;
    st = ri_run(x->m, 0u, target, ev + next, RI_OW_EVENTS - next, &r);
    if (st != IVS_OK || r.input_consumed > RI_OW_EVENTS - next) {
      ivs_test_diag("pattern %u call %u refused or overconsumed", (unsigned)pattern, (unsigned)step);
      return 0;
    }
    next += r.input_consumed;
    /* Applied exactly the events with at_tick < floor (at or below the last unit's start). */
    for (i = 0u; i < RI_OW_EVENTS; i++) {
      if ((i < next) != (ri_ow_ticks[i] < r.input_floor_tick)) {
        ivs_test_diag("pattern %u call %u: event %u (tick %llu) %s with floor %llu", (unsigned)pattern,
                      (unsigned)step, (unsigned)i, (unsigned long long)ri_ow_ticks[i], i < next ? "applied" : "pending",
                      (unsigned long long)r.input_floor_tick);
        return 0;
      }
    }
    if (step > 400u) {
      return 0;
    }
  }
  *total = next;
  return 1;
}

static int ri_overshoot_window(void)
{
  uint64_t b[RI_UNITS + 1u];
  ivs_input_event ev[RI_OW_EVENTS];
  ivs_ms_inst ref;
  uint32_t i;
  uint32_t p;
  uint32_t total = 0u;
  uint32_t due = 0u;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ri_bounds(b));
  for (i = 0u; i < RI_OW_EVENTS; i++) {
    uint32_t w[4] = {0x1000u << i, i, 0u, 0u};
    ri_event(&ev[i], ri_ow_ticks[i], w);
    if (ri_ow_ticks[i] <= b[9]) {
      due++;
    }
  }
  IVS_CHECK(due == RI_OW_EVENTS && ri_ow_ticks[RI_OW_EVENTS - 1u] < b[10]);
  IVS_CHECK(ivs_ms_powered(&ref, IVS_MS_CONTENT_A, RI_POISON));
  RI_CHECK(ri_ow_run(&ref, ev, 0u, b[10], &total) && total == due);
  for (p = 1u; p < 4u; p++) {
    ivs_ms_inst x;
    int same;
    RI_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
    same = ri_ow_run(&x, ev, p, b[10], &total) && total == due &&
           memcmp(x.storage, ref.storage, IVS_MACHINE_BYTES) == 0;
    ivs_ms_free(&x);
    if (!same) {
      ivs_test_diag("slicing pattern %u differs from one call", (unsigned)p);
      goto done;
    }
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&ref);
  return rc;
}

/* ---- halt, reset, isolation ---- */

/* Program H at $1000: MVII #$0123, R1; HLT; NOP; NOP fill (decles from the Phase 2 vector data, as
 * in run_bound.c). With IVS_RUN_F_STOP_ON_HALT and the CPU HALTED a call stops before applying
 * anything (units 0, consumed 0, storage unchanged); without the flag the idle units apply it. */
static int ri_halt_stops_first(void)
{
  static const uint16_t prog[4] = {0x02B9u, 0x0123u, 0x0000u, 0x0034u};
  ivs_input_event ev;
  ivs_ms_inst x;
  ivs_run_snap s;
  ivs_run_report r;
  uint8_t *exec;
  ivs_status st;
  uint64_t h;
  size_t i;
  int rc = IVS_TEST_FAIL;
  s.copy = NULL;
  IVS_CHECK(ivs_ms_new(&x, RI_POISON));
  exec = malloc(IVS_MS_EXEC_BYTES);
  RI_CHECK(exec != NULL);
  for (i = 0u; i < IVS_MS_EXEC_BYTES / 2u; i++) {
    uint16_t w = i < 4u ? prog[i] : 0x0034u;
    exec[2u * i] = (uint8_t)(w >> 8);
    exec[2u * i + 1u] = (uint8_t)(w & 0xFFu);
  }
  RI_CHECK(ivs_ms_load(x.m, IVS_KIND_EXEC, exec, IVS_MS_EXEC_BYTES));
  RI_CHECK(ivs_ms_load(x.m, IVS_KIND_GROM, ivs_ms_grom(0u), IVS_MS_GROM_BYTES));
  RI_CHECK(ivs_machine_reset(x.m, IVS_RESET_POWER_ON) == IVS_OK);
  st = ri_run(x.m, IVS_RUN_F_STOP_ON_HALT, 100000u, NULL, 0u, &r);
  RI_CHECK(st == IVS_OK && r.stop_reason == IVS_STOP_CPU_HALTED);
  h = r.now_ticks;
  ri_event(&ev, h, ri_words_a);
  RI_CHECK(ivs_run_snap_take(&s, &x, 1));
  st = ri_run(x.m, IVS_RUN_F_STOP_ON_HALT, h + 100u, &ev, 1u, &r);
  RI_CHECK(st == IVS_OK && r.stop_reason == IVS_STOP_CPU_HALTED && r.units_executed == 0u);
  RI_CHECK(r.input_consumed == 0u && memcmp(r.controller_now, ri_zero, sizeof ri_zero) == 0);
  RI_CHECK(ivs_run_snap_same(&s, &x));
  st = ri_run(x.m, 0u, h + 4u, &ev, 1u, &r);
  RI_CHECK(st == IVS_OK && r.units_executed == 1u && ri_input_report(&r, 1u, h + 1u, ri_words_a));
  rc = IVS_TEST_PASS;
done:
  ivs_run_snap_free(&s);
  ivs_ms_free(&x);
  return rc;
}

/* A power-on reset after applied events returns the INPUT row to h0 with words, count and floor 0
 * (the literal row), the instance equals a fresh powered twin, and an event below the old floor is
 * accepted again. */
static int ri_reset(void)
{
  uint64_t b[RI_UNITS + 1u];
  ivs_input_event ev;
  ivs_ms_inst x;
  ivs_ms_inst y;
  ivs_run_report r;
  ivs_status st;
  int same;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ri_bounds(b));
  ri_event(&ev, RI_TICK, ri_words_a);
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
  st = ri_run(x.m, 0u, b[6], &ev, 1u, &r);
  RI_CHECK(st == IVS_OK && r.input_consumed == 1u);
  RI_CHECK(ivs_machine_reset(x.m, IVS_RESET_POWER_ON) == IVS_OK);
  RI_CHECK(ri_input_row(x.m, ri_zero, 0u, 0u, ri_h0));
  RI_CHECK(ivs_ms_powered(&y, IVS_MS_CONTENT_A, 0x5Au));
  same = memcmp(x.storage, y.storage, IVS_MACHINE_BYTES) == 0;
  ivs_ms_free(&y);
  RI_CHECK(same);
  ri_event(&ev, 0u, ri_words_b);
  st = ri_run(x.m, 0u, 1u, &ev, 1u, &r);
  RI_CHECK(st == IVS_OK && ri_input_report(&r, 1u, 1u, ri_words_b));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* Two instances with different schedules, stepped alternately one unit per call, each end
 * byte-identical to a solo run of its own schedule (nothing passes between instances). */
static int ri_isolation(void)
{
  uint64_t b[RI_UNITS + 1u];
  ivs_input_event ea[2];
  ivs_input_event eb[2];
  ivs_ms_inst x[2];
  ivs_ms_inst solo;
  uint32_t next[2] = {0u, 0u};
  uint32_t k;
  uint32_t j;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ri_bounds(b));
  ri_event(&ea[0], 25u, ri_words_a);
  ri_event(&ea[1], 100u, ri_words_b);
  ri_event(&eb[0], 55u, ri_words_b);
  ri_event(&eb[1], 160u, ri_words_a);
  IVS_CHECK(ivs_ms_powered(&x[0], IVS_MS_CONTENT_A, RI_POISON));
  if (!ivs_ms_powered(&x[1], IVS_MS_CONTENT_A, RI_POISON)) {
    ivs_ms_free(&x[0]);
    return IVS_TEST_FAIL;
  }
  solo.storage = NULL;
  for (k = 0u; k < RI_UNITS; k++) {
    for (j = 0u; j < 2u; j++) {
      ivs_run_report r;
      const ivs_input_event *ev = j == 0u ? ea : eb;
      RI_CHECK(ri_run(x[j].m, 0u, b[k] + 1u, ev + next[j], 2u - next[j], &r) == IVS_OK);
      next[j] += r.input_consumed;
    }
  }
  for (j = 0u; j < 2u; j++) {
    ivs_run_report r;
    RI_CHECK(next[j] == 2u);
    RI_CHECK(ivs_ms_powered(&solo, IVS_MS_CONTENT_A, RI_POISON));
    RI_CHECK(ri_run(solo.m, 0u, b[RI_UNITS], j == 0u ? ea : eb, 2u, &r) == IVS_OK && r.input_consumed == 2u);
    RI_CHECK(memcmp(solo.storage, x[j].storage, IVS_MACHINE_BYTES) == 0);
    RI_CHECK(memcmp(x[j].m->obs.input.words, j == 0u ? ri_words_b : ri_words_a, 16u) == 0);
    ivs_ms_free(&solo);
  }
  rc = IVS_TEST_PASS;
done:
  if (solo.storage != NULL) {
    ivs_ms_free(&solo);
  }
  ivs_ms_free(&x[0]);
  ivs_ms_free(&x[1]);
  return rc;
}

/* ---- guest invisibility (plan 04-12, D-15) ---- */

/* Stand-in program C at $1000, project-authored and hand-assembled here from the Phase 2 vector
 * encodings (MVI direct.json cpu.MVI row, 1010 000 ddd + address; MVO, INCR and B as in
 * tests/machine/machine_standin.c), no assembler and no listing: forever read both PSG I/O ports
 * into RAM and count.
 *   $1000 MVI $01FE, R1   $1002 MVO R1, $0200   $1004 MVI $01FF, R2   $1006 MVO R2, $0201
 *   $1008 INCR R3         $1009 B $1000 (PC+2 $100B, d $00A) */
static const uint16_t ri_prog_ports[] = {0x0281u, 0x01FEu, 0x0241u, 0x0200u, 0x0282u, 0x01FFu,
                                         0x0242u, 0x0201u, 0x000Bu, 0x0220u, 0x000Au};

/* A powered instance running program C (GROM and cartridge as content A). */
static int ri_powered_ports(ivs_ms_inst *x)
{
  size_t n = 0u;
  uint8_t *exec;
  uint8_t *cart;
  uint32_t i;
  if (!ivs_ms_new(x, RI_POISON)) {
    return 0;
  }
  exec = ivs_ms_exec(IVS_MS_PROG_A);
  if (exec != NULL) {
    for (i = 0u; i < (uint32_t)(sizeof ri_prog_ports / sizeof ri_prog_ports[0]); i++) {
      exec[2u * i] = (uint8_t)(ri_prog_ports[i] >> 8);
      exec[2u * i + 1u] = (uint8_t)(ri_prog_ports[i] & 0xFFu);
    }
  }
  if (!ivs_ms_load(x->m, IVS_KIND_EXEC, exec, IVS_MS_EXEC_BYTES) ||
      !ivs_ms_load(x->m, IVS_KIND_GROM, ivs_ms_grom(0u), IVS_MS_GROM_BYTES)) {
    ivs_ms_free(x);
    return 0;
  }
  cart = ivs_ms_cart(0x5000u, 16u, 0u, &n);
  if (!ivs_ms_load(x->m, IVS_KIND_CARTRIDGE, cart, n) || ivs_machine_reset(x->m, IVS_RESET_POWER_ON) != IVS_OK) {
    ivs_ms_free(x);
    return 0;
  }
  return 1;
}

#define RI_INV_EVENTS 10u
#define RI_INV_UNITS 72u

/* Phase 5 (plan 05-05, D-01, D-04): the words are guest-visible. This case replaces the Phase 4
 * run.input.guest-invisible, whose three digest equalities (CPU, MEM, composite) are deliberately
 * inverted. Program C reads $01FE and $01FF in a loop; one instance runs with no event, its twin
 * with ten events alternating between zero words (even) and nonzero words on both ports (odd, so the
 * last one leaves the pins held), both one unit per call (the twin resubmits its unconsumed suffix).
 * Until the first nonzero event takes effect (the first unit that starts at or after tick 220) the CPU
 * states are equal after every call; afterwards the register trace diverges. At the end the CPU, MEM
 * and composite digests and the INPUT digest all differ, the twin applied all ten events, the guest
 * RAM holds different port reads and what each guest stored is what its port reads now. The whole
 * instance storage is equal outside four areas: the INPUT row (the chain, as in Phase 4), the CPU
 * (registers that hold the bytes read), the two RAM words the program stores to, and
 * mem.port_pulldown, the second area that now differs (the pull-down masks, MEM rows of plan 05-03). */
static int ri_guest_visible(void)
{
  ivs_input_event ev[RI_INV_EVENTS];
  ivs_ms_inst x;
  ivs_ms_inst y;
  uint32_t next = 0u;
  uint32_t k;
  uint64_t start;
  int diverged = 0;
  uint8_t dx[32];
  uint8_t dy[32];
  uint16_t px[4];
  uint16_t py[4];
  static uint8_t cy[IVS_MACHINE_BYTES];
  struct ivs_machine *ix;
  struct ivs_machine *iy;
  static const uint32_t dom_differ[4] = {IVS_DIGEST_CPU, IVS_DIGEST_MEM, IVS_DIGEST_COMPOSITE, IVS_DIGEST_INPUT};
  int rc = IVS_TEST_FAIL;
  for (k = 0u; k < RI_INV_EVENTS; k++) {
    static const uint32_t on[4] = {0x00000FFFu, 0x0001F00Fu, 0u, 0u};
    ri_event(&ev[k], 20u + 200u * k, (k & 1u) == 1u ? on : ri_zero);
  }
  IVS_CHECK(ri_powered_ports(&x));
  if (!ri_powered_ports(&y)) {
    ivs_ms_free(&x);
    return IVS_TEST_FAIL;
  }
  for (k = 0u; k < RI_INV_UNITS; k++) {
    ivs_run_report rx;
    ivs_run_report ry;
    ivs_cpu_state_report cx;
    ivs_cpu_state_report cy_state;
    uint64_t target;
    start = ri_now(x.m);
    target = start + 1u;
    RI_CHECK(ri_run(x.m, 0u, target, NULL, 0u, &rx) == IVS_OK && rx.units_executed == 1u);
    RI_CHECK(ri_run(y.m, 0u, target, ev + next, RI_INV_EVENTS - next, &ry) == IVS_OK && ry.units_executed == 1u);
    next += ry.input_consumed;
    RI_CHECK(rx.now_ticks == ry.now_ticks && rx.stop_reason == ry.stop_reason);
    memset(&cx, 0, sizeof cx);
    memset(&cy_state, 0, sizeof cy_state);
    cx.struct_size = (uint32_t)sizeof cx;
    cy_state.struct_size = (uint32_t)sizeof cy_state;
    RI_CHECK(ivs_machine_cpu_state(x.m, &cx) == IVS_OK && ivs_machine_cpu_state(y.m, &cy_state) == IVS_OK);
    if (memcmp(&cx, &cy_state, sizeof cx) != 0) {
      if (start < 220u) {
        ivs_test_diag("register trace differs after the unit at %llu, before any nonzero word was applied",
                      (unsigned long long)start);
        goto done;
      }
      diverged = 1;
    }
  }
  RI_CHECK(diverged);
  RI_CHECK(next == RI_INV_EVENTS && y.m->obs.input.count == RI_INV_EVENTS);
  for (k = 0u; k < 4u; k++) {
    RI_CHECK(ivs_machine_state_digest(x.m, dom_differ[k], dx) == IVS_OK);
    RI_CHECK(ivs_machine_state_digest(y.m, dom_differ[k], dy) == IVS_OK);
    RI_CHECK(memcmp(dx, dy, 32u) != 0);
  }
  RI_CHECK(ivs_machine_peek(x.m, 0x0200u, 2u, px) == IVS_OK && ivs_machine_peek(x.m, 0x01FEu, 2u, px + 2) == IVS_OK);
  RI_CHECK(ivs_machine_peek(y.m, 0x0200u, 2u, py) == IVS_OK && ivs_machine_peek(y.m, 0x01FEu, 2u, py + 2) == IVS_OK);
  RI_CHECK(px[0] == 0x00FFu && px[1] == 0x00FFu && px[2] == 0x00FFu && px[3] == 0x00FFu); /* no event: idle */
  RI_CHECK(py[2] != 0x00FFu && py[3] != 0x00FFu);                                          /* the pins are held */
  RI_CHECK(py[0] == py[2] && py[1] == py[3]); /* the program stored what the ports read */
  /* the whole storage outside the four areas is equal: y's copy takes x's bytes there first */
  ix = (struct ivs_machine *)(void *)x.storage;
  iy = (struct ivs_machine *)(void *)y.storage;
  memcpy(cy, y.storage, sizeof cy);
  memcpy(cy + offsetof(struct ivs_machine, obs) + offsetof(ivs_machine_obs, input),
         (const uint8_t *)x.storage + offsetof(struct ivs_machine, obs) + offsetof(ivs_machine_obs, input),
         sizeof ix->obs.input);
  memcpy(cy + offsetof(struct ivs_machine, cpu), (const uint8_t *)x.storage + offsetof(struct ivs_machine, cpu),
         sizeof ix->cpu);
  memcpy(cy + offsetof(struct ivs_machine, mem) + offsetof(ivs_memsys, port_pulldown),
         (const uint8_t *)x.storage + offsetof(struct ivs_machine, mem) + offsetof(ivs_memsys, port_pulldown),
         sizeof ix->mem.port_pulldown);
  memcpy(cy + offsetof(struct ivs_machine, mem) + offsetof(ivs_memsys, sysram),
         (const uint8_t *)x.storage + offsetof(struct ivs_machine, mem) + offsetof(ivs_memsys, sysram), 2u * sizeof(uint16_t));
  RI_CHECK(memcmp(cy, x.storage, sizeof cy) == 0);
  RI_CHECK(iy->mem.port_pulldown[0] != 0u && ix->mem.port_pulldown[0] == 0u); /* the second differing area */
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  ivs_ms_free(&y);
  return rc;
}

const ivs_test_case ivs_run_input_cases[] = {
    {"run.input.event-size", ri_event_size, NULL},
    {"run.input.one-event", ri_one_event, NULL},
    {"run.input.equal-ticks", ri_equal_ticks, NULL},
    {"run.input.at-target", ri_at_target, NULL},
    {"run.input.overshoot-window", ri_overshoot_window, NULL},
    {"run.input.halt-stops-first", ri_halt_stops_first, NULL},
    {"run.input.reset", ri_reset, NULL},
    {"run.input.isolation", ri_isolation, NULL},
    {"run.input.event-count-cap-ok", ri_count_cap_ok, NULL},
    {"run.input.refuse.events-null-with-count", ri_refuse_events_null, NULL},
    {"run.input.refuse.event-count-above-cap", ri_refuse_count_cap, NULL},
    {"run.input.refuse.events-inside-instance", ri_refuse_inside_instance, NULL},
    {"run.input.refuse.events-overlap-report", ri_refuse_overlap_report, NULL},
    {"run.input.refuse.events-overlap-video", ri_refuse_overlap_video, NULL},
    {"run.input.refuse.events-overlap-audio", ri_refuse_overlap_audio, NULL},
    {"run.input.refuse.reserved-in-nonzero", ri_refuse_reserved_in, NULL},
    {"run.input.refuse.unsorted", ri_refuse_unsorted, NULL},
    {"run.input.refuse.below-floor", ri_refuse_below_floor, NULL},
    {"run.input.refuse.tick-range", ri_refuse_tick_range, NULL},
    {"run.input.refuse.control-reserved", ri_refuse_control_reserved, NULL},
    {"run.input.refuse.event-flags", ri_refuse_event_flags, NULL},
    {"run.input.refuse.event-reserved", ri_refuse_event_reserved, NULL},
    {"run.input.refuse.extra-controller-word", ri_refuse_extra_controller, NULL},
    {"run.input.extra-word-zero-ok", ri_extra_word_zero_ok, NULL},
    {"run.input.guest-visible", ri_guest_visible, NULL},
    {"run.prec.unsupported-over-events", ri_prec_unsupported_over_events, NULL},
    {"run.prec.buffer-over-events", ri_prec_buffer_over_events, NULL},
    {"run.prec.events-over-state", ri_prec_events_over_state, NULL},
    {"run.prec.capacity-over-event-order", ri_prec_capacity_over_event_order, NULL},
    {"run.prec.event-order-over-content", ri_prec_event_order_over_content, NULL},
};

const size_t ivs_run_input_count = sizeof ivs_run_input_cases / sizeof ivs_run_input_cases[0];
