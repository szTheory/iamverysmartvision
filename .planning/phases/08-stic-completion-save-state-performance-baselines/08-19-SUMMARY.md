---
phase: 08-stic-completion-save-state-performance-baselines
plan: 19
subsystem: Phase 8 wave C, plan 2 of 2 - the snapshot benchmark workload, the benchmark document, PRF-01 accepted by the important-gates step; the baseline record (Task 3) is pending the orchestrator's input file
tags: [C, PRF-01, bench, ivs-bench, snapshot-workload, docs, important-gates, D-23, D-24, D-25, D-39, D-43, T-08-43, T-08-44]
status: partial - Tasks 1 and 2 complete and committed; Task 3 (the baseline record, its format id, the PRF-01 mark, the final gates and the closing STATE, ROADMAP and REQUIREMENTS edits) is NOT done: baseline-input-missing

requires:
  - phase: 08-stic-completion-save-state-performance-baselines
    provides: "plan 08-18 end state: steps=38/38 controls=380/380 faults=235/235 fault-runs=235 tests-expected=4952 tests-executed=4948 tests-skipped=4 tests-missing=0 claims=28 provisional=27 gates=89 lines=304 goldens=29 pkg-selftest=19/19 state-rows=88 state-cuts=18 (taken from the 08-18 SUMMARY; the plan-start gate was not re-run, deviation 5)"
provides:
  - "tools/bench/bench.c: a workload row with a non-zero roundtrip_ticks passes the cadence to every execution (iteration 0, warmup and timed iterations) through IVS_RUNNER_OPT_ROUNDTRIP and the roundtrip_ticks option of the runner; a run in which the hook made no cut is a harness failure (exit 14, one line); the plan-08-18 refusal of a cadence is gone"
  - "data/bench/workloads.tsv: bench-lamp-snapshot (scenario bench-lamp-idle, 3 warmup, 15 iterations, roundtrip_ticks 1,792,080, ticks 7,168,352), no scenario and no golden added; its digest precondition is data/golden/bench-lamp-idle.digests"
  - "suite bench: bench.verify.bench-lamp-snapshot (static counts equal those of the idle row) and bench.workloads.table-consistent (files and text only; three broken scratch tables each must produce the finding of their rule); 10 ids, 20 with the presets"
  - "docs/bench.md: definitions, the 22 columns in three groups plus provenance, how to run and read it, refusal and exit 15, the snapshot workload, the repeatability-only statement, the gate rule, CI time as a pointer, the baseline procedure"
  - "tools/check/steps/important-gates.sh accepts PRF-01; data/expected/important-gates.ids +2 lines over the gate prf01-digest-first (gates 89 to 90, lines 304 to 306); toy-prf01 in each of the ten toy registries"
affects: [08-20, 08-21]

actuals:
  tokens: 6402
  tasks: 2
  commits: 2
plan_head_before: 8453283a2b439cc20900b7d6446571ff272d8890
plan_head_after: 8f61ecd8c32bcbaaca58827bab46da010c7ab093

key-decisions:
  - "The snapshot workload needs no new scenario and no new golden: the idle scenario run with a save, a reset and a restore every 30 frames must produce the idle report, so the digest-first rule of the tool proves the restore equal before any figure of the snapshot workload can be printed"
  - "A cadence that produces no cut is a harness failure of the tool, not a silent no-op: without it a stand-in that never hands the cadence to the runner would verify (the report equals the idle report) and measure nothing about snapshots"
  - "The table-consistency id carries its own judge (three scratch tables broken one way each) at the cost of a few awk launches and no tool launch, so the rules are shown able to fail inside the id and no id was added"
  - "PRF-01 gets no claim row: the registry names the two mechanisms of the digest-first rule, the document says a figure is never evidence"

key-files:
  created:
    - docs/bench.md
    - .planning/phases/08-stic-completion-save-state-performance-baselines/08-19-SUMMARY.md
  modified:
    - tools/bench/bench.c
    - data/bench/workloads.tsv
    - tests/bench-tap.sh
    - data/expected/tests/bench.ids
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

requirements-completed: []

completed: pending (Task 3)
---

# Phase 8 Plan 19: the snapshot workload and the benchmark document Summary (partial: Tasks 1 and 2)

**`ivs-bench` now hands the `roundtrip_ticks` cadence of a table row to every execution of that workload, so `bench-lamp-snapshot` (the idle scenario with a save, a reset and a restore every 30 frames, three cuts in the 120-frame run) is verified in every gate run against the idle golden before any figure can be printed; `docs/bench.md` says what the numbers are and are not; the important-gates step accepts PRF-01 with two registry lines that name the digest-first mechanisms, and each of the ten toy registries covers it. The gates pass under sh and dash on the Task 1 tree and on the Task 2 tree, and under the Ubuntu container on both, with `steps=38/38 controls=380/380 faults=235/235 fault-runs=235 tests-expected=4956 tests-executed=4952 tests-skipped=4 tests-missing=0 goldens=29 claims=28 provisional=27 gates=90 lines=306`. The plan is NOT complete: Task 3 (the baseline record taken from `build/bench/baseline-input.tsv`, its format id, the PRF-01 mark, the final gates at 4958 and 4954, STATE.md, ROADMAP.md and REQUIREMENTS.md) stopped at its precondition: `baseline-input-missing`.**

## Commits

| Commit | Task | What |
|--------|------|------|
| `2cdcc60` | 1 (tracer) | the cadence wiring and the zero-cut guard in `tools/bench/bench.c`, the row `bench-lamp-snapshot`, the ids `bench.verify.bench-lamp-snapshot` and `bench.workloads.table-consistent` |
| `8f61ecd` | 2 | `docs/bench.md`, PRF-01 accepted by `important-gates.sh`, the two registry lines, the ten toy lines |
| the commit that adds this file | | this SUMMARY only (partial state; STATE.md, ROADMAP.md and REQUIREMENTS.md are left to the Task 3 closing commit) |

## The snapshot workload (Task 1)

`bench_run` zeroes the options struct as before and, for a row with `roundtrip_ticks != 0`, sets `opts.flags = IVS_RUNNER_OPT_ROUNDTRIP` and `opts.roundtrip_ticks`; the same struct goes to iteration 0, every warmup iteration and every timed iteration, so the round trip is part of what is verified and of what is timed. The golden is looked up by the row's scenario column, so the snapshot row compares with `data/golden/bench-lamp-idle.digests` and with the idle `ticks` (7,168,352). The refusal of plan 08-18 (`roundtrip_ticks other than 0 is not supported by this tool yet`) is removed. Nothing else in the digest-first flow changed and no file under `core/` or `runner/` changed.

**Cut count: 3.** Cuts at 30, 60 and 90 frames (multiples 1,792,080, 3,584,160 and 5,376,240 of machine time); the fourth multiple, 7,168,320, is the end bound of the run-to and the hook makes no cut there. Read from `ivs_runner_result.roundtrip_cuts` by a scratch build of the tool (outside the repository, not committed) that printed the field after iteration 0: `cuts=3`.

**Verification row** (dev preset, `build/dev/tools/bench/ivs-bench --verify-only --root . --content-dir build/dev/images --workload bench-lamp-snapshot`, exit 0, `<TAB>` stands for the tab character; the idle row has the same `ticks_per_iter`, `cpu_cycles_per_iter` and `frames_per_iter`):

```
bench-lamp-snapshot<TAB>verified<TAB>bench-lamp-idle<TAB>repeatability-only<TAB>AppleClang-21.0.0.21000101-Debug-5be05b529cd4<TAB>0<TAB>on<TAB>18<TAB>3<TAB>15<TAB>7168352<TAB>1792088<TAB>120<TAB><TAB><TAB><TAB><TAB><TAB><TAB><TAB><TAB>see-ci-runs.tsv
```

**One timed release run for information (not committed, not a baseline).** Release preset, a scratch table outside the repository (idle and snapshot rows, warmup 1, iterations 3), the host loaded (load average 17 on 18 cpus), exit 0, no stderr, `git status` identical before and after: the idle row read a median wall of 26.3 ms and the snapshot row 28.1 ms (about 1.8 ms more for the three round trips, about 7 percent), 14.7 and 15.7 ns per guest cycle. Three samples on a loaded host show that the timed path with the cadence works and nothing more; the baseline is the person-written record of Task 3.

**The ids.** `bench.verify.bench-lamp-snapshot` is `verify_case bench-lamp-snapshot bench-lamp-idle bench-lamp-idle`: the case now keeps the output of each workload (`$tmp/v.<workload>.out`) and, given a peer, requires columns 11 to 13 (`ticks_per_iter`, `cpu_cycles_per_iter`, `frames_per_iter`) of its row to equal the peer's and not be empty. `bench.workloads.table-consistent` is `table_findings`, shell only, over files and text: seven columns per row, a unique workload id, the scenario file, the line of `data/expected/runner-scenarios.ids`, the golden, warmup and iterations positive integers, `roundtrip_ticks` 0 or at least a 64th of the run length (the runner's cap is 64 cuts), `ticks` equal to the golden's last `step.<k>.now_ticks`, every `bench-*` line of the inventory the scenario of some row, a comment line listing the columns. It reads the golden of each distinct scenario once.

**Shown able to fail** (each by a scratch stand-in or tree outside the repository, nothing committed):

| Mechanism | How it was made to fail | Result |
|-----------|--------------------------|--------|
| `bench.verify.bench-lamp-snapshot` and the zero-cut guard | a copy of `bench.c` without the two lines that set the flag and the cadence | `not ok 4 bench.verify.bench-lamp-snapshot`: exit 14, `ivs-bench: bench-lamp-snapshot: the round-trip hook made no cut` (the digest alone would have matched: the report equals the idle report) |
| the same guard, cadence beyond the run | a scratch table with `roundtrip_ticks` 900,000 on the 806,568-tick cold scenario | exit 14, one stderr line; the format line and header are on stdout, no row |
| `bench.workloads.table-consistent` and the per-row rules | a scratch copy of `data/` whose `bench-lamp-input` ticks were 7,168,353 | `not ok 10`: `bench-lamp-input: ticks 7168353 differs from the final machine time 7168352 of the golden`, and `not ok 2` (the tool refuses the same table: `ticks-mismatch table=7168353 run=7168352`) |
| the judge inside the table id | the three scratch tables (ticks plus one, the input row removed, the cadence 1) | each produces the finding of its own rule; the case reports `ok` only when all three do |

A cadence of 1 on the cold scenario is refused by the tool, not failed: the runner's cap of 64 cuts makes the run a harness failure whose report differs from the golden, so the tool prints `bench-refuse bench-lamp-cold digest-mismatch outcome` and exits 15 (the digest-first rule at work; not a committed case).

## The document and the registry (Task 2)

`docs/bench.md` holds the third truth of the plan: definitions (workload, verification iteration, warmup iteration, timed iteration, fixed counts, median), the 22-column table with group, unit and computation, the run instructions with repository-relative paths and the release preset, the reading rules (CPU time for trend and regression, wall time for latency; **`lat_first_ms` is the first TIMED iteration of a warm process, not a cold-start figure**, as the 08-18 SUMMARY required; about 43 percent of a verified run is SHA-256 over every frame twice, measured once on a loaded host during the planning of this phase, so indicative only, and there is no switch to remove it), the refusal reasons and exit 15, the snapshot workload, no CPU-bound label and why, the `repeatability-only` label that is neither an evidence tier nor a claim (D10 and D11 untouched), that the gate never reads a timer and never compares a figure with a threshold, CI time as a pointer to `.planning/ci-runs.tsv`, the baseline procedure and the unimplemented local-only comparison idea. The text step found three untagged claim words (the pointer text `see-ci-runs.tsv` twice and the word that hands the cadence to the runner); two lines carry `[nc]`, one was reworded; no bare clock word occurs. The section "The committed baseline" says that no record is committed yet: Task 3 replaces that paragraph with the file name, host class, date and load statement.

`tools/check/steps/important-gates.sh` accepts PRF-01 (the `nreq` list and the header comment). `data/expected/important-gates.ids` gains a comment naming this plan and D-25 and the two lines `prf01-digest-first mutant bench.mutant.refuse-mutated-golden PRF-01` and `prf01-digest-first mutant bench.judge.refuse-has-no-timing PRF-01`; the step reads `gates=90 lines=306` (89 plus 1, 304 plus 2), exactly as derived. The ten toy registries (the known-good tree and the nine overlays that `grep -rl toy-sta01 tools/check/controls` lists) each get a comment and `toy-prf01 fault GOOD_FAULT PRF-01` after the last line, so no line number that an EXPECT names moved; no CHECK, EXPECT or FORBID file changed, and `sh tools/check/run-controls.sh` read `controls=380/380` with the ten `important-gates-*` controls `control ok`. It can fail: on a scratch copy of the registry without the two PRF-01 lines the step reports `FAIL important-gates-uncovered-gate: PRF-01`, and with the judge id renamed `FAIL important-gates-unknown-id: mutant bench.judge.no-such-judge line 396`. No claim row exists for PRF-01 (`git show --stat HEAD` of Task 2 lists no `data/claims.json`).

## Task 3: pending - `baseline-input-missing`

The plan stopped after Task 2 because the precondition of Task 3 is not met: `build/bench/baseline-input.tsv` does not exist (`ls build/bench` finds no directory). It was not measured here and no dev-preset or loaded-host run was substituted (the host is loaded: load average 17 to 21 on 18 cpus during this run, partly this plan's own gates). Nothing of Task 3 was started: no `data/bench/baseline-*.tsv`, no `bench.baseline.record-format` id, no mark of PRF-01 in REQUIREMENTS.md, no STATE.md or ROADMAP.md edit.

What the orchestrator must run (from `<repo-root>`, on an idle host, after the container gate of this plan has finished; the release preset, no sanitizers):

```
uptime
cmake --preset release
cmake --build --preset release
mkdir -p build/bench
build/release/tools/bench/ivs-bench --root . --content-dir build/release/images --workloads data/bench/workloads.tsv > build/bench/baseline-body.tsv || echo "ivs-bench failed: no baseline"
{ printf '# host: darwin arm64, 18 cpus\n# idle: <the load statement: what else was running and the load average>\n# date: <ISO date>\n'
  cat build/bench/baseline-body.tsv; } > build/bench/baseline-input.tsv
```

The workloads table path is `data/bench/workloads.tsv` (the default), the output has the format line, the header and four `measured` rows (idle, input, cold, snapshot) in table order; the three comment lines name a host class and the load only (no machine name, user, path or address). Task 3 then resumes with the rules of its first action. Interface for the resumed Task 3: the exact header is the `cols` line of `tests/bench-tap.sh` (22 columns); a timed row says `measured`; `bench.baseline.record-format` goes before the final TAP block of `tests/bench-tap.sh`, registered in `data/expected/tests/bench.ids`; the paragraph to replace in `docs/bench.md` begins `**The committed baseline.**`; REQUIREMENTS.md has the PRF-01 checkbox (line 107), its traceability row (line 231) and the close-out note style of STA-01 and STA-02 at line 242; the closing STATE.md and ROADMAP.md edits are by hand as plans 08-14 to 08-18 did (the ROADMAP line is `- [ ] 08-19-PLAN.md`); gate counts at plan end are `tests-expected=4958 tests-executed=4954`.

## Deviations from Plan

**1. [Rule 2 - missing critical functionality] A zero-cut guard in the tool.** The plan wires the cadence and relies on the digest. A tool that failed to hand the cadence on would still match the idle golden and would then time an idle run under the name of the snapshot workload. `bench_run` therefore requires `res.roundtrip_cuts != 0` after iteration 0 for a row with a cadence (a harness failure otherwise, after the digest and ticks checks so the refusal order is unchanged). Four lines of C, shown able to fail above, no new id (the existing verify id fails when it fires) and no launch added.

**2. [Interpretation] The table-consistency id includes its own judge.** The plan lists the rules; a check of rules that never fails proves nothing (lesson 7), so three scratch tables broken one way each must each produce the finding of their rule. The cost is a few awk launches, no tool launch and no new id.

**3. [Interpretation] The snapshot verify id compares its static counts through a saved output.** `verify_case` keeps each workload's output in `$tmp/v.<workload>.out` instead of one shared file and takes an optional peer; no extra launch of the tool.

**4. [Information] The `roundtrip_ticks` rule is the plan's (0 or at least a 64th of the run length, `ticks / 64` by integer arithmetic on the golden's final tick); it does not bound the cadence from above.** Zero cuts are caught by the guard of deviation 1, not by the table rule.

**5. [Information] The plan-start gate was not re-run.** The plan says to re-measure the counts with `sh tools/gate.sh` first; I took the 08-18 end-state counts and compared the Task 1 tree with the figures the plan derives (4952 to 4956 expected), which matched exactly (a drift would have shown there); the first gate of this plan is the Task 1 tree.

**6. [Information] The container gates ran on the uncommitted Task 1 tree and on the committed Task 2 tree.** The Task 1 container tree held exactly the four Task 1 files (the container reported `gates=89 lines=304`, the Task 1 counts); the Task 2 files were edited after the archive had been streamed.

No auth gates, no package installs (T-08-SC), no `gh` call, no push, no tag, no `tools/ci-record.sh`; `.planning/config.json`, `.planning/state.json` and `.planning/ci-runs.tsv` were never staged. No file under `core/`, `runner/`, `fixtures/` or `data/golden/` changed and no scenario was added (`git show --stat` of Task 1 lists none). D10 and D11 are not performed, no claim row exists for a benchmark figure, and no figure is quoted in the repository (the timed numbers above are in this file as information about a loaded host, not as a result).

## Decisions Made

- The digest of the snapshot workload is the idle golden: that is the whole proof that a save, reset and restore every 30 frames changes nothing observable, and it costs one more verification launch instead of a scenario and a golden.
- A guard that turns a silent no-op into a harness failure is cheaper than a test of the no-op: the existing verify id is the test.
- A figure is never a claim: PRF-01 is held by two mechanisms of the digest-first rule and a document, not by a claim row.

## Cost (laptop)

| Item | Value |
|------|-------|
| estimate of the plan | 90,000 tokens (raw, confidence low), 3 tasks |
| actuals so far | 6,402 tokens (chars/4 over the added lines of the 2 commits outside `.planning/`), 2 tasks of 3, 2 commits (measured from the ledger of the plan head) |
| gate total, sh and dash, Task 1 tree | sh `gate-time total=99` (wall 98.7 s, user 161.4 sys 138.5), dash 95.1 s (user 162.6 sys 138.6), identical summary line (md5 48e5afbec81465e5ca15468890224d02) |
| gate total, sh and dash, Task 2 tree | sh 106.0 s (user 171.0 sys 144.2), dash 87.9 s (user 158.7 sys 138.4), identical summary line (md5 0c38a50a0f4d834504bcce8ba47cf859); the laptop load average was 15 to 21 |
| container gate (gcc 13.3.0, cmake 3.28.3, x86_64, `GATE_LINUX_PLATFORM=`) | Task 1 tree: exit 0, `gate: PASS`, the Task 1 counts, 0 FAIL lines, `gate-time total=410`; Task 2 tree (tracked=2483): exit 0, `gate: PASS`, the Task 2 counts (`gates=90 lines=306`), `core-imports=5`, 0 FAIL lines, `gate-time total=355` |
| the `bench` ctest case (three runs each) | dev 1.55, 1.60 and 1.78 cpu-s (08-18: about 1.69), release 0.39, 0.38 and 0.38 (08-18: about 0.25) at a load average of 17 |
| the two new ids, by difference | `bench.verify.bench-lamp-snapshot`: dev about 0.19 cpu-s (0.11 user, 0.08 sys), release about 0.04; `bench.workloads.table-consistent`: about 0.03 per preset (shell and awk, five runs each way, release tool, dash); about 0.29 local in all |
| the added gate steps and controls | none; the important-gates step adds nothing measurable (a pure awk checker) |

**Cost position against the checkpoint C bound and the lines.** About 0.29 local cpu-s are added by Tasks 1 and 2 (the `bench-lamp-snapshot` launch dominates the dev preset, as predicted in the 08-18 SUMMARY), and Task 3's format id will add about 0.06 more (shell, both presets): about 0.35 local, 0.6 hosted at the factor 1.675, about +0.011 in `r` at a hosted `build_s` of 52. Against the last five hosted rows as the orchestrator gave them (median 7.86, newest 7.72, head ce469aa) plus the 08-18 reading of +0.075 for the bench suite, the projection after this plan is about 7.95 by the median and 7.81 by the newest row: under the line 8.3 by 0.35, under the reading line 8.0 by 0.05 (the margin the 08-18 summary named). The named unapplied trims of the 08-18 SUMMARY (the eight bad-argument vectors reduced to four on the dev preset, the one timed iteration of the mutant case on release only, one verify id sharing a launch with the format case) were not applied and were not needed to stay under the line; they remain available at the next checkpoint. The bound of checkpoint C for plans 08-18 to 08-21 (about 0.4 local) is exceeded by plan 08-18 alone; this plan does not add to the overrun beyond the 0.35 above.

## Open Items

1. **Task 3 is not done: `baseline-input-missing`.** The orchestrator takes `build/bench/baseline-input.tsv` on an idle host in the release preset (the commands above) and the plan is resumed at Task 3; PRF-01 is NOT marked complete, STATE.md, ROADMAP.md and REQUIREMENTS.md are untouched, and the id count of the plan end (4958 expected, 4954 executed) is not reached yet.
2. **The timed path has run on macOS arm64 only, on a loaded host,** and never on Linux; the gate runs the tool with `--verify-only` only. `lat_first_ms` is the first timed iteration of a warm process (stated in `docs/bench.md`).
3. **`docs/runner.md` does not list the golden-writing commands of the two bench goldens** (08-18 open item 6, unchanged); they are in the 08-18 SUMMARY.
4. **Information.** D10 and D11 are not performed; no claim above provisional exists and none concerns a benchmark; REL-01 stays Pending (D-28); the document tags every claim word `[nc]` or avoids it.

## Interfaces for the next plans

- Counts after this partial state: `steps=38/38 controls=380/380 faults=235/235 fault-runs=235 tests-expected=4956 tests-executed=4952 tests-skipped=4 tests-missing=0 goldens=29 claims=28 provisional=27 gates=90 lines=306`; Task 3 adds one test id (4958 and 4954).
- The suite names after this state: `bench.verify.<idle|input|cold|snapshot>`, `bench.mutant.refuse-mutated-golden`, `bench.refuse.wrong-ticks`, `bench.refuse.bad-args`, `bench.tsv.format`, `bench.judge.refuse-has-no-timing`, `bench.workloads.table-consistent` (10 ids); `bench.baseline.record-format` is the eleventh, Task 3.
- Threat ids: T-08-44 (the registry change) is mitigated here; T-08-43 (the committed baseline) is mitigated only when Task 3 lands (its format id, the idle-host and release-preset provenance rules, and the statement in `docs/bench.md` of the procedure, which exists); the next plans continue from T-08-45 or the numbering their plans assign.

## Final gate

```
sh tools/gate.sh:      Task 1 tree and Task 2 tree: gate: PASS steps=38/38 controls=380/380 ... faults=235/235 ... fault-runs=235 ... tests-expected=4956 tests-executed=4952 tests-skipped=4 tests-missing=0 ... claims=28 provisional=27 ... goldens=29 (Task 2 tree: gates=90 lines=306; Task 1 tree: gates=89 lines=304), 0 FAIL lines
dash tools/gate.sh:    the identical summary line on each tree (0 FAIL lines)
GATE_LINUX_PLATFORM= sh tools/gate-linux.sh: Task 1 tree (the four Task 1 files, uncommitted when it ran): exit 0, gate: PASS, the Task 1 counts (gates=89 lines=304), 0 FAIL lines, gate-time total=410; Task 2 tree (tracked=2483, the committed code, data, registry and docs tree of the plan): exit 0, gate: PASS, steps=38/38 controls=380/380 faults=235/235 fault-runs=235 tests-expected=4956 tests-executed=4952 tests-skipped=4 tests-missing=0 goldens=29 claims=28 provisional=27 gates=90 lines=306, 0 FAIL lines, gate-time total=355
```

The closing tree adds only this file to the Task 2 tree (STATE.md, ROADMAP.md and REQUIREMENTS.md are not edited in the partial state), which the sh and dash gates cover (the steps `hygiene`, `integrity`, `files`, `text` and `glossary` read it); the closing-tree results are in the next paragraph.

On the closing tree (this SUMMARY added, `tracked=2484`): sh (`gate-time total=84`, 84.5 s wall, user 153.4 sys 131.8) and dash (`gate-time total=85`, 84.3 s wall, user 152.8 sys 132.0) `gate: PASS` with the identical summary line (md5 13b4578ade8082682948dd94e8c7eba6), 0 FAIL lines, `steps=38/38 controls=380/380 faults=235/235 fault-runs=235 tests-expected=4956 tests-executed=4952 tests-skipped=4 tests-missing=0 goldens=29 claims=28 provisional=27 gates=90 lines=306 pkg-selftest=19/19 state-rows=88 state-cuts=18`. The container gate was not repeated on the closing tree (only this SUMMARY differs from the Task 2 tree that passed in the container); this is stated, not hidden. The plan-end counts of the plan (`tests-expected=4958 tests-executed=4954`) belong to Task 3 and have NOT been reached.

## Known Stubs

None. (The paragraph "The committed baseline" of `docs/bench.md` says in words that no record is committed yet; it is true until Task 3 and it is not a stub of a function.)

## Threat Flags

None new. T-08-44 (a registry line that names a mechanism that cannot fail): both lines resolve to test ids of the suite `bench` through the registry check; the two mechanisms were shown able to fail in the plan before (08-18: two broken stand-ins of the tool) and the step was shown to fail on a registry without the lines and with a renamed id; every toy registry keeps covering every accepted requirement; no CHECK, EXPECT or FORBID file changed. T-08-43 (a mislabelled baseline): pending Task 3. T-08-SC: no installs.

## Self-Check: PASSED (for Tasks 1 and 2 only; the plan is partial)

Verified before the final commit: the SUMMARY and the created file `docs/bench.md` exist and are tracked (the SUMMARY is added by this commit); commits `2cdcc60` and `8f61ecd` are ancestors of HEAD and `git rev-list --count 8453283a2b439cc20900b7d6446571ff272d8890..HEAD` printed 2 before this file's commit; nothing is staged but this file, and `.planning/config.json`, `.planning/state.json` and `.planning/ci-runs.tsv` are not staged; `git ls-files build` lists nothing and `build/bench/baseline-input.tsv` does not exist; `git ls-files` lists no `.ivsc`, `.ivsn`, `.bin`, `.rom`, `.exec`, `.wav`, `.ppm` or `.vraw` file; the diff of the two code commits outside `.planning/` holds no home, scratch or temp path, no at sign before a dotted word and no "Intellivision"; no commit deletes a file and none touches `core/`, `runner/`, `fixtures/`, `data/golden/` or `data/scenarios/` or `data/claims.json`; `grep -c '^prf01-digest-first ' data/expected/important-gates.ids` prints 2 and `grep -c 'toy-prf01' ` over the ten toy registries finds one line in each; `grep -c 'ns_per_guest_cycle\|lat_first_ms\|ci_time' docs/bench.md` prints 5; the text step accepts the document (`text pass docs=60`).
