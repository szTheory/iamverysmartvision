/* SPDX-License-Identifier: MIT */
/* Bounded execution (D-01..D-05, D-07, D-09, plan 04-07): ivs_machine_run and ivs_run_stop_name
 * through the public header ivs/ivs_run.h.
 *
 * Validate-then-commit (analog: machine_load.c). ivs_bound_validate reads only const inputs and
 * fills a small stack plan (the request fields it needs); the ladder is one stage per class, the
 * first failing stage wins (D-05):
 *   1. BAD_ARG       req NULL or its struct_size below the v1 size or above IVS_STRUCT_SIZE_MAX
 *                    (which bounds the stage 4 tail scan); rep NULL or its struct_size below the
 *                    v1 size or above IVS_STRUCT_SIZE_MAX (nothing written);
 *   2. BAD_HANDLE    NULL or invalid handle;
 *   3. BAD_ARG       rep overlaps the storage of the (valid) instance, uintptr_t differences only
 *                    (ivs_machine_span_overlaps; nothing written);
 *   4. UNSUPPORTED   req: an unknown flag (IVS_RUN_F_RESERVED_FRAME included), then a nonzero
 *                    byte past the v1 size of a larger struct;
 *   (slot)           the buffer and event argument checks of D-05 go here, after UNSUPPORTED and
 *                    before BAD_STATE: output buffers (plan 04-10) and input events (plan 04-11);
 *   5. BAD_STATE     the machine is OFF;
 *   6. OUT_OF_RANGE  target_ticks above IVS_RUN_TARGET_MAX (2^63), before any unit, so now plus
 *                    one unit can never wrap;
 *   (later)          BUFFER_TOO_SMALL (plan 04-10) and the per-event order and channel checks
 *                    (plan 04-11) follow OUT_OF_RANGE, still before the first unit.
 * A refusal writes struct_size and the IVS_REASON_* only into a report that is not the cause and
 * does not overlap the bytes the handle names (ivs_bound_report_ok), whatever stage refused.
 *
 * Commit: while machine time is below the target, step one unit through the internal seam
 * ivs_machine_step_unit_internal (machine_run.c; the seam stays internal). A FAULTED CPU is never
 * stepped; with IVS_RUN_F_STOP_ON_HALT a HALTED CPU is not stepped; a unit the seam refuses, or a
 * unit that does not advance machine time, ends the loop (INTERNAL unless the CPU is FAULTED), so
 * the loop always terminates. The stop reason is then a pure function of the exit state: INTERNAL,
 * GUEST_FAULT, CPU_HALTED (flag), TARGET. The report is filled last; overshoot is derived. Nothing
 * outlives the call (no budget, deficit or pointer): slicing invariance holds by construction.
 * The file holds no static or global state.
 * Named faults (plan 04-07, D-07): RUN_TARGET_UNCHECKED, RUN_STOP_BEFORE_OVERSHOOT,
 * RUN_FAULT_STEPS. */
#include "common/ivs_prelude.h"

#include "ivs/ivs.h"
#include "machine/machine.h"

_Static_assert(sizeof(ivs_run_request) == 16u, "ivs_run_request v1 is 16 bytes");
_Static_assert(sizeof(ivs_run_report) == 56u, "ivs_run_report v1 is 56 bytes");
_Static_assert(offsetof(ivs_run_report, now_ticks) % 8u == 0u, "64-bit report members are 8-aligned");

/* What the commit loop needs from a validated request. */
typedef struct ivs_bound_plan {
  uint64_t target;
  uint32_t flags;
} ivs_bound_plan;

/* 1 when every byte of p[from, to) is zero. */
static uint32_t ivs_bound_tail_zero(const uint8_t *p, size_t from, size_t to)
{
  size_t i;
  for (i = from; i < to; i++) {
    if (p[i] != 0u) {
      return 0u;
    }
  }
  return 1u;
}

/* 1 when the caller's report may receive refusal detail: present, a size the core accepts, and
 * not overlapping the instance bytes the handle names. That last test uses only the handle's
 * address (uintptr_t differences, nothing read through it), so a report inside the storage of an
 * invalid or destroyed instance gets nothing either. */
static uint32_t ivs_bound_report_ok(const ivs_machine *m, const ivs_run_report *rep)
{
  if (rep == NULL || rep->struct_size < (uint32_t)sizeof(ivs_run_report) ||
      rep->struct_size > IVS_STRUCT_SIZE_MAX) {
    return 0u;
  }
  if (m != NULL && ivs_machine_span_overlaps(rep, sizeof *rep, m, sizeof(struct ivs_machine)) != 0u) {
    return 0u;
  }
  return 1u;
}

/* The ladder (header comment). Reads only const inputs; on IVS_OK the plan is filled. */
static ivs_status ivs_bound_validate(const ivs_machine *m, const ivs_run_request *req, const ivs_run_report *rep,
                                     ivs_bound_plan *plan, uint32_t *reason)
{
  const struct ivs_machine *im;
  /* 1. BAD_ARG: NULL arguments and struct sizes. */
  if (rep == NULL || rep->struct_size < (uint32_t)sizeof(ivs_run_report) ||
      rep->struct_size > IVS_STRUCT_SIZE_MAX) {
    *reason = IVS_REASON_RUN_STRUCT_SIZE;
    return IVS_E_BAD_ARG;
  }
  if (req == NULL) {
    *reason = IVS_REASON_RUN_NULL_ARG;
    return IVS_E_BAD_ARG;
  }
  if (req->struct_size < (uint32_t)sizeof(ivs_run_request) || req->struct_size > IVS_STRUCT_SIZE_MAX) {
    *reason = IVS_REASON_RUN_STRUCT_SIZE;
    return IVS_E_BAD_ARG;
  }
  /* 2. BAD_HANDLE. */
  im = ivs_machine_from_handle(m);
  if (im == NULL) {
    *reason = IVS_REASON_HANDLE;
    return IVS_E_BAD_HANDLE;
  }
  /* 3. BAD_ARG: the report overlaps the instance storage. */
  if (ivs_machine_span_overlaps(rep, sizeof *rep, im, sizeof *im) != 0u) {
    *reason = IVS_REASON_RUN_REPORT_OVERLAP;
    return IVS_E_BAD_ARG;
  }
  /* 4. UNSUPPORTED: unknown flags, then a nonzero tail of a larger request. */
  if ((req->flags & ~IVS_RUN_F_STOP_ON_HALT) != 0u) {
    *reason = IVS_REASON_RUN_FLAGS;
    return IVS_E_UNSUPPORTED;
  }
  if (ivs_bound_tail_zero((const uint8_t *)req, sizeof(ivs_run_request), (size_t)req->struct_size) == 0u) {
    *reason = IVS_REASON_RUN_TAIL;
    return IVS_E_UNSUPPORTED;
  }
  /* (slot: output buffers, plan 04-10; input events, plan 04-11.) */
  /* 5. BAD_STATE: the machine is OFF. */
  if (im->phase != IVS_PHASE_POWERED) {
    *reason = IVS_REASON_RUN_STATE_OFF;
    return IVS_E_BAD_STATE;
  }
  /* 6. OUT_OF_RANGE: the target. */
#ifndef IVS_FAULT_RUN_TARGET_UNCHECKED /* named fault RUN_TARGET_UNCHECKED: the stage is skipped */
  if (req->target_ticks > IVS_RUN_TARGET_MAX) {
    *reason = IVS_REASON_RUN_TARGET_RANGE;
    return IVS_E_OUT_OF_RANGE;
  }
#endif
  plan->target = req->target_ticks;
  plan->flags = req->flags;
  return IVS_OK;
}

/* The stop reason of an exit state (pure: fault, then halt with the flag, then target). */
static uint32_t ivs_bound_stop(const struct ivs_machine *im, uint32_t flags, uint64_t target)
{
#ifdef IVS_FAULT_RUN_FAULT_STEPS /* and the stop order tests the target first */
  if (im->time_ticks >= target) {
    return IVS_STOP_TARGET; /* the target is checked before the guest state */
  }
#endif
  if (im->cpu.run_state == IVS_CP1610_RS_FAULTED) {
    return IVS_STOP_GUEST_FAULT;
  }
  if ((flags & IVS_RUN_F_STOP_ON_HALT) != 0u && im->cpu.run_state == IVS_CP1610_RS_HALTED) {
    return IVS_STOP_CPU_HALTED;
  }
  (void)target;
  return IVS_STOP_TARGET;
}

ivs_status ivs_machine_run(ivs_machine *m, const ivs_run_request *req, ivs_run_report *rep)
{
  ivs_bound_plan plan;
  ivs_run_report r;
  struct ivs_machine *im;
  ivs_status st;
  uint32_t reason = IVS_REASON_NONE;
  uint32_t internal = 0u;
  uint64_t entry;
  uint64_t units = 0u;
  memset(&plan, 0, sizeof plan);
  st = ivs_bound_validate(m, req, rep, &plan, &reason);
  if (st != IVS_OK) {
    /* Refusal detail only into a report that is not itself the cause. */
    if (ivs_bound_report_ok(m, rep) != 0u) {
      rep->struct_size = (uint32_t)sizeof(ivs_run_report);
      rep->refusal_reason = reason;
    }
    return st;
  }
  im = ivs_machine_from_handle_mut(m);
  entry = im->time_ticks;
  while (im->time_ticks < plan.target) {
    ivs_cp1610_result unit;
    uint64_t before = im->time_ticks;
#ifdef IVS_FAULT_RUN_STOP_BEFORE_OVERSHOOT
    /* Named fault RUN_STOP_BEFORE_OVERSHOOT: a unit that would end past the target is taken back
     * (CPU and time; its memory writes stay), so the run stops at the last boundary below it. */
    ivs_cp1610 cpu_before = im->cpu;
#endif
#ifndef IVS_FAULT_RUN_FAULT_STEPS /* named fault RUN_FAULT_STEPS: a faulted CPU is stepped */
    if (im->cpu.run_state == IVS_CP1610_RS_FAULTED) {
      break; /* a faulted CPU is never stepped */
    }
#endif
    if ((plan.flags & IVS_RUN_F_STOP_ON_HALT) != 0u && im->cpu.run_state == IVS_CP1610_RS_HALTED) {
      break;
    }
    memset(&unit, 0, sizeof unit);
    if (ivs_machine_step_unit_internal(m, &unit) != IVS_OK) {
      internal = 1u; /* the seam refused: nothing changed, the state is the last whole boundary */
      break;
    }
#ifdef IVS_FAULT_RUN_STOP_BEFORE_OVERSHOOT
    if (im->time_ticks > plan.target) {
      im->cpu = cpu_before;
      im->time_ticks = before;
      break;
    }
#endif
    units++;
    if (im->time_ticks == before) {
      /* No progress: never loop on it. A FAULTED CPU (a zero-cycle fault unit) is a guest stop. */
      internal = im->cpu.run_state == IVS_CP1610_RS_FAULTED ? 0u : 1u;
      break;
    }
  }
  memset(&r, 0, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  r.stop_reason = internal != 0u ? IVS_STOP_INTERNAL : ivs_bound_stop(im, plan.flags, plan.target);
  r.refusal_reason = IVS_REASON_NONE;
  r.cpu_run_state = im->cpu.run_state;
  r.cpu_fault_reason = im->cpu.fault_reason;
  r.now_ticks = im->time_ticks;
  r.overshoot_ticks = im->time_ticks > plan.target ? im->time_ticks - plan.target : 0u;
  r.elapsed_ticks = im->time_ticks - entry;
  r.units_executed = units;
  *rep = r;
  return IVS_OK;
}

const char *ivs_run_stop_name(uint32_t stop)
{
  switch (stop) {
  case IVS_STOP_TARGET:
    return "IVS_STOP_TARGET";
  case IVS_STOP_CPU_HALTED:
    return "IVS_STOP_CPU_HALTED";
  case IVS_STOP_GUEST_FAULT:
    return "IVS_STOP_GUEST_FAULT";
  case IVS_STOP_INTERNAL:
    return "IVS_STOP_INTERNAL";
  default:
    return "IVS_STOP_UNKNOWN";
  }
}
