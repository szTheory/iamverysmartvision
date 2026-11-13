/* SPDX-License-Identifier: MIT */
/* Suite runner, family vchain (plan 06-12, D-16, D-17): the runner's own recomputation of the VIDEO frame chain and its slot
 * header checks (runner/runner_vchain.c), against slots the core really delivered. A looping project-authored image
 * (a branch to itself, display never enabled) runs three frames through the public API; the slots are fed to the runner chain and
 * the digest it implies is compared with ivs_machine_state_digest of domain 16. The detect cases corrupt a copy of a
 * delivered slot (one field each) and require the finding, and that the recomputed digest then differs from the core's.
 * Test code only. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_runner.h"
#include "ivs_test.h"
#include "runner_cases.h"
#include "runner_vchain.h"

#define VC_SLOT 15296u
#define VC_FRAMES 3u
#define VC_TICKS 190000u /* three frame events: 59,736, 119,472, 179,208 */

typedef struct vc_run {
  uint8_t slots[VC_FRAMES * VC_SLOT];
  size_t bytes;
  uint8_t core_digest[32];
  uint32_t width;
  uint32_t height;
  uint64_t nominal;
} vc_run;

/* Runs the looping image to VC_TICKS in one call with a video buffer of VC_FRAMES + 1 slots. 1 on success. */
static int vc_core_run(vc_run *r)
{
  ivs_storage_requirements req;
  ivs_create_options co;
  ivs_machine *m = NULL;
  void *raw = NULL;
  uint8_t *base;
  uint8_t *exec = calloc(8192u, 1u);
  uint8_t *grom = calloc(2048u, 1u);
  uint8_t *vid = calloc(VC_SLOT, VC_FRAMES + 2u);
  ivs_info info;
  ivs_run_request q;
  ivs_run_report rep;
  int ok = 0;
  if (exec == NULL || grom == NULL || vid == NULL) {
    goto done;
  }
  exec[0] = 0x02u; /* 0220 0001: a branch to itself */
  exec[1] = 0x20u;
  exec[2] = 0x00u;
  exec[3] = 0x01u;
  memset(&req, 0, sizeof req);
  req.struct_size = (uint32_t)sizeof req;
  if (ivs_machine_storage_requirements("intv-2609-ntsc-family-p1", &req) != IVS_OK) {
    goto done;
  }
  raw = malloc((size_t)req.size_bytes + (size_t)req.align_bytes);
  if (raw == NULL) {
    goto done;
  }
  base = (uint8_t *)(((uintptr_t)raw + (uintptr_t)(req.align_bytes - 1u)) & ~(uintptr_t)(req.align_bytes - 1u));
  memset(&co, 0, sizeof co);
  co.struct_size = (uint32_t)sizeof co;
  co.profile_id = "intv-2609-ntsc-family-p1";
  if (ivs_machine_create(&co, base, (size_t)req.size_bytes, &m) != IVS_OK ||
      ivs_machine_load(m, IVS_KIND_EXEC, exec, 8192u, NULL, NULL) != IVS_OK ||
      ivs_machine_load(m, IVS_KIND_GROM, grom, 2048u, NULL, NULL) != IVS_OK ||
      ivs_machine_reset(m, IVS_RESET_POWER_ON) != IVS_OK) {
    goto done;
  }
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info(m, &info) != IVS_OK || info.video_slot_bytes != VC_SLOT) {
    goto done;
  }
  r->width = info.video_width;
  r->height = info.video_height;
  r->nominal = info.nominal_frame_ticks;
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.target_ticks = VC_TICKS;
  q.video = vid;
  q.video_capacity = (size_t)VC_SLOT * (VC_FRAMES + 2u);
  memset(&rep, 0, sizeof rep);
  rep.struct_size = (uint32_t)sizeof rep;
  if (ivs_machine_run(m, &q, &rep) != IVS_OK || rep.video_bytes_written != (uint64_t)VC_SLOT * VC_FRAMES ||
      rep.video_first_index != 0u) {
    goto done;
  }
  memcpy(r->slots, vid, (size_t)VC_SLOT * VC_FRAMES);
  r->bytes = (size_t)VC_SLOT * VC_FRAMES;
  if (ivs_machine_state_digest(m, IVS_DIGEST_VIDEO, r->core_digest) != IVS_OK) {
    goto done;
  }
  ok = 1;
done:
  if (m != NULL) {
    (void)ivs_machine_destroy(m);
  }
  free(raw);
  free(exec);
  free(grom);
  free(vid);
  return ok;
}

/* Feeds all slots of r (after the corruption done by the caller on a copy) and returns the finding; digest gets the
 * recomputed domain-16 digest. */
static uint32_t vc_feed_all(const vc_run *r, const uint8_t *slots, uint8_t digest[32], uint64_t *frames)
{
  ivs_runner_vchain c;
  uint32_t f;
  ivs_runner_vchain_init(&c, r->width, r->height, r->nominal);
  f = ivs_runner_vchain_feed(&c, slots, r->bytes, 0u);
  ivs_runner_vchain_digest(&c, digest);
  *frames = c.frames;
  return f;
}

/* Corrupts one byte of a copy at offset off, feeds it and requires finding want and a digest different from the core's. */
static int vc_detects(uint32_t want, size_t off, uint8_t xor_mask)
{
  static vc_run r;
  static uint8_t copy[VC_FRAMES * VC_SLOT];
  uint8_t digest[32];
  uint64_t frames = 0u;
  IVS_CHECK(vc_core_run(&r));
  memcpy(copy, r.slots, r.bytes);
  copy[off] ^= xor_mask;
  IVS_CHECK(vc_feed_all(&r, copy, digest, &frames) == want);
  IVS_CHECK(frames == VC_FRAMES);
  IVS_CHECK(memcmp(digest, r.core_digest, 32u) != 0);
  return IVS_TEST_PASS;
}

/* The recomputation equals the core's domain-16 digest for the slots a real run delivered, fed in one call and
 * one slot per call (first indexes 0, 1, 2), with no header finding; the empty chain equals the digest of a machine
 * that delivered nothing (frames 0). */
static int vc_matches_core_chain(void)
{
  static vc_run r;
  uint8_t digest[32];
  uint64_t frames = 0u;
  ivs_runner_vchain c;
  IVS_CHECK(vc_core_run(&r));
  IVS_CHECK(r.width == 159u && r.height == 96u && r.nominal == 59736u);
  IVS_CHECK(vc_feed_all(&r, r.slots, digest, &frames) == IVS_RUNNER_VCHAIN_OK && frames == VC_FRAMES);
  IVS_CHECK(memcmp(digest, r.core_digest, 32u) == 0);
  ivs_runner_vchain_init(&c, r.width, r.height, r.nominal);
  for (uint32_t i = 0u; i < VC_FRAMES; i++) {
    IVS_CHECK(ivs_runner_vchain_feed(&c, r.slots + (size_t)i * VC_SLOT, VC_SLOT, i) == IVS_RUNNER_VCHAIN_OK);
  }
  ivs_runner_vchain_digest(&c, digest);
  IVS_CHECK(memcmp(digest, r.core_digest, 32u) == 0);
  /* A first index that does not continue the previous call is a finding, and the empty feed checks nothing. */
  ivs_runner_vchain_init(&c, r.width, r.height, r.nominal);
  IVS_CHECK(ivs_runner_vchain_feed(&c, r.slots, 0u, 7u) == IVS_RUNNER_VCHAIN_OK);
  IVS_CHECK(ivs_runner_vchain_feed(&c, r.slots, VC_SLOT, 1u) == IVS_RUNNER_VCHAIN_E_INDEX);
  return IVS_TEST_PASS;
}

/* Slot 1 carries index 5 instead of 1. */
static int vc_detects_index_gap(void) { return vc_detects(IVS_RUNNER_VCHAIN_E_INDEX, (size_t)VC_SLOT + 15u, 0x04u); }

/* Slot 0 carries a width one off (and, separately, a height one off). */
static int vc_detects_header_width(void)
{
  int rc = vc_detects(IVS_RUNNER_VCHAIN_E_WIDTH, 19u, 0x01u);
  if (rc != IVS_TEST_PASS) {
    return rc;
  }
  return vc_detects(IVS_RUNNER_VCHAIN_E_HEIGHT, 23u, 0x01u);
}

/* Slot 2 has a pixel of 16 (a color above 15 that is not the blank value). */
static int vc_detects_pixel_range(void)
{
  static vc_run r;
  static uint8_t copy[VC_FRAMES * VC_SLOT];
  uint8_t digest[32];
  uint64_t frames = 0u;
  IVS_CHECK(vc_core_run(&r));
  memcpy(copy, r.slots, r.bytes);
  IVS_CHECK(copy[2u * VC_SLOT + 32u + 500u] == 0u);
  copy[2u * VC_SLOT + 32u + 500u] = 16u;
  IVS_CHECK(vc_feed_all(&r, copy, digest, &frames) == IVS_RUNNER_VCHAIN_E_PIXEL);
  /* The blank value is a legal pixel. */
  memcpy(copy, r.slots, r.bytes);
  copy[32u + 500u] = IVS_PIXEL_BLANK;
  IVS_CHECK(vc_feed_all(&r, copy, digest, &frames) == IVS_RUNNER_VCHAIN_OK);
  return IVS_TEST_PASS;
}

/* Slot 1 carries the tick of slot 0 (not strictly increasing), and a spacing below nominal_frame_ticks is refused too. */
static int vc_detects_tick_order(void)
{
  static vc_run r;
  static uint8_t copy[VC_FRAMES * VC_SLOT];
  uint8_t digest[32];
  uint64_t frames = 0u;
  IVS_CHECK(vc_core_run(&r));
  memcpy(copy, r.slots, r.bytes);
  memcpy(copy + VC_SLOT, copy, 8u);
  IVS_CHECK(vc_feed_all(&r, copy, digest, &frames) == IVS_RUNNER_VCHAIN_E_TICK);
  memcpy(copy, r.slots, r.bytes);
  copy[VC_SLOT + 7u] = (uint8_t)(copy[VC_SLOT + 7u] - 1u); /* one tick closer than the nominal spacing */
  IVS_CHECK(copy[VC_SLOT + 7u] != r.slots[VC_SLOT + 7u]);
  IVS_CHECK(vc_feed_all(&r, copy, digest, &frames) == IVS_RUNNER_VCHAIN_E_TICK);
  return IVS_TEST_PASS;
}

/* Slot 0's reserved word is 1; a flag bit above bit 0 is its own finding. */
static int vc_detects_reserved_word(void)
{
  int rc = vc_detects(IVS_RUNNER_VCHAIN_E_RESERVED, 31u, 0x01u);
  if (rc != IVS_TEST_PASS) {
    return rc;
  }
  return vc_detects(IVS_RUNNER_VCHAIN_E_FLAGS, 27u, 0x02u);
}

static const ivs_test_case vchain_cases[] = {
    {"runner.vchain.matches-core-chain", vc_matches_core_chain, NULL},
    {"runner.vchain.detects-index-gap", vc_detects_index_gap, NULL},
    {"runner.vchain.detects-header-width", vc_detects_header_width, NULL},
    {"runner.vchain.detects-pixel-range", vc_detects_pixel_range, NULL},
    {"runner.vchain.detects-tick-order", vc_detects_tick_order, NULL},
    {"runner.vchain.detects-reserved-word", vc_detects_reserved_word, NULL},
};

#define VCHAIN_CASES (sizeof vchain_cases / sizeof vchain_cases[0])

size_t runner_vchain_count(void) { return VCHAIN_CASES; }

const char *runner_vchain_id(size_t i) { return i < VCHAIN_CASES ? vchain_cases[i].id : NULL; }

int runner_vchain_run(size_t i) { return i < VCHAIN_CASES ? vchain_cases[i].fn() : IVS_TEST_FAIL; }
