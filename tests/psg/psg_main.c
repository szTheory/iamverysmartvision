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

/* Order: the fixed cases of family reg, its vectors (reg, reset), then family port (plan 05-05): its
 * fixed cases and its vectors. */
static size_t psg_n_reg(void)
{
  return ivs_psg_reg_count + ivs_psg_reg_vector_count();
}

static const char *psg_case_id(const void *data, size_t i)
{
  (void)data;
  if (i < ivs_psg_reg_count) {
    return ivs_psg_reg_cases[i].id;
  }
  if (i < psg_n_reg()) {
    return ivs_psg_reg_vector_id(i - ivs_psg_reg_count);
  }
  i -= psg_n_reg();
  if (i < ivs_psg_port_count) {
    return ivs_psg_port_cases[i].id;
  }
  return ivs_psg_port_vector_id(i - ivs_psg_port_count);
}

static int psg_case_run(const void *data, size_t i)
{
  (void)data;
  if (i < ivs_psg_reg_count) {
    return ivs_psg_reg_cases[i].fn();
  }
  if (i < psg_n_reg()) {
    return ivs_psg_reg_vector_run(i - ivs_psg_reg_count);
  }
  i -= psg_n_reg();
  if (i < ivs_psg_port_count) {
    return ivs_psg_port_cases[i].fn();
  }
  return ivs_psg_port_vector_run(i - ivs_psg_port_count);
}

int main(int argc, char **argv)
{
  ivs_test_suite suite;
  static ivs_test_source sources[8];
  size_t n_reg = 0u;
  size_t n_port = 0u;
  const ivs_test_source *reg_src = ivs_psg_reg_sources(&n_reg);
  const ivs_test_source *port_src = ivs_psg_port_sources(&n_port);
  size_t total = psg_n_reg() + ivs_psg_port_count + ivs_psg_port_vector_count();
  if (total > PSG_MAX_CASES) {
    fprintf(stderr, "psg: more than %u cases; raise PSG_MAX_CASES\n", PSG_MAX_CASES);
    return 1;
  }
  memset(&suite, 0, sizeof suite);
  suite.name = "psg";
  suite.count = total;
  suite.id = psg_case_id;
  suite.run = psg_case_run;
  if (n_reg + n_port > sizeof sources / sizeof sources[0]) {
    fprintf(stderr, "psg: more than %u sources\n", (unsigned)(sizeof sources / sizeof sources[0]));
    return 1;
  }
  memcpy(sources, reg_src, n_reg * sizeof sources[0]);
  memcpy(sources + n_reg, port_src, n_port * sizeof sources[0]);
  suite.sources = sources;
  suite.n_sources = n_reg + n_port;
  return ivs_test_main(argc, argv, &suite);
}
