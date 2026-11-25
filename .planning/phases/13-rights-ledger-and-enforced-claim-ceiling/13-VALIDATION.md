---
phase: "13"
slug: "rights-ledger-and-enforced-claim-ceiling"
# status lifecycle: draft (seeded by plan-phase) → validated (set by validate-phase §6)
# audit-milestone §5.5 distinguishes NOT-VALIDATED (draft) from PARTIAL (validated + nyquist_compliant: false)
status: draft
nyquist_compliant: false
wave_0_complete: false
created: "2026-11-26"
---

# Phase 13 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | The project's own gate harness: 38 POSIX `sh` steps + CMake `-P` checkers + negative-control trees. No external test framework. |
| **Config file** | `tools/gate.sh` (runner), `data/expected/gate-steps/*.ids` (step inventory), `data/expected/controls/*.ids` (control inventory), `data/expected/*.ids` (per-suite baselines) |
| **Quick run command** | `git ls-files > /tmp/fl && ROOT="$PWD" CHECK_DIR="$PWD/tools/check" FILELIST=/tmp/fl sh tools/check/steps/<step>.sh` |
| **Full suite command** | `sh tools/gate.sh` (the local fast path, same as the PR gate) |
| **Estimated runtime** | ~120 s full gate (`gate_s` last-five median 121); each single step < 2 s except `files` (~2 s) |

---

## Sampling Rate

- **After every task commit:** run the single affected step directly (all < 2 s).
- **After every plan wave:** `sh tools/check/run-controls.sh` — the fastest way to catch a control-overlay mistake.
- **Before `/gsd:verify-work`:** `sh tools/gate.sh` full suite green, `steps=38/38`, `faults=238/238`, `goldens=29`.
- **Max feedback latency:** ~120 seconds (full gate); ~2 seconds per step.

---

## Per-Task Verification Map

Filled by the planner/executor as plans land. Every row's automated command must be a real, runnable command with a stated failing direction in its plan task.

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 13-01-01 | 01 | 1 | RTS-01 | — | N/A | unit (step) | `sh tools/check/steps/manifests.sh` | ❌ W0 | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] `data/rights/*.json` (10 rows) — every rights id depends on these existing
- [ ] `data/expected/rights.ids`, `data/expected/rights-pinned.ids` — membership and pins
- [ ] `tools/check/schema/rights.txt` (+ hook) or the inline equivalent — the ledger checker
- [ ] `tools/check/controls/_good/tree/data/rights/*.json` + its `rights.ids` / `rights-pinned.ids` — the toy tree must pass the new checks
- [ ] `data/expected/runner-selftest.ids` — the four selftest case names (EV-07 deviation limb)
- [ ] `data/expected/controls/rights.ids` + `data/expected/controls/claims.ids` — inventory ids for the new control dirs
- [ ] Framework install: none — the harness is the project's own.

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| none | — | — | All phase behaviors have automated verification. |

---

## Validation Sign-Off

- [ ] All tasks have `<automated>` verify or Wave 0 dependencies
- [ ] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references
- [ ] No watch-mode flags
- [ ] Feedback latency < 120s
- [ ] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
