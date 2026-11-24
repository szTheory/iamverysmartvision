---
phase: 03-memory-map-content-loading-and-refusal
plan: 10
subsystem: api
tags: [public-api, content-loading, validate-then-commit, ivsc, layer-2, sha256, two-digest, named-faults, D-10, D-11, D-13, D-24, D-25, D-26, D-27, D-32, D-33, A-01, A-04, A-06, A-08, MEM-03, MEM-04, API-06]

requires:
  - phase: 03-memory-map-content-loading-and-refusal
    provides: "plan 03-04: memsys install primitives, slot clear, profile registry (exec/grom slot shapes, cart span, features_supported); plan 03-07: IVSC v1 layer-1 parser and the generated fixture table; plan 03-09: public headers, the instance struct with slot records, ivs_image_report, state digest and suite machine"
provides:
  - "Public ivs_machine_load and ivs_machine_unload (seven IVS_API prototypes in ivs/ivs_machine.h), ivs_load_options, ivs_load_report (embeds ivs_image_report, A-08 f), IVS_LOAD_F_EXPECT_SHA256, IVS_REASON_* 0..33 and IVS_SEGMENT_NONE"
  - "core/src/machine/machine_load.c: validate (const instance, stack plan) then a void copy-only commit, in the A-08 ladder order"
  - "Layer 2 of the cartridge load (span, features) and the internal seam ivs_machine_cart_span_check"
  - "ivs_ivsc_view.file_sha256: the whole-file SHA-256 from the parser's single pass (A-04)"
  - "tests/ivsc/ivsc_table.h: the fixture-table row types shared by suites ivsc and machine"
  - "Suite machine family load (16 cases) and three named faults (machine faults 11)"
affects: [03-11, 03-12, 03-13, 03-14]

actuals:
  tokens: 15690
  tasks: 3
  commits: 3
plan_head_before: 75237de280e321777c4766b7e9105c80d26a60a0
plan_head_after: ca80f1f0ff5e555d66648af1a4337bb3c790d3eb

tech-stack:
  added: []
  patterns:
    - "Load ladder as stages: argument values, handle, options, state, slot, length bound, bounded-span overlap, content, expect digest; one refuse helper fills the report detail"
    - "One-pass two digests: a copied SHA-256 context is finished for the trailer comparison while the original continues over the trailer for the whole-file digest"
    - "A fault site that only a seam can reach gets a seam case with a synthetic input (cart-span-end)"

key-files:
  created:
    - core/src/machine/machine_load.c
    - tests/machine/machine_load.c
    - tests/ivsc/ivsc_table.h
  modified:
    - core/include/ivs/ivs_machine.h
    - core/src/machine/machine.h
    - core/src/machine/machine.c
    - core/src/content/ivsc.h
    - core/src/content/ivsc.c
    - core/CMakeLists.txt
    - tests/CMakeLists.txt
    - tests/ivsc/ivsc_main.c
    - tests/machine/machine_cases.h
    - tests/machine/machine_main.c
    - data/expected/tests/machine.ids
    - data/expected/faults/machine.ids
    - data/expected/faults/sha256.ids

key-decisions:
  - "Load ladder (as implemented): BAD_ARG (kind, NULL bytes with len > 0, opts struct_size short; a short report struct is BAD_ARG with nothing written), BAD_HANDLE, UNSUPPORTED (unknown option flag, nonzero opts tail), BAD_STATE (POWERED), DUPLICATE_LOAD, OVERSIZED (u64 length against 8192 / 2048 / 131,256), BAD_ARG input overlap (wrap-proof uintptr_t differences, only on a bounded span), content (system: MALFORMED short then EXEC word width; cartridge: parser in D-33 order, then span OUT_OF_RANGE, then features UNSUPPORTED), DIGEST_MISMATCH (expect_sha256)"
  - "IVS_REASON_* 1..18 equal the IVSC v1 reason table; 19..33 are the load's own reasons (span, features, system-image size and width, expect digest, argument, handle, option, state and slot)"
  - "A refused load writes the report's detail fields only (reason, segment_index, byte_offset, missing_features; struct_size set) and leaves image all zero; segment_index is IVS_SEGMENT_NONE whenever no single segment is concerned, success included"
  - "The cartridge OVERSIZED of the caller length is checked by the loader (reason TOO_LARGE, offset 131,256, as the parser would report) so the overlap test runs only on a bounded span"
  - "Layer-2 OUT_OF_RANGE reports the offending entry's addr field offset (24 + 8 * index); FEATURES reports offset 12 and the missing bits"
  - "Cartridge report: length_units is the total decle count, unit_bits 10, base_addr the first segment's address, sha256 the whole file (view.file_sha256)"
  - "unload of an empty slot returns OK without writing; a loaded slot's record is reset to {kind} and the memsys slot clear scrubs its words"
  - "No requirement is marked complete: MEM-03's system-image shape refusals (short, long, wide word, overlap) are tested by plan 03-11's matrix; MEM-04 needs 03-11/03-12; API-06 needs 03-13"

patterns-established:
  - "Refused-load helper (tests): poisoned report, call, compare status, detail fields, zero image and every instance byte"
  - "Literal image digests typed from `cmake -E sha256sum` over scratch files of generated stand-in bytes"

requirements-completed: []

coverage:
  - id: D1
    description: "Typed EXEC, GROM and cartridge loads with length and digest reports (D-25, SC2)"
    requirement: MEM-03
    verification:
      - kind: unit
        ref: "build/dev/tests/ivs_tests_machine --only machine.load. (16 cases, ASan+UBSan); ctest --preset release -R '^(machine|ivsc)$'"
        status: pass
    human_judgment: false
  - id: D2
    description: "Layer 2 refusals and option refusals leave the instance byte-identical (D-12)"
    requirement: MEM-04
    verification:
      - kind: unit
        ref: "machine.load.cart-out-of-span, cart-feature, expect-mismatch, flags-unknown"
        status: pass
    human_judgment: false
  - id: D3
    description: "Three named faults for the positive load contract (D-37)"
    requirement: API-06
    verification:
      - kind: integration
        ref: "sh and dash tools/gate.sh: faults=101/101 fault-runs=101"
        status: pass
    human_judgment: false

duration: 12min
completed: 2026-11-11
status: complete
---

# Phase 3 Plan 10: Typed content loading, layer 2 and the two-digest pass Summary

**A host can now load a stand-in EXEC, GROM and IVSC cartridge through `ivs_machine_load`, one typed image per call, and get each image's length, units, width, base and SHA-256 back. It can also unload any slot back to the fresh state. Validation runs on a const instance and fills a stack plan. Commit is void and copy-only. The cartridge path runs the layer-1 parser, then layer 2: span OUT_OF_RANGE, then features UNSUPPORTED. The parser now produces the trailer check and the whole-file digest in one pass. Final gate under sh and dash (identical summary lines): `gate: PASS steps=25/25 controls=215/215 ... fault-runs=101 ... faults=101/101 ... tests-expected=1548 tests-executed=1548 tests-skipped=0 tests-missing=0`.**

## Performance

- **Duration:** about 12 min (2026-11-11T07:27:55Z to 07:39Z).
- **Tasks:** 3 of 3.
- **Files:** 16 in 3 commits (diff +1510 / -67).
- **Suite machine wall time:** dev (ASan+UBSan) 0.14 s for the whole suite, up from 0.12 s. Release ctest for `machine` and `ivsc` together takes 0.07 s.
- **Gate time (local, warm build):** `gate-time total` 22 to 24 s (sh 22, 24, 23; dash 23), against 21 to 26 s before the plan.
  - New build cost: one core TU, one test TU per preset, and three owner copies for the faults.
  - No gate step or control was added.
  - The hosted figure is still to be read (budget 40 s, revisit trigger 50 s).

## Accomplishments

### Public surface (`ivs/ivs_machine.h`)

- **Structs:**
  - `ivs_load_options { struct_size; flags; expect_sha256[32] }` (40 bytes).
  - `ivs_load_report { struct_size; reason; segment_index; missing_features; byte_offset (u64); ivs_image_report image }` (88 bytes). It embeds the same struct `ivs_machine_info` returns (A-08 f).
- **Constants:** `IVS_LOAD_F_EXPECT_SHA256 0x1u`, `IVS_SEGMENT_NONE 0xFFFFFFFFu` and `IVS_REASON_*` (table below).
- **Prototypes:** `ivs_machine_load` and `ivs_machine_unload`, each a one-line `IVS_API` declaration. `grep -c '^IVS_API ' core/include/ivs/ivs_machine.h` prints 7, and `grep -c 'ivs_image_report image;'` prints 1.

### Implemented load ladder (first failing stage wins)

| Stage | Status | Check | Reason |
|---|---|---|---|
| 1 | BAD_ARG | kind not 1..3; `bytes == NULL && len != 0`; `opts->struct_size` short. A short `report->struct_size` is refused first and nothing is written. | ARG_KIND, ARG_NULL_BYTES, ARG_STRUCT_SIZE |
| 2 | BAD_HANDLE | NULL or wrong magic | HANDLE |
| 3 | UNSUPPORTED | an option flag other than bit 0; a nonzero byte in a larger opts tail | OPTION_FLAGS, OPTION_TAIL |
| 4 | BAD_STATE | phase POWERED | STATE_POWERED |
| 5 | DUPLICATE_LOAD | slot loaded | SLOT_OCCUPIED |
| 6 | OVERSIZED | `(uint64_t)len` above 8192 (EXEC), 2048 (GROM) or 131,256 (cartridge), before any byte is read | IMAGE_LONG (offset = bound), TOO_LARGE for a cartridge (offset 131,256) |
| 7 | BAD_ARG | the bounded span overlaps `[m, m + sizeof instance)`. `b >= s ? b - s < sizeof : s - b < len`, on `uintptr_t`, with `len != 0`. | ARG_INPUT_OVERLAP |
| 8 | MALFORMED | system image shorter than its exact size (zero included, with or without a pointer); then the first EXEC word with a bit of 15..10 set | IMAGE_SHORT (offset = len), IMAGE_WORD (offset of the word) |
| 8 | parser codes | cartridge: `ivs_ivsc_parse` in D-33 order. Its reason, segment and offset are copied into the report. | 1..18 |
| 8 | OUT_OF_RANGE | layer 2: the first segment (table order) not wholly inside `memmap.cart_span`. Any start is allowed (A-01). | OUT_OF_SPAN (segment, offset 24 + 8i) |
| 8 | UNSUPPORTED | layer 2: `required_features & ~cartridge.features_supported` | FEATURES (offset 12, missing bits) |
| 9 | DIGEST_MISMATCH | `expect_sha256` against the reported digest, by a fixed 32-byte XOR-accumulate loop | EXPECT_DIGEST |

- **Commit:** a `switch` over the kind:
  - EXEC: `ivs_memsys_install_exec`.
  - GROM: `ivs_memsys_install_grom`.
  - Cartridge: `ivs_memsys_install_cart_segment` per segment, at its computed data offset.
  - It then writes the slot record. The commit has no check and no early return.
- **Report:** the public function builds the report on the stack and copies it out once (`sizeof` bytes, `struct_size` set).
  - On success, `image` is the new slot's report.
  - On a refusal, only the detail fields are written and `image` stays all zero.
- **Unload** refuses in this order:
  1. BAD_ARG for an unknown kind.
  2. BAD_HANDLE.
  3. BAD_STATE when POWERED.

  An empty slot is OK with no write. Otherwise the slot record is reset to its kind and `ivs_memsys_clear_slot` scrubs the words, the loaded flag and, for a cartridge, the segment table.

### Reason constants (`IVS_REASON_*`)

| No. | Name | Status |
|---|---|---|
| 0 | NONE | OK |
| 1 to 18 | TRUNCATED, TOO_LARGE, MAGIC, VERSION, FLAGS, RESERVED, TRAILING, SEGMENT_COUNT, SIZE_MISMATCH, SEGMENT_LENGTH, SEGMENT_WIDTH, SEGMENT_ATTR, SEGMENT_RESERVED, ADDR_WRAP, ORDER, OVERLAP, DIGEST, WORD_RANGE | as in the reason table of `docs/formats/ivsc-v1.md` |
| 19 | OUT_OF_SPAN | OUT_OF_RANGE |
| 20 | FEATURES | UNSUPPORTED |
| 21 | IMAGE_SHORT | MALFORMED |
| 22 | IMAGE_LONG | OVERSIZED |
| 23 | IMAGE_WORD | MALFORMED |
| 24 | EXPECT_DIGEST | DIGEST_MISMATCH |
| 25 | ARG_KIND | BAD_ARG |
| 26 | ARG_NULL_BYTES | BAD_ARG |
| 27 | ARG_STRUCT_SIZE | BAD_ARG |
| 28 | ARG_INPUT_OVERLAP | BAD_ARG |
| 29 | HANDLE | BAD_HANDLE |
| 30 | OPTION_FLAGS | UNSUPPORTED |
| 31 | OPTION_TAIL | UNSUPPORTED |
| 32 | STATE_POWERED | BAD_STATE |
| 33 | SLOT_OCCUPIED | DUPLICATE_LOAD |

### Two digests in one pass (A-04)

- **Parser change:** `ivs_ivsc_parse` now hashes bytes `0..total-33` into a zero-initialized context. It copies the context and finishes the copy for the trailer comparison. It then feeds the 32 trailer bytes into the original and finishes it into the new `ivs_ivsc_view.file_sha256`.
- **What did not change:** the parser keeps its D-33 order and stays pure. A refusal still writes only the report.
- **Ivsc suite:** it now also checks `view.file_sha256` against every positive and build record's typed `whole_sha256`, and the ivsc suite stays green (60 cases).

### Stand-in images and literal digests (D-24, D-34)

The bytes were generated in a scratch directory by a throwaway script, which is not committed. The digests were then computed with `cmake -E sha256sum exec.bin grom.bin`:

| Image | Bytes | sha256 (cmake -E sha256sum) |
|---|---|---|
| EXEC, word i = `(37*i + 11) & 0x3FF`, high byte first | 8192 | `e1a0eed96da89af0f85ae8027b461669320b22f9a4c1c7ff20e627e6d2d2eee2` |
| GROM, byte i = `(7*i + 3) & 0xFF` | 2048 | `dfff795a6b8cdf421e2e0815987ba9eed246a3474ee26aeff7e70f0f2e5cc16b` |

- **Placing the literals:** both were pasted into the test mechanically (byte array generated from the hex string).
- **Cartridge digests:** they come from the fixture records' typed `whole_sha256` (plan 03-07).
- **No image is embedded in or fetched by the core.**

### Family load (16 cases, `tests/machine/machine_load.c`)

**EXEC and GROM:**
- `exec-ok`: report fields, the literal digest, an info slot equal to the report, and bus reads at `$1000`, `$1001` and `$1FFF`.
- `exec-unload-fresh`: the digest and every instance byte equal a fresh instance's; a second unload is OK.
- `grom-ok`: all 2048 bus reads.
- `grom-unload-fresh`.

**Cartridge:**
- `cart-ok`: uses P01. The address and the word come from the fixture record and its bytes; the next address reads `0xFFFF`.
- `cart-whole-digest-not-trailer`: P01 and P02. The digest equals the typed whole digest and the test-side SHA-256, and it is not the trailer.
- `cart-span-full`: 49,152 words built from the registry span; both end words read back.
- `cart-out-of-span`: a segment starting one word below the span. Expects OUT_OF_RANGE, segment 0, offset 24, and the instance unchanged.
- `cart-span-end`: the layer-2 seam with the synthetic span `[$4000, $7FFF]`; covers both edges and both table positions.
- `cart-feature`: masks `0x1` and `0x80000001`. Expects UNSUPPORTED, `missing_features` equal to the mask, and the instance unchanged.

**Options:**
- `expect-ok`: the EXEC literal, and a cartridge's whole digest.
- `expect-mismatch`: the EXEC literal with its last bit flipped, and a cartridge whose expected digest is its own trailer. Both are DIGEST_MISMATCH with the instance unchanged.
- `flags-unknown`: flags `0x2` and `0x3`.

**Order-free and unload:**
- `order-free`: all six orders give one composite digest and identical instance bytes.
- `info-all-slots`.
- `all-unload-all-fresh`.

### Named faults (`data/expected/faults/machine.ids`, now 11 rows)

| Fault | Site (`machine_load.c`) | Must-fail | Observed failures (all allowed) | Must-pass |
|---|---|---|---|---|
| `CART_REPORT_TRAILER_DIGEST` | the plan digest overwritten with the trailer | `machine.load.cart-whole-digest-not-trailer` | cart-ok, cart-whole-digest-not-trailer, expect-mismatch, expect-ok | `machine.create.ok-info-destroy` |
| `UNLOAD_KEEPS_IMAGE` | unload passes slot 0 to the memsys clear, so the words and the loaded flag stay while the slot record clears | `machine.load.exec-unload-fresh` | exec-unload-fresh, grom-unload-fresh, all-unload-all-fresh (prefix `machine.load.`) | `machine.create.ok-info-destroy` |
| `CART_SPAN_END_INCLUSIVE` | the layer-2 end limit raised by one word | `machine.load.cart-span-end` | cart-span-end | `machine.create.ok-info-destroy` |

**Existing faults re-judged:**
- **`SHA_K_WRONG`, `SHA_ROTATE_WRONG` and `SHA_LENGTH_LE`** break the load cases' literal digests, which is their own rule. Each now also allows `machine.load.`.
- **`INIT_SKIPS_MEMSET`** leaves poisoned slot records, so a load sees an occupied slot, again the fault's own rule. It now also allows `machine.load.`.
- **Unchanged:** `SHA_PAD_BOUNDARY`, `SHA_FINAL_BLOCK_DROPPED`, `SHA_INIT_NO_RESET` and the seven other machine faults fail no load case. Every must-pass id stays green.

## Task Commits

1. **Task 1 (tracer): EXEC end to end.** `b057df1` (feat).
   - Verify: `build/dev/tests/ivs_tests_machine --only machine.load.exec` passed 2 cases.
   - The first gate run failed in `faults-verdict` on the expected re-judging (the SHA and INIT_SKIPS_MEMSET faults now failing `machine.load.exec-*`). After the prefixes were widened, `sh` and `dash tools/gate.sh` both ended `gate: PASS ... faults=98/98 ... tests-missing=0`.
   - Tracer feedback gate: the automated verify was re-run end to end inside the gate before the expansion.
2. **Task 2: GROM, cartridge, layer 2, two digests, options, order-free.** `4b0dcf1` (feat).
   - `--only machine.load.` passed 16 cases, and `ctest --preset release -R '^(machine|ivsc)$'` passed 2 of 2.
   - `--list | grep -c '^machine\.load\.'` prints 16 (15 or more required). `ivs_tests_ivsc` exits 0.
   - `sh` and `dash tools/gate.sh` both PASS with `tests-expected=1548`.
3. **Task 3: three faults.** `ca80f1f` (test).
   - `grep -v '^#' data/expected/faults/machine.ids | grep -c .` prints 11.
   - `sh` and `dash tools/gate.sh` both end `gate: PASS steps=25/25 controls=215/215 ... fault-runs=101 ... faults=101/101 ... tests-missing=0`. The two summary lines are byte-identical (`cmp`).

**Plan metadata:** recorded in the docs(03-10) completion commit.

## Decisions Made

See `key-decisions` in the frontmatter.

## Deviations from Plan

### Auto-fixed Issues

**1. [Plan count] Controls 215, not 213**
- The plan text predates the controls of plans 03-05 and 03-06, as in plan 03-09. No control was added, so the gate reads `controls=215/215`.
- **Commit:** none needed.

**2. [Rule 3 - Blocking] Fault sites land with Task 3, not Task 2**
- **Issue:** the gate refuses a site with no fault row (`fault-orphan-site`).
- **Fix:** the two cartridge fault blocks were written during Task 2. They were held out of the Task 2 commit and committed with their rows in Task 3.
- **Commits:** `4b0dcf1`, `ca80f1f`.

**3. [Repo reality] `CART_SPAN_END_INCLUSIVE` needs a seam case**
- **Issue:** the registered cartridge span ends at `$FFFF`. A segment that ends one word past it is refused first by the parser's address-wrap check, as OUT_OF_RANGE with reason ADDR_WRAP. So through the public API the fault would be an equivalent mutant.
- **Fix:**
  - The layer-2 span rule is the internal function `ivs_machine_cart_span_check(lo, hi, view)`, declared in `machine.h`, not in `core/include`.
  - The dedicated case `machine.load.cart-span-end` drives it with a synthetic span. The plan allowed "a dedicated end-of-span case".
  - `machine.load.cart-out-of-span` still covers the lower edge through the public API.
- **Commits:** `4b0dcf1`, `ca80f1f`.

**4. [Small refactors outside the file list]**
- **Shared fixture types:** `tests/ivsc/ivsc_table.h` holds the fixture-table row types that `ivsc_main.c` used to define privately. Both suites include it, instead of the machine test keeping a second copy. `ivsc_main.c` also gained the `view.file_sha256` check.
- **Executable order:** `tests/CMakeLists.txt` declares `ivs_tests_machine` after suite ivsc, because `IVS_CART_INC` is set there.
- **Sources line:** family load exports the fixture table's source list, so the machine result file names `data/cart/ivsc-parse.json` and the literal map for tests-inventory.
- **Shared slot report:** `machine.c` gained `ivs_machine_slot_report`, shared by info and the load report.
- **Commits:** `b057df1`, `4b0dcf1`.

**5. [Design detail] Reasons for every refusal class**
- **What was done:** argument, handle, option, state and slot reasons (25 to 33) were added beyond the system-image ones the plan named. Every refused load that can write its report now names its cause.
- **Why:** the 03-11 matrix can then assert on the reason, not only the status.
- **Commit:** `b057df1`.

**6. [Process] TDD order in Task 2**
- **What happened:** the Task 2 implementation was written before its cases, so no RED run was observed for those cases.
- **Negative evidence instead:** the three named faults (each turns its case red) and the observed failure sets of the re-judged SHA faults.
- **Commit:** `4b0dcf1`.

---

**Total deviations:** 6.
- One count update (1).
- One blocking-rule ordering (2).
- One repo-reality adjustment (3).
- Three small refactors, design details or a process note (4, 5, 6).

No existing test id, control or step changed meaning. Four fault prefix strings were widened, each for its own rule.

## Issues Encountered

- **CMake variable order:** the first Task 2 build would have compiled the machine executable without the cartridge table, because `IVS_CART_INC` is defined after the machine block. It was caught on reading and fixed by moving the executable declaration.
- **Orphan fault sites:** the first Task 2 gate run failed on them (deviation 2).

## Open items

- **Requirements stay open:**
  - **MEM-03:** its function is delivered here and ROADMAP SC2 holds, but the system-image shape refusals have no test yet. Those are short, long, wide word and input overlap (reasons 21, 22, 23 and 28, and the A-08 a precedence). Plan 03-11's matrix covers them, and that plan also lists MEM-03.
  - **MEM-04:** needs the 03-11 matrix and the 03-12 reset rows.
  - **API-06:** needs the 03-13 effect table.
- **BAD_STATE paths of load and unload** are not reachable through the public API until plan 03-12 adds reset. They are untested now.
- **Report aliasing:** the report buffer is not checked for overlap with the instance (A-06 names input buffers only). The report is written after the commit, from a stack copy. A host that points the report into its own instance would corrupt that instance. This is worth a sentence in the 03-13 effect table.
- **Precedence rows:** the `machine.prec.*` rows for the load ladder, including the A-08 a row and the cross-stage cartridge rows, are plan 03-11's.
- **sha256 fault prefixes:** they were re-checked for the new family. Recheck them whenever suite machine gains literal-digest cases again (03-11).
- **Hosted timing:** read it from the next hosted run (budget 40 s, revisit trigger 50 s). The local warm gate takes 22 to 24 s.

## Known Stubs

None. Every load path writes real state. The reason constants are all reachable except `STATE_POWERED`, which becomes reachable with reset in plan 03-12.

## Threat Flags

None beyond the register:
- **T-03-33:** a const validate and a void commit; every refusal case compares every instance byte.
- **T-03-34:** the length bound comes before the wrap-proof overlap test. The matrix rows that exercise it are plan 03-11's.
- **T-03-35:** the core computes the whole-file digest; literal digests come from `cmake -E sha256sum` and the fixture records; `CART_REPORT_TRAILER_DIGEST`; and a fixed-length expect comparison.
- **T-03-36:** stand-in bytes only.

## User Setup Required

None.

## Next Phase Readiness

- **Plan 03-11** can drive every refusal class through `ivs_machine_load` and assert on status, reason, segment, offset and missing bits. The `ml_refused` helper pattern is in the test file: it checks the poisoned report, the zero image and every instance byte.
- **Plan 03-12** adds reset. Load and unload already refuse BAD_STATE when the phase is POWERED.
- **Final gate:**
  - `sh tools/gate.sh`: `gate: PASS steps=25/25 controls=215/215 cc=AppleClang c-presets=2 fault-runs=101 core-imports=4 purity-selftest=5/5 ... faults=101/101 tracked=1100 ... tests-expected=1548 tests-executed=1548 tests-skipped=0 tests-missing=0 docs=12 claim-refs=6 tags=10 tiers=8` (`gate-time total=23`)
  - `dash tools/gate.sh`: the identical summary line (`gate-time total=23`).

## Self-Check: PASSED

- **Files found:** `core/src/machine/machine_load.c`, `tests/machine/machine_load.c` and `tests/ivsc/ivsc_table.h`.
- **Commits:** `b057df1`, `4b0dcf1` and `ca80f1f` are ancestors of HEAD. The ledger count from `75237de` is 3, and each message ends with the Co-Authored-By line.
- **Hygiene:**
  - No absolute home path and no "Intellivision" in any added line.
  - No rights-bearing bytes; every image is a generated stand-in.
  - `.planning/config.json` and `.planning/state.json` are still ` M` and in no commit of this plan.
