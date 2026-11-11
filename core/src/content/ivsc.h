/* SPDX-License-Identifier: MIT */
/* IVSC v1 cartridge container, layer 1 (D-29..D-33, A-01, A-03): a pure, zero-copy, bounded
 * parser from container bytes to a pointer-free view. The format is specified in
 * docs/formats/ivsc-v1.md; this header carries its limits, the reason numbers of that spec and
 * the parser interface. Internal core interface, not installed and not an ABI.
 *
 * Layer 1 knows no profile: it checks the container's own rules only. Layer 2 (plan 03-11)
 * takes the view, checks each segment against the profile's cartridge span and the
 * required_features mask against the profile, and copies the words into an instance. */
#ifndef IVS_IVSC_H
#define IVS_IVSC_H

#include <stddef.h>
#include <stdint.h>

#include "ivs/ivs_status.h"

/* Limits and fixed sizes (bytes unless named otherwise). */
#define IVS_IVSC_MAGIC_BYTES 8u
#define IVS_IVSC_PREFIX_BYTES 24u
#define IVS_IVSC_ENTRY_BYTES 8u
#define IVS_IVSC_TRAILER_BYTES 32u
#define IVS_IVSC_MAX_SEGMENTS 16u
#define IVS_IVSC_MAX_SEGMENT_WORDS 65535u
#define IVS_IVSC_MIN_BYTES 66u     /* prefix 24 + one entry 8 + one word 2 + trailer 32 */
#define IVS_IVSC_MAX_BYTES 131256u /* prefix 24 + 16 entries 128 + 65536 words 131072 + 32 */
#define IVS_IVSC_VERSION 1u
#define IVS_IVSC_WIDTH_BITS 10u
#define IVS_IVSC_WORD_MAX 0x03FFu
#define IVS_IVSC_ADDRESS_SPACE_WORDS 0x10000u

/* Refusal reasons. The numbers are stable and listed in the reason table of the spec; a new
 * reason gets the next number. Each reason maps to exactly one status (spec, "Refusal
 * reasons"). */
#define IVS_IVSC_R_NONE 0u
#define IVS_IVSC_R_TRUNCATED 1u        /* MALFORMED: length below 66, or total_bytes above it */
#define IVS_IVSC_R_TOO_LARGE 2u        /* OVERSIZED: length or total_bytes above 131,256 */
#define IVS_IVSC_R_MAGIC 3u            /* MALFORMED */
#define IVS_IVSC_R_VERSION 4u          /* MALFORMED */
#define IVS_IVSC_R_FLAGS 5u            /* MALFORMED */
#define IVS_IVSC_R_RESERVED 6u         /* MALFORMED: prefix reserved field */
#define IVS_IVSC_R_TRAILING 7u         /* MALFORMED: total_bytes below the length */
#define IVS_IVSC_R_SEGMENT_COUNT 8u    /* MALFORMED: count 0 or above 16 */
#define IVS_IVSC_R_SIZE_MISMATCH 9u    /* MALFORMED: total_bytes differs from the table */
#define IVS_IVSC_R_SEGMENT_LENGTH 10u  /* MALFORMED: len_words 0 */
#define IVS_IVSC_R_SEGMENT_WIDTH 11u   /* MALFORMED: width other than 10 */
#define IVS_IVSC_R_SEGMENT_ATTR 12u    /* MALFORMED: attr other than 0 */
#define IVS_IVSC_R_SEGMENT_RESERVED 13u /* MALFORMED: entry reserved field */
#define IVS_IVSC_R_ADDR_WRAP 14u       /* OUT_OF_RANGE: addr + len_words above 0x10000 */
#define IVS_IVSC_R_ORDER 15u           /* MALFORMED: start below the previous start */
#define IVS_IVSC_R_OVERLAP 16u         /* OVERLAP */
#define IVS_IVSC_R_DIGEST 17u          /* DIGEST_MISMATCH */
#define IVS_IVSC_R_WORD_RANGE 18u      /* MALFORMED: a data word above 0x3FF */
#define IVS_IVSC_REASON_COUNT 19u

/* segment_index of a report whose reason concerns no single segment. */
#define IVS_IVSC_NO_SEGMENT 0xFFFFFFFFu

/* One segment of an accepted container. data_byte_offset is where its first word starts in
 * the container; it is computed from the checked counts, never read from the file. */
typedef struct ivs_ivsc_segment {
  uint16_t addr;
  uint16_t len_words;
  uint32_t data_byte_offset;
} ivs_ivsc_segment;

/* Pointer-free view of an accepted container (zero-copy: the words stay in the caller's
 * buffer at segs[i].data_byte_offset, big-endian, two bytes each). Entries past
 * segment_count are zero. */
typedef struct ivs_ivsc_view {
  uint16_t version;
  uint16_t flags;
  uint32_t required_features;
  uint32_t total_bytes;
  uint32_t segment_count;
  ivs_ivsc_segment segs[IVS_IVSC_MAX_SEGMENTS];
} ivs_ivsc_view;

/* Refusal report. byte_offset names the byte where the defect is seen (spec, "Refusal
 * reasons"); segment_index is the entry concerned or IVS_IVSC_NO_SEGMENT; required_features
 * is the prefix field when the length stage passed, else 0. */
typedef struct ivs_ivsc_report {
  uint32_t reason;
  uint32_t segment_index;
  uint32_t byte_offset;
  uint32_t required_features;
} ivs_ivsc_report;

/* Parse len bytes at bytes. Checks run in the fixed order of the spec (length, magic,
 * version, prefix, table, order and overlap, digest, word range); the first failing check
 * decides. Returns:
 *   IVS_OK           *view is written, *rep is not touched;
 *   a content code   (MALFORMED, OVERSIZED, OVERLAP, OUT_OF_RANGE, DIGEST_MISMATCH) *rep is
 *                    written, *view is not touched;
 *   IVS_E_BAD_ARG    view or rep is NULL, or bytes is NULL with len above 0; nothing written.
 * A len above IVS_IVSC_MAX_BYTES is refused before any byte is read. No byte at or past
 * bytes + len is ever read. Pure: no allocation, no global state. */
ivs_status ivs_ivsc_parse(const uint8_t *bytes, size_t len, ivs_ivsc_view *view, ivs_ivsc_report *rep);

#endif
