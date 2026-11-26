---
phase: "18"
slug: "the-inspection-surface"
status: draft
nyquist_compliant: false
wave_0_complete: false
created: "2026-11-26"
---

# Phase 18 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | the project's own TAP shell suites, one `ctest` test per suite (`tests/CMakeLists.txt:2-4`), under the `dev` and `release` presets |
| **Config file** | `CMakePresets.json`; `tests/CMakeLists.txt` registers the suites; `data/expected/tests/*.ids` are the id inventories |
| **Quick run command** | `ctest --preset dev -R 'runner-process\|asm' --output-on-failure` |
| **Full suite command** | `sh tools/gate.sh` and `dash tools/gate.sh` (the `GUARDRAILS-v0.4` §4 definition of done) |
| **Estimated runtime** | hosted `gate_s` median 164 at the Phase 17 start commit; this phase's line is 2.96 hosted cpu-s |

---

## Sampling Rate

- **After every task commit:** `ctest --preset dev -R 'runner-process|asm' --output-on-failure`
- **After every plan wave:** `sh tools/gate.sh` and `dash tools/gate.sh`, both ending `gate: PASS` with the plan's predicted token string byte for byte
- **Before `/gsd:verify-work`:** the full gate plus one `sh tools/gate-linux.sh`; copy `build/gate-linux.log` aside first (lesson 18), one container gate at a time
- **Max feedback latency:** 300 s

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 18-01-T1 | 01 | 1 | TOOL-02, TOOL-09 | T-18-01 | the frozen document cannot be retro-fitted to the code | verify | `git merge-base --is-ancestor "$(git log --format=%H -1 -- docs/inspection.md)" "$(git log --format=%H --diff-filter=A -1 -- tools/inspect)"` | ❌ W0 | ⬜ pending |
| 18-02-T1 | 02 | 2 | TOOL-02 | T-18-02 | no new core surface; the tool opens its own file | build | `cmake --build --preset dev --target ivs_inspect` then `ivs_inspect --help` lists the four subcommands | ❌ W0 | ⬜ pending |
| 18-02-T2 | 02 | 2 | TOOL-02 | T-18-03 | the trace reconciles with the report rather than restating it | suite | `ctest --preset dev -R runner-process` — the trace ids assert records > 0, non-decreasing ticks, last tick == `step.<k>.now_ticks`, and nine `digest.*` lines equal to the untraced run | ❌ W0 | ⬜ pending |
| 18-02-T3 | 02 | 2 | TOOL-02 | T-18-04 | a breakpoint cannot alter state | suite | the same suite: the hit/non-hit pair `(1, 0)` plus `digest.*` equality under a breakpoint | ❌ W0 | ⬜ pending |
| 18-02-T4 | 02 | 2 | TOOL-09 | T-18-05 | the output cannot enter the tree | suite | the same suite: `inspect-output-tree` refused inside the tree, `refs/` accepted, an existing file refused; `git ls-files \| grep -E '\.ivst$'` empty | ❌ W0 | ⬜ pending |
| 18-02-T5 | 02 | 2 | TOOL-09 | T-18-06 | the rights row precedes the bytes | step | `sh tools/check/steps/rights.sh` (row ↔ id ↔ doc), `files` (the `ivst` refusal and its control), `hygiene` (binary-by-name) | ❌ W0 | ⬜ pending |
| 18-02-T6 | 02 | 2 | TOOL-02 | T-18-07 | build-only, proved both ways | step | `pkg-selftest=<n>/<n>` includes the absence case; `release-selftest=<n>/<n>` includes the member refusal; `prefix-files=11` | ❌ W0 | ⬜ pending |
| 18-03-T1 | 03 | 3 | TOOL-02 | T-18-08 | the round trip is against the assembler, not against itself | suite | `ctest --preset dev -R '^asm$'` — disassemble, re-assemble, bytes equal, `words > 0`, a two-word form and an immediate present, constant-mnemonic stand-in failing | ❌ W0 | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] `docs/inspection.md` — wave 1, alone, with the ancestry proof (W5); its `## Refusal tokens` block is what the coverage id reads
- [ ] `tools/inspect/` + `tools/inspect/CMakeLists.txt` — wave 2
- [ ] `data/expected/tests/runner-process.ids`, `data/expected/tests/asm.ids` — the new ids
- [ ] `data/rights/inspection-output.json`, `data/expected/rights.ids`, `docs/RIGHTS.md` row — one commit
- [ ] `tools/check/binary-ext.txt`, `.gitignore` — the same commit
- [ ] `tools/check/controls/files-unlisted-binary-ivst/` and `tools/check/controls/c-build-unlisted-inspect-source/`
- [ ] `tools/check/integrity.cmake` — `inspect` joins the tools alternation so SPDX/ASCII coverage is not silently lost
- [ ] `data/expected/pkg-selftests.ids` and `data/expected/release-assets-selftests.ids` — one case each
- [ ] `data/expected/important-gates.ids` — one `mutant` row per stand-in case

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| none | — | — | Every behavior this phase produces is judged by a committed id, a control, an in-step selftest or the gate's own token line. The one judgement a tool cannot make — whether the frozen document states the right contract — was made in the discuss record and is not re-asked. |

*No `data/expected/handoffs.ids` row is added: this project forbids a handoff row for something a tool can check.*

---

## Validation Sign-Off

- [ ] All tasks have `<automated>` verify or Wave 0 dependencies
- [ ] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references
- [ ] No watch-mode flags
- [ ] Feedback latency < 300s
- [ ] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
