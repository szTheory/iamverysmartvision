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
| **Quick run command** | `IVS_TEST_OUT_DIR=<tmp> ctest --preset dev -R '^stic$' --output-on-failure` — the ids are read from `<tmp>/stic.txt`, whose grammar is `ok <n> <id>` with **no dash** (`tests/harness/ivs_test.h`); a passing suite is not echoed by ctest, so its stdout is never the source |
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
| 19-01-T1 | 01 | 1 | VID-07 | T-19-01 | G1 moves under its own field only, and its guard can fail | suite | `IVS_TEST_OUT_DIR=<tmp> ctest --preset dev -R '^stic$'` — **one** guest (G1, the tracer) with its registered observable, its liveness guard, its committed stand-in, the displayed-word equivalence and that id's stand-in; four ids ok in the suite's own result file | ❌ W0 | ⬜ pending |
| 19-01-T2 | 01 | 1 | VID-07 | T-19-02 | the latch rule is a profile field before it is swept, and the default reproduces today's bytes | step | `profile.sh`, `state.sh` and both gate shells — the new field row, C82's `evidence`, the parameter digest and **every** `ivsn-literals.ids` literal retargeted in one commit, with no golden, rule, claim or `discrepancies.ids` movement | ❌ W0 | ⬜ pending |
| 19-01-T3 | 01 | 1 | VID-07 | T-19-01 | the remaining three guests move under their own fields only | suite | `IVS_TEST_OUT_DIR=<tmp> ctest --preset dev -R '^stic$'` — G2, G3, G4 and their three judges; six ids ok, no profile row, literal or golden moved | ❌ W0 | ⬜ pending |
| 19-02-T1 | 02 | 2 | VID-07 | T-19-03 | the probe cannot leave `refs/`, refuses CI, and is bounded | suite | `IVS_TEST_OUT_DIR=<tmp> ctest --preset dev -R '^runner-process$'` and the same on `release` — the three `runner-process.disc-sweep.*` refusal ids, each run against a copy of the helper in a scratch root outside the work tree | ❌ W0 | ⬜ pending |
| 19-02-T2 | 02 | 2 | VID-07 | T-19-09 | the probe is a visible skip and nobody's exit criterion | suite + gate | `ctest --preset {dev,release} -R '^runner-process$'` — the case is `ok <n> runner-process.scenario.disc-sweep-probe # SKIP`, and the gate prints exactly two `-- SKIPPED <preset>/runner-process/runner-process.scenario.disc-sweep-probe: ...` lines | ❌ W0 | ⬜ pending |
| 19-03-T1 | 03 | 3 | VID-07 | T-19-04 | the exit is decided by measurement, and the record cannot be improvised | verify | the diagonal matrix is complete (16 cells, each guest moves under exactly one field) with `stic.judge.disc-diagonal-two-fields` shown able to fail, and both enumerations committed before the sweep ran | ❌ W0 | ⬜ pending |
| 19-03-T2 | 03 | 3 | VID-07 | T-19-05 | no claim, tier or golden moves, and the enumeration is honoured | step | the gate token line: `claims=29 provisional=28 goldens=29 faults=238/238 steps=38/38 prefix-files=11 controls=405/405`, `unknown-default-chosen` still on the field, and the commit's file list equal to the enumeration read from Task 1's blob for the exit stated as taken | ✅ | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] four guests authored as the suite's own decle-builder data in `tests/stic/stic_disc.c` (the `stic_progs.c` convention, no binary, nothing rights-bearing) — not `ivs_asm` sources plus scenarios, which is the form the three branches made unnecessary
- [ ] the latch rule as a new profile field row with its tag and cite, and C82's `evidence` extended in the same commit
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
