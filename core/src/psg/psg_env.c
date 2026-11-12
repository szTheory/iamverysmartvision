/* SPDX-License-Identifier: MIT */
/* PSG envelope generator (plan 05-07, D-10, AUD-01, rules PSG-R11..R13, PSG-R07): the 16-state machine of
 * the 16 shapes. State (psg.h): level 0..15, direction (env_dir 1 = rising), held flag, the index of the
 * current state inside its 16-state cycle (env_cycles, 0..15) and the step counter.
 * Start (a shape write, ivs_psg_env_restart): level = attack ? 0 : 15, direction to match, state 0, not
 * held. One envelope state lasts psg.envelope.steps_per_ep x EP chip steps (2 EP on the NTSC profile, C72),
 * EP the 16-bit period (coarse at $01F7 over fine at $01F3) with a written 0 acting as
 * psg.period_zero.envelope (65536). The step counter follows the convention of the tone (policy POL-PSG-02):
 * empty at a restart and reloaded from the effective period on the step that finds it empty, so the first
 * level change after a shape write is at step 2 EP and the level of step 1 is the initial level. That phase
 * is a convention, not a page statement; the restart property and the 2 EP step rate are the ones the
 * vectors and the blind second read pin (docs/psg/second-read-record-env.md).
 * At the end of a cycle (the advance of state 15) the shape bits of the stored shape register decide
 * (psg.envelope.shape_bits): continue 0 holds level 0; continue 1 with hold 1 holds the last level when
 * alternate is 0 and the INITIAL level of the finished cycle when alternate is 1 (shape 1011 holds 15,
 * shape 1111 holds 0); continue 1 with hold 0 starts a new cycle at the initial level when alternate is 0
 * and reverses the direction when alternate is 1 (a new cycle begins at its own initial level, so the
 * extreme level is doubled across the boundary). The 16-state table is the engine's constant: the 32-step
 * envelope is a different part (C12). A write to the shape register restarts the generator; a period write
 * does not. After a reset the generator is held at level 0 until the first shape write (psg.reset, C77).
 * No static or global state.
 * Named faults (plan 05-07, suite psg): PSG_ENV_SHAPE_ATTACK_DECAY_SWAPPED, PSG_ENV_PERIOD_TIMES_16,
 * PSG_ENV_HOLD_ALTERNATE_NO_RESET. */
#include "common/ivs_prelude.h"

#include "psg/psg.h"

#define IVS_PSG_ENV_STATES 16u /* states per cycle (the 8910 table); the level range is 0..15 */

uint32_t ivs_psg_env_period(const ivs_psg_params *p, const uint16_t *regs)
{
  uint32_t fine = (uint32_t)regs[IVS_PSG_SLOT_ENV_FINE] & 0xFFu;
  uint32_t coarse = (uint32_t)regs[IVS_PSG_SLOT_ENV_COARSE] & 0xFFu;
  uint32_t ep = (coarse << 8) | fine;
  return ep == 0u ? p->period_zero_env : ep;
}

static uint32_t ivs_psg_shape_bit(const ivs_psg_params *p, uint32_t shape, uint32_t which)
{
  uint32_t bit = 0u;
  switch (which) {
  case 0u:
    bit = p->env_bit_continue;
    break;
  case 1u:
    bit = p->env_bit_attack;
    break;
  case 2u:
    bit = p->env_bit_alternate;
    break;
  default:
    bit = p->env_bit_hold;
    break;
  }
  return (shape >> bit) & 1u;
}

void ivs_psg_env_restart(const ivs_psg_params *p, ivs_psg_state *s, uint32_t shape)
{
  uint32_t attack = ivs_psg_shape_bit(p, shape, 1u);
#ifdef IVS_FAULT_PSG_ENV_SHAPE_ATTACK_DECAY_SWAPPED
  attack ^= 1u; /* named fault: attack and decay are swapped, a rising shape falls and a falling shape rises */
#endif
  s->env_dir = attack;
  s->env_level = attack != 0u ? 0u : 15u;
  s->env_held = 0u;
  s->env_cycles = 0u;
  s->env_counter = 0u;
}

/* The advance to the next state: inside a cycle one level in the current direction, at the end of the
 * cycle the shape bits decide. */
static void ivs_psg_env_advance(const ivs_psg_params *p, ivs_psg_state *s, uint32_t shape)
{
  uint32_t initial;
  if (s->env_cycles + 1u < IVS_PSG_ENV_STATES) {
    s->env_cycles += 1u;
    if (s->env_dir != 0u) {
      s->env_level += 1u;
    } else {
      s->env_level -= 1u;
    }
    return;
  }
  initial = s->env_dir != 0u ? 0u : 15u;
  if (ivs_psg_shape_bit(p, shape, 0u) == 0u) {
    s->env_level = 0u;
    s->env_held = 1u;
    return;
  }
  if (ivs_psg_shape_bit(p, shape, 3u) != 0u) {
    uint32_t alternate = ivs_psg_shape_bit(p, shape, 2u);
#ifdef IVS_FAULT_PSG_ENV_HOLD_ALTERNATE_NO_RESET
    alternate = 0u; /* named fault: a held alternating shape keeps its last level (1011 holds 0, not 15) */
#endif
    if (alternate != 0u) {
      s->env_level = initial;
    }
    s->env_held = 1u;
    return;
  }
  if (ivs_psg_shape_bit(p, shape, 2u) != 0u) {
    s->env_dir ^= 1u;
  }
  s->env_level = s->env_dir != 0u ? 0u : 15u;
  s->env_cycles = 0u;
}

void ivs_psg_env_step(const ivs_psg_params *p, ivs_psg_state *s, const uint16_t *regs)
{
  if (s->env_held != 0u) {
    return;
  }
  if (s->env_counter == 0u) {
    uint32_t reload = ivs_psg_env_period(p, regs);
#ifdef IVS_FAULT_PSG_ENV_PERIOD_TIMES_16
    reload *= 16u; /* named fault: every envelope state lasts 16 times too many steps */
#endif
    s->env_counter = p->env_steps_per_ep * reload;
  }
  s->env_counter -= 1u;
  if (s->env_counter == 0u) {
    ivs_psg_env_advance(p, s, (uint32_t)regs[IVS_PSG_SLOT_ENV_SHAPE]);
  }
}

void ivs_psg_after_write(const ivs_psg_params *p, ivs_psg_state *s, const uint16_t *regs, uint32_t idx)
{
  if (idx == IVS_PSG_SLOT_ENV_SHAPE && p->env_restart_on_write != 0u) {
    ivs_psg_env_restart(p, s, (uint32_t)regs[IVS_PSG_SLOT_ENV_SHAPE]);
  }
}
