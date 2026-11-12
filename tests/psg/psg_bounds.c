/* SPDX-License-Identifier: MIT */
/* Suite psg, family bounds (AUD-02, plan 05-06, D-14, D-17, A-03, T-05-16): the capacity of the audio buffer
 * rests on two numbers derived here from the CPU model, not from the notes' illustrations: the maximum
 * ticks of one CPU unit U and the minimum gap g between two PSG register writes. Every probe steps the real
 * ivs_cp1610_step over all 1024 decles with a synthetic bus (every access of every kind answers the wait
 * under test, and the core alone decides which kinds take it, A-11), with SDBD off and on, waits 0, 1 and the
 * profile's wait cap, and operand words that include every value of the jump group's first operand.
 *
 * Derivation (recorded in the SUMMARY of plan 05-06):
 *   U on the real bus (wait 0): 12 CPU cycles = 48 master ticks (decle $004 of the jump group).
 *   U with every wait-taking access waiting the cap 35: 12 + 3 x 35 = 117 CPU cycles = 468 ticks: three
 *   wait-taking accesses (the fetch and the two operand fetches of the jump) each wait the cap.
 *   g: a unit that writes the PSG writes once, at CPU cycle offset o of an e-cycle unit; the next write comes
 *   at offset o2 of the following unit, so the gap is e - o + o2. The smallest e - o is 5 (MVO@ at offset 4 of
 *   a 9-cycle unit, MVO direct at offset 6 of an 11-cycle unit); the smallest offset of an instruction's write
 *   is 4, so instruction to instruction the minimum gap is 9 CPU cycles = 36 ticks (the only density reachable in
 *   Phase 5); the interrupt-entry unit writes (kind DW) at offset 2, so 5 + 2 = 7 CPU cycles = 28 ticks is the
 *   minimum gap overall, and 28 is the number a Phase 6 interrupt source must not silently break.
 *   Waits only lengthen a gap (a wait before the write moves its offset up, nothing follows the write).
 * The published psg.max_unit_ticks (468) is asserted equal to the measured maximum with waits, and to
 * (12 + 3 x bus.wait_limit_cpu_cycles) x ticks_per_cpu_cycle. Plan 05-08 publishes psg.min_write_gap_ticks (28)
 * and psg.log_records_per_frame (2,152 = ceil(F / g) + ceil(U / g) + 1 = 2,134 + 17 + 1), where the write log
 * consumes them; psg.bounds.g-and-u re-proves both against the published fields.
 *   psg.bounds.unit-ticks-real-bus   wait 0: the maximum unit is 12 cycles = 48 ticks
 *   psg.bounds.unit-ticks-max        every wait-taking access waits the cap: 117 cycles = 468 ticks, equal to the
 *                                    formula and to the published field
 *   psg.bounds.write-gap-min         36 ticks instruction to instruction, 28 ticks with the interrupt entry
 *   psg.bounds.memsys-bus-never-waits  the memory system and the machine wrapper answer wait 0 for every kind
 *                                    at every address, the premise of U = 48 on the real bus (a Phase 6 stall
 *                                    fails this id loudly)
 *   psg.bounds.slack-covers-worst-case  the audio capacity the info layout publishes covers the samples the
 *                                    last unit's overshoot can add, for the measured unit lengths
 *   psg.bounds.g-and-u               (plan 05-08) over every opcode x SDBD x wait in {0, the cap}, interrupt-entry
 *                                    units stepped with the line asserted: the smallest gap between two PSG
 *                                    writes of ANY two units is 28 ticks (instruction to instruction 36 as a
 *                                    separate assertion) and equals psg.min_write_gap_ticks; the largest unit
 *                                    is 468 and equals psg.max_unit_ticks; hence writes in T ticks are at most
 *                                    (T + U) / g + 1 for every T below, and W_log = ceil(F / g) + ceil(U / g) + 1
 *                                    equals the published psg_log_records_per_frame
 *   psg.bounds.log-capacity-formula  the arithmetic: 2,134, 17, 2,152, 34,432 bytes per frame slot, 71 slots and
 *                                    2,444,672 bytes for a run of 1 << 22 ticks, through ivs_info and a run call
 *   psg.bounds.log-bound-at-g        the machine bus wrapper driven with accesses spaced exactly 28 ticks apart
 *                                    for one frame slot plus the longest unit: the records stay within W_log
 *                                    and the log bytes within the frame slot's capacity */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cpu/cp1610.h"
#include "ivs/ivs.h"
#include "ivs/ivs_run.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "machine_cases.h"
#include "memmap/memsys.h"
#include "psg/psg.h"
#include "psg_cases.h"
#include "psg_util.h"

#define PB_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

#define PB_PC 0x1000u
#define PB_PSG_LO 0x01F0u
#define PB_PSG_HI 0x01FFu

typedef struct pb_bus {
  uint16_t decle;
  uint16_t op1;
  uint16_t op2;
  uint16_t wait;
} pb_bus;

static uint16_t pb_pattern(uint16_t a)
{
  return (uint16_t)((((uint32_t)a * 0x9E37u) ^ 0x5A5Au) & 0x3FFu);
}

static ivs_bus_reply pb_bus_fn(void *ctx, const ivs_cp1610_access *a)
{
  const pb_bus *b = (const pb_bus *)ctx;
  ivs_bus_reply r;
  r.data = 0u;
  r.wait_cpu_cycles = b->wait;
  switch (a->kind) {
  case IVS_ACC_FETCH:
  case IVS_ACC_READ:
  case IVS_ACC_ADAR:
    if (a->addr == PB_PC) {
      r.data = b->decle;
    } else if (a->addr == PB_PC + 1u) {
      r.data = b->op1;
    } else if (a->addr == PB_PC + 2u) {
      r.data = b->op2;
    } else {
      r.data = pb_pattern(a->addr);
    }
    break;
  case IVS_ACC_IAB:
    r.data = (uint16_t)PB_PC; /* the interrupt vector */
    break;
  default:
    break;
  }
  return r;
}

typedef struct pb_unit {
  ivs_cp1610_result res;
  ivs_cp1610_access log[IVS_CP1610_MAX_ACCESSES];
  uint32_t n_psg_writes;
  uint32_t psg_offset; /* CPU cycle offset of the (first) PSG write, when n_psg_writes is nonzero */
  ivs_status rc;
} pb_unit;

/* One unit: an instruction decle (entry 0) or an interrupt entry (entry 1) from a state whose R0..R6 are
 * fill (R6 = fill too), with the wait of the bus, operand words op1, op2 and the SDBD prefix state dbd. */
static void pb_step(pb_unit *u, uint32_t entry, uint32_t decle, uint32_t dbd, uint32_t wait, uint32_t op1,
                    uint32_t op2, uint16_t fill, uint32_t wait_cap)
{
  ivs_cp1610 cpu;
  ivs_cp1610_config cfg;
  ivs_cp1610_lines lines;
  ivs_cp1610_port port;
  pb_bus bus;
  uint32_t i;
  memset(u, 0, sizeof *u);
  memset(&cpu, 0, sizeof cpu);
  memset(&lines, 0, sizeof lines);
  bus.decle = (uint16_t)decle;
  bus.op1 = (uint16_t)op1;
  bus.op2 = (uint16_t)op2;
  bus.wait = (uint16_t)wait;
  for (i = 0u; i < 7u; i++) {
    cpu.r[i] = fill;
  }
  cpu.r[7] = (uint16_t)PB_PC;
  cpu.run_state = IVS_CP1610_RS_RUNNING;
  cpu.dbd = (uint8_t)dbd;
  if (entry != 0u) {
    cpu.prev_interruptible = 1u;
    lines.intr = 1u;
  }
  ivs_cp1610_config_default(&cfg);
  cfg.wait_limit_cpu_cycles = (uint8_t)wait_cap;
  port.bus = pb_bus_fn;
  port.ebci = NULL;
  port.ctx = &bus;
  u->rc = ivs_cp1610_step(&cpu, &cfg, &lines, &port, u->log, IVS_CP1610_MAX_ACCESSES, &u->res);
  for (i = 0u; i < u->res.n_accesses && i < IVS_CP1610_MAX_ACCESSES; i++) {
    const ivs_cp1610_access *a = &u->log[i];
    if ((a->kind == IVS_ACC_WRITE || a->kind == IVS_ACC_DW) && a->addr >= PB_PSG_LO && a->addr <= PB_PSG_HI) {
      if (u->n_psg_writes == 0u) {
        u->psg_offset = a->offset_cpu_cycles;
      }
      u->n_psg_writes++;
    }
  }
}

static uint32_t pb_profile_index(void)
{
  uint32_t p;
  for (p = 0u; p < ivs_profile_count(); p++) {
    if (strcmp(ivs_profile_get(p)->id, IVS_MS_PROFILE) == 0) {
      return p;
    }
  }
  return ivs_profile_count();
}

/* Maximum elapsed CPU cycles of one unit over every decle, SDBD off and on, the fills and operand sets below,
 * for a bus answering wait. 0 when a step is refused (a probe failure). *units gets the units stepped. */
static uint32_t pb_max_cycles(uint32_t wait, uint32_t wait_cap, uint64_t *units)
{
  static const uint16_t fills[2] = {0x0100u, 0x01F0u};
  static const uint16_t ops[3] = {0x0000u, 0x03FFu, 0x0155u};
  uint32_t best = 0u;
  uint32_t d;
  for (d = 0u; d < 1024u; d++) {
    uint32_t dbd;
    for (dbd = 0u; dbd < 2u; dbd++) {
      uint32_t f;
      for (f = 0u; f < 2u; f++) {
        uint32_t a;
        for (a = 0u; a < 3u; a++) {
          uint32_t b;
          uint32_t n1 = d == 0x004u ? 1024u : 1u; /* the jump group: every first operand word */
          uint32_t k;
          for (b = 0u; b < 3u; b++) {
            for (k = 0u; k < n1; k++) {
              pb_unit u;
              uint32_t op1 = d == 0x004u ? k : ops[a];
              pb_step(&u, 0u, d, dbd, wait, op1, ops[b], fills[f], wait_cap);
              (*units)++;
              if (u.rc != IVS_OK) {
                return 0u;
              }
              if (u.res.elapsed_cpu_cycles > best) {
                best = u.res.elapsed_cpu_cycles;
              }
            }
          }
        }
      }
    }
  }
  return best;
}

static int pb_unit_ticks_real_bus(void)
{
  const ivs_profile_desc *p = ivs_profile_get(pb_profile_index());
  uint64_t units = 0u;
  uint32_t best;
  uint32_t entry_max = 0u;
  uint32_t fill;
  PB_CHECK(p != NULL);
  best = pb_max_cycles(0u, p->wait_limit_cpu_cycles, &units);
  /* the interrupt-entry unit is shorter than the longest instruction */
  for (fill = 0u; fill < 2u; fill++) {
    pb_unit u;
    pb_step(&u, 1u, 0u, 0u, 0u, 0u, 0u, fill != 0u ? 0x01F0u : 0x0100u, p->wait_limit_cpu_cycles);
    PB_CHECK(u.rc == IVS_OK && u.res.unit_kind == IVS_CP1610_UNIT_INT_ENTRY);
    if (u.res.elapsed_cpu_cycles > entry_max) {
      entry_max = u.res.elapsed_cpu_cycles;
    }
  }
  ivs_test_diag("wait 0: %llu units stepped, maximum %u CPU cycles, interrupt entry %u", (unsigned long long)units,
                (unsigned)best, (unsigned)entry_max);
  PB_CHECK(best == 12u);
  PB_CHECK(best * p->ticks_per_cpu_cycle == 48u);
  PB_CHECK(entry_max <= best);
  return IVS_TEST_PASS;
}

static int pb_unit_ticks_max(void)
{
  const ivs_profile_desc *p = ivs_profile_get(pb_profile_index());
  const ivs_psg_params *pp = ivs_psg_params_get(pb_profile_index());
  uint64_t units = 0u;
  uint32_t w1;
  uint32_t wl;
  PB_CHECK(p != NULL && pp != NULL);
  w1 = pb_max_cycles(1u, p->wait_limit_cpu_cycles, &units);
  wl = pb_max_cycles(p->wait_limit_cpu_cycles, p->wait_limit_cpu_cycles, &units);
  ivs_test_diag("%llu units stepped; maximum %u CPU cycles with every wait 1, %u with every wait %u",
                (unsigned long long)units, (unsigned)w1, (unsigned)wl, (unsigned)p->wait_limit_cpu_cycles);
  PB_CHECK(w1 == 12u + 3u * 1u);
  /* the literal: 12 + 3 x 35 = 117 cycles = 468 ticks, for the profile's wait cap of 35 */
  PB_CHECK(p->wait_limit_cpu_cycles == 35u);
  PB_CHECK(wl == 117u);
  PB_CHECK(wl * p->ticks_per_cpu_cycle == 468u);
  PB_CHECK(wl * p->ticks_per_cpu_cycle == (12u + 3u * p->wait_limit_cpu_cycles) * p->ticks_per_cpu_cycle);
  PB_CHECK(pp->max_unit_ticks == 468u);
  PB_CHECK(wl * p->ticks_per_cpu_cycle == pp->max_unit_ticks);
  return IVS_TEST_PASS;
}

/* Minimum e - o and minimum o over the units that write the PSG, for the bus wait; instruction units and the
 * interrupt-entry unit separately. Returns 0 on a refused step or a unit with more than one PSG write. */
typedef struct pb_gap {
  uint32_t instr_e_minus_o;
  uint32_t instr_o;
  uint32_t entry_e_minus_o;
  uint32_t entry_o;
  uint64_t instr_writers;
  uint64_t entry_writers;
} pb_gap;

static int pb_gap_scan(pb_gap *g, uint32_t wait, uint32_t wait_cap)
{
  static const uint16_t fills[2] = {0x01F0u, 0x01F1u};
  uint32_t d;
  g->instr_e_minus_o = 0xFFFFu;
  g->instr_o = 0xFFFFu;
  g->entry_e_minus_o = 0xFFFFu;
  g->entry_o = 0xFFFFu;
  g->instr_writers = 0u;
  g->entry_writers = 0u;
  for (d = 0u; d < 1024u; d++) {
    uint32_t dbd;
    for (dbd = 0u; dbd < 2u; dbd++) {
      uint32_t f;
      for (f = 0u; f < 2u; f++) {
        /* operand words 0x1F0 (a direct address) and, under SDBD, the low then the high byte of it */
        uint32_t op1 = dbd != 0u ? 0x0F0u : 0x1F0u;
        uint32_t op2 = dbd != 0u ? 0x001u : 0x1F0u;
        pb_unit u;
        pb_step(&u, 0u, d, dbd, wait, op1, op2, fills[f], wait_cap);
        if (u.rc != IVS_OK || u.n_psg_writes > 1u) {
          return 0;
        }
        if (u.n_psg_writes == 1u) {
          uint32_t e = u.res.elapsed_cpu_cycles;
          g->instr_writers++;
          if (e - u.psg_offset < g->instr_e_minus_o) {
            g->instr_e_minus_o = e - u.psg_offset;
          }
          if (u.psg_offset < g->instr_o) {
            g->instr_o = u.psg_offset;
          }
        }
      }
    }
  }
  {
    uint32_t f;
    for (f = 0u; f < 2u; f++) {
      pb_unit u;
      pb_step(&u, 1u, 0u, 0u, wait, 0u, 0u, fills[f], wait_cap);
      if (u.rc != IVS_OK || u.res.unit_kind != IVS_CP1610_UNIT_INT_ENTRY || u.n_psg_writes > 1u) {
        return 0;
      }
      if (u.n_psg_writes == 1u) {
        g->entry_writers++;
        if (u.res.elapsed_cpu_cycles - u.psg_offset < g->entry_e_minus_o) {
          g->entry_e_minus_o = u.res.elapsed_cpu_cycles - u.psg_offset;
        }
        if (u.psg_offset < g->entry_o) {
          g->entry_o = u.psg_offset;
        }
      }
    }
  }
  return 1;
}

static int pb_write_gap_min(void)
{
  const ivs_profile_desc *p = ivs_profile_get(pb_profile_index());
  uint32_t tpc;
  uint32_t waits[3];
  uint32_t i;
  uint32_t instr0 = 0u;
  uint32_t all0 = 0u;
  PB_CHECK(p != NULL);
  tpc = p->ticks_per_cpu_cycle;
  waits[0] = 0u;
  waits[1] = 1u;
  waits[2] = p->wait_limit_cpu_cycles;
  for (i = 0u; i < 3u; i++) {
    pb_gap g;
    uint32_t instr_gap;
    uint32_t all_gap;
    PB_CHECK(pb_gap_scan(&g, waits[i], p->wait_limit_cpu_cycles));
    PB_CHECK(g.instr_writers > 0u && g.entry_writers > 0u);
    instr_gap = g.instr_e_minus_o + g.instr_o;
    all_gap = g.entry_o < g.instr_o ? g.entry_o : g.instr_o;
    all_gap += g.instr_e_minus_o < g.entry_e_minus_o ? g.instr_e_minus_o : g.entry_e_minus_o;
    ivs_test_diag("wait %u: %llu instruction writers, e - o >= %u, o >= %u; interrupt entry e - o %u, o %u; gap "
                  "instruction %u, overall %u CPU cycles",
                  (unsigned)waits[i], (unsigned long long)g.instr_writers, (unsigned)g.instr_e_minus_o,
                  (unsigned)g.instr_o, (unsigned)g.entry_e_minus_o, (unsigned)g.entry_o, (unsigned)instr_gap,
                  (unsigned)all_gap);
    if (i == 0u) {
      instr0 = instr_gap;
      all0 = all_gap;
      /* the literals: 9 CPU cycles = 36 ticks instruction to instruction, 7 CPU cycles = 28 ticks overall */
      PB_CHECK(instr_gap == 9u && instr_gap * tpc == 36u);
      PB_CHECK(all_gap == 7u && all_gap * tpc == 28u);
    } else {
      /* a wait only lengthens a gap */
      PB_CHECK(instr_gap >= instr0 && all_gap >= all0);
    }
  }
  return IVS_TEST_PASS;
}

static int pb_memsys_bus_never_waits(void)
{
  ivs_ms_inst x;
  ivs_machine_bus_ctx ctx;
  ivs_psg_sink sink;
  struct ivs_machine *im;
  uint32_t kind;
  uint32_t addr;
  uint32_t profile = pb_profile_index();
  int rc = IVS_TEST_FAIL;
  if (!ivs_ms_powered(&x, IVS_MS_CONTENT_A, 0x5Au)) {
    return IVS_TEST_FAIL;
  }
  im = ivs_machine_from_handle_mut(x.m);
  if (im == NULL) {
    goto done;
  }
  ivs_psg_sink_init(&sink, NULL, 0u);
  memset(&ctx, 0, sizeof ctx);
  ctx.m = im;
  ctx.psg = ivs_psg_params_get(profile);
  ctx.sink = &sink;
  ctx.ticks_per_cpu_cycle = ivs_profile_get(profile)->ticks_per_cpu_cycle;
  for (kind = IVS_ACC_FETCH; kind <= IVS_ACC_STROBE_PCIT; kind++) {
    for (addr = 0u; addr < 0x10000u; addr++) {
      ivs_cp1610_access a;
      ivs_bus_reply r1;
      ivs_bus_reply r2;
      memset(&a, 0, sizeof a);
      a.kind = (uint8_t)kind;
      a.addr = (uint16_t)addr;
      a.data = (uint16_t)(addr * 7u);
      a.flags = kind == IVS_ACC_IAB ? IVS_ACC_F_INTERRUPT : 0u;
      r1 = ivs_memsys_bus(&im->mem, &a);
      ctx.unit_start_tick = 0u;
      r2 = ivs_machine_bus(&ctx, &a);
      if (r1.wait_cpu_cycles != 0u || r2.wait_cpu_cycles != 0u) {
        ivs_test_diag("kind %u addr %04X waits %u / %u", (unsigned)kind, (unsigned)addr, (unsigned)r1.wait_cpu_cycles,
                      (unsigned)r2.wait_cpu_cycles);
        goto done;
      }
    }
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* The info layout's capacity in samples for a run of t ticks: audio_bytes_per_frame x (t / F + 1) / 4. */
static int pb_slack_covers(void)
{
  const ivs_profile_desc *p = ivs_profile_get(pb_profile_index());
  ivs_ms_inst x;
  ivs_info info;
  uint64_t frame;
  uint64_t per_frame_samples;
  uint64_t units = 0u;
  uint32_t lens[2];
  uint32_t li;
  static const uint64_t spans[7] = {1u, 2u, 47u, 59735u, 59736u, 59737u, 3u * 59736u - 1u};
  uint32_t si;
  int rc = IVS_TEST_FAIL;
  PB_CHECK(p != NULL);
  if (!ivs_ms_powered(&x, IVS_MS_CONTENT_A, 0x5Au)) {
    return IVS_TEST_FAIL;
  }
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info(x.m, &info) != IVS_OK || info.audio_sample_bytes != 4u || info.audio_sample_ticks != 16u) {
    ivs_test_diag("info refused or the sample figures are not 4 bytes and 16 ticks");
    goto done;
  }
  frame = info.nominal_frame_ticks;
  per_frame_samples = info.audio_bytes_per_frame / info.audio_sample_bytes;
  /* the unit lengths of the real bus (wait 0) and of the worst bus (every wait the cap), measured above */
  lens[0] = pb_max_cycles(0u, p->wait_limit_cpu_cycles, &units) * p->ticks_per_cpu_cycle;
  lens[1] = pb_max_cycles(p->wait_limit_cpu_cycles, p->wait_limit_cpu_cycles, &units) * p->ticks_per_cpu_cycle;
  if (lens[0] != 48u || lens[1] != 468u) {
    ivs_test_diag("measured maximum unit ticks %u and %u, want 48 and 468", (unsigned)lens[0], (unsigned)lens[1]);
    goto done;
  }
  for (li = 0u; li < 2u; li++) {
    for (si = 0u; si < 7u; si++) {
      /* A run of t ticks from an entry at phase phi (0..15) ends at its first unit boundary at or after the
       * target: at most t + U - 1 ticks later. It emits floor((phi + t + U - 1) / 16) - floor(phi / 16)
       * samples at most; the worst phase is 15. */
      uint64_t t = spans[si];
      uint64_t worst = (15u + t + lens[li] - 1u) / 16u;
      uint64_t capacity = per_frame_samples * (t / frame + 1u);
      if (worst > capacity) {
        ivs_test_diag("run of %llu ticks, unit %u ticks: up to %llu samples against a capacity of %llu",
                      (unsigned long long)t, (unsigned)lens[li], (unsigned long long)worst,
                      (unsigned long long)capacity);
        goto done;
      }
    }
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* ---- plan 05-08: the published write-log bounds ---- */

/* PB_CHECK that leaves through the function's cleanup label. */
#define PB_CHECKG(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      goto done;                                                                                       \
    }                                                                                                  \
  } while (0)

/* The smallest gap between two PSG writes of any two units, in CPU cycles, for the bus wait: the least e - o over
 * every writer (instruction or interrupt entry) plus the least o over every writer. */
static int pb_overall_gap(uint32_t wait, uint32_t wait_cap, uint32_t *instr_gap, uint32_t *all_gap)
{
  pb_gap g;
  uint32_t min_eo;
  uint32_t min_o;
  if (!pb_gap_scan(&g, wait, wait_cap) || g.instr_writers == 0u || g.entry_writers == 0u) {
    return 0;
  }
  *instr_gap = g.instr_e_minus_o + g.instr_o;
  min_eo = g.instr_e_minus_o < g.entry_e_minus_o ? g.instr_e_minus_o : g.entry_e_minus_o;
  min_o = g.instr_o < g.entry_o ? g.instr_o : g.entry_o;
  *all_gap = min_eo + min_o;
  return 1;
}

static uint64_t pb_ceil_div(uint64_t a, uint64_t b)
{
  return (a + b - 1u) / b;
}

static int pb_g_and_u(void)
{
  const ivs_profile_desc *p = ivs_profile_get(pb_profile_index());
  const ivs_psg_params *pp = ivs_psg_params_get(pb_profile_index());
  ivs_ms_inst x;
  ivs_info info;
  uint64_t units = 0u;
  uint64_t frame;
  uint64_t g;
  uint64_t u;
  uint64_t t;
  uint32_t waits[2];
  uint32_t i;
  static const uint64_t spans[8] = {0u, 1u, 27u, 28u, 29u, 59735u, 59736u, 3u * 59736u};
  int rc = IVS_TEST_FAIL;
  x.storage = NULL;
  x.m = NULL;
  PB_CHECKG(p != NULL && pp != NULL);
  waits[0] = 0u;
  waits[1] = p->wait_limit_cpu_cycles;
  for (i = 0u; i < 2u; i++) {
    uint32_t instr_gap = 0u;
    uint32_t all_gap = 0u;
    PB_CHECKG(pb_overall_gap(waits[i], p->wait_limit_cpu_cycles, &instr_gap, &all_gap));
    if (i == 0u) {
      PB_CHECKG(instr_gap * p->ticks_per_cpu_cycle == 36u); /* instruction to instruction: the density Phase 5 reaches */
      PB_CHECKG(all_gap * p->ticks_per_cpu_cycle == 28u);   /* with an interrupt-entry unit: the published bound */
    } else {
      PB_CHECKG(all_gap * p->ticks_per_cpu_cycle >= 28u); /* a wait only lengthens a gap */
    }
    if (i == 0u) {
      g = (uint64_t)all_gap * p->ticks_per_cpu_cycle;
    }
  }
  PB_CHECKG(g == 28u);
  PB_CHECKG(g == (uint64_t)p->psg_min_write_gap_ticks); /* equals the published psg.min_write_gap_ticks */
  /* U: the longest unit with every wait-taking access waiting the cap */
  u = (uint64_t)pb_max_cycles(p->wait_limit_cpu_cycles, p->wait_limit_cpu_cycles, &units) * p->ticks_per_cpu_cycle;
  PB_CHECKG(u == 468u && u == pp->max_unit_ticks);
  /* writes in T ticks: n writes at least g apart span at least (n - 1) g, and a run of T ticks lasts at most
   * T + U - 1 ticks, so n <= (T + U - 1) / g + 1 <= (T + U) / g + 1; checked against W_log for T = F */
  for (t = 0u; t < sizeof spans / sizeof spans[0]; t++) {
    uint64_t span = spans[t];
    uint64_t n = (span + u - 1u) / g + 1u; /* the densest schedule: a write every g ticks over span + U - 1 ticks */
    PB_CHECKG(n <= (span + u) / g + 1u);
  }
  if (!ivs_ms_powered(&x, IVS_MS_CONTENT_A, 0x5Au)) {
    return IVS_TEST_FAIL;
  }
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info(x.m, &info) != IVS_OK) {
    goto done;
  }
  frame = info.nominal_frame_ticks;
  PB_CHECKG(frame == 59736u);
  PB_CHECKG(pb_ceil_div(frame, g) + pb_ceil_div(u, g) + 1u == 2152u);
  PB_CHECKG(info.psg_log_records_per_frame == 2152u && info.psg_log_record_bytes == 16u);
  PB_CHECKG(p->psg_log_records_per_frame == 2152u);
  PB_CHECKG(pb_ceil_div(frame, g) + pb_ceil_div(u, g) + 1u == (uint64_t)info.psg_log_records_per_frame);
  /* the densest window of one frame slot: a write every g ticks over the frame and the longest unit */
  PB_CHECKG((frame + u - 1u) / g + 1u <= (uint64_t)info.psg_log_records_per_frame);
  rc = IVS_TEST_PASS;
done:
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  return rc;
}

static int pb_log_capacity_formula(void)
{
  ivs_ms_inst x;
  ivs_info info;
  uint64_t need;
  int rc = IVS_TEST_FAIL;
  x.storage = NULL;
  x.m = NULL;
  /* the literal arithmetic of D-14 / A-03: F = 262 x 57 x 4, g = 28, U = 468 */
  PB_CHECKG(262u * 57u * 4u == 59736u);
  PB_CHECKG((59736u + 27u) / 28u == 2134u);
  PB_CHECKG((468u + 27u) / 28u == 17u);
  PB_CHECKG(2134u + 17u + 1u == 2152u);
  PB_CHECKG(16u * 2152u == 34432u);
  PB_CHECKG((1u << 22) / 59736u + 1u == 71u);
  PB_CHECKG(71u * 34432u == 2444672u);
  if (!ivs_ms_powered(&x, IVS_MS_CONTENT_A, 0x5Au)) {
    return IVS_TEST_FAIL;
  }
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  PB_CHECKG(ivs_machine_info(x.m, &info) == IVS_OK);
  PB_CHECKG(info.psg_log_record_bytes * info.psg_log_records_per_frame == 34432u);
  need = pu_log_need(x.m, 0u, 1u << 22);
  PB_CHECKG(need == 2444672u);
  /* through the run call: one byte below is refused (RUN_PSGLOG_SMALL), the exact capacity is accepted */
  {
    ivs_run_request q;
    ivs_run_report rep;
    uint8_t *log = malloc((size_t)need);
    uint8_t *audio = malloc((size_t)pu_need(x.m, 0u, 1u << 22)); /* requested, so the long run is not a counting run */
    PB_CHECKG(log != NULL && audio != NULL);
    memset(&q, 0, sizeof q);
    q.struct_size = (uint32_t)sizeof q;
    q.target_ticks = 1u << 22;
    q.audio = audio;
    q.audio_capacity = pu_need(x.m, 0u, 1u << 22);
    q.psg_log = log;
    q.psg_log_capacity = need - 1u;
    memset(&rep, 0, sizeof rep);
    rep.struct_size = (uint32_t)sizeof rep;
    if (ivs_machine_run(x.m, &q, &rep) != IVS_E_BUFFER_TOO_SMALL || rep.refusal_reason != IVS_REASON_RUN_PSGLOG_SMALL) {
      free(log);
      free(audio);
      ivs_test_diag("one byte below the capacity was not refused as PSGLOG_SMALL");
      goto done;
    }
    q.psg_log_capacity = need;
    memset(&rep, 0, sizeof rep);
    rep.struct_size = (uint32_t)sizeof rep;
    if (ivs_machine_run(x.m, &q, &rep) != IVS_OK) {
      free(log);
      free(audio);
      goto done;
    }
    free(log);
    free(audio);
  }
  rc = IVS_TEST_PASS;
done:
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  return rc;
}

static int pb_log_bound_at_g(void)
{
  ivs_ms_inst x;
  ivs_machine_bus_ctx ctx;
  ivs_psg_sink sink;
  struct ivs_machine *im;
  uint8_t *log = NULL;
  uint64_t cap;
  uint64_t i;
  uint64_t frame = 59736u;
  uint64_t g = 28u;
  uint64_t u = 468u;
  uint32_t profile = pb_profile_index();
  int rc = IVS_TEST_FAIL;
  x.storage = NULL;
  x.m = NULL;
  if (!ivs_ms_powered(&x, IVS_MS_CONTENT_A, 0x5Au)) {
    return IVS_TEST_FAIL;
  }
  im = ivs_machine_from_handle_mut(x.m);
  PB_CHECKG(im != NULL);
  cap = 34432u; /* one frame slot: a run of fewer than F ticks needs max(target - now, 0) / F + 1 = 1 slot */
  log = malloc((size_t)cap);
  PB_CHECKG(log != NULL);
  memset(log, 0xA5u, (size_t)cap);
  ivs_psg_sink_init(&sink, NULL, 0u);
  ivs_psg_sink_log(&sink, log, cap);
  memset(&ctx, 0, sizeof ctx);
  ctx.m = im;
  ctx.psg = ivs_psg_params_get(profile);
  ctx.sink = &sink;
  ctx.ticks_per_cpu_cycle = ivs_profile_get(profile)->ticks_per_cpu_cycle;
  /* a run of F - 1 ticks overshoots by at most U - 1: its writes lie below F - 1 + U - 1 + 1; space them 28 apart */
  for (i = 0u; i * g < frame - 1u + u - 1u; i++) {
    ivs_cp1610_access a;
    memset(&a, 0, sizeof a);
    a.kind = IVS_ACC_WRITE;
    a.addr = 0x01FBu;
    a.data = (uint16_t)(i & 0x0Fu);
    a.offset_cpu_cycles = 0u;
    ctx.unit_start_tick = i * g;
    (void)ivs_machine_bus(&ctx, &a);
  }
  ivs_test_diag("%llu records at gap 28 over %llu ticks", (unsigned long long)i, (unsigned long long)(frame - 1u + u - 1u));
  PB_CHECKG(i <= 2152u);
  PB_CHECKG(sink.log_overflow == 0u && sink.log_written == 16u * i && sink.log_records == i);
  PB_CHECKG(sink.log_written <= cap);
  PB_CHECKG(im->obs.psglog.count == i);
  /* spaced one tick closer, the slot would overflow: the bound is not slack beyond one record */
  PB_CHECKG((frame - 1u + u - 1u) / (g - 1u) + 1u > 2152u);
  rc = IVS_TEST_PASS;
done:
  free(log);
  ivs_ms_free(&x);
  return rc;
}

const ivs_test_case ivs_psg_bounds_cases[] = {
    {"psg.bounds.unit-ticks-real-bus", pb_unit_ticks_real_bus, NULL},
    {"psg.bounds.unit-ticks-max", pb_unit_ticks_max, NULL},
    {"psg.bounds.write-gap-min", pb_write_gap_min, NULL},
    {"psg.bounds.memsys-bus-never-waits", pb_memsys_bus_never_waits, NULL},
    {"psg.bounds.slack-covers-worst-case", pb_slack_covers, NULL},
    {"psg.bounds.g-and-u", pb_g_and_u, NULL},
    {"psg.bounds.log-capacity-formula", pb_log_capacity_formula, NULL},
    {"psg.bounds.log-bound-at-g", pb_log_bound_at_g, NULL},
};
const size_t ivs_psg_bounds_count = sizeof ivs_psg_bounds_cases / sizeof ivs_psg_bounds_cases[0];
