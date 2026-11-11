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
 * Validation (ivs_input_check) reads the whole array before the run writes anything: first every
 * event for order, range and floor (BAD_ARG), then every event for flags, the reserved word and
 * reserved control bits (UNSUPPORTED); the lowest failing index of the first failing pass wins.
 * The file holds no static or global state outside named-fault blocks.
 * Named faults (plan 04-11, D-07, D-16): INPUT_LOG_SLICE_RELATIVE_TICK, INPUT_NO_FLOOR_CHECK,
 * INPUT_STATIC_CARRY, RUN_EVENT_AT_TARGET_ONLY. */
#include "common/ivs_prelude.h"

#include "common/ivs_sha256.h"
#include "ivs/ivs.h"
#include "machine/machine.h"

_Static_assert(sizeof(ivs_input_event) == 32u, "ivs_input_event is 32 bytes");
_Static_assert(IVS_INPUT_CONTROLLER_WORDS == 4u, "four controller words per event");

#ifdef IVS_FAULT_INPUT_STATIC_CARRY
/* Named fault INPUT_STATIC_CARRY (plan 04-11, D-16): a file-scope copy of the last words applied
 * by any instance becomes the words in force of whichever instance begins a unit next, so input
 * leaks between instances (run.input.isolation must catch it). */
static uint32_t ivs_input_carry[IVS_INPUT_CONTROLLER_WORDS];
#endif

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
#ifdef IVS_FAULT_INPUT_STATIC_CARRY
  memcpy(ivs_input_carry, ev->controller, sizeof ivs_input_carry);
#endif
}

uint32_t ivs_input_check(const ivs_machine_obs_input *in, const ivs_input_event *ev, uint32_t count,
                         ivs_status *st, uint32_t *index)
{
  uint32_t i;
  /* Pass 1, BAD_ARG: order, range and floor over the whole array (the lowest failing index). */
  for (i = 0u; i < count; i++) {
    uint64_t t = ev[i].at_tick;
    uint32_t reason = IVS_REASON_NONE;
    if (t >= IVS_RUN_TARGET_MAX) {
      reason = IVS_REASON_INPUT_TICK_RANGE;
    } else if (i > 0u && t < ev[i - 1u].at_tick) {
      reason = IVS_REASON_INPUT_UNSORTED;
    }
#ifndef IVS_FAULT_INPUT_NO_FLOOR_CHECK /* named fault INPUT_NO_FLOOR_CHECK: events below the floor pass */
    else if (t < in->floor_tick) {
      reason = IVS_REASON_INPUT_BEFORE_FLOOR;
    }
#endif
    if (reason != IVS_REASON_NONE) {
      *st = IVS_E_BAD_ARG;
      *index = i;
      return reason;
    }
  }
  /* Pass 2, UNSUPPORTED: flags, the reserved word, reserved control bits. */
  for (i = 0u; i < count; i++) {
    uint32_t reason = IVS_REASON_NONE;
    uint32_t w;
    if (ev[i].flags != 0u) {
      reason = IVS_REASON_INPUT_EVENT_FLAGS;
    } else if (ev[i].reserved != 0u) {
      reason = IVS_REASON_INPUT_EVENT_RESERVED;
    }
    for (w = 0u; reason == IVS_REASON_NONE && w < IVS_INPUT_CONTROLLER_WORDS; w++) {
      if ((ev[i].controller[w] & IVS_CTRL_RESERVED_MASK) != 0u) {
        reason = IVS_REASON_INPUT_CONTROL_RESERVED;
      }
    }
    if (reason != IVS_REASON_NONE) {
      *st = IVS_E_UNSUPPORTED;
      *index = i;
      return reason;
    }
  }
  (void)in; /* read by the floor test; named fault INPUT_NO_FLOOR_CHECK drops it */
  *st = IVS_OK;
  *index = IVS_INPUT_INDEX_NONE;
  return IVS_REASON_NONE;
}

uint32_t ivs_input_apply_due(struct ivs_machine *im, const ivs_input_event *ev, uint32_t count, uint32_t next,
                             uint64_t start, uint64_t entry)
{
  while (next < count) {
    ivs_input_event e = ev[next]; /* one read of the caller's event */
    uint64_t tick = e.at_tick;
#ifdef IVS_FAULT_RUN_EVENT_AT_TARGET_ONLY
    /* Named fault RUN_EVENT_AT_TARGET_ONLY (plan 04-11, D-07): an event applies only at a unit that
     * starts exactly at its at_tick, so an event inside a unit is never applied. */
    if (e.at_tick != start) {
      break;
    }
#else
    if (e.at_tick > start) {
      break;
    }
#endif
#ifdef IVS_FAULT_INPUT_LOG_SLICE_RELATIVE_TICK
    /* Named fault INPUT_LOG_SLICE_RELATIVE_TICK (plan 04-11, D-16): the log records the tick
     * relative to the call's entry, so the chain depends on how the host slices. */
    tick -= entry;
#endif
    ivs_input_apply(&im->obs.input, &e, tick);
    next++;
  }
#ifdef IVS_FAULT_INPUT_STATIC_CARRY
  memcpy(im->obs.input.words, ivs_input_carry, sizeof ivs_input_carry);
#endif
  (void)entry;
  return next;
}
