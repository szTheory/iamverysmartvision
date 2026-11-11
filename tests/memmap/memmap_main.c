/* SPDX-License-Identifier: MIT */
/* Suite memmap: the profile-driven memory system (D-16..D-20, plan 03-04), driven through its
 * bus function. Expected values come from this file's rule table, keyed on device kind and
 * width, and from addresses written here by hand; never from the generated descriptors.
 * All image words are generated stand-ins built in the test; no rights-bearing bytes. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cpu/cp1610.h"
#include "ivs_test.h"
#include "memmap/memsys.h"
#include "memmap_rows.h"

/* Fails the case with a diagnostic naming the line and the failed expression. */
#define IVS_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

/* Rule table (D-17, D-B3, D-B9), keyed on device kind: the read of a stored word is
 * (stored & rmask) | (fill & ~rmask); a store keeps value & rmask when keep is 1. Values are
 * written here from the decisions, not read from the generated descriptors. */
typedef struct mm_rule {
  uint8_t kind;
  uint16_t rmask;
  uint16_t fill;
  uint8_t keep;
} mm_rule;

static const mm_rule mm_rules[] = {
    {IVS_MEMSYS_DEVICE_ROM10, 0x03FFu, 0x0000u, 0u}, /* 10-bit decle, upper bits driven 0 */
    {IVS_MEMSYS_DEVICE_GROM8, 0x00FFu, 0x0000u, 0u}, /* 8-bit GROM, upper bits driven 0 */
    {IVS_MEMSYS_DEVICE_RAM8, 0x00FFu, 0x0000u, 1u},  /* 8-bit RAM, upper byte 0 (C60) */
    {IVS_MEMSYS_DEVICE_RAM16, 0xFFFFu, 0x0000u, 1u}, /* 16-bit system RAM */
    {IVS_MEMSYS_DEVICE_STUB, 0x0000u, 0xFFFFu, 0u},  /* no state, undriven value (C60) */
};

#define MM_UNDRIVEN 0xFFFFu

static const mm_rule *mm_rule_of(uint8_t kind)
{
  size_t i;
  for (i = 0; i < sizeof mm_rules / sizeof mm_rules[0]; i++) {
    if (mm_rules[i].kind == kind) {
      return &mm_rules[i];
    }
  }
  return NULL;
}

static uint16_t mm_expect(uint8_t kind, uint16_t stored)
{
  const mm_rule *r = mm_rule_of(kind);
  if (r == NULL) {
    return MM_UNDRIVEN;
  }
  return (uint16_t)((stored & r->rmask) | (r->fill & (uint16_t)~r->rmask));
}

/* Regions of the first profile, written by hand from the phase 3 context table (D-18). A
 * write-only alias of a backed device names the read address of its backing (obs). */
#define MM_R 1u
#define MM_W 2u
#define MM_RW 3u
typedef struct mm_row {
  const char *slug;
  uint8_t view;
  uint8_t kind;
  uint16_t lo;
  uint16_t hi;
  uint16_t obs; /* write-only alias: read address of backing word 0, else 0 */
  uint16_t mirror;
} mm_row;

static const mm_row mm_rows[] = {
    {"stic-regs", MM_RW, IVS_MEMSYS_DEVICE_STUB, 0x0000u, 0x003Fu, 0u, 0u},
    {"stic-alias-read", MM_R, IVS_MEMSYS_DEVICE_STUB, 0x0040u, 0x007Fu, 0u, 0u},
    {"scratchpad", MM_RW, IVS_MEMSYS_DEVICE_RAM8, 0x0100u, 0x01EFu, 0u, 0u},
    {"psg-regs", MM_RW, IVS_MEMSYS_DEVICE_STUB, 0x01F0u, 0x01FFu, 0u, 0u},
    {"sysram", MM_RW, IVS_MEMSYS_DEVICE_RAM16, 0x0200u, 0x035Fu, 0u, 0u},
    {"exec", MM_R, IVS_MEMSYS_DEVICE_ROM10, 0x1000u, 0x1FFFu, 0u, 0u},
    {"grom", MM_R, IVS_MEMSYS_DEVICE_GROM8, 0x3000u, 0x37FFu, 0u, 0u},
    {"gram", MM_RW, IVS_MEMSYS_DEVICE_RAM8, 0x3800u, 0x3FFFu, 0u, 512u},
    {"stic-alias-w4", MM_W, IVS_MEMSYS_DEVICE_STUB, 0x4000u, 0x403Fu, 0u, 0u},
    {"gram-alias-w7", MM_W, IVS_MEMSYS_DEVICE_RAM8, 0x7800u, 0x7FFFu, 0x3800u, 512u},
    {"stic-alias-w8", MM_W, IVS_MEMSYS_DEVICE_STUB, 0x8000u, 0x803Fu, 0u, 0u},
    {"gram-alias-wb", MM_W, IVS_MEMSYS_DEVICE_RAM8, 0xB800u, 0xBFFFu, 0x3800u, 512u},
    {"stic-alias-wc", MM_W, IVS_MEMSYS_DEVICE_STUB, 0xC000u, 0xC03Fu, 0u, 0u},
    {"gram-alias-wf", MM_W, IVS_MEMSYS_DEVICE_RAM8, 0xF800u, 0xFFFFu, 0x3800u, 512u},
};
#define MM_ROWS (sizeof mm_rows / sizeof mm_rows[0])

static ivs_memsys mm_m;
static uint8_t mm_exec_img[IVS_MEMSYS_EXEC_BYTES];
static uint8_t mm_grom_img[IVS_MEMSYS_GROM_BYTES];

static int mm_init(void)
{
  return ivs_memsys_init(&mm_m, 0u) == IVS_OK;
}

static ivs_bus_reply mm_bus(uint8_t kind, uint8_t flags, uint16_t addr, uint16_t data)
{
  ivs_cp1610_access a;
  memset(&a, 0, sizeof a);
  a.kind = kind;
  a.flags = flags;
  a.addr = addr;
  a.data = data;
  return ivs_memsys_bus(&mm_m, &a);
}

/* Bus READ; a reply with a wait gives 0x5A5A, which no case expects (wait is 0, D-20). */
static uint16_t mm_bus_read(uint16_t addr)
{
  ivs_bus_reply r = mm_bus(IVS_ACC_READ, 0u, addr, 0u);
  return r.wait_cpu_cycles == 0u ? r.data : 0x5A5Au;
}

static int mm_bus_write(uint16_t addr, uint16_t value)
{
  ivs_bus_reply r = mm_bus(IVS_ACC_WRITE, 0u, addr, value);
  return r.wait_cpu_cycles == 0u && r.data == 0u;
}

/* A stand-in EXEC image: word i is i ^ 0x0155 (10 bits), word 0 is 0xFFFF (wide). */
static void mm_build_exec(void)
{
  uint32_t i;
  for (i = 0u; i < IVS_MEMSYS_EXEC_WORDS; i++) {
    uint16_t w = (uint16_t)((i ^ 0x0155u) & 0x03FFu);
    if (i == 0u) {
      w = 0xFFFFu;
    }
    mm_exec_img[2u * i] = (uint8_t)(w >> 8);
    mm_exec_img[2u * i + 1u] = (uint8_t)(w & 0xFFu);
  }
}

/* A stand-in GROM image: byte i is (i * 7) & 0xFF. */
static void mm_build_grom(void)
{
  uint32_t i;
  for (i = 0u; i < IVS_MEMSYS_GROM_BYTES; i++) {
    mm_grom_img[i] = (uint8_t)((i * 7u) & 0xFFu);
  }
}

/* Fresh memory system with stand-in EXEC and GROM images loaded. */
static int mm_init_loaded(void)
{
  if (!mm_init()) {
    return 0;
  }
  mm_build_exec();
  mm_build_grom();
  ivs_memsys_install_exec(&mm_m, mm_exec_img);
  ivs_memsys_install_grom(&mm_m, mm_grom_img);
  return 1;
}

/* One anchor probe at addr of a row, by the rule table:
 *   read-only ROM: a raw wide word reads masked; a bus write is dropped;
 *   readable RAM: a bus write of 0x1234 reads back by the rule; 0xFFFF too;
 *   stub: a bus write is dropped and reads give the undriven value;
 *   write-only alias of RAM: the write lands in the backing (read at obs), the alias address
 *   itself reads undriven (no cartridge segment there);
 *   write-only stub alias: reads give the undriven value. */
static int mm_anchor(const mm_row *row, uint16_t addr)
{
  const mm_rule *r = mm_rule_of(row->kind);
  IVS_CHECK(r != NULL);
  IVS_CHECK(mm_init_loaded());
  if (row->view == MM_R && r->keep == 0u && row->kind != IVS_MEMSYS_DEVICE_STUB) {
    ivs_memsys_poke_raw_internal(&mm_m, addr, 0xFFFFu);
    IVS_CHECK(mm_bus_read(addr) == mm_expect(row->kind, 0xFFFFu));
    IVS_CHECK(mm_bus_write(addr, 0x0155u));
    IVS_CHECK(mm_bus_read(addr) == mm_expect(row->kind, 0xFFFFu));
  } else if (row->view == MM_RW && r->keep != 0u) {
    IVS_CHECK(mm_bus_write(addr, 0x1234u));
    IVS_CHECK(mm_bus_read(addr) == mm_expect(row->kind, 0x1234u));
    IVS_CHECK(mm_bus_write(addr, 0xFFFFu));
    IVS_CHECK(mm_bus_read(addr) == mm_expect(row->kind, 0xFFFFu));
  } else if (row->view == MM_W && r->keep != 0u) {
    uint16_t obs = (uint16_t)(row->obs + ((uint16_t)(addr - row->lo) & (uint16_t)(row->mirror - 1u)));
    IVS_CHECK(mm_bus_read(obs) == 0x0000u);
    IVS_CHECK(mm_bus_write(addr, 0x1234u));
    IVS_CHECK(mm_bus_read(obs) == mm_expect(row->kind, 0x1234u));
    IVS_CHECK(mm_bus_read(addr) == MM_UNDRIVEN);
  } else {
    IVS_CHECK(mm_bus_write(addr, 0x1234u));
    IVS_CHECK(mm_bus_read(addr) == MM_UNDRIVEN);
    IVS_CHECK(mm_bus_read(addr) == mm_expect(IVS_MEMSYS_DEVICE_STUB, 0x1234u));
  }
  return IVS_TEST_PASS;
}

/* ---- width formula with wide stored values (D-17) ---- */

static int mm_width_rom10(void)
{
  IVS_CHECK(mm_init_loaded());
  ivs_memsys_poke_raw_internal(&mm_m, 0x1001u, 0xFFFFu);
  IVS_CHECK(mm_bus_read(0x1001u) == 0x03FFu);
  ivs_memsys_poke_raw_internal(&mm_m, 0x1002u, 0xFC00u);
  IVS_CHECK(mm_bus_read(0x1002u) == 0x0000u);
  return IVS_TEST_PASS;
}

static int mm_width_grom8(void)
{
  IVS_CHECK(mm_init_loaded());
  ivs_memsys_poke_raw_internal(&mm_m, 0x3001u, 0xFFFFu);
  IVS_CHECK(mm_bus_read(0x3001u) == 0x00FFu);
  return IVS_TEST_PASS;
}

static int mm_width_ram16(void)
{
  IVS_CHECK(mm_init());
  IVS_CHECK(mm_bus_write(0x0300u, 0xFFFFu));
  IVS_CHECK(mm_bus_read(0x0300u) == 0xFFFFu);
  IVS_CHECK(mm_bus_write(0x0301u, 0xA5C3u));
  IVS_CHECK(mm_bus_read(0x0301u) == 0xA5C3u);
  return IVS_TEST_PASS;
}

static int mm_width_ram8(void)
{
  IVS_CHECK(mm_init());
  IVS_CHECK(mm_bus_write(0x0110u, 0x1234u));
  IVS_CHECK(mm_bus_read(0x0110u) == 0x0034u);
  ivs_memsys_poke_raw_internal(&mm_m, 0x0111u, 0xFFFFu);
  IVS_CHECK(mm_bus_read(0x0111u) == 0x00FFu);
  return IVS_TEST_PASS;
}

static int mm_width_stub(void)
{
  IVS_CHECK(mm_init());
  IVS_CHECK(mm_bus_write(0x0001u, 0x1234u));
  IVS_CHECK(mm_bus_read(0x0001u) == 0xFFFFu);
  IVS_CHECK(mm_bus_write(0x01F1u, 0x00AAu));
  IVS_CHECK(mm_bus_read(0x01F1u) == 0xFFFFu);
  ivs_memsys_poke_raw_internal(&mm_m, 0x01F2u, 0x1234u);
  IVS_CHECK(mm_bus_read(0x01F2u) == 0xFFFFu);
  return IVS_TEST_PASS;
}

static int mm_width_undriven(void)
{
  static const uint16_t gaps[] = {0x0080u, 0x00FFu, 0x0360u, 0x0FFFu, 0x2000u, 0x2FFFu};
  size_t i;
  IVS_CHECK(mm_init_loaded());
  for (i = 0; i < sizeof gaps / sizeof gaps[0]; i++) {
    IVS_CHECK(mm_bus_write(gaps[i], 0x1234u));
    IVS_CHECK(mm_bus_read(gaps[i]) == 0xFFFFu);
  }
  return IVS_TEST_PASS;
}

/* GRAM mirror (512-word backing over $3800-$3FFF): words written at $3800 and $3900 read back
 * one and two mirror steps up. */
static int mm_mirror_gram_3a00(void)
{
  IVS_CHECK(mm_init());
  IVS_CHECK(mm_bus_write(0x3800u, 0x0011u));
  IVS_CHECK(mm_bus_write(0x3900u, 0x0022u));
  IVS_CHECK(mm_bus_read(0x3A00u) == 0x0011u);
  IVS_CHECK(mm_bus_read(0x3B00u) == 0x0022u);
  IVS_CHECK(mm_bus_read(0x3E00u) == 0x0011u);
  IVS_CHECK(mm_bus_read(0x3FFFu) == 0x0000u);
  IVS_CHECK(mm_bus_write(0x3FFFu, 0x0033u));
  IVS_CHECK(mm_bus_read(0x39FFu) == 0x0033u);
  return IVS_TEST_PASS;
}

/* ---- gates (D-19) ---- */

static int mm_gate_gmem_closed(void)
{
  IVS_CHECK(mm_init_loaded());
  ivs_memsys_poke_raw_internal(&mm_m, 0x3000u, 0x00ABu);
  IVS_CHECK(mm_bus_write(0x3800u, 0x0012u));
  ivs_memsys_set_gate_internal(&mm_m, IVS_MEMSYS_GATE_GMEM, 0u);
  IVS_CHECK(mm_bus_read(0x3000u) == MM_UNDRIVEN);
  IVS_CHECK(mm_bus_read(0x3800u) == MM_UNDRIVEN);
  IVS_CHECK(mm_bus_write(0x3800u, 0x0099u));
  ivs_memsys_set_gate_internal(&mm_m, IVS_MEMSYS_GATE_GMEM, 1u);
  IVS_CHECK(mm_bus_read(0x3000u) == 0x00ABu);
  IVS_CHECK(mm_bus_read(0x3800u) == 0x0012u);
  return IVS_TEST_PASS;
}

static int mm_gate_stic_regs_scope(void)
{
  IVS_CHECK(mm_init_loaded());
  ivs_memsys_poke_raw_internal(&mm_m, 0x3001u, 0x00CDu);
  IVS_CHECK(mm_bus_write(0x0110u, 0x0021u));
  IVS_CHECK(mm_bus_write(0x0300u, 0x4321u));
  IVS_CHECK(mm_bus_write(0x3801u, 0x0042u));
  ivs_memsys_set_gate_internal(&mm_m, IVS_MEMSYS_GATE_STIC_REGS, 0u);
  IVS_CHECK(mm_bus_read(0x0000u) == MM_UNDRIVEN);
  IVS_CHECK(mm_bus_read(0x007Fu) == MM_UNDRIVEN);
  IVS_CHECK(mm_bus_read(0x0110u) == 0x0021u);
  IVS_CHECK(mm_bus_read(0x0300u) == 0x4321u);
  IVS_CHECK(mm_bus_read(0x3001u) == 0x00CDu);
  IVS_CHECK(mm_bus_read(0x3801u) == 0x0042u);
  IVS_CHECK(mm_bus_read(0x1001u) == (0x0001u ^ 0x0155u));
  IVS_CHECK(mm_bus_write(0x0110u, 0x0022u));
  IVS_CHECK(mm_bus_read(0x0110u) == 0x0022u);
  return IVS_TEST_PASS;
}

static int mm_gate_default_open(void)
{
  IVS_CHECK(mm_init_loaded());
  IVS_CHECK(mm_m.gate_open[IVS_MEMSYS_GATE_STIC_REGS] == 1u);
  IVS_CHECK(mm_m.gate_open[IVS_MEMSYS_GATE_GMEM] == 1u);
  IVS_CHECK(mm_bus_read(0x3002u) == (uint16_t)((2u * 7u) & 0xFFu));
  return IVS_TEST_PASS;
}

/* ---- unloaded content reads undriven, never 0000 (D-B7) ---- */

static int mm_unloaded_exec_reads_ffff(void)
{
  IVS_CHECK(mm_init());
  IVS_CHECK(mm_bus_read(0x1000u) == MM_UNDRIVEN);
  IVS_CHECK(mm_bus_read(0x1FFFu) == MM_UNDRIVEN);
  return IVS_TEST_PASS;
}

static int mm_unloaded_grom_reads_ffff(void)
{
  IVS_CHECK(mm_init());
  ivs_memsys_poke_raw_internal(&mm_m, 0x3000u, 0x00ABu);
  IVS_CHECK(mm_bus_read(0x3000u) == MM_UNDRIVEN);
  IVS_CHECK(mm_bus_read(0x37FFu) == MM_UNDRIVEN);
  return IVS_TEST_PASS;
}

static int mm_unloaded_cart_reads_ffff(void)
{
  IVS_CHECK(mm_init());
  IVS_CHECK(mm_bus_read(0x4000u) == MM_UNDRIVEN);
  IVS_CHECK(mm_bus_read(0x5000u) == MM_UNDRIVEN);
  IVS_CHECK(mm_bus_read(0xFFFFu) == MM_UNDRIVEN);
  return IVS_TEST_PASS;
}

static int mm_unloaded_after_clear(void)
{
  static const uint8_t word[2] = {0x01u, 0x23u};
  IVS_CHECK(mm_init_loaded());
  ivs_memsys_install_cart_segment(&mm_m, 0x5000u, word, 1u);
  IVS_CHECK(mm_bus_read(0x5000u) == 0x0123u);
  ivs_memsys_clear_slot(&mm_m, IVS_MEMSYS_SLOT_EXEC);
  ivs_memsys_clear_slot(&mm_m, IVS_MEMSYS_SLOT_GROM);
  ivs_memsys_clear_slot(&mm_m, IVS_MEMSYS_SLOT_CART);
  IVS_CHECK(mm_bus_read(0x1001u) == MM_UNDRIVEN);
  IVS_CHECK(mm_bus_read(0x3001u) == MM_UNDRIVEN);
  IVS_CHECK(mm_bus_read(0x5000u) == MM_UNDRIVEN);
  IVS_CHECK(mm_m.exec_loaded == 0u && mm_m.grom_loaded == 0u && mm_m.cart_count == 0u);
  return IVS_TEST_PASS;
}

/* ---- cartridge span: any start, any length, bounded segment scan (A-01, A-08 c) ---- */

static const uint8_t mm_cart_word[2] = {0x02u, 0xABu};

static int mm_cart_one_word(void)
{
  IVS_CHECK(mm_init());
  ivs_memsys_install_cart_segment(&mm_m, 0x5000u, mm_cart_word, 1u);
  IVS_CHECK(mm_bus_read(0x5000u) == mm_expect(IVS_MEMSYS_DEVICE_ROM10, 0x02ABu));
  return IVS_TEST_PASS;
}

static int mm_cart_below(void)
{
  IVS_CHECK(mm_init());
  ivs_memsys_install_cart_segment(&mm_m, 0x5000u, mm_cart_word, 1u);
  IVS_CHECK(mm_bus_read(0x4FFFu) == MM_UNDRIVEN);
  return IVS_TEST_PASS;
}

static int mm_cart_above(void)
{
  IVS_CHECK(mm_init());
  ivs_memsys_install_cart_segment(&mm_m, 0x5000u, mm_cart_word, 1u);
  IVS_CHECK(mm_bus_read(0x5001u) == MM_UNDRIVEN);
  return IVS_TEST_PASS;
}

static int mm_cart_wide_masked(void)
{
  IVS_CHECK(mm_init());
  ivs_memsys_install_cart_segment(&mm_m, 0x5000u, mm_cart_word, 1u);
  ivs_memsys_poke_raw_internal(&mm_m, 0x5000u, 0xFFFFu);
  IVS_CHECK(mm_bus_read(0x5000u) == 0x03FFu);
  return IVS_TEST_PASS;
}

static int mm_cart_writes_ignored(void)
{
  IVS_CHECK(mm_init());
  ivs_memsys_install_cart_segment(&mm_m, 0x5000u, mm_cart_word, 1u);
  IVS_CHECK(mm_bus_write(0x5000u, 0x0111u));
  IVS_CHECK(mm_bus_read(0x5000u) == 0x02ABu);
  return IVS_TEST_PASS;
}

/* 16 segments of two words at $4100 + $0A00 * k (k = 0..15), each word (k << 4) | j; a 17th
 * segment is not recorded. */
static int mm_cart_sixteen_segments(void)
{
  uint8_t words[4];
  uint32_t k;
  IVS_CHECK(mm_init());
  for (k = 0u; k < 17u; k++) {
    uint16_t at = (uint16_t)(0x4100u + 0x0A00u * k);
    words[0] = 0x00u;
    words[1] = (uint8_t)(k << 4);
    words[2] = 0x00u;
    words[3] = (uint8_t)((k << 4) | 1u);
    ivs_memsys_install_cart_segment(&mm_m, at, words, 2u);
  }
  IVS_CHECK(mm_m.cart_count == IVS_MEMSYS_CART_SEGMENTS);
  for (k = 0u; k < 16u; k++) {
    uint16_t at = (uint16_t)(0x4100u + 0x0A00u * k);
    IVS_CHECK(mm_bus_read((uint16_t)(at - 1u)) == MM_UNDRIVEN);
    IVS_CHECK(mm_bus_read(at) == (uint16_t)(k << 4));
    IVS_CHECK(mm_bus_read((uint16_t)(at + 1u)) == (uint16_t)((k << 4) | 1u));
    IVS_CHECK(mm_bus_read((uint16_t)(at + 2u)) == MM_UNDRIVEN);
  }
  IVS_CHECK(mm_bus_read((uint16_t)(0x4100u + 0x0A00u * 16u)) == MM_UNDRIVEN);
  return IVS_TEST_PASS;
}

/* A segment under the GRAM write-only alias reads as ROM while writes reach GRAM (C61). */
static int mm_cart_under_alias(void)
{
  static const uint8_t words[4] = {0x01u, 0x23u, 0x00u, 0x45u};
  IVS_CHECK(mm_init());
  ivs_memsys_install_cart_segment(&mm_m, 0x7800u, words, 2u);
  IVS_CHECK(mm_bus_read(0x7800u) == 0x0123u);
  IVS_CHECK(mm_bus_write(0x7800u, 0x0055u));
  IVS_CHECK(mm_bus_read(0x7800u) == 0x0123u);
  IVS_CHECK(mm_bus_read(0x3800u) == 0x0055u);
  IVS_CHECK(mm_bus_read(0x7802u) == MM_UNDRIVEN);
  return IVS_TEST_PASS;
}

/* ---- bus port (D-20) ---- */

static int mm_bus_iab_reset(void)
{
  ivs_bus_reply r;
  IVS_CHECK(mm_init());
  r = mm_bus(IVS_ACC_IAB, IVS_ACC_F_RESET, 0x0000u, 0u);
  IVS_CHECK(r.data == 0x1000u && r.wait_cpu_cycles == 0u);
  return IVS_TEST_PASS;
}

static int mm_bus_iab_interrupt(void)
{
  ivs_bus_reply r;
  IVS_CHECK(mm_init());
  r = mm_bus(IVS_ACC_IAB, IVS_ACC_F_INTERRUPT, 0x0000u, 0u);
  IVS_CHECK(r.data == 0x1004u && r.wait_cpu_cycles == 0u);
  return IVS_TEST_PASS;
}

static int mm_bus_no_data_kinds(void)
{
  static const uint8_t kinds[] = {IVS_ACC_INTAK, IVS_ACC_STROBE_TCI, IVS_ACC_STROBE_PCIT};
  size_t i;
  IVS_CHECK(mm_init());
  IVS_CHECK(mm_bus_write(0x0300u, 0x7777u));
  for (i = 0; i < sizeof kinds / sizeof kinds[0]; i++) {
    ivs_bus_reply r = mm_bus(kinds[i], 0u, 0x0300u, 0x1111u);
    IVS_CHECK(r.data == 0u && r.wait_cpu_cycles == 0u);
  }
  IVS_CHECK(mm_bus_read(0x0300u) == 0x7777u);
  return IVS_TEST_PASS;
}

static int mm_bus_read_write_kinds(void)
{
  ivs_bus_reply r;
  IVS_CHECK(mm_init_loaded());
  r = mm_bus(IVS_ACC_WRITE, 0u, 0x0300u, 0x1357u);
  IVS_CHECK(r.data == 0u && r.wait_cpu_cycles == 0u);
  r = mm_bus(IVS_ACC_DW, 0u, 0x0301u, 0x2468u);
  IVS_CHECK(r.data == 0u && r.wait_cpu_cycles == 0u);
  r = mm_bus(IVS_ACC_FETCH, 0u, 0x0300u, 0u);
  IVS_CHECK(r.data == 0x1357u && r.wait_cpu_cycles == 0u);
  r = mm_bus(IVS_ACC_READ, 0u, 0x0301u, 0u);
  IVS_CHECK(r.data == 0x2468u && r.wait_cpu_cycles == 0u);
  r = mm_bus(IVS_ACC_ADAR, 0u, 0x1001u, 0u);
  IVS_CHECK(r.data == (0x0001u ^ 0x0155u) && r.wait_cpu_cycles == 0u);
  return IVS_TEST_PASS;
}

/* ---- introspection seam and registry (A-05b, D-03) ---- */

static int mm_describe_stub_vs_undriven(void)
{
  ivs_memsys_desc d;
  IVS_CHECK(ivs_memsys_describe(0u, IVS_MEMSYS_VIEW_R, 0x0000u, &d) == IVS_OK);
  IVS_CHECK(d.device == IVS_MEMSYS_DEVICE_STUB && d.impl == IVS_MEMSYS_IMPL_STUB);
  IVS_CHECK(strcmp(d.phys_name, "stic") == 0 && d.gate == IVS_MEMSYS_GATE_STIC_REGS);
  IVS_CHECK(ivs_memsys_describe(0u, IVS_MEMSYS_VIEW_R, 0x0080u, &d) == IVS_OK);
  IVS_CHECK(d.device == IVS_MEMSYS_DEVICE_UNDRIVEN && d.impl == IVS_MEMSYS_IMPL_NONE);
  IVS_CHECK(strcmp(d.phys_name, "undriven") == 0);
  IVS_CHECK(ivs_memsys_describe(0u, IVS_MEMSYS_VIEW_R, 0x7800u, &d) == IVS_OK);
  IVS_CHECK(strcmp(d.phys_name, "cart") == 0 && d.device == IVS_MEMSYS_DEVICE_ROM10);
  IVS_CHECK(ivs_memsys_describe(0u, IVS_MEMSYS_VIEW_W, 0x7800u, &d) == IVS_OK);
  IVS_CHECK(strcmp(d.phys_name, "gram") == 0 && d.mirror_words == 512u);
  IVS_CHECK(ivs_memsys_describe(0u, IVS_MEMSYS_VIEW_W, 0x1000u, &d) == IVS_OK);
  IVS_CHECK(strcmp(d.phys_name, "undriven") == 0);
  IVS_CHECK(ivs_memsys_describe(0u, IVS_MEMSYS_VIEW_R, 0x0200u, &d) == IVS_OK);
  IVS_CHECK(strcmp(d.phys_name, "sysram") == 0 && d.mirror_words == 0u);
  IVS_CHECK(ivs_memsys_describe(0u, 3u, 0x0000u, &d) == IVS_E_BAD_ARG);
  IVS_CHECK(ivs_memsys_describe(ivs_profile_count(), IVS_MEMSYS_VIEW_R, 0x0000u, &d) == IVS_E_BAD_ARG);
  IVS_CHECK(ivs_memsys_describe(0u, IVS_MEMSYS_VIEW_R, 0x0000u, NULL) == IVS_E_BAD_ARG);
  return IVS_TEST_PASS;
}

static int mm_registry_first_profile(void)
{
  const ivs_profile_desc *p;
  IVS_CHECK(ivs_profile_count() >= 1u);
  IVS_CHECK(ivs_profile_get(ivs_profile_count()) == NULL);
  p = ivs_profile_get(0u);
  IVS_CHECK(p != NULL);
  IVS_CHECK(strcmp(p->id, "intv-2609-ntsc-family-p1") == 0);
  IVS_CHECK(p->undriven_fill == 0xFFFFu && p->has_cart_span == 1u);
  IVS_CHECK(p->cart_lo == 0x4000u && p->cart_hi == 0xFFFFu && p->features_supported == 0u);
  IVS_CHECK(p->exec.units == 4096u && p->exec.unit_bits == 10u && p->exec.base == 0x1000u);
  IVS_CHECK(p->grom.units == 2048u && p->grom.unit_bits == 8u && p->grom.base == 0x3000u);
  IVS_CHECK(p->exec.required == 1u && p->grom.required == 1u);
  IVS_CHECK(ivs_memsys_init(NULL, 0u) == IVS_E_BAD_ARG);
  IVS_CHECK(ivs_memsys_init(&mm_m, ivs_profile_count()) == IVS_E_BAD_ARG);
  return IVS_TEST_PASS;
}

/* ---- case table: fixed cases, one lo and one hi anchor per region row, then the literal-map
 * family of memmap_rows.c ---- */

static const ivs_test_case mm_fixed[] = {
    {"memmap.width.rom10-wide-reads-03ff", mm_width_rom10, NULL},
    {"memmap.width.grom8-wide-reads-00ff", mm_width_grom8, NULL},
    {"memmap.width.ram16-keeps-16", mm_width_ram16, NULL},
    {"memmap.width.ram8-truncates", mm_width_ram8, NULL},
    {"memmap.width.stub-reads-ffff", mm_width_stub, NULL},
    {"memmap.width.undriven-reads-ffff", mm_width_undriven, NULL},
    {"memmap.mirror.gram-3a00", mm_mirror_gram_3a00, NULL},
    {"memmap.gate.gmem-closed-blocks", mm_gate_gmem_closed, NULL},
    {"memmap.gate.stic-regs-scope", mm_gate_stic_regs_scope, NULL},
    {"memmap.gate.default-open", mm_gate_default_open, NULL},
    {"memmap.unloaded.exec-reads-ffff", mm_unloaded_exec_reads_ffff, NULL},
    {"memmap.unloaded.grom-reads-ffff", mm_unloaded_grom_reads_ffff, NULL},
    {"memmap.unloaded.cart-reads-ffff", mm_unloaded_cart_reads_ffff, NULL},
    {"memmap.unloaded.after-clear", mm_unloaded_after_clear, NULL},
    {"memmap.cart.one-word-5000", mm_cart_one_word, NULL},
    {"memmap.cart.below-4fff", mm_cart_below, NULL},
    {"memmap.cart.above-5001", mm_cart_above, NULL},
    {"memmap.cart.wide-word-masked", mm_cart_wide_masked, NULL},
    {"memmap.cart.writes-ignored", mm_cart_writes_ignored, NULL},
    {"memmap.cart.sixteen-segments", mm_cart_sixteen_segments, NULL},
    {"memmap.cart.under-gram-alias", mm_cart_under_alias, NULL},
    {"memmap.bus.iab-reset", mm_bus_iab_reset, NULL},
    {"memmap.bus.iab-interrupt", mm_bus_iab_interrupt, NULL},
    {"memmap.bus.no-data-kinds", mm_bus_no_data_kinds, NULL},
    {"memmap.bus.read-write-kinds", mm_bus_read_write_kinds, NULL},
    {"memmap.describe.stub-vs-undriven", mm_describe_stub_vs_undriven, NULL},
    {"memmap.registry.first-profile", mm_registry_first_profile, NULL},
};
#define MM_FIXED (sizeof mm_fixed / sizeof mm_fixed[0])
#define MM_ANCHORS (MM_FIXED + 2u * MM_ROWS)

static char mm_ids[2u * MM_ROWS][48];

static const char *mm_case_id(const void *data, size_t i)
{
  (void)data;
  if (i >= MM_ANCHORS) {
    return mm_rows_id(i - MM_ANCHORS);
  }
  return i < MM_FIXED ? mm_fixed[i].id : mm_ids[i - MM_FIXED];
}

static int mm_case_run(const void *data, size_t i)
{
  const mm_row *row;
  (void)data;
  if (i < MM_FIXED) {
    return mm_fixed[i].fn();
  }
  if (i >= MM_ANCHORS) {
    return mm_rows_run(i - MM_ANCHORS);
  }
  row = &mm_rows[(i - MM_FIXED) / 2u];
  return mm_anchor(row, ((i - MM_FIXED) % 2u) == 0u ? row->lo : row->hi);
}

int main(int argc, char **argv)
{
  ivs_test_suite suite;
  size_t i;
  for (i = 0; i < MM_ROWS; i++) {
    (void)snprintf(mm_ids[2u * i], sizeof mm_ids[0], "memmap.anchor.%s-lo", mm_rows[i].slug);
    (void)snprintf(mm_ids[2u * i + 1u], sizeof mm_ids[0], "memmap.anchor.%s-hi", mm_rows[i].slug);
  }
  if (!mm_rows_init()) {
    fprintf(stderr, "# memmap: a literal-row case id does not fit\n");
    return 1;
  }
  memset(&suite, 0, sizeof suite);
  suite.name = "memmap";
  suite.count = MM_ANCHORS + mm_rows_count();
  suite.id = mm_case_id;
  suite.run = mm_case_run;
  suite.sources = mm_rows_sources(&suite.n_sources);
  return ivs_test_main(argc, argv, &suite);
}
