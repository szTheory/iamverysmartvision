---
phase: 02-cp-1610-cpu-core
plan: 10
subsystem: cpu-core
tags: [c17, cp1610, direct, indirect, stack, auto-increment, adar, vectors, named-faults, cpu-01, cpu-04, cpu-05]

requires:
  - phase: 02-cp-1610-cpu-core
    plan: 09
    provides: ivs_imm_addr / ivs_imm_read (EA = PC+1 with the PC advance), IVS_DATA_BAR_OFFSET, ivs_op_add / ivs_op_sub, ivs_wreg and the immediate-row vectors
provides:
  - the 16 memory rows MVO MVI ADD SUB CMP AND XOR (direct), MVO@ MVI@ ADD@ SUB@ CMP@ AND@ XOR@ (indirect), PSHR and PULR
  - direct-mode bus order FETCH, READ of the address word at 4, ADAR of the effective address at 6 (pdf 30, pdf 11)
  - ivs_ind_addr / ivs_ind_read / ivs_ind_write / ivs_ind_op (C17 stack direction, R4/R5 post-increment, 16-bit wrap, POL-STK-02)
  - ivs_read_op, one operation routine for every read row keyed by the Fig 3.2.1 op field (bits 8..6)
  - POL-MVO-01 refusal ($264 $26D $276 $27F) as a sticky UNSUPPORTED_MVO_SSS_EQ_MMM fault before any effect
  - data/cp1610/vectors/direct.json (group direct, 19 vectors) and data/cp1610/vectors/indirect.json (group indirect, 35 vectors); cpu.MVOI.sss7-unsupported in immediate.json
  - 8 named faults PUSH_DIRECTION PULL_DIRECTION DIRECT_ADAR_ORDER AUTOINC_R4R5_MISSING AUTOINC_R1R3 MMM6_BONUS_MISSING STK_RESULT_LOSES MVO_SSS_EQ_MMM_ALLOWED
affects: [02-11, 02-12, 02-13, 02-14, 02-15, 02-16, 02-17]

actuals:
  tokens: 40801
  tasks: 3
  commits: 3
plan_head_before: ceaa3a45428a664d50805de6686c7a7d02c4bf5a
plan_head_after: 45d35af2915ca76dbfc34b380bba12994a6a5869

tech-stack:
  added: []
  patterns:
    - "Every read row (immediate, direct, indirect, PULR) ends in ivs_read_op, keyed by the op field of Fig 3.2.1 (pdf 46); the addressing helper decides only where the operand comes from"
    - "One pointer routine, ivs_ind_addr, owns every pointer update and the four stack/auto-increment fault sites"
    - "Refused encodings are rejected after the fetch through the same sticky-fault path as UNIMPLEMENTED: PC stays, the fetch is traced and counted, nothing else changes"
    - "Vector JSON is laid out by a scratch formatter from hand-written values; it checks hex against binary, flag tags against the hand values and the email trap, and never computes a result"

key-files:
  created:
    - data/cp1610/vectors/direct.json
    - data/cp1610/vectors/indirect.json
  modified:
    - core/src/cpu/cp1610.h
    - core/src/cpu/cp1610.c
    - core/src/cpu/cp1610_decode.c
    - data/cp1610/vectors/immediate.json
    - data/expected/cp1610-vector-files.ids
    - data/expected/cp1610-vectors.ids
    - data/expected/tests/cpu-vectors.ids
    - data/expected/cp1610-coverage-pending.ids
    - data/expected/faults/cpu.ids

key-decisions:
  - "Direct read rows record FETCH at 0, a READ of the address word at PC+1 at 4 and an ADAR of the effective address at 6 carrying the operand (figure basis, pdf 30; ADAR meaning pdf 11)"
  - "Direct MVO records its WRITE at the ADAR address phase, offset 6 (DW 8, DWS 9, NACT 10 = the printed 11); no figure prints a direct write, so its bus list is inference and the plan's cpu.MVO.write-figure became cpu.MVO.write-sequence"
  - "A pull (PULR and the @ read rows with MMM = 6) puts its READ at offset 7: Fig 3.2.4 orders the decrement before EA <- (MMM), so the 3 extra cycles are taken to come first. Inference (C36); no vector asserts the offset"
  - "PULR keeps its printed 11 as the row base; the +3 is added only to the 8-cycle @ rows, and cpu.PULR.decrement-then-read (cycles:base, 11) is the A-13 witness for MVI@ cycles:mod"
  - "POL-STK-02 (C55): the pointer moves before the operation, so the result is the final register value and ADD@ R4,R4 adds the old-R4 word to R4+1"
  - "MVO@ R1,R1 (SSS = MMM = 1) is supported: the pdf 55 note excludes only 4, 5, 6 and 7"
  - "requirements-completed stays empty as in plans 02-02..02-09: CPU-01, CPU-04 and CPU-05 advance; later plans in this phase still declare them"

patterns-established:
  - "A new addressing mode reuses the PC-advance helper (ivs_imm_read) and extends the IMMEDIATE_PC_ADVANCE prefix rather than adding a second PC-advance site"

requirements-completed: []
requirements-advanced: [CPU-01, CPU-04, CPU-05]

coverage:
  - id: D1
    description: "PSHR writes at R6 then increments it and PULR decrements R6 then reads (C17), with $FFFF/$0000 wrap and a push-then-pull round trip, each guarded by a direction fault (PUSH_DIRECTION, PULL_DIRECTION)"
    requirement: CPU-04
    verification:
      - kind: integration
        ref: "sh tools/gate.sh (cp1610-vectors pass vectors=210; faults-verdict pass faults=28/28)"
        status: pass
    human_judgment: false
  - id: D2
    description: "The 7 direct rows report FETCH, the address-word READ and the ADAR operand access in pdf 30 order and pass 19 page-derived vectors; DIRECT_ADAR_ORDER fails all 16 read-row vectors that carry a bus list"
    requirement: CPU-05
    verification:
      - kind: integration
        ref: "sh tools/gate.sh (cpu-vectors executed=462; build/gate/faults/DIRECT_ADAR_ORDER/cpu-vectors.txt)"
        status: pass
    human_judgment: false
  - id: D3
    description: "MVO@, MVI@ and the five ALU@ rows meet their derived coverage (addr:indirect, addr:autoinc, addr:stack, cycles:base/mod with the PULR witness, flags, bus); POL-STK-02 and POL-MVO-01 have policy vectors and faults; rows-pending=18"
    requirement: CPU-01
    verification:
      - kind: integration
        ref: "sh tools/gate.sh and dash tools/gate.sh (gate: PASS steps=24/24 rows-pending=18 policies-pending=15 faults=28/28)"
        status: pass
    human_judgment: false

duration: 12min
completed: 2026-11-10
status: complete
---

# Phase 2 Plan 10: Direct, Indirect and Stack Groups Summary

**The 16 memory rows now run. Direct mode reads its address word and then the operand through an ADAR access, in the order of the pdf 30 figure. The @ rows use R1-R3 unchanged, R4/R5 post-increment and R6 as a stack that grows upward (C17). The result wins over the pointer update (POL-STK-02). MVO@ with SSS = MMM in 4..7 is refused with a sticky fault (POL-MVO-01). 55 hand-derived vectors cover the rows and 8 named faults guard them.**

## Performance

- **Duration:** about 12 min
- **Started:** 2026-11-10T07:55:59Z
- **Completed:** 2026-11-10T08:08Z
- **Tasks:** 3 of 3
- **Files modified:** 11 (4547 insertions, 41 deletions)

## Accomplishments

- **Stack and pointers.** `ivs_ind_addr` follows Fig 3.2.4 (pdf 48) and S3.4.6 (pdf 62):
  - MMM 1-3 leaves the register alone.
  - MMM 4-5 uses the register, then adds 1.
  - MMM 6 on an output (MVO@, PSHR) writes at R6, then adds 1.
  - MMM 6 on an input subtracts 1 first, then reads at R6.
  - Every update wraps at 16 bits.
- **Direct mode.** The address word at PC+1 is read through `ivs_imm_read`, which advances the PC. The operand then goes through `ivs_access` as kind ADAR at offset 6 with the full 16-bit effective address (C51). Direct MVO writes at that address phase. It runs 11 cycles and is not interruptible (C56).
- **One operation routine.** `ivs_read_op` takes the op field (bits 8..6, Fig 3.2.1, pdf 46):
  - MVI loads.
  - ADD goes through `ivs_op_add`. SUB and CMP go through `ivs_op_sub`, and CMP stores nothing (C46).
  - AND and XOR change S and Z only.
- **Cycles.** The @ read rows add 3 when MMM = 6. PULR keeps its printed 11.
- **POL-STK-02.** `ivs_ind_op` updates the pointer first and then writes the result, so MVI@ R4,R4 and PULR R6 end with the loaded word. ADD@ R4,R4 adds the word at the old R4 to R4+1.
- **POL-MVO-01.** `ivs_mvo_refused` catches 1001 MMM SSS with SSS = MMM in 4..7 before the handler runs. PC stays at the offender, the 4-cycle fetch is traced and counted, and nothing is written. The covered decles are MVO@ $264 and $26D, PSHR R6 $276 and MVOI R7 $27F.
- **Vectors.** `direct.json` has 19 and `indirect.json` has 35, plus `cpu.MVOI.sss7-unsupported` in `immediate.json`. All 55 passed against the core on their first run.
- **Gate.** The pending list went from 34 rows to 18 (J and the 17 branches), and policies-pending went from 18 to 15. The gate now runs 210 vectors, 462 tests and 28 faults.

## Task Commits

1. **Task 1: Tracer, PSHR and PULR stack round trip** - `d4f78ea` (feat). This added 5 stack vectors and the PUSH_DIRECTION and PULL_DIRECTION faults. Tracer gate: `sh tools/gate.sh` re-ran green (`faults=22/22 rows-pending=32`), so expansion went ahead.
2. **Task 2: The direct rows with ADAR and DIRECT_ADAR_ORDER** - `0311dbd` (feat). This added 19 vectors and gave `faults=23/23 rows-pending=25`.
3. **Task 3: MVI@, MVO@ and the ALU@ rows with five faults** - `45d35af` (feat). This added 31 vectors and gave `faults=28/28 rows-pending=18`.

**Plan metadata:** recorded in the docs(02-10) completion commit.

## Bus offsets and the pages that print them

| Access | Offsets | Basis | Page |
|--------|---------|-------|------|
| Direct read rows (MVI ADD SUB CMP AND XOR) | FETCH 0; READ of the address word at PC+1, 4; ADAR of the effective address, 6 (data at 8); 10 cycles | figure | pdf 30, "Instruction fetch & data access (direct addr. mode)": BAR, NACT, DTB, NACT, BAR, NACT, ADAR, NACT, DTB; ADAR meaning from pdf 11 |
| Direct MVO | FETCH 0; READ of the address word, 4; WRITE at the ADAR phase, 6 (DW 8, DWS 9); 11 cycles | inference | pdf 30 address phases combined with the pdf 31 write tail; no direct-write figure exists |
| @ reads with MMM 1-5 | FETCH 0; READ, 4 (data at 6); 8 cycles | figure | pdf 29, indirect reg. mode |
| MVO@ and PSHR writes | FETCH 0; WRITE, 4 (DW 6, DWS 7); 9 cycles | figure | pdf 31, MVO timing (write operation) |
| Pull reads (PULR, @ rows with MMM = 6) | FETCH 0; READ, 7; 11 cycles | inference | no figure; Fig 3.2.4 (pdf 48) puts the decrement before EA, so the 3 extra cycles are placed before the BAR (C36) |
| Refused MVO encodings | FETCH 0; 4 cycles | figure | pdf 28, instruction fetch |

## Vector disagreements and resolutions

None against the core. All 55 vectors passed on their first run. No `expect` was edited after a run, and no core change was made to match a vector.

The formatter's tag check caught two authoring slips before any run. Neither was a disagreement with the core:

- The first `cpu.AND.mask` draft listed OV and C in `expect.status`, so its `flag-preserved` tags could not be proven. The expect now lists only S and Z, the AND row's status column. The same fix went into the three other AND/XOR vectors with S/Z-only expects.
- The values themselves did not change.

The plan 02-17 second reader should re-derive these readings independently:

- **C17 direction.** These vectors separate push from pull:
  - `cpu.PSHR.write-then-increment`: writes $02F0 and gives R6 = $02F1.
  - `cpu.PULR.decrement-then-read`: reads $02F0 from R6 = $02F1. $02F1 is never initialised, so a post-decrement fails as an uninitialised read.
  - The two wrap vectors.
- **POL-STK-02 (C55).** The policy vectors are:
  - `cpu.MVI@.ddd-equals-pointer-r4`: R4 = $1234, not $0301.
  - `cpu.PULR.ddd-r6-result-wins`: R6 = $4000.
  - `cpu.ADD@.r4-r4-sees-increment`: $0010 + $0301 = $0311.
- **C46 (CMP direction).** It applies to `cpu.CMP.no-write` and `cpu.CMP@.r1-no-write`, both $0005 against a $0003 word.
- **C51.** `cpu.MVI.full-16-bit-address` reads $C123, where a 10-bit address would be $0123.
- **MVO@ R1,R1.** `cpu.MVO@.r1-sss-eq-mmm-allowed` is supported: the pdf 55 note excludes only 4..7.
- **Inferred offsets.** Direct MVO's WRITE at 6 and the pull READ at 7 have no printed figure. Their vectors carry `bus_basis: inference`, so the offsets are not asserted.

## Fault verdicts (latest gate run)

| Fault | Vectors that fail (all inside the allowed prefixes) |
|-------|------------------------------------------------------|
| PUSH_DIRECTION | PSHR write-then-increment, r6-wrap-ffff, then-PULR-round-trip |
| PULL_DIRECTION | the 3 PULR vectors, the round trip and the 5 ALU@ r6-mmm6-bonus vectors |
| DIRECT_ADAR_ORDER | all 16 direct read-row vectors (every one carries a bus list) |
| AUTOINC_R4R5_MISSING | 11 R4/R5 vectors of MVO@, MVI@ and the ALU@ rows. `cpu.MVI@.ddd-equals-pointer-r4` passes because the result overwrites R4 either way |
| AUTOINC_R1R3 | 9 R1-R3 vectors, including `cpu.MVO@.r1-sss-eq-mmm-allowed` |
| MMM6_BONUS_MISSING | the 5 ALU@ r6-mmm6-bonus vectors |
| STK_RESULT_LOSES | `cpu.MVI@.ddd-equals-pointer-r4`, `cpu.PULR.ddd-r6-result-wins`, `cpu.ADD@.r4-r4-sees-increment` |
| MVO_SSS_EQ_MMM_ALLOWED | the 4 refusal vectors (MVO@ $264 and $26D, PSHR $276, MVOI $27F) |
| SUB_CARRY (existing) | now also every SUB, CMP, SUB@ and CMP@ vector |
| IMMEDIATE_PC_ADVANCE (existing) | now also all 19 direct vectors (the address word goes through `ivs_imm_read`) |
| ADD_OVERFLOW (existing) | now also `cpu.ADD.7fff-plus-1-overflow`, `cpu.ADD.ffff-plus-1`, `cpu.ADD@.r4-autoinc` and `cpu.ADD@.r6-mmm6-bonus` (prefixes were already listed) |

Every must-pass vector passes under its fault. They are `cpu.MVII.r1-load`, `cpu.PSHR.write-then-increment`, `cpu.MVI@.r2-no-increment`, `cpu.MVI@.r4-autoinc`, `cpu.ADD@.r2-no-increment` and `cpu.MVO@.r4-autoinc`.

## A-13 witness

The witness line is `MVI@ cycles:mod PULR`, in `data/expected/cp1610-modifier-witness.ids`. The PULR vector that meets it is `cpu.PULR.decrement-then-read`: single step, tagged `cycles:base`, `expect.cycles` 11 = MVI@ base 8 + 3. No vector labels an MMM = 6 decle as MVI@, so `cpu.MVI@.r6-mmm6-bonus` does not exist. MVI@ is off the pending list, and the gate passes with only `cycles:base` vectors for it.

## Decisions Made

- **Handler ids.** PSHR is 41 and PULR 42. MVO, MVI, ADD, SUB, CMP, AND and XOR are 43-49. The @ rows are 50-56, named `IVS_CP1610_OP_<ROW>_AT`, because `@` cannot appear in a C identifier. All ids sit above `IVS_CP1610_OP_ADCR`, so R7_CYCLE_BONUS_ALL's handler range does not reach them.
- **Address word.** The direct address word goes through `ivs_imm_read`, the one PC-advance site. IMMEDIATE_PC_ADVANCE therefore covers direct mode too, and its prefix gained the seven direct stems.
- **Register read order.** MVO@ and PSHR read register SSS before the pointer moves. Direct MVO reads SSS after the address word, as MVOI does. Neither order is visible in an allowed encoding, because SSS = MMM in 4..7 is refused and SSS = 7 is not vectored.
- **Flag exemptions.** None. Every printed flag of the 14 flag-bearing new rows can be both set and cleared, so `cp1610-flag-exempt.ids` is unchanged.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Ordering] PULL_DIRECTION landed in Task 1, not Task 3**
- **Found during:** Task 1
- **Issue:** The PULR pre-decrement lives in the shared `ivs_ind_addr`, so its fault site exists from Task 1. A site without an inventory line fails `fault-orphan-site`, and the gate was red.
- **Fix:** The plan's exact PULL_DIRECTION line was added in Task 1. Task 2's gate therefore reads `faults=23/23`, not the plan's 22/22. Task 3 ends at the planned `faults=28/28`.
- **Committed in:** d4f78ea

**2. [Rule 1 - Honest basis] `cpu.MVO.write-figure` became `cpu.MVO.write-sequence` with `bus_basis: inference`**
- **Found during:** Task 2, re-reading pdf 31 as the 02-09 hand-off asked
- **Issue:** The pdf 31 figure is the 9-cycle write with no address word and no ADAR. It matches MVO@, PSHR and MVOI, not the 11-cycle direct MVO. A figure-basis direct-MVO vector would assert extrapolated offsets.
- **Fix:** The vector combines the pdf 30 address phases with the pdf 31 write tail. Its derivation shows that this gives the printed 11 cycles. It is tagged inference, so its offsets are not asserted, and it is renamed so the id does not claim a figure. No fault line or later plan names the old id.
- **Committed in:** 0311dbd

**3. [Rule 3 - Fault prefix] Existing faults gained row stems**
- **Found during:** Tasks 2 and 3
- **Issue:** The new rows share `ivs_op_sub` (SUB_CARRY) and `ivs_imm_read` (IMMEDIATE_PC_ADVANCE).
- **Fix:** Following the plan 02-08 rule, SUB_CARRY gained `cpu.SUB.|cpu.CMP.|cpu.SUB@.|cpu.CMP@.`. IMMEDIATE_PC_ADVANCE gained `cpu.MVO.|cpu.MVI.|cpu.ADD.|cpu.SUB.|cpu.CMP.|cpu.AND.|cpu.XOR.`.
- **Committed in:** 0311dbd, 45d35af

**4. [Rule 3 - Files] Header and decode table edited**
- **Found during:** Task 1
- **Issue:** The plan lists only `cp1610.c`, but each row needs a handler id in `cp1610.h` and its handler in the `cp1610_decode.c` row. This is the same pattern as plans 02-08 and 02-09.
- **Fix:** Both files were edited in every task.

**Smaller choices, no behaviour risk:**
- Vector counts are 19 direct (the plan said about 21) and 35 indirect (about 45). Every row meets its derived coverage. Extra cases are `cpu.MVO@.r1-sss-eq-mmm-allowed`, `cpu.MVO@.sss-eq-mmm-r5-unsupported`, `cpu.MVI@.r1-read` and `cpu.ADD@.r5-wrap-ffff`.

---

**Total deviations:** 4 auto-fixed (1 ordering, 1 honest-basis rename, 1 prefix extension, 1 file list).
**Impact on plan:** Every acceptance count holds:
- 5 PSHR/PULR ids, 1 PUSH_DIRECTION line, 0 POL-STK-01 pending
- 0 direct rows pending, 16 `"ADAR"` strings in direct.json
- `rows-pending=18`, 0 of POL-STK-01/02 and MVO-01 pending
- 0 `cpu.MVI@.r6-mmm6-bonus`, 4 unsupported vectors
- `faults=28/28`, and the gate passes under sh and dash

## Measured cost (D-22 budget input)

- Final `sh tools/gate.sh`: 35 s wall, unpiped. The summary line is `gate: PASS steps=24/24 controls=182/182 ... vectors=210 source=200 policy=10 rows-pending=18 policies-pending=15 ... faults=28/28 ... tests-expected=462 tests-executed=462 tests-skipped=0 tests-missing=0`.
- `dash tools/gate.sh`: 37 s, `gate: PASS`.
- Plan 02-09 measured 33.0 s. The 8 extra fault executables and 55 extra vectors add about 2 s.

## Issues Encountered

None.

## Known Stubs

- The 18 rows still pending (J and the 17 branches) report the temporary UNIMPLEMENTED fault through their `IVS_CP1610_OP_NONE` handler. This is plan design. The pending list tracks it and the gate counts it (`rows-pending=18`), so it is not added to `.planning/WINDOWS.md`.

## User Setup Required

None.

## Next Phase Readiness

- **Plan 02-11/02-12:** the memory rows are done. Branches and J are the remaining handlers.
- **Plan 02-13 (SDBD):** the six @ read rows already go through `ivs_ind_op` / `ivs_ind_read`, so the double-byte path hooks in there for MMM 1-5. The immediate path is still `ivs_imm_read`. Refusal of MMM 0 and 6, MVO@, MVOI and the other targets can sit beside `ivs_mvo_refused` in `ivs_unit_instr`.
- **Plan 02-14:** the MVO family is non-interruptible from the decode table (C56). The `defers-intr` vectors can use any of the new MVO, MVO@ and PSHR encodings.
- **Plan 02-15 (`cpu.MVI.wait-on-adar`):** the ADAR access already goes through `ivs_access`, and ADAR is one of the kinds that take waits. A wait on the effective address shifts the data phase and the unit length.
- **Unasserted inferences:**
  - MVO, MVO@ and the direct ALU rows with register 7 read R7 as the advanced PC (PC+2 for direct, PC+1 for @). No vector asserts this.
  - An @ read row with DDD = 7 writes through `ivs_wreg`. A later vector of that kind must add its exact id to the MOVR_R7_PC prefix.

---
*Phase: 02-cp-1610-cpu-core*
*Completed: 2026-11-10*

## Self-Check: PASSED

- The 3 key files exist: `direct.json`, `indirect.json` and this SUMMARY.
- Commits `d4f78ea`, `0311dbd` and `45d35af` are ancestors of HEAD. The ledger measures `git rev-list --count ceaa3a4..HEAD` = 3.
- The acceptance commands of all three tasks were re-run and printed the expected values.
- Final gate: `sh tools/gate.sh` gate: PASS (35 s) and `dash tools/gate.sh` gate: PASS (37 s).
