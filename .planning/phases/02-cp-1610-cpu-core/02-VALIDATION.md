---
phase: "2"
slug: "cp-1610-cpu-core"
status: draft
nyquist_compliant: false
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

Task ids are assigned by the plans; each plan's tasks carry `<automated>` commands drawn from the rows below. Requirement coverage target (from research):

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

- [ ] All tasks have `<automated>` verify or Wave 0 dependencies
- [ ] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references
- [ ] No watch-mode flags
- [ ] Feedback latency < 30s
- [ ] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
