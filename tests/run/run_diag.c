/* SPDX-License-Identifier: MIT */
/* Suite run, family diag (plan 04-16, D-27, D-31, RUN-06): the first project-authored diagnostic,
 * ivs-diag-cpu-1 (fixtures/FX-diag-cpu-1/diag-cpu-1.listing.txt), packed by tools/pack into the
 * build-tree IVSC container whose bytes ivs_images.inc carries, entered from a power-on reset
 * through the project boot stub (contract 1) and driven through the public API only.
 *   run.diag.predicate  load the stub EXEC, 2048 zero bytes as GROM (a project stand-in, not a GROM)
 *                       and the diagnostic cartridge; reset; one ivs_machine_run with
 *                       IVS_RUN_F_STOP_ON_HALT and a bound of 4000 CPU cycles (in ticks of the
 *                       profile's timebase); the stop reason is CPU_HALTED, and R0-R6 and the ten
 *                       memory words equal the predicate.
 * The expected values are not in this file: diag_cpu_1_expect.inc is generated at build time from
 * the assert lines of data/golden/diag-cpu-1.expect (tests/expect-inc.cmake), where they were
 * derived by hand from arithmetic and the cited manual tables before the program first ran. R7,
 * the flags at the stop and machine time are not part of the predicate (D-27). No image byte is
 * committed (D-30). */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_test.h"
#include "run_cases.h"

#include "ivs_images.inc"

#define RD_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      goto done;                                                                                       \
    }                                                                                                  \
  } while (0)

#define RD_POISON 0x3Cu
#define RD_ZERO_GROM_BYTES 2048u
/* The run bound of run.diag.predicate in CPU cycles (the hand-summed path is 520). */
#define RD_BOUND_CPU_CYCLES 4000u

/* ---- the predicate, from data/golden/diag-cpu-1.expect ---- */
#define RD_HALTED 0u
#define RD_REG 1u
#define RD_MEM 2u

typedef struct rd_clause {
  uint32_t kind;  /* RD_HALTED, RD_REG, RD_MEM */
  uint16_t where; /* register number or word address */
  uint16_t value; /* expected word; 1 or 0 for RD_HALTED */
} rd_clause;

static const rd_clause rd_expect[] = {
#define IVS_EXPECT_HALTED(y) {RD_HALTED, 0u, (uint16_t)(y)},
#define IVS_EXPECT_REG(r, v) {RD_REG, (uint16_t)(r), (uint16_t)(v)},
#define IVS_EXPECT_MEM(a, v) {RD_MEM, (uint16_t)(a), (uint16_t)(v)},
#include "diag_cpu_1_expect.inc"
#undef IVS_EXPECT_HALTED
#undef IVS_EXPECT_REG
#undef IVS_EXPECT_MEM
};
#define RD_N_EXPECT (sizeof rd_expect / sizeof rd_expect[0])

/* ---- helpers ---- */

/* A heap copy of n bytes at src (ivs_ms_load frees it), or NULL. */
static uint8_t *rd_copy(const uint8_t *src, size_t n)
{
  uint8_t *b = malloc(n);
  if (b != NULL) {
    memcpy(b, src, n);
  }
  return b;
}

/* A powered diagnostic instance in storage poisoned with poison: stub EXEC, zero GROM, the
 * diagnostic cartridge, a power-on reset. 1 on success. */
static int rd_powered(ivs_ms_inst *x, uint8_t poison)
{
  ivs_status st;
  if (!ivs_ms_new(x, poison)) {
    return 0;
  }
  if (!ivs_ms_load(x->m, IVS_KIND_EXEC, rd_copy(ivs_img_boot_stub, IVS_IMG_BOOT_STUB_BYTES),
                   IVS_IMG_BOOT_STUB_BYTES) ||
      !ivs_ms_load(x->m, IVS_KIND_GROM, calloc(RD_ZERO_GROM_BYTES, 1u), RD_ZERO_GROM_BYTES) ||
      !ivs_ms_load(x->m, IVS_KIND_CARTRIDGE, rd_copy(ivs_img_diag_cpu_1, IVS_IMG_DIAG_CPU_1_BYTES),
                   IVS_IMG_DIAG_CPU_1_BYTES)) {
    ivs_ms_free(x);
    return 0;
  }
  st = ivs_machine_reset(x->m, IVS_RESET_POWER_ON);
  if (st != IVS_OK) {
    ivs_test_diag("reset refused (%s)", ivs_status_name(st));
    ivs_ms_free(x);
    return 0;
  }
  return 1;
}

/* One public run from now with IVS_RUN_F_STOP_ON_HALT and a bound of cycles CPU cycles; the report
 * in *rep. 1 when the call returned IVS_OK. */
static int rd_run_cycles(ivs_machine *m, uint64_t cycles, ivs_run_report *rep)
{
  ivs_info info;
  ivs_run_request q;
  ivs_status st;
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info(m, &info) != IVS_OK || info.ticks_per_cpu_cycle == 0u) {
    ivs_test_diag("info refused or no timebase");
    return 0;
  }
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.flags = IVS_RUN_F_STOP_ON_HALT;
  q.target_ticks = info.time_ticks + cycles * (uint64_t)info.ticks_per_cpu_cycle;
  memset(rep, 0, sizeof *rep);
  rep->struct_size = (uint32_t)sizeof *rep;
  st = ivs_machine_run(m, &q, rep);
  if (st != IVS_OK) {
    ivs_test_diag("run refused (%s)", ivs_status_name(st));
    return 0;
  }
  return 1;
}

/* 1 when the stopped instance meets every clause of the predicate; each failing clause is reported
 * as expected versus actual. */
static int rd_meets(const ivs_machine *m, const ivs_run_report *rep)
{
  ivs_cpu_state_report c;
  size_t i;
  int ok = 1;
  memset(&c, 0, sizeof c);
  c.struct_size = (uint32_t)sizeof c;
  if (ivs_machine_cpu_state(m, &c) != IVS_OK) {
    ivs_test_diag("cpu_state refused");
    return 0;
  }
  for (i = 0u; i < RD_N_EXPECT; i++) {
    const rd_clause *e = &rd_expect[i];
    if (e->kind == RD_HALTED) {
      uint16_t halted = (uint16_t)(rep->stop_reason == IVS_STOP_CPU_HALTED && c.run_state == IVS_CPU_STATE_HALTED);
      if (halted != e->value) {
        ivs_test_diag("halted: expected %u, actual stop %s run_state %u", (unsigned)e->value,
                      ivs_run_stop_name(rep->stop_reason), (unsigned)c.run_state);
        ok = 0;
      }
    } else if (e->kind == RD_REG) {
      if (e->where > 7u || c.r[e->where] != e->value) {
        ivs_test_diag("reg R%u: expected %04X, actual %04X", (unsigned)e->where, (unsigned)e->value,
                      e->where > 7u ? 0u : (unsigned)c.r[e->where]);
        ok = 0;
      }
    } else {
      uint16_t w = 0u;
      ivs_status st = ivs_machine_peek(m, e->where, 1u, &w);
      if (st != IVS_OK || w != e->value) {
        ivs_test_diag("mem %04X: expected %04X, actual %04X (peek %s)", (unsigned)e->where, (unsigned)e->value,
                      (unsigned)w, ivs_status_name(st));
        ok = 0;
      }
    }
  }
  return ok;
}

/* ---- cases ---- */

static int rd_predicate(void)
{
  ivs_ms_inst x;
  ivs_run_report rep;
  size_t i;
  uint32_t regs = 0u;
  uint32_t mems = 0u;
  uint32_t halts = 0u;
  int rc = IVS_TEST_FAIL;
  /* The generated predicate has the shape D-27 names: halted, R0-R6 and ten memory words. */
  for (i = 0u; i < RD_N_EXPECT; i++) {
    regs += rd_expect[i].kind == RD_REG ? 1u : 0u;
    mems += rd_expect[i].kind == RD_MEM ? 1u : 0u;
    halts += rd_expect[i].kind == RD_HALTED ? 1u : 0u;
  }
  if (halts != 1u || regs != 7u || mems != 10u) {
    ivs_test_diag("predicate shape: %u halted, %u reg, %u mem clauses; want 1, 7, 10", (unsigned)halts,
                  (unsigned)regs, (unsigned)mems);
    return IVS_TEST_FAIL;
  }
  if (!rd_powered(&x, RD_POISON)) {
    return IVS_TEST_FAIL;
  }
  RD_CHECK(rd_run_cycles(x.m, RD_BOUND_CPU_CYCLES, &rep));
  RD_CHECK(rep.stop_reason == IVS_STOP_CPU_HALTED);
  RD_CHECK(rd_meets(x.m, &rep));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

const ivs_test_case ivs_run_diag_cases[] = {
    {"run.diag.predicate", rd_predicate, NULL},
};
const size_t ivs_run_diag_count = sizeof ivs_run_diag_cases / sizeof ivs_run_diag_cases[0];
