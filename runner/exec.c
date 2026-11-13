/* SPDX-License-Identifier: MIT */
/* Runner scenario executor (D-08, D-09, D-17, D-20, D-22, D-30, plan 04-18). One scenario on one
 * fresh instance the executor creates in the host's storage buffer, aligned by hand; the public
 * API only (ivs/ivs.h); no globals and no allocation (the host owns every buffer). Each run-to is
 * executed as slices of ivs_machine_run calls (plan 04-19, D-21: at most slice_ticks each, the
 * unconsumed input suffix resubmitted each call, cancellation and the wall cap checked before
 * every slice, the max_ticks ceiling a deterministic budget), always with IVS_RUN_F_STOP_ON_HALT
 * so a halt is a stop the scenario must expect. The outcome is the
 * pure precedence of ivs_runner_outcome_resolve over what the run observed (outcome.c), except a
 * load refusal that matches expect-load, which the executor decides as pass before that call.
 * C rule (hosted-only hazard): one call per statement when one argument reads state another
 * argument's call changes. Named-fault sites (data/expected/faults/runner.ids) sit in this TU. */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_runner.h"
#include "ivs_refpcm.h"
#include "frame_export.h"
#include "runner_chain.h"
#include "runner_vchain.h"

/* Observation slots for the first detail of each kind (index = bit number of IVS_RUNNER_OBS_*). */
#define EX_OBS_BITS 6u
#define EX_READ_BUF (IVS_RUNNER_CAP_CART + 1u)
#define EX_WAV_FRAMES 2048u /* frames of the converter chunk buffer (an out bound of at most this many per feed) */
#define EX_WAV_CHUNK 1024u  /* input samples fed per converter call, halved until its out bound fits */

typedef struct ex_state {
  const ivs_scenario *scn;
  const ivs_runner_host *host;
  ivs_runner_record *rec;
  ivs_machine *m;
  uint8_t *readbuf; /* EX_READ_BUF bytes of host storage after the instance */
  uint64_t slice_ticks;
  uint64_t max_ticks;
  uint64_t now;        /* machine time after the last run call (ivs_info time_ticks before any) */
  uint64_t wall_start; /* now_ms at the start of the first step */
  uint32_t wall_cap_ms;
  uint32_t observed;
  uint8_t *vid_buf; /* the three output channels cut from host->buffers (ex_plan_buffers); NULL: not requested */
  size_t vid_cap;
  uint8_t *aud_buf;
  size_t aud_cap;
  uint8_t *log_buf;
  size_t log_cap;
  ivs_runner_achain chain; /* the runner's own recomputation of the AUDIO chain (D-18) */
  uint32_t log_record_bytes; /* ivs_info psg_log_record_bytes: bytes of one write-log record */
  uint8_t *cap_log;          /* capture storage after the read buffer: the first records (16 bytes each) */
  uint8_t *cap_aud;          /* ... and the first audio level-change entries (12 bytes each) */
  uint32_t cap_log_n;
  uint32_t cap_aud_n;
  uint64_t out_max;         /* the operator's byte bound of each raw output file */
  uint64_t out_audio_bytes; /* bytes handed to write_audio_raw */
  uint64_t out_log_bytes;   /* bytes handed to write_psg_log */
  /* Operator WAV (plan 05-10, D-21): the reference converter state, carried across slices, and the chunk
   * buffers; wav_requested when both hooks exist, wav_started once the converter is initialised. */
  ivs_refpcm_state rp;
  int wav_requested;
  int wav_started;
  int wav_failed; /* a wav hook or the byte bound failed: the file is not finished */
  uint32_t wav_rate;
  uint64_t out_wav_bytes; /* PCM bytes handed to write_wav_pcm */
  int16_t wav_pcm[EX_WAV_FRAMES];
  uint8_t wav_bytes[2u * EX_WAV_FRAMES];
  /* Video (plan 06-12, D-16, D-17): the runner's own recomputation of the frame chain and slot checks, the capture
   * store (IVS_SCN_CAPTURE_VIDEO_MAX slots after the audio captures), the byte bound of the operator raw file. */
  ivs_runner_vchain vchain;
  uint8_t *cap_vid;
  uint32_t cap_vid_mask;     /* bit i: window slot i holds a delivered frame */
  uint32_t vid_slot_bytes;   /* ivs_info video_slot_bytes (0: no video layout) */
  uint32_t vid_width;
  uint32_t vid_height;
  uint64_t out_video_max;    /* the operator's byte bound of the raw video file */
  uint64_t out_video_bytes;  /* bytes handed to write_video_raw */
  int have_last_slot;        /* last_slot holds the last delivered frame (kept only for an operator frame export) */
  uint8_t last_slot[IVS_RUNNER_VIDEO_SLOT_MAX];
  uint8_t ppm[IVS_RUNNER_PPM_MAX];
  uint32_t slices;         /* run calls that returned IVS_OK */
  uint32_t slices_audio;   /* ... of which delivered into an audio buffer */
  uint32_t slices_video;   /* ... of which delivered into a video buffer */
  char detail[EX_OBS_BITS][IVS_RUNNER_DETAIL_MAX];
} ex_state;

static const uint32_t ex_role_kind[IVS_SCN_ROLES] = {IVS_KIND_EXEC, IVS_KIND_GROM, IVS_KIND_CARTRIDGE};
static const char *const ex_role_name[IVS_SCN_ROLES] = {"exec", "grom", "cart"};
static const size_t ex_role_cap[IVS_SCN_ROLES] = {IVS_RUNNER_CAP_EXEC, IVS_RUNNER_CAP_GROM, IVS_RUNNER_CAP_CART};

/* Digest rows in record order (report.c prints them in this order). */
#define EX_DOMAINS 9u
#define EX_ROW_VIDEO 5u
#define EX_ROW_AUDIO 6u
static const uint32_t ex_domains[EX_DOMAINS] = {IVS_DIGEST_CPU,       IVS_DIGEST_MEM,   IVS_DIGEST_IMAGES,
                                                IVS_DIGEST_INSTANCE,  IVS_DIGEST_COMPOSITE, IVS_DIGEST_VIDEO,
                                                IVS_DIGEST_AUDIO,     IVS_DIGEST_INPUT, IVS_DIGEST_PSGLOG};

/* Records an observation; the first detail of each kind is kept. */
static void ex_observe(ex_state *x, uint32_t bit, const char *detail)
{
  for (uint32_t i = 0u; i < EX_OBS_BITS; i++) {
    if (bit == (1u << i)) {
      if ((x->observed & bit) == 0u) {
        (void)snprintf(x->detail[i], sizeof x->detail[i], "%s", detail);
      }
      break;
    }
  }
  x->observed |= bit;
}

static const char *ex_status_token(ivs_status st) { return st == IVS_OK ? "ok" : ivs_status_name(st); }

uint32_t ivs_runner_stop_observation(uint32_t stop_reason, uint32_t expected_stop)
{
  if (expected_stop != IVS_SCN_STOP_TARGET && expected_stop != IVS_SCN_STOP_HALTED &&
      expected_stop != IVS_SCN_STOP_FAULTED) {
    return IVS_RUNNER_OBS_HARNESS_FAIL;
  }
#ifdef IVS_FAULT_RUNNER_STOP_EXPECT_IGNORED
  /* Fault (plan 04-18, D-20, D-23): the stop expectation is not compared. */
  if (stop_reason == IVS_STOP_TARGET || stop_reason == IVS_STOP_CPU_HALTED || stop_reason == IVS_STOP_GUEST_FAULT) {
    return 0u;
  }
#endif
  switch (stop_reason) {
  case IVS_STOP_TARGET:
    return expected_stop == IVS_SCN_STOP_TARGET ? 0u : IVS_RUNNER_OBS_TIMEOUT_BUDGET;
  case IVS_STOP_CPU_HALTED:
    return expected_stop == IVS_SCN_STOP_HALTED ? 0u : IVS_RUNNER_OBS_GUEST_FAIL;
  case IVS_STOP_GUEST_FAULT:
    return expected_stop == IVS_SCN_STOP_FAULTED ? 0u : IVS_RUNNER_OBS_GUEST_FAIL;
  default: /* IVS_STOP_INTERNAL (D-04: a harness failure) or not a stop reason */
    return IVS_RUNNER_OBS_HARNESS_FAIL;
  }
}

/* ---- host calls, each counted for host.calls ---- */

static int ex_cancelled(ex_state *x)
{
#ifdef IVS_FAULT_RUNNER_CANCEL_IGNORED
  /* Fault (plan 04-19, D-21): the cancel flag is never read, so a signal cannot end a run. */
  return 0;
#endif
  if (x->host->cancel_requested == NULL) {
    return 0;
  }
  x->rec->host_calls++;
  return x->host->cancel_requested(x->host->ctx) != 0;
}

/* now_ms, or 0 without a clock (then the wall cap never ends a run). */
static uint64_t ex_now_ms(ex_state *x)
{
  if (x->host->now_ms == NULL) {
    return 0u;
  }
  x->rec->host_calls++;
  return x->host->now_ms(x->host->ctx);
}

/* The check before every slice of step k (D-21): cancel_requested once, then now_ms once against
 * the wall cap. Both are observed when both hold (the precedence makes it cancelled). Returns 1
 * when the step ends here. */
static int ex_between_slices(ex_state *x, uint32_t k)
{
  char d[IVS_RUNNER_DETAIL_MAX];
  int cancel = ex_cancelled(x);
  int wall = 0;
  if (x->host->now_ms != NULL) {
    uint64_t t = ex_now_ms(x);
    wall = t >= x->wall_start && t - x->wall_start >= x->wall_cap_ms;
  }
  if (cancel) {
    (void)snprintf(d, sizeof d, "signal step=%u", (unsigned)(k + 1u));
    ex_observe(x, IVS_RUNNER_OBS_CANCELLED, d);
  }
  if (wall) {
    (void)snprintf(d, sizeof d, "wall step=%u cap-ms=%u", (unsigned)(k + 1u), (unsigned)x->wall_cap_ms);
#ifdef IVS_FAULT_RUNNER_WALL_AS_GUEST_FAIL
    /* Fault (plan 04-19, D-21): the wall cap is classified as a guest failure. */
    ex_observe(x, IVS_RUNNER_OBS_GUEST_FAIL, d);
#else
    ex_observe(x, IVS_RUNNER_OBS_TIMEOUT_WALL, d);
#endif
  }
  return cancel || wall;
}

/* Reads the bytes of one role into x->readbuf; returns an IVS_RUNNER_READ_* result and the
 * length in *len (zero-grom is made here, without a host call). */
static int ex_read_role(ex_state *x, uint32_t role, size_t *len)
{
  const ivs_scenario_content *c = &x->scn->content[role];
  size_t cap = ex_role_cap[role];
  int r;
  *len = 0u;
  switch (c->kind) {
  case IVS_SCN_CONTENT_ZERO_GROM:
    memset(x->readbuf, 0, IVS_RUNNER_CAP_GROM);
    *len = IVS_RUNNER_CAP_GROM;
    return IVS_RUNNER_READ_OK;
  case IVS_SCN_CONTENT_BOOT_STUB:
    x->rec->host_calls++;
    r = x->host->read_file(x->host->ctx, IVS_RUNNER_BOOT_STUB_FILE, cap, x->readbuf, len);
    break;
  case IVS_SCN_CONTENT_PATH:
    x->rec->host_calls++;
    r = x->host->read_file(x->host->ctx, c->path, cap, x->readbuf, len);
    break;
  case IVS_SCN_CONTENT_USER:
    if (x->host->read_user == NULL) {
      return IVS_RUNNER_READ_MISSING;
    }
    x->rec->host_calls++;
    r = x->host->read_user(x->host->ctx, role, cap, x->readbuf, len);
    break;
  default:
    return IVS_RUNNER_READ_IO_ERROR;
  }
  if (*len > cap + 1u) {
    return IVS_RUNNER_READ_IO_ERROR; /* the host broke its contract */
  }
  if (r == IVS_RUNNER_READ_TOO_LARGE && *len != cap + 1u) {
    return IVS_RUNNER_READ_IO_ERROR;
  }
  if (r == IVS_RUNNER_READ_OK && *len > cap) {
    return IVS_RUNNER_READ_TOO_LARGE;
  }
  return r;
}

/* The name a detail gives a role's content: path=<relpath>, path=boot-stub.exec or user. */
static void ex_content_name(const ex_state *x, uint32_t role, char *out, size_t cap)
{
  const ivs_scenario_content *c = &x->scn->content[role];
  if (c->kind == IVS_SCN_CONTENT_USER) {
    (void)snprintf(out, cap, "user");
  } else if (c->kind == IVS_SCN_CONTENT_BOOT_STUB) {
    (void)snprintf(out, cap, "path=%s", IVS_RUNNER_BOOT_STUB_FILE);
  } else if (c->kind == IVS_SCN_CONTENT_ZERO_GROM) {
    (void)snprintf(out, cap, "zero-grom");
  } else {
    (void)snprintf(out, cap, "path=%s", c->path);
  }
}

/* Loads every role with a content line. Returns 1 to go on, 0 when the scenario ends here (an
 * observation was made, or a refusal matched expect-load: pass, with a detail). */
static int ex_load_all(ex_state *x)
{
  char what[IVS_SCN_MAX_PATH + 16u];
  char d[IVS_RUNNER_DETAIL_MAX];
  for (uint32_t role = 0u; role < IVS_SCN_ROLES; role++) {
    const ivs_scenario_expect *e = &x->scn->expect[role];
    ivs_runner_slot_record *sl = &x->rec->slot[role];
    ivs_load_options lo;
    ivs_load_report lr;
    ivs_status st;
    size_t len = 0u;
    int r;
    int32_t want;
    if (x->scn->content[role].kind == IVS_SCN_CONTENT_NONE) {
      continue;
    }
    r = ex_read_role(x, role, &len);
    ex_content_name(x, role, what, sizeof what);
    if (r == IVS_RUNNER_READ_MISSING || r == IVS_RUNNER_READ_NOT_REGULAR) {
      (void)snprintf(d, sizeof d, "%s role=%s %s", r == IVS_RUNNER_READ_MISSING ? "missing" : "not-regular",
                     ex_role_name[role], what);
      ex_observe(x, IVS_RUNNER_OBS_MISSING_FIXTURE, d);
      return 0;
    }
    if (r != IVS_RUNNER_READ_OK && r != IVS_RUNNER_READ_TOO_LARGE) {
      (void)snprintf(d, sizeof d, "io-error role=%s %s", ex_role_name[role], what);
      ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, d);
      return 0;
    }
    memset(&lo, 0, sizeof lo);
    lo.struct_size = (uint32_t)sizeof lo;
    if (e->sha_set) {
      lo.flags = IVS_LOAD_F_EXPECT_SHA256;
      memcpy(lo.expect_sha256, e->sha256, sizeof lo.expect_sha256);
    }
    memset(&lr, 0, sizeof lr);
    lr.struct_size = (uint32_t)sizeof lr;
    lr.image.struct_size = (uint32_t)sizeof lr.image;
    st = ivs_machine_load(x->m, ex_role_kind[role], x->readbuf, len, &lo, &lr);
    sl->attempted = 1u;
    sl->status = (int32_t)st;
    sl->reason = st == IVS_OK ? IVS_REASON_NONE : lr.reason;
    if (st == IVS_OK) {
      sl->loaded = lr.image.loaded;
      sl->length_bytes = lr.image.length_bytes;
      memcpy(sl->sha256, lr.image.sha256, sizeof sl->sha256);
    }
    want = e->load_set ? e->load_status : IVS_OK;
    if (st == IVS_OK && want == IVS_OK) {
      continue;
    }
    if (st != IVS_OK && st == want && (e->load_reason == 0u || e->load_reason == lr.reason)) {
      /* D-20: a refusal the scenario expects ends it as pass. The parser guarantees nothing
       * follows it (no later role, run-to or input: review WR-02), so no line goes unchecked. */
      (void)snprintf(x->rec->detail, sizeof x->rec->detail, "load-refused-as-expected role=%s status=%s reason=%u",
                     ex_role_name[role], ivs_status_name(st), (unsigned)lr.reason);
      return 0;
    }
    if (st == IVS_OK) {
      (void)snprintf(d, sizeof d, "load-not-refused role=%s expected=%s", ex_role_name[role], ivs_status_name(want));
    } else {
      (void)snprintf(d, sizeof d, "load-refused role=%s status=%s reason=%u", ex_role_name[role], ex_status_token(st),
                     (unsigned)lr.reason);
    }
    ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, d);
    return 0;
  }
  return 1;
}

/* n * mult as a tick count; 0 when it exceeds IVS_RUN_TARGET_MAX or mult is 0. */
static int ex_ticks(uint64_t n, uint64_t mult, uint64_t *out)
{
  if (mult == 0u || n > IVS_RUN_TARGET_MAX / mult) {
    return 0;
  }
  *out = n * mult;
  return 1;
}

/* ---- asserts (D-08: observation through cpu_state, peek and state_digest only) ---- */

static uint64_t ex_get_be64(const uint8_t *p)
{
  uint64_t v = 0u;
  for (size_t i = 0u; i < 8u; i++) {
    v = (v << 8) | p[i];
  }
  return v;
}

/* Keeps the delivered write-log records (16 bytes each, whole records only) while the first
 * IVS_SCN_CAPTURE_MAX fit; one more is a harness failure, never a silent drop (T-05-24). */
static void ex_capture_log(ex_state *x, const uint8_t *b, size_t n)
{
  if ((x->scn->capture & IVS_SCN_CAPTURE_PSGLOG) == 0u) {
    return;
  }
  for (size_t off = 0u; n - off >= 16u; off += 16u) {
    if (x->cap_log_n >= IVS_SCN_CAPTURE_MAX) {
#ifndef IVS_FAULT_RUNNER_CAPTURE_OVERFLOW_SILENT
      ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "capture-overflow target=psglog");
#endif
      /* Fault RUNNER_CAPTURE_OVERFLOW_SILENT (plan 05-09, T-05-24): the entries past the bound are dropped
       * without a word, so an assert reads an incomplete capture. */
      return;
    }
    memcpy(x->cap_log + 16u * (size_t)x->cap_log_n, b + off, 16u);
    x->cap_log_n++;
  }
}

/* The operator's log file: one ASCII line per delivered record, `index tick addr value source`
 * (decimal fields, LF), through write_psg_log in small batches. first is the index of the slice's
 * first record. Returns 0, 1 on a write error, 2 when the file would pass the byte bound. */
static int ex_psg_log_out(ex_state *x, size_t n, uint64_t first)
{
  char buf[512];
  size_t used = 0u;
  for (size_t off = 0u; n - off >= 16u; off += 16u) {
    const uint8_t *rec = x->log_buf + off;
    int k = snprintf(buf + used, sizeof buf - used, "%llu %llu %u %u %u\n",
                     (unsigned long long)(first + off / 16u), (unsigned long long)ex_get_be64(rec),
                     (unsigned)rec[10], (unsigned)(((uint32_t)rec[8] << 8) | rec[9]), (unsigned)rec[11]);
    if (k < 0 || (size_t)k >= sizeof buf - used) {
      return 1;
    }
    used += (size_t)k;
    if (used > sizeof buf - 96u || n - off == 16u) {
      if ((uint64_t)used > x->out_max - x->out_log_bytes) {
        return 2;
      }
      if (x->host->write_psg_log(x->host->ctx, buf, used) != 0) {
        return 1;
      }
      x->out_log_bytes += used;
      used = 0u;
    }
  }
  return 0;
}

/* The WAV rates of the reference converter (plan 05-10, D-21): exactly 44100 and 48000. The option parser
 * of main.c refuses any other --wav-rate through this function, and so does the executor. */
int ivs_runner_wav_rate_valid(uint32_t rate)
{
#ifdef IVS_FAULT_RUNNER_WAV_RATE_UNCHECKED
  /* Fault (plan 05-10, D-20): any --wav-rate value is accepted. */
  (void)rate;
  return 1;
#else
  return rate == IVS_REFPCM_RATE_44100 || rate == IVS_REFPCM_RATE_48000;
#endif
}

/* Converts n_bytes of dense samples (4 bytes each) and hands the PCM to write_wav_pcm chunk by chunk, with the
 * converter state carried across calls (so slicing the run never changes the output). Bounded like the raw
 * files: the WAV may not pass the operator's byte bound, 44 header bytes included. 0, or 1 after a harness
 * failure observation. */
static int ex_wav_feed(ex_state *x, const uint8_t *b, size_t n_bytes)
{
  size_t n = n_bytes / 4u;
  size_t off = 0u;
  uint64_t limit = x->out_max > 44u ? x->out_max - 44u : 0u;
  while (off < n) {
    size_t m = n - off < EX_WAV_CHUNK ? n - off : EX_WAV_CHUNK;
    size_t produced = 0u;
    size_t bytes;
    while (m > 1u && ivs_refpcm_out_bound(&x->rp, m) > EX_WAV_FRAMES) {
      m /= 2u;
    }
    if (ivs_refpcm_feed(&x->rp, b + 4u * off, m, x->wav_pcm, EX_WAV_FRAMES, &produced) != IVS_REFPCM_OK) {
      x->wav_failed = 1;
      ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "wav-convert");
      return 1;
    }
    off += m;
    if (produced == 0u) {
      continue;
    }
    bytes = 2u * produced;
    for (size_t i = 0u; i < produced; i++) {
      uint16_t u = (uint16_t)x->wav_pcm[i];
      x->wav_bytes[2u * i] = (uint8_t)(u & 0xFFu);
      x->wav_bytes[2u * i + 1u] = (uint8_t)(u >> 8);
    }
    if ((uint64_t)bytes > limit || x->out_wav_bytes > limit - (uint64_t)bytes) {
      x->wav_failed = 1;
      ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "wav-out-max-bytes");
      return 1;
    }
    if (x->host->write_wav_pcm(x->host->ctx, x->wav_bytes, bytes) != 0) {
      x->wav_failed = 1;
      ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "wav-out");
      return 1;
    }
    x->out_wav_bytes += bytes;
  }
  return 0;
}

/* Keeps the audio level-change entries of the delivered samples (k as be64, then the four sample
 * bytes) with the same bound; called before the chain state takes the slice in. */
static void ex_capture_audio(ex_state *x, const uint8_t *b, size_t n)
{
  uint64_t k = x->chain.samples;
  uint32_t prev = x->chain.prev;
  if ((x->scn->capture & IVS_SCN_CAPTURE_AUDIO) == 0u) {
    return;
  }
  for (size_t off = 0u; n - off >= 4u; off += 4u) {
    uint32_t word = (uint32_t)b[off] | ((uint32_t)b[off + 1u] << 8) | ((uint32_t)b[off + 2u] << 16) |
                    ((uint32_t)b[off + 3u] << 24);
    k++;
    if (word != prev) {
      uint8_t *e;
      if (x->cap_aud_n >= IVS_SCN_CAPTURE_MAX) {
#ifndef IVS_FAULT_RUNNER_CAPTURE_OVERFLOW_SILENT
        ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "capture-overflow target=audio");
#endif
        return;
      }
      e = x->cap_aud + 12u * (size_t)x->cap_aud_n;
      for (size_t i = 0u; i < 8u; i++) {
        e[i] = (uint8_t)(k >> (56u - 8u * i));
      }
      memcpy(e + 8u, b + off, 4u);
      x->cap_aud_n++;
      prev = word;
    }
  }
}

/* The PSGLOG and AUDIO asserts: counts from the running totals, records and levels from the
 * captures. Returns 0 (held set), -1 (observation refused) or -2 (an `audio at` tick that is not a
 * multiple of the real sample period: a harness failure with its own detail). */
static int ex_assert_stream(ex_state *x, const ivs_scenario_assert *a, ivs_runner_assert_record *ar, int *held)
{
  const ivs_runner_record *r = x->rec;
  uint64_t have;
  if (a->subject == IVS_SCN_SUBJ_PSGLOG && a->form == IVS_SCN_FORM_COUNT) {
    have = x->log_record_bytes != 0u ? r->psglog_bytes / x->log_record_bytes : 0u;
    *held = have == a->num;
    (void)snprintf(ar->got, sizeof ar->got, "%llu", (unsigned long long)have);
    return 0;
  }
  if (a->subject == IVS_SCN_SUBJ_AUDIO && a->form == IVS_SCN_FORM_EMITTED) {
    *held = r->audio_samples == a->num;
    (void)snprintf(ar->got, sizeof ar->got, "%llu", (unsigned long long)r->audio_samples);
    return 0;
  }
  if (a->subject == IVS_SCN_SUBJ_PSGLOG && a->form == IVS_SCN_FORM_RECORD) {
    const uint8_t *rec;
    uint64_t tick;
    uint32_t value;
    uint32_t addr;
    uint32_t src;
    if (a->index >= x->cap_log_n) {
      *held = 0;
      (void)snprintf(ar->got, sizeof ar->got, "no-record");
      return 0;
    }
    rec = x->cap_log + 16u * (size_t)a->index;
    tick = ex_get_be64(rec);
    value = ((uint32_t)rec[8] << 8) | rec[9];
    addr = rec[10];
    src = rec[11];
    *held = ((a->fields & IVS_SCN_FIELD_TICK) == 0u || tick == a->num) &&
            ((a->fields & IVS_SCN_FIELD_ADDR) == 0u || addr == a->addr) &&
            ((a->fields & IVS_SCN_FIELD_VALUE) == 0u || value == a->value) &&
            ((a->fields & IVS_SCN_FIELD_SOURCE) == 0u || src == a->source);
    (void)snprintf(ar->got, sizeof ar->got, "tick=%llu addr=%u value=%04X source=%s", (unsigned long long)tick,
                   (unsigned)addr, (unsigned)value,
                   src == IVS_PSGLOG_SRC_CPU_WRITE ? "cpu-write" : src == IVS_PSGLOG_SRC_CPU_DW ? "cpu-dw" : "unknown");
    return 0;
  }
  if (a->subject == IVS_SCN_SUBJ_AUDIO && a->form == IVS_SCN_FORM_AT) {
    uint64_t period = r->audio_sample_ticks;
    uint64_t k;
    const uint8_t *lv = NULL;
    if (period == 0u || a->num % period != 0u) {
      return -2;
    }
    k = a->num / period;
    if (k > r->audio_samples) {
      *held = 0;
      (void)snprintf(ar->got, sizeof ar->got, "not-emitted");
      return 0;
    }
    /* The last level-change entry at or before sample k (entries are in increasing k); none: 0, 0, 0. */
    for (uint32_t i = 0u; i < x->cap_aud_n; i++) {
      const uint8_t *e = x->cap_aud + 12u * (size_t)i;
      if (ex_get_be64(e) > k) {
        break;
      }
      lv = e + 8u;
    }
    *held = (lv != NULL ? lv[0] : 0u) == a->level[0] && (lv != NULL ? lv[1] : 0u) == a->level[1] &&
            (lv != NULL ? lv[2] : 0u) == a->level[2];
    (void)snprintf(ar->got, sizeof ar->got, "A=%u B=%u C=%u", lv != NULL ? (unsigned)lv[0] : 0u,
                   lv != NULL ? (unsigned)lv[1] : 0u, lv != NULL ? (unsigned)lv[2] : 0u);
    return 0;
  }
  return -1;
}


/* Keeps the delivered slots whose index lies in the capture window `capture video <first> <count>` (whole slots only; the
 * window is at most IVS_SCN_CAPTURE_VIDEO_MAX slots, so nothing can overflow and nothing is silently dropped). */
static void ex_capture_video(ex_state *x, const uint8_t *b, size_t n)
{
  size_t slot = x->vid_slot_bytes;
  if ((x->scn->capture & IVS_SCN_CAPTURE_VIDEO) == 0u || slot == 0u) {
    return;
  }
  for (size_t off = 0u; n - off >= slot; off += slot) {
    uint64_t index = ex_get_be64(b + off + 8u);
    if (index >= x->scn->video_first && index - x->scn->video_first < x->scn->video_count) {
      uint32_t i = (uint32_t)(index - x->scn->video_first);
      memcpy(x->cap_vid + (size_t)i * IVS_RUNNER_VIDEO_SLOT_MAX, b + off, slot);
      x->cap_vid_mask |= 1u << i;
    }
  }
}

/* Returns the captured slot of frame n, or NULL when n lies outside the capture window or was never delivered. */
static const uint8_t *ex_frame_slot(const ex_state *x, uint64_t n)
{
  const ivs_scenario *s = x->scn;
  if ((s->capture & IVS_SCN_CAPTURE_VIDEO) == 0u || n < s->video_first || n - s->video_first >= s->video_count) {
    return NULL;
  }
  if ((x->cap_vid_mask & (1u << (uint32_t)(n - s->video_first))) == 0u) {
    return NULL;
  }
  return x->cap_vid + (size_t)(n - s->video_first) * IVS_RUNNER_VIDEO_SLOT_MAX;
}

static void ex_hex(const uint8_t sha[32], char *out)
{
  static const char digits[] = "0123456789abcdef";
  for (size_t i = 0u; i < 32u; i++) {
    out[2u * i] = digits[sha[i] >> 4];
    out[2u * i + 1u] = digits[sha[i] & 0x0Fu];
  }
  out[64] = '\0';
}

/* The FRAME and VIDEO asserts (plan 06-12). Returns 0 (held set), -3 (frame not captured), -4 (a hash assert on a
 * frame with a blank position), -5 (a pixel outside the published plane) or -1 (not a video form). */
static int ex_assert_video(ex_state *x, const ivs_scenario_assert *a, ivs_runner_assert_record *ar, int *held)
{
  const ivs_runner_record *r = x->rec;
  if (a->subject == IVS_SCN_SUBJ_VIDEO) {
    if (a->form == IVS_SCN_FORM_COUNT) {
      uint64_t have = x->vid_slot_bytes != 0u ? r->video_bytes / x->vid_slot_bytes : 0u;
      *held = have == a->num;
      (void)snprintf(ar->got, sizeof ar->got, "%llu", (unsigned long long)have);
      return 0;
    }
    if (a->form == IVS_SCN_FORM_CHAIN) {
      uint8_t v[32];
      ivs_runner_vchain_value(&x->vchain, v);
      *held = memcmp(v, a->sha256, sizeof v) == 0;
      ex_hex(v, ar->got);
      return 0;
    }
    return -1;
  }
  if (a->subject == IVS_SCN_SUBJ_FRAME) {
    const uint8_t *slot = ex_frame_slot(x, a->num);
    const uint8_t *plane;
    if (slot == NULL) {
      return -3;
    }
    plane = slot + IVS_FRAME_HEADER_BYTES;
    if (a->form == IVS_SCN_FORM_PIXEL) {
      uint8_t p;
      if (a->index >= x->vid_width || a->addr >= x->vid_height) {
        return -5;
      }
      p = plane[(size_t)a->addr * x->vid_width + a->index];
      *held = (uint32_t)p == a->level[0];
      if (p == IVS_PIXEL_BLANK) {
        (void)snprintf(ar->got, sizeof ar->got, "blank");
      } else {
        (void)snprintf(ar->got, sizeof ar->got, "%u", (unsigned)p);
      }
      return 0;
    }
    if (a->form == IVS_SCN_FORM_HASH) {
      uint8_t h[32];
      if (ivs_runner_frame_text_sha256(plane, x->vid_width, x->vid_height, h) != 0) {
        return -4;
      }
      *held = memcmp(h, a->sha256, sizeof h) == 0;
      ex_hex(h, ar->got);
      return 0;
    }
    if (a->form == IVS_SCN_FORM_DISPLAYED) {
      uint32_t f = ((uint32_t)slot[24] << 24) | ((uint32_t)slot[25] << 16) | ((uint32_t)slot[26] << 8) | slot[27];
      uint32_t shown = (f & IVS_FRAME_F_DISPLAYED) != 0u ? 1u : 0u;
      *held = shown == (uint32_t)a->value;
      (void)snprintf(ar->got, sizeof ar->got, "%s", shown != 0u ? "yes" : "no");
      return 0;
    }
  }
  return -1;
}

/* Evaluates assert a into ar. Returns 1 when it held, 0 when it failed, -1 when an observation
 * call was refused (a harness failure), or a code of ex_assert_video/ex_assert_stream below -1. */
static int ex_assert_eval(ex_state *x, const ivs_scenario_assert *a, ivs_runner_assert_record *ar)
{
  int held = 0;
  ivs_status st;
  switch (a->subject) {
  case IVS_SCN_SUBJ_HALTED:
  case IVS_SCN_SUBJ_REG: {
    ivs_cpu_state_report c;
    memset(&c, 0, sizeof c);
    c.struct_size = (uint32_t)sizeof c;
    st = ivs_machine_cpu_state(x->m, &c);
    if (st != IVS_OK) {
      return -1;
    }
    if (a->subject == IVS_SCN_SUBJ_HALTED) {
      uint16_t halted = (uint16_t)(c.run_state == IVS_CPU_STATE_HALTED ? 1u : 0u);
      held = halted == a->value;
      (void)snprintf(ar->got, sizeof ar->got, "%s", halted ? "yes" : "no");
    } else {
      if (a->index > 7u) {
        return -1;
      }
      held = c.r[a->index] == a->value;
      (void)snprintf(ar->got, sizeof ar->got, "%04X", (unsigned)c.r[a->index]);
    }
    break;
  }
  case IVS_SCN_SUBJ_MEM:
  case IVS_SCN_SUBJ_MEM_RANGE: {
    /* Every word of [lo, hi] equals the value; got is the common value or the first word that
     * differs, as AAAA=HHHH. */
    uint16_t words[64];
    uint32_t addr = a->addr_lo;
    uint32_t hi = a->addr_hi;
    int all = 1;
    if (hi < addr) {
      return -1;
    }
    (void)snprintf(ar->got, sizeof ar->got, "%04X", 0u);
    while (addr <= hi && all) {
      uint32_t n = hi - addr + 1u;
      if (n > 64u) {
        n = 64u;
      }
      st = ivs_machine_peek(x->m, (uint16_t)addr, n, words);
      if (st != IVS_OK) {
        return -1;
      }
      for (uint32_t i = 0u; i < n; i++) {
        if (words[i] != a->value) {
          all = 0;
          if (a->subject == IVS_SCN_SUBJ_MEM) {
            (void)snprintf(ar->got, sizeof ar->got, "%04X", (unsigned)words[i]);
          } else {
            (void)snprintf(ar->got, sizeof ar->got, "%04X=%04X", (unsigned)(addr + i), (unsigned)words[i]);
          }
          break;
        }
      }
      addr += n;
    }
    if (all) {
      (void)snprintf(ar->got, sizeof ar->got, "%04X", (unsigned)a->value);
    }
    held = all;
    break;
  }
  case IVS_SCN_SUBJ_DIGEST: {
    static const char digits[] = "0123456789abcdef";
    uint8_t sha[32];
    st = ivs_machine_state_digest(x->m, a->index, sha);
    if (st != IVS_OK) {
      return -1;
    }
    held = memcmp(sha, a->sha256, sizeof sha) == 0;
    for (size_t i = 0u; i < 32u; i++) {
      ar->got[2u * i] = digits[sha[i] >> 4];
      ar->got[2u * i + 1u] = digits[sha[i] & 0x0Fu];
    }
    ar->got[64] = '\0';
    break;
  }
  case IVS_SCN_SUBJ_PSGLOG:
  case IVS_SCN_SUBJ_AUDIO: {
    int r = ex_assert_stream(x, a, ar, &held);
    if (r != 0) {
      return r;
    }
    break;
  }
  case IVS_SCN_SUBJ_FRAME:
  case IVS_SCN_SUBJ_VIDEO: {
    int r = ex_assert_video(x, a, ar, &held);
    if (r != 0) {
      return r;
    }
    break;
  }
  default:
    return -1;
  }
  if (a->op == IVS_SCN_OP_NE) {
    held = !held;
  } else if (a->op != IVS_SCN_OP_EQ) {
    return -1;
  }
#ifdef IVS_FAULT_RUNNER_ASSERT_ALWAYS_PASS
  /* Fault (plan 04-18, D-20, D-23): every assert evaluates as held, so a runner cannot fail. */
  held = 1;
#endif
#ifdef IVS_FAULT_RUNNER_VIDEO_ASSERT_IGNORED
  /* Fault (plan 06-12, D-17): a frame assert evaluates as held, so a wrong pixel, hash or flag passes. */
  if (a->subject == IVS_SCN_SUBJ_FRAME) {
    held = 1;
  }
#endif
#ifdef IVS_FAULT_RUNNER_PSGLOG_ASSERT_IGNORED
  /* Fault (plan 05-09, D-20): a psglog assert evaluates as held, so a wrong count or record passes. */
  if (a->subject == IVS_SCN_SUBJ_PSGLOG) {
    held = 1;
  }
#endif
  return held ? 1 : 0;
}

/* Evaluates the asserts of step k; a failure is a guest-fail naming the first failing one. */
static void ex_step_asserts(ex_state *x, uint32_t k)
{
  for (uint32_t j = 0u; j < x->scn->n_asserts; j++) {
    const ivs_scenario_assert *a = &x->scn->asserts[j];
    ivs_runner_assert_record *ar = &x->rec->assert_rec[j];
    char d[IVS_RUNNER_DETAIL_MAX];
    int r;
    if (a->step != k) {
      continue;
    }
    r = ex_assert_eval(x, a, ar);
    if (r < 0) {
      ar->state = IVS_RUNNER_ASSERT_NOT_RUN;
      (void)snprintf(d, sizeof d, "%s assert=%u line=%u",
                     r == -2   ? "audio-tick-misaligned"
                     : r == -3 ? "frame-not-captured"
                     : r == -4 ? "hash-needs-colors-only"
                     : r == -5 ? "frame-pixel-range"
                               : "observe-refused",
                     (unsigned)(j + 1u), (unsigned)a->line);
      ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, d);
      return;
    }
    ar->state = r == 1 ? IVS_RUNNER_ASSERT_PASS : IVS_RUNNER_ASSERT_FAIL;
    if (ar->state == IVS_RUNNER_ASSERT_FAIL) {
      char clause[IVS_RUNNER_DETAIL_MAX / 2u];
      ivs_runner_assert_clause(a, clause, sizeof clause);
      (void)snprintf(d, sizeof d, "assert=%u line=%u %s got %s", (unsigned)(j + 1u), (unsigned)a->line, clause,
                     ar->got);
      ex_observe(x, IVS_RUNNER_OBS_GUEST_FAIL, d);
    }
  }
}

/* ---- steps ---- */

/* Cuts the host's buffer block into the three output channels (plan 05-09, A-11, D-17; plan 06-09, A-03): audio and the log each get
 * bytes_per_frame * (slice_ticks / nominal_frame_ticks + 1) bytes, the most a slice of slice_ticks can require (ivs_run.h), and video
 * gets video_bytes_per_frame * (slice_ticks / nominal_frame_ticks + 1 + video_slack_frames), the slack read from ivs_info v4 (a call
 * can complete one frame more than the Phase 4 count). The per-frame bytes come from ivs_info (video; audio; the log's record bytes
 * times its records per frame). A channel whose layout needs 0 bytes is not requested (NULL with capacity 0). No block: nothing is
 * requested. 0 when the block cannot hold the three (a harness failure, never a truncation). */
static int ex_plan_buffers(ex_state *x, const ivs_info *info)
{
  uint64_t slots = x->slice_ticks / info->nominal_frame_ticks + 1u;
  uint64_t need[3];
  uint64_t total = 0u;
  uint64_t block = x->host->buffer_bytes > IVS_RUNNER_BUFFERS_MAX ? IVS_RUNNER_BUFFERS_MAX : x->host->buffer_bytes;
  uint64_t per[3];
  uint8_t *p = x->host->buffers;
  if (p == NULL || block == 0u) {
    return 1;
  }
  per[0] = info->video_bytes_per_frame;
  per[1] = info->audio_bytes_per_frame;
  per[2] = (uint64_t)info->psg_log_record_bytes * (uint64_t)info->psg_log_records_per_frame;
  for (size_t i = 0u; i < 3u; i++) {
    uint64_t n = i == 0u ? slots + (uint64_t)info->video_slack_frames : slots; /* the video channel carries the slack frames */
    need[i] = per[i] * n; /* at most 2^32 * 2^32 / 1: slots is below 2^23 for a slice of 2^22 ticks */
    if (need[i] > block || total > block - need[i]) {
      return 0;
    }
    total += need[i];
  }
  if (need[0] != 0u) {
    x->vid_buf = p;
    x->vid_cap = (size_t)need[0];
  }
  p += need[0];
  if (need[1] != 0u) {
    x->aud_buf = p;
    x->aud_cap = (size_t)need[1];
  }
  p += need[1];
  if (need[2] != 0u) {
    x->log_buf = p;
    x->log_cap = (size_t)need[2];
  }
  return 1;
}


static void ex_run_steps(ex_state *x, const ivs_info *info)
{
  const ivs_scenario *s = x->scn;
  ivs_input_event events[IVS_SCN_MAX_INPUTS];
  uint32_t consumed = 0u;
  char d[IVS_RUNNER_DETAIL_MAX];
  uint64_t step_mult = 1u;
  uint64_t input_mult = s->input_unit == IVS_SCN_INPUT_AT_CYCLE ? info->ticks_per_cpu_cycle : 1u;
  if (s->step_unit == IVS_SCN_UNIT_CPU_CYCLES) {
    step_mult = info->ticks_per_cpu_cycle;
  } else if (s->step_unit == IVS_SCN_UNIT_FRAMES) {
    step_mult = info->nominal_frame_ticks;
  }
  /* The input schedule in master ticks (D-09, D-13); words 2 and 3 stay 0. */
  memset(events, 0, sizeof events);
  for (uint32_t i = 0u; i < s->n_inputs && i < IVS_SCN_MAX_INPUTS; i++) {
    if (!ex_ticks(s->inputs[i].at, input_mult, &events[i].at_tick) || events[i].at_tick >= IVS_RUN_TARGET_MAX) {
      (void)snprintf(d, sizeof d, "input-out-of-range line=%u", (unsigned)s->inputs[i].line);
      ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, d);
      return;
    }
    events[i].controller[0] = s->inputs[i].controller[0];
    events[i].controller[1] = s->inputs[i].controller[1];
  }
  if (!ex_plan_buffers(x, info)) {
    ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "buffers-too-small");
    return;
  }
  x->rec->audio_sample_ticks = info->audio_sample_ticks;
  x->log_record_bytes = info->psg_log_record_bytes;
  x->vid_slot_bytes = info->video_slot_bytes;
  x->vid_width = info->video_width;
  x->vid_height = info->video_height;
  ivs_runner_vchain_init(&x->vchain, info->video_width, info->video_height, info->nominal_frame_ticks);
  if (x->wav_requested) {
    /* The operator WAV needs the dense stream: no audio buffer is a harness failure, never an empty file. */
    if (x->aud_buf == NULL || info->audio_sample_bytes != 4u) {
      x->wav_failed = 1;
      ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "wav-without-audio");
      return;
    }
    if (ivs_refpcm_init(&x->rp, info->crystal_hz_num, info->crystal_hz_den, info->audio_sample_ticks, x->wav_rate) !=
        IVS_REFPCM_OK) {
      x->wav_failed = 1;
      ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "wav-init");
      return;
    }
    x->wav_started = 1;
  }
  {
    /* A capture or a psglog or audio assert reads what the buffers deliver: without the buffer it
     * would read nothing and could pass vacuously (a harness failure instead). */
    int want_log = (s->capture & IVS_SCN_CAPTURE_PSGLOG) != 0u;
    int want_aud = (s->capture & IVS_SCN_CAPTURE_AUDIO) != 0u;
    int want_vid = (s->capture & IVS_SCN_CAPTURE_VIDEO) != 0u;
    for (uint32_t j = 0u; j < s->n_asserts; j++) {
      want_log = want_log || s->asserts[j].subject == IVS_SCN_SUBJ_PSGLOG;
      want_aud = want_aud || s->asserts[j].subject == IVS_SCN_SUBJ_AUDIO;
      want_vid = want_vid || s->asserts[j].subject == IVS_SCN_SUBJ_FRAME || s->asserts[j].subject == IVS_SCN_SUBJ_VIDEO;
    }
    if ((want_log && (x->log_buf == NULL || x->log_record_bytes == 0u)) || (want_aud && x->aud_buf == NULL) ||
        (want_vid && (x->vid_buf == NULL || x->vid_slot_bytes == 0u))) {
      ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL,
                 want_log && x->log_buf == NULL   ? "stream-without-buffer target=psglog"
                 : want_aud && x->aud_buf == NULL ? "stream-without-buffer target=audio"
                                                  : "stream-without-buffer target=video");
      return;
    }
    if (want_vid && x->vid_slot_bytes > IVS_RUNNER_VIDEO_SLOT_MAX) {
      ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "video-slot-too-large");
      return;
    }
  }
  x->wall_start = ex_now_ms(x);
  for (uint32_t k = 0u; k < s->n_steps && k < IVS_SCN_MAX_STEPS; k++) {
    const ivs_scenario_step *st = &s->steps[k];
    ivs_runner_step_record *sr = &x->rec->step[k];
    uint64_t limit;
    uint32_t stop = 0u;
    uint32_t sobs;
    if (!ex_ticks(st->target, step_mult, &sr->target_ticks)) {
      (void)snprintf(d, sizeof d, "bound-too-large step=%u line=%u", (unsigned)(k + 1u), (unsigned)st->line);
      ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, d);
      return;
    }
    sr->converted = 1u;
    limit = sr->target_ticks < x->max_ticks ? sr->target_ticks : x->max_ticks;
    /* Slices (D-21): each ends at the first unit boundary at or after its target; the loop ends at
     * a stop that is not TARGET or once machine time reached the limit. */
    for (;;) {
      ivs_run_request q;
      ivs_run_report rep;
      ivs_status rs;
      if (ex_between_slices(x, k)) {
        return;
      }
#ifdef IVS_FAULT_RUNNER_SLICE_DROPS_REMAINDER
      /* Fault (plan 04-19, D-21): after a slice, a last slice shorter than slice_ticks is skipped. */
      if (sr->ran && x->now < limit && limit - x->now < x->slice_ticks) {
        break;
      }
#endif
      memset(&q, 0, sizeof q);
      q.struct_size = (uint32_t)sizeof q;
      q.flags = IVS_RUN_F_STOP_ON_HALT;
      q.target_ticks = (x->now < limit && limit - x->now > x->slice_ticks) ? x->now + x->slice_ticks : limit;
      q.video = x->vid_buf;
      q.video_capacity = x->vid_cap;
      q.audio = x->aud_buf;
      q.audio_capacity = x->aud_cap;
      q.psg_log = x->log_buf;
      q.psg_log_capacity = x->log_cap;
      q.events = consumed < s->n_inputs ? &events[consumed] : NULL;
      q.event_count = s->n_inputs - consumed;
      memset(&rep, 0, sizeof rep);
      rep.struct_size = (uint32_t)sizeof rep;
      rs = ivs_machine_run(x->m, &q, &rep);
      if (rs != IVS_OK) {
        (void)snprintf(d, sizeof d, "run-refused step=%u status=%s reason=%u", (unsigned)(k + 1u),
                       ivs_status_name(rs), (unsigned)rep.refusal_reason);
        ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, d);
        return;
      }
      x->now = rep.now_ticks;
      stop = rep.stop_reason;
      sr->ran = 1u;
      sr->now_ticks = x->now;
      sr->overshoot_ticks = x->now > sr->target_ticks ? x->now - sr->target_ticks : 0u;
      sr->stop_reason = stop;
      consumed += rep.input_consumed;
      x->rec->have_input = 1u;
      x->rec->input_consumed = consumed;
      memcpy(x->rec->controller, rep.controller_now, sizeof x->rec->controller);
      x->rec->video_bytes += rep.video_bytes_written;
      x->rec->video_frames += rep.video_frames_completed;
      if (rep.last_frame_end_ticks != 0u) {
        x->rec->video_last_tick = rep.last_frame_end_ticks;
        x->rec->video_have_frame = 1u;
      }
      x->rec->audio_bytes += rep.audio_bytes_written;
      x->rec->psglog_bytes += rep.psg_log_bytes_written;
      x->rec->audio_samples = x->rec->audio_bytes / 4u; /* the running total the audio asserts read */
      x->slices++;
      if (x->aud_buf != NULL) {
        /* Dense 4-byte samples, never more than the capacity: the chain re-hash reads what the
         * core delivered and nothing else (D-18). */
        size_t got = rep.audio_bytes_written > x->aud_cap ? x->aud_cap : (size_t)rep.audio_bytes_written;
        if (x->host->write_audio_raw != NULL && got != 0u) {
          /* The operator's raw file: bounded, a refusal is a harness failure, never a truncation. */
          if ((uint64_t)got > x->out_max - x->out_audio_bytes) {
            ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "audio-out-max-bytes");
            return;
          }
          if (x->host->write_audio_raw(x->host->ctx, x->aud_buf, got) != 0) {
            ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "audio-out");
            return;
          }
          x->out_audio_bytes += got;
        }
        if (x->wav_started && got != 0u && ex_wav_feed(x, x->aud_buf, got) != 0) {
          return;
        }
        ex_capture_audio(x, x->aud_buf, got);
        ivs_runner_achain_feed(&x->chain, x->aud_buf, got);
        x->slices_audio++;
      }
      if (x->vid_buf != NULL) {
        /* Whole slots, never more than the capacity: the chain re-hash reads what the core delivered (D-16). */
        size_t vgot = rep.video_bytes_written > x->vid_cap ? x->vid_cap : (size_t)rep.video_bytes_written;
        if (x->host->write_video_raw != NULL && vgot != 0u) {
          /* The operator's raw file: bounded, a refusal is a harness failure, never a truncation. */
          if ((uint64_t)vgot > x->out_video_max - x->out_video_bytes) {
            ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "video-out-max-bytes");
            return;
          }
          if (x->host->write_video_raw(x->host->ctx, x->vid_buf, vgot) != 0) {
            ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "video-out");
            return;
          }
          x->out_video_bytes += vgot;
        }
        ex_capture_video(x, x->vid_buf, vgot);
        if (x->host->write_frame_ppm != NULL && x->vid_slot_bytes != 0u && x->vid_slot_bytes <= IVS_RUNNER_VIDEO_SLOT_MAX &&
            vgot >= x->vid_slot_bytes) {
          /* The last whole slot of this slice is the last frame delivered so far. */
          memcpy(x->last_slot, x->vid_buf + (vgot / x->vid_slot_bytes - 1u) * x->vid_slot_bytes, x->vid_slot_bytes);
          x->have_last_slot = 1;
        }
        (void)ivs_runner_vchain_feed(&x->vchain, x->vid_buf, vgot, rep.video_first_index);
        x->slices_video++;
      }
      if (x->log_buf != NULL) {
        size_t lgot = rep.psg_log_bytes_written > x->log_cap ? x->log_cap : (size_t)rep.psg_log_bytes_written;
        if (x->host->write_psg_log != NULL && lgot != 0u) {
          int lo = ex_psg_log_out(x, lgot, (x->rec->psglog_bytes - rep.psg_log_bytes_written) / x->log_record_bytes);
          if (lo != 0) {
            ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, lo == 2 ? "psg-log-out-max-bytes" : "psg-log-out");
            return;
          }
        }
        ex_capture_log(x, x->log_buf, lgot);
      }
      if ((x->observed & IVS_RUNNER_OBS_HARNESS_FAIL) != 0u) {
        return;
      }
      if (stop != IVS_STOP_TARGET || x->now >= limit) {
        break;
      }
    }
    if (stop == IVS_STOP_TARGET && limit < sr->target_ticks && x->now < sr->target_ticks) {
      /* The max_ticks ceiling cut this run-to short: a deterministic budget timeout. A last unit
       * that straddled the ceiling and carried machine time to or past the target did not cut the
       * run-to short: the machine is the one an uncapped run gives, so the step is judged as
       * usual (review WR-01). */
      (void)snprintf(d, sizeof d, "budget step=%u line=%u max-ticks=%llu", (unsigned)(k + 1u), (unsigned)st->line,
                     (unsigned long long)x->max_ticks);
      ex_observe(x, IVS_RUNNER_OBS_TIMEOUT_BUDGET, d);
      return;
    }
    sobs = ivs_runner_stop_observation(stop, st->stop);
    if ((sobs & IVS_RUNNER_OBS_HARNESS_FAIL) != 0u) {
      (void)snprintf(d, sizeof d, "stop-internal step=%u line=%u got=%s", (unsigned)(k + 1u), (unsigned)st->line,
                     ivs_runner_stop_token(stop));
      ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, d);
      return;
    }
    if (sobs != 0u) {
      (void)snprintf(d, sizeof d, "%s step=%u line=%u expected=%s got=%s",
                     sobs == IVS_RUNNER_OBS_TIMEOUT_BUDGET ? "budget" : "stop", (unsigned)(k + 1u),
                     (unsigned)st->line, ivs_runner_expected_stop_token(st->stop), ivs_runner_stop_token(stop));
      ex_observe(x, sobs, d);
    }
    ex_step_asserts(x, k);
    if (x->observed != 0u) {
      return;
    }
  }
}

/* The audio chain check (D-18): the runner recomputed the level-change chain from the dense bytes it
 * received; the AUDIO digest it implies must equal the core's domain-17 digest. Not checked (`-`)
 * unless an audio buffer was delivered on every slice of at least one slice. A mismatch is a harness
 * failure with the detail audio-chain (the core's own state and what it delivered disagree). */
static void ex_audio_chain_check(ex_state *x)
{
  uint8_t calc[32];
  int ok;
  if (x->slices == 0u || x->slices != x->slices_audio) {
    x->rec->chain_check = IVS_RUNNER_CHAIN_NONE;
    return;
  }
  ivs_runner_achain_digest(&x->chain, calc);
  ok = memcmp(calc, x->rec->digest[EX_ROW_AUDIO], sizeof calc) == 0;
#ifdef IVS_FAULT_RUNNER_AUDIO_CHAIN_CHECK_SKIPPED
  /* Fault (plan 05-09, D-18): the chain is not compared, the check reports pass whatever was delivered. */
  ok = 1;
#endif
  x->rec->chain_check = ok ? IVS_RUNNER_CHAIN_PASS : IVS_RUNNER_CHAIN_FAIL;
  if (!ok) {
    ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "audio-chain");
  }
}

/* The video chain check (D-16): the runner recomputed the frame chain from the slots it received and checked every
 * header; the domain-16 digest it implies must equal the core's, and no header finding may stand. Not checked (`-`)
 * unless a video buffer was delivered on every slice of at least one slice. A mismatch is a harness failure with
 * the detail video-chain (the core's own state and what it delivered disagree). */
static void ex_video_chain_check(ex_state *x)
{
  uint8_t calc[32];
  int ok;
  ivs_runner_vchain_value(&x->vchain, x->rec->video_chain);
  if (x->slices == 0u || x->slices != x->slices_video) {
    x->rec->video_chain_check = IVS_RUNNER_CHAIN_NONE;
    return;
  }
  ivs_runner_vchain_digest(&x->vchain, calc);
  ok = memcmp(calc, x->rec->digest[EX_ROW_VIDEO], sizeof calc) == 0 && x->vchain.fault == IVS_RUNNER_VCHAIN_OK;
#ifdef IVS_FAULT_RUNNER_VIDEO_CHAIN_UNCHECKED
  /* Fault (plan 06-12, D-16): the chain is not compared, the check reports pass whatever was delivered. */
  ok = 1;
#endif
  x->rec->video_chain_check = ok ? IVS_RUNNER_CHAIN_PASS : IVS_RUNNER_CHAIN_FAIL;
  if (!ok) {
    ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "video-chain");
  }
}

/* The operator's frame export (plan 06-12): the last delivered frame as a PPM through write_frame_ppm, once, after the run. A frame
 * that is not a plane of color indices, a size above the buffer or the byte bound, and a write error are harness failures. */
static void ex_frame_ppm(ex_state *x)
{
  size_t written = 0u;
  size_t bound;
  if (x->host->write_frame_ppm == NULL || !x->have_last_slot || (x->observed & IVS_RUNNER_OBS_HARNESS_FAIL) != 0u) {
    return;
  }
  bound = ivs_frameppm_bound(x->vid_width, x->vid_height);
  if (bound == 0u || bound > sizeof x->ppm ||
      ivs_frameppm_convert(x->last_slot + IVS_FRAME_HEADER_BYTES, x->vid_width, x->vid_height, x->ppm, sizeof x->ppm, &written) !=
          IVS_FRAMEPPM_OK) {
    ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "frame-ppm-convert");
    return;
  }
  if ((uint64_t)written > x->out_video_max) {
    ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "frame-ppm-out-max-bytes");
    return;
  }
  if (x->host->write_frame_ppm(x->host->ctx, x->ppm, written) != 0) {
    ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "frame-ppm-out");
  }
}

/* Final observation for the report: CPU state and every digest domain. */
static void ex_final(ex_state *x)
{
  ivs_cpu_state_report c;
  memset(&c, 0, sizeof c);
  c.struct_size = (uint32_t)sizeof c;
  if (ivs_machine_cpu_state(x->m, &c) != IVS_OK) {
    ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "observe-refused final");
    return;
  }
  x->rec->cpu_run_state = c.run_state;
  x->rec->cpu_fault_reason = c.fault_reason;
  for (size_t i = 0u; i < EX_DOMAINS; i++) {
    if (ivs_machine_state_digest(x->m, ex_domains[i], x->rec->digest[i]) != IVS_OK) {
      ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "observe-refused final");
      return;
    }
  }
  x->rec->have_machine = 1u;
  x->rec->audio_samples = x->rec->audio_bytes / 4u;
  x->rec->audio_changes = x->chain.changes;
  if (x->log_record_bytes != 0u) {
    x->rec->psglog_records = x->rec->psglog_bytes / x->log_record_bytes;
  }
  ex_audio_chain_check(x);
  ex_video_chain_check(x);
}

/* Creates the instance in host storage, aligned by hand (D-17, D-22). */
static int ex_create(ex_state *x)
{
  ivs_storage_requirements req;
  ivs_create_options co;
  ivs_status st;
  uintptr_t base;
  uintptr_t aligned;
  size_t usable;
  size_t pad;
  size_t need_tail;
  char d[IVS_RUNNER_DETAIL_MAX];
  memset(&req, 0, sizeof req);
  req.struct_size = (uint32_t)sizeof req;
  st = ivs_machine_storage_requirements(x->scn->profile, &req);
  if (st != IVS_OK) {
    (void)snprintf(d, sizeof d, "profile-refused status=%s", ivs_status_name(st));
    ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, d);
    return 0;
  }
  usable = x->host->storage_bytes > IVS_RUNNER_STORAGE_MAX ? IVS_RUNNER_STORAGE_MAX : x->host->storage_bytes;
  if (x->host->storage == NULL || req.align_bytes == 0u || (req.align_bytes & (req.align_bytes - 1u)) != 0u ||
      req.size_bytes > usable) {
    ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "storage-too-small");
    return 0;
  }
  base = (uintptr_t)x->host->storage;
  aligned = (base + (uintptr_t)(req.align_bytes - 1u)) & ~(uintptr_t)(req.align_bytes - 1u);
  pad = (size_t)(aligned - base);
  need_tail = EX_READ_BUF + (x->scn->capture != 0u ? (size_t)IVS_RUNNER_CAPTURE_BYTES : 0u);
  if (pad > usable || usable - pad < req.size_bytes || usable - pad - (size_t)req.size_bytes < need_tail) {
    ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "storage-too-small");
    return 0;
  }
  memset(&co, 0, sizeof co);
  co.struct_size = (uint32_t)sizeof co;
  co.profile_id = x->scn->profile;
  st = ivs_machine_create(&co, (void *)aligned, (size_t)req.size_bytes, &x->m);
  if (st != IVS_OK) {
    x->m = NULL;
    (void)snprintf(d, sizeof d, "create-refused status=%s", ivs_status_name(st));
    ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, d);
    return 0;
  }
  x->readbuf = (uint8_t *)aligned + (size_t)req.size_bytes;
  if (x->scn->capture != 0u) {
    x->cap_log = x->readbuf + EX_READ_BUF;
    x->cap_aud = x->cap_log + (size_t)IVS_SCN_CAPTURE_MAX * 16u;
    x->cap_vid = x->cap_aud + (size_t)IVS_SCN_CAPTURE_MAX * 12u;
  }
  return 1;
}

/* 1 when any role of the scenario is `content <role> user`. */
static int ex_names_user_content(const ivs_scenario *scn)
{
  for (uint32_t role = 0u; role < IVS_SCN_ROLES; role++) {
    if (scn->content[role].kind == IVS_SCN_CONTENT_USER) {
      return 1;
    }
  }
  return 0;
}

uint32_t ivs_runner_execute(const ivs_scenario *scn, const ivs_runner_host *host, const ivs_runner_options *opts,
                            ivs_runner_result *result)
{
  ivs_runner_record rec;
  ex_state x;
  ivs_info info;
  uint64_t lines = 0u;
  uint32_t outcome;
  int wrote;
  if (result != NULL) {
    memset(result, 0, sizeof *result);
    result->outcome = IVS_RUNNER_OUTCOME_HARNESS_FAIL;
    result->exit_code = ivs_runner_exit_code(IVS_RUNNER_OUTCOME_HARNESS_FAIL);
  }
  if (scn == NULL || host == NULL || host->read_file == NULL || host->write_report == NULL ||
      (opts != NULL && (opts->flags != 0u || opts->slice_ticks > IVS_RUNNER_SLICE_TICKS_MAX ||
                        opts->max_ticks > IVS_RUN_TARGET_MAX || opts->reserved != 0u ||
                        (opts->wav_rate != 0u && !ivs_runner_wav_rate_valid(opts->wav_rate)))) ||
      (host->write_wav_pcm == NULL) != (host->finish_wav == NULL)) {
    return IVS_RUNNER_OUTCOME_HARNESS_FAIL;
  }
  memset(&rec, 0, sizeof rec);
  memset(&x, 0, sizeof x);
  rec.scn = scn;
  x.scn = scn;
  x.host = host;
  x.rec = &rec;
  x.slice_ticks = opts != NULL && opts->slice_ticks != 0u ? opts->slice_ticks : IVS_RUNNER_SLICE_TICKS_MAX;
  x.max_ticks = opts != NULL && opts->max_ticks != 0u ? opts->max_ticks : IVS_RUNNER_MAX_TICKS_DEFAULT;
  x.wall_cap_ms = opts != NULL && opts->wall_timeout_ms != 0u ? opts->wall_timeout_ms : IVS_RUNNER_WALL_MS_DEFAULT;
  x.out_max = opts != NULL && opts->audio_out_max_bytes != 0u ? opts->audio_out_max_bytes
                                                              : IVS_RUNNER_AUDIO_OUT_MAX_DEFAULT;
  x.out_video_max = opts != NULL && opts->video_out_max_bytes != 0u ? opts->video_out_max_bytes
                                                                    : IVS_RUNNER_VIDEO_OUT_MAX_DEFAULT;
  x.wav_requested = host->write_wav_pcm != NULL;
  x.wav_rate = opts != NULL && opts->wav_rate != 0u ? opts->wav_rate : IVS_RUNNER_WAV_RATE_DEFAULT;
  if (scn->wall_timeout_ms != 0u && scn->wall_timeout_ms < x.wall_cap_ms) {
    x.wall_cap_ms = scn->wall_timeout_ms; /* D-21: a scenario only lowers the cap */
  }
  if (host->write_frame_ppm != NULL && ex_names_user_content(scn)) {
    /* A frame rendered from an owner image is derived art of rights-bearing content: no export (T-06-17). */
    ex_observe(&x, IVS_RUNNER_OBS_HARNESS_FAIL, "frame-ppm-user-content");
  } else if (ex_create(&x) && ex_load_all(&x)) {
    ivs_status st = ivs_machine_reset(x.m, IVS_RESET_POWER_ON);
    if (st != IVS_OK) {
      char d[IVS_RUNNER_DETAIL_MAX];
      (void)snprintf(d, sizeof d, "reset-refused status=%s", ivs_status_name(st));
      ex_observe(&x, IVS_RUNNER_OBS_HARNESS_FAIL, d);
    } else {
      memset(&info, 0, sizeof info);
      info.struct_size = (uint32_t)sizeof info;
      st = ivs_machine_info(x.m, &info);
      if (st != IVS_OK || info.ticks_per_cpu_cycle == 0u || info.nominal_frame_ticks == 0u) {
        ex_observe(&x, IVS_RUNNER_OBS_HARNESS_FAIL, "no-timebase");
      } else {
        x.now = info.time_ticks;
        rec.video_width = info.video_width;
        rec.video_height = info.video_height;
        rec.video_row_lines = info.video_row_lines;
        ex_run_steps(&x, &info);
      }
    }
  }
  if (x.m != NULL) {
    ex_final(&x);
    ex_frame_ppm(&x);
  }
  if (x.wav_requested) {
    /* The operator WAV is finished once, whatever the scenario ended as (the file holds what the run
     * produced), unless a hook or the byte bound already failed it. */
    rec.refpcm_requested = 1u;
    rec.refpcm_rate = x.wav_rate;
    rec.refpcm_frames = x.rp.frames;
    if (!x.wav_failed) {
      uint8_t header[IVS_REFPCM_WAV_HEADER_BYTES];
      if (ivs_refpcm_wav_header(header, x.wav_rate, x.rp.frames) == IVS_REFPCM_OK &&
          host->finish_wav(host->ctx, header, rec.refpcm_sha256) == 0) {
        rec.refpcm_sha_set = 1u;
      } else {
        ex_observe(&x, IVS_RUNNER_OBS_HARNESS_FAIL, "wav-out");
      }
    }
  }
  outcome = ivs_runner_outcome_resolve(x.observed);
  if (x.observed != 0u) {
    /* The detail of the winning observation (the first of its kind). */
    for (uint32_t i = 0u; i < EX_OBS_BITS; i++) {
      uint32_t bit = 1u << i;
      if ((x.observed & bit) != 0u && ivs_runner_outcome_resolve(bit) == outcome) {
        (void)snprintf(rec.detail, sizeof rec.detail, "%s", x.detail[i]);
        break;
      }
    }
  }
  rec.outcome = outcome;
  if (outcome == IVS_RUNNER_OUTCOME_HARNESS_FAIL) {
    /* ivs-run removes its output files after a harness failure (review WR-01), so the report states no WAV:
     * refpcm.frames and refpcm.sha256 are `-` (a library host decides for itself what it keeps). */
    rec.refpcm_sha_set = 0u;
  }
  if (x.m != NULL) {
    (void)ivs_machine_destroy(x.m);
    x.m = NULL;
  }
  wrote = ivs_runner_report_write(&rec, host, &lines);
  if (wrote != 0) {
    outcome = IVS_RUNNER_OUTCOME_HARNESS_FAIL;
  }
  if (result != NULL) {
    result->outcome = outcome;
    result->exit_code = ivs_runner_exit_code(outcome);
    result->host_calls = rec.host_calls;
    result->report_lines = lines;
    result->report_write_failed = wrote != 0 ? 1u : 0u;
  }
  return outcome;
}
