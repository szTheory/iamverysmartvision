/* SPDX-License-Identifier: MIT */
/* Suite runner, family exec (D-17..D-20, D-22, D-30, plan 04-18): the scenario executor and the
 * report writer of the runner library, in process, through a fake host vtable. The fake host
 * serves boot-stub.exec and diag-cpu-1.ivsc from the build-tree image include (ivs_images.inc,
 * written by tools/pack; nothing committed) and collects the report in memory; it has no clock
 * and no file system.
 *   runner.exec.diag-pass  the diag-boot-pass scenario (the 18 assert lines of
 *                          data/golden/diag-cpu-1.expect, read through diag_cpu_1_expect_text.inc)
 *                          runs to pass, exit 0, with the label lines and a complete report. */
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

static const ivs_test_case exec_cases[] = {
    {"runner.exec.diag-pass", ex_diag_pass, NULL},
};

#define EXEC_CASES (sizeof exec_cases / sizeof exec_cases[0])

size_t runner_exec_count(void) { return EXEC_CASES; }

const char *runner_exec_id(size_t i) { return i < EXEC_CASES ? exec_cases[i].id : NULL; }

int runner_exec_run(size_t i) { return i < EXEC_CASES ? exec_cases[i].fn() : IVS_TEST_FAIL; }
