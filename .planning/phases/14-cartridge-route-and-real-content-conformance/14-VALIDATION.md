---
phase: "14"
slug: "cartridge-route-and-real-content-conformance"
# status lifecycle: draft (seeded by plan-phase) → validated (set by validate-phase §6)
# audit-milestone §5.5 distinguishes NOT-VALIDATED (draft) from PARTIAL (validated + nyquist_compliant: false) (#2117)
status: draft
nyquist_compliant: false
wave_0_complete: false
created: "2026-11-26"
---

# Phase 14 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.
> Filled by the planner from the four plans' own `<verify>` blocks; `status`, `nyquist_compliant`,
> `wave_0_complete` and the sign-off stay for plan 14-04 Task 2, which is where the measurements exist.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | CTest over 22 registered suites plus the 38-step gate (`tools/gate.sh`); 18 in-process TAP binaries, 4 shell TAP drivers (`pack-tap.sh`, `asm-tap.sh`, `runner-process-tap.sh`, `headers-tap.sh`) |
| **Config file** | `CMakePresets.json` (presets `dev`/`release`), `tests/CMakeLists.txt`, `data/expected/tests/*.ids` (one expected-id file per suite) |
| **Quick run command** | `ctest --preset dev -R '^(pack|runner-process|state|run|stic|cpu-vectors)$' --output-on-failure` |
| **Full suite command** | `sh tools/gate.sh` (`sh tools/gate-linux.sh` for the Ubuntu substance) |
| **Estimated runtime** | ~14 seconds for the quick run; the full gate measured `gate-time total=81` on this host at plan time |

---

## Sampling Rate

- **After every task commit:** the quick run above.
- **After every plan wave:** `sh tools/gate.sh`.
- **Before `/gsd:verify-work`:** the full gate green under both `sh` and `dash`, plus one container run.
- **Max feedback latency:** 14 seconds (quick run).

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 14-01-T1 | 01 | 1 | TOOL-07 | T-14-01 / T-14-02 / T-14-03 / T-14-04 / T-14-05 / T-14-06 | the size arithmetic is bounded before any multiplication, the output parent is resolved physically, the open is exclusive, and a failed write leaves no file | integration (process) | wrap 8192 zero bytes at `5000`, load as `content cart user`, `grep -x -F "slot.cart.sha256 <printed>"` | ✅ exists (new case, existing suites) | ⬜ pending |
| 14-01-T2 | 01 | 1 | TOOL-07 | T-14-01 / T-14-03 / T-14-05 | each refusal exits 1 with one token line and no output file; the existing-output case leaves the file byte-identical | integration (process) | `ctest --preset dev -R '^pack$'` → 23 ids, no `not ok` | ❌ W0 (9 new ids) | ⬜ pending |
| 14-02-T1 | 02 | 1 | CPU-06 | T-14-07 / T-14-08 / T-14-09 / T-14-10 / T-14-11 | the profile scalar governs the machine, both settings are explicit, and the literals are recomputed by a second path | in-process | `ctest --preset dev -R '^(state\|run\|stic\|cpu-vectors)$'` green; the five pre-flip literals gone; `"config": {` count 7 | ✅ exists | ⬜ pending |
| 14-02-T2 | 02 | 1 | CPU-06 | T-14-10 / T-14-11 | an observation moves a default, never a tier; the log's shape and its `unknown-default-chosen: ` prefix hold | registry (gate step) | `sh tools/check/steps/discrepancies.sh` → `discrepancies=110 pending-resolvers=20 open=101` | ✅ exists | ⬜ pending |
| 14-03-T1 | 03 | 2 | SCN-03 | T-14-12 / T-14-13 / T-14-16 | all six derived outputs refused with one line each and no file; the snapshot hook is accepted, cuts, and writes nothing | integration (process) | `ctest --preset dev -R '^runner-process$'` → 136 ids, no `not ok` | ❌ W0 (1 new id, 6th argument) | ⬜ pending |
| 14-03-T2 | 03 | 2 | SCN-03 | T-14-14 / T-14-16 | the digest equality is live: one flipped input byte changes the printed digest and `slot.cart.sha256` | integration (process, 2 runs) | same suite → 137 ids | ❌ W0 (1 new id) | ⬜ pending |
| 14-03-T3 | 03 | 2 | SCN-03 | T-14-13 / T-14-15 / T-14-16 | the cartridge is an option, the lock keeps two roles, and the pass path is shown able to fail by a one-byte EXEC mutation | integration (process) | same suite → 138 ids; `files` step `rights-pinned=4/4 binaries-listed=0` | ❌ W0 (1 new id) | ⬜ pending |
| 14-04-T1 | 04 | 3 | TOOL-07 / SCN-03 / CPU-06 | T-14-17 / T-14-21 | a flip and its row move together in both directions; the toy tree's derived set stays closed | registry (gate step) | `important-gates` → `gates=117 lines=357`; toy tree `gates=46 lines=49`; `run-controls.sh` → `controls=402/402` | ✅ exists | ⬜ pending |
| 14-04-T2 | 04 | 3 | TOOL-07 / SCN-03 / CPU-06 | T-14-18 / T-14-19 / T-14-20 | the closing line is measured, not quoted; one cost verdict with its arithmetic; one container gate at a time | integration (two shells + container) | `sh`/`dash` `tail -1` byte-identical, the predicted token string, `gate: PASS` in the container log | ✅ exists | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] `tools/flatwrap/{flatwrap.c,CMakeLists.txt}` — the tool and its target, with the SPDX header and the source-list writer (plan 14-01)
- [ ] `data/expected/flat-refusals.ids` — the seven pinned tokens, append-only (plan 14-01)
- [ ] `tests/pack-tap.sh` third argument and `tests/CMakeLists.txt`'s `pack` line (plan 14-01)
- [ ] `data/expected/tests/pack.ids` — nine ids (plan 14-01)
- [ ] `tools/check/controls/c-build-unlisted-flatwrap-source/` and its registry line (plan 14-01)
- [ ] `tools/check/controls/_good/tree/CMakeLists.txt` — the seventh toy source list (plan 14-01)
- [ ] `data/expected/ivsn-literals.ids` — five re-pinned values (plan 14-02)
- [ ] `tests/runner-process-tap.sh` sixth argument and `tests/CMakeLists.txt`'s `runner-process` line (plan 14-03)
- [ ] `data/expected/tests/runner-process.ids` — three ids (plan 14-03)
- [ ] No new framework, no new suite, no new gate step: every item above is a file, a case or an id inside what exists

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| — | — | None: the phase's ceiling is no claim, so nothing is handed off | — |

*To be replaced by "All phase behaviors have automated verification." at the 14-04 sign-off.*

---

## Validation Sign-Off

- [ ] All tasks have `<automated>` verify or Wave 0 dependencies
- [ ] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references
- [ ] No watch-mode flags
- [ ] Feedback latency < 14s
- [ ] `nyquist_compliant: true` set in frontmatter

**Approval:** pending (plan 14-04 Task 2)
