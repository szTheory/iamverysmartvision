/* SPDX-License-Identifier: MIT */
/* Suite run, family diag-ctrl (plan 05-11, D-24, D-05, D-27; INP-01..03, API-03, AUD-01, AUD-02): the
 * second project-authored diagnostic, ivs-diag-ctrl-psg-1 (fixtures/FX-diag-ctrl-psg-1/
 * diag-ctrl-psg-1.listing.txt), packed by tools/pack into the build-tree IVSC container whose bytes
 * ivs_images.inc carries, entered from a power-on reset through the project boot stub (contract 1)
 * and driven through the public API only: the seven controller events of the scenario
 * data/scenarios/diag-ctrl-psg-pass.ivss, an audio buffer and a write-log buffer sized from ivs_info.
 * The expected values are not in this file: diag_ctrl_psg_pass_expect.inc is generated at build time
 * from the assert lines of data/golden/diag-ctrl-psg-pass.expect (tests/expect-inc.cmake), where they
 * were derived by hand (docs/input/diag-ctrl-psg-1-derivation.md, plan 05-01) before the program
 * existed; no expected value is read from a core output. The three structural facts that the
 * derivation states as properties (channel B constant 8, channel C and channel A silent) are
 * hand-derived constants below. Machine time is never asserted (V-07), except the 20,000-tick cap
 * of the plan and the 65,536-tick horizon of the tone-A reload, which are bounds.
 *   run.diag.ctrl-psg.ram-tables              the 17 RAM clauses (marker, port 0 table, port 1 table)
 *   run.diag.ctrl-psg.psg-registers           the 11 PSG register read-backs, masked
 *   run.diag.ctrl-psg.log-list                eleven records: addr, value, source from the file; ticks
 *                                             strictly increasing; tick = unit start + 6 CPU cycles
 *                                             (a direct MVO writes at cycle 6) against observed unit starts
 *   run.diag.ctrl-psg.channel-b-constant-8    after the amplitude-B write every sample has level_B 8, before
 *                                             it 0, level_C always 0
 *   run.diag.ctrl-psg.tone-a-silent-until-reload  level_A is 0 for the whole run (C12, C77)
 *   run.diag.ctrl-psg.halts                   stop CPU_HALTED, run state HALTED, below the 20,000-tick cap
 *   run.diag.ctrl-psg.marker-last             the marker word becomes non-zero in the last stores: the tables
 *                                             and the write log do not change afterwards
 *   run.diag.ctrl-psg.slicing                 one call versus slices of 1 and 7 ticks: guest RAM, log bytes,
 *                                             sample bytes and every digest equal
 *   run.diag.ctrl-psg.nokey-timeout           no input: the guest is still in its poll loop at a 12,000-tick
 *                                             target, marker unwritten
 *   run.diag.ctrl-psg.order-equals-pass       every event split into two lines one tick apart (port 1 first)
 *                                             gives the same guest state, log, samples and digests but the
 *                                             input digest */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_test.h"
#include "run_cases.h"

#include "ivs_images.inc"

#define DC_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      goto done;                                                                                       \
    }                                                                                                  \
  } while (0)

#define DC_POISON 0x3Cu
#define DC_ZERO_GROM_BYTES 2048u
#define DC_PASS_TARGET 24000u    /* the run-to bound of the scenario */
#define DC_NOKEY_TARGET 12000u   /* the run-to bound of diag-ctrl-psg-nokey */
#define DC_TICK_CAP 20000u       /* plan cap: reset to halt */
#define DC_RELOAD_HORIZON 65536u /* one empty-period tone half-cycle: 4096 steps x 16 ticks */
#define DC_MAX_UNITS 4096u
#define DC_AUD_MAX 16384u
#define DC_LOG_MAX 4096u
#define DC_RAM_BASE 0x0200u
#define DC_RAM_WORDS 0x0160u /* 0200-035F, the system RAM of the profile */
#define DC_MARKER 0x020Fu
#define DC_TABLE_FIRST 0x0210u
#define DC_TABLE_WORDS 24u   /* 0210-0227: both tables */
#define DC_PSG_BASE 0x01F0u
#define DC_PSG_REGS 16u
#define DC_DOMAINS 9u
#define DC_MVO_WRITE_CYCLES 6u /* a direct MVO writes at CPU cycle 6 of its 11-cycle unit (data/cp1610 vectors) */

/* ---- the predicate, from data/golden/diag-ctrl-psg-pass.expect ---- */
#define DC_HALTED 0u
#define DC_MEM 1u
#define DC_PSGCOUNT 2u
#define DC_PSGLOG 3u

typedef struct dc_clause {
  uint32_t kind;
  uint16_t where; /* memory address, record index, or the count */
  uint16_t value; /* expected word */
  uint16_t addr;  /* PSG register number of a record */
  uint16_t src;   /* record source code */
} dc_clause;

static const dc_clause dc_expect[] = {
#define IVS_EXPECT_HALTED(y) {DC_HALTED, 0u, (uint16_t)(y), 0u, 0u},
#define IVS_EXPECT_MEM(a, v) {DC_MEM, (uint16_t)(a), (uint16_t)(v), 0u, 0u},
#define IVS_EXPECT_PSGCOUNT(n) {DC_PSGCOUNT, (uint16_t)(n), 0u, 0u, 0u},
#define IVS_EXPECT_PSGLOG(i, a, v, s) {DC_PSGLOG, (uint16_t)(i), (uint16_t)(v), (uint16_t)(a), (uint16_t)(s)},
#include "diag_ctrl_psg_pass_expect.inc"
#undef IVS_EXPECT_HALTED
#undef IVS_EXPECT_MEM
#undef IVS_EXPECT_PSGCOUNT
#undef IVS_EXPECT_PSGLOG
};
#define DC_N_EXPECT (sizeof dc_expect / sizeof dc_expect[0])

/* ---- the input schedule of the scenario (data/scenarios/diag-ctrl-psg-pass.ivss) ---- */
#define DC_N_EV 7u
static const ivs_input_event dc_pass_ev[DC_N_EV] = {
    {4000u, 0u, 0u, {IVS_CTRL_KEY_1, 0u, 0u, 0u}},
    {4800u, 0u, 0u, {0u, IVS_CTRL_DISC_E, 0u, 0u}},
    {5600u, 0u, 0u, {IVS_CTRL_KEY_1 | IVS_CTRL_KEY_9, IVS_CTRL_SIDE_LOWER_LEFT, 0u, 0u}},
    {6400u, 0u, 0u, {0u, 0u, 0u, 0u}},
    {7200u, 0u, 0u, {IVS_CTRL_DISC_N, 0u, 0u, 0u}},
    {8000u, 0u, 0u, {IVS_CTRL_DISC_N | IVS_CTRL_SIDE_TOP, 0u, 0u, 0u}},
    {8800u, 0u, 0u, {IVS_CTRL_KEY_5 | IVS_CTRL_SIDE_TOP, 0u, 0u, 0u}},
};

/* One run's observations. */
typedef struct dc_obs {
  uint64_t starts[DC_MAX_UNITS]; /* unit start ticks (only with per-unit stepping) */
  uint32_t n_starts;
  uint8_t aud[DC_AUD_MAX];
  uint64_t aud_n;
  uint8_t log[DC_LOG_MAX];
  uint64_t log_n;
  uint16_t ram[DC_RAM_WORDS];
  uint16_t psg[DC_PSG_REGS];
  uint8_t dig[DC_DOMAINS][32];
  uint64_t now;
  uint32_t stop;
  uint32_t run_state;
  uint16_t r7;
  uint32_t ticks_per_cpu_cycle;
  uint32_t sample_ticks;
  int marker_seen;                       /* the marker became non-zero during stepping */
  uint16_t marker_tables[DC_TABLE_WORDS]; /* the tables when the marker first became non-zero */
  uint64_t marker_log_n;                  /* the log bytes at that moment */
} dc_obs;

typedef struct dc_plan {
  const ivs_input_event *ev;
  uint32_t n_ev;
  uint64_t target;
  uint64_t slice; /* 0: one call to the target */
  int track;      /* per-unit stepping bookkeeping (slice must be 1) */
} dc_plan;

static const uint32_t dc_domains[DC_DOMAINS] = {IVS_DIGEST_COMPOSITE, IVS_DIGEST_CPU,   IVS_DIGEST_MEM,
                                                IVS_DIGEST_IMAGES,    IVS_DIGEST_INSTANCE, IVS_DIGEST_VIDEO,
                                                IVS_DIGEST_AUDIO,     IVS_DIGEST_INPUT,    IVS_DIGEST_PSGLOG};
#define DC_D_CPU 1u
#define DC_D_MEM 2u
#define DC_D_IMAGES 3u
#define DC_D_AUDIO 6u
#define DC_D_INPUT 7u
#define DC_D_PSGLOG 8u

/* ---- helpers ---- */

/* A heap copy of n bytes at src (ivs_ms_load frees it), or NULL. */
static uint8_t *dc_copy(const uint8_t *src, size_t n)
{
  uint8_t *b = malloc(n);
  if (b != NULL) {
    memcpy(b, src, n);
  }
  return b;
}

/* A powered instance: stub EXEC, zero GROM, the diagnostic cartridge, a power-on reset. 1 on success. */
static int dc_powered(ivs_ms_inst *x)
{
  ivs_status st;
  if (!ivs_ms_new(x, DC_POISON)) {
    return 0;
  }
  if (!ivs_ms_load(x->m, IVS_KIND_EXEC, dc_copy(ivs_img_boot_stub, IVS_IMG_BOOT_STUB_BYTES), IVS_IMG_BOOT_STUB_BYTES) ||
      !ivs_ms_load(x->m, IVS_KIND_GROM, calloc(DC_ZERO_GROM_BYTES, 1u), DC_ZERO_GROM_BYTES) ||
      !ivs_ms_load(x->m, IVS_KIND_CARTRIDGE, dc_copy(ivs_img_diag_ctrl_psg_1, IVS_IMG_DIAG_CTRL_PSG_1_BYTES),
                   IVS_IMG_DIAG_CTRL_PSG_1_BYTES)) {
    ivs_ms_free(x);
    return 0;
  }
  st = ivs_machine_reset(x->m, IVS_RESET_POWER_ON);
  if (st != IVS_OK) {
    ivs_test_diag("reset refused (%s)", ivs_status_name(st));
    ivs_ms_free(x);
    return 0;
  }
  return 1;
}

/* Runs the plan on a fresh instance and fills *o. The audio and log buffers of every call are one
 * frame slot sized from ivs_info (the targets are below one nominal frame, so one slot satisfies the
 * capacity rule of every call); the delivered bytes of consecutive calls are appended. 1 on success. */
static int dc_run(const dc_plan *p, dc_obs *o)
{
  ivs_ms_inst x;
  ivs_info info;
  ivs_cpu_state_report c;
  uint8_t *slot_aud = NULL;
  uint8_t *slot_log = NULL;
  uint64_t aud_cap;
  uint64_t log_cap;
  uint32_t consumed = 0u;
  uint32_t calls = 0u;
  uint32_t i;
  int rc = 0;
  memset(o, 0, sizeof *o);
  if (!dc_powered(&x)) {
    return 0;
  }
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  DC_CHECK(ivs_machine_info(x.m, &info) == IVS_OK);
  DC_CHECK(info.ticks_per_cpu_cycle != 0u && info.nominal_frame_ticks > p->target);
  DC_CHECK(info.audio_format == IVS_OUTPUT_FORMAT_PSG_LEVELS_V1 && info.audio_sample_bytes == 4u);
  DC_CHECK(info.psg_log_record_bytes == 16u && info.audio_sample_ticks != 0u);
  o->ticks_per_cpu_cycle = info.ticks_per_cpu_cycle;
  o->sample_ticks = info.audio_sample_ticks;
  aud_cap = info.audio_bytes_per_frame;
  log_cap = (uint64_t)info.psg_log_record_bytes * info.psg_log_records_per_frame;
  slot_aud = malloc((size_t)aud_cap);
  slot_log = malloc((size_t)log_cap);
  DC_CHECK(slot_aud != NULL && slot_log != NULL);
  for (;;) {
    ivs_run_request q;
    ivs_run_report rep;
    uint64_t t = p->target;
    uint64_t before;
    ivs_info now_info;
    memset(&now_info, 0, sizeof now_info);
    now_info.struct_size = (uint32_t)sizeof now_info;
    DC_CHECK(ivs_machine_info(x.m, &now_info) == IVS_OK);
    before = now_info.time_ticks;
    if (before >= p->target) {
      break;
    }
    /* a faulted core that stops short of every target would loop forever: one call per unit at most */
    calls++;
    DC_CHECK(calls <= DC_MAX_UNITS);
    if (p->slice != 0u && before + p->slice < t) {
      t = before + p->slice;
    }
    memset(&q, 0, sizeof q);
    q.struct_size = (uint32_t)sizeof q;
    q.flags = IVS_RUN_F_STOP_ON_HALT;
    q.target_ticks = t;
    q.audio = slot_aud;
    q.audio_capacity = aud_cap;
    q.psg_log = slot_log;
    q.psg_log_capacity = log_cap;
    q.events = p->n_ev - consumed != 0u ? p->ev + consumed : NULL;
    q.event_count = p->n_ev - consumed;
    memset(&rep, 0, sizeof rep);
    rep.struct_size = (uint32_t)sizeof rep;
    DC_CHECK(ivs_machine_run(x.m, &q, &rep) == IVS_OK);
    consumed += rep.input_consumed;
    DC_CHECK(o->aud_n + rep.audio_bytes_written <= DC_AUD_MAX);
    DC_CHECK(o->log_n + rep.psg_log_bytes_written <= DC_LOG_MAX);
    memcpy(o->aud + o->aud_n, slot_aud, (size_t)rep.audio_bytes_written);
    memcpy(o->log + o->log_n, slot_log, (size_t)rep.psg_log_bytes_written);
    o->aud_n += rep.audio_bytes_written;
    o->log_n += rep.psg_log_bytes_written;
    DC_CHECK(rep.now_ticks > before || rep.stop_reason == IVS_STOP_CPU_HALTED);
    o->now = rep.now_ticks;
    o->stop = rep.stop_reason;
    if (p->track) {
      uint16_t marker = 0u;
      DC_CHECK(o->n_starts < DC_MAX_UNITS);
      o->starts[o->n_starts++] = before;
      DC_CHECK(ivs_machine_peek(x.m, (uint16_t)DC_MARKER, 1u, &marker) == IVS_OK);
      if (marker != 0u && !o->marker_seen) {
        o->marker_seen = 1;
        DC_CHECK(ivs_machine_peek(x.m, (uint16_t)DC_TABLE_FIRST, DC_TABLE_WORDS, o->marker_tables) == IVS_OK);
        o->marker_log_n = o->log_n;
      }
    }
    if (rep.stop_reason == IVS_STOP_CPU_HALTED) {
      break;
    }
    DC_CHECK(rep.stop_reason == IVS_STOP_TARGET);
  }
  DC_CHECK(ivs_machine_peek(x.m, (uint16_t)DC_RAM_BASE, DC_RAM_WORDS, o->ram) == IVS_OK);
  DC_CHECK(ivs_machine_peek(x.m, (uint16_t)DC_PSG_BASE, DC_PSG_REGS, o->psg) == IVS_OK);
  memset(&c, 0, sizeof c);
  c.struct_size = (uint32_t)sizeof c;
  DC_CHECK(ivs_machine_cpu_state(x.m, &c) == IVS_OK);
  o->run_state = c.run_state;
  o->r7 = c.r[7];
  for (i = 0u; i < DC_DOMAINS; i++) {
    DC_CHECK(ivs_machine_state_digest(x.m, dc_domains[i], o->dig[i]) == IVS_OK);
  }
  rc = 1;
done:
  free(slot_aud);
  free(slot_log);
  ivs_ms_free(&x);
  return rc;
}

/* The canonical run: the scenario's schedule stepped one unit per call, kept for the cases that read
 * one run. 1 when it was made (and is valid). */
static dc_obs dc_canon;
static int dc_canon_state; /* 0 not made, 1 made, -1 failed */

static const dc_obs *dc_canonical(void)
{
  if (dc_canon_state == 0) {
    dc_plan p;
    p.ev = dc_pass_ev;
    p.n_ev = DC_N_EV;
    p.target = DC_PASS_TARGET;
    p.slice = 1u;
    p.track = 1;
    dc_canon_state = dc_run(&p, &dc_canon) ? 1 : -1;
  }
  return dc_canon_state == 1 ? &dc_canon : NULL;
}

/* A fresh one-shot run of the pass schedule with the given slicing. */
static int dc_pass_run(uint64_t slice, dc_obs *o)
{
  dc_plan p;
  p.ev = dc_pass_ev;
  p.n_ev = DC_N_EV;
  p.target = DC_PASS_TARGET;
  p.slice = slice;
  p.track = 0;
  return dc_run(&p, o);
}

static uint64_t dc_be64(const uint8_t *b)
{
  uint64_t v = 0u;
  int i;
  for (i = 0; i < 8; i++) {
    v = (v << 8) | b[i];
  }
  return v;
}

static const dc_clause *dc_record_clause(uint16_t reg)
{
  size_t i;
  for (i = 0u; i < DC_N_EXPECT; i++) {
    if (dc_expect[i].kind == DC_PSGLOG && dc_expect[i].addr == reg) {
      return &dc_expect[i];
    }
  }
  return NULL;
}

/* The tick of the record of register reg in the log of o (the first one), or 0 when none. */
static uint64_t dc_record_tick(const dc_obs *o, const dc_clause *e)
{
  return dc_be64(o->log + (size_t)e->where * 16u);
}

/* 1 when two runs agree on guest RAM, PSG registers, sample bytes, log bytes and the CPU, MEM, IMAGES, VIDEO, AUDIO
 * and PSGLOG digests (the composite, the instance and the input digest also fold the input history and are compared by
 * the callers that need them). */
static int dc_same(const dc_obs *a, const dc_obs *b, const char *what)
{
  size_t i;
  if (memcmp(a->ram, b->ram, sizeof a->ram) != 0 || memcmp(a->psg, b->psg, sizeof a->psg) != 0) {
    ivs_test_diag("%s: guest RAM or PSG registers differ", what);
    return 0;
  }
  if (a->aud_n != b->aud_n || memcmp(a->aud, b->aud, (size_t)a->aud_n) != 0) {
    ivs_test_diag("%s: sample bytes differ (%llu versus %llu bytes)", what, (unsigned long long)a->aud_n,
                  (unsigned long long)b->aud_n);
    return 0;
  }
  if (a->log_n != b->log_n || memcmp(a->log, b->log, (size_t)a->log_n) != 0) {
    ivs_test_diag("%s: log bytes differ", what);
    return 0;
  }
  for (i = 0u; i < DC_DOMAINS; i++) {
    if (i == DC_D_INPUT || i == 0u || i == 4u) {
      continue;
    }
    if (memcmp(a->dig[i], b->dig[i], 32u) != 0) {
      ivs_test_diag("%s: digest of domain %u differs", what, (unsigned)dc_domains[i]);
      return 0;
    }
  }
  return 1;
}

/* ---- cases ---- */

static int dc_ram_tables(void)
{
  const dc_obs *o = dc_canonical();
  size_t i;
  uint32_t n = 0u;
  if (o == NULL) {
    return IVS_TEST_FAIL;
  }
  for (i = 0u; i < DC_N_EXPECT; i++) {
    const dc_clause *e = &dc_expect[i];
    if (e->kind == DC_MEM && e->where >= DC_RAM_BASE && e->where < DC_RAM_BASE + DC_RAM_WORDS) {
      uint16_t have = o->ram[e->where - DC_RAM_BASE];
      n++;
      if (have != e->value) {
        ivs_test_diag("mem %04X: expected %04X, actual %04X", (unsigned)e->where, (unsigned)e->value,
                      (unsigned)have);
        return IVS_TEST_FAIL;
      }
    }
  }
  if (n != 17u) {
    ivs_test_diag("the predicate has %u RAM clauses; want 17 (marker, 8 + 8 table words)", (unsigned)n);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

static int dc_psg_registers(void)
{
  const dc_obs *o = dc_canonical();
  size_t i;
  uint32_t n = 0u;
  if (o == NULL) {
    return IVS_TEST_FAIL;
  }
  for (i = 0u; i < DC_N_EXPECT; i++) {
    const dc_clause *e = &dc_expect[i];
    if (e->kind == DC_MEM && e->where >= DC_PSG_BASE && e->where < DC_PSG_BASE + DC_PSG_REGS) {
      uint16_t have = o->psg[e->where - DC_PSG_BASE];
      n++;
      if (have != e->value) {
        ivs_test_diag("psg %04X: expected %04X, actual %04X", (unsigned)e->where, (unsigned)e->value,
                      (unsigned)have);
        return IVS_TEST_FAIL;
      }
    }
  }
  if (n != 11u) {
    ivs_test_diag("the predicate has %u PSG register clauses; want 11", (unsigned)n);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

static int dc_log_list(void)
{
  const dc_obs *o = dc_canonical();
  size_t i;
  uint32_t n = 0u;
  uint64_t prev = 0u;
  uint32_t want_count = 0xFFFFFFFFu;
  int rc = IVS_TEST_FAIL;
  if (o == NULL) {
    return IVS_TEST_FAIL;
  }
  for (i = 0u; i < DC_N_EXPECT; i++) {
    if (dc_expect[i].kind == DC_PSGCOUNT) {
      want_count = dc_expect[i].where;
    }
  }
  DC_CHECK(want_count == 11u && o->log_n == (uint64_t)want_count * 16u);
  for (i = 0u; i < DC_N_EXPECT; i++) {
    const dc_clause *e = &dc_expect[i];
    const uint8_t *rec;
    uint64_t tick;
    uint32_t k;
    int found = 0;
    if (e->kind != DC_PSGLOG) {
      continue;
    }
    n++;
    DC_CHECK(e->where < want_count);
    rec = o->log + (size_t)e->where * 16u;
    tick = dc_be64(rec);
    DC_CHECK((uint16_t)(((uint16_t)rec[8] << 8) | rec[9]) == e->value);
    DC_CHECK(rec[10] == e->addr && rec[11] == e->src);
    DC_CHECK(rec[12] == 0u && rec[13] == 0u && rec[14] == 0u && rec[15] == 0u);
    DC_CHECK(e->where == 0u || tick > prev);
    prev = tick;
    for (k = 0u; k < o->n_starts; k++) {
      if (o->starts[k] + (uint64_t)DC_MVO_WRITE_CYCLES * o->ticks_per_cpu_cycle == tick) {
        found = 1;
        break;
      }
    }
    if (!found) {
      ivs_test_diag("record %u tick %llu is no observed unit start + 6 cycles", (unsigned)e->where,
                    (unsigned long long)tick);
      goto done;
    }
  }
  DC_CHECK(n == 11u);
  rc = IVS_TEST_PASS;
done:
  return rc;
}

/* Samples: sample i (0-based) is stamped (i + 1) x sample_ticks and sees a write stamped w when w is below
 * its stamp (psg.log.effect-boundary). */
static int dc_channel_b(void)
{
  const dc_obs *o = dc_canonical();
  const dc_clause *amp;
  uint64_t w;
  uint64_t i;
  uint8_t level;
  int rc = IVS_TEST_FAIL;
  if (o == NULL) {
    return IVS_TEST_FAIL;
  }
  amp = dc_record_clause(12u);
  DC_CHECK(amp != NULL && o->aud_n % 4u == 0u && o->aud_n >= 4u * 100u);
  /* the amplitude-B value is fixed-level mode (bit 4 clear), so its level is the value; the mixer
   * disables both tone B and noise B (bits 1 and 4 of 003E), so nothing modulates it */
  DC_CHECK((amp->value & 0x10u) == 0u);
  level = (uint8_t)(amp->value & 0x0Fu);
  DC_CHECK(level == 8u);
  w = dc_record_tick(o, amp);
  for (i = 0u; i < o->aud_n / 4u; i++) {
    uint64_t stamp = (i + 1u) * o->sample_ticks;
    uint8_t want_b = stamp > w ? level : (uint8_t)0u;
    const uint8_t *s = o->aud + 4u * i;
    if (s[1] != want_b || s[2] != 0u || s[3] != 0u) {
      ivs_test_diag("sample %llu (stamp %llu): B %u (want %u), C %u, pad %u", (unsigned long long)i,
                    (unsigned long long)stamp, (unsigned)s[1], (unsigned)want_b, (unsigned)s[2], (unsigned)s[3]);
      goto done;
    }
  }
  rc = IVS_TEST_PASS;
done:
  return rc;
}

static int dc_tone_a_silent(void)
{
  const dc_obs *o = dc_canonical();
  const dc_clause *mix;
  const dc_clause *amp_a;
  uint64_t i;
  int rc = IVS_TEST_FAIL;
  if (o == NULL) {
    return IVS_TEST_FAIL;
  }
  mix = dc_record_clause(8u);
  amp_a = dc_record_clause(11u);
  DC_CHECK(mix != NULL && amp_a != NULL);
  /* the silence is not an artefact of a disabled or muted channel: tone A is enabled in the mixer
   * (bit 0 clear) with a non-zero fixed amplitude, and its period was programmed non-zero */
  DC_CHECK((mix->value & 0x01u) == 0u && (amp_a->value & 0x1Fu) != 0u);
  /* the first reload happened at the first step, with period 0 (4096 steps = 65,536 ticks); the
   * period written later takes effect only at the next reload, beyond the end of this run */
  DC_CHECK(o->now < DC_RELOAD_HORIZON);
  DC_CHECK(o->aud_n >= 4u * 100u);
  for (i = 0u; i < o->aud_n / 4u; i++) {
    if (o->aud[4u * i] != 0u) {
      ivs_test_diag("sample %llu: level A is %u, expected 0 for the whole run", (unsigned long long)i,
                    (unsigned)o->aud[4u * i]);
      goto done;
    }
  }
  rc = IVS_TEST_PASS;
done:
  return rc;
}

static int dc_halts(void)
{
  const dc_obs *o = dc_canonical();
  size_t i;
  uint32_t halted_clauses = 0u;
  if (o == NULL) {
    return IVS_TEST_FAIL;
  }
  for (i = 0u; i < DC_N_EXPECT; i++) {
    if (dc_expect[i].kind == DC_HALTED) {
      halted_clauses++;
      if (dc_expect[i].value != 1u) {
        ivs_test_diag("the predicate does not expect a halt");
        return IVS_TEST_FAIL;
      }
    }
  }
  if (halted_clauses != 1u || o->stop != IVS_STOP_CPU_HALTED || o->run_state != IVS_CPU_STATE_HALTED ||
      o->now > DC_TICK_CAP) {
    ivs_test_diag("stop %s, run_state %u, now %llu (cap %u)", ivs_run_stop_name(o->stop), (unsigned)o->run_state,
                  (unsigned long long)o->now, (unsigned)DC_TICK_CAP);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

static int dc_marker_last(void)
{
  const dc_obs *o = dc_canonical();
  int rc = IVS_TEST_FAIL;
  if (o == NULL) {
    return IVS_TEST_FAIL;
  }
  DC_CHECK(o->marker_seen);
  /* when the marker first showed, both tables and the whole write log were already final */
  DC_CHECK(memcmp(o->marker_tables, o->ram + (DC_TABLE_FIRST - DC_RAM_BASE), sizeof o->marker_tables) == 0);
  DC_CHECK(o->marker_log_n == o->log_n);
  DC_CHECK(o->ram[DC_MARKER - DC_RAM_BASE] != 0u);
  rc = IVS_TEST_PASS;
done:
  return rc;
}

static int dc_slicing(void)
{
  static dc_obs one;
  static dc_obs s1;
  static dc_obs s7;
  const dc_obs *canon = dc_canonical();
  size_t i;
  if (canon == NULL || !dc_pass_run(0u, &one) || !dc_pass_run(7u, &s7) || !dc_pass_run(1u, &s1)) {
    return IVS_TEST_FAIL;
  }
  if (!dc_same(&one, &s1, "one call versus 1-tick slices") || !dc_same(&one, &s7, "one call versus 7-tick slices") ||
      !dc_same(&one, canon, "one call versus the tracked run")) {
    return IVS_TEST_FAIL;
  }
  /* composite, instance and input fold the whole state including the input chain: equal across slicings */
  for (i = 0u; i < DC_DOMAINS; i++) {
    if (memcmp(one.dig[i], s1.dig[i], 32u) != 0 || memcmp(one.dig[i], s7.dig[i], 32u) != 0) {
      ivs_test_diag("digest of domain %u differs across slicings", (unsigned)dc_domains[i]);
      return IVS_TEST_FAIL;
    }
  }
  if (one.now != s1.now || one.now != s7.now || one.stop != IVS_STOP_CPU_HALTED) {
    ivs_test_diag("end time or stop differs across slicings");
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

static int dc_nokey_timeout(void)
{
  static dc_obs o;
  dc_plan p;
  int rc = IVS_TEST_FAIL;
  p.ev = NULL;
  p.n_ev = 0u;
  p.target = DC_NOKEY_TARGET;
  p.slice = 0u;
  p.track = 0;
  if (!dc_run(&p, &o)) {
    return IVS_TEST_FAIL;
  }
  /* idle ports read FF: each port stores its first value and no change follows, so port 0 never
   * reaches eight values and the guest is still in the poll loop (5036-504B) when the target is
   * reached; the end marker is unwritten and the machine is not halted */
  DC_CHECK(o.stop == IVS_STOP_TARGET && o.run_state != IVS_CPU_STATE_HALTED);
  DC_CHECK(o.ram[DC_MARKER - DC_RAM_BASE] == 0u);
  DC_CHECK(o.ram[0x0210u - DC_RAM_BASE] == 0x00FFu && o.ram[0x0211u - DC_RAM_BASE] == 0u);
  DC_CHECK(o.ram[0x0220u - DC_RAM_BASE] == 0x00FFu && o.ram[0x0221u - DC_RAM_BASE] == 0u);
  DC_CHECK(o.r7 >= 0x5036u && o.r7 <= 0x504Bu);
  DC_CHECK(o.log_n == 11u * 16u); /* the programming writes happen without any input */
  rc = IVS_TEST_PASS;
done:
  return rc;
}

static int dc_order_equals_pass(void)
{
  static dc_obs pass;
  static dc_obs ord;
  ivs_input_event split[2u * DC_N_EV];
  dc_plan p;
  uint32_t k;
  uint32_t prev0 = 0u;
  for (k = 0u; k < DC_N_EV; k++) {
    /* the first line, at tick T, changes only the port-1 word and repeats the previous port-0 word; the
     * second line, one tick later, sets the port-0 word */
    split[2u * k] = dc_pass_ev[k];
    split[2u * k].controller[0] = prev0;
    split[2u * k + 1u] = dc_pass_ev[k];
    split[2u * k + 1u].at_tick = dc_pass_ev[k].at_tick + 1u;
    prev0 = dc_pass_ev[k].controller[0];
  }
  p.ev = split;
  p.n_ev = 2u * DC_N_EV;
  p.target = DC_PASS_TARGET;
  p.slice = 0u;
  p.track = 0;
  if (!dc_pass_run(0u, &pass) || !dc_run(&p, &ord)) {
    return IVS_TEST_FAIL;
  }
  if (!dc_same(&pass, &ord, "pass versus order")) {
    return IVS_TEST_FAIL;
  }
  /* the schedules differ (twice the events), so the input chain differs: the order variant is a different
   * input history with the same guest-visible result */
  if (memcmp(pass.dig[DC_D_INPUT], ord.dig[DC_D_INPUT], 32u) == 0) {
    ivs_test_diag("the input digest is equal: the split schedule was not applied");
    return IVS_TEST_FAIL;
  }
  if (pass.now != ord.now || ord.stop != IVS_STOP_CPU_HALTED) {
    ivs_test_diag("end time or stop differs between pass and order");
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_run_diag_ctrl_cases[] = {
    {"run.diag.ctrl-psg.ram-tables", dc_ram_tables, NULL},
    {"run.diag.ctrl-psg.psg-registers", dc_psg_registers, NULL},
    {"run.diag.ctrl-psg.log-list", dc_log_list, NULL},
    {"run.diag.ctrl-psg.channel-b-constant-8", dc_channel_b, NULL},
    {"run.diag.ctrl-psg.tone-a-silent-until-reload", dc_tone_a_silent, NULL},
    {"run.diag.ctrl-psg.halts", dc_halts, NULL},
    {"run.diag.ctrl-psg.marker-last", dc_marker_last, NULL},
    {"run.diag.ctrl-psg.slicing", dc_slicing, NULL},
    {"run.diag.ctrl-psg.nokey-timeout", dc_nokey_timeout, NULL},
    {"run.diag.ctrl-psg.order-equals-pass", dc_order_equals_pass, NULL},
};
const size_t ivs_run_diag_ctrl_count = sizeof ivs_run_diag_ctrl_cases / sizeof ivs_run_diag_ctrl_cases[0];
