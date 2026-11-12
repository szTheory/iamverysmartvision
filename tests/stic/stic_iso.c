/* SPDX-License-Identifier: MIT */
/* Suite stic, family iso and regress (plan 06-08; D-10, D-16): two instances never influence each other, and a guest that never sets INTFF
 * and never strobes the display runs exactly as it ran before the STIC drove any line.
 *   stic.iso.two-instances   two display-on interrupt loops of different shapes run interleaved in calls of 1,000 ticks; each ends byte for byte
 *                            like the same guest run alone in one call to the same time
 *   stic.regress.guest-without-eis-or-strobe-unchanged  stand-in program A (no EIS, no strobe): the machine's unit sequence (kind, cycles,
 *                            opcode), time and CPU digest equal those of the pre-STIC-lines seam, replicated here: the CPU stepper driven with
 *                            all four lines 0 through the machine bus wrapper; the Phase 4 and 5 goldens pin the same fact for the fixtures */
#include <stdint.h>
#include <string.h>

#include "ivs/ivs.h"
#include "machine/machine.h"
#include "machine_cases.h"
#include "stic_cases.h"

static int sx_call(ivs_machine *m, uint64_t target)
{
  ivs_run_request q;
  ivs_run_report rep;
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.target_ticks = target;
  memset(&rep, 0, sizeof rep);
  rep.struct_size = (uint32_t)sizeof rep;
  return ivs_machine_run(m, &q, &rep) == IVS_OK;
}

static int sx_two_instances(void)
{
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint16_t wa[SC_PROG_MAX + 8u];
  uint16_t wb[SC_PROG_MAX + 8u];
  size_t na = sc_prog_vbl(wa, SC_BODY_INCR | SC_BODY_BACKTAB);
  size_t nb = sc_prog_vbl(wb, SC_BODY_J);
  ivs_ms_inst a;
  ivs_ms_inst b;
  ivs_ms_inst sa;
  ivs_ms_inst sb;
  const struct ivs_machine *ia;
  const struct ivs_machine *ib;
  uint64_t total = 3u * (uint64_t)p->frame_ticks;
  int ok = 0;
  memset(&a, 0, sizeof a);
  memset(&b, 0, sizeof b);
  memset(&sa, 0, sizeof sa);
  memset(&sb, 0, sizeof sb);
  if (!sc_guest_n(&a, wa, na) || !sc_guest_n(&b, wb, nb)) {
    goto done;
  }
  ia = ivs_machine_from_handle(a.m);
  ib = ivs_machine_from_handle(b.m);
  while (ia->time_ticks < total || ib->time_ticks < total) {
    if ((ia->time_ticks < total && !sx_call(a.m, ia->time_ticks + 1000u)) || (ib->time_ticks < total && !sx_call(b.m, ib->time_ticks + 1000u))) {
      goto done;
    }
  }
  if (!sc_guest_n(&sa, wa, na) || !sc_guest_n(&sb, wb, nb) || !sx_call(sa.m, ia->time_ticks) || !sx_call(sb.m, ib->time_ticks)) {
    goto done;
  }
  if (memcmp(a.storage, sa.storage, IVS_MACHINE_BYTES) != 0 || memcmp(b.storage, sb.storage, IVS_MACHINE_BYTES) != 0 ||
      ia->stic.late_grants != 0u || ia->cpu.r[2] < 3u || ib->cpu.r[2] < 3u) { /* each ran its own interrupts: R2 counts them */
    ivs_test_diag("an interleaved instance differs from its solo run (or a guest never ran its interrupts)");
    goto done;
  }
  ok = 1;
done:
  ivs_ms_free(&a);
  ivs_ms_free(&b);
  if (sa.storage != NULL) {
    ivs_ms_free(&sa);
  }
  if (sb.storage != NULL) {
    ivs_ms_free(&sb);
  }
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int sx_regress(void)
{
  ivs_ms_inst x;
  ivs_ms_inst y;
  struct ivs_machine *ix;
  struct ivs_machine *iy;
  const ivs_profile_desc *pr;
  ivs_cp1610_config cfg;
  ivs_cp1610_lines lines = {0u, 0u, 0u, 0u};
  ivs_cp1610_port port;
  uint8_t cx[32];
  uint8_t cy[32];
  uint8_t cpu_x[32];
  uint8_t cpu_y[32];
  uint32_t k;
  int ok = 0;
  memset(&x, 0, sizeof x);
  memset(&y, 0, sizeof y);
  if (!ivs_ms_powered_prog(&x, IVS_MS_PROG_A, 0x5Au)) {
    return IVS_TEST_FAIL;
  }
  if (!ivs_ms_powered_prog(&y, IVS_MS_PROG_A, 0x5Au)) {
    ivs_ms_free(&x);
    return IVS_TEST_FAIL;
  }
  ix = ivs_machine_from_handle_mut(x.m);
  iy = ivs_machine_from_handle_mut(y.m);
  pr = ivs_profile_get(ix->profile);
  memset(&cfg, 0, sizeof cfg);
  cfg.interrupt_accept_clears_intff = pr->interrupt_accept_clears_intff;
  cfg.release_latency_cpu_cycles = pr->release_latency_cpu_cycles;
  cfg.wait_limit_cpu_cycles = pr->wait_limit_cpu_cycles;
  memset(&port, 0, sizeof port);
  port.bus = ivs_machine_bus;
  for (k = 0u; k < 6000u; k++) { /* 6,000 units cover more than a frame of the 4-instruction loop */
    ivs_cp1610_result r1;
    ivs_cp1610_result r2;
    ivs_machine_bus_ctx c;
    memset(&r2, 0, sizeof r2);
    if (ivs_machine_step_unit_internal(x.m, &r1) != IVS_OK) {
      goto done;
    }
    memset(&c, 0, sizeof c); /* a bare wrapper: no STIC, no PSG sink */
    c.m = iy;
    c.unit_start_tick = iy->time_ticks;
    c.ticks_per_cpu_cycle = pr->ticks_per_cpu_cycle;
    port.ctx = &c;
    if (ivs_cp1610_step(&iy->cpu, &cfg, &lines, &port, NULL, 0u, &r2) != IVS_OK) {
      goto done;
    }
    iy->time_ticks += (uint64_t)r2.elapsed_cpu_cycles * (uint64_t)pr->ticks_per_cpu_cycle;
    if (r1.elapsed_cpu_cycles != r2.elapsed_cpu_cycles || r1.unit_kind != r2.unit_kind || r1.opcode != r2.opcode) {
      ivs_test_diag("unit %u: machine %u cycles kind %u opcode %04X, pre-STIC-lines seam %u cycles kind %u opcode %04X", (unsigned)k,
                    (unsigned)r1.elapsed_cpu_cycles, (unsigned)r1.unit_kind, (unsigned)r1.opcode, (unsigned)r2.elapsed_cpu_cycles,
                    (unsigned)r2.unit_kind, (unsigned)r2.opcode);
      goto done;
    }
  }
  if (ix->time_ticks != iy->time_ticks || !ivs_ms_digests(x.m, cx, cpu_x) || !ivs_ms_digests(y.m, cy, cpu_y) ||
      memcmp(cpu_x, cpu_y, 32u) != 0 || ix->time_ticks <= (uint64_t)ivs_stic_params_get(ix->profile)->frame_ticks) {
    ivs_test_diag("time %llu vs %llu, or the CPU digests differ, or the run did not pass a frame", (unsigned long long)ix->time_ticks,
                  (unsigned long long)iy->time_ticks);
    goto done;
  }
  if (ix->stic.display_latch != 0u || ix->stic.windows_active != 0u || ix->stic.late_grants != 0u || ix->stic.grants_in_frame != 0u) {
    ivs_test_diag("a guest without strobe left the display latch, windows or grant counters set");
    goto done;
  }
  ok = 1;
done:
  ivs_ms_free(&x);
  ivs_ms_free(&y);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_case ivs_stic_iso_cases[] = {
    {"stic.iso.two-instances", sx_two_instances, NULL},
    {"stic.regress.guest-without-eis-or-strobe-unchanged", sx_regress, NULL},
};
const size_t ivs_stic_iso_count = sizeof ivs_stic_iso_cases / sizeof ivs_stic_iso_cases[0];
