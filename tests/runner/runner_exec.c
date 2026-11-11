/* SPDX-License-Identifier: MIT */
/* Suite runner, family exec (D-17..D-20, D-22, D-30, plan 04-18): the scenario executor and the
 * report writer of the runner library, in process, through a fake host vtable. The fake host
 * serves boot-stub.exec and diag-cpu-1.ivsc from the build-tree image include (ivs_images.inc,
 * written by tools/pack; nothing committed) and collects the report in memory; it has no clock
 * and no file system.
 *   runner.exec.diag-pass  the diag-boot-pass scenario (the 18 assert lines of
 *                          data/golden/diag-cpu-1.expect, read through diag_cpu_1_expect_text.inc)
 *                          runs to pass, exit 0, with the label lines and a complete report.
 *   Outcome paths (D-20): guest-assert-mismatch and stop-mismatch (guest-fail, exit 10),
 *   halt-bound-small (timeout budget, exit 11), stop-internal (the pure stop classification:
 *   INTERNAL is harness-fail), missing-file, not-regular, user-role (missing-fixture, exit 13),
 *   io-error, unexpected-refusal, too-large, sha-mismatch, unknown-profile, write-error
 *   (harness-fail, exit 14), expect-load-refusal and sha-match (pass), cancel (exit 12).
 *   Report: key-order (the pinned key sequence, written out here independently of report.c),
 *   no-paths, repeat-identical (byte-equal apart from host.calls), overshoot, units (cycles and
 *   frames sugar), input-schedule (only the input lines and the INPUT digest change),
 *   two-steps and assert-forms. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_runner.h"
#include "ivs_test.h"
#include "runner_cases.h"

#include "ivs_images.inc"

#define FH_MAX_FILES 4u
#define FH_OUT_CAP (256u * 1024u)
#define FH_STORAGE (512u * 1024u)
#define RX_SCN_CAP 8192u
#define RX_VALUE_CAP 512u

/* ---- the fake host ---- */

typedef struct fh_file {
  const char *name;
  const uint8_t *bytes;
  size_t len;
  int result; /* IVS_RUNNER_READ_OK, or a forced result */
} fh_file;

typedef struct fake_host {
  fh_file files[FH_MAX_FILES];
  size_t n_files;
  int cancel;
  int fail_write;
  char *out;
  size_t out_len;
} fake_host;

static int fh_read(void *ctx, const char *relpath, size_t cap, uint8_t *buf, size_t *len)
{
  fake_host *h = ctx;
  *len = 0u;
  for (size_t i = 0u; i < h->n_files; i++) {
    const fh_file *f = &h->files[i];
    size_t n;
    if (strcmp(f->name, relpath) != 0) {
      continue;
    }
    if (f->result != IVS_RUNNER_READ_OK) {
      return f->result;
    }
    n = f->len > cap + 1u ? cap + 1u : f->len;
    memcpy(buf, f->bytes, n);
    *len = n;
    return n > cap ? IVS_RUNNER_READ_TOO_LARGE : IVS_RUNNER_READ_OK;
  }
  return IVS_RUNNER_READ_MISSING;
}

static int fh_cancel(void *ctx) { return ((fake_host *)ctx)->cancel; }

static int fh_write(void *ctx, const char *bytes, size_t len)
{
  fake_host *h = ctx;
  if (h->fail_write || h->out_len + len + 1u > FH_OUT_CAP) {
    return -1;
  }
  memcpy(h->out + h->out_len, bytes, len);
  h->out_len += len;
  h->out[h->out_len] = '\0';
  return 0;
}

static void fh_add(fake_host *h, const char *name, const uint8_t *bytes, size_t len, int result)
{
  if (h->n_files < FH_MAX_FILES) {
    h->files[h->n_files].name = name;
    h->files[h->n_files].bytes = bytes;
    h->files[h->n_files].len = len;
    h->files[h->n_files].result = result;
    h->n_files++;
  }
}

/* A host serving the stub and the diagnostic cartridge. */
static void fh_init(fake_host *h)
{
  memset(h, 0, sizeof *h);
  fh_add(h, "boot-stub.exec", ivs_img_boot_stub, IVS_IMG_BOOT_STUB_BYTES, IVS_RUNNER_READ_OK);
  fh_add(h, "diag-cpu-1.ivsc", ivs_img_diag_cpu_1, IVS_IMG_DIAG_CPU_1_BYTES, IVS_RUNNER_READ_OK);
}

/* ---- scenarios ---- */

static const char rx_head[] = "ivss 1\n"
                              "id diag-boot-pass\n"
                              "profile intv-2609-ntsc-family-p1\n";
static const char rx_content[] = "content exec boot-stub\n"
                                 "content grom zero-grom\n"
                                 "content cart diag-cpu-1.ivsc\n";
static const char rx_diag_asserts[] = ""
#include "diag_cpu_1_expect_text.inc"
    ;

/* head + content + body into buf (NUL-terminated). 1 when it fits. */
static int rx_scn(char *buf, size_t cap, const char *content, const char *body)
{
  size_t a = strlen(rx_head);
  size_t b = strlen(content);
  size_t c = strlen(body);
  if (a + b + c + 1u > cap) {
    return 0;
  }
  memcpy(buf, rx_head, a);
  memcpy(buf + a, content, b);
  memcpy(buf + a + b, body, c + 1u);
  return 1;
}

/* The diag-boot-pass text: run-to line, then the 18 .expect assert lines. */
static int rx_diag(char *buf, size_t cap)
{
  char body[RX_SCN_CAP];
  size_t r = strlen("run-to 4000 cpu-cycles stop halted\n");
  if (r + strlen(rx_diag_asserts) + 1u > sizeof body) {
    return 0;
  }
  memcpy(body, "run-to 4000 cpu-cycles stop halted\n", r);
  memcpy(body + r, rx_diag_asserts, strlen(rx_diag_asserts) + 1u);
  return rx_scn(buf, cap, rx_content, body);
}

/* Parses text, executes it on h with fresh heap storage and returns the outcome; the report is in
 * h->out. 0xFFFFFFFF when the text does not parse or memory is short. */
static uint32_t rx_exec(const char *text, fake_host *h, ivs_runner_result *res)
{
  ivs_scenario *s = malloc(sizeof *s);
  ivs_scenario_error e;
  ivs_runner_host host;
  uint32_t out = 0xFFFFFFFFu;
  void *storage = malloc(FH_STORAGE);
  h->out = malloc(FH_OUT_CAP);
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
  memset(storage, 0xA5, FH_STORAGE);
  memset(&host, 0, sizeof host);
  host.ctx = h;
  host.read_file = fh_read;
  host.cancel_requested = fh_cancel;
  host.write_report = fh_write;
  host.storage = storage;
  host.storage_bytes = FH_STORAGE;
  out = ivs_runner_execute(s, &host, NULL, res);
done:
  free(storage);
  free(s);
  return out;
}

static void rx_free(fake_host *h)
{
  free(h->out);
  h->out = NULL;
}

/* ---- report reading ---- */

/* The value of the line whose key is key (text after `key `), copied into v; 1 when found once. */
static int rx_value(const char *report, const char *key, char *v, size_t cap)
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

static int rx_is(const char *report, const char *key, const char *want)
{
  char v[RX_VALUE_CAP];
  if (!rx_value(report, key, v, sizeof v)) {
    ivs_test_diag("report: no single line %s", key);
    return 0;
  }
  if (strcmp(v, want) != 0) {
    ivs_test_diag("report: %s is '%s', want '%s'", key, v, want);
    return 0;
  }
  return 1;
}

/* Number of lines of the report. */
static size_t rx_lines(const char *report)
{
  size_t n = 0u;
  for (const char *p = report; *p != '\0'; p++) {
    n += *p == '\n' ? 1u : 0u;
  }
  return n;
}

/* 1 when s is 64 lower-case hex digits. */
static int rx_hex64(const char *s)
{
  size_t i;
  for (i = 0u; s[i] != '\0'; i++) {
    if (!((s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'f'))) {
      return 0;
    }
  }
  return i == 64u;
}

/* The report is complete: first line `ivsr 1`, last `report.end <lines before it>`, LF ended. */
static int rx_complete(const char *report, const ivs_runner_result *res)
{
  char v[RX_VALUE_CAP];
  size_t n = rx_lines(report);
  size_t len = strlen(report);
  if (strncmp(report, "ivsr 1\n", 7u) != 0 || len == 0u || report[len - 1u] != '\n') {
    ivs_test_diag("report does not start with ivsr 1 or end with LF");
    return 0;
  }
  if (!rx_value(report, "report.end", v, sizeof v) || strtoull(v, NULL, 10) != (unsigned long long)(n - 1u)) {
    ivs_test_diag("report.end is not the count of the %zu lines before it", n - 1u);
    return 0;
  }
  if (strstr(report, "\nreport.end ") == NULL || strchr(strstr(report, "\nreport.end ") + 1, '\n') != report + len - 1u) {
    ivs_test_diag("report.end is not the last line");
    return 0;
  }
  if (res != NULL && res->report_lines != (uint64_t)(n - 1u)) {
    ivs_test_diag("result report_lines %llu, report has %zu", (unsigned long long)res->report_lines, n - 1u);
    return 0;
  }
  return 1;
}

/* ---- cases ---- */

static int ex_diag_pass(void)
{
  fake_host h;
  ivs_runner_result res;
  char scn[RX_SCN_CAP];
  char v[RX_VALUE_CAP];
  char want[RX_VALUE_CAP];
  int rc = IVS_TEST_FAIL;
  uint32_t out;
  fh_init(&h);
  if (!rx_diag(scn, sizeof scn)) {
    return IVS_TEST_FAIL;
  }
  out = rx_exec(scn, &h, &res);
  if (out != IVS_RUNNER_OUTCOME_PASS || res.outcome != out || res.exit_code != 0u || res.report_write_failed != 0u) {
    ivs_test_diag("outcome %u exit %u\n%s", (unsigned)out, (unsigned)res.exit_code, h.out != NULL ? h.out : "");
    goto done;
  }
  if (!rx_complete(h.out, &res) || !rx_is(h.out, "outcome", "pass") || !rx_is(h.out, "exit.code", "0") ||
      !rx_is(h.out, "outcome.detail", "-") || !rx_is(h.out, "scenario.id", "diag-boot-pass") ||
      !rx_is(h.out, "scenario.profile", "intv-2609-ntsc-family-p1") || !rx_is(h.out, "slot.exec.loaded", "1") ||
      !rx_is(h.out, "slot.exec.length_bytes", "8192") || !rx_is(h.out, "slot.grom.length_bytes", "2048") ||
      !rx_is(h.out, "slot.cart.status", "ok") || !rx_is(h.out, "grom:", "zero image, not a GROM") ||
      !rx_is(h.out, "step.1.stop_reason", "halted") || !rx_is(h.out, "step.1.expected_stop", "halted") ||
      !rx_is(h.out, "step.1.overshoot_ticks", "0") || !rx_is(h.out, "cpu.run_state", "halted")) {
    goto done;
  }
  /* The stub label carries the digest of the loaded stub (D-30). */
  if (!rx_value(h.out, "slot.exec.sha256", v, sizeof v) || !rx_hex64(v)) {
    goto done;
  }
  (void)snprintf(want, sizeof want, "project boot stub, not an EXEC sha256=%s", v);
  if (!rx_is(h.out, "exec:", want)) {
    goto done;
  }
  /* Every one of the 18 asserts passed and is in the report. */
  for (unsigned j = 1u; j <= 18u; j++) {
    char key[32];
    (void)snprintf(key, sizeof key, "step.1.assert.%u", j);
    if (!rx_value(h.out, key, v, sizeof v) || strncmp(v, "pass ", 5u) != 0) {
      ivs_test_diag("%s: '%s'", key, v);
      goto done;
    }
  }
  if (!rx_value(h.out, "digest.composite", v, sizeof v) || !rx_hex64(v)) {
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  rx_free(&h);
  return rc;
}

/* ---- helpers of the outcome cases ---- */

/* src with the first occurrence of old replaced by repl, into out. 1 when old was found. */
static int rx_replace(const char *src, const char *old, const char *repl, char *out, size_t cap)
{
  const char *at = strstr(src, old);
  size_t a;
  size_t b = strlen(repl);
  size_t c;
  if (at == NULL) {
    ivs_test_diag("text '%s' not found", old);
    return 0;
  }
  a = (size_t)(at - src);
  c = strlen(at + strlen(old));
  if (a + b + c + 1u > cap) {
    return 0;
  }
  memcpy(out, src, a);
  memcpy(out + a, repl, b);
  memcpy(out + a + b, at + strlen(old), c + 1u);
  return 1;
}

/* 1 when the value of key starts with prefix. */
static int rx_starts(const char *report, const char *key, const char *prefix)
{
  char v[RX_VALUE_CAP];
  if (!rx_value(report, key, v, sizeof v) || strncmp(v, prefix, strlen(prefix)) != 0) {
    ivs_test_diag("report: %s is '%s', want prefix '%s'", key, v, prefix);
    return 0;
  }
  return 1;
}

/* Runs text on a fresh fake host (initialised by the caller) and checks the outcome, the exit
 * code and the completeness of the report; the report stays in h->out. */
static int rx_expect(const char *text, fake_host *h, uint32_t outcome, uint32_t exit_code)
{
  ivs_runner_result res;
  uint32_t out = rx_exec(text, h, &res);
  if (out != outcome || res.outcome != outcome || res.exit_code != exit_code) {
    ivs_test_diag("outcome %u exit %u, want %u exit %u\n%s", (unsigned)out, (unsigned)res.exit_code, (unsigned)outcome,
                  (unsigned)exit_code, h->out != NULL ? h->out : "");
    return 0;
  }
  if (h->out == NULL || !rx_complete(h->out, &res)) {
    return 0;
  }
  return 1;
}

/* The diagnostic text with the content lines replaced by content. */
static int rx_diag_content(char *buf, size_t cap, const char *content)
{
  char diag[RX_SCN_CAP];
  return rx_diag(diag, sizeof diag) && rx_replace(diag, rx_content, content, buf, cap);
}

/* The profile timebase through a short-lived instance (ivs_machine_info). */
static int rx_timebase(uint64_t *tpc, uint64_t *nft)
{
  ivs_storage_requirements req;
  ivs_create_options co;
  ivs_machine *m = NULL;
  ivs_info info;
  void *mem;
  size_t bytes;
  int ok = 0;
  memset(&req, 0, sizeof req);
  req.struct_size = (uint32_t)sizeof req;
  if (ivs_machine_storage_requirements("intv-2609-ntsc-family-p1", &req) != IVS_OK) {
    return 0;
  }
  bytes = (size_t)((req.size_bytes + req.align_bytes - 1u) / req.align_bytes * req.align_bytes);
  mem = aligned_alloc((size_t)req.align_bytes, bytes);
  if (mem == NULL) {
    return 0;
  }
  memset(&co, 0, sizeof co);
  co.struct_size = (uint32_t)sizeof co;
  co.profile_id = "intv-2609-ntsc-family-p1";
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_create(&co, mem, bytes, &m) == IVS_OK && ivs_machine_info(m, &info) == IVS_OK) {
    *tpc = info.ticks_per_cpu_cycle;
    *nft = info.nominal_frame_ticks;
    ok = *tpc != 0u && *nft != 0u;
  }
  free(mem);
  return ok;
}

/* ---- outcome cases (plan 04-18 Task 2) ---- */

/* A correct run with a deliberately wrong expected word: guest-fail naming the assert. */
static int ex_guest_assert_mismatch(void)
{
  fake_host h;
  char diag[RX_SCN_CAP];
  char scn[RX_SCN_CAP];
  int rc = IVS_TEST_FAIL;
  fh_init(&h);
  if (!rx_diag(diag, sizeof diag) || !rx_replace(diag, "assert reg R0 == 03C5", "assert reg R0 == 03C4", scn, sizeof scn)) {
    return IVS_TEST_FAIL;
  }
  if (rx_expect(scn, &h, IVS_RUNNER_OUTCOME_GUEST_FAIL, 10u) && rx_is(h.out, "outcome", "guest-fail") &&
      rx_is(h.out, "exit.code", "10") && rx_is(h.out, "outcome.detail", "assert=2 line=9 reg R0 == 03C4 got 03C5") &&
      rx_is(h.out, "step.1.assert.2", "fail reg R0 == 03C4 got 03C5") &&
      rx_is(h.out, "step.1.assert.3", "pass reg R1 == 8080 got 8080") && rx_is(h.out, "step.1.stop_reason", "halted")) {
    rc = IVS_TEST_PASS;
  }
  rx_free(&h);
  return rc;
}

/* `stop target` (and the default) expected, the guest halted: guest-fail naming the stop. */
static int ex_stop_mismatch(void)
{
  static const char *const forms[] = {"run-to 4000 cpu-cycles stop target\n", "run-to 4000 cpu-cycles\n"};
  char diag[RX_SCN_CAP];
  char scn[RX_SCN_CAP];
  if (!rx_diag(diag, sizeof diag)) {
    return IVS_TEST_FAIL;
  }
  for (size_t i = 0u; i < 2u; i++) {
    fake_host h;
    int ok;
    fh_init(&h);
    if (!rx_replace(diag, "run-to 4000 cpu-cycles stop halted\n", forms[i], scn, sizeof scn)) {
      return IVS_TEST_FAIL;
    }
    ok = rx_expect(scn, &h, IVS_RUNNER_OUTCOME_GUEST_FAIL, 10u) &&
         rx_is(h.out, "outcome.detail", "stop step=1 line=7 expected=target got=halted") &&
         rx_is(h.out, "step.1.expected_stop", "target") && rx_is(h.out, "step.1.stop_reason", "halted") &&
         rx_is(h.out, "step.1.assert.1", "pass halted == yes got yes");
    rx_free(&h);
    if (!ok) {
      return IVS_TEST_FAIL;
    }
  }
  return IVS_TEST_PASS;
}

/* The stop classification is a pure function: INTERNAL (which a valid scenario cannot drive the
 * core into) is harness-fail, exit 14, whatever was expected; the other pairs per D-20. */
static int ex_stop_internal(void)
{
  static const uint32_t expected[] = {IVS_SCN_STOP_TARGET, IVS_SCN_STOP_HALTED, IVS_SCN_STOP_FAULTED};
  for (size_t i = 0u; i < 3u; i++) {
    uint32_t obs = ivs_runner_stop_observation(IVS_STOP_INTERNAL, expected[i]);
    IVS_CHECK(obs == IVS_RUNNER_OBS_HARNESS_FAIL);
    IVS_CHECK(ivs_runner_outcome_resolve(obs) == IVS_RUNNER_OUTCOME_HARNESS_FAIL);
    IVS_CHECK(ivs_runner_exit_code(ivs_runner_outcome_resolve(obs)) == 14u);
    IVS_CHECK(ivs_runner_stop_observation(0u, expected[i]) == IVS_RUNNER_OBS_HARNESS_FAIL);
    IVS_CHECK(ivs_runner_stop_observation(5u, expected[i]) == IVS_RUNNER_OBS_HARNESS_FAIL);
  }
  IVS_CHECK(ivs_runner_stop_observation(IVS_STOP_TARGET, 0u) == IVS_RUNNER_OBS_HARNESS_FAIL);
  IVS_CHECK(ivs_runner_stop_observation(IVS_STOP_CPU_HALTED, 4u) == IVS_RUNNER_OBS_HARNESS_FAIL);
  /* Matches. */
  IVS_CHECK(ivs_runner_stop_observation(IVS_STOP_TARGET, IVS_SCN_STOP_TARGET) == 0u);
  IVS_CHECK(ivs_runner_stop_observation(IVS_STOP_CPU_HALTED, IVS_SCN_STOP_HALTED) == 0u);
  IVS_CHECK(ivs_runner_stop_observation(IVS_STOP_GUEST_FAULT, IVS_SCN_STOP_FAULTED) == 0u);
  /* The bound reached while a halt or fault was expected: timeout (budget), not guest-fail. */
  IVS_CHECK(ivs_runner_stop_observation(IVS_STOP_TARGET, IVS_SCN_STOP_HALTED) == IVS_RUNNER_OBS_TIMEOUT_BUDGET);
  IVS_CHECK(ivs_runner_stop_observation(IVS_STOP_TARGET, IVS_SCN_STOP_FAULTED) == IVS_RUNNER_OBS_TIMEOUT_BUDGET);
  /* The guest halted or faulted when something else was expected: guest-fail. */
  IVS_CHECK(ivs_runner_stop_observation(IVS_STOP_CPU_HALTED, IVS_SCN_STOP_TARGET) == IVS_RUNNER_OBS_GUEST_FAIL);
  IVS_CHECK(ivs_runner_stop_observation(IVS_STOP_CPU_HALTED, IVS_SCN_STOP_FAULTED) == IVS_RUNNER_OBS_GUEST_FAIL);
  IVS_CHECK(ivs_runner_stop_observation(IVS_STOP_GUEST_FAULT, IVS_SCN_STOP_TARGET) == IVS_RUNNER_OBS_GUEST_FAIL);
  IVS_CHECK(ivs_runner_stop_observation(IVS_STOP_GUEST_FAULT, IVS_SCN_STOP_HALTED) == IVS_RUNNER_OBS_GUEST_FAIL);
  return IVS_TEST_PASS;
}

/* `stop halted` with a bound too small: timeout (budget), not guest-fail, though the asserts at
 * that stop fail too (timeout ranks above guest-fail). */
static int ex_halt_bound_small(void)
{
  fake_host h;
  char diag[RX_SCN_CAP];
  char scn[RX_SCN_CAP];
  int rc = IVS_TEST_FAIL;
  fh_init(&h);
  if (!rx_diag(diag, sizeof diag) ||
      !rx_replace(diag, "run-to 4000 cpu-cycles", "run-to 100 cpu-cycles", scn, sizeof scn)) {
    return IVS_TEST_FAIL;
  }
  if (rx_expect(scn, &h, IVS_RUNNER_OUTCOME_TIMEOUT_BUDGET, 11u) && rx_is(h.out, "outcome", "timeout") &&
      rx_is(h.out, "outcome.detail", "budget step=1 line=7 expected=halted got=target") &&
      rx_is(h.out, "step.1.stop_reason", "target") && rx_is(h.out, "step.1.assert.1", "fail halted == yes got no")) {
    rc = IVS_TEST_PASS;
  }
  rx_free(&h);
  return rc;
}

/* A content file the host does not have (the cartridge, then the reserved stub file): missing-
 * fixture, exit 13, nothing run. */
static int ex_missing_file(void)
{
  fake_host h;
  char diag[RX_SCN_CAP];
  int rc = IVS_TEST_FAIL;
  if (!rx_diag(diag, sizeof diag)) {
    return IVS_TEST_FAIL;
  }
  fh_init(&h);
  h.n_files = 1u; /* the stub only */
  if (!rx_expect(diag, &h, IVS_RUNNER_OUTCOME_MISSING_FIXTURE, 13u) ||
      !rx_is(h.out, "outcome.detail", "missing role=cart path=diag-cpu-1.ivsc") ||
      !rx_is(h.out, "outcome", "missing-fixture") || !rx_is(h.out, "slot.cart.status", "-") ||
      !rx_is(h.out, "slot.cart.loaded", "0") || !rx_is(h.out, "step.1.now_ticks", "-") ||
      !rx_is(h.out, "step.1.assert.1", "not-run halted == yes got -") || !rx_is(h.out, "input.controller.0", "-")) {
    goto done;
  }
  rx_free(&h);
  fh_init(&h);
  h.files[0].name = "stub.exec"; /* not the reserved name */
  if (rx_expect(diag, &h, IVS_RUNNER_OUTCOME_MISSING_FIXTURE, 13u) &&
      rx_is(h.out, "outcome.detail", "missing role=exec path=boot-stub.exec") &&
      rx_is(h.out, "slot.grom.status", "-")) {
    rc = IVS_TEST_PASS;
  }
done:
  rx_free(&h);
  return rc;
}

/* A file the host reports as not regular (a FIFO, a device): missing-fixture. */
static int ex_not_regular(void)
{
  fake_host h;
  char diag[RX_SCN_CAP];
  int rc = IVS_TEST_FAIL;
  if (!rx_diag(diag, sizeof diag)) {
    return IVS_TEST_FAIL;
  }
  fh_init(&h);
  h.files[1].result = IVS_RUNNER_READ_NOT_REGULAR;
  if (rx_expect(diag, &h, IVS_RUNNER_OUTCOME_MISSING_FIXTURE, 13u) &&
      rx_is(h.out, "outcome.detail", "not-regular role=cart path=diag-cpu-1.ivsc")) {
    rc = IVS_TEST_PASS;
  }
  rx_free(&h);
  return rc;
}

/* A `user` role without an operator image (no read_user): missing-fixture. */
static int ex_user_role(void)
{
  fake_host h;
  char scn[RX_SCN_CAP];
  int rc = IVS_TEST_FAIL;
  if (!rx_diag_content(scn, sizeof scn, "content exec boot-stub\ncontent grom zero-grom\ncontent cart user\n")) {
    return IVS_TEST_FAIL;
  }
  fh_init(&h);
  if (rx_expect(scn, &h, IVS_RUNNER_OUTCOME_MISSING_FIXTURE, 13u) &&
      rx_is(h.out, "outcome.detail", "missing role=cart user")) {
    rc = IVS_TEST_PASS;
  }
  rx_free(&h);
  return rc;
}

/* A host read error: harness-fail. */
static int ex_io_error(void)
{
  fake_host h;
  char diag[RX_SCN_CAP];
  int rc = IVS_TEST_FAIL;
  if (!rx_diag(diag, sizeof diag)) {
    return IVS_TEST_FAIL;
  }
  fh_init(&h);
  h.files[1].result = IVS_RUNNER_READ_IO_ERROR;
  if (rx_expect(diag, &h, IVS_RUNNER_OUTCOME_HARNESS_FAIL, 14u) &&
      rx_is(h.out, "outcome.detail", "io-error role=cart path=diag-cpu-1.ivsc")) {
    rc = IVS_TEST_PASS;
  }
  rx_free(&h);
  return rc;
}

/* A truncated cartridge: the core's refusal is harness-fail unless expect-load names it. */
static int ex_unexpected_refusal(void)
{
  fake_host h;
  char diag[RX_SCN_CAP];
  int rc = IVS_TEST_FAIL;
  if (!rx_diag(diag, sizeof diag)) {
    return IVS_TEST_FAIL;
  }
  fh_init(&h);
  h.files[1].len = 16u;
  if (rx_expect(diag, &h, IVS_RUNNER_OUTCOME_HARNESS_FAIL, 14u) &&
      rx_starts(h.out, "outcome.detail", "load-refused role=cart status=IVS_E_MALFORMED reason=") &&
      rx_is(h.out, "slot.cart.status", "IVS_E_MALFORMED") && rx_is(h.out, "slot.cart.loaded", "0") &&
      rx_is(h.out, "slot.cart.sha256", "-") && rx_is(h.out, "step.1.stop_reason", "-")) {
    rc = IVS_TEST_PASS;
  }
  rx_free(&h);
  return rc;
}

/* A refusal that matches expect-load ends the scenario as pass (status alone, then status and
 * reason); a wrong reason, or a load that was not refused, is harness-fail. */
static int ex_expect_load_refusal(void)
{
  static const char *const ok_forms[] = {"content cart diag-cpu-1.ivsc\nexpect-load cart IVS_E_MALFORMED\n",
                                         "content cart diag-cpu-1.ivsc\nexpect-load cart IVS_E_MALFORMED reason 1\n"};
  char diag[RX_SCN_CAP];
  char scn[RX_SCN_CAP];
  fake_host h;
  if (!rx_diag(diag, sizeof diag)) {
    return IVS_TEST_FAIL;
  }
  for (size_t i = 0u; i < 2u; i++) {
    int ok;
    fh_init(&h);
    h.files[1].len = 16u; /* IVSC reason 1: truncated */
    if (!rx_replace(diag, "content cart diag-cpu-1.ivsc\n", ok_forms[i], scn, sizeof scn)) {
      return IVS_TEST_FAIL;
    }
    ok = rx_expect(scn, &h, IVS_RUNNER_OUTCOME_PASS, 0u) &&
         rx_is(h.out, "outcome.detail", "load-refused-as-expected role=cart status=IVS_E_MALFORMED reason=1") &&
         rx_is(h.out, "slot.cart.status", "IVS_E_MALFORMED") && rx_is(h.out, "slot.cart.reason", "1") &&
         rx_is(h.out, "step.1.now_ticks", "-") && rx_is(h.out, "step.1.assert.1", "not-run halted == yes got -");
    rx_free(&h);
    if (!ok) {
      return IVS_TEST_FAIL;
    }
  }
  fh_init(&h);
  h.files[1].len = 16u;
  if (!rx_replace(diag, "content cart diag-cpu-1.ivsc\n",
                  "content cart diag-cpu-1.ivsc\nexpect-load cart IVS_E_MALFORMED reason 3\n", scn, sizeof scn) ||
      !rx_expect(scn, &h, IVS_RUNNER_OUTCOME_HARNESS_FAIL, 14u) ||
      !rx_starts(h.out, "outcome.detail", "load-refused role=cart status=IVS_E_MALFORMED reason=1")) {
    rx_free(&h);
    return IVS_TEST_FAIL;
  }
  rx_free(&h);
  fh_init(&h);
  if (!rx_replace(diag, "content cart diag-cpu-1.ivsc\n", ok_forms[0], scn, sizeof scn) ||
      !rx_expect(scn, &h, IVS_RUNNER_OUTCOME_HARNESS_FAIL, 14u) ||
      !rx_is(h.out, "outcome.detail", "load-not-refused role=cart expected=IVS_E_MALFORMED")) {
    rx_free(&h);
    return IVS_TEST_FAIL;
  }
  rx_free(&h);
  return IVS_TEST_PASS;
}

/* expect-sha256 is the core's DIGEST_MISMATCH refusal: harness-fail, or pass when expect-load
 * names it. */
static int ex_sha_mismatch(void)
{
  static const char zero[] = "0000000000000000000000000000000000000000000000000000000000000000";
  char diag[RX_SCN_CAP];
  char line[256];
  char scn[RX_SCN_CAP];
  fake_host h;
  int rc = IVS_TEST_FAIL;
  if (!rx_diag(diag, sizeof diag)) {
    return IVS_TEST_FAIL;
  }
  (void)snprintf(line, sizeof line, "content cart diag-cpu-1.ivsc\nexpect-sha256 cart %s\n", zero);
  fh_init(&h);
  if (!rx_replace(diag, "content cart diag-cpu-1.ivsc\n", line, scn, sizeof scn) ||
      !rx_expect(scn, &h, IVS_RUNNER_OUTCOME_HARNESS_FAIL, 14u) ||
      !rx_is(h.out, "outcome.detail", "load-refused role=cart status=IVS_E_DIGEST_MISMATCH reason=24")) {
    goto done;
  }
  rx_free(&h);
  (void)snprintf(line, sizeof line, "content cart diag-cpu-1.ivsc\nexpect-sha256 cart %s\nexpect-load cart IVS_E_DIGEST_MISMATCH\n",
                 zero);
  fh_init(&h);
  if (rx_replace(diag, "content cart diag-cpu-1.ivsc\n", line, scn, sizeof scn) &&
      rx_expect(scn, &h, IVS_RUNNER_OUTCOME_PASS, 0u) &&
      rx_is(h.out, "outcome.detail", "load-refused-as-expected role=cart status=IVS_E_DIGEST_MISMATCH reason=24")) {
    rc = IVS_TEST_PASS;
  }
done:
  rx_free(&h);
  return rc;
}

/* expect-sha256 with the cartridge's own digest (from a first run's report) loads and passes. */
static int ex_sha_match(void)
{
  char diag[RX_SCN_CAP];
  char sha[RX_VALUE_CAP];
  char line[256];
  char scn[RX_SCN_CAP];
  fake_host h;
  int rc = IVS_TEST_FAIL;
  if (!rx_diag(diag, sizeof diag)) {
    return IVS_TEST_FAIL;
  }
  fh_init(&h);
  if (!rx_expect(diag, &h, IVS_RUNNER_OUTCOME_PASS, 0u) || !rx_value(h.out, "slot.cart.sha256", sha, sizeof sha) ||
      !rx_hex64(sha)) {
    goto done;
  }
  rx_free(&h);
  (void)snprintf(line, sizeof line, "content cart diag-cpu-1.ivsc\nexpect-sha256 cart %s\n", sha);
  fh_init(&h);
  if (rx_replace(diag, "content cart diag-cpu-1.ivsc\n", line, scn, sizeof scn) &&
      rx_expect(scn, &h, IVS_RUNNER_OUTCOME_PASS, 0u) && rx_is(h.out, "slot.cart.sha256", sha)) {
    rc = IVS_TEST_PASS;
  }
done:
  rx_free(&h);
  return rc;
}

/* A cartridge file above the bound: the host returns cap + 1 bytes and the core's own OVERSIZED
 * refusal decides (harness-fail). */
static int ex_too_large(void)
{
  fake_host h;
  char diag[RX_SCN_CAP];
  size_t n = IVS_RUNNER_CAP_CART + 100u;
  uint8_t *big = calloc(n, 1u);
  int rc = IVS_TEST_FAIL;
  if (big == NULL || !rx_diag(diag, sizeof diag)) {
    free(big);
    return IVS_TEST_FAIL;
  }
  memcpy(big, ivs_img_diag_cpu_1, IVS_IMG_DIAG_CPU_1_BYTES);
  fh_init(&h);
  h.files[1].bytes = big;
  h.files[1].len = n;
  if (rx_expect(diag, &h, IVS_RUNNER_OUTCOME_HARNESS_FAIL, 14u) &&
      rx_starts(h.out, "outcome.detail", "load-refused role=cart status=IVS_E_OVERSIZED reason=")) {
    rc = IVS_TEST_PASS;
  }
  rx_free(&h);
  free(big);
  return rc;
}

/* An unknown profile: harness-fail, and the report still has every key. */
static int ex_unknown_profile(void)
{
  fake_host h;
  char diag[RX_SCN_CAP];
  char scn[RX_SCN_CAP];
  int rc = IVS_TEST_FAIL;
  if (!rx_diag(diag, sizeof diag) ||
      !rx_replace(diag, "profile intv-2609-ntsc-family-p1", "profile no-such-profile", scn, sizeof scn)) {
    return IVS_TEST_FAIL;
  }
  fh_init(&h);
  if (rx_expect(scn, &h, IVS_RUNNER_OUTCOME_HARNESS_FAIL, 14u) &&
      rx_is(h.out, "outcome.detail", "profile-refused status=IVS_E_UNKNOWN_PROFILE") &&
      rx_is(h.out, "cpu.run_state", "-") && rx_is(h.out, "digest.cpu", "-")) {
    rc = IVS_TEST_PASS;
  }
  rx_free(&h);
  return rc;
}

/* Cancellation asked before the first step: cancelled, exit 12. */
static int ex_cancel(void)
{
  fake_host h;
  char diag[RX_SCN_CAP];
  int rc = IVS_TEST_FAIL;
  if (!rx_diag(diag, sizeof diag)) {
    return IVS_TEST_FAIL;
  }
  fh_init(&h);
  h.cancel = 1;
  if (rx_expect(diag, &h, IVS_RUNNER_OUTCOME_CANCELLED, 12u) && rx_is(h.out, "outcome.detail", "signal step=1") &&
      rx_is(h.out, "step.1.now_ticks", "-")) {
    rc = IVS_TEST_PASS;
  }
  rx_free(&h);
  return rc;
}

/* A failing report write: harness-fail in the result whatever the report says. */
static int ex_write_error(void)
{
  fake_host h;
  ivs_runner_result res;
  char diag[RX_SCN_CAP];
  uint32_t out;
  if (!rx_diag(diag, sizeof diag)) {
    return IVS_TEST_FAIL;
  }
  fh_init(&h);
  h.fail_write = 1;
  out = rx_exec(diag, &h, &res);
  rx_free(&h);
  IVS_CHECK(out == IVS_RUNNER_OUTCOME_HARNESS_FAIL);
  IVS_CHECK(res.outcome == IVS_RUNNER_OUTCOME_HARNESS_FAIL && res.exit_code == 14u && res.report_write_failed == 1u);
  return IVS_TEST_PASS;
}

/* Two runs of one scenario give byte-identical reports apart from host.calls. */
static int ex_repeat_identical(void)
{
  fake_host a;
  fake_host b;
  char diag[RX_SCN_CAP];
  char *pa;
  char *pb;
  int rc = IVS_TEST_FAIL;
  if (!rx_diag(diag, sizeof diag)) {
    return IVS_TEST_FAIL;
  }
  fh_init(&a);
  fh_init(&b);
  if (!rx_expect(diag, &a, IVS_RUNNER_OUTCOME_PASS, 0u) || !rx_expect(diag, &b, IVS_RUNNER_OUTCOME_PASS, 0u)) {
    goto done;
  }
  /* Cut both at host.calls; the lines before it must be equal byte for byte. */
  pa = strstr(a.out, "\nhost.calls ");
  pb = strstr(b.out, "\nhost.calls ");
  if (pa == NULL || pb == NULL || pa - a.out != pb - b.out || memcmp(a.out, b.out, (size_t)(pa - a.out)) != 0) {
    ivs_test_diag("reports differ before host.calls");
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  rx_free(&a);
  rx_free(&b);
  return rc;
}

/* The key order of the diagnostic report, written out here independently of report.c's table. */
static int ex_key_order(void)
{
  static const char *const head[] = {"ivsr",           "scenario.id", "scenario.profile", "api.version",
                                     "outcome",        "outcome.detail", "exit.code"};
  static const char *const roles[] = {"exec", "grom", "cart"};
  static const char *const slot[] = {"loaded", "length_bytes", "sha256", "status", "reason"};
  static const char *const step[] = {"target_ticks", "now_ticks", "overshoot_ticks", "stop_reason", "expected_stop"};
  static const char *const tail[] = {"cpu.run_state",      "cpu.fault_reason",   "input.events",
                                     "input.consumed",     "input.controller.0", "input.controller.1",
                                     "input.controller.2", "input.controller.3", "digest.cpu",
                                     "digest.mem",         "digest.images",      "digest.instance",
                                     "digest.composite",   "digest.video",       "digest.audio",
                                     "digest.input",       "output.video.bytes", "output.audio.bytes",
                                     "host.calls",         "report.end"};
  char want[128][40];
  size_t n = 0u;
  fake_host h;
  char diag[RX_SCN_CAP];
  const char *p;
  size_t i = 0u;
  int rc = IVS_TEST_FAIL;
  for (size_t k = 0u; k < sizeof head / sizeof head[0]; k++) {
    (void)snprintf(want[n++], sizeof want[0], "%s", head[k]);
  }
  for (size_t r = 0u; r < 3u; r++) {
    for (size_t k = 0u; k < 5u; k++) {
      (void)snprintf(want[n++], sizeof want[0], "slot.%s.%s", roles[r], slot[k]);
    }
  }
  (void)snprintf(want[n++], sizeof want[0], "exec:");
  (void)snprintf(want[n++], sizeof want[0], "grom:");
  for (size_t k = 0u; k < 5u; k++) {
    (void)snprintf(want[n++], sizeof want[0], "step.1.%s", step[k]);
  }
  for (unsigned j = 1u; j <= 18u; j++) {
    (void)snprintf(want[n++], sizeof want[0], "step.1.assert.%u", j);
  }
  for (size_t k = 0u; k < sizeof tail / sizeof tail[0]; k++) {
    (void)snprintf(want[n++], sizeof want[0], "%s", tail[k]);
  }
  if (!rx_diag(diag, sizeof diag)) {
    return IVS_TEST_FAIL;
  }
  fh_init(&h);
  if (!rx_expect(diag, &h, IVS_RUNNER_OUTCOME_PASS, 0u)) {
    goto done;
  }
  for (p = h.out; *p != '\0'; i++) {
    const char *sp = strchr(p, ' ');
    const char *eol = strchr(p, '\n');
    size_t kl;
    if (sp == NULL || eol == NULL || sp > eol || i >= n) {
      ivs_test_diag("line %zu: no `key value` form or more lines than keys", i + 1u);
      goto done;
    }
    kl = (size_t)(sp - p);
    if (kl != strlen(want[i]) || strncmp(p, want[i], kl) != 0) {
      ivs_test_diag("line %zu: key '%.*s', want '%s'", i + 1u, (int)kl, p, want[i]);
      goto done;
    }
    p = eol + 1;
  }
  if (i != n) {
    ivs_test_diag("%zu lines, want %zu", i, n);
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  rx_free(&h);
  return rc;
}

/* No host path or other free text: every byte is printable ASCII or LF, no `/` anywhere (the
 * scenarios here name no subdirectory), single-space `key value` lines, and no clock or
 * environment key (the key set is the closed one of ex_key_order). Checked on a pass and on a
 * missing-fixture report, whose detail names the relative content path. */
static int ex_no_paths(void)
{
  char diag[RX_SCN_CAP];
  if (!rx_diag(diag, sizeof diag)) {
    return IVS_TEST_FAIL;
  }
  for (int variant = 0; variant < 2; variant++) {
    fake_host h;
    int ok;
    fh_init(&h);
    if (variant == 1) {
      h.n_files = 1u;
    }
    ok = rx_expect(diag, &h, variant == 0 ? IVS_RUNNER_OUTCOME_PASS : IVS_RUNNER_OUTCOME_MISSING_FIXTURE,
                   variant == 0 ? 0u : 13u);
    for (const char *p = ok ? h.out : ""; ok && *p != '\0'; p++) {
      unsigned char c = (unsigned char)*p;
      if (c == '/' || (c != '\n' && (c < 0x20u || c > 0x7Eu)) || (c == ' ' && (p[1] == ' ' || p[1] == '\n'))) {
        ivs_test_diag("variant %d: byte 0x%02X at offset %zu", variant, (unsigned)c, (size_t)(p - h.out));
        ok = 0;
      }
    }
    if (ok && (strstr(h.out, "time") != NULL || strstr(h.out, "wall") != NULL || strstr(h.out, "pid") != NULL)) {
      ivs_test_diag("variant %d: a clock or process word in the report", variant);
      ok = 0;
    }
    rx_free(&h);
    if (!ok) {
      return IVS_TEST_FAIL;
    }
  }
  return IVS_TEST_PASS;
}

/* An input schedule (at_cycle, converted through the timebase) changes only the INPUT digest and
 * the input lines. */
static int ex_input_schedule(void)
{
  static const char *const may_differ[] = {"digest.input",       "input.events",       "input.consumed",
                                           "input.controller.0", "input.controller.1", "input.controller.2",
                                           "input.controller.3", "host.calls"};
  fake_host a;
  fake_host b;
  char diag[RX_SCN_CAP];
  char scn[RX_SCN_CAP];
  const char *pa;
  const char *pb;
  char va[RX_VALUE_CAP];
  char vb[RX_VALUE_CAP];
  int rc = IVS_TEST_FAIL;
  if (!rx_diag(diag, sizeof diag) ||
      !rx_replace(diag, "run-to 4000", "input at_cycle=10 c0=KEY_1 c1=0x0\ninput at_cycle=100 c0=DISC_N+SIDE_TOP c1=KEY_ENTER\nrun-to 4000",
                  scn, sizeof scn)) {
    return IVS_TEST_FAIL;
  }
  fh_init(&a);
  fh_init(&b);
  if (!rx_expect(diag, &a, IVS_RUNNER_OUTCOME_PASS, 0u) || !rx_expect(scn, &b, IVS_RUNNER_OUTCOME_PASS, 0u) ||
      !rx_is(b.out, "input.events", "2") || !rx_is(b.out, "input.consumed", "2") ||
      !rx_is(b.out, "input.controller.0", "10001000") || !rx_is(b.out, "input.controller.1", "00000800") ||
      !rx_is(b.out, "input.controller.2", "00000000") || !rx_is(a.out, "input.controller.0", "00000000")) {
    goto done;
  }
  if (!rx_value(a.out, "digest.input", va, sizeof va) || !rx_value(b.out, "digest.input", vb, sizeof vb) ||
      strcmp(va, vb) == 0) {
    ivs_test_diag("the input digest did not change");
    goto done;
  }
  /* Line by line: same keys; a differing value only under a key of may_differ. */
  pa = a.out;
  pb = b.out;
  while (*pa != '\0' && *pb != '\0') {
    const char *ea = strchr(pa, '\n');
    const char *eb = strchr(pb, '\n');
    size_t la = (size_t)(ea - pa);
    size_t lb = (size_t)(eb - pb);
    if (la != lb || memcmp(pa, pb, la) != 0) {
      int allowed = 0;
      for (size_t k = 0u; k < sizeof may_differ / sizeof may_differ[0]; k++) {
        size_t kl = strlen(may_differ[k]);
        if (strncmp(pa, may_differ[k], kl) == 0 && pa[kl] == ' ' && strncmp(pb, may_differ[k], kl) == 0 &&
            pb[kl] == ' ') {
          allowed = 1;
        }
      }
      if (!allowed) {
        ivs_test_diag("line differs: '%.*s' vs '%.*s'", (int)la, pa, (int)lb, pb);
        goto done;
      }
    }
    pa = ea + 1;
    pb = eb + 1;
  }
  if (*pa != '\0' || *pb != '\0') {
    ivs_test_diag("reports have different line counts");
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  rx_free(&a);
  rx_free(&b);
  return rc;
}

/* A target inside a unit: the run ends past it and the overshoot line is now - target, above 0. */
static int ex_overshoot(void)
{
  fake_host h;
  char scn[RX_SCN_CAP];
  char v[RX_VALUE_CAP];
  unsigned long long target;
  unsigned long long now;
  unsigned long long over;
  int rc = IVS_TEST_FAIL;
  if (!rx_scn(scn, sizeof scn, rx_content, "run-to 2 cpu-cycles\n")) {
    return IVS_TEST_FAIL;
  }
  fh_init(&h);
  if (!rx_expect(scn, &h, IVS_RUNNER_OUTCOME_PASS, 0u) || !rx_is(h.out, "step.1.stop_reason", "target")) {
    goto done;
  }
  if (!rx_value(h.out, "step.1.target_ticks", v, sizeof v)) {
    goto done;
  }
  target = strtoull(v, NULL, 10);
  if (!rx_value(h.out, "step.1.now_ticks", v, sizeof v)) {
    goto done;
  }
  now = strtoull(v, NULL, 10);
  if (!rx_value(h.out, "step.1.overshoot_ticks", v, sizeof v)) {
    goto done;
  }
  over = strtoull(v, NULL, 10);
  if (now <= target || over != now - target || over == 0u) {
    ivs_test_diag("target %llu now %llu overshoot %llu", target, now, over);
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  rx_free(&h);
  return rc;
}

/* cpu-cycles and frames bounds are n * ticks_per_cpu_cycle and n * nominal_frame_ticks, and a
 * frames bound is labelled nominal, not frame events (D-09). */
static int ex_units(void)
{
  fake_host h;
  char diag[RX_SCN_CAP];
  char scn[RX_SCN_CAP];
  char want[RX_VALUE_CAP];
  uint64_t tpc = 0u;
  uint64_t nft = 0u;
  int ok;
  if (!rx_timebase(&tpc, &nft) || !rx_diag(diag, sizeof diag) ||
      !rx_replace(diag, "run-to 4000 cpu-cycles", "run-to 1 frames", scn, sizeof scn)) {
    return IVS_TEST_FAIL;
  }
  fh_init(&h);
  (void)snprintf(want, sizeof want, "%llu from=4000 cpu-cycles", (unsigned long long)(4000u * tpc));
  ok = rx_expect(diag, &h, IVS_RUNNER_OUTCOME_PASS, 0u) && rx_is(h.out, "step.1.target_ticks", want);
  rx_free(&h);
  if (!ok) {
    return IVS_TEST_FAIL;
  }
  fh_init(&h);
  (void)snprintf(want, sizeof want, "%llu from=1 frames nominal, not frame events", (unsigned long long)nft);
  ok = rx_expect(scn, &h, IVS_RUNNER_OUTCOME_PASS, 0u) && rx_is(h.out, "step.1.target_ticks", want);
  rx_free(&h);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* Two steps: a checkpoint while the guest still runs, then the halt; asserts keep their
 * scenario-wide numbers. A failed assert at step 1 ends the scenario before step 2. */
static int ex_two_steps(void)
{
  fake_host h;
  char diag[RX_SCN_CAP];
  char scn[RX_SCN_CAP];
  char bad[RX_SCN_CAP];
  int rc = IVS_TEST_FAIL;
  if (!rx_diag(diag, sizeof diag) ||
      !rx_replace(diag, "run-to 4000", "run-to 100 cpu-cycles\nassert halted == no\nassert mem 020F != 03C5\nrun-to 4000",
                  scn, sizeof scn) ||
      !rx_replace(scn, "assert halted == no", "assert halted == yes", bad, sizeof bad)) {
    return IVS_TEST_FAIL;
  }
  fh_init(&h);
  if (!rx_expect(scn, &h, IVS_RUNNER_OUTCOME_PASS, 0u) || !rx_is(h.out, "step.1.stop_reason", "target") ||
      !rx_is(h.out, "step.1.expected_stop", "target") || !rx_is(h.out, "step.1.assert.1", "pass halted == no got no") ||
      !rx_is(h.out, "step.1.assert.2", "pass mem 020F != 03C5 got 0000") ||
      !rx_is(h.out, "step.2.stop_reason", "halted") || !rx_is(h.out, "step.2.assert.3", "pass halted == yes got yes") ||
      !rx_is(h.out, "step.2.assert.20", "pass mem 020F == 03C5 got 03C5")) {
    goto done;
  }
  rx_free(&h);
  fh_init(&h);
  if (rx_expect(bad, &h, IVS_RUNNER_OUTCOME_GUEST_FAIL, 10u) &&
      rx_is(h.out, "outcome.detail", "assert=1 line=8 halted == yes got no") &&
      rx_is(h.out, "step.2.now_ticks", "-") && rx_is(h.out, "step.2.assert.3", "not-run halted == yes got -")) {
    rc = IVS_TEST_PASS;
  }
done:
  rx_free(&h);
  return rc;
}

/* The assert forms: != on every subject, a word range, a digest; a range failure names the first
 * word that differs. */
static int ex_assert_forms(void)
{
  fake_host h;
  char diag[RX_SCN_CAP];
  char cpu[RX_VALUE_CAP];
  char extra[1024];
  char scn[RX_SCN_CAP];
  int rc = IVS_TEST_FAIL;
  if (!rx_diag(diag, sizeof diag)) {
    return IVS_TEST_FAIL;
  }
  fh_init(&h);
  if (!rx_expect(diag, &h, IVS_RUNNER_OUTCOME_PASS, 0u) || !rx_value(h.out, "digest.cpu", cpu, sizeof cpu)) {
    goto done;
  }
  rx_free(&h);
  (void)snprintf(extra, sizeof extra,
                 "assert halted != no\nassert reg R0 != 0000\nassert mem 0209..020E == 0000\n"
                 "assert mem 0200..0203 != 7FFF\nassert digest cpu == %s\nassert mem 0200 != 0000\n",
                 cpu);
  if (!rx_replace(diag, "assert halted == yes\n", extra, scn, sizeof scn)) {
    goto done;
  }
  fh_init(&h);
  if (!rx_expect(scn, &h, IVS_RUNNER_OUTCOME_PASS, 0u) ||
      !rx_is(h.out, "step.1.assert.3", "pass mem 0209..020E == 0000 got 0000") ||
      !rx_is(h.out, "step.1.assert.4", "pass mem 0200..0203 != 7FFF got 0201=8080") ||
      !rx_starts(h.out, "step.1.assert.5", "pass digest cpu == ")) {
    goto done;
  }
  rx_free(&h);
  (void)snprintf(extra, sizeof extra, "assert mem 0200..0203 == 7FFF\nassert digest cpu != %s\n", cpu);
  if (!rx_replace(diag, "assert halted == yes\n", extra, scn, sizeof scn)) {
    goto done;
  }
  fh_init(&h);
  if (rx_expect(scn, &h, IVS_RUNNER_OUTCOME_GUEST_FAIL, 10u) &&
      rx_is(h.out, "outcome.detail", "assert=1 line=8 mem 0200..0203 == 7FFF got 0201=8080") &&
      rx_starts(h.out, "step.1.assert.2", "fail digest cpu != ")) {
    rc = IVS_TEST_PASS;
  }
done:
  rx_free(&h);
  return rc;
}

static const ivs_test_case exec_cases[] = {
    {"runner.exec.assert-forms", ex_assert_forms, NULL},
    {"runner.exec.cancel", ex_cancel, NULL},
    {"runner.exec.diag-pass", ex_diag_pass, NULL},
    {"runner.exec.expect-load-refusal", ex_expect_load_refusal, NULL},
    {"runner.exec.guest-assert-mismatch", ex_guest_assert_mismatch, NULL},
    {"runner.exec.halt-bound-small", ex_halt_bound_small, NULL},
    {"runner.exec.input-schedule", ex_input_schedule, NULL},
    {"runner.exec.io-error", ex_io_error, NULL},
    {"runner.exec.key-order", ex_key_order, NULL},
    {"runner.exec.missing-file", ex_missing_file, NULL},
    {"runner.exec.no-paths", ex_no_paths, NULL},
    {"runner.exec.not-regular", ex_not_regular, NULL},
    {"runner.exec.overshoot", ex_overshoot, NULL},
    {"runner.exec.repeat-identical", ex_repeat_identical, NULL},
    {"runner.exec.sha-match", ex_sha_match, NULL},
    {"runner.exec.sha-mismatch", ex_sha_mismatch, NULL},
    {"runner.exec.stop-internal", ex_stop_internal, NULL},
    {"runner.exec.stop-mismatch", ex_stop_mismatch, NULL},
    {"runner.exec.too-large", ex_too_large, NULL},
    {"runner.exec.two-steps", ex_two_steps, NULL},
    {"runner.exec.units", ex_units, NULL},
    {"runner.exec.unexpected-refusal", ex_unexpected_refusal, NULL},
    {"runner.exec.unknown-profile", ex_unknown_profile, NULL},
    {"runner.exec.user-role", ex_user_role, NULL},
    {"runner.exec.write-error", ex_write_error, NULL},
};

#define EXEC_CASES (sizeof exec_cases / sizeof exec_cases[0])

size_t runner_exec_count(void) { return EXEC_CASES; }

const char *runner_exec_id(size_t i) { return i < EXEC_CASES ? exec_cases[i].id : NULL; }

int runner_exec_run(size_t i) { return i < EXEC_CASES ? exec_cases[i].fn() : IVS_TEST_FAIL; }
