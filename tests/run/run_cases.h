/* SPDX-License-Identifier: MIT */
/* Suite run (A-06, plan 04-06): run, observation, buffer and input tests through the public API,
 * in their own executable so the 512-case machine suite and its faults stay bounded (Pitfall
 * 12). Each family file defines its case table and count here declared; run_main.c concatenates
 * them into one suite. Stand-in content comes from tests/machine/machine_standin.c (the ivs_ms_*
 * helpers of machine_cases.h, D-24: no rights-bearing byte). */
#ifndef IVS_RUN_CASES_H
#define IVS_RUN_CASES_H

#include <stddef.h>
#include <stdint.h>

#include "ivs/ivs.h"
#include "ivs_test.h"
#include "machine_cases.h"

/* Family observe (plan 04-06, D-08): ivs_machine_cpu_state and ivs_machine_peek, their
 * side-effect freedom and their refusals (run_observe.c). */
extern const ivs_test_case ivs_run_observe_cases[];
extern const size_t ivs_run_observe_count;

/* ---- the refusal and side-effect oracle (SP4), shared by the run families ----
 * A snapshot holds a heap copy of the whole instance storage, the composite and the seven domain
 * digests (1..4, 16..18) and info's time_ticks; a destroyed instance has no digests or info. */
#define IVS_RUN_SNAP_DIGESTS 8u

typedef struct ivs_run_snap {
  uint8_t *copy; /* IVS_MACHINE_BYTES of heap */
  uint8_t digest[IVS_RUN_SNAP_DIGESTS][32];
  uint64_t time_ticks;
  int live; /* 1 when digests and time were taken */
} ivs_run_snap;

/* Takes the snapshot of x (digests and time only when live is 1). 1 on success. */
int ivs_run_snap_take(ivs_run_snap *s, const ivs_ms_inst *x, int live);
/* 1 when x is byte-identical to the snapshot and, for a live snapshot, its digests and time are
 * equal; else a diagnostic naming what changed and 0. */
int ivs_run_snap_same(const ivs_run_snap *s, const ivs_ms_inst *x);
void ivs_run_snap_free(ivs_run_snap *s);

#endif
