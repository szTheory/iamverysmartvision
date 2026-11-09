---
phase: 01-evidence-profile-and-rights-foundations
plan: 10
subsystem: cp1610-second-reader
tags: [cp1610, second-reader, blind-read, gate-step, c17, fnd-06]
requires:
  - phase: 01-evidence-profile-and-rights-foundations
    provides: "opcode family table (01-03), discrepancy log with C17 (01-04), cp1610 gate step and row inventory (01-09)"
provides:
  - "record kind second-reader: schema, hook and id parity with data/expected/cp1610-rows.ids for data/cp1610/second-reader.json"
  - "gate step cp1610-reader and three negative controls; notes section Second-reader pass"
  - "independent blind readings of all 74 family rows and of the C17 stack pages (pdf 24, 46, 62)"
affects: [01-12 publication gate, Phase 2 CP-1610 vectors]
actuals:
  tokens: 8900
  tasks: 3
  commits: 1
plan_head_before: 5c5b98fb5b79f6be8d847876eb4a9e57abe1304a
plan_head_after: 4f8d9e591bd3768c783cba645e0ed5ce368ce16f
tech-stack:
  added: []
  patterns:
    - "Kind hook reads the whole document through REC_DOC for top-level keys (c17_reading) and counts verdicts with gate_incr"
    - "Throwaway scratch scripts and blind extractions stay in the session scratchpad; only verdicts are committed"
key-files:
  created:
    - data/cp1610/second-reader.json
    - tools/check/schema/second-reader.txt
    - tools/check/kinds/second-reader.cmake
    - tools/check/steps/cp1610-reader.sh
    - data/expected/gate-steps/cp1610-reader.ids
    - data/expected/controls/cp1610-reader.ids
    - tools/check/controls/_good/tree/data/cp1610/second-reader.json
    - tools/check/controls/cp1610-reader-missing-id/
    - tools/check/controls/cp1610-reader-resolved-without-note/
    - tools/check/controls/cp1610-reader-missing-key/
  modified:
    - docs/cp1610/reference-notes.md
key-decisions:
  - "The pdf form and the c17_reading form are checked in the hook (the DSL cannot hold a space in a regex); tokens second-reader-format and second-reader-c17-format"
  - "Page-internal differences between a format figure and an instruction table are recorded as resolved entries with the winning page, and the table is left unchanged"
patterns-established:
  - "A verdict file shares the table's id inventory so a dropped or extra row fails the gate"
requirements-completed: [FND-06]
duration: about 40 min of tool time
completed: 2026-11-09
status: complete
---

# Phase 1 Plan 10: Blind second-reader pass over the CP-1610 family table Summary

**An independent blind read of every family row and of the C17 stack pages agreed with the table on 71 rows and resolved three page-internal differences without changing any table row; the verdicts are committed with id parity enforced by a new gate step.**

## Performance

- Duration: about 40 min of tool time.
- Gate: `sh tools/gate.sh` and `dash tools/gate.sh` both end `gate: PASS steps=16/16 controls=84/84`; the decode coverage stays `unclaimed=0 ambiguous=0 coincidences=1`; the new step prints `-- cp1610-reader pass agree=71 resolved=3`.

## Accomplishments

- Task 1 (tracer): pdf 53 was rendered at 110 and 150 dpi, read blind into a scratch file, then diffed by a throwaway script.
- Task 2: pdf 50, 51, 52, 54, 55, 56, 57, 58 and the stack pages 24, 46 and 62 were read the same way, each saved before the table or notes were opened for that page.
- Task 3: `data/cp1610/second-reader.json` (74 entries in table order), kind `second-reader`, step `cp1610-reader`, three controls, notes section.

## Blind-read protocol and record

The first reader's table, the notes, the discrepancy log and every earlier SUMMARY were not opened before the matching blind extraction was saved.
Only row ids and the table's key names (not values) were read before the first diff, to write the comparison script.
Independence is procedural: both readers are the same model family.

| Page | Rows or items read | Agreements | Resolutions |
| ---- | ------------------ | ---------- | ----------- |
| pdf 50 | 6 encodings and the shift mode legend | 6 | 0 |
| pdf 51 | 14 encodings | 12 | 2 (GSWD, RSWD printed form) |
| pdf 52 | 1 (J, three decles) | 1 | 0 |
| pdf 53 | 22 | 22 | 0 |
| pdf 54 | 17 | 17 | 0 |
| pdf 55 | 16 | 16 | 0 |
| pdf 56 | 11 | 11 | 0 |
| pdf 57 | 14 entries (7 immediate, SDBD, 6 SDBD indirect) | 13 | 1 (XOR@ printed form) |
| pdf 58 | 6 SDBD immediate forms | 6 | 0 |
| pdf 24, 46, 62 | stack order and direction | agree | 0 |

- Pages blind-read: 12 (9 table pages and 3 stack pages).
- All 74 ids in `data/expected/cp1610-rows.ids` were found on at least one page; none was missing.
- Raw mechanical disagreements were format noise only (spacing, how the table collapses identical N=0 and N=1 status bits, how modifiers are phrased, the `supported: <cycles>` SDBD form); after normalizing, three real differences remained.
- Disagreements: 3.
- First-reader rows changed: 0.

### Resolved entries (all re-read from 200 dpi crops, table unchanged)

1. GSWD, field `printed`, pdf 56: the format figure on pdf 51 shows the generic field DDD; the instruction table on pdf 56 shows 0DD. Winner 0DD (instruction table; the pdf 51 footnote limits true GSWD to R0-R3). Table already held 0DD.
2. RSWD, field `printed`, pdf 56: pdf 51 figure shows DDD; the pdf 56 instruction table shows SSS. Winner SSS. Table already held SSS.
3. XOR@, field `printed`, pdf 55: pdf 55 prints 1111 MMM DDD; the double byte section on pdf 57 prints 111 MMM DDD (one digit short). Winner 1111. Table already held 1111.

Also observed and not a table field: the ADD@ SDBD prefix on pdf 57 is printed with a three-digit first group; recorded in the notes.

### C17 stack pages

Result line recorded in `c17_reading`: `agree: pdf 24, 46, 62`.
The blind reading was: push writes at R6 then increments R6; pull decrements R6 then reads; the stack grows from low to high addresses; the interrupt entry stores the PC at R6 then increments.
It matched the Stack operations section and `C17 status: resolved-primary`, so no change was made to the notes or to the C17 log entry (no D-21 deviation).

## Task Commits

Tasks 1 and 2 changed no tracked file (no table, notes or log correction was needed), so they have no commit; the evidence for them is the verdict file and this record.

1. Task 3: `4f8d9e5` feat(01-10): blind second-reader verdicts, cp1610-reader gate step and controls.

## Files Created or Modified

See `key-files` above; all repo-relative.
The refs cache `refs/SRC-gi-cp1600-um-1975.pdf` was present with the expected digest (the download fallback was not used).
Rendered pages live under gitignored `refs/pages/`; scratch extractions and the diff script stayed in the session scratchpad.

## Deviations from Plan

None - plan executed exactly as written, with these notes:

- Tasks 1 and 2 produced no commit because no first-reader row needed correction.
- The step prints the standard kind line plus `-- cp1610-reader pass agree=<n> resolved=<n>`; the acceptance regex matches the kind line (`-- second-reader pass records=74 agree=71 resolved=3`).

## Known Stubs

None.

## Threat Flags

None.

## Issues Encountered

- A first shell invocation of a step without `ROOT` failed as expected; steps need `ROOT` and `CHECK_DIR`.

## Residual Risk

Agreement between two readings from the same model family is not proof that a cite matches the page; common-mode misreads remain possible.
The decode check covers encodings only; cycles, status bits and interruptibility rest on the two readings.
This risk is recorded for agent review, not owner UAT (FND-06 probe item).

## Self-Check: PASSED

- FOUND: data/cp1610/second-reader.json, tools/check/schema/second-reader.txt, tools/check/kinds/second-reader.cmake, tools/check/steps/cp1610-reader.sh
- FOUND: commit 4f8d9e5 is an ancestor of HEAD
- Gate under sh and dash: PASS; controls=84/84
