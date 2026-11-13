/* SPDX-License-Identifier: MIT */
/* Operator-side frame export (plan 06-12, D-17); the contract and the palette note are in frame_export.h: operator preview
 * palette, not STIC colour data. Integer only; no I/O, no allocation, no globals beyond the constant palette. */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "frame_export.h"
#include "ivs/ivs.h"

/* Project-authored, arbitrary values (a hue ramp with a dark and a light end), chosen only to be mutually distinct. */
static const uint8_t fp_palette[IVS_FRAMEPPM_PALETTE_ENTRIES][3] = {
    {0u, 0u, 0u},       {34u, 34u, 170u},   {170u, 34u, 34u},   {204u, 136u, 34u},  {34u, 136u, 34u},   {34u, 170u, 170u},
    {170u, 34u, 170u},  {221u, 221u, 51u},  {85u, 85u, 85u},    {102u, 102u, 238u}, {238u, 102u, 102u}, {255u, 187u, 102u},
    {102u, 204u, 102u}, {102u, 238u, 238u}, {238u, 102u, 238u}, {255u, 255u, 255u}, {255u, 0u, 255u}};

static size_t fp_digits(uint32_t v)
{
  size_t n = 1u;
  while (v >= 10u) {
    v /= 10u;
    n++;
  }
  return n;
}

size_t ivs_frameppm_bound(uint32_t width, uint32_t height)
{
  uint64_t px;
  uint64_t total;
  if (width == 0u || height == 0u) {
    return 0u;
  }
  px = (uint64_t)width * (uint64_t)height;
  if (px > (UINT64_MAX - 64u) / 3u || px * 3u + 64u > (uint64_t)SIZE_MAX) {
    return SIZE_MAX;
  }
  total = 3u + (uint64_t)fp_digits(width) + 1u + (uint64_t)fp_digits(height) + 1u + 4u + px * 3u;
  return (size_t)total;
}

int ivs_frameppm_palette(uint32_t i, uint8_t rgb[3])
{
  if (i >= IVS_FRAMEPPM_PALETTE_ENTRIES || rgb == NULL) {
    return 0;
  }
  memcpy(rgb, fp_palette[i], 3u);
  return 1;
}

static size_t fp_put_dec(uint8_t *out, uint32_t v)
{
  size_t n = fp_digits(v);
  for (size_t i = n; i > 0u; i--) {
    out[i - 1u] = (uint8_t)('0' + (v % 10u));
    v /= 10u;
  }
  return n;
}

int ivs_frameppm_convert(const uint8_t *plane, uint32_t width, uint32_t height, uint8_t *out, size_t cap, size_t *written)
{
  size_t bound;
  size_t k = 0u;
  if (plane == NULL || out == NULL || written == NULL || width == 0u || height == 0u) {
    return IVS_FRAMEPPM_E_ARG;
  }
  *written = 0u;
  bound = ivs_frameppm_bound(width, height);
  if (bound == SIZE_MAX) {
    return IVS_FRAMEPPM_E_ARG;
  }
  if (cap < bound) {
    return IVS_FRAMEPPM_E_SPACE;
  }
  out[k++] = (uint8_t)'P';
  out[k++] = (uint8_t)'6';
  out[k++] = (uint8_t)'\n';
  k += fp_put_dec(out + k, width);
  out[k++] = (uint8_t)' ';
  k += fp_put_dec(out + k, height);
  out[k++] = (uint8_t)'\n';
  out[k++] = (uint8_t)'2';
  out[k++] = (uint8_t)'5';
  out[k++] = (uint8_t)'5';
  out[k++] = (uint8_t)'\n';
  for (size_t i = 0u; i < (size_t)width * (size_t)height; i++) {
    uint8_t p = plane[i];
    uint32_t e;
    if (p <= IVS_PIXEL_COLOR_MAX) {
      e = p;
    } else if (p == IVS_PIXEL_BLANK) {
      e = IVS_FRAMEPPM_PALETTE_ENTRIES - 1u;
    } else {
      return IVS_FRAMEPPM_E_PIXEL;
    }
    memcpy(out + k, fp_palette[e], 3u);
    k += 3u;
  }
  *written = k;
  return IVS_FRAMEPPM_OK;
}
