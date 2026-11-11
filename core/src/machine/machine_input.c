/* SPDX-License-Identifier: MIT */
/* Input application and the input log (D-12..D-16, A-04, plan 04-11): the INPUT row of the
 * instance (obs.input, digest domain 18) and the functions the run call uses on it. Declared in
 * machine/machine.h; nothing here is public.
 *
 * Core input is logical switch state, one uint32_t per controller indexed by PSG port (D-12). An
 * applied event sets the words in force, adds one to the applied count and advances the chain
 *   h0 = SHA-256("IVS-INPUT-LOG-V1")  (the 16 ASCII bytes, no NUL)
 *   h(n+1) = SHA-256(h(n) || be64(at_tick) || be32(controller[0]) .. be32(controller[3]))
 * over the event's ABSOLUTE at_tick (the script tick, never a tick relative to a call or a unit),
 * so the log equals the accepted script whatever the slicing (D-15). The input floor is set by the
 * step seam (machine_run.c) when a unit begins. Nothing in the machine reads the words in Phase 4.
 * The file holds no static or global state. */
#include "common/ivs_prelude.h"

#include "common/ivs_sha256.h"
#include "ivs/ivs.h"
#include "machine/machine.h"

_Static_assert(sizeof(ivs_input_event) == 32u, "ivs_input_event is 32 bytes");
_Static_assert(IVS_INPUT_CONTROLLER_WORDS == 4u, "four controller words per event");

void ivs_input_init(ivs_machine_obs_input *in)
{
  static const char tag[] = "IVS-INPUT-LOG-V1";
  memset(in, 0, sizeof *in);
  ivs_sha256_digest((const uint8_t *)tag, sizeof tag - 1u, in->chain);
}

/* Applies one event logged at tick: words in force, count, chain. */
static void ivs_input_apply(ivs_machine_obs_input *in, const ivs_input_event *ev, uint64_t tick)
{
  uint8_t pre[32u + 8u + 4u * IVS_INPUT_CONTROLLER_WORDS];
  uint32_t i;
  memcpy(pre, in->chain, 32u);
  for (i = 0u; i < 8u; i++) {
    pre[32u + i] = (uint8_t)(tick >> (56u - 8u * i));
  }
  for (i = 0u; i < IVS_INPUT_CONTROLLER_WORDS; i++) {
    uint32_t w = ev->controller[i];
    pre[40u + 4u * i] = (uint8_t)(w >> 24);
    pre[41u + 4u * i] = (uint8_t)(w >> 16);
    pre[42u + 4u * i] = (uint8_t)(w >> 8);
    pre[43u + 4u * i] = (uint8_t)w;
    in->words[i] = w;
  }
  ivs_sha256_digest(pre, sizeof pre, in->chain);
  in->count++;
}

uint32_t ivs_input_apply_due(struct ivs_machine *im, const ivs_input_event *ev, uint32_t count, uint32_t next,
                             uint64_t start, uint64_t entry)
{
  while (next < count && ev[next].at_tick <= start) {
    uint64_t tick = ev[next].at_tick;
    ivs_input_apply(&im->obs.input, &ev[next], tick);
    next++;
  }
  (void)entry;
  return next;
}
