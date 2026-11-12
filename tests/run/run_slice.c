/* SPDX-License-Identifier: MIT */
/* Suite run, family slice (D-07, API-02, plan 04-09): slicing invariance of ivs_machine_run. The
 * whole instance is the resumable state, so any split of one run into calls must give the state,
 * digests and counters of a unit-by-unit reference.
 *
 * Reference: a twin of the same program stepped one unit at a time through the internal step seam
 * (the primitive run.single-step.equals-seam ties run(now + 1) to), never through the call under
 * test, so a run-path defect cannot move its own oracle. It records the boundary times b[0..n]
 * and the CPU-domain digest at each boundary, and fixes the end target E from the program's own
 * stop: the HALTED boundary + 41 (inside a 4-tick idle unit, so the last call overshoots) for
 * program H, the FAULTED boundary + 1000 for program F (later calls run nothing).
 *
 * Each pattern replays the program on a fresh identical instance as a sequence of calls; every
 * target but the single-step one is absolute, the last call's target is E:
 *   single     run(now + 1) until now >= E or the CPU faults, then run(E);
 *   fixed-7    targets 7, 14, 21, ...;
 *   prime      slices of 2, 3, 5, ... 61 ticks, cycling;
 *   seeded     slices of 1..61 ticks from xorshift32 with a fixed seed (no host time or
 *              randomness);
 *   one-call   run(E).
 * Per call (oracle from the reference only): status IVS_OK; the call ends at reference boundary
 * j = the first boundary at or after the target from the entry boundary (the FAULTED boundary at
 * most); now_ticks = b[j], units_executed = j - entry index, elapsed_ticks = b[j] - b[entry],
 * overshoot = max(b[j] - target, 0), the stop reason (GUEST_FAULT at the FAULTED boundary, else
 * TARGET), and the CPU-domain digest equal to the reference's at b[j]. Sampled ends (the first call
 * end at or after the reference midpoint, and the end): the composite and the seven domain digests
 * and the whole instance storage equal a second seam-stepped twin at the same unit count. Summed
 * units and elapsed ticks equal the reference end. Calls are capped, so a defect that stops
 * progress fails instead of hanging.
 *   run.slice.edges  targets exactly on a boundary, one tick after one, below now and equal to
 *                    now, then E, under the same oracle.
 * Programs (EXEC at $1000, then NOP fill; decles hand-assembled from the project's Phase 2 vector
 * data in data/cp1610/vectors, read from the GI CP-1600 Users Manual tables; no assembler):
 *   MVII #8, R1    $02B9 $0008   (immediate.json, cpu.MVII.r1-load)
 *   DECR R1        $0011         (register.json, DECR: 0000 010 DDD)
 *   BNZE $1002     $022C $0002   (branch.json, cpu.BNZE.back-taken: target = PC+2 + ~d, so from
 *                                 $1003, PC+2 = $1005 and d = 2 gives $1002)
 *   H: HLT $0000, then TSTR R1 $0089 (register.json, cpu.TSTR.base) as the follower
 *   F: MVO@ R4, R4 $0264, a sticky guest fault (indirect.json, cpu.MVO@.sss-eq-mmm-r4-unsupported)
 * Cost: one CPU digest per call, full digests and storage only at two sampled ends.
 * ivs_run_slice_halting (run_cases.h, plan 04-16) runs the five patterns on another halting
 * program that a caller powers up, so the same oracle checks the real diagnostic (D-07). */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "run_cases.h"

#define RS_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      goto done;                                                                                       \
    }                                                                                                  \
  } while (0)

#define RS_POISON 0xA5u
/* Reference boundaries at most (program H needs about 40). */
#define RS_MAX_UNITS 128u
/* Calls per pattern at most (fixed-7 needs E / 7 + 2, below 200). */
#define RS_MAX_CALLS 1024u
#define RS_SEED 0x9E3779B9u

/* Program H: loop of 8, HLT, TSTR R1 follower. Program F: the same loop, then a guest fault. */
static const uint16_t rs_prog_halt[] = {0x02B9u, 0x0008u, 0x0011u, 0x022Cu, 0x0002u, 0x0000u, 0x0089u};
static const uint16_t rs_prog_fault[] = {0x02B9u, 0x0008u, 0x0011u, 0x022Cu, 0x0002u, 0x0264u};
#define RS_N(a) (sizeof(a) / sizeof((a)[0]))

#define RS_PROG_HALT 0u
#define RS_PROG_FAULT 1u

/* A program under test: powered builds a POWERED instance of it; faults is 1 when it ends in a
 * guest fault (end E = FAULTED boundary + 1000), 0 when it halts (E = HALTED boundary + 41). */
typedef struct rs_prog {
  ivs_run_powered_fn powered;
  int faults;
} rs_prog;

#define RS_SINGLE 0u
#define RS_FIXED_7 1u
#define RS_PRIME 2u
#define RS_SEEDED 3u
#define RS_ONE_CALL 4u
#define RS_EDGES 5u

static const uint32_t rs_primes[] = {2u, 3u, 5u, 7u, 11u, 13u, 17u, 19u, 23u, 29u, 31u, 37u, 41u, 43u, 47u, 53u, 59u, 61u};

static const uint32_t rs_domains[8] = {IVS_DIGEST_COMPOSITE, IVS_DIGEST_CPU,   IVS_DIGEST_MEM,   IVS_DIGEST_IMAGES,
                                       IVS_DIGEST_INSTANCE,  IVS_DIGEST_VIDEO, IVS_DIGEST_AUDIO, IVS_DIGEST_INPUT};

typedef struct rs_ref {
  uint64_t b[RS_MAX_UNITS + 1u];
  uint8_t cd[RS_MAX_UNITS + 1u][32];
  uint32_t n;     /* last recorded boundary index */
  uint32_t fault; /* index of the FAULTED boundary, or UINT32_MAX */
  uint64_t end;   /* E */
} rs_ref;

/* A POWERED instance running program prog (EXEC: the decles, then NOP fill; GROM: the stand-in
 * pattern; no cartridge). 1 on success. */
static int rs_custom(ivs_ms_inst *x, uint32_t prog)
{
  const uint16_t *p = prog == RS_PROG_HALT ? rs_prog_halt : rs_prog_fault;
  size_t n = prog == RS_PROG_HALT ? RS_N(rs_prog_halt) : RS_N(rs_prog_fault);
  uint8_t *exec;
  size_t i;
  if (!ivs_ms_new(x, RS_POISON)) {
    return 0;
  }
  exec = malloc(IVS_MS_EXEC_BYTES);
  if (exec == NULL) {
    ivs_ms_free(x);
    return 0;
  }
  for (i = 0u; i < IVS_MS_EXEC_BYTES / 2u; i++) {
    uint16_t w = i < n ? p[i] : 0x0034u;
    exec[2u * i] = (uint8_t)(w >> 8);
    exec[2u * i + 1u] = (uint8_t)(w & 0xFFu);
  }
  if (!ivs_ms_load(x->m, IVS_KIND_EXEC, exec, IVS_MS_EXEC_BYTES) ||
      !ivs_ms_load(x->m, IVS_KIND_GROM, ivs_ms_grom(0u), IVS_MS_GROM_BYTES) ||
      ivs_machine_reset(x->m, IVS_RESET_POWER_ON) != IVS_OK) {
    ivs_test_diag("program content refused");
    ivs_ms_free(x);
    return 0;
  }
  return 1;
}

/* Machine time (info), or UINT64_MAX when refused. */
static uint64_t rs_now(const ivs_machine *m)
{
  ivs_info info;
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info(m, &info) != IVS_OK) {
    return UINT64_MAX;
  }
  return info.time_ticks;
}

/* The CPU run state (cpu_state), or UINT32_MAX when refused. */
static uint32_t rs_state(const ivs_machine *m)
{
  ivs_cpu_state_report c;
  memset(&c, 0, sizeof c);
  c.struct_size = (uint32_t)sizeof c;
  if (ivs_machine_cpu_state(m, &c) != IVS_OK) {
    return UINT32_MAX;
  }
  return c.run_state;
}

/* The reference of program prog (header comment). 1 on success. */
static int rs_reference(const rs_prog *prog, rs_ref *r)
{
  ivs_ms_inst y;
  uint32_t i;
  int ok = 0;
  memset(r, 0, sizeof *r);
  r->fault = UINT32_MAX;
  r->end = UINT64_MAX;
  if (!prog->powered(&y)) {
    return 0;
  }
  r->b[0] = rs_now(y.m);
  if (r->b[0] != 0u || ivs_machine_state_digest(y.m, IVS_DIGEST_CPU, r->cd[0]) != IVS_OK) {
    goto done;
  }
  for (i = 1u; i <= RS_MAX_UNITS; i++) {
    uint32_t s;
    if (!ivs_ms_step(y.m, 1u, NULL)) {
      goto done;
    }
    r->b[i] = rs_now(y.m);
    if (r->b[i] <= r->b[i - 1u] || r->b[i] == UINT64_MAX ||
        ivs_machine_state_digest(y.m, IVS_DIGEST_CPU, r->cd[i]) != IVS_OK) {
      ivs_test_diag("reference unit %u made no progress", (unsigned)i);
      goto done;
    }
    s = rs_state(y.m);
    if (prog->faults && s == IVS_CPU_STATE_FAULTED) {
      r->n = i;
      r->fault = i;
      r->end = r->b[i] + 1000u;
      ok = 1;
      goto done;
    }
    if (!prog->faults && s == IVS_CPU_STATE_HALTED && r->end == UINT64_MAX) {
      r->end = r->b[i] + 41u;
    }
    if (r->b[i] >= r->end) {
      r->n = i;
      ok = 1;
      goto done;
    }
  }
  ivs_test_diag("reference did not reach its end within %u units", (unsigned)RS_MAX_UNITS);
done:
  ivs_ms_free(&y);
  return ok;
}

/* The boundary index a call with target t entered at index from must end at. */
static uint32_t rs_expect(const rs_ref *r, uint32_t from, uint64_t t)
{
  uint32_t j = from;
  while (j < r->fault && j < r->n && r->b[j] < t) {
    j++;
  }
  return j;
}

/* 1 when x and y have equal composite and domain digests and identical storage. */
static int rs_full_same(const ivs_ms_inst *x, const ivs_ms_inst *y)
{
  uint32_t i;
  for (i = 0u; i < 8u; i++) {
    uint8_t dx[32];
    uint8_t dy[32];
    if (ivs_machine_state_digest(x->m, rs_domains[i], dx) != IVS_OK ||
        ivs_machine_state_digest(y->m, rs_domains[i], dy) != IVS_OK || memcmp(dx, dy, 32u) != 0) {
      ivs_test_diag("digest of domain %u differs from the reference", (unsigned)rs_domains[i]);
      return 0;
    }
  }
  if (memcmp(x->storage, y->storage, IVS_MACHINE_BYTES) != 0) {
    ivs_test_diag("instance storage differs from the reference");
    return 0;
  }
  return 1;
}

/* Replay state of one pattern. */
typedef struct rs_play {
  const rs_ref *r;
  ivs_ms_inst x;    /* the sliced instance */
  ivs_ms_inst y;    /* the seam twin for sampled ends */
  uint32_t at;      /* reference index x is at */
  uint32_t y_at;    /* reference index y is at */
  uint32_t calls;
  uint32_t sampled; /* 1 after the midpoint sample */
  uint64_t units;   /* summed units_executed */
  uint64_t elapsed; /* summed elapsed_ticks */
} rs_play;

/* Steps the twin to index j and compares everything. 1 on success. */
static int rs_sample(rs_play *p, uint32_t j)
{
  if (j < p->y_at || !ivs_ms_step(p->y.m, j - p->y_at, NULL)) {
    ivs_test_diag("twin cannot reach index %u", (unsigned)j);
    return 0;
  }
  p->y_at = j;
  return rs_full_same(&p->x, &p->y);
}

/* One call with target t under the per-call oracle (header comment). 1 on success. */
static int rs_call(rs_play *p, uint64_t t)
{
  const rs_ref *r = p->r;
  ivs_run_request q;
  ivs_run_report rep;
  ivs_status st;
  uint8_t cd[32];
  uint32_t j = rs_expect(r, p->at, t);
  uint32_t stop = j == r->fault ? IVS_STOP_GUEST_FAULT : IVS_STOP_TARGET;
  uint64_t over = r->b[j] > t ? r->b[j] - t : 0u;
  if (p->calls >= RS_MAX_CALLS) {
    ivs_test_diag("more than %u calls", (unsigned)RS_MAX_CALLS);
    return 0;
  }
  p->calls++;
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.target_ticks = t;
  memset(&rep, RS_POISON, sizeof rep);
  rep.struct_size = (uint32_t)sizeof rep;
  st = ivs_machine_run(p->x.m, &q, &rep);
  if (st != IVS_OK || rep.now_ticks != r->b[j] || rep.units_executed != (uint64_t)(j - p->at) ||
      rep.elapsed_ticks != r->b[j] - r->b[p->at] || rep.overshoot_ticks != over || rep.stop_reason != stop) {
    ivs_test_diag("call %u target %llu from boundary %u: status %d now %llu units %llu elapsed %llu over %llu "
                  "stop %u; want now %llu (boundary %u) units %u elapsed %llu over %llu stop %u",
                  (unsigned)p->calls, (unsigned long long)t, (unsigned)p->at, st,
                  (unsigned long long)rep.now_ticks, (unsigned long long)rep.units_executed,
                  (unsigned long long)rep.elapsed_ticks, (unsigned long long)rep.overshoot_ticks,
                  (unsigned)rep.stop_reason, (unsigned long long)r->b[j], (unsigned)j, (unsigned)(j - p->at),
                  (unsigned long long)(r->b[j] - r->b[p->at]), (unsigned long long)over, (unsigned)stop);
    return 0;
  }
  if (ivs_machine_state_digest(p->x.m, IVS_DIGEST_CPU, cd) != IVS_OK || memcmp(cd, r->cd[j], 32u) != 0) {
    ivs_test_diag("call %u: CPU digest differs from the reference at boundary %u", (unsigned)p->calls, (unsigned)j);
    return 0;
  }
  p->units += rep.units_executed;
  p->elapsed += rep.elapsed_ticks;
  p->at = j;
  if (!p->sampled && 2u * j >= r->n) {
    p->sampled = 1u;
    if (!rs_sample(p, j)) {
      return 0;
    }
  }
  return 1;
}

/* Runs pattern pat of program prog against its reference. */
static int rs_case(const rs_prog *prog, uint32_t pat)
{
  rs_ref *r = malloc(sizeof *r);
  rs_play p;
  uint64_t t = 0u;
  uint32_t k = 0u;
  uint32_t seed = RS_SEED;
  int rc = IVS_TEST_FAIL;
  memset(&p, 0, sizeof p);
  if (r == NULL || !rs_reference(prog, r)) {
    free(r);
    return IVS_TEST_FAIL;
  }
  p.r = r;
  if (!prog->powered(&p.x)) {
    free(r);
    return IVS_TEST_FAIL;
  }
  if (!prog->powered(&p.y)) {
    ivs_ms_free(&p.x);
    free(r);
    return IVS_TEST_FAIL;
  }
  switch (pat) {
  case RS_SINGLE:
    while (r->b[p.at] < r->end && p.at != r->fault) {
      RS_CHECK(rs_call(&p, r->b[p.at] + 1u));
    }
    break;
  case RS_EDGES:
    RS_CHECK(r->n > 6u);
    RS_CHECK(rs_call(&p, r->b[3]));          /* exactly on a boundary: no overshoot */
    RS_CHECK(rs_call(&p, r->b[4] + 1u));     /* one tick after a boundary: ends at b[5] */
    RS_CHECK(rs_call(&p, r->b[5] - 1u));     /* below now: nothing runs */
    RS_CHECK(rs_call(&p, r->b[5]));          /* equal to now: nothing runs */
    break;
  case RS_ONE_CALL:
    break;
  default:
    for (;;) {
      uint64_t step;
      if (pat == RS_FIXED_7) {
        step = 7u;
      } else if (pat == RS_PRIME) {
        step = rs_primes[k % (sizeof rs_primes / sizeof rs_primes[0])];
      } else {
        seed ^= seed << 13;
        seed ^= seed >> 17;
        seed ^= seed << 5;
        step = 1u + (uint64_t)(seed % 61u);
      }
      k++;
      t += step;
      if (t >= r->end) {
        break;
      }
      RS_CHECK(rs_call(&p, t));
    }
    break;
  }
  RS_CHECK(rs_call(&p, r->end));
  RS_CHECK(p.at == r->n);
  RS_CHECK(p.units == (uint64_t)r->n && p.elapsed == r->b[r->n]);
  RS_CHECK(rs_sample(&p, p.at));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&p.y);
  ivs_ms_free(&p.x);
  free(r);
  return rc;
}

static int rs_powered_halt(ivs_ms_inst *x) { return rs_custom(x, RS_PROG_HALT); }
static int rs_powered_fault(ivs_ms_inst *x) { return rs_custom(x, RS_PROG_FAULT); }
static const rs_prog rs_halt = {rs_powered_halt, 0};
static const rs_prog rs_fault = {rs_powered_fault, 1};

static int rs_halt_single(void) { return rs_case(&rs_halt, RS_SINGLE); }
static int rs_halt_fixed(void) { return rs_case(&rs_halt, RS_FIXED_7); }
static int rs_halt_prime(void) { return rs_case(&rs_halt, RS_PRIME); }
static int rs_halt_seeded(void) { return rs_case(&rs_halt, RS_SEEDED); }
static int rs_halt_one(void) { return rs_case(&rs_halt, RS_ONE_CALL); }
static int rs_fault_single(void) { return rs_case(&rs_fault, RS_SINGLE); }
static int rs_fault_fixed(void) { return rs_case(&rs_fault, RS_FIXED_7); }
static int rs_fault_prime(void) { return rs_case(&rs_fault, RS_PRIME); }
static int rs_fault_seeded(void) { return rs_case(&rs_fault, RS_SEEDED); }
static int rs_fault_one(void) { return rs_case(&rs_fault, RS_ONE_CALL); }
static int rs_edges(void) { return rs_case(&rs_halt, RS_EDGES); }

int ivs_run_slice_halting(ivs_run_powered_fn powered)
{
  static const uint32_t pats[] = {RS_SINGLE, RS_FIXED_7, RS_PRIME, RS_SEEDED, RS_ONE_CALL};
  static const char *const names[] = {"single", "fixed-7", "prime", "seeded", "one-call"};
  rs_prog prog;
  size_t i;
  prog.powered = powered;
  prog.faults = 0;
  for (i = 0u; i < sizeof pats / sizeof pats[0]; i++) {
    if (rs_case(&prog, pats[i]) != IVS_TEST_PASS) {
      ivs_test_diag("slicing pattern %s failed", names[i]);
      return IVS_TEST_FAIL;
    }
  }
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_run_slice_cases[] = {
    {"run.slice.halt.single", rs_halt_single, NULL},
    {"run.slice.halt.fixed-7", rs_halt_fixed, NULL},
    {"run.slice.halt.prime", rs_halt_prime, NULL},
    {"run.slice.halt.seeded", rs_halt_seeded, NULL},
    {"run.slice.halt.one-call", rs_halt_one, NULL},
    {"run.slice.fault.single", rs_fault_single, NULL},
    {"run.slice.fault.fixed-7", rs_fault_fixed, NULL},
    {"run.slice.fault.prime", rs_fault_prime, NULL},
    {"run.slice.fault.seeded", rs_fault_seeded, NULL},
    {"run.slice.fault.one-call", rs_fault_one, NULL},
    {"run.slice.edges", rs_edges, NULL},
};
const size_t ivs_run_slice_count = sizeof ivs_run_slice_cases / sizeof ivs_run_slice_cases[0];
