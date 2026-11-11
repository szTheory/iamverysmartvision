/* SPDX-License-Identifier: MIT */
/* Suite runner: the runner library in process (D-17, D-20, D-23, plan 04-03). Family outcome:
 * the exit code and report name of each of the six outcomes (both timeout kinds share code 11
 * and the name timeout). Named faults: data/expected/faults/runner.ids. */
#include <stdint.h>
#include <string.h>

#include "ivs_runner.h"
#include "ivs_test.h"

/* Fails the case with a diagnostic naming the line and the failed expression. */
#define IVS_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

static int name_is(uint32_t outcome, const char *want)
{
  const char *got = ivs_runner_outcome_name(outcome);
  if (got == NULL || strcmp(got, want) != 0) {
    ivs_test_diag("outcome 0x%x: name %s, want %s", (unsigned)outcome, got == NULL ? "(null)" : got, want);
    return 0;
  }
  return 1;
}

/* ---- exit codes (D-20) ---- */

static int out_exit_pass(void)
{
  IVS_CHECK(ivs_runner_exit_code(IVS_RUNNER_OUTCOME_PASS) == 0u);
  return IVS_TEST_PASS;
}

static int out_exit_guest_fail(void)
{
  IVS_CHECK(ivs_runner_exit_code(IVS_RUNNER_OUTCOME_GUEST_FAIL) == 10u);
  return IVS_TEST_PASS;
}

static int out_exit_timeout(void)
{
  IVS_CHECK(ivs_runner_exit_code(IVS_RUNNER_OUTCOME_TIMEOUT_BUDGET) == 11u);
  IVS_CHECK(ivs_runner_exit_code(IVS_RUNNER_OUTCOME_TIMEOUT_WALL) == 11u);
  return IVS_TEST_PASS;
}

static int out_exit_cancelled(void)
{
  IVS_CHECK(ivs_runner_exit_code(IVS_RUNNER_OUTCOME_CANCELLED) == 12u);
  return IVS_TEST_PASS;
}

static int out_exit_missing_fixture(void)
{
  IVS_CHECK(ivs_runner_exit_code(IVS_RUNNER_OUTCOME_MISSING_FIXTURE) == 13u);
  return IVS_TEST_PASS;
}

/* Harness-fail is 14, and so is every value that is not an outcome (internal error). */
static int out_exit_harness_fail(void)
{
  IVS_CHECK(ivs_runner_exit_code(IVS_RUNNER_OUTCOME_HARNESS_FAIL) == 14u);
  IVS_CHECK(ivs_runner_exit_code(1u) == 14u);
  IVS_CHECK(ivs_runner_exit_code(77u) == 14u);
  IVS_CHECK(ivs_runner_exit_code(IVS_RUNNER_OUTCOME_F_WALL) == 14u);
  IVS_CHECK(ivs_runner_exit_code(IVS_RUNNER_OUTCOME_GUEST_FAIL | IVS_RUNNER_OUTCOME_F_WALL) == 14u);
  IVS_CHECK(ivs_runner_exit_code(0xFFFFFFFFu) == 14u);
  return IVS_TEST_PASS;
}

/* ---- report names (D-19, D-20) ---- */

static int out_name_pass(void)
{
  IVS_CHECK(name_is(IVS_RUNNER_OUTCOME_PASS, "pass"));
  return IVS_TEST_PASS;
}

static int out_name_guest_fail(void)
{
  IVS_CHECK(name_is(IVS_RUNNER_OUTCOME_GUEST_FAIL, "guest-fail"));
  return IVS_TEST_PASS;
}

static int out_name_timeout(void)
{
  IVS_CHECK(name_is(IVS_RUNNER_OUTCOME_TIMEOUT_BUDGET, "timeout"));
  IVS_CHECK(name_is(IVS_RUNNER_OUTCOME_TIMEOUT_WALL, "timeout"));
  return IVS_TEST_PASS;
}

static int out_name_cancelled(void)
{
  IVS_CHECK(name_is(IVS_RUNNER_OUTCOME_CANCELLED, "cancelled"));
  return IVS_TEST_PASS;
}

static int out_name_missing_fixture(void)
{
  IVS_CHECK(name_is(IVS_RUNNER_OUTCOME_MISSING_FIXTURE, "missing-fixture"));
  return IVS_TEST_PASS;
}

static int out_name_harness_fail(void)
{
  IVS_CHECK(name_is(IVS_RUNNER_OUTCOME_HARNESS_FAIL, "harness-fail"));
  IVS_CHECK(name_is(1u, "harness-fail"));
  IVS_CHECK(name_is(0xFFFFFFFFu, "harness-fail"));
  return IVS_TEST_PASS;
}

static const ivs_test_case runner_cases[] = {
    {"runner.outcome.exit-cancelled", out_exit_cancelled, NULL},
    {"runner.outcome.exit-guest-fail", out_exit_guest_fail, NULL},
    {"runner.outcome.exit-harness-fail", out_exit_harness_fail, NULL},
    {"runner.outcome.exit-missing-fixture", out_exit_missing_fixture, NULL},
    {"runner.outcome.exit-pass", out_exit_pass, NULL},
    {"runner.outcome.exit-timeout", out_exit_timeout, NULL},
    {"runner.outcome.name-cancelled", out_name_cancelled, NULL},
    {"runner.outcome.name-guest-fail", out_name_guest_fail, NULL},
    {"runner.outcome.name-harness-fail", out_name_harness_fail, NULL},
    {"runner.outcome.name-missing-fixture", out_name_missing_fixture, NULL},
    {"runner.outcome.name-pass", out_name_pass, NULL},
    {"runner.outcome.name-timeout", out_name_timeout, NULL},
};

int main(int argc, char **argv)
{
  ivs_test_suite suite;
  ivs_test_suite_from_cases(&suite, "runner", runner_cases, sizeof runner_cases / sizeof runner_cases[0]);
  return ivs_test_main(argc, argv, &suite);
}
