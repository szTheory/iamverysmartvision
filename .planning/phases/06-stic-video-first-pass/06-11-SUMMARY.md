---
phase: 06-stic-video-first-pass
plan: 11
subsystem: core/stic, tests/video, tests/stic, cmake
tags: [color-stack, colored-squares, mode-switch, cs-ptr, frame-literals, bit-role-sweep, metamorphic, named-faults, D-06, D-18, D-19, VID-04, VID-06]
status: complete

requires:
  - phase: 06-stic-video-first-pass
    provides: "plan 06-10 Foreground/Background renderer, graphics accessor, role sweep; plan 06-07 report (cs_ptr) and peek sweeps; plan 06-06 mode switch hook; plan 06-01 hand-derived frames F-B1..F-B3 and card literal; gate steps=36/36 controls=332/332 faults=218/218"
provides:
  - "core/src/stic/stic_cards.c: Color Stack decode by (b12, b11): graphics ROM card, graphics RAM card, graphics RAM card with foreground 8 + fg, Colored Squares; advance before draw, wrap 3 to 0, one pointer across all rows, the cut-off 20th-column card advances, a squares card never advances; the pointer is carried through ivs_stic_render_cards_row (cs_pos in and out)"
  - "ivs_stic_frame_deliver stores the final pointer of a display-on frame in state cs_ptr (a display-off frame leaves it; it is not cleared at frame start); each frame render starts at 0"
  - "video.cstack.* (13), video.mode.* (2), video.card.cs-decode, video.probe.bit-roles-cs-* (3), video.meta.mode-agree and permute, two judges; stic.peek.sweep-bytes-equal-after-render, stic.state.cs-ptr-after-render; fault VIDEO_CSTACK_NO_ADVANCE"
  - "suite video 76 to 99 ids, suite stic 97 to 99 ids; faults 218 to 219"
affects: [06-12, 06-13, 06-14]

actuals:
  tokens: 36000
  tasks: 3
  commits: 3
plan_head_before: 3f47bf0cf4c20fec8ebdad0500c0cca57d074417
plan_head_after: 398f3be5b2d72aeca8115b51f47540ee6484039f

key-decisions:
  - "ivs_stic_render_cards_row takes the pointer in and out (cs_pos) so the machine keeps rendering one card row at a time (no large stack frame); the plane form takes an optional cs_final and starts at 0; ivs_stic_render_card_row gained cs_pos and ivs_stic_render_plane_cs was added beside ivs_stic_render_plane"
  - "ivs_stic_frame_deliver takes a non-const state (it writes cs_ptr); the stored pointer is the final pointer of the last display-on frame, 0 after a Foreground/Background frame"
  - "The frame generator reads the stack line as four hex digits (the literals write F for 15)"

key-files:
  created:
    - tests/video/video_cstack.c
  modified:
    - core/src/stic/stic.h, stic_cards.c, stic_frame.c
    - cmake/IvsVideoFrames.cmake, tests/CMakeLists.txt
    - tests/video/video_cases.h, video_main.c, video_fgbg.c, video_card.c, video_probe.c, video_meta.c, video_judge.c, video_progs.c, video_slot.c, vid_fields.c
    - tests/stic/stic_peek.c, stic_state.c
    - data/expected/tests/video.ids, stic.ids, data/expected/faults/video.ids, data/stic/tests-pending.ids
    - docs/video/frame-derivations.md, docs/stic/reference-notes.md

duration: about 3 hours elapsed (laptop loaded)
completed: 2026-11-12
---

# Phase 6 Plan 11: Color Stack and Colored Squares rendering, mode switch, inspection completion Summary

**A display-on Color Stack frame is decoded from the row latches and the four stack registers with advance before draw, wrap, a per-frame restart and the cut-off column rule; Colored Squares is implemented; the hand-derived frames F-B1, F-B2 and F-B3 of plan 06-01 are reproduced byte for byte in the unit path and F-B1 and F-B3 also through the whole machine (a stand-in guest reads $0021 inside the register window), and the VID-06 inspection sweeps now run with a rendered frame present.** No frame or literal was edited; the first run of F-B1 passed with no renderer change after the first draft.

Oracle commit this plan follows: `eae8ca5` (06-01 task 5). First commit of the plan (FIRST): `159dcd1`. Status of the blind second derivations of F-B1..F-B3: identical to the committed planes (frames 10 of 10 identical, no unknown, no disagreement); card bit roles 62 of 64 identical with two labelling-only differences on the cs-grom and cs-gram bit 11 rows (recorded; the renderer decodes the class by the bit pair 12 and 11 as the plan states, so the labels do not change a pixel). Agreement of the renderer with the literals checks transcription and derivation by one model family, never an independent implementation (C43).

## What was built

- Task 1, tracer (`159dcd1`): Color Stack branch of the pure renderer, the pointer carried across rows, the final pointer stored by the frame event, the generator and the frame table extended to F-B1..F-B3, `video.cstack.F-B1` and `machine-F-B1`, the reviewed inversion.
- Task 2 (`5a5d60d`): F-B2/F-B3, the rule tests, squares tests, the 48 probes, metamorphic relations, the mode switch tests, the two stic sweeps, fault `VIDEO_CSTACK_NO_ADVANCE` with its site, two judges.
- Task 3 (docs commit): the Colored Squares cut line recorded as a fallback NOT applied (`docs/video/frame-derivations.md`), C96, C97, C98, C102 status with test ids (`docs/stic/reference-notes.md`), one pedantic-ISO-C fix found by the container gate, the three gates.

## Reviewed test change (named)

`video.frame.cs-not-decoded` is INVERTED into `video.frame.cs-decoded` (a strobed Color Stack frame over all-zero BACKTAB, graphics and stack registers is color 0 at every published position, and the report pointer is 0). No other existing test changed; the allowed collateral prefixes of five earlier faults gained `video.cstack.` (and `video.mode.`, `video.meta.` where the new ids fail for the right reason: a wrong card mode fails mode-agree and permute; an event-order fault leaves the displayed flag clear so the mode tests fail).

## Tests and fault

| Fault | Site | Must fail | Also fails (observed) |
|-------|------|-----------|-----------------------|
| VIDEO_CSTACK_NO_ADVANCE | stic_cards.c | video.cstack.advance-before-draw | cstack F-B2, F-B3, machine-F-B3, advance-wrap, one-stack-across-rows, cutoff-column-advances, reset-per-frame, squares-stack7, the three cs probes, both cs judges (the plan names advance-before-draw, F-B2 and advance-wrap: all three fail) |

The faults file allows one must-fail id per fault. Suite times (plain run, loaded laptop): `video` dev 0.56 s (dev target 1.5 s), release 0.17 s; `stic` dev 0.86 s (target 2.0 s), release 0.31 s.

## Deviations from Plan

**1. [Interpretation, recorded] The frame-literal fixture checker accepts the ignored bits.** F-B3 reuses the Foreground/Background words of F-A3, whose bits 10 and 9 are set; in a graphics RAM Color Stack word they are ignored (CR-10), so the annotation encoder compares the word with bits 10 and 9 masked for a cs graphics RAM cell. The frame literal was not edited.

**2. [Interpretation, recorded] The graphics ROM probe uses a synthetic graphics ROM in the unit-path memory only.** The cs-grom card number and foreground bits are invisible over the zero graphics ROM (the same issue plan 06-10 hit), so the Color Stack sweep pokes a project-authored pattern (card n: n at row 0, 5A at row 7) into the test memory's graphics ROM and graphics RAM. It is not an image of any ROM and never enters a fixture or the machine path (the machine-path tests clear the GROM, D-19). The sweep decodes each probe word by its class (bits 12 and 11) from the literal roles, so a fixed row (bit 12 of cs-grom) is exercised as a class change.

**3. [Interpretation] Extra ids.** `video.mode.write-selects-next-frame` (a write of $0021 in frame 1's register window; the wait of 960 loop iterations lands inside it, the test passes for 930 to 1010 iterations) and the judge `video.judge.cs-roles-swapped` (each cs sweep fails with two roles swapped) were added beyond the plan's list; `video.mode.switch-end-to-end` covers the four cases the plan names (read selects, write selects, a read outside the window changes nothing and counts as blocked, persistence across two displayed frames).

**4. [Rule 1 - Bug, hosted] Pedantic ISO C.** gcc 13 in the container rejected passing a pointer-to-array of `const char *` to a parameter of `const char *const` arrays (`-Wpedantic`); the helper parameters in `tests/video/video_probe.c` now match the array type. Apple clang did not report it. Found by the first container run; the repeat is green.

**5. [Interpretation] `ivs_stic_render_cards_row` and the plane entry points changed signature** (cs_pos; cs_final; `ivs_stic_render_plane_cs`), the only callers being the frame writer and the tests.

## Decisions Made

- No requirement is closed: VID-04 stays Pending (runner scenarios and end-to-end evidence are plans 06-12 to 06-14) and VID-06 stays Pending for the same reason; the gate evidence of this plan completes the render-side inspection proofs but not the end-to-end scenarios.

## Neutrality and cost

`git status --short data/golden data/scenarios data/expected/pkg-consumer.golden` is empty and `git diff --stat 3f47bf0..HEAD -- data/golden data/scenarios data/expected/pkg-consumer.golden` prints nothing: no scenario shows a display-on frame, so video is still not observed in the runner and the consumer, with equal `digest.*` lines. Cost: one fault (reruns the video suite once), 23 ids, no new core TU, no dependency; the video suite is 0.56 s dev. The normalised metric r = cpu_s / build_s can only be read from the first hosted row of this plan; this plan is not expected to move it measurably (an expectation, not a measurement; median 7.9, trigger 8.3 stands until a hosted row says otherwise). Local gate times: sh 61 to 82 s, dash 77 s, container 505 s (first full run, loaded laptop).

## Open Items

- Hosted `r` for this plan is not yet known; the arbiter is the first hosted row.
- The cs-gram and cs-grom bit 11 labelling difference of the blind second read stays recorded (no pixel effect).
- Colored Squares is a provisional default of one community lineage (C98, the order of the two low bits of the fourth color is the recorded open reading); no frame literal uses it, only constructed cells and probes. The cut line is documented, not applied.
- Carried unchanged: Phase 4 and 5 deferrals, `sh tools/hygiene.sh --history` on earlier planning commits.

## Final gate

```
sh tools/gate.sh:       gate: PASS steps=36/36 controls=332/332 faults=219/219 tests-expected=4304 tests-executed=4302 tests-skipped=2 tests-missing=0
dash tools/gate.sh:     gate: PASS steps=36/36 controls=332/332 faults=219/219 tests-expected=4304 tests-executed=4302 tests-skipped=2 tests-missing=0 (same summary line as sh apart from the tracked-file count)
sh tools/gate-linux.sh: GATE_LINUX_PLATFORM= run, exit 0, last line of build/gate-linux.log starts gate: PASS, x86_64 gcc 13 cmake 3.28, steps=36/36 controls=332/332 faults=219/219 tests-missing=0 (gate-time total=505, first run failed on the pedantic finding of deviation 4)
```

## Known Stubs

None. (The Color Stack interim of plan 06-09 is gone.)

## Threat Flags

None new. T-06-15 stands as accepted: every layout row is tier4-community (C96..C98, C102), claims stay provisional, no measured-hardware claim. T-06-SC: no installs.

## Self-Check: PASSED

Created file exists (`tests/video/video_cstack.c`); commits `159dcd1` and `5a5d60d` are ancestors of HEAD; `grep -c '^video.probe.bit-roles-cs' data/expected/tests/video.ids` is 3, `grep -c '^VIDEO_CSTACK_NO_ADVANCE' data/expected/faults/video.ids` is 1, `grep -c cs_ptr core/src/stic/stic_frame.c` is at least 1, `grep -c '^video.frame.cs-decoded$' data/expected/tests/video.ids` is 1.
