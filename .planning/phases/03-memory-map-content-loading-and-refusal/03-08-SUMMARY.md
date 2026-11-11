---
phase: 03-memory-map-content-loading-and-refusal
plan: 08
subsystem: content
tags: [ivsc, cartridge, parser, fuzzing, named-faults, libfuzzer, claim, D-29, D-33, D-34, D-37, MEM-05, MEM-04]

requires:
  - phase: 03-memory-map-content-loading-and-refusal
    provides: "plan 03-07: ivs_ivsc_parse, the generated fixture table ivs_cart_table.inc, suite ivsc, the registered fault suite ivsc, FX-ivsc-vectors and tools/cart/manifest-outputs.sh"
provides:
  - "Seeded fuzz family ivsc.fuzz.seed-01..08 and ivsc.fuzz.reasons-covered (tests/ivsc/ivsc_fuzz.c, interface tests/ivsc/ivsc_fuzz.h)"
  - "Six IVSC named faults in data/expected/faults/ivsc.ids (suite ivsc), one IVS_FAULT_IVSC_<ID> site each in core/src/content/ivsc.c"
  - "Fixture ivsc.N34-table-short: a table smaller than total_bytes (the direction of the implied-size check no fixed fixture pinned)"
  - "Shared canonical re-encoder tests/ivsc/ivsc_reencode.c (X02, the fuzz family, the optional target)"
  - "Optional non-gate libFuzzer target tests/fuzz/fuzz_ivsc.c behind IVS_ENABLE_LIBFUZZER (default OFF) with IVS_LIBFUZZER_SANITIZERS"
  - "Fuzzing section of docs/formats/ivsc-v1.md and claim CL-0007"
affects: [03-09, 03-10, 03-11, 03-13, 03-14]

actuals:
  tokens: 7944
  tasks: 3
  commits: 3
plan_head_before: 8b24c0b16b672f9e10c117375396e13b987e7f42
plan_head_after: bc5d80318023d51b32e88dd17a952a4603fd3ba9

tech-stack:
  added: []
  patterns:
    - "Seeded mutation fuzzing of a pure parser in the gate: fixed seeds, xorshift32, one mutation per iteration, reseal on alternate iterations, invariants plus a reason-coverage non-vacuity case"
    - "Fault sites that stay memory-safe in the plain (non-sanitized) fault build: a fault that would over-read or overflow is shaped so the faulty build still never leaves its buffers"

key-files:
  created:
    - tests/ivsc/ivsc_fuzz.c
    - tests/ivsc/ivsc_fuzz.h
    - tests/ivsc/ivsc_reencode.c
    - tests/fuzz/fuzz_ivsc.c
    - data/expected/faults/ivsc.ids
  modified:
    - core/src/content/ivsc.c
    - tests/ivsc/ivsc_main.c
    - tests/CMakeLists.txt
    - data/expected/tests/ivsc.ids
    - data/cart/ivsc-parse.json
    - fixtures/FX-ivsc-vectors/manifest.json
    - docs/formats/ivsc-v1.md
    - data/claims.json
    - data/expected/claims.ids

key-decisions:
  - "Fuzz bases are the accepted positive records with literal hex (P01-P04, P06, B01); the 98 KB to 131 KB build records are left out to keep the family fast"
  - "IVSC_TRUST_TOTAL_BYTES drops only the table-smaller-than-total_bytes half of the implied-size check; the other half guards against a word over-read, and removing it would make the plain fault build read past its buffer. No fixed fixture pinned that half, so ivsc.N34-table-short was added as its must-fail"
  - "IVSC_COUNT_17 lets a count of 17 pass the count rule and reads the table as 16 entries, so the plain fault build never writes a seventeenth view entry"
  - "Each fault has a must-pass neighbour that the same rule still decides (for example N20 overlap for ADJACENT_AS_OVERLAP, B06 wrap for END_OFF_BY_ONE, N11 for TRUST_TOTAL_BYTES)"
  - "CL-0007 tier source-expectation (the project's own specification plus independently computed digests); provisional because cartridge.slots is unknown-default-chosen"
  - "The libFuzzer sanitizer list is a cache string (default fuzzer,address,undefined) because the Homebrew LLVM ASan runtime hangs at start-up on this macOS host"
  - "MEM-05 and MEM-04 stay open: the layer-2 load fixtures and refusal matrix (plans 03-09 to 03-11) are still to come"

patterns-established:
  - "A fault whose removal of a check would be memory-unsafe in the plain fault build is narrowed to the memory-safe half, and the half's own fixture is added if none exists"

requirements-completed: []

coverage:
  - id: D1
    description: "Seeded IVSC fuzz family with invariants and reason coverage (D-34, T-03-25, T-03-26)"
    requirement: MEM-05
    verification:
      - kind: unit
        ref: "build/dev/tests/ivs_tests_ivsc --only ivsc.fuzz. (9 cases, ASan+UBSan, 0.07 s); ctest --preset release -R '^ivsc$'"
        status: pass
    human_judgment: false
  - id: D2
    description: "Six IVSC named faults each turn a named fixture red (D-37, D-33, T-03-27)"
    requirement: MEM-05
    verification:
      - kind: integration
        ref: "sh and dash tools/gate.sh: faults=90/90 fault-runs=90"
        status: pass
    human_judgment: false
  - id: D3
    description: "Optional non-gate libFuzzer target and recipe (D-34)"
    requirement: MEM-05
    verification:
      - kind: other
        ref: "scratch build with Homebrew LLVM 21, IVS_LIBFUZZER_SANITIZERS=fuzzer,undefined, 30 s run: 1,036,838 inputs, no crash"
        status: pass
    human_judgment: false
  - id: D4
    description: "Claim CL-0007 and .ivsc name-level hygiene (D-29, D-34, T-03-28)"
    requirement: MEM-04
    verification:
      - kind: integration
        ref: "gate claims=7 provisional=6 claim-refs=6; grep -c -F '*.ivsc' .gitignore -> 1; grep -c -x ivsc tools/check/binary-ext.txt -> 1"
        status: pass
    human_judgment: false

duration: 14min
completed: 2026-11-11
status: complete
---

# Phase 3 Plan 08: IVSC seeded fuzzing, named faults, optional libFuzzer and CL-0007 Summary

**The IVSC v1 parser is now tested beyond its fixed fixtures. A seeded fuzz family in suite `ivsc` makes 12,000 mutants from 8 fixed seeds. It checks five invariants on every mutant and reaches all 18 refusal reasons. Six named faults each turn a named fixture red. One of them needed a new fixture, `ivsc.N34-table-short`, because no fixed fixture pinned the table-smaller-than-`total_bytes` half of the size check. An optional libFuzzer target sits outside the gate, and claim CL-0007 records the evidence. Final gate under sh and dash: `gate: PASS steps=25/25 controls=215/215 ... fault-runs=90 ... claims=7 provisional=6 ... faults=90/90 ... tests-expected=1388 tests-executed=1388 tests-skipped=0 tests-missing=0`.**

## Performance

- **Duration:** about 14 min (2026-11-11T06:48:58Z to 07:02:37Z).
- **Tasks:** 3 of 3.
- **Files:** 14 in 3 commits (diff +752 / -69).
- **Fuzz family wall time (dev preset, ASan+UBSan):** `build/dev/tests/ivs_tests_ivsc --only ivsc.fuzz.` took 0.07 s real in three runs, against a target of 0.3 s. That covers 8 seeds of 1,500 iterations, plus `reasons-covered`, which replays all 8 seeds.
- **Fault executables:** each of the six ran its whole suite, fuzz family included, in at most 0.01 s.
- **Gate time (local, warm build):** `gate-time total` was 20 to 23 s (sh 20 and 22, dash 20 and 23). Before the plan it was 20 to 22 s.
  - New build cost: two test TUs (`ivsc_fuzz.c`, `ivsc_reencode.c`) in each preset, plus six owner copies of `ivsc.c` for the fault executables.
  - The fault suite's test sources compile once (W7), and no gate step or control was added.
  - The hosted figure should stay near the last hosted 30 s (budget 40 s, revisit trigger 50 s).

## Accomplishments

### Seeded fuzz family (`tests/ivsc/ivsc_fuzz.c`)

- **Seeds and bases.** Eight fixed 32-bit seeds drive an in-repo xorshift32 generator. Nothing reads host randomness or time. The bases are the six accepted positives with literal hex: P01 to P04, P06 and B01.
- **Mutations.** Each iteration applies exactly one of these:
  - a bit flip;
  - a truncation to any shorter length, 0 included;
  - a growth of 1 to 8 random bytes at a random offset;
  - a field edit of a prefix or entry field. The value is an edge value (0, 1, all ones, the original ±1, one bit flipped, random) or a field-specific one: the length ±1 or the maximum + 1 for `total_bytes`; 17 or any count for `segment_count`; another entry's start or the wrap edge for `addr`.
- **Reseal and parse.** Every second mutant is resealed with the core SHA-256. The mutant is copied into a heap buffer of exactly its size (NULL for 0 bytes) and parsed into a poisoned view and report.
- **Invariants checked on every mutant:**
  - the status is OK or an IVSC refusal status, and the reason maps to that status as in the spec's reason table;
  - a refusal leaves the view poisoned, and an acceptance leaves the report poisoned;
  - the input is never written;
  - the segment index is NO_SEGMENT or below 16, and the byte offset is at most the length;
  - a second parse gives an identical status, view and report;
  - an acceptance has `total_bytes` equal to the length and re-encodes canonically to the input.
- **`ivsc.fuzz.reasons-covered`.** This case replays all seeds and requires every reason 1..18 and at least one acceptance. Observed counts across the seeds:

  | Reason | Count | Reason | Count |
  |---|---|---|---|
  | truncated | 3149 | size-mismatch | 378 |
  | too-large | 227 | segment-length | 74 |
  | magic | 526 | segment-width | 389 |
  | version | 384 | segment-attr | 322 |
  | flags | 327 | segment-reserved | 419 |
  | reserved | 614 | addr-wrap | 84 |
  | trailing | 2337 | order | 93 |
  | segment-count | 206 | overlap | 18 |
  | | | digest | 1069 |
  | | | word-range | 93 |

  Mutants accepted: 1,291.
- **The invariants bite.** In a scratch probe the parser wrote the view on a digest refusal. Seeds 01 and 02 failed at once (`a refusal (reason 17) wrote the view`). The probe was reverted.

### Named faults (`data/expected/faults/ivsc.ids`, `# suite=ivsc`)

Each fault has one site in `core/src/content/ivsc.c`, placed after the good statement.

| Fault | Site | Must-fail | Allowed prefixes (observed failures) | Must-pass |
|---|---|---|---|---|
| `IVSC_SKIP_DIGEST` | `diff = 0u` after the trailer compare | `ivsc.N27-data-bit-flip` | N27, N28, N29, N31, `ivsc.fuzz.` | `ivsc.P02-two-segments` |
| `IVSC_END_OFF_BY_ONE` | wrap limit 0xFFFF (end must be below 0x10000) | `ivsc.B10-len-65535` | B07, B10, B12, P05, X02 | `ivsc.B06-addr-wrap` |
| `IVSC_ACCEPT_WORD_400` | word maximum 0x400 | `ivsc.N24-word-400-first` | N24, B11 | `ivsc.N25-word-ffff-last` |
| `IVSC_COUNT_17` | count 17 passes the count rule, table read as 16 | `ivsc.B02-segs-17` | B02, `ivsc.fuzz.` | `ivsc.B01-segs-16` |
| `IVSC_TRUST_TOTAL_BYTES` | implied size refused only when above `total_bytes` | `ivsc.N34-table-short` | N34, `ivsc.fuzz.` | `ivsc.N11-table-size-mismatch` |
| `IVSC_ADJACENT_AS_OVERLAP` | `prev_end += 1` (adjacent compared with `<=`) | `ivsc.P04-adjacent` | P04, B07, B10, B12, X02 | `ivsc.N20-overlap-one-word` |

The gate records `faults=90/90` and `fault-runs=90` (84 + 6).

### Optional libFuzzer target (`tests/fuzz/fuzz_ivsc.c`)

- **What it does.** `LLVMFuzzerTestOneInput` copies the input into an exact-size buffer and reseals its last 32 bytes. It then parses twice and aborts on any broken invariant, using the same invariants as the family. Oversize inputs are skipped, because B08 covers them.
- **Build switch.** `option(IVS_ENABLE_LIBFUZZER ... OFF)` in `tests/CMakeLists.txt` adds the target only when ON. The target is `EXCLUDE_FROM_ALL`, set by no preset, never registered with ctest and never run in CI.
- **How it builds.** It compiles `ivsc.c` and `ivs_sha256.c` itself, so coverage instrumentation reaches the parser. `IVS_LIBFUZZER_SANITIZERS` (default `fuzzer,address,undefined`) sets the sanitizers.
- **Recipe.** The header names Homebrew LLVM (`$(brew --prefix llvm)/bin/clang` or `llvm@21`) or a Linux clang, because Apple clang lacks the runtime. It runs with `-max_total_time=60`.
- **Run result.** It was run as a throwaway, optional build in a scratch directory outside the repository:
  - With Homebrew LLVM 21 and LLVM 18 and the default sanitizer list, the binary hung in `__asan::AsanInitInternal` (a spin lock in `InitializeShadowMemory`) before any input. A `sample` of the process showed this. It is a host toolchain issue with ASan start-up on this macOS 26 host, not a parser finding.
  - With `-DIVS_LIBFUZZER_SANITIZERS=fuzzer,undefined` (Homebrew LLVM 21, Debug), the CMake-built target ran 30 s from the six positive records as seed corpus: 1,036,838 inputs, cov 369, ft 1217, corpus 48 entries, no crash.
  - The header records this fallback. The dev preset's seeded family still runs the parser under ASan on every gate run.

### Hygiene, documentation and claim

- **`.ivsc` hygiene.** Plan 03-07 had already added `*.ivsc` to `.gitignore` and `ivsc` to `tools/check/binary-ext.txt`, and both checks hold. No line was added; see Deviation 4.
- **Spec.** `docs/formats/ivsc-v1.md` gained a "Fuzzing" section: the seeded family, the faults and the optional target. It cites `[claim:CL-0007]` and uses no untagged claim word.
- **CL-0007.**
  - Statement: the parser accepts every positive and refuses every malformed record of `FX-ivsc-vectors` with the documented status, reason, segment index and byte offset, and the fuzz family reaches every refusal reason without breaking the invariants.
  - Artifact: `docs/formats/ivsc-v1.md`. Tier: `source-expectation`.
  - `depends_on`: `cartridge.features_supported` (delegated-decision) and `cartridge.slots` (unknown-default-chosen). The weakest-field rule therefore makes the claim `provisional`, and the gate reports `provisional=6`.
  - The note says this is conformance to the project's own specification, not to hardware or a third-party format, and that layer 2 is not covered.

## Task Commits

1. **Task 1 (tracer): the seeded fuzz family end to end in suite ivsc.** `5e42a1d` (test).
   - `cmake --build --preset dev && build/dev/tests/ivs_tests_ivsc --only ivsc.fuzz. && cmake --build --preset release && ctest --preset release -R '^ivsc$'` passed.
   - `sh tools/gate.sh` ended `gate: PASS ... tests-missing=0` (tests-expected 1386).
   - The tracer feedback gate re-ran the automated verify, and it was green before expansion.
2. **Task 2: six IVSC named faults and fixture N34.** `ed02f3f` (feat).
   - `sh` and `dash tools/gate.sh` ended `gate: PASS ... faults=90/90 ... tests-expected=1388`.
   - The manifest helper `--check` was current.
3. **Task 3: optional libFuzzer target, fuzzing section and CL-0007.** `bc5d803` (feat).
   - `sh` and `dash tools/gate.sh` both ended `gate: PASS ... claims=7 ... faults=90/90 ... controls=215/215`, with identical summary lines.

**Plan metadata:** recorded in the docs(03-08) completion commit.

## Acceptance evidence

| Check | Result |
|-------|--------|
| `grep -c '^ivsc\.fuzz\.' data/expected/tests/ivsc.ids` | 9 |
| `grep -v '^#' data/expected/faults/ivsc.ids \| grep -c .` | 6 |
| `grep -c 'IVS_FAULT_IVSC_' core/src/content/ivsc.c` | 6 |
| `grep -c -F 'IVS_ENABLE_LIBFUZZER' tests/CMakeLists.txt` | 2 (baseline 0) |
| `grep -c -x 'ivsc' tools/check/binary-ext.txt` | 1 |
| `grep -c -F '*.ivsc' .gitignore` | 1 |
| `grep -c -F '"id": "CL-0007"' data/claims.json` | 1 |
| tracked `.ivsc`/`.rom`/`.bin` files | 0 |
| gate faults / controls / claims | `faults=90/90`, `controls=215/215` (plan text: 213, see Deviation 1), `claims=7` |
| fuzz family dev wall time | 0.07 s (target under 0.3 s) |

## Decisions Made

See `key-decisions` in the frontmatter.

## Deviations from Plan

### Auto-fixed Issues

**1. [Plan count] Controls 215, not 213**
- The plan text predates the controls that plans 03-05 and 03-06 added. The orchestrator gave 215 as the current count. This plan adds no control. **Commit:** none needed.

**2. [Rule 1/2 - Fault design] IVSC_TRUST_TOTAL_BYTES narrowed, and fixture ivsc.N34-table-short added**
- **Issue:** The plan's fault skips the whole implied-size check, with must-fail "N09 or N11, whichever the check alone catches". N09 is caught by the trailing check, not this one. Skipping the whole check lets a table that claims more words than the file holds drive the word-range loop past the buffer. Fault executables are plain builds without ASan, so that over-read would crash or not depending on heap layout, and the control would be flaky.
- **Finding:** A scratch probe weakened `!=` to `>`, the memory-safe half. It showed that no fixed fixture pinned the table-smaller-than-`total_bytes` direction. Only the nine `ivsc.fuzz.*` cases failed.
- **Fix:**
  - The fault drops only that half and keeps the over-read guard.
  - New patch record `ivsc.N34-table-short`: P02 with segment 2 claiming one word, resealed. It is refused size-mismatch at byte 16 and is the must-fail.
  - `data/cart/ivsc-parse.json`, `data/expected/tests/ivsc.ids` and the `FX-ivsc-vectors` outputs (sha256 `9ded845c...b8e6`, 21,033 bytes) moved together in one commit. `tools/cart/manifest-outputs.sh --check` reports them current.
- **Commit:** `ed02f3f`.

**3. [Rule 1 - Fault design] IVSC_COUNT_17 reads the table as 16 entries**
- **Issue:** Raising the count limit to 17 alone would make the table walk write `v.segs[16]`, one past the 16-entry view on the stack. That is undefined behaviour in the plain fault build.
- **Fix:** The site, one line, turns a count of 17 into 16 before the count rule. A 17-entry container passes the count rule, its table is read as 16 entries, and the build stays in bounds.
  - B02 then fails with size-mismatch instead of segment-count.
  - The fuzz family also sees the loss: an accepted 17-count container does not re-encode to itself.
- **Commit:** `ed02f3f`.

**4. [Already done] `.ivsc` hygiene lines**
- Plan 03-07 (its Deviation 8) had already added `*.ivsc` to `.gitignore` and `ivsc` to `tools/check/binary-ext.txt`. Both acceptance greps print 1, and no duplicate line was added. **Commit:** none.

**5. [Structure] Interface header and shared re-encoder**
- **Issue:** The fuzz family and the optional target both need the canonical re-encoder, which was static in `ivsc_main.c`.
- **Fix:**
  - `tests/ivsc/ivsc_fuzz.h` declares the base accessors, the re-encoder and the fuzz case functions.
  - The re-encoder moved verbatim in behaviour to `tests/ivsc/ivsc_reencode.c`. It gained an entry-bounds guard and copies words byte by byte.
  - That file joins the `ivsc` test and fault-suite sources and the optional target.
- **Commits:** `5e42a1d`, `bc5d803`.

**6. [Rule 3 - Host blocker] `IVS_LIBFUZZER_SANITIZERS` cache string**
- **Issue:** On this host the Homebrew LLVM 18 and 21 ASan runtimes hang at start-up, so the planned `-fsanitize=fuzzer,address,undefined` target cannot run here.
- **Fix:** The sanitizer list is a cache string. The default is the planned `fuzzer,address,undefined`, and the recipe names `fuzzer,undefined` as the fallback. The target is still off by default and outside the gate. **Commit:** `bc5d803`.

**7. [Small addition] Extra fuzz invariants**
- Beyond the plan's list, the family also checks:
  - the reason-to-status mapping;
  - that an acceptance leaves the report untouched;
  - that the input is never written;
  - bounds on the segment index and byte offset;
  - that at least one mutant is accepted.
- They are cheap and close the "refusal writes only the report" contract from both sides. **Commit:** `5e42a1d`.

---

**Total deviations:** 7.
- Two keep fault executables memory-safe (2, 3), and Deviation 2 also closes a real fixture gap.
- One is a count update (1), one a no-op (4) and one structural (5).
- One works around a host toolchain blocker (6), and one is a cheap strengthening (7).

No existing test id, token, control or fault changed.

## Issues Encountered

- The first libFuzzer run, with ASan, spun for over two minutes in the ASan runtime's start-up and never reached the first input. A `sample` of the process located the hang, and the process was killed. See Deviation 6.

## Open items

- **SHA fault prefixes (03-02 open item):** still owed by the plan that connects SHA-256 to the loader (03-11). `faults/sha256.ids` carries `# suite=machine`, so this plan's `ivsc` cases cannot fail under those faults. `faults=90/90` confirms it.
- **MEM-05 and MEM-04 stay open.** The layer-2 load fixtures (L ids, X01 span drift, B07 load refusal) and the refusal matrix (plans 03-09 to 03-11) are still to come. Neither requirement was marked complete.
- **libFuzzer with ASan not run on this host.** The ASan configuration of the optional target was built but not run here, because of the runtime start-up hang. A Linux clang, or a later Homebrew LLVM, should run the default list. This is optional and outside the gate.
- **No fault for the trailing check.** The `total_bytes`-below-length check is pinned by N09, and growth mutants in the fuzz family also reach it, but no named fault covers it. The plan's six faults did not include one. Add one if a later plan revisits the IVSC faults.
- **Hosted timing:** read from the next hosted run (budget 40 s, revisit trigger 50 s).
- **Manifest refresh rule:** unchanged for later plans. Any edit of `data/cart/*.json` or `cart-fixture-files.ids` reruns `sh tools/cart/manifest-outputs.sh` and updates the manifest in the same commit.

## Known Stubs

None.

## Threat Flags

None. The new surface is covered by the threat register:
- T-03-25: the seeded family runs under ASan and UBSan in the dev preset, with bounded loops; the optional libFuzzer target is the deeper path.
- T-03-26: refusals write only the report, acceptances only the view, the input is unwritten and results are deterministic. Each is checked on every mutant.
- T-03-27: six named faults.
- T-03-28: `*.ivsc` is ignored and binary by name. The libFuzzer recipe keeps its corpus and crash files under the ignored `build/`.

## User Setup Required

None.

## Next Phase Readiness

- Plan 03-09 and later can rely on:
  - `data/expected/faults/ivsc.ids` as the place for any further parser fault;
  - `ivsc_test_reencode_equal` for any new acceptance test.
- Plan 03-11 can add L records to `data/cart/*.json`, with a manifest refresh. It may also widen the SHA fault prefixes once loader cases exist.
- **Final gate:**
  - `sh tools/gate.sh`: `gate: PASS steps=25/25 controls=215/215 cc=AppleClang c-presets=2 fault-runs=90 core-imports=4 ... claims=7 provisional=6 ... faults=90/90 tracked=1086 ... manifests=2 effective-yes=1 effective-no=1 ... tests-expected=1388 tests-executed=1388 tests-skipped=0 tests-missing=0 docs=12 claim-refs=6 tags=10 tiers=8` (`gate-time total=20`)
  - `dash tools/gate.sh`: the identical summary line (`gate-time total=23`).

## Self-Check: PASSED

- **Files found:** `tests/ivsc/ivsc_fuzz.c`, `tests/ivsc/ivsc_fuzz.h`, `tests/ivsc/ivsc_reencode.c`, `tests/fuzz/fuzz_ivsc.c` and `data/expected/faults/ivsc.ids`.
- **Commits:** `5e42a1d`, `ed02f3f` and `bc5d803` are ancestors of HEAD. The ledger count from `8b24c0b` is 3, and each message ends with the Co-Authored-By line.
- **Hygiene:**
  - No absolute home path and no "Intellivision" in any added line.
  - No `.ivsc`, `.rom` or `.bin` file is tracked.
  - `.planning/config.json` and `.planning/state.json` are still ` M` and in no commit of this plan.
