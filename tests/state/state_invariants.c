/* SPDX-License-Identifier: MIT */
/* Suite state, family restore, the derived invariants proved on real programs (plan 08-13, D-15, D-16, docs/formats/ivsn-v1.md "Derived
 * invariants"). A restore accepts a snapshot only if its decoded values satisfy rules that every reachable state satisfies, so a wrong rule
 * would refuse a real snapshot: each of INV-1 to INV-6 is implemented once, as one list of rules (ivs_snap_inv_rules) that both the restore
 * code (ivs_snap_check_probe) and ivs_snapshot_invariants walk, and is held here against the live instances of real programs at EVERY unit
 * boundary, before it is allowed to refuse anything. The programs: the Lamp Tone with no input and with the six events of its scenario, the boot stub alone, the first STIC
 * diagnostic with and without its selector, the second with each selector word, the nine stand-in programs of the machine suite, the stand-in
 * boot program, and one program that halts and one that ends in a guest fault (the decles of tests/run/run_slice.c, each derived there from
 * the printed vector rows). A boundary is the state after a unit, reached through the public run call one unit at a time with the events
 * resubmitted from the applied-event count (the way a host steps), or through the internal step seam for programs without input.
 * The test also proves that it can see what it claims: the programs reach states where each invariant has something to say (a closed
 * register gate and a closed graphics gate, a held key with a pull-down, the audio level 15, a halted and a faulted CPU, frames completed),
 * and the reset state (time 0, the cursor at the very start) occurs exactly once per program, the one state the first clause of INV-2
 * needed an exception for. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "machine/machine.h"
#include "state_cases.h"
#include "state_util.h"

/* What a scan saw. */
typedef struct inv_stats {
  uint64_t boundaries;
  uint64_t checked;
  uint64_t violated[7]; /* index n: boundaries where INV-n was the first that failed */
  uint64_t reset_state; /* boundaries with time 0, frame 0, event 0 */
  uint64_t regs_closed;
  uint64_t gmem_closed;
  uint64_t pulldown_set;
  uint64_t halted;
  uint64_t faulted;
  uint32_t max_level;
  uint64_t max_frames;
} inv_stats;

#define INV_RESET_STATE(p) ((p).time_ticks == 0u && (p).stic.frame_index == 0u && (p).stic.event_index == 0u)

/* Judges one boundary of the instance x (profile index 0 is the only registered profile) and accumulates what it saw; stride thins the
 * checks of a fault run (1 checks every boundary). */
static void inv_boundary(const char *name, const ivs_machine *m, uint32_t profile, inv_stats *st, uint64_t unit, uint32_t stride, int *first_report)
{
  ivs_snap_probe p;
  uint32_t v;
  uint32_t lv;
  st->boundaries++;
  if (unit % stride != 0u) {
    return;
  }
  ivs_snapshot_probe_from_instance(m, &p);
  st->checked++;
  v = ivs_snapshot_invariants(&p, profile);
  if (v != 0u) {
    st->violated[v]++;
    if (*first_report) {
      *first_report = 0;
      ivs_test_diag("%s: unit %llu violates INV-%u: time %llu, frame %llu, event %u, next_sample %llu, emitted %llu, floor %llu, video %llu",
                    name, (unsigned long long)unit, (unsigned)v, (unsigned long long)p.time_ticks,
                    (unsigned long long)p.stic.frame_index, (unsigned)p.stic.event_index, (unsigned long long)p.psg.next_sample,
                    (unsigned long long)p.audio_emitted, (unsigned long long)p.input.floor_tick, (unsigned long long)p.video_count);
    }
  }
  if (INV_RESET_STATE(p)) {
    st->reset_state++;
  }
  st->regs_closed += (uint64_t)(p.gate_open[IVS_MEMSYS_GATE_STIC_REGS] == 0u);
  st->gmem_closed += (uint64_t)(p.gate_open[IVS_MEMSYS_GATE_GMEM] == 0u);
  st->pulldown_set += (uint64_t)((p.port_pulldown[0] | p.port_pulldown[1]) != 0u);
  st->halted += (uint64_t)(p.cpu.run_state == IVS_CP1610_RS_HALTED);
  st->faulted += (uint64_t)(p.cpu.run_state == IVS_CP1610_RS_FAULTED);
  for (lv = 0u; lv < 3u; lv++) {
    uint32_t level = (p.audio_prev_levels >> (8u * lv)) & 0xFFu;
    if (level > st->max_level) {
      st->max_level = level;
    }
  }
  if (p.video_count > st->max_frames) {
    st->max_frames = p.video_count;
  }
}

static void inv_add(inv_stats *to, const inv_stats *from)
{
  uint32_t i;
  to->boundaries += from->boundaries;
  to->checked += from->checked;
  for (i = 0u; i < 7u; i++) {
    to->violated[i] += from->violated[i];
  }
  to->reset_state += from->reset_state;
  to->regs_closed += from->regs_closed;
  to->gmem_closed += from->gmem_closed;
  to->pulldown_set += from->pulldown_set;
  to->halted += from->halted;
  to->faulted += from->faulted;
  if (from->max_level > to->max_level) {
    to->max_level = from->max_level;
  }
  if (from->max_frames > to->max_frames) {
    to->max_frames = from->max_frames;
  }
}

/* One scenario program run through the public call, one unit at a time, until machine time reaches end ticks. */
static int inv_scan_scenario(uint32_t sc, uint64_t end, inv_stats *total)
{
  ivs_ms_inst x;
  inv_stats st;
  const ivs_input_event *ev;
  uint32_t nev = 0u;
  uint32_t consumed = 0u;
  uint64_t unit = 0u;
  int first = 1;
  int ok = 1;
  if (!su_scenario_new(&x, sc, 1)) {
    ivs_test_diag("scenario %s: no instance", su_scenario_name(sc));
    return 0;
  }
  ev = su_scenario_events(sc, &nev);
  memset(&st, 0, sizeof st);
  for (;;) {
    const struct ivs_machine *im = x.m;
    inv_boundary(su_scenario_name(sc), x.m, 0u, &st, unit, ST_FAULT_STRIDE, &first);
    if (im->time_ticks >= end || im->cpu.run_state == IVS_CP1610_RS_FAULTED) {
      break;
    }
    if (!su_run_unit(x.m, ev, nev, &consumed)) {
      ivs_test_diag("scenario %s: the run call refused at unit %llu", su_scenario_name(sc), (unsigned long long)unit);
      ok = 0;
      break;
    }
    unit++;
  }
  ivs_test_diag("scenario %s: %llu unit boundaries to tick %llu, %u events applied of %u, %llu frames", su_scenario_name(sc),
                (unsigned long long)st.boundaries, (unsigned long long)end, (unsigned)consumed, (unsigned)nev,
                (unsigned long long)st.max_frames);
  inv_add(total, &st);
  ivs_ms_free(&x);
  return ok && consumed == nev;
}

/* A program of a stand-in or a hand-built set of words, stepped through the internal seam for units units, or until its CPU faults. */
static int inv_scan_words(const char *name, ivs_ms_inst *x, uint32_t units, uint32_t stride, inv_stats *total)
{
  inv_stats st;
  uint64_t unit;
  int first = 1;
  memset(&st, 0, sizeof st);
  for (unit = 0u; unit <= units; unit++) {
    const struct ivs_machine *im = x->m;
    inv_boundary(name, x->m, 0u, &st, unit, stride, &first);
    if (unit == units || im->cpu.run_state == IVS_CP1610_RS_FAULTED) {
      break;
    }
    if (!ivs_ms_step(x->m, 1u, NULL)) {
      ivs_ms_free(x);
      return 0;
    }
  }
  ivs_test_diag("program %s: %llu unit boundaries", name, (unsigned long long)st.boundaries);
  inv_add(total, &st);
  ivs_ms_free(x);
  return 1;
}

/* The loop-then-HLT and loop-then-fault programs of tests/run/run_slice.c (decles derived there from the vector rows): a loop of 8, then HLT
 * ($0000, the instruction that follows it still executes) or MVO@ R4, R4 ($0264, a sticky guest fault). */
static const uint16_t inv_prog_halt[] = {0x02B9u, 0x0008u, 0x0011u, 0x022Cu, 0x0002u, 0x0000u, 0x0089u};
static const uint16_t inv_prog_fault[] = {0x02B9u, 0x0008u, 0x0011u, 0x022Cu, 0x0002u, 0x0264u};

/* state.restore.invariants-hold-reachable (D-15, D-16): every one of INV-1 to INV-6 holds at every unit boundary of every program above. A
 * fault run takes every 16th boundary and one frame of each scenario (the id is the must-fail id of no fault), a normal run all of them. The
 * counts of boundaries per program are printed (the SUMMARY of plan 08-13 records them), and the case fails when a program shows the reset
 * state other than once, or when it never reached the states that give the invariants something to say. */
static int inv_hold_reachable(void)
{
  static const uint64_t frame = 59736u;
  static const uint64_t frames_pass = 14u; /* 806,568 ticks: past the last of the six events (700,000), also in a fault run (stepping is cheap there) */
#ifdef IVS_STATE_FAULT_RUN
  static const uint64_t frames_lamp = 1u;
  static const uint64_t frames_other = 1u;
  static const uint32_t units_standin = 1500u;
#else
  static const uint64_t frames_lamp = 4u;
  static const uint64_t frames_other = 4u;
  static const uint32_t units_standin = 20000u;
#endif
  inv_stats total;
  ivs_ms_inst x;
  uint32_t sc;
  uint32_t prog;
  uint32_t programs = 0u;
  int ok = 1;
  memset(&total, 0, sizeof total);
  for (sc = 0u; sc < SU_SC_COUNT; sc++) {
    uint64_t frames = sc == SU_SC_LAMP_PASS ? frames_pass : (sc == SU_SC_LAMP_IDLE ? frames_lamp : frames_other);
    ST_REQ(ok, inv_scan_scenario(sc, frames * frame, &total));
    programs++;
  }
  for (prog = 0u; prog < IVS_MS_PROGS; prog++) {
    char name[24];
    (void)snprintf(name, sizeof name, "standin-%u", (unsigned)prog);
    if (!ivs_ms_powered_prog(&x, prog, 0x5Au)) {
      return IVS_TEST_FAIL;
    }
    ST_REQ(ok, inv_scan_words(name, &x, units_standin, ST_FAULT_STRIDE, &total));
    programs++;
  }
  ST_CHECK(su_boot_new(&x, SU_IMG_ALL, 1, 0x5Au));
  ST_REQ(ok, inv_scan_words("boot", &x, units_standin, ST_FAULT_STRIDE, &total));
  programs++;
  ST_CHECK(ivs_ms_powered_words(&x, inv_prog_halt, sizeof inv_prog_halt / sizeof inv_prog_halt[0], 0x5Au));
  ST_REQ(ok, inv_scan_words("halt", &x, 300u, 1u, &total)); /* tiny: every boundary, also in a fault run */
  programs++;
  ST_CHECK(ivs_ms_powered_words(&x, inv_prog_fault, sizeof inv_prog_fault / sizeof inv_prog_fault[0], 0x5Au));
  ST_REQ(ok, inv_scan_words("fault", &x, 300u, 1u, &total));
  programs++;

  ivs_test_diag("%u programs, %llu unit boundaries, %llu checked; first failing invariant: INV-1 %llu, INV-2 %llu, INV-3 %llu, INV-4 %llu, INV-5 %llu, INV-6 %llu",
                (unsigned)programs, (unsigned long long)total.boundaries, (unsigned long long)total.checked, (unsigned long long)total.violated[1],
                (unsigned long long)total.violated[2], (unsigned long long)total.violated[3], (unsigned long long)total.violated[4],
                (unsigned long long)total.violated[5], (unsigned long long)total.violated[6]);
  ivs_test_diag("seen: register gate closed at %llu, graphics gate closed at %llu, a pull-down at %llu, halted at %llu, faulted at %llu boundaries; audio level %u; %llu frames; reset state %llu times",
                (unsigned long long)total.regs_closed, (unsigned long long)total.gmem_closed, (unsigned long long)total.pulldown_set,
                (unsigned long long)total.halted, (unsigned long long)total.faulted, (unsigned)total.max_level,
                (unsigned long long)total.max_frames, (unsigned long long)total.reset_state);
  ST_REQ(ok, total.violated[1] == 0u && total.violated[2] == 0u && total.violated[3] == 0u && total.violated[4] == 0u &&
                 total.violated[5] == 0u && total.violated[6] == 0u);
  /* the reset state is the unit-0 boundary of each program, and only that one; at a stride of 1 every program shows it */
  ST_REQ(ok, total.reset_state == programs);
  /* the invariants had something to say */
  ST_REQ(ok, total.regs_closed > 0u && total.gmem_closed > 0u);
  ST_REQ(ok, total.halted > 0u && total.faulted > 0u);
#ifdef IVS_STATE_FAULT_RUN
  ST_REQ(ok, total.max_frames >= 1u); /* the one-frame window of a fault run still completes a frame */
#else
  ST_REQ(ok, total.pulldown_set > 0u);
  ST_REQ(ok, total.max_level == 15u);
  ST_REQ(ok, total.max_frames >= 13u);
#endif
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_case ivs_state_invariants_cases[] = {
    {"state.restore.invariants-hold-reachable", inv_hold_reachable, NULL},
};
const size_t ivs_state_invariants_count = sizeof ivs_state_invariants_cases / sizeof ivs_state_invariants_cases[0];
