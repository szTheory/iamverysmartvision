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
 *   minimum gap overall, and 28 is the number an interrupt source must not silently break (plan 06-08 re-proves U, g and
 *   W_log through the full machine with the STIC interrupt level and bus windows live: stic.bounds.*; a bus-grant stall
 *   only lengthens the time between two writes).
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
 *                                    at every address, the premise of U = 48 on the real bus (a bus-grant stall is
 *                                    the stepper's own BUS_GRANT unit sequence, not a wait reply: stic.bounds.no-wait-reply)
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

/* The engine vector rows of the families whose expectations run on a chip: the register programs that psg.state.validate replays
 * (plan 08-10). The generated includes are the ones the family files read; their source lists are not used here. */
#include "ivs_psg_vectors_ampenv.inc"
#include "ivs_psg_vectors_env.inc"
#include "ivs_psg_vectors_noise.inc"
#include "ivs_psg_vectors_reseng.inc"
#include "ivs_psg_vectors_tone.inc"

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
  uint64_t g = 0u;
  uint64_t u = 0u;
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

/* ---- psg.state.validate (plan 08-10, D-15, D-16): the range check of a decoded PSG state ----
 * ivs_psg_state_validate (psg_state.c, bounds and their proofs in psg.h) is what the restore of plan 08-13 asks before it writes
 * anything. A false bound refuses a real snapshot, a missing bound admits an unreachable state, so this case does three things.
 * (1) Reachable states: the reset state, then the state after every chip step of a grid of register programs (the extremes of
 *     every period register, all sixteen envelope shapes, long runs that pass the first reload of the 12-bit tone counter, of the
 *     5-bit noise counter and of the 16-bit envelope counter), after every step of every engine vector the suite already
 *     judges (ampenv, env, noise, reseng, tone: their register writes replayed one chip step at a time), and after every unit
 *     of every stand-in program on a full machine (the unit boundaries are where a snapshot is taken), is accepted. The counters
 *     are also seen at the literal bound: the bounds are met, not only respected. The literals below are the NTSC profile's own
 *     figures (period-zero values 4096, 32 and 65536, two steps per noise or envelope unit, the 17-bit register with mask $12000
 *     and seed 1), asserted against the parameters first so a profile edit shows here.
 * (2) Each bound refuses: one member of a valid state is moved just past its bound (and, where a bound is an edge, to the
 *     edge, which is accepted).
 * (3) The arguments: NULL parameters or state is BAD_ARG, the all-zero state of an OFF instance is BAD_STATE.
 * The Lamp Tone idle and pass scenarios are runner-level programs (their images are not part of this suite): the stand-in
 * programs here and the engine vectors are the programs of this suite; the in-process round trip of plan 08-12 runs the
 * validator on the states of the lamp runs. */
#define PB_TONE_BOUND 4096u      /* tone_counter < 4096: max(0xFFF, period_zero_tone 4096) */
#define PB_NOISE_BOUND 64u       /* noise_counter < 64: 2 steps per unit x max(31, period_zero_noise 32) */
#define PB_ENV_BOUND 131072u     /* env_counter < 131072: 2 steps per unit x max(65535, period_zero_env 65536) */
#define PB_LFSR_LIMIT 0x20000u   /* noise_lfsr < 2^17: the bit length of $12000 | 1 */

typedef struct pb_seen {
  uint32_t tone;
  uint32_t noise;
  uint32_t env;
  uint32_t level;
  uint32_t cycles;
} pb_seen;

static void pb_seen_note(pb_seen *m, const ivs_psg_state *s)
{
  uint32_t i;
  for (i = 0u; i < 3u; i++) {
    if (s->tone_counter[i] > m->tone) {
      m->tone = s->tone_counter[i];
    }
  }
  if (s->noise_counter > m->noise) {
    m->noise = s->noise_counter;
  }
  if (s->env_counter > m->env) {
    m->env = s->env_counter;
  }
  if (s->env_level > m->level) {
    m->level = s->env_level;
  }
  if (s->env_cycles > m->cycles) {
    m->cycles = s->env_cycles;
  }
}

/* 1 when the state is accepted; the diagnostic names the context. */
static int pb_state_ok(const ivs_psg_params *p, const ivs_psg_state *s, pb_seen *m, const char *what, uint64_t at)
{
  if (ivs_psg_state_validate(p, s) != IVS_OK) {
    ivs_test_diag("%s: the state at %llu is refused (tone %u %u %u, noise %u, lfsr %x, env counter %u, level %u, dir %u, held %u, cycles %u)",
                  what, (unsigned long long)at, (unsigned)s->tone_counter[0], (unsigned)s->tone_counter[1],
                  (unsigned)s->tone_counter[2], (unsigned)s->noise_counter, (unsigned)s->noise_lfsr, (unsigned)s->env_counter,
                  (unsigned)s->env_level, (unsigned)s->env_dir, (unsigned)s->env_held, (unsigned)s->env_cycles);
    return 0;
  }
  pb_seen_note(m, s);
  return 1;
}

/* One grid point: the three tone periods derived from tone_p (channel A tone_p, B its complement, C 0 or 1), the noise and envelope
 * periods, the mixer, the amplitude registers on the envelope, then the shape write; steps single chip steps, the state judged
 * after the reset, after the setup writes and after every step. 1 when every state is accepted. */
static int pb_walk(const ivs_psg_params *p, uint32_t tone_p, uint32_t noise_p, uint32_t env_p, uint32_t shape, uint32_t mixer,
                   uint32_t steps, pb_seen *m)
{
  pu_rig r;
  ivs_psg_sink k;
  uint64_t tps = p->sample_period_ticks;
  uint32_t t;
  uint32_t c;
  uint32_t period[3];
  memset(&r, 0, sizeof r);
  r.p = p;
  ivs_psg_state_reset(p, &r.s);
  ivs_psg_sink_init(&k, NULL, 0u);
  if (!pb_state_ok(p, &r.s, m, "reset", 0u)) {
    return 0;
  }
  period[0] = tone_p;
  period[1] = tone_p ^ 0xFFFu;
  period[2] = tone_p == 0u ? 0u : 1u;
  for (c = 0u; c < 3u; c++) {
    pu_write(&r, &k, 0u, IVS_PSG_SLOT_TONE_FINE + c, period[c] & 0xFFu);
    pu_write(&r, &k, 0u, IVS_PSG_SLOT_TONE_COARSE + c, (period[c] >> 8) & 0x0Fu);
    pu_write(&r, &k, 0u, IVS_PSG_SLOT_AMP + c, 0x1Fu);
  }
  pu_write(&r, &k, 0u, IVS_PSG_SLOT_NOISE, noise_p);
  pu_write(&r, &k, 0u, IVS_PSG_SLOT_ENV_FINE, env_p & 0xFFu);
  pu_write(&r, &k, 0u, IVS_PSG_SLOT_ENV_COARSE, (env_p >> 8) & 0xFFu);
  pu_write(&r, &k, 0u, IVS_PSG_SLOT_MIXER, mixer);
  pu_write(&r, &k, 0u, IVS_PSG_SLOT_ENV_SHAPE, shape);
  if (!pb_state_ok(p, &r.s, m, "after the setup writes", 0u)) {
    return 0;
  }
  for (t = 1u; t <= steps; t++) {
    pu_run(&r, &k, (uint64_t)t * tps);
    if (!pb_state_ok(p, &r.s, m, "walk", t)) {
      ivs_test_diag("tone %x noise %u env %x shape %u mixer %x", (unsigned)tone_p, (unsigned)noise_p, (unsigned)env_p,
                    (unsigned)shape, (unsigned)mixer);
      return 0;
    }
  }
  return 1;
}

/* Replays the register programs of n vector rows one chip step at a time (a write at step s is applied after the chip step s,
 * as ivs_pe_eval does), judging the state after every step. */
static int pb_vector_rows(const ivs_psg_params *p, const ivs_pe *rows, size_t n, pb_seen *m, uint64_t *steps_run)
{
  uint64_t tps = p->sample_period_ticks;
  size_t i;
  for (i = 0u; i < n; i++) {
    const ivs_pe *v = &rows[i];
    pu_rig r;
    ivs_psg_sink k;
    uint32_t t;
    uint32_t w;
    if (v->n_writes == 0u || v->steps == 0u) {
      continue;
    }
    memset(&r, 0, sizeof r);
    r.p = p;
    ivs_psg_state_reset(p, &r.s);
    ivs_psg_sink_init(&k, NULL, 0u);
    for (t = 0u; t <= v->steps; t++) {
      for (w = 0u; w < v->n_writes; w++) {
        if (v->writes[w].step == t) {
          pu_write(&r, &k, (uint64_t)t * tps, (uint32_t)v->writes[w].addr - 0x01F0u, v->writes[w].value);
        }
      }
      if (!pb_state_ok(p, &r.s, m, v->id, t)) {
        return 0;
      }
      if (t < v->steps) {
        pu_run(&r, &k, (uint64_t)(t + 1u) * tps);
        *steps_run += 1u;
      }
    }
  }
  return 1;
}

/* One stand-in program on a full machine, the PSG state judged after every unit. */
static int pb_machine_units(uint32_t program, uint32_t units, pb_seen *m)
{
  ivs_ms_inst x;
  uint32_t u;
  int ok;
  x.storage = NULL;
  x.m = NULL;
  if (!ivs_ms_powered_prog(&x, program, 0x5Au)) {
    return 0;
  }
  ok = 1;
  for (u = 0u; ok && u < units; u++) {
    const struct ivs_machine *im = ivs_machine_from_handle(x.m);
    ok = im != NULL && ivs_ms_step(x.m, 1u, NULL) != 0 && pb_state_ok(ivs_psg_params_get(im->profile), &im->psg, m, "stand-in program", u);
    if (!ok) {
      ivs_test_diag("program %u, unit %u", (unsigned)program, (unsigned)u);
    }
  }
  ivs_ms_free(&x);
  return ok;
}

/* 1 when v is refused with BAD_STATE. */
static int pb_refused(const ivs_psg_params *p, const ivs_psg_state *s, const char *what)
{
  if (ivs_psg_state_validate(p, s) != IVS_E_BAD_STATE) {
    ivs_test_diag("%s: not refused", what);
    return 0;
  }
  return 1;
}

static int pb_accepted(const ivs_psg_params *p, const ivs_psg_state *s, const char *what)
{
  if (ivs_psg_state_validate(p, s) != IVS_OK) {
    ivs_test_diag("%s: refused", what);
    return 0;
  }
  return 1;
}

static int pb_state_validate(void)
{
  static const uint32_t tones[] = {0u, 1u, 2u, 0xFFFu};
  static const uint32_t noises[] = {0u, 1u, 31u};
  static const uint32_t envs[] = {1u, 2u, 3u};
  const ivs_psg_params *p = ivs_psg_params_get(pb_profile_index());
  pb_seen seen;
  ivs_psg_state base;
  ivs_psg_state b;
  uint64_t vector_steps = 0u;
  uint32_t ti;
  uint32_t ni;
  uint32_t ei;
  uint32_t shape;
  uint32_t i;
  PB_CHECK(p != NULL);
  /* the figures the literals above stand for */
  PB_CHECK(p->period_zero_tone == 4096u && p->period_zero_noise == 32u && p->period_zero_env == 65536u);
  PB_CHECK(p->noise_steps_per_np == 2u && p->env_steps_per_ep == 2u && p->noise_galois_mask == 0x12000u && p->noise_seed == 1u);
  memset(&seen, 0, sizeof seen);
  /* (1a) the grid: every shape, three envelope periods, four tone and three noise period sets, 160 chip steps each */
  for (shape = 0u; shape < 16u; shape++) {
    for (ei = 0u; ei < sizeof envs / sizeof envs[0]; ei++) {
      for (ti = 0u; ti < sizeof tones / sizeof tones[0]; ti++) {
        for (ni = 0u; ni < sizeof noises / sizeof noises[0]; ni++) {
          PB_CHECK(pb_walk(p, tones[ti], noises[ni], envs[ei], shape, (shape & 1u) != 0u ? 0x00u : 0x38u, 160u, &seen));
        }
      }
    }
  }
  /* (1b) the long runs: past the first reload of each counter at its largest period (the tone counter wraps at 4096 steps,
   * the envelope counter reaches its first state change at 2 x 65536 = 131,072 steps), a rising and a falling shape */
  PB_CHECK(pb_walk(p, 0u, 0u, 0u, 0x0Cu, 0x00u, 140000u, &seen));
  PB_CHECK(pb_walk(p, 0xFFFu, 31u, 0xFFFFu, 0x08u, 0x00u, 9000u, &seen));
  /* the counters were seen at their literal bound, the level and the cycle at 15 */
  PB_CHECK(seen.tone == PB_TONE_BOUND - 1u);
  PB_CHECK(seen.noise == PB_NOISE_BOUND - 1u);
  PB_CHECK(seen.env == PB_ENV_BOUND - 1u);
  PB_CHECK(seen.level == 15u && seen.cycles == 15u);
  /* (1c) the engine vectors */
  PB_CHECK(pb_vector_rows(p, ivs_pe_ampenv, IVS_PV_ampenv_COUNT, &seen, &vector_steps));
  PB_CHECK(pb_vector_rows(p, ivs_pe_env, IVS_PV_env_COUNT, &seen, &vector_steps));
  PB_CHECK(pb_vector_rows(p, ivs_pe_noise, IVS_PV_noise_COUNT, &seen, &vector_steps));
  PB_CHECK(pb_vector_rows(p, ivs_pe_reseng, IVS_PV_reseng_COUNT, &seen, &vector_steps));
  PB_CHECK(pb_vector_rows(p, ivs_pe_tone, IVS_PV_tone_COUNT, &seen, &vector_steps));
  PB_CHECK(vector_steps > 100000u); /* the replay really ran (the envelope vectors alone are about 330,000 steps) */
  /* (1d) every stand-in program on a full machine, after every unit */
  for (i = 0u; i < IVS_MS_PROGS; i++) {
    PB_CHECK(pb_machine_units(i, 6000u, &seen));
  }
  /* (2) a valid base: a rising shape with a short period, 40 steps in (not held, mid cycle) */
  {
    pu_rig r;
    ivs_psg_sink k;
    memset(&r, 0, sizeof r);
    r.p = p;
    ivs_psg_state_reset(p, &r.s);
    ivs_psg_sink_init(&k, NULL, 0u);
    pu_write(&r, &k, 0u, IVS_PSG_SLOT_ENV_FINE, 1u);
    pu_write(&r, &k, 0u, IVS_PSG_SLOT_NOISE, 3u);
    pu_write(&r, &k, 0u, IVS_PSG_SLOT_ENV_SHAPE, 0x0Cu); /* continue, attack: a rising sawtooth */
    pu_run(&r, &k, 40u * (uint64_t)p->sample_period_ticks);
    base = r.s;
  }
  PB_CHECK(pb_accepted(p, &base, "the base state"));
  PB_CHECK(base.env_held == 0u && base.env_level != 0u && base.env_level < 15u); /* mid cycle, so the relation is live */
  for (i = 0u; i < 3u; i++) {
    b = base;
    b.tone_counter[i] = PB_TONE_BOUND - 1u;
    PB_CHECK(pb_accepted(p, &b, "a tone counter at its bound"));
    b.tone_counter[i] = PB_TONE_BOUND;
    PB_CHECK(pb_refused(p, &b, "a tone counter past its bound"));
    b = base;
    b.tone_out[i] = 2u;
    PB_CHECK(pb_refused(p, &b, "a tone output flag of 2"));
    b = base;
    b.reserved[i] = 1u;
    PB_CHECK(pb_refused(p, &b, "a nonzero reserved word"));
  }
  b = base;
  b.noise_counter = PB_NOISE_BOUND - 1u;
  PB_CHECK(pb_accepted(p, &b, "the noise counter at its bound"));
  b.noise_counter = PB_NOISE_BOUND;
  PB_CHECK(pb_refused(p, &b, "the noise counter past its bound"));
  b = base;
  b.noise_lfsr = PB_LFSR_LIMIT - 1u;
  PB_CHECK(pb_accepted(p, &b, "the noise register at its width"));
  b.noise_lfsr = PB_LFSR_LIMIT;
  PB_CHECK(pb_refused(p, &b, "the noise register past its width"));
  b.noise_lfsr = 0u;
  PB_CHECK(pb_refused(p, &b, "the noise register at the lock-up value"));
  b = base;
  b.env_counter = PB_ENV_BOUND - 1u;
  PB_CHECK(pb_accepted(p, &b, "the envelope counter at its bound"));
  b.env_counter = PB_ENV_BOUND;
  PB_CHECK(pb_refused(p, &b, "the envelope counter past its bound"));
  b = base;
  b.env_level = 16u;
  PB_CHECK(pb_refused(p, &b, "an envelope level of 16"));
  b = base;
  b.env_cycles = 16u;
  PB_CHECK(pb_refused(p, &b, "an envelope cycle index of 16"));
  b = base;
  b.env_dir = 2u;
  PB_CHECK(pb_refused(p, &b, "an envelope direction of 2"));
  b = base;
  b.env_held = 2u;
  PB_CHECK(pb_refused(p, &b, "an envelope held flag of 2"));
  /* the relation of a running envelope: one level off, or the direction flipped, is refused; a held generator is free of it */
  b = base;
  b.env_level += 1u;
  PB_CHECK(pb_refused(p, &b, "a level that does not follow the cycle index"));
  b = base;
  b.env_dir ^= 1u;
  PB_CHECK(pb_refused(p, &b, "a direction that does not follow the level"));
  b = base;
  b.env_held = 1u;
  b.env_counter = 0u;
  b.env_level = 7u;
  b.env_cycles = 3u;
  PB_CHECK(pb_accepted(p, &b, "a held generator at a free level"));
  b.env_counter = 1u;
  PB_CHECK(pb_refused(p, &b, "a held generator with a running counter"));
  b = base;
  b.next_sample = 1u;
  PB_CHECK(pb_accepted(p, &b, "the first chip step pending"));
  b.next_sample = 0u;
  PB_CHECK(pb_refused(p, &b, "no chip step pending"));
  /* (3) the arguments and the state of an OFF instance */
  PB_CHECK(ivs_psg_state_validate(NULL, &base) == IVS_E_BAD_ARG);
  PB_CHECK(ivs_psg_state_validate(p, NULL) == IVS_E_BAD_ARG);
  PB_CHECK(ivs_psg_state_validate(NULL, NULL) == IVS_E_BAD_ARG);
  memset(&b, 0, sizeof b);
  PB_CHECK(pb_refused(p, &b, "the all-zero state of an OFF instance"));
  ivs_psg_state_reset(p, &b);
  PB_CHECK(pb_accepted(p, &b, "the reset state"));
  return IVS_TEST_PASS;
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
    {"psg.state.validate", pb_state_validate, NULL},
};
const size_t ivs_psg_bounds_count = sizeof ivs_psg_bounds_cases / sizeof ivs_psg_bounds_cases[0];
