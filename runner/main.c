/* SPDX-License-Identifier: MIT */
/* ivs-run: the headless runner executable (D-17, D-20, D-22, plan 04-19). One process runs one
 * IVSS v1 scenario on one fresh instance through the runner library and prints the report on
 * stdout; the exit code is the outcome's (pass 0, guest-fail 10, timeout 11, cancelled 12,
 * missing-fixture 13, harness-fail 14). Thin: option parsing, the host vtable over hostio.c (the
 * only POSIX file), and the exit code. ISO C only here. A usage error, an unreadable or refused
 * scenario and a failed report write are harness-fail (14) with one line on stderr. System images
 * come only from the operator's --exec, --grom and --cart (for `content <role> user`); a scenario
 * names content only under the content directory. Documented in docs/runner.md. */
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

typedef struct main_host {
  const char *content_dir;
  const char *user[IVS_SCN_ROLES]; /* operator images by role, NULL when not given */
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

static int mh_cancel(void *ctx)
{
  (void)ctx;
  return ivs_hostio_cancelled();
}

static int mh_write(void *ctx, const char *bytes, size_t len)
{
  (void)ctx;
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

int main(int argc, char **argv)
{
  main_host mh;
  ivs_runner_host host;
  ivs_runner_result res;
  ivs_scenario *scn = NULL;
  ivs_scenario_error err;
  ivs_storage_requirements req;
  uint8_t *text = NULL;
  void *storage = NULL;
  const char *scenario_path = NULL;
  char dirbuf[MAIN_DIR_MAX];
  size_t text_len = 0u;
  size_t storage_bytes;
  int r;
  int code;
  memset(&mh, 0, sizeof mh);
  for (int i = 1; i < argc; i++) {
    const char *a = argv[i];
    const char **slot = NULL;
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
    if (*slot != NULL) {
      return main_usage_error("option given twice");
    }
    i++;
    *slot = argv[i];
  }
  if (scenario_path == NULL) {
    return main_usage_error("no scenario");
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
  (void)ivs_runner_execute(scn, &host, NULL, &res);
  free(storage);
  free(scn);
  code = (int)res.exit_code;
  if (fflush(stdout) != 0 || ferror(stdout)) {
    return main_fail("report write failed");
  }
  if (res.report_write_failed != 0u) {
    return main_fail("report write failed");
  }
  return code;
}
