---
phase: "19"
slug: "discriminating-the-display-path"
status: draft
nyquist_compliant: false
wave_0_complete: false
created: "2026-11-27"
---

# Phase 19 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | the project's own TAP shell suites under `ctest` (`dev`, `release` presets); the `stic` suite carries the discrimination ids |
| **Config file** | `tests/CMakeLists.txt`; `data/expected/tests/stic*.ids` are the id inventories |
| **Quick run command** | `ctest --preset dev -R '^stic' --output-on-failure` |
| **Full suite command** | `sh tools/gate.sh` **then** `dash tools/gate.sh` — **sequentially, never concurrently** (lesson 33: both write `build/`, and a concurrent pair reports a phantom `c-build-failed`) |
| **Estimated runtime** | hosted `gate_s` median 164; this phase's line is 1.64 hosted cpu-s |

---

## Sampling Rate

- **After every task commit:** `ctest --preset dev -R '^stic' --output-on-failure`
- **After every plan wave:** the two gate runs, sequentially, `cmp`-ing their summary lines
- **Before `/gsd:verify-work`:** the full gate plus one `sh tools/gate-linux.sh` (copy `build/gate-linux.log` aside first); if the daemon is unavailable, a visible `not run` line, never a pass
- **Max feedback latency:** 300 s

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 19-01-T1 | 01 | 1 | VID-07 | T-19-01 | each guest moves under its own field only | suite | `ctest --preset dev -R '^stic'` — four guests, one observable each, one liveness guard each, a committed stand-in failing each guard | ❌ W0 | ⬜ pending |
| 19-01-T2 | 01 | 1 | VID-07 | T-19-02 | the latch rule is a profile field before it is swept | step | `sh tools/check/steps/profile.sh` and `state.sh` — the new field row, the parameter digest and **every** `ivsn-literals.ids` literal retargeted in one commit | ❌ W0 | ⬜ pending |
| 19-02-T1 | 02 | 2 | VID-07 | T-19-03 | the probe cannot leave `refs/` and cannot gate the phase | suite | `ctest --preset dev -R '^stic'` — the sweep is `skip-ok` (exit 13, `missing-fixture`) in both inventories and bounded | ❌ W0 | ⬜ pending |
| 19-03-T1 | 03 | 3 | VID-07 | T-19-04 | the exit is decided by measurement, and the record cannot be improvised | verify | the diagonal matrix is complete (16 cells, each guest moves under exactly one field) with the `mutant` row shown able to fail; then Exit A's enumeration equals the commit's file list, or Exit B's record reads "default unchanged, observation appended, rivals named" | ❌ W0 | ⬜ pending |
| 19-03-T2 | 03 | 3 | VID-07 | T-19-05 | no claim, tier or golden moves | step | the gate token line: `claims=29 provisional=28 goldens=29 faults=238/238 steps=38/38 prefix-files=11`, and `unknown-default-chosen` still on the field | ✅ | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] four guest sources + four scenarios (project-authored, `ivs_asm`)
- [ ] the latch rule as a new profile field row with its tag and cite
- [ ] the `W1` chain retargeted in the same commit (parameter digest + every snapshot literal)
- [ ] the 16-cell matrix and the diagonal assertion with its `mutant` row
- [ ] the bounded `refs/`-only sweep with its `skip-ok` markers
- [ ] the C-row observation append in `data/discrepancies.json`

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| none | — | — | The four observables are machine state read at the seam; the sweep is a matrix of parameter rows; the exit is a recorded comparison. The one act no tool can perform — the D10-VID-08 hardware probe — is a handoff row with `evidence: none`, never a phase criterion. |

*No `data/expected/handoffs.ids` row is added by this phase: D10-VID-08 already exists and Phase 20 writes its packet.*

---

## Validation Sign-Off

- [ ] All tasks have `<automated>` verify or Wave 0 dependencies
- [ ] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references
- [ ] No watch-mode flags
- [ ] Feedback latency < 300s
- [ ] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
