/* SPDX-License-Identifier: MIT */
/* Suite input (INP-01, INP-02, INP-03, API-03, plan 05-05): concatenates the cases of every family
 * declared in input_cases.h into one indexed suite. Ids are unique across the families (the harness
 * rejects a duplicate before running anything). Its named faults are data/expected/faults/input.ids;
 * their sites are in core/src/machine and core/src/memmap. The TAP header carries one `# source <path>
 * <sha256>` line for the controller literal. */
#include <stdio.h>
#include <string.h>

#include "input_cases.h"

#define II_MAX_CASES 512u

static const ii_family *const ii_families[] = {&ivs_input_fam_single, &ivs_input_fam_combo, &ivs_input_fam_port,
                                                 &ivs_input_fam_slice};
#define II_FAMILIES (sizeof ii_families / sizeof ii_families[0])

/* Family and index inside it of case i. */
static const ii_family *ii_locate(size_t i, size_t *j)
{
  size_t f;
  for (f = 0u; f < II_FAMILIES; f++) {
    size_t n = ii_families[f]->count();
    if (i < n) {
      *j = i;
      return ii_families[f];
    }
    i -= n;
  }
  return NULL;
}

static const char *ii_case_id(const void *data, size_t i)
{
  size_t j = 0u;
  const ii_family *fam = ii_locate(i, &j);
  (void)data;
  return fam != NULL ? fam->id(j) : NULL;
}

static int ii_case_run(const void *data, size_t i)
{
  size_t j = 0u;
  const ii_family *fam = ii_locate(i, &j);
  (void)data;
  return fam != NULL ? fam->run(j) : IVS_TEST_FAIL;
}

int main(int argc, char **argv)
{
  ivs_test_suite suite;
  size_t total = 0u;
  size_t f;
  for (f = 0u; f < II_FAMILIES; f++) {
    total += ii_families[f]->count();
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
  suite.sources = ivs_input_sources(&suite.n_sources);
  return ivs_test_main(argc, argv, &suite);
}
