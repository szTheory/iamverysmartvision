---
phase: 06-stic-video-first-pass
plan: 08
subsystem: core/stic, core/machine, core/memmap, tests/stic, cmake, data/profiles
tags: [frame-lines, intrm, busrq, bus-grant, row-latch, late-grant, tie-rule, bounds-reproof, c41, slicing, fuzz, D-08, D-09, D-10, D-11, A-05, A-06, A-09, D-23, VID-02]
status: complete

requires:
  - phase: 06-stic-video-first-pass
    provides: "plan 06-06 event table, catch-up at every unit end, hook classes; plan 06-07 inspection call; gate steps=36/36 controls=331/331 faults=203/203"
provides:
  - "ivs_stic_lines (core/src/stic/stic_lines.c): the pure INTRM and BUSRQ levels from state, sampled by the machine seam one profile lag before every unit start; the hard-wired zero lines are gone, the CPU stepper is unchanged"
  - "BACKTAB row latches: ROW_FETCH(i) copies 20 words through ivs_memsys_stic_backtab_word_internal; grants_in_frame counted at BUS_RELEASED; FRAME_END adds bus_count - grants_in_frame to late_grants"
  - "suite stic 54 to 97 ids, 43 new (lines 2, irq 7, busrq 7, fetch 4, tile 1, steal 2, slice 4, fuzz 9, iso 1, regress 1, bounds 4, judge 1), five faults, generator second refusal, stale statements rewritten"
affects: [06-09, 06-10, 06-11, 06-12, 06-13, 06-14]

actuals:
  tokens: 150000
  tasks: 5
  commits: 4
plan_head_before: 7f9092e534fed92652880c288ad135133dc96d6e
plan_head_after: e8669ebb05a3b746d8b43dcff50c43c0b366b9b6

key-decisions:
  - "The stand-in interrupt programs keep their stack in system RAM ($0300): the scratchpad is 8 bits wide and cut the pushed PC (see deviation 3)"
  - "STIC_GRANT_RELEASE_LATENCY_DROPPED shortens the release by one cycle (the stepper refuses a configuration of 0 cycles, so latency 0 broke every step)"
  - "A halted CPU (one unit per cycle) is the instrument for the sampling lag: the first grant of a window starts exactly one lag after it"
  - "The straddle sweep needs a stream of non-interruptible stores: the window that starts 57 cycles before a row's fetch tick stalls the CPU unless the CPU is inside such a stream"

key-files:
  created:
    - core/src/stic/stic_lines.c
    - tests/stic/stic_lines.c, stic_irq.c, stic_busrq.c, stic_fetch.c, stic_tile.c, stic_slice.c, stic_fuzz.c, stic_iso.c, stic_bounds.c
  modified:
    - core/src/stic/stic.h, stic_events.c, core/CMakeLists.txt, core/src/machine/machine_run.c, machine_bus.c, core/src/memmap/memsys.h, memsys.c
    - cmake/IvsSticTable.cmake, data/profiles/intv-2609-ntsc-family-p1.json, tests/psg/psg_bounds.c, docs/stic/timing-notes.md
    - tests/stic/stic_progs.c, stic_cases.h, stic_main.c, tests/CMakeLists.txt
    - data/expected/tests/stic.ids, data/expected/faults/stic.ids, data/stic/tests-pending.ids

duration: about 8 hours elapsed on a loaded laptop (seven sh gate runs, one dash run, one container run)
completed: 2026-11-12
---

# Phase 6 Plan 08: frame timeline lines, row latches, late grants, bounds re-proof Summary

**The STIC now drives the CPU's INTRM level and BUSRQ windows from the frame schedule through the unchanged stepper: a window starting at tick w resumes the CPU at w + 468 through the real machine (tested for six loop bodies and many grant phases), BACKTAB is latched row by row with the tie rule, late grants are counted exactly, and U = 468, g = 28 and W_log = 2,152 are re-proved through the full machine with the lines live.**

Oracle commit this plan follows: `eae8ca5` (06-01 task 5). First commit of the plan (FIRST): `b5d87ac`.

## What was built

- Task 1, tracer (`b5d87ac`): `ivs_stic_lines`, the seam wiring (sample = start - lag, a unit below the lag sees neither line), stand-ins (a DIS-first counting ISR at the IAB vector, display-on loop bodies, a halted CPU, a stream of non-interruptible stores), a unit-trace helper, 12 ids, faults `STIC_LINE_LAG_IGNORED`, `STIC_IRQ_LEVEL_NEVER_RELEASED`, `STIC_BUSRQ_WHEN_DISPLAY_OFF`, `STIC_GRANT_RELEASE_LATENCY_DROPPED`.
- Task 2 (`a109bd4`): the pure accessor, ROW_FETCH and FRAME_END handlers, grant counting in the seam, `stic.fetch.*` (4), `stic.busrq.late-grant-counter`, judge `stic.judge.late-grant-latched`, fault `STIC_BACKTAB_WRITE_NOT_HOOKED`.
- Task 3 (`fdbf961`): tiling, accounting, the C41 family, slicing, fuzz, isolation, regression (21 ids, no new fault).
- Task 4 (`e8669eb`): bounds re-proof (4 ids), the second generator refusal, stale statements, measured figures, and the stack fix (deviation 3). Task 5 made no commit of its own (the gates and neutrality are below).

## Bounds re-proof through the full machine (measured)

| Quantity | Published | Measured through the machine with INTRM and BUSRQ live |
|----------|-----------|--------------------------------------------------------|
| U, longest unit | 468 ticks | 48 ticks over all 1,024 decles x SDBD off/on x (interrupt level held, window open): 2 x 2,048 guests, 28,305 grant units, 14,340 entries; grant, hold, release at most 8 ticks, interrupt entry 36 ticks; the 468 bound stays (wait cap, proven at the stepper level by psg.bounds.*) |
| g, shortest write gap | 28 ticks | 36 ticks between writes of instruction units in the 12 adversarial writer programs (468 grants, 36 entries, three frames each); any k + 1 writes span at least k g ticks; 28 needs an interrupt-entry write and is proven at the stepper level (unchanged, `psg.bounds.g-and-u` green) |
| W_log | 2,152 records per frame | densest reachable writer (back-to-back non-interruptible MVO@, never stalled): 1,660 records in a span of one frame, display on and display off |
| waits | 0 | no access of any traced unit carries a wait; the wrapper replies 0 for every kind at 100,000+ sampled addresses inside and outside the windows |

What a stall changes: grant, hold and release units are at most 8 ticks, the entry 36, both far below U, and a stall only lengthens the time between two writes, so g and W_log hold. Capacities and goldens did not move.

Other figures: executed instruction cycles of a steady frame of the 12-cycle loop are within 1.0 percent of 13,518 (C95 plausibility, an output); with C41 = 0 and a service routine without DIS the entries of one frame follow the closed form from the entry (9) plus first instruction (6) period, about 253; the late-grant stream (1,355 stores) counts 13 late grants and 0 grants in frame 0, an interruptible loop counts 0.

## Tests and faults

| Fault | Site | Must fail |
|-------|------|-----------|
| STIC_LINE_LAG_IGNORED | machine_run.c | stic.lines.sampling-lag |
| STIC_IRQ_LEVEL_NEVER_RELEASED | stic_lines.c | stic.irq.one-per-frame |
| STIC_BUSRQ_WHEN_DISPLAY_OFF | stic_lines.c | stic.busrq.none-when-display-off |
| STIC_GRANT_RELEASE_LATENCY_DROPPED | machine_run.c | stic.busrq.closed-form-resume |
| STIC_BACKTAB_WRITE_NOT_HOOKED | machine_bus.c | stic.fetch.backtab-straddle (a store swept across row 3's fetch tick; unit starts observed by single stepping, the stamp read from the access record; before, at (the tie) and after the tick all occur) |

Pending lines `stic.lines.`, `stic.irq.`, `stic.busrq.`, `stic.fetch.`, `stic.tile.`, `stic.steal.`, `stic.fuzz.`, `stic.iso.`, `stic.regress.`, `stic.bounds.` were deleted in the commits that registered their first ids. Collateral allowed prefixes of three 06-06 faults (`STIC_WINDOW_OFF_BY_ONE`, `STIC_CATCHUP_LAGS_UNIT`, plus the new ones) were extended as the gate required.

Suite `stic` plain-run time: dev 0.45 s (target 2.0 s), release 0.18 s; slice-1 and fuzz use the thinned comparison (every call within 64 units of a frame event, grant or entry, every 64th elsewhere, final state always); slices 7, 97 (prime) and 468 compare the whole 114,416-byte instance at every call end.

## Deviations from Plan

**1. [Rule 1 - plan text] `STIC_GRANT_RELEASE_LATENCY_DROPPED` drops one cycle, not all.** The stepper refuses a configuration with release latency 0, so a latency of 0 broke every step in every test. The site now makes the machine build `latency - 1` (1 cycle); it fails `stic.busrq.closed-form-resume` and `stic.busrq.halted-grants` and `stic.steal.accounting` only.

**2. [Rule 1 - plan text] The sampling-lag perturbation is a pure-function relation.** The lag lives in a const generated table, so a machine run cannot perturb it; the perturbed lags 0..3 cycles are checked through `ivs_stic_lines` with the seam's rule (first seen boundary = window start + lag) and the real seam is checked with a halted CPU. The same holds for `stic.busrq.hold-from-profile`, which drives the real stepper with the line function of a stack copy of the parameters.

**3. [Rule 1 - defect in my stand-ins, found late] The stack of the interrupt stand-ins was in the 8-bit scratchpad.** The pushed PC lost its high byte, so after the first service routine the CPU ran garbage near $0014 and the loop bodies were never executing in the Task 1 to 3 commits; the closed-form tests passed anyway because the lines are driven by the schedule and the garbage loop is interruptible. Found by the Task 4 PSG writer (one write in three frames). Fixed in the Task 4 commit (the stack is $0300 in 16-bit system RAM); every earlier test was re-run with the real bodies and passed unchanged, including the closed form for six bodies and the executed-cycle plausibility check.

**4. [Rule 3 - interpretation] The late-grant stream, the straddle and the iso/regress cases needed larger guests than 24 words,** so `sc_guest_n` builds up to 4,096 words; derivations stay in `stic_cases.h`.

**5. [Interpretation] `stic.regress.guest-without-eis-or-strobe-unchanged` replicates the pre-lines seam in the test** (the stepper driven with all four lines 0 through the bare machine wrapper) instead of a literal taken from the 06-06 tree; the four Phase 4 and 5 goldens and the consumer golden, which pin the same unit sequences and CPU digests, did not move.

**6. [Interpretation] Distinct loop body lengths are 6, 7, 8, 9, 10 and 12 cycles** (INCR, TSTR R6, MVII, taken branch, MVI, J); 4 and 5 are not interruptible lengths and 11 has no single interruptible instruction without side effects. Boundaries are not uniform for bodies with a branch back; at least five distinct grant phases are asserted.

**7. [Rule 3] `STIC_MAX_SECTIONS` in `stic_main.c` raised 16 to 32** (17 case families).

## Neutrality

`phase 6 does not move any phase 5 timing`: `git status --short data/golden data/scenarios data/expected/pkg-consumer.golden` is empty and `git diff --stat b5d87ac^..HEAD` on those paths prints nothing; the consumer and the runner golden still show video not observed with equal `digest.*` lines.

## Open Items

- VID-02 is NOT marked complete: plans 06-13 and 06-14 list it, so the closing plan decides; the evidence of this plan is the closed form through the real machine, the exact late-grant accounting and the bounds re-proof.
- The community aggregate 13,518 is only a plausibility check (C95); the grant phase coverage is whatever the six bodies reach (asserted at least five distinct phases).
- g = 28 itself is not reachable through the machine in Phase 6 (the interrupt-entry write goes to RAM); the stepper-level probe still proves it.
- Carried unchanged: `ivs_info` v4, report v3 and the env slack word (plan 06-09), the 06-01 second-read records, the `run.refuse.flags-frame-reserved` inversion (06-09), domain 16 value init (06-10), the Phase 4 and 5 deferrals, and `sh tools/hygiene.sh --history` on earlier planning commits.
- The container gate ran natively on arm64 (hosted CI covers x86).

## Final gate

```
sh tools/gate.sh:       gate: PASS steps=36/36 controls=331/331 ... api-calls=15 ... faults=208/208 ... profile-keys=143 ... tests-expected=4088 tests-executed=4086 tests-skipped=2 tests-missing=0   (gate-time total=69)
dash tools/gate.sh:     the same summary line (identical bytes)                                                                                                                              (gate-time total=75)
sh tools/gate-linux.sh: GATE_LINUX_PLATFORM= (native arm64 container), exit 0, last line of build/gate-linux.log starts gate: PASS, cc=GNU, steps=36/36 controls=331/331 api-calls=15 faults=208/208 tests-missing=0   (gate-time total=299)
```

## Known Stubs

None. `ivs_stic_catch_up` still takes a NULL sink until plan 06-09.

## Threat Flags

None new. T-06-10 (accepted): every Phase 6 stand-in service routine starts with DIS; `stic.irq.level-held-reentry-c41-0` and `stic.irq.c41-both-values` record the consequence. T-06-11 (mitigated): `stic.bounds.*` through the full machine, the second generator refusal (scratch run with latency 200 refuses with `IvsSticTable: bounds: ...`), capacities and goldens unchanged. T-06-SC: no installs.

## Self-Check: PASSED

Created files exist (`core/src/stic/stic_lines.c`, nine `tests/stic/stic_*.c` family files); commits `b5d87ac`, `a109bd4`, `fdbf961`, `e8669eb` are ancestors of HEAD; the gate lines above are from the tree of `e8669eb` plus this SUMMARY.
