---
phase: 02-cp-1610-cpu-core
plan: 12
subsystem: cpu-core
tags: [c17, cp1610, bext, ebci, jump, jsr, intff, pol-br-01, vectors, named-faults, cpu-01, cpu-02]

requires:
  - phase: 02-cp-1610-cpu-core
    plan: 11
    provides: ivs_branch (displacement read, direction rule, POL-BR-01 cycles), branch.json with 29 POL-BR-01 vectors, the ebci field of ivs_cp1610_port, the vector checker's ebci:* proofs
provides:
  - BEXT through ivs_branch with bit 4 selecting the external condition, ivs_branch_ext asking port->ebci(ctx, decle & $F) after the displacement READ; NULL provider is false (C57)
  - ivs_jump for J, JE, JD, JSR, JSRE, JSRD (IVS_CP1610_OP_JUMP) - target from bits 7..2 of word 2 and bits 9..0 of word 3, PC+3 saved in R4/R5/R6, DE 01/10 set/clear INTFF, DE 11 the temporary UNIMPLEMENTED fault after word 2
  - Test-harness ebci semantics - key absent gives a NULL provider, key present (even empty) gives a provider answering true exactly for the listed codes (has_ebci)
  - 6 BEXT vectors in data/cp1610/vectors/branch.json (98 vectors, 31 POL-BR-01) and data/cp1610/vectors/jump.json (group jump, 11 vectors)
  - 4 named faults BEXT_IGNORES_EBCI BEXT_E_FIELD_IGNORED JSR_RETURN_ADDR JUMP_TARGET_BITS
  - All 74 rows have vectors (rows-pending=0)
affects: [02-13, 02-14, 02-15, 02-16, 02-17, 02-18]

actuals:
  tokens: 14696
  tasks: 2
  commits: 2
plan_head_before: f84c658d8e4917d4f52b488953e50aa9e178920c
plan_head_after: da5679dc351049311ede57a1ce6e0fb1bb4943ba

tech-stack:
  added: []
  patterns:
    - "A vector's optional key is passed to the C table with an explicit has_<key> flag, so 'absent' and 'present but empty' stay different"
    - "A multi-word instruction reads every operand word through ivs_imm_addr, the one PC-advance site, so R7 = PC + words before any register save"

key-files:
  created:
    - data/cp1610/vectors/jump.json
  modified:
    - core/src/cpu/cp1610.c
    - core/src/cpu/cp1610.h
    - core/src/cpu/cp1610_decode.c
    - cmake/IvsVectorTable.cmake
    - tests/harness/ivs_vec.h
    - tests/cpu/cpu_vectors.c
    - data/cp1610/vectors/branch.json
    - data/expected/cp1610-vector-files.ids
    - data/expected/cp1610-vectors.ids
    - data/expected/tests/cpu-vectors.ids
    - data/expected/cp1610-coverage-pending.ids
    - data/expected/faults/cpu.ids

key-decisions:
  - "BEXT asks the provider after the displacement READ: pdf 32 shows EBCI 'valid input throughout TS1' of the BAR that outputs PC+1, and EBCA0-3 are bits 0-3 of the instruction register (pdf 11), so E = decle & $F. Any nonzero answer is true."
  - "The test port's ebci semantics needed a generator change, not a testbus change: n_ebci > 0 had stood for 'provider present', so an empty list meant NULL. A has_ebci flag now carries 'key present'."
  - "cpu.J.target-ffff uses the JSR R4 form (BB = 00, second word $0FC). With BB = 11 and all A bits 1, bits 9..4 of the second word equal bits 7..2, so JUMP_TARGET_BITS could not fail a plain J to $FFFF."
  - "Jump bus offsets are an inference (C36, no jump figure): FETCH at 0, READ of word 2 at 4, READ of word 3 at 8, three 4-cycle word transfers in the 12 printed cycles. Jump bus lists carry bus_basis inference."
  - "The jump target takes only the printed fields; bits 15..8 of word 2 (BB and Future Use) and bits 15..10 of word 3 (Not Used) take no part. No vector asserts the high bits (unasserted reading, pdf 52)."
  - "The JSR return address is R7 after both operand reads, which is PC+3 by construction (pdf 56, C58); it wraps at 16 bits (cpu.J.jsr-return-wraps: $FFFE + 3 = $0001)."
  - "Jump DE = 11 stops after word 2 with R7 back at the offender, 8 cycles, both reads traced, UNIMPLEMENTED for now; plan 02-13 renames it UNSUPPORTED_JUMP_DE."
  - "requirements-completed stays empty, as in plans 02-02..02-11: CPU-01 and CPU-02 advance, and plans 02-13..02-18 still declare them."

patterns-established:
  - "When a must-fail fault mutation coincides with the correct reading for one encoding, choose the vector's encoding so the two readings differ, and say so in the derivation"

requirements-completed: []
requirements-advanced: [CPU-01, CPU-02]

coverage:
  - id: D1
    description: "BEXT branches exactly when the provider answers true for the decle's own E code, in both directions, with E = 5 and E = 10 each answered true and false (cpu.BEXT.fwd-taken-e5, back-not-taken-e5, back-taken-e10, fwd-not-taken-e10). A NULL provider is false (cpu.BEXT.cycles-not-taken). BEXT_IGNORES_EBCI and BEXT_E_FIELD_IGNORED each fail their must-fail vector and only cpu.BEXT. vectors."
    requirement: CPU-01
    verification:
      - kind: integration
        ref: "sh tools/gate.sh (faults=38/38; build/gate/faults/BEXT_IGNORES_EBCI/cpu-vectors.txt, build/gate/faults/BEXT_E_FIELD_IGNORED/cpu-vectors.txt)"
        status: pass
    human_judgment: false
  - id: D2
    description: "J, JE, JD, JSR, JSRE and JSRD load the 16-bit target from the pdf 52 fields, save PC+3 in R4, R5 or R6, and set or clear INTFF by DE. That is 11 jump vectors with 12 cycles. JSR_RETURN_ADDR and JUMP_TARGET_BITS each fail their must-fail vector, and all 74 rows have vectors (rows-pending=0)."
    requirement: CPU-01
    verification:
      - kind: integration
        ref: "sh tools/gate.sh and dash tools/gate.sh (gate: PASS vectors=319 source=278 policy=41 rows-pending=0 faults=38/38 tests-expected=680 tests-executed=680)"
        status: pass
    human_judgment: false

duration: 10min
completed: 2026-11-10
status: complete
---

# Phase 2 Plan 12: BEXT and Jump Group Summary

**BEXT now asks the port's `ebci(ctx, E)` provider with E = the decle's low 4 bits, after the displacement read. A NULL provider means false (C57). Six vectors cover E codes 5 and 10 in four directions, plus the two POL-BR-01 cycle values. The jump family (J, JE, JD, JSR, JSRE, JSRD) runs through a new `ivs_jump`. It takes the target from bits 7..2 of word 2 and bits 9..0 of word 3, saves PC+3 in R4, R5 or R6, and sets or clears INTFF by DE. Eleven hand-derived vectors cover it. Four new named faults guard the rules. All 74 rows now have vectors.**

## Performance

- **Duration:** about 10 min
- **Started:** 2026-11-10T08:26:07Z
- **Completed:** 2026-11-10T08:36Z
- **Tasks:** 2 of 2
- **Files modified:** 13 (1069 insertions, 17 deletions)

## Accomplishments

- **BEXT.** The decode row now routes BEXT to `IVS_CP1610_OP_BRANCH`. In `ivs_branch`, bit 4 picks the condition source:
  - bit 4 = 0 uses `ivs_branch_cond`, as before;
  - bit 4 = 1 uses `ivs_branch_ext`, which asks `port->ebci(ctx, decle & $F)`.
  
  The displacement read, the direction rule and POL-BR-01 cycles are the same shared path.
- **The test port's ebci semantics.** Before this plan, a vector with `"ebci": []` got a NULL provider, because the runner tested `n_ebci > 0`. The generator now emits `has_ebci`:
  - key absent: NULL provider;
  - key present, even empty: a provider that answers true exactly for the listed codes.
  
  `cpu.BEXT.back-not-taken-e5` uses the empty-provider path and `cpu.BEXT.cycles-not-taken` the NULL path. The generated table holds `1u` and `0u` for them.
- **Jumps.** `ivs_jump` works like this:
  - It reads word 2 at offset 4 and word 3 at offset 8, both through `ivs_imm_addr`, so R7 is PC+3 after the reads.
  - The target is `(word2 >> 2 & $3F) << 10 | (word3 & $3FF)`.
  - BB = 0..2 saves R7 (PC+3) in R4+BB. BB = 3 saves nothing.
  - DE = 01 sets INTFF, DE = 10 clears it, and DE = 00 leaves it.
  - A jump takes 12 cycles and is interruptible.
  - DE = 11 stops after word 2: R7 goes back to the offender and the unit reports the temporary UNIMPLEMENTED fault, taking 8 cycles.
- **Vectors.**
  - `branch.json` has 98 vectors, 31 of them POL-BR-01: 29 + `cpu.BEXT.cycles-taken` (9) + `cpu.BEXT.cycles-not-taken` (7).
  - `jump.json` has 11 vectors.
  - Every new vector passed on its first run.
- **Gate.** The pending list has no rows left (`rows-pending=0`, 14 policies pending). The gate runs 319 vectors, 680 tests and 38 faults.

## Task Commits

1. **Task 1: Tracer, BEXT end to end through the ebci provider** - `d0a5137` (feat). Gate: `faults=36/36 rows-pending=1`. Tracer gate: `sh tools/gate.sh` ran green before Task 2 started.
2. **Task 2: J, JE, JD, JSR, JSRE, JSRD with return address and INTFF effects** - `da5679d` (feat). Gate: `faults=38/38 rows-pending=0`.

**Plan metadata:** recorded in the docs(02-12) completion commit.

## Bus offsets and the pages that print them

| Access | Offsets | Basis | Page |
|--------|---------|-------|------|
| BEXT (taken and not taken) | FETCH of the BEXT word at 0; READ of the displacement at PC+1 at 4 | figure | pdf 32 "Branch on External Instruction": BAR "output program counter", NACT, DTB "input BEXT instruction", NACT, BAR "output PC+1 to fetch displacement". EBCI is "valid input throughout TS1" of that second BAR. The EBCI sample is not a bus access and is not listed. |
| J family | FETCH at 0; READ of word 2 (PC+1) at 4; READ of word 3 (PC+2) at 8 | inference | No jump timing figure exists. The 12 cycles of pdf 56 are read as three 4-cycle word transfers like the fetch of pdf 28 (C36). `cpu.J.plain` carries the list with `bus_basis` inference. |

## Vector disagreements and resolutions

None against the core. All 17 new vectors passed on their first run. No `expect` was edited after a run, and no core change was made to match a vector.

One vector was changed before it ever ran:

- **`cpu.J.target-ffff` encoding.** The plain J encoding for target $FFFF is second word 11 111111 00. In that word, bits 9..4 (111111) equal bits 7..2, so the planned JUMP_TARGET_BITS mutation would still produce $FFFF and could not fail its must-fail vector. The vector therefore uses JSR R4 (BB = 00, second word $0FC). There the faulty reading of bits 9..4 (001111) gives $3FFF. This changes only which encoding the vector uses, not any expected value. The derivation states the reason.

Page readings for the plan 02-17 second reader to re-derive independently:

- **BEXT E field.**
  - pdf 54 prints `1000 S1 EEEE`.
  - pdf 11 says EBCA0-3 are "the buffered outputs from the 4 least significant bits of the instruction register (bits 0-3)".
  - EBCI is "high active such that a logic '1' would test as true and cause the branch".
- **EBCI sample point.** pdf 32 shows EBCI "valid input throughout TS1" at the displacement BAR. That makes the order "displacement READ, then ask the provider" a reading of the figure. The vectors do not observe this order. Only BEXT_IGNORES_EBCI and BEXT_E_FIELD_IGNORED observe the call itself.
- **Jump target fields.** pdf 52: "bits 2-7 of the second word as bits 10-15 of the jump address and ... bits 0-9 of the third word as bits 0-9". Fig 3.3.5 labels word 2's high bits "Future Use" and word 3's high bits "Not Used". The core ignores both. No vector puts nonzero bits there (C51 left the wide-word question to Phase 3).
- **Return address.** pdf 56 prints "(PC+3)" legibly in the JSR, JSRE and JSRD rows (C58). pdf 52 prints it smudged. PC is the address of the first word, so the return address is $1003 for a program at $1000.
- **BB = 3.** pdf 52 says the designated register "becomes 7 (the PC) which is the jump instruction". The reading is that the plain jump saves nothing, because R7 is then loaded with the target.
- **Interruptibility.** The jump rows of pdf 56 carry no "Not Interruptable" note, so every jump vector shows prev_interruptible 0 -> 1.

## Fault verdicts (latest gate run)

| Fault | Vectors that fail (all inside the allowed prefixes) |
|-------|------------------------------------------------------|
| BEXT_IGNORES_EBCI | `cpu.BEXT.fwd-taken-e5`, `cpu.BEXT.back-taken-e10` and `cpu.BEXT.cycles-taken` |
| BEXT_E_FIELD_IGNORED | the same 3 (the provider asked for E = 0 always answers false); must-pass `cpu.BEXT.fwd-not-taken-e10` passes |
| JSR_RETURN_ADDR | the 7 vectors that save a return address (`jsr-r4/r5/r6-return`, `jsre-r5`, `jsrd-r4`, `jsr-return-wraps` and `target-ffff`); must-pass `cpu.J.plain` passes |
| JUMP_TARGET_BITS | all 11 `cpu.J.` vectors; must-pass `cpu.MVII.r1-load` passes |
| IMMEDIATE_PC_ADVANCE (existing) | now also the 6 BEXT and the 11 J vectors |
| BRANCH_BACKWARD_SIGN, BRANCH_TAKEN_CYCLES, BRANCH_NOT_TAKEN_CYCLES (existing) | their existing `cpu.BEXT.` prefixes now catch `back-taken-e10`, `cycles-taken` and `cycles-not-taken` |

## Decisions Made

- **One branch handler.** BEXT shares `ivs_branch`, and its condition lives in its own function, `ivs_branch_ext`. That function holds the two BEXT fault sites, one line each.
- **Provider answer.** Any nonzero answer is true (T-02-25: the result is treated as a boolean).
- **Refusal after word 1.** `ivs_unit_instr` now checks `run_state == FAULTED` after `ivs_exec` returns. A refusal found after the first word (jump DE = 11) then skips the prev_interruptible update and reports `IVS_JUMP_DE_FAULT_CYCLES` (8). A throwaway probe in session scratch confirmed the result:
  - unit FAULT, 8 cycles, 2 traced reads;
  - R7 back at $1000;
  - R4 and INTFF untouched.
  
  No vector covers this path yet. Plan 02-13 adds `cpu.J.de11-unsupported`.
- **Flag backgrounds.** Each row gets its four `flag-preserved` proofs from forward or plain vectors whose flags start at 1 (`cpu.BEXT.fwd-taken-e5`, `fwd-not-taken-e10`, `cpu.J.plain`, `cpu.J.backward-target`). No flag exemptions were added.
- **INTFF coverage.** Each DE value is shown in both directions:
  - set: `je-sets-intff`, `jsre-r5`;
  - clear: `jd-clears-intff`, `jsrd-r4`;
  - kept at 1: `plain`, `jsr-r4-return`;
  - kept at 0: `jsr-r5-return` and others.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] The test port passed a NULL provider for an empty `ebci` list**
- **Found during:** Task 1 (step 1: confirm the test port's semantics)
- **Issue:** `cpu_vectors.c` called `ivs_testbus_port(..., v->n_ebci > 0u)`, so `"ebci": []` behaved like an absent key. That breaks the plan's rule: "present (even empty) means a provider".
- **Fix:** The fix belongs in the generator and runner, not in `tests/harness/ivs_testbus.c`. The generator now emits `has_ebci`, `ivs_vec` has a new `has_ebci` field, and the runner passes `v->has_ebci`.
- **Files modified:** `cmake/IvsVectorTable.cmake`, `tests/harness/ivs_vec.h`, `tests/cpu/cpu_vectors.c`
- **Verification:** The generated table holds `1u` for `cpu.BEXT.back-not-taken-e5` and `0u` for `cpu.BEXT.cycles-not-taken`, and the gate passes.
- **Committed in:** d0a5137

**2. [Rule 3 - Fault prefix] IMMEDIATE_PC_ADVANCE gained `cpu.BEXT.` and `cpu.J.`**
- **Found during:** Tasks 1 and 2
- **Issue:** Both rows read their operand words through `ivs_imm_addr`, the single PC-advance site. The existing fault therefore fails their vectors, which fall outside its allowed prefixes.
- **Fix:** Each task added its own stem in the same commit (the plan 02-08/02-11 rule; no bare `cpu.` prefix).
- **Committed in:** d0a5137, da5679d

**3. [Rule 3 - Files] Header, decode table and generator edited**
- **Found during:** Tasks 1 and 2
- **Issue:** The plan lists `cp1610.c` and `ivs_testbus.c` only. The work also needed:
  - an op id in `cp1610.h` (`IVS_CP1610_OP_JUMP`);
  - the BEXT and J routing in `cp1610_decode.c`;
  - the ebci fix (item 1).
- **Fix:** Those files were edited. `ivs_testbus.c` needed no change.

**4. [Rule 1 - Vector design] `cpu.J.target-ffff` uses the JSR R4 form**
- **Found during:** Task 2
- **Issue:** With a plain J encoding, the planned JUMP_TARGET_BITS mutation (bits 9..4) gives the same target, $FFFF, so the must-fail vector would pass.
- **Fix:** The vector uses BB = 00, second word $0FC. The derivation says why. The inventory line stays exactly as planned.
- **Committed in:** da5679d

---

**Total deviations:** 4 auto-fixed (1 harness bug, 1 prefix extension, 1 file list, 1 vector encoding choice).
**Impact on plan:** No scope change. Every acceptance count holds:
- 31 POL-BR-01 vectors;
- 4 BEXT direction ids;
- 2 BEXT fault lines;
- 0 rows pending;
- 11 `"cpu.J.` ids;
- `faults=38/38`, `rows-pending=0`, and the gate passes under sh and dash.

## Measured cost (D-22 budget input)

- **Final `sh tools/gate.sh`:** 43.1 s wall, unpiped (output to a file). The machine was under concurrent load. The summary line is `gate: PASS steps=24/24 controls=182/182 ... vectors=319 source=278 policy=41 rows-pending=0 policies-pending=14 bus-figure-pages=25 ... faults=38/38 ... tests-expected=680 tests-executed=680 tests-skipped=0 tests-missing=0`.
- **`dash tools/gate.sh`:** 37.2 s, `gate: PASS`.
- **Task 1 gate:** 36.4 s. For comparison, plan 02-11 measured 36.5 s.

## Issues Encountered

None.

## Known Stubs

- Jump DE = 11 reports the temporary UNIMPLEMENTED fault reason. This is by design: plan 02-13 replaces it with `UNSUPPORTED_JUMP_DE`, deletes reason 5 and adds `cpu.J.de11-unsupported`. Plan 02-13 tracks it, so it is not added to `.planning/WINDOWS.md`.

## User Setup Required

None.

## Next Phase Readiness

- **Plan 02-13 (encoding, SDBD):**
  - The DE = 11 path already stops after word 2 with R7 at the offender and the reads traced. Only the reason constant changes.
  - `ivs_unit_instr` already handles a fault raised inside `ivs_exec`.
  - No row uses `IVS_CP1610_OP_NONE` any more, so the UNIMPLEMENTED reason is now reached only through jump DE = 11.
- **Plan 02-14 (interrupts):** JE and JSRE set INTFF inside the unit, and the jump rows are interruptible. That gives POL-INT-03 its "JE takes effect before the boundary test" witness.
- **Plan 02-17 (second reader):** read the "Page readings" list above. The `has_ebci` distinction belongs to the vector schema the protocol document must state: absent means no provider, present means a provider answering true for the listed codes.
- **Unasserted inferences:** the jump bus offsets (READ at 4 and 8), and the jump target ignoring the high bits of words 2 and 3.

---
*Phase: 02-cp-1610-cpu-core*
*Completed: 2026-11-10*

## Self-Check: PASSED

- The key files exist: `data/cp1610/vectors/jump.json`, `data/cp1610/vectors/branch.json`, `core/src/cpu/cp1610.c` and this SUMMARY.
- Commits `d0a5137` and `da5679d` are ancestors of HEAD. The ledger measures `git rev-list --count f84c658..HEAD` = 2.
- Both tasks' acceptance commands were re-run and printed the expected values: 31; 4; 2; 0; 11.
- Final gate: `sh tools/gate.sh` gate: PASS (43.1 s) and `dash tools/gate.sh` gate: PASS (37.2 s).
