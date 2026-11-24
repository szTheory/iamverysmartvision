---
phase: 02-cp-1610-cpu-core
plan: 14
subsystem: cpu-core
tags: [c17, cp1610, interrupts, intr, intrm, intff, iab, int-entry, pol-int-01, pol-int-02, pol-int-03, pol-int-04, pol-line-01, pol-iab-01, pol-sdbd-03, pol-rst-01, vectors, named-faults, cpu-02, cpu-04, cpu-05]

requires:
  - phase: 02-cp-1610-cpu-core
    plan: 13
    provides: SDBD double byte data and the DBD flag consumed by the target, the decode table's interruptible column (C56 defaults), the refusal path and the reset entry unit
provides:
  - ivs_int_accept - the acceptance test at the start of a RUNNING step (prev_interruptible open, DBD clear, INTR or INTRM with INTFF; levels, never latched)
  - ivs_unit_int_entry - the 9-cycle INT_ENTRY unit (INTAK at 0 with R6, DW of R7 at 2, R6 + 1, IAB at 5 loads R7), prev_interruptible 0 afterwards, INTFF cleared only under config interrupt_accept_clears_intff = 1
  - ivs_iab - one IAB helper shared by the reset entry and the interrupt entry; the core holds no vector constant
  - data/cp1610/vectors/interrupt.json (group interrupt, 39 vectors, all kind policy)
  - 10 named faults INT_ENTRY_ORDER PREV_INTERRUPTIBLE_IGNORED INTFF_IGNORED LINES_LATCHED MVO_INTERRUPTIBLE ACCEPT_CLEARS_INTFF_IGNORED JE_AFTER_BOUNDARY IAB_HARDCODED_1004 SDBD_INTERRUPTIBLE FIRST_ISR_INSTR_INTERRUPTIBLE
  - POL-INT-01..04, POL-LINE-01, POL-IAB-01 and POL-SDBD-03 no longer pending; only POL-BUS-01, POL-WAIT-01, POL-HLT-01, POL-HLT-02 remain for plan 02-15
affects: [02-15, 02-16, 02-17, 02-18]

actuals:
  tokens: 35700
  tasks: 3
  commits: 3
plan_head_before: ed193d0bbc4a596e80bdc314063ed819fc86c507
plan_head_after: f757bdfbef233d137b5ab962040746fd773db635

tech-stack:
  added: []
  patterns:
    - "Acceptance is a pure predicate over (state, lines) evaluated before the unit is chosen; the entry is a unit kind of its own, so a step is still exactly one unit"
    - "Every interrupt vector that must run its first row before any test sets init.prev_interruptible 0 explicitly and keeps INTR high throughout; vectors that test a single boundary use step_lines instead"

key-files:
  created:
    - data/cp1610/vectors/interrupt.json
  modified:
    - core/src/cpu/cp1610.c
    - data/expected/cp1610-vector-files.ids
    - data/expected/cp1610-vectors.ids
    - data/expected/tests/cpu-vectors.ids
    - data/expected/cp1610-coverage-pending.ids
    - data/expected/faults/cpu.ids

key-decisions:
  - "The acceptance test runs only in RUNNING state at the start of a step: open = prev_interruptible and DBD clear; INTR always, INTRM only with INTFF. RESET_PENDING never reads the lines; HALTED and BUSRQ stay plan 02-15."
  - "The INT_ENTRY bus records are INTAK at 0 (addr R6), DW at 2 (addr R6, data old R7) and IAB at 5 (addr 0, flags INTERRUPT); the unit is 9 cycles and takes no waits (A-11). The DW record sits at DW, one cycle before DWS (pdf 10)."
  - "The INTAK record's data field is whatever the bus replied (the flat test bus answers $0000); the CPU ignores it. The vectors assert $0000 for it."
  - "All 39 interrupt vectors are kind policy: the one-unit deferral and every entry depend on POL-INT-01/04 even where the row's Not Interruptable note is printed (pdf 53, pdf 56)."
  - "ivs_iab is shared by the reset and interrupt entries, so IAB_HARDCODED_1004 hits the reset provider vectors too, as the plan's prefix column expects."
  - "JE_AFTER_BOUNDARY is modelled as JE and JSRE leaving INTFF unchanged at their own unit end: the 32-byte state has no field that could carry a pending enable across the boundary, so a JE whose enable lands after the boundary test reads INTFF 0 at its unit end."
  - "MVO_INTERRUPTIBLE flips the row flag after the unit (the acceptance test cannot know the previous row), so the 12 existing MVO-family vectors that assert prev_interruptible 0 fail with it."
  - "requirements-completed stays empty: requirements.ready-ids reports CPU-02, CPU-04 and CPU-05 blocked by sibling plans that still declare them."

patterns-established:
  - "Fault prefixes grow only by exact vector ids (never a bare cpu. stem); every extension is listed in this SUMMARY with the reason it legitimately fails"

requirements-completed: []
requirements-advanced: [CPU-02, CPU-04, CPU-05]

coverage:
  - id: D1
    description: "An accepted interrupt is one 9-cycle INT_ENTRY unit: INTAK at 0 with R6, DW of the old R7 at R6 at 2, R6 + 1, IAB at 5 loading all 16 bits of R7 from the bus; flags not pushed; prev_interruptible 0 so the first service routine instruction runs (cpu.UNIT-INT.entry-figure, figure basis pdf 35; cpu.UNIT-INT.first-isr-instruction-runs). INT_ENTRY_ORDER and FIRST_ISR_INSTR_INTERRUPTIBLE fail their must-fail vectors."
    requirement: CPU-05
    verification:
      - kind: integration
        ref: "sh tools/gate.sh (faults=55/55; build/gate/faults/INT_ENTRY_ORDER/cpu-vectors.txt, build/gate/faults/FIRST_ISR_INSTR_INTERRUPTIBLE/cpu-vectors.txt)"
        status: pass
    human_judgment: false
  - id: D2
    description: "Acceptance matrix: INTR always, INTRM only with INTFF, all lines low no entry, levels never latched; the 17 literal non-interruptible rows each defer INTR by exactly one unit and SIN, NOP, ADDR accept right after; EIS, JE, DIS, GSWD and both config values for INTFF on acceptance; IAB providers $0000, $1004, $ABCD. PREV_INTERRUPTIBLE_IGNORED, INTFF_IGNORED, LINES_LATCHED, MVO_INTERRUPTIBLE, ACCEPT_CLEARS_INTFF_IGNORED, JE_AFTER_BOUNDARY and IAB_HARDCODED_1004 each fail their must-fail vector."
    requirement: CPU-02
    verification:
      - kind: integration
        ref: "sh tools/gate.sh and dash tools/gate.sh (gate: PASS vectors=390 policy=99 policies-pending=4 faults=55/55)"
        status: pass
    human_judgment: false
  - id: D3
    description: "SDBD followed by its target with INTR high takes no entry between them (POL-SDBD-03); with INTR high during reset the reset entry comes first and the first instruction runs before any acceptance; reset from FAULTED clears fault_reason and DBD. SDBD_INTERRUPTIBLE fails its must-fail vector."
    requirement: CPU-04
    verification:
      - kind: integration
        ref: "sh tools/gate.sh (cpu.MVII.sdbd-then-intr-deferred, cpu.UNIT-RESET.intr-during-reset, cpu.UNIT-RESET.from-faulted; build/gate/faults/SDBD_INTERRUPTIBLE/cpu-vectors.txt)"
        status: pass
    human_judgment: false

duration: 18min
completed: 2026-11-10
status: complete
---

# Phase 2 Plan 14: Interrupt Acceptance and Entry Summary

**A RUNNING step now tests the interrupt lines before it fetches. After an interruptible unit with DBD clear, INTR, or INTRM with INTFF set, starts a 9-cycle INT_ENTRY unit instead of a fetch: INTAK puts out R6, DW writes the old PC at R6, R6 goes up by one and IAB loads R7 from the bus reply. The entry leaves prev_interruptible 0, so the first service routine instruction always runs. Lines are levels and are never latched. The 17 non-interruptible rows (8 shifts, EIS, DIS, TCI, CLRC, SETC and the MVO family) each have a literal vector showing they defer INTR by exactly one unit. Ten named faults guard the rules. The new file `interrupt.json` holds 39 hand-derived vectors, and every one passed on its first run.**

## Performance

- **Duration:** about 18 min
- **Started:** 2026-11-10T09:00:49Z
- **Completed:** 2026-11-10T09:18:35Z
- **Tasks:** 3 of 3
- **Files modified:** 7 (3901 insertions, 12 deletions)

## Accomplishments

- **Acceptance.** `ivs_int_accept(n, lines)` runs only in RUNNING state.
  - It returns 0 when `prev_interruptible` is 0 or DBD is set.
  - Otherwise it accepts on INTR, or on INTRM with INTFF set.
  - It reads the lines given to this step only, so nothing is latched.
  - RESET_PENDING never reads the lines. A comment at that case cites Fig 2-6 (pdf 22).
- **Entry.** `ivs_unit_int_entry` produces these records and changes:
  - INTAK at 0 (addr R6);
  - DW at 2 (addr R6, data old R7; kind DW, never WRITE, A-11);
  - R6 + 1 mod 2^16;
  - IAB at 5 (addr 0, flag INTERRUPT), whose reply is the new R7;
  - 9 cycles; flags untouched; `prev_interruptible` 0;
  - INTFF cleared only when `cfg->interrupt_accept_clears_intff` is 1.
- **IAB.** `ivs_iab` is the single IAB helper for the reset entry and the interrupt entry. The core holds no vector constant.
- **Vectors.** `interrupt.json` (group `interrupt`) has 39 policy vectors:
  - 2 from the tracer;
  - 33 for the acceptance matrix: 4 POL-INT-01, 1 POL-LINE-01, 17 `defers-intr`, 3 `.interruptible`, 6 POL-INT-03 and 2 POL-IAB-01;
  - 4 for SDBD, reset and the first ISR instruction.
- **Gate.** `vectors=390 source=291 policy=99 rows-pending=0 policies-pending=4 bus-figure-pages=31 faults=55/55 tests-expected=972 tests-executed=972`.

## Task Commits

1. **Task 1: Tracer, INTR after NOP produces the 9-cycle INT_ENTRY end to end** - `203adb8` (feat). Gate `faults=46/46`. Tracer feedback gate: `sh tools/gate.sh` re-ran green (37 s) and `INT_ENTRY_ORDER` failed exactly `cpu.NOP.then-intr-entry` and `cpu.UNIT-INT.entry-figure` before Task 2 started.
2. **Task 2: Acceptance matrix, the literal non-interruptible list, seven faults** - `147e8d7` (feat). Gate `faults=53/53`.
3. **Task 3: SDBD deferral, reset interaction, the first ISR instruction rule, two faults** - `f757bdf` (feat). Gate `faults=55/55`, sh and dash.

**Plan metadata:** recorded in the docs(02-14) completion commit.

## Bus offsets and the pages that print them

| Access | Offset in the entry unit | Basis | Page |
|--------|--------------------------|-------|------|
| INTAK (output stack pointer) | 0 | figure | pdf 35; SC.6 step 1, pdf 24 |
| DW (write current PC onto the stack) | 2 (DWS at 3 is not a record) | figure | pdf 35; DW "one machine cycle time prior to DWS", pdf 10 |
| IAB (input service routine address) | 5 | figure | pdf 35; SC.6 step 3, pdf 25 |
| First BAR of the service routine | 9 = the next unit's FETCH | figure | pdf 35 |

The entry figure counts INTAK, NACT, DW, DWS, NACT, IAB, NACT, NACT, NACT and then BAR. That makes 9 cycles (C30).

## Vector disagreements and resolutions

None against the core.
- All 39 vectors passed on their first run, in Tasks 1, 2 and 3.
- No `expect` value was edited after a run.
- No core change was made to match a vector.

Page readings for the plan 02-17 second reader to re-derive independently:
- **Acceptance condition.**
  - Fig 2-6 (pdf 22) tests `INTR + INTRM . INTFF` only on the INTERRUPTABLE INSTRUCTION branch. An UNINTERRUPTABLE INSTRUCTION returns straight to the fetch.
  - pdf 12: "INTR* is always honored", and "INTRM* is honored ... only if ... INTFF, is set".
  - SC.6 (pdf 24) gives the same rule.
- **Not interruptable.** "Shifts are not interruptable." is in the shift table header (pdf 53). EIS, DIS, TCI, CLRC and SETC each print "Not Interruptable." (pdf 56). The MVO family follows C56, and SDBD and HLT follow C56 as well.
- **SIN.** pdf 13: "The interrupt will be acknowledged at the end of the SIN instruction", so SIN is interruptible.
- **First ISR instruction.** In Fig 2-6 the INPUT DEVICE PC (IAB) box returns to the fetch with no test. pdf 25: "The CPU will then return to the Fetch Sequence".
- **Reset.** In Fig 2-6, MSYNC leads to INPUT STARTING PC (IAB) and then to the fetch, with no test. SC.1 (pdf 21) says registers and status are undefined after MSYNC. The zero-fill is the profile default under C32.
- **Sampling.** pdf 25: "the CPU samples the INTR* and INTRM* inputs every TS1". The CPU-side rule is levels per step (D-05). The sampling point is the machine-layer rule C38.
- **INTFF on acceptance.** The page is silent, which is C41. The default leaves INTFF unchanged, and the vectors cover both config values.
- **JE before the boundary.** This is C41, default-chosen. `cpu.J.je-then-intrm-immediate` takes INTRM at the boundary right after JE.

## Fault verdicts (latest gate run)

| Fault | Site | Vectors that fail |
|-------|------|-------------------|
| INT_ENTRY_ORDER | entry: an IAB before the DW, so the new PC is pushed | all 34 vectors that contain an INT_ENTRY |
| PREV_INTERRUPTIBLE_IGNORED | acceptance: `open = 1` | the 17 `defers-intr`, `CLRC.level-not-latched`, `EIS.then-intrm-after-next`, the 3 `.interruptible`, `MVII.sdbd-then-intr-deferred`, `UNIT-INT.first-isr-instruction-runs`, `UNIT-RESET.intr-during-reset` |
| INTFF_IGNORED | acceptance: `intff = 1` | `UNIT-INT.intrm-intff-clear-ignored`, `DIS.blocks-intrm` |
| LINES_LATCHED | acceptance: a static latch of INTR, cleared at an open boundary | `CLRC.level-not-latched` |
| MVO_INTERRUPTIBLE | after the unit: MVO, MVO@, MVOI, PSHR set prev_interruptible 1 | the 4 MVO-family `defers-intr` and 12 existing MVO-family vectors |
| ACCEPT_CLEARS_INTFF_IGNORED | entry: `clears = 0` | `UNIT-INT.accept-clears-intff-config` |
| JE_AFTER_BOUNDARY | jump: DE = 01 treated as 00 | `J.je-then-intrm-immediate`, `J.je-sets-intff`, `J.jsre-r5` |
| IAB_HARDCODED_1004 | `ivs_iab`: the reply is replaced by $1004 | `UNIT-INT.iab-0000`, `UNIT-INT.iab-abcd`, `UNIT-RESET.provider-0000`, `UNIT-RESET.provider-ffff`, `UNIT-RESET.from-faulted` |
| SDBD_INTERRUPTIBLE | acceptance: with DBD set, the boundary is treated as open | `MVII.sdbd-then-intr-deferred` |
| FIRST_ISR_INSTR_INTERRUPTIBLE | entry: prev_interruptible 1 | all 34 vectors that contain an INT_ENTRY |

Every must-pass vector passes. Existing faults needed no change: RESET_ENTRY (prefix `cpu.UNIT-RESET.`) also catches the two new reset vectors, and EIS_NO_INTFF and SDBD_DBD_NOT_CLEARED (stems `cpu.EIS.` and `cpu.MVII.sdbd`) cover the new ids.

### Prefix extensions (Task 1 rule: exact ids only)

- **INT_ENTRY_ORDER.** The plan line is `cpu.UNIT-INT.|cpu.NOP.then-intr`. It gains every later vector that contains an entry:
  - the 17 `defers-intr`;
  - the 3 `.interruptible`;
  - `cpu.EIS.then-intrm-after-next`, `cpu.J.je-then-intrm-immediate`, `cpu.GSWD.intff-unchanged`;
  - `cpu.MVII.sdbd-then-intr-deferred`, `cpu.UNIT-RESET.intr-during-reset`.
- **PREV_INTERRUPTIBLE_IGNORED.** It gains these ids. Each starts at a closed boundary with INTR high, so a core that ignores the boundary accepts too early:
  - `cpu.SIN.interruptible`, `cpu.NOP.interruptible`, `cpu.ADDR.interruptible` (Task 2, see deviation 1);
  - `cpu.MVII.sdbd-then-intr-deferred`, `cpu.UNIT-INT.first-isr-instruction-runs`, `cpu.UNIT-RESET.intr-during-reset` (Task 3).
- **MVO_INTERRUPTIBLE.** It gains 12 existing ids (deviation 2):
  - `cpu.MVO.write-sequence`, `cpu.MVO.address-ffff`, `cpu.MVO.overwrites-word`;
  - `cpu.MVOI.store-next-word`, `cpu.MVOI.flags-preserved`, `cpu.MVOI.r3-ffff`;
  - `cpu.PSHR.write-then-increment`, `cpu.PSHR.r6-wrap-ffff`;
  - `cpu.MVO@.r2-no-increment`, `cpu.MVO@.r4-autoinc`, `cpu.MVO@.r5-wrap-ffff`, `cpu.MVO@.r1-sss-eq-mmm-allowed`.
- **JE_AFTER_BOUNDARY.** It gains `cpu.J.je-sets-intff` and `cpu.J.jsre-r5` (deviation 3).
- **FIRST_ISR_INSTR_INTERRUPTIBLE.** Its prefix column is the exact list of the 34 ids with an INT_ENTRY, as planned.

## Decisions Made

See `key-decisions` in the frontmatter. The main ones:
- acceptance is a predicate evaluated before the unit is chosen;
- all interrupt vectors are kind policy;
- one IAB helper serves both entries;
- the JE and MVO fault models are as described there.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Fault design] PREV_INTERRUPTIBLE_IGNORED prefixes include the three `.interruptible` ids**
- **Found during:** Task 2
- **Issue:** The plan fixes the line exactly and also asks that `cpu.SIN.interruptible`, `cpu.NOP.interruptible` and `cpu.ADDR.interruptible` set `init.prev_interruptible` 0 with INTR high throughout. A core that ignores `prev_interruptible` accepts at step 1 of those vectors, so they fail under the fault and would break the verdict.
- **Fix:** Added the three exact ids to the prefix column, the same treatment as the Task 1 rule.
- **Committed in:** 147e8d7

**2. [Rule 1 - Fault design] MVO_INTERRUPTIBLE sits at the row flag; 12 existing ids added**
- **Found during:** Task 2
- **Issue:** The acceptance test sees only `prev_interruptible`, never the previous row. The only site that can make the MVO family interruptible is where the row flag is copied after the unit. The earlier MVO-family vectors (plans 02-09/02-10) assert `prev_interruptible` 0 after the unit, so they fail too. That is correct detection, but the plan's prefix lists only the `defers` stems.
- **Fix:** Extended the prefix by the 12 exact ids listed above.
- **Committed in:** 147e8d7

**3. [Rule 1 - Fault design] JE_AFTER_BOUNDARY as "JE leaves INTFF unchanged at its unit end"**
- **Found during:** Task 2
- **Issue:** A faithful "enable after the boundary test" needs a pending bit carried across the step boundary. The 32-byte state (D-03) has no such field, and adding one would be an architectural change. Any mutant that can be expressed therefore shows the wrong INTFF (or prev_interruptible) at the JE unit's end, so `cpu.J.je-sets-intff` and `cpu.J.jsre-r5` fail too.
- **Fix:** One site in `ivs_jump` (`de = de == 1u ? 0u : de;`). The two existing ids were added to the prefix.
- **Committed in:** 147e8d7

**4. [Rule 1 - Fault design] LINES_LATCHED uses a static inside the fault site**
- **Found during:** Task 2
- **Issue:** Latching needs memory across steps, and the state has no field for it.
- **Fix:**
  - The site is a 1-line block with a function-scope `static` latch. It is set by INTR and cleared at every open boundary, where a latched INTR is always taken.
  - Only the fault executable compiles it. The plain build, the c-purity compile and the symbol checks never define it.
  - Every vector in the current inventory consumes a latched INTR before it ends, so nothing carries over to the next vector. The verdict shows only `cpu.CLRC.level-not-latched` failing.
- **Committed in:** 147e8d7

**5. [Rule 1 - Fault design] SDBD_INTERRUPTIBLE opens the boundary while DBD is set**
- **Found during:** Task 3
- **Issue:** SDBD's row flag already makes the boundary after it closed (C56). Removing only the DBD check would therefore leave `cpu.MVII.sdbd-then-intr-deferred` passing, and the fault would go undetected.
- **Fix:** The 1-line site returns the line test when DBD is set, ignoring both the DBD check and `prev_interruptible` at that boundary. This models "the SDBD-to-target boundary is interruptible", the D-11 fault "DBD leaks into the ISR".
- **Committed in:** f757bdf

**6. [Rule 1 - Fault design] INT_ENTRY_ORDER issues an extra early IAB**
- **Found during:** Task 1
- **Issue:** "IAB loads R7 before the push" cannot be written in one site without the IAB access happening before the DW.
- **Fix:** The site does an IAB at offset 2 before the DW (2 lines), and the normal IAB at 5 still follows. The mutant pushes the new PC, and its bus list shows the extra record.
- **Committed in:** 203adb8

---

**Total deviations:** 6 auto-fixed (all fault-site or prefix design, Rule 1).

**Impact on plan:** No scope change. The planned inventory lines are kept verbatim except for exact-id additions to prefix columns. Every acceptance count holds:
- Task 1: INTAK 2 (1 or more needed); figure basis 2; POL-INT-04 pending 0.
- Task 2: 17 `defers-intr`; 0 `cpu.HLT.defers-intr`; 0 pending for the five policies; 20 init prev 0.
- Task 3: 4 POL lines pending; `cpu.UNIT-RESET.intr-during-reset` once; the SDBD vector's init prev 0 once.

## Measured cost (D-22 budget input)

- **Final `sh tools/gate.sh`:** 41 s wall, unpiped (output to a file).
  - Summary line: `gate: PASS steps=24/24 controls=182/182 cc=AppleClang c-presets=2 fault-runs=55 ... vectors=390 source=291 policy=99 rows-pending=0 policies-pending=4 bus-figure-pages=31 ... faults=55/55 ... tests-expected=972 tests-executed=972 tests-skipped=0 tests-missing=0 ...`.
- **`dash tools/gate.sh`:** 36 s, `gate: PASS`.
- **Task gates:** 37 s for Task 1 and 43 s for Task 2. For comparison, plan 02-13 measured 33 s.

## Issues Encountered

None.

## Known Stubs

None.

## User Setup Required

None.

## Next Phase Readiness

- **Plan 02-15 (BUSRQ, waits, HLT):**
  - `ivs_int_accept` is the place for BUSRQ priority. pdf 25 says BUSRQ is honoured before interrupts, and POL-INT-01 says "BUSRQ is tested before INTR".
  - HALTED still returns an idle unit with no line test.
  - The no-wait-on-DW vector can reuse any entry vector with a `waits` entry at the R6 address.
  - **LINES_LATCHED caution:** a POL-HLT-01 vector holding INTR into a HALTED unit never reaches an open boundary in the RUNNING path. In the fault build that leaves the static latch set for the next vector by id. If that vector then fails under LINES_LATCHED, add its exact id or clear the latch on HALTED entry inside the same site.
  - The `cpu.HLT.intr-held-then-halted` vector (A-13) is still owed by 02-15.
- **Plan 02-16/02-17 (second reader):** re-derive from the page readings list above. The entry offsets come from counting pdf 35; C30 is still open, pending a bus trace.
- **Unasserted or inferred:**
  - the INTAK data field (the bus reply, $0000 on the test bus);
  - an INTRM entry with config 1 when INTR is also high (no vector has both lines high).

---
*Phase: 02-cp-1610-cpu-core*
*Completed: 2026-11-10*

## Self-Check: PASSED

- Key files exist: `data/cp1610/vectors/interrupt.json`, `core/src/cpu/cp1610.c` and this SUMMARY.
- Commits `203adb8`, `147e8d7` and `f757bdf` are ancestors of HEAD. The ledger measures `git rev-list --count ed193d0..f757bdf` = 3.
- Every task's acceptance commands were re-run and printed the expected values.
- Final gate: `sh tools/gate.sh` gate: PASS (41 s) and `dash tools/gate.sh` gate: PASS (36 s).
