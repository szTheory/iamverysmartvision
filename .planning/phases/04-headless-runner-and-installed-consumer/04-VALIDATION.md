---
phase: "4"
slug: "headless-runner-and-installed-consumer"
status: draft
nyquist_compliant: true
wave_0_complete: false
created: "2026-11-11"
---

# Phase 4 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution. Full requirement-to-check map: `04-RESEARCH.md` "Validation Architecture"; decisions: `04-CONTEXT.md` D-01..D-47 and amendments A-01..A-08; hazards: `04-PATTERNS.md` (18 ranked) and `04-RESEARCH.md` Pitfalls 1-16.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | In-repo C harness `tests/harness/ivs_test.{h,c}` (TAP) driven by CTest; shell TAP suites (`tests/headers-tap.sh`, new `tests/pack-tap.sh`, `tests/runner-process-tap.sh`); `cmake -P` and `sh` gate steps; named-fault executables (`tests/faults.cmake`, generalized to `runner/` owners in plan 04-03); negative controls (`tools/check/run-controls.sh`); in-step selftests for steps a control cannot drive (`ci-summary`, `pkg-install`) |
| **Config file** | `CMakePresets.json` (`dev`: Debug, ASan+UBSan; `release`: -O2), `tests/CMakeLists.txt` (suite and fault-suite registry), `data/expected/tests/<suite>.ids`, `data/expected/faults/*.ids`, `data/expected/controls/*.ids`, `data/expected/gate-steps/*.ids`, `data/expected/{pkg,ci-summary}-selftests.ids`, `data/expected/important-gates.ids` |
| **Quick run command** | `cmake --build --preset dev && build/dev/tests/ivs_tests_run --only run.<family>.` (also `ivs_tests_runner --only runner.<family>.`, `ivs_tests_machine --only machine.digest.`); one suite: `ctest --preset dev -R '^(run|runner|runner-process|pack|machine)$'` |
| **Full suite command** | `sh tools/gate.sh` (the single local command == the CI command) and `dash tools/gate.sh` at every plan end; `sh tools/gate-linux.sh` (ubuntu 24.04, gcc 13, dash, cmake 3.28 in a container) before pushing plans 04-03, 04-07, 04-11, 04-14, 04-18, 04-19, 04-22, 04-23, 04-25 |
| **Estimated runtime** | Phase 4 start: 26 steps, 231 controls, 119 faults, 2144 test executions; hosted gate 30-49 s on identical code. Budget (D-45): apply the next lever when the median of the last 3 hosted gate-times exceeds 40 s; revisit when any run exceeds 60 s or the median of the last 5 exceeds 50 s. Measured from plan 04-01 on (`gate-env`, `gate-cpu`, `step-time`), recorded per hosted run in `.planning/ci-runs.tsv` by the orchestrator (`tools/ci-record.sh` after every push from plan 04-02 on); budget checkpoints after plans 04-10 and 04-19, where the orchestrator may insert a lever plan, and the phase-end evaluation in plan 04-25 |

---

## Sampling Rate

- **After every task commit:** dev-preset quick run of the touched family (`--only <prefix>`), or the touched step through `sh tools/gate.sh`, or the control runner when controls changed
- **After every plan (each plan is its own wave; execution is strictly sequential):** `sh tools/gate.sh` and `dash tools/gate.sh`, both ending `gate: PASS` with the plan's exact counts
- **Before pushing a runner, packaging or close-out plan:** `sh tools/gate-linux.sh` (hosted-only gcc/glibc/PIE/ELF classes; executors run it, the orchestrator pushes)
- **Before `/gsd-verify-work`:** gate green under sh and dash with the final counts of plan 04-25 (`steps=33/33`, `controls>=275`, `faults>=149`, `pkg-selftest=10/10`, `ci-summary-selftest=11/11`, `claims=8`, `tests-missing=0`)
- **Budget checkpoints (D-45, orchestrator):** after the pushes of plans 04-10 and 04-19, evaluate the median of the last 3 hosted gate-times; above 40 s, insert a lever plan (background `c-build` for independent steps, cheaper suites) before continuing
- **Cost discipline:** prefer controls and in-step selftests over named faults when both prove the same thing; every new suite (`runner`, `run`, `pack`, `runner-process`) reports its measured plain-run time per preset in its plan SUMMARY
- **Max feedback latency:** 40 seconds hosted median (D-45)

---

## Per-Task Verification Map

"gate" = `sh tools/gate.sh` with its final line checked for `gate: PASS` and the listed tokens (an executable loop over `grep -F`); "dash" = the same with `dash tools/gate.sh`; "quick" = `cmake --build --preset dev && build/dev/tests/<suite exe> --only <prefix>`; "controls" = `out=$(sh tools/check/run-controls.sh) && test "$(printf '%s\n' "$out" | tail -1)" = "controls=N/N"`. Counts move by plan: steps 26 -> 27 (04-01) -> 28 (04-02) -> 29 (04-04) -> 30 (04-15) -> 31 (04-21) -> 32 (04-22) -> 33 (04-24); controls 231 -> 237 -> 241 -> 243 -> 249 -> 252 (04-08) -> 254 (04-13) -> 263 (04-15) -> 273 (04-21) -> 275+ (04-24); faults 119 -> 123 (04-03) -> 125 (04-05) -> 127 (04-06) -> 130 (04-07) -> 132 (04-09) -> 134 (04-10) -> 140 (04-11) -> 142 (04-17) -> 146 (04-18) -> 149 (04-19). Every task has at least one automated command; no three consecutive tasks lack one.

| Task | Plan | Wave | Requirement | Threat Ref | Test Type | Automated Command | Status |
|------|------|------|-------------|------------|-----------|-------------------|--------|
| 04-01-T1 | 01 | 1 | PKG-04, PKG-03 (D-42, D-44) | T-04-01, T-04-02 | gate end to end with `GITHUB_STEP_SUMMARY` (tracer) | gate with summary file; unwritable path keeps PASS | ⬜ pending |
| 04-01-T2 | 01 | 1 | PKG-03 (D-42) | T-04-01, T-04-02 | in-step selftest (11 cases) | gate (`ci-summary-selftest=11/11`, `steps=27/27`) | ⬜ pending |
| 04-01-T3 | 01 | 1 | PKG-03 (D-41, D-43, A-07) | T-04-03, T-04-04 | controls (6) | controls (237/237); gate + dash | ⬜ pending |
| 04-02-T1 | 02 | 2 | PKG-04 (D-44) | T-04-05, T-04-06 | step over the run record (tracer) + offline recorder round trip | gate (`ci-runs=0`, `steps=28/28`) | ⬜ pending |
| 04-02-T2 | 02 | 2 | PKG-04 (D-45) | T-04-05 | controls (4) + budget rule record | controls (241/241); gate + dash | ⬜ pending |
| 04-03-T1 | 03 | 3 | RUN-04, RUN-02 (D-24, D-17, D-20) | T-04-08, T-04-09, T-04-10 | fault framework end to end (tracer) | quick `ctest -R '^runner$'`; gate (`faults=120/120`) | ⬜ pending |
| 04-03-T2 | 03 | 3 | RUN-02, RUN-04 | T-04-08, T-04-11 | unit + 3 faults + controls (2) | controls (243/243); gate + dash (`faults=123/123`); gate-linux | ⬜ pending |
| 04-04-T1 | 04 | 4 | RUN-04, RUN-01 (D-23) | T-04-12 | static step (tracer) | gate (`runner-files=`, `steps=29/29`) | ⬜ pending |
| 04-04-T2 | 04 | 4 | RUN-04 (A-07) | T-04-12, T-04-13, T-04-14 | controls (6) + build-tree probe | controls (249/249); gate + dash | ⬜ pending |
| 04-05-T1 | 05 | 5 | RUN-05 (D-10, A-03) | T-04-15, T-04-16 | literal pin then one live domain (tracer) | quick `machine.digest.`; gate | ⬜ pending |
| 04-05-T2 | 05 | 5 | RUN-05 | T-04-15, T-04-16, T-04-17 | unit (coverage, sensitivity, reset, refusal) + 2 faults | quick `machine.digest.`, `machine.reset.`; gate + dash (`faults=125/125`) | ⬜ pending |
| 04-06-T1 | 06 | 6 | API-02, RUN-01 (D-08, A-01, A-06) | T-04-18 | new suite `run` via public header (tracer) | `ctest -R '^(run|headers)$'`; gate (`api-calls=11`) | ⬜ pending |
| 04-06-T2 | 06 | 6 | API-02 (D-08) | T-04-18, T-04-19, T-04-20 | refusal rows + side-effect proofs + 2 faults | quick `run.`; gate + dash (`faults=127/127`, `api-calls=12`) | ⬜ pending |
| 04-07-T1 | 07 | 7 | API-02 (D-01..D-03) | T-04-21 | run to absolute target (tracer) | quick `run.target.`, `run.single-step.`; gate (`api-calls=14`) | ⬜ pending |
| 04-07-T2 | 07 | 7 | API-02 (D-04, D-05) | T-04-22, T-04-23 | stop reasons, ladder, precedence | quick `run.` both presets; gate | ⬜ pending |
| 04-07-T3 | 07 | 7 | API-02 (D-07) | T-04-21, T-04-23 | 3 faults + gate-linux | gate + dash (`faults=130/130`); gate-linux | ⬜ pending |
| 04-08-T1 | 08 | 8 | API-02, RUN-04 (D-04, D-05) | T-04-24, T-04-25 | define pin (tracer) | gate (`pinned-defines=`) | ⬜ pending |
| 04-08-T2 | 08 | 8 | RUN-04 | T-04-24 | controls (3) | controls (252/252); gate + dash | ⬜ pending |
| 04-09-T1 | 09 | 9 | API-02 (D-06, D-09, A-02) | T-04-26 | generator to `ivs_info` (tracer) | quick `run.info.`, `ctest -R '^(machine|memmap)$'`; gate | ⬜ pending |
| 04-09-T2 | 09 | 9 | API-02, RUN-05 (D-07) | T-04-26, T-04-27 | five-pattern slicing vs reference + 2 faults | quick `run.slice.` both presets; gate + dash (`faults=132/132`) | ⬜ pending |
| 04-10-T1 | 10 | 10 | API-04 (D-06) | T-04-28, T-04-29 | env-seam refusal (tracer) | quick `run.buffers.`; gate | ⬜ pending |
| 04-10-T2 | 10 | 10 | API-04 (D-02, D-03, D-05) | T-04-28, T-04-29, T-04-30 | buffer matrix + 2 faults | quick `run.buffers.`, `run.prec.`; gate + dash (`faults=134/134`) | ⬜ pending |
| 04-11-T1 | 11 | 11 | RUN-01, RUN-05 (D-12..D-15, A-04, A-08) | T-04-31, T-04-33 | one event end to end with independent chain oracle (tracer) | quick `run.input.`, `run.slice.`; gate | ⬜ pending |
| 04-11-T2 | 11 | 11 | RUN-01 (D-14, D-05) | T-04-31, T-04-32, T-04-34 | event-argument slot (NULL, 65536 cap, overlaps, `reserved_in`), validation, floor, reset, isolation | quick `run.input.` both presets; gate | ⬜ pending |
| 04-11-T3 | 11 | 11 | RUN-05 (D-16, D-07, D-05) | T-04-31..T-04-34 | 6 faults + gate-linux | gate + dash (`faults=140/140`); gate-linux | ⬜ pending |
| 04-12-T1 | 12 | 12 | RUN-01 (D-13, D-15) | T-04-35 | profile field to API refusal (tracer) | quick `run.input.`, `ctest -R '^(memmap|machine)$'`; gate | ⬜ pending |
| 04-12-T2 | 12 | 12 | RUN-05 (D-15) | T-04-36 | guest invisibility | quick `run.input.guest-invisible`; gate + dash | ⬜ pending |
| 04-13-T1 | 13 | 13 | RUN-01 (D-12, D-16) | T-04-37 | bit pin (tracer) | gate + 31-name count | ⬜ pending |
| 04-13-T2 | 13 | 13 | RUN-04 | T-04-37 | controls (2) | controls (254/254) | ⬜ pending |
| 04-13-T3 | 13 | 13 | RUN-05 (D-16, A-05, A-02) | T-04-38, T-04-39 | records + seeded fuzz + struct sizes | quick `run.fuzz.`, `run.struct.`; gate + dash | ⬜ pending |
| 04-14-T1 | 14 | 14 | RUN-06 (D-25, D-26, D-28, D-30) | T-04-40, T-04-41 | stub listing, manifest and images in both presets to boot path (tracer) | 8192-byte image equal across presets; quick `run.stub.`; gate | ⬜ pending |
| 04-14-T2 | 14 | 14 | RUN-06 (D-29) | T-04-40, T-04-42 | pack source guard, trap test, review rule | quick `run.stub.`; gate + dash; gate-linux | ⬜ pending |
| 04-15-T1 | 15 | 15 | RUN-06 (D-26) | T-04-43 | listing step (tracer) | gate (`listings=1`, `steps=30/30`) | ⬜ pending |
| 04-15-T2 | 15 | 15 | RUN-06, RUN-04 (D-29) | T-04-43, T-04-44 | controls (9) | controls (263/263) | ⬜ pending |
| 04-15-T3 | 15 | 15 | RUN-06 | T-04-45 | TAP suite `pack` (two parsers) | `ctest -R '^pack$'`; gate + dash | ⬜ pending |
| 04-16-T1 | 16 | 16 | RUN-06 (D-31, D-25) | T-04-46, T-04-47 | diagnostic listing, manifest, predicate via public API (tracer) | quick `run.diag.`; `ctest -R '^pack$'`; gate (`listings=2`) | ⬜ pending |
| 04-16-T2 | 16 | 16 | RUN-06, RUN-05 (D-11, D-27, D-07, A-01) | T-04-46 | hand derivation `.expect`, slicing and repeat on the real program | quick `run.diag.` both presets; header greps | ⬜ pending |
| 04-16-T3 | 16 | 16 | RUN-06 (D-27, D-31) | T-04-48 | claim CL-0008 + discrepancy | gate + dash (`claims=8`) | ⬜ pending |
| 04-17-T1 | 17 | 17 | RUN-01 (D-18) | T-04-49 | parser on the diagnostic scenario (tracer) | quick `runner.parse.`; gate | ⬜ pending |
| 04-17-T2 | 17 | 17 | RUN-01, RUN-02 (D-22) | T-04-49, T-04-50, T-04-51 | parser table + seeded fuzz (ASan/UBSan) | `ivs_tests_runner` both presets | ⬜ pending |
| 04-17-T3 | 17 | 17 | RUN-04 | T-04-49, T-04-50 | 2 faults | gate + dash (`faults=142/142`) | ⬜ pending |
| 04-18-T1 | 18 | 18 | RUN-01 (D-17, D-19, D-30, D-08) | T-04-52, T-04-54 | in-process scenario to pass report (tracer) | quick `runner.exec.`; gate | ⬜ pending |
| 04-18-T2 | 18 | 18 | RUN-01, RUN-02, RUN-05 (D-20, D-32, D-09) | T-04-52, T-04-53 | outcomes, stop INTERNAL mapping, determinism | quick `runner.exec.` both presets | ⬜ pending |
| 04-18-T3 | 18 | 18 | RUN-04 | T-04-53 | 4 faults + gate-linux | gate + dash (`faults=146/146`); gate-linux | ⬜ pending |
| 04-19-T1 | 19 | 19 | RUN-01 (D-17, D-22, D-23) | T-04-55, T-04-58 | `ivs-run` on the diagnostic scenario (tracer) | executable exit 0, `ivsr 1`, `report.end`, stub label; gate | ⬜ pending |
| 04-19-T2 | 19 | 19 | RUN-02 (D-21) | T-04-56, T-04-57 | fake-clock slices, budget, wall, cancel; self-signal hook | quick `runner.time.`; executable exit 12 | ⬜ pending |
| 04-19-T3 | 19 | 19 | RUN-02, RUN-04 (D-24) | T-04-56, T-04-57, T-04-58 | 3 faults + docs + gate-linux | gate + dash (`faults=149/149`); gate-linux | ⬜ pending |
| 04-20-T1 | 20 | 20 | RUN-01, RUN-06 (D-23) | T-04-59, T-04-60 | process suite on the diagnostic (tracer) | `ctest -R '^runner-process$'`; per-preset report files | ⬜ pending |
| 04-20-T2 | 20 | 20 | RUN-02, RUN-06 (D-20, D-32, A-01) | T-04-59, T-04-61 | one scenario per outcome (six-column inventory with host-dependent class), diagnostic mutants, repeat/slice/cwd | `ctest -R '^runner-process$'`; gate (visible skip) | ⬜ pending |
| 04-20-T3 | 20 | 20 | RUN-02, RUN-04 (D-23) | T-04-59 | judge self-controls + outcome anti-drift | gate + dash | ⬜ pending |
| 04-21-T1 | 21 | 21 | RUN-05 (D-11, D-19, A-01) | T-04-62, T-04-63, T-04-64 | golden step on the real tree (tracer) | gate (`goldens=2`, `steps=31/31`) | ⬜ pending |
| 04-21-T2 | 21 | 21 | RUN-05 | T-04-62 | input and budget goldens | gate (`goldens=4`) | ⬜ pending |
| 04-21-T3 | 21 | 21 | RUN-05, RUN-04 (D-21) | T-04-62, T-04-63, T-04-65, T-04-79 | controls (10, incl. host-dependent exclusion) | controls (273/273); gate + dash | ⬜ pending |
| 04-22-T1 | 22 | 22 | PKG-01 (D-33, D-34, D-35, D-37) | T-04-66, T-04-67 | relocated install and exact inventory (tracer) | gate (`prefix-files=`, `steps=32/32`); sanitized install refused | ⬜ pending |
| 04-22-T2 | 22 | 22 | PKG-01 (D-37, D-38) | T-04-67, T-04-68 | header/archive/path/symbol/headers-alone proofs | gate (`public-symbols=`, `hdr-alone=`) | ⬜ pending |
| 04-22-T3 | 22 | 22 | PKG-01 (D-40) | T-04-66..T-04-69 | in-step selftests (8) | gate + dash (`pkg-selftest=8/8`); gate-linux | ⬜ pending |
| 04-23-T1 | 23 | 23 | PKG-01 (D-36, D-39) | T-04-70, T-04-72 | consumer via `find_package` in the moved prefix (tracer) | gate (`consumer-lines=`) | ⬜ pending |
| 04-23-T2 | 23 | 23 | PKG-01 (D-37, D-38, D-39) | T-04-70, T-04-71 | deps, link line, `ivs_DIR`, runner-golden agreement, call pin, C++ link | gate (`cxx-link=`) | ⬜ pending |
| 04-23-T3 | 23 | 23 | PKG-01 (D-40, D-35) | T-04-70, T-04-71 | selftests 9-10 + docs | gate + dash (`pkg-selftest=10/10`); gate-linux | ⬜ pending |
| 04-24-T1 | 24 | 24 | RUN-04 (D-46) | T-04-73 | registry step (tracer) | gate (`gates=`, `steps=33/33`) | ⬜ pending |
| 04-24-T2 | 24 | 24 | RUN-04 | T-04-73, T-04-74 | audit + gap fills | gate | ⬜ pending |
| 04-24-T3 | 24 | 24 | RUN-04 | T-04-73 | controls (2) | controls (>=275); gate + dash | ⬜ pending |
| 04-25-T1 | 25 | 25 | all 10 | T-04-75 | phase evidence (tracer) | gate (all final stats) | ⬜ pending |
| 04-25-T2 | 25 | 25 | records (D-16, D-27, D-31, A-05) | T-04-76 | per-id record script shown to fail first | `cmake -DROOT="$PWD" -P build/phase4-records.cmake`; gate + dash | ⬜ pending |
| 04-25-T3 | 25 | 25 | PKG-03, PKG-04 (D-41, D-45, D-47) | T-04-77, T-04-78 | measurement, D-45 evaluation, records, caveats | gate + dash; record greps; gate-linux | ⬜ pending |

Requirement coverage target:

| Requirement | Automated check | Where | Status |
|-------------|-----------------|-------|--------|
| API-02 | `run.target.*`, `run.overshoot.*`, `run.stop.*`, `run.refuse.*`, `run.prec.*`, `run.single-step.*`, `run.slice.*` (five patterns vs unit-by-unit reference), `run.diag.slice`; stop and reason pins; faults `RUN_TARGET_UNCHECKED`, `RUN_STOP_BEFORE_OVERSHOOT`, `RUN_FAULT_STEPS`, `RUN_BUDGET_CARRY`, `INFO_TAIL_UNGUARDED`, `PEEK_*` | suite `run`, step `api-effects` | ⬜ pending |
| API-04 | `run.buffers.*` through the env seam with synthetic layouts and the public call with the real zero layout; whole-storage memcmp and poison patterns; faults `RUN_CAPACITY_AFTER_STEP`, `OUTBUF_PARTIAL_WRITE`; event-argument refusals and `RUN_EVENTS_UNCHECKED` (plan 04-11) | suite `run`, step `api-effects` | ⬜ pending |
| RUN-01 | IVSS parser (`runner.parse.*`, fuzz), executor (`runner.exec.*`), `ivs-run` (`runner-process.scenario.*`), input schedule (`run.input.*`, `diag-input-pass`), observation (`run.cpu-state.*`, `run.peek`) | suites `runner`, `runner-process`, `run` | ⬜ pending |
| RUN-02 | `runner.outcome.*`, `runner.time.*`, one demonstrating scenario per outcome in `data/expected/runner-scenarios.ids`, judge self-controls; faults `OUTCOME_*`, `RUNNER_*` | suites `runner`, `runner-process` | ⬜ pending |
| RUN-04 | `data/expected/important-gates.ids` checked by step `important-gates` (faults, controls, mutants resolved against inventories); fault framework covers `runner/` (controls `faults-*-runner`) | steps `important-gates`, `faults-verdict` | ⬜ pending |
| RUN-05 | seven domain digests (`machine.digest.*` coverage, composite excludes 16-18, stepped literals), INPUT chain oracle, repeat/slice/dev-vs-release equality, golden headers and host-dependent exclusion (step `golden`, 10 controls) | suites `machine`, `run`, `runner-process`; step `golden` | ⬜ pending |
| RUN-06 | stub and diagnostic listings (step `listing`, 9 controls, suite `pack`), `run.stub.*`, `run.diag.*`, `runner-process.scenario.diag-boot-pass` with the stub label, four program mutants guest-fail, `.expect` with derivation, CL-0008 | steps `listing`, `claims`; suites `run`, `pack`, `runner-process` | ⬜ pending |
| PKG-01 | step `pkg-install`: relocated install, exact inventory, header/archive equality, path leaks, symbol rules, headers alone, consumer via `find_package`, deps/link/`ivs_DIR` independence, consumer golden and runner-golden agreement, `pkg-selftest=10/10` | step `pkg-install` | ⬜ pending |
| PKG-03 | `ci-parity` bans (action allowlist, secrets, matrix, runs-on, timeout; 6 controls), job summary with a skips-and-missing table (`ci-summary` selftest), visible `needs-user-exec` skip, no caches or artifacts | steps `ci-parity`, `ci-summary`, `tests-inventory` | ⬜ pending |
| PKG-04 | `gate-env`, `gate-cpu`, `step-time` lines in every run; `.planning/ci-runs.tsv` validated by `ci-log` (4 controls); `tools/ci-record.sh` (orchestrator, two `gh` calls per run); D-45 rule evaluated at plan 04-25 | steps `ci-log`; CI-EFFICIENCY record | ⬜ pending |

Success criteria to checks (zero human UAT):

| SC | Automated check |
|----|-----------------|
| SC1 | `runner-process.scenario.diag-boot-pass` (exit 0, `exec: project boot stub, not an EXEC sha256=...`), `data/golden/diag-cpu-1.expect` (`# oracle: independent-derivation`, derivation cites), `run.diag.predicate`, step `listing`, suite `pack` |
| SC2 | `data/expected/runner-scenarios.ids` covering all outcomes with distinct exit codes, `runner-process.outcomes-covered`, judge self-controls, `OUTCOME_*`/`RUNNER_*` faults, step `important-gates` |
| SC3 | `run.target.*` and `run.overshoot.*`, `run.slice.*` and `run.diag.slice` (seven domain digests), `runner-process.repeat.*`/`.slices.*`, step `golden` (headers, dev == release), `run.buffers.*` refusals before any state change |
| SC4 | step `pkg-install` with consumer stats and `pkg-selftest=10/10` |
| SC5 | `ci-parity` with its bans, `ci-summary` selftest, `gate-env`/`gate-cpu`/`step-time` lines, `ci-log` over `.planning/ci-runs.tsv`; the job itself is still one `sh tools/gate.sh` |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] Plan 04-01 lands the measurement lines and the job summary before any other Phase 4 change (every later hosted run is measured)
- [ ] Plan 04-03 generalizes the fault framework to `runner/` owners (D-24) before any runner fault exists; all 119 earlier faults stay green
- [ ] Plan 04-04 extends integrity and the unlisted-source guard to `runner/` and `tools/**/*.c` before the runner grows
- [ ] Plan 04-05 pins a stepped-state composite literal set on the unmodified Phase 3 code before the digest table changes (A-03)
- [ ] Each new suite (`runner` 04-03, `run` 04-06, `pack` 04-15, `runner-process` 04-20) lands with its ids file, ctest registration and TAP in both presets in the same plan
- [ ] Each new step (`ci-summary` 04-01, `ci-log` 04-02, `runner-scan` 04-04, `listing` 04-15, `golden` 04-21, `pkg-install` 04-22, `important-gates` 04-24) lands with its gate-step inventory line and its controls (pure steps, with good-tree toy inputs) or in-step selftests (helper and build steps)
- [ ] Framework install: none (cmake, ninja, cc, nm, ar, docker for the optional local Linux gate; optional c++ for visible-skip cases; no assembler)

---

## Manual-Only Verifications

All phase behaviors have automated verification; nothing is handed to the owner as UAT. Recorded, not handed off: D10 hardware measurement (controller matrix, unplugged and output-driven port reads, which connector is R14; C63 onward) and D11 (a run with an owner-held EXEC/GROM) are hard stops in `.planning/AUTONOMY.md` and stay outside the automated exit. Branch protection on `main` is a repository setting recorded as an owner/orchestrator-assist recommendation (D-47); it blocks nothing. Hosted gate times are read by the orchestrator (`tools/ci-record.sh`); agents never call `gh`.

---

## Validation Sign-Off

- [x] All tasks have `<automated>` verify or Wave 0 dependencies
- [x] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references
- [x] No watch-mode flags
- [ ] Feedback latency < 40s hosted median (measured from plan 04-01, evaluated at plan 04-25)
- [x] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
