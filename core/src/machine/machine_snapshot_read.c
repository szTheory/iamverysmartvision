/* SPDX-License-Identifier: MIT */
/* The snapshot reader (plan 08-12, D-13, D-14, D-15, D-16, docs/formats/ivsn-v1.md "Check order" and "Refusal reasons"): turns the bytes of
 * an IVSN v1 container into a validated view of its header and section table without touching any instance, and answers the public
 * instance-free call ivs_snapshot_describe. This is layer 1 of the container, in the sense of core/src/content/ivsc.c against
 * machine_load.c: restore (a later plan) adds the layer that judges what the snapshot means for one instance, on top of this reader.
 *
 * Contract of ivs_snapshot_read: the container is only read, the view is the only thing written, every length and offset is checked
 * against the supplied length before the byte it names is read, a multi-byte field is assembled from single bytes (big-endian, the one
 * wire order of the project), every sum is a 64-bit value and no pointer into the buffer outlives the call. No allocation, no host
 * service, integer only.
 *
 * Check order (the first failing class wins; this is the order of the specification, step by step, and the number is the reason):
 *    2  length    above IVS_SNAPSHOT_MAX_BYTES: OVERSIZED 60, before any byte of the buffer is read; below the smallest container (the
 *                 header and the trailer, 644 bytes): MALFORMED 61
 *    3  header    magic 62 (the offset of the first differing byte), format version 63, flags 64, schema version 65, schema digest 66
 *                 (the last four UNSUPPORTED), the total the container states against the length (above it: 61; below it, or different
 *                 from the size this build derives: 67), the section count 68
 *    4  table     per entry in order: the domain is the one the table of this build has at that position (an unknown, repeated, skipped
 *                 or descending domain is not), the three bytes after it are zero, the offset is the end of the entry before (612 for the
 *                 first, so sections are adjacent), the length reaches no further than the trailer; after the last entry the end of the
 *                 last section is 32 bytes before the total: MALFORMED 69 with the index of the entry
 *    5  trailer   the SHA-256 of every earlier byte equals the last 32 bytes: DIGEST_MISMATCH 70
 *    6a form      the profile id: printable ASCII up to a NUL inside the 64 bytes, and zero after it: MALFORMED 71. The id is not looked up
 *                 (only restore consults the registry), and an empty or unregistered id of good form is accepted here.
 * The number of the step is the number of docs/formats/ivsn-v1.md. The section lengths are not compared with the lengths derived from
 * the table of this build here: that is the check of restore (reason 80, step 8), and the total (step 3) already ties the sum.
 *
 * Offsets reported with a refusal: the first differing byte for the magic; the supplied length for a truncation; the bound for OVERSIZED;
 * the offset of the field that failed for every other header reason (8, 10, 16, 24, 12, 20); for the table the offset of the field of
 * the entry that failed (the domain at the entry start, the first nonzero zero byte, the offset field at +4, the length field at +8; for
 * the end of the last section the length field of the last entry); the start of the trailer for 70; the first offending byte of the id
 * for 71, or the last byte of the field when the id has no NUL.
 *
 * Named faults (plan 08-12, suite state): SNAPSHOT_LENGTH_TRUSTED (the claimed total above the supplied length is not refused, so the
 * trailer is read at the claim) and SNAPSHOT_TRAILER_UNCHECKED (the trailer comparison is skipped). One site each, below. */
#include "common/ivs_prelude.h"

#include "common/ivs_sha256.h"
#include "ivs/ivs.h"
#include "machine/machine.h"

/* 0x89, IVSN, CR LF SUB: a high-bit first byte and a CR LF pair expose 7-bit and text-mode transfers. */
static const uint8_t ivs_snap_magic[8] = {0x89u, 0x49u, 0x56u, 0x53u, 0x4Eu, 0x0Du, 0x0Au, 0x1Au};

#define IVS_SNAP_MIN_BYTES (IVS_SNAP_HEADER_BYTES + IVS_SNAP_TRAILER_BYTES)

static uint32_t ivs_snap_be16(const uint8_t *p)
{
  return ((uint32_t)p[0] << 8) | (uint32_t)p[1];
}

static uint32_t ivs_snap_be32(const uint8_t *p)
{
  return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

/* The OR of the byte differences of two n-byte runs: 0 when they are equal (every byte is looked at). */
static uint32_t ivs_snap_diff(const uint8_t *a, const uint8_t *b, size_t n)
{
  uint32_t d = 0u;
  size_t i;
  for (i = 0u; i < n; i++) {
    d |= (uint32_t)(a[i] ^ b[i]);
  }
  return d;
}

/* The one refusal helper: fills the refusal part of the view and returns the status. */
static ivs_status ivs_snap_refuse(ivs_snap_view *v, ivs_status st, uint32_t reason, uint32_t section, uint64_t off)
{
  v->status = st;
  v->reason = reason;
  v->section_index = section;
  v->byte_offset = off;
  return st;
}

/* The form of the profile id field (step 6a): 0 for a good form, else the offset of the first offending byte; *len is the number of
 * bytes before the NUL. No byte of the field is looked at twice and the scan stops inside the 64 bytes. */
static uint64_t ivs_snap_id_form(const uint8_t *id, uint32_t *len)
{
  uint32_t k = 0u;
  uint32_t j;
  while (k < IVS_PROFILE_ID_MAX_BYTES && id[k] != 0u) {
    if (id[k] < 0x20u || id[k] > 0x7Eu) {
      return (uint64_t)IVS_SNAP_OFF_PROFILE_ID + k;
    }
    k++;
  }
  if (k == IVS_PROFILE_ID_MAX_BYTES) {
    return (uint64_t)IVS_SNAP_OFF_PROFILE_ID + IVS_PROFILE_ID_MAX_BYTES - 1u; /* no NUL: the last byte should have been one */
  }
  for (j = k + 1u; j < IVS_PROFILE_ID_MAX_BYTES; j++) {
    if (id[j] != 0u) {
      return (uint64_t)IVS_SNAP_OFF_PROFILE_ID + j;
    }
  }
  *len = k;
  return 0u;
}

ivs_status ivs_snapshot_read(const uint8_t *buf, uint64_t len, ivs_snap_view *v)
{
  uint8_t want[32];
  uint8_t got[32];
  uint64_t total;
  uint64_t payload_end;
  uint64_t expect_off;
  uint64_t bad;
  uint32_t id_len = 0u;
  uint32_t i;
  uint32_t k;
  memset(v, 0, sizeof *v);
  v->status = IVS_OK;
  v->section_index = IVS_SEGMENT_NONE;

  /* Step 2: the length bound, before any byte of the buffer is read. */
  if (len > IVS_SNAPSHOT_MAX_BYTES) {
    return ivs_snap_refuse(v, IVS_E_OVERSIZED, IVS_REASON_SNAP_OVERSIZED, IVS_SEGMENT_NONE, IVS_SNAPSHOT_MAX_BYTES);
  }
  if (buf == NULL) {
    len = 0u; /* the caller has decided this (ivs_snapshot_arg_reason): a NULL buffer is read as no bytes at all */
  }

  /* What the supplied length lets us read of the header, as the bytes stand: for a host that wants to say who made a snapshot. */
  if (len >= 12u) {
    v->format_version = ivs_snap_be16(buf + IVS_SNAP_OFF_FORMAT);
    v->flags = ivs_snap_be16(buf + IVS_SNAP_OFF_FLAGS);
    v->known |= IVS_SNAP_KNOWN_FORMAT;
  }
  if (len >= 16u) {
    v->total_bytes = (uint64_t)ivs_snap_be32(buf + IVS_SNAP_OFF_TOTAL);
    v->known |= IVS_SNAP_KNOWN_TOTAL;
  }
  if (len >= 24u) {
    v->schema_version = ivs_snap_be32(buf + IVS_SNAP_OFF_SCHEMA);
    v->section_count = ivs_snap_be32(buf + IVS_SNAP_OFF_COUNT);
    v->known |= IVS_SNAP_KNOWN_SCHEMA;
  }
  if (len < IVS_SNAP_MIN_BYTES) {
    return ivs_snap_refuse(v, IVS_E_MALFORMED, IVS_REASON_SNAP_TRUNCATED, IVS_SEGMENT_NONE, len);
  }

  /* Step 3: the header. From here on the buffer holds at least the whole header and a trailer. */
  for (k = 0u; k < sizeof ivs_snap_magic; k++) {
    if (buf[k] != ivs_snap_magic[k]) {
      return ivs_snap_refuse(v, IVS_E_MALFORMED, IVS_REASON_SNAP_MAGIC, IVS_SEGMENT_NONE, k);
    }
  }
  if (v->format_version != IVS_SNAPSHOT_FORMAT_VERSION) {
    return ivs_snap_refuse(v, IVS_E_UNSUPPORTED, IVS_REASON_SNAP_FORMAT_VERSION, IVS_SEGMENT_NONE, IVS_SNAP_OFF_FORMAT);
  }
  if (v->flags != 0u) {
    return ivs_snap_refuse(v, IVS_E_UNSUPPORTED, IVS_REASON_SNAP_FLAGS, IVS_SEGMENT_NONE, IVS_SNAP_OFF_FLAGS);
  }
  if (v->schema_version != IVS_SNAPSHOT_SCHEMA_VERSION) {
    return ivs_snap_refuse(v, IVS_E_UNSUPPORTED, IVS_REASON_SNAP_SCHEMA_VERSION, IVS_SEGMENT_NONE, IVS_SNAP_OFF_SCHEMA);
  }
  ivs_snapshot_schema_digest(want);
  if (ivs_snap_diff(buf + IVS_SNAP_OFF_SCHEMA_DIGEST, want, 32u) != 0u) {
    return ivs_snap_refuse(v, IVS_E_UNSUPPORTED, IVS_REASON_SNAP_SCHEMA_DIGEST, IVS_SEGMENT_NONE, IVS_SNAP_OFF_SCHEMA_DIGEST);
  }
  total = v->total_bytes;
#ifndef IVS_FAULT_SNAPSHOT_LENGTH_TRUSTED
  if (total > len) {
    return ivs_snap_refuse(v, IVS_E_MALFORMED, IVS_REASON_SNAP_TRUNCATED, IVS_SEGMENT_NONE, len);
  }
#endif
  /* Named fault SNAPSHOT_LENGTH_TRUSTED (plan 08-12): the refusal above is compiled out, so a claimed total above the supplied length is
   * believed and the trailer below is read at the claim, past the bytes the caller supplied. */
  if (total < len || total != ivs_snapshot_derived_total()) {
    return ivs_snap_refuse(v, IVS_E_MALFORMED, IVS_REASON_SNAP_TOTAL_LENGTH, IVS_SEGMENT_NONE, IVS_SNAP_OFF_TOTAL);
  }
  if (v->section_count != IVS_SNAP_SECTIONS) {
    return ivs_snap_refuse(v, IVS_E_MALFORMED, IVS_REASON_SNAP_SECTION_COUNT, IVS_SEGMENT_NONE, IVS_SNAP_OFF_COUNT);
  }

  /* Step 4: the section table. The header (612 bytes) lies inside the buffer, so every entry does. */
  payload_end = total - IVS_SNAP_TRAILER_BYTES;
  expect_off = IVS_SNAP_HEADER_BYTES;
  for (i = 0u; i < IVS_SNAP_SECTIONS; i++) {
    uint64_t entry = (uint64_t)IVS_SNAP_OFF_TABLE + (uint64_t)IVS_SNAP_ENTRY_BYTES * i;
    const uint8_t *e = buf + entry;
    uint64_t off;
    uint64_t length;
    if (e[0] != ivs_snap_domain[i]) {
      return ivs_snap_refuse(v, IVS_E_MALFORMED, IVS_REASON_SNAP_SECTION_TABLE, i, entry);
    }
    for (k = 1u; k < 4u; k++) {
      if (e[k] != 0u) {
        return ivs_snap_refuse(v, IVS_E_MALFORMED, IVS_REASON_SNAP_SECTION_TABLE, i, entry + k);
      }
    }
    off = (uint64_t)ivs_snap_be32(e + 4u);
    length = (uint64_t)ivs_snap_be32(e + 8u);
    if (off != expect_off) {
      return ivs_snap_refuse(v, IVS_E_MALFORMED, IVS_REASON_SNAP_SECTION_TABLE, i, entry + 4u);
    }
    /* expect_off is at most payload_end here (it starts below it and only grows by a length that fits), so the subtraction cannot wrap */
    if (length > payload_end - off) {
      return ivs_snap_refuse(v, IVS_E_MALFORMED, IVS_REASON_SNAP_SECTION_TABLE, i, entry + 8u);
    }
    v->section[i].domain = e[0];
    v->section[i].offset = (uint32_t)off;
    v->section[i].length = (uint32_t)length;
    memcpy(v->section[i].digest, e + 12u, 32u);
    expect_off = off + length;
  }
  if (expect_off != payload_end) {
    return ivs_snap_refuse(v, IVS_E_MALFORMED, IVS_REASON_SNAP_SECTION_TABLE, IVS_SNAP_SECTIONS - 1u,
                           (uint64_t)IVS_SNAP_OFF_TABLE + (uint64_t)IVS_SNAP_ENTRY_BYTES * (IVS_SNAP_SECTIONS - 1u) + 8u);
  }

  /* Step 5: the trailer, over every byte before it. total equals the supplied length here (a fault build aside). */
  ivs_sha256_digest(buf, (size_t)payload_end, got);
  bad = ivs_snap_diff(got, buf + payload_end, IVS_SNAP_TRAILER_BYTES);
#ifdef IVS_FAULT_SNAPSHOT_TRAILER_UNCHECKED
  bad = 0u; /* named fault SNAPSHOT_TRAILER_UNCHECKED (plan 08-12): the comparison is skipped, so a damaged container is accepted */
#endif
  if (bad != 0u) {
    return ivs_snap_refuse(v, IVS_E_DIGEST_MISMATCH, IVS_REASON_SNAP_TRAILER, IVS_SEGMENT_NONE, payload_end);
  }

  /* Step 6a: the form of the profile id. */
  bad = ivs_snap_id_form(buf + IVS_SNAP_OFF_PROFILE_ID, &id_len);
  if (bad != 0u) {
    return ivs_snap_refuse(v, IVS_E_MALFORMED, IVS_REASON_SNAP_PROFILE_ID_FORM, IVS_SEGMENT_NONE, bad);
  }

  /* Accepted: the identity fields. */
  v->profile_id_length = id_len;
  memcpy(v->profile_id, buf + IVS_SNAP_OFF_PROFILE_ID, IVS_PROFILE_ID_MAX_BYTES);
  memcpy(v->params_sha256, buf + IVS_SNAP_OFF_PARAMS, 32u);
  memcpy(v->composite_sha256, buf + IVS_SNAP_OFF_COMPOSITE, 32u);
  for (i = 0u; i < IVS_SLOT_COUNT; i++) {
    const uint8_t *r = buf + IVS_SNAP_OFF_SLOTS + (size_t)IVS_SNAP_SLOT_BYTES * i;
    v->slot[i].loaded = ivs_snap_be32(r);
    memcpy(v->slot[i].sha256, r + 4u, 32u);
    v->slot[i].length_bytes = ivs_snap_be32(r + 36u);
  }
  return IVS_OK;
}

uint32_t ivs_snapshot_arg_reason(const uint8_t *buf, uint64_t len, const void *rep, size_t rep_bytes)
{
  uint64_t span = len > IVS_SNAPSHOT_MAX_BYTES ? (uint64_t)IVS_SNAPSHOT_MAX_BYTES : len;
  if (buf == NULL && len > 0u) {
    return IVS_REASON_SNAP_BUF_NULL;
  }
  if (ivs_machine_span_overlaps(buf, (size_t)span, rep, rep_bytes) != 0u) {
    return IVS_REASON_SNAP_REPORT_OVERLAP;
  }
  return 0u;
}

/* A refusal into the report: only the members the contract of ivs/ivs_state.h lists, and the header facts the supplied length let the
 * reader see. Every other member of the report, and struct_size, keep the caller's value. */
static void ivs_snap_report_refusal(ivs_snapshot_report *rep, const ivs_snap_view *v)
{
  rep->reason = v->reason;
  rep->byte_offset = v->byte_offset;
  rep->section_index = v->section_index;
  if ((v->known & IVS_SNAP_KNOWN_FORMAT) != 0u) {
    rep->format_version = v->format_version;
  }
  if ((v->known & IVS_SNAP_KNOWN_TOTAL) != 0u) {
    rep->total_bytes = v->total_bytes;
  }
  if ((v->known & IVS_SNAP_KNOWN_SCHEMA) != 0u) {
    rep->schema_version = v->schema_version;
  }
}

ivs_status ivs_snapshot_describe(const uint8_t *buf, uint64_t len, ivs_snapshot_report *rep)
{
  ivs_snap_view v;
  ivs_snapshot_report r;
  ivs_status st;
  uint32_t reason;
  uint32_t i;
  if (rep == NULL || rep->struct_size < IVS_SNAPSHOT_REPORT_V1_SIZE || rep->struct_size > IVS_STRUCT_SIZE_MAX) {
    return IVS_E_BAD_ARG; /* an unusable report: nothing is written */
  }
  reason = ivs_snapshot_arg_reason(buf, len, rep, IVS_SNAPSHOT_REPORT_V1_SIZE);
  if (reason != 0u) {
    /* the decision to refuse is made: the report may now be written even where it overlaps the buffer, which is not read any more */
    memset(&v, 0, sizeof v);
    v.reason = reason;
    v.section_index = IVS_SEGMENT_NONE;
    ivs_snap_report_refusal(rep, &v);
    return IVS_E_BAD_ARG;
  }
  st = ivs_snapshot_read(buf, len, &v);
  if (st != IVS_OK) {
    ivs_snap_report_refusal(rep, &v);
    return st;
  }
  memset(&r, 0, sizeof r);
  r.struct_size = IVS_SNAPSHOT_REPORT_V1_SIZE;
  r.reason = IVS_REASON_NONE;
  r.format_version = v.format_version;
  r.schema_version = v.schema_version;
  r.total_bytes = v.total_bytes;
  r.byte_offset = 0u;
  r.section_index = IVS_SEGMENT_NONE;
  r.kind = 0u;
  memcpy(r.profile_id, v.profile_id, IVS_PROFILE_ID_MAX_BYTES);
  memcpy(r.profile_params_sha256, v.params_sha256, 32u);
  memcpy(r.composite_sha256, v.composite_sha256, 32u);
  for (i = 0u; i < IVS_SLOT_COUNT; i++) {
    r.slots[i].struct_size = (uint32_t)sizeof r.slots[i];
    r.slots[i].kind = i + 1u;
    r.slots[i].loaded = v.slot[i].loaded;
    r.slots[i].length_bytes = v.slot[i].length_bytes;
    memcpy(r.slots[i].sha256, v.slot[i].sha256, 32u);
  }
  memcpy(rep, &r, IVS_SNAPSHOT_REPORT_V1_SIZE);
  return IVS_OK;
}
