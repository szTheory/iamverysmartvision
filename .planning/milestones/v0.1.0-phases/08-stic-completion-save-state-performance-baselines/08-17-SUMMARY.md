---
phase: 08-stic-completion-save-state-performance-baselines
plan: 17
subsystem: Phase 8 wave B, plan 9 - the close-out of the state wave - the snapshot extension as a rights-bearing binary by name with its control, the provisional claim CL-0028 and the regenerated release-notes table, STA-01 and STA-02 accepted by the important-gates step with 22 registry lines and the toy registries, the requirement marks on recorded terms and cost checkpoint C
tags: [B, STA-01, STA-02, rights, ivsn, claims, CL-0028, important-gates, checkpoint-C, D-19, D-36, D-37, D-43, D-44, T-08-39, T-08-40]
status: complete

requires:
  - phase: 08-stic-completion-save-state-performance-baselines
    provides: "plan 08-16 end state (the round-trip hook, the fifth installed-consumer replay, the 18 cuts, the corruption family); counts at plan start steps=38/38 controls=378/378 faults=235/235 fault-runs=235 tests-expected=4932 tests-executed=4928 tests-skipped=4 tests-missing=0 claims=27 provisional=26 gates=83 lines=282 goldens=27 pkg-selftest=19/19 state-rows=88 state-cuts=18 (re-measured with sh tools/gate.sh before the first edit)"
provides:
  - "tools/check/binary-ext.txt and .gitignore: ivsn (the extension of a snapshot file) is a rights-bearing binary by name; control files-unlisted-binary-ivsn (CHECK files, MAKE write docs/save.ivsn x, EXPECT FAIL files-unlisted-binary: docs/save.ivsn), controls 378 to 379; docs/owner-images.md section Snapshots; docs/formats/ivsn-v1.md section Rights (as built, plan 08-17), below the frozen sections"
  - "data/claims.json CL-0028 (tier deterministic-repeatability, provisional, artifact tests/state/state_cut.c, 51 profile members), data/expected/claims.ids, the CHANGELOG.md claim table regenerated (28 claims, 27 provisional), docs/evidence-tiers.md; claims 27 to 28, provisional 26 to 27"
  - "tools/check/steps/important-gates.sh accepts STA-01 and STA-02; data/expected/important-gates.ids +22 lines over six gates (gates 83 to 89, lines 282 to 304); one toy STA-01 line and one toy STA-02 line in each of the ten toy registries (the known-good tree and nine overlays), no CHECK, EXPECT or FORBID file changed"
  - ".planning/REQUIREMENTS.md: STA-01 and STA-02 complete with the wave B close-out note; .planning/CI-EFFICIENCY.md: the entry Phase 8 checkpoint C (plan 08-17) with the line checkpointC-08: no-lever; docs/state/hidden-state-checklist.md: the as-built end state, appended"
affects: [08-18, 08-19, 08-20, 08-21]

actuals:
  tokens: 5004
  tasks: 3
  commits: 3
plan_head_before: 1a27cdd5ad1ee8c9ff9b5ab94316f4e73e18c52d
plan_head_after: 027605e1775ca45edcf5071f04a4676ba4ebfa6b

key-decisions:
  - "The persistence policy (open question 8) is a delegated decision recorded here for the Decision Register: bytes only from the core, files are a host's policy, no default location, exact-build and not archival; reversible by adding sections and raising the schema version (the text is below)"
  - "One claim for the snapshot behaviour, no stronger than its evidence: CL-0028 at deterministic-repeatability, provisional, for one build and one profile, naming 18 hard points and the classes of refusal, with the oracles, the same-model-family caveat, the exemption count and a full Excluded on purpose list; no hardware, timing, cross-build or file statement"
  - "The frozen oracle data stays frozen: the exemption list of plan 08-14 lives in data/expected/state-sweep-notes.ids and the claim, the close-out note and the checklist as-built section cite it there; the checklist file, the schema literal and the sections of the specification are unchanged since the blind read (git diff against the oracle commit removes 0 lines)"
  - "Checkpoint C applies the rule as written: the median of the last five hosted rows, 7.98, is the projection because rows already contain wave B, and it is below both conditions (8.0 or more, above 8.3), so the line is no-lever; the entry states the other readings that sit at the lines (the newest row 8.29 plus the remaining additions reads 8.30, the median row counted reads 8.18 to 8.29) and does not soften them"

key-files:
  created:
    - tools/check/controls/files-unlisted-binary-ivsn/CHECK
    - tools/check/controls/files-unlisted-binary-ivsn/EXPECT
    - tools/check/controls/files-unlisted-binary-ivsn/MAKE
    - .planning/phases/08-stic-completion-save-state-performance-baselines/08-17-SUMMARY.md
  modified:
    - tools/check/binary-ext.txt
    - .gitignore
    - docs/owner-images.md
    - docs/formats/ivsn-v1.md
    - data/expected/controls/rights.ids
    - data/claims.json
    - data/expected/claims.ids
    - CHANGELOG.md
    - docs/evidence-tiers.md
    - tools/check/steps/important-gates.sh
    - data/expected/important-gates.ids
    - tools/check/controls/_good/tree/data/expected/important-gates.ids
    - tools/check/controls/important-gates-bad-req-phase5/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-bad-req-phase6/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-req-uncovered-phase5/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-req-uncovered-phase6/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-req-uncovered-phase7/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-uncovered-gate/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-unknown-id-phase5/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-unknown-id-phase6/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-unknown-id/overlay/data/expected/important-gates.ids
    - docs/state/hidden-state-checklist.md
    - .planning/REQUIREMENTS.md
    - .planning/CI-EFFICIENCY.md
    - .planning/STATE.md
    - .planning/ROADMAP.md

requirements-completed: [STA-01, STA-02]

completed: 2026-11-14
---

# Phase 8 Plan 17: the state wave close-out Summary

**A snapshot file can no longer be committed by accident, the snapshot behaviour is one provisional claim no stronger than its evidence, STA-01 and STA-02 are held by named mechanisms that the gate shows can fail, and the cost position after wave B is on record. `ivsn` is a rights-bearing extension by name (the binary list that four rules read, `.gitignore`, and a control that fails when the extension is removed from the list), and `docs/owner-images.md` says what a snapshot of a run with owner-supplied images is and what the project does about it. CL-0028 states once, for `intv-2609-ntsc-family-p1` and this build only, that a snapshot restored into a poisoned or dirtied target and continued equals the uninterrupted run at the 18 named hard points and that the named classes of bad snapshots are refused with the instance unchanged; it is provisional at the tier `deterministic-repeatability` with the oracles, the same-model-family caveat, the nine exemptions and a full exclusion list written into its note, and the CHANGELOG table shows 28 claims. The important-gates step accepts STA-01 and STA-02 and its registry names 22 mechanisms over six gates (nine faults, six judges and the seven controls of the step `state`), each of which resolves, with one valid toy line for each requirement in the ten toy registries so that every pristine tree and whole-file overlay still covers every accepted requirement; no CHECK, EXPECT or FORBID file changed. STA-01 and STA-02 are marked complete in REQUIREMENTS on the recorded terms (a provisional claim, one build and one profile, one author and a same-family blind reader, nine exempt rows with arguments, informational digests, no file persistence, the file options deferred, nothing measured). Checkpoint C reads the median 7.98 of the last five hosted rows (budget 7.8, reading 8.0, line 8.3): the line is `checkpointC-08: no-lever`, with the readings that sit at the lines stated and the lever order and first trims named. No C, no fault, no test id, one control, no golden. The gates pass under sh and dash on every task tree and on the closing tree, and under the Ubuntu container on the Task 3 tree (the container run of the closing tree could not be completed by the environment, deviation 7), with the plan-end counts `steps=38/38 controls=379/379 faults=235/235 fault-runs=235 tests-expected=4932 tests-executed=4928 tests-skipped=4 tests-missing=0 claims=28 provisional=27 gates=89 lines=304 goldens=27 pkg-selftest=19/19 state-rows=88 state-cuts=18`.**

## Commits

| Commit | Task | What |
|--------|------|------|
| `7f981d6` | 1 (tracer) | `ivsn` in `tools/check/binary-ext.txt` and `.gitignore`, the control `files-unlisted-binary-ivsn` (controls 378 to 379), the Snapshots section of `docs/owner-images.md`, the as-built Rights section of the specification |
| `da36670` | 2 | CL-0028, `data/expected/claims.ids`, the regenerated CHANGELOG table (28 claims, 27 provisional), `docs/evidence-tiers.md` |
| `027605e` | 3 | STA-01 and STA-02 accepted by `important-gates.sh`, the 22 registry lines, the 20 toy lines of the ten toy registries, REQUIREMENTS (the two marks and the close-out note), the checkpoint C entry with its line, the as-built end state of the checklist document |
| the commit that adds this file | | this SUMMARY, STATE.md and ROADMAP.md |

## The rights rule (Task 1)

`ivsn` is the last line of `tools/check/binary-ext.txt`, the one list that the `integrity`, `files` and `binscan` rules and `tools/hygiene.sh` read, so a tracked file with that extension is a rights finding by name (`files-unlisted-binary`) and is skipped by the text rules; `*.ivsn` is in the rights-bearing block of `.gitignore` with a comment that names derived snapshot files. The control `files-unlisted-binary-ivsn` is the pattern of its neighbours: CHECK `files`, MAKE `write docs/save.ivsn x`, EXPECT `FAIL files-unlisted-binary: docs/save.ivsn`, registered in `data/expected/controls/rights.ids`. It can fail: with the line `ivsn` removed from the list (a scratch inventory holding the known-good tree and this one control, the list restored at once) the run reports `FAIL control-exit-zero: files-unlisted-binary-ivsn` and `controls=0/1`; with it present the whole control set reads `controls=379/379` and every earlier control still reports `control ok`. `sh tools/hygiene.sh --tree` passes and `git ls-files` lists no `.ivsn` file.

`docs/owner-images.md` gains the section Snapshots: a snapshot never holds an image byte (the three image arrays are not serialized; only the SHA-256 identities and the loaded flags of the slots are written) but does hold guest RAM, graphics RAM and the device registers that owner-supplied images caused to be written, so it is derived rights-bearing material in the class of the raw audio, the raw video and the frame outputs; the core writes no file and chooses no name, slot or location; the runner has no snapshot file option in this phase (the options to write and read one are deferred) and its `--roundtrip-at-ticks` hook keeps the snapshot in memory for one cut; the extension is refused by name and ignored by git; the format is exact-build and not archival; and a snapshot shows repeatability of one build on one profile and nothing about accuracy or any real EXEC or GROM image. The specification gets a new section Rights (as built, plan 08-17) at its end instead of an edit of the frozen Rights section, which still says the rule comes "by a later plan": the new section states that it has come, and what the persistence policy is.

## The claim CL-0028 (Task 2)

| Field | Value |
|-------|-------|
| profile, tier, standing | `intv-2609-ntsc-family-p1`, `deterministic-repeatability`, `provisional` |
| artifact | `tests/state/state_cut.c`, the comparison oracle (it exists; the claim step checks the path) |
| statement | for this build only: a snapshot taken at a unit boundary, restored into a target whose state rows hold poison or another run's state and run on, equals the uninterrupted run (the video, audio and write-log streams, the nine state digests and the instance bytes) at each of the 18 named hard points of `docs/state/hard-points.md`; a truncated, corrupt, foreign-format, foreign-schema, foreign-profile, foreign-image or unreachable-state snapshot is refused with the instance unchanged |
| `depends_on` | 51 profile members that the hard-point programs and the diagnostics rest on (the tick and cycle constants, the reset, interrupt and bus entries, the power-on fills, the memory-map devices and regions the programs touch, the controller model, the PSG register map, counter model and envelope constants, the STIC register map, access effects, backtab, windows, interrupt, frame phase and bus schedule, and the MOB field origin); every one is a member of the profile (the claim step fails an unknown field), and several are tier 4, inference or unknown-default-chosen, so the weakest-field rule keeps the claim provisional |
| note | the oracles (the container derived on paper before any serializer, the poisoned-target memcmp, the concatenated streams, the nine digests, the witness scan, the row-omission sweep, the corruption family with its classifier and model, the matrices, the invariants held at 411,075 boundaries, the runner hook and the fifth replay), the mechanisms shown able to fail (nine faults, six judges, seven controls, the rights control), the shared-core and same-model-family caveat (one author for the checklist, the container and the implementation; the blind read of the checklist was by the same family: 87 of 88 classes agree, 0 corrections), the exemption count (9 of 51, the list of plan 08-14), the two known acceptances, and the `Excluded on purpose` list |
| exclusions | no claim across builds or versions (exact-build, not archival); no hardware or timing claim; the eight machine digests of the header are informational and not authenticated; the trailer is integrity and not authenticity; no big-endian host was run; no file persistence exists; the field table and the checklist are inference checked by tests; no snapshot of a run with an EXEC or GROM image of any console (D11 open); no commercial content; a nineteenth point or another program is another claim; no measured tier exists (D10 and D11 are not performed) |

`data/expected/claims.ids` gains `CL-0028`; the CHANGELOG region between the markers is the output of `cmake -DROOT=. -P tools/check/claims-table.cmake` (28 rows, `Claims: 28 in total, 27 provisional, 1 established; none at tier measured-hardware.`), pasted in the same commit so that the drift check of plan 08-02 holds at the claim's own commit; `docs/evidence-tiers.md` lists the claim under its tier with `[claim:CL-0028]`. `grep -c 'Excluded on purpose' data/claims.json` went from 13 to 14.

## The registry (Task 3)

`tools/check/steps/important-gates.sh` accepts STA-01 and STA-02 (the `nreq` list and the header comment), so the accepted set is also the must-be-covered set. `data/expected/important-gates.ids` gains a comment line naming this plan and the 22 lines of the plan, each resolving to an id of its inventory (the faults of `data/expected/faults/state.ids`, the judges as `state.judge.<slug>` ids of `data/expected/tests/state.ids`, the controls of `data/expected/controls/state.ids`):

| Gate | Mechanism | Ids | Requirement |
|------|-----------|-----|-------------|
| `sta01-serializer-faults` | fault | SNAPSHOT_LITTLE_ENDIAN, SNAPSHOT_PAD_SERIALIZED, SNAPSHOT_IMAGE_BYTES_EMBEDDED | STA-01 |
| `sta01-reader-faults` | fault | SNAPSHOT_TRAILER_UNCHECKED, SNAPSHOT_LENGTH_TRUSTED | STA-01 |
| `sta01-restore-faults` | fault | RESTORE_MUTATES_BEFORE_VALIDATE, RESTORE_PROFILE_UNCHECKED, RESTORE_IMAGE_DIGEST_UNCHECKED | STA-01 |
| `sta01-judges` | mutant | `state.judge.header-literal-corrupt`, `.matrix-expectation-corrupt`, `.refusal-expectation-corrupt`, `.fuzz-invariant-detects` | STA-01 |
| `sta02-hardpoint-mechanisms` | fault, mutant | SNAPSHOT_STIC_ROWS_OMITTED, `state.judge.cut-compare-detects-divergence`, `state.judge.row-sweep-detects-omission` | STA-02 |
| `sta02-checklist-controls` | control | `state-checklist-row-missing`, `-line-extra`, `-witness-unknown`, `-test-unknown`, `-exempt-no-reason`, `state-schema-literal-stale`, `state-class-source-drift` | STA-02 |

The step reads `gates=89 lines=304` (83 plus 6, 282 plus 22; the derived figures of the plan held exactly, no difference). It can fail: on a scratch copy of the registry with one judge renamed to an id that is in no inventory and the STA-02 lines removed it reports `FAIL important-gates-uncovered-gate: STA-02` and `FAIL important-gates-unknown-id: mutant state.judge.no-such-judge line 380`; the ten `important-gates-*` controls still report `control ok`.

The ten toy registries (the known-good tree and the nine overlays that carry the toy VID-05 line, the ten files `grep -rl toy-vid05 tools/check/controls` lists) each get a comment line and `toy-sta01 fault GOOD_FAULT STA-01` and `toy-sta02 fault GOOD_FAULT STA-02` after the toy VID-05 line, which is the last line of every one of them, so no line number that an EXPECT names (`line 19`, `line 24`) moved. `git diff --name-only` from the commit of the blind-read record, restricted to `tools/check/controls` and the word `important-gates`, lists those ten files and no CHECK, EXPECT or FORBID file (the grep of the acceptance criterion prints 0).

## The requirement marks and the end state of wave B

STA-01 and STA-02 are marked complete in `.planning/REQUIREMENTS.md` (the two checkboxes and the two traceability rows), with the Phase 8 wave B close-out note in the style of the VID-05 note: on these terms only, one provisional claim at the tier `deterministic-repeatability`, one build and one profile, one author and a same-family blind reader of the checklist, the nine exemptions with the list of plan 08-14, the two pinned acceptances, the informational digests and the trailer that is integrity and not authenticity, byte-by-byte encoding with no big-endian run, no file persistence and the optional file options deferred, the applied-event count without a getter, nothing measured. The conditions of the plan are met and checked by command: `grep -v '^#' data/state/tests-pending.ids | grep -c .` prints 0; `grep -v '^#' data/expected/state-cuts.ids | grep -c pending-` prints 0; the step `state` reports `state-rows=88 state-cuts=18` and holds that every checklist row names a registered test id (the control `state-checklist-test-unknown` shows it can fail); and `git diff 02899e1 -- docs/formats/ivsn-v1.md data/state/schema.literal.txt data/expected/state-checklist.ids` removes 0 lines (02899e1 is the oracle commit of the blind-read record; the specification has only added lines since, the other two files are unchanged). PRF-01 and REL-01 are untouched (Pending); no other row moved.

`docs/state/hidden-state-checklist.md` gains an appended section, As built (plan 08-17): the end state (no pending line, 18 pinned cuts, the registry gates that answer the adversarial list, the claim) and the limits listed and not hidden. The frozen sections and the checklist file are untouched.

## Checkpoint C (plan 08-17)

The entry is `## 2026-11-14 Phase 8 checkpoint C (plan 08-17)` in `.planning/CI-EFFICIENCY.md`, with exactly one line starting `checkpointC-08: `. The five last successful rows of the working-tree `.planning/ci-runs.tsv` (63 lines; nothing staged, no `gh`, no `tools/ci-record.sh`), recomputed with awk:

| Run | head | `build_s` | `gate_s` | `cpu_s` | `r` | contains wave B |
|-----|------|-----------|----------|---------|-----|-----------------|
| 43593328300 | 9bd9bc1 | 59 | 151 | 482 | 8.17 | no |
| 43616659462 | c57328a | 63 | 156 | 503 | 7.98 | plans 08-09 to 08-12 |
| 43658364307 | 5d56e21 | 46 | 109 | 335 | 7.28 | to plan 08-13 |
| 43669236621 | 2eaab6e | 44 | 112 | 346 | 7.86 | to plan 08-15 |
| 43674301502 | 1a27cdd | 52 | 134 | 431 | 8.29 | to plan 08-16 |

**Median `r` 7.98 (7.9841).** Four rows contain wave B (`git merge-base --is-ancestor 06d4820`), so by the rule the projection is the median. It is 0.02 below the reading line 8.0 and 0.32 below the line 8.3: neither condition of the rule (a projection above 8.3, or a median of 8.0 or more) is met. **`checkpointC-08: no-lever`.** Said as plainly in the entry as here: the newest row alone reads 8.29, and with what no row holds (this plan's control and the planned waves C and D, about +0.4 local) it reads 8.30 to 8.31, on the line and not under it; the median row counted for what it lacks reads 8.18 to 8.27, or 8.19 to 8.29 with waves C and D; the two steps between the three newest rows were +0.58 and +0.43 where the unit costs expected +0.21 and +0.05, which is runner speed (`build_s` 44 to 52 for much the same compile) more than code. A fact of the window, not used for the answer: the next recorded row cannot raise the median above 7.98, and the median can read above 8.3 only from the third row on, if all three do.

Measured on this plan's tree (`/usr/bin/time -p`, user plus sys cpu, three runs each, factor 1.675 of the record): suite `state` 3.30 local cpu-s on the dev preset and 1.94 on release (8.78 hosted, against 0.85 at the unit cost of the record); the nine fault runs 0.49, 0.38, 0.17, 0.85, 0.79, 0.81, 0.82, 0.80 and 0.39, sum **5.50, under the target of about 6** (9.21 hosted; the record's unit gives 9.0); step `state` 0.09 (dash 0.08); step `important-gates` 0.14; `ivs-run` launches of `runner-process` 0.59 dev and 0.01 release; the runner ids and fault executables 0.36 (plan 08-16's measurement); step `pkg-install` 3.28 whole, about 0.2 of it added by the fifth replay. The whole of wave B at these figures is about **21.4 hosted cpu-s** (80 ids, 9 faults, 8 controls, the step, the replay), of which about 8.8 is in the median row.

Stop rule and levers, as the entry states them: a last-five median above 8.3 means the orchestrator creates a lever plan and no further test case is added until it lands. The remaining plans (08-18 to 08-21) add about 0.4 local by estimate and are bound to it: any id, control, fault or step beyond their plans recounts before it commits. Lever order: (2) the mutant inventories on the release preset only is applied; **(3) folding `demo-lamp-tone-swap` or `demo-lamp-tone-shift` into repeat variants of one mutant is next (queued, untouched, saving not measured)**; then the owner-decision levers (fault objects at `-O0`, one runtime-selected object per owner translation unit, a two-job split under D12). First trims of wave B, if a median reads above 8.3: the four dev-preset launches of `runner-process.roundtrip.refuse-range` made static (0.24), the cut-cap id on the dev preset (0.09), the digests of every 8th refusal in `state.restore.atomic-every-refusal` (0.4) and the second value of the unsealed byte flips in the payload (0.4), together about 1.1 local cpu-s (0.03 to 0.04 in `r`): too small to move a median by more than one row's spread, so they are named for the order and do not carry the answer. Not levers: dropping an oracle, a judge or a pinned-default test, caching.

## The persistence-policy decision (open question 8), for the Decision Register

Decided here as a delegated decision, to be written into the Decision Register by the close-out plan with this section as its source:

> **Snapshot persistence policy (OQ8, Phase 8, D-19).** The core returns bytes and nothing else: it writes no file and chooses no name, slot, location, compression, encryption or autosave, and no machine behaviour depends on a file. Where a snapshot goes is a host's policy; the runner has no snapshot file option in this phase (`--snapshot-out` and `--snapshot-in` are deferred, and when added are runner-only, take an explicit path and are refused for a `content user` scenario). The format IVSN v1 is exact-build and not archival: one build restores exactly one pair of format version and schema version and refuses any other as UNSUPPORTED, with no migration before 1.0. `.ivsn` is rights-bearing by name (`tools/check/binary-ext.txt`, `.gitignore`, the control `files-unlisted-binary-ivsn`), because a snapshot of a run with owner-supplied images is derived material. Reversal evidence: an owner or integrator need for archival snapshots (a migration story and a versioning policy), for a default location, or for battery-backed cartridge RAM (add a section and raise the section count and the schema version together); reversible by adding sections and bumping the schema.

## Deviations from Plan

**1. [Interpretation] The specification gets an as-built section and its frozen Rights section is not edited.** The plan allows a sentence in the Rights section "below the frozen sections only if it adds a fact not already there (no edit of a frozen line)". The facts that are new (the rule is in place, the control exists, the policy) fit no frozen paragraph without editing one, so they are a new section at the end, Rights (as built, plan 08-17), in the manner of the other as-built sections. The commit that carries it is Task 1 (the plan lists the file under Task 3); it moved with the rule it describes (lesson 1).

**2. [Interpretation] The note of CL-0028 and the close-out cite the exemption list of plan 08-14 as `data/expected/state-sweep-notes.ids`.** The frozen checklist file holds no exemption (08-14 deviation 2: the edit was refused and the freeze forbids it), so the claim, the REQUIREMENTS note and the checklist as-built section say where the list is and that the checklist file carries none; the `EXEMPT:` count check of plan 08-14 is not part of this plan's verify.

**3. [Interpretation] The fault runs are priced from `build/dev/tests/ivs_fault_<ID>` three times each.** The plan asks for the unsanitized `state` binary of a fault build times the factor; these are the executables of the gate's own `ivs_faults` target (the ones the `c-faults` step runs, with the results directory and the nonce replaced by scratch values), so the figure is the step's per-fault cost.

**4. [Information] The counts moved with their tasks.** `controls=379/379` from the first commit, `claims=28 provisional=27` from the second, `gates=89 lines=304` from the third; every task's own gate had the counts of its commit (sh and dash, identical summary lines).

**5. [Information] STATE.md and ROADMAP.md are edited by hand by the closing commit,** as plans 08-14 to 08-16 did, and not by the gsd-tools plan-complete verbs.

**6. [Information] A scratch draft of this SUMMARY was refused by the tool layer ("subagents should return findings as text, not write report files").** It was not retried by another route; this file at its repository path is the plan's required output and was written directly.

**7. [Information] The container gate was not completed on the closing tree.** The Task 3 tree passed in the container (exit 0, `gate: PASS`, the plan-end counts, 0 `FAIL` lines, 332 s). On the closing tree three attempts were ended by the environment: the first stopped making progress inside the `c-tests` step (an `ivs_asm` process of the `asm` suite idle at 0 percent cpu for 13 minutes under the Rosetta layer of the docker host; killed, exit 137), the second and third never left the container's `apt-get install` (a few hundred kilobytes received in 9 to 14 minutes while the host reached the Ubuntu mirrors normally; killed, exit 137). A mistyped command of mine had also started a stray second container gate on the Task 3 tree, which I killed after about 7 seconds. The EXEC-RULES line applies: this is written as a visible not-run line in the final gate section and as open item 10, not as a pass. No gate step failed and nothing was weakened.

No auth gates, no package installs (T-08-SC), no `gh` call, no push, no tag, no `tools/ci-record.sh`; `.planning/config.json`, `.planning/state.json` and `.planning/ci-runs.tsv` were never staged. No C file, no `core/`, `runner/`, `tests/` or `fixtures/` file changed in any commit of the plan (`git show --stat` of the three code commits lists none) and no golden changed. D10 and D11 are not performed and no claim above provisional exists: every statement is about the repeatability of one build and one profile, and nothing is measured on hardware.

## Decisions Made

- A rights rule is as strong as its control: `ivsn` is in the list that four rules read and in `.gitignore`, and the control was shown to fail when the extension is removed.
- A claim says what its oracles show and lists, in its own note, every limit the close-out lists elsewhere, so a reader of the CHANGELOG table or the ledger finds the exclusions where the claim is.
- A registry line is only as good as its mechanism: each of the 22 resolves to an inventory id, the step fails a renamed one, and the toy registries make the accepted set coverable in every control tree.
- The cost rule is applied as written and its margin is stated as it is: no lever by the rule, nil margin by the other readings, the remaining plans bound to their estimate and the lever order on record.

## Cost (laptop)

| Item | Value |
|------|-------|
| estimate of the plan | 100,000 tokens (raw, confidence low), 3 tasks |
| actuals | 5,004 tokens (chars/4 over the added lines of the 3 commits outside `.planning/`), 3 tasks, 3 commits (measured from the ledger of the plan head) |
| gate total, sh and dash, Task 1 tree | 86 s and 85 s, identical summary line (md5 7a646a7a61c863cd31b645c2210b47c1) |
| gate total, sh and dash, Task 2 tree | 81 s and 83 s, identical summary line (md5 a1f6931c3873228d21f093440e4a1c3f) |
| gate total, sh and dash, Task 3 tree | 83 s and 85 s (`gate-cpu` user 143.9 sys 116.9 and user 145.0 sys 119.3), identical summary line (md5 e0a4b4c3198dfa521e151c37abf00d6b) |
| container gate (gcc 13.3.0, cmake 3.28.3, x86_64) | Task 3 tree: exit 0, `gate-time total=332` (`gate-cpu` user 1,188 sys 71 emulated), 0 FAIL lines; closing tree: not completed, see the final gate section (three attempts ended by the environment) |
| the whole control set after Task 1 and Task 3 | `controls=379/379`, every earlier control `control ok` |
| the checkpoint C measurements | the table of the entry (about 3 minutes of cpu in all, at a load average of 12 to 14) |

## Open Items

1. **The applied-event count of an instance is not reported by any public call** (open item 1 of plan 08-16, deviation 6 there). The close-out records it: a host keeps the count its run calls reported next to the snapshot. A getter (in `ivs_info` or the snapshot report) is an API addition (a reasons or struct-size change) and is left to a later phase or to the owner.
2. **The optional snapshot file options `--snapshot-out` and `--snapshot-in` are deferred (D-18).** No code writes or reads a snapshot file; when added they are runner-only, take an explicit path and are refused for `content user`; the policy text above is the decision they would implement. The close-out record of plan 08-21 lists them as deferred.
3. **The two known acceptances stay pinned, not closed** (plan 08-15): a STIC register word with a bit outside its write mask (a candidate INV-7, derivable and provable on the 411,075 boundaries, but it changes the core and the invariant function) and machine time not tied to a unit boundary. The claim note, the REQUIREMENTS note and the checklist as-built section name them.
4. **`stic.late_grants` has no non-default witness among the 18 cuts, and `obs.input.words` is exempt only while the next event lies inside the window** (plan 08-14). A designed program with a non-interruptible stretch of at least 1,026 cycles would witness the first and let its exemption go.
5. **The consumer is blind to a restore that skips the STIC register words, no cut is made at the end of a run-to, and the refused-save, -reset and -restore paths of the hook have no test that reaches them** (plan 08-16 open items 2 to 4); the state suite and the two STIC process cases see the first.
6. **The checklist file carries no `EXEMPT:` line** (plan 08-14 deviation 2): the nine exemptions are in `data/expected/state-sweep-notes.ids`. Folding the notes into the checklist is one mechanical commit if the freeze is ever lifted.
7. **Cost:** the line is `no-lever` by the rule at a nil margin (the newest row plus the remaining additions reads 8.30). Waves C and D are bound to about 0.4 local cpu-s. If a hosted median reads above 8.3 the lever order is (3) first, then the owner-decision levers; the four wave B trims are named in the entry and are worth about 0.03 to 0.04 in `r`.
8. **The `depends_on` list of CL-0028 is a judgement** (51 profile members chosen by what the programs of the cuts and the diagnostics touch); a field that a later program starts to depend on belongs to a new claim, not a silent edit of this one.
9. **Information.** D10 and D11 are not performed; no claim above is more than provisional; no hosted-only check was run (the hosted-style container gate ran green); PRF-01 and REL-01 stay Pending (D-28).
10. **The container gate on the closing tree was not completed** (deviation 7): `GATE_LINUX_PLATFORM= sh tools/gate-linux.sh` should be repeated by the orchestrator once the docker network answers; the Task 3 tree, which holds every code, data, registry and docs file of the plan, passed in the container (332 s, 0 `FAIL` lines).

## Interfaces for the next plans

- Wave C (benchmarks): the snapshot workload uses `ivs_runner_options.roundtrip_ticks` with the flag `IVS_RUNNER_OPT_ROUNDTRIP` and the cap `IVS_RUNNER_ROUNDTRIP_MAX_CUTS` (64 cuts, a harness failure beyond), or the four public calls directly (`ivs_machine_snapshot_size`, `_save`, `_restore`, `ivs_snapshot_describe`); the snapshot is 4,299 bytes, a cut is made at a unit boundary of a powered machine, and a restored run equals the straight run (the digest of `bench-lamp-snapshot` equals the idle digest).
- The state suite names (`state.literal.`, `.size.`, `.save.`, `.read.`, `.restore.`, `.witness.`, `.cut.`, `.sweep.`, `.fuzz.`, `.judge.`; 61 ids) are in `data/expected/tests/state.ids`; the nine faults are in `data/expected/faults/state.ids`; the seven controls are in `data/expected/controls/state.ids`.
- PRF-01 is not accepted by `important-gates.sh` yet: plan 08-19 adds it to the `nreq` list (which now ends `SCN-01 SCN-02 STA-01 STA-02`) with its registry lines and one toy line in each of the ten toy registries, the way this plan did for STA-01 and STA-02.
- Counts after this plan: `claims=28 provisional=27 gates=89 lines=304 controls=379 faults=235 tests-expected=4932 tests-executed=4928 tests-skipped=4 goldens=27 pkg-selftest=19/19 state-rows=88 state-cuts=18`; the next plan's gate starts from them.
- Threat ids T-08-24 to T-08-40 are used by the state wave (T-08-39 and T-08-40 here, T-08-SC throughout); the next plans continue from T-08-41 or the numbering their plans assign.
- `.planning/CI-EFFICIENCY.md`: the entry above is the cost position at the end of wave B; the `phase8-close` entry of plan 08-21 reads the next hosted rows.

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=38/38 controls=379/379 ... faults=235/235 ... fault-runs=235 ... tests-expected=4932 tests-executed=4928 tests-skipped=4 tests-missing=0 ... claims=28 provisional=27 ... gates=89 lines=304 ... goldens=27 ... pkg-selftest=19/19 ... state-rows=88 state-cuts=18 (0 FAIL lines)
dash tools/gate.sh:    the identical summary line (0 FAIL lines)
GATE_LINUX_PLATFORM= sh tools/gate-linux.sh: Task 3 tree (the plan-end code, data and registry tree): exit 0, gate: PASS, the plan-end counts, 0 FAIL lines, gate-time total=332; CLOSING TREE: NOT RUN TO COMPLETION (three attempts ended by the environment, see below)
```

After the Task 1 tree: `controls=379/379`, sh 86 s and dash 85 s with the identical summary line (md5 7a646a7a61c863cd31b645c2210b47c1). After the Task 2 tree: `claims=28 provisional=27`, sh 81 s and dash 83 s, identical (md5 a1f6931c3873228d21f093440e4a1c3f). After the Task 3 tree: the plan-end counts `gates=89 lines=304` under sh (83 s) and dash (85 s) with the identical summary line (md5 e0a4b4c3198dfa521e151c37abf00d6b), and the container gate on the same tree: exit 0, `gate: PASS` (gcc 13.3.0, cmake 3.28.3, x86_64), the plan-end counts, `core-imports=5`, 0 `FAIL` lines, `gate-time total=332` (`gate-cpu` user 1,188 sys 71 emulated). On the closing tree (this SUMMARY, STATE.md and ROADMAP.md added or edited; `tracked=2467`): sh (`gate-time total=76`, `gate-cpu` user 139.0 sys 113.0) and dash (76 s, user 139.4 sys 112.1) `gate: PASS` with the identical summary line (md5 9e3910124561477a327fe249c02528f1), 0 `FAIL` lines and the plan-end counts above (a first closing run, with the placeholders of this file still in it, read sh 86 s and dash 107 s with the same line while a container gate ran), and `GATE_LINUX_PLATFORM= sh tools/gate-linux.sh` on the closing tree: **not run to completion, a visible not-run line and not a pass** (three attempts, each ended by the environment, below). The container pass of the plan stands on the Task 3 tree above, which holds every code, data, registry and docs file of the plan; the closing tree adds only this file, STATE.md and ROADMAP.md, which the sh and dash gates just quoted do cover (the steps `hygiene`, `integrity`, `files` and `glossary` read them) and which no container-specific check reads differently. Environment incidents, none a gate result: a mistyped command of mine started a second container gate on the Task 3 tree that I killed after about 7 seconds (it truncated `build/gate-linux.log`, whose Task 3 result had been read from the wrapper output and is quoted above); the first closing attempt stopped making progress inside the `c-tests` step (an `ivs_asm` process of the `asm` suite under `ctest --preset dev -j 8` idle at 0 percent cpu for 13 minutes under the Rosetta layer of the docker host) and was killed (exit 137); the second and third attempts never left the container's `apt-get install` (a few hundred kilobytes received in 9 to 14 minutes, with another container of the host stalled the same way, while the host itself reached the Ubuntu mirrors in 0.6 s and 4 s) and were killed (exit 137). `build/gate-linux.log` therefore holds a killed run and is not evidence; the orchestrator should repeat `GATE_LINUX_PLATFORM= sh tools/gate-linux.sh` once the docker network answers (open item 10).

## Known Stubs

None.

## Threat Flags

None new. T-08-39 (a snapshot of an owner-image run committed by accident): `ivsn` in the binary list that four rules read and in `.gitignore`, a control shown to fail without the list line, the writer tested for no image window (plan 08-11), no code that writes a file, and the statement in `docs/owner-images.md`. T-08-40 (overclaim in CL-0028 and the close-out note): one provisional claim at the tier `deterministic-repeatability`, an `Excluded on purpose` list that names every limit, the exemption count and the list, the same-model-family caveat, the informational digests and the meaning of the trailer in the note, the text step over CHANGELOG and docs (claim words tagged), and the completion note that states the terms. T-08-SC: no installs.

## Self-Check: PASSED

Verified before the final commit: the SUMMARY and the created files of the plan exist (FOUND for `tools/check/controls/files-unlisted-binary-ivsn/CHECK`, `data/claims.json`, `data/expected/important-gates.ids`, `.planning/CI-EFFICIENCY.md` and the SUMMARY); commits `7f981d6`, `da36670` and `027605e` are ancestors of HEAD and `git rev-list --count 1a27cdd5ad1ee8c9ff9b5ab94316f4e73e18c52d..HEAD` printed 3 before this file's commit; nothing is staged but the files of this commit and `.planning/config.json`, `.planning/state.json` and `.planning/ci-runs.tsv` are not staged; `git ls-files` lists no `.ivsc`, `.bin`, `.rom`, `.exec`, `.ivsn`, `.wav`, `.ppm` or `.vraw` file; the diff of the three commits outside `.planning/` holds no home, scratch or temp path, no at sign before a dotted word and no "Intellivision"; no commit deletes a file and none touches a `core/`, `runner/`, `tests/`, `fixtures/` or `data/golden/` file; `grep -c '^checkpointC-08: ' .planning/CI-EFFICIENCY.md` prints 1; `grep -c ' STA-0[12]$' data/expected/important-gates.ids` prints 22 and `grep -c 'sta0[12]-'` prints 22 with each of the six gate names present; `grep -v '^#' data/state/tests-pending.ids | grep -c .` and the count of `pending-` ticks in `data/expected/state-cuts.ids` print 0; `cmake -DROOT=. -P tools/check/claims-table.cmake | grep -c '^| CL-'` prints 28 and `grep -c '"standing": "provisional"' data/claims.json` prints 27; no CHECK, EXPECT or FORBID file of an `important-gates-*` control changed since the blind-read record; no claim is at the `measured-hardware` tier; the claim words of the text step do not occur untagged in the new text of `docs/`, `CHANGELOG.md` and `README.md` (the gate's `text` step passes).
