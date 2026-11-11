---
phase: "2"
slug: "cp-1610-cpu-core"
status: draft
nyquist_compliant: true
wave_0_complete: false
created: "2026-11-09"
---

# Phase 2 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution. Full requirement-to-check map: `02-RESEARCH.md` § Validation Architecture (authoritative detail); decisions: `02-CONTEXT.md` D-14..D-25 and amendments A-01..A-13.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | In-repo C harness `tests/harness/ivs_test.{h,c}` (TAP-like) driven by CTest; the gate parses TAP files, not ctest |
| **Config file** | `CMakePresets.json` (v6), `tests/CMakeLists.txt`, `tests/faults.cmake` (all created in Wave 0 / the walking-skeleton plan) |
| **Quick run command** | `cmake --preset dev && cmake --build --preset dev && ctest --preset dev` (plus harness `--only <prefix>`) |
| **Full suite command** | `sh tools/gate.sh` (the single local command == the CI command) |
| **Estimated runtime** | hosted gate step at most 30 s (projected 17-20 s); local idle/warm at most 25 s (D-22) |

---

## Sampling Rate

- **After every task commit:** dev-preset quick run (`--only <group-prefix>`) plus the touched check script
- **After every plan wave:** `sh tools/gate.sh`
- **Before `/gsd-verify-work`:** gate green on both presets and `second-read=n/n`
- **Max feedback latency:** 30 seconds

---

## Per-Task Verification Map

Filled by the planner (18 plans, 49 tasks; revised after checker iteration 2). "gate" = `sh tools/gate.sh` (fails on non-zero exit or a last line not starting `gate: PASS`); "quick" = `cmake --build --preset dev && ctest --preset dev`; "controls" = `out=$(sh tools/check/run-controls.sh) && printf '%s\n' "$out" | tail -1` (the runner's exit status is observed through `&&`, never masked by the pipe). Every task has at least one automated command; no three consecutive tasks lack one. The phase is serial (one plan per wave) so each plan's expected counts are deterministic.

| Task | Plan | Wave | Requirement | Threat Ref | Test Type | Automated Command | Status |
|------|------|------|-------------|------------|-----------|-------------------|--------|
| 02-01-T1 | 01 | 1 | PKG-02, RUN-03 | T-02-01 | gate + probe | gate; `test -s build/gate/NONCE` | ⬜ pending |
| 02-01-T2 | 01 | 1 | RUN-03 | T-02-02, T-02-04 | controls | controls (116/116); `dash tools/gate.sh` | ⬜ pending |
| 02-01-T3 | 01 | 1 | CPU-01..04 (records) | T-02-03 | data check | gate; C36-C58 count = 23 (G8 = C58; C53 states the A-11 wait rule) | ⬜ pending |
| 02-02-T1 | 02 | 2 | CPU-05, CPU-04, PKG-02 | T-02-05, T-02-07 | build + tracer vector | `cmake --preset dev && cmake --build --preset dev && ctest --preset dev`; gate (`steps=19/19`) | ⬜ pending |
| 02-02-T2 | 02 | 2 | CPU-01, CPU-05 | T-02-05, T-02-07 | vectors (full schema, DW kind, one wait rule) | quick; gate (7 vectors ok) | ⬜ pending |
| 02-02-T3 | 02 | 2 | RUN-03 | T-02-06 | inventory controls | controls (128/128); gate (`tests-missing=0`) | ⬜ pending |
| 02-03-T1 | 03 | 3 | CPU-05, RUN-03 | T-02-44 | unit (cpu-api tracer) | quick; gate | ⬜ pending |
| 02-03-T2 | 03 | 3 | CPU-05, CPU-04, CPU-01 | T-02-44, T-02-45 | unit (cpu-api 16, harness 4) | quick; gate | ⬜ pending |
| 02-04-T1 | 04 | 4 | RUN-03, CPU-04 | T-02-09, T-02-10 | fault controls (9 tokens) | gate (`faults=2/2`); controls (137/137) | ⬜ pending |
| 02-04-T2 | 04 | 4 | CPU-01, RUN-03 | T-02-12, T-02-46 | structural vector checker (17 tokens) | gate (`vectors=7 source=4 policy=3`); controls (154/154) | ⬜ pending |
| 02-05-T1 | 05 | 5 | CPU-01, RUN-03 | T-02-11 | coverage derivation (8 tokens) | gate (`rows-pending=72 policies-pending=18`); controls (162/162) | ⬜ pending |
| 02-05-T2 | 05 | 5 | CPU-01, CPU-05 | T-02-11, T-02-47 | required tags, branch directions, witness, bus (7 tokens) | gate; controls (169/169); 35/68/1 data lines | ⬜ pending |
| 02-06-T1 | 06 | 6 | CPU-01, CPU-02 | T-02-48 | vectors + fault | quick; gate (`faults=3/3`, `rows-pending=70`) | ⬜ pending |
| 02-06-T2 | 06 | 6 | CPU-01, CPU-05 | T-02-48, T-02-49 | vectors + faults | gate (`faults=6/6`, `rows-pending=63`); `dash tools/gate.sh` | ⬜ pending |
| 02-07-T1 | 07 | 7 | API-05 | T-02-13 | symbol check | gate (`core-imports=`); controls | ⬜ pending |
| 02-07-T2 | 07 | 7 | API-05 | T-02-13, T-02-14 | purity controls + self-test | controls (13 core-purity); `dash tools/gate.sh` (`purity-selftest=5/5`) | ⬜ pending |
| 02-07-T3 | 07 | 7 | API-05 | T-02-15 | ctest core-symbols | `cmake --build --preset release && ctest --preset release -R core-symbols`; gate | ⬜ pending |
| 02-08-T1 | 08 | 8 | CPU-01 | T-02-16 | vectors + fault | quick; gate | ⬜ pending |
| 02-08-T2 | 08 | 8 | CPU-01 | T-02-17, T-02-18 | vectors + faults | quick; gate (`faults=12/12`) | ⬜ pending |
| 02-08-T3 | 08 | 8 | CPU-01 | T-02-17 | vectors + faults | gate (`faults=14/14`); `dash tools/gate.sh` | ⬜ pending |
| 02-09-T1 | 09 | 9 | CPU-01, CPU-05 | T-02-19 | vectors + fault | quick; gate | ⬜ pending |
| 02-09-T2 | 09 | 9 | CPU-01 | T-02-19 | vectors + fault | gate (`faults=16/16`) | ⬜ pending |
| 02-09-T3 | 09 | 9 | CPU-01 | T-02-20 | vectors + faults | gate (`faults=20/20`); `dash tools/gate.sh` | ⬜ pending |
| 02-10-T1 | 10 | 10 | CPU-04 | T-02-23 | vectors + fault (PULR witness vector) | quick; gate | ⬜ pending |
| 02-10-T2 | 10 | 10 | CPU-01, CPU-05 | T-02-21 | vectors + fault | gate (`faults=22/22`) | ⬜ pending |
| 02-10-T3 | 10 | 10 | CPU-04, CPU-01 | T-02-21, T-02-22 | vectors + faults (MVI@ witnessed by PULR, A-13) | gate (`faults=28/28`, `rows-pending=18`); `dash tools/gate.sh` | ⬜ pending |
| 02-11-T1 | 11 | 11 | CPU-01 | T-02-24 | vectors + faults | quick; gate (`faults=30/30`) | ⬜ pending |
| 02-11-T2 | 11 | 11 | CPU-01 | T-02-24, T-02-50 | vectors (4 directions) + faults | gate (`faults=32/32`, `rows-pending=9`) | ⬜ pending |
| 02-11-T3 | 11 | 11 | CPU-01 | T-02-24, T-02-50 | vectors (4 directions) + faults | gate (`faults=34/34`, `rows-pending=2`); `dash tools/gate.sh` | ⬜ pending |
| 02-12-T1 | 12 | 12 | CPU-01 | T-02-25 | BEXT vectors (E5/E10) + faults | quick; gate (`faults=36/36`) | ⬜ pending |
| 02-12-T2 | 12 | 12 | CPU-01, CPU-02 | T-02-51 | vectors + faults | gate (`faults=38/38`, `rows-pending=0`); `dash tools/gate.sh` | ⬜ pending |
| 02-13-T1 | 13 | 13 | CPU-03 | T-02-27 | vectors + fault | quick; gate (`faults=39/39`) | ⬜ pending |
| 02-13-T2 | 13 | 13 | CPU-03 | T-02-27 | vectors (12 refusals) + faults | gate (`faults=43/43`) | ⬜ pending |
| 02-13-T3 | 13 | 13 | CPU-01 | T-02-26, T-02-28 | sweep + faults | quick (cpu-sweep listed); gate (`faults=45/45`) | ⬜ pending |
| 02-14-T1 | 14 | 14 | CPU-02, CPU-05 | T-02-29, T-02-30 | vectors (DW kind) + fault | quick; gate (`faults=46/46`) | ⬜ pending |
| 02-14-T2 | 14 | 14 | CPU-02 | T-02-29, T-02-31 | vectors (17 defers-intr) + faults | gate (`faults=53/53`) | ⬜ pending |
| 02-14-T3 | 14 | 14 | CPU-02, CPU-04 | T-02-29 | vectors + faults | gate (`faults=55/55`); `dash tools/gate.sh` | ⬜ pending |
| 02-15-T1 | 15 | 15 | CPU-02 | T-02-34 | vectors + fault | quick; gate (`faults=56/56`) | ⬜ pending |
| 02-15-T2 | 15 | 15 | CPU-02, CPU-05 | T-02-32, T-02-34 | vectors (no-wait-on-DW, POL-HLT-01) + faults | gate (`faults=67/67`, `rows-pending=0`, `policies-pending=0`) | ⬜ pending |
| 02-15-T3 | 15 | 15 | CPU-05 | T-02-32, T-02-33 | fuzz + unit | quick (cpu-fuzz listed); gate; `dash tools/gate.sh` | ⬜ pending |
| 02-16-T1 | 16 | 16 | CPU-01, RUN-03 | T-02-35, T-02-37, T-02-38 | blind projection + marker/handoff controls | gate (`blind-files=12`, `second-read=0/12`); controls (190/190) | ⬜ pending |
| 02-16-T2 | 16 | 16 | CPU-01 | T-02-38 | diff tool + probes | gate; per-token check (`diff-tokens-ok`) | ⬜ pending |
| 02-16-T3 | 16 | 16 | CPU-01 | T-02-35 | protocol + policy docs | gate; 6 protocol sections, 19 policy sections | ⬜ pending |
| 02-17-T1 | 17 | 17 | CPU-01 (D-15, A-12) | T-02-40 | checkpoint: orchestrator fan-out (12 isolated readers, HANDOFF.txt) | `handoff-complete` (12 well-formed HANDOFF lines, 12 derived files) | ⬜ pending |
| 02-17-T2 | 17 | 17 | CPU-01 | T-02-40, T-02-41 | adjudication (control) + committed handoff record | diff tool `diff-summary file=control` (exit status observed); gate (`second-read=1/12`) | ⬜ pending |
| 02-17-T3 | 17 | 17 | CPU-01..05 | T-02-40, T-02-41 | adjudication (11 files) | `review-all-ok`; gate (`second-read=12/12`); `reviewed-set-ok` | ⬜ pending |
| 02-18-T1 | 18 | 18 | CPU-01 | T-02-42 | reference notes | gate; notes section counts | ⬜ pending |
| 02-18-T2 | 18 | 18 | PKG-02 | T-02-39 | budget measurement | gate (`controls=190/190`); `dash tools/gate.sh` | ⬜ pending |
| 02-18-T3 | 18 | 18 | all 8 | T-02-42 | phase evidence | gate (all final stats); `dash tools/gate.sh` | ⬜ pending |

Requirement coverage target (from research):

| Requirement | Automated check | Where | Status |
|-------------|-----------------|-------|--------|
| CPU-01 | vectors + decode sweep + vector-data checker + arithmetic/flag/cycle faults | ctest dev+release, gate `cp1610-vectors`, `c-faults` | ⬜ pending |
| CPU-02 | POL-INT/BUS/HLT/LINE policy vectors, one fault each (17 defers-intr rows; HLT via the POL-HLT-01 INSTR/INSTR/HALTED vector, A-13) | `c-tests`, `c-faults` | ⬜ pending |
| CPU-03 | SDBD modes 1-5 vectors, 12 refusal vectors (MMM=6 refusal labelled PULR, A-13), SDBD faults | `c-tests`, `c-faults` | ⬜ pending |
| CPU-04 | stack, auto-increment, reset vectors and faults | `c-tests`, `c-faults` | ⬜ pending |
| CPU-05 | per-vector `bus` lists in the `cpu-vectors` suite (every row has at least one, rule `vector-bus-uncovered`; exact cycle offsets only where a printed figure exists, `bus_basis: figure`), `cpu-api` (bus order matches log, log refusal, `state_validate`, determinism, isolation, no-progress units), `cpu-fuzz`; there is no separate `cpu-bus` suite | `c-tests`, `cp1610-vectors` | ⬜ pending |
| API-05 | `c-purity` + ctest suite `core-symbols` + 13 `core-purity-*` controls | gate, ctest | ⬜ pending |
| PKG-02 | `sh tools/gate.sh` runs the C steps; ci-parity unchanged; ninja/cc prechecks | gate, CI | ⬜ pending |
| RUN-03 | `tests-inventory` over TAP + NONCE + ids + `ctest -N`; 12 `tests-inventory-*` controls; every `faults-verdict` and `cp1610-vectors` token has its own control (9 + 32) | gate, control runner | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] `CMakeLists.txt`, `CMakePresets.json`, `cmake/IvsWarnings.cmake`, `tests/{CMakeLists.txt,faults.cmake,harness/ivs_test.{h,c}}`
- [ ] `core/src/common/ivs_status.h` and the core prelude header (std headers first, `#pragma GCC poison float double` last)
- [ ] vector generator (`cmake -P`) and the multi-file vector checker (`vectors.cmake`)
- [ ] `tools/check/steps/{c-build,c-tests,c-faults,c-purity,tests-inventory,cp1610-vectors}.sh`, `tools/check/core-symbols.sh`, `tools/check/core-symbols-allow.txt`
- [ ] `data/expected/{cp1610-vectors.ids,cp1610-skips.ids,tests/*.ids,faults/cpu.ids,gate-steps/cpu.ids,controls/*.ids}` and `_good/tree` fixtures (landed atomically with each step, A-08)
- [ ] Framework install: none (cmake, ninja, cc, nm, ar only; the hosted image has all)

---

## Manual-Only Verifications

All phase behaviors have automated verification (D-25). Hardware timing, BUSRQ release latency and HLT behaviour stay recorded as missing evidence (D10 hard stop); nothing is handed to the owner.

---

## Validation Sign-Off

- [x] All tasks have `<automated>` verify or Wave 0 dependencies
- [x] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references
- [x] No watch-mode flags
- [ ] Feedback latency < 30s
- [x] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
