/* SPDX-License-Identifier: MIT */
/* Suite memmap, literal-map family (D-21, D-22, A-05, plan 03-06). Every case here derives its
 * addresses and expected projections from ivs_memmap_probes.inc, which cmake/IvsMemmapProbes.cmake
 * generates from the sourced literal maps data/memmap/<profile>.literal.txt only; never from the
 * profile's region descriptors under test.
 *
 * memmap.dump.matches-literal: for every profile with a literal map, a memory system holding
 * stand-in EXEC, GROM and cartridge words (one segment per literal cart row) is probed at all
 * 65,536 addresses of both views through the real bus function (ivs_memsys_bus):
 *   width (behavioural): read view, popcount of (read after raw-poke FFFF) XOR (read after
 *     raw-poke 0000); the raw-poke seam (A-05a) only sets the backing word, the reads go through
 *     the bus. Write view, rows the literal gives a width: bus-write FFFF then 0000 at the
 *     address and read each back through the bus at the cell's read-view address, taken from the
 *     literal (the same address when the literal's read row there has the same device, else the
 *     literal's read row of that device at lo_R + ((a - lo_W) mod mirror_W)). Write view, rows
 *     the literal gives width 0: one state snapshot per row, a bus write of FFFF and of 0000 to
 *     every address of the row, then a memcmp with the snapshot (equal: width 0; else the case
 *     fails naming the first address that changes the state).
 *   mirror (behavioural for rows with a width): the smallest N inside the merged row where a
 *     change at lo is seen at lo+N (read view: raw pokes at lo, bus reads at lo+N; write view:
 *     bus writes at lo+N, bus reads at the read-side address of lo); - when there is none.
 *   device, impl and gate of every address, and the mirror of width-0 rows (declarative): from
 *     the introspection function ivs_memsys_describe (A-05b).
 * Adjacent equal tuples are merged into `<view> <LO>-<HI> <device> w=<n> impl=<x> mirror=<m|->
 * gate=<g|->` lines, compared with the literal's projection, and written (when IVS_TEST_OUT_DIR
 * is set) to $IVS_TEST_OUT_DIR/memmap-dump.txt, one `# memmap-dump nonce=<IVS_TEST_NONCE or
 * none> profile=<id>` header per profile, for the memmap gate step (H6).
 *
 * memmap.row.<view>-<LO>-<HI>, one case per literal row (D-22, MEM-02): probes at lo-1 and hi+1
 * (judged by the neighbouring literal row), lo and hi, and for a row with a mirror inside it
 * lo+mirror-1 and lo+mirror. Expected values come only from mr_rules below, keyed on the
 * literal's device and width, never from the profile descriptors:
 *   read view: a raw-poked FFFF reads (FFFF & rmask) | (fill & ~rmask), a raw-poked 0000 reads
 *     fill & ~rmask (03FF, 00FF or FFFF and 0000 for real rows; FFFF for stub and undriven rows);
 *     at the mirror wrap, a word poked at lo reads back at lo+mirror and lo+mirror-1 keeps its own;
 *   write view, rows that keep a store: a bus write of 1234 and then of FFFF reads back at the
 *     read-side address as value & rmask, and changes exactly one 16-bit word of the machine state,
 *     to value & rmask (so a store kept wider than the read mask is seen); a store at lo+mirror
 *     reads back at the read-side address of lo; rows that drop a store: writes at the probe
 *     addresses, and FFFF and 0000 at every address of the row itself, leave the whole state
 *     unchanged.
 * memmap.row-gate.<gate>-closed, one case per gate of the literal (D-19): with the gate closed
 * every row of that gate reads FFFF (the undriven value) and drops writes; reopened, a real row
 * reads its stored word again.
 * Test code may use stdio and getenv; the core may not. All image words are generated stand-ins
 * built here. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu/cp1610.h"
#include "ivs_test.h"
#include "memmap/memsys.h"
#include "memmap_rows.h"

/* One literal row (field order of the generator's initializers). */
typedef struct ivs_mp_row {
  const char *profile;
  char view;       /* 'R' or 'W' */
  uint16_t lo;
  uint16_t hi;
  uint8_t w;       /* driven bits, 0..16 */
  uint32_t mirror; /* words, 0 for - */
  const char *device;
  const char *impl;
  const char *gate; /* "-" for none */
  const char *line; /* structural projection */
} ivs_mp_row;

#include "ivs_memmap_probes.inc"

_Static_assert(IVS_MP_ROWS > 0u && IVS_MP_ROWS < 255u, "literal row count fits a u8 row map");

#define MR_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

#define MR_NONE 0xFFu
#define MR_SPACE 65536u
#define MR_LINES 256u
#define MR_LINE 96u

static ivs_memsys mr_m;
static ivs_memsys mr_snap;
static uint8_t mr_exec_img[IVS_MEMSYS_EXEC_BYTES];
static uint8_t mr_grom_img[IVS_MEMSYS_GROM_BYTES];
static uint8_t mr_cart_img[2u * IVS_MEMSYS_CART_WORDS];
static uint8_t mr_rowmap[2][MR_SPACE]; /* literal row index per view and address */
static char mr_lines[MR_LINES][MR_LINE];
static size_t mr_nlines;

/* ---- literal lookups (current profile) ---- */

static int mr_view_index(char view)
{
  return view == 'R' ? 0 : 1;
}

/* Fills mr_rowmap for profile id pid (kept until another profile is mapped); returns 0 when a
 * view is not tiled by its rows. */
static int mr_map_profile(const char *pid)
{
  static const char *mapped = NULL;
  uint32_t i;
  uint32_t a;
  if (mapped != NULL && strcmp(mapped, pid) == 0) {
    return 1;
  }
  mapped = NULL;
  memset(mr_rowmap, MR_NONE, sizeof mr_rowmap);
  for (i = 0u; i < IVS_MP_ROWS; i++) {
    const ivs_mp_row *r = &ivs_mp_rows[i];
    if (strcmp(r->profile, pid) != 0) {
      continue;
    }
    for (a = r->lo; a <= r->hi; a++) {
      mr_rowmap[mr_view_index(r->view)][a] = (uint8_t)i;
    }
  }
  for (a = 0u; a < MR_SPACE; a++) {
    if (mr_rowmap[0][a] == MR_NONE || mr_rowmap[1][a] == MR_NONE) {
      return 0;
    }
  }
  mapped = pid;
  return 1;
}

static const ivs_mp_row *mr_row_at(char view, uint32_t addr)
{
  return &ivs_mp_rows[mr_rowmap[mr_view_index(view)][addr & 0xFFFFu]];
}

/* Read-side address of a write-view address, from the literal only: the address itself when the
 * literal's read row there names the same device, else the literal's read row of that device at
 * lo_R + ((a - lo_W) mod mirror_W). 0 when there is none (*ok cleared). */
static uint16_t mr_obs(uint16_t addr, int *ok)
{
  const ivs_mp_row *wr = mr_row_at('W', addr);
  const ivs_mp_row *rd = mr_row_at('R', addr);
  uint32_t off;
  uint32_t i;
  *ok = 1;
  if (strcmp(rd->device, wr->device) == 0) {
    return addr;
  }
  off = (uint32_t)addr - wr->lo;
  if (wr->mirror != 0u) {
    off %= wr->mirror;
  }
  for (i = 0u; i < IVS_MP_ROWS; i++) {
    const ivs_mp_row *r = &ivs_mp_rows[i];
    if (r->view == 'R' && strcmp(r->profile, wr->profile) == 0 && strcmp(r->device, wr->device) == 0
        && (uint32_t)r->lo + off <= r->hi) {
      return (uint16_t)(r->lo + off);
    }
  }
  *ok = 0;
  return 0u;
}

/* ---- bus access through the real port (D-20) ---- */

static uint16_t mr_read(uint16_t addr)
{
  ivs_cp1610_access a;
  ivs_bus_reply r;
  memset(&a, 0, sizeof a);
  a.kind = IVS_ACC_READ;
  a.addr = addr;
  r = ivs_memsys_bus(&mr_m, &a);
  return r.wait_cpu_cycles == 0u ? r.data : 0x5A5Au;
}

static void mr_write(uint16_t addr, uint16_t value)
{
  ivs_cp1610_access a;
  memset(&a, 0, sizeof a);
  a.kind = IVS_ACC_WRITE;
  a.addr = addr;
  a.data = value;
  (void)ivs_memsys_bus(&mr_m, &a);
}

static unsigned mr_popcount(uint16_t v)
{
  unsigned n = 0u;
  while (v != 0u) {
    n += v & 1u;
    v = (uint16_t)(v >> 1);
  }
  return n;
}

/* ---- stand-in content (no rights-bearing bytes) ---- */

static void mr_put_be(uint8_t *p, uint32_t i, uint16_t w)
{
  p[2u * i] = (uint8_t)(w >> 8);
  p[2u * i + 1u] = (uint8_t)(w & 0xFFu);
}

static void mr_build_images(void)
{
  static int built = 0;
  uint32_t i;
  if (built) {
    return;
  }
  built = 1;
  for (i = 0u; i < IVS_MEMSYS_EXEC_WORDS; i++) {
    mr_put_be(mr_exec_img, i, (uint16_t)((37u * i + 11u) & 0x03FFu));
  }
  for (i = 0u; i < IVS_MEMSYS_GROM_BYTES; i++) {
    mr_grom_img[i] = (uint8_t)((53u * i + 7u) & 0xFFu);
  }
  for (i = 0u; i < IVS_MEMSYS_CART_WORDS; i++) {
    mr_put_be(mr_cart_img, i, (uint16_t)((29u * i + 5u) & 0x03FFu));
  }
}

/* Fresh memory system of registry profile p with stand-in EXEC and GROM images and one cartridge
 * segment per literal read row of device cart (the literal's span, not the profile's). */
static int mr_setup(uint32_t p, const char *pid)
{
  uint32_t i;
  if (ivs_memsys_init(&mr_m, p) != IVS_OK) {
    return 0;
  }
  mr_build_images();
  ivs_memsys_install_exec(&mr_m, mr_exec_img);
  ivs_memsys_install_grom(&mr_m, mr_grom_img);
  for (i = 0u; i < IVS_MP_ROWS; i++) {
    const ivs_mp_row *r = &ivs_mp_rows[i];
    uint32_t len = (uint32_t)r->hi - r->lo + 1u;
    if (r->view == 'R' && strcmp(r->profile, pid) == 0 && strcmp(r->device, "cart") == 0) {
      if (len > IVS_MEMSYS_CART_WORDS) {
        return 0;
      }
      ivs_memsys_install_cart_segment(&mr_m, r->lo, mr_cart_img, (uint16_t)len);
    }
  }
  return 1;
}

/* Registry index of profile id pid, or ivs_profile_count() when absent. */
static uint32_t mr_profile_index(const char *pid)
{
  uint32_t p;
  for (p = 0u; p < ivs_profile_count(); p++) {
    if (strcmp(ivs_profile_get(p)->id, pid) == 0) {
      return p;
    }
  }
  return ivs_profile_count();
}

/* ---- rule table (D-17, D-19, D-B9), keyed on the literal's device and width ---- */

typedef struct mr_rule {
  const char *device;
  uint8_t w;
  uint16_t rmask;
  uint16_t fill;
  uint8_t keep; /* 1: a store keeps value & rmask */
} mr_rule;

/* Written here from the decisions, not read from the generated descriptors: decle ROM 10 bits and
 * 8-bit GROM with the upper bits driven 0, 8-bit RAM reads its byte with the upper byte 0 (C60),
 * system RAM 16 bits, the PSG registers keep and read 4 to 8 low bits with the upper byte 0 (psg8,
 * C64, masks of the register-address literal C70; the shape register reads 0, C75), stub and
 * undriven reads give the undriven value FFFF (C5, C60). */
static const mr_rule mr_rules[] = {
    {"exec", 10u, 0x03FFu, 0x0000u, 0u},      {"cart", 10u, 0x03FFu, 0x0000u, 0u},
    {"grom", 8u, 0x00FFu, 0x0000u, 0u},       {"scratchpad", 8u, 0x00FFu, 0x0000u, 1u},
    {"gram", 8u, 0x00FFu, 0x0000u, 1u},       {"sysram", 16u, 0xFFFFu, 0x0000u, 1u},
    {"stic", 0u, 0x0000u, 0xFFFFu, 0u},       {"psg", 8u, 0x00FFu, 0x0000u, 1u},
    /* plan 06-06, the STIC register file (stic14, C81, C86; the widths of the register-address literal
     * data/stic): a read gives (stored & rmask) | (FFFF & ~rmask), so a collision register (10 bits) reads FC00
     * OR its stored bits and a write-only register (w 0 in the read view) reads FFFF; a store keeps value & the
     * write mask of the register, 11, 12, 14, 4, 3 or 2 bits, and a strobe or reserved register keeps nothing */
    {"stic", 10u, 0x03FFu, 0xFFFFu, 1u},      {"stic", 11u, 0x07FFu, 0xFFFFu, 1u},
    {"stic", 12u, 0x0FFFu, 0xFFFFu, 1u},      {"stic", 14u, 0x3FFFu, 0xFFFFu, 1u},
    {"stic", 4u, 0x000Fu, 0xFFFFu, 1u},       {"stic", 3u, 0x0007u, 0xFFFFu, 1u},
    {"stic", 2u, 0x0003u, 0xFFFFu, 1u},
    {"psg", 4u, 0x000Fu, 0x0000u, 1u},        {"psg", 5u, 0x001Fu, 0x0000u, 1u},
    {"psg", 6u, 0x003Fu, 0x0000u, 1u},        {"psg", 0u, 0x0000u, 0x0000u, 0u},
    {"undriven", 0u, 0x0000u, 0xFFFFu, 0u},
};

static const mr_rule *mr_rule_of(const ivs_mp_row *r)
{
  size_t i;
  for (i = 0u; i < sizeof mr_rules / sizeof mr_rules[0]; i++) {
    if (strcmp(mr_rules[i].device, r->device) == 0 && mr_rules[i].w == r->w) {
      return &mr_rules[i];
    }
  }
  ivs_test_diag("no rule for device %s width %u (literal row %s)", r->device, (unsigned)r->w, r->line);
  return NULL;
}

static uint16_t mr_expect(const mr_rule *u, uint16_t stored)
{
  return (uint16_t)((stored & u->rmask) | (u->fill & (uint16_t)~u->rmask));
}

/* Declared exception registers (the literal's x= rows, plans 05-04 and 06-06): the shape register $01FA, whose
 * read mask is 0 (C75), and the two port registers $01FE and $01FF, whose read follows the direction
 * bit and the pins (plan 05-05). A store to one of them is observed in the register storage of the
 * memory system, not through a bus read, so the width a store keeps is still measured. */
/* Plan 06-06: every write-view address of the STIC register file ($0000-$003F and the three write aliases at
 * $4000, $8000 and $C000, named by the literal's x=reads-zero and x=strobe rows) is observed in the register
 * storage too: a write-only register keeps bits that no read returns, and the strobe and reserved registers
 * (x=strobe, nothing is stored) must leave the storage as it was. The collision registers read back through the
 * bus as well; the storage gives the same count of kept bits. */
static int mr_is_stic_write(uint16_t addr)
{
  return (addr & 0x3FC0u) == 0u; /* includes the strobe registers $0020 and $0021 and their aliases */
}

static int mr_is_exception(uint16_t addr)
{
  return addr == 0x01FAu || addr == 0x01FEu || addr == 0x01FFu || mr_is_stic_write(addr);
}

/* The port registers, whose bus read follows the pins (plan 05-05): after power-on both ports are in
 * input mode with no pin held low, so a read gives the idle byte 00FF whatever the store holds. */
static int mr_is_pins(uint16_t addr)
{
  return addr == 0x01FEu || addr == 0x01FFu;
}

/* What a store at addr left behind, read back at obs (the read-side address of addr). */
static uint16_t mr_observe(uint16_t addr, uint16_t obs)
{
  if (mr_is_stic_write(addr)) {
    return mr_m.stic_reg[addr & 0x3Fu];
  }
  if (mr_is_exception(addr)) {
    return mr_m.psg_reg[(uint32_t)addr - 0x01F0u];
  }
  return mr_read(obs);
}

/* Number of 16-bit state words that differ from mr_snap; *last is the new value of the last one. */
static uint32_t mr_changed_words(uint16_t *last)
{
  const unsigned char *a = (const unsigned char *)&mr_snap;
  const unsigned char *b = (const unsigned char *)&mr_m;
  uint32_t n = 0u;
  size_t i;
  *last = 0u;
  for (i = 0u; i + 1u < sizeof mr_m; i += 2u) {
    if (a[i] != b[i] || a[i + 1u] != b[i + 1u]) {
      uint16_t v;
      memcpy(&v, b + i, sizeof v);
      *last = v;
      n++;
    }
  }
  return n;
}

/* Profile of literal row r mapped and set up fresh; 0 on failure. */
static int mr_fresh(const ivs_mp_row *r)
{
  uint32_t p = mr_profile_index(r->profile);
  if (p >= ivs_profile_count()) {
    ivs_test_diag("literal map of profile %s has no registry profile", r->profile);
    return 0;
  }
  if (!mr_map_profile(r->profile)) {
    ivs_test_diag("profile %s: literal rows do not tile both views", r->profile);
    return 0;
  }
  return mr_setup(p, r->profile);
}

/* Read-view probe at addr, judged by the literal row there. */
static int mr_probe_read(uint16_t addr)
{
  const mr_rule *u = mr_rule_of(mr_row_at('R', addr));
  uint16_t want_ff;
  uint16_t want_00;
  MR_CHECK(u != NULL);
  want_ff = mr_expect(u, 0xFFFFu);
  want_00 = mr_expect(u, 0x0000u);
  if (mr_is_pins(addr)) {
    want_ff = 0x00FFu; /* idle pins, input mode: the byte does not depend on the stored word */
    want_00 = 0x00FFu;
  }
  ivs_memsys_poke_raw_internal(&mr_m, addr, 0xFFFFu);
  if (mr_read(addr) != want_ff) {
    ivs_test_diag("R %04X: raw FFFF reads %04X, rule gives %04X", (unsigned)addr, (unsigned)mr_read(addr),
                  (unsigned)want_ff);
    return IVS_TEST_FAIL;
  }
  ivs_memsys_poke_raw_internal(&mr_m, addr, 0x0000u);
  if (mr_read(addr) != want_00) {
    ivs_test_diag("R %04X: raw 0000 reads %04X, rule gives %04X", (unsigned)addr, (unsigned)mr_read(addr),
                  (unsigned)want_00);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

/* One kept store: reads back at obs as value & rmask and changes exactly one state word to it. */
static int mr_store_kept(const mr_rule *u, uint16_t addr, uint16_t obs, uint16_t value)
{
  uint16_t want = (uint16_t)(value & u->rmask);
  uint16_t got;
  uint32_t n;
  uint16_t seen;
  uint16_t prev = mr_observe(addr, obs);
  memcpy(&mr_snap, &mr_m, sizeof mr_snap);
  mr_write(addr, value);
  seen = mr_observe(addr, obs);
  if (seen != want) {
    ivs_test_diag("W %04X: store %04X reads %04X at %04X, rule gives %04X", (unsigned)addr, (unsigned)value,
                  (unsigned)seen, (unsigned)obs, (unsigned)want);
    return IVS_TEST_FAIL;
  }
  n = mr_changed_words(&got);
  if (prev == want) {
    /* a store of a value that the register already holds (a 2-bit register stores 1234 as 0, plan 06-06) changes no word */
    if (n != 0u) {
      ivs_test_diag("W %04X: store %04X changed %lu state words, rule gives none", (unsigned)addr,
                    (unsigned)value, (unsigned long)n);
      return IVS_TEST_FAIL;
    }
    return IVS_TEST_PASS;
  }
  if (n != 1u || got != want) {
    ivs_test_diag("W %04X: store %04X changed %lu state words (last %04X), rule gives one word %04X",
                  (unsigned)addr, (unsigned)value, (unsigned long)n, (unsigned)got, (unsigned)want);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

/* Write-view probe at addr, judged by the literal row there. */
static int mr_probe_write(uint16_t addr)
{
  const mr_rule *u = mr_rule_of(mr_row_at('W', addr));
  MR_CHECK(u != NULL);
  if (u->keep != 0u) {
    int ok;
    uint16_t obs = mr_obs(addr, &ok);
    MR_CHECK(ok);
    MR_CHECK(mr_store_kept(u, addr, obs, 0x1234u) == IVS_TEST_PASS);
    MR_CHECK(mr_store_kept(u, addr, obs, 0xFFFFu) == IVS_TEST_PASS);
    return IVS_TEST_PASS;
  }
  memcpy(&mr_snap, &mr_m, sizeof mr_snap);
  mr_write(addr, 0x1234u);
  mr_write(addr, 0xFFFFu);
  if (memcmp(&mr_snap, &mr_m, sizeof mr_snap) != 0) {
    ivs_test_diag("W %04X: a store the rule drops changed the machine state", (unsigned)addr);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

static int mr_dropped_row(const ivs_mp_row *r);

/* Case memmap.row.<view>-<LO>-<HI>. */
static int mr_row_case(const ivs_mp_row *r)
{
  uint32_t probes[6];
  size_t np = 0u;
  size_t k;
  const mr_rule *u = mr_rule_of(r);
  MR_CHECK(u != NULL);
  if (r->lo > 0u) {
    probes[np++] = r->lo - 1u;
  }
  probes[np++] = r->lo;
  probes[np++] = r->hi;
  if (r->hi < 0xFFFFu) {
    probes[np++] = r->hi + 1u;
  }
  for (k = 0u; k < np; k++) {
    uint16_t a = (uint16_t)probes[k];
    MR_CHECK(mr_fresh(r));
    if (r->view == 'R') {
      MR_CHECK(mr_probe_read(a) == IVS_TEST_PASS);
    } else {
      MR_CHECK(mr_probe_write(a) == IVS_TEST_PASS);
    }
  }
  if (r->mirror != 0u && (uint32_t)r->lo + r->mirror <= r->hi) {
    uint16_t at = (uint16_t)(r->lo + r->mirror);
    uint16_t before = (uint16_t)(at - 1u);
    MR_CHECK(mr_fresh(r));
    if (r->view == 'R') {
      ivs_memsys_poke_raw_internal(&mr_m, before, 0x02AAu);
      ivs_memsys_poke_raw_internal(&mr_m, r->lo, 0x0155u);
      MR_CHECK(mr_read(at) == mr_expect(u, 0x0155u));
      MR_CHECK(before == r->lo || mr_read(before) == mr_expect(u, 0x02AAu));
    } else if (u->keep != 0u) {
      int ok;
      uint16_t obs = mr_obs(r->lo, &ok);
      MR_CHECK(ok);
      mr_write(r->lo, 0x0011u);
      mr_write(at, 0x0022u);
      MR_CHECK(mr_read(obs) == (uint16_t)(0x0022u & u->rmask));
      mr_write(before, 0x0033u);
      MR_CHECK(before == r->lo || mr_read(obs) == (uint16_t)(0x0022u & u->rmask));
    }
  }
  if (r->view == 'W' && u->keep == 0u) {
    MR_CHECK(mr_fresh(r));
    MR_CHECK(mr_dropped_row(r));
  }
  return IVS_TEST_PASS;
}

/* Gate names of the literal and their setter ids (D-19). */
static uint32_t mr_gate_id(const char *name)
{
  if (strcmp(name, "stic-regs") == 0) {
    return IVS_MEMSYS_GATE_STIC_REGS;
  }
  if (strcmp(name, "gmem") == 0) {
    return IVS_MEMSYS_GATE_GMEM;
  }
  return IVS_MEMSYS_GATE_NONE;
}

/* Case memmap.row-gate.<gate>-closed over every literal row of that gate. */
static int mr_gate_case(const char *gate)
{
  uint32_t g = mr_gate_id(gate);
  uint32_t i;
  MR_CHECK(g != IVS_MEMSYS_GATE_NONE);
  for (i = 0u; i < IVS_MP_ROWS; i++) {
    const ivs_mp_row *r = &ivs_mp_rows[i];
    const mr_rule *u;
    uint32_t k;
    if (strcmp(r->gate, gate) != 0) {
      continue;
    }
    u = mr_rule_of(r);
    MR_CHECK(u != NULL);
    MR_CHECK(mr_fresh(r));
    if (r->view == 'R') {
      for (k = 0u; k < 2u; k++) {
        uint16_t a = k == 0u ? r->lo : r->hi;
        ivs_memsys_poke_raw_internal(&mr_m, a, 0x0155u);
        ivs_memsys_set_gate_internal(&mr_m, g, 0u);
        if (mr_read(a) != 0xFFFFu) {
          ivs_test_diag("R %04X: reads %04X with gate %s closed", (unsigned)a, (unsigned)mr_read(a), gate);
          return IVS_TEST_FAIL;
        }
        ivs_memsys_set_gate_internal(&mr_m, g, 1u);
        MR_CHECK(mr_read(a) == mr_expect(u, 0x0155u));
      }
    } else {
      ivs_memsys_set_gate_internal(&mr_m, g, 0u);
      memcpy(&mr_snap, &mr_m, sizeof mr_snap);
      mr_write(r->lo, 0x1234u);
      mr_write(r->hi, 0x1234u);
      if (memcmp(&mr_snap, &mr_m, sizeof mr_snap) != 0) {
        ivs_test_diag("W %04X-%04X: a store landed with gate %s closed", (unsigned)r->lo, (unsigned)r->hi, gate);
        return IVS_TEST_FAIL;
      }
      ivs_memsys_set_gate_internal(&mr_m, g, 1u);
      if (u->keep != 0u) {
        int ok;
        uint16_t obs = mr_obs(r->lo, &ok);
        MR_CHECK(ok);
        MR_CHECK(mr_store_kept(u, r->lo, obs, 0x1234u) == IVS_TEST_PASS);
      }
    }
  }
  return IVS_TEST_PASS;
}

/* ---- the decode dump ---- */

static const char *mr_impl_name(uint8_t impl)
{
  switch (impl) {
  case IVS_MEMSYS_IMPL_REAL:
    return "real";
  case IVS_MEMSYS_IMPL_STUB:
    return "stub";
  default:
    return "none";
  }
}

static const char *mr_gate_name(uint8_t gate)
{
  switch (gate) {
  case IVS_MEMSYS_GATE_STIC_REGS:
    return "stic-regs";
  case IVS_MEMSYS_GATE_GMEM:
    return "gmem";
  default:
    return "-";
  }
}

/* One address's structural tuple; w 0xFF marks a dropped-row write that changed the state. */
typedef struct mr_tuple {
  const char *device;
  uint8_t impl;
  uint8_t gate;
  uint8_t w;
  uint16_t decl_mirror; /* declarative mirror, part of the key only for width-0 addresses */
} mr_tuple;

static int mr_same(const mr_tuple *a, const mr_tuple *b)
{
  return strcmp(a->device, b->device) == 0 && a->impl == b->impl && a->gate == b->gate && a->w == b->w
         && (a->w != 0u || a->decl_mirror == b->decl_mirror);
}

/* Behavioural mirror of a merged run with a width. */
static uint32_t mr_mirror_read(uint16_t lo, uint16_t hi)
{
  uint32_t n;
  for (n = 1u; n <= (uint32_t)hi - lo; n++) {
    uint16_t at = (uint16_t)(lo + n);
    uint16_t x;
    uint16_t y;
    ivs_memsys_poke_raw_internal(&mr_m, lo, 0x0000u);
    x = mr_read(at);
    ivs_memsys_poke_raw_internal(&mr_m, lo, 0xFFFFu);
    y = mr_read(at);
    if (x != y) {
      return n;
    }
  }
  return 0u;
}

static uint32_t mr_mirror_write(uint16_t lo, uint16_t hi)
{
  int ok;
  uint16_t obs = mr_obs(lo, &ok);
  uint32_t n;
  if (!ok) {
    return 0u;
  }
  for (n = 1u; n <= (uint32_t)hi - lo; n++) {
    uint16_t x;
    uint16_t y;
    mr_write(lo, 0x0000u);
    x = mr_observe(lo, obs);
    mr_write((uint16_t)(lo + n), 0xFFFFu);
    y = mr_observe(lo, obs);
    if (x != y) {
      return n;
    }
  }
  return 0u;
}

static int mr_emit(char view, uint16_t lo, uint16_t hi, const mr_tuple *t, uint32_t mirror)
{
  char mbuf[12];
  char wbuf[4];
  int k;
  if (mr_nlines >= MR_LINES) {
    return 0;
  }
  if (mirror == 0u) {
    (void)snprintf(mbuf, sizeof mbuf, "-");
  } else {
    (void)snprintf(mbuf, sizeof mbuf, "%lu", (unsigned long)mirror);
  }
  if (t->w == 0xFFu) {
    (void)snprintf(wbuf, sizeof wbuf, "?");
  } else {
    (void)snprintf(wbuf, sizeof wbuf, "%u", (unsigned)t->w);
  }
  k = snprintf(mr_lines[mr_nlines], MR_LINE, "%c %04X-%04X %s w=%s impl=%s mirror=%s gate=%s", view,
               (unsigned)lo, (unsigned)hi, t->device, wbuf, mr_impl_name(t->impl), mbuf,
               mr_gate_name(t->gate));
  if (k < 0 || (size_t)k >= MR_LINE) {
    return 0;
  }
  mr_nlines++;
  return 1;
}

/* Write view, rows the literal gives width 0: every write must leave the state unchanged.
 * Returns 1 when it does; else names the first address that changes it and returns 0.
 * Two sweeps, each followed by a whole-state comparison (review WR-05): FFFF into every address,
 * then 0000. A kept store into a backing that holds v changes the state in the sweep whose value
 * differs from v, so a store kept anywhere in the row is seen whatever the backing held; one
 * sweep that ended on 0000 could not see a store into a zero RAM backing. */
static int mr_dropped_row(const ivs_mp_row *r)
{
  static const uint16_t vals[2] = {0xFFFFu, 0x0000u};
  uint32_t a;
  uint32_t k;
  int same = 1;
  memcpy(&mr_snap, &mr_m, sizeof mr_snap);
  for (k = 0u; k < 2u && same; k++) {
    for (a = r->lo; a <= r->hi; a++) {
      mr_write((uint16_t)a, vals[k]);
    }
    same = memcmp(&mr_snap, &mr_m, sizeof mr_snap) == 0;
  }
  if (same) {
    return 1;
  }
  for (k = 0u; k < 2u; k++) {
    memcpy(&mr_m, &mr_snap, sizeof mr_m);
    for (a = r->lo; a <= r->hi; a++) {
      mr_write((uint16_t)a, vals[k]);
      if (memcmp(&mr_snap, &mr_m, sizeof mr_snap) != 0) {
        ivs_test_diag("W %04X: a write of %04X in literal row %s changed the machine state", (unsigned)a,
                      (unsigned)vals[k], r->line);
        memcpy(&mr_m, &mr_snap, sizeof mr_m);
        return 0;
      }
    }
  }
  memcpy(&mr_m, &mr_snap, sizeof mr_m);
  return 0;
}

/* Probes one view of the current profile and appends its merged lines. */
static int mr_dump_view(uint32_t p, char view, int *clean)
{
  static uint8_t dropped_bad[IVS_MP_ROWS];
  uint32_t view_id = view == 'R' ? IVS_MEMSYS_VIEW_R : IVS_MEMSYS_VIEW_W;
  mr_tuple run;
  uint16_t run_lo = 0u;
  uint32_t a;
  uint32_t i;
  memset(dropped_bad, 0, sizeof dropped_bad);
  memset(&run, 0, sizeof run);
  if (view == 'W') {
    for (i = 0u; i < IVS_MP_ROWS; i++) {
      const ivs_mp_row *r = &ivs_mp_rows[i];
      if (r->view == 'W' && strcmp(r->profile, ivs_profile_get(p)->id) == 0 && r->w == 0u
          && !mr_dropped_row(r)) {
        dropped_bad[i] = 1u;
        *clean = 0;
      }
    }
  }
  for (a = 0u; a < MR_SPACE; a++) {
    uint16_t addr = (uint16_t)a;
    ivs_memsys_desc d;
    mr_tuple t;
    uint16_t x;
    uint16_t y;
    if (ivs_memsys_describe(p, view_id, addr, &d) != IVS_OK) {
      return 0;
    }
    t.device = d.phys_name;
    t.impl = d.impl;
    t.gate = d.gate;
    t.decl_mirror = d.mirror_words;
    if (view == 'R') {
      ivs_memsys_poke_raw_internal(&mr_m, addr, 0xFFFFu);
      x = mr_read(addr);
      ivs_memsys_poke_raw_internal(&mr_m, addr, 0x0000u);
      y = mr_read(addr);
      t.w = (uint8_t)mr_popcount((uint16_t)(x ^ y));
    } else if (mr_row_at('W', a)->w == 0u) {
      t.w = dropped_bad[mr_rowmap[1][a]] != 0u ? 0xFFu : 0u;
    } else {
      int ok;
      uint16_t obs = mr_obs(addr, &ok);
      t.w = 0u;
      if (ok) {
        mr_write(addr, 0xFFFFu);
        x = mr_observe(addr, obs);
        mr_write(addr, 0x0000u);
        y = mr_observe(addr, obs);
        t.w = (uint8_t)mr_popcount((uint16_t)(x ^ y));
      }
    }
    if (a == 0u) {
      run = t;
      run_lo = 0u;
    } else if (!mr_same(&run, &t)) {
      uint16_t hi = (uint16_t)(a - 1u);
      uint32_t m = run.w == 0u || run.w == 0xFFu ? run.decl_mirror
                   : (view == 'R' ? mr_mirror_read(run_lo, hi) : mr_mirror_write(run_lo, hi));
      if (!mr_emit(view, run_lo, hi, &run, m)) {
        return 0;
      }
      run = t;
      run_lo = addr;
    }
  }
  {
    uint32_t m = run.w == 0u || run.w == 0xFFu ? run.decl_mirror
                 : (view == 'R' ? mr_mirror_read(run_lo, 0xFFFFu) : mr_mirror_write(run_lo, 0xFFFFu));
    return mr_emit(view, run_lo, 0xFFFFu, &run, m);
  }
}

static int mr_dump_case(void)
{
  const char *dir = getenv("IVS_TEST_OUT_DIR");
  const char *nonce = getenv("IVS_TEST_NONCE");
  FILE *f = NULL;
  int ok = 1;
  uint32_t i;
  const char *done[IVS_MP_SOURCES];
  size_t ndone = 0u;
  if (nonce == NULL || nonce[0] == '\0') {
    nonce = "none";
  }
  if (dir != NULL && dir[0] != '\0') {
    char path[4096];
    int k = snprintf(path, sizeof path, "%s/memmap-dump.txt", dir);
    MR_CHECK(k > 0 && (size_t)k < sizeof path);
    f = fopen(path, "w");
    MR_CHECK(f != NULL);
  }
  /* One pass per profile, in literal-file order (the generator sorts the files). */
  for (i = 0u; i < IVS_MP_ROWS && ok; i++) {
    const char *pid = ivs_mp_rows[i].profile;
    uint32_t p = mr_profile_index(pid);
    size_t j;
    size_t n_lit = 0u;
    size_t k;
    int clean = 1;
    int seen = 0;
    for (j = 0u; j < ndone; j++) {
      seen = seen || strcmp(done[j], pid) == 0;
    }
    if (seen) {
      continue;
    }
    if (ndone < IVS_MP_SOURCES) {
      done[ndone++] = pid;
    }
    if (p >= ivs_profile_count()) {
      ivs_test_diag("literal map of profile %s has no registry profile", pid);
      ok = 0;
      break;
    }
    if (!mr_map_profile(pid) || !mr_setup(p, pid)) {
      ivs_test_diag("profile %s: literal rows do not tile both views or the setup failed", pid);
      ok = 0;
      break;
    }
    mr_nlines = 0u;
    if (!mr_dump_view(p, 'R', &clean) || !mr_dump_view(p, 'W', &clean)) {
      ivs_test_diag("profile %s: the dump has more than %u lines or a line is too long", pid, MR_LINES);
      ok = 0;
      break;
    }
    if (f != NULL) {
      fprintf(f, "# memmap-dump nonce=%s profile=%s\n", nonce, pid);
      for (k = 0u; k < mr_nlines; k++) {
        fprintf(f, "%s\n", mr_lines[k]);
      }
    }
    /* Compare with the literal projection of this profile, in generator order. */
    k = 0u;
    for (j = 0u; j < IVS_MP_ROWS; j++) {
      if (strcmp(ivs_mp_rows[j].profile, pid) != 0) {
        continue;
      }
      if (ok && (k >= mr_nlines || strcmp(mr_lines[k], ivs_mp_rows[j].line) != 0)) {
        ivs_test_diag("profile %s: dump line %zu is '%s', literal has '%s'", pid, k + 1u,
                      k < mr_nlines ? mr_lines[k] : "(none)", ivs_mp_rows[j].line);
        ok = 0;
      }
      k++;
      n_lit++;
    }
    if (ok && mr_nlines != n_lit) {
      ivs_test_diag("profile %s: dump line %zu is '%s', literal has none", pid, n_lit + 1u, mr_lines[n_lit]);
      ok = 0;
    }
    if (!clean) {
      ok = 0;
    }
  }
  if (f != NULL && fclose(f) != 0) {
    ivs_test_diag("cannot close memmap-dump.txt");
    ok = 0;
  }
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- case table of this family: the dump, one case per literal row, one per gate ---- */

#define MR_GATES_MAX 8u
#define MR_CASES (1u + IVS_MP_ROWS + MR_GATES_MAX)
static char mr_ids[MR_CASES][48];
static const char *mr_gates[MR_GATES_MAX];
static size_t mr_ngates;

int mm_rows_init(void)
{
  uint32_t i;
  size_t j;
  int k = snprintf(mr_ids[0], sizeof mr_ids[0], "memmap.dump.matches-literal");
  if (k <= 0 || (size_t)k >= sizeof mr_ids[0]) {
    return 0;
  }
  for (i = 0u; i < IVS_MP_ROWS; i++) {
    const ivs_mp_row *r = &ivs_mp_rows[i];
    int seen = 0;
    k = snprintf(mr_ids[1u + i], sizeof mr_ids[0], "memmap.row.%c-%04X-%04X", r->view, (unsigned)r->lo,
                 (unsigned)r->hi);
    if (k <= 0 || (size_t)k >= sizeof mr_ids[0]) {
      return 0;
    }
    if (strcmp(r->gate, "-") == 0) {
      continue;
    }
    for (j = 0u; j < mr_ngates; j++) {
      seen = seen || strcmp(mr_gates[j], r->gate) == 0;
    }
    if (!seen) {
      if (mr_ngates >= MR_GATES_MAX) {
        return 0;
      }
      mr_gates[mr_ngates] = r->gate;
      k = snprintf(mr_ids[1u + IVS_MP_ROWS + mr_ngates], sizeof mr_ids[0], "memmap.row-gate.%s-closed", r->gate);
      if (k <= 0 || (size_t)k >= sizeof mr_ids[0]) {
        return 0;
      }
      mr_ngates++;
    }
  }
  return 1;
}

size_t mm_rows_count(void)
{
  return 1u + IVS_MP_ROWS + mr_ngates;
}

const char *mm_rows_id(size_t i)
{
  return mr_ids[i];
}

int mm_rows_run(size_t i)
{
  if (i == 0u) {
    return mr_dump_case();
  }
  if (i <= IVS_MP_ROWS) {
    return mr_row_case(&ivs_mp_rows[i - 1u]);
  }
  return mr_gate_case(mr_gates[i - 1u - IVS_MP_ROWS]);
}

const ivs_test_source *mm_rows_sources(size_t *n)
{
  *n = IVS_MP_SOURCES;
  return ivs_mp_sources;
}
