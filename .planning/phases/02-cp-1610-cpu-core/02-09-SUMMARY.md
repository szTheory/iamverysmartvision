---
phase: 02-cp-1610-cpu-core
plan: 09
subsystem: cpu-core
tags: [c17, cp1610, immediate, shifts, vectors, named-faults, cpu-01]

requires:
  - phase: 02-cp-1610-cpu-core
    plan: 08
    provides: the 74-row decode table, ivs_alu_add / ivs_op_add / ivs_op_sub, ivs_wreg and the register-group vectors and faults
provides:
  - immediate group MVII ADDI SUBI CMPI ANDI XORI MVOI, the first non-fetch bus traffic (READ or WRITE at PC+1, offset 4)
  - ivs_imm_addr / ivs_imm_read helpers (EA = PC+1, PC advances past the operand) for the memory and SDBD plans to reuse
  - ivs_shift for SWAP SLL RLC SLLC SLR SAR RRC SARC (N = 0 6 cycles, N = 1 8 cycles, not interruptible)
  - data/cp1610/vectors/immediate.json (group immediate, 23 vectors) and data/cp1610/vectors/shift.json (group shift, 43 vectors)
  - 6 named faults IMMEDIATE_PC_ADVANCE MVOI_ADDRESS SHIFT_N1_CYCLES RIGHT_SHIFT_S_BIT15 RRC_LINK_SWAP SWAP_N1_AS_N0
affects: [02-10, 02-11, 02-12, 02-13, 02-17]

actuals:
  tokens: 48596
  tasks: 3
  commits: 3
plan_head_before: 0d8b4b14f3cfd3206fa15b12f9fc91c9b2dd3809
plan_head_after: 5e8d8955d314bcea61ccdbfc96e41b39b8c85cd7

tech-stack:
  added: []
  patterns:
    - "Immediate mode is the data-access BAR at offset 4 with the effective address PC+1; one helper owns the PC advance and its fault site"
    - "One shift function computes result and link bits for all eight rows; a row writes C or OV only when its status column lists them for that N, so N = 0 keeps OV"
    - "Vector JSON is laid out by a scratch formatter from hand-written values (binary groups from the page format, hex prefixed by transcription); it checks flag tags against the hand values and never computes a result"

key-files:
  created:
    - data/cp1610/vectors/immediate.json
    - data/cp1610/vectors/shift.json
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
  - "Immediate reads and writes are figure-basis bus records at offset 4: pdf 29 (fetch and data access, indirect register mode) for the reads, pdf 31 (the 9-cycle MVO write) for MVOI, with EA = PC+1 from Fig 3.2.5 (pdf 49)"
  - "S after right shifts and SWAP is bit 7 of the result (C47); left shifts use bit 15"
  - "RLC N=1 rotates C OV R15..R0 (C bit 17, OV bit 16); RRC N=1 rotates OV C R15..R0 (C bit 16, OV bit 17), OV is an input (C48); N = 0 forms keep OV"
  - "SARC N=1 takes the RRC link layout (bit 0 to C, bit 1 to OV): the page names no positions for SARC, so this is an inference stated in every SARC N=1 derivation"
  - "SWAP N=1 leaves the register in its original form, as the page text says"
  - "requirements-completed stays empty: CPU-01 advances by 15 rows and completes when the last row leaves the pending list"

patterns-established:
  - "Every memory-operand row reads its operand with ivs_access at a named offset constant (IVS_DATA_BAR_OFFSET), never inline numbers"

requirements-completed: []
requirements-advanced: [CPU-01]

coverage:
  - id: D1
    description: "The 7 immediate rows read or write the word at PC+1 through the bus port at offset 4, advance PC past it, and pass 23 page-derived vectors (each with an addr:immediate tag and a figure-basis bus list)"
    requirement: CPU-01
    verification:
      - kind: integration
        ref: "sh tools/gate.sh (-- cp1610-vectors pass vectors=155 ... rows-pending=34)"
        status: pass
    human_judgment: false
  - id: D2
    description: "The 8 shift and SWAP rows pass 43 page-derived vectors with N=0 (6 cycles) and N=1 (8 cycles), the bit-7 S rule, the C48 link strings with initial OV = 1, and the N=0 OV-preserved cases"
    requirement: CPU-01
    verification:
      - kind: integration
        ref: "sh tools/gate.sh (tests-executed=352; vector-flag/cycles/bus coverage clean)"
        status: pass
    human_judgment: false
  - id: D3
    description: "Six new named faults each fail their must-fail vector only inside their allowed prefixes and keep their must-pass vector green (faults=20/20)"
    requirement: CPU-01
    verification:
      - kind: integration
        ref: "sh tools/gate.sh (faults=20/20); build/gate/faults/<ID>/cpu-vectors.txt"
        status: pass
    human_judgment: false

duration: 13min
completed: 2026-11-10
status: complete
---

# Phase 2 Plan 09: Immediate and Shift Groups Summary

**The seven immediate rows now read or write the word at PC+1 through the bus port at offset 4 and advance the PC past it. The eight shift and SWAP rows run with 6/8 cycles, S from bit 7 on right shifts and SWAP, and the C48 link-bit strings. 66 hand-derived vectors cover them and six named faults guard them.**

## Performance

- **Duration:** about 13 min
- **Started:** 2026-11-10T07:32:34Z
- **Completed:** 2026-11-10T07:45Z
- **Tasks:** 3 of 3
- **Files modified:** 10 (4825 insertions, 32 deletions)

## Accomplishments

- **Immediate path.**
  - `ivs_imm_addr` returns the current R7, which is PC+1 because the fetch already advanced it, and then moves R7 past the operand.
  - `ivs_imm_read` issues one READ at `IVS_DATA_BAR_OFFSET` (4) and keeps all 16 bits (C51).
  - MVII writes DDD through `ivs_wreg`, so MVII R7 makes the operand the new PC.
  - ADDI goes through `ivs_op_add` and SUBI through `ivs_op_sub`. CMPI is register SSS minus the operand and is not stored (C46). ANDI and XORI update S and Z only.
  - MVOI takes the address first, then issues a WRITE of register SSS at PC+1, offset 4. It runs 9 cycles, changes no flags and is not interruptible (C56).
- **Shifts.** `ivs_shift` handles the 0001 BLA N RR format (Fig 3.3.2, pdf 50) on R0-R3.
  - It returns 2 extra cycles when N = 1.
  - Left shifts (SLL, RLC, SLLC) take S from bit 15. Right shifts and SWAP take S from bit 7.
  - C and OV are written back only where the page lists them for that N, so every N = 0 form keeps OV.
- **Vectors.**
  - `immediate.json`: 23 vectors. Every row has an `addr:immediate` vector with a figure-basis bus list.
  - `shift.json`: 43 vectors, all `kind: source`. Each asserts `expect.cycles` and carries a FETCH bus list.
  - Every vector passed against the core on its first run.
- **Gate.** The pending list went from 48 to 34 rows (49 before this plan). There are 155 vectors, 352 tests and 20 faults.

## Task Commits

1. **Task 1: Tracer, MVII end to end** - `2bb3122` (feat). It added 5 MVII vectors and IMMEDIATE_PC_ADVANCE. Tracer gate: `sh tools/gate.sh` re-ran green (`faults=15/15 rows-pending=48`), so expansion went ahead.
2. **Task 2: ADDI, SUBI, CMPI, ANDI, XORI and MVOI with MVOI_ADDRESS** - `b61f2cc` (feat). It added 18 vectors and gave `faults=16/16 rows-pending=42`.
3. **Task 3: the shift and SWAP rows with four faults** - `5e8d895` (feat). It added 43 vectors and gave `faults=20/20 rows-pending=34`.

**Plan metadata:** recorded in the docs(02-09) completion commit.

## Bus offsets and the pages that print them

| Access | Offset | Basis | Page |
|--------|--------|-------|------|
| FETCH of the instruction | 0 | figure | pdf 28 (instruction fetch) |
| READ of the immediate word (MVII, ADDI, SUBI, CMPI, ANDI, XORI) | 4 (data in at 6), unit 8 | figure | pdf 29, "Instruction fetch & data access (indirect reg. mode)": BAR, NACT, DTB, NACT, BAR (output effective address), NACT, DTB (input fetched operand) |
| WRITE of register SSS (MVOI) | 4 (DW at 6, DWS at 7), unit 9 | figure | pdf 31, "MVO timing (write operation)": BAR, NACT, DTB, NACT, BAR (output effective address), NACT, DW, DWS, NACT |

Immediate mode is that same data access with the effective address PC+1 (Fig 3.2.5, pdf 49: EA <- PC+1, PC <- PC+2). The manual prints no figure labelled "immediate".

## Vector disagreements and resolutions

None. All 66 vectors passed on their first run. No `expect` was edited after a run, and no core change was made to match a vector. The plan 02-17 second reader has no recorded resolution from this plan to revisit.

The readings it should re-derive independently are:

- **C47 (S after right shifts and SWAP):** S is bit 7 of the result. These vectors separate bit 7 from bit 15:
  - `cpu.SLR.s-from-bit7`
  - `cpu.SAR.n0`
  - `cpu.SWAP.n0`
  - `cpu.SWAP.s-bit7-not-bit15`
  - `cpu.RRC.n0-sign-bit15`
- **C48 (link bits):**
  - RLC N=1 rotates the 18-bit string C OV R15..R0. `cpu.RLC.n1-ov-in` starts with OV = 1 and C = 0 and expects R = $0001, C = 1, OV = 0.
  - RRC N=1 rotates OV C R15..R0. `cpu.RRC.n1-ov-in` starts with OV = 1 and C = 0 and expects R = $8000, C = 1, OV = 0. The exchanged layout would give R = $4000, C = 0, OV = 1.
  - N = 0 keeps OV, shown by `cpu.RLC.n0-ov-preserved` and `cpu.RRC.n0-ov-preserved`.
- **SARC N=1 link layout (new inference, no discrepancy record yet):** "thru Carry and OV" names no positions. The vectors use the RRC layout from the same page (bit 0 to C, bit 1 to OV). That is also the rule that C gets the first bit shifted out, as in SLLC and RLC. The affected vectors are `cpu.SARC.n1`, `cpu.SARC.n1-ov-clear` and `cpu.SARC.n1-zero`. A later plan should consider a discrepancy record next to C48.
- **SWAP N=1:** the register keeps its original form ("then swap them back to original form", pdf 53), and S is bit 7 of that value.
- **C46 (CMPI):** CMPI computes SSS minus the immediate, which is the order the CMPI row states. `cpu.CMPI.no-write` ($0005 - $0003) separates the two orders.
- **Immediate bus offsets:** these are figure-basis readings of pdf 29 and pdf 31 under EA = PC+1. See the table above.

## Fault verdicts (latest gate run)

| Fault | Vectors that fail (all inside the allowed prefixes) |
|-------|------------------------------------------------------|
| IMMEDIATE_PC_ADVANCE | all 23 immediate vectors except `cpu.MVII.r7-dest`, where the R7 write replaces the PC |
| MVOI_ADDRESS | all 3 MVOI vectors |
| SHIFT_N1_CYCLES | all 22 N=1 shift vectors |
| RIGHT_SHIFT_S_BIT15 | 14 vectors where bit 7 and bit 15 of the result differ (SLR, SAR, RRC, SARC, SWAP) |
| RRC_LINK_SWAP | `cpu.RRC.n1-ov-in` |
| SWAP_N1_AS_N0 | `cpu.SWAP.n1`, `cpu.SWAP.n1-s-bit7` |
| ADD_OVERFLOW (existing) | now also `cpu.ADDI.7fff-plus-1-overflow` and `cpu.ADDI.ffff-plus-1-carry-zero` (the prefix already listed `cpu.ADDI.`) |
| SUB_CARRY (existing) | now also every SUBI and CMPI vector |
| MOVR_R7_PC (existing) | now also `cpu.MVII.r7-dest` |

Every must-pass vector passes under its fault: `cpu.ADDR.0001-plus-0001`, `cpu.MVII.r1-load`, `cpu.SLL.n0` and `cpu.SWAP.n0`.

## Decisions Made

- **Handler ids.** Each row has its own handler id: MVII 26, ADDI-MVOI 27-32 and SWAP-SARC 33-40. All eight shift ids go through one `ivs_shift` call. The ids sit above `IVS_CP1610_OP_ADCR`, so R7_CYCLE_BONUS_ALL's handler range does not reach them.
- **MVOI ordering.** MVOI evaluates the address before it reads register SSS, in explicit sequence rather than in C argument order. The operand-address fault sites live in `ivs_imm_addr` and the MVOI case.
- **Flag exemptions.** None. Every printed flag of the 15 rows can be both set and cleared with some operand, so `cp1610-flag-exempt.ids` is unchanged.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Fault prefix] MOVR_R7_PC also allows `cpu.MVII.r7-dest`**
- **Found during:** Task 1
- **Issue:** MVII writes DDD through `ivs_wreg`, which holds the MOVR_R7_PC site. MVII R7 therefore legitimately fails under that fault.
- **Fix:** The exact stem `cpu.MVII.r7-dest` was added to that fault's prefix column, as the 02-08 hand-off asks.
- **Committed in:** 2bb3122

**2. [Rule 3 - Fault prefix] SUB_CARRY also allows `cpu.SUBI.` and `cpu.CMPI.`**
- **Found during:** Task 2
- **Issue:** SUBI and CMPI share `ivs_op_sub`, so every one of their vectors fails under SUB_CARRY.
- **Fix:** The two row stems were added in the same commit, following the plan 02-08 `<context>` rule.
- **Committed in:** b61f2cc

**Smaller choices, no behaviour risk:**
- There are 23 immediate vectors (the plan said about 21) and 43 shift vectors (about 48; acceptance needs 40 or more). Every row meets its derived coverage. Extra edge cases ($FFFF, zero results, bit-7-only signs) replaced repeats.
- The immediate-row ALU vectors also tag `addr:immediate`. Every immediate and shift vector carries a bus list, not only one per row.

---

**Total deviations:** 2 auto-fixed (both fault-prefix extensions under the established rule).
**Impact on plan:** Every acceptance count holds:
- 5 `cpu.MVII.` ids, and 23 vectors with `"bus_basis": "figure"`
- 1 IMMEDIATE_PC_ADVANCE line
- 0 immediate rows pending
- 23 ids in immediate.json and 43 in shift.json
- 2 n0-ov-preserved ids
- 0 policy vectors in shift.json
- `rows-pending=34`, `faults=20/20`

## Notes for later plans

- **Plan 02-10 (direct MVO figure):** the pdf 31 "MVO timing" figure is 9 cycles long: fetch, then BAR of the effective address at 4, then DW and DWS. It has no address-word read and no ADAR, so it matches MVO@, MVOI and PSHR (9 cycles), not direct MVO (11 cycles, pdf 55). If `cpu.MVO.write-figure` cites pdf 31 for direct MVO, its offsets after the address word would be an extrapolation. Re-read pdf 31 before using `figure` basis there.
- **Plan 02-10 (`cpu.MVOI.sss7-unsupported`):** MVOI with SSS = 7 currently writes R7 after the PC advance, which is $1002 for a program at $1000. No vector asserts this. POL-MVO-01 will refuse it.
- **Plan 02-13 (SDBD):** the immediate handlers ignore `dbd`, so an SDBD before MVII currently leaves DBD set. Hook the double-byte path into `ivs_imm_read`, where the PC advance and the IMMEDIATE_PC_ADVANCE fault site already live.
- **Plan 02-11/02-12:** ADD, ADD@ and ADDI reach ADD_OVERFLOW through `ivs_op_add`, and SUB, CMP and their @ forms reach SUB_CARRY through `ivs_op_sub`. Add `cpu.SUB.`, `cpu.SUB@.`, `cpu.CMP.` and `cpu.CMP@.` to SUB_CARRY as their vectors land.
- **Unasserted R7 inference:** an ALU immediate with DDD = 7 reads R7 as PC+2, after the operand advance. No vector asserts this.

## Measured cost (D-22 budget input)

- Final `sh tools/gate.sh`: 33.0 s wall, unpiped. `dash tools/gate.sh` also ends `gate: PASS`.
- The previous plan measured 34.0 s. The 6 extra fault executables and 66 extra vectors add no visible time.

## Issues Encountered

None.

## Known Stubs

- The 34 rows still pending report the temporary UNIMPLEMENTED fault through their `IVS_CP1610_OP_NONE` handler: J, the 17 branches and 16 external-reference rows. This is plan design. The pending list tracks it and the gate counts it (`rows-pending=34`), so it is not added to `.planning/WINDOWS.md`.

## User Setup Required

None.

## Next Phase Readiness

- The memory plans (02-10..02-12) can reuse `IVS_DATA_BAR_OFFSET`, `ivs_access` READ/WRITE records and the family helpers.
- SDBD (02-13) extends `ivs_imm_read`.
- The interrupt plan reads the shifts' non-interruptible flag from the decode table, which is unchanged.

---
*Phase: 02-cp-1610-cpu-core*
*Completed: 2026-11-10*

## Self-Check: PASSED

- The 3 key files exist: `immediate.json`, `shift.json` and this SUMMARY.
- Commits `2bb3122`, `b61f2cc` and `5e8d895` are ancestors of HEAD. The ledger measures `git rev-list --count 0d8b4b1..HEAD` = 3.
- Acceptance counts were re-run:
  - 23 `bus_basis: figure` in immediate.json
  - 1 IMMEDIATE_PC_ADVANCE line
  - 0 immediate rows pending
  - 43 `cpu.` ids in shift.json
  - 2 n0-ov-preserved ids
  - 0 policy vectors in shift.json
- Final gate: `sh tools/gate.sh` gate: PASS steps=24/24 controls=182/182 vectors=155 rows-pending=34 faults=20/20 tests-executed=352 (33.0 s). `dash tools/gate.sh` passed too.
