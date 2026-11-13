/* SPDX-License-Identifier: MIT */
/* The runner's own recomputation of the VIDEO frame chain (plan 06-12, D-16, D-17): from the frame
 * slots the core delivered (a 32-byte big-endian header, then height rows of width index bytes, the
 * layout documented beside IVS_OUTPUT_FORMAT_INDEX8_FRAMES_V1 in ivs/ivs_machine.h) it rebuilds the
 * per-frame chain
 *   h0 = SHA-256 of the 16 ASCII bytes IVS-VIDEO-LOG-V1,  h(n+1) = SHA-256(h(n) || slot bytes)
 * and the frame count, and from them the domain-16 digest
 *   SHA-256('I' 'V' 'S' 'D' || 16 || be16(1) || be32(32) || chain || be64(frames)),
 * without reading any core state, and it checks each slot header: index continuity, width and
 * height equal to ivs_info, the reserved word zero, only flag bit 0, every pixel a color 0..15 or
 * the blank value, and strictly increasing ticks at least nominal_frame_ticks apart. A header
 * finding is sticky and never stops the chain, so the recomputed value stays comparable with the
 * core's. Pure functions over a plain struct; no I/O, no allocation, no globals. The library never
 * shares code with the core: the SHA-256 is the runner-local copy (runner_sha256.c). */
#ifndef IVS_RUNNER_VCHAIN_H
#define IVS_RUNNER_VCHAIN_H

#include <stddef.h>
#include <stdint.h>

/* Findings of the header checks (the first one met is kept). */
#define IVS_RUNNER_VCHAIN_OK 0u
#define IVS_RUNNER_VCHAIN_E_INDEX 1u    /* index is not the report's first index, then +1 */
#define IVS_RUNNER_VCHAIN_E_WIDTH 2u    /* header width differs from ivs_info */
#define IVS_RUNNER_VCHAIN_E_HEIGHT 3u   /* header height differs from ivs_info */
#define IVS_RUNNER_VCHAIN_E_RESERVED 4u /* reserved word is not zero */
#define IVS_RUNNER_VCHAIN_E_FLAGS 5u    /* a flag bit other than bit 0 */
#define IVS_RUNNER_VCHAIN_E_PIXEL 6u    /* a pixel above 15 that is not the blank value */
#define IVS_RUNNER_VCHAIN_E_TICK 7u     /* ticks not strictly increasing or closer than nominal_frame_ticks */
#define IVS_RUNNER_VCHAIN_E_SIZE 8u     /* a byte count that is not a whole number of slots */

#define IVS_RUNNER_VCHAIN_HEADER 32u

typedef struct ivs_runner_vchain {
  uint8_t chain[32];
  uint64_t frames;      /* slots fed so far (chained) */
  uint64_t next_index;  /* the index the next slot must carry */
  uint64_t last_tick;   /* tick of the last slot */
  uint64_t nominal;     /* nominal_frame_ticks: the least spacing of two slots */
  uint32_t width;
  uint32_t height;
  uint32_t fault;       /* IVS_RUNNER_VCHAIN_*: the first finding, sticky */
  uint32_t reserved;
} ivs_runner_vchain;

/* Chain of nothing: h0, no frames, the first index 0. width, height and nominal come from ivs_info. */
void ivs_runner_vchain_init(ivs_runner_vchain *c, uint32_t width, uint32_t height, uint64_t nominal_frame_ticks);

/* Feeds n bytes, a whole number of slots (the slots of one run call, in order). first_index is the
 * report's video_first_index of that call: the first slot must carry it and it must continue the
 * previous call (frames fed so far). Returns the sticky finding (IVS_RUNNER_VCHAIN_OK when none). */
uint32_t ivs_runner_vchain_feed(ivs_runner_vchain *c, const uint8_t *bytes, size_t n, uint64_t first_index);

/* The domain-16 digest this chain implies (framing above). */
void ivs_runner_vchain_digest(const ivs_runner_vchain *c, uint8_t out[32]);

/* The chain value itself (the `assert video chain` value). */
void ivs_runner_vchain_value(const ivs_runner_vchain *c, uint8_t out[32]);

/* SHA-256 of the canonical text form of a published plane: height lines of width uppercase hex
 * digits (0-9, A-F), each ending in LF. Returns 0, or 1 when a position is not a color 0..15 (a blank
 * position has no digit: the hash form refuses such a frame, detail hash-needs-colors-only). */
int ivs_runner_frame_text_sha256(const uint8_t *plane, uint32_t width, uint32_t height, uint8_t out[32]);

#endif
