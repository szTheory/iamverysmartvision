/* SPDX-License-Identifier: MIT */
/* STIC frame timeline (plan 06-06; D-05, D-06, D-07, D-10, A-06): the 16-entry per-frame event table of the
 * generated ivs_stic_params, a cursor (frame_index, event_index) that is STATE, and ivs_stic_catch_up, the one
 * function that moves it.
 *
 * Next event tick: phase + frame_index x F + event_tick[event_index], computed from the state, never from
 * to_tick / F (D-07: the frame index is a count of FRAME_END events, so a nonzero phase, a long run and a
 * sliced run all give the same cursor; named fault STIC_FRAME_BY_MODULO derives it by division instead).
 * Tie rule (A-06): an event whose tick is at or below to_tick is applied before the caller acts, so an access
 * stamped exactly at an event tick sees the event (step first, then act: an access at the close tick is blocked)
 * and a frame contains the accesses stamped strictly below its end (FRAME_END at F is followed, at the same tick,
 * by FRAME_START of the next frame, which is the next event of the loop). Named fault STIC_WINDOW_OFF_BY_ONE
 * applies an event only when its tick is strictly below to_tick.
 * Events (this plan; the renderer events arrive with the timeline plans):
 *   FRAME_START  clears display_latch, windows_active and grants_in_frame and opens the register gate and the
 *                graphics-memory gate. cs_ptr is NOT cleared here: it keeps the final pointer of the last
 *                rendered frame and the renderer starts each frame at 0.
 *   REG_CLOSE    closes the register gate (the STIC registers can no longer be reached).
 *   GMEM_CLOSE   closes the graphics-memory gate (GROM and GRAM) and decides windows_active from the display
 *                latch: the last event before window 0 starts; the latch can only change inside the register window.
 *   ROW_FETCH(i) (plan 06-08, D-11, A-05) copies the 20 BACKTAB words of card row i into row_latch[i] through the memory system's pure
 *                backing accessor (never the CPU view, no gate) when windows_active is set; inert otherwise. A guest store stamped
 *                before the event's tick is seen by the row, one stamped at or after it is not (the machine bus catches the STIC up to
 *                the store's stamp first, the tie rule A-06). Window 12 steals time and fetches nothing: the table has no event for it.
 *   FRAME_END    (plan 06-08, D-09) adds bus_count - grants_in_frame to late_grants when windows_active (saturating), then advances
 *                frame_index and wraps the cursor to 0; the late-grant policy is fetch-at-schedule: the fetch never waits for the CPU.
 * The interval before S_0 (a nonzero frame phase) schedules nothing: the first event of frame 0 is at the phase, so
 * both gates stay open until then. The gates are flipped only through ivs_memsys_set_gate_internal.
 * sink: accepted and unused until frames are delivered (plan 06-09). Calling twice with the same to_tick changes
 * nothing the second time (idempotent). No static or global state. */
#include "common/ivs_prelude.h"

#include "stic/stic.h"

void ivs_stic_catch_up(const ivs_stic_params *p, ivs_stic_state *s, ivs_memsys *mem, uint64_t to_tick,
                       ivs_stic_sink *sink)
{
  (void)sink;
  while (s->event_index < p->event_count) {
    uint32_t ei = s->event_index;
    uint32_t kind = p->event_kind[ei];
    uint64_t at = (uint64_t)p->frame_phase_ticks + s->frame_index * (uint64_t)p->frame_ticks + (uint64_t)p->event_tick[ei];
#ifdef IVS_FAULT_STIC_WINDOW_OFF_BY_ONE
    if (at >= to_tick) { /* named fault: an event is applied only when its tick is strictly below the stamp */
      break;
    }
#else
    if (at > to_tick) {
      break;
    }
#endif
    switch (kind) {
    case IVS_STIC_EV_FRAME_START:
      s->display_latch = 0u;
      s->windows_active = 0u;
      s->grants_in_frame = 0u;
      ivs_memsys_set_gate_internal(mem, IVS_MEMSYS_GATE_STIC_REGS, 1u);
      ivs_memsys_set_gate_internal(mem, IVS_MEMSYS_GATE_GMEM, 1u);
      break;
    case IVS_STIC_EV_REG_CLOSE:
      ivs_memsys_set_gate_internal(mem, IVS_MEMSYS_GATE_STIC_REGS, 0u);
      break;
    case IVS_STIC_EV_GMEM_CLOSE:
      ivs_memsys_set_gate_internal(mem, IVS_MEMSYS_GATE_GMEM, 0u);
      s->windows_active = s->display_latch != 0u ? 1u : 0u;
      break;
    case IVS_STIC_EV_ROW_FETCH: {
      uint32_t row = p->event_arg[ei];
      if (s->windows_active != 0u && row < IVS_STIC_LATCH_ROWS) {
        uint32_t c;
        for (c = 0u; c < IVS_STIC_ROW_WORDS; c++) {
          s->row_latch[row][c] = ivs_memsys_stic_backtab_word_internal(mem, row * IVS_STIC_ROW_WORDS + c);
        }
      }
      break;
    }
    case IVS_STIC_EV_FRAME_END:
      if (s->windows_active != 0u && s->grants_in_frame < p->bus_count) {
        uint32_t late = p->bus_count - s->grants_in_frame;
        s->late_grants = late > 0xFFFFFFFFu - s->late_grants ? 0xFFFFFFFFu : s->late_grants + late;
      }
      break;
    default:
      break;
    }
    if (kind == IVS_STIC_EV_FRAME_END) {
      s->frame_index += 1u;
      s->event_index = 0u;
    } else {
      s->event_index = ei + 1u;
    }
  }
#ifdef IVS_FAULT_STIC_FRAME_BY_MODULO
  if (p->frame_ticks != 0u) {
    s->frame_index = to_tick / (uint64_t)p->frame_ticks; /* named fault: the frame index is derived from time by division */
  }
#endif
}
