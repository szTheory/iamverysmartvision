/* SPDX-License-Identifier: MIT */
/* Streaming SHA-256 for the runner's audio chain re-hash (plan 05-09, D-18): a copy of the core's
 * core/src/common/ivs_sha256.c with renamed symbols (ivs_rsha256_*) and without the core's named-fault
 * blocks, because the runner library links only the public ivs_core and must not reach core/src.
 * Written from FIPS 180-4 (NIST, August 2015) alone: the functions of section 4.1.2, the constants K
 * of section 4.2.2, the padding of section 5.1.1, the initial hash value of section 5.3.3 and the
 * computation of section 6.2.2. Integer-only; no allocation; no state.
 *
 * Words are assembled big-endian by shifts after widening each byte to uint32_t, every rotate is on
 * uint32_t with an amount in 1..31, and the message length is appended as a 64-bit big-endian bit
 * count. Suite runner, case runner.exec.sha256-fips, checks it against FIPS 180-4 and
 * data/sha256/vectors.txt literals. */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "runner_sha256.h"

_Static_assert(sizeof(ivs_rsha256) == IVS_RSHA256_CTX_BYTES, "ivs_rsha256 has no implicit padding");

/* FIPS 180-4 section 4.2.2: the first 32 bits of the fractional parts of the cube roots of the
 * first 64 primes. */
static const uint32_t ivs_rsha256_k[64] = {
    0x428A2F98u, 0x71374491u, 0xB5C0FBCFu, 0xE9B5DBA5u, 0x3956C25Bu, 0x59F111F1u, 0x923F82A4u, 0xAB1C5ED5u,
    0xD807AA98u, 0x12835B01u, 0x243185BEu, 0x550C7DC3u, 0x72BE5D74u, 0x80DEB1FEu, 0x9BDC06A7u, 0xC19BF174u,
    0xE49B69C1u, 0xEFBE4786u, 0x0FC19DC6u, 0x240CA1CCu, 0x2DE92C6Fu, 0x4A7484AAu, 0x5CB0A9DCu, 0x76F988DAu,
    0x983E5152u, 0xA831C66Du, 0xB00327C8u, 0xBF597FC7u, 0xC6E00BF3u, 0xD5A79147u, 0x06CA6351u, 0x14292967u,
    0x27B70A85u, 0x2E1B2138u, 0x4D2C6DFCu, 0x53380D13u, 0x650A7354u, 0x766A0ABBu, 0x81C2C92Eu, 0x92722C85u,
    0xA2BFE8A1u, 0xA81A664Bu, 0xC24B8B70u, 0xC76C51A3u, 0xD192E819u, 0xD6990624u, 0xF40E3585u, 0x106AA070u,
    0x19A4C116u, 0x1E376C08u, 0x2748774Cu, 0x34B0BCB5u, 0x391C0CB3u, 0x4ED8AA4Au, 0x5B9CCA4Fu, 0x682E6FF3u,
    0x748F82EEu, 0x78A5636Fu, 0x84C87814u, 0x8CC70208u, 0x90BEFFFAu, 0xA4506CEBu, 0xBEF9A3F7u, 0xC67178F2u,
};

_Static_assert(sizeof ivs_rsha256_k / sizeof ivs_rsha256_k[0] == 64u, "64 round constants");

/* FIPS 180-4 section 5.3.3: the first 32 bits of the fractional parts of the square roots of
 * the first 8 primes. */
static const uint32_t ivs_rsha256_h0[8] = {
    0x6A09E667u, 0xBB67AE85u, 0x3C6EF372u, 0xA54FF53Au, 0x510E527Fu, 0x9B05688Cu, 0x1F83D9ABu, 0x5BE0CD19u,
};

_Static_assert(sizeof ivs_rsha256_h0 / sizeof ivs_rsha256_h0[0] == 8u, "8 initial hash words");

/* ROTR^n(x), section 2.2.2; n is always a constant in 1..31. */
static uint32_t ivs_rotr(uint32_t x, uint32_t n)
{
  return (x >> n) | (x << (32u - n));
}

/* Section 4.1.2, functions (4.2) to (4.7). */
static uint32_t ivs_ch(uint32_t x, uint32_t y, uint32_t z)
{
  return (x & y) ^ (~x & z);
}

static uint32_t ivs_maj(uint32_t x, uint32_t y, uint32_t z)
{
  return (x & y) ^ (x & z) ^ (y & z);
}

static uint32_t ivs_bsig0(uint32_t x)
{
  return ivs_rotr(x, 2u) ^ ivs_rotr(x, 13u) ^ ivs_rotr(x, 22u);
}

static uint32_t ivs_bsig1(uint32_t x)
{
  uint32_t r = ivs_rotr(x, 6u) ^ ivs_rotr(x, 11u) ^ ivs_rotr(x, 25u);
  return r;
}

static uint32_t ivs_ssig0(uint32_t x)
{
  return ivs_rotr(x, 7u) ^ ivs_rotr(x, 18u) ^ (x >> 3);
}

static uint32_t ivs_ssig1(uint32_t x)
{
  return ivs_rotr(x, 17u) ^ ivs_rotr(x, 19u) ^ (x >> 10);
}

/* Section 6.2.2: process one 64-byte block b into the hash value h. */
static void ivs_rsha256_compress(uint32_t h[8], const uint8_t *b)
{
  uint32_t w[64];
  uint32_t a = h[0], bb = h[1], cc = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
  uint32_t t;

  /* Step 1: the message schedule. */
  for (t = 0u; t < 16u; t++) {
    const uint8_t *q = b + 4u * t;
    w[t] = ((uint32_t)q[0] << 24) | ((uint32_t)q[1] << 16) | ((uint32_t)q[2] << 8) | (uint32_t)q[3];
  }
  for (t = 16u; t < 64u; t++) {
    w[t] = ivs_ssig1(w[t - 2u]) + w[t - 7u] + ivs_ssig0(w[t - 15u]) + w[t - 16u];
  }

  /* Steps 2 and 3: the 64 rounds (all additions are modulo 2^32 on uint32_t). */
  for (t = 0u; t < 64u; t++) {
    uint32_t k = ivs_rsha256_k[t];
    uint32_t t1 = hh + ivs_bsig1(e) + ivs_ch(e, f, g) + k + w[t];
    uint32_t t2 = ivs_bsig0(a) + ivs_maj(a, bb, cc);
    hh = g;
    g = f;
    f = e;
    e = d + t1;
    d = cc;
    cc = bb;
    bb = a;
    a = t1 + t2;
  }

  /* Step 4: the intermediate hash value. */
  h[0] += a;
  h[1] += bb;
  h[2] += cc;
  h[3] += d;
  h[4] += e;
  h[5] += f;
  h[6] += g;
  h[7] += hh;
}

void ivs_rsha256_init(ivs_rsha256 *c)
{
  uint64_t total = 0u;
  uint32_t i;
  for (i = 0u; i < 8u; i++) {
    c->h[i] = ivs_rsha256_h0[i];
  }
  c->total_bytes = total;
  memset(c->block, 0, sizeof c->block);
  c->used = 0u;
  c->reserved = 0u;
}

void ivs_rsha256_update(ivs_rsha256 *c, const uint8_t *p, size_t n)
{
  size_t i = 0u;
  if (n == 0u) {
    return;
  }
  c->total_bytes += (uint64_t)n;
  if (c->used != 0u) {
    while (i < n && c->used < IVS_RSHA256_BLOCK_BYTES) {
      c->block[c->used] = p[i];
      c->used++;
      i++;
    }
    if (c->used < IVS_RSHA256_BLOCK_BYTES) {
      return;
    }
    ivs_rsha256_compress(c->h, c->block);
    c->used = 0u;
  }
  while (n - i >= IVS_RSHA256_BLOCK_BYTES) {
    ivs_rsha256_compress(c->h, p + i);
    i += IVS_RSHA256_BLOCK_BYTES;
  }
  while (i < n) {
    c->block[c->used] = p[i];
    c->used++;
    i++;
  }
}

/* Section 5.1.1: append the bit 1 (byte 0x80), zero bytes up to 56 mod 64, then the message
 * length in bits as a 64-bit big-endian number. The pending bytes plus padding fill one block
 * when at most 55 bytes are pending, else two. */
void ivs_rsha256_final(ivs_rsha256 *c, uint8_t out[32])
{
  uint8_t pad[2u * IVS_RSHA256_BLOCK_BYTES];
  uint32_t rem = c->used;
  uint64_t bits = c->total_bytes << 3;
  uint32_t spill = rem >= 56u ? 1u : 0u;
  uint32_t nblocks = spill + 1u;
  uint32_t end = IVS_RSHA256_BLOCK_BYTES * nblocks;
  uint32_t ncompress;
  uint32_t i;

  memset(pad, 0, sizeof pad);
  for (i = 0u; i < rem; i++) {
    pad[i] = c->block[i];
  }
  pad[rem] = 0x80u;
  for (i = 0u; i < 8u; i++) {
    pad[end - 1u - i] = (uint8_t)(bits >> (8u * i));
  }
  ncompress = nblocks;
  for (i = 0u; i < ncompress; i++) {
    ivs_rsha256_compress(c->h, pad + IVS_RSHA256_BLOCK_BYTES * i);
  }
  for (i = 0u; i < 8u; i++) {
    out[4u * i] = (uint8_t)(c->h[i] >> 24);
    out[4u * i + 1u] = (uint8_t)(c->h[i] >> 16);
    out[4u * i + 2u] = (uint8_t)(c->h[i] >> 8);
    out[4u * i + 3u] = (uint8_t)c->h[i];
  }
}

void ivs_rsha256_digest(const uint8_t *p, size_t n, uint8_t out[32])
{
  ivs_rsha256 c = {{0u}, 0u, {0u}, 0u, 0u};
  ivs_rsha256_init(&c);
  ivs_rsha256_update(&c, p, n);
  ivs_rsha256_final(&c, out);
}
