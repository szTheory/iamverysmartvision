/* SPDX-License-Identifier: MIT */
/* Suite stic, family reg (plan 06-06; D-02, D-04): the register file judged against the independent register-address literal
 * data/stic/<profile>.regmap.literal.txt (sixty-four rows: class, write mask, read mask), turned into a C include by
 * cmake/IvsSticProbes.cmake, which reads that literal, the memory-map literal and the geometry literal and nothing of the core
 * or the profile. A store keeps value & the write mask; a read gives (store & read mask) | (FFFF & ~read mask), so a write-only
 * register reads FFFF and a collision register reads FC00 OR its stored bits; strobe and reserved registers keep nothing.
 *   stic.table.matches-literal   the generated parameter table (profile stic.regmap) equals the literal, all 64 rows
 *   stic.reg.width               a store of FFFF keeps exactly the literal write mask; bits 15 and 14 are never stored
 *   stic.reg.masks               value & wmask stored, the width formula read, for every row
 *   stic.reg.write-only-reads-ffff, .collision-readback, .reserved-drops, .strobe-stores-nothing */
#include <stdint.h>
#include <string.h>

#include "ivs/ivs.h"
#include "machine/machine.h"
#include "stic_cases.h"

typedef sl_reg_row stic_lit_reg_row;
#include "ivs_stic_probe.inc"

const sl_reg_row *sl_regs(void)
{
  return stic_lit_reg;
}

uint32_t sl_hook(int write_view, uint32_t granule)
{
  return (write_view != 0 ? stic_lit_hook_wr : stic_lit_hook_rd)[granule & 0xFFFu];
}

size_t sl_mode_count(void)
{
  return STIC_LIT_MODE_ADDRS;
}

uint16_t sl_mode_addr(size_t i)
{
  return stic_lit_mode_addr[i];
}

const ivs_test_source *sl_sources(size_t *n)
{
  *n = STIC_LIT_SOURCES;
  return stic_lit_sources;
}

#define SL_STORED 0u
#define SL_STROBE 1u
#define SL_COLLISION 2u
#define SL_RESERVED 3u

static ivs_memsys sr_mem;

static int sr_init(void)
{
  return ivs_memsys_init(&sr_mem, SC_PROFILE) == IVS_OK;
}

static uint16_t sr_expect_read(const sl_reg_row *r, uint16_t stored)
{
  return (uint16_t)((stored & r->rmask) | (0xFFFFu & (uint16_t)~r->rmask));
}

/* 0 when the generated table equals the literal rows, else the 1-based index of the first differing row. */
static uint32_t sr_table_diff(const ivs_stic_params *p, const sl_reg_row *lit)
{
  uint32_t i;
  for (i = 0u; i < 64u; i++) {
    uint32_t want_class = IVS_STIC_CLASS_STORED;
    if (lit[i].cls == SL_STROBE) {
      want_class = IVS_STIC_CLASS_STROBE;
    } else if (lit[i].cls == SL_COLLISION) {
      want_class = IVS_STIC_CLASS_COLLISION;
    } else if (lit[i].cls == SL_RESERVED) {
      want_class = IVS_STIC_CLASS_RESERVED;
    }
    if (lit[i].addr != i || p->reg_wmask[i] != lit[i].wmask || p->reg_rmask[i] != lit[i].rmask ||
        (uint32_t)p->reg_class[i] != want_class) {
      return i + 1u;
    }
  }
  return 0u;
}

/* The judge's input: the checking function is run on a copy with one row changed (stic_judge.c). */
uint32_t sr_table_diff_copy(uint32_t row, uint16_t wmask_delta);
uint32_t sr_table_diff_copy(uint32_t row, uint16_t wmask_delta)
{
  sl_reg_row copy[64];
  memcpy(copy, stic_lit_reg, sizeof copy);
  copy[row & 63u].wmask = (uint16_t)(copy[row & 63u].wmask ^ wmask_delta);
  return sr_table_diff(ivs_stic_params_get(SC_PROFILE), copy);
}

static int sr_table_matches(void)
{
  SC_CHECK(sr_table_diff(ivs_stic_params_get(SC_PROFILE), stic_lit_reg) == 0u);
  return IVS_TEST_PASS;
}

static int sr_width(void)
{
  uint32_t i;
  SC_CHECK(sr_init());
  for (i = 0u; i < 64u; i++) {
    const sl_reg_row *r = &stic_lit_reg[i];
    uint16_t want = (r->cls == SL_STORED || r->cls == SL_COLLISION) ? r->wmask : 0u;
    ivs_memsys_write(&sr_mem, (uint16_t)i, 0xFFFFu);
    if (sr_mem.stic_reg[i] != want || (sr_mem.stic_reg[i] & 0xC000u) != 0u) {
      ivs_test_diag("register %02X: a store of FFFF kept %04X, the literal keeps %04X", (unsigned)i,
                    (unsigned)sr_mem.stic_reg[i], (unsigned)want);
      return IVS_TEST_FAIL;
    }
  }
  return IVS_TEST_PASS;
}

static int sr_masks(void)
{
  static const uint16_t vals[3] = {0x3A95u, 0xC5A5u, 0x0001u};
  uint32_t i;
  uint32_t k;
  for (k = 0u; k < 3u; k++) {
    SC_CHECK(sr_init());
    for (i = 0u; i < 64u; i++) {
      const sl_reg_row *r = &stic_lit_reg[i];
      uint16_t stored = (uint16_t)(vals[k] & r->wmask);
      ivs_memsys_write(&sr_mem, (uint16_t)i, vals[k]);
      SC_CHECK(sr_mem.stic_reg[i] == stored);
      if (ivs_memsys_read(&sr_mem, (uint16_t)i) != sr_expect_read(r, stored)) {
        ivs_test_diag("register %02X: stored %04X reads %04X, the literal gives %04X", (unsigned)i, (unsigned)stored,
                      (unsigned)ivs_memsys_read(&sr_mem, (uint16_t)i), (unsigned)sr_expect_read(r, stored));
        return IVS_TEST_FAIL;
      }
    }
  }
  return IVS_TEST_PASS;
}

static int sr_write_only(void)
{
  uint32_t i;
  SC_CHECK(sr_init());
  for (i = 0u; i < 64u; i++) {
    if (stic_lit_reg[i].rmask != 0u) {
      continue;
    }
    ivs_memsys_write(&sr_mem, (uint16_t)i, 0xFFFFu);
    SC_CHECK(ivs_memsys_read(&sr_mem, (uint16_t)i) == 0xFFFFu);
    ivs_memsys_write(&sr_mem, (uint16_t)i, 0x0000u);
    SC_CHECK(ivs_memsys_read(&sr_mem, (uint16_t)i) == 0xFFFFu);
    SC_CHECK(ivs_memsys_read(&sr_mem, (uint16_t)(0x0040u + i)) == 0xFFFFu); /* the read alias mirrors the file */
  }
  return IVS_TEST_PASS;
}

static int sr_collision(void)
{
  uint32_t i;
  uint32_t n = 0u;
  SC_CHECK(sr_init());
  for (i = 0u; i < 64u; i++) {
    const sl_reg_row *r = &stic_lit_reg[i];
    if (r->cls != SL_COLLISION) {
      continue;
    }
    n++;
    ivs_memsys_write(&sr_mem, (uint16_t)i, 0x00A5u);
    SC_CHECK(ivs_memsys_read(&sr_mem, (uint16_t)i) == (uint16_t)(0xFC00u | 0x00A5u));
    SC_CHECK(ivs_memsys_read(&sr_mem, (uint16_t)(0x0040u + i)) == (uint16_t)(0xFC00u | 0x00A5u));
    ivs_memsys_write(&sr_mem, (uint16_t)i, 0x0000u);
    SC_CHECK(ivs_memsys_read(&sr_mem, (uint16_t)i) == 0xFC00u);
  }
  SC_CHECK(n == 8u); /* one per MOB in the literal */
  return IVS_TEST_PASS;
}

static int sr_reserved_drops(void)
{
  uint32_t i;
  SC_CHECK(sr_init());
  for (i = 0u; i < 64u; i++) {
    if (stic_lit_reg[i].cls != SL_RESERVED) {
      continue;
    }
    ivs_memsys_write(&sr_mem, (uint16_t)i, 0xFFFFu);
    SC_CHECK(sr_mem.stic_reg[i] == 0u && ivs_memsys_read(&sr_mem, (uint16_t)i) == 0xFFFFu);
  }
  return IVS_TEST_PASS;
}

static int sr_strobe_stores_nothing(void)
{
  uint32_t i;
  uint32_t n = 0u;
  SC_CHECK(sr_init());
  for (i = 0u; i < 64u; i++) {
    if (stic_lit_reg[i].cls != SL_STROBE) {
      continue;
    }
    n++;
    ivs_memsys_write(&sr_mem, (uint16_t)i, 0xFFFFu);
    SC_CHECK(sr_mem.stic_reg[i] == 0u);
  }
  SC_CHECK(n == 2u); /* the display-enable and the mode-select registers */
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_stic_reg_cases[] = {
    {"stic.table.matches-literal", sr_table_matches, NULL},
    {"stic.reg.width", sr_width, NULL},
    {"stic.reg.masks", sr_masks, NULL},
    {"stic.reg.write-only-reads-ffff", sr_write_only, NULL},
    {"stic.reg.collision-readback", sr_collision, NULL},
    {"stic.reg.reserved-drops", sr_reserved_drops, NULL},
    {"stic.reg.strobe-stores-nothing", sr_strobe_stores_nothing, NULL},
};
const size_t ivs_stic_reg_count = sizeof ivs_stic_reg_cases / sizeof ivs_stic_reg_cases[0];
