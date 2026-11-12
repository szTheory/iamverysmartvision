---
phase: 04-headless-runner-and-installed-consumer
plan: 09
subsystem: api, run
tags: [api, run, slicing, info, D-06, D-07, D-09, A-02, API-02]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-07: ivs_machine_run (no state between calls); 04-08: define pins; steps=29/29, controls=252/252, faults=130/130, tests-expected=2310, api-calls=14"
provides:
  - "ivs_info grows from 216 to 256 bytes: ticks_per_cpu_cycle, cpu_cycles_per_scanline, scanlines_per_frame, video_format, nominal_frame_ticks (u64 at 232), video_bytes_per_frame, audio_format, audio_bytes_per_frame, reserved2; IVS_OUTPUT_FORMAT_NONE 0u"
  - "A-02 rule in ivs_machine_info: minimum struct_size IVS_INFO_V1_SIZE (216, internal core/src/machine/machine.h, _Static_assert = offsetof(ivs_info, ticks_per_cpu_cycle)); exactly min(struct_size, 256) bytes written, struct_size = that count; overlap test over the bytes written"
  - "Profile generator emits scanlines_per_frame and cpu_cycles_per_scanline (mm_uint, 1..65535, fail closed) into ivs_profile_desc; same output name and -DROOT -DOUT interface"
  - "Suite run families info (4 cases) and slice (11 cases); faults RUN_BUDGET_CARRY, INFO_TAIL_UNGUARDED"
affects: [04-10, 04-13, 04-16, 04-17]

actuals:
  tokens: 8850
  tasks: 2
  commits: 2
plan_head_before: 7bcfa1734b556cd89b37ef42f85e057754f5a9fc
plan_head_after: 4ed59d2caa47b7015d1de0ae7b3d378484c7f795

tech-stack:
  added: []
  patterns:
    - "Grown out struct (A-02): the v1 size stays the minimum; the core fills a local and copies min(caller size, own size) bytes"
    - "Slicing oracle: a seam-stepped twin records boundary times and CPU digests; each call's expected end boundary, units, elapsed, overshoot and stop come from it; full digests and storage only at two sampled ends; calls capped so a stall fails instead of hanging"

key-files:
  created:
    - tests/run/run_info.c
    - tests/run/run_slice.c
  modified:
    - cmake/IvsMemmapTable.cmake
    - core/src/memmap/memsys.h
    - core/include/ivs/ivs_machine.h
    - core/src/machine/machine.c
    - core/src/machine/machine.h
    - core/src/machine/machine_bound.c
    - tests/machine/machine_create.c
    - data/api/api-effects.json
    - docs/time-base.md
    - tests/CMakeLists.txt
    - tests/run/run_main.c
    - tests/run/run_cases.h
    - data/expected/tests/run.ids
    - data/expected/faults/run.ids

key-decisions:
  - "The slicing reference is stepped through the internal seam, not run(now + 1), so a run-path fault cannot move its own oracle; run(now + 1) is the single pattern, and run.single-step.equals-seam already ties the two"
  - "End targets come from each program's own stop: HALTED boundary + 41 (inside a 4-tick idle unit, so the last call overshoots) and FAULTED boundary + 1000 (later calls run nothing)"
  - "RUN_BUDGET_CARRY subtracts the last overshoot from the next target only when the call continues the same handle from the boundary the last call ended at, so fresh instances are unaffected"

patterns-established:
  - "A new invariance family is allowed collateral for older faults it also catches: widen their prefixes in the faults file, keep their must-fail ids"

requirements-completed: []

duration: about 45 min
completed: 2026-11-11
---

# Phase 4 Plan 09: ivs_info growth under A-02 and slicing invariance Summary

**`ivs_info` now publishes the profile timebase (with `nominal_frame_ticks` = 59,736 on NTSC, nominal, not a frame event) and the output layout (NONE, sizes 0), still accepting a Phase 3-sized caller without touching its tail; eleven `run.slice.*` cases prove that any split of a run (single steps, 7-tick, prime, seeded and one-call slices, and target edges) gives the state, digests and counters of a unit-by-unit reference.**

## Performance

- **Duration:** about 45 min
- **Tasks:** 2 of 2 (tracer first; its verify re-run green before Task 2)
- **Files:** 16 changed (751 insertions, 22 deletions)
- **Suite `run` cost (plain run of `build/<preset>/tests/ivs_tests_run`, 3 runs each, load average 11-14):** dev (ASan+UBSan) 0.27-0.28 s (04-07: 0.21-0.22 s); release 0.12 s (04-07: 0.09 s). The slice family alone (`--only run.slice.`): dev 0.12 s, release 0.03 s, well under the plan's 2 s budget. Program H has 32 reference units (E = 609 ticks), program F 19 (fault at 540 ticks); calls per case 1 to 220.
- **Gate:** `step-time c-faults` 4-5 s, `faults-verdict` 4 s, `gate-time total` 33-37 s (04-08: 39-43 s at load 12-13).

## Counts

Start: `steps=29/29 controls=252/252 faults=130/130 tests-expected=2310 api-calls=14`.
After Task 1: `faults=130/130 tests-expected=2318 tests-missing=0`.
End: `steps=29/29 controls=252/252 api-calls=14 api-statuses=14 api-rows=37 pinned-defines=45 fault-runs=132 faults=132/132 tests-expected=2340 tests-executed=2340 tests-skipped=0 tests-missing=0`. Suite `run`: 53 -> 68 cases (cap 256).

## Accomplishments

- **Generator (Task 1):** `scanlines_per_frame` and `cpu_cycles_per_scanline` are registry scalars read with `mm_uint` (max 65535, zero fails closed with `must be nonzero`), appended to the positional `ivs_profile_desc` initializer; the header comment lists them. `GEN_MAP_MUTATION` and c-purity's generated-include review pass unchanged.
- **`ivs_info` (D-06, D-09, A-02):** fields appended after `slots[3]` with the byte offsets documented in the header; `sizeof(ivs_info) == 256` and 8-alignment of `nominal_frame_ticks` asserted in `machine.c`. `nominal_frame_ticks` is the product of the three profile figures in 64-bit arithmetic. The header comment carries the A-02 rule and "nominal, not a frame event".
- **Cases (info):** `run.info.timebase` (equal to the registry record, and the literal 59736 from `docs/time-base.md`), `run.info.layout-none` (over a poisoned struct), `run.info.v1-size-caller` (sizes 216 and 232 inside a larger poisoned buffer: OK, `struct_size` = caller size, bytes equal to a full caller's, every byte past the size untouched), `run.info.short-refused` (215: BAD_ARG, buffer and instance unchanged with the SP4 snapshot). `machine.refuse.info.short` retargeted to `IVS_INFO_V1_SIZE - 1` in the same commit; `machine.info.larger-out` still sees `struct_size == sizeof(ivs_info)`.
- **Docs:** `api-effects.json` row of `ivs_machine_info` (on_ok, the v1 minimum in the BAD_ARG check, overlap over the bytes written, an A-02 note; `run.info.short-refused` added to the BAD_ARG tests); one line in `docs/time-base.md` naming `ivs_info.nominal_frame_ticks`.
- **Slicing (Task 2, D-07):** programs H (MVII #8, R1; DECR R1; BNZE back; HLT; TSTR R1 follower; idle units) and F (same loop; MVO@ R4, R4 sticky fault), decles hand-assembled from `data/cp1610/vectors` rows (BNZE displacement derived from `cpu.BNZE.back-taken`'s one's-complement rule). Each pattern checks per call, from the reference: end boundary, `now_ticks`, `units_executed`, `elapsed_ticks`, `overshoot_ticks`, stop reason (GUEST_FAULT at the fault boundary) and the CPU digest; at the first call end past the reference midpoint and at the end, the composite, the seven domain digests and whole-storage memcmp against a second seam-stepped twin; summed units and elapsed ticks against the reference end. `run.slice.edges`: target on boundary 3, `b[4] + 1`, `b[5] - 1` (below now), `b[5]` (equal), then E.
- **Faults:** `RUN_BUDGET_CARRY` fails 9 slice cases (all but the two one-call cases) plus `run.overshoot.equals-now-minus-target` and `run.single-step.equals-seam`; must-fail `run.slice.halt.fixed-7`. `INFO_TAIL_UNGUARDED` fails only `run.info.v1-size-caller`. Both checked by hand before the gate (no hang: the single-step stall under the carry fault is stopped by the call cap).
- **TDD (Task 2):** the behaviour already held by construction (04-07 keeps no state between calls), so RED evidence is the fault builds: `RUN_BUDGET_CARRY`, `RUN_STOP_BEFORE_OVERSHOOT` and `RUN_FAULT_STEPS` all fail `run.slice.*`; GREEN 11/11 in dev and release.

## Task Commits

1. **Task 1 (tracer): timebase and layout in ivs_info under A-02** - `03754a9` feat(04-09)
2. **Task 2: five-pattern slicing invariance and two faults** - `4ed59d2` test(04-09)

## Final gate summary lines

- `sh tools/gate.sh` (macOS, Apple clang 21): `gate: PASS steps=29/29 controls=252/252 api-calls=14 api-statuses=14 api-rows=37 pinned-defines=45 cc=AppleClang c-presets=2 fault-runs=132 ... faults=132/132 ... tests-expected=2340 tests-executed=2340 tests-skipped=0 tests-missing=0 ...`; `gate-time total=33`.
- `dash tools/gate.sh`: the same tokens, `gate: PASS`; `gate-time total=37`.
- `sh tools/gate-linux.sh` (ubuntu container, gcc, dash as /bin/sh; not required by the plan, run because the struct growth and new C are gcc-sensitive): `gate: PASS steps=29/29 controls=252/252 api-calls=14 api-statuses=14 api-rows=37 pinned-defines=45 cc=GNU c-presets=2 fault-runs=132 ... faults=132/132 ... tests-expected=2340 tests-executed=2340 tests-skipped=0 tests-missing=0`, `gate-time total=179` (emulated); last line `gate-linux: exit 0 (log build/gate-linux.log)`. As before it shows `refs-verified=0/4` and `core-imports=3` where macOS shows `1/4` and `4`.

## Deviations from Plan

**1. [Rule 1 - Bug] Collateral of the 04-07 faults**
- The new slice family also fails `RUN_STOP_BEFORE_OVERSHOOT` (10 slice cases) and `RUN_FAULT_STEPS` (the five fault-program cases), which `faults-verdict` reported as `fault-wrong-reason`. Their allowed prefixes gained `run.slice.` with a comment; must-fail ids unchanged. Commit `4ed59d2`.

**2. RUN_BUDGET_CARRY collateral and allowed prefixes**
- The plan named `run.slice.|run.target.`; the carry also breaks `run.overshoot.*` and `run.single-step.*` (both slice one run over consecutive calls), so those prefixes are allowed too. Must-pass `run.info.timebase`.

**3. Reference stepping**
- The plan says the reference runs `run(now + 1)` per unit; it steps the internal seam instead (key decision above). The single pattern is the `run(now + 1)` form.

**4. Partial-field caller sizes**
- As the plan states, info copies exactly `min(struct_size, 256)` bytes, so a size that ends inside a field gets part of it; hosts pass a header's `sizeof`, which always ends on a field. Not tested beyond the field-aligned sizes 216 and 232.

## Open Items

- API-02 stays Pending: slicing invariance holds on stand-ins; D-07 requires it on the real diagnostic (plan 04-16), and the frame-target convenience is runner sugar over `nominal_frame_ticks` (plans 04-17..04-19); close-out 04-25 lists API-02. RUN-05 stays Pending (VIDEO/AUDIO/INPUT chains, 04-10, 04-11).
- Plan 04-10: the internal env seam supplies a synthetic layout; `ivs_info`'s real layout stays NONE/0 until a profile has output. Plan 04-13: pin `sizeof(ivs_info) == 256` and `IVS_INFO_V1_SIZE` 216 from the installed package (with the run struct sizes 16 and 56 or their grown sizes).
- Plans 04-10/04-11: every new `IVS_REASON_*`/`IVS_STOP_*` needs its pin line in `data/expected/api-reasons.ids`/`api-stop-reasons.ids` in the same commit. Plan 04-13 adds the `IVS_CTRL_` pin class.
- Any later run-path fault should expect `run.slice.` collateral (the family judges every call).
- Carried from 04-07/04-08: 04-14 adds `tools/pack/` to the c-build listed-source loop; the orchestrator records hosted runs with `sh tools/ci-record.sh <run-id>` and applies D-45 after 04-10 and 04-19; PKG-03/PKG-04 Pending; job summary lists `pending-resolvers=17` and `unknown-fields=16`; `sh tools/hygiene.sh --history` still fails on earlier planning commits (this plan's commits add no home paths).

## Threat Flags

None beyond the plan's threat model: T-04-26 mitigated (A-02 copy bound; `run.info.v1-size-caller` with a poisoned tail; `INFO_TAIL_UNGUARDED`); T-04-27 mitigated (five patterns and edges against a seam reference; `RUN_BUDGET_CARRY`; c-purity writable-data rule still passes, the only statics sit inside the fault block); T-04-SC (no package installs).

## Self-Check: PASSED

- Files present: tests/run/run_info.c, tests/run/run_slice.c and the fourteen modified files.
- Commits reachable from HEAD: 03754a9, 4ed59d2 (`git rev-list --count 7bcfa17..HEAD` = 2).
