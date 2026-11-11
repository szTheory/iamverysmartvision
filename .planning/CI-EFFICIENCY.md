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
