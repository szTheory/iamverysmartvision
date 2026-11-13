/* SPDX-License-Identifier: MIT */
/* Suite video, family meta (plan 06-10, VID-04, D-18, D-19): metamorphic relations of the Foreground/Background renderer that need no
 * expected frame. Translation: moving a cell word one cell to the right moves its 8 x 8 block one cell to the right. Bits 15 and 14 are
 * ignored: seeded random frames render identically with those bits cleared, set or random. Purity: two renders of one input are equal and
 * the whole memory system is unchanged by rendering (no read side effect). Graphics RAM is random but fixed per seed; the graphics ROM
 * is the zero image. */
#include <stdint.h>
#include <string.h>

#include "video_cases.h"

static uint8_t vm_a[VF_PLANE_BYTES];
static uint8_t vm_b[VF_PLANE_BYTES];
static uint8_t vm_c[VF_PLANE_BYTES];
static uint16_t vm_words[240];
static uint16_t vm_alt[240];
static ivs_memsys vm_snap;

static uint32_t vm_next(uint32_t *s)
{
  *s ^= *s << 13;
  *s ^= *s >> 17;
  *s ^= *s << 5;
  return *s;
}

static void vm_random_gram(uint32_t *s)
{
  uint32_t card;
  for (card = 0u; card < 64u; card++) {
    uint8_t b[8];
    uint32_t y;
    for (y = 0u; y < 8u; y++) {
      b[y] = (uint8_t)(vm_next(s) & 0xFFu);
    }
    vf_poke_gram_card(vf_mem(), card, b);
  }
}

/* 1 when the 8 x 8 block (ra, ca) of plane a equals the block (rb, cb) of plane b. */
static int vm_block_eq(const uint8_t *a, uint32_t ra, uint32_t ca, const uint8_t *b, uint32_t rb, uint32_t cb)
{
  uint32_t y;
  for (y = 0u; y < 8u; y++) {
    if (memcmp(a + (8u * ra + y) * VF_PLANE_W + 8u * ca, b + (8u * rb + y) * VF_PLANE_W + 8u * cb, 8u) != 0) {
      return 0;
    }
  }
  return 1;
}

/* The listed cells of F-A3 (graphics RAM cards 1..4 in 16 color pairs) moved one cell right. */
static int vm_translate(void)
{
  const vf_frame *f = vf_frame_get(2u);
  uint32_t i;
  VC_CHECK(f != NULL && f->n_cells == 16u);
  for (i = 0u; i < f->n_cells; i++) {
    const vf_cell *cell = &f->cells[i];
    uint32_t j;
    vf_mem_clear();
    vf_poke_gram(vf_mem(), f);
    for (j = 0u; j < 240u; j++) {
      vm_words[j] = (uint16_t)f->fill;
      vm_alt[j] = (uint16_t)f->fill;
    }
    vm_words[20u * cell->r + cell->c] = (uint16_t)cell->word;
    vm_alt[20u * cell->r + cell->c + 1u] = (uint16_t)cell->word;
    vf_render_words(vm_words, 0u, vm_a);
    vf_render_words(vm_alt, 0u, vm_b);
    VC_CHECK(vm_block_eq(vm_a, cell->r, cell->c, vm_b, cell->r, cell->c + 1u)); /* the block moved */
    VC_CHECK(vm_block_eq(vm_a, cell->r, cell->c + 1u, vm_b, cell->r, cell->c)); /* and left the fill block behind */
  }
  return IVS_TEST_PASS;
}

/* 64 seeded random frames: clearing, setting or randomising bits 15 and 14 of every word changes nothing. */
static int vm_bits_15_14_ignored(void)
{
  uint32_t k;
  for (k = 0u; k < 64u; k++) {
    uint32_t s = 0x9E3779B9u ^ (0x01000193u * (k + 1u));
    uint32_t i;
    vf_mem_clear();
    vm_random_gram(&s);
    for (i = 0u; i < 240u; i++) {
      vm_words[i] = (uint16_t)(vm_next(&s) & 0xFFFFu);
    }
    for (i = 0u; i < 240u; i++) {
      vm_alt[i] = (uint16_t)(vm_words[i] & 0x3FFFu);
    }
    vf_render_words(vm_alt, 0u, vm_a);
    for (i = 0u; i < 240u; i++) {
      vm_alt[i] = (uint16_t)(vm_words[i] | 0xC000u);
    }
    vf_render_words(vm_alt, 0u, vm_b);
    vf_render_words(vm_words, 0u, vm_c);
    if (memcmp(vm_a, vm_b, VF_PLANE_BYTES) != 0 || memcmp(vm_a, vm_c, VF_PLANE_BYTES) != 0) {
      ivs_test_diag("seed %u: a frame changed with bits 15 and 14", (unsigned)k);
      return IVS_TEST_FAIL;
    }
  }
  return IVS_TEST_PASS;
}

/* Rendering twice gives equal planes and leaves the whole memory system (every byte of the instance) as it was. */
static int vm_purity(void)
{
  const vf_frame *f = vf_frame_get(1u); /* F-A2 */
  VC_CHECK(f != NULL);
  vf_mem_clear();
  vf_poke_gram(vf_mem(), f);
  vf_words(f, vm_words);
  memcpy(&vm_snap, vf_mem(), sizeof vm_snap);
  vf_render_words(vm_words, 0u, vm_a);
  VC_CHECK(memcmp(&vm_snap, vf_mem(), sizeof vm_snap) == 0);
  vf_render_words(vm_words, 0u, vm_b);
  VC_CHECK(memcmp(&vm_snap, vf_mem(), sizeof vm_snap) == 0);
  VC_CHECK(memcmp(vm_a, vm_b, VF_PLANE_BYTES) == 0);
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_video_meta_cases[] = {
    {"video.meta.translate", vm_translate, NULL},
    {"video.meta.bits-15-14-ignored", vm_bits_15_14_ignored, NULL},
    {"video.meta.purity", vm_purity, NULL},
};
const size_t ivs_video_meta_count = sizeof ivs_video_meta_cases / sizeof ivs_video_meta_cases[0];
