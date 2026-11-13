---
phase: 06-stic-video-first-pass
plan: 09
subsystem: core/stic, core/machine (run API), runner, tests/video, tests/run, tests/installed-consumer, tools/check
tags: [raw-framebuffer, video-slot, domain-16-chain, capacity-slack, frame-stop, report-v3-live, info-v4-live, runner-sizing, consumer-buffer, symbol-layer, checkpoint-B, D-12, D-13, D-14, D-15, D-16, D-17, A-02, A-03, A-06, A-08, D-21, VID-03]
status: complete

requires:
  - phase: 06-stic-video-first-pass
    provides: "plan 06-03 ABI growth (report v3, ivs_info v4, env slack word, frame constants); plan 06-04 stic parameters; plans 06-05 and 06-06 state, event table and catch-up; plan 06-08 lines, row latches and bounds; gate steps=36/36 controls=331/331 faults=208/208"
provides:
  - "ivs_psg_sink video channel (video, video_cap, video_written, video_frames, video_overflow; zero is the counting sink) and ivs_psg_sink_video; ivs_psg_sink_init is unchanged"
  - "core/src/stic/stic_frame.c: ivs_stic_geom_derive (pure, from the primary inputs), ivs_stic_render_card_row and ivs_stic_render_plane (display-off border fill, interim blank for display on), ivs_stic_frame_deliver (32-byte header byte by byte, rows cropped to 159, the per-frame SHA-256 chain), ivs_video_chain_init (h0 at create and power-on reset), ivs_stic_next_frame_end"
  - "ivs_stic_catch_up takes the VIDEO row and the sink; FRAME_END completes the frame at the event (A-06); the machine seam and the bus wrapper pass both"
  - "layout real: INDEX8_FRAMES_V1, slot 15,296, slack 1, the whole ivs_info v4 tail; capacity slot x (floor(D / F) + 1 + slack) with refusal before any state change; a flagged call needs one slot; never-expected pre-unit INTERNAL guard"
  - "IVS_RUN_F_STOP_ON_FRAME accepted: T' = min(target, E_next) from the STIC state, IVS_STOP_FRAME in the exit-state order, report v3 live (video_first_index, last_frame_end_ticks, next_frame_end_ticks, video_frames_completed, video_bytes_written)"
  - "runner ex_plan_buffers sizes video with the slack from ivs_info v4; the installed consumer requests a video buffer from ivs_info"
  - "suite video (54 ids), run ids +6 -1, machine id +1, runner id +1; six video faults; control core-symbols-palette-defined"
affects: [06-10, 06-11, 06-12, 06-13, 06-14]

actuals:
  tokens: 37700
  tasks: 5
  commits: 5
plan_head_before: f6d95ce7f3ad055efdb7d4632a3de8e06890cc0f
plan_head_after: a303622b2e59719a1b9681179f4fa7cd252b3aad

key-decisions:
  - "The core renders one card row at a time (at most 1,280 bytes of stack), never a 15,360-byte working plane: the platform stack probe (__chkstk_darwin on Apple clang) is a forbidden import of the symbol layer. ivs_stic_render_plane stays as a test and review entry point that fills a caller-owned buffer by calling the card-row function"
  - "ivs_stic_catch_up gained a trailing VIDEO row parameter next to the sink; NULL (the module-level tests) delivers nothing, the machine always passes both, so the chain never depends on a buffer"
  - "A flagged call's video requirement is exactly one slot, computed by the same ivs_bound_fits call with target = now and no slack, so a flagged run with IVS_RUN_TARGET_MAX needs one slot and not an astronomical capacity"
  - "The pre-unit guard fires only when a frame could complete in the unit (E_next within psg.max_unit_ticks of the unit start) and the requested buffer cannot hold one more slot"
  - "The border color register address (0x2C) is a file constant of stic_frame.c, as the plan states; the geometry comes from the generated parameters"

key-files:
  created:
    - core/src/stic/stic_frame.c
    - cmake/IvsVideoLiterals.cmake
    - tests/video/video_main.c, video_cases.h, video_progs.c, video_geom.c, video_slot.c, video_buffers.c, video_stop.c, video_slice.c
    - data/expected/tests/video.ids, data/expected/faults/video.ids
    - tools/check/controls/core-symbols-palette-defined/
  modified:
    - core/include/ivs/ivs_run.h
    - core/src/psg/psg.h, psg_state.c, core/src/stic/stic.h, stic_events.c, core/src/machine/machine.h, machine.c, machine_run.c, machine_bus.c, machine_bound.c, core/CMakeLists.txt
    - runner/exec.c, docs/runner.md
    - tests/CMakeLists.txt, tests/machine/machine_digest.c, tests/stic/stic_events.c, stic_bounds.c, tests/run/run_info.c, run_buffers.c, run_struct.c, run_input.c, run_bound.c, tests/runner/runner_exec.c, tests/installed-consumer/main.c
    - data/api/api-effects.json, data/expected/tests/machine.ids, run.ids, runner.ids, data/expected/faults/machine.ids, run.ids, data/expected/pkg-consumer.golden, data/expected/controls/core-symbols.ids, data/stic/tests-pending.ids
    - tools/check/core-symbols.sh, core-symbols-deny.txt
    - .planning/CI-EFFICIENCY.md

duration: about 6 hours elapsed on a heavily loaded laptop (1-minute load 55 to 120 for the checkpoint runs)
completed: 2026-11-12
---

# Phase 6 Plan 09: raw framebuffer, video buffers, domain 16, frame stop, runner and consumer sizing Summary

**The core now produces a raw palette-index frame at every frame event into a caller-owned buffer: a 32-byte big-endian header and 96 rows of 159 index bytes (slot 15,296), chained per frame into digest domain 16 (h0 at create and reset), with the capacity law slot x (floor(D / F) + 1 + 1), the frame-stop flag with its `IVS_STOP_FRAME` reason and report v3, and the runner and the installed consumer sizing their video buffers from `ivs_info`. Every existing golden scenario ends before its first frame end, so no golden moved.**

Oracle commit this plan follows: `eae8ca5` (06-01 task 5). First commit of the plan (FIRST): `598c4ba`. The 06-01 oracle files (geometry and chain-framing literals) were read, never edited: the suite reads them through `cmake/IvsVideoLiterals.cmake` only.

## What was built

- Task 1, tracer (`598c4ba`): the video channel of the sink, `stic_frame.c`, the FRAME_END hook, h0 at create and reset, suite `video` (geom and slot families, 17 ids), the machine id `machine.digest.video-h0-create-and-reset`, faults `VIDEO_STATE_DEPENDS_ON_BUFFER`, `VIDEO_CHAIN_FRAMING_STRUCT_HASH`, `STIC_EVENT_ORDER_START_BEFORE_END`. The first display-off frames equal the shell-derived literal: the headers, both slot digests, and the chain steps h0, h1, h2.
- Task 2 (`dfe503d`): layout real (`ivs_machine_profile_env`, `ivs_machine_info` v4 tail), the capacity law with slack, the video sink in the run loop, the pre-unit guard, buffer family (10 ids), `run.buffers.video-real`, `runner.exec.buffers-video-slack`, runner sizing, consumer request, faults `VIDEO_CAP_PLUS_ONE`, `VIDEO_OUTBUF_PARTIAL_WRITE`.
- Task 3 (`ed63ab9`): the frame stop (flag, T', reason, report v3), the inverted Phase 4 test, stop (6), boundary (4), slice (7), fuzz (8) and iso (1) families, fault `VIDEO_FRAME_AT_CALL_END`.
- Task 4 (`5096044`): the second defined-symbol rule (`palette|rgb|ppm|framepreview`, token `core-symbols-palette-defined`), the deny-list classes, the control, `docs/runner.md`.
- Task 5 (`a303622`): checkpoint B in `.planning/CI-EFFICIENCY.md`, then the three gates below.

## Reviewed test edits (named, as the plan requires)

| Edit | Where | Change |
|------|-------|--------|
| 1 | `tests/run/run_bound.c`: `run.refuse.flags-frame-reserved` | **inverted**: the id is removed from `data/expected/tests/run.ids` and `data/api/api-effects.json`; the positive ids `run.flags.stop-on-frame-accepted`, `.stop-on-frame-clamps-target`, `.stop-reason-frame`, `.report-v3-fields` replace it (the other bad-flag sites use bit 4 and are unchanged, `run.refuse.flags-unknown` still refuses an unknown bit) |
| 2 | `run.info.layout-real`, `run.info.v4-fields` (`tests/run/run_info.c`) | video format 2, bytes per frame 15,296 and the v4 tail 159, 96, 15,296, 32, slack 1, row lines 2, phase 0 (literals from the geometry page) instead of NONE and 0 |
| 3 | `run.struct.env-size` (`run_struct.c`) | the profile env's `video_slack_frames` is 1, not 0 |
| 4 | `run.struct.report-v3-caller` (`run_struct.c`) | `next_frame_end_ticks` is 59,736 (live), the first index and last frame end stay 0 |
| 5 | `run.buffers.real-layout-audio` (`run_buffers.c`) | video is not requested in all three variants (a 64-byte video buffer is now too small); frames are counted whether or not a buffer is given, so the third 20,000-tick call (which crosses 59,736) counts the frame it completes and delivers 0 bytes |
| 6 | `run.input.refuse.events-overlap-video` (`run_input.c`) | the adjacent positive control uses the real requirement of a run to tick 1,000, two slots of 15,296 bytes; the buffer starts 8 bytes in so the event array stays 8-aligned |
| 7 | `machine.reset.clears-observation` (`machine_digest.c`) | the fresh and reset observation rows are zero except the input chain AND the video chain, both h0 |
| 8 | `tests/stic/stic_events.c`, `stic_bounds.c` | the 21 `ivs_stic_catch_up` call sites pass `NULL, NULL` for the sink and the row (signature change only) |

## Installed consumer and golden

`run_step` requests a video buffer of `video_bytes_per_frame x (slots + video_slack_frames)` from `ivs_info` and frees it after the call; the consumer's `delivered_video_bytes` stays 0 (every replay ends before tick 59,736), so video is NOT observed. The whole diff of `data/expected/pkg-consumer.golden` against the plan start is one line (regenerated from `build/gate/pkg-consumer.txt` behind its preserved `#` header, never by hand):

```
-info phase=1 time_ticks=0 ticks_per_cpu_cycle=4 nominal_frame_ticks=59736 video_format=0 audio_format=1
+info phase=1 time_ticks=0 ticks_per_cpu_cycle=4 nominal_frame_ticks=59736 video_format=2 audio_format=1
```

## Golden classification

None moved. `git diff --stat 598c4ba^..HEAD -- data/golden data/scenarios` prints nothing, and `git status --short data/golden data/scenarios` is empty after every gate. Why: no gate scenario reaches its first frame end (the longest, `diag-ctrl-psg-pass`, ends at 16,752 ticks, the first frame ends at 59,736), so `output.video.bytes` is 0 and video is `none` in every `# domains:` header; the h0 initialization changes the VIDEO row, which is not in the composite, and no `.digests` file prints `digest.video`. The consumer and the runner golden show the same observed set (video not observed, audio and psglog as before) and equal `digest.*` lines; `pkg-install` compares them at every gate.

## Tests and faults

| Fault | Site | Must fail |
|-------|------|-----------|
| VIDEO_STATE_DEPENDS_ON_BUFFER | stic_frame.c | video.noop.null-buffer |
| VIDEO_CHAIN_FRAMING_STRUCT_HASH | stic_frame.c | video.chain.framing-oracle |
| STIC_EVENT_ORDER_START_BEFORE_END | stic_events.c | video.slot.displayed-flag-follows-latch |
| VIDEO_CAP_PLUS_ONE | machine_bound.c | video.buffers.worst-case-two-frames |
| VIDEO_OUTBUF_PARTIAL_WRITE | machine_bound.c | video.buffers.undersized-refused |
| VIDEO_FRAME_AT_CALL_END | machine_bound.c | video.boundary.write-straddling |

Collateral prefixes were extended where the gate required: six existing `run` faults (`RUN_STOP_BEFORE_OVERSHOOT`, `RUN_FAULT_STEPS`, `RUN_CAPACITY_AFTER_STEP`, `OUTBUF_PARTIAL_WRITE`, `RUN_REPORT_TAIL_UNGUARDED`, `RUN_REPORT_V3_TAIL_UNGUARDED`) now also fail the new `run.flags.*` or `run.struct.report-v3-live` ids, `OBS_NOT_CLEARED_ON_RESET` also fails the new machine id, and `VIDEO_STATE_DEPENDS_ON_BUFFER` also fails every video id that runs through the public call without a buffer. Pending lines deleted with the first registered id of each family: `video.geom.`, `.layout.`, `.slot.`, `.frame.`, `.noop.`, `.chain.` (task 1), `.buffers.` (task 2), `.boundary.`, `.stop.`, `.slice.`, `.fuzz.`, `.iso.` and `run.flags.` (task 3).

Suite `video`: 54 ids (geom 5, slot 13, buffers 10, stop 6, boundary 4, slice 7, fuzz 8, iso 1), plain-run time dev 0.26 s (target 0.8 s) on a quiet laptop, release 0.02 s.

## Checkpoint B (cost)

`checkpointB: no-lever`, recorded in `.planning/CI-EFFICIENCY.md` (section `Phase 6 checkpoint B (plan 06-09)`). The normalised metric `r = cpu_s / build_s` of the last five successful hosted rows is 7.49, 7.97, 7.98, 7.88, 7.90, median **7.90** against the budget 7.8 and the revisit trigger 8.3: not tripped, so **no lever pass is required before plan 06-10** by the rule in force. The margin is thin: the projection formula of the 06-02 entry gives 280.4 cpu-s (on the raw trigger of 280, 19.6 under the budget of 300) and this plan alone is expected to move `r` to about 8.0 (7.9 to 8.2). Cumulative adds since 06-02: faults 193 to 214, controls 328 to 332, ids 3778 to 4210. Local gates: 90 and 107 s (242 and 252 cpu-s) on the quieter runs at the task commits; the three final runs read 157, 212 and 168 s at a load of 55 to 120 (the machine, not the code). One 71-slot slice of video SHA-256 is 9.1 ms at `-O2` (laptop, loaded). The orchestrator should read `r` from the first hosted row of this plan: at or above 8.3 starts the lever queue before 06-10.

## Deviations from Plan

**1. [Rule 3 - blocking, found by the gate] No 15 KB plane on the stack.** The plan puts the 160 x 96 working plane on the stack; on Apple clang a frame that size calls `__chkstk_darwin`, a forbidden import of the symbol layer (`core-symbols-forbidden`, c-purity and the shipped archive). The renderer works one card row at a time (`ivs_stic_render_card_row`, at most 1,280 bytes), the slot writer streams rows to the hash and the buffer, and `ivs_stic_render_plane` remains as a caller-buffer entry point for tests and the card decoder's review. The chain and the delivered bytes are identical to the plane form (the literal oracle passes). The card decoder of plan 06-10 renders per card row, which is also the natural unit for the color-stack pointer.

**2. [Rule 3] Signature change of `ivs_stic_catch_up`** (a trailing `ivs_stic_video_row *row`), 21 test call sites in two stic test files edited (listed in the reviewed-edit table); the typedef `ivs_stic_sink` is now `ivs_psg_sink`.

**3. [Interpretation] `ivs_machine_profile_env` keeps its signature** and finds the STIC parameter row by scanning the registry for the profile pointer (the three external callers are unchanged).

**4. [Interpretation] Named-fault site of `VIDEO_FRAME_AT_CALL_END` is in `machine_bound.c`** (after the loop it re-renders every display-off slot the call delivered from the registers at call end), not in `stic_frame.c`; the plan allows "or its caller". `VIDEO_OUTBUF_PARTIAL_WRITE` is a second site beside the Phase 4 `OUTBUF_PARTIAL_WRITE` so each fault has its own registration.

**5. [Interpretation] The constructed capacity corner is found by observation, not placed at exactly `E_k - 4`.** `video.buffers.worst-case-two-frames` single-steps whole units until the machine time is within 24 ticks below an event (the corner needs at most 24 ticks with the 36-tick unit of the loop guest; frames 1 to 4 are tried), then asserts one slot refused and two slots accepted and filled with two frames. The INTERNAL stop of `video.stop.frame-reason-order` is reached through the env seam with a frame declared four times too long (requirement one slot, two frames completing), the same guard the capacity law makes unreachable for the real layout.

**6. [Interpretation] `video.chain.recompute-from-slots` uses the core's one-shot SHA-256 over a concatenation built in the test** (a different API path from the incremental one the delivery uses; the core's SHA-256 has its own known-answer suite); the independent oracle is the shell-derived literal (`framing-oracle`), not a second SHA implementation.

**7. [Rule 3] Files outside the plan's list were edited:** `tests/stic/stic_events.c`, `tests/stic/stic_bounds.c` (signature), `tests/run/run_input.c` (overlap sizes), `data/expected/faults/machine.ids` and `run.ids` (collateral prefixes), `docs/runner.md` (task 4 names it), `data/api/api-effects.json` text, `tests/video/video_slice.c` (the plan names slice, fuzz and iso tests without a file; they live in one extra file).

## Decisions Made

- No requirement is closed: VID-03 stays Pending in `REQUIREMENTS.md` (plans 06-10 to 06-14 list it, and card rendering, the runner keys and the end-to-end evidence are still to come).
- The consumer prints `video_format=2` and requests the buffer but delivers nothing; the observed-domain rule is count-based, so the agreement with the runner golden holds without a golden move.

## Open Items

- `video.frame.display-on-not-decoded` pins the interim display-on frame (every position 0xFF); plan 06-10 inverts it as a reviewed change when the card decoder lands.
- Domain 16's digest bytes are the chain and the count rows (as before); the value-level definition and the video observation row of the runner (`video.chain.check`, `ivsr 3`) belong to plans 06-10 and 06-12.
- `ivs_stic_render_plane` has no user in the core (test and review entry point only).
- The three checkpoint gate runs were taken under a heavy laptop load (see the table in `.planning/CI-EFFICIENCY.md`); `r` from the hosted rows is the arbiter.
- Carried unchanged: the 06-01 second-read records, the Phase 4 and 5 deferrals, `sh tools/hygiene.sh --history` on earlier planning commits.
- The container gate ran with `GATE_LINUX_PLATFORM=` (the platform string gate-linux printed is x86_64 gcc 13.3.0); hosted CI is the remaining runner-speed arbiter.

## Final gate

```
sh tools/gate.sh:       gate: PASS steps=36/36 controls=332/332 ... api-calls=15 ... faults=214/214 ... profile-keys=143 ... tests-expected=4210 tests-executed=4208 tests-skipped=2 tests-missing=0
dash tools/gate.sh:     the same summary line (identical md5 471510524a57fc1c203ba013492bca35)
sh tools/gate-linux.sh: GATE_LINUX_PLATFORM= run, exit 0, last line of build/gate-linux.log starts gate: PASS, gate-linux reports x86_64 gcc 13.3.0 cmake 3.28.3, steps=36/36 controls=332/332 faults=214/214 tests-missing=0, 0 warnings   (gate-time total=715 under a laptop load of 55 to 120)
```

## Known Stubs

The display-on frame is every position 0xFF until plan 06-10 (documented in `stic_frame.c`, pinned by `video.frame.display-on-not-decoded`); no other stub.

## Threat Flags

None new. T-06-12 (aliased or partially written buffer) is mitigated: the overlap stage covers video, the capacity is checked before the first unit, whole slots only, `video.buffers.overlap-*`, `video.buffers.sentinel`, fault `VIDEO_OUTBUF_PARTIAL_WRITE`. T-06-13 (astronomical capacity, the `+1` corner) is mitigated: a flagged call needs one slot, the 64-bit guard test, `VIDEO_CAP_PLUS_ONE`, the INTERNAL guard test. T-06-SC: no installs.

## Self-Check: PASSED

Created files exist (`core/src/stic/stic_frame.c`, `cmake/IvsVideoLiterals.cmake`, the eight `tests/video` files, `data/expected/tests/video.ids`, `data/expected/faults/video.ids`, `tools/check/controls/core-symbols-palette-defined/`); commits `598c4ba`, `dfe503d`, `ed63ab9`, `5096044`, `a303622` are ancestors of HEAD; `grep -c video_slack_frames runner/exec.c` is at least 1; `grep -c 'run.refuse.flags-frame-reserved' data/expected/tests/run.ids data/api/api-effects.json` is 0 for both.
