/* SPDX-License-Identifier: MIT */
/* Suite video, family fgbg (plan 06-10, VID-04, D-02, D-11, D-18, D-19): the Foreground/Background card renderer against the hand-derived
 * frame literals F-A1..F-A5 of plan 06-01 (data/video/frames, written from the card rules before any renderer existed, never produced by
 * a run), in two paths. The unit path builds the 240 BACKTAB words and pokes graphics RAM through the raw seam, renders with
 * ivs_stic_render_cards and compares the 160 x 96 working plane byte for byte. The machine path pokes BACKTAB and graphics RAM into a
 * created instance, runs a strobe stand-in across the first frame event and compares the DELIVERED slot plane (159 columns) with the
 * cropped expectation and the SHA-256 of its canonical text form with the literal's sha256-text. The graphics ROM is the zero image in
 * every run (D-19: no GROM in CI): the machine path clears the stand-in GROM pattern of the shared guest builder first.
 *
 * Every hand-typed word is first checked against its annotation by the test-side encoder (vid_fields.c); a mismatch there is reported as
 * FIXTURE, not as a renderer failure. The text form of the expected plane must also hash to the literal's own sha256-text (a fixture
 * self-consistency check). A mismatch prints the first differing card as two 8 x 8 blocks with its word, annotation, vector name and
 * literal line number. The generated include (cmake/IvsVideoFrames.cmake) reads only the literals. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common/ivs_sha256.h"
#include "video_cases.h"
#include "ivs_video_frames.inc"

#define VF_SLOT 15296u
#define VF_E0 59736u

uint32_t vf_frame_count(void)
{
  return (uint32_t)(sizeof vf_frames / sizeof vf_frames[0]);
}

const vf_frame *vf_frame_get(uint32_t i)
{
  return i < vf_frame_count() ? &vf_frames[i] : NULL;
}

const vf_role *vf_role_table(size_t *n)
{
  *n = sizeof vf_roles / sizeof vf_roles[0];
  return vf_roles;
}

const ivs_test_source *vf_source_table(size_t *n)
{
  *n = sizeof vf_sources / sizeof vf_sources[0];
  return vf_sources;
}

/* ---- the unit-path memory system and renderer call ---- */
static ivs_memsys vf_m;
static uint8_t vf_got[VF_PLANE_BYTES];
static uint8_t vf_exp[VF_PLANE_BYTES];

ivs_memsys *vf_mem(void)
{
  return &vf_m;
}

void vf_mem_clear(void)
{
  (void)ivs_memsys_init(&vf_m, SC_PROFILE);
}

void vf_poke_gram_card(ivs_memsys *m, uint32_t card, const uint8_t b[8])
{
  uint32_t y;
  for (y = 0u; y < 8u; y++) {
    ivs_memsys_poke_raw_internal(m, (uint16_t)(VF_GRAM_BASE + 8u * card + y), b[y]);
  }
}

void vf_poke_gram(ivs_memsys *m, const vf_frame *f)
{
  uint32_t i;
  for (i = 0u; i < f->n_gram; i++) {
    vf_poke_gram_card(m, f->gram[i].card, f->gram[i].b);
  }
}

void vf_render_words(const uint16_t *words, uint32_t mode, uint8_t *plane)
{
  ivs_stic_cardin in;
  memset(&in, 0, sizeof in);
  in.mode = mode;
  (void)ivs_stic_render_cards(&in, words, &vf_m, plane);
}

void vf_words(const vf_frame *f, uint16_t *words)
{
  uint32_t i;
  for (i = 0u; i < 240u; i++) {
    words[i] = (uint16_t)f->fill;
  }
  for (i = 0u; i < f->n_cells; i++) {
    words[20u * f->cells[i].r + f->cells[i].c] = (uint16_t)f->cells[i].word;
  }
}

void vf_render_frame(const vf_frame *f, uint8_t *plane)
{
  uint16_t words[240];
  vf_mem_clear();
  vf_poke_gram(&vf_m, f);
  vf_words(f, words);
  vf_render_words(words, f->mode, plane);
}

void vf_expected(const vf_frame *f, uint8_t *exp)
{
  uint32_t y;
  uint32_t x;
  for (y = 0u; y < VF_PLANE_H; y++) {
    for (x = 0u; x < VF_PLANE_W; x++) {
      char ch = f->plane[y][x];
      uint8_t v;
      if (ch == '-') {
        v = 0xFFu;
      } else if (ch >= '0' && ch <= '9') {
        v = (uint8_t)(ch - '0');
      } else {
        v = (uint8_t)(10 + (ch - 'A'));
      }
      exp[y * VF_PLANE_W + x] = v;
    }
  }
}

long vf_first_diff(const uint8_t *got, uint32_t gs, const uint8_t *exp, uint32_t es, uint32_t w, uint32_t h)
{
  uint32_t y;
  uint32_t x;
  for (y = 0u; y < h; y++) {
    for (x = 0u; x < w; x++) {
      if (got[y * gs + x] != exp[y * es + x]) {
        return (long)(y * w + x);
      }
    }
  }
  return -1;
}

static char vf_hexc(uint8_t v)
{
  return v < 16u ? "0123456789ABCDEF"[v] : '-';
}

void vf_report(const vf_frame *f, const uint8_t *got, uint32_t gs, const uint8_t *exp, uint32_t es, long idx)
{
  uint32_t w = gs < es ? gs : es; /* the compared width of the row-major index */
  uint32_t y = (uint32_t)idx / w;
  uint32_t x = (uint32_t)idx % w;
  uint32_t r = y / 8u;
  uint32_t c = x / 8u;
  uint32_t word = f->fill;
  const char *ann = "fill word";
  uint32_t i;
  uint32_t k;
  for (i = 0u; i < f->n_cells; i++) {
    if (f->cells[i].r == r && f->cells[i].c == c) {
      word = f->cells[i].word;
      ann = f->cells[i].ann;
    }
  }
  ivs_test_diag("vector %s (literal plane at line %u): first difference at pixel (%u,%u), card (%u,%u), word %04X, %s", f->name,
                (unsigned)f->plane_line, (unsigned)x, (unsigned)y, (unsigned)r, (unsigned)c, (unsigned)word, ann);
  for (k = 0u; k < 8u; k++) {
    char g[9];
    char e[9];
    uint32_t j;
    for (j = 0u; j < 8u; j++) {
      uint32_t px = c * 8u + j;
      uint32_t py = r * 8u + k;
      g[j] = px < gs ? vf_hexc(got[py * gs + px]) : '.';
      e[j] = px < es ? vf_hexc(exp[py * es + px]) : '.';
    }
    g[8] = '\0';
    e[8] = '\0';
    ivs_test_diag("  got %s   expected %s", g, e);
  }
}

void vf_text_sha(const uint8_t *plane, uint32_t stride, uint32_t w, char *hex)
{
  static uint8_t text[VF_PLANE_H * (VF_PLANE_W + 1u)];
  uint8_t d[32];
  uint32_t y;
  uint32_t x;
  size_t n = 0u;
  for (y = 0u; y < VF_PLANE_H; y++) {
    for (x = 0u; x < w; x++) {
      text[n++] = (uint8_t)vf_hexc(plane[y * stride + x]);
    }
    text[n++] = (uint8_t)'\n';
  }
  ivs_sha256_digest(text, n, d);
  for (x = 0u; x < 32u; x++) {
    hex[2u * x] = "0123456789abcdef"[d[x] >> 4];
    hex[2u * x + 1u] = "0123456789abcdef"[d[x] & 15u];
  }
  hex[64] = '\0';
}

/* The fixture checks every frame test starts with: each word against its annotation (FIXTURE), and the literal plane against its own
 * sha256-text key over the 159 published columns (FIXTURE). 1 when the fixture is consistent. */
static int vf_fixture_ok(const vf_frame *f)
{
  char hex[65];
  vf_expected(f, vf_exp);
  if (!vid_check_cells(f)) {
    return 0;
  }
  vf_text_sha(vf_exp, VF_PLANE_W, VF_PLANE_W - 1u, hex);
  if (strcmp(hex, f->sha_text) != 0) {
    ivs_test_diag("FIXTURE: %s: the literal plane hashes to %s, not its sha256-text %s", f->name, hex, f->sha_text);
    return 0;
  }
  return 1;
}

/* Unit path: the frame literal through ivs_stic_render_cards. */
static int vf_unit(uint32_t i)
{
  const vf_frame *f = vf_frame_get(i);
  long d;
  VC_CHECK(f != NULL && f->mode == 0u && f->display == 1u);
  VC_CHECK(vf_fixture_ok(f));
  vf_render_frame(f, vf_got);
  d = vf_first_diff(vf_got, VF_PLANE_W, vf_exp, VF_PLANE_W, VF_PLANE_W, VF_PLANE_H);
  if (d >= 0) {
    vf_report(f, vf_got, VF_PLANE_W, vf_exp, VF_PLANE_W, d);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

/* Machine path: raw pokes into a created instance, a strobe stand-in (MVO R0, $0020 then DIS and B self: 6 words, derived in
 * tests/stic/stic_cases.h and video_progs.c), the first frame event, and the DELIVERED slot plane. */
static int vf_machine(uint32_t i)
{
  const vf_frame *f = vf_frame_get(i);
  vp_run r;
  uint16_t w[SC_PROG_MAX];
  uint16_t words[240];
  size_t n = vp_prog_strobe_once(w);
  struct ivs_machine *im;
  char hex[65];
  uint32_t k;
  long d;
  VC_CHECK(f != NULL && f->mode == 0u && f->display == 1u);
  VC_CHECK(vf_fixture_ok(f));
  VC_CHECK(vp_new(&r, w, n, 2u * VF_SLOT));
  im = ivs_machine_from_handle_mut(r.x.m);
  memset(im->mem.grom, 0, sizeof im->mem.grom); /* GROM is the zero image in CI (D-19): the shared stand-in builder fills a pattern */
  vf_words(f, words);
  for (k = 0u; k < 240u; k++) {
    ivs_memsys_poke_raw_internal(&im->mem, (uint16_t)(VF_BACKTAB_BASE + k), words[k]);
  }
  vf_poke_gram(&im->mem, f);
  if (!vp_until(&r, VF_E0)) {
    vp_free(&r);
    return IVS_TEST_FAIL;
  }
  if (r.sink.video_frames != 1u || r.sink.video_written != VF_SLOT) {
    ivs_test_diag("%s: frames %u written %u, expected 1 and %u", f->name, (unsigned)r.sink.video_frames, (unsigned)r.sink.video_written,
                  (unsigned)VF_SLOT);
    vp_free(&r);
    return IVS_TEST_FAIL;
  }
  d = vf_first_diff(r.buf + 32u, 159u, vf_exp, VF_PLANE_W, 159u, VF_PLANE_H);
  if (d >= 0) {
    vf_report(f, r.buf + 32u, 159u, vf_exp, VF_PLANE_W, d);
    vp_free(&r);
    return IVS_TEST_FAIL;
  }
  vf_text_sha(r.buf + 32u, 159u, 159u, hex);
  if (strcmp(hex, f->sha_text) != 0) {
    ivs_test_diag("%s: the delivered plane hashes to %s, the literal says %s", f->name, hex, f->sha_text);
    vp_free(&r);
    return IVS_TEST_FAIL;
  }
  vp_free(&r);
  return IVS_TEST_PASS;
}

static int vf_unit_a1(void)
{
  return vf_unit(0u);
}

static int vf_machine_a1(void)
{
  return vf_machine(0u);
}

const ivs_test_case ivs_video_fgbg_cases[] = {
    {"video.fgbg.F-A1", vf_unit_a1, NULL},
    {"video.fgbg.machine-F-A1", vf_machine_a1, NULL},
};
const size_t ivs_video_fgbg_count = sizeof ivs_video_fgbg_cases / sizeof ivs_video_fgbg_cases[0];
