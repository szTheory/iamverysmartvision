/* SPDX-License-Identifier: MIT */
/* STIC device state: the power-on reset and the range check (plan 06-05, D-04, D-06, D-16). No behaviour
 * runs against this state yet; see stic.h. */
#include "common/ivs_prelude.h"

#include "stic/stic.h"

void ivs_stic_state_reset(ivs_stic_state *s, const ivs_stic_params *p)
{
  memset(s, 0, sizeof *s);
  if (p != NULL) {
    s->mode = p->power_on_mode;
    s->display_latch = p->power_on_display_latch;
  }
}

ivs_status ivs_stic_state_validate(const ivs_stic_state *s)
{
  if (s == NULL) {
    return IVS_E_BAD_ARG;
  }
  if (s->event_index >= IVS_STIC_EVENT_MAX || s->display_latch > 1u || s->windows_active > 1u || s->mode > 1u ||
      s->cs_ptr >= 4u || s->reserved0 != 0u) {
    return IVS_E_BAD_STATE;
  }
  return IVS_OK;
}
