/* SPDX-License-Identifier: MIT */
/* Public run, observation and input API (D-08, A-01, plan 04-06). Observation types come first:
 * a host, the runner and the installed consumer read guest state (CPU registers and memory)
 * through these calls, never through core internals.
 *
 * Observation calls are side-effect-free by contract (D-08): they take a const instance, advance
 * no machine time, cause no gate or stub side effect and read memory through the non-mutating
 * view of the memory map, so the instance storage and every state digest are the same before
 * and after the call. Both are valid while the machine is OFF and while it is POWERED.
 *
 * The struct rule of ivs/ivs_machine.h applies (D-04, A-02): an out struct starts with uint32_t
 * struct_size; a size below the v1 size, or above IVS_STRUCT_SIZE_MAX, is BAD_ARG with nothing
 * written; otherwise the core writes its own (v1) size and sets struct_size to it, and bytes of a
 * larger caller struct past that size are untouched. An out struct or buffer that overlaps the
 * instance storage is BAD_ARG with nothing written; that test runs after the handle check (a
 * NULL or invalid handle is BAD_HANDLE wherever the out parameter lies). Every refusal happens
 * before any write to the out parameter.
 * Standard headers only; constants are #defines (no enums, bool or bitfields). */
#ifndef IVS_RUN_H
#define IVS_RUN_H

#include <stdint.h>

#include "ivs/ivs_machine.h"
#include "ivs/ivs_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CPU run states (ivs_cpu_state_report.run_state). A new instance is UNRESET; a power-on reset
 * leaves RESET_PENDING (the CPU takes its reset entry on its next unit), then RUNNING. HALTED
 * after HLT; FAULTED after a guest fault, whose reason is in fault_reason. */
#define IVS_CPU_STATE_UNRESET 0u
#define IVS_CPU_STATE_RESET_PENDING 1u
#define IVS_CPU_STATE_RUNNING 2u
#define IVS_CPU_STATE_HALTED 3u
#define IVS_CPU_STATE_FAULTED 4u

/* Sticky guest fault reasons (ivs_cpu_state_report.fault_reason): nonzero only while FAULTED. */
#define IVS_CPU_FAULT_NONE 0u
#define IVS_CPU_FAULT_UNSUPPORTED_SDBD_TARGET 1u
#define IVS_CPU_FAULT_UNSUPPORTED_MVO_SSS_EQ_MMM 2u
#define IVS_CPU_FAULT_UNSUPPORTED_JUMP_DE 3u
#define IVS_CPU_FAULT_BUS_WAIT_LIMIT 4u

/* Out: a CPU snapshot. r[6] is the stack pointer and r[7] the program counter. s, z, ov and c are
 * the sign, zero, overflow and carry flags and intr_enabled the interrupt enable, each 0 or 1.
 * v1 size 36 bytes; reserved is written 0. */
typedef struct ivs_cpu_state_report {
  uint32_t struct_size;
  uint32_t run_state;    /* IVS_CPU_STATE_* */
  uint32_t fault_reason; /* IVS_CPU_FAULT_* */
  uint16_t r[8];
  uint8_t s;
  uint8_t z;
  uint8_t ov;
  uint8_t c;
  uint8_t intr_enabled;
  uint8_t reserved[3];
} ivs_cpu_state_report;

/* Copies the CPU state into *out; the instance is not written. Refusal order: BAD_ARG (out NULL,
 * or out->struct_size below the v1 size or above IVS_STRUCT_SIZE_MAX); BAD_HANDLE; BAD_ARG (out
 * overlapping the instance storage). Nothing is written on a refusal. */
IVS_API ivs_status ivs_machine_cpu_state(const ivs_machine *m, ivs_cpu_state_report *out);

/* Reads count 16-bit words at addr, addr + 1, ... into out[0..count-1]; the caller supplies at
 * least count words at out (there is no capacity argument). Each word is what a CPU read at that
 * address returns now: decode, width mask, closed gates and undriven space included (a closed
 * gate, a stub, an empty image slot and an address no device drives read the profile's undriven
 * value). The instance is not written and machine time does not advance (D-08); valid OFF and
 * POWERED. Refusal order: BAD_ARG (out NULL or count 0); BAD_HANDLE; OUT_OF_RANGE (addr + count
 * above 0x10000; addr + count == 0x10000 is accepted); BAD_ARG (the count * 2 bytes at out
 * overlap the instance storage). Nothing is written on a refusal. */
IVS_API ivs_status ivs_machine_peek(const ivs_machine *m, uint16_t addr, uint32_t count, uint16_t *out);

/* ---- bounded execution (D-01..D-05, D-09, plan 04-07) ----
 * ivs_machine_run runs a POWERED machine to an absolute target in MASTER TICKS (docs/time-base.md,
 * D2): while machine time is below the target it steps one CPU unit, so it ends at the first
 * unit boundary at or after the target. A unit is never split, so the run may end past the
 * target by part of one unit (the overshoot, reported, never carried). The call keeps no state
 * between calls (no budget, deficit or pointer): the whole instance is the resumable state, and
 * any slicing of one target into several calls gives the same machine. run(now + 1) runs exactly
 * one unit (the single-step idiom). CPU-cycle and frame bounds are host sugar over the tick
 * target (D-09): a host converts them; the core knows only ticks.
 * Stops are not refusals: a run that executes ends with IVS_OK and a stop reason, a guest fault
 * included (a faulted CPU is never stepped again; every later call stops GUEST_FAULT at once). */

/* Run flags (ivs_run_request.flags). Any other bit is UNSUPPORTED. */
#define IVS_RUN_F_STOP_ON_HALT 1u   /* stop at the first boundary where the CPU is HALTED */
#define IVS_RUN_F_RESERVED_FRAME 2u /* reserved for a frame stop; UNSUPPORTED until Phase 6 */

/* Largest accepted target: 2^63 master ticks; a target above it is OUT_OF_RANGE. */
#define IVS_RUN_TARGET_MAX 0x8000000000000000u

/* Stop reasons (ivs_run_report.stop_reason), append-only; 0 is not a stop reason (a refused
 * call does not write stop_reason). The reason is a pure function of the exit state, in this order: INTERNAL (a unit
 * step was refused or made no progress: a harness failure, the state is the last whole
 * boundary), GUEST_FAULT (the CPU is FAULTED), CPU_HALTED (only with IVS_RUN_F_STOP_ON_HALT: the
 * CPU is HALTED), TARGET (machine time reached the target). */
#define IVS_STOP_TARGET 1u
#define IVS_STOP_CPU_HALTED 2u
#define IVS_STOP_GUEST_FAULT 3u
#define IVS_STOP_INTERNAL 4u

/* In: one run. v1 size 16 bytes; a larger struct_size (up to IVS_STRUCT_SIZE_MAX) is accepted
 * only when every byte past the v1 size is zero. Phase 4 plans append fields (output buffers,
 * input events) before the v1 size is pinned for release. */
typedef struct ivs_run_request {
  uint32_t struct_size;
  uint32_t flags;        /* IVS_RUN_F_* */
  uint64_t target_ticks; /* absolute machine time in master ticks, at most IVS_RUN_TARGET_MAX */
} ivs_run_request;

/* Out: what one run did. v1 size 56 bytes. On IVS_OK every v1 field is written: the stop reason,
 * refusal_reason IVS_REASON_NONE, the CPU run state and fault reason at exit (IVS_CPU_STATE_*,
 * IVS_CPU_FAULT_*), machine time at exit, overshoot_ticks = max(now_ticks - target_ticks, 0)
 * (derived, never state), elapsed_ticks = now_ticks minus machine time at entry and the units
 * executed by this call. On a refusal only struct_size and refusal_reason (IVS_REASON_*) are
 * written, and nothing at all when the report itself is the cause (NULL, size, overlap). */
typedef struct ivs_run_report {
  uint32_t struct_size;
  uint32_t stop_reason;      /* IVS_STOP_* */
  uint32_t refusal_reason;   /* IVS_REASON_* */
  uint32_t cpu_run_state;    /* IVS_CPU_STATE_* */
  uint32_t cpu_fault_reason; /* IVS_CPU_FAULT_* */
  uint32_t reserved;         /* written 0 */
  uint64_t now_ticks;
  uint64_t overshoot_ticks;
  uint64_t elapsed_ticks;
  uint64_t units_executed;
} ivs_run_report;

/* Runs m to req->target_ticks (above). Every refusal happens before the first unit, so a refused
 * call changes nothing in the instance. */
IVS_API ivs_status ivs_machine_run(ivs_machine *m, const ivs_run_request *req, ivs_run_report *rep);

/* The static, constant name of a stop reason ("IVS_STOP_TARGET", ...) or "IVS_STOP_UNKNOWN" for
 * a value that is not one (0 included). Cannot refuse and touches no instance. */
IVS_API const char *ivs_run_stop_name(uint32_t stop);

#ifdef __cplusplus
}
#endif

#endif
