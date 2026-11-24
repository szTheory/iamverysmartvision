---
phase: 08-stic-completion-save-state-performance-baselines
plan: 05
subsystem: Phase 8 wave A, plan 3 - the collision registers held to the frozen oracle of plan 08-03 - a non-const frame deliver and an internal OR mutator, per-television-line collision over the object grid accumulated in a local uint32_t[8] and ORed into the register file at the frame end, four pinned profile fields, the video.collision. family (16 ids) and the truth-table judge, one named fault
tags: [A, VID-05, collision, D-02, D-06, D-07, D-08, D-10, T-08-15, T-08-16]
status: complete

requires:
  - phase: 08-stic-completion-save-state-performance-baselines
    provides: "plan 08-04 renderer: ivs_stic_mob_decode, ivs_stic_overlay_mobs_row, ivs_stic_cards_on_row, ivs_stic_compose_card_row, video_mob.c with the register-bit sweep; oracle of plan 08-03 (docs/video/diag-stic-2-derivation.md sections (a) to (j), frames F-M0..F-M9, second-read record) frozen since 9250265; counts at plan start steps=37/37 controls=371/371 faults=224/224 tests-expected=4668 tests-executed=4664 tests-skipped=4 profile-keys=146 unknown-fields=39 goldens=15 (re-measured with a full gate run at the start, identical)"
provides:
  - "core/src/memmap/memsys.[ch]: ivs_memsys_stic_collision_or_internal (the STIC's own write into $0018 + k, masked 03FF, no gate test, no counter, no access record)"
  - "core/src/stic/stic_mob.c: the act flag of ivs_stic_mob_decode (interaction enable, visible bit not required, X coordinate 0 only with the pinned field), ivs_stic_collide_lines (per television line over the 167 x 105 object grid) and ivs_stic_collide_edges (the grid lines above and below the plane, or the whole grid for a display-off frame); core/src/stic/stic_frame.c: ivs_stic_frame_deliver takes a non-const ivs_memsys, accumulates hit[8] through ivs_stic_compose_card_row and ORs it at the frame end"
  - "profile fields video.collision_x0_interacts (0), video.collision_border_bit (1), video.collision_offscreen (1) and video.collision_display_off (0), all unknown-default-chosen (C87, D10-VID-10), through the generator into ivs_stic_params"
  - "tests/video/video_collision.c (17 ids: 16 video.collision. and video.judge.collision-truth-table-corrupt), fault COLLISION_NOT_STICKY, the interaction-enable row of video.mob.register-bit-roles observed through the collision registers"
affects: [08-06, 08-07, 08-08, 08-09]

actuals:
  tokens: 29000
  tasks: 3
  commits: 3
plan_head_before: 0912c6e3205fe8a076c769ec7f4698d2bcff47f3
plan_head_after: 25575761a693f6dc0752f1006a564a51e6984e8b

key-decisions:
  - "Collision is evaluated per television line over the whole 167 x 105 object grid, not per plane row: ivs_stic_collide_lines runs on the 16 lines of each card row inside ivs_stic_compose_card_row (where the card on-pixel mask is at hand) and ivs_stic_collide_edges runs on the grid lines no card row owns, so the answer is the same for every video_mob_line_sample, the border bit and the off-screen MOB against MOB bits exist at all, and the plan's 'local uint32_t hit[8] carried through the per-row composition' is exactly that (one extra hit parameter, NULL for the whole-plane entry points that hold a const memory system)"
  - "The object grid has 8 columns left of the plane and none right of it (167 = 8 + 159) and 8 rows above and one below (105 = 8 + 96 + 1): bit 9 can come from the left, top and bottom edges and never from the right edge; the default-pinned.border-bit id holds those bounds"
  - "The own bit of a collision register is never SET by the frame writer, but a CPU write of the own bit is still stored: the write mask stays 03FF (see Deviations 1)"
  - "The machine-level guests of this family zero the seeded stand-in graphics ROM in the instance, so the card layer has no on pixels under the MOBs and the registers equal the oracle's words exactly (the unit scenes use the project's zero ROM)"

key-files:
  created:
    - tests/video/video_collision.c
    - .planning/phases/08-stic-completion-save-state-performance-baselines/08-05-SUMMARY.md
  modified:
    - core/src/memmap/memsys.h
    - core/src/memmap/memsys.c
    - core/src/stic/stic.h
    - core/src/stic/stic_frame.c
    - core/src/stic/stic_mob.c
    - core/src/stic/stic_params.h
    - cmake/IvsSticTable.cmake
    - data/profiles/intv-2609-ntsc-family-p1.json
    - data/expected/profile-fields.ids
    - data/stic/fields-pending.ids
    - data/stic/tests-pending.ids
    - tests/CMakeLists.txt
    - tests/video/video_cases.h
    - tests/video/video_main.c
    - tests/video/video_mob.c
    - data/expected/tests/video.ids
    - data/expected/faults/video.ids
    - docs/stic/reference-notes.md
    - .planning/STATE.md
    - .planning/ROADMAP.md

requirements-completed: []

completed: 2026-11-14
---

# Phase 8 Plan 05: collision registers Summary

**A guest that programs two MOBs with the interaction bit now reads sticky collision bits in the register window of the frame after the one that produced them, and everything the oracle can reach matched on the first run: all sixteen rows of the truth table of derivation section (d), with no disagreement between the core and the frozen oracle. The bits are ORed into the register file at the frame end, with or without a video buffer, through a new internal mutator beside the gate setter; `ivs_stic_frame_deliver` lost its `const`; no hidden state was added (`ivs_stic_state` is still 528 bytes), so all 15 goldens and both digest literal files are byte-identical. The four pinned behaviors are profile fields with tests that flip them, one named fault catches a non-sticky write-back, and the final gates pass under sh, dash and the Ubuntu container with `steps=37/37 controls=371/371 faults=225/225 fault-runs=225 tests-expected=4702 tests-executed=4698 tests-skipped=4 tests-missing=0 profile-keys=150 unknown-fields=43 stic-rules=67 video-frames=26 goldens=15`. One rule of the lineage is recorded and not applied (a CPU write of a MOB's own bit is still stored, see Deviations 1). VID-05 stays Pending (it closes in plan 08-08).**

## Commits

| Commit | Task | What |
|--------|------|------|
| `af1dfaf` | 1 (tracer) | non-const deliver, `ivs_memsys_stic_collision_or_internal`, the act flag and `ivs_stic_collide_lines` (MOB against MOB), the hit array through `ivs_stic_compose_card_row`, ids `video.collision.mob-mob-bits`, `or-at-frame-end`, `buffer-independent`, the exemption of the interaction-enable bit removed from the register sweep (the collision registers are the second observation channel), two fault-inventory prefixes |
| `0f7f865` | 2 | background bit 8, border bit 9 and the off-screen bits over the object grid (`ivs_stic_collide_edges`), the four profile fields with the generator, nine ids (truth table, background bit, own bit, interaction bit, visibility, sticky, CPU write, read, slicing) |
| `2557576` | 3 | the four `default-pinned` ids, the truth-table judge, `COLLISION_NOT_STICKY`, the reference notes |
| the commit that adds this file | | this SUMMARY, STATE.md and ROADMAP.md |

## The seam, the mutator and the hit accumulation

- **Seam.** `ivs_stic_frame_deliver(const ivs_stic_params *, ivs_stic_state *, ivs_memsys *mem, ...)`: the `const` is gone from the memory parameter in `stic_frame.c` and `stic.h`. Its only machine caller, the FRAME_END case of `stic_events.c`, already held a non-const pointer, so that file is untouched. The guard that returns before `mem` is used for a geometry the working buffer cannot hold is kept (`video.buffers.unrenderable-geometry-overflows` still passes `NULL`, so `video_buffers.c` needed no edit).
- **Mutator.** `ivs_memsys_stic_collision_or_internal(ivs_memsys *m, uint32_t k, uint32_t bits)` beside `ivs_memsys_set_gate_internal`: ORs `bits & 03FF` into `stic_reg[0x18 + k]` for k below 8, no gate test (the register window is closed at a frame end), no counter, no access record; `IVS_MEMSYS_STIC_COLLISION_FIRST` and `_COUNT` are the two constants. It is the frame writer's path and, beside the CPU store, the only writer of the collision registers.
- **Accumulation.** `ivs_stic_frame_deliver` keeps a local `uint32_t hit[8]` (bit j of hit[i]: MOB i and MOB j share a position; bit 8 a set MOB pixel on an on pixel of the card layer; bit 9 a set MOB pixel in the object grid and outside the plane). `ivs_stic_compose_card_row` gained a `hit` parameter; for each card row with an act MOB it computes the card on-pixel mask (the pure `ivs_stic_cards_on_row` of plan 08-04) and calls `ivs_stic_collide_lines` on the 16 television lines of the row. After the card rows, `ivs_stic_collide_edges` covers the grid lines above and below the plane (or the whole grid for a display-off frame when `video_collision_display_off` asks for it). The OR into the register file comes after the whole frame, before any sink matters, so `VIDEO_STATE_DEPENDS_ON_BUFFER` (which returns early without a buffer) is detected by the new ids too. No static or global state, no allocation; every loop is bounded by 8 MOBs, 8 pattern columns, 16 positions per MOB row, 192 grid columns and at most 1,024 grid lines.
- **One decode.** The overlay and the collision evaluation share `ivs_stic_mob_line_byte` (the Y flip, the pair of cards of a tall MOB, the card number), so they cannot disagree about where a pattern pixel is; the overlay was refactored onto it and every F-M frame, the 48-row sweep and the three pinned sampling cases still match.

## What was built

- **Profile fields.** `video.collision_x0_interacts` 0, `video.collision_border_bit` 1, `video.collision_offscreen` 1 and `video.collision_display_off` 0 (unknown-default-chosen, C87, `measurement: D10-VID-10`), read by `cmake/IvsSticTable.cmake` (each 0 or 1, fail closed) into `video_collision_x0_interacts`, `video_collision_border_bit`, `video_collision_offscreen` and `video_collision_display_off` of `ivs_stic_params`; the four names are in `data/expected/profile-fields.ids` and their pending lines are deleted in the same commit (profile-keys 146 to 150, unknown-fields 39 to 43). The `ivs_stic_mob` struct gained a one-byte `act` member (12 to 14 bytes).
- **Behavior.** The derivable core of rule STIC-R44 (MOB against MOB, MOB against a set background pixel: card foreground and Colored Squares colors 0 to 6, not 7; interaction enable required on the colliding MOBs; the visible bit not required; the own bit never set; sticky across frames; evaluated per television line) and the pinned remainder of STIC-R45 (X coordinate 0, the border bit, off-screen MOB against MOB, display off), the last four being fields. Collision read and clear semantics are held by absence: no read path changed (`ivs_memsys_read` and the bus are untouched).
- **Tests.** `tests/video/video_collision.c` (about 1,100 lines): the words of the three collision scenes (A7, scenes 7, 8 and 9) and the cards and BACKTAB of A2 and A3 are typed in the test source, the truth table is transcribed row by row from section (d) with the section named, and every other shape is derived by hand in the test text. Ids: Task 1 `mob-mob-bits`, `or-at-frame-end` (a stand-in guest: frame 0 reads C0 and C1 inside the frame it describes and gets the cleared registers, the register file holds the bits right after the frame end, the window reads of frame 1 return FC02 and FC01, a CPU write of 0 clears and frame 1 shows no overlap), `buffer-independent` (the frame writer with a buffer, the counting sink and no sink, whole memory-system memcmp, and the machine with three forms of the video channel, whole instance memcmp); Task 2 `truth-table` (16 rows, each from a fresh scene), `mob-background-bit8` (eleven rows including a one-pixel overlap at (84, 52) and none at (85, 52), colors 0, 3 and 7), `own-bit-never-set` (eight MOBs on the same pixels: every register is the seven other bits and bit 8), `intr-required`, `visibility-not-required` (also: an invisible MOB draws nothing), `sticky-across-frames` (a frame with no overlap, a frame with the interaction bits cleared, a second overlap ORed with the first, the CPU's write of 0), `cpu-write-replaces`, `read-no-effect` (see Deviations 2), `slice-invariant` (calls of 1, 7, 468 and 97 ticks, whole-instance memcmp against one call); Task 3 `default-pinned.x0`, `.border-bit`, `.offscreen`, `.display-off` and the judge.
- **Register sweep.** The sweep of `video.mob.register-bit-roles` now observes a change of the plane or of any collision register `$0018` to `$001F` after a delivered frame, and its interaction-enable row asks for a change of the collision registers and none of the plane in a new two-MOB base state (MOB 0 without the interaction bit over MOB 1 with it); the id passes, which is the proof that the role is real and not exempt.

## Disagreements between the core and the oracle

None for any value the oracle asserts. All sixteen rows of the truth table (T1 to T16) and the three register words of the program's reads matched on the first run of the finished code, and no oracle file was edited (`git diff --quiet 9250265 -- docs/video/diag-stic-2-derivation.md data/video/frames` exits 0 on every commit of this plan).

One statement of the rule layer is not met by the core, and it is not an oracle value: rule STIC-R44, its note, the C87 default policy and the pinned row PD-7 say the own bit of a collision register reads 0 whatever the CPU writes. The core never SETS the own bit, but a CPU write is stored under the unchanged write mask 03FF, so a guest that writes the own bit reads it back. Reported here, not fixed in the oracle; see Deviations 1.

## Stack budget (T-08-13, the stack-probe rule)

`-fstack-usage` of the clang in this checkout, at -O0 (the dev preset) and -O2: `ivs_stic_frame_deliver` 2,448 and 2,416 bytes (2,416 and 2,368 before this plan: +32 for the local hit array at -O0), `ivs_stic_compose_card_row` 480 and 416, `ivs_stic_collide_lines` 400 and 384, `ivs_stic_collide_edges` 208 and 208, `ivs_stic_overlay_mobs_row` 160 and 144, `ivs_stic_mob_line_byte` 64 at -O0. The deepest chain `deliver -> compose -> collide_lines` is 3,328 bytes at -O0 and 3,216 at -O2, under the 4,096-byte probe threshold (the `core-symbols` step is green on both platforms, core-imports 5 on macOS and 3 on Linux, no stack probe). The plan-08-06 warning of plan 08-04 stands: about 770 bytes of headroom remain at -O0 for a delay carry, so a carry buffer must not live in the deliver frame.

## Fault-inventory upkeep (rule fault-wrong-reason)

Each addition is appended to column 3 (allowed prefixes); no must-fail or must-pass column was edited.

| Task | Fault line | Added | Failing id that demanded it |
|------|------------|-------|-----------------------------|
| 1 | STIC_EVENT_ORDER_START_BEFORE_END | `video.collision.` | or-at-frame-end (a displayed frame delivered with the flag clear evaluates no collision) |
| 1 | VIDEO_STATE_DEPENDS_ON_BUFFER | `video.collision.` and `video.judge.` | buffer-independent, mob-mob-bits, or-at-frame-end (no OR without a buffer); `video.judge.mob-role-swapped` (its control sweep observes the collision registers through a frame with no sink) |
| 2 | none | | the run demanded nothing |
| 3 | COLLISION_NOT_STICKY (new line) | `video.collision.` and `video.judge.` | under the fault five ids fail: sticky-across-frames (the must-fail id), or-at-frame-end, truth-table, cpu-write-replaces and `video.judge.collision-truth-table-corrupt` |

The plan predicted more additions (VIDEO_FRAME_AT_CALL_END, the card faults, MOB_PRIORITY_SWAP and MOB_FLIP_SWAPPED, VIDEO_20TH_COLUMN_WIDTH); the runs did not demand them and none was added.

## Checks shown able to fail

- Ad hoc defects in `stic_mob.c`, each restored afterwards: requiring the visible bit for `act` fails `truth-table` and `visibility-not-required`; dropping the interaction-bit requirement fails `register-bit-roles` (its interaction row), `video.judge.mob-role-swapped`, `truth-table`, `mob-background-bit8` and `intr-required`; setting the own bit fails ten ids (`own-bit-never-set` among them).
- The named fault `COLLISION_NOT_STICKY` fails exactly the five ids above and leaves `video.geom.profile-derives-159x96` and the other 132 passing.
- `video.judge.collision-truth-table-corrupt` changes one expected word of a copy of the table at four rows (T1, T7, T12, T16) and requires the comparison to report exactly one disagreement each time, with the true table reporting none.
- `read-no-effect` flips the existing `stic.mode_select_addrs` list (and adds a read row of `$0018` to `stic.access_effects`) in a stack copy of the parameters and requires the same read to switch the mode, so a read effect would be seen. The four `default-pinned` ids assert the default and require the flipped field to change the result.
- Non-vacuity: `or-at-frame-end` asserts that the in-frame reads returned the cleared register (FC00), not a blocked read (FFFF); `slice-invariant` asserts that the colliding run left 0002 and 0001 in C0 and C1; `buffer-independent` asserts the bits before comparing the three forms.

## Deviations from Plan

**1. [Conflict, recorded and not applied] The own bit of a CPU write.** The 08-03 summary, the note of rule STIC-R44, the C87 policy and the hand-off of the orchestrator say plan 08-05 changes the stored view of the collision rows so that the write mask of register n loses bit n. The plan text says the opposite twice (a CPU write replaces the register under the write mask 03FF; `stic.peek.*` and `stic.reg.collision-readback` pass unchanged), and `files_modified` lists neither the regmap literal nor the profile table nor any Phase 6 test. Dropping the own bit changes the independent Phase 6 literal (`data/stic/intv-2609-ntsc-family-p1.regmap.literal.txt`, a frozen oracle of its own, with blind second reads), eight `wmask` values of `stic.regmap` in the profile, and at least three existing tests whose expected values write an own bit and read it back (`stic.reg.collision-readback` writes `00A5` to all eight registers, `memmap.width.stic14-registers` writes `00FF` to `$0018`, `stic.alias.read-0040` writes `00A5` to `$0018`). No oracle row depends on it: the program writes only zero to a collision register, and the oracle's row P6 (a nonzero CPU write) is pinned and never asserted. I followed the plan text and its acceptance criteria, kept the Phase 6 evidence untouched, and report the gap: `video.collision.cpu-write-replaces` writes values whose own bit is clear so that it asserts nothing about it. Proposed follow-up (about a dozen lines of data and three expectation edits, one commit): `w=03FE`, `03FD`, `03FB`, `03F7`, `03EF`, `03DF`, `03BF`, `037F` in the literal and the profile, the three tests above computing their expectation from the literal's write mask, and the note of STIC-R44 closed.

**2. [Interpretation] The flip of `read-no-effect` needs an effect row too.** The oracle (PD-7) and the plan say the existing field `stic.mode_select_addrs` extended with a collision address makes the read switch the mode. In the core a READ has an effect only when its address is in that list AND a read row for the register exists in `stic.access_effects` (the only read row is the one of `$0021`). The flip therefore extends both existing fields of the stack copy (the address list and one effect row); no new field.

**3. [Addition] Collision over the whole object grid, not only the card rows.** The plan describes the accumulation through the card rows; the pinned behaviors (border bit, off-screen MOB against MOB, a MOB at X coordinate 0 reaching columns -8..-1) need the television lines above and below the plane, so `ivs_stic_collide_edges` exists and `ivs_stic_collide_lines` works on grid columns -16..175 with the grid bounds of the profile (`video_grid_w`, `video_grid_h`, the object-field origin). The profile's grid has no column right of the plane, which my first draft of the border-bit test got wrong and the run corrected (the expectation was derived by arithmetic from the profile, not read from the run).

**4. [Interpretation] Machine-level guests zero the stand-in graphics ROM.** The machine harness installs a seeded pattern as graphics ROM, so the zero BACKTAB shows card 0 of that ROM and the card layer has on pixels under the MOBs (bit 8 appears in the machine run, correctly). Those tests are about MOB against MOB and time, so they zero `mem.grom` in the instance after `sc_guest_n` (the raw backing, as the harness already pokes graphics RAM); the unit scenes use the project's zero ROM.

**5. [Information] Timing of the stand-in guest.** The first draft of `or-at-frame-end` used a delay of 1,000 iterations and landed its frame-1 strobe at cycle 1,941 of the frame, so its third read hit the closed window (FFFF). A unit trace gave the cause; the delay is 900 iterations (strobe near cycle 440). The test asserts the values read, so a miss cannot pass.

**6. [Addition] Interaction-enable row of the sweep.** The plan asks to delete the exemption marker and extend observability; the sweep also asserts that the plane does NOT change for that row (collision registers only), and `vm_collision_regs` clears and restores the registers around each probe.

No auth gates, no package installs (T-08-SC), no `gh` call, no push, no tag; `.planning/config.json` and `.planning/state.json` were never staged.

## Decisions Made

- The mutator and the two internal evaluation functions are the whole core change; `ivs_stic_state` is still 528 bytes, `ivs_stic_params` grew by 16 bytes (four uint32) and `ivs_stic_mob` by 2, so no digest literal moved and the wave-B snapshot checklist gains no STIC row (the bits are in `stic_reg`, already serialized).
- The truth table is run from a fresh scene per row (the registers cleared by the CPU, scene 7, scene 8, and for T9 to T16 the CPU's write of zero and scene 9), as the program's routines of frames 1 to 4 do; the table in the test source has seven nonzero and nine zero rows and no row for a pinned case.
- A display-off frame with `video_collision_display_off` 1 evaluates MOB against MOB and the border bit over the whole grid with no card layer (a display-off frame has none); with 0 it evaluates nothing.

## Cost (laptop, load average 13 to 17 during the runs)

| Item | Value |
|------|-------|
| gate total, sh, at plan start (full run) | 82 s, gate-cpu user 140.2 sys 121.8 |
| gate total, sh and dash, Task 1 tree | 78 s and 80 s |
| gate total, sh and dash, Task 2 tree | 91 s and 83 s |
| gate total, sh and dash, Task 3 tree | 90 s and 85 s, gate-cpu user 161.0 sys 145.2 (sh), 145.5 and 125.7 (dash) |
| container gate, Task 3 tree | 319 s (gcc 13.3.0, cmake 3.28.3, x86_64), gate-cpu user 1246.5 sys 77.1 |
| `ivs_tests_video`, all 137 ids, dev preset (ASan, UBSan) | 0.37 s for the 120 ids before this plan (08-04 summary), 0.44 to 0.46 s for 137 ids under a load average of 11 and 0.60 s with the container gate running: the 17 ids add about 0.1 s |
| the 225 fault runs | one more run of the video suite (0.4 to 0.6 s) |

The additions are inside the noise of a loaded laptop; `.planning/CI-EFFICIENCY.md` is not edited by this plan (the checkpoint line of plan 08-08 carries the figures).

## Open Items

- **Own bit on a CPU write (Deviations 1).** Needs the owner of the Phase 6 regmap literal: either apply the write-mask change listed there (one small commit) or reword rule STIC-R44, its note, C87 and PD-7 to say the own bit is merely never set by the frame writer. Candidates: plan 08-06 or 08-08. No oracle row, scenario or claim above provisional depends on it.
- **Plan 08-06 (delay and strips).** `ivs_stic_compose_card_row` is still the one place; its collision call uses the television lines of the card row and the grid bounds, so a vertical delay that moves the cards and MOBs together must shift the line argument and the on-pixel mask rows together (and the pinned PD-8 to PD-12 stay as in 08-03). The stack headroom of the deliver chain is about 770 bytes at -O0: no carry buffer in the deliver frame.
- **Plan 08-07 (program and scenarios).** Carried unchanged from plans 08-03 and 08-04: every register read and write of a routine ends before cycle 1,900, the re-arm count N = 500, the scenario lines equal the assert blocks of derivation section (f) line for line. The program of the collision scenario reads C0 to C7 in the routines of frames 3 and 4 and writes only zero; this plan holds the same sequence in process, so a scenario mismatch will be a program or runner problem, not a collision one.
- **Plan 08-08 (claims).** The final ids of the four pinned-default families: this plan registers `video.collision.default-pinned.x0`, `.border-bit`, `.offscreen`, `.display-off`, `video.collision.read-no-effect` and `video.collision.cpu-write-replaces` (the held-by-absence pins PD-7). CL-0026 carries the exclusions of derivation section (h) (no X coordinate 0, edge, off-screen or display-off collision, no read or write semantics beyond sticky bits and a write of 0).
- **Information.** D10 and D11 are not performed and no claim above provisional exists; every expected value of this plan is `source-expectation`, tier 4 lineage, with a blind second read of the same model family (C43).

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=37/37 controls=371/371 ... fault-runs=225 ... faults=225/225 ... profile-keys=150 ... goldens=15 ... fields=150 unknown-fields=43 ... tests-expected=4702 tests-executed=4698 tests-skipped=4 tests-missing=0 ... stic-rules=67 ... video-frames=26 ... (0 FAIL lines; gate-time total=90 s on the Task 3 tree)
dash tools/gate.sh:    the identical summary line (md5 3b648122ebb68ab1a524155b6991495c for both shells; 0 FAIL lines; total=85 s)
GATE_LINUX_PLATFORM= sh tools/gate-linux.sh: exit 0 on the same tree (build/gate-linux.log ends gate: PASS; gcc 13.3.0, cmake 3.28.3, x86_64, cc=GNU, core-imports=3), steps=37/37 controls=371/371 faults=225/225 fault-runs=225 tests-expected=4702 tests-executed=4698 tests-skipped=4 tests-missing=0 profile-keys=150 stic-rules=67 video-frames=26 goldens=15, 0 FAIL lines (gate-time total=319 s)
```

After Task 1 (before its commit): sh and dash `tests-expected=4674 tests-executed=4670 tests-skipped=4 faults=224/224 profile-keys=146 unknown-fields=39 goldens=15` (md5 bbda28a15609e9e301d7824b666c6112 for both shells); the first gate run stopped at the `faults-verdict` step with the five `fault-wrong-reason` findings of the table above and nothing else. After Task 2: `tests-expected=4692 tests-executed=4688 faults=224/224 profile-keys=150 unknown-fields=43` (md5 39c2c615d560f9e17fe56fb942a1319a for both shells; no fault-inventory change). `git diff --quiet` of `data/golden` and both digest literal files against the merge base, and `git diff --quiet 9250265 -- data/video/frames docs/video/diag-stic-2-derivation.md`, exit 0 on the closing tree.

## Known Stubs

None.

## Threat Flags

None new. T-08-15 (hidden state, gate bypass misuse, a read side effect): the bits live only in `stic_reg` (already serialized), the mutator is internal and masked 03FF, `video.collision.read-no-effect` (with the flip that shows the check can fail) and the unchanged `stic.peek.*` ids hold, digest literals and goldens are unchanged, no state member was added. T-08-16 (machine state depending on a buffer or on slicing): the OR comes after the frame and is not conditional on a sink, `video.collision.buffer-independent` (writer with a buffer, counting sink and no sink, and the machine with three channel forms) and `video.collision.slice-invariant` (1, 7, 468, 97) hold, and `VIDEO_STATE_DEPENDS_ON_BUFFER` is detected by the new ids as well. T-08-SC: no installs.

## Self-Check: PASSED

Verified before the final commit: `tests/video/video_collision.c` exists; commits `af1dfaf`, `0f7f865` and `2557576` are ancestors of HEAD and `git rev-list --count 0912c6e3205fe8a076c769ec7f4698d2bcff47f3..HEAD` printed 3 before this file's commit; `.planning/config.json` and `.planning/state.json` are not staged; `git ls-files` lists no `.ivsc`, `.bin`, `.rom`, `.exec`, `.wav`, `.ppm`, `.vraw` or `.ivsn` file; the acceptance greps give 0 deliver declarations with a const memory parameter, 17 ids (`^video.collision.` 16 and `video.judge.collision-` 1) in `data/expected/tests/video.ids`, 0 `video.collision.` and 0 `video.collision_` pending lines, 4 `video.collision_` fields in `data/expected/profile-fields.ids`, 1 `COLLISION_NOT_STICKY` fault line and its site.
