/* SPDX-License-Identifier: MIT */
/* Suite video, family judge (plan 06-10, VID-04, D-02, D-19): the comparison functions of the frame and probe tests must be able to fail.
 * A corrupted copy of one expected row makes the plane comparison of a correct render fail (and the uncorrupted copy passes); a literal
 * whose roles of two bits were swapped makes the bit-role sweep fail (and the true roles pass). */
#include <stdint.h>
#include <string.h>

#include "video_cases.h"

static uint8_t vj_got[VF_PLANE_BYTES];
static uint8_t vj_exp[VF_PLANE_BYTES];

static int vj_frame_row_changed(void)
{
  const vf_frame *f = vf_frame_get(2u); /* F-A3 */
  uint32_t y;
  VC_CHECK(f != NULL);
  vf_render_frame(f, vj_got);
  vf_expected(f, vj_exp);
  VC_CHECK(vf_first_diff(vj_got, VF_PLANE_W, vj_exp, VF_PLANE_W, VF_PLANE_W, VF_PLANE_H) == -1); /* the control passes */
  for (y = 0u; y < VF_PLANE_H; y += 37u) { /* rows 0, 37 and 74: a changed pixel in the first, a middle and a late row */
    uint8_t saved = vj_exp[y * VF_PLANE_W + 5u];
    vj_exp[y * VF_PLANE_W + 5u] = (uint8_t)(saved ^ 1u);
    VC_CHECK(vf_first_diff(vj_got, VF_PLANE_W, vj_exp, VF_PLANE_W, VF_PLANE_W, VF_PLANE_H) == (long)(y * VF_PLANE_W + 5u));
    vj_exp[y * VF_PLANE_W + 5u] = saved;
  }
  return IVS_TEST_PASS;
}

static int vj_card_bit_flipped(void)
{
  const char *role[16];
  const char *swapped[16];
  const char *tmp;
  uint32_t i;
  vpr_roles(role);
  VC_CHECK(vpr_sweep(role, 1) == 0u); /* the control passes */
  for (i = 0u; i < 16u; i++) {
    swapped[i] = role[i];
  }
  tmp = swapped[0];
  swapped[0] = swapped[1]; /* the roles of bit 0 and bit 1 (foreground 0 and 1) exchanged in a copy of the literal */
  swapped[1] = tmp;
  VC_CHECK(vpr_sweep(swapped, 1) != 0u);
  return IVS_TEST_PASS;
}

static int vj_cs_roles_swapped(void)
{
  return vpr_cs_judge();
}

const ivs_test_case ivs_video_judge_cases[] = {
    {"video.judge.frame-row-changed", vj_frame_row_changed, NULL},
    {"video.judge.card-bit-flipped", vj_card_bit_flipped, NULL},
    {"video.judge.cs-roles-swapped", vj_cs_roles_swapped, NULL},
};
const size_t ivs_video_judge_count = sizeof ivs_video_judge_cases / sizeof ivs_video_judge_cases[0];
