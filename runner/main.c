/* SPDX-License-Identifier: MIT */
/* ivs-run: the headless runner executable (D-17, D-20, D-22, plan 04-19). One process runs one
 * IVSS v1 scenario on one fresh instance through the runner library and prints the report on
 * stdout; the exit code is the outcome's (pass 0, guest-fail 10, timeout 11, cancelled 12,
 * missing-fixture 13, harness-fail 14). Thin: option parsing, the host vtable over hostio.c (the
 * only POSIX file), and the exit code. ISO C only here. A usage error, an unreadable or refused
 * scenario and a failed report write are harness-fail (14) with one line on stderr. System images
 * come only from the operator's --exec, --grom and --cart (for `content <role> user`); a scenario
 * names content only under the content directory. Slicing, the --max-ticks ceiling, the wall cap
 * and cancellation are the library's (ivs_runner_options); the two deterministic test hooks live
 * here: --self-signal-after-slices N calls raise(SIGINT) in the cancel check that follows slice N
 * (the library calls cancel_requested once before every slice), so the real handler path runs
 * with no race and no sleep; --self-signal-count K (1..4, default 1) raises it K times there, so a
 * repeated signal is tested the same way (review WR-04); --wall-timeout-ms 1 with a long bound can
 * only end by the wall cap.
 * --update-golden <file> (plan 04-21, D-11) also writes the run's repeatability golden: a header
 * (tier, oracle, scenario, profile, domains) and the report lines of the golden key list; it is run
 * by a person, never by the gate, and refuses `user` content and host-dependent outcomes.
 * Documented in docs/runner.md. */
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hostio.h"
#include "ivs/ivs.h"
#include "ivs_runner.h"

#define MAIN_EXIT_HARNESS 14
#define MAIN_DIR_MAX 4096u
#define MAIN_GOLDEN_CAP (512u * 1024u) /* report bytes kept for --update-golden */

typedef struct main_host {
  const char *content_dir;
  const char *user[IVS_SCN_ROLES]; /* operator images by role, NULL when not given */
  int self_signal;                 /* --self-signal-after-slices given */
  uint64_t self_signal_after;      /* its N */
  int self_signal_repeat;          /* --self-signal-count given */
  uint64_t self_signal_count;      /* its K: raises at that check (1 when not given) */
  uint64_t cancel_checks;          /* cancel_requested calls so far */
  char *golden_buf;                /* report copy for --update-golden, NULL otherwise */
  size_t golden_len;
  int golden_overflow;             /* the report did not fit MAIN_GOLDEN_CAP */
} main_host;

static const char main_usage[] =
    "usage: ivs-run [options] <scenario.ivss>\n"
    "Runs one IVSS v1 scenario on one fresh machine instance and prints its report on stdout.\n"
    "options:\n"
    "  --content-dir <dir>  directory of the scenario's relative content paths\n"
    "                       (default: the directory of the scenario file)\n"
    "  --exec <file>        operator image for `content exec user`\n"
    "  --grom <file>        operator image for `content grom user`\n"
    "  --cart <file>        operator image for `content cart user`\n"
    "  --max-ticks <n>      machine-time ceiling in master ticks, 1..9223372036854775808\n"
    "                       (default 1099511627776); reaching it is timeout (budget)\n"
    "  --wall-timeout-ms <n>  host wall cap in milliseconds, 1..4294967295 (default 30000);\n"
    "                       a scenario's wall-timeout-ms only lowers it; reaching it is\n"
    "                       timeout (wall), never golden\n"
    "  --slice-ticks <n>    master ticks per slice, 1..4194304 (default 4194304)\n"
    "  --self-signal-after-slices <n>  test hook: raise SIGINT after slice n (0: before the\n"
    "                       first slice); the run ends cancelled\n"
    "  --self-signal-count <n>  test hook: raise SIGINT n times there, 1..4 (default 1);\n"
    "                       needs --self-signal-after-slices\n"
    "  --update-golden <file>  also write the run's repeatability golden (.digests) to file;\n"
    "                       refused for `user` content and host-dependent outcomes\n"
    "  --help               print this text and exit 0\n"
    "outcomes (outcome exit-code):\n"
    "pass 0\n"
    "guest-fail 10\n"
    "timeout 11\n"
    "cancelled 12\n"
    "missing-fixture 13\n"
    "harness-fail 14\n";

/* ---- host vtable ---- */

static int mh_read_file(void *ctx, const char *relpath, size_t cap, uint8_t *buf, size_t *len)
{
  const main_host *h = ctx;
  return ivs_hostio_read(h->content_dir, relpath, cap, buf, len);
}

static int mh_read_user(void *ctx, uint32_t role, size_t cap, uint8_t *buf, size_t *len)
{
  const main_host *h = ctx;
  *len = 0u;
  if (role >= IVS_SCN_ROLES || h->user[role] == NULL) {
    return IVS_RUNNER_READ_MISSING;
  }
  return ivs_hostio_read(NULL, h->user[role], cap, buf, len);
}

static uint64_t mh_now_ms(void *ctx)
{
  (void)ctx;
  return ivs_hostio_now_ms();
}

/* Called once before every slice: the call that follows slice N is call N + 1. */
static int mh_cancel(void *ctx)
{
  main_host *h = ctx;
  h->cancel_checks++;
  if (h->self_signal && h->cancel_checks == h->self_signal_after + 1u) {
    for (uint64_t k = 0u; k < h->self_signal_count; k++) {
      (void)raise(SIGINT);
    }
  }
  return ivs_hostio_cancelled();
}

static int mh_write(void *ctx, const char *bytes, size_t len)
{
  main_host *h = ctx;
  if (h->golden_buf != NULL) {
    if (len <= MAIN_GOLDEN_CAP - h->golden_len) {
      memcpy(h->golden_buf + h->golden_len, bytes, len);
      h->golden_len += len;
    } else {
      h->golden_overflow = 1;
    }
  }
  return fwrite(bytes, 1u, len, stdout) == len ? 0 : -1;
}

/* ---- options ---- */

static int main_fail(const char *what)
{
  fprintf(stderr, "ivs-run: %s\n", what);
  return MAIN_EXIT_HARNESS;
}

static int main_usage_error(const char *what)
{
  fprintf(stderr, "ivs-run: %s\n%s", what, main_usage);
  return MAIN_EXIT_HARNESS;
}

/* Decimal digits only, lo..hi. 0 on anything else (sign, blank, overflow, out of range). */
static int main_number(const char *a, uint64_t lo, uint64_t hi, uint64_t *out)
{
  uint64_t v = 0u;
  if (a[0] == '\0') {
    return 0;
  }
  for (const char *p = a; *p != '\0'; p++) {
    uint64_t dgt;
    if (*p < '0' || *p > '9') {
      return 0;
    }
    dgt = (uint64_t)(*p - '0');
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

/* The directory part of path ("." when it has none, "/" for a file at the root). 0 when it does
 * not fit. */
static int main_dirname(const char *path, char *out, size_t cap)
{
  const char *slash = strrchr(path, '/');
  size_t n;
  if (slash == NULL) {
    if (cap < 2u) {
      return 0;
    }
    out[0] = '.';
    out[1] = '\0';
    return 1;
  }
  n = (size_t)(slash - path);
  if (n == 0u) {
    n = 1u;
  }
  if (n + 1u > cap) {
    return 0;
  }
  memcpy(out, path, n);
  out[n] = '\0';
  return 1;
}

/* ---- --update-golden (plan 04-21, D-11) ---- */

/* 1 when the line of length n has the key k (the text before the first space). */
static int main_key_is(const char *line, size_t n, const char *k)
{
  size_t kl = strlen(k);
  return n > kl && line[kl] == ' ' && memcmp(line, k, kl) == 0;
}

/* The value of key k in the report text, "-" when the key is absent. Points into buf; *vn gets
 * its length. */
static const char *main_report_value(const char *buf, size_t len, const char *k, size_t *vn)
{
  size_t i = 0u;
  while (i < len) {
    const char *nl = memchr(buf + i, '\n', len - i);
    size_t n = nl != NULL ? (size_t)(nl - (buf + i)) : len - i;
    if (main_key_is(buf + i, n, k)) {
      size_t kl = strlen(k) + 1u;
      *vn = n - kl;
      return buf + i + kl;
    }
    i += n + 1u;
  }
  *vn = 1u;
  return "-";
}

/* A live domain is observed when its count is a decimal number other than 0; `-` (no machine)
 * and 0 are none, so a hash of nothing is never recorded as an observation. */
static int main_count_observed(const char *v, size_t n)
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

/* The golden key list (docs/runner.md): scenario, outcome and exit code, slots, steps, CPU state,
 * input and output counts, the digests of cpu, mem, images, instance and composite, and the
 * digest of each observed live domain. Never ivsr, api.version, the exec:/grom: labels,
 * host.calls or report.end. */
static int main_golden_keep(const char *line, size_t n, const int live[3])
{
  static const char *const prefix[] = {"slot.", "step.", "cpu.", "input."};
  static const char *const exact[] = {"scenario.id",   "scenario.profile", "outcome",
                                      "outcome.detail", "exit.code",        "digest.cpu",
                                      "digest.mem",     "digest.images",    "digest.instance",
                                      "digest.composite", "output.video.bytes", "output.audio.bytes"};
  static const char *const live_key[] = {"digest.video", "digest.audio", "digest.input"};
  for (size_t i = 0u; i < sizeof prefix / sizeof prefix[0]; i++) {
    size_t pl = strlen(prefix[i]);
    if (n > pl && memcmp(line, prefix[i], pl) == 0) {
      return 1;
    }
  }
  for (size_t i = 0u; i < sizeof exact / sizeof exact[0]; i++) {
    if (main_key_is(line, n, exact[i])) {
      return 1;
    }
  }
  for (size_t i = 0u; i < 3u; i++) {
    if (live[i] && main_key_is(line, n, live_key[i])) {
      return 1;
    }
  }
  return 0;
}

/* Writes the .digests golden of the captured report to path. 0, or MAIN_EXIT_HARNESS after one
 * line on stderr (host-dependent outcome, incomplete capture, write error). */
static int main_write_golden(const char *path, const main_host *h, const ivs_scenario *scn)
{
  static const char *const live_count[] = {"output.video.bytes", "output.audio.bytes", "input.events"};
  const char *buf = h->golden_buf;
  size_t len = h->golden_len;
  const char *v;
  size_t vn;
  int live[3];
  FILE *f;
  int ok = 1;
  size_t i = 0u;
  if (h->golden_overflow || len == 0u || buf[len - 1u] != '\n') {
    return main_fail("golden refused: incomplete report");
  }
  /* The wall watchdog and cancellation are host-dependent and never golden (D-21). */
  v = main_report_value(buf, len, "outcome", &vn);
  if (vn == 9u && memcmp(v, "cancelled", 9u) == 0) {
    return main_fail("golden refused: host-dependent outcome");
  }
  v = main_report_value(buf, len, "outcome.detail", &vn);
  if (vn >= 5u && memcmp(v, "wall ", 5u) == 0) {
    return main_fail("golden refused: host-dependent outcome");
  }
  for (size_t k = 0u; k < 3u; k++) {
    v = main_report_value(buf, len, live_count[k], &vn);
    live[k] = main_count_observed(v, vn);
  }
  f = fopen(path, "w");
  if (f == NULL) {
    return main_fail("golden write failed");
  }
  if (fprintf(f,
              "# tier: deterministic-repeatability\n"
              "# oracle: self-recorded\n"
              "# scenario: %s\n"
              "# profile: %s\n"
              "# domains: cpu=observed mem=observed images=observed instance=observed video=%s audio=%s "
              "input=%s\n"
              "# Written by ivs-run --update-golden, never by the gate; a change is a reviewed diff\n"
              "# (docs/runner.md). The body is the run's report filtered to the golden key list.\n",
              scn->id, scn->profile, live[0] ? "observed" : "none", live[1] ? "observed" : "none",
              live[2] ? "observed" : "none") < 0) {
    ok = 0;
  }
  while (ok && i < len) {
    const char *nl = memchr(buf + i, '\n', len - i);
    size_t n = (size_t)(nl - (buf + i)) + 1u;
    if (main_golden_keep(buf + i, n - 1u, live) && fwrite(buf + i, 1u, n, f) != n) {
      ok = 0;
    }
    i += n;
  }
  if (fclose(f) != 0) {
    ok = 0;
  }
  return ok ? 0 : main_fail("golden write failed");
}

int main(int argc, char **argv)
{
  main_host mh;
  ivs_runner_options opts;
  ivs_runner_host host;
  ivs_runner_result res;
  ivs_scenario *scn = NULL;
  ivs_scenario_error err;
  ivs_storage_requirements req;
  uint8_t *text = NULL;
  void *storage = NULL;
  const char *scenario_path = NULL;
  const char *golden_path = NULL;
  char dirbuf[MAIN_DIR_MAX];
  size_t text_len = 0u;
  size_t storage_bytes;
  int r;
  int code;
  int have_max = 0;
  int have_wall = 0;
  int have_slice = 0;
  memset(&mh, 0, sizeof mh);
  memset(&opts, 0, sizeof opts);
  for (int i = 1; i < argc; i++) {
    const char *a = argv[i];
    const char **slot = NULL;
    int *seen = NULL;
    uint64_t lo = 0u;
    uint64_t hi = 0u;
    uint64_t v = 0u;
    if (strcmp(a, "--help") == 0) {
      fputs(main_usage, stdout);
      return fflush(stdout) == 0 ? 0 : MAIN_EXIT_HARNESS;
    }
    if (strcmp(a, "--content-dir") == 0) {
      slot = &mh.content_dir;
    } else if (strcmp(a, "--exec") == 0) {
      slot = &mh.user[IVS_SCN_ROLE_EXEC];
    } else if (strcmp(a, "--grom") == 0) {
      slot = &mh.user[IVS_SCN_ROLE_GROM];
    } else if (strcmp(a, "--cart") == 0) {
      slot = &mh.user[IVS_SCN_ROLE_CART];
    } else if (strcmp(a, "--update-golden") == 0) {
      slot = &golden_path;
    } else if (strcmp(a, "--max-ticks") == 0) {
      seen = &have_max;
      lo = 1u;
      hi = IVS_RUN_TARGET_MAX;
    } else if (strcmp(a, "--wall-timeout-ms") == 0) {
      seen = &have_wall;
      lo = 1u;
      hi = UINT32_MAX;
    } else if (strcmp(a, "--slice-ticks") == 0) {
      seen = &have_slice;
      lo = 1u;
      hi = IVS_RUNNER_SLICE_TICKS_MAX;
    } else if (strcmp(a, "--self-signal-after-slices") == 0) {
      seen = &mh.self_signal;
      lo = 0u;
      hi = UINT32_MAX;
    } else if (strcmp(a, "--self-signal-count") == 0) {
      seen = &mh.self_signal_repeat;
      lo = 1u;
      hi = 4u;
    } else if (a[0] == '-' && a[1] == '-') {
      return main_usage_error("unknown option");
    } else if (scenario_path != NULL) {
      return main_usage_error("more than one scenario");
    } else {
      scenario_path = a;
      continue;
    }
    if (i + 1 >= argc || argv[i + 1][0] == '\0') {
      return main_usage_error("option without a value");
    }
    i++;
    if (slot != NULL) {
      if (*slot != NULL) {
        return main_usage_error("option given twice");
      }
      *slot = argv[i];
      continue;
    }
    if (*seen) {
      return main_usage_error("option given twice");
    }
    if (!main_number(argv[i], lo, hi, &v)) {
      return main_usage_error("option value out of range");
    }
    *seen = 1;
    if (seen == &have_max) {
      opts.max_ticks = v;
    } else if (seen == &have_wall) {
      opts.wall_timeout_ms = (uint32_t)v;
    } else if (seen == &have_slice) {
      opts.slice_ticks = v;
    } else if (seen == &mh.self_signal_repeat) {
      mh.self_signal_count = v;
    } else {
      mh.self_signal_after = v;
    }
  }
  if (scenario_path == NULL) {
    return main_usage_error("no scenario");
  }
  if (mh.self_signal_repeat && !mh.self_signal) {
    return main_usage_error("--self-signal-count needs --self-signal-after-slices");
  }
  if (!mh.self_signal_repeat) {
    mh.self_signal_count = 1u;
  }
  if (mh.content_dir == NULL) {
    if (!main_dirname(scenario_path, dirbuf, sizeof dirbuf)) {
      return main_fail("scenario path too long");
    }
    mh.content_dir = dirbuf;
  }
  if (ivs_hostio_install_signals() != 0) {
    return main_fail("cannot install signal handlers");
  }

  /* The scenario, bounded like content: cap + 1 bytes reach the parser's own size refusal. */
  text = malloc((size_t)IVS_SCN_MAX_BYTES + 1u);
  scn = malloc(sizeof *scn);
  if (text == NULL || scn == NULL) {
    free(text);
    free(scn);
    return main_fail("out of memory");
  }
  r = ivs_hostio_read(NULL, scenario_path, IVS_SCN_MAX_BYTES, text, &text_len);
  if (r != IVS_RUNNER_READ_OK && r != IVS_RUNNER_READ_TOO_LARGE) {
    free(text);
    free(scn);
    return main_fail(r == IVS_RUNNER_READ_MISSING       ? "scenario not readable: missing"
                     : r == IVS_RUNNER_READ_NOT_REGULAR ? "scenario not readable: not a regular file"
                                                        : "scenario not readable: io-error");
  }
  memset(&err, 0, sizeof err);
  if (ivs_scenario_parse(text, text_len, scn, &err) != 0) {
    char line[128];
    free(text);
    free(scn);
    if (ivs_scenario_error_format(&err, line, sizeof line) != 0) {
      return main_fail("scenario-syntax");
    }
    return main_fail(line);
  }
  free(text);
  /* A golden never records operator content (D-11): refused before the run, no report. */
  if (golden_path != NULL) {
    for (uint32_t role = 0u; role < IVS_SCN_ROLES; role++) {
      if (scn->content[role].kind == IVS_SCN_CONTENT_USER) {
        free(scn);
        return main_fail("golden refused: content user");
      }
    }
    mh.golden_buf = malloc(MAIN_GOLDEN_CAP);
    if (mh.golden_buf == NULL) {
      free(scn);
      return main_fail("out of memory");
    }
  }

  /* Storage: the instance (aligned by the executor), then one read buffer of the largest role. A
   * refused profile gets a token buffer, so the executor reports profile-refused itself. */
  memset(&req, 0, sizeof req);
  req.struct_size = (uint32_t)sizeof req;
  storage_bytes = 64u;
  if (ivs_machine_storage_requirements(scn->profile, &req) == IVS_OK && req.size_bytes <= IVS_RUNNER_STORAGE_MAX &&
      req.align_bytes <= IVS_RUNNER_STORAGE_MAX) {
    storage_bytes = (size_t)req.size_bytes + (size_t)req.align_bytes + (size_t)IVS_RUNNER_CAP_CART + 1u;
  }
  storage = malloc(storage_bytes);
  if (storage == NULL) {
    free(mh.golden_buf);
    free(scn);
    return main_fail("out of memory");
  }

  memset(&host, 0, sizeof host);
  host.ctx = &mh;
  host.read_file = mh_read_file;
  host.read_user = mh_read_user;
  host.now_ms = mh_now_ms;
  host.cancel_requested = mh_cancel;
  host.write_report = mh_write;
  host.storage = storage;
  host.storage_bytes = storage_bytes;
  memset(&res, 0, sizeof res);
  (void)ivs_runner_execute(scn, &host, &opts, &res);
  free(storage);
  code = (int)res.exit_code;
  if (fflush(stdout) != 0 || ferror(stdout) || res.report_write_failed != 0u) {
    free(mh.golden_buf);
    free(scn);
    return main_fail("report write failed");
  }
  if (golden_path != NULL && main_write_golden(golden_path, &mh, scn) != 0) {
    code = MAIN_EXIT_HARNESS;
  }
  free(mh.golden_buf);
  free(scn);
  return code;
}
