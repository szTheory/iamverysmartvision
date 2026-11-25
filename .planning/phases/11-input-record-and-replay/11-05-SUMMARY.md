---
phase: 11-input-record-and-replay
plan: "05"
subsystem: input-record-replay
tags: [ivsl, named-faults, requirement-close, registry, toy-registries, phase-exit]
requires:
  - "11-02 (runner/ivsl.c's writer and runner/exec.c's replay substitution, the equality and the round trip)"
  - "11-03 (the nine refusal ids and no-content-digest)"
  - "11-04 (the two options and the one process case)"
provides:
  - "runner/ivsl.c: the IVS_FAULT_RECORD_DROPS_LAST_EVENT site (one event line fewer than the log holds)"
  - "runner/exec.c: the IVS_FAULT_REPLAY_APPLIES_LATE site (one tick added to every replayed event)"
  - "data/expected/faults/runner.ids: two rows, each must-failing runner.record.replay-equals-straight"
  - "data/expected/important-gates.ids: four runner-record rows, TOOL-01 Complete with its ten toy registry lines and three toy record rows"
  - "the phase exit: three gate shapes with the two local ones re-run after the records commit, and Phase 11's five deferred items in STATE.md"
affects:
  - "the phase boundary: the container gate and verify-work are the orchestrator's"
tech-stack:
  added: []
  patterns:
    - "a fault's allowed column names exactly the ids that fail, never a prefix: a widened prefix hides the fault from the id that has to catch it"
    - "the requirement flip, its registry rows, its toy lines and its toy records are one commit, because the accepted set is derived from the record"
    - "the exit evidence is re-run after the plan's own records commit by construction, not by promise"
key-files:
  created: []
  modified:
    - runner/ivsl.c
    - runner/exec.c
    - data/expected/faults/runner.ids
    - tests/runner/runner_exec.c
    - data/expected/important-gates.ids
    - tools/check/steps/important-gates.sh
    - .planning/REQUIREMENTS.md
    - .planning/ROADMAP.md
    - .planning/STATE.md
decisions:
  - "the writer fault's allowed column names the format round trip explicitly, measured: a drop inside ivs_ivsl_format is applied again to the log it just wrote, so format is not a fixpoint under this fault and the plan's 'safe must-pass' premise is false (deviation 1)"
  - "the complete-uncovered control's whole-file copy of the toy requirement record is a third toy record, not a second: leaving it alone would make the toy registry's TOOL-01 line an unknown id in that control (deviation 2)"
  - "no exclusion-ledger row for TOOL-01: four real mechanisms, and an exclusion row would be the registry declining to cover what it just built"
metrics:
  duration: "45m"
  completed: "2026-11-25T10:19:29Z"
  tasks: 3
  files: 22
status: complete
actuals:
  tokens: 7400
  tasks: 3
  commits: 3
  plan_head_before: 311b9b1
  plan_head_after: bcbcf34
---

# Phase 11 Plan 05: the two named faults, the requirement close and the phase exit Summary

The phase's last plan lands the two faults that make TOOL-01's evidence non-vacuous, closes the
requirement with its registry rows and toy obligations in one commit, and takes the exit evidence
with the two local gates re-run **after** the plan's own records commit. `faults` and `fault-runs`
move 235 to 237, `gates` 109 to 110, `lines` 333 to 337; `steps=38/38` never moves, no claim is
minted or raised, and the tier ceiling stays `deterministic-repeatability`.

`commits: 3` is measured with `git rev-list --count 311b9b1..bcbcf34`; this SUMMARY's own commit is
the plan's fourth and adds only this file.

## The two faults, and exactly which ids each one fails

Both sites are in `IVS_RUNNER_SOURCES` library TUs, never in `runner/main.c` or `runner/hostio.c`
(those are `IVS_RUN_SOURCES` and outside the fault framework by construction, `tests/faults.cmake:155`).
Each fault executable was built and run once; a `not ok` line is preceded by its own diagnostic.

**`IVS_FAULT_RECORD_DROPS_LAST_EVENT`** — `runner/ivsl.c`'s writer emits one event line fewer than the
log holds (`n_lines = n_events - 1`), and the byte count reads the same variable so the log still
parses and is short by one event:

```
# the log took 116 bytes, want 152
not ok 202 runner.record.replay-equals-straight
# the recorded log does not parse: reason ok line 0
not ok 203 runner.record.format-roundtrip
ok 22 runner.parse.diag-boot-pass
```

**`IVS_FAULT_REPLAY_APPLIES_LATE`** — `runner/exec.c`'s substitution loop adds 1 to every replayed
`at_tick`, so the applied-event chain (which hashes `be64(at_tick)`) differs from the recorded run's:

```
# (one diagnostic) the recorded and the replayed reports differ at digest.input
not ok 202 runner.record.replay-equals-straight
ok 22 runner.parse.diag-boot-pass
```

The late fault fails **exactly** the equality, one `not ok` line in 261 ids. The drop fault fails the
equality **and** the round trip, and that is the measured deviation below.

The stale figure the plan asked to correct: `tests/runner/runner_exec.c:2988` said "the 19 runner
faults". `grep -rn '19 runner' tests/ runner/ tools/` returned that one line at plan start and no
other; it now reads 21, and the same grep returns nothing.

## Deviation 1 (measured, not assumed): the round trip is not a safe must-pass

The plan's must_have says the format round trip "is a **safe must-pass** and must not be relied on as
a detector", on the reasoning that a consistent drop round-trips to identical bytes, and adds the
remedy "if `runner.record.format-roundtrip` turns out to fail under either fault, the round-trip id is
asserting a count derived from the scenario where it must compare parse to parse - fix the id, never
widen this column."

Measured: the round trip **does** fail under the drop fault, and the remedy is structurally
unavailable. `ex_record_format_roundtrip` starts from a log a run produced; the fault makes that log
hold one event, so the case's `l1.n_events != 2u` fires (`# the recorded log does not parse: reason ok
line 0`). Making the count parse-derived does not help, because the drop lives in
`ivs_ivsl_format` itself: formatting the parsed one-event log drops again and writes zero event lines,
so `n2 != n1` at assertion 1 and format is not a fixpoint under this fault. No edit to that id can
make a drop inside the formatter round-trip.

Two routes were open and the second was taken:

- put the site in `runner/exec.c`'s recorder instead, so `ivs_ivsl_format` stays a fixpoint and the
  round trip passes — this satisfies the plan's must-pass but requires relaxing the round trip's
  scenario-derived count assertions, which makes that id accept a total drop (a log with zero events
  round-trips to itself). That weakens a green id to accommodate a fault.
- keep the plan's site (`runner/ivsl.c`'s writer, the artifact path the plan names) and name the round
  trip **explicitly** in the allowed column:
  `runner.record.replay-equals-straight|runner.record.format-roundtrip`. Two exact ids, never a
  `runner.record.` prefix, so nothing is hidden from the id that has to catch the fault.

The second route is recorded in the registry's own comment above the two rows, in `faults-verdict`'s
terms (the column is a list of allowed failing ids, matched as prefixes, so two exact ids is the
narrowest honest declaration), and `faults=237/237` measures it.

## The requirement close: one commit, and the identity closes by arithmetic

The three derived counts, from the three commands the gate itself uses (the awk over
`.planning/REQUIREMENTS.md` and `.planning/milestones/*-REQUIREMENTS.md` for the `## Traceability`
rows whose status begins `Complete`; the ledger's non-comment first column; the registry's distinct
fourth column):

| | complete | excluded | accepted | registered |
|---|---|---|---|---|
| at plan start | 57 | 12 | 45 | 45 |
| after the flip | 58 | 12 | 46 | 46 |

`57 - 12 = 45` and `58 - 12 = 46` close exactly, and `diff` of the accepted list against the
registry's distinct requirement list is empty. `TOOL-01` is in the accepted set, has **no** row in
`data/expected/important-gates-excluded.ids` (that file is untouched, `excluded` stays 12), and takes
four rows under one new gate name:

```
runner-record fault RECORD_DROPS_LAST_EVENT TOOL-01
runner-record fault REPLAY_APPLIES_LATE TOOL-01
runner-record mutant runner.judge.record-replay-differs TOOL-01
runner-record mutant runner.judge.record-replay-inert TOOL-01
```

`-- important-gates pass gates=110 lines=337`: `gates` 109 to 110 (one new gate name) and `lines`
333 to 337 (four new rows). R2's mechanism side is retargeted in
`tools/check/steps/important-gates.sh`'s phase table with a Phase 11 line naming `TOOL-01`, plan
11-05 and `runner-record`; the script's logic is unchanged, because the accepted set is derived and
the `nreq` split string the R2 row named no longer exists (plan 09-05).

**One commit** (`16afb31`), `git show --stat` file count **16**: the flip in `.planning/REQUIREMENTS.md`
(both the `- [x] **TOOL-01**` checkbox line and the `| TOOL-01 | Phase 11 | Complete |` Traceability
row), the four registry rows, the R2 header retarget, ten toy registry lines and three toy record
rows.

## The toy obligations, by path, and the proof the line numbers did not move

One appended line, `toy-tool01 fault GOOD_FAULT TOOL-01`, as the **last** line of all ten whole-file
toy registries (`awk`-counted: ten files, each whose last line is that exact string):

```
tools/check/controls/_good/tree/data/expected/important-gates.ids
tools/check/controls/important-gates-bad-req-phase5/overlay/data/expected/important-gates.ids
tools/check/controls/important-gates-bad-req-phase6/overlay/data/expected/important-gates.ids
tools/check/controls/important-gates-req-uncovered-phase5/overlay/data/expected/important-gates.ids
tools/check/controls/important-gates-req-uncovered-phase6/overlay/data/expected/important-gates.ids
tools/check/controls/important-gates-req-uncovered-phase7/overlay/data/expected/important-gates.ids
tools/check/controls/important-gates-uncovered-gate/overlay/data/expected/important-gates.ids
tools/check/controls/important-gates-unknown-id-phase5/overlay/data/expected/important-gates.ids
tools/check/controls/important-gates-unknown-id-phase6/overlay/data/expected/important-gates.ids
tools/check/controls/important-gates-unknown-id/overlay/data/expected/important-gates.ids
```

(Ten files: the good tree plus the nine `important-gates-*` overlays that carry a whole-file toy
registry. `important-gates-complete-uncovered` and `important-gates-excluded-malformed` overlay only
the requirement record and the exclusion ledger, so neither has a registry to append to.)

One appended row, `| TOOL-01 | Phase 11 | Complete |`, in each of the toy requirement records:

- `tools/check/controls/_good/tree/.planning/REQUIREMENTS.md`
- `tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md`
- `tools/check/controls/important-gates-complete-uncovered/overlay/.planning/REQUIREMENTS.md`

**Deviation 2:** the third file is not in the plan's file list. That control's overlay is a
**whole-file** copy of the toy requirement record (measured: `diff` against the good tree is the one
added line `| FND-99 | Phase 1 | Complete |`). Leaving it unamended would remove `TOOL-01` from the
accepted set **in that control's run only**, so the good tree's `toy-tool01` line would be an
`important-gates-unknown-id` there and the control's `EXPECT` would gain a finding it does not
declare. It is a copy of the same record, so it gets the same row; the diff against the good tree is
still exactly the `FND-99` line, re-measured after the edit.

**The line numbers did not move**, measured after the appends:

```
$ sed -n '19p' .../important-gates-bad-req-phase5/overlay/data/expected/important-gates.ids
toy-input fault GOOD_FAULT INP-04
$ sed -n '24p' .../important-gates-bad-req-phase6/overlay/data/expected/important-gates.ids
toy-video fault GOOD_FAULT VID-99
```

Those are the two bad rows their `EXPECT` files name (`FAIL important-gates-unknown-id: line 19` and
`line 24`), unchanged, because every line is **appended**. The toy comparison is over non-comment line
**sets**, so a line added to the good tree and to every overlay leaves every declared difference
unchanged.

`controls=395/395` with **no** `control-toy-drift` and **no** `control-deviation-unused` finding, and
no new control directory (`controls` stayed 395, `mode=tree`).

## The phase exit evidence

**Three gate shapes; two ran here and the container line is a visible not-run.**

Taken in this order on the final tree, then re-run on the records commit:

1. Before the records commit, `sh tools/gate.sh` and `dash tools/gate.sh`, both exit 0, last lines
   byte-identical.
2. The records commit `bcbcf34` (`.planning/ROADMAP.md` and `.planning/STATE.md` only).
3. Both local gates **re-run on that commit**, both exit 0, and their last line is **identical to the
   pre-commit line** — the plan 09-06 defect (a records commit moving a token after the last run) is
   therefore closed by construction here, and it measured as a no-op: no gate step reads
   `.planning/STATE.md` or `.planning/ROADMAP.md` (`grep -rln 'STATE.md\|ROADMAP.md'
   tools/check/steps/ tools/*.sh` returns nothing).

```
gate: PASS steps=38/38 controls=395/395 fault-runs=237 claims=28 provisional=27 faults=237/237
tracked=2646 gates=110 lines=337 files=2646 ... tests-expected=4988 tests-executed=4984
tests-skipped=4 tests-missing=0 runner-files=18 docs=64 doc-files=63 host-dependent=3
release-only=2 prefix-files=11 claim-refs=26 api-calls=19 api-statuses=15 api-rows=61
public-symbols=19 pinned-defines=120 pkg-selftest=19/19 consumer-lines=164 state-rows=88
state-cuts=18 input-rows=31 input-policies=11 input-profile-fields=31 cases=13 terms=9 mode=tree
```

`-- FAIL ` lines in either log: **0**. The token was compared token by token against the plan's
frozen prediction: every token is at its predicted value, and the only tokens that differ from the
plan-start baseline are the four the phase moves (`faults`, `fault-runs`, `gates`, `lines`).
`files=2646` and `tracked=2646` are equal, so the plan's file delta is 0 for every task.

**Container gate: `not run`.** The dispatch for this plan reserves it for the orchestrator's phase
boundary (an approved budget deviation, recorded here and in the final narration): this plan ran `sh`
and `dash` at each task and at the exit, and did **not** run `tools/gate-linux.sh`. There is no
`gate: PASS` to copy from `build/gate-linux.log` for this tree, and none is claimed (lesson 24).

## The seven verify-work items, in GUARDRAILS section 5's order

1. **The gate re-run on the final tree** — done twice: before and after the records commit, both
   `gate: PASS`, last lines identical, quoted above. Never an older SUMMARY.
2. **The token compared token by token** — done; every frozen token at its predicted value, the four
   moved tokens are the four the plan predicted, `steps=38/38` unchanged.
3. **Each new mechanism can fail on its stand-in, and the guard does the work** — the two faults'
   TAP output above (each fails its must-fail id; the late fault fails nothing else); the two judge
   self-controls (`runner.judge.record-replay-differs`, `runner.judge.record-replay-inert`) are `ok`
   in the normal suite and each was shown able to fail by its own stand-in in plan 11-02;
   `runner.record.no-content-digest` is `ok` and its stand-in (a real 64-hex digest) fires, with its
   scratch inversion in plan 11-03; the process case's no-log witness is the equality's liveness
   guard, and it ends guest-fail 10 where the replay passes. Measured in the normal suite:
   `# suite=runner executed=261 skipped=0 failed=0`, ids 202 to 215 all `ok`.
4. **The frozen oracle is unchanged** — `git diff 1e63733..HEAD -- docs/formats/ivsl-v1.md` is empty
   (0 lines): this plan does not edit the document, and its frozen sections are compared only to
   themselves. Ancestry re-proved: the document's add commit `dcfcd65` is an ancestor of
   `runner/ivsl.c`'s add commit `50148e7` (`git merge-base --is-ancestor`, exit 0).
5. **Hygiene, `binary-ext` and `git ls-files`** — `sh tools/hygiene.sh --tree` passes
   (`hygiene-files=2646`); `ivsl` appears in neither `tools/check/binary-ext.txt` nor `.gitignore`
   (both greps empty); no `.ivsl` file is tracked (`git ls-files | grep -E '[.]ivsl$'` is empty); no
   `.ivsl` file was ever produced inside the repository, and `git status --porcelain` carries only
   the orchestrator's `.planning/state.json`, which this executor never stages.
6. **What was automated, what was handed off, what a tool could not judge** — everything in this plan
   was automated: the faults, the registration, the identity arithmetic, the toy registries, the
   gates. **Nothing was handed off**, and **nothing in this plan needed a human judgement**; the only
   item this plan did not perform is the container gate, which is reserved by the dispatch and is
   recorded as a visible `not run`, not as a pass.
7. **Every `H<n>-<k>` or `D<n>` id mentioned has a row where it belongs, and no claim cites it** —
   this plan mentions `D15`, which is a decision-register row (ROADMAP line 128) and not a handoff, so
   `data/expected/handoffs.ids` is unchanged and contains no `D15` row by design. No claim was minted
   or raised: `claims=28`, `provisional=27`, `claim-refs=26` at both ends, and the tier ceiling
   `deterministic-repeatability` is a ceiling, not a claim — the replay is the same core, so it is
   never an independent comparison.

## Deferred items, recorded where a later run will read them

All five are in `.planning/STATE.md`'s `## Deferred Items` table, appended by plan 11-05 with their
reason and their home; none is a new gate mechanism, and `lines` moved only for the four TOOL-01 rows.

| Item | Why deferred | Where a later run reads it |
|---|---|---|
| the `scenario`/`profile` mismatch refusals (research O-1, O-2) | a new `ivsl` refusal token is a version decision; the limit is stated, not a defect | `docs/formats/ivsl-v1.md` `## Identity`; STATE.md Deferred Items |
| a content-identity field in the log | D15: the log is an input artifact and carries no content bytes or digest; the route is `expect-sha256` | `docs/formats/ivsl-v1.md`; STATE.md Deferred Items |
| promoting the process case to a real-executable gate case | it would be a new gate step (GUARDRAILS 2.1); the process case already runs the built binary | STATE.md Deferred Items (reversal trigger: a defect only at the process boundary) |
| the snapshot file options `--snapshot-out`/`--snapshot-in` | still deferred from Phase 8; Phase 11 neither needed nor touched them | STATE.md Deferred Items (unchanged row) |
| a new gate step for the record/replay evidence | forbidden by GUARDRAILS 2.1 and by the phase's cost gate; `steps=38/38` never moved | STATE.md Deferred Items |

## Deviations from the plan

**1. [Rule 1 - Bug, measured] The round trip fails under the writer fault, and the plan's remedy is
unavailable.** Found during Task 1. The fault's allowed column therefore names
`runner.record.format-roundtrip` explicitly beside the equality. Files: `data/expected/faults/runner.ids`
(and its comment). Commit `3a48161`. Detail and the two routes considered are above.

**2. [Rule 2 - Missing critical record] The complete-uncovered control's whole-file copy of the toy
requirement record is a third toy record.** Found during Task 2. Without it, that control reports an
undeclared `important-gates-unknown-id`. File:
`tools/check/controls/important-gates-complete-uncovered/overlay/.planning/REQUIREMENTS.md`.
Commit `16afb31`.

**3. [Budget, approved] The container gate was not run.** The dispatch reserves
`tools/gate-linux.sh` for the orchestrator's phase boundary; the SUMMARY carries a visible `not run`
line with that reason and no pass.

## Known Stubs

None. No file this plan touched carries a hardcoded empty value, a placeholder, a `TODO`/`FIXME`, or a
component with no data source. The two fault sites are `#ifdef`-guarded and inert in every normal
build; `runner/main.c` and `runner/hostio.c` carry no `IVS_FAULT_` text at all.

## Threat surface

No new surface. The two sites are compile-time conditionals inside existing library TUs, the registry
rows name mechanisms that already existed, and the only files this plan edited outside `.planning/`
are a fault registry, a step header and ten toy registries. No network, auth, file-access or schema
change; `T-11-18` to `T-11-22` are each met by a measured check quoted above.

## Self-Check: PASSED

- `runner/ivsl.c` contains `IVS_FAULT_RECORD_DROPS_LAST_EVENT`; `runner/exec.c` contains
  `IVS_FAULT_REPLAY_APPLIES_LATE`; neither `runner/main.c` nor `runner/hostio.c` contains
  `IVS_FAULT_` (grep empty).
- Both rows are in `data/expected/faults/runner.ids` with the four columns the file defines, and both
  fault executables' TAP output is quoted above.
- `git merge-base --is-ancestor` finds all three commits (`3a48161`, `16afb31`, `bcbcf34`) on HEAD.
- `-- important-gates pass gates=110 lines=337`; the identity closes at 46 = 58 - 12.
