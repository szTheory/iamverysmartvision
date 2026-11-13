/* SPDX-License-Identifier: MIT */
/* Suite stic, family irq (plan 06-08; D-08, A-09): the VBLANK interrupt request as the CPU stepper accepts it. INTRM is a level from
 * each frame start; the stepper accepts it only at an interruptible boundary with INTFF set (EIS), and with C41 = 0 acceptance does
 * not clear INTFF, so every service routine of these stand-ins starts with DIS. The stand-ins are the programs of stic_cases.h.
 * Expectations are relations to the parameters (an entry starts at or after the frame start plus the sampling lag and its sample is
 * inside the INTRM level) and to the guest's own counter (R2), never a policy number (D-02).
 *   stic.irq.one-per-frame         EIS and a DIS-first counting ISR: one interrupt entry per frame start crossed, each inside its level
 *   stic.irq.open-boundary-only    a stream of non-interruptible stores is never interrupted; the first entry is the boundary after it
 *   stic.irq.dis-blocks            the same program without EIS has no entry in two frames
 *   stic.irq.eis-late-misses       an EIS whose next open boundary is after the level ends misses the frame, and the next frame is taken
 *   stic.irq.level-held-reentry-c41-0  C41 = 0 and a service routine without DIS: re-entered at every open boundary while the level is held;
 *                                  the entry count of one frame is the closed form from the entry and instruction cycle counts
 *   stic.irq.dis-first-isr-one-entry   a service routine that starts with DIS and never re-enables: exactly one entry in three frames
 *   stic.irq.c41-both-values       the stepper-level family: both values of cpu.interrupt_accept_clears_intff crossed with a DIS-first and
 *                                  a not DIS-first service routine, driven by ivs_stic_lines (a test-built configuration copy) */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "machine/machine.h"
#include "machine_cases.h"
#include "stic_cases.h"

#define SI_CAP 40000u

static int si_one_per_frame(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  const struct ivs_machine *im;
  ivs_ms_inst x;
  sc_unit *u = malloc(SI_CAP * sizeof *u);
  uint16_t w[SC_PROG_MAX];
  size_t n;
  size_t k;
  uint32_t per_frame[4] = {0u, 0u, 0u, 0u};
  uint32_t entries = 0u;
  int ok = 0;
  memset(&x, 0, sizeof x);
  if (u == NULL || !sc_guest(&x, w, sc_prog_vbl(w, SC_BODY_INCR), 0u)) {
    free(u);
    return IVS_TEST_FAIL;
  }
  im = ivs_machine_from_handle(x.m);
  n = sc_trace(&x, 4u * (uint64_t)p->frame_ticks, u, SI_CAP);
  if (n == (size_t)-1) {
    ivs_test_diag("trace refused or overflowed");
    goto done;
  }
  for (k = 0u; k < n; k++) {
    uint64_t frame;
    uint64_t sk;
    if (u[k].kind != IVS_CP1610_UNIT_INT_ENTRY) {
      continue;
    }
    frame = (u[k].start - p->frame_phase_ticks) / p->frame_ticks;
    sk = (uint64_t)p->frame_phase_ticks + frame * (uint64_t)p->frame_ticks;
    if (frame >= 4u || u[k].start < sk + p->line_sample_lag_ticks ||
        u[k].start - p->line_sample_lag_ticks >= sk + p->irq_low_ticks) {
      ivs_test_diag("entry at %llu is outside the level of frame %llu", (unsigned long long)u[k].start, (unsigned long long)frame);
      goto done;
    }
    per_frame[frame]++;
    entries++;
  }
  for (k = 0u; k < 4u; k++) {
    if (per_frame[k] != 1u) {
      ivs_test_diag("frame %u has %u interrupt entries (one expected)", (unsigned)k, (unsigned)per_frame[k]);
      goto done;
    }
  }
  if (im->cpu.r[2] != entries) { /* the guest's own counter agrees: INCR R2 once per entry */
    ivs_test_diag("the guest counted %u entries, the trace shows %u", (unsigned)im->cpu.r[2], (unsigned)entries);
    goto done;
  }
  ok = 1;
done:
  free(u);
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* A stream of count MVO stores after an EIS, then B self: runs the trace to until and fills the entry indexes. */
static int si_stream(ivs_ms_inst *x, sc_unit *u, size_t count, uint64_t until, size_t *n)
{
  uint16_t *w = malloc(4096u * sizeof *w);
  int ok = 0;
  if (w == NULL) {
    return 0;
  }
  if (sc_guest_n(x, w, sc_prog_stream(w, count))) {
    *n = sc_trace(x, until, u, SI_CAP);
    ok = *n != (size_t)-1;
    if (!ok) {
      ivs_ms_free(x);
    }
  }
  free(w);
  return ok;
}

static int si_open_boundary_only(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  ivs_ms_inst x;
  sc_unit *u = malloc(SI_CAP * sizeof *u);
  size_t n = 0u;
  size_t k;
  size_t first_b = (size_t)-1;
  size_t first_entry = (size_t)-1;
  int ok = 0;
  memset(&x, 0, sizeof x);
  /* 100 stores of 11 cycles end well inside the level (3,791 cycles): the boundary after the last store is the first open one */
  if (u == NULL || !si_stream(&x, u, 100u, 2u * (uint64_t)p->frame_ticks, &n)) {
    free(u);
    return IVS_TEST_FAIL;
  }
  for (k = 0u; k < n; k++) {
    if (u[k].kind == IVS_CP1610_UNIT_INT_ENTRY) {
      first_entry = k;
      break;
    }
    if (first_b == (size_t)-1 && u[k].kind == IVS_CP1610_UNIT_INSTR && (u[k].opcode & 0x3DFu) == 0x200u) {
      first_b = k;
    }
  }
  if (first_entry == (size_t)-1 || first_b == (size_t)-1 || first_entry != first_b + 1u) {
    ivs_test_diag("first branch unit %lld, first entry unit %lld (the entry must be the unit right after the first branch)",
                  (long long)(first_b == (size_t)-1 ? -1 : (long long)first_b),
                  (long long)(first_entry == (size_t)-1 ? -1 : (long long)first_entry));
    goto done;
  }
  for (k = 0u; k < first_b; k++) { /* every unit before the first branch is the EIS or a store: never an entry, never a grant */
    if (u[k].kind != IVS_CP1610_UNIT_INSTR && u[k].kind != IVS_CP1610_UNIT_RESET_ENTRY) {
      ivs_test_diag("unit %lu before the stream end has kind %u", (unsigned long)k, (unsigned)u[k].kind);
      goto done;
    }
  }
  ok = 1;
done:
  free(u);
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int si_dis_blocks(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  ivs_ms_inst x;
  sc_unit *u = malloc(SI_CAP * sizeof *u);
  uint16_t w[SC_PROG_MAX];
  size_t n;
  size_t k;
  size_t words;
  uint32_t entries = 0u;
  int ok = 0;
  memset(&x, 0, sizeof x);
  words = sc_prog_vbl(w, SC_BODY_INCR);
  w[17] = 0x0003u; /* the main EIS (word 17, $1011) becomes DIS: INTFF is never set, the ISR is never reached */
  if (u == NULL || !sc_guest(&x, w, words, 0u)) {
    free(u);
    return IVS_TEST_FAIL;
  }
  n = sc_trace(&x, 2u * (uint64_t)p->frame_ticks, u, SI_CAP);
  if (n == (size_t)-1) {
    goto done;
  }
  for (k = 0u; k < n; k++) {
    entries += u[k].kind == IVS_CP1610_UNIT_INT_ENTRY ? 1u : 0u;
  }
  if (entries != 0u || n < 1000u) {
    ivs_test_diag("%u entries in %lu units with INTFF clear", (unsigned)entries, (unsigned long)n);
    goto done;
  }
  ok = 1;
done:
  free(u);
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int si_eis_late_misses(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  ivs_ms_inst x;
  sc_unit *u = malloc(SI_CAP * sizeof *u);
  size_t n = 0u;
  size_t k;
  uint32_t in_frame0 = 0u;
  uint32_t in_frame1 = 0u;
  int ok = 0;
  memset(&x, 0, sizeof x);
  /* 400 stores of 11 cycles outlast the level (3,791 cycles): the first open boundary is after the level ended, frame 0 is missed */
  if (u == NULL || !si_stream(&x, u, 400u, (uint64_t)p->frame_ticks + 2000u, &n)) {
    free(u);
    return IVS_TEST_FAIL;
  }
  for (k = 0u; k < n; k++) {
    if (u[k].kind == IVS_CP1610_UNIT_INT_ENTRY) {
      if (u[k].start < (uint64_t)p->frame_ticks) {
        in_frame0++;
      } else {
        in_frame1++;
      }
    }
  }
  if (in_frame0 != 0u || in_frame1 < 1u) {
    ivs_test_diag("entries in frame 0: %u (0 expected), in frame 1: %u (at least 1 expected)", (unsigned)in_frame0, (unsigned)in_frame1);
    goto done;
  }
  ok = 1;
done:
  free(u);
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* Entries of one frame of the program sc_prog_isr(dis_first) on the machine; *t0 gets the start tick of the first entry. */
static uint32_t si_machine_entries(uint32_t dis_first, uint64_t frames, uint64_t *t0, uint32_t *counter)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  const struct ivs_machine *im;
  ivs_ms_inst x;
  sc_unit *u = malloc(SI_CAP * sizeof *u);
  uint16_t w[SC_PROG_MAX];
  uint32_t entries = 0u;
  size_t n;
  size_t k;
  memset(&x, 0, sizeof x);
  *t0 = 0u;
  if (u == NULL || !sc_guest(&x, w, sc_prog_isr(w, dis_first), 0u)) {
    free(u);
    return 0xFFFFFFFFu;
  }
  im = ivs_machine_from_handle(x.m);
  n = sc_trace(&x, frames * (uint64_t)p->frame_ticks, u, SI_CAP);
  if (n == (size_t)-1) {
    entries = 0xFFFFFFFFu;
  } else {
    for (k = 0u; k < n; k++) {
      if (u[k].kind == IVS_CP1610_UNIT_INT_ENTRY) {
        *t0 = entries == 0u ? u[k].start : *t0;
        entries++;
      }
    }
    *counter = im->cpu.r[2];
  }
  free(u);
  ivs_ms_free(&x);
  return entries;
}

/* The closed form of the entries of a held level (C41 = 0, no DIS): entries start every entry + INCR cycles from the first one while
 * start - lag is below the level length. */
static uint32_t si_reentry_closed_form(const ivs_stic_params *p, uint64_t t0, uint64_t period_ticks)
{
  uint64_t limit = (uint64_t)p->irq_low_ticks + (uint64_t)p->line_sample_lag_ticks;
  return t0 >= limit ? 0u : (uint32_t)((limit - t0 + period_ticks - 1u) / period_ticks);
}

static int si_reentry_c41_0(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint64_t t0 = 0u;
  uint32_t counter = 0u;
  uint32_t entries = si_machine_entries(0u, 1u, &t0, &counter);
  /* an entry is 9 CPU cycles (the stepper's constant) and the first service instruction INCR 6: one period of 15 cycles */
  uint32_t expect = si_reentry_closed_form(p, t0, 15u * (uint64_t)p->ticks_per_cpu_cycle);
  if (entries == 0xFFFFFFFFu || entries < 100u || entries != expect || counter != entries) {
    ivs_test_diag("%u entries in frame 0 (closed form %u from the first entry at %llu), the guest counted %u", (unsigned)entries,
                  (unsigned)expect, (unsigned long long)t0, (unsigned)counter);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

static int si_dis_first_one_entry(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint64_t t0 = 0u;
  uint32_t counter = 0u;
  uint32_t entries = si_machine_entries(1u, 3u, &t0, &counter);
  if (entries != 1u || counter != 1u || t0 >= (uint64_t)p->frame_ticks) {
    ivs_test_diag("%u entries in three frames (one expected), the guest counted %u", (unsigned)entries, (unsigned)counter);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

/* ---- the stepper-level rig: the CPU stepper on a flat 8K-word memory, lines from ivs_stic_lines, a test-built configuration ---- */
static uint16_t si_mem[0x2000];

static ivs_bus_reply si_rig_bus(void *ctx, const ivs_cp1610_access *a)
{
  ivs_bus_reply r;
  (void)ctx;
  r.wait_cpu_cycles = 0u;
  r.data = 0u;
  if (a->kind == IVS_ACC_IAB) {
    r.data = (a->flags & IVS_ACC_F_INTERRUPT) != 0u ? 0x1004u : 0x1000u; /* the interrupt vector and the reset entry of the stand-ins */
  } else if (a->kind == IVS_ACC_WRITE || a->kind == IVS_ACC_DW) {
    si_mem[a->addr & 0x1FFFu] = a->data;
  } else if (a->kind == IVS_ACC_FETCH || a->kind == IVS_ACC_READ || a->kind == IVS_ACC_ADAR) {
    r.data = si_mem[a->addr & 0x1FFFu];
  }
  return r;
}

/* One frame of sc_prog_isr(dis_first) on the rig with the given C41 value: the number of interrupt entries; *t0 the first entry start. */
static uint32_t si_rig_entries(uint32_t c41, uint32_t dis_first, uint64_t *t0)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint16_t w[SC_PROG_MAX];
  size_t n = sc_prog_isr(w, dis_first);
  size_t i;
  ivs_cp1610 cpu;
  ivs_cp1610_config cfg;
  ivs_cp1610_lines lines = {0u, 0u, 0u, 0u};
  ivs_cp1610_port port;
  ivs_stic_state s;
  uint64_t now = 0u;
  uint32_t entries = 0u;
  uint32_t units = 0u;
  memset(si_mem, 0, sizeof si_mem);
  for (i = 0u; i < n; i++) {
    si_mem[0x1000u + i] = w[i];
  }
  memset(&cfg, 0, sizeof cfg);
  cfg.interrupt_accept_clears_intff = (uint8_t)c41;
  cfg.release_latency_cpu_cycles = (uint8_t)ivs_profile_get(SC_PROFILE)->release_latency_cpu_cycles;
  cfg.wait_limit_cpu_cycles = 35u;
  memset(&port, 0, sizeof port);
  port.bus = si_rig_bus;
  (void)ivs_cp1610_reset(&cpu);
  ivs_stic_state_reset(&s, p);
  *t0 = 0u;
  while (now < (uint64_t)p->frame_ticks && units++ < 20000u) {
    ivs_cp1610_result r;
    ivs_stic_line_levels l = {0u, 0u};
    if (now >= p->line_sample_lag_ticks) {
      l = ivs_stic_lines(p, &s, now - p->line_sample_lag_ticks);
    }
    lines.intrm = (uint8_t)l.intrm;
    if (ivs_cp1610_step(&cpu, &cfg, &lines, &port, NULL, 0u, &r) != IVS_OK) {
      return 0xFFFFFFFFu;
    }
    if (r.unit_kind == IVS_CP1610_UNIT_INT_ENTRY) {
      *t0 = entries == 0u ? now : *t0;
      entries++;
    }
    now += (uint64_t)r.elapsed_cpu_cycles * (uint64_t)p->ticks_per_cpu_cycle;
  }
  return entries;
}

static int si_c41_both_values(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint64_t t0 = 0u;
  uint32_t c41;
  uint32_t dis;
  for (c41 = 0u; c41 < 2u; c41++) {
    for (dis = 0u; dis < 2u; dis++) {
      uint32_t got = si_rig_entries(c41, dis, &t0);
      uint32_t expect = (c41 == 0u && dis == 0u) ? si_reentry_closed_form(p, t0, 15u * (uint64_t)p->ticks_per_cpu_cycle) : 1u;
      if (got == 0xFFFFFFFFu || got != expect || (c41 == 0u && dis == 0u && got < 100u)) {
        ivs_test_diag("C41 = %u, DIS first = %u: %u entries in a frame, %u expected", (unsigned)c41, (unsigned)dis, (unsigned)got,
                      (unsigned)expect);
        return IVS_TEST_FAIL;
      }
    }
  }
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_stic_irq_cases[] = {
    {"stic.irq.one-per-frame", si_one_per_frame, NULL},
    {"stic.irq.open-boundary-only", si_open_boundary_only, NULL},
    {"stic.irq.dis-blocks", si_dis_blocks, NULL},
    {"stic.irq.eis-late-misses", si_eis_late_misses, NULL},
    {"stic.irq.level-held-reentry-c41-0", si_reentry_c41_0, NULL},
    {"stic.irq.dis-first-isr-one-entry", si_dis_first_one_entry, NULL},
    {"stic.irq.c41-both-values", si_c41_both_values, NULL},
};
const size_t ivs_stic_irq_count = sizeof ivs_stic_irq_cases / sizeof ivs_stic_irq_cases[0];
