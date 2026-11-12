---
phase: 05-hand-controllers-and-psg-sound
plan: 02
subsystem: core/machine (run API), core/include, tests/run, tools/check, data/expected
tags: [abi-growth, behaviour-neutral, struct-size, v1-minimum, psg-log, refusal-ladder, D-13, D-14, D-17, D-19, A-04, A-11, API-03, AUD-02]
status: complete

requires:
  - phase: 05-hand-controllers-and-psg-sound
    provides: "plan 05-01 oracle commit 14a1904 (the last of its five commits; blind second reads recorded after it in 95a0f87 and b82c707, the HEAD this plan started from)"
provides:
  - "ivs_run_request v2 (psg_log, psg_log_capacity at 64 and 72, sizeof 80) and ivs_run_report v2 (audio_first_index, psg_log_bytes_written, psg_log_first_index at 112, 120, 128, sizeof 136) behind the pinned minimums IVS_RUN_REQUEST_V1_SIZE 64 and IVS_RUN_REPORT_V1_SIZE 112"
  - "the third buffer in stages 4b and 4c order video, audio, psg_log with reasons RUN_PSGLOG_NULL 55, RUN_PSGLOG_OVERLAP 56, RUN_PSGLOG_SMALL 57; the NULL-plus-0 not-requested rule (D-17)"
  - "ivs_info v3 (sizeof 288, tail at 256..288, all fields 0 until plans 05-06 and 05-08), IVS_INFO_V1_SIZE public and 216, IVS_OUTPUT_FORMAT_PSG_LEVELS_V1 1u"
  - "IVS_PSGLOG_SRC_CPU_WRITE 1 and CPU_DW 2 pinned by data/expected/psg-log-source.ids and registered as a define class with 2 controls"
  - "ivs_run_env.psg_log_bytes_per_frame (internal seam, 0 for every profile until plan 05-08)"
affects: [05-03, 05-05, 05-06, 05-08, 05-09, 05-11]

actuals:
  tokens: 31000
  tasks: 4
  commits: 4
plan_head_before: b82c7073cef4624204886adb7026a082281c0e73
plan_head_after: 9be491d817d917fb4206cf0c93981470f0760f2e

tech-stack:
  added: []
  patterns:
    - "A-02 growth rule applied to the request and report: a pinned v1 minimum constant, fields read or written only as far as struct_size covers, a byte copy of min(caller size, own size), refusal writes inside the v1 prefix"
    - "The span a report occupies in every overlap test is the bytes the core would write, not sizeof, so a v1-sized report in a tight layout is not falsely overlapped"
    - "Pinned define class registered in DEFINE_CLASSES with a toy header in the known-good control tree (the run header stays the one that other controls overlay)"

key-files:
  created:
    - tests/run/run_struct.c
    - data/expected/psg-log-source.ids
    - tools/check/controls/api-psglog-src-orphan/
    - tools/check/controls/api-psglog-src-unpinned/
    - tools/check/controls/_good/tree/core/include/ivs/ivs_machine.h
    - tools/check/controls/_good/tree/data/expected/psg-log-source.ids
  modified:
    - core/include/ivs/ivs_run.h, core/include/ivs/ivs_machine.h
    - core/src/machine/machine.h, machine.c, machine_bound.c
    - data/api/api-effects.json, data/expected/api-reasons.ids, data/expected/tests/run.ids, data/expected/faults/run.ids
    - data/expected/pkg-consumer.golden, tests/installed-consumer/main.c
    - tests/run/run_buffers.c, run_input.c, run_info.c, run_fuzz.c, run_bound.c, run_cases.h, run_main.c, tests/CMakeLists.txt
    - tools/check/api-effects.cmake, data/expected/controls/api-effects.ids

key-decisions:
  - "The minimum accepted struct_size is the pinned constant, never sizeof of the grown struct; the nonzero-tail rule is rebased on the core's own size, so a caller between v1 and v2 is valid and its partial appended bytes are not read"
  - "The report span in every overlap test is min(struct_size, 136) bytes: a v1-sized (112 byte) report with the log right after it is accepted, one byte lower is refused"
  - "psg_log versus the event array is decided in stage 4b (RUN_PSGLOG_OVERLAP), not by growing the events stage list, because stage 4b runs first and the events reason would then be unreachable"
  - "NULL with capacity 0 needs nothing whatever the layout says; a non-NULL buffer with capacity 0 against a layout that needs bytes is still BUFFER_TOO_SMALL (Phase 4 behaviour kept for non-NULL)"
  - "IVS_INFO_V1_SIZE moved from the internal machine.h to the public ivs_machine.h so the installed consumer can pin it from the installed headers"

duration: about 35 min of edits plus about a dozen gate runs (first commit 02:13, last 02:27)
completed: 2026-11-12
---

# Phase 5 Plan 02: behaviour-neutral ABI growth Summary

**The public run API now has its Phase 5 slots at pinned offsets with v1-sized callers proven unaffected: request v2 and report v2 behind pinned v1 minimums, the write-log buffer as a third channel in the refusal ladder (reasons 55..57), the NULL-plus-0 not-requested rule, `ivs_info` v3, and the log-source define class. No digest, golden or report changed; `steps=35/35 controls=296/296 faults=152/152` under `sh`, `dash` and the Ubuntu gcc 13 container.**

Oracle commit this plan follows: `14a1904` (05-01 task 5); the plan started from `b82c707` (the recorded blind second reads, docs only).

## What was built

- Task 1 (`9b31c5b`, tracer): `IVS_RUN_REQUEST_V1_SIZE 64u`, `IVS_RUN_REPORT_V1_SIZE 112u`, the v2 fields and static asserts; stage 1 and the report-ok test use the constants; the report is written as a byte copy of `min(rep->struct_size, 136)` with `struct_size` set to that count; `psg_log` and its capacity are read only when `struct_size >= 80`; `tests/run/run_struct.c` with five cases; the `run.struct.v1-sizes` pins moved to the constants.
- Task 2 (`6bda898`): stage 4b gains psg_log after video and audio (NULL, overlap with instance, report, video, audio, event array, top of the address space), stage 7 names `RUN_PSGLOG_SMALL`; `ivs_bound_fits` takes the buffer pointer (D-17); reasons 55..57 in the header and `api-reasons.ids`; `IVS_OUTPUT_FORMAT_PSG_LEVELS_V1`; `api-effects.json` rows; six cases.
- Task 3 (`47661b2`): `ivs_info` v3, `IVS_PSGLOG_SRC_*` defines, pin file, `DEFINE_CLASSES` entry, two controls, two cases.
- Task 4 (`9be491d`): the three named faults and their inventory lines; the three-gate close.

## Changed Phase 4 expectations (named, as the plan requires)

| Case | Old expectation | New expectation |
|------|-----------------|-----------------|
| `run.prec.capacity-over-event-order` (run_input.c) | first call: NULL video buffer with capacity 0 under a 16 bytes per frame env, unsorted events: `BUFFER_TOO_SMALL` `RUN_BUFFER_SMALL` | that pair now means "channel not requested" (D-17), so the call gives a nonzero capacity of 31 (one byte below the 32 needed) and expects the same `BUFFER_TOO_SMALL`; the second call (capacity 32 gives `INPUT_UNSORTED` at index 1) is unchanged |

No other case relied on NULL with capacity 0 against a nonzero layout (the full `run` and `runner` suites were run after the rule landed; this was the only failure). Harness sizes only (no expectation changed): `run_bound.c` `RB_REP_BYTES` 128 to 192 and its trailing allocation 128 to 256; `run_buffers.c` `BF_ARENA` 256 to 512 and `BF_REP_BYTES` 128 to 192 (reports are now 136 bytes, the old buffers were too small); `run.struct.v1-sizes` and `-lp64` pins now read the constants (report 112 as `IVS_RUN_REPORT_V1_SIZE` with `sizeof` 136, request 64 as `IVS_RUN_REQUEST_V1_SIZE` with `sizeof` 80, info `sizeof` 288). The `ivs-run` runner keeps working: it never sets `host.buffers`, the real profile's layout is still 0 bytes, and the `runner`, `runner-process` and `pkg-install` steps are green (A-11 holds through the new rule).

## `sizeof` of the request and report in `machine_bound.c`

`grep -n 'sizeof(ivs_run_request)\|sizeof(ivs_run_report)'` lists: the struct-cap and size static asserts (lines 73, 77, 83), `ivs_bound_rep_bytes` (the own-size cap of the byte count, line 219) and the tail scan start (line 331, the rebased own size). None is a minimum check; the minimums are the two constants (stage 1 and `ivs_bound_req_sized`, `ivs_bound_report_ok`).

## Counts and times

- Start: `steps=35/35 controls=294/294 faults=149/149 tests-expected=2818 tests-missing=0` (the plan said 2816 as in 05-01; the baseline is 2818).
- End: `steps=35/35 controls=296/296 faults=152/152 tests-expected=2844 tests-executed=2842 tests-skipped=2 tests-missing=0`, `pinned-defines=95` (90 + 3 reasons + 2 sources). The run suite gained 13 ids (5 struct, 6 buffers and prec, 2 info; 135 to 148 cases), which the inventory counts in both presets: 2818 + 26 = 2844.
- `data/golden`: `git diff --stat b82c707 -- data/golden` is empty (goldens unchanged byte for byte).
- The only golden-side file that moved is `data/expected/pkg-consumer.golden`, the size line only (`v1-sizes ...`; see Deviations 1).
- Suite `run` wall time, one run right after a gate on a loaded laptop: dev (ASan and UBSan) 0.51 s, release 0.28 s, 148 cases in each.
- Gate wall time: `sh` gate-time total=33, `dash` about 36 s wall, container (x86_64 emulated, gcc 13.3, cmake 3.28.3) total=129.

## Deviations from Plan

**1. [Rule 3 - Blocking] The installed consumer's size line moved in Task 1, not Task 4.** `v1_sizes()` asserted `sizeof(ivs_run_report) == 112`, so the gate was red as soon as the report grew; Task 1 is gate-green by the plan's own rule. It was rewritten to the final form in Task 1 (pinned constants and grown sizes) and the golden line updated; Task 3 moved `info=256` to `info=288` in the same line. Net change against the plan start: that one line only.

**2. [Rule 3 - Blocking] `IVS_INFO_V1_SIZE` is now public.** It lived in the internal `machine/machine.h`; the plan has the installed consumer print it from the installed headers, so it moved to `ivs/ivs_machine.h` (value 216, same meaning, `machine.c` static assert kept). `machine.h` keeps a pointer comment. No pinned define class covers it (`pinned-defines` unchanged by the move).

**3. [Rule 1 - Plan inconsistency] `psg_log` versus the event array is `RUN_PSGLOG_OVERLAP`, decided in stage 4b.** The plan text says to add the log to the event-array overlap list (which gives `INPUT_EVENTS_OVERLAP`) and, in must_haves, that overlap with events refuses `RUN_PSGLOG_OVERLAP`. Stage 4b runs before 4c, so the events list would never be reached; the log's overlap list in 4b includes the events span (zero when `event_count` is above the cap, as for the other event tests). Header, `api-effects.json` and the case `run.buffers.psglog-overlap` say so.

**3b. [Rule 1] The precedence of the log is "after any video or audio problem"**, not per check kind: video and audio NULL then overlap, then the log NULL then overlap (one reading of "order video, audio, psg_log"), so `run.prec.psglog-after-audio` can state that any video or audio refusal is reported before any log refusal.

**4. [Rule 2] The report's overlap span is the written byte count.** The plan does not say; without it a v1-sized (112 byte) report next to a buffer would be refused against the 136 bytes the core never writes. `ivs_bound_rep_bytes` is used by the stage 3 test, the report-ok test and every buffer and event overlap test; the case `run.buffers.psglog-overlap` has the positive control (log right after a 112-byte report accepted, one byte lower refused).

**5. [Rule 1] The env seam has no `struct_size`.** `ivs_run_env` is an internal plain struct built with `memset` by its callers, so "the same treatment" is only the appended field `psg_log_bytes_per_frame` (and one reserved word to keep the struct a multiple of 8); a caller that does not know the field leaves it 0.

**6. Test design fix.** The first draft of `run.buffers.psglog-overlap` used a capacity of `UINT64_MAX` from an ordinary pointer; in the non-sanitised builds (the fault builds) that span also covers the caller's stack report, which then is "part of the cause" and is not written, so the report-reason check failed there only. That sub-case was removed (the near-top-of-address-space pointer remains; the report-overlap sub-cases cover the report being part of the cause).

**7. Fault allowed-prefix additions.** `RUN_CAPACITY_AFTER_STEP` and `RUN_TARGET_UNCHECKED` gained `run.prec.psglog-after-audio` (its last sub-cases need the capacity and range stages), `INFO_TAIL_UNGUARDED` gained `run.info.v2-size-caller`. Each fault still fails its named case.

## Decisions Made

- The log stage precedes `BAD_STATE` and `OUT_OF_RANGE` and follows video and audio (D-14); a log problem on an OFF machine is `RUN_PSGLOG_NULL`.
- Request v2 fields are ignored (read as NULL and 0) for any `struct_size` below 80, including 65..79; the bytes of such a partial tail are not scanned.
- The requirement for the log is `psg_log_bytes_per_frame * (max(target - now, 0) / nominal_frame_ticks + 1)` with the same overflow guards as video and audio; it is 0 until plan 05-08.

## Open Items

- **API-03 and AUD-02 stay Pending in REQUIREMENTS.md.** This plan only grows the layout; the input byte (05-05), the audio samples (05-06) and the log (05-08) and the end-to-end evidence (05-11) close them.
- `run.info.v3-fields-zero` and `run.struct.report-v2-fields` assert the appended info and report fields are 0; plans 05-06 and 05-08 must replace those checks with their literals (named in the case comments).
- `data/api/api-effects.json` still states that "Nothing in the machine reads the words" and the `run.input.guest-invisible` text: untouched on purpose, plan 05-05 rewrites them.
- `IVS_RUN_REQUEST_V1_SIZE` is the 64-bit-pointer statement; on other pointer widths the layout differs and only the visible skip of `run.struct.request-v2-fields` and `run.struct.v1-sizes-lp64` applies (as in Phase 4).
- Carried from earlier plans: the pending blind second reads recorded in 05-01 are done (`b82c707`); D10/D11 items, the unobtained AY-3-8910 reprint and the pin chart, and Phase 4 items IN-04, IN-06, IN-09..IN-11 are unchanged.

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=35/35 controls=296/296 ... faults=152/152 ... tests-expected=2844 tests-executed=2842 tests-skipped=2 tests-missing=0`, `gate-time total=33`.
- `dash tools/gate.sh`: the same line, `gate: PASS`, 0 `FAIL` lines.
- `sh tools/gate-linux.sh`: exit 0; log last line `gate: PASS steps=35/35 controls=296/296 ... faults=152/152 ... tests-missing=0` (`cc=GNU`, gcc 13.3.0, cmake 3.28.3, `gate-time total=129`).

## Threat Flags

None new. Mitigations from the plan's threat model: T-05-05 (report written only inside the caller's `struct_size`: `run.struct.report-v1-caller`, fault `RUN_REPORT_TAIL_UNGUARDED`), T-05-06 (psg_log aliasing: pairwise and wrap tests, nothing written on refusal: `run.buffers.psglog-overlap`, fault `RUN_PSGLOG_OVERLAP_UNCHECKED`), T-05-07 (the overflow guard and the above-2^63 refusal before capacity kept; `run.buffers.psglog-small` includes the 2^63 target with 1-tick frames), T-05-SC (no installs).

## Self-Check: PASSED

- Files exist: tests/run/run_struct.c, data/expected/psg-log-source.ids, tools/check/controls/api-psglog-src-orphan, tools/check/controls/api-psglog-src-unpinned.
- Commits on HEAD: 9b31c5b, 6bda898, 47661b2, 9be491d.
