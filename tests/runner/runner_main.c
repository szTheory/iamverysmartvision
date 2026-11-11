/* SPDX-License-Identifier: MIT */
/* Suite runner: the runner library in process (D-17, D-20, D-23, plan 04-03). Family outcome:
 * the exit code and report name of each of the six outcomes (both timeout kinds share code 11
 * and the name timeout), distinct codes, and the fixed precedence of
 * ivs_runner_outcome_resolve. Families parse and fuzz (runner_parse.c, runner_fuzz.c, plan
 * 04-17): the IVSS v1 scenario parser. Family exec (runner_exec.c, plan 04-18): the executor and
 * report writer through a fake host. Named faults: data/expected/faults/runner.ids. */
#include <stdint.h>
#include <string.h>

#include "ivs_runner.h"
#include "ivs_test.h"
#include "runner_cases.h"

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

/* ---- distinct codes (D-20): six outcomes, six codes, none 1 (ASan), 77 (skip) or >125 ---- */

static int out_codes_distinct(void)
{
  static const uint32_t outcomes[] = {
      IVS_RUNNER_OUTCOME_PASS,      IVS_RUNNER_OUTCOME_GUEST_FAIL,      IVS_RUNNER_OUTCOME_TIMEOUT_BUDGET,
      IVS_RUNNER_OUTCOME_CANCELLED, IVS_RUNNER_OUTCOME_MISSING_FIXTURE, IVS_RUNNER_OUTCOME_HARNESS_FAIL,
  };
  uint32_t codes[sizeof outcomes / sizeof outcomes[0]];
  size_t n = sizeof outcomes / sizeof outcomes[0];
  for (size_t i = 0; i < n; i++) {
    codes[i] = ivs_runner_exit_code(outcomes[i]);
    IVS_CHECK(codes[i] != 1u && codes[i] != 77u && codes[i] <= 125u);
  }
  IVS_CHECK(ivs_runner_exit_code(IVS_RUNNER_OUTCOME_TIMEOUT_WALL) == codes[2]);
  for (size_t i = 0; i < n; i++) {
    for (size_t j = i + 1u; j < n; j++) {
      if (codes[i] == codes[j]) {
        ivs_test_diag("outcomes %zu and %zu share exit code %u", i, j, (unsigned)codes[i]);
        return IVS_TEST_FAIL;
      }
    }
  }
  return IVS_TEST_PASS;
}

/* ---- precedence (D-20): harness-fail > missing-fixture > cancelled > timeout(wall) >
 * timeout(budget) > guest-fail > pass; one case per adjacent pair ---- */

static int out_prec_harness_over_missing(void)
{
  IVS_CHECK(ivs_runner_outcome_resolve(IVS_RUNNER_OBS_HARNESS_FAIL | IVS_RUNNER_OBS_MISSING_FIXTURE) ==
            IVS_RUNNER_OUTCOME_HARNESS_FAIL);
  return IVS_TEST_PASS;
}

static int out_prec_missing_over_cancelled(void)
{
  IVS_CHECK(ivs_runner_outcome_resolve(IVS_RUNNER_OBS_MISSING_FIXTURE | IVS_RUNNER_OBS_CANCELLED) ==
            IVS_RUNNER_OUTCOME_MISSING_FIXTURE);
  return IVS_TEST_PASS;
}

static int out_prec_cancelled_over_wall(void)
{
  IVS_CHECK(ivs_runner_outcome_resolve(IVS_RUNNER_OBS_CANCELLED | IVS_RUNNER_OBS_TIMEOUT_WALL) ==
            IVS_RUNNER_OUTCOME_CANCELLED);
  return IVS_TEST_PASS;
}

static int out_prec_wall_over_budget(void)
{
  IVS_CHECK(ivs_runner_outcome_resolve(IVS_RUNNER_OBS_TIMEOUT_WALL | IVS_RUNNER_OBS_TIMEOUT_BUDGET) ==
            IVS_RUNNER_OUTCOME_TIMEOUT_WALL);
  return IVS_TEST_PASS;
}

static int out_prec_budget_over_guest(void)
{
  IVS_CHECK(ivs_runner_outcome_resolve(IVS_RUNNER_OBS_TIMEOUT_BUDGET | IVS_RUNNER_OBS_GUEST_FAIL) ==
            IVS_RUNNER_OUTCOME_TIMEOUT_BUDGET);
  return IVS_TEST_PASS;
}

static int out_prec_guest_over_pass(void)
{
  IVS_CHECK(ivs_runner_outcome_resolve(IVS_RUNNER_OBS_GUEST_FAIL) == IVS_RUNNER_OUTCOME_GUEST_FAIL);
  return IVS_TEST_PASS;
}

static int out_prec_empty_is_pass(void)
{
  IVS_CHECK(ivs_runner_outcome_resolve(0u) == IVS_RUNNER_OUTCOME_PASS);
  return IVS_TEST_PASS;
}

/* Each observation alone resolves to its own outcome; every mask with the harness bit, or with
 * a bit outside IVS_RUNNER_OBS_ALL, is harness-fail. */
static int out_prec_single_and_unknown(void)
{
  IVS_CHECK(ivs_runner_outcome_resolve(IVS_RUNNER_OBS_TIMEOUT_BUDGET) == IVS_RUNNER_OUTCOME_TIMEOUT_BUDGET);
  IVS_CHECK(ivs_runner_outcome_resolve(IVS_RUNNER_OBS_TIMEOUT_WALL) == IVS_RUNNER_OUTCOME_TIMEOUT_WALL);
  IVS_CHECK(ivs_runner_outcome_resolve(IVS_RUNNER_OBS_CANCELLED) == IVS_RUNNER_OUTCOME_CANCELLED);
  IVS_CHECK(ivs_runner_outcome_resolve(IVS_RUNNER_OBS_MISSING_FIXTURE) == IVS_RUNNER_OUTCOME_MISSING_FIXTURE);
  for (uint32_t m = 0u; m <= IVS_RUNNER_OBS_ALL; m++) {
    if ((m & IVS_RUNNER_OBS_HARNESS_FAIL) != 0u) {
      IVS_CHECK(ivs_runner_outcome_resolve(m) == IVS_RUNNER_OUTCOME_HARNESS_FAIL);
    }
  }
  IVS_CHECK(ivs_runner_outcome_resolve(IVS_RUNNER_OBS_ALL + 1u) == IVS_RUNNER_OUTCOME_HARNESS_FAIL);
  IVS_CHECK(ivs_runner_outcome_resolve(0x80000000u | IVS_RUNNER_OBS_GUEST_FAIL) == IVS_RUNNER_OUTCOME_HARNESS_FAIL);
  return IVS_TEST_PASS;
}

static const ivs_test_case runner_cases[] = {
    {"runner.outcome.codes-distinct", out_codes_distinct, NULL},
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
    {"runner.outcome.prec-budget-over-guest", out_prec_budget_over_guest, NULL},
    {"runner.outcome.prec-cancelled-over-wall", out_prec_cancelled_over_wall, NULL},
    {"runner.outcome.prec-empty-is-pass", out_prec_empty_is_pass, NULL},
    {"runner.outcome.prec-guest-over-pass", out_prec_guest_over_pass, NULL},
    {"runner.outcome.prec-harness-over-missing", out_prec_harness_over_missing, NULL},
    {"runner.outcome.prec-missing-over-cancelled", out_prec_missing_over_cancelled, NULL},
    {"runner.outcome.prec-single-and-unknown", out_prec_single_and_unknown, NULL},
    {"runner.outcome.prec-wall-over-budget", out_prec_wall_over_budget, NULL},
};

#define OUTCOME_CASES (sizeof runner_cases / sizeof runner_cases[0])

/* Suite index i: the outcome family, then the parse, fuzz and exec families. */
static const char *runner_case_id(const void *data, size_t i)
{
  (void)data;
  if (i < OUTCOME_CASES) {
    return runner_cases[i].id;
  }
  i -= OUTCOME_CASES;
  if (i < runner_parse_count()) {
    return runner_parse_id(i);
  }
  i -= runner_parse_count();
  if (i < runner_fuzz_count()) {
    return runner_fuzz_id(i);
  }
  return runner_exec_id(i - runner_fuzz_count());
}

static int runner_case_run(const void *data, size_t i)
{
  (void)data;
  if (i < OUTCOME_CASES) {
    return runner_cases[i].fn();
  }
  i -= OUTCOME_CASES;
  if (i < runner_parse_count()) {
    return runner_parse_run(i);
  }
  i -= runner_parse_count();
  if (i < runner_fuzz_count()) {
    return runner_fuzz_run(i);
  }
  return runner_exec_run(i - runner_fuzz_count());
}

int main(int argc, char **argv)
{
  ivs_test_suite suite;
  memset(&suite, 0, sizeof suite);
  suite.name = "runner";
  suite.count = OUTCOME_CASES + runner_parse_count() + runner_fuzz_count() + runner_exec_count();
  suite.id = runner_case_id;
  suite.run = runner_case_run;
  return ivs_test_main(argc, argv, &suite);
}
