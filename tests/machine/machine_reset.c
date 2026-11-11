/* SPDX-License-Identifier: MIT */
/* Suite machine, family reset (D-05, D-14, D-20, A-08 d, e, plan 03-12): the power-on reset
 * through the public header and one CPU unit at a time through the internal step seam
 * ivs_machine_step_unit_internal (machine/machine.h, never public).
 *
 * The tracer case runs a project-authored stand-in program (program A of machine_standin.c, its
 * decles hand-assembled from the project's Phase 2 vector rows) on the profile-driven bus,
 * entered through the profile's reset vector, and sees its RAM writes. Expected values are the
 * program's own arithmetic (R1 starts at $155 and is stored, then incremented, each pass), the
 * profile's reset.r7 ($1000) and ticks_per_cpu_cycle (4, docs/time-base.md) as literals, and the
 * sum of the cycles the seam reports; none is read back from the state under test except the
 * values being checked. */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cpu/cp1610.h"
#include "ivs/ivs.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "machine_cases.h"
#include "memmap/memsys.h"

#define IVS_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

#define MR_RESET_R7 0x1000u      /* profile reset.r7 */
#define MR_TICKS_PER_CYCLE 4u    /* profile ticks_per_cpu_cycle (D2) */
#define MR_MAX_UNITS 64u

/* The instance behind a test handle (the tests may read internals through machine.h). */
static const struct ivs_machine *mr_im(const ivs_machine *m)
{
  return ivs_machine_from_handle(m);
}

/* Tracer: load the stand-ins, reset, step program A until its store has run twice. */
static int mr_step_program(void)
{
  ivs_ms_inst x;
  const struct ivs_machine *im;
  ivs_info info;
  uint64_t cycles = 0u;
  uint32_t units = 0u;
  uint32_t stores = 0u;
  int rc = IVS_TEST_FAIL;
  if (!ivs_ms_new(&x, 0x5Au)) {
    return IVS_TEST_FAIL;
  }
  if (!ivs_ms_load(x.m, IVS_KIND_EXEC, ivs_ms_exec(IVS_MS_PROG_A), IVS_MS_EXEC_BYTES) ||
      !ivs_ms_load(x.m, IVS_KIND_GROM, ivs_ms_grom(0u), IVS_MS_GROM_BYTES)) {
    goto done;
  }
  if (ivs_machine_reset(x.m, IVS_RESET_POWER_ON) != IVS_OK) {
    ivs_test_diag("reset refused");
    goto done;
  }
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info(x.m, &info) != IVS_OK || info.phase != IVS_PHASE_POWERED || info.time_ticks != 0u) {
    ivs_test_diag("after reset: phase %u time %llu", (unsigned)info.phase, (unsigned long long)info.time_ticks);
    goto done;
  }
  im = mr_im(x.m);
  while (stores < 2u && units < MR_MAX_UNITS) {
    ivs_cp1610_result r;
    if (ivs_machine_step_unit_internal(x.m, &r) != IVS_OK) {
      ivs_test_diag("step %u refused", (unsigned)units);
      goto done;
    }
    units++;
    cycles += r.elapsed_cpu_cycles;
    if (r.unit_kind == IVS_CP1610_UNIT_INSTR && r.pc_before == 0x1002u) {
      stores++;
    }
  }
  if (stores != 2u) {
    ivs_test_diag("the store ran %u times in %u units", (unsigned)stores, (unsigned)units);
    goto done;
  }
  /* Pass 1 stored $155, pass 2 stored $156 (one INCR between them). */
  if (ivs_memsys_read(&im->mem, 0x0200u) != 0x0156u) {
    ivs_test_diag("[$0200] = $%04X, want $0156", (unsigned)ivs_memsys_read(&im->mem, 0x0200u));
    goto done;
  }
  if (im->cpu.r[1] != 0x0156u || im->cpu.r[7] < 0x1002u || im->cpu.r[7] > 0x1006u) {
    ivs_test_diag("R1 $%04X PC $%04X", (unsigned)im->cpu.r[1], (unsigned)im->cpu.r[7]);
    goto done;
  }
  if (ivs_machine_info(x.m, &info) != IVS_OK || info.time_ticks != cycles * MR_TICKS_PER_CYCLE ||
      cycles == 0u) {
    ivs_test_diag("time %llu ticks, want %llu cycles x %u", (unsigned long long)info.time_ticks,
                  (unsigned long long)cycles, MR_TICKS_PER_CYCLE);
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* The first unit after reset is the reset entry, whose IAB the machine bus answers with the
 * profile's reset.r7; the next unit fetches program A's first decle there. */
static int mr_iab_reset_vector(void)
{
  ivs_ms_inst x;
  ivs_cp1610_result r;
  const struct ivs_machine *im;
  int rc = IVS_TEST_FAIL;
  if (!ivs_ms_powered(&x, IVS_MS_CONTENT_A, 0xA5u)) {
    return IVS_TEST_FAIL;
  }
  im = mr_im(x.m);
  if (im->cpu.run_state != IVS_CP1610_RS_RESET_PENDING) {
    ivs_test_diag("run state %u after reset", (unsigned)im->cpu.run_state);
  } else if (ivs_machine_step_unit_internal(x.m, &r) != IVS_OK || r.unit_kind != IVS_CP1610_UNIT_RESET_ENTRY ||
             im->cpu.r[7] != MR_RESET_R7) {
    ivs_test_diag("first unit kind %u, R7 $%04X", (unsigned)r.unit_kind, (unsigned)im->cpu.r[7]);
  } else if (ivs_profile_get(im->profile) == NULL || ivs_profile_get(im->profile)->reset_r7 != MR_RESET_R7 ||
             ivs_profile_get(im->profile)->ticks_per_cpu_cycle != MR_TICKS_PER_CYCLE) {
    ivs_test_diag("registry reset.r7 or ticks_per_cpu_cycle differs from the profile literal");
  } else if (ivs_machine_step_unit_internal(x.m, &r) != IVS_OK || r.unit_kind != IVS_CP1610_UNIT_INSTR ||
             r.pc_before != MR_RESET_R7 || r.opcode != 0x02B9u) {
    ivs_test_diag("second unit kind %u at $%04X opcode $%04X", (unsigned)r.unit_kind, (unsigned)r.pc_before,
                  (unsigned)r.opcode);
  } else {
    rc = IVS_TEST_PASS;
  }
  ivs_ms_free(&x);
  return rc;
}

const ivs_test_case ivs_machine_reset_cases[] = {
    {"machine.reset.iab-reset-vector", mr_iab_reset_vector, NULL},
    {"machine.reset.step-program", mr_step_program, NULL},
};
const size_t ivs_machine_reset_count = sizeof ivs_machine_reset_cases / sizeof ivs_machine_reset_cases[0];
