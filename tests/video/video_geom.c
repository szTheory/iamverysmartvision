/* SPDX-License-Identifier: MIT */
/* Suite video, family geom (plan 06-09, D-12, D-13): the published geometry judged against the independent geometry literal
 * (data/video/geometry.literal.txt, read by cmake/IvsVideoLiterals.cmake only, never the profile or the core) and the pure derivation
 * ivs_stic_geom_derive. A pass shows that the arithmetic is self-consistent, not that a console shows 159 columns. */
#include <stdint.h>
#include <string.h>

#include "video_cases.h"
#include "ivs_video_lit.inc"

const uint8_t *vl_h(uint32_t k)
{
  return k == 0u ? vl_chain_h0 : (k == 1u ? vl_chain_h1 : vl_chain_h2);
}

const uint8_t *vl_header(uint32_t k)
{
  return k == 0u ? vl_chain_slot0_header : vl_chain_slot1_header;
}

const uint8_t *vl_slot_sha(uint32_t k)
{
  return k == 0u ? vl_chain_slot0_sha : vl_chain_slot1_sha;
}

const ivs_test_source *vl_source_table(size_t *n)
{
  *n = sizeof vl_sources / sizeof vl_sources[0];
  return vl_sources;
}

/* The profile's STIC parameters derive the published surface of the page. */
static int vg_profile_derives(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  ivs_stic_geom g;
  VC_CHECK(p != NULL);
  g = ivs_stic_geom_derive(p);
  VC_CHECK(g.width == VL_GEO_WIDTH);
  VC_CHECK(g.height == VL_GEO_HEIGHT);
  VC_CHECK(g.stride == VL_GEO_WIDTH);
  VC_CHECK(g.plane_bytes == VL_GEO_PLANE_BYTES);
  VC_CHECK(g.header_bytes == VL_GEO_HEADER_BYTES);
  VC_CHECK(g.slot_bytes == VL_GEO_SLOT_BYTES);
  VC_CHECK(g.width == 159u && g.height == 96u && g.slot_bytes == 15296u); /* the numbers the owner's decision D8 states */
  return IVS_TEST_PASS;
}

/* The page agrees with itself and with the public constants. */
static int vg_literal_matches_page(void)
{
  VC_CHECK(VL_GEO_WIDTH == (VL_GEO_CARD_COLUMNS - 1u) * VL_GEO_CARD_W + VL_GEO_LAST_COLUMN_PX);
  VC_CHECK(VL_GEO_HEIGHT == VL_GEO_CARD_ROWS * VL_GEO_CARD_H);
  VC_CHECK(VL_GEO_PLANE_BYTES == VL_GEO_WIDTH * VL_GEO_HEIGHT);
  VC_CHECK(VL_GEO_SLOT_BYTES == VL_GEO_HEADER_BYTES + VL_GEO_PLANE_BYTES);
  VC_CHECK(VL_GEO_HEADER_BYTES == IVS_FRAME_HEADER_BYTES);
  VC_CHECK(VL_GEO_BLANK == IVS_PIXEL_BLANK);
  VC_CHECK(VL_GEO_BLANK > IVS_PIXEL_COLOR_MAX);
  VC_CHECK(VL_GEO_ROW_LINES == 2u);
  /* the framed headers of the chain literal carry the same width and height (be32 at offsets 16 and 20) */
  VC_CHECK(vl_chain_slot0_header[19] == (uint8_t)VL_GEO_WIDTH && vl_chain_slot0_header[23] == (uint8_t)VL_GEO_HEIGHT);
  VC_CHECK(vl_chain_slot1_header[19] == (uint8_t)VL_GEO_WIDTH && vl_chain_slot1_header[23] == (uint8_t)VL_GEO_HEIGHT);
  return IVS_TEST_PASS;
}

/* C10 is a one-field reversal: video.last_card_column_px 8 gives 160 columns, stride 160, plane 15,360 and slot 15,392, and nothing
 * else of the surface changes (a stack copy of the parameters, one member edited). */
static int vg_reversal_one_field(void)
{
  ivs_stic_params p = *ivs_stic_params_get(SC_PROFILE);
  ivs_stic_geom g0 = ivs_stic_geom_derive(&p);
  ivs_stic_geom g1;
  p.video_last_column_px = 8u;
  g1 = ivs_stic_geom_derive(&p);
  VC_CHECK(g1.width == 160u && g1.stride == 160u);
  VC_CHECK(g1.plane_bytes == 15360u && g1.slot_bytes == 15392u);
  VC_CHECK(g1.height == g0.height && g1.header_bytes == g0.header_bytes);
  return IVS_TEST_PASS;
}

/* The generated derived members of the parameters equal the pure function's output (the registry profile). */
static int vg_generated_equals_derive(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  ivs_stic_geom g = ivs_stic_geom_derive(p);
  VC_CHECK(p->video_width == g.width);
  VC_CHECK(p->video_height == g.height);
  VC_CHECK(p->video_stride == g.stride);
  VC_CHECK(p->video_plane_bytes == g.plane_bytes);
  VC_CHECK(p->video_header_bytes == g.header_bytes);
  VC_CHECK(p->video_slot_bytes == g.slot_bytes);
  return IVS_TEST_PASS;
}

/* ivs_info v4 republishes the surface the page names and the generated parameters carry: format, bytes per frame, width, height, slot and
 * header bytes, the slack frames, the TV lines of one display row and the frame phase; the env of the public run call agrees. */
static int vg_info_matches_profile(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  ivs_ms_inst x;
  ivs_info a;
  ivs_run_env env;
  VC_CHECK(ivs_ms_new(&x, 0x5Au));
  memset(&a, 0xA5, sizeof a);
  a.struct_size = (uint32_t)sizeof a;
  if (ivs_machine_info(x.m, &a) != IVS_OK) {
    ivs_ms_free(&x);
    return IVS_TEST_FAIL;
  }
  memset(&env, 0, sizeof env);
  ivs_machine_profile_env(ivs_profile_get(SC_PROFILE), &env);
  ivs_ms_free(&x);
  VC_CHECK(a.video_format == IVS_OUTPUT_FORMAT_INDEX8_FRAMES_V1 && a.video_bytes_per_frame == VL_GEO_SLOT_BYTES);
  VC_CHECK(a.video_width == VL_GEO_WIDTH && a.video_height == VL_GEO_HEIGHT);
  VC_CHECK(a.video_slot_bytes == VL_GEO_SLOT_BYTES && a.video_header_bytes == VL_GEO_HEADER_BYTES);
  VC_CHECK(a.video_row_lines == VL_GEO_ROW_LINES);
  VC_CHECK(a.video_slack_frames == p->video_slack_frames && a.video_slack_frames == 1u);
  VC_CHECK(a.frame_phase_ticks == p->frame_phase_ticks && a.nominal_frame_ticks == p->frame_ticks);
  VC_CHECK(env.video_format == a.video_format && env.video_bytes_per_frame == a.video_bytes_per_frame);
  VC_CHECK(env.video_slack_frames == a.video_slack_frames && env.nominal_frame_ticks == a.nominal_frame_ticks);
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_video_geom_cases[] = {
    {"video.geom.profile-derives-159x96", vg_profile_derives, NULL},
    {"video.geom.literal-matches-page", vg_literal_matches_page, NULL},
    {"video.geom.reversal-one-field", vg_reversal_one_field, NULL},
    {"video.geom.info-matches-profile", vg_info_matches_profile, NULL},
    {"video.geom.generated-equals-derive", vg_generated_equals_derive, NULL},
};
const size_t ivs_video_geom_count = sizeof ivs_video_geom_cases / sizeof ivs_video_geom_cases[0];
