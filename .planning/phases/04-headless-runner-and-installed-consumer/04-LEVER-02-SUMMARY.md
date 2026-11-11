---
phase: 04-headless-runner-and-installed-consumer
plan: LEVER-02
subsystem: gate, ci
tags: [gate, ctest, ci-efficiency, D-45, perf]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-14 end state: steps=29/29 controls=254/254 faults=140/140 tests-expected=2460; the 04-LEVER-01 Linux step profile (c-build 30, c-faults 20, c-tests 18 cpu-s, ctest serial)"
provides:
  - "c-tests runs ctest with -j <ncpu>, and each preset's ctest starts in the background as soon as that preset's build ends, so the dev and release suites overlap"
  - "ncpu probe in c-tests: nproc, then sysctl -n hw.ncpu, then getconf _NPROCESSORS_ONLN, else 2"
  - "c-tests still reports findings in preset order (build log, ctest log, ctest -N error); a signal stops the background ctest runs"
affects: [04-19, 04-25]

actuals:
  tokens: 4000
  tasks: 1
  commits: 1
plan_head_before: cec0e3d09aa1e5dbba3ead1f951ebceedeec1660
plan_head_after: 24c4850

tech-stack:
  added: []
  patterns:
    - "Start in the background, report in order: each job writes its own log and state file, and a second loop waits for the jobs in a fixed order and prints their findings, so the output never depends on scheduling"
    - "Fault shims on PATH (a fake ctest or cmake that fails one case) prove that the old and new step scripts give the same output and the same result files on every failure path"

key-files:
  created:
    - .planning/phases/04-headless-runner-and-installed-consumer/04-LEVER-02-SUMMARY.md
  modified:
    - tools/check/steps/c-tests.sh
    - .planning/CI-EFFICIENCY.md

key-decisions:
  - "The 12 ctest suites of each preset share no state, so no RESOURCE_LOCK or RUN_SERIAL is needed. Each suite writes only <results>/<preset>/<suite>.txt (memmap also writes memmap-dump.txt). The shell suites use their own mktemp dirs, the harness self-test uses tmpfile(), and the two presets have separate build and results dirs"
  - "Starting cp1610-vectors in the background at gate start was prototyped and rejected. It saved about 3 s on native Linux, which is within run-to-run noise. It would also make the gate's step loop special-case one step, turn that step's step-time line into a join wait, and let its CPU be counted in whichever step the shell happens to reap it"

requirements-completed: []

duration: about 1.5 h (mostly measurement)
completed: 2026-11-11
---

# Phase 4 Lever 02: Parallel ctest in c-tests (D-45) Summary

**c-tests now runs ctest with `-j <ncpu>` and runs the dev and release ctest at the same time. On a native 4-core Ubuntu 24.04 container (gcc 13.3, cold build, the hosted runner's shape) the step drops from 18.4 s to 7.2 s and the whole gate from 48 s to 35 s, with no change in gate CPU. Every check, token, count, result file and output line is unchanged apart from timing lines.**

## Independence check (before parallelising)

- **Result files:** the harness (`tests/harness/ivs_test.c`) writes `<IVS_TEST_OUT_DIR>/<suite>.txt` only. `memmap` also writes `memmap-dump.txt`, which no other suite writes. `core-symbols-tap.sh` and `headers-tap.sh` build their TAP in their own `mktemp -d` dirs and copy it to `<suite>.txt`. `tools/check/core-symbols.sh` uses its own `mktemp -d`.
- **Shared temp state:** none. The harness self-test captures through `tmpfile()`. No test source opens a fixed path, and `machine`, `run` and `memmap` are in-process suites over their own machine instances.
- **Presets:** `build/dev` and `build/release` are separate build trees with separate ctest `Testing/` dirs, and results go to `build/gate/dev/` and `build/gate/release/`. ASan and UBSan runs are separate processes.
- **No RUN_SERIAL / RESOURCE_LOCK added.** With a fixed nonce, old (serial) and new (parallel) `c-tests` give byte-identical `build/gate/{dev,release}/*` trees, including `ctest-list.txt` and `memmap-dump.txt`.

## What changed in `tools/check/steps/c-tests.sh`

- ncpu comes from the first probe that prints a positive integer: `nproc`, `sysctl -n hw.ncpu`, `getconf _NPROCESSORS_ONLN`. The default is 2. An absent tool prints nothing.
- Loop 1 builds each preset in turn. A stale binary is still never tested, because each preset's ctest starts only after that preset's own build ends. That preset's `ctest --preset <p> -j <ncpu>` then starts in the background, and `<p>.state` records `mkdir`, `build` or `run <pid>`.
- Loop 2 goes through the presets in order. It prints a build log and FAIL, or it waits for that preset's ctest (and on failure prints the ctest log and FAIL). Then it runs `ctest -N` into `ctest-list.txt`. The order of findings is the same as before.
- On HUP/INT/TERM the background ctest pids get TERM. This matters because background jobs of a non-interactive shell ignore SIGINT. Tested under sh and dash: TERM to the step during a run leaves no ctest or test process behind.
- Tokens (`c-tests-failed: <preset>`) and stats (`ctest-runs=<n>`) are unchanged. The only visible difference is on a failing run: the printed ctest log lists tests in completion order, as `ctest -j` does.

## Equivalence evidence

- **Fault shims (PATH), old vs new, under sh and dash:** 7 cases: none, dev ctest fails, release ctest fails, dev `ctest -N` fails, both ctest runs fail, release build fails, dev build fails. Output, exit status and result tree are identical in every case. The one difference is in `bothfail`, where the shim echoes its own arguments, which now include `-j 18`.
- **Full gate output, apart from step-time/gate-cpu/gate-time lines:** identical across all old and new runs. That covers 7 macOS sh, 6 macOS dash, 16 native Linux (5 old, 8 new, 3 prototype) and 6 emulated amd64 runs. The final `sh` and `dash` runs are identical to the pre-lever output.

## Before / after (cpu-s = user+sys; c-tests cpu from `times` taken after each step in a profiling copy of gate.sh)

| platform (runs) | c-tests wall s | c-tests cpu-s | gate wall s | gate cpu-s |
|---|---|---|---|---|
| Linux arm64 native, 4 cpus (cpuset), cold, 5 old / 5 new, alternating | 21 19 15 19 18 (18.4) -> 8 7 7 7 7 (7.2) | 18.6 -> 19.9 | 57 52 40 44 48 (48.2) -> 41 37 32 33 33 (35.2) | 96.9 -> 95.6 |
| Linux amd64 emulated, 8 cpus (`tools/gate-linux.sh`), 3 / 3 | 10 13 11 -> 4 4 3 | n/a | 126 159 142 -> 134 148 132 (controls-bound: 87-116 s) | 602 -> 628 (emulation noise) |
| macOS sh, warm, load 20-37, 3 / 3 | 5 5 5 -> 2 1 1 | 4.81 -> 5.11 | 38 41 38 -> 34 43 35 (controls-bound) | 116.3 -> 119.1 |
| macOS dash, warm, load 23-60, 3 / 3 | 6 6 5 -> 2 1 2 | 5.04 -> 5.01 | 44 44 41 -> 43 44 51 (controls-bound, load rising) | 120.2 -> 122.8 |

- Native Linux is the honest proxy for hosted (ubuntu-24.04, 4 cores, gcc 13, cold build). There, c-tests sits on the critical path, because the controls end after about 7 s. The gate falls by about 13 s (27 percent). c-tests CPU rises by about 1.3 cpu-s (7 percent of the step, about 1 percent of the gate), which is consistent with cache contention between overlapped ASan processes. Whole-gate CPU does not rise beyond noise.
- On macOS and in emulation, the overlapped controls step (20-37 s locally, 87-116 s emulated) bounds the gate, so the c-tests saving does not show in gate-time there.
- **Expected hosted effect:** c-tests at about 15-18 s hosted should fall to about 6-8 s, so roughly 10 s off `gate_s` (median of the last 3 rows: 55, 62, 62), with `cpu_s` unchanged. The next rows in `.planning/ci-runs.tsv` (orchestrator) will show the real effect.

## Levers applied

1. `ctest -j <ncpu>` per preset, and the two presets' ctest runs overlapped (each starts right after its own build).

## Levers evaluated and rejected

- **cp1610-vectors in the background from gate start:** this step needs no build (it only runs `cmake -P` scripts over `data/`), so running it early is safe in principle. A prototype in a scratch copy of gate.sh started it right after the controls, joined it at its sorted place and printed its buffered output there. Output was byte-identical. Native Linux, 4 cpus, cold, 3 pairs: gate 37 35 37 -> 39 32 29 s (mean -3 s), within noise; run 2 was slower because the job overlapped the core-saturated c-build window. Rejected for these reasons: (a) the gain is about 3 s and not clearly above noise; (b) the gate's generic step loop would need a named special case plus a second background job in the EXIT trap; (c) `step-time cp1610-vectors` would become a join wait (0 s), so the hosted log would lose that step's cost; (d) its CPU is charged to whichever step is running when the shell reaps it (dash charged it to c-build), which blurs the per-step profile that D-45 levers rely on. Revisit only if the cores turn out to be idle during c-build, which the hosted `cpu_s / (ncpu x gate_s)` of about 0.65 does not show for that window.
- **RUN_SERIAL / RESOURCE_LOCK:** not needed. No suite shares state (see the independence check).

## What remains the biggest hosted cost

- Cold `c-build` (about 30-50 cpu-s, 11-17 s wall on 4 native cores) and `c-faults` (about 20-25 cpu-s, 6-8 s wall, already in 4 lanes). The vectors JSON re-parse noted in 04-LEVER-01 is still deferred.

## Task Commits

1. **Lever: parallel ctest in c-tests** - `24c4850` perf(04)

## Final gate summary lines

- `sh tools/gate.sh` (macOS): `gate: PASS steps=29/29 controls=254/254 ... fault-runs=140 ... ctest-runs=2 ... faults=140/140 ... tests-expected=2460 tests-executed=2460 tests-skipped=0 tests-missing=0 ...`, `gate-cpu user=57.22 sys=55.97`, `gate-time total=31`, `step-time c-tests 1`.
- `dash tools/gate.sh`: the same summary line byte for byte; `gate-cpu user=54.48 sys=49.13`, `gate-time total=27`.
- `sh tools/gate-linux.sh` (Ubuntu 24.04 amd64 under emulation, gcc 13.3, cmake 3.28, dash), final tree: `gate: PASS steps=29/29 controls=254/254 ... cc=GNU ...`, `gate-cpu user=581.89 sys=47.23`, `gate-time total=132`, `gate-linux: exit 0`.
- Native arm64 Ubuntu 24.04 (4 cpus), final c-tests: `gate: PASS steps=29/29 controls=254/254 ...` in all 8 new-variant runs.

## Deviations from Plan

- This is not a numbered plan. STATE.md and ROADMAP.md counters are not touched, as the brief required.
- For per-step CPU, native arm64 Ubuntu runs were added next to the emulated `tools/gate-linux.sh` runs, because emulation makes the gate controls-bound and hides the c-tests change.

## Open Items

- D-45: confirm the hosted effect from the next `ci-runs.tsv` rows (expected: about -10 s `gate_s`, no change in `cpu_s`).
- Carried from 04-LEVER-01: the vectors JSON re-parse (cp1610-vectors and the cold-build generators) is still deferred.

## Self-Check: PASSED

- Files present: tools/check/steps/c-tests.sh (modified), this SUMMARY.
- Commit reachable from HEAD: 24c4850 (`git rev-list --count cec0e3d..24c4850` = 1).
