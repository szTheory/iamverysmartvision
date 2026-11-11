/* SPDX-License-Identifier: MIT */
/* Suite machine (D-15): concatenates every case family declared in machine_cases.h into one
 * bounded case table, followed by the indexed refuse family (plan 03-11), and runs them as one
 * suite. Ids are unique across families (the harness rejects a duplicate before running
 * anything). */
#include <stdio.h>
#include <string.h>

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

static size_t ivs_machine_n_cases;

/* Index i: the case table first, then the refuse family. */
static const char *ivs_machine_id(const void *data, size_t i)
{
  (void)data;
  return i < ivs_machine_n_cases ? ivs_machine_all[i].id : ivs_machine_refuse_id(i - ivs_machine_n_cases);
}

static const char *ivs_machine_skip(const void *data, size_t i)
{
  (void)data;
  return i < ivs_machine_n_cases ? ivs_machine_all[i].skip : ivs_machine_refuse_skip(i - ivs_machine_n_cases);
}

static int ivs_machine_run(const void *data, size_t i)
{
  (void)data;
  return i < ivs_machine_n_cases ? ivs_machine_all[i].fn() : ivs_machine_refuse_run(i - ivs_machine_n_cases);
}

int main(int argc, char **argv)
{
  ivs_test_suite suite;
  size_t n = 0;
  size_t ns = 0;
  if (ivs_machine_add(&n, ivs_machine_sha256_cases, ivs_machine_sha256_count) != 0 ||
      ivs_machine_add_sources(&ns, ivs_machine_sha256_sources, ivs_machine_sha256_source_count) != 0 ||
      ivs_machine_add(&n, ivs_machine_create_cases, ivs_machine_create_count) != 0 ||
      ivs_machine_add(&n, ivs_machine_digest_cases, ivs_machine_digest_count) != 0 ||
      ivs_machine_add(&n, ivs_machine_load_cases, ivs_machine_load_count) != 0 ||
      ivs_machine_add_sources(&ns, ivs_machine_load_sources, ivs_machine_load_source_count) != 0 ||
      ivs_machine_add(&n, ivs_machine_reset_cases, ivs_machine_reset_count) != 0) {
    return 1;
  }
  ivs_machine_n_cases = n;
  memset(&suite, 0, sizeof suite);
  suite.name = "machine";
  suite.count = n + ivs_machine_refuse_count();
  suite.id = ivs_machine_id;
  suite.skip = ivs_machine_skip;
  suite.run = ivs_machine_run;
  suite.sources = ivs_machine_all_sources;
  suite.n_sources = ns;
  return ivs_test_main(argc, argv, &suite);
}
