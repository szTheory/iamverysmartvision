/* SPDX-License-Identifier: MIT */
/* Suite machine (D-15): concatenates every case family declared in machine_cases.h into one
 * bounded case table and runs it. Ids are unique across families (the harness rejects a
 * duplicate before running anything). */
#include <stdio.h>

#include "ivs_test.h"
#include "machine_cases.h"

#define IVS_MACHINE_MAX_CASES 512u
#define IVS_MACHINE_MAX_SOURCES 8u

static ivs_test_case ivs_machine_all[IVS_MACHINE_MAX_CASES];
static ivs_test_source ivs_machine_all_sources[IVS_MACHINE_MAX_SOURCES];

static int ivs_machine_add(size_t *n, const ivs_test_case *cases, size_t count)
{
  size_t i;
  if (count > IVS_MACHINE_MAX_CASES - *n) {
    fprintf(stderr, "machine: more than %u cases; raise IVS_MACHINE_MAX_CASES\n", IVS_MACHINE_MAX_CASES);
    return -1;
  }
  for (i = 0; i < count; i++) {
    ivs_machine_all[*n + i] = cases[i];
  }
  *n += count;
  return 0;
}

static int ivs_machine_add_sources(size_t *n, const ivs_test_source *sources, size_t count)
{
  size_t i;
  if (count > IVS_MACHINE_MAX_SOURCES - *n) {
    fprintf(stderr, "machine: more than %u sources; raise IVS_MACHINE_MAX_SOURCES\n", IVS_MACHINE_MAX_SOURCES);
    return -1;
  }
  for (i = 0; i < count; i++) {
    ivs_machine_all_sources[*n + i] = sources[i];
  }
  *n += count;
  return 0;
}

int main(int argc, char **argv)
{
  ivs_test_suite suite;
  size_t n = 0;
  size_t ns = 0;
  if (ivs_machine_add(&n, ivs_machine_sha256_cases, ivs_machine_sha256_count) != 0 ||
      ivs_machine_add_sources(&ns, ivs_machine_sha256_sources, ivs_machine_sha256_source_count) != 0 ||
      ivs_machine_add(&n, ivs_machine_create_cases, ivs_machine_create_count) != 0) {
    return 1;
  }
  ivs_test_suite_from_cases(&suite, "machine", ivs_machine_all, n);
  suite.sources = ivs_machine_all_sources;
  suite.n_sources = ns;
  return ivs_test_main(argc, argv, &suite);
}
