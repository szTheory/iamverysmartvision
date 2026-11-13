/* SPDX-License-Identifier: MIT */
/* STIC register access effects (plan 06-06; D-04, D-06, A-07): pure functions of the generated parameters and an access,
 * and the one place that applies an effect to the device state. No register address literal appears here: the rows of
 * stic.access_effects (register address, access R or W, effect) and the list stic.mode_select_addrs come from the
 * profile through ivs_stic_params.
 * ivs_stic_effect: a WRITE or DW takes the effect row of its register (address & 63, the write aliases included): arm
 * (the display-enable strobe) or the Foreground/Background mode select; a READ or FETCH takes an effect only when its
 * ADDRESS, before any decode, is a member of stic.mode_select_addrs (the alias addresses decode to the cartridge in the
 * read view, A-07), and then the effect row of that register (the Color Stack mode select). A read of the other
 * registers has no effect, so the read view of the register file and PEEK never switch a mode.
 * ivs_stic_apply_effect changes only the display latch and the mode. ivs_stic_count_blocked counts a refused access,
 * saturating at 0xFFFFFFFF. */
#include "common/ivs_prelude.h"

#include "stic/stic.h"

uint32_t ivs_stic_is_mode_select_addr(const ivs_stic_params *p, uint32_t addr)
{
  uint32_t i;
  for (i = 0u; i < p->mode_addr_count; i++) {
    if ((uint32_t)p->mode_addr[i] == (addr & 0xFFFFu)) {
      return 1u;
    }
  }
  return 0u;
}

uint32_t ivs_stic_effect(const ivs_stic_params *p, uint32_t access, uint32_t addr)
{
  uint32_t reg = addr & (IVS_STIC_REG_COUNT - 1u);
  uint32_t i;
  if (access == IVS_STIC_ACCESS_READ && ivs_stic_is_mode_select_addr(p, addr) == 0u) {
    return IVS_STIC_EFFECT_NONE;
  }
  for (i = 0u; i < p->effect_count; i++) {
    if ((uint32_t)p->effect_access[i] == access && ((uint32_t)p->effect_addr[i] & (IVS_STIC_REG_COUNT - 1u)) == reg) {
      return p->effect_code[i];
    }
  }
  return IVS_STIC_EFFECT_NONE;
}

void ivs_stic_apply_effect(ivs_stic_state *s, uint32_t effect)
{
  switch (effect) {
  case IVS_STIC_EFFECT_ARM:
    s->display_latch = 1u;
    break;
  case IVS_STIC_EFFECT_MODE_FGBG:
    s->mode = IVS_STIC_MODE_FGBG;
    break;
  case IVS_STIC_EFFECT_MODE_CS:
    s->mode = IVS_STIC_MODE_COLOR_STACK;
    break;
  default:
    break;
  }
}

void ivs_stic_count_blocked(ivs_stic_state *s, uint32_t is_read)
{
  uint32_t *c = is_read != 0u ? &s->blocked_reads : &s->blocked_writes;
  if (*c != 0xFFFFFFFFu) {
    *c += 1u;
  }
}
