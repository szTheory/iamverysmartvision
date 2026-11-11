/* SPDX-License-Identifier: MIT */
/* Public machine API (D-01..D-08, A-08 f, plan 03-09). A host sizes an instance for a profile,
 * creates it in storage the host owns, queries it and destroys it. The core never allocates and
 * takes no allocator, clock, seed, path or callback (D-01, D-03). The instance type is
 * incomplete here; its layout is internal and not an ABI (D-02).
 *
 * Struct rule (D-04): every struct passed across this API starts with uint32_t struct_size.
 *   - in structs: a size below the v1 size is BAD_ARG; a larger size is accepted only when every
 *     byte the core does not know is zero (else UNSUPPORTED); nonzero flags are UNSUPPORTED;
 *   - out structs: a size below the v1 size is BAD_ARG with nothing written; otherwise the core
 *     writes min(caller size, own size) bytes and sets struct_size to the size it wrote.
 * A refused call writes only its out parameters (D-08). A NULL or invalid handle is BAD_HANDLE
 * for every call that takes one; argument refusals (BAD_ARG) are checked first.
 * Distinct instances are independent; one instance must be serialized by the host (D-07).
 * Standard headers only; constants are #defines (no enums, bool or bitfields). */
#ifndef IVS_MACHINE_H
#define IVS_MACHINE_H

#include <stddef.h>
#include <stdint.h>

#include "ivs/ivs_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The instance handle (D-01): its value is the address of the host's storage. */
typedef struct ivs_machine ivs_machine;

/* Image kinds, one slot each (D-25). */
#define IVS_KIND_EXEC 1u
#define IVS_KIND_GROM 2u
#define IVS_KIND_CARTRIDGE 3u
#define IVS_SLOT_COUNT 3u

/* Run phases (D-13, D-14): a new instance is OFF; a successful power-on reset makes it POWERED. */
#define IVS_PHASE_OFF 0u
#define IVS_PHASE_POWERED 1u

/* State digest domains (D-35). 0 is the composite of 1..4; 16..18 are reserved for the RUN-05
 * video, audio and input observations and are refused until they exist. */
#define IVS_DIGEST_COMPOSITE 0u
#define IVS_DIGEST_CPU 1u
#define IVS_DIGEST_MEM 2u
#define IVS_DIGEST_IMAGES 3u
#define IVS_DIGEST_INSTANCE 4u
#define IVS_DIGEST_RESERVED_VIDEO 16u
#define IVS_DIGEST_RESERVED_AUDIO 17u
#define IVS_DIGEST_RESERVED_INPUT 18u

/* Bound on a profile id: at most 63 ASCII bytes and a NUL within the first 64 bytes. */
#define IVS_PROFILE_ID_MAX_BYTES 64u

/* Out: storage size and alignment for one instance of a profile. */
typedef struct ivs_storage_requirements {
  uint32_t struct_size;
  uint32_t reserved; /* written 0 */
  uint64_t size_bytes;
  uint64_t align_bytes;
} ivs_storage_requirements;

/* In: create options. flags must be 0. profile_id is an exact, case-sensitive registered id. */
typedef struct ivs_create_options {
  uint32_t struct_size;
  uint32_t flags;
  const char *profile_id;
} ivs_create_options;

/* Out: one image slot (D-25). A slot with no image reports loaded 0, zero lengths and a zero
 * digest. sha256 is the digest of the exact container bytes supplied to the load. */
typedef struct ivs_image_report {
  uint32_t struct_size;
  uint32_t kind; /* IVS_KIND_* */
  uint32_t loaded; /* 0 or 1 */
  uint32_t length_bytes;
  uint32_t length_units;
  uint32_t unit_bits;
  uint32_t base_addr;
  uint32_t reserved; /* written 0 */
  uint8_t sha256[32];
} ivs_image_report;

/* Out: instance information. slots[i] is the slot of kind i + 1. time_ticks is machine time in
 * master ticks (docs/time-base.md). */
typedef struct ivs_info {
  uint32_t struct_size;
  uint32_t phase; /* IVS_PHASE_* */
  uint64_t time_ticks;
  uint32_t profile_index;
  uint32_t reserved; /* written 0 */
  ivs_image_report slots[3];
} ivs_info;

/* Size and alignment of an instance of profile_id. BAD_ARG: req NULL or short, id NULL or not
 * NUL-terminated within 64 bytes; UNKNOWN_PROFILE: id not registered. */
IVS_API ivs_status ivs_machine_storage_requirements(const char *profile_id, ivs_storage_requirements *req);

/* Creates an instance in storage (storage_bytes long). Validates everything before the first
 * write; on success zero-fills the instance bytes and sets *out. Refusal order: BAD_ARG (NULL
 * opts, storage or out; short opts; bad profile id; misaligned storage), UNSUPPORTED (flags,
 * nonzero unknown tail), UNKNOWN_PROFILE, BUFFER_TOO_SMALL. On refusal *out is NULL (when out
 * is not NULL) and the storage is untouched. */
IVS_API ivs_status ivs_machine_create(const ivs_create_options *opts, void *storage, size_t storage_bytes, ivs_machine **out);

/* Zero-scrubs the instance bytes; the handle is then invalid (a later call is BAD_HANDLE). */
IVS_API ivs_status ivs_machine_destroy(ivs_machine *m);

/* Phase, machine time, profile index and the three slot reports. */
IVS_API ivs_status ivs_machine_info(const ivs_machine *m, ivs_info *out);

/* SHA-256 of the canonical state (D-35) of one domain (IVS_DIGEST_CPU, _MEM, _IMAGES, _INSTANCE)
 * or of their composite (IVS_DIGEST_COMPOSITE). The serialization is field by field and
 * big-endian, so the value does not depend on struct layout, padding or host byte order; it
 * excludes the magic and padding. BAD_ARG: out NULL or any other domain; then BAD_HANDLE. */
IVS_API ivs_status ivs_machine_state_digest(const ivs_machine *m, uint32_t domain, uint8_t out_sha256[32]);

#ifdef __cplusplus
}
#endif

#endif
