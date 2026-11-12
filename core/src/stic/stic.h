/* SPDX-License-Identifier: MIT */
/* STIC device state (plan 06-05, D-04, D-06, D-16, A-05, A-11): the O(1) state of the STIC, a flat,
 * pointer-free struct inside the machine instance (digest domain 4, INSTANCE). Internal; it lives in
 * core/src, which only the core and the tests (ivs_core_internal) see. The guest-visible register file
 * (64 words) is not here: it lives in ivs_memsys (stic_reg), beside the PSG registers, and is a MEM row.
 *
 * Plan 06-05 adds storage only: nothing reads or writes this state yet (the device plan 06-06 and the
 * timeline plan 06-08 do); the reset and validate functions below are the whole behaviour.
 *
 * Layout derivation (bytes, no implicit padding; machine.h states the instance arithmetic):
 *   frame_index      u64  at   0   index k of the frame in progress (never derived from time by modulo, D-07)
 *   event_index      u32  at   8   cursor into the per-frame event table, 0 .. IVS_STIC_EVENT_MAX - 1;
 *                                  0 means FRAME_START of frame_index is still to be processed
 *   display_latch    u32  at  12   0 or 1, the display-enable handshake of the current frame (D-06)
 *   windows_active   u32  at  16   0 or 1, decided at the first window of the frame from the latch
 *   mode             u32  at  20   0 Foreground/Background, 1 Color Stack (persistent, D-06)
 *   cs_ptr           u32  at  24   color-stack pointer 0..3, reset to 0 each frame
 *   blocked_reads    u32  at  28   reads refused by a closed gate (D-06), saturating
 *   blocked_writes   u32  at  32   writes dropped by a closed gate (D-06), saturating
 *   late_grants      u32  at  36   bus windows that ended without a grant (T-6)
 *   grants_in_frame  u32  at  40   grants of the frame in progress
 *   reserved0        u32  at  44   0, a PAD row of the digest table (never serialized)
 *   row_latch   u16[12][20] at 48  the BACKTAB row latches, 12 rows x 20 words = 480 bytes (A-05: window 12
 *                                  fetches nothing, so 12 rows and not 13), ending at 528
 * Total 528 = 8 + 10 x 4 + 480, a multiple of 8 (the u64 member makes the struct 8-aligned). */
#ifndef IVS_STIC_H
#define IVS_STIC_H

#include <stdint.h>

#include "ivs/ivs_status.h"
#include "memmap/memsys.h"
#include "stic/stic_params.h"

#define IVS_STIC_LATCH_ROWS 12u  /* card rows with a fetch window (window 12 fetches nothing, A-05) */
#define IVS_STIC_ROW_WORDS 20u   /* BACKTAB words per card row */

typedef struct ivs_stic_state {
  uint64_t frame_index;
  uint32_t event_index;
  uint32_t display_latch;
  uint32_t windows_active;
  uint32_t mode;
  uint32_t cs_ptr;
  uint32_t blocked_reads;
  uint32_t blocked_writes;
  uint32_t late_grants;
  uint32_t grants_in_frame;
  uint32_t reserved0; /* 0 */
  uint16_t row_latch[IVS_STIC_LATCH_ROWS][IVS_STIC_ROW_WORDS];
} ivs_stic_state;

_Static_assert(sizeof(ivs_stic_state) == 528u, "ivs_stic_state must be 528 bytes (machine.h layout)");

/* Sets s to the power-on state (D-06): every member 0 (frame_index 0 and event_index 0, so FRAME_START of
 * frame 0 is still to be processed; windows_active 0; cs_ptr 0; the counters and the row latches 0),
 * except mode and display_latch, which come from the profile's stic.power_on record. */
void ivs_stic_state_reset(ivs_stic_state *s, const ivs_stic_params *p);

/* IVS_OK when every member is in range (the cursor below IVS_STIC_EVENT_MAX, display_latch and
 * windows_active 0 or 1, mode 0 or 1, cs_ptr below 4, reserved0 0; frame_index, the counters and the
 * row latches are any value), IVS_E_BAD_STATE otherwise, IVS_E_BAD_ARG for NULL. In the style of
 * ivs_cp1610_state_validate: a Phase 8 snapshot reload refuses a state this function does not accept. */
ivs_status ivs_stic_state_validate(const ivs_stic_state *s);

/* The frame sink of the STIC (plan 06-09 delivers frames through it); opaque until then, NULL accepted. */
typedef struct ivs_stic_sink ivs_stic_sink;

/* Moves the timeline cursor of s to machine tick to_tick: applies, in order, every event of the per-frame table whose
 * tick is at or below to_tick (the next tick is phase + frame_index x F + event_tick[event_index], from the state),
 * opening and closing the register and graphics-memory gates of mem and deciding windows_active (stic_events.c, D-05,
 * D-07, D-10, A-06). Idempotent for one to_tick; sink is unused in plan 06-06. */
void ivs_stic_catch_up(const ivs_stic_params *p, ivs_stic_state *s, ivs_memsys *mem, uint64_t to_tick,
                       ivs_stic_sink *sink);

/* Access effects (stic_regs.c, plan 06-06; pure functions of the parameters, no address constant). access is
 * IVS_STIC_ACCESS_READ (a READ or FETCH) or IVS_STIC_ACCESS_WRITE (a WRITE or DW); the result is an IVS_STIC_EFFECT_*.
 * A read has an effect only when its address is in stic.mode_select_addrs (decided on the address before decode, A-07). */
uint32_t ivs_stic_is_mode_select_addr(const ivs_stic_params *p, uint32_t addr);
uint32_t ivs_stic_effect(const ivs_stic_params *p, uint32_t access, uint32_t addr);
/* Applies an effect to the display latch or the mode of s; any other value does nothing. */
void ivs_stic_apply_effect(ivs_stic_state *s, uint32_t effect);
/* Counts one refused access (blocked_reads when is_read is nonzero, else blocked_writes), saturating at 0xFFFFFFFF. */
void ivs_stic_count_blocked(ivs_stic_state *s, uint32_t is_read);

#endif
