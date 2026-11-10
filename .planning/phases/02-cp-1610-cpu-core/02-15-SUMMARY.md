---
phase: 02-cp-1610-cpu-core
plan: 15
subsystem: cpu-core
tags: [c17, cp1610, busrq, busak, bus-grant, bdrdy, wait-states, wait-cap, hlt, stpst, halted, pol-bus-01, pol-wait-01, pol-hlt-01, pol-hlt-02, fuzz, xorshift, vectors, named-faults, cpu-02, cpu-04, cpu-05]

requires:
  - phase: 02-cp-1610-cpu-core
    plan: 14
    provides: the interrupt acceptance predicate, the 9-cycle INT_ENTRY unit, the IAB helper and the prefix-extension rule for fault inventory lines
provides:
  - ivs_step_running - the RUNNING step in the order STPST edge, held grant, boundary test (stop > BUSRQ > INTR > INTRM), instruction
  - ivs_step_halted - the HALTED step (STPST edge resumes; BUSRQ granted and released while staying HALTED; otherwise a 1-cycle HALTED unit; INTR/INTRM ignored)
  - ivs_unit_grant - the BUS_GRANT unit (grant and hold 1 cycle, release = config release_latency_cpu_cycles with BUS_RELEASED; prev_interruptible untouched)
  - ivs_boundary_open - the one boundary predicate shared by the stop, grant and interrupt tests
  - data/cp1610/vectors/busrq-halt.json (group busrq-halt, 22 vectors, all kind policy)
  - 12 named faults BUSRQ_PRIORITY GRANT_LOSES_INTERRUPTIBLE GRANT_AFTER_INT_ENTRY RELEASE_LATENCY_IGNORED WAIT_ON_IAB WAIT_ON_DW WAIT_LIMIT_MISSING HLT_IMMEDIATE HLT_INTERRUPTIBLE HALT_PRIORITY HALTED_TAKES_INTR STPST_LEVEL
  - suite cpu-fuzz (tests/cpu/cpu_fuzz.c, 8 fixed seeds) and 4 cpu-api cases (20 in total)
  - docs/cp1610/stepper-conventions.md
  - an empty coverage pending list (rows-pending=0, policies-pending=0)
affects: [02-16, 02-17, 02-18]

actuals:
  tokens: 25310
  tasks: 3
  commits: 3
plan_head_before: a9d455494d55bf863ad1017b52a3654760f6fbf3
plan_head_after: 6fae4e5e628a5eaa22e122687aa752ed5a38a12f

tech-stack:
  added: []
  patterns:
    - "The RUNNING step computes every boundary decision (stop, grant, accept) before choosing one, so the acceptance test always sees the step's lines, and priority faults are one-line overrides"
    - "A seeded robustness suite: in-repo xorshift32, a stateless bus whose replies are a function of seed and address, and per-step invariants keyed on the run state read before the step"

key-files:
  created:
    - data/cp1610/vectors/busrq-halt.json
    - tests/cpu/cpu_fuzz.c
    - data/expected/tests/cpu-fuzz.ids
    - docs/cp1610/stepper-conventions.md
  modified:
    - core/src/cpu/cp1610.c
    - core/src/cpu/cp1610_decode.c
    - tests/cpu/cpu_api.c
    - tests/CMakeLists.txt
    - data/expected/cp1610-vector-files.ids
    - data/expected/cp1610-vectors.ids
    - data/expected/tests/cpu-vectors.ids
    - data/expected/tests/cpu-api.ids
    - data/expected/cp1610-coverage-pending.ids
    - data/expected/faults/cpu.ids

key-decisions:
  - "Step start in RUNNING: the STPST edge toggles the stop latch first; a held grant (BUSAK 1) is served next (hold 1 cycle while BUSRQ is high, else the release unit); then at an interruptible boundary stop > BUSRQ > INTR > INTRM; a closed boundary fetches."
  - "A set stop latch at an interruptible boundary gives a 1-cycle HALTED unit that clears the latch, enters HALTED and reports STOPPED; later HALTED idle units report no event."
  - "In HALTED a STPST edge resumes RUNNING and the same step continues as a RUNNING step (so the resume step is normally the next fetch); resume also clears halt_pending."
  - "A STPST edge sets the same latch as HLT but does not report HALT_LATCHED; that event stays the mark of an executed HLT."
  - "The BUS_WAIT_LIMIT fault unit now ends one cycle past the cap (address cycle + limit + 1) instead of after the full answered wait, so its length is bounded by the config, never by the device reply."
  - "A config with release_latency_cpu_cycles 0 is refused with IVS_E_BAD_ARG, so no BUS_GRANT unit can take 0 cycles."
  - "GRANT_AFTER_INT_ENTRY is modelled as the grant test ignoring the closed boundary: the 32-byte state cannot tell 'after an entry' from 'after a non-interruptible unit', so the CLRC no-grant vector fails with it too (prefix extended by that exact id)."
  - "requirements-completed stays empty: CPU-02, CPU-04 and CPU-05 are still declared by plans 02-17 and 02-18."

patterns-established:
  - "Fault prefixes grow only by exact vector ids; every extension is listed in this SUMMARY with the reason it legitimately fails"

requirements-completed: []
requirements-advanced: [CPU-02, CPU-04, CPU-05]

coverage:
  - id: D1
    description: "BUSRQ and INTR at the same interruptible boundary give grant, hold, release (2 cycles) and then INT_ENTRY; a grant keeps prev_interruptible (A-02); no grant after a non-interruptible unit or directly after an entry; the release latency comes from the config (cpu.UNIT-BUS.busrq-and-intr, grant-release-then-fetch, no-grant-after-entry, release-latency-config-5, cpu.CLRC.no-grant-after-noninterruptible). BUSRQ_PRIORITY, GRANT_LOSES_INTERRUPTIBLE, GRANT_AFTER_INT_ENTRY and RELEASE_LATENCY_IGNORED each fail their must-fail vector."
    requirement: CPU-02
    verification:
      - kind: integration
        ref: "sh tools/gate.sh (faults=67/67; build/gate/faults/BUSRQ_PRIORITY/cpu-vectors.txt and the three other grant faults)"
        status: pass
    human_judgment: false
  - id: D2
    description: "The one wait rule over FETCH, READ, WRITE and ADAR shifts later access stamps and the unit length; INTAK, IAB and DW take none; 35 is accepted and 36 faults BUS_WAIT_LIMIT (cpu.MVII.wait-on-fetch, cpu.MVI.wait-on-adar, cpu.MVI@.wait-8-plus-2w, cpu.MVO@.wait-on-write, cpu.UNIT-INT.no-wait-on-iab, cpu.UNIT-INT.no-wait-on-dw, cpu.MVII.wait-at-limit-35, cpu.MVII.wait-36-faults; cpu-api.wait-shifts-offsets). WAIT_ON_IAB, WAIT_ON_DW and WAIT_LIMIT_MISSING fail their must-fail vectors."
    requirement: CPU-05
    verification:
      - kind: integration
        ref: "sh tools/gate.sh and dash tools/gate.sh (gate: PASS vectors=412 policy=121 rows-pending=0 policies-pending=0 faults=67/67)"
        status: pass
    human_judgment: false
  - id: D3
    description: "HLT then the next instruction then HALTED, also with INTR held (INSTR, INSTR, HALTED); the stop beats BUSRQ and INTR; STPST edges stop, cancel and resume; HALTED ignores INTR/INTRM and grants BUSRQ. HLT_IMMEDIATE, HLT_INTERRUPTIBLE, HALT_PRIORITY, HALTED_TAKES_INTR and STPST_LEVEL fail their must-fail vectors."
    requirement: CPU-04
    verification:
      - kind: integration
        ref: "sh tools/gate.sh (cpu.HLT.*, cpu.UNIT-HALT.* vectors; build/gate/faults/HLT_INTERRUPTIBLE/cpu-vectors.txt)"
        status: pass
    human_judgment: false
  - id: D4
    description: "Seeded robustness: 8 seeds x 20000 steps under ASan/UBSan (dev) and plain (release); every step OK, within the bound, valid afterwards, deterministic on repeat; steps that start in FAULTED or UNRESET take 0 cycles and no access; HALTED and BUS_GRANT units take at least 1 cycle; corrupted states refused without mutation; every unit kind reached per seed."
    requirement: CPU-05
    verification:
      - kind: unit
        ref: "build/gate/dev/cpu-fuzz.txt (8 ok), build/gate/dev/cpu-api.txt (20 ok), same in release"
        status: pass
    human_judgment: false

duration: 15min
completed: 2026-11-10
status: complete
---

# Phase 2 Plan 15: Bus Grant, Wait States, HLT/STPST and HALTED Summary

**The stepper now serves the bus request before the interrupt at an interruptible boundary, with 1-cycle grant and hold units and a release unit of the profile latency that keeps the boundary open for the interrupt test. Waits shift later access stamps and the unit; a wait over the cap is a sticky BUS_WAIT_LIMIT fault. HLT and STPST share one stop latch that wins at the next interruptible boundary, so HLT always lets the next instruction run. HALTED ignores INTR and honours BUSRQ. A seeded fuzz suite checks the no-progress guard. All 19 policies now have vectors and faults, and nothing is pending. All 22 hand-derived vectors passed on their first run.**

## Performance

- **Duration:** about 15 min (09:27 to 09:42 UTC)
- **Tasks:** 3 of 3
- **Files modified:** 14 (2551 insertions, 35 deletions)

## Accomplishments

- **Grant (POL-BUS-01).** `ivs_unit_grant` makes a 1-cycle grant or hold unit with BUSAK 1, or, when BUSRQ is low with BUSAK 1, a release unit of `cfg->release_latency_cpu_cycles` with BUSAK 0 and `BUS_RELEASED`. A grant never touches `prev_interruptible`.
- **Boundary test.** `ivs_boundary_open` is the one boundary predicate (prev_interruptible, with the existing PREV_INTERRUPTIBLE_IGNORED site moved into it). `ivs_step_running` computes stop, grant and accept, then picks in the order stop, grant, entry, instruction.
- **Waits (POL-WAIT-01).** The plan 02-02 accounting already matched the one wait rule. The over-cap fault unit now ends at the address cycle plus `limit + 1`.
- **Stop (POL-HLT-01/02).** The STPST edge is processed first, toggling the latch in RUNNING and resuming from HALTED. `ivs_step_halted` handles the HALTED state.
- **Conventions.** A comment block in `cp1610.c` ("Step-start conventions") and `docs/cp1610/stepper-conventions.md` record the unit conventions.
- **Gate.** `vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 faults=67/67 tests-expected=1040 tests-executed=1040`.

## Unit conventions (fixed by this plan)

| Unit | Cycles | Accesses | State change |
|------|--------|----------|--------------|
| BUS_GRANT, grant or hold (BUSRQ high) | 1 | none | BUSAK 1 |
| BUS_GRANT, release (BUSRQ low, BUSAK 1) | config `release_latency_cpu_cycles` (default 2, C52) | none | BUSAK 0, event BUS_RELEASED |
| HALTED, entry at an interruptible boundary with the latch set | 1 | none | run state HALTED, halt_pending 0, event STOPPED |
| HALTED, idle | 1 | none | none |
| Step starting in UNRESET or FAULTED | 0 | none | none |
| FAULT for BUS_WAIT_LIMIT | access offset + 1 + limit + 1 | the accesses up to the faulting one | state as before the unit, FAULTED |

Step-start order: the STPST edge (toggle in RUNNING, resume in HALTED), then a held grant, then the boundary test with priority stop > BUSRQ > INTR > INTRM. A closed boundary tests nothing and fetches.

## Task Commits

1. **Task 1: Tracer - BUSRQ and INTR at the same boundary end to end** - `54e3671` (feat). Gate `faults=56/56`. Tracer feedback gate: `sh tools/gate.sh` re-ran green (39 s), and BUSRQ_PRIORITY failed exactly `cpu.UNIT-BUS.busrq-and-intr` before Task 2 started.
2. **Task 2: Remaining grant rules, waits and cap, HLT/STPST/HALTED, eleven faults, empty pending list** - `246c707` (feat). Gate `faults=67/67 rows-pending=0 policies-pending=0`.
3. **Task 3: cpu-fuzz suite, API cases, stepper conventions doc, bus-figure audit** - `6fae4e5` (test). Gate PASS under sh and dash.

**Plan metadata:** recorded in the docs(02-15) completion commit.

## Vector disagreements and resolutions

None against the core.
- All 22 vectors passed on their first run (2 in Task 1, 20 in Task 2).
- No `expect` value was edited after a run.
- No core change was made to match a vector.

Page readings for the plan 02-17 second reader to re-derive independently:
- **BUSRQ before INTR.** Fig 2-6 (pdf 22) puts BUS REQUEST before INTERRUPT REQUEST on the interruptible branch, and the WAIT (OUTPUT BUSAK, NACT) loop exits into the interrupt test. pdf 25: "the CPU will honor Bus Requests before it will honor Interrupt Requests".
- **No grant after a closed boundary.** In Fig 2-6 the UNINTERRUPTABLE INSTRUCTION branch and the INPUT DEVICE PC (IAB) box both return straight to the fetch. pdf 36 marks the grant at the "END OF CURRENT INTERRUPTABLE INSTRUCTION".
- **Release latency.** pdf 36 shows BUSAK rising tBU after the TS1 that samples BUSRQ high, and the next bus activity about two cycles later. That count is the C52 inference, so the vectors are inference basis.
- **Waits.** pdf 37 (BDRDY timing) shows the wait as NACT cycles between the BAR and the DTB. pdf 40 says BDRDY is sampled "every TS1 immediately following every micro state in which a BAR or ADAR bus control signal is issued". pdf 43 gives "8 + 2W = 8 + W + W = 8 + 0 + 2 = 10" for fast program memory and slow data memory, which `cpu.MVI@.wait-8-plus-2w` reproduces. pdf 12 says the wait "must be less than 40 microseconds" (cap 35, C53).
- **Stop.** pdf 14: STPST stops the CPU "only after the completion of an interruptable instruction", and "The next negative transition of STPST will cause the CPU to return to the run mode". pdf 56 (HLT): "HaLT after next instruction is executed. Resume on control start." C40, C54 and C56 are the defaults.
- **HALTED and lines.** The pages are silent (C45). The A-10 default is used.

## Bus-figure audit (pdf 27-37)

| pdf | Figure | figure-basis vectors | Note |
|-----|--------|----------------------|------|
| 27 | Initialization sequence | 4 (`cpu.UNIT-RESET.*`) | |
| 28 | Instruction fetch | 150 | |
| 29 | Fetch and data access, indirect | 40 (now including `cpu.MVII.wait-on-fetch`, `cpu.MVI@.wait-8-plus-2w`, `cpu.MVII.wait-at-limit-35`) | |
| 30 | Direct addressing | 17 (now including `cpu.MVI.wait-on-adar`) | |
| 31 | MVO write | 13 (now including `cpu.MVO@.wait-on-write`) | |
| 32 | Branch | 17 | The figure ends at the displacement BAR, so only the stamps up to it are figure basis; the cycle split is POL-BR-01 |
| 33 | TCI timing | 2 | |
| 34 | PCIT timing | 3 | |
| 35 | Interrupt sequence | 34 (now including `no-grant-after-entry`, `no-wait-on-iab`, `no-wait-on-dw`) | |
| 36 | Bus request/acknowledge | 1 (`cpu.UNIT-BUS.no-grant-after-entry`, whose stamps come from pdf 35) | The grant sequence has no access record to stamp. Its only number is the release latency, which is the C52 inference, so the grant and release vectors are inference basis by design. No vector is missing. |
| 37 | BDRDY timing | 5 (the wait vectors above) | New in this plan |

Every page from pdf 27 to 37 now has at least one figure-basis vector. No figure page that prints a complete sequence for a modelled unit lacks one, so no vector was added in Task 3. The gate's `bus-figure-pages` went from 31 to 35. That stat counts every `pdf N` in a figure vector's cite, not only the figure pages.

## Fault verdicts (latest gate run)

| Fault | Site | Vectors that fail |
|-------|------|-------------------|
| BUSRQ_PRIORITY | `ivs_step_running`: grant dropped when INTR is accepted | `UNIT-BUS.busrq-and-intr` |
| GRANT_LOSES_INTERRUPTIBLE | `ivs_unit_grant`: prev_interruptible 0 | `UNIT-BUS.busrq-and-intr`, `UNIT-HALT.halted-grants-busrq`, `CLRC.no-grant-after-noninterruptible`, `UNIT-BUS.no-grant-after-entry` |
| GRANT_AFTER_INT_ENTRY | `ivs_step_running`: grant ignores the closed boundary | `UNIT-BUS.no-grant-after-entry`, `CLRC.no-grant-after-noninterruptible` |
| RELEASE_LATENCY_IGNORED | `ivs_unit_grant`: release takes 1 cycle | `UNIT-BUS.busrq-and-intr`, `grant-release-then-fetch`, `release-latency-config-5`, `UNIT-HALT.halted-grants-busrq` |
| WAIT_ON_IAB | `ivs_kind_takes_wait`: IAB takes waits | `UNIT-INT.no-wait-on-iab` |
| WAIT_ON_DW | `ivs_kind_takes_wait`: DW takes waits | `UNIT-INT.no-wait-on-dw` |
| WAIT_LIMIT_MISSING | `ivs_access`: limit $FFFF | `MVII.wait-36-faults` |
| HLT_IMMEDIATE | HLT handler: enters HALTED at once | the four `cpu.HLT.` vectors that reach the stop or assert RUNNING, including plan 02-06's `sets-halt-pending` |
| HLT_INTERRUPTIBLE | `cp1610_decode.c`: HLT row interruptible 1 | the same four `cpu.HLT.` vectors |
| HALT_PRIORITY | `ivs_step_running`: stop dropped when a grant is due | `HLT.halt-beats-busrq-and-intr` |
| HALTED_TAKES_INTR | `ivs_step_halted`: INTR acceptance resumes | `UNIT-HALT.halted-ignores-intr` |
| STPST_LEVEL | `ivs_step_running`: an edge sets the latch, never clears it | `UNIT-HALT.stpst-second-edge-cancels`, `HLT.stpst-edge-cancels-hlt` |

Every must-pass vector passes. The planned must-pass ids were kept and none needed swapping. The plan 02-14 LINES_LATCHED caution did not apply: every RUNNING step evaluates the acceptance test, and the fault's verdict is unchanged (only `cpu.CLRC.level-not-latched` fails).

### Prefix extensions (exact ids only)

- **INT_ENTRY_ORDER** and **FIRST_ISR_INSTR_INTERRUPTIBLE** gain `cpu.UNIT-BUS.busrq-and-intr` (Task 1), and `cpu.UNIT-BUS.no-grant-after-entry`, `cpu.UNIT-INT.no-wait-on-iab` and `cpu.UNIT-INT.no-wait-on-dw` (Task 2). These are the new vectors that contain an INT_ENTRY.
- **PREV_INTERRUPTIBLE_IGNORED** gains `cpu.HLT.` and `cpu.CLRC.no-grant-after-noninterruptible`, as planned. It also gains:
  - `cpu.UNIT-BUS.no-grant-after-entry`, because the boundary after the entry opens and the CPU grants;
  - `cpu.UNIT-HALT.stpst-second-edge-cancels`, because the first edge then stops at once.
- **GRANT_AFTER_INT_ENTRY** gains `cpu.CLRC.no-grant-after-noninterruptible` (see the key decisions).
- **GRANT_LOSES_INTERRUPTIBLE** gains `cpu.CLRC.no-grant-after-noninterruptible` and `cpu.UNIT-BUS.no-grant-after-entry`. Both end with a grant and assert prev_interruptible 1.
- **MVO_INTERRUPTIBLE** gains `cpu.MVO@.wait-on-write`, which asserts the MVO family's prev_interruptible 0.
- **STPST_LEVEL** gains `cpu.HLT.stpst-edge-cancels-hlt`. Under the fault, its cancelling edge sets the latch instead.

## Decisions Made

See `key-decisions` in the frontmatter.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] The wait-cap fault unit length depended on the device reply**
- **Found during:** Task 2, while deriving the fuzz bound
- **Issue:** The plan 02-02 code ended the faulting unit after the full answered wait (offset + 1 + wait). The unit length was then unbounded by the config, which broke the plan's fuzz bound `14 + 4 * wait_limit + release_latency + 1`. It also contradicted "one more cycle faults `BUS_WAIT_LIMIT`".
- **Fix:** The fault unit now ends at offset + 1 + limit + 1. For the 36-cycle vector both rules give 37. No existing vector or test asserted the old length.
- **Files modified:** core/src/cpu/cp1610.c
- **Commit:** 246c707

**2. [Rule 2 - Missing guard] Release latency 0 refused**
- **Found during:** Task 2
- **Issue:** A config release latency of 0 would make a 0-cycle BUS_GRANT unit, breaking the D-12 "BUS_GRANT units take at least 1 cycle" guard.
- **Fix:** `ivs_config_valid` rejects it with IVS_E_BAD_ARG. The profile default is 2.
- **Files modified:** core/src/cpu/cp1610.c
- **Commit:** 246c707

**3. [Rule 1 - Fault design] Prefix extensions beyond the planned lines**
- **Found during:** Task 2 (the first gate run listed 7 `fault-wrong-reason` findings)
- **Issue:** Seven extra failures appeared. Each comes from a new vector that asserts exactly the behaviour its fault changes.
- **Fix:** Added the exact ids listed under "Prefix extensions" above. The plan's inventory lines are otherwise verbatim.
- **Commit:** 246c707

**4. [Rule 2 - Coverage] Three vectors beyond the planned list**
- **Found during:** Task 2
- **Issue:** Three gaps in the planned list:
  - The one wait rule names WRITE and READ, but the planned list had no write-wait or data-read-wait vector.
  - The page's own "8+2W" example had no vector.
  - "One latch serves HLT and STPST" (C54) had no HLT-side vector.
- **Fix:** Added three vectors, for 22 in the file (2 from Task 1, 17 planned in Task 2, these 3):
  - `cpu.MVO@.wait-on-write`
  - `cpu.MVI@.wait-8-plus-2w`
  - `cpu.HLT.stpst-edge-cancels-hlt`
- **Commit:** 246c707

**5. [Rule 3 - Blocking] `halt-beats-busrq-and-intr` uses step_lines**
- **Found during:** Task 2 design
- **Issue:** With BUSRQ held from step 1, GRANT_AFTER_INT_ENTRY would also catch this vector.
- **Fix:** The lines are low for HLT and NOP and high only at the boundary under test. That boundary is the one the vector is about.
- **Commit:** 246c707

---

**Total deviations:** 5 auto-fixed (2 Rule 1, 2 Rule 2, 1 Rule 3).

**Impact on plan:** No scope change. Every acceptance count holds:
- Task 1: busrq-and-intr 1, BUSRQ_PRIORITY 1.
- Task 2: pending data lines 0, POL-HLT-02 policy lines 2, intr-held init prev 0 matched 1.
- Task 3: cpu-fuzz 8 ok and cpu-api 20 ok in dev and release; `rand/srand/time/clock(` count 0 in cpu_fuzz.c; 5 `## ` headings in the doc.

## Measured cost (D-22 budget input)

- **Final `sh tools/gate.sh`:** 36 s wall, unpiped (output to a file).
  - Summary line: `gate: PASS steps=24/24 controls=182/182 cc=AppleClang c-presets=2 fault-runs=67 ... vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 ... faults=67/67 ... tests-expected=1040 tests-executed=1040 tests-skipped=0 tests-missing=0 ...`.
- **`dash tools/gate.sh`:** 37 s, `gate: PASS`.
- **Task gates:** 39 s for Task 1 and 37 s for Task 2. For comparison, plan 02-14 measured 41 s.
- **cpu-fuzz alone:** about 0.1 s for 8 x 20000 steps, each stepped twice.

## Issues Encountered

None.

## Known Stubs

None.

## User Setup Required

None.

## Next Phase Readiness

- **Plan 02-16/02-17 (blind second reading):** re-derive the readings listed above.
  - The inferred conventions (release latency, stop timing, HALTED line handling, the fault unit length) are policy and inference basis.
  - The reader protocol must list `docs/cp1610/stepper-conventions.md` as forbidden (A-12).
- **Plan 02-18 (D-22 budget):** the gate measured 36 to 39 s across this plan.
- **Unasserted or inferred:**
  - a STPST edge while a grant is held (the latch toggles; the stop applies after the release);
  - random states with BUSAK 1 at a closed boundary (the fuzz suite exercises them; no vector asserts them).

---
*Phase: 02-cp-1610-cpu-core*
*Completed: 2026-11-10*

## Self-Check: PASSED

- Key files exist: `data/cp1610/vectors/busrq-halt.json`, `tests/cpu/cpu_fuzz.c`, `data/expected/tests/cpu-fuzz.ids`, `docs/cp1610/stepper-conventions.md` and this SUMMARY.
- Commits `54e3671`, `246c707` and `6fae4e5` are ancestors of HEAD. The ledger measures `git rev-list --count a9d4554..6fae4e5` = 3.
- Every task's acceptance commands were re-run and printed the expected values.
- Final gate: `sh tools/gate.sh` gate: PASS (36 s) and `dash tools/gate.sh` gate: PASS (37 s).
