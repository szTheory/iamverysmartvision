---
phase: 06-stic-video-first-pass
plan: 03
subsystem: core/include, core/machine (run API), tests/run, tests/installed-consumer, data/expected
tags: [abi-growth, behaviour-neutral, struct-size, report-v3, info-v4, stop-frame, D-13, D-14, D-15, A-02, A-10, A-11, VID-03]
status: complete

requires:
  - phase: 06-stic-video-first-pass
    provides: "plan 06-01 oracle commit eae8ca5; plan 06-02 lever pass (the plan started from 9aab9ce, docs only); gate steps=36/36 controls=328/328 faults=193/193"
provides:
  - "IVS_RUN_F_STOP_ON_FRAME 2u (renamed from IVS_RUN_F_RESERVED_FRAME, still refused as an unknown flag) and IVS_STOP_FRAME 5u (pinned in api-stop-reasons.ids, named by ivs_run_stop_name)"
  - "IVS_OUTPUT_FORMAT_INDEX8_FRAMES_V1 2u, IVS_FRAME_HEADER_BYTES 32u, IVS_FRAME_F_DISPLAYED 1u, IVS_PIXEL_BLANK 0xFFu, IVS_PIXEL_COLOR_MAX 15u and the documented 32-byte big-endian slot header"
  - "ivs_run_report v3 (video_first_index 136, last_frame_end_ticks 144, next_frame_end_ticks 152, sizeof 160, pointer-free) and ivs_info v4 (frame_phase_ticks 288, video_width 296, video_height 300, video_slot_bytes 304, video_header_bytes 308, video_slack_frames 312, video_row_lines 316, sizeof 320), all 0 in this plan; IVS_RUN_REPORT_V1_SIZE 112, IVS_INFO_V1_SIZE 216 and the request (80) unchanged; ivs_run_env still 32 bytes with video_slack_frames in the former reserved word"
  - "public header text for the video capacity rule, the exact frame-event spacing and the domain 16 chain"
  - "named fault RUN_REPORT_V3_TAIL_UNGUARDED and eight new test ids"
affects: [06-04, 06-06, 06-09, 06-10, 06-14]

actuals:
  tokens: 7300
  tasks: 3
  commits: 3
plan_head_before: 9aab9ce9df05f346f5c081897523f11d90182c63
plan_head_after: 33ffa8d6b4f4b8a81dcdb96c9da92b0430032831

tech-stack:
  added: []
  patterns:
    - "A-02 growth rule applied to report v3 and info v4: the local report or info is zeroed and copied min(struct_size, sizeof) bytes last, so a smaller caller never has a byte past its size touched; dedicated v2-sized and v3-sized caller tests with a 0xA5 tail catch what a struct rebuilt from the new header would hide"
    - "One named test constant for 'an arbitrary unsupported flag' (RUN_TEST_BAD_FLAG, bit 4) so reserving or implementing a flag moves one dedicated test, not five generic ones"

key-files:
  created: []
  modified:
    - core/include/ivs/ivs_run.h, core/include/ivs/ivs_machine.h
    - core/src/machine/machine.c, machine.h, machine_bound.c
    - data/api/api-effects.json, data/expected/api-stop-reasons.ids, data/expected/tests/run.ids, data/expected/faults/run.ids, data/expected/pkg-consumer.golden
    - tests/run/run_bound.c, run_input.c, run_buffers.c, run_cases.h, run_struct.c, run_info.c, run_fuzz.c
    - tests/installed-consumer/main.c

key-decisions:
  - "The report tail and the info tail need no new write code: both are built in a zeroed local and copied min(struct_size, sizeof) bytes, so zero tail fields are written exactly when the caller's size covers them; the guard is proven by tests and the new fault, not by new branches"
  - "The new fault copies the full 160 bytes only for a caller of 136 or more (below that, the existing RUN_REPORT_TAIL_UNGUARDED already covers the v1 caller), so it fails exactly the v2-caller case it is named for"
  - "docs/runner.md is unchanged: it quotes no report or info size and no reserved flag, and the plan adds no runner behaviour"

duration: about 3 hours elapsed, most of it three gate runs (sh about 55 s, dash 84 s, container 332 s on a loaded laptop)
completed: 2026-11-12
---

# Phase 6 Plan 03: ABI growth, behaviour-neutral Summary

**The Phase 6 run and info slots exist at pinned offsets with old-size callers proven unaffected: the frame stop reason and flag name, the frame format id and slot constants, report v3 (160 bytes), `ivs_info` v4 (320 bytes), the env slack word, the video capacity rule in the header and one named fault. Nothing a guest or a golden can observe moved: `steps=36/36 controls=328/328 faults=194/194` under `sh`, `dash` and the Ubuntu gcc 13 container.**

Oracle commit this plan follows: `eae8ca5` (06-01 task 5; blind second reads recorded after it in `7f87798`; 06-02 added `-g0` on fault targets). The plan started from `9aab9ce` (docs only).

## What was built

- Task 1 (`5d596f0`, tracer): the flag rename, `IVS_STOP_FRAME` (header define with the exit-state order, `ivs_run_stop_name` case, pin line, effect text, ladder comment), the frame format id and the four constants with the slot layout, `run.stop.frame-name`, and the reviewed test edits below.
- Task 2 (`735dd81`): report v3 and info v4 with their derivation comments and `_Static_assert` lines (every offset and size), the env slack word, six new run-suite ids, the fault `RUN_REPORT_V3_TAIL_UNGUARDED`, the installed consumer's size line and compile-time constant checks, and the one-line golden refresh.
- Task 3 (`33ffa8d`): header text only: the capacity paragraph (`required = video_bytes_per_frame x (floor(D / F) + 1 + video_slack_frames)`, with the derivation that a call can complete `floor(D / F) + 2` frame events), `nominal_frame_ticks` as the exact spacing for a profile that publishes frame events, and domain 16 beside domain 17.

## Reviewed test edits (named, as the plan requires)

Oracle files of 06-01 were not touched. The existing run-suite edits:

| Edit | Where | Change |
|------|-------|--------|
| 1-3 | `tests/run/run_bound.c` (state-destroyed, state-off precedence and `run.prec.unsupported-over-state` sites) | the generic "unsupported flag" moved from `IVS_RUN_F_RESERVED_FRAME` to `RUN_TEST_BAD_FLAG` (bit 4) |
| 4 | `tests/run/run_input.c` (`ri_prec_unsupported_over_events`) | same |
| 5 | `tests/run/run_buffers.c` (flags refusal) | same |
| 6 | `run.refuse.flags-frame-reserved` (`run_bound.c`) | id and refused values (2 and 2 or 1) unchanged; the symbol is now `IVS_RUN_F_STOP_ON_FRAME` and the comment names the plan (06-09 inverts it) |
| 7 | `run.stop-name.known` and `run.stop-name.unknown` (`run_bound.c`) | known gains the fifth name and constant 5; unknown's value list moves 5 to 6 |
| extra a | `tests/run/run_fuzz.c` `run.struct.v1-sizes` | pins 136 to the v2 end (offset of `video_first_index`) plus `sizeof` 160, and 288 to the v3 end plus `sizeof` 320 |
| extra b | `tests/run/run_info.c` `run.info.v3-fields` | `sizeof(ivs_info)` and the full caller's `struct_size` 288 to 320 (the v3-sized caller is the new `run.info.v3-size-caller`); `run.info.layout-real` also pins the v4 tail 0 |
| extra c | `tests/run/run_struct.c` `run.struct.report-v2-fields` | `sizeof` 136 to 160, the v2 end pinned through the offset of `video_first_index`, full caller `struct_size` 160 |

`run.struct.v1-sizes-lp64` needed no edit: the request stays 80 bytes.

## New ids and fault

`run.stop.frame-name`, `run.struct.report-v3-offsets`, `run.struct.report-v2-caller`, `run.struct.report-v3-caller`, `run.struct.env-size`, `run.info.v4-fields`, `run.info.v3-size-caller` (7 new, run suite 169 to 176 cases; 3778 to 3792 inventory in both presets, 3790 executed, 2 skipped). Fault `RUN_REPORT_V3_TAIL_UNGUARDED` (one site in `machine_bound.c`, must fail `run.struct.report-v2-caller`; the verdict first showed three collateral names, now allowed: `run.struct.report-v3-caller` and `run.struct.report-v2-caller` under the existing `RUN_REPORT_TAIL_UNGUARDED`, `run.info.v3-size-caller` under `INFO_TAIL_UNGUARDED`; the new fault itself needed no extra prefix beyond its own family).

## Installed consumer and golden

`tests/installed-consumer/main.c` pins `sizeof(ivs_run_report) == 160`, `sizeof(ivs_info) == 320`, the offsets 136 and 288, the request 80, and three file-scope `_Static_assert` lines on `IVS_STOP_FRAME`, `IVS_OUTPUT_FORMAT_INDEX8_FRAMES_V1` and `IVS_FRAME_HEADER_BYTES` from the installed headers. `data/expected/pkg-consumer.golden` was refreshed from `build/gate/pkg-consumer.txt` (the documented refresh, comment header kept) in the Task 2 commit; the whole diff against the plan start is one line:

```
-v1-sizes ... run_report_v1=112 run_report=136 info_v1=216 info=288 input_event=32 cpu_state_report=36 ok=yes
+v1-sizes ... run_report_v1=112 run_report=160 info_v1=216 info=320 input_event=32 cpu_state_report=36 ok=yes
```

The consumer still requests no video buffer and prints `video_format=0`; its `digest.*` lines and observed-domain set equal the runner golden as before. `git diff 9aab9ce..HEAD -- data/golden data/scenarios` is empty.

## Deviations from Plan

**1. [Rule 3 - Blocking] Three more pre-existing size pins moved.** `run_fuzz.c` (`run.struct.v1-sizes`), `run_info.c` (`run.info.v3-fields`) and `run_struct.c` (`run.struct.report-v2-fields`) hard-coded 136 and 288, so the suite failed the moment the structs grew; the plan lists only the first as "updated" and the files `run_fuzz.c` and `run_info.c` are not in its `files_modified` (run_info is, run_fuzz is not). Edited in the Task 2 commit; the table above names them.

**2. [Rule 3 - Blocking] `RUN_TEST_BAD_FLAG` lives in `tests/run/run_cases.h`** (already in `files_modified`), one definition for the three test files.

**3. [Rule 1 - tooling hazard] A `]` in header comment text broke the api-effects checker.** The first draft of the capacity paragraph wrote the interval as `(now, target + U - 1]`; `tools/check/api-effects.cmake` reads headers with `file(STRINGS)` into a CMake list, where an unbalanced bracket merges lines and made `ivs_machine_run` and `ivs_run_stop_name` look undeclared (`api-call-undeclared`). The text was reworded without brackets before the commit; no header line in the repository now holds an unbalanced `[` or `]` from this plan. Worth knowing for any later header prose.

**4. Task 3 `docs/runner.md`: no change.** It quotes no sizes and no reserved flag; the plan says to change it only if it does.

**5. The plan commit ledger file was written after the Task 1 commit** (the executor's setup order), with the value `9aab9ce`, the HEAD the plan started from; `plan_head_before` and the commit count above are measured from it (3).

## Decisions Made

- `run.stop.frame-name` sits in the bound family beside the stop tests (the plan names no file); the plan's other suites (struct, info) took their ids as named.
- `video_slack_frames` in `ivs_info` v4 is copied from the env seam (0 for every profile now), so the field and the capacity rule cannot drift when plan 06-09 sets the env word.
- No requirement is closed: VID-03 stays Pending (ABI only; plans 06-04 to 06-14 deliver the framebuffer and its evidence).

## Open Items

- `run.refuse.flags-frame-reserved` keeps refusing value 2; plan 06-09 inverts it as a reviewed change when the frame stop is implemented. Until then the comment above the ladder loop still reads "INTERNAL, GUEST_FAULT, CPU_HALTED (flag), TARGET" (the FRAME step joins it in 06-09; the header text already states the five-step order).
- Every v3 and v4 field is 0 and documented as such; plans 06-04 (profile fields into info), 06-06 and 06-09 must replace the "0" assertions in `run.info.v4-fields`, `run.info.layout-real` and `run.struct.report-v3-caller` with literals (the case comments say so).
- Domain 16's digest bytes (what `ivs_machine_state_digest(VIDEO)` hashes) are not yet defined beyond the chain recipe in the header text; plan 06-10 (the video observation row) defines and tests it.
- Carried from earlier plans: pending items listed in the 06-01 and 06-02 summaries are unchanged; the per-phase CI checkpoints CP-A..CP-D are due in later plans; `sh tools/hygiene.sh --history` still fails on earlier planning commits.

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=36/36 controls=328/328 ... faults=194/194 ... tests-missing=0   pinned-defines=96   (gate-time total about 52 on a loaded laptop)
dash tools/gate.sh:    the same summary line (identical bytes), 0 FAIL lines                            (gate-time total=84)
sh tools/gate-linux.sh: exit 0, last line of build/gate-linux.log starts gate: PASS, cc=GNU (gcc 13), same counts (gate-time total=332)
```

`pinned-defines` is 96 (95 at the plan start, plus the `IVS_STOP_FRAME` pin line; the new frame constants are not a pinned define class).

## Known Stubs

None. The new fields are intentionally 0 until the plans named above write them; they are documented as such in the headers.

## Threat Flags

None new. T-06-04 (report v3 and info v4 tails written past a smaller caller's struct_size) is mitigated: byte copy of `min(struct_size, sizeof)` from a zeroed local, `run.struct.report-v2-caller`, `run.struct.report-v3-caller`, `run.info.v3-size-caller`, and the fault `RUN_REPORT_V3_TAIL_UNGUARDED` (must fail the v2-caller case). T-06-SC: no installs.

## Self-Check: PASSED

- Files exist: `core/include/ivs/ivs_run.h` (contains `IVS_STOP_FRAME`), `core/include/ivs/ivs_machine.h` (contains `IVS_OUTPUT_FORMAT_INDEX8_FRAMES_V1`), `data/expected/api-stop-reasons.ids` (contains `IVS_STOP_FRAME 5`), `tests/run/run_struct.c`.
- `grep -rn RESERVED_FRAME core tests data/api` prints nothing; `grep -c IVS_FAULT_RUN_REPORT_V3_TAIL_UNGUARDED core/src/machine/machine_bound.c` is 1; `grep -c '^RUN_REPORT_V3_TAIL_UNGUARDED' data/expected/faults/run.ids` is 1.
- Commits on HEAD: `5d596f0`, `735dd81`, `33ffa8d`.
