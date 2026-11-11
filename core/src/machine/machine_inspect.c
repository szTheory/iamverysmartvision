/* SPDX-License-Identifier: MIT */
/* Read-only guest observation (D-08, A-01, plan 04-06): ivs_machine_cpu_state and
 * ivs_machine_peek through the public header ivs/ivs_run.h. Both take a const instance and read
 * only const state: they advance no machine time and write nothing in the instance, so the
 * instance storage and every state digest are equal before and after them; both are valid OFF
 * and POWERED. peek reads through ivs_memsys_read over the const memory system, the read view a
 * CPU READ uses (decode, width mask, gates, undriven fill), with no access log and no device
 * side effect.
 *
 * Refusal ladders, every class before any write to the out parameter (analog: ivs_machine_info):
 *   cpu_state  BAD_ARG (out NULL, struct_size below the v1 size or above IVS_STRUCT_SIZE_MAX),
 *              BAD_HANDLE, BAD_ARG (out overlapping the instance storage);
 *   peek       BAD_ARG (out NULL or count 0), BAD_HANDLE, OUT_OF_RANGE (addr + count, in 64
 *              bits, above 0x10000; tested before any span arithmetic, so the out span below is
 *              at most 131072 bytes), BAD_ARG (the count * 2 bytes at out overlap the instance).
 * The overlap tests use ivs_machine_span_overlaps (uintptr_t differences only) against a
 * validated instance. The CPU report is built in a local and copied out last.
 * The file holds no static or global state.
 * Named faults (plan 04-06, D-08): PEEK_ADVANCES_TIME, PEEK_GATE_IGNORED. */
#include "common/ivs_prelude.h"

#include "ivs/ivs.h"
#include "machine/machine.h"

_Static_assert(sizeof(ivs_cpu_state_report) == 36u, "ivs_cpu_state_report v1 is 36 bytes");
_Static_assert(IVS_CPU_STATE_UNRESET == IVS_CP1610_RS_UNRESET, "run state UNRESET");
_Static_assert(IVS_CPU_STATE_RESET_PENDING == IVS_CP1610_RS_RESET_PENDING, "run state RESET_PENDING");
_Static_assert(IVS_CPU_STATE_RUNNING == IVS_CP1610_RS_RUNNING, "run state RUNNING");
_Static_assert(IVS_CPU_STATE_HALTED == IVS_CP1610_RS_HALTED, "run state HALTED");
_Static_assert(IVS_CPU_STATE_FAULTED == IVS_CP1610_RS_FAULTED, "run state FAULTED");
_Static_assert(IVS_CPU_FAULT_NONE == IVS_CP1610_FAULT_NONE, "fault reason NONE");
_Static_assert(IVS_CPU_FAULT_UNSUPPORTED_SDBD_TARGET == IVS_CP1610_FAULT_UNSUPPORTED_SDBD_TARGET, "fault reason 1");
_Static_assert(IVS_CPU_FAULT_UNSUPPORTED_MVO_SSS_EQ_MMM == IVS_CP1610_FAULT_UNSUPPORTED_MVO_SSS_EQ_MMM,
               "fault reason 2");
_Static_assert(IVS_CPU_FAULT_UNSUPPORTED_JUMP_DE == IVS_CP1610_FAULT_UNSUPPORTED_JUMP_DE, "fault reason 3");
_Static_assert(IVS_CPU_FAULT_BUS_WAIT_LIMIT == IVS_CP1610_FAULT_BUS_WAIT_LIMIT, "fault reason 4");
_Static_assert(IVS_CPU_FAULT_BUS_WAIT_LIMIT == IVS_CP1610_FAULT_LAST, "every CPU fault reason is public");

/* 1 when the status bit is set in the CPU's status byte, else 0. */
static uint8_t ivs_inspect_flag(uint8_t status, uint32_t bit)
{
  return (uint8_t)(((uint32_t)status & bit) != 0u ? 1u : 0u);
}

ivs_status ivs_machine_cpu_state(const ivs_machine *m, ivs_cpu_state_report *out)
{
  const struct ivs_machine *im;
  ivs_cpu_state_report r;
  uint32_t i;
  if (out == NULL || out->struct_size < (uint32_t)sizeof(ivs_cpu_state_report) ||
      out->struct_size > IVS_STRUCT_SIZE_MAX) {
    return IVS_E_BAD_ARG;
  }
  im = ivs_machine_from_handle(m);
  if (im == NULL) {
    return IVS_E_BAD_HANDLE;
  }
  if (ivs_machine_span_overlaps(out, sizeof *out, im, sizeof *im) != 0u) {
    return IVS_E_BAD_ARG;
  }
  memset(&r, 0, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  r.run_state = im->cpu.run_state;
  r.fault_reason = im->cpu.fault_reason;
  for (i = 0u; i < 8u; i++) {
    r.r[i] = im->cpu.r[i];
  }
  r.s = ivs_inspect_flag(im->cpu.status, IVS_CP1610_S);
  r.z = ivs_inspect_flag(im->cpu.status, IVS_CP1610_Z);
  r.ov = ivs_inspect_flag(im->cpu.status, IVS_CP1610_OV);
  r.c = ivs_inspect_flag(im->cpu.status, IVS_CP1610_C);
  r.intr_enabled = (uint8_t)(im->cpu.intff != 0u ? 1u : 0u);
  *out = r;
  return IVS_OK;
}

ivs_status ivs_machine_peek(const ivs_machine *m, uint16_t addr, uint32_t count, uint16_t *out)
{
  const struct ivs_machine *im;
  uint32_t i;
  if (out == NULL || count == 0u) {
    return IVS_E_BAD_ARG;
  }
  im = ivs_machine_from_handle(m);
  if (im == NULL) {
    return IVS_E_BAD_HANDLE;
  }
  if ((uint64_t)addr + (uint64_t)count > 0x10000u) {
    return IVS_E_OUT_OF_RANGE;
  }
  if (ivs_machine_span_overlaps(out, (size_t)count * sizeof *out, im, sizeof *im) != 0u) {
    return IVS_E_BAD_ARG;
  }
  for (i = 0u; i < count; i++) {
    uint16_t a = IVS_U16((uint32_t)addr + i);
    uint16_t v = ivs_memsys_read(&im->mem, a);
#ifdef IVS_FAULT_PEEK_GATE_IGNORED
    {
      /* Named fault PEEK_GATE_IGNORED: the backing word is read as if every gate were open (the
       * gate flags are opened around the read and put back). */
      ivs_memsys *w = (ivs_memsys *)(uintptr_t)&im->mem;
      uint8_t stic = w->gate_open[IVS_MEMSYS_GATE_STIC_REGS];
      uint8_t gmem = w->gate_open[IVS_MEMSYS_GATE_GMEM];
      w->gate_open[IVS_MEMSYS_GATE_STIC_REGS] = 1u;
      w->gate_open[IVS_MEMSYS_GATE_GMEM] = 1u;
      v = ivs_memsys_read(w, a);
      w->gate_open[IVS_MEMSYS_GATE_STIC_REGS] = stic;
      w->gate_open[IVS_MEMSYS_GATE_GMEM] = gmem;
    }
#endif
    out[i] = v;
  }
#ifdef IVS_FAULT_PEEK_ADVANCES_TIME
  /* Named fault PEEK_ADVANCES_TIME: an accepted peek adds one master tick to machine time. */
  ((struct ivs_machine *)(uintptr_t)im)->time_ticks += 1u;
#endif
  return IVS_OK;
}
