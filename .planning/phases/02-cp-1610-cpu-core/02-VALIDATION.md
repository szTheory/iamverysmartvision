---
phase: "2"
slug: "cp-1610-cpu-core"
status: draft
nyquist_compliant: true
wave_0_complete: false
created: "2026-11-09"
---

# Phase 2 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution. Full requirement-to-check map: `02-RESEARCH.md` § Validation Architecture (authoritative detail); decisions: `02-CONTEXT.md` D-14..D-25 and amendments A-01..A-10.

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

Filled by the planner (13 plans, 39 tasks). "gate" = `sh tools/gate.sh` (fails on non-zero exit or a last line not starting `gate: PASS`); "quick" = `cmake --build --preset dev && ctest --preset dev`; "controls" = `sh tools/check/run-controls.sh | tail -1`. Every task has at least one automated command; no three consecutive tasks lack one.

| Task | Plan | Wave | Requirement | Threat Ref | Test Type | Automated Command | Status |
|------|------|------|-------------|------------|-----------|-------------------|--------|
| 02-01-T1 | 01 | 1 | PKG-02, RUN-03 | T-02-01 | gate + probe | gate; `test -s build/gate/NONCE` | ⬜ pending |
| 02-01-T2 | 01 | 1 | RUN-03 | T-02-02, T-02-04 | controls | controls; `dash tools/gate.sh` | ⬜ pending |
| 02-01-T3 | 01 | 1 | CPU-01..04 (records) | T-02-03 | data check | gate; C36-C57 count = 22 | ⬜ pending |
| 02-02-T1 | 02 | 2 | CPU-05, CPU-04, PKG-02 | T-02-05, T-02-07 | build + vectors | `cmake --preset dev && cmake --build --preset dev && ctest --preset dev`; gate | ⬜ pending |
| 02-02-T2 | 02 | 2 | RUN-03 | T-02-06 | inventory controls | controls; gate (`tests-missing=0`) | ⬜ pending |
| 02-02-T3 | 02 | 2 | CPU-05 | T-02-05 | unit (cpu-api, harness) | quick; gate | ⬜ pending |
| 02-03-T1 | 03 | 3 | RUN-03, CPU-04 | T-02-09, T-02-10 | fault controls | gate (`faults=2/2`); controls | ⬜ pending |
| 02-03-T2 | 03 | 3 | CPU-01, RUN-03 | T-02-11, T-02-12 | data checker controls | gate (`second-read=0/1`); controls | ⬜ pending |
| 02-03-T3 | 03 | 3 | CPU-01 | T-02-11 | vectors + faults | quick; gate (`faults=6/6`) | ⬜ pending |
| 02-04-T1 | 04 | 4 | API-05 | T-02-13 | symbol check | gate (`core-imports=`); controls | ⬜ pending |
| 02-04-T2 | 04 | 4 | API-05 | T-02-13, T-02-14 | purity controls + self-test | controls (13 core-purity); `dash tools/gate.sh` (`purity-selftest=5/5`) | ⬜ pending |
| 02-04-T3 | 04 | 4 | API-05 | T-02-15 | ctest core-symbols | `cmake --build --preset release && ctest --preset release -R core-symbols`; gate | ⬜ pending |
| 02-05-T1 | 05 | 4 | CPU-01 | T-02-16 | vectors + fault | quick; gate | ⬜ pending |
| 02-05-T2 | 05 | 4 | CPU-01 | T-02-17, T-02-18 | vectors + faults | quick; gate (`faults=12/12`) | ⬜ pending |
| 02-05-T3 | 05 | 4 | CPU-01 | T-02-17 | vectors + faults | gate (`faults=14/14`); `dash tools/gate.sh` | ⬜ pending |
| 02-06-T1 | 06 | 5 | CPU-01, CPU-05 | T-02-19 | vectors + fault | quick; gate | ⬜ pending |
| 02-06-T2 | 06 | 5 | CPU-01 | T-02-19 | vectors + fault | gate (`faults=16/16`) | ⬜ pending |
| 02-06-T3 | 06 | 5 | CPU-01 | T-02-20 | vectors + faults | gate (`faults=20/20`); `dash tools/gate.sh` | ⬜ pending |
| 02-07-T1 | 07 | 6 | CPU-04 | T-02-23 | vectors + fault | quick; gate | ⬜ pending |
| 02-07-T2 | 07 | 6 | CPU-01, CPU-05 | T-02-21 | vectors + fault | gate (`faults=22/22`) | ⬜ pending |
| 02-07-T3 | 07 | 6 | CPU-04, CPU-01 | T-02-21, T-02-22 | vectors + faults | gate (`faults=28/28`); `dash tools/gate.sh` | ⬜ pending |
| 02-08-T1 | 08 | 7 | CPU-01 | T-02-24 | vectors + faults | quick; gate | ⬜ pending |
| 02-08-T2 | 08 | 7 | CPU-01 | T-02-24, T-02-25 | vectors + faults | gate (`faults=33/33`) | ⬜ pending |
| 02-08-T3 | 08 | 7 | CPU-01, CPU-02 | T-02-24 | vectors + faults | gate (`faults=35/35`); `dash tools/gate.sh` | ⬜ pending |
| 02-09-T1 | 09 | 8 | CPU-03 | T-02-27 | vectors + fault | quick; gate | ⬜ pending |
| 02-09-T2 | 09 | 8 | CPU-03 | T-02-27 | vectors + faults | gate (`faults=40/40`) | ⬜ pending |
| 02-09-T3 | 09 | 8 | CPU-01 | T-02-26, T-02-28 | sweep + faults | quick (cpu-sweep listed); gate (`faults=42/42`) | ⬜ pending |
| 02-10-T1 | 10 | 9 | CPU-02, CPU-05 | T-02-29, T-02-30 | vectors + fault | quick; gate | ⬜ pending |
| 02-10-T2 | 10 | 9 | CPU-02 | T-02-29, T-02-31 | vectors + faults | gate (`faults=50/50`) | ⬜ pending |
| 02-10-T3 | 10 | 9 | CPU-02, CPU-04 | T-02-29 | vectors + faults | gate (`faults=52/52`); `dash tools/gate.sh` | ⬜ pending |
| 02-11-T1 | 11 | 10 | CPU-02 | T-02-34 | vectors + fault | quick; gate | ⬜ pending |
| 02-11-T2 | 11 | 10 | CPU-02, CPU-05 | T-02-32, T-02-34 | vectors + faults | gate (`faults=62/62`, `rows-pending=0`, `policies-pending=0`) | ⬜ pending |
| 02-11-T3 | 11 | 10 | CPU-05 | T-02-32, T-02-33 | fuzz + unit | quick (cpu-fuzz listed); gate; `dash tools/gate.sh` | ⬜ pending |
| 02-12-T1 | 12 | 11 | CPU-01 | T-02-35, T-02-36 | blind review | gate (`second-read=1/12`) | ⬜ pending |
| 02-12-T2 | 12 | 11 | CPU-01 | T-02-35, T-02-37 | blind review | gate (`second-read=5/12`) | ⬜ pending |
| 02-12-T3 | 12 | 11 | PKG-02 | T-02-38 | budget measurement | gate; `dash tools/gate.sh` | ⬜ pending |
| 02-13-T1 | 13 | 12 | CPU-01, CPU-04 | T-02-39 | blind review | gate (`second-read=6/12`) | ⬜ pending |
| 02-13-T2 | 13 | 12 | CPU-01..05 | T-02-39, T-02-41 | blind review | gate (`second-read=12/12`); reviewed set equals vector-file set | ⬜ pending |
| 02-13-T3 | 13 | 12 | all 8 | T-02-40 | phase evidence | gate (all pending stats 0); `dash tools/gate.sh` | ⬜ pending |

Requirement coverage target (from research):

| Requirement | Automated check | Where | Status |
|-------------|-----------------|-------|--------|
| CPU-01 | vectors + decode sweep + vector-data checker + arithmetic/flag/cycle faults | ctest dev+release, gate `cp1610-vectors`, `c-faults` | ⬜ pending |
| CPU-02 | POL-INT/BUS/HLT/LINE policy vectors, one fault each | `c-tests`, `c-faults` | ⬜ pending |
| CPU-03 | SDBD modes 1-5 vectors, refusal vectors, SDBD faults | `c-tests`, `c-faults` | ⬜ pending |
| CPU-04 | stack, auto-increment, reset vectors and faults | `c-tests`, `c-faults` | ⬜ pending |
| CPU-05 | `cpu-bus` suite (exact offsets only where a printed figure exists), `state_validate`, determinism, fuzz | `c-tests` | ⬜ pending |
| API-05 | `c-purity` + ctest `core_symbols` + 13 `core-purity-*` controls | gate, ctest | ⬜ pending |
| PKG-02 | `sh tools/gate.sh` runs the C steps; ci-parity unchanged; ninja/cc prechecks | gate, CI | ⬜ pending |
| RUN-03 | `tests-inventory` over TAP + NONCE + ids + `ctest -N`; ~10-12 `tests-inventory-*` controls | gate, control runner | ⬜ pending |

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
