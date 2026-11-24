---
phase: 08-stic-completion-save-state-performance-baselines
plan: 18
subsystem: Phase 8 wave C, plan 1 of 2 - ivs-bench, the digest-first benchmark tool, its idle, input and cold workloads, the verify suite and the negative control of the refusal
tags: [C, PRF-01, bench, ivs-bench, digest-first, D-21, D-22, D-23, D-24, D-25, D-26, D-39, D-43, T-08-41, T-08-42]
status: complete

requires:
  - phase: 08-stic-completion-save-state-performance-baselines
    provides: "plan 08-17 end state: steps=38/38 controls=379/379 faults=235/235 fault-runs=235 tests-expected=4932 tests-executed=4928 tests-skipped=4 tests-missing=0 claims=28 provisional=27 gates=89 lines=304 goldens=27 pkg-selftest=19/19 state-rows=88 state-cuts=18 (re-measured with sh tools/gate.sh before the first edit)"
provides:
  - "tools/bench (ivs-bench, target ivs_bench): bench.c in ISO C (no POSIX include, no clock name), bench_host.c the only file with the monotonic clock and the process CPU time, bench_host.h; never installed, built in both presets; verifies iteration 0 against the committed golden through ivs_runner_golden_keep before any clock is read, refuses with `bench-refuse` and exit 15, prints TSV `bench-format 1` in three groups plus the ci_time pointer; --verify-only reads no clock"
  - "data/bench/workloads.tsv (bench-lamp-idle, bench-lamp-input, bench-lamp-cold), data/scenarios/bench-lamp-idle.ivss and bench-lamp-input.ivss with the goldens data/golden/bench-lamp-idle.digests and bench-lamp-input.digests (written by ivs-run --update-golden), lines in data/expected/runner-scenarios.ids and ids runner-process.scenario.bench-lamp-idle and bench-lamp-input"
  - "suite bench (tests/bench-tap.sh, one CTest case, SKIP_RETURN_CODE 77, data/expected/tests/bench.ids with 8 ids: 3 verify, mutant, wrong-ticks, bad-args, tsv.format, judge); control c-build-unlisted-bench-source and the registrations a new C tool needs (root add_subdirectory, c-build module line and the toy tree list, SPDX pattern)"
affects: [08-19, 08-20, 08-21]

actuals:
  tokens: 19957
  tasks: 3
  commits: 3
plan_head_before: ce469aa8c893bcaf10d6df18b588fa9a6ce5b96a
plan_head_after: ef2032d619b0cb2e645edbfb9c371665b130bccf

key-decisions:
  - "The digest comes before any clock, structurally: iteration 0 is an untimed run whose report is filtered with the public ivs_runner_golden_keep and compared with the committed golden, and the timed code is unreachable on the refusal path; --verify-only is the only mode the gate runs"
  - "A refused row holds no number: every column from ncpu to lat_max_ms is empty (an empty field cannot be mistaken for a measurement), the provenance columns stay; a verified row fills the static group and leaves throughput and latency empty"
  - "The refusal and format cases use the 13-frame cold workload, not the 120-frame idle workload: the refusal follows iteration 0 on any workload, so the cheapest proves the same path (deviation 1)"
  - "roundtrip_ticks other than 0 in the workload table is a harness failure of this tool until plan 08-19 supports it, never silently ignored"

key-files:
  created:
    - tools/bench/bench.c
    - tools/bench/bench_host.c
    - tools/bench/bench_host.h
    - tools/bench/CMakeLists.txt
    - tests/bench-tap.sh
    - data/expected/tests/bench.ids
    - data/bench/workloads.tsv
    - data/scenarios/bench-lamp-idle.ivss
    - data/scenarios/bench-lamp-input.ivss
    - data/golden/bench-lamp-idle.digests
    - data/golden/bench-lamp-input.digests
    - tools/check/controls/c-build-unlisted-bench-source/CHECK
    - tools/check/controls/c-build-unlisted-bench-source/EXPECT
    - tools/check/controls/c-build-unlisted-bench-source/overlay/tools/bench/extra.c
    - .planning/phases/08-stic-completion-save-state-performance-baselines/08-18-SUMMARY.md
  modified:
    - CMakeLists.txt
    - tools/check/steps/c-build.sh
    - tools/check/integrity.cmake
    - tools/check/controls/_good/tree/CMakeLists.txt
    - data/expected/controls/runner-hygiene.ids
    - data/expected/runner-scenarios.ids
    - data/expected/tests/runner-process.ids
    - tests/CMakeLists.txt
    - .planning/STATE.md
    - .planning/ROADMAP.md

requirements-completed: []

completed: 2026-11-15
---

# Phase 8 Plan 18: the benchmark tool Summary

**`ivs-bench` cannot print a throughput figure for a run whose report differs from the committed golden: iteration 0 of every workload is an untimed run, its report is filtered with the public `ivs_runner_golden_keep` and compared with `data/golden/<scenario>.digests` and with the `ticks` column of the workload table, and only an exact match reaches a warmup or a clock; a mismatch prints `bench-refuse <workload> <reason> <detail>` on stderr, a row of status `refused` with every numeric column empty, and exits 15. The tool is two C17 translation units (`bench.c` in ISO C with no POSIX include, `bench_host.c` the only file of the repository that reads the monotonic clock and the process CPU time for a measurement), links `ivs_runner_lib` and `ivs_core`, calls `ivs_runner_execute` in process, is never installed and is registered everywhere a new C tool must be (root `add_subdirectory`, the `c-build` module line, the SPDX pattern, the toy tree and the control `c-build-unlisted-bench-source`). Three workloads are verified by the gate in both presets: `bench-lamp-idle` (Lamp Tone, 120 frames, final tick 7,168,352), `bench-lamp-input` (the same run with the 64 events of the scenario cap: the six events of `demo-lamp-tone-pass` verbatim, then 58 self-recorded events, 32 of them moved by the settling rule) and `bench-lamp-cold` (the existing scenario `demo-lamp-tone-pass` and its golden, named by a row of the workload table). The suite `bench` holds 8 ids; its refusal cases were shown able to fail against two broken stand-ins of the tool and its absence-of-measurement checker against synthetic rows. No file under `core/` or `runner/` changed, no clock runs in the gate, no claim and no threshold exists. The gates pass under sh and dash on every task tree, and under the Ubuntu container (gcc 13.3.0) on the Task 1 tree and on the Task 3 tree, with the plan-end counts `steps=38/38 controls=380/380 faults=235/235 fault-runs=235 tests-expected=4952 tests-executed=4948 tests-skipped=4 tests-missing=0 goldens=29 claims=28 provisional=27`.**

## Commits

| Commit | Task | What |
|--------|------|------|
| `93a4b49` | 1 (tracer) | `tools/bench` (the tool), the scenario `bench-lamp-idle` and its golden, `data/bench/workloads.tsv`, the suite `bench` with `bench.verify.bench-lamp-idle`, `runner-process.scenario.bench-lamp-idle`, the registrations and the control `c-build-unlisted-bench-source` (controls 379 to 380) |
| `d9645ef` | 2 | the scenario `bench-lamp-input` and its golden, the rows `bench-lamp-input` and `bench-lamp-cold`, the ids `runner-process.scenario.bench-lamp-input`, `bench.verify.bench-lamp-input` and `bench.verify.bench-lamp-cold` |
| `ef2032d` | 3 | the cases `bench.mutant.refuse-mutated-golden`, `bench.refuse.wrong-ticks`, `bench.refuse.bad-args`, `bench.tsv.format` and `bench.judge.refuse-has-no-timing` |
| the commit that adds this file | | this SUMMARY, STATE.md and ROADMAP.md |

## The tool (Task 1)

Flow per workload, in this order: parse the scenario and read the golden body (the lines that do not start with `#`) during a first phase that can fail before a byte is printed; then, per workload, iteration 0 (execute with the report captured in memory, filter it line by line with `ivs_runner_golden_keep`, the live-domain flags derived from `output.video.bytes`, `output.audio.bytes`, `input.events` and `output.psglog.bytes` exactly as `runner/main.c` derives them, compare it with the golden body, and check that the last `step.<k>.now_ticks` equals the `ticks` column); with `--verify-only` print a `verified` row and go on; otherwise `warmup` untimed iterations (each still re-checked) and `iterations` timed ones, each bracketed by two clock reads around `ivs_runner_execute` alone, with the outcome, the `digest.composite` line and every `step.<k>.now_ticks` line compared with iteration 0 after the span. Counts are fixed per workload in the table, never a time budget. Statistics: the median of the per-iteration wall times (the mean of the two middle values for an even count), their maximum, the first timed iteration, the CPU time of the whole timed loop divided by the iteration count. The timebase (`ticks_per_cpu_cycle`, the crystal fraction) is read through the public `ivs_machine_info` from a throwaway instance, not written into the tool. Exit codes: 0, 14 (usage error or harness failure; for a usage error or an unreadable input one stderr line and no byte on stdout), 15 (a refusal).

Output: `bench-format 1`, one header line, one row per workload, 22 tab-separated columns in groups that never merge: provenance and static (`workload status scenario tier build_id optimized sanitizers ncpu warmup iterations ticks_per_iter cpu_cycles_per_iter frames_per_iter`), throughput (`thr_cycles_per_s_wall thr_cycles_per_s_cpu thr_frames_per_s realtime_x ns_per_guest_cycle`), latency (`lat_first_ms lat_median_ms lat_max_ms`) and the pointer `ci_time` with the text `see-ci-runs.tsv`. The tier column says `repeatability-only`. The provenance comes from `tools/bench/CMakeLists.txt` (compiler id and version, build type and a digest of the flags of that build type; `optimized` 1 only for Release, RelWithDebInfo and MinSizeRel; `sanitizers` from `IVS_SAN`); a build without these definitions does not compile (`#error`).

Verify-only output of the idle workload in the dev preset (`build/dev/tools/bench/ivs-bench --verify-only --root . --content-dir build/dev/images --workload bench-lamp-idle`, exit 0; `<TAB>` stands for the tab character):

```
bench-format 1
workload<TAB>status<TAB>scenario<TAB>tier<TAB>build_id<TAB>optimized<TAB>sanitizers<TAB>ncpu<TAB>warmup<TAB>iterations<TAB>ticks_per_iter<TAB>cpu_cycles_per_iter<TAB>frames_per_iter<TAB>thr_cycles_per_s_wall<TAB>thr_cycles_per_s_cpu<TAB>thr_frames_per_s<TAB>realtime_x<TAB>ns_per_guest_cycle<TAB>lat_first_ms<TAB>lat_median_ms<TAB>lat_max_ms<TAB>ci_time
bench-lamp-idle<TAB>verified<TAB>bench-lamp-idle<TAB>repeatability-only<TAB>AppleClang-21.0.0.21000101-Debug-5be05b529cd4<TAB>0<TAB>on<TAB>18<TAB>3<TAB>15<TAB>7168352<TAB>1792088<TAB>120<TAB><TAB><TAB><TAB><TAB><TAB><TAB><TAB><TAB>see-ci-runs.tsv
```

The confinement of the clock is held by commands: `grep -c 'clock_gettime\|getrusage\|#include <time.h>\|#include <sys/' tools/bench/bench.c` prints 0, `bench_host.c` holds `clock_gettime`, and `git diff --name-only ce469aa..HEAD` lists no file under `core/` or `runner/`. `core-symbols` and `runner-scan` are untouched.

## The timed run (information only, not committed, not a baseline)

One timed run in the release preset with a scratch workload table outside the repository (`warmup 1`, `iterations 3`, the idle scenario; the host was loaded, load average 15 on 18 cpus): exit 0, no stderr, and `git status` before and after identical (no file under the repository was written).

```
bench-lamp-idle<TAB>measured<TAB>bench-lamp-idle<TAB>repeatability-only<TAB>AppleClang-21.0.0.21000101-Release-866bf148412a<TAB>1<TAB>off<TAB>18<TAB>1<TAB>3<TAB>7168352<TAB>1792088<TAB>120<TAB>60224082<TAB>60366764<TAB>4032.66<TAB>67.298<TAB>16.60<TAB>29.540<TAB>29.757<TAB>30.294<TAB>see-ci-runs.tsv
```

Read plainly: about 29.8 ms median for the 120-frame run (60.2 million guest cycles per wall second, 67 times real time, 16.6 ns per guest cycle), on a loaded laptop with three samples: this shows the timed path works and nothing more; the baseline of plan 08-19 is the person-written record on an idle host.

## The workloads (Tasks 1 and 2)

`bench-lamp-idle` and `bench-lamp-input` are ordinary scenarios (no assert, no capture, no `.expect`; the precedent is `timeout-budget`): `ivss 1`, the profile, the three content lines of `demo-lamp-tone-pass` (boot stub 2, the zero graphics ROM, the Lamp Tone cartridge) and `run-to 120 frames`, registered as `bench-lamp-idle 0 pass deterministic run -` and `bench-lamp-input 0 pass deterministic run -`, so the existing `runner-process` and `golden` steps verify them in both presets and compare dev with release. Their goldens were written with

```
build/dev/runner/ivs-run --content-dir build/dev/images --update-golden data/golden/bench-lamp-idle.digests data/scenarios/bench-lamp-idle.ivss
build/dev/runner/ivs-run --content-dir build/dev/images --update-golden data/golden/bench-lamp-input.digests data/scenarios/bench-lamp-input.ivss
```

The idle golden: `output.video.frames 120`, `video.chain.check pass`, observed domains cpu, mem, images, instance, video, audio and psglog (`input=none`), the final machine time `step.1.now_ticks 7168352` (the 120-frame bound 7,168,320 plus the 32-tick overshoot of the last unit). The input golden: the same plus the input domain observed (`input.events 64`, `input.consumed 64`, `digest.input` present), the same final tick. `docs/runner.md` is not edited: its list of the commands that wrote the tracked goldens was already incomplete (it names none of the six STIC2 goldens), so the two commands are recorded here.

`data/bench/workloads.tsv` (tab separated, header comment naming the columns): `bench-lamp-idle` 3 warmup, 15 iterations, `ticks` 7168352; `bench-lamp-input` 3 and 15, 7168352; `bench-lamp-cold` (scenario `demo-lamp-tone-pass`, no new scenario and no new golden) 2 and 30, `ticks` 806568 (the last `step.4.now_ticks` of its golden). `roundtrip_ticks` is 0 in every row; no workload is labelled cpu-bound.

**The input workload and the settling rule.** The first six `input` lines are those of `demo-lamp-tone-pass` character for character (`cmp` of the two sets of lines succeeds), and they already satisfy the rule (remainders 40,264, 40,792, 41,320, 41,848, 42,376 and 42,904 modulo 59,736). The other 58 lie on the grid 820,000 + 110,000 k (the last at 7,090,000) with the six controller word pairs of the pass scenario in order; every event whose plain remainder fell outside 16,800 to 42,936 was moved by the smallest amount that puts it on the nearer edge of the band (forward to 16,800 or back to 42,936), which keeps the order. 32 of the 58 were moved (16 forward, 16 backward, the largest 16,440 ticks, the sum of absolute shifts 268,856 ticks); event number (1-based position among the 64): plain tick to tick used (shift):

7: 820,000 to 819,504 (-496); 10: 1,150,000 to 1,151,784 (+1784); 11: 1,260,000 to 1,271,256 (+11256); 12: 1,370,000 to 1,357,128 (-12872); 13: 1,480,000 to 1,476,600 (-3400); 17: 1,920,000 to 1,928,352 (+8352); 18: 2,030,000 to 2,014,224 (-15776); 19: 2,140,000 to 2,133,696 (-6304); 23: 2,580,000 to 2,585,448 (+5448); 24: 2,690,000 to 2,704,920 (+14920); 25: 2,800,000 to 2,790,792 (-9208); 29: 3,240,000 to 3,242,544 (+2544); 30: 3,350,000 to 3,362,016 (+12016); 31: 3,460,000 to 3,447,888 (-12112); 32: 3,570,000 to 3,567,360 (-2640); 36: 4,010,000 to 4,019,112 (+9112); 37: 4,120,000 to 4,104,984 (-15016); 38: 4,230,000 to 4,224,456 (-5544); 42: 4,670,000 to 4,676,208 (+6208); 43: 4,780,000 to 4,795,680 (+15680); 44: 4,890,000 to 4,881,552 (-8448); 48: 5,330,000 to 5,333,304 (+3304); 49: 5,440,000 to 5,452,776 (+12776); 50: 5,550,000 to 5,538,648 (-11352); 51: 5,660,000 to 5,658,120 (-1880); 54: 5,990,000 to 5,990,400 (+400); 55: 6,100,000 to 6,109,872 (+9872); 56: 6,210,000 to 6,195,744 (-14256); 57: 6,320,000 to 6,315,216 (-4784); 61: 6,760,000 to 6,766,968 (+6968); 62: 6,870,000 to 6,886,440 (+16440); 63: 6,980,000 to 6,972,312 (-7688).

The check of the plan holds: exactly 64 events, strictly ascending, every remainder in 16,800 to 42,936 (`events=64 bad=0`), the last at 7,090,000 before the bound 7,168,320. The 58 events are self-recorded repeatability only; the golden carries the tier `deterministic-repeatability` and no claim.

## The negative control (Task 3)

| Id | What it holds |
|----|---------------|
| `bench.mutant.refuse-mutated-golden` | the golden of the cold workload copied to a scratch directory with the first hex digit of its first `digest.` line changed (checked: exactly one line differs, so the mutation cannot be a no-op); a `--verify-only` run and a run of one timed iteration (scratch table, no `--verify-only`) both exit 15, print the one `bench-refuse` line naming the workload and the mutated key, and print the format line, the header and one `refused` row in which `no_measurement` finds every column from `ncpu` to `lat_max_ms` empty; no `measured` row exists |
| `bench.refuse.wrong-ticks` | a scratch table whose `ticks` is one above the golden's final tick: exit 15, `bench-refuse bench-lamp-cold ticks-mismatch table=<n+1> run=<n>`, a refused row with no number |
| `bench.refuse.bad-args` | eight vectors, each exit 14, exactly one `ivs-bench: ` line on stderr and nothing on stdout: an unknown option, an option without a value, an option given twice, a non-numeric table value, an unknown workload, a missing scenario file, a missing golden, a missing `--content-dir` |
| `bench.tsv.format` | one run on a scratch two-row table: `bench-format 1` once, the header equal to the column list in order, the groups at columns 8 to 13, 14 to 18 and 19 to 21 with `ci_time` last of 22, the field count of both rows, the tier and the pointer text, rows in table order, each judged by the rules of `bench.verify.<workload>` |
| `bench.judge.refuse-has-no-timing` | `no_measurement`, the checker the mutant and wrong-ticks cases use, is fed synthetic refused rows: a value in `thr_cycles_per_s_cpu` and one in `ns_per_guest_cycle` must each be rejected with a finding that names that column alone, a clean row must be accepted |

Quoted from the mutant run on the dev preset (golden of the cold workload, first digest line changed from `1ac14963...` to `0ac14963...`), exit 15:

```
stderr: bench-refuse bench-lamp-cold digest-mismatch digest.cpu
stdout: bench-lamp-cold<TAB>refused<TAB>demo-lamp-tone-pass<TAB>repeatability-only<TAB>AppleClang-21.0.0.21000101-Debug-5be05b529cd4<TAB>0<TAB>on<TAB><TAB><TAB><TAB><TAB><TAB><TAB><TAB><TAB><TAB><TAB><TAB><TAB><TAB><TAB>see-ci-runs.tsv
```

The cases can fail, shown on the real cases and not only on the judge: with the tool replaced by a stand-in that puts the value 123456 into `thr_cycles_per_s_cpu` of every refused row, `bench.mutant.refuse-mutated-golden` and `bench.refuse.wrong-ticks` report `not ok` (the other six `ok`, failed=2); with a stand-in that turns exit 15 into exit 0 the same two report `not ok`; with the real tool all eight are `ok`. Both stand-ins were scratch scripts outside the repository. No defect of `bench.c` was exposed by any case, so Task 3 changed no code.

Time of the `bench` ctest case (three runs each, `ctest --preset <p> -R '^bench$'`): dev preset 1.76, 1.77 and 1.79 s wall (cpu about 1.69 s: user 0.34, sys 1.36, which is process start-up under the sanitizers, about 0.09 s of sys for each of the 15 launches); release preset 0.28, 0.25 and 0.26 s wall (cpu about 0.25 s).

## Deviations from Plan

**1. [Cost] The refusal cases and the format case use the 13-frame cold workload, not the 120-frame idle one.** The Task 3 action copies the idle golden for the mutant case. The refusal is decided after iteration 0 on any workload, so the cheapest workload proves the same path; the first version of the cases (idle workload, and a fourth set of three verify runs for the format case) cost 2.4 cpu-s in the dev preset, the committed version 1.7 (measured above). `bench.tsv.format` runs once on a scratch two-row table of the cold scenario instead of re-running the three workloads. The plan's text of the ids, the exit codes and the output assertions are unchanged.

**2. [Interpretation] A nonzero `roundtrip_ticks` is a harness failure (exit 14, one line) of this tool.** The column exists and is parsed (decimal digits, 0 to the machine-time maximum); plan 08-19 gives it meaning. Silently ignoring a cadence would make a snapshot workload measure nothing about snapshots. Plan 08-19 lifts the refusal in `bench_load_workload` and sets `opts.flags` and `opts.roundtrip_ticks` in `bench_run`.

**3. [Interpretation] Which columns are empty.** The plan says a refused row has "every numeric column empty" and a verified row "every timing column empty". Implemented: a refused row keeps the seven provenance columns (`workload status scenario tier build_id optimized sanitizers`, none is a measurement) and leaves `ncpu` to `lat_max_ms` empty (14 columns), because the run behind them did not match its golden; a verified row fills the static group (`ncpu warmup iterations ticks_per_iter cpu_cycles_per_iter frames_per_iter`) and leaves the 8 throughput and latency columns empty.

**4. [Information] `lat_first_ms` is the first TIMED iteration, not a cold-process figure.** By the plan's order the process has already run iteration 0 and the warmup before the first clock read, so the first timed iteration is warm for the code and the page cache; the cold workload is a short run in a warmed process. The documentation of plan 08-19 should say so; no code change is needed.

**5. [Information] The `ticks` column is the golden's final tick, 7,168,352, not 7,168,320.** The plan says so ("the golden's final step end tick"); the 120-frame bound plus the 32-tick overshoot of the last unit.

**6. [Information] `_DARWIN_C_SOURCE` in `bench_host.c`, for Apple only.** Apple libc hides `_SC_NPROCESSORS_ONLN` under a strict `_POSIX_C_SOURCE`; the macro sits in the one POSIX file behind `#ifdef __APPLE__` (found by the first dev build, fixed before any commit). `runner-scan` covers `runner/` only.

**7. [Information] `tests/CMakeLists.txt` was edited in Task 3** (a comment that states which case reaches a clock: none); the file is in the plan's `files_modified` but not in Task 3's file list.

**8. [Information] STATE.md and ROADMAP.md are edited by hand by the closing commit,** as plans 08-14 to 08-17 did. PRF-01 is not marked in REQUIREMENTS: plan 08-19 closes it (accepted by the important-gates step, the baseline record).

**9. [Information] The first session of this plan was cut off by a network error during Task 1.** On resume the partial files (two registry lines, the idle scenario and golden, the workload table, `bench_host.c` and `.h`) were re-read and verified rather than trusted: the idle report was reproduced in both presets and agreed apart from `host.calls`, the golden and the table bytes were checked, and the remaining files were written after the plan was re-read.

No auth gates, no package installs (T-08-SC), no `gh` call, no push, no tag, no `tools/ci-record.sh`; `.planning/config.json`, `.planning/state.json` and `.planning/ci-runs.tsv` were never staged. No file under `core/`, `runner/` or `fixtures/` changed and no golden other than the two new ones exists in the commits (`git show --stat` of Task 2 lists exactly one golden). D10 and D11 are not performed, no claim row exists for a benchmark figure and every statement is about the repeatability of one build.

## Decisions Made

- A benchmark that can print a number for a wrong run is worse than none: the digest and the final tick are checked before any clock is read, and a refusal is a row with nothing in it, not a row with zeros.
- The gate pays for what it must: each workload is verified in both presets (the verify ids and the scenario ids), the timed path is never run by the gate, and the refusal cases take the cheapest workload.
- A tool's registration is part of its first commit: a tool that links the runner library and is not on the `c-build` list would be a source the build compiles and the unlisted-source guard does not see.

## Cost (laptop)

| Item | Value |
|------|-------|
| estimate of the plan | 130,000 tokens (raw, confidence low), 3 tasks |
| actuals | 19,957 tokens (chars/4 over the added lines of the 3 commits outside `.planning/`), 3 tasks, 3 commits (measured from the ledger of the plan head) |
| gate total, sh and dash, Task 1 tree | 84 s and 98 s (the dash run overlapped a container gate), identical summary line |
| gate total, sh and dash, Task 2 tree | 87 s and 81 s (the sh run overlapped a container gate), identical summary line |
| gate total, sh and dash, Task 3 tree | 78 s and 83 s (`gate-cpu` user 145.2 sys 115.1 and user 151.7 sys 124.8), identical summary line (md5 ecd692eb9fd3f28156695b7eed64a97d); plan start 77 s, `gate-cpu` user 139.5 sys 109.7; the difference is inside the load noise of the laptop (load average 12 to 18) |
| container gate (gcc 13.3.0, cmake 3.28.3, x86_64, `GATE_LINUX_PLATFORM=`) | Task 1 tree: exit 0, `gate: PASS`, `gate-time total=352`, 0 FAIL lines; Task 3 tree: exit 0, `gate: PASS`, the plan-end counts, `core-imports=5`, 0 FAIL lines, `gate-time total=347` (`gate-cpu` user 1,340 sys 90 emulated); closing tree (this SUMMARY, STATE.md and ROADMAP.md included): exit 0, `gate: PASS`, the plan-end counts, 0 FAIL lines, `gate-time total=351` |
| the `bench` ctest case | dev about 1.69 cpu-s (1.76 to 1.79 s wall), release about 0.25 cpu-s (0.25 to 0.28 s wall) |
| the two new scenario ids of `runner-process` | one `ivs-run` launch each per preset: dev 0.16 cpu-s each (user 0.08, sys 0.08), release 0.03 each |
| the control `c-build-unlisted-bench-source` | not separable from the load noise: the whole `controls` step read 24 to 26 s (35 s while a container gate ran) against 24 s at plan start |

**Cost position against the checkpoint C bound.** Measured local additions that can be named: the suite 1.94 cpu-s (dev 1.69, release 0.25) plus the two scenario ids 0.38 (dev 0.32, release 0.06), 2.32 in all, plus the control, whose share is not resolved. At the factor 1.675 of the record that is 3.9 hosted cpu-s, about +0.075 in `r` at a hosted `build_s` of 52 (44 to 63 gives +0.06 to +0.09). Against the last five hosted rows as the orchestrator gave them (median 7.86, newest 7.72, head ce469aa, which holds none of this plan) the projection reads about 7.94 by the median and 7.80 by the newest row: under the line 8.3 by 0.36 and under the reading line 8.0 by 0.06. **The remaining-plans bound of checkpoint C (about 0.4 local cpu-s for plans 08-18 to 08-21) is exceeded: this plan alone adds about 2.3 local.** The estimate of D-37 ("bench about +0.4 local") counted the runs and not the launches: about 0.09 cpu-s of sys time per process start under the sanitizers dominates a suite of short runs (15 launches in the dev preset). Plan 08-19 adds one verify id (one dev launch, about 0.2 cpu-s, and about 0.04 release). Trims that are named and not applied, each computed from launch counts and not measured: the eight `bench.refuse.bad-args` vectors reduced to four in the dev preset and run whole in the release preset (about 0.36 local), the run of one timed iteration of the mutant case on the release preset only (about 0.11), and one verify id sharing a launch with the format case (about 0.09). They trade dev-preset (sanitizer and leak) coverage of the error paths of the tool for time, so they are left as the owner's or the orchestrator's choice at the next checkpoint; lever (3) of the record stays next in the order.

## Open Items

1. **The cost bound of checkpoint C is exceeded by this plan (about 2.3 local cpu-s against 0.4 for all remaining plans)** while the projection stays under 8.3 (about 7.94 by the median, 7.80 by the newest row). The trims above are available; the `phase8-close` entry of plan 08-21 should read the next hosted rows with this plan in them.
2. **`lat_first_ms` is the first timed iteration of a warm process** (deviation 4); plan 08-19's document must not call it a cold-start figure.
3. **PRF-01 is not accepted by `important-gates.sh` yet and is not marked complete:** plan 08-19 adds it to the `nreq` list with its two registry lines (`bench.mutant.refuse-mutated-golden` and `bench.judge.refuse-has-no-timing` are the ids it names) and one toy line in each of the ten toy registries, as plan 08-17 did for STA-01 and STA-02.
4. **The snapshot cadence is refused by the tool until plan 08-19** (deviation 2).
5. **The timed path has run once, in the release preset on macOS arm64 on a loaded host,** and never on Linux: the container gate runs the tool with `--verify-only` only (as does every gate). The CPU time comes from `CLOCK_PROCESS_CPUTIME_ID`; where that macro is missing the CPU columns would be empty (not exercised).
6. **`docs/runner.md` does not list the two new golden commands** (the list was already incomplete); they are in this SUMMARY.
7. **Information.** D10 and D11 are not performed; no claim above provisional exists and none concerns a benchmark; REL-01 stays Pending (D-28).

## Interfaces for the next plans

- Plan 08-19: the table column `roundtrip_ticks` is parsed into `bench_workload.roundtrip_ticks`; the refusal to lift is in `bench_load_workload` (the text `roundtrip_ticks other than 0 is not supported by this tool yet (plan 08-19)`); the options struct is the zeroed `opts` of `bench_run`, passed to every `ivs_runner_execute` call (set `opts.flags = IVS_RUNNER_OPT_ROUNDTRIP` and `opts.roundtrip_ticks`; the 64-cut cap is `IVS_RUNNER_ROUNDTRIP_MAX_CUTS`; 1,792,080 ticks gives three cuts in the 120-frame run). The snapshot row shares the idle golden, so its verification is the digest-first proof that a round trip every 30 frames reproduces the idle report. The suite helpers to reuse are `tsv_check`, `verify_case`, `expect_refusal`, `no_measurement` and `synthetic_row` in `tests/bench-tap.sh`; the ids file header names plan 08-19.
- The suite names: `bench.verify.<workload>` (3), `bench.mutant.refuse-mutated-golden`, `bench.refuse.wrong-ticks`, `bench.refuse.bad-args`, `bench.tsv.format`, `bench.judge.refuse-has-no-timing`; 8 ids, 16 with the presets.
- Counts after this plan: `steps=38/38 controls=380/380 faults=235/235 fault-runs=235 tests-expected=4952 tests-executed=4948 tests-skipped=4 tests-missing=0 goldens=29 claims=28 provisional=27 gates=89 lines=304 pkg-selftest=19/19 state-rows=88 state-cuts=18`; plan 08-19 starts from them.
- Threat ids T-08-41 and T-08-42 are used here (T-08-SC throughout); the next plans continue from T-08-43 or the numbering their plans assign.

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=38/38 controls=380/380 ... faults=235/235 ... fault-runs=235 ... tests-expected=4952 tests-executed=4948 tests-skipped=4 tests-missing=0 ... claims=28 provisional=27 ... gates=89 lines=304 ... goldens=29 ... pkg-selftest=19/19 ... state-rows=88 state-cuts=18 (0 FAIL lines)
dash tools/gate.sh:    the identical summary line (0 FAIL lines)
GATE_LINUX_PLATFORM= sh tools/gate-linux.sh: Task 1 tree (352 s) and Task 3 tree (347 s, the plan-end code, data and registry tree): exit 0, gate: PASS, the plan-end counts, 0 FAIL lines; CLOSING TREE (tracked=2482, this SUMMARY, STATE.md and ROADMAP.md included): exit 0, gate: PASS, the plan-end counts, 0 FAIL lines, gate-time total=351
```

After the Task 1 tree: `controls=380/380 tests-expected=4936 tests-executed=4932 goldens=28`, sh 84 s and dash 98 s with the identical summary line. After the Task 2 tree: `tests-expected=4942 tests-executed=4938 goldens=29`, sh 87 s and dash 81 s, identical. After the Task 3 tree: the plan-end counts, sh 78 s and dash 83 s, identical (md5 ecd692eb9fd3f28156695b7eed64a97d). On the closing tree (this SUMMARY, STATE.md and ROADMAP.md added or edited; `tracked=2482`): sh (`gate-time total=96`, `gate-cpu` user 157.2 sys 133.6) and dash (92 s, user 155.6 sys 134.6) `gate: PASS` with the identical summary line (md5 a247fb0eaac765cbf5cef768f2c3a3c8), 0 FAIL lines and the plan-end counts above (the laptop load average was 18 to 19). The container gates ran in the Ubuntu 24.04 image (gcc 13.3.0, cmake 3.28.3, x86_64 under emulation on the arm64 host) and compiled `tools/bench` with `-Werror` under gcc without a warning; the dev preset there is the AddressSanitizer and UndefinedBehaviorSanitizer build (whether leak detection is active under the emulation was not checked, so no leak-freedom of the error paths is claimed). The Task 3 container summary line: `steps=38/38 controls=380/380 faults=235/235 fault-runs=235 tests-expected=4952 tests-executed=4948 tests-skipped=4 tests-missing=0 claims=28 provisional=27 gates=89 lines=304 goldens=29 pkg-selftest=19/19 state-rows=88 state-cuts=18`. The closing-tree container run read the same summary line with `tracked=2482`; the SUMMARY text was completed with this line after that run, which no gate step reads differently.

## Known Stubs

None.

## Threat Flags

None new. T-08-41 (a speed reported before or without the digest check): iteration 0 is compared with the committed golden and the table's final tick before any warmup or clock read; every warmup and timed iteration re-checks the outcome, the composite digest and the step end ticks; a refusal prints no measurement and exits 15; shown by `bench.mutant.refuse-mutated-golden`, `bench.refuse.wrong-ticks` and the judge, and by the two broken stand-ins that made the real cases fail; the tier column says `repeatability-only` and no claim row exists. T-08-42 (the clock leaking into the core, the runner or the gate): the clock calls exist only in `tools/bench/bench_host.c`, `bench.c` has none (the grep prints 0), the gate runs the tool only with `--verify-only`, no file under `core/` or `runner/` changed and no throughput threshold exists. T-08-SC: no installs.

## Self-Check: PASSED

Verified before the final commit: the created files of the plan exist and are tracked (`tools/bench/bench.c`, `bench_host.c`, `bench_host.h`, `CMakeLists.txt`, `tests/bench-tap.sh`, `data/expected/tests/bench.ids`, `data/bench/workloads.tsv`, the two scenarios and the two goldens, the three files of the control); commits `93a4b49`, `d9645ef` and `ef2032d` are ancestors of HEAD and `git rev-list --count ce469aa8c893bcaf10d6df18b588fa9a6ce5b96a..HEAD` printed 3 before this file's commit; nothing is staged but the files of this commit and `.planning/config.json`, `.planning/state.json` and `.planning/ci-runs.tsv` are not staged; `git ls-files` lists no `.ivsc`, `.ivsn`, `.bin`, `.rom`, `.exec`, `.wav`, `.ppm` or `.vraw` file; the diff of the three commits outside `.planning/` holds no home, scratch or temp path, no at sign before a dotted word and no "Intellivision"; no commit deletes a file; none touches `core/`, `runner/` or `fixtures/`; `grep -c 'ns_per_guest_cycle' tests/bench-tap.sh` prints 8; `grep -c '^bench[.]' data/expected/tests/bench.ids` prints 8 and the judge id appears once; the verify checks of Tasks 1 and 2 pass (`events=64 bad=0`, the six-event prefix `cmp`).
