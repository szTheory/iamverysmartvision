/* SPDX-License-Identifier: MIT */
/* The host shim (plan 15-01, TOOL-08, D-48, D-35): the nine entries of host/libretro_shim.h over the
 * unchanged static core. The subset is libretro-shaped and never compatible; nothing is vendored.
 * Three rules this file exists to keep:
 *   - no palette of its own: the index -> XRGB conversion goes through ivs_frameppm_palette from
 *     runner/frame_export.c, compiled into this object, so the tree has one palette and one converter;
 *   - no second PCM converter: the delivered level stream is fed to tools/pcm/ivs_refpcm.c;
 *   - no digest recomputed: ivs_retro_digest is one call to ivs_machine_state_digest and nothing else,
 *     because the phase's whole claim rests on there being exactly one producer.
 * It also opens no file (the frontend hands over three byte spans) and keeps no clock, so the object's
 * undefined names are the core's plus the allocator's and nothing else. */
#include "libretro_shim.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "frame_export.h"
#include "ivs/ivs.h"
#include "ivs_refpcm.h"

/* The one registered profile this shim drives (data/profiles/intv-2609-ntsc-family-p1.json). */
#define SHIM_PROFILE "intv-2609-ntsc-family-p1"
/* The rate the reference converter runs at, and the rate av_info declares: the shim converts, so this
 * is the converter's output rate and never the core's native level rate (223721.5625 Hz). */
#define SHIM_SAMPLE_RATE IVS_REFPCM_RATE_48000
/* The largest instance storage this shim will allocate; the profile needs far less. */
#define SHIM_STORAGE_MAX ((size_t)1u << 20)
/* The OUTSTANDING input events the shim holds between calls (the core's own cap is 65536 events). The
 * applied prefix is reclaimed before every drain, so this is not a lifetime count; a poll that fills it
 * is refused rather than truncated (review CR-01). */
#define SHIM_EVENTS_MAX 4096u
/* Bytes of one XRGB pixel this shim delivers. */
#define SHIM_PIXEL_BYTES 4u
/* The largest frame-slot count one call may need; a larger target is refused rather than sized, so a
 * hostile target cannot wrap the capacity arithmetic. */
#define SHIM_SLOTS_MAX ((uint64_t)1u << 20)
/* Bytes of one delivered level sample (level_A, level_B, level_C, 0). */
#define SHIM_LEVEL_BYTES 4u

typedef struct shim_state {
  ivs_machine *machine;
  void *raw_storage;  /* the malloc'd block; storage is an aligned address inside it */
  void *storage;      /* the aligned instance bytes handed to ivs_machine_create */
  size_t storage_bytes;
  ivs_retro_host host;
  int have_host;
  ivs_input_event sched[SHIM_EVENTS_MAX];
  uint32_t sched_n;
  uint32_t consumed;
  uint8_t *video;
  uint64_t video_cap;
  uint8_t *audio;
  uint64_t audio_cap;
  uint8_t *log;
  uint64_t log_cap;
  uint32_t *xrgb;
  size_t xrgb_cap;
  int16_t *pcm;
  size_t pcm_cap;
  ivs_refpcm_state rp;
  int rp_ready;
  ivs_info info;
  int have_info;
} shim_state;

static shim_state shim;

/* Grows *buf to at least need bytes. 1 on success, 0 when the request cannot be represented or the
 * allocator refused; never a truncation. */
static int shim_reserve(uint8_t **buf, uint64_t *cap, uint64_t need)
{
  size_t n;
  uint8_t *p;
  if (need <= *cap) {
    return 1;
  }
  n = (size_t)need;
  if ((uint64_t)n != need) {
    return 0;
  }
  p = (uint8_t *)realloc(*buf, n);
  if (p == NULL) {
    return 0;
  }
  *buf = p;
  *cap = need;
  return 1;
}

/* The same for one frame of XRGB pixels (a different element type, so a separate function). */
static int shim_reserve_pixels(uint32_t **buf, size_t *cap, size_t need)
{
  uint32_t *p;
  if (need <= *cap) {
    return 1;
  }
  if (need > SIZE_MAX / sizeof *p) {
    return 0;
  }
  p = (uint32_t *)realloc(*buf, need * sizeof *p);
  if (p == NULL) {
    return 0;
  }
  *buf = p;
  *cap = need;
  return 1;
}

/* The same for one PCM block. */
static int shim_reserve_pcm(int16_t **buf, size_t *cap, size_t need)
{
  int16_t *p;
  if (need <= *cap) {
    return 1;
  }
  if (need > SIZE_MAX / sizeof *p) {
    return 0;
  }
  p = (int16_t *)realloc(*buf, need * sizeof *p);
  if (p == NULL) {
    return 0;
  }
  *buf = p;
  *cap = need;
  return 1;
}

/* Reads the instance information into shim.info. 1 on success. */
static int shim_read_info(void)
{
  if (shim.machine == NULL) {
    return 0;
  }
  memset(&shim.info, 0, sizeof shim.info);
  shim.info.struct_size = (uint32_t)sizeof shim.info;
  if (ivs_machine_info(shim.machine, &shim.info) != IVS_OK) {
    shim.have_info = 0;
    return 0;
  }
  shim.have_info = 1;
  return 1;
}

/* Destroys the machine and frees the instance storage; the buffers this object owns stay, because a
 * second retro_init reuses them. */
static void shim_teardown_machine(void)
{
  if (shim.machine != NULL) {
    (void)ivs_machine_destroy(shim.machine);
    shim.machine = NULL;
  }
  free(shim.raw_storage);
  shim.raw_storage = NULL;
  shim.storage = NULL;
  shim.storage_bytes = 0u;
  shim.sched_n = 0u;
  shim.consumed = 0u;
  shim.rp_ready = 0;
  shim.have_info = 0;
}

IVS_API unsigned retro_api_version(void)
{
  return IVS_RETRO_SHAPE_VERSION;
}

IVS_API void retro_init(void)
{
  ivs_storage_requirements req;
  ivs_create_options co;
  ivs_status st;
  uintptr_t base;
  uintptr_t aligned;
  size_t total;
  void *raw;

  /* The host is NOT cleared here: the shape lets a frontend install its callbacks before it creates the
   * instance, and a creation that silently dropped them would refuse every run. */
  shim_teardown_machine();

  memset(&req, 0, sizeof req);
  req.struct_size = (uint32_t)sizeof req;
  st = ivs_machine_storage_requirements(SHIM_PROFILE, &req);
  if (st != IVS_OK || req.size_bytes == 0u || req.align_bytes == 0u || req.size_bytes > SHIM_STORAGE_MAX ||
      (req.align_bytes & (req.align_bytes - 1u)) != 0u) {
    return;
  }
  total = (size_t)req.size_bytes + (size_t)req.align_bytes;
  raw = malloc(total);
  if (raw == NULL) {
    return;
  }
  /* Aligned by hand, exactly as runner/exec.c's ex_create does: the core takes no allocator (D-01). */
  base = (uintptr_t)raw;
  aligned = (base + (uintptr_t)(req.align_bytes - 1u)) & ~(uintptr_t)(req.align_bytes - 1u);
  memset(&co, 0, sizeof co);
  co.struct_size = (uint32_t)sizeof co;
  co.profile_id = SHIM_PROFILE;
  st = ivs_machine_create(&co, (void *)aligned, (size_t)req.size_bytes, &shim.machine);
  if (st != IVS_OK) {
    shim.machine = NULL;
    free(raw);
    return;
  }
  shim.raw_storage = raw;
  shim.storage = (void *)aligned;
  shim.storage_bytes = (size_t)req.size_bytes;
  (void)shim_read_info();
}

IVS_API void retro_deinit(void)
{
  shim_teardown_machine();
  shim.have_host = 0;
  memset(&shim.host, 0, sizeof shim.host);
  free(shim.video);
  free(shim.audio);
  free(shim.log);
  free(shim.xrgb);
  free(shim.pcm);
  memset(&shim, 0, sizeof shim);
}

IVS_API int retro_set_environment(const ivs_retro_host *host)
{
  if (host == NULL || host->video_refresh == NULL || host->audio_batch == NULL || host->input_poll == NULL) {
    shim.have_host = 0;
    memset(&shim.host, 0, sizeof shim.host);
    return 0;
  }
  shim.host = *host;
  shim.have_host = 1;
  return 1;
}

IVS_API void retro_get_system_av_info(ivs_retro_av_info *out)
{
  double fps = 0.0;
  if (out == NULL) {
    return;
  }
  memset(out, 0, sizeof *out);
  if (!shim.have_info) {
    return;
  }
  out->base_width = (unsigned)shim.info.video_width;
  out->base_height = (unsigned)shim.info.video_height;
  out->max_width = (unsigned)shim.info.video_width;
  out->max_height = (unsigned)shim.info.video_height;
  if (shim.info.video_height != 0u) {
    out->aspect_ratio = (double)shim.info.video_width / (double)shim.info.video_height;
  }
  if (shim.info.crystal_hz_den != 0u && shim.info.nominal_frame_ticks != 0u) {
    fps = (double)shim.info.crystal_hz_num /
          ((double)shim.info.crystal_hz_den * (double)shim.info.nominal_frame_ticks);
  }
  out->fps = fps;
  out->sample_rate = (double)SHIM_SAMPLE_RATE;
  out->frame_phase_ticks = shim.info.frame_phase_ticks;
  out->nominal_frame_ticks = shim.info.nominal_frame_ticks;
}

IVS_API int retro_load_game(const ivs_retro_images *imgs)
{
  ivs_load_report rep;
  ivs_status st;

  if (shim.machine == NULL || imgs == NULL) {
    return 0;
  }
  memset(&rep, 0, sizeof rep);
  rep.struct_size = (uint32_t)sizeof rep;
  rep.image.struct_size = (uint32_t)sizeof rep.image;
  st = ivs_machine_load(shim.machine, IVS_KIND_EXEC, imgs->exec, imgs->exec_len, NULL, &rep);
  if (st != IVS_OK) {
    return 0;
  }
  st = ivs_machine_load(shim.machine, IVS_KIND_GROM, imgs->grom, imgs->grom_len, NULL, &rep);
  if (st != IVS_OK) {
    return 0;
  }
  st = ivs_machine_load(shim.machine, IVS_KIND_CARTRIDGE, imgs->cart, imgs->cart_len, NULL, &rep);
  if (st != IVS_OK) {
    return 0;
  }
  st = ivs_machine_reset(shim.machine, IVS_RESET_POWER_ON);
  if (st != IVS_OK) {
    return 0;
  }
  if (!shim_read_info()) {
    return 0;
  }
  shim.sched_n = 0u;
  shim.consumed = 0u;
  shim.rp_ready = 0;
  if (ivs_refpcm_init(&shim.rp, shim.info.crystal_hz_num, shim.info.crystal_hz_den, shim.info.audio_sample_ticks,
                      SHIM_SAMPLE_RATE) != IVS_REFPCM_OK) {
    return 0;
  }
  shim.rp_ready = 1;
  return 1;
}

/* Appends what the host's poll has produced to the schedule. 1 on success; 0 when the poll went
 * backwards (a host error: the core's log is a chain over the events in order, so a reordered script
 * would silently produce a different digest) or when the host has more outstanding events than the
 * schedule holds (review CR-01: refused, never truncated - a full schedule cannot be told from a
 * truncated one without consuming an event the shim could not store, so filling it IS the refusal).
 * The applied prefix is reclaimed first, so the cap is on outstanding events and a long run cannot lose
 * input. */
static int shim_drain_poll(uint64_t now_ticks)
{
  if (shim.consumed > 0u) {
    if (shim.consumed < shim.sched_n) {
      memmove(&shim.sched[0], &shim.sched[shim.consumed],
              (size_t)(shim.sched_n - shim.consumed) * sizeof shim.sched[0]);
      shim.sched_n -= shim.consumed;
    } else {
      shim.sched_n = 0u;
    }
    shim.consumed = 0u;
  }
  while (shim.sched_n < SHIM_EVENTS_MAX) {
    ivs_retro_input ev;
    memset(&ev, 0, sizeof ev);
    if (shim.host.input_poll(shim.host.ctx, now_ticks, &ev) != 1) {
      return 1;                       /* script exhausted */
    }
    if (shim.sched_n > 0u && ev.at_tick < shim.sched[shim.sched_n - 1u].at_tick) {
      return 0;
    }
    memset(&shim.sched[shim.sched_n], 0, sizeof shim.sched[0]);
    shim.sched[shim.sched_n].at_tick = ev.at_tick;
    memcpy(shim.sched[shim.sched_n].controller, ev.controller, sizeof ev.controller);
    shim.sched_n++;
  }
  return 0;                           /* still full: the host has more to say than the shim can hold */
}

/* Delivers the whole slots this call completed: one video_refresh per frame at the declared pitch. 1 on
 * success, 0 on a slot that is not a palette index (a core change, never something to render anyway). */
static int shim_deliver_video(uint64_t bytes)
{
  uint64_t frames;
  uint64_t i;
  unsigned w = (unsigned)shim.info.video_width;
  unsigned h = (unsigned)shim.info.video_height;
  size_t npix = (size_t)w * (size_t)h;
  size_t stride;

  if (bytes == 0u) {
    return 1;
  }
  if (shim.info.video_slot_bytes == 0u || w == 0u || h == 0u) {
    return 0;
  }
  frames = bytes / shim.info.video_slot_bytes;
  if (frames == 0u) {
    return 1;
  }
  if (!shim_reserve_pixels(&shim.xrgb, &shim.xrgb_cap, npix)) {
    return 0;
  }
  stride = (size_t)w;
  for (i = 0u; i < frames; i++) {
    const uint8_t *slot = shim.video + (size_t)(i * shim.info.video_slot_bytes);
    const uint8_t *plane = slot + IVS_FRAME_HEADER_BYTES;
    size_t k;
    for (k = 0u; k < npix; k++) {
      uint8_t rgb[3];
      uint32_t entry = plane[k] == IVS_PIXEL_BLANK ? IVS_FRAMEPPM_PALETTE_ENTRIES - 1u : (uint32_t)plane[k];
      if (ivs_frameppm_palette(entry, rgb) != 1) {
        return 0;
      }
      shim.xrgb[k] = ((uint32_t)rgb[0] << 16) | ((uint32_t)rgb[1] << 8) | (uint32_t)rgb[2];
    }
    shim.host.video_refresh(shim.host.ctx, shim.xrgb, w, h, stride * SHIM_PIXEL_BYTES);
  }
  return 1;
}

/* Delivers the level samples this call wrote through the one reference converter. 1 on success. */
static int shim_deliver_audio(uint64_t bytes)
{
  size_t nsamples;
  size_t bound;
  size_t produced = 0u;

  if (bytes == 0u || !shim.rp_ready) {
    return 1;
  }
  nsamples = (size_t)(bytes / SHIM_LEVEL_BYTES);
  if (nsamples == 0u) {
    return 1;
  }
  bound = ivs_refpcm_out_bound(&shim.rp, nsamples);
  if (bound == SIZE_MAX || !shim_reserve_pcm(&shim.pcm, &shim.pcm_cap, bound)) {
    return 0;
  }
  if (ivs_refpcm_feed(&shim.rp, shim.audio, nsamples, shim.pcm, bound, &produced) != IVS_REFPCM_OK) {
    return 0;
  }
  if (produced != 0u) {
    shim.host.audio_batch(shim.host.ctx, shim.pcm, produced);
  }
  return 1;
}

IVS_API int retro_run(uint64_t target_ticks)
{
  ivs_run_request q;
  ivs_run_report rep;
  ivs_status st;
  uint64_t now;
  uint64_t delta;
  uint64_t slots;
  uint64_t video_need;
  uint64_t audio_need;
  uint64_t log_need;

  if (shim.machine == NULL || !shim.have_host) {
    return 0;
  }
  if (!shim_read_info() || shim.info.nominal_frame_ticks == 0u) {
    return 0;
  }
  now = shim.info.time_ticks;
  if (!shim_drain_poll(now)) {
    return 0;
  }
  /* The documented capacity formulas (ivs/ivs_run.h), the same arithmetic runner/exec.c's
   * ex_plan_buffers does: floor(D / F) + 1 slots for audio and the write log, and that plus the
   * profile's slack for video. */
  delta = target_ticks > now ? target_ticks - now : 0u;
  slots = delta / shim.info.nominal_frame_ticks + 1u;
  if (slots > SHIM_SLOTS_MAX) {
    return 0;
  }
  video_need = (uint64_t)shim.info.video_bytes_per_frame * (slots + (uint64_t)shim.info.video_slack_frames);
  audio_need = (uint64_t)shim.info.audio_bytes_per_frame * slots;
  log_need = (uint64_t)shim.info.psg_log_record_bytes * (uint64_t)shim.info.psg_log_records_per_frame * slots;
  if (!shim_reserve(&shim.video, &shim.video_cap, video_need) || !shim_reserve(&shim.audio, &shim.audio_cap, audio_need) ||
      !shim_reserve(&shim.log, &shim.log_cap, log_need)) {
    return 0;
  }
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.flags = 0u; /* no IVS_RUN_F_STOP_ON_FRAME: the target tick is the authority */
  q.target_ticks = target_ticks;
  q.video = video_need != 0u ? shim.video : NULL;
  q.video_capacity = video_need;
  q.audio = audio_need != 0u ? shim.audio : NULL;
  q.audio_capacity = audio_need;
  q.psg_log = log_need != 0u ? shim.log : NULL;
  q.psg_log_capacity = log_need;
  /* The unconsumed suffix, exactly as the runner resubmits it: the whole array again is refused
   * IVS_REASON_INPUT_BEFORE_FLOOR once one event has been applied. */
  q.events = shim.consumed < shim.sched_n ? &shim.sched[shim.consumed] : NULL;
  q.event_count = shim.sched_n - shim.consumed;
  memset(&rep, 0, sizeof rep);
  rep.struct_size = (uint32_t)sizeof rep;
  st = ivs_machine_run(shim.machine, &q, &rep);
  if (st != IVS_OK || rep.stop_reason == IVS_STOP_INTERNAL) {
    return 0;
  }
  shim.consumed += rep.input_consumed;
  if (!shim_deliver_video(rep.video_bytes_written) || !shim_deliver_audio(rep.audio_bytes_written)) {
    return 0;
  }
  return rep.video_bytes_written != 0u ? 1 : 0;
}

IVS_API void retro_unload_game(void)
{
  shim_teardown_machine();
}

IVS_API int ivs_retro_digest(uint32_t domain, uint8_t out32[32])
{
  if (shim.machine == NULL || out32 == NULL) {
    return 0;
  }
  if (ivs_machine_state_digest(shim.machine, domain, out32) != IVS_OK) {
    return 0;
  }
  return 1;
}
