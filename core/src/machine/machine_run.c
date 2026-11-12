/* SPDX-License-Identifier: MIT */
/* Power-on reset and the internal step seam (D-05, D-14, D-20, A-08 d, e, plan 03-12):
 * ivs_machine_reset through the public header ivs/ivs_machine.h, and
 * ivs_machine_step_unit_internal, declared only in machine/machine.h (never under core/include).
 *
 * Reset validates before its first write, in this order:
 *   BAD_ARG              kind is not IVS_RESET_POWER_ON (an unknown enumerant, A-08 e);
 *   BAD_HANDLE           NULL or invalid handle (a stored profile index outside the registry
 *                        included, review WR-03);
 *   MISSING_SYSTEM_IMAGE a slot the profile marks required is empty, EXEC checked before GROM
 *                        (D-14: reset is the first operation that needs the system images).
 * Only then does it apply the power-on state: system RAM, scratchpad and GRAM take the profile's
 * power-on fill (zero; the registry generator refuses any other fill), every gate opens, the CPU
 * is reset (POL-RST-01: its next step is the reset entry, whose IAB the memory system answers
 * with the profile's reset.r7), machine time is 0, the observation rows (digest domains 16..19,
 * plans 04-05 and 05-03), the PSG registers, pull-downs and device state are zero and the phase
 * is POWERED. The image slots, the committed cartridge segments and the profile index are untouched. Reset is allowed from OFF and
 * from POWERED; from POWERED it is a power cycle that gives the same state as a first reset with
 * the same content. Nothing returns an instance to OFF (A-08 d).
 *
 * The step seam runs exactly one CPU unit with the profile's CPU configuration, inactive lines
 * (Phase 3 has no device that drives INTR, INTRM, BUSRQ or STPST) and a port whose bus is
 * ivs_machine_bus, the machine-level wrapper (plan 05-04, A-01), which forwards every access to
 * ivs_memsys_bus on the instance's own memory system. Machine time advances by the unit's
 * elapsed CPU cycles times the profile's ticks_per_cpu_cycle (D2: time is kept in master ticks); then the PSG is caught up to the new
 * machine time (plan 05-06, A-02), after the catch-ups of its own register writes (machine_bus.c).
 * A unit the CPU accepts sets the input floor to its start tick + 1 (plan 04-11, D-14), so the run
 * call and a seam-stepped twin leave the same INPUT row.
 * The file holds no static or global state: every value lives in the instance (D-07).
 * Named faults (plan 03-12): RESET_CLEARS_BEFORE_IMAGE_CHECK, RESET_KEEPS_RAM, STATIC_CARRY;
 * (plan 04-05): OBS_NOT_CLEARED_ON_RESET. */
#include "common/ivs_prelude.h"

#include "ivs/ivs.h"
#include "machine/machine.h"

#ifdef IVS_FAULT_STATIC_CARRY
/* Named fault STATIC_CARRY: a file-scope static carries the previous unit's cycle count, of
 * whichever instance stepped last, into the next call's time. */
static uint32_t ivs_run_carry_cycles;
#endif

/* The power-on state of every volatile part of the instance (no check, no early return). */
static void ivs_run_power_on(struct ivs_machine *im)
{
  size_t ram = sizeof im->mem.sysram;
#ifdef IVS_FAULT_RESET_KEEPS_RAM
  ram = 0u; /* system RAM keeps what the program wrote before the power cycle */
#endif
  memset(im->mem.sysram, 0, ram);
  memset(im->mem.scratchpad, 0, sizeof im->mem.scratchpad);
  memset(im->mem.gram, 0, sizeof im->mem.gram);
  /* Plan 05-03 (D-04, D-16): power-on register state of the PSG is zero, both ports idle, and the
   * device state starts at its power-on values. */
  memset(im->mem.psg_reg, 0, sizeof im->mem.psg_reg);
#ifndef IVS_FAULT_INPUT_PINS_NOT_RESET /* named fault INPUT_PINS_NOT_RESET: power-on leaves the pull-down masks */
  memset(im->mem.port_pulldown, 0, sizeof im->mem.port_pulldown);
#endif
#ifndef IVS_FAULT_PSG_STATE_NOT_RESET_ON_POWER_ON
  ivs_psg_state_reset(ivs_psg_params_get(im->profile), &im->psg); /* named fault PSG_STATE_NOT_RESET_ON_POWER_ON skips the call */
#endif
  /* Plan 06-05 (D-04, D-06, D-16): the STIC register file is zero (the profile's power-on registers
   * record) and the device state starts at its power-on record: counters, frame_index, event_index and
   * the color-stack pointer 0, the row latches 0, mode and display latch from the profile. */
  memset(im->mem.stic_reg, 0, sizeof im->mem.stic_reg);
  ivs_stic_state_reset(&im->stic, ivs_stic_params_get(im->profile));
  ivs_memsys_set_gate_internal(&im->mem, IVS_MEMSYS_GATE_STIC_REGS, 1u);
  ivs_memsys_set_gate_internal(&im->mem, IVS_MEMSYS_GATE_GMEM, 1u);
  (void)ivs_cp1610_reset(&im->cpu);
  im->time_ticks = 0u;
  /* The observation rows (digest domains 16..18, plan 04-05, D-10) start as create leaves them:
   * every byte zero except the input log, which starts at h0 (plan 04-11, D-15); plan 05-03 adds
   * the audio level rows and the PSGLOG rows (domains 17 and 19), also zero. */
  size_t obs = sizeof im->obs;
#ifdef IVS_FAULT_OBS_NOT_CLEARED_ON_RESET
  obs = 0u; /* the observation rows keep what they held before the reset */
#endif
  memset(&im->obs, 0, obs);
  if (obs != 0u) {
    ivs_input_init(&im->obs.input);
  }
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
  /* BAD_HANDLE also covers a stored profile index outside the registry (WR-03), so p is set. */
  im = ivs_machine_from_handle_mut(m);
  if (im == NULL) {
    return IVS_E_BAD_HANDLE;
  }
  p = ivs_profile_get(im->profile);
#ifdef IVS_FAULT_RESET_CLEARS_BEFORE_IMAGE_CHECK
  ivs_run_power_on(im); /* the volatile state is reset before the images are checked */
#endif
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
  ivs_psg_sink counting;
  ivs_psg_sink_init(&counting, NULL, 0u); /* the counting sink: the same code, nothing delivered */
  return ivs_machine_step_unit_sink_internal(m, out, &counting);
}

ivs_status ivs_machine_step_unit_sink_internal(ivs_machine *m, ivs_cp1610_result *out, ivs_psg_sink *sink)
{
  return ivs_machine_step_unit_trace_internal(m, out, sink, NULL, 0u);
}

ivs_status ivs_machine_step_unit_trace_internal(ivs_machine *m, ivs_cp1610_result *out, ivs_psg_sink *sink,
                                                ivs_cp1610_access *trace, size_t trace_cap)
{
  struct ivs_machine *im;
  const ivs_profile_desc *p;
  ivs_cp1610_config cfg;
  ivs_cp1610_lines lines;
  ivs_cp1610_port port;
  ivs_machine_bus_ctx bus_ctx;
  ivs_cp1610_result r;
  ivs_status st;
  uint32_t cycles;
  uint64_t start;
  if (out == NULL || sink == NULL) {
    return IVS_E_BAD_ARG;
  }
  im = ivs_machine_from_handle_mut(m);
  if (im == NULL) {
    return IVS_E_BAD_HANDLE;
  }
  if (im->phase != IVS_PHASE_POWERED) {
    return IVS_E_BAD_STATE;
  }
  p = ivs_profile_get(im->profile); /* registered: the handle check covers the index (WR-03) */
  memset(&cfg, 0, sizeof cfg);
  cfg.interrupt_accept_clears_intff = p->interrupt_accept_clears_intff;
  cfg.release_latency_cpu_cycles = p->release_latency_cpu_cycles;
  cfg.wait_limit_cpu_cycles = p->wait_limit_cpu_cycles;
  memset(&lines, 0, sizeof lines);
  start = im->time_ticks;
  /* The per-unit bus context (plan 05-04, A-01): the instance, the unit's start tick and the
   * profile's master ticks per CPU cycle. Built here for every unit, the run call included (it steps
   * through this seam). The bus wrapper (machine_bus.c) stamps every PSG write from it, catches the PSG
   * up to that stamp and appends the write-log record. */
  memset(&bus_ctx, 0, sizeof bus_ctx);
  bus_ctx.m = im;
  bus_ctx.unit_start_tick = start;
  bus_ctx.ticks_per_cpu_cycle = p->ticks_per_cpu_cycle;
  bus_ctx.psg = ivs_psg_params_get(im->profile); /* never NULL: the handle check covers the index in every generated table (WR-04) */
  bus_ctx.sink = sink;
  memset(&port, 0, sizeof port);
  port.bus = ivs_machine_bus;
  port.ebci = NULL;
  port.ctx = &bus_ctx;
  memset(&r, 0, sizeof r);
  st = ivs_cp1610_step(&im->cpu, &cfg, &lines, &port, trace, trace_cap, &r);
  if (st != IVS_OK) {
    return st;
  }
  /* A unit began at start: the input floor is its start + 1 (plan 04-11, D-14, A-04), so no later
   * event may precede one the run already applied before this unit. */
  im->obs.input.floor_tick = start + 1u;
  cycles = r.elapsed_cpu_cycles;
#ifdef IVS_FAULT_STATIC_CARRY
  {
    uint32_t previous = ivs_run_carry_cycles;
    ivs_run_carry_cycles = cycles;
    cycles = previous;
  }
#endif
  im->time_ticks += (uint64_t)cycles * (uint64_t)p->ticks_per_cpu_cycle;
  /* Plan 05-06 (A-02): the PSG is caught up to the unit boundary at EVERY unit end, in the seam and
   * in the run call alike (which steps through here), never to a call target. */
  ivs_psg_catch_up(bus_ctx.psg, &im->psg, im->mem.psg_reg, &im->obs.audio, im->time_ticks, sink);
  *out = r;
  return IVS_OK;
}
