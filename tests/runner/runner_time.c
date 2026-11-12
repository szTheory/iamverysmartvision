/* SPDX-License-Identifier: MIT */
/* Suite runner, family time (D-21, plan 04-19): slicing, the machine-time budget, the wall cap and
 * cancellation of the runner library, in process, through a fake host with a fake clock (a fixed
 * step per now_ms call) and a fake cancel flag (raised from a given cancel_requested call on). No
 * sleeps, no signals, no wall time. The fake host serves boot-stub.exec and diag-cpu-1.ivsc from
 * the build-tree image include (ivs_images.inc; nothing committed).
 *   slices-equal       a halting scenario (diag-boot-pass) and a two-step target scenario with an
 *                      input schedule (the unconsumed suffix is resubmitted every slice): sliced
 *                      reports equal the single-call report except host.calls
 *   slice-sizes-equal  slice sizes 1, 7 and 1 << 22 give one report (except host.calls)
 *   budget             `stop halted` expected and the bound reached: timeout (budget), exit 11,
 *                      the stop time as in one call
 *   max-ticks          the ceiling below the bound: timeout (budget, max-ticks=); above it: pass
 *   wall               the fake clock passes the cap between slices: timeout (wall), exit 11,
 *                      never guest-fail
 *   wall-only-lowers   a scenario wall-timeout-ms above the operator cap is clamped, below it
 *                      lowers the cap; the default cap is 30000 ms
 *   cancel             cancel raised after slice 2: cancelled, exit 12, two slices ran
 *   cancel-over-wall   cancel and the wall cap at the same check: cancelled (precedence)
 *   no-clock           no now_ms: no wall cap, the run passes
 *   check-per-slice    cancel_requested and now_ms are called once before every slice
 *   options-refused    out-of-range options are harness-fail without a report */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_runner.h"
#include "ivs_test.h"
#include "runner_cases.h"

#include "ivs_images.inc"

#define TH_OUT_CAP (256u * 1024u)
#define TH_STORAGE (512u * 1024u)
#define TH_SCN_CAP 8192u
#define TH_VALUE_CAP 512u

/* ---- the fake host ---- */

typedef struct time_host {
  uint64_t clock;          /* current fake time in ms */
  uint64_t clock_step;     /* added at every now_ms call, before returning */
  uint64_t clock_calls;
  uint64_t cancel_from;    /* cancel_requested returns 1 from this call on (1-based); 0: never */
  uint64_t cancel_calls;
  int with_clock;          /* now_ms is in the vtable */
  char *out;
  size_t out_len;
} time_host;

static int th_read(void *ctx, const char *relpath, size_t cap, uint8_t *buf, size_t *len)
{
  const uint8_t *src = NULL;
  size_t n = 0u;
  (void)ctx;
  *len = 0u;
  if (strcmp(relpath, "boot-stub.exec") == 0) {
    src = ivs_img_boot_stub;
    n = IVS_IMG_BOOT_STUB_BYTES;
  } else if (strcmp(relpath, "diag-cpu-1.ivsc") == 0) {
    src = ivs_img_diag_cpu_1;
    n = IVS_IMG_DIAG_CPU_1_BYTES;
  } else {
    return IVS_RUNNER_READ_MISSING;
  }
  if (n > cap) {
    return IVS_RUNNER_READ_IO_ERROR; /* never in this family */
  }
  memcpy(buf, src, n);
  *len = n;
  return IVS_RUNNER_READ_OK;
}

static uint64_t th_now(void *ctx)
{
  time_host *h = ctx;
  h->clock_calls++;
  h->clock += h->clock_step;
  return h->clock;
}

static int th_cancel(void *ctx)
{
  time_host *h = ctx;
  h->cancel_calls++;
  return h->cancel_from != 0u && h->cancel_calls >= h->cancel_from;
}

static int th_write(void *ctx, const char *bytes, size_t len)
{
  time_host *h = ctx;
  if (h->out_len + len + 1u > TH_OUT_CAP) {
    return -1;
  }
  memcpy(h->out + h->out_len, bytes, len);
  h->out_len += len;
  h->out[h->out_len] = '\0';
  return 0;
}

/* ---- scenarios ---- */

static const char th_head[] = "ivss 1\n"
                              "id diag-boot-pass\n"
                              "profile intv-2609-ntsc-family-p1\n"
                              "content exec boot-stub\n"
                              "content grom zero-grom\n"
                              "content cart diag-cpu-1.ivsc\n";
static const char th_diag_asserts[] = ""
#include "diag_cpu_1_expect_text.inc"
    ;

/* head + extra header lines + body into buf (NUL-terminated); 1 when it fits. */
static int th_scn(char *buf, size_t cap, const char *header, const char *body)
{
  int n = snprintf(buf, cap, "%s%s%s", th_head, header, body);
  return n > 0 && (size_t)n < cap;
}

/* diag-boot-pass with the run-to line given (asserts after it). */
static int th_diag(char *buf, size_t cap, const char *header, const char *runto)
{
  char body[TH_SCN_CAP];
  int n = snprintf(body, sizeof body, "%s%s", runto, th_diag_asserts);
  if (n <= 0 || (size_t)n >= sizeof body) {
    return 0;
  }
  return th_scn(buf, cap, header, body);
}

/* Two steps to targets (before the guest halts at 2084 ticks) with two input events; slice 300
 * leaves a partial remainder in both steps. */
static const char th_two_steps[] = "input at_tick=100 c0=0x10 c1=0x0\n"
                                   "run-to 500 ticks\n"
                                   "input at_tick=1200 c0=0x0 c1=0x800\n"
                                   "run-to 2000 ticks\n"
                                   "assert halted == no\n";

/* Parses text and executes it on h with fresh heap storage; the report is in h->out (the caller
 * frees it with th_free). 0xFFFFFFFF when the text does not parse or memory is short. */
static uint32_t th_exec(const char *text, time_host *h, const ivs_runner_options *opts, ivs_runner_result *res)
{
  ivs_scenario *s = malloc(sizeof *s);
  ivs_scenario_error e;
  ivs_runner_host host;
  uint32_t out = 0xFFFFFFFFu;
  void *storage = malloc(TH_STORAGE);
  h->out = malloc(TH_OUT_CAP);
  h->out_len = 0u;
  if (s == NULL || storage == NULL || h->out == NULL) {
    goto done;
  }
  h->out[0] = '\0';
  memset(&e, 0, sizeof e);
  if (ivs_scenario_parse((const uint8_t *)text, strlen(text), s, &e) != 0) {
    ivs_test_diag("scenario refused: line %u reason %s", (unsigned)e.line, ivs_scenario_reason_name(e.reason));
    goto done;
  }
  memset(storage, 0x5A, TH_STORAGE);
  memset(&host, 0, sizeof host);
  host.ctx = h;
  host.read_file = th_read;
  host.now_ms = h->with_clock ? th_now : NULL;
  host.cancel_requested = th_cancel;
  host.write_report = th_write;
  host.storage = storage;
  host.storage_bytes = TH_STORAGE;
  out = ivs_runner_execute(s, &host, opts, res);
done:
  free(storage);
  free(s);
  return out;
}

static void th_free(time_host *h)
{
  free(h->out);
  h->out = NULL;
}

static void th_init(time_host *h, int with_clock)
{
  memset(h, 0, sizeof *h);
  h->with_clock = with_clock;
  h->clock = 1000u;
}

static void th_opts(ivs_runner_options *o, uint64_t slice, uint32_t wall_ms, uint64_t max_ticks)
{
  memset(o, 0, sizeof *o);
  o->slice_ticks = slice;
  o->wall_timeout_ms = wall_ms;
  o->max_ticks = max_ticks;
}

/* ---- report reading ---- */

/* The value of the single line whose key is key, into v. */
static int th_value(const char *report, const char *key, char *v, size_t cap)
{
  size_t kl = strlen(key);
  const char *p = report;
  int found = 0;
  while (p != NULL && *p != '\0') {
    const char *eol = strchr(p, '\n');
    size_t ll = eol != NULL ? (size_t)(eol - p) : strlen(p);
    if (ll > kl && strncmp(p, key, kl) == 0 && p[kl] == ' ') {
      size_t n = ll - kl - 1u;
      if (found || n + 1u > cap) {
        return 0;
      }
      memcpy(v, p + kl + 1u, n);
      v[n] = '\0';
      found = 1;
    }
    p = eol != NULL ? eol + 1 : NULL;
  }
  return found;
}

static int th_is(const char *report, const char *key, const char *want)
{
  char v[TH_VALUE_CAP];
  if (!th_value(report, key, v, sizeof v)) {
    ivs_test_diag("report: no single line %s", key);
    return 0;
  }
  if (strcmp(v, want) != 0) {
    ivs_test_diag("report: %s is '%s', want '%s'", key, v, want);
    return 0;
  }
  return 1;
}

static uint64_t th_u64(const char *report, const char *key)
{
  char v[TH_VALUE_CAP];
  uint64_t n = 0u;
  if (!th_value(report, key, v, sizeof v)) {
    return UINT64_MAX;
  }
  for (const char *p = v; *p >= '0' && *p <= '9'; p++) {
    n = n * 10u + (uint64_t)(*p - '0');
  }
  return n;
}

/* a and b are equal line by line, except the host.calls line (the one host-dependent key). */
static int th_same_but_calls(const char *a, const char *b, const char *what)
{
  size_t line = 1u;
  while (*a != '\0' || *b != '\0') {
    const char *ea = strchr(a, '\n');
    const char *eb = strchr(b, '\n');
    size_t la = ea != NULL ? (size_t)(ea - a) : strlen(a);
    size_t lb = eb != NULL ? (size_t)(eb - b) : strlen(b);
    int calls = strncmp(a, "host.calls ", 11u) == 0 && strncmp(b, "host.calls ", 11u) == 0;
    if (!calls && (la != lb || memcmp(a, b, la) != 0)) {
      ivs_test_diag("%s: line %zu differs: '%.*s' vs '%.*s'", what, line, (int)la, a, (int)lb, b);
      return 0;
    }
    a = ea != NULL ? ea + 1 : a + la;
    b = eb != NULL ? eb + 1 : b + lb;
    line++;
  }
  return 1;
}

/* Runs text with the given slice size (no clock, no cancel) and copies the report into out. */
static int th_report(const char *text, uint64_t slice, char *out, size_t cap, uint32_t *outcome)
{
  time_host h;
  ivs_runner_options o;
  ivs_runner_result res;
  int ok;
  th_init(&h, 0);
  th_opts(&o, slice, 0u, 0u);
  *outcome = th_exec(text, &h, &o, &res);
  ok = h.out != NULL && h.out_len + 1u <= cap;
  if (ok) {
    memcpy(out, h.out, h.out_len + 1u);
  }
  th_free(&h);
  return ok;
}

/* Every slice size in sizes gives the report of the single call (slice 1 << 22). */
static int th_sizes_equal(const char *text, const uint64_t *sizes, size_t n, uint32_t want)
{
  char *ref = malloc(TH_OUT_CAP);
  char *got = malloc(TH_OUT_CAP);
  uint32_t o1 = 0u;
  uint32_t o2 = 0u;
  int ok = ref != NULL && got != NULL && th_report(text, IVS_RUNNER_SLICE_TICKS_MAX, ref, TH_OUT_CAP, &o1);
  if (ok && o1 != want) {
    ivs_test_diag("single call: outcome %u, want %u", (unsigned)o1, (unsigned)want);
    ok = 0;
  }
  for (size_t i = 0u; ok && i < n; i++) {
    char what[64];
    ok = th_report(text, sizes[i], got, TH_OUT_CAP, &o2);
    (void)snprintf(what, sizeof what, "slice %llu", (unsigned long long)sizes[i]);
    ok = ok && o2 == o1 && th_same_but_calls(ref, got, what);
  }
  free(ref);
  free(got);
  return ok;
}

/* ---- cases ---- */

static int tm_slices_equal(void)
{
  static const uint64_t sizes[] = {300u, 1000u};
  char diag[TH_SCN_CAP];
  char two[TH_SCN_CAP];
  IVS_CHECK(th_diag(diag, sizeof diag, "", "run-to 4000 cpu-cycles stop halted\n"));
  IVS_CHECK(th_scn(two, sizeof two, "", th_two_steps));
  IVS_CHECK(th_sizes_equal(diag, sizes, 2u, IVS_RUNNER_OUTCOME_PASS));
  IVS_CHECK(th_sizes_equal(two, sizes, 2u, IVS_RUNNER_OUTCOME_PASS));
  return IVS_TEST_PASS;
}

static int tm_slice_sizes_equal(void)
{
  static const uint64_t sizes[] = {1u, 7u, IVS_RUNNER_SLICE_TICKS_MAX};
  char diag[TH_SCN_CAP];
  char two[TH_SCN_CAP];
  IVS_CHECK(th_diag(diag, sizeof diag, "", "run-to 4000 cpu-cycles stop halted\n"));
  IVS_CHECK(th_scn(two, sizeof two, "", th_two_steps));
  IVS_CHECK(th_sizes_equal(diag, sizes, 3u, IVS_RUNNER_OUTCOME_PASS));
  IVS_CHECK(th_sizes_equal(two, sizes, 3u, IVS_RUNNER_OUTCOME_PASS));
  return IVS_TEST_PASS;
}

/* The bound (100 cycles) comes before the halt: budget, with the stop time of one call. */
static int tm_budget(void)
{
  static const uint64_t sizes[] = {64u, 1u};
  char text[TH_SCN_CAP];
  char *rep = malloc(TH_OUT_CAP);
  uint32_t o = 0u;
  int ok;
  IVS_CHECK(rep != NULL);
  ok = th_diag(text, sizeof text, "", "run-to 100 cpu-cycles stop halted\n") &&
       th_report(text, 64u, rep, TH_OUT_CAP, &o) && o == IVS_RUNNER_OUTCOME_TIMEOUT_BUDGET &&
       th_is(rep, "outcome", "timeout") && th_is(rep, "exit.code", "11") &&
       th_is(rep, "outcome.detail", "budget step=1 line=7 expected=halted got=target") &&
       th_is(rep, "step.1.stop_reason", "target") && th_u64(rep, "step.1.now_ticks") >= 400u &&
       th_u64(rep, "step.1.now_ticks") < 2084u && th_sizes_equal(text, sizes, 2u, IVS_RUNNER_OUTCOME_TIMEOUT_BUDGET);
  free(rep);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

static int tm_max_ticks(void)
{
  char text[TH_SCN_CAP];
  time_host h;
  ivs_runner_options o;
  ivs_runner_result res;
  uint32_t out;
  int ok;
  IVS_CHECK(th_diag(text, sizeof text, "", "run-to 4000 cpu-cycles stop halted\n"));
  /* Ceiling 400 ticks, below the halt at 2084: timeout (budget), cut at the first boundary. */
  th_init(&h, 0);
  th_opts(&o, 64u, 0u, 400u);
  out = th_exec(text, &h, &o, &res);
  ok = out == IVS_RUNNER_OUTCOME_TIMEOUT_BUDGET && res.exit_code == 11u &&
       th_is(h.out, "outcome.detail", "budget step=1 line=7 max-ticks=400") &&
       th_is(h.out, "step.1.target_ticks", "16000 from=4000 cpu-cycles") && th_u64(h.out, "step.1.now_ticks") >= 400u &&
       th_u64(h.out, "step.1.now_ticks") < 2084u && th_is(h.out, "step.1.assert.1", "not-run halted == yes got -");
  th_free(&h);
  IVS_CHECK(ok);
  /* Ceiling above the halt: no effect. */
  th_init(&h, 0);
  th_opts(&o, 64u, 0u, 3000u);
  out = th_exec(text, &h, &o, &res);
  ok = out == IVS_RUNNER_OUTCOME_PASS && th_is(h.out, "step.1.now_ticks", "2084");
  th_free(&h);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

/* Clock step 3 ms, cap 10 ms: the start reads 1003, the checks 1006, 1009, 1012, 1015 (wall at
 * the fourth check, elapsed 12 ms, after three slices). */
static int tm_wall(void)
{
  char text[TH_SCN_CAP];
  time_host h;
  ivs_runner_options o;
  ivs_runner_result res;
  uint32_t out;
  int ok;
  IVS_CHECK(th_diag(text, sizeof text, "", "run-to 4000 cpu-cycles stop halted\n"));
  th_init(&h, 1);
  h.clock_step = 3u;
  th_opts(&o, 64u, 10u, 0u);
  out = th_exec(text, &h, &o, &res);
  ok = out == IVS_RUNNER_OUTCOME_TIMEOUT_WALL && res.exit_code == 11u && th_is(h.out, "outcome", "timeout") &&
       th_is(h.out, "exit.code", "11") && th_is(h.out, "outcome.detail", "wall step=1 cap-ms=10") &&
       h.clock_calls == 5u && h.cancel_calls == 4u && th_u64(h.out, "step.1.now_ticks") >= 192u &&
       th_u64(h.out, "step.1.now_ticks") < 2084u;
  th_free(&h);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

static int tm_wall_only_lowers(void)
{
  char text[TH_SCN_CAP];
  time_host h;
  ivs_runner_options o;
  ivs_runner_result res;
  uint32_t out;
  int ok;
  /* Scenario 100000 ms above the operator's 10 ms: clamped to 10. */
  IVS_CHECK(th_diag(text, sizeof text, "wall-timeout-ms 100000\n", "run-to 4000 cpu-cycles stop halted\n"));
  th_init(&h, 1);
  h.clock_step = 3u;
  th_opts(&o, 64u, 10u, 0u);
  out = th_exec(text, &h, &o, &res);
  ok = out == IVS_RUNNER_OUTCOME_TIMEOUT_WALL && th_is(h.out, "outcome.detail", "wall step=1 cap-ms=10");
  th_free(&h);
  IVS_CHECK(ok);
  /* Scenario 5 ms below the operator's 1000 ms: lowered to 5 (start 1003, wall at 1009). */
  IVS_CHECK(th_diag(text, sizeof text, "wall-timeout-ms 5\n", "run-to 4000 cpu-cycles stop halted\n"));
  th_init(&h, 1);
  h.clock_step = 3u;
  th_opts(&o, 64u, 1000u, 0u);
  out = th_exec(text, &h, &o, &res);
  ok = out == IVS_RUNNER_OUTCOME_TIMEOUT_WALL && th_is(h.out, "outcome.detail", "wall step=1 cap-ms=5") &&
       h.cancel_calls == 2u;
  th_free(&h);
  IVS_CHECK(ok);
  /* No cap given: the default 30000 ms; about 33 checks of 3 ms each never reach it. */
  IVS_CHECK(th_diag(text, sizeof text, "", "run-to 4000 cpu-cycles stop halted\n"));
  th_init(&h, 1);
  h.clock_step = 3u;
  th_opts(&o, 64u, 0u, 0u);
  out = th_exec(text, &h, &o, &res);
  ok = out == IVS_RUNNER_OUTCOME_PASS && h.clock_calls > 2u;
  th_free(&h);
  IVS_CHECK(ok);
  /* The default cap is reached with a large clock step. */
  th_init(&h, 1);
  h.clock_step = IVS_RUNNER_WALL_MS_DEFAULT;
  out = th_exec(text, &h, NULL, &res);
  ok = out == IVS_RUNNER_OUTCOME_TIMEOUT_WALL && th_is(h.out, "outcome.detail", "wall step=1 cap-ms=30000");
  th_free(&h);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

/* Cancel raised at the third check (after slice 2): cancelled, exit 12, two slices ran. */
static int tm_cancel(void)
{
  char text[TH_SCN_CAP];
  time_host h;
  ivs_runner_options o;
  ivs_runner_result res;
  uint32_t out;
  int ok;
  IVS_CHECK(th_diag(text, sizeof text, "", "run-to 4000 cpu-cycles stop halted\n"));
  th_init(&h, 1);
  h.cancel_from = 3u;
  th_opts(&o, 64u, 0u, 0u);
  out = th_exec(text, &h, &o, &res);
  ok = out == IVS_RUNNER_OUTCOME_CANCELLED && res.exit_code == 12u && th_is(h.out, "outcome", "cancelled") &&
       th_is(h.out, "exit.code", "12") && th_is(h.out, "outcome.detail", "signal step=1") && h.cancel_calls == 3u &&
       th_u64(h.out, "step.1.now_ticks") >= 128u && th_u64(h.out, "step.1.now_ticks") < 2084u &&
       th_is(h.out, "step.1.assert.1", "not-run halted == yes got -");
  th_free(&h);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

/* Cancel and the wall cap (9 ms, elapsed 9 ms) at the same third check: cancelled wins. */
static int tm_cancel_over_wall(void)
{
  char text[TH_SCN_CAP];
  time_host h;
  ivs_runner_options o;
  ivs_runner_result res;
  uint32_t out;
  int ok;
  IVS_CHECK(th_diag(text, sizeof text, "", "run-to 4000 cpu-cycles stop halted\n"));
  th_init(&h, 1);
  h.clock_step = 3u;
  h.cancel_from = 3u;
  th_opts(&o, 64u, 9u, 0u);
  out = th_exec(text, &h, &o, &res);
  ok = out == IVS_RUNNER_OUTCOME_CANCELLED && res.exit_code == 12u && th_is(h.out, "outcome.detail", "signal step=1") &&
       h.cancel_calls == 3u && h.clock_calls == 4u;
  th_free(&h);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

/* Without now_ms there is no wall cap: slice 1 and a 1 ms cap still pass. */
static int tm_no_clock(void)
{
  char text[TH_SCN_CAP];
  time_host h;
  ivs_runner_options o;
  ivs_runner_result res;
  uint32_t out;
  int ok;
  IVS_CHECK(th_diag(text, sizeof text, "", "run-to 4000 cpu-cycles stop halted\n"));
  th_init(&h, 0);
  th_opts(&o, 1u, 1u, 0u);
  out = th_exec(text, &h, &o, &res);
  ok = out == IVS_RUNNER_OUTCOME_PASS && h.clock_calls == 0u && h.cancel_calls > 1u;
  th_free(&h);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

/* One cancel_requested and one now_ms call before every slice (plus the start's now_ms): two
 * steps in one slice each make 2 and 3; host.calls counts them with the two file reads. */
static int tm_check_per_slice(void)
{
  char text[TH_SCN_CAP];
  time_host h;
  ivs_runner_result res;
  uint32_t out;
  int ok;
  IVS_CHECK(th_scn(text, sizeof text, "", th_two_steps));
  th_init(&h, 1);
  out = th_exec(text, &h, NULL, &res);
  ok = out == IVS_RUNNER_OUTCOME_PASS && h.cancel_calls == 2u && h.clock_calls == 3u && res.host_calls == 7u &&
       th_is(h.out, "host.calls", "7");
  th_free(&h);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

static int tm_options_refused(void)
{
  char text[TH_SCN_CAP];
  time_host h;
  ivs_runner_options o;
  ivs_runner_result res;
  uint32_t out;
  int ok;
  IVS_CHECK(th_diag(text, sizeof text, "", "run-to 4000 cpu-cycles stop halted\n"));
  th_init(&h, 0);
  th_opts(&o, IVS_RUNNER_SLICE_TICKS_MAX + 1u, 0u, 0u);
  out = th_exec(text, &h, &o, &res);
  ok = out == IVS_RUNNER_OUTCOME_HARNESS_FAIL && res.exit_code == 14u && h.out_len == 0u;
  th_free(&h);
  IVS_CHECK(ok);
  th_init(&h, 0);
  th_opts(&o, 0u, 0u, (uint64_t)IVS_RUN_TARGET_MAX + 1u);
  out = th_exec(text, &h, &o, &res);
  ok = out == IVS_RUNNER_OUTCOME_HARNESS_FAIL && h.out_len == 0u;
  th_free(&h);
  IVS_CHECK(ok);
  th_init(&h, 0);
  th_opts(&o, 0u, 0u, 0u);
  o.flags = 1u;
  out = th_exec(text, &h, &o, &res);
  ok = out == IVS_RUNNER_OUTCOME_HARNESS_FAIL && h.out_len == 0u;
  th_free(&h);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

static const ivs_test_case time_cases[] = {
    {"runner.time.budget", tm_budget, NULL},
    {"runner.time.cancel", tm_cancel, NULL},
    {"runner.time.cancel-over-wall", tm_cancel_over_wall, NULL},
    {"runner.time.check-per-slice", tm_check_per_slice, NULL},
    {"runner.time.max-ticks", tm_max_ticks, NULL},
    {"runner.time.no-clock", tm_no_clock, NULL},
    {"runner.time.options-refused", tm_options_refused, NULL},
    {"runner.time.slice-sizes-equal", tm_slice_sizes_equal, NULL},
    {"runner.time.slices-equal", tm_slices_equal, NULL},
    {"runner.time.wall", tm_wall, NULL},
    {"runner.time.wall-only-lowers", tm_wall_only_lowers, NULL},
};

#define TIME_CASES (sizeof time_cases / sizeof time_cases[0])

size_t runner_time_count(void) { return TIME_CASES; }

const char *runner_time_id(size_t i) { return i < TIME_CASES ? time_cases[i].id : NULL; }

int runner_time_run(size_t i) { return i < TIME_CASES ? time_cases[i].fn() : IVS_TEST_FAIL; }
