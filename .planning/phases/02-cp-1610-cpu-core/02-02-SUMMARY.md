---
phase: 02-cp-1610-cpu-core
plan: 02
subsystem: cpu-core
tags: [c17, cmake, cp1610, harness, tap, test-inventory, gate]

requires:
  - phase: 02-cp-1610-cpu-core
    plan: 01
    provides: gate pass-line rule, build/gate NONCE contract, SPDX/ASCII rules for C and CMake sources, profile fields for the CPU config
provides:
  - CMake skeleton with dev and release presets, ivs_warnings and ivs_san interface targets
  - internal CPU API core/src/cpu/cp1610.h (fixed shape) and a unit stepper with reset entry, NOT_RESET, CLRC, SETC, wait accounting and sticky faults
  - cmake -P vector generator (JSON to build-tree C table, embedded source sha256)
  - in-repo TAP harness, flat test bus, suite cpu-vectors with 7 vectors
  - gate steps c-build, c-tests, tests-inventory; 12 tests-inventory controls
affects: [02-03, 02-04, 02-05, 02-08, 02-12, 02-13, 02-14]

actuals:
  tokens: 31090
  tasks: 3
  commits: 3

tech-stack:
  added: []
  patterns:
    - "Flags per target through INTERFACE libraries (ivs_warnings, ivs_san); no global CMAKE_C_FLAGS edits"
    - "Vectors flow JSON -> cmake -P generator -> build-tree ivs_vectors.inc -> harness; the generator fails the build on any unknown key, bad hex, bad name or program binary that disagrees with its hex"
    - "Result files are judged by a pure checker (tests-inventory) that reads only build/gate and data/expected, so text-overlay controls drive it"

key-files:
  created:
    - CMakeLists.txt
    - CMakePresets.json
    - cmake/IvsWarnings.cmake
    - cmake/IvsVectorTable.cmake
    - core/CMakeLists.txt
    - core/src/common/ivs_prelude.h
    - core/src/common/ivs_status.h
    - core/src/cpu/cp1610.h
    - core/src/cpu/cp1610.c
    - core/src/cpu/cp1610_decode.c
    - tests/CMakeLists.txt
    - tests/harness/ivs_test.h
    - tests/harness/ivs_test.c
    - tests/harness/ivs_testbus.h
    - tests/harness/ivs_testbus.c
    - tests/harness/ivs_vec.h
    - tests/cpu/cpu_vectors.c
    - data/cp1610/vectors/control.json
    - data/expected/cp1610-vector-files.ids
    - data/expected/cp1610-vectors.ids
    - data/expected/cp1610-skips.ids
    - data/expected/tests/cpu-vectors.ids
    - data/expected/gate-steps/cpu.ids
    - data/expected/controls/tests-inventory.ids
    - tools/check/steps/c-build.sh
    - tools/check/steps/c-tests.sh
    - tools/check/steps/tests-inventory.sh
    - tools/check/controls/tests-inventory-*/ (12 controls)
    - tools/check/controls/_good/tree/build/gate/ (NONCE, dev and release toy.txt and ctest-list.txt)
    - tools/check/controls/_good/tree/data/cp1610/vectors/toy.json
    - tools/check/controls/_good/tree/data/expected/cp1610-vector-files.ids
    - tools/check/controls/_good/tree/data/expected/tests/toy.ids
  modified:
    - tools/check/controls/sources-no-files/MAKE

key-decisions:
  - "The access log carries the transferred value (read reply or written value) and the applied wait; the bus port sees the request with wait 0 and, for reads, data 0. Order, kind, address and offset are identical in both"
  - "A wait above wait_limit_cpu_cycles abandons the unit: architectural state stays as before it, run_state FAULTED, fault BUS_WAIT_LIMIT, elapsed = faulting access offset + 1 + wait (inference; pdf 12 bounds the wait but defines nothing past it)"
  - "Step refusals also cover lines fields outside 0/1 and a config with interrupt_accept_clears_intff outside 0/1 or a nonzero reserved byte (IVS_E_BAD_ARG, state untouched)"
  - "Vector units and events are asserted only when listed in expect; every state field and every memory word (including which words are initialised) is always asserted"
  - "Unimplemented decles fault after a counted 4-cycle fetch (pdf 28 fetch figure: BAR 0, NACT, DTB 2, NACT), PC left at the offending word"

patterns-established:
  - "A vector run compares the full 64K image and its init bitmap against init + program + expect.mem, so a stray write anywhere fails the vector"
  - "TAP diagnostics go to stdout only; the result file holds header, source lines, plan, results and trailer, nothing else"

requirements-advanced: [CPU-05, CPU-04, CPU-01, RUN-03, PKG-02]
requirements-completed: []

coverage:
  - id: D1
    description: "Gate configures, builds and tests the dev (ASan+UBSan, Werror) and release (-O2, Werror) presets through c-build and c-tests; the developer loop runs the same suites"
    requirement: PKG-02
    verification:
      - kind: other
        ref: "sh tools/gate.sh (cc=AppleClang c-presets=2 ctest-runs=2); cmake --preset dev && cmake --build --preset dev && ctest --preset dev (100% tests passed)"
        status: pass
    human_judgment: false
  - id: D2
    description: "A JSON vector flows through the generator, harness, stepper and TAP into the inventory step; reset entry, not-reset, CLRC and SETC vectors pass in both presets"
    requirement: CPU-01
    verification:
      - kind: other
        ref: "build/gate/dev/cpu-vectors.txt and build/gate/release/cpu-vectors.txt: 7 ok, 0 not ok"
        status: pass
    human_judgment: false
  - id: D3
    description: "Reset entry: 5-cycle unit, one IAB at offset 1 loading all 16 bits of R7; providers $0000 and $FFFF; zeroed state gives NOT_RESET"
    requirement: CPU-04
    verification:
      - kind: other
        ref: "cpu.UNIT-RESET.provider-0000, cpu.UNIT-RESET.provider-ffff, cpu.UNIT-NOTRESET.zeroed-state"
        status: pass
    human_judgment: false
  - id: D4
    description: "Stepper reports ordered, cycle-stamped accesses; waits shift later offsets and only FETCH/READ/WRITE/ADAR take them"
    requirement: CPU-05
    verification:
      - kind: other
        ref: "bus lists in control.json (figure basis); throwaway probes listed under Verification"
        status: pass
    human_judgment: false
  - id: D5
    description: "tests-inventory fails with a distinct token for each of 12 result defects, each shown by a control, and reports expected/executed/skipped/missing counts"
    requirement: RUN-03
    verification:
      - kind: other
        ref: "sh tools/check/run-controls.sh (controls=128/128, 12 tests-inventory controls ok, _good/tests-inventory ok)"
        status: pass
    human_judgment: false

duration: 14min
completed: 2026-11-10
status: complete
plan_head_before: 42723a485cc8028176cf0067c86c04e2fe9c6d8b
plan_head_after: 25ae72367e976beb389e62ce6f5851a2f8b2f27a
commits: 3
---

# Phase 2 Plan 02: C walking skeleton, first stepper slice and test inventory Summary

**A C17 + CMake core now builds in two presets, steps reset entry, not-reset, CLRC and SETC from page-derived JSON vectors through a cmake -P generator and an in-repo TAP harness, and the gate judges every result file with an exact inventory checker backed by 12 controls.**

## Performance

- **Duration:** about 14 min measured from the first file write (plan and page reading before that)
- **Completed:** 2026-11-10
- **Tasks:** 3 (1 tracer, 2 auto)
- **Files:** 72 changed in the three task commits (71 created, 1 modified)

## Accomplishments

- Build (D-19): `CMakeLists.txt` (3.25 floor, C17, extensions off, options `IVS_WERROR`, `IVS_SAN`, `IVS_FAULT_CONTROLS`), `CMakePresets.json` v6 (`dev`: Debug `-O1 -g`, ASan+UBSan, Werror, fault controls; `release`: `-O2 -DNDEBUG`, Werror; test presets with `noTestsAction` error), `cmake/IvsWarnings.cmake` (`ivs_warnings`, `ivs_san`), `core/CMakeLists.txt` (explicit sources, one OBJECT library each, STATIC `ivs_core`, IPO off, writes `build/<preset>/ivs-core-sources.txt`).
- Core (D-01..D-07): `cp1610.h` holds the full interface contract with `_Static_assert` on 32 and 12 bytes. `cp1610.c`: `state_size`, `state_validate` (bad run_state, fault_reason, nonzero reserved bytes, status bits 0-3, non-0/1 booleans), `config_default` (0, 2, 35), `reset` (POL-RST-01), `step` (refusals that never write state; NOT_RESET; RESET_ENTRY with IAB at offset 1 and 5 cycles; FETCH at offset 0; CLRC and SETC; sticky UNIMPLEMENTED and BUS_WAIT_LIMIT faults; FAULTED reports a 0-cycle FAULT unit). The one wait rule is quoted above a single predicate `ivs_kind_takes_wait`. `cp1610_decode.c` holds the row table with CLRC ($006) and SETC ($007), read from pdf 56.
- Generator: `cmake/IvsVectorTable.cmake` supports every key of the plan's schema (steps, step_lines, config, ebci, waits, iab, reset, program_at, units, events, cycles not asserted when absent), merges init with the expect delta, sorts by id, keeps duplicates for the harness to reject, marks skips from `cp1610-skips.ids`, embeds `file(SHA256)` of each source.
- Harness: `ivs_test.{h,c}` (TAP-like output, `--only`, `--list`, duplicate ids rejected before anything runs, exit 77 only when every selected case is skipped), `ivs_testbus.{h,c}` (64K image with init bitmap, uninit-read detection, per-address waits for every kind, IAB value, EBCI set, call log), `cpu_vectors.c` (full state, full memory image, cycles, units, events and bus-sequence comparison; failure output shows id, cite, derivation, field diff and bus diff).
- Vectors: `data/cp1610/vectors/control.json` with 7 vectors: `cpu.UNIT-RESET.provider-0000`, `cpu.UNIT-RESET.provider-ffff`, `cpu.UNIT-NOTRESET.zeroed-state` (policy POL-RST-01), `cpu.CLRC.clears-c`, `cpu.CLRC.c-already-clear-flags-preserved`, `cpu.SETC.sets-c`, `cpu.SETC.flags-preserved` (source, source-expectation, each with a figure-basis `bus` list).
- Gate: steps `c-build`, `c-tests`, `tests-inventory` in `data/expected/gate-steps/cpu.ids`; 12 `tests-inventory-*` controls plus good-tree fixtures.

## Task Commits

1. Task 1 (tracer): 5b4fc72 - `feat(02-02): C skeleton, CP-1610 API, reset-entry stepper, vector generator and harness`
2. Task 2: fbaaf10 - `feat(02-02): CLRC/SETC rows, wait accounting and six control vectors`
3. Task 3: 25ae723 - `feat(02-02): tests-inventory step with 12 controls and good-tree fixtures`

Tracer feedback gate: after Task 1 the tracer `<verify>` ran end to end (`cmake --preset dev && cmake --build --preset dev && ctest --preset dev` at 100 percent, then `sh tools/gate.sh` with `gate: PASS steps=19/19 ... c-presets=2 ctest-runs=2`) before the expansion tasks.

## A-08 split

A-08 names one walking-skeleton plan holding build, harness, generator, minimal stepper, first vector group, inventory step and the fault path with one fault. That single plan is deliberately spread over plans 02-02, 02-03 and 02-04, each ending with a green gate: this plan (build, harness, generator, minimal stepper, first vector group, inventory step with its controls), plan 02-03 (the `cpu-api` and `harness` suites) and plan 02-04 (the fault path with its first real fault, and the vector data checker). A-08's intent is unchanged: no layer is retrofitted, and every later plan lands its own faults.

## Uncontrolled-token exceptions

- `c-build-failed`: environment or compile failure; proven by the real gate whenever the build breaks; an overlay control would need a broken real toolchain or build.
- `c-build-unlisted-source`: compile-inventory failure proven by the real gate when a core source is added without being listed; an overlay control would need a real CMake build of a modified tree, at a cost the control runner does not carry.
- `c-tests-failed`: proven by the real gate whenever a CTest case fails (every fault run in plan 02-04 onward exercises the failing path); an overlay control would need a real failing build and test run.

## Verification results

- `sh tools/gate.sh` (unpiped, final run): `gate: PASS steps=20/20 controls=128/128 cc=AppleClang c-presets=2 ctest-runs=2 ... tests-expected=14 tests-executed=14 tests-skipped=0 tests-missing=0 ...`, wall time 22.4 s (`7.59s user 13.40s system 93% cpu 22.355 total`).
- `dash tools/gate.sh`: `gate: PASS steps=20/20 controls=128/128 ...`. Each new step was also run once directly under `/bin/dash` (pass lines as above).
- `sh tools/check/run-controls.sh`: `controls=128/128`; 12 lines `control ok   tests-inventory-*` and one `control ok   _good/tests-inventory`.
- Task 1 acceptance: `grep -c '^ok '` on `build/gate/dev/cpu-vectors.txt` was 1 after the Task 1 gate; static-assert grep 1; `git ls-files --error-unmatch -- '*ivs_vectors.inc'` reports "did not match"; banned-include grep 0 for both core files; `SKIP_RETURN_CODE 77` grep 2.
- Task 2 acceptance: 7 `ok` and 0 `not ok` in `build/gate/dev/cpu-vectors.txt` (and 7 in release); `build/dev/tests/ivs_tests_cpu_vectors --list | grep -c '^cpu\.'` = 7; `--only cpu.CLRC. | grep -c '^ok '` = 2. The executable path is `build/dev/tests/ivs_tests_cpu_vectors` as planned.
- Task 3 acceptance: `grep -c -E 'ctest --|cmake --build' tools/check/steps/tests-inventory.sh` = 0; the gate tail matches `tests-expected=[0-9]+ tests-executed=[0-9]+ tests-skipped=0 tests-missing=0`.
- Idempotency edge: two runs of the suite differ only in the `nonce=` header line (`diff` shows line 1 only). Ordering edge: ids are emitted sorted (`--list` output is in C-locale order).
- Throwaway probes (scratch edits to `control.json` and `cp1610-skips.ids`, restored before commit, nothing committed): a changed `expect.cycles` fails with `cycles: expected 6, got 5`; a 3-cycle wait on the fetch address fails a 4-cycle expectation and passes a 7-cycle one; a 9-cycle wait on the IAB address of a reset vector has no effect; a 36-cycle wait gives FAULT `BUS_WAIT_LIMIT` with 37 cycles and state unchanged; a program placed away from R7 fails with `uninit-read ... first at $1000`; a skip line gives `ok N id # SKIP C1`; a duplicated id makes the suite print `duplicate id ...: nothing run` and exit 1.

## Decisions Made

- The log carries the transferred value and the applied wait, while the bus port sees the request (wait 0, read data 0). Plan 02-03's `cpu-api.bus-order-matches-log` should compare order, kind, address and offset between the port's call log and the step log, and data only for writes.
- BUS_WAIT_LIMIT abandons the unit (state as before, fault sticky, elapsed = faulting offset + 1 + wait). Inference, recorded in a code comment; no vector asserts it yet.
- `ivs_cp1610_step` refuses invalid lines (any field not 0/1) and an invalid config (`interrupt_accept_clears_intff` not 0/1 or nonzero `reserved`) with `IVS_E_BAD_ARG`.
- Vector `units` and `events` are asserted only when listed; state and memory are always asserted in full.
- The UNIMPLEMENTED fault counts the 4-cycle fetch of pdf 28 (BAR at 0, DTB at 2).
- Row handler ids live in `cp1610.h` as `IVS_CP1610_OP_*` (internal, not part of any ABI).

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] sources-no-files control broke when the good tree gained a data JSON**
- **Found during:** Task 3 (first control run: `control-token-missing: sources-no-files`, controls=127/128)
- **Issue:** The control deletes every `data/**/*.json` in the good tree so the `sources` step finds no data file. The new fixture `data/cp1610/vectors/toy.json` survived, so the token never fired.
- **Fix:** Added `delete data/cp1610/vectors/toy.json` to the control's MAKE.
- **Files modified:** tools/check/controls/sources-no-files/MAKE
- **Commit:** 25ae723

**2. [Rule 2 - Missing critical functionality] Config validation in the step**
- **Found during:** Task 2
- **Issue:** The contract refuses bad arguments but did not cover an out-of-range config flag or a nonzero reserved byte.
- **Fix:** `ivs_config_valid` refuses them with `IVS_E_BAD_ARG` before any state is touched.
- **Files modified:** core/src/cpu/cp1610.c
- **Commit:** fbaaf10

**3. [Scope order] Harness and generator features landed in Task 1, not Task 2**
- **Found during:** Task 1
- **Issue:** The plan builds a minimal generator and harness in Task 1 and completes them in Task 2. Writing the full schema, `--only`, `--list`, uninit-read, the waits table, EBCI, the call log and the failure diffs once was cheaper than writing them twice.
- **Fix:** All of these are in commit 5b4fc72. Task 2's commit holds the core work (CLRC, SETC, wait accounting, line and config validation) and the six vectors. Behaviour and acceptance are unchanged.
- **Commit:** 5b4fc72

**4. [Rule 1 - Bug, cite form] The cites carry no section number**
- **Found during:** Task 2
- **Issue:** My first draft prefixed the pdf 56 cites with a section label I had not read from the page.
- **Fix:** Cites name the table or figure and the page (`Control instructions table, CLRC row, pdf 56; instruction fetch figure, pdf 28`). They are ASCII and carry `pdf N`, which plan 02-04's checker requires. A-06's `S3.3.4 ...` example is a form for cites with a known section, not a required prefix.
- **Files modified:** data/cp1610/vectors/control.json
- **Commit:** fbaaf10

**Total deviations:** 4 (1 Rule 3, 1 Rule 2, 1 Rule 1, 1 task-order shift). **Impact:** none on scope or the interface contract.

## Authentication Gates

None.

## Known Stubs

- `core/src/cpu/cp1610.c` (`case IVS_CP1610_RS_HALTED`): the HALTED unit reports 0 cycles and no state change until the halt policy lands (plan 02-14, POL-HLT-01).
- `core/src/cpu/cp1610.h` (`IVS_CP1610_FAULT_UNIMPLEMENTED`): a temporary fault reason for decles without a row; plan 02-13 removes it once every row exists.
- `core/src/cpu/cp1610.c` (`ivs_cp1610_step`): lines are validated but not acted on until plan 02-14 (interrupt, BUSRQ and STPST policies).

All three are scheduled by the plan text itself; none blocks this plan's goal.

## Threat Flags

None. All new surface is in the plan's threat model (bus replies and caller state into the stepper, JSON into the generator, TAP into the gate). No endpoints, auth paths or file access in the core.

## Issues Encountered

None beyond the deviations above.

## Next Phase Readiness

Ready for 02-03 (suites `cpu-api` and `harness`). `ivs_test_run` takes explicit FILE streams, so the harness self-test can drive it directly. `ivs_testbus` keeps a call log for `cpu-api.bus-order-matches-log`. Requirements CPU-01, CPU-04, CPU-05, RUN-03 and PKG-02 are advanced but stay Pending in REQUIREMENTS.md, because each needs later plans of this phase.

## Self-Check: PASSED

- Created files found: all files listed under key-files exist (checked with `git ls-files`; `ivs_vectors.inc` is in the build tree only).
- Commits 5b4fc72, fbaaf10, 25ae723 are ancestors of HEAD; `git rev-list --count 42723a4..HEAD` = 3 at close of the task commits.
