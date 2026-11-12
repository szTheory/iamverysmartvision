/* SPDX-License-Identifier: MIT */
/* The runner's own recomputation of the AUDIO level-change chain (plan 05-09, D-18): from the dense
 * sample bytes the core delivered (4 per sample: level A, B, C, 0) it rebuilds the chain, the
 * level-change count, the emitted-sample count and the previous levels, and from them the
 * domain-17 digest, with the framing documented in ivs/ivs_machine.h, without reading any core
 * state. A chip step k (k = 1, 2, ...) whose three levels differ from the previous sample's
 * (initially 0) appends the record k || A || B || C || 0 to the chain:
 *   chain' = SHA256('I' 'V' 'S' 'R' || 17 || chain || be64(changes before) || be32(12) || record)
 * with chain initially 32 zero bytes. Pure functions over a plain struct; no I/O, no allocation. */
#ifndef IVS_RUNNER_CHAIN_H
#define IVS_RUNNER_CHAIN_H

#include <stddef.h>
#include <stdint.h>

typedef struct ivs_runner_achain {
  uint8_t chain[32];
  uint64_t changes; /* level-change records appended so far */
  uint64_t samples; /* samples fed so far (the step number of the last one) */
  uint32_t prev;    /* the previous sample's four bytes as a little-endian number (A in bits 7..0) */
  uint32_t reserved;
} ivs_runner_achain;

/* Chain of nothing: 32 zero bytes, no changes, no samples, previous levels 0. */
void ivs_runner_achain_init(ivs_runner_achain *c);

/* Feeds n bytes, n a multiple of 4 (a trailing partial sample is ignored): the dense samples of
 * one run call, in order. */
void ivs_runner_achain_feed(ivs_runner_achain *c, const uint8_t *bytes, size_t n);

/* The domain-17 digest this chain implies: SHA-256 of
 * 'I' 'V' 'S' 'D' || 17 || be16(1) || be32(32) || chain || be64(changes) || be64(samples) ||
 * be32(prev). */
void ivs_runner_achain_digest(const ivs_runner_achain *c, uint8_t out[32]);

#endif
