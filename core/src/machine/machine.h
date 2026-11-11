/* SPDX-License-Identifier: MIT */
/* Machine instance (D-01, D-02, D-08, D-35, plan 03-09): internal layout and seams. Not
 * installed and not an ABI: it lives in core/src, which only the core and the tests
 * (ivs_core_internal) see. The public handle type ivs_machine (ivs/ivs_machine.h) is this struct.
 *
 * One flat struct with no pointers: magic, profile index, run phase, a reserved word, machine
 * time, the embedded 32-byte CPU state, the memory system (gates, loaded flags, the committed
 * cartridge segment table and every backing array at the profile's worst-case capacity), an
 * explicit pad word, the observation rows (plan 04-05, D-10: the video, audio and input digest
 * domains 16..18) and one record per image slot. Every field is fixed width; the only bytes that
 * are not fields are the reserved word, the CPU's reserved bytes and the observation pad word,
 * and all of them stay zero. The instance holds no last-error or timestamp (D-08); the
 * observation counts are machine state of their domains.
 * Layout (bytes): header 24, cpu 32, mem 113,228 (ends at an offset that is 4 mod 8), pad 4,
 * obs 144, slots 3 x 56: 113,600, a multiple of 16, so there is no tail padding (machine_digest.c
 * asserts it). The previous layout (Phase 3, no obs) was 113,456 with 4 bytes of tail padding. */
#ifndef IVS_MACHINE_INTERNAL_H
#define IVS_MACHINE_INTERNAL_H

#include <stddef.h>
#include <stdint.h>

#include "content/ivsc.h"
#include "cpu/cp1610.h"
#include "ivs/ivs_machine.h"
#include "memmap/memsys.h"

#define IVS_MACHINE_MAGIC 0x4956534Du /* "IVSM" */
#define IVS_MACHINE_ALIGN 16u
#define IVS_MACHINE_BYTES 113600u

/* The v1 (Phase 3) size of ivs_info (A-02, plan 04-09): the smallest struct_size info accepts.
 * Fields after it are written only when the caller's struct_size covers them. machine.c ties it
 * to offsetof(ivs_info, ticks_per_cpu_cycle). */
#define IVS_INFO_V1_SIZE 216u

/* One image slot (kind fixed at create: slot i holds kind i + 1). */
typedef struct ivs_machine_slot {
  uint32_t kind;
  uint32_t loaded; /* 0 or 1 */
  uint32_t length_bytes;
  uint32_t length_units;
  uint32_t unit_bits;
  uint32_t base_addr;
  uint8_t sha256[32];
} ivs_machine_slot;

/* Observation rows (plan 04-05, D-10, A-03): a chain[32] and a u64 record count for VIDEO
 * (digest domain 16) and AUDIO (17); INPUT (18) holds the controller words in force, the applied
 * event count, the input floor tick and the input chain. Every row is zero at create and after a
 * reset (the INPUT chain's start value arrives with plan 04-11). Domain version 1; not part of the
 * composite, which stays version 1 over domains 1..4. */
typedef struct ivs_machine_obs_av {
  uint8_t chain[32];
  uint64_t count;
} ivs_machine_obs_av;

typedef struct ivs_machine_obs_input {
  uint32_t words[4];
  uint64_t count;
  uint64_t floor_tick;
  uint8_t chain[32];
} ivs_machine_obs_input;

typedef struct ivs_machine_obs {
  ivs_machine_obs_av video;
  ivs_machine_obs_av audio;
  ivs_machine_obs_input input;
} ivs_machine_obs;

struct ivs_machine {
  _Alignas(16) uint32_t magic; /* IVS_MACHINE_MAGIC while valid; 0 after destroy */
  uint32_t profile;            /* registry index */
  uint32_t phase;              /* IVS_PHASE_* */
  uint32_t reserved;           /* 0 */
  uint64_t time_ticks;         /* machine time in master ticks */
  ivs_cp1610 cpu;
  ivs_memsys mem;
  uint32_t obs_pad;    /* 0: mem ends at 4 mod 8, so this word keeps obs 8-aligned with no hole */
  ivs_machine_obs obs; /* before slot[]: IVS_FAULT_DIGEST_SKIPS_IMAGE_TABLE skips rows from slot on */
  ivs_machine_slot slot[IVS_SLOT_COUNT];
};

_Static_assert(sizeof(ivs_machine_slot) == 56u, "ivs_machine_slot must be 56 bytes");
_Static_assert(sizeof(ivs_machine_obs) == 144u, "ivs_machine_obs must be 144 bytes");
_Static_assert(offsetof(struct ivs_machine, obs) == offsetof(struct ivs_machine, obs_pad) + 4u,
               "no implicit hole between the observation pad word and obs");
_Static_assert(_Alignof(struct ivs_machine) == IVS_MACHINE_ALIGN, "instance alignment is 16");
_Static_assert(sizeof(struct ivs_machine) == IVS_MACHINE_BYTES,
               "instance size changed: review the digest field table (machine_digest.c)");

/* ---- canonical state digest (D-35, machine_digest.c) ----
 * One field table covers every instance byte exactly once. A UINT row is count elements of width
 * bytes (1, 2, 4 or 8), each read natively and written big-endian by shifts; a row with count > 1
 * is preceded by its u32 count. A PROFILE row is a u32 registry index written as the profile id:
 * u32 length then the ASCII bytes (an index outside the registry writes u32 0xFFFFFFFF then the
 * index). MAGIC and PAD rows (the magic, the reserved word, the CPU's reserved bytes and the
 * observation pad word) are declared but never serialized; their domain names the neighbour they
 * sit with. Domain preimage: "IVSD" || u8 domain || u16 IVS_DIGEST_DOMAIN_VERSION || the domain's
 * rows in table order. Composite: "IVSSTATE" || u16 IVS_DIGEST_COMPOSITE_VERSION || for domains
 * 1..4: u8 domain || its 32-byte SHA-256. Domains 16..18 (VIDEO, AUDIO, INPUT) have their own
 * digests and are not in the composite (D-10). */
#define IVS_DIGEST_KIND_UINT 1u
#define IVS_DIGEST_KIND_PROFILE 2u
#define IVS_DIGEST_KIND_MAGIC 3u
#define IVS_DIGEST_KIND_PAD 4u
#define IVS_DIGEST_DOMAIN_VERSION 1u
#define IVS_DIGEST_COMPOSITE_VERSION 1u

typedef struct ivs_digest_field {
  const char *name;
  uint32_t offset; /* offsetof(struct ivs_machine, ...) */
  uint32_t width;  /* bytes per element */
  uint32_t count;  /* elements */
  uint8_t domain;  /* IVS_DIGEST_CPU..IVS_DIGEST_INSTANCE or IVS_DIGEST_VIDEO..IVS_DIGEST_INPUT */
  uint8_t kind;    /* IVS_DIGEST_KIND_* */
  uint16_t reserved;
} ivs_digest_field;

/* The field table (coverage seam for the tests). */
size_t ivs_machine_digest_field_count(void);
const ivs_digest_field *ivs_machine_digest_field(size_t i);

/* Writes the preimage of domain 1..4 or 16..18 of m into buf (at most cap bytes; buf may be NULL when cap
 * is 0) and returns its full length; 0 for NULL m or another domain. No handle check: the tests
 * pass modified copies. */
size_t ivs_machine_digest_preimage(const struct ivs_machine *m, uint32_t domain, uint8_t *buf, size_t cap);

/* Writes the public report of one slot record (shared by ivs_machine_info and the load report,
 * A-08 f). */
void ivs_machine_slot_report(const ivs_machine_slot *s, ivs_image_report *o);

/* Layer 2 span rule of a cartridge load (D-33, A-01, machine_load.c): the index of the first
 * segment of v (table order) that does not lie wholly inside the words [lo, hi], or
 * IVS_SEGMENT_NONE. A test seam as well: the registered profile's span ends at $FFFF, where the
 * parser's address-wrap check already refuses a segment that runs past it, so the span's upper
 * edge is tested here with a synthetic span. */
uint32_t ivs_machine_cart_span_check(uint32_t lo, uint32_t hi, const ivs_ivsc_view *v);

/* 1 when the n bytes at p overlap the inst_bytes bytes at inst (A-06, A-08 a). Differences of
 * uintptr_t values only, so nothing wraps whatever the pointers are; n or inst_bytes 0 is 0.
 * Used for input spans (only within the kind's bound) and for every out parameter that points
 * into caller memory: an out struct or buffer that overlaps the instance storage is refused
 * BAD_ARG with nothing written, so a host cannot aim a report into its own instance. */
uint32_t ivs_machine_span_overlaps(const void *p, size_t n, const void *inst, size_t inst_bytes);

/* The out-parameter rule of create, info and state_digest (plan 03-11): 1 when the n bytes at out
 * overlap the inst_bytes bytes at inst (ivs_machine_span_overlaps), so the call refuses BAD_ARG
 * with nothing written. info, state_digest and the load report test it only against a validated
 * instance, after BAD_HANDLE (review WR-01): a NULL handle has no bytes to overlap. One function so the named fault OUT_OVERLAP_UNCHECKED (plan 03-12)
 * removes the rule from all three calls at once; the load report keeps its own check and fault
 * (REPORT_INSIDE_UNCHECKED). */
uint32_t ivs_machine_out_overlaps(const void *out, size_t n, const void *inst, size_t inst_bytes);

/* ---- internal step seam (D-05, D-20, plan 03-12, machine_run.c) ----
 * Steps one CPU unit of a POWERED instance through its memory system: the CPU configuration
 * comes from the profile registry (cpu.interrupt_accept_clears_intff,
 * bus.release_latency_cpu_cycles, bus.wait_limit_cpu_cycles), every line is inactive (no device
 * drives INTR, INTRM, BUSRQ or STPST in Phase 3), and the port's bus is ivs_memsys_bus on the
 * instance's own memory system, so the IAB of the reset entry answers the profile's reset.r7.
 * No access log is kept. Machine time advances by the unit's elapsed CPU cycles times the
 * profile's ticks_per_cpu_cycle. BAD_ARG: out NULL; BAD_HANDLE; BAD_STATE: the instance is OFF;
 * else the CPU step's status (a CPU refusal leaves time unchanged). *out is written only on
 * IVS_OK. Never public: it is declared here and nowhere under core/include; Phase 4's bounded
 * run API is built on it. */
ivs_status ivs_machine_step_unit_internal(ivs_machine *m, ivs_cp1610_result *out);

/* The instance behind a handle, or NULL for NULL, a wrong magic, or a stored profile index that
 * is outside the registry or differs from the memory system's (review WR-03): every call that
 * takes a handle refuses such an instance BAD_HANDLE, and nothing behind it indexes a profile
 * table with an unchecked index. */
const struct ivs_machine *ivs_machine_from_handle(const ivs_machine *m);
struct ivs_machine *ivs_machine_from_handle_mut(ivs_machine *m);

/* Length of a profile id (bytes before its NUL), bounded by IVS_PROFILE_ID_MAX_BYTES; BAD_ARG for
 * NULL or no NUL in the first 64 bytes. Hand-written bounded loop, no string functions (A-06). */
ivs_status ivs_machine_profile_id_length(const char *id, uint32_t *len);

/* Registry index of a profile id: BAD_ARG as above, UNKNOWN_PROFILE for a registry miss. */
ivs_status ivs_machine_profile_lookup(const char *id, uint32_t *index);

#endif
