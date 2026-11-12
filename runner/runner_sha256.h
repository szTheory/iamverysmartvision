/* SPDX-License-Identifier: MIT */
/* Streaming SHA-256 (D-28, A-04): runner-local copy (plan 05-09) of the core utility ivs_sha256 with renamed symbols, written from FIPS 180-4 (sections
 * 4.1.2, 4.2.2, 5.1.1, 5.3.3, 6.2.2). Not installed and not an ABI; used only by the audio chain re-hash (runner/runner_chain.c).
 *
 * The context is a plain struct with no pointers and no implicit padding, so assigning one
 * context to another is a snapshot of a partial hash: finishing the copy gives the digest of
 * the bytes fed so far while the original can keep going (A-04: the IVSC trailer digest and the
 * whole-file digest come out of one pass). Layout, 112 bytes (IVS_RSHA256_CTX_BYTES):
 *   h[8]         32  working hash value H(i)
 *   total_bytes   8  message bytes fed so far (the length field is 8 * total_bytes, mod 2^64)
 *   block[64]    64  pending bytes of the current block
 *   used          4  pending byte count, 0..63
 *   reserved      4  always 0 (keeps the struct free of implicit padding)
 * The digest is 32 bytes, H0..H7 each written big-endian (FIPS 180-4 section 6.2.2 step 4). */
#ifndef IVS_RSHA256_H
#define IVS_RSHA256_H

#include <stddef.h>
#include <stdint.h>

#define IVS_RSHA256_DIGEST_BYTES 32u
#define IVS_RSHA256_BLOCK_BYTES 64u
#define IVS_RSHA256_CTX_BYTES 112u

typedef struct ivs_rsha256 {
  uint32_t h[8];
  uint64_t total_bytes;
  uint8_t block[64];
  uint32_t used;
  uint32_t reserved;
} ivs_rsha256;

/* Set c to the FIPS 180-4 initial hash value with no message bytes fed. Every field is
 * written, so c may hold anything before the call. */
void ivs_rsha256_init(ivs_rsha256 *c);

/* Feed n bytes at p (p may be NULL only when n is 0). */
void ivs_rsha256_update(ivs_rsha256 *c, const uint8_t *p, size_t n);

/* Pad, finish and write the 32-byte digest to out. c is consumed: call ivs_rsha256_init before
 * feeding it again. */
void ivs_rsha256_final(ivs_rsha256 *c, uint8_t out[32]);

/* One-shot digest of n bytes at p. */
void ivs_rsha256_digest(const uint8_t *p, size_t n, uint8_t out[32]);

#endif
