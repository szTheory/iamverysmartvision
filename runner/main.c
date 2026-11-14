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
 * by a person, never by the gate, and refuses `user` content, host-dependent outcomes and a path that
 * is one of the operator output files (review WR-02). A run that ends harness-fail removes the operator
 * output files it created (review WR-01) and writes no golden; the golden is written to a temporary file and
 * renamed, so an existing one is never truncated by a failed run (Phase 6 review WR-02). Documented in docs/runner.md. */
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hostio.h"
#include "ivs/ivs.h"
#include "ivs_runner.h"
#include "runner_sha256.h"

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
  uint64_t video_written;          /* bytes written to the --video-raw file */
  uint64_t ppm_written;            /* bytes written to the --frame-ppm file */
  char *golden_buf;                /* report copy for --update-golden, NULL otherwise */
  size_t golden_len;
  int golden_overflow;             /* the report did not fit MAIN_GOLDEN_CAP */
  ivs_hostio_out audio_out;        /* --audio-raw file (plan 05-09), fd -1 when not given */
  ivs_hostio_out log_out;          /* --psg-log-out file */
  ivs_hostio_out wav_out;          /* --audio-wav file (plan 05-10): 44 reserved header bytes, then the PCM */
  ivs_hostio_out video_out;        /* --video-raw file (plan 06-12): the delivered slots, mode 0600 */
  ivs_hostio_out ppm_out;          /* --frame-ppm file (plan 06-12): the last frame as a PPM, mode 0600 */
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
    "                       refused for `user` content, host-dependent outcomes and an output file's path;\n"
    "                       a failed run (harness-fail) writes none and keeps an existing file\n"
    "  --audio-raw <file>   also write the dense audio sample bytes (4 per sample: levels A, B, C, 0)\n"
    "                       to a NEW file (an existing path is refused, exit 14, never overwritten);\n"
    "                       a run that ends harness-fail (exit 14) removes the files it created\n"
    "  --psg-log-out <file>  also write one ASCII line per PSG write-log record, `index tick addr\n"
    "                       value source` (decimal), to a NEW file (same rule)\n"
    "  --audio-out-max-bytes <n>  bound of each of those files in bytes, 1..1099511627776\n"
    "                       (default 268435456); passing it is a harness failure (exit 14) and the\n"
    "                       files of that run are removed, never a truncation on disk\n"
    "  --audio-wav <file>   also write the dense audio as a mono 16-bit WAV of the reference converter\n"
    "                       (reference conversion v1, converter repeatability only) to a NEW file (same\n"
    "                       rule); the report then ends with refpcm.rate, refpcm.frames and refpcm.sha256\n"
    "                       (the SHA-256 of the file as written); the file is bounded by\n"
    "                       --audio-out-max-bytes; never a golden, never a claim\n"
    "  --wav-rate <n>       the WAV rate, 44100 or 48000 (default 48000); needs --audio-wav\n"
    "  --video-raw <file>   also write the delivered video frame slots (each with its 32-byte header, so the\n"
    "                       stream is self-describing) to a NEW file created mode 0600 (same rules: an existing\n"
    "                       path is refused, a harness-fail run removes it; refused for `content <role> user`;\n"
    "                       a run that delivers no frame leaves an empty file and says so); never a golden,\n"
    "                       never evidence\n"
    "  --video-out-max-bytes <n>  bound of --video-raw and --frame-ppm in bytes, 1..1099511627776\n"
    "                       (default 268435456); passing it is a harness failure (exit 14), file removed\n"
    "  --frame-ppm <file>   also write the LAST delivered frame as a binary PPM (P6) in the operator preview\n"
    "                       palette (not STIC colour data) to a NEW file created mode 0600; refused for a\n"
    "                       scenario with `content <role> user`; an exported frame is never evidence\n"
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

static int mh_audio_raw(void *ctx, const uint8_t *bytes, size_t len)
{
  main_host *h = ctx;
  return ivs_hostio_out_write(&h->audio_out, bytes, len);
}

static int mh_video_raw(void *ctx, const uint8_t *bytes, size_t len)
{
  main_host *h = ctx;
  int r = ivs_hostio_out_write(&h->video_out, bytes, len);
  if (r == 0) {
    h->video_written += (uint64_t)len;
  }
  return r;
}

static int mh_frame_ppm(void *ctx, const uint8_t *ppm, size_t len)
{
  main_host *h = ctx;
  int r = ivs_hostio_out_write(&h->ppm_out, ppm, len);
  if (r == 0) {
    h->ppm_written += (uint64_t)len;
  }
  return r;
}

static int mh_psg_log(void *ctx, const char *lines, size_t len)
{
  main_host *h = ctx;
  return ivs_hostio_out_write(&h->log_out, lines, len);
}

static int mh_wav_pcm(void *ctx, const uint8_t *pcm, size_t len)
{
  main_host *h = ctx;
  return ivs_hostio_out_write(&h->wav_out, pcm, len);
}

/* The header goes in front of the PCM (the 44 bytes reserved at creation), then the finished file is read
 * back from offset 0 and hashed: the SHA-256 of the WAV file bytes as written. */
static int mh_wav_finish(void *ctx, const uint8_t header[44], uint8_t sha256[32])
{
  main_host *h = ctx;
  ivs_rsha256 c;
  uint8_t buf[16384];
  uint64_t off = 0u;
  size_t got = 0u;
  if (ivs_hostio_out_write_at(&h->wav_out, 0u, header, 44u) != 0) {
    return -1;
  }
  ivs_rsha256_init(&c);
  for (;;) {
    if (ivs_hostio_out_read_at(&h->wav_out, off, buf, sizeof buf, &got) != 0) {
      return -1;
    }
    if (got == 0u) {
      break;
    }
    ivs_rsha256_update(&c, buf, got);
    off += (uint64_t)got;
  }
  if (off < 44u) {
    return -1;
  }
  ivs_rsha256_final(&c, sha256);
  return 0;
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

/* Creates the operator output file exclusively (hostio.c). 0, or -1 after one line on stderr. */
static int main_open_out(ivs_hostio_out *o, const char *path, const char *option, unsigned int mode)
{
  int r = ivs_hostio_out_create(o, path, mode);
  if (r == 1) {
    fprintf(stderr, "ivs-run: %s: output file exists, refusing to overwrite it\n", option);
    return -1;
  }
  if (r != 0) {
    fprintf(stderr, "ivs-run: %s: cannot create the output file\n", option);
    return -1;
  }
  return 0;
}

/* The same, for a file that is also read back (the operator WAV, plan 05-10). */
static int main_open_out_rw(ivs_hostio_out *o, const char *path, const char *option)
{
  int r = ivs_hostio_out_create_rw(o, path, IVS_HOSTIO_MODE_SHARED);
  if (r == 1) {
    fprintf(stderr, "ivs-run: %s: output file exists, refusing to overwrite it\n", option);
    return -1;
  }
  if (r != 0) {
    fprintf(stderr, "ivs-run: %s: cannot create the output file\n", option);
    return -1;
  }
  return 0;
}

/* Closes o and removes the file this run created at path, when path still names it (hostio.c compares the
 * identity recorded at creation, so a replaced file is never removed). path NULL: not given, nothing to do.
 * option non-NULL: one line on stderr when a file was removed. Review WR-01. */
static void main_discard(ivs_hostio_out *o, const char *path, const char *option)
{
  if (path == NULL) {
    return;
  }
  (void)ivs_hostio_out_close(o);
  if (ivs_hostio_out_remove_if_same(o, path) == 0 && option != NULL) {
    fprintf(stderr, "ivs-run: %s: the run failed, the incomplete output file was removed\n", option);
  }
}

/* 1 when the two option values are both given and the same string. */
static int main_same_path(const char *a, const char *b) { return a != NULL && b != NULL && strcmp(a, b) == 0; }

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

/* The golden key list is ivs_runner_golden_keep (runner/report.c, plan 06-12: moved into the library so an in-process suite
 * reaches it); the same list sits in tools/check/steps/golden.sh and docs/runner.md (plan 05-09, A-09). live: video, audio,
 * input, psglog. */
#define MAIN_LIVE 4u
/* Suffix of the temporary file the golden is written to before the rename. */
#define MAIN_GOLDEN_TMP ".ivs-tmp"

/* Writes the .digests golden of the captured report to path. 0, or MAIN_EXIT_HARNESS after one
 * line on stderr (host-dependent or failed outcome, incomplete capture, write error). The golden is
 * written to a sibling temporary file and renamed over path only when the whole file was written, so a
 * failed run or a failed write never creates, truncates or replaces a golden (Phase 6 review WR-02). */
static int main_write_golden(const char *path, const main_host *h, const ivs_scenario *scn)
{
  static const char *const live_count[MAIN_LIVE] = {"output.video.bytes", "output.audio.bytes", "input.events",
                                                    "output.psglog.bytes"};
  const char *buf = h->golden_buf;
  size_t len = h->golden_len;
  const char *v;
  size_t vn;
  int live[MAIN_LIVE];
  FILE *f;
  char tmp_path[4096];
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
  /* A harness failure or a missing fixture is not a result of the machine: the report is short of the run. */
  if ((vn == 12u && memcmp(v, "harness-fail", 12u) == 0) || (vn == 15u && memcmp(v, "missing-fixture", 15u) == 0)) {
    return main_fail("golden refused: the run failed, no golden was written");
  }
  v = main_report_value(buf, len, "outcome.detail", &vn);
  if (vn >= 5u && memcmp(v, "wall ", 5u) == 0) {
    return main_fail("golden refused: host-dependent outcome");
  }
  for (size_t k = 0u; k < MAIN_LIVE; k++) {
    v = main_report_value(buf, len, live_count[k], &vn);
    live[k] = main_count_observed(v, vn);
  }
  if (strlen(path) + sizeof MAIN_GOLDEN_TMP > sizeof tmp_path) {
    return main_fail("golden write failed");
  }
  (void)snprintf(tmp_path, sizeof tmp_path, "%s%s", path, MAIN_GOLDEN_TMP);
  f = fopen(tmp_path, "wx");
  if (f == NULL) {
    return main_fail("golden write failed");
  }
  if (fprintf(f,
              "# tier: deterministic-repeatability\n"
              "# oracle: self-recorded\n"
              "# scenario: %s\n"
              "# profile: %s\n"
              "# domains: cpu=observed mem=observed images=observed instance=observed video=%s audio=%s "
              "input=%s psglog=%s\n"
              "# Written by ivs-run --update-golden, never by the gate; a change is a reviewed diff\n"
              "# (docs/runner.md). The body is the run's report filtered to the golden key list.\n",
              scn->id, scn->profile, live[0] ? "observed" : "none", live[1] ? "observed" : "none",
              live[2] ? "observed" : "none", live[3] ? "observed" : "none") < 0) {
    ok = 0;
  }
  while (ok && i < len) {
    const char *nl = memchr(buf + i, '\n', len - i);
    size_t n = (size_t)(nl - (buf + i)) + 1u;
    if (ivs_runner_golden_keep(buf + i, n - 1u, live) && fwrite(buf + i, 1u, n, f) != n) {
      ok = 0;
    }
    i += n;
  }
  if (fclose(f) != 0) {
    ok = 0;
  }
  if (ok && rename(tmp_path, path) != 0) {
    ok = 0;
  }
  if (!ok) {
    (void)remove(tmp_path);
    return main_fail("golden write failed");
  }
  return 0;
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
  uint8_t *buffers = NULL;
  const char *scenario_path = NULL;
  const char *golden_path = NULL;
  const char *audio_raw_path = NULL;
  const char *psg_log_path = NULL;
  const char *audio_wav_path = NULL;
  const char *video_raw_path = NULL;
  const char *frame_ppm_path = NULL;
  char dirbuf[MAIN_DIR_MAX];
  size_t text_len = 0u;
  size_t storage_bytes;
  int r;
  int code;
  int have_max = 0;
  int have_wall = 0;
  int have_slice = 0;
  int have_out_max = 0;
  int have_wav_rate = 0;
  int have_vout_max = 0;
  memset(&mh, 0, sizeof mh);
  mh.audio_out.fd = -1;
  mh.log_out.fd = -1;
  mh.wav_out.fd = -1;
  mh.video_out.fd = -1;
  mh.ppm_out.fd = -1;
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
    } else if (strcmp(a, "--audio-raw") == 0) {
      slot = &audio_raw_path;
    } else if (strcmp(a, "--psg-log-out") == 0) {
      slot = &psg_log_path;
    } else if (strcmp(a, "--audio-wav") == 0) {
      slot = &audio_wav_path;
    } else if (strcmp(a, "--video-raw") == 0) {
      slot = &video_raw_path;
    } else if (strcmp(a, "--frame-ppm") == 0) {
      slot = &frame_ppm_path;
    } else if (strcmp(a, "--video-out-max-bytes") == 0) {
      seen = &have_vout_max;
      lo = 1u;
      hi = UINT64_C(1099511627776);
    } else if (strcmp(a, "--wav-rate") == 0) {
      seen = &have_wav_rate;
      lo = 1u;
      hi = UINT32_MAX;
    } else if (strcmp(a, "--audio-out-max-bytes") == 0) {
      seen = &have_out_max;
      lo = 1u;
      hi = UINT64_C(1099511627776);
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
    if (seen == &have_wav_rate && !ivs_runner_wav_rate_valid((uint32_t)v)) {
      return main_usage_error("--wav-rate must be 44100 or 48000");
    }
    *seen = 1;
    if (seen == &have_wav_rate) {
      opts.wav_rate = (uint32_t)v;
    } else if (seen == &have_max) {
      opts.max_ticks = v;
    } else if (seen == &have_wall) {
      opts.wall_timeout_ms = (uint32_t)v;
    } else if (seen == &have_slice) {
      opts.slice_ticks = v;
    } else if (seen == &have_out_max) {
      opts.audio_out_max_bytes = v;
    } else if (seen == &have_vout_max) {
      opts.video_out_max_bytes = v;
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
  if (have_wav_rate && audio_wav_path == NULL) {
    return main_usage_error("--wav-rate needs --audio-wav");
  }
  /* --update-golden overwrites its file, so it must never be one of the operator output files (review WR-02):
   * the same string is refused here, before anything is created; the same file under another spelling is
   * refused below once the outputs exist. */
  if (main_same_path(golden_path, audio_raw_path) || main_same_path(golden_path, psg_log_path) ||
      main_same_path(golden_path, audio_wav_path) || main_same_path(golden_path, video_raw_path) ||
      main_same_path(golden_path, frame_ppm_path)) {
    return main_fail("--update-golden must not name an operator output file");
  }
  /* Two operator outputs on one path: refused before anything is created, naming the clash (Phase 6 review IN-05; without this the
   * second create reported "output file exists" for a path that was free). The same file under another spelling is still caught by
   * the exclusive create. */
  {
    const char *const out_path[5] = {audio_raw_path, psg_log_path, audio_wav_path, video_raw_path, frame_ppm_path};
    static const char *const out_opt[5] = {"--audio-raw", "--psg-log-out", "--audio-wav", "--video-raw", "--frame-ppm"};
    for (size_t a = 0u; a < 5u; a++) {
      for (size_t b = a + 1u; b < 5u; b++) {
        if (main_same_path(out_path[a], out_path[b])) {
          fprintf(stderr, "ivs-run: %s: names the same path as %s\n", out_opt[b], out_opt[a]);
          return MAIN_EXIT_HARNESS;
        }
      }
    }
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
  /* A frame rendered from an owner image is derived art of rights-bearing content: no frame export for a scenario that names
   * `content ... user` (plan 06-12, T-06-17); refused before anything is created, no report. --video-raw writes the same pixels as
   * index bytes, so it follows the same rule (Phase 6 review IN-01). */
  if (frame_ppm_path != NULL || video_raw_path != NULL) {
    for (uint32_t role = 0u; role < IVS_SCN_ROLES; role++) {
      if (scn->content[role].kind == IVS_SCN_CONTENT_USER) {
        free(scn);
        return main_fail(frame_ppm_path != NULL ? "--frame-ppm refused: content user" : "--video-raw refused: content user");
      }
    }
  }
  /* The sound of an owner image is derived material of rights-bearing content too: the PCM bytes, the PSG write log and the WAV of a
   * scenario that names `content ... user` are refused like the frame outputs above (plan 07-11, D-31, T-07-29; the Phase 5 audio
   * options had no such refusal). Refused before any storage or file is created, no report; when several of the three options are
   * given the first of this fixed order is named. */
  if (audio_raw_path != NULL || psg_log_path != NULL || audio_wav_path != NULL) {
    for (uint32_t role = 0u; role < IVS_SCN_ROLES; role++) {
      if (scn->content[role].kind == IVS_SCN_CONTENT_USER) {
        free(scn);
        return main_fail(audio_raw_path != NULL   ? "--audio-raw refused: content user"
                         : psg_log_path != NULL ? "--psg-log-out refused: content user"
                                                : "--audio-wav refused: content user");
      }
    }
  }
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
    storage_bytes = (size_t)req.size_bytes + (size_t)req.align_bytes + (size_t)IVS_RUNNER_CAP_CART + 1u +
                    (size_t)IVS_RUNNER_CAPTURE_BYTES;
  }
  storage = malloc(storage_bytes);
  /* One output block for the three channels (video, audio, PSG write log), bounded by
   * IVS_RUNNER_BUFFERS_MAX; the executor cuts it per slice from ivs_info (plan 05-09, A-11). */
  buffers = malloc((size_t)IVS_RUNNER_BUFFERS_MAX);
  if (storage == NULL || buffers == NULL) {
    free(storage);
    free(buffers);
    free(mh.golden_buf);
    free(scn);
    return main_fail("out of memory");
  }

  /* The operator's raw outputs, created exclusively before the run: an existing path is refused with
   * one line and exit 14, nothing is overwritten (plan 05-09, T-05-23). */
  if (audio_raw_path != NULL && main_open_out(&mh.audio_out, audio_raw_path, "--audio-raw", IVS_HOSTIO_MODE_SHARED) != 0) {
    free(storage);
    free(buffers);
    free(mh.golden_buf);
    free(scn);
    return MAIN_EXIT_HARNESS;
  }
  if (psg_log_path != NULL && main_open_out(&mh.log_out, psg_log_path, "--psg-log-out", IVS_HOSTIO_MODE_SHARED) != 0) {
    main_discard(&mh.audio_out, audio_raw_path, NULL);
    free(storage);
    free(buffers);
    free(mh.golden_buf);
    free(scn);
    return MAIN_EXIT_HARNESS;
  }
  /* The operator WAV (plan 05-10): created exclusively like the other files, 44 bytes reserved for the
   * header (written after the stream is complete, so the file is never seen with a wrong length), then the
   * PCM follows. A refusal abandons the files this run already created. */
  if (audio_wav_path != NULL) {
    static const uint8_t zero_header[44] = {0};
    int wr = main_open_out_rw(&mh.wav_out, audio_wav_path, "--audio-wav");
    if (wr == 0 && ivs_hostio_out_write(&mh.wav_out, zero_header, sizeof zero_header) != 0) {
      fprintf(stderr, "ivs-run: --audio-wav: cannot write the output file\n");
      main_discard(&mh.wav_out, audio_wav_path, NULL);
      wr = -1;
    }
    if (wr != 0) {
      main_discard(&mh.audio_out, audio_raw_path, NULL);
      main_discard(&mh.log_out, psg_log_path, NULL);
      free(storage);
      free(buffers);
      free(mh.golden_buf);
      free(scn);
      return MAIN_EXIT_HARNESS;
    }
  }
  /* The operator's video outputs (plan 06-12): the same exclusive create, created mode 0600; a refusal abandons the files
   * this run already created (review WR-01 form). */
  if ((video_raw_path != NULL && main_open_out(&mh.video_out, video_raw_path, "--video-raw", IVS_HOSTIO_MODE_PRIVATE) != 0) ||
      (frame_ppm_path != NULL && main_open_out(&mh.ppm_out, frame_ppm_path, "--frame-ppm", IVS_HOSTIO_MODE_PRIVATE) != 0)) {
    main_discard(&mh.audio_out, audio_raw_path, NULL);
    main_discard(&mh.log_out, psg_log_path, NULL);
    main_discard(&mh.wav_out, audio_wav_path, NULL);
    main_discard(&mh.video_out, video_raw_path, NULL);
    main_discard(&mh.ppm_out, frame_ppm_path, NULL);
    free(storage);
    free(buffers);
    free(mh.golden_buf);
    free(scn);
    return MAIN_EXIT_HARNESS;
  }
  /* The golden path against the files just created, whatever its spelling or links (review WR-02). */
  if (golden_path != NULL && (ivs_hostio_out_is_path(&mh.audio_out, golden_path) == 1 ||
                              ivs_hostio_out_is_path(&mh.log_out, golden_path) == 1 ||
                              ivs_hostio_out_is_path(&mh.wav_out, golden_path) == 1 ||
                              ivs_hostio_out_is_path(&mh.video_out, golden_path) == 1 ||
                              ivs_hostio_out_is_path(&mh.ppm_out, golden_path) == 1)) {
    main_discard(&mh.audio_out, audio_raw_path, NULL);
    main_discard(&mh.log_out, psg_log_path, NULL);
    main_discard(&mh.wav_out, audio_wav_path, NULL);
    main_discard(&mh.video_out, video_raw_path, NULL);
    main_discard(&mh.ppm_out, frame_ppm_path, NULL);
    free(storage);
    free(buffers);
    free(mh.golden_buf);
    free(scn);
    return main_fail("--update-golden must not name an operator output file");
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
  host.buffers = buffers;
  host.buffer_bytes = (size_t)IVS_RUNNER_BUFFERS_MAX;
  if (audio_raw_path != NULL) {
    host.write_audio_raw = mh_audio_raw;
  }
  if (psg_log_path != NULL) {
    host.write_psg_log = mh_psg_log;
  }
  if (audio_wav_path != NULL) {
    host.write_wav_pcm = mh_wav_pcm;
    host.finish_wav = mh_wav_finish;
  }
  if (video_raw_path != NULL) {
    host.write_video_raw = mh_video_raw;
  }
  if (frame_ppm_path != NULL) {
    host.write_frame_ppm = mh_frame_ppm;
  }
  memset(&res, 0, sizeof res);
  (void)ivs_runner_execute(scn, &host, &opts, &res);
  free(storage);
  free(buffers);
  code = (int)res.exit_code;
  if (audio_raw_path != NULL && ivs_hostio_out_close(&mh.audio_out) != 0) {
    code = MAIN_EXIT_HARNESS;
    fprintf(stderr, "ivs-run: --audio-raw: close failed\n");
  }
  if (psg_log_path != NULL && ivs_hostio_out_close(&mh.log_out) != 0) {
    code = MAIN_EXIT_HARNESS;
    fprintf(stderr, "ivs-run: --psg-log-out: close failed\n");
  }
  if (audio_wav_path != NULL && ivs_hostio_out_close(&mh.wav_out) != 0) {
    code = MAIN_EXIT_HARNESS;
    fprintf(stderr, "ivs-run: --audio-wav: close failed\n");
  }
  if (video_raw_path != NULL && ivs_hostio_out_close(&mh.video_out) != 0) {
    code = MAIN_EXIT_HARNESS;
    fprintf(stderr, "ivs-run: --video-raw: close failed\n");
  }
  if (frame_ppm_path != NULL && ivs_hostio_out_close(&mh.ppm_out) != 0) {
    code = MAIN_EXIT_HARNESS;
    fprintf(stderr, "ivs-run: --frame-ppm: close failed\n");
  }
  /* A run that ends without a failure but delivered no frame leaves an empty file: say so (Phase 6 review IN-01), exit code and
   * report unchanged. */
  if (code != MAIN_EXIT_HARNESS) {
    if (video_raw_path != NULL && mh.video_written == 0u) {
      fprintf(stderr, "ivs-run: --video-raw: no frame was delivered, the file is empty\n");
    }
    if (frame_ppm_path != NULL && mh.ppm_written == 0u) {
      fprintf(stderr, "ivs-run: --frame-ppm: no frame was delivered, the file is empty\n");
    }
  }
  if (fflush(stdout) != 0 || ferror(stdout) || res.report_write_failed != 0u) {
    code = MAIN_EXIT_HARNESS;
    fprintf(stderr, "ivs-run: report write failed\n");
  } else if (golden_path != NULL && main_write_golden(golden_path, &mh, scn) != 0) {
    code = MAIN_EXIT_HARNESS;
  }
  /* A run that ends harness-fail leaves no operator output file (review WR-01): what the files hold may be
   * short of the run (a bound hit, a failed write), and an existing file would block the retry. Every other
   * outcome ends with files that hold the whole run up to its end. */
  if (code == MAIN_EXIT_HARNESS) {
    main_discard(&mh.audio_out, audio_raw_path, "--audio-raw");
    main_discard(&mh.log_out, psg_log_path, "--psg-log-out");
    main_discard(&mh.wav_out, audio_wav_path, "--audio-wav");
    main_discard(&mh.video_out, video_raw_path, "--video-raw");
    main_discard(&mh.ppm_out, frame_ppm_path, "--frame-ppm");
  }
  free(mh.golden_buf);
  free(scn);
  return code;
}
