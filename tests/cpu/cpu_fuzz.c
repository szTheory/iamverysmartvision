/* SPDX-License-Identifier: MIT */
/* Suite cpu-fuzz (D-12; threats T-02-32, T-02-33): a seeded robustness run of ivs_cp1610_step.
 * Each case is one fixed 32-bit seed driving an in-repo xorshift32 generator; nothing reads a
 * host source of randomness or time, so every run is identical. Per seed FUZZ_STEPS steps run
 * over a stateless fuzz bus whose read data and wait replies (0..40 cycles) are a fixed function
 * of the seed and the address, from random valid starting states (re-drawn every FUZZ_REDRAW
 * steps), with random lines and a random valid config on every step. A state that ends a step in
 * FAULTED or NOT_RESET is reset after its no-progress step has been checked.
 *
 * Invariants checked on every step:
 *   - the step returns IVS_OK, n_accesses <= IVS_CP1610_MAX_ACCESSES and equals the bus calls;
 *   - elapsed <= 14 + 4 * wait_limit + release_latency + 1 (FUZZ_BOUND);
 *   - the state validates afterwards;
 *   - repeating the step from a copy of the starting state gives the same status, result, state
 *     and access log (determinism, D-05);
 *   - a step that STARTS in FAULTED or UNRESET takes 0 cycles, makes no access and leaves the
 *     state unchanged (keyed on the run state read before the step; the step that enters FAULTED
 *     is a FAULT unit with elapsed above 0 and is held only to the bound);
 *   - HALTED and BUS_GRANT units take at least 1 cycle (the D-12 no-progress guard);
 *   - every record's address phase lies inside its unit (a TCI or PCIT strobe up to one cycle
 *     past it, C39), and a BUS_WAIT_LIMIT fault unit ends at the record that took the over-cap
 *     wait: no access, WRITE included, follows it (POL-WAIT-01, C53; review CR-01, WR-03).
 * Every FUZZ_CORRUPT_EVERY steps a corrupted copy (a run state past FAULTED, a nonzero reserved
 * byte, or a fault reason that contradicts the run state) must be refused with IVS_E_BAD_STATE
 * without a write or a bus call. Each seed must also reach every unit kind at least once, so the
 * run is not vacuous. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cpu/cp1610.h"
#include "ivs_test.h"

#define FUZZ_STEPS 20000u
#define FUZZ_REDRAW 256u
#define FUZZ_CORRUPT_EVERY 97u
#define FUZZ_UNIT_KINDS 8u

/* Fails the case with a diagnostic naming the seed, the step and the failed expression. */
#define FUZZ_CHECK(f, cond)                                                                            \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("seed=0x%08X step=%u: check failed at line %d: %s", (unsigned)(f)->seed,          \
                    (unsigned)(f)->step, __LINE__, #cond);                                             \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

typedef struct fuzz_ctx {
  uint32_t seed;
  uint32_t x; /* xorshift32 state, never 0 */
  uint32_t step;
  uint32_t calls;
  uint32_t units[FUZZ_UNIT_KINDS];
} fuzz_ctx;

/* xorshift32 (shifts 13, 17, 5): a full-period generator over the nonzero 32-bit values. */
static uint32_t fuzz_xorshift32(uint32_t x)
{
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return x;
}

static uint32_t fuzz_next(fuzz_ctx *f)
{
  f->x = fuzz_xorshift32(f->x);
  return f->x;
}

/* A value in 0..n-1 (n > 0); the slight modulo bias is irrelevant here. */
static uint32_t fuzz_below(fuzz_ctx *f, uint32_t n)
{
  return fuzz_next(f) % n;
}

/* The fixed reply word of an address under a seed: three xorshift rounds over seed and address. */
static uint32_t fuzz_hash(uint32_t seed, uint32_t addr)
{
  uint32_t h = seed ^ (addr * 0x9E3779B1u) ^ 0x5BD1E995u;
  if (h == 0u) {
    h = 1u;
  }
  return fuzz_xorshift32(fuzz_xorshift32(fuzz_xorshift32(h)));
}

/* Stateless bus: data and wait are a function of seed and address only. One address in eight
 * answers a wait of 0..40 cycles, so waits under, at and over the configured cap all occur. */
static ivs_bus_reply fuzz_bus(void *ctx, const ivs_cp1610_access *a)
{
  fuzz_ctx *f = (fuzz_ctx *)ctx;
  uint32_t h = fuzz_hash(f->seed, a->addr);
  ivs_bus_reply r;
  r.data = IVS_U16(h);
  r.wait_cpu_cycles = (uint16_t)(((h >> 16) & 7u) == 0u ? (h >> 19) % 41u : 0u);
  f->calls++;
  return r;
}

static uint8_t fuzz_ebci(void *ctx, uint8_t e)
{
  const fuzz_ctx *f = (const fuzz_ctx *)ctx;
  return (uint8_t)((fuzz_hash(f->seed, 0x10000u + (uint32_t)e) >> 7) & 1u);
}

/* A random valid state: random registers and flags, RUNNING or HALTED, or (one draw in eight)
 * an all-zero UNRESET state. */
static void fuzz_draw_state(fuzz_ctx *f, ivs_cp1610 *s)
{
  size_t i;
  memset(s, 0, sizeof *s);
  if (fuzz_below(f, 8u) == 0u) {
    return;
  }
  for (i = 0u; i < 8u; i++) {
    s->r[i] = IVS_U16(fuzz_next(f));
  }
  s->status = (uint8_t)(fuzz_next(f) & IVS_CP1610_STATUS_MASK);
  s->intff = (uint8_t)fuzz_below(f, 2u);
  s->dbd = (uint8_t)fuzz_below(f, 2u);
  s->prev_interruptible = (uint8_t)fuzz_below(f, 2u);
  s->halt_pending = (uint8_t)fuzz_below(f, 2u);
  s->busak = (uint8_t)fuzz_below(f, 2u);
  s->run_state = (uint8_t)(fuzz_below(f, 4u) == 0u ? IVS_CP1610_RS_HALTED : IVS_CP1610_RS_RUNNING);
  /* Keep the cross-field invariants of ivs_cp1610_state_validate (WR-01): no DBD or stop latch in
   * HALTED, and DBD only at a closed boundary without the grant. */
  if (s->run_state == IVS_CP1610_RS_HALTED) {
    s->dbd = 0u;
    s->halt_pending = 0u;
  }
  if (s->dbd != 0u) {
    s->prev_interruptible = 0u;
    s->busak = 0u;
  }
}

/* Lines: each of INTR, INTRM, BUSRQ high one step in four, a STPST edge one step in sixteen. */
static void fuzz_draw_lines(fuzz_ctx *f, ivs_cp1610_lines *l)
{
  l->intr = (uint8_t)(fuzz_below(f, 4u) == 0u);
  l->intrm = (uint8_t)(fuzz_below(f, 4u) == 0u);
  l->busrq = (uint8_t)(fuzz_below(f, 4u) == 0u);
  l->stpst = (uint8_t)(fuzz_below(f, 16u) == 0u);
}

/* A valid config: either INTFF behaviour, release latency 1..8, wait cap 35 (the profile value)
 * half the time, else 0..40. */
static void fuzz_draw_config(fuzz_ctx *f, ivs_cp1610_config *c)
{
  ivs_cp1610_config_default(c);
  c->interrupt_accept_clears_intff = (uint8_t)fuzz_below(f, 2u);
  c->release_latency_cpu_cycles = (uint8_t)(1u + fuzz_below(f, 8u));
  c->wait_limit_cpu_cycles = (uint8_t)(fuzz_below(f, 2u) == 0u ? 35u : fuzz_below(f, 41u));
}

/* A corrupted copy is refused with IVS_E_BAD_STATE, and neither the copy, the result nor the bus
 * is touched. */
static int fuzz_corrupt(fuzz_ctx *f, const ivs_cp1610 *s, const ivs_cp1610_config *cfg,
                        const ivs_cp1610_lines *l, const ivs_cp1610_port *port)
{
  ivs_cp1610 bad = *s;
  ivs_cp1610 seen;
  ivs_cp1610_result out;
  ivs_cp1610_result out_seen;
  ivs_cp1610_access log[IVS_CP1610_MAX_ACCESSES];
  uint32_t calls = f->calls;
  switch (fuzz_below(f, 3u)) {
  case 0u:
    bad.run_state = (uint8_t)(IVS_CP1610_RS_FAULTED + 1u + fuzz_below(f, 255u - IVS_CP1610_RS_FAULTED));
    break;
  case 1u:
    bad.reserved[fuzz_below(f, (uint32_t)sizeof bad.reserved)] = (uint8_t)(1u + fuzz_below(f, 255u));
    break;
  default:
    /* A fault reason that contradicts the run state (WR-01). */
    bad.fault_reason = (uint8_t)(bad.run_state == IVS_CP1610_RS_FAULTED ? IVS_CP1610_FAULT_NONE
                                                                         : IVS_CP1610_FAULT_BUS_WAIT_LIMIT);
    break;
  }
  seen = bad;
  memset(&out, 0xA5, sizeof out);
  out_seen = out;
  FUZZ_CHECK(f, ivs_cp1610_step(&bad, cfg, l, port, log, IVS_CP1610_MAX_ACCESSES, &out) == IVS_E_BAD_STATE);
  FUZZ_CHECK(f, memcmp(&bad, &seen, sizeof bad) == 0);
  FUZZ_CHECK(f, memcmp(&out, &out_seen, sizeof out) == 0);
  FUZZ_CHECK(f, f->calls == calls);
  return IVS_TEST_PASS;
}

static int fuzz_seed(uint32_t seed)
{
  fuzz_ctx f;
  ivs_cp1610 s;
  ivs_cp1610_port port;
  uint32_t k;
  memset(&f, 0, sizeof f);
  f.seed = seed;
  f.x = seed != 0u ? seed : 1u;
  port.bus = fuzz_bus;
  port.ebci = fuzz_ebci;
  port.ctx = &f;
  fuzz_draw_state(&f, &s);
  for (k = 0u; k < FUZZ_STEPS; k++) {
    ivs_cp1610 before;
    ivs_cp1610 again;
    ivs_cp1610_config cfg;
    ivs_cp1610_lines lines;
    ivs_cp1610_result r;
    ivs_cp1610_result r2;
    ivs_cp1610_access log[IVS_CP1610_MAX_ACCESSES];
    ivs_cp1610_access log2[IVS_CP1610_MAX_ACCESSES];
    ivs_status st;
    ivs_status st2;
    uint32_t calls;
    uint32_t bound;
    uint32_t i;
    uint8_t start;
    f.step = k;
    if (k > 0u && k % FUZZ_REDRAW == 0u) {
      fuzz_draw_state(&f, &s);
    }
    fuzz_draw_config(&f, &cfg);
    fuzz_draw_lines(&f, &lines);
    if (k % FUZZ_CORRUPT_EVERY == 0u && fuzz_corrupt(&f, &s, &cfg, &lines, &port) != IVS_TEST_PASS) {
      return IVS_TEST_FAIL;
    }
    before = s;
    start = s.run_state;
    calls = f.calls;
    st = ivs_cp1610_step(&s, &cfg, &lines, &port, log, IVS_CP1610_MAX_ACCESSES, &r);
    FUZZ_CHECK(&f, st == IVS_OK);
    FUZZ_CHECK(&f, r.n_accesses <= IVS_CP1610_MAX_ACCESSES);
    FUZZ_CHECK(&f, f.calls - calls == (uint32_t)r.n_accesses);
    bound = 14u + 4u * (uint32_t)cfg.wait_limit_cpu_cycles + (uint32_t)cfg.release_latency_cpu_cycles + 1u;
    FUZZ_CHECK(&f, r.elapsed_cpu_cycles <= bound);
    FUZZ_CHECK(&f, ivs_cp1610_state_validate(&s) == IVS_OK);
    FUZZ_CHECK(&f, r.unit_kind >= 1u && r.unit_kind < FUZZ_UNIT_KINDS);
    f.units[r.unit_kind]++;

    again = before;
    st2 = ivs_cp1610_step(&again, &cfg, &lines, &port, log2, IVS_CP1610_MAX_ACCESSES, &r2);
    FUZZ_CHECK(&f, st2 == st);
    FUZZ_CHECK(&f, memcmp(&r2, &r, sizeof r) == 0);
    FUZZ_CHECK(&f, memcmp(&again, &s, sizeof s) == 0);
    FUZZ_CHECK(&f, memcmp(log2, log, (size_t)r.n_accesses * sizeof log[0]) == 0);

    if (start == IVS_CP1610_RS_FAULTED || start == IVS_CP1610_RS_UNRESET) {
      FUZZ_CHECK(&f, r.elapsed_cpu_cycles == 0u);
      FUZZ_CHECK(&f, r.n_accesses == 0u);
      FUZZ_CHECK(&f, memcmp(&s, &before, sizeof s) == 0);
      FUZZ_CHECK(&f, r.unit_kind == (start == IVS_CP1610_RS_FAULTED ? IVS_CP1610_UNIT_FAULT : IVS_CP1610_UNIT_NOT_RESET));
      FUZZ_CHECK(&f, ivs_cp1610_reset(&s) == IVS_OK);
    } else if (r.unit_kind == IVS_CP1610_UNIT_FAULT) {
      FUZZ_CHECK(&f, s.run_state == IVS_CP1610_RS_FAULTED && r.elapsed_cpu_cycles > 0u);
      if (s.fault_reason == IVS_CP1610_FAULT_BUS_WAIT_LIMIT) {
        /* The unit ends at the access whose wait exceeds the cap: it is the last record. */
        FUZZ_CHECK(&f, r.n_accesses > 0u);
        FUZZ_CHECK(&f, log[r.n_accesses - 1u].wait_cpu_cycles > cfg.wait_limit_cpu_cycles);
        for (i = 0u; i + 1u < (uint32_t)r.n_accesses; i++) {
          FUZZ_CHECK(&f, log[i].wait_cpu_cycles <= cfg.wait_limit_cpu_cycles);
        }
      }
    }
    /* Every record sits inside its unit; the TCI and PCIT strobes may sit one cycle past (C39). */
    for (i = 0u; i < (uint32_t)r.n_accesses; i++) {
      if (log[i].kind == IVS_ACC_STROBE_TCI || log[i].kind == IVS_ACC_STROBE_PCIT) {
        FUZZ_CHECK(&f, log[i].offset_cpu_cycles <= r.elapsed_cpu_cycles + 1u);
      } else {
        FUZZ_CHECK(&f, log[i].offset_cpu_cycles < r.elapsed_cpu_cycles);
      }
    }
    if (r.unit_kind == IVS_CP1610_UNIT_HALTED || r.unit_kind == IVS_CP1610_UNIT_BUS_GRANT) {
      FUZZ_CHECK(&f, r.elapsed_cpu_cycles >= 1u);
    }
  }
  ivs_test_diag("seed=0x%08X steps=%u instr=%u int_entry=%u reset_entry=%u bus_grant=%u halted=%u fault=%u "
                "not_reset=%u",
                (unsigned)seed, (unsigned)FUZZ_STEPS, (unsigned)f.units[IVS_CP1610_UNIT_INSTR],
                (unsigned)f.units[IVS_CP1610_UNIT_INT_ENTRY], (unsigned)f.units[IVS_CP1610_UNIT_RESET_ENTRY],
                (unsigned)f.units[IVS_CP1610_UNIT_BUS_GRANT], (unsigned)f.units[IVS_CP1610_UNIT_HALTED],
                (unsigned)f.units[IVS_CP1610_UNIT_FAULT], (unsigned)f.units[IVS_CP1610_UNIT_NOT_RESET]);
  for (k = 1u; k < FUZZ_UNIT_KINDS; k++) {
    f.step = FUZZ_STEPS;
    FUZZ_CHECK(&f, f.units[k] > 0u);
  }
  return IVS_TEST_PASS;
}

/* Fixed seeds: arbitrary odd 32-bit constants, one per case. */
static const uint32_t fuzz_seeds[] = {0x2545F491u, 0x9E3779B9u, 0x1B873593u, 0xCC9E2D51u,
                                      0x85EBCA6Bu, 0xC2B2AE35u, 0x27D4EB2Fu, 0x165667B1u};

static const char *const fuzz_ids[] = {"cpu-fuzz.seed-01", "cpu-fuzz.seed-02", "cpu-fuzz.seed-03",
                                       "cpu-fuzz.seed-04", "cpu-fuzz.seed-05", "cpu-fuzz.seed-06",
                                       "cpu-fuzz.seed-07", "cpu-fuzz.seed-08"};

_Static_assert(sizeof fuzz_seeds / sizeof fuzz_seeds[0] == sizeof fuzz_ids / sizeof fuzz_ids[0],
               "one id per seed");

static const char *fuzz_id(const void *data, size_t i)
{
  (void)data;
  return fuzz_ids[i];
}

static int fuzz_run(const void *data, size_t i)
{
  (void)data;
  return fuzz_seed(fuzz_seeds[i]);
}

int main(int argc, char **argv)
{
  ivs_test_suite suite;
  memset(&suite, 0, sizeof suite);
  suite.name = "cpu-fuzz";
  suite.count = sizeof fuzz_ids / sizeof fuzz_ids[0];
  suite.id = fuzz_id;
  suite.run = fuzz_run;
  return ivs_test_main(argc, argv, &suite);
}
