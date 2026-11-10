---
phase: 02-cp-1610-cpu-core
plan: 05
subsystem: cpu-core
tags: [cmake, cp1610, vector-coverage, gate, controls]

requires:
  - phase: 02-cp-1610-cpu-core
    plan: 04
    provides: structural cp1610-vectors checker (17 tokens, decle_leaves, per-token cap), faults/cpu.ids, cp1610-policies.ids
provides:
  - derived coverage in tools/check/vectors.cmake (15 new tokens; 32 in total, each with one control)
  - data/expected/cp1610-coverage-pending.ids (72 rows, class:sdbd, 18 policies)
  - data/expected/cp1610-flag-exempt.ids (CLRC/flag-set:C, SETC/flag-clear:C)
  - data/expected/cp1610-addr-required.ids (35 lines), cp1610-branch-required.ids (68 lines), cp1610-modifier-witness.ids (MVI@ cycles:mod PULR)
  - stats rows-pending=, policies-pending=, bus-figure-pages=
affects: [02-06, 02-07, 02-08, 02-09, 02-10, 02-11, 02-12, 02-13, 02-14, 02-16]

actuals:
  tokens: 13400
  tasks: 2
  commits: 2

tech-stack:
  added: []
  patterns:
    - "A coverage tag counts toward a requirement only after the vector's own init/expect/program proves it; unproven tags fail vector-tag-false and count for nothing"
    - "Free-form labels outside the defined tag families (fetch:bus, unit:*, iab:*, reset:*, toy) are not proven and not counted"
    - "Requirement and witness files share one shape: `#` header, data lines, `#allow-empty: <reason>` when empty; malformed, unknown-row, out-of-family and repeated lines fail the file's format token"

key-files:
  created:
    - data/expected/cp1610-coverage-pending.ids
    - data/expected/cp1610-flag-exempt.ids
    - data/expected/cp1610-addr-required.ids
    - data/expected/cp1610-branch-required.ids
    - data/expected/cp1610-modifier-witness.ids
    - tools/check/controls/vectors-{tag-false,row-uncovered,flag-uncovered,cycles-uncovered,cycles-basis,policy-uncovered,policy-no-fault,pending-covered}/
    - tools/check/controls/vectors-{addr-uncovered,addr-required-format,bus-uncovered,branch-uncovered,branch-required-format,witness-format,witness-uncovered}/
    - tools/check/controls/_good/tree/data/expected/{cp1610-coverage-pending,cp1610-flag-exempt,cp1610-addr-required,cp1610-branch-required,cp1610-modifier-witness}.ids
  modified:
    - tools/check/vectors.cmake
    - tools/check/steps/cp1610-vectors.sh
    - data/cp1610/vectors/control.json (covers tags only)
    - data/expected/controls/vectors.ids
    - tools/check/controls/_good/tree/data/cp1610/vectors/toy.json
    - tools/check/controls/_good/tree/build/gate/{dev,release,faults/GOOD_FAULT}/toy.txt (digest only)
    - tools/check/controls/{faults-*,tests-inventory-*}/overlay/.../toy.txt (12 files, digest only)
    - tools/check/controls/vectors-*/overlay/data/cp1610/vectors/toy.json (11 older controls rebased onto the new good file)

key-decisions:
  - "Only proven tags satisfy derived requirements, so one false tag gives vector-tag-false and never also hides a coverage gap"
  - "control.json loses six flag tags its data does not prove under this plan's semantics (flag-preserved needs init 1; flag-clear needs init 1; flag-set needs init 0); CLRC and SETC stay fully covered by the other vector of each pair"
  - "The branch direction bit is bit 5, printed as S on pdf 54 (`S is 0 for +, 1 for -`) and explained on pdf 47; the plan's `pdf 52` is the jump page, so the checker comment cites pdf 54 and pdf 47"
  - "cycles:n0/n1 prove only expect.cycles 6/8 on kind=source vectors of `6/8` rows, as the plan states; the N bit of the decle is not read"
  - "B is identified by row id for the taken-only rule (pdf 54: unconditional); the policy-row set is cycles_printed string-equal to `7/9`"
  - "vector-bus-uncovered is reported only for rows that have vectors (a row with none already fails vector-row-uncovered); the addr:sdbd need of `supported` sdbd rows is reported as vector-addr-uncovered once class:sdbd leaves the pending list"
  - "A malformed or meaningless flag-exempt line is reported as vector-flag-uncovered; a repeated required-tag or witness line fails the file's format token"
  - "requirements-completed stays empty as in plans 02-02..02-04: CPU-01, CPU-04, CPU-05 and RUN-03 are advanced and completed by later plans of this phase"

patterns-established:
  - "A plan that takes a row off cp1610-coverage-pending.ids must land every derived need of that row in the same change: a vector, flag set/clear/preserved, cycle tags, required addr/branch tags, a bus list"
  - "A plan that takes a policy off the pending list also lands a fault whose must-fail vector names that policy"

requirements-completed: []
requirements-advanced: [CPU-01, CPU-04, CPU-05, RUN-03]

coverage:
  - id: D1
    description: "Tags flag-*, cycles:*, policy:*, addr:*, branch:*, ebci:* are proven against each vector's own data; a false tag fails vector-tag-false"
    requirement: RUN-03
    verification:
      - kind: other
        ref: "sh tools/check/run-controls.sh: 'control ok   vectors-tag-false'; throwaway probe on a scratch copy of the real data (true and false cases for each family)"
        status: pass
    human_judgment: false
  - id: D2
    description: "Row, flag, cycle (incl. A-05 basis), policy, fault-link and pending requirements derived from the table for non-pending rows; CLRC and SETC are the first covered rows"
    requirement: CPU-01
    verification:
      - kind: other
        ref: "sh tools/gate.sh: '-- cp1610-vectors pass vectors=7 source=4 policy=3 rows-pending=72 policies-pending=18 bus-figure-pages=4'; 7 controls vectors-{row,flag,cycles}-uncovered, cycles-basis, policy-uncovered, policy-no-fault, pending-covered"
        status: pass
    human_judgment: false
  - id: D3
    description: "Exact addressing (35) and branch/E-code (68) requirement lists, per-row bus rule and the MVI@ -> PULR modifier witness (A-13)"
    requirement: CPU-05
    verification:
      - kind: other
        ref: "grep counts 35, 68, 1, 16, 4; 7 controls vectors-{addr-uncovered,addr-required-format,bus-uncovered,branch-uncovered,branch-required-format,witness-format,witness-uncovered}; positive witness probe"
        status: pass
    human_judgment: false
  - id: D4
    description: "Every cp1610-vectors token (32) has exactly one control; gate green under sh and dash"
    requirement: CPU-04
    verification:
      - kind: other
        ref: "per-token EXPECT loop prints 0 mismatches; gate: PASS steps=23/23 controls=169/169 (sh and dash)"
        status: pass
    human_judgment: false

duration: 16min
completed: 2026-11-10
status: complete
plan_head_before: 02d0e10d76b0d3d69f4b111e9fe2122edbd5d296
plan_head_after: e6f53c5bf5e2a989e3f9f51b7d1d1ca75ebecde2
commits: 2
---

# Phase 2 Plan 05: derived vector coverage Summary

**`cp1610-vectors` now derives each row's required coverage from the opcode table and proves every coverage tag against the vector's own data. Coverage cannot be met by claim alone. CLRC and SETC are the first rows held to it. 72 rows and 18 policies are explicitly pending, and every one of the 32 checker tokens has its own control.**

## Performance

- **Duration:** about 16 min of execution: from the plan-02-04 metadata commit at 01:15:50 local time to the Task 2 commit at 01:29:43, plus this summary. Plan and page reading came before that.
- **Started:** 2026-11-10T06:15:50Z
- **Completed:** 2026-11-10T06:33Z
- **Tasks:** 2 (1 tracer, 1 auto)
- **Files:** 89 changed in the two task commits (1883 insertions, 56 deletions). Most are small control files and digest-only fixture refreshes.

## Accomplishments

- **Tag proofs (`vector-tag-false`).** Each tag is checked against the vector itself:
  - `flag-set|clear|preserved:F` against init and expect status.
  - `cycles:base|mod` against single-step `expect.cycles` and the row's leading printed integer, plus k.
  - `cycles:n0|n1` as kind=source vectors of `6/8` rows with 6 or 8 cycles.
  - `cycles:taken|not-taken` as POL-BR-01 policy vectors of `7/9` rows with 9 or 7 cycles.
  - `addr:*` from the MMM and DDD fields. The target word is the second one after an SDBD word.
  - `branch:*` from bit 5 and the final R7 against start + 2 (16-bit).
  - `ebci:e<N>-true|false` from decle AND $F and the vector's `ebci` list.
  - `policy:*` against `cp1610-policies.ids`.
- **Derived requirements.** These apply to every table row not listed in `cp1610-coverage-pending.ids`:
  - at least one vector;
  - set and clear for each `status_bits` flag (the union for the `N=` forms) and preserved for every other flag, with exemptions only in `cp1610-flag-exempt.ids`;
  - the cycle tags listed above. A row listed in the witness file has its `cycles:mod` need met by the witness instead;
  - the required addressing and branch tags;
  - a vector with a non-empty `bus` list;
  - an `addr:sdbd` vector for `supported` sdbd rows once `class:sdbd` leaves the pending list.
- **A-05 cycle basis (`vector-cycles-basis`).** The policy rows are exactly the 16 rows whose `cycles_printed` is the string `7/9`. Their kind=source vectors must omit `expect.cycles`. Every other vector, shift vectors included, must assert it.
- **Policies.** Each policy that is not pending needs a vector that names it, through its `policy` key or a proven `policy:` tag. It also needs a fault in `faults/cpu.ids` whose must-fail vector names it. A pending row or policy that holds any vector fails `vector-pending-covered`.
- **Data:**
  - The pending list (72 rows, `class:sdbd`, 18 policies; only POL-RST-01 is live).
  - The flag exemptions.
  - The 35-line addressing list and the 68-line branch list.
  - The `MVI@ cycles:mod PULR` witness.
- **Stats:** `rows-pending=72 policies-pending=18 bus-figure-pages=4`. The four pages are pdf 22, 27, 28 and 56.
- **15 new controls,** one per new token. They are appended to `data/expected/controls/vectors.ids`.

## Task Commits

1. Task 1 (tracer): 14218de - `feat(02-05): derived vector coverage with tag proofs and eight controls`
2. Task 2: e6f53c5 - `feat(02-05): required addressing and branch tags, modifier witness and bus rule`

Tracer feedback gate: the tracer `<verify>` ran end to end before Task 2 started. It gave `gate: PASS steps=23/23 controls=162/162` with `-- cp1610-vectors pass vectors=7 source=4 policy=3 rows-pending=72 policies-pending=18 bus-figure-pages=4`.

## Verification results

- **Final gate.** Committed tree, run unpiped: `gate: PASS steps=23/23 controls=169/169 ... vectors=7 source=4 policy=3 rows-pending=72 policies-pending=18 bus-figure-pages=4 ... faults=2/2 ... tests-expected=54 tests-executed=54 tests-skipped=0 tests-missing=0 ...`. Wall time 32.8 s (`11.17s user 19.59s system 93% cpu 32.776 total`).
- **Other gate runs.** The redirected `sh` run after Task 2 took 30.2 s. `dash tools/gate.sh` gave `gate: PASS steps=23/23 controls=169/169` in 43.3 s (under load).
- **Task 1 acceptance:**
  - `^(CLRC|SETC)$` count 0 and `^policy:POL-` count 18 in the pending list.
  - The control.json diff has 0 lines outside `covers|flag-|policy:`.
  - 8 of the new `control ok   vectors-*` lines are present.
- **Task 2 acceptance:**
  - The line counts are 35, 68, 1, 16 (back-not-taken) and 4 (BEXT E codes 5 and 10).
  - The per-token loop over all 32 tokens prints 0 mismatches.
  - Each token occurs in `vectors.cmake`.
- **Collateral tokens.** Every control was also run on its own. Each emits its own token. The only extra tokens are those its defect implies. For example, a row with no vector also misses its flag cases, and a pending row that gains a vector also fires `vector-pending-covered`. No control depends on an extra token.
- **Throwaway probes (scratch only, nothing committed).**
  - Real-data copy with probe vectors:
    - Proven true: BEXT fwd-taken, `ebci:e5-true`, PULR `cycles:base` (11), `addr:stack`, SDBD+MVII `addr:sdbd`, `addr:immediate`, `addr:r7-dest`, BC back-not-taken, and the POL-BR-01 vector's `cycles:taken`.
    - Reported false: wrong direction, wrong E code, `cycles:taken` on a source vector, `addr:direct` on PULR, `addr:autoinc` on MVII, MOVR or TSTR `addr:r7-dest` with DDD != 7, TSTR `cycles:mod` with SSS=1, and `cycles:not-taken` with 9 cycles.
  - Good-tree witness probe: a 7-cycle `cycles:base` witness meets the `+3` branch, and the row then needs no `cycles:mod`.

## Decisions Made

See `key-decisions` in the frontmatter. For plan 02-06 and later, the rule from `patterns-established` applies: a row comes off the pending list only together with every one of its derived needs.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Six coverage tags in control.json were not proven by their own data**
- **Found during:** Task 1
- **Issue:** Under this plan's tag semantics, several plan 02-03 tags were false:
  - `cpu.CLRC.clears-c` and `cpu.SETC.sets-c` tagged `flag-preserved:S/Z/OV` with init 0 (preserved needs init 1).
  - `cpu.CLRC.c-already-clear-flags-preserved` tagged `flag-clear:C` with init C=0.
  - `cpu.SETC.flags-preserved` tagged `flag-set:C` with init C=1.
- **Fix:** Removed those tags and changed no value. The other vector of each pair proves the preserved cases. The reset and not-reset vectors gained `policy:POL-RST-01` as the first `covers` element, so no existing line changed.
- **Files modified:** data/cp1610/vectors/control.json
- **Verification:** The acceptance diff check prints 0, and the real tree passes with CLRC and SETC fully covered.
- **Commit:** 14218de

**2. [Rule 3 - Blocking] Digest refresh and overlay rebase after the good toy.json changed**
- **Found during:** Tasks 1 and 2
- **Issue:** The good toy.json changed twice: alpha's init flags and preserved tags, then its `addr:direct` tag.
  - The digest sits in 3 good TAP files and in 12 faults and tests-inventory overlay TAP files.
  - 11 older `vectors-*` controls carry whole-file copies of toy.json.
  - Without a refresh, these controls would stop being single-defect.
- **Fix:**
  - Replaced the digest in all 15 TAP files. `tests-inventory-stale-digest` keeps its deliberately wrong one.
  - Rebased each older overlay with `diff`/`patch` onto the new good file.
  - Regenerated the two Task 1 overlays that edit the same lines.
- **Commits:** 14218de, e6f53c5

**3. [Plan text] Page number for the branch direction bit**
- **Issue:** The plan cites the printed `S` field on pdf 52. The page image gi-052 is the jump page (3.3.5). The branch table with `1000 S0 cccc` and "S is 0 for +, 1 for -" is pdf 54, which is also the cite of the B..BEXT rows in `opcode-families.json`.
- **Fix:** The checker comment cites pdf 54 and pdf 47. pdf 47 says direction bit 0 adds the displacement to PC+2 and 1 adds its ones complement.
- **Commit:** e6f53c5

**4. [Scope note] No `cycles:base` tags added to control.json**
- **Issue:** Step 2 of Task 1 asks to add any tag the vectors prove but lack. `cycles:base` is provable on the four CLRC and SETC vectors. However, the Task 1 acceptance check allows only `covers|flag-|policy:` lines in the diff.
- **Resolution:** The acceptance check wins. No cycle tag is required for these rows (they have no `+k` modifier).

**Total deviations:** 2 auto-fixed (Rule 1, Rule 3), 1 plan-text page correction and 1 scope note. **Impact:** none on scope. Every token has exactly one control, and the stats are exactly as planned.

## Authentication Gates

None.

## Known Stubs

None. The pending list is intentional by design. Each later plan removes lines from it as it lands vectors, and `vector-pending-covered` stops a row from holding partial work while it is still listed.

## Threat Flags

None. All new surface is in the plan's threat model:
- T-02-11: tag proofs and pending rows that can hold no vector.
- T-02-47: an empty requirement or witness file needs `#allow-empty:`, the line counts are exact (35, 68, 1), the `refines` back-link is checked, and 7 controls cover these rules.

## Issues Encountered

- **Gate time:** 32.8 s local wall time (30.2 s in the redirected run), above the D-22 local budget of 25 s. The 15 new controls add about 1.5 s. Plan 02-16 owns the measurement and the levers (background C build, parallel control runner), as plan 02-04 recorded.
- **Checker size:** `vectors.cmake` is now 1328 lines. The coverage pass costs about 0.1 s on the real tree (7 vectors). Its per-tag `IN_LIST` lookups scale linearly. Watch the cost at about 300 vectors (plan 02-16).

## Next Phase Readiness

Plan 02-06 and later plans take rows off `data/expected/cp1610-coverage-pending.ids`. Each row they remove must, in the same change, meet:
- its flag cases;
- its cycle tags (MVI@ through a single-step PULR `cycles:base` vector of 11 cycles);
- its required `addr:` and `branch:` lines;
- a bus list.

Policies leave the list only together with a fault whose must-fail vector names them. `class:sdbd` leaves together with `addr:sdbd` vectors for the 12 `supported` rows.

## Self-Check: PASSED
