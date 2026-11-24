---
phase: "09"
slug: "registered-evidence-and-gate-cost"
# status lifecycle: draft (seeded by plan-phase) → validated (set by validate-phase §6)
status: draft
nyquist_compliant: false
wave_0_complete: false
created: "2026-11-24"
---

# Phase 9 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | CTest over the in-repo TAP harness (`tests/harness/`), plus the shell gate driver `tools/gate.sh` and the control runner `tools/check/run-controls.sh` |
| **Config file** | `CMakePresets.json` (dev/release presets); suite registration in `tests/CMakeLists.txt`; expected-id inventories in `data/expected/tests/*.ids` |
| **Quick run command** | `sh tools/check/run-controls.sh` for a step/control change; `cmake --build --preset dev && ctest --preset dev -R '^runner-process$'` for a runner-suite change; `ctest --preset dev -R '^state$'` for a state-suite change |
| **Full suite command** | `sh tools/gate.sh` **and** `dash tools/gate.sh`; container: `sh tools/gate-linux.sh >/dev/null 2>&1 && grep -q '^gate: PASS' build/gate-linux.log` |
| **Estimated runtime** | local gate ≈ 220 s of cpu, ≈ 120 s wall; container gate ≈ 180 s |

---

## Sampling Rate

- **After every task commit:** the quick command for the file class the task touched.
- **After every plan wave:** `sh tools/gate.sh` and `dash tools/gate.sh`, both ending `gate: PASS` with the plan's predicted token.
- **At each wave boundary:** one container gate, one at a time (lesson 18), `build/gate-linux.log` copied before anything else runs; if docker is unavailable write a visible `not run` line — never a pass (lesson 24).
- **Before `/gsd:verify-work`:** full suite green on the final tree under `sh`, `dash` and the container.
- **Max feedback latency:** ~25 s (the preflight partition's whole point: a self-consistency verdict before the build-and-suite work).

---

## Per-Task Verification Map

Filled by the planner from the PLAN files' `<verify>` blocks. The map below is the
phase-level contract; per-task rows are added when the plans exist.

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 09-??-?? | ?? | A1 | EV-01 | — | no check weakened to go faster | suite + static | `ctest --preset dev -R '^runner-process$'` and `--preset release` | ✅ | ⬜ pending |
| 09-??-?? | ?? | A1 | EV-01 | — | whole-instance comparison kept on every refusal row | suite | `ctest --preset dev -R '^state$'` and `--preset release` | ✅ | ⬜ pending |
| 09-??-?? | ?? | A2 | EV-02 | — | registry names only mechanisms that already run | gate step + controls | `sh tools/check/run-controls.sh` → `controls=` predicted | ✅ | ⬜ pending |
| 09-??-?? | ?? | A2 | EV-03 | — | each control fails for its intended reason only | controls | `sh tools/check/run-controls.sh` | ✅ (3 new) | ⬜ pending |
| 09-??-?? | ?? | A2 | EV-04 | — | a Complete requirement cannot lose coverage silently | gate step + controls | `sh tools/check/run-controls.sh` with the two new controls | ❌ W0 | ⬜ pending |
| 09-??-?? | ?? | A2 | EV-01/EV-04 | — | normalised gate output identical before and after the partition | end-to-end | `sh tools/gate.sh` and `dash tools/gate.sh`, diffed against the committed pre-change capture | ❌ W0 | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] `tools/check/controls/_good/tree/.planning/REQUIREMENTS.md` — the toy requirement record EV-04 reads (the good tree's `.planning/` currently holds only `PROJECT.md` and `ci-runs.tsv`)
- [ ] `tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md` — the second record, minimal and consistent with the toy registry by construction
- [ ] `data/expected/important-gates-excluded.ids` (A-01) and its good-tree copy
- [ ] The EV-04 controls (a Complete requirement with no registry line; a drifted toy registry) plus their lines in the controls inventory
- [ ] `DEVIATION` files for the nine `important-gates-*/` overlays (A-09)
- [ ] The pre-change gate capture (`sh` and `dash`) that the partition's normalised diff is taken against
- [ ] The derivation-equality judgement's committed scratch stand-in (a one-value-changed scenario body)
- [ ] Framework install: none — ctest, cmake, ninja and a C17 compiler are already required by `tools/gate.sh`

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| none | — | — | — |

All phase behaviours have automated verification. Two items are *orchestrator-owned*
rather than manual: the hosted row (`tools/ci-record.sh`, after the A1 push) and the
repository settings (branch and tag rulesets, applied with `gh`). Neither is a check a
tool cannot judge; both are actions no executor may take.

---

## Validation Sign-Off

- [ ] All tasks have `<automated>` verify or Wave 0 dependencies
- [ ] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references
- [ ] No watch-mode flags
- [ ] Feedback latency < 25 s for the preflight verdict
- [ ] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
