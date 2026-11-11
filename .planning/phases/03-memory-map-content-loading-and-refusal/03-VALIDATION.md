---
phase: "3"
slug: "memory-map-content-loading-and-refusal"
status: draft
nyquist_compliant: true
wave_0_complete: false
created: "2026-11-10"
---

# Phase 3 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution. Full requirement-to-check map: `03-RESEARCH.md` Section D (Validation Architecture); decisions: `03-CONTEXT.md` D-01..D-38 and amendments A-01..A-07.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | In-repo C harness `tests/harness/ivs_test.{h,c}` (TAP) driven by CTest, plus one shell TAP suite (`tests/headers-tap.sh`); the gate parses TAP files, not ctest output |
| **Config file** | `CMakePresets.json` (`dev`: Debug, ASan+UBSan, fault controls; `release`: -O2), `tests/CMakeLists.txt` (suite and fault-suite registry), `tests/faults.cmake` (generalized in plan 03-01) |
| **Quick run command** | `cmake --build --preset dev && ctest --preset dev -R '^(machine|memmap|ivsc|headers)$'`; one family: `build/dev/tests/ivs_tests_machine --only machine.refuse.` |
| **Full suite command** | `sh tools/gate.sh` (the single local command == the CI command); also `dash tools/gate.sh` at each plan end |
| **Estimated runtime** | local planning baseline `gate-time total=29` s; hosted budget re-baselined to 40 s with revisit trigger 50 s (D-38, A-07), measured at plans 03-02, 03-11 and 03-14 |

---

## Sampling Rate

- **After every task commit:** dev-preset quick run of the touched family (`--only <prefix>`) plus the touched step script or control runner
- **After every plan (each plan is its own wave):** `sh tools/gate.sh` and `dash tools/gate.sh`, both ending `gate: PASS` with exact inventories
- **Before `/gsd-verify-work`:** gate green under sh and dash with the final counts of plan 03-14 (`steps=26/26`, `controls=221/221`, `faults=112/112`, `tests-missing=0`)
- **Max feedback latency:** 40 seconds (hosted budget, D-38)

---

## Per-Task Verification Map

"gate" = `sh tools/gate.sh` (fails on non-zero exit or a last line not starting `gate: PASS`); "dash" = `dash tools/gate.sh`; "quick" = `cmake --build --preset dev && ctest --preset dev -R '^<suite>$'` or `--only <prefix>`; "controls" = `out=$(sh tools/check/run-controls.sh) && printf '%s\n' "$out" | tail -1` (runner status observed through `&&`). Execution is sequential (one plan per wave), so each plan's expected counts are deterministic. Every task has at least one automated command; no three consecutive tasks lack one.

| Task | Plan | Wave | Requirement | Threat Ref | Test Type | Automated Command | Status |
|------|------|------|-------------|------------|-----------|-------------------|--------|
| 03-01-T1 | 01 | 1 | API-06, MEM-04 (fault infra, D-37) | T-03-01, T-03-03 | gate + scratch two-file probe | gate (`faults=67/67`); grep `faults/*.ids` in three files | ⬜ pending |
| 03-01-T2 | 01 | 1 | API-06 | T-03-01, T-03-02 | controls (4) | controls (196/196); gate + dash | ⬜ pending |
| 03-02-T1 | 02 | 2 | MEM-03 | T-03-04 | unit + fault (tracer) | quick `machine.sha256.fips-abc`; gate (`faults=68/68`) | ⬜ pending |
| 03-02-T2 | 02 | 2 | MEM-03, API-06 | T-03-04, T-03-05, T-03-06 | unit (FIPS, boundaries, split, snapshot) + 5 faults + generator oracle | quick both presets; gate (`faults=73/73`) | ⬜ pending |
| 03-02-T3 | 02 | 2 | (D-38, A-07) | — | measurement | gate + dash; CI-EFFICIENCY grep `A-07` | ⬜ pending |
| 03-03-T1 | 03 | 3 | API-01 (D-06) | T-03-08 | build + purity (tracer) | quick both presets; gate | ⬜ pending |
| 03-03-T2 | 03 | 3 | API-06 (A-06) | T-03-07 | controls (4) | controls (200/200); gate | ⬜ pending |
| 03-03-T3 | 03 | 3 | API-01 (D-07) | T-03-09, T-03-10 | purity rule + controls (2) | controls (202/202); gate + dash (`purity-selftest=5/5`) | ⬜ pending |
| 03-04-T1 | 04 | 4 | MEM-01 | T-03-11, T-03-14 | generator + decode (tracer) | quick `memmap.anchor.exec-lo`; gate | ⬜ pending |
| 03-04-T2 | 04 | 4 | MEM-01, MEM-02 | T-03-12, T-03-13 | unit (regions, width, gates, bus) | quick both presets | ⬜ pending |
| 03-04-T3 | 04 | 4 | MEM-01, MEM-02 | T-03-11, T-03-12 | 6 faults incl. core-consumed GEN | gate + dash (`faults=79/79`) | ⬜ pending |
| 03-05-T1 | 05 | 5 | MEM-01 | T-03-15, T-03-16, T-03-17 | static step (tracer) | gate (`steps=25/25`, `-- memmap pass literal-rows=`) | ⬜ pending |
| 03-05-T2 | 05 | 5 | MEM-01 | T-03-15, T-03-16 | controls (8) | controls (210/210); gate + dash | ⬜ pending |
| 03-06-T1 | 06 | 6 | MEM-01 | T-03-18 | dump producer + pure consumer (tracer) | quick `memmap.dump.`; gate (`dump-lines=`) | ⬜ pending |
| 03-06-T2 | 06 | 6 | MEM-02 | T-03-19 | generated boundary probes + 5 faults | quick `memmap.row` both presets; gate (`faults=84/84`) | ⬜ pending |
| 03-06-T3 | 06 | 6 | MEM-01 | T-03-18, T-03-20 | controls (3) + claim CL-0006 | controls (213/213); gate + dash (`claims=6`) | ⬜ pending |
| 03-07-T1 | 07 | 7 | MEM-05 | T-03-21, T-03-22 | parser (tracer, P01) | quick `ivsc.P01-min`; gate | ⬜ pending |
| 03-07-T2 | 07 | 7 | MEM-05, MEM-04 | T-03-21, T-03-22, T-03-23 | spec + P/B/N/X fixtures | quick both presets; gate (`tests-missing=0`) | ⬜ pending |
| 03-07-T3 | 07 | 7 | MEM-05 | T-03-24 | manifest + helper | helper `--check` (sh, dash); gate + dash (`manifests=2`) | ⬜ pending |
| 03-08-T1 | 08 | 8 | MEM-05, MEM-04 | T-03-25, T-03-26 | seeded fuzz (tracer) | quick `ivsc.fuzz.` both presets; gate | ⬜ pending |
| 03-08-T2 | 08 | 8 | MEM-05 | T-03-27 | 6 faults | gate (`faults=90/90`) | ⬜ pending |
| 03-08-T3 | 08 | 8 | MEM-05 | T-03-28 | hygiene + optional libFuzzer + CL-0007 | gate + dash (`claims=7`); grep option and extension | ⬜ pending |
| 03-09-T1 | 09 | 9 | API-01 | T-03-29 | lifecycle via public header (tracer) | quick `machine.create.`; gate | ⬜ pending |
| 03-09-T2 | 09 | 9 | API-01, MEM-04, API-06 | T-03-29, T-03-30, T-03-31 | refusal rows with storage memcmp + status pin | quick both presets | ⬜ pending |
| 03-09-T3 | 09 | 9 | API-06 | T-03-32 | state digest + 8 faults | quick `machine.digest.`; gate + dash (`faults=98/98`) | ⬜ pending |
| 03-10-T1 | 10 | 10 | MEM-03 | T-03-33, T-03-35, T-03-36 | EXEC load end to end (tracer) | quick `machine.load.exec`; gate | ⬜ pending |
| 03-10-T2 | 10 | 10 | MEM-03, MEM-05 | T-03-34, T-03-35 | GROM, cartridge, layer 2, A-04, options, order-free | quick both presets (`machine`, `ivsc`) | ⬜ pending |
| 03-10-T3 | 10 | 10 | MEM-03 | T-03-35 | 3 faults | gate + dash (`faults=101/101`) | ⬜ pending |
| 03-11-T1 | 11 | 11 | MEM-04 | T-03-37 | full-oracle refusal row x 3 priors (tracer) | quick `machine.refuse.load.exec.oversized-by-one`; gate | ⬜ pending |
| 03-11-T2 | 11 | 11 | MEM-04, MEM-05, API-06 | T-03-37, T-03-38, T-03-39, T-03-40 | refusal matrix, prec rows, L/B/X fixtures, manifest refresh | quick `machine.refuse.`, `machine.prec.`, ivsc; helper `--check`; release ctest | ⬜ pending |
| 03-11-T3 | 11 | 11 | MEM-04 | T-03-37, T-03-38, T-03-39 | 7 ordering faults + measurement 2 | gate + dash (`faults=108/108`) | ⬜ pending |
| 03-12-T1 | 12 | 12 | API-01 | T-03-42 | reset + step seam program (tracer) | quick `machine.reset.`; gate | ⬜ pending |
| 03-12-T2 | 12 | 12 | API-01, MEM-04 | T-03-41, T-03-42, T-03-43 | reset refusals, powered prior, isolation | quick `machine.iso.`, `machine.refuse.reset.` both presets | ⬜ pending |
| 03-12-T3 | 12 | 12 | API-01, MEM-04, API-06 | T-03-44, T-03-45 | sweeps, headers suite, 4 faults | quick (`machine`, `headers`), `dash tests/headers-tap.sh`; gate + dash (`faults=112/112`) | ⬜ pending |
| 03-13-T1 | 13 | 13 | API-06 | T-03-46, T-03-47, T-03-48 | static effect-table step (tracer) | step run (`api-calls=10`); gate (`steps=26/26`) | ⬜ pending |
| 03-13-T2 | 13 | 13 | API-06 | T-03-46, T-03-47, T-03-48 | controls (8) | controls (221/221); gate + dash | ⬜ pending |
| 03-13-T3 | 13 | 13 | API-06 | T-03-47 | return-site audit | gate + dash (`tests-missing=0`) | ⬜ pending |
| 03-14-T1 | 14 | 14 | all 7 | T-03-49 | phase evidence (tracer) | gate (all final stats) | ⬜ pending |
| 03-14-T2 | 14 | 14 | MEM-01 (records, D-23, D-24) | T-03-49 | record reconciliation | gate + dash; grep resolving-evidence text | ⬜ pending |
| 03-14-T3 | 14 | 14 | (D-38, A-07) | T-03-50 | budget re-baseline + caveats | gate + dash; grep `hosted gate budget 30 s -> 40 s` | ⬜ pending |

Requirement coverage target:

| Requirement | Automated check | Where | Status |
|-------------|-----------------|-------|--------|
| MEM-01 | decode dump of all 65,536 addresses x 2 views equals the sourced literal (step `memmap`, nonce-bound, both presets); literal grammar/tiling/provenance rules with 8 controls; generator fail-closed; `GEN_MAP_MUTATION` and decode faults; CL-0006 provisional | suite `memmap`, steps `memmap`, `c-faults`/`faults-verdict` | ⬜ pending |
| MEM-02 | generated boundary probes per literal row (lo-1, lo, hi, hi+1, mirror wrap), width formula direct tests, gates and stubs; faults `MEM_ROM_NO_MASK`, `MEM_RAM8_NO_TRUNC`, `MEM_RAM16_TRUNC`, `MEM_ROM_WRITABLE`, `MEM_GATE_IGNORED` | suite `memmap` (`memmap.row.*`, `memmap.width.*`, `memmap.gate.*`) | ⬜ pending |
| MEM-03 | `machine.load.*`: EXEC, GROM, cartridge as separate typed loads; report length, units, base and whole-file SHA-256 equal to literals from `cmake -E sha256sum`; order-free; SHA-256 checked against FIPS and `string(SHA256)` | suite `machine` (families `sha256`, `load`) | ⬜ pending |
| MEM-04 | `machine.refuse.*` across priors f/s/c/w with status, whole-storage `memcmp`, digest equality and report detail; `machine.prec.*`; reset rows for missing EXEC/GROM; create row for unknown profile; ordering faults; sweeps | suite `machine`, suite `ivsc` | ⬜ pending |
| MEM-05 | `docs/formats/ivsc-v1.md`; IVSC P/B/N/L/X fixtures in both presets; seeded fuzz; 6 IVSC faults; manifest `FX-ivsc-vectors` checked by `manifests`; CL-0007 | suite `ivsc`, steps `manifests`, `files` | ⬜ pending |
| API-01 | create/reset/unload/destroy through public headers; `machine.iso.*` (interleaved vs sequential, poisoned storage, memcpy relocation, destroy mid-run, cross-load); c-purity writable-data rule with control; `STATIC_CARRY` fault; headers suite | suite `machine`, step `c-purity`, suite `headers` | ⬜ pending |
| API-06 | `data/api/api-effects.json` checked by step `api-effects` against the status pin, the `IVS_API` declarations and the test inventories (8 controls); every refusal row asserts effect `none` by `memcmp` and digest | step `api-effects`, suite `machine` | ⬜ pending |

Success criteria to checks (zero human UAT):

| SC | Automated check |
|----|-----------------|
| SC1 | step `memmap` (`dump-lines=`, controls `memmap-dump-*`, `memmap-literal-*`) + `memmap.row.*` probes; eleven memmap faults |
| SC2 | `machine.load.*` with independent literal digests; stand-in bytes generated in tests; `files`/`hygiene` steps forbid committed images; `.ivsc` in ignore and binary lists |
| SC3 | `machine.refuse.*`, `machine.prec.*`, `machine.refuse.reset.*`, `machine.refuse.create.unknown-profile`; digest equality and storage `memcmp` in every row |
| SC4 | `docs/formats/ivsc-v1.md` (text and glossary steps), `ivsc.*` ids, `manifests` step on `FX-ivsc-vectors` |
| SC5 | `machine.iso.*`; step `api-effects` with its controls |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] Plan 03-01 generalizes the fault loop (`tests/faults.cmake`, `c-faults`, `faults-verdict`) before any second faults file exists
- [ ] Plan 03-03 opens `core/include` and generated includes in `c-purity` before the first generated core include (plan 03-04) and the public machine headers (plan 03-09)
- [ ] Each new suite (`machine` 03-02, `memmap` 03-04, `ivsc` 03-07, `headers` 03-12) lands with its ids file, ctest registration and TAP in both presets in the same plan (H9)
- [ ] Each new step (`memmap` 03-05, `api-effects` 03-13) lands with its gate-step inventory line, controls and good-tree toy input in the same plan
- [ ] Framework install: none (cmake, ninja, cc, nm, ar; optional c++ and Homebrew LLVM only for visible-skip or non-gate paths)

---

## Manual-Only Verifications

All phase behaviors have automated verification. Nothing is handed to the owner as UAT. Missing evidence that no test can supply is recorded, not handed off: D10 hardware measurement (C20, C21, C60, C61, C62) and D11 (a real owner-held EXEC/GROM to confirm the big-endian system-image byte order of D-26) are hard stops in `.planning/AUTONOMY.md` and stay outside the milestone's automated exit. The hosted gate time is recorded by the orchestrator from CI (agents never call `gh`).

---

## Validation Sign-Off

- [x] All tasks have `<automated>` verify or Wave 0 dependencies
- [x] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references
- [x] No watch-mode flags
- [ ] Feedback latency < 40s (measured at plans 03-02, 03-11, 03-14)
- [x] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
