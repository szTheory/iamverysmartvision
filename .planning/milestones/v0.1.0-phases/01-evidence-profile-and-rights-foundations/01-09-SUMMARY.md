---
phase: 01-evidence-profile-and-rights-foundations
plan: 09
subsystem: cp1610-coverage-gate
tags: [cp1610, coverage, gate-step, claims, c17, fnd-06]
requires:
  - phase: 01-evidence-profile-and-rights-foundations
    provides: "gate and records DSL (01-01), opcode family table (01-03), discrepancy log with C17 (01-04), claim ledger (01-05)"
provides:
  - "record kind cp1610-row: schema, hook and exact 74-id row inventory for data/cp1610/opcode-families.json"
  - "tools/check/coverage.cmake: 1024-pattern decode coverage with declared refines and also_encodes coincidences"
  - "tools/check/c17-consistency.cmake: notes-to-log consistency of the C17 status"
  - "gate step cp1610 and seven negative controls; claim CL-0005; notes section Decode coverage"
affects: [01-10 blind second reader, 01-11 gate latency fallback, Phase 2 coverage inventory]
actuals:
  tokens: 36000
  tasks: 2
  commits: 2
plan_head_before: f39f183aba4b37fc68ff87c67fb97107fc031e1e
plan_head_after: 01719880b9e52a23a2715ad88b49425681a79286
tech-stack:
  added: []
  patterns:
    - "Kind hook for checks the records DSL cannot express (regexes holding a space); same pattern as plan 01-04"
    - "Coverage computed by integer mask/value tests over parallel lists, relations evaluated only for the few rows that pass the mask"
key-files:
  created:
    - tools/check/coverage.cmake
    - tools/check/c17-consistency.cmake
    - tools/check/schema/cp1610-row.txt
    - tools/check/kinds/cp1610-row.cmake
    - tools/check/steps/cp1610.sh
    - data/expected/cp1610-rows.ids
    - data/expected/gate-steps/cp1610.ids
    - data/expected/controls/cp1610.ids
    - tools/check/controls/_good/tree/data/cp1610/opcode-families.json
    - tools/check/controls/_good/tree/data/expected/cp1610-rows.ids
    - tools/check/controls/_good/tree/docs/cp1610/reference-notes.md
  modified:
    - data/claims.json
    - data/expected/claims.ids
    - docs/cp1610/reference-notes.md
    - tools/check/controls/text-no-files/MAKE
key-decisions:
  - "The cite form (`pdf <n>`) and the sdbd form (`supported: <detail>`) are checked in the hook, not by DSL `match` lines, because the DSL splits schema lines on whitespace (plan 01-04 limitation); token `cp1610-row-format` is unchanged"
  - "No cycle value is required on any row, so NOPP's printed 7 (not 7/9) passes; the table was not edited to fit the checker"
  - "coverage.cmake also reports `coverage-refines-outside` when a refining row matches a pattern its target does not; this strengthens the plan, and the real table passes it"
patterns-established:
  - "A good-tree doc added for one step must also be deleted by the text-no-files control MAKE"
requirements-completed: []  # FND-06 also depends on 01-10; left open on purpose
duration: about 25 min of tool time
completed: 2026-11-09
status: complete
---

# Phase 1 Plan 09: CP-1610 family table coverage gate Summary

**The 74-row CP-1610 family table is now a permanently checked record: row shape and page cites, printed/pattern consistency, a 1024-pattern decode coverage of `unclaimed=0 ambiguous=0 coincidences=1`, and a notes-to-log C17 consistency check, all in one gate step that takes about 0.7 s.**

## Performance

- **cp1610 step time (measured, idle-ish host, load average about 16):** 0.69, 0.69 and 0.75 s wall time for `sh tools/check/steps/cp1610.sh`; the records check is about 0.2 s and `coverage.cmake` about 0.45 s. This is under the 2 s bound (D-12.9), so no fallback was needed: the decode stays in `cmake -P` and nothing moves to a Phase 2 C test (I-4 recorded; plan 01-11's parallel/batch fallback is not triggered by this step).
- **Tasks:** 2 of 2 (tracer, then C17 link and remaining controls).

## Accomplishments

- **Row schema and hook.** `tools/check/schema/cp1610-row.txt` has one `req` line (with type) for each of the 17 row keys, the `keys` list, the group, interruptible and pattern regexes, nonblank lines and the inventory `data/expected/cp1610-rows.ids` (74 ids). The hook `tools/check/kinds/cp1610-row.cmake` checks pattern length 10, printed (spaces removed, letters as `.`) equal to pattern, the `pdf <n>` cite, the sdbd form, the `eq` entry form, and that every `refines` and `also_encodes` name an existing row.
- **Coverage.** `tools/check/coverage.cmake` reads the rows once and tests all 1024 patterns. The real table reports `patterns=1024 unclaimed=0 ambiguous=0 coincidences=1` (the coincidence is 0010111111: TSTR, JR, MOVR). Findings are capped at 20 lines plus a `coverage-more` line; the totals stay exact.
- **C17 link.** `tools/check/c17-consistency.cmake` reads the C17 status from `data/discrepancies.json` with `string(JSON)` and the single `C17 status: ` line from the notes. The real log and notes agree (`c17=resolved-primary`).
- **Claim and notes.** CL-0005 (source-expectation, provisional, artifact `data/cp1610/opcode-families.json`, depends on `cpu.part`) and the notes section `## Decode coverage` with `[claim:CL-0005]` on the result line. Gate summary shows `claims=5 provisional=4`.
- **Row count.** The final row count is 74. It is below D-19's "about 90" because SDBD forms are attributes of the memory rows, branch aliases share one row and the jump family is one first-decle row, as plan 01-03 records (I8). Not padded.
- **No table row was changed.** The checker accepted all 74 rows as transcribed in plan 01-03, so there is no list of row fixes with pages to record.

## Task Commits

1. Task 1 (tracer): `61cb7bb` feat(01-09): cp1610 row schema, 1024-pattern coverage check and gate step
2. Task 2: `0171988` feat(01-09): C17 notes-to-log consistency, claim CL-0005 and five cp1610 controls

## Verification (executed)

- Tracer gate (auto mode): after Task 1, `sh tools/gate.sh` printed `gate: PASS steps=14/14 controls=72/72 ... cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1`; then expanded.
- Final: `sh tools/gate.sh` and `dash tools/gate.sh` both end with `gate: PASS steps=14/14 controls=77/77 ci-uses=1 claims=5 provisional=4 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 ...`.
- Controls: all seven cp1610 controls plus `_good/cp1610` report ok; each was also run by hand and fails for its stated reason (`coverage-unclaimed`, `cp1610-row-missing: ALL/cite`, `cp1610-row-printed-mismatch: ZERO`, `coverage-ambiguous: 0000000000`, `cp1610-row-format: ALL/cite`, `cp1610-row-empty`, `cp1610-c17-inconsistent: log=open notes=resolved-primary`).
- Non-vacuity on the real table (scratch copy, outside the repo): removing RSWD gives `cp1610-row-id-missing: RSWD` and eight `coverage-unclaimed` lines (0000111000 to 0000111111); removing the `C17 status:` line gives `cp1610-c17-missing`; the 1022-unclaimed control prints 20 lines plus `coverage-more`.
- No absolute home or scratch path appears in any tracked file written by this plan (`grep` over `tools data docs` printed nothing).

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Cite and sdbd regexes cannot live in the schema DSL**
- **Found during:** Task 1 design
- **Issue:** The plan lists `match cite .*pdf [0-9]+.*` and `match sdbd ^(not-applicable|unsupported|supported: .+)$`; the records DSL splits each schema line on whitespace, so a regex holding a space is truncated (the same limitation plan 01-04 hit).
- **Fix:** Both checks run in `tools/check/kinds/cp1610-row.cmake` with the exact regexes and emit `cp1610-row-format: <id>/cite` and `cp1610-row-format: <id>/sdbd`. Control `cp1610-row-without-cite` proves the cite check fails. No check was weakened.
- **Files modified:** `tools/check/schema/cp1610-row.txt`, `tools/check/kinds/cp1610-row.cmake`
- **Commit:** `61cb7bb`

**2. [Rule 3 - Blocking] Good-tree notes doc broke the `text-no-files` control**
- **Found during:** Task 2 (first gate run reported `control-token-missing: text-no-files`)
- **Issue:** That control expects no scannable docs in the temp tree; the new good-tree `docs/cp1610/reference-notes.md` is one.
- **Fix:** Added `delete docs/cp1610/reference-notes.md` to `tools/check/controls/text-no-files/MAKE`.
- **Commit:** `0171988`

**3. [Rule 3 - Blocking] Section sign in a good-tree doc fails the integrity step**
- **Found during:** Task 2 (`integrity-non-ascii`)
- **Issue:** `tools/**` must be ASCII, and the good-tree doc sits under `tools/check/controls/`, so the plan's D-18 `§` form cannot appear there.
- **Fix:** The good-tree sentence cites `S3.4.6, pdf 62` (record style). The real notes keep `§`.
- **Commit:** `0171988`

### Plan expectations that were corrected

- **NOPP prints 7.** The checker requires no cycle value on any row, so NOPP (`7`, pdf 54) and every `7/9` row pass. The checker spec was fixed, not the row.
- **SDBD pages.** The sdbd check requires the form `supported: <detail>` and does not name a page, so the indirect (pdf 57) and immediate (pdf 58) rows both pass as transcribed.
- **Added check (not in the plan):** `coverage-refines-outside` flags a refining row that matches a pattern its `refines` target does not; the real table passes it.
- **Acceptance literal:** `sh tools/gate.sh | grep -F 'cp1610-c17'` prints one line, `control ok   cp1610-c17-inconsistent`, which is the control's own name in the runner output. No `FAIL` line and no `cp1610-c17-missing` or `cp1610-c17-inconsistent` token from the real tree appears, which is the criterion's intent.

## Known Stubs

None.

## Threat Flags

None. T-01-23: shape, printed/pattern, full decode coverage and the C17 link are permanent checks with controls; the independence rule D-20 is restated in the notes section. T-01-24: rows are read once into lists, findings are capped at 20, and the step is measured at about 0.7 s. The checks cover encoding consistency and cite presence only; whether a cite matches the page image is not machine-decidable and remains with plan 01-10's blind second reader (flagged assumption for FND-06).

## Issues Encountered

None beyond the deviations above. `.planning/config.json` shows a pre-existing modification that this plan did not touch and did not stage.

## Self-Check: PASSED

- Files exist: `tools/check/coverage.cmake`, `tools/check/c17-consistency.cmake`, `tools/check/steps/cp1610.sh`, `tools/check/schema/cp1610-row.txt`, `tools/check/kinds/cp1610-row.cmake`, `data/expected/cp1610-rows.ids`.
- Commits `61cb7bb` and `0171988` are ancestors of HEAD.
