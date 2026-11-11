---
phase: 03-memory-map-content-loading-and-refusal
verified: 2026-11-11T13:30:00Z
status: passed
score: 12/12 must-haves verified (5 roadmap SCs + 7 requirements)
covered_files: [".gitignore", ".planning/phases/03-memory-map-content-loading-and-refusal/03-01-PLAN.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-01-SUMMARY.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-02-PLAN.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-02-SUMMARY.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-03-PLAN.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-03-SUMMARY.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-04-PLAN.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-04-SUMMARY.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-05-PLAN.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-05-SUMMARY.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-06-PLAN.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-06-SUMMARY.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-07-PLAN.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-07-SUMMARY.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-08-PLAN.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-08-SUMMARY.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-09-PLAN.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-09-SUMMARY.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-10-PLAN.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-10-SUMMARY.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-11-PLAN.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-11-SUMMARY.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-12-PLAN.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-12-SUMMARY.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-13-PLAN.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-13-SUMMARY.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-14-PLAN.md", ".planning/phases/03-memory-map-content-loading-and-refusal/03-14-SUMMARY.md", "README.md", "cmake/IvsCartTable.cmake", "cmake/IvsMemmapProbes.cmake", "cmake/IvsMemmapTable.cmake", "cmake/IvsShaVectors.cmake", "core/CMakeLists.txt", "core/include/ivs/ivs.h", "core/include/ivs/ivs_machine.h", "core/include/ivs/ivs_status.h", "core/src/common/ivs_sha256.c", "core/src/common/ivs_sha256.h", "core/src/content/ivsc.c", "core/src/content/ivsc.h", "core/src/cpu/cp1610.h", "core/src/machine/machine.c", "core/src/machine/machine.h", "core/src/machine/machine_digest.c", "core/src/machine/machine_load.c", "core/src/machine/machine_run.c", "core/src/memmap/memsys.c", "core/src/memmap/memsys.h", "data/api/api-effects.json", "data/cart/ivsc-load.json", "data/cart/ivsc-parse.json", "data/claims.json", "data/discrepancies.json", "data/expected/api-statuses.ids", "data/expected/cart-fixture-files.ids", "data/expected/claims.ids", "data/expected/controls/api-effects.ids", "data/expected/controls/core-purity.ids", "data/expected/controls/faults-verdict.ids", "data/expected/controls/memmap.ids", "data/expected/core-generated.ids", "data/expected/discrepancies.ids", "data/expected/faults/ivsc.ids", "data/expected/faults/machine.ids", "data/expected/faults/memmap.ids", "data/expected/faults/sha256.ids", "data/expected/fixtures.ids", "data/expected/gate-steps/machine.ids", "data/expected/profile-fields.ids", "data/expected/tests/headers.ids", "data/expected/tests/ivsc.ids", "data/expected/tests/machine.ids", "data/expected/tests/memmap.ids", "data/memmap/intv-2609-ntsc-family-p1.literal.txt", "data/profiles/intv-2609-ntsc-family-p1.json", "data/sha256/vectors.txt", "docs/formats/ivsc-v1.md", "fixtures/FX-ivsc-vectors/manifest.json", "tests/CMakeLists.txt", "tests/faults.cmake", "tests/fuzz/fuzz_ivsc.c", "tests/headers-tap.sh", "tests/ivsc/ivsc_fuzz.c", "tests/ivsc/ivsc_fuzz.h", "tests/ivsc/ivsc_main.c", "tests/ivsc/ivsc_reencode.c", "tests/ivsc/ivsc_table.h", "tests/machine/machine_cases.h", "tests/machine/machine_create.c", "tests/machine/machine_digest.c", "tests/machine/machine_iso.c", "tests/machine/machine_load.c", "tests/machine/machine_main.c", "tests/machine/machine_refuse.c", "tests/machine/machine_reset.c", "tests/machine/machine_sha256.c", "tests/machine/machine_standin.c", "tests/machine/machine_sweep.c", "tests/memmap/memmap_main.c", "tests/memmap/memmap_rows.c", "tests/memmap/memmap_rows.h", "tools/cart/manifest-outputs.sh", "tools/check/api-effects.cmake", "tools/check/binary-ext.txt", "tools/check/core-symbols-allow.txt", "tools/check/core-symbols.sh", "tools/check/memmap.cmake", "tools/check/steps/api-effects.sh", "tools/check/steps/c-faults.sh", "tools/check/steps/c-purity.sh", "tools/check/steps/faults-verdict.sh", "tools/check/steps/memmap.sh", "tools/gate-linux.sh"]
covered_digest: "v4:sha256:52518b0cb2004edc70e6c55c29452bb9d489679a06d27e4773b3fb785aa98a52"
behavior_unverified: 0
overrides_applied: 0
human_verification: []
---

# Phase 3: Memory Map, Content Loading and Refusal — Verification Report

**Phase goal:** A host can create isolated machine instances and load typed system images and cartridge content against the profile's memory map. Every bad input gets a typed refusal that leaves prior machine state unchanged.
**Verified:** 2026-11-11, branch `gsd/phase-3-memory-map-content-loading` at `3d866f5`. The working tree had only `.planning/config.json` and `.planning/state.json` modified; neither is gate evidence.
**Status:** passed
**Re-verification:** No. This is the first verification.

## Method

Every check below was run during this verification. None is taken from SUMMARY.md.

1. **Gate run.** `sh tools/gate.sh` exited 0. It took 53 s of local wall time at a load average of about 13. Final line:
   `gate: PASS steps=26/26 controls=231/231 api-calls=10 api-statuses=14 api-rows=27 ... fault-runs=119 core-imports=4 purity-selftest=7/7 ctest-runs=2 ... faults=119/119 ... binaries-listed=0 ... manifests=2 effective-yes=1 effective-no=1 literal-rows=28 views=2 dump-lines=56 ... tests-expected=2144 tests-executed=2144 tests-skipped=0 tests-missing=0`.
   Step lines that matter for this phase: `memmap pass literal-rows=28 views=2 dump-lines=56`, `api-effects pass api-calls=10 api-statuses=14 api-rows=27`, `c-faults pass fault-runs=119`, `faults-verdict pass faults=119/119`, `manifests pass manifests=2`, `c-purity pass core-imports=4 purity-selftest=7/7`.
2. **Independent probes.** I wrote these myself in the scratch directory and did not commit them. They link `build/dev/core/libivs_core.a` and were built with ASan and UBSan. Their SHA-256 oracle is Apple CommonCrypto, not the core's in-repo SHA-256. Their IVSC containers were built from the text of `docs/formats/ivsc-v1.md`, not from project code.
   - `sc1.c` reads the literal map file at run time. It checks the runtime decode against the literal for all 2 × 65,536 addresses. It also checks read-width behaviour through the CPU bus port in two raw-poke passes (FFFF and 0000), and write-view store truncation and dropping at every address by comparing the memory-system bytes. All counts were 0: 0 structural mismatches, 0 read mismatches, 0 write mismatches, and 0 failures across 19 boundary probes. **Negative control:** a copy of the literal with `exec w=10→16` and a GRAM `mirror=512→256` gave 2048 structural and 4096 read mismatches.
   - `sc235.c` checks three things:
     - **SC2:** EXEC, GROM and cartridge load reports give the length and a SHA-256 equal to CommonCrypto's. Reported values: EXEC 8192 bytes, 4096 units, 10 bits, base 1000. GROM 2048 bytes, 2048 units, 8 bits, base 3000. Cartridge 120 bytes, 24 units, 10 bits, base 5000. Info slots match. `expect_sha256` accepts a matching digest. Unloading everything restores the fresh digest.
     - **SC3:** 61 refusal rows over three priors (fresh, other slots loaded, powered after 50 steps), plus duplicate, missing-system-image ×3, powered unload, input inside the instance, destroyed handle and unknown profile. Each row got its own typed status, with whole-storage `memcmp` and all five digest domains identical before and after.
     - **SC5:** two instances with different EXEC, GROM and cartridge content were stepped interleaved for 3000 units each. Each step left the other instance's storage byte-identical. Final digests equal those of copies stepped one after the other. Each program wrote only its own RAM. A memcpy-relocated copy stayed in lockstep, and destroying B left A unchanged.
   - Result: `probe PASS fails=0`.
3. **Fixture and rights checks.**
   - `shasum -a 256` of `data/cart/ivsc-parse.json` and `ivsc-load.json` matches `fixtures/FX-ivsc-vectors/manifest.json`.
   - The fixture records are P=6, B=11 and N=34, plus L=6 load rows.
   - `git log --all --diff-filter=A` shows no `.rom/.bin/.ivsc/.int/.itv` file ever added.
   - `core/` contains no `fopen` or `getenv`.
4. **GSD tools.** `verify.artifacts` and `verify.key-links` passed for all 14 plans (artifacts 35/35, links 26/26).

## MVP-mode note

ROADMAP.md marks Phase 3 `**Mode:** mvp`, but its goal is not written as a user story. Phases 1 and 2 had the same mismatch. Following their precedent, this report verifies against SC1-SC5 and the 7 requirement IDs, with no User Flow Coverage table. To make the record consistent, either run `/gsd-mvp-phase 3` or drop the `Mode` line.

## Goal Achievement — Roadmap Success Criteria

| # | Success criterion | Status | Evidence |
|---|---|---|---|
| SC1 | Decode-table dump from profile descriptors matches the sourced literal map; decle/8/16-bit width masking and store truncation pass at each region boundary | ✓ VERIFIED | **Gate:** `memmap pass literal-rows=28 views=2 dump-lines=56` (dev and release dumps compared with `data/memmap/intv-2609-ntsc-family-p1.literal.txt`). The gate also has 8+ memmap controls (literal hole/overlap/tier5/no-cite/stale/diff) and 23 memmap faults. Cases `memmap.row.*` (28), `memmap.anchor.*` (28) and `memmap.width.*` (6). **Probe `sc1.c`:** 0 mismatches over 131,072 address/view pairs. Reads: ROM10 gives 03FF, GROM8/RAM8 give 00FF, RAM16 gives FFFF, and stub/undriven give FFFF. Writes keep 8 or 16 bits or are dropped. The negative control fails as expected. The literal states its independence rule, and its rows cite sources; tier 5 is refused by `tools/check/memmap.cmake`. |
| SC2 | EXEC, GROM and cartridge segments load as separate typed inputs and report length and digest; tests use only project-authored stand-ins; no real system image is committed, cached or required | ✓ VERIFIED | **API:** `ivs_machine_load(m, kind, bytes, len, opts, &report)` with `IVS_KIND_{EXEC,GROM,CARTRIDGE}` and `ivs_image_report{length_bytes,length_units,unit_bits,base_addr,sha256}` (`core/include/ivs/ivs_machine.h`). **Probe:** the digests equal the CommonCrypto SHA-256 of the supplied bytes. **Tests:** `machine.load.*` (16) and `machine.sha256.*`; the stand-ins are generated (`tests/machine/machine_standin.c`). **Rights:** the history has no binary image, the gate reports `binaries-listed=0`, the core has no file access, and the profile field `system_images.exec_grom` says "user-supplied only". |
| SC3 | Every refusal-matrix row (malformed, oversized, overlapping, out-of-range, missing system image, unknown profile) returns its own typed error, and the state digest is the same before and after | ✓ VERIFIED | **Matrix:** `tests/machine/machine_refuse.c` has 274 `machine.refuse.*` and 25 `machine.prec.*` ids. Each row is checked against the exact status, a whole-allocation `memcmp`, the composite digest and a poisoned report, over priors f/x/s/c/w. **Probe:** 61+ rows; each status was distinct and as expected (MALFORMED, OVERSIZED, OVERLAP, OUT_OF_RANGE, MISSING_SYSTEM_IMAGE, UNKNOWN_PROFILE, DIGEST_MISMATCH, UNSUPPORTED, DUPLICATE_LOAD, BAD_STATE, BAD_ARG, BAD_HANDLE). Storage and all 5 digest domains were unchanged. An unknown-profile create left the storage byte-identical and set `*out` to NULL. Missing system image is owned by `reset` (D-14). |
| SC4 | Cartridge import format specified in the project's own words; positive, boundary and malformed fixtures pass and are recorded in the fixture manifest | ✓ VERIFIED | **Spec:** `docs/formats/ivsc-v1.md` gives the layout, limits, check order, reason table, worked example and evolution rule. It names no third-party format apart from a disclaimer. **Fixtures:** `data/cart/ivsc-parse.json` (P6/B8/N34) and `ivsc-load.json` (B3/L6). The manifest `FX-ivsc-vectors` lists both with SHA-256 values that I recomputed and that match; the gate reports `manifests pass`. **Tests:** suite `ivsc` (85 ids, including the `ivsc.fuzz.*` seeds and `reasons-covered`) and 16 ivsc faults. **Claim:** CL-0007 is `source-expectation provisional` and is worded as conformance to the project specification. |
| SC5 | Two instances with different content step without affecting each other (isolation test); every API error and refusal has a documented state effect that a test verifies | ✓ VERIFIED | **Isolation tests:** `machine.iso.{interleaved-equals-sequential,poisoned-storage-identical,memcpy-relocation,destroy-mid-run,cross-load}`. `c-purity` forbids writable data (`core-symbols.sh --forbid-writable-data` on `-fno-pie` archives). The probe repeated the interleaved run independently. **Effects:** `data/api/api-effects.json` has 10 calls and 27 refusal rows, all with effect `none` and each with cited test ids. The `api-effects` gate step cross-checks them against the headers, the status pin and the test inventories. Spot checks: `machine.refuse.reset.no-grom` (`machine_refuse.c:1173`) and `machine.refuse.handle.*` (`machine_reset.c:409-411`) assert the status and the storage/digest oracle. |

**Score:** 5/5 SCs verified (0 present-but-behaviour-unverified). The behaviour-dependent truths, refusal effect `none` and isolation, are covered by passing named tests and by this verifier's own probes.

## Requirements Coverage

| Requirement | Plans | Status | Evidence |
|---|---|---|---|
| MEM-01 descriptor-generated map; dump equals sourced literal | 03-04, 03-05, 03-06 | ✓ SATISFIED | `cmake/IvsMemmapTable.cmake` is fail-closed and generates `ivs_profiles.inc`. The memmap gate step passes. Probe sc1 found 0 mismatches. CL-0006 is source-expectation and provisional. |
| MEM-02 width masking and store truncation tested at each boundary | 03-06 | ✓ SATISFIED | `cmake/IvsMemmapProbes.cmake` generates the probes from the literal. Faults `MEM_ROM_NO_MASK`, `MEM_RAM8_NO_TRUNC`, `MEM_RAM16_TRUNC`, `MEM_ROM_WRITABLE`, `MEM_GATE_IGNORED` and `MEM_DROPPED_WRITE_KEPT`. The probe's full-address read and write sweep passes. |
| MEM-03 typed EXEC/GROM/cartridge loads report length and digest (D5 policy) | 03-02, 03-09, 03-10 | ✓ SATISFIED | See SC2. D5 is decided as D-24 (user-supplied only); PROJECT.md row D5 is updated. |
| MEM-04 malformed, oversized, overlapping, out-of-range, missing-system-image and unknown-profile refusals are typed and leave state unchanged | 03-11, 03-12 | ✓ SATISFIED | See SC3. |
| MEM-05 one documented cartridge format with positive, boundary and malformed fixtures | 03-07, 03-08 | ✓ SATISFIED | See SC4. |
| API-01 create/reset/unload/destroy independent instances without shared mutable state | 03-03, 03-09, 03-12 | ✓ SATISFIED | 10-function public surface (`api-calls=10`). Caller-owned storage. `destroy` zero-scrubs, and later calls are BAD_HANDLE (probe). `machine.iso.*` tests and the probe pass, and the writable-data purity rule passes. |
| API-06 every API error/refusal has a documented state effect covered by a test | 03-13 | ✓ SATISFIED | `api-effects` step `api-rows=27 api-statuses=14`. Each row cites test ids that exist in the inventories and ran (2144/2144 executed). Spot-checked rows assert the status. Accepted caveat OQ-5: the step proves the ids exist, not what they assert. |

No orphaned requirements. REQUIREMENTS.md maps exactly these 7 IDs to Phase 3.

## Required Artifacts (spot list)

| Artifact | Status | Details |
|---|---|---|
| `core/include/ivs/{ivs.h,ivs_status.h,ivs_machine.h}` | ✓ VERIFIED | Substantive. The internal seams (`step_unit_internal`, `poke_raw`, `ivs_memsys`) are absent from `core/include` (checked with grep). |
| `core/src/machine/{machine,machine_load,machine_run,machine_digest}.c` | ✓ VERIFIED | Exercised by the probes through the public API. |
| `core/src/memmap/memsys.c` + generated `ivs_profiles.inc` | ✓ VERIFIED | Exercised by probe sc1 over the whole address space. |
| `core/src/content/ivsc.c` | ✓ VERIFIED | Exercised by probe containers built from the spec, including overlap, wrap, span, trailer, feature and wide-word cases. |
| `core/src/common/ivs_sha256.c` | ✓ VERIFIED | Agrees with CommonCrypto on the 8192-, 2048- and 120-byte inputs. |
| `data/memmap/intv-2609-ntsc-family-p1.literal.txt`, `data/api/api-effects.json`, `docs/formats/ivsc-v1.md`, `fixtures/FX-ivsc-vectors/manifest.json` | ✓ VERIFIED | Gate steps memmap, api-effects and manifests pass. |

## Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|---|---|---|---|
| Full gate | `sh tools/gate.sh` | `gate: PASS steps=26/26 controls=231/231 faults=119/119 tests 2144/2144 skipped=0` | ✓ PASS |
| SC1 decode/width sweep | `scratchpad/probes/sc1 data/memmap/...literal.txt` | 0/0/0 mismatches; `sc1 PASS` | ✓ PASS |
| SC1 negative control | same probe on a mutated literal | 2048 + 4096 mismatches; `sc1 FAIL` | ✓ PASS (fails as it should) |
| SC2/SC3/SC5 | `scratchpad/probes/sc235` | `probe PASS fails=0` | ✓ PASS |

## Claim-wording check

- CL-0006 (memory map) and CL-0007 (IVSC) are `source-expectation`, `provisional`. Their notes say "transcription consistency ... not hardware truth" and "conformance to the project specification, not to any hardware or third-party format".
- The literal map header says a gate pass "shows that the runtime decode is consistent with this transcription, not that the console behaves this way".
- In the Phase 3 diff (outside `.planning`), "independent" is used only for the `cmake -E sha256sum` / CMake `string(SHA256)` digest oracle, which is a second SHA-256 implementation. It is never used for an emulator comparison.
- I found no "hardware-validated", "cycle-accurate" or "matches hardware" wording.
- The discrepancy resolvers point to D10 hardware measurement.

## Anti-Patterns Found

| File | Pattern | Severity | Impact |
|---|---|---|---|
| (changed files) | `XXX` matches only in `$XXXX` hex notation (`cmake/IvsMemmapTable.cmake`) and `mktemp ...XXXXXX` templates | ℹ️ Info | Not debt markers |
| core/tests | no TODO/HACK/placeholder | — | — |

## Record drift (warning, not a goal gap)

Stale "D5 open" text remains even though D5 is decided (D-24, PROJECT.md row D5, ROADMAP.md line 64 "Delegated decision 2026-11-11"):
- `.planning/ROADMAP.md:156` (Phase 3 decision gates): "D5 (owner, open) sets the system-image supply policy ... 'User-supplied only, plus a CI boot stub' is recommended, not decided." The same line also still says the cartridge format "`.rom` is recommended ... confirmed at discussion", but IVSC v1 was decided (D-29).
- `.planning/ROADMAP.md:187` (Phase 4 decision gates): "D5 (owner, open) must be resolved before the boot path is built".
- `.planning/REQUIREMENTS.md:167` (decision table): "D5 | ... | Owner — open".

These lines contradict PROJECT.md and ROADMAP.md:64 but do not affect the code or the gate. Fix them in the phase close-out record update.

## Known and accepted caveats (listed, not failed)

1. Six info items deferred in the 03-REVIEW.md Disposition: IN-01 `has_cart_span` dead branches, IN-02 units/unit_bits enforcement, IN-05 fault-site forms, IN-07 `profile_index` stability, IN-09 memmap row ids without a profile part, IN-10 strict aliasing on char-array storage.
2. The `api-effects` step proves that the cited test ids exist, not that each test asserts the status (OQ-5). My spot checks found real status assertions.
3. Isolation is shown behaviourally only; there is no threaded test (D-36).
4. The system-image byte order (high byte first) is [BG] and stays unverified until D11.
5. Cartridge span and window addresses are tier 4 (C62).
6. The libFuzzer target `tests/fuzz/fuzz_ivsc.c` was not run with ASan on the dev host (Apple clang lacks the runtime).
7. There are no `memmap-gen-*` generator controls and no digest-fields golden.
8. The `len-wrap` refusal rows exist only where `size_t` is 64-bit (`RF_SKIP32`).
9. Planning prose quotes stale totals: controls 224 or 231, faults 115 or 119, tests 2094 or 2144. This gate run measured 231, 119 and 2144. For example, STATE.md and 03-SECURITY.md say 224/115/2094.

## Further observations (info)

- 03-SECURITY.md was audited at `684e729`, before the 11 review-fix commits (`2339673`..`f8e7594`). Those commits harden the code: the WR-02 struct_size cap, the WR-03 profile-index handle check, the WR-01 out-overlap ordering and the WR-04 purity bypasses. The audit does not cover them. The gate and this verifier's probes pass at HEAD.
- Local gate wall time was 53 s at load average ~13. The D-38/A-07 budget is hosted: 40 s, with a revisit trigger at 50 s. CI-EFFICIENCY.md says the orchestrator measures hosted time on the phase PR, so this local figure is not a budget breach.

## Gaps Summary

None. All 5 success criteria and all 7 requirements are backed by code, by gate steps with controls and named faults, and by this verifier's own probes, including a failing negative control. The phase goal is achieved: isolated instances, typed EXEC/GROM/IVSC loads against a profile-generated memory map, and typed refusals with effect `none`. The only follow-ups are record drift (the stale D5 and `.rom` text) and the accepted caveats above.

---

_Verified: 2026-11-11_
_Verifier: Claude (gsd-verifier)_
