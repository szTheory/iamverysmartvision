---
phase: 04-headless-runner-and-installed-consumer
plan: 03
subsystem: faults, runner
tags: [faults, runner, outcome, D-17, D-20, D-24, RUN-02, RUN-04]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-02: steps=28/28, controls=241/241, faults=119/119 baseline"
provides:
  - "tests/faults.cmake: repo-relative owner TUs (core/ plus runner/), configure-time stem uniqueness, per-suite MODULES (default core)"
  - "faults-verdict site scan over ^(core/src|runner)/"
  - "runner/: ivs_runner_lib (links only ivs_core), outcome.c with the D-20 exit codes, names and precedence; ivs-runner-sources.txt in the build tree"
  - "suite runner (21 ids, both presets) and data/expected/faults/runner.ids (4 faults)"
  - "controls faults-orphan-site-runner, faults-missing-site-runner; _good/tree runner/good.c with site GOOD_RUNNER"
affects: [04-04, 04-17, 04-18, 04-19]

actuals:
  tokens: 10900
  tasks: 2
  commits: 2
plan_head_before: b263100f92c502ed4fb96af74db68e2b2b5a785d
plan_head_after: 64d8901

tech-stack:
  added: []
  patterns:
    - "A fault owner TU is a repo-relative path; its module (first path component) decides the plain/owner copy include path: core sees core/include, core/src and the generated dir; runner sees only core/include and runner/"
    - "An outcome value is its own exit code; a detail flag (0x100) separates the wall timeout from the budget timeout"

key-files:
  created:
    - runner/CMakeLists.txt
    - runner/ivs_runner.h
    - runner/outcome.c
    - tests/runner/runner_main.c
    - data/expected/tests/runner.ids
    - data/expected/faults/runner.ids
    - tools/check/controls/_good/tree/runner/good.c
    - tools/check/controls/_good/tree/build/gate/faults/GOOD_RUNNER/toy.txt
    - tools/check/controls/faults-orphan-site-runner/
    - tools/check/controls/faults-missing-site-runner/
  modified:
    - CMakeLists.txt
    - tests/CMakeLists.txt
    - tests/faults.cmake
    - tools/check/steps/faults-verdict.sh
    - data/expected/controls/faults-verdict.ids
    - tools/check/controls/_good/tree/data/expected/faults/second.ids
    - tools/check/controls/faults-duplicate-id/overlay/data/expected/faults/second.ids
    - tools/check/controls/faults-suite-missing/overlay/data/expected/faults/second.ids

key-decisions:
  - "Outcome constants equal their exit codes; IVS_RUNNER_OUTCOME_F_WALL (0x100) marks the wall timeout, so TIMEOUT_BUDGET = 11 and TIMEOUT_WALL = 0x10B; any other value is an internal error mapped to harness-fail (exit 14, name harness-fail)"
  - "ivs_runner_outcome_resolve takes an observation bit mask (6 bits); a bit outside IVS_RUNNER_OBS_ALL resolves to harness-fail (fail closed)"
  - "ivs_fault_suite MODULES accepts only `core` or `core runner`; a fault whose owner TU is outside its suite's modules fails configure"
  - "CONSUMER core:<src> of ivs_fault_gen keeps its core-relative syntax; faults.cmake maps it to core/<src>"

patterns-established:
  - "New owner modules join ivs_fault_modules in tests/faults.cmake plus ivs_fault_tu_setup; the stem check covers them automatically"

requirements-completed: []

duration: 40min
completed: 2026-11-11
---

# Phase 4 Plan 03: Repo-relative fault owners and the runner outcome taxonomy Summary

**The named-fault framework now takes owner TUs under `core/` and `runner/` (stem-unique, one owner, per-suite modules), `faults-verdict` scans runner sites, and the first runner TU `runner/outcome.c` fixes the D-20 exit codes and precedence with 21 tests, 4 faults and 2 controls; all 119 earlier faults still pass, also under gcc 13 in the Ubuntu container.**

## Performance

- **Duration:** about 40 min
- **Tasks:** 2 of 2
- **Files:** 24 changed (616 insertions, 38 deletions)
- **Suite `runner` plain-run cost (W7):** dev (ASan+UBSan) 0.06-0.07 s per ctest run (3 runs), release 0.00 s (3 runs); direct run `real 0.06` dev, `real 0.00` release. The four runner faults add 4 fault runs of the same 21-case suite.

## Counts

Start: `steps=28/28 controls=241/241 faults=119/119 tests-expected=2144`.
After Task 1: `steps=28/28 controls=241/241 faults=120/120 tests-expected=2168 tests-missing=0`.
End: `steps=28/28 controls=243/243 faults=123/123 fault-runs=123 tests-expected=2186 tests-executed=2186 tests-skipped=0 tests-missing=0` (2186 = 2144 + 21 ids x 2 presets), matching the plan's stated end counts.

## Accomplishments

- `runner/CMakeLists.txt`: explicit `IVS_RUNNER_SOURCES` (`outcome.c`), `ivs_runner_lib` STATIC linking `ivs_core` PUBLIC and `ivs_warnings ivs_san` PRIVATE (never `ivs_core_internal`), include dir `runner/`, `build/<preset>/ivs-runner-sources.txt` via the `file(WRITE)` + `file(COPY_FILE ... ONLY_IF_DIFFERENT)` idiom. Root `add_subdirectory(runner)` between `core` and `tests`.
- `runner/ivs_runner.h` + `runner/outcome.c`: `ivs_runner_exit_code`, `ivs_runner_outcome_name`, `ivs_runner_outcome_resolve` (harness-fail > missing-fixture > cancelled > timeout wall > timeout budget > guest-fail > pass). ISO headers only, no I/O, no statics.
- `tests/faults.cmake`: owner list `core/<IVS_CORE_SOURCES>` + `runner/<IVS_RUNNER_SOURCES>`; `faults: file stem <s> is used by <a> and <b>` at configure; owner scan over the combined list ("exactly one owner TU under core/ or runner/"); plain copies of every owner TU by stem; `ivs_fault_tu_setup` sets the include path by module; `ivs_fault_exe` and `ivs_fault_exe_core_gen` link the plain copies of the suite's modules only. The shared per-suite object library and the GEN_ path are unchanged.
- `tests/CMakeLists.txt`: `ivs_fault_suite(... MODULES ...)` (default `core`); ctest `runner` (`ivs_tests_runner`, `SKIP_RETURN_CODE 77`) and `ivs_fault_suite(runner SOURCES runner/runner_main.c INCLUDE_DIRS runner MODULES core runner)`. Runner fault executables land in `build/dev/tests/ivs_fault_<ID>`, where `c-faults.sh` already looks: `c-faults.sh` unchanged.
- `faults-verdict.sh`: site scan `^(core/src|runner)/.*[.](c|h)$`, header comment updated.
- Faults (`data/expected/faults/runner.ids`, `# suite=runner`): `OUTCOME_GUEST_AS_HARNESS`, `OUTCOME_TIMEOUT_AS_GUEST`, `OUTCOME_MISSING_FIXTURE_EXIT0` (each fails its `exit-*` case plus the allowed collateral `codes-distinct`; must-pass `exit-pass`), `OUTCOME_CANCEL_AS_TIMEOUT` (fails only `prec-cancelled-over-wall`; must-pass `prec-wall-over-budget`).
- Good tree: `runner/good.c` with `#ifdef IVS_FAULT_GOOD_RUNNER`, a `GOOD_RUNNER` row in `second.ids` (toy suite), `build/gate/faults/GOOD_RUNNER/toy.txt`; the row is copied into the `second.ids` overlays of `faults-duplicate-id` and `faults-suite-missing` so they report only their own token.

## Task Commits

1. **Task 1: tracer, generalized fault loop with one runner fault** - `12818eb` feat(04-03)
2. **Task 2: precedence cases, three faults, good-tree runner site, two controls** - `64d8901` feat(04-03)

## Task 1 proofs

- Tracer: `ivs_fault_OUTCOME_GUEST_AS_HARNESS` printed exactly one `not ok 2 runner.outcome.exit-guest-fail` of 12; the gate reported `gate: PASS steps=28/28 controls=241/241 ... fault-runs=120 ... faults=120/120 ... tests-expected=2168 tests-executed=2168 tests-skipped=0 tests-missing=0`, so all 119 Phase 2/3 faults still pass after the generalization.
- Duplicate-stem probe (a scratch copy of the tracked tree with `core/src/machine/machine.c` copied to `runner/machine.c` and listed in `IVS_RUNNER_SOURCES`; deleted afterwards):
  ```
  CMake Error at tests/faults.cmake:58 (message):
    faults: file stem machine is used by core/src/machine/machine.c and
    runner/machine.c
  ```

## Task 2 probes

- Fault runs (dev): GUEST_AS_HARNESS -> `codes-distinct`, `exit-guest-fail`; TIMEOUT_AS_GUEST -> `codes-distinct`, `exit-timeout`; MISSING_FIXTURE_EXIT0 -> `codes-distinct`, `exit-missing-fixture`; CANCEL_AS_TIMEOUT -> `prec-cancelled-over-wall` only.
- Each faults-verdict tree run alone: `_good` -> `-- faults-verdict pass faults=3/3`; `faults-orphan-site-runner` -> only `-- FAIL fault-orphan-site: IVS_FAULT_ORPHAN_RUNNER runner/good.c`; `faults-missing-site-runner` -> only `-- FAIL fault-missing-site: GOOD_RUNNER`; `faults-duplicate-id` and `faults-suite-missing` -> only their own token.
- Regression probe: a scratch copy of `faults-verdict.sh` with the old core-only scan fails `_good` and the orphan control with `fault-missing-site: GOOD_RUNNER`, so a revert of the scan cannot pass the control runner.
- `sh tools/check/run-controls.sh`: `controls=243/243`.

## Final gate summary lines

- `sh tools/gate.sh` (macOS, Apple clang 21): `gate: PASS steps=28/28 controls=243/243 ... fault-runs=123 ... faults=123/123 ... tests-expected=2186 tests-executed=2186 tests-skipped=0 tests-missing=0`; `gate-cpu user=39.66 sys=45.64`, `gate-time total=30`.
- `dash tools/gate.sh`: the same tokens, `gate: PASS`; `gate-cpu user=45.03 sys=55.49`, `gate-time total=34`.
- `sh tools/gate-linux.sh` (ubuntu:24.04 amd64 emulated on arm64, `gate-env ncpu=8 cc=GNU-13.3.0`): `gate: PASS steps=28/28 controls=243/243 cc=GNU ... fault-runs=123 ... faults=123/123 ... tests-expected=2186 tests-executed=2186 tests-skipped=0 tests-missing=0`, `gate-time total=158`, last line `gate-linux: exit 0 (log build/gate-linux.log)` (233 s wall with package installation; emulated times are not hosted times). First runner C under gcc: no warning, no failure.

## Deviations from Plan

### Auto-fixed / discretion

**1. [Rule 1 - plan text] Orphan-control EXPECT text**
- The plan's EXPECT `FAIL fault-orphan-site: ORPHAN_RUNNER` is not a substring of the step's actual line, which names the macro and the file (`fault-orphan-site: IVS_FAULT_ORPHAN_RUNNER runner/good.c`); the first control run failed `control-token-missing`. EXPECT is now `FAIL fault-orphan-site: IVS_FAULT_ORPHAN_RUNNER runner/good.c` (also pins that the site was found under `runner/`), plus FORBID `fault-missing-site:` so the control fails only for its own token.

**2. Duplicate-stem probe in a scratch tree copy**
- The plan asked for an untracked `runner/machine.c` in the working tree; the probe ran on a scratch copy of the tracked tree instead (the file must also be listed in `IVS_RUNNER_SOURCES` for the stem rule to see it), so the working tree and `runner/CMakeLists.txt` were never touched. Same configure, same error.

**3. Extra cases**
- Beyond the plan's list: `runner.outcome.prec-empty-is-pass` (the behaviour's empty-mask clause as its own id) and `runner.outcome.prec-single-and-unknown` (each observation alone; harness bit wins every mask; unknown bits fail closed). `exit-harness-fail` and `name-harness-fail` also pin the internal-error mapping of non-outcome values (1, 77, 0x100, 0xFFFFFFFF).

**4. `ivs_fault_suite` MODULES validation**
- Only `core` and `core runner` are accepted, and a fault whose owner lies outside its suite's modules fails configure (otherwise its executable would silently not contain the faulted TU).

## Open Items

- Plan 04-04 owns the remaining core-only places of hazard 2: the `c-build` unlisted-source guard (`build/dev/ivs-runner-sources.txt` is now written for it) and `integrity.cmake` SPDX/ASCII coverage of `runner/` (both new runner files carry SPDX and are ASCII).
- RUN-02 and RUN-04 stay Pending: the taxonomy is a tested pure function, but the executable, report and demonstrating scenarios (D-20 per-outcome scenarios) land in plans 04-17..04-19; `important-gates` (RUN-04) is later.
- Carried from 04-02: the orchestrator records hosted runs with `sh tools/ci-record.sh <run-id>` and applies D-45 after 04-10 and 04-19; PKG-03/PKG-04 Pending.
- Carried from 04-01/04-02: the job summary's "Skips and missing" table lists `pending-resolvers=17` and `unknown-fields=16`; `sh tools/hygiene.sh --history` still fails on earlier planning commits (orchestrator decision; this plan's commits add no home paths).
- Carried from 03-14: no hosted run yet observed for the Phase 3 post-verification fixes on this branch's lineage (agents never call `gh`).

## Threat Flags

None beyond the plan's threat model: T-04-08 mitigated (scan covers `runner/`, two controls, regression probe); T-04-09 (configure stem check, probe quoted); T-04-10 (`ivs_runner_lib` links only `ivs_core`; runner plain/owner copies see only `core/include` and `runner/`); T-04-11 (four OUTCOME_* faults, `codes-distinct` rejects 1, 77 and >125).

## Self-Check: PASSED

- Files present: runner/CMakeLists.txt, runner/ivs_runner.h, runner/outcome.c, tests/runner/runner_main.c, data/expected/tests/runner.ids, data/expected/faults/runner.ids, the good-tree runner files and both control dirs.
- Commits reachable from HEAD: 12818eb, 64d8901 (`git rev-list --count b263100..HEAD` = 2).
