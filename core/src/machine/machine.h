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

/* The instance behind a handle, or NULL for NULL or a wrong magic. */
const struct ivs_machine *ivs_machine_from_handle(const ivs_machine *m);
struct ivs_machine *ivs_machine_from_handle_mut(ivs_machine *m);

/* Length of a profile id (bytes before its NUL), bounded by IVS_PROFILE_ID_MAX_BYTES; BAD_ARG for
 * NULL or no NUL in the first 64 bytes. Hand-written bounded loop, no string functions (A-06). */
ivs_status ivs_machine_profile_id_length(const char *id, uint32_t *len);

/* Registry index of a profile id: BAD_ARG as above, UNKNOWN_PROFILE for a registry miss. */
ivs_status ivs_machine_profile_lookup(const char *id, uint32_t *index);

#endif
