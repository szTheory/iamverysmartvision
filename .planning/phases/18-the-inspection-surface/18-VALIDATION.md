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
| **Estimated runtime** | hosted `gate_s` median 164 at the Phase 17 start commit; the frozen line is 2.96 hosted cpu-s, and `18-02`'s recomputation over the items the plans actually ship is 3.59 - recorded there as a §2.4 repricing candidate, not edited into the frozen figure |

---

## Sampling Rate

- **After every task commit:** `ctest --preset dev -R 'runner-process|asm' --output-on-failure`
- **After every plan wave:** `sh tools/gate.sh` and `dash tools/gate.sh`, both ending `gate: PASS` with the plan's predicted token string byte for byte
- **Before `/gsd:verify-work`:** the full gate plus one `sh tools/gate-linux.sh`; copy `build/gate-linux.log` aside first (lesson 18), one container gate at a time
- **Max feedback latency:** 300 s

---

## Per-Task Verification Map

The three plans ship **five tasks**, not ten: `18-02` is one tracer task (the target, the two passes, the reconciliation, the output policy, the rights row and four `runner-process` ids), one expansion task (breakpoints, the `step` surface, the token-coverage id and four stand-ins) and one task that closes the two build-only proofs and the registry. The earlier revision of this table listed `18-02-T1..T6`, which no plan ever shipped.

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 18-01-T1 | 01 | 1 | TOOL-02, TOOL-09 | T-18-01, T-18-02 | the frozen document cannot be retro-fitted to the code, and it is written to conditions rather than to a scanner | verify | the plan's own `<automated>` block: `git log --diff-filter=A -- docs/inspection.md` yields one commit, that commit's only file is the page, the freeze version is a byte prefix of the page now, and `git merge-base --is-ancestor <freeze-commit> <first tools/inspect commit>` is the criterion `18-02` re-runs | ❌ W0 | ⬜ pending |
| 18-02-T1 | 02 | 2 | TOOL-02, TOOL-09 | T-18-03, T-18-04, T-18-08 | the report is the library's bytes captured through `write_report` (no record is ever constructed), the trace reconciles with that text, and no output can enter the tree | build + suite + step | `cmake --build --preset dev --target ivs_inspect`; the four `runner-process.inspect.*` ids; the three in-tree spellings refused and `refs/` accepted; `rights_scan_naming` clean over `tools/inspect/*.c` and the three edited pages; `sh tools/check/steps/rights.sh`; `git ls-files \| grep -E '\.ivst$'` empty | ❌ W0 | ⬜ pending |
| 18-02-T2 | 02 | 2 | TOOL-02 | T-18-05, T-18-07 | a check that cannot pass vacuously, and a bounded trace that cannot silently truncate | suite | `ctest --preset dev -R runner-process` - the breakpoint pair `(1, 0)` with the in-run and file-derived counts reconciling, byte-identical digests and access records under a breakpoint, the `step` unit count equal to the trace's, the token coverage in both directions, and four stand-ins each judged by the same function the real case uses. The trace ids assert `records > 0`, ticks non-decreasing **and not all equal** (at least two distinct tick values), the end record's tick equal to the report's own `step.<k>.now_ticks`, and the nine `digest.*` lines equal to the untraced run | ❌ W0 | ⬜ pending |
| 18-02-T3 | 02 | 2 | TOOL-02, TOOL-09 | T-18-06 | the tool cannot acquire an install rule or a release membership unnoticed, and a row cannot outrun its requirement flip | step | `pkg-selftest=21/21` includes `inspect-absent` (`pkg-inventory-extra: bin/ivs-inspect`); `release-selftest=13/13` includes `inspect-ivst-member`; `prefix-files=11` with `data/expected/pkg-install-files.ids` untouched; `important-gates` prints `gates=121 lines=371` with no finding; TOOL-02 and TOOL-09 `Complete` in both the checkbox and the traceability row | ❌ W0 | ⬜ pending |
| 18-03-T1 | 03 | 3 | TOOL-02 | T-18-09, T-18-10, T-18-11 | the round trip is against the assembler, not against itself, and every `ivs_asm` call names a relative source path so the verdict is the words and not the usage rule | suite | `ctest --preset dev -R '^asm$'` and the release preset - disassemble, re-assemble, words equal, `words > 0`, a two-word form and an immediate present, at least three distinct mnemonics, `inspect-dis-row` fail-closed, constant-mnemonic stand-in failing the byte equality, determinism asserted once | ❌ W0 | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] `docs/inspection.md` — wave 1, alone, with the ancestry proof (W5); its `## Refusal tokens` block is what the token-coverage id reads, and its `## Command surface` pins `--help` because `18-02` tests it
- [ ] `tools/inspect/` + `tools/inspect/CMakeLists.txt` — wave 2, with `inspect_dis.c` joining `IVS_INSPECT_SOURCES` in wave 3
- [ ] `data/expected/tests/runner-process.ids` — four ids in wave 2 task 1, nine more in wave 2 task 2; `data/expected/tests/asm.ids` — six ids in wave 3
- [ ] `data/rights/inspection-output.json`, `data/expected/rights.ids`, `docs/RIGHTS.md` row — one commit
- [ ] `tools/check/binary-ext.txt`, `.gitignore` — the same commit
- [ ] `tools/check/controls/files-unlisted-binary-ivst/` and `data/expected/controls/rights.ids` — the phase's **one** control
- [ ] `tools/check/integrity.cmake` — `inspect` joins the tools alternation so SPDX/ASCII coverage is not silently lost
- [ ] `data/expected/pkg-selftests.ids` and `data/expected/release-assets-selftests.ids` — one case each
- [ ] `data/expected/important-gates.ids` — five rows in wave 2 (four `mutant` under the existing gate name `runner-process`, one `control` under `files`) and one `asm mutant` row in wave 3

### Recorded deviation from `18-CONTEXT.md`'s control count

`18-CONTEXT.md`'s specificity 3 prices `controls` at **+1** and its specificity 1 parenthetical names both a `files-unlisted-*` control and a `c-build` unlisted-source control. The plans ship **one** control, `files-unlisted-binary-ivst`, and this file records the reading rather than leaving the difference implicit:

- `GUARDRAILS-v0.4` **W3** names a `files-unlisted-*` control for a trace artifact explicitly, and `18-02` adds it.
- The `c-build` unlisted-source limb is already covered by four existing per-tool controls, so a fifth would re-prove an existing mechanism for no new evidence (the same reasoning `tools/check/binary-ext.txt` records for `dll` and `bundle`).
- The result is `controls` **+1**, which is the number `18-CONTEXT.md` prices. `steps` stays 38 and `faults` stays 238; the toy source list and the module line are edited in place, not added.

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|-------------|-------------|------------|-------------------|
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
