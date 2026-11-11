---
phase: 04-headless-runner-and-installed-consumer
plan: 15
subsystem: tools/check, tests
tags: [listing, boot-stub, anti-hle, tools-pack, D-26, D-29, RUN-06, RUN-04]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-14: fixtures/FX-boot-stub/boot-stub.listing.txt (header keys, zero-fill HLT rows with text 'fill', '-' only for operand words, 10-digit bit groups), ivs_pack --dump-words (upper-case 'ADDR WORD'); 04-LEVER-02: parallel ctest in c-tests; counts steps=29/29 controls=254/254 faults=140/140 tests-expected=2460"
provides:
  - "Gate step `listing` (tools/check/listing.cmake + steps/listing.sh): grammar, hex-equals-bits, leaf row, operand count, range and header rules over data/expected/listings.ids (exact set), and the D-29 boot-stub guards"
  - "data/expected/listings.ids (`<id> <kind> <path>`, kinds boot-stub and cart), data/expected/bootstub-rows.ids (HLT DIS EIS MVII MVO TSTR B J)"
  - "listing.cmake dump mode: -DDUMP=<id> prints the checker's own parse as 'ADDR WORD' lines (only when that listing is clean)"
  - "Nine negative controls (one per token) and a toy cart listing in controls/_good/tree"
  - "TAP suite `pack` in both presets: pack.words-<id> = ivs_pack --dump-words vs the checker's parse, byte for byte"
affects: [04-16, 04-18, 04-23]

actuals:
  tokens: 9500
  tasks: 3
  commits: 3
plan_head_before: 731c9c2937f10efcb0fce00dc6f249c6ba1951d6
plan_head_after: 67ffc58

tech-stack:
  added: []
  patterns:
    - "A checker with a dump mode: the same cmake -P script that gates a file prints its own parse of it, so a second, independently written tool's decode can be compared byte for byte without either side being the core"

key-files:
  created:
    - tools/check/listing.cmake
    - tools/check/steps/listing.sh
    - data/expected/listings.ids
    - data/expected/bootstub-rows.ids
    - data/expected/gate-steps/listing.ids
    - data/expected/controls/listing.ids
    - tests/pack-tap.sh
    - data/expected/tests/pack.ids
  modified:
    - tests/CMakeLists.txt

key-decisions:
  - "The operand-count rule reads the `words` column of data/cp1610/opcode-families.json (added to the copied vectors.cmake row loader); an SDBD prefix is not modelled, so a listing that uses one fails closed (listing-operand-count) until a listing needs it"
  - "Header keys are recognised only from a fixed set (id kind contract profile window label entry stack) and only before the first word line; required: id, kind, window, plus profile and contract for kind boot-stub; id and kind must equal the inventory line"
  - "A word line that does not parse as ADDR HEX4 BITS ROW ASM, or whose bit groups are not exactly 10 digits, reports listing-bits-mismatch (no seventh grammar token, so the plan's one-control-per-token count holds)"
  - "listing-unlisted scans every FILELIST *.listing.txt outside tools/check/controls/ (control trees carry toy listings by design)"
  - "The stub vector guard reads the profile at data/profiles/<header profile>.json, requires its id to equal the header value and its reset.r7 and interrupt.iab_value values to be $XXXX, and needs a non-zero listed word at each"
  - "No emulator-name token in this step: the denylist step already scans listings (they are not in its excluded paths) for the cite-list names"

patterns-established:
  - "bootstub-* controls overlay a toy stub listing, a two-line inventory and a toy-stub profile with only the two vector fields, so the good tree's profile and the profile step stay untouched"

requirements-completed: []

duration: about 8 min from the first edit (measured), plus the container gate
completed: 2026-11-11
---

# Phase 4 Plan 15: listing checker, boot-stub guards and the two-parser pack suite Summary

**Every hand-assembled listing is now gate-checked three ways: the new `listing` step checks each line's hex against its bit groups, its row against the most specific leaf row of the decle, the operand lines, the address range and the header; for the project boot stub it also enforces the D-29 anti-HLE guards (1000-101F, at most 16 non-zero words, eight allowed rows, non-zero words at the profile's reset and interrupt addresses); and suite `pack` requires `ivs_pack --dump-words` to equal the checker's own parse byte for byte.**

## Performance

- **Duration:** about 8 min from the first edit (measured); load average about 16 during the gates.
- **Tasks:** 3 of 3 (the tracer first; its gate passed before Task 2).
- **Files:** 45 changed (44 created), most of them control files.
- **listing step cost (measured, warm):** `step-time listing 0` in every gate; standalone 0.135-0.140 s wall for the step (one `cmake -P` over 74 family rows and the 22-word stub), 0.11 s for one `-DDUMP` run.
- **Suite `pack` plain-run time (W7, measured, 3 runs each):** dev 0.25 s (0.247-0.256), release 0.14 s (0.139-0.148), wall, `sh tests/pack-tap.sh`; under ctest 0.24 s (dev) and 0.12 s (release). Most of it is one cmake start; the dev side also pays the ASan runtime start of `ivs_pack`.
- **Controls:** 9 more control runs; `step-time controls` was 19-28 s across the three gates (19 s before, within the noise of this loaded laptop). Gate totals: Task 1 `gate-time total=33`; Task 2 `32`; final `43` (sh) and `38` (dash).

## Counts

Start: `steps=29/29 controls=254/254 faults=140/140 tests-expected=2460`.
End: `steps=30/30 controls=263/263 fault-runs=140 faults=140/140 tests-expected=2462 tests-executed=2462 tests-skipped=0 tests-missing=0 listings=1 listing-words=22` (one new case in both presets). No discrepancy was added; the next free id stays **C68**.

Acceptance checks:
- `grep -c 'decle' tools/check/listing.cmake` = 7.
- `sh tools/check/run-controls.sh | grep -c -E 'control ok +(listing-[a-z-]+|bootstub-(size|row|vector))$'` = 9; `control ok _good/listing` present; last line `controls=263/263`.
- `grep -c '^pack.words-boot-stub$' data/expected/tests/pack.ids` = 1.
- `ctest --preset dev -R '^pack$'` and `ctest --preset release -R '^pack$'` pass.

## What changed

- **tools/check/listing.cmake:** copies `vectors.cmake`'s capped-failure skeleton, the row loader (plus `words`), `decle_leaves` and `program_word` (one rule set; no new decoder semantics). Tokens: `listing-bits-mismatch`, `listing-row-mismatch`, `listing-operand-count`, `listing-range`, `listing-header`, `listing-unlisted`, `bootstub-size`, `bootstub-row`, `bootstub-vector`. Stats `listings=<n> listing-words=<n>`. Dump mode `-DDUMP=<id>` needs only ROOT and prints nothing but `ADDR WORD` lines (via one `cmake -E echo_append`), and exits non-zero with no dump if that listing has a finding.
- **Non-vacuity checked by hand on scratch copies of the real stub:** TSTR relabelled MOVR (row-mismatch, decle 0080 labelled by TSTR), a bit flipped (bits-mismatch), the MVII immediate removed (operand-count and range), J relabelled JSR (row-mismatch, bootstub-row, operand-count), the 1004 word zeroed (bootstub-vector at $1004), an unknown profile (bootstub-vector), the id key removed (listing-header); an EIS fill word in the stub passed.
- **Controls (each fails with only its own token):** bits-mismatch (5002 bits differ from 0155), row-mismatch (0000 labelled ALL; ZERO refines ALL), operand-count (overlaid toy table gives ONES two words, so ONES at 5000 lacks its operand line), range (address gap 5001 to 5003), header (no id key), unlisted (an extra `data/listings/extra.listing.txt`), bootstub-size (40 words 1000-1027, 20 non-zero), bootstub-row (a stub word labelled ALL, outside the toy `bootstub-rows.ids` ZERO ONES; the toy table has no JSR row), bootstub-vector (the interrupt entry word moved from 1004 to 1005).
- **Suite pack:** `tests/pack-tap.sh <root> <ivs_pack>`, TAP shape, nonce, trailer and exit codes as `headers-tap.sh`; temp files under `mktemp -d`; registered after `headers` with `SKIP_RETURN_CODE 77` and `CMAKE=${CMAKE_COMMAND}`. Failure path checked: an `ivs_pack` wrapper adding `--patch 1001=000F` gives `not ok 1 pack.words-boot-stub` with the diff as `#` lines and exit 1. Runs under sh and dash.

## Deviations from Plan

**1. `data/expected/bootstub-rows.ids` landed in Task 1 (the plan lists it in Task 2).** The checker carries the D-29 guards from its first commit, and the real stub needs the row set to pass; the guard controls followed in the next commit, as the grammar controls did by the plan's own task split. Commit `13c1258`.

**2. Control `bootstub-row` uses the toy row ALL instead of a JSR row.** The good tree's family table has only ALL, ZERO and ONES; a JSR row would fail as row-mismatch first. The same rule (a stub row outside `bootstub-rows.ids`) is exercised; on the real stub a JSR label was checked by hand (above). Commit `51d77c9`.

**3. `listing-operand-count` control overlays the toy family table (ONES words 2) rather than deleting a `-` line**, because every toy row has one word. The finding is the plan's "missing operand line" case (`ONES lacks 1 operand line(s)`). Commit `51d77c9`.

## Open Items

- Plan 04-16: `diag-cpu-1 cart fixtures/FX-diag-cpu-1/diag-cpu-1.listing.txt` joins `listings.ids` and `pack.words-diag-cpu-1` joins `pack.ids`. Its header needs `id`, `kind`, `window` (5000-5037); the cart kind gets no stub guards. If the diagnostic uses SDBD the operand rule must learn the prefix first (it fails closed today).
- Emulator names in listing text: the denylist step covers the cite-list names; the vector-only list name is not checked in listings. Add a token and control only if a listing ever needs it.
- RUN-06 and RUN-04 stay Pending: the listing gate and the two-parser comparison exist; the diagnostic that reaches a hand-derived predicate is plan 04-16, and RUN-04's remaining evidence is later plans.
- Carried from 04-14 and 04-13: the runner (04-18) and installed consumer (04-23) take `build/<preset>/images/boot-stub.exec`; the consumer should pin the v1 sizes from the installed headers; the first appended request/report field needs a v1-size constant (A-02); C14 and C63-C67 revisited in Phase 5; `API-02`, `PKG-03`, `PKG-04`, RUN-01, RUN-04, RUN-05 Pending; `pending-resolvers=17`, `unknown-fields=16`; `sh tools/hygiene.sh --history` still fails on earlier planning commits (this plan's commits add no home paths). Next discrepancy id: C68.

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=30/30 controls=263/263 api-calls=14 api-statuses=14 api-rows=38 pinned-defines=90 cc=AppleClang c-presets=2 fault-runs=140 ... faults=140/140 ... listings=1 listing-words=22 manifests=3 ... tests-expected=2462 tests-executed=2462 tests-skipped=0 tests-missing=0 docs=13 ...`
- `dash tools/gate.sh`: the same line, `gate: PASS`.
- `sh tools/gate-linux.sh` (not required by this plan; run for the new cmake and shell code on cmake 3.28 and dash; Ubuntu 24.04 container, x86_64, gcc 13.3.0, cmake 3.28.3): `gate: PASS steps=30/30 controls=263/263 ... cc=GNU ... faults=140/140 ... listings=1 listing-words=22 ... tests-expected=2462 tests-executed=2462 tests-skipped=0 tests-missing=0`, `gate-time total=157`, `gate-linux: exit 0`.

## Threat Flags

None beyond the plan's threat model:
- T-04-43 mitigated: the `listing` step with six grammar controls.
- T-04-44 mitigated: `bootstub-size`, `bootstub-row`, `bootstub-vector`, one control each, and the real stub passes them.
- T-04-45 mitigated: `pack.words-*` compares two independently written parsers (C in `tools/pack`, cmake in `tools/check`), never the core.
- T-04-SC: no package installs.

## Self-Check: PASSED

- Files present: tools/check/listing.cmake, tools/check/steps/listing.sh, data/expected/listings.ids, data/expected/bootstub-rows.ids, data/expected/gate-steps/listing.ids, data/expected/controls/listing.ids, tests/pack-tap.sh, data/expected/tests/pack.ids.
- Commits reachable from HEAD: 13c1258, 51d77c9, 67ffc58 (`git rev-list --count 731c9c2..HEAD` = 3 before the docs commit).
