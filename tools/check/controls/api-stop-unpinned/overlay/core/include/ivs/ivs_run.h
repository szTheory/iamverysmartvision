/* SPDX-License-Identifier: MIT */
/* Toy run header of the known-good control tree (plan 04-08): two stop reasons and two refusal
 * reasons, pinned by the toy data/expected/api-stop-reasons.ids and api-reasons.ids, so the
 * generic define pin of the api-effects step has something to compare. A commented-out
 * #define IVS_STOP_COMMENTED 9u and the function-like macro below are not pinned defines. No
 * IVS_API line, so the toy effect table needs no new call. Two input bits (plan 04-13) are
 * pinned by bit index in the toy data/expected/input-bits.ids; the reserved mask is excluded. */
#ifndef IVS_RUN_H
#define IVS_RUN_H

#define IVS_STOP_TARGET 1u
#define IVS_STOP_CPU_HALTED (2u)
#define IVS_STOP_GUEST_FAULT 3u
#define IVS_STOP_IS_TARGET(r) ((r) == IVS_STOP_TARGET)

#define IVS_REASON_NONE 0u
#define IVS_REASON_TRUNCATED 1u /* a trailing comment */

#define IVS_CTRL_KEY_1 (1u << 0)
#define IVS_CTRL_KEY_2 (1u << 1)
#define IVS_CTRL_RESERVED_MASK 0x80000000u

#endif
