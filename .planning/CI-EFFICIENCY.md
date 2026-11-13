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
