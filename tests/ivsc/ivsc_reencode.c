/* SPDX-License-Identifier: MIT */
/* Canonical IVSC v1 re-encoder for tests (D-34, plans 03-07 and 03-08): rebuilds a container
 * from an accepted view (prefix, entries, the words found at the view's data offsets, a trailer
 * from the core SHA-256) and compares it with the input. Used by ivsc.X02-canonical-roundtrip,
 * the seeded fuzz family and the optional libFuzzer target tests/fuzz/fuzz_ivsc.c. Test code
 * only; it may allocate. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common/ivs_sha256.h"
#include "content/ivsc.h"
#include "ivsc_fuzz.h"

static const uint8_t re_magic[IVS_IVSC_MAGIC_BYTES] = {0x89u, 0x49u, 0x56u, 0x53u, 0x43u, 0x0Du, 0x0Au, 0x1Au};

static void re_put16(uint8_t *p, uint32_t v)
{
  p[0] = (uint8_t)((v >> 8) & 0xFFu);
  p[1] = (uint8_t)(v & 0xFFu);
}

static void re_put32(uint8_t *p, uint32_t v)
{
  p[0] = (uint8_t)((v >> 24) & 0xFFu);
  p[1] = (uint8_t)((v >> 16) & 0xFFu);
  p[2] = (uint8_t)((v >> 8) & 0xFFu);
  p[3] = (uint8_t)(v & 0xFFu);
}

/* Re-encode an accepted view; 1 when the encoding equals the input bytes. */
int ivsc_test_reencode_equal(const uint8_t *b, size_t len, const ivs_ivsc_view *v)
{
  uint8_t *e;
  size_t at;
  uint32_t i;
  uint32_t w;
  int same;
  if (v->total_bytes != len || v->segment_count == 0u || v->segment_count > IVS_IVSC_MAX_SEGMENTS ||
      len < IVS_IVSC_MIN_BYTES) {
    return 0;
  }
  e = malloc(len);
  if (e == NULL) {
    return 0;
  }
  memcpy(e, re_magic, sizeof re_magic);
  re_put16(e + 8, v->version);
  re_put16(e + 10, v->flags);
  re_put32(e + 12, v->required_features);
  re_put32(e + 16, v->total_bytes);
  re_put16(e + 20, v->segment_count);
  re_put16(e + 22, 0u);
  at = IVS_IVSC_PREFIX_BYTES;
  for (i = 0u; i < v->segment_count; i++) {
    if (at + IVS_IVSC_ENTRY_BYTES > len) {
      free(e);
      return 0;
    }
    re_put16(e + at, v->segs[i].addr);
    re_put16(e + at + 2u, v->segs[i].len_words);
    e[at + 4u] = (uint8_t)IVS_IVSC_WIDTH_BITS;
    e[at + 5u] = 0u;
    re_put16(e + at + 6u, 0u);
    at += IVS_IVSC_ENTRY_BYTES;
  }
  for (i = 0u; i < v->segment_count; i++) {
    for (w = 0u; w < v->segs[i].len_words; w++) {
      size_t src = (size_t)v->segs[i].data_byte_offset + 2u * (size_t)w;
      if (at + 2u > len - IVS_IVSC_TRAILER_BYTES || src + 2u > len) {
        free(e);
        return 0;
      }
      e[at] = b[src];
      e[at + 1u] = b[src + 1u];
      at += 2u;
    }
  }
  if (at != len - IVS_IVSC_TRAILER_BYTES) {
    free(e);
    return 0;
  }
  ivs_sha256_digest(e, at, e + at);
  same = memcmp(e, b, len) == 0;
  free(e);
  return same;
}
