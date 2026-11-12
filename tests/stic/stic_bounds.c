/* SPDX-License-Identifier: MIT */
/* Suite stic, family bounds (plan 06-08; D-10, A-09, T-06-11): the published bounds are RE-PROVED through the full machine with the STIC
 * driving its lines: U = psg.max_unit_ticks = 468 (the longest CPU unit), g = psg.min_write_gap_ticks = 28 (the shortest gap between two
 * PSG register writes) and W_log = psg.log_records_per_frame = 2,152. What a bus-grant stall changes: its units (the 1-cycle grant and hold
 * units, the release of bus.release_latency_cpu_cycles = 2 cycles) are at most 8 ticks and the interrupt entry (9 cycles) 36 ticks, both far
 * below U, and a stall only LENGTHENS the time between two writes, so g and W_log stay valid. The probes step the machine through the internal
 * seam with the access record of every unit (write stamps are read from it) and assert the bounds, never take them.
 *   stic.bounds.g-and-u-full-machine        an adversarial PSG writer in the interrupt program, display on, windows and VBLANK entries live, at
 *                                           twelve phases of the writer relative to the windows: no unit exceeds U, no access carries a wait,
 *                                           consecutive writes are at least g apart and any k + 1 writes span at least k g ticks (so writes in
 *                                           T ticks are at most (T + U) / g + 1)
 *   stic.bounds.log-records-per-frame-le-W  the densest reachable writer (MVO@ back to back, not interruptible) makes at most W_log records in
 *                                           any span of one frame, with the display on and off; the measured count is recorded
 *   stic.bounds.unit-ticks-le-U             all 1,024 decles, SDBD off and on, with the interrupt level held (INTFF set) and a bus window
 *                                           open: every unit of the real machine is at most U
 *   stic.bounds.no-wait-reply               the machine wrapper replies a wait of 0 for every access kind at sampled addresses inside and
 *                                           outside the register and graphics windows (the premise of U on the real bus) */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "machine/machine.h"
#include "machine_cases.h"
#include "stic_cases.h"

#define SO_STAMPS 12000u
#define SO_CAP 40000u

typedef struct so_stats {
  uint64_t max_unit_ticks;
  uint64_t min_gap;
  uint32_t waits;
  uint32_t entries;
  uint32_t grants;
  size_t writes;
} so_stats;

/* Steps the guest unit by unit to machine time until, collecting the stamps of every PSG register write (WRITE or DW at $01F0..$01FF) from the
 * access records. 1 on success. */
static int so_run(const uint16_t *w, size_t n, uint64_t until, uint64_t *stamps, so_stats *st)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  const ivs_psg_params *pp = ivs_psg_params_get(SC_PROFILE);
  ivs_ms_inst x;
  const struct ivs_machine *im;
  ivs_psg_sink sink;
  uint32_t units = 0u;
  int ok = 0;
  memset(&x, 0, sizeof x);
  memset(st, 0, sizeof *st);
  st->min_gap = UINT64_MAX;
  if (!sc_guest_n(&x, w, n)) {
    return 0;
  }
  im = ivs_machine_from_handle(x.m);
  ivs_psg_sink_init(&sink, NULL, 0u);
  while (im->time_ticks < until && units++ < 200000u) {
    ivs_cp1610_access acc[8];
    ivs_cp1610_result r;
    uint64_t start = im->time_ticks;
    uint32_t j;
    if (ivs_machine_step_unit_trace_internal(x.m, &r, &sink, acc, 8u) != IVS_OK) {
      goto done;
    }
    if ((uint64_t)r.elapsed_cpu_cycles * p->ticks_per_cpu_cycle > st->max_unit_ticks) {
      st->max_unit_ticks = (uint64_t)r.elapsed_cpu_cycles * p->ticks_per_cpu_cycle;
    }
    st->entries += r.unit_kind == IVS_CP1610_UNIT_INT_ENTRY ? 1u : 0u;
    st->grants += (r.events & IVS_CP1610_EV_BUS_RELEASED) != 0u ? 1u : 0u;
    for (j = 0u; j < r.n_accesses && j < 8u; j++) {
      st->waits += acc[j].wait_cpu_cycles != 0u ? 1u : 0u;
      if ((acc[j].kind == IVS_ACC_WRITE || acc[j].kind == IVS_ACC_DW) && (acc[j].addr & 0xFFF0u) == 0x01F0u && st->writes < SO_STAMPS) {
        uint64_t stamp = start + (uint64_t)acc[j].offset_cpu_cycles * p->ticks_per_cpu_cycle + pp->write_strobe_ticks;
        if (st->writes > 0u && stamp - stamps[st->writes - 1u] < st->min_gap) {
          st->min_gap = stamp - stamps[st->writes - 1u];
        }
        stamps[st->writes++] = stamp;
      }
    }
  }
  ok = im->time_ticks >= until;
done:
  ivs_ms_free(&x);
  return ok;
}

static int so_g_and_u(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  const ivs_profile_desc *pr = ivs_profile_get(SC_PROFILE);
  uint64_t u = ivs_psg_params_get(SC_PROFILE)->max_unit_ticks;
  uint64_t g = pr->psg_min_write_gap_ticks;
  uint64_t *stamps = malloc(SO_STAMPS * sizeof *stamps);
  uint16_t w[64];
  uint32_t k;
  uint32_t pad;
  uint32_t total_entries = 0u;
  uint32_t total_grants = 0u;
  uint64_t overall_min = UINT64_MAX;
  uint64_t overall_max_unit = 0u;
  int ok = 0;
  if (stamps == NULL) {
    return IVS_TEST_FAIL;
  }
  static const uint32_t ks[3] = {1u, 2u, 4u};
  for (k = 0u; k < 3u; k++) {
    for (pad = 0u; pad < 4u; pad++) {
      so_stats st;
      size_t i;
      size_t j;
      if (!so_run(w, sc_prog_psg(w, ks[k], pad), 3u * (uint64_t)p->frame_ticks, stamps, &st)) {
        ivs_test_diag("run (%u stores, %u pads) failed", (unsigned)ks[k], (unsigned)pad);
        goto done;
      }
      if (st.max_unit_ticks > u || st.waits != 0u || st.writes < 100u || (st.writes > 1u && st.min_gap < g)) {
        ivs_test_diag("(%u, %u): max unit %llu (U %llu), %u waits, %lu writes, min gap %llu (g %llu)", (unsigned)ks[k], (unsigned)pad,
                      (unsigned long long)st.max_unit_ticks, (unsigned long long)u, (unsigned)st.waits, (unsigned long)st.writes,
                      (unsigned long long)st.min_gap, (unsigned long long)g);
        goto done;
      }
      for (i = 0u; i < st.writes; i++) { /* writes in a span of T ticks: at most (T + U) / g + 1 */
        for (j = i + 1u; j < st.writes && j < i + 65u; j++) {
          if ((uint64_t)(j - i + 1u) > (stamps[j] - stamps[i] + u) / g + 1u) {
            ivs_test_diag("%lu writes within %llu ticks exceed (T + U) / g + 1", (unsigned long)(j - i + 1u),
                          (unsigned long long)(stamps[j] - stamps[i]));
            goto done;
          }
        }
      }
      total_entries += st.entries;
      total_grants += st.grants;
      overall_min = st.min_gap < overall_min ? st.min_gap : overall_min;
      overall_max_unit = st.max_unit_ticks > overall_max_unit ? st.max_unit_ticks : overall_max_unit;
    }
  }
  if (total_entries < 24u || total_grants < 12u * 3u * p->bus_count / 2u) { /* non-vacuity: the lines were live */
    ivs_test_diag("vacuous: %u entries, %u grants", (unsigned)total_entries, (unsigned)total_grants);
    goto done;
  }
  ivs_test_diag("measured through the full machine: max unit %llu ticks (U %llu), min write gap %llu ticks (g %llu), %u grants, %u entries",
                (unsigned long long)overall_max_unit, (unsigned long long)u, (unsigned long long)overall_min, (unsigned long long)g,
                (unsigned)total_grants, (unsigned)total_entries);
  ok = 1;
done:
  free(stamps);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* The most records in any span of one frame, from the write stamps. */
static size_t so_max_in_frame(const uint64_t *s, size_t n, uint64_t frame)
{
  size_t best = 0u;
  size_t i;
  size_t j = 0u;
  for (i = 0u; i < n; i++) {
    while (j < n && s[j] < s[i] + frame) {
      j++;
    }
    best = j - i > best ? j - i : best;
  }
  return best;
}

static int so_log_le_w(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  ivs_ms_inst x;
  ivs_info info;
  uint64_t *stamps = malloc(SO_STAMPS * sizeof *stamps);
  uint16_t *w = malloc(4096u * sizeof *w);
  uint32_t strobe;
  int ok = 0;
  memset(&x, 0, sizeof x);
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (stamps == NULL || w == NULL) {
    goto done;
  }
  if (!ivs_ms_powered(&x, IVS_MS_CONTENT_A, 0x5Au) || ivs_machine_info(x.m, &info) != IVS_OK) {
    goto done;
  }
  ivs_ms_free(&x);
  memset(&x, 0, sizeof x);
  for (strobe = 0u; strobe < 2u; strobe++) {
    so_stats st;
    size_t most;
    size_t n = sc_prog_psg_stream(w, 1800u, strobe);
    if (!so_run(w, n, 2u * (uint64_t)p->frame_ticks, stamps, &st) || st.waits != 0u || st.grants != 0u) {
      ivs_test_diag("stream run failed (strobe %u): %u waits, %u grants (a non-interruptible stream is never stalled)", (unsigned)strobe,
                    (unsigned)st.waits, (unsigned)st.grants);
      goto done;
    }
    most = so_max_in_frame(stamps, st.writes, p->frame_ticks);
    ivs_test_diag("densest writer, display %s: %lu PSG writes in a span of one frame (W_log %u)", strobe != 0u ? "on" : "off",
                  (unsigned long)most, (unsigned)info.psg_log_records_per_frame);
    if (most < 1500u || most > info.psg_log_records_per_frame) {
      goto done;
    }
  }
  ok = 1;
done:
  free(stamps);
  free(w);
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* A one-instruction guest for every decle: word 0 is the decle (preceded by SDBD when sdbd is set), the rest of the image is the generated
 * pattern of the stand-in EXEC; the instance is reset, the interrupt flip-flop is set and the STIC is at the start of its first bus window. */
static int so_unit_ticks(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint64_t u = ivs_psg_params_get(SC_PROFILE)->max_unit_ticks;
  uint16_t w[2] = {0x0034u, 0x0034u};
  ivs_ms_inst x;
  struct ivs_machine *im;
  uint32_t decle;
  uint32_t sdbd;
  uint32_t phase;
  uint64_t max_ticks = 0u;
  uint32_t grants = 0u;
  uint32_t entries = 0u;
  int ok = 0;
  memset(&x, 0, sizeof x);
  if (!sc_guest_n(&x, w, 2u)) {
    return IVS_TEST_FAIL;
  }
  im = ivs_machine_from_handle_mut(x.m);
  for (phase = 0u; phase < 2u; phase++) { /* 0: at the frame start, the interrupt level is up; 1: at the first bus window */
    for (sdbd = 0u; sdbd < 2u; sdbd++) {
      for (decle = 0u; decle < 1024u; decle++) {
        uint32_t k;
        uint64_t t0 = (uint64_t)p->frame_phase_ticks + (uint64_t)p->bus_first_start_ticks - 8u * (uint64_t)p->ticks_per_cpu_cycle;
        if (ivs_machine_reset(x.m, IVS_RESET_POWER_ON) != IVS_OK) {
          ivs_test_diag("reset refused at decle %u", (unsigned)decle);
          goto done;
        }
        ivs_memsys_poke_raw_internal(&im->mem, 0x1000u, sdbd != 0u ? 0x0001u : (uint16_t)decle);
        ivs_memsys_poke_raw_internal(&im->mem, 0x1001u, sdbd != 0u ? (uint16_t)decle : 0x0034u);
        if (phase != 0u) {
          ivs_stic_catch_up(p, &im->stic, &im->mem, t0, NULL); /* the schedule up to the first window; the latch decision is made by hand: windows on */
          im->stic.windows_active = 1u;
          im->time_ticks = t0;
        }
        for (k = 0u; k < 24u; k++) {
          ivs_cp1610_result r;
          ivs_status st;
          if (k == 1u) {
            im->cpu.intff = 1u; /* set after the reset entry (a latch before it is not a state the stepper accepts) */
          }
          st = ivs_machine_step_unit_internal(x.m, &r);
          if (st != IVS_OK) {
            ivs_test_diag("step refused (%s) at decle %u sdbd %u unit %u", ivs_status_name(st), (unsigned)decle, (unsigned)sdbd, (unsigned)k);
            goto done;
          }
          if ((uint64_t)r.elapsed_cpu_cycles * p->ticks_per_cpu_cycle > max_ticks) {
            max_ticks = (uint64_t)r.elapsed_cpu_cycles * p->ticks_per_cpu_cycle;
          }
          grants += r.unit_kind == IVS_CP1610_UNIT_BUS_GRANT ? 1u : 0u;
          entries += r.unit_kind == IVS_CP1610_UNIT_INT_ENTRY ? 1u : 0u;
          if (r.unit_kind == IVS_CP1610_UNIT_FAULT) {
            break;
          }
        }
      }
    }
  }
  ivs_test_diag("2 x 2,048 programs: max unit %llu ticks (U %llu), %u bus-grant units, %u interrupt entries", (unsigned long long)max_ticks,
                (unsigned long long)u, (unsigned)grants, (unsigned)entries);
  if (max_ticks > u || max_ticks < 48u || grants < 1000u || entries < 500u) {
    goto done;
  }
  ok = 1;
done:
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int so_no_wait(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint16_t w[SC_PROG_MAX];
  ivs_ms_inst x;
  struct ivs_machine *im;
  uint32_t kind;
  uint32_t addr;
  uint32_t phase;
  uint32_t checked = 0u;
  int ok = 0;
  memset(&x, 0, sizeof x);
  if (!sc_guest(&x, w, sc_w_loop(w, 0u), 0u)) {
    return IVS_TEST_FAIL;
  }
  im = ivs_machine_from_handle_mut(x.m);
  for (phase = 0u; phase < 3u; phase++) { /* inside the register window, between the windows, after both */
    uint64_t at = phase == 0u ? 100u : (phase == 1u ? (uint64_t)p->reg_window_ticks + 100u : (uint64_t)p->gmem_window_ticks + 100u);
    for (kind = IVS_ACC_FETCH; kind <= IVS_ACC_STROBE_PCIT; kind++) {
      for (addr = 0u; addr < 0x10000u; addr += 17u) {
        ivs_machine_bus_ctx c;
        ivs_cp1610_access a;
        ivs_bus_reply r;
        memset(&c, 0, sizeof c);
        c.m = im;
        c.unit_start_tick = at;
        c.ticks_per_cpu_cycle = p->ticks_per_cpu_cycle;
        c.psg = ivs_psg_params_get(SC_PROFILE);
        c.stic = p;
        memset(&a, 0, sizeof a);
        a.kind = (uint8_t)kind;
        a.addr = (uint16_t)addr;
        a.data = 0x0055u;
        r = ivs_machine_bus(&c, &a);
        if (r.wait_cpu_cycles != 0u) {
          ivs_test_diag("kind %u at %04X, tick %llu: wait %u", (unsigned)kind, (unsigned)addr, (unsigned long long)at, (unsigned)r.wait_cpu_cycles);
          goto done;
        }
        checked++;
      }
    }
  }
  ok = checked > 100000u;
done:
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_case ivs_stic_bounds_cases[] = {
    {"stic.bounds.g-and-u-full-machine", so_g_and_u, NULL},
    {"stic.bounds.log-records-per-frame-le-W", so_log_le_w, NULL},
    {"stic.bounds.unit-ticks-le-U", so_unit_ticks, NULL},
    {"stic.bounds.no-wait-reply", so_no_wait, NULL},
};
const size_t ivs_stic_bounds_count = sizeof ivs_stic_bounds_cases / sizeof ivs_stic_bounds_cases[0];
