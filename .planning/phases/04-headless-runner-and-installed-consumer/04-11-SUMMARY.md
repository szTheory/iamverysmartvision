---
phase: 04-headless-runner-and-installed-consumer
plan: 11
subsystem: api, run, input
tags: [api, run, input, digest, D-05, D-12, D-13, D-14, D-15, D-16, A-04, A-08, RUN-01, RUN-05]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-10: ivs_machine_run ladder with the D-05 buffer slot and the env seam; 04-05: INPUT row obs.input {words, count, floor_tick, chain}; LEVER-01: faster control runner and faults-verdict; steps=29/29, controls=252/252, faults=134/134, tests-expected=2372, api-calls=14"
provides:
  - "ivs_input_event (32 bytes: u64 at_tick, u32 flags, u32 reserved, u32 controller[4] by PSG port), IVS_INPUT_CONTROLLER_WORDS 4, IVS_CTRL_RESERVED_MASK 0x80000000, IVS_INPUT_EVENTS_MAX 65536, IVS_INPUT_INDEX_NONE"
  - "ivs_run_request gains events, event_count, reserved_in (64 bytes on LP64); ivs_run_report gains input_consumed, input_failing_index, input_floor_tick, controller_now[4] (112 bytes)"
  - "core/src/machine/machine_input.c: ivs_input_init (h0 = SHA-256(\"IVS-INPUT-LOG-V1\") at create and reset), ivs_input_check (whole-array validation, two passes), ivs_input_apply_due (due events applied before a unit begins, one read per event)"
  - "The step seam sets the input floor (unit start + 1) when the CPU accepts a unit"
  - "Reasons IVS_REASON_INPUT_* 44..53, pinned in data/expected/api-reasons.ids"
  - "Suite run family input: 22 run.input.* and 5 run.prec.* cases; literal chain oracle h0, h1, h2, g2 from printf octal escapes and cmake -E sha256sum"
  - "Faults INPUT_APPLY_AT_RUN_START, INPUT_LOG_SLICE_RELATIVE_TICK, INPUT_NO_FLOOR_CHECK, INPUT_STATIC_CARRY, RUN_EVENT_AT_TARGET_ONLY, RUN_EVENTS_UNCHECKED"
affects: [04-12, 04-13, 04-16, 04-19, 04-24, 04-25]

actuals:
  tokens: 21700
  tasks: 3
  commits: 3
plan_head_before: 3a3b37deeef67133a509f53d63cf2e4aa54697b0
plan_head_after: e3392f8295640d4a4c938b602d8d675c5f66dc49

tech-stack:
  added: []
  patterns:
    - "Binary chain oracle: authoring-time preimages written with printf octal escapes (a POSIX sh oct() helper), hashed with cmake -E sha256sum, recipe and literals in the test header; the test rebuilds the INPUT row preimage from expected values and the literal chain and compares it byte for byte with the core's serializer"
    - "State a unit owns is set by the step seam, not the run loop, so run-stepped and seam-stepped twins stay byte-identical (the existing memcmp oracles keep working)"
    - "Whole-array validation in two passes (BAD_ARG order/range/floor, then UNSUPPORTED content) keeps the ladder a class order: the first failing class wins, then the lowest index"

key-files:
  created:
    - core/src/machine/machine_input.c
    - tests/run/run_input.c
  modified:
    - core/include/ivs/ivs_run.h
    - core/include/ivs/ivs_machine.h
    - core/src/machine/machine.h
    - core/src/machine/machine.c
    - core/src/machine/machine_bound.c
    - core/src/machine/machine_run.c
    - core/CMakeLists.txt
    - tests/CMakeLists.txt
    - tests/run/run_main.c
    - tests/run/run_cases.h
    - tests/machine/machine_digest.c
    - data/api/api-effects.json
    - data/expected/api-reasons.ids
    - data/expected/tests/run.ids
    - data/expected/faults/run.ids
    - data/expected/faults/machine.ids

key-decisions:
  - "The input floor is written by ivs_machine_step_unit_internal (start + 1 when the CPU accepts the unit), not by the run loop: the run.target, run.single-step and run.slice oracles compare whole storage with seam-stepped twins, and the floor is a property of a begun unit either way"
  - "Per-event validation is two whole-array passes after BUFFER_TOO_SMALL: BAD_ARG (at_tick >= 2^63, then below the predecessor, then below the floor, per event) and then UNSUPPORTED (flags, reserved word, bit 31); the lowest failing index of the first failing pass is reported"
  - "reserved_in is checked last in the UNSUPPORTED stage (after RUN_FLAGS and RUN_TAIL); the event slot after the buffer slot checks NULL with a count, then the 65536 cap, then overlap with instance, report, video and audio"
  - "A report that overlaps the event array gets no refusal detail (it is part of the cause; writing it would change the array); input_failing_index is written only by the per-event stages and is IVS_INPUT_INDEX_NONE on IVS_OK"
  - "The run copies each event once (ivs_input_event e = ev[next]) before using it; the header says the array must not change during the call"

patterns-established:
  - "Run-path faults that change accepted runs also fail run.input.*: widen their prefixes, keep must-fail ids"

requirements-completed: []

duration: about 95 min
completed: 2026-11-11
---

# Phase 4 Plan 11: Input events and the INPUT chain Summary

**`ivs_machine_run` now carries a caller-owned array of 32-byte logical input events. Each event takes effect before the first unit that starts at or after its `at_tick`: exit tests run first and nothing is applied at exit. The INPUT row (words in force, count, floor, chain) is live from create, and the chain matches literals made outside the code with `printf` and `cmake -E sha256sum`. The whole array is validated before any write. Six named faults are each caught by their intended case.**

## Performance

- **Duration:** about 95 min (load average 12-60; other containers were running on the laptop).
- **Tasks:** 3 of 3 (the tracer first; its gate passed before Task 2).
- **Files:** 18 changed.
- **Plain-run time per suite:** see the table below.

Times are from `ctest --test-dir build/<preset> -j1`, two runs each, in seconds, at load 24-50. Every suite reads about 2x its 04-10 figure at load 16-26, so compare the `run` row with the others measured in the same session.

| suite | dev (ASan+UBSan) | release |
|-------|------------------|---------|
| cpu-vectors | 0.18 0.21 | 0.04 0.03 |
| cpu-sweep | 0.17 0.21 | 0.02 0.01 |
| cpu-api | 0.12 0.22 | 0.00 0.00 |
| memmap | 0.17 0.22 | 0.02 0.01 |
| ivsc | 0.23 0.25 | 0.07 0.06 |
| machine | 1.01 1.16 | 0.53 0.43 |
| run | 0.96 0.95 | 0.47 0.39 |
| cpu-fuzz | 0.23 0.20 | 0.02 0.02 |
| harness | 0.67 1.22 | 0.76 1.22 |
| runner | 0.13 0.13 | 0.00 0.00 |
| core-symbols | 0.47 0.46 | 0.32 0.55 |
| headers | 0.80 0.48 | 0.42 0.43 |

  Suite `run` direct (`/usr/bin/time`): dev 0.91 s, release 0.45 s (04-10: 0.38 and 0.17 at lower load). The input family alone (`--only run.input.`): dev 0.31 s, release 0.08 s. Its largest case is `event-count-cap-ok`, which validates 65536 events twice; the 22 cases stay cheap.

- **Gate (D-45 local data):** the final gates ran at load 48-60: `sh tools/gate.sh` `gate-time total=89`, `gate-cpu user=90.31 sys=102.50`; `dash tools/gate.sh` `gate-time total=94`, `gate-cpu user=76.28 sys=90.65`. The same code before the gcc fix read 55 s (sh) and 65 s (dash) at load 14-16. The Task 1 gate read 22 s and the Task 2 gate 44 s. Six more fault executables (134 -> 140 fault runs) is the plan's main gate cost.

## Counts

Start: `steps=29/29 controls=252/252 faults=134/134 tests-expected=2372 api-calls=14 api-rows=38 pinned-defines=48`.
After Task 1: `faults=134/134 tests-expected=2376 tests-missing=0`.
After Task 2: `faults=134/134 tests-expected=2426 pinned-defines=58 tests-missing=0`.
End: `steps=29/29 controls=252/252 api-calls=14 api-statuses=14 api-rows=38 pinned-defines=58 fault-runs=140 faults=140/140 tests-expected=2426 tests-executed=2426 tests-skipped=0 tests-missing=0`. Suite `run`: 84 -> 111 cases (cap 256). `grep -c '^run.input.refuse.' data/expected/tests/run.ids` = 13.

## Chain oracle (A-08)

The recipe below is in the header of `tests/run/run_input.c`. It runs in POSIX sh and dash in a `mktemp -d` directory, and nothing from it is committed.

```sh
oct() { h=$1; while [ -n "$h" ]; do b=${h%"${h#??}"}; h=${h#??}; printf '\\%03o' "0x$b"; done; }
sha() { cmake -E sha256sum "$1" | cut -c1-64; }
printf 'IVS-INPUT-LOG-V1' > h0.in; H0=$(sha h0.in)
printf "$(oct "${H0}0000000000000046000020044000080000000000""00000000")" > h1.in; H1=$(sha h1.in)
printf "$(oct "${H1}0000000000000046100000010000000000000000""00000000")" > h2.in; H2=$(sha h2.in)
printf "$(oct "${H0}0000000000000046100000010000000000000000""00000000")" > g1.in; G1=$(sha g1.in)
printf "$(oct "${G1}0000000000000046000020044000080000000000""00000000")" > g2.in; G2=$(sha g2.in)
```

The inputs are at_tick 70 (0x46, inside unit 3 of stand-in content A, which spans ticks 52..96), event A = {0x00002004, 0x40000800, 0, 0} and event B = {0x10000001, 0, 0, 0}. Each preimage is 56 bytes: h(n), then be64 of the tick, then be32 of each of the four words.

| value | SHA-256 |
|-------|---------|
| h0 = SHA-256("IVS-INPUT-LOG-V1") | `8a922f4b648b2806561758caf4cf2614f0986267349546860a23cac3da3a79dc` (also `shasum -a 256`) |
| h1 (A at 70) | `304446b614988f7c2907a271cfaaf27455af480bade24056ad1b68d48e98267e` |
| h2 (A then B at 70) | `eb3b457e4da04583df129161a3bd413e969e7cef82e2ed83829f4223cb91e229` |
| g2 (B then A at 70) | `bc0052b53cb320d613c6d14d33e785c6a1136d3ca9d180e580eaf8477444bcf6` |

The recipe printed identical values under sh and dash. These literals check the chain's framing and order only, not hardware behaviour. The test also rebuilds the whole INPUT row preimage ("IVSD" || 18 || version 1 || words || count || floor || chain) from the expected values. It compares that preimage byte for byte with the core's serializer and checks that the public INPUT digest is its SHA-256.

## Accomplishments

- **Public types (D-12, D-13):** `ivs_input_event` and the request and report fields, exactly as in the must-haves. The header says the index is the PSG port and not the player, that the words are logical switch state, that the electrical byte is a Phase 5 observation, how application timing works, the chain rule, and that the array must not change during a call. There is no static assertion in the public header; `run.input.event-size` pins the 32 bytes and the field offsets.
- **Application (D-14, A-04):** the loop tests the target, a fault, and a halt with the flag. Only after those does it apply every event with `at_tick <= start`, in array order, and then step. The seam sets the floor to start + 1. Nothing is applied at exit. The report returns `input_consumed`, `input_failing_index` (`IVS_INPUT_INDEX_NONE`), `input_floor_tick` and `controller_now`.
- **INPUT row (D-15):** create and reset set the row to h0 with words, count and floor 0. Nothing in the machine reads the words.
- **Ladder (D-05):** `reserved_in` is UNSUPPORTED. The event slot sits after the buffer slot and before BAD_STATE. The two per-event passes come after BUFFER_TOO_SMALL. The failing index goes into the report.
- **Cases (Task 1):** `run.input.event-size` and `run.input.one-event`. In `one-event`, single steps over units 1 and 2 and a target of 71 inside unit 3 leave the event unconsumed. The call that begins unit 4 at tick 96 applies it (consumed 1, floor 97, row equals h1). A later call with no events keeps the words.
- **Cases (Task 2):** `equal-ticks` (h2 and g2), `at-target` (a boundary tick and an interior tick), `overshoot-window` (eight events, four slicings, compared byte for byte with one call; after every call the applied set is exactly the events below the floor), `halt-stops-first`, `reset`, `isolation` (two instances interleaved, each equal to its solo run), `event-count-cap-ok` (65536 events, 97 applied by a run to tick 120). There are 13 `run.input.refuse.*` cases, each with the SP4 oracle (status, reason, failing index, every other report byte still poison, storage memcmp, all digests and time). The overlap cases include accepted adjacent positive controls. There are also 5 `run.prec.*` cases.
- **TDD:** RED before the slot and stages existed: the 13 refusals, `unsupported-over-events`, `events-over-state`, `capacity-over-event-order` and `event-order-over-content` failed, and the NULL case crashed the suite. `run.prec.buffer-over-events` passed already, because the buffer slot came first. GREEN: 111/111 in dev and release.
- **Faults (Task 3):** each was run by hand before the gate. Must-fail result and collateral:

| fault | must-fail | collateral (allowed) |
|-------|-----------|----------------------|
| INPUT_APPLY_AT_RUN_START | run.input.one-event | equal-ticks, overshoot-window, halt-stops-first, event-count-cap-ok |
| INPUT_LOG_SLICE_RELATIVE_TICK | run.input.overshoot-window | one-event, equal-ticks, at-target, isolation |
| INPUT_NO_FLOOR_CHECK | run.input.refuse.below-floor | none |
| INPUT_STATIC_CARRY | run.input.isolation | equal-ticks, at-target, halt-stops-first |
| RUN_EVENT_AT_TARGET_ONLY | run.input.one-event | seven other run.input.* cases and two refusals |
| RUN_EVENTS_UNCHECKED | run.input.refuse.events-inside-instance | the three other overlap refusals |

  The must-pass ids are outside `run.input.`: run.target.ends-at-first-boundary-ge-target, run.slice.halt.fixed-7 and run.buffers.inside-instance.

## Task Commits

1. **Task 1 (tracer): input events in the run request, applied per unit, with the INPUT chain** - `0317875` feat(04-11)
2. **Task 2: whole-array event validation, floor semantics, reset and isolation** - `eca0fee` test(04-11)
3. **Task 3: six input faults; report rule ignores an over-cap event span** - `e3392f8` test(04-11)

## Final gate summary lines

- `sh tools/gate.sh` (macOS, Apple clang 21): `gate: PASS steps=29/29 controls=252/252 api-calls=14 api-statuses=14 api-rows=38 pinned-defines=58 cc=AppleClang c-presets=2 fault-runs=140 ... faults=140/140 ... tests-expected=2426 tests-executed=2426 tests-skipped=0 tests-missing=0 ...`.
- `dash tools/gate.sh`: the same summary line, byte for byte.
- `sh tools/gate-linux.sh` (ubuntu 24.04 container, x86_64, gcc 13.3, cmake 3.28, dash as /bin/sh): the first run FAILED with `tests-failed: dev/run/run.input.refuse.event-count-above-cap` (deviation 5). After the fix: `gate: PASS steps=29/29 controls=252/252 api-calls=14 api-statuses=14 api-rows=38 pinned-defines=58 cc=GNU c-presets=2 fault-runs=140 ... faults=140/140 ... tests-expected=2426 tests-executed=2426 tests-skipped=0 tests-missing=0`, `gate-cpu user=861.50 sys=109.75`, `gate-time total=238`. Last line: `gate-linux: exit 0 (log build/gate-linux.log)`. As before it shows `refs-verified=0/4` and `core-imports=3` where macOS shows `1/4` and `4`.

## Deviations from Plan

**1. [Rule 1 - Bug] The floor is set by the step seam**
- The plan's sketch sets the floor in the run loop. That would make every run-stepped instance differ from its seam-stepped twin in the INPUT row, and would break the whole-storage memcmp oracles of run.target, run.single-step and run.slice. The seam now sets `floor_tick = start + 1` when the CPU accepts a unit. The loop still applies events right before the unit begins, so the semantics are as D-14 and A-04 describe. Commit `0317875`.

**2. [Rule 1 - Bug] machine.reset.clears-observation retargeted**
- The case expected every observation byte to be zero after reset. It now expects the create value: zero except the input chain, which holds h0 and is checked to be non-zero. `OBS_NOT_CLEARED_ON_RESET` is still caught. Because stepping now writes the floor, the fault also fails `machine.reset.equivalence`, so that id was added to its allowed prefixes. This changes `tests/machine/machine_digest.c` and `data/expected/faults/machine.ids`, which the plan did not list. Commit `0317875`.

**3. [Rule 1 - Bug] Collateral of earlier run faults**
- `RUN_BUDGET_CARRY` and `RUN_STOP_BEFORE_OVERSHOOT` fail `run.input.*` (commit `0317875`). `OUTBUF_PARTIAL_WRITE` fails `run.input.refuse.events-overlap-video`, which checks its buffer arena. `RUN_CAPACITY_AFTER_STEP` fails `run.prec.capacity-over-event-order`, which then sees the per-event refusal (commit `eca0fee`). Their prefixes were widened with comments. The must-fail and must-pass ids are unchanged.

**4. Additions beyond the behaviour list**
- `IVS_INPUT_EVENTS_MAX` and `IVS_INPUT_INDEX_NONE` are new public defines. Neither belongs to a pinned define class.
- `run.prec.buffer-over-events`, `run.prec.capacity-over-event-order` and `run.prec.event-order-over-content` were added because the new slots have three neighbour pairs, not two.
- The report also refuses to receive detail when it overlaps the event array (header, `ivs_bound_report_ok`).

**5. [Rule 1 - Bug] gcc/Linux only: the report rule and an over-cap count (found by gate-linux)**
- `run.input.refuse.event-count-above-cap` failed only in the container. With `event_count = UINT32_MAX` the claimed span was 2^37 bytes from a stack address, and on Linux the stack report lay inside it. `ivs_bound_report_ok` therefore treated the report as part of the cause and wrote no detail. Whether that happens depends on memory layout. A count above `IVS_INPUT_EVENTS_MAX` names no array the core honours, so it now names no span for the report rule. The header documents this. The case gained a fixed-layout check (a report placed right after a one-event array with count 65537 must receive `INPUT_EVENT_COUNT`), so macOS covers the condition too. Commit `e3392f8`.

**6. Fault site timing**
- As in 04-10, `faults-verdict` rejects a site without an ids line, so all six sites landed in Task 3 with their ids lines. Tasks 1 and 2 had no input fault blocks.

## Open Items

- RUN-01 and RUN-05 stay Pending. This plan provides the core mechanism: the input schedule through the public run call, and the live input-log digest with an independent oracle. RUN-01 still needs the runner's `input` lines (04-16 and later), and RUN-05 needs the goldens with tier and oracle headers.
- Plan 04-12: the words 2-3 rule for the 2-controller profile should go in `ivs_input_check` pass 2 (UNSUPPORTED), with a new reason from 54 pinned in the same commit, together with `input.controllers` and the bit names. The literal oracle events keep words 2-3 at zero, so they stay valid.
- Plan 04-13: pin the request at 64 bytes (LP64; `offsetof(video) == 16`, `offsetof(events) == 48`) and the report at 112, then apply A-02. `run.input.event-size` already pins the 32-byte event.
- 04-24 can register the six input faults in `important-gates.ids` (for example `input-timing fault INPUT_APPLY_AT_RUN_START RUN-01`, `input-log fault INPUT_LOG_SLICE_RELATIVE_TICK RUN-05`).
- Carried: `API-02` and `RUN-05` Pending; 04-14 adds `tools/pack/` to the c-build listed-source loop; the orchestrator records hosted runs and applies D-45; PKG-03 and PKG-04 Pending; job summary `pending-resolvers=17`, `unknown-fields=16`; `sh tools/hygiene.sh --history` still fails on earlier planning commits.

## Threat Flags

None beyond the plan's threat model:
- T-04-31 mitigated: the event slot (NULL, cap, overlap with instance, report and both buffers) and whole-array validation run before any write; refusal rows use memcmp; `RUN_EVENTS_UNCHECKED`; the header documents the no-change contract.
- T-04-32 mitigated: ticks at or above 2^63 are BAD_ARG; the floor is in the instance; `INPUT_NO_FLOOR_CHECK`.
- T-04-33 mitigated: the absolute-tick chain has a literal oracle; `INPUT_LOG_SLICE_RELATIVE_TICK`.
- T-04-34 mitigated: no statics outside fault blocks; `INPUT_STATIC_CARRY`; the isolation case.
- T-04-SC: no package installs.

## Self-Check: PASSED

- Files present: core/src/machine/machine_input.c, tests/run/run_input.c and the sixteen modified files.
- Commits reachable from HEAD: 0317875, eca0fee, e3392f8 (`git rev-list --count 3a3b37d..HEAD` = 3).
