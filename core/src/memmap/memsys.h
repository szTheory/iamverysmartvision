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
 * grom, cart, each u16), then (plan 05-03, D-04) the guest-visible PSG words: 16 registers of
 * 16 bits (32 bytes), the two port pull-down masks (2 bytes) and an explicit 2-byte pad: 113,264
 * bytes in all, divisible by 8; plan 06-05 (D-04, D-16) appends the 64 STIC register words (128
 * bytes), 113,392 bytes in all, divisible by 16. Plan 06-06 (D-04, A-01): the five STIC regions are the real device kind
 * stic14: a read gives the register word under the register's read mask of the generated ivs_stic_params with the
 * undriven fill above it, a store keeps value & the register's write mask (a strobe or reserved register stores
 * nothing); side effects (the display-enable strobe, the mode select) live in the machine's bus wrapper, never here,
 * so a peek can never switch a mode. Since plan 05-04 the PSG region ($01F0-$01FF) is the real device
 * kind psg8: a read gives the register store under the profile's read mask (psg.regmap) with
 * bits 15..8 zero, a store keeps value & the write mask, through ivs_psg_reg_load and
 * ivs_psg_reg_store (psg/psg.h). */
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
#define IVS_MEMSYS_PSG_WORDS 16u /* PSG register words: the backing of the psg region (plan 05-04) */
#define IVS_MEMSYS_PSG_REGS IVS_MEMSYS_PSG_WORDS
#define IVS_MEMSYS_PSG_PORTS 2u /* PSG I/O ports: R14 = port 0, R15 = port 1 (D-04) */
#define IVS_MEMSYS_STIC_REGS 64u /* STIC register words $0000..$003F (plan 06-05, D-04; machine.h ties it to IVS_STIC_REG_COUNT) */
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
#define IVS_MEMSYS_DEVICE_PSG8 6u /* plan 05-04: byte-wide PSG register file, upper byte 0 (C64) */
#define IVS_MEMSYS_DEVICE_STIC14 7u /* plan 06-06: the 14-bit STIC register file, per-register masks (stic.regmap) */
#define IVS_MEMSYS_DEVICE_KINDS 8u

/* Hook classes (plan 06-06, D-04, D-05, A-07): one per 16-word granule and view, generated from the profile. The machine's
 * bus wrapper hooks an access whose class is STIC_REGS, GMEM, BACKTAB or STIC_ALIAS_READ; PSG (1) is the class the wrapper
 * already serves through its own write-view decode. STIC_ALIAS_READ exists in the read view only: the granules that hold a
 * read-sensitive mode-select address outside the STIC register span, which the read view decodes to the cartridge. */
#define IVS_MEMSYS_HOOK_NONE 0u
#define IVS_MEMSYS_HOOK_PSG 1u
#define IVS_MEMSYS_HOOK_STIC_REGS 2u
#define IVS_MEMSYS_HOOK_GMEM 3u
#define IVS_MEMSYS_HOOK_BACKTAB 4u
#define IVS_MEMSYS_HOOK_STIC_ALIAS_READ 5u

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
  /* Plan 05-03 (D-04, A-13): guest-visible PSG state, MEM digest rows. Zero at create and after a
   * power-on reset; written by the masked PSG store and (the pull-down masks) by the input path, read
   * by guest reads and by the PSG engine. */
  uint16_t psg_reg[IVS_MEMSYS_PSG_REGS];
  uint8_t port_pulldown[IVS_MEMSYS_PSG_PORTS]; /* low-driven pin masks of the two ports */
  uint8_t psg_reserved[2];                     /* 0: pad to 4 bytes, no implicit hole */
  /* Plan 06-05 (D-04, D-16): the guest-visible STIC register file, words $0000..$003F, MEM digest rows.
   * Zero at create and after a power-on reset (the profile's registers record is "zero"); the 24 reserved
   * slots stay 0 for good. Nothing reads or writes it until the device plan (06-06). */
  uint16_t stic_reg[IVS_MEMSYS_STIC_REGS];
} ivs_memsys;

/* 113,264 (Phase 5) + 64 x 2 = 113,392, a multiple of 16 (7,087 x 16): psg_reserved ends 4-aligned, so the
 * u16 array follows with no hole and nothing trails it. */
_Static_assert(sizeof(ivs_memsys) == 113392u, "ivs_memsys must be 113,392 bytes (machine.h layout)");

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
  uint16_t ticks_per_cpu_cycle; /* master ticks per CPU cycle (D2, plan 03-12); nonzero */
  uint8_t has_cart_span;
  uint8_t interrupt_accept_clears_intff;
  uint8_t release_latency_cpu_cycles;
  uint8_t wait_limit_cpu_cycles;
  uint32_t features_supported; /* cartridge.features_supported (D-32) */
  ivs_slot_desc exec;
  ivs_slot_desc grom;
  uint16_t scanlines_per_frame;     /* nominal frame timebase (D-06, D-09, plan 04-09); nonzero */
  uint16_t cpu_cycles_per_scanline; /* nominal; nonzero; never a frame event */
  uint16_t input_controllers;       /* input.controllers, 1..4 (D-13, D-15, plan 04-12): a nonzero
                                     * controller word at or above it is refused UNSUPPORTED */
  /* Plan 05-06 (D-09, D-15, D-17, D-19): the PSG time chain and the crystal passthrough. */
  uint16_t ticks_per_psg_clock;     /* master ticks per PSG chip clock (ticks_per_psg_clock), nonzero */
  uint16_t psg_sample_period_ticks; /* master ticks per chip step = audio sample (psg.sample_period_ticks) */
  uint16_t psg_max_unit_ticks;      /* proven maximum ticks of one CPU unit (psg.max_unit_ticks, C53) */
  uint16_t psg_log_records_per_frame; /* write-log records one frame slot is sized for (psg.log_records_per_frame, C53, plan 05-08) */
  uint16_t psg_min_write_gap_ticks;   /* minimum ticks between two PSG register writes (psg.min_write_gap_ticks, C53, plan 05-08); the core never reads it, the tests re-prove it */
  uint64_t crystal_hz_num;          /* crystal_hz as a fraction; the core never reads it (D-19) */
  uint64_t crystal_hz_den;
} ivs_profile_desc;

/* What the decode says about one address in one view (A-05b). */
typedef struct ivs_memsys_desc {
  const char *phys_name; /* stic, scratchpad, psg, sysram, exec, grom, gram, cart, undriven */
  uint8_t phys;          /* IVS_MEMSYS_PHYS_* */
  uint8_t device;        /* IVS_MEMSYS_DEVICE_* */
  uint8_t impl;          /* IVS_MEMSYS_IMPL_* */
  uint8_t gate;          /* IVS_MEMSYS_GATE_* */
  uint16_t mirror_words; /* size_words when the extent exceeds it or the region is an alias, else 0 */
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

/* The register store of PSG register idx (0..15 = $01F0 + idx): the stored word under the profile's
 * read mask, which is what a bus read of that address returns (upper byte 0); 0 for idx at or
 * above IVS_MEMSYS_PSG_WORDS. Pure. It does not follow the port direction and pins, so it is also
 * what the register-file tests read for the two port registers (plan 05-05 changes the bus read
 * of $01FE and $01FF, not this accessor). */
uint16_t ivs_memsys_psg_reg_internal(const ivs_memsys *m, uint32_t idx);

/* The register slot (0..15 = $01F0 + slot) that a WRITE or DW to addr reaches in the PSG register file, by the
 * write view's own decode (plan 05-07: the machine's bus wrapper restarts the envelope after a shape write
 * without holding an address constant), or IVS_MEMSYS_NO_SLOT when the write view does not decode addr to the
 * PSG region. Pure. */
#define IVS_MEMSYS_NO_SLOT 0xFFFFFFFFu
uint32_t ivs_memsys_psg_write_slot_internal(const ivs_memsys *m, uint16_t addr);

/* Internal pull-down setter (plan 05-03, D-04; called from the input path in plan 05-05): stores
 * mask as the set of low-driven pins of PSG port 0 or 1. A port at or above IVS_MEMSYS_PSG_PORTS
 * does nothing. A guest read of a port that is in input mode returns the idle byte XOR this mask
 * (ivs_mm_port_read in memsys.c). */
void ivs_memsys_set_port_pulldown_internal(ivs_memsys *m, uint32_t port, uint32_t mask);

/* Raw-poke seam (A-05a): stores value unmasked into the backing word of addr (read view first,
 * then write view; the cartridge span stores at addr - span lo). No backing: nothing happens. */
void ivs_memsys_poke_raw_internal(ivs_memsys *m, uint16_t addr, uint16_t value);

/* The STIC's own view of one background memory word (plan 06-08, D-11): word i of BACKTAB is system RAM word $0200 + i for i below
 * 240, read from the backing store with no gate, no fill and no access record (never the CPU view). Any other index gives 0. */
uint16_t ivs_memsys_stic_backtab_word_internal(const ivs_memsys *m, uint32_t i);

/* The hook class of an access of kind IVS_ACC_* at addr: FETCH, READ and ADAR (the operand read of a direct-addressed
 * instruction, which the memory system serves as a read) look in the read table, WRITE and DW in the write table, every
 * other kind is NONE. O(1), pure, depends only on the profile (plan 06-06). */
uint32_t ivs_memsys_hook_class_internal(const ivs_memsys *m, uint32_t kind, uint32_t addr);

/* Decode introspection (A-05b). view is IVS_MEMSYS_VIEW_R or IVS_MEMSYS_VIEW_W. Returns IVS_OK,
 * or BAD_ARG for NULL out, a bad profile index or a bad view (out untouched). */
int ivs_memsys_describe(uint32_t profile, uint32_t view, uint16_t addr, ivs_memsys_desc *out);

#endif
