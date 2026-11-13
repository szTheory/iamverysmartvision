/* SPDX-License-Identifier: MIT */
/* Suite stic, family slice (plan 06-08; D-10): nothing about the host's call slicing may change a byte of the instance. The guest is the
 * interrupt program VBL (EIS, a DIS-first counting ISR that strobes the display and stores its counter into BACKTAB, a display-on
 * loop), so every call crosses frame starts, bus windows, interrupt entries, row fetches and BACKTAB stores. The reference is the same
 * guest advanced one unit per call (run(now + 1)); the comparison is a memcmp of the whole instance storage (114 KB, every byte the
 * machine owns), never a digest.
 *   stic.slice.one-call-vs-1 .. .7 .. .468 .. .prime   calls of 1, 7, 468 and 97 ticks (the last is prime); the 1-tick case also equals
 *                                                       one single call to the same end (a thinned comparison, see ss_compare) */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "machine/machine.h"
#include "machine_cases.h"
#include "stic_cases.h"

static uint32_t ss_xs(uint32_t *s)
{
  uint32_t x = *s;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  *s = x;
  return x;
}

static int ss_call(ivs_machine *m, uint64_t target)
{
  ivs_run_request q;
  ivs_run_report rep;
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.target_ticks = target;
  memset(&rep, 0, sizeof rep);
  rep.struct_size = (uint32_t)sizeof rep;
  return ivs_machine_run(m, &q, &rep) == IVS_OK;
}

int ss_compare(const uint16_t *w, size_t n, uint32_t slice, uint32_t seed, uint64_t total, uint32_t stride, int big, uint32_t *events)
{
  ivs_ms_inst a;
  ivs_ms_inst b;
  ivs_ms_inst c;
  const struct ivs_machine *ia;
  const struct ivs_machine *ib;
  uint32_t rng = seed;
  uint32_t since_event = 1000u;
  uint32_t calls = 0u;
  uint16_t prev_sp = 0u;
  uint8_t prev_busak = 0u;
  uint32_t prev_ev = 0u;
  int ok = 0;
  memset(&a, 0, sizeof a);
  memset(&b, 0, sizeof b);
  memset(&c, 0, sizeof c);
  *events = 0u;
  if (!sc_guest_n(&a, w, n)) {
    return 0;
  }
  if (!sc_guest_n(&b, w, n)) {
    ivs_ms_free(&a);
    return 0;
  }
  ia = ivs_machine_from_handle(a.m);
  ib = ivs_machine_from_handle(b.m);
  while (ia->time_ticks < total) {
    uint32_t sl = seed != 0u ? 1u + ss_xs(&rng) % 2999u : slice;
    int compare;
    if (!ss_call(a.m, ia->time_ticks + sl)) {
      goto done;
    }
    while (ib->time_ticks < ia->time_ticks) {
      if (!ss_call(b.m, ib->time_ticks + 1u)) {
        goto done;
      }
      if (ib->cpu.busak != prev_busak || ib->cpu.r[6] != prev_sp || ib->stic.event_index != prev_ev) {
        since_event = 0u;
        *events |= ib->cpu.busak != prev_busak ? 1u : 0u;
        *events |= (ib->cpu.r[6] != prev_sp && ib->cpu.r[6] > prev_sp) ? 2u : 0u;
      } else {
        since_event += 1u;
      }
      prev_busak = ib->cpu.busak;
      prev_sp = ib->cpu.r[6];
      prev_ev = ib->stic.event_index;
    }
    calls++;
    compare = stride == 1u || since_event < 64u || calls % stride == 0u;
    if (ib->time_ticks != ia->time_ticks) {
      ivs_test_diag("call %u: the reference is at %llu, the sliced instance at %llu", (unsigned)calls,
                    (unsigned long long)ib->time_ticks, (unsigned long long)ia->time_ticks);
      goto done;
    }
    if (compare && memcmp(a.storage, b.storage, IVS_MACHINE_BYTES) != 0) {
      ivs_test_diag("call %u at tick %llu: the instance differs from the unit-by-unit reference", (unsigned)calls,
                    (unsigned long long)ia->time_ticks);
      goto done;
    }
  }
  if (memcmp(a.storage, b.storage, IVS_MACHINE_BYTES) != 0) {
    ivs_test_diag("the final instance differs from the reference");
    goto done;
  }
  if (big != 0) {
    if (!sc_guest_n(&c, w, n) || !ss_call(c.m, ia->time_ticks) || memcmp(a.storage, c.storage, IVS_MACHINE_BYTES) != 0) {
      ivs_test_diag("one call to tick %llu differs from %u calls", (unsigned long long)ia->time_ticks, (unsigned)calls);
      goto done;
    }
  }
  ok = 1;
done:
  ivs_ms_free(&a);
  ivs_ms_free(&b);
  if (c.storage != NULL) {
    ivs_ms_free(&c);
  }
  return ok;
}

static int ss_slice(uint32_t slice, uint32_t stride, uint64_t frames, int big)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint16_t w[SC_PROG_MAX + 8u];
  uint32_t events = 0u;
  size_t n = sc_prog_vbl(w, SC_BODY_J | SC_BODY_BACKTAB);
  if (!ss_compare(w, n, slice, 0u, frames * (uint64_t)p->frame_ticks, stride, big, &events)) {
    return IVS_TEST_FAIL;
  }
  if ((events & 3u) != 3u) { /* non-vacuity: the span held a bus grant and an interrupt entry */
    ivs_test_diag("vacuous: events %u", (unsigned)events);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

static int ss_one(void) { return ss_slice(1u, 64u, 3u, 1); }
static int ss_seven(void) { return ss_slice(7u, 1u, 3u, 0); }
static int ss_468(void) { return ss_slice(468u, 1u, 3u, 0); }
static int ss_prime(void) { return ss_slice(97u, 1u, 3u, 0); }

const ivs_test_case ivs_stic_slice_cases[] = {
    {"stic.slice.one-call-vs-1", ss_one, NULL},
    {"stic.slice.one-call-vs-7", ss_seven, NULL},
    {"stic.slice.one-call-vs-468", ss_468, NULL},
    {"stic.slice.one-call-vs-prime", ss_prime, NULL},
};
const size_t ivs_stic_slice_count = sizeof ivs_stic_slice_cases / sizeof ivs_stic_slice_cases[0];
