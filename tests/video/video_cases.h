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
extern const ivs_test_case ivs_video_stop_cases[];
extern const size_t ivs_video_stop_count;
extern const ivs_test_case ivs_video_slice_cases[];
extern const size_t ivs_video_slice_count;
extern const ivs_test_case ivs_video_fgbg_cases[];
extern const size_t ivs_video_fgbg_count;
extern const ivs_test_case ivs_video_card_cases[];
extern const size_t ivs_video_card_count;
extern const ivs_test_case ivs_video_probe_cases[];
extern const size_t ivs_video_probe_count;
extern const ivs_test_case ivs_video_meta_cases[];
extern const size_t ivs_video_meta_count;
extern const ivs_test_case ivs_video_judge_cases[];
extern const size_t ivs_video_judge_count;

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


/* ---- the hand-derived frames and the card bit roles (plan 06-10; cmake/IvsVideoFrames.cmake -> ivs_video_frames.inc, video_fgbg.c) ----
 * The expected planes are the committed literals of plan 06-01 (never a run). A vf_frame is one frame literal: the mode (0 Foreground/
 * Background, 1 Color Stack), display, border, the four stack values, the fill word of every unlisted cell, the listed cells (row,
 * column, word, source line, annotation string), the graphics RAM patterns, the 96 plane lines of 160 symbols (- is the cut-off pixel,
 * 0xFF) and the SHA-256 of the 159-column text form. A vf_role is one row of data/video/cards.literal.txt. */
typedef struct vf_cell {
  uint32_t r;
  uint32_t c;
  uint32_t word;
  uint32_t line;
  const char *ann;
} vf_cell;
typedef struct vf_gram {
  uint32_t card;
  uint8_t b[8];
} vf_gram;
typedef struct vf_frame {
  const char *name;
  uint32_t mode;
  uint32_t display;
  uint32_t border;
  uint32_t stack[4];
  uint32_t fill;
  uint32_t n_cells;
  const vf_cell *cells;
  uint32_t n_gram;
  const vf_gram *gram;
  const char *const *plane;
  const char *sha_text;
  uint32_t plane_line;
} vf_frame;
typedef struct vf_role {
  const char *variant;
  uint32_t bit;
  const char *role;
} vf_role;

#define VF_PLANE_W 160u
#define VF_PLANE_H 96u
#define VF_PLANE_BYTES (VF_PLANE_W * VF_PLANE_H)
#define VF_GRAM_BASE 0x3800u /* the graphics RAM word of (card, row y) is VF_GRAM_BASE + 8 card + y (the raw poke seam) */
#define VF_BACKTAB_BASE 0x0200u

uint32_t vf_frame_count(void);
const vf_frame *vf_frame_get(uint32_t i);
const vf_role *vf_role_table(size_t *n);
const ivs_test_source *vf_source_table(size_t *n);
/* The expected 160 x 96 plane of a frame literal (the symbol - is IVS_PIXEL_BLANK) and its 240 BACKTAB words (the fill word, then the cells). */
void vf_expected(const vf_frame *f, uint8_t *exp);
void vf_words(const vf_frame *f, uint16_t *words);
/* The unit-path memory system (a static instance, cleared): graphics RAM poked through the raw seam, then a frame rendered. */
ivs_memsys *vf_mem(void);
void vf_mem_clear(void);
void vf_poke_gram_card(ivs_memsys *m, uint32_t card, const uint8_t b[8]);
void vf_poke_gram(ivs_memsys *m, const vf_frame *f);
void vf_render_words(const uint16_t *words, uint32_t mode, uint8_t *plane);
void vf_render_frame(const vf_frame *f, uint8_t *plane);
/* The index of the first byte at which got (stride gs) differs from exp (stride es) over w columns and h rows, or -1; and the printout of
 * the card that holds it (two 8 x 8 blocks with the word, the annotation, the vector name and the literal line number). */
long vf_first_diff(const uint8_t *got, uint32_t gs, const uint8_t *exp, uint32_t es, uint32_t w, uint32_t h);
void vf_report(const vf_frame *f, const uint8_t *got, uint32_t gs, const uint8_t *exp, uint32_t es, long idx);
/* The SHA-256 (64 lower-case hex digits and a NUL into hex, 65 bytes) of the canonical text of a 96-line plane of w columns: uppercase hex
 * digits, w characters and an LF per line, built here (independent of the core's frame writer). */
void vf_text_sha(const uint8_t *plane, uint32_t stride, uint32_t w, char *hex);
/* The test-side word-versus-fields encoder (vid_fields.c, encode direction only; no include from core/src). */
uint32_t vid_encode_fgbg(uint32_t fg, uint32_t bg, uint32_t sel, uint32_t card);
/* Checks every listed cell word of f against its annotation; a mismatch is reported as FIXTURE (not a renderer failure); 1 when all match. */
int vid_check_cells(const vf_frame *f);

/* The fgbg bit roles by bit (video_probe.c) and the single-bit sweep over a role table: the number of probes whose rendered block differs
 * from the block the roles predict (0 is a pass; quiet 1 suppresses the diagnostics, for the judge). */
void vpr_roles(const char *role[16]);
uint32_t vpr_sweep(const char *const role[16], int quiet);

#endif
