---
phase: "15"
slug: "build-only-host-and-the-fake-frontend-harness"
# status lifecycle: draft (seeded by plan-phase) → validated (set by validate-phase §6)
# audit-milestone §5.5 distinguishes NOT-VALIDATED (draft) from PARTIAL (validated + nyquist_compliant: false) (#2117)
status: validated
nyquist_compliant: true
wave_0_complete: true
created: "2026-11-26"
---

# Phase 15 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | CTest over the registered suites plus the 38-step gate; the phase's ids live inside the existing `runner-process` suite (POSIX-sh TAP), never in a new suite |
| **Config file** | `CMakePresets.json` (`dev`: ASan+UBSan+Werror+fault controls; `release`: -O2 -DNDEBUG+Werror) and `data/expected/tests/*.ids` |
| **Quick run command** | `IVS_TEST_OUT_DIR=<dir> ctest --preset dev -R '^runner-process$'` |
| **Full suite command** | `sh tools/gate.sh` (`sh tools/gate-linux.sh` for the Ubuntu substance) |
| **Estimated runtime** | measured, not estimated: the quick run is **19.0-19.8 s** wall (two runs this session; the plan-time "≈29.5 s before this phase" figure is stale by measurement and is named as stale rather than carried); the full gate measured `gate-time total=74` on this host at the phase close (86-147 s at Phase 14's close, a range because this host's figure has varied by 70 % run to run) and **279 s** in the amd64-emulated container |

---

## Sampling Rate

- **After every task commit:** Run `IVS_TEST_OUT_DIR=<dir> ctest --preset dev -R '^runner-process$'` and read the trailer `# suite=runner-process executed=<n> skipped=<n> failed=0`.
- **After every plan wave:** Run `sh tools/gate.sh` and compare `tail -1` against the plan's predicted token line.
- **Before `/gsd:verify-work`:** Full suite must be green, `sh tools/hygiene.sh --tree` silent, and `git ls-files | grep -E '[.](ivsc|bin|rom|so|dylib|dll|bundle|cfg|ivsn)$'` empty.
- **Max feedback latency:** **19.0-19.8 s**, measured twice in plan 15-03's sign-off (`ctest --preset dev -R '^runner-process$'`, `Total Test time (real) = 19.01 sec` and `19.76 sec`, trailer `executed=144 skipped=2 failed=0`). The plan-time "≈29.5 s" figure is stale by measurement.

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 15-01-T1 | 01 | 1 | TOOL-08 | T-15-01, T-15-02, T-15-03, T-15-04, T-15-05, T-15-06 | The `dlopen` path comes from CMake's `$<TARGET_FILE:…>`, never the environment; the export set is linker-enforced; the shim's imports exclude every file, clock and environment name | process (TAP) + static symbol comparison | `cmake --build --preset dev && IVS_TEST_OUT_DIR=<dir> ctest --preset dev -R '^runner-process$'`; `nm -P -g --defined-only` set equals the nine ids; `sh tools/gate.sh` | ✅ | ✅ green |
| 15-01-T2 | 01 | 1 | TOOL-08 | T-15-02, T-15-03 | The export surface is refused by name in both directions and on both linkers; a disallowed import is refused by name | process (TAP) + static symbol comparison | `sh tools/check/host-symbols.sh <so>` → `-- host-symbols pass exported=9 imports=0`; the case's appended-id run must fail with `host-symbols-missing` | ✅ | ✅ green |
| 15-02-T1 | 02 | 2 | TOOL-08 | T-15-08, T-15-09, T-15-10 | The drive ends at exactly 806568; the poll carries absolute ticks and only the unconsumed suffix is submitted; one palette and one converter | process (TAP) | `IVS_TEST_OUT_DIR=<dir> ctest --preset dev -R '^runner-process$'` → `host.api`, `host.frames`, `host.unload` `ok`; trailer `executed=141 skipped=2 failed=0` | ✅ | ✅ green |
| 15-02-T2 | 02 | 2 | TOOL-08 | T-15-07 | The digest equality cannot pass on a stub or a constant: the empty poll differs, `digest.images` stays equal, and both stand-ins are refused for their own token | process (TAP) + judge self-controls | `IVS_TEST_OUT_DIR=<dir> ctest --preset dev -R '^runner-process$'` → `host.empty-poll`, `judge.host-vacuous`, `judge.host-digest-diff` `ok`; trailer `executed=144 skipped=2 failed=0` | ✅ | ✅ green |
| 15-02-T3 | 02 | 2 | TOOL-08 | T-15-11, T-15-12, T-15-13 | A tracked shared object is refused by name in the tree, in an archive and in the installed package; no untagged claim word enters `docs/` | control + in-step selftests | `sh tools/check/run-controls.sh` → `controls=404/404`; `sh tools/gate.sh` → `pkg-selftest=20/20 release-selftest=12/12 binaries-listed=0 prefix-files=11` | ✅ | ✅ green |
| 15-03-T1 | 03 | 3 | TOOL-08 | T-15-14, T-15-19 | A requirement flip and its registry rows move in one commit; the toy tree's derived set still closes | registry step + control runner | `sh tools/check/steps/important-gates.sh` → `gates=118 lines=358` (toy `gates=47 lines=50`); `sh tools/check/run-controls.sh` → `controls=404/404` | ✅ | ✅ green |
| 15-03-T2 | 03 | 3 | TOOL-08 | T-15-15 | One claim at the frozen tier ceiling, `provisional`, resting on eight green ids, with the release-notes table rendered rather than typed | claim ledger step + generated-table comparison | `sh tools/gate.sh` → `claims=29 provisional=28 claim-refs=27`; `cmake -DROOT=<repo> -P tools/check/claims-table.cmake` equals the CHANGELOG block byte for byte | ✅ | ✅ green |
| 15-03-T3 | 03 | 3 | TOOL-08 | T-15-16, T-15-17, T-15-18 | The closing line is measured under two shells and in the container; the checkpoint is arithmetic over the committed TSV with exactly one verdict | gate (two shells + container) + records | `sh tools/gate.sh` and `dash tools/gate.sh` byte-identical `tail -1`; `sh tools/gate-linux.sh` → `gate: PASS`; `sh tools/hygiene.sh --tree` silent | ✅ | ✅ green |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [x] `host/libretro_shim.h` — the project-authored subset (eight shaped entries + `ivs_retro_digest`), *libretro-shaped*, never compatible — **satisfied by `host/libretro_shim.h`** (plan 15-01)
- [x] `host/ivs_retro_shim.c` — the nine entries, the index→XRGB conversion through `ivs_frameppm_palette`, the refpcm feed — **satisfied by `host/ivs_retro_shim.c`** (plan 15-01)
- [x] `host/CMakeLists.txt` — the `SHARED` target, the generated export list, the only `IVS_API` definition, no install rule — **satisfied by `host/CMakeLists.txt`**; `prefix-files=11` measures the absent install rule
- [x] `tests/host/ivs_host_harness.c` — `dlopen`, drive, assert, print; not linked against the core — **satisfied by `tests/host/ivs_host_harness.c`**; also `CL-0029`'s `artifact`
- [x] `tests/CMakeLists.txt` — the harness target and the suite's seventh `$<TARGET_FILE:…>` argument — **satisfied by `tests/CMakeLists.txt`** (plan 15-01)
- [x] `tools/check/host-symbols.sh`, `data/expected/host-symbols.ids`, `data/expected/host-imports.ids` — the export/import contract and its check — **satisfied by `tools/check/host-symbols.sh`, `data/expected/host-symbols.ids`, `data/expected/host-imports.ids`** (the last gaining the four weak undefined ELF crt stubs in `85470b8`)
- [x] `data/expected/tests/runner-process.ids` — 8 id lines and the header paragraph — **satisfied by `data/expected/tests/runner-process.ids`**; measured trailer `executed=144 skipped=2 failed=0`
- [x] `tools/check/controls/files-unlisted-binary-so/{CHECK,EXPECT,MAKE}` and `-dylib` — the two refusal controls — **satisfied by `tools/check/controls/files-unlisted-binary-so/` and `-dylib/`**; `controls=404/404`
- [x] `data/expected/controls/rights.ids` — the two control lines — **satisfied by `data/expected/controls/rights.ids`**
- [x] `data/expected/pkg-selftests.ids` — `host-so-absent` (19 → 20) — **satisfied by `data/expected/pkg-selftests.ids`**; measured `pkg-selftest=20/20`
- [x] `data/expected/release-assets-selftests.ids` — `so-member` (11 → 12) — **satisfied by `data/expected/release-assets-selftests.ids`**; measured `release-selftest=12/12`
- [x] `tools/check/binary-ext.txt` — `so dylib dll bundle` — **satisfied by `tools/check/binary-ext.txt`**; measured `binaries-listed=0`
- [x] `data/expected/important-gates.ids` — TOOL-08's two `mutant` rows and the `host` gate name — **satisfied by `data/expected/important-gates.ids`**; re-derived `gates=118 lines=358` (toy `gates=47 lines=50`)
- [x] `data/claims.json`, `data/expected/claims.ids`, `CHANGELOG.md`, `docs/evidence-tiers.md` — `CL-0029` and V3's retarget set — **satisfied by those four files**; measured `claims=29 provisional=28 claim-refs=27`, and the rendered table `cmp`s clean against the CHANGELOG block (`tools/check/kinds/claim.cmake` needed no edit: the new row exercises the hook's existing rules, recorded in `15-03-SUMMARY.md`)
- [x] `docs/packaging.md`, `.planning/ops/RELEASE-OPS.md` — the "not installed / not shipped" statements — **satisfied by `docs/packaging.md` and `.planning/ops/RELEASE-OPS.md`** (plan 15-02)
- [x] Measured invariants to hold at every commit: `steps=38/38 faults=238/238 goldens=29 prefix-files=11` — no new framework, no new suite, no new gate step — **held at the close, measured**: `steps=38/38 faults=238/238 goldens=29 controls=404/404 prefix-files=11`

*No framework is installed: the phase's ids are ids inside `runner-process`, and the three new equivalences reuse `self_control`.*

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| — | — | — | *All phase behaviors have automated verification.* The phase's ceiling is one `deterministic-repeatability` claim resting on committed ids, nothing is handed off, and no row carries `evidence: none`. |

---

## Validation Sign-Off

- [x] All tasks have `<automated>` verify or Wave 0 dependencies — eight of eight rows carry an automated command
- [x] Sampling continuity: no 3 consecutive tasks without automated verify — the quick run follows every task commit and the full gate every wave
- [x] Wave 0 covers all MISSING references — every bullet above is ticked and names the file that satisfies it
- [x] No watch-mode flags — none in any task, none in `CMakePresets.json`
- [x] Feedback latency measured and recorded (not a plan-time figure) — 19.0-19.8 s, measured twice at the close; the plan-time 29.5 s figure is named stale
- [x] `nyquist_compliant: true` set in frontmatter

**Approval:** 2026-11-26 — measured at `85470b8`, exit 0 and byte-identical under `sh` and `dash` (`cmp` on `tail -1`, 1509 bytes each), `gate-time total=74`, zero `^-- FAIL` lines, and repeated once inside the Ubuntu 24.04 container (`gate: PASS`, `gate-time total=279`, four documented host tokens differing: `cc=GNU`, `core-imports=5`, `archive=sha256`, `refs-verified=0/4`): `gate: PASS steps=38/38 controls=404/404 … claims=29 provisional=28 … faults=238/238 … tracked=2771 binaries-listed=0 … goldens=29 … mode=tree hygiene-files=2771 … gates=118 lines=358 … prefix-files=11 … public-symbols=19 defined=141 … pkg-selftest=20/20 … release-selftest=12/12 … tests-expected=5036 tests-executed=5032 tests-skipped=4 tests-missing=0 … claim-refs=27`. The full line, unelided, is quoted once in `15-03-SUMMARY.md`.
