/* SPDX-License-Identifier: MIT */
/* Suite stic, family fetch (plan 06-08; D-11, A-05, A-06): the row-granular fetch of BACKTAB into the 12 row latches at the ROW_FETCH
 * events of a display-on frame. The tick of a row's fetch is the table tick of its event (a relation to the parameters, D-02); the
 * guest words are generated patterns; the straddle case observes unit starts by single stepping and reads the stamp of the store
 * from the unit's access record, so it needs no arithmetic about the stalls in front of it.
 *   stic.fetch.rows-latched-at-schedule  row i holds system RAM row i as of its fetch tick; the table has 12 fetches and none for window 12
 *   stic.fetch.display-off-no-latch      without the display strobe no row is fetched
 *   stic.fetch.write-after-latch-tears   a store after row 3's fetch and before the frame end leaves row 3 old and the unfetched row 5 new
 *   stic.fetch.backtab-straddle          a store swept across a fetch tick: the latch holds the old word iff the stamp is at or after the tick */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "machine/machine.h"
#include "machine_cases.h"
#include "stic_cases.h"

/* The table tick of the fetch of row r (from the frame start), or UINT64_MAX; counts the fetch events in *n. */
static uint64_t sf_fetch_tick(const ivs_stic_params *p, uint32_t r, uint32_t *n)
{
  uint32_t i;
  uint64_t t = UINT64_MAX;
  *n = 0u;
  for (i = 0u; i < p->event_count; i++) {
    if (p->event_kind[i] == IVS_STIC_EV_ROW_FETCH) {
      *n += 1u;
      if (p->event_arg[i] == r) {
        t = (uint64_t)p->frame_phase_ticks + (uint64_t)p->event_tick[i];
      }
    }
  }
  return t;
}

static uint16_t sf_pat_a(uint32_t i)
{
  return (uint16_t)(0x8000u + i);
}

static uint16_t sf_pat_b(uint32_t i)
{
  return (uint16_t)(sf_pat_a(i) ^ 0x7FFFu);
}

static void sf_poke_all(struct ivs_machine *im, uint16_t (*pat)(uint32_t))
{
  uint32_t i;
  for (i = 0u; i < 240u; i++) {
    ivs_memsys_poke_raw_internal(&im->mem, (uint16_t)(0x0200u + i), pat(i));
  }
}

/* 1 when row r of the latches holds pat for every word. */
static int sf_row_is(const struct ivs_machine *im, uint32_t r, uint16_t (*pat)(uint32_t))
{
  uint32_t c;
  for (c = 0u; c < IVS_STIC_ROW_WORDS; c++) {
    if (im->stic.row_latch[r][c] != pat(r * IVS_STIC_ROW_WORDS + c)) {
      return 0;
    }
  }
  return 1;
}

static int sf_row_zero(const struct ivs_machine *im, uint32_t r)
{
  uint32_t c;
  for (c = 0u; c < IVS_STIC_ROW_WORDS; c++) {
    if (im->stic.row_latch[r][c] != 0u) {
      return 0;
    }
  }
  return 1;
}

static int sf_rows_at_schedule(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint16_t w[SC_PROG_MAX];
  ivs_ms_inst x;
  struct ivs_machine *im;
  uint32_t n = 0u;
  uint32_t r;
  uint64_t t5 = sf_fetch_tick(p, 5u, &n);
  int ok = 0;
  if (n != IVS_STIC_LATCH_ROWS || sf_fetch_tick(p, IVS_STIC_LATCH_ROWS, &n) != UINT64_MAX || t5 == UINT64_MAX) {
    ivs_test_diag("the table has %u fetch events (12 expected, none for window 12)", (unsigned)n);
    return IVS_TEST_FAIL;
  }
  if (!sc_guest(&x, w, sc_prog_halt(w), 0u)) {
    return IVS_TEST_FAIL;
  }
  im = ivs_machine_from_handle_mut(x.m);
  sf_poke_all(im, sf_pat_a);
  if (!sc_run_until(&x, t5 + 4u)) {
    goto done;
  }
  for (r = 0u; r < IVS_STIC_LATCH_ROWS; r++) { /* as of tick t5: rows 0..5 fetched the old words, rows 6..11 are still to come */
    if (r <= 5u ? !sf_row_is(im, r, sf_pat_a) : !sf_row_zero(im, r)) {
      ivs_test_diag("at t5 + 4 row %u is not as scheduled", (unsigned)r);
      goto done;
    }
  }
  sf_poke_all(im, sf_pat_b);
  if (!sc_run_until(&x, (uint64_t)p->frame_phase_ticks + (uint64_t)p->frame_ticks - 100u)) {
    goto done;
  }
  for (r = 0u; r < IVS_STIC_LATCH_ROWS; r++) { /* rows 0..5 kept what they fetched, rows 6..11 fetched the words written after row 5 */
    if (!sf_row_is(im, r, r <= 5u ? sf_pat_a : sf_pat_b)) {
      ivs_test_diag("at the frame end row %u is not as scheduled", (unsigned)r);
      goto done;
    }
  }
  ok = 1;
done:
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int sf_display_off(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint16_t w[SC_PROG_MAX];
  ivs_ms_inst x;
  struct ivs_machine *im;
  uint32_t r;
  int ok = 0;
  if (!sc_guest(&x, w, sc_prog_loop(w, 0u), 0u)) {
    return IVS_TEST_FAIL;
  }
  im = ivs_machine_from_handle_mut(x.m);
  sf_poke_all(im, sf_pat_a);
  if (!sc_run_until(&x, (uint64_t)p->frame_phase_ticks + (uint64_t)p->frame_ticks - 100u)) {
    goto done;
  }
  for (r = 0u; r < IVS_STIC_LATCH_ROWS; r++) {
    if (!sf_row_zero(im, r)) {
      ivs_test_diag("row %u was fetched in a frame without the display strobe", (unsigned)r);
      goto done;
    }
  }
  ok = im->stic.windows_active == 0u;
done:
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int sf_write_after_latch_tears(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint16_t w[SC_PROG_MAX];
  ivs_ms_inst x;
  struct ivs_machine *im;
  uint32_t n = 0u;
  uint64_t t3 = sf_fetch_tick(p, 3u, &n);
  int ok = 0;
  if (!sc_guest(&x, w, sc_prog_halt(w), 0u)) {
    return IVS_TEST_FAIL;
  }
  im = ivs_machine_from_handle_mut(x.m);
  sf_poke_all(im, sf_pat_a);
  if (!sc_run_until(&x, t3 + 40u)) {
    goto done;
  }
  ivs_memsys_poke_raw_internal(&im->mem, (uint16_t)(0x0200u + 3u * 20u + 7u), 0x1111u); /* a word of row 3, already fetched */
  ivs_memsys_poke_raw_internal(&im->mem, (uint16_t)(0x0200u + 5u * 20u + 9u), 0x2222u); /* a word of row 5, not yet fetched */
  if (!sc_run_until(&x, (uint64_t)p->frame_phase_ticks + (uint64_t)p->frame_ticks - 100u)) {
    goto done;
  }
  if (im->stic.row_latch[3][7] != sf_pat_a(3u * 20u + 7u) || im->stic.row_latch[5][9] != 0x2222u) {
    ivs_test_diag("row 3 word 7 is %04X (old %04X), row 5 word 9 is %04X (new 2222)", (unsigned)im->stic.row_latch[3][7],
                  (unsigned)sf_pat_a(3u * 20u + 7u), (unsigned)im->stic.row_latch[5][9]);
    goto done;
  }
  ok = 1;
done:
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* strobe; R2 = $155; delay n_delay iterations; a x INCR R5 (6 cycles), b x TSTR R6 (7), c x CLRC (4, not interruptible); K stores to the
 * scratchpad (11 cycles, not interruptible: the stream keeps the CPU running through the window that starts just before the fetch
 * tick, the late-grant case, so the fetch tick falls inside the stream); MVO R2, $023C (row 3, word 0); B self */
static size_t sf_prog(uint16_t *w, uint32_t n_delay, uint32_t a, uint32_t b, uint32_t c)
{
  size_t n = sc_w_mvo(w, 0u, 0u, 0x0020u);
  uint32_t k;
  n = sc_w_mvii(w, n, 2u, 0x0155u);
  n = sc_w_delay(w, n, n_delay);
  for (k = 0u; k < a; k++) {
    w[n++] = 0x000Du;
  }
  for (k = 0u; k < b; k++) {
    w[n++] = 0x00B6u;
  }
  for (k = 0u; k < c; k++) {
    w[n++] = 0x0006u;
  }
  for (k = 0u; k < SC_STRADDLE_K; k++) {
    n = sc_w_mvo(w, n, 0u, 0x0100u); /* a non-interruptible store to the scratchpad: the window cannot stall the CPU before it ends */
  }
  n = sc_w_mvo(w, n, 2u, 0x023Cu);
  return sc_w_loop(w, n);
}

/* Runs the program unit by unit past the fetch tick of row 3; returns the stamp of the store (from the access record) in *stamp and 1
 * when the latch holds the old word, 0 when it holds the new one, -1 on an error or when the store was never seen. */
static int sf_straddle_run(uint32_t n_delay, uint32_t a, uint32_t b, uint32_t c, uint64_t t3, uint64_t *stamp)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint16_t w[64];
  ivs_ms_inst x;
  const struct ivs_machine *im;
  ivs_psg_sink sink;
  uint32_t units = 0u;
  int seen = 0;
  int verdict = -1;
  *stamp = 0u;
  if (!sc_guest_n(&x, w, sf_prog(w, n_delay, a, b, c))) {
    return -1;
  }
  im = ivs_machine_from_handle(x.m);
  ivs_psg_sink_init(&sink, NULL, 0u);
  while (im->time_ticks < t3 + 1500u && units++ < 6000u) {
    ivs_cp1610_access acc[8];
    ivs_cp1610_result r;
    uint64_t start = im->time_ticks;
    uint32_t j;
    if (ivs_machine_step_unit_trace_internal(x.m, &r, &sink, acc, 8u) != IVS_OK) {
      goto done;
    }
    for (j = 0u; j < r.n_accesses && j < 8u; j++) {
      if (acc[j].kind == IVS_ACC_WRITE && acc[j].addr == 0x023Cu) {
        *stamp = start + (uint64_t)acc[j].offset_cpu_cycles * (uint64_t)p->ticks_per_cpu_cycle + (uint64_t)p->access_strobe_ticks;
        seen = 1;
      }
    }
  }
  if (seen != 0 && im->mem.sysram[60] == 0x0155u) {
    verdict = im->stic.row_latch[3][0] == 0u ? 1 : (im->stic.row_latch[3][0] == 0x0155u ? 0 : -1);
  }
done:
  ivs_ms_free(&x);
  return verdict;
}

static int sf_backtab_straddle(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint32_t n = 0u;
  uint64_t t3 = sf_fetch_tick(p, 3u, &n);
  uint32_t before = 0u;
  uint32_t tie = 0u;
  uint32_t after = 0u;
  uint32_t nd;
  uint32_t a;
  uint32_t b;
  uint32_t c;
  for (nd = SC_STRADDLE_N0; nd < SC_STRADDLE_N0 + 3u; nd++) {
    for (a = 0u; a < 4u; a++) {
      for (b = 0u; b < 4u; b++) {
        for (c = 0u; c < 3u; c++) {
          uint64_t stamp;
          int verdict = sf_straddle_run(nd, a, b, c, t3, &stamp);
          if (verdict < 0) {
            ivs_test_diag("run (%u, %u, %u, %u) did not reach a verdict", (unsigned)nd, (unsigned)a, (unsigned)b, (unsigned)c);
            return IVS_TEST_FAIL;
          }
          if (stamp + 200u < t3 || stamp > t3 + 200u) {
            continue; /* far from the tick: not a straddle */
          }
          if ((verdict == 1) != (stamp >= t3)) { /* old word iff the stamp is at or after the fetch tick (the tie rule, A-06) */
            ivs_test_diag("store stamped %lld ticks from the fetch tick: the latch holds the %s word", (long long)stamp - (long long)t3,
                          verdict == 1 ? "old" : "new");
            return IVS_TEST_FAIL;
          }
          if (stamp < t3) {
            before++;
          } else if (stamp == t3) {
            tie++;
          } else {
            after++;
          }
        }
      }
    }
  }
  if (before < 3u || tie < 1u || after < 3u) {
    ivs_test_diag("the sweep covered %u stamps before, %u at and %u after the fetch tick", (unsigned)before, (unsigned)tie, (unsigned)after);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_stic_fetch_cases[] = {
    {"stic.fetch.rows-latched-at-schedule", sf_rows_at_schedule, NULL},
    {"stic.fetch.display-off-no-latch", sf_display_off, NULL},
    {"stic.fetch.write-after-latch-tears", sf_write_after_latch_tears, NULL},
    {"stic.fetch.backtab-straddle", sf_backtab_straddle, NULL},
};
const size_t ivs_stic_fetch_count = sizeof ivs_stic_fetch_cases / sizeof ivs_stic_fetch_cases[0];
