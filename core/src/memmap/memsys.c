/* SPDX-License-Identifier: MIT */
/* Profile-driven memory system (D-16..D-20, A-05, A-08 c, plan 03-04). The decode tables,
 * device records and profile registry come from ivs_profiles.inc, generated from
 * the profile files (data/profiles) by cmake/IvsMemmapTable.cmake; this file is their only includer, and it
 * restates no region (D-16). Width values rest on the profile's device records: rom10 and grom8
 * read zero in the upper bits (GI 1980 catalog pdf 194, 197, 202), ram16 keeps 16 bits (pdf 199),
 * ram8 and stub reads are chosen defaults (C60), undriven reads are the profile fill (C5). The PSG
 * region (plan 05-04) is the device kind psg8: its register file lives in psg_reg and is read and
 * written through ivs_psg_reg_load and ivs_psg_reg_store with the masks of the profile's psg.regmap;
 * the width formula then gives the upper byte the psg8 fill, 0 (C64). Plan 05-05 (D-04): the two port
 * registers $01FE and $01FF (slots 14 and 15) read from the pins: with the direction bit of the mixer
 * register selecting input the low byte is the idle byte XOR the port's pull-down mask
 * (port_pulldown, set only by the input path), with it selecting output the stored latch; every bit
 * number and the idle byte come from ivs_psg_params, a store always keeps the latch.
 * Named fault CTRL_NO_INVERSION (plan 05-05): the input-mode read returns the pull-down mask itself.
 *
 * Read path: rd_region[addr >> 4] names a region, the cartridge span or nothing. A region of a
 * backed device reads its backing at (addr - lo) & (size_words - 1); the cartridge span scans
 * the committed segment table (at most 16 entries) and reads cart[addr - span lo] through the
 * rom10 record inside a segment. Every result is one call of ivs_mm_width (D-17). A closed gate
 * and an EXEC or GROM region with no image read the undriven fill (D-19, D-B7).
 * Write path: wr_region[addr >> 4]; the device's store policy keeps value & rmask or drops it;
 * a closed gate drops it. Dispatch is a switch on the device or physical id; there are no
 * function pointers and no heap (D-19). A segment table replaces D-20's per-256-word page table
 * because A-01 allows a segment to start at any address (A-08 c). */
#include "common/ivs_prelude.h"

#include "memmap/memsys.h"
#include "psg/psg.h"

/* Generated record types (the generator writes initializers in this field order). */
typedef struct ivs_mm_region {
  uint16_t lo;
  uint16_t hi;
  uint16_t size_words; /* backing size, a power of two */
  uint8_t view;        /* 1 R, 2 W, 3 RW */
  uint8_t device;      /* IVS_MEMSYS_DEVICE_* */
  uint8_t gate;        /* IVS_MEMSYS_GATE_* */
  uint8_t impl;        /* IVS_MEMSYS_IMPL_* */
  uint8_t phys;        /* IVS_MEMSYS_PHYS_* */
  uint8_t reserved;
} ivs_mm_region;

typedef struct ivs_mm_device {
  uint16_t rmask;
  uint16_t fill;
  uint8_t keep; /* 1: a store keeps value & rmask; 0: dropped */
  uint8_t reserved;
} ivs_mm_device;

/* Granule markers in the generated tables. */
#define IVS_MM_UNDRIVEN 0xFFu
#define IVS_MM_CART 0xFEu

#include "ivs_profiles.inc"

uint32_t ivs_profile_count(void)
{
  return IVS_MM_PROFILE_COUNT;
}

const ivs_profile_desc *ivs_profile_get(uint32_t index)
{
  if (index >= IVS_MM_PROFILE_COUNT) {
    return NULL;
  }
  return &ivs_mm_profiles[index];
}

int ivs_memsys_init(ivs_memsys *m, uint32_t profile)
{
  /* A profile without a row in the PSG parameter table is refused here, so every access to the PSG
   * region below finds its parameters (review WR-04). */
  if (m == NULL || profile >= IVS_MM_PROFILE_COUNT || profile >= ivs_psg_params_count()) {
    return IVS_E_BAD_ARG;
  }
  memset(m, 0, sizeof *m);
  m->profile = profile;
  m->gate_open[IVS_MEMSYS_GATE_NONE] = 1u;
  m->gate_open[IVS_MEMSYS_GATE_STIC_REGS] = 1u;
  m->gate_open[IVS_MEMSYS_GATE_GMEM] = 1u;
  return IVS_OK;
}

/* The one width formula (D-17). */
static uint16_t ivs_mm_width(uint16_t stored, uint16_t rmask, uint16_t fill)
{
  return IVS_U16(((uint32_t)stored & rmask) | ((uint32_t)fill & ~(uint32_t)rmask));
}

/* The profile's undriven fill (memmap.undriven_read, C5). */
static uint16_t ivs_mm_undriven(const ivs_memsys *m)
{
  uint16_t v = ivs_mm_profiles[m->profile].undriven_fill;
#ifdef IVS_FAULT_MEM_UNDRIVEN_ZERO
  v = 0x0000u;
#endif
  return v;
}

/* Backing index of addr in a region: always below size_words. */
static uint32_t ivs_mm_index(const ivs_mm_region *r, uint16_t addr)
{
  uint32_t i = ((uint32_t)addr - r->lo) & ((uint32_t)r->size_words - 1u);
#ifdef IVS_FAULT_MEM_MIRROR_MASK
  i = ((uint32_t)addr - r->lo) & (((uint32_t)r->size_words - 1u) >> 1);
#endif
  return i;
}

/* The low byte a bus read of PSG port 0 or 1 returns (D-04, plan 05-05): pure. */
static uint16_t ivs_mm_port_read(const ivs_memsys *m, const ivs_psg_params *pp, uint32_t port)
{
  uint32_t mixer = ivs_psg_reg_load(pp, m->psg_reg, IVS_PSG_SLOT_MIXER);
  uint32_t dir = (mixer >> pp->port_dir_bit[port]) & 1u;
  uint32_t low;
  if (dir == pp->port_dir_output && pp->port_output_read_latch != 0u) {
    low = ivs_psg_reg_load(pp, m->psg_reg, IVS_PSG_SLOT_PORT0 + port); /* output mode: the latch */
  } else {
    low = (uint32_t)pp->port_idle ^ (uint32_t)m->port_pulldown[port]; /* input mode: the pins */
#ifdef IVS_FAULT_CTRL_NO_INVERSION
    low = (uint32_t)m->port_pulldown[port];
#endif
  }
  return IVS_U16(low & 0x00FFu);
}

static uint16_t ivs_mm_load(const ivs_memsys *m, uint8_t phys, uint32_t i)
{
  switch (phys) {
  case IVS_MEMSYS_PHYS_SCRATCHPAD:
    return m->scratchpad[i];
  case IVS_MEMSYS_PHYS_SYSRAM:
    return m->sysram[i];
  case IVS_MEMSYS_PHYS_EXEC:
    return m->exec[i];
  case IVS_MEMSYS_PHYS_GROM:
    return m->grom[i];
  case IVS_MEMSYS_PHYS_GRAM:
    return m->gram[i];
  case IVS_MEMSYS_PHYS_PSG: {
    const ivs_psg_params *pp = ivs_psg_params_get(m->profile); /* a row exists: ivs_memsys_init refused any other profile */
    if (i == IVS_PSG_SLOT_PORT0 || i == IVS_PSG_SLOT_PORT1) {
      return ivs_mm_port_read(m, pp, i - IVS_PSG_SLOT_PORT0);
    }
    return ivs_psg_reg_load(pp, m->psg_reg, i);
  }
  default:
    return 0u; /* stub devices hold no state */
  }
}

static void ivs_mm_store(ivs_memsys *m, uint8_t phys, uint32_t i, uint16_t v)
{
  switch (phys) {
  case IVS_MEMSYS_PHYS_SCRATCHPAD:
    m->scratchpad[i] = v;
    break;
  case IVS_MEMSYS_PHYS_SYSRAM:
    m->sysram[i] = v;
    break;
  case IVS_MEMSYS_PHYS_EXEC:
    m->exec[i] = v;
    break;
  case IVS_MEMSYS_PHYS_GROM:
    m->grom[i] = v;
    break;
  case IVS_MEMSYS_PHYS_GRAM:
    m->gram[i] = v;
    break;
  case IVS_MEMSYS_PHYS_PSG: {
    /* a row exists: ivs_memsys_init refused any other profile */
    ivs_psg_reg_store(ivs_psg_params_get(m->profile), m->psg_reg, i, v); /* keeps v & the register's write mask */
    break;
  }
  default:
    break; /* stub devices hold no state */
  }
}

/* 1 when the physical device holds content (EXEC and GROM need a loaded image). */
static uint8_t ivs_mm_present(const ivs_memsys *m, uint8_t phys)
{
  switch (phys) {
  case IVS_MEMSYS_PHYS_EXEC:
    return m->exec_loaded;
  case IVS_MEMSYS_PHYS_GROM:
    return m->grom_loaded;
  default:
    return 1u;
  }
}

/* Read of a region index (IVS_MM_UNDRIVEN or any index past the array: undriven). */
static uint16_t ivs_mm_region_read(const ivs_memsys *m, uint8_t idx, uint16_t addr)
{
  uint16_t stored = 0u;
  uint16_t rmask = 0u;
  uint16_t fill = ivs_mm_undriven(m);
  if (idx < IVS_MM_REGION_COUNT) {
    const ivs_mm_region *r = &ivs_mm_regions[idx];
    const ivs_mm_device *d = &ivs_mm_devices[m->profile][r->device];
    uint8_t open = m->gate_open[r->gate];
#ifdef IVS_FAULT_MEM_GATE_IGNORED
    open = 1u;
#endif
    if (open != 0u && ivs_mm_present(m, r->phys) != 0u) {
      stored = ivs_mm_load(m, r->phys, ivs_mm_index(r, addr));
      rmask = d->rmask;
      fill = d->fill;
#ifdef IVS_FAULT_MEM_ROM_NO_MASK
      if (r->device == IVS_MEMSYS_DEVICE_ROM10) {
        rmask = 0xFFFFu;
      }
#endif
    }
  }
  return ivs_mm_width(stored, rmask, fill);
}

/* Read inside the cartridge span: a bounded scan of the committed segments (A-08 c). */
static uint16_t ivs_mm_cart_read(const ivs_memsys *m, uint16_t addr)
{
  const ivs_profile_desc *p = &ivs_mm_profiles[m->profile];
  const ivs_mm_device *d = &ivs_mm_devices[m->profile][IVS_MEMSYS_DEVICE_ROM10];
  uint16_t stored = 0u;
  uint16_t rmask = 0u;
  uint16_t fill = ivs_mm_undriven(m);
  uint8_t found = 0u;
  uint32_t k;
  uint16_t v;
  for (k = 0u; k < IVS_MEMSYS_CART_SEGMENTS && k < m->cart_count; k++) {
    uint32_t a = m->cart_seg[k].addr;
    uint32_t end = a + m->cart_seg[k].len_words;
    uint8_t in = (uint8_t)((uint32_t)addr >= a && (uint32_t)addr < end);
#ifdef IVS_FAULT_MEM_CART_END_INCLUSIVE
    in = (uint8_t)((uint32_t)addr >= a && (uint32_t)addr <= end);
#endif
    if (in != 0u && found == 0u) {
      stored = m->cart[(uint32_t)addr - p->cart_lo];
      rmask = d->rmask;
      fill = d->fill;
      found = 1u;
    }
  }
  v = ivs_mm_width(stored, rmask, fill);
#ifdef IVS_FAULT_MEM_WO_READS_DEVICE
  v = (found != 0u) ? v : ivs_mm_region_read(m, ivs_mm_wr_region[m->profile][addr >> 4u], addr);
#endif
  return v;
}

uint16_t ivs_memsys_read(const ivs_memsys *m, uint16_t addr)
{
  uint8_t idx = ivs_mm_rd_region[m->profile][addr >> 4u];
  if (idx == IVS_MM_CART) {
    return ivs_mm_cart_read(m, addr);
  }
  return ivs_mm_region_read(m, idx, addr);
}

void ivs_memsys_write(ivs_memsys *m, uint16_t addr, uint16_t value)
{
  uint8_t idx = ivs_mm_wr_region[m->profile][addr >> 4u];
  if (idx < IVS_MM_REGION_COUNT) {
    const ivs_mm_region *r = &ivs_mm_regions[idx];
    const ivs_mm_device *d = &ivs_mm_devices[m->profile][r->device];
    uint16_t v = IVS_U16((uint32_t)value & d->rmask);
#ifdef IVS_FAULT_MEM_RAM8_NO_TRUNC
    if (r->device == IVS_MEMSYS_DEVICE_RAM8) {
      v = value;
    }
#endif
#ifdef IVS_FAULT_MEM_RAM16_TRUNC
    if (r->device == IVS_MEMSYS_DEVICE_RAM16) {
      v = IVS_U16((uint32_t)value & 0x00FFu);
    }
#endif
    if (d->keep != 0u && m->gate_open[r->gate] != 0u) {
      ivs_mm_store(m, r->phys, ivs_mm_index(r, addr), v);
    }
  }
#ifdef IVS_FAULT_MEM_DROPPED_WRITE_KEPT
  /* a store to the undriven $0080-$008F, inside a width-0 write row and away from its edges, is
   * kept in the system RAM words of $0210-$021F; the decode description does not change */
  if (idx >= IVS_MM_REGION_COUNT && (addr & 0xFFF0u) == 0x0080u) {
    ivs_mm_store(m, IVS_MEMSYS_PHYS_SYSRAM, 0x10u + (uint32_t)(addr & 0x000Fu), value);
  }
#endif
#ifdef IVS_FAULT_MEM_ROM_WRITABLE
  if (idx >= IVS_MM_REGION_COUNT) {
    uint8_t ri = ivs_mm_rd_region[m->profile][addr >> 4u];
    if (ri < IVS_MM_REGION_COUNT && ivs_mm_regions[ri].phys == IVS_MEMSYS_PHYS_EXEC) {
      ivs_mm_store(m, IVS_MEMSYS_PHYS_EXEC, ivs_mm_index(&ivs_mm_regions[ri], addr), value);
    }
  }
#endif
}

/* IAB answer by reason flag (D-20): the start vector for a reset, the interrupt vector for an
 * interrupt; with neither flag no device answers and the bus floats. */
static uint16_t ivs_mm_iab(const ivs_memsys *m, uint8_t flags)
{
  const ivs_profile_desc *p = &ivs_mm_profiles[m->profile];
  uint16_t reset = p->reset_r7;
  uint16_t intr = p->iab_value;
#ifdef IVS_FAULT_MEM_IAB_VECTOR_SWAPPED
  reset = p->iab_value;
  intr = p->reset_r7;
#endif
  if ((flags & IVS_ACC_F_RESET) != 0u) {
    return reset;
  }
  if ((flags & IVS_ACC_F_INTERRUPT) != 0u) {
    return intr;
  }
  return ivs_mm_undriven(m);
}

ivs_bus_reply ivs_memsys_bus(void *ctx, const ivs_cp1610_access *a)
{
  ivs_memsys *m = (ivs_memsys *)ctx;
  ivs_bus_reply reply;
  reply.data = 0u;
  reply.wait_cpu_cycles = 0u;
  switch (a->kind) {
  case IVS_ACC_FETCH:
  case IVS_ACC_READ:
  case IVS_ACC_ADAR:
    reply.data = ivs_memsys_read(m, a->addr);
    break;
  case IVS_ACC_WRITE:
  case IVS_ACC_DW:
    ivs_memsys_write(m, a->addr, a->data);
    break;
  case IVS_ACC_IAB:
    reply.data = ivs_mm_iab(m, a->flags);
    break;
  default:
    break; /* INTAK and the strobes transfer no data */
  }
  return reply;
}

void ivs_memsys_install_exec(ivs_memsys *m, const uint8_t *bytes)
{
  uint32_t i;
  for (i = 0u; i < IVS_MEMSYS_EXEC_WORDS; i++) {
    m->exec[i] = IVS_U16(((uint32_t)bytes[2u * i] << 8) | bytes[2u * i + 1u]);
  }
  m->exec_loaded = 1u;
}

void ivs_memsys_install_grom(ivs_memsys *m, const uint8_t *bytes)
{
  uint32_t i;
  for (i = 0u; i < IVS_MEMSYS_GROM_WORDS; i++) {
    m->grom[i] = bytes[i];
  }
  m->grom_loaded = 1u;
}

void ivs_memsys_install_cart_segment(ivs_memsys *m, uint16_t addr, const uint8_t *be_words,
                                     uint16_t len_words)
{
  const ivs_profile_desc *p = &ivs_mm_profiles[m->profile];
  uint32_t i;
  for (i = 0u; i < len_words; i++) {
    uint32_t at = (uint32_t)addr + i;
    if (p->has_cart_span != 0u && at >= p->cart_lo && at <= p->cart_hi) {
      m->cart[at - p->cart_lo] = IVS_U16(((uint32_t)be_words[2u * i] << 8) | be_words[2u * i + 1u]);
    }
  }
  if (m->cart_count < IVS_MEMSYS_CART_SEGMENTS) {
    m->cart_seg[m->cart_count].addr = addr;
    m->cart_seg[m->cart_count].len_words = len_words;
    m->cart_count = IVS_U16(m->cart_count + 1u);
  }
}

void ivs_memsys_clear_slot(ivs_memsys *m, uint32_t slot)
{
  switch (slot) {
  case IVS_MEMSYS_SLOT_EXEC:
    memset(m->exec, 0, sizeof m->exec);
    m->exec_loaded = 0u;
    break;
  case IVS_MEMSYS_SLOT_GROM:
    memset(m->grom, 0, sizeof m->grom);
    m->grom_loaded = 0u;
    break;
  case IVS_MEMSYS_SLOT_CART:
    memset(m->cart, 0, sizeof m->cart);
    memset(m->cart_seg, 0, sizeof m->cart_seg);
    m->cart_count = 0u;
    break;
  default:
    break;
  }
}

void ivs_memsys_set_gate_internal(ivs_memsys *m, uint32_t gate, uint8_t open)
{
  if (gate != IVS_MEMSYS_GATE_NONE && gate < IVS_MEMSYS_GATES) {
    m->gate_open[gate] = (uint8_t)(open != 0u);
  }
}

uint16_t ivs_memsys_psg_reg_internal(const ivs_memsys *m, uint32_t idx)
{
  return ivs_psg_reg_load(ivs_psg_params_get(m->profile), m->psg_reg, idx); /* a row exists, see ivs_memsys_init */
}

uint32_t ivs_memsys_psg_write_slot_internal(const ivs_memsys *m, uint16_t addr)
{
  uint8_t idx = ivs_mm_wr_region[m->profile][addr >> 4u];
  if (idx < IVS_MM_REGION_COUNT && ivs_mm_regions[idx].phys == IVS_MEMSYS_PHYS_PSG) {
    return ivs_mm_index(&ivs_mm_regions[idx], addr) & (IVS_MEMSYS_PSG_WORDS - 1u);
  }
  return IVS_MEMSYS_NO_SLOT;
}

void ivs_memsys_set_port_pulldown_internal(ivs_memsys *m, uint32_t port, uint32_t mask)
{
  if (port < IVS_MEMSYS_PSG_PORTS) {
    m->port_pulldown[port] = (uint8_t)(mask & 0xFFu);
  }
}

void ivs_memsys_poke_raw_internal(ivs_memsys *m, uint16_t addr, uint16_t value)
{
  const ivs_profile_desc *p = &ivs_mm_profiles[m->profile];
  uint8_t idx = ivs_mm_rd_region[m->profile][addr >> 4u];
  if (idx == IVS_MM_CART) {
    m->cart[(uint32_t)addr - p->cart_lo] = value;
    return;
  }
  if (idx >= IVS_MM_REGION_COUNT) {
    idx = ivs_mm_wr_region[m->profile][addr >> 4u];
  }
  if (idx < IVS_MM_REGION_COUNT) {
    const ivs_mm_region *r = &ivs_mm_regions[idx];
    if (r->phys == IVS_MEMSYS_PHYS_PSG) {
      /* the raw seam stores the whole word, as for every other backing; the read mask applies on
       * read (plan 05-04) */
      m->psg_reg[ivs_mm_index(r, addr) & (IVS_MEMSYS_PSG_WORDS - 1u)] = value;
      return;
    }
    ivs_mm_store(m, r->phys, ivs_mm_index(r, addr), value);
  }
}

static const char *ivs_mm_phys_name(uint8_t phys)
{
  switch (phys) {
  case IVS_MEMSYS_PHYS_STIC:
    return "stic";
  case IVS_MEMSYS_PHYS_SCRATCHPAD:
    return "scratchpad";
  case IVS_MEMSYS_PHYS_PSG:
    return "psg";
  case IVS_MEMSYS_PHYS_SYSRAM:
    return "sysram";
  case IVS_MEMSYS_PHYS_EXEC:
    return "exec";
  case IVS_MEMSYS_PHYS_GROM:
    return "grom";
  case IVS_MEMSYS_PHYS_GRAM:
    return "gram";
  case IVS_MEMSYS_PHYS_CART:
    return "cart";
  default:
    return "undriven";
  }
}

/* 1 when region idx is an alias: another region of the same profile (one that the profile's
 * read or write table places at its lo) reaches the same physical device from a lower lo. The
 * literal map's mirror column (plan 03-05 header) names the shared register file or backing on
 * every alias, also when the alias is no larger than it. */
static uint8_t ivs_mm_is_alias(uint32_t profile, uint8_t idx)
{
  const ivs_mm_region *r = &ivs_mm_regions[idx];
  uint32_t j;
  for (j = 0u; j < IVS_MM_REGION_COUNT; j++) {
    const ivs_mm_region *o = &ivs_mm_regions[j];
    uint32_t g = (uint32_t)o->lo >> 4u;
    if (j != idx && o->phys == r->phys && o->lo < r->lo
        && (ivs_mm_rd_region[profile][g] == j || ivs_mm_wr_region[profile][g] == j)) {
      return 1u;
    }
  }
  return 0u;
}

int ivs_memsys_describe(uint32_t profile, uint32_t view, uint16_t addr, ivs_memsys_desc *out)
{
  ivs_memsys_desc d;
  uint8_t idx;
  if (out == NULL || profile >= IVS_MM_PROFILE_COUNT
      || (view != IVS_MEMSYS_VIEW_R && view != IVS_MEMSYS_VIEW_W)) {
    return IVS_E_BAD_ARG;
  }
  memset(&d, 0, sizeof d);
  idx = (view == IVS_MEMSYS_VIEW_R) ? ivs_mm_rd_region[profile][addr >> 4u]
                                    : ivs_mm_wr_region[profile][addr >> 4u];
  if (idx == IVS_MM_CART) {
    d.phys = IVS_MEMSYS_PHYS_CART;
    d.device = IVS_MEMSYS_DEVICE_ROM10;
    d.impl = IVS_MEMSYS_IMPL_REAL;
  } else if (idx < IVS_MM_REGION_COUNT) {
    const ivs_mm_region *r = &ivs_mm_regions[idx];
    uint32_t extent = (uint32_t)r->hi - r->lo + 1u;
    d.phys = r->phys;
    d.device = r->device;
    d.impl = r->impl;
    d.gate = r->gate;
    d.mirror_words = (extent > r->size_words || ivs_mm_is_alias(profile, idx) != 0u) ? r->size_words : 0u;
  }
  d.phys_name = ivs_mm_phys_name(d.phys);
  *out = d;
  return IVS_OK;
}
