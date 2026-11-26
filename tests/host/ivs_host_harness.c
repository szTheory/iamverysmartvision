/* SPDX-License-Identifier: MIT */
/* The fake frontend (plan 15-01, TOOL-08, D-48): dlopen the build-only shared object, resolve the nine
 * names data/expected/host-symbols.ids pins, drive the project's own lamp-tone cartridge with the pass
 * scenario's six events, and print the nine digest lines the runner's golden carries.
 * It is NOT linked against ivs_core, and that is the point: everything it knows about the machine comes
 * through the object's entries, which is what makes it evidence of the seam rather than of the runner.
 * The one thing it compiles in is runner/frame_export.c, for ivs_frameppm_palette alone, used only to
 * check that every delivered pixel is exactly one palette entry.
 * It recomputes no digest: the nine lines are what ivs_retro_digest returned.
 * Usage: ivs_host_harness <shim-path> <content-dir> [--empty-poll | --schedule-full | --schedule-long |
 *        --refused-load]
 * Output: one `key value` line per observation on stdout; on a failed assertion one `fail <key> <detail>`
 * line and exit 1, so the suite can quote the reason. Exit 0 only when every assertion held.
 * The four alternate modes are the review's two measured defects, each made observable from the object's
 * own answers: --schedule-full and --schedule-long are CR-01 (a host that outruns the shim's bounded
 * input schedule must be refused, and one that outruns it only over a long run must not lose input) and
 * --refused-load is WR-01 (a refused load must leave nothing committed, so a retry is meaningful). In
 * those modes the frontend's own verdicts are the observations, so the suite asserts them. */
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "frame_export.h"
#include "libretro_shim.h"

/* The core's exact system-image sizes (ivs/ivs_machine.h): EXEC 8192 bytes, GROM 2048 bytes, and the
 * largest cartridge container the profile accepts. */
#define HARNESS_EXEC_BYTES 8192u
#define HARNESS_GROM_BYTES 2048u
#define HARNESS_CART_MAX_BYTES 131256u
/* The pass scenario's shape (data/scenarios/demo-lamp-tone-pass.ivss): the six events' absolute ticks
 * and the four run-to bounds, the last of which is 806568. */
#define HARNESS_EVENT_COUNT 6u
#define HARNESS_FRAME_CALLS 13u
#define HARNESS_LAST_TARGET 806568u

/* The mode selector: the pass drive, the empty poll, and the review's two measured defects. */
#define HARNESS_MODE_PASS 0u
#define HARNESS_MODE_EMPTY 1u
#define HARNESS_MODE_SCHEDULE_FULL 2u
#define HARNESS_MODE_SCHEDULE_LONG 3u
#define HARNESS_MODE_REFUSED_LOAD 4u

/* The schedule modes' own scripts (review CR-01). Both counts are the HARNESS's, never the shim's cap: a
 * frontend says how much it has to say and the shim decides what it can hold. FULL exceeds the shim's
 * 4096 outstanding events in ONE drain; LONG says 8000 events over two calls (4000 at a time), so it
 * exceeds that bound over its lifetime but never at once - which is what tells a cap on outstanding
 * events from a cap on lifetime ones. The poll reports a batch only once the machine has come within one
 * frame of it, so LONG is a host saying "nothing yet", never a host that drops its own events. */
#define HARNESS_SCHEDULE_FULL 5000u
#define HARNESS_SCHEDULE_BATCH 4000u
#define HARNESS_SCHEDULE_BATCHES 2u
#define HARNESS_SCHEDULE_BASE 1000u

/* What the object must report, pinned from the profile the shim reads through ivs_info
 * (data/profiles/intv-2609-ntsc-family-p1.json). This harness is not linked against the core, so it cannot
 * call ivs_machine_info and does not pretend to: the profile is the committed source both sides read, and the
 * derivation of each constant from that file's own fields is stated here, field for field:
 *   159     = (video.card_columns 20 - 1) x video.card_size_px.w 8 + video.last_card_column_px 7
 *   96      = video.card_rows 12 x video.card_size_px.h 8
 *   59736   = scanlines_per_frame 262 x cpu_cycles_per_scanline 57 x ticks_per_cpu_cycle 4
 *   0       = stic.frame_phase_ticks
 *   3579545 = crystal_hz.num (crystal_hz.den is 1)
 *   48000   = the PCM rate the shim delivers through its converter (tools/pcm/ivs_refpcm.c), not the core's
 *             native level rate; the harness asserts what the shim declares (15-CONTEXT, resolved questions) */
#define HARNESS_BASE_WIDTH 159u
#define HARNESS_BASE_HEIGHT 96u
#define HARNESS_NOMINAL_FRAME_TICKS 59736u
#define HARNESS_FRAME_PHASE_TICKS 0u
#define HARNESS_SAMPLE_RATE 48000u
#define HARNESS_FPS_NUM 3579545u
#define HARNESS_FPS_DEN 59736u
/* The nine digest domains in the golden's order (data/golden/demo-lamp-tone-pass.digests). */
#define HARNESS_DOMAINS 9u
/* Bytes of one XRGB pixel the shim delivers. */
#define HARNESS_PIXEL_BYTES 4u

/* The pass scenario's six events: the full logical state of both controller words from at_tick on, the
 * same words data/scenarios/demo-lamp-tone-pass.ivss declares (c0 the port at 01FE, c1 the port at 01FF). */
static const ivs_retro_input harness_script[HARNESS_EVENT_COUNT] = {
    {100000u, {0x00000001u, 0x00000000u, 0u, 0u}},
    {220000u, {0x00000101u, 0x00000000u, 0u, 0u}},
    {340000u, {0x00000000u, 0x00010000u, 0u, 0u}},
    {460000u, {0x10000000u, 0x00010000u, 0u, 0u}},
    {580000u, {0x00000000u, 0x00000000u, 0u, 0u}},
    {700000u, {0x00000101u, 0x20000000u, 0u, 0u}},
};

static const uint32_t harness_domain[HARNESS_DOMAINS] = {
    IVS_DIGEST_CPU,     IVS_DIGEST_MEM,   IVS_DIGEST_IMAGES, IVS_DIGEST_INSTANCE, IVS_DIGEST_COMPOSITE,
    IVS_DIGEST_VIDEO,   IVS_DIGEST_AUDIO, IVS_DIGEST_INPUT,  IVS_DIGEST_PSGLOG,
};

static const char *const harness_domain_name[HARNESS_DOMAINS] = {
    "cpu", "mem", "images", "instance", "composite", "video", "audio", "input", "psglog",
};

/* The host's counters, its poll cursor and the events the poll actually returned (recorded so every printed
 * word is what the object handed over, never a restatement of the script this harness believes in). */
typedef struct harness_host {
  unsigned frames;
  unsigned geometry_ok;
  uint64_t plane_bytes;
  uint64_t audio_frames;
  uint64_t pixel_probes;
  unsigned expect_width;
  unsigned expect_height;
  unsigned script_at;
  unsigned poll_events;
  ivs_retro_input poll_got[HARNESS_EVENT_COUNT];
  unsigned mode;
  int geometry_bad;
  int pitch_bad;
  int palette_bad;
  int poll_backwards;
} harness_host;

/* The nine entries, resolved by name exactly as data/expected/host-symbols.ids spells them. */
typedef struct harness_api {
  unsigned (*api_version)(void);
  void (*init)(void);
  void (*deinit)(void);
  int (*set_environment)(const ivs_retro_host *host);
  void (*get_system_av_info)(ivs_retro_av_info *out);
  int (*load_game)(const ivs_retro_images *imgs);
  int (*run)(uint64_t target_ticks);
  void (*unload_game)(void);
  int (*digest)(uint32_t domain, uint8_t out32[32]);
} harness_api;

/* One failed assertion: its own line, then exit 1 so the suite can quote the reason. */
static void fail(const char *key, const char *detail)
{
  printf("fail %s %s\n", key, detail != NULL ? detail : "-");
  (void)fflush(stdout);
  exit(1);
}

static void on_video(void *ctx, const uint32_t *xrgb, unsigned w, unsigned h, size_t pitch_bytes)
{
  harness_host *c = (harness_host *)ctx;
  size_t stride;
  unsigned row;
  unsigned col;

  c->frames++;
  c->plane_bytes += (uint64_t)w * (uint64_t)h;
  if (w != c->expect_width || h != c->expect_height) {
    c->geometry_bad = 1;
  }
  if (pitch_bytes % HARNESS_PIXEL_BYTES != 0u) {
    c->pitch_bad = 1;
    return;
  }
  stride = pitch_bytes / HARNESS_PIXEL_BYTES;
  if (stride < (size_t)w) {
    c->pitch_bad = 1;
    return;
  }
  /* The frame satisfied the callback contract: the declared geometry, a pitch of at least one XRGB pixel per
   * position, and (below) an index plane of exactly w * h positions. Counted per delivered frame, so the suite
   * can require the count to equal the frame count rather than trusting a single frame. */
  c->geometry_ok++;
  /* The pixel probe: the 17 palette triples are pinned distinct (tests/runner/runner_exec.c), so a
   * delivered pixel must equal exactly one of them. A pixel that equals none, or two, is a conversion
   * this harness will not accept. */
  for (row = 0u; row < h; row++) {
    for (col = 0u; col < w; col++) {
      uint32_t px = xrgb[(size_t)row * stride + (size_t)col];
      uint32_t matches = 0u;
      uint32_t i;
      for (i = 0u; i < IVS_FRAMEPPM_PALETTE_ENTRIES; i++) {
        uint8_t rgb[3];
        if (ivs_frameppm_palette(i, rgb) != 1) {
          c->palette_bad = 1;
          return;
        }
        if ((((uint32_t)rgb[0] << 16) | ((uint32_t)rgb[1] << 8) | (uint32_t)rgb[2]) == px) {
          matches++;
        }
      }
      if (matches != 1u) {
        c->palette_bad = 1;
        return;
      }
      c->pixel_probes++;
    }
  }
}

static void on_audio(void *ctx, const int16_t *pcm, size_t frames)
{
  harness_host *c = (harness_host *)ctx;
  (void)pcm;
  c->audio_frames += (uint64_t)frames;
}

static int on_poll(void *ctx, uint64_t now_ticks, ivs_retro_input *ev)
{
  harness_host *c = (harness_host *)ctx;
  if (c->mode == HARNESS_MODE_EMPTY) {
    return 0;
  }
  if (c->mode == HARNESS_MODE_SCHEDULE_FULL || c->mode == HARNESS_MODE_SCHEDULE_LONG) {
    /* A generated, strictly increasing script: the words stay zero (a released pad) and only the tick
     * moves, so what the schedule holds is exactly what the poll handed over. In the long mode a batch
     * is reported only once now_ticks is within one frame of it; before that the host has nothing to say
     * yet and answers 0, which is not the same thing as dropping an event. */
    unsigned batch;
    unsigned within;
    uint64_t base;
    uint64_t horizon;
    if (c->mode == HARNESS_MODE_SCHEDULE_FULL) {
      if (c->script_at >= HARNESS_SCHEDULE_FULL) {
        return 0;
      }
      batch = 0u;
      within = c->script_at;
    } else {
      batch = c->script_at / HARNESS_SCHEDULE_BATCH;
      within = c->script_at % HARNESS_SCHEDULE_BATCH;
      if (batch >= HARNESS_SCHEDULE_BATCHES) {
        return 0;
      }
    }
    base = (uint64_t)HARNESS_SCHEDULE_BASE + (uint64_t)batch * (uint64_t)HARNESS_NOMINAL_FRAME_TICKS;
    if (c->mode == HARNESS_MODE_SCHEDULE_LONG) {
      horizon = now_ticks + (uint64_t)HARNESS_NOMINAL_FRAME_TICKS;
      if (base + (uint64_t)within >= horizon) {
        return 0; /* nothing to say yet: the batch is a frame or more away */
      }
    }
    memset(ev, 0, sizeof *ev);
    ev->at_tick = base + (uint64_t)within;
    c->script_at++;
    c->poll_events++;
    return 1;
  }
  if (c->script_at >= HARNESS_EVENT_COUNT) {
    return 0;
  }
  if (harness_script[c->script_at].at_tick < now_ticks) {
    /* The machine has already passed an event the script still holds: the script can no longer be
     * delivered in order, which is a host error and never something to skip silently. */
    c->poll_backwards = 1;
    return 0;
  }
  *ev = harness_script[c->script_at];
  if (c->poll_events < HARNESS_EVENT_COUNT) {
    c->poll_got[c->poll_events] = *ev;
  }
  c->poll_events++;
  c->script_at++;
  return 1;
}

/* Reads one whole file into a fresh buffer. 1 on success. */
static int read_span(const char *dir, const char *name, size_t max_bytes, uint8_t **out, size_t *out_len)
{
  char path[4096];
  FILE *f;
  long size;
  size_t got;
  int n;

  n = snprintf(path, sizeof path, "%s/%s", dir, name);
  if (n < 0 || (size_t)n >= sizeof path) {
    return 0;
  }
  f = fopen(path, "rb");
  if (f == NULL) {
    return 0;
  }
  if (fseek(f, 0, SEEK_END) != 0) {
    (void)fclose(f);
    return 0;
  }
  size = ftell(f);
  if (size < 0 || (unsigned long)size > (unsigned long)max_bytes) {
    (void)fclose(f);
    return 0;
  }
  if (fseek(f, 0, SEEK_SET) != 0) {
    (void)fclose(f);
    return 0;
  }
  *out = (uint8_t *)malloc(size == 0 ? 1u : (size_t)size);
  if (*out == NULL) {
    (void)fclose(f);
    return 0;
  }
  got = fread(*out, 1u, (size_t)size, f);
  (void)fclose(f);
  if (got != (size_t)size) {
    free(*out);
    *out = NULL;
    return 0;
  }
  *out_len = got;
  return 1;
}

/* Resolves one name; a NULL resolve is a named failure, never a silent skip. */
static void *resolve(void *handle, const char *name)
{
  void *p;
  (void)dlerror();
  p = dlsym(handle, name);
  if (p == NULL) {
    fail("dlsym", name);
  }
  return p;
}

int main(int argc, char **argv)
{
  const char *shim_path;
  const char *content_dir;
  void *handle;
  harness_api api;
  harness_host host;
  ivs_retro_host hooks;
  ivs_retro_av_info av;
  ivs_retro_images imgs;
  uint8_t *exec_bytes = NULL;
  uint8_t *grom_bytes = NULL;
  uint8_t *cart_bytes = NULL;
  size_t exec_len = 0u;
  size_t grom_len = 0u;
  size_t cart_len = 0u;
  uint8_t sha[32];
  unsigned k;
  unsigned i;
  unsigned mode = HARNESS_MODE_PASS;

  if (argc == 4 && strcmp(argv[3], "--empty-poll") == 0) {
    mode = HARNESS_MODE_EMPTY;
  } else if (argc == 4 && strcmp(argv[3], "--schedule-full") == 0) {
    mode = HARNESS_MODE_SCHEDULE_FULL;
  } else if (argc == 4 && strcmp(argv[3], "--schedule-long") == 0) {
    mode = HARNESS_MODE_SCHEDULE_LONG;
  } else if (argc == 4 && strcmp(argv[3], "--refused-load") == 0) {
    mode = HARNESS_MODE_REFUSED_LOAD;
  } else if (argc != 3) {
    printf("usage: ivs_host_harness <shim-path> <content-dir> [--empty-poll | --schedule-full | "
           "--schedule-long | --refused-load]\n");
    return 1;
  }
  shim_path = argv[1];
  content_dir = argv[2];

  handle = dlopen(shim_path, RTLD_NOW | RTLD_LOCAL);
  if (handle == NULL) {
    fail("dlopen", dlerror());
  }
  /* The nine names, spelled exactly as data/expected/host-symbols.ids spells them. */
  *(void **)(&api.api_version) = resolve(handle, "retro_api_version");
  *(void **)(&api.init) = resolve(handle, "retro_init");
  *(void **)(&api.deinit) = resolve(handle, "retro_deinit");
  *(void **)(&api.set_environment) = resolve(handle, "retro_set_environment");
  *(void **)(&api.get_system_av_info) = resolve(handle, "retro_get_system_av_info");
  *(void **)(&api.load_game) = resolve(handle, "retro_load_game");
  *(void **)(&api.run) = resolve(handle, "retro_run");
  *(void **)(&api.unload_game) = resolve(handle, "retro_unload_game");
  *(void **)(&api.digest) = resolve(handle, "ivs_retro_digest");

  if (api.api_version() != IVS_RETRO_SHAPE_VERSION) {
    fail("api.version", "not IVS_RETRO_SHAPE_VERSION");
  }
  printf("api.version %u\n", (unsigned)api.api_version());

  api.init();

  /* A host the object will not accept is refused, not ignored: a NULL input_poll must give 0. */
  memset(&hooks, 0, sizeof hooks);
  memset(&host, 0, sizeof host);
  hooks.ctx = &host;
  hooks.video_refresh = on_video;
  hooks.audio_batch = on_audio;
  hooks.input_poll = NULL;
  if (api.set_environment(&hooks) != 0) {
    fail("env.denied", "a host without input_poll was accepted");
  }
  hooks.input_poll = on_poll;
  if (api.set_environment(&hooks) != 1) {
    fail("env.accepted", "a complete host was refused");
  }
  printf("env.denied 1\n");
  printf("env.accepted 1\n");

  /* The metadata a frontend places a frame with, asserted against the profile's own fields (the pinned constants
   * above) and then printed from the struct the object filled - so a shim that declared the core's native level
   * rate, or dropped the two project-shaped tick fields, fails here rather than printing a plausible number. */
  api.get_system_av_info(&av);
  if (av.base_width != HARNESS_BASE_WIDTH || av.base_height != HARNESS_BASE_HEIGHT) {
    fail("av_info.geometry", "not the profile's 159 x 96 published plane");
  }
  if (av.max_width != HARNESS_BASE_WIDTH || av.max_height != HARNESS_BASE_HEIGHT) {
    fail("av_info.max", "a max geometry above the base geometry");
  }
  if (av.frame_phase_ticks != (uint64_t)HARNESS_FRAME_PHASE_TICKS) {
    fail("av_info.frame_phase_ticks", "not stic.frame_phase_ticks");
  }
  if (av.nominal_frame_ticks != (uint64_t)HARNESS_NOMINAL_FRAME_TICKS) {
    fail("av_info.nominal_frame_ticks", "not the profile's frame spacing");
  }
  if ((unsigned long long)(av.sample_rate + 0.5) != (unsigned long long)HARNESS_SAMPLE_RATE) {
    fail("av_info.sample_rate", "not the converter's rate");
  }
  if ((unsigned long long)(av.fps * (double)av.nominal_frame_ticks + 0.5) != (unsigned long long)HARNESS_FPS_NUM) {
    fail("av_info.fps_num", "not crystal_hz.num");
  }
  host.expect_width = av.base_width;
  host.expect_height = av.base_height;
  host.mode = mode;
  printf("av_info.base_width %u\n", av.base_width);
  printf("av_info.base_height %u\n", av.base_height);
  printf("av_info.max_width %u\n", av.max_width);
  printf("av_info.max_height %u\n", av.max_height);
  printf("av_info.frame_phase_ticks %llu\n", (unsigned long long)av.frame_phase_ticks);
  printf("av_info.nominal_frame_ticks %llu\n", (unsigned long long)av.nominal_frame_ticks);
  printf("av_info.fps_num %llu\n", (unsigned long long)(av.fps * (double)av.nominal_frame_ticks + 0.5));
  printf("av_info.fps_den %llu\n", (unsigned long long)av.nominal_frame_ticks);
  printf("av_info.sample_rate %llu\n", (unsigned long long)(av.sample_rate + 0.5));

  /* The three images: the build-tree boot stub and cartridge, and the zero GROM the scenario names
   * (`content grom zero-grom` is made here, exactly as runner/exec.c makes it, never read). */
  if (!read_span(content_dir, "boot-stub-2.exec", HARNESS_EXEC_BYTES, &exec_bytes, &exec_len) ||
      exec_len != HARNESS_EXEC_BYTES) {
    fail("images.exec", "boot-stub-2.exec is missing or not 8192 bytes");
  }
  if (!read_span(content_dir, "lamp-tone.ivsc", HARNESS_CART_MAX_BYTES, &cart_bytes, &cart_len) || cart_len == 0u) {
    fail("images.cart", "lamp-tone.ivsc is missing or empty");
  }
  grom_bytes = (uint8_t *)calloc(HARNESS_GROM_BYTES, 1u);
  if (grom_bytes == NULL) {
    fail("images.grom", "no storage for the zero GROM");
  }
  grom_len = HARNESS_GROM_BYTES;

  memset(&imgs, 0, sizeof imgs);
  imgs.exec = exec_bytes;
  imgs.exec_len = exec_len;
  imgs.grom = grom_bytes;
  imgs.grom_len = grom_len;
  imgs.cart = cart_bytes;
  imgs.cart_len = cart_len;

  if (mode == HARNESS_MODE_REFUSED_LOAD) {
    /* WR-01: a refused load must leave nothing committed. The observations are a witness pair - the
     * digest of the untouched instance is unmoved by the refused attempt (nothing was committed), the
     * run is refused (there is nothing to run), and the SAME call with the right images then succeeds and
     * moves the digest - so the refusal cannot pass as a machine that is simply dead. */
    ivs_retro_images bad = imgs;
    uint8_t before[32];
    uint8_t after[32];
    uint8_t retried[32];
    bad.cart_len = 4u; /* a container header with nothing behind it: the core's own malformed refusal */
    if (api.digest(IVS_DIGEST_COMPOSITE, before) != 1) {
      fail("load.before.digest", "the fresh instance answered no digest");
    }
    printf("load.before.digest 1\n");
    if (api.load_game(&bad) != 0) {
      fail("load.refused", "retro_load_game accepted a four-byte cartridge");
    }
    printf("load.refused 1\n");
    if (api.digest(IVS_DIGEST_COMPOSITE, after) != 1) {
      fail("load.refused.digest", "the instance answered no digest after a refused load");
    }
    printf("load.refused.digest 1\n");
    if (memcmp(before, after, sizeof before) != 0) {
      fail("load.refused.unchanged", "a refused load moved the instance digest: something was committed");
    }
    printf("load.refused.unchanged 1\n");
    if (api.run((uint64_t)HARNESS_NOMINAL_FRAME_TICKS) != 0) {
      fail("load.refused.run", "a run was accepted on a machine whose load was refused");
    }
    printf("load.refused.run 0\n");
    if (api.load_game(&imgs) != 1) {
      fail("load.retry", "the correct images were refused after a refused load");
    }
    printf("load.retry 1\n");
    if (api.digest(IVS_DIGEST_COMPOSITE, retried) != 1) {
      fail("load.retry.digest", "the retried load answered no digest");
    }
    printf("load.retry.digest 1\n");
    if (memcmp(before, retried, sizeof before) == 0) {
      fail("load.retry.moved", "the retried load's digest equals the image-less instance's");
    }
    printf("load.retry.moved 1\n");
    goto done;
  }

  if (api.load_game(&imgs) != 1) {
    fail("load.images", "retro_load_game refused the three spans");
  }
  printf("load.images 1\n");

  if (mode == HARNESS_MODE_SCHEDULE_FULL) {
    /* CR-01, first limb: 5000 events pending in ONE drain, more than the shim's schedule holds. The run
     * must be refused and must deliver nothing. The poll count is printed so the suite can require that
     * the refusal came while the host still had events left, never because the host ran out. */
    int r = api.run((uint64_t)HARNESS_NOMINAL_FRAME_TICKS);
    printf("schedule.mode full\n");
    printf("schedule.polled %u\n", host.poll_events);
    printf("schedule.runs 1\n");
    printf("schedule.runs.ok %d\n", r == 1 ? 1 : 0);
    printf("schedule.frames %u\n", host.frames);
    goto done;
  }
  if (mode == HARNESS_MODE_SCHEDULE_LONG) {
    /* CR-01, second limb: 8000 events over two calls, 4000 at a time, so the lifetime count exceeds the
     * shim's schedule while the outstanding count never does. Every call must succeed and every batch
     * must reach the core: a schedule that never reclaimed its applied prefix would refuse the second
     * batch here. */
    unsigned ok_runs = 0u;
    for (k = 1u; k <= HARNESS_SCHEDULE_BATCHES; k++) {
      if (api.run(av.nominal_frame_ticks * (uint64_t)k) == 1) {
        ok_runs++;
      }
    }
    printf("schedule.mode long\n");
    printf("schedule.polled %u\n", host.poll_events);
    printf("schedule.runs %u\n", HARNESS_SCHEDULE_BATCHES);
    printf("schedule.runs.ok %u\n", ok_runs);
    printf("schedule.frames %u\n", host.frames);
    goto done;
  }

  /* The drive: one call per frame boundary, then the scenario's last bound. A frame-only stop ends at
   * 776568 and moves seven of the nine lines, so the final call to 806568 is what makes them equal. */
  if (mode == HARNESS_MODE_EMPTY) {
    (void)api.run((uint64_t)HARNESS_LAST_TARGET);
  } else {
    for (k = 1u; k <= HARNESS_FRAME_CALLS; k++) {
      (void)api.run(av.nominal_frame_ticks * (uint64_t)k);
    }
    (void)api.run((uint64_t)HARNESS_LAST_TARGET);
  }
  if (host.geometry_bad != 0) {
    fail("video.geometry", "a delivered frame was not base_width x base_height");
  }
  if (host.pitch_bad != 0) {
    fail("video.pitch", "a delivered frame's pitch was below width * 4 bytes");
  }
  if (host.palette_bad != 0) {
    fail("video.palette", "a delivered pixel was not exactly one palette entry");
  }
  if (host.poll_backwards != 0) {
    fail("poll.backwards", "the machine passed an event the script still held");
  }
  /* A positive frame count and a positive PCM frame count: the two non-vacuity limbs of the drive. */
  if (host.frames == 0u) {
    fail("frames.zero", "the run delivered no frame");
  }
  if (host.geometry_ok != host.frames) {
    fail("frame.geometry", "a delivered frame did not satisfy the callback contract");
  }
  if (host.audio_frames == 0u) {
    fail("audio.frames", "the run delivered no PCM frame");
  }
  printf("frames %u\n", host.frames);
  printf("frame.geometry ok %u\n", host.geometry_ok);
  printf("video.plane.bytes %llu\n", (unsigned long long)host.plane_bytes);
  printf("audio.frames %llu\n", (unsigned long long)host.audio_frames);
  printf("audio.bytes %llu\n", (unsigned long long)(host.audio_frames * 2u));
  printf("pixel.probes ok %llu\n", (unsigned long long)host.pixel_probes);

  /* The poll's six events, printed in order as the three values the object returned: a frame-quantized poll (the
   * failure research Pitfall 2 names) moves an at_tick and fails the suite's word comparison. The empty-poll run
   * carries no event by construction, so the assertion is made only on the scripted run. */
  if (mode == HARNESS_MODE_EMPTY) {
    if (host.poll_events != 0u) {
      fail("input.words", "--empty-poll returned an event");
    }
  } else {
    if (host.poll_events != HARNESS_EVENT_COUNT) {
      fail("input.words", "the poll did not return the scenario's six events");
    }
    for (i = 0u; i < HARNESS_EVENT_COUNT; i++) {
      if (host.poll_got[i].at_tick != harness_script[i].at_tick) {
        fail("input.tick", "a returned event's at_tick is not the scenario's");
      }
      printf("input.words %llu %08x %08x\n", (unsigned long long)host.poll_got[i].at_tick,
             (unsigned)host.poll_got[i].controller[0], (unsigned)host.poll_got[i].controller[1]);
    }
  }

  /* The nine lines, in the golden's order, exactly as the object returned them. */
  for (i = 0u; i < HARNESS_DOMAINS; i++) {
    unsigned b;
    if (api.digest(harness_domain[i], sha) != 1) {
      fail("digest.refused", harness_domain_name[i]);
    }
    printf("digest.%s ", harness_domain_name[i]);
    for (b = 0u; b < 32u; b++) {
      printf("%02x", (unsigned)sha[b]);
    }
    printf("\n");
  }

  /* A real teardown, proved by the refusal it leaves behind: a frontend that only pretends to unload
   * cannot pass. */
  api.unload_game();
  if (api.digest(IVS_DIGEST_CPU, sha) != 0) {
    fail("unload.digest", "a digest was answered after retro_unload_game");
  }
  printf("unload.digest.refused 1\n");

done:
  api.deinit();
  free(exec_bytes);
  free(grom_bytes);
  free(cart_bytes);
  (void)dlclose(handle);
  return 0;
}
