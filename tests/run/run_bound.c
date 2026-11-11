/* SPDX-License-Identifier: MIT */
/* Suite run, family bound (D-01..D-05, D-07, plan 04-07): bounded execution through the public
 * call ivs_machine_run (ivs/ivs_run.h).
 *   run.target.*       the run ends at the first unit boundary at or after an absolute target in
 *                      master ticks. The expected boundaries come from a reference twin instance
 *                      stepped unit by unit through the internal step seam (the stepping primitive
 *                      the Phase 2 vectors and Phase 3 reset cases pin), never from the run call
 *                      under test; the whole instance storage is compared with the twin's.
 *   run.overshoot.*    overshoot_ticks = max(now - target, 0) for every target in a window,
 *                      called in sequence on one instance (later targets below now run nothing).
 *   run.single-step.*  N calls of run(now + 1) equal N seam steps (CPU and composite digests at
 *                      every boundary, whole-storage memcmp at the end).
 * Stand-in content A of tests/machine/machine_standin.c (no rights-bearing byte). */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "run_cases.h"

/* Fails the case with a diagnostic naming the line and the failed expression. */
#define IVS_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

/* Like IVS_CHECK, inside a case that must release its resources (rc, label done). */
#define RB_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      goto done;                                                                                       \
    }                                                                                                  \
  } while (0)

#define RB_POISON 0xA5u
/* Reference boundaries recorded per case (unit count of the twin). */
#define RB_UNITS 24u

/* ---- helpers ---- */

/* Machine time of m (info), or UINT64_MAX when info refuses. */
static uint64_t rb_now(const ivs_machine *m)
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

/* One run call: the request is built, the report poisoned, then the call (separate statements). */
static ivs_status rb_run(ivs_machine *m, uint32_t flags, uint64_t target, ivs_run_report *rep)
{
  ivs_run_request q;
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.flags = flags;
  q.target_ticks = target;
  memset(rep, RB_POISON, sizeof *rep);
  rep->struct_size = (uint32_t)sizeof *rep;
  return ivs_machine_run(m, &q, rep);
}

/* 1 when an accepted run's report says what is expected (CPU fields not judged here). */
static int rb_report(const ivs_run_report *r, uint32_t stop, uint64_t now, uint64_t target, uint64_t entry,
                     uint64_t units)
{
  uint64_t over = now > target ? now - target : 0u;
  if (r->struct_size != (uint32_t)sizeof *r || r->stop_reason != stop || r->refusal_reason != IVS_REASON_NONE ||
      r->reserved != 0u || r->now_ticks != now || r->overshoot_ticks != over || r->elapsed_ticks != now - entry ||
      r->units_executed != units) {
    ivs_test_diag("report size %u stop %u refusal %u now %llu over %llu elapsed %llu units %llu; want stop %u now "
                  "%llu over %llu elapsed %llu units %llu",
                  (unsigned)r->struct_size, (unsigned)r->stop_reason, (unsigned)r->refusal_reason,
                  (unsigned long long)r->now_ticks, (unsigned long long)r->overshoot_ticks,
                  (unsigned long long)r->elapsed_ticks, (unsigned long long)r->units_executed, (unsigned)stop,
                  (unsigned long long)now, (unsigned long long)over, (unsigned long long)(now - entry),
                  (unsigned long long)units);
    return 0;
  }
  return 1;
}

/* The reference boundaries of content A: b[0] = 0 after reset, b[i] = machine time after i seam
 * steps of a twin instance. 1 on success. */
static int rb_bounds(uint64_t b[RB_UNITS + 1u])
{
  ivs_ms_inst y;
  uint32_t i;
  int ok = 0;
  if (!ivs_ms_powered(&y, IVS_MS_CONTENT_A, RB_POISON)) {
    return 0;
  }
  b[0] = rb_now(y.m);
  for (i = 1u; i <= RB_UNITS; i++) {
    if (!ivs_ms_step(y.m, 1u, NULL)) {
      goto done;
    }
    b[i] = rb_now(y.m);
    if (b[i] <= b[i - 1u] || b[i] == UINT64_MAX) {
      ivs_test_diag("reference unit %u made no progress", (unsigned)i);
      goto done;
    }
  }
  ok = b[0] == 0u;
done:
  ivs_ms_free(&y);
  return ok;
}

/* 1 when x and a twin of content A stepped units seam units have identical storage. */
static int rb_same_as_seam(const ivs_ms_inst *x, uint32_t units)
{
  ivs_ms_inst y;
  int ok = 0;
  if (!ivs_ms_powered(&y, IVS_MS_CONTENT_A, RB_POISON)) {
    return 0;
  }
  if (ivs_ms_step(y.m, units, NULL)) {
    ok = memcmp(x->storage, y.storage, IVS_MACHINE_BYTES) == 0;
    if (!ok) {
      ivs_test_diag("storage differs from the twin stepped %u units", (unsigned)units);
    }
  }
  ivs_ms_free(&y);
  return ok;
}

/* ---- target (D-01) ---- */

/* A target strictly inside unit k + 1 (b[k] < target < b[k + 1]) ends at b[k + 1]: now >= target
 * and now minus that unit's ticks is below the target. Three targets per unit, k = 0..15, each on a
 * fresh instance whose storage then equals the twin stepped k + 1 units. */
static int rb_target_first_boundary(void)
{
  uint64_t b[RB_UNITS + 1u];
  uint32_t k;
  uint32_t j;
  IVS_CHECK(rb_bounds(b));
  for (k = 0u; k < 16u; k++) {
    uint64_t span = b[k + 1u] - b[k];
    uint64_t inside[3];
    IVS_CHECK(span >= 2u);
    inside[0] = b[k] + 1u;
    inside[1] = b[k] + span / 2u;
    inside[2] = b[k + 1u] - 1u;
    for (j = 0u; j < 3u; j++) {
      ivs_ms_inst x;
      ivs_run_report r;
      ivs_status st;
      int rc = IVS_TEST_FAIL;
      IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
      st = rb_run(x.m, 0u, inside[j], &r);
      RB_CHECK(st == IVS_OK);
      RB_CHECK(rb_report(&r, IVS_STOP_TARGET, b[k + 1u], inside[j], 0u, k + 1u));
      RB_CHECK(r.now_ticks >= inside[j] && r.now_ticks - span < inside[j]);
      RB_CHECK(r.cpu_run_state == IVS_CPU_STATE_RUNNING && r.cpu_fault_reason == IVS_CPU_FAULT_NONE);
      RB_CHECK(rb_now(x.m) == b[k + 1u]);
      RB_CHECK(rb_same_as_seam(&x, k + 1u));
      rc = IVS_TEST_PASS;
    done:
      ivs_ms_free(&x);
      if (rc != IVS_TEST_PASS) {
        ivs_test_diag("unit %u, target %llu", (unsigned)(k + 1u), (unsigned long long)inside[j]);
        return rc;
      }
    }
  }
  return IVS_TEST_PASS;
}

/* A target on a boundary ends exactly there: overshoot 0, k units. */
static int rb_target_on_boundary(void)
{
  uint64_t b[RB_UNITS + 1u];
  uint32_t k;
  IVS_CHECK(rb_bounds(b));
  for (k = 1u; k <= 16u; k++) {
    ivs_ms_inst x;
    ivs_run_report r;
    ivs_status st;
    int rc = IVS_TEST_FAIL;
    IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
    st = rb_run(x.m, 0u, b[k], &r);
    RB_CHECK(st == IVS_OK);
    RB_CHECK(rb_report(&r, IVS_STOP_TARGET, b[k], b[k], 0u, k));
    RB_CHECK(r.overshoot_ticks == 0u);
    RB_CHECK(rb_same_as_seam(&x, k));
    rc = IVS_TEST_PASS;
  done:
    ivs_ms_free(&x);
    if (rc != IVS_TEST_PASS) {
      ivs_test_diag("boundary %u", (unsigned)k);
      return rc;
    }
  }
  return IVS_TEST_PASS;
}

/* A target at or below now runs nothing: zero units, stop TARGET, overshoot now - target, elapsed
 * 0, and the instance storage, digests and time unchanged. */
static int rb_target_already_reached(void)
{
  static const uint32_t below[4] = {4u, 1u, 0u, 99u};
  uint64_t b[RB_UNITS + 1u];
  ivs_ms_inst x;
  ivs_run_snap s;
  ivs_run_report r;
  ivs_status st;
  uint32_t i;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(rb_bounds(b));
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
  s.copy = NULL;
  /* After reset (now 0): target 0. */
  RB_CHECK(ivs_run_snap_take(&s, &x, 1));
  st = rb_run(x.m, 0u, 0u, &r);
  RB_CHECK(st == IVS_OK);
  RB_CHECK(rb_report(&r, IVS_STOP_TARGET, 0u, 0u, 0u, 0u));
  RB_CHECK(r.cpu_run_state == IVS_CPU_STATE_RESET_PENDING);
  RB_CHECK(ivs_run_snap_same(&s, &x));
  ivs_run_snap_free(&s);
  /* After four units: targets b[4], b[1], 0 and b[4] - 1 (inside unit 4). */
  st = rb_run(x.m, 0u, b[4], &r);
  RB_CHECK(st == IVS_OK);
  RB_CHECK(rb_report(&r, IVS_STOP_TARGET, b[4], b[4], 0u, 4u));
  RB_CHECK(ivs_run_snap_take(&s, &x, 1));
  for (i = 0u; i < 4u; i++) {
    uint64_t t = i == 3u ? b[4] - 1u : b[below[i]];
    st = rb_run(x.m, 0u, t, &r);
    RB_CHECK(st == IVS_OK);
    RB_CHECK(rb_report(&r, IVS_STOP_TARGET, b[4], t, b[4], 0u));
    RB_CHECK(ivs_run_snap_same(&s, &x));
  }
  rc = IVS_TEST_PASS;
done:
  ivs_run_snap_free(&s);
  ivs_ms_free(&x);
  return rc;
}

/* Every target from 1 to b[12], in sequence on one instance: now is the first boundary at or after
 * the target (or the unchanged now when the target is already reached), overshoot is now - target
 * and the units of all calls add up to 12. */
static int rb_overshoot(void)
{
  uint64_t b[RB_UNITS + 1u];
  ivs_ms_inst x;
  uint64_t t;
  uint64_t units = 0u;
  uint32_t k = 0u;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(rb_bounds(b));
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
  for (t = 1u; t <= b[12]; t++) {
    ivs_run_report r;
    ivs_status st;
    uint64_t entry = b[k];
    uint32_t k0 = k;
    while (b[k] < t) {
      k++;
    }
    st = rb_run(x.m, 0u, t, &r);
    RB_CHECK(st == IVS_OK);
    if (!rb_report(&r, IVS_STOP_TARGET, b[k], t, entry, (uint64_t)(k - k0))) {
      ivs_test_diag("target %llu", (unsigned long long)t);
      goto done;
    }
    RB_CHECK(r.overshoot_ticks == b[k] - t);
    units += r.units_executed;
  }
  RB_CHECK(units == 12u && k == 12u);
  RB_CHECK(rb_same_as_seam(&x, 12u));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* ---- single step (D-01) ---- */

/* run(now + 1) is exactly one unit: 40 calls on x equal 40 seam steps on a twin y, digest by
 * digest at every boundary and storage byte for byte at the end. */
static int rb_single_step(void)
{
  ivs_ms_inst x;
  ivs_ms_inst y;
  uint32_t i;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
  if (!ivs_ms_powered(&y, IVS_MS_CONTENT_A, RB_POISON)) {
    ivs_ms_free(&x);
    return IVS_TEST_FAIL;
  }
  for (i = 0u; i < 40u; i++) {
    ivs_run_report r;
    ivs_status st;
    uint8_t cx[32];
    uint8_t dx[32];
    uint8_t cy[32];
    uint8_t dy[32];
    uint64_t now = rb_now(x.m);
    st = rb_run(x.m, 0u, now + 1u, &r);
    RB_CHECK(st == IVS_OK);
    RB_CHECK(ivs_ms_step(y.m, 1u, NULL));
    RB_CHECK(rb_report(&r, IVS_STOP_TARGET, rb_now(y.m), now + 1u, now, 1u));
    RB_CHECK(ivs_ms_digests(x.m, cx, dx));
    RB_CHECK(ivs_ms_digests(y.m, cy, dy));
    RB_CHECK(memcmp(cx, cy, 32u) == 0 && memcmp(dx, dy, 32u) == 0);
  }
  RB_CHECK(memcmp(x.storage, y.storage, IVS_MACHINE_BYTES) == 0);
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&y);
  ivs_ms_free(&x);
  return rc;
}

const ivs_test_case ivs_run_bound_cases[] = {
    {"run.target.ends-at-first-boundary-ge-target", rb_target_first_boundary, NULL},
    {"run.target.on-boundary-no-overshoot", rb_target_on_boundary, NULL},
    {"run.target.already-reached", rb_target_already_reached, NULL},
    {"run.overshoot.equals-now-minus-target", rb_overshoot, NULL},
    {"run.single-step.equals-seam", rb_single_step, NULL},
};
const size_t ivs_run_bound_count = sizeof ivs_run_bound_cases / sizeof ivs_run_bound_cases[0];
