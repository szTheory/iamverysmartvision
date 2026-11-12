/* SPDX-License-Identifier: MIT */
/* Suite stic (VID-01, plan 06-06): concatenates the fixed cases of every family declared in stic_cases.h into one
 * indexed suite. Ids are unique across the families (the harness rejects a duplicate before running anything). Its
 * named faults are data/expected/faults/stic.ids; their sites are in core/src/stic and core/src/machine. The TAP
 * header carries one `# source <path> <sha256>` line per data file a family reads. */
#include <stdio.h>
#include <string.h>

#include "ivs_test.h"
#include "stic_cases.h"

#define STIC_MAX_CASES 256u
#define STIC_MAX_SECTIONS 16u

typedef struct stic_section {
  const ivs_test_case *fixed;
  size_t n_fixed;
} stic_section;

static stic_section stic_sections[STIC_MAX_SECTIONS];
static size_t stic_n_sections;
static size_t stic_total;

static void stic_add(const ivs_test_case *fixed, size_t n_fixed)
{
  stic_section *s = &stic_sections[stic_n_sections++];
  s->fixed = fixed;
  s->n_fixed = n_fixed;
  stic_total += n_fixed;
}

static const ivs_test_case *stic_find(size_t i)
{
  size_t k;
  for (k = 0u; k < stic_n_sections; k++) {
    const stic_section *s = &stic_sections[k];
    if (i < s->n_fixed) {
      return &s->fixed[i];
    }
    i -= s->n_fixed;
  }
  return NULL;
}

static const char *stic_case_id(const void *data, size_t i)
{
  const ivs_test_case *c = stic_find(i);
  (void)data;
  return c != NULL ? c->id : "";
}

static int stic_case_run(const void *data, size_t i)
{
  const ivs_test_case *c = stic_find(i);
  (void)data;
  return c != NULL ? c->fn() : IVS_TEST_FAIL;
}

int main(int argc, char **argv)
{
  ivs_test_suite suite;
  stic_add(ivs_stic_events_cases, ivs_stic_events_count);
  stic_add(ivs_stic_access_cases, ivs_stic_access_count);
  stic_add(ivs_stic_reg_cases, ivs_stic_reg_count);
  stic_add(ivs_stic_hook_cases, ivs_stic_hook_count);
  stic_add(ivs_stic_profile_cases, ivs_stic_profile_count);
  stic_add(ivs_stic_judge_cases, ivs_stic_judge_count);
  stic_add(ivs_stic_state_cases, ivs_stic_state_count);
  stic_add(ivs_stic_peek_cases, ivs_stic_peek_count);
  stic_add(ivs_stic_lines_cases, ivs_stic_lines_count);
  stic_add(ivs_stic_irq_cases, ivs_stic_irq_count);
  stic_add(ivs_stic_busrq_cases, ivs_stic_busrq_count);
  stic_add(ivs_stic_fetch_cases, ivs_stic_fetch_count);
  if (stic_total > STIC_MAX_CASES) {
    fprintf(stderr, "stic: more than %u cases; raise STIC_MAX_CASES\n", STIC_MAX_CASES);
    return 1;
  }
  memset(&suite, 0, sizeof suite);
  suite.name = "stic";
  suite.count = stic_total;
  suite.id = stic_case_id;
  suite.run = stic_case_run;
  suite.sources = sl_sources(&suite.n_sources);
  return ivs_test_main(argc, argv, &suite);
}
