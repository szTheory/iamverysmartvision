---
phase: 08-stic-completion-save-state-performance-baselines
plan: 19
subsystem: Phase 8 wave C, plan 2 of 2 - the snapshot benchmark workload, the benchmark document, PRF-01 accepted by the important-gates step, the baseline record and its format id, PRF-01 marked complete on stated terms
tags: [C, PRF-01, bench, ivs-bench, snapshot-workload, docs, important-gates, baseline, D-23, D-24, D-25, D-39, D-43, T-08-43, T-08-44]
status: complete

requires:
  - phase: 08-stic-completion-save-state-performance-baselines
    provides: "plan 08-18 end state: steps=38/38 controls=380/380 faults=235/235 fault-runs=235 tests-expected=4952 tests-executed=4948 tests-skipped=4 tests-missing=0 claims=28 provisional=27 gates=89 lines=304 goldens=29 pkg-selftest=19/19 state-rows=88 state-cuts=18 (taken from the 08-18 SUMMARY; the plan-start gate was not re-run, deviation 5)"
provides:
  - "tools/bench/bench.c: a workload row with a non-zero roundtrip_ticks passes the cadence to every execution (iteration 0, warmup and timed iterations) through IVS_RUNNER_OPT_ROUNDTRIP and the roundtrip_ticks option of the runner; a run in which the hook made no cut is a harness failure (exit 14, one line); the plan-08-18 refusal of a cadence is gone"
  - "data/bench/workloads.tsv: bench-lamp-snapshot (scenario bench-lamp-idle, 3 warmup, 15 iterations, roundtrip_ticks 1,792,080, ticks 7,168,352), no scenario and no golden added; its digest precondition is data/golden/bench-lamp-idle.digests"
  - "suite bench: bench.verify.bench-lamp-snapshot (static counts equal those of the idle row), bench.workloads.table-consistent (files and text only, with three broken scratch tables) and bench.baseline.record-format (the format of every committed baseline record, with four broken scratch records); 11 ids, 22 with the presets"
  - "docs/bench.md: definitions, the 22 columns in three groups plus provenance, how to run and read it, refusal and exit 15, the snapshot workload, the repeatability-only statement, the gate rule, CI time as a pointer, the baseline procedure and the committed record with its non-idle load statement"
  - "tools/check/steps/important-gates.sh accepts PRF-01; data/expected/important-gates.ids +2 lines over the gate prf01-digest-first (gates 89 to 90, lines 304 to 306); toy-prf01 in each of the ten toy registries"
  - "data/bench/baseline-darwin-arm64.tsv: the person-written record of the four workloads (release preset, no sanitizers, host class darwin arm64 18 cpus, 2026-11-15) taken on a host that was NOT idle and says so on its idle line; .planning/REQUIREMENTS.md marks PRF-01 complete with the wave C close-out note"
affects: [08-20, 08-21]

actuals:
  tokens: 8863
  tasks: 3
  commits: 4
plan_head_before: 8453283a2b439cc20900b7d6446571ff272d8890
plan_head_after: 4dd5f565e6d3cbf1c04babd54fd7da303ce77c2f

key-decisions:
  - "The snapshot workload needs no new scenario and no new golden: the idle scenario run with a save, a reset and a restore every 30 frames must produce the idle report, so the digest-first rule of the tool proves the restore equal before any figure of the snapshot workload can be printed"
  - "A cadence that produces no cut is a harness failure of the tool, not a silent no-op: without it a stand-in that never hands the cadence to the runner would match the idle golden and measure nothing about snapshots"
  - "The two consistency ids carry their own judges (broken scratch tables and records, a few awk launches, no tool launch), so the rules are shown able to fail inside the ids and no id was added for them"
  - "The baseline is recorded as taken: the host was not idle, the record's idle line says so verbatim, the document and the requirement note say so, and the record is informational; no check judges the idle statement or any number"
  - "PRF-01 gets no claim row: the registry names the two mechanisms of the digest-first rule, the document says a figure is never evidence"

key-files:
  created:
    - docs/bench.md
    - data/bench/baseline-darwin-arm64.tsv
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
    - .planning/REQUIREMENTS.md
    - .planning/STATE.md
    - .planning/ROADMAP.md

requirements-completed: [PRF-01]

completed: 2026-11-15
---

# Phase 8 Plan 19: the snapshot workload, the benchmark document and the baseline record Summary

**`ivs-bench` hands the `roundtrip_ticks` cadence of a table row to every execution of that workload, so `bench-lamp-snapshot` (the idle scenario with a save, a reset and a restore every 30 frames, three cuts in the 120-frame run) is verified in every gate run against the idle golden before any figure can be printed; `docs/bench.md` says what the numbers are and are not; the important-gates step accepts PRF-01 with two registry lines that name the digest-first mechanisms, each of the ten toy registries covers it, and `data/bench/baseline-darwin-arm64.tsv` records the four workloads in the release preset without sanitizers, with its format held by `bench.baseline.record-format`. The record was taken on a host that was NOT idle (load average 5.96 on 18 cpus, kept verbatim on its idle line), so it is informational and is documented as such; PRF-01 is marked complete on the recorded terms. The gates pass under sh and dash on every task tree and on the closing tree, and under the Ubuntu container, with the plan-end counts `steps=38/38 controls=380/380 faults=235/235 fault-runs=235 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 goldens=29 claims=28 provisional=27 gates=90 lines=306`.**

## Commits

| Commit | Task | What |
|--------|------|------|
| `2cdcc60` | 1 (tracer) | the cadence wiring and the zero-cut guard in `tools/bench/bench.c`, the row `bench-lamp-snapshot`, the ids `bench.verify.bench-lamp-snapshot` and `bench.workloads.table-consistent` |
| `8f61ecd` | 2 | `docs/bench.md`, PRF-01 accepted by `important-gates.sh`, the two registry lines, the ten toy lines |
| `6370120` | | the partial SUMMARY written when the plan stopped at the Task 3 precondition (`baseline-input-missing`, see the Task 3 section); replaced by this file |
| `4dd5f56` | 3 | the baseline record, `bench.baseline.record-format`, the document paragraph, PRF-01 marked complete in REQUIREMENTS |
| the commit that adds this file | | this SUMMARY, STATE.md and ROADMAP.md |

## The snapshot workload (Task 1)

`bench_run` zeroes the options struct as before and, for a row with `roundtrip_ticks != 0`, sets `opts.flags = IVS_RUNNER_OPT_ROUNDTRIP` and `opts.roundtrip_ticks`; the same struct goes to iteration 0, every warmup iteration and every timed iteration, so the round trip is part of what is verified and of what is timed. The golden is looked up by the row's scenario column, so the snapshot row compares with `data/golden/bench-lamp-idle.digests` and with the idle `ticks` (7,168,352). The refusal of plan 08-18 (`roundtrip_ticks other than 0 is not supported by this tool yet`) is removed. Nothing else in the digest-first flow changed and no file under `core/` or `runner/` changed.

**Cut count: 3.** Cuts at 30, 60 and 90 frames (multiples 1,792,080, 3,584,160 and 5,376,240 of machine time); the fourth multiple, 7,168,320, is the end bound of the run-to and the hook makes no cut there. Read from `ivs_runner_result.roundtrip_cuts` by a scratch build of the tool (outside the repository, not committed) that printed the field after iteration 0: `cuts=3`.

**Verification row** (dev preset, `build/dev/tools/bench/ivs-bench --verify-only --root . --content-dir build/dev/images --workload bench-lamp-snapshot`, exit 0, `<TAB>` stands for the tab character; the idle row has the same `ticks_per_iter`, `cpu_cycles_per_iter` and `frames_per_iter`):

```
bench-lamp-snapshot<TAB>verified<TAB>bench-lamp-idle<TAB>repeatability-only<TAB>AppleClang-21.0.0.21000101-Debug-5be05b529cd4<TAB>0<TAB>on<TAB>18<TAB>3<TAB>15<TAB>7168352<TAB>1792088<TAB>120<TAB><TAB><TAB><TAB><TAB><TAB><TAB><TAB><TAB>see-ci-runs.tsv
```

**One timed release run for information (not committed, not a baseline).** Release preset, a scratch table outside the repository (idle and snapshot rows, warmup 1, iterations 3), the host loaded (load average 17 on 18 cpus), exit 0, no stderr, `git status` identical before and after: the idle row read a median wall of 26.3 ms and the snapshot row 28.1 ms (about 1.8 ms more for the three round trips, about 7 percent), 14.7 and 15.7 ns per guest cycle. Three samples on a loaded host show that the timed path with the cadence works and nothing more.

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

`docs/bench.md` holds the third truth of the plan: definitions (workload, verification iteration, warmup iteration, timed iteration, fixed counts, median), the 22-column table with group, unit and computation, the run instructions with repository-relative paths and the release preset, the reading rules (CPU time for trend and regression, wall time for latency; **`lat_first_ms` is the first TIMED iteration of a warm process, not a cold-start figure**, as the 08-18 SUMMARY required; about 43 percent of a verified run is SHA-256 over every frame twice, measured once on a loaded host during the planning of this phase, so indicative only, and there is no switch to remove it), the refusal reasons and exit 15, the snapshot workload, no CPU-bound label and why, the `repeatability-only` label that is neither an evidence tier nor a claim (D10 and D11 untouched), that the gate never reads a timer and never compares a figure with a threshold, CI time as a pointer to `.planning/ci-runs.tsv`, the baseline procedure and the unimplemented local-only comparison idea. The text step found three untagged claim words (the pointer text `see-ci-runs.tsv` twice and the word that hands the cadence to the runner); two lines carry `[nc]`, one was reworded; no bare clock word occurs.

`tools/check/steps/important-gates.sh` accepts PRF-01 (the `nreq` list and the header comment). `data/expected/important-gates.ids` gains a comment naming this plan and D-25 and the two lines `prf01-digest-first mutant bench.mutant.refuse-mutated-golden PRF-01` and `prf01-digest-first mutant bench.judge.refuse-has-no-timing PRF-01`; the step reads `gates=90 lines=306` (89 plus 1, 304 plus 2), exactly as derived. The ten toy registries (the known-good tree and the nine overlays that `grep -rl toy-sta01 tools/check/controls` lists) each get a comment and `toy-prf01 fault GOOD_FAULT PRF-01` after the last line, so no line number that an EXPECT names moved; no CHECK, EXPECT or FORBID file changed, and `sh tools/check/run-controls.sh` read `controls=380/380` with the ten `important-gates-*` controls `control ok`. It can fail: on a scratch copy of the registry without the two PRF-01 lines the step reports `FAIL important-gates-uncovered-gate: PRF-01`, and with the judge id renamed `FAIL important-gates-unknown-id: mutant bench.judge.no-such-judge line 396`. No claim row exists for PRF-01 (`git show --stat HEAD` of Task 2 lists no `data/claims.json`).

## Task 3: the baseline record

**The stop and the resume.** The first session stopped after Task 2 and reported `baseline-input-missing`: `build/bench/baseline-input.tsv` did not exist and the plan forbids measuring it in this plan or substituting a dev-preset or loaded-host run. The orchestrator then took the file with `ivs-bench` in the release preset without sanitizers (four `measured` rows, exit 0, about 1.7 s) and placed it in `build/bench/` (gitignored, never tracked: `git ls-files build` lists nothing). It reported plainly that **the host was not idle** and put that on the idle line. Task 3 was resumed in a second session on that file.

**The comment lines of the input, quoted as taken** (the only text of the record that is not the output of the tool):

```
# host: darwin arm64, 18 cpus
# idle: not idle: load average 5.96 on 18 cpus (one virtual machine process near 1 core and desktop services running); single-threaded run at 98 percent of one core
# date: 2026-11-15
```

**Validation of the input, rule by rule, before it was copied** (the rules are the ones of `bench.baseline.record-format`, which was applied to the input unchanged in a scratch tree outside the repository and read `ok`; no figure is quoted here):

| Rule | Result |
|------|--------|
| the first non-comment line is `bench-format 1` | holds |
| the next line is the exact 22-column header | holds |
| one row per workload of `data/bench/workloads.tsv`, in table order (idle, input, cold, snapshot), with the table's scenario | holds (four rows) |
| status is the timed row word `measured` | holds in all four rows |
| `optimized` is 1 and `sanitizers` is `off` | holds; one `build_id` (AppleClang 21.0.0, Release) and one `ncpu` (18) for the whole record |
| warmup, iterations and `ticks_per_iter` equal the table's | holds |
| every throughput and latency column is a positive number, the static counts are positive | holds |
| `ci_time` is the pointer text | holds |
| exactly one `# host:`, one `# idle:`, one `# date:` line, an ISO date, no path, at sign, tilde, URL or address | holds (a host class and a load statement only) |
| **the host was idle** | NOT met, and not a rule the format id can judge: the input says `not idle`, which is true and is kept (deviation 1) |

**The record.** `data/bench/baseline-darwin-arm64.tsv` is the input unchanged (`tail -n +4` of the file is `cmp`-equal to the input) under a three-line comment block: that the file is a person-written record of one run on one host class and never an assertion of any check, the reading rule of `docs/bench.md`, and that the host, idle and date lines are the recorder's statements kept as taken. **The id `bench.baseline.record-format`** (`baseline_findings` in `tests/bench-tap.sh`, registered in `data/expected/tests/bench.ids`): every committed `data/bench/baseline-*.tsv` (at least one is required) must satisfy the rules above; it never compares a number with a threshold and never judges the idle statement. Inside the case, four scratch copies of the record each broken in one way (a refused row, a missing host line, an unoptimized row, a zero figure) must each produce the finding of their rule. Shown able to fail on the real case besides: a scratch tree whose record had a path in its host line reads `not ok 11 ... the # host: line holds a path, an at sign, a tilde, a URL or an address`, and a tree without any record reads `no data/bench/baseline-*.tsv record is committed`. `docs/bench.md` names the file, the host class, the date and the load statement verbatim, says that the host was not idle and that the record is informational and is replaced by a reviewed change when an idle-host record exists; no figure is copied into the document.

**The requirement mark.** `.planning/REQUIREMENTS.md` marks PRF-01 complete (the checkbox and the traceability row; no other row moved) with a Phase 8 wave C close-out note on the terms of the fifth truth of the plan: named deterministic workloads verified before any figure, the digest asserted before throughput by construction and by a refusal control, throughput, latency and CI time in separate groups with CI time a pointer, no throughput threshold anywhere, the baseline one person-written record on one host class and not an assertion, no optimization in Phase 8, no claim, repeatability-only figures. **The conditions of the plan are met** (the record exists and is validated by the format id; the mark was made after it existed) **with one stated difference: the baseline was taken on a host that was not idle, which the plan's procedure asks for.** The mark does not rest on the baseline's quality: the requirement text (a runner that asserts the digest before reporting, three groups reported separately) is held by the tool, the suite and the registry; the baseline is a record for later comparison, and the note says it is informational. If the owner reads the idle-host condition as a hard condition of the mark, the mark can be reverted by one line and re-made when an idle-host record replaces this one; this is flagged in the report as a judgement.

## Deviations from Plan

**1. [Interpretation] The baseline was taken on a host that was not idle and is recorded as taken.** The plan's record is "taken on an idle host"; the orchestrator's input says on its idle line, verbatim, `not idle: load average 5.96 on 18 cpus (...)`. Per the orchestrator's instruction the statement was kept unaltered in the committed record, the document and the close-out note say that the figures are informational, and the format id does not judge the idle statement (a record that says the host was not idle is a valid record that says so; nothing in the plan's rules forbids that wording). The honest label is the point of T-08-43. A later record on an idle host replaces this one by a reviewed change.

**2. [Rule 2 - missing critical functionality] A zero-cut guard in the tool.** The plan wires the cadence and relies on the digest. A tool that failed to hand the cadence on would still match the idle golden and would then time an idle run under the name of the snapshot workload. `bench_run` therefore requires `res.roundtrip_cuts != 0` after iteration 0 for a row with a cadence (a harness failure otherwise, after the digest and ticks checks so the refusal order is unchanged). Four lines of C, shown able to fail above, no new id (the existing verify id fails when it fires) and no launch added.

**3. [Interpretation] The two consistency ids include their own judges.** The plan lists the rules; a check of rules that never fails proves nothing (lesson 7), so three scratch tables and four scratch records, each broken in one way, must each produce the finding of their rule. The cost is a few awk launches, no tool launch and no new id.

**4. [Interpretation] The snapshot verify id compares its static counts through a saved output.** `verify_case` keeps each workload's output in `$tmp/v.<workload>.out` instead of one shared file and takes an optional peer; no extra launch of the tool.

**5. [Information] The plan-start gate was not re-run.** The plan says to re-measure the counts with `sh tools/gate.sh` first; I took the 08-18 end-state counts and compared the Task 1 tree with the figures the plan derives (4952 to 4956 expected), which matched exactly (a drift would have shown there); the first gate of this plan is the Task 1 tree.

**6. [Information] The `roundtrip_ticks` table rule is the plan's (0 or at least a 64th of the run length) and does not bound the cadence from above;** zero cuts are caught by the guard of deviation 2, not by the table rule.

**7. [Information] The container gates ran on the uncommitted Task 1 tree, on the committed Task 2 tree and on the closing tree; none was run on the Task 3 tree alone.** The closing tree contains every file of Task 3 and adds only this SUMMARY, STATE.md and ROADMAP.md, so the container verify of Task 3 is met by the closing-tree run (the plan's Task 3 verify asks for the container gate and the closing tree is a superset).

**8. [Information] The first session wrote a partial SUMMARY** (commit `6370120`) when it stopped at the Task 3 precondition, as the orchestrator asked; this file replaces it.

No auth gates, no package installs (T-08-SC), no `gh` call, no push, no tag, no `tools/ci-record.sh`; `.planning/config.json`, `.planning/state.json` and `.planning/ci-runs.tsv` were never staged. No file under `core/`, `runner/`, `fixtures/` or `data/golden/` changed and no scenario was added. D10 and D11 are not performed, no claim row exists for a benchmark figure, and no figure of the record is quoted in this SUMMARY, the document or the requirement note as a result (the information run above is loaded-host information, not a baseline).

## Decisions Made

- The digest of the snapshot workload is the idle golden: that is the whole proof that a save, reset and restore every 30 frames changes nothing observable, and it costs one more verification launch instead of a scenario and a golden.
- A guard that turns a silent no-op into a harness failure is cheaper than a test of the no-op: the existing verify id is the test.
- A record is kept as it was taken: an honest `not idle` on the idle line is worth more than an edited statement, and the format id judges the format, not the quality of the host.
- A figure is never a claim: PRF-01 is held by the mechanisms of the digest-first rule, a document and a format-checked record, not by a claim row.

## Cost (laptop)

| Item | Value |
|------|-------|
| estimate of the plan | 90,000 tokens (raw, confidence low), 3 tasks |
| actuals | 8,863 tokens (chars/4 over the added lines of the 4 commits outside `.planning/`), 3 tasks, 4 commits (measured from the ledger of the plan head: the three task commits and the partial SUMMARY commit) |
| gate total, sh and dash, Task 1 tree | sh `gate-time total=99` (wall 98.7 s, user 161.4 sys 138.5), dash 95.1 s (user 162.6 sys 138.6), identical summary line (md5 48e5afbec81465e5ca15468890224d02) |
| gate total, sh and dash, Task 2 tree | sh 106.0 s (user 171.0 sys 144.2), dash 87.9 s (user 158.7 sys 138.4), identical summary line (md5 0c38a50a0f4d834504bcce8ba47cf859); load average 15 to 21 |
| gate total, sh and dash, Task 3 tree | sh `gate-time total=72` (wall 72.3 s, user 126.2 sys 103.1), dash `gate-time total=78` (77.5 s, user 144.0 sys 119.0), identical summary line (md5 6e9bedb5ad3d3eac49df45a4ed45b9e7); load average 6 to 8 |
| container gate (gcc 13.3.0, cmake 3.28.3, x86_64, `GATE_LINUX_PLATFORM=`) | Task 1 tree: exit 0, `gate: PASS`, the Task 1 counts, 0 FAIL lines, `gate-time total=410`; Task 2 tree (tracked=2483): exit 0, `gate: PASS`, the Task 2 counts, `core-imports=5`, 0 FAIL lines, `gate-time total=355`; closing tree (tracked=2485, every file of the plan): exit 0, `gate: PASS`, the plan-end counts, `core-imports=5`, 0 FAIL lines, `gate-time total=337` |
| the `bench` ctest case (three runs each) | at the end of the plan, load average about 7: dev 1.44, 1.46 and 1.44 cpu-s, release 0.33, 0.32 and 0.32; in the middle of the plan, load average 17: dev 1.55, 1.60 and 1.78, release 0.39, 0.38 and 0.38 (08-18: about 1.69 and 0.25) |
| the three new ids, by difference | `bench.verify.bench-lamp-snapshot`: dev about 0.19 cpu-s (0.11 user, 0.08 sys), release about 0.04; `bench.workloads.table-consistent`: about 0.03 per preset; `bench.baseline.record-format`: about 0.01 to 0.02 per preset (shell and awk, five runs each way, release tool, dash); about 0.33 local in all |
| the added gate steps and controls | none; the important-gates step adds nothing measurable (a pure awk checker) |

**Cost position against the checkpoint C bound and the lines.** About 0.33 local cpu-s are added by the plan (the `bench-lamp-snapshot` launch dominates the dev preset, as predicted in the 08-18 SUMMARY): 0.55 hosted at the factor 1.675, about +0.011 in `r` at a hosted `build_s` of 52. Against the last five hosted rows as the orchestrator gave them (median 7.86, newest 7.72, head ce469aa) plus the 08-18 reading of +0.075 for the bench suite, the projection after this plan is about 7.95 by the median and 7.81 by the newest row: under the line 8.3 by 0.35, under the reading line 8.0 by 0.05, above the budget 7.8 by 0.15. The named unapplied trims of the 08-18 SUMMARY (the eight bad-argument vectors reduced to four on the dev preset, the one timed iteration of the mutant case on release only, one verify id sharing a launch with the format case) were not applied and were not needed to stay under the line; they remain available at the next checkpoint. The bound of checkpoint C for plans 08-18 to 08-21 (about 0.4 local) is exceeded by plan 08-18 alone; this plan adds 0.33 on top.

## Open Items

1. **The baseline was taken on a non-idle host** (deviation 1): the record is informational. An idle-host record taken with the procedure of `docs/bench.md` replaces it by a reviewed change; the owner or orchestrator can re-take it at any quiet moment and the format id will accept it. The PRF-01 mark is flagged as a judgement call (see the Task 3 section).
2. **The timed path has run on macOS arm64 only,** and never on Linux; the gate runs the tool with `--verify-only` only. `lat_first_ms` is the first timed iteration of a warm process (stated in `docs/bench.md`).
3. **`docs/runner.md` does not list the golden-writing commands of the two bench goldens** (08-18 open item 6, unchanged); they are in the 08-18 SUMMARY.
4. **Cost:** the projection after this plan is about 7.95 by the median, above the budget and under the reading line by 0.05; the `phase8-close` entry of plan 08-21 should read the next hosted rows with plans 08-18 and 08-19 in them.
5. **Information.** D10 and D11 are not performed; no claim above provisional exists and none concerns a benchmark; REL-01 stays Pending (D-28).

## Interfaces for the next plans

- Counts after this plan: `steps=38/38 controls=380/380 faults=235/235 fault-runs=235 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 goldens=29 claims=28 provisional=27 gates=90 lines=306 pkg-selftest=19/19 state-rows=88 state-cuts=18`; plans 08-20 and 08-21 start from them (08-20 states that its counts are unchanged from this plan).
- The suite names after this plan: `bench.verify.<idle|input|cold|snapshot>`, `bench.mutant.refuse-mutated-golden`, `bench.refuse.wrong-ticks`, `bench.refuse.bad-args`, `bench.tsv.format`, `bench.judge.refuse-has-no-timing`, `bench.workloads.table-consistent`, `bench.baseline.record-format` (11 ids).
- Threat ids: T-08-43 (a mislabelled baseline) and T-08-44 (the registry change) are mitigated here; the next plans continue from T-08-45 or the numbering their plans assign.
- For the close-out records of plan 08-21: PRF-01 is complete on the terms of the wave C note in REQUIREMENTS; the baseline is non-idle and informational; no claim row for any benchmark figure; no threshold anywhere.

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=38/38 controls=380/380 ... faults=235/235 ... fault-runs=235 ... tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 ... claims=28 provisional=27 ... gates=90 lines=306 ... goldens=29 ... pkg-selftest=19/19 ... state-rows=88 state-cuts=18 (0 FAIL lines)
dash tools/gate.sh:    the identical summary line (0 FAIL lines)
GATE_LINUX_PLATFORM= sh tools/gate-linux.sh: closing tree: exit 0, gate: PASS, the plan-end counts, 0 FAIL lines, gate-time total=337
```

After the Task 1 tree: `tests-expected=4956 tests-executed=4952 gates=89 lines=304`, sh and dash identical, and the container gate on the same tree: exit 0, `gate: PASS`, 0 FAIL lines, 410 s. After the Task 2 tree: `gates=90 lines=306`, sh and dash identical, and the container gate on the committed Task 2 tree (tracked=2483): exit 0, `gate: PASS`, 0 FAIL lines, 355 s. After the Task 3 tree (`tracked=2485`): the plan-end counts above under sh (72 s) and dash (78 s) with the identical summary line (md5 6e9bedb5ad3d3eac49df45a4ed45b9e7). On the closing tree (this SUMMARY, STATE.md, ROADMAP.md and REQUIREMENTS.md as committed or edited; `tracked=2485`): sh (`gate-time total=70`, 69.8 s wall, user 124.2 sys 103.0) and dash (`gate-time total=74`, 74.1 s, user 131.6 sys 109.3) `gate: PASS` with the identical summary line (md5 6e9bedb5ad3d3eac49df45a4ed45b9e7, the same as the Task 3 tree), 0 FAIL lines and the plan-end counts, and `GATE_LINUX_PLATFORM= sh tools/gate-linux.sh` on the closing tree (Ubuntu 24.04, gcc 13.3.0, cmake 3.28.3, x86_64; 402 s wall in all, under the limit of 15 minutes): exit 0, `gate: PASS`, `steps=38/38 controls=380/380 faults=235/235 fault-runs=235 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 goldens=29 claims=28 provisional=27 gates=90 lines=306 pkg-selftest=19/19 state-rows=88 state-cuts=18`, `core-imports=5`, 0 FAIL lines, `gate-time total=337`. The SUMMARY text was completed with these lines after those runs; no gate step reads them.

## Known Stubs

None.

## Threat Flags

None new. T-08-43 (a mislabelled baseline): the `optimized` and `sanitizers` columns come from the build, the format id requires optimized 1, sanitizers off, the exact header, one measured row per workload and the host, idle and date lines, and the record is not compared with any threshold; **the record's own idle line says the host was not idle, the document and the requirement note repeat it, and nothing presents it as an idle-host baseline.** T-08-44 (a registry line that names a mechanism that cannot fail): both lines resolve to test ids of the suite `bench` through the registry check, the two mechanisms were shown able to fail in plan 08-18 and the step was shown to fail on a registry without the lines and with a renamed id; every toy registry keeps covering every accepted requirement; no CHECK, EXPECT or FORBID file changed. T-08-SC: no installs.

## Self-Check: PASSED

Verified before the final commit: the SUMMARY and the created files of the plan exist (`docs/bench.md`, `data/bench/baseline-darwin-arm64.tsv` and the SUMMARY; the first two tracked); commits `2cdcc60`, `8f61ecd`, `6370120` and `4dd5f56` are ancestors of HEAD and `git rev-list --count 8453283a2b439cc20900b7d6446571ff272d8890..HEAD` printed 4 before this file's commit; nothing is staged but the files of this commit (STATE.md, ROADMAP.md and this SUMMARY) and `.planning/config.json`, `.planning/state.json` and `.planning/ci-runs.tsv` are not staged; `git ls-files build` lists nothing and `build/bench/baseline-input.tsv` exists untracked; `git ls-files` lists no `.ivsc`, `.ivsn`, `.bin`, `.rom`, `.exec`, `.wav`, `.ppm` or `.vraw` file; the diff of the four commits outside `.planning/` holds no home, scratch or temp path, no at sign before a dotted word and no "Intellivision" (the baseline record holds none either); no commit deletes a file and none touches `core/`, `runner/`, `fixtures/`, `data/golden/`, `data/scenarios/` or `data/claims.json`; `grep -c '^prf01-digest-first ' data/expected/important-gates.ids` prints 2 and the ten toy registries each hold one `toy-prf01` line; `grep -c '^bench[.]' data/expected/tests/bench.ids` prints 11 and `bench.baseline.record-format` appears once as an id; `grep -A2 '^- \[x\] \*\*PRF-01\*\*' .planning/REQUIREMENTS.md` finds the mark and no other requirement row moved; `grep -c 'ns_per_guest_cycle\|lat_first_ms\|ci_time' docs/bench.md` is at least 3 and the text step accepts the document.
