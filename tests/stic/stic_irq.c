/* SPDX-License-Identifier: MIT */
/* Suite stic, family irq (plan 06-08; D-08, A-09): the VBLANK interrupt request as the CPU stepper accepts it. INTRM is a level from
 * each frame start; the stepper accepts it only at an interruptible boundary with INTFF set (EIS), and with C41 = 0 acceptance does
 * not clear INTFF, so every service routine of these stand-ins starts with DIS. The stand-ins are the programs of stic_cases.h.
 * Expectations are relations to the parameters (an entry starts at or after the frame start plus the sampling lag and its sample is
 * inside the INTRM level) and to the guest's own counter (R2), never a policy number (D-02).
 *   stic.irq.one-per-frame         EIS and a DIS-first counting ISR: one interrupt entry per frame start crossed, each inside its level
 *   stic.irq.open-boundary-only    a stream of non-interruptible stores is never interrupted; the first entry is the boundary after it
 *   stic.irq.dis-blocks            the same program without EIS has no entry in two frames
 *   stic.irq.eis-late-misses       an EIS whose next open boundary is after the level ends misses the frame, and the next frame is taken */
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

const ivs_test_case ivs_stic_irq_cases[] = {
    {"stic.irq.one-per-frame", si_one_per_frame, NULL},
    {"stic.irq.open-boundary-only", si_open_boundary_only, NULL},
    {"stic.irq.dis-blocks", si_dis_blocks, NULL},
    {"stic.irq.eis-late-misses", si_eis_late_misses, NULL},
};
const size_t ivs_stic_irq_count = sizeof ivs_stic_irq_cases / sizeof ivs_stic_irq_cases[0];
