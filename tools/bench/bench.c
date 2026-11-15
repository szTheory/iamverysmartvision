/* SPDX-License-Identifier: MIT */
/* ivs-bench: the benchmark tool (plan 08-18, D-21 to D-26, D-39). ISO C only: the three host calls that reach a clock or the machine
 * (bench_now_ns, bench_cpu_ns, bench_ncpu) are in bench_host.c, the one POSIX file, and the only calls of the two time functions are
 * inside the timed loop below. The tool links ivs_runner_lib and ivs_core and calls ivs_runner_execute in process; it is never
 * installed and never run by the gate except with --verify-only, which reads no clock.
 *
 * Order of operations per workload (D-22): (1) iteration 0 is an untimed verification run: the report is captured in memory, filtered
 * line by line with ivs_runner_golden_keep (the key list of the golden writer, the live-domain flags derived from the report counts
 * as runner/main.c derives them) and compared with the committed data/golden/<scenario>.digests; the final machine time must also
 * equal the `ticks` column of the workload table. Any difference is a refusal: one line `bench-refuse <workload> <reason> <detail>`
 * on stderr, a row with status=refused and every numeric column empty (an empty field cannot be mistaken for a measurement), and
 * exit 15 at the end; no clock is read for that workload. (2) With --verify-only the row says status=verified with every
 * throughput and latency column empty. (3) Otherwise W untimed warmup iterations and N timed iterations, each timed iteration
 * bracketed by two clock reads around ivs_runner_execute alone; the outcome, the digest.composite line and the step end ticks of every
 * iteration are compared with iteration 0 after the timed span (a difference is a refusal and no row of measurements). W and N are
 * fixed per workload in the table, never a time budget. Statistics: the median of the per-iteration wall times (the mean of the two
 * middle values for an even N), their maximum, the first timed iteration, and the CPU time of the whole timed loop divided by N.
 * A row with a non-zero roundtrip_ticks (the snapshot workload, plan 08-19, D-39) passes that cadence to every execution through
 * IVS_RUNNER_OPT_ROUNDTRIP: its golden is the one of its scenario, so a save, reset and restore that changed the report is refused
 * like any other mismatch, and a run in which the hook made no cut is a harness failure.
 *
 * Output (D-24): TSV on stdout, the line `bench-format 1`, one header line, one row per workload; three groups that never merge:
 * provenance and static, throughput, latency, then the pointer column ci_time (CI time is never measured here). The tier column is
 * `repeatability-only`: a figure of this tool is never a claim and never evidence.
 * Exit codes: 0 every workload verified or measured; 14 usage error or harness failure (one line on stderr, and for a usage error
 * or an unreadable input no row at all); 15 at least one workload refused (the runner's own codes are 10 to 14). */
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench_host.h"
#include "ivs/ivs.h"
#include "ivs_runner.h"

#ifndef IVS_BENCH_BUILD_ID
#error "IVS_BENCH_BUILD_ID is set by tools/bench/CMakeLists.txt (compiler id and version, build type, flags digest)"
#endif
#ifndef IVS_BENCH_OPTIMIZED
#error "IVS_BENCH_OPTIMIZED is set by tools/bench/CMakeLists.txt (1 for a build type that optimizes)"
#endif
#ifndef IVS_BENCH_SANITIZERS
#error "IVS_BENCH_SANITIZERS is set by tools/bench/CMakeLists.txt (1 when the preset builds with sanitizers)"
#endif

#define BENCH_EXIT_HARNESS 14
#define BENCH_EXIT_REFUSED 15
#define BENCH_MAX_WORKLOADS 16u
#define BENCH_PATH_MAX 4096u
#define BENCH_TABLE_MAX (64u * 1024u)
#define BENCH_GOLDEN_MAX (512u * 1024u)
#define BENCH_REPORT_CAP (512u * 1024u)
#define BENCH_SIG_MAX 8192u
#define BENCH_WARMUP_MAX 1000u
#define BENCH_ITER_MAX 1000u
#define BENCH_COLUMNS 22u
#define BENCH_TIER "repeatability-only"
#define BENCH_CI_POINTER "see-ci-runs.tsv"
#define BENCH_FORMAT_LINE "bench-format 1"
#define BENCH_HEADER                                                                                                                 \
  "workload\tstatus\tscenario\ttier\tbuild_id\toptimized\tsanitizers\tncpu\twarmup\titerations\tticks_per_iter\tcpu_cycles_per_iter\t" \
  "frames_per_iter\tthr_cycles_per_s_wall\tthr_cycles_per_s_cpu\tthr_frames_per_s\trealtime_x\tns_per_guest_cycle\tlat_first_ms\t"    \
  "lat_median_ms\tlat_max_ms\tci_time"

/* Results of the internal functions: a refused workload (its row is printed) or a harness failure (one line is printed). */
#define BENCH_OK 0
#define BENCH_REFUSED 1
#define BENCH_FAIL (-1)

static const char bench_usage[] =
    "usage: ivs-bench --content-dir <dir> [options]\n"
    "Verifies each workload of the table against its committed golden, then (without --verify-only) times it, and prints TSV on stdout.\n"
    "options:\n"
    "  --content-dir <dir>  directory of the images the scenarios name (required: the image directory of a build tree)\n"
    "  --root <dir>         repository root (default: the current directory); data/bench/workloads.tsv, data/scenarios and\n"
    "                       data/golden resolve from it\n"
    "  --workloads <file>   workload table (default: <root>/data/bench/workloads.tsv)\n"
    "  --golden-dir <dir>   directory of the .digests goldens (default: <root>/data/golden)\n"
    "  --workload <id>      run only this workload; repeatable (default: every workload of the table)\n"
    "  --verify-only        iteration 0 only: no warmup, no timed iteration, no clock; rows say status=verified\n"
    "  --help               print this text and exit 0\n"
    "output: the line `bench-format 1`, a header line and one row per workload, tab separated, in the groups provenance and static,\n"
    "throughput, latency and the pointer column ci_time; the tier is repeatability-only (a figure is never a claim).\n"
    "exit codes: 0 verified or measured; 14 usage error or harness failure; 15 a workload was refused (its report differs from\n"
    "its golden): `bench-refuse <workload> <reason> <detail>` on stderr and a row with status=refused and no number.\n";

/* One line of data/bench/workloads.tsv with the parsed inputs it owns. */
typedef struct bench_workload {
  char id[IVS_SCN_MAX_ID + 1u];
  char scenario[IVS_SCN_MAX_ID + 1u];
  uint32_t warmup;
  uint32_t iterations;
  uint64_t roundtrip_ticks;
  uint64_t ticks;
  int selected;
  ivs_scenario *scn;
  char *golden_body; /* the golden without its `#` lines */
  size_t golden_len;
  size_t storage_bytes;
} bench_workload;

typedef struct bench_ctx {
  const char *content_dir;
  char *report;
  size_t report_len;
  int overflow;
} bench_ctx;

/* The values of one output row; a column whose have flag is 0 is printed empty. */
typedef struct bench_row {
  const char *workload;
  const char *status;
  const char *scenario;
  int have_static;
  uint32_t ncpu;
  uint32_t warmup;
  uint32_t iterations;
  uint64_t ticks;
  uint64_t cycles;
  int have_frames;
  uint64_t frames;
  int have_thr_wall;
  double thr_cycles_wall;
  double ns_per_cycle;
  int have_thr_cpu;
  double thr_cycles_cpu;
  int have_thr_frames;
  double thr_frames;
  double realtime_x;
  int have_lat;
  double lat_first_ms;
  double lat_median_ms;
  double lat_max_ms;
} bench_row;

/* ---- small helpers ---- */

static int bench_err(const char *who, const char *what)
{
  if (who != NULL) {
    fprintf(stderr, "ivs-bench: %s: %s\n", who, what);
  } else {
    fprintf(stderr, "ivs-bench: %s\n", what);
  }
  return BENCH_FAIL;
}

/* Decimal digits only, lo..hi; 0 on anything else (sign, blank, overflow, out of range). */
static int bench_number(const char *s, size_t n, uint64_t lo, uint64_t hi, uint64_t *out)
{
  uint64_t v = 0u;
  if (n == 0u) {
    return 0;
  }
  for (size_t i = 0u; i < n; i++) {
    uint64_t dgt;
    if (s[i] < '0' || s[i] > '9') {
      return 0;
    }
    dgt = (uint64_t)(s[i] - '0');
    if (v > (UINT64_MAX - dgt) / 10u) {
      return 0;
    }
    v = v * 10u + dgt;
  }
  if (v < lo || v > hi) {
    return 0;
  }
  *out = v;
  return 1;
}

/* [a-z0-9-]{1,48}, the id grammar of the scenario parser. */
static int bench_id_ok(const char *s)
{
  size_t n = strlen(s);
  if (n == 0u || n > IVS_SCN_MAX_ID) {
    return 0;
  }
  for (size_t i = 0u; i < n; i++) {
    if (!((s[i] >= 'a' && s[i] <= 'z') || (s[i] >= '0' && s[i] <= '9') || s[i] == '-')) {
      return 0;
    }
  }
  return 1;
}

/* Reads at most cap + 1 bytes of the regular file at path (plain stdio). One of the IVS_RUNNER_READ_* results. */
static int bench_read_all(const char *path, size_t cap, uint8_t *buf, size_t *len)
{
  FILE *f;
  size_t got;
  *len = 0u;
  f = fopen(path, "rb");
  if (f == NULL) {
    return IVS_RUNNER_READ_MISSING;
  }
  got = fread(buf, 1u, cap + 1u, f);
  if (ferror(f)) {
    (void)fclose(f);
    return IVS_RUNNER_READ_IO_ERROR;
  }
  if (fclose(f) != 0) {
    return IVS_RUNNER_READ_IO_ERROR;
  }
  *len = got;
  return got > cap ? IVS_RUNNER_READ_TOO_LARGE : IVS_RUNNER_READ_OK;
}

/* Joins two path parts; 0 on success, nonzero when the result does not fit. */
static int bench_join(char *out, size_t cap, const char *a, const char *b, const char *c)
{
  int n = snprintf(out, cap, "%s%s%s", a, b, c);
  return (n < 0 || (size_t)n >= cap) ? -1 : 0;
}

/* ---- the host vtable of the runner ---- */

static int bh_read_file(void *ctx, const char *relpath, size_t cap, uint8_t *buf, size_t *len)
{
  const bench_ctx *c = ctx;
  char path[BENCH_PATH_MAX];
  *len = 0u;
  if (bench_join(path, sizeof path, c->content_dir, "/", relpath) != 0) {
    return IVS_RUNNER_READ_IO_ERROR;
  }
  return bench_read_all(path, cap, buf, len);
}

static int bh_write_report(void *ctx, const char *bytes, size_t len)
{
  bench_ctx *c = ctx;
  if (len > BENCH_REPORT_CAP - c->report_len) {
    c->overflow = 1;
    return -1;
  }
  memcpy(c->report + c->report_len, bytes, len);
  c->report_len += len;
  return 0;
}

/* ---- report text (the small helpers of runner/main.c, copied: they are static there) ---- */

/* 1 when the line of length n has the key k (the text before the first space). */
static int bench_key_is(const char *line, size_t n, const char *k)
{
  size_t kl = strlen(k);
  return n > kl && line[kl] == ' ' && memcmp(line, k, kl) == 0;
}

/* The value of key k in the report text, "-" when the key is absent. Points into buf; *vn gets its length. */
static const char *bench_report_value(const char *buf, size_t len, const char *k, size_t *vn)
{
  size_t i = 0u;
  while (i < len) {
    const char *nl = memchr(buf + i, '\n', len - i);
    size_t n = nl != NULL ? (size_t)(nl - (buf + i)) : len - i;
    if (bench_key_is(buf + i, n, k)) {
      size_t kl = strlen(k) + 1u;
      *vn = n - kl;
      return buf + i + kl;
    }
    i += n + 1u;
  }
  *vn = 1u;
  return "-";
}

/* A live domain is observed when its count is a decimal number other than 0. */
static int bench_count_observed(const char *v, size_t n)
{
  int nonzero = 0;
  if (n == 0u) {
    return 0;
  }
  for (size_t i = 0u; i < n; i++) {
    if (v[i] < '0' || v[i] > '9') {
      return 0;
    }
    if (v[i] != '0') {
      nonzero = 1;
    }
  }
  return nonzero;
}

/* Filters the report to the golden key list into out (capacity cap); *outn gets the length. 0, or -1 when the report is not
 * complete (empty, no final LF) or the result does not fit. */
static int bench_filter(const char *rep, size_t len, char *out, size_t cap, size_t *outn)
{
  static const char *const live_count[4] = {"output.video.bytes", "output.audio.bytes", "input.events", "output.psglog.bytes"};
  int live[4];
  size_t i = 0u;
  size_t o = 0u;
  if (len == 0u || rep[len - 1u] != '\n') {
    return -1;
  }
  for (size_t k = 0u; k < 4u; k++) {
    size_t vn;
    const char *v = bench_report_value(rep, len, live_count[k], &vn);
    live[k] = bench_count_observed(v, vn);
  }
  while (i < len) {
    const char *nl = memchr(rep + i, '\n', len - i);
    size_t n = (size_t)(nl - (rep + i)) + 1u;
    if (ivs_runner_golden_keep(rep + i, n - 1u, live)) {
      if (n > cap - o) {
        return -1;
      }
      memcpy(out + o, rep + i, n);
      o += n;
    }
    i += n;
  }
  *outn = o;
  return 0;
}

/* The key of a line (the text before the first space, or the whole line) into key (capacity cap). */
static void bench_key_of(const char *line, size_t n, char *key, size_t cap)
{
  size_t k = 0u;
  while (k < n && line[k] != ' ') {
    k++;
  }
  if (k > cap - 1u) {
    k = cap - 1u;
  }
  memcpy(key, line, k);
  key[k] = '\0';
}

/* Compares two LF-ended line texts. 0 when equal; otherwise 1 with the key of the first line that differs (the line of a, or of b
 * when a has no line there) in key. */
static int bench_diff(const char *a, size_t an, const char *b, size_t bn, char *key, size_t cap)
{
  size_t i = 0u;
  size_t j = 0u;
  while (i < an || j < bn) {
    const char *nla = i < an ? memchr(a + i, '\n', an - i) : NULL;
    const char *nlb = j < bn ? memchr(b + j, '\n', bn - j) : NULL;
    size_t na = i < an ? (nla != NULL ? (size_t)(nla - (a + i)) : an - i) : 0u;
    size_t nb = j < bn ? (nlb != NULL ? (size_t)(nlb - (b + j)) : bn - j) : 0u;
    if (!(i < an && j < bn && na == nb && memcmp(a + i, b + j, na) == 0)) {
      if (i < an) {
        bench_key_of(a + i, na, key, cap);
      } else {
        bench_key_of(b + j, nb, key, cap);
      }
      return 1;
    }
    i += na + 1u;
    j += nb + 1u;
  }
  return 0;
}

/* 1 when the line is one of the lines a timed iteration re-checks: outcome, digest.composite and step.<k>.now_ticks. */
static int bench_sig_line(const char *l, size_t n)
{
  size_t i;
  if (bench_key_is(l, n, "outcome") || bench_key_is(l, n, "digest.composite")) {
    return 1;
  }
  if (n > 5u && memcmp(l, "step.", 5u) == 0) {
    i = 5u;
    while (i < n && l[i] >= '0' && l[i] <= '9') {
      i++;
    }
    return i > 5u && n > i + 11u && memcmp(l + i, ".now_ticks ", 11u) == 0;
  }
  return 0;
}

/* Collects the re-checked lines of a report (each with its LF) into out. 0, or -1 when they do not fit. */
static int bench_sig(const char *rep, size_t len, char *out, size_t cap, size_t *outn)
{
  size_t i = 0u;
  size_t o = 0u;
  while (i < len) {
    const char *nl = memchr(rep + i, '\n', len - i);
    size_t n = nl != NULL ? (size_t)(nl - (rep + i)) : len - i;
    if (bench_sig_line(rep + i, n)) {
      if (n + 1u > cap - o) {
        return -1;
      }
      memcpy(out + o, rep + i, n);
      out[o + n] = '\n';
      o += n + 1u;
    }
    i += n + 1u;
  }
  *outn = o;
  return 0;
}

/* The value of the last step.<k>.now_ticks line of a signature. 0 and *ticks, or -1 when there is none. */
static int bench_last_ticks(const char *sig, size_t len, uint64_t *ticks)
{
  size_t i = 0u;
  int found = -1;
  while (i < len) {
    const char *nl = memchr(sig + i, '\n', len - i);
    size_t n = nl != NULL ? (size_t)(nl - (sig + i)) : len - i;
    if (n > 5u && memcmp(sig + i, "step.", 5u) == 0) {
      size_t sp = 0u;
      while (sp < n && sig[i + sp] != ' ') {
        sp++;
      }
      if (sp < n && bench_number(sig + i + sp + 1u, n - sp - 1u, 0u, UINT64_MAX, ticks)) {
        found = 0;
      }
    }
    i += n + 1u;
  }
  return found;
}

/* ---- the workload table ---- */

/* Parses the table text (len bytes, NUL at text[len]) into w. 0, or BENCH_FAIL after one line on stderr. */
static int bench_parse_table(char *text, size_t len, const char *path, bench_workload *w, size_t *nw)
{
  size_t i = 0u;
  unsigned line_no = 0u;
  char what[160];
  *nw = 0u;
  if (memchr(text, '\0', len) != NULL) {
    return bench_err(path, "the table holds a NUL byte");
  }
  text[len] = '\0';
  while (i < len) {
    char *line = text + i;
    char *nl = memchr(line, '\n', len - i);
    size_t n = nl != NULL ? (size_t)(nl - line) : len - i;
    char *f[7];
    size_t nf = 0u;
    char *p = line;
    uint64_t v;
    bench_workload *cur;
    line_no++;
    i += n + 1u;
    line[n] = '\0';
    if (n == 0u || line[0] == '#') {
      continue;
    }
    while (nf < 7u) {
      f[nf++] = p;
      if (nf == 7u) {
        break;
      }
      p = strchr(p, '\t');
      if (p == NULL) {
        break;
      }
      *p++ = '\0';
    }
    if (nf < 6u) {
      (void)snprintf(what, sizeof what, "line %u: fewer than 6 tab-separated columns", line_no);
      return bench_err(path, what);
    }
    if (*nw >= BENCH_MAX_WORKLOADS) {
      return bench_err(path, "more than 16 workloads");
    }
    cur = &w[*nw];
    memset(cur, 0, sizeof *cur);
    if (!bench_id_ok(f[0]) || !bench_id_ok(f[1])) {
      (void)snprintf(what, sizeof what, "line %u: workload and scenario must be [a-z0-9-]{1,48}", line_no);
      return bench_err(path, what);
    }
    memcpy(cur->id, f[0], strlen(f[0]) + 1u);
    memcpy(cur->scenario, f[1], strlen(f[1]) + 1u);
    for (size_t k = 0u; k < *nw; k++) {
      if (strcmp(w[k].id, cur->id) == 0) {
        (void)snprintf(what, sizeof what, "line %u: workload %s appears twice", line_no, cur->id);
        return bench_err(path, what);
      }
    }
    if (!bench_number(f[2], strlen(f[2]), 0u, BENCH_WARMUP_MAX, &v)) {
      (void)snprintf(what, sizeof what, "line %u: warmup must be decimal digits, 0 to %u", line_no, BENCH_WARMUP_MAX);
      return bench_err(path, what);
    }
    cur->warmup = (uint32_t)v;
    if (!bench_number(f[3], strlen(f[3]), 1u, BENCH_ITER_MAX, &v)) {
      (void)snprintf(what, sizeof what, "line %u: iterations must be decimal digits, 1 to %u", line_no, BENCH_ITER_MAX);
      return bench_err(path, what);
    }
    cur->iterations = (uint32_t)v;
    if (!bench_number(f[4], strlen(f[4]), 0u, IVS_RUN_TARGET_MAX, &cur->roundtrip_ticks)) {
      (void)snprintf(what, sizeof what, "line %u: roundtrip_ticks must be decimal digits", line_no);
      return bench_err(path, what);
    }
    if (!bench_number(f[5], strlen(f[5]), 1u, IVS_SCN_TIME_MAX, &cur->ticks)) {
      (void)snprintf(what, sizeof what, "line %u: ticks must be decimal digits, at least 1", line_no);
      return bench_err(path, what);
    }
    (*nw)++;
  }
  if (*nw == 0u) {
    return bench_err(path, "the table holds no workload");
  }
  return BENCH_OK;
}

/* ---- loading a workload (everything that can be a usage or harness failure, before anything is printed) ---- */

static void bench_free_workload(bench_workload *w)
{
  free(w->scn);
  free(w->golden_body);
  w->scn = NULL;
  w->golden_body = NULL;
}

/* Reads and parses the scenario, reads the golden and keeps its body, and asks the core for the storage of the profile. */
static int bench_load_workload(const char *root, const char *golden_dir, bench_workload *w)
{
  char path[BENCH_PATH_MAX];
  uint8_t *text = NULL;
  char *gold = NULL;
  size_t len = 0u;
  size_t o = 0u;
  size_t i = 0u;
  int r;
  int rc = BENCH_FAIL;
  ivs_scenario_error err;
  ivs_storage_requirements req;
  char line[128];
  text = malloc((size_t)IVS_SCN_MAX_BYTES + 1u);
  w->scn = malloc(sizeof *w->scn);
  gold = malloc((size_t)BENCH_GOLDEN_MAX + 1u);
  w->golden_body = malloc((size_t)BENCH_GOLDEN_MAX + 1u);
  if (text == NULL || w->scn == NULL || gold == NULL || w->golden_body == NULL) {
    (void)bench_err(w->id, "out of memory");
    goto done;
  }
  if (bench_join(path, sizeof path, root, "/data/scenarios/", w->scenario) != 0 || strlen(path) + 6u >= sizeof path) {
    (void)bench_err(w->id, "scenario path too long");
    goto done;
  }
  memcpy(path + strlen(path), ".ivss", 6u);
  r = bench_read_all(path, IVS_SCN_MAX_BYTES, text, &len);
  if (r != IVS_RUNNER_READ_OK && r != IVS_RUNNER_READ_TOO_LARGE) {
    (void)bench_err(w->id, r == IVS_RUNNER_READ_MISSING ? "scenario file is missing" : "scenario file is not readable");
    goto done;
  }
  memset(&err, 0, sizeof err);
  if (ivs_scenario_parse(text, len, w->scn, &err) != 0) {
    if (ivs_scenario_error_format(&err, line, sizeof line) != 0) {
      (void)snprintf(line, sizeof line, "scenario-syntax");
    }
    (void)bench_err(w->id, line);
    goto done;
  }
  if (strcmp(w->scn->id, w->scenario) != 0) {
    (void)bench_err(w->id, "the scenario id differs from the scenario column");
    goto done;
  }
  if (bench_join(path, sizeof path, golden_dir, "/", w->scenario) != 0 || strlen(path) + 9u >= sizeof path) {
    (void)bench_err(w->id, "golden path too long");
    goto done;
  }
  memcpy(path + strlen(path), ".digests", 9u);
  r = bench_read_all(path, BENCH_GOLDEN_MAX, (uint8_t *)gold, &len);
  if (r != IVS_RUNNER_READ_OK) {
    (void)bench_err(w->id, r == IVS_RUNNER_READ_MISSING ? "golden file is missing" : "golden file is not readable");
    goto done;
  }
  while (i < len) {
    const char *nl = memchr(gold + i, '\n', len - i);
    size_t n = nl != NULL ? (size_t)(nl - (gold + i)) : len - i;
    if (n > 0u && gold[i] != '#') {
      memcpy(w->golden_body + o, gold + i, n);
      w->golden_body[o + n] = '\n';
      o += n + 1u;
    }
    i += n + 1u;
  }
  if (o == 0u) {
    (void)bench_err(w->id, "the golden holds no body line");
    goto done;
  }
  w->golden_len = o;
  memset(&req, 0, sizeof req);
  req.struct_size = (uint32_t)sizeof req;
  if (ivs_machine_storage_requirements(w->scn->profile, &req) != IVS_OK || req.size_bytes > IVS_RUNNER_STORAGE_MAX ||
      req.align_bytes == 0u || req.align_bytes > IVS_RUNNER_STORAGE_MAX) {
    (void)bench_err(w->id, "the profile of the scenario is refused by the core");
    goto done;
  }
  w->storage_bytes = (size_t)req.size_bytes + (size_t)req.align_bytes + (size_t)IVS_RUNNER_CAP_CART + 1u +
                     (size_t)IVS_RUNNER_CAPTURE_BYTES;
  rc = BENCH_OK;
done:
  free(text);
  free(gold);
  if (rc != BENCH_OK) {
    bench_free_workload(w);
  }
  return rc;
}

/* ---- output ---- */

static void bench_fmt_u64(char *out, size_t cap, int have, uint64_t v)
{
  if (have) {
    (void)snprintf(out, cap, "%" PRIu64, v);
  } else {
    out[0] = '\0';
  }
}

static void bench_fmt_f(char *out, size_t cap, int have, int prec, double v)
{
  if (have) {
    (void)snprintf(out, cap, "%.*f", prec, v);
  } else {
    out[0] = '\0';
  }
}

static void bench_print_row(const bench_row *r)
{
  char buf[BENCH_COLUMNS][32];
  const char *f[BENCH_COLUMNS];
  const int st = r->have_static;
  size_t c = 0u;
  f[c++] = r->workload;
  f[c++] = r->status;
  f[c++] = r->scenario;
  f[c++] = BENCH_TIER;
  f[c++] = IVS_BENCH_BUILD_ID;
  f[c++] = IVS_BENCH_OPTIMIZED ? "1" : "0";
  f[c++] = IVS_BENCH_SANITIZERS ? "on" : "off";
  bench_fmt_u64(buf[c], sizeof buf[c], st && r->ncpu != 0u, r->ncpu);
  f[c] = buf[c];
  c++;
  bench_fmt_u64(buf[c], sizeof buf[c], st, r->warmup);
  f[c] = buf[c];
  c++;
  bench_fmt_u64(buf[c], sizeof buf[c], st, r->iterations);
  f[c] = buf[c];
  c++;
  bench_fmt_u64(buf[c], sizeof buf[c], st, r->ticks);
  f[c] = buf[c];
  c++;
  bench_fmt_u64(buf[c], sizeof buf[c], st, r->cycles);
  f[c] = buf[c];
  c++;
  bench_fmt_u64(buf[c], sizeof buf[c], st && r->have_frames, r->frames);
  f[c] = buf[c];
  c++;
  bench_fmt_f(buf[c], sizeof buf[c], r->have_thr_wall, 0, r->thr_cycles_wall);
  f[c] = buf[c];
  c++;
  bench_fmt_f(buf[c], sizeof buf[c], r->have_thr_cpu, 0, r->thr_cycles_cpu);
  f[c] = buf[c];
  c++;
  bench_fmt_f(buf[c], sizeof buf[c], r->have_thr_frames, 2, r->thr_frames);
  f[c] = buf[c];
  c++;
  bench_fmt_f(buf[c], sizeof buf[c], r->have_thr_wall, 3, r->realtime_x);
  f[c] = buf[c];
  c++;
  bench_fmt_f(buf[c], sizeof buf[c], r->have_thr_wall, 2, r->ns_per_cycle);
  f[c] = buf[c];
  c++;
  bench_fmt_f(buf[c], sizeof buf[c], r->have_lat, 3, r->lat_first_ms);
  f[c] = buf[c];
  c++;
  bench_fmt_f(buf[c], sizeof buf[c], r->have_lat, 3, r->lat_median_ms);
  f[c] = buf[c];
  c++;
  bench_fmt_f(buf[c], sizeof buf[c], r->have_lat, 3, r->lat_max_ms);
  f[c] = buf[c];
  c++;
  f[c++] = BENCH_CI_POINTER;
  for (size_t k = 0u; k < c; k++) {
    fputs(f[k], stdout);
    fputc(k + 1u == c ? '\n' : '\t', stdout);
  }
  (void)fflush(stdout);
}

/* The refusal: one line on stderr and the row without a number. */
static int bench_refuse(const bench_workload *w, const char *reason, const char *detail)
{
  bench_row row;
  memset(&row, 0, sizeof row);
  row.workload = w->id;
  row.status = "refused";
  row.scenario = w->scenario;
  fprintf(stderr, "bench-refuse %s %s %s\n", w->id, reason, detail);
  bench_print_row(&row);
  return BENCH_REFUSED;
}

/* ---- statistics ---- */

static void bench_sort_u64(uint64_t *v, size_t n)
{
  for (size_t i = 1u; i < n; i++) {
    uint64_t x = v[i];
    size_t j = i;
    while (j > 0u && v[j - 1u] > x) {
      v[j] = v[j - 1u];
      j--;
    }
    v[j] = x;
  }
}

/* ---- one workload ---- */

/* The timebase of the profile through the public API: a throwaway instance in the storage block, read with ivs_machine_info and
 * scrubbed again. 0, or -1. */
static int bench_timebase(const bench_workload *w, void *storage, size_t storage_bytes, ivs_info *info)
{
  ivs_storage_requirements req;
  ivs_create_options co;
  ivs_machine *m = NULL;
  uintptr_t base;
  uintptr_t aligned;
  ivs_status st;
  memset(&req, 0, sizeof req);
  req.struct_size = (uint32_t)sizeof req;
  if (ivs_machine_storage_requirements(w->scn->profile, &req) != IVS_OK) {
    return -1;
  }
  base = (uintptr_t)storage;
  aligned = (base + (uintptr_t)(req.align_bytes - 1u)) & ~(uintptr_t)(req.align_bytes - 1u);
  if ((size_t)(aligned - base) > storage_bytes || storage_bytes - (size_t)(aligned - base) < (size_t)req.size_bytes) {
    return -1;
  }
  memset(&co, 0, sizeof co);
  co.struct_size = (uint32_t)sizeof co;
  co.profile_id = w->scn->profile;
  if (ivs_machine_create(&co, (void *)aligned, (size_t)req.size_bytes, &m) != IVS_OK) {
    return -1;
  }
  memset(info, 0, sizeof *info);
  info->struct_size = (uint32_t)sizeof *info;
  st = ivs_machine_info(m, info);
  (void)ivs_machine_destroy(m);
  return st == IVS_OK && info->ticks_per_cpu_cycle != 0u && info->crystal_hz_num != 0u && info->crystal_hz_den != 0u ? 0 : -1;
}

/* One execution: the report goes to c->report. Returns the outcome. */
static uint32_t bench_execute(const bench_workload *w, bench_ctx *c, const ivs_runner_host *host, const ivs_runner_options *opts,
                              ivs_runner_result *res)
{
  c->report_len = 0u;
  c->overflow = 0;
  return ivs_runner_execute(w->scn, host, opts, res);
}

/* Compares the re-checked lines of the last execution with those of iteration 0. 0 when equal; otherwise 1 with the key. */
static int bench_recheck(const bench_ctx *c, uint32_t outcome, uint32_t outcome0, const char *sig0, size_t sig0_len, char *key,
                         size_t kcap)
{
  char sig[BENCH_SIG_MAX];
  size_t n = 0u;
  if (c->overflow || bench_sig(c->report, c->report_len, sig, sizeof sig, &n) != 0) {
    (void)snprintf(key, kcap, "report");
    return 1;
  }
  if (outcome != outcome0) {
    (void)snprintf(key, kcap, "outcome");
    return 1;
  }
  return bench_diff(sig, n, sig0, sig0_len, key, kcap);
}

/* Runs one workload and prints its row. BENCH_OK, BENCH_REFUSED (row printed) or BENCH_FAIL (a harness failure, one line). */
static int bench_run(const char *content_dir, int verify_only, const bench_workload *w)
{
  bench_ctx c;
  ivs_runner_host host;
  ivs_runner_options opts;
  ivs_runner_result res;
  ivs_info info;
  bench_row row;
  void *storage = NULL;
  uint8_t *buffers = NULL;
  char *filt = NULL;
  char sig0[BENCH_SIG_MAX];
  char key[96];
  char detail[160];
  uint64_t wall[BENCH_ITER_MAX];
  size_t sig0_len = 0u;
  size_t filt_len = 0u;
  uint64_t ticks = 0u;
  uint64_t cycles;
  uint32_t outcome0;
  uint32_t outcome;
  int rc = BENCH_FAIL;
  memset(&c, 0, sizeof c);
  memset(&opts, 0, sizeof opts);
  if (w->roundtrip_ticks != 0u) {
    /* The snapshot workload (plan 08-19, D-39): the same options on every execution of the workload, iteration 0, warmup and timed
     * iterations alike, so the round trip (save, reset, restore at the cadence) is part of what is verified and of what is timed. */
    opts.flags = IVS_RUNNER_OPT_ROUNDTRIP;
    opts.roundtrip_ticks = w->roundtrip_ticks;
  }
  c.content_dir = content_dir;
  c.report = malloc(BENCH_REPORT_CAP);
  filt = malloc(BENCH_REPORT_CAP);
  storage = malloc(w->storage_bytes);
  buffers = malloc((size_t)IVS_RUNNER_BUFFERS_MAX);
  if (c.report == NULL || filt == NULL || storage == NULL || buffers == NULL) {
    (void)bench_err(w->id, "out of memory");
    goto done;
  }
  memset(&host, 0, sizeof host);
  host.ctx = &c;
  host.read_file = bh_read_file;
  host.write_report = bh_write_report;
  host.storage = storage;
  host.storage_bytes = w->storage_bytes;
  host.buffers = buffers;
  host.buffer_bytes = (size_t)IVS_RUNNER_BUFFERS_MAX;
  if (bench_timebase(w, storage, w->storage_bytes, &info) != 0) {
    (void)bench_err(w->id, "cannot read the timebase of the profile");
    goto done;
  }

  /* Iteration 0: the untimed verification run. No clock is read before it has matched the golden. */
  outcome0 = bench_execute(w, &c, &host, &opts, &res);
  if (c.overflow || res.report_write_failed != 0u) {
    (void)bench_err(w->id, "the report did not fit the capture buffer");
    goto done;
  }
  if (bench_filter(c.report, c.report_len, filt, BENCH_REPORT_CAP, &filt_len) != 0) {
    (void)bench_err(w->id, "the report is incomplete");
    goto done;
  }
  if (bench_diff(filt, filt_len, w->golden_body, w->golden_len, key, sizeof key) != 0) {
    rc = bench_refuse(w, "digest-mismatch", key);
    goto done;
  }
  if (bench_sig(c.report, c.report_len, sig0, sizeof sig0, &sig0_len) != 0 || bench_last_ticks(sig0, sig0_len, &ticks) != 0) {
    (void)bench_err(w->id, "the report holds no step end ticks");
    goto done;
  }
  if (ticks != w->ticks) {
    (void)snprintf(detail, sizeof detail, "table=%" PRIu64 " run=%" PRIu64, w->ticks, ticks);
    rc = bench_refuse(w, "ticks-mismatch", detail);
    goto done;
  }
  if (w->roundtrip_ticks != 0u && res.roundtrip_cuts == 0u) {
    /* A cadence beyond the run would leave a snapshot workload that measures nothing about snapshots: never a silent no-op. */
    (void)bench_err(w->id, "the round-trip hook made no cut (roundtrip_ticks is beyond the run, or the hook was not requested)");
    goto done;
  }

  memset(&row, 0, sizeof row);
  row.workload = w->id;
  row.status = verify_only ? "verified" : "measured";
  row.scenario = w->scenario;
  row.have_static = 1;
  row.ncpu = bench_ncpu();
  row.warmup = w->warmup;
  row.iterations = w->iterations;
  row.ticks = ticks;
  cycles = ticks / info.ticks_per_cpu_cycle;
  row.cycles = cycles;
  {
    size_t vn;
    const char *v = bench_report_value(c.report, c.report_len, "output.video.frames", &vn);
    row.have_frames = bench_number(v, vn, 0u, UINT64_MAX, &row.frames);
  }
  if (verify_only) {
    bench_print_row(&row);
    rc = BENCH_OK;
    goto done;
  }

  /* Warmup: untimed, but still checked. */
  for (uint32_t i = 0u; i < w->warmup; i++) {
    outcome = bench_execute(w, &c, &host, &opts, &res);
    if (bench_recheck(&c, outcome, outcome0, sig0, sig0_len, key, sizeof key) != 0) {
      (void)snprintf(detail, sizeof detail, "warmup=%" PRIu32 " %s", i + 1u, key);
      rc = bench_refuse(w, "run-mismatch", detail);
      goto done;
    }
  }

  /* The timed loop: the two clock reads bracket ivs_runner_execute alone; the re-check is outside the span. */
  {
    uint64_t cpu0 = 0u;
    uint64_t cpu1 = 0u;
    uint64_t t0;
    uint64_t t1;
    uint64_t first;
    uint64_t median;
    uint64_t cpu_total;
    int have_cpu = bench_cpu_ns(&cpu0) == 0;
    for (uint32_t i = 0u; i < w->iterations; i++) {
      c.report_len = 0u;
      c.overflow = 0;
      if (bench_now_ns(&t0) != 0) {
        (void)bench_err(w->id, "the monotonic clock is unavailable");
        goto done;
      }
      outcome = ivs_runner_execute(w->scn, &host, &opts, &res);
      if (bench_now_ns(&t1) != 0) {
        (void)bench_err(w->id, "the monotonic clock is unavailable");
        goto done;
      }
      wall[i] = t1 >= t0 ? t1 - t0 : 0u;
      if (bench_recheck(&c, outcome, outcome0, sig0, sig0_len, key, sizeof key) != 0) {
        (void)snprintf(detail, sizeof detail, "iteration=%" PRIu32 " %s", i + 1u, key);
        rc = bench_refuse(w, "run-mismatch", detail);
        goto done;
      }
    }
    have_cpu = have_cpu && bench_cpu_ns(&cpu1) == 0 && cpu1 >= cpu0;
    cpu_total = have_cpu ? cpu1 - cpu0 : 0u;
    first = wall[0];
    bench_sort_u64(wall, w->iterations);
    median = (w->iterations % 2u == 1u) ? wall[w->iterations / 2u]
                                        : (wall[w->iterations / 2u - 1u] + wall[w->iterations / 2u]) / 2u;
    row.have_lat = 1;
    row.lat_first_ms = (double)first / 1e6;
    row.lat_median_ms = (double)median / 1e6;
    row.lat_max_ms = (double)wall[w->iterations - 1u] / 1e6;
    if (median > 0u) {
      double med_s = (double)median / 1e9;
      double guest_s = (double)ticks * (double)info.crystal_hz_den / (double)info.crystal_hz_num;
      row.have_thr_wall = 1;
      row.thr_cycles_wall = (double)cycles / med_s;
      row.ns_per_cycle = cycles > 0u ? (double)median / (double)cycles : 0.0;
      row.realtime_x = guest_s / med_s;
      if (row.have_frames) {
        row.have_thr_frames = 1;
        row.thr_frames = (double)row.frames / med_s;
      }
    }
    if (cpu_total > 0u) {
      double cpu_s = ((double)cpu_total / (double)w->iterations) / 1e9;
      row.have_thr_cpu = 1;
      row.thr_cycles_cpu = (double)cycles / cpu_s;
    }
  }
  bench_print_row(&row);
  rc = BENCH_OK;
done:
  free(c.report);
  free(filt);
  free(storage);
  free(buffers);
  return rc;
}

/* ---- options ---- */

typedef struct bench_args {
  const char *root;
  const char *content_dir;
  const char *workloads;
  const char *golden_dir;
  const char *pick[BENCH_MAX_WORKLOADS];
  size_t npick;
  int verify_only;
} bench_args;

/* One line on stderr, the usage exit. */
static int bench_usage_error(const char *what, const char *arg)
{
  if (arg != NULL) {
    fprintf(stderr, "ivs-bench: %s: %s (see --help)\n", what, arg);
  } else {
    fprintf(stderr, "ivs-bench: %s (see --help)\n", what);
  }
  return BENCH_EXIT_HARNESS;
}

/* Parses argv into a. Returns -1 to go on, or the exit code to leave with. */
static int bench_parse_args(int argc, char **argv, bench_args *a)
{
  for (int i = 1; i < argc; i++) {
    const char *arg = argv[i];
    const char **slot = NULL;
    if (strcmp(arg, "--help") == 0) {
      fputs(bench_usage, stdout);
      return fflush(stdout) == 0 ? 0 : BENCH_EXIT_HARNESS;
    }
    if (strcmp(arg, "--verify-only") == 0) {
      if (a->verify_only) {
        return bench_usage_error("option given twice", arg);
      }
      a->verify_only = 1;
      continue;
    }
    if (strcmp(arg, "--root") == 0) {
      slot = &a->root;
    } else if (strcmp(arg, "--content-dir") == 0) {
      slot = &a->content_dir;
    } else if (strcmp(arg, "--workloads") == 0) {
      slot = &a->workloads;
    } else if (strcmp(arg, "--golden-dir") == 0) {
      slot = &a->golden_dir;
    } else if (strcmp(arg, "--workload") != 0) {
      return bench_usage_error(arg[0] == '-' ? "unknown option" : "unexpected argument", arg);
    }
    if (i + 1 >= argc || argv[i + 1][0] == '\0' || (argv[i + 1][0] == '-' && argv[i + 1][1] == '-')) {
      return bench_usage_error("option without a value", arg);
    }
    i++;
    if (slot != NULL) {
      if (*slot != NULL) {
        return bench_usage_error("option given twice", arg);
      }
      *slot = argv[i];
      continue;
    }
    if (a->npick >= BENCH_MAX_WORKLOADS) {
      return bench_usage_error("too many --workload options", NULL);
    }
    for (size_t k = 0u; k < a->npick; k++) {
      if (strcmp(a->pick[k], argv[i]) == 0) {
        return bench_usage_error("workload given twice", argv[i]);
      }
    }
    a->pick[a->npick++] = argv[i];
  }
  if (a->content_dir == NULL) {
    return bench_usage_error("missing --content-dir", NULL);
  }
  return -1;
}

int main(int argc, char **argv)
{
  bench_args a;
  bench_workload w[BENCH_MAX_WORKLOADS];
  char table_path[BENCH_PATH_MAX];
  char golden_dir[BENCH_PATH_MAX];
  char *text = NULL;
  size_t text_len = 0u;
  size_t nw = 0u;
  size_t loaded = 0u;
  int code;
  int refused = 0;
  int r;
  memset(&a, 0, sizeof a);
  memset(w, 0, sizeof w);
  code = bench_parse_args(argc, argv, &a);
  if (code >= 0) {
    return code;
  }
  if (a.root == NULL) {
    a.root = ".";
  }
  if (a.workloads == NULL) {
    if (bench_join(table_path, sizeof table_path, a.root, "/data/bench/workloads.tsv", "") != 0) {
      (void)bench_err(NULL, "path too long");
      return BENCH_EXIT_HARNESS;
    }
    a.workloads = table_path;
  }
  if (a.golden_dir == NULL) {
    if (bench_join(golden_dir, sizeof golden_dir, a.root, "/data/golden", "") != 0) {
      (void)bench_err(NULL, "path too long");
      return BENCH_EXIT_HARNESS;
    }
    a.golden_dir = golden_dir;
  }

  /* Phase 1: everything that can fail before a byte is printed: the table, the selection, the scenarios, the goldens, the profiles. */
  text = malloc((size_t)BENCH_TABLE_MAX + 2u);
  if (text == NULL) {
    (void)bench_err(NULL, "out of memory");
    return BENCH_EXIT_HARNESS;
  }
  r = bench_read_all(a.workloads, BENCH_TABLE_MAX, (uint8_t *)text, &text_len);
  if (r != IVS_RUNNER_READ_OK) {
    free(text);
    (void)bench_err(a.workloads, r == IVS_RUNNER_READ_MISSING ? "the workload table is missing" : "the workload table is not readable");
    return BENCH_EXIT_HARNESS;
  }
  r = bench_parse_table(text, text_len, a.workloads, w, &nw);
  free(text);
  if (r != BENCH_OK) {
    return BENCH_EXIT_HARNESS;
  }
  for (size_t k = 0u; k < a.npick; k++) {
    int found = 0;
    for (size_t i = 0u; i < nw; i++) {
      if (strcmp(w[i].id, a.pick[k]) == 0) {
        w[i].selected = 1;
        found = 1;
      }
    }
    if (!found) {
      return bench_usage_error("unknown workload", a.pick[k]);
    }
  }
  for (size_t i = 0u; i < nw; i++) {
    if (a.npick == 0u) {
      w[i].selected = 1;
    }
  }
  code = 0;
  for (size_t i = 0u; i < nw; i++) {
    if (!w[i].selected) {
      continue;
    }
    if (bench_load_workload(a.root, a.golden_dir, &w[i]) != BENCH_OK) {
      code = BENCH_EXIT_HARNESS;
      break;
    }
    loaded = i + 1u;
  }

  /* Phase 2: the verification run and, unless --verify-only, the timed loop of each selected workload. */
  if (code == 0) {
    fputs(BENCH_FORMAT_LINE "\n" BENCH_HEADER "\n", stdout);
    (void)fflush(stdout);
    for (size_t i = 0u; i < nw && code == 0; i++) {
      if (!w[i].selected) {
        continue;
      }
      r = bench_run(a.content_dir, a.verify_only, &w[i]);
      if (r == BENCH_FAIL) {
        code = BENCH_EXIT_HARNESS;
      } else if (r == BENCH_REFUSED) {
        refused = 1;
      }
    }
    if (code == 0 && (fflush(stdout) != 0 || ferror(stdout))) {
      (void)bench_err(NULL, "write to stdout failed");
      code = BENCH_EXIT_HARNESS;
    }
    if (code == 0 && refused) {
      code = BENCH_EXIT_REFUSED;
    }
  }
  for (size_t i = 0u; i < loaded && i < nw; i++) {
    bench_free_workload(&w[i]);
  }
  return code;
}
