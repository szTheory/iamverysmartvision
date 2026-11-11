/* SPDX-License-Identifier: MIT */
/* Machine instance (D-01, D-02, D-08, D-35, plan 03-09): internal layout and seams. Not
 * installed and not an ABI: it lives in core/src, which only the core and the tests
 * (ivs_core_internal) see. The public handle type ivs_machine (ivs/ivs_machine.h) is this struct.
 *
 * One flat struct with no pointers: magic, profile index, run phase, a reserved word, machine
 * time, the embedded 32-byte CPU state, the memory system (gates, loaded flags, the committed
 * cartridge segment table and every backing array at the profile's worst-case capacity) and one
 * record per image slot. Every field is fixed width; the only bytes that are not fields are the
 * reserved word, the CPU's reserved bytes and the tail padding up to the 16-byte alignment, and
 * all of them stay zero. The instance holds no last-error, counter or timestamp (D-08).
 * Layout (bytes): header 24, cpu 32, mem 113,228, slots 3 x 56, tail padding 4: 113,456. */
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
#define IVS_MACHINE_BYTES 113456u

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

struct ivs_machine {
  _Alignas(16) uint32_t magic; /* IVS_MACHINE_MAGIC while valid; 0 after destroy */
  uint32_t profile;            /* registry index */
  uint32_t phase;              /* IVS_PHASE_* */
  uint32_t reserved;           /* 0 */
  uint64_t time_ticks;         /* machine time in master ticks */
  ivs_cp1610 cpu;
  ivs_memsys mem;
  ivs_machine_slot slot[IVS_SLOT_COUNT];
};

_Static_assert(sizeof(ivs_machine_slot) == 56u, "ivs_machine_slot must be 56 bytes");
_Static_assert(_Alignof(struct ivs_machine) == IVS_MACHINE_ALIGN, "instance alignment is 16");
_Static_assert(sizeof(struct ivs_machine) == IVS_MACHINE_BYTES,
               "instance size changed: review the digest field table (machine_digest.c)");

/* ---- canonical state digest (D-35, machine_digest.c) ----
 * One field table covers every instance byte exactly once. A UINT row is count elements of width
 * bytes (1, 2, 4 or 8), each read natively and written big-endian by shifts; a row with count > 1
 * is preceded by its u32 count. A PROFILE row is a u32 registry index written as the profile id:
 * u32 length then the ASCII bytes (an index outside the registry writes u32 0xFFFFFFFF then the
 * index). MAGIC and PAD rows (the magic, the reserved word, the CPU's reserved bytes and the tail
 * padding) are declared but never serialized; their domain names the neighbour they sit with.
 * Domain preimage: "IVSD" || u8 domain || u16 IVS_DIGEST_DOMAIN_VERSION || the domain's rows in
 * table order. Composite: "IVSSTATE" || u16 IVS_DIGEST_COMPOSITE_VERSION || for domains 1..4:
 * u8 domain || its 32-byte SHA-256. */
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
  uint8_t domain;  /* IVS_DIGEST_CPU..IVS_DIGEST_INSTANCE */
  uint8_t kind;    /* IVS_DIGEST_KIND_* */
  uint16_t reserved;
} ivs_digest_field;

/* The field table (coverage seam for the tests). */
size_t ivs_machine_digest_field_count(void);
const ivs_digest_field *ivs_machine_digest_field(size_t i);

/* Writes the preimage of domain 1..4 of m into buf (at most cap bytes; buf may be NULL when cap
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

/* The instance behind a handle, or NULL for NULL or a wrong magic. */
const struct ivs_machine *ivs_machine_from_handle(const ivs_machine *m);
struct ivs_machine *ivs_machine_from_handle_mut(ivs_machine *m);

/* Length of a profile id (bytes before its NUL), bounded by IVS_PROFILE_ID_MAX_BYTES; BAD_ARG for
 * NULL or no NUL in the first 64 bytes. Hand-written bounded loop, no string functions (A-06). */
ivs_status ivs_machine_profile_id_length(const char *id, uint32_t *len);

/* Registry index of a profile id: BAD_ARG as above, UNKNOWN_PROFILE for a registry miss. */
ivs_status ivs_machine_profile_lookup(const char *id, uint32_t *index);

#endif
