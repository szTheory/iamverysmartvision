---
phase: 02-cp-1610-cpu-core
plan: 08
subsystem: cpu-core
tags: [c17, cp1610, decode-table, register-group, vectors, named-faults, cpu-01]

requires:
  - phase: 02-cp-1610-cpu-core
    plan: 07
    provides: the purity step and core-symbols suite every core change must keep green; the stepper, vector generator, harness and fault machinery of plans 02-02..02-06
provides:
  - complete 74-row decode table in core/src/cpu/cp1610_decode.c (row id, mask, value, eq, words, base cycles, interruptible, handler); every 10-bit decle maps to exactly one row
  - ivs_cp1610_row now carries `id` (row id string) and `eq` (bits 5..3 == bits 2..0, for TSTR and CLRR)
  - register-group semantics for the 14 rows ADDR SUBR CMPR ANDR XORR CLRR INCR DECR COMR NEGR ADCR MOVR TSTR JR
  - shared ALU adder ivs_alu_add (S, Z, C, OV per pdf 45), ivs_op_add / ivs_op_sub family helpers, ivs_wreg register write (R7 write overrides the PC)
  - ivs_exec returns the cycles added to the row's base count (MOVR/TSTR R6/R7 bonus)
  - data/cp1610/vectors/register.json (group register, 62 hand-derived vectors)
  - 8 named faults ADD_OVERFLOW SUB_CARRY CMP_WRITES_DEST ADCR_IGNORES_C INCR_TOUCHES_C NEGR_CARRY R7_CYCLE_BONUS_ALL MOVR_R7_PC
  - flag exemptions CLRR/flag-clear:Z, CLRR/flag-set:S, ADCR/flag-set:C
affects: [02-09, 02-10, 02-11, 02-12, 02-13, 02-14, 02-15, 02-16, 02-17]

actuals:
  tokens: 39705
  tasks: 3
  commits: 3
plan_head_before: ab7ed27fd74488a331d78a64570adb6c3fe1e232
plan_head_after: 61cebbd4c1e32909c8adc2729eac71c4689a73e4

tech-stack:
  added: []
  patterns:
    - "Decode is one ordered table, most specific row first; a row whose semantics have not landed carries handler IVS_CP1610_OP_NONE and reports the temporary UNIMPLEMENTED fault"
    - "Rows that need equal register fields (TSTR, CLRR) carry an eq flag instead of being expanded into 8 mask/value rows"
    - "Family helpers own their family's fault site (ADD_OVERFLOW in ivs_op_add, SUB_CARRY in ivs_op_sub) so later ADD/SUB/CMP forms inherit the fault without new sites"
    - "Vector JSON is laid out by a scratch formatter from hand-written values; the formatter checks the hex/binary transcription and labels flag tags from the hand values, and never computes a result"

key-files:
  created:
    - data/cp1610/vectors/register.json
  modified:
    - core/src/cpu/cp1610.h
    - core/src/cpu/cp1610.c
    - core/src/cpu/cp1610_decode.c
    - data/expected/cp1610-vector-files.ids
    - data/expected/cp1610-vectors.ids
    - data/expected/tests/cpu-vectors.ids
    - data/expected/cp1610-coverage-pending.ids
    - data/expected/cp1610-flag-exempt.ids
    - data/expected/faults/cpu.ids

key-decisions:
  - "Decle $0BF (0010 111 111) decodes to the TSTR row (first match); TSTR R7, JR R7 and MOVR R7,R7 run the same move and give R7 = PC+1, S/Z from it, 7 cycles"
  - "A register read of R7 sees the PC already incremented by the fetch (inference from pdf 28 and POL-STK-01, recorded in each derivation that depends on it)"
  - "CMPR computes DDD - SSS (C46 default), not stored; NEGR is 0 + ~DDD + 1 and ADCR is DDD + 0 + C with C and OV from that addition (C49 default)"
  - "ADCR/flag-set:C is exempt: with C = 0 the only carry in is 0, so DDD + 0 cannot carry out"
  - "requirements-completed stays empty as in plans 02-02..02-07: CPU-01 is advanced (14 more rows) and completed when the last row leaves the pending list"

patterns-established:
  - "Every register write of the register group goes through ivs_wreg, so the R7-as-PC rule has one site"
  - "Negative cycle-bonus vectors (R6/R7 operands on rows without the footnote) sit on every ALU row so R7_CYCLE_BONUS_ALL fails broadly inside its prefixes"

requirements-completed: []
requirements-advanced: [CPU-01]

coverage:
  - id: D1
    description: "Complete 74-row decode table; every one of the 1024 decles maps to a row that is a leaf of the Phase 1 family table, with matching words and base cycles (scratch cross-check, 0 differences)"
    requirement: CPU-01
    verification:
      - kind: integration
        ref: "sh tools/gate.sh (cp1610-rows=74 patterns=1024); scratch decode dump of all 1024 decles compared with data/cp1610/opcode-families.json"
        status: pass
    human_judgment: false
  - id: D2
    description: "The 14 register rows pass 62 page-derived vectors meeting their derived coverage (flag set/clear/preserved, cycles base/mod, R7 as destination, bus list) and are off the pending list (rows-pending=49)"
    requirement: CPU-01
    verification:
      - kind: integration
        ref: "sh tools/gate.sh (-- cp1610-vectors pass vectors=89 ... rows-pending=49; tests-executed=220)"
        status: pass
    human_judgment: false
  - id: D3
    description: "Eight named faults each fail their must-fail vector only inside their allowed prefixes and keep their must-pass vector green (faults=14/14)"
    requirement: CPU-01
    verification:
      - kind: integration
        ref: "sh tools/gate.sh (-- faults-verdict pass faults=14/14); build/gate/faults/<ID>/cpu-vectors.txt"
        status: pass
    human_judgment: false

duration: 19min
completed: 2026-11-10
status: complete
---

# Phase 2 Plan 08: Decode Table and Register Group Summary

**The decode table now covers all 74 rows and every 10-bit decle. The 14 register-register and single-register rows run with S/Z/C/OV per pdf 45. 62 hand-derived vectors cover them, including the R6/R7 cycle-bonus boundary and R7 as destination, and eight named faults guard them.**

## Performance

- **Duration:** about 19 min
- **Started:** 2026-11-10T07:08:37Z
- **Completed:** 2026-11-10T07:27Z
- **Tasks:** 3 of 3
- **Files modified:** 10 (4397 insertions, 48 deletions)

## Accomplishments

- **Decode table.** `cp1610_decode.c` holds 74 rows, most specific first, transcribed from Fig 3.3.1-3.3.5 (pdf 50-52) and the tables on pdf 53-58. Each row records its id, mask, value, eq flag, words, base cycles, interruptible flag and handler. `ivs_cp1610_decode` checks `eq` for TSTR and CLRR. A `_Static_assert` pins the row count at 74. The 52 rows without semantics carry `IVS_CP1610_OP_NONE` and report the temporary UNIMPLEMENTED fault through the same path unknown decles used before (`row == NULL || handler == OP_NONE`).
- **Register group.**
  - `ivs_alu_add(a, b, cin)` computes in a `uint32_t` temporary. C is bit 16 of the sum, OV uses the sign rule `~(a^b) & (a^sum) & 0x8000`, and S and Z come from the result.
  - ADDR goes through `ivs_op_add`. SUBR and CMPR go through `ivs_op_sub` (`a + ~b + 1`), and CMPR does not store its result.
  - NEGR is `ivs_alu_add(0, ~DDD, 1)` and ADCR is `ivs_alu_add(DDD, 0, C)`.
  - ANDR, XORR, CLRR, INCR, DECR and COMR update S and Z only.
  - MOVR, TSTR and JR share one move: MOVR adds 1 cycle when DDD is 6 or 7, TSTR adds 1 when SSS is 6 or 7, and JR is a flat 7.
  - Every write goes through `ivs_wreg`, so an R7 write is the new PC.
- **Vectors.** `register.json` (group `register`) has 62 vectors. Each row has set and clear cases for its printed flags and preserved cases for the others. Edge operands are 0000, 0001, 7FFF, 8000 and FFFF. Each vector carries a figure-basis FETCH bus list (pdf 28) and its C46/C49 reading where one applies. Every value was derived by hand. All 62 passed on the first run.
- **Faults.** There are eight new faults, for 14 in total. Their failing vectors are listed under "Fault verdicts" below.
- **Gate.** The pending list went from 63 rows to 49. The gate now runs 89 vectors and 220 tests.

## Task Commits

1. **Task 1: Tracer, the full 74-row decode table plus ADDR end to end** - `58bebe1` (feat). This added 7 ADDR vectors and ADD_OVERFLOW. Tracer gate: the gate re-ran green (`faults=7/7 rows-pending=62`), so expansion went ahead.
2. **Task 2: SUBR, CMPR, ANDR, XORR, CLRR, INCR, DECR, COMR, NEGR and ADCR with five faults** - `576bc15` (feat). This added 41 vectors and gave `faults=12/12 rows-pending=52`.
3. **Task 3: MOVR, TSTR and JR with the R6/R7 bonus and R7 as destination** - `61cebbd` (feat). This added 14 vectors and gave `faults=14/14 rows-pending=49`.

**Plan metadata:** recorded in the docs(02-08) completion commit.

## Decode rows and page cites (74)

The base cycles column is the printed count without modifiers. "NI" marks a row that is not interruptible.

| Group | Rows (value / mask, words, base cycles) | Cite |
|-------|------------------------------------------|------|
| Internal control | HLT $000 NI, SDBD $001 NI, EIS $002 NI, DIS $003 NI, TCI $005 NI, CLRC $006 NI, SETC $007 NI (mask $3FF, 1 word, 4) | Fig 3.3.4, pdf 51; control table, pdf 56; SDBD, pdf 57; HLT, SDBD NI by C56 |
| Jump | J $004 (mask $3FF, 3 words, 12) | Fig 3.3.5, pdf 52; jump table, pdf 56 |
| Single register | INCR $008, DECR $010, COMR $018, NEGR $020, ADCR $028, RSWD $038 (mask $3F8); GSWD $030 (mask $3FC); NOP $034, SIN $036 (mask $3FE); 1 word, 6 | Fig 3.3.3 and footnote, pdf 51; pdf 53; control table, pdf 56 |
| Shifts | SWAP $040, SLL $048, RLC $050, SLLC $058, SLR $060, SAR $068, RRC $070, SARC $078 (mask $3F8, 1 word, 6, NI) | Fig 3.3.2, pdf 50; shift table, pdf 53 |
| Register to register | TSTR $080 eq, JR $087 (mask $3C7, 7), MOVR $080, ADDR $0C0, SUBR $100, CMPR $140, ANDR $180, CLRR $1C0 eq, XORR $1C0 (mask $3C0, 1 word, 6) | Fig 3.3.1, pdf 50; register table, pdf 53 |
| Branches | B $200, BC $201, BOV $202, BPL $203, BZE $204, BLT $205, BLE $206, BUSC $207, NOPP $208, BNC $209, BNOV $20A, BMI $20B, BNZE $20C, BGE $20D, BGT $20E, BESC $20F (mask $3DF); BEXT $210 (mask $3D0); 2 words, 7 | Branch table, pdf 54 |
| Output | MVO $240 (2 words, 11), PSHR $270 (9), MVOI $278 (2 words, 9) (mask $3F8); MVO@ $240 (mask $3C0, 9); all NI by C56 | Direct and indirect tables, pdf 55; immediate table, pdf 57 |
| Input | MVI $280 (2 words, 10), PULR $2B0 (11), MVII $2B8 (2 words, 8) (mask $3F8); MVI@ $280 (mask $3C0, 8) | pdf 55; pdf 57 |
| ALU memory | ADD $2C0, SUB $300, CMP $340, AND $380, XOR $3C0 (mask $3F8, 2 words, 10); ADDI $2F8, SUBI $338, CMPI $378, ANDI $3B8, XORI $3F8 (mask $3F8, 2 words, 8); ADD@ $2C0, SUB@ $300, CMP@ $340, AND@ $380, XOR@ $3C0 (mask $3C0, 1 word, 8) | pdf 55; pdf 57 |

Readings that differ from `data/cp1610/opcode-families.json`: **none**. A scratch program dumped all 1024 decles and compared each with the family table's patterns, `eq` relations and `refines` leaves. In every case the decoded row is a leaf for that decle. Words and the leading printed cycle count agree. Rows the table marks `interruptible: no` are non-interruptible, and the only manual-silent rows set non-interruptible are HLT, SDBD, MVO, MVO@, PSHR and MVOI (C56). No discrepancy candidate arises. One open choice: decle $0BF decodes to TSTR because TSTR comes before JR. The plan 02-13 sweep should accept TSTR or JR for $0BF, as the family table's `also_encodes` does.

## Vector disagreements and resolutions

None. All 62 vectors passed against the core on the first run. No `expect` was edited after a run, and no core change was made to match a vector. The plan 02-17 second reader therefore has no recorded resolution from this plan to revisit. The inferences it should re-derive independently are:

- **C46 (CMP direction):** CMPR flags are those of DDD - SSS. `cpu.CMPR.no-write` uses $0005 - $0003, which separates the two operand orders by S and C.
- **C49 (NEGR and ADCR C and OV):** NEGR is 0 + ~DDD + 1 and ADCR is DDD + 0 + C, by analogy with S3.1.2.
- **R7 as a source is the incremented PC:** an inference from pdf 28 and POL-STK-01. It is used by `cpu.ADDR.r7-src-no-bonus`, `cpu.ADDR.r7-dest`, `cpu.MOVR.coincidence-0bf` and `cpu.TSTR.sss7-bonus`, and stated in each of their derivations.

## Fault verdicts (latest gate run)

| Fault | Vectors that fail (all inside the allowed prefixes) |
|-------|------------------------------------------------------|
| ADD_OVERFLOW | ADDR 7fff-plus-1-overflow, ffff-plus-1-carry-zero, same-register-doubles |
| SUB_CARRY | all 6 SUBR and all 5 CMPR vectors |
| CMP_WRITES_DEST | all 5 CMPR vectors |
| ADCR_IGNORES_C | ADCR 7fff-overflow, carry-in-one, ffff-carry-zero |
| INCR_TOUCHES_C | INCR 0000, 7fff-sign, ffff-c-preserved |
| NEGR_CARRY | NEGR 8000, c-from-carry-out, zero |
| R7_CYCLE_BONUS_ALL | ADDR r7-dest and r7-src-no-bonus; ANDR same-register; CLRR r6-no-bonus; COMR r6-no-bonus; INCR r6-no-bonus; SUBR r6-dst-no-bonus; XORR r6-dst-no-bonus |
| MOVR_R7_PC | ADDR r7-dest; JR negative-target, to-r7, zero-target; MOVR coincidence-0bf, ddd7-bonus; TSTR sss7-bonus |

Every must-pass vector passes. Those are `cpu.SETC.sets-c`, `cpu.ADDR.0001-plus-0001`, `cpu.SUBR.equal-operands-carry-zero`, `cpu.MOVR.base` and `cpu.TSTR.base`.

## Decisions Made

- The row struct gained `const char *id` and `uint8_t eq`. The plan asks for a row id in the table, and plan 02-13's sweep compares the decoded row id. `eq` expresses TSTR and CLRR without expanding them into 8 rows each.
- `ivs_exec` returns extra cycles, and the unit length is base + extra + waits. JR keeps its printed 7 as the base, so a row-based cycle check (the 02-13 sweep) sees 7 for every JR decle.
- Fault sites live in the family helpers (`ivs_op_add`, `ivs_op_sub`) and in `ivs_wreg`. Plans 02-11 and 02-12 then reach ADD/SUB/CMP and their R7 writes through the same sites, and they extend prefixes rather than add sites.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug in plan text] The coincidence decle is $0BF, not $2FF**
- **Found during:** Task 1 (transcribing pdf 53)
- **Issue:** The plan, CONTEXT POL-ILL-01 and 02-04 call the MOVR/TSTR/JR coincidence "$2FF". On pdf 53 it is 0010 111 111 = $0BF. $2FF is 10 1111 1111, which is in the external-reference space, where 1011 111 111 is ADDI with DDD = 7.
- **Fix:** The vector is `cpu.MOVR.coincidence-0bf`, with program `00BF 0010 111 111`, not `cpu.MOVR.coincidence-2ff`. This keeps the id debuggable from its name (CONTEXT specifics).
- **Downstream:** Plan 02-13 names `cpu.MOVR.coincidence-2ff-high-bits` and a DECODE_HIGH_BITS prefix `cpu.MOVR.coincidence-2ff-high`. Its executor should use `...coincidence-0bf-high-bits` (word for example $FCBF) and the matching prefix.
- **Committed in:** 61cebbd

**2. [Rule 1 - Bug in plan text] `cpu.MOVR.ddd7-bonus` is labelled JR and tagged `cycles:base`, not `cycles:mod`**
- **Found during:** Task 3
- **Issue:** Every MOVR encoding with DDD = 7 is the JR encoding 0010 SSS 111, and JR refines MOVR. The checker's most-specific rule therefore requires row JR (`vector-row-mismatch` otherwise). JR prints a plain 7 with no modifier, so `cycles:mod` would be `vector-tag-false`.
- **Fix:** The vector keeps id `cpu.MOVR.ddd7-bonus` and uses row JR, decle $0B7 (MOVR R6, R7 = JR R6), tags `cycles:base` (7) and `addr:r7-dest`. Its derivation shows that the MOVR reading (6 + 1) and the JR reading (7) agree. The MOVR `cycles:mod` requirement is met by `cpu.MOVR.ddd6-bonus`.
- **Committed in:** 61cebbd

**3. [Rule 2 - Missing coverage exemption] ADCR/flag-set:C**
- **Found during:** Task 2
- **Issue:** Under the C49 reading the carry in is the old C. A flag-set:C case needs init C = 0, and then DDD + 0 + 0 cannot carry out. The case is impossible.
- **Fix:** An exemption line with a reason comment. CLRR's two exemptions were added as the plan says.
- **Committed in:** 576bc15

**4. [Rule 3 - Fault prefix] MOVR_R7_PC also allows `cpu.TSTR.sss7-bonus`**
- **Found during:** Task 3
- **Issue:** TSTR R7 is the $0BF decle. It writes R7 with its own value, so the "PC incremented after the R7 write" fault legitimately breaks it.
- **Fix:** Following the plan's prefix rule, the exact stem `cpu.TSTR.sss7-bonus` was added to that fault's prefix column, not `cpu.TSTR.`. The must-pass `cpu.TSTR.base` is unaffected and passes.
- **Committed in:** 61cebbd

**Smaller choices, no behaviour risk:**
- There are 62 register vectors where the plan said "about 65". Extras include an R6-operand no-bonus case on most ALU rows, and `cpu.TSTR.sss6-bonus`. `cpu.TSTR.sss7-bonus` runs at $8000 so that it differs from the coincidence vector (S = 1).
- `cpu.ADDR.r7-dest` was added in Task 3 as the plan says, together with the other R7-destination vectors.

---

**Total deviations:** 4 auto-fixed (2 plan-text bugs, 1 missing exemption, 1 prefix extension).
**Impact on plan:** Every acceptance count holds: `rows-pending=49`, `faults=14/14`, 62 register ids (60 or more required), 7 ADDR ids, and every listed fault line. Only the coincidence vector's name differs from the plan.

## Measured cost (D-22 budget input)

- Full gate: `sh tools/gate.sh` 34.0 s wall (unpiped, final), `dash tools/gate.sh` 32.6 s, `steps=24/24 controls=182/182`. Plan 02-07 measured 35.3 s and 41.4 s. The 8 extra fault executables and 62 extra vectors add no visible time.

## Issues Encountered

None.

## Known Stubs

- The 52 rows still pending (J, the 8 shifts, the 17 branches and the 26 external-reference rows) decode to their row with handler `IVS_CP1610_OP_NONE`. They report the temporary UNIMPLEMENTED fault. This is plan design, not a defect: each later plan (02-09..02-15) adds handlers and removes rows from the pending list, and plan 02-13 removes the UNIMPLEMENTED reason. It is not added to `.planning/WINDOWS.md` because the pending list already tracks it and the gate counts it (`rows-pending=49`).

## User Setup Required

None.

## Next Phase Readiness

- Later groups add handler ids and cases only. The table, row flags (including interruptible for the interrupt plan) and decode order are final.
- **ADD, ADD@ and ADDI** should call `ivs_op_add`. **SUB, CMP and their @ and I forms** should call `ivs_op_sub`. CMP forms skip the write. Under ADD_OVERFLOW and SUB_CARRY, the new vectors then fail inside prefixes that already list `cpu.ADD.`, `cpu.ADD@.` and `cpu.ADDI.`. For SUB_CARRY, add `cpu.SUB.`, `cpu.SUB@.`, `cpu.SUBI.`, `cpu.CMP.`, `cpu.CMP@.` and `cpu.CMPI.` as their vectors land.
- Any new register write should use `ivs_wreg`. If a new vector writes R7 that way, extend the MOVR_R7_PC prefix with its exact id.
- **Plan 02-13:** use the `$0BF` coincidence name (deviation 1). The sweep's row-id check must accept TSTR or JR for $0BF.

---
*Phase: 02-cp-1610-cpu-core*
*Completed: 2026-11-10*

## Self-Check: PASSED

- The 3 key files exist: `register.json`, `cp1610_decode.c` and this SUMMARY.
- Commits `58bebe1`, `576bc15` and `61cebbd` are ancestors of HEAD. The ledger measures `git rev-list --count ab7ed27..HEAD` = 3.
- Acceptance counts were re-run:
  - 8 `cpu.ADDR.` ids
  - 1 ADD_OVERFLOW line
  - 0 of the 14 register rows pending
  - 5 Task 2 fault lines
  - 62 `cpu.` ids in register.json
- Final gate: `sh tools/gate.sh` gate: PASS steps=24/24 controls=182/182 faults=14/14 rows-pending=49 tests-executed=220 (34.0 s). `dash tools/gate.sh` passed the same (32.6 s).
