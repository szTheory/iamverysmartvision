/* SPDX-License-Identifier: MIT */
/* Suite video (VID-03, plan 06-09): the raw framebuffer slot, its per-frame chain, the video buffer rules, the frame stop and the
 * geometry. One executable; each family file defines its case table declared here and video_main.c concatenates them into one suite.
 * Expected values come from the two independent literals through cmake/IvsVideoLiterals.cmake (never the profile or the core) or are
 * relations to the generated ivs_stic_params; no value is copied from a core table (D-02). Stand-in guests are the sc_* builders of
 * tests/stic/stic_progs.c (decles derived in the comments of stic_cases.h). */
#ifndef IVS_VIDEO_CASES_H
#define IVS_VIDEO_CASES_H

#include <stddef.h>
#include <stdint.h>

#include "ivs_test.h"
#include "machine/machine.h"
#include "stic_cases.h"

#define VC_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

extern const ivs_test_case ivs_video_geom_cases[];
extern const size_t ivs_video_geom_count;
extern const ivs_test_case ivs_video_slot_cases[];
extern const size_t ivs_video_slot_count;
extern const ivs_test_case ivs_video_buffers_cases[];
extern const size_t ivs_video_buffers_count;

/* ---- the independent literals (video_geom.c includes the generated ivs_video_lit.inc) ---- */
const uint8_t *vl_h(uint32_t k);           /* the chain value h0, h1 or h2 (32 bytes) */
const uint8_t *vl_header(uint32_t k);      /* the framed 32-byte header of display-off slot 0 or 1 */
const uint8_t *vl_slot_sha(uint32_t k);    /* the SHA-256 of the whole display-off slot 0 or 1 */
const ivs_test_source *vl_source_table(size_t *n);

/* ---- a run harness over the internal step seam (video_progs.c) ----
 * A POWERED instance of the guest w[0..n) with a per-call sink that carries a video buffer of cap bytes (a poisoned heap buffer) or,
 * for cap 0, the counting video sink. The sink lives across vp_until calls, so its counters accumulate. */
typedef struct vp_run {
  ivs_ms_inst x;
  uint8_t *buf;
  uint64_t cap;
  ivs_psg_sink sink;
} vp_run;
#define VP_POISON 0xA5u
int vp_new(vp_run *r, const uint16_t *w, size_t n, uint64_t cap);
/* Steps units through the seam while machine time is below ticks (at most 400,000 units); 1 on success. */
int vp_until(vp_run *r, uint64_t ticks);
const struct ivs_machine *vp_im(const vp_run *r);
void vp_free(vp_run *r);

/* Guests: [DIS] B self, never strobing the display (3 words); MVII #v, R0 and MVO R0, $002C (the border color), then [DIS] B self. */
size_t vp_prog_off(uint16_t *w);
size_t vp_prog_border(uint16_t *w, uint32_t value);
/* MVO R0, $0020 once (the display-enable strobe in the register window of frame 0), then [DIS] B self. */
size_t vp_prog_strobe_once(uint16_t *w);

#endif
