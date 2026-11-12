/* SPDX-License-Identifier: MIT */
/* Audio level-change chain recomputation of the runner (plan 05-09, D-18); framing in
 * runner_chain.h. Hand-written big-endian stores; the SHA-256 is the runner-local copy
 * (runner_sha256.c). Named faults: none here (the fault sites of the runner are in exec.c). */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "runner_chain.h"
#include "runner_sha256.h"

static void rc_put_be64(uint8_t *d, uint64_t v)
{
  for (uint32_t i = 0u; i < 8u; i++) {
    d[i] = (uint8_t)(v >> (56u - 8u * i));
  }
}

static void rc_put_be32(uint8_t *d, uint32_t v)
{
  for (uint32_t i = 0u; i < 4u; i++) {
    d[i] = (uint8_t)(v >> (24u - 8u * i));
  }
}

void ivs_runner_achain_init(ivs_runner_achain *c) { memset(c, 0, sizeof *c); }

void ivs_runner_achain_feed(ivs_runner_achain *c, const uint8_t *bytes, size_t n)
{
  for (size_t off = 0u; n - off >= 4u; off += 4u) {
    const uint8_t *q = bytes + off;
    uint32_t word = (uint32_t)q[0] | ((uint32_t)q[1] << 8) | ((uint32_t)q[2] << 16) | ((uint32_t)q[3] << 24);
    c->samples += 1u;
    if (word != c->prev) {
      uint8_t pre[4u + 1u + 32u + 8u + 4u + 12u];
      uint32_t k = 0u;
      pre[k++] = (uint8_t)'I';
      pre[k++] = (uint8_t)'V';
      pre[k++] = (uint8_t)'S';
      pre[k++] = (uint8_t)'R';
      pre[k++] = 17u;
      memcpy(pre + k, c->chain, 32u);
      k += 32u;
      rc_put_be64(pre + k, c->changes);
      k += 8u;
      rc_put_be32(pre + k, 12u);
      k += 4u;
      rc_put_be64(pre + k, c->samples);
      k += 8u;
      memcpy(pre + k, q, 4u);
      k += 4u;
      ivs_rsha256_digest(pre, k, c->chain);
      c->changes += 1u;
    }
    c->prev = word;
  }
}

void ivs_runner_achain_digest(const ivs_runner_achain *c, uint8_t out[32])
{
  uint8_t pre[4u + 1u + 2u + 4u + 32u + 8u + 8u + 4u];
  uint32_t k = 0u;
  pre[k++] = (uint8_t)'I';
  pre[k++] = (uint8_t)'V';
  pre[k++] = (uint8_t)'S';
  pre[k++] = (uint8_t)'D';
  pre[k++] = 17u;
  pre[k++] = 0u;
  pre[k++] = 1u;
  rc_put_be32(pre + k, 32u);
  k += 4u;
  memcpy(pre + k, c->chain, 32u);
  k += 32u;
  rc_put_be64(pre + k, c->changes);
  k += 8u;
  rc_put_be64(pre + k, c->samples);
  k += 8u;
  rc_put_be32(pre + k, c->prev);
  k += 4u;
  ivs_rsha256_digest(pre, k, out);
}
