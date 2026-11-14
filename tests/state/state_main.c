/* SPDX-License-Identifier: MIT */
/* Suite state (STA-01, STA-02, plans 08-11 to 08-15): concatenates the fixed cases of every family declared in state_cases.h into
 * one indexed suite. Ids are unique across the families (the harness rejects a duplicate before running anything). Its named faults
 * are data/expected/faults/state.ids; their sites are in core/src/machine. The TAP header carries one `# source <path> <sha256>`
 * line per oracle data file the families read (the literal file and the schema literal). */
#include <stdio.h>
#include <string.h>

#include "ivs_test.h"
#include "state_cases.h"

#define STATE_MAX_CASES 128u
#define STATE_MAX_SECTIONS 16u

typedef struct state_section {
  const ivs_test_case *fixed;
  size_t n_fixed;
} state_section;

static state_section state_sections[STATE_MAX_SECTIONS];
static size_t state_n_sections;
static size_t state_total;

static void state_add(const ivs_test_case *fixed, size_t n_fixed)
{
  state_section *s = &state_sections[state_n_sections++];
  s->fixed = fixed;
  s->n_fixed = n_fixed;
  state_total += n_fixed;
}

static const ivs_test_case *state_find(size_t i)
{
  size_t k;
  for (k = 0u; k < state_n_sections; k++) {
    const state_section *s = &state_sections[k];
    if (i < s->n_fixed) {
      return &s->fixed[i];
    }
    i -= s->n_fixed;
  }
  return NULL;
}

static const char *state_case_id(const void *data, size_t i)
{
  const ivs_test_case *c = state_find(i);
  (void)data;
  return c != NULL ? c->id : "";
}

static int state_case_run(const void *data, size_t i)
{
  const ivs_test_case *c = state_find(i);
  (void)data;
  return c != NULL ? c->fn() : IVS_TEST_FAIL;
}

int main(int argc, char **argv)
{
  ivs_test_suite suite;
  state_add(ivs_state_save_cases, ivs_state_save_count);
  state_add(ivs_state_literal_cases, ivs_state_literal_count);
  state_add(ivs_state_read_cases, ivs_state_read_count);
  state_add(ivs_state_restore_cases, ivs_state_restore_count);
  if (state_total > STATE_MAX_CASES) {
    fprintf(stderr, "state: more than %u cases; raise STATE_MAX_CASES\n", STATE_MAX_CASES);
    return 1;
  }
  memset(&suite, 0, sizeof suite);
  suite.name = "state";
  suite.count = state_total;
  suite.id = state_case_id;
  suite.run = state_case_run;
  suite.sources = ivs_state_sources(&suite.n_sources);
  return ivs_test_main(argc, argv, &suite);
}
