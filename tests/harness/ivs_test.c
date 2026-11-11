/* SPDX-License-Identifier: MIT */
/* In-repo test harness; format and exit rules are in ivs_test.h (D-17, D-20). */
#include "ivs_test.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *ivs_case_id(const void *data, size_t i)
{
  return ((const ivs_test_case *)data)[i].id;
}

static const char *ivs_case_skip(const void *data, size_t i)
{
  return ((const ivs_test_case *)data)[i].skip;
}

static int ivs_case_run(const void *data, size_t i)
{
  return ((const ivs_test_case *)data)[i].fn();
}

void ivs_test_suite_from_cases(ivs_test_suite *s, const char *name, const ivs_test_case *cases, size_t n)
{
  memset(s, 0, sizeof *s);
  s->name = name;
  s->count = n;
  s->data = cases;
  s->id = ivs_case_id;
  s->skip = ivs_case_skip;
  s->run = ivs_case_run;
}

int ivs_test_parse_args(int argc, char **argv, ivs_test_opts *o)
{
  int i;
  o->only = NULL;
  o->list = 0;
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--list") == 0) {
      o->list = 1;
    } else if (strcmp(argv[i], "--only") == 0 && i + 1 < argc) {
      o->only = argv[++i];
    } else {
      return -1;
    }
  }
  return 0;
}

size_t ivs_test_find_duplicate(const ivs_test_suite *s)
{
  size_t i;
  size_t j;
  for (i = 1; i < s->count; i++) {
    const char *a = s->id(s->data, i);
    for (j = 0; j < i; j++) {
      if (strcmp(a, s->id(s->data, j)) == 0) {
        return i;
      }
    }
  }
  return s->count;
}

static int ivs_selected(const ivs_test_suite *s, const ivs_test_opts *o, size_t i)
{
  return o->only == NULL || strncmp(s->id(s->data, i), o->only, strlen(o->only)) == 0;
}

static void ivs_emit(FILE *out, FILE *tap, const char *fmt, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 3, 4)))
#endif
    ;

static void ivs_emit(FILE *out, FILE *tap, const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(out, fmt, ap);
  va_end(ap);
  if (tap != NULL) {
    va_start(ap, fmt);
    vfprintf(tap, fmt, ap);
    va_end(ap);
  }
}

void ivs_test_diag(const char *fmt, ...)
{
  va_list ap;
  fputs("# ", stdout);
  va_start(ap, fmt);
  vfprintf(stdout, fmt, ap);
  va_end(ap);
  fputc('\n', stdout);
}

int ivs_test_run(const ivs_test_suite *s, const ivs_test_opts *o, const char *nonce, FILE *out, FILE *tap)
{
  size_t i;
  size_t dup;
  size_t planned = 0;
  size_t num = 0;
  size_t executed = 0;
  size_t skipped = 0;
  size_t failed = 0;

  dup = ivs_test_find_duplicate(s);
  if (dup < s->count) {
    fprintf(out, "# suite=%s duplicate id %s: nothing run\n", s->name, s->id(s->data, dup));
    return 1;
  }
  if (o->list) {
    for (i = 0; i < s->count; i++) {
      if (ivs_selected(s, o, i)) {
        fprintf(out, "%s\n", s->id(s->data, i));
      }
    }
    return 0;
  }
  for (i = 0; i < s->count; i++) {
    if (ivs_selected(s, o, i)) {
      planned++;
    }
  }
  ivs_emit(out, tap, "# suite=%s nonce=%s\n", s->name, nonce);
  for (i = 0; i < s->n_sources; i++) {
    ivs_emit(out, tap, "# source %s %s\n", s->sources[i].path, s->sources[i].sha256);
  }
  ivs_emit(out, tap, "1..%zu\n", planned);
  for (i = 0; i < s->count; i++) {
    const char *id;
    const char *reason;
    if (!ivs_selected(s, o, i)) {
      continue;
    }
    num++;
    id = s->id(s->data, i);
    reason = s->skip != NULL ? s->skip(s->data, i) : NULL;
    if (reason != NULL) {
      skipped++;
      ivs_emit(out, tap, "ok %zu %s # SKIP %s\n", num, id, reason);
      continue;
    }
    executed++;
    fflush(out);
    if (s->run(s->data, i) == IVS_TEST_PASS) {
      ivs_emit(out, tap, "ok %zu %s\n", num, id);
    } else {
      failed++;
      ivs_emit(out, tap, "not ok %zu %s\n", num, id);
    }
  }
  ivs_emit(out, tap, "# suite=%s executed=%zu skipped=%zu failed=%zu\n", s->name, executed, skipped, failed);
  if (failed > 0) {
    return 1;
  }
  if (planned == 0) {
    return 1; /* nothing selected, a mistyped --only prefix included (review IN-06) */
  }
  if (skipped == planned) {
    return IVS_TEST_EXIT_SKIP;
  }
  return 0;
}

int ivs_test_main(int argc, char **argv, const ivs_test_suite *s)
{
  ivs_test_opts o;
  const char *nonce;
  const char *dir;
  FILE *tap = NULL;
  int rc;
  if (ivs_test_parse_args(argc, argv, &o) != 0) {
    fprintf(stderr, "usage: %s [--list] [--only <prefix>]\n", argc > 0 ? argv[0] : "test");
    return 1;
  }
  nonce = getenv("IVS_TEST_NONCE");
  if (nonce == NULL || nonce[0] == '\0') {
    nonce = "none";
  }
  dir = getenv("IVS_TEST_OUT_DIR");
  if (!o.list && dir != NULL && dir[0] != '\0') {
    char path[4096];
    int k = snprintf(path, sizeof path, "%s/%s.txt", dir, s->name);
    if (k < 0 || (size_t)k >= sizeof path) {
      fprintf(stderr, "# result path too long\n");
      return 1;
    }
    if (ivs_test_find_duplicate(s) < s->count) {
      return ivs_test_run(s, &o, nonce, stdout, NULL);
    }
    tap = fopen(path, "w");
    if (tap == NULL) {
      fprintf(stderr, "# cannot write %s\n", path);
      return 1;
    }
  }
  rc = ivs_test_run(s, &o, nonce, stdout, tap);
  if (tap != NULL && fclose(tap) != 0) {
    fprintf(stderr, "# cannot close the result file\n");
    rc = 1;
  }
  fflush(stdout);
  return rc;
}
