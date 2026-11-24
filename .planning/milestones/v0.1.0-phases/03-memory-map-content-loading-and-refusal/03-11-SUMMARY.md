---
phase: 03-memory-map-content-loading-and-refusal
plan: 11
subsystem: api
tags: [refusal-matrix, precedence, validate-then-commit, ivsc, layer-2, load-fixtures, named-faults, gate-measurement, D-08, D-10, D-11, D-12, D-13, D-26, D-33, D-34, D-37, A-06, A-07, A-08, MEM-03, MEM-04, MEM-05, API-06]

requires:
  - phase: 03-memory-map-content-loading-and-refusal
    provides: "plan 03-07: IVSC v1 parser, fixture generator and table; plan 03-09: public headers, instance struct, state digest, suite machine; plan 03-10: ivs_machine_load/unload with the A-08 ladder, IVS_REASON_* and the refused-load helper pattern"
provides:
  - "tests/machine/machine_refuse.c: the load refusal matrix (163 machine.refuse.load cases over priors f/x/s/c, 8 unload, 4 out-parameter rows) and 20 machine.prec rows, each with the full D-12 oracle"
  - "Out parameters aimed into the instance (load report, info out, digest out, create out) are refused BAD_ARG with nothing written; shared wrap-proof ivs_machine_span_overlaps (machine.h)"
  - "data/cart/ivsc-load.json and generator kind load: span-relative addresses resolved from the literal map's cart row, fail-closed span checks, in_span flag and resolved span in the fixture table"
  - "Suite ivsc load level: B03-B05, L01-L06 records, code cases L07-L10, B07-load-out-of-span, X01-span-matches-map"
  - "Nine named faults in machine_load.c (machine faults 20, total 110)"
  - "CI-EFFICIENCY measurement 2 (A-07)"
affects: [03-12, 03-13, 03-14]

actuals:
  tokens: 34200
  tasks: 3
  commits: 3
plan_head_before: 4753591c8e78ab14c0039464aa96b502e4f60d18
plan_head_after: b01095a63529c4aef1b2652048219b36b564c412

tech-stack:
  added: []
  patterns:
    - "Indexed refusal family: one row table expanded at first use into one case per (row, prior), dispatched after suite machine's case table"
    - "Refusal oracle per row: prior built in exact-size heap storage, snapshot of the whole host allocation, composite digest before and after, report poisoned with 0xA5 and checked field by field"
    - "Lying lengths in two placements (16-byte heap buffer, pointer into the middle of the instance) so a status that depended on buffer placement would show"
    - "Fault sites kept memory-safe for unsanitized fault executables: size-only reorderings, length narrowed before the whole call, whole-image buffers behind lying lengths"
    - "Load-level fixtures name segments, not bytes; the generator resolves span_lo/span_hi from the literal and fails closed when a record's reason no longer fits the resolved span"

key-files:
  created:
    - tests/machine/machine_refuse.c
    - data/cart/ivsc-load.json
  modified:
    - core/include/ivs/ivs_machine.h
    - core/src/machine/machine.h
    - core/src/machine/machine.c
    - core/src/machine/machine_digest.c
    - core/src/machine/machine_load.c
    - cmake/IvsCartTable.cmake
    - tests/ivsc/ivsc_main.c
    - tests/ivsc/ivsc_table.h
    - tests/machine/machine_main.c
    - tests/machine/machine_cases.h
    - tests/CMakeLists.txt
    - data/expected/tests/machine.ids
    - data/expected/tests/ivsc.ids
    - data/expected/faults/machine.ids
    - data/expected/faults/sha256.ids
    - data/expected/faults/ivsc.ids
    - data/expected/cart-fixture-files.ids
    - fixtures/FX-ivsc-vectors/manifest.json
    - .planning/CI-EFFICIENCY.md

key-decisions:
  - "Row placement follows the ladder: argument BAD_ARG and UNSUPPORTED rows run on f, s and c; OVERSIZED, input-overlap BAD_ARG, content and DIGEST_MISMATCH rows on f and x; DUPLICATE_LOAD rows only on occupied slots (s, c; c for the cartridge); handle and out-parameter rows carry no prior suffix"
  - "Expected reasons, segments and byte offsets in the matrix are literals from the fixture records or worked out from the container layout, never read back from the parser at test time"
  - "Out-parameter aliasing (orchestrator addition): ivs_machine_load refuses a report overlapping [m, m + sizeof instance) in its argument stage with nothing written; info and digest do the same for their out struct and buffer; create refuses an out pointer inside the bytes it may write (min(storage_bytes, instance size)) before *out is set"
  - "Load records: an absent segment_index means IVS_SEGMENT_NONE and an absent byte_offset or missing_features means 0, so every detail field of a load report is checked"
  - "B04 (one word past the span) expects addr-wrap: the literal span ends at FFFF; the generator refuses the record if a span change makes another reason true"
  - "Requirements: MEM-03 and MEM-05 complete; MEM-04 stays open for plan 03-12 (missing-system-image reset rows, powered prior); API-06 stays open for plan 03-13"

patterns-established:
  - "A refusal row is (id, op, kind, prior mask, builder, fixture, status, reason, segment, offset, missing, flags); builders only synthesize inputs and may poke the instance before the snapshot"

requirements-completed: [MEM-03, MEM-05]

coverage:
  - id: D1
    description: "Every load refusal class returns its own typed status with byte-identical storage and equal digest over its priors (D-12, SC3 load side)"
    requirement: MEM-04
    verification:
      - kind: unit
        ref: "build/dev/tests/ivs_tests_machine --only machine.refuse. (216 cases) and --only machine.prec. (20), ASan+UBSan; ctest --preset release -R '^(machine|ivsc)$'"
        status: pass
    human_judgment: false
  - id: D2
    description: "System-image shape refusals (short, long, wide word, input overlap) through the load path"
    requirement: MEM-03
    verification:
      - kind: unit
        ref: "machine.refuse.load.exec.{short-by-one,odd-length-*,oversized-*,wide-word-*,input-*}, grom.{short-by-one,oversized-*,input-inside}"
        status: pass
    human_judgment: false
  - id: D3
    description: "Layer-2 cartridge fixtures generated from the literal map, with the span drift detector"
    requirement: MEM-05
    verification:
      - kind: unit
        ref: "build/dev/tests/ivs_tests_ivsc (75 cases incl. L01-L10, B03-B05, B07-load, X01); sh tools/cart/manifest-outputs.sh --check fixtures/FX-ivsc-vectors/manifest.json"
        status: pass
    human_judgment: false
  - id: D4
    description: "Nine ordering and typing faults each fail a named row"
    requirement: API-06
    verification:
      - kind: integration
        ref: "sh and dash tools/gate.sh: faults=110/110 fault-runs=110"
        status: pass
    human_judgment: false

duration: 20min
completed: 2026-11-11
status: complete
---

# Phase 3 Plan 11: Load refusal matrix, precedence rows and layer-2 cartridge fixtures Summary

**Every load refusal class now has its own typed status, proven over its prior states with a whole-storage `memcmp`, an equal composite state digest and a report written only in its detail fields. Twenty `machine.prec.*` rows pin every adjacent pair of the load ladder, A-08 a and the cross-stage cartridge outcomes. Layer-2 cartridge fixtures take their addresses from the literal map. Nine named faults show that a mutation before refusal, replace-on-duplicate, a 32-bit length, a masking loader, swapped checks and an aliased report would each be caught. Out parameters aimed into the instance are now refused (orchestrator addition). Final gate under sh and dash (byte-identical summary lines): `gate: PASS steps=25/25 controls=215/215 ... fault-runs=110 ... faults=110/110 ... manifests=2 ... tests-expected=1968 tests-executed=1968 tests-skipped=0 tests-missing=0`.**

## Performance

- **Duration:** about 20 min (2026-11-11T07:48:59Z to 08:09Z).
- **Tasks:** 3 of 3.
- **Files:** 21 in 3 commits (diff +2650 / -60).
- **Suite machine:**
  - Release run: 0.159 s for 299 cases. The refuse and prec families take 0.127 s of that.
  - Dev run (ASan+UBSan): 0.33 s.
- **Suite ivsc:** release run 0.029 s for 75 cases.
- **Gate time (local, warm, loaded host):** median `gate-time total` 23 s (sh 24, 23, 23; dash 22). The measurement table is below.

## Accomplishments

### The refusal matrix (`tests/machine/machine_refuse.c`)

**Row table:** each row is `{id, op, kind, prior mask, builder, fixture, status, reason, segment, offset, missing, flags}`. It expands at first use into one case per (row, prior).

**Per case, the runner:**
1. Creates the instance in a fresh exact-size heap allocation, poisoned with 0x5A first. Lead rows add 16 KiB in front of it.
2. Builds the prior.
3. Builds the input (inputs are heap buffers or pointers into the allocation, never stack).
4. Takes the composite digest and a copy of the whole allocation.
5. Makes the call with a report poisoned with 0xA5.
6. Checks:
   - the status;
   - `struct_size`, reason, segment, offset and missing bits;
   - an all-zero image;
   - every allocation byte;
   - the digest.

   Report-refusal rows instead check that every report byte stayed poisoned. Positive controls check OK, a reported image and that the digest moved.

**Case counts (`machine.refuse.load.*`, 163 cases):**

| Class | Rows | Priors | Cases |
|---|---|---|---|
| BAD_ARG, argument values (kind 0/4/255, NULL bytes with len 8 per kind, opts short, opts size 0, report short, report inside the instance per EXEC and cartridge) | 11 | f s c | 33 |
| UNSUPPORTED, options (flag bit 1, flag bit 31, nonzero tail for EXEC and GROM) | 4 | f s c | 12 |
| DUPLICATE_LOAD (`duplicate-load`) | 3 | s c (EXEC, GROM); c (cartridge) | 5 |
| OVERSIZED | 11 | f x | 22 |
| BAD_ARG, input overlap (inside, per kind; 4096 bytes below and straddling) and the adjacent-below OK control | 5 | f x | 10 |
| MALFORMED (zero length with and without a pointer per kind; EXEC 8191, 4097, 1; GROM 2047; wide word first and last; 14 IVSC parse classes) | 26 | f x | 52 |
| OUT_OF_RANGE (address wrap, one word below the span, straddling the span start, EXEC region) | 4 | f x | 8 |
| OVERLAP (exact, partial) and the adjacent-segments OK control | 3 | f x | 6 |
| DIGEST_MISMATCH (trailer bit flip; `expect_sha256` per kind) | 4 | f x | 8 |
| UNSUPPORTED, layer 2 (feature bit 0, bit 31) | 2 | f x | 4 |
| BAD_HANDLE (NULL machine for EXEC and cartridge, destroyed instance) | 3 | none | 3 |

- **By prior:** f 70, x 55, s 17, c 18; 3 cases have no prior.
- **OVERSIZED rows:** EXEC 8193 bytes, GROM 2049 bytes, and the parser's `total_bytes` over the maximum (B09). Every lying length runs as both `-heap` and `-inside`:
  - EXEC claiming 1 MiB;
  - GROM claiming 1 MiB;
  - cartridge claiming 131,257 bytes;
  - EXEC claiming 2^32 + 8192 (`len-wrap`, `skip-ok` where `size_t` is 32 bits).
- **Unload rows (8):** NULL machine, destroyed instance, and kinds 0 and 4 on f, s and c.
- **Out-parameter rows (4):** `machine.refuse.info.out-inside`, `machine.refuse.digest.out-inside`, `machine.refuse.create.out-inside-storage` and the control `machine.refuse.create.out-after-instance-ok`.
- **Acceptance counts:**

  | Check | Got | Needed |
  |---|---|---|
  | `machine.refuse.load` ids | 163 | 60 or more |
  | `.x$` lines | 53 (55 with the two `skip-ok` lines) | 20 or more |
  | `duplicate-load` on f or x | 0 | 0 |
  | `inside` ids | 20 | 3 or more |
  | `machine.prec.*` ids | 20 | 14 or more |
  | the two A-08 a prec ids | 2 | 2 |
  | tracer ids `oversized-by-one.[fx]` | 2 | 2 |

### Precedence rows (`machine.prec.*`, 20)

Each pair is "first class wins over second", with the expected outcome:

| Row | Prior | Expected |
|---|---|---|
| `arg-over-handle` | f | BAD_ARG (kind 0 with a NULL machine) |
| `handle-over-unsupported` | f | BAD_HANDLE |
| `unsupported-over-duplicate` | s | UNSUPPORTED |
| `duplicate-over-oversized`, `duplicate-over-overlap`, `duplicate-over-malformed` | s | DUPLICATE_LOAD |
| `duplicate-over-digest` | c | DUPLICATE_LOAD |
| `oversized-over-overlap` | f | OVERSIZED: a lying 1 MiB length with the pointer inside the instance (A-08 a) |
| `overlap-over-malformed` | f | BAD_ARG: an 8191-byte span inside the instance |
| `oversized-over-malformed` | f | OVERSIZED: 8193 bytes with a wide first word |
| `malformed-over-out-of-range` | f | MALFORMED: width 8 on an entry that would also wrap |
| `wrap-over-overlap` | f | OUT_OF_RANGE: entry 0 wraps, entry 1 overlaps it |
| `overlap-over-digest` | f | OVERLAP: N20 without reseal |
| `malformed-over-expect-digest` | f | MALFORMED |

**Cartridge stage order (A-08 b), all on prior f:**

| Row | Expected |
|---|---|
| `cart-overlap-before-span` | OVERLAP |
| `cart-trailer-before-span` | DIGEST_MISMATCH |
| `cart-malformed-before-span` | MALFORMED |
| `cart-malformed-before-feature` | MALFORMED |
| `cart-span-before-feature` | OUT_OF_RANGE |
| `cart-feature-before-expect` | UNSUPPORTED |

### Out parameters aimed into the instance (orchestrator addition)

- **Shared check:** `ivs_machine_span_overlaps(p, n, inst, inst_bytes)` in `machine.c`, declared in `machine.h`. It uses only `uintptr_t` differences, so it cannot wrap. The input-overlap stage now uses it too.
- **Load report:** `ivs_machine_load` refuses BAD_ARG, with nothing written, when the report overlaps the instance, alongside the short-report check.
- **Info and digest:** `ivs_machine_info` and `ivs_machine_state_digest` refuse BAD_ARG when their out struct or out buffer overlaps the instance.
- **Create:** `ivs_machine_create` refuses BAD_ARG before `*out = NULL` when `out` lies inside the bytes create may write, so the storage stays untouched.
- **Documentation:** `ivs/ivs_machine.h` states the rule in the struct-rule paragraph and in the comments of create, info, state_digest and load. Plan 03-13 tabulates it: these are BAD_ARG rows with effect `none` and outs "nothing written".

### Layer-2 cartridge fixtures (`data/cart/ivsc-load.json`, group `ivsc-load`)

- **Record format:** the file names its profile. Records of the new kind `load` list segments, whose addresses are four hex digits or `span_lo±n` / `span_hi±n`, plus `required_features` and `expect.load`.
- **Generator (`cmake/IvsCartTable.cmake`):**
  - It resolves span-relative addresses from the single R cart row of `data/memmap/<profile>.literal.txt`. It fails closed if there is no single cart row.
  - It checks every record against the resolved span. A load that is not OUT_OF_RANGE must lie inside it. `out-of-span` needs a segment outside it. `addr-wrap` needs a segment past FFFF.
  - It emits `in_span` and the resolved span into the table.
  - Fail-closed behaviour was checked on a scratch copy. Moving L01 into the span, moving B03 to `span_hi+1`, giving B04 the wrong reason, and shifting the literal span each stop the generator with a message.
- **Records:**
  - B03 (last word of the span, OK);
  - B04 (one word past it, addr-wrap, since the span ends at FFFF);
  - B05 (one word below it);
  - L01 (`$0100`), L02 (`$1000`), L03 (straddling the span start), L04 (`$0200`), all out-of-span;
  - L05 and L06 (features 1 and 0x80000000).
- **How suite ivsc runs a load record:** it assembles and seals the container at test time and loads it on a fresh instance with the full oracle. An accepted load also checks the report, reads every word on the bus, and checks that unload returns the never-loaded digest.
- **Code cases:**

  | Case | What it checks |
  |---|---|
  | `L07-second-load-duplicate` | DUPLICATE_LOAD, and the first slot report is unchanged |
  | `L08-null-args` | BAD_HANDLE, BAD_ARG, and BAD_ARG when both arguments are bad |
  | `L09-unload-restores` | the digest and the instance bytes equal a fresh instance's |
  | `L10-two-instances` | B stays byte-identical, and its bus reads `$5000` as FFFF |
  | `B07-load-out-of-span` | OUT_OF_RANGE, segment 0, offset 24 |
  | `X01-span-matches-map` | every in-span record lies in the registry span, and each load record's literal span equals the registry span |

- **Manifest:** `fixtures/FX-ivsc-vectors/manifest.json` now lists two outputs; the `--check` passes.
- **ids:** `grep -c -E '^ivsc\.L(0[1-9]|10)-' data/expected/tests/ivsc.ids` prints 10.

### Named faults (`data/expected/faults/machine.ids`, now 20 rows; 110 in total)

| Fault | Site (`machine_load.c`) | Must-fail | Rows failed |
|---|---|---|---|
| `LOAD_LEN_BEFORE_CHECK` | slot `length_bytes` written before validation | `machine.refuse.load.exec.oversized-by-one.f` | 133 |
| `LOAD_TABLE_BEFORE_VALIDATE` | decode-table `exec_loaded`/`grom_loaded` set before validation | `machine.refuse.load.grom.short-by-one.f` | 58 |
| `DUPLICATE_REPLACES` | the occupancy check skipped, so the commit overwrites | `machine.refuse.load.exec.duplicate-load.s` | 9 |
| `LEN_TRUNCATED_32` | `len` narrowed to 32 bits before the call validates | `machine.refuse.load.exec.len-wrap-heap.f` | 4 |
| `ROM_DECLE_MASKED_NOT_REFUSED` | wide EXEC words accepted (left to the read mask) | `machine.refuse.load.exec.wide-word-first.f` | 4 |
| `CHECK_ORDER_MALFORMED_FIRST` | a wrong system-image size answered MALFORMED before the bound | `machine.prec.oversized-over-malformed` | 19 |
| `CART_COPY_BEFORE_VALIDATE` | the first segment installed before layer 2 | `machine.refuse.load.cart.feature-bit-0.f` | 17 |
| `OVERLAP_BEFORE_LENGTH` | the input-overlap test on the unbounded span, before the bound | `machine.prec.oversized-over-overlap` | 9 |
| `REPORT_INSIDE_UNCHECKED` | the report overlap check dropped (addition) | `machine.refuse.load.exec.report-inside.f` | 6 |

- **Must-pass ids:** `machine.load.exec-ok`, except `DUPLICATE_REPLACES`, whose must-pass is `machine.load.order-free`.
- **`OVERLAP_BEFORE_LENGTH` placement:** it failed only the inside placements here. Its heap placements could fail on another allocator, and the allowed prefix covers that.

**Existing faults re-judged** (each widening is for the fault's own rule):

| Fault | Now also allowed | Why |
|---|---|---|
| `INIT_SKIPS_MEMSET` | `machine.refuse.load.`, `machine.refuse.unload.`, `machine.prec.` | poisoned slot records |
| `SHA_K_WRONG`, `SHA_ROTATE_WRONG`, `SHA_LENGTH_LE` | the same three prefixes | the P02 prior's authored trailer is refused |
| `DESTROY_NO_SCRUB` | the two destroyed-handle rows | the magic survives destroy |
| `IVSC_END_OFF_BY_ONE` | `ivsc.B03-` | B03 ends on FFFF |

The new faults `LOAD_LEN_BEFORE_CHECK`, `LOAD_TABLE_BEFORE_VALIDATE` and `CART_COPY_BEFORE_VALIDATE` also allow family load's refusal cases, which compare every instance byte. Every must-pass id stays green.

### Gate measurement 2 (A-07; `.planning/CI-EFFICIENCY.md`)

The second point moved from 03-07 to 03-11 and is recorded there. All figures are local, on a warm tree, with the host loaded (1-minute load 7.3 to 10.3, 26 sessions). None is a hosted number; the hosted figure is the orchestrator's to record (last hosted run: 30 s; budget 40 s, revisit trigger 50 s).

| Run | Load | total | c-build | c-faults | faults-verdict | c-tests | controls | controls-wait |
|---|---|---|---|---|---|---|---|---|
| sh 1 | 8.35 | 24 | 1 | 2 | 3 | 2 | 12 | 0 |
| sh 2 | 8.74 | 23 | 0 | 3 | 3 | 2 | 11 | 0 |
| sh 3 | 8.47 | 23 | 1 | 2 | 3 | 1 | 11 | 0 |
| dash | 7.27 | 22 | 0 | 2 | 4 | 2 | 11 | 0 |

- **Median:** 23 s (03-02: 20 s at 73 faults; 03-10: 22 to 24 s at 101 faults).
- **Lever:** none applied. The trigger is a local median above 40 s with load below 4; it cannot be evaluated at this load and is not met.
- **Matrix cost:** the refusal matrix costs about twice its 0.06 s family budget (0.127 s), and the composite digest dominates. It stays, because it is the D-12 oracle. The machine suite as a whole is still inside its 0.25 s phase-end budget.

## Task Commits

1. **Task 1 (tracer): one row with the full oracle over priors f and x.** `a00c54a` (test).
   - `--only machine.refuse.load.exec.oversized-by-one` passed 2 cases.
   - The first gate run failed only on the expected re-judging (INIT_SKIPS_MEMSET and three SHA faults). After the prefixes were widened, sh and dash both PASS.
   - Tracer feedback gate: the automated verify was re-run end to end before expanding.
2. **Task 2: matrix, precedence, out-parameter refusals, layer-2 fixtures.** `0ce8a86` (feat).
   - Every verify command passed: `--only machine.refuse.` (216), `--only machine.prec.` (20), `ivs_tests_ivsc` (75), the manifest `--check`, and release ctest for `machine` and `ivsc`.
   - sh and dash gate PASS.
3. **Task 3: nine faults, measurement 2.** `b01095a` (test).
   - `grep -v '^#' data/expected/faults/machine.ids | grep -c .` prints 20.
   - sh and dash gate PASS with `faults=110/110 controls=215/215 tests-missing=0`. The summary lines are identical (`cmp`).

## Decisions Made

See `key-decisions` in the frontmatter.

## Deviations from Plan

### Auto-fixed Issues and additions

**1. [Orchestrator addition] Out parameters overlapping instance storage are BAD_ARG**
- **Where:** load report, info out, digest out, create out (`machine.c`, `machine_digest.c`, `machine_load.c`, header).
- **Tests:**
  - Matrix rows: `machine.refuse.load.{exec,cart}.report-inside.{f,s,c}`, `machine.refuse.info.out-inside`, `machine.refuse.digest.out-inside`, `machine.refuse.create.out-inside-storage` and the control `out-after-instance-ok`.
  - The named fault `REPORT_INSIDE_UNCHECKED`.
- **Count change:** faults are 110/110 and machine fault rows are 20, not the planned 109 and 19.
- **Commits:** `0ce8a86`, `b01095a`.

**2. [Plan count] Controls 215, not 213**
- The plan text predates the controls of plans 03-05 and 03-06. No control was added.

**3. [Repo reality] Fault sites shaped to stay memory-safe**
- **Why:** fault executables are unsanitized.
- **`CHECK_ORDER_MALFORMED_FIRST`:** reorders the size check only (a wrong EXEC or GROM size is answered MALFORMED before the bound). Scanning words behind a lying length would read past a 16-byte buffer.
- **`LEN_TRUNCATED_32`:** narrows `len` before the whole call validates, and its must-fail row's heap buffer holds a whole EXEC image. The narrowed load therefore succeeds visibly instead of reading 4 GiB.
- **`LOAD_TABLE_BEFORE_VALIDATE`:** sets the decode table's loaded flag, not the slot record. Setting the slot record would turn every load into DUPLICATE_LOAD, a mutant that every case catches, not a targeted one.
- **Commit:** `b01095a`.

**4. [Refactor for a fault site] `ivs_load_cart` takes the instance**
- It needs the instance so `CART_COPY_BEFORE_VALIDATE` can install a segment. A plain build carries `(void)im;`.
- **Commit:** `b01095a`.

**5. [Harness shape] Suite machine dispatches an indexed family**
- `machine_main.c` now builds its suite from callbacks: the case table first, then the refuse family. The (row, prior) ids are generated at first use, not written as 195 functions.
- **Commit:** `a00c54a`.

**6. [Generator extension] New record kind and table fields**
- **Generator:** a top-level `profile` key and the record kind `load`.
- **Table struct:** `ivs_cart_fixture` gains `profile`, `features`, `missing`, `in_span`, `span_lo` and `span_hi`.
- **X02:** skips load records.
- **B07 load refusal:** its id is `ivsc.B07-load-out-of-span`, a code case on the B07 build record.
- **Commit:** `0ce8a86`.

**7. [Process] TDD order in Task 2**
- **What happened:** the core out-parameter check and its rows were written together, so no RED run was observed.
- **Negative evidence:** the `REPORT_INSIDE_UNCHECKED` fault turns the report rows red, and the matrix rows are red under the eight ordering faults.
- **Gap:** the info, digest and create out-parameter checks have rows but no named fault.

---

**Total deviations:** 7.
- One addition the orchestrator asked for (1).
- One count update (2).
- Two repo-reality adjustments (3, 4).
- Three structural or process notes (5, 6, 7).

No existing test id, control, step or token changed meaning. Six existing fault prefix strings were widened, each for its own rule.

## Issues Encountered

- **Missing ids:** the first full gate after Task 2 failed `tests-unlisted` for the four out-parameter rows, because the id-collection pattern missed them. Every machine fault then failed `fault-list-mismatch`, a consequence of the same gap. Adding the ids fixed both.
- **Fault re-judging:** the existing faults needed re-judging against the new cases (table above).

## Open items

- **MEM-04 stays open:**
  - Plan 03-12 adds the missing-system-image reset rows and the powered prior `w`. Its BAD_STATE rows for load and unload are still unreachable until reset exists.
  - The unknown-profile rows exist (plan 03-09).
- **API-06 stays open (plan 03-13).** The effect table must carry the out-parameter BAD_ARG rule for load, info, state_digest and create. Plan 03-13's expected counts shift by one fault: its plan text says `faults=113/113`, which becomes 114 if plan 03-12 adds its four.
- **No named fault yet for three out-parameter checks:** those of info, state_digest and create. Only the load report check has one.
- **`len-wrap` rows** exist only where `size_t` is 64 bits (`skip-ok` elsewhere). The 32-bit path is not exercised on any current CI host.
- **Hosted timing:** the orchestrator reads it from the next hosted run. Locally the median is 23 s on a loaded host.
- **Machine-suite cost:** the refusal matrix is at twice its family budget. Watch the suite total at 03-12 (sweeps and isolation) and 03-14.
- **sha256 fault prefixes:** they were rechecked for the new families. Recheck them again when plan 03-12 adds cases that load the P02 prior or compare literal digests.

## Known Stubs

None.

## Threat Flags

No new surface beyond the register. The out-parameter refusal narrows existing surface: aliasing a report into the instance was the open item from 03-10.
- **T-03-37:** memcmp and digest on every row, plus four mutation-before-refusal faults.
- **T-03-38:** the 64-bit wrap rows, `LEN_TRUNCATED_32`, both placements and `OVERLAP_BEFORE_LENGTH`.
- **T-03-39:** the precedence rows, `CHECK_ORDER_MALFORMED_FIRST` and `ROM_DECLE_MASKED_NOT_REFUSED`.
- **T-03-40:** span-relative addresses resolved from the literal, the generator's fail-closed checks, and X01.

## User Setup Required

None.

## Next Phase Readiness

- **Plan 03-12:** add the powered prior `w` to the row table as one more prior bit, plus the reset rows. The runner and the prior builder take a new prior with a few lines.
- **Plan 03-13:** cite `machine.refuse.*`, `machine.prec.*` and the `ivsc.L*`/`B0*` ids per status. `IVS_REASON_*` lets the table name the cause of each refusal.

## Self-Check: PASSED

- **Files found:** `tests/machine/machine_refuse.c` and `data/cart/ivsc-load.json`.
- **Commits:** `a00c54a`, `0ce8a86` and `b01095a` are ancestors of HEAD. The ledger count from `4753591` is 3, and each message ends with the Co-Authored-By line.
- **Hygiene:**
  - No absolute home path and no "Intellivision" in any added line.
  - No rights-bearing bytes; every image and container is a generated stand-in or a project fixture.
  - `.planning/config.json` and `.planning/state.json` are still ` M` and in no commit of this plan.
