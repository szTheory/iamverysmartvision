/* SPDX-License-Identifier: MIT */
/* Profile-driven memory system (D-16..D-20, A-05, plan 03-04): internal API. Not installed and
 * not an ABI: it lives in core/src, which only the core and the tests (ivs_core_internal) see.
 *
 * The decode is generated from the profile by cmake/IvsMemmapTable.cmake into ivs_profiles.inc,
 * which only memsys.c includes. Every read goes through one width formula,
 *   read = (stored & rmask) | (fill & ~rmask)                                (D-17)
 * with the device record of the region; a stub, an undriven address, a closed gate and an EXEC,
 * GROM or cartridge region with no image loaded read the profile's undriven fill (D-19, D-B7).
 * Bus stores keep value & rmask or are dropped, per the device record's store policy.
 *
 * ivs_memsys is one flat struct with no pointers and no implicit padding: the profile index,
 * the gate flags, the loaded flags, the committed cartridge segment table (the only mutable
 * decode state, A-08 c) and the backing arrays. Each backing array is sized to its device's
 * power-of-two size_words, so the index (addr - lo) & (size_words - 1) stays inside it. Backing
 * words are 16 bits wide for every device: install primitives store whole words and masking
 * happens on read, so a wide stored value (raw-poke seam, A-05a) reads back masked.
 * Layout: 76 bytes of header, then 113,152 bytes of backing (sysram, scratchpad, gram, exec,
 * grom, cart, each u16). */
#ifndef IVS_MEMSYS_H
#define IVS_MEMSYS_H

#include <stddef.h>
#include <stdint.h>

#include "cpu/cp1610.h"

/* Backing capacities in 16-bit words (the generator asserts every profile fits). */
#define IVS_MEMSYS_EXEC_WORDS 4096u      /* 4096 decles */
#define IVS_MEMSYS_GROM_WORDS 2048u      /* 2048 bytes, one per word */
#define IVS_MEMSYS_CART_WORDS 49152u     /* the $4000-$FFFF span */
#define IVS_MEMSYS_SYSRAM_WORDS 512u     /* backing for the 352-word extent */
#define IVS_MEMSYS_SCRATCHPAD_WORDS 256u /* backing for the 240-byte extent */
#define IVS_MEMSYS_GRAM_WORDS 512u
#define IVS_MEMSYS_CART_SEGMENTS 16u
#define IVS_MEMSYS_EXEC_BYTES 8192u /* raw EXEC image: 4096 big-endian words (D-26) */
#define IVS_MEMSYS_GROM_BYTES 2048u /* raw GROM image (D-26) */
#define IVS_MEMSYS_PROFILE_ID_CAP 64u

/* Device kinds (dispatch is a switch on these, D-19). */
#define IVS_MEMSYS_DEVICE_UNDRIVEN 0u
#define IVS_MEMSYS_DEVICE_ROM10 1u
#define IVS_MEMSYS_DEVICE_GROM8 2u
#define IVS_MEMSYS_DEVICE_RAM8 3u
#define IVS_MEMSYS_DEVICE_RAM16 4u
#define IVS_MEMSYS_DEVICE_STUB 5u
#define IVS_MEMSYS_DEVICE_KINDS 6u

/* Gates (D-19): open after init; the internal setter closes them. */
#define IVS_MEMSYS_GATE_NONE 0u
#define IVS_MEMSYS_GATE_STIC_REGS 1u
#define IVS_MEMSYS_GATE_GMEM 2u
#define IVS_MEMSYS_GATES 3u

#define IVS_MEMSYS_IMPL_NONE 0u
#define IVS_MEMSYS_IMPL_REAL 1u
#define IVS_MEMSYS_IMPL_STUB 2u

/* Physical devices: a region's name up to its first `_`; cart is the cartridge span. */
#define IVS_MEMSYS_PHYS_UNDRIVEN 0u
#define IVS_MEMSYS_PHYS_STIC 1u
#define IVS_MEMSYS_PHYS_SCRATCHPAD 2u
#define IVS_MEMSYS_PHYS_PSG 3u
#define IVS_MEMSYS_PHYS_SYSRAM 4u
#define IVS_MEMSYS_PHYS_EXEC 5u
#define IVS_MEMSYS_PHYS_GROM 6u
#define IVS_MEMSYS_PHYS_GRAM 7u
#define IVS_MEMSYS_PHYS_CART 8u

#define IVS_MEMSYS_VIEW_R 1u
#define IVS_MEMSYS_VIEW_W 2u

#define IVS_MEMSYS_SLOT_EXEC 1u
#define IVS_MEMSYS_SLOT_GROM 2u
#define IVS_MEMSYS_SLOT_CART 3u

/* One cartridge segment: words [addr, addr + len_words). */
typedef struct ivs_memsys_segment {
  uint16_t addr;
  uint16_t len_words;
} ivs_memsys_segment;

typedef struct ivs_memsys {
  uint32_t profile;                  /* registry index */
  uint8_t gate_open[4];              /* by IVS_MEMSYS_GATE_*; [NONE] always 1, [3] always 0 */
  uint8_t exec_loaded;               /* 0 or 1 */
  uint8_t grom_loaded;               /* 0 or 1 */
  uint16_t cart_count;               /* committed segments, 0..16 */
  ivs_memsys_segment cart_seg[IVS_MEMSYS_CART_SEGMENTS];
  uint16_t sysram[IVS_MEMSYS_SYSRAM_WORDS];
  uint16_t scratchpad[IVS_MEMSYS_SCRATCHPAD_WORDS];
  uint16_t gram[IVS_MEMSYS_GRAM_WORDS];
  uint16_t exec[IVS_MEMSYS_EXEC_WORDS];
  uint16_t grom[IVS_MEMSYS_GROM_WORDS];
  uint16_t cart[IVS_MEMSYS_CART_WORDS]; /* index addr - cart span lo */
} ivs_memsys;

/* A system-image slot descriptor (D-24, D-26): units of unit_bits each, loaded at base. */
typedef struct ivs_slot_desc {
  uint32_t units;
  uint16_t base;
  uint8_t unit_bits;
  uint8_t required; /* 0 or 1 */
} ivs_slot_desc;

/* One profile registry record (D-03), generated from data/profiles. */
typedef struct ivs_profile_desc {
  char id[IVS_MEMSYS_PROFILE_ID_CAP];
  uint16_t reset_r7;      /* IAB answer for a reset */
  uint16_t iab_value;     /* IAB answer for an interrupt */
  uint16_t undriven_fill; /* memmap.undriven_read */
  uint16_t cart_lo;       /* memmap.cart_span */
  uint16_t cart_hi;
  uint16_t reserved; /* 0; keeps the record free of implicit padding */
  uint8_t has_cart_span;
  uint8_t interrupt_accept_clears_intff;
  uint8_t release_latency_cpu_cycles;
  uint8_t wait_limit_cpu_cycles;
  uint32_t features_supported; /* cartridge.features_supported (D-32) */
  ivs_slot_desc exec;
  ivs_slot_desc grom;
} ivs_profile_desc;

/* What the decode says about one address in one view (A-05b). */
typedef struct ivs_memsys_desc {
  const char *phys_name; /* stic, scratchpad, psg, sysram, exec, grom, gram, cart, undriven */
  uint8_t phys;          /* IVS_MEMSYS_PHYS_* */
  uint8_t device;        /* IVS_MEMSYS_DEVICE_* */
  uint8_t impl;          /* IVS_MEMSYS_IMPL_* */
  uint8_t gate;          /* IVS_MEMSYS_GATE_* */
  uint16_t mirror_words; /* size_words when the extent exceeds it, else 0 */
  uint16_t reserved;
} ivs_memsys_desc;

/* Registry (generated). ivs_profile_get returns NULL for an index >= ivs_profile_count(). */
uint32_t ivs_profile_count(void);
const ivs_profile_desc *ivs_profile_get(uint32_t index);

/* Zero-fills m, selects the profile and opens every gate. BAD_ARG for NULL or a bad index. */
int ivs_memsys_init(ivs_memsys *m, uint32_t profile);

/* Reads and writes through the read and write views. m must have been initialized. */
uint16_t ivs_memsys_read(const ivs_memsys *m, uint16_t addr);
void ivs_memsys_write(ivs_memsys *m, uint16_t addr, uint16_t value);

/* The CPU port (D-20), signature of ivs_bus_fn; ctx is an ivs_memsys. FETCH, READ and ADAR
 * read; WRITE and DW write; IAB answers reset.r7 (IVS_ACC_F_RESET) or interrupt.iab_value
 * (IVS_ACC_F_INTERRUPT), else the undriven fill; INTAK and strobes return data 0. Wait is 0. */
ivs_bus_reply ivs_memsys_bus(void *ctx, const ivs_cp1610_access *a);

/* Install primitives: copy-only, validated input assumed (validation is the loader's job,
 * D-10). They store whole 16-bit words; masking happens on read. A cartridge segment outside
 * the span is not stored; a 17th segment is not recorded. */
void ivs_memsys_install_exec(ivs_memsys *m, const uint8_t *bytes);  /* IVS_MEMSYS_EXEC_BYTES */
void ivs_memsys_install_grom(ivs_memsys *m, const uint8_t *bytes);  /* IVS_MEMSYS_GROM_BYTES */
void ivs_memsys_install_cart_segment(ivs_memsys *m, uint16_t addr, const uint8_t *be_words,
                                     uint16_t len_words);
/* Empties one slot (IVS_MEMSYS_SLOT_*) and zeroes its backing; any other value does nothing. */
void ivs_memsys_clear_slot(ivs_memsys *m, uint32_t slot);

/* Internal gate setter (Phase 6 drives it from the STIC). open is 0 or nonzero. */
void ivs_memsys_set_gate_internal(ivs_memsys *m, uint32_t gate, uint8_t open);

/* Raw-poke seam (A-05a): stores value unmasked into the backing word of addr (read view first,
 * then write view; the cartridge span stores at addr - span lo). No backing: nothing happens. */
void ivs_memsys_poke_raw_internal(ivs_memsys *m, uint16_t addr, uint16_t value);

/* Decode introspection (A-05b). view is IVS_MEMSYS_VIEW_R or IVS_MEMSYS_VIEW_W. Returns IVS_OK,
 * or BAD_ARG for NULL out, a bad profile index or a bad view (out untouched). */
int ivs_memsys_describe(uint32_t profile, uint32_t view, uint16_t addr, ivs_memsys_desc *out);

#endif
