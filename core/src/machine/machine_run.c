/* SPDX-License-Identifier: MIT */
/* Power-on reset and the internal step seam (D-05, D-14, D-20, A-08 d, e, plan 03-12):
 * ivs_machine_reset through the public header ivs/ivs_machine.h, and
 * ivs_machine_step_unit_internal, declared only in machine/machine.h (never under core/include).
 *
 * Reset validates before its first write, in this order:
 *   BAD_ARG              kind is not IVS_RESET_POWER_ON (an unknown enumerant, A-08 e);
 *   BAD_HANDLE           NULL or invalid handle;
 *   MISSING_SYSTEM_IMAGE a slot the profile marks required is empty, EXEC checked before GROM
 *                        (D-14: reset is the first operation that needs the system images).
 * Only then does it apply the power-on state: system RAM, scratchpad and GRAM take the profile's
 * power-on fill (zero; the registry generator refuses any other fill), every gate opens, the CPU
 * is reset (POL-RST-01: its next step is the reset entry, whose IAB the memory system answers
 * with the profile's reset.r7), machine time is 0 and the phase is POWERED. The image slots, the
 * committed cartridge segments and the profile index are untouched. Reset is allowed from OFF and
 * from POWERED; from POWERED it is a power cycle that gives the same state as a first reset with
 * the same content. Nothing returns an instance to OFF (A-08 d).
 *
 * The step seam runs exactly one CPU unit with the profile's CPU configuration, inactive lines
 * (Phase 3 has no device that drives INTR, INTRM, BUSRQ or STPST) and a port whose bus is
 * ivs_memsys_bus on the instance's own memory system. Machine time advances by the unit's
 * elapsed CPU cycles times the profile's ticks_per_cpu_cycle (D2: time is kept in master ticks).
 * The file holds no static or global state: every value lives in the instance (D-07). */
#include "common/ivs_prelude.h"

#include "ivs/ivs.h"
#include "machine/machine.h"

/* The power-on state of every volatile part of the instance (no check, no early return). */
static void ivs_run_power_on(struct ivs_machine *im)
{
  size_t ram = sizeof im->mem.sysram;
  memset(im->mem.sysram, 0, ram);
  memset(im->mem.scratchpad, 0, sizeof im->mem.scratchpad);
  memset(im->mem.gram, 0, sizeof im->mem.gram);
  ivs_memsys_set_gate_internal(&im->mem, IVS_MEMSYS_GATE_STIC_REGS, 1u);
  ivs_memsys_set_gate_internal(&im->mem, IVS_MEMSYS_GATE_GMEM, 1u);
  (void)ivs_cp1610_reset(&im->cpu);
  im->time_ticks = 0u;
}

ivs_status ivs_machine_reset(ivs_machine *m, uint32_t kind)
{
  struct ivs_machine *im;
  const ivs_profile_desc *p;
  /* BAD_ARG: the reset kind (A-08 e). */
  if (kind != IVS_RESET_POWER_ON) {
    return IVS_E_BAD_ARG;
  }
  /* BAD_HANDLE. */
  im = ivs_machine_from_handle_mut(m);
  if (im == NULL) {
    return IVS_E_BAD_HANDLE;
  }
  p = ivs_profile_get(im->profile);
  if (p == NULL) {
    return IVS_E_BAD_HANDLE;
  }
  /* MISSING_SYSTEM_IMAGE: EXEC, then GROM. */
  if (p->exec.required != 0u && im->slot[IVS_KIND_EXEC - 1u].loaded == 0u) {
    return IVS_E_MISSING_SYSTEM_IMAGE;
  }
  if (p->grom.required != 0u && im->slot[IVS_KIND_GROM - 1u].loaded == 0u) {
    return IVS_E_MISSING_SYSTEM_IMAGE;
  }
  /* Commit. */
  ivs_run_power_on(im);
  im->phase = IVS_PHASE_POWERED;
  return IVS_OK;
}

ivs_status ivs_machine_step_unit_internal(ivs_machine *m, ivs_cp1610_result *out)
{
  struct ivs_machine *im;
  const ivs_profile_desc *p;
  ivs_cp1610_config cfg;
  ivs_cp1610_lines lines;
  ivs_cp1610_port port;
  ivs_cp1610_result r;
  ivs_status st;
  uint32_t cycles;
  if (out == NULL) {
    return IVS_E_BAD_ARG;
  }
  im = ivs_machine_from_handle_mut(m);
  if (im == NULL) {
    return IVS_E_BAD_HANDLE;
  }
  if (im->phase != IVS_PHASE_POWERED) {
    return IVS_E_BAD_STATE;
  }
  p = ivs_profile_get(im->profile);
  if (p == NULL) {
    return IVS_E_BAD_HANDLE;
  }
  memset(&cfg, 0, sizeof cfg);
  cfg.interrupt_accept_clears_intff = p->interrupt_accept_clears_intff;
  cfg.release_latency_cpu_cycles = p->release_latency_cpu_cycles;
  cfg.wait_limit_cpu_cycles = p->wait_limit_cpu_cycles;
  memset(&lines, 0, sizeof lines);
  memset(&port, 0, sizeof port);
  port.bus = ivs_memsys_bus;
  port.ebci = NULL;
  port.ctx = &im->mem;
  memset(&r, 0, sizeof r);
  st = ivs_cp1610_step(&im->cpu, &cfg, &lines, &port, NULL, 0u, &r);
  if (st != IVS_OK) {
    return st;
  }
  cycles = r.elapsed_cpu_cycles;
  im->time_ticks += (uint64_t)cycles * (uint64_t)p->ticks_per_cpu_cycle;
  *out = r;
  return IVS_OK;
}
