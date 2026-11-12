/* SPDX-License-Identifier: MIT */
/* PSG device state storage (plan 05-03) and the power-on reset of the state struct (plan 05-06: the
 * first chip step is step 1, so next_sample starts at 1). */
#include "common/ivs_prelude.h"

#include "psg/psg.h"

void ivs_psg_state_reset(ivs_psg_state *s)
{
  memset(s, 0, sizeof *s);
  s->next_sample = 1u;
}

void ivs_psg_sink_init(ivs_psg_sink *k, uint8_t *buf, uint64_t cap)
{
  memset(k, 0, sizeof *k);
  k->buf = buf;
  k->cap = buf != NULL ? cap : 0u;
  k->fresh = 1u;
}
