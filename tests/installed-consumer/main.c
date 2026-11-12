/* SPDX-License-Identifier: MIT */
/* The independent consumer of the installed ivs package (PKG-01, D-34, D-39, plan 04-23).
 * Built only through find_package(ivs) against the installed, relocated prefix (gate step
 * pkg-install). Usage: consumer <boot-stub.exec> <diag-cpu-1.ivsc>; both images come from the
 * build tree's tools/pack output and the 2048-byte zero GROM is made here, so the program embeds
 * no content bytes. It replays the runner scenario data/scenarios/diag-input-pass.ivss through
 * the public API alone:
 *   version tie   ivs_VERSION (from the package config), IVS_API_VERSION, ivs_api_version();
 *   v1 sizes      the frozen v1 struct sizes as the installed headers declare them;
 *   create        storage sized by ivs_machine_storage_requirements, aligned by hand;
 *   refusals      a truncated EXEC (MALFORMED) and a duplicate cartridge load (DUPLICATE_LOAD),
 *                 every state digest equal before and after each;
 *   load, reset   boot stub, zero GROM, diagnostic cartridge, power-on reset;
 *   bounded run   run-to 1000 ticks, then run-to 16000 ticks stopping at the halt, with the
 *                 scenario's two input events (tick 400 controller 0 KEY_1, tick 1200 none),
 *                 the unconsumed suffix resubmitted on the second call;
 *   digests       one digest.<domain> line per observed domain (video and audio only when the
 *                 profile's output layout is not NONE), the form of the runner golden;
 *   second instance  a fresh instance without the refusals must give the same digests.
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
#define C_DOMAINS 8u
#define C_EVENTS 2u

typedef struct c_images {
  uint8_t exec[C_EXEC_BYTES + 1u];
  size_t exec_len;
  uint8_t grom[C_GROM_BYTES];
  uint8_t cart[C_CART_MAX + 1u];
  size_t cart_len;
} c_images;

typedef struct c_digests {
  uint8_t sha[C_DOMAINS][32];
} c_digests;

static c_images images;
static int failures;

/* Digest domains in the order of the runner report; observed[i] says whether a line is printed. */
static const uint32_t c_domain[C_DOMAINS] = {IVS_DIGEST_CPU,       IVS_DIGEST_MEM,   IVS_DIGEST_IMAGES, IVS_DIGEST_INSTANCE,
                                             IVS_DIGEST_COMPOSITE, IVS_DIGEST_VIDEO, IVS_DIGEST_AUDIO,  IVS_DIGEST_INPUT};
static const char *const c_domain_name[C_DOMAINS] = {"cpu",       "mem",   "images", "instance",
                                                     "composite", "video", "audio",  "input"};

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

/* Bytes of video and audio the session's calls delivered (reports): the consumer supplies no output
 * buffer (NULL with capacity 0, "not requested"), so both stay 0. A domain is observed when its
 * delivered bytes are nonzero, the count rule of the runner golden (plan 05-06, B1): the layout in
 * ivs_info says what a call could deliver, not what this session observed. */
static uint64_t delivered_video_bytes;
static uint64_t delivered_audio_bytes;

/* One run call of the replay; returns the report's input_consumed (0 on a refusal). */
static uint32_t run_step(ivs_machine *m, unsigned step, uint64_t target, const ivs_input_event *events, uint32_t count,
                         uint32_t want_stop, int print)
{
  ivs_run_request q;
  ivs_run_report rep;
  ivs_status st;
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.flags = IVS_RUN_F_STOP_ON_HALT;
  q.target_ticks = target;
  q.events = count > 0u ? events : NULL;
  q.event_count = count;
  memset(&rep, 0, sizeof rep);
  rep.struct_size = (uint32_t)sizeof rep;
  st = ivs_machine_run(m, &q, &rep);
  if (st != IVS_OK) {
    if (print) {
      printf("step.%u status=%s reason=%u\n", step, status_token(st), (unsigned)rep.refusal_reason);
    }
    fail("run refused");
    return 0u;
  }
  if (print) {
    printf("step.%u target_ticks=%llu stop=%s now_ticks=%llu overshoot_ticks=%llu input_consumed=%u "
           "controller0=%08x\n",
           step, (unsigned long long)target, ivs_run_stop_name(rep.stop_reason), (unsigned long long)rep.now_ticks,
           (unsigned long long)rep.overshoot_ticks, (unsigned)rep.input_consumed, (unsigned)rep.controller_now[0]);
  }
  if (rep.stop_reason != want_stop) {
    fail("stop reason");
  }
  delivered_video_bytes += rep.video_bytes_written;
  delivered_audio_bytes += rep.audio_bytes_written;
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
  for (size_t i = 0u; i < C_DOMAINS; i++) {
    observed[i] = 1;
  }
  observed[5] = delivered_video_bytes != 0u;
  observed[6] = delivered_audio_bytes != 0u;
  instance_destroy(&x);
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
 * and report, current info). The request's size holds pointers: 64 and 80 with the events at 48 on
 * LP64, a visible skip elsewhere. */
static void v1_sizes(void)
{
  int lp64 = sizeof(void *) == 8u && sizeof(long) == 8u;
  int ok = IVS_RUN_REPORT_V1_SIZE == 112u && IVS_INFO_V1_SIZE == 216u && sizeof(ivs_input_event) == 32u &&
           sizeof(ivs_cpu_state_report) == 36u && offsetof(ivs_info, ticks_per_cpu_cycle) == IVS_INFO_V1_SIZE &&
           offsetof(ivs_run_report, audio_first_index) == IVS_RUN_REPORT_V1_SIZE && sizeof(ivs_run_report) == 136u &&
           sizeof(ivs_info) >= IVS_INFO_V1_SIZE;
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
  if (argc != 3) {
    fprintf(stderr, "usage: consumer <boot-stub.exec> <diag-cpu-1.ivsc>\n");
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
  printf("result %s failures=%d\n", failures == 0 ? "ok" : "fail", failures);
  return failures == 0 ? 0 : 1;
}
