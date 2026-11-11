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

#ifdef __cplusplus
}
#endif

#endif
