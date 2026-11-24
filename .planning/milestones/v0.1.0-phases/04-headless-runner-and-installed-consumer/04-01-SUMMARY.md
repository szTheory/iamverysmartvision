---
phase: 04-headless-runner-and-installed-consumer
plan: 01
subsystem: ci
tags: [ci, measurement, job-summary, ci-parity, D-41, D-42, D-43, D-44, A-07, PKG-03, PKG-04]
status: complete

requires:
  - phase: 03-memory-map-content-loading-and-refusal
    provides: "gate.sh with step-time lines and the background control runner; ci-parity step and its eight controls; tools/gate-linux.sh"
provides:
  - "gate.sh prints `gate-env ncpu=<n> cc=<id>-<version>` and `gate-cpu user=<s> sys=<s>` before `gate-time` on every run"
  - "EXIT-trap job summary through tools/check/ci-summary.sh when GITHUB_STEP_SUMMARY is set (headline tokens, Skips and missing table, 8 slowest steps, first 40 FAIL lines fenced, backticks stripped); early exits summarized"
  - "Step ci-summary: in-step self-test of the helper, 11 cases pinned by data/expected/ci-summary-selftests.ids"
  - "ci-parity bans: action allowlist (actions/checkout only), secrets, strategy/matrix, runs-on other than ubuntu-24.04, gate job timeout-minutes other than 5; six controls"
  - ".github/workflows/gate.yml timeout-minutes 5"
affects: [04-02, 04-24, 04-25, ci]

actuals:
  tokens: 10600
  tasks: 3
  commits: 4
plan_head_before: eaa832e3d370d36ab470af40615c386a8ec7de17
plan_head_after: ae7a35cd1f12f22c627874eb7f8d4e38bb2c79f2

tech-stack:
  added: []
  patterns:
    - "Measurement lines (gate-env, gate-cpu) are printed like step-time: not tokens, never parsed in-gate"
    - "A gate helper that is not a step is self-tested by a dedicated step with an exact case inventory; an unknown case name fails"
    - "Job summary content comes only from files the gate itself writes into GATE_TMP"

key-files:
  created:
    - tools/check/ci-summary.sh
    - tools/check/steps/ci-summary.sh
    - data/expected/ci-summary-selftests.ids
    - data/expected/gate-steps/ci-measure.ids
    - data/expected/controls/ci-bans.ids
    - tools/check/controls/ci-parity-action-cache/
    - tools/check/controls/ci-parity-action-upload/
    - tools/check/controls/ci-parity-secrets/
    - tools/check/controls/ci-parity-matrix/
    - tools/check/controls/ci-parity-runs-on/
    - tools/check/controls/ci-parity-timeout/
  modified:
    - tools/gate.sh
    - tools/check/steps/ci-parity.sh
    - .github/workflows/gate.yml
    - tools/check/controls/_good/tree/.github/workflows/gate.yml
    - "tools/check/controls/ci-parity-{conditional,continue-on-error,extra-workflow-target,extra-workflow-unpinned,missing-gate,run-spacing,unpinned-action,write-permissions}/overlay/.github/workflows/gate.yml"
    - .planning/phases/04-headless-runner-and-installed-consumer/04-RESEARCH.md

key-decisions:
  - "`times` runs as `command times > \"$GATE_TMP/times.out\"` in the main gate shell after the controls wait: `command` keeps a redirection error of the special built-in from exiting the gate"
  - "The job summary headline also shows the gate-time, gate-env and gate-cpu lines (an extra optional input file measure.lines written by the gate), as 04-DISCUSS-ci E3 lists them"
  - "ci-parity-timeout applies to gate.yml only: the job `gate` needs a job-level `timeout-minutes: 5`; any other value anywhere in gate.yml, an absent key, or a step-level-only key fails"
  - "The Skips and missing table lists every matching token with its value, nonzero values first; `none` only when no token matches"

patterns-established:
  - "Every overlay copy of the toy workflow tracks the toy tree, so each ci-parity control fails for exactly its own token"

requirements-completed: []

duration: 45min
completed: 2026-11-11
---

# Phase 4 Plan 01: CI measurement, job summary and ci-parity bans Summary

**Every gate run now prints `gate-env` and `gate-cpu` next to `gate-time`; under GitHub Actions the gate's EXIT trap writes a job summary that cannot change the verdict; and a cache, upload, secret, matrix, runner or timeout change to CI fails ci-parity with a named token and control.**

## Performance

- **Duration:** about 45 min
- **Started:** 2026-11-11T15:57:24Z
- **Tasks:** 3 of 3
- **Files:** 36 changed (553 insertions, 27 deletions)

## Counts

Start (Phase 3 end, confirmed by the first run of this plan): `steps=26/26 controls=231/231 faults=119/119 tests-expected=2144`.
End: `steps=27/27 controls=237/237 faults=119/119 tests-expected=2144 tests-executed=2144 tests-skipped=0 tests-missing=0 ci-summary-selftest=11/11`. Matches the plan's stated end counts.

## Accomplishments

- `tools/gate.sh`: `bad()`, every step's output, the controls output and `step_time()` feed `fail.lines` and `step.times` in `$GATE_TMP`; the final line goes to `summary.line`, early exits write `gate: FAIL` there; `gate-env` (ncpu from `getconf`, compiler id and version from `build/dev/CMakeFiles/<cmake version>/CMakeCCompiler.cmake`, the c-build idiom) and `gate-cpu` lines print before `gate-time`. The EXIT trap is a function `on_exit` that stops the control runner, runs the helper only when `GITHUB_STEP_SUMMARY` is non-empty (`> /dev/null 2>&1 || :`), then removes the temp dir.
- `tools/check/ci-summary.sh` (not a step): `--cpu` parser for both observed `times` shapes, `gate-cpu unknown` otherwise; summary mode writes markdown only from gate-written files, prints nothing, exits 0 always, emits no annotations.
- Step `ci-summary` with 11 cases (`render-headline`, `skips-none`, `skips-listed`, `slowest-eight`, `fail-cap-40`, `backticks-stripped`, `unwritable-keeps-exit`, `unset-writes-nothing`, `cpu-dash-format`, `cpu-sh-format`, `cpu-garbage-unknown`); every helper call sets `GITHUB_STEP_SUMMARY` explicitly, so a CI run of the step never appends to the real summary.
- `ci-parity`: tokens `ci-parity-action-not-allowed`, `ci-parity-secrets`, `ci-parity-matrix`, `ci-parity-runs-on`, `ci-parity-timeout`; the pin check stays separate (`actions/checkout@v7` still reports only `ci-unpinned-action`). `gate.yml` changes only `timeout-minutes: 10` to `5`.

## Task Commits

1. **Pre-existing gate failure (Rule 3)** - `4ccc9bb` docs(04): drop a scratch-path shape from the research text
2. **Task 1: measurement lines and the job summary (tracer)** - `d214cbc` feat(04-01)
3. **Task 2: ci-summary selftest step** - `0c9e40e` feat(04-01)
4. **Task 3: ci-parity bans, timeout 5, six controls** - `ae7a35c` feat(04-01)

## Task 1 end-to-end proof (local, macOS, Apple clang 21, CMake 4.4.3)

- `GITHUB_STEP_SUMMARY=<mktemp file> sh tools/gate.sh`: exit 0; stdout has
  `gate-env ncpu=18 cc=AppleClang-21.0.0.21000101`, `gate-cpu user=46.83 sys=56.42`,
  `gate-time total=36`, last line `gate: PASS steps=26/26 controls=231/231 ...`, and no
  `Skips and missing` text. The file holds `### gate: PASS`, a fenced token and measurement block,
  the `Skips and missing` table (rows `pending-resolvers 17`, `unknown-fields 16`,
  `rows-pending 0`, `policies-pending 0`, `tests-skipped 0`, `tests-missing 0`), the 8 slowest
  steps (`controls 17`, `faults-verdict 5`, `c-faults 4`, `hygiene 4`, `hygiene-history 4`,
  `c-purity 3`, `c-tests 3`, `cp1610-vectors 3`) and `Failures (0)` / `none`.
- `GITHUB_STEP_SUMMARY=/nonexistent-dir/x sh tools/gate.sh`: exit 0, last line `gate: PASS`
  (`gate-cpu user=45.17 sys=53.10`, `gate-time total=34`).
- Early exit, sh and dash: `CC=no-such-cc-ivs GITHUB_STEP_SUMMARY=<file>` exits 1 and the file
  holds `### gate: FAIL` with the fenced `-- FAIL gate-no-cc: no-such-cc-ivs`; with the
  unwritable path the exit status is still 1.
- Acceptance: `grep -c ci-summary.sh tools/gate.sh` = 3; `grep -c 'times > ' tools/gate.sh` = 1;
  the two `--cpu` probes print `gate-cpu user=1.50 sys=0.25` and `gate-cpu unknown`.

## Task 2 probes (scratch copies, not committed)

- Case `cpu-sh-format` renamed in a copy of the step: `-- FAIL ci-summary-selftest-missing: cpu-sh-format`, exit 1.
- A helper copy without the 40-line cap, without backtick stripping and with ascending sort:
  `ci-summary-selftest-slowest-eight`, `-fail-cap-40` and `-backticks-stripped` fail, exit 1.

## Task 3 probes and first Phase 4 measurement

- Each of the 14 ci-parity controls run alone on its tree reports only its own token (the
  write-permissions control keeps its two `ci-permissions` lines). Extra probes: absent
  `timeout-minutes`, a step-level-only `timeout-minutes: 5` -> `ci-parity-timeout`; a local
  action `./local-action` -> `ci-parity-action-not-allowed` plus `ci-unpinned-action`;
  `secrets: inherit` -> `ci-parity-secrets`; quoted `"5"` and a quoted action name pass.
- `sh tools/check/run-controls.sh`: `controls=237/237`; 6 lines `control ok   ci-parity-(new)`.
- Local measurement at `ae7a35c` (laptop under load):
  - sh: `gate-env ncpu=18 cc=AppleClang-21.0.0.21000101`, `gate-cpu user=37.81 sys=45.02`, `gate-time total=30` (controls 15 s overlapped)
  - dash: `gate-env ncpu=18 cc=AppleClang-21.0.0.21000101`, `gate-cpu user=64.84 sys=84.50`, `gate-time total=51` (controls 34 s overlapped)
  - Step cost of this plan: `step-time ci-summary` 0-1 s, `ci-parity` 0 s.
- Container (`sh tools/gate-linux.sh` at `ae7a35c`, ubuntu:24.04 amd64 emulated on arm64; x86_64, gcc 13.3.0, cmake 3.28.3, dash as /bin/sh): `gate-env ncpu=8 cc=GNU-13.3.0`, `gate-cpu user=759.12 sys=90.84`, `gate-time total=235` (controls 158 s; emulated times are not hosted times), `gate: PASS steps=27/27 controls=237/237 ... faults=119/119 ... tests-expected=2144 tests-executed=2144 tests-skipped=0 tests-missing=0 ... ci-summary-selftest=11/11`, `gate-linux: exit 0`. This confirms the Linux `times` shape parses and `getconf`/the compiler id read work with GNU tools.

## Final gate summary lines

- `sh tools/gate.sh`: `gate: PASS steps=27/27 controls=237/237 ... faults=119/119 ... tests-expected=2144 tests-executed=2144 tests-skipped=0 tests-missing=0 ... ci-summary-selftest=11/11`
- `dash tools/gate.sh`: the same tokens, `gate: PASS`.
- Ubuntu container (`tools/gate-linux.sh`): the same tokens, `gate: PASS` (line above).

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] The gate failed at the plan's starting commit**
- **Found during:** Task 1 (first gate run)
- **Issue:** `hygiene-home-path` and `hygiene-history-home-path` on `04-RESEARCH.md` line 276: the scratch-path placeholder written with three dots after the temp-dir prefix matches the scratch-path rule (committed in `92c4dc7`).
- **Fix:** the same placeholder form the file already uses on line 53 (`/private/tmp/<name>`).
- **Commit:** `4ccc9bb`

**2. [Discretion] `command times` and a function EXIT trap**
- `times` is a special built-in, so a failed redirection would exit the gate; `command times > ...` runs it in the main shell without that property. The trap body became a function `on_exit` for readability; exit-status preservation is checked by the selftest case `unwritable-keeps-exit` and by the early-exit probes under sh and dash.

**3. [Discretion] Extra helper input `measure.lines`**
- The headline also shows `gate-time`, `gate-env` and `gate-cpu` (04-DISCUSS-ci E3 lists them in the headline). The file is optional; the helper works without it.

**4. [Plan step 3.2] Eight existing ci-parity overlay copies set to `timeout-minutes: 5`**
- None has a FORBID, so they would still pass, but each now fails only for its own token, as the plan's truth requires for the unpinned-checkout control.

**5. [Process] Commit trailer**
- The four commits end with `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`, the model that executed them, not the `Claude Sonnet 5.5` trailer that rule (e) prescribes. Reported to the orchestrator; no history rewrite.

## Open Items

- PKG-03 and PKG-04 stay Pending: `tools/ci-record.sh`, `.planning/ci-runs.tsv` and the `ci-log` step (later Phase 4 plans) are still to land, and no hosted run of these commits has been observed (agents never call `gh`).
- The Skips and missing table on the real gate lists `pending-resolvers=17` and `unknown-fields=16`, which are record stats, not skips; their key names match the D-42 pattern. A later plan may want an explicit allowlist if this reads as noise.
- Publication scan `sh tools/hygiene.sh --history` (not a gate step) fails on this branch: `hygiene-history-home-path` for the planning commits 345c48d, cccdfef, b78af4f, 92c4dc7 and eaa832e (the research text fixed in 4ccc9bb), and `hygiene-message-home-path` for 4ccc9bb itself (its message quotes the offending text). Rewriting pushed or foreign history is an orchestrator decision (a squash merge drops all of them); no rewrite was done here.
- Annotations: none emitted (plan text; D-42 allows token-only annotations if ever wanted).

## Threat Flags

None beyond the plan's threat model (T-04-01..04 mitigated as planned).

## Self-Check: PASSED

- Files present: tools/check/ci-summary.sh, tools/check/steps/ci-summary.sh, data/expected/ci-summary-selftests.ids, data/expected/gate-steps/ci-measure.ids, data/expected/controls/ci-bans.ids, the six control dirs.
- Commits reachable from HEAD: 4ccc9bb, d214cbc, 0c9e40e, ae7a35c (`git rev-list --count eaa832e..HEAD` = 4).
