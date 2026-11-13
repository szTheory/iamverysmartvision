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
 *   frames sugar), input-schedule (only the input lines and the INPUT, MEM and composite digests change),
 *   two-steps and assert-forms. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_refpcm.h"
#include "ivs_runner.h"
#include "ivs_test.h"
#include "runner_cases.h"
#include "runner_chain.h"
#include "runner_sha256.h"
#include "runner_vchain.h"

#include "ivs_images.inc"

#define FH_MAX_FILES 4u
#define FH_OUT_CAP (256u * 1024u)
#define FH_STORAGE (1024u * 1024u)
#define RX_SCN_CAP 8192u
#define RX_VALUE_CAP 512u
#define RX_SCN_CAPTURE_CAP 4096u

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
  size_t buffer_bytes; /* the host's output block (0: none, so no channel is requested) */
  uint8_t *buffers;
  int corrupt_audio;   /* the audio hook flips one delivered byte before the chain check reads it */
  uint64_t raw_bytes;  /* bytes the audio hook received */
  uint32_t raw_calls;
  int want_log;        /* install the PSG log hook */
  int no_audio_hook;   /* leave the raw audio hook out */
  char log_text[4096]; /* the lines the log hook received */
  size_t log_len;
  uint64_t out_max;    /* options.audio_out_max_bytes (0: the default) */
  int want_wav;        /* install the operator WAV hooks (plan 05-10) */
  uint32_t wav_rate;   /* options.wav_rate (0: the default 48000; an invalid value must be refused) */
  uint64_t slice_ticks; /* options.slice_ticks (0: the default) */
  int wav_fail_pcm;    /* the PCM hook fails */
  int wav_fail_finish; /* the finish hook fails */
  uint8_t wav_pcm[8192]; /* the PCM bytes the hook received, in order */
  size_t wav_pcm_len;
  uint8_t wav_hdr[44]; /* the header the finish hook received */
  uint32_t wav_finishes;
  uint8_t wav_sha[32]; /* what the finish hook returned: SHA-256 of header and PCM as a file */
  int want_vraw;       /* install the raw video hook (plan 06-12) */
  int corrupt_video;   /* the video hook flips one delivered byte before the chain check reads it */
  uint64_t vraw_bytes; /* bytes the video hook received */
  uint64_t video_out_max; /* options.video_out_max_bytes (0: the default) */
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

/* The operator-only raw video hook (plan 06-12): counts what it receives and, when asked, flips the low bit of the first
 * delivered byte in the host's own block (the executor then reads a corrupted stream). */
static int fh_video_raw(void *ctx, const uint8_t *bytes, size_t len)
{
  fake_host *h = ctx;
  h->vraw_bytes += len;
  if (h->corrupt_video && h->buffers != NULL && bytes >= h->buffers) {
    h->buffers[bytes - h->buffers] ^= 0x01u;
  }
  return 0;
}

/* The operator-only raw audio hook of the vtable: counts what it receives and, when asked, flips the
 * low bit of the first byte in the host's own block (the executor then reads a corrupted stream). */
static int fh_audio_raw(void *ctx, const uint8_t *bytes, size_t len)
{
  fake_host *h = ctx;
  h->raw_bytes += len;
  h->raw_calls++;
  if (h->corrupt_audio && h->buffers != NULL && bytes >= h->buffers) {
    h->buffers[bytes - h->buffers] ^= 0x01u;
  }
  return 0;
}

/* The operator WAV hooks of the vtable: keep the PCM, and at the end answer with the SHA-256 of header
 * followed by PCM (the host writing the file as the library asked). */
static int fh_wav_pcm(void *ctx, const uint8_t *pcm, size_t len)
{
  fake_host *h = ctx;
  if (h->wav_fail_pcm || len > sizeof h->wav_pcm - h->wav_pcm_len) {
    return -1;
  }
  memcpy(h->wav_pcm + h->wav_pcm_len, pcm, len);
  h->wav_pcm_len += len;
  return 0;
}

static int fh_wav_finish(void *ctx, const uint8_t header[44], uint8_t sha256[32])
{
  fake_host *h = ctx;
  ivs_rsha256 c;
  h->wav_finishes++;
  if (h->wav_fail_finish) {
    return -1;
  }
  memcpy(h->wav_hdr, header, 44u);
  ivs_rsha256_init(&c);
  ivs_rsha256_update(&c, header, 44u);
  ivs_rsha256_update(&c, h->wav_pcm, h->wav_pcm_len);
  ivs_rsha256_final(&c, h->wav_sha);
  memcpy(sha256, h->wav_sha, 32u);
  return 0;
}

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

static int fh_psg_log(void *ctx, const char *lines, size_t len)
{
  fake_host *h = ctx;
  if (len > sizeof h->log_text - 1u - h->log_len) {
    return -1;
  }
  memcpy(h->log_text + h->log_len, lines, len);
  h->log_len += len;
  h->log_text[h->log_len] = '\0';
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
  if (!h->no_audio_hook) {
    host.write_audio_raw = fh_audio_raw;
  }
  if (h->want_log) {
    host.write_psg_log = fh_psg_log;
  }
  if (h->want_vraw) {
    host.write_video_raw = fh_video_raw;
  }
  if (h->want_wav) {
    host.write_wav_pcm = fh_wav_pcm;
    host.finish_wav = fh_wav_finish;
  }
  host.storage = storage;
  host.storage_bytes = FH_STORAGE;
  if (h->buffer_bytes != 0u) {
    h->buffers = malloc(h->buffer_bytes);
    if (h->buffers == NULL) {
      goto done;
    }
    memset(h->buffers, 0xCC, h->buffer_bytes);
    host.buffers = h->buffers;
    host.buffer_bytes = h->buffer_bytes;
  }
  if (h->out_max != 0u || h->wav_rate != 0u || h->slice_ticks != 0u || h->video_out_max != 0u) {
    ivs_runner_options o;
    memset(&o, 0, sizeof o);
    o.audio_out_max_bytes = h->out_max;
    o.video_out_max_bytes = h->video_out_max;
    o.wav_rate = h->wav_rate;
    o.slice_ticks = h->slice_ticks;
    out = ivs_runner_execute(s, &host, &o, res);
  } else {
    out = ivs_runner_execute(s, &host, NULL, res);
  }
done:
  free(h->buffers);
  h->buffers = NULL;
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

/* The report is complete: first line `ivsr 3`, last `report.end <lines before it>`, LF ended. */
static int rx_complete(const char *report, const ivs_runner_result *res)
{
  char v[RX_VALUE_CAP];
  size_t n = rx_lines(report);
  size_t len = strlen(report);
  if (strncmp(report, "ivsr 3\n", 7u) != 0 || len == 0u || report[len - 1u] != '\n') {
    ivs_test_diag("report does not start with ivsr 3 or end with LF");
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
  char want[RX_VALUE_CAP + 64u];
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
 * reason); a wrong reason, or a load that was not refused, is harness-fail. The scenario has
 * nothing after the refused load (review WR-02: the parser refuses a run-to there), so the report
 * has no step keys. */
static int ex_expect_load_refusal(void)
{
  static const char *const ok_forms[] = {"content cart diag-cpu-1.ivsc\nexpect-load cart IVS_E_MALFORMED\n",
                                         "content cart diag-cpu-1.ivsc\nexpect-load cart IVS_E_MALFORMED reason 1\n"};
  char diag[RX_SCN_CAP];
  char scn[RX_SCN_CAP];
  fake_host h;
  if (!rx_scn(diag, sizeof diag, rx_content, "")) {
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
         strstr(h.out, "\nstep.") == NULL;
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
  /* The expected refusal ends the scenario: no run-to after it (review WR-02). */
  if (rx_scn(diag, sizeof diag, rx_content, "") &&
      rx_replace(diag, "content cart diag-cpu-1.ivsc\n", line, scn, sizeof scn) &&
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
  char line[RX_VALUE_CAP + 128u];
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

/* The key order of the diagnostic report, written out here independently of report.c's table
 * (ivsr 3, plan 06-12, five video keys after audio.chain.check; ivsr 2 was plan 05-09): with no output block (the keys are all present, their values `-` or 0) and
 * with the block ivs-run gives (the same keys in the same order). */
static int ex_key_order_with(size_t buffer_bytes)
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
                                     "digest.input",       "digest.psglog",      "output.video.bytes",
                                     "output.audio.bytes", "output.audio.samples", "output.audio.changes",
                                     "output.psglog.records", "output.psglog.bytes", "audio.sample_ticks",
                                     "audio.chain.check",  "output.video.frames", "video.geometry",
                                     "video.row_lines",    "video.frame.last_tick", "video.chain.check",
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
  h.buffer_bytes = buffer_bytes;
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

static int ex_key_order(void) { return ex_key_order_with(0u); }

/* ivsr 3 (plan 06-12; the audio keys of ivsr 2 are plan 05-09, D-20): the first line is `ivsr 3`, the key table is the pinned one also with a
 * real output block, and the new keys carry their values: the samples are the delivered bytes over 4,
 * the log is empty for the diagnostic (it writes no PSG register), the sample period is the profile's 16
 * and the audio chain check ran and passed. */
static int ex_ivsr2_keys(void)
{
  fake_host h;
  char diag[RX_SCN_CAP];
  char v[RX_VALUE_CAP];
  int rc = IVS_TEST_FAIL;
  unsigned long long bytes;
  if (ex_key_order_with(IVS_RUNNER_BUFFERS_MAX) != IVS_TEST_PASS || !rx_diag(diag, sizeof diag)) {
    return IVS_TEST_FAIL;
  }
  fh_init(&h);
  h.buffer_bytes = IVS_RUNNER_BUFFERS_MAX;
  if (!rx_expect(diag, &h, IVS_RUNNER_OUTCOME_PASS, 0u)) {
    goto done;
  }
  if (strncmp(h.out, "ivsr 3\n", 7u) != 0 || !rx_value(h.out, "output.audio.bytes", v, sizeof v)) {
    goto done;
  }
  bytes = strtoull(v, NULL, 10);
  if (bytes == 0u || bytes % 4u != 0u || !rx_value(h.out, "output.audio.samples", v, sizeof v) ||
      strtoull(v, NULL, 10) * 4u != bytes) {
    ivs_test_diag("output.audio.bytes %llu, samples '%s'", bytes, v);
    goto done;
  }
  if (!rx_is(h.out, "output.audio.changes", "0") || !rx_is(h.out, "output.psglog.records", "0") ||
      !rx_is(h.out, "output.psglog.bytes", "0") || !rx_is(h.out, "audio.sample_ticks", "16") ||
      !rx_is(h.out, "audio.chain.check", "pass") || !rx_is(h.out, "output.video.bytes", "0")) {
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  rx_free(&h);
  return rc;
}

/* ---- the audio chain check and the SHA-256 copy (plan 05-09, D-18) ---- */

/* Project-authored EXEC stand-in (plan 05-06 encodings: MVII is 02B8 + destination, MVO Rs, a is 0240 +
 * source, the branch to itself is 0220 0001): mixer R7 = $3F (tone and noise of every channel off, so a
 * channel is the constant 1 and its level is its amplitude), then amplitude A = $0F, then $00, then $07,
 * then a branch to itself. Channel A's level goes 0, 15, 0, 7 while B and C stay 0: by construction three
 * level changes (0 to 15, 15 to 0, 0 to 7), each level held for several chip steps (one MVII and MVO pair
 * is 19 CPU cycles, 76 ticks, and a step is 16 ticks). */
static const uint16_t rx_psg_words[] = {0x02B8u, 0x003Fu, 0x0240u, 0x01F8u, 0x02B8u, 0x000Fu, 0x0240u, 0x01FBu,
                                        0x02B8u, 0x0000u, 0x0240u, 0x01FBu, 0x02B8u, 0x0007u, 0x0240u, 0x01FBu,
                                        0x0220u, 0x0001u};
static const char rx_psg_content[] = "content exec psg-prog.exec\n"
                                     "content grom zero-grom\n";
static uint8_t rx_psg_exec[8192];

/* The words as an 8192-byte EXEC image (4096 words, high byte first, the rest zero). */
static void rx_psg_image(void)
{
  memset(rx_psg_exec, 0, sizeof rx_psg_exec);
  for (size_t i = 0u; i < sizeof rx_psg_words / sizeof rx_psg_words[0]; i++) {
    rx_psg_exec[2u * i] = (uint8_t)(rx_psg_words[i] >> 8);
    rx_psg_exec[2u * i + 1u] = (uint8_t)(rx_psg_words[i] & 0xFFu);
  }
}

static int rx_psg_scenario(char *scn, size_t cap, const char *run)
{
  return rx_scn(scn, cap, rx_psg_content, run);
}

/* The stand-in under a real output block: three changes by hand, samples times 4 equal the bytes, the
 * hook saw every delivered byte, and the chain the runner rebuilt equals the core's AUDIO digest
 * (`pass`). With the hook flipping a delivered byte the runner reads a stream the core did not emit:
 * `fail`, harness-fail, detail audio-chain, exit 14 (the check is a comparison, not a label). */
static int ex_audio_chain_check(void)
{
  fake_host h;
  char scn[RX_SCN_CAP];
  char v[RX_VALUE_CAP];
  unsigned long long bytes;
  int rc = IVS_TEST_FAIL;
  rx_psg_image();
  if (!rx_psg_scenario(scn, sizeof scn, "run-to 3000 ticks\n")) {
    return IVS_TEST_FAIL;
  }
  fh_init(&h);
  fh_add(&h, "psg-prog.exec", rx_psg_exec, sizeof rx_psg_exec, IVS_RUNNER_READ_OK);
  h.buffer_bytes = IVS_RUNNER_BUFFERS_MAX;
  if (!rx_expect(scn, &h, IVS_RUNNER_OUTCOME_PASS, 0u)) {
    goto done;
  }
  /* By hand: three level changes; the guest makes four PSG writes (mixer and three amplitudes): four
   * 16-byte log records. */
  if (!rx_is(h.out, "audio.chain.check", "pass") || !rx_is(h.out, "output.audio.changes", "3") ||
      !rx_is(h.out, "output.psglog.records", "4") || !rx_is(h.out, "output.psglog.bytes", "64")) {
    goto done;
  }
  if (!rx_value(h.out, "output.audio.bytes", v, sizeof v)) {
    goto done;
  }
  bytes = strtoull(v, NULL, 10);
  if (bytes == 0u || bytes % 4u != 0u || h.raw_bytes != bytes || !rx_value(h.out, "output.audio.samples", v, sizeof v) ||
      strtoull(v, NULL, 10) * 4u != bytes) {
    ivs_test_diag("audio bytes %llu, hook %llu, samples '%s'", bytes, (unsigned long long)h.raw_bytes, v);
    goto done;
  }
  rx_free(&h);
  /* The same run with a host that corrupts the first delivered byte. */
  fh_init(&h);
  fh_add(&h, "psg-prog.exec", rx_psg_exec, sizeof rx_psg_exec, IVS_RUNNER_READ_OK);
  h.buffer_bytes = IVS_RUNNER_BUFFERS_MAX;
  h.corrupt_audio = 1;
  if (!rx_expect(scn, &h, IVS_RUNNER_OUTCOME_HARNESS_FAIL, 14u)) {
    goto done;
  }
  if (!rx_is(h.out, "audio.chain.check", "fail") || !rx_is(h.out, "outcome.detail", "audio-chain")) {
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  rx_free(&h);
  return rc;
}

/* The runner's SHA-256 copy against FIPS 180-4 and the literals of data/sha256/vectors.txt (the
 * core's own vector file; each digest was typed from `cmake -E sha256sum` and is re-derived by the
 * machine suite's generator): the empty message, `abc`, the 448-bit and 896-bit examples, the padding
 * boundaries 55, 56, 57, 63, 64, 65 bytes of the pat: pattern (byte i is character i mod 62 of
 * A..Z a..z 0..9), and a split feed (one byte at a time equals one call). */
static int rx_unhex(const char *hex, uint8_t out[32])
{
  for (size_t i = 0u; i < 32u; i++) {
    unsigned v = 0u;
    for (size_t k = 0u; k < 2u; k++) {
      char c = hex[2u * i + k];
      unsigned d = c >= '0' && c <= '9' ? (unsigned)(c - '0') : c >= 'a' && c <= 'f' ? (unsigned)(c - 'a') + 10u : 99u;
      if (d > 15u) {
        return 0;
      }
      v = v * 16u + d;
    }
    out[i] = (uint8_t)v;
  }
  return 1;
}

static int ex_sha256_fips(void)
{
  static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  static const struct {
    const char *text;
    size_t pat; /* when text is NULL: the pattern length */
    const char *hex;
  } vec[] = {
      {"", 0u, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"},
      {"abc", 0u, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"},
      {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 0u,
       "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1"},
      {"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
       0u, "cf5b16a778af8380036ce59e7b0492370b249b11e8f07a51afac45037afee9d1"},
      {NULL, 55u, "59521aa5a72bfd087fc7b180efff1e20dc27a7d6232cc1ebb733183d02a8c062"},
      {NULL, 56u, "f50e77559fcfb0be5b298f780857cf0c5f06af725838431f7c9da48ff024ba30"},
      {NULL, 57u, "a70cdc426e03c55f6bab861695327e7c7b032d7da1ed0f1aca0c252125d4e77a"},
      {NULL, 63u, "ef283a28d035e563d2f760d768f74b8529f5771fcd165baa87c27dfc270b1276"},
      {NULL, 64u, "180efa1cd028fb62950d3e535632c471d0dc92486413a656237c971c78cc28a0"},
      {NULL, 65u, "0b2f888a463b8355dce14614389a281d915fc7bf0b43137c21a160fa4a812d6e"},
  };
  uint8_t msg[128];
  for (size_t v = 0u; v < sizeof vec / sizeof vec[0]; v++) {
    uint8_t want[32];
    uint8_t got[32];
    ivs_rsha256 c;
    size_t n;
    if (vec[v].text != NULL) {
      n = strlen(vec[v].text);
      memcpy(msg, vec[v].text, n);
    } else {
      n = vec[v].pat;
      for (size_t i = 0u; i < n; i++) {
        msg[i] = (uint8_t)alphabet[i % 62u];
      }
    }
    IVS_CHECK(n <= sizeof msg && rx_unhex(vec[v].hex, want));
    ivs_rsha256_digest(msg, n, got);
    if (memcmp(got, want, 32u) != 0) {
      ivs_test_diag("vector %zu (%zu bytes): digest differs", v, n);
      return IVS_TEST_FAIL;
    }
    ivs_rsha256_init(&c);
    for (size_t i = 0u; i < n; i++) {
      ivs_rsha256_update(&c, msg + i, 1u);
    }
    ivs_rsha256_final(&c, got);
    if (memcmp(got, want, 32u) != 0) {
      ivs_test_diag("vector %zu: byte-wise feed differs", v);
      return IVS_TEST_FAIL;
    }
  }
  return IVS_TEST_PASS;
}

/* ---- PSG log and audio asserts, captures, digests of the new domains (plan 05-09, D-20) ---- */

/* Runs the stand-in with the extra scenario lines (captures and asserts) under a real output block and
 * returns the outcome check; the report stays in h. */
static int rx_psg_run(fake_host *h, const char *body, uint32_t outcome, uint32_t exit_code)
{
  char scn[RX_SCN_CAPTURE_CAP];
  rx_psg_image();
  if (!rx_scn(scn, sizeof scn, rx_psg_content, body)) {
    return 0;
  }
  fh_init(h);
  fh_add(h, "psg-prog.exec", rx_psg_exec, sizeof rx_psg_exec, IVS_RUNNER_READ_OK);
  h->buffer_bytes = IVS_RUNNER_BUFFERS_MAX;
  return rx_expect(scn, h, outcome, exit_code);
}

/* The stand-in's four PSG writes by hand (cycle tables as in plan 05-08: reset entry 5 CPU cycles,
 * MVII 8, a direct MVO 11 with its write at cycle 6, 4 ticks per cycle): the first MVO starts at
 * (5 + 8) x 4 = 52 and writes at 52 + 24 = 76; each MVII and MVO pair is 19 cycles = 76 ticks, so the
 * writes are at 76, 152, 228, 304, to addresses $01F8 (low nibble 8) then $01FB (11) three times,
 * with the raw values $003F, $000F, $0000, $0007, all source cpu-write. A changed expectation fails
 * guest-fail naming the assert (exit 10); a record that does not exist is a failed assert. */
static int ex_psglog_assert(void)
{
  static const char good[] = "capture psglog\n"
                             "run-to 3000 ticks\n"
                             "assert psglog count == 4\n"
                             "assert psglog count != 5\n"
                             "assert psglog 0 tick=76 addr=8 value=003F source=cpu-write\n"
                             "assert psglog 1 tick=152 addr=11 value=000F source=cpu-write\n"
                             "assert psglog 2 tick=228 addr=11 value=0000\n"
                             "assert psglog 3 tick=304 addr=11 value=0007 source=cpu-write\n"
                             "assert psglog 3 addr=11\n";
  static const char *const bad[][2] = {
      {"tick=304 addr=11 value=0007 source=cpu-write", "tick=305 addr=11 value=0007 source=cpu-write"},
      {"tick=76 addr=8 value=003F", "tick=76 addr=9 value=003F"},
      {"tick=228 addr=11 value=0000", "tick=228 addr=11 value=0001"},
      {"psglog 3 addr=11", "psglog 3 source=cpu-dw"},
      {"psglog count == 4", "psglog count == 3"},
      {"psglog count != 5", "psglog count != 4"},
      {"psglog 3 addr=11", "psglog 4 addr=11"},
  };
  fake_host h;
  char text[2048];
  int rc = IVS_TEST_FAIL;
  if (!rx_psg_run(&h, good, IVS_RUNNER_OUTCOME_PASS, 0u)) {
    goto done;
  }
  if (!rx_is(h.out, "output.psglog.records", "4") || !rx_starts(h.out, "step.1.assert.3", "pass psglog 0 tick=76")) {
    goto done;
  }
  rx_free(&h);
  for (size_t i = 0u; i < sizeof bad / sizeof bad[0]; i++) {
    char want[64];
    if (!rx_replace(good, bad[i][0], bad[i][1], text, sizeof text)) {
      goto done;
    }
    if (!rx_psg_run(&h, text, IVS_RUNNER_OUTCOME_GUEST_FAIL, 10u)) {
      ivs_test_diag("changed expectation %zu did not fail guest-fail", i);
      goto done;
    }
    (void)snprintf(want, sizeof want, "assert=");
    if (!rx_starts(h.out, "outcome.detail", want)) {
      goto done;
    }
    rx_free(&h);
  }
  rc = IVS_TEST_PASS;
done:
  rx_free(&h);
  return rc;
}

/* The levels by hand: writes land at 76 (mixer, all tones and noises off), 152 (A = $0F), 228
 * (A = 0), 304 (A = 7); a sample at tick 16 k sees the writes stamped below 16 k (step first, then
 * write, C78), so A is 0 at 144, 15 at 160 and 224, 0 at 240 and 304, 7 at 320 and at 2992 (the last
 * multiple of 16 below 3000); B and C stay 0. The emitted count of a first run is the sample count the
 * report gives; a changed count, level or tick fails; a tick that is not a multiple of the real sample
 * period is a harness failure (exit 14). */
static int ex_audio_assert(void)
{
  static const char good[] = "capture audio\n"
                             "run-to 3000 ticks\n"
                             "assert audio at 16 A=0 B=0 C=0\n"
                             "assert audio at 144 A=0 B=0 C=0\n"
                             "assert audio at 160 A=15 B=0 C=0\n"
                             "assert audio at 224 A=15 B=0 C=0\n"
                             "assert audio at 240 A=0 B=0 C=0\n"
                             "assert audio at 304 A=0 B=0 C=0\n"
                             "assert audio at 320 A=7 B=0 C=0\n"
                             "assert audio at 2992 A=7 B=0 C=0\n"
                             "assert audio emitted != 0\n";
  fake_host h;
  char text[2048];
  char v[64];
  char count[32];
  int rc = IVS_TEST_FAIL;
  if (!rx_psg_run(&h, good, IVS_RUNNER_OUTCOME_PASS, 0u) || !rx_value(h.out, "output.audio.samples", count, sizeof count)) {
    goto done;
  }
  rx_free(&h);
  (void)snprintf(v, sizeof v, "assert audio emitted == %s", count);
  if (!rx_replace(good, "assert audio emitted != 0", v, text, sizeof text) ||
      !rx_psg_run(&h, text, IVS_RUNNER_OUTCOME_PASS, 0u)) {
    goto done;
  }
  rx_free(&h);
  {
    static const char *const bad[][2] = {
        {"at 160 A=15", "at 160 A=14"},
        {"at 160 A=15 B=0 C=0", "at 144 A=15 B=0 C=0"},
        {"at 320 A=7 B=0 C=0", "at 320 A=7 B=1 C=0"},
        {"at 2992 A=7", "at 2992 A=0"},
        {"emitted != 0", "emitted == 0"},
        {"at 16 A=0 B=0 C=0", "at 160000 A=0 B=0 C=0"},
    };
    for (size_t i = 0u; i < sizeof bad / sizeof bad[0]; i++) {
      if (!rx_replace(good, bad[i][0], bad[i][1], text, sizeof text) ||
          !rx_psg_run(&h, text, IVS_RUNNER_OUTCOME_GUEST_FAIL, 10u)) {
        ivs_test_diag("changed audio expectation %zu did not fail guest-fail", i);
        goto done;
      }
      rx_free(&h);
    }
  }
  if (!rx_replace(good, "at 160 A=15", "at 161 A=15", text, sizeof text) ||
      !rx_psg_run(&h, text, IVS_RUNNER_OUTCOME_HARNESS_FAIL, 14u) ||
      !rx_starts(h.out, "outcome.detail", "audio-tick-misaligned assert=")) {
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  rx_free(&h);
  return rc;
}

/* A program that writes the PSG without end (`MVII #$01FB, R3; MVII #$0F, R0; MVO@ R0, R3; B back`),
 * one record every 16 CPU cycles: past the 4096-entry capture the run ends harness-fail with the detail
 * capture-overflow, never a silent drop; without `capture psglog` the same run passes with its records
 * counted. The program is the second host file. */
static int ex_capture_overflow(void)
{
  static const uint16_t words[] = {0x02BBu, 0x01FBu, 0x02B8u, 0x000Fu, 0x0258u, 0x0220u, 0x0002u};
  static uint8_t image[8192];
  fake_host h;
  char scn[RX_SCN_CAP];
  char v[RX_VALUE_CAP];
  int rc = IVS_TEST_FAIL;
  memset(image, 0, sizeof image);
  for (size_t i = 0u; i < sizeof words / sizeof words[0]; i++) {
    image[2u * i] = (uint8_t)(words[i] >> 8);
    image[2u * i + 1u] = (uint8_t)(words[i] & 0xFFu);
  }
  for (int cap = 1; cap >= 0; cap--) {
    if (!rx_scn(scn, sizeof scn, rx_psg_content,
                cap != 0 ? "capture psglog\nrun-to 400000 ticks\n" : "run-to 400000 ticks\n")) {
      return IVS_TEST_FAIL;
    }
    fh_init(&h);
    fh_add(&h, "psg-prog.exec", image, sizeof image, IVS_RUNNER_READ_OK);
    h.buffer_bytes = IVS_RUNNER_BUFFERS_MAX;
    if (!rx_expect(scn, &h, cap != 0 ? IVS_RUNNER_OUTCOME_HARNESS_FAIL : IVS_RUNNER_OUTCOME_PASS,
                   cap != 0 ? 14u : 0u)) {
      goto done;
    }
    if (cap != 0 && !rx_is(h.out, "outcome.detail", "capture-overflow target=psglog")) {
      goto done;
    }
    if (!rx_value(h.out, "output.psglog.records", v, sizeof v) || strtoull(v, NULL, 10) <= 4096u) {
      ivs_test_diag("records %s, want more than 4096", v);
      goto done;
    }
    rx_free(&h);
  }
  rc = IVS_TEST_PASS;
done:
  rx_free(&h);
  return rc;
}

/* `assert digest audio` and `assert digest psglog` (the existing digest assert with two domain names):
 * the digest a first run reports passes `==` in a second run and fails `!=`; a different digest fails
 * `==`. */
static int ex_digest_assert_audio_psglog(void)
{
  static const char *const dom[2] = {"audio", "psglog"};
  fake_host h;
  char audio[RX_VALUE_CAP];
  char psglog[RX_VALUE_CAP];
  char body[1024];
  int rc = IVS_TEST_FAIL;
  if (!rx_psg_run(&h, "run-to 3000 ticks\n", IVS_RUNNER_OUTCOME_PASS, 0u) ||
      !rx_value(h.out, "digest.audio", audio, sizeof audio) || !rx_value(h.out, "digest.psglog", psglog, sizeof psglog)) {
    goto done;
  }
  rx_free(&h);
  for (size_t d = 0u; d < 2u; d++) {
    const char *hex = d == 0u ? audio : psglog;
    const char *other = d == 0u ? psglog : audio;
    for (int variant = 0; variant < 3; variant++) {
      (void)snprintf(body, sizeof body, "run-to 3000 ticks\nassert digest %s %s %s\n", dom[d],
                     variant == 1 ? "!=" : "==", variant == 2 ? other : hex);
      if (!rx_psg_run(&h, body, variant == 0 ? IVS_RUNNER_OUTCOME_PASS : IVS_RUNNER_OUTCOME_GUEST_FAIL,
                      variant == 0 ? 0u : 10u)) {
        ivs_test_diag("domain %s variant %d", dom[d], variant);
        goto done;
      }
      rx_free(&h);
    }
  }
  rc = IVS_TEST_PASS;
done:
  rx_free(&h);
  return rc;
}

/* The operator hooks (plan 05-09, D-20): the raw audio hook receives exactly the delivered bytes (its
 * length is output.audio.bytes) and the log hook the ASCII lines `index tick addr value source` of
 * every delivered record, one per record: the stand-in's four writes are, by the hand derivation of
 * runner.exec.psglog-assert, `0 76 8 63 1`, `1 152 11 15 1`, `2 228 11 0 1`, `3 304 11 7 1`. With
 * the hooks absent nothing changes in the report. */
static int ex_operator_files_hook(void)
{
  static const char want[] = "0 76 8 63 1\n1 152 11 15 1\n2 228 11 0 1\n3 304 11 7 1\n";
  fake_host h;
  char scn[RX_SCN_CAP];
  char v[RX_VALUE_CAP];
  int rc = IVS_TEST_FAIL;
  rx_psg_image();
  if (!rx_psg_scenario(scn, sizeof scn, "run-to 3000 ticks\n")) {
    return IVS_TEST_FAIL;
  }
  fh_init(&h);
  fh_add(&h, "psg-prog.exec", rx_psg_exec, sizeof rx_psg_exec, IVS_RUNNER_READ_OK);
  h.buffer_bytes = IVS_RUNNER_BUFFERS_MAX;
  h.want_log = 1;
  if (!rx_expect(scn, &h, IVS_RUNNER_OUTCOME_PASS, 0u)) {
    goto done;
  }
  if (!rx_value(h.out, "output.audio.bytes", v, sizeof v) || h.raw_bytes != strtoull(v, NULL, 10) || h.raw_bytes == 0u) {
    ivs_test_diag("raw bytes %llu, output.audio.bytes %s", (unsigned long long)h.raw_bytes, v);
    goto done;
  }
  if (strcmp(h.log_text, want) != 0 || !rx_is(h.out, "output.psglog.records", "4")) {
    ivs_test_diag("log text:\n%s", h.log_text);
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  rx_free(&h);
  return rc;
}

/* The byte bound (--audio-out-max-bytes): a bound below what the run produces is a harness failure
 * (exit 14, detail audio-out-max-bytes), not a truncated success, and a bound exactly at the produced
 * size passes; the same bound applies to the log file on its own (the log hook alone with a bound of 20
 * bytes, below the 48 bytes of the four lines, fails psg-log-out-max-bytes). */
static int ex_out_max_bytes(void)
{
  fake_host h;
  char scn[RX_SCN_CAP];
  char v[RX_VALUE_CAP];
  unsigned long long bytes;
  int rc = IVS_TEST_FAIL;
  rx_psg_image();
  if (!rx_psg_scenario(scn, sizeof scn, "run-to 3000 ticks\n")) {
    return IVS_TEST_FAIL;
  }
  fh_init(&h);
  fh_add(&h, "psg-prog.exec", rx_psg_exec, sizeof rx_psg_exec, IVS_RUNNER_READ_OK);
  h.buffer_bytes = IVS_RUNNER_BUFFERS_MAX;
  if (!rx_expect(scn, &h, IVS_RUNNER_OUTCOME_PASS, 0u) || !rx_value(h.out, "output.audio.bytes", v, sizeof v)) {
    goto done;
  }
  bytes = strtoull(v, NULL, 10);
  rx_free(&h);
  fh_init(&h);
  fh_add(&h, "psg-prog.exec", rx_psg_exec, sizeof rx_psg_exec, IVS_RUNNER_READ_OK);
  h.buffer_bytes = IVS_RUNNER_BUFFERS_MAX;
  h.out_max = bytes;
  if (!rx_expect(scn, &h, IVS_RUNNER_OUTCOME_PASS, 0u) || h.raw_bytes != bytes) {
    goto done;
  }
  rx_free(&h);
  fh_init(&h);
  fh_add(&h, "psg-prog.exec", rx_psg_exec, sizeof rx_psg_exec, IVS_RUNNER_READ_OK);
  h.buffer_bytes = IVS_RUNNER_BUFFERS_MAX;
  h.out_max = bytes - 1u;
  if (!rx_expect(scn, &h, IVS_RUNNER_OUTCOME_HARNESS_FAIL, 14u) || !rx_is(h.out, "outcome.detail", "audio-out-max-bytes") ||
      h.raw_bytes >= bytes) {
    goto done;
  }
  rx_free(&h);
  fh_init(&h);
  fh_add(&h, "psg-prog.exec", rx_psg_exec, sizeof rx_psg_exec, IVS_RUNNER_READ_OK);
  h.buffer_bytes = IVS_RUNNER_BUFFERS_MAX;
  h.want_log = 1;
  h.out_max = bytes; /* the audio file fits; the log file does not exceed it either */
  if (!rx_expect(scn, &h, IVS_RUNNER_OUTCOME_PASS, 0u)) {
    goto done;
  }
  rx_free(&h);
  fh_init(&h);
  fh_add(&h, "psg-prog.exec", rx_psg_exec, sizeof rx_psg_exec, IVS_RUNNER_READ_OK);
  h.buffer_bytes = IVS_RUNNER_BUFFERS_MAX;
  h.want_log = 1;
  h.out_max = 20u;
  h.no_audio_hook = 1;
  if (!rx_expect(scn, &h, IVS_RUNNER_OUTCOME_HARNESS_FAIL, 14u) ||
      !rx_is(h.out, "outcome.detail", "psg-log-out-max-bytes") || h.log_len != 0u) {
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  rx_free(&h);
  return rc;
}

/* ---- the operator WAV (plan 05-10, D-20, D-21) ---- */

/* The keys that end a report, last first: report.end, then (only with the option) the refpcm group right after
 * host.calls. Returns 1 when the last n lines of the report have exactly the keys in want (in order). */
static int rx_tail_is(const char *report, const char *const *want, size_t n)
{
  size_t total = rx_lines(report);
  size_t line = 0u;
  const char *p = report;
  if (total < n) {
    return 0;
  }
  while (*p != '\0') {
    const char *eol = strchr(p, '\n');
    const char *sp = strchr(p, ' ');
    if (eol == NULL) {
      return 0;
    }
    if (line >= total - n) {
      size_t k = line - (total - n);
      size_t kl = strlen(want[k]);
      if (sp == NULL || sp > eol || (size_t)(sp - p) != kl || strncmp(p, want[k], kl) != 0) {
        ivs_test_diag("tail line %zu: key '%.*s', want '%s'", line + 1u, (int)(sp != NULL && sp < eol ? sp - p : eol - p),
                      p, want[k]);
        return 0;
      }
    }
    line++;
    p = eol + 1;
  }
  return 1;
}

/* The refpcm.* lines exist only with the option (the fault RUNNER_REFPCM_KEYS_WITHOUT_OPTION prints them
 * always): none without the hooks, and with them exactly refpcm.rate, refpcm.frames, refpcm.sha256 between
 * host.calls and report.end. */
static int ex_refpcm_keys_only_with_option(void)
{
  static const char *const plain_tail[] = {"video.chain.check", "host.calls", "report.end"};
  static const char *const wav_tail[] = {"host.calls", "refpcm.rate", "refpcm.frames", "refpcm.sha256", "report.end"};
  fake_host h;
  char scn[RX_SCN_CAP];
  int rc = IVS_TEST_FAIL;
  rx_psg_image();
  if (!rx_psg_scenario(scn, sizeof scn, "run-to 3000 ticks\n")) {
    return IVS_TEST_FAIL;
  }
  fh_init(&h);
  fh_add(&h, "psg-prog.exec", rx_psg_exec, sizeof rx_psg_exec, IVS_RUNNER_READ_OK);
  h.buffer_bytes = IVS_RUNNER_BUFFERS_MAX;
  if (!rx_expect(scn, &h, IVS_RUNNER_OUTCOME_PASS, 0u)) {
    goto done;
  }
  if (strstr(h.out, "refpcm.") != NULL || !rx_tail_is(h.out, plain_tail, 3u)) {
    ivs_test_diag("a refpcm. line without the option, or the tail changed");
    goto done;
  }
  rx_free(&h);
  fh_init(&h);
  fh_add(&h, "psg-prog.exec", rx_psg_exec, sizeof rx_psg_exec, IVS_RUNNER_READ_OK);
  h.buffer_bytes = IVS_RUNNER_BUFFERS_MAX;
  h.want_wav = 1;
  if (!rx_expect(scn, &h, IVS_RUNNER_OUTCOME_PASS, 0u) || !rx_tail_is(h.out, wav_tail, 5u) ||
      !rx_is(h.out, "refpcm.rate", "48000")) {
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  rx_free(&h);
  return rc;
}

/* Frames of s samples at rate R by hand: the implicit leading cell makes s + 1 cells of 16 ticks, floor((s +
 * 1) * 16 * R / 3,579,545). */
static unsigned long long rx_wav_frames(unsigned long long samples, unsigned long long rate)
{
  return (samples + 1u) * 16u * rate / 3579545u;
}

/* One WAV run of the stand-in: the report values and what the hooks saw. frames from the report must be the
 * by-hand count, the header the hook received equals ivs_refpcm_wav_header for it, the PCM is twice the
 * frames in bytes and not all zero (the guest raises channel A to level 15), the sha256 key is the hash of
 * header and PCM as a file would hold them. slice (0: default) cuts the run into slices of that many ticks. */
static int rx_wav_run(fake_host *h, const char *scn, uint32_t rate, uint64_t slice)
{
  char v[RX_VALUE_CAP];
  char hex[65];
  uint8_t want_hdr[44];
  unsigned long long samples;
  unsigned long long frames;
  static const char digits[] = "0123456789abcdef";
  int nonzero = 0;
  fh_init(h);
  fh_add(h, "psg-prog.exec", rx_psg_exec, sizeof rx_psg_exec, IVS_RUNNER_READ_OK);
  h->buffer_bytes = IVS_RUNNER_BUFFERS_MAX;
  h->want_wav = 1;
  h->wav_rate = rate == IVS_RUNNER_WAV_RATE_DEFAULT ? 0u : rate;
  h->slice_ticks = slice;
  if (!rx_expect(scn, h, IVS_RUNNER_OUTCOME_PASS, 0u) || !rx_value(h->out, "output.audio.samples", v, sizeof v)) {
    return 0;
  }
  samples = strtoull(v, NULL, 10);
  frames = rx_wav_frames(samples, rate);
  if (!rx_value(h->out, "refpcm.frames", v, sizeof v) || strtoull(v, NULL, 10) != frames || frames == 0u) {
    ivs_test_diag("refpcm.frames '%s', by hand %llu", v, frames);
    return 0;
  }
  (void)snprintf(v, sizeof v, "%u", (unsigned)rate);
  if (!rx_is(h->out, "refpcm.rate", v) || h->wav_finishes != 1u || h->wav_pcm_len != 2u * frames) {
    ivs_test_diag("finishes %u, pcm bytes %zu, want %llu", (unsigned)h->wav_finishes, h->wav_pcm_len, 2u * frames);
    return 0;
  }
  if (ivs_refpcm_wav_header(want_hdr, rate, frames) != IVS_REFPCM_OK || memcmp(want_hdr, h->wav_hdr, 44u) != 0) {
    ivs_test_diag("the header the hook received is not the reference header");
    return 0;
  }
  for (size_t i = 0u; i < 32u; i++) {
    hex[2u * i] = digits[h->wav_sha[i] >> 4];
    hex[2u * i + 1u] = digits[h->wav_sha[i] & 0x0Fu];
  }
  hex[64] = '\0';
  if (!rx_is(h->out, "refpcm.sha256", hex)) {
    return 0;
  }
  for (size_t i = 0u; i < h->wav_pcm_len; i++) {
    nonzero |= h->wav_pcm[i] != 0u;
  }
  if (!nonzero || h->wav_pcm[0] != 0u || h->wav_pcm[1] != 0u) {
    ivs_test_diag("PCM: first frame %02x%02x, nonzero %d", (unsigned)h->wav_pcm[1], (unsigned)h->wav_pcm[0], nonzero);
    return 0;
  }
  return 1;
}

/* runner.exec.buffers-video-slack (plan 06-09, A-03): the runner sizes the video channel as slot x (slice / F + 1 + slack) with the slack
 * read from ivs_info v4, so every slice (1 tick, 7 ticks, the default 4,194,304) runs under the real video layout instead of failing
 * BUFFER_TOO_SMALL. The default slice needs 4,194,304 / 59,736 = 70 + 1 = 71 audio and log slots and 72 video slots: video 72 x 15,296 =
 * 1,101,312 bytes, audio 71 x 15,060 = 1,069,260, log 71 x 34,432 = 2,444,672, in all 4,615,244, re-derived here from the layout's
 * published per-frame figures; a block of exactly that size runs and one byte less is a harness failure buffers-too-small. */
static int ex_buffers_video_slack(void)
{
  static const uint64_t slices[3] = {1u, 7u, 0u};
  const uint64_t slot = 15296u;
  const uint64_t audio = 15060u;
  const uint64_t log = 34432u;
  const uint64_t frame = 59736u;
  const uint64_t def_slots = 4194304u / frame + 1u;
  const uint64_t total = slot * (def_slots + 1u) + audio * def_slots + log * def_slots;
  fake_host h;
  char scn[RX_SCN_CAP];
  int rc = IVS_TEST_FAIL;
  rx_psg_image();
  if (!rx_psg_scenario(scn, sizeof scn, "run-to 3000 ticks\n")) {
    return IVS_TEST_FAIL;
  }
  if (def_slots != 71u || slot * (def_slots + 1u) != 1101312u || total != 4615244u || total >= IVS_RUNNER_BUFFERS_MAX) {
    ivs_test_diag("derived total %llu", (unsigned long long)total);
    return IVS_TEST_FAIL;
  }
  for (size_t i = 0u; i < 3u; i++) {
    fh_init(&h);
    fh_add(&h, "psg-prog.exec", rx_psg_exec, sizeof rx_psg_exec, IVS_RUNNER_READ_OK);
    h.buffer_bytes = IVS_RUNNER_BUFFERS_MAX;
    h.slice_ticks = slices[i];
    if (!rx_expect(scn, &h, IVS_RUNNER_OUTCOME_PASS, 0u)) {
      goto done;
    }
    rx_free(&h);
  }
  fh_init(&h);
  fh_add(&h, "psg-prog.exec", rx_psg_exec, sizeof rx_psg_exec, IVS_RUNNER_READ_OK);
  h.buffer_bytes = (size_t)total;
  if (!rx_expect(scn, &h, IVS_RUNNER_OUTCOME_PASS, 0u)) {
    goto done;
  }
  rx_free(&h);
  fh_init(&h);
  fh_add(&h, "psg-prog.exec", rx_psg_exec, sizeof rx_psg_exec, IVS_RUNNER_READ_OK);
  h.buffer_bytes = (size_t)total - 1u;
  if (!rx_expect(scn, &h, IVS_RUNNER_OUTCOME_HARNESS_FAIL, 14u) || !rx_is(h.out, "outcome.detail", "buffers-too-small")) {
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  rx_free(&h);
  return rc;
}


/* ---- video capture, frame and video asserts, the video chain check (plan 06-12, D-16, D-17) ---- */

/* SHA-256 of the canonical text form of a display-off frame with the border color 0: 96 lines of 159 `0` digits, LF ended
 * (typed from `printf` and `shasum -a 256` over that text, independent of the runner's helper). */
static const char rx_vid_hash0[] = "0aff949bb8caff5803ed97a99d08d2ea999639170d09add9f2ff661b854fc314";
/* The chain after two display-off frames of border 0: h2 of data/video/chain-framing.literal.txt (derived there by recipe). */
static const char rx_vid_h2[] = "4cb3788af5c5743ae6d5ba3fdd22c6317c50c4432817acf20f577c6d8974df4a";
static const char rx_vid_bad[] = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
#define RX_VID_BYTES 30592u /* two slots of 15,296 */

/* The looping stand-in (display never enabled: border-colored, undisplayed frames) under a real output block with the
 * scenario body after the content lines (the run-to line is line 7 when the body starts with a capture line, the first
 * assert is then line 8). */
static int rx_vid_run(fake_host *h, const char *body, uint32_t outcome, uint32_t exit_code)
{
  char scn[RX_SCN_CAP];
  rx_psg_image();
  if (!rx_psg_scenario(scn, sizeof scn, body)) {
    return 0;
  }
  fh_init(h);
  fh_add(h, "psg-prog.exec", rx_psg_exec, sizeof rx_psg_exec, IVS_RUNNER_READ_OK);
  h->buffer_bytes = IVS_RUNNER_BUFFERS_MAX;
  return rx_expect(scn, h, outcome, exit_code);
}

/* The tracer: capture video 0 1 across the first frame end of the looping image and a pixel assert, then a window of two
 * frames, the delivered bytes seen by the operator hook, and the same asserts with the run cut into slices of 50,000 ticks (the chain
 * continues across calls with first indexes 0 and 1: no harness failure). */
static int ex_video_capture(void)
{
  fake_host h;
  int rc = IVS_TEST_FAIL;
  char v[RX_VALUE_CAP];
  fh_init(&h);
  if (!rx_vid_run(&h, "capture video 0 1\nrun-to 70000 ticks\nassert frame 0 pixel 0 0 == 0\n", IVS_RUNNER_OUTCOME_PASS, 0u)) {
    goto done;
  }
  if (!rx_is(h.out, "step.1.assert.1", "pass frame 0 pixel 0 0 == 0 got 0") || !rx_is(h.out, "output.video.bytes", "15296")) {
    goto done;
  }
  rx_free(&h);
  fh_init(&h);
  h.want_vraw = 1;
  {
    char scn[RX_SCN_CAP];
    rx_psg_image();
    if (!rx_psg_scenario(scn, sizeof scn,
                         "capture video 0 2\nrun-to 130000 ticks\nassert video count == 2\nassert frame 0 pixel 0 0 == 0\n"
                         "assert frame 1 pixel 158 95 == 0\nassert frame 1 displayed == no\n")) {
      goto done;
    }
    fh_add(&h, "psg-prog.exec", rx_psg_exec, sizeof rx_psg_exec, IVS_RUNNER_READ_OK);
    h.buffer_bytes = IVS_RUNNER_BUFFERS_MAX;
    if (!rx_expect(scn, &h, IVS_RUNNER_OUTCOME_PASS, 0u) || h.vraw_bytes != RX_VID_BYTES ||
        !rx_is(h.out, "output.video.bytes", "30592") || !rx_is(h.out, "step.1.assert.3", "pass frame 1 pixel 158 95 == 0 got 0")) {
      goto done;
    }
    rx_free(&h);
    fh_init(&h);
    fh_add(&h, "psg-prog.exec", rx_psg_exec, sizeof rx_psg_exec, IVS_RUNNER_READ_OK);
    h.buffer_bytes = IVS_RUNNER_BUFFERS_MAX;
    h.slice_ticks = 50000u;
    if (!rx_expect(scn, &h, IVS_RUNNER_OUTCOME_PASS, 0u) || !rx_value(h.out, "output.video.bytes", v, sizeof v) ||
        strcmp(v, "30592") != 0) {
      goto done;
    }
  }
  rc = IVS_TEST_PASS;
done:
  rx_free(&h);
  return rc;
}

/* Every assert form passes and fails for the right reason: pixel (an equal, an unequal and the blank form), hash (the
 * literal text hash, a wrong one), displayed, count and chain; a failure is a guest-fail naming the clause and the value
 * read. The blank refusal of the hash helper is shown on a synthetic plane. */
static int ex_video_assert(void)
{
  fake_host h;
  char body[2048];
  int rc = IVS_TEST_FAIL;
  uint8_t plane[4 * 2];
  uint8_t sha[32];
  (void)snprintf(body, sizeof body,
                 "capture video 0 2\nrun-to 130000 ticks\n"
                 "assert frame 0 pixel 5 5 == 0\nassert frame 0 pixel 5 5 != blank\nassert frame 1 pixel 5 5 != 1\n"
                 "assert frame 0 hash == %s\nassert frame 1 hash != %s\nassert frame 0 displayed == no\n"
                 "assert frame 1 displayed != yes\nassert video count == 2\nassert video count != 3\n"
                 "assert video chain == %s\nassert video chain != %s\n",
                 rx_vid_hash0, rx_vid_bad, rx_vid_h2, rx_vid_bad);
  fh_init(&h);
  if (!rx_vid_run(&h, body, IVS_RUNNER_OUTCOME_PASS, 0u) ||
      !rx_is(h.out, "step.1.assert.4", "pass frame 0 hash == 0aff949bb8caff5803ed97a99d08d2ea999639170d09add9f2ff661b854fc314 got "
                                       "0aff949bb8caff5803ed97a99d08d2ea999639170d09add9f2ff661b854fc314") ||
      !rx_is(h.out, "step.1.assert.6", "pass frame 0 displayed == no got no") ||
      !rx_is(h.out, "step.1.assert.8", "pass video count == 2 got 2")) {
    goto done;
  }
  rx_free(&h);
  /* The failing forms, one scenario each: the clause and the value read (lines: the first assert is line 8). */
  {
    static const struct {
      const char *assert_line;
      const char *detail;
    } fails[] = {
        {"assert frame 0 pixel 0 0 == 1\n", "assert=1 line=8 frame 0 pixel 0 0 == 1 got 0"},
        {"assert frame 1 displayed == yes\n", "assert=1 line=8 frame 1 displayed == yes got no"},
        {"assert video count == 3\n", "assert=1 line=8 video count == 3 got 2"},
    };
    for (size_t i = 0u; i < sizeof fails / sizeof fails[0]; i++) {
      (void)snprintf(body, sizeof body, "capture video 0 2\nrun-to 130000 ticks\n%s", fails[i].assert_line);
      fh_init(&h);
      if (!rx_vid_run(&h, body, IVS_RUNNER_OUTCOME_GUEST_FAIL, 10u) || !rx_is(h.out, "outcome.detail", fails[i].detail)) {
        goto done;
      }
      rx_free(&h);
    }
    (void)snprintf(body, sizeof body, "capture video 0 2\nrun-to 130000 ticks\nassert frame 0 hash == %s\n", rx_vid_bad);
    fh_init(&h);
    if (!rx_vid_run(&h, body, IVS_RUNNER_OUTCOME_GUEST_FAIL, 10u) || !rx_starts(h.out, "outcome.detail", "assert=1 line=8 frame 0 hash == 0123") ||
        strstr(h.out, "got 0aff949bb8caff58") == NULL) {
      goto done;
    }
    rx_free(&h);
    (void)snprintf(body, sizeof body, "capture video 0 2\nrun-to 130000 ticks\nassert video chain == %s\n", rx_vid_bad);
    fh_init(&h);
    if (!rx_vid_run(&h, body, IVS_RUNNER_OUTCOME_GUEST_FAIL, 10u) || strstr(h.out, rx_vid_h2) == NULL) {
      goto done;
    }
    rx_free(&h);
    /* A pixel outside the published plane is a harness failure, not a vacuous != pass. */
    fh_init(&h);
    if (!rx_vid_run(&h, "capture video 0 1\nrun-to 70000 ticks\nassert frame 0 pixel 159 0 != 3\n", IVS_RUNNER_OUTCOME_HARNESS_FAIL, 14u) ||
        !rx_is(h.out, "outcome.detail", "frame-pixel-range assert=1 line=8")) {
      goto done;
    }
  }
  /* The text hash helper refuses a plane with a blank position (the hash form has no digit for it) and equals the
   * hand text hash for a 4 x 2 plane of digits (typed: `01AF` and `FA10`, LF ended). */
  memset(plane, 0, sizeof plane);
  plane[1] = 1u;
  plane[2] = 10u;
  plane[3] = 15u;
  plane[4] = 15u;
  plane[5] = 10u;
  plane[6] = 1u;
  plane[7] = 0u;
  {
    ivs_rsha256 c;
    uint8_t want[32];
    static const char text[] = "01AF\nFA10\n";
    ivs_rsha256_init(&c);
    ivs_rsha256_update(&c, (const uint8_t *)text, sizeof text - 1u);
    ivs_rsha256_final(&c, want);
    if (ivs_runner_frame_text_sha256(plane, 4u, 2u, sha) != 0 || memcmp(sha, want, 32u) != 0) {
      goto done;
    }
  }
  plane[3] = IVS_PIXEL_BLANK;
  if (ivs_runner_frame_text_sha256(plane, 4u, 2u, sha) != 1) {
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  rx_free(&h);
  return rc;
}

/* The runner recomputes the chain from the delivered slots and the core's domain 16 agrees (an uncorrupted run is a pass); with a host
 * that flips a delivered byte before the check reads it the check disagrees: harness-fail, detail video-chain, exit 14
 * (the check is a comparison, not a label). */
static int ex_video_chain_check(void)
{
  fake_host h;
  int rc = IVS_TEST_FAIL;
  fh_init(&h);
  h.want_vraw = 1;
  if (!rx_vid_run(&h, "run-to 130000 ticks\nassert video chain == 4cb3788af5c5743ae6d5ba3fdd22c6317c50c4432817acf20f577c6d8974df4a\n",
                  IVS_RUNNER_OUTCOME_PASS, 0u)) {
    goto done;
  }
  rx_free(&h);
  fh_init(&h);
  h.want_vraw = 1;
  h.corrupt_video = 1;
  {
    char scn[RX_SCN_CAP];
    rx_psg_image();
    if (!rx_psg_scenario(scn, sizeof scn, "run-to 130000 ticks\n")) {
      goto done;
    }
    fh_add(&h, "psg-prog.exec", rx_psg_exec, sizeof rx_psg_exec, IVS_RUNNER_READ_OK);
    h.buffer_bytes = IVS_RUNNER_BUFFERS_MAX;
    if (!rx_expect(scn, &h, IVS_RUNNER_OUTCOME_HARNESS_FAIL, 14u) || !rx_is(h.out, "outcome.detail", "video-chain")) {
      goto done;
    }
  }
  rc = IVS_TEST_PASS;
done:
  rx_free(&h);
  return rc;
}

/* A capture, a frame assert or a video assert run without a delivered video buffer is a harness failure, never a vacuous
 * pass; a run that asks for none of them still passes without a buffer. */
static int ex_video_without_buffer(void)
{
  static const char *const bodies[] = {"capture video 0 1\nrun-to 70000 ticks\n", "run-to 70000 ticks\nassert video count == 0\n",
                                       "run-to 70000 ticks\nassert video chain != 0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\n"};
  fake_host h;
  char scn[RX_SCN_CAP];
  int rc = IVS_TEST_FAIL;
  rx_psg_image();
  for (size_t i = 0u; i < sizeof bodies / sizeof bodies[0]; i++) {
    if (!rx_psg_scenario(scn, sizeof scn, bodies[i])) {
      return IVS_TEST_FAIL;
    }
    fh_init(&h);
    fh_add(&h, "psg-prog.exec", rx_psg_exec, sizeof rx_psg_exec, IVS_RUNNER_READ_OK);
    if (!rx_expect(scn, &h, IVS_RUNNER_OUTCOME_HARNESS_FAIL, 14u) ||
        !rx_is(h.out, "outcome.detail", "stream-without-buffer target=video")) {
      goto done;
    }
    rx_free(&h);
  }
  if (!rx_psg_scenario(scn, sizeof scn, "run-to 70000 ticks\n")) {
    return IVS_TEST_FAIL;
  }
  fh_init(&h);
  fh_add(&h, "psg-prog.exec", rx_psg_exec, sizeof rx_psg_exec, IVS_RUNNER_READ_OK);
  if (!rx_expect(scn, &h, IVS_RUNNER_OUTCOME_PASS, 0u)) {
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  rx_free(&h);
  return rc;
}

/* An asserted frame outside the capture window, or one that was never delivered, is a harness failure naming the assert; the
 * hash-needs-colors-only refusal is exercised on a synthetic plane in runner.exec.video-assert. */
static int ex_frame_not_captured(void)
{
  fake_host h;
  int rc = IVS_TEST_FAIL;
  fh_init(&h);
  if (!rx_vid_run(&h, "capture video 1 1\nrun-to 130000 ticks\nassert frame 0 pixel 0 0 == 0\n", IVS_RUNNER_OUTCOME_HARNESS_FAIL, 14u) ||
      !rx_is(h.out, "outcome.detail", "frame-not-captured assert=1 line=8")) {
    goto done;
  }
  rx_free(&h);
  fh_init(&h);
  if (!rx_vid_run(&h, "capture video 0 16\nrun-to 130000 ticks\nassert frame 5 displayed == no\n", IVS_RUNNER_OUTCOME_HARNESS_FAIL, 14u) ||
      !rx_is(h.out, "outcome.detail", "frame-not-captured assert=1 line=8")) {
    goto done;
  }
  rx_free(&h);
  /* The window frame that was delivered is readable: frame 1 of the window 1..1. */
  fh_init(&h);
  if (!rx_vid_run(&h, "capture video 1 1\nrun-to 130000 ticks\nassert frame 1 pixel 0 0 == 0\n", IVS_RUNNER_OUTCOME_PASS, 0u)) {
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  rx_free(&h);
  return rc;
}


/* ivsr 3 (plan 06-12): the five video keys with their values, from the independent key table of runner.exec.key-order and
 * read here by value. A run with no machine (an unknown profile) prints 0 and `-`; a diagnostic run that ends before its first
 * frame end completes none (frames 0, geometry 159x96, row lines 2, no last tick, chain check pass over an empty chain); the
 * looping image over two frame ends completes two (last tick 2 x 59,736); a run without an output block cannot check the chain (`-`). */
static int ex_report_keys_ivsr3(void)
{
  fake_host h;
  char diag[RX_SCN_CAP];
  char scn[RX_SCN_CAP];
  int rc = IVS_TEST_FAIL;
  if (!rx_diag(diag, sizeof diag) || !rx_replace(diag, "profile intv-2609-ntsc-family-p1", "profile no-such-profile", scn, sizeof scn)) {
    return IVS_TEST_FAIL;
  }
  fh_init(&h);
  if (!rx_expect(scn, &h, IVS_RUNNER_OUTCOME_HARNESS_FAIL, 14u) || strncmp(h.out, "ivsr 3\n", 7u) != 0 ||
      !rx_is(h.out, "output.video.frames", "0") || !rx_is(h.out, "video.geometry", "-") ||
      !rx_is(h.out, "video.row_lines", "-") || !rx_is(h.out, "video.frame.last_tick", "-") ||
      !rx_is(h.out, "video.chain.check", "-")) {
    goto done;
  }
  rx_free(&h);
  fh_init(&h);
  h.buffer_bytes = IVS_RUNNER_BUFFERS_MAX;
  if (!rx_expect(diag, &h, IVS_RUNNER_OUTCOME_PASS, 0u) || !rx_is(h.out, "output.video.frames", "0") ||
      !rx_is(h.out, "video.geometry", "159x96") || !rx_is(h.out, "video.row_lines", "2") ||
      !rx_is(h.out, "video.frame.last_tick", "-") || !rx_is(h.out, "video.chain.check", "pass")) {
    goto done;
  }
  rx_free(&h);
  fh_init(&h);
  if (!rx_expect(diag, &h, IVS_RUNNER_OUTCOME_PASS, 0u) || !rx_is(h.out, "video.geometry", "159x96") ||
      !rx_is(h.out, "video.chain.check", "-")) {
    goto done;
  }
  rx_free(&h);
  fh_init(&h);
  if (!rx_vid_run(&h, "run-to 130000 ticks\n", IVS_RUNNER_OUTCOME_PASS, 0u) || !rx_is(h.out, "output.video.frames", "2") ||
      !rx_is(h.out, "video.frame.last_tick", "119472") || !rx_is(h.out, "video.chain.check", "pass") ||
      !rx_is(h.out, "output.video.bytes", "30592")) {
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  rx_free(&h);
  return rc;
}

/* The golden key list (ivs_runner_golden_keep, used by ivs-run --update-golden) keeps the video keys only while video is observed
 * (the count rule: output.video.bytes is a decimal other than 0): with video observed the filtered report holds digest.video and
 * the five video keys; with video not observed none of them, and the filtered body equals the lines whose keys are on the
 * pre-ivsr-3 list written out here (so a golden of a scenario that delivers no frame is byte-identical); audio.chain.check and
 * video.chain.check never appear for a not-observed video, and host.calls, ivsr and report.end never. */
static int rx_keep_text(const char *report, const int live[4], char *out, size_t cap)
{
  size_t used = 0u;
  const char *p = report;
  while (*p != '\0') {
    const char *eol = strchr(p, '\n');
    size_t n = eol != NULL ? (size_t)(eol - p) : strlen(p);
    if (ivs_runner_golden_keep(p, n, live)) {
      if (used + n + 2u > cap) {
        return 0;
      }
      memcpy(out + used, p, n);
      out[used + n] = '\n';
      used += n + 1u;
    }
    p += n + (eol != NULL ? 1u : 0u);
  }
  out[used] = '\0';
  return 1;
}

static int ex_golden_keys_video_live(void)
{
  static const char *const vkeys[] = {"digest.video ", "output.video.frames ", "video.geometry ", "video.row_lines ",
                                      "video.frame.last_tick ", "video.chain.check "};
  static const char *const never[] = {"ivsr ", "api.version ", "host.calls ", "report.end ", "audio.chain.check ", "refpcm."};
  fake_host h;
  char text[8192];
  char diag[RX_SCN_CAP];
  int live_on[4] = {1, 0, 0, 0};
  int live_off[4] = {0, 0, 0, 0};
  int rc = IVS_TEST_FAIL;
  fh_init(&h);
  if (!rx_vid_run(&h, "run-to 130000 ticks\n", IVS_RUNNER_OUTCOME_PASS, 0u) || !rx_keep_text(h.out, live_on, text, sizeof text)) {
    goto done;
  }
  for (size_t i = 0u; i < sizeof vkeys / sizeof vkeys[0]; i++) {
    if (strstr(text, vkeys[i]) == NULL) {
      ivs_test_diag("observed video: golden body lacks %s", vkeys[i]);
      goto done;
    }
  }
  if (strstr(text, "\nvideo.chain.check pass\n") == NULL || strstr(text, "\nvideo.geometry 159x96\n") == NULL) {
    goto done;
  }
  if (!rx_keep_text(h.out, live_off, text, sizeof text)) {
    goto done;
  }
  for (size_t i = 0u; i < sizeof vkeys / sizeof vkeys[0]; i++) {
    if (strstr(text, vkeys[i]) != NULL) {
      ivs_test_diag("video not observed: golden body holds %s", vkeys[i]);
      goto done;
    }
  }
  rx_free(&h);
  /* A scenario that delivers no frame: video is not observed, the body is the pre-ivsr-3 set of lines. */
  if (!rx_diag(diag, sizeof diag)) {
    return IVS_TEST_FAIL;
  }
  fh_init(&h);
  h.buffer_bytes = IVS_RUNNER_BUFFERS_MAX;
  if (!rx_expect(diag, &h, IVS_RUNNER_OUTCOME_PASS, 0u) || !rx_is(h.out, "output.video.bytes", "0") ||
      !rx_keep_text(h.out, live_off, text, sizeof text)) {
    goto done;
  }
  for (const char *p = h.out; *p != '\0';) {
    const char *eol = strchr(p, '\n');
    size_t n = (size_t)(eol - p);
    int old_list = 0;
    static const char *const old_exact[] = {"scenario.id ", "scenario.profile ", "outcome ", "outcome.detail ", "exit.code ",
                                            "digest.cpu ", "digest.mem ", "digest.images ", "digest.instance ",
                                            "digest.composite ", "output.video.bytes ", "output.audio.bytes "};
    static const char *const old_prefix[] = {"slot.", "step.", "cpu.", "input."};
    int kept = ivs_runner_golden_keep(p, n, live_off);
    for (size_t i = 0u; i < sizeof old_exact / sizeof old_exact[0]; i++) {
      old_list = old_list || strncmp(p, old_exact[i], strlen(old_exact[i])) == 0;
    }
    for (size_t i = 0u; i < sizeof old_prefix / sizeof old_prefix[0]; i++) {
      old_list = old_list || strncmp(p, old_prefix[i], strlen(old_prefix[i])) == 0;
    }
    if (kept != old_list) {
      ivs_test_diag("line '%.*s': kept %d, pre-ivsr-3 list %d", (int)n, p, kept, old_list);
      goto done;
    }
    p = eol + 1;
  }
  for (size_t i = 0u; i < sizeof never / sizeof never[0]; i++) {
    if (strstr(text, never[i]) != NULL && strncmp(never[i], "ivsr ", 5u) != 0) {
      goto done;
    }
  }
  rc = IVS_TEST_PASS;
done:
  rx_free(&h);
  return rc;
}

/* The hook contract: default rate 48000 and 44100 both give the by-hand frame count with the reference
 * header and the file hash; a run cut into slices of 7 ticks gives the same PCM and hash as one slice
 * (the converter state is carried across slices). */
static int ex_wav_hook(void)
{
  fake_host a;
  fake_host b;
  char scn[RX_SCN_CAP];
  int ok;
  rx_psg_image();
  if (!rx_psg_scenario(scn, sizeof scn, "run-to 3000 ticks\n")) {
    return IVS_TEST_FAIL;
  }
  ok = rx_wav_run(&a, scn, 48000u, 0u);
  if (ok) {
    rx_free(&a);
    ok = rx_wav_run(&a, scn, 44100u, 0u);
  }
  rx_free(&a);
  if (ok) {
    ok = rx_wav_run(&a, scn, 48000u, 0u);
    if (ok) {
      ok = rx_wav_run(&b, scn, 48000u, 7u) && a.wav_pcm_len == b.wav_pcm_len &&
           memcmp(a.wav_pcm, b.wav_pcm, a.wav_pcm_len) == 0 && memcmp(a.wav_sha, b.wav_sha, 32u) == 0;
      rx_free(&b);
    }
    rx_free(&a);
  }
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* Failure shapes of the WAV option: a PCM hook that fails (harness-fail wav-out, no hash), a finish hook that
 * fails (wav-out), a byte bound below 44 + 2 x frames (wav-out-max-bytes), hooks without an audio buffer
 * (wav-without-audio), only one of the two hooks (harness-fail before any report), and an invalid rate in the
 * options (the same). */
static int ex_wav_failures(void)
{
  fake_host h;
  char scn[RX_SCN_CAP];
  ivs_scenario *s = NULL;
  int rc = IVS_TEST_FAIL;
  rx_psg_image();
  if (!rx_psg_scenario(scn, sizeof scn, "run-to 3000 ticks\n")) {
    return IVS_TEST_FAIL;
  }
  for (int shape = 0; shape < 4; shape++) {
    fh_init(&h);
    fh_add(&h, "psg-prog.exec", rx_psg_exec, sizeof rx_psg_exec, IVS_RUNNER_READ_OK);
    h.buffer_bytes = shape == 3 ? 0u : IVS_RUNNER_BUFFERS_MAX;
    h.want_wav = 1;
    h.wav_fail_pcm = shape == 0;
    h.wav_fail_finish = shape == 1;
    h.out_max = shape == 2 ? 100u : 0u; /* 44 + 80 bytes are produced; the bound is judged on the WAV alone: */
    h.no_audio_hook = shape == 2;       /* the raw audio hook is left out, it would hit the same bound first */
    if (!rx_expect(scn, &h, IVS_RUNNER_OUTCOME_HARNESS_FAIL, 14u) ||
        !rx_is(h.out, "outcome.detail",
               shape == 0 ? "wav-out" : shape == 1 ? "wav-out" : shape == 2 ? "wav-out-max-bytes" : "wav-without-audio") ||
        !rx_is(h.out, "refpcm.sha256", "-") || !rx_is(h.out, "refpcm.frames", "-")) { /* review WR-01: no frames of an unfinished file */
      ivs_test_diag("failure shape %d", shape);
      rx_free(&h);
      goto done;
    }
    rx_free(&h);
  }
  /* only one hook, or an invalid rate: harness-fail without a report */
  s = malloc(sizeof *s);
  if (s != NULL) {
    ivs_scenario_error e;
    ivs_runner_host host;
    ivs_runner_options o;
    ivs_runner_result res;
    void *storage = malloc(FH_STORAGE);
    int ok = 0;
    fh_init(&h);
    fh_add(&h, "psg-prog.exec", rx_psg_exec, sizeof rx_psg_exec, IVS_RUNNER_READ_OK);
    h.out = malloc(FH_OUT_CAP);
    memset(&e, 0, sizeof e);
    if (storage != NULL && h.out != NULL && ivs_scenario_parse((const uint8_t *)scn, strlen(scn), s, &e) == 0) {
      h.out[0] = '\0';
      h.out_len = 0u;
      memset(&host, 0, sizeof host);
      host.ctx = &h;
      host.read_file = fh_read;
      host.write_report = fh_write;
      host.storage = storage;
      host.storage_bytes = FH_STORAGE;
      host.write_wav_pcm = fh_wav_pcm; /* finish_wav missing */
      ok = ivs_runner_execute(s, &host, NULL, &res) == IVS_RUNNER_OUTCOME_HARNESS_FAIL && h.out_len == 0u;
      host.finish_wav = fh_wav_finish;
      memset(&o, 0, sizeof o);
      o.wav_rate = 8000u;
      ok = ok && ivs_runner_execute(s, &host, &o, &res) == IVS_RUNNER_OUTCOME_HARNESS_FAIL && h.out_len == 0u;
      o.wav_rate = 0u;
      o.reserved = 1u;
      ok = ok && ivs_runner_execute(s, &host, &o, &res) == IVS_RUNNER_OUTCOME_HARNESS_FAIL && h.out_len == 0u;
    }
    free(storage);
    free(h.out);
    h.out = NULL;
    if (!ok) {
      goto done;
    }
  }
  rc = IVS_TEST_PASS;
done:
  free(s);
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
  /* digest.mem and digest.composite join the list in plan 05-05: the words are guest-visible, so the
   * pull-down masks of the two ports (MEM rows) differ while a controller word is held; the scenario
   * holds nonzero words at its end. The CPU and image digests still do not see them (the diagnostic
   * never reads the ports). */
  static const char *const may_differ[] = {"digest.input",       "input.events",       "input.consumed",
                                           "input.controller.0", "input.controller.1", "input.controller.2",
                                           "input.controller.3", "host.calls",         "digest.mem",
                                           "digest.composite"};
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
    {"runner.exec.ivsr2-keys", ex_ivsr2_keys, NULL},
    {"runner.exec.audio-chain-check", ex_audio_chain_check, NULL},
    {"runner.exec.sha256-fips", ex_sha256_fips, NULL},
    {"runner.exec.psglog-assert", ex_psglog_assert, NULL},
    {"runner.exec.audio-assert", ex_audio_assert, NULL},
    {"runner.exec.capture-overflow", ex_capture_overflow, NULL},
    {"runner.exec.digest-assert-audio-psglog", ex_digest_assert_audio_psglog, NULL},
    {"runner.exec.operator-files-hook", ex_operator_files_hook, NULL},
    {"runner.exec.out-max-bytes", ex_out_max_bytes, NULL},
    {"runner.exec.refpcm-keys-only-with-option", ex_refpcm_keys_only_with_option, NULL},
    {"runner.exec.buffers-video-slack", ex_buffers_video_slack, NULL},
    {"runner.exec.video-capture", ex_video_capture, NULL},
    {"runner.exec.video-assert", ex_video_assert, NULL},
    {"runner.exec.video-chain-check", ex_video_chain_check, NULL},
    {"runner.exec.video-without-buffer", ex_video_without_buffer, NULL},
    {"runner.exec.frame-not-captured", ex_frame_not_captured, NULL},
    {"runner.exec.report-keys-ivsr3", ex_report_keys_ivsr3, NULL},
    {"runner.golden.keys-video-live", ex_golden_keys_video_live, NULL},
    {"runner.exec.wav-hook", ex_wav_hook, NULL},
    {"runner.exec.wav-failures", ex_wav_failures, NULL},
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
