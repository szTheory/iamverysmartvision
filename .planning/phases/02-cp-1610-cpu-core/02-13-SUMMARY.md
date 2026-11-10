---
phase: 02-cp-1610-cpu-core
plan: 13
subsystem: cpu-core
tags: [c17, cp1610, sdbd, double-byte-data, pol-sdbd-01, pol-sdbd-02, pol-ill-01, decode-sweep, vectors, named-faults, cpu-01, cpu-03]

requires:
  - phase: 02-cp-1610-cpu-core
    plan: 12
    provides: ivs_jump with the DE = 11 stop after word 2, the fault path for refusals found inside ivs_exec, all 74 rows with handlers and vectors
provides:
  - ivs_dbd_read, the double byte operand (low 8 bits of two words) used by the shared immediate and indirect readers when DBD is set; the target unit costs 2 more cycles (4 + 10 = 14, C34) and clears DBD
  - POL-SDBD-01 indirect modes - MMM 4-5 two reads with two post-increments, MMM 1-3 the same address read twice (A-03, C37)
  - POL-SDBD-02 - ivs_refusal refuses every other decoded target with the sticky fault UNSUPPORTED_SDBD_TARGET after the fetch, R7 at the offender, DBD kept
  - POL-ILL-01 - jump DE = 11 faults UNSUPPORTED_JUMP_DE; bits 15..10 of a word take no part (checked over all 65536 words)
  - The temporary unimplemented fault reason and IVS_CP1610_OP_NONE are gone; IVS_CP1610_FAULT_LAST bounds validation, so fault_reason 5 is rejected
  - data/cp1610/vectors/sdbd.json (group sdbd, 27 vectors) and data/cp1610/vectors/encoding.json (group encoding, 5 vectors)
  - cmake/IvsSweepTable.cmake and suite cpu-sweep (75 ids) - the core decode table against the Phase 1 family table for every decle, with and without DBD
  - 7 named faults SDBD_IMMEDIATE_PC SDBD_INDIRECT_INCREMENT SDBD_MODE13_ONE_READ SDBD_DBD_NOT_CLEARED SDBD_TARGET_ACCEPTED DECODE_HIGH_BITS JUMP_DE11_ALLOWED
affects: [02-14, 02-15, 02-16, 02-17, 02-18]

actuals:
  tokens: 31740
  tasks: 3
  commits: 3
plan_head_before: f6f69dc7ca2fc8c54f9a4096f9f427644e0f32bf
plan_head_after: 210b3d13adb955c8271fbc7e52e7e80c7ec409cc

tech-stack:
  added: []
  patterns:
    - "A prefix state (DBD) is read inside the shared operand readers, so every target row gets the double byte path from one site and the refusal decides which rows may reach it"
    - "A second cmake -P generator turns an independent inventory table into a build-tree C table for a test-only comparison, with the table's sha256 echoed as a `# source` line so the gate re-hashes it"

key-files:
  created:
    - data/cp1610/vectors/sdbd.json
    - data/cp1610/vectors/encoding.json
    - cmake/IvsSweepTable.cmake
    - tests/cpu/cpu_sweep.c
    - data/expected/tests/cpu-sweep.ids
  modified:
    - core/src/cpu/cp1610.c
    - core/src/cpu/cp1610.h
    - core/src/cpu/cp1610_decode.c
    - cmake/IvsVectorTable.cmake
    - tests/CMakeLists.txt
    - tests/cpu/cpu_api.c
    - data/expected/cp1610-vector-files.ids
    - data/expected/cp1610-vectors.ids
    - data/expected/tests/cpu-vectors.ids
    - data/expected/cp1610-coverage-pending.ids
    - data/expected/faults/cpu.ids

key-decisions:
  - "The double byte high READ sits at offset 7 of the target unit (inference, C36): the printed 10 cycles are the 8-cycle read figure (pdf 29) with the second BAR, NACT, DTB in place of its trailing NACT. The low READ keeps offset 4. The SDBD vectors carry bus_basis inference."
  - "DBD is read inside ivs_imm_read and ivs_ind_read (the 02-09 hand-off): only rows that ivs_sdbd_target accepts can reach those readers with DBD set, because every other target is refused before ivs_exec runs."
  - "ivs_sdbd_target: the six immediate rows, and the six @ read rows with MMM 1-5. MMM 6 of ADD@..XOR@ decodes to the @ row itself (only MVI@ MMM 6 is PULR), so the mode check is on the decle, not the row."
  - "Refusal order in ivs_refusal: the SDBD target check comes before POL-MVO-01, since the prefix decides how the decle is read. With DBD set, $264, $26D, $276 and $27F report UNSUPPORTED_SDBD_TARGET."
  - "A refused SDBD target leaves prev_interruptible as the SDBD unit set it (0) and costs only the 4 fetch cycles, like the MVO refusal; the vectors assert 4 + 4 = 8."
  - "cpu.MVII.sdbd-dbd-cleared runs three units (SDBD, double byte MVII, single MVII) so a leaked DBD shows as a second operand read, not only as the state field."
  - "DECODE_HIGH_BITS sits in ivs_refusal (cp1610.c), not in the decoder. A mask decoder that keeps bits 15..10 still matches every 10-bit mask, so that bug cannot be written there. The modelled bug is 'a word with Future Use bits set is an undefined encoding and is refused'."
  - "cpu.MOVR.coincidence-0bf-high-bits keeps the plan's id but carries row TSTR: the vector checker requires a leaf row, and $0BF has leaves TSTR and JR (the same choice as cpu.MOVR.coincidence-0bf in plan 02-08)."
  - "The sweep also runs every decle with DBD set. A row is expected to be accepted when its family-table sdbd column reads supported and MMM is not 110. The mode-6 exclusion comes from pdf 49 and pdf 57, not from the table, whose column is per row."
  - "requirements-completed stays empty, as in plans 02-02..02-12: CPU-03 and CPU-01 advance, and plans 02-17 and 02-18 still declare them."

patterns-established:
  - "Refusal before effect: one function decides every pre-execution refusal (SDBD target, MVO@ SSS = MMM) from the decoded row and the word, so refusal order is written once"

requirements-completed: []
requirements-advanced: [CPU-03, CPU-01]

coverage:
  - id: D1
    description: "SDBD followed by MVII, ADDI, SUBI, CMPI, ANDI or XORI reads the low bytes of PC+1 and PC+2 (junk in bits 15..8 ignored), leaves R7 at PC+4 from the SDBD word, takes 4 + 10 = 14 cycles and clears DBD. SDBD_IMMEDIATE_PC and SDBD_DBD_NOT_CLEARED each fail their must-fail vector and only cpu.*.sdbd vectors."
    requirement: CPU-03
    verification:
      - kind: integration
        ref: "sh tools/gate.sh (faults=45/45; build/gate/faults/SDBD_IMMEDIATE_PC/cpu-vectors.txt, build/gate/faults/SDBD_DBD_NOT_CLEARED/cpu-vectors.txt)"
        status: pass
    human_judgment: false
  - id: D2
    description: "SDBD followed by an @ read row reads two words with two post-increments for MMM 4-5 and the same address twice for MMM 1-3 (POL-SDBD-01). Every other target is refused with UNSUPPORTED_SDBD_TARGET, R7 at the offender, DBD kept and only the fetch traced (12 POL-SDBD-02 vectors). SDBD_INDIRECT_INCREMENT, SDBD_MODE13_ONE_READ and SDBD_TARGET_ACCEPTED each fail their must-fail vector."
    requirement: CPU-03
    verification:
      - kind: integration
        ref: "sh tools/gate.sh and dash tools/gate.sh (gate: PASS vectors=351 policy=60 policies-pending=11 faults=45/45)"
        status: pass
    human_judgment: false
  - id: D3
    description: "Every one of the 1024 decles decodes to a family-table leaf with the same word count, PC advance, plain printed cycles and untouched flags outside status_bits, with and without DBD. All 65536 words step like their low 10 bits. Jump DE = 11 faults UNSUPPORTED_JUMP_DE. No temporary fault reason remains, and DECODE_HIGH_BITS and JUMP_DE11_ALLOWED fail their must-fail vectors."
    requirement: CPU-01
    verification:
      - kind: unit
        ref: "build/gate/dev/cpu-sweep.txt and build/gate/release/cpu-sweep.txt (75 ok each)"
        status: pass
      - kind: integration
        ref: "sh tools/gate.sh (tests-expected=894 tests-executed=894 faults=45/45)"
        status: pass
    human_judgment: false

duration: 18min
completed: 2026-11-10
status: complete
---

# Phase 2 Plan 13: SDBD, Encoding Policy and Decode Sweep Summary

**SDBD now gives the next instruction a double byte operand, built from the low 8 bits of two words. For immediates the words are PC+1 and PC+2. For @ rows with MMM 4-5 it is two reads with two post-increments, and for MMM 1-3 the same address is read twice. The target costs 2 more cycles (4 + 10 = 14) and clears DBD. Every other target is refused with the sticky fault UNSUPPORTED_SDBD_TARGET. Jump DE = 11 now faults UNSUPPORTED_JUMP_DE, and the temporary unimplemented reason is gone. A new suite, cpu-sweep, checks all 1024 decles against the independent Phase 1 family table and all 65536 words against their low 10 bits. Seven named faults guard the rules.**

## Performance

- **Duration:** about 18 min
- **Started:** 2026-11-10T08:39:10Z
- **Completed:** 2026-11-10T08:57:19Z
- **Tasks:** 3 of 3
- **Files modified:** 16 (3414 insertions, 31 deletions)

## Accomplishments

- **Double byte operand.** `ivs_dbd_read(n, u, mmm)` takes both addresses through the existing address rules:
  - immediate: `ivs_imm_addr` twice;
  - indirect: `ivs_ind_addr` twice.

  It then issues two READ records (offsets 4 and 7) and returns `lo | hi << 8`, each byte taken from the low 8 bits of its word. `ivs_imm_read` and `ivs_ind_read` call it when DBD is set. `ivs_unit_instr` clears DBD after a consuming target and adds `IVS_SDBD_EXTRA_CYCLES` (2).
- **Refusal.** `ivs_refusal(n, row, word)` returns a refusal before anything executes, in this order:
  1. UNSUPPORTED_SDBD_TARGET, when DBD is set and `ivs_sdbd_target` says no;
  2. UNSUPPORTED_MVO_SSS_EQ_MMM (POL-MVO-01);
  3. FAULT_NONE.

  A refused unit is the fetch only: 4 cycles, R7 at the offender, DBD kept.
- **POL-ILL-01.**
  - Jump DE = 11 reports UNSUPPORTED_JUMP_DE (8 cycles, both reads traced, R7 at the offender).
  - Removed: `IVS_CP1610_FAULT_UNIMPLEMENTED` (value 5), `IVS_CP1610_OP_NONE`, the decode-NULL refusal and the generator's `UNIMPLEMENTED` name.
  - `IVS_CP1610_FAULT_LAST` bounds `ivs_cp1610_state_validate`. `cpu-api.validate-rejects-bad-fault-reason` now expects reason 5 to be rejected.
  - A NULL row from decode is a `__builtin_trap()`. It is unreachable: decode masks to 10 bits, and cpu-sweep proves every decle has a row.
- **Vectors.**
  - `sdbd.json` has 27 vectors:
    - 12 `addr:sdbd` source vectors, one per `supported` row (6 immediate, 6 indirect with MMM 4 or 5);
    - 2 POL-SDBD-01 policy vectors for MMM 1-3;
    - `cpu.MVII.sdbd-dbd-cleared`, three units;
    - 12 POL-SDBD-02 refusals.
  - `encoding.json` has 5 POL-ILL-01 vectors.
  - Every vector passed on its first run.
- **Sweep.** `cmake/IvsSweepTable.cmake` reads `opcode-families.json` and writes `ivs_sweep.inc` with these columns per decle:
  - leaf rows;
  - words;
  - plain base cycles;
  - status mask, with N-dependent shift columns resolved by the `N` position in `printed`;
  - pc_check;
  - sdbd.

  `tests/cpu/cpu_sweep.c` runs one case per row, 74 in all. Each case steps every decle of the row three times: flags 0, flags 1, and DBD set. The last case steps all 65536 words. The whole suite runs in about 0.1 s under ASan.
- **Gate.** `vectors=351 source=291 policy=60 rows-pending=0 policies-pending=11 faults=45/45 tests-expected=894 tests-executed=894`.

## Task Commits

1. **Task 1: Tracer, SDBD + MVII end to end** - `c7ce174` (feat). Gate `faults=39/39`. Tracer feedback gate: `sh tools/gate.sh` re-ran green before Task 2 started.
2. **Task 2: SDBD indirect modes 1-5, refusal of every other target, DBD clearing, four faults** - `35db0f0` (feat). Gate `faults=43/43`.
3. **Task 3: POL-ILL-01 encodings, removal of the temporary fault reason, cpu-sweep** - `210b3d1` (feat). Gate `faults=45/45`.

**Plan metadata:** recorded in the docs(02-13) completion commit.

## Bus offsets and the pages that print them

| Access | Offsets in the target unit | Basis | Page |
|--------|----------------------------|-------|------|
| SDBD unit | FETCH at 0, 4 cycles | figure | pdf 28 (fetch), pdf 57 (SDBD 4) |
| Double byte target (immediate and indirect) | FETCH at 0; low READ at 4; high READ at 7; unit 10 cycles | inference (C36) | Neither the 10 nor the 14 cycles (pdf 57, 58) come with a figure. The low READ follows the read figure (pdf 29), and the high READ fills the 2 extra cycles. In the two-step vectors these are 4, 8 and 11. |
| Refused SDBD target | FETCH at 0, 4 cycles | figure | pdf 28 |
| Jump DE = 11 | FETCH at 0; READ of word 2 at 4; 8 cycles | inference (C36) | pdf 52, 56 |

## Vector disagreements and resolutions

None against the core. All 32 new vectors passed on their first run. No `expect` value was edited after a run, and no core change was made to match a vector.

Readings that differ from `data/cp1610/opcode-families.json`: **none**.
- All 74 row cases and the high-bits case passed on the first sweep run in both presets.
- No discrepancy entry was needed.
- To show the sweep is not vacuous, a scratch build of the sweep linked against fault-enabled cores (session scratch only, not tracked) failed as expected:
  - DECODE_HIGH_BITS: the high-bits case;
  - SDBD_TARGET_ACCEPTED: 67 row cases;
  - SDBD_DBD_NOT_CLEARED: the 12 supported rows;
  - INCR_TOUCHES_C: cpu-sweep.INCR.
- MMM6_BONUS_MISSING was **not** caught. That is expected: the @ rows print `8*`, a count with a modifier, so the sweep asserts no cycles for them. Their vectors cover it.

Page readings for the plan 02-17 second reader to re-derive independently:
- **Double byte byte order.**
  - pdf 47: "The least significant data byte is obtained from the low order eight bits of the first word and the most significant data byte ... from the low order eight bits of the second word."
  - Fig 3.2.7 (pdf 49) prints bits 15..8 of PC+1 and PC+2 as "Not Used", and pdf 58 prints them `XX`.
  - The vectors put junk there ($AB, $CD, $12, $34 ...).
- **Immediate PC.** Fig 3.2.7 prints `PC <- PC+3`, where PC is the target word, so R7 ends at the SDBD address + 4.
- **Indirect modes.**
  - Fig 3.2.6 (pdf 49), MMM 4 and 5: `EA_L <- (MMM)`, `(MMM) <- (MMM)+1`, `EA_H <- (MMM)`, `(MMM) <- (MMM)+1`. MMM 1-3: `EA_L <- (MMM)`, `EA_H <- (MMM)`.
  - The pdf 57 SDBD row text says the high byte address "will be EA+1 if register 4, 5 or 7 is used", and that modes 1-3 "will access the same byte twice resulting in both bytes of data being the same".
  - pdf 47 says only one word is accessed. A-03 / C37 choose two reads, as policy.
- **Refusal set.**
  - pdf 57: "next instruction which must be an external reference instruction ... Use of modes 0 and 6 are not supported".
  - Fig 3.2.6 note: "Mode 000 and 110 are not supported for double byte data operations".
  - pdf 58 lists no MVOI double byte row.
  - The 12 refusal targets are MVO@, PULR, PSHR, MVI and ADD direct, B, J, SDBD, ADDR, SLL, CLRC and MVOI.
- **Cycles.** pdf 58 prints 14 including the SDBD word, and pdf 57 prints 4 then 10, so the target unit is 10 (C34, A-05).
- **Future Use bits.** Fig 3.3.1-3.3.5 (pdf 50-52) and Fig 3.2.2 (pdf 47) print bits 15..10 as "Future Use".
- **DE = 11.** The jump table (pdf 56) prints DE 00, 01 and 10 only.

## Fault verdicts (latest gate run)

| Fault | Vectors that fail (all inside the allowed prefixes) |
|-------|------------------------------------------------------|
| SDBD_IMMEDIATE_PC | the 6 immediate double byte vectors and `cpu.MVII.sdbd-dbd-cleared`; must-pass `cpu.MVII.r1-load` passes |
| SDBD_INDIRECT_INCREMENT | the 6 MMM 4/5 vectors (`cpu.MVI@.sdbd-r4` ... `cpu.XOR@.sdbd-r5`) |
| SDBD_MODE13_ONE_READ | `cpu.MVI@.sdbd-r1-same-address-twice` and `cpu.ADD@.sdbd-r2-same-address-twice`. The register value is the same under one read; the bus list catches it. |
| SDBD_DBD_NOT_CLEARED | all 15 accepted SDBD vectors |
| SDBD_TARGET_ACCEPTED | all 12 POL-SDBD-02 refusal vectors |
| DECODE_HIGH_BITS | the 4 high-bit vectors; must-pass `cpu.CLRC.clears-c` passes |
| JUMP_DE11_ALLOWED | `cpu.J.de11-unsupported`; must-pass `cpu.J.plain` passes |

No existing fault needed a prefix change: their prefixes already covered the new `cpu.<ROW>.sdbd-*` vectors.

## Decisions Made

See `key-decisions` in the frontmatter. The main ones:
- the high READ offset is an inference;
- DBD is consumed inside the shared readers;
- the SDBD check comes before the MVO@ check;
- the DECODE_HIGH_BITS site is in the stepper;
- the coincidence vector is labelled TSTR;
- the sweep has a DBD pass with the mode-6 exclusion.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Files] Generator and header edits outside the task file lists**
- **Found during:** Task 3
- **Issue:** `cmake/IvsVectorTable.cmake` listed `UNIMPLEMENTED` in `VT_FAULTS`. That is a use of the temporary reason, which the plan says to delete everywhere, but the file is not in `files_modified`.
- **Fix:**
  - Removed the name from the generator.
  - Also removed `IVS_CP1610_OP_NONE`, whose only meaning was "reports UNIMPLEMENTED".
  - Added `IVS_CP1610_FAULT_LAST` so that validation and the cpu-api test do not hard-code the last reason.
- **Files modified:** `cmake/IvsVectorTable.cmake`, `core/src/cpu/cp1610.h`
- **Committed in:** 210b3d1

**2. [Rule 1 - Fault design] DECODE_HIGH_BITS placed in the stepper's refusal function**
- **Found during:** Task 3
- **Issue:** The plan describes the fault as "decode uses all 16 bits". In this mask decoder, keeping bits 15..10 changes nothing, because every row mask is 10 bits wide. A site in `cp1610_decode.c` would therefore never fail its must-fail vector.
- **Fix:** The site is 2 lines in `ivs_refusal`. A word above $3FF is refused (reported as UNSUPPORTED_JUMP_DE), which models a decoder that treats Future Use bits as an undefined encoding. The inventory line is exactly as planned.
- **Committed in:** 210b3d1

**3. [Rule 2 - Coverage] The sweep also checks the DBD path**
- **Found during:** Task 3
- **Issue:** The plan says "SDBD-target refusal and fault rows check the fault instead". With DBD clear, no SDBD refusal can happen, so that check needs a DBD pass.
- **Fix:** Each decle also runs once with DBD set:
  - The expected outcome is acceptance when the family table's sdbd column reads `supported` and MMM != 110 (pdf 49, 57). An accepted decle must clear DBD, advance R7 by 1 + 2 x (words - 1) and take base + 2 cycles.
  - Otherwise the expected outcome is the refusal.
- **Committed in:** 210b3d1

---

**Total deviations:** 3 auto-fixed (1 file list, 1 fault-site placement, 1 coverage extension).
**Impact on plan:** No scope change. Every acceptance count holds:
- Task 1: 15 `addr:sdbd` (3 or more needed); 1 SDBD_IMMEDIATE_PC line.
- Task 2: 0 pending SDBD lines; 12 POL-SDBD-02 vectors, each id once in the file and once in the SDBD_TARGET_ACCEPTED prefix column; 15 `addr:sdbd`; 0 `mmm6-refused`.
- Task 3: 0 FAULT_UNIMPLEMENTED; 75 `ok` lines in `build/gate/release/cpu-sweep.txt`; 0 POL-ILL-01 pending lines; `encoding` listed once.

## Measured cost (D-22 budget input)

- **Final `sh tools/gate.sh`:** 33 s wall, unpiped (output to a file), load average about 7 to 10. Summary line: `gate: PASS steps=24/24 controls=182/182 ... vectors=351 source=291 policy=60 rows-pending=0 policies-pending=11 bus-figure-pages=25 ... faults=45/45 ... tests-expected=894 tests-executed=894 tests-skipped=0 tests-missing=0`.
- **`dash tools/gate.sh`:** 34 s, `gate: PASS`.
- **Task gates:** 54 s for Task 1 and 48 s for Task 2, both under heavier load (load averages above 10). For comparison, plan 02-12 measured 43.1 s.
- **cpu-sweep run time:** about 0.1 s per preset. It covers 3072 decle steps plus 131072 high-bit steps.

## Issues Encountered

None.

## Known Stubs

None. No temporary fault reason or handler-less row remains.

## User Setup Required

None.

## Next Phase Readiness

- **Plan 02-14 (interrupts, POL-SDBD-03):**
  - SDBD is non-interruptible through the decode table (C56).
  - A refused target leaves DBD at 1 and prev_interruptible at 0. A refused unit is FAULTED and sticky, so no interrupt can follow it.
  - The "SDBD interruptible" fault can flip the SDBD row's interruptible flag or the boundary test.
- **Plan 02-16/02-17 (second reader):**
  - Read the page readings list above.
  - The SDBD vectors use `steps` 2 or 3 and cumulative bus offsets.
  - The policy statement for POL-SDBD-01 must give the A-03 two-read rule in words.
- **Unasserted or inferred:**
  - the high READ offset 7 (C36);
  - the order in which a refusal with both DBD and SSS = MMM reports its reason (SDBD first; no vector has both).

---
*Phase: 02-cp-1610-cpu-core*
*Completed: 2026-11-10*

## Self-Check: PASSED

- Key files exist: `data/cp1610/vectors/sdbd.json`, `data/cp1610/vectors/encoding.json`, `cmake/IvsSweepTable.cmake`, `tests/cpu/cpu_sweep.c`, `data/expected/tests/cpu-sweep.ids` and this SUMMARY.
- Commits `c7ce174`, `35db0f0` and `210b3d1` are ancestors of HEAD. The ledger measures `git rev-list --count f6f69dc..210b3d1` = 3.
- Every task's acceptance commands were re-run and printed the expected values.
- Final gate: `sh tools/gate.sh` gate: PASS (33 s) and `dash tools/gate.sh` gate: PASS (34 s).
