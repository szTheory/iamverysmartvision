/* SPDX-License-Identifier: MIT */
/* Suite run, family stub (plan 04-14, D-28..D-30, RUN-06): the project boot stub, a 22-word
 * hand-assembled listing (fixtures/FX-boot-stub/boot-stub.listing.txt), packed by tools/pack into
 * the build-tree EXEC image whose bytes ivs_images.inc carries, loaded with a 2048-byte zero GROM
 * (a project stand-in, not a GROM) and driven from a power-on reset through the public API.
 *   run.stub.path   one run call per unit (target now + 1): after the reset entry unit, R7 at the
 *                   start of each following unit is 1000, 1010, 1011, 1013, 5000 (B $1010, DIS,
 *                   MVII #$0340,R6, J $5000), hand-derived from the listing; the case stops at the
 *                   first unit at 5000, since the cartridge window is empty.
 *   run.stub.trap   at 5000, one INTR pulse through the internal CPU interface (no Phase 4 device
 *                   drives INTR; a held level would re-enter forever, so the test pulses): the
 *                   interrupt entry goes to 1004 (interrupt.iab_value), the trap writes [035F] = 1,
 *                   and a public run with IVS_RUN_F_STOP_ON_HALT stops CPU_HALTED with R7 = 100A,
 *                   inside the trap block 1004-100A.
 *   run.stub.halt-needs-follower
 *                   why every ending carries a follower (D-29): HLT only sets the stop latch, the
 *                   CPU stops after the next instruction and only at an interruptible boundary,
 *                   and HLT is not interruptible. Unit by unit after the pulse the stub shows
 *                   HLT executed with the CPU still RUNNING at 1009, the TSTR R0 follower executed
 *                   (R7 = 100A, still RUNNING), then HALTED at 100A. With the follower patched to
 *                   a zero word (an in-memory copy of the image, word 1009 = 0000) the zero fill
 *                   decodes as more HLTs, the CPU runs through 100A-100F and the reset code
 *                   (DIS, MVII #$0340,R6) and halts only at 1013, outside the trap block: zero
 *                   fill is not a trap.
 * The expected values are literals worked out from the listing and the manual pages it cites,
 * never read back from the code under test. No image byte is committed (D-30). */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cpu/cp1610.h"
#include "ivs/ivs.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "run_cases.h"

#include "ivs_images.inc"

#define IVS_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

/* Like IVS_CHECK, inside a case that must release its instance (label done). */
#define RS_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      goto done;                                                                                       \
    }                                                                                                  \
  } while (0)

#define RS_POISON 0x5Au
#define RS_ZERO_GROM_BYTES 2048u

/* R7 at the start of the units after the reset entry (listing lines 1000, 1010, 1011, 1013, then
 * the cartridge entry 5000). */
static const uint16_t rs_path_r7[] = {0x1000u, 0x1010u, 0x1011u, 0x1013u, 0x5000u};

/* ---- helpers ---- */

/* A heap copy of the build-tree stub image (ivs_ms_load frees it), or NULL. */
static uint8_t *rs_stub_image(void)
{
  uint8_t *b = malloc(IVS_IMG_BOOT_STUB_BYTES);
  if (b != NULL) {
    memcpy(b, ivs_img_boot_stub, IVS_IMG_BOOT_STUB_BYTES);
  }
  return b;
}

/* A powered instance: stub EXEC from bytes (consumed), 2048 zero bytes as GROM, no cartridge, a
 * power-on reset. 1 on success. */
static int rs_powered(ivs_ms_inst *x, uint8_t *exec)
{
  ivs_status st;
  if (!ivs_ms_new(x, RS_POISON)) {
    free(exec);
    return 0;
  }
  if (!ivs_ms_load(x->m, IVS_KIND_EXEC, exec, IVS_IMG_BOOT_STUB_BYTES) ||
      !ivs_ms_load(x->m, IVS_KIND_GROM, calloc(RS_ZERO_GROM_BYTES, 1u), RS_ZERO_GROM_BYTES)) {
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

static int rs_cpu(const ivs_machine *m, ivs_cpu_state_report *r)
{
  memset(r, 0, sizeof *r);
  r->struct_size = (uint32_t)sizeof *r;
  if (ivs_machine_cpu_state(m, r) != IVS_OK) {
    ivs_test_diag("cpu_state refused");
    return 0;
  }
  return 1;
}

/* One run call to now + 1 (exactly one unit); 1 when it ran one unit and stopped at the target. */
static int rs_one_unit(ivs_machine *m)
{
  ivs_info info;
  ivs_run_request q;
  ivs_run_report rep;
  ivs_status st;
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info(m, &info) != IVS_OK) {
    ivs_test_diag("info refused");
    return 0;
  }
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.target_ticks = info.time_ticks + 1u;
  memset(&rep, 0, sizeof rep);
  rep.struct_size = (uint32_t)sizeof rep;
  st = ivs_machine_run(m, &q, &rep);
  if (st != IVS_OK || rep.stop_reason != IVS_STOP_TARGET || rep.units_executed != 1u) {
    ivs_test_diag("run to now + 1: status %s stop %u units %llu", ivs_status_name(st), (unsigned)rep.stop_reason,
                  (unsigned long long)rep.units_executed);
    return 0;
  }
  return 1;
}

/* Drives a powered stub instance from reset to the first unit at the cartridge entry 5000, checking
 * the path; 1 when every check holds. */
static int rs_to_entry(ivs_ms_inst *x)
{
  ivs_cpu_state_report r;
  size_t k;
  if (!rs_cpu(x->m, &r)) {
    return 0;
  }
  if (r.run_state != IVS_CPU_STATE_RESET_PENDING) {
    ivs_test_diag("after reset: run_state %u, want RESET_PENDING", (unsigned)r.run_state);
    return 0;
  }
  for (k = 0u; k < sizeof rs_path_r7 / sizeof rs_path_r7[0]; k++) {
    if (!rs_one_unit(x->m) || !rs_cpu(x->m, &r)) {
      return 0;
    }
    if (r.run_state != IVS_CPU_STATE_RUNNING || r.r[7] != rs_path_r7[k]) {
      ivs_test_diag("unit %u: run_state %u R7 %04X, want RUNNING and R7 %04X", (unsigned)(k + 1u),
                    (unsigned)r.run_state, (unsigned)r.r[7], (unsigned)rs_path_r7[k]);
      return 0;
    }
    /* After DIS (R7 = 1011) the interrupt system is off; after MVII #$0340,R6 (R7 = 1013) R6 is the
     * project stack start. */
    if ((rs_path_r7[k] == 0x1011u && r.intr_enabled != 0u) || (rs_path_r7[k] == 0x1013u && r.r[6] != 0x0340u)) {
      ivs_test_diag("unit %u: intr_enabled %u R6 %04X", (unsigned)(k + 1u), (unsigned)r.intr_enabled,
                    (unsigned)r.r[6]);
      return 0;
    }
  }
  return 1;
}



/* One unit with the INTR line high for that unit only (the pulse), through the internal CPU
 * interface on the instance's own memory system and the profile's CPU configuration, as the
 * machine step seam does with inactive lines; machine time and the input floor advance as the
 * seam advances them. 1 when the unit is an interrupt entry that leaves R7 at 1004. */
static int rs_pulse_intr(ivs_ms_inst *x)
{
  struct ivs_machine *im = (struct ivs_machine *)(void *)x->storage;
  const ivs_profile_desc *p = ivs_profile_get(im->profile);
  ivs_cp1610_config cfg;
  ivs_cp1610_lines lines;
  ivs_cp1610_port port;
  ivs_cp1610_result r;
  ivs_status st;
  uint64_t start;
  if (p == NULL) {
    ivs_test_diag("no profile descriptor");
    return 0;
  }
  memset(&cfg, 0, sizeof cfg);
  cfg.interrupt_accept_clears_intff = p->interrupt_accept_clears_intff;
  cfg.release_latency_cpu_cycles = p->release_latency_cpu_cycles;
  cfg.wait_limit_cpu_cycles = p->wait_limit_cpu_cycles;
  memset(&lines, 0, sizeof lines);
  lines.intr = 1u;
  memset(&port, 0, sizeof port);
  port.bus = ivs_memsys_bus;
  port.ctx = &im->mem;
  memset(&r, 0, sizeof r);
  start = im->time_ticks;
  st = ivs_cp1610_step(&im->cpu, &cfg, &lines, &port, NULL, 0u, &r);
  if (st != IVS_OK) {
    ivs_test_diag("pulse step refused (%s)", ivs_status_name(st));
    return 0;
  }
  im->obs.input.floor_tick = start + 1u;
  im->time_ticks += (uint64_t)r.elapsed_cpu_cycles * (uint64_t)p->ticks_per_cpu_cycle;
  if (r.unit_kind != IVS_CP1610_UNIT_INT_ENTRY || im->cpu.r[7] != 0x1004u) {
    ivs_test_diag("pulse: unit kind %u R7 %04X, want an interrupt entry to 1004", (unsigned)r.unit_kind,
                  (unsigned)im->cpu.r[7]);
    return 0;
  }
  return 1;
}

/* A public run with IVS_RUN_F_STOP_ON_HALT and a bound far beyond the trap (2^20 ticks); 1 when it
 * stopped CPU_HALTED. */
static int rs_run_to_halt(ivs_machine *m)
{
  ivs_info info;
  ivs_run_request q;
  ivs_run_report rep;
  ivs_status st;
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info(m, &info) != IVS_OK) {
    ivs_test_diag("info refused");
    return 0;
  }
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.flags = IVS_RUN_F_STOP_ON_HALT;
  q.target_ticks = info.time_ticks + 0x100000u;
  memset(&rep, 0, sizeof rep);
  rep.struct_size = (uint32_t)sizeof rep;
  st = ivs_machine_run(m, &q, &rep);
  if (st != IVS_OK || rep.stop_reason != IVS_STOP_CPU_HALTED) {
    ivs_test_diag("run to halt: status %s stop %u", ivs_status_name(st), (unsigned)rep.stop_reason);
    return 0;
  }
  return 1;
}

/* The word at addr through the public read view (0xFFFF when peek refuses). */
static uint16_t rs_peek(const ivs_machine *m, uint16_t addr)
{
  uint16_t w = 0xFFFFu;
  if (ivs_machine_peek(m, addr, 1u, &w) != IVS_OK) {
    ivs_test_diag("peek %04X refused", (unsigned)addr);
    return 0xFFFFu;
  }
  return w;
}

/* After the pulse, the R7 and run state at the end of each run(now + 1) call: MVII #$0001,R0,
 * MVO R0,$035F, HLT (the CPU keeps RUNNING), the TSTR R0 follower, then the stop at the
 * interruptible boundary after it. */
static const uint16_t rs_trap_r7[] = {0x1006u, 0x1008u, 0x1009u, 0x100Au, 0x100Au};
static const uint32_t rs_trap_state[] = {IVS_CPU_STATE_RUNNING, IVS_CPU_STATE_RUNNING, IVS_CPU_STATE_RUNNING,
                                         IVS_CPU_STATE_RUNNING, IVS_CPU_STATE_HALTED};

/* ---- cases ---- */


static int rs_trap(void)
{
  ivs_ms_inst x;
  ivs_cpu_state_report r;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(rs_powered(&x, rs_stub_image()));
  RS_CHECK(rs_to_entry(&x));
  RS_CHECK(rs_peek(x.m, 0x035Fu) == 0x0000u);
  RS_CHECK(rs_pulse_intr(&x));
  RS_CHECK(rs_run_to_halt(x.m));
  RS_CHECK(rs_cpu(x.m, &r));
  RS_CHECK(r.run_state == IVS_CPU_STATE_HALTED);
  RS_CHECK(r.r[7] >= 0x1004u && r.r[7] <= 0x100Au);
  RS_CHECK(r.r[7] == 0x100Au);
  RS_CHECK(r.r[0] == 0x0001u);
  RS_CHECK(rs_peek(x.m, 0x035Fu) == 0x0001u);
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

static int rs_halt_needs_follower(void)
{
  ivs_ms_inst x;
  ivs_cpu_state_report r;
  uint8_t *patched;
  size_t k;
  int rc = IVS_TEST_FAIL;
  /* The stub as listed: HLT at 1008 does not stop the CPU by itself. */
  IVS_CHECK(rs_powered(&x, rs_stub_image()));
  RS_CHECK(rs_to_entry(&x));
  RS_CHECK(rs_pulse_intr(&x));
  for (k = 0u; k < sizeof rs_trap_r7 / sizeof rs_trap_r7[0]; k++) {
    RS_CHECK(rs_one_unit(x.m));
    RS_CHECK(rs_cpu(x.m, &r));
    if (r.r[7] != rs_trap_r7[k] || r.run_state != rs_trap_state[k]) {
      ivs_test_diag("after pulse unit %u: R7 %04X state %u, want R7 %04X state %u", (unsigned)(k + 1u),
                    (unsigned)r.r[7], (unsigned)r.run_state, (unsigned)rs_trap_r7[k], (unsigned)rs_trap_state[k]);
      goto done;
    }
  }
  ivs_ms_free(&x);
  /* The follower replaced by a zero word (image offset 2 * (1009 - 1000)): zero fill is not a trap. */
  patched = rs_stub_image();
  IVS_CHECK(patched != NULL);
  if (patched[2u * 9u] != 0x00u || patched[2u * 9u + 1u] != 0x80u) {
    ivs_test_diag("image word 1009 is not TSTR R0 (0080)");
    free(patched);
    return IVS_TEST_FAIL;
  }
  patched[2u * 9u + 1u] = 0x00u;
  IVS_CHECK(rs_powered(&x, patched));
  RS_CHECK(rs_to_entry(&x));
  RS_CHECK(rs_pulse_intr(&x));
  RS_CHECK(rs_run_to_halt(x.m));
  RS_CHECK(rs_cpu(x.m, &r));
  RS_CHECK(r.run_state == IVS_CPU_STATE_HALTED);
  RS_CHECK(r.r[7] == 0x1013u);
  RS_CHECK(r.r[6] == 0x0340u);
  RS_CHECK(rs_peek(x.m, 0x035Fu) == 0x0001u);
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

static int rs_path(void)
{
  ivs_ms_inst x;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(IVS_IMG_BOOT_STUB_BYTES == 8192u);
  IVS_CHECK(rs_powered(&x, rs_stub_image()));
  RS_CHECK(rs_to_entry(&x));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

const ivs_test_case ivs_run_stub_cases[] = {
    {"run.stub.path", rs_path, NULL},
    {"run.stub.trap", rs_trap, NULL},
    {"run.stub.halt-needs-follower", rs_halt_needs_follower, NULL},
};

const size_t ivs_run_stub_count = sizeof ivs_run_stub_cases / sizeof ivs_run_stub_cases[0];
