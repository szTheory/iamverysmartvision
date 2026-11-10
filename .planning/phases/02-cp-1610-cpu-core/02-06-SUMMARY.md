---
phase: 02-cp-1610-cpu-core
plan: 06
subsystem: cpu-core
tags: [c17, cp1610, control-group, vectors, named-faults, strobes]

requires:
  - phase: 02-cp-1610-cpu-core
    plan: 05
    provides: derived vector coverage (tag proofs, pending list, flag/bus rules) and the four-column fault inventory
provides:
  - decode rows and handlers HLT, SDBD, EIS, DIS, TCI, GSWD, NOP, SIN, RSWD (all 11 control rows implemented)
  - STROBE_TCI record at unit end + 1 (pdf 33) and STROBE_PCIT record at cycle 4 inside SIN (pdf 34)
  - 20 new page-cited vectors in data/cp1610/vectors/control.json (27 in total)
  - named faults EIS_NO_INTFF, GSWD_LAYOUT, TCI_STROBE_STAMP, HLT_NO_LATCH (faults=6/6)
  - coverage pending list down to 63 rows
affects: [02-13, 02-14, 02-15, 02-16, 02-17]

actuals:
  tokens: 11764
  tasks: 2
  commits: 2

tech-stack:
  added: []
  patterns:
    - "A strobe is an access record routed through the bus port with addr = data = 0; it takes no wait and is shifted by earlier waits like any later record"
    - "Handlers that need the unit (strobes) or the instruction word (register fields) get them as ivs_exec arguments; the decode row stays mask/value/words/cycles/interruptible/handler"
    - "Interruptible rows' vectors set init.prev_interruptible 0 so expect 1 proves the row flag; non-interruptible rows rely on the schema default 1 going to 0"

key-files:
  created: []
  modified:
    - core/src/cpu/cp1610.c
    - core/src/cpu/cp1610.h
    - core/src/cpu/cp1610_decode.c
    - data/cp1610/vectors/control.json
    - data/expected/cp1610-coverage-pending.ids
    - data/expected/cp1610-vectors.ids
    - data/expected/tests/cpu-vectors.ids
    - data/expected/faults/cpu.ids

key-decisions:
  - "HLT reports the event HALT_LATCHED in the unit that sets halt_pending; plan 02-15 adds the STOPPED event when the latch is evaluated"
  - "STROBE_TCI is stamped at row base cycles + 1 (+ waits) = elapsed_cpu_cycles + 1 (pdf 33, C39); STROBE_PCIT at nominal cycle 4 of the 6-cycle SIN (pdf 34: the output pulse starts in cycle 4 and ends in cycle 5)"
  - "Strobe records carry addr = data = $0000 and pass through the bus port so the machine layer sees the pulse"
  - "RSWD with SSS = 7 reads the already incremented PC (inference; no vector asserts it; for the plan 02-17 discrepancy pass)"
  - "No flag exemptions were needed: RSWD can set and clear each of S, Z, OV, C, and every other new row is status-preserving"
  - "requirements-completed stays empty as in plans 02-02..02-05: CPU-01, CPU-02 and CPU-05 are advanced here and completed by later plans of this phase"

patterns-established:
  - "Status-preserving rows get one vector with all four flags 1 tagged flag-preserved:S/Z/OV/C; a second vector with flags 0 exercises the other value of the state bit the row touches"
  - "Bit-mapping rows (RSWD, GSWD) get vectors whose flag patterns tell every pair of flags apart, so a swapped bit cannot pass"

requirements-completed: []
requirements-advanced: [CPU-01, CPU-02, CPU-05]

coverage:
  - id: D1
    description: "EIS sets and DIS clears INTFF; neither changes a status bit; both non-interruptible (pdf 56, POL-INT-03)"
    requirement: CPU-02
    verification:
      - kind: unit
        ref: "build/dev/tests/ivs_tests_cpu_vectors: cpu.EIS.sets-intff, cpu.EIS.intff-already-set, cpu.DIS.clears-intff, cpu.DIS.intff-already-clear"
        status: pass
      - kind: other
        ref: "sh tools/gate.sh: fault EIS_NO_INTFF fails only cpu.EIS.* (field intff), cpu.DIS.clears-intff passes"
        status: pass
    human_judgment: false
  - id: D2
    description: "HLT sets halt_pending (HALT_LATCHED), SDBD sets dbd, NOP and SIN identical for N=0 and N=1, GSWD writes the pdf 56 layout into R0-R3, RSWD loads S/Z/OV/C from bits 7..4"
    requirement: CPU-01
    verification:
      - kind: unit
        ref: "build/dev/tests/ivs_tests_cpu_vectors: cpu.HLT.*, cpu.SDBD.*, cpu.NOP.n0/n1, cpu.SIN.n0/n1, cpu.GSWD.* (4), cpu.RSWD.* (4)"
        status: pass
      - kind: other
        ref: "faults GSWD_LAYOUT (r0 F0F0 vs 0F0F) and HLT_NO_LATCH (halt_pending) fail for the intended reason; faults=6/6"
        status: pass
    human_judgment: false
  - id: D3
    description: "TCI emits STROBE_TCI at elapsed_cpu_cycles + 1 and event TCI; SIN emits STROBE_PCIT at cycle 4 and event PCIT; both in figure-basis bus lists"
    requirement: CPU-05
    verification:
      - kind: unit
        ref: "build/dev/tests/ivs_tests_cpu_vectors: cpu.TCI.strobe-figure, cpu.TCI.intff-clear-unchanged, cpu.SIN.n0, cpu.SIN.n1"
        status: pass
      - kind: other
        ref: "fault TCI_STROBE_STAMP: actual [4, STROBE_TCI] vs expected [5, STROBE_TCI], only cpu.TCI.* fail, cpu.SIN.n0 passes"
        status: pass
    human_judgment: false
  - id: D4
    description: "All 11 control rows meet the plan 02-05 derived coverage and are off the pending list; gate green under sh and dash"
    requirement: CPU-01
    verification:
      - kind: other
        ref: "sh tools/gate.sh: gate: PASS steps=23/23 controls=169/169 ... vectors=27 rows-pending=63 ... faults=6/6 ... tests-expected=94 tests-executed=94; dash tools/gate.sh: gate: PASS steps=23/23 controls=169/169"
        status: pass
    human_judgment: false

duration: 8min
completed: 2026-11-10
status: complete
plan_head_before: 537d2c87d5ca7b0e221c0bff08b25af66ff711b7
plan_head_after: 637f666d5d357ac238a81d8fa45cae5e2a9fd271
commits: 2
---

# Phase 2 Plan 06: control group Summary

**All 11 CP-1610 control rows now run on the stepper. HLT, SDBD, EIS, DIS, TCI, GSWD, NOP, SIN and RSWD joined CLRC and SETC. Each row has page-cited vectors that meet the plan 02-05 derived coverage. TCI and SIN emit their strobe records at the stamps read from the pdf 33 and pdf 34 timing figures. Four new named faults each fail their must-fail vector for the intended reason.**

## Performance

- **Duration:** about 8 min of execution, from 2026-11-10T06:36:43Z (after the plan and page reading) to 06:45Z. The SUMMARY and state updates came after that.
- **Tasks:** 2 (1 tracer, 1 auto)
- **Files:** 8 changed in the two task commits (1396 insertions, 14 deletions). Most of the insertions are vector JSON.

## Accomplishments

- **Decode rows.** All values were transcribed from the page images:

  | Row | Word | Cycles | Interruptible | Basis |
  |---|---|---|---|---|
  | HLT | $000 | 4 | no | C56 default |
  | SDBD | $001 | 4 | no | C56 default, pdf 57 |
  | EIS | $002 | 4 | no | printed |
  | DIS | $003 | 4 | no | printed |
  | TCI | $005 | 4 | no | printed |
  | GSWD | $030-$033 (`0000 110 0DD`) | 6 | yes | |
  | NOP | $034-$035 (`10N`) | 6 | yes | |
  | SIN | $036-$037 (`11N`) | 6 | yes | |
  | RSWD | $038-$03F (`0000 111 SSS`) | 6 | yes | |

  $004 (J) still decodes to the temporary UNIMPLEMENTED fault. Each handler sets `prev_interruptible` from its row flag through the existing `ivs_unit_instr` path.
- **Effects:**
  - EIS sets INTFF and DIS clears it (POL-INT-03).
  - HLT sets `halt_pending` and reports HALT_LATCHED. The stop is evaluated in plan 02-15.
  - SDBD sets `dbd`. Consumption is plan 02-13.
  - TCI emits STROBE_TCI at `elapsed_cpu_cycles + 1` plus event TCI.
  - SIN emits STROBE_PCIT at cycle 4 plus event PCIT.
  - GSWD writes `status | status << 8` (the pdf 56 layout) into R(DD).
  - RSWD loads `R(SSS) & $F0` into the status byte.
  - NOP does nothing.
- **Vectors.** There are 20 new vectors, all kind source, tier source-expectation. Each asserts cycles, units and events, and each has a figure-basis bus list:
  - EIS: `sets-intff`, `intff-already-set`
  - DIS: `clears-intff`, `intff-already-clear`
  - HLT: `sets-halt-pending`
  - SDBD: `sets-dbd`
  - TCI: `strobe-figure`, `intff-clear-unchanged`
  - GSWD: `r0-all-flags` ($F0F0), `r3-s-z` ($C0C0), `r1-ov-c` ($3030), `r2-no-flags` ($0000 over an init of $FFFF)
  - NOP: `n0`, `n1`
  - SIN: `n0`, `n1`
  - RSWD: `sets-all`, `clears-all` (R5 = $FF0F), `s-c-only` ($0090), `s-ov-only` ($00A0)

  The two RSWD mixed patterns tell every pair of flags apart.
- **Coverage.** All 11 control rows are off `cp1610-coverage-pending.ids`, so `rows-pending=63`. No flag exemption was added.
- **Faults.** Four inventory lines were added exactly as planned. No must-pass swap was needed:
  - EIS_NO_INTFF fails `cpu.EIS.sets-intff` and `cpu.EIS.intff-already-set` (field intff).
  - GSWD_LAYOUT fails `cpu.GSWD.r0-all-flags`, `r1-ov-c` and `r3-s-z` (for example, r0 expected $F0F0, got $0F0F).
  - TCI_STROBE_STAMP fails both TCI vectors (the strobe lands at cycle 4 instead of 5).
  - HLT_NO_LATCH fails `cpu.HLT.sets-halt-pending` (field halt_pending).

  Every must-pass vector passes.

## Task Commits

1. Task 1 (tracer): `6ba7b95` - `feat(02-06): EIS and DIS rows with INTFF effects, vectors and fault EIS_NO_INTFF`
2. Task 2: `637f666` - `feat(02-06): HLT, SDBD, TCI, GSWD, NOP, SIN and RSWD with vectors and three faults`

**Tracer feedback gate.** The tracer `<verify>` ran end to end before Task 2 started:
- `ctest --preset dev`: 100% of 3 passed.
- `sh tools/gate.sh`: `gate: PASS steps=23/23 controls=169/169 ... rows-pending=70 ... faults=3/3` in 28.2 s.

## Verification results

- **Final gate.** Committed tree, run unpiped with `sh`:
  - `gate: PASS steps=23/23 controls=169/169 cc=AppleClang c-presets=2 fault-runs=6 ctest-runs=2 ... vectors=27 source=24 policy=3 rows-pending=63 policies-pending=18 bus-figure-pages=11 ... faults=6/6 ... tests-expected=94 tests-executed=94 tests-skipped=0 tests-missing=0 ...`
  - Wall time 32.6 s (`9.56s user 18.29s system 85% cpu 32.589 total`). Two redirected runs of the same tree took 26.0 s and 27.0 s.
- **dash.** `dash tools/gate.sh` gave `gate: PASS steps=23/23 controls=169/169 ...` in 33.4 s.
- **Task 1 acceptance:**
  - The pending-list count of `^(EIS|DIS)$` is 0.
  - The count of `^EIS_NO_INTFF ` in `faults/cpu.ids` is 1.
  - Both `not ok` lines in `build/gate/faults/EIS_NO_INTFF/cpu-vectors.txt` contain `cpu.EIS.`.
- **Task 2 acceptance:**
  - The pending count over the 11 control rows is 0.
  - The fault-id count is 4.
  - The count of `"bus_basis": "figure"` in control.json is 26.
- **Page against code.** Every vector passed on its first run. No disagreement occurred, so no `expect` was revisited.
- **Fault diagnostics.** The intended reasons were confirmed by running each `ivs_fault_<ID>` binary with `--only` (field diffs and bus-sequence diffs as listed above).

## Decisions Made

See `key-decisions` in the frontmatter. Page readings behind the stamps:
- **pdf 33.** The TCI figure shows BAR (0), NACT (1), DTB (2), NACT (3) and then the next instruction's BAR (4). The TCI pulse rises t_TO after the TS1 of the following NACT, cycle 5.
- **pdf 34.** A zoomed render of the PCIT figure (scratch only) shows the output pulse starting in microcycle 4 and ending in microcycle 5. The next BAR is at cycle 6.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] `core/src/cpu/cp1610.h` modified although not in `files_modified`**
- **Found during:** Task 1
- **Issue:** The row handler ids (`IVS_CP1610_OP_*`) live in `cp1610.h`, next to CLRC and SETC. The new decode rows cannot name a handler without them.
- **Fix:** Added `IVS_CP1610_OP_EIS` through `IVS_CP1610_OP_RSWD` (ids 3-11). Nothing else in the header changed.
- **Files modified:** core/src/cpu/cp1610.h
- **Commits:** 6ba7b95, 637f666

**2. [Scope note] HLT reports HALT_LATCHED**
- **Issue:** The plan says only that HLT sets `halt_pending`. The header already defines `IVS_CP1610_EV_HALT_LATCHED`, and no unit produced it.
- **Resolution:** HLT reports the event in the unit that sets the latch. `cpu.HLT.sets-halt-pending` asserts `events: ["HALT_LATCHED"]`. This is reporting only: no machine state beyond `halt_pending` changes. Plan 02-15 keeps or revises it together with that vector.
- **Commit:** 637f666

**3. [Scope note] Extra vectors beyond the named list**
- **Issue:** The plan names about 16 vectors for Task 2 and allows "plus one more each" in Task 1.
- **Resolution:** 20 vectors were added in total, which matches the plan's "about 20". The extras are:
  - `cpu.EIS.intff-already-set` and `cpu.DIS.intff-already-clear` (the other INTFF value with flags 0);
  - `cpu.TCI.intff-clear-unchanged`;
  - `cpu.GSWD.r1-ov-c` and `cpu.GSWD.r2-no-flags` (the remaining layout bits and zero-filled bits);
  - `cpu.RSWD.s-c-only` and `cpu.RSWD.s-ov-only` (flag pairs told apart).

**Total deviations:** 1 auto-fixed (Rule 3), plus 2 scope notes. **Impact:** none on scope. Every stat matches the plan: `faults=6/6`, `rows-pending=63`, `controls=169/169`.

## Authentication Gates

None.

## Known Stubs

None. The temporary UNIMPLEMENTED fault for rows that have no handler yet is pre-existing (plan 02-02). Plan 02-13 removes it.

## Threat Flags

None. The strobe records and the stamps they carry are inside T-02-49, which is mitigated by figure-basis bus lists and TCI_STROBE_STAMP. The derivations cite pdf 13, 14, 28, 33, 34, 45, 51, 56 and 57 plus C39, C40 and C56 (T-02-48). The residual shared-misreading risk stays logged as C43.

## Issues Encountered

- **Gate time.** The unpiped run took 32.6 s, and the redirected runs took 26-27 s. Both are above the D-22 local budget of 25 s, as in plans 02-04 and 02-05. Plan 02-16 owns the levers.
- **`bus-figure-pages` rose from 4 to 11.** The stat counts every `pdf N` in the cite of a figure-basis vector, so table pages count too: pdf 13, 14, 45, 51, 56 and 57 alongside the figure pages 27, 28, 33 and 34. It is a stat only, and no plan asserts a value. Plan 02-16 may narrow it to figure pages if it wants the stat to mean that.

## Next Phase Readiness

- **Plan 02-13 (SDBD).** SDBD is a 4-cycle non-interruptible unit that sets `dbd`. Nothing consumes `dbd` yet. A CLRC or other row after SDBD currently ignores it, and plan 02-13 replaces that with consumption or refusal. `cpu.SDBD.sets-dbd` is a single-step vector, so it is unaffected by the two-step SDBD vectors there.
- **Plan 02-14 (interrupts).**
  - POL-INT-03 is still pending. The EIS and DIS vectors here do not tag it.
  - `cpu.EIS.defers-intr`, `cpu.DIS.defers-intr` and `cpu.TCI.defers-intr`, `cpu.SIN.interruptible` and `cpu.NOP.interruptible` can rely on the row flags set here.
- **Plan 02-15 (HLT).** `halt_pending` is set and HALT_LATCHED is reported. The fault prefix `cpu.HLT.` in HLT_IMMEDIATE and HLT_INTERRUPTIBLE also covers `cpu.HLT.sets-halt-pending`, which stays a single-step vector with units `["INSTR"]`.
- **Plan 02-17 (discrepancies).** Candidate entries:
  - RSWD with SSS = 7 reads the incremented PC (an inference).
  - The PCIT stamp at cycle 4 is a figure reading. C39 already records the strobe-stamp default.

## Self-Check: PASSED

- All 8 modified files exist on disk.
- Both commits are ancestors of HEAD: `6ba7b95` and `637f666`.
- The ledger measures `git rev-list --count 537d2c8..HEAD` = 2.
- The acceptance criteria of both tasks and the plan-level verification (sh and dash, `faults=6/6`, `rows-pending=63`, `controls=169/169`) were re-run after the last code commit, and all pass.
