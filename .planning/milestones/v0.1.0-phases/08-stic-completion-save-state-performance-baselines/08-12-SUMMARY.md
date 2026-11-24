---
phase: 08-stic-completion-save-state-performance-baselines
plan: 12
subsystem: Phase 8 wave B, plan 4 - the IVSN v1 state reader - the instance-free container reader, the public ivs_snapshot_describe, the 14 reasons 58 to 71, the refusal matrix written from the specification, the heap-exact sweep, the report footprint rules, the reasons-covered check, the matrix self-control and two named faults
tags: [B, STA-01, reader, describe, ivsn-v1, reasons-58-71, refusal-matrix, heap-exact, D-13, D-14, D-15, D-16, D-44, T-08-30]

requires:
  - phase: 08-stic-completion-save-state-performance-baselines
    provides: "plan 08-11 end state (the writer, ivs_state.h with size and save, the 360-byte report struct, the literals, the state suite of 9 ids, 229 faults); counts at plan start steps=38/38 controls=378/378 faults=229/229 tests-expected=4808 tests-executed=4804 tests-skipped=4 api-calls=17 api-rows=47 pinned-defines=96 public-symbols=17 (re-measured with a full gate run at the start, identical)"
provides:
  - "core/src/machine/machine_snapshot_read.c: ivs_snapshot_read (the validated view of the header and the section table, no instance, no pointer kept past the call), ivs_snapshot_arg_reason (the argument classes 58 and 59) and the public ivs_snapshot_describe"
  - "core/include/ivs/ivs_state.h: ivs_snapshot_describe and the 14 reasons IVS_REASON_SNAP_BUF_NULL 58 to IVS_REASON_SNAP_PROFILE_ID_FORM 71, each commented with its status, pinned in data/expected/api-reasons.ids; the call record in data/api/api-effects.json (5 refusal rows)"
  - "core/src/machine/machine.h: the IVSN v1 layout constants (the IVS_SNAP_OFF_* offsets, the section count, the entry and slot sizes, the header and trailer sizes, the domain table ivs_snap_domain) moved from machine_snapshot.c without changing a byte, ivs_snap_view, ivs_snapshot_section_derived_length and ivs_snapshot_derived_total (the lengths and the total derived from the field table with no instance)"
  - "tests/state/state_read.c: the matrix (388 table rows in six families and 18 rows of the argument classes written as code), 11 ids state.read.* and the self-control state.judge.matrix-expectation-corrupt; faults SNAPSHOT_TRAILER_UNCHECKED and SNAPSHOT_LENGTH_TRUSTED; the as-built appendix of docs/formats/ivsn-v1.md"
affects: [08-13, 08-14, 08-15, 08-16, 08-17]

actuals:
  tokens: 31343
  tasks: 2
  commits: 2
plan_head_before: 8e85d93d78d845352a084da265e9dedbe7303ec3
plan_head_after: 713c5d83e67c83d6eb0a2cf79e980547c0ea0dc7

key-decisions:
  - "The reader follows the specification step by step, and where the plan prose reads differently the specification wins and the difference is recorded below: the form of the profile id (reason 71) is judged after the trailer (step 6 follows step 5), and the lengths of the sections are not compared with the lengths derived from the table of the build by the reader (that is restore, step 8, reason 80), so reason 80 stays reachable; the total is tied to the derived size (reason 67) and the sections must be adjacent and end at the trailer start"
  - "The report contract is the one of the existing refusal reports (the load report): a refusal writes only reason, byte_offset, section_index and the header facts the supplied length let it read (format_version from 12 bytes, total_bytes from 16, schema_version from 24), keeps struct_size and every other member; a success writes the 360 bytes in full and keeps the tail of a larger caller struct; an unusable report writes nothing. The test holds the whole 400-byte object to it with two poison bytes on every row"
  - "The check order inside the section table is entry by entry (domain, three zero bytes, offset against the end of the entry before, length against the end of the payload); the section index of a defect is that entry and the byte offset the field of that entry; the last section that does not end at the trailer start is reported at the last entry, length field. The domain test is the exact sequence of the table of the build, which is the same as known, strictly ascending and complete for seven entries"
  - "A length above the bound counts as the bound for the report-overlap rule (the first 16,384 bytes at most), so that an absurd length is OVERSIZED and not an overlap; an empty and an unregistered profile id of good form are accepted by describe because only restore consults the registry"
  - "Fault sites: SNAPSHOT_LENGTH_TRUSTED compiles out the refusal of a total above the supplied length (the trailer is then read at the claim) and SNAPSHOT_TRAILER_UNCHECKED forces the trailer comparison result to equal; one site each in machine_snapshot_read.c. The test buffers are exact heap copies in the normal builds and carry a zeroed tail only in a fault build (IVS_STATE_FAULT_RUN), so the dev sanitizer finds any over-read and a fault build reads defined bytes"

key-files:
  created:
    - core/src/machine/machine_snapshot_read.c
    - tests/state/state_read.c
    - .planning/phases/08-stic-completion-save-state-performance-baselines/08-12-SUMMARY.md
  modified:
    - core/src/machine/machine.h
    - core/src/machine/machine_snapshot.c
    - core/include/ivs/ivs_state.h
    - core/CMakeLists.txt
    - tests/CMakeLists.txt
    - tests/state/state_cases.h
    - tests/state/state_main.c
    - data/expected/tests/state.ids
    - data/expected/faults/state.ids
    - data/expected/api-reasons.ids
    - data/api/api-effects.json
    - docs/formats/ivsn-v1.md
    - .planning/STATE.md
    - .planning/ROADMAP.md

requirements-completed: []

completed: 2026-11-14
---

# Phase 8 Plan 12: the IVSN v1 state reader Summary

**A snapshot can now be asked what it claims to be without an instance. `ivs_snapshot_describe(buf, len, rep)` checks the container in the order of the specification up to the SHA-256 trailer and the form of the profile id (the length bound before any byte is read, the magic, the format and schema versions and the schema digest of the build, the total against the length and the size this build derives, the section count, the section table entry by entry, the trailer, the id form), refuses with one of the 14 new reasons 58 to 71 and the five statuses BAD_ARG, OVERSIZED, MALFORMED, UNSUPPORTED and DIGEST_MISMATCH, and reports on a refusal what it could read (the two versions, the total, the byte offset and the section index of the defect) and on success the profile id, the parameter digest, the composite digest and the three slot identities. The reader is a separate file (`machine_snapshot_read.c`) that shares the layout constants with the writer (moved into `machine.h` without changing a byte: the writer's output is still the 4,299 bytes of the literals) and derives the section lengths and the total from the field table alone. The matrix that holds it is written from the specification, not from the reader: 406 rows (every one of the 14 reasons is produced by at least 5 rows), each run twice into a poisoned 400-byte report that must equal the report the contract allows byte for byte, a sweep of every length from 0 to the total plus one from a heap buffer of exactly that length under the dev sanitizer, a check that every reason is expected and produced, and a self-control that corrupts one expectation of each family and requires the runner to report exactly that row. Two named faults fail exactly their ids. The gates pass under sh, dash and the Ubuntu container with the plan-end counts `steps=38/38 controls=378/378 faults=231/231 fault-runs=231 tests-expected=4832 tests-executed=4828 tests-skipped=4 tests-missing=0 api-calls=18 api-rows=52 api-statuses=14 pinned-defines=110 public-symbols=18 hdr-alone=10/10 prefix-files=11 pkg-selftest=17/17 consumer-lines=135`. STA-01 and STA-02 stay Pending until restore and the hard-point suite exist.**

## Commits

| Commit | Task | What |
|--------|------|------|
| `785f93b` | 1 (tracer) | the layout constants and the domain table moved into `machine.h`, `ivs_snapshot_read`, `ivs_snapshot_arg_reason`, `ivs_snapshot_describe`, the 14 reasons and the call record in the same commit, ids describe-ok, -refuse-args, -refuse-length, -refuse-header, -refuse-trailer, faults SNAPSHOT_TRAILER_UNCHECKED and SNAPSHOT_LENGTH_TRUSTED (ids 4808 to 4818, faults 229 to 231, api-calls 17 to 18, api-rows 47 to 52, pinned-defines 96 to 110, public-symbols 17 to 18) |
| `713c5d8` | 2 | ids -refuse-section-table, -profile-id-forms, -heap-exact, -writes-only-report, -report-v1-size-caller, reasons-covered and the judge, the precedence rows and test lists of the call record, the as-built appendix of the specification (additions only: 0 removed lines) (ids 4818 to 4832) |
| the commit that adds this file | | this SUMMARY, STATE.md and ROADMAP.md |

## The first move: the layout constants

The open item of plan 08-11 was done first and changes no byte. `IVS_SNAP_OFF_*`, `IVS_SNAP_SECTIONS`, `IVS_SNAP_ENTRY_BYTES`, `IVS_SNAP_SLOT_BYTES`, `IVS_SNAP_HEADER_BYTES`, `IVS_SNAP_TRAILER_BYTES`, the six `_Static_assert`s that chain the offsets and the domain table now live in `core/src/machine/machine.h` (the domain table as `extern const uint8_t ivs_snap_domain[7]`, defined once in `machine_snapshot.c`); the `ivs_snapshot_report` layout asserts stay in `machine_snapshot.c` next to the writer. The state literals (`state.literal.header-structure`, `state.literal.schema-digest`, `state.size.derived-and-constant`) and the stepped-snapshot literal `087a15d3...5b6b` still pass in the dev and release presets and the container, so the writer's bytes are unchanged. Two small shared functions were added next to them, both instance-free: `ivs_snapshot_section_derived_length(i)` and `ivs_snapshot_derived_total()`, which derive the lengths from the field table (7 bytes of frame plus the width times the count of each STATE row, plus 4 bytes of count for a row with more than one element); `state.read.describe-ok` holds them to the section-bytes literals, to the writer's own counting pass on a live instance and to the sum 4,299. The save cost (about 0.84 ms because the composite digest hashes MEM twice) was not touched, as the open item said (optional); describe measured at about 37 microseconds in a release build (the schema digest and the trailer hash).

## The reader and its check order

| Step (specification) | Check | Reason, status | Byte offset reported |
|---|---|---|---|
| arguments | the report: NULL, struct_size below 360 or above 4096 | BAD_ARG, nothing written | n/a |
| arguments | NULL buffer with a length above 0; then the report overlapping the first min(len, 16,384) bytes of the buffer | 58, 59 BAD_ARG | 0 |
| 2 | length above 16,384, before any byte is read | 60 OVERSIZED | 16,384 |
| 2 | length below 644 | 61 MALFORMED | the length |
| 3 | magic | 62 MALFORMED | the first differing byte |
| 3 | format version, flags, schema version, schema digest | 63, 64, 65, 66 UNSUPPORTED | 8, 10, 16, 24 |
| 3 | the total above the length; below it or not the derived size | 61; 67 MALFORMED | the length; 12 |
| 3 | section count | 68 MALFORMED | 20 |
| 4 | section table, entry by entry | 69 MALFORMED | the field of the entry; the section index is the entry |
| 5 | SHA-256 trailer | 70 DIGEST_MISMATCH | the trailer start (total minus 32) |
| 6 (form) | the profile id | 71 MALFORMED | the first offending byte, or 119 when there is no NUL |

The reader reads every multi-byte field from single bytes, compares every offset and length with the supplied length before the byte it names is read (all sums are 64-bit), refuses a length above the bound before it looks at the buffer, and hashes `total - 32` bytes with the in-tree SHA-256. `core-imports` stays 7 (5 in the gcc container, as at plan start); the new symbols are `ivs_snapshot_describe` (public) and five internal ones (`defined=129` to `135`).

## The matrix (rows per reason, lengths swept)

Rows per reason, counting a call as a row: 58: 7, 59: 5, 60: 11, 61: 36, 62: 13, 63: 7, 64: 18, 65: 12, 66: 35, 67: 14, 68: 12, 69: 92, 70: 95, 71: 24, that is 381 refusal rows, and 25 rows that are accepted (the snapshot as written, the informational fields changed with a valid trailer, an empty id, an unregistered id, a 63-byte id and the printable edge characters, two pairs of chained section lengths, and the two argument rows that place the report just beside the buffer): 406 rows in all. The six table families hold 388 rows (365 refusals and 23 accepted): the header with the order among its fields 97, the length and the total the snapshot states 54, the trailer after the structure 100, the section table 93, the profile id 36 and the accepted rows 8. The argument rows are written as code, 18 calls: the NULL buffer with 7 lengths, a zero length with a NULL and with a real buffer, and 9 object rows that place the report against the buffer (at its start, inside it, across its end, the buffer starting on the last report byte, right after the report, the report right after the buffer, and three rows with a length of 16,384 and above); the unusable report is tried with 12 struct_size values in 3 call shapes (nothing written).

Lengths swept: `state.read.describe-heap-exact` describes every length from 0 to 4,300 (the total plus one), 4,301 lengths, from a heap buffer of exactly that many bytes; the status and the reason by class are pinned (below 644 and up to 4,298: truncated with the length as the offset; 4,299: accepted; 4,300: the total-length reason). A fault run takes every 16th length and always the ones around 644 and the last four. `state.read.describe-refuse-length` adds lengths 0, 1, 7, 8, 11, 12, 15, 16, 23, 24, 100, 303, 304, 611, 612, 643, 644, 645, 700, 2,000, 4,000, 4,266, 4,267 and 4,298, ten lengths above the bound claimed for a one-byte real buffer (16,385 to 2^64 minus 1, including 2^32, 2^32 plus 4,299 and 2^63), and four cut snapshots with a damaged header (the length class comes first). `state.read.describe-report-v1-size-caller` sweeps every struct_size from 0 to 359 and eight values above the maximum.

What every row checks: the status, the whole report against the contract with two poison bytes (so a byte written that the contract does not list, or a listed byte not written, fails), the bytes of the buffer unchanged, and, for a refusal, that the instance is not involved at all (there is none). `state.read.describe-writes-only-report` repeats the footprint with an independent classifier over every row of every family (which bytes of a 400-byte object a call wrote, by two poison bytes): reason, byte offset and section index, plus the facts the length allowed (offsets 8 to 11 from 12 bytes, 16 to 23 from 16, 12 to 15 from 24), nothing else; a success writes exactly the 360 bytes.

## The two faults (each fails exactly the id named for it, nothing outside the prefix, the must-pass id stays green)

| Fault | Site | Must-fail id | What the id shows | Failing in the fault run (all with the prefix `state.`) | Must-pass id |
|---|---|---|---|---|---|
| SNAPSHOT_TRAILER_UNCHECKED | `machine_snapshot_read.c`, the trailer comparison result | `state.read.describe-refuse-trailer` | a damaged trailer, a flipped trailer byte and a byte that only the trailer covers are accepted instead of reason 70 | describe-refuse-trailer, describe-writes-only-report, describe-report-v1-size-caller, reasons-covered, matrix-expectation-corrupt | `state.read.describe-ok` |
| SNAPSHOT_LENGTH_TRUSTED | `machine_snapshot_read.c`, the refusal of a total above the supplied length | `state.read.describe-refuse-length` | a truncated snapshot is reported with another reason (the trailer is read at the claim, over the zeroed tail of the buffer) | describe-refuse-length, describe-refuse-header, describe-heap-exact, reasons-covered, matrix-expectation-corrupt | `state.read.describe-ok` |

Each id occurs once in `core/src`, in `machine_snapshot_read.c`; a fault run of the whole suite takes about 0.1 s.

## What each new test can fail on (mutation probes, run in the working tree and reverted)

| Probe on `machine_snapshot_read.c` | Result |
|---|---|
| the magic offset reported one off | describe-refuse-header |
| the length bound one above (16,385 accepted) | AddressSanitizer abort (heap overflow on the one-byte buffer) |
| the minimum length one below (643 accepted) | describe-refuse-length (the four cut snapshots with a damaged header) |
| the flags test loosened; the format version and schema version tested on a part of their bytes | describe-refuse-header |
| the schema digest compared over 31 bytes; the section count test loosened | describe-refuse-header (and trailer) |
| the total below the length accepted; the derived size not compared | describe-refuse-length |
| the header facts read from 10 bytes, from 12 bytes for the total, from 20 bytes for the schema version | AddressSanitizer abort on the cut buffers |
| the trailer compared over 31 bytes; the trailer hashed over one byte less | describe-refuse-trailer; describe-ok and the argument id |
| the section fit test in 32-bit arithmetic; a zero byte of the table not tested; the offset compared with less-than; the domain compared with less-than; the end of the last section not tested | describe-refuse-section-table (and the footprint and coverage ids) |
| the id: a byte after the NUL not tested; the printable bound one above or below; the no-NUL offset one off | describe-profile-id-forms |
| struct_size set on a refusal; a member written on a refusal | the footprint of every row of every family |
| the success write one byte short; one write of the caller's struct_size; the struct_size bound removed or loosened | eight ids; AddressSanitizer abort; describe-refuse-args and describe-report-v1-size-caller |
| the overlap judged over the unclamped length; the NULL-buffer test limited | describe-refuse-args |
| the slot kind off by one; the byte offset of OVERSIZED changed; the section index not reported | describe-ok and the argument id; describe-refuse-length; the trailer, table and id families |

Two probes survived and are equivalent mutants, not gaps: copying 63 instead of 64 bytes of the profile id into the report (an accepted id has a NUL at or before byte 63, and everything after it is zero), and comparing the end of the last section with `<` instead of `!=` (it can never exceed the payload end after the per-entry fit test).

## Deviations from Plan

**1. [Interpretation - plan prose against the specification] The profile id form comes after the trailer.** The prose of the plan lists "the profile id form, then the SHA-256 trailer". The specification (Check order, step 5 trailer, step 6 profile, and the numbering 70 before 71) puts the trailer first, and the same reader serves restore, whose order is the specification's. The code follows the specification; a bad id with a stale trailer is reason 70; two rows pin it (`profile id with a control byte, trailer stale`, `profile id without a NUL, trailer stale`), and the as-built appendix says so.

**2. [Interpretation - plan prose against the specification and plan 08-13] The reader does not compare each section length with the derived length.** The plan prose lists "lengths equal to the derived lengths" in the table check. The specification puts that comparison in step 8 (reason 80) and plan 08-13 tests reason 80 with a "section table consistent" row, which can only exist if the reader leaves the comparison to restore. The reader checks everything else the plan lists (the seven domains as the exact ascending sequence, the zero bytes, the first offset 612, adjacency, the last end 32 bytes before the total) plus the total against the derived size. A length one more or one less than derived changes adjacency or the last end and is reason 69 in every row (the rows the plan names); a chained pair that keeps the sum is accepted by describe (two rows) and left to restore. Recorded in the appendix and in the open items.

**2a. [Interpretation] The section index and byte offset conventions of a table defect.** The specification does not say which entry or which offset; the convention (entry by entry, the failing field, the last entry's length field for the end of the last section) is fixed in the header comment of `machine_snapshot_read.c` and the appendix and held by 92 rows.

**3. [Interpretation] What a refusal writes into the report.** The plan says a refusal "writes only the report" and the test "changes only the fields the contract lists". Following the load report ("only the detail fields are written"), a refusal writes reason, byte_offset, section_index and the readable header facts and keeps every other member and struct_size; the contract text is in `ivs_state.h`, the call record and the appendix. A success writes the 360 bytes and keeps a larger caller's tail.

**4. [Interpretation] Where the zeroed guard bytes are.** The plan says the length buffers carry zeroed guard bytes after the supplied length "so the fault build reads defined bytes". The normal builds keep every input an exact heap copy (so the dev sanitizer finds an over-read in this id too) and the zeroed tail of 4,400 bytes is added only when `IVS_STATE_FAULT_RUN` is defined (the fault executables), which is exactly where a reader that trusts a claimed length reads past its input. The same rule serves the heap-exact sweep.

**5. [Interpretation] The overlap rule for a length above the bound** counts the bound, not the length, so a length such as 2^64 minus 1 is OVERSIZED and never "a report overlapping the buffer" (the argument classes still come before the length class, as the specification orders them).

**6. [Information] Empty and unregistered ids are accepted** (the plan said "reason 71 or acceptance by the specification's rule"; the rule lists three defects and an empty id has none). **The slot records of a success** carry struct_size of an image report (as every other image report of the core) and 0 in the members the snapshot does not hold (length_units, unit_bits, base_addr, reserved).

**7. [Information] The container gate of Task 1 ran on the tree before the test file reached its final form** (the file was refactored and four rows were added after that run: `r_row_make`, the quiet flag and counters, four cut-with-damaged-header rows). The container gate was run again on the Task 2 tree, which contains the final form of everything in the Task 1 commit (see the final gate), and sh and dash were run on the exact Task 1 tree.

**8. [Information] The closing docs commit corrects one line of the as-built appendix** (the row count: 381 refusal rows and 25 accepted rows, 406 in all; the first text of the appendix counted 404 because it left out the two accepted argument rows). It is the only removed line of `docs/formats/ivsn-v1.md` since the plan head, inside the section this plan appended; the oracle sections are untouched.

No auth gates, no package installs (T-08-SC), no `gh` call, no push, no tag, no `tools/ci-record.sh`; `.planning/config.json` and `.planning/state.json` were never staged. No oracle data changed: `data/state/schema.literal.txt`, `data/expected/state-checklist.ids` and `data/expected/ivsn-literals.ids` are untouched and `docs/formats/ivsn-v1.md` has only additions (an appended section, 0 removed lines).

## Decisions Made

- The reader is a pure function of its bytes with a view that holds no pointer: restore (plan 08-13) calls it and adds the identity, image and content layers on top, exactly as `machine_load.c` sits on `ivsc.c`.
- Every refusal has a status that follows from its reason by the table of the specification, so a row states only the reason; the status is a function in the test written from the specification table.
- The expected report of every row is built from the supplied bytes with the layout of the specification and the offsets of the public struct, never from the reader, and compared as a whole object with two poison bytes.

## Cost (laptop)

| Item | Value |
|------|-------|
| estimate of the plan | 120,000 tokens (raw, confidence low), 2 tasks |
| actuals | 31,343 tokens (chars/4 over the added lines of the two task commits outside `.planning/`), 2 tasks, 2 commits (measured from the ledger of the plan head) |
| gate total, sh, at plan start | 105 s |
| gate total, sh and dash, Task 1 tree | 107 s and 107 s, identical summary line (md5 c9769351e036ba151d6e3e0bd2b425c7) |
| gate total, sh and dash, Task 2 tree | 122 s and 113 s, identical summary line (md5 c501c29c4de3ec1e5a6b71a3e292e5a8) |
| container gate, Task 1 tree (earlier form of the test file) | 432 s (gcc 13.3.0, cmake 3.28.3, x86_64 emulated), exit 0, 0 FAIL lines |
| container gate, Task 2 tree | 462 s, exit 0, 0 FAIL lines |
| gate total, sh and dash, closing tree | 108 s and 102 s, identical summary line (md5 c3315d4eb8b6bf80a2fed3265cbbe8b0); container 432 s, exit 0, 0 FAIL lines |
| suite `state` (21 ids) | dev preset (sanitized, -O1) 0.77 s, release 0.36 s (before the plan: 0.17 s and 0.06 s); a fault run 0.1 s |
| `ivs_snapshot_describe` | about 37 microseconds per call for a 4,299-byte snapshot in a release build on a loaded laptop (the schema digest over 88 lines and the SHA-256 trailer) |

The heap-exact sweep is most of the added time (4,301 lengths, two calls each: about 0.3 s release, 0.6 s dev); a fault run takes every 16th length. The hosted cost is a few tenths of a CPU second per preset plus the build of one more core TU and one larger test file; the gate total moved within measurement noise (105 s at the start, 107 s to 122 s at the ends).

## Open Items

- **Plan 08-13 (restore):** call `ivs_snapshot_read` (it fills `ivs_snap_view`: header, the three slot records, the composite and parameter digests, the seven section entries with offset, length and digest, the profile id and its length) after its own argument classes (`ivs_snapshot_arg_reason` gives 58 and 59; reason 81, the buffer or report overlapping the instance, is restore's) and the handle check; the reader stops after the id form (step 6a), so restore continues with the registry (72), the instance's id (73), the parameter digest (74), the slots (75, 76) and the content. **The reader does not compare section lengths with the derived lengths**: restore compares each `view.section[i].length` with `ivs_snapshot_section_derived_length(i)` (reason 80); a pair of changes that chain and keep the sum reaches it (two rows of the describe matrix show the reader accepts it). Restore's report should follow the same refusal contract (reason, byte_offset, section_index, facts; the kind for a slot reason), and `IVS_E_STATE_MISMATCH` (-24) and the reasons 72 to 81 are added there as planned (the status count stays 14 until then).
- **Plans 08-13 to 08-15 (tests):** a buffer a reader may be given must be an exact heap copy in the normal builds and carry a zeroed tail under `IVS_STATE_FAULT_RUN` (the idiom of `r_heap_copy` in `tests/state/state_read.c`), otherwise the fault `SNAPSHOT_LENGTH_TRUSTED` reads past the input and a fault run crashes instead of failing an id. The row machinery (`r_row`, the two-poison `r_call_check`, `r_expect_report`) is static in `state_read.c`; move it to `state_util.c` if restore or the fuzz family needs it.
- **Plan 08-15 (fuzz):** `ivs_snapshot_describe` is fuzzable without an instance (a libFuzzer target needs only `machine_snapshot_read.c`, `machine_snapshot.c`'s schema functions, the memory-system and STIC table accessors the field table touches, and the SHA-256 TU); the matrix of this plan already pins every truncation length and every field of the header and the table.
- **Plan 08-16/08-17:** the installed consumer does not call `ivs_snapshot_describe` yet; the call record and the symbol are in the inventories (`api-calls=18`, `public-symbols=18`), the consumer golden and its calls list are unchanged.
- **Information.** D10 and D11 are not performed; no claim above provisional exists; no hosted-only check was run (the hosted-style container gate ran green); the snapshot is exact-build state and describe says what a snapshot claims, never that it restores; STA-01 and STA-02 stay Pending until restore and the hard-point suite exist; REL-01 stays Pending (D-28).

## Interfaces for the next plans

- Public, `ivs/ivs_state.h`: `ivs_snapshot_describe(buf, len, rep)` and the reasons `IVS_REASON_SNAP_BUF_NULL` (58) to `IVS_REASON_SNAP_PROFILE_ID_FORM` (71).
- Internal, `machine/machine.h`: `ivs_snap_view` (and `ivs_snap_slot_view`, `ivs_snap_section_view`, `IVS_SNAP_KNOWN_FORMAT`, `_TOTAL`, `_SCHEMA`), `ivs_snapshot_read(buf, len, view)`, `ivs_snapshot_arg_reason(buf, len, rep, rep_bytes)`, `ivs_snapshot_section_derived_length(i)`, `ivs_snapshot_derived_total()`, `ivs_snap_domain[7]` and the `IVS_SNAP_*` layout constants.
- Tests: `r_call_check`, `r_expect_report`, `r_heap_copy`, the row builders and `r_family` in `tests/state/state_read.c` (static); `IVS_STATE_TEST_SOURCES` now holds `state/state_read.c`.
- Gate counts after this plan `steps=38/38 controls=378/378 faults=231/231 fault-runs=231 tests-expected=4832 tests-executed=4828 tests-skipped=4 tests-missing=0 api-calls=18 api-rows=52 api-statuses=14 pinned-defines=110 public-symbols=18 hdr-alone=10/10 prefix-files=11 pkg-selftest=17/17 consumer-lines=135 core-imports=7 state-rows=88 state-cuts=18`; `data/state/tests-pending.ids` still holds six lines (the cut ids of plan 08-14, the restore ids of plan 08-13).
- Threat id T-08-30 is used here (T-08-SC throughout); the next state plans continue from T-08-31 or the numbering their plans assign.

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=38/38 controls=378/378 ... faults=231/231 ... fault-runs=231 ... tests-expected=4832 tests-executed=4828 tests-skipped=4 tests-missing=0 ... api-calls=18 api-rows=52 api-statuses=14 pinned-defines=110 public-symbols=18 hdr-alone=10/10 prefix-files=11 pkg-selftest=17/17 consumer-lines=135 ... (0 FAIL lines)
dash tools/gate.sh:    the identical summary line (0 FAIL lines)
GATE_LINUX_PLATFORM= sh tools/gate-linux.sh: exit 0, build/gate-linux.log ends gate: PASS (gcc 13.3.0, cmake 3.28.3, x86_64 emulated), the plan-end counts, 0 FAIL lines
```

After the Task 1 tree: sh and dash `faults=231/231 tests-expected=4818 tests-executed=4814 api-calls=18 api-rows=52 pinned-defines=110 public-symbols=18` with the identical summary line (md5 c9769351e036ba151d6e3e0bd2b425c7), container exit 0 (432 s, earlier form of the test file, see deviation 7). After the Task 2 tree: the plan-end counts under sh (122 s) and dash (113 s) with the identical summary line (md5 c501c29c4de3ec1e5a6b71a3e292e5a8), container exit 0 (462 s). Every first gate run of every task passed (no retry, no `FAIL` line at any gate of the plan). On the closing tree (this SUMMARY, STATE.md and ROADMAP.md added or edited and the one-line appendix correction; `tracked=2446`): sh (108 s) and dash (102 s) `gate: PASS` with the identical summary line (md5 c3315d4eb8b6bf80a2fed3265cbbe8b0), 0 `FAIL` lines, the plan-end counts, and the container gate was run again (`GATE_LINUX_PLATFORM= sh tools/gate-linux.sh`): exit 0, `gate: PASS` with the plan-end counts, 0 `FAIL` lines, gate-time total=432 s; `build/gate-linux.log` holds that run. The dev-preset `ctest` suite `state` (21 ids) and the release-preset `state` suite pass; the two new fault executables fail exactly the ids of the table above.

## Known Stubs

None.

## Threat Flags

None new. T-08-30 (snapshot bytes into the reader, every length, offset and count attacker-controlled): the length bound is checked before the buffer is looked at, every offset and length is compared with the supplied length before the byte it names is read, all sums are 64-bit, the structure is checked before the trailer, fields are assembled from single bytes; the sweep of every length from 0 to the total plus one runs from exact heap buffers under the dev sanitizer, the matrix names the status, reason, offset and section index of 406 rows written from the specification, and the faults SNAPSHOT_LENGTH_TRUSTED and SNAPSHOT_TRAILER_UNCHECKED show the two checks can fail; the libFuzzer target and the resealed families are plan 08-15. A describe call costs about 37 microseconds for a 4,299-byte input and reads at most 16,384 bytes, so it is no amplification. T-08-SC: no installs.

## Self-Check: PASSED

Verified before the final commit: the SUMMARY and the created files of the plan exist (FOUND for `machine_snapshot_read.c`, `state_read.c` and the SUMMARY); commits `785f93b` and `713c5d8` are ancestors of HEAD and `git rev-list --count 8e85d93d78d845352a084da265e9dedbe7303ec3..HEAD` printed 2 before this file's commit; nothing is staged but the files of this commit and `.planning/config.json` and `.planning/state.json` are not staged; `git ls-files` lists no `.ivsc`, `.bin`, `.rom`, `.exec`, `.ivsn`, `.wav`, `.ppm` or `.vraw` file; the diff of the two task commits holds no home, scratch or temp path and no "Intellivision"; no commit deletes a file; `grep -c IVS_FAULT_SNAPSHOT_ core/src/machine/machine_snapshot_read.c` prints 2 and `grep -rho IVS_FAULT_SNAPSHOT_ core/src` shows each of the five ids once; `git show 713c5d8 -- docs/formats/ivsn-v1.md` has 0 removed lines; `grep -c '^state[.]read[.]' data/expected/tests/state.ids` prints 11 and `grep -c '^state[.]judge[.]' data/expected/tests/state.ids` prints 2; `grep -c '^IVS_REASON_SNAP_' data/expected/api-reasons.ids` prints 14.
