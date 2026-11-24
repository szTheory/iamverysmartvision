---
phase: 08-stic-completion-save-state-performance-baselines
plan: 14
subsystem: Phase 8 wave B, plan 6 - the hard-point suite of the save state - 18 named cuts found by a witness scan and pinned, the poisoned-target comparison oracle against the uninterrupted run, the designed latch program, the writer-side STIC fault, the fixed point and canonical snapshot literals, slicing independence and the runtime row-omission sweep with its exemption notes
tags: [B, STA-02, hard-points, witness-scan, poisoned-target, row-sweep, exemptions, canonical-form, D-12, D-16, D-17, D-40, D-44, T-08-34, T-08-35]
status: complete

requires:
  - phase: 08-stic-completion-save-state-performance-baselines
    provides: "plan 08-13 end state (restore, IVS_E_STATE_MISMATCH, state_util.c builders, poison and digests, the invariants); counts at plan start steps=38/38 controls=378/378 faults=234/234 tests-expected=4864 tests-executed=4860 tests-skipped=4 (the first gate of Task 1 read 4870/4866, the start plus the 6 ids of that task)"
provides:
  - "tests/state-cuts-inc.cmake (forms cuts, checklist, notes) and the generated includes state_cuts.inc, state_checklist.inc and state_notes.inc: the pinned ticks, the STATE rows with witness and sweep column, and the appended sweep notes reach the tests through generated includes, never retyped"
  - "tests/state/state_progs.c (the five stand-in programs P-SDBD, P-HLT, P-FAULT, P-LATCH, P-PSG, the run table, the 18 cuts, the end tick of each reference), state_witness.c (the scan and the 18 predicates), state_cut.c (legs, the cached reference, the comparison oracle, the cut ids, the fixed point and slicing ids, the latch discriminator) and state_sweep.c (the row-omission sweep); 12 test ids (4864 to 4888) and one fault, SNAPSHOT_STIC_ROWS_OMITTED (234 to 235)"
  - "data/expected/state-cuts.ids with all 18 ticks pinned (no pending token), three snapshot SHA-256 literals in data/expected/ivsn-literals.ids, data/expected/state-sweep-notes.ids (the appended notes of the frozen checklist: 3 witness amendments, 9 exemptions), the as-built sections of docs/state/hard-points.md and docs/state/hidden-state-checklist.md"
affects: [08-15, 08-16, 08-17]

actuals:
  tokens: 34852
  tasks: 3
  commits: 3
plan_head_before: 5d56e2128943ea79e1369da7c1c81059231e8054
plan_head_after: 22215de8a8f71375568bdb1f329306303910c839

key-decisions:
  - "The comparison oracle is table-free and independent of the shared field table: REF is the uninterrupted run from a fresh instance to E; SUB restores into a target whose STATE and PAD rows hold 0xA5 and compares at once the whole instance (memcmp) and the nine digests with the source, continues with fresh buffers and events resubmitted from the applied count and compares the concatenated video, audio and PSG log streams, the first-index fields, the run reports, the final instance, the nine digests and the public reports with REF"
  - "The sweep is judged against a checklist that stays frozen: the witness amendments and the exemptions are appended notes (data/expected/state-sweep-notes.ids) that the sweep reads after the checklist and judges itself; an edit of data/expected/state-checklist.ids was refused by the permission system and contradicts the freeze of the oracle data, so the plan's edit of that file was not made (deviation 2)"
  - "The sweep value is the byte-wise complement first, then other members of the set; each candidate is judged by the validators and invariants of the restore on a probe of the modified instance, candidates that pass all six rules first, then those that break only a derived rule (or lie in the range of an envelope row whose relation to a neighbour the validator enforces); divergence in behaviour ends the search, a divergence only in a public report is noted and the search continues"
  - "ROW needs a divergence (behaviour or a public report) and a row that is not default at its cut; EXEMPT needs no divergence in behaviour (a report that only echoes the row is allowed); the guard compares with the power-on row, and for phase and for every row at tick 0 (the power-on state itself) with the zero row of a fresh target"
  - "The sweep found weak witnesses and corrected the cuts, not the oracle: HP-STEAL-A is the boundary before the first bus grant (74,928) and HP-STEAL-B the boundary before a release (79,008), P-PSG uses a triangle envelope (shape $0A) so that psg.env_dir is non-default (HP-ENV-MID 9,504); the corrections are appended to docs/state/hard-points.md"
  - "Fault-run cost rule held: under IVS_STATE_FAULT_RUN every id that is not the must-fail id of a fault takes every 4th cut (every 16th row for the sweep) and a one-frame window; state.cut.stic-points always runs the full set; the nine fault runs sum to 6.7 local cpu-s"

key-files:
  created:
    - tests/state-cuts-inc.cmake
    - tests/state/state_progs.c
    - tests/state/state_witness.c
    - tests/state/state_cut.c
    - tests/state/state_sweep.c
    - data/expected/state-sweep-notes.ids
    - .planning/phases/08-stic-completion-save-state-performance-baselines/08-14-SUMMARY.md
  modified:
    - core/src/machine/machine_digest.c
    - tests/CMakeLists.txt
    - tests/state/state_cases.h
    - tests/state/state_literal.c
    - tests/state/state_main.c
    - tests/state/state_util.h
    - data/expected/tests/state.ids
    - data/expected/faults/state.ids
    - data/expected/state-cuts.ids
    - data/expected/ivsn-literals.ids
    - data/state/tests-pending.ids
    - docs/state/hard-points.md
    - docs/state/hidden-state-checklist.md
    - .planning/STATE.md
    - .planning/ROADMAP.md

requirements-completed: []

completed: 2026-11-14
---

# Phase 8 Plan 14: the hard-point suite Summary

**Snapshot, restore and run now equals the uninterrupted run at the 18 named hard points, shown against poisoned targets with oracles that do not read the shared table. A witness scan over the step seam finds, for each cut, the first unit boundary at which its predicate holds, and the 18 ticks are pinned in `data/expected/state-cuts.ids` and re-proved by three witness ids (the scan's first occurrence equals the pin, a fresh instance run to the pin through the public call ends exactly there with the scan's bytes, the instance part of the predicate holds). For each cut the comparison restores the snapshot into an instance whose STATE and PAD rows hold `0xA5`, compares the whole instance and the nine digests at once, continues to the end tick with fresh buffers and resubmitted events, and compares the concatenated video, audio and PSG log streams, the first-index fields, the run reports, the final instance, the nine digests and the public reports with the uninterrupted run. The designed latch program discriminates a restore that recomputes `stic.row_latch` (8 of 15,296 bytes of the first slot after the cut differ), a self-control shows the comparison can fail, and the new fault `SNAPSHOT_STIC_ROWS_OMITTED` is caught by `state.cut.stic-points`. The snapshot at every cut is a fixed point, equal for an independently built source and independent of how the run to the cut was sliced, and the SHA-256 of the snapshot at three cuts is pinned and identical in the dev and release presets. The runtime row-omission sweep puts each of the 51 STATE rows back at its witness cut with other values: 39 rows diverge in behaviour, 3 in a public report only, 9 are exempt with an argument each, and a judge shows the sweep can report both. The sweep found three weak witnesses and a program that could not witness a row; the cuts were corrected (three predicates and one program word) and the frozen checklist was left untouched, the amendments being the appended notes `data/expected/state-sweep-notes.ids`. The gates pass under sh, dash and the Ubuntu container with the plan-end counts `steps=38/38 controls=378/378 faults=235/235 fault-runs=235 tests-expected=4888 tests-executed=4884 tests-skipped=4 tests-missing=0 api-calls=19 public-symbols=19 state-rows=88 state-cuts=18`. STA-02 stays Pending until plans 08-16 and 08-17 close it.**

## Commits

| Commit | Task | What |
|--------|------|------|
| `01bf8da` | 1 (tracer) | the cut generator, the three cpu programs, the 18 cuts and end ticks, the scan with the eleven cpu predicates, the legs, the cached reference and the comparison oracle, `state.witness.cpu-points`, `state.cut.cpu-points`, the judge `state.judge.cut-compare-detects-divergence`, the eleven pinned ticks (ids 4864 to 4870) |
| `0686241` | 2 | P-LATCH and P-PSG, the seven stic and device predicates and pins, `state.witness.stic-points`, `state.witness.device-points`, `state.cut.stic-points`, `state.cut.device-points`, `state.cut.latch-not-recomputed`, the fault SNAPSHOT_STIC_ROWS_OMITTED (ids 4870 to 4880, faults 234 to 235) |
| `22215de` | 3 | `state.cut.fixed-point-and-canonical` and the three snapshot literals, `state.cut.slicing-independent`, the sweep and its judge, the sweep notes, the three cut corrections with their re-pinned ticks, the as-built sections (ids 4880 to 4888) |
| the commit that adds this file | | this SUMMARY, STATE.md and ROADMAP.md |

## The cuts: pinned ticks, end ticks, units stepped

Found by the scan (`state.witness.*`), written into `data/expected/state-cuts.ids`, self-recorded repeatability pins. Units are the units from the reset to the cut plus the units of the continuation (leg 1 + leg 2 of the comparison; REF steps the sum).

| Cut | Program | Tick | End tick E | Units (to the cut + after) | Arithmetic cross-check |
|-----|---------|------|-----------|----------------------------|------------------------|
| HP-SDBD | P-SDBD | 112 | 1,000 | 4 + 25 | 28 cycles (5 + 8 + 11 + 4) times 4 ticks: 112 exactly |
| HP-STEAL-A | lamp-idle | 74,928 | 238,944 | 2,255 + 8,848 | first window at cycle 3,791 (tick 74,900), one cycle of line lag, the grant is the first unit that starts at 74,904 or later (it starts at 74,928) |
| HP-STEAL-B | lamp-idle | 79,008 | 238,944 | 2,580 + 8,523 | second window 78,548, hold 114 cycles: BUSRQ falls at 79,004, the release is the unit that starts at 79,008 |
| HP-STEAL-C | lamp-idle | 79,016 | 238,944 | 2,581 + 8,522 | the release takes 2 cycles: 79,008 + 8 |
| HP-OPEN | lamp-idle | 59,772 | 238,944 | 1,750 + 9,353 | frame 1 starts at 59,736, INTRM is seen 4 ticks after: the first open boundary with the enable set |
| HP-CLOSED | lamp-idle | 59,900 | 238,944 | 1,753 + 9,350 | the entry (59,808) plus the jump (12 cycles) and the strobe MVO (11 cycles): 92 ticks |
| HP-INT-ENTRY | lamp-idle | 59,808 | 238,944 | 1,751 + 9,352 | 59,772 plus the entry, 9 cycles = 36 ticks |
| HP-HLT | P-HLT | 372 | 70,000 | 12 + 17,386 | 93 cycles, 372 exactly |
| HP-HALTED | P-HLT | 15,168 | 70,000 | 3,703 + 13,695 | window of frame 0 at cycle 3,791 (tick 15,164) plus one cycle of lag |
| HP-FAULTED | P-FAULT | 540 | 1,540 | 19 + 0 | 135 cycles, 540 exactly (19 units: reset entry, MVII, 8 DECR, 8 branches, the fault unit) |
| HP-RESETPEND | none | 0 | 119,472 | 0 + 4,867 | the boundary before any unit |
| HP-FRAME-MID | lamp-idle | 96,652 | 238,944 | 3,545 + 7,558 | frame cycle 9,229: after the row 5 fetch (8,408) and the re-arm EIS (9,186 to 9,597) |
| HP-LATCH | P-LATCH | 75,456 | 238,944 | 3,232 + 7,706 | frame cycle 3,930: the row 0 fetch at 3,848 plus within one 70-cycle pass of the loop |
| HP-MOB | diag-stic-2-collision | 153,104 | 238,944 | 6,499 + 4,600 | frame 2 cycle 8,408: exactly the row 5 fetch, the first mid-frame boundary after a frame end that wrote collision bits |
| HP-PSG-MID | P-PSG | 66,336 | 238,944 | 1,976 + 4,795 | step 4,145 of the chip (tick 66,320), see the finding below |
| HP-ENV-MID | P-PSG | 9,504 | 119,472 | 312 + 3,140 | the rising half of the triangle envelope after 16 falling levels (scan; no closed form) |
| HP-INPUT | lamp-pass | 100,064 | 238,944 | 3,741 + 7,351 | the first event at 100,000 applies at the unit starting at 100,028; the idle unit is 36 ticks |
| HP-LOG | lamp-pass | 25,920 | 119,472 | 810 + 4,057 | the first PSG register write of the initialisation, before the first EIS and tick 26,500 |

The end ticks follow the rule of the plan (a halting program: the halt plus idle ticks to past the frame end; the fault program: the faulted boundary plus 1,000; the lamp runs: two frames past the cut, here four frames in all) and are in `tests/state/state_progs.c` with their arithmetic. No cut was added: the cut set is the 18 of the plan.

Findings of the scan (a predicate that does not hold where the document expected it, or an expectation that was wrong):
- All eighteen predicates hold in their programs. No predicate failed to occur.
- **HP-PSG-MID** was expected near tick 13,000 ("within a few chip steps" of the second period rewrite). The scan finds tick 66,336. The reason is the tone convention (a counter is empty at the reset and is loaded on the first chip step that finds it empty): the first load reads zero periods and gives the period-zero value 4,096, so all three counters run down for 4,096 steps (tick 65,536) before they reload from the registers, which by then hold the second pair of periods; the outputs of A and B first differ 49 steps later. The program is as designed; only the arithmetic of the expectation did not apply the rule. Appended note in `docs/state/hard-points.md`.
- **HP-STEAL-A, HP-STEAL-B, HP-ENV-MID** were redefined after the sweep (see "The sweep" below); the cuts and programs that moved are listed with their reasons in the appended corrections.

## The comparison oracle and its results

`state.cut.cpu-points` (11 cuts), `state.cut.stic-points` (3, the must-fail id of the fault, always the full set), `state.cut.device-points` (4): every cut agrees on every comparison: the whole target instance and the nine digests equal the source right after the restore, the concatenated video streams (15,296 bytes per slot), audio streams (4 bytes per sample) and PSG log streams (16 bytes per record) of the two legs equal the uninterrupted run byte for byte, the first-index fields of leg 2 are the counts of leg 1, the run reports (time, units, stop reason, events applied, controller words, input floor) equal, and the final instance, the nine digests and the CPU, STIC and info reports equal. Per-cut checks add that the cut is the state its predicate names: a video slot completes after the restore at HP-FRAME-MID, HP-LATCH and HP-MOB, MOB registers and a sticky collision bit are set at HP-MOB, one event is applied and a pull-down is in force at HP-INPUT, the log has records before the cut at HP-LOG, audio samples fall on both sides at HP-PSG-MID and HP-ENV-MID. The reference of a run is built once and cached.

`state.judge.cut-compare-detects-divergence` runs the comparison at HP-INT-ENTRY three ways: untouched (mask 0), with one byte of the STATE row `time_ticks` changed after the restore (the immediate memcmp, the immediate digests, the final memcmp, the final digests, the reports and the run report all report it: mask 0x3C3) and with a byte of leg 2's video stream flipped (only the video comparison reports it: mask 0x004).

`state.cut.latch-not-recomputed` restores the snapshot of HP-LATCH twice into poisoned targets: the real restore equals the source and continues to the reference (the first video slot after the cut is the reference's byte for byte), and the variant, in which `stic.row_latch` is rebuilt from the twelve BACKTAB rows, renders the first slot differently in 8 of 15,296 bytes (the first pixel row of card 0, whose foreground color is the toggled value). The test fails if the variant equals the reference: the program discriminates a restore that recomputes the latch.

`state.cut.fixed-point-and-canonical`: save, restore into a poisoned target and save gives the same 4,299 bytes at every cut; an independently built source run to the cut in slices of 468 ticks gives the same bytes; the SHA-256 of the snapshot at three cuts equals the pinned literals (self-recorded, repeatability only), identical in the dev and release presets:

| Literal | SHA-256 of the 4,299 bytes |
|---------|-----------------------------|
| `cut-snapshot-sha256.hp-frame-mid` | `9dc4ae7e20a746b4b8d6638cb14305ce3a8d4ec9bd4da872916353272c68567f` |
| `cut-snapshot-sha256.hp-latch` | `63507cb48fa4bbf42cfebb99e07fa5d722db44797641c3404118c1a9826cee1d` |
| `cut-snapshot-sha256.hp-mob` | `27e98b02b605d6f201d2745ebf35c3e2a137d62cc9d892b9d0eb7322f09bc768` |

`state.cut.slicing-independent`: HP-FRAME-MID, HP-LATCH, HP-MOB and HP-INPUT, run in calls of 1, 7, 468 and 4,194,304 ticks and in calls of pseudo-random length from the fixed seed `0x2545F4914F6CDD1D`: the whole instance and the snapshot at the cut equal the run in one call. A call ends on the first unit boundary at or after its target and the last target of each sliced run is the cut tick, a unit boundary, so every sliced run ends exactly on it (the cut tick is the first boundary at or after the target).

## The fault

| Fault | Site | Must-fail id | Must-pass id | What the fault does |
|-------|------|--------------|--------------|---------------------|
| SNAPSHOT_STIC_ROWS_OMITTED | `core/src/machine/machine_digest.c`, the state-only filter of the walker (one site, `#ifdef`-hidden) | `state.cut.stic-points` | `state.witness.stic-points` | the state-only walk skips `mem.stic_reg` and the `stic.` rows, so the sections of a snapshot lack them: the writer's total no longer equals the derived total and the restore refuses the snapshot of the cut |

In a fault run 42 of the 45 ids that existed then fail (all with the prefix `state.`, the reader and writer ids as collateral) and `state.witness.stic-points` stays green. The grep of the acceptance criterion prints 1 for `machine_digest.c` and the id occurs once in `core/src`.

## The row-omission sweep

51 STATE rows at their witness cuts, 0.29 s for the whole id on the dev preset (process start included), no window reduction in either preset (the whole window to E in both; the reduced set exists only under the fault-run definition). Per row: guard, then sweep values until one changes the continuation.

| Family | Rows | Divergence in behaviour | In a public report only | Exempt |
|--------|------|-------------------------|-------------------------|--------|
| instance (`phase`, `time_ticks`) | 2 | 2 | 0 | 0 |
| CPU | 9 | 8 | 1 (`cpu.fault_reason`) | 0 |
| memory system | 7 | 7 | 0 | 0 |
| PSG | 10 | 9 | 0 | 1 (`psg.env_level`) |
| STIC | 11 | 7 | 2 (`stic.blocked_reads`, `stic.blocked_writes`) | 2 (`stic.late_grants`, `stic.cs_ptr`) |
| observation rows | 12 | 6 | 0 | 6 (four chains, `obs.input.count`, `obs.input.words`) |

Rows that differ in a public report only keep ROW (a faulted CPU's reason and the two saturating counters of blocked accesses are read by nothing but the reports; their witness cuts hold non-zero values).

**The exemption list (9; the plan expects at most 11), each with its argument and the cut at which the sweep found it inert (3 to 6 safe sweep values tried, no change in behaviour):**

| Row | Cut | Argument |
|-----|-----|----------|
| `obs.video.chain` | HP-FRAME-MID | h(n+1) is the SHA-256 of the previous value and the new frame record; nothing but the VIDEO digest and its own next value reads it |
| `obs.audio.chain` | HP-PSG-MID | the same over the level-change records |
| `obs.input.chain` | HP-INPUT | the same over the applied events |
| `obs.psglog.chain` | HP-LOG | the same over the PSG register writes |
| `obs.input.count` | HP-INPUT | incremented by every applied event and read by nothing; the host resubmits from the applied count the report of its own earlier call gave |
| `obs.input.words` | HP-INPUT | every event carries the whole controller state and overwrites the four words; the pull-downs the guest reads are a separate row (`mem.port_pulldown`, ROW) and only the run report echoes the words |
| `psg.env_level` | HP-ENV-MID | a function of the cycle index and the direction (the validator enforces it), recomputed at every envelope step before it is used |
| `stic.cs_ptr` | HP-LATCH | assigned by every render before anything reads it |
| `stic.late_grants` | HP-STEAL-C | a count that the frame end adds to and nothing reads; zero at every one of the 18 cuts, so no cut witnesses it non-default |

Each exempt row is covered by the poisoned-target equivalence, the whole-instance and digest equality of every cut comparison and, for the chains, the digest of the domain. The arguments are appended to `docs/state/hidden-state-checklist.md`.

**Witnesses the sweep found inert and the corrections** (each cut is still the witness of at least one row; the corrections of the cuts are appended to `docs/state/hard-points.md`):
- `cpu.intff`: HP-FRAME-MID to HP-OPEN (a lost enable and the frame's interrupt is not accepted: the display is not enabled for that frame).
- `cpu.prev_interruptible`: HP-OPEN to HP-STEAL-A, now the boundary before the first bus grant. At HP-OPEN a boundary closed by mistake takes the interrupt one idle unit late, which the idle loop (units of 36 ticks) cannot show; before a grant a closed boundary does not test BUSRQ and the run report counts other units.
- `cpu.busak`: HP-STEAL-A to HP-STEAL-B, now the boundary before a release. After the grant a lost BUSAK is harmless (BUSRQ is still asserted, the grant is taken again).
- `psg.env_dir` could not be non-default in P-PSG: shape 8 only falls. The shape word is now `$0A` (a triangle), HP-ENV-MID requires the rising direction (tick 9,504).

Guard notes: `phase` and `mem.gate_open` equal their power-on rows and are non-zero (held to the zero row of a fresh target), `psg.env_held` is zero at its cut and differs from its power-on value 1, `stic.late_grants` is zero and equal to its power-on row (an exempt row).

## What each new test can fail on (mutation probes, run in the working tree and reverted)

| Probe | Failing ids |
|-------|-------------|
| `machine_snapshot_restore.c`: the commit rebuilds `stic.row_latch` from the BACKTAB words | roundtrip-fixed-point, poisoned-target-equals-source, same-profile-any-instance, sets-powered, digest-fields-informational, `state.cut.cpu-points`, `.stic-points`, `.device-points`, `.latch-not-recomputed`, `.fixed-point-and-canonical`, `state.sweep.row-omission` (its correct restore no longer equals the source) |
| `cp1610.c`: `IVS_INT_ENTRY_CYCLES` 9 to 10 | `state.witness.cpu-points` (names HP-INT-ENTRY: the pinned tick 59,808 is not the first occurrence 59,812, and HP-CLOSED and HP-STEAL-A likewise), `state.cut.cpu-points`, `state.cut.fixed-point-and-canonical`, `state.judge.cut-compare-detects-divergence`, `state.sweep.row-omission` |
| the fault SNAPSHOT_STIC_ROWS_OMITTED | `state.cut.stic-points` and the 41 other `state.` ids that need a valid snapshot; `state.witness.stic-points` stays green |

Self-controls: `state.judge.cut-compare-detects-divergence` (a changed STATE byte is seen by the immediate memcmp, the digests, the final memcmp and the final digests; a flipped stream byte by the stream comparison) and `state.judge.row-sweep-detects-omission` (`cpu.r` at HP-SDBD is divergent, an exempt row is inert).

## Deviations from Plan

**1. [Finding, corrected in an appended note] The sweep showed three weak witnesses and a program that could not witness a row.** HP-STEAL-A (after the first grant) witnessed `cpu.busak`, which is inert there; HP-STEAL-B (the first mid-hold unit) was the witness of `stic.late_grants`; HP-OPEN witnessed `cpu.prev_interruptible`, inert on an idle loop; P-PSG's shape 8 leaves `psg.env_dir` at 0. The plan's own rule is followed (a predicate or program is fixed in an appended note and reported, a witness is corrected): HP-STEAL-A and HP-STEAL-B are redefined as the boundaries before the first grant and before a release (ticks 74,928 and 79,008, re-pinned; HP-STEAL-C, 79,016, is unchanged), P-PSG's shape word is `$0A` and HP-ENV-MID requires the rising direction (tick 9,504). Task 1 and Task 2 had pinned the old ticks; Task 3 re-pinned them in its commit, so every commit's tree is self-consistent (each task's gate was green at its own commit).

**2. [Interpretation, forced by the freeze and a refused edit] The checklist `data/expected/state-checklist.ids` was not edited; the exemptions and the witness amendments are the appended notes `data/expected/state-sweep-notes.ids`.** The plan's Task 3 asks for `EXEMPT:` entries and corrected witnesses in the checklist file, and the orchestrator's instruction freezes the oracle data (the checklist among it: "change only by an appended note"). An edit of the checklist file was attempted and refused by the permission system, so it was not repeated by another route. The notes file is data in the same style (`<row> <witness cut> <sweep>`), is read by the sweep after the checklist (a note replaces the witness and the sweep column of its row) and is judged by the sweep test: a note must name a STATE row of the checklist and a cut of the cut file, an exemption needs a reason of at least three words, and after the notes every cut must be the witness of at least one row. Consequences, stated plainly: `grep -c 'EXEMPT:' data/expected/state-checklist.ids` prints 0, not the 9 of this SUMMARY (the acceptance criterion of Task 3 is not met in its literal form, the 9 are in `data/expected/state-sweep-notes.ids`); the pure gate step `state` reads the checklist only, so it does not see the notes (its checks on the checklist, including that every cut is a witness, still hold with the checklist as it was; the notes are judged at run time by `state.sweep.row-omission`); and the awk check of Task 3 (at least 40 ROW lines of the checklist) is trivially met by the 51 unchanged ROW lines. If the freeze is lifted, folding the notes into the checklist is one mechanical commit (move the witness and the sweep column, delete the notes file and its generator form).

**3. [Plan text against reality] The must-pass id of the fault is `state.witness.stic-points`, not `state.read.describe-refuse-args`.** The plan named the latter because it "does not depend on the section lengths"; it builds on a valid snapshot of the writer and fails under this fault with the rest of the reader ids. The witness id of the same cuts reads no snapshot and stays green.

**4. [Interpretation] The literal keys are lower case** (`cut-snapshot-sha256.hp-frame-mid`): the literal generator (`tests/ivsn-literals-inc.cmake`) accepts lower-case keys only and is not in the plan's file list. The count check of the plan (3 lines starting `cut-snapshot-sha256.`) holds.

**5. [Interpretation] The guard compares with the power-on row, with the zero row where the power-on row is non-zero by nature** (`phase`, and every row at tick 0), and ROW accepts a divergence in a public report. The plan's rule (the row differs from the power-on state) cannot hold for `phase` (POWERED at every cut) or for a row at the reset cut, so the zero row of a fresh target stands there; and the three rows that only a report reads (`cpu.fault_reason`, `stic.blocked_reads`, `stic.blocked_writes`) are ROW because the plan lists the public reports as divergence. An EXEMPT row may differ in a report that only echoes it (`stic.late_grants`), but not in behaviour.

**6. [Information] The sweep value list is longer than the narrative's.** The first candidate is the complement; further candidates (the lowest bit flipped, the fill, zero, the first or last element, every small value of a scalar) follow when the complement is invalid or inert, every one judged by the restore's validators and invariants first (the open item of plan 08-13). Four envelope rows take a value inside their range (a level, cycle, direction or hold flag below 16 or 2) because the validator also enforces their relation to a neighbour and no single-row replacement passes it.

**7. [Information] Files beyond the plan's lists:** `tests/state/state_literal.c` (three added `#include` lines so that the result file carries the `# source` lines of the cut, checklist and notes files), `data/expected/state-sweep-notes.ids` (new). `tests/state/state_util.c` was not changed. No core file changed in Task 1 (the filter prints 0); the only core change of the plan is the fault site of Task 2.

**8. [Information] The container gate of Tasks 1 and 2 ran concurrently with the next task's work and with the dash gate; the Task 3 container gate ran while the sh and dash gates ran.** Timings below are therefore higher than isolated ones.

No auth gates, no package installs (T-08-SC), no `gh` call, no push, no tag, no `tools/ci-record.sh`; `.planning/config.json`, `.planning/state.json` and `.planning/ci-runs.tsv` were never staged. D10 and D11 are not performed and no claim above provisional exists; every pin and literal is a repeatability pin.

## Decisions Made

- The comparison oracle owns no table: instance bytes, streams, run reports and public reports of an uninterrupted run are the expectation; the field table is used only to poison and to place a sweep value (offset, width, count).
- A cut is found by a scan and proved by a witness id, never retyped: the pins reach the tests through a generated include and a drifting literal fails the witness id and names the cut and the rule.
- A weak witness is a finding about the cut, not about the oracle: the cuts and the P-PSG program were corrected, the frozen checklist was left alone and the amendments are appended notes.

## Cost (laptop)

| Item | Value |
|------|-------|
| estimate of the plan | 170,000 tokens (raw, confidence low), 3 tasks |
| actuals | 34,852 tokens (chars/4 over the added lines of the 3 commits outside `.planning/`), 3 tasks, 3 commits (measured from the ledger of the plan head) |
| gate total, sh and dash, Task 1 tree | 97 s and 95 s, identical summary line (md5 80f78e70aa4607d380cf807bd7fc86be) |
| gate total, sh and dash, Task 2 tree | 107 s and 128 s, identical summary line (md5 98e9d0f8c976d2c6e33de70ee34a547e) |
| gate total, sh and dash, Task 3 tree | 146 s and 153 s (the container gate was running at the same time), identical summary line (md5 43a0eb908478c3702a9dc5d5ba2a455e) |
| container gate (gcc 13.3.0, cmake 3.28.3, x86_64) | Task 1 tree 323 s, Task 2 tree 363 s, Task 3 tree 468 s (run while the sh and dash gates ran), all exit 0, 0 FAIL lines |
| suite `state` (49 ids) | dev preset (sanitized, `-O1`) 2.5 to 2.7 s user plus 0.15 s sys, release 1.25 to 1.4 s (before the plan: 2.17 and 1.05 s for 37 ids); the new ids together about +0.5 s dev and +0.25 s release; no window reduction |
| heaviest new ids, dev | `state.cut.*` (6 ids and the judge) 0.40 s with the 0.13 s process start, `state.sweep.row-omission` 0.29 s with it, `state.witness.*` 0.03 s |
| fault runs, unsanitized `-O1`, the whole suite once each | SNAPSHOT_LITTLE_ENDIAN 0.69, PAD_SERIALIZED 0.52, IMAGE_BYTES_EMBEDDED 0.23, TRAILER_UNCHECKED 0.96, LENGTH_TRUSTED 0.95, RESTORE_PROFILE_UNCHECKED 0.92, RESTORE_IMAGE_DIGEST_UNCHECKED 0.90, RESTORE_MUTATES_BEFORE_VALIDATE 0.95, SNAPSHOT_STIC_ROWS_OMITTED 0.53: sum 6.7 local cpu-s (the eight fault runs of plan 08-13 summed 5.87, so the reduced point sets of the new ids cost about 0.3 s over all eight) against the target of about 6 |

Cost stop rule (checkpoint B entry, `.planning/CI-EFFICIENCY.md`): the additions of this plan are the 12 ids (+0.5 s dev, +0.25 s release, 0.8 local cpu-s, 1.3 hosted at the factor 1.675), one fault (0.53 s of fault run, 0.9 hosted, and about 1.0 for its build at the unit cost of the record) and no step or control, about 3.2 hosted cpu-s in all; at the hosted `build_s` of 40 to 63 that is +0.05 to +0.08 in `r`, inside the 3.5 hosted cpu-s the checkpoint estimated for plans 08-14 to 08-17. On the recorded median 7.98 the projection is about 8.03 to 8.06, far from the line 8.3, so the stop rule did not fire and no test case was withheld. The orchestrator's recorded row 43658364307 (r = 7.28) is a normal-speed row and was not used to lower the projection.

## Open Items

- **Plan 08-17 (registry and close-out):** the exemption list of this plan is in `data/expected/state-sweep-notes.ids` (9 lines with their arguments in `docs/state/hidden-state-checklist.md`) and not in the frozen checklist file; the close-out note should cite the notes file. The registry lines named in the plan of 08-17 exist: the fault `SNAPSHOT_STIC_ROWS_OMITTED` and the judges `state.judge.cut-compare-detects-divergence` and `state.judge.row-sweep-detects-omission`. The control `state-checklist-exempt-no-reason` of the step `state` applies to the checklist file, which has no exemption; the notes file's reasons are judged by the sweep test (three words at least).
- **`stic.late_grants` has no non-default witness among the 18 cuts.** It is zero at every cut (every bus window of the programs is granted). The exemption is argued and the row is covered by the poisoned-target equivalence and the digest equality; a designed program with a non-interruptible stretch of at least 1,026 cycles (about 94 consecutive stores of 11 cycles) in a displayed frame would witness it and let the exemption go. Not built here (a new program and a new cut).
- **`obs.input.words` is exempt only while the next event lies inside the comparison window** (the Lamp Tone pass run has events at 100,000 and 220,000; the window of HP-INPUT is 238,944). The argument is in the notes.
- **Three ROW rows are visible in a public report only** (`cpu.fault_reason`, `stic.blocked_reads`, `stic.blocked_writes`); they keep ROW because the plan counts the reports as divergence.
- **Plan 08-15 (corruption and fuzz family):** `sc_new`, `sc_run_leg` and `sc_cut_compare` (state_util.h) build a program's instance, run a leg with fresh buffers and compare a cut; the 18 pinned ticks are real states a fuzzer can start from.
- **Plan 08-16 (`ivs-run --roundtrip-at-ticks`):** the pinned ticks of the lamp cuts (59,772, 59,808, 59,900, 74,928, 79,008, 79,016, 96,652, 100,064) are unit boundaries of the Lamp Tone run; the runner hook can reuse them as its round-trip points.
- **Information.** D10 and D11 are not performed; no claim above provisional exists; no hosted-only check was run (the hosted-style container gate ran green); the snapshot is exact-build state; STA-01 and STA-02 stay Pending (plan 08-17 closes them on the terms of its plan); REL-01 stays Pending (D-28). `data/state/tests-pending.ids` holds no id line any more.

## Interfaces for the next plans

- Tests (`state_util.h`): `sc_run`, `sc_run_find`, `sc_new`, `sc_events` (a program of a cut), `sc_cut`, `sc_cut_at`, `sc_cut_find`, `sc_cut_end`, `sc_window_end` (the fault-run window), `sc_witness_find`, `sc_witness_state_holds`, `sc_leg`, `sc_run_leg`, `sc_ref_get`, `sc_reports`, `sc_cut_compare` (a bit mask of what differed, `SC_F_*`, with the tampers `SC_T_*` for self-controls).
- Data: `data/expected/state-cuts.ids` (all pinned), `data/expected/state-sweep-notes.ids`, the three `cut-snapshot-sha256.*` literals.
- Gate counts after this plan `steps=38/38 controls=378/378 faults=235/235 fault-runs=235 tests-expected=4888 tests-executed=4884 tests-skipped=4 tests-missing=0 api-calls=19 public-symbols=19 state-rows=88 state-cuts=18` (the next plan adds 12 ids and no fault).
- Threat ids T-08-34 and T-08-35 are used here (T-08-SC throughout); the next state plans continue from T-08-36 or the numbering their plans assign.

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=38/38 controls=378/378 ... faults=235/235 ... fault-runs=235 ... tests-expected=4888 tests-executed=4884 tests-skipped=4 tests-missing=0 ... api-calls=19 ... public-symbols=19 ... state-rows=88 state-cuts=18 (0 FAIL lines)
dash tools/gate.sh:    the identical summary line (0 FAIL lines)
GATE_LINUX_PLATFORM= sh tools/gate-linux.sh: exit 0, build/gate-linux.log ends gate: PASS (gcc 13.3.0, cmake 3.28.3, x86_64), the plan-end counts, 0 FAIL lines
```

After the Task 1 tree: sh and dash `faults=234/234 tests-expected=4870 tests-executed=4866 state-cuts=18`, identical summary line, container exit 0 (323 s). After the Task 2 tree: `faults=235/235 fault-runs=235 tests-expected=4880 tests-executed=4876`, identical summary line, container exit 0 (363 s). After the Task 3 tree: the plan-end counts under sh (146 s) and dash (153 s) with the identical summary line, container exit 0 (468 s). On the closing tree (this SUMMARY, STATE.md and ROADMAP.md added or edited; `tracked=2459`): sh (`gate-time total=183`, `gate-cpu` user 216.0 sys 174.5) and dash (140 s, user 187.0 sys 148.3) `gate: PASS` with the identical summary line (md5 2bcfdd2c4b9e654b5d73412c09c0d69e), 0 `FAIL` lines, the plan-end counts above (both run while the container gate ran, so the times are contended), and `GATE_LINUX_PLATFORM= sh tools/gate-linux.sh`: exit 0, `gate: PASS`, the plan-end counts, 0 `FAIL` lines, `gate-time total=473`; `build/gate-linux.log` holds that run. The dev-preset `ctest` suite `state` (49 ids) and the release-preset `state` suite pass; the nine fault executables fail exactly the ids their lines of `data/expected/faults/state.ids` name, with only `state.` collateral.

## Known Stubs

None.

## Threat Flags

None new. T-08-34 (a restore that recomputes a derived field, a zero-coincident witness, the shared-table blind spot): poisoned targets with a whole-instance memcmp, streams and the nine digests; the zero-coincidence guard per row; the sweep in both directions with its judge; the designed latch program with a recomputing variant that must differ; the probe that makes the real restore recompute the latch fails the cut ids. T-08-35 (silent drift of a pinned tick): the ticks come from a generated include, the witness ids re-prove each against the scan and against the instance of the public run call, and the probe that changes the interrupt entry by one cycle fails the witness id with the cut and its rule. T-08-SC: no installs.

## Self-Check: PASSED

Verified before the final commit: the SUMMARY and the created files of the plan exist (FOUND for `tests/state-cuts-inc.cmake`, `state_progs.c`, `state_witness.c`, `state_cut.c`, `state_sweep.c`, `data/expected/state-sweep-notes.ids` and the SUMMARY); commits `01bf8da`, `0686241` and `22215de` are ancestors of HEAD and `git rev-list --count 5d56e2128943ea79e1369da7c1c81059231e8054..HEAD` printed 3 before this file's commit; nothing is staged but the files of this commit and `.planning/config.json`, `.planning/state.json` and `.planning/ci-runs.tsv` are not staged; `git ls-files` lists no `.ivsc`, `.bin`, `.rom`, `.exec`, `.ivsn`, `.wav`, `.ppm` or `.vraw` file; the diff of the plan outside `.planning/` holds no home, scratch or temp path and no "Intellivision"; no commit deletes a file; `grep -c IVS_FAULT_SNAPSHOT_STIC_ROWS_OMITTED core/src/machine/machine_digest.c` prints 1 and the id occurs once in `core/src`; `grep -c '^state[.]cut[.]\|^state[.]witness[.]\|^state[.]sweep[.]' data/expected/tests/state.ids` prints 10 and `grep -c '^state[.]judge[.]' ...` prints 5; `grep -v '^#' data/expected/state-cuts.ids | grep -c pending-08-14` prints 0 and `data/state/tests-pending.ids` holds no id line; `git diff --stat 5d56e21 HEAD -- docs/formats/ivsn-v1.md` is empty: the specification is not edited by this plan; `grep -c '^cut-snapshot-sha256[.]' data/expected/ivsn-literals.ids` prints 3; the acceptance criterion `grep -c 'EXEMPT:' data/expected/state-checklist.ids` equal to the exemptions listed is NOT met (0 against 9: deviation 2, the exemptions are in `data/expected/state-sweep-notes.ids`).
