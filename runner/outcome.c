/* SPDX-License-Identifier: MIT */
/* Runner outcome taxonomy (D-20, plan 04-03): exit codes, names and the fixed precedence.
 * Pure: ISO headers only, no I/O, no state. Named-fault sites (data/expected/faults/runner.ids)
 * sit in this TU. */
#include "ivs_runner.h"

uint32_t ivs_runner_exit_code(uint32_t outcome)
{
  switch (outcome) {
  case IVS_RUNNER_OUTCOME_PASS:
    return 0u;
  case IVS_RUNNER_OUTCOME_GUEST_FAIL:
#ifdef IVS_FAULT_OUTCOME_GUEST_AS_HARNESS
    return 14u;
#else
    return 10u;
#endif
  case IVS_RUNNER_OUTCOME_TIMEOUT_BUDGET:
  case IVS_RUNNER_OUTCOME_TIMEOUT_WALL:
#ifdef IVS_FAULT_OUTCOME_TIMEOUT_AS_GUEST
    return 10u;
#else
    return 11u;
#endif
  case IVS_RUNNER_OUTCOME_CANCELLED:
    return 12u;
  case IVS_RUNNER_OUTCOME_MISSING_FIXTURE:
#ifdef IVS_FAULT_OUTCOME_MISSING_FIXTURE_EXIT0
    return 0u;
#else
    return 13u;
#endif
  default:
    return 14u;
  }
}

const char *ivs_runner_outcome_name(uint32_t outcome)
{
  switch (outcome) {
  case IVS_RUNNER_OUTCOME_PASS:
    return "pass";
  case IVS_RUNNER_OUTCOME_GUEST_FAIL:
    return "guest-fail";
  case IVS_RUNNER_OUTCOME_TIMEOUT_BUDGET:
  case IVS_RUNNER_OUTCOME_TIMEOUT_WALL:
    return "timeout";
  case IVS_RUNNER_OUTCOME_CANCELLED:
    return "cancelled";
  case IVS_RUNNER_OUTCOME_MISSING_FIXTURE:
    return "missing-fixture";
  default:
    return "harness-fail";
  }
}

uint32_t ivs_runner_outcome_resolve(uint32_t observed_mask)
{
  if ((observed_mask & ~IVS_RUNNER_OBS_ALL) != 0u || (observed_mask & IVS_RUNNER_OBS_HARNESS_FAIL) != 0u) {
    return IVS_RUNNER_OUTCOME_HARNESS_FAIL;
  }
  if ((observed_mask & IVS_RUNNER_OBS_MISSING_FIXTURE) != 0u) {
    return IVS_RUNNER_OUTCOME_MISSING_FIXTURE;
  }
#ifdef IVS_FAULT_OUTCOME_CANCEL_AS_TIMEOUT
  if ((observed_mask & IVS_RUNNER_OBS_TIMEOUT_WALL) != 0u) {
    return IVS_RUNNER_OUTCOME_TIMEOUT_WALL;
  }
#endif
  if ((observed_mask & IVS_RUNNER_OBS_CANCELLED) != 0u) {
    return IVS_RUNNER_OUTCOME_CANCELLED;
  }
  if ((observed_mask & IVS_RUNNER_OBS_TIMEOUT_WALL) != 0u) {
    return IVS_RUNNER_OUTCOME_TIMEOUT_WALL;
  }
  if ((observed_mask & IVS_RUNNER_OBS_TIMEOUT_BUDGET) != 0u) {
    return IVS_RUNNER_OUTCOME_TIMEOUT_BUDGET;
  }
  if ((observed_mask & IVS_RUNNER_OBS_GUEST_FAIL) != 0u) {
    return IVS_RUNNER_OUTCOME_GUEST_FAIL;
  }
  return IVS_RUNNER_OUTCOME_PASS;
}
