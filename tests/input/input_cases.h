/* SPDX-License-Identifier: MIT */
/* Suite input (INP-01, INP-02, INP-03, API-03, plan 05-05): the two hand controllers as guest-visible
 * machine input through the PSG ports at $01FE and $01FF. One executable; each family file defines its
 * case table declared here and input_main.c concatenates them into one suite. Expected values come
 * from sourced text only: the controller literal data/input/<profile>.literal.txt (turned into a C
 * include by cmake/IvsInputProbes.cmake, which reads neither the profile nor the core) and the four
 * statements of the wiki prose (KEY_1 reads $7E, keys 1 and 9 read $5A, an idle port reads $FF,
 * written here as literals), never from the profile, the generated ivs_ctrl_lines table or a core
 * output. The shared helpers are static inline so every family file can use them. */
#ifndef IVS_INPUT_CASES_H
#define IVS_INPUT_CASES_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs/ivs_run.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "machine_cases.h"

#define II_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      goto done;                                                                                       \
    }                                                                                                  \
  } while (0)

#define II_POISON 0xA5u
/* Ticks that cover the two reads of stand-in program PORTS several times over. */
#define II_RUN_TICKS 4000u

/* The families (one source file each). */
extern const ivs_test_case ivs_input_single_cases[];
extern const size_t ivs_input_single_count;

/* One event: words 0 and 1 (ports 0 and 1) at at_tick. */
static inline void ii_event(ivs_input_event *e, uint64_t at, uint32_t w0, uint32_t w1)
{
  memset(e, 0, sizeof *e);
  e->at_tick = at;
  e->controller[0] = w0;
  e->controller[1] = w1;
}

/* One run call to target with n events; the report is poisoned first (separate statements). */
static inline ivs_status ii_run(ivs_machine *m, uint64_t target, const ivs_input_event *ev, uint32_t n,
                                ivs_run_report *rep)
{
  ivs_run_request q;
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.target_ticks = target;
  q.events = ev;
  q.event_count = n;
  memset(rep, II_POISON, sizeof *rep);
  rep->struct_size = (uint32_t)sizeof *rep;
  return ivs_machine_run(m, &q, rep);
}

/* Peeks one word at addr; 0xFFFFFFFF when the peek refuses. */
static inline uint32_t ii_peek(const ivs_machine *m, uint16_t addr)
{
  uint16_t w = 0u;
  if (ivs_machine_peek(m, addr, 1u, &w) != IVS_OK) {
    return 0xFFFFFFFFu;
  }
  return w;
}

#endif
