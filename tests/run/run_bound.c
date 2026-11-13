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
 *   run.stop.*         stop reasons (D-04) on short project programs written here as literal decles
 *                      (below); expected boundaries again from a seam-stepped twin.
 *   run.refuse.*,      every refusal class of the D-05 ladder and one precedence case per adjacent
 *   run.prec.*         pair, with the SP4 oracle: status, refusal_reason, every other report byte
 *                      kept, whole instance storage memcmp-equal and (valid handle) the composite and
 *                      seven domain digests and machine time equal.
 *   run.stop-name.*    ivs_run_stop_name.
 * Stand-in content A of tests/machine/machine_standin.c and the programs below (no rights-bearing
 * byte). Their decles are hand-assembled from the project's own Phase 2 vector data
 * (data/cp1610/vectors, read from the GI CP-1600 Users Manual tables), never from an assembler:
 *   MVII #i, R1  $02B9, then the immediate      (immediate.json, cpu.MVII.r1-load)
 *   HLT          $0000                          (control.json, HLT; busrq-halt.json POL-HLT-01)
 *   NOP          $0034                          (busrq-halt.json, HLT; NOP; <stop>)
 *   MVO@ R4, R4  $0264, sticky guest fault UNSUPPORTED_MVO_SSS_EQ_MMM after its 4-cycle fetch
 *                                               (indirect.json, cpu.MVO@.sss-eq-mmm-r4-unsupported)
 * HLT stops the CPU only after the next instruction, at an interruptible boundary, with a 1-cycle
 * HALTED unit; a HALTED CPU then idles in 1-cycle units (4 master ticks each). */
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

/* An offset inside the instance, 16-aligned, in cartridge backing words that no stand-in segment
 * uses (content A writes $5000..$500F), as in run_observe.c. */
static size_t rb_inside_off(void)
{
  size_t off = offsetof(struct ivs_machine, mem) + offsetof(ivs_memsys, cart) + 2u * 0x6000u;
  return (off + 15u) & ~(size_t)15u;
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

/* ---- project programs (header comment) ---- */

/* Program H at $1000: MVII #$0123, R1; HLT; NOP; then NOP fill (the CPU halts after the NOP). */
static const uint16_t rb_prog_halt[] = {0x02B9u, 0x0123u, 0x0000u, 0x0034u};
/* Program F at $1000: MVII #$0123, R1; MVO@ R4, R4 (sticky guest fault). */
static const uint16_t rb_prog_fault[] = {0x02B9u, 0x0123u, 0x0264u};
/* Program FH at $1000: HLT; MVO@ R4, R4 (the instruction after HLT faults: fault and halt latch at
 * one boundary). */
static const uint16_t rb_prog_fault_halt[] = {0x0000u, 0x0264u};

#define RB_N(a) (sizeof(a) / sizeof((a)[0]))

/* A POWERED instance running prog at $1000 (EXEC: prog, then NOP fill; GROM: the stand-in
 * pattern; no cartridge). 1 on success. */
static int rb_custom(ivs_ms_inst *x, const uint16_t *prog, size_t n)
{
  uint8_t *exec;
  size_t i;
  if (!ivs_ms_new(x, RB_POISON)) {
    return 0;
  }
  exec = malloc(IVS_MS_EXEC_BYTES);
  if (exec == NULL) {
    ivs_ms_free(x);
    return 0;
  }
  for (i = 0u; i < IVS_MS_EXEC_BYTES / 2u; i++) {
    uint16_t w = i < n ? prog[i] : 0x0034u;
    exec[2u * i] = (uint8_t)(w >> 8);
    exec[2u * i + 1u] = (uint8_t)(w & 0xFFu);
  }
  if (!ivs_ms_load(x->m, IVS_KIND_EXEC, exec, IVS_MS_EXEC_BYTES) ||
      !ivs_ms_load(x->m, IVS_KIND_GROM, ivs_ms_grom(0u), IVS_MS_GROM_BYTES) ||
      ivs_machine_reset(x->m, IVS_RESET_POWER_ON) != IVS_OK) {
    ivs_test_diag("custom content refused");
    ivs_ms_free(x);
    return 0;
  }
  return 1;
}

/* The CPU run state of m (cpu_state), or 0xFFFFFFFF when refused. */
static uint32_t rb_cpu_run_state(const ivs_machine *m)
{
  ivs_cpu_state_report c;
  memset(&c, 0, sizeof c);
  c.struct_size = (uint32_t)sizeof c;
  if (ivs_machine_cpu_state(m, &c) != IVS_OK) {
    return 0xFFFFFFFFu;
  }
  return c.run_state;
}

/* Reference: a twin running prog stepped through the seam until its CPU is in run state want;
 * *units and *now are the seam units and the machine time at that boundary. 1 on success. */
static int rb_seam_until(const uint16_t *prog, size_t n, uint32_t want, uint64_t *units, uint64_t *now)
{
  ivs_ms_inst y;
  uint64_t u = 0u;
  int ok = 0;
  if (!rb_custom(&y, prog, n)) {
    return 0;
  }
  while (u < 64u && rb_cpu_run_state(y.m) != want) {
    if (!ivs_ms_step(y.m, 1u, NULL)) {
      goto done;
    }
    u++;
  }
  ok = rb_cpu_run_state(y.m) == want;
  *units = u;
  *now = rb_now(y.m);
  if (!ok) {
    ivs_test_diag("reference never reached run state %u", (unsigned)want);
  }
done:
  ivs_ms_free(&y);
  return ok;
}

/* ---- stop reasons (D-04) ---- */

/* With IVS_RUN_F_STOP_ON_HALT the run stops CPU_HALTED at the first boundary where the CPU is
 * HALTED, below the target; a second call runs nothing and stops CPU_HALTED again. */
static int rb_stop_halt_flag(void)
{
  ivs_ms_inst x;
  ivs_run_report r;
  ivs_status st;
  uint64_t k;
  uint64_t h;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(rb_seam_until(rb_prog_halt, RB_N(rb_prog_halt), IVS_CPU_STATE_HALTED, &k, &h));
  IVS_CHECK(k >= 4u);
  IVS_CHECK(rb_custom(&x, rb_prog_halt, RB_N(rb_prog_halt)));
  st = rb_run(x.m, IVS_RUN_F_STOP_ON_HALT, h + 1000u, &r);
  RB_CHECK(st == IVS_OK);
  RB_CHECK(rb_report(&r, IVS_STOP_CPU_HALTED, h, h + 1000u, 0u, k));
  RB_CHECK(r.now_ticks < h + 1000u && r.cpu_run_state == IVS_CPU_STATE_HALTED);
  st = rb_run(x.m, IVS_RUN_F_STOP_ON_HALT, h + 1000u, &r);
  RB_CHECK(st == IVS_OK);
  RB_CHECK(rb_report(&r, IVS_STOP_CPU_HALTED, h, h + 1000u, h, 0u));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* Without the flag a HALTED CPU idles in 1-cycle units (4 ticks) to the target and stops TARGET:
 * h + 40 ends on it (10 idle units), h + 41 ends at h + 44 (11 idle units, overshoot 3). */
static int rb_stop_halt_no_flag(void)
{
  ivs_ms_inst x;
  ivs_run_report r;
  ivs_status st;
  uint64_t k;
  uint64_t h;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(rb_seam_until(rb_prog_halt, RB_N(rb_prog_halt), IVS_CPU_STATE_HALTED, &k, &h));
  IVS_CHECK(rb_custom(&x, rb_prog_halt, RB_N(rb_prog_halt)));
  st = rb_run(x.m, 0u, h + 40u, &r);
  RB_CHECK(st == IVS_OK);
  RB_CHECK(rb_report(&r, IVS_STOP_TARGET, h + 40u, h + 40u, 0u, k + 10u));
  RB_CHECK(r.cpu_run_state == IVS_CPU_STATE_HALTED);
  ivs_ms_free(&x);
  IVS_CHECK(rb_custom(&x, rb_prog_halt, RB_N(rb_prog_halt)));
  st = rb_run(x.m, 0u, h + 41u, &r);
  RB_CHECK(st == IVS_OK);
  RB_CHECK(rb_report(&r, IVS_STOP_TARGET, h + 44u, h + 41u, 0u, k + 11u));
  RB_CHECK(r.overshoot_ticks == 3u && r.cpu_run_state == IVS_CPU_STATE_HALTED);
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* A guest fault ends the run with IVS_OK and stop GUEST_FAULT at the faulting boundary, below the
 * target, with the CPU FAULTED and its reason. */
static int rb_stop_guest_fault(void)
{
  ivs_ms_inst x;
  ivs_run_report r;
  ivs_status st;
  uint64_t k;
  uint64_t f;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(rb_seam_until(rb_prog_fault, RB_N(rb_prog_fault), IVS_CPU_STATE_FAULTED, &k, &f));
  IVS_CHECK(rb_custom(&x, rb_prog_fault, RB_N(rb_prog_fault)));
  st = rb_run(x.m, 0u, 1000000u, &r);
  RB_CHECK(st == IVS_OK);
  RB_CHECK(rb_report(&r, IVS_STOP_GUEST_FAULT, f, 1000000u, 0u, k));
  RB_CHECK(r.cpu_run_state == IVS_CPU_STATE_FAULTED &&
           r.cpu_fault_reason == IVS_CPU_FAULT_UNSUPPORTED_MVO_SSS_EQ_MMM);
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* A FAULTED CPU is never stepped: later calls with a target above now, equal to now and below it
 * run zero units, stop GUEST_FAULT and leave storage, digests and time unchanged. */
static int rb_stop_fault_sticky(void)
{
  ivs_ms_inst x;
  ivs_run_snap s;
  ivs_run_report r;
  ivs_status st;
  uint64_t k;
  uint64_t f;
  uint32_t i;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(rb_seam_until(rb_prog_fault, RB_N(rb_prog_fault), IVS_CPU_STATE_FAULTED, &k, &f));
  IVS_CHECK(rb_custom(&x, rb_prog_fault, RB_N(rb_prog_fault)));
  s.copy = NULL;
  st = rb_run(x.m, 0u, f, &r);
  RB_CHECK(st == IVS_OK && r.stop_reason == IVS_STOP_GUEST_FAULT && r.now_ticks == f);
  RB_CHECK(ivs_run_snap_take(&s, &x, 1));
  for (i = 0u; i < 4u; i++) {
    uint64_t t = i == 0u ? f + 1000u : i == 1u ? f + 1u : i == 2u ? f : 0u;
    st = rb_run(x.m, i == 1u ? IVS_RUN_F_STOP_ON_HALT : 0u, t, &r);
    RB_CHECK(st == IVS_OK);
    RB_CHECK(rb_report(&r, IVS_STOP_GUEST_FAULT, f, t, f, 0u));
    RB_CHECK(r.cpu_run_state == IVS_CPU_STATE_FAULTED);
    RB_CHECK(ivs_run_snap_same(&s, &x));
  }
  rc = IVS_TEST_PASS;
done:
  ivs_run_snap_free(&s);
  ivs_ms_free(&x);
  return rc;
}

/* HLT latched and the next instruction faults: at that boundary the stop is GUEST_FAULT, with the
 * stop-on-halt flag too (fault is tested before halt). */
static int rb_stop_fault_before_halt(void)
{
  ivs_ms_inst x;
  ivs_run_report r;
  ivs_status st;
  uint64_t k;
  uint64_t f;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(rb_seam_until(rb_prog_fault_halt, RB_N(rb_prog_fault_halt), IVS_CPU_STATE_FAULTED, &k, &f));
  IVS_CHECK(rb_custom(&x, rb_prog_fault_halt, RB_N(rb_prog_fault_halt)));
  st = rb_run(x.m, IVS_RUN_F_STOP_ON_HALT, f + 1000u, &r);
  RB_CHECK(st == IVS_OK);
  RB_CHECK(rb_report(&r, IVS_STOP_GUEST_FAULT, f, f + 1000u, 0u, k));
  RB_CHECK(r.cpu_run_state == IVS_CPU_STATE_FAULTED &&
           r.cpu_fault_reason == IVS_CPU_FAULT_UNSUPPORTED_MVO_SSS_EQ_MMM);
  RB_CHECK(((const struct ivs_machine *)(const void *)x.storage)->cpu.halt_pending == 1u);
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* A unit the seam refuses (here: CPU state damaged through the test seam, which the CPU step
 * refuses) stops INTERNAL with IVS_OK, zero units, and nothing changed. */
static int rb_stop_internal(void)
{
  ivs_ms_inst x;
  ivs_run_snap s;
  ivs_run_report r;
  ivs_status st;
  struct ivs_machine *im;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
  s.copy = NULL;
  RB_CHECK(ivs_ms_step(x.m, 3u, NULL));
  im = (struct ivs_machine *)(void *)x.storage;
  im->cpu.run_state = 9u;
  RB_CHECK(ivs_run_snap_take(&s, &x, 0));
  st = rb_run(x.m, 0u, im->time_ticks + 100u, &r);
  RB_CHECK(st == IVS_OK);
  RB_CHECK(r.stop_reason == IVS_STOP_INTERNAL && r.units_executed == 0u && r.elapsed_ticks == 0u &&
           r.overshoot_ticks == 0u && r.cpu_run_state == 9u);
  RB_CHECK(ivs_run_snap_same(&s, &x));
  rc = IVS_TEST_PASS;
done:
  ivs_run_snap_free(&s);
  ivs_ms_free(&x);
  return rc;
}

/* Review IN-01: the same refused unit with two input events due at its start (at the entry
 * tick): the INPUT row is restored, so the instance is byte-identical (words, count, floor and
 * chain), and input_consumed is 0, as the INTERNAL contract states. The second event clears the
 * words again, so the static of the named fault INPUT_STATIC_CARRY ends all zero and later cases
 * of that fault build see no carry from this one. */
static int rb_stop_internal_input(void)
{
  ivs_ms_inst x;
  ivs_run_snap s;
  ivs_run_request q;
  ivs_run_report r;
  ivs_input_event ev[2];
  ivs_status st;
  struct ivs_machine *im;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
  s.copy = NULL;
  RB_CHECK(ivs_ms_step(x.m, 3u, NULL));
  im = (struct ivs_machine *)(void *)x.storage;
  im->cpu.run_state = 9u;
  RB_CHECK(ivs_run_snap_take(&s, &x, 0));
  memset(ev, 0, sizeof ev);
  ev[0].at_tick = im->time_ticks;
  ev[0].controller[0] = IVS_CTRL_KEY_1;
  ev[1].at_tick = im->time_ticks;
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.target_ticks = im->time_ticks + 100u;
  q.events = ev;
  q.event_count = 2u;
  memset(&r, RB_POISON, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  st = ivs_machine_run(x.m, &q, &r);
  RB_CHECK(st == IVS_OK);
  RB_CHECK(r.stop_reason == IVS_STOP_INTERNAL && r.units_executed == 0u && r.elapsed_ticks == 0u &&
           r.input_consumed == 0u && r.controller_now[0] == 0u && r.controller_now[1] == 0u);
  RB_CHECK(ivs_run_snap_same(&s, &x));
  rc = IVS_TEST_PASS;
done:
  ivs_run_snap_free(&s);
  ivs_ms_free(&x);
  return rc;
}

/* Review IN-02: a span that runs past the top of the address space names no object and is
 * refused as overlapping before the pairwise tests, with nothing written or read: an audio buffer
 * (RUN_BUFFER_OVERLAP; the video buffer takes the same test), and a peek out array (BAD_ARG). The pointers are never
 * dereferenced by a correct core (Phase 4 writes no buffer byte; a refused peek writes nothing). */
static int rb_wrap_buffers(void)
{
  ivs_ms_inst x;
  ivs_run_snap s;
  ivs_run_request q;
  ivs_run_report r;
  ivs_status st;
  uint8_t *top = (uint8_t *)(uintptr_t)(UINTPTR_MAX - 15u);
  uint32_t pass;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
  s.copy = NULL;
  for (pass = 0u; pass < 2u; pass++) {
    memset(&q, 0, sizeof q);
    q.struct_size = (uint32_t)sizeof q;
    q.target_ticks = 100u;
    /* The audio buffer: the named fault OUTBUF_PARTIAL_WRITE writes video[0] of a refused run,
     * which would dereference a wrapping video pointer in that fault build. */
    q.audio = top;
    q.audio_capacity = pass == 0u ? 17u : UINT64_MAX; /* one byte past the top; far past it */
    memset(&r, RB_POISON, sizeof r);
    r.struct_size = (uint32_t)sizeof r;
    RB_CHECK(ivs_run_snap_take(&s, &x, 0));
    st = ivs_machine_run(x.m, &q, &r);
    RB_CHECK(st == IVS_E_BAD_ARG && r.refusal_reason == IVS_REASON_RUN_BUFFER_OVERLAP);
    RB_CHECK(ivs_run_snap_same(&s, &x));
    ivs_run_snap_free(&s);
  }
  rc = IVS_TEST_PASS;
done:
  ivs_run_snap_free(&s);
  ivs_ms_free(&x);
  return rc;
}

static int rb_peek_out_wraps(void)
{
  ivs_ms_inst x;
  uint16_t *top = (uint16_t *)(uintptr_t)(UINTPTR_MAX - 7u); /* 8 bytes to the top, even */
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
  RB_CHECK(ivs_machine_peek(x.m, 0x0100u, 8u, top) == IVS_E_BAD_ARG); /* 16 bytes: wraps */
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* ---- refusals (D-05) ---- */

#define RB_NO_WRITE 0xFFFFFFFFu
#define RB_REP_BYTES 192u

typedef union rb_rep_buf {
  ivs_run_report r;
  uint8_t b[RB_REP_BYTES];
} rb_rep_buf;

/* A request with struct_size size (the bytes past the v1 size zero), flags and target. */
typedef union rb_req_buf {
  ivs_run_request q;
  uint8_t b[IVS_STRUCT_SIZE_MAX];
} rb_req_buf;

static void rb_req(rb_req_buf *q, uint32_t size, uint32_t flags, uint64_t target)
{
  memset(q, 0, sizeof *q);
  q->q.struct_size = size;
  q->q.flags = flags;
  q->q.target_ticks = target;
}

/* 1 when a refused call left the report as expected: with RB_NO_WRITE every byte kept (struct_size
 * rep_size, the rest poison); else struct_size the v1 size, refusal_reason reason and every other
 * byte poison. */
static int rb_rep_kept(const rb_rep_buf *u, uint32_t rep_size, uint32_t reason)
{
  size_t i;
  if (reason == RB_NO_WRITE) {
    if (u->r.struct_size != rep_size) {
      ivs_test_diag("report struct_size written by a refusal that must write nothing");
      return 0;
    }
  } else if (u->r.struct_size != (uint32_t)sizeof(ivs_run_report) || u->r.refusal_reason != reason) {
    ivs_test_diag("report struct_size %u refusal_reason %u, want %u %u", (unsigned)u->r.struct_size,
                  (unsigned)u->r.refusal_reason, (unsigned)sizeof(ivs_run_report), (unsigned)reason);
    return 0;
  }
  for (i = 4u; i < RB_REP_BYTES; i++) {
    if (reason != RB_NO_WRITE && i >= offsetof(ivs_run_report, refusal_reason) &&
        i < offsetof(ivs_run_report, refusal_reason) + 4u) {
      continue;
    }
    if (u->b[i] != RB_POISON) {
      ivs_test_diag("report byte %u written by a refused call", (unsigned)i);
      return 0;
    }
  }
  return 1;
}

/* One refused call with the SP4 oracle: m and req as given, a poisoned report with struct_size
 * rep_size; the status, the report (rb_rep_kept) and the snapshot of x (live: digests and time). */
static int rb_refused(const ivs_ms_inst *x, int live, ivs_machine *m, const ivs_run_request *req, uint32_t rep_size,
                      ivs_status want, uint32_t reason)
{
  ivs_run_snap s;
  rb_rep_buf u;
  ivs_status st;
  int ok = 0;
  memset(&u, RB_POISON, sizeof u);
  u.r.struct_size = rep_size;
  if (!ivs_run_snap_take(&s, x, live)) {
    ivs_run_snap_free(&s);
    return 0;
  }
  st = ivs_machine_run(m, req, &u.r);
  if (st != want) {
    ivs_test_diag("status %s, want %s", ivs_status_name(st), ivs_status_name(want));
  } else {
    ok = rb_rep_kept(&u, rep_size, reason) && ivs_run_snap_same(&s, x);
  }
  ivs_run_snap_free(&s);
  return ok;
}

/* The core's own (current, v2) sizes: the ladder tests use the full structs (plan 05-02); the
 * v1-sized callers are the run.struct.* cases (run_struct.c). */
#define RB_V1_REQ ((uint32_t)sizeof(ivs_run_request))
#define RB_V1_REP ((uint32_t)sizeof(ivs_run_report))

/* BAD_ARG: req NULL (RUN_NULL_ARG written into the report). */
static int rb_refuse_null_request(void)
{
  ivs_ms_inst x;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
  RB_CHECK(rb_refused(&x, 1, x.m, NULL, RB_V1_REP, IVS_E_BAD_ARG, IVS_REASON_RUN_NULL_ARG));
  RB_CHECK(rb_refused(&x, 1, x.m, NULL, 4096u, IVS_E_BAD_ARG, IVS_REASON_RUN_NULL_ARG));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* BAD_ARG: rep NULL (nothing can be written), with a valid and with a NULL request. */
static int rb_refuse_null_report(void)
{
  ivs_ms_inst x;
  ivs_run_snap s;
  rb_req_buf q;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
  s.copy = NULL;
  rb_req(&q, RB_V1_REQ, 0u, 1000u);
  RB_CHECK(ivs_run_snap_take(&s, &x, 1));
  RB_CHECK(ivs_machine_run(x.m, &q.q, NULL) == IVS_E_BAD_ARG);
  RB_CHECK(ivs_machine_run(x.m, NULL, NULL) == IVS_E_BAD_ARG);
  RB_CHECK(ivs_run_snap_same(&s, &x));
  rc = IVS_TEST_PASS;
done:
  ivs_run_snap_free(&s);
  ivs_ms_free(&x);
  return rc;
}

/* BAD_ARG: request struct_size 0, 15 (below v1), 4097 and 0xFFFFFFFF (above the cap; never
 * followed) -> RUN_STRUCT_SIZE. */
static int rb_refuse_struct_size_request(void)
{
  static const uint32_t sizes[4] = {0u, 15u, 4097u, 0xFFFFFFFFu};
  ivs_ms_inst x;
  rb_req_buf q;
  uint32_t i;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
  for (i = 0u; i < 4u; i++) {
    rb_req(&q, sizes[i], 0u, 1000u);
    RB_CHECK(rb_refused(&x, 1, x.m, &q.q, RB_V1_REP, IVS_E_BAD_ARG, IVS_REASON_RUN_STRUCT_SIZE));
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* BAD_ARG: report struct_size 0, 55 (below v1), 4097 and 0xFFFFFFFF -> nothing written. */
static int rb_refuse_struct_size_report(void)
{
  static const uint32_t sizes[4] = {0u, 55u, 4097u, 0xFFFFFFFFu};
  ivs_ms_inst x;
  rb_req_buf q;
  uint32_t i;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
  rb_req(&q, RB_V1_REQ, 0u, 1000u);
  for (i = 0u; i < 4u; i++) {
    RB_CHECK(rb_refused(&x, 1, x.m, &q.q, sizes[i], IVS_E_BAD_ARG, RB_NO_WRITE));
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* BAD_HANDLE (IVS_REASON_HANDLE): a NULL handle and a destroyed instance. A report inside the
 * destroyed instance's storage gets nothing (the storage a handle names is never written). */
static int rb_refuse_bad_handle(void)
{
  ivs_ms_inst x;
  ivs_run_snap s;
  rb_req_buf q;
  ivs_run_report *in;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
  s.copy = NULL;
  rb_req(&q, RB_V1_REQ, 0u, 1000u);
  RB_CHECK(rb_refused(&x, 1, NULL, &q.q, RB_V1_REP, IVS_E_BAD_HANDLE, IVS_REASON_HANDLE));
  RB_CHECK(ivs_machine_destroy(x.m) == IVS_OK);
  RB_CHECK(rb_refused(&x, 0, x.m, &q.q, RB_V1_REP, IVS_E_BAD_HANDLE, IVS_REASON_HANDLE));
  in = (ivs_run_report *)(void *)(x.storage + rb_inside_off());
  in->struct_size = RB_V1_REP;
  RB_CHECK(ivs_run_snap_take(&s, &x, 0));
  RB_CHECK(ivs_machine_run(x.m, &q.q, in) == IVS_E_BAD_HANDLE);
  RB_CHECK(ivs_run_snap_same(&s, &x));
  rc = IVS_TEST_PASS;
done:
  ivs_run_snap_free(&s);
  ivs_ms_free(&x);
  return rc;
}

/* BAD_ARG: the report inside the instance storage and straddling its end (nothing written). A
 * positive control places the report just after the instance bytes of a longer host allocation
 * (accepted, a full report). */
static int rb_refuse_report_inside(void)
{
  ivs_ms_inst x;
  ivs_run_snap s;
  rb_req_buf q;
  ivs_run_report *in;
  ivs_run_report *tail;
  ivs_run_report *after;
  ivs_create_options o;
  uint8_t *big = NULL;
  ivs_machine *bm = NULL;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
  s.copy = NULL;
  rb_req(&q, RB_V1_REQ, 0u, 1000u);
  in = (ivs_run_report *)(void *)(x.storage + rb_inside_off());
  in->struct_size = RB_V1_REP;
  tail = (ivs_run_report *)(void *)(x.storage + IVS_MACHINE_BYTES - 16u);
  tail->struct_size = RB_V1_REP;
  RB_CHECK(ivs_run_snap_take(&s, &x, 1));
  RB_CHECK(ivs_machine_run(x.m, &q.q, in) == IVS_E_BAD_ARG);
  RB_CHECK(ivs_machine_run(x.m, &q.q, tail) == IVS_E_BAD_ARG);
  RB_CHECK(ivs_run_snap_same(&s, &x));
  memset(&o, 0, sizeof o);
  o.struct_size = (uint32_t)sizeof o;
  o.profile_id = IVS_MS_PROFILE;
  big = aligned_alloc(IVS_MACHINE_ALIGN, IVS_MACHINE_BYTES + 256u);
  RB_CHECK(big != NULL);
  memset(big, RB_POISON, IVS_MACHINE_BYTES + 256u);
  RB_CHECK(ivs_machine_create(&o, big, IVS_MACHINE_BYTES + 256u, &bm) == IVS_OK);
  RB_CHECK(ivs_ms_content(bm, IVS_MS_CONTENT_A) && ivs_machine_reset(bm, IVS_RESET_POWER_ON) == IVS_OK);
  after = (ivs_run_report *)(void *)(big + IVS_MACHINE_BYTES);
  after->struct_size = RB_V1_REP;
  RB_CHECK(ivs_machine_run(bm, &q.q, after) == IVS_OK);
  RB_CHECK(after->struct_size == RB_V1_REP && after->stop_reason == IVS_STOP_TARGET && after->now_ticks >= 1000u);
  rc = IVS_TEST_PASS;
done:
  free(big);
  ivs_run_snap_free(&s);
  ivs_ms_free(&x);
  return rc;
}

/* UNSUPPORTED (RUN_FLAGS): unknown flag bits 4, 0x80000000, all bits. */
static int rb_refuse_flags_unknown(void)
{
  static const uint32_t flags[3] = {4u, 0x80000000u, 0xFFFFFFFFu};
  ivs_ms_inst x;
  rb_req_buf q;
  uint32_t i;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
  for (i = 0u; i < 3u; i++) {
    rb_req(&q, RB_V1_REQ, flags[i], 1000u);
    RB_CHECK(rb_refused(&x, 1, x.m, &q.q, RB_V1_REP, IVS_E_UNSUPPORTED, IVS_REASON_RUN_FLAGS));
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* Plan 06-09 (D-14, A-02; a REVIEWED CHANGE): the Phase 4 test run.refuse.flags-frame-reserved, which refused IVS_RUN_F_STOP_ON_FRAME (value 2,
 * alone and with the stop-on-halt bit) as an unknown flag, is inverted into the positive run.flags.stop-on-frame-* ids below. The generic
 * bad-flag sites already use bit 4 (RUN_TEST_BAD_FLAG), so an unknown bit is still refused (run.refuse.flags-unknown).
 * The frame events of the stand-in content are the timeline events of the profile: E_k = (k + 1) x 59,736 ticks (the page's frame, the chain
 * literal's first tick), frame_phase_ticks 0. One run call with a flag and an optional video buffer. */
#define RB_E0 59736u
static ivs_status rb_flag_run(ivs_ms_inst *x, uint32_t flags, uint64_t target, uint8_t *video, uint64_t cap, ivs_run_report *r)
{
  ivs_run_request q;
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.flags = flags;
  q.target_ticks = target;
  q.video = video;
  q.video_capacity = cap;
  memset(r, 0, sizeof *r);
  r->struct_size = (uint32_t)sizeof *r;
  return ivs_machine_run(x->m, &q, r);
}

/* The flag is accepted alone and with the stop-on-halt bit; a call whose target lies below the frame end is a plain run to the target. */
static int rb_flags_stop_on_frame_accepted(void)
{
  ivs_ms_inst x;
  ivs_run_report r;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
  RB_CHECK(rb_flag_run(&x, IVS_RUN_F_STOP_ON_FRAME, 1000u, NULL, 0u, &r) == IVS_OK && r.stop_reason == IVS_STOP_TARGET);
  RB_CHECK(r.video_frames_completed == 0u && r.now_ticks >= 1000u);
  RB_CHECK(rb_flag_run(&x, IVS_RUN_F_STOP_ON_FRAME | IVS_RUN_F_STOP_ON_HALT, 2000u, NULL, 0u, &r) == IVS_OK);
  RB_CHECK(r.stop_reason == IVS_STOP_TARGET || r.stop_reason == IVS_STOP_CPU_HALTED);
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* A flagged call with the largest target stops at the first frame end (the effective target is the smaller of the two) and needs exactly one
 * slot: capacity one byte short is BUFFER_TOO_SMALL, exactly one slot is accepted and filled (the target of 2^63 would otherwise demand an
 * astronomical capacity, T-06-13). */
static int rb_flags_stop_on_frame_clamps_target(void)
{
  enum { SLOT = 15296 };
  ivs_ms_inst x;
  ivs_run_report r;
  uint8_t *v = malloc((size_t)SLOT + 32u);
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(v != NULL);
  memset(v, RB_POISON, (size_t)SLOT + 32u);
  if (!ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON)) {
    free(v);
    return IVS_TEST_FAIL;
  }
  RB_CHECK(rb_flag_run(&x, IVS_RUN_F_STOP_ON_FRAME, IVS_RUN_TARGET_MAX, v, (uint64_t)SLOT - 1u, &r) == IVS_E_BUFFER_TOO_SMALL);
  RB_CHECK(r.refusal_reason == IVS_REASON_RUN_BUFFER_SMALL && v[0] == RB_POISON);
  RB_CHECK(rb_flag_run(&x, IVS_RUN_F_STOP_ON_FRAME, IVS_RUN_TARGET_MAX, v, (uint64_t)SLOT + 32u, &r) == IVS_OK);
  RB_CHECK(r.stop_reason == IVS_STOP_FRAME && r.video_frames_completed == 1u && r.video_bytes_written == SLOT);
  RB_CHECK(r.now_ticks >= RB_E0 && r.now_ticks < RB_E0 + 1000u && r.elapsed_ticks == r.now_ticks);
  RB_CHECK(r.overshoot_ticks == 0u); /* overshoot stays against the caller's target (2^63) */
  RB_CHECK(v[SLOT] == RB_POISON);
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  free(v);
  return rc;
}

/* The exit-state order: a target equal to the frame end reports FRAME (not TARGET); a target below it with no unit crossing the frame end
 * reports TARGET; a halted CPU reports CPU_HALTED before FRAME (the stop-on-halt bit is set and the CPU is halted at once). */
static int rb_flags_stop_reason_frame(void)
{
  ivs_ms_inst x;
  ivs_run_report r;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
  RB_CHECK(rb_flag_run(&x, IVS_RUN_F_STOP_ON_FRAME, 30000u, NULL, 0u, &r) == IVS_OK && r.stop_reason == IVS_STOP_TARGET);
  RB_CHECK(rb_flag_run(&x, IVS_RUN_F_STOP_ON_FRAME, RB_E0, NULL, 0u, &r) == IVS_OK);
  RB_CHECK(r.stop_reason == IVS_STOP_FRAME && r.video_frames_completed == 1u && r.now_ticks >= RB_E0);
  /* the same target without the flag is TARGET (the frame completes either way) */
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* Report v3: video_first_index is the number of frames completed before the call, last_frame_end_ticks the E of the last frame the call
 * completed (0 when none), next_frame_end_ticks the end of the frame in progress at exit; three flagged calls to the third frame end. */
static int rb_flags_report_v3_fields(void)
{
  ivs_ms_inst x;
  ivs_run_report r;
  uint32_t k;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
  RB_CHECK(rb_flag_run(&x, IVS_RUN_F_STOP_ON_FRAME, 100u, NULL, 0u, &r) == IVS_OK); /* a call that completes no frame */
  RB_CHECK(r.video_first_index == 0u && r.last_frame_end_ticks == 0u && r.next_frame_end_ticks == RB_E0 && r.video_frames_completed == 0u);
  for (k = 0u; k < 3u; k++) {
    RB_CHECK(rb_flag_run(&x, IVS_RUN_F_STOP_ON_FRAME, IVS_RUN_TARGET_MAX, NULL, 0u, &r) == IVS_OK);
    RB_CHECK(r.stop_reason == IVS_STOP_FRAME && r.video_frames_completed == 1u && r.video_bytes_written == 0u);
    RB_CHECK(r.video_first_index == k);
    RB_CHECK(r.last_frame_end_ticks == (uint64_t)(k + 1u) * RB_E0 && r.next_frame_end_ticks == (uint64_t)(k + 2u) * RB_E0);
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* UNSUPPORTED (RUN_TAIL): a larger request with a nonzero byte just past the v1 size and at the
 * cap's last byte. Positive control: a 4096-byte request with an all-zero tail runs. */
static int rb_refuse_tail(void)
{
  ivs_ms_inst x;
  rb_req_buf q;
  ivs_run_report r;
  ivs_status st;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
  rb_req(&q, RB_V1_REQ + 8u, 0u, 1000u);
  q.b[RB_V1_REQ] = 1u;
  RB_CHECK(rb_refused(&x, 1, x.m, &q.q, RB_V1_REP, IVS_E_UNSUPPORTED, IVS_REASON_RUN_TAIL));
  rb_req(&q, IVS_STRUCT_SIZE_MAX, 0u, 1000u);
  q.b[IVS_STRUCT_SIZE_MAX - 1u] = 0x80u;
  RB_CHECK(rb_refused(&x, 1, x.m, &q.q, RB_V1_REP, IVS_E_UNSUPPORTED, IVS_REASON_RUN_TAIL));
  rb_req(&q, IVS_STRUCT_SIZE_MAX, 0u, 1000u);
  memset(&r, RB_POISON, sizeof r);
  r.struct_size = RB_V1_REP;
  st = ivs_machine_run(x.m, &q.q, &r);
  RB_CHECK(st == IVS_OK && r.stop_reason == IVS_STOP_TARGET && r.now_ticks >= 1000u);
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* BAD_STATE (RUN_STATE_OFF): a fresh instance and one with content loaded but never reset. */
static int rb_refuse_state_off(void)
{
  ivs_ms_inst x;
  rb_req_buf q;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_new(&x, RB_POISON));
  rb_req(&q, RB_V1_REQ, 0u, 1000u);
  RB_CHECK(rb_refused(&x, 1, x.m, &q.q, RB_V1_REP, IVS_E_BAD_STATE, IVS_REASON_RUN_STATE_OFF));
  RB_CHECK(ivs_ms_content(x.m, IVS_MS_CONTENT_A));
  RB_CHECK(rb_refused(&x, 1, x.m, &q.q, RB_V1_REP, IVS_E_BAD_STATE, IVS_REASON_RUN_STATE_OFF));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* OUT_OF_RANGE (RUN_TARGET_RANGE): 2^63 + 1 and UINT64_MAX, first on a FAULTED CPU (a run there
 * would end at once, so a missing range check fails here instead of running for ever), then on a
 * running CPU. */
static int rb_refuse_target_range(void)
{
  ivs_ms_inst x;
  rb_req_buf q;
  uint32_t pass;
  int rc = IVS_TEST_FAIL;
  for (pass = 0u; pass < 2u; pass++) {
    if (pass == 0u) {
      ivs_run_report r;
      ivs_status st;
      IVS_CHECK(rb_custom(&x, rb_prog_fault, RB_N(rb_prog_fault)));
      st = rb_run(x.m, 0u, 1000000u, &r);
      RB_CHECK(st == IVS_OK && r.stop_reason == IVS_STOP_GUEST_FAULT);
    } else {
      IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
      RB_CHECK(ivs_ms_step(x.m, 3u, NULL));
    }
    rb_req(&q, RB_V1_REQ, 0u, IVS_RUN_TARGET_MAX + 1u);
    RB_CHECK(rb_refused(&x, 1, x.m, &q.q, RB_V1_REP, IVS_E_OUT_OF_RANGE, IVS_REASON_RUN_TARGET_RANGE));
    rb_req(&q, RB_V1_REQ, IVS_RUN_F_STOP_ON_HALT, UINT64_MAX);
    RB_CHECK(rb_refused(&x, 1, x.m, &q.q, RB_V1_REP, IVS_E_OUT_OF_RANGE, IVS_REASON_RUN_TARGET_RANGE));
    ivs_ms_free(&x);
  }
  return IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* The largest target, 2^63, is accepted: on a FAULTED CPU (stop GUEST_FAULT, zero units) and on a
 * HALTED CPU with the stop-on-halt flag (stop CPU_HALTED). */
static int rb_target_max_accepted(void)
{
  ivs_ms_inst x;
  ivs_run_report r;
  ivs_status st;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(rb_custom(&x, rb_prog_fault, RB_N(rb_prog_fault)));
  st = rb_run(x.m, 0u, 1000000u, &r);
  RB_CHECK(st == IVS_OK && r.stop_reason == IVS_STOP_GUEST_FAULT);
  st = rb_run(x.m, 0u, IVS_RUN_TARGET_MAX, &r);
  RB_CHECK(st == IVS_OK && r.stop_reason == IVS_STOP_GUEST_FAULT && r.units_executed == 0u);
  ivs_ms_free(&x);
  IVS_CHECK(rb_custom(&x, rb_prog_halt, RB_N(rb_prog_halt)));
  st = rb_run(x.m, IVS_RUN_F_STOP_ON_HALT, IVS_RUN_TARGET_MAX, &r);
  RB_CHECK(st == IVS_OK && r.stop_reason == IVS_STOP_CPU_HALTED && r.overshoot_ticks == 0u);
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* ---- precedence: one case per adjacent ladder pair (D-05) ---- */

/* Stage 1 over 2: req NULL with a NULL handle is BAD_ARG (RUN_NULL_ARG). */
static int rb_prec_arg_over_handle(void)
{
  ivs_ms_inst x;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
  RB_CHECK(rb_refused(&x, 1, NULL, NULL, RB_V1_REP, IVS_E_BAD_ARG, IVS_REASON_RUN_NULL_ARG));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* Stage 2 over 3: a destroyed handle with the report inside its storage is BAD_HANDLE (the overlap
 * is tested only against a valid instance), and nothing is written into that storage. */
static int rb_prec_handle_over_overlap(void)
{
  ivs_ms_inst x;
  ivs_run_snap s;
  rb_req_buf q;
  ivs_run_report *in;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
  s.copy = NULL;
  RB_CHECK(ivs_machine_destroy(x.m) == IVS_OK);
  rb_req(&q, RB_V1_REQ, RUN_TEST_BAD_FLAG, 1000u);
  in = (ivs_run_report *)(void *)(x.storage + rb_inside_off());
  in->struct_size = RB_V1_REP;
  RB_CHECK(ivs_run_snap_take(&s, &x, 0));
  RB_CHECK(ivs_machine_run(x.m, &q.q, in) == IVS_E_BAD_HANDLE);
  RB_CHECK(ivs_run_snap_same(&s, &x));
  rc = IVS_TEST_PASS;
done:
  ivs_run_snap_free(&s);
  ivs_ms_free(&x);
  return rc;
}

/* Stage 3 over 4: the report inside the instance with the reserved frame flag is BAD_ARG, nothing
 * written. */
static int rb_prec_overlap_over_unsupported(void)
{
  ivs_ms_inst x;
  ivs_run_snap s;
  rb_req_buf q;
  ivs_run_report *in;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
  s.copy = NULL;
  rb_req(&q, RB_V1_REQ, RUN_TEST_BAD_FLAG, 1000u);
  in = (ivs_run_report *)(void *)(x.storage + rb_inside_off());
  in->struct_size = RB_V1_REP;
  RB_CHECK(ivs_run_snap_take(&s, &x, 1));
  RB_CHECK(ivs_machine_run(x.m, &q.q, in) == IVS_E_BAD_ARG);
  RB_CHECK(ivs_run_snap_same(&s, &x));
  rc = IVS_TEST_PASS;
done:
  ivs_run_snap_free(&s);
  ivs_ms_free(&x);
  return rc;
}

/* Inside stage 4: an unknown flag over a nonzero tail (RUN_FLAGS, not RUN_TAIL). */
static int rb_prec_flags_over_tail(void)
{
  ivs_ms_inst x;
  rb_req_buf q;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RB_POISON));
  rb_req(&q, RB_V1_REQ + 8u, 4u, 1000u);
  q.b[RB_V1_REQ] = 1u;
  RB_CHECK(rb_refused(&x, 1, x.m, &q.q, RB_V1_REP, IVS_E_UNSUPPORTED, IVS_REASON_RUN_FLAGS));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* Stage 4 over 5: an OFF machine with the reserved frame flag, and with a nonzero tail, is
 * UNSUPPORTED. */
static int rb_prec_unsupported_over_state(void)
{
  ivs_ms_inst x;
  rb_req_buf q;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_new(&x, RB_POISON));
  rb_req(&q, RB_V1_REQ, RUN_TEST_BAD_FLAG, 1000u);
  RB_CHECK(rb_refused(&x, 1, x.m, &q.q, RB_V1_REP, IVS_E_UNSUPPORTED, IVS_REASON_RUN_FLAGS));
  rb_req(&q, RB_V1_REQ + 8u, 0u, 1000u);
  q.b[RB_V1_REQ + 7u] = 1u;
  RB_CHECK(rb_refused(&x, 1, x.m, &q.q, RB_V1_REP, IVS_E_UNSUPPORTED, IVS_REASON_RUN_TAIL));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* Stage 5 over 6: an OFF machine with a target above 2^63 is BAD_STATE. */
static int rb_prec_state_over_range(void)
{
  ivs_ms_inst x;
  rb_req_buf q;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_new(&x, RB_POISON));
  RB_CHECK(ivs_ms_content(x.m, IVS_MS_CONTENT_A));
  rb_req(&q, RB_V1_REQ, 0u, UINT64_MAX);
  RB_CHECK(rb_refused(&x, 1, x.m, &q.q, RB_V1_REP, IVS_E_BAD_STATE, IVS_REASON_RUN_STATE_OFF));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* ---- stop names (D-04) ---- */

static int rb_stop_name_known(void)
{
  IVS_CHECK(strcmp(ivs_run_stop_name(IVS_STOP_TARGET), "IVS_STOP_TARGET") == 0);
  IVS_CHECK(strcmp(ivs_run_stop_name(IVS_STOP_CPU_HALTED), "IVS_STOP_CPU_HALTED") == 0);
  IVS_CHECK(strcmp(ivs_run_stop_name(IVS_STOP_GUEST_FAULT), "IVS_STOP_GUEST_FAULT") == 0);
  IVS_CHECK(strcmp(ivs_run_stop_name(IVS_STOP_INTERNAL), "IVS_STOP_INTERNAL") == 0);
  IVS_CHECK(strcmp(ivs_run_stop_name(IVS_STOP_FRAME), "IVS_STOP_FRAME") == 0);
  IVS_CHECK(IVS_STOP_TARGET == 1u && IVS_STOP_CPU_HALTED == 2u && IVS_STOP_GUEST_FAULT == 3u &&
            IVS_STOP_INTERNAL == 4u && IVS_STOP_FRAME == 5u);
  return IVS_TEST_PASS;
}

static int rb_stop_name_unknown(void)
{
  static const uint32_t v[4] = {0u, 6u, 0x80000000u, 0xFFFFFFFFu};
  const char *first = ivs_run_stop_name(0u);
  uint32_t i;
  IVS_CHECK(first != NULL);
  for (i = 0u; i < 4u; i++) {
    const char *n = ivs_run_stop_name(v[i]);
    IVS_CHECK(n != NULL && strcmp(n, "IVS_STOP_UNKNOWN") == 0 && n == first);
  }
  return IVS_TEST_PASS;
}

/* run.stop.frame-name (plan 06-03, D-14): the fifth stop reason is named, and 0, 6 and the large values
 * stay unknown, all with one static string. */
static int rb_stop_frame_name(void)
{
  static const uint32_t v[4] = {0u, 6u, 0x80000000u, 0xFFFFFFFFu};
  const char *unknown = ivs_run_stop_name(0u);
  const char *frame = ivs_run_stop_name(IVS_STOP_FRAME);
  uint32_t i;
  IVS_CHECK(IVS_STOP_FRAME == 5u);
  IVS_CHECK(frame != NULL && strcmp(frame, "IVS_STOP_FRAME") == 0);
  IVS_CHECK(unknown != NULL && strcmp(unknown, "IVS_STOP_UNKNOWN") == 0 && frame != unknown);
  for (i = 0u; i < 4u; i++) {
    const char *n = ivs_run_stop_name(v[i]);
    IVS_CHECK(n != NULL && strcmp(n, "IVS_STOP_UNKNOWN") == 0 && n == unknown);
  }
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_run_bound_cases[] = {
    {"run.target.ends-at-first-boundary-ge-target", rb_target_first_boundary, NULL},
    {"run.target.on-boundary-no-overshoot", rb_target_on_boundary, NULL},
    {"run.target.already-reached", rb_target_already_reached, NULL},
    {"run.overshoot.equals-now-minus-target", rb_overshoot, NULL},
    {"run.single-step.equals-seam", rb_single_step, NULL},
    {"run.target.max-accepted", rb_target_max_accepted, NULL},
    {"run.stop.halt-flag", rb_stop_halt_flag, NULL},
    {"run.stop.halt-no-flag", rb_stop_halt_no_flag, NULL},
    {"run.stop.guest-fault", rb_stop_guest_fault, NULL},
    {"run.stop.fault-sticky", rb_stop_fault_sticky, NULL},
    {"run.stop.fault-before-halt", rb_stop_fault_before_halt, NULL},
    {"run.stop.internal-step-refused", rb_stop_internal, NULL},
    {"run.stop.internal-input-restored", rb_stop_internal_input, NULL},
    {"run.stop.frame-name", rb_stop_frame_name, NULL},
    {"run.buffers.wrap-overlap", rb_wrap_buffers, NULL},
    {"run.refuse.peek.out-wraps", rb_peek_out_wraps, NULL},
    {"run.refuse.null-request", rb_refuse_null_request, NULL},
    {"run.refuse.null-report", rb_refuse_null_report, NULL},
    {"run.refuse.struct-size-request", rb_refuse_struct_size_request, NULL},
    {"run.refuse.struct-size-report", rb_refuse_struct_size_report, NULL},
    {"run.refuse.bad-handle", rb_refuse_bad_handle, NULL},
    {"run.refuse.report-inside-instance", rb_refuse_report_inside, NULL},
    {"run.refuse.flags-unknown", rb_refuse_flags_unknown, NULL},
    {"run.flags.stop-on-frame-accepted", rb_flags_stop_on_frame_accepted, NULL},
    {"run.flags.stop-on-frame-clamps-target", rb_flags_stop_on_frame_clamps_target, NULL},
    {"run.flags.stop-reason-frame", rb_flags_stop_reason_frame, NULL},
    {"run.flags.report-v3-fields", rb_flags_report_v3_fields, NULL},
    {"run.refuse.tail-nonzero", rb_refuse_tail, NULL},
    {"run.refuse.state-off", rb_refuse_state_off, NULL},
    {"run.refuse.target-above-2-63", rb_refuse_target_range, NULL},
    {"run.prec.arg-over-handle", rb_prec_arg_over_handle, NULL},
    {"run.prec.handle-over-overlap", rb_prec_handle_over_overlap, NULL},
    {"run.prec.overlap-over-unsupported", rb_prec_overlap_over_unsupported, NULL},
    {"run.prec.flags-over-tail", rb_prec_flags_over_tail, NULL},
    {"run.prec.unsupported-over-state", rb_prec_unsupported_over_state, NULL},
    {"run.prec.state-over-range", rb_prec_state_over_range, NULL},
    {"run.stop-name.known", rb_stop_name_known, NULL},
    {"run.stop-name.unknown", rb_stop_name_unknown, NULL},
};
const size_t ivs_run_bound_count = sizeof ivs_run_bound_cases / sizeof ivs_run_bound_cases[0];
