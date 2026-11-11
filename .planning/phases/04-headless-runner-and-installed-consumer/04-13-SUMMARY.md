---
phase: 04-headless-runner-and-installed-consumer
plan: 13
subsystem: api, run, input, records
tags: [api, run, input, records, D-12, D-14, D-16, D6, A-02, A-05, C14, C15, RUN-01, RUN-04, RUN-05]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-12: input.controllers, the extra-word refusal, run.input.guest-invisible; 04-11: ivs_input_event, the INPUT row and chain; 04-08: the generic define pin (DEFINE_CLASSES); steps=29/29, controls=252/252, faults=140/140, tests-expected=2432, api-calls=14, pinned-defines=59"
provides:
  - "31 literal one-bit defines IVS_CTRL_KEY_1..KEY_9, KEY_CLEAR, KEY_0, KEY_ENTER (bits 0-11), IVS_CTRL_DISC_N..DISC_NNW (12-27, clockwise from north), IVS_CTRL_SIDE_TOP, SIDE_LOWER_LEFT, SIDE_LOWER_RIGHT (28-30) in core/include/ivs/ivs_run.h"
  - "Define class IVS_CTRL_ in tools/check/api-effects.cmake with the new value form `bit` ((1u << n) read as bit index n, 0..31) and the exclusion IVS_CTRL_RESERVED_MASK; pin file data/expected/input-bits.ids (NAME BITINDEX, append-only)"
  - "Controls api-input-bit-duplicate and api-input-bit-unpinned (controls=254/254)"
  - "Discrepancy records: C14 rewritten (D6 logical switch state, reversal trigger), C15 modelling choice, new C63-C67"
  - "Cases run.fuzz.seed-01..08, run.fuzz.cases-covered, run.struct.v1-sizes, run.struct.v1-sizes-lp64 (skip-ok); v1 sizes frozen: request 64 (LP64), report 112, event 32, cpu_state_report 36, ivs_info 216/256"
affects: [04-14, 04-16, 04-17, 04-19, 04-24, Phase 5]

actuals:
  tokens: 13700
  tasks: 3
  commits: 3
plan_head_before: 861d233
plan_head_after: d3a7202

tech-stack:
  added: []
  patterns:
    - "A define class value form other than an integer: the form is a column of DEFINE_CLASSES and section 6 of api-effects.cmake branches on it; `bit` reads `(1u << n)` exactly and pins n"
    - "Slicing fuzz oracle: seam-stepped boundaries plus a unit-by-unit reference through the public run, checked against an expectation computed from the boundaries and the schedule alone; every sliced call end is compared with the reference at the same boundary"
    - "Pointer-bearing struct sizes pinned in a separate case whose skip is a preprocessor test on UINTPTR_MAX (a visible skip-ok on 32-bit pointers)"

key-files:
  created:
    - data/expected/input-bits.ids
    - tools/check/controls/_good/tree/data/expected/input-bits.ids
    - tools/check/controls/api-input-bit-duplicate/
    - tools/check/controls/api-input-bit-unpinned/
    - tests/run/run_fuzz.c
  modified:
    - core/include/ivs/ivs_run.h
    - tools/check/api-effects.cmake
    - tools/check/controls/_good/tree/core/include/ivs/ivs_run.h
    - tools/check/controls/api-stop-unpinned/overlay/core/include/ivs/ivs_run.h
    - tools/check/controls/api-reason-unpinned/overlay/core/include/ivs/ivs_run.h
    - data/expected/controls/api-effects.ids
    - data/discrepancies.json
    - data/expected/discrepancies.ids
    - data/expected/faults/run.ids
    - data/expected/tests/run.ids
    - tests/CMakeLists.txt
    - tests/run/run_cases.h
    - tests/run/run_main.c

key-decisions:
  - "The bit value form accepts exactly `(1u << n)` (u or U, blanks optional, n 0..31 without a leading zero) and pins n; `(1 << n)`, `(1u << 32)` and `(1u<<05)` are api-effects-format (checked by hand in a scratch tree)"
  - "The duplicate control overlays both the toy header and the toy pin (two names on bit 0 on each side), so it fails only api-define-duplicate and forbids renumbered, unpinned, orphan and format"
  - "C14 stays open with resolver `pending: Phase 5 control-to-byte table and D10 controller measurement`; C63-C66 are hardware questions with `measurement: D10 ...` resolvers, C67 (unit-start granularity) has a `test:` resolver"
  - "ivs_run_report is pointer-free and 112 bytes on every ABI, so it is pinned unconditionally; only the request needs the LP64 case"

patterns-established:
  - "Non-vacuity of a fuzz: a cases-covered case replays the seeds and requires each interesting situation (event in an overshoot window, equal ticks, event on a call target, applied events, events left pending) to have occurred"

requirements-completed: []

duration: about 35 min
completed: 2026-11-11
---

# Phase 4 Plan 13: input bit names, D6 records, input/slicing fuzz and v1 struct sizes Summary

**The 31 logical controller input names are now literal one-bit defines in `ivs_run.h`, pinned by bit index in `data/expected/input-bits.ids` through a new `bit` value form of the generic define pin, with two controls. The discrepancy log records the D6 decision (C14, C15) and five open questions (C63-C67). A seeded fuzz compares random input schedules under random slicings with a unit-by-unit reference, and the v1 sizes of the Phase 4 public structs are frozen.**

## Performance

- **Duration:** about 35 min (load average 30-38 during the gates).
- **Tasks:** 3 of 3 (the tracer first; its gate passed under sh and dash before Task 2).
- **Files:** 18 changed (5 created).
- **Suite time (`run`, measured on this laptop at load above 30):** dev (ASan+UBSan) 0.84 s for the whole suite, of which `--only run.fuzz.` is 0.13 s; release 0.43 s, `run.fuzz.` 0.01 s. Per seed: 8 to 27 events, 15 to 42 sliced calls and 96 reference calls. The fault reruns of the run suite each pay the same 0.13 s at most.
- **Gate (D-45 local data):** Task 1 gate (sh) `gate-time total=34`; final gates `gate-time total=52` (sh) and `51` (dash). Every step time rose by 1-9 s between the two, the unrelated `controls` step from 20 to 29 s, so the difference is laptop load, not the new cases.

## Counts

Start: `steps=29/29 controls=252/252 faults=140/140 tests-expected=2432 api-calls=14 pinned-defines=59 discrepancies=62 open=57 pending-resolvers=17`.
After Task 1: `controls=252/252 pinned-defines=90` (59 + 31).
After Task 2: `controls=254/254`.
End: `steps=29/29 controls=254/254 api-calls=14 api-statuses=14 api-rows=38 pinned-defines=90 fault-runs=140 faults=140/140 discrepancies=67 pending-resolvers=17 open=62 tests-expected=2454 tests-executed=2454 tests-skipped=0 tests-missing=0` (11 new cases, counted in both presets). `grep -c -E '^#define IVS_CTRL_[A-Z0-9_]+ \(1u << [0-9]+\)' core/include/ivs/ivs_run.h` = 31; `grep -c '^IVS_CTRL_KEY_1 0$' data/expected/input-bits.ids` = 1; `grep -c -E '^C6[3-7]$' data/expected/discrepancies.ids` = 5; `grep -c 'logical switch state per controller' data/discrepancies.json` = 1. The last discrepancy id used is **C67**; plan 04-16 continues at C68.

## What changed

- **Header (D-12, D-16):** 31 defines, one per line, `#define IVS_CTRL_<NAME> (1u << n)`: keys 1-9 on bits 0-8, CLEAR 9, 0 10, ENTER 11; disc N, NNE, NE, ENE, E, ESE, SE, SSE, S, SSW, SW, WSW, W, WNW, NW, NNW on bits 12-27; SIDE_TOP 28 (the two top buttons as one code, C15), SIDE_LOWER_LEFT 29, SIDE_LOWER_RIGHT 30. `IVS_CTRL_RESERVED_MASK` stays. The comment says the bits are logical switches, the word is indexed by PSG port, and the electrical byte is a derived Phase 5 observation. The request and report comments now state their frozen v1 sizes and the A-02 rule.
- **Define pin:** `DEFINE_CLASSES` gains `IVS_CTRL_|data/expected/input-bits.ids|bit|IVS_CTRL_RESERVED_MASK`. Section 6 of `api-effects.cmake` has a `bit` branch next to `uint`. Duplicate bit indices are `api-define-duplicate` through the existing per-class value check.
- **Toy tree (same commit as the checker):** the toy `ivs_run.h` gains `IVS_CTRL_KEY_1 (1u << 0)`, `IVS_CTRL_KEY_2 (1u << 1)` and a toy reserved mask (which exercises the exclusion), and a toy `input-bits.ids` pins the two bits.
- **Records (A-05):** C14 keeps `open`. Its `default_policy` now reads `unknown-default-chosen: logical switch state per controller indexed by PSG port (D6, delegated decision): ...` and names the reversal trigger. Its resolver is `pending: Phase 5 control-to-byte table and D10 controller measurement`, and its note names D6, plan 04-13, C16 and C63-C67. C15 records the modelling choice (the top pair is one bit; a host maps either top button to it) and stays open. New entries: C63 disc internals and two positions closed together; C64 unplugged connector and upper-byte read; C65 read of a port the PSG drives as output; C66 which connector is R14; C67 input application at CPU-unit start. C63-C66 have `measurement: D10 ...` resolvers; C67 has a `test:` resolver.

## Cases

- `run.fuzz.seed-01..08`: each is one fixed xorshift32 seed over stand-in content A. Boundaries come from a seam-stepped twin. The reference runs `run(now + 1)` 96 times and must match an expectation computed from the boundaries and the schedule alone: consumed = events with `at_tick <= b[k - 1]`, words = the last of them, floor `b[k - 1] + 1`, one unit per call. The sliced replay picks targets from five modes (now + 1..48, one of the next three pending events' ticks, inside one of the next four units, one of the next four boundaries, now itself) and ends with a target inside the last unit. Every sliced call must end on a reference boundary and match the reference there: consumed, `controller_now`, floor, INPUT and CPU digests. The final storage must equal the reference byte for byte.
- `run.fuzz.cases-covered`: across the eight seeds, 8 events lay inside an overshoot window, 31 pairs had equal ticks, 86 pending events sat on a call target, 135 were applied and 5 were still pending at the end (measured with a temporary diagnostic that was not committed).
- `run.struct.v1-sizes`: `ivs_input_event` 32 (flags 8, controller 16); `ivs_cpu_state_report` 36 (r 12, s 28, intr_enabled 32, reserved 33); `ivs_run_report` 112 (reserved 20, now_ticks 24, units_executed 48, audio_bytes_written 72, input_consumed 80, input_failing_index 84, input_floor_tick 88, controller_now 96); `ivs_info` v1 end 216 (`ticks_per_cpu_cycle`), `nominal_frame_ticks` 232, whole 256; request prefix flags 4, target_ticks 8, video 16; every size at most `IVS_STRUCT_SIZE_MAX`.
- `run.struct.v1-sizes-lp64` (skip-ok): request 64 with video_capacity 24, audio 32, audio_capacity 40, events 48, event_count 56, reserved_in 60. Skipped where `UINTPTR_MAX` is not 64-bit. It runs on macOS arm64 and Linux x86_64 (the gate shows `tests-skipped=0`).

## Deviations from Plan

**1. [Rule 3 - Blocking] Two existing control overlays changed (not in the plan's file list).** `api-stop-unpinned` and `api-reason-unpinned` overlay the whole toy `ivs_run.h`. Without the new toy `IVS_CTRL_*` lines their trees would have pinned bits with no define, and `api-define-orphan`, which both FORBID, would fire. Both overlays now carry the same three lines as the good tree, so each still fails only for its own token. Commit `cebed60`.

**2. [Rule 2 - Strength] The duplicate control overlays the toy pin as well as the header.** With only the header changed, `api-define-renumbered` would fire alongside the duplicate. Putting both names on bit 0 on each side makes the control fail only `api-define-duplicate`, and its FORBID list covers renumbered, unpinned, orphan and format. Commit `c3de177`.

**3. [Rule 3 - Blocking] `data/expected/faults/run.ids`: six allowed prefixes widened with `run.fuzz.` (not in the plan's file list).** The fuzz catches six existing faults: RUN_STOP_BEFORE_OVERSHOOT, RUN_BUDGET_CARRY, INPUT_APPLY_AT_RUN_START, INPUT_LOG_SLICE_RELATIVE_TICK, INPUT_STATIC_CARRY and RUN_EVENT_AT_TARGET_ONLY. The fault verdict reported this as `fault-wrong-reason` until their prefixes allowed it. INPUT_STATIC_CARRY is caught because the reference and sliced instances run one after the other, so the carried static leaks into the second. Comments were added; the must-fail and must-pass ids are unchanged and `faults=140/140` holds. Commit `d3a7202`.

**4. [Rule 2 - Doc] `core/include/ivs/ivs_run.h` changed in Task 3 as well:** the request and report comments said "grows until plan 04-13 pins it" and now state the frozen v1 sizes and the A-02 rule. Commit `d3a7202`.

**5. The non-vacuity case also requires applied events and events left pending.** The plan named three situations; the two extra ones keep a schedule generator that never left an event unconsumed from passing silently. The first generator did exactly that (no event past the last unit start), so the tick range was widened to the horizon + 512 ticks.

**6. ivs_info sizes pinned in the run suite.** The 04-09 open item asked to pin `sizeof(ivs_info) == 256` and the v1 size 216 "from the installed package". They are pinned here from the public header (the header the package installs). The installed-consumer check itself belongs to the packaging plans.

## Open Items

- RUN-01, RUN-04 and RUN-05 stay Pending. RUN-01 needs the runner's `input` lines (04-16 and later, the parser maps names to the bits pinned here; 04-17). RUN-05 needs the goldens. RUN-04 closes with the important-gates register (04-24).
- The installed consumer should pin the v1 sizes from the installed headers (request 64 on LP64, report 112, `ivs_info` 216/256), so the package view is proven equal to the build view (04-14 and later).
- The minimum `struct_size` of the request and report is still `sizeof` in `machine_bound.c`. It equals the frozen v1 size now; the first plan that appends a field must introduce a v1-size constant (as `IVS_INFO_V1_SIZE`) so the minimum stays the v1 size (A-02). `run.struct.*` fails on any growth, which forces that step.
- Phase 5: C14 and C63-C67 are revisited with the control-to-byte table and the D10 measurements; `run.input.guest-invisible` becomes a positive port-read test.
- Carried: `API-02`, `PKG-03`, `PKG-04` Pending; 04-24 may register the input faults in `important-gates.ids`; job summary `pending-resolvers=17`, `unknown-fields=16`; `sh tools/hygiene.sh --history` still fails on earlier planning commits (this plan's commits add no home paths).

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=29/29 controls=254/254 api-calls=14 api-statuses=14 api-rows=38 pinned-defines=90 cc=AppleClang ... fault-runs=140 ... discrepancies=67 pending-resolvers=17 open=62 faults=140/140 ... tests-expected=2454 tests-executed=2454 tests-skipped=0 tests-missing=0 ...`
- `dash tools/gate.sh`: same line, `gate: PASS`.
- `sh tools/gate-linux.sh` (Ubuntu 24.04 container, x86_64, gcc 13.3.0, cmake 3.28.3): `gate: PASS steps=29/29 controls=254/254 ... pinned-defines=90 cc=GNU ... fault-runs=140 ... faults=140/140 ... tests-expected=2454 tests-executed=2454 tests-skipped=0 tests-missing=0`, `gate-time total=203`, exit 0. Not required by the plan; run so the LP64 size pins are seen to hold under gcc on Linux as well as on macOS (`run.struct.v1-sizes-lp64` executed, not skipped).

## Threat Flags

None beyond the plan's threat model:
- T-04-37 mitigated: the `IVS_CTRL_` define pin with controls `api-input-bit-duplicate` and `api-input-bit-unpinned`.
- T-04-38 mitigated: C14 and C15 rewritten, C63-C67 recorded with D10 or test resolvers; the discrepancies step is green (`discrepancies=67`).
- T-04-39 mitigated: the seeded fuzz with its non-vacuity case, shown to catch six existing timing and isolation faults.
- T-04-SC: no package installs.

## Self-Check: PASSED

- Files present: data/expected/input-bits.ids, tools/check/controls/_good/tree/data/expected/input-bits.ids, tools/check/controls/api-input-bit-duplicate/, tools/check/controls/api-input-bit-unpinned/, tests/run/run_fuzz.c and the modified files listed above.
- Commits reachable from HEAD: cebed60, c3de177, d3a7202 (`git rev-list --count 861d233..HEAD` = 3 before the docs commit).
