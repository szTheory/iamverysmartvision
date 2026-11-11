/* SPDX-License-Identifier: MIT */
/* Runner library interface (D-17, D-20, plan 04-03). Outcome taxonomy: an outcome value is its
 * process exit code (pass 0, guest-fail 10, timeout 11, cancelled 12, missing-fixture 13,
 * harness-fail 14; 1 is AddressSanitizer's default, 77 the ctest skip code and 126 and above
 * belong to the shell). A timeout carries a detail flag: IVS_RUNNER_OUTCOME_F_WALL marks the
 * host watchdog (host-dependent, never golden); without it the machine-time budget ended
 * (deterministic). Pure functions: no I/O, no state. */
#ifndef IVS_RUNNER_H
#define IVS_RUNNER_H

#include <stdint.h>

#define IVS_RUNNER_OUTCOME_PASS 0u
#define IVS_RUNNER_OUTCOME_GUEST_FAIL 10u
#define IVS_RUNNER_OUTCOME_TIMEOUT 11u
#define IVS_RUNNER_OUTCOME_CANCELLED 12u
#define IVS_RUNNER_OUTCOME_MISSING_FIXTURE 13u
#define IVS_RUNNER_OUTCOME_HARNESS_FAIL 14u

/* Timeout detail flag and the two timeout outcomes. */
#define IVS_RUNNER_OUTCOME_F_WALL 0x100u
#define IVS_RUNNER_OUTCOME_TIMEOUT_BUDGET IVS_RUNNER_OUTCOME_TIMEOUT
#define IVS_RUNNER_OUTCOME_TIMEOUT_WALL (IVS_RUNNER_OUTCOME_TIMEOUT | IVS_RUNNER_OUTCOME_F_WALL)

/* Observation bits for ivs_runner_outcome_resolve: what the run saw. */
#define IVS_RUNNER_OBS_GUEST_FAIL 0x01u
#define IVS_RUNNER_OBS_TIMEOUT_BUDGET 0x02u
#define IVS_RUNNER_OBS_TIMEOUT_WALL 0x04u
#define IVS_RUNNER_OBS_CANCELLED 0x08u
#define IVS_RUNNER_OBS_MISSING_FIXTURE 0x10u
#define IVS_RUNNER_OBS_HARNESS_FAIL 0x20u
#define IVS_RUNNER_OBS_ALL 0x3Fu

/* Exit code of an outcome. A value that is not one of the seven outcomes above is an internal
 * error and maps to the harness-fail code. */
uint32_t ivs_runner_exit_code(uint32_t outcome);

/* Report name of an outcome: pass, guest-fail, timeout (both kinds), cancelled,
 * missing-fixture, harness-fail; a value that is not an outcome is named harness-fail. */
const char *ivs_runner_outcome_name(uint32_t outcome);

/* The highest-precedence outcome present in observed_mask: harness-fail > missing-fixture >
 * cancelled > timeout (wall) > timeout (budget) > guest-fail > pass. An empty mask is pass; a
 * bit outside IVS_RUNNER_OBS_ALL is harness-fail. A load refusal that matches the scenario's
 * expect-load is decided by the executor before this call. */
uint32_t ivs_runner_outcome_resolve(uint32_t observed_mask);

#endif
