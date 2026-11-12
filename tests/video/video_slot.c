/* SPDX-License-Identifier: MIT */
/* Suite video, family slot (plan 06-09, D-13, D-16, A-06): the first display-off frames delivered as slots whose header, plane and chain
 * step equal the independently framed literal of data/video/chain-framing.literal.txt (a shell recipe over printf and cmake -E
 * sha256sum, never the core), the displayed flag, the frame count, the no-op of a missing buffer and the chain recomputed from the
 * delivered bytes (the core's one-shot SHA-256, a different API path from the incremental one the delivery uses, over a concatenation
 * built here). Frames come from stand-in guests of at most five words (a DIS then B self loop, with or without a strobe or a border
 * store); frame 0 ends at tick 59,736 and frame 1 at 119,472 (the literal). */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common/ivs_sha256.h"
#include "video_cases.h"

#define VS_SLOT 15296u
#define VS_E0 59736u
#define VS_E1 119472u

/* Runs the guest of w[0..n) to tick `ticks` with a buffer of `slots` slots; 1 on success. */
static int vs_run(vp_run *r, const uint16_t *w, size_t n, uint32_t slots, uint64_t ticks)
{
  if (!vp_new(r, w, n, (uint64_t)slots * VS_SLOT)) {
    return 0;
  }
  if (!vp_until(r, ticks)) {
    vp_free(r);
    return 0;
  }
  return 1;
}

static uint64_t vs_be(const uint8_t *p, uint32_t n)
{
  uint64_t v = 0u;
  uint32_t i;
  for (i = 0u; i < n; i++) {
    v = (v << 8) | p[i];
  }
  return v;
}

/* 1 when every plane byte of the slot at s is v. */
static int vs_plane_all(const uint8_t *s, uint8_t v)
{
  uint32_t i;
  for (i = 32u; i < VS_SLOT; i++) {
    if (s[i] != v) {
      return 0;
    }
  }
  return 1;
}

static int vs_layout_bytes(void)
{
  vp_run r;
  uint16_t w[SC_PROG_MAX];
  size_t n = vp_prog_off(w);
  VC_CHECK(vs_run(&r, w, n, 3u, VS_E1));
  VC_CHECK(r.sink.video_frames == 2u && r.sink.video_written == 2u * VS_SLOT && r.sink.video_overflow == 0u);
  VC_CHECK(vp_im(&r)->obs.video.count == 2u);
  VC_CHECK(r.buf[2u * VS_SLOT] == VP_POISON && r.buf[3u * VS_SLOT - 1u] == VP_POISON); /* nothing past the two slots */
  VC_CHECK(VS_SLOT == ivs_stic_geom_derive(ivs_stic_params_get(SC_PROFILE)).slot_bytes);
  vp_free(&r);
  return IVS_TEST_PASS;
}

static int vs_header_bytes(void)
{
  vp_run r;
  uint16_t w[SC_PROG_MAX];
  size_t n = vp_prog_off(w);
  VC_CHECK(vs_run(&r, w, n, 3u, VS_E1));
  VC_CHECK(memcmp(r.buf, vl_header(0u), 32u) == 0);
  VC_CHECK(memcmp(r.buf + VS_SLOT, vl_header(1u), 32u) == 0);
  /* the literal decodes to the values the page names: tick 59,736 = 0xE958, index 0, width 159, height 96, flags 0, reserved 0 */
  VC_CHECK(vs_be(r.buf, 8u) == VS_E0 && vs_be(r.buf + 8u, 8u) == 0u);
  VC_CHECK(vs_be(r.buf + 16u, 4u) == 159u && vs_be(r.buf + 20u, 4u) == 96u);
  VC_CHECK(vs_be(r.buf + 24u, 4u) == 0u && vs_be(r.buf + 28u, 4u) == 0u);
  VC_CHECK(vs_be(r.buf + VS_SLOT, 8u) == VS_E1 && vs_be(r.buf + VS_SLOT + 8u, 8u) == 1u);
  vp_free(&r);
  return IVS_TEST_PASS;
}

static int vs_plane_stride(void)
{
  vp_run r;
  uint16_t w[SC_PROG_MAX];
  size_t n = vp_prog_off(w);
  uint8_t d[32];
  uint32_t k;
  VC_CHECK(vs_run(&r, w, n, 3u, VS_E1));
  for (k = 0u; k < 2u; k++) {
    /* the whole slot (the header and 96 rows of 159 bytes, tightly packed) hashes to the literal's digest of slot k */
    ivs_sha256_digest(r.buf + (size_t)k * VS_SLOT, VS_SLOT, d);
    VC_CHECK(memcmp(d, vl_slot_sha(k), 32u) == 0);
  }
  vp_free(&r);
  return IVS_TEST_PASS;
}

/* A border color stored in the register window fills the display-off plane (C99, video.display_off_fill border). */
static int vs_blank_frame_border_fill(void)
{
  vp_run r;
  uint16_t w[SC_PROG_MAX];
  uint32_t v;
  for (v = 0u; v < 16u; v += 5u) { /* 0, 5, 10, 15 */
    size_t n = vp_prog_border(w, v);
    VC_CHECK(vs_run(&r, w, n, 3u, VS_E1));
    VC_CHECK(vs_plane_all(r.buf, (uint8_t)v) && vs_plane_all(r.buf + VS_SLOT, (uint8_t)v));
    VC_CHECK(vs_be(r.buf + 24u, 4u) == 0u);
    vp_free(&r);
  }
  return IVS_TEST_PASS;
}

/* The displayed flag follows the display latch of the frame: a strobe in the register window of frame 0 sets it for frame 0 only (the
 * latch is cleared at every frame start, D-06), so frame 1 is a display-off frame again. */
static int vs_displayed_flag_follows_latch(void)
{
  vp_run r;
  uint16_t w[SC_PROG_MAX];
  size_t n = vp_prog_strobe_once(w);
  VC_CHECK(vs_run(&r, w, n, 3u, VS_E1));
  VC_CHECK(vs_be(r.buf + 24u, 4u) == IVS_FRAME_F_DISPLAYED);
  VC_CHECK(vs_be(r.buf + VS_SLOT + 24u, 4u) == 0u);
  vp_free(&r);
  return IVS_TEST_PASS;
}

/* The number of frames completed equals the number of FRAME_END events inside the run interval (those at or below the machine time). */
static int vs_count_equals_events(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint64_t stops[3];
  uint32_t k;
  VC_CHECK(p->frame_ticks == VS_E0);
  stops[0] = 1000u;
  stops[1] = 3u * (uint64_t)p->frame_ticks + 5000u;
  stops[2] = 5u * (uint64_t)p->frame_ticks + 77u;
  for (k = 0u; k < 3u; k++) {
    vp_run r;
    uint16_t w[SC_PROG_MAX];
    size_t n = vp_prog_off(w);
    uint64_t now;
    uint64_t events;
    VC_CHECK(vs_run(&r, w, n, 8u, stops[k]));
    now = vp_im(&r)->time_ticks;
    events = now >= (uint64_t)p->frame_phase_ticks + p->frame_ticks ? (now - p->frame_phase_ticks) / p->frame_ticks : 0u;
    VC_CHECK(r.sink.video_frames == events && vp_im(&r)->obs.video.count == events && vp_im(&r)->stic.frame_index == events);
    VC_CHECK(r.sink.video_written == events * VS_SLOT);
    vp_free(&r);
  }
  return IVS_TEST_PASS;
}

/* Interim (plan 06-10 inverts it as a reviewed change): until the card decoder exists a display-on frame is every position not
 * displayed, IVS_PIXEL_BLANK, so it can never be mistaken for a rendered picture. */
static int vs_display_on_not_decoded(void)
{
  vp_run r;
  uint16_t w[SC_PROG_MAX];
  size_t n = vp_prog_strobe_once(w);
  VC_CHECK(vs_run(&r, w, n, 3u, VS_E1));
  VC_CHECK(vs_plane_all(r.buf, (uint8_t)IVS_PIXEL_BLANK));
  VC_CHECK(vs_plane_all(r.buf + VS_SLOT, 0u)); /* frame 1 is display off: the border color 0 */
  vp_free(&r);
  return IVS_TEST_PASS;
}

/* The machine state never depends on a buffer: one guest, one run with a buffer and one with none (the counting sink), the whole
 * instance storage, the chain and the count are equal; only the delivered byte count differs. */
static int vs_noop_null_buffer(void)
{
  vp_run a;
  vp_run b;
  uint16_t w[SC_PROG_MAX];
  size_t n = vp_prog_strobe_once(w);
  VC_CHECK(vs_run(&a, w, n, 4u, 3u * (uint64_t)VS_E0 + 1000u));
  if (!vp_new(&b, w, n, 0u) || !vp_until(&b, vp_im(&a)->time_ticks)) {
    vp_free(&a);
    return IVS_TEST_FAIL;
  }
  VC_CHECK(vp_im(&a)->time_ticks == vp_im(&b)->time_ticks);
  VC_CHECK(vp_im(&a)->obs.video.count == 3u && vp_im(&b)->obs.video.count == 3u);
  VC_CHECK(memcmp(vp_im(&a)->obs.video.chain, vp_im(&b)->obs.video.chain, 32u) == 0);
  VC_CHECK(memcmp(vp_im(&a), vp_im(&b), IVS_MACHINE_BYTES) == 0);
  VC_CHECK(a.sink.video_frames == 3u && b.sink.video_frames == 3u);
  VC_CHECK(a.sink.video_written == 3u * VS_SLOT && b.sink.video_written == 0u);
  vp_free(&a);
  vp_free(&b);
  return IVS_TEST_PASS;
}

/* The chain steps equal the literal: h0 at power-on, h1 after frame 0, h2 after frame 1, each equal to the value the shell recipe
 * derived from hand-framed bytes. */
static int vs_chain_framing_oracle(void)
{
  vp_run r;
  uint16_t w[SC_PROG_MAX];
  size_t n = vp_prog_off(w);
  VC_CHECK(vp_new(&r, w, n, 3u * VS_SLOT));
  VC_CHECK(memcmp(vp_im(&r)->obs.video.chain, vl_h(0u), 32u) == 0 && vp_im(&r)->obs.video.count == 0u);
  VC_CHECK(vp_until(&r, VS_E0));
  VC_CHECK(vp_im(&r)->obs.video.count == 1u && memcmp(vp_im(&r)->obs.video.chain, vl_h(1u), 32u) == 0);
  VC_CHECK(vp_until(&r, VS_E1));
  VC_CHECK(vp_im(&r)->obs.video.count == 2u && memcmp(vp_im(&r)->obs.video.chain, vl_h(2u), 32u) == 0);
  vp_free(&r);
  return IVS_TEST_PASS;
}

/* The chain recomputed from the delivered bytes alone: h(n+1) = SHA-256(h(n) || slot n) over a concatenation built here. */
static int vs_chain_recompute_from_slots(void)
{
  vp_run r;
  uint16_t w[SC_PROG_MAX];
  size_t n = vp_prog_strobe_once(w);
  uint8_t h[32];
  uint8_t *cat;
  uint32_t k;
  VC_CHECK(vs_run(&r, w, n, 4u, 3u * (uint64_t)VS_E0 + 100u));
  VC_CHECK(r.sink.video_frames == 3u);
  cat = (uint8_t *)malloc(32u + VS_SLOT);
  if (cat == NULL) {
    vp_free(&r);
    return IVS_TEST_FAIL;
  }
  ivs_sha256_digest((const uint8_t *)"IVS-VIDEO-LOG-V1", 16u, h);
  for (k = 0u; k < 3u; k++) {
    memcpy(cat, h, 32u);
    memcpy(cat + 32u, r.buf + (size_t)k * VS_SLOT, VS_SLOT);
    ivs_sha256_digest(cat, 32u + VS_SLOT, h);
  }
  free(cat);
  VC_CHECK(memcmp(h, vp_im(&r)->obs.video.chain, 32u) == 0);
  vp_free(&r);
  return IVS_TEST_PASS;
}

/* The header index counts the frames completed before the slot, and a run split in two calls continues the sequence. */
static int vs_chain_index_continuity(void)
{
  vp_run r;
  uint16_t w[SC_PROG_MAX];
  size_t n = vp_prog_off(w);
  uint32_t k;
  VC_CHECK(vp_new(&r, w, n, 5u * VS_SLOT));
  VC_CHECK(vp_until(&r, VS_E0 + 20u));
  VC_CHECK(vp_until(&r, 3u * (uint64_t)VS_E0 + 500u));
  VC_CHECK(vp_until(&r, 4u * (uint64_t)VS_E0 + 1u));
  VC_CHECK(r.sink.video_frames == 4u);
  for (k = 0u; k < 4u; k++) {
    VC_CHECK(vs_be(r.buf + (size_t)k * VS_SLOT + 8u, 8u) == k);
    VC_CHECK(vs_be(r.buf + (size_t)k * VS_SLOT, 8u) == (uint64_t)(k + 1u) * VS_E0);
  }
  vp_free(&r);
  return IVS_TEST_PASS;
}

/* A power-on reset returns the chain to h0 and the count to 0, and the frames that follow repeat the first run. */
static int vs_chain_reset_clears(void)
{
  vp_run r;
  uint16_t w[SC_PROG_MAX];
  size_t n = vp_prog_off(w);
  uint8_t first[32];
  VC_CHECK(vs_run(&r, w, n, 3u, VS_E1));
  memcpy(first, vp_im(&r)->obs.video.chain, 32u);
  VC_CHECK(ivs_machine_reset(r.x.m, IVS_RESET_POWER_ON) == IVS_OK);
  VC_CHECK(vp_im(&r)->obs.video.count == 0u && memcmp(vp_im(&r)->obs.video.chain, vl_h(0u), 32u) == 0);
  ivs_psg_sink_init(&r.sink, NULL, 0u);
  ivs_psg_sink_video(&r.sink, r.buf, r.cap);
  VC_CHECK(vp_until(&r, VS_E1));
  VC_CHECK(memcmp(vp_im(&r)->obs.video.chain, first, 32u) == 0 && memcmp(first, vl_h(2u), 32u) == 0);
  vp_free(&r);
  return IVS_TEST_PASS;
}

/* h0 is the SHA-256 of the 16 ASCII bytes at create (before any image is loaded) and at a power-on reset: the same two call sites. */
static int vs_chain_init_h0(void)
{
  ivs_ms_inst x;
  uint8_t h0[32];
  ivs_sha256_digest((const uint8_t *)"IVS-VIDEO-LOG-V1", 16u, h0);
  VC_CHECK(memcmp(h0, vl_h(0u), 32u) == 0); /* the test's own digest equals the shell-derived literal */
  VC_CHECK(ivs_ms_new(&x, VP_POISON));
  VC_CHECK(memcmp(ivs_machine_from_handle(x.m)->obs.video.chain, h0, 32u) == 0 && ivs_machine_from_handle(x.m)->obs.video.count == 0u);
  ivs_ms_free(&x);
  VC_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, VP_POISON));
  VC_CHECK(memcmp(ivs_machine_from_handle(x.m)->obs.video.chain, h0, 32u) == 0 && ivs_machine_from_handle(x.m)->obs.video.count == 0u);
  ivs_ms_free(&x);
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_video_slot_cases[] = {
    {"video.layout.bytes", vs_layout_bytes, NULL},
    {"video.slot.header-bytes", vs_header_bytes, NULL},
    {"video.slot.plane-stride", vs_plane_stride, NULL},
    {"video.slot.blank-frame-border-fill", vs_blank_frame_border_fill, NULL},
    {"video.slot.displayed-flag-follows-latch", vs_displayed_flag_follows_latch, NULL},
    {"video.frame.count-equals-events", vs_count_equals_events, NULL},
    {"video.frame.display-on-not-decoded", vs_display_on_not_decoded, NULL},
    {"video.noop.null-buffer", vs_noop_null_buffer, NULL},
    {"video.chain.framing-oracle", vs_chain_framing_oracle, NULL},
    {"video.chain.recompute-from-slots", vs_chain_recompute_from_slots, NULL},
    {"video.chain.index-continuity", vs_chain_index_continuity, NULL},
    {"video.chain.reset-clears", vs_chain_reset_clears, NULL},
    {"video.chain.init-h0-create-and-reset", vs_chain_init_h0, NULL},
};
const size_t ivs_video_slot_count = sizeof ivs_video_slot_cases / sizeof ivs_video_slot_cases[0];
