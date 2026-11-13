/* SPDX-License-Identifier: MIT */
/* Suite video, families cstack and mode (plan 06-11, VID-04, VID-06, D-06, D-18, D-19): the Color Stack renderer against the hand-derived frame
 * literals F-B1..F-B3 of plan 06-01 (data/video/frames, written from the card rules before any renderer existed, never produced by a run), in
 * the unit path (ivs_stic_render_cards) and through the whole machine (a stand-in guest reads $0021 inside the register window, which selects
 * Color Stack, strobes $0020, and the DELIVERED slot plane is compared). The frame helpers are those of video_fgbg.c; the graphics ROM is
 * the zero image and graphics RAM holds project-authored geometric shapes. */
#include <stdint.h>
#include <string.h>

#include "video_cases.h"

#define VCS_FB1 5u /* the index of F-B1 in the generated frame table (F-A1..F-A5 come first) */
#define VCS_FB2 6u
#define VCS_FB3 7u

static int vcs_unit_b1(void)
{
  return vf_unit(VCS_FB1, 1u);
}

static int vcs_machine_b1(void)
{
  return vf_machine(VCS_FB1, 1u);
}

const ivs_test_case ivs_video_cstack_cases[] = {
    {"video.cstack.F-B1", vcs_unit_b1, NULL},
    {"video.cstack.machine-F-B1", vcs_machine_b1, NULL},
};
const size_t ivs_video_cstack_count = sizeof ivs_video_cstack_cases / sizeof ivs_video_cstack_cases[0];
