/* SPDX-License-Identifier: MIT */
/* Public snapshot API (plan 08-11, D-12, D-13, D-14; the container is specified in docs/formats/ivsn-v1.md). A snapshot is the state of
 * one instance at a unit boundary of a powered machine, written as bytes into a buffer the host owns. The core allocates nothing,
 * reads no clock and opens no file: it takes no path, no name and no slot, and the host decides where the bytes go, if anywhere
 * (D-19). The instance-free describe call is here (plan 08-12) and so is restore (plan 08-13), the one call of this header that writes
 * an instance.
 *
 * What a snapshot holds: the sections of the machine state in the one wire byte order of the project, big-endian, assembled field
 * by field, so the bytes do not depend on the byte order of the host, on the padding of a struct or on the layout of the instance;
 * the name and the parameter digest of the profile; the identity of the three image slots; two digests of the schema of the rows;
 * and a SHA-256 trailer over every earlier byte. What it never holds: an image byte. The three image arrays are not written, only
 * the SHA-256 of the container that was loaded into each slot and its length. A hash establishes identity, not rights; a snapshot
 * of a run with images the host supplied is still derived material of those images and the host treats it so.
 * What the format does not claim: it is state of one build and one profile, not an archival format and not portable to another
 * build; the trailer finds accidental damage, it does not authenticate the writer; and it makes no statement about hardware or about
 * what a guest program means.
 *
 * Struct rule of ivs/ivs_machine.h: an in struct starts with uint32_t struct_size; a size below the v1 size or above
 * IVS_STRUCT_SIZE_MAX is BAD_ARG. For an out struct a size below the v1 size is BAD_ARG with nothing written; otherwise the core
 * writes the smaller of the caller size and its own size in bytes and sets struct_size to that count. The report below is such an out
 * struct. Lengths and capacities are 64-bit values, so a host on a 32-bit platform and one on a 64-bit platform read the same
 * contract.
 * Standard headers only; constants are #defines (no enums, bool or bitfields). */
#ifndef IVS_STATE_H
#define IVS_STATE_H

#include <stdint.h>

#include "ivs/ivs_machine.h"
#include "ivs/ivs_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The format version and the schema version of the container this build writes. A build reads and writes exactly one pair. */
#define IVS_SNAPSHOT_FORMAT_VERSION 1u
#define IVS_SNAPSHOT_SCHEMA_VERSION 1u

/* The largest length of a snapshot a call of this API accepts as input: a longer one is refused before any byte is read. The first
 * profile writes 4,299 bytes; the bound leaves room for later sections. */
#define IVS_SNAPSHOT_MAX_BYTES 16384u

/* The v1 size of ivs_snapshot_report, the smallest struct_size the calls that fill it accept (append-only growth). */
#define IVS_SNAPSHOT_REPORT_V1_SIZE 360u

/* Refusal reasons of the calls that read a snapshot (plan 08-12, D-14; ivs_snapshot_report.reason). Each reason travels with the one
 * status named in its comment. The numbers continue the list of ivs/ivs_machine.h, are pinned in data/expected/api-reasons.ids and are
 * append-only. The section of docs/formats/ivsn-v1.md on the refusal reasons gives each meaning; the reasons from 72 on are those of
 * restore (plan 08-13), which judges what a snapshot means for one instance. */
#define IVS_REASON_SNAP_BUF_NULL 58u        /* BAD_ARG: a NULL buffer with a length above 0 */
#define IVS_REASON_SNAP_REPORT_OVERLAP 59u  /* BAD_ARG: the report overlaps the buffer */
#define IVS_REASON_SNAP_OVERSIZED 60u       /* OVERSIZED: the length is above IVS_SNAPSHOT_MAX_BYTES, refused before any byte is read */
#define IVS_REASON_SNAP_TRUNCATED 61u       /* MALFORMED: the length is below the smallest container, or the total it states is above it */
#define IVS_REASON_SNAP_MAGIC 62u           /* MALFORMED: the magic differs; the byte offset of the first difference is reported */
#define IVS_REASON_SNAP_FORMAT_VERSION 63u  /* UNSUPPORTED: a format version other than IVS_SNAPSHOT_FORMAT_VERSION */
#define IVS_REASON_SNAP_FLAGS 64u           /* UNSUPPORTED: a flag bit is set */
#define IVS_REASON_SNAP_SCHEMA_VERSION 65u  /* UNSUPPORTED: a schema version other than IVS_SNAPSHOT_SCHEMA_VERSION */
#define IVS_REASON_SNAP_SCHEMA_DIGEST 66u   /* UNSUPPORTED: the schema digest differs from the one of this build */
#define IVS_REASON_SNAP_TOTAL_LENGTH 67u    /* MALFORMED: the total it states is below the length or differs from the size this build derives */
#define IVS_REASON_SNAP_SECTION_COUNT 68u   /* MALFORMED: a section count other than 7 */
#define IVS_REASON_SNAP_SECTION_TABLE 69u   /* MALFORMED: an unknown, duplicate or out of order domain, or an offset or length that does not fit */
#define IVS_REASON_SNAP_TRAILER 70u         /* DIGEST_MISMATCH: the trailer differs from the SHA-256 of every earlier byte */
#define IVS_REASON_SNAP_PROFILE_ID_FORM 71u /* MALFORMED: the profile id has no NUL, a byte after the NUL, or a byte that is not printable ASCII */
#define IVS_REASON_SNAP_UNKNOWN_PROFILE 72u      /* UNKNOWN_PROFILE: the profile id of the snapshot is not registered in this build */
#define IVS_REASON_SNAP_PROFILE_MISMATCH 73u     /* STATE_MISMATCH: the profile id is registered and differs from the one of the instance */
#define IVS_REASON_SNAP_PROFILE_PARAMS 74u       /* STATE_MISMATCH: the parameter digest of the snapshot differs from the one of this build */
#define IVS_REASON_SNAP_SLOT_MISSING 75u         /* MISSING_SYSTEM_IMAGE: a slot the snapshot has loaded is empty in the instance */
#define IVS_REASON_SNAP_SLOT_MISMATCH 76u        /* STATE_MISMATCH: a slot is loaded on both sides with another identity, or loaded only in the instance */
#define IVS_REASON_SNAP_ROW_COUNT 77u            /* MALFORMED: the count of an array row differs from the count of the table */
#define IVS_REASON_SNAP_FIELD_RANGE 78u          /* MALFORMED: a decoded value fails its range or the validator of its device */
#define IVS_REASON_SNAP_INVARIANT 79u            /* MALFORMED: a derived invariant of the state does not hold */
#define IVS_REASON_SNAP_SECTION_LENGTH 80u       /* MALFORMED: a section length differs from the length this build derives for it */
#define IVS_REASON_SNAP_ARG_OVERLAP_INSTANCE 81u /* BAD_ARG: the buffer or the report overlaps the storage of the instance */

/* Out: what is known about a snapshot, by the calls that read one (ivs_snapshot_describe below). The size and save calls do not fill
 * it. All members are u32, u64, bytes or the image report of ivs/ivs_machine.h, so the struct has no pointer and no padding. Offsets
 * in bytes:
 *   struct_size            0  u32
 *   reason                 4  u32, an IVS_REASON_ number of a refusal, 0 for none
 *   format_version         8  u32, the version field of the snapshot when it was read
 *   schema_version        12  u32, the schema version field of the snapshot when it was read
 *   total_bytes           16  u64, the total length the snapshot states
 *   byte_offset           24  u64, where in the snapshot a defect was seen
 *   section_index         32  u32, the section table entry a defect concerns, or 0xFFFFFFFF (the value of IVS_SEGMENT_NONE)
 *   kind                  36  u32, an image slot kind a defect concerns, or 0
 *   profile_id            40  64 bytes, the profile id of the snapshot, NUL padded
 *   profile_params_sha256 104  32 bytes, the parameter digest of the profile tables of the writer
 *   composite_sha256     136  32 bytes, the composite state digest at the time of the snapshot
 *   slots                168  three image reports, slot of kind i + 1 at index i: loaded, length, SHA-256 of the loaded container
 * The v1 size is 360 bytes. */
typedef struct ivs_snapshot_report {
  uint32_t struct_size;
  uint32_t reason;
  uint32_t format_version;
  uint32_t schema_version;
  uint64_t total_bytes;
  uint64_t byte_offset;
  uint32_t section_index;
  uint32_t kind;
  char profile_id[IVS_PROFILE_ID_MAX_BYTES];
  uint8_t profile_params_sha256[32];
  uint8_t composite_sha256[32];
  ivs_image_report slots[3];
} ivs_snapshot_report;

/* The exact length in bytes of a snapshot of m, written to *out_bytes. The length is the same for every powered instance of one
 * profile and one build, so a host never needs to hard-code it. The instance is not changed and nothing is allocated.
 * The first failing class wins:
 *   BAD_ARG       out_bytes is NULL;
 *   BAD_HANDLE    a NULL or invalid handle;
 *   BAD_ARG       out_bytes overlaps the storage of the valid instance;
 *   BAD_STATE     the machine is OFF: a snapshot point is a unit boundary of a powered machine.
 * A refusal writes nothing, so *out_bytes keeps its value. */
IVS_API ivs_status ivs_machine_snapshot_size(const ivs_machine *m, uint64_t *out_bytes);

/* Writes the snapshot of m into the cap bytes at buf and its length to *out_written. The instance is not changed, machine time does
 * not advance, nothing is allocated and the core touches no file or clock. The bytes depend on the state of the instance and on
 * nothing else, so two saves of one instance are byte-identical. No image byte is written. The first failing class wins:
 *   BAD_ARG            out_written is NULL; buf is NULL with cap above 0; or out_written lies inside the cap bytes at buf;
 *   BAD_HANDLE         a NULL or invalid handle;
 *   BAD_ARG            out_written or the cap bytes at buf overlap the storage of the valid instance, or the cap bytes run past
 *                      the top of the address space;
 *   BAD_STATE          the machine is OFF;
 *   BUFFER_TOO_SMALL   cap is below the exact length: *out_written is set to that length, so a call with cap 0 asks for the size.
 * Only the refusal BUFFER_TOO_SMALL writes anything, and only *out_written: no refusal writes a byte of buf. On success exactly
 * *out_written bytes of buf are written and no byte past them. */
IVS_API ivs_status ivs_machine_snapshot_save(const ivs_machine *m, uint8_t *buf, uint64_t cap, uint64_t *out_written);

/* What the len bytes at buf claim to be, without an instance (plan 08-12, D-13, D-14): checks the container in the order of the
 * specification up to and including the SHA-256 trailer and the form of the profile id, never reads a byte past len and never writes
 * anything but the report. A host calls it to learn what a snapshot is, for example that another version made it, before it has an
 * instance to restore into. It does not say that the snapshot restores into any machine: the profile, the images and the content are
 * judged by restore. The first failing class wins:
 *   BAD_ARG           rep is NULL, or its struct_size is below IVS_SNAPSHOT_REPORT_V1_SIZE or above IVS_STRUCT_SIZE_MAX;
 *   BAD_ARG           buf is NULL with len above 0 (IVS_REASON_SNAP_BUF_NULL);
 *   BAD_ARG           the report overlaps the buffer (IVS_REASON_SNAP_REPORT_OVERLAP); a len above IVS_SNAPSHOT_MAX_BYTES counts as
 *                     IVS_SNAPSHOT_MAX_BYTES for this test, since no byte past that bound is read;
 *   OVERSIZED         len is above IVS_SNAPSHOT_MAX_BYTES, refused before any byte is read (IVS_REASON_SNAP_OVERSIZED);
 *   MALFORMED         the length, the magic, the total it states, the section count, the section table or the profile id has a defect
 *                     (IVS_REASON_SNAP_TRUNCATED, _MAGIC, _TOTAL_LENGTH, _SECTION_COUNT, _SECTION_TABLE, _PROFILE_ID_FORM);
 *   UNSUPPORTED       the format version, the flags, the schema version or the schema digest is not the one of this build
 *                     (IVS_REASON_SNAP_FORMAT_VERSION, _FLAGS, _SCHEMA_VERSION, _SCHEMA_DIGEST);
 *   DIGEST_MISMATCH   the trailer differs from the SHA-256 of every earlier byte (IVS_REASON_SNAP_TRAILER).
 * The trailer is checked after the structure, so a defect of the structure with a wrong trailer is reported as the defect of the
 * structure, and the form of the profile id is checked after the trailer. The reader looks at the first IVS_SNAPSHOT_MAX_BYTES bytes
 * at most. The report: a refusal by an unusable report struct writes nothing. Every other refusal writes only these members of the
 * report. reason. byte_offset, the offset of the defect: the length for a truncation, 0 for a refusal by an argument,
 * IVS_SNAPSHOT_MAX_BYTES for OVERSIZED, the offset of the first differing byte for the magic and otherwise the offset of the field that
 * failed. section_index, the table entry that a section table defect concerns and otherwise 0xFFFFFFFF. And, only when len let the bytes
 * be read, format_version when len is at least 12, total_bytes (the total the snapshot states) when len is at least 16 and
 * schema_version when len is at least 24; these are read from the bytes as they stand, whatever the defect, so a host can say that
 * another version made a snapshot. Every other member keeps the value the caller gave it, and so does struct_size. On success the
 * report is written in full: the smaller of struct_size and the v1 size in bytes, with struct_size set to that count. It holds
 * reason 0, the two versions, total_bytes, byte_offset 0, section_index 0xFFFFFFFF, kind 0, the profile id, the parameter digest and
 * the composite digest of the writer, and the three image slot records (struct_size the size of an image report, kind the slot index
 * plus one, loaded, length_bytes and sha256 as the snapshot states them, every other member 0). Nothing in the report says anything
 * about any instance. */
IVS_API ivs_status ivs_snapshot_describe(const uint8_t *buf, uint64_t len, ivs_snapshot_report *rep);

/* Restores the snapshot in the len bytes at buf into the instance m (plan 08-13, D-14, D-15, D-41): the state of the instance becomes the
 * state the snapshot holds, and the machine is POWERED, whatever it was before (an OFF instance with its images loaded is the normal
 * fresh target, a POWERED one is rewound). The core validates everything before it writes the first byte of the instance and then
 * commits with copies that cannot fail, so a refusal leaves every byte of the instance as it was and writes only the report. A snapshot
 * belongs to one profile, one build and the three images it was taken with: the images are never part of a snapshot, so the host loads
 * the same images into the target first. Restore never writes the handle, the profile of the instance, the three image slots or the
 * image arrays. Nothing is allocated, no clock is read and no file is opened. The host keeps what is outside the instance: its output
 * buffers, and the input events, which it resubmits from the count of events the restored instance has applied.
 * The first failing class wins, in this order:
 *   BAD_ARG                rep is not NULL and its struct_size is below IVS_SNAPSHOT_REPORT_V1_SIZE or above IVS_STRUCT_SIZE_MAX, and
 *                          nothing is written;
 *   BAD_ARG                buf is NULL with len above 0 (IVS_REASON_SNAP_BUF_NULL), or the report overlaps the buffer
 *                          (IVS_REASON_SNAP_REPORT_OVERLAP), judged over the first IVS_SNAPSHOT_MAX_BYTES bytes at most;
 *   BAD_HANDLE             a NULL or invalid handle;
 *   BAD_ARG                the buffer or the report overlaps the storage of the valid instance (IVS_REASON_SNAP_ARG_OVERLAP_INSTANCE);
 *   the container          exactly the checks of ivs_snapshot_describe, in its order: OVERSIZED, MALFORMED, UNSUPPORTED and
 *                          DIGEST_MISMATCH with the reasons IVS_REASON_SNAP_OVERSIZED to IVS_REASON_SNAP_PROFILE_ID_FORM;
 *   UNKNOWN_PROFILE        the profile id is not registered in this build (IVS_REASON_SNAP_UNKNOWN_PROFILE);
 *   STATE_MISMATCH         the id differs from the one of the instance (IVS_REASON_SNAP_PROFILE_MISMATCH), or the parameter digest of the
 *                          profile tables differs from the one of this build (IVS_REASON_SNAP_PROFILE_PARAMS);
 *   per image slot         in the order EXEC, GROM, cartridge: a loaded flag above 1, or an empty slot record that holds a length or a
 *                          digest, is MALFORMED (IVS_REASON_SNAP_FIELD_RANGE); a slot the snapshot has loaded and the instance has not is
 *                          MISSING_SYSTEM_IMAGE (IVS_REASON_SNAP_SLOT_MISSING); a slot loaded only in the instance, or loaded on both
 *                          sides with another SHA-256 or length, is STATE_MISMATCH (IVS_REASON_SNAP_SLOT_MISMATCH);
 *   MALFORMED              a section length that differs from the derived one (IVS_REASON_SNAP_SECTION_LENGTH), then a row count that
 *                          differs from the table (IVS_REASON_SNAP_ROW_COUNT), then a value out of range, a section frame that is not
 *                          its frame, or a CPU, STIC or PSG state its validator refuses, a phase other than POWERED or a machine time
 *                          above 2 to the power 63 (IVS_REASON_SNAP_FIELD_RANGE), then a derived invariant that does not hold
 *                          (IVS_REASON_SNAP_INVARIANT).
 * There is no BAD_STATE: restore is allowed in every phase. The digest fields of the header (the composite and the seven section digests)
 * are informational and not checked, and the trailer is a checksum, never a proof of who wrote the snapshot: a snapshot whose trailer was
 * recomputed is judged by the checks above, not by the checksum.
 * The report is optional: rep may be NULL. A refusal writes the members of ivs_snapshot_report as ivs_snapshot_describe does (reason,
 * byte_offset, section_index and the header facts the length let it read, struct_size and every other member kept) and, for a refusal
 * about an image slot, kind as well: the kind of the slot, 1 for EXEC, 2 for GROM and 3 for the cartridge, with the byte offset of the
 * field of that slot record that failed. The byte offset of a refusal about the content is the offset in the snapshot of the first byte of
 * the field, the array count or the section that failed. A refusal by an unusable report struct, and a refusal while the report overlaps
 * the instance, write nothing. A refusal by an invalid handle writes the reason IVS_REASON_HANDLE. On success the report is written in
 * full as describe writes it. */
IVS_API ivs_status ivs_machine_snapshot_restore(ivs_machine *m, const uint8_t *buf, uint64_t len, ivs_snapshot_report *rep);

#ifdef __cplusplus
}
#endif

#endif
