/* SPDX-License-Identifier: MIT */
/* VIDEO frame chain recomputation and slot checks of the runner (plan 06-12, D-16, D-17); framing in
 * runner_vchain.h. Hand-written big-endian reads and stores; the SHA-256 is the runner-local copy
 * (runner_sha256.c). Named faults: none here (the fault sites of the runner are in exec.c). */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "runner_sha256.h"
#include "runner_vchain.h"

static uint64_t vc_get_be(const uint8_t *p, uint32_t n)
{
  uint64_t v = 0u;
  for (uint32_t i = 0u; i < n; i++) {
    v = (v << 8) | p[i];
  }
  return v;
}

static void vc_put_be(uint8_t *d, uint64_t v, uint32_t n)
{
  for (uint32_t i = 0u; i < n; i++) {
    d[i] = (uint8_t)(v >> (8u * (n - 1u - i)));
  }
}

static void vc_h0(uint8_t out[32])
{
  static const uint8_t label[16] = {'I', 'V', 'S', '-', 'V', 'I', 'D', 'E', 'O', '-', 'L', 'O', 'G', '-', 'V', '1'};
  ivs_rsha256_digest(label, sizeof label, out);
}

void ivs_runner_vchain_init(ivs_runner_vchain *c, uint32_t width, uint32_t height, uint64_t nominal_frame_ticks)
{
  memset(c, 0, sizeof *c);
  vc_h0(c->chain);
  c->width = width;
  c->height = height;
  c->nominal = nominal_frame_ticks;
}

static void vc_note(ivs_runner_vchain *c, uint32_t finding)
{
  if (c->fault == IVS_RUNNER_VCHAIN_OK) {
    c->fault = finding;
  }
}

uint32_t ivs_runner_vchain_feed(ivs_runner_vchain *c, const uint8_t *bytes, size_t n, uint64_t first_index)
{
  size_t slot = (size_t)IVS_RUNNER_VCHAIN_HEADER + (size_t)c->width * (size_t)c->height;
  if (n % slot != 0u) {
    vc_note(c, IVS_RUNNER_VCHAIN_E_SIZE);
    n -= n % slot; /* a trailing partial slot is not chained */
  }
  if (n != 0u && first_index != c->next_index) {
    vc_note(c, IVS_RUNNER_VCHAIN_E_INDEX);
  }
  for (size_t off = 0u; off < n; off += slot) {
    const uint8_t *s = bytes + off;
    ivs_rsha256 h;
    uint64_t index = vc_get_be(s + 8u, 8u);
    uint64_t tick = vc_get_be(s + 0u, 8u);
    if (index != c->next_index) {
      vc_note(c, IVS_RUNNER_VCHAIN_E_INDEX);
    }
    if (vc_get_be(s + 16u, 4u) != c->width) {
      vc_note(c, IVS_RUNNER_VCHAIN_E_WIDTH);
    }
    if (vc_get_be(s + 20u, 4u) != c->height) {
      vc_note(c, IVS_RUNNER_VCHAIN_E_HEIGHT);
    }
    if ((vc_get_be(s + 24u, 4u) & ~(uint64_t)1u) != 0u) {
      vc_note(c, IVS_RUNNER_VCHAIN_E_FLAGS);
    }
    if (vc_get_be(s + 28u, 4u) != 0u) {
      vc_note(c, IVS_RUNNER_VCHAIN_E_RESERVED);
    }
    if (c->frames != 0u && (tick <= c->last_tick || tick - c->last_tick < c->nominal)) {
      vc_note(c, IVS_RUNNER_VCHAIN_E_TICK);
    }
    for (size_t i = IVS_RUNNER_VCHAIN_HEADER; i < slot; i++) {
      if (s[i] > 15u && s[i] != 0xFFu) {
        vc_note(c, IVS_RUNNER_VCHAIN_E_PIXEL);
        break;
      }
    }
    ivs_rsha256_init(&h);
    ivs_rsha256_update(&h, c->chain, 32u);
    ivs_rsha256_update(&h, s, slot);
    ivs_rsha256_final(&h, c->chain);
    c->frames += 1u;
    c->next_index += 1u;
    c->last_tick = tick;
  }
  return c->fault;
}

void ivs_runner_vchain_value(const ivs_runner_vchain *c, uint8_t out[32]) { memcpy(out, c->chain, 32u); }

void ivs_runner_vchain_digest(const ivs_runner_vchain *c, uint8_t out[32])
{
  uint8_t pre[4u + 1u + 2u + 4u + 32u + 8u];
  uint32_t k = 0u;
  pre[k++] = (uint8_t)'I';
  pre[k++] = (uint8_t)'V';
  pre[k++] = (uint8_t)'S';
  pre[k++] = (uint8_t)'D';
  pre[k++] = 16u;
  pre[k++] = 0u;
  pre[k++] = 1u;
  vc_put_be(pre + k, 32u, 4u);
  k += 4u;
  memcpy(pre + k, c->chain, 32u);
  k += 32u;
  vc_put_be(pre + k, c->frames, 8u);
  k += 8u;
  ivs_rsha256_digest(pre, k, out);
}

int ivs_runner_frame_text_sha256(const uint8_t *plane, uint32_t width, uint32_t height, uint8_t out[32])
{
  static const char digits[] = "0123456789ABCDEF";
  ivs_rsha256 h;
  uint8_t line[256];
  if (width == 0u || width >= sizeof line) {
    return 1;
  }
  ivs_rsha256_init(&h);
  for (uint32_t y = 0u; y < height; y++) {
    for (uint32_t x = 0u; x < width; x++) {
      uint8_t p = plane[(size_t)y * width + x];
      if (p > 15u) {
        return 1;
      }
      line[x] = (uint8_t)digits[p];
    }
    line[width] = (uint8_t)'\n';
    ivs_rsha256_update(&h, line, (size_t)width + 1u);
  }
  ivs_rsha256_final(&h, out);
  return 0;
}
