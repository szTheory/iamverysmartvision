---
phase: "17"
slug: "records-cost-window-and-the-count-gate-contract"
status: draft
nyquist_compliant: false
wave_0_complete: false
created: "2026-11-26"
---

# Phase 17 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | The project's own gate: 38 shell steps plus TAP suites under CTest in two presets (`dev`, `release`) |
| **Config file** | `tests/CMakeLists.txt` (suite registration), `tools/gate.sh` (step order), `data/expected/tests/*.ids` (id inventories) |
| **Quick run command** | `sh tools/gate.sh > /tmp/g.out 2>&1; tail -1 /tmp/g.out` (and the same with `dash`) |
| **Full suite command** | `sh tools/gate-linux.sh > /dev/null 2>&1; grep -q '^gate: PASS' build/gate-linux.log` |
| **Estimated runtime** | ~115 s hosted `gate_s` median at the plan's start commit; local cold build ~34 s |

---

## Sampling Rate

- **After every task commit:** the quick run command, under both `sh` and `dash`
- **After every plan wave:** the full suite command, one container gate at a time
- **Before `/gsd:verify-work`:** full suite green, `build/gate-linux.log` copied before anything else runs
- **Max feedback latency:** 200 s

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 17-01-01 | 01 | 1 | EV-08 (c1) | — | N/A | record | `awk -F'\t' 'NR>1{print $13}' .planning/ci-runs.tsv \| tail -5 \| sort -n \| sed -n 3p` prints `572` | ✅ | ⬜ pending |
| 17-01-02 | 01 | 1 | EV-08 (c1) | — | N/A | step | `sh tools/check/steps/ci-log.sh` reports `ci-runs=118` | ✅ | ⬜ pending |
| 17-02-01 | 02 | 2 | EV-08 (c2) | — | N/A | verify | `grep '^flatwrap-' data/expected/flat-refusals.ids > /tmp/a; grep '^flatwrap-' docs/formats/ivsc-v1.md > /tmp/b; diff /tmp/a /tmp/b` is silent | ✅ | ⬜ pending |
| 17-02-02 | 02 | 2 | EV-08 (c3) | — | N/A | suite | `runner-process.judge.flatwrap-shim` in `tests/runner-process-tap.sh`, both presets | ❌ W0 | ⬜ pending |
| 17-02-03 | 02 | 2 | EV-08 (c4) | — | N/A | verify | the corrected `RELEASE-OPS.md` (i) line occurs verbatim twice in `docs/packaging.md` | ✅ | ⬜ pending |
| 17-03-01 | 03 | 3 | EV-08 (c5) | — | N/A | registry | `important-gates` row `count-gate mutant slowest-eight EV-08` plus the `st-pass-line` row | ❌ W0 | ⬜ pending |
| 17-03-02 | 03 | 3 | EV-08 (c5) | — | N/A | step | the gate token line carries `faults=238/238`, `goldens=29`, `claims=29`, `provisional=28` | ✅ | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] `runner-process.judge.flatwrap-shim` — the id itself is the phase's deliverable (17-02)
- [ ] `count-gate mutant slowest-eight EV-08` — the registry row that carries lesson 32's mechanism (17-03)

*Everything else already exists: the suite, the two documents, the inventory and the step.*

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| The window is recomputed rather than quoted | EV-08 (c1) | A record, not a check: the phase record is the artifact | Print the three medians and the recipe before the first pricing sentence, then exactly one checkpoint token |
| The packaging letter and `RELEASE-OPS.md` (i) agree sentence for sentence | EV-08 (c4) | Two prose documents; no step pins their wording | Extract the (i) line and require two verbatim occurrences in `docs/packaging.md` |

*The container gate (`tools/gate-linux.sh`) records a visible `not run` line when docker is unavailable — never a pass (lesson 24).*

---

## Validation Sign-Off

- [ ] All tasks have `<automated>` verify or Wave 0 dependencies
- [ ] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references
- [ ] No watch-mode flags
- [ ] Feedback latency < 200s
- [ ] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
