---
phase: 04-headless-runner-and-installed-consumer
plan: 07
subsystem: api, machine, tests
tags: [api, run, bounded-execution, D-01, D-02, D-03, D-04, D-05, D-07, API-02]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-06: ivs/ivs_run.h with the observation calls, suite run (22 cases) and its SP4 snapshot oracle; steps=29/29, controls=249/249, faults=127/127, tests-expected=2248, api-calls=12, api-rows=32"
provides:
  - "ivs_machine_run (core/src/machine/machine_bound.c): absolute master-tick target, while now < target one unit through the internal seam, no state between calls, run(now + 1) is one unit"
  - "ivs_run_request v1 16 bytes {struct_size, flags, target_ticks}; ivs_run_report v1 56 bytes {struct_size, stop_reason, refusal_reason, cpu_run_state, cpu_fault_reason, reserved, now_ticks, overshoot_ticks, elapsed_ticks, units_executed}"
  - "IVS_RUN_F_STOP_ON_HALT 1, IVS_RUN_F_RESERVED_FRAME 2 (UNSUPPORTED), IVS_RUN_TARGET_MAX 2^63, IVS_STOP_TARGET 1 / CPU_HALTED 2 / GUEST_FAULT 3 / INTERNAL 4, ivs_run_stop_name"
  - "IVS_REASON_RUN_NULL_ARG 34, RUN_STRUCT_SIZE 35, RUN_REPORT_OVERLAP 36, RUN_FLAGS 37, RUN_TAIL 38, RUN_STATE_OFF 39, RUN_TARGET_RANGE 40 (ivs/ivs_machine.h, append-only)"
  - "suite run family bound (tests/run/run_bound.c): 31 cases; project HLT and MVO@ R4,R4 stand-in programs; reference twin stepped through the seam"
  - "faults RUN_TARGET_UNCHECKED, RUN_STOP_BEFORE_OVERSHOOT, RUN_FAULT_STEPS (data/expected/faults/run.ids)"
  - "api-effects.json entries ivs_machine_run (6 precedence entries, 5 refusal rows) and ivs_run_stop_name (api-calls 14, api-rows 37)"
affects: [04-08, 04-09, 04-10, 04-11, 04-13, 04-17, 04-18]

actuals:
  tokens: 18400
  tasks: 3
  commits: 3
plan_head_before: 9c46bc340f45d4d7486dbb7fdf45ad6047b790f1
plan_head_after: e34eeb3912052653297ba13679f38a82670437fa

tech-stack:
  added: []
  patterns:
    - "Run tests take expected boundaries from a reference twin stepped unit by unit through ivs_machine_step_unit_internal (the Phase 2/3-pinned primitive), never from the call under test; whole-storage memcmp against the twin"
    - "A refusal-range test whose fault variant would otherwise run toward 2^64 is ordered to try a FAULTED CPU first, so the broken build ends at once"
    - "Refusal detail goes only into a report that is not the cause and does not overlap the bytes the handle names (uintptr_t only), whatever the handle's validity"

key-files:
  created:
    - core/src/machine/machine_bound.c
    - tests/run/run_bound.c
  modified:
    - core/include/ivs/ivs_run.h
    - core/include/ivs/ivs_machine.h
    - core/CMakeLists.txt
    - data/api/api-effects.json
    - tests/CMakeLists.txt
    - tests/run/run_main.c
    - tests/run/run_cases.h
    - tests/machine/machine_main.c
    - data/expected/tests/run.ids
    - data/expected/faults/run.ids

key-decisions:
  - "Loop: exit tests (fault, then halt with the flag) before each step; any unit that does not advance time ends the loop (INTERNAL unless the CPU is FAULTED), so the loop always terminates; stop reason order INTERNAL, GUEST_FAULT, CPU_HALTED (flag), TARGET; status IVS_OK for every stop"
  - "Refusal reasons 34..40 appended as the plan lists them; BAD_HANDLE reuses IVS_REASON_HANDLE (29); RUN_REPORT_OVERLAP names the class but is never written (the report is the cause)"
  - "No IVS_STOP_NONE: a refusal does not write stop_reason; ivs_run_stop_name(0) is IVS_STOP_UNKNOWN"

patterns-established:
  - "Every later run plan (04-10 buffers, 04-11 events) extends the ladder in machine_bound.c at the named slot after UNSUPPORTED and after OUT_OF_RANGE"

requirements-completed: []

duration: 55min
completed: 2026-11-11
---

# Phase 4 Plan 07: Bounded execution (ivs_machine_run) Summary

**`ivs_machine_run` runs a POWERED machine to an absolute master-tick target one CPU unit at a time through the internal seam, ends at the first boundary at or after the target, reports a derived overshoot and a stop reason that is a pure function of the exit state, keeps no state between calls, and refuses every bad input in the D-05 order before the first unit; 31 new cases in suite `run` and three named faults.**

## Performance

- **Duration:** about 55 min
- **Tasks:** 3 of 3 (tracer first; its two verify commands re-run green before Task 2)
- **Files:** 12 changed (1,584 insertions, 6 deletions)
- **Cost of suite `run` after this plan (W7), plain run of `build/<preset>/tests/ivs_tests_run`, 3 runs each:** dev (ASan+UBSan) 0.21-0.22 s wall (was 0.12-0.14 s); release 0.09 s (was 0.02 s), of which the refusal family is about 0.04 s (two 8-digest snapshots per refused call) and single-step 0.02 s. Three new fault executables (`fault-runs` 127 -> 130); `step-time c-faults` 5-6 s and `faults-verdict` 4-5 s, `gate-time total` 36-38 s on this host with load average 13-15.

## Counts

Start: `steps=29/29 controls=249/249 faults=127/127 tests-expected=2248 api-calls=12 api-rows=32`.
After Task 1: `api-calls=14 api-rows=32 faults=127/127 tests-expected=2258 tests-missing=0`.
After Task 2: `api-calls=14 api-rows=37 faults=127/127 tests-expected=2310 tests-missing=0`.
End: `steps=29/29 controls=249/249 api-calls=14 api-statuses=14 api-rows=37 fault-runs=130 faults=130/130 tests-expected=2310 tests-executed=2310 tests-skipped=0 tests-missing=0`. Suite `run`: 22 -> 53 cases (cap 256).

## Accomplishments

- **Header (`ivs/ivs_run.h`, D-01..D-04):** request and report structs (64-bit members 8-aligned, sizes pinned by `_Static_assert` in `machine_bound.c`), flags, `IVS_RUN_TARGET_MAX`, stop reasons, one-line `IVS_API` prototypes, and a header comment stating: target in master ticks, CPU-cycle and frame bounds are host sugar (D-09), no state between calls, stops are not refusals, and the full refusal order with reasons.
- **`machine_bound.c`:** `ivs_bound_validate` over const inputs fills a stack plan (target, flags); the ladder comment names the D-05 buffer/event slot after UNSUPPORTED (plans 04-10, 04-11) and the BUFFER_TOO_SMALL and per-event checks after OUT_OF_RANGE. Commit loop through `ivs_machine_step_unit_internal`; report filled last, `overshoot_ticks = now > target ? now - target : 0`.
- **Refusal ladder (D-05):** BAD_ARG (req NULL -> RUN_NULL_ARG; req size -> RUN_STRUCT_SIZE; rep NULL or size -> nothing written), BAD_HANDLE (IVS_REASON_HANDLE), BAD_ARG (rep inside the instance, nothing written), UNSUPPORTED (flags incl. the reserved frame bit -> RUN_FLAGS, then tail -> RUN_TAIL), BAD_STATE (OFF -> RUN_STATE_OFF), OUT_OF_RANGE (above 2^63 -> RUN_TARGET_RANGE). No new status numbers.
- **Final `IVS_REASON_RUN_*` numbers:** `RUN_NULL_ARG 34`, `RUN_STRUCT_SIZE 35`, `RUN_REPORT_OVERLAP 36`, `RUN_FLAGS 37`, `RUN_TAIL 38`, `RUN_STATE_OFF 39`, `RUN_TARGET_RANGE 40`.
- **Cases (31, family bound):** `run.target.ends-at-first-boundary-ge-target` (16 units x 3 targets inside each, fresh instance each, `now >= target` and `now - unit < target`, storage equal to the twin), `on-boundary-no-overshoot`, `already-reached` (target 0, equal, below and inside the last unit: zero units, SP4 oracle unchanged), `max-accepted` (2^63 on a FAULTED and on a HALTED-with-flag CPU); `run.overshoot.equals-now-minus-target` (every target 1..b[12] in sequence); `run.single-step.equals-seam` (40 x `run(now + 1)` vs 40 seam steps, digests per boundary, storage at the end); `run.stop.halt-flag`, `halt-no-flag` (h + 40 exact, h + 41 ends at h + 44), `guest-fault`, `fault-sticky`, `fault-before-halt` (HLT then MVO@ R4,R4: halt latch set and FAULTED, stop GUEST_FAULT with the flag), `internal-step-refused` (damaged CPU run state: IVS_OK, INTERNAL, zero units, storage unchanged); 11 `run.refuse.*` and 6 `run.prec.*` cases with the SP4 oracle (status, reason, every other report byte kept, storage memcmp, digests and time); `run.stop-name.known/unknown`.
- **Faults (Task 3):** `RUN_TARGET_UNCHECKED` fails only `run.refuse.target-above-2-63` (status IVS_OK on the faulted CPU instead of OUT_OF_RANGE); `RUN_STOP_BEFORE_OVERSHOOT` fails `run.target.ends-at-first-boundary-ge-target` (`now 0`, want 20 for target 1) plus allowed collateral under the same rule; `RUN_FAULT_STEPS` fails `run.stop.fault-sticky` (the fault boundary equal to the target reports TARGET) plus the other guest-fault stops and `run.target.max-accepted`. Must-pass ids `run.stop-name.known` and `run.cpu-state.off`. Each checked by hand (no hang) before the gate.
- **TDD (Task 2):** RED was the compile failure of the 26 new cases against the Task 1 code (`use of undeclared identifier 'IVS_REASON_RUN_NULL_ARG'`, 17 errors; the Task 1 ladder also lacked the UNSUPPORTED stage and refusal writes); GREEN 53/53 in dev and release.

## Task Commits

1. **Task 1 (tracer): run to an absolute target; target, overshoot, single-step cases** - `013f227` feat(04-07)
2. **Task 2: stop reasons, refusal ladder, stop names** - `2da5bf0` feat(04-07)
3. **Task 3: three run-bound faults** - `e34eeb3` test(04-07)

## Final gate summary lines

- `sh tools/gate.sh` (macOS, Apple clang 21): `gate: PASS steps=29/29 controls=249/249 api-calls=14 api-statuses=14 api-rows=37 cc=AppleClang c-presets=2 fault-runs=130 ... faults=130/130 ... tests-expected=2310 tests-executed=2310 tests-skipped=0 tests-missing=0`; `gate-cpu user=57.26 sys=67.62`, `gate-time total=38`.
- `dash tools/gate.sh`: the same tokens, `gate: PASS`; `gate-cpu user=48.34 sys=57.34`, `gate-time total=36`.
- `sh tools/gate-linux.sh` (ubuntu:24.04 x86_64 emulated, `gate-env ncpu=8 cc=GNU-13.3.0`, dash as /bin/sh): `gate: PASS steps=29/29 controls=249/249 api-calls=14 api-statuses=14 api-rows=37 cc=GNU c-presets=2 fault-runs=130 ... faults=130/130 ... tests-expected=2310 tests-executed=2310 tests-skipped=0 tests-missing=0`, `gate-time total=174` (emulated, not a hosted time); last line `gate-linux: exit 0 (log build/gate-linux.log)`. As in earlier container runs it shows `refs-verified=0/4` and `core-imports=3` where macOS shows `1/4` and `4` (not tokens this plan touches).

## Deviations from Plan

### Auto-fixed / discretion

**1. [Rule 3 - blocking] Test callback renamed**
- `tests/machine/machine_main.c` had a file-static suite callback named `ivs_machine_run`, which collided with the new public prototype (compile error). Renamed `ivs_machine_suite_run`; no behaviour change. File not in the plan's list.

**2. Report write guard (stricter than load)**
- A refusal writes `struct_size` and `refusal_reason` only into a report that passes the size rule and does not overlap the instance bytes the handle names, tested by address even for an invalid or destroyed handle (no dereference). So a report aimed into a destroyed instance's storage gets nothing (`run.refuse.bad-handle`, `run.prec.handle-over-overlap`), as `cpu_state` behaves. Status precedence is unchanged (the BAD_ARG overlap stage still applies only to a valid instance).

**3. Reasons**
- The plan's seven `IVS_REASON_RUN_*` are appended as listed; BAD_HANDLE reuses the existing `IVS_REASON_HANDLE` (29). `RUN_REPORT_OVERLAP` (36) can never be written into the report it concerns; it is kept so the class has a name (effect table, runner diagnostics) and documented as such.

**4. Stop names and ids**
- No `IVS_STOP_NONE` (a refusal does not write `stop_reason`); `ivs_run_stop_name(0)` is `IVS_STOP_UNKNOWN`. Test ids made concrete: `run.refuse.null-request/null-report`, `run.refuse.struct-size-request/-report`; extra cases `run.target.max-accepted`, `run.stop.internal-step-refused`, `run.prec.flags-over-tail` (reason precedence inside UNSUPPORTED).

**5. Fault models**
- `RUN_STOP_BEFORE_OVERSHOOT` takes back the overshooting unit's CPU state and time (its memory writes stay): it models "stops at the last boundary below the target" as observed through now and the CPU. `RUN_FAULT_STEPS` steps a faulted CPU once (a zero-progress unit ends the loop in every build, T-04-23) and tests the target before the fault; `RUN_TARGET_UNCHECKED`'s must-fail case tries a FAULTED CPU first so the broken build cannot run toward 2^64.

## Open Items

- API-02 stays Pending: the bounded call, overshoot and stop reasons exist; the slicing-invariance test (D-07, plan 04-09) and the reason/stop-reason pins (`api-reasons.ids`, `api-stop-reasons.ids`, plan 04-08) close it with later plans. Plan 04-08 must pin reasons 34..40 above.
- `ivs_machine_run` is not yet exercised from the installed package (plan 04-13 should call it and pin the v1 struct sizes 16 and 56, or the sizes after 04-10/04-11 append fields).
- Carried from 04-06: RUN-05 Pending (nothing writes the VIDEO/AUDIO chains or the INPUT row yet; 04-10, 04-11); RUN-01 Pending (runner 04-17..04-19).
- Carried from 04-04: plan 04-14 adds `tools/pack/` to the c-build listed-source loop.
- Carried from 04-02..04-06: the orchestrator records hosted runs with `sh tools/ci-record.sh <run-id>` and applies D-45 after 04-10 and 04-19; PKG-03/PKG-04 Pending.
- Carried from 04-01..04-06: the job summary lists `pending-resolvers=17` and `unknown-fields=16` as skips-and-missing; `sh tools/hygiene.sh --history` still fails on earlier planning commits (orchestrator decision; this plan's commits add no home paths).

## Threat Flags

None beyond the plan's threat model: T-04-21 mitigated (OUT_OF_RANGE above 2^63 before any unit; `RUN_TARGET_UNCHECKED`; 2^63 accepted boundary); T-04-22 (overlap after BAD_HANDLE with uintptr_t arithmetic; report inside and straddling the instance refused with storage memcmp; positive control after the instance bytes of a longer allocation); T-04-23 (a faulted CPU is never stepped; any zero-progress unit ends the loop; `RUN_FAULT_STEPS`; seam refusal stops INTERNAL); T-04-SC (no package installs).

## Self-Check: PASSED

- Files present: core/src/machine/machine_bound.c, tests/run/run_bound.c and the ten modified files.
- Commits reachable from HEAD: 013f227, 2da5bf0, e34eeb3 (`git rev-list --count 9c46bc3..HEAD` = 3).
