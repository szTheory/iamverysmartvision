---
phase: 05-hand-controllers-and-psg-sound
plan: 12
subsystem: claims, discrepancies, important-gates registry, traceability, Decision Register, CI close-out, verification
tags: [close-out, claims, discrepancies, d10-inventory, important-gates, verification, D-26, D-45, V-16, A-12, INP-01, INP-02, INP-03, API-03, AUD-01, AUD-02]
status: complete

requires:
  - phase: 05-hand-controllers-and-psg-sound
    provides: "plans 05-01 to 05-11; oracle commit 14a1904 (05-01); HEAD at start 8225f4e"
provides:
  - "claims CL-0009 to CL-0013 (provisional; none measured-hardware); with CL-0014 and CL-0015 of plan 05-11 the Phase 5 claims are CL-0009 to CL-0015"
  - "final Phase 5 state of the discrepancy records C11, C12, C14, C15, C16, C23, C63 to C67, C69 to C80 (evidence names the profile field and the test ids; C67 resolver gains the passing boundary tests; C73 text aligned with the profile tag; C78 and C79 resolver ids corrected)"
  - "important-gate registry for INP-01, INP-02, INP-03, API-03, AUD-01, AUD-02 (gates=48 lines=175) and three new controls (316 in total)"
  - "REQUIREMENTS.md: the six requirements complete with the provisional standing stated; ROADMAP.md lists 05-12"
  - "PROJECT.md Decision Register notes (D3, D4, D6, D7 confirmed); CI-EFFICIENCY.md Phase 5 close-out (phase5-close: no-lever)"
  - "05-VERIFICATION.md with three tables; handoff-items=32"
  - "c-faults watchdog arming fix (Rule 1)"
affects: [phase-5-verification, phase-6]

actuals:
  tokens: 19000
  tasks: 4
  commits: 5
plan_head_before: 8225f4ee6e150da9f3b69c7b78f539c3e4ebf2d3
plan_head_after: d03719c40c601c1ff39fa4538c27ca9e95bc5444

tech-stack:
  added: []
  patterns:
    - "A registry line names an existing mechanism and the requirement whose silent pass it would allow; ids are resolved against the inventories"
    - "A FIFO that a signal can interrupt is held open read-write on one descriptor and read in a retry loop"

key-files:
  created:
    - .planning/phases/05-hand-controllers-and-psg-sound/05-VERIFICATION.md
    - tools/check/controls/important-gates-req-uncovered-phase5/
    - tools/check/controls/important-gates-unknown-id-phase5/
    - tools/check/controls/important-gates-bad-req-phase5/
  modified:
    - data/claims.json
    - data/expected/claims.ids
    - data/discrepancies.json
    - data/expected/important-gates.ids
    - data/expected/controls/important-gates.ids
    - tools/check/steps/important-gates.sh
    - tools/check/steps/c-faults.sh
    - tools/check/controls/_good/tree/data/expected/important-gates.ids
    - tools/check/controls/important-gates-uncovered-gate/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-unknown-id/overlay/data/expected/important-gates.ids
    - docs/input/second-read-request.md
    - docs/psg/second-read-request.md
    - docs/psg/reference-notes.md
    - .planning/REQUIREMENTS.md
    - .planning/ROADMAP.md
    - .planning/PROJECT.md
    - .planning/CI-EFFICIENCY.md

key-decisions:
  - "The C73 inconsistency is resolved toward the profile field: the discrepancy now says unknown-default-chosen (the page prints no sentence for the sense and a community page orders the bits the other way); D-04 said inference. Behaviour is unchanged and both tags make dependent claims provisional"
  - "The second reads are recorded as tier source-expectation, second-read: controller 31 of 31 rows agree; PSG 90 of 120 vectors agree, 7 unknown, 21 not comparable, 2 convention differences. The stale pending State lines of the two request files and one sentence of reference-notes.md were corrected"
  - "The D-22 listening check is not performable: the only WAV is a one-step click with no tone (tone A is silent until a reload about 65,536 ticks after a period write). It is recorded as a deferred item with that reason and is not counted as a handoff"
  - "AUD-01 (marked complete by plan 05-07) is confirmed justified on its own words (per-rule vectors, tagged profile data, empty pending lists) with the caveat that the part is not a verified console part (C11)"
  - "The CI rule is read under the 05-L1 budgets in force (cpu median 260, revisit above 280; wall revisit above 90 s or any run above 120 s); the older 200 and 190 figures quoted by the plan are shown beside the result and superseded"

duration: about 55 minutes of edits and runs (08:30 to 09:20 local), most of it waiting on gate runs under a host load of 8 to 60
completed: 2026-11-12
---

# Phase 5 Plan 12: close-out Summary

**Five claims, the final discrepancy states, an important-gate registry for the six Phase 5 requirements with three new controls, traceability, Decision Register notes, the D-45 evaluation and a three-table verification record; all three gates end `gate: PASS steps=35/35 controls=316/316 faults=191/191 tests-missing=0`.**

Oracle commit (05-01): `14a1904` (the controller and PSG literals, the PSG rules and vectors, and the diagnostic derivation, committed before any implementation plan).

## What was built

- `583d3c6` (Deviation 1): `tools/check/steps/c-faults.sh` watchdog arming made signal-safe.
- `18bd61c` (Task 1): CL-0009 to CL-0013 (CL-0009 and CL-0010 and CL-0012 `source-expectation`, CL-0011 and CL-0013 `deterministic-repeatability`, all `provisional`, statements free of the claim words, `depends_on` ids all present in the profile), evidence text for C11, C12, C14, C15, C16, C23, C63 to C67 and C69 to C80 naming the profile field and the test ids (every named test id was checked against the inventories), resolver fixes for C67, C78 and C79, the C73 alignment, and the stale second-read wording.
- `3c2d71d` (Task 2): the registry step accepts the six Phase 5 requirements and demands each be covered; 73 registry lines (`gates=26 lines=102` to `gates=48 lines=175`); controls `important-gates-req-uncovered-phase5`, `important-gates-unknown-id-phase5`, `important-gates-bad-req-phase5` (313 to 316); the toy registry of the good tree and the two older overlays gained Phase 5 toy lines; REQUIREMENTS.md and ROADMAP.md.
- `033677e` (Task 3): the Decision Register notes in PROJECT.md and the CI-EFFICIENCY Phase 5 close-out entry.
- `d03719c` (Task 4): `05-VERIFICATION.md`.

## Figures

- Final gate (all three shells): `gate: PASS steps=35/35 controls=316/316 ... faults=191/191 ... tests-expected=3772 tests-executed=3770 tests-skipped=2 tests-missing=0`, `gates=48 lines=175`, `d10-items=32`, `claims=15 provisional=14`, `discrepancies=80 open=72 pending-resolvers=13`, `consumer-lines=43 pkg-selftest=13/13`, `goldens=7`, `psg-rules=21 psg-vectors=120 psg-policies=12`, 0 FAIL lines. Phase 5 start was `steps=33/33 controls=277/277 faults=149/149 tests-expected=2816`.
- `handoff-items=32` (the data lines of `data/expected/d10-measurements.ids`, equal to `d10-items=32`; the plan's "14 Phase 5 items" is the earlier planning figure: the file holds 12 system, 7 controller, 11 PSG and 2 owner-image items, unchanged by this plan).
- Timing (laptop, other work running; the host was at a 1-minute load of 8 to 62 during the final runs, so none of these is a cost figure): `sh tools/gate.sh` `gate-time total=231`, `gate-cpu` 140.2 + 175.0 (load 22.6); `dash tools/gate.sh` `total=57`, 80.4 + 80.6 (load 62 at start); container (`sh tools/gate-linux.sh`, Ubuntu 24.04, gcc 13.3, emulated amd64) `total=230`, `gate-cpu` 896.8 + 64.7, `gate-linux: exit 0`, last log line `gate: PASS`, `cc=GNU`. Three earlier `sh` runs on the same tree read 77, 79 and 97 s (249, 242 and 269 cpu-s) at loads 8 to 12; plan 05-11 read 41 s and 126 cpu-s on a quieter day. A reference run of the unchanged `psg` suite took 0.80 s against 0.34 s at plan 05-08, so the host explains the difference (CI-EFFICIENCY entry).
- Hosted rows (read, not recorded here): 35 rows, 12 since the Phase 4 close; last-5 medians `gate_s` 78 and `cpu_s` 231; the two outlier runs (103 and 106 s, 323 and 336 cpu-s) sit between 58 s and 72 s runs. The rule does not fire; checkpoint lines in the record: `checkpoint-1: lever-required` (05-04), `checkpoint-2: no-lever` (05-08), and `phase5-close: no-lever`.

## Deviations from Plan

**1. [Rule 1 - Bug] The `c-faults` watchdog arming could hang under macOS `sh`.** The first gate run of this plan stalled for over 10 minutes at no CPU in `c-faults`: in `run_limited` (changed in `8225f4e` to arm the watchdog through a FIFO) the parent re-opened the FIFO for reading in a retry loop; a SIGCHLD from the fast fault process interrupted the open after the writer had been released, the writer went on, and the retry had no writer for good. The parent now holds the FIFO read-write on descriptor 9 and retries the read on it (data stays in the pipe while the descriptor is open). An A/B timing of the old and new function (400 arm and release cycles on four lanes) shows the same cost (about 2 s), and 1,200 cycles plus a firing case ran clean under `sh`, `dash` and `bash`. Commit `583d3c6`.

**2. [Rule 1 - Plan inconsistency] Registry line count.** The plan says "about 30 lines"; its own list of mechanisms needs more (the registry has 73 new lines, `lines=175`). Every id was resolved against the inventories by the step; none was added to make a line fit. Lines under AUD-02 for the `runner.judge.refpcm-*` self-controls and `golden-refpcm-mismatch` follow the plan, although D-21 says nothing in the raw-audio evidence depends on the converter: they are commented in the registry as guards for the separation, and no claim reads them.

**3. [Rule 1] The D-45 figures of the plan are superseded.** The plan quotes "revisit above 200 cpu-s, lever trigger at a projection of 190". The orchestrator's 05-L1 decision re-baselined the budgets (cpu median 260, revisit above 280; wall 75 and above 90 or any run above 120). The close-out shows the result under the rule in force and states that the last-5 cpu median of 231 is above the old line.

**4. [Rule 1] The independent recomputation script lives in the scratch area outside the repository** (not a `mktemp -d` directory as the plan words it); it is not committed.

**5. [Rule 3] Frontmatter of `05-VERIFICATION.md`.** `covered_files` and `covered_digest` come from the fingerprint verb of the GSD tools over the files this plan changed and the key Phase 5 evidence files, as `04-VERIFICATION.md` does; the independent verifier regenerates them.

**6. [Other] No `roadmap.update-plan-progress`, `requirements.mark-complete` or `phase.complete` was run.** ROADMAP.md (plan list and the progress row, left `In Progress`) and REQUIREMENTS.md were edited by hand, in the shape those commands produce, so that the orchestrator completes the phase after review, security and verification.

## Decisions Made

See key-decisions. D7 was read against D-21 and the register row already matched, so it was confirmed and not edited; the D3, D4 and D6 notes are in PROJECT.md under the register.

## Known Stubs

None. (`data/psg/fields-pending.ids` and `data/psg/vectors-pending.ids` hold no line.)

## Open Items

- **Handed off, not performed (`handoff-items=32`):** the D10 system, controller and PSG items and the two D11 owner-image items (05-VERIFICATION table 2).
- **The D-22 listening check is deferred, not handed off:** no Phase 5 artifact has a listenable tone (the WAV is 224 frames, sample values 0 to 757, one level step); a later phase needs a guest that runs long enough to sound.
- **The AY-3-8910 reprint and the 1998 pin chart were not obtained (05-01);** a tier 1 AY-3-8914 data sheet, if the owner holds one, would move the PSG rule vectors from `inference`.
- **Hosted rows after this plan's push** are for the orchestrator to record and re-evaluate (`phase5-close: no-lever` rests on rows up to the 05-11 tree).
- Carried unchanged: Phase 4 review deferrals IN-04, IN-06, IN-09 to IN-11; `sh tools/hygiene.sh --history` still fails on earlier planning commits; the interrupt-entry bound inputs of 05-08 (Phase 6 must re-read `psg.bounds.g-and-u` and `psg.bounds.memsys-bus-never-waits`); the noise first-bits and envelope phase conventions.
- Not done on purpose: `input.fuzz.*` and `input.profile.fields-are-lines-only` named in `05-VALIDATION.md` do not exist as ids (plan 05-05 delivered the guest-visible slicing proofs as `input.slice.guest-visible` and `input.iso.interleaved-instances` and the field-shape control `input-profile-field-shape`); 05-VERIFICATION cites only ids that exist.

## Threat Flags

None new. T-05-29 (a silently shrinking handoff list or inflated tier): `d10-items=32` is printed by the `discrepancies` step and copied as `handoff-items=32`; all seven Phase 5 claims are `provisional` and the `claims` step rejects the measured tier; the three new controls guard the registry. T-05-SC: no package installs.

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=35/35 controls=316/316 ... faults=191/191 ... tests-expected=3772 tests-executed=3770 tests-skipped=2 tests-missing=0`, 0 FAIL lines, `gate-time total=231` (load 22.6).
- `dash tools/gate.sh`: the same line, 0 FAIL lines, `gate-time total=57`.
- `sh tools/gate-linux.sh`: the same tokens with `cc=GNU`, exit status 0, `build/gate-linux.log` last line begins `gate: PASS`, `gate-time total=230`.

## Self-Check: PASSED

- Files exist: `.planning/phases/05-hand-controllers-and-psg-sound/05-VERIFICATION.md`, `tools/check/controls/important-gates-req-uncovered-phase5/`, `tools/check/controls/important-gates-unknown-id-phase5/`, `tools/check/controls/important-gates-bad-req-phase5/`, `data/expected/important-gates.ids`, `data/claims.json`.
- Commits on HEAD: `583d3c6`, `18bd61c`, `3c2d71d`, `033677e`, `d03719c` (`commits: 5` measured from the ledger with `git rev-list --count 8225f4e..HEAD` at write time; the documentation commit that adds this file follows).
