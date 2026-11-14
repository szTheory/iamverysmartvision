/* SPDX-License-Identifier: MIT */
/* Public snapshot API (plan 08-11, D-12, D-13, D-14; the container is specified in docs/formats/ivsn-v1.md). A snapshot is the state of
 * one instance at a unit boundary of a powered machine, written as bytes into a buffer the host owns. The core allocates nothing,
 * reads no clock and opens no file: it takes no path, no name and no slot, and the host decides where the bytes go, if anywhere
 * (D-19). Describe and restore are added by later plans of the same phase with their own call records.
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

/* Out: what is known about a snapshot, by the calls that read one. The size and save calls below do not fill it; it is defined here
 * so that the layout is pinned before the calls that use it exist. All members are u32, u64, bytes or the image report of
 * ivs/ivs_machine.h, so the struct has no pointer and no padding. Offsets in bytes:
 *   struct_size            0  u32
 *   reason                 4  u32, an IVS_REASON_ number of a refusal, 0 for none
 *   format_version         8  u32, the version field of the snapshot when it was read
 *   schema_version        12  u32, the schema version field of the snapshot when it was read
 *   total_bytes           16  u64, the total length the snapshot states
 *   byte_offset           24  u64, where in the snapshot a defect was seen
 *   section_index         32  u32, the section a defect concerns, or 0xFFFFFFFF
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

#ifdef __cplusplus
}
#endif

#endif
