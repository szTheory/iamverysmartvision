/* SPDX-License-Identifier: MIT */
/* The independent consumer of the installed ivs package (PKG-01, D-34, D-39, plan 04-23).
 * Built only through find_package(ivs) against the installed, relocated prefix (gate step
 * pkg-install). Usage: consumer <boot-stub.exec> <diag-cpu-1.ivsc> <diag-ctrl-psg-1.ivsc> <diag-stic-1.ivsc>; the
 * images come from the build tree's tools/pack output and the 2048-byte zero GROM is made here, so
 * the program embeds no content bytes. It replays the runner scenario
 * data/scenarios/diag-input-pass.ivss through the public API alone:
 *   version tie   ivs_VERSION (from the package config), IVS_API_VERSION, ivs_api_version();
 *   v1 sizes      the frozen v1 struct sizes as the installed headers declare them;
 *   create        storage sized by ivs_machine_storage_requirements, aligned by hand;
 *   refusals      a truncated EXEC (MALFORMED) and a duplicate cartridge load (DUPLICATE_LOAD),
 *                 every state digest equal before and after each;
 *   load, reset   boot stub, zero GROM, diagnostic cartridge, power-on reset;
 *   bounded run   run-to 1000 ticks, then run-to 16000 ticks stopping at the halt, with the
 *                 scenario's two input events (tick 400 controller 0 KEY_1, tick 1200 none),
 *                 the unconsumed suffix resubmitted on the second call;
 *   digests       one digest.<domain> line per observed domain (video, audio and psglog only when
 *                 the session's calls delivered bytes of that channel: the count rule of the
 *                 runner golden), the form of the runner golden; the replay passes an audio and a
 *                 write-log buffer sized from ivs_info, as the runner does;
 *   second instance  a fresh instance without the refusals must give the same digests.
 * Plan 05-11 (D-19, D-05, V-14) adds a second replay, of data/scenarios/diag-ctrl-psg-pass.ivss, whose
 * lines carry the prefix `ctrl.`: the seven controller events of that scenario through the installed
 * headers, an audio and a write-log buffer sized from ivs_info (never hard-coded), a first run to
 * tick 4100 after which the peeked $01FE is printed (0x007E, KEY_1 on port 0), then the run to the
 * halt, the guest words at 020F and 0210..0227, and one ctrl.digest.<domain> line per observed domain
 * (audio and psglog included; video not), equal to data/golden/diag-ctrl-psg-pass.digests.
 * Plan 06-07 (D-04, VID-06) adds the STIC inspection call: after the first session's last run call and its
 * digests one ivs_machine_stic_state call on the replay machine prints a `stic-state` line (the stored view,
 * never a digest) and the two refusals `stic-state.refusal.null` and `.small`; the installed header's
 * ivs_stic_report size is checked at compile time. These lines carry no digest, so the comparison with the
 * runner golden (digest lines and observed domains) is unaffected.
 * Plan 06-13 (D-17, D-20, F-14, SC3) adds a third replay, of data/scenarios/diag-stic-fgbg-pass.ivss, whose lines carry the prefix
 * `stic.`: the third diagnostic (no input) through the installed headers, a video, an audio and a write-log buffer sized from ivs_info
 * (the video buffer includes video_slack_frames; no size is hard-coded), one run to the halt, then the first slot header (index 0, tick
 * 59,736, the geometry of ivs_info, flags 1), twelve literal pixels and two rectangles taken from the hand derivation
 * (docs/video/diag-stic-1-derivation.md, F-D1; named constants below), video_bytes_written equal to frames x video_slot_bytes, frame
 * index continuity and strictly increasing ticks, the guest words, a `stic.stic-state` line, and a second pass on a fresh machine with
 * IVS_RUN_F_STOP_ON_FRAME (one slot per call) whose frame stream and end digests must equal the first pass. One stic.digest.<domain> line is
 * printed per observed domain, observed by the count rule (a domain is observed when its delivered byte or event count is nonzero: video
 * and audio here, the write log and the input events not), and they equal data/golden/diag-stic-fgbg-pass.digests. The consumer
 * has no SHA-256 code: the chain recompute stays in the runner.
 * Stdout is pinned by data/expected/pkg-consumer.golden and its digest lines must equal those of
 * data/golden/diag-input-pass.digests. Exit 0 only when every check held; a failed check prints
 * a `FAIL <what>` line. No path, time or address is printed (the output is host independent).
 * C rule: one call per statement when one argument reads state another argument's call changes. */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ivs/ivs.h>

#define C_PROFILE "intv-2609-ntsc-family-p1"
#define C_EXEC_BYTES 8192u
#define C_GROM_BYTES 2048u
#define C_CART_MAX 131256u
#define C_DOMAINS 9u
#define C_EVENTS 2u
#define C_CTRL_EVENTS 7u
/* Plan 06-13: the third diagnostic's hand-derived constants (docs/video/diag-stic-1-derivation.md; the frame F-D1 of data/video/frames). */
#define C_STIC_TARGET 150000u    /* the run-to bound of data/scenarios/diag-stic-fgbg-pass.ivss */
#define C_STIC_FIRST_TICK 59736u /* E_0: the end of frame 0 */
#define C_STIC_FRAMES_MAX 4u

typedef struct c_images {
  uint8_t exec[C_EXEC_BYTES + 1u];
  size_t exec_len;
  uint8_t grom[C_GROM_BYTES];
  uint8_t cart[C_CART_MAX + 1u];
  size_t cart_len;
  uint8_t cart2[C_CART_MAX + 1u]; /* the second diagnostic (plan 05-11) */
  size_t cart2_len;
  uint8_t cart3[C_CART_MAX + 1u]; /* the third diagnostic (plan 06-13) */
  size_t cart3_len;
} c_images;

typedef struct c_digests {
  uint8_t sha[C_DOMAINS][32];
} c_digests;

static c_images images;
static int failures;

/* Digest domains in the order of the runner report; observed[i] says whether a line is printed. */
static const uint32_t c_domain[C_DOMAINS] = {IVS_DIGEST_CPU,       IVS_DIGEST_MEM,   IVS_DIGEST_IMAGES,
                                             IVS_DIGEST_INSTANCE,  IVS_DIGEST_COMPOSITE, IVS_DIGEST_VIDEO,
                                             IVS_DIGEST_AUDIO,     IVS_DIGEST_INPUT, IVS_DIGEST_PSGLOG};
static const char *const c_domain_name[C_DOMAINS] = {"cpu",       "mem",   "images", "instance", "composite",
                                                     "video",     "audio", "input",  "psglog"};

static void fail(const char *what)
{
  printf("FAIL %s\n", what);
  failures++;
}

static const char *status_token(ivs_status st) { return ivs_status_name(st); }

static void print_hex(const uint8_t *b, size_t n)
{
  for (size_t i = 0u; i < n; i++) {
    printf("%02x", (unsigned)b[i]);
  }
}

/* Reads at most cap bytes of path into buf; a longer file is refused. Returns 1 on success. */
static int read_image(const char *path, uint8_t *buf, size_t cap, size_t *len)
{
  FILE *f = fopen(path, "rb");
  size_t n;
  if (f == NULL) {
    return 0;
  }
  n = fread(buf, 1u, cap + 1u, f);
  if (ferror(f) != 0 || n > cap) {
    (void)fclose(f);
    return 0;
  }
  if (fclose(f) != 0) {
    return 0;
  }
  *len = n;
  return 1;
}

/* Every digest domain of m into d. Returns 1 when every call succeeded. */
static int digest_all(const ivs_machine *m, c_digests *d)
{
  for (size_t i = 0u; i < C_DOMAINS; i++) {
    ivs_status st = ivs_machine_state_digest(m, c_domain[i], d->sha[i]);
    if (st != IVS_OK) {
      return 0;
    }
  }
  return 1;
}

typedef struct c_instance {
  void *raw;
  ivs_machine *m;
} c_instance;

/* Creates an instance in heap storage the host owns, aligned by hand. Returns 1 on success. */
static int instance_create(c_instance *x)
{
  ivs_storage_requirements req;
  ivs_create_options co;
  ivs_status st;
  uintptr_t base;
  uintptr_t aligned;
  size_t total;
  memset(x, 0, sizeof *x);
  memset(&req, 0, sizeof req);
  req.struct_size = (uint32_t)sizeof req;
  st = ivs_machine_storage_requirements(C_PROFILE, &req);
  if (st != IVS_OK || req.align_bytes == 0u || (req.align_bytes & (req.align_bytes - 1u)) != 0u ||
      req.size_bytes > (uint64_t)(SIZE_MAX / 2u) || req.align_bytes > (uint64_t)(SIZE_MAX / 2u)) {
    return 0;
  }
  total = (size_t)req.size_bytes + (size_t)req.align_bytes;
  x->raw = malloc(total);
  if (x->raw == NULL) {
    return 0;
  }
  base = (uintptr_t)x->raw;
  aligned = (base + (uintptr_t)(req.align_bytes - 1u)) & ~(uintptr_t)(req.align_bytes - 1u);
  memset(&co, 0, sizeof co);
  co.struct_size = (uint32_t)sizeof co;
  co.profile_id = C_PROFILE;
  st = ivs_machine_create(&co, (void *)aligned, (size_t)req.size_bytes, &x->m);
  if (st != IVS_OK) {
    free(x->raw);
    x->raw = NULL;
    x->m = NULL;
    return 0;
  }
  return 1;
}

static void instance_destroy(c_instance *x)
{
  if (x->m != NULL) {
    if (ivs_machine_destroy(x->m) != IVS_OK) {
      fail("destroy");
    }
    x->m = NULL;
  }
  free(x->raw);
  x->raw = NULL;
}

/* One load with a report. */
static ivs_status load(ivs_machine *m, uint32_t kind, const uint8_t *bytes, size_t len, ivs_load_report *lr)
{
  memset(lr, 0, sizeof *lr);
  lr->struct_size = (uint32_t)sizeof *lr;
  lr->image.struct_size = (uint32_t)sizeof lr->image;
  return ivs_machine_load(m, kind, bytes, len, NULL, lr);
}

/* A load that must be refused with want_st and want_reason and leave every digest unchanged. */
static void refusal(ivs_machine *m, const char *name, uint32_t kind, const uint8_t *bytes, size_t len, ivs_status want_st,
                    uint32_t want_reason, int print)
{
  c_digests before;
  c_digests after;
  ivs_load_report lr;
  ivs_status st;
  int same;
  if (!digest_all(m, &before)) {
    fail("refusal digest before");
    return;
  }
  st = load(m, kind, bytes, len, &lr);
  if (!digest_all(m, &after)) {
    fail("refusal digest after");
    return;
  }
  same = memcmp(&before, &after, sizeof before) == 0;
  if (print) {
    printf("refusal.%s status=%s reason=%u digest-unchanged=%s\n", name, status_token(st), (unsigned)lr.reason,
           same ? "yes" : "no");
  }
  if (st != want_st || lr.reason != want_reason || !same) {
    fail(name);
  }
}

/* Bytes of video, audio and write log the session's calls delivered (reports): the replay requests the
 * audio, the log and (plan 06-09) the video frames through buffers sized from ivs_info, as the runner does
 * (plan 05-09; video: video_bytes_per_frame x (slots + video_slack_frames), the slack from ivs_info v4, never
 * a hard-coded size). A domain is observed when its delivered bytes are nonzero, the
 * count rule of the runner golden (B1): the layout in ivs_info says what a call could deliver, not
 * what this session observed. */
static uint64_t delivered_video_bytes;
static uint64_t delivered_audio_bytes;
static uint64_t delivered_psglog_bytes;
static uint64_t delivered_frames;
/* With keep_video set, run_step appends the delivered video bytes to kept_video (the third replay compares them). */
static int keep_video;
static uint8_t *kept_video;
static uint64_t kept_video_n;
/* The line prefix of run_step's printed lines: empty for the diag-input-pass replay, `ctrl.` for the
 * diag-ctrl-psg-pass replay (plan 05-11). */
static const char *run_prefix = "";

/* One run call of the replay; returns the report's input_consumed (0 on a refusal). The audio and
 * log capacities are bytes_per_frame * ((target - now) / nominal_frame_ticks + 1) from ivs_info, the
 * most the call can require (ivs_run.h); the video capacity adds the layout's slack frames (a call can
 * complete one frame event more than that count). */
static uint32_t run_step(ivs_machine *m, unsigned step, uint64_t target, const ivs_input_event *events, uint32_t count,
                         uint32_t want_stop, int print)
{
  ivs_run_request q;
  ivs_run_report rep;
  ivs_info info;
  ivs_status st;
  uint8_t *audio = NULL;
  uint8_t *plog = NULL;
  uint8_t *video = NULL;
  uint64_t slots;
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  st = ivs_machine_info(m, &info);
  if (st != IVS_OK || info.nominal_frame_ticks == 0u) {
    fail("info");
    return 0u;
  }
  slots = (target > info.time_ticks ? target - info.time_ticks : 0u) / info.nominal_frame_ticks + 1u;
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.flags = IVS_RUN_F_STOP_ON_HALT;
  q.target_ticks = target;
  q.events = count > 0u ? events : NULL;
  q.event_count = count;
  if (info.video_bytes_per_frame != 0u) {
    q.video_capacity = (uint64_t)info.video_bytes_per_frame * (slots + (uint64_t)info.video_slack_frames);
    video = malloc((size_t)q.video_capacity);
    q.video = video;
  }
  if (info.audio_bytes_per_frame != 0u) {
    q.audio_capacity = (uint64_t)info.audio_bytes_per_frame * slots;
    audio = malloc((size_t)q.audio_capacity);
    q.audio = audio;
  }
  if (info.psg_log_record_bytes != 0u && info.psg_log_records_per_frame != 0u) {
    q.psg_log_capacity = (uint64_t)info.psg_log_record_bytes * info.psg_log_records_per_frame * slots;
    plog = malloc((size_t)q.psg_log_capacity);
    q.psg_log = plog;
  }
  if ((q.video_capacity != 0u && video == NULL) || (q.audio_capacity != 0u && audio == NULL) ||
      (q.psg_log_capacity != 0u && plog == NULL)) {
    free(video);
    free(audio);
    free(plog);
    fail("buffers");
    return 0u;
  }
  memset(&rep, 0, sizeof rep);
  rep.struct_size = (uint32_t)sizeof rep;
  st = ivs_machine_run(m, &q, &rep);
  if (st == IVS_OK && keep_video && rep.video_bytes_written != 0u) {
    uint8_t *grown = realloc(kept_video, (size_t)(kept_video_n + rep.video_bytes_written));
    if (grown == NULL) {
      fail("video keep");
    } else {
      kept_video = grown;
      memcpy(kept_video + kept_video_n, video, (size_t)rep.video_bytes_written);
      kept_video_n += rep.video_bytes_written;
    }
  }
  free(video);
  free(audio);
  free(plog);
  if (st != IVS_OK) {
    if (print) {
      printf("%sstep.%u status=%s reason=%u\n", run_prefix, step, status_token(st), (unsigned)rep.refusal_reason);
    }
    fail("run refused");
    return 0u;
  }
  if (print) {
    printf("%sstep.%u target_ticks=%llu stop=%s now_ticks=%llu overshoot_ticks=%llu input_consumed=%u "
           "controller0=%08x\n",
           run_prefix, step, (unsigned long long)target, ivs_run_stop_name(rep.stop_reason), (unsigned long long)rep.now_ticks,
           (unsigned long long)rep.overshoot_ticks, (unsigned)rep.input_consumed, (unsigned)rep.controller_now[0]);
  }
  if (rep.stop_reason != want_stop) {
    fail("stop reason");
  }
  delivered_video_bytes += rep.video_bytes_written;
  delivered_frames += rep.video_frames_completed;
  delivered_audio_bytes += rep.audio_bytes_written;
  delivered_psglog_bytes += rep.psg_log_bytes_written;
  return rep.input_consumed;
}

/* The CPU run state and registers, and the words at 0200..020F (the scenario's assert range). */
static void observe(const ivs_machine *m, const char *tag)
{
  ivs_cpu_state_report c;
  uint16_t w[16];
  ivs_status st;
  memset(&c, 0, sizeof c);
  c.struct_size = (uint32_t)sizeof c;
  st = ivs_machine_cpu_state(m, &c);
  if (st != IVS_OK) {
    fail("cpu_state");
    return;
  }
  st = ivs_machine_peek(m, 0x0200u, 16u, w);
  if (st != IVS_OK) {
    fail("peek");
    return;
  }
  printf("%s.cpu run_state=%u fault_reason=%u r=", tag, (unsigned)c.run_state, (unsigned)c.fault_reason);
  for (size_t i = 0u; i < 8u; i++) {
    printf("%s%04X", i == 0u ? "" : ",", (unsigned)c.r[i]);
  }
  printf("\n%s.mem 0200..020F=", tag);
  for (size_t i = 0u; i < 16u; i++) {
    printf("%s%04X", i == 0u ? "" : ",", (unsigned)w[i]);
  }
  printf("\n");
}

/* The STIC inspection call on a machine (plan 06-07): the stored view and two refusals. Each call is its own statement. */
static int stic_state_line(const ivs_machine *m, const char *prefix)
{
  ivs_stic_report r;
  ivs_status st;
  memset(&r, 0, sizeof r);
  r.struct_size = IVS_STIC_REPORT_V1_SIZE;
  st = ivs_machine_stic_state(m, &r);
  if (st != IVS_OK || r.struct_size != IVS_STIC_REPORT_V1_SIZE) {
    fail("stic_state");
    return 0;
  }
  printf("%sstic-state mode=%u latch=%u windows=%u regs-window=%u gmem-window=%u event=%u frame=%llu blocked=%u/%u\n", prefix,
         (unsigned)r.mode, (unsigned)r.display_latch, (unsigned)r.windows_active, (unsigned)r.regs_window_open,
         (unsigned)r.gmem_window_open, (unsigned)r.event_index, (unsigned long long)r.frame_index,
         (unsigned)r.blocked_reads, (unsigned)r.blocked_writes);
  return 1;
}

static void stic_state_lines(const ivs_machine *m)
{
  ivs_stic_report r;
  ivs_status st_null;
  ivs_status st_small;
  if (!stic_state_line(m, "")) {
    return;
  }
  st_null = ivs_machine_stic_state(m, NULL);
  memset(&r, 0xA5, sizeof r);
  r.struct_size = IVS_STIC_REPORT_V1_SIZE - 1u;
  st_small = ivs_machine_stic_state(m, &r);
  printf("stic-state.refusal.null status=%s\n", status_token(st_null));
  printf("stic-state.refusal.small status=%s written=%s\n", status_token(st_small),
         r.mode == 0xA5A5A5A5u && r.regs[63] == 0xA5A5u && r.struct_size == IVS_STIC_REPORT_V1_SIZE - 1u ? "no" : "yes");
  if (st_null != IVS_E_BAD_ARG || st_small != IVS_E_BAD_ARG) {
    fail("stic_state refusals");
  }
}

/* One whole session on a fresh instance: optional refusals, loads, reset, the two-step replay,
 * final digests into d. print selects the pinned output (the first instance only). */
static void session(int with_refusals, int print, c_digests *d, int *observed)
{
  c_instance x;
  ivs_load_report lr;
  ivs_info info;
  ivs_status st;
  ivs_input_event ev[C_EVENTS];
  uint32_t consumed = 0u;
  static const uint32_t kinds[3] = {IVS_KIND_EXEC, IVS_KIND_GROM, IVS_KIND_CARTRIDGE};
  static const char *const kind_name[3] = {"exec", "grom", "cart"};
  const uint8_t *bytes[3];
  size_t lens[3];
  bytes[0] = images.exec;
  bytes[1] = images.grom;
  bytes[2] = images.cart;
  lens[0] = images.exec_len;
  lens[1] = C_GROM_BYTES;
  lens[2] = images.cart_len;
  memset(d, 0, sizeof *d);
  delivered_video_bytes = 0u;
  delivered_audio_bytes = 0u;
  delivered_psglog_bytes = 0u;
  if (!instance_create(&x)) {
    fail("create");
    return;
  }
  if (print) {
    printf("create status=IVS_OK profile=%s\n", C_PROFILE);
  }
  if (with_refusals) {
    refusal(x.m, "truncated-exec", IVS_KIND_EXEC, images.exec, images.exec_len - 1u, IVS_E_MALFORMED,
            IVS_REASON_IMAGE_SHORT, print);
  }
  for (size_t i = 0u; i < 3u; i++) {
    st = load(x.m, kinds[i], bytes[i], lens[i], &lr);
    if (print) {
      printf("load.%s status=%s length_bytes=%u sha256=", kind_name[i], status_token(st), (unsigned)lr.image.length_bytes);
      print_hex(lr.image.sha256, sizeof lr.image.sha256);
      printf("\n");
    }
    if (st != IVS_OK) {
      fail("load");
      instance_destroy(&x);
      return;
    }
  }
  if (with_refusals) {
    refusal(x.m, "duplicate", IVS_KIND_CARTRIDGE, images.cart, images.cart_len, IVS_E_DUPLICATE_LOAD,
            IVS_REASON_SLOT_OCCUPIED, print);
  }
  st = ivs_machine_reset(x.m, IVS_RESET_POWER_ON);
  if (print) {
    printf("reset status=%s\n", status_token(st));
  }
  if (st != IVS_OK) {
    fail("reset");
    instance_destroy(&x);
    return;
  }
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  st = ivs_machine_info(x.m, &info);
  if (st != IVS_OK || info.phase != IVS_PHASE_POWERED || info.time_ticks != 0u) {
    fail("info");
    instance_destroy(&x);
    return;
  }
  if (print) {
    printf("info phase=%u time_ticks=%llu ticks_per_cpu_cycle=%u nominal_frame_ticks=%llu video_format=%u "
           "audio_format=%u\n",
           (unsigned)info.phase, (unsigned long long)info.time_ticks, (unsigned)info.ticks_per_cpu_cycle,
           (unsigned long long)info.nominal_frame_ticks, (unsigned)info.video_format, (unsigned)info.audio_format);
  }
  /* The scenario's targets are in master ticks already (run-to <n> ticks): the multiplier is 1.
   * Its input lines: at_tick=400 c0=KEY_1 c1=0x0, then at_tick=1200 c0=0x0 c1=0x0. */
  memset(ev, 0, sizeof ev);
  ev[0].at_tick = 400u;
  ev[0].controller[0] = IVS_CTRL_KEY_1;
  ev[1].at_tick = 1200u;
  consumed += run_step(x.m, 1u, 1000u, &ev[0], C_EVENTS, IVS_STOP_TARGET, print);
  if (print) {
    observe(x.m, "step.1");
  }
  if (consumed > C_EVENTS) {
    fail("input_consumed");
    consumed = C_EVENTS;
  }
  consumed += run_step(x.m, 2u, 16000u, &ev[consumed], C_EVENTS - consumed, IVS_STOP_CPU_HALTED, print);
  if (print) {
    observe(x.m, "step.2");
    printf("input events=%u consumed=%u\n", (unsigned)C_EVENTS, (unsigned)consumed);
  }
  if (consumed != C_EVENTS) {
    fail("input consumed total");
  }
  if (!digest_all(x.m, d)) {
    fail("digest");
  }
  if (print) {
    stic_state_lines(x.m);
  }
  for (size_t i = 0u; i < C_DOMAINS; i++) {
    observed[i] = 1;
  }
  observed[5] = delivered_video_bytes != 0u;
  observed[6] = delivered_audio_bytes != 0u;
  observed[8] = delivered_psglog_bytes != 0u;
  instance_destroy(&x);
}

/* The seven controller events of data/scenarios/diag-ctrl-psg-pass.ivss (tick, controller 0 word,
 * controller 1 word). */
static void ctrl_events(ivs_input_event *ev)
{
  memset(ev, 0, C_CTRL_EVENTS * sizeof ev[0]);
  ev[0].at_tick = 4000u;
  ev[0].controller[0] = IVS_CTRL_KEY_1;
  ev[1].at_tick = 4800u;
  ev[1].controller[1] = IVS_CTRL_DISC_E;
  ev[2].at_tick = 5600u;
  ev[2].controller[0] = IVS_CTRL_KEY_1 | IVS_CTRL_KEY_9;
  ev[2].controller[1] = IVS_CTRL_SIDE_LOWER_LEFT;
  ev[3].at_tick = 6400u;
  ev[4].at_tick = 7200u;
  ev[4].controller[0] = IVS_CTRL_DISC_N;
  ev[5].at_tick = 8000u;
  ev[5].controller[0] = IVS_CTRL_DISC_N | IVS_CTRL_SIDE_TOP;
  ev[6].at_tick = 8800u;
  ev[6].controller[0] = IVS_CTRL_KEY_5 | IVS_CTRL_SIDE_TOP;
}

/* The replay of diag-ctrl-psg-pass on a fresh instance (plan 05-11): boot stub, zero GROM, the second
 * diagnostic, power-on reset, then run-to 4100 ticks (the first event is consumed), the peeked $01FE,
 * and run-to 24000 ticks stopping at the halt with the six remaining events. The digests go into d and
 * the observed set into observed (video only when bytes of it were delivered; audio and the write log
 * are delivered through buffers sized from ivs_info). print selects the pinned output. */
static void ctrl_session(int print, c_digests *d, int *observed)
{
  c_instance x;
  ivs_load_report lr;
  ivs_status st;
  ivs_input_event ev[C_CTRL_EVENTS];
  uint16_t w[24];
  uint16_t pin = 0u;
  uint32_t consumed = 0u;
  static const uint32_t kinds[3] = {IVS_KIND_EXEC, IVS_KIND_GROM, IVS_KIND_CARTRIDGE};
  const uint8_t *bytes[3];
  size_t lens[3];
  bytes[0] = images.exec;
  bytes[1] = images.grom;
  bytes[2] = images.cart2;
  lens[0] = images.exec_len;
  lens[1] = C_GROM_BYTES;
  lens[2] = images.cart2_len;
  memset(d, 0, sizeof *d);
  delivered_video_bytes = 0u;
  delivered_audio_bytes = 0u;
  delivered_psglog_bytes = 0u;
  run_prefix = "ctrl.";
  ctrl_events(ev);
  if (!instance_create(&x)) {
    fail("ctrl create");
    run_prefix = "";
    return;
  }
  for (size_t i = 0u; i < 3u; i++) {
    st = load(x.m, kinds[i], bytes[i], lens[i], &lr);
    if (print && i == 2u) {
      printf("ctrl.load.cart status=%s length_bytes=%u sha256=", status_token(st), (unsigned)lr.image.length_bytes);
      print_hex(lr.image.sha256, sizeof lr.image.sha256);
      printf("\n");
    }
    if (st != IVS_OK) {
      fail("ctrl load");
      instance_destroy(&x);
      run_prefix = "";
      return;
    }
  }
  st = ivs_machine_reset(x.m, IVS_RESET_POWER_ON);
  if (st != IVS_OK) {
    fail("ctrl reset");
    instance_destroy(&x);
    run_prefix = "";
    return;
  }
  consumed += run_step(x.m, 1u, 4100u, &ev[0], C_CTRL_EVENTS, IVS_STOP_TARGET, print);
  if (consumed != 1u) {
    fail("ctrl input_consumed");
    consumed = 1u;
  }
  st = ivs_machine_peek(x.m, 0x01FEu, 1u, &pin);
  if (st != IVS_OK) {
    fail("ctrl peek");
  }
  if (print) {
    printf("ctrl.peek 01FE=0x%04X\n", (unsigned)pin);
  }
  consumed += run_step(x.m, 2u, 24000u, &ev[consumed], C_CTRL_EVENTS - consumed, IVS_STOP_CPU_HALTED, print);
  if (print) {
    printf("ctrl.input events=%u consumed=%u\n", (unsigned)C_CTRL_EVENTS, (unsigned)consumed);
  }
  if (consumed != C_CTRL_EVENTS) {
    fail("ctrl input consumed total");
  }
  st = ivs_machine_peek(x.m, 0x020Fu, 1u, w);
  if (st != IVS_OK) {
    fail("ctrl peek marker");
  } else if (print) {
    printf("ctrl.mem 020F=%04X\n", (unsigned)w[0]);
  }
  st = ivs_machine_peek(x.m, 0x0210u, 24u, w);
  if (st != IVS_OK) {
    fail("ctrl peek tables");
  } else if (print) {
    printf("ctrl.mem 0210..0227=");
    for (size_t i = 0u; i < 24u; i++) {
      printf("%s%04X", i == 0u ? "" : ",", (unsigned)w[i]);
    }
    printf("\n");
  }
  if (!digest_all(x.m, d)) {
    fail("ctrl digest");
  }
  for (size_t i = 0u; i < C_DOMAINS; i++) {
    observed[i] = 1;
  }
  observed[5] = delivered_video_bytes != 0u;
  observed[6] = delivered_audio_bytes != 0u;
  observed[8] = delivered_psglog_bytes != 0u;
  instance_destroy(&x);
  run_prefix = "";
}

/* ---- the third replay (plan 06-13): data/scenarios/diag-stic-fgbg-pass.ivss ---- */

/* Hand-derived pixels of F-D1 (the Foreground/Background frame of the third diagnostic): x, y, color number. They are the pixel asserts of
 * data/golden/diag-stic-fgbg-pass.expect, read from the card rules and the cell lines of the derivation. */
typedef struct c_pixel {
  uint32_t x;
  uint32_t y;
  uint32_t v;
} c_pixel;
static const c_pixel c_stic_pixels[] = {{0u, 0u, 0u},   {158u, 0u, 0u}, {24u, 64u, 1u}, {24u, 65u, 5u},
                                        {33u, 65u, 3u}, {41u, 65u, 3u}, {49u, 64u, 12u}, {0u, 88u, 5u},
                                        {1u, 88u, 6u},  {152u, 88u, 7u}, {158u, 95u, 7u}, {153u, 89u, 10u}};
#define C_STIC_N_PIXELS (sizeof c_stic_pixels / sizeof c_stic_pixels[0])
/* The box pattern of card 1 (derivation, section (a)): a set bit is foreground, bit 7 the leftmost position. Cell (8,4) shows it with
 * foreground 2 and background 3 from x 32, y 64 (eight positions wide); cell (11,19) with foreground 7 and background 10 from x 152, y 88,
 * clipped to seven positions by the plane width. */
static const uint8_t c_stic_box[8] = {0xFFu, 0x81u, 0x81u, 0x81u, 0x81u, 0x81u, 0x81u, 0xFFu};
/* The guest words at the stop (derivation, section (d)): address, value. */
typedef struct c_word {
  uint16_t addr;
  uint16_t value;
} c_word;
static const c_word c_stic_words[] = {{0x02F0u, 0x03C5u}, {0x02F1u, 0x00FFu}, {0x02A3u, 0x2A01u}, {0x02A4u, 0x0E0Au}, {0x02A5u, 0x1A13u},
                                      {0x02A6u, 0x381Cu}, {0x02DCu, 0x2C1Du}, {0x02EFu, 0x1C0Fu}, {0x0200u, 0x0000u}};
#define C_STIC_N_WORDS (sizeof c_stic_words / sizeof c_stic_words[0])

static uint32_t be32(const uint8_t *b)
{
  return ((uint32_t)b[0] << 24) | ((uint32_t)b[1] << 16) | ((uint32_t)b[2] << 8) | (uint32_t)b[3];
}

static uint64_t be64(const uint8_t *b)
{
  return ((uint64_t)be32(b) << 32) | (uint64_t)be32(b + 4);
}

/* A powered instance of the third replay: boot stub, zero GROM, the third diagnostic, power-on reset. 1 on success. */
static int stic_powered(c_instance *x, int print)
{
  ivs_load_report lr;
  ivs_status st;
  static const uint32_t kinds[3] = {IVS_KIND_EXEC, IVS_KIND_GROM, IVS_KIND_CARTRIDGE};
  const uint8_t *bytes[3];
  size_t lens[3];
  bytes[0] = images.exec;
  bytes[1] = images.grom;
  bytes[2] = images.cart3;
  lens[0] = images.exec_len;
  lens[1] = C_GROM_BYTES;
  lens[2] = images.cart3_len;
  if (!instance_create(x)) {
    fail("stic create");
    return 0;
  }
  for (size_t i = 0u; i < 3u; i++) {
    st = load(x->m, kinds[i], bytes[i], lens[i], &lr);
    if (print && i == 2u) {
      printf("stic.load.cart status=%s length_bytes=%u sha256=", status_token(st), (unsigned)lr.image.length_bytes);
      print_hex(lr.image.sha256, sizeof lr.image.sha256);
      printf("\n");
    }
    if (st != IVS_OK) {
      fail("stic load");
      instance_destroy(x);
      return 0;
    }
  }
  st = ivs_machine_reset(x->m, IVS_RESET_POWER_ON);
  if (st != IVS_OK) {
    fail("stic reset");
    instance_destroy(x);
    return 0;
  }
  return 1;
}

/* Positions of the plane of the slot at slot0 (width x height from ivs_info) that differ from a hand-derived 8 x 8 box block at (x0, y0) of
 * w positions (w at most 8): foreground fg where the pattern bit is set, background bg elsewhere. */
static uint32_t stic_rect_mismatches(const uint8_t *slot0, const ivs_info *info, uint32_t x0, uint32_t y0, uint32_t w, uint32_t fg,
                                     uint32_t bg)
{
  uint32_t bad = 0u;
  for (uint32_t y = 0u; y < 8u; y++) {
    for (uint32_t x = 0u; x < w; x++) {
      uint32_t want = ((c_stic_box[y] >> (7u - x)) & 1u) != 0u ? fg : bg;
      uint32_t px = x0 + x;
      uint32_t py = y0 + y;
      if (px >= info->video_width || py >= info->video_height ||
          slot0[info->video_header_bytes + (size_t)py * info->video_width + px] != want) {
        bad++;
      }
    }
  }
  return bad;
}

/* The checks of the first slot and of the frame stream of the third replay; prints the pinned lines when print is set. */
static void stic_check_frames(const ivs_info *info, int print)
{
  const uint8_t *s0 = kept_video;
  uint32_t px_bad = 0u;
  uint32_t r1;
  uint32_t r2;
  uint64_t slot = info->video_slot_bytes;
  int cont = 1;
  if (slot == 0u || kept_video_n != delivered_frames * slot || delivered_frames == 0u || delivered_frames > C_STIC_FRAMES_MAX) {
    fail("stic video bytes");
    return;
  }
  if (print) {
    printf("stic.video frames=%llu bytes=%llu slot_bytes=%llu header_bytes=%u width=%u height=%u\n",
           (unsigned long long)delivered_frames, (unsigned long long)kept_video_n, (unsigned long long)slot,
           (unsigned)info->video_header_bytes, (unsigned)info->video_width, (unsigned)info->video_height);
  }
  if (be64(s0) != C_STIC_FIRST_TICK || be64(s0 + 8) != 0u || be32(s0 + 16) != info->video_width ||
      be32(s0 + 20) != info->video_height || be32(s0 + 24) != IVS_FRAME_F_DISPLAYED || be32(s0 + 28) != 0u) {
    fail("stic slot header");
  }
  if (print) {
    printf("stic.slot0 index=%llu tick=%llu width=%u height=%u flags=%u reserved=%u\n", (unsigned long long)be64(s0 + 8),
           (unsigned long long)be64(s0), (unsigned)be32(s0 + 16), (unsigned)be32(s0 + 20), (unsigned)be32(s0 + 24),
           (unsigned)be32(s0 + 28));
  }
  for (size_t i = 0u; i < C_STIC_N_PIXELS; i++) {
    const c_pixel *p = &c_stic_pixels[i];
    if (p->x >= info->video_width || p->y >= info->video_height ||
        s0[info->video_header_bytes + (size_t)p->y * info->video_width + p->x] != p->v) {
      px_bad++;
    }
  }
  r1 = stic_rect_mismatches(s0, info, 32u, 64u, 8u, 2u, 3u);
  r2 = stic_rect_mismatches(s0, info, 152u, 88u, 7u, 7u, 10u);
  if (px_bad != 0u || r1 != 0u || r2 != 0u) {
    fail("stic pixels");
  }
  if (print) {
    printf("stic.pixels checked=%zu mismatches=%u\n", (size_t)C_STIC_N_PIXELS, (unsigned)px_bad);
    printf("stic.rect cell=8,4 x=32 y=64 w=8 h=8 fg=2 bg=3 mismatches=%u\n", (unsigned)r1);
    printf("stic.rect cell=11,19 x=152 y=88 w=7 h=8 fg=7 bg=10 mismatches=%u\n", (unsigned)r2);
  }
  for (uint64_t k = 0u; k < delivered_frames; k++) {
    const uint8_t *sk = kept_video + (size_t)(k * slot);
    if (be64(sk + 8) != k || (k > 0u && be64(sk) <= be64(sk - (size_t)slot))) {
      cont = 0;
    }
  }
  if (!cont) {
    fail("stic frame continuity");
  }
  if (print) {
    printf("stic.continuity frames=%llu index=%s ticks=%s\n", (unsigned long long)delivered_frames, cont ? "ok" : "bad",
           cont ? "increasing" : "bad");
  }
}

/* The second pass (D-14): a fresh machine run with IVS_RUN_F_STOP_ON_FRAME, one slot of video capacity per call, until the FRAME stop; the
 * delivered slot must equal the first slot of the first pass, and a second flagged call to the halt must end in the digests of the first
 * pass (want). One call per statement: the report fields are read after the call. */
static void stic_frame_stop_pass(const ivs_info *info, const c_digests *want)
{
  c_instance x;
  ivs_run_request q;
  ivs_run_report rep;
  c_digests got;
  uint8_t *buf;
  uint64_t slot = info->video_slot_bytes;
  ivs_status st;
  int same_frame;
  int same_digests;
  buf = malloc((size_t)slot);
  if (buf == NULL) {
    fail("stic frame-stop buffer");
    return;
  }
  if (!stic_powered(&x, 0)) {
    free(buf);
    return;
  }
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.flags = IVS_RUN_F_STOP_ON_HALT | IVS_RUN_F_STOP_ON_FRAME;
  q.target_ticks = C_STIC_TARGET;
  q.video = buf;
  q.video_capacity = slot;
  memset(&rep, 0, sizeof rep);
  rep.struct_size = (uint32_t)sizeof rep;
  st = ivs_machine_run(x.m, &q, &rep);
  if (st != IVS_OK || rep.stop_reason != IVS_STOP_FRAME || rep.video_frames_completed != 1u || rep.video_bytes_written != slot) {
    fail("stic frame-stop first call");
    printf("stic.frame-stop.1 status=%s stop=%s\n", status_token(st), ivs_run_stop_name(rep.stop_reason));
    free(buf);
    instance_destroy(&x);
    return;
  }
  same_frame = kept_video_n >= slot && memcmp(buf, kept_video, (size_t)slot) == 0;
  printf("stic.frame-stop.1 stop=%s now_ticks=%llu frames=%llu frame0-equal=%s\n", ivs_run_stop_name(rep.stop_reason),
         (unsigned long long)rep.now_ticks, (unsigned long long)rep.video_frames_completed, same_frame ? "yes" : "no");
  if (!same_frame) {
    fail("stic frame-stop frame");
  }
  memset(&rep, 0, sizeof rep);
  rep.struct_size = (uint32_t)sizeof rep;
  st = ivs_machine_run(x.m, &q, &rep);
  if (st != IVS_OK || rep.stop_reason != IVS_STOP_CPU_HALTED || rep.video_frames_completed != 0u) {
    fail("stic frame-stop second call");
  }
  printf("stic.frame-stop.2 stop=%s now_ticks=%llu frames=%llu\n", ivs_run_stop_name(rep.stop_reason),
         (unsigned long long)rep.now_ticks, (unsigned long long)rep.video_frames_completed);
  memset(&got, 0, sizeof got);
  same_digests = digest_all(x.m, &got) && memcmp(&got, want, sizeof got) == 0;
  printf("stic.frame-stop.digests equal=%s\n", same_digests ? "yes" : "no");
  if (!same_digests) {
    fail("stic frame-stop digests");
  }
  free(buf);
  instance_destroy(&x);
}

/* The replay of diag-stic-fgbg-pass on a fresh instance: one run to the halt (no input), the checks above, the guest words, the STIC state
 * line, and (first instance only) the frame-stop pass. The digests go into d and the observed set into observed by the count rule: video and
 * audio by delivered bytes, the write log by delivered bytes, the input domain only when input events were applied (none here). */
static void stic_session(int print, c_digests *d, int *observed)
{
  c_instance x;
  ivs_info info;
  ivs_status st;
  uint32_t consumed;
  uint16_t w[1];
  memset(d, 0, sizeof *d);
  delivered_video_bytes = 0u;
  delivered_audio_bytes = 0u;
  delivered_psglog_bytes = 0u;
  delivered_frames = 0u;
  kept_video_n = 0u;
  keep_video = 1;
  run_prefix = "stic.";
  if (!stic_powered(&x, print)) {
    keep_video = 0;
    run_prefix = "";
    return;
  }
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  st = ivs_machine_info(x.m, &info);
  if (st != IVS_OK || info.video_format != IVS_OUTPUT_FORMAT_INDEX8_FRAMES_V1 || info.video_slot_bytes == 0u) {
    fail("stic info");
    instance_destroy(&x);
    keep_video = 0;
    run_prefix = "";
    return;
  }
  consumed = run_step(x.m, 1u, C_STIC_TARGET, NULL, 0u, IVS_STOP_CPU_HALTED, print);
  if (print) {
    printf("stic.input events=0 consumed=%u\n", (unsigned)consumed);
  }
  stic_check_frames(&info, print);
  for (size_t i = 0u; i < C_STIC_N_WORDS; i++) {
    st = ivs_machine_peek(x.m, c_stic_words[i].addr, 1u, w);
    if (st != IVS_OK || w[0] != c_stic_words[i].value) {
      fail("stic guest word");
    }
    if (print) {
      printf("stic.mem %04X=%04X\n", (unsigned)c_stic_words[i].addr, st == IVS_OK ? (unsigned)w[0] : 0xFFFFu);
    }
  }
  if (print) {
    (void)stic_state_line(x.m, "stic.");
  }
  if (!digest_all(x.m, d)) {
    fail("stic digest");
  }
  for (size_t i = 0u; i < C_DOMAINS; i++) {
    observed[i] = 1;
  }
  observed[5] = delivered_video_bytes != 0u;
  observed[6] = delivered_audio_bytes != 0u;
  observed[7] = consumed != 0u;
  observed[8] = delivered_psglog_bytes != 0u;
  if (print) {
    stic_frame_stop_pass(&info, d);
  }
  instance_destroy(&x);
  keep_video = 0;
  free(kept_video);
  kept_video = NULL;
  kept_video_n = 0u;
  run_prefix = "";
}

static void version_tie(void)
{
  uint32_t lib = ivs_api_version();
  int pkg_eq = IVS_PKG_VERSION_MAJOR == IVS_API_VERSION_MAJOR && IVS_PKG_VERSION_MINOR == IVS_API_VERSION_MINOR &&
               IVS_PKG_VERSION_PATCH == IVS_API_VERSION_PATCH;
  int lib_eq = lib == (uint32_t)IVS_API_VERSION;
  printf("version package=%u.%u.%u header=%u.%u.%u api=0x%06x library=0x%06x tie=%s\n", (unsigned)IVS_PKG_VERSION_MAJOR,
         (unsigned)IVS_PKG_VERSION_MINOR, (unsigned)IVS_PKG_VERSION_PATCH, (unsigned)IVS_API_VERSION_MAJOR,
         (unsigned)IVS_API_VERSION_MINOR, (unsigned)IVS_API_VERSION_PATCH, (unsigned)IVS_API_VERSION, (unsigned)lib,
         pkg_eq && lib_eq ? "yes" : "no");
  if (!pkg_eq || !lib_eq) {
    fail("version tie");
  }
}

/* The frozen v1 sizes as the installed headers declare them (plans 04-09, 04-13, 05-02): the
 * pinned v1 constants (IVS_RUN_REQUEST_V1_SIZE, IVS_RUN_REPORT_V1_SIZE, IVS_INFO_V1_SIZE) are the
 * minimum struct_size a v1-sized caller passes, and the grown sizes are the core's own (v2 request
 * and report v3, info v4). The request's size holds pointers: 64 and 80 with the events at 48 on
 * LP64, a visible skip elsewhere. */
/* Plan 06-03 (D-13, D-14, F-14): the Phase 6 constants, checked at compile time against the installed
 * headers: the fifth stop reason, the frame format id after PSG_LEVELS_V1 and the slot header size. */
_Static_assert(IVS_STOP_FRAME == 5u, "IVS_STOP_FRAME is 5 in the installed header");
_Static_assert(IVS_OUTPUT_FORMAT_INDEX8_FRAMES_V1 == 2u, "IVS_OUTPUT_FORMAT_INDEX8_FRAMES_V1 is 2 in the installed header");
_Static_assert(IVS_FRAME_HEADER_BYTES == 32u, "IVS_FRAME_HEADER_BYTES is 32 in the installed header");

/* Plan 06-07 (D-04, VID-06): the installed header's STIC report, checked at compile time. */
_Static_assert(IVS_STIC_REPORT_V1_SIZE == 208u, "IVS_STIC_REPORT_V1_SIZE is 208 in the installed header");
_Static_assert(sizeof(ivs_stic_report) == IVS_STIC_REPORT_V1_SIZE, "ivs_stic_report is the pinned v1 size");
_Static_assert(offsetof(ivs_stic_report, frame_index) == 32u && offsetof(ivs_stic_report, regs) == 64u,
               "ivs_stic_report offsets in the installed header");

static void v1_sizes(void)
{
  int lp64 = sizeof(void *) == 8u && sizeof(long) == 8u;
  int ok = IVS_RUN_REPORT_V1_SIZE == 112u && IVS_INFO_V1_SIZE == 216u && sizeof(ivs_input_event) == 32u &&
           sizeof(ivs_cpu_state_report) == 36u && offsetof(ivs_info, ticks_per_cpu_cycle) == IVS_INFO_V1_SIZE &&
           offsetof(ivs_run_report, audio_first_index) == IVS_RUN_REPORT_V1_SIZE &&
           offsetof(ivs_run_report, video_first_index) == 136u && sizeof(ivs_run_report) == 160u &&
           sizeof(ivs_info) >= IVS_INFO_V1_SIZE && offsetof(ivs_info, frame_phase_ticks) == 288u &&
           sizeof(ivs_info) == 320u;
  if (lp64) {
    ok = ok && IVS_RUN_REQUEST_V1_SIZE == 64u && offsetof(ivs_run_request, events) == 48u &&
         offsetof(ivs_run_request, psg_log) == IVS_RUN_REQUEST_V1_SIZE && sizeof(ivs_run_request) == 80u;
    printf("v1-sizes run_request_v1=%u events_at=%zu run_request=%zu", (unsigned)IVS_RUN_REQUEST_V1_SIZE,
           offsetof(ivs_run_request, events), sizeof(ivs_run_request));
  } else {
    printf("v1-sizes run_request_v1=skip-non-lp64 events_at=skip-non-lp64 run_request=skip-non-lp64");
  }
  printf(" run_report_v1=%u run_report=%zu info_v1=%u info=%zu input_event=%zu cpu_state_report=%zu ok=%s\n",
         (unsigned)IVS_RUN_REPORT_V1_SIZE, sizeof(ivs_run_report), (unsigned)IVS_INFO_V1_SIZE, sizeof(ivs_info),
         sizeof(ivs_input_event), sizeof(ivs_cpu_state_report), ok ? "yes" : "no");
  if (!ok) {
    fail("v1 sizes");
  }
}

int main(int argc, char **argv)
{
  c_digests first;
  c_digests second;
  int observed[C_DOMAINS];
  int observed2[C_DOMAINS];
  int equal;
  if (argc != 5) {
    fprintf(stderr, "usage: consumer <boot-stub.exec> <diag-cpu-1.ivsc> <diag-ctrl-psg-1.ivsc> <diag-stic-1.ivsc>\n");
    return 2;
  }
  if (!read_image(argv[1], images.exec, C_EXEC_BYTES, &images.exec_len) || images.exec_len == 0u) {
    fprintf(stderr, "consumer: cannot read the EXEC image\n");
    return 2;
  }
  if (!read_image(argv[2], images.cart, C_CART_MAX, &images.cart_len)) {
    fprintf(stderr, "consumer: cannot read the cartridge image\n");
    return 2;
  }
  if (!read_image(argv[3], images.cart2, C_CART_MAX, &images.cart2_len)) {
    fprintf(stderr, "consumer: cannot read the second cartridge image\n");
    return 2;
  }
  if (!read_image(argv[4], images.cart3, C_CART_MAX, &images.cart3_len)) {
    fprintf(stderr, "consumer: cannot read the third cartridge image\n");
    return 2;
  }
  memset(images.grom, 0, sizeof images.grom);
  printf("ivs-consumer 1\n");
  version_tie();
  v1_sizes();
  memset(observed, 0, sizeof observed);
  memset(observed2, 0, sizeof observed2);
  session(1, 1, &first, observed);
  for (size_t i = 0u; i < C_DOMAINS; i++) {
    if (observed[i]) {
      printf("digest.%s ", c_domain_name[i]);
      print_hex(first.sha[i], sizeof first.sha[i]);
      printf("\n");
    }
  }
  session(0, 0, &second, observed2);
  equal = memcmp(&first, &second, sizeof first) == 0 && memcmp(observed, observed2, sizeof observed) == 0;
  printf("second-instance equal=%s\n", equal ? "yes" : "no");
  if (!equal) {
    fail("second instance");
  }
  /* The second replay (plan 05-11): its digests carry the prefix `ctrl.` so the lines of the first replay
   * stay as they were. */
  memset(observed, 0, sizeof observed);
  memset(observed2, 0, sizeof observed2);
  ctrl_session(1, &first, observed);
  for (size_t i = 0u; i < C_DOMAINS; i++) {
    if (observed[i]) {
      printf("ctrl.digest.%s ", c_domain_name[i]);
      print_hex(first.sha[i], sizeof first.sha[i]);
      printf("\n");
    }
  }
  ctrl_session(0, &second, observed2);
  equal = memcmp(&first, &second, sizeof first) == 0 && memcmp(observed, observed2, sizeof observed) == 0;
  printf("ctrl.second-instance equal=%s\n", equal ? "yes" : "no");
  if (!equal) {
    fail("ctrl second instance");
  }
  /* The third replay (plan 06-13): its digests carry the prefix `stic.`; the lines of the two earlier replays stay as they were. */
  memset(observed, 0, sizeof observed);
  memset(observed2, 0, sizeof observed2);
  stic_session(1, &first, observed);
  for (size_t i = 0u; i < C_DOMAINS; i++) {
    if (observed[i]) {
      printf("stic.digest.%s ", c_domain_name[i]);
      print_hex(first.sha[i], sizeof first.sha[i]);
      printf("\n");
    }
  }
  stic_session(0, &second, observed2);
  equal = memcmp(&first, &second, sizeof first) == 0 && memcmp(observed, observed2, sizeof observed) == 0;
  printf("stic.second-instance equal=%s\n", equal ? "yes" : "no");
  if (!equal) {
    fail("stic second instance");
  }
  printf("result %s failures=%d\n", failures == 0 ? "ok" : "fail", failures);
  return failures == 0 ? 0 : 1;
}
