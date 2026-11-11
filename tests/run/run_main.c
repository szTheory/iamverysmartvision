/* SPDX-License-Identifier: MIT */
/* Suite run (A-06, plan 04-06): concatenates every case family declared in run_cases.h into one
 * bounded case table and runs them as one suite. Ids are unique across families (the harness
 * rejects a duplicate before running anything). */
#include <stdio.h>
#include <string.h>

#include "ivs_test.h"
#include "run_cases.h"

#define IVS_RUN_MAX_CASES 256u

static ivs_test_case ivs_run_all[IVS_RUN_MAX_CASES];

static int ivs_run_add(size_t *n, const ivs_test_case *cases, size_t count)
{
  size_t i;
  if (count > IVS_RUN_MAX_CASES - *n) {
    fprintf(stderr, "run: more than %u cases; raise IVS_RUN_MAX_CASES\n", IVS_RUN_MAX_CASES);
    return -1;
  }
  for (i = 0; i < count; i++) {
    ivs_run_all[*n + i] = cases[i];
  }
  *n += count;
  return 0;
}

int main(int argc, char **argv)
{
  ivs_test_suite suite;
  size_t n = 0;
  if (ivs_run_add(&n, ivs_run_observe_cases, ivs_run_observe_count) != 0 ||
      ivs_run_add(&n, ivs_run_bound_cases, ivs_run_bound_count) != 0 ||
      ivs_run_add(&n, ivs_run_info_cases, ivs_run_info_count) != 0 ||
      ivs_run_add(&n, ivs_run_slice_cases, ivs_run_slice_count) != 0) {
    return 1;
  }
  ivs_test_suite_from_cases(&suite, "run", ivs_run_all, n);
  return ivs_test_main(argc, argv, &suite);
}
