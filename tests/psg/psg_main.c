/* SPDX-License-Identifier: MIT */
/* Suite psg (AUD-01, plan 05-04): concatenates the fixed cases of every family declared in
 * psg_cases.h and the vector cases of the generated tables into one indexed suite. Ids are unique
 * across the families (the harness rejects a duplicate before running anything). Its named faults
 * are data/expected/faults/psg.ids; their sites are in core/src/psg. The TAP header carries one
 * `# source <path> <sha256>` line per data file (vector files and the register-address literal). */
#include <stdio.h>
#include <string.h>

#include "ivs_test.h"
#include "psg_cases.h"

#define PSG_MAX_CASES 256u

static const char *psg_case_id(const void *data, size_t i)
{
  (void)data;
  if (i < ivs_psg_reg_count) {
    return ivs_psg_reg_cases[i].id;
  }
  return ivs_psg_reg_vector_id(i - ivs_psg_reg_count);
}

static int psg_case_run(const void *data, size_t i)
{
  (void)data;
  if (i < ivs_psg_reg_count) {
    return ivs_psg_reg_cases[i].fn();
  }
  return ivs_psg_reg_vector_run(i - ivs_psg_reg_count);
}

int main(int argc, char **argv)
{
  ivs_test_suite suite;
  size_t total = ivs_psg_reg_count + ivs_psg_reg_vector_count();
  if (total > PSG_MAX_CASES) {
    fprintf(stderr, "psg: more than %u cases; raise PSG_MAX_CASES\n", PSG_MAX_CASES);
    return 1;
  }
  memset(&suite, 0, sizeof suite);
  suite.name = "psg";
  suite.count = total;
  suite.id = psg_case_id;
  suite.run = psg_case_run;
  suite.sources = ivs_psg_reg_sources(&suite.n_sources);
  return ivs_test_main(argc, argv, &suite);
}
