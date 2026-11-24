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

Filled by the planner from the PLAN files' `<verify>` blocks (2026-11-24). Every row below
cites the plan and task that owns it; the `<verify>` block of that task is the authority and
carries the predicted token for the commit it ends.

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 09-01-1 | 09-01 | A1 | EV-01 | T-09-01, T-09-02, T-09-03 | the folded variants are judged against the frozen derivation and no skip is emitted | suite + static | `ctest --preset dev -R '^runner-process$'`, `--preset release`, then `sh`/`dash tools/gate.sh` with `reports=30 release-only=2 tests-skipped=4` | ✅ | ⬜ pending |
| 09-01-2 | 09-01 | A1 | EV-01 | T-09-01 | trim (a) keeps the live `zero` launch and three static judgements that can fail | suite + static | `sh tools/gate.sh` with `tests-executed=4954` and exactly one `^rt_refuse` line | ✅ | ⬜ pending |
| 09-01-3 | 09-01 | A1 | EV-01 | — | the mode of every run is visible in the output | suite | `# mutants:` line carries `variants=`/`variant-static=` under `sh` and `dash` | ✅ | ⬜ pending |
| 09-02-1 | 09-02 | A1 | EV-01 | T-09-04 | whole-instance comparison and `produced` kept on every refusal row | suite | `ctest --preset dev -R '^state$'`, `--preset release`, and the two guard-site positional checks | ✅ | ⬜ pending |
| 09-02-2 | 09-02 | A1 | EV-01 | T-09-05 | the unsealed offset set is unchanged and the acceptance refusal survives | suite | `ctest --preset dev -R '^state$'` plus the loop-bound and `c.accepted != 0` greps | ✅ | ⬜ pending |
| 09-02-3 | 09-02 | A1 | EV-01 | — | the trims are documented where a reader looks | gate | `sh`/`dash tools/gate.sh` with the unchanged token | ✅ | ⬜ pending |
| 09-03-1 | 09-03 | A1 | EV-01 | T-09-06 | every figure is recomputed from the committed record | gate + arithmetic | median recomputed from `.planning/ci-runs.tsv` equals the value printed in the entry | ✅ | ⬜ pending |
| 09-03-2 | 09-03 | A1 | EV-01 | T-09-06 | the exact pushed tree is green under three gate shapes | end-to-end | `sh`, `dash`, container gate; `git status --porcelain` clean | ✅ | ⬜ pending |
| 09-04-1 | 09-04 | A2 | EV-02 | T-09-07, T-09-08 | registry names only mechanisms that already run | gate step + controls | `sh`/`dash tools/gate.sh` with `gates=97 lines=316`, plus 7-additions/0-deletions per toy registry | ✅ | ⬜ pending |
| 09-04-2 | 09-04 | A2 | EV-02 | T-09-07, T-09-08 | the full 17-requirement registration with no new mechanism | gate step + controls | `sh`/`dash tools/gate.sh` with `gates=107 lines=330`; `run-controls.sh` `controls=380/380` | ✅ | ⬜ pending |
| 09-04-3 | 09-04 | A2 | EV-02 | T-09-07 | the registration commit is proved on its own terms and the row is an ancestor | evidence | non-comment row delta over `git diff <plan-base>..HEAD` = 24 added / 0 deleted (the real registry's `--numstat` also counts its `# <REQ>:` comment blocks); toy registries 17/0 by `--numstat`; `git merge-base --is-ancestor <row> HEAD` | ✅ | ⬜ pending |
| 09-05-1 | 09-05 | A2 | EV-04 | T-09-09, T-09-10 | a Complete requirement cannot lose coverage silently; the GUARDRAILS 2.3 registration deferral is recorded (D-09-06) | gate step | `sh`/`dash tools/gate.sh` green with `50 - 6 = 44 = registered` and a Task 1 file delta of exactly 4 against `git ls-tree -r --name-only HEAD^` (no absolute `files=` value: it is derived at run time); `09-05-SUMMARY.md` carries D-09-06 with the nine-mechanism enumeration, the frozen `lines=330` and its reversal condition | ❌ W0 (created here) | ⬜ pending |
| 09-05-2 | 09-05 | A2 | EV-04 | T-09-09 | both directions of the ledger contract are shown able to fail | controls | `run-controls.sh` `controls=381/381` with `control ok important-gates-complete-uncovered` | ❌ W0 (created here) | ⬜ pending |
| 09-06-1 | 09-06 | A2 | EV-04 | T-09-11 | a toy registry that drifts from the good tree is refused both ways | controls | `run-controls.sh` `controls=381/381`; nine `DEVIATION` files equal the measured `comm` output, each a single-line deviation; `cases=10` | ✅ | ⬜ pending |
| 09-06-2 | 09-06 | A2 | EV-03 | T-09-12 | a step that prints after its pass line is refused | controls + selftest | `good_one` guard silent on 381 controls; committed `st-pass-line-not-last` case in `runner-selftest.sh` prints `-- FAIL control-pass-line-not-last: profile` and `cases=11` | ✅ | ⬜ pending |
| 09-07-1 | 09-07 | A2 | EV-03 | T-09-13 | a truncated or case-mangled `SRC-` reference cannot pass | controls | `run-controls.sh` `controls=382/382`; 0 unregistered tokens on the pristine tree | ✅ | ⬜ pending |
| 09-07-2 | 09-07 | A2 | EV-03 | T-09-13 | a manifest output cannot point outside its fixture or `data/` | controls | `run-controls.sh` `controls=383/383`; the nine real manifests satisfy the rule | ✅ | ⬜ pending |
| 09-07-3 | 09-07 | A2 | EV-03 | T-09-14, T-09-15 | the build's generator runs under the gate and its two failures are distinguishable | controls + selftest | `run-controls.sh` `controls=384/384`; `_good/memmap` still ok; committed `st-memmap-generator-missing` case prints its token and `cases=12`; `literal-rows=80` unchanged | ✅ | ⬜ pending |
| 09-08-1 | 09-08 | A2 | EV-01/EV-04 | T-09-16 | the preserving condition has a committed left-hand side | evidence | both captures end `gate: PASS`, are normalised and exceed 100 lines | ❌ W0 (created here) | ⬜ pending |
| 09-08-2 | 09-08 | A2 | EV-01/EV-04 | T-09-16 | the partition runs cheap steps first and keeps the output order | end-to-end | one `gate-partition preflight=29 producers=9:` line with `29 + 9 = 38` (the split is derived from step source text, so the parts are asserted to still sum to the step count) and one `gate-preflight: PASS steps=29/29` line, token unchanged | ✅ | ⬜ pending |
| 09-08-3 | 09-08 | A2 | EV-01/EV-04 | T-09-16 | normalised gate output identical before and after the partition | end-to-end | `sh` and `dash` after-captures diffed empty against the committed captures after the **closed** derived-at-run-time normalisation (`step-time`/`gate-cpu`/`gate-time`/`gate-env` shapes plus `ci-runs=`, `files=`, `tracked=`, `hygiene-files=`, `history-commits=`); container gate | ❌ W0 (the capture is 09-08-1) | ⬜ pending |
| 09-09-1 | 09-09 | A2 | EV-01..EV-04 | T-09-17 | stale records are corrected by measurement, never rewritten | gate | `sh`/`dash tools/gate.sh` unchanged; the core diff is comment lines only | ✅ | ⬜ pending |
| 09-09-2 | 09-09 | A2 | EV-01..EV-04 | T-09-17 | the status flip and its ledger rows are inseparable | gate step | `sh`/`dash tools/gate.sh` green with ten ledger rows and four `Complete` | ✅ | ⬜ pending |
| 09-09-3 | 09-09 | A2 | EV-01..EV-04 | T-09-17 | the final tree is green under all three gate shapes | end-to-end | `sh`, `dash`, container gate lines recorded; tree clean | ✅ | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

Each item is created by the plan named beside it, inside that plan's first commit, so no task
depends on a scaffold that does not exist yet.

- [ ] `tools/check/controls/_good/tree/.planning/REQUIREMENTS.md` — the toy requirement record EV-04 reads (owner: 09-05 task 1)
- [ ] `tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md` — the second record, minimal and consistent with the toy registry by construction (owner: 09-05 task 1)
- [ ] `data/expected/important-gates-excluded.ids` (A-01) and its good-tree copy (owner: 09-05 task 1; four rows added by 09-09 task 2 with the status flip)
- [ ] The EV-04 mechanisms and their proofs: the `important-gates-complete-uncovered` control (owner: 09-05 task 2) and the toy-registry drift check with its two `runner-selftest` cases (owner: 09-06 task 1; a control directory cannot reach the control runner, recorded as a deviation from A-01's wording)
- [ ] The committed stand-ins for the two guards no control can reach: `st-pass-line-not-last` (owner: 09-06 task 2) and `st-memmap-generator-missing` (owner: 09-07 task 3), both cases in `tools/check/steps/runner-selftest.sh` using the copied-`CHECK_DIR` helper, taking `cases=` from 8 to 12
- [ ] `DEVIATION` files for the nine `important-gates-*/` overlays (A-09) (owner: 09-06 task 1)
- [ ] The pre-change gate capture (`sh` and `dash`) that the partition's normalised diff is taken against (owner: 09-08 task 1, committed before `tools/gate.sh` changes)
- [ ] The derivation-equality judgement's stand-in: landed as a permanent in-case negative half in 09-01 task 1 (a one-value-changed scenario body in the suite's own temp dir), which is stronger than a one-off scratch file and adds no test id (A-03)
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
