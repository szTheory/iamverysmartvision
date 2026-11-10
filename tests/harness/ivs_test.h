/* SPDX-License-Identifier: MIT */
/* In-repo test harness (D-17, D-20). A suite is an explicit, indexed case list (no
 * constructors). Output is TAP-like, to stdout and, when IVS_TEST_OUT_DIR is set, to
 * $IVS_TEST_OUT_DIR/<suite>.txt:
 *   # suite=<s> nonce=<IVS_TEST_NONCE or none>
 *   # source <path> <sha256>          (one per vector source, when the suite has any)
 *   1..N
 *   ok 1 <id> | not ok 2 <id> | ok 3 <id> # SKIP <reason>
 *   # suite=<s> executed=<n> skipped=<k> failed=<f>
 * Diagnostics (ivs_test_diag) go to stdout only, never into the result file.
 * Exit status: 1 on any failure or a duplicate id (checked before anything runs), 77 only
 * when every selected case is skipped, else 0. Flags: --only <prefix>, --list. */
#ifndef IVS_TEST_H
#define IVS_TEST_H

#include <stddef.h>
#include <stdio.h>

#define IVS_TEST_PASS 0
#define IVS_TEST_FAIL 1
#define IVS_TEST_EXIT_SKIP 77

typedef struct ivs_test_source {
  const char *path;
  const char *sha256;
} ivs_test_source;

typedef struct ivs_test_suite {
  const char *name;
  size_t count;
  const void *data;
  const char *(*id)(const void *data, size_t i);
  const char *(*skip)(const void *data, size_t i); /* reason, or NULL to run; may be NULL */
  int (*run)(const void *data, size_t i);          /* IVS_TEST_PASS or IVS_TEST_FAIL */
  const ivs_test_source *sources;
  size_t n_sources;
} ivs_test_suite;

/* Explicit case table for suites that are plain functions. */
typedef struct ivs_test_case {
  const char *id;
  int (*fn)(void);
  const char *skip; /* reason, or NULL */
} ivs_test_case;

typedef struct ivs_test_opts {
  const char *only; /* id prefix filter, or NULL */
  int list;
} ivs_test_opts;

/* Wrap a case table as a suite (data points at the table). */
void ivs_test_suite_from_cases(ivs_test_suite *s, const char *name, const ivs_test_case *cases, size_t n);

/* Parse --only <prefix> and --list. Returns 0, or -1 on a usage error. */
int ivs_test_parse_args(int argc, char **argv, ivs_test_opts *o);

/* Index of the second occurrence of a duplicated id, or s->count when all ids are unique. */
size_t ivs_test_find_duplicate(const ivs_test_suite *s);

/* Run the suite, writing results to out and, when tap is not NULL, to tap. Returns the exit
 * status (0, 1 or 77). */
int ivs_test_run(const ivs_test_suite *s, const ivs_test_opts *o, const char *nonce, FILE *out, FILE *tap);

/* main() body: parse args, read IVS_TEST_NONCE and IVS_TEST_OUT_DIR, run. */
int ivs_test_main(int argc, char **argv, const ivs_test_suite *s);

/* A diagnostic line ("# " prefix) on stdout. */
void ivs_test_diag(const char *fmt, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 1, 2)))
#endif
    ;

#endif
