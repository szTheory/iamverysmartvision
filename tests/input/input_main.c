/* SPDX-License-Identifier: MIT */
/* Suite input (INP-01, INP-02, INP-03, API-03, plan 05-05): concatenates the cases of every family
 * declared in input_cases.h into one indexed suite. Ids are unique across the families (the harness
 * rejects a duplicate before running anything). Its named faults are data/expected/faults/input.ids;
 * their sites are in core/src/machine and core/src/memmap. */
#include <stdio.h>
#include <string.h>

#include "input_cases.h"

#define II_FAMILIES 1u
#define II_MAX_CASES 512u

typedef struct ii_family {
  const ivs_test_case *cases;
  size_t count;
} ii_family;

static ii_family ii_families[II_FAMILIES];

static const ivs_test_case *ii_case(size_t i)
{
  size_t f;
  for (f = 0u; f < II_FAMILIES; f++) {
    if (i < ii_families[f].count) {
      return &ii_families[f].cases[i];
    }
    i -= ii_families[f].count;
  }
  return NULL;
}

static const char *ii_case_id(const void *data, size_t i)
{
  const ivs_test_case *c = ii_case(i);
  (void)data;
  return c != NULL ? c->id : NULL;
}

static int ii_case_run(const void *data, size_t i)
{
  const ivs_test_case *c = ii_case(i);
  (void)data;
  return c != NULL ? c->fn() : IVS_TEST_FAIL;
}

int main(int argc, char **argv)
{
  ivs_test_suite suite;
  size_t total = 0u;
  size_t f;
  ii_families[0].cases = ivs_input_single_cases;
  ii_families[0].count = ivs_input_single_count;
  for (f = 0u; f < II_FAMILIES; f++) {
    total += ii_families[f].count;
  }
  if (total > II_MAX_CASES) {
    fprintf(stderr, "input: more than %u cases; raise II_MAX_CASES\n", II_MAX_CASES);
    return 1;
  }
  memset(&suite, 0, sizeof suite);
  suite.name = "input";
  suite.count = total;
  suite.id = ii_case_id;
  suite.run = ii_case_run;
  return ivs_test_main(argc, argv, &suite);
}
