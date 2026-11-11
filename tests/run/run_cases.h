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

/* Family bound (plan 04-07, D-01..D-05): ivs_machine_run targets, overshoot, single step, stop
 * reasons, the refusal ladder and stop names (run_bound.c). */
extern const ivs_test_case ivs_run_bound_cases[];
extern const size_t ivs_run_bound_count;

/* Family info (plan 04-09, D-06, D-09, A-02): the timebase and output layout of ivs_info and the
 * v1-size growth rule (run_info.c). */
extern const ivs_test_case ivs_run_info_cases[];
extern const size_t ivs_run_info_count;

/* Family slice (plan 04-09, D-07): five slicing patterns of two stand-in programs and the target
 * edges against a seam-stepped unit-by-unit reference (run_slice.c). */
extern const ivs_test_case ivs_run_slice_cases[];
extern const size_t ivs_run_slice_count;

/* Family buffers (plan 04-10, D-02, D-03, D-05, D-06, API-04): the caller-owned output buffers of
 * ivs_machine_run, their capacity rule through the internal env seam with a synthetic layout,
 * their refusals and the real zero-length layout (run_buffers.c). */
extern const ivs_test_case ivs_run_buffers_cases[];
extern const size_t ivs_run_buffers_count;

/* Family input (plan 04-11, D-12..D-16, A-04, A-08): the input schedule of ivs_machine_run, its
 * timing at unit boundaries, its validation and the INPUT log against an independent literal
 * chain oracle (run_input.c). */
extern const ivs_test_case ivs_run_input_cases[];
extern const size_t ivs_run_input_count;

/* Family fuzz (plan 04-13, D-12..D-16, A-02): the seeded input/slicing fuzz against a unit-by-unit
 * reference and the frozen v1 sizes of the Phase 4 public structs (run_fuzz.c). */
extern const ivs_test_case ivs_run_fuzz_cases[];
extern const size_t ivs_run_fuzz_count;

/* Family stub (plan 04-14, D-28..D-30): the project boot stub from the build-tree image of
 * tools/pack, from reset to the cartridge entry and its unexpected-interrupt trap (run_stub.c). */
extern const ivs_test_case ivs_run_stub_cases[];
extern const size_t ivs_run_stub_count;

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
