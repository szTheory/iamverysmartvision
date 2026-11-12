---
phase: 04-headless-runner-and-installed-consumer
plan: 10
subsystem: api, run
tags: [api, run, buffers, env-seam, D-02, D-03, D-05, D-06, D-07, API-04]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-07: ivs_machine_run ladder and loop; 04-08: define pins; 04-09: ivs_info layout (NONE, 0 bytes) and nominal_frame_ticks; steps=29/29, controls=252/252, faults=132/132, tests-expected=2340, api-calls=14"
provides:
  - "ivs_run_request gains uint8_t *video, video_capacity, uint8_t *audio, audio_capacity (48 bytes on LP64); ivs_run_report gains video_frames_completed, video_bytes_written, audio_bytes_written (80 bytes; all three 0 in Phase 4)"
  - "Internal env seam ivs_machine_run_env_internal(m, req, rep, env) and ivs_run_env (formats, bytes per frame, nominal frame ticks), declared only in core/src/machine/machine.h; ivs_machine_run wraps it with ivs_machine_profile_env, which ivs_machine_info now also uses (one source for the real layout)"
  - "Capacity rule bytes_per_frame x (max(target - now, 0) / nominal_frame_ticks + 1) per channel, BUFFER_TOO_SMALL after OUT_OF_RANGE and before any unit; D-05 buffer slot (BAD_ARG) after UNSUPPORTED and before BAD_STATE"
  - "Reasons IVS_REASON_RUN_BUFFER_SMALL 41, RUN_BUFFER_NULL 42, RUN_BUFFER_OVERLAP 43 (pinned in data/expected/api-reasons.ids)"
  - "Suite run family buffers: 12 run.buffers.* and 4 run.prec.* cases; faults RUN_CAPACITY_AFTER_STEP, OUTBUF_PARTIAL_WRITE"
affects: [04-11, 04-13, 04-16, 04-24, 04-25]

actuals:
  tokens: 19300
  tasks: 2
  commits: 2
plan_head_before: 7f6688195ca4ebb645bbdb99fc392ab4bfc628ff
plan_head_after: 5e1e29edddf5b16eb03ff4aed51326fbf4c786de

tech-stack:
  added: []
  patterns:
    - "Internal env seam: the public call builds the real environment from the profile and calls an internal function that tests drive with a synthetic one; an entry-point equality case ties the two"
    - "Buffer overlap: uintptr_t differences compared with 64-bit lengths (no size_t truncation, no wrap); a NULL pointer or length 0 names no byte"
    - "A report that overlaps an output buffer is part of the cause: no refusal detail is written into it, so no refusal touches a buffer byte"

key-files:
  created:
    - tests/run/run_buffers.c
  modified:
    - core/include/ivs/ivs_run.h
    - core/include/ivs/ivs_machine.h
    - core/src/machine/machine.h
    - core/src/machine/machine.c
    - core/src/machine/machine_bound.c
    - data/api/api-effects.json
    - data/expected/api-reasons.ids
    - data/expected/faults/run.ids
    - data/expected/tests/run.ids
    - tests/CMakeLists.txt
    - tests/run/run_main.c
    - tests/run/run_cases.h
    - tests/run/run_bound.c

key-decisions:
  - "The request and report minimum sizes move with sizeof until plan 04-13 pins them (04-13 must-have: from there A-02 applies); the request's static assertion covers offsetof(video) == 16 and sizeof <= IVS_STRUCT_SIZE_MAX (Pitfall 13), the pointer-free report is asserted at 80"
  - "The capacity formula is applied literally with max(target - now, 0), so a target at or below now still needs one frame of capacity on a channel with bytes per frame; a frame of 0 ticks with bytes above 0 can never be met, and the frame count is guarded against wrapping even below 2^63"
  - "A report that overlaps a buffer receives no refusal detail (otherwise the refusal would write a buffer byte); a NULL buffer with capacity is refused RUN_BUFFER_NULL before any overlap test and is ignored by the overlap tests"
  - "ivs_machine_profile_env is the one source of the real layout for info and run; entry-points-equal builds its env from the public ivs_info fields, not from that helper"

patterns-established:
  - "Run-path faults that change accepted runs also fail run.buffers.* (exact-fit and zero-layout run on one instance twice): widen their prefixes, keep must-fail ids"

requirements-completed: [API-04]

duration: about 40 min
completed: 2026-11-11
---

# Phase 4 Plan 10: Output buffers through an internal env seam Summary

**`ivs_machine_run` now takes caller-owned video and audio buffers with capacities and checks them in the D-05 slots (NULL/overlap BAD_ARG after UNSUPPORTED, BUFFER_TOO_SMALL after OUT_OF_RANGE, both before any unit); because the real Phase 4 layout is zero-length, the capacity contract is proven through `ivs_machine_run_env_internal` with a synthetic 16-byte, 1000-tick layout, and the public call equals the seam given an equal env.**

## Performance

- **Duration:** about 40 min (load average 15-52 during the run)
- **Tasks:** 2 of 2 (tracer first; its gate re-run green before Task 2)
- **Files:** 14 changed (1073 insertions, 72 deletions)
- **Plain-run time per suite** (`ctest --test-dir build/<preset> -j1`, two runs each, seconds, load 16-26):

| suite | dev (ASan+UBSan) | release |
|-------|------------------|---------|
| cpu-vectors | 0.11 0.11 | 0.01 0.02 |
| cpu-sweep | 0.10 0.11 | 0.01 0.01 |
| cpu-api | 0.08 0.09 | 0.00 0.00 |
| memmap | 0.11 0.12 | 0.01 0.01 |
| ivsc | 0.14 0.14 | 0.03 0.04 |
| machine | 0.52 0.56 | 0.26 0.28 |
| run | 0.38 0.41 | 0.17 0.18 |
| cpu-fuzz | 0.11 0.12 | 0.01 0.01 |
| harness | 0.09 0.10 | 0.01 0.01 |
| runner | 0.09 0.09 | 0.00 0.00 |
| core-symbols | 0.16 0.17 | 0.16 0.16 |
| headers | 0.23 0.24 | 0.23 0.24 |

  Suite `run` direct (`/usr/bin/time`): dev 0.38 s, release 0.17 s (04-09: 0.27-0.28 s and 0.12 s); the buffers family alone (`--only run.buffers.`): dev 0.12 s, release 0.02 s.
- **Gate (D-45 local data):** final `sh tools/gate.sh` `gate-time total=44`, `gate-cpu user=61.58 sys=73.69` (load 15-27); `dash tools/gate.sh` `gate-time total=42`, `gate-cpu user=55.66 sys=64.31`. Steps of note: `controls` 23 s, `faults-verdict` 7 s, `c-faults` 6 s, `hygiene-history` 6 s, `c-tests` 4 s, `cp1610-vectors` 4 s. The Task 1 gates ran at load 40-52 and read 43 s (sh) and 64 s (dash); a first failing run at load 52 read 160 s. 04-09 read 33-37 s at load 11-14.

## Counts

Start: `steps=29/29 controls=252/252 faults=132/132 tests-expected=2340 api-calls=14 api-rows=37 pinned-defines=45`.
After Task 1: `faults=132/132 tests-expected=2342 tests-missing=0 api-rows=38 pinned-defines=46`.
End: `steps=29/29 controls=252/252 api-calls=14 api-statuses=14 api-rows=38 pinned-defines=48 fault-runs=134 faults=134/134 tests-expected=2372 tests-executed=2372 tests-skipped=0 tests-missing=0`. Suite `run`: 68 -> 84 cases (cap 256).

## Accomplishments

- **Public structs (D-02, D-03):** request `video`, `video_capacity`, `audio`, `audio_capacity`; report `video_frames_completed`, `video_bytes_written`, `audio_bytes_written`. The header states the capacity rule, that Phase 4 layouts are NONE with 0 bytes so nothing is written and the counts are 0, and that the struct sizes are pinned by plan 04-13.
- **Env seam (D-06):** `ivs_run_env` and `ivs_machine_run_env_internal` in `core/src/machine/machine.h` with the "Never public" comment; the old body moved into the seam unchanged apart from the new stages. `grep -c ivs_machine_run_env_internal core/include/ivs/*.h` sums to 0; `machine.h` has it.
- **Ladder (D-05):** stage 4b BAD_ARG (`RUN_BUFFER_NULL`, then `RUN_BUFFER_OVERLAP` against the instance, the other buffer and the report) and stage 7 BUFFER_TOO_SMALL (`RUN_BUFFER_SMALL`). Every refusal leaves the whole storage, the digests, the report (except struct_size and reason) and a 256-byte poisoned buffer arena unchanged.
- **Cases:** `run.buffers.too-small-video` (at time 0 and after three units), `too-small-audio` (also both channels with only audio short), `exact-fit-video|audio|both` (two targets, one crossing a frame edge; report and storage equal to a public-call twin), `zero-layout` (0 bytes per frame, frame 1000 and frame 0), `null-with-capacity` (public and seam, capacity up to UINT64_MAX), `inside-instance` (inside, straddling the end, and a host allocation with buffers touching the instance on both sides accepted, one byte more refused), `video-audio-overlap` (same, one shared byte, nested, real layout; adjacent accepted), `report-overlap` (report shares a byte with video, with audio: nothing written; adjacent accepted), `real-layout-zero` (NULL/0 and 1, 7, 64/128-byte capacities, non-NULL with 0), `entry-points-equal` (five calls including target = now and stop-on-halt; env from public `ivs_info`); `run.prec.handle-over-buffer`, `run.prec.unsupported-over-buffer`, `run.prec.buffer-over-state`, `run.prec.range-over-capacity`.
- **Effect table:** `on_ok` describes the buffers and the zero counts; a BAD_ARG precedence entry for the buffer slot, `pinned_by` on UNSUPPORTED and OUT_OF_RANGE; the BUFFER_TOO_SMALL row (effect none); the BAD_ARG, BAD_HANDLE, UNSUPPORTED and OUT_OF_RANGE rows list the new cases; notes name reasons 34..43, the six run faults and the D-06 bound.
- **Faults (D-07):** `RUN_CAPACITY_AFTER_STEP` fails only the two too-small cases (must-fail `run.buffers.too-small-video`, must-pass `run.buffers.real-layout-zero`). `OUTBUF_PARTIAL_WRITE` fails seven cases across `run.buffers.` and `run.prec.` (must-fail `run.buffers.video-audio-overlap`, must-pass `run.buffers.exact-fit-video`). Both run by hand before the gate.
- **TDD (Task 2):** RED with the reasons defined but no buffer slot: `null-with-capacity`, `inside-instance`, `video-audio-overlap`, `report-overlap` and `run.prec.buffer-over-state` failed; GREEN 84/84 in dev and release after the slot.

## Task Commits

1. **Task 1 (tracer): output buffers and BUFFER_TOO_SMALL through the env seam** - `cd20ee2` feat(04-10)
2. **Task 2: buffer matrix, entry-point equality, two faults** - `5e1e29e` test(04-10)

## Final gate summary lines

- `sh tools/gate.sh` (macOS, Apple clang 21): `gate: PASS steps=29/29 controls=252/252 api-calls=14 api-statuses=14 api-rows=38 pinned-defines=48 cc=AppleClang c-presets=2 fault-runs=134 ... faults=134/134 ... tests-expected=2372 tests-executed=2372 tests-skipped=0 tests-missing=0 ...`; `gate-time total=44`.
- `dash tools/gate.sh`: the same tokens, `gate: PASS`; `gate-time total=42`.
- `sh tools/gate-linux.sh` (ubuntu container, gcc 13.3, cmake 3.28, dash as /bin/sh; not required by the plan, run because the pointer-bearing request and new C are gcc-sensitive): `gate: PASS steps=29/29 controls=252/252 api-calls=14 api-statuses=14 api-rows=38 pinned-defines=48 cc=GNU c-presets=2 fault-runs=134 ... faults=134/134 ... tests-expected=2372 tests-executed=2372 tests-skipped=0 tests-missing=0`, `gate-time total=417` (emulated, laptop loaded; 04-09: 179), `gate-cpu user=1246.53 sys=192.12`; last line `gate-linux: exit 0`. As before it shows `refs-verified=0/4` and `core-imports=3` where macOS shows `1/4` and `4`.

## Deviations from Plan

**1. [Rule 3 - Blocking] Report-after-instance control allocation**
- `run.refuse.report-inside-instance` places a report just past the instance in a host block of instance + 64 bytes; the report grew to 80 bytes (UBSan: insufficient space). The block is now instance + 128. Commit `cd20ee2`.

**2. [Rule 1 - Bug] Collateral of earlier faults**
- `RUN_STOP_BEFORE_OVERSHOOT` (exact-fit and zero-layout expect now >= target) and `RUN_BUDGET_CARRY` (zero-layout runs twice on one instance) fail `run.buffers.*`; `RUN_TARGET_UNCHECKED` fails `run.prec.range-over-capacity` (it reaches BUFFER_TOO_SMALL). Their allowed prefixes were widened with comments; must-fail and must-pass ids unchanged. Commit `5e1e29e`.

**3. Fault site timing**
- The plan's Task 1 gate requires `faults=132/132`, and `faults-verdict` rejects a fault site without an ids line (`fault-orphan-site`), so the `RUN_CAPACITY_AFTER_STEP` site landed in Task 2 with its ids line.

**4. Report overlapping a buffer**
- Not stated in the plan: such a report gets no refusal detail, because writing it would put bytes into a buffer, which the must-have forbids. Header, effect table and `run.buffers.report-overlap` say so.

**5. Request size rule**
- The plan's Pitfall 13 note is followed (offsetof prefix and the cap); the minimum request/report struct_size stays sizeof (it grows with this plan) rather than a frozen v1 size, as plan 04-13 pins the sizes and starts A-02 for them.

## Open Items

- API-04 is marked complete on the contract (caller-owned buffers, refusal before any state change, both entry points, faults). Real bytes arrive with video/audio devices (Phases 6-7); the env seam is where they plug in, and 04-25 re-lists API-04 at close-out. 04-24 can register `output-buffers fault RUN_CAPACITY_AFTER_STEP API-04` in `important-gates.ids`.
- Plan 04-11: the input event checks go in the same D-05 slot after the buffer checks (`ivs_bound_buffers`), and their new reasons start at 44 with pin lines in the same commit; `ivs_run_env` is the natural place for the channel table D-06 mentions. Any run-path fault there should expect `run.slice.` and `run.buffers.` collateral.
- Plan 04-13: pin request 48 bytes (LP64; `offsetof(video) == 16` everywhere) and report 80 bytes, or their grown sizes after 04-11, then apply A-02.
- Carried: `RUN-05` and `API-02` Pending (04-09); 04-14 adds `tools/pack/` to the c-build listed-source loop; the orchestrator records hosted runs and applies D-45 after this plan and after 04-19; PKG-03/PKG-04 Pending; job summary `pending-resolvers=17`, `unknown-fields=16`; `sh tools/hygiene.sh --history` still fails on earlier planning commits.

## Threat Flags

None beyond the plan's threat model: T-04-28 mitigated (overlap with instance, other buffer and report after BAD_HANDLE, uintptr_t with 64-bit lengths; refusal rows with memcmp and poison); T-04-29 mitigated (capacity check before the first unit; `RUN_CAPACITY_AFTER_STEP`); T-04-30 mitigated (`OUTBUF_PARTIAL_WRITE`; poison arena in every refusal case); T-04-SC (no package installs). The env seam is internal (`core/src`, not installed).

## Self-Check: PASSED

- Files present: tests/run/run_buffers.c and the thirteen modified files.
- Commits reachable from HEAD: cd20ee2, 5e1e29e (`git rev-list --count 7f66881..HEAD` = 2).
