/* SPDX-License-Identifier: MIT */
/* Suite cpu-sweep (D-16, POL-ILL-01, plan 02-13): the decode table of the core against the Phase 1
 * family table data/cp1610/opcode-families.json, an independent transcription of the formats.
 * cmake/IvsSweepTable.cmake turns the family table into ivs_sweep.inc (build tree only). The
 * family table is an inventory, never an expected-value source of the vectors; here it supplies
 * only what its columns state: leaf row, words, plain base cycles, status bits, sdbd support.
 *
 * Case cpu-sweep.<ROW> runs every decle whose leaf rows include ROW, one step each, from a fixed
 * state (R0-R6 fixed, R7 = $1000, flags all 0 and again all 1) over a fully initialised 64K
 * pattern memory (no access ever reads poison), and checks:
 *   - the core decodes the decle to a leaf row of the family table, with the same word count;
 *   - R7 = $1000 + words, unless the row branches or jumps or its destination field is R7;
 *   - the unit lasts the plain printed cycle count, where the table has one;
 *   - flags outside the row's status_bits keep their values;
 *   - the four MVO@ forms SSS = MMM in 4..7 ($264, $26D, $276, $27F, pdf 55) are refused with
 *     UNSUPPORTED_MVO_SSS_EQ_MMM after the fetch (POL-MVO-01), and nothing else faults;
 *   - with DBD set (POL-SDBD-01/02): a row whose sdbd column reads `supported` is accepted,
 *     except mode 6 (MMM = 110), which pdf 49 and 57 say is not supported; an accepted target
 *     clears DBD, each operand word becomes two (R7 = $1000 + 1 + 2 x (words - 1)) and a plain
 *     base count gains 2 (C34); every other decle is refused with UNSUPPORTED_SDBD_TARGET after
 *     the fetch, R7 at $1000 and DBD kept.
 * Case cpu-sweep.high-bits-65536 steps all 65536 words and checks that each decodes to the same
 * row and gives the same unit (state, cycles, events, accesses) as its low 10 bits (POL-ILL-01).
 * A mismatch is investigated against the page, never fixed by editing either table to agree. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cpu/cp1610.h"
#include "ivs_test.h"

typedef struct ivs_sweep_decle {
  uint8_t leaf[2]; /* row indexes of ivs_sweep_row_id; 255 = none */
  uint8_t words;
  uint8_t base; /* 0 = no plain printed count */
  uint8_t smask;
  uint8_t pc_check;
  uint8_t sdbd;
} ivs_sweep_decle;

#include "ivs_sweep.inc"

#define SWEEP_PC 0x1000u
#define SWEEP_MAX_WRITES 8u
#define SWEEP_MAX_NOTES 6

/* Pattern memory: the word under test at $1000, zero operand words at $1001 and $1002 (a zero
 * displacement, a jump with BB = 00 and DE = 00, a zero address word), a fixed pattern elsewhere;
 * writes of the current step are kept in an overlay that the next step discards. */
typedef struct sweep_bus {
  uint16_t word;
  uint16_t waddr[SWEEP_MAX_WRITES];
  uint16_t wdata[SWEEP_MAX_WRITES];
  unsigned n_writes;
  int overflow;
} sweep_bus;

static uint16_t sweep_pattern(const sweep_bus *b, uint16_t a)
{
  unsigned k;
  for (k = b->n_writes; k > 0u; k--) {
    if (b->waddr[k - 1u] == a) {
      return b->wdata[k - 1u];
    }
  }
  if (a == SWEEP_PC) {
    return b->word;
  }
  if (a == SWEEP_PC + 1u || a == SWEEP_PC + 2u) {
    return 0u;
  }
  return (uint16_t)((((uint32_t)a * 0x9E37u) ^ 0x5A5Au) & 0xFFFFu);
}

static ivs_bus_reply sweep_bus_fn(void *ctx, const ivs_cp1610_access *a)
{
  sweep_bus *b = (sweep_bus *)ctx;
  ivs_bus_reply r;
  r.data = 0u;
  r.wait_cpu_cycles = 0u;
  switch (a->kind) {
  case IVS_ACC_FETCH:
  case IVS_ACC_READ:
  case IVS_ACC_ADAR:
    r.data = sweep_pattern(b, a->addr);
    break;
  case IVS_ACC_WRITE:
  case IVS_ACC_DW:
    if (b->n_writes < SWEEP_MAX_WRITES) {
      b->waddr[b->n_writes] = a->addr;
      b->wdata[b->n_writes] = a->data;
      b->n_writes++;
    } else {
      b->overflow = 1;
    }
    break;
  default:
    break;
  }
  return r;
}

typedef struct sweep_run {
  ivs_cp1610 s;
  ivs_cp1610_result res;
  ivs_cp1610_access log[IVS_CP1610_MAX_ACCESSES];
  ivs_status rc;
} sweep_run;

static void sweep_step(sweep_run *out, uint16_t word, uint8_t status, uint8_t dbd)
{
  static const uint16_t regs[8] = {0x0100u, 0x0201u, 0x0302u, 0x0403u, 0x0504u, 0x0605u, 0x02F0u, SWEEP_PC};
  ivs_cp1610_config cfg;
  ivs_cp1610_lines lines;
  ivs_cp1610_port port;
  sweep_bus bus;
  memset(out, 0, sizeof *out);
  memset(&bus, 0, sizeof bus);
  memset(&lines, 0, sizeof lines);
  bus.word = word;
  memcpy(out->s.r, regs, sizeof regs);
  out->s.status = status;
  out->s.dbd = dbd;
  out->s.run_state = IVS_CP1610_RS_RUNNING;
  ivs_cp1610_config_default(&cfg);
  port.bus = sweep_bus_fn;
  port.ebci = NULL;
  port.ctx = &bus;
  out->rc = ivs_cp1610_step(&out->s, &cfg, &lines, &port, out->log, IVS_CP1610_MAX_ACCESSES, &out->res);
  if (bus.overflow) {
    out->rc = IVS_E_BAD_STATE;
  }
}

static int sweep_notes;

static int sweep_bad(unsigned d, const char *what, unsigned want, unsigned got)
{
  if (sweep_notes++ < SWEEP_MAX_NOTES) {
    ivs_test_diag("decle $%03X: %s: expected %u ($%04X), got %u ($%04X)", d, what, want, want, got, got);
  }
  return 1;
}

static int sweep_is_mvo_refusal(unsigned d)
{
  return d == 0x264u || d == 0x26Du || d == 0x276u || d == 0x27Fu;
}

/* One decle, flags all `status`, DBD clear. Returns the number of failed checks. */
static int sweep_plain(unsigned d, const ivs_sweep_decle *t, uint8_t status)
{
  sweep_run run;
  int bad = 0;
  uint32_t keep = ~(uint32_t)t->smask & IVS_CP1610_STATUS_MASK;
  sweep_step(&run, (uint16_t)d, status, 0u);
  if (run.rc != IVS_OK) {
    return sweep_bad(d, "step status", 0u, (unsigned)(run.rc < 0 ? -run.rc : run.rc));
  }
  if (sweep_is_mvo_refusal(d)) {
    bad += run.res.unit_kind != IVS_CP1610_UNIT_FAULT ? sweep_bad(d, "unit (MVO refusal)", IVS_CP1610_UNIT_FAULT, run.res.unit_kind) : 0;
    bad += run.s.fault_reason != IVS_CP1610_FAULT_UNSUPPORTED_MVO_SSS_EQ_MMM
               ? sweep_bad(d, "fault reason", IVS_CP1610_FAULT_UNSUPPORTED_MVO_SSS_EQ_MMM, run.s.fault_reason)
               : 0;
    bad += run.s.r[7] != SWEEP_PC ? sweep_bad(d, "R7 (refused)", SWEEP_PC, run.s.r[7]) : 0;
    bad += run.res.elapsed_cpu_cycles != 4u ? sweep_bad(d, "cycles (refused)", 4u, run.res.elapsed_cpu_cycles) : 0;
    return bad;
  }
  if (run.res.unit_kind != IVS_CP1610_UNIT_INSTR) {
    return sweep_bad(d, "unit", IVS_CP1610_UNIT_INSTR, run.res.unit_kind);
  }
  if (t->pc_check && run.s.r[7] != SWEEP_PC + t->words) {
    bad += sweep_bad(d, "R7 after the unit", SWEEP_PC + t->words, run.s.r[7]);
  }
  if (t->base != 0u && run.res.elapsed_cpu_cycles != t->base) {
    bad += sweep_bad(d, "cycles", t->base, run.res.elapsed_cpu_cycles);
  }
  if ((((uint32_t)run.s.status ^ status) & keep) != 0u) {
    bad += sweep_bad(d, "flags outside status_bits", status & keep, run.s.status & keep);
  }
  return bad;
}

/* One decle with DBD set (flags 0). */
static int sweep_dbd(unsigned d, const ivs_sweep_decle *t)
{
  sweep_run run;
  int bad = 0;
  int accept = t->sdbd && ((d >> 3) & 7u) != 6u;
  unsigned advance = 1u + 2u * ((unsigned)t->words - 1u);
  sweep_step(&run, (uint16_t)d, 0u, 1u);
  if (run.rc != IVS_OK) {
    return sweep_bad(d, "step status (DBD)", 0u, (unsigned)(run.rc < 0 ? -run.rc : run.rc));
  }
  if (!accept) {
    bad += run.res.unit_kind != IVS_CP1610_UNIT_FAULT ? sweep_bad(d, "unit (DBD refusal)", IVS_CP1610_UNIT_FAULT, run.res.unit_kind) : 0;
    bad += run.s.fault_reason != IVS_CP1610_FAULT_UNSUPPORTED_SDBD_TARGET
               ? sweep_bad(d, "fault reason (DBD)", IVS_CP1610_FAULT_UNSUPPORTED_SDBD_TARGET, run.s.fault_reason)
               : 0;
    bad += run.s.r[7] != SWEEP_PC ? sweep_bad(d, "R7 (DBD refusal)", SWEEP_PC, run.s.r[7]) : 0;
    bad += run.s.dbd != 1u ? sweep_bad(d, "DBD kept", 1u, run.s.dbd) : 0;
    bad += run.res.elapsed_cpu_cycles != 4u ? sweep_bad(d, "cycles (DBD refusal)", 4u, run.res.elapsed_cpu_cycles) : 0;
    return bad;
  }
  if (run.res.unit_kind != IVS_CP1610_UNIT_INSTR) {
    return sweep_bad(d, "unit (DBD target)", IVS_CP1610_UNIT_INSTR, run.res.unit_kind);
  }
  bad += run.s.dbd != 0u ? sweep_bad(d, "DBD cleared", 0u, run.s.dbd) : 0;
  if (t->pc_check && run.s.r[7] != SWEEP_PC + advance) {
    bad += sweep_bad(d, "R7 after the DBD target", SWEEP_PC + advance, run.s.r[7]);
  }
  if (t->base != 0u && run.res.elapsed_cpu_cycles != t->base + 2u) {
    bad += sweep_bad(d, "cycles (DBD target)", t->base + 2u, run.res.elapsed_cpu_cycles);
  }
  if ((run.s.status & ~t->smask & IVS_CP1610_STATUS_MASK) != 0u) {
    bad += sweep_bad(d, "flags outside status_bits (DBD)", 0u, run.s.status & ~t->smask & IVS_CP1610_STATUS_MASK);
  }
  return bad;
}

static int sweep_row_case(size_t ri)
{
  unsigned d;
  unsigned n = 0u;
  int bad = 0;
  sweep_notes = 0;
  for (d = 0u; d < 1024u; d++) {
    const ivs_sweep_decle *t = &ivs_sweep_table[d];
    const ivs_cp1610_row *row;
    size_t k;
    int leaf_ok = 0;
    if (t->leaf[0] != ri && t->leaf[1] != ri) {
      continue;
    }
    n++;
    row = ivs_cp1610_decode((uint16_t)d);
    if (row == NULL) {
      bad += sweep_bad(d, "decoded row (NULL)", 0u, 0u);
      continue;
    }
    for (k = 0u; k < 2u; k++) {
      if (t->leaf[k] < IVS_SWEEP_ROWS && strcmp(row->id, ivs_sweep_row_id[t->leaf[k]]) == 0) {
        leaf_ok = 1;
      }
    }
    if (!leaf_ok) {
      if (sweep_notes++ < SWEEP_MAX_NOTES) {
        ivs_test_diag("decle $%03X: core row %s, family table leaf %s%s%s", d, row->id, ivs_sweep_row_id[t->leaf[0]],
                      t->leaf[1] < IVS_SWEEP_ROWS ? " or " : "",
                      t->leaf[1] < IVS_SWEEP_ROWS ? ivs_sweep_row_id[t->leaf[1]] : "");
      }
      bad++;
      continue;
    }
    if (row->words != t->words) {
      bad += sweep_bad(d, "words", t->words, row->words);
    }
    bad += sweep_plain(d, t, 0x00u);
    bad += sweep_plain(d, t, 0xF0u);
    bad += sweep_dbd(d, t);
  }
  if (n == 0u) {
    ivs_test_diag("row %s: no decle of the family table has it as a leaf", ivs_sweep_row_id[ri]);
    return IVS_TEST_FAIL;
  }
  return bad == 0 ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* Every 16-bit word against its low 10 bits: same row, same unit. The fetched word itself (the
 * first FETCH record's data and the result opcode) is the only allowed difference. */
static int sweep_high_bits(void)
{
  uint32_t w;
  int bad = 0;
  sweep_notes = 0;
  for (w = 0u; w < 65536u; w++) {
    uint16_t lo = (uint16_t)(w & 0x3FFu);
    sweep_run a;
    sweep_run b;
    size_t k;
    int same;
    if (ivs_cp1610_decode((uint16_t)w) != ivs_cp1610_decode(lo)) {
      bad += sweep_bad((unsigned)w, "decoded row differs from the low 10 bits", lo, (unsigned)w);
      continue;
    }
    sweep_step(&a, (uint16_t)w, 0xF0u, 0u);
    sweep_step(&b, lo, 0xF0u, 0u);
    same = a.rc == b.rc && memcmp(&a.s, &b.s, sizeof a.s) == 0 &&
           a.res.elapsed_cpu_cycles == b.res.elapsed_cpu_cycles && a.res.unit_kind == b.res.unit_kind &&
           a.res.events == b.res.events && a.res.n_accesses == b.res.n_accesses;
    for (k = 0u; same && k < a.res.n_accesses && k < IVS_CP1610_MAX_ACCESSES; k++) {
      ivs_cp1610_access x = a.log[k];
      ivs_cp1610_access y = b.log[k];
      if (k == 0u) {
        x.data = 0u;
        y.data = 0u;
      }
      same = x.offset_cpu_cycles == y.offset_cpu_cycles && x.addr == y.addr && x.data == y.data &&
             x.wait_cpu_cycles == y.wait_cpu_cycles && x.kind == y.kind && x.flags == y.flags;
    }
    if (!same) {
      bad += sweep_bad((unsigned)w, "unit differs from the low 10 bits", lo, (unsigned)w);
    }
  }
  return bad == 0 ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

#define SWEEP_CASES (IVS_SWEEP_ROWS + 1u)
static char sweep_ids[SWEEP_CASES][40];

static const char *sweep_id(const void *data, size_t i)
{
  (void)data;
  return sweep_ids[i];
}

static int sweep_run_case(const void *data, size_t i)
{
  (void)data;
  return i < IVS_SWEEP_ROWS ? sweep_row_case(i) : sweep_high_bits();
}

int main(int argc, char **argv)
{
  ivs_test_suite suite;
  size_t i;
  for (i = 0u; i < IVS_SWEEP_ROWS; i++) {
    (void)snprintf(sweep_ids[i], sizeof sweep_ids[i], "cpu-sweep.%s", ivs_sweep_row_id[i]);
  }
  (void)snprintf(sweep_ids[IVS_SWEEP_ROWS], sizeof sweep_ids[IVS_SWEEP_ROWS], "cpu-sweep.high-bits-65536");
  memset(&suite, 0, sizeof suite);
  suite.name = "cpu-sweep";
  suite.count = SWEEP_CASES;
  suite.data = NULL;
  suite.id = sweep_id;
  suite.skip = NULL;
  suite.run = sweep_run_case;
  suite.sources = ivs_sweep_sources;
  suite.n_sources = sizeof ivs_sweep_sources / sizeof ivs_sweep_sources[0];
  return ivs_test_main(argc, argv, &suite);
}
