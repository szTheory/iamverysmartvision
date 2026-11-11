/* SPDX-License-Identifier: MIT */
/* IVSC v1 parser, layer 1 (D-29..D-33, A-01, A-03). The format and its check order are
 * specified in the project's own words in docs/formats/ivsc-v1.md; nothing here is taken from
 * another container format or another implementation. Pure and zero-copy: the input is only
 * read, a refusal writes only the report, an acceptance writes only the view. Every size is
 * computed in uint64_t from counts already checked, every multi-byte field is assembled from
 * single bytes by shifts after widening (big-endian, D-30), and every loop is bounded by a
 * checked count. No allocation, no host services, integer only. */
#include "common/ivs_prelude.h"

#include "content/ivsc.h"

#include "common/ivs_sha256.h"

/* 0x89, "IVSC", CR LF SUB: a high-bit first byte and a CR LF pair expose 7-bit and text-mode
 * transfers. */
static const uint8_t ivsc_magic[IVS_IVSC_MAGIC_BYTES] = {0x89u, 0x49u, 0x56u, 0x53u, 0x43u, 0x0Du, 0x0Au, 0x1Au};

/* Prefix field offsets. */
#define IVSC_OFF_VERSION 8u
#define IVSC_OFF_FLAGS 10u
#define IVSC_OFF_FEATURES IVS_IVSC_FEATURES_OFFSET
#define IVSC_OFF_TOTAL 16u
#define IVSC_OFF_COUNT 20u
#define IVSC_OFF_RESERVED 22u

/* Entry field offsets inside an 8-byte segment entry. */
#define IVSC_ENT_ADDR 0u
#define IVSC_ENT_LEN 2u
#define IVSC_ENT_WIDTH 4u
#define IVSC_ENT_ATTR 5u
#define IVSC_ENT_RESERVED 6u

static uint32_t ivsc_be16(const uint8_t *p)
{
  return ((uint32_t)p[0] << 8) | (uint32_t)p[1];
}

static uint32_t ivsc_be32(const uint8_t *p)
{
  return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

/* Fill the report and return the status. Every offset passed here is at most
 * IVS_IVSC_MAX_BYTES, so it fits the 32-bit field. */
static ivs_status ivsc_refuse(ivs_ivsc_report *rep, ivs_status st, uint32_t reason, uint32_t seg, uint64_t off,
                              uint32_t features)
{
  rep->reason = reason;
  rep->segment_index = seg;
  rep->byte_offset = (uint32_t)off;
  rep->required_features = features;
  return st;
}

ivs_status ivs_ivsc_parse(const uint8_t *bytes, size_t len, ivs_ivsc_view *view, ivs_ivsc_report *rep)
{
  ivs_ivsc_view v;
  ivs_sha256 ctx = {{0u}, 0u, {0u}, 0u, 0u}; /* zeroed first, as ivs_sha256_digest does */
  ivs_sha256 trail;
  uint8_t digest[IVS_SHA256_DIGEST_BYTES];
  uint8_t file_digest[IVS_SHA256_DIGEST_BYTES];
  uint64_t n_bytes;
  uint64_t total;
  uint64_t n_seg;
  uint64_t words;
  uint64_t data_at;
  uint64_t wrap_limit;
  uint32_t features;
  uint32_t word_max;
  uint32_t diff;
  uint32_t i;
  uint32_t k;

  if (view == NULL || rep == NULL || (bytes == NULL && len != 0u)) {
    return IVS_E_BAD_ARG;
  }

  /* 1. Length: refused before any byte is read. */
  n_bytes = (uint64_t)len;
  if (n_bytes > IVS_IVSC_MAX_BYTES) {
    return ivsc_refuse(rep, IVS_E_OVERSIZED, IVS_IVSC_R_TOO_LARGE, IVS_IVSC_NO_SEGMENT, IVS_IVSC_MAX_BYTES, 0u);
  }
  if (n_bytes < IVS_IVSC_MIN_BYTES) {
    return ivsc_refuse(rep, IVS_E_MALFORMED, IVS_IVSC_R_TRUNCATED, IVS_IVSC_NO_SEGMENT, n_bytes, 0u);
  }
  features = ivsc_be32(bytes + IVSC_OFF_FEATURES);

  /* 2. Magic: the first differing byte is reported. */
  for (k = 0u; k < IVS_IVSC_MAGIC_BYTES; k++) {
    if (bytes[k] != ivsc_magic[k]) {
      return ivsc_refuse(rep, IVS_E_MALFORMED, IVS_IVSC_R_MAGIC, IVS_IVSC_NO_SEGMENT, k, features);
    }
  }

  /* 3. Version. */
  if (ivsc_be16(bytes + IVSC_OFF_VERSION) != IVS_IVSC_VERSION) {
    return ivsc_refuse(rep, IVS_E_MALFORMED, IVS_IVSC_R_VERSION, IVS_IVSC_NO_SEGMENT, IVSC_OFF_VERSION, features);
  }

  /* 4. Prefix: flags, reserved, total_bytes against the limit and the length, segment count.
   * required_features is not judged here: it is a profile question (layer 2, D-32). */
  if (ivsc_be16(bytes + IVSC_OFF_FLAGS) != 0u) {
    return ivsc_refuse(rep, IVS_E_MALFORMED, IVS_IVSC_R_FLAGS, IVS_IVSC_NO_SEGMENT, IVSC_OFF_FLAGS, features);
  }
  if (ivsc_be16(bytes + IVSC_OFF_RESERVED) != 0u) {
    return ivsc_refuse(rep, IVS_E_MALFORMED, IVS_IVSC_R_RESERVED, IVS_IVSC_NO_SEGMENT, IVSC_OFF_RESERVED, features);
  }
  total = (uint64_t)ivsc_be32(bytes + IVSC_OFF_TOTAL);
  if (total > IVS_IVSC_MAX_BYTES) {
    return ivsc_refuse(rep, IVS_E_OVERSIZED, IVS_IVSC_R_TOO_LARGE, IVS_IVSC_NO_SEGMENT, IVSC_OFF_TOTAL, features);
  }
  if (total > n_bytes) {
    return ivsc_refuse(rep, IVS_E_MALFORMED, IVS_IVSC_R_TRUNCATED, IVS_IVSC_NO_SEGMENT, n_bytes, features);
  }
  if (total < n_bytes) {
    return ivsc_refuse(rep, IVS_E_MALFORMED, IVS_IVSC_R_TRAILING, IVS_IVSC_NO_SEGMENT, total, features);
  }
  n_seg = (uint64_t)ivsc_be16(bytes + IVSC_OFF_COUNT);
#ifdef IVS_FAULT_IVSC_COUNT_17
  if (n_seg == IVS_IVSC_MAX_SEGMENTS + 1u) n_seg = IVS_IVSC_MAX_SEGMENTS; /* 17 passes; table read as 16 */
#endif
  if (n_seg == 0u || n_seg > IVS_IVSC_MAX_SEGMENTS) {
    return ivsc_refuse(rep, IVS_E_MALFORMED, IVS_IVSC_R_SEGMENT_COUNT, IVS_IVSC_NO_SEGMENT, IVSC_OFF_COUNT, features);
  }

  /* 5. Table. Every entry plus one word per entry plus the trailer must fit before an entry is
   * read, so no entry read can pass the end of the buffer (total equals len here). */
  if (total < IVS_IVSC_PREFIX_BYTES + n_seg * (IVS_IVSC_ENTRY_BYTES + 2u) + IVS_IVSC_TRAILER_BYTES) {
    return ivsc_refuse(rep, IVS_E_MALFORMED, IVS_IVSC_R_SIZE_MISMATCH, IVS_IVSC_NO_SEGMENT, IVSC_OFF_TOTAL, features);
  }
  wrap_limit = IVS_IVSC_ADDRESS_SPACE_WORDS;
#ifdef IVS_FAULT_IVSC_END_OFF_BY_ONE
  wrap_limit = IVS_IVSC_ADDRESS_SPACE_WORDS - 1u;
#endif
  memset(&v, 0, sizeof v);
  words = 0u;
  data_at = IVS_IVSC_PREFIX_BYTES + n_seg * IVS_IVSC_ENTRY_BYTES;
  for (i = 0u; i < (uint32_t)n_seg; i++) {
    uint64_t at = IVS_IVSC_PREFIX_BYTES + (uint64_t)i * IVS_IVSC_ENTRY_BYTES;
    const uint8_t *e = bytes + (size_t)at;
    uint32_t addr = ivsc_be16(e + IVSC_ENT_ADDR);
    uint32_t len_words = ivsc_be16(e + IVSC_ENT_LEN);
    if (len_words == 0u) {
      return ivsc_refuse(rep, IVS_E_MALFORMED, IVS_IVSC_R_SEGMENT_LENGTH, i, at + IVSC_ENT_LEN, features);
    }
    if (e[IVSC_ENT_WIDTH] != IVS_IVSC_WIDTH_BITS) {
      return ivsc_refuse(rep, IVS_E_MALFORMED, IVS_IVSC_R_SEGMENT_WIDTH, i, at + IVSC_ENT_WIDTH, features);
    }
    if (e[IVSC_ENT_ATTR] != 0u) {
      return ivsc_refuse(rep, IVS_E_MALFORMED, IVS_IVSC_R_SEGMENT_ATTR, i, at + IVSC_ENT_ATTR, features);
    }
    if (ivsc_be16(e + IVSC_ENT_RESERVED) != 0u) {
      return ivsc_refuse(rep, IVS_E_MALFORMED, IVS_IVSC_R_SEGMENT_RESERVED, i, at + IVSC_ENT_RESERVED, features);
    }
    if ((uint64_t)addr + (uint64_t)len_words > wrap_limit) {
      return ivsc_refuse(rep, IVS_E_OUT_OF_RANGE, IVS_IVSC_R_ADDR_WRAP, i, at + IVSC_ENT_ADDR, features);
    }
    v.segs[i].addr = (uint16_t)addr;
    v.segs[i].len_words = (uint16_t)len_words;
    v.segs[i].data_byte_offset = (uint32_t)(data_at + 2u * words);
    words += (uint64_t)len_words;
  }
  diff = (uint32_t)(data_at + 2u * words + IVS_IVSC_TRAILER_BYTES != total);
#ifdef IVS_FAULT_IVSC_TRUST_TOTAL_BYTES
  diff = (uint32_t)(data_at + 2u * words + IVS_IVSC_TRAILER_BYTES > total);
#endif
  if (diff != 0u) {
    return ivsc_refuse(rep, IVS_E_MALFORMED, IVS_IVSC_R_SIZE_MISMATCH, IVS_IVSC_NO_SEGMENT, IVSC_OFF_TOTAL, features);
  }

  /* 6. Order and overlap, pair by pair in table order: a start below the previous start is an
   * order defect; a start below the previous end is an overlap. A start equal to the previous
   * end (adjacent segments) is accepted. Ascending starts with each pair disjoint make every
   * pair disjoint. */
  for (i = 1u; i < (uint32_t)n_seg; i++) {
    uint64_t at = IVS_IVSC_PREFIX_BYTES + (uint64_t)i * IVS_IVSC_ENTRY_BYTES;
    uint64_t start = v.segs[i].addr;
    uint64_t prev_start = v.segs[i - 1u].addr;
    uint64_t prev_end = prev_start + (uint64_t)v.segs[i - 1u].len_words;
#ifdef IVS_FAULT_IVSC_ADJACENT_AS_OVERLAP
    prev_end += 1u;
#endif
    if (start < prev_start) {
      return ivsc_refuse(rep, IVS_E_MALFORMED, IVS_IVSC_R_ORDER, i, at, features);
    }
    if (start < prev_end) {
      return ivsc_refuse(rep, IVS_E_OVERLAP, IVS_IVSC_R_OVERLAP, i, at, features);
    }
  }

  /* 7. Digest: SHA-256 of every byte before the trailer, compared over all 32 bytes. The same
   * pass gives the whole-file digest (A-04): a copy of the context is finished for the trailer
   * comparison, and the original goes on over the 32 trailer bytes. */
  ivs_sha256_init(&ctx);
  ivs_sha256_update(&ctx, bytes, (size_t)(total - IVS_IVSC_TRAILER_BYTES));
  trail = ctx;
  ivs_sha256_final(&trail, digest);
  ivs_sha256_update(&ctx, bytes + (size_t)(total - IVS_IVSC_TRAILER_BYTES), IVS_IVSC_TRAILER_BYTES);
  ivs_sha256_final(&ctx, file_digest);
  diff = 0u;
  for (k = 0u; k < IVS_SHA256_DIGEST_BYTES; k++) {
    diff |= (uint32_t)(digest[k] ^ bytes[(size_t)(total - IVS_IVSC_TRAILER_BYTES) + k]);
  }
#ifdef IVS_FAULT_IVSC_SKIP_DIGEST
  diff = 0u;
#endif
  if (diff != 0u) {
    return ivsc_refuse(rep, IVS_E_DIGEST_MISMATCH, IVS_IVSC_R_DIGEST, IVS_IVSC_NO_SEGMENT,
                       total - IVS_IVSC_TRAILER_BYTES, features);
  }

  /* 8. Word range: a word above 0x3FF is refused, never masked; the first one is reported. */
  word_max = IVS_IVSC_WORD_MAX;
#ifdef IVS_FAULT_IVSC_ACCEPT_WORD_400
  word_max = IVS_IVSC_WORD_MAX + 1u;
#endif
  for (i = 0u; i < (uint32_t)n_seg; i++) {
    uint64_t at = v.segs[i].data_byte_offset;
    for (k = 0u; k < (uint32_t)v.segs[i].len_words; k++) {
      if (ivsc_be16(bytes + (size_t)(at + 2u * (uint64_t)k)) > word_max) {
        return ivsc_refuse(rep, IVS_E_MALFORMED, IVS_IVSC_R_WORD_RANGE, i, at + 2u * (uint64_t)k, features);
      }
    }
  }

  v.version = (uint16_t)IVS_IVSC_VERSION;
  v.flags = 0u;
  v.required_features = features;
  v.total_bytes = (uint32_t)total;
  v.segment_count = (uint32_t)n_seg;
  memcpy(v.file_sha256, file_digest, sizeof v.file_sha256);
  *view = v;
  return IVS_OK;
}
