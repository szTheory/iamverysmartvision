/* SPDX-License-Identifier: MIT */
/* Suite harness: self-test of the harness library (ivs_test.c) with no CPU code. Inner suites
 * run through ivs_test_run into tmpfile() streams; the captured text is compared exactly with
 * the format documented in ivs_test.h, and call counters show which cases ran. */
#include <stdio.h>
#include <string.h>

#include "ivs_test.h"

#define HS_BUF 2048u

static char hs_out[HS_BUF];
static char hs_tap[HS_BUF];
static unsigned hs_calls[4];

static int hs_pass_0(void)
{
  hs_calls[0]++;
  return IVS_TEST_PASS;
}

static int hs_pass_1(void)
{
  hs_calls[1]++;
  return IVS_TEST_PASS;
}

static int hs_pass_2(void)
{
  hs_calls[2]++;
  return IVS_TEST_PASS;
}

static int hs_fail_3(void)
{
  hs_calls[3]++;
  return IVS_TEST_FAIL;
}

static unsigned hs_total_calls(void)
{
  return hs_calls[0] + hs_calls[1] + hs_calls[2] + hs_calls[3];
}

static int hs_slurp(FILE *f, char *buf)
{
  size_t n;
  if (fflush(f) != 0) {
    return -1;
  }
  rewind(f);
  n = fread(buf, 1u, HS_BUF - 1u, f);
  buf[n] = '\0';
  return ferror(f) ? -1 : 0;
}

/* Runs s with opts o and nonce N1, capturing out into hs_out and tap into hs_tap; the
 * harness exit status goes to *rc. Returns 0, or -1 when a stream fails. */
static int hs_run(const ivs_test_suite *s, const ivs_test_opts *o, int *rc)
{
  FILE *out = tmpfile();
  FILE *tap = tmpfile();
  int ok = out != NULL && tap != NULL;
  memset(hs_calls, 0, sizeof hs_calls);
  hs_out[0] = '\0';
  hs_tap[0] = '\0';
  if (ok) {
    *rc = ivs_test_run(s, o, "N1", out, tap);
    ok = hs_slurp(out, hs_out) == 0 && hs_slurp(tap, hs_tap) == 0;
  }
  if (out != NULL) {
    fclose(out);
  }
  if (tap != NULL) {
    fclose(tap);
  }
  if (!ok) {
    ivs_test_diag("tmpfile stream failed");
  }
  return ok ? 0 : -1;
}

/* Prints text one diagnostic line per line, so captured inner TAP lines never reach stdout
 * as bare `ok`/`not ok` lines of the outer suite. */
static void hs_diag_lines(const char *label, const char *text)
{
  const char *p = text;
  while (*p != '\0') {
    const char *e = strchr(p, '\n');
    int len = e != NULL ? (int)(e - p) : (int)strlen(p);
    ivs_test_diag("  %s | %.*s", label, len, p);
    p = e != NULL ? e + 1 : p + len;
  }
}

static int hs_text(const char *what, const char *got, const char *want)
{
  if (strcmp(got, want) != 0) {
    ivs_test_diag("%s differs", what);
    hs_diag_lines("want", want);
    hs_diag_lines("got ", got);
    return 0;
  }
  return 1;
}

#define HS_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

#define HS_COUNT(cases) (sizeof(cases) / sizeof((cases)[0]))

/* A duplicated id is found before anything runs: the suite reports it, runs no case, writes
 * nothing to the result stream and exits 1, also in --list mode. */
static int hs_duplicate_id_rejected(void)
{
  static const ivs_test_case dup[] = {
      {"d.a", hs_pass_0, NULL},
      {"d.b", hs_pass_1, NULL},
      {"d.a", hs_pass_2, NULL},
  };
  static const ivs_test_case uniq[] = {
      {"d.a", hs_pass_0, NULL},
      {"d.b", hs_pass_1, NULL},
  };
  ivs_test_suite s;
  ivs_test_opts o = {NULL, 0};
  int rc = -1;
  ivs_test_suite_from_cases(&s, "dup", dup, HS_COUNT(dup));
  HS_CHECK(ivs_test_find_duplicate(&s) == 2u);
  HS_CHECK(hs_run(&s, &o, &rc) == 0);
  HS_CHECK(rc == 1);
  HS_CHECK(hs_text("out", hs_out, "# suite=dup duplicate id d.a: nothing run\n"));
  HS_CHECK(hs_text("tap", hs_tap, ""));
  HS_CHECK(hs_total_calls() == 0u);
  o.list = 1;
  HS_CHECK(hs_run(&s, &o, &rc) == 0);
  HS_CHECK(rc == 1);
  HS_CHECK(hs_text("list out", hs_out, "# suite=dup duplicate id d.a: nothing run\n"));
  ivs_test_suite_from_cases(&s, "uniq", uniq, HS_COUNT(uniq));
  HS_CHECK(ivs_test_find_duplicate(&s) == s.count);
  return IVS_TEST_PASS;
}

/* --only <prefix> selects by id prefix: the plan counts only selected cases, numbering is
 * over selected cases, unselected cases never run; --list honours the filter; an --only prefix
 * that matches no case exits 1, so a mistyped prefix cannot pass with nothing run (IN-06). Bad
 * arguments are usage errors. */
static int hs_only_filter(void)
{
  static const ivs_test_case cases[] = {
      {"f.a.one", hs_pass_0, NULL},
      {"f.b.one", hs_pass_1, NULL},
      {"f.a.two", hs_pass_2, NULL},
  };
  char a0[] = "prog";
  char a1[] = "--only";
  char a2[] = "f.a.";
  char a3[] = "--list";
  char a4[] = "--bogus";
  char *argv_only[] = {a0, a1, a2, NULL};
  char *argv_list[] = {a0, a3, a1, a2, NULL};
  char *argv_bogus[] = {a0, a4, NULL};
  char *argv_noval[] = {a0, a1, NULL};
  ivs_test_suite s;
  ivs_test_opts o;
  int rc = -1;
  ivs_test_suite_from_cases(&s, "flt", cases, HS_COUNT(cases));

  HS_CHECK(ivs_test_parse_args(3, argv_only, &o) == 0);
  HS_CHECK(o.only != NULL && strcmp(o.only, "f.a.") == 0 && o.list == 0);
  HS_CHECK(hs_run(&s, &o, &rc) == 0);
  HS_CHECK(rc == 0);
  HS_CHECK(hs_text("out", hs_out,
                   "# suite=flt nonce=N1\n1..2\nok 1 f.a.one\nok 2 f.a.two\n"
                   "# suite=flt executed=2 skipped=0 failed=0\n"));
  HS_CHECK(hs_text("tap", hs_tap, hs_out));
  HS_CHECK(hs_calls[0] == 1u && hs_calls[1] == 0u && hs_calls[2] == 1u);

  HS_CHECK(ivs_test_parse_args(4, argv_list, &o) == 0);
  HS_CHECK(o.list == 1);
  HS_CHECK(hs_run(&s, &o, &rc) == 0);
  HS_CHECK(rc == 0);
  HS_CHECK(hs_text("list out", hs_out, "f.a.one\nf.a.two\n"));
  HS_CHECK(hs_text("list tap", hs_tap, ""));
  HS_CHECK(hs_total_calls() == 0u);

  o.list = 0;
  o.only = "zzz";
  HS_CHECK(hs_run(&s, &o, &rc) == 0);
  HS_CHECK(rc == 1);
  HS_CHECK(hs_text("empty selection", hs_out, "# suite=flt nonce=N1\n1..0\n# suite=flt executed=0 skipped=0 failed=0\n"));

  HS_CHECK(ivs_test_parse_args(2, argv_bogus, &o) == -1);
  HS_CHECK(ivs_test_parse_args(2, argv_noval, &o) == -1);
  return IVS_TEST_PASS;
}

/* A skipped case prints `ok N <id> # SKIP <reason>` and never runs; a suite whose every
 * selected case is skipped exits 77; one run case among skips gives exit 0. */
static int hs_skip_line_format(void)
{
  static const ivs_test_case all_skip[] = {
      {"s.one", hs_pass_0, "C1"},
      {"s.two", hs_pass_1, "needs-profile"},
  };
  static const ivs_test_case mixed[] = {
      {"s.one", hs_pass_0, "C1"},
      {"s.two", hs_pass_1, NULL},
  };
  ivs_test_suite s;
  ivs_test_opts o = {NULL, 0};
  int rc = -1;
  ivs_test_suite_from_cases(&s, "skp", all_skip, HS_COUNT(all_skip));
  HS_CHECK(hs_run(&s, &o, &rc) == 0);
  HS_CHECK(rc == IVS_TEST_EXIT_SKIP);
  HS_CHECK(hs_text("out", hs_out,
                   "# suite=skp nonce=N1\n1..2\nok 1 s.one # SKIP C1\nok 2 s.two # SKIP needs-profile\n"
                   "# suite=skp executed=0 skipped=2 failed=0\n"));
  HS_CHECK(hs_text("tap", hs_tap, hs_out));
  HS_CHECK(hs_total_calls() == 0u);
  ivs_test_suite_from_cases(&s, "skp", mixed, HS_COUNT(mixed));
  HS_CHECK(hs_run(&s, &o, &rc) == 0);
  HS_CHECK(rc == 0);
  HS_CHECK(hs_text("mixed out", hs_out,
                   "# suite=skp nonce=N1\n1..2\nok 1 s.one # SKIP C1\nok 2 s.two\n"
                   "# suite=skp executed=1 skipped=1 failed=0\n"));
  HS_CHECK(hs_calls[0] == 0u && hs_calls[1] == 1u);
  return IVS_TEST_PASS;
}

/* The trailer counts executed, skipped and failed cases; a failure prints `not ok` and exits
 * 1; source lines follow the header; an empty suite without a filter exits 1. */
static int hs_trailer_counts(void)
{
  static const ivs_test_case cases[] = {
      {"t.a", hs_pass_0, NULL},
      {"t.b", hs_fail_3, NULL},
      {"t.c", hs_pass_1, "C2"},
      {"t.d", hs_pass_2, NULL},
  };
  static const ivs_test_source src[] = {{"data/x.json", "0123abcd"}};
  ivs_test_suite s;
  ivs_test_opts o = {NULL, 0};
  int rc = -1;
  ivs_test_suite_from_cases(&s, "cnt", cases, HS_COUNT(cases));
  s.sources = src;
  s.n_sources = 1u;
  HS_CHECK(hs_run(&s, &o, &rc) == 0);
  HS_CHECK(rc == 1);
  HS_CHECK(hs_text("out", hs_out,
                   "# suite=cnt nonce=N1\n# source data/x.json 0123abcd\n1..4\nok 1 t.a\nnot ok 2 t.b\n"
                   "ok 3 t.c # SKIP C2\nok 4 t.d\n# suite=cnt executed=3 skipped=1 failed=1\n"));
  HS_CHECK(hs_text("tap", hs_tap, hs_out));
  HS_CHECK(hs_calls[0] == 1u && hs_calls[1] == 0u && hs_calls[2] == 1u && hs_calls[3] == 1u);
  ivs_test_suite_from_cases(&s, "none", cases, 0u);
  HS_CHECK(hs_run(&s, &o, &rc) == 0);
  HS_CHECK(rc == 1);
  HS_CHECK(hs_text("empty out", hs_out, "# suite=none nonce=N1\n1..0\n# suite=none executed=0 skipped=0 failed=0\n"));
  return IVS_TEST_PASS;
}

static const ivs_test_case hs_cases[] = {
    {"harness.duplicate-id-rejected", hs_duplicate_id_rejected, NULL},
    {"harness.only-filter", hs_only_filter, NULL},
    {"harness.skip-line-format", hs_skip_line_format, NULL},
    {"harness.trailer-counts", hs_trailer_counts, NULL},
};

int main(int argc, char **argv)
{
  ivs_test_suite suite;
  ivs_test_suite_from_cases(&suite, "harness", hs_cases, HS_COUNT(hs_cases));
  return ivs_test_main(argc, argv, &suite);
}
