/* SPDX-License-Identifier: MIT */
/* STIC line levels (plan 06-08; D-08, D-09, A-09): ivs_stic_lines, the pure function from (parameters, state, tick) to the INTRM and
 * BUSRQ levels. The machine step seam (machine_run.c) samples it at every unit start, one sampling lag before the start, and hands
 * the levels to the CPU stepper, which is unchanged: the stall of a bus window is the stepper's own BUS_GRANT unit sequence.
 * Nothing here advances state or holds static data: the frame in progress comes from the state (frame_index), never from tick / F.
 * Named faults (plan 06-08): STIC_IRQ_LEVEL_NEVER_RELEASED (INTRM stays asserted to the frame end) and STIC_BUSRQ_WHEN_DISPLAY_OFF
 * (windows_active is ignored, so a frame whose display latch was never strobed still drives bus requests). */
#include "common/ivs_prelude.h"

#include "stic/stic.h"

ivs_stic_line_levels ivs_stic_lines(const ivs_stic_params *p, const ivs_stic_state *s, uint64_t tick)
{
  ivs_stic_line_levels l;
  uint64_t sk = (uint64_t)p->frame_phase_ticks + s->frame_index * (uint64_t)p->frame_ticks;
  uint64_t rel;
  uint32_t active = s->windows_active;
  l.intrm = 0u;
  l.busrq = 0u;
  if (tick < sk) {
    return l; /* the previous frame: both levels are 0 there */
  }
  rel = tick - sk;
  if (rel < (uint64_t)p->irq_low_ticks) {
    l.intrm = 1u;
  }
#ifdef IVS_FAULT_STIC_IRQ_LEVEL_NEVER_RELEASED
  if (rel < (uint64_t)p->frame_ticks) {
    l.intrm = 1u; /* named fault: the level is never released inside the frame */
  }
#endif
#ifdef IVS_FAULT_STIC_BUSRQ_WHEN_DISPLAY_OFF
  active = 1u; /* named fault: the display latch decision is ignored */
#endif
  if (active != 0u && rel >= (uint64_t)p->bus_first_start_ticks && p->bus_pitch_ticks != 0u) {
    uint64_t off = rel - (uint64_t)p->bus_first_start_ticks;
    uint64_t index = off / (uint64_t)p->bus_pitch_ticks;
    if (index < (uint64_t)p->bus_count && off % (uint64_t)p->bus_pitch_ticks < (uint64_t)p->bus_hold_ticks) {
      l.busrq = 1u;
    }
  }
  return l;
}
