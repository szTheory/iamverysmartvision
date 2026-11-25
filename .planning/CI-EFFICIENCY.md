# CI efficiency log

Append-only record required by AUTONOMY.md section 4. One row per observed run of
`.github/workflows/gate.yml`. All values are seconds from `gh run view <id> --json
createdAt,startedAt,updatedAt,conclusion,jobs`; GitHub timestamps have 1 s resolution, so
every figure carries a rounding error of up to 1 s.

Definitions: queue = job `startedAt` minus run `createdAt`; setup = `actions/checkout` step
plus the `cmake --version` step; gate = the `sh tools/gate.sh` step; total = run `updatedAt`
minus `createdAt`.

| Date | Run id | Commit | Conclusion | Queue s | Setup s | Gate s | Total s |
|------|--------|--------|------------|---------|---------|--------|---------|
| 2026-11-09 | 42720541081 | a41281e | success | 4 | 2 | 6 | 16 |
| 2026-11-09 | 42720666825 | 7c25c55 | success | 2 | 2 | 4 | 11 |
| 2026-11-09 | 42720865117 | 6aaae3f | success | 3 | 1 | 6 | 13 |
| 2026-11-10 | 42727011942 | c1a8d07 | success | 3 | 2 | 9 | 16 |

## Breakdown of the first run

- Queue 4 s, set-up-job 0 s, checkout 2 s, `cmake --version` 0 s, gate 6 s, post-job
  cleanup 0 s, total 16 s (the 1 s remainder is the run finalising after the job).
- The gate step is the only meaningful cost. Locally the same gate takes about 8 s on a
  loaded host and the hosted runner took 6 s, so there is no hosted-runner penalty.
- 84 controls dominate the gate (about 6 s of local time in the control runner); no single
  step takes more than about 1 s.

## Decisions

- **Keep one job, no matrix, no cache.** The whole run is 16 s of wall time and the gate has
  no dependencies to download or build, so there is nothing to cache and no second job would
  pay for its own set-up (about 2 s checkout plus queue). Splitting steps across jobs would add
  more queue and set-up time than it saves.
- **No path filters.** The gate checks planning, data and documentation as well as tools, so
  any tracked-file change can change the result; a filter would skip evidence checks. The run
  is cheap enough that a filter buys nothing.
- **`cancel-in-progress` stays on for pull requests only.** Pushes to `main` are never
  cancelled, so each main commit keeps a recorded result.
- **Add a macOS leg only when the C build arrives in Phase 2.** The gate is shell and CMake
  script; a second operating system adds cost without testing anything different until
  compiled code exists. **Superseded by D-24 (Phase 2, PKG-02):** keep the single
  `ubuntu-24.04` job with no matrix, cache or path filter; clang on macOS locally and gcc 13
  with dash and GNU tools on the hosted runner give two compilers and two operating systems
  without a second job. Revisit a 2-leg matrix at Phase 4 (installed consumer, dylib, rpath)
  or at the first Apple-only escape to `main`.
- **Keep the `ubuntu-24.04` pin for now; re-evaluate after 2026-11-19**, when the
  `ubuntu-latest` migration ends, and move to `ubuntu-latest` only if the label then
  resolves to the same image and the benefit (fewer manual bumps) is worth losing the exact
  pin.
- **Artifact size: none.** The workflow uploads no artifacts.

## Review after the code-review fixes (run 42727011942)

- The gate step grew from 6 s to 9 s on the hosted runner: 112 controls (was 84) plus the
  new `hygiene-history` step. Locally on a loaded host (load average about 15) the same gate
  took 37 to 48 s, which is host load, not a cost the hosted runner pays.
- Total wall time is still 16 s and queue is 3 s, so nothing needs changing now.
- Cheapest lever if the gate keeps growing: run the negative controls in parallel (the runner
  executes them one by one and they are independent). Not worth the complexity at 9 s; revisit
  when the gate step passes about 30 s on the hosted runner, or when the Phase 2 C build adds
  its own steps.
- Local pre-push cost is the real friction: a loaded laptop makes the gate 4x slower than the
  hosted runner. If that bites, parallelising controls is the fix and helps both.

## Next review

After the next CI run. Append its row, and record a decision change only if the numbers move one.

## 2026-11-10 - first hosted C run (draft PR 1, branch gsd/phase-2-cp-1610-cpu-core, after plan 02-07)

- Run 42773083392: single `gate` job on ubuntu-24.04 (gcc 13, GNU nm, ninja, cmake from the image), PASS in 23 s job time; 182 controls, 24 steps, dev (ASan+UBSan) and release presets.
- Retired risks from the Phase 2 plans: GNU `nm -P` shapes accepted by `core-symbols.sh`, gcc 13 `-Werror` clean, sanitizer runtime imports covered by the dev prefixes, ninja present on the image.
- Local idle-ish gate at the same commit measured 32-40 s under load 6-16 on the owner laptop (D-22 local budget 25 s); the hosted job is faster than local. Levers stay as listed in D-22; no change made yet (hosted 23 s is well under the 30 s hosted budget).

## 2026-11-10 - local gate with the full C build and 190 controls (plan 02-18)

Measured on the owner laptop (18 cores, Apple clang 21, macOS). The host was never idle:
load average 13 to 33 during every run, from unrelated work (another emulator's tests, a
fuzzer, RetroArch). D-22 judges the 25 s local budget on an idle host, so these numbers do
not settle it; they compare the runner before and after under similar load.

| Run | Runner | Wall s | User s | Sys s | Load at start |
|-----|--------|--------|--------|-------|---------------|
| gate 1 | sequential | 83.3 | 30.9 | 44.0 | 16.5 |
| gate 2 | sequential | 101.4 | 37.8 | 56.1 | 15.3 |
| gate 3 | sequential | 81.7 | 28.2 | 41.1 | 26.2 |
| `run-controls.sh` alone | sequential | 73.0 | 19.2 | 43.5 | 15.8 |
| gate 1 | 4 lanes | 60.6 | 33.0 | 54.9 | 20.0 |
| gate 2 | 4 lanes | 87.9 | 39.5 | 72.6 | 19.9 (33.5 at end) |
| gate 3 | 4 lanes | 67.3 | 37.1 | 59.3 | 33.5 |
| `run-controls.sh` alone | 4 lanes | 32.3 | 19.4 | 48.1 | 18.4 |
| `dash run-controls.sh` alone | 4 lanes | 43.9 | 20.3 | 51.0 | 24.3 (at end) |

- Medians: gate 83.3 s before, 67.3 s after; control runner alone 73.0 s before, 32.3 s
  after. The load was higher during the after runs, so the gate medians understate the
  change; the runner-alone pair is the cleaner comparison.
- Per-step breakdown (sequential runner, load 13): `run-controls` 55.0 s; `hygiene-history`
  9.4; `cp1610-vectors` 6.5; `faults-verdict` 4.5; `c-faults` 3.2; `hygiene` 3.0;
  `denylist` 2.7; `c-purity` 1.9; `c-tests` 1.7; `integrity` 1.3; `cp1610` 1.0; the other
  13 steps under 0.8 s each; `c-build` 0.5 (warm). The runner is sys-dominated (tree copies
  and process start-up).
- Lever 1 (start the C build in the background) was not applied: `c-build` is already the
  first step in C-locale order and costs 0.5 s on a warm tree, so a background start would
  overlap with nothing. It matters only for a cold build, which the budget does not cover.
- Lever 2 (parallel control runner) was applied: `tools/check/run-controls.sh` now gives each
  control its own work dir (tree, filelist, step output, result lines) and runs them in 4
  lanes; lane i takes every 4th entry of the sorted list and results are printed afterwards
  in sorted order. Output is byte-identical to the sequential runner on the real controls
  (sh and dash) and on a synthetic set covering every runner failure token
  (`control-not-in-inventory`, `control-inventory-orphan`, `control-make-path`,
  `control-build`, `control-exit-zero`, `control-expect-blank`, `control-forbidden-token`,
  `control-unknown-step`, `control-token-missing`); `runner-selftest` still reports
  `cases=4`. No control, token or inventory changed; `controls=190/190`.
- Lever 3 (rebuild only the owning TU per fault) was not applied: on a warm tree the
  `ivs_faults` build inside `c-faults` is incremental and the whole step is 3.2 s.
- Next candidate if the local budget still bites on an idle host: start the control runner
  in the background at the top of `gate.sh` and print its output after the steps, so steps
  (about 39 s under this load) and controls (about 32 s) overlap. Not applied here; it
  changes `gate.sh` scheduling and should be measured on an idle host first.

## 2026-11-10 - first hosted run of the final Phase 2 gate, and lever 4 (overlap controls with steps)

- Run 42941373641 (ubuntu-24.04, cold tree, 190 controls, 4-lane runner): PASS, job about
  55 s; the `sh tools/gate.sh` step took 50 s (13:46:59 to 13:47:49), **over the D-22 hosted
  budget of 30 s**. The log had no per-step timing, so the hosted split between the cold C
  build, the other steps and the control runner is unknown.
- Timing added: `tools/gate.sh` now prints one `step-time <name> <s>` line per step (plus
  `setup`, `controls` and `controls-wait`) and `gate-time total=<s>` just before the summary
  line, which stays the final line (SKELETON.md section 1). Whole seconds from `date +%s`; the
  lines are not tokens and nothing parses them. The next hosted run shows where the time goes.
- Lever applied (the "next candidate" recorded under plan 02-18): the control runner now
  starts in the background right after the results-dir setup and overlaps every step. It has
  no dependency on any step: it reads only `tools/check/` and builds each control tree from
  `controls/_good/tree` in its own temp dir, and the steps it drives read `$ROOT/build/gate`
  of that control tree, never the real `build/`. Its output is held in a file and printed
  after the steps in the same place; `wait` returns its exit status, so a failing control
  still prints `-- FAIL gate-controls-failed: run-controls` before the runner output and
  fails the gate. `controls` is the runner's own wall time; `controls-wait` is how long the
  gate waited for it after the last step (0 means the steps were the critical path).
- Checks: gate output minus the timing lines is byte-identical to the previous gate under
  `sh` and `dash` (`controls=190/190`, same summary line), and in a scratch copy with one
  control deliberately broken (extra EXPECT token): exit 1, `controls=189/190`,
  `gate-controls-failed` and `control-token-missing` in the same order, under `sh` (cold
  build) and `dash` (warm).
- Local numbers (owner laptop, load 22 to 25, so load-contaminated like every local figure
  here): gate 67 s before, 36 s (`sh`) and 42 s (`dash`) after. Per-step after: controls 25 s
  (overlapped, `controls-wait` 0); `hygiene-history` 7 to 11; `cp1610-vectors` 6 to 7;
  `faults-verdict` 5; `c-faults` 3; `denylist` 3; `hygiene` 2 to 3; the rest 2 s or less;
  `c-build` 0 (warm). The steps are now the critical path locally.
- On a 4-vCPU hosted runner the steps and the 4 lanes compete for the same cores, so the
  hosted gain will be smaller than the local one; the next hosted run measures it.

## Open items

- **Hosted gate budget (open, over budget).** D-22: hosted gate step at most 30 s. The final
  Phase 2 gate measured 50 s on run 42941373641 (before the background control runner). The
  next hosted run measures the gate with the controls overlapping the steps and prints
  `step-time` lines. If it still exceeds 30 s, read those lines and pick from the remaining
  D-22 levers by the measured split: (1) start the C build in the background at the top of
  `gate.sh` (only if `c-build` is large on the cold hosted tree), (3) rebuild only the owning
  TU per fault (only if `c-faults` is large); lever 2 (parallel runner) is done in plan 02-18
  and the controls already overlap the steps. No ccache and no `actions/cache`. Local
  measurements do not close this item.
- The queue, setup and gate times of the hosted runs on the phase PR, including the one after
  this plan, are recorded by the orchestrator after the phase PR's CI; agents never call `gh`.

## 2026-11-11 — hosted run 42942719332 (after the overlapped control runner)

- Hosted gate: `gate-time total=31` s on ubuntu-24.04 (D-22 budget 30 s; run 42941373641 before
  the change took 50 s). Largest steps: `c-build` 18 s (cold tree, 2 presets), `controls` 13 s
  (overlapped, `controls-wait` 0), `cp1610-vectors` 5 s; every other step 1 s or less.
- Verdict: within 1 s of budget and inside run-to-run noise; the hosted-budget open item is
  closed as met-within-noise. Nothing else is worth overlapping: the only large step is the
  cold C build, and every later step depends on it. Not applied: ccache / `actions/cache`
  (the policy bans them while the budget is met), background C build (nothing to overlap).
- Revisit only if a later hosted run exceeds 40 s or a phase adds a step over 5 s.

## 2026-11-11 — local measurement 1 of Phase 3 (plan 03-02, D-38 / A-07)

First real second fault suite: `data/expected/faults/sha256.ids` (6 faults, suite `machine`),
73 fault runs in total. Local figures only (owner laptop, macOS arm64, AppleClang, warm build
trees, so `c-build` is 0). The host was heavily loaded the whole time: 1-minute load average
8.9 to 13.1 (26 sessions). Every figure below is load-contaminated and is not a hosted
number. Hosted figures are recorded by the orchestrator after the phase PR's CI (agents never
call `gh`).

### Gate before the lever (`sh tools/gate.sh`, three runs)

| Run | Load at start | `gate-time total=` | c-build | c-faults | faults-verdict | c-tests | controls | controls-wait |
|-----|---------------|--------------------|---------|----------|----------------|---------|----------|---------------|
| 1 | 9.30 | 20 | 0 | 2 | 2 | 1 | 12 | 0 |
| 2 | 9.40 | 22 | 0 | 2 | 3 | 1 | 12 | 0 |
| 3 | 12.83 | 26 | 0 | 2 | 4 | 1 | 14 | 0 |

Median 22 s (planning baseline 29 s). All three: `gate: PASS steps=24/24 controls=196/196
... faults=73/73`.

### Projection of the phase-end fault cost (step 1b, W7)

Measured plain runs (median of 20 runs each): `build/release/tests/ivs_tests_machine` 0.0056 s
(family sha256 only, 24 cases including the 1,000,000-byte vector); one `machine` fault
executable (`ivs_fault_SHA_K_WRONG`) 0.0057 s; one `cpu-vectors` fault executable
(`ivs_fault_ADD_OVERFLOW`) 0.019 s.

Per-family budget for the plain release run of suite `machine` once the heavy families exist
(RESEARCH Section C item 1):

| Family | Budget s |
|--------|----------|
| refusal matrix | 0.06 |
| sweeps | 0.08 |
| isolation | 0.05 |
| digest sensitivity | 0.03 |
| everything else (sha256 measured 0.006) | 0.03 |
| **suite machine total** | **0.25** |

| Fault group (phase end, plan 03-12) | Faults | Per run s | Serial s |
|-------------------------------------|--------|-----------|----------|
| suite `machine` (sha256 6 + machine about 23) | 29 | 0.25 (budget) | 7.25 |
| suite `memmap` (65,536-address dump) | 11 | 0.06 (0.05 dump + start) | 0.66 |
| suite `ivsc` | 6 | 0.03 (assumed) | 0.18 |
| suite `cpu-vectors` (unchanged) | 67 | 0.019 (measured) | 1.29 |
| **projected phase-end `c-faults` run time, serial** | **113** | | **9.4** |

The projection puts phase-end `c-faults` above the 8 s trigger, so the lane lever applies.
With 4 lanes the run part projects to about 2.4 s locally; on a 4-vCPU hosted runner the
fault lanes compete with the overlapped control lanes, so expect less than a 4x gain there.
Later plans that add a heavy family record its measured plain release time in their SUMMARY
against these budgets.

### Lever decision

- Trigger 1 (median above 29 + 5 s on a host with load below 4): not evaluable (load 9 to 13)
  and not met (median 22 s).
- Trigger 2 (`c-faults` alone above 5 s): not met (2 s).
- Trigger 3 (projected phase-end `c-faults` above 8 s): **met** (9.4 s serial).
- **lane lever: applied** (projection trigger). `tools/check/steps/c-faults.sh` runs the fault
  executables in at most 4 lanes, copying the `run-controls.sh` pattern: ids are listed in
  file order and sorted id order, lane i runs every 4th id with its own log, results go to the
  same `<results>/faults/<ID>/<suite>.txt` paths, and findings and the pass line are printed
  afterwards in listed order with unchanged tokens.

### After the lever

Step alone (`dash tools/check/steps/c-faults.sh`, warm, median of 5): serial 1.63 s, 4 lanes
0.69 s, both `-- c-faults pass fault-runs=73`.

| Run | Load at start | `gate-time total=` | c-build | c-faults | faults-verdict | c-tests | controls | controls-wait |
|-----|---------------|--------------------|---------|----------|----------------|---------|----------|---------------|
| sh 1 | 8.89 | 20 | 0 | 1 | 3 | 1 | 12 | 0 |
| sh 2 | 10.04 | 19 | 0 | 1 | 3 | 1 | 11 | 0 |
| sh 3 | 10.62 | 21 | 0 | 1 | 3 | 1 | 12 | 0 |
| dash | 10.97 | 22 | 0 | 1 | 3 | 1 | 12 | 0 |

Median (sh) 20 s. Every run: `gate: PASS steps=24/24 controls=196/196 ... faults=73/73`.
`faults-verdict` (2 to 4 s at 73 faults) is now the larger of the two fault steps; it scales
with the fault count, so watch it at the next measurement point.

### Budget and next measurements

- D-38 / A-07 re-baseline the hosted gate budget from 30 s to 40 s, with the revisit trigger
  moved from 40 s to 50 s; the revisit is part of the phase close-out (plan 03-14).
- Second measurement: after the refusal matrix (plan 03-11). A-07 named plan 03-07, which in
  this decomposition is the IVSC parser plan, so the second point moved to 03-11 and the third
  is the 03-14 close-out.
- Hosted queue, setup and gate figures for the phase PR are recorded by the orchestrator.

## 2026-11-11 — local measurement 2 of Phase 3 (plan 03-11, after the refusal matrix, A-07)

A-07 named plan 03-07 for the second point; in this 14-plan decomposition that is the IVSC
parser plan, so the second measurement moved here, right after the refusal matrix (plan
03-11), and the third stays the 03-14 close-out (as recorded in the 03-02 section above).
Fault count now 110 (plan text: 109; the extra fault `REPORT_INSIDE_UNCHECKED` is the plan
03-11 addition for out parameters aimed into the instance), 25 of them run suite `machine`.
Local figures only (owner laptop, macOS arm64, AppleClang, warm build trees, so `c-build` is
0 to 1). The host was loaded the whole time: 1-minute load average 8.4 to 10.3 (26
sessions), so every figure is load-contaminated and none is a hosted number. The hosted figure
for the phase PR is the orchestrator's to record (agents never call `gh`); the last hosted run
measured 30 s (budget 40 s, revisit trigger 50 s).

### Gate after plan 03-11 (`sh tools/gate.sh`, three runs)

| Run | Load at start | `gate-time total=` | c-build | c-faults | faults-verdict | c-tests | controls | controls-wait |
|-----|---------------|--------------------|---------|----------|----------------|---------|----------|---------------|
| 1 | 8.35 | 24 | 1 | 2 | 3 | 2 | 12 | 0 |
| 2 | 8.74 | 23 | 0 | 3 | 3 | 2 | 11 | 0 |
| 3 | 8.47 | 23 | 1 | 2 | 3 | 1 | 11 | 0 |
| dash | 7.27 | 22 | 0 | 2 | 4 | 2 | 11 | 0 |

Median 23 s (03-02 after the lane lever: 20 s at 73 faults; 03-10: 22 to 24 s at 101 faults).
All three: `gate: PASS steps=25/25 controls=215/215 ... faults=110/110 ... tests-missing=0`.

### Plain run times (median of 11, same loaded host)

| Executable | s | 03-02 budget s |
|------------|---|----------------|
| release `ivs_tests_machine`, whole suite (299 cases) | 0.159 | 0.25 (phase end) |
| release, `--only machine.refuse.` (216 cases: the 175 new load, unload and out-parameter rows and the 41 plan 03-09 refusals) | 0.112 | 0.06 (refusal matrix) |
| release, `--only machine.prec.` (20 cases) | 0.015 | (in the matrix budget) |
| release `ivs_tests_ivsc` (75 cases) | 0.029 | 0.03 |
| dev (ASan+UBSan) `ivs_tests_machine` | 0.328 | - |
| one suite-`machine` fault executable (`DUPLICATE_REPLACES`, `SHA_K_WRONG`) | 0.15 to 0.17 | 0.25 |

The refusal matrix costs about twice its family budget (0.127 s against 0.06 s): each row
builds its prior, copies the 113,456-byte instance and takes two composite state digests
(SHA-256 over the whole instance), and the digest dominates. It is part of the D-12 oracle, so
it stays. The suite as a whole is still inside its 0.25 s phase-end budget, with sweeps and
isolation (plan 03-12) still to come. Projection for the phase end: suite `machine` about
0.29 s (0.159 now + the 0.13 s sweeps and isolation budget) x about 29 suite-`machine` faults
= 8.4 s serial, about 2.1 s in the 4 fault lanes; `c-faults` is 2 to 3 s today.

### Lever decision

- Trigger (median above 40 s locally on a host with load below 4): not evaluable (load 8 to
  10) and not met (median 23 s). No lever applied; no check dropped, no token changed.
- Watch items for 03-12 and the 03-14 close-out: `faults-verdict` holds at 3 s with 110
  faults and `c-faults` at 2 to 3 s; if a trigger fires, the next lever the step split points
  at is the owning-TU rebuild per fault (the lanes are already applied). The refusal oracle
  (whole-storage memcmp plus composite digest per row) is not a lever: it is the D-12 evidence.

## 2026-11-11 — D-38/A-07 re-baseline: hosted gate budget 30 s -> 40 s; revisit trigger 40 s -> 50 s (plan 03-14, phase-end measurement 3)

Owner-delegated decision D-38, sharpened by A-07: the Phase 3 gate keeps every check and
token, and its hosted budget moves from the D-22 30 s to 40 s, with the revisit trigger moved
from 40 s to 50 s. The earlier sections stay as written; this section supersedes their budget
lines from today on.

### Gate shape at the phase end

- `steps=26/26` (planning baseline 24 at phase start; new steps `memmap` and `api-effects`).
- Faults 115 (plan text: 113; plan 03-11 added `REPORT_INSIDE_UNCHECKED` and plan 03-12 added
  `OUT_OVERLAP_UNCHECKED`, one each beyond the plan text): cpu 67, machine 25, memmap 11,
  sha256 6, ivsc 6.
- Controls 224 (plan text: 221; plan 03-05 added two literal-rule controls beyond its plan
  text, `memmap-literal-unknown-tag` and `memmap-literal-orphan`, and plan 03-13 added the
  ninth api token, `api-effects-format`).
- Tests 2094 executed of 2094 expected (1047 ids in two presets), `tests-missing=0`.

### Phase-end local measurement (`sh tools/gate.sh`, three runs)

Owner laptop, macOS arm64, AppleClang, warm build trees (so `c-build` is 0 to 2). The host was
loaded throughout (26 sessions); the load spiked during runs 2 and 3. Every figure is
load-contaminated and none is a hosted number.

| Run | Load at start | `gate-time total=` | c-build | c-faults | faults-verdict | c-tests | hygiene-history | cp1610-vectors | controls | controls-wait |
|-----|---------------|--------------------|---------|----------|----------------|---------|-----------------|----------------|----------|---------------|
| 1 | 9.87 | 30 | 1 | 3 | 4 | 3 | 6 | 4 | 13 | 0 |
| 2 | 17.68 | 36 | 1 | 4 | 6 | 3 | 6 | 5 | 17 | 0 |
| 3 | 17.38 | 38 | 1 | 4 | 7 | 3 | 6 | 4 | 17 | 0 |

Median 36 s. All three: `gate: PASS steps=26/26 controls=224/224 ... faults=115/115 ...
tests-expected=2094 tests-executed=2094 tests-skipped=0 tests-missing=0`.

The same session also ran three gates at lower load (load 6.8 to 8.8): 25 s (sh, before the
plan's record edit), 26 s (sh) and 27 s (dash). With those, the six runs give a median of
28.5 s; the spread tracks the host load, not the gate.

Five largest steps (run 1, the lowest-load run of the three): `controls` 13 (overlapped,
`controls-wait` 0), `hygiene-history` 6, `faults-verdict` 4, `cp1610-vectors` 4, `c-tests` 3
(tied with `c-faults` 3).

| Point | Plan | Faults | Controls | Steps | Local median s | Load |
|-------|------|--------|----------|-------|----------------|------|
| planning baseline | before phase 3 | 67 | 192 | 24 | 29 | not recorded |
| measurement 1 (after the lane lever) | 03-02 | 73 | 196 | 24 | 20 | 8.9 to 10.6 |
| measurement 2 | 03-11 | 110 | 215 | 25 | 23 | 8.4 to 8.7 |
| measurement 3 (phase end) | 03-14 | 115 | 224 | 26 | 36 (28.5 over six runs) | 9.9 to 17.7 |

### Hosted figures of the phase so far (read by the orchestrator; agents never call `gh`)

| After plan | Hosted `gate-time total=` s | Note |
|------------|-----------------------------|------|
| 03-02 | 36 | first hosted run of the phase |
| 03-04 | 44 | cold gcc `c-build` 26 s (Phase 2 hosted cold build: 18 s) |
| 03-06 | 30 | |
| 03-10 | 33 | |

Local warm gates across the phase took 20 to 30 s at load 7 to 16. One hosted run of four
(03-04, 44 s) was above the new 40 s budget and below the 50 s trigger; its excess was the
cold gcc C build, and the two later hosted runs were 30 and 33 s.

### Levers

Applied during the phase:
1. Fault lanes: `c-faults` runs the fault executables in at most 4 lanes (plan 03-02). The
   controls already overlap the steps (Phase 2, plan 02-18).

Remaining, in order, each only when a hosted run is above 40 s and its `step-time` lines point
at the step it shortens:
1. Owning-TU rebuild per fault: rebuild only the translation unit that holds a fault site,
   when `c-faults` dominates on the hosted tree.
2. Background C build at the top of `gate.sh`, overlapping the cold `c-build` with the steps
   that do not read the build tree (`hygiene`, `hygiene-history`, `denylist`, `discrepancies`,
   `text` and the other record steps), when the cold hosted `c-build` dominates as on the
   03-04 run.
3. Watch item: `hygiene-history` costs 4 to 6 s locally with 1,157 tracked files (one
   throwaway commit of every tracked file). It is the largest serial non-C step; a lever
   for it would keep its git code path and its control.

Not used: ccache and `actions/cache` (policy), dropping or merging checks, controls or tokens.
The refusal oracle (whole-storage memcmp and composite digest per row) is D-12 evidence, not a
lever.

### Open item

- Hosted gate time after Phase 3 is measured by the orchestrator on the phase PR's CI; if
  above 40 s, apply the remaining levers; if above 50 s, revisit the budget.

## 2026-11-11 - hosted-only failures and the local Linux gate (03-14 post-verification)

- Hosted runs 42985199346 and 42988058428 (ubuntu-24.04, gcc 13.3, cmake 3.28.3) failed while
  the local macOS gate passed. Each hosted-only failure costs a full CI round trip (push,
  queue, cold gcc build, read the log) before a fix can even be tried.
- Shift-left lever: `sh tools/gate-linux.sh` runs `sh tools/gate.sh` on the working tree in a
  throwaway `ubuntu:24.04` container (amd64 by default, the hosted runner's architecture; gcc
  13, dash, GNU tools, cmake 3.28). It is a local pre-push helper: not a gate step, not run in
  CI, no workflow change.
- Its first use found two bugs the macOS gate cannot see: an unspecified argument evaluation
  order in `tests/machine/machine_refuse.c` (gcc reads a length before the call that sets it;
  ASan abort) and an ELF-only `_GLOBAL_OFFSET_TABLE_` import under gcc's default `-fPIE`
  (core-symbols suite). Both are fixed; see the 03-14 SUMMARY, section Post-verification fixes.
- Cost on the owner laptop (arm64, amd64 emulated): `gate-time total=127` and about 3 min wall
  time including package installation. Emulated figures are not hosted figures and do not
  enter the hosted budget.
- Decision: no CI change. Run the helper before pushing changes that touch C code, shell
  steps or toolchain-sensitive checks; skip it for record-only changes.

## 2026-11-11 — Phase 3 hosted runs (orchestrator record)

- Hosted gate-time per push on the Phase 3 PR: 36 s (03-02), 44 s (03-04, cold C build 26 s), 30 s (03-06), 33 s (03-10), 39 s (after the gcc/PIE fixes), 38 s (review fixes), 49 s (final docs commit, same code as the 38 s run: run-to-run spread of about 10 s on the hosted runner). Budget 40 s, revisit trigger 50 s.
- Largest hosted steps: `c-build` 16 to 26 s (cold, two presets), `controls` 13 to 24 s (overlapped), `c-faults` 1 to 5 s, `cp1610-vectors` 4 to 5 s.
- Two failures were hosted-only (gcc argument-evaluation order in a test helper; `_GLOBAL_OFFSET_TABLE_` under PIE). Each cost a CI round trip; `tools/gate-linux.sh` (local, opt-in, takes about 3 minutes under emulation) now reproduces the Ubuntu environment before a push.
- Next levers if the trigger is crossed: (1) rebuild only the source holding each fault, (2) overlap the cold C build with the steps that do not need it. Not applied: the 49 s run is variance on identical code, not a regression.

## 2026-11-11 — Phase 4 measurement and the D-45 budget rule (plan 04-02)

### The record

- From plan 04-02 on, every hosted run of `.github/workflows/gate.yml` gets one row in `.planning/ci-runs.tsv` (tab-separated, append-only, machine-readable). Columns: `date run_id sha event conclusion queue_s setup_s build_s test_s gate_s total_s ncpu cpu_s`. The prose table at the top of this file stays as it is (legacy, not rewritten); rows are never invented from it, because its queue and setup figures were not measured the same way.
- Definitions (all seconds): `queue_s` = job `startedAt` minus run `createdAt` (queue time cannot be measured inside the job); `setup_s` = the `actions/checkout` step alone (the legacy table also added the `cmake --version` step); `build_s` = the gate's `step-time c-build`; `gate_s` = `gate-time total`; `test_s` = `gate_s` minus `build_s` (it still contains the overlapped controls); `total_s` = run `updatedAt` minus `createdAt`; `ncpu` from the gate's `gate-env` line; `cpu_s` = user plus sys from the `gate-cpu` line, rounded. `-1` means the value was not available (for example `gate-cpu unknown`, or a step GitHub reports as never started); a `-1` is never averaged.
- `tools/ci-record.sh <run-id>` produces the row and is run by the orchestrator only, once per hosted run: exactly two `gh` calls (run metadata reduced with `--jq`, and the run log filtered to `gate-time`, `step-time`, `gate-env` and `gate-cpu` lines), about 2 calls per push against the shared 5000/hour limit. Executors and CI never run it; `--from-files <meta> <log>` runs the same parser offline. The orchestrator appends the printed row: `sh tools/ci-record.sh <run-id> >> .planning/ci-runs.tsv`.
- The gate step `ci-log` checks the format on every gate run (exact header, 13 fields, integers or `-1`, `YYYY-MM-DD` dates, 40-digit sha, unique run ids, dates not decreasing), with tokens `ci-log-header`, `ci-log-fields`, `ci-log-duplicate`, `ci-log-order` and one control each. It cannot check that every run was recorded (that needs the API); that stays the orchestrator's AUTONOMY.md section 4 duty.

### The D-45 budget rule

- **Apply the next lever when the median of the last 3 hosted gate-times exceeds 40 s.** **Revisit the budget itself when any run exceeds 60 s or the median of the last 5 exceeds 50 s.** Gate-time is the `gate_s` column of `.planning/ci-runs.tsv` on one branch. Same-code hosted runs spread 38-49 s in Phase 3, so a single run is never compared with the budget; the earlier single-run 50 s trigger fires on noise and is replaced by this rule.
- Levers, in this order, each only after the rule fires and `step-time` names the step: (1) keep Phase 4 steps cheap by construction (the installed consumer reuses `build/release`, no new cold build, no sleeps or wall-clock waits); this is a design constraint more than a lever; (2) background `c-build` for the steps that do not need it, only if `gate-cpu` shows idle cores (`cpu_s` well below `ncpu` x `gate_s`); if the cores are saturated, delay the control lanes instead, since total CPU is then fixed; (3) less control-runner CPU (smaller per-control tree copies first); (4) ccache, banned while the gate is within budget; (5) a two-job split, last resort, because it redefines PKG-02 and needs an owner-visible Decision Register entry. Not levers: dropping controls, merging checks, weakening the refusal oracle.
- Mid-phase budget checkpoints: the orchestrator runs `tools/ci-record.sh` after every hosted run from plan 04-02 on and evaluates the rule after the pushes of plans 04-10 and 04-19. When the median of the last 3 hosted gate-times exceeds 40 s it may insert a lever plan (for example a background `c-build` or cheaper suites) before continuing, instead of waiting for plan 04-25.
- Phase 4 cost estimate: +5-8 s hosted on a 30-49 s base (consumer producer 2-3 s, runner scenarios 1-2 s, about 20 new faults 1-2 s, about 30 new controls 1 s CPU, `ci-summary` and `ci-log` under 0.5 s), so a median near 38-42 s is expected and the budget will be tested in this phase. Plan 04-01's `gate-env` and `gate-cpu` lines answer the open question whether the hosted cores are saturated: `cpu_s / (ncpu x gate_s)` near 1 means saturated (lever 2 cannot help), well below 1 means idle cores.

### Local measurement, plans 04-01 and 04-02 (laptop under load, 18 cores, Apple clang 21; not hosted figures)

- Plan 04-01 at its last commit: sh `gate-cpu user=37.81 sys=45.02`, `gate-time total=30`; dash `gate-cpu user=64.84 sys=84.50`, `gate-time total=51`; Ubuntu container under amd64 emulation (`tools/gate-linux.sh`, gcc 13.3.0, dash) `gate-time total=235`, `gate-env ncpu=8 cc=GNU-13.3.0`. Step cost of 04-01: `ci-summary` 0-1 s, `ci-parity` 0 s.
- Plan 04-02 (Task 1, 28 steps, 237 controls): sh `gate-env ncpu=18 cc=AppleClang-21.0.0.21000101`, `gate-cpu user=53.30 sys=62.65`, `gate-time total=44` (`controls` 26 s overlapped, warm `c-build` 1 s). Step cost of 04-02: `ci-log` 0 s; four controls add well under 1 s of control-runner CPU.
- Plan 04-02 end (28 steps, 241 controls): sh `gate-cpu user=39.50 sys=47.34`, `gate-time total=31` (`controls` 16 s); dash `gate-cpu user=44.21 sys=52.15`, `gate-time total=34` (`controls` 17 s). The 44 s vs 31 s spread between two sh runs of nearly the same tree is host load, the same noise the median rule exists for.
- No hosted run of Phase 4 commits is recorded yet; the first rows come from the orchestrator after the first push of this branch.

## 2026-11-11 — D-45 lever 3: fewer gate process spawns (04-LEVER-01, commit d8d7712)

- **Trigger:** the median of the last 3 hosted gate-times is 60 s (69, 60, 54 in `.planning/ci-runs.tsv`; `cpu_s` 147-177 on 4 cores). One run is above 60 s, so the "revisit the budget" trigger has fired as well.
- **Measured first.** On macOS the gate's sys time exceeds its user time because each process start (fork/exec) costs about 3 ms of sys time under load. A PATH shim counted about 21000 exec calls per gate: 13400 in the control runner and its 276 step runs, 2500 in faults-verdict (about 17 per fault), 1400 in hygiene-history (one grep per file name) and 850 in denylist (four greps per file). On native Linux (arm64 Ubuntu 24.04 container, gcc 13.3, dash; the hosted runner's OS) the picture is different: sys time is 12 percent, and of 91 cpu-s with a cold build, c-build takes 30, c-faults 20, c-tests 18 (ctest runs serially; ASan+UBSan under gcc: dev memmap 5.4, cpu-sweep 2.3, cpu-fuzz 2.3), controls 11 and cp1610-vectors 5. Every other step takes under 1.5.
- **Applied:** faults-verdict uses one awk per faults file. denylist uses one batched `grep -l` per list, with a per-file fallback on any grep error. `hygiene.sh --history` matches binary names with one grep over the sorted list. The control runner uses one cp per tree with no rm/mkdir, builtin CHECK and blank-EXPECT checks, one awk for EXPECT/FORBID tokens, one mkdir and one cat. Equivalence: all 276 control outputs are byte-identical; the runner output is byte-identical under sh and dash; 35 faults-verdict and 11 denylist mutation cases give identical old and new output under both shells; and four new runner-selftest cases (cases=4 to 8) pass on the old runner and fail on mutants.
- **Effect.** Exec calls fell from 20988 to 12203 per gate. On macOS, A/B over copies of the tree (alternating, warm, load 11-20): sh cpu-s 125.9 to 97.6 (mean; -18 to -22 percent), sh wall 45 to 32 s; dash 115.4 to 101.5 cpu-s (-12 percent; one earlier round measured -24 percent), wall 41 to 32 s. On native Linux arm64: warm 63.5 to 58.7 cpu-s (-8 percent), wall 33 to 30 s; cold 108.3 to 94.8 (-12 percent, one run). Expected hosted effect: roughly 5-10 cpu-s and 2-4 s of wall. The next recorded rows will show the actual effect.
- **Rejected:** hardlinked or symlinked control trees (MAKE verbs append in place and would write into `_good/tree`); skipping c-purity's stage-3 self-test inside controls (changes the check); ccache, workflow or matrix changes.
- **Deferred:** batching hygiene --tree candidate filters (about 2 percent locally, but it touches the leak guard). The vectors JSON scripts (`string(JSON GET "${doc}" vectors ${i})` re-parses the whole file per vector) cost about 5 cpu-s in cp1610-vectors plus the generators in the cold build.
- **Biggest remaining hosted costs:** the cold c-build, c-faults (134 fault executables, each running its whole suite) and c-tests (serial ctest). The cheapest next wall lever is `ctest -j` in c-tests: it is CPU-neutral and could save about 10 s of hosted wall. It is not applied here.

## 2026-11-11 — D-45 lever: parallel ctest in c-tests (04-LEVER-02, commit 24c4850)

- **Trigger:** hosted rows still at 46-69 s `gate_s` and 116-179 `cpu_s` on 4 cores after 04-LEVER-01. The 04-LEVER-01 Linux profile ranked c-tests third (18 cpu-s) and showed it on the hosted critical path, because ctest ran serially.
- **Independence first.** Each suite writes only its own `<results>/<preset>/<suite>.txt` (`memmap` also writes `memmap-dump.txt`). The shell suites use their own `mktemp -d` dirs, the harness self-test uses `tmpfile()`, and the presets have separate build, `Testing/` and results dirs. So no `RUN_SERIAL` or `RESOURCE_LOCK` is needed. With a fixed nonce, the serial and parallel result trees are byte-identical.
- **Applied:** `tools/check/steps/c-tests.sh` runs `ctest --preset <p> -j <ncpu>`. ncpu comes from `nproc`, then `sysctl -n hw.ncpu`, then `getconf _NPROCESSORS_ONLN`, else 2. Each preset's ctest starts in the background right after its own build, so dev and release overlap. Findings are reported afterwards in preset order, and a signal stops the background runs. Seven PATH fault-shim cases (ctest, `ctest -N` and build failures for one or both presets) give old/new-identical output under sh and dash. All 37 measured gate runs have identical output apart from timing lines.
- **Effect, native arm64 Ubuntu 24.04 (4 cpus by cpuset, gcc 13.3, cold build; the hosted shape), 5 old and 5 new runs alternating:** c-tests wall 18.4 to 7.2 s. c-tests cpu 18.6 to 19.9 (about +1 cpu-s, overlap contention). Gate wall 48.2 to 35.2 s (-27 percent). Gate cpu 96.9 to 95.6 (no rise). In emulated amd64 (`tools/gate-linux.sh`, 8 cpus), c-tests went from 10-13 to 3-4 s, but the gate is bound by controls there (87-116 s). On macOS, c-tests went from 5-6 to 1-2 s, and the gate is bound by controls there too (20-37 s). Expected hosted effect: about -10 s `gate_s`, with `cpu_s` unchanged; the next recorded rows will confirm it.
- **Rejected after a prototype:** starting cp1610-vectors in the background at gate start. It needs no build and its output stayed byte-identical. But it saved only about 3 s (37 35 37 to 39 32 29 s natively, within noise), because it overlaps the core-saturated c-build. It would also need a named special case in the generic step loop and a second job in the EXIT trap. Its `step-time` would become a join wait, and its CPU would be charged to whichever step reaps it. That blurs the per-step profile these levers depend on.
- **Biggest remaining hosted costs:** cold c-build (11-17 s wall natively on 4 cpus) and c-faults (6-8 s, already 4 lanes). The vectors JSON re-parse is still deferred.

## 2026-11-11 — Phase 4 close-out (plan 04-25): D-45 evaluation and hosted budget re-baseline

### Counts

- Phase 4 start: `steps=26/26 controls=231/231 faults=119/119 tests-expected=2144`. Phase 4 end: `steps=33/33 controls=276/276 faults=149/149 tests-expected=2780 tests-executed=2778 tests-skipped=2 tests-missing=0`, with four new suites per preset (`run`, `runner`, `runner-process`, `pack`), the installed-consumer step `pkg-install` and the `important-gates` registry (`gates=26 lines=102`).
- Since the hosted budget was first set at 30 s (Phase 2 end, `controls=190/190`), the gate has grown to 276 controls and 149 faults (the orchestrator's count of the Phase 2 baseline is 192 controls and 67 faults). The 40 s budget was set at the Phase 3 end (`controls=221/221 faults=113/113`).

### Local measurement (laptop, 18 cores, Apple clang 21, other work running; not hosted figures)

| Run | Load (1 min) | `gate-time total=` | `gate-cpu` user + sys | Five largest `step-time` |
|-----|--------------|--------------------|-----------------------|--------------------------|
| sh 1 (before the 04-25 record edits) | 6.1 | 50 | 83.70 + 83.57 | controls 28, c-faults 12, c-tests 7, cp1610-vectors 6, c-purity 5 (pkg-install 4) |
| sh 2 | 8.6 | 45 | 80.96 + 80.40 | controls 27, c-faults 11, c-tests 7, cp1610-vectors 5, c-purity 5 (pkg-install 2) |
| sh 3 | 8.8 | 53 | 84.08 + 82.17 | controls 25, c-faults 11, c-tests 6, cp1610-vectors 5, c-purity 5 (pkg-install 4) |
| dash | 13.2 | 45 | 81.09 + 77.43 | controls 24, c-faults 11, c-tests 6, cp1610-vectors 5, c-purity 5 (pkg-install 3) |

- `gate-env ncpu=18 cc=AppleClang-21.0.0.21000101` on every run; `c-build` 1 s (warm); `important-gates` 0 s.
- Local median (three sh runs): `gate-time total=50`, about 166 cpu-s. Plan 04-01 measured `gate-time total=30` and 82.8 cpu-s (sh, 26 steps). Locally the gate is bound by the overlapped `controls` lane (24-28 s); `c-faults` and `c-tests` follow.

### Budget checkpoints during the phase

- After plan 04-10 the D-45 rule fired (last 3 hosted `gate_s` 69, 60, 54; median 60 s > 40 s; one run > 60 s). The orchestrator inserted 04-LEVER-01 (fewer process starts: exec calls 20988 -> 12203 per gate; large gain on macOS, about 8 percent cpu on native Linux) and 04-LEVER-02 (parallel ctest in `c-tests`: about -10 s wall at equal CPU on native arm64 Linux).
- At the 04-19 checkpoint the rows still read above 40 s; no further lever was inserted (the remaining levers are listed below).

### Hosted rows and the D-45 rule

- `.planning/ci-runs.tsv` holds 23 hosted rows (recorded by the orchestrator with `tools/ci-record.sh`; one failed run, 22 successful). `gate_s` ranges 39-75 s, median 59 s; `cpu_s` (22 rows with a value, 4 cores) ranges 114-225, median 164. `build_s` (`c-build`, cold) 19-34 s.
- Rule result: median of the last 3 `gate_s` (59, 59, 75) = 59 s > 40 s: **fires**. Any run > 60 s: **yes** (75 s, run 43135732132). Median of the last 5 (41, 53, 59, 59, 75) = 59 s > 50 s: **fires**. Both the lever trigger and the revisit trigger have fired.
- What the rows show: near-identical code swings from about 40 s to 67 s between adjacent runs (for example 41 s at 324040001e and 67 s at 2ee9358f, both after the levers, with `c-build` 21 s vs 29 s), a hosted machine-speed spread of about 1.7x. `cpu_s` swings with it (125 vs 211), so the hosted cores themselves differ in speed, not only the waiting. The two levers did move the gate (the four runs right after 04-LEVER-02 read 39-57 s), but a single slow host erases their effect. The step profile of the slow rows is the same as the fast rows, scaled.

### Decision: the 30 s and 40 s hosted wall budgets are superseded (re-baseline)

- **New hosted budget:** median `gate_s` of 75 s, and a median `cpu_s` of 200 as a cost signal that is less sensitive to waiting. **Revisit trigger:** the median of the last 5 `gate_s` above 90 s, or any run above 120 s, or the median `cpu_s` of the last 5 above 200. Today: median `gate_s` 59 s (last 5: 59 s), median `cpu_s` 164 (last 5: 175); inside the new budget.
- **Why a re-baseline and not more levers now.** The budget was set when the gate carried Phase 2 evidence. It now carries Phase 3 and Phase 4 evidence: controls 190 -> 276, faults 113 -> 149 since the 40 s budget (149 fault executables each rerunning its suite), and four new suites in two presets, the installed-consumer build and the important-gates registry. The rows show that wall time is dominated by which hosted machine the job lands on (a 1.7x spread on the same code), not by any one step: a lever that saves 5 s is inside the run-to-run noise. Each remaining lever adds structural risk (background build jobs in the EXIT trap, rewriting the vectors generators, changing how faults rerun suites, or splitting the job, which redefines PKG-02). With the gate at about one minute and no queue pressure, that risk is not worth it now. No check is weakened, dropped or merged; no CI shape change (D-41..D-43 unchanged).
- **Remaining levers, in order, for when the new trigger fires:** (1) background `c-build` for the steps that do not need it, only if `gate-cpu` shows idle cores (today `cpu_s / (ncpu x gate_s)` is about 0.7: 164 / (4 x 59)); (2) batch the cp1610-vectors JSON re-parse (about 5 cpu-s per gate, plus the cold-build generators); (3) cut the per-fault suite reruns in `c-faults` (run only the suites a fault's must-fail ids name; needs the verdict to keep its must-pass evidence); (4) a two-job split, last resort, because it redefines PKG-02 and needs an owner-visible Decision Register entry. Not levers: dropping controls, merging checks, weakening oracles.

### This plan's Linux parity run

- `sh tools/gate-linux.sh` (Ubuntu 24.04 container, x86_64 emulated on arm64, gcc 13.3.0, cmake 3.28.3, dash as /bin/sh), run after the record edits of Task 2 and the AUTONOMY and CI-EFFICIENCY text of Task 3: `gate: PASS steps=33/33 controls=276/276 ... cc=GNU ... faults=149/149 ... gates=26 lines=102 ... consumer-lines=26 pkg-selftest=10/10 ... tests-expected=2780 tests-executed=2778 tests-skipped=2 tests-missing=0`, `gate-time total=199` (emulated; not a hosted figure), last line `gate-linux: exit 0 (log build/gate-linux.log)`.

## 2026-11-12 - Phase 5 checkpoint 1 (plan 05-04, V-16): first measured cost of the PSG suite and faults

All figures marked local are from the laptop (18 cores, Apple clang 21, other work running, 1-minute load 8 to 11); they are not hosted figures. Nothing here calls `gh`; the hosted rows are read from `.planning/ci-runs.tsv`, which the orchestrator records.

### Counts

- Plan 05-04 end: `steps=35/35 controls=300/300 faults=158/158 tests-expected=2968 tests-executed=2966 tests-skipped=2 tests-missing=0` (plan 05-03 end: `controls=296 faults=155 tests-expected=2862`). New: suite `psg` (40 ids per preset), 13 memmap ids (the PSG rows split, one id for the new width case), 3 faults, 4 controls.

### Local gate (`sh tools/gate.sh`, three runs, plan 05-04 final tree)

| Run | Load at start (1 min) | `gate-time total=` | `gate-cpu` user + sys | Five largest `step-time` (seconds) |
|-----|-----------------------|--------------------|-----------------------|-------------------------------------|
| 1 | 8.1 | 32 | 54.89 + 48.83 = 103.7 | controls 13, c-faults 8, c-tests 5, hygiene 3, cp1610-vectors 3 |
| 2 | 11.3 | 32 | 54.39 + 48.93 = 103.3 | controls 13, c-faults 8, c-tests 5, hygiene 3, cp1610-vectors 3 |
| 3 | 11.5 | 35 | 57.75 + 54.89 = 112.6 | controls 15, c-faults 7, c-tests 5, c-purity 4, hygiene 3 |

- `gate-env ncpu=18 cc=AppleClang-21.0.0.21000101`; `c-build` 1 s (warm). `c-faults` 7 to 8 s and `c-tests` 5 s on every run. `dash tools/gate.sh`: `gate-time total=32`.
- The Phase 4 close-out row (same host, a heavier day: gate 45 to 53 s, 80 + 80 cpu-s, `c-faults` 11, `c-tests` 6) is not comparable: the host load differs by more than the effect of nine faults. So the per-fault cost is measured directly, not as a step-time difference.

### Measured unit costs (local)

- Per fault: the `c-faults` step alone (build up to date, 158 fault executables, nothing else) costs 21.7 user + 1.1 sys = 22.8 cpu-s, wall 6.6 s (three runs: 22.8, 23.1, 23.0), so **0.144 cpu-s per fault** on average, inside the 0.15 to 0.25 band of the V-16 estimate and far below the 0.4 threshold. The three PSG faults run in under 0.01 s each (the suite has no sanitizer and the register file is small); their cost is dominated by the verdict's process starts, so they sit below the average.
- Per control: the control runner alone costs 14.7 user + 40.7 sys = 55.4 cpu-s for 300 controls, **0.185 cpu-s per control on macOS** (process-start heavy; V-16 uses about 0.04 on native Linux).
- Per test id of the `psg` suite, from its plain-run time (median of 11): dev (ASan and UBSan) 0.072 s wall, 0.069 cpu-s; release 0.004 s wall, 0.003 cpu-s; 40 ids, so **1.7 ms per id in dev and 0.1 ms in release**, under the 3 ms average assumed in V-16 and far under the plan targets (0.3 s release, 1.5 s dev). For comparison, the machine suite runs 0.52 s dev and 0.27 s release for 397 ids and memmap 0.10 s and 0.01 s.

### Projection against the hosted trigger (V-16)

- Hosted rows read from `.planning/ci-runs.tsv` (successful runs): last 5 `cpu_s` 179, 225, 136, 160, 252, median **179**; last 5 `gate_s` 59, 75, 45, 55, 81, median **59**. Budget since 04-25: median `gate_s` 75 s, median `cpu_s` 200; revisit when the last-5 `gate_s` median is above 90 s, any run above 120 s, or the last-5 `cpu_s` median above 200; the lever trigger of V-16 is a projection of 190 cpu-s.
- Adds not yet in a hosted row: plan 05-04 itself (3 faults, 4 controls, 106 test ids over both presets) and the remaining Phase 5 plans: faults 8 (05-05) + 8 (05-06) + 6 (05-07) + 6 (05-08) + 3 (05-09) + 2 (05-10) = 33, controls 3 + 4 + 5 + 1 + 3 = 16, about 280 more test ids per preset (560 over both), and one more runner suite with two more steps already counted.
- Arithmetic with the V-16 native-Linux unit costs and the measured fault cost: plan 05-04 own adds 3 x 0.15 + 4 x 0.04 + 106 x 0.003 = 0.93; remaining 33 x 0.15 + 16 x 0.04 + 560 x 0.003 + 1.5 (one more suite, two presets) = 4.95 + 0.64 + 1.68 + 1.5 = 8.8; total +9.7, giving **188.7 cpu-s**.
- Cross-check with the laptop figures scaled to the hosted rows (hosted last-5 median 179 over a local gate of about 104 cpu-s is a factor of 1.7): the same adds cost about 9.7 x 1.7 = 16.5, giving **195.5 cpu-s**.
- Wall: the local gate did not move (plan 05-03 end 32 to 34 s, now 32 to 35 s). Projected last-5 hosted `gate_s` median about 59 + 4 to 8 = 63 to 67 s, under the 90 s trigger. Not at risk.
- Result: the projection is **188.7 to 195.5 cpu-s**, straddling the 190 trigger; the hosted spread of the same code (136 to 252 cpu-s in the last five) is larger than the whole projected add, so a single row will not settle it. The rule reaches 190 on the conservative estimate.

### Decision rule (V-16, verbatim)

If the projection or the last-5 median reaches 190 cpu-s, the lever order is (1) batch the `cp1610-vectors` JSON re-parse (about 5 cpu-s, no evidence change), (2) limit fault reruns to the suites a fault names (the verdict keeps its must-pass vector), (3) background `c-build` only if `cpu_s / (ncpu x gate_s)` shows idle cores (about 0.7 before); not levers: dropping a control, merging checks, weakening an oracle, a two-job split (redefines PKG-02). If the measured cost is above 0.4 cpu-s per fault, list the six lowest-value faults of plans 05-05 to 05-08 that can become `psg.judge.*` or `input.judge.*` self-controls.

### Outcome

- The measured cost is 0.144 cpu-s per fault, below 0.4, so no fault is converted to a judge self-control and no list is needed.
- Lever list for the orchestrator to turn into plan 05-L1 if it wants to act on the conservative estimate (the executor does not create 05-L1): (1) batch the `cp1610-vectors` JSON re-parse (about 5 cpu-s, no evidence change; alone it brings the projection to 184 to 191); (2) limit fault reruns to the suites a fault names, only if (1) is not enough; (3) background `c-build` only if `cpu_s / (ncpu x gate_s)` shows idle cores (about 0.7 today: 179 / (4 x 59) = 0.76, so not yet). The next hosted rows (05-04 onward) will show whether the 195 reading or the 189 reading is the better one; a last-5 median at or above 190 fires the rule independently of this projection.

checkpoint-1: lever-required

## 2026-11-12 Phase 5 lever pass 05-L1: where the gate's CPU goes, and a no-action decision

Trigger (read from `.planning/ci-runs.tsv`, successful rows, orchestrator-recorded): last-5 `cpu_s` 225, 136, 160, 252, 263, median **225** (above the 200 trigger); last-5 `gate_s` 75, 45, 55, 81, 85, median 75 (under 90). `cpu_s / (ncpu x gate_s)` for the last two rows: 252 / (4 x 81) = 0.78 and 263 / (4 x 85) = 0.77, so the hosted cores are saturated and total CPU is fixed by the work, not by scheduling (V-16 lever 3 cannot help). Hosted split of the last row: `build_s` 37 of `gate_s` 85. Nothing here calls `gh`.

### Measurement method (no edits to the repository for it)

- Local macOS: `sh tools/gate.sh` plus each step and the control runner run alone with the gate's environment (results dir, nonce, file list) and `times` for children CPU. Warm build, so `c-build` is not representative.
- Linux shape: `sh tools/gate-linux.sh` (Ubuntu 24.04, gcc 13.3.0, cmake 3.28.3, dash, x86_64 under emulation on an arm64 host, 8 cores; a cold build every run), then the same per-step harness run inside the container with the steps one after another (no overlap with the controls). The emulation inflates CPU by about 2x against the hosted runner (container `gate-cpu` 515 to 551 against hosted 263 for nearly the same code), so container figures are used for shares and for sizing a lever, hosted rows for the trigger.

### Ranked cost table (container, cold, steps run alone and in sequence; cpu-s = user + sys, wall-s)

| Rank | Step or phase | cpu-s | wall-s | Share of about 515 | Notes |
|------|---------------|-------|--------|--------------------|-------|
| 1 | `c-build` | 190.1 | 29.5 | 37% | dev build 41.2 (5.9 wall), `ivs_faults` build 114.1 (16.7), release build 40.8 (5.9), two configures 2 |
| 2 | control runner (300 controls, 4 lanes) | 165.8 | 49.8 | 32% | step compute about 125, spawn and tree copy about 41; ten `c-purity` controls that reach the symbol layer cost 4.3 to 4.5 each (about 44), `cp1610-vectors` 39 runs 14.4, `faults-verdict` 17 runs 8.7, `golden` 11 runs 7.2 |
| 3 | `c-faults` (158 runs, 4 lanes) | 43.4 | 14.2 | 8% | 0.27 cpu-s per fault run |
| 4 | `c-tests` (ctest dev and release in parallel) | 32.9 | 11.9 | 6% | 2966 tests, dev under ASan and UBSan |
| 5 | `c-purity` | 13.1 | 12.5 | 3% | compiles every core TU at -O0 and -O2 plus the self-test; serial |
| 6 | `pkg-install` | 7.8 | 7.3 | 1.5% | |
| 7 | `hygiene` | 7.1 | 5.6 | 1.4% | |
| 8 | `cp1610-vectors` | 4.7 | 4.7 | 0.9% | one cmake script, pure JSON string work |
| 9 | `faults-verdict` | 4.1 | 3.1 | 0.8% | |
| 10 | `tests-inventory` | 3.9 | 3.7 | 0.8% | |
| - | the other 25 steps | about 17 | about 22 | 3% | each under 3.1 cpu-s |

- Container gate runs: `gate-time total=131` and `123` (two runs), `gate-cpu user=517.01 sys=33.60` and `user=486.22 sys=28.87`; `step-time` controls 79 and 73 (overlapped), `c-build` 38 and 36, `c-faults` 18 and 17, `c-purity` 16 and 14, `c-tests` 13 and 12. The wall is the serial step chain; the controls overlap it.
- Local macOS (18 cores, load 7 to 9, warm build): `sh tools/gate.sh` `gate-time total=31`, `gate-cpu user=53.45 sys=48.05` (101.5). Alone: controls 42.6 cpu-s (15.8 wall), `c-faults` 21.9 (6.3), `c-tests` 9.5 (4.1), `hygiene-history` 3.9, `hygiene` 2.9, `cp1610-vectors` 2.9, `c-purity` 2.0, `pkg-install` 1.9, the rest under 1 each. The laptop cannot see the dominant hosted cost, because its `c-build` is warm (0.7 cpu-s).
- Unit costs (container): per fault, build 114.1 / 158 = **0.72 cpu-s** (an owner TU recompiled at -O1 with the warning set, about 0.61 wall on average, plus a link of about 0.19; the ninja log shows 157 owner-object compiles for 158 faults) and run 43.4 / 158 = 0.27 cpu-s. A front end only compile (`-fsyntax-only`) of a core TU is 0.08 s against 0.36 s at -O1, so the compile is optimizer time, not warning time (`-w` saves 0.01 s).
- Finding on the V-16 projection: the 0.144 cpu-s per fault of checkpoint 1 counted the fault run only. The fault build is 2.7x the run cost, so a fault costs about (0.72 + 0.27) x 0.51 = **0.5 cpu-s hosted** (0.51 is the hosted over container CPU ratio), not 0.15. The 33 faults still planned (05-05 to 05-10) therefore add about 16 cpu-s hosted, not the 5 assumed, and the last-5 median is expected to settle around 240 to 260 by the phase end rather than 190.

### Levers considered

| Lever | Measured saving (container cpu-s) | Evidence change | Decision |
|-------|-----------------------------------|-----------------|----------|
| V-16 (1) batch the `cp1610-vectors` JSON re-parse | about 5: the whole step is 4.7 alone and 14.4 over 39 controls, most of the control share is fixed (cmake start, blind projection, tree copy); halving the script cost saves about 2 + 3 | none intended, but it is a rewrite of a 1328-line checker guarded by 40 controls | **rejected**: under the 15 cpu-s bar by a factor of 3, and a rewrite risk out of proportion |
| V-16 (2) limit fault reruns to the suites a fault names | 0 | would weaken | **rejected, already true**: `c-faults` runs each fault once and the executable runs only the suite its ids file names (`# suite=<name>`); there is no full-suite rerun per fault. Running fewer vectors per fault would break the verdict's exact `list-mismatch` check, so it cannot be done without losing evidence |
| V-16 (3) background `c-build` | 0 cpu (wall only) | none | **rejected**: cores are saturated (0.77), total CPU is fixed |
| `-g0` on the fault objects and executables only (tests/faults.cmake) | about 10 (removing `-g` from the whole dev preset measured 13.9: dev 41.2 to 37.6, faults 114.1 to 103.8) | none (codegen is unchanged; fault binaries are never debugged) | **rejected for now**: 10 cpu-s is about 5 hosted (2%) against a hosted spread of 136 to 263 for the same code, under the 15 bar; keep it as the first item of a later pass |
| fault objects at -O0 instead of -O1 | about 35 to 40 (a compile drops from 0.36 to 0.22 s) | the fault executables would no longer be built with the dev flags the suites are proven with; a fault could in principle pass at -O0 and fail at -O1 | **rejected**: changes the evidence build; it is an owner decision, not a lever |
| fewer controls, controls without the `c-purity` self-test, shared compile cache between control runs | 40 to 50 | would weaken the control or the step it proves | **rejected** (listed in the D-45 rule as not levers) |
| ccache | not measured | none | banned by the D-45 rule while the gate is within the wall budget, and a hosted cache needs an action step and a cache key (a CI change, AGENTS.md: measure first) |

### Decision

No lever clears the bar (at least about 15 cpu-s on the container shape with the evidence unchanged and a simple, safe change), so **nothing was applied and no tracked file other than this entry changed**. The cost is structural: 37% is the cold build and its 157 per-fault owner-TU compiles, 32% is the 300 controls, and both grow linearly with the faults and controls the phase still has to add. The gate wall is far from its trigger (median 75 s against 90 s), the hosted cores are saturated, and the CPU figure is not billed time; it is the trigger the D-45 rule uses to ask whether a cheaper design exists, and the measurement says the only cheap one (`-g0`, 10 cpu-s) is below the bar.

Recommended follow-ups, for the orchestrator and the owner (not done here):

- Re-baseline the hosted cpu budget with the corrected per-fault unit cost: median `cpu_s` 200 is already exceeded by the current code (225), and the 33 planned faults add about 16 and the 16 planned controls about 6 hosted; a budget near 260 and a trigger near 280 describe the phase end, the wall budget (median `gate_s` 75, trigger 90) stays as it is. Updating V-16's per-fault figure from 0.15 to about 0.5 hosted cpu-s keeps later projections honest.
- If a lever is still wanted before the phase end, in this order: `-g0` for the fault targets (about 5 hosted cpu-s, no evidence change), then a decision by the owner on whether the fault executables must be built at the dev flags (the -O0 option above, about 19 hosted cpu-s, changes the evidence build). Wider changes (ccache, a second job) stay banned or last-resort per the D-45 rule.

### Before and after

- Before: counts `steps=35/35 controls=300/300 faults=158/158 tests-expected=2968 tests-executed=2966 tests-skipped=2 tests-missing=0`; local `gate-time total=31`, `gate-cpu` 101.5; container `gate-time total=131`, 550.6 cpu-s.
- After (this entry is the only change to the tree): `sh tools/gate.sh` `gate-time total=31`, `gate-cpu user=53.68 sys=45.80` (99.5); `dash tools/gate.sh` `gate-time total=33`, `gate-cpu user=54.55 sys=49.75`. The `gate:` summary line of the sh run before and the sh and dash runs after is byte-identical (same md5): `steps=35/35 controls=300/300 faults=158/158 tests-expected=2968 tests-executed=2966 tests-skipped=2 tests-missing=0`. Container (`sh tools/gate-linux.sh`): exit 0, last log line `gate: PASS steps=35/35 controls=300/300 ... faults=158/158 ... tests-expected=2968 tests-executed=2966 tests-skipped=2 tests-missing=0`, `gate-time total=124`, `gate-cpu user=481.83 sys=29.08` (510.9); its summary line is byte-identical to the before run (same md5). CPU and wall are unchanged within the host noise, as expected for a no-action pass.

## 2026-11-12 Phase 5 budget decision after lever pass 05-L1 (orchestrator)

- Trigger fired (last-5 hosted `cpu_s` median 225 over 200); wall did not (`gate_s` median 75, trigger 90). The lever pass (above) measured and applied nothing: V-16 (1) saves about a third of the bar for a risky rewrite, V-16 (2) already holds, V-16 (3) cannot help at cpu/(ncpu x gate_s) of 0.77.
- Correction to V-16: a fault costs about 0.5 hosted cpu-s (build 0.72 container cpu-s plus run 0.27), not 0.15. The 33 faults and 16 controls still planned add about 16 to 25 hosted cpu-s, so the last-5 median is expected to settle near 250 by the end of Phase 5.
- **Decision (delegated under AUTONOMY section 1; policy, reversible):** re-baseline the hosted `cpu_s` budget to a median of 260 with a revisit trigger at a last-5 median above 280. The wall budget is unchanged (median `gate_s` 75, trigger a last-5 median above 90 or any run above 120). No evidence is dropped, no check is moved out of the gate.
- First candidate for the next lever pass if either trigger fires: `-g0` on the fault targets only (about 5 hosted cpu-s, no evidence change). Fault objects at `-O0` (about 37 container cpu-s) changes the evidence build and is an owner decision.

## 2026-11-12 Phase 5 checkpoint 2 (plan 05-08, V-16): cost of the write log and the projection to the phase end

All figures marked local are from the laptop (18 cores, Apple clang 21, other work running, 1-minute load 8 to 11); they are not hosted figures. Nothing here calls `gh`; the hosted rows are read from `.planning/ci-runs.tsv`, which the orchestrator records. Triggers in force since the 05-L1 decision: hosted `cpu_s` budget median 260, revisit when the last-5 median is above 280; hosted wall budget median `gate_s` 75, revisit when the last-5 median is above 90 or any run above 120. (The text of plan 05-08 still quotes the older 190 and 200 figures; the later decision supersedes it.)

### Counts

- Plan 05-08 end: `steps=35/35 controls=303/303 faults=186/186 tests-expected=3610 tests-executed=3608 tests-skipped=2 tests-missing=0` (plan 05-07 end: `faults=180/180 tests-expected=3538`). New: 36 ids in suite `psg` (191 to 227 per preset, 72 over both presets), six named faults, no control.

### Local gate (`sh tools/gate.sh`, three runs, plan 05-08 final tree)

| Run | Load at start (1 min) | `gate-time total=` | `gate-cpu` user + sys | Five largest `step-time` (seconds) |
|-----|-----------------------|--------------------|-----------------------|-------------------------------------|
| 1 | 7.9 | 37 | 66.81 + 52.19 = 119.0 | controls 14, c-faults 11, c-tests 5, hygiene 4, c-purity 3 |
| 2 | 11.1 | 35 | 60.63 + 46.58 = 107.2 | controls 12, c-faults 10, c-tests 5, hygiene 3, cp1610-vectors 3 |
| 3 | 9.6 | 37 | 63.23 + 49.65 = 112.9 | controls 13, c-faults 10, c-tests 5, hygiene 4, cp1610-vectors 3 |

- `gate-env ncpu=18 cc=AppleClang-21.0.0.21000101`; `c-build` 1 s (warm). `dash tools/gate.sh`: `gate-time total=41`, 75.3 + 60.0 = 135.3 cpu-s (a run in the middle of other work). Checkpoint 1 (plan 05-04): 32, 32, 35 s and 103.7, 103.3, 112.6 cpu-s; plan 05-07 final `sh`: 39 s and 128.7 cpu-s on a heavier day. The spread between runs on this host (107 to 135 cpu-s for the same tree) is larger than any effect of this plan, so the adds are measured directly below.
- Container (`sh tools/gate-linux.sh`, Ubuntu 24.04, gcc 13.3.0, emulated amd64, 8 cores; a correctness check, not a hosted figure): `gate-time total=151`, `gate-cpu user=629.53 sys=34.66`.

### Measured unit costs (local)

- Per fault (run only, the fault executables run alone and serially, build up to date): 186 executables cost 25.5 cpu-s, wall 26.3 s, so **0.137 cpu-s per fault run** (checkpoint 1: 0.144). The increase of the fault runs since the Phase 4 close-out (149 faults) is the 37 added faults x 0.137 = **5.1 cpu-s local**, spread over four lanes (the `c-faults` step reads 10 to 11 s now against 11 to 12 s at the close-out; the difference is inside the load noise). Add the build of a fault, measured in the 05-L1 pass at about 0.72 container cpu-s (an owner TU recompiled at -O1, a link) and scaled with the hosted over container ratio 0.51: a fault costs about **0.5 hosted cpu-s** (the corrected unit of 05-L1), so the 37 faults since the close-out account for about 18 hosted cpu-s of the rise of the hosted median from 164 (close-out) to 252 (now); the rest is the 27 added controls, the new suites and test ids, and the spread between hosted machines.
- Per control: unchanged since checkpoint 1 (303 controls, none added by this plan). On macOS 0.185 cpu-s per control, about 0.04 on native Linux.
- Per test id, from plain-run times (median of 11): `psg` 227 ids: dev (ASan and UBSan) 0.344 s wall, 0.338 cpu-s (**1.5 ms per id**), release 0.120 s, 0.119 cpu-s (0.5 ms per id); against 0.185 and 0.054 s for 191 ids at plan 05-07. The 36 new ids cost 0.16 s dev and 0.07 s release: the exhaustive write-gap probe over every decle at waits 0, 1 and 35 (about 0.1 s dev) and the eight seeded fuzz cases (0.05 s dev) are most of it. The suite stays under its plan targets (0.3 s release, 1.5 s dev). `run` 159 ids: 0.565 s dev, 0.306 s release (3.5 ms and 1.9 ms per id); `input` 123 ids: 0.104 and 0.022 s; `machine` 397 ids: 0.514 and 0.276 s.
- The five largest local steps are the same five as at checkpoint 1 (controls, `c-faults`, `c-tests`, hygiene, `c-purity` or `cp1610-vectors`).

### Projection against the hosted trigger (V-16)

- Hosted rows from `.planning/ci-runs.tsv` (the last five, all successful, the newest at plan 05-07 `6f3001e`): `cpu_s` 252, 263, 273, 220, 231, median **252**; `gate_s` 81, 85, 89, 72, 78, median **81** (none of the five includes this plan).
- Adds not yet in a hosted row, at hosted unit costs: this plan, 6 faults x 0.5 + 72 test ids x 3 ms x 1.7 (hosted scaling) + the heavier probe = 3.0 + 0.4 = **3.4 cpu-s**. Remaining plans: faults 3 (05-09) + 2 (05-10) = 5 x 0.5 = 2.5; controls 4 + 5 + 1 + 3 = 13 x 0.04 = 0.5; runner and process test ids of plans 05-09 to 05-11 (about 55 ids per preset, 110 over both presets, x 3 ms = 0.33) and about 40 process spawns (0.6); one installed-consumer scenario and one more golden pair (about 1.5): **5.4 cpu-s**. Total adds **8.8 cpu-s**.
- Projected last-5 `cpu_s` median at the phase end: 252 + 8.8 = **260.8**; with a 20% margin on the adds, 262.6. That is at the 260 budget line and **19 cpu-s under the 280 trigger**; the hosted spread of one tree (220 to 273 in the last five) is larger than the whole projected add, so a single row cannot settle it, but the median needs a +28 move to fire.
- Projected `gate_s`: the hosted wall follows the cpu figure within the run-to-run spread, +3.5 percent gives 81 x 1.035 = **84**. The median is above the 75 budget (81 now) and the margin to the 90 trigger is about 6 s; the newest five include one 89 s row. It is not at the trigger.
- Result: neither trigger is projected to fire.

### Decision rule (V-16, verbatim)

If the projection or the last-5 median reaches the trigger the lever order is (1) batch the `cp1610-vectors` JSON re-parse (about 5 cpu-s, no evidence change), (2) limit fault reruns to the suites a fault names (the verdict keeps its must-pass vector), (3) background `c-build` only if `cpu_s / (ncpu x gate_s)` shows idle cores (252 / (4 x 81) = 0.78 now: none); not levers: dropping a control, merging checks, weakening an oracle, a two-job split (redefines PKG-02). The 05-L1 pass already found (2) to be a property of the gate and (1) below the bar; the first candidate it named for a future pass is `-g0` on the fault targets only (about 5 hosted cpu-s, no evidence change), then an owner decision on the fault executables' optimisation level.

### Outcome

- The measured cost is 0.137 cpu-s per fault run, far below 0.4; no fault is converted to a judge self-control. The write-log suite adds 0.16 s dev to a plain run of `psg`, inside its targets.
- **No lever pass is required before plan 05-09.** Watch items for the orchestrator, from the next hosted rows (recorded after this plan's push): a last-5 `cpu_s` median at or above 280 or a last-5 `gate_s` median at or above 90 fires the rule; the first lever to apply is `-g0` on the fault targets (tests/faults.cmake), the next an owner decision as above. No plan 05-L2 is created here.

checkpoint-2: no-lever

## 2026-11-12 - Phase 5 close-out (plan 05-12): D-45 and V-16 evaluation

All figures marked local are from the laptop (18 cores, Apple clang 21, other work running); they are not hosted figures. Nothing here calls `gh`: the hosted rows are those the orchestrator recorded in `.planning/ci-runs.tsv`.

### Counts

- Phase 5 start (Phase 4 end): `steps=33/33 controls=277/277 faults=149/149 tests-expected=2816`. Phase 5 end: `steps=35/35 controls=316/316 faults=191/191 tests-expected=3772 tests-executed=3770 tests-skipped=2 tests-missing=0`, important-gate registry `gates=48 lines=175` (Phase 4 end: `gates=26 lines=102`), `d10-items=32`.
- What grew: two steps (`input`, `psg-vectors`), 39 controls (277 to 316; plan 05-12 added 3 of them), 42 faults (149 to 191), three suites (`input` and `psg` new, `run`, `runner` and `machine` grown) plus the reference-converter cases in `runner`, the installed-consumer replay (`consumer-lines` 26 to 43, `pkg-selftest` 10 to 13) and seven goldens.

### Local measurement (`sh tools/gate.sh`, three runs on the final tree)

| Run | Load at start (1 min) | `gate-time total=` | `gate-cpu` user + sys | Five largest `step-time` (seconds) |
|-----|-----------------------|--------------------|-----------------------|-------------------------------------|
| 1 | 8.2 | 77 | 126.2 + 122.9 = 249.1 | controls 39, c-faults 24, c-tests 13, c-purity 8, hygiene 5 |
| 2 | 12.3 | 79 | 127.1 + 115.2 = 242.4 | controls 33, c-faults 23, c-tests 15, c-purity 7, cp1610-vectors 6 |
| 3 | 11.9 | 97 | 134.1 + 134.5 = 268.6 | controls 52, c-faults 29, c-tests 17, c-purity 8, hygiene 7 |

- These local figures are about twice those of plan 05-11 (`gate-time total=41`, 126 cpu-s) for a tree that differs only by three controls and a registry. The host explains it: the 1-minute load was 8 to 15 with other projects' builds and a container gate running, and a reference run of the unchanged suite `psg` (227 ids, dev preset) took 0.80 s wall now against 0.34 s at plan 05-08 (a factor of 2.3). Plan 05-12 itself adds three controls (about 0.04 cpu-s each on native Linux) and a step that reads a few small files; there is no measurable cost to attribute to it. The local figures are therefore not a basis for a budget decision; the hosted rows are.
- `c-build` was warm (1 to 4 s); `important-gates` 0 s.

### Hosted rows and the rule

- `.planning/ci-runs.tsv` holds 35 hosted rows, 12 of them since the Phase 4 close-out (one failed run, 43148343848, a 109 s run with `test_s` 84; 11 successful). Since the Phase 4 close: `gate_s` 45 to 109 s (successful runs 45 to 106 s), `cpu_s` 136 to 336 with a median of 226.
- Last five rows (runs at plans 05-08 to 05-11: 43202448987, 43209453151, 43214813968, 43220350877, 43227486537): `gate_s` 78, 58, 103, 106, 72, median **78**; `cpu_s` 231, 173, 323, 336, 221, median **231**; `cpu_s / (4 x gate_s)` 0.74 to 0.79 (cores saturated). Two outlier runs (103 s and 106 s, 323 and 336 cpu-s) sit between runs at 58 s and 72 s on trees that were larger, so they read as host speed (the Phase 4 close-out already measured a 1.7x spread on identical code), not as code cost; this is an inference from adjacent rows, not a measurement of the hosts.
- Rule in force (the 05-L1 decision of the orchestrator re-baselined the budgets on 2026-11-12): hosted `cpu_s` budget median 260 and revisit when the last-5 median is above 280; hosted wall budget median `gate_s` 75 and revisit when the last-5 median is above 90 s or any run above 120 s. Result: last-5 `gate_s` median 78 (under 90), maximum since the Phase 4 close 109 s (under 120), last-5 `cpu_s` median 231 (under 280). **The rule does not fire.** Read against the older figures that plan 05-12 quotes (revisit above 200 cpu-s, lever trigger at a projection of 190), the cpu median of 231 is above them; that line is superseded by the 05-L1 decision and is listed here so the reader can see both.
- Projection: the last hosted row (72 s, 221 cpu-s) is the tree at the end of plan 05-11. Plan 05-12 adds three controls and registry lines: at most 0.2 hosted cpu-s. The last-5 median cannot move by more than the run-to-run spread (136 to 336 for similar code), so no figure here supports a lever. The orchestrator re-evaluates with the first hosted rows after this plan's push.

### Checkpoints and levers

- Checkpoint 1 (plan 05-04): `checkpoint-1: lever-required` on the conservative projection (188.7 to 195.5 against a 190 trigger). The measuring pass 05-L1 (recorded above) applied nothing (no lever cleared the bar of about 15 container cpu-s with the evidence unchanged), and the orchestrator re-baselined the hosted cpu budget to median 260 and trigger 280 (see the entries above).
- Checkpoint 2 (plan 05-08): `checkpoint-2: no-lever` (projected last-5 median 260.8 at the phase end, against the 280 trigger).
- Plan 05-L1 exists as the measuring pass recorded above (created outside this plan); no lever ran. Plan 05-12 creates no plan and calls no tool that reads the hosted API.
- Lever order if the rule fires later: (1) batch the `cp1610-vectors` JSON re-parse (about 5 cpu-s, no evidence change; judged under the bar by 05-L1), (2) limit fault reruns to the suites a fault names (already true: each fault runs once and only its suite), (3) background `c-build` only if `cpu_s / (ncpu x gate_s)` shows idle cores (0.74 to 0.79 now: none). The first candidate named by 05-L1 for a future pass is `-g0` on the fault targets only (about 5 hosted cpu-s, no evidence change). Not levers: dropping a control, merging checks, weakening an oracle, a two-job split (it redefines PKG-02).
- Gate robustness found in this plan (not a cost item): the watchdog arming of `c-faults` (the FIFO read added in `8225f4e`) could strand a lane forever under macOS `sh` when a child exit interrupted the blocking open after the writer was released (one hang of 10 minutes at no CPU, then killed). The parent now holds the FIFO read-write on one descriptor and retries the read there (commit `583d3c6`); 1,200 arm and release cycles per shell cost the same as before and ran clean under `sh`, `dash` and `bash`.

phase5-close: no-lever

## 2026-11-12 Phase 5 trigger review after the close-out push (orchestrator)

- Hosted rows since plan 05-08 (gate_s / cpu_s): 58 / 173 (05-08), 103 / 323 (05-09), 106 / 336 (05-10), 72 / 221 (05-11), 115 / 363 (05-12 head). Last-5 medians now **gate 103 s and cpu 323**, both over the triggers (90 and 280): **both fire**. One run (115 s) is close to the 120 s any-run trigger.
- Per-step comparison of the fastest recent run (43209453151, gate 58 s) with the slowest (43237288204, gate 115 s): `c-build` 25 -> 49, `controls` 22 -> 43, `c-faults` 15 -> 37, `c-tests` 4 -> 10, `cp1610-vectors` 4 -> 7. `controls / c-build` is 0.88 in both runs and every large step roughly doubled, so the swing is runner speed (the 1.7x host variance recorded since Phase 4), not a code change; the same-day pair 72 s (05-11) and 115 s (05-12) differs by planning documents and two shell-test fixes only. `c-faults / c-build` moved from 0.60 to 0.76; the 05-12 head carries five more faults and the armed-watchdog handshake, a candidate small real growth of a few seconds.
- **Decision (reversible):** no lever applied now, because a speed-normalised comparison shows no growth that a lever would remove and the cheapest candidate (`-g0` on the fault targets, about 5 hosted cpu-s) is below the bar. The budget stays at median gate 75 s, cpu 260. **Next step:** the first hosted rows of Phase 6 decide; if the last-5 medians are still over 90 s / 280 cpu-s after rows from at least three different runner speeds, apply `-g0` on the fault targets first, then the V-16 ordering, and consider moving the cold-build `ivs_faults` target (114 of about 515 container cpu-s) to a cached or prebuilt arrangement, which needs an owner decision because D12 bans caches.

## 2026-11-12 Phase 6 start (plan 06-02): `-g0` on the fault targets, the speed-normalised metric, the checkpoints

Nothing here calls `gh` or `tools/ci-record.sh`; `.planning/ci-runs.tsv` is orchestrator-owned and untouched. Local figures are from the laptop (18 cores, Apple clang 21, other work running, 1-minute load 9 to 22); container figures are `sh tools/gate-linux.sh` (Ubuntu 24.04, gcc 13.3.0, emulated amd64 on an arm64 host, 8 cores, cold build); neither is a hosted figure. Counts at plan start and end: `steps=36/36 controls=328/328 faults=193/193 tests-expected=3778 tests-executed=3776 tests-skipped=2 tests-missing=0`.

### Why the raw triggers fire by runner speed, and the normalised metric

The last-5 hosted medians through run 43246638522 were gate 106 s and cpu 336 against the triggers 90 and 280, and after the later rows (through 43286006804) they are gate 90 s and cpu 267. Raw seconds and cpu-seconds move with the speed of the runner that happened to pick the job (the 1.7x spread recorded since Phase 4), so the pair is read together with the speed-normalised ratio **`r = cpu_s / build_s`** of the same row: `build_s` is the hosted compile time of the same job on the same machine, so it measures the machine, and `cpu_s` divided by it is the work expressed in "machine-build-seconds". Over the rows of the last-5 set the raw cpu moves between 219 and 363 while `r` stays between 6.4 and 7.4. Every row of `.planning/ci-runs.tsv` since Phase 5 started (2026-11-11 and later, `cpu_s` and `build_s` above 0; 38 rows) follows (`gate_s` is the hosted gate step wall):

| Date | Run | Conclusion | `build_s` | `gate_s` | `cpu_s` | `r` |
|------|-----|------------|-----------|----------|---------|-----|
| 2026-11-11 | 43058596162 | success | 28 | 62 | 164 | 5.86 |
| 2026-11-11 | 43060935649 | success | 30 | 62 | 164 | 5.47 |
| 2026-11-11 | 43063741813 | success | 28 | 61 | 164 | 5.86 |
| 2026-11-11 | 43067196274 | success | 22 | 46 | 116 | 5.27 |
| 2026-11-11 | 43070516137 | success | 25 | 54 | 147 | 5.88 |
| 2026-11-11 | 43073575336 | success | 32 | 69 | 177 | 5.53 |
| 2026-11-11 | 43075005155 | success | 27 | 60 | 170 | 6.30 |
| 2026-11-11 | 43077768591 | success | 24 | 55 | 136 | 5.67 |
| 2026-11-11 | 43094232714 | success | 29 | 62 | 179 | 6.17 |
| 2026-11-11 | 43099862529 | success | 33 | 69 | 185 | 5.61 |
| 2026-11-11 | 43102991214 | success | 29 | 64 | 182 | 6.28 |
| 2026-11-11 | 43106276178 | success | 25 | 55 | 154 | 6.16 |
| 2026-11-11 | 43113503262 | success | 24 | 52 | 150 | 6.25 |
| 2026-11-11 | 43115430207 | success | 19 | 39 | 114 | 6.00 |
| 2026-11-11 | 43117857552 | success | 20 | 40 | 118 | 5.90 |
| 2026-11-11 | 43121804870 | success | 27 | 57 | 161 | 5.96 |
| 2026-11-12 | 43124846207 | success | 29 | 67 | 211 | 7.28 |
| 2026-11-12 | 43127157643 | success | 21 | 41 | 125 | 5.95 |
| 2026-11-12 | 43129448676 | success | 26 | 53 | 148 | 5.69 |
| 2026-11-12 | 43131258957 | success | 27 | 59 | 175 | 6.48 |
| 2026-11-12 | 43133660327 | success | 28 | 59 | 179 | 6.39 |
| 2026-11-12 | 43135732132 | success | 34 | 75 | 225 | 6.62 |
| 2026-11-12 | 43148343848 | failure | 25 | 109 | 150 | 6.00 |
| 2026-11-12 | 43152062835 | success | 21 | 45 | 136 | 6.48 |
| 2026-11-12 | 43175636749 | success | 25 | 55 | 160 | 6.40 |
| 2026-11-12 | 43178237404 | success | 36 | 81 | 252 | 7.00 |
| 2026-11-12 | 43182282322 | success | 37 | 85 | 263 | 7.11 |
| 2026-11-12 | 43193144083 | success | 39 | 89 | 273 | 7.00 |
| 2026-11-12 | 43198505852 | success | 30 | 72 | 220 | 7.33 |
| 2026-11-12 | 43202448987 | success | 33 | 78 | 231 | 7.00 |
| 2026-11-12 | 43209453151 | success | 25 | 58 | 173 | 6.92 |
| 2026-11-12 | 43214813968 | success | 45 | 103 | 323 | 7.18 |
| 2026-11-12 | 43220350877 | success | 47 | 106 | 336 | 7.15 |
| 2026-11-12 | 43227486537 | success | 33 | 72 | 221 | 6.70 |
| 2026-11-12 | 43237288204 | success | 49 | 115 | 363 | 7.41 |
| 2026-11-12 | 43246638522 | success | 50 | 114 | 338 | 6.76 |
| 2026-11-12 | 43248899132 | success | 34 | 75 | 219 | 6.44 |
| 2026-11-12 | 43286006804 | success | 36 | 90 | 267 | 7.42 |

Summary of the table: minimum 5.27, median 6.30, maximum 7.42. The Phase 4 rows (2026-11-11) sit at 5.3 to 6.3 and the Phase 5 close rows at 6.4 to 7.4: `r` rose by about 1 over Phase 5, which is the real growth of the work (the 42 faults, 39 controls and three suites added) beyond machine speed. The last five rows (43227486537 to 43286006804) give `r` 6.70, 7.41, 6.76, 6.44, 7.42 (median 6.76) against raw gate medians 90 s and cpu 267. A fast runner and a slow runner differ by 1.7x on the same code in the raw columns and by under 0.2 in `r` (rows 43209453151 at 58 s and 43214813968 at 103 s: 6.92 and 7.18).

### The rule for Phase 6

- The raw owner triggers stay as the 05-L1 decision set them (copied, not changed): hosted gate median budget 75 s, revisit when the last-5 median is above 90 s or any run is above 120 s; hosted cpu median budget 260, revisit when the last-5 median is above 280.
- The pair is read together with `r`: budget 7.8, revisit above 8.3 (last-5 median of successful rows). A raw trigger that fires while `r` is inside 7.8 is a runner-speed event (record the rows and keep going); a raw trigger and `r` above 8.3 together is code growth and starts the lever order below.
- The orchestrator owns the budgets (see the request at the end of this entry).

### Measured unit costs (carried into the projections)

- Fault: 0.137 local cpu-s per run (checkpoint 2 of Phase 5), about 0.5 hosted cpu-s with its build (the corrected unit of 05-L1: build 0.72 plus run 0.27 container cpu-s, times the hosted over container ratio 0.51; `-g0` below lowers the build share).
- Control: 0.185 local cpu-s; about 0.07 hosted for a pure step, about 2.2 hosted for one that reaches the C symbol layer (ten `c-purity` controls cost 4.3 to 4.5 container cpu-s each).
- Test id: 1.5 to 3.5 ms per id and preset (`psg` 1.5 ms dev, `run` 3.5 ms dev, measured in Phase 5 checkpoint 2), release about a third of that.
- Hosted speed varies about 1.7x between runners on identical code (the reason for `r`).

### Lever applied: `-g0` on the fault targets only

`tests/faults.cmake` gains `ivs_fault_nodebug(<target>)` (a `target_compile_options(... PRIVATE -g0)` after the preset's `-O1 -g`, where a later `-g0` wins on clang and gcc), called from `ivs_fault_tu_setup` (plain and owner object libraries), for `ivs_fault_plain_harness`, in `ivs_fault_suite_obj`, and for the fault executables and owner objects of `ivs_fault_exe` and `ivs_fault_exe_core_gen` (site, generator and core-generator faults). Scope check (`ninja -t commands`): `ivs_fault_ADCR_IGNORES_C` 24 compile lines with ` -g0`, a runner-owner fault (`OUTCOME_GUEST_AS_HARNESS`) 36, a sha256 fault 33, a generator fault 29; `ivs_tests_psg` in `build/dev` and in `build/release` none; the whole `build/release` graph none; no `-g0` command in `build/dev` outside targets named for faults. The pristine suites, the runner, the sanitized dev objects and every release target are unchanged, so no evidence build changed; code generation of the fault objects is unchanged (only debug information is dropped).

### Before and after

Method: three local and two container full gates alternated between the base and the patched `tests/faults.cmake` on the same host (the file was swapped between runs, so each run's `c-build` includes the rebuild of the changed fault objects), then a paired cold build of the `ivs_faults` target alone, because the whole-gate figures are dominated by host load (load 9 to 22) and cannot show a saving of this size.

| Run | Variant | Load at start (1 min) | `gate-time total=` | `gate-cpu` user + sys | `c-faults` | `c-build` | `controls` | `c-tests` |
|-----|---------|-----------------------|--------------------|-----------------------|------------|-----------|------------|-----------|
| local 1 | base | 15.3 | 82 | 113.10 + 110.82 = 223.9 | 22 | 2 | 40 | 15 |
| local 1 | `-g0` | 19.4 | 77 | 138.48 + 123.51 = 262.0 | 19 | 7 | 38 | 13 |
| local 2 | base | 20.7 | 69 | 133.23 + 117.30 = 250.5 | 18 | 7 | 29 | 12 |
| local 2 | `-g0` | 17.6 | 78 | 132.79 + 117.31 = 250.1 | 17 | 6 | 27 | 12 |
| local 3 | base | 19.4 | 78 | 138.28 + 126.75 = 265.0 | 19 | 10 | 36 | 13 |
| local 3 | `-g0` | 17.5 | 80 | 136.16 + 128.83 = 265.0 | 19 | 10 | 41 | 15 |
| container 1 | base | 21.9 | 213 | 860.83 + 61.47 = 922.3 | 36 | 68 | 118 | 24 |
| container 1 | `-g0` | 9.5 | 225 | 881.03 + 67.50 = 948.5 | 33 | 73 | 121 | 29 |
| container 2 | base | 11.2 | 232 | 910.36 + 66.22 = 976.6 | 37 | 73 | 124 | 24 |
| container 2 | `-g0` | 12.2 | 222 | 835.14 + 61.63 = 896.8 | 35 | 63 | 112 | 26 |

- Whole gate: local `gate-cpu` base 223.9, 250.5, 265.0 (mean 246.5) against `-g0` 262.0, 250.1, 265.0 (mean 259.0); container base 922.3 and 976.6 (mean 949.4) against 948.5 and 896.8 (mean 922.7, minus 27 or 2.8 percent). Every difference is inside the spread between runs of the same variant (the loaded laptop moves the same tree by 40 cpu-s), so **the whole-gate measurement cannot resolve the saving; it is below noise there, as the 05-L1 estimate (about 10 container cpu-s of about 515) predicted.** The `c-faults` step (the fault runs, which do not change) reads 22, 18, 19 s against 19, 17, 19 locally and 36, 37 against 33, 35 in the container: the same noise.
- Paired cold build of `ivs_faults` (the only place the lever acts; separate source copies and build directories per variant so no reconfigure mixes them; three rounds alternated, `-j 8`, `times` user + sys): Ubuntu 24.04 container, base 250.4, 274.7, 229.6 cpu-s (mean 251.6, wall 34, 38, 32 s) against `-g0` 229.7, 223.3, 194.9 (mean 216.0, wall 32, 32, 27 s): **minus 35.6 container cpu-s (14 percent) and about 4 s wall; each pair favours `-g0` (minus 20.7, 51.4, 34.7)**. Fault object bytes drop from 16.8 MB to 5.1 MB. Scaled by the hosted over container ratio 0.51 that is about **18 hosted cpu-s** (the pairs allow 10 to 26), about 3 times the 05-L1 estimate (which extrapolated from removing `-g` from the whole dev preset on a loaded host). On macOS (Apple clang, two rounds) the same pair reads 42.8, 52.0 cpu-s against 44.5, 47.8: no measurable difference (object bytes 10.7 MB to 4.4 MB), so the lever helps the Linux gcc build the hosted runner uses and not the laptop.
- Evidence: the `gate:` summary line is byte-identical before and after: the six local runs share one md5, the four container runs share another (counts `steps=36/36 controls=328/328 faults=193/193 tests-expected=3778 tests-executed=3776 tests-skipped=2 tests-missing=0`), and every fault still ends with its registered verdict (the `c-faults` step and `faults-verdict` are the judges). The lever is applied because it changes no evidence and saves real hosted cpu, not because a trigger fired.

- Final tree of this plan (lever plus this entry): `sh tools/gate.sh` `gate-time total=52`, `gate-cpu` 85.67 + 76.52 = 162.2; `dash tools/gate.sh` `gate-time total=54`, 90.38 + 84.00 = 174.4; `sh tools/gate-linux.sh` exit 0, last log line `gate: PASS`, `gate-time total=220`, 853.64 + 67.07 = 920.7 (the laptop was quieter for the local pair). The summary lines are byte-identical to the base runs (same md5 per platform).

### Planned checkpoints for Phase 6

Each checkpoint is a section of this file written by the executor of the plan named, from the gate lines of that plan's final tree and `.planning/ci-runs.tsv` as the orchestrator recorded it; it ends with the line `checkpoint-N: lever-required` or `checkpoint-N: no-lever`.

| Checkpoint | Where | Cumulative adds read from the gate lines |
|------------|-------|------------------------------------------|
| CP-A | after plan 06-06 | faults, controls and test ids added since the counts above (`faults=193`, `controls=328`, `tests-expected=3778` at plan 06-02) |
| CP-B | after plan 06-09 | the same, cumulative |
| CP-C | after plan 06-12 | the same, cumulative |
| CP-D | at plan 06-14 (phase close) | the same, cumulative, plus the full hosted row evaluation |

Projection at each checkpoint (the realised overrun factor of Phase 5 is 2.0 and 231 is the Phase 5 close hosted median):

`projected_last5_median_cpu = 231 + 2.0 x (0.5 x faults_added + suite_term + 0.07 x pure_controls_added + 2.2 x symbol_controls_added + 0.0034 x ids_added_both_presets)`

where `suite_term` is the sum over suites of (faults in the suite x that suite's measured dev plain-run seconds), because `c-faults` reruns the whole suite executable once per fault, `ids_added_both_presets` is the added test ids counted once per preset, and the `0.5` per fault already includes its build (lower after `-g0`; the formula is kept as written so the checkpoints stay comparable with Phase 5). The checkpoint also prints `r` of the last five successful hosted rows and applies the rule above.

Stop rule: if `r` of the last five hosted rows exceeds 8.3 at a checkpoint, the orchestrator creates plan 06-L1 and no further fault is added until it lands. Executors never create it and never call `gh` or `tools/ci-record.sh`.

### Pre-committed lever order after `-g0` (applied only when the rule fires)

1. (done here) `-g0` on the fault targets.
2. Fault objects at `-O0` only, with an owner decision because it changes the evidence build (the fault executables would no longer be built at the dev flags the suites are proven with): about 19 hosted cpu-s.
3. One runtime-selected fault-enabled object per owner translation unit (instead of one recompiled object per fault).
4. Batch the `cp1610-vectors` JSON re-parse: about 2.5 hosted cpu-s.
5. A two-job split stays the last resort because it redefines PKG-02.

Not levers: dropping a control or a fault, merging checks, weakening an oracle, ccache while the gate is within budget (D12 bans caches).

### Request to the orchestrator

Please re-baseline the hosted budgets in view of the ratio (the 05-L1 precedent: a delegated, reversible policy decision): keep the raw triggers (gate median 75 s with trigger 90 s or any run above 120 s, cpu median 260 with trigger 280) and add `r = cpu_s / build_s` as the arbiter, budget 7.8 and revisit above 8.3, so that a runner-speed swing alone does not stop the phase. Also record `r` when the hosted rows are appended to `.planning/ci-runs.tsv`. This entry changes no budget and edits no earlier section.

phase6-start: lever-applied

## 2026-11-12 Phase 6 budget re-baseline after lever pass 06-02 (orchestrator)

- Lever 06-02 (`-g0` on fault-only targets) is applied and measured (about 18 hosted cpu-s saved on the cold `ivs_faults` build, no evidence change). The normalised metric `r = cpu_s / build_s` separates code growth from runner speed: over the 38 hosted rows it sits between 5.27 and 7.42 (median 6.30) and rose from 5.3-6.3 in Phase 4 to 6.4-7.4 at the Phase 5 close.
- Raw rows are dominated by runner speed (1.7x spread for equal code), so the raw medians no longer say whether the code got costlier. **Decision (delegated under AUTONOMY section 1; policy, reversible):** the arbiter for code cost is `r`: budget median 7.8, revisit trigger when the last-5 median `r` exceeds 8.3. The raw hosted budgets are re-baselined to the observed band so they stop firing on speed alone: median `gate_s` 90 s with a trigger at a last-5 median above 110 s or any run above 130 s; median `cpu_s` 300 with a trigger at a last-5 median above 330. The 5-minute job timeout is unchanged. No evidence is dropped and no check moves out of the gate.
- Levers still queued in order (06-02 entry): fault objects at `-O0` (owner decision, changes the evidence build), one runtime-selected object per owner TU, batching the `cp1610-vectors` re-parse, a two-job split (owner decision under D12).

## 2026-11-12 Phase 6 checkpoint A (plan 06-06)

Measured on the final tree of plan 06-06 (laptop, 18 cores, Apple clang 21, other work running; local figures, not hosted ones). Nothing here calls `gh` or touches `.planning/ci-runs.tsv`; plan 06-L1 is not created by the executor.

### Cumulative adds since the plan 06-02 counts (`faults=193 controls=328 tests-expected=3778`)

| Item | At plan 06-02 | Now (`gate:` line) | Added |
|------|---------------|--------------------|-------|
| faults | 193 | 202 | 9 (machine +3, run +1, stic +5) |
| controls | 328 | 331 | 3 (memmap +1, video +2; all pure step controls, no symbol control) |
| test ids (`tests-expected`, both presets) | 3778 | 3984 | 206 |
| `steps` / `profile-keys` | 36/36 / 142 at plan 06-06 start | 36/36 / 143 | 0 / 1 |

(The plan text quotes the controls range 316 to 331; the gate line at plan 06-02 reads 328, which is the base used here, so the controls term is 3, not 15.)

### Local gate, three `sh tools/gate.sh` runs and one `dash tools/gate.sh` run (same summary line, same md5)

| Run | `gate-time total` | `gate-cpu` user + sys | five largest `step-time` lines (s) |
|-----|-------------------|-----------------------|------------------------------------|
| A | 73 | 119.3 + 108.1 = 227.4 | controls 29, c-faults 19, c-tests 12, c-purity 7, hygiene 5 |
| B | 76 | 120.2 + 109.2 = 229.5 | controls 30, c-faults 20, c-tests 12, c-purity 6, hygiene 5 |
| C | 139 | 158.2 + 155.9 = 314.1 | controls 50, c-faults 30, c-tests 28, cp1610-vectors 13, c-purity 11 |
| dash | 107 | 153.5 + 150.7 = 304.2 | controls 58, c-faults 33, c-tests 19, c-purity 12, hygiene 7 |

Runs A and B are the quiet runs; C and the dash run ran while the laptop was loaded by another process (every step slower by the same factor, the summary line identical), so the honest local figure is A and B: 73 to 76 s and about 228 cpu-s, against a Phase 6 start of the same order (06-05: 145 s on a loaded laptop, 96 to 150 s measured this session on the same tree family). The `c-faults` step is 19 to 20 s for 202 faults (about 0.1 s per fault, the new small translation units `stic_events.c`, `stic_regs.c` and the one-line sites in `machine_run.c` and `machine_bus.c` recompile and relink in about that time, so they cost no more than the average fault).

### Suite plain-run times (dev preset, three runs each)

- `stic` (45 ids): dev 0.09, 0.09, 0.10 s; release below the clock resolution (0.00 s). Per-plan target after plan 06-06: at most 0.8 s dev; met by a factor of eight.
- `machine` (409 ids) 0.96 s and `run` 0.92 s, dev (the two other suites that gained faults since plan 06-02).

### Projection (formula of the Phase 6 start section)

`projected_last5_median_cpu = 231 + 2.0 x (0.5 x faults_added + suite_term + 0.07 x pure_controls_added + 2.2 x symbol_controls_added + 0.0034 x ids_added_both_presets)`

- `0.5 x faults_added = 0.5 x 9 = 4.50`
- `suite_term` = machine 3 x 0.96 + run 1 x 0.92 + stic 5 x 0.09 = 2.88 + 0.92 + 0.45 = 4.25 (each fault reruns the whole suite executable once, dev preset)
- `0.07 x pure_controls_added = 0.07 x 3 = 0.21`; `2.2 x symbol_controls_added = 0`
- `0.0034 x ids_added_both_presets = 0.0034 x 206 = 0.70`
- Sum 9.66; `231 + 2.0 x 9.66 = 250.3` against the 280 trigger (and the re-baselined hosted cpu budget of 300): 29.7 cpu-s of headroom to the trigger.

The speed-normalised arbiter `r = cpu_s / build_s` is read from `.planning/ci-runs.tsv` by the orchestrator when it records the hosted row of this plan; this plan adds no workflow, matrix or cache change and one generator step input (the probe include of the `stic` suite), so no change of `r` is expected beyond the added suite and faults counted above.

checkpointA: no-lever

## 2026-11-12 Phase 6 trigger review after plan 06-07 (orchestrator)

- Last-5 hosted medians: gate 115 s and cpu 345 (raw triggers 110 / 330: both over). Normalised metric `r = cpu_s / build_s` for the last five rows: 7.67, 7.49, 7.97, 7.98, 7.88, median **7.88** against the budget 7.8 and the revisit trigger 8.3: under the arbiter's trigger. The last three Phase 5 rows read 6.4 to 7.4, so r has drifted up about 10 percent over plans 06-03 to 06-07 (faults 193 to 203, ids 3778 to about 4000): a small real growth on top of runner speed.
- Decision: no lever now (arbiter not tripped); the next candidate in order is the 06-02 queue (fault objects at `-O0` is an owner decision; batching the `cp1610-vectors` re-parse, about 5 cpu-s, is the cheap one). If the median r exceeds 8.3 at checkpoint B (after 06-09) a lever pass runs before 06-10.

## 2026-11-12 Phase 6 checkpoint B (plan 06-09)

Measured on the final tree of plan 06-09 (laptop, 18 cores, Apple clang 21; local figures, not hosted ones). Nothing here calls `gh` or touches `.planning/ci-runs.tsv`; plan 06-L1 is not created by the executor.

### Cumulative adds since the plan 06-02 counts (`faults=193 controls=328 tests-expected=3778`)

| Item | At plan 06-02 | Now (`gate:` line) | Added |
|------|---------------|--------------------|-------|
| faults | 193 | 214 | 21 (machine +3, run +1, stic +11, video +6) |
| controls | 328 | 332 | 4 (three pure step controls and `core-symbols-palette-defined`, which reaches the C symbol layer) |
| test ids (`tests-expected`, both presets) | 3778 | 4210 | 432 (this plan: 4,088 at the start, +122) |
| `steps` / `profile-keys` / `api-calls` | 36/36 / 142 / 15 | 36/36 / 143 / 15 | 0 / 1 / 0 |

(The plan text quotes the controls range 316 to 332 and the faults range 193 to 214; the gate line at plan 06-02 reads 328 controls, the base used here.)

### Local gate, three `sh tools/gate.sh` runs on the final tree (same summary line, same md5), and two earlier runs at the Task 3 and Task 4 commits

| Run | `gate-time total` | `gate-cpu` user + sys | load at the end (1 min) | five largest `step-time` lines (s) |
|-----|-------------------|-----------------------|-------------------------|------------------------------------|
| final 1 | 157 | 153.6 + 156.8 = 310.4 | 63 | controls 79, c-faults 41, c-tests 24, c-purity 21, hygiene-history 11 |
| final 2 | 212 | 165.2 + 162.7 = 327.9 | 120 | c-faults 75, controls 66, c-tests 23, cp1610-vectors 19, c-purity 13 |
| final 3 | 168 | 166.0 + 170.3 = 336.3 | 56 | controls 85, c-faults 61, c-tests 21, c-purity 15, hygiene 10 |
| Task 3 commit | 90 | 128.5 + 113.9 = 242.4 | lower | controls 29, c-faults 25, c-tests 15, c-purity 8, hygiene-history 6 |
| Task 4 commit | 107 | 125.2 + 126.5 = 251.7 | lower | controls 37, c-faults 24, c-tests 21, hygiene 10, c-purity 10 |

The three final runs ran while other agents loaded the laptop to a 1-minute load of 55 to 120 (the machine has 18 cores), so every step is stretched by the same factor and the three figures are not a speed measurement; the two earlier runs, taken when the laptop was quieter, are the honest local figures: 90 to 107 s and 242 to 252 cpu-s, against 73 to 76 s and 228 cpu-s at checkpoint A (the growth is the 12 faults, one symbol-layer control and 226 ids added since). `c-faults` is 24 to 25 s for 214 faults on the quieter runs (checkpoint A: 19 to 20 s for 202): about 0.4 s per added fault (5 s for 12), above the 0.1 s per fault of the earlier small units because three of the sites sit in `machine_bound.c`, the largest core unit, and each fault reruns its whole suite.

### Suite plain-run times

- Quiet laptop (this session, Task 1 to Task 3 commits): `video` (54 ids) dev 0.26 s (the plan's target is at most 0.8 s with its 6 faults: met), release 0.02 s; `stic` (97 ids) dev 0.45 s release 0.18 s (06-08 figure, target 2.0 s).
- Loaded laptop at the time of the final runs (best of three, load 55): `video` dev 0.64 s release 0.15 s; `stic` dev 0.95 s release 0.31 s; `machine` dev 1.76 s release 0.90 s; `run` dev 1.57 s release 0.76 s. These are about twice the quiet figures (the quiet `machine` and `run` figures of checkpoint A are 0.96 s and 0.92 s).
- One 71-slot slice of video SHA-256 (the default slice of 4,194,304 ticks completes 70 frame events; 71 slots of 15,296 bytes plus the 32-byte chain input each, 1,086,016 bytes hashed through the incremental interface): 9.1 ms at `-O2` and 38.3 ms at `-O0` on the loaded laptop. The video chain costs nothing in the gate itself, because no gate scenario reaches its first frame end; it matters only to a host that runs long slices.

### Projection (formula of the Phase 6 start section)

`projected_last5_median_cpu = 231 + 2.0 x (0.5 x faults_added + suite_term + 0.07 x pure_controls_added + 2.2 x symbol_controls_added + 0.0034 x ids_added_both_presets)`

- `0.5 x faults_added = 0.5 x 21 = 10.50`
- `suite_term` = machine 3 x 0.96 + run 1 x 0.92 + stic 11 x 0.45 + video 6 x 0.26 = 2.88 + 0.92 + 4.95 + 1.56 = 10.31 (each fault reruns the whole suite executable once, dev preset, quiet figures)
- `0.07 x pure_controls_added = 0.07 x 3 = 0.21`; `2.2 x symbol_controls_added = 2.2 x 1 = 2.20`
- `0.0034 x ids_added_both_presets = 0.0034 x 432 = 1.47`
- Sum 24.69; `231 + 2.0 x 24.69 = 280.4`, on the raw cpu trigger of 280 and 19.6 under the re-baselined hosted cpu budget of 300.

The speed-normalised arbiter, read from `.planning/ci-runs.tsv` (successful rows, last five: runs 43305635550, 43311565235, 43321727175, 43325371818, 43331294071): `r = cpu_s / build_s` is 7.49, 7.97, 7.98, 7.88 and 7.90, median **7.90** against the budget 7.8 and the revisit trigger 8.3 (raw gate seconds 92, 93, 121, 121, 124 and cpu 277, 287, 375, 378, 387: the raw medians are over the raw triggers, as the 06-07 review recorded, and the arbiter says that is runner speed, not growth beyond the budget). Expected movement from this plan alone: it adds 6 faults, 1 symbol-layer control and 122 ids, about 2.0 x (0.5 x 6 + 6 x 0.26 + 2.2 + 0.0034 x 122) = 14.4 hosted cpu-s on the 387 of the last row (3.7 percent) and about one build second on 49 (2 percent), so the next hosted `r` is expected near 8.0 (range 7.9 to 8.2), under 8.3 by a margin of 0.1 to 0.4.

Reading: no lever is required before plan 06-10 by the rule in force (`r` under 8.3; the raw projection of 280.4 is the raw trigger, which the rule reads together with `r`). The margin is thin: plan 06-10 adds the card renderer and its fault set (about the cost of this plan's video suite), so the orchestrator should read `r` from the first hosted row of this plan and, if the median reaches 8.3, run the lever queue of the 06-07 review before 06-10 (batching the `cp1610-vectors` re-parse, about 5 cpu-s, is the cheap one; `-O0` fault objects remain an owner decision). The container gate figure of this plan is in its SUMMARY.

checkpointB: no-lever

## 2026-11-12 Phase 6 checkpoint C (plan 06-12)

Measured on the final tree of plan 06-12 (laptop, 18 cores, Apple clang 21; local figures, not hosted ones). Nothing here calls `gh` or touches `.planning/ci-runs.tsv`; plan 06-L1 is not created by the executor.

### Cumulative adds since the plan 06-02 counts (`faults=193 controls=328 tests-expected=3778`)

| Item | At plan 06-02 | Now (`gate:` line) | Added |
|------|---------------|--------------------|-------|
| faults | 193 | 221 | 28 (machine +3, run +1, runner +2, stic +11, video +11) |
| controls | 328 | 335 | 7 (six pure step controls, three of them from this plan, and `core-symbols-palette-defined`, which reaches the C symbol layer) |
| test ids (`tests-expected`, both presets) | 3778 | 4382 | 604 (this plan: 4,304 at the start, +78) |
| `steps` / `api-calls` | 36/36 / 15 | 36/36 / 15 | 0 / 0 |

(The plan text quotes the controls range 316 to 335 and the faults range 193 to 221; the gate line at plan 06-02 reads 328 controls, the base used here, as at the earlier checkpoints.)

### Local gate, four `sh`/`dash` runs on the final tree (same summary line apart from the tracked-file counts)

| Run | `gate-time total` | `gate-cpu` user + sys | load at the end (1 min) | five largest `step-time` lines (s) |
|-----|-------------------|-----------------------|-------------------------|------------------------------------|
| sh 1 | 258 | 177.4 + 195.5 = 372.8 | 36 | controls 189, c-tests 75, c-faults 73, c-purity 26, hygiene 14 |
| dash | 170 | 175.2 + 177.8 = 353.0 | 33 | controls 100, c-faults 49, c-tests 34, c-purity 15, hygiene 11 |
| sh 2 | 162 | 176.5 + 177.0 = 353.5 | 25 | controls 82, c-faults 38, c-tests 32, c-purity 20, hygiene 11 |
| sh 3 | 142 | 171.3 + 170.5 = 341.8 | 21 | controls 90, c-faults 39, c-tests 27, c-purity 16, hygiene 9 |

The laptop was loaded by other agents for the whole session (1-minute load 21 to 36 on an 18-core machine), so every step is stretched; the quieter runs (sh 3: 142 s, 342 cpu-s, c-faults 39 s for 221 faults) are the honest local figures, against 90 to 107 s and 242 to 252 cpu-s at checkpoint B (when the laptop was quieter) and 73 to 76 s at checkpoint A. The growth since checkpoint B is 7 faults, 3 controls and 154 ids; the `controls` step (335 controls, each on a fresh tree copy) is the largest step in every run.

`runner-process` ctest time (the process-level suite, which the five new operator cases and the changed header check join; the slice-1 and slice-7 replays stay limited to one scenario in plan 06-13): dev 15.0 s, release 8.0 s, at a load of 13 to 27 (first run in this session 23.7 s at a higher load). The new operator cases add two runs of the 130,000-tick looping scenario per preset and a few of the 3,000-tick one.

### Suite plain-run times (dev preset, best of three, laptop loaded)

`runner` 1.37 s (240 ids; it now holds the video chain re-hash against real core slots and the 130,000-tick scenarios), `stic` 0.70 s, `video` 0.40 s, `machine` 1.07 s, `run` 1.28 s. The two new runner faults rerun the `runner` suite once each (2 x 1.37 s).

### Projection (formula of the Phase 6 start section)

`projected_last5_median_cpu = 231 + 2.0 x (0.5 x faults_added + suite_term + 0.07 x pure_controls_added + 2.2 x symbol_controls_added + 0.0034 x ids_added_both_presets)`

- `0.5 x faults_added = 0.5 x 28 = 14.00`
- `suite_term` = machine 3 x 1.07 + run 1 x 1.28 + runner 2 x 1.37 + stic 11 x 0.70 + video 11 x 0.40 = 3.21 + 1.28 + 2.74 + 7.70 + 4.40 = 19.33 (each fault reruns the whole suite executable once, dev preset, loaded-laptop figures, so this term is on the high side)
- `0.07 x pure_controls_added = 0.07 x 6 = 0.42`; `2.2 x symbol_controls_added = 2.2 x 1 = 2.20`
- `0.0034 x ids_added_both_presets = 0.0034 x 604 = 2.05`
- Sum 38.00; `231 + 2.0 x 38.00 = 307.0`, above the raw cpu trigger of 280 and 7 over the re-baselined hosted cpu budget of 300 (checkpoint B projected 280.4 for 21 faults and 432 ids; the 7 faults, 3 controls and 154 ids since, and the heavier suite times of a loaded laptop, account for the step).

The speed-normalised arbiter, read from `.planning/ci-runs.tsv` (successful rows, last five: runs 43325371818, 43331294071, 43341351141, 43346071659, 43352432394): `r = cpu_s / build_s` is 7.88, 7.90, 5.95, 7.64 and 7.68, median **7.68** against the budget median 7.8 and the revisit trigger 8.3 (cpu 378, 387, 262, 321, 315 over build 48, 49, 44, 42, 41): not tripped. The last three hosted rows, which already contain plans 06-09 to 06-11, are at or under the budget, so the raw projection of 307 over-states the hosted cost (it uses the Phase 5 overrun factor 2.0 and loaded-laptop suite times); the arbiter is the normalised metric, as the 06-07 review recorded. This plan's own hosted cost is two faults on the runner suite (about 2 x 0.5 + 2 x 1.37 hosted-scale seconds), three controls (pure, 0.07 each) and 78 ids, about 2.0 x (1.0 + 2.74 + 0.21 + 0.27) = 8.4 cpu-s on the 315 of the last row (under 3 percent), so `r` is expected to stay near 7.7 to 7.9. The orchestrator should still read `r` from the first hosted row of this plan: at 8.3 or above the lever queue of the 06-07 review (batching the `cp1610-vectors` re-parse, about 5 cpu-s; `-O0` fault objects remain an owner decision) runs before plan 06-13 adds its two goldens and the diagnostic scenarios. The container gate figure of this plan is in its SUMMARY.

checkpointC: no-lever

### Plan 06-12 hosted row review (orchestrator)

Run 43359195974 (head of plan 06-12): gate 131 s, cpu 433, build 48 s, test 83 s. Single-row `r = 433 / 48 = 9.02`, above the 8.3 trigger; last-five `r` is 7.90, 5.95, 7.64, 7.68, 9.02, median **7.68** (not tripped). The plan was projected to add under 3 percent, but test_s rose 60 to 83 s (38 percent) with the same build time, which the plan's own change (two faults, three pure controls, 78 ids) does not explain, so a noisy runner is the working hypothesis. Decision: no lever pass on one sample (the trigger is defined on the last-five median); the next hosted row, from plan 06-13, is the confirmation. If that row is also at or above 8.3, run the 06-07 lever queue (batch the `cp1610-vectors` re-parse, then the owner-decision `-O0` fault objects) before plan 06-14. Recorded so the outlier is not silently absorbed.

### Plan 06-13 hosted row review (orchestrator)

Run 43365038335 (head of plan 06-13, which adds the fourth image, three scenarios, seven mutant cases and the consumer third replay): gate 87 s, cpu 271, build 36 s, test about 51 s, `r = 271 / 36 = 7.53`. This confirms the 06-12 row (`r` 9.02) was runner noise: the last-five `r` is 7.64, 7.68, 9.02, 7.53 and the row before, median about 7.64, under the 8.3 trigger. No lever pass is needed; the lever queue stays queued. Raw gate 87 s and cpu 271 are under the raw budgets (90 and 300).

## 2026-11-12 - Phase 6 close-out (plan 06-14): D-45 evaluation, checkpoint D and the normalised metric

Local figures are from the laptop (18 cores, Apple clang 21, other work running, 1-minute load 40 to 70 during runs 1 and 2 and lower during run 3); they are not hosted figures. Nothing here calls `gh` or touches `.planning/ci-runs.tsv`: the hosted rows are those the orchestrator recorded. Plan 06-L1 was never created and this plan does not create it.

### Counts

- Phase 5 end (the start of Phase 6, plan 06-02 base for the checkpoints in brackets): `steps=35/35 controls=316/316 [328 at 06-02] faults=193/193 tests-expected=3778`. Phase 6 end: `steps=36/36 controls=339/339 faults=221/221 tests-expected=4422 tests-executed=4420 tests-skipped=2 tests-missing=0`, `claims=22 provisional=21`, `discrepancies=105 d10-items=42`, `listings=4 pkg-selftest=15/15 consumer-lines=76 video-hash-ties=2`, important-gate registry `gates=71 lines=238` (Phase 5 end: `gates=48 lines=175`).
- What grew since the Phase 5 end: one step (`video`), 23 controls (316 to 339; plan 06-14 added 3), 28 faults (193 to 221: machine 3, run 1, runner 2, stic 11, video 11), two new suites (`stic` and `video`) plus the grown `run`, `runner`, `runner-process`, `machine` and `memmap` suites (644 test ids over both presets), the generator inputs for the `stic.*` and `video.*` profile fields, the fourth image and three scenarios with seven mutant cases (06-13), and the third installed-consumer replay (`consumer-lines` 43 to 76, `pkg-selftest` 13 to 15).

### Local measurement (`sh tools/gate.sh`, three runs; `dash tools/gate.sh` once)

| Run | `gate-time total=` | `gate-cpu` user + sys | Five largest `step-time` (seconds) |
|-----|--------------------|-----------------------|-------------------------------------|
| sh 1 (data-only tree of task 1) | 244 | 181.5 + 196.7 = 378.2 | controls 128, c-faults 93, c-tests 35, c-purity 17, c-build 17 |
| sh 2 (task 2 tree, registry and controls) | 178 | 178.5 + 188.1 = 366.6 | controls 93, c-faults 42, c-tests 39, c-purity 16, hygiene-history 13 |
| dash (same tree as sh 2) | 145 | 171.8 + 162.4 = 334.2 | controls 72, c-faults 41, c-tests 31, c-purity 12, hygiene 9 |
| sh 3 (task 3 tree, Decision Register and this section) | 118 | 170.4 + 157.0 = 327.4 | controls 50, c-faults 31, c-tests 26, c-purity 11, hygiene 9 |

The laptop was heavily loaded for runs 1 and 2 (1-minute load 40 to 70 on 18 cores), so those steps are stretched (run 1 had a c-faults step of 93 s that the same code takes 31 to 42 s in the others); the quieter runs, 118 to 178 s and 327 to 367 cpu-s, are the honest local figures and match checkpoint C (142 to 258 s, 342 to 373 cpu-s), run 3 being the lightest-loaded of the phase's close-out. The `controls` step (339 controls, each on a fresh tree copy) is the largest step in every run; plan 06-14 added three pure controls (about 0.07 cpu-s each) and a registry that the `important-gates` step reads in well under a second, so its own local cost is not measurable against the load. `c-faults` is 41 to 42 s for 221 faults when the machine is not stalled (checkpoint C: 38 to 39 s at 221).

### Hosted rows and the rule

- `.planning/ci-runs.tsv` holds 51 hosted rows, 14 of them since the Phase 5 close (run 43246638522, head 68e7d06), all successful. Since then: `gate_s` 75 to 131 (median of the 14: 98), `cpu_s` 210 to 433 (median 301), `r = cpu_s / build_s` 5.95 to 9.02.
- Last five rows (runs at plans 06-12 and 06-13 and the three before): 43341351141, 43346071659, 43352432394, 43359195974, 43365038335. `gate_s` 95, 102, 101, 131, 87, median **101**. `cpu_s` 262, 321, 315, 433, 271, median **315**. `build_s` 44, 42, 41, 48, 36. `r` 5.95, 7.64, 7.68, 9.02, 7.53, median **7.64**.
- The arbiter is the set the orchestrator re-baselined in the entry `Phase 6 budget re-baseline after lever pass 06-02` (and copied into the Phase 6 start section as the rule for Phase 6): gate median budget 90 s, trigger a last-5 median above 110 s or any run above 130 s; cpu median budget 300, trigger a last-5 median above 330; `r` budget 7.8, revisit above 8.3. The plan text quotes the older figures (gate budget 75 s with trigger 90 s or any run above 120 s, cpu budget 260 with trigger 280); this evaluation uses the re-baselined ones and reads the older ones beside them.
- Result against the re-baselined rule: last-5 `gate_s` median 101 is under 110; last-5 `cpu_s` median 315 is above the budget (300) and under the trigger (330); `r` median 7.64 is inside the budget (7.8) and under the revisit line (8.3). One run exceeded the any-run bound: 43359195974 at 131 s, one second above 130. It is judged runner noise: a single sample, the next row (43365038335) is 87 s on a tree that is larger (plan 06-13 added scenarios, mutants and a replay); the row has the slowest `build_s` of the five (48 against 41 and 36 on its neighbours) and its `test_s` rose from 60 to 83 against a change of two faults, three pure controls and 78 ids that cannot account for it; its single-row `r` of 9.02 is above 8.3 but both neighbours are at 7.68 and 7.53, and the trigger is defined on the median. The orchestrator reviews recorded in this file after the 06-12 and 06-13 rows say the same. Read against the older figures the raw triggers would fire (median 101 above 90, a 131 s run above 120, cpu median 315 above 280); the rule says a raw trigger that fires while `r` is inside the budget is a runner-speed event, and `r` is inside it.
- Four rows of the Phase 6 run are above the `r` budget of 7.8 (7.97, 7.98, 7.88 and 7.90, at plans 06-07 to 06-09) and none above the revisit line except the one noisy row; the later rows fell back to 7.5 to 7.7. Over the whole phase `r` drifted from 6.4 to 7.4 at the Phase 5 close to about 7.6, a small real growth (faults, ids, the second consumer replay) on top of runner speed, still under 8.3.

### Projection and checkpoints

- Cumulative adds since the plan 06-02 counts (`faults=193 controls=328 tests-expected=3778`): faults +28, controls +11 (10 pure, 1 reaching the C symbol layer), ids +644. `projected_last5_median_cpu = 231 + 2.0 x (0.5 x 28 + suite_term + 0.07 x 10 + 2.2 x 1 + 0.0034 x 644)` with `suite_term` 19.33 (carried from checkpoint C: no suite gained a fault in plans 06-13 and 06-14) = `231 + 2.0 x (14.00 + 19.33 + 0.70 + 2.20 + 2.19)` = `231 + 2.0 x 38.42` = **307.8**, above the cpu budget of 300 and under the trigger of 330 (and over the old 280, the line that the re-baseline superseded). The raw projection uses the Phase 5 overrun factor and loaded-laptop suite times, so it is the pessimistic figure; the hosted median read from rows is 315 and the arbiter `r` is 7.64.
- Checkpoint A (plan 06-06): `checkpointA: no-lever`. Checkpoint B (plan 06-09): `checkpointB: no-lever`. Checkpoint C (plan 06-12): `checkpointC: no-lever`. Checkpoint D (this plan): no lever, on the evaluation above.
- Plan 06-L1 was not created (the orchestrator's stop rule, `r` of the last five above 8.3, never held). The only lever of the phase was plan 06-02, `-g0` on the fault targets only (about 18 hosted cpu-s saved on the cold `ivs_faults` build, no evidence change). Plan 06-14 creates no plan and calls no tool that reads the hosted API.
- Pre-committed lever order if the rule fires after this phase (unchanged): (1) batch the `cp1610-vectors` JSON re-parse (about 2.5 to 5 hosted cpu-s, no evidence change), (2) fault objects at `-O0` (owner decision, because it changes the evidence build, about 19 hosted cpu-s), (3) one runtime-selected fault-enabled object per owner translation unit, (4) a two-job split only as the last resort (it redefines PKG-02, owner decision under D12). Not levers: dropping a control or a fault, merging checks, weakening an oracle, ccache while the gate is within budget (D12 bans caches).
- Open for the next phase (not a lever): Phase 7 adds a fourth scenario and most likely a project interrupt routine and a second visible fixture; the first hosted rows after this push are the arbiter, and the margin to the `r` trigger (7.64 against 8.3) is about 8 percent.

phase6-close: no-lever

### Phase 6 review-fix head hosted row (orchestrator)

Run 43373151931 (head with the review fixes: 5 controls, 1 fault, 10 ids more): gate 107 s, cpu 335, build 41 s, test 66 s, `r = 8.17`. Last five hosted rows (43352432394, 43359195974, 43365038335, 43369159094, 43373151931): cpu median 335 (raw trigger 330 is crossed by 5 cpu-s), gate median 107 (trigger 110, not crossed), `r` 7.68, 9.02, 7.53, 8.02, 8.17, median **8.02** (budget 7.8, trigger 8.3: not crossed). Reading: the raw cpu trigger is crossed while the speed-normalised arbiter is not, and the three rows since 06-12 sit at 7.5 to 9.0, so the trend is flat to slightly up, not a step. Decision (delegated authority): no lever pass inside Phase 6; Phase 7 plan 1 starts with a trigger check and, if the last-five `r` median is 8.0 or above at that point, the first cheap lever (batching the `cp1610-vectors` re-parse, about 5 cpu-s, plus fault objects at `-g0` already applied) runs as its first task before new tests are added. `-O0` fault objects and the two-job split remain owner decisions.

## 2026-11-13 Phase 7 start: trigger check and lever (plan 07-01)

Nothing here calls `gh` or `tools/ci-record.sh`; `.planning/ci-runs.tsv` is orchestrator-owned and untouched. Local figures are from the laptop (18 cores, Apple clang 21, other work running, 1-minute load 10 to 25); container figures are Ubuntu 24.04 with cmake 3.28.3, emulated amd64 on an arm64 host; neither is a hosted figure. Counts at plan start and end: `steps=36/36 controls=344/344 faults=222/222 tests-expected=4432 tests-executed=4430 tests-skipped=2 tests-missing=0`.

### Trigger check (D-34)

The last five successful hosted rows of `.planning/ci-runs.tsv`, recomputed from the file (the same five as at planning time, no row was appended since):

| Run | `cpu_s` | `build_s` | `r = cpu_s / build_s` |
|-----|---------|-----------|------------------------|
| 43352432394 | 315 | 41 | 7.68 |
| 43359195974 | 433 | 48 | 9.02 |
| 43365038335 | 271 | 36 | 7.53 |
| 43369159094 | 393 | 49 | 8.02 |
| 43373151931 | 335 | 41 | 8.17 |

Median `r` **8.02** against the budget 7.8 and the trigger 8.3 (D-34): at 8.0 or above, so the first cheap lever of the queue (batching the `cp1610-vectors` JSON re-parse) ran before any Phase 7 test was added. No test, control, fault, scenario or fixture is added by this plan.

### What the lever turned out to be

The queued description (every `string(JSON ... GET "${doc}" ...)`, `json_type` and `vget` call parses the whole file again) was half right. `vectors.cmake` already read the members from the vector object and not from the file: the file text was parsed once per vector (the element fetch) and the vector object, about 2.5 KB of text, once per member lookup. On this machine one `string(JSON)` call costs about 120 thousand instructions of fixed command cost plus about 55 instructions per byte of the text it parses: about 260 thousand for a lookup in a vector object, about 13 million for the element fetch from the 237 KB `branch.json`. A profile of the checker found about 180 `string(JSON)` calls per vector (17.6 thousand calls for the 98 vectors of `branch.json`), 41.5 G instructions retired for the 412 vectors (about 100 million per vector), and `vector-blind.cmake` doing the same kind of work plus a second element fetch per vector (18.9 G). Changes, all in `tools/check/vectors.cmake` and `tools/review/vector-blind.cmake`:

1. `vshape` takes the member list of the object it checks: a spec key that is not a member is MISSING without a lookup, a member is typed with one lookup, and the types are handed back to the caller (`VT_<key>`). The spec lists are split once (`vspecs`) instead of once per object.
2. The vector loop fetches each member value it uses once (`VS_<key>`: the string, the number text, or the JSON text of an array or object). The later checks read those variables, or look into the JSON text of the member (`sources`, `program`, `covers`, `ebci`, `init.r`, `init.status`, `expect.r`, `expect.status`, `expect.cycles`, a few hundred bytes) and not into the whole vector object again. The `covers` walk collects the coverage tags in the same pass, and `decle_leaves` is computed once per decle value.
3. `vector_elements` splits the printed `vectors` array into its element texts after one parse of the file, so the element fetch is no longer one file-sized lookup per vector. CMake's writer prints each element of an array of objects between a line `  {` and a line `  }` at indent 2 and escapes newlines in strings, so the text `newline, 2 spaces, }, comma, newline, 2 spaces, {` occurs only between two elements. A file that does not print that way (different count, head or tail) is refused and takes the old per-element lookup, and a split element that does not parse is looked up on its own. The helper is in both files because `vector-blind.cmake` runs as a separate script and `lib.cmake` is shared by all checkers.
4. `vector-blind.cmake` fetches each vector object once instead of twice and skips the lookups of `bus` and of the stimulus members a vector does not have.

Findings keep their tokens, detail text, emission order and `${CAP}` listing; the stats are unchanged; nothing was weakened to gain time (T-07-01).

### Measurement (before and after)

| Measure | Before | After |
|---------|--------|-------|
| `vectors.cmake`, instructions retired (laptop, `time -l`) | 41.5 G | 22.3 G (-46 percent); 26.4 G after the first task alone |
| `vector-blind.cmake`, instructions retired | 18.9 G | 9.5 G (-50 percent) |
| `string(JSON)` calls per vector (profile, `branch.json`) | about 180 | about 115, the file-sized one gone |
| `cp1610-vectors` step in isolation, user plus sys cpu, three alternated runs each | 3.37, 2.98, 2.91 (median 2.98 s) | 1.82, 1.57, 1.67 (median 1.67 s, -44 percent) |
| container, cmake 3.28.3, `vectors.cmake` user plus sys, three alternated runs | 5.85, 5.80, 6.08 (median 5.85 s) | 3.43, 3.18, 2.74 (median 3.18 s, -46 percent) |
| container, `vector-blind.cmake`, the same runs | 2.25, 2.60, 2.79 (median 2.60 s) | 1.52, 1.28, 1.14 (median 1.28 s, -51 percent) |
| `step-time cp1610-vectors` in the gate log (wall, laptop) | 3 s | 1 s |

The base runs use a copy of the original files kept outside the tree and the same `ROOT`, `FILELIST` and `CHECK_DIR` invocation as the step; the first task alone gave 4.20, 4.63, 4.59 against 3.51, 3.36, 3.50 cpu-s on a more loaded laptop. The split engages on cmake 3.28.3 (the container figures above show the same saving), so the hosted toolchain gets the fast path.

### Evidence unchanged

- The `gate:` summary line of `sh tools/gate.sh` is byte-identical before and after (the same md5, 3a7b713fdf61db70b1fae10ceb49dabf, for the base run, after the first task and after the second task; counts `steps=36/36 controls=344/344 faults=222/222 tests-expected=4432 tests-executed=4430 tests-skipped=2 tests-missing=0`), and the sorted `control` lines of the log are identical, so every control of `vectors.ids` and `vectors-review.ids` still ends with its EXPECT line. The container summary line differs from the previous container log only in `ci-runs` (52 to 53) and in the tracked-file counts (`tracked`, `hygiene-files`, `files`: 2105 to 2127), the tree growth since that log.
- The blind copies written by `vector-blind.cmake` are byte-identical (`cmp` per file, 12 files, and the stdout), on the laptop and under cmake 3.28.3.
- A differential run of 700 mutated copies of one vector file each (member deletions, type changes, odd strings, wrong element and array shapes, status and register shapes) gave identical output, exit status and blind files from the old and the new checkers.

### Hosted effect and the phase budget

The queued estimate was about 5 hosted cpu-s. The measurement does not support that: the step lost 1.3 cpu-s on the laptop (2.98 to 1.67) and the two checkers lost about 4.0 emulated container cpu-s (8.5 to 4.5), so the expected hosted saving is about 2 to 3 cpu-s. The planning headroom of about 11.5 hosted cpu-s before the lever therefore becomes about 14 (not 16) against the phase estimate of 10 to 19 hosted cpu-s in `07-RESEARCH.md`: the high end is still over the line, so the checkpoints below stay the control. The hosted arbiter is the orchestrator's next recorded row; this plan reads none. What is left in the checker is about 1 G of setup, about 18 G in the vector loop (about 115 lookups, most of them in small texts, plus command overhead) and about 3 G in the coverage section; no second pass over it is queued.

### Checkpoints and stop rule for Phase 7

- C-B after plan 07-08, C-C after plan 07-09, and the close in plan 07-12: each is a section of this file written by the executor of that plan from the gate lines of its final tree and `.planning/ci-runs.tsv` as the orchestrator recorded it.
- Stop rule: if the last-five median `r` is above 8.3 at a checkpoint, the orchestrator creates a lever plan and no further test case is added until it lands. Executors never create it and never call `gh` or `tools/ci-record.sh`.

### Pre-committed lever order for Phase 7 (applied only when the rule fires)

1. (done here) Batch the `cp1610-vectors` JSON re-parse.
2. Run the mutant inventory on the release preset only.
3. Fold `demo-lamp-tone-swap` or `demo-lamp-tone-shift` into repeat variants of one mutant.

Owner-decision levers (fault objects at `-O0`, one runtime-selected object per owner translation unit, a two-job split) are out of scope for an executor. Not levers: dropping a mutant class, the judge or an oracle, caching (D12 bans caches).

phase7-start: lever-applied

## 2026-11-13 Phase 7 checkpoint C-B (plan 07-08)

Nothing here calls `gh` or `tools/ci-record.sh`; `.planning/ci-runs.tsv` is orchestrator-owned and untouched (54 lines, the newest row is still run 43373151931, the head of the Phase 6 review fixes: no hosted row exists for any Phase 7 commit). Local figures are laptop figures (18 cores, Apple clang 21, other work running, 1-minute load 8 to 20 during the timings below); none is a hosted figure. Counts at the end of this plan: `steps=36/36 controls=354/354 faults=222/222 goldens=15 tests-expected=4596 tests-executed=4594 tests-skipped=2 tests-missing=0` (plan 07-07 ended at `tests-expected=4570`, plan 07-06 at `4546`).

### Cases added by the heaviest wave (W4)

| Plan | Cases per preset | What |
|------|------------------|------|
| 07-07 | 12 | 4 scenarios, 2 pair judgements, 3 repeatability, 3 judge self-controls |
| 07-08 | 13 | 12 Lamp Tone mutants and 1 negative row (`runner-process.mutant.SCN-*`) |

w4-cases: 25 per preset (S-4 ceiling 19; +6 = second re-arm edge and five locked runtime mutant classes, D-25, A-03, A-14, A-17)

`tests-expected` moved 4546 to 4570 (07-07, 12 cases x 2 presets) to 4596 (07-08, 13 x 2): +50 for the wave. This plan adds 0 controls and 0 faults (the mutants are suite cases and registry lines). The phase's pure controls so far: 10 of the planned 13 (A-11: 07-02 four, 07-03 two, 07-04 two, 07-05 one, 07-06 one; 07-10 two and 07-12 one are to come), `controls` 344 to 354; faults stay 222. No mutant class was dropped and no check was weakened to save time.

### Local suite time of `tests/runner-process-tap.sh`

| Measure | dev | release |
|---------|-----|---------|
| before 07-07 (tree of `a34dda1`, median of three, from the 07-07 SUMMARY) | 12.03 s | 6.24 s |
| after 07-07 (tree of `d533c1f`, median of three, from the 07-07 SUMMARY) | 14.42 s | 6.78 s |
| start of this plan (tree of `e6565b4`, 84 ok cases, three runs 12.29, 13.54, 14.45 / 6.78, 6.83, 6.87, load about 20) | 13.54 s | 6.83 s |
| after this plan (tree of `647365a`, 97 ok cases, three runs 16.59, 17.38, 17.85 / 7.49, 7.47, 7.61, load about 12) | 17.38 s | 7.49 s |
| alternated pairs in one session (the script of `e6565b4` against the script of this plan on the same tree, three each, medians: dev 15.33 to 17.84, release 6.85 to 7.67) | +2.5 s | +0.8 s |

Reading: the 13 new cases cost about 0.19 s each on dev and 0.06 s each on release (alternated pairs), about three times more on the ASan and UBSan preset. Over the wave the suite grew from 12.03 s to 17.38 s on dev (+5.4 s, both plans; the machine load differed between the sessions, so the alternated pairs above are the cleaner delta) and from 6.24 s to 7.49 s on release (+1.3 s). A mutant case is one `ivs_pack` call (the `--dump-words` read is once per inventory, not per row) and one `ivs-run` of a full scenario: 0.06 s on dev for the pass scenario, plus the per-run harness (watchdog arming, report judgement).

### Hosted rows and the rule (D-34)

The last five successful hosted rows of `.planning/ci-runs.tsv`, recomputed from the file with awk (the same five as at the Phase 7 start entry; no row was appended since):

| Run | `cpu_s` | `build_s` | `r = cpu_s / build_s` |
|-----|---------|-----------|------------------------|
| 43352432394 | 315 | 41 | 7.68 |
| 43359195974 | 433 | 48 | 9.02 |
| 43365038335 | 271 | 36 | 7.53 |
| 43369159094 | 393 | 49 | 8.02 |
| 43373151931 | 335 | 41 | 8.17 |

Median `r` **8.02** (8.0204), budget 7.8, revisit line 8.3: the median does not exceed 8.3. None of the five rows includes a Phase 7 commit (the newest, run 43373151931, is the Phase 6 review-fix head; no plan 07-07 or 07-08 commit has been pushed, `git merge-base --is-ancestor` of the first 07-08 commit against each of the five shas is false), so the projection is the median plus the whole wave: 8.02 + 25 case-pairs x 0.7 hosted cpu-s (the S-4 upper unit cost) / 41 (`build_s` of the newest row, run 43373151931) = 8.02 + 17.5 / 41 = 8.02 + 0.43 = **8.45**. The projection exceeds 8.3, so by the rule of this checkpoint the answer is `lever-required`; it is not softened.

### Decision

Lever (2) of the pre-committed order of the Phase 7 start entry is the planned fallback: run the mutant inventories on the release preset only. It is created by the orchestrator as a lever plan (an executor never creates it). No further test case is added by any plan until that lever plan lands. [inference] The cases to move are the 29 `runner-process.mutant.*` cases per preset (16 of earlier phases and the 13 of this plan); this plan's measurement suggests the dev half is about three quarters of a mutant case's cost, so the lever would remove only part of the projected 17.5 hosted cpu-s of the wave, and the lever plan has to measure its effect, not assume it: the hosted rows after the push are the arbiter. Lever (3) (fold `demo-lamp-tone-swap` or `-shift` into repeat variants of one mutant) stays queued behind it. Not levers: dropping a mutant class, the judge or an oracle, caching.

checkpointB-07: lever-required

### Checkpoint C-B resolution (orchestrator, hosted row 43528527276)

The recorded `lever-required` came from the branch of the rule that applies when no hosted row contains plan 07-08's commits (projection 8.45). The orchestrator pushed the phase branch and opened a draft pull request, so the other branch of the same rule now applies. Run 43528527276 (head 785dce0, the head of plan 07-08, so it contains plans 07-01 to 07-08 including the lever of 07-01 and all 25 W4 case pairs): gate 91 s, cpu 286, build 37 s, test 54 s, `r = 286 / 37 = 7.73`. Last five hosted rows (43359195974, 43365038335, 43369159094, 43373151931, 43528527276): `r` 9.02, 7.53, 8.02, 8.17, 7.73, median **8.02**, which does not exceed 8.3; the median is the projection when a row already includes the plan, so the answer by the rule is no lever. Raw: gate 91 s and cpu 286 are under the re-baselined budgets (90 s median / 300 cpu) apart from one second. Reading: the measured hosted cost of the 25 case pairs is far below the 17.5 cpu-s of the S-4 upper unit cost; it is one row, so the trend is not settled. Decision (delegated authority): lever (2) is not run now and stays queued; plan 07-09 (checkpoint C-C) re-reads the last five and runs the lever before any further case if the median exceeds 8.3. The `checkpointB-07: lever-required` line above is left as the executor wrote it, as the record of the rule's first answer.

## 2026-11-13 Phase 7 checkpoint C-C (plan 07-09)

Nothing here calls `gh` or `tools/ci-record.sh`; `.planning/ci-runs.tsv` is orchestrator-owned and untouched (55 lines; the newest row is run 43528527276, head 785dce0, the head of plan 07-08). Local figures are laptop figures (18 cores, Apple clang 21, other work running, 1-minute load 7 to 10 during the timings below); container figures are Ubuntu 24.04 with cmake 3.28.3 and gcc 13.3.0, emulated amd64 on an arm64 host; neither is a hosted figure. Counts at the end of this plan: `steps=36/36 controls=354/354 faults=222/222 goldens=15 pkg-selftest=17/17 tests-expected=4596 tests-executed=4594 tests-skipped=2 tests-missing=0`.

### What this plan added to the gate

One more replay in the installed consumer (the original program run through the installed headers: three machine runs of 806,568 ticks per consumer start, 13 frames each) and two in-step self-tests of the `pkg-install` step (each a `grep` and an `awk` over the consumer output). No process case, control, fault or test id: `tests-expected` stays 4596, `controls` 354, `faults` 222. The consumer source grew from 1,074 to 1,767 lines and its output from 76 to 135 lines; it is compiled once per gate run with the strict flags.

### Local time of the `pkg-install` step

The step in isolation (`ROOT` and `CHECK_DIR` set, the release tree built), three runs on the tree before this plan and six on the tree after it (real, then user plus sys cpu-s):

| Measure | Before (tree of 290f057) | After (tree of 5ee7d13) |
|---------|--------------------------|--------------------------|
| real s | 2.40, 2.41, 2.23 (median 2.40) | 2.33, 2.24, 2.21 and, later at a higher load, 2.55, 2.53, 2.48 (median 2.41) |
| user plus sys cpu-s | 2.40, 2.40, 2.28 (median 2.40) | 2.41, 2.33, 2.30 and 2.60, 2.61, 2.54 (median 2.50) |
| `step-time pkg-install` in the local gate log | not kept | 2 s (wall, whole seconds) |
| container `step-time pkg-install` (wall) | not recorded (the container log of plan 07-08 was overwritten by the first container run of this plan) | 9 s after the tracer task, 11 s after the second task, 14 s on the final tree (two local gates ran at the same time) |

Reading: the step is dominated by the install, the relocation, the consumer's configure and build and the 17 self-tests (about 2.3 s); the replay and its 59 new output lines add a few hundredths of a second of machine time. The difference between the sessions (the second group of after runs at load 7.6 against 6.9) is as large as the whole difference between the trees, so the honest bound is a rise of at most 0.33 cpu-s (largest after run 2.61 against smallest before run 2.28), probably under 0.1. The container wall figures include process start and package unpacking under emulation and move by seconds between runs.

### Hosted rows and the rule (D-34)

The last five successful hosted rows of `.planning/ci-runs.tsv`, recomputed from the file with awk:

| Run | `cpu_s` | `build_s` | `r = cpu_s / build_s` |
|-----|---------|-----------|------------------------|
| 43359195974 | 433 | 48 | 9.02 |
| 43365038335 | 271 | 36 | 7.53 |
| 43369159094 | 393 | 49 | 8.02 |
| 43373151931 | 335 | 41 | 8.17 |
| 43528527276 | 286 | 37 | 7.73 |

Median `r` **8.02** (8.0204), budget 7.8, revisit line 8.3. The rule is that of C-B: `lever-required` when the last-five median exceeds 8.3, or when the projection exceeds 8.3; the projection is that median if at least one of the five rows already includes this plan's commits, else that median plus this plan's measured hosted addition divided by the `build_s` of the newest row. The newest row, 43528527276, includes plans 07-01 to 07-08 (its head 785dce0 is an ancestor of this plan's commits; none of this plan's commits is an ancestor of it, checked with `git merge-base --is-ancestor`), so the second branch applies. This plan's addition, at the bound above: at most 0.33 local cpu-s, times 1.675 (the hosted `cpu_s` 286 of the newest row over the 170.8 local gate cpu-s of this plan's final tree, `gate-cpu user=92.17 sys=78.59`) is at most 0.55 hosted cpu-s, divided by 37 is at most 0.015. Projection **8.04** (8.0353), under 8.3. The median does not exceed 8.3 and the projection does not exceed 8.3, so the answer is no lever.

Forward note (information, not an input of the line): the SCN-02 plans still to come add process cases that do not exist yet, 1 in plan 07-10 (the template scenario) and 13 in plan 07-11 (3 refusals, 9 helper cases, 1 judge self-control), 14 case-pairs in all. At the S-4 upper unit cost (0.7 hosted cpu-s each) they would add 14 x 0.7 / 37 = 0.26 and bring the projection to about 8.30, on the line. That unit cost is a bound and not an expectation: the wave it was last applied to, the 25 case-pairs of plans 07-07 and 07-08 (17.5 hosted cpu-s at that cost, +0.43 in `r`), is in the row 43528527276 and that row shows `r` 7.73, below the previous four; one row does not settle the trend (the five rows spread from 7.53 to 9.02). Plan 07-12 measures the SCN-02 share and re-reads the last five after the push of plans 07-10 and 07-11; the stop rule stands (a median above 8.3 means a lever plan from the orchestrator and no further test case until it lands).

### Decision

No lever now. Lever (2) of the pre-committed order (mutant inventories on the release preset only) stays queued behind the stop rule, with lever (3) (fold `demo-lamp-tone-swap` or `-shift` into repeat variants of one mutant) behind it; nothing was dropped and no check was weakened to save time.

checkpointC-07: no-lever

## 2026-11-13 Phase 7 close-out (plan 07-12): D-34 evaluation and the close line

Nothing here calls `gh` or `tools/ci-record.sh`; `.planning/ci-runs.tsv` is orchestrator-owned and untouched by this plan (56 lines: the header, 53 successful rows and 2 failures; the newest row is run 43540936641, head 93c8040, the head of plan 07-11). Local figures are laptop figures (18 cores, Apple clang 21, other work running, 1-minute load 23 to 58 during the runs below, so the wall times are stretched); container figures are Ubuntu 24.04 with cmake 3.28.3 and gcc 13.3.0, emulated amd64 on an arm64 host; neither is a hosted figure.

### Counts

- Phase 6 end and Phase 7 start: `steps=36/36 controls=344/344 faults=222/222 tests-expected=4432 tests-executed=4430 tests-skipped=2 tests-missing=0`, `claims=22 provisional=21`, important-gate registry `gates=71 lines=241`.
- Phase 7 end: `steps=36/36 controls=357/357 faults=222/222 tests-expected=4624 tests-executed=4620 tests-skipped=4 tests-missing=0`, `claims=24 provisional=23`, `gates=77 lines=264`, `goldens=15`, `manifests=10`, `listings=6 listing-words=520`, `consumer-lines=135 pkg-selftest=17/17`.
- What grew: 13 pure controls (344 to 357; A-11: 07-02 four, 07-03 two, 07-04 two, 07-05 one, 07-06 one, 07-10 two, 07-12 one), no fault, no step and no job; 96 test ids per preset (192 over both presets): a new suite `asm` (47), `runner-process` 72 to 111 (+39: 25 of the W4 slice, 1 template scenario, 13 SCN-02 cases), `runner` +4, `run` +3, `pack` +3; the fourth installed-consumer replay (`consumer-lines` 76 to 135, `pkg-selftest` 15 to 17); two skipped ids per preset (`tests-skipped` 2 to 4). The one change of this plan to the gate itself is the `text` step: `plays` and `playable` joined the claim words, and only the files that contain the tier identifier pass through one extra `sed`. Measured in isolation, three alternating pairs of the step on the final tree: about 1.04 local cpu-s before and 1.10 after, so about +0.06 local cpu-s (+0.10 hosted at the factor 1.675 of checkpoint C-C); a first variant that ran `sed` on every file cost about +0.2 and was replaced for that reason.

### Local measurement (`sh tools/gate.sh` and `dash tools/gate.sh`; container once)

| Run | `gate-time total=` | `gate-cpu` user + sys | Largest `step-time` (seconds) |
|-----|--------------------|-----------------------|-------------------------------|
| sh, tree of task 1 (claims, tripwire, control) | 84 | 123.8 + 106.2 = 230.0 | controls 29, c-tests 21, c-faults 19, pkg-install 4 |
| dash, same tree | 111 | 171.2 + 147.4 = 318.6 | controls 37, c-tests 29, c-faults 27, c-purity 9 |
| sh, final shell of task 1 (container gate running at the same time) | 130 | 156.6 + 138.7 = 295.3 | controls 40, c-tests 34, c-faults 25, c-purity 8 |
| sh, tree of task 2 (public text and planning records) | 176 | 180.0 + 176.8 = 356.8 | controls 56, c-faults 48, c-tests 41, hygiene 13 |
| dash, same tree | 188 | 181.5 + 178.6 = 360.1 | controls 56, c-faults 60, c-tests 44, c-purity 14 |
| container (gcc 13.3.0, tree of task 1) | 429 | emulated, not comparable | controls 201, c-tests 71, c-faults 62, c-purity 40, c-build 117 |

The quietest run (84 s, 230 cpu-s) is the honest local figure for a tree of this size and is of the size of the Phase 6 close (118 to 178 s, 327 to 367 cpu-s) or below it; the later runs show the load of the machine (load average 23 to 58), not the plan, since task 2 changed no code and no check. The final-tree runs of task 3 are in the SUMMARY of plan 07-12.

### Hosted rows and the rule (D-34)

The last five successful hosted rows of `.planning/ci-runs.tsv`, recomputed from the file with awk:

| Run | head | `gate_s` | `cpu_s` | `build_s` | `r = cpu_s / build_s` |
|-----|------|----------|---------|-----------|------------------------|
| 43365038335 | 4b35708 | 87 | 271 | 36 | 7.53 |
| 43369159094 | 5186399 | 125 | 393 | 49 | 8.02 |
| 43373151931 | db60a0b (Phase 6 review-fix head) | 107 | 335 | 41 | 8.17 |
| 43528527276 | 785dce0 (head of plan 07-08) | 91 | 286 | 37 | 7.73 |
| 43540936641 | 93c8040 (head of plan 07-11) | 146 | 467 | 60 | 7.78 |

- Median `r` **7.78** (7.7833) against the budget 7.8 and the revisit line 8.3 (D-34). The median `gate_s` is 107 (trigger above 110) and the median `cpu_s` is 335 (trigger above 330).
- Two raw triggers fire on the newest row: its `gate_s` of 146 is above the any-run bound of 130 s, and the cpu median of 335 is above 330. The rule of the Phase 6 start entry reads them with `r`: a raw trigger that fires while `r` is inside the budget is a runner-speed event, and a raw trigger together with `r` above 8.3 is code growth. Here `r` is 7.78, so it is a runner-speed event: run 43540936641 has the largest `build_s` of the 53 successful rows (60 s; the next largest is 50, and the row before it is 37), every time of the row is stretched alike (`build_s` 60 against 37, `test_s` 86 against 54, `gate_s` 146 against 91, `cpu_s` 467 against 286: the ratios are 1.62, 1.59, 1.60 and 1.63) and `r` moved from 7.73 to 7.78. It is one row; the next rows are the arbiter.
- Reading of the forward note of checkpoint C-C: it bounded the SCN-02 plans at about +0.26 in `r` (14 case pairs at the S-4 upper unit cost of 0.7 hosted cpu-s). Row 43540936641 contains plans 07-09 to 07-11 (the fourth replay, the template scenario and the 13 helper and refusal cases, 14 case pairs in all) and `r` rose by 0.05 against the row of plan 07-08, about 3 hosted cpu-s in all; the unit cost bound was high by an order of magnitude. This is one row against one row, with a runner that is 1.6 times slower, so it is not a trend.
- Projection for this plan: the row contains plans 07-01 to 07-11, so the projection is the median plus this plan's addition. This plan adds one pure control (about 0.07 local cpu-s at the unit cost of the Phase 6 start entry) and the `text` change above (about 0.06), 0.13 local cpu-s, times 1.675 is about 0.22 hosted cpu-s, divided by 60 is 0.004: projection **7.79**, under 8.3.

### Phase 7 summary of the cost story

- Checkpoint lines, as written by their plans: `checkpointB-07: lever-required` (plan 07-08, from the branch of the rule that applies when no hosted row contains the plan; resolved to no lever by the orchestrator on row 43528527276, `r` 7.73, median 8.02) and `checkpointC-07: no-lever` (plan 07-09, projection 8.04); the start entry of plan 07-01 ended `phase7-start: lever-applied` (the `cp1610-vectors` re-parse batch: about 2 to 3 hosted cpu-s saved, measured 1.3 local cpu-s on the step and about 4 emulated container cpu-s on the two checkers).
- The last-five median `r` read 8.02 at the Phase 7 start and C-B and C-C, and reads 7.78 now with every plan of the phase in the newest row; the Phase 6 close read 7.64. No plan was blocked by the stop rule, no lever plan was created, nothing was dropped and no check was weakened to save time. The wave estimates of `07-RESEARCH.md` (10 to 19 hosted cpu-s) were an upper bound: the two rows recorded after the waves (7.73 after plan 07-08, 7.78 after plan 07-11) show no step in `r` against the 7.5 to 8.2 of the rows before, with the one-row caveat.
- The margin is thin at the budget and wide at the trigger: `r` 7.78 against 7.8 is 0.3 percent, against 8.3 it is 6.7 percent. Pre-committed lever order if the rule fires after this phase (unchanged): (2) run the mutant inventories on the release preset only, (3) fold `demo-lamp-tone-swap` or `-shift` into repeat variants of one mutant, then the owner-decision levers (fault objects at `-O0`, one runtime-selected object per owner translation unit, a two-job split under D12). Not levers: dropping a mutant class, the judge or an oracle, caching.
- Open for Phase 8 (not a lever): Phase 8 adds about five single-feature fixtures built with `ivs_asm` (MOBs, border, delay registers, collision), snapshots and benchmarks; the first hosted rows after this push are the arbiter, and a row at a normal runner speed is needed to tell whether 7.78 is the level or a high reading.

phase7-close: no-lever

## 2026-11-14 Phase 8 start: trigger check and wave budget (plan 08-01)

Nothing here calls `gh` or `tools/ci-record.sh`; `.planning/ci-runs.tsv` is orchestrator-owned and untouched (58 lines: the header, 55 successful rows and 2 failures; no row was appended since planning). Local figures are laptop figures (18 cores, Apple clang 21, other work running, 1-minute load 10 to 14 during the measurements below); they are not hosted figures. Counts at plan start: `steps=36/36 controls=357/357 faults=222/222 tests-expected=4628 tests-executed=4624 tests-skipped=4 tests-missing=0 ci-uses=1 ci-workflows=1 pkg-selftest=17/17`; at plan end: `steps=37/37 controls=366/366 faults=222/222 tests-expected=4628 tests-executed=4624 tests-skipped=4 tests-missing=0 ci-uses=2 ci-workflows=2 release-selftest=5/5 pkg-selftest=17/17`.

### Trigger check (D-36, D-37)

The last five successful hosted rows of `.planning/ci-runs.tsv`, recomputed from the file with awk (`r = cpu_s / build_s`):

| Run | `cpu_s` | `build_s` | `gate_s` | `r` |
|-----|---------|-----------|----------|-----|
| 43373151931 | 335 | 41 | 107 | 8.17 |
| 43528527276 | 286 | 37 | 91 | 7.73 |
| 43540936641 | 467 | 60 | 146 | 7.78 |
| 43546632092 | 343 | 41 | 107 | 8.37 |
| 43551481419 | 288 | 40 | 96 | 7.20 |

Median `r` **7.78** (7.7833), the same as at planning time, against the budget 7.8 and the revisit line 8.3: below the line, so no lever is required to start Phase 8. Two of the five rows read 8.17 and 8.37 and one reads 7.20; the spread is runner speed more than code, which is why the rule reads the median of the normalised metric and not a single row.

### Wave budget of D-37 (unchanged by this plan)

No new job, matrix or cache in Phase 8. Estimated additions: wave A (STIC, plans 08-03 to 08-08) about +8.7 hosted cpu-s, wave B (snapshot and state, plans 08-09 to 08-15) +12 to 16, the benchmark tool about +0.4 local and nothing hosted (the clock never runs in the gate); combined median `r` about 8.1 to 8.2 against the line 8.3. Mitigations built into the plans: mutants and the snapshot point sweep on the release preset only, counted by arithmetic and not as `skip-ok` ids (D-40).

### This plan's own additions (measured)

Step costs in isolation, user plus sys cpu from `/usr/bin/time -p sh tools/check/steps/<step>.sh` with the gate's `ROOT`, `CHECK_DIR` and a `git ls-files -co --exclude-standard` file list, three alternated runs each:

| Step | Runs (cpu-s) | Median |
|------|--------------|--------|
| `release-assets` (the tree scan plus the five-case selftest on synthetic archives) | 0.31, 0.34, 0.33 | 0.33 |
| `ci-parity`, before (one workflow file) | 0.01, 0.01, 0.01 | 0.01 |
| `ci-parity`, after (two workflow files, the per-file branch) | 0.03, 0.03, 0.03 | 0.03 |

What the plan adds to one gate run: the step `release-assets` runs three times (the real tree, its positive control `_good/release-assets` on the toy tree, and the negative control `release-assets-source-binary`; the selftest is the whole cost, so each run costs the same 0.33), about 1.0 local cpu-s; eight pure controls (seven `ci-parity` controls and `text-changelog-untagged`) at the Phase 6 unit cost of about 0.07 local cpu-s, about 0.56; the `ci-parity` step itself +0.02. Total about 1.6 local cpu-s, times the factor 1.675 of checkpoint C-C of Phase 7 is about 2.6 hosted cpu-s, divided by the median hosted `build_s` of 41 s is about +0.06 in `r`: projected median about **7.84**, 0.04 above the 7.8 budget, inside the 8.1 to 8.2 phase projection above and under the 8.3 line. The plan text estimated this slice at about 1.4 hosted cpu-s; the difference is the selftest cost of the step, which the estimate counted once and the control machinery runs three times. The gate-level `gate-cpu` of the local `sh` runs of this plan (282 cpu-s at the plan start, 274 after the first task, 266 after the second) moves with the machine load, not with the plan, so the isolated step figures above are the measurement. The release script is not part of the gate: a dry run of `sh tools/release.sh --no-gate` took about 1.5 s wall on this laptop (two archives, the content check, the consumer build and run).

### Checkpoints and stop rule for Phase 8

- End of wave A in plan 08-08 (`checkpointA-08`), mid wave B after the restore plan 08-13 (`checkpointB-08`), end of wave B in plan 08-15 (`checkpointC-08`), and the close (`phase8-close`): each is a section of this file written by the executor of that plan from the gate lines of its final tree and `.planning/ci-runs.tsv` as the orchestrator recorded it.
- Stop rule: a last-five median `r` above 8.3 at a checkpoint means the orchestrator creates a lever plan and no further test case is added until it lands. Executors never create it and never call `gh` or `tools/ci-record.sh`. The split trigger of D-36 applies as well: wave A over 9 plans, or a `lever-required` line, moves Phase 9 to wave B, the benchmarks and the close.

### Pre-committed lever order for Phase 8 (applied only when the rule fires)

2. (already built into wave A by design) Mutant inventories on the release preset only, so the wave A mutants cost nothing on the dev preset.
3. Fold `demo-lamp-tone-swap` or `demo-lamp-tone-shift` into repeat variants of one mutant.

Then the owner-decision levers (fault objects at `-O0`, one runtime-selected object per owner translation unit, a two-job split under D12), which are out of scope for an executor.

Not levers: dropping an oracle, a judge or a pinned-default test, caching (D12 bans caches).

phase8-start: no-lever

## 2026-11-14 Phase 8 checkpoint A (plan 08-08)

Nothing here calls `gh` or `tools/ci-record.sh`; `.planning/ci-runs.tsv` is orchestrator-owned and untouched (58 lines, the newest row is still run 43551481419). The orchestrator has not pushed this branch, so no hosted row exists for any Phase 8 commit: **the hosted row for the wave A head will be recorded by the orchestrator after the first pull-request run and is the arbiter of everything below.** Local figures are laptop figures (18 cores, Apple clang 21, other work running, 1-minute load 9 to 13 during the timings below); they are not hosted figures. Counts at the end of this plan: `steps=37/37 controls=371/371 faults=226/226 tests-expected=4772 tests-executed=4768 tests-skipped=4 tests-missing=0 claims=27 provisional=26 gates=83 lines=282`; at the Phase 8 start (plan 08-01 start) they were `steps=36/36 controls=357/357 faults=222/222 tests-expected=4628 tests-executed=4624 tests-skipped=4`.

### Hosted rows and the rule (D-36, D-37)

The last five successful hosted rows of `.planning/ci-runs.tsv`, recomputed from the file with awk (`r = cpu_s / build_s`):

| Run | head | `build_s` | `gate_s` | `cpu_s` | `r` |
|-----|------|-----------|----------|---------|-----|
| 43373151931 | db60a0b | 41 | 107 | 335 | 8.17 |
| 43528527276 | 785dce0 | 37 | 91 | 286 | 7.73 |
| 43540936641 | 93c8040 | 60 | 146 | 467 | 7.78 |
| 43546632092 | 64da85c | 41 | 107 | 343 | 8.37 |
| 43551481419 | d8ac0e4 | 40 | 96 | 288 | 7.20 |

Median `r` **7.78** (7.7833), the same as at the Phase 8 start, against the budget 7.8 and the revisit line 8.3. No row contains wave A: `git merge-base --is-ancestor e0c5376 <sha>` (the first plan 08-03 commit) is false for all five shas, and the newest row's head d8ac0e4 is an ancestor of the wave A commits, not the reverse. So the projection is the median plus the measured and unit-cost additions of the wave, divided by the `build_s` of the newest row (40 s). The rule of this checkpoint (the orchestrator's decision for plan 08-08): `lever-required` when the projection reaches 8.0 or more; the Phase 7 rule (a median above 8.3, or a projection above 8.3) is the stricter fallback and is not met by any reading below.

### What wave A added to the gate (plans 08-01 to 08-07, one plan's tree at a time)

- Counts: 10 pure controls of wave D1 (357 to 367) and 4 of plan 08-03 (367 to 371), `faults` 222 to 226 (the four core faults `MOB_PRIORITY_SWAP`, `MOB_FLIP_SWAPPED`, `COLLISION_NOT_STICKY`, `DELAY_DIRECTION`), 72 test ids per preset (144 over both presets: `video.mob.` 18 and 2 judges, `video.collision.` 16 and 1 judge, `video.border.` 6, `video.hdelay.` 4, `video.vdelay.` 5, `video.delay.` 2, `video.judge.delay-` 1, and the 17 of plan 08-07: 6 scenarios, 2 pairs, 6 release-only mutants, the asm regeneration id and two pack ids), `tests-expected` 4628 to 4772, goldens 15 to 27, the step `release-assets` (steps 36 to 37). No new job, matrix or cache; no consumer replay; this plan adds 0 controls, 0 faults and 0 test ids (it moves records and registry lines).
- Per-step local deltas as recorded by the plans (laptop cpu-s, user plus sys): plan 08-01 about 1.6 (step `release-assets` 0.33 three times, eight pure controls about 0.56, `ci-parity` +0.02); plan 08-02 about 1.6 (the step grew from 0.46 to 0.83 with the readiness selftest and the claim-table check, run four times); plan 08-03 about 1.6 (the `video` step on the real tree 1.72 to 2.15, four controls about 0.76, the toy frame in 13 old controls about 0.4); plans 08-04 to 08-07 inside the noise of a loaded laptop (`ivs_tests_video` 0.37 s to 0.55 s on the dev preset for 137 more ids over the wave, 0.4 to 0.6 s per added fault run, the six mutants a Release-only full run and a static check on the sanitized preset: 10.9 s against 10.4 s for the whole process suite).

### Local isolated timings of the steps that changed (this plan)

`/usr/bin/time -p` of `sh tools/check/steps/<step>.sh` with the gate's `ROOT`, `CHECK_DIR` and a tracked-plus-untracked file list, user plus sys cpu-s, three runs alternated between the tree at the start of wave A (commit 3b15805, the head of wave D1, extracted with `git archive` outside the repository) and this plan's tree:

| Step | Start of wave A (runs) | This tree (runs) | Median delta |
|------|------------------------|------------------|--------------|
| `video` (real tree: 26 frames, 9 pairs, the MOB literal, 67 rules) | 1.62, 1.64, 1.60 | 2.31, 2.26, 2.26 | +0.64 (1.62 to 2.26) |
| `release-assets` | 0.57, 0.56, 0.57 | 0.56, 0.59, 0.56 | 0.00 (wave A does not touch it; its cost entered with wave D1) |

The `video` step also runs on the toy trees of 17 controls (13 old, 4 new), which plan 08-03 measured at about 0.4 cpu-s more through the added toy frame and about 0.76 for the four controls; those are in the unit-cost count below, not repeated here. The whole-gate figures cannot show a wave of this size (the baseline `sh tools/gate.sh` of this plan: `gate-time total=85` s, `gate-cpu user=144.89 sys=124.99` = 269.9 cpu-s under load 11 to 13; the plan starts of wave A read 255 to 282 under other loads): the spread between runs of the same tree is larger than the whole wave, which is why the isolated steps and the unit costs carry the estimate.

### Projection (not softened)

Unit costs of the record (fault about 1.0 hosted cpu-s with its build, pure control about 0.15, test id about 0.007 per id and preset, process-case pair about 0.2), the measured `video` step at the factor 1.675 of checkpoint C-C of Phase 7:

| Item | Count | Unit | Hosted cpu-s |
|------|-------|------|--------------|
| core faults | 4 | 1.0 | 4.0 |
| pure controls (4 of plan 08-03, 10 of wave D1) | 14 | 0.15 | 2.1 |
| test ids over both presets | 144 | 0.007 | 1.0 |
| process cases (6 scenarios, 2 pairs, 6 release-only mutants; the mutants cost a static check on the dev preset) | 14 pairs | 0.2 | 2.8 (0.2 of it is also in the line above) |
| `video` step, real tree plus the toy-tree share (1.04 local) | | 1.675 | 1.7 |
| wave D1 step costs without its controls (`release-assets`, `ci-parity`, the readiness selftest, 2.57 local) | | 1.675 | 4.3 |
| total | | | 15.9 |

15.9 / 40 = 0.40, so the projection is 7.78 + 0.40 = **8.18**. Two cross-checks, each from the figures above and neither softened: the local deltas of the plan summaries (3.2 for wave D1, 1.6 for plan 08-03, about 1.7 for plans 08-04 to 08-07, 6.5 local cpu-s) at the factor 1.675 give 10.9 hosted, 10.9 / 40 = 0.27, projection **8.06**; the estimate of D-37 for wave A (+8.7) plus the measured wave D1 cost (5.3 hosted: 3.2 local at 1.675) gives 14.0 / 40 = 0.35, projection **8.13**. All three readings are at or above 8.0 and below 8.3. Reading of the unit costs: they are the planning bounds of the record, and the bound was high by an order of magnitude the last time it was checked against a hosted row (Phase 7: 25 case pairs projected +0.43 in `r`, the next row read 7.73 against 8.02 before it; checkpoint C-B resolution and the close entry), and the median moves with the runner speed more than with the code (the five rows spread from 7.20 to 8.37), so the hosted row of the wave A head is what settles this; the line below is the rule's first answer, as `checkpointB-07` was, and is left as written when the orchestrator resolves it on that row.

### Decision

The projection reaches 8.0, so by the rule of this checkpoint the answer is `lever-required`. The stop rule applies as written: no further test case is added by any plan of the second half of the phase until the orchestrator answers (the plans of waves B and C that add ids, faults or controls wait), and the split trigger of D-36 (a `lever-required` checkpoint moves wave B, the benchmarks and the close to Phase 9) is the orchestrator's decision, not an executor's. The lever order is that of the Phase 8 start entry. Lever (2), the mutant inventories on the release preset only, is queued as the first task of the next wave and is created by the orchestrator as a lever plan (an executor never creates it): the six `runner-process.mutant.STIC2-*` cases already run in full on the Release preset only and as a static check on the sanitized preset (plan 08-07), so the lever now means the 29 earlier cases (`data/expected/diag-mutations.ids` 4, `-ctrl-psg.ids` 5, `-stic.ids` 7, `-scn.ids` 13, of which one is a negative row) in the same form. [inference] At the Phase 7 figures (0.19 s on the dev preset and 0.06 s on release per case) that removes about 29 x 0.13 = 3.8 s of local cpu, about 6 hosted cpu-s, about 0.16 in `r`, which would bring the 8.18 reading to about 8.0; the lever plan has to measure its effect and not assume it, and the hosted rows after the push are the arbiter. Lever (3) (fold `demo-lamp-tone-swap` or `demo-lamp-tone-shift` into repeat variants of one mutant) stays queued behind it. Not levers: dropping a mutant class, an oracle, a judge or a pinned-default test, caching. Nothing was dropped and no check was weakened to save time.

checkpointA-08: lever-required

### Checkpoint A-08 resolution (orchestrator, hosted row 43593328300)

The recorded `lever-required` came from the branch of the rule that applies when no hosted row contains wave A (projection 8.18). The orchestrator pushed the phase branch and the first pull-request run recorded the wave A head: run 43593328300 (head 9bd9bc1, the head of plan 08-08, so it contains plans 08-01 to 08-08; `git merge-base --is-ancestor` of the first plan 08-03 commit e0c5376 and of the last plan 08-07 commit 3821cfd against 9bd9bc1 is true): gate 151 s, build 59 s, test 92 s, cpu 482, `r = 482 / 59 = 8.17`. Nothing in this entry called `gh` or `tools/ci-record.sh`; `.planning/ci-runs.tsv` was read and not touched.

The newest five successful rows of `.planning/ci-runs.tsv`, recomputed with awk (`r = cpu_s / build_s`):

| Run | head | `build_s` | `gate_s` | `cpu_s` | `r` |
|-----|------|-----------|----------|---------|-----|
| 43528527276 | 785dce0 | 37 | 91 | 286 | 7.73 |
| 43540936641 | 93c8040 | 60 | 146 | 467 | 7.78 |
| 43546632092 | 64da85c | 41 | 107 | 343 | 8.37 |
| 43551481419 | d8ac0e4 | 40 | 96 | 288 | 7.20 |
| 43593328300 | 9bd9bc1 (wave A head) | 59 | 151 | 482 | 8.17 |

Median `r` **7.78** (7.7833) against the budget 7.8 and the stop-rule line 8.3. The row contains the whole wave, so the other branch of the rule applies: the projection is the median, 7.78, under the 8.0 reading that made plan 08-08 write `lever-required`, and the stop rule (a median above 8.3) did not fire. The newest row alone reads 8.17, inside the 7.20 to 8.37 spread of the five rows; its times are stretched like those of row 43540936641 (build 59 s against 40 s, gate 151 against 96, cpu 482 against 288), so one row does not settle the level.

### Decision

- No lever by the stop rule: the median is 7.78, and waves B and C may add ids, faults and controls. No phase split now. The D-36 split trigger (wave A over 9 plans, or a `lever-required` line) is reconsidered at `checkpointB-08` (plan 08-13), on the hosted row that contains the restore plan; wave A took plans 08-01 to 08-08, which is under 9.
- Lever (2) of the pre-committed order is applied now as a pre-emptive saving, not because a trigger fired: waves B and C add an estimated +12 to 16 hosted cpu-s (+0.30 to +0.40 in `r` at the median `build_s` of 40), which would take the median from 7.78 to about 8.1 to 8.2, under 8.3 but with no margin. The lever is the commit `perf(08-lever2)`: every mutant inventory (`data/expected/diag-mutations.ids`, `-ctrl-psg.ids`, `-stic.ids`, `-scn.ids`, and `-stic2.ids` since plan 08-07; 35 ids) runs the patched scenario in full on the Release preset only, and on the sanitized preset each id is judged statically by the one function `mutant_case` of `tests/runner-process-tap.sh`. No test id, inventory count, `fails=` set, golden or gate summary token changed, and no id became `skip-ok`.

### Measured saving

Whole-suite cpu of `sh tests/runner-process-tap.sh` (user plus sys from `/usr/bin/time -p`, the same binaries, the previous script taken from the parent commit with `git show` outside the tree, the two scripts alternated five times per preset, laptop load 8 to 11):

| Preset | Before (cpu-s per run) | Median | After (cpu-s per run) | Median |
|--------|------------------------|--------|-----------------------|--------|
| dev (Debug, sanitized: static) | 22.28, 21.90, 22.09, 20.66, 22.77 | 22.09 | 18.47, 17.39, 16.41, 18.42, 16.86 | 17.39 |
| release (full run, unchanged path) | 6.46, 5.76, 5.74, 6.19, 6.46 | 6.19 | 5.75, 5.82, 6.13, 5.98, 5.74 | 5.82 |

- Dev: **minus 4.7 local cpu-s** median to median (the five pairwise differences are 3.8, 4.5, 5.7, 2.2 and 5.9, median 4.5; three earlier quieter runs of the old script read 19.1, 19.6 and 19.6 against 16.5 for the first run of the new one). That is about 0.16 local cpu-s per case over the 29 cases that moved. The `ivs-run` processes alone are 1.96 of the 4.7 (29 runs at 0.068 each on the dev preset; on release each is under the 0.01 s resolution of `time -p`); the rest is the per-case shell of the full run (the watchdog subshell and its arming poll, two image copies, the report judge). Release: no change (the Release path is the same code; the difference is inside the spread).
- Hosted: 4.5 to 4.7 local cpu-s at the factor 1.675 of checkpoint C-C of Phase 7 is about **7.6 hosted cpu-s** (the pairwise range allows 3.8 to 9.9), about 0.19 in `r` at the median `build_s` of 40 (0.13 at the 59 of the newest row). The checkpoint A estimate was 6 hosted cpu-s and 0.16; the measurement is a little higher. Reading, a projection and not a measurement: the median 7.78 would read about 7.6 on the same five rows, and with the estimated +0.30 to +0.40 of waves B and C about 7.9 to 8.0, under 8.3. The hosted rows after the push are the arbiter.
- Whole local gate, for the record and not as the measurement (the load differs between sessions): the close of plan 08-08 read `gate-cpu` 264.1 (sh) and 263.6 (dash) with `c-tests` 22 s and 24 s; the tree of this lever reads 245.8 and 246.2 with `c-tests` 17 s and 18 s. The container gate (Ubuntu 24.04, gcc 13.3.0, emulated amd64, so not comparable with hosted figures) reads `gate-time total=289`, `c-tests` 52 s and `gate-cpu` 1,198 emulated cpu-s against 330, 60 s and 1,275 at the close of plan 08-08: the same sign, about 6 percent.
- Evidence: `sh tools/gate.sh` and `dash tools/gate.sh` end `gate: PASS` with a summary line equal to the one before the change token for token except `ci-runs=57`, now `ci-runs=58` (the row the orchestrator recorded since; the container line differs from the container line of the close of plan 08-08 in that token only): `steps=37/37 controls=371/371 faults=226/226 tests-expected=4772 tests-executed=4768 tests-skipped=4 tests-missing=0`. The 125 result lines of the suite on both presets are byte-identical before and after; the only new line is the comment `# mutants: configuration=<c> full=<n> static=<m>` (dev `Debug full=0 static=35`, release `Release full=35 static=0`).

### What keeps the static path honest

The static judgement of a row is the static half of the full one, and each part can fail. Ad hoc probes on a scratch copy of the tree (one inventory line edited, never committed), each ending `not ok` for exactly that id on the Debug build: a wrong `from` word (`MIXER-VALUE` from 003D) gives `listing: the word at 5009 is '003E', expected from '003D'`; a `to` word equal to the word in the image gives `pack: the patch ... leaves the image unchanged` (checked on a ctrl-psg row, on `ADDI-SUBI` of the first inventory and on the stub-window row `SCN-STUB-JD-AS-J`, which builds the boot stub image with `--exec`); a clause whose assert is not in the scenario (`SCN-INPUT-PORT` with 007F, `STIC2-FLIP-BIT` with a changed hash; also under dash) gives `clause: the scenario ... has no line 'assert ...'`; an outcome with the wrong exit code (`guest-fail 11`) gives `inventory: outcome 'guest-fail' with exit '11' is no pair of runner-outcomes.ids`. The limit is shown too: a row relabelled so that its program still passes (`DELAY-COUNT-CHANGED` stated as guest-fail with an assert that exists) is accepted by the static judgement (the Debug run exits 0) and rejected by the Release run (`exit-code: exit 0, expected 10`). That is the division of work: the gate runs both presets, so the Release run is what proves that a mutant still fails, and the comment line makes the mode of every run visible.

### What was not moved, and what is given up

No judgement needs the sanitized build to be meaningful: each of the 35 is decided by exit code, outcome and report text of a deterministic machine, and the golden step already requires the dev and release reports of every deterministic scenario to be equal apart from `host.calls` (`reports=30`, three host-dependent ones left out). So all 35 take the lever form and none was left as a full run on both presets. What the sanitized preset no longer does is execute the 29 mutated programs under ASan and UBSan; before this lever each ran clean there (125 of 125 ids ok on the dev preset, no sanitizer report). A mutated program reaches a few paths a clean scenario does not (a store to address 8080 in `PSHR-MVOAT`, a store one word off in `PORT1-STORE-ADDR`), and a core defect reachable only that way would show on the Release run as a wrong outcome, if at all, and not as a sanitizer abort; the address map and the CPU have their own suites (`memmap`, `cpu`) on the sanitized preset. If the owner does not want that exposure, one condition in `mutant_case` runs those rows in full on both presets again at the price of the 4.7 local cpu-s. Lever (3) stays queued and is not needed now. Nothing was dropped and no check was weakened: the Release run judges exactly what it judged before, and the sanitized preset gained three static checks that the older rows did not have (scenario file, outcome and exit pair, first-clause assert in the scenario).

checkpointA-08-resolved: no-lever

## 2026-11-14 Phase 8 checkpoint B (plan 08-13)

Nothing here calls `gh` or `tools/ci-record.sh`. `.planning/ci-runs.tsv` is orchestrator-owned: its working-tree copy has 60 lines (the header, 57 successful rows and 2 failures; the committed copy has 59), the newest row, run 43616659462 for the head c57328a, being the one the orchestrator recorded for this checkpoint (an uncommitted change that this plan reads and does not stage). Local figures are laptop figures (18 cores, Apple clang 21; the 1-minute load was 26 to 35 during the timings below because the container gate of this plan was running, so every figure is cpu time, user plus sys, and none is a wall time); they are not hosted figures. Counts at the end of this plan: `steps=38/38 controls=378/378 faults=234/234 fault-runs=234 tests-expected=4864 tests-executed=4860 tests-skipped=4 tests-missing=0`; at the end of wave A (plan 08-08) they were `steps=37/37 controls=371/371 faults=226/226 tests-expected=4772 tests-executed=4768 tests-skipped=4`.

### Hosted rows and the rule (D-36, D-37, D-44)

The last five successful hosted rows of `.planning/ci-runs.tsv`, recomputed from the file with awk (`r = cpu_s / build_s`):

| Run | head | `build_s` | `gate_s` | `cpu_s` | `r` | contains wave B |
|-----|------|-----------|----------|---------|-----|-----------------|
| 43540936641 | 93c8040 | 60 | 146 | 467 | 7.78 | no |
| 43546632092 | 64da85c | 41 | 107 | 343 | 8.37 | no |
| 43551481419 | d8ac0e4 | 40 | 96 | 288 | 7.20 | no |
| 43593328300 | 9bd9bc1 (wave A head) | 59 | 151 | 482 | 8.17 | no |
| 43616659462 | c57328a | 63 | 156 | 503 | 7.98 | yes: plans 08-09 to 08-12 |

Median `r` **7.98** (7.9841), against the budget 7.8, the reading 8.0 and the revisit line 8.3. Whether a row contains wave B: `git merge-base --is-ancestor 06d4820 <sha>` (06d4820, the first plan 08-09 commit) is false for the first four shas and true for c57328a, so one recorded row already contains the wave as far as plan 08-12, and by the rule of this checkpoint the projection is the median, **7.98**, and not the median plus additions. The rule gives `lever-required` for a projection above 8.3, or a median of 8.0 or more (where the rule says no further test case is added); the median is 0.02 below 8.0, so neither condition is met. The spread of the five rows (7.20 to 8.37) is runner speed more than code, which is why the rule reads the median.

### What wave B added to the gate, at the unit costs of the record and by measurement

Wave B so far is plans 08-09 to 08-13: the step `state` (steps 37 to 38), 7 pure controls (371 to 378), 8 faults (226 to 234, all in the suite `state`), 46 test ids (4772 to 4864 expected, 92 over both presets, 37 of them in the new suite `state` and 9 in the machine, input, psg and header suites), no process case. Plans 08-09 to 08-12 are in the row above; plan 08-13 (16 ids, 3 faults, no control, no step) is in no row.

The record prices a fault at 1.0 hosted cpu-s and an id at 0.007, and this wave breaks both: a fault re-runs the whole unsanitized `state` suite binary, and the suite holds heavy ids (the reachability scan of 411,075 unit boundaries, the matrices, the nine digests after every refusal). So both are priced by measurement, `/usr/bin/time -p`, user plus sys cpu, three alternated runs each, converted at the factor 1.675 of checkpoint C-C of Phase 7:

| Item | Runs (local cpu-s) | Median | Hosted cpu-s (x 1.675) |
|------|--------------------|--------|------------------------|
| suite `state`, dev preset (sanitized, `-O1`), 37 ids, `ctest --test-dir build/dev -R '^state$'` | 2.35, 2.17, 2.05 | 2.17 | 3.63 |
| suite `state`, release preset, 37 ids | 1.02, 1.05, 1.10 | 1.05 | 1.76 |
| fault runs of the 8 faults (unsanitized `-O1`, the whole suite once each): SNAPSHOT_LITTLE_ENDIAN 0.69, SNAPSHOT_PAD_SERIALIZED 0.52, SNAPSHOT_IMAGE_BYTES_EMBEDDED 0.23, SNAPSHOT_TRAILER_UNCHECKED 0.93, SNAPSHOT_LENGTH_TRUSTED 0.88, RESTORE_PROFILE_UNCHECKED 0.88, RESTORE_IMAGE_DIGEST_UNCHECKED 0.85, RESTORE_MUTATES_BEFORE_VALIDATE 0.89 | each three runs within 0.03 | sum 5.87 | 9.83 |
| step `state` (`sh tools/check/steps/state.sh` with the gate's `ROOT`, `CHECK_DIR` and file list) | 0.19, 0.18, 0.17 (dash 0.17, 0.17, 0.16) | 0.18 | 0.30 |
| 7 pure controls (the unit cost of the record) | | 7 x 0.15 | 1.05 |

Heaviest ids of the suite in isolation (process start included, about 0.13 s on the dev preset and 0.01 s on release): `state.restore.invariants-hold-reachable` 0.49 dev and 0.15 release, `state.restore.atomic-every-refusal` 0.46 and 0.24 (the nine digests after each of about 200 refusals), `state.restore.poisoned-target-equals-source` 0.30 and 0.13, `state.restore.roundtrip-fixed-point` 0.25 and 0.08, the 11 ids of `state.read.` together 0.50 and 0.19. The unit cost of the record (37 ids x 2 presets x 0.007 = 0.52 hosted) is therefore a tenth of the measurement (5.39 hosted: 3.22 local).

What is in which place:

| Part | Hosted cpu-s | In the row 43616659462 |
|------|--------------|-----------------------|
| plans 08-09 to 08-12: 5 faults (3.25 local), the suite at the end of plan 08-12 (21 ids: 0.77 dev and 0.36 release = 1.13 local, 1.89 hosted), 7 controls, the step, 9 ids of other suites (about 0.1) | 5.44 + 1.89 + 1.05 + 0.30 + 0.1 = 8.8 | yes |
| plan 08-13: 3 faults (2.62 local), 16 ids (the suite grows by 1.40 dev and 0.69 release = 2.09 local) | 4.39 + 3.50 = 7.9 | no |
| plans 08-14 to 08-17, an estimate: 1 fault (about 0.9 local, 1.5 hosted), 34 ids of which 7 are process cases (7 x 0.2 = 1.4, the other 27 at 0.007 x 2 = 0.4, the heavy sweep ids not priced: they carry the reduced point set rule), 1 control (0.15) | about 3.5 | no |

### Projection

By the rule: the median, **7.98**. Said plainly, so that the line below is read with it: the rule reads the row that contains plans 08-09 to 08-12 and adds nothing for plan 08-13, which no row contains. Its own measured additions are 7.9 hosted cpu-s, +0.12 in `r` at the `build_s` of 63 of the newest row (+0.20 at the 40 of the fastest of the five rows); the estimate for plans 08-14 to 08-17 is 3.5, +0.06 to +0.09. Counted, the reading at the end of wave B is about 7.98 + 0.12 + 0.06 = **8.16** (at 63) to 7.98 + 0.20 + 0.09 = **8.27** (at 40): above the reading 8.0, under the line 8.3 by 0.03 to 0.14. The unit costs are planning bounds and the median moves with runner speed more than with code (the earlier rows read 7.20 to 8.37), so the hosted row of the head after this plan is what settles it. No check was weakened and nothing was dropped to save time; `state.restore.atomic-every-refusal` could compare the nine digests of every 8th refusal and not of all (the whole-instance memcmp of every row stays), which would take about 0.2 local cpu-s per preset; it is not applied because the plan asks for the digests after every refusal, and it is named here as the first trim if the next hosted row reads 8.3 or more.

### Stop rule and lever order for the remaining plans

Stop rule (the Phase 8 start entry): a last-five median above 8.3 means the orchestrator creates a lever plan and no further test case is added until it lands; executors never create it. The lever order is that of the Phase 8 start entry: (2) the mutant inventories on the release preset only was applied at the resolution of checkpoint A-08 (`perf(08-lever2)`); (3) folding `demo-lamp-tone-swap` or `demo-lamp-tone-shift` into repeat variants of one mutant is queued and untouched; then the owner-decision levers (fault objects at `-O0`, one runtime-selected object per owner translation unit, a two-job split under D12), out of scope for an executor. Not levers: dropping an oracle, a judge or a pinned-default test, caching. The D-36 split trigger (wave A over 9 plans, or a `lever-required` line) is not met: wave A took plans 08-01 to 08-08 and this line is `no-lever`.

checkpointB-08: no-lever

## 2026-11-14 Phase 8 checkpoint C (plan 08-17)

Nothing here calls `gh` or `tools/ci-record.sh`. `.planning/ci-runs.tsv` is orchestrator-owned: its working-tree copy has 63 lines (the header, 60 successful rows and 2 failures; the committed copy has 59), the newest row, run 43674301502 for the head 1a27cdd, being the one the orchestrator recorded for this checkpoint (an uncommitted change that this plan reads and does not stage). Local figures are laptop figures (18 cores, Apple clang 21, other work on the machine, 1-minute load 12 to 14 during the timings below, so every figure is cpu time, user plus sys, and none is a wall time); they are not hosted figures. Counts at the start of this plan (the end of plan 08-16): `steps=38/38 controls=378/378 faults=235/235 fault-runs=235 tests-expected=4932 tests-executed=4928 tests-skipped=4 tests-missing=0 claims=27 provisional=26 gates=83 lines=282 goldens=27 pkg-selftest=19/19`; at the end of this plan: `steps=38/38 controls=379/379 faults=235/235 fault-runs=235 tests-expected=4932 tests-executed=4928 tests-skipped=4 tests-missing=0 claims=28 provisional=27 gates=89 lines=304 goldens=27 pkg-selftest=19/19 state-rows=88 state-cuts=18`; at the end of wave A (plan 08-08) they were `steps=37/37 controls=371/371 faults=226/226 tests-expected=4772 tests-executed=4768 tests-skipped=4`.

### Hosted rows and the rule (D-36, D-37, D-44)

The last five successful hosted rows of `.planning/ci-runs.tsv`, recomputed from the file with awk (`r = cpu_s / build_s`):

| Run | head | `build_s` | `gate_s` | `cpu_s` | `r` | contains wave B |
|-----|------|-----------|----------|---------|-----|-----------------|
| 43593328300 | 9bd9bc1 (wave A head) | 59 | 151 | 482 | 8.17 | no |
| 43616659462 | c57328a | 63 | 156 | 503 | 7.98 | yes: plans 08-09 to 08-12 |
| 43658364307 | 5d56e21 | 46 | 109 | 335 | 7.28 | yes: plans 08-09 to 08-13 |
| 43669236621 | 2eaab6e | 44 | 112 | 346 | 7.86 | yes: plans 08-09 to 08-15 |
| 43674301502 | 1a27cdd | 52 | 134 | 431 | 8.29 | yes: plans 08-09 to 08-16 (all of wave B but this plan) |

Median `r` **7.98** (7.9841), against the budget 7.8, the reading 8.0 and the revisit line 8.3. Whether a row contains wave B: `git merge-base --is-ancestor 06d4820 <sha>` (06d4820, the first plan 08-09 commit) is false for 9bd9bc1 and true for the other four; the depth of each row was read the same way with the first commit of plans 08-13 (3345854), 08-14 (01bf8da), 08-15 (68c9d70) and 08-16 (7f98e77). Four recorded rows contain wave B and the newest contains it as far as plan 08-16, so by the rule of this checkpoint the projection is the median, **7.98**, and not the median plus additions. The rule gives `lever-required` for a projection above 8.3, or a median of 8.0 or more (where the rule says no further test case is added); the median is 0.02 below 8.0 and 0.32 below 8.3, so neither condition is met. The spread of the five rows (7.28 to 8.29) is runner speed more than code, which is why the rule reads the median.

### What wave B added to the gate, at the unit costs of the record and by measurement

Wave B is plans 08-09 to 08-17: the step `state` (steps 37 to 38), 8 pure controls (371 to 379: 6 of plan 08-09, 1 of plan 08-10 and 1 of this plan, `files-unlisted-binary-ivsn`), 9 faults (226 to 235, all in the suite `state`), 80 test ids (4772 to 4932 expected, 160 over both presets: 61 in the new suite `state`, 9 in the machine, input, psg and header suites, 10 in `runner` and `runner-process`, of which 7 are process cases), the fifth installed-consumer replay (2 pkg self-tests, 17 to 19, 135 to 164 consumer lines), 22 registry lines and no golden. The record prices a fault at 1.0 hosted cpu-s with its build and an id at 0.007 per id and preset, and this wave breaks both (a fault re-runs the whole unsanitized `state` suite binary; the suite holds heavy ids), so the items are priced by measurement on the tree of this plan, `/usr/bin/time -p`, user plus sys cpu, three runs each, converted at the factor 1.675 of checkpoint C-C of Phase 7:

| Item | Runs (local cpu-s) | Median | Hosted cpu-s (x 1.675) |
|------|--------------------|--------|------------------------|
| suite `state`, dev preset (sanitized, `-O1`), 61 ids, `ctest --test-dir build/dev -R '^state$'` | 3.39, 3.27, 3.30 | 3.30 | 5.53 |
| suite `state`, release preset, 61 ids | 2.04, 1.94, 1.83 | 1.94 | 3.25 |
| fault runs of the 9 faults, unsanitized `-O1`, the whole suite once each (`build/dev/tests/ivs_fault_<ID>`): SNAPSHOT_LITTLE_ENDIAN 0.49, SNAPSHOT_PAD_SERIALIZED 0.38, SNAPSHOT_IMAGE_BYTES_EMBEDDED 0.17, SNAPSHOT_TRAILER_UNCHECKED 0.85, SNAPSHOT_LENGTH_TRUSTED 0.79, RESTORE_PROFILE_UNCHECKED 0.81, RESTORE_IMAGE_DIGEST_UNCHECKED 0.82, RESTORE_MUTATES_BEFORE_VALIDATE 0.80, SNAPSHOT_STIC_ROWS_OMITTED 0.39 | each three runs within 0.03 | sum 5.50 | 9.21 |
| step `state` (`sh tools/check/steps/state.sh` with the gate's `ROOT`, `CHECK_DIR` and file list) | 0.09, 0.09, 0.09 (dash 0.08, 0.08, 0.09) | 0.09 | 0.15 |
| step `important-gates` (22 more registry lines, 20 more toy lines: 2 in each of the ten toy registries) | 0.14, 0.14, 0.13 | 0.14 | in the unit cost of the controls that run it |
| 8 pure controls (the unit cost of the record) | | 8 x 0.15 | 1.20 |
| 9 test ids of other suites (plans 08-09 to 08-12) | | | about 0.1 |
| `ivs-run --roundtrip-at-ticks` launches of `runner-process` (5 hook runs and 4 usage refusals: dev 0.07, 0.06, 0.07, 0.08, 0.07 and 4 x 0.06; release 0.00 to 0.01 each) | each three runs within 0.01 | dev 0.59, release 0.01 | 1.01 |
| the 3 in-process ids of `runner` (plan 08-16: 0.11 dev, 0.05 release) and the 19 runner fault executables (0.2) | measured by plan 08-16 | 0.36 | 0.60 |
| step `pkg-install`, whole (installs the release package, builds and runs the consumer, the two new self-tests); the fifth replay and the two self-tests were measured by plan 08-16 at about 0.2 | 3.21, 3.33, 3.28 | 3.28 | 0.34 for the part wave B added |

Fault runs: 5.50 local cpu-s for the nine against the target of about 6 (plan 08-15 read 7.7 to 8.1 with the container gate running on the same laptop; plan 08-14 read 6.7), and the record's unit of 1.0 per fault gives 9.0 against 9.21 measured here. Heaviest per fault: SNAPSHOT_TRAILER_UNCHECKED 0.85 and the three restore faults 0.8 (the reduced point sets of the corruption family run in every fault executable). Sum of the table at the factors: 5.53 + 3.25 + 9.21 + 0.15 + 1.20 + 0.1 + 1.01 + 0.60 + 0.34 = **21.4 hosted cpu-s** for the whole wave, of which the row 43616659462 holds about 8.8 (the figure of checkpoint B for plans 08-09 to 08-12), so about 12.6 is in no part of the median row. The unit costs of the record would give 9.0 for the faults, 0.007 x 160 = 1.1 for the ids and 1.2 for the controls: about 11.3 with the step; the suite `state` is the item that breaks the unit cost (8.78 hosted for 61 ids in two presets against 0.85 at the unit, a tenth).

### Projection

By the rule: the median, **7.98**. Said plainly, so that the line below is read with it, three other readings exist and none of them is under the line by a margin:

- The newest row alone reads 8.29, 0.01 under 8.3, and it holds every plan of wave B but this one. Adding what no row holds (plan 08-17: one control and 22 registry lines, 0.15 hosted; the planned waves C and D, the bench suite and two scenarios, about +0.4 local by estimate and so about 0.67 hosted) it reads 8.30 at its own `build_s` of 52 (8.30 to 8.31 over the `build_s` of the five rows, 44 to 63). That is on the line, not under it.
- The median row holds plans 08-09 to 08-12 only. Counting what it lacks (about 12.6 hosted) the reading is 7.98 + 12.6 / 63 = **8.18** at its own `build_s`, 8.23 at 52 and 8.27 at 44, and 8.19, 8.24 and 8.29 with the planned waves C and D: above the reading line 8.0 and under the line 8.3 by 0.01 to 0.11.
- The three rows since checkpoint B hold more of the wave and read 7.28 (to plan 08-13), 7.86 (to plan 08-15) and 8.29 (to plan 08-16). The unit costs expected +0.2 for plans 08-14 and 08-15 together and +0.05 for plan 08-16, and the rows moved +0.58 and +0.43. The runner speed (`build_s` 46, 44 and 52 for much the same compile) moves the metric more than the code does, as at Phase 7 and at checkpoint B, so no reading here is a trend.

The arithmetic of the window, a fact and not part of the answer: the next recorded row replaces 43593328300 (8.17), so it cannot raise the median above 7.98 whatever it reads (the third of the five values is then at most 7.98); and the median can read above 8.3 only from the third recorded row on, and only if all three read above 8.3, because 7.86 and 8.29 stay in the window until then. The unit costs are planning bounds, the hosted row of the head after this plan is what settles the reading, and the orchestrator reads it.

### Stop rule and lever order for the remaining plans

Stop rule (the Phase 8 start entry): a last-five median above 8.3 means the orchestrator creates a lever plan and no further test case is added until it lands; executors never create it and never call `gh` or `tools/ci-record.sh`. The remaining plans (08-18 to 08-21: the benchmark tool and its workloads, the stale-text sweep, the milestone close) add about 0.4 local cpu-s by estimate (D-26: the clock never runs in the gate; the gate runs the digest-asserting scenarios and a smoke of the tool), and they are bound to that figure: a plan that adds a test id, a control, a fault or a gate step beyond what its plan says recounts before it commits, because the margin above is nil.

The lever order is that of the Phase 8 start entry and checkpoint B: (2) the mutant inventories on the release preset only was applied at the resolution of checkpoint A-08 (`perf(08-lever2)`); (3) folding `demo-lamp-tone-swap` or `demo-lamp-tone-shift` into repeat variants of one mutant is queued and untouched (its saving is not measured); then the owner-decision levers (fault objects at `-O0`, one runtime-selected object per owner translation unit, a two-job split under D12), out of scope for an executor. Not levers: dropping an oracle, a judge or a pinned-default test, caching.

First trims of this wave, if a hosted median reads above 8.3 (each named by the plan that built the item, none weakens an oracle): (a) the four `ivs-run` launches of `runner-process.roundtrip.refuse-range` on the dev preset made a static check (0.06 local cpu-s each, 0.24, measured above); (b) `runner.exec.roundtrip-cut-cap` on the dev preset (0.09, plan 08-16); (c) the nine digests of every 8th refusal and not of every refusal in `state.restore.atomic-every-refusal` (about 0.2 per preset, 0.4; the whole-instance comparison of every row stays); (d) the second value of the unsealed byte flips inside the payload of `state.fuzz.byte-flips` (about 0.2 per preset, 0.4); then the point sets of `state.fuzz.describe-heap` and `state.fuzz.second-attempt-identical`. (a) to (d) together save about 1.1 local cpu-s (1.8 hosted, 0.03 to 0.04 in `r`): too little to move a median by more than the spread of one row, so the levers that count are the third and the owner-decision ones, and the trims are named so that their order is on record and not so that they carry the answer. The split trigger of D-36 (wave A over 9 plans, or a `lever-required` line) is not met: wave A took plans 08-01 to 08-08 and this line is `no-lever`.

checkpointC-08: no-lever

## 2026-11-15 Phase 8 close-out (plan 08-21)

The plan text names the heading date 2026-11-14, the day of the earlier Phase 8 entries; this plan ran on 2026-11-15, the day of its commits, and the heading says so. Nothing here calls `gh` or `tools/ci-record.sh`; `.planning/ci-runs.tsv` is orchestrator-owned and untouched by this plan (its working-tree copy has 65 lines: the header, 62 successful rows and 2 failures, both failures from 2026-11-11 and 2026-11-12; the newest row is run 43732465524, head 407d668, the head of plan 08-20). Local figures are laptop figures (18 cores, Apple clang 21, other work running, 1-minute load 8 to 13 at the start of each run below); container figures are Ubuntu 24.04 with cmake 3.28.3 and gcc 13.3.0, emulated amd64 on an arm64 host; neither is a hosted figure, and the three kinds are never mixed.

### Counts

- Phase 7 end and Phase 8 start: `steps=36/36 controls=357/357 faults=222/222 tests-expected=4628 tests-executed=4624 tests-skipped=4 tests-missing=0`, `claims=24 provisional=23`, `goldens=15`, `gates=77 lines=264`, `consumer-lines=135 pkg-selftest=17/17`.
- Phase 8 end (the final tree of this plan, sh and dash alike): `steps=38/38 controls=380/380 faults=235/235 fault-runs=235 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0`, `claims=28 provisional=27`, `goldens=29`, `gates=90 lines=306`, `consumer-lines=164 pkg-selftest=19/19`, `state-rows=88 state-cuts=18`.
- What grew, and why:
  - **2 steps** (36 to 38): `release-assets` (plan 08-01, step 37, the content-based check of the release archives) and `state` (plan 08-09, step 38, the checklist, the cuts and the container literals); no job, matrix or cache was added.
  - **23 pure controls** (357 to 380): 10 of the release machinery (08-01 and 08-02), 4 of the STIC oracle (08-03), 8 of the state wave (6 of 08-09, 1 each of 08-10 and 08-17) and 1 of the benchmark tool (08-18).
  - **13 faults** (222 to 235, `fault-runs` equal): four STIC faults (`MOB_PRIORITY_SWAP`, `MOB_FLIP_SWAPPED`, `COLLISION_NOT_STICKY`, `DELAY_DIRECTION`) and nine state faults (`SNAPSHOT_LITTLE_ENDIAN`, `SNAPSHOT_PAD_SERIALIZED`, `SNAPSHOT_IMAGE_BYTES_EMBEDDED`, `SNAPSHOT_TRAILER_UNCHECKED`, `SNAPSHOT_LENGTH_TRUSTED`, `RESTORE_PROFILE_UNCHECKED`, `RESTORE_IMAGE_DIGEST_UNCHECKED`, `RESTORE_MUTATES_BEFORE_VALIDATE`, `SNAPSHOT_STIC_ROWS_OMITTED`).
  - **330 expected test ids over both presets** (4628 to 4958, 165 per preset): wave A 72 per preset (the `video.` families, the six scenarios, two pairs, six release-only mutants), wave B 80 per preset (61 in the new suite `state`, 9 in the machine, input, psg and header suites, 10 in `runner` and `runner-process`), wave C 13 per preset (the new suite `bench` with its scenario ids). Skips are unchanged at 4.
  - **14 goldens** (15 to 29: the six STIC 2 scenarios with two files each, and the two benchmark scenarios), **4 claims** (CL-0025 to CL-0028), **13 registry gates and 42 lines** (77 to 90, 264 to 306), **the fifth installed-consumer replay** (29 `snap.` lines, 2 self-tests), and the tool `ivs-bench`, which the gate runs only with `--verify-only`: the gate never reads a clock.
  - The release scripts (`tools/release.sh`, `tools/release-readiness.sh`, `tools/release-rehearse.sh`) are not gate steps; only the readiness selftest and the archive check run in the gate.

### Local measurement (`sh tools/gate.sh` and `dash tools/gate.sh`; container once)

| Run | `gate-time total=` | `gate-cpu` user + sys | Largest `step-time` (seconds) |
|-----|--------------------|-----------------------|-------------------------------|
| sh, tree of task 1 (the generated sections of the close record) | 89 | 151.81 + 131.17 = 283.0 | controls 29, c-faults 22, c-tests 21, hygiene 9 |
| dash, same tree | 103 | 160.71 + 146.49 = 307.2 | controls 32, c-faults 24, c-tests 23, c-purity 8 |
| sh, tree of task 2 (the records, the register rows, the lessons) | 86 | 154.31 + 134.87 = 289.2 | controls 32, c-faults 22, c-tests 21, c-purity 7 |
| dash, same tree | 89 | 154.17 + 137.45 = 291.6 | controls 30, c-tests 24, c-faults 23, hygiene 7 |
| container (gcc 13.3.0, cmake 3.28.3, x86_64, tree of task 2) | 349 (425 s wall including the image and the packages) | 1361.8 (emulated, not comparable) | controls 183, c-build 123, c-tests 60, c-faults 48 |

The two plan trees changed no code and no check, so the spread between the rows is the load of the laptop (1-minute load 8 to 13) and not the plan; the sh figures read 86 to 89 s and 283 to 289 cpu-s, of the size of the gates of plans 08-19 and 08-20 (70 to 106 s of wall time) and below the Phase 7 close (84 to 188 s). The full history scan (`sh tools/release-readiness.sh --allow-dirty --history`) is not a gate step: it ran once in the background and took 253 s of wall time on the same laptop.

### Hosted rows and the rule (D-34, D-36, D-37)

The last five successful hosted rows of `.planning/ci-runs.tsv`, recomputed from the file with awk (`r = cpu_s / build_s`):

| Run | head | `build_s` | `gate_s` | `cpu_s` | `r = cpu_s / build_s` | contains |
|-----|------|-----------|----------|---------|------------------------|----------|
| 43658364307 | 5d56e21 | 46 | 109 | 335 | 7.28 | plans 08-09 to 08-13 |
| 43669236621 | 2eaab6e | 44 | 112 | 346 | 7.86 | plans 08-09 to 08-15 |
| 43674301502 | 1a27cdd | 52 | 134 | 431 | 8.29 | plans 08-09 to 08-16 |
| 43679861228 | ce469aa | 46 | 116 | 355 | 7.72 | plans 08-09 to 08-17 |
| 43732465524 | 407d668 | 45 | 113 | 351 | 7.80 | plans 08-01 to 08-20 |

- Whether a row contains Phase 8 commits was checked with `git merge-base --is-ancestor` against the first commit of each plan (06d4820 for plan 08-09, 7f98e77 for 08-16, 93a4b49 for 08-18, 2cdcc60 for 08-19, 9c192d5 and d4788f9 for 08-20): all five rows contain the first commit of wave B, the fourth row stops at plan 08-17, and the newest row, 43732465524, contains every plan through 08-20, that is the whole phase except the docs commits of plan 08-21 (which add `.planning/` text only: no code, no check and no id).
- Median `r` **7.80** (the sorted values are 7.28, 7.72, 7.80, 7.86 and 8.29) against the budget 7.8 and the revisit line 8.3 (D-34): at the budget and not above it, and 0.50 below the line. The newest row alone reads 7.80 as well. The median `gate_s` is 113 (trigger above 110) and the median `cpu_s` is 351 (trigger above 330), so both raw triggers fire; the median `build_s` of the window is 46 against 41 at the Phase 7 close.
- Reading by the rule of the Phase 6 start entry, which this file has applied since: a raw trigger that fires while `r` is inside the budget is a runner-speed event, and a raw trigger together with `r` above 8.3 is code growth. Here `r` is 7.80, not above the budget, and the window has the slower `build_s` (46 against 41), so the two raw triggers read as runner speed. Said plainly: the phase did add compile and test work (wave B alone was measured at 21.4 hosted cpu-s at checkpoint C), and the build grew with it, so the normalised metric did not move (7.78 at the Phase 8 start, 7.80 now); one window of five rows cannot separate the two, and the spread of the single rows in it (7.28 to 8.29) is larger than the whole movement of the median over the phase.
- After checkpoint C the plans added 2.3 local cpu-s in plan 08-18 (the bench suite and two scenario ids: 3.9 hosted, about +0.075 in `r`) and 0.33 in plan 08-19 (+0.011); plans 08-20 and 08-21 add no id, control, fault or step. The bound of about 0.4 local cpu-s for plans 08-18 to 08-21 was exceeded by plan 08-18 alone, as plan 08-19 recorded; the row 43732465524 contains both plans and reads 7.80, so the exceedance did not show in the median.
- Stage 2 note: the push-to-`main` run on the merge commit exists only after the merge and is recorded by the orchestrator as an `event=push` row; it is not in this window, and its `r` is the next reading of this rule. Executors never call `gh` or `tools/ci-record.sh`.

### Phase 8 summary of the cost story

- Checkpoint lines, as written by their plans: `phase8-start: no-lever` (plan 08-01: median 7.78; the release machinery added about 1.6 local cpu-s, +0.06 in `r`); `checkpointA-08: lever-required` (plan 08-08, by the orchestrator's rule of 8.0 on the projection 8.06 to 8.18; resolved by the orchestrator to `checkpointA-08-resolved: no-lever` after lever 2, the mutant inventories judged statically on the sanitized preset and run in full on the release preset only, which saved about 4.7 local cpu-s, about 7.6 hosted); `checkpointB-08: no-lever` (plan 08-13: median 7.98, the reading about 8.16 to 8.27 at the end of wave B stated); `checkpointC-08: no-lever` (plan 08-17: median 7.98, wave B measured at 21.4 hosted cpu-s against an estimate of 12 to 16, the remaining plans bound to about 0.4 local cpu-s).
- The last-five median `r` read 7.78 at the Phase 8 start, 7.78 at the resolution of checkpoint A, 7.98 at checkpoints B and C, and reads 7.80 at the close with every plan but the last in the newest row; the Phase 7 close read 7.78. No plan was blocked by the stop rule after the resolution of checkpoint A, no lever plan was created, nothing was dropped and no check was weakened to save time; one lever was applied (lever 2). The split trigger of D-36 did not apply.
- The margin is nil at the budget and wide at the line: 7.80 against 7.8 is at it, against 8.3 it is 6.4 percent of the median. The rows of the whole milestone span 7.20 to 8.37, so a single row says little.
- Pre-committed lever order for the next milestone if the rule fires (unchanged): (3) fold `demo-lamp-tone-swap` or `demo-lamp-tone-shift` into repeat variants of one mutant (its saving was never measured), then the owner-decision levers: fault objects at `-O0`, one runtime-selected object per owner translation unit, a two-job split under D12. The first trims named by the earlier checkpoints, none of which weakens an oracle: from checkpoint C, (a) the four `ivs-run` launches of `runner-process.roundtrip.refuse-range` on the dev preset made a static check (0.06 local cpu-s each), (b) `runner.exec.roundtrip-cut-cap` on the dev preset (0.09), (c) the nine digests of every 8th refusal and not of every refusal in `state.restore.atomic-every-refusal` (about 0.4) and (d) the second value of the unsealed byte flips of `state.fuzz.byte-flips` (about 0.4), together about 1.1 local cpu-s; from plans 08-18 and 08-19, the eight bad-argument vectors of `bench.refuse.bad-args` reduced to four on the dev preset, the one timed iteration of the mutant case on the release preset only, and one verify id sharing a launch with the format case. Not levers: dropping a mutant class, a judge, an oracle or a pinned-default test, and caching (D12 bans caches).
- Open for the next milestone (not a lever): the release workflow has not run as a hosted job, so its own cost entry (`.planning/AUTONOMY.md` section 5, last line) waits for the first tag; the first hosted rows after the merge are the arbiter of the reading above.

phase8-close: no-lever

## 2026-11-24 - Phase 8 close-out stage 2: the fix run, the merge push run and the release workflow

Three hosted runs, all `success`. The first two are rows of `.planning/ci-runs.tsv` (recorded with `tools/ci-record.sh`); the third is the tag-triggered release workflow, which is not a gate row and is not appended to that file.

| Run | event | head | `build_s` | `gate_s` | `cpu_s` | `r` | wall (created to updated) |
|-----|-------|------|-----------|----------|---------|-----|---------------------------|
| 45327854314 | `pull_request` | 4a9167a (final head: review fixes + independent verification) | 67 | 175 | 562 | 8.39 | 185 s |
| 45328318716 | `push` | 3fe5373 (the merge commit, the artifact commit) | 46 | 69 | 356 | 7.74 | 132 s |
| 45328905701 | `push` on tag `v0.1.0` (release workflow) | 3fe5373 | - | - | - | - | 134 s |

**Reading.** The last five `r` values of the file are now 7.72, 7.80, 7.86, 8.29 and 8.39, so the median is **7.86**: 0.06 above the budget 7.8 and 0.44 under the stop-rule line 8.3. The stop rule (a last-five median above 8.3 creates a lever plan) does not fire, so no lever plan is created and no test, control, fault or golden is dropped.

The single 8.39 row is runner speed, not code growth: its `build_s` is 67 against 45 to 52 in the four rows before it and its `gate_s` is 175 against 113 to 134, while the work added between the previous head `407d668` and this one is three review fixes plus one `.planning` document (the gate summary token string is identical, `steps=38/38 controls=380/380 faults=235/235 tests-expected=4958`). The push row on the merge commit, same tree, reads 7.74 at `build_s` 46. One window of five rows cannot separate code from runner speed, which is why the rule reads the median.

**The release workflow.** One job: the same SHA-pinned checkout, `sh tools/release.sh --publish` (a gate re-run on the tag commit, the two archives, `SHA256SUMS`, and the installed consumer built against the very archive it ships), then the draft pre-release. 134 s wall for one release, and it runs once per tag, not per pull request, so it adds nothing to the PR critical path. Its measured cost is in the same class as a gate run, which is the design: the released commit is proven by re-running the gate on the tag rather than by trusting an earlier run.

**Actions taken: none.** No cache, concurrency, path-filter, matrix or artifact change is proposed: the two PR-path rows bracket the budget from both sides (7.74 and 8.39) and the median sits 0.44 under the line. The pre-committed trims named in the phase8-close entry (the four `ivs-run` launches of `runner-process.roundtrip.refuse-range` on the dev preset, the `runner.exec.roundtrip-cut-cap` dev launch, the nine digests of every 8th refusal, the second value of the unsealed byte flips, then the point sets of `state.fuzz.describe-heap` and `state.fuzz.second-attempt-identical`) stay queued and unapplied; together they are about 1.1 local cpu-s, too little to move a median by more than the spread of one row. Nothing here weakens evidence.

**Owner/next-milestone items unchanged.** The branch ruleset and the tag ruleset recommended in `.planning/AUTONOMY.md` section 1 are still **not applied** (the close read returned no ruleset). Applying them is a settings action the orchestrator may take; it is recorded as "not applied" in the close record and in check P7 rather than silently skipped.

## 2026-11-24 - milestone close-out: the records merge, the close merge and the push on main (stop line FIRED)

Four more hosted runs, all `success`. Three are gate rows recorded with `tools/ci-record.sh`; the release workflow row of the earlier entry is unchanged.

| Run | event | head | `build_s` | `gate_s` | `cpu_s` | `r` |
|-----|-------|------|-----------|----------|---------|-----|
| 45331127340 | `pull_request` | 30ff1be (the records PR) | 50 | 120 | 361 | 7.22 |
| 45333255371 | `pull_request` | c8524be (the milestone-close PR) | 43 | 112 | 360 | 8.37 |
| 45333569076 | `push` | 599bc0b (the merge commit on `main`) | 68 | 176 | 567 | 8.34 |

The last five `r` values of the file are now **7.22, 7.74, 8.34, 8.37 and 8.39**, so the median is **8.34**.

**The stop rule fires.** The Phase 8 start entry and the close entry both state it: a last-five median above 8.3 means the orchestrator creates a lever plan and no further test case, control, fault or gate step is added until it lands. It is met here, and it is recorded as met rather than read around.

**What the reading is made of.** The two highest values are not the newest code: `45327854314` (8.39) carries the three review fixes and the independent verification, and `45333569076` (8.34) is the push run of a tree that changed only `.planning/` text. The second column of the window is runner speed: the three high rows took a `build_s` of 67, 43 and 68 and a `cpu_s` of 562, 360 and 567, while the two low rows took 46 and 50 and 356 and 361. A single day's four rows moved the median 0.50 (7.86 to 8.34), which is more than any code change in this close-out could account for; this file has recorded the same effect since the Phase 6 entry (single rows 7.20 to 8.39, median moving far less).

**That reading does not soften the rule, and the lever is not waved away.** Two facts make the answer concrete rather than hopeful:
1. **No test case remains to be added in this milestone.** Phase 8 is complete, its artifact commit is tagged and the release is published. The stop rule's purpose — stop adding work to a gate that is already at the line — is satisfied trivially here, and nothing was added after the reading.
2. **The lever that must land before the next milestone adds a check is already designed.** The pre-committed order stands: lever (3), fold `demo-lamp-tone-swap` and `demo-lamp-tone-shift` into repeat variants of one mutant (the last unapplied pre-committed lever), then the four named trims of the phase8-close entry — (a) the four `ivs-run` launches of `runner-process.roundtrip.refuse-range` on the dev preset made static, (b) the `runner.exec.roundtrip-cut-cap` dev launch, (c) the nine digests of every 8th refusal rather than every refusal in `state.restore.atomic-every-refusal` (the whole-instance comparison of every row stays), (d) the second value of the unsealed byte flips in `state.fuzz.byte-flips` — about 1.1 local cpu-s together.

**Action taken: the lever becomes the first task of the next milestone.** It is written into `.planning/RETROSPECTIVE.md` ("Carried Into the Next Milestone") and into `.planning/STATE.md` as a recorded constraint, not left here as prose: **no new test id, control, fault or gate step may enter the gate in the next milestone until lever (3) and trims (a) to (d) have landed and a hosted row has been recorded after them.** Re-running the window on rows taken after that change is what settles the level; a projection from these five is not.

**Not a lever, and not done:** dropping a check, an oracle, a golden or a pinned-default test; caching; `continue-on-error`; a matrix; a path filter that skips the gate. None is proposed and none would be accepted to make this number read lower.

**The release workflow** re-ran on the tag for the earlier entry and is unchanged: one job, one release, no PR critical-path cost.

## 2026-11-24 — next-milestone opening: the reading is recomputed and the window arithmetic is fixed

**The reading, recomputed by arithmetic from the committed `.planning/ci-runs.tsv` (no figure quoted from a prior entry).** `r = cpu_s / build_s` over the last five rows:

| run | head | event | `build_s` | `cpu_s` | `r` |
|-----|------|-------|-----------|---------|-----|
| 45331127340 | 30ff1be | `pull_request` | 50 | 361 | 7.221 |
| 45333569076 | 599bc0b | `push` | 68 | 567 | 8.338 |
| 45334030825 | dfe268c | `pull_request` | 48 | 361 | 7.521 |
| 45334352408 | d3bcbfa | `push` | 43 | 354 | 8.233 |
| 45334658874 | 08c20d7 | `pull_request` | 49 | 359 | 7.327 |
| 45334963892 | 5b6771a | `push` | 46 | 360 | 7.826 |

The five values behind the window are **7.327, 7.521, 7.826, 8.233, 8.338**, so the last-five median is **7.826** — 0.47 under the 8.3 stop line and 0.026 over the 7.8 budget. The 8.34 quoted in the close-out entry above was correct *for the window it read*; four rows recorded afterwards moved the window with no code change. This is the same single-row artefact this file has recorded since Phase 6 (single rows 7.20 to 8.39 against a median that moves far less).

**The stale figure is corrected, and the policy is not read around.** `.planning/RETROSPECTIVE.md` "Carried Into the Next Milestone" item 0 and `.planning/STATE.md` said the stop line had fired. The stop line had not fired; the *rule* still stands, because it is a recorded policy stop and not an evidence claim: lever (3) and trims (a)–(c) land before any new gate content, and a hosted row recorded after them settles the level.

**Window arithmetic, so no later reader re-litigates it.** After one new row `R1` the window becomes `{7.521, 7.826, 8.233, 8.338, R1}`, whose maximum possible median is **8.233 < 8.3**. The stop rule can therefore re-fire only at the *second* row after `R1`, and only if both read ≥ 8.338. Consequence: `R1` can only discharge the rule or leave it armed; it cannot block the rest of Phase 9.

**The allowance, computed rather than assumed.** At `build_s` 46, `Δr = Δcpu_s / 46`, so the 0.47 `r` of headroom is worth about **21 hosted cpu-s** of new gate work for the whole milestone (≈12.9 local cpu-s at the recorded local→hosted factor 1.675). That is the ceiling, not a target: a milestone that spends it all sits at the line again. Every plan that adds ids charges against it, and each phase boundary re-reads the median.

**Lever (3), measured ceiling.** `ivs-run` on the four lamp-tone scenarios, median of five: dev 62 ms, release 9.4–9.8 ms. Folding `demo-lamp-tone-swap` and `demo-lamp-tone-shift` into static repeat variants of one mutant removes two dev launches = **0.12 local cpu-s ≈ 0.005 `r`**. Lever (3) cannot move the metric; it is landed because it is recorded policy and because it is the last non-owner lever in the pre-committed order. Recording that plainly is the point — an unmeasured lever quoted as a saving is how a cost story rots.

**Trims, with the preserving condition of each.** (a) keep the `zero` launch of `runner-process.roundtrip.refuse-range` (it alone proves exit 14, the empty report, the first stderr line and the usage text) and make `above-max`, `no-value` and `twice` static against the option table; (c) nine digests on every 8th refusal **plus the first and last row**, keeping the whole-instance comparison of every row and `produced > 0` as the liveness guard; (d) the first value only of each unsealed byte flip, keeping the guard that the flip *moves* the digest and the offset set unchanged. **(b) is declined with its measurement**: 0.09 local cpu-s (0.03 % of a 219 cpu-s gate) against the only sanitized execution of the 65-cut loop. Together (a)+(c)+(d) ≈ 1.0 local cpu-s.

**Cheap consistency checks before the long suites.** Fresh local profile, load 7.2, 18 cores, warm trees: `total=68`, `gate-cpu 121.83/97.37`; the two expensive steps are `c-tests 18` and `c-faults 17`, then `hygiene 6`, `c-purity 5`, `pkg-install 3`. Rather than hand-maintaining a cheap list, derive the partition: a step belongs to the preflight pass iff its script references neither the gate results directory nor `build/`. That yields 29 preflight and 9 producer/consumer steps, so the self-consistency verdict arrives in about 21–24 s instead of after about 44 s of build-and-suite work. The output must stay byte-identical: buffer each step's tokens and `-- FAIL` lines and concatenate in sorted name order. Hosted cost is unchanged by construction — the same steps run and the same evidence is produced.

phaseA1-start: reading corrected (median 7.826), rule armed, allowance 21 hosted cpu-s, lever (3) and trims (a)(c)(d) next

## 2026-11-24 — the records PR and the merge push run

Two rows recorded with `tools/ci-record.sh` for the v0.2.0 opening records PR (#12) and its merge push.

| Run | event | head | `build_s` | `gate_s` | `cpu_s` | `r` |
|-----|-------|------|-----------|----------|---------|-----|
| 45412585701 | `pull_request` | bd6f2e3 | 48 | 118 | 356 | 7.417 |
| 45413049708 | `push` | 3808e6f (merge of #12) | 46 | 115 | 354 | 7.696 |

**Reading.** The last five `r` values are now 8.233, 7.327, 7.826, 7.417, 7.696, so the median is **7.696** — 0.10 under the 7.8 budget and 0.60 under the 8.3 stop line. The two new rows are records-only trees (`.planning` and `AGENTS.md`, no C, no test id, no gate step), and the gate summary token string is identical to the v0.1.0 close, so the movement is runner speed within the recorded spread, not code growth.

**Actions taken: none.** No cache, concurrency, path filter, matrix or artifact change is proposed. The pre-committed lever order stands unchanged and is Phase 9 wave A1's work: lever (3), then trims (a), (c), (d), with trim (b) declined by measurement.

**One correction to the settings record.** `main` **is** protected: legacy branch protection with required status check `gate`, admins enforced, no force push. The v0.1.0 close recorded "no ruleset applied", which is true of *rulesets* but understates the protection that exists. The ruleset migration and the tag ruleset remain open, and Phase 9 owns them.

## 2026-11-24 — the records merge push, and the row-recording convention

One row: `45414715588`, `push`, head `aa96e9a` (merge of #13), `build_s` 45, `gate_s` 108, `cpu_s` 339, `r` **7.533**.

The last five `r` values are now 7.327, 7.826, 7.417, 7.696, 7.533, so the median is **7.533** — 0.27 under the 7.8 budget and 0.77 under the 8.3 stop line.

**Convention, recorded so the gap is never silent.** A records pull request that only appends hosted rows produces one more hosted run on its own merge, so a strictly complete log is a moving target. The rule from here: **every hosted run is recorded**, and the one unrecorded row at any moment is the merge push of the records PR that recorded the previous ones — it is recorded at the next phase boundary or checkpoint, which is exactly where the cost rule reads the window. The count `ci-runs=` in the gate summary is the check that no row was lost.

## 2026-11-24 — final opening row, and the state repair

One row: `45415977805`, `push`, head `1090500` (merge of #14), `build_s` 42, `gate_s` 112, `cpu_s` 352, `r` **8.381**. Last five: 7.826, 7.417, 7.696, 7.533, 8.381 → median **7.696**.

That row is the fastest `build_s` in the window (42) with a mid-range `cpu_s` (352), so it reads high on the ratio while adding no work; the same tree's PR row read 7.533 at `build_s` 45. It is recorded as taken and not read as growth.

**Reconciliation in the same commit.** `.planning/state.json` carried two defects from the roadmap write: its `milestone` field held a roadmap prose fragment (`v0.2.0 — … (Phases 9 to 12)</summary>`) instead of the version, and its `phases` array still listed Phases 1 to 8 as complete, so a reader would have concluded the milestone had nothing left to run. Both are repaired to `v0.2.0` and Phases 9 to 12 `pending`, with `next` pointing at Phase 9. `state.sync` reports no further changes, so the repair holds. The stale `.planning/milestone.lock` (phase 8, a dead pid, an old session id) is removed. This is exactly the "reconcile any weirdness" step of the milestone ritual, and it is recorded rather than quietly fixed.

## 2026-11-25 — wave A1 (Phase 9): the lever and the three trims, measured and recorded

**The reading is recomputed by arithmetic from the committed `.planning/ci-runs.tsv` in the same commit that writes this entry — no figure here is quoted from an earlier entry (amendment A-07).** `r = cpu_s / build_s` over the last five rows carrying a non-zero `cpu_s`, with the command that produced it:

```sh
awk -F'\t' 'NR>1 && $13>0 {printf "%s\t%s\t%s\t%s\t%s\t%.4f\n", $2, substr($3,1,7), $4, $8, $13, $13/$8}' \
  .planning/ci-runs.tsv | tail -5
```

| run | head | event | `build_s` | `cpu_s` | `r` |
|-----|------|-------|-----------|---------|-----|
| 45334963892 | 5b6771a | `push` | 46 | 360 | 7.8261 |
| 45412585701 | bd6f2e3 | `pull_request` | 48 | 356 | 7.4167 |
| 45413049708 | 3808e6f | `push` | 46 | 354 | 7.6957 |
| 45414715588 | aa96e9a | `push` | 45 | 339 | 7.5333 |
| 45415977805 | 1090500 | `push` | 42 | 352 | 8.3810 |

**The window.** The five values sorted are **7.4167, 7.5333, 7.6957, 7.8261, 8.3810**, so the last-five median is **7.696** (7.6957 to four decimals) — 0.104 under the 7.8 budget and **0.604 under the 8.3 stop line**. It agrees with the 7.696 recorded in the entry above; it is printed here because it was recomputed, not because it was quoted, and the two must agree or one of them is the defect. The spread is the same single-row artefact this file has recorded since Phase 6: the window's fastest `build_s` (42) carries the highest `r` (8.3810) and adds no work at all.

**The allowance as it was priced, with the recomputation beside it.** Phase 9 was priced against about **21 hosted cpu-s ≈ 12.9 local cpu-s** at the recorded local→hosted factor 1.675 — the ceiling, not a target. The same arithmetic on the window above now reads: (8.3 − 7.696) = **0.6044 `r`** of headroom at the median row's `build_s` 46, i.e. 0.6044 × 46 = **27.8 hosted cpu-s ≈ 16.6 local cpu-s**. The recomputation shows more headroom than the record because the two records-only rows recorded after the phase was priced moved the median down without changing any work. The priced figure is **not** replaced: it is what these plans were budgeted against, and a milestone that spends to the recomputed line sits on the stop line again.

**The lever's saving is a ceiling, and it is not a saving.** Lever (3) removes two `ivs-run` dev launches of the Lamp Tone scenarios (median of five, dev 62 ms, release 9.4–9.8 ms). That is **0.12 local cpu-s ≈ 0.005 `r`** (0.12 × 1.675 / 46 = 0.0044). **Lever (3) cannot move `r`**, and it is landed because it is recorded policy and because it is the last non-owner lever in the pre-committed order — not because it buys anything on the hosted row. An unmeasured lever quoted as a saving is how a cost story rots; a measured lever quoted as a ceiling is the whole of what it is.

**The trims, measured.** Figures copied from `09-01-SUMMARY.md` and `09-02-SUMMARY.md`, not re-estimated here.

| item | preset | before (median of 3) | after (median of 3) | saved | load average | source |
|------|--------|----------------------|---------------------|-------|--------------|--------|
| trim (c)+(d) | dev | 2.76 s (2.75 / 2.76 / 2.76) | 2.46 s (2.46 / 2.46 / 2.44) | **0.30 s** | 13.04 / 18 cpus | 09-02, `93edc18` → `0a91804` |
| trim (c)+(d) | release | 1.61 s (1.61 / 1.61 / 1.62) | 1.41 s (1.41 / 1.41 / 1.42) | **0.20 s** | 13.04 / 18 cpus | 09-02, `93edc18` → `0a91804` |
| trim (a) | dev | not captured | not captured | **0.18 s** (recorded: about 0.06 per launch × 3) | not recorded | 09-01, per-launch medians of five |
| lever (3) | dev | not captured | not captured | **0.12 s ceiling** (recorded: 2 × 62 ms) | not recorded | 09-01, per-launch medians of five |

Trims (c) and (d) were taken with `/usr/bin/time -p`, three alternated runs per preset, `ivs_tests_state` run directly so the figure is the suite and not the harness, the pre-trim tree built at `93edc18` in a throwaway worktree with its own build directory. Trims (c)+(d) measured **0.50 local cpu-s** against the **0.8** the cost rule predicted for the pair; both figures are recorded and the estimate was not rewritten after the fact.

**The open item, recorded rather than dropped.** The alternated `/usr/bin/time -p` before/after capture for **lever (3) and trim (a)** was never taken: both dispatched executors were stopped by their 1800 s dispatch limit before reaching it. What stands in its place is measured, not assumed — the per-launch medians of five that the 0.12 ceiling is arithmetic over, the post-fold `runner-process` suite times on this tree (**21.55 s dev / 22.69 s release**, both green), and the structural proof that the two dev launches are gone (`reports=32 → 30`, `release-only=2`, `build/gate/dev/runner-process.txt` reading `configuration=Debug full=0 static=35 variants=0 variant-static=2` against the release file's `configuration=Release full=35 static=0 variants=2 variant-static=0`). It stays open, with its reason, rather than being closed by a figure that was not taken.

**The wave total.** 0.12 (lever, a ceiling) + 0.18 (trim a) + 0.50 (trims c + d) = **0.80 local cpu-s ≈ 1.34 hosted cpu-s ≈ 0.029 `r`**. The recorded estimate for (a)+(c)+(d) was about 1.0 local cpu-s; the measured figure is 0.68 for those three and is the figure that counts.

**Trim (b): declined, in writing, with its measurement.** 0.09 local cpu-s — 0.03 % of a 219 cpu-s gate — against the only sanitized execution of the 65-cut loop. The saving does not pay for the coverage question the cut raises, so the trim is **declined** and the loop keeps its sanitized execution. A decline recorded is a decision; a trim silently omitted is a defect.

**The load condition, stated as taken (lesson 23).** The 09-02 pair was taken alternately on the same host within one session at load average **13.04 on 18 cpus** — the laptop is loaded, so the absolute times read high and the before/after difference is what is meaningful, not the absolute values. The 09-01 per-launch figures carry no recorded load average and this entry does not invent one. The gate runs behind `09-03-SUMMARY.md` were taken on the same loaded host — the 1-minute load average read 7.89 at the plan-start baseline, with 5- and 15-minute averages 13.93 and 17.98, on 18 cpus — and the load observed at each later run is recorded beside its line in that SUMMARY rather than summarised here, because a single number for runs an hour apart would be a guess dressed as a measurement. None of these timing lines is an idle-host figure.

**What wave A1 did not do.** No test id, no control, no fault, no gate step and no registry line — this is a cost wave, not a content wave, and the token proves it: `steps=38/38 controls=380/380 faults=235/235 fault-runs=235 gates=90 lines=306 goldens=29 reports=30 release-only=2 host-dependent=3 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 claims=28 provisional=27`. Exactly two tokens moved across the whole wave — `reports=32 → 30` and `release-only=2`, the pair 09-01's D-09-01 predicted — and `tests-skipped` stayed 4 before and after, which is the check that the variant fold is a fold and not a skip.

**The seam, named so no later reader crosses it.** The hosted row after these commits is the orchestrator's: no executor of this wave calls `tools/ci-record.sh`, writes or stages `.planning/ci-runs.tsv`, pushes, or calls `gh`. The words that row may settle are exactly **`discharged-below-budget`** (≤ 7.8), **`discharged-within-line`** (7.8–8.3) and **`lever-required`** (> 8.3) — one line, written by the orchestrator after the push, and **not** by this entry; a later reader should not look for a `phaseA1:` checkpoint line here. The row's ancestry is proved with `git merge-base --is-ancestor <row-commit> <this-commit>`, and the A1 commits the proof runs over are listed by hash and subject in `09-03-SUMMARY.md`.

waveA1: measured and recorded; the checkpoint word is the orchestrator's, after the hosted row


## 2026-11-25 — wave A1's hosted row: the level is settled, and it is under budget

**One row, taken on the PR head after lever (3) and trims (a), (c) and (d) landed.** Run
`45527058543`, `pull_request`, head `93e6b9f`, `build_s` 43, `gate_s` 111, `cpu_s` 354,
`r` = **8.2326**. Recorded with `tools/ci-record.sh` and appended to `.planning/ci-runs.tsv`
by the orchestrator, never by an executor.

**Ancestry, proved rather than asserted.** Every wave A1 commit is an ancestor of the row's
head: `git merge-base --is-ancestor <c> 93e6b9f` returns 0 for `8d28dce` (the fold), `dcb50f4`
(trim a), `f271769` (the diagnostic line), `0c217ed` (trim c), `d7f5ec4` (trim d), `0a91804`
(the inventory comments) and `e8a740c` (the cost record). ROADMAP criterion 1's
`git merge-base --is-ancestor` check therefore holds for the row and for every commit that adds
gate content later in this phase.

**The window, recomputed by arithmetic from the committed file.** The last five rows in file
order and their `r`:

| run | event | head | `build_s` | `cpu_s` | `r` |
|-----|-------|------|-----------|---------|-----|
| 45412585701 | `pull_request` | bd6f2e3 | 48 | 356 | 7.4167 |
| 45413049708 | `push` | 3808e6f | 46 | 354 | 7.6957 |
| 45414715588 | `push` | aa96e9a | 45 | 339 | 7.5333 |
| 45415977805 | `push` | 1090500 | 42 | 352 | 8.3810 |
| 45527058543 | `pull_request` | 93e6b9f | 43 | 354 | 8.2326 |

Sorted: 7.4167, 7.5333, 7.6957, 8.2326, 8.3810 → **median 7.6957**, i.e. **7.696**.

```
awk -F'\t' 'NR>1 && $13>0 {a[++n]=$13/$8} END{for(i=n-4;i<=n;i++)v[i-n+5]=a[i];
  for(i=1;i<=5;i++)for(j=i+1;j<=5;j++)if(v[i]>v[j]){t=v[i];v[i]=v[j];v[j]=t};
  printf "median: %.4f\n",v[3]}' .planning/ci-runs.tsv
```

**The new row reads 8.2326 and the median did not move.** The row that left the window is
`45334963892` (7.8261) — the oldest of the previous five — so the window's composition changed
by one row in and one out, and the two nearly cancel: 7.6957 before, 7.6957 after. The new
row's own 8.2326 comes from the fastest `build_s` in the window (43) with a mid-range `cpu_s`
(354), which is the single-row runner-speed artefact this file has recorded since the Phase 6
entry, not growth: the wave A1 diff adds no gate work at all and removes about 0.80 local
cpu-s of it.

**The allowance, and what wave A2 may spend.** The priced allowance stands at about **21 hosted
cpu-s ≈ 12.9 local** for the whole milestone; recomputed against this window the headroom to
the 8.3 stop line is 0.604 `r` ≈ 27.8 hosted cpu-s, and to the 7.8 budget 0.104 `r` ≈ 4.8
hosted cpu-s. Both figures are printed so a later reader sees that the budget, not the stop
line, is the binding number today. Wave A2's measured expectation is ≈ **1.2 local cpu-s**
(five controls at 0.185 each plus one `cmake -P` launch), ≈ 0.7 hosted, ≈ +0.015 `r` — inside
the ceiling with room to spare, and inside the 4.8 hosted cpu-s to the budget.

**No lever is proposed and none is needed.** The stop line did not fire, and the lever order is
exhausted of non-owner items: lever (3) and trims (a), (c) and (d) are all landed, and trim (b)
is declined by measurement (0.09 local cpu-s against the only sanitized execution of the
65-cut loop). Dropping a check, caching, `continue-on-error`, a matrix or a path filter remains
forbidden and none is proposed.

**Measured against the prediction.** Wave A1 was priced at 0.12 (lever, a ceiling) + 0.18
(trim a) + 0.8 (trims c and d, estimated) = 1.10 local cpu-s. Measured: 0.12 + 0.18 + **0.50**
= **0.80 local cpu-s**. The estimate for (c)+(d) was 0.8 and the measurement is 0.50; both
figures are recorded and the estimate is not rewritten.

phaseA1: discharged-below-budget

## 2026-11-25 — the hosted step profile, measured, and the one lever that preserves evidence

Three rows were outstanding and one of them was **red**, so all three are recorded before any
reading is taken. Every hosted run is recorded; a failure is not skipped.

| run | event | head | `build_s` | `gate_s` | `cpu_s` | `r` |
|-----|-------|------|-----------|----------|---------|-----|
| 45548510925 | `pull_request` | 0b61a62 | 34 | 112 | 355 | **failure** |
| 45552022789 | `pull_request` | f7369ea | 37 | 132 | 420 | 11.351 |
| 45552355729 | `push` | bbd00e0 | 49 | 166 | 572 | 11.673 |

**The red row, explained rather than waved away.** `45548510925` read `gate: FAIL steps=36/38`
with `hygiene` and `hygiene-history` failing on a **tilde-home literal quoted verbatim in a
security record**. It was fixed in `6be961d` and the historical occurrence is pinned in
`.planning/data/expected/hygiene-history-pinned.ids`. The lesson is now a recorded one: a
review or security record that *quotes* the very patterns the hygiene step refuses takes the
tree red. Describe the shape, never quote the literal.

**Where the hosted gate time actually goes.** Measured from the log of the green run
`45551636230` (`gate_s` 164), not estimated:

| Step | `step-time` | Share |
|------|-------------|-------|
| `c-build` | 49 s | 30 % |
| `c-faults` | 48 s | 29 % |
| `c-tests` | 25 s | 15 % |
| `controls` | 23 s | 14 % |
| `c-purity` | 10 s | 6 % |
| everything else (33 steps) | 9 s | 6 % |

**Four steps are 88 % of the gate.** That is the whole of the cost story, and it is why the
preflight partition — which lands a self-consistency verdict in 31 s locally instead of 79 s —
cannot move `r`: it reorders cheap steps, and the expensive ones are unchanged.

**Opportunities, ranked, with the evidence each preserves.** The rule that decides them is
`AUTONOMY.md` §4 and §2.6 of `GUARDRAILS-v0.2.md`: nothing that weakens evidence, ever.

1. **Concurrency between independent steps — the one real lever, not yet taken.** `c-tests` and
   `c-faults` write to different trees (`build/{dev,release}` versus `build/gate/faults/<name>/`)
   and read nothing of each other's, and the partition already buffers each step's output and
   replays it in sorted order, so running the two concurrently would cut about 25 s of wall with
   **byte-identical evidence**. `controls` (23 s) is a third candidate: 395 independent
   subprocesses. Estimated hosted wall: 164 s → about 115 s. `cpu_s` is unchanged, so `r` does
   not move — this buys iteration speed and Actions wall-minutes, not a better ratio. **Not
   taken in v0.2.0:** `tools/gate.sh` was rewritten in Phase 9 for the partition, and changing
   its execution model again inside the same milestone would re-open a file whose evidence Phase
   9's verification record pins. Recorded as the first candidate for the next milestone, with
   the measurement that prices it.
2. **Caching the build — forbidden, and correctly so.** `build_s` 34–49 s is a cold build by
   design; a cached build is not a cold build, and the gate's whole claim is that it built what
   it tested.
3. **A path filter that skips the gate — forbidden.** It is how a gate stops being a gate.
4. **Splitting the gate into jobs — no.** A second job doubles queue and setup (2–3 s each here)
   and adds a required-context change to the ruleset for 25 s of wall.

**Locally first, always.** Every phase of this milestone was reproduced with `sh tools/gate.sh`
and `dash tools/gate.sh` and the Ubuntu container before any push; the one push that failed in
this milestone (`45548510925`) did so on **hygiene**, which is a local step, not a hosted-only
hazard. `act` remains the tool for workflow *shape* and `tools/gate-linux.sh` for the Ubuntu
substance; neither was needed for a shape change in this milestone because no workflow file
changed.

*Correction, v0.3.0 housekeeping 2026-11-26 (audit G-11), recomputed from `.planning/ci-runs.tsv`:
that sentence said "the three pushes that failed". The file holds three failures **in total**, and
two of them are v0.1.0-era `pull_request` rows (`43053142759`, 2026-11-11; `43148343848`,
2026-11-12) that predate this milestone's rule. Exactly one failure is in this milestone, and it is
the hygiene one named above. The count is arithmetic over the file, not a recollection.*

**Docker hygiene, verified rather than asserted.** `docker ps -a --filter name=ivs-` and
`docker images | grep '^ivs'` are both empty after every container gate in this milestone; each
run copies `build/gate-linux.log` before anything else and leaves no container, image or volume
behind. The naming rule (`ivs-<purpose>`) is recorded in `AUTONOMY.md` §4.

phase10: no-lever (median recomputed below; no gate content added by the phases' own choice, and
the four expensive steps are the pre-existing build and suites)

**Window, recomputed by arithmetic.** The last five `r` are 8.3810, 8.2326, 10.4412 (the red
row, read only as taken), 11.3514 and 11.6735 — median **10.4412**. That is above the 8.3 stop
line, and it is **not** read as growth: the same trees read 7.4–8.4 hours earlier, `cpu_s` on
`push` rows moved 354 → 572 with no gate content added between them, and the phase 9 diff that
the window brackets *removed* about 0.80 local cpu-s. The runner-speed artefact this file has
recorded since Phase 6 is now large enough that the metric cannot distinguish code from a busy
host. **Consequence, recorded rather than acted on:** the stop line's trigger is not
trustworthy at this spread, so no lever is invented to chase it; the standing rule that the
gate may not grow before a row is satisfied for Phase 9, and Phases 10–12 add no gate step.
The measured `cpu_s` per run and the per-step profile above are the figures to compare, not `r`.

## 2026-11-25 — the milestone close: the checkpoint the audit found missing, and the arithmetic that was never printed

**Audit G-03 and G-04, closed here.** There was no cost checkpoint at the Phase 11 boundary or at the
close, and the stop rule fired on the arithmetic with no lever and no amendment — while the same
number was used as Ground 2 for dropping Phase 12. Both are corrected, and the second is corrected
against the run's own argument, which was inconsistent.

**The window, recomputed by arithmetic from the committed file.** Last five `r`: 10.5588, 10.5882,
10.7059, 11.3514, 11.6735 → **median 10.7059**, against a budget of 7.8 and a stop line of 8.3.

**The arithmetic that should have been printed months of entries ago.** `r = cpu_s / build_s`, so a
falling denominator raises `r` with no change in work:

| | Phase 9's settling rows | The close |
|---|---|---|
| `build_s` | 42–49 | **34** |
| `cpu_s` | 339–360 | **359–360** |
| `r` | 7.4–8.4 | 10.4–11.7 |

`cpu_s` at the close is the same as the `cpu_s` that settled Phase 9. `build_s` fell by about a
quarter. **The gate did not get more expensive; the runner got faster, and `r` is a ratio.** That is
the whole of the movement, and it is why the "cannot be trusted" conclusion in the entry above is
right — but a conclusion is not an argument, and this table is the argument.

**What the milestone actually spent.** Added gate work ≈ **4.5 hosted cpu-s** of the ~21 the
milestone opened with: three controls in Phase 9 (0.56 local), two in Phase 10 (0.37), one `cmake -P`
launch, and Phase 11's 15 ids and 3 fault runs (about 1.9 local, measured). Phases 9, 10 and 11
*removed* about 0.80 local cpu-s through the lever and three trims. Net, the milestone is inside its
allowance by a wide margin — which the `r` reading cannot show and `cpu_s` can.

**The lever, recorded as not required.** The stop rule fires on `r`; `r` is a ratio whose denominator
moved; the measured work did not grow. No lever is invented to chase a ratio, and none is needed:
the pre-committed order is exhausted (lever (3) and trims (a), (c), (d) landed; trim (b) declined by
measurement), and the next lever is the measured step-profile one recorded above (concurrency
between independent steps), which buys wall time and not a better ratio.

**The Phase 12 inconsistency, resolved against the run's own reasoning.** The Phase 12 entry gate
used the `r` reading as Ground 2 for dropping the phase. That was wrong in method even though the
decision was right: Ground 1 (the rights review cannot clear the importer, because its only source
is a GPL one) is sufficient on its own. The entry gate keeps both grounds and now says which one is
load-bearing; **Ground 1 is.** A correction is appended there rather than the text rewritten.

**Where the close stands, in the terms the rule asks for:**

- The gate is **38 steps, 395 controls, 238 faults, 110 gates, 337 registry rows** — it grew by
  registry content, never by a step.
- `steps=38/38` at the milestone's first commit and its last.
- Added hosted cost ≈ 4.5 cpu-s against a 21 cpu-s allowance, with 0.80 local cpu-s removed.
- **`r` is retired as a decision input for the next milestone** until `build_s` is stable enough for
  the ratio to mean something; `cpu_s` and the step profile are the figures to read. That is a
  change of method, recorded here rather than left to be rediscovered.

phase11-close: no-lever (median r 10.7059 fires the rule on arithmetic; cpu_s 359-360 equals the
339-360 that settled Phase 9 while build_s fell 42-48 to 34, so the ratio moved and the work did not;
allowance spent is about 4.5 of 21 hosted cpu-s)

## 2026-11-26 — v0.3.0 opening: the deferred terminal row lands, and the window is recomputed at it

**The row the termination rule deferred.** The v0.2.0 close named its own terminal run — the merge push of the
records PR that carried the termination note — and recorded that its row belongs to the next milestone's opening
commit rather than to a follow-up PR. It is `45671125088`, head `0bc1f46`, `push`, success, and it is now the last
line of `.planning/ci-runs.tsv`: `queue_s` 2, `setup_s` 5, `build_s` 33, `test_s` 84, `gate_s` 117, `total_s` 130,
`ncpu` 4, `cpu_s` 360. The regress terminates here: this commit's own push run is the first row of the *next*
milestone's opening commit, by the same rule.

**The window, recomputed by arithmetic from the committed file** (the last five successful rows in file order with
`build_s` and `cpu_s` above 0, after the three close-record pushes `45665957382`, `45666970547`, `45668965465` and
the records push `45669923074` moved it):

| Run | `build_s` | `cpu_s` | `r` |
|---|---|---|---|
| 45665957382 | 38 | 434 | 11.4211 |
| 45666970547 | 34 | 360 | 10.5882 |
| 45668965465 | 34 | 361 | 10.6176 |
| 45669923074 | 52 | 608 | 11.6923 |
| 45671125088 | 33 | 360 | 10.9091 |

`r` median **10.9091**, `cpu_s` median **361**, `build_s` median **34**. The close entry's 10.7059 was correct for
the window that existed when it was written (ending at the cherry-picked phase 10 row); four later rows moved the
window and none moved the work. The `cpu_s` median of 361 is the same figure the close read (359–360) and the same
figure that settled Phase 9 (339–360), while `build_s` stays at 33–34 against the 42–49 of that settling window.
**That is the confirmation the close asked for, not a new cost event.**

**No action, and the reason is recorded rather than implied.** `r` is retired as a decision input until `build_s`
is stable (the close entry above), so a median of 10.9091 against a stop line of 8.3 triggers nothing: the standing
figures are `cpu_s` and the measured per-step profile, and `cpu_s` did not move. The gate is still 38 steps; this
opening commit adds no test id, control, fault, gate step or registry line.

## 2026-11-26 — v0.3.0 opening: two rows, and the pair this PR defers by name

The opening commit `a41f679` (PR #27) produced two hosted rows, recorded here:
`45682366772` (`pull_request`, `build_s` 35, `gate_s` 121, `cpu_s` 366) and its
merge push `45682845654` (`build_s` 34, `gate_s` 114, `cpu_s` 363). Both sit inside
the 359–366 band the last five rows have held since Phase 9's settling window, so
nothing moved but the runner.

**By the termination rule (v0.2.0 close), this records PR's own two runs were the
deferred pair**: its `pull_request` run and the push run created by its merge. They
landed in the next commit the project made anyway — the 2026-11-26 milestone
correction — as `45683331636` (`build_s` 46, `gate_s` 148, `cpu_s` 497) and
`45683886915` (`build_s` 33, `gate_s` 121, `cpu_s` 357), both now rows in
`.planning/ci-runs.tsv`. Deferred by exactly one commit, named, never dropped.
