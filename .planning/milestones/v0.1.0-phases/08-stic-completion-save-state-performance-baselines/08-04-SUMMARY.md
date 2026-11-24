---
phase: 08-stic-completion-save-state-performance-baselines
plan: 04
subsystem: Phase 8 wave A, plan 2 - the pure MOB renderer held to the frozen oracle of plan 08-03 - three profile fields, the card-layer on-pixel mask, a line-resolved MOB decode and overlay, one composition path for the streamed slot and the whole plane, the video.mob. test family (20 ids), two named faults and three pinned defaults that flip
tags: [A, VID-05, renderer, mob, D-01, D-02, D-03, D-05, D-10, T-08-12, T-08-13, T-08-14]
status: complete

requires:
  - phase: 08-stic-completion-save-state-performance-baselines
    provides: "plan 08-03 oracle: docs/video/diag-stic-2-derivation.md sections (a) to (j), frames F-M0..F-M9, the MOB literal, rules STIC-R37..R50; the blind second read closed at 1f6e19c (two wording corrections); counts at plan start steps=37/37 controls=371/371 faults=222/222 tests-expected=4628 tests-executed=4624 tests-skipped=4 profile-keys=143 fields=143 unknown-fields=37 stic-rules=67 video-frames=26 goldens=15 (re-measured with a full gate run at the start, identical)"
provides:
  - "core/src/stic/stic_mob.c: ivs_stic_mob_decode and ivs_stic_overlay_mobs_row (pure, line-resolved, no static or global data); core/src/stic/stic_cards.c: ivs_stic_cards_on_row; core/src/stic/stic_frame.c: ivs_stic_compose_card_row, called by ivs_stic_frame_deliver and ivs_stic_render_plane_cs (one composition path)"
  - "profile fields video.mob_field_origin (8, 8, tier4-community), video.mob_line_sample (0) and video.mob_prio_hides (0), the last two unknown-default-chosen (C108, C107), through the generator into ivs_stic_params"
  - "tests/video/video_mob.c (20 ids: 18 video.mob. and 2 video.judge.mob-), the FRAMESET=mob include of cmake/IvsVideoFrames.cmake, faults MOB_PRIORITY_SWAP and MOB_FLIP_SWAPPED, STIC-R28 reworded"
affects: [08-05, 08-06, 08-07, 08-08]

actuals:
  tokens: 150000
  tasks: 3
  commits: 4
plan_head_before: 1f6e19c2be55da61e675be3837b667023858b69f
plan_head_after: 7323b7e786ccc6a77308ac26ab47222e6d5ce46d

key-decisions:
  - "Order of the overlay: among the MOBs with a set pattern pixel on a position the lowest index owns it (R41), and only then is the owner's priority bit tested against the card layer, so a hidden owner still blocks a higher index; the lineage does not say what a priority-set owner over a higher-index MOB shows, so this detail belongs to the pinned PD-1 behavior and no frame, assert or marker depends on it"
  - "The overlay never writes working column 159 (x at or beyond video_width), so the cut-off pixel of the card layer stays the blank sentinel and the 160 x 96 working plane of a MOB frame equals its literal including the wildcard column"
  - "The on-pixel mask of the card layer is computed only for a card row where a MOB with the priority bit is on; with every MOB register at its reset value the composition returns the card row untouched (neutrality, T-08-12)"
  - "video.mob.params-match-profile reads the three profile values with a second reader of the profile in cmake/IvsVideoFrames.cmake (the precedent of cmake/IvsPsgProfileProbe.cmake), separate code from the core generator"

key-files:
  created:
    - core/src/stic/stic_mob.c
    - tests/video/video_mob.c
    - .planning/phases/08-stic-completion-save-state-performance-baselines/08-04-SUMMARY.md
  modified:
    - core/src/stic/stic.h
    - core/src/stic/stic_cards.c
    - core/src/stic/stic_frame.c
    - core/src/stic/stic_params.h
    - core/CMakeLists.txt
    - cmake/IvsSticTable.cmake
    - cmake/IvsVideoFrames.cmake
    - data/profiles/intv-2609-ntsc-family-p1.json
    - data/expected/profile-fields.ids
    - data/stic/fields-pending.ids
    - data/stic/tests-pending.ids
    - data/stic/rules.json
    - docs/stic/reference-notes.md
    - docs/video/diag-stic-2-second-read-record.md
    - docs/video/diag-stic-2-derivation.md
    - tests/CMakeLists.txt
    - tests/video/video_cases.h
    - tests/video/video_main.c
    - tests/stic/stic_cases.h
    - tests/stic/stic_progs.c
    - data/expected/tests/video.ids
    - data/expected/faults/video.ids
    - .planning/STATE.md
    - .planning/ROADMAP.md

requirements-completed: []

completed: 2026-11-14
---

# Phase 8 Plan 04: the pure MOB renderer Summary

**A guest that programs the MOB registers now gets sprites over the cards, drawn to the pixels of the frozen oracle through the one composition path that both the streamed slot and the whole plane use, and nothing else moved: all 15 goldens, both digest literal files and the 16 card frames are byte-identical. Every oracle literal the renderer can reach (F-M0 to F-M3, and the two pinned sampling frames F-M8 and F-M9) matched on the first run, with no disagreement between the core and the oracle. The pinned pieces are profile fields held by tests that flip them, two named faults hold the priority and flip rules, and the final gates pass under sh, dash and the Ubuntu container with `steps=37/37 controls=371/371 faults=224/224 fault-runs=224 tests-expected=4668 tests-executed=4664 tests-skipped=4 tests-missing=0 profile-keys=146 stic-rules=67 video-frames=26 goldens=15`. VID-05 stays Pending (it closes in plan 08-08, after collision, delay, border extension and the scenarios).**

## Commits

| Commit | Task | What |
|--------|------|------|
| `9250265` | 1 (precondition) | the blind-read corrections applied: the line `corrections-applied: yes` in the record and a paragraph in derivation section (k); the new freeze base |
| `f587524` | 1 (tracer) | three profile fields, `ivs_stic_cards_on_row`, `stic_mob.c` (placement, X size, one Y size bit, color, MOB over MOB), `ivs_stic_compose_card_row`, the FRAMESET=mob include, ids `video.mob.base-neutral`, `frame-placement-clip`, `field-origin-8-8`, `params-match-profile`, STIC-R28 reworded |
| `5140403` | 2 | tall mode, both flips, the Y size codes, color bit 3, the priority bit, the 48-row register-bit sweep, stream equals plane, slice invariance (11 ids) |
| `7323b7e` | 3 | `MOB_PRIORITY_SWAP` and `MOB_FLIP_SWAPPED`, the three `video.mob.default-pinned.*` ids and the two judges (5 ids) |
| the commit that adds this file | | this SUMMARY, STATE.md and ROADMAP.md |

## Precondition check (Task 1 step 1)

- `docs/video/diag-stic-2-second-read-record.md` line 1 is `state: closed`; `oracle-commit` 641a12e3a1059b242cc88a3ee4308e3f58d9b36d is an ancestor of HEAD (`git merge-base --is-ancestor` holds).
- The blind-spec range of the derivation (`sed -n '/^<!-- blind-spec-begin -->$/,/^<!-- blind-spec-end -->$/p'`) hashes to `f0e0c8767e22802f4fc3d14f92e2e1f4089aa91ea44ae06b6d086a8ef36e84bf` now, against `b73a15b9288729ed5c053d1570fa0b5ff2ab7a6b58f6e1f15d58d672e09102ed` at the oracle commit. A diff of the two ranges shows exactly two changed lines, A5 (the clear is `$0200` to `$02EF`) and A11 (no collision from a position outside the plane, the clipped MOB of scene 1 named), which are the two listed corrections. No `blind-spec-changed` halt applied.
- The record lists `corrections: 2`. The orchestrator had already applied both in 1f6e19c, so this plan edited no frame, hash, pair or assert; commit `9250265` appends `corrections-applied: yes` to the record and one paragraph to derivation section (k). The freeze checks of this plan diff against that commit (the last commit that touched the record): `git diff --quiet 9250265 -- data/video/frames docs/video/diag-stic-2-derivation.md` exits 0 on every later commit.

## The A2 grep (assumption A2 of 08-RESEARCH)

All six committed program listings were read for stores to the MOB registers (`$0000` to `$0017`) and the delay and extension registers (`$0030` to `$0032`): boot stub 1 (one store, `$035F`), boot stub 2 (no store), Lamp Tone (direct stores to `$0020`, `$0021`, `$01xx` and `$02xx`; indirect stores through R4 pointing at `$0256`, `$0292`, `$3800` and `$0200`), the CPU diagnostic (`$0200` to `$020F`), the controller and sound diagnostic (`$01xx`, `$0210` and `$0220` pointers, `$020F`) and STIC diagnostic 1 (`$0020`, `$0028` to `$002B` through R5, `$3800` through R5, `$0200` through R4, a few `$02xx`). No program stores to a MOB, delay or extension register, so no `golden-would-change` halt applied; the 15 goldens and both digest literal files are byte-identical (`git diff --quiet` against the merge base is empty at every commit).

## What was built

- **Profile fields and generator.** `video.mob_field_origin` {x 8, y 8} (tier4-community, rule R37, C105), `video.mob_line_sample` 0 (C108, resolver D10-VID-14) and `video.mob_prio_hides` 0 (C107, resolver D10-VID-11), read by `cmake/IvsSticTable.cmake` with fail-closed range checks (origin 0 to 15, selectors 0 or 1, the sampled line below `video.row_lines`) into the members `video_mob_origin_x`, `video_mob_origin_y`, `video_mob_line_sample`, `video_mob_prio_hides` of `ivs_stic_params`; the three names are in `data/expected/profile-fields.ids` and their pending lines are deleted in the same commit.
- **Renderer.** `ivs_stic_cards_on_row` is the pure sibling of the card row renderer (one bit per position, 20 bytes per plane row; foreground set pixels and Colored Squares colors 0 to 6 are on, color 7 and the cut-off pixel are not); `ivs_stic_render_cards_row` is untouched. `stic_mob.c` decodes the three registers of each MOB into a 12-byte struct and overlays them per plane row on the card row, line-resolved: pattern row r covers `lpr = 1 << (Y size code)` television lines, the plane row y publishes line `video_row_lines x y + video_mob_line_sample`, so the zooms 2x, 4x and 8x are sampling-invariant and only the two half-height codes depend on the pinned selector. `ivs_stic_compose_card_row` renders the card row, then overlays; a display-off row, a geometry the card decoder does not draw and a row with no MOB on are returned as before.
- **Tests.** `video_mob.c` (about 1,100 lines): the F-M literals come through a new generated include (`-DFRAMESET=mob` of `cmake/IvsVideoFrames.cmake`, table `vm_frames[10]` kept apart from `vf_frames[]`, `-` kept as the wildcard, `sha256-text none` accepted, 48-row role table from `data/video/mob.literal.txt`, three profile values), the register words are typed in the test source from derivation sections (a) and (c), and every shape (zoom, tall, flips, colors, overlaps, origin table, priority cases) is derived by hand in the test text.
- **Faults and pins.** `IVS_FAULT_MOB_PRIORITY_SWAP` and `IVS_FAULT_MOB_FLIP_SWAPPED`, one site each in `stic_mob.c`; the three `default-pinned` ids each assert the default and flip one field of a stack copy of `ivs_stic_params`; two judges show the frame comparison and the register sweep can fail.

## Disagreements between the core and the oracle

None. The seven literals that the renderer can reach without plans 08-05 and 08-06 (F-M0, F-M1, F-M2, F-M3 with its hash, and F-M8 and F-M9 under the even-line default) matched position by position and by hash on the first run of the finished renderer; no oracle file was edited and none needed to be. F-M4 to F-M7 (border strips and delay) need plan 08-06 and are not compared here.

## Stack budget (T-08-13)

`-fstack-usage` of the clang in this checkout, at -O0 (the dev preset) and -O2: `ivs_stic_frame_deliver` **2,416 bytes** at -O0 and 2,368 at -O2 (identical to its frame before this plan, 2,416 and 2,368), `ivs_stic_compose_card_row` 448 and 384, `ivs_stic_overlay_mobs_row` 176 and 144, `ivs_stic_cards_on_row` 128 and 144, `ivs_stic_mob_decode` 96 and 0. The deepest chain `deliver -> compose -> overlay` is 3,040 bytes at -O0 and 2,896 at -O2, under the 4,096-byte probe threshold; the `core-symbols` step is green on both platforms (core-imports 5 on macOS, 3 on Linux, no stack probe). The working buffers are the 2,048-byte card row of `deliver`, a 160-byte on-mask and eight 12-byte MOB structs in `compose`, and a 20-byte claimed mask in the overlay; loops are bounded by 8 MOBs, 8 plane rows and 16 positions per MOB row.

## Fault-inventory upkeep (rule fault-wrong-reason)

Each addition is the family prefix `video.mob.` appended to column 3 (allowed prefixes); no must-fail or must-pass column was edited.

| Task | Fault line | Added | Failing id that demanded it |
|------|------------|-------|-----------------------------|
| 1 | VIDEO_WRONG_CARD_MODE | `video.mob.` | base-neutral, frame-placement-clip (the card decode every F-M frame contains) |
| 1 | VIDEO_CARD_BIT_ORDER_REVERSED | `video.mob.` | the same two ids |
| 1 | VIDEO_GRAM_GROM_SELECT_SWAPPED | `video.mob.` | the same two ids |
| 2 | STIC_EVENT_ORDER_START_BEFORE_END | `video.mob.` | slice-invariant (a displayed frame delivered with the flag clear) |
| 2 | VIDEO_STATE_DEPENDS_ON_BUFFER | `video.mob.` | stream-equals-plane (the no-buffer chain) |
| 3 | MOB_PRIORITY_SWAP, MOB_FLIP_SWAPPED (new lines) | `video.mob.` only | front-lower-index with frame-priority-colour and slice-invariant; frame-mirror with size-8x16 |

The plan predicted further additions (VIDEO_20TH_COLUMN_WIDTH, VIDEO_CSTACK_NO_ADVANCE, VIDEO_FRAME_AT_CALL_END, `video.judge.`); the runs did not demand them and none was added.

## Checks shown able to fail

- The two named faults make exactly the intended ids fail with the allowed prefix (a run of each fault executable: MOB_PRIORITY_SWAP fails frame-priority-colour, front-lower-index and slice-invariant; MOB_FLIP_SWAPPED fails size-8x16 and frame-mirror), and `video.geom.profile-derives-159x96` passes under both.
- `video.judge.mob-frame-corrupt` (one expected position of F-M1 changed in a copy at three places: the comparison reports that very position and the hash of the copy differs; the uncorrupted copy passes; a wildcard position is skipped, never compared) and `video.judge.mob-role-swapped` (the roles of a used bit and an ignored bit exchanged in a copy of the table, three pairs: the sweep reports at least two disagreeing rows each time; the true table passes).
- The three `default-pinned` ids fail if a default or its flip loses its effect: prio-set expects 39 MOB and 25 card positions under the default and none of the MOB under the flip; the two sampling frames compare the even line to F-M8 and F-M9 and require a changed pixel under line sample 1.
- The 48-row register sweep requires a change for every non-ignored bit and none for the five, four and two ignored bits of the three registers (each ignored bit in two base states), with one named exemption: the interaction-enable bit (X register bit 8), which acts only through collision accumulation (plan 08-05) and so must leave the plane unchanged here (`exempt: collision-only, see 08-05`).

## Deviations from Plan

**1. [Interpretation] The blind-read corrections were already applied.** The orchestrator applied both corrections in 1f6e19c, so the first commit of this plan only appends `corrections-applied: yes` and the paragraph of section (k) (the plan text expected this executor to edit the oracle files). The blind-spec hash was checked against the two listed correction lines instead of being compared to equality.

**2. [Addition] `tests/CMakeLists.txt` in Task 2.** The generator now also depends on `data/video/mob.literal.txt`, so the custom command of the include lists it (the file is in the plan's `files_modified`, not in Task 2's file list).

**3. [Addition] A second reader of the profile.** `video.mob.params-match-profile` needs "the profile values read independently". The generator `cmake/IvsVideoFrames.cmake` (mob branch) reads the three values with its own code and emits them as macros; the comment at the head of that script, which said test generators never read the profile, names the exception and its precedent. `tests/video/video_main.c` grew its TAP source table from 16 to 32 entries to carry the ten F-M literals, the role literal and the profile.

**4. [Interpretation] The order of precedence of an overlap that involves the priority bit.** See key-decisions: the rules give the order of two MOBs (R41) and the card rule of a priority-set MOB (R43) separately; the renderer applies them in that order. It is part of the pinned behavior (C107) and nothing asserts it.

**5. [Interpretation] The placement of the ignored-bit checks.** The sweep checks every ignored bit in two base states (the staircase and the MOB over the card block), because in the first base state a priority bit is invisible (a priority-set MOB over a card background shows through under the default), which let the role-swap judge pass when the swapped pair included the priority bit.

**6. [Information] No plan-listed prefix beyond `video.mob.` was needed** (see the fault-inventory table), and the three derivation (i) test-id proposals for sampling (`video.mob.default-pinned.line-sample`) became the two ids that the plan names (`sampling-y1x`, `sampling-8x16`); plan 08-08 writes the final ids.

No auth gates, no package installs (T-08-SC), no `gh` call, no push, no tag; `.planning/config.json` and `.planning/state.json` were never staged.

## Decisions Made

- The three new fields and the four members are the whole parameter change; `ivs_stic_params` grew by 16 bytes (four uint32), `ivs_stic_state` is still 528 bytes, no digest literal moved, so the snapshot waves see no new hidden state from this plan.
- The test cards: the three program cards of derivation A2 plus the pair of F-M9 (card 4 the staircase, card 5 its right-aligned mirror) are poked into every scene; the generated graphics ROM of the register sweep (byte y of card c is c + 1 + 37 y) exists only in that one base state, is generated at run time and is not a graphics ROM image of any console.
- The F-M literals are compared over the 159 published columns, a `-` position skipped, and the hash of the published columns is compared whenever the literal has one (F-M7 has none).

## Cost (laptop, load average high during the runs)

| Item | Value |
|------|-------|
| gate total, sh, at plan start (full run) | 91 s, gate-cpu user 145.9 sys 133.4 |
| gate total, sh, Task 3 tree | 89 s, user 154.9 sys 142.5 |
| gate total, dash, Task 3 tree | 84 s, user 140.8 sys 123.5 |
| container gate, Task 3 tree | 305 s (gcc 13.3.0, cmake 3.28.3, x86_64 emulated), gate-cpu user 1221 sys 75 |
| `ivs_tests_video`, all 120 ids, dev preset (ASan, UBSan) | 0.37 s real, of which the 20 plan ids take 0.10 s |
| the 224 fault runs | two more runs of the video suite (0.4 s each) |

The additions are inside the noise of a loaded laptop; `.planning/CI-EFFICIENCY.md` is not edited by this plan (the checkpoint line of plan 08-08 carries the figures).

## Open Items

- **Plan 08-05 (collision).** `ivs_stic_frame_deliver` still takes `const ivs_memsys *`; the FRAME_END mutator and the non-const change are 08-05's. The overlay computes owners, not collisions: a collision needs the set pixels of every active MOB, not only the owner, and an invisible MOB still interacts (R44), so `ivs_stic_mob_decode` will need an active flag that does not require the visible bit and the interaction bit (X register bit 8, decoded nowhere yet). Remove the exemption row in `vm_base_for` of `tests/video/video_mob.c` (the `intr` role) and add the collision registers `$0018` to `$001F` as the observability channel (choice (b) of the round-2 review: the id stays `video.mob.register-bit-roles`, so no per-plan count moves). The stored view of the collision rows loses the own bit (R44 note, carried from 08-03).
- **Plan 08-06 (delay and strips).** `ivs_stic_compose_card_row` is the one place to add them. The vertical-delay band carries up to 7 plane rows (7 x 160 = 1,120 bytes) across two source card rows: a carry buffer in the frame of `ivs_stic_frame_deliver` would take the chain `deliver -> compose -> overlay` to about 4,160 bytes at -O0 and 4,016 at -O2, which is at or over the 4,096-byte probe threshold. Do not add a carry buffer; re-render the lower plane rows of the previous card row (the renderer is pure) or restructure the composition. The pins PD-8 to PD-12 stay as in 08-03.
- **Plan 08-07 (program and scenarios).** Carried unchanged: every register read and write of a routine must end before cycle 1,900 (reference 1,745, straight 1,394), the re-arm count N = 500, the scenario lines equal the assert blocks of derivation section (f) line for line, `# frame:` ties in the goldens. The MOB frames the scenarios assert are F-M1, F-M2 and F-M3 (frames 2, 5, 8 of the `mob` scenario); this plan holds them in process, so a scenario mismatch will be a program or a runner problem, not a renderer one.
- **Plan 08-08 (claims).** The final ids of the pinned-default families: this plan registers `video.mob.default-pinned.prio-set`, `.sampling-y1x`, `.sampling-8x16`. Claim exclusions for PD-1 and PD-2 carry as in derivation section (h).
- **Information.** D10 and D11 are not performed and no claim above provisional exists; every expected value of this plan is `source-expectation`, tier 4 lineage, with a blind second read of the same model family (C43).

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=37/37 controls=371/371 ... fault-runs=224 ... faults=224/224 ... profile-keys=146 ... goldens=15 ... fields=146 unknown-fields=39 ... tests-expected=4668 tests-executed=4664 tests-skipped=4 tests-missing=0 ... stic-rules=67 ... video-frames=26 ... (0 FAIL lines; gate-time total=89 s on the Task 3 tree)
dash tools/gate.sh:    the identical summary line (md5 423a5c0421c7ab28c03427c7c1f61d81 for both shells; 0 FAIL lines; total=84 s)
sh tools/gate-linux.sh: exit 0 on the same tree (build/gate-linux.log ends gate: PASS; gcc 13.3.0, cmake 3.28.3, x86_64 emulated, cc=GNU, core-imports=3), steps=37/37 controls=371/371 faults=224/224 fault-runs=224 tests-expected=4668 tests-executed=4664 tests-skipped=4 tests-missing=0 profile-keys=146 stic-rules=67 video-frames=26 goldens=15, 0 FAIL lines (gate-time total=305 s)
```

On the closing tree (this SUMMARY, STATE.md and ROADMAP.md staged, `tracked=2362`): sh and dash `gate: PASS` with the identical summary line (md5 f648aadd3439d982e23745efdf77bba6 for both shells; 0 FAIL lines; total 80 s each).

After Task 1 (before its commit): sh and dash `tests-expected=4636 tests-executed=4632 tests-skipped=4 faults=222/222 profile-keys=146 fields=146 unknown-fields=39 goldens=15` (md5 0ca2bb8cc35b14bef9bbbdba7d29a2aa for both shells). After Task 2: `tests-expected=4658 tests-executed=4654 faults=222/222` (md5 e88fd875da97de4316d5c769540bc939 for both shells). The first Task 1 and Task 2 gate runs each stopped at the `faults-verdict` step with the `fault-wrong-reason` findings of the table above (and nothing else), which the upkeep removed. `git diff --quiet` of `data/golden` and both digest literal files against the merge base, and `git diff --quiet 9250265 -- data/video/frames docs/video/diag-stic-2-derivation.md`, exit 0 on the closing tree.

## Known Stubs

None. The interaction-enable exemption of the register sweep is a named, plan-specified exemption (a bit with no visible effect until plan 08-05), not a stub.

## Threat Flags

None new. T-08-12 (the composition changes existing pictures): all 15 goldens, both digest literal files and the 16 card frames are byte-identical, and `video.mob.base-neutral` compares every F-A and F-B frame and the display-off frame. T-08-13 (stack growth, unbounded loops): the stack figures above, loops bounded by 8 MOBs, 8 plane rows and 16 positions, the register sweep and the slice test over hostile and ignored bits. T-08-14 (a pinned default read as confirmed behavior): three default-pinned ids with a one-field flip, both fields tagged unknown-default-chosen with D10 resolvers, F-M8 and F-M9 used only in process. T-08-SC: no installs.

## Self-Check: PASSED

Verified before the final commit: `core/src/stic/stic_mob.c` and `tests/video/video_mob.c` exist; commits `9250265`, `f587524`, `5140403` and `7323b7e` are ancestors of HEAD and `git rev-list --count 1f6e19c2be55da61e675be3837b667023858b69f..HEAD` printed 4 before this file's commit; `.planning/config.json` and `.planning/state.json` are not staged; `git ls-files` lists no `.ivsc`, `.bin`, `.rom`, `.exec`, `.wav`, `.ppm`, `.vraw` or `.ivsn` file; the acceptance greps give 3 fields in `data/expected/profile-fields.ids`, 0 `video.mob` pending field lines, 1 `stic_mob.c` line in `core/CMakeLists.txt`, 18 `video.mob.` ids, 2 `video.judge.mob-` ids and 1 STIC-R28 statement that says the MOB registers are applied.
