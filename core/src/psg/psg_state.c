/* SPDX-License-Identifier: MIT */
/* PSG device state storage (plan 05-03): the power-on reset of the state struct. */
#include "common/ivs_prelude.h"

#include "psg/psg.h"

void ivs_psg_state_reset(ivs_psg_state *s)
{
  memset(s, 0, sizeof *s);
}
