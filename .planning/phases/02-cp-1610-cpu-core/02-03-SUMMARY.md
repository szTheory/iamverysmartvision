---
phase: 02-cp-1610-cpu-core
plan: 03
subsystem: cpu-core
tags: [c17, cmake, cp1610, harness, tap, test-inventory, contract-tests]

requires:
  - phase: 02-cp-1610-cpu-core
    plan: 02
    provides: internal CPU API cp1610.h, unit stepper with reset entry, CLRC and SETC, TAP harness, flat test bus with call log, c-tests and tests-inventory gate steps
provides:
  - suite cpu-api (16 ids) pinning sizes, validation, refusal without mutation, access-log rules, determinism and instance isolation
  - suite harness (4 ids) proving the harness library itself, built from ivs_test.c alone
  - inventories data/expected/tests/cpu-api.ids and data/expected/tests/harness.ids; tests-inventory now counts three suites in both presets
affects: [02-04, 02-05, 02-08, 02-12, 02-13, 02-14]

actuals:
  tokens: 8300
  tasks: 2
  commits: 2

tech-stack:
  added: []
  patterns:
    - "Contract cases are plain functions in an explicit case table; IVS_CHECK prints the failing line and expression as a diagnostic"
    - "Refusal cases pre-fill the log and result with 0xA5 and byte-compare state, log, result and the bus call count after each refused call"
    - "Equality cases first assert the run did what the program says (api_run_sane), so a comparison of two broken runs cannot pass"
    - "The harness self-test captures inner suites through tmpfile() and echoes any mismatch as '#'-prefixed diagnostic lines only"

key-files:
  created:
    - tests/cpu/cpu_api.c
    - tests/harness/harness_selftest.c
    - data/expected/tests/cpu-api.ids
    - data/expected/tests/harness.ids
  modified:
    - tests/CMakeLists.txt

key-decisions:
  - "The harness self-test executable compiles tests/harness/ivs_test.c directly and links only ivs_warnings and ivs_san, so it cannot depend on the core or the test bus"
  - "refuse-small-log covers log_cap 0..7 (0 is also below IVS_CP1610_MAX_ACCESSES), with log_cap 8 as the accepted control"
  - "bus-order-matches-log compares kind, flags, address and offset between port call and log record, data only for WRITE and DW; for reads the port must see data 0 and wait 0 while the log carries the read value and the applied wait (plan 02-02 decision)"

patterns-established:
  - "Every refusal case also checks that the bus was never called and that the log and result buffers were not written"
  - "Determinism and isolation cases run on two separate static test buses and compare call logs, memory images and init bitmaps as well as results"

requirements-completed: []
requirements-advanced: [CPU-05, CPU-04, CPU-01, RUN-03]

coverage:
  - id: D1
    description: "Suite cpu-api runs in both presets through ctest, writes build/gate/<preset>/cpu-api.txt and is counted by tests-inventory against data/expected/tests/cpu-api.ids"
    requirement: RUN-03
    verification:
      - kind: other
        ref: "sh tools/gate.sh (tests-expected=54 tests-executed=54 tests-skipped=0 tests-missing=0); grep -c '^ok ' build/gate/release/cpu-api.txt = 16"
        status: pass
    human_judgment: false
  - id: D2
    description: "State validation accepts a zeroed state and rejects bad run_state, bad fault_reason, nonzero reserved bytes and status bits 0-3; sizes are 32 and 12 bytes"
    requirement: CPU-01
    verification:
      - kind: unit
        ref: "tests/cpu/cpu_api.c#cpu-api.state-size-32, access-size-12, validate-zeroed-state, validate-rejects-*"
        status: pass
    human_judgment: false
  - id: D3
    description: "Every refusal (each NULL argument, log_cap 0..7 with a log, an invalid state) returns its IVS_E_* code and leaves state, log, result and bus untouched"
    requirement: CPU-01
    verification:
      - kind: unit
        ref: "tests/cpu/cpu_api.c#cpu-api.refuse-null-args, refuse-small-log, refuse-invalid-state"
        status: pass
    human_judgment: false
  - id: D4
    description: "Access-log rules: NULL log (any log_cap) equals a full log; the port is called once per log record in log order with the same records"
    requirement: CPU-05
    verification:
      - kind: unit
        ref: "tests/cpu/cpu_api.c#cpu-api.null-log-equivalent, bus-order-matches-log"
        status: pass
    human_judgment: false
  - id: D5
    description: "Reset is idempotent and ignores prior contents; a never-reset state reports NOT_RESET with no access and no change"
    requirement: CPU-04
    verification:
      - kind: unit
        ref: "tests/cpu/cpu_api.c#cpu-api.reset-idempotent, not-reset-unit"
        status: pass
    human_judgment: false
  - id: D6
    description: "Repeated and interleaved two-instance runs are byte-identical to single runs (no shared mutable state)"
    requirement: CPU-01
    verification:
      - kind: unit
        ref: "tests/cpu/cpu_api.c#cpu-api.deterministic-repeat, two-instances-interleaved"
        status: pass
    human_judgment: false
  - id: D7
    description: "Suite harness proves duplicate-id rejection, the --only filter, the skip line and exit 77, and trailer counts without CPU code"
    requirement: RUN-03
    verification:
      - kind: unit
        ref: "tests/harness/harness_selftest.c#harness.duplicate-id-rejected, only-filter, skip-line-format, trailer-counts; grep -c '^ok ' build/gate/dev/harness.txt = 4"
        status: pass
    human_judgment: false

duration: 6min
completed: 2026-11-10
status: complete
plan_head_before: ee0366ad8a9ad8b4edc5aa475d4d1b737b5ec6a5
plan_head_after: 5e5545da595c074b6448e7922ef04fd0dfe9c58d
commits: 2
---

# Phase 2 Plan 03: cpu-api contract suite and harness self-test Summary

**The internal CPU contract (sizes, validation, refusal without mutation, NULL-log equivalence, bus/log ordering, NOT_RESET, reset idempotency, determinism, two-instance isolation) is pinned by a 16-case `cpu-api` suite, and a 4-case `harness` suite proves the TAP harness itself. Both run in both presets and are counted by the inventory.**

## Performance

- **Duration:** about 6 min from the first file write (plan and source reading before that)
- **Started:** 2026-11-10T05:48:47Z
- **Completed:** 2026-11-10T05:54:47Z
- **Tasks:** 2 (1 tracer, 1 auto)
- **Files:** 5 changed in the two task commits (4 created, 1 modified; 900 insertions)

## Accomplishments

- `tests/cpu/cpu_api.c`, suite `cpu-api`, 16 ids:
  - Sizes and validation: `state-size-32`, `access-size-12`, `validate-zeroed-state`, `validate-rejects-bad-run-state` (all 256 values, 0..4 accepted), `validate-rejects-bad-fault-reason` (all 256 values, 0..5 accepted), `validate-rejects-reserved-nonzero` (each of the 8 bytes), `validate-rejects-status-low-bits` (each of bits 0-3; `0xF0` accepted).
  - Refusals: `refuse-null-args` (s, cfg, lines, port, port->bus, out; also `reset(NULL)` and `config_default(NULL)`), `refuse-small-log` (log_cap 0..7), `refuse-invalid-state` (eight corruptions). Each case byte-compares state, log and result before and after and checks the bus was never called. Each ends with an accepted control call.
  - Log rules: `null-log-equivalent` (log NULL with caps 0, 1, 7, 8 and SIZE_MAX against a full log, including bus calls) and `bus-order-matches-log` (call count per unit equals `n_accesses`, records match in order, IAB at offset 1, FETCH at offset 0, a 3-cycle wait on the second fetch lengthens that unit by 3 over its row's base cycles).
  - Units: `not-reset-unit` (three steps of a zeroed state) and `reset-idempotent` (reset twice, reset of a 0xA5-filled struct, reset of a state that has run: all byte-identical).
  - Determinism and isolation: `deterministic-repeat` (two identical bus images; results, logs, final states, call logs, memory images and init bitmaps identical) and `two-instances-interleaved` (different programs, bases $1000/$2000 and waits 2/5; interleaved equals alone).
- `tests/harness/harness_selftest.c`, suite `harness`, 4 ids. Inner suites run through `ivs_test_run` into `tmpfile()` streams and the output is compared exactly with the `ivs_test.h` format: duplicate ids rejected before any case runs (also with `--list`); `--only` plan, numbering, unselected cases not run, `--list` filtered, empty selection exit 0, bad arguments are usage errors; skip line `ok N <id> # SKIP <reason>` and exit 77 when all cases are skipped; trailer counts with a fail, a skip and a source line, and an unfiltered empty suite exits 1.
- `tests/CMakeLists.txt`: `ivs_tests_cpu_api` (harness library, core, warnings, sanitizers) and `ivs_tests_harness` (compiled from `harness_selftest.c` plus `ivs_test.c`, linking only `ivs_warnings` and `ivs_san`). Both are registered with `SKIP_RETURN_CODE 77`.

## Task Commits

1. Task 1 (tracer): 5cc9758 - `feat(02-03): cpu-api suite with size and validation cases in ctest, TAP and inventory`
2. Task 2: 5e5545d - `feat(02-03): refusal, log, determinism and isolation cases plus harness self-test`

Tracer feedback gate: after Task 1 the tracer `<verify>` ran end to end. `cmake --build --preset dev && ctest --preset dev` passed at 100 percent (2 tests), then `sh tools/gate.sh` gave `gate: PASS steps=20/20 controls=128/128 ... tests-expected=28 tests-executed=28 tests-skipped=0 tests-missing=0`. Only then did the expansion task start.

## Files Created/Modified

- `tests/cpu/cpu_api.c`: CPU contract suite.
- `tests/harness/harness_selftest.c`: harness self-test suite.
- `tests/CMakeLists.txt`: two executables and two ctest registrations.
- `data/expected/tests/cpu-api.ids`: 16 ids with the header comment `# Suite cpu-api: internal CPU contract (plan 02-03).`
- `data/expected/tests/harness.ids`: 4 ids with a header comment.

## Verification results

- `sh tools/gate.sh` (unpiped, final run): `gate: PASS steps=20/20 controls=128/128 cc=AppleClang c-presets=2 ctest-runs=2 ... tests-expected=54 tests-executed=54 tests-skipped=0 tests-missing=0 ...`. Wall time 21.7 s (`7.99s user 12.93s system 96% cpu 21.668 total`).
- `dash tools/gate.sh`: `gate: PASS steps=20/20 controls=128/128 ...`.
- Task 1 acceptance (after the Task 1 gate): `grep -c '^ok ' build/gate/release/cpu-api.txt` = 7 and `grep -c '^not ok'` = 0. `grep -c -F 'SKIP_RETURN_CODE 77' tests/CMakeLists.txt` = 3 at that point (the header comment mentions it once) and 4 after Task 2.
- Task 2 acceptance: `grep -c '^ok ' build/gate/release/cpu-api.txt` = 16 (dev also 16) and `grep -c '^ok ' build/gate/dev/harness.txt` = 4 (release also 4). `ctest --preset dev -N` lists cpu-vectors, cpu-api and harness.
- Determinism acceptance: `ctest --preset dev` ran twice, with `IVS_TEST_OUT_DIR` set to two scratch dirs and `IVS_TEST_NONCE=fixed`. `cmp` exits 0 for `cpu-vectors.txt`, `cpu-api.txt` and `harness.txt`.
- Mutation probes (throwaway edits, each restored with `git checkout -- <file>` before commit, nothing committed):
  - The small-log check weakened to `log_cap < 7`: `refuse-small-log` fails.
  - A state write on the BAD_STATE path: `refuse-invalid-state` fails.
  - A static counter in the stepper added to `elapsed`: `bus-order-matches-log`, `deterministic-repeat`, `not-reset-unit`, `null-log-equivalent` and `two-instances-interleaved` fail (and 4 cpu-vectors).
  - The log dropping the applied wait: `bus-order-matches-log` fails.
  - The harness skip format changed to `#SKIP`: `harness.skip-line-format` and `harness.trailer-counts` fail.

## Decisions Made

- The harness self-test target compiles `ivs_test.c` itself instead of linking `ivs_test_harness`. The plan asks for a link with `ivs_warnings` and `ivs_san`, and that library also carries the test bus and `ivs_core`. Compiling it in keeps the suite independent of CPU code by construction (threat T-02-45).
- `refuse-small-log` includes log_cap 0, because 0 is also below the 8-record minimum (must-have truth: "log_cap below IVS_CP1610_MAX_ACCESSES").
- `bus-order-matches-log` follows the plan 02-02 hand-off. It compares data directly only for WRITE and DW. For reads it checks that the port saw data 0 and wait 0, and that the log holds the bus word (FETCH) or the vector (IAB) and the applied wait.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Harness self-test mismatch output leaked bare TAP lines to stdout**
- **Found during:** Task 2 (mutation probe on the skip format)
- **Issue:** The first `hs_text` printed the expected and captured inner output as one diagnostic with embedded newlines, so lines such as `not ok 2 t.b` reached the outer suite's stdout without a `#` prefix. The result file was unaffected (diagnostics never go there), but any reader of stdout would see stray results.
- **Fix:** `hs_diag_lines` prints each captured line as its own `#`-prefixed diagnostic (`#   want | ...`, `#   got  | ...`).
- **Files modified:** tests/harness/harness_selftest.c
- **Verification:** The probe re-ran and showed only `#` lines plus the outer `ok`/`not ok` lines.
- **Committed in:** 5e5545d

**2. [Scope note] Harness target links the harness source, not the harness library**
- See Decisions Made. The behaviour asked for is unchanged, and the links to `ivs_warnings` and `ivs_san` are exactly as planned.

**Total deviations:** 1 auto-fixed (Rule 1) and 1 scope note. **Impact:** none on scope or the interface contract. No core source changed.

## Authentication Gates

None.

## Known Stubs

None. This plan adds tests only. The stubs that plan 02-02 recorded (HALTED unit, temporary UNIMPLEMENTED fault, lines validated but not acted on) are unchanged and still scheduled for plans 02-13 and 02-14.

## Threat Flags

None. T-02-44 is mitigated by the byte-compare refusal cases under ASan+UBSan in the dev preset. T-02-45 is mitigated by the `harness` suite, built without core code. No new surface.

## Issues Encountered

None beyond the deviation above.

## User Setup Required

None.

## Next Phase Readiness

Ready for 02-04 (fault path and vector data checker). New core behaviour must keep `cpu-api` green. Two points matter for later plans:
- `api_run_sane` assumes one access per CLRC/SETC unit and is used only with those programs.
- `bus-order-matches-log` already handles WRITE and DW data, so it extends to multi-access units without change.

CPU-01, CPU-04, CPU-05 and RUN-03 are advanced but stay Pending, because each needs later plans of this phase.

---
*Phase: 02-cp-1610-cpu-core*
*Completed: 2026-11-10*

## Self-Check: PASSED

- Created files found: tests/cpu/cpu_api.c, tests/harness/harness_selftest.c, data/expected/tests/cpu-api.ids, data/expected/tests/harness.ids.
- Commits 5cc9758 and 5e5545d are ancestors of HEAD; `git rev-list --count ee0366a..HEAD` = 2 at close of the task commits.
