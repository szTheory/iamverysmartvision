---
phase: 08-stic-completion-save-state-performance-baselines
plan: 03
subsystem: Phase 8 wave A, plan 1 - the VID-05 oracle before any code - extended frame grammar (unknown regions, mob records, hash none), MOB bit-role literal, rules STIC-R37..R50, discrepancies C107..C110, owner hand-offs D10-VID-10..14, the derivation of ivs-diag-stic-2 with ten hand-derived frames, two pairs and the blind-read request
tags: [A, VID-05, oracle, frame-grammar, mob-literal, D-01, D-02, D-03, D-05, D-08, D-09, D-10, T-08-09, T-08-10, T-08-11]
status: complete

requires:
  - phase: 08-stic-completion-save-state-performance-baselines
    provides: "plan 08-02 end gate: steps=37/37 controls=367/367 faults=222/222 tests-expected=4628 tests-executed=4624 tests-skipped=4 tests-missing=0 stic-rules=53 video-frames=16 video-frame-pairs=7 video-hash-ties=11 discrepancies=106 open=97 pending-resolvers=21 d10-items=42 (re-measured at the start of this plan, identical); HEAD 3b15805 at the start"
provides:
  - "tools/check/video.cmake: the frame grammar records `unknown <x> <y> <w> <h>` and `mob <i> <x> <y> <w> <h> color=<hex> pixels=<n>`, `sha256-text none` exactly when a frame has an unknown record, the cell rules skipping the cells that those rectangles meet, the refusal of a pair or an expect hash on a frame with an unknown region, pending owners 0[68]-NN, and one shared row-grammar function for the card and the MOB bit-role literals"
  - "data/video/mob.literal.txt: 48 rows, one per (register, bit) of mobx, moby and moba, tier4-community, source SRC-community-emu-docs"
  - "data/stic/rules.json: STIC-R37..R50 (stic-rules=67); data/discrepancies.json: C107..C110 and updates of C81, C87, C92, C105 (discrepancies=110 open=101 pending-resolvers=20); data/expected/d10-measurements.ids: D10-VID-10..14 (d10-items=47), none performed"
  - "docs/video/diag-stic-2-derivation.md (blind-spec block, arithmetic, state tables, collision truth table of 16 rows and 6 pinned rows, six assert blocks, six mutants, claim text, pinned-default table PD-1..PD-13), data/video/frames/F-M0..F-M9.frame (video-frames=26), two pair lines (video-frame-pairs=9), docs/video/diag-stic-2-second-read-request.md (pending)"
affects: [08-04, 08-05, 08-06, 08-07, 08-08]

actuals:
  tokens: 95190
  tasks: 3
  commits: 5
plan_head_before: 3b158058280b3bb80a3e94363c577c0883bac3d1
plan_head_after: 641a12e3a1059b242cc88a3ee4308e3f58d9b36d

key-decisions:
  - "The oracle values are derived by hand and expanded by a throwaway script written from the rules (never committed, never reading the core); every count was derived by hand first (38, 36, 144, 244, 72, 2,012, 32) and the script agreed on all"
  - "The delayed frames F-M5 to F-M7 pass the unchanged cell rules because the base card is a 5 x 5 block in its cell corner (its set pixels stay in their own cell for every delay of at most 3) and the Colored Squares cards, which would break the cell rules under a shift, exist only in the collision scenario, which has no delay; no grammar change beyond the plan was needed"
  - "F-M4, F-M5 and F-M6 are hashed (the plan allows only F-M7 without a hash), so they are derived under the defaults of PD-8 (top strip of 8 plane rows) and PD-9 (strips fixed to the display); the dependence is stated in the pinned-default table, the claim exclusions and the open items, and every pixel probe of the scenarios avoids the positions that either pin moves"
  - "The collision truth table uses solid cards and a read-out in the routines of frames 3 and 4 (sticky bits read after a frame in which the MOBs no longer overlap, then cleared by a CPU write of zero), so no row depends on a read side effect, a nonzero CPU write, X coordinate 0, the border bit, a position outside the plane or a display-off frame"
  - "The MOB literal and the cards literal share one row-grammar function; the MOB check is proven able to fail by five ad hoc probes (the plan adds no control for it)"

key-files:
  created:
    - data/video/mob.literal.txt
    - data/video/frames/F-M0.frame
    - data/video/frames/F-M1.frame
    - data/video/frames/F-M2.frame
    - data/video/frames/F-M3.frame
    - data/video/frames/F-M4.frame
    - data/video/frames/F-M5.frame
    - data/video/frames/F-M6.frame
    - data/video/frames/F-M7.frame
    - data/video/frames/F-M8.frame
    - data/video/frames/F-M9.frame
    - docs/video/diag-stic-2-derivation.md
    - docs/video/diag-stic-2-second-read-request.md
    - tools/check/controls/_good/tree/data/video/frames/T-4.frame
    - tools/check/controls/video-frame-unknown-outside-region/
    - tools/check/controls/video-frame-mob-count/
    - tools/check/controls/video-frame-hash-none-without-unknown/
    - tools/check/controls/video-frame-pair-unknown/
    - .planning/phases/08-stic-completion-save-state-performance-baselines/08-03-SUMMARY.md
  modified:
    - tools/check/video.cmake
    - data/stic/rules.json
    - data/expected/stic-rules.ids
    - data/stic/tests-pending.ids
    - data/stic/fields-pending.ids
    - data/discrepancies.json
    - data/expected/discrepancies.ids
    - data/expected/d10-measurements.ids
    - data/expected/controls/video.ids
    - data/expected/video-frames.ids
    - data/expected/video-frame-pairs.ids
    - tools/check/controls/_good/tree/data/expected/video-frames.ids
    - docs/provenance/CONSULTATIONS.md
    - docs/stic/d10-procedure.md
    - .planning/STATE.md
    - .planning/ROADMAP.md

requirements-completed: []

completed: 2026-11-14
---

# Phase 8 Plan 03: the VID-05 oracle Summary

**The expected values for moving objects, collision, delay and border extension exist before any renderer, program or scenario: a frame grammar that can hold both a MOB overlay and a declared-unknown cell and refuses every way of smuggling an unknown into evidence, the raw sources re-read and logged, fourteen rules and four new discrepancies that name every pinned default with its field, owner hand-off and claim exclusion, a derivation with a blind-spec block, ten hand-derived frames, a collision truth table, two declared pairs and six assert blocks tied to frame hashes. The final gates pass under sh, dash and the Ubuntu container with `steps=37/37 controls=371/371 faults=222/222 tests-expected=4628 tests-executed=4624 tests-skipped=4 tests-missing=0 stic-rules=67 video-frames=26 video-frame-pairs=9 video-hash-ties=11 discrepancies=110 open=101 pending-resolvers=20 d10-items=47`. No file under `core/`, `runner/`, `tests/`, `fixtures/` or `data/scenarios/` changed. VID-05 stays Pending (it closes in plan 08-08). The blind second read is the orchestrator's next step, before plan 08-04.**

## Commits

| Commit | Task | What |
|--------|------|------|
| `e0c5376` | 1 (tracer) | frame grammar (unknown regions, mob records, `sha256-text none`, cross rules, pending owners `0[68]-NN`), toy frame T-4, four controls (controls 367 to 371) |
| `fee1322` | 2 | sources re-read rows, `data/video/mob.literal.txt` with the shared literal check, rules STIC-R37..R50, pending lines, C107..C110 and the updates of C81, C87, C92, C105, D10-VID-10..14 with their procedure anchors |
| `67829c6` | 3 | the derivation, F-M0..F-M9, two pair lines, the blind-read request |
| `d550a51` | 2 (fix) | the consultation statements corrected (the second row is the only reading for licence determination; the three new rows are documentation readings) |
| `641a12e` | 3 (fix) | rule R38 of the blind spec now states the plane positions of a pattern pixel under zoom (found while re-reading the spec before the freeze); no frame, hash, pair or assert changed. This is the oracle commit: full hash `641a12e3a1059b242cc88a3ee4308e3f58d9b36d`, the one the reader record cites |
| the commit that adds this file | | this SUMMARY, STATE.md and ROADMAP.md |

## Raw documents read (D-01) and what differed from the discussion note

The orchestrator placed the raw text in the gitignored cache `refs/stic/`; nothing from there is tracked. Read in full as plain text: the community STIC register description (`stic.txt`, 24,533 bytes), the interrupt notes (`interrupts.txt`, 32,729 bytes), the later reverse-engineering notes of the same author (`intv88-notes.txt`, 33,211 bytes, the whole text layer including the part the discussion had not seen) and the cached wiki STIC page (`stic-15008.txt`, revision 15008; its five images are BACKTAB word figures, there is no MOB register figure, so no `raw-source-unavailable` halt applied: the register layout is text). One consultation row per document is in `docs/provenance/CONSULTATIONS.md` (documentation text only, nothing copied, no new SRC id; the locators are hyphenated words such as `programming-stic-mob-controls`).

Where the raw text differs from `08-DISCUSS-stic.md` section 2 (the raw text wins; all four are in derivation section (j) item 6):

1. **Own bit.** The discussion said a MOB's own bit is never set. The raw text adds that it also reads 0 when the CPU tries to write it, and that writing bit X of the register of MOB Y does not change bit Y of the register of MOB X. Rule STIC-R44 states both; plan 08-05 must change the stored view of the collision rows for it (the write mask of register n loses bit n); the program only ever writes zero to a collision register.
2. **Blank area colour.** The discussion called the colour of the exposed area unknown (default not displayed). The raw text says the blank area is filled with the background colour as the display mode defines it, which names no single colour index in either mode, so the field stays pinned (PD-10) and the raw sentence is recorded in C110; F-M7 shows the exposed rectangles as unknown.
3. **Strips.** The discussion treated the 8-pixel width and the unit of the top strip as one unknown. The raw text states "8-pixel" for both edges and gives only a drawing for the delay at its maximum; its stated purpose (the blank areas are never visible, for a delay up to 7) is what makes 8 plane rows and a place fixed to the display the default (PD-8, PD-9).
4. **Register-width corroboration.** The later notes list, for one other board, which bits of each register read as 0, 1 or read-write; the widths (X 11, Y 12, attribute 14, collision 10 with the own bit fixed to 0, delay 3, extension 2) match the Phase 6 literal. It is a tier 4 report of one other board by the same author, so it corroborates the widths and proves nothing for the profile. The "bits 15 and 14 read 0" note stays a logged C81 update, not acted on (D-08).

The interrupt notes agree with the discussion on the 13,518 and about 13,572 usable cycles at vertical delay 0 and above 0 (C92 pin, D-07).

## What was built

- **Frame grammar (T-08-10).** `unknown <x> <y> <w> <h>` (non-negative integers, inside the 159 x 96 plane, no overlap between unknown rectangles) and `mob <i> <x> <y> <w> <h> color=<hex> pixels=<n>` (index 0 to 7 once, rectangle inside the plane). A `-` outside every unknown rectangle, or a position inside one that is not `-`, is `video-frame-symbol` (the first position is named). A mob colour must occur in the plane exactly as many times as its records declare and only inside their rectangles (`video-frame-popcount`). `sha256-text none` is accepted exactly when the frame has an unknown record, and a numeric hash with an unknown record is `video-frame-hash-drift`. A pair line, or an `.expect` hash, that names a frame with an unknown region or no hash is refused (`video-frame-pair-region`, `video-frame-hash-expect-drift`). The cell count and uniformity rules skip every cell whose area meets an unknown or mob rectangle; the annotation grammar of a listed cell is still checked (hardening beyond the plan). The 16 real frames, 7 pairs and 11 ties keep their verdict, and all 13 existing video controls give byte-identical findings under the old and the new checker (compared line by line on a toy tree without T-4).
- **MOB literal.** 48 rows, roles for X (position, interaction, visible, double width), Y (position, tall, two size bits, two mirrors) and attribute (colour, card, RAM select, colour bit 3, priority); validated by `check_role_literal`, the function that the card literal now also uses. Proven able to fail by five probes (a missing row, an `ign` bit given a role, a repeated bit, an unknown source, a role outside the set).
- **Rules and registry.** STIC-R37..R50 (derivable: placement, size, mirror, colour and card, MOB over MOB, priority clear, collision bits, delay, strips; pinned `default-unknown` with resolver and trigger: priority set, collision pins, delay pins, strip pins, line sampling). Pending lines for five test families and eleven fields (owners 08-04, 08-05, 08-06). C107 (priority), C108 (half height), C109 (strips), C110 (delay area) and the updates; D10-VID-10..14, none performed.
- **The derivation.** Program `ivs-diag-stic-2` with one selector read (KEY_1..KEY_5 give 7E, BE, DE, 7D, BD), ten scenes (three MOB scenes by frame number, one border, one hdelay, one vdelay, three collision scenes), the register tables, the timing arithmetic (longest routine 1,745 cycles against the 1,900 bound, 1,394 for a straight sequence; run bound 640,000 ticks, 17,096 before a frame end), the 16-row truth table, ten frames and two pairs, six assert blocks (15, 21, 13, 12, 12 and 23 asserts) tied to seven frame hashes, six mutants, the claim text CL-0025..27 with exclusions, the 13 pinned defaults and the rights checklist. The truth table has 16 non-pinned rows (T1 to T16) and 6 `pinned: not asserted` rows (P1 to P6).

## Blind-spec range (for plan 08-04)

SHA-256 of the lines from `<!-- blind-spec-begin -->` to `<!-- blind-spec-end -->` of `docs/video/diag-stic-2-derivation.md` (53 lines, 12,952 bytes, `sed -n '/^<!-- blind-spec-begin -->$/,/^<!-- blind-spec-end -->$/p'`): `b73a15b9288729ed5c053d1570fa0b5ff2ab7a6b58f6e1f15d58d672e09102ed` (the first version in commit `67829c6` had `359e5f1f48309fdf211df4aa0b886749c9997bb07e29b674953788cb3f5143e0`, 12,682 bytes, before the mapping sentence of rule R38 was added; no reader has seen either). The `sha256-text` of each frame is in its frame file and in derivation section (e) (`cmake -E sha256sum` over the canonical text agrees with the script for the nine hashed frames).

## Checks shown able to fail

- The four controls fail with their intended tokens on their own trees (controls 371/371 in the gate) and the positive control is the toy frame T-4 (a MOB overlay and an unknown region, no hash). Ad hoc probes, not controls: an unknown rectangle or a mob outside the plane, two overlapping unknown rectangles, a repeated mob index, a non-`-` symbol inside an unknown rectangle (`position (43,41) inside an unknown rectangle is not -`), an `.expect` hash on a `none` frame (`the frame has no hash`), a pending line `07-01`; and on the real frames in a scratch mirror: one MOB pixel of F-M3 changed (hash drift and `mob colour 5 occurs 243 times in the plane, the mob records declare 244 pixels`) and one unknown position of F-M7 given a colour (symbol finding). The `.expect` refusal has no control because D-10 caps this plan at four.
- Guards exist where the controls point (lesson 7): each of the four tokens is raised in `tools/check/video.cmake` (`grep -c` of the token is 6, 12, 5 and 10).

## Deviations from Plan

**1. [Interpretation] The delayed frames and the cell rules.** The checker's cell rules are aligned to the 8 x 8 grid, so a frame whose cards are shifted by a delay would fail them unless its card content stays inside its own cell. Instead of a sixth grammar construct the oracle keeps every card cell of the delay scenarios shift-tolerant (the base card is a 5 x 5 block in the cell corner) and puts the Colored Squares cards, which cannot be annotated and are uniform full-cell blocks, only into the collision scenario. No grammar change was needed; the constraint is stated in derivation section (e).

**2. [Interpretation] F-M4 to F-M6 depend on two pinned defaults.** The plan names the top-strip unit and strip-versus-delay as pinned and also requires every frame except F-M7 to carry a hash. A frame with an unknown region cannot have a hash, so F-M4 to F-M6 take 8 plane rows and strips fixed to the display. The dependence is carried by PD-8 and PD-9, the claim exclusions and open item 5 of section (j); the scenario probes avoid every position that either pin moves.

**3. [Addition] Hardening of the checker.** The annotation grammar is still checked for a listed cell that a mob or unknown rectangle meets; an `unknown` or `mob` rectangle is bounds-checked; unknown rectangles may not overlap; a mob index may not repeat. The card literal check was refactored into the shared function `check_role_literal` with unchanged messages and verdicts (the card literal has no control, as before).

**4. [Information] No MOB register figure exists.** The plan expected `mob-*.png` figures; the wiki page has none and the orchestrator said so, so the roles come from the register description's text (D-01 still met: raw text, not a summary).

**5. [Addition] The request first line.** The plan's check is `head -1 ... | grep -q '^pending:'`, so the first line of the request is `pending: blind read not yet run` and the title follows (the Lamp Tone request has the title first).

**6. [Information] `docs/stic/d10-procedure.md` header.** The line that named `D10-VID-01..09` now says `D10-VID-01..14`. Two small commits follow the Task 3 commit: the consultation statement correction (no oracle file) and the R38 mapping sentence of the blind spec (the oracle commit named above).

No auth gates, no package installs (T-08-SC), no `gh` call, no push, no tag; `.planning/config.json` and `.planning/state.json` were never staged (D-42).

## Decisions Made

- Scene numbers (marker `$02F3`): idle 0, mob 1 to 3, border 4, hdelay 5, vdelay 6, collision 7 to 9; selector bytes FF, 7E, BE, DE, 7D, BD.
- Frame asserts: idle at frame 5, mob at frames 2, 5, 8, border, hdelay and vdelay at frame 5 (seven hash ties after plan 08-07: 11 + 7 = 18); the collision scenario asserts RAM markers and two pixel probes.
- `capture video 0 10` and one step `run-to 640000 ticks` for all six scenarios; no frame assert names frame 0 or 1 in a scenario with the tick-0 input.
- The collision read-out is in the routines of frames 3 and 4 into `$0300..$030F`; the program writes only zero to a collision register.

## Cost (laptop, user plus sys cpu, load average about 12 during the measurements)

| Item | cpu-s |
|------|-------|
| `video` step on the real tree before (16 frames, new grammar absent) | 1.72 |
| `video` step on the real tree after (26 frames, new grammar), three runs | 2.14 to 2.16 |
| `video` step on the toy tree (3 frames) | 0.18 |

The step runs once on the real tree and 17 times (13 old and 4 new controls) on toy trees, so the plan adds about 0.45 cpu-s for the real tree, about 0.76 for the four new controls and about 0.4 for the extra toy frame in the 13 old ones: about 1.6 local cpu-s, about 2.7 hosted at the factor 1.675, about +0.06 in the median `r`. `.planning/CI-EFFICIENCY.md` is not edited by this plan (it is not in its file list); the figures are for the checkpoint of plan 08-08. Gate-time total 79 s (sh) and 80 s (dash), gate-cpu user 136 sys 119 (laptop under load).

## Open Items

- **Orchestrator: the blind second read.** Run the job of `docs/video/diag-stic-2-second-read-request.md` and commit its record `docs/video/diag-stic-2-second-read-record.md` (first line `state: closed`, `oracle-commit: 641a12e3a1059b242cc88a3ee4308e3f58d9b36d`, `corrections: <n>`) before plan 08-04. The blinded copy is `sed -n '/^<!-- blind-spec-begin -->$/,/^<!-- blind-spec-end -->$/p' docs/video/diag-stic-2-derivation.md > build/review/blind/diag-stic-2-spec.md` (gitignored directory); the reader receives only that file, `data/video/mob.literal.txt`, the register-address literal, the controller literal, `docs/runner.md` and `docs/cp1610/reference-notes.md`. A correction to the derivation, a frame or a pair goes in the same commit as the record.
- **Plan 08-04**: delete `video.mob. 08-04` and the three fields `video.mob_field_origin`, `video.mob_line_sample`, `video.mob_prio_hides` from the pending lists in the commit that registers the first id and adds the field; reword STIC-R28 in the commit that changes the behaviour; widen the frame-include glob of `tests/CMakeLists.txt` for `F-M*`; the card row buffer must hold the vertical-delay band (`IVS_STIC_CARDROW_MAX`).
- **Plan 08-05**: the stored view of the collision rows loses the own bit (R44 note); the FRAME_END mutator; delete its pending lines. **Plan 08-06**: delete its lines; the exposed-area and strip pins.
- **Plan 08-07**: the program must keep every register read and write of a routine before cycle 1,900 (reference sequence 1,745, straight sequence 1,394), the re-arm delay N = 500 is the reference, the scenario lines equal the assert blocks of section (f) line for line, and `data/golden/*.expect` carry the `# frame:` ties; the `fails=` sets are filled there.
- **Plan 08-08**: write CL-0025..CL-0027 (claim words need the claim tag, not the waiver, once the ids exist), the final test ids of the pinned-default families (the ids of section (i) are proposals), the important-gates rows and the regenerated CHANGELOG claim table.
- **Information**: F-M4 to F-M6 depend on PD-8 and PD-9; F-M8 and F-M9 on PD-2; the reading that a strip hides a MOB is open item 4 of section (j) for the blind reader. D10 and D11 are not performed and no claim above provisional exists.

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=37/37 controls=371/371 faults=222/222 ... stic-rules=67 ... discrepancies=110 pending-resolvers=20 open=101 d10-items=47 ... tracked=2357 ... tests-expected=4628 tests-executed=4624 tests-skipped=4 tests-missing=0 docs=53 claim-refs=16 ... video-frames=26 video-hash-ties=11 video-frame-pairs=9 (0 FAIL lines; gate-time total=79 s, on the Task 3 tree)
dash tools/gate.sh:    the identical summary line (md5 c53394dc551739574034c802d4b96288 for both shells; 0 FAIL lines; total=80 s)
sh tools/gate-linux.sh: exit 0 on the tree of commit 67829c6 (the derivation and frames; the later commits change two docs and the SUMMARY; build/gate-linux.log ends gate: PASS; gcc 13.3.0, cmake 3.28.3, x86_64 emulated), steps=37/37 controls=371/371 faults=222/222 stic-rules=67 video-frames=26 video-frame-pairs=9 video-hash-ties=11 discrepancies=110 open=101 pending-resolvers=20 d10-items=47 tests-expected=4628 tests-executed=4624 tests-skipped=4 tests-missing=0 docs=53, 0 FAIL lines (gate-time total=340 s with a local gate running at the same time; also exit 0 after Task 1, 304 s)
```

On the closing tree (this SUMMARY, STATE.md and ROADMAP.md added or edited, the two docs commits in; `tracked=2358`): sh (89 s) and dash (93 s) `gate: PASS` with the identical summary line (md5 bdf1c3f8fc0dc1bbd0e20c46cb15c4db), 0 `FAIL` lines, run while the container gate ran in parallel (gate-cpu user 146 sys 130 under sh, load average 14 to 18); on the tree of `67829c6` (`tracked=2357`) the identical line had md5 c53394dc551739574034c802d4b96288 under both shells.

After Task 1 (before its commit): sh and dash `controls=371/371 stic-rules=53 video-frames=16 video-frame-pairs=7 video-hash-ties=11`, container exit 0 (gcc 13, x86_64 emulated, 0 FAIL lines, total 304 s). After Task 2: sh and dash `controls=371/371 stic-rules=67 discrepancies=110 open=101 pending-resolvers=20 d10-items=47 video-frames=16`. After Task 3: `video-frames=26 video-frame-pairs=9`. Counts at plan start were `steps=37/37 controls=367/367 stic-rules=53 video-frames=16 video-frame-pairs=7 video-hash-ties=11 discrepancies=106 open=97 pending-resolvers=21 d10-items=42`; no test id and no fault was added. `git diff --quiet 3b15805 -- core runner tests fixtures data/scenarios` exits 0.

## Known Stubs

None. The line under section (k) of the derivation ("Appended by plan 08-07 after the blind-read record; empty here") is a deliberate placeholder named by the plan.

## Threat Flags

None new. T-08-09 (shared misreading, a pinned default read as confirmed): raw re-read before freezing, a blind second read pending, `-` cells and `sha256-text none` keep unknowns out of evidence, every pinned item has a discrepancy, a field line, a D10 line and a claim exclusion. T-08-10 (grammar weakens existing checks): four controls, 13 existing controls identical, the cell rules still apply outside declared regions. T-08-11 (text, figures and names in tracked files): facts restated in project words, roles in a project literal with SRC ids, names only in the consultation log; the acceptance grep for emulator names is 0. T-08-SC: no installs.

## Self-Check: PASSED

Verified before the final commit: every file listed under `key-files.created` exists (the ten frames, the literal, the two docs, the toy frame and the four control directories); commits `e0c5376`, `fee1322`, `67829c6`, `d550a51` and `641a12e` are ancestors of HEAD and `git rev-list --count 3b158058280b3bb80a3e94363c577c0883bac3d1..HEAD` printed 5 before this file's commit; `.planning/config.json` and `.planning/state.json` are not staged; `git ls-files` lists no `.ivsc`, `.bin`, `.rom`, `.exec`, `.wav`, `.ppm` or `.vraw` file.
