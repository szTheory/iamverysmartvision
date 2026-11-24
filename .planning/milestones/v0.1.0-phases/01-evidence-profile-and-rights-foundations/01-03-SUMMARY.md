---
phase: 01-evidence-profile-and-rights-foundations
plan: 03
subsystem: cp1610-opcode-inventory
tags: [cp1610, opcode-families, page-image-reading, coverage, fnd-06]
requires:
  - phase: 01-evidence-profile-and-rights-foundations
    provides: "refs/ cache holding the GI manual with verified sha256 (plan 01-02); gate and sources registry (plans 01-01, 01-14)"
provides:
  - "data/cp1610/opcode-families.json: 74 hand-transcribed opcode family rows from pdf 50-58 in the fixed 17-key row shape, each with a `pdf <n>` cite"
  - "Throwaway-checked proof that the rows tile all 1024 decle patterns: unclaimed=0 ambiguous=0 coincidences=1 (0010111111)"
affects: [01-09 permanent coverage checker, 01-10 blind second reader, 01-04 discrepancies, Phase 2 coverage inventory]
actuals:
  tokens: 21000
  tasks: 3
  commits: 3
plan_head_before: 8b42932c4b10e1efc7890a236a37da6fd721a11a
plan_head_after: e6e684b4ea88faec687bc25fe50004d55487b1aa
tech-stack:
  added: []
  patterns:
    - "Rows carry `printed` exactly as on the page and a derived 10-character `pattern`; relational fields use `eq`; specializations use `refines`; a documented coincidence uses `also_encodes`"
key-files:
  created:
    - data/cp1610/opcode-families.json
  modified: []
key-decisions:
  - "Row count is 74, below D-19's about 90: SDBD forms are attributes on memory rows, branch aliases share one row, and the jump family is one first-decle row (RESEARCH Pitfall 9.6); no padding"
  - "NOPP is transcribed with the printed cycle count 7, not 7/9; the plan's Task 2 verify expected 7/9 for every non-BEXT branch row"
  - "Interruptibility: `no` only where the page prints it (shifts, EIS, DIS, TCI, CLRC, SETC); every other row is `manual-silent`; no row is `yes`"
  - "Rows sharing an identical `pattern` string (MOVR/TSTR, XORR/CLRR, MVO@/MVO ...) are ordered general row first, then the refining row"
requirements-completed: []  # FND-06 also depends on 01-09 and 01-10; left open on purpose
duration: about 5 min of tool time (plus reading)
completed: 2026-11-09
status: complete
---

# Phase 1 Plan 03: CP-1610 opcode family inventory Summary

**`data/cp1610/opcode-families.json` holds 74 opcode family rows transcribed from the GI manual page images (pdf 50-58), each cited, and a throwaway decode over all 1024 patterns finds `unclaimed=0 ambiguous=0 coincidences=1`.**

## Performance

- **Duration:** about 5 min of tool time after reading the page images
- **Tasks:** 3 of 3 (tracer, branch rows, memory rows)
- **Files:** 1 created (`data/cp1610/opcode-families.json`, about 53 KB, ASCII, LF)

## Accomplishments

- Precondition route: the manual was read from the repo-local ignored cache `refs/SRC-gi-cp1600-um-1975.pdf` populated by plan 01-02 (route: refs cache, not the D-16 local copy and not a download). `cmake -E sha256sum` printed `ecdabe5c52b5e4429a40d1779d82315ab74bae9db00df774e605f8a882dc2b72`, equal to the pinned digest. This plan wrote nothing under `refs/` or `.git/info/exclude`; pdf 46 and 50-58 were rendered with `pdftoppm -r 110` into the session scratchpad (outside the repo) and re-rendered at `-r 170` with crops for pdf 53, 54, 55 and 57. Every encoding, cycle count, modifier and status bit comes from a page image; no OCR text, emulator source or third-party opcode table was used.
- **Table shape.** 74 rows: 8 internal-control (HLT, SDBD, EIS, DIS, J with aliases JE JD JSR JSRE JSRD, TCI, CLRC, SETC), 9 single-register (INCR DECR COMR NEGR ADCR GSWD NOP SIN RSWD), 8 register-shift, 9 register-register, 17 branch (16 condition codes with BLGT, BEQ, BLLT, BNEQ as aliases, plus BEXT), 23 memory-family (7 indirect, 7 direct, 7 immediate, PSHR, PULR). Generic layouts such as `0000 ooo DDD` and `0001 ooo NRR` are quoted in row notes only; no 0000-region or 0001-region row has a `.` in an op-code position.
- **Declared refines (19, as planned):** TSTR and JR to MOVR; CLRR to XORR; the seven direct rows and seven immediate rows to their indirect rows; PSHR to MVO@; PULR to MVI@. No page-driven difference from the planned set.
- **Declared coincidence:** `0010 111 111` (TSTR R7, JR R7, MOVR R7,R7) is linked by `also_encodes` on TSTR and JR and explained in both notes.
- **Cross-check:** the memory-family high nibbles (MVO 1001, MVI 1010, ADD 1011, SUB 1100, CMP 1101, AND 1110, XOR 1111) and the mode field MMM were compared with the pdf 46 Fig 3.2.1 operation and mode code table; they agree.
- The top-level `note` states the D-20 rule: Phase 2 vectors are hand-derived per opcode from the cited page and never generated from this table.

## Task Commits

1. Task 1 (tracer): `cdca4a2` feat(01-03): opcode family table, control through register-register rows (34 rows)
2. Task 2: `58e75f1` feat(01-03): opcode family table, branch rows (51 rows)
3. Task 3: `e6e684b` feat(01-03): opcode family table, memory, indirect and immediate rows with SDBD attributes (74 rows)

## Verification (executed)

- Tracer gate (auto mode active): the Task 1 `<verify>` command was re-run end to end after the commit and printed `34` (threshold 34); then expanded.
- Throwaway decode (python3 -I script kept in the session scratchpad, never tracked), including printed-vs-pattern consistency and reference integrity (`consistency_errors=0` in every run):
  - Task 1, patterns 0..511: `range=0..511 unclaimed=0 ambiguous=0 coincidences=1 consistency_errors=0 0010111111`
  - Task 2, patterns 512..575: `range=512..575 unclaimed=0 ambiguous=0 coincidences=0 consistency_errors=0`
  - Task 3, full 0..1023: `range=0..1023 unclaimed=0 ambiguous=0 coincidences=1 consistency_errors=0 0010111111` (74 rows)
- Non-vacuity spot check, RSWD removed in a scratch copy: `range=0..1023 unclaimed=8 ambiguous=0 coincidences=1 consistency_errors=0 0010111111`, so the check can fail.
- The whole-table overlap audit is the full decode: it covered all 1024 patterns and every overlap is declared by `refines` or `also_encodes` (`ambiguous=0`).
- `grep -c '"refines": "'` = 19; `grep -c '"cite": ".*pdf [0-9]'` = 74 (equals the row count); 17 branch rows, all with a `pdf 54` cite; the Task 3 verify printed `74`.
- `LC_ALL=C grep -n '[^[:print:][:space:]]'` on the JSON printed nothing; `git ls-files refs` printed nothing; the decode and serializer scripts are not tracked; `git status --porcelain` showed only the JSON (plus a pre-existing unrelated `.planning/config.json` change that this plan did not touch).
- `sh tools/gate.sh` after Task 3: `gate: PASS steps=3/3 controls=20/20 files=133 records=23 fields=23 unknown-fields=12 tags=10 tiers=8`.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Page truth over plan expectation] NOPP cycles transcribed as `7`, not `7/9`**
- **Found during:** Task 2
- **Issue:** The plan's Task 2 `<verify>` asserts every non-BEXT branch row prints `7/9`, but the pdf 54 image (zoomed at 170 dpi) prints NOPP as `7`, consistent with the notes (`docs/cp1610/reference-notes.md`, branch timing section) and RESEARCH.
- **Fix:** Row NOPP carries `cycles_printed: "7"` with a note; the verify was run with NOPP also exempted (`non-7/9 non-BEXT: ['NOPP']`). The check was not weakened elsewhere; all 15 other non-BEXT branch rows and BEXT print `7/9`.
- **Files modified:** `data/cp1610/opcode-families.json`
- **Commit:** `58e75f1`
- **Follow-up for plan 01-09/01-10:** a permanent check must not require `7/9` on NOPP.

**2. [Rule 1 - Page truth] `sdbd` cite for indirect rows is pdf 57, not pdf 58**
- **Found during:** Task 3
- **Issue:** The plan's row-shape text writes `supported: <cycles as printed> (pdf 58)`, but the indirect SDBD table is on pdf 57; only the immediate SDBD table is on pdf 58.
- **Fix:** Indirect rows use `supported: 10 (pdf 57)`, immediate rows `supported: 14 (pdf 58)`. Both satisfy the 01-09 pattern `^(not-applicable|unsupported|supported: .+)$`.
- **Commit:** `e6e684b`

### Notes (not deviations)

- **D-19 row count:** 74 rows against D-19's "about 90", for the reasons in the plan's Decision notes (SDBD as attributes, aliases on one row, one jump row). Not padded.
- **XOR@ SDBD print slip:** the pdf 57 XOR@ SDBD row prints its op code as `111` (three digits); the row note records the inferred reading `1111` labelled [INF].
- **SDBD attribute values:** direct rows and PSHR/PULR are `unsupported` (pdf 57 says modes 0 and 6 are not supported); MVO@ and MVOI are `not-applicable` because no SDBD form is printed for the output rows and the manual does not exclude them; this is a reading choice that plan 01-10's blind reader may differ on.
- **Blank status columns:** GSWD, NOP, SIN, HLT, SDBD, EIS, DIS, TCI, branch rows and the non-arithmetic memory rows print no status bits; they carry `none`. GSWD's note records the footnote about R4/R5 and R6/R7 (pdf 51).
- **Shift cycles:** `cycles_printed` is `6/8`, a rendering of the two figures printed on two lines (6 for N=0, 8 for N=1), with the page header "add 2 cycles if shift is 2 bits or two bytes" in `cycle_modifiers`; status bits are given per N value for RLC, SLLC, RRC and SARC.
- **Ordering:** rows with identical pattern strings (MOVR then TSTR, XORR then CLRR, and each general indirect row then its direct row) are in general-then-refining order; this tie-break is stated in the top-level note.
- Fig 3.3.2 mode-bit naming versus the shift mnemonics (pdf 50 vs 53) was already logged as a discrepancy candidate by plan 01-02; the rows follow the table.
- FND-06 was not marked complete: it also depends on plans 01-09 and 01-10.

## Known Stubs

None.

## Threat Flags

None. T-01-09: renders lived only in the session scratchpad; `git ls-files refs` empty; no PDF or image tracked. T-01-10: printed-vs-pattern consistency, the full 1024 decode with the RSWD non-vacuity check, and the pdf 46 high-nibble cross-check were run; the blind second reader remains plan 01-10. No absolute home or scratch path appears in the JSON or this SUMMARY.

## Issues Encountered

None.

## Next Phase Readiness

Plan 01-09 can add `data/expected/cp1610-rows.ids` from the 74 row ids and the permanent coverage checker (including the NOPP and sdbd-page points above). Plan 01-10 can run the blind second reader over pdf 50-58 against these rows.

## Self-Check: PASSED

- `data/cp1610/opcode-families.json` exists (74 rows); commits `cdca4a2`, `58e75f1` and `e6e684b` are ancestors of HEAD.
