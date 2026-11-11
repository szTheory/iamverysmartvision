---
phase: 02-cp-1610-cpu-core
plan: 11
subsystem: cpu-core
tags: [c17, cp1610, branch, displacement, pol-br-01, vectors, named-faults, cpu-01]

requires:
  - phase: 02-cp-1610-cpu-core
    plan: 10
    provides: ivs_imm_read (the one PC-advance site, READ at offset 4), the 74-row decode table with the 16 branch rows routed to IVS_CP1610_OP_NONE, the vector checker's branch:* and POL-BR-01 cycles:* proofs
provides:
  - B, the 14 internal-condition branches (BC BOV BPL BZE BLT BLE BUSC BNC BNOV BMI BNZE BGE BGT BESC) and NOPP through one handler, IVS_CP1610_OP_BRANCH
  - ivs_branch (displacement read at PC+1, forward adds it to PC+2, backward adds its 16-bit one's complement, C50) and ivs_branch_cond (bits 2..0 pick the pdf 54 test, bit 3 negates it, NOPP never branches)
  - POL-BR-01 cycles - base 7, +2 (IVS_BRANCH_TAKEN_EXTRA) when taken; NOPP 7
  - data/cp1610/vectors/branch.json (group branch, 92 vectors - 63 source, 29 POL-BR-01 policy)
  - 6 named faults BRANCH_BACKWARD_SIGN BRANCH_TAKEN_CYCLES BRANCH_NEGATE_IGNORED BRANCH_NOT_TAKEN_CYCLES NOPP_TAKEN BRANCH_COND_SIGNED
affects: [02-12, 02-13, 02-14, 02-15, 02-17]

actuals:
  tokens: 58933
  tasks: 3
  commits: 3
plan_head_before: 9b38a070664ab774d9210ba9e3bbfffacc714b2e
plan_head_after: a831fdcfcd507780fd0eed37bc186154de62ee32

tech-stack:
  added: []
  patterns:
    - "One branch handler for the 16 internal-condition rows: the row only routes, the condition field of the decle selects the test"
    - "Branch cycles live only in kind=policy POL-BR-01 vectors; source vectors on 7/9 rows carry no expect.cycles (A-05)"
    - "Per conditional row, forward vectors keep the untested flags at 1 and backward vectors at 0, so a test of the wrong flag fails in one direction or the other"

key-files:
  created:
    - data/cp1610/vectors/branch.json
  modified:
    - core/src/cpu/cp1610.h
    - core/src/cpu/cp1610.c
    - core/src/cpu/cp1610_decode.c
    - data/expected/cp1610-vector-files.ids
    - data/expected/cp1610-vectors.ids
    - data/expected/tests/cpu-vectors.ids
    - data/expected/cp1610-coverage-pending.ids
    - data/expected/faults/cpu.ids

key-decisions:
  - "Branch condition = test(bits 2..0) xor bit 3. The negate bit is a reading of the pdf 54 table: each bit-3 = 1 row prints the opposite test of its bit-3 = 0 partner. NOPP (1000) is B negated, so it never branches."
  - "The displacement word is read through ivs_imm_read, so the one PC-advance site stays single and R7 is PC+2 before the target is formed. IMMEDIATE_PC_ADVANCE therefore covers all 16 branch rows."
  - "Backward target = PC+2 + (~disp & $FFFF) (C50). With displacement $0000 that is PC+1, the cheapest case that separates one's from two's complement."
  - "Branch bus lists are FETCH at 0 and a READ of PC+1 at 4, the displacement. They are figure basis (pdf 32, which ends at the displacement BAR), and no later access is listed."
  - "Branch rows are interruptible (no Not Interruptable note on pdf 54); every branch vector shows prev_interruptible 0 -> 1."
  - "requirements-completed stays empty, as in plans 02-02..02-10: CPU-01 advances, and plan 02-12 still declares it"

patterns-established:
  - "A row family that differs only in a condition field gets one handler and one condition function, with one fault site per condition class"

requirements-completed: []
requirements-advanced: [CPU-01]

coverage:
  - id: D1
    description: "B forward and backward follow the pdf 47 displacement rule with C50's 16-bit one's complement (cpu.B.backward-minus-1, cpu.B.backward-wrap, cpu.B.forward-zero pass); BRANCH_BACKWARD_SIGN fails every backward-taken vector"
    requirement: CPU-01
    verification:
      - kind: integration
        ref: "sh tools/gate.sh (faults-verdict pass faults=34/34; build/gate/faults/BRANCH_BACKWARD_SIGN/cpu-vectors.txt)"
        status: pass
    human_judgment: false
  - id: D2
    description: "The 14 conditional rows are proven taken and not taken in both directions (56 source vectors, no cycles) and carry cycles only in 28 POL-BR-01 policy vectors plus cpu.B.cycles-taken; the negate bit, the signed rule, NOPP and both cycle values each have a fault"
    requirement: CPU-01
    verification:
      - kind: integration
        ref: "sh tools/gate.sh and dash tools/gate.sh (gate: PASS vectors=302 source=263 policy=39 rows-pending=2 faults=34/34)"
        status: pass
    human_judgment: false

duration: 9min
completed: 2026-11-10
status: complete
---

# Phase 2 Plan 11: Branch Group Summary

**B, the 14 internal-condition branches and NOPP now run through one handler. The handler reads the displacement word at PC+1. Going forward, it adds the displacement to PC+2. Going backward, it adds the 16-bit one's complement (pdf 47, C50). Bits 2..0 of the condition field pick the pdf 54 test and bit 3 negates it. 92 hand-derived vectors cover every row in both directions. Branch cycles appear only in the 29 POL-BR-01 policy vectors. 6 named faults guard the rules.**

## Performance

- **Duration:** about 9 min
- **Started:** 2026-11-10T08:14:25Z
- **Completed:** 2026-11-10T08:23Z
- **Tasks:** 3 of 3
- **Files modified:** 9 (5037 insertions, 35 deletions)

## Accomplishments

- **Target rule.** `ivs_branch` reads the displacement through `ivs_imm_read`:
  - It is a READ at offset 4 of PC+1, and R7 ends at PC+2.
  - Not taken, R7 stays at PC+2.
  - Taken and bit 5 = 0, R7 = PC+2 + displacement.
  - Taken and bit 5 = 1, R7 = PC+2 + (~displacement & $FFFF). Sums wrap at 16 bits.
  - No status bit changes.
- **Conditions.** `ivs_branch_cond` reads the tests off pdf 54:
  - 000 B (always)
  - 001 BC: C = 1
  - 010 BOV: OV = 1
  - 011 BPL: S = 0
  - 100 BZE: Z = 1
  - 101 BLT: S xor OV = 1
  - 110 BLE: Z or (S xor OV) = 1
  - 111 BUSC: C xor S = 1
  
  Bit 3 negates each test. That gives BNC, BNOV, BMI, BNZE, BGE, BGT and BESC, and NOPP (1000), which never branches.
- **Cycles (POL-BR-01, C29).** The decode base is 7, and a taken branch adds `IVS_BRANCH_TAKEN_EXTRA` (2), giving 9. NOPP is always 7, the single figure printed for it.
- **Vectors.** `branch.json` has 92:
  - B has 4 source vectors and `cpu.B.cycles-taken`.
  - Each of the 14 conditional rows has 6: four direction/outcome source vectors plus `cycles-taken` (9) and `cycles-not-taken` (7) policy vectors.
  - There is one signed-rule vector, `cpu.BLT.s1-ov1-not-taken`.
  - NOPP has 2: `cpu.NOPP.fwd-never-taken` and `cpu.NOPP.back-never-taken` (7 cycles, `cycles:base`).
  - Every row has a figure-basis bus list on its forward-taken vector. NOPP's is on its forward vector.
  - All 92 passed against the core on their first run.
- **Gate.** The pending list went from 18 rows to 2 (BEXT and J), and `policy:POL-BR-01` left it. The gate now runs 302 vectors, 646 tests and 34 faults.

## Task Commits

1. **Task 1: Tracer, B forward and backward end to end** - `9255c42` (feat). This added 5 B vectors and the BRANCH_BACKWARD_SIGN and BRANCH_TAKEN_CYCLES faults, giving `faults=30/30 rows-pending=17`. Tracer gate: `sh tools/gate.sh` re-ran green before expansion.
2. **Task 2: BC, BNC, BOV, BNOV, BPL, BMI, BZE, BNZE** - `a8266bf` (feat). This added 48 vectors and the BRANCH_NEGATE_IGNORED and BRANCH_NOT_TAKEN_CYCLES faults, giving `faults=32/32 rows-pending=9`.
3. **Task 3: BLT, BGE, BLE, BGT, BUSC, BESC, the signed rule and NOPP** - `a831fdc` (feat). This added 39 vectors and the NOPP_TAKEN and BRANCH_COND_SIGNED faults, giving `faults=34/34 rows-pending=2`.

**Plan metadata:** recorded in the docs(02-11) completion commit.

## Bus offsets and the pages that print them

| Access | Offsets | Basis | Page |
|--------|---------|-------|------|
| All 16 rows (taken, not taken, NOPP) | FETCH of the branch word at 0; READ of the displacement word at PC+1 at 4 | figure | pdf 32, the one branch timing figure (drawn for BEXT): BAR "output program counter", NACT, DTB, NACT, BAR "output PC+1 to fetch displacement". The figure ends there, so no access after the displacement BAR is listed (RESEARCH 1b, A9). |

## Vector disagreements and resolutions

None against the core. All 92 vectors passed on their first run. No `expect` was edited after a run, and no core change was made to match a vector.

Page readings for the plan 02-17 second reader to re-derive independently:

- **BLE on pdf 54.** The rendered line is "Z V (S xor OV) = 1" with a faint mark above the Z. A 250 dpi re-render shows a broken dotted smudge, not a solid overbar like the ones printed elsewhere. It was read as a scan artifact, so BLE = Z or (S xor OV), which is also what "Less than or Equal" means. If it were an overbar, BLE would be (not Z) or (S xor OV). `cpu.BLE.fwd-taken` (Z = 1, S xor OV = 0) and `cpu.BLE.fwd-not-taken` (Z = 0, S xor OV = 0) separate the two readings.
- **BGE on pdf 54.** The test prints as "S xor OV = (" with a damaged last character. It was read as 0, since BGE is the bit-3 partner of BLT ("= 1") and "Greater than or Equal".
- **BMI and BEQ.** These print "S - 1" and "Z - 1". They were read as "= 1" (scan dropout of the second stroke).
- **The negate bit.** The page never names bit 3 as a negate bit. The reading rests on the 8 printed pairs: 0000/1000 B/NOPP, 0001/1001 C = 1 / C = 0, and so on through 0111/1111 C xor S = 1 / = 0.
- **NOPP never branches.** pdf 54 prints "NoOPeration, two words", leaves the operand column blank and prints a single 7. POL-BR-01 also names NOPP as the never-taken twin of B.
- **C50.** `cpu.B.backward-minus-1` uses displacement $0000, so one's complement gives $1002 + $FFFF = $1001 and two's complement would give $1002. `cpu.B.backward-wrap` starts at PC $0004, so $0006 + $FFEF = $FFF5, below $0000. Every `back-taken` vector uses $0010, where $1002 + $FFEF = $0FF1 and two's complement would give $0FF2.
- **Direction bit.** Bit 5 is the direction bit. pdf 54 prints it as S ("S is 0 for +, 1 for -"), and pdf 47 prints it as the +/- bit 5 of Fig 3.2.2.

## Fault verdicts (latest gate run)

| Fault | Vectors that fail (all inside the allowed prefixes) |
|-------|------------------------------------------------------|
| BRANCH_BACKWARD_SIGN | the 2 B backward vectors and the 14 `back-taken` vectors |
| BRANCH_TAKEN_CYCLES | the 15 `cycles-taken` policy vectors |
| BRANCH_NEGATE_IGNORED | all 6 vectors of BNC, BNOV, BMI, BNZE, BGE, BGT and BESC, plus both NOPP vectors (44) |
| BRANCH_NOT_TAKEN_CYCLES | the 14 `cycles-not-taken` policy vectors and both NOPP vectors |
| NOPP_TAKEN | `cpu.NOPP.fwd-never-taken`, `cpu.NOPP.back-never-taken` |
| BRANCH_COND_SIGNED | 11 vectors of BLT, BGE, BLE and BGT, including `cpu.BLT.s1-ov1-not-taken` |
| IMMEDIATE_PC_ADVANCE (existing) | now also every branch vector (133 in all) |

Every must-pass vector passes under its fault: `cpu.MVII.r1-load`, `cpu.BC.fwd-taken`, `cpu.B.cycles-taken` and `cpu.B.forward-zero`.

## Decisions Made

- **One handler.** All 16 rows use `IVS_CP1610_OP_BRANCH` (57). Each task routed only its own rows from `IVS_CP1610_OP_NONE`, so no pending row had a handler early. BEXT keeps `IVS_CP1610_OP_NONE` for plan 02-12.
- **Fault sites.** Each fault site is one or two lines. The backward-sign and taken-cycles sites are in `ivs_branch`, and the not-taken-cycles site is on its not-taken path. The negate, NOPP and signed sites are in `ivs_branch_cond`.
- **Flag backgrounds.** Forward vectors keep the untested flags at 1, and backward vectors keep them at 0. The forward vectors supply each row's four `flag-preserved` proofs, and a test of the wrong flag fails in at least one direction.
- **`cpu.B.forward-zero` has no `branch:*` tag.** Its target equals PC+2, and the checker counts a branch as taken only when R7 differs from PC+2. B's two `*-taken` requirements are met by `forward-plus-10`, `backward-minus-1` and `backward-wrap`.
- **Flag exemptions.** None. The branch rows change no flag, so `cp1610-flag-exempt.ids` is unchanged.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Fault prefix] IMMEDIATE_PC_ADVANCE gained the 16 branch row stems**
- **Found during:** Task 1
- **Issue:** The displacement word goes through `ivs_imm_read`, the single PC-advance site. Under IMMEDIATE_PC_ADVANCE every branch vector fails, and those failures fall outside its allowed prefixes.
- **Fix:** Following the plan 02-08 rule, each task added its own exact stems in the same commit: `cpu.B.`, then the eight single-flag stems, then `cpu.BLT.`, `cpu.BGE.`, `cpu.BLE.`, `cpu.BGT.`, `cpu.BUSC.`, `cpu.BESC.` and `cpu.NOPP.`. No bare `cpu.` prefix was used. This is the same pattern as the direct rows in plan 02-10.
- **Committed in:** 9255c42, a8266bf, a831fdc

**2. [Rule 3 - Files] Header and decode table edited**
- **Found during:** Task 1
- **Issue:** The plan lists only `cp1610.c`. The handler needs an id in `cp1610.h`, and the rows need it in `cp1610_decode.c`. Plans 02-08 to 02-10 did the same.
- **Fix:** `cp1610.h` was edited once, and `cp1610_decode.c` in every task.

---

**Total deviations:** 2 auto-fixed (1 prefix extension, 1 file list).
**Impact on plan:** None on scope. Every acceptance count holds:
- 5 `"cpu.B.` ids and 2 Task 1 fault lines
- 0 single-flag rows pending and 32 single-flag direction ids
- 0 of the 16 rows pending, and BEXT and J still pending (2)
- 29 POL-BR-01 vectors, 56 direction ids and 92 ids
- `faults=34/34`, and the gate passes under sh and dash

## Measured cost (D-22 budget input)

- Final `sh tools/gate.sh`: 36.5 s wall, unpiped. The summary line is `gate: PASS steps=24/24 controls=182/182 ... vectors=302 source=263 policy=39 rows-pending=2 policies-pending=14 bus-figure-pages=25 ... faults=34/34 ... tests-expected=646 tests-executed=646 tests-skipped=0 tests-missing=0`.
- `dash tools/gate.sh`: 34 s, `gate: PASS`.
- Plan 02-10 measured 35 s. The 6 extra fault executables and 92 extra vectors added little or nothing (36.5 s).

## Issues Encountered

None.

## Known Stubs

- BEXT and J still report the temporary UNIMPLEMENTED fault through `IVS_CP1610_OP_NONE`. This is plan design: the pending list tracks both rows and the gate counts them (`rows-pending=2`), so they are not added to `.planning/WINDOWS.md`.

## User Setup Required

None.

## Next Phase Readiness

- **Plan 02-12 (BEXT):**
  - BEXT is 1000 S1 EEEE, so `ivs_branch` already handles its displacement read, direction and cycles.
  - Only the condition source changes: call `port->ebci(ctx, E)`, with NULL meaning false (C57).
  - The fault lines already list `cpu.BEXT.` (BRANCH_BACKWARD_SIGN, BRANCH_TAKEN_CYCLES) and `cpu.BEXT.cycles-not-taken` (BRANCH_NOT_TAKEN_CYCLES).
  - IMMEDIATE_PC_ADVANCE will need `cpu.BEXT.` added.
  - Plan 02-12 brings POL-BR-01 to 31 vectors.
- **Plan 02-12 (J):** a jump is three words, and its target is assembled from fields, not a displacement. It will not reuse `ivs_branch`.
- **Plan 02-14:** the branch rows are interruptible, so any branch encoding can serve as an interruptible-boundary witness.
- **Unasserted inference:** the bus lists stop at the displacement BAR. Any later idle cycles of a taken branch (the 2 extra) are not placed on the bus.

---
*Phase: 02-cp-1610-cpu-core*
*Completed: 2026-11-10*

## Self-Check: PASSED

- The 3 key files exist: `branch.json`, `cp1610.c` and this SUMMARY.
- Commits `9255c42`, `a8266bf` and `a831fdc` are ancestors of HEAD. The ledger measures `git rev-list --count 9b38a07..HEAD` = 3.
- The acceptance commands of all three tasks were re-run and printed the expected values (5; 2; 0; 32; 0 and 2; 29; 56 and 92).
- Final gate: `sh tools/gate.sh` gate: PASS (36.5 s) and `dash tools/gate.sh` gate: PASS (34 s).
