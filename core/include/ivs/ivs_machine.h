/* SPDX-License-Identifier: MIT */
/* Public machine API (D-01..D-08, A-08 f, plan 03-09). A host sizes an instance for a profile,
 * creates it in storage the host owns, queries it and destroys it. The core never allocates and
 * takes no allocator, clock, seed, path or callback (D-01, D-03). The instance type is
 * incomplete here; its layout is internal and not an ABI (D-02).
 *
 * Struct rule (D-04): every struct passed across this API starts with uint32_t struct_size.
 *   - in structs: a size below the v1 size, or above IVS_STRUCT_SIZE_MAX, is BAD_ARG; a larger
 *     size up to IVS_STRUCT_SIZE_MAX is accepted only when every byte the core does not know is
 *     zero (else UNSUPPORTED); nonzero flags are UNSUPPORTED. The core reads at most
 *     IVS_STRUCT_SIZE_MAX bytes of an in struct, so a garbage struct_size is refused, never
 *     followed (review WR-02);
 *   - out structs: a size below the v1 size is BAD_ARG with nothing written; otherwise the core
 *     writes min(caller size, own size) bytes and sets struct_size to the size it wrote.
 * Out parameters must not overlap the instance storage: an out struct or buffer that overlaps
 * the instance's bytes is BAD_ARG with nothing written, so a host cannot aim a report into its
 * own instance (plan 03-11). Only a valid instance has bytes to overlap, so for info,
 * state_digest and the load report this test runs after the handle check: with a NULL or invalid
 * handle the call is BAD_HANDLE wherever the out parameter lies (create tests its out pointer
 * against the storage argument instead).
 * A refused call writes only its out parameters (D-08). A NULL or invalid handle is BAD_HANDLE
 * for every call that takes one; the other argument refusals (BAD_ARG) are checked first. A handle
 * is invalid when its storage holds no live instance (never created, destroyed) or when the
 * instance's stored profile index is damaged (outside the registry, or inconsistent).
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

/* Run phases (D-13, D-14): a new instance is OFF; a successful power-on reset makes it POWERED.
 * Nothing returns a POWERED instance to OFF: a later reset is a power cycle, and only destroy
 * ends the instance (A-08 d). */
#define IVS_PHASE_OFF 0u
#define IVS_PHASE_POWERED 1u

/* Reset kinds (D-14). Any other value is BAD_ARG (A-08 e). */
#define IVS_RESET_POWER_ON 1u

/* State digest domains (D-35, D-10). 0 is the composite of 1..4. 16..18 are live domains: the
 * RUN-05 video, audio and input observations, domain version 1; not part of the composite, which
 * stays version 1 over domains 1..4. Every other value is refused. */
#define IVS_DIGEST_COMPOSITE 0u
#define IVS_DIGEST_CPU 1u
#define IVS_DIGEST_MEM 2u
#define IVS_DIGEST_IMAGES 3u
#define IVS_DIGEST_INSTANCE 4u
#define IVS_DIGEST_VIDEO 16u
#define IVS_DIGEST_AUDIO 17u
#define IVS_DIGEST_INPUT 18u

/* Largest struct_size of an in struct (D-04): above it the call is BAD_ARG. */
#define IVS_STRUCT_SIZE_MAX 4096u

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

/* Output layout formats of ivs_info (D-06). NONE: the profile produces no bytes of that kind yet
 * (the size per frame is then 0). */
#define IVS_OUTPUT_FORMAT_NONE 0u

/* Out: instance information. slots[i] is the slot of kind i + 1. time_ticks is machine time in
 * master ticks (docs/time-base.md).
 * Growth (A-02, plan 04-09): the v1 size of ivs_info is the Phase 3 size (216 bytes, up to and
 * including slots); fields after it are written only when struct_size covers them (A-02). A
 * caller sized for v1 is accepted and its bytes past the v1 size are untouched; the core writes
 * min(struct_size, sizeof(ivs_info)) bytes and sets struct_size to that count. A struct_size
 * above IVS_STRUCT_SIZE_MAX is BAD_ARG with nothing written (review IN-03).
 * Appended fields (byte offsets; 64-bit members 8-aligned), all from the profile:
 *   216 ticks_per_cpu_cycle      master ticks per CPU cycle (D2)
 *   220 cpu_cycles_per_scanline  nominal
 *   224 scanlines_per_frame      nominal
 *   228 video_format             IVS_OUTPUT_FORMAT_*
 *   232 nominal_frame_ticks      ticks_per_cpu_cycle * cpu_cycles_per_scanline *
 *                                scanlines_per_frame; nominal_frame_ticks is nominal, not a frame
 *                                event (D-09): frames are timeline events, never a modulo of time
 *   240 video_bytes_per_frame    0 with IVS_OUTPUT_FORMAT_NONE
 *   244 audio_format             IVS_OUTPUT_FORMAT_*
 *   248 audio_bytes_per_frame    0 with IVS_OUTPUT_FORMAT_NONE
 *   252 reserved2                written 0
 * sizeof(ivs_info) is 256. */
typedef struct ivs_info {
  uint32_t struct_size;
  uint32_t phase; /* IVS_PHASE_* */
  uint64_t time_ticks;
  uint32_t profile_index;
  uint32_t reserved; /* written 0 */
  ivs_image_report slots[3];
  /* ---- end of the v1 (Phase 3) size ---- */
  uint32_t ticks_per_cpu_cycle;
  uint32_t cpu_cycles_per_scanline;
  uint32_t scanlines_per_frame;
  uint32_t video_format; /* IVS_OUTPUT_FORMAT_* */
  uint64_t nominal_frame_ticks;
  uint32_t video_bytes_per_frame;
  uint32_t audio_format; /* IVS_OUTPUT_FORMAT_* */
  uint32_t audio_bytes_per_frame;
  uint32_t reserved2; /* written 0 */
} ivs_info;

/* ---- typed content loading (D-10, D-11, D-24..D-27, D-32, D-33, A-04, A-08, plan 03-10) ----
 * One typed image per call. The core copies the supplied span into instance storage during the
 * call, keeps no pointer and allocates nothing. System images are raw: EXEC is exactly 8192
 * bytes (4096 16-bit words, high byte first, bits 15..10 zero), GROM exactly 2048 bytes. A
 * cartridge is an IVSC v1 container (docs/formats/ivsc-v1.md) whose segments must lie in the
 * profile's cartridge span. Loads and unloads are legal only while the machine is OFF. */

/* Load option flags. Any other bit is UNSUPPORTED. */
#define IVS_LOAD_F_EXPECT_SHA256 0x1u

/* In: load options (may be NULL). With IVS_LOAD_F_EXPECT_SHA256 the load is refused
 * DIGEST_MISMATCH unless expect_sha256 equals the SHA-256 of the supplied bytes (D-27). */
typedef struct ivs_load_options {
  uint32_t struct_size;
  uint32_t flags;
  uint8_t expect_sha256[32];
} ivs_load_options;

/* Refusal reasons of a load report. 1..18 are the reason numbers of the IVSC v1 reason table
 * (docs/formats/ivsc-v1.md); 19..33 are the load's own reasons and 34 and up the run's (one
 * list for every report). Numbers are append-only. */
#define IVS_REASON_NONE 0u
#define IVS_REASON_TRUNCATED 1u        /* MALFORMED */
#define IVS_REASON_TOO_LARGE 2u        /* OVERSIZED */
#define IVS_REASON_MAGIC 3u            /* MALFORMED */
#define IVS_REASON_VERSION 4u          /* MALFORMED */
#define IVS_REASON_FLAGS 5u            /* MALFORMED */
#define IVS_REASON_RESERVED 6u         /* MALFORMED */
#define IVS_REASON_TRAILING 7u         /* MALFORMED */
#define IVS_REASON_SEGMENT_COUNT 8u    /* MALFORMED */
#define IVS_REASON_SIZE_MISMATCH 9u    /* MALFORMED */
#define IVS_REASON_SEGMENT_LENGTH 10u  /* MALFORMED */
#define IVS_REASON_SEGMENT_WIDTH 11u   /* MALFORMED */
#define IVS_REASON_SEGMENT_ATTR 12u    /* MALFORMED */
#define IVS_REASON_SEGMENT_RESERVED 13u /* MALFORMED */
#define IVS_REASON_ADDR_WRAP 14u       /* OUT_OF_RANGE */
#define IVS_REASON_ORDER 15u           /* MALFORMED */
#define IVS_REASON_OVERLAP 16u         /* OVERLAP */
#define IVS_REASON_DIGEST 17u          /* DIGEST_MISMATCH: the container's trailer */
#define IVS_REASON_WORD_RANGE 18u      /* MALFORMED: a cartridge word above 0x3FF */
#define IVS_REASON_OUT_OF_SPAN 19u     /* OUT_OF_RANGE: a segment leaves the profile's cartridge span */
#define IVS_REASON_FEATURES 20u        /* UNSUPPORTED: required_features bits the profile lacks */
#define IVS_REASON_IMAGE_SHORT 21u     /* MALFORMED: a system image below its exact size (0 included) */
#define IVS_REASON_IMAGE_LONG 22u      /* OVERSIZED: a system image above its exact size */
#define IVS_REASON_IMAGE_WORD 23u      /* MALFORMED: an EXEC word with a bit of 15..10 set */
#define IVS_REASON_EXPECT_DIGEST 24u   /* DIGEST_MISMATCH: expect_sha256 differs */
#define IVS_REASON_ARG_KIND 25u        /* BAD_ARG: kind is not IVS_KIND_* */
#define IVS_REASON_ARG_NULL_BYTES 26u  /* BAD_ARG: bytes NULL with len above 0 */
#define IVS_REASON_ARG_STRUCT_SIZE 27u /* BAD_ARG: opts->struct_size short or above the cap */
#define IVS_REASON_ARG_INPUT_OVERLAP 28u /* BAD_ARG: the input span overlaps the instance */
#define IVS_REASON_HANDLE 29u          /* BAD_HANDLE */
#define IVS_REASON_OPTION_FLAGS 30u    /* UNSUPPORTED: an unknown option flag */
#define IVS_REASON_OPTION_TAIL 31u     /* UNSUPPORTED: a nonzero byte in a larger opts tail */
#define IVS_REASON_STATE_POWERED 32u   /* BAD_STATE: the machine is POWERED */
#define IVS_REASON_SLOT_OCCUPIED 33u   /* DUPLICATE_LOAD: the slot holds an image */
/* Refusal reasons of ivs_machine_run (ivs_run_report.refusal_reason, plan 04-07, D-05). A refused
 * run writes the reason only into a report that is not itself the cause; a bad handle reports
 * IVS_REASON_HANDLE. */
#define IVS_REASON_RUN_NULL_ARG 34u       /* BAD_ARG: req NULL */
#define IVS_REASON_RUN_STRUCT_SIZE 35u    /* BAD_ARG: req->struct_size short or above the cap */
#define IVS_REASON_RUN_REPORT_OVERLAP 36u /* BAD_ARG: the report overlaps the instance (never written into it) */
#define IVS_REASON_RUN_FLAGS 37u          /* UNSUPPORTED: an unknown or reserved run flag */
#define IVS_REASON_RUN_TAIL 38u           /* UNSUPPORTED: a nonzero byte in a larger request tail */
#define IVS_REASON_RUN_STATE_OFF 39u      /* BAD_STATE: the machine is OFF */
#define IVS_REASON_RUN_TARGET_RANGE 40u   /* OUT_OF_RANGE: target_ticks above 2^63 */
#define IVS_REASON_RUN_BUFFER_SMALL 41u   /* BUFFER_TOO_SMALL: an output capacity below the layout's requirement */
#define IVS_REASON_RUN_BUFFER_NULL 42u    /* BAD_ARG: an output buffer NULL with a nonzero capacity */
#define IVS_REASON_RUN_BUFFER_OVERLAP 43u /* BAD_ARG: a buffer overlaps the instance, the other buffer or the report */
/* Input reasons of ivs_machine_run (plan 04-11, D-05, D-13, D-14; 54 plan 04-12, D-15). 48..54
 * concern one event: the report's input_failing_index names it. */
#define IVS_REASON_INPUT_RESERVED_IN 44u      /* UNSUPPORTED: req->reserved_in is nonzero */
#define IVS_REASON_INPUT_EVENTS_NULL 45u      /* BAD_ARG: events NULL with event_count above 0 */
#define IVS_REASON_INPUT_EVENT_COUNT 46u      /* BAD_ARG: event_count above IVS_INPUT_EVENTS_MAX */
#define IVS_REASON_INPUT_EVENTS_OVERLAP 47u   /* BAD_ARG: the events overlap the instance, the report or a buffer */
#define IVS_REASON_INPUT_TICK_RANGE 48u       /* BAD_ARG: an at_tick at or above 2^63 */
#define IVS_REASON_INPUT_UNSORTED 49u         /* BAD_ARG: an at_tick below the previous event's */
#define IVS_REASON_INPUT_BEFORE_FLOOR 50u     /* BAD_ARG: an at_tick below the input floor */
#define IVS_REASON_INPUT_EVENT_FLAGS 51u      /* UNSUPPORTED: an event's flags are nonzero */
#define IVS_REASON_INPUT_EVENT_RESERVED 52u   /* UNSUPPORTED: an event's reserved word is nonzero */
#define IVS_REASON_INPUT_CONTROL_RESERVED 53u /* UNSUPPORTED: a controller word has IVS_CTRL_RESERVED_MASK set */
#define IVS_REASON_INPUT_EXTRA_CONTROLLER 54u /* UNSUPPORTED: a nonzero word at or above input.controllers */

/* segment_index of a report whose reason concerns no single segment. */
#define IVS_SEGMENT_NONE 0xFFFFFFFFu

/* Out: load report (may be NULL). On success image is the slot's new report and reason is
 * IVS_REASON_NONE. On a refusal only the detail fields are written (reason, segment_index,
 * byte_offset, missing_features) and image stays zero. segment_index is IVS_SEGMENT_NONE unless
 * the reason concerns one segment; byte_offset is where the defect is seen in the input. */
typedef struct ivs_load_report {
  uint32_t struct_size;
  uint32_t reason;           /* IVS_REASON_* */
  uint32_t segment_index;    /* or IVS_SEGMENT_NONE */
  uint32_t missing_features; /* FEATURES: required bits the profile does not support */
  uint64_t byte_offset;
  ivs_image_report image;
} ivs_load_report;

/* Size and alignment of an instance of profile_id. BAD_ARG: req NULL or short, id NULL or not
 * NUL-terminated within 64 bytes; UNKNOWN_PROFILE: id not registered. */
IVS_API ivs_status ivs_machine_storage_requirements(const char *profile_id, ivs_storage_requirements *req);

/* Creates an instance in storage (storage_bytes long). Validates everything before the first
 * write; on success zero-fills the instance bytes and sets *out. Refusal order: BAD_ARG (out
 * inside the instance bytes of storage; NULL opts, storage or out; opts struct_size short or
 * above IVS_STRUCT_SIZE_MAX; bad profile id; misaligned storage), UNSUPPORTED (flags, nonzero
 * unknown tail), UNKNOWN_PROFILE, BUFFER_TOO_SMALL. The instance bytes are the first
 * min(storage_bytes, instance size) bytes of storage; any bytes after them belong to the host
 * (D-01). On refusal the instance bytes are untouched and *out is set to NULL, unless out is NULL
 * or lies inside the instance bytes (then nothing at all is written). An out that lies in storage
 * after the instance bytes is host memory: it gets the NULL (review WR-06). */
IVS_API ivs_status ivs_machine_create(const ivs_create_options *opts, void *storage, size_t storage_bytes, ivs_machine **out);

/* Zero-scrubs the instance bytes; the handle is then invalid (a later call is BAD_HANDLE). */
IVS_API ivs_status ivs_machine_destroy(ivs_machine *m);

/* Phase, machine time, profile index and the three slot reports, then (when struct_size covers
 * them, A-02) the timebase and output layout. BAD_ARG: out NULL or struct_size below the v1 size
 * (216); then BAD_HANDLE; then BAD_ARG: the bytes to be written overlap the instance storage. */
IVS_API ivs_status ivs_machine_info(const ivs_machine *m, ivs_info *out);

/* SHA-256 of the canonical state (D-35) of one domain (IVS_DIGEST_CPU, _MEM, _IMAGES, _INSTANCE,
 * _VIDEO, _AUDIO, _INPUT) or of the composite of the first four (IVS_DIGEST_COMPOSITE; the
 * observation domains are not in it, D-10). The serialization is field by field and
 * big-endian, so the value does not depend on struct layout, padding or host byte order; it
 * excludes the magic and padding. BAD_ARG: out NULL or any other domain; then BAD_HANDLE; then
 * BAD_ARG: out overlapping the instance storage. */
IVS_API ivs_status ivs_machine_state_digest(const ivs_machine *m, uint32_t domain, uint8_t out_sha256[32]);

/* Loads one image of kind (IVS_KIND_*) from len bytes at bytes. Validates everything before the
 * first write to the instance; a refusal leaves the instance unchanged (D-10, D-12). The first
 * failing class wins:
 *   BAD_ARG         report struct_size short, or report overlapping the storage of a valid
 *                   instance (nothing written; not tested for a NULL or invalid handle); kind
 *                   unknown; bytes NULL with len above 0; opts struct_size short or above
 *                   IVS_STRUCT_SIZE_MAX;
 *   BAD_HANDLE      NULL or invalid handle;
 *   UNSUPPORTED     an unknown option flag or a nonzero opts tail;
 *   BAD_STATE       the machine is POWERED;
 *   DUPLICATE_LOAD  the slot already holds an image (unload first);
 *   OVERSIZED       len above the kind's bound (EXEC 8192, GROM 2048, cartridge 131,256);
 *   BAD_ARG         the input span overlaps the instance storage (tested only within the bound);
 *   content         system image: MALFORMED (short, or an EXEC word wider than 10 bits);
 *                   cartridge: the IVSC v1 checks in spec order, then OUT_OF_RANGE (a segment
 *                   outside the profile's cartridge span), then UNSUPPORTED (required_features);
 *   DIGEST_MISMATCH expect_sha256 differs from the SHA-256 of the supplied bytes.
 * A refusal status never depends on where the host placed its buffer. */
IVS_API ivs_status ivs_machine_load(ivs_machine *m, uint32_t kind, const uint8_t *bytes, size_t len, const ivs_load_options *opts, ivs_load_report *report);

/* Empties the slot of kind: its words are zero-scrubbed and it reports loaded 0. An empty slot
 * unloads OK. BAD_ARG: unknown kind; BAD_HANDLE; BAD_STATE: the machine is POWERED. After every
 * slot is unloaded the state digest equals a fresh instance's (D-13). */
IVS_API ivs_status ivs_machine_unload(ivs_machine *m, uint32_t kind);

/* Power-on reset (D-14, A-08 d, e). Validates before the first write: BAD_ARG for a kind other
 * than IVS_RESET_POWER_ON; BAD_HANDLE; MISSING_SYSTEM_IMAGE when a slot the profile requires is
 * empty (EXEC is checked before GROM). A refusal leaves the instance unchanged. On success the
 * volatile state takes its power-on fill (system RAM, scratchpad and GRAM zero), every gate is
 * open, the CPU is armed to take its reset entry through the profile's reset vector, machine
 * time is 0 and the phase is POWERED; the image slots are untouched. Allowed from OFF and from
 * POWERED (a power cycle, which gives the same state for the same content); after it every
 * load and unload is BAD_STATE until a new instance. */
IVS_API ivs_status ivs_machine_reset(ivs_machine *m, uint32_t kind);

#ifdef __cplusplus
}
#endif

#endif
