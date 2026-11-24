---
phase: "7"
slug: "first-visible-original-scenario"
status: draft
nyquist_compliant: true
wave_0_complete: false
created: "2026-11-13"
---

# Phase 7 - Validation Strategy

> Per-phase validation contract for feedback sampling during execution. Decisions: `07-CONTEXT.md` D-01..D-36 and R-1..R-5; code grounding, spec defects X1-X10, file inventory and the full check table V1-V20: `07-RESEARCH.md` (section "Validation Architecture", the authoritative copy of the table below); option analysis: `07-DISCUSS-*.md`.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | In-repo C harness (TAP) under CTest plus gate steps in sh/cmake; real `ivsr` in `runner-process`; new suite `asm` for `ivs_asm` |
| **Config file** | `CMakePresets.json` (`dev` ASan+UBSan, `release`), `tests/CMakeLists.txt`, `data/expected/tests/*.ids`, `data/expected/runner-scenarios.ids` |
| **Quick run command** | `cmake --build --preset dev && ctest --preset dev -R '^(asm|pack|runner|runner-process)$' --output-on-failure` (not yet executed) |
| **Full suite command** | `sh tools/gate.sh` and `dash tools/gate.sh`; container proxy `sh tools/gate-linux.sh` |
| **Baseline** | steps 36/36, about 344 controls, 222 faults, 2,216 test ids (4,432 across both presets), `tests-skipped=2` (becomes 4 after the SCN-02 wave); re-measure at plan start |
| **Gate time budget** | `.planning/CI-EFFICIENCY.md`; last-five hosted `r = cpu_s/build_s` median 8.02 against budget 7.8 and trigger 8.3, so the wave-0 lever runs before new tests |

---

## Sampling Rate

- **After every task commit:** the task's own verify command, plus `sh tools/gate.sh` when a step, inventory or registry changed.
- **After every plan:** `sh tools/gate.sh` and `dash tools/gate.sh` with the plan's exact counts; `sh tools/gate-linux.sh` for plans adding C, shell or cmake.
- **Oracle-first:** the derivation, frame literals and `.expect` text are committed before the program listing and before any implementation that renders or assembles them; the verifier checks `git log` order.
- **Before `/gsd-verify-work`:** gate green under sh, dash and gate-linux with final counts; hosted `gate_s`/`cpu_s` rows recorded by the orchestrator; cost checkpoints C-A/C-B/C-C evaluated.
- **Max feedback latency:** local quick samples under 10 s; hosted gate within the CI-EFFICIENCY budget.

---

## Per-Task Verification Map

Task ids are assigned by the plans; each plan task must cite the V-row(s) it proves. Rows (check, failing direction, owner hand-off) are in `07-RESEARCH.md` "Validation Architecture":

| V-row | Requirement / decision | Behaviour |
|-------|------------------------|-----------|
| V1 | SCN-01, D-28 | manifest, recipe hashing, licence, `redistributable: yes` |
| V2 | D-16..D-19 | `ivs_asm` correct (pairs, twins, regeneration, negatives, judge) |
| V3 | D-09, D-10 | stub-2 hand-off (`JD`, entry convention, `frame displayed`) |
| V4 | D-04, D-05, D-14 | guest-state progress predicate, `halted == no` |
| V5 | D-12, D-13 | sustained display, re-arm margin, mutants |
| V6 | D-21, D-23 | video differs exactly in the declared region (two literals, pair-region token, consumer diff) |
| V7 | D-22 | audio by derivation (write log, silent and constant-level windows) |
| V8 | D-23 | idle versus pass metamorphic judge |
| V9 | D-24 | scenario-shape lint ("not a title screen or hash alone") |
| V10 | D-25 | every channel's asserts can fail (mutants, `fails=`) |
| V11 | D-26, PKG-01 | installed-consumer replay against the runner golden |
| V12 | D-06 | repeatability, slicing, cwd independence, dev equals release |
| V13 | D-29 | SCN ids accepted by `important-gates`; names rule |
| V14 | D-27 | claim wording and text tripwire |
| V15 | SCN-02, D-30 | owner route labelled, owner manifests `redistributable: no` |
| V16 | D-31 | visible, counted SKIPPED scenario |
| V17 | D-31 | user-content consistency rule both directions |
| V18 | D-31 | owner-run helper refusals |
| V19 | D-31 | derived outputs refused for user content |
| V20 | D-34 | cost metric and checkpoints |

*Status per row is recorded in the plan SUMMARY files and VERIFICATION.md.*

---

## Wave 0 Requirements

- [ ] cost trigger recompute and the cp1610-vectors lever (no new tests before it lands)
- [ ] `asm` suite files and ids, `run.stub2.*`, `diag-mutations-scn.ids`, `scenario-pairs.ids`, consumer replay lines, helper tests (all created by their own plans; no framework to install)

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| Listen to the exported WAV | SCN-01 | subjective sound; never evidence | `tools/listen-demo.sh` (H7-1) |
| View exported frames | SCN-01 | subjective look; never evidence | `--frame-ppm` export (H7-2) |
| D10 hardware items | none | physical hardware | unchanged (H7-3) |
| D11 owner-local EXEC/GROM run | SCN-02 | owner-held files | `docs/owner-images.md` route, not performed (H7-4) |
| Publication | none | delegated flow | orchestrator (H7-5) |

---

## Validation Sign-Off

- [ ] All tasks have `<automated>` verify or Wave 0 dependencies
- [ ] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references
- [ ] No watch-mode flags
- [ ] Feedback latency within budget
- [x] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
