---
phase: 04-headless-runner-and-installed-consumer
plan: 06
subsystem: api, machine, tests
tags: [api, observation, D-08, A-01, A-06, API-02, RUN-01]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-05: instance 113,600 bytes with obs rows; digest domains 16..18; steps=29/29, controls=249/249, faults=125/125, tests-expected=2200, api-calls=10"
provides:
  - "public header core/include/ivs/ivs_run.h (included by ivs/ivs.h): ivs_cpu_state_report (v1 36 bytes), IVS_CPU_STATE_* and IVS_CPU_FAULT_* constants"
  - "ivs_machine_cpu_state and ivs_machine_peek (core/src/machine/machine_inspect.c): side-effect-free, valid OFF and POWERED, refused before any write"
  - "suite run (tests/run/, data/expected/tests/run.ids): 22 cases; shared SP4 snapshot oracle ivs_run_snap_take/same/free in run_cases.h for later run families"
  - "faults PEEK_ADVANCES_TIME and PEEK_GATE_IGNORED (data/expected/faults/run.ids, # suite=run)"
  - "api-effects.json entries ivs_machine_cpu_state and ivs_machine_peek (api-calls 12, api-rows 32)"
affects: [04-07, 04-11, 04-13, 04-17, 04-18, 04-19]

actuals:
  tokens: 14500
  tasks: 2
  commits: 2
plan_head_before: ace741cab299552332433a32bf94ab8a1edaec01
plan_head_after: a569084c6b1814db76d56204c85df30112c99d10

tech-stack:
  added: []
  patterns:
    - "Suite run reuses tests/machine/machine_standin.c (stand-in content A/B) by adding tests/machine to its include path; no stem collision (test sources are not owner TUs)"
    - "Refusal oracle helper ivs_run_snap_*: whole-storage copy, composite plus seven domain digests and info time_ticks, compared after the refused call; destroyed instances take a storage-only snapshot"
    - "A fault that must touch const state goes through a uintptr_t round trip (no -Wcast-qual finding) inside its #ifdef block only"

key-files:
  created:
    - core/include/ivs/ivs_run.h
    - core/src/machine/machine_inspect.c
    - tests/run/run_main.c
    - tests/run/run_cases.h
    - tests/run/run_observe.c
    - data/expected/tests/run.ids
    - data/expected/faults/run.ids
  modified:
    - core/include/ivs/ivs.h
    - core/CMakeLists.txt
    - tests/CMakeLists.txt
    - data/api/api-effects.json
    - data/expected/tests/headers.ids

key-decisions:
  - "ivs_cpu_state_report is 36 bytes (struct_size, run_state, fault_reason, r[8], s, z, ov, c, intr_enabled, reserved[3]); IVS_CPU_STATE_* and IVS_CPU_FAULT_* mirror the CPU's internal run states and fault reasons, pinned by _Static_assert in machine_inspect.c (a new CPU fault reason fails the build until it is made public)"
  - "Out-struct rule for cpu_state: struct_size below 36 or above IVS_STRUCT_SIZE_MAX is BAD_ARG with nothing written; a larger size is accepted, the core writes 36 bytes and sets struct_size 36, the caller tail stays untouched (A-02 from first release)"
  - "Both observation calls use ivs_machine_span_overlaps directly (not ivs_machine_out_overlaps), so the machine fault OUT_OVERLAP_UNCHECKED does not reach them; their overlap class is pinned by run.refuse.*.out-inside-instance"

patterns-established:
  - "Run, observation, buffer and input tests go in suite run with its own fault inventory; the machine suite stays at 391 of 512 cases"

requirements-completed: []

duration: 30min
completed: 2026-11-11
---

# Phase 4 Plan 06: Public read-only guest observation (cpu_state, peek) and suite run Summary

**A new public header `ivs/ivs_run.h` declares `ivs_cpu_state_report`, `ivs_machine_cpu_state` and `ivs_machine_peek` (D-08 signatures exactly); both read only const state through the memory system's read view, are valid OFF and POWERED, refuse before any write, and are proven side-effect-free over 1000 calls each by storage memcmp, eight digests and machine time, in the new 22-case suite `run` with two named faults.**

## Performance

- **Duration:** about 30 min
- **Tasks:** 2 of 2 (tracer first; tracer gate re-verified end-to-end before Task 2)
- **Files:** 12 changed (1,299 insertions, 1 deletion)
- **Cost of the new suite `run` (W7), plain run of `build/<preset>/tests/ivs_tests_run`, 3 runs each:** dev (ASan+UBSan) 0.12-0.14 s wall; release 0.02 s wall. Two new fault executables (`fault-runs` 125 -> 127), each running the 22-case suite (about 0.1 s each); `step-time c-faults` 6-7 s and `faults-verdict` 6-7 s, `c-tests` 4 s, no visible rise against 04-05 (5-7 s and 8-9 s). Load average 14-16 during the gates.

## Counts

Start: `steps=29/29 controls=249/249 faults=125/125 tests-expected=2200 api-calls=10 api-rows=27`.
After Task 1: `steps=29/29 controls=249/249 api-calls=11 api-rows=29 faults=125/125 tests-expected=2222 tests-missing=0`.
End: `steps=29/29 controls=249/249 api-calls=12 api-rows=32 fault-runs=127 faults=127/127 tests-expected=2248 tests-executed=2248 tests-skipped=0 tests-missing=0`. Suite `run`: 22 cases (cap 256 in `run_main.c`); headers: 8 ids (6 -> 8). Machine suite unchanged at 391 of 512.

## Accomplishments

- **Header (A-01):** `core/include/ivs/ivs_run.h`, included by `ivs/ivs.h`: `IVS_CPU_STATE_UNRESET/RESET_PENDING/RUNNING/HALTED/FAULTED` (0..4), `IVS_CPU_FAULT_NONE..BUS_WAIT_LIMIT` (0..4), `ivs_cpu_state_report` (36 bytes), and one-line `IVS_API` prototypes for both calls. The header comment states the out-struct rule (A-02 from first release), the side-effect contract and that the caller supplies `count` words for peek.
- **`machine_inspect.c`:** `ivs_machine_cpu_state` (BAD_ARG for NULL or struct_size outside 36..4096, BAD_HANDLE, BAD_ARG overlap, local report copied out last) and `ivs_machine_peek` (BAD_ARG for NULL out or count 0, BAD_HANDLE, OUT_OF_RANGE for `(uint64_t)addr + count > 0x10000` before any span arithmetic, BAD_ARG when the `count * 2`-byte span overlaps the instance, then `ivs_memsys_read(&im->mem, ...)` over the const memory system). `_Static_assert`s pin the public run-state and fault-reason numbers to the CPU's internal ones.
- **Suite `run` (A-06):** `tests/run/run_main.c` (bounded concatenation of families), `run_cases.h` (family declarations plus the shared SP4 snapshot oracle `ivs_run_snap_take/same/free`), `run_observe.c` (family observe). Stand-in content comes from `tests/machine/machine_standin.c`, compiled into the run executable and its fault suite.
- **Cases (22):** cpu_state OFF, after reset (equals the CPU's own fields), after 1, 2 and 4 stepped units of program A (literal registers), flag mapping, larger struct (64 and 4096 bytes, tail untouched); peek loaded words (EXEC, GROM, cartridge literals from the stand-in generators, OFF and POWERED, raw-poked $FFFF reads $03FF), gated (GROM and GRAM behind closed gate gmem read $FFFF like a CPU READ through `ivs_memsys_bus`, data again after reopening), undriven, after-steps (`[$0200] == $155`, R1 `$156`), range end (`$FFFF,1`, `$F000,$1000`, the whole space); side effects OFF and POWERED (1000 peeks across the space plus 1000 cpu_state calls: storage memcmp, composite and seven domain digests, info time_ticks equal); 10 refusal cases (4 cpu_state, 6 peek) with poisoned outs, storage memcmp and, where the handle is valid, digests and time equal. Positive controls in the overlap cases: an out struct or span ending exactly where the instance begins or beginning where it ends is accepted.
- **Effect table:** `ivs_machine_cpu_state` (3 precedence entries, 2 refusal rows) and `ivs_machine_peek` (4 precedence entries, 3 refusal rows: BAD_ARG, BAD_HANDLE, OUT_OF_RANGE), all effect `none`, every row citing `run.refuse.*` ids.
- **Faults (`data/expected/faults/run.ids`, `# suite=run`):** `PEEK_ADVANCES_TIME` (an accepted peek adds one master tick) fails `run.observe.no-side-effect-off`, `-powered` and `run.refuse.peek.out-inside-instance` (its positive controls run before the final snapshot comparison); must-pass `run.cpu-state.off`. `PEEK_GATE_IGNORED` (gates opened around the read and put back) fails only `run.peek.gated`; must-pass `run.peek.loaded-words`. Both checked by hand before the gate.
- **TDD (Task 2):** RED was the compile failure of the peek cases against the Task 1 header (`call to undeclared function 'ivs_machine_peek'`); GREEN after the header line and implementation, 22/22.

## Task Commits

1. **Task 1 (tracer): public CPU snapshot, new header, suite run** - `1a8b676` feat(04-06)
2. **Task 2: ivs_machine_peek, refusal rows of both calls, two faults** - `a569084` feat(04-06)

## Final gate summary lines

- `sh tools/gate.sh` (macOS, Apple clang 21): `gate: PASS steps=29/29 controls=249/249 api-calls=12 api-statuses=14 api-rows=32 cc=AppleClang c-presets=2 fault-runs=127 ... faults=127/127 ... tests-expected=2248 tests-executed=2248 tests-skipped=0 tests-missing=0`; `gate-cpu user=64.86 sys=76.95`, `gate-time total=47`.
- `dash tools/gate.sh`: the same tokens, `gate: PASS`; `gate-cpu user=56.08 sys=69.54`, `gate-time total=45`.
- `sh tools/gate-linux.sh` (ubuntu:24.04 x86_64 emulated, gcc 13.3.0, cmake 3.28.3): `gate: PASS steps=29/29 controls=249/249 api-calls=12 api-rows=32 cc=GNU c-presets=2 fault-runs=127 ... faults=127/127 ... tests-expected=2248 tests-executed=2248 tests-skipped=0 tests-missing=0`, `gate-time total=382` (emulated, under load). Not required by the plan; run because a public header (C++ header case under g++) and new core C landed. The container line shows `refs-verified=0/4` and `core-imports=3` where macOS shows `1/4` and `4`, as in earlier container runs (not tokens this plan touches).

## Deviations from Plan

### Auto-fixed / discretion

**1. Overlap helper name**
- The plan names `ivs_machine_span_overlaps`; it exists and is used. `ivs_machine_info` uses the wrapper `ivs_machine_out_overlaps` (which the machine fault `OUT_OVERLAP_UNCHECKED` disables); the observation calls deliberately call `ivs_machine_span_overlaps` directly, so a machine-suite fault cannot silently remove their overlap class while the run suite (not rerun under machine faults) stays green.

**2. Extra cases beyond the plan's named ids**
- `run.cpu-state.after-steps`, `run.cpu-state.flag-bits`, `run.cpu-state.larger-struct`, `run.peek.loaded-words`, `run.peek.gated`, `run.peek.undriven`, `run.peek.after-steps`, `run.peek.range-end-ok`, `run.observe.no-side-effect-off/-powered`: the plan's behaviour list made concrete; all in `run.ids`.

**3. Public fault-reason constants**
- `IVS_CPU_FAULT_*` (not named in the plan) so `fault_reason` is interpretable through the package alone; a `_Static_assert` ties `IVS_CPU_FAULT_BUS_WAIT_LIMIT` to `IVS_CP1610_FAULT_LAST`.

**4. PEEK_ADVANCES_TIME collateral**
- Allowed prefixes `run.observe.|run.peek.|run.refuse.peek.`; actual collateral is `run.refuse.peek.out-inside-instance` (accepted positive-control peeks before its last snapshot comparison).

## Open Items

- API-02 and RUN-01 stay Pending: this plan gives them the public read-only access path (RUN-01 assertions, RUN-06 predicate), but the bounded run call (04-07) and the runner executable and scenarios (04-17..04-19) close them.
- `ivs_cpu_state_report` and `ivs_machine_peek` are not yet exercised from the installed package; the installed consumer (04-13) should call both.
- Carried from 04-05: RUN-05 Pending (nothing writes the VIDEO/AUDIO chains or the INPUT row yet; run loop 04-07, input log 04-11).
- Carried from 04-04: plan 04-14 adds `tools/pack/` to the c-build listed-source loop.
- Carried from 04-02..04-05: the orchestrator records hosted runs with `sh tools/ci-record.sh <run-id>` and applies D-45 after 04-10 and 04-19; PKG-03/PKG-04 Pending.
- Carried from 04-01..04-05: the job summary lists `pending-resolvers=17` and `unknown-fields=16` as skips-and-missing; `sh tools/hygiene.sh --history` still fails on earlier planning commits (orchestrator decision; this plan's commits add no home paths).

## Threat Flags

None beyond the plan's threat model: T-04-18 mitigated (overlap after BAD_HANDLE with uintptr_t arithmetic; poisoned outs and storage memcmp in every refusal case; adjacency positive controls); T-04-19 (const reads only; both faults; 1000-call memcmp, digest and time equality OFF and POWERED); T-04-20 (OUT_OF_RANGE above 0x10000 in 64-bit arithmetic, before the span size is formed; `range-before-overlap`); T-04-SC (no package installs).

## Self-Check: PASSED

- Files present: all seven created files and five modified files.
- Commits reachable from HEAD: 1a8b676, a569084 (`git rev-list --count ace741c..HEAD` = 2).
