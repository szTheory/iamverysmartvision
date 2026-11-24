---
phase: 03-memory-map-content-loading-and-refusal
plan: 07
subsystem: content
tags: [ivsc, cartridge, parser, fixtures, manifest, status-codes, D-09, D-29, D-30, D-31, D-32, D-33, D-34, A-01, A-03, MEM-05, MEM-04]

requires:
  - phase: 03-memory-map-content-loading-and-refusal
    provides: "plan 03-02: core SHA-256 (ivs_sha256_digest); plan 03-05/03-06: the sourced literal map with the cart row R 4000-FFFF, checked by the decode dump"
provides:
  - "docs/formats/ivsc-v1.md: IVSC v1 in the project's own words (layout, big-endian rule, limits, check order, stable reason table, two digests, required_features, evolution, worked 66-byte example)"
  - "core/include/ivs/ivs_status.h: the full D-09 status list (-3..-5, -16..-23; -24..-31 reserved)"
  - "core/src/content/ivsc.{h,c}: ivs_ivsc_parse, a pure, zero-copy, bounded layer-1 parser with a pointer-free view and a report {reason, segment_index, byte_offset, required_features}"
  - "cmake/IvsCartTable.cmake: fail-closed build-tree table ivs_cart_table.inc from data/cart/*.json (listed files, listed ids, positive/build/patch records, span rule against the literal cart rows)"
  - "data/cart/ivsc-parse.json and suite ivsc: 50 parse-level ids (P01-P06, B01, B02, B06-B12, N01-N33, X02, X03) passing in dev and release"
  - "fixtures/FX-ivsc-vectors/manifest.json (effective yes) and tools/cart/manifest-outputs.sh (print and --check)"
  - "ivs_fault_suite(ivsc) registered, ready for data/expected/faults/ivsc.ids in plan 03-08"
affects: [03-08, 03-09, 03-11, 03-13, 03-14]

actuals:
  tokens: 23550
  tasks: 3
  commits: 3
plan_head_before: 02f35d9ce0fb1ccdbfbb4b15efa467e36ae9a634
plan_head_after: 1619bbf38a8b4b3310475636e4a0ab34e2b50ee7

tech-stack:
  added: []
  patterns:
    - "Fixture digests from an independent oracle: every positive trailer and whole-file digest is typed from `cmake -E sha256sum` at authoring time, so the core SHA-256 inside the parser meets a value it did not compute"
    - "Three fixture record kinds in one JSON file: positive (full hex), build (repeated words assembled by the test, typed trailer), patch (generator applies replace/insert/delete operations to a positive base; optional reseal at test time)"
    - "Span rule in the generator: positive segments must lie inside an R cart row of every literal map; an explicit format-edge mark is refused when it is not needed"
    - "Refusal writes only the report, acceptance only the view: every case fills both with sentinels and checks the untouched one"
    - "Exact-size heap buffers (NULL for zero bytes) so ASan in the dev preset sees any read past the container"

key-files:
  created:
    - docs/formats/ivsc-v1.md
    - core/src/content/ivsc.h
    - core/src/content/ivsc.c
    - cmake/IvsCartTable.cmake
    - data/cart/ivsc-parse.json
    - data/expected/cart-fixture-files.ids
    - data/expected/tests/ivsc.ids
    - tests/ivsc/ivsc_main.c
    - fixtures/FX-ivsc-vectors/manifest.json
    - tools/cart/manifest-outputs.sh
  modified:
    - core/include/ivs/ivs_status.h
    - core/CMakeLists.txt
    - tests/CMakeLists.txt
    - data/expected/fixtures.ids
    - .gitignore
    - tools/check/binary-ext.txt

key-decisions:
  - "IVSC reasons have stable numbers 1..18 (truncated, too-large, magic, version, flags, reserved, trailing, segment-count, size-mismatch, segment-length, segment-width, segment-attr, segment-reserved, addr-wrap, order, overlap, digest, word-range), each mapped to one status; the spec table and the generator's consistency table carry the same mapping"
  - "Table stage starts with a size pre-check (every entry plus one word per entry plus the trailer must fit in total_bytes) so a lying segment_count can never make an entry read pass the buffer; then per entry: len_words 0, width, attr, reserved, addr-wrap; then the implied-size check"
  - "Order and overlap are checked pair by pair in table order: a start below the previous start is order (MALFORMED), then a start below the previous end is overlap (OVERLAP); an identical duplicate is overlap"
  - "Report byte_offset names the defect's byte: the length for truncated, 131,256 for an oversize caller length, the field offset otherwise, total_bytes for trailing bytes, total_bytes - 32 for the digest, the word for word-range"
  - "BAD_ARG (NULL view, NULL report, NULL bytes with a length) writes nothing and precedes the length check"
  - "Long containers (P05 span-full, B07, B10, B12) are JSON build records with typed trailers rather than pure test code, so their digests stay independent of the core"
  - "SHA fault prefixes in faults/sha256.ids are not widened: those faults run suite machine only, and this plan connects SHA-256 to the parser (suite ivsc), not to the loader. The plan that connects the loader (03-11) still owes the widening"
  - "MEM-05 and MEM-04 are not marked complete: faults and the IVSC claim (03-08) and the layer-2 load fixtures (03-11) are still open"

patterns-established:
  - "data/cart/<stem>.json + data/expected/cart-fixture-files.ids + fixtures/FX-ivsc-vectors outputs move together in one commit (helper tools/cart/manifest-outputs.sh)"

requirements-completed: []

coverage:
  - id: D1
    description: "IVSC v1 specified in the project's own words (D-29..D-33, A-01, A-03)"
    requirement: MEM-05
    verification:
      - kind: other
        ref: "grep -c -F '89 49 56 53 43 0D 0A 1A' docs/formats/ivsc-v1.md -> 2; gate text, glossary and denylist steps pass (docs=12, scanned=156)"
        status: pass
    human_judgment: false
  - id: D2
    description: "Positive, boundary and malformed parse-level fixtures pass in both presets (D-34, T-03-21..T-03-23)"
    requirement: MEM-05
    verification:
      - kind: unit
        ref: "build/dev/tests/ivs_tests_ivsc (50 cases, ASan+UBSan); ctest --preset release -R '^ivsc$'"
        status: pass
      - kind: integration
        ref: "sh and dash tools/gate.sh: tests-expected=1368 tests-executed=1368 tests-missing=0"
        status: pass
    human_judgment: false
  - id: D3
    description: "Fixtures recorded in an effective-yes manifest with digests and sizes (D-34, Pitfall 9, T-03-24)"
    requirement: MEM-05
    verification:
      - kind: integration
        ref: "gate manifests=2 effective-yes=1; sh and dash tools/cart/manifest-outputs.sh --check fixtures/FX-ivsc-vectors/manifest.json"
        status: pass
    human_judgment: false

duration: 15min
completed: 2026-11-11
status: complete
---

# Phase 3 Plan 07: IVSC v1 cartridge format, parser, fixtures and manifest Summary

**IVSC v1, the project's own cartridge container, is specified in `docs/formats/ivsc-v1.md` and parsed by a pure, zero-copy, bounded core function (`ivs_ivsc_parse`) in the D-33 check order. 50 parse-level fixture ids (6 positive, 9 boundary, 33 malformed, 2 cross-cutting) pass in the dev and release presets, with every positive digest typed from `cmake -E sha256sum`. The fixtures are text only and recorded in the effective-yes manifest `FX-ivsc-vectors`. Final gate under sh and dash: `gate: PASS steps=25/25 controls=215/215 ... faults=84/84 ... manifests=2 effective-yes=1 effective-no=1 ... tests-expected=1368 tests-executed=1368 tests-skipped=0 tests-missing=0`.**

## Performance

- **Duration:** about 15 min of execution (2026-11-11T06:29:57Z to 06:45Z), not counting the reading before the recorded start
- **Tasks:** 3 of 3
- **Files:** 16 in 3 commits (diff +2214 / -1)
- **Gate time (local, warm build):** `gate-time total=20` to `22` (sh) and `19` (dash), against `20` to `22` before the plan. Suite `ivsc` runs in about 0.01 s in release. It includes three 131 KB containers assembled and hashed in test code. The generator runs once per fixture edit. Its cost is a few hundred CMake string operations on hex text. The new build cost is one test TU and one core TU. No new gate step, control or fault was added, so the hosted figure should stay near the 44 s of the last hosted run.

## Accomplishments

- **Status codes (D-09).** `ivs_status.h` now carries `BAD_HANDLE -3`, `UNSUPPORTED -4`, `BUFFER_TOO_SMALL -5` and the content codes `UNKNOWN_PROFILE -16` to `DIGEST_MISMATCH -23`. A comment records the rules: append-only, -6..-15 unassigned, -24..-31 reserved. The pin file and `ivs_status_name` stay with plan 03-09.
- **Parser (`core/src/content/ivsc.{h,c}`).** The parser checks in this order:
  1. length: OVERSIZED above 131,256 before any read, MALFORMED below 66;
  2. magic, reporting the first differing byte;
  3. version;
  4. prefix: flags, reserved, total_bytes against the limit and the length, segment_count 1..16;
  5. table: a size pre-check, then per entry the length, width, attr, reserved and addr-wrap checks (addr-wrap is OUT_OF_RANGE), then the implied size;
  6. order and overlap, pair by pair;
  7. the SHA-256 trailer, compared over all 32 bytes;
  8. word range, refused rather than masked.

  The code style rules:
  - size arithmetic is in `uint64_t`;
  - fields are read big-endian by shifts after widening;
  - every loop is bounded by a checked count;
  - data offsets are computed from the counts, never read from the file.

  On a refusal only the report is written; on an acceptance only the view. The view has no pointers, and `required_features` passes through it for layer 2. The core still imports only the allowed symbols (`core-imports=4`).
- **Spec (`docs/formats/ivsc-v1.md`).** It covers:
  - purpose and scope (the project's own format; importers live outside the core and each needs a rights review);
  - the conventions, the layout and the entry table;
  - limits and check order;
  - the reason table (number, name, status, byte offset, segment);
  - the two digests (A-04), required_features (D-32), evolution by version bump only;
  - a worked 66-byte example and where the fixtures live.

  It names no third-party format or emulator and uses no unwaived claim word.
- **Generator (`cmake/IvsCartTable.cmake`, prefix `ct_`).** It fails closed on:
  - an unlisted or missing file, an unknown key, a bad, duplicate or unlisted id;
  - non-lowercase or odd hex;
  - a patch outside the bytes, a base that is not a positive record;
  - an unknown reason, or a reason whose status does not match the spec table;
  - a span violation, a needless format-edge mark, a literal map without a cart row.

  It embeds the SHA256 of the fixture file and of the literal map as `# source` lines, and `tests-inventory` re-hashes them.

  Scratch probes confirmed every refusal above. Each probe ran the generator on a copied tree (`unlisted`, `missing`, `id-unlisted`, `span`, `needless-edge`, `status-map`, `unknown-key`, `patch-out`, `bad-reason`, `upper-hex`, `odd-group`, `bad-base`, `dup-id`, `no-cart-row`), and each stopped with an `IvsCartTable: <context>: <reason>` message.
- **Suite `ivsc` (`tests/ivsc/ivsc_main.c`).** There is one case per record:
  - positive: compare the whole-file digest, parse, compare the view;
  - build: assemble, append the typed trailer, compare the digest, parse;
  - patch: the generator's bytes, resealed with the core SHA-256 when asked, parsed and compared on status, reason, segment index and byte offset.

  Three cases are built in code:
  - `B08`: caller length 131,257 and SIZE_MAX with a 1-byte buffer, which is never read;
  - `X02`: every accepted record re-encoded from its view equals the input bytes;
  - `X03`: NULL arguments are BAD_ARG, write nothing and come before the length check.

  A mutation check of the generated table confirmed the comparisons bite. An offset changed by 1 on N20 and a P05 segment one word short made N20, P05 and X02 fail, and they passed again after regeneration.
- **Manifest and helper.** `fixtures/FX-ivsc-vectors/manifest.json` is effective yes:
  - kind `original-source`, committed, MIT, licence evidence `LICENSE` sha256 `aeb83b57...7c26`;
  - `author-attested` on 2026-11-11, allowed use `local-test, ci, repo`.

  It pins `data/cart/ivsc-parse.json` (sha256 `64a2d42c...5224`, 20,599 bytes). `tools/cart/manifest-outputs.sh` prints the entries under sh and dash. With `--check` it exits 1 on a difference; a scratch manifest with the size off by one was refused.
- **Hardening.** `*.ivsc` was added to `.gitignore` and to `tools/check/binary-ext.txt`. A stray container is now caught by name as well as by the content scan.

## Final parse-level id list (50)

| Class | Ids |
|-------|-----|
| Positive (6) | `ivsc.P01-min`, `ivsc.P02-two-segments`, `ivsc.P03-word-patterns`, `ivsc.P04-adjacent`, `ivsc.P05-span-full` (build), `ivsc.P06-partial` |
| Boundary (9) | `ivsc.B01-segs-16`, `ivsc.B02-segs-17`, `ivsc.B06-addr-wrap`, `ivsc.B07-two-segments-64k` (build, format-edge), `ivsc.B08-len-over-max` (code), `ivsc.B09-total-over-max`, `ivsc.B10-len-65535` (build, format-edge), `ivsc.B11-word-3ff-vs-400`, `ivsc.B12-max-bytes` (build, format-edge) |
| Malformed (33) | `ivsc.N01-empty`, `N02-len-65`, `N03-magic-bit`, `N04-magic-lf-only`, `N05-version-0`, `N06-version-2`, `N07-flags`, `N08-prefix-reserved`, `N09-trailing-byte`, `N10-total-plus-one`, `N11-table-size-mismatch`, `N12-count-0`, `N13-segment-length-0`, `N14-width-8`, `N15-width-16`, `N16-width-0`, `N17-attr-1`, `N18-attr-80`, `N19-segment-reserved`, `N20-overlap-one-word`, `N21-duplicate-segment`, `N22-inner-segment`, `N23-descending`, `N24-word-400-first`, `N25-word-ffff-last`, `N26-word-8000`, `N27-data-bit-flip`, `N28-trailer-bit-flip`, `N29-trailer-zero`, `N30-trailer-31-bytes`, `N31-digest-and-word`, `N32-magic-and-short`, `N33-version-and-flags` (all `ivsc.` prefixed) |
| Cross-cutting (2) | `ivsc.X02-canonical-roundtrip` (code), `ivsc.X03-bad-args` (code) |

Expected results of the negatives:
- MALFORMED by reason: truncated N01, N02, N10; magic N03, N04, N32; version N05, N06, N33; flags N07; reserved N08; trailing N09; size-mismatch N11, N30; segment-count N12, B02; segment-length N13; segment-width N14-N16; segment-attr N17, N18; segment-reserved N19; order N23; word-range N24-N26, B11.
- OVERLAP: N20-N22.
- OUT_OF_RANGE (addr-wrap): B06.
- OVERSIZED: B08, B09.
- DIGEST_MISMATCH: N27-N29, N31.

The precedence pairs are N31 (digest over word range), N32 (magic over length disagreement) and N33 (version over flags). Each record also asserts its byte offset, and its segment index where the reason concerns one segment.

## Authoring commands for every digest

All bodies were written to scratch files outside the repository and hashed with `cmake -E sha256sum` (CMake 4.4.3). `shasum -a 256` agreed on P01 for both files. The trailer is the digest of `<id>.body` (every byte before the trailer), and `whole_sha256` is the digest of `<id>.whole` (body plus trailer).

```
P01: body 34 bytes, whole 66 bytes
  cmake -E sha256sum P01.body  ->  c11992ef4947434da01aec35a64397065150011f25d776f89fcee5189a4990ef
  cmake -E sha256sum P01.whole  ->  5d774e95472f05ec629b2673481d0e05cd8a8e7f25db7cda6d94807424957dfd
P02: body 50 bytes, whole 82 bytes
  cmake -E sha256sum P02.body  ->  4f5d67b73899b74bb5a4fdb244a18befad13f3ec660ec6cbaaba6f4bee9b50df
  cmake -E sha256sum P02.whole  ->  aa287dcdf1a5f57b0a8be2aece819478cff63124743f3b6716224a5115914b78
P03: body 40 bytes, whole 72 bytes
  cmake -E sha256sum P03.body  ->  86b8ee7c51a7cb1a9bbca06a95d11259cf3bf8debccb9e4275f7abc66553f7ce
  cmake -E sha256sum P03.whole  ->  6bfc2c3ead19f983202ba60114db065d51f9334c208905d6c0414c3179d047cc
P04: body 46 bytes, whole 78 bytes
  cmake -E sha256sum P04.body  ->  d2339a45fe0d715e08f56d2c7d614f89e161d65a5bc8aa1eeecdfbb1868083ae
  cmake -E sha256sum P04.whole  ->  a43f0e7154c89cac4672f4697990ddcc6678fe42141c22d27d3ff31979a9eca2
P06: body 64 bytes, whole 96 bytes
  cmake -E sha256sum P06.body  ->  605f0a1e8e24b07f0e242c29a386994e2f01db28ff4f5d2a7560735d562b3408
  cmake -E sha256sum P06.whole  ->  1d66ebb5f3dea25ee3bf1363da0c515d9feb9b4730b683d4da781085d3a50e4a
B01: body 184 bytes, whole 216 bytes
  cmake -E sha256sum B01.body  ->  6d9364640e2f505fd644fee03a768fd0fa8c182b2ec4fc1fed821adbb52b4843
  cmake -E sha256sum B01.whole  ->  df9de7ac68157805352fe2fb36215d1e556233e4c295cbf74627298252fb192b
P05: body 98336 bytes, whole 98368 bytes
  cmake -E sha256sum P05.body  ->  3f39c15943ed6801fb672b206794c77e8047b00fa1690149cdda5220de8bb236
  cmake -E sha256sum P05.whole  ->  1112f24dd2d37bdf6adf2814e9e4c43c586b0fc179e173caba8b0d87e304a29a
B07: body 131112 bytes, whole 131144 bytes
  cmake -E sha256sum B07.body  ->  1aecd3ffbed69a981b72522f4c2da4d695cd4371ca463839ebe841dd2b9cee69
  cmake -E sha256sum B07.whole  ->  4ba25816ec5546ee1c23c84739eb8890545417e7bf14bf85993208f48c6cb5b6
B10: body 131112 bytes, whole 131144 bytes
  cmake -E sha256sum B10.body  ->  d76c5599c8b2778d63745bfcd53640489246a8d972db537afa166ff58fef617b
  cmake -E sha256sum B10.whole  ->  27874ab4d6b03224b1780fcb7f21cfbfb1f1b7c43e269d551e748ac543d48ba0
B12: body 131224 bytes, whole 131256 bytes
  cmake -E sha256sum B12.body  ->  30044418d2feb89ede0e99a47e878a6ed3ceefb9d9e9973b9772918048aea64d
  cmake -E sha256sum B12.whole  ->  0e426006ad055c459e1a3d055aa5a5edf7c27f8d5aee8b851c2db811aea3d96f
```

The bodies were built by a scratch script, separate from the core and the test, from the layout in the spec. Patch records carry no typed digest. Their reseal uses the core SHA-256 at test time, and that function is itself checked against FIPS vectors and `string(SHA256)` in suite `machine` (plan 03-02).

## Task Commits

1. **Task 1 (tracer): the 66-byte minimum container from a JSON fixture through the generator, the core parser and suite ivsc.** `00629af` (feat). `cmake --build --preset dev && build/dev/tests/ivs_tests_ivsc --only ivsc.P01-min` passed, and `sh tools/gate.sh` ended `gate: PASS` (tests-expected 1270). The tracer feedback gate (auto mode, automated verify) re-ran green before expansion.
2. **Task 2: spec and the full parse-level inventory.** `e67809e` (feat). All 50 cases passed in dev (ASan+UBSan), `ctest --preset release -R '^ivsc$'` passed, and `sh tools/gate.sh` ended `gate: PASS ... tests-missing=0` (tests-expected 1368).
3. **Task 3: fixture manifest and refresh helper.** `1619bbf` (feat). `sh` and `dash tools/cart/manifest-outputs.sh --check fixtures/FX-ivsc-vectors/manifest.json` passed. `sh tools/gate.sh` and `dash tools/gate.sh` both ended `gate: PASS ... faults=84/84 ... manifests=2 effective-yes=1`.

**Plan metadata:** recorded in the docs(03-07) completion commit.

## Acceptance evidence

| Check | Result |
|-------|--------|
| `tracked=$(git ls-files) && printf '%s\n' "$tracked" \| grep -c -E '[.](ivsc\|rom\|bin)$'` | 0 (baseline 0) |
| `grep -c -E '^ivsc\.N[0-9]{2}-' data/expected/tests/ivsc.ids` | 33 |
| `grep -c -E '^ivsc\.P0[1-6]-' data/expected/tests/ivsc.ids` | 6 |
| `grep -c -F '89 49 56 53 43 0D 0A 1A' docs/formats/ivsc-v1.md` | 2 |
| `grep -c '^FX-ivsc-vectors$' data/expected/fixtures.ids` | 1 |
| gate manifests stat | `manifests=2 effective-yes=1 effective-no=1` (baseline `manifests=1 effective-yes=0`) |
| gate faults / controls | `faults=84/84`, `controls=215/215` (plan text: 213, see Deviations) |

## TDD note (Task 2)

The tracer task already held the whole parser, so the 49 new cases ran against existing code and passed on their first run. RED evidence for the comparisons comes from the mutation check of the generated table described above. The named faults of plan 03-08 add RED evidence for the parser itself.

## Decisions Made

See `key-decisions` in the frontmatter.

## Deviations from Plan

### Auto-fixed Issues

**1. [Plan count] Controls 215, not 213**
- The plan text predates the controls that plans 03-05 and 03-06 added. The orchestrator gave 215 as the current count. This plan adds no control. **Commit:** none needed.

**2. [Rule 2 - Missing check] Table size pre-check before reading entries**
- **Issue:** D-33's order reads the entries in the table stage. With a lying `segment_count` (for example 16 in a 66-byte file), reading 16 entries would pass the buffer before the implied-size check could run.
- **Fix:** Before any entry is read, the parser checks that every entry, one word per entry and the trailer fit in total_bytes (reason size-mismatch). The spec states it as step 5's first check. **Commit:** `00629af`.

**3. [Small addition] Boundary B12-max-bytes and cross-cutting X03-bad-args**
- **Issue:** No planned id accepts a container of exactly 131,256 bytes, so the length bound had no off-by-one witness. No planned id covered the BAD_ARG contract.
- **Fix:** `ivsc.B12-max-bytes` (16 segments of 4,096 words) and `ivsc.X03-bad-args`. The inventory is 50 ids. **Commit:** `e67809e`.

**4. [Form] P05, B07 and B10 are JSON build records, not pure code cases**
- **Issue:** The plan calls B07, B08, P05 and X02 code-built, and B10 (also 131,144 bytes) cannot be literal hex either. Pure code would have to compute its own trailer with the core SHA-256, losing the independent digest.
- **Fix:** These records describe their words as repeats in the JSON. The test assembles them, and each carries an authoring-time trailer and whole-file digest from `cmake -E sha256sum`. B08 and X02 (with X03) are code cases. **Commit:** `e67809e`.

**5. [Form] Patch operations include insert and delete**
- **Issue:** N04 (CR removed from the magic), N09 (a trailing byte), N30 (the trailer cut by one byte) and B02 (a seventeenth entry and word) cannot be written as in-place replacements of a positive base.
- **Fix:** The patch record allows `{offset, hex | insert | delete}` operations, applied in order by the generator. **Commit:** `00629af`.

**6. [Fixture design] N04 and N32 use the 82-byte base P02**
- **Issue:** On the 66-byte P01, removing one byte or truncating by one gives 65 bytes. The length check fires before the magic check, so the planned reason (magic) could not be reached.
- **Fix:** Base P02, so the shortened container still clears the 66-byte minimum. **Commit:** `e67809e`.

**7. [Fixture design] B11 is a patch of P03**
- **Issue:** One record cannot expect both "03FF accepted" and "0400 refused".
- **Fix:** B11 raises P03's accepted 03FF word by one to 0400 (resealed, word-range). P03 itself is the accepted half. **Commit:** `e67809e`.

**8. [Rule 2 - Hardening] `*.ivsc` binary by name and ignored**
- RESEARCH and PATTERNS suggested it. Two one-line additions, `.gitignore` and `tools/check/binary-ext.txt`; no control or step changed. **Commit:** `00629af`.

**9. [Context] SHA fault prefixes not widened**
- **Issue:** The orchestrator flagged that `faults/sha256.ids` prefixes must be widened by the plan that connects SHA-256 to the loader.
- **Finding:** This plan connects SHA-256 to the layer-1 parser in suite `ivsc`. The SHA faults carry `# suite=machine`, so their executables run only suite `machine`, and no `ivsc` case can fail under them. `faults=84/84` confirms it. The widening stays with the loader plan (03-11), which adds `machine.load.*` cases. **Commit:** none.

---

**Total deviations:** 9. Two concern parser safety or contract (2, 3). Five are fixture forms the plan's wording did not allow (4 to 7, and the 50-id count). One is a count update (1), one a cheap hardening (8), and one a recorded non-change (9). No existing test id, token, control or fault changed.

## Issues Encountered

- The first tracer build failed on `-Wunused-function` for a helper (`ic_get16`) that only Task 2 uses. It was removed from Task 1 and returned with X02.
- The spec's first draft used the claim word "passes" once, and the text step's word list caught it during review before the gate run. The line was reworded.

## Open items

- **SHA fault prefixes (03-02 open item):** still owed by the plan that connects SHA-256 to the loader (03-11). See Deviation 9.
- **MEM-05 and MEM-04 stay open.** Still to come:
  - the ivsc faults, fuzz suite and IVSC claim (plan 03-08);
  - the layer-2 load fixtures (L ids, X01 span drift, B07 load refusal) and the refusal matrix (plans 03-09 to 03-11).

  Neither requirement was marked complete.
- **Report `required_features` on refusals.** It is filled from the prefix once the length check has passed. No fixture yet carries a nonzero mask; layer-2 fixtures (L05, L06) will.
- **Multi-profile span rule.** A positive must lie inside a cart row of every literal map. A second profile with a narrower span would force fixtures into the intersection, or a per-profile mark. Revisit when a second literal exists.
- **Hosted timing.** The local gate is unchanged at 19 to 22 s. The cold hosted cost of the two new TUs and of the generator is read from the next hosted run (budget 40 s, revisit trigger 50 s).
- **Manifest refresh rule.** It holds for every later plan: any edit of `data/cart/*.json` or `data/expected/cart-fixture-files.ids` reruns `sh tools/cart/manifest-outputs.sh` and updates `fixtures/FX-ivsc-vectors/manifest.json` outputs in the same commit. The `manifests` step fails otherwise.

## Known Stubs

None.

## Threat Flags

None. The new surface is covered by the threat register:
- T-03-21: `uint64_t` arithmetic, OVERSIZED before any read (B08 with a 1-byte buffer), addr-wrap (B06), total_bytes cross-checks (N09 to N11) and the table size pre-check.
- T-03-22: data offsets are computed from checked counts, exact-size heap buffers run under ASan in dev, and truncations are covered by N01, N02, N10 and N30.
- T-03-23: the digest is checked before the word range, expected trailers come from `cmake -E sha256sum`, and N27 to N29 and N31 cover it.
- T-03-24: fixtures are text JSON only, the manifest is effective yes, and `*.ivsc` is now binary by name.

## User Setup Required

None.

## Next Phase Readiness

- Plan 03-08 can add `data/expected/faults/ivsc.ids` (`# suite=ivsc`). The fault suite is already registered with `ivsc/ivsc_main.c` and the generated table. Fault sites go in `core/src/content/ivsc.c`.
- Plan 03-11 can take `ivs_ivsc_view` for layer 2, read the segment words at `data_byte_offset`, and use `required_features` from the view. It can add L records to a new or the same `data/cart/*.json` (with a manifest refresh).
- **Final gate:**
  - `sh tools/gate.sh`: `gate: PASS steps=25/25 controls=215/215 cc=AppleClang c-presets=2 fault-runs=84 core-imports=4 ... faults=84/84 tracked=1080 ... manifests=2 effective-yes=1 effective-no=1 ... tests-expected=1368 tests-executed=1368 tests-skipped=0 tests-missing=0 docs=12 ...` (`gate-time total=20`)
  - `dash tools/gate.sh`: the same summary line (`gate-time total=19`).

## Self-Check: PASSED

- Files: the spec, `ivsc.h`/`ivsc.c`, the generator, the fixture JSON, both inventories, the test, the manifest and the helper were found.
- Commits: `00629af`, `e67809e` and `1619bbf` are ancestors of HEAD. The ledger count from `02f35d9` is 3, and each commit message ends with the Co-Authored-By line.
- Hygiene: no absolute home path and no "Intellivision" in any added line. No `.ivsc`, `.rom` or `.bin` file is tracked. `.planning/config.json` and `.planning/state.json` are still ` M` and in no commit of this plan.
