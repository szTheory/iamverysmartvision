pending: blind read not yet run

# Diagnostic STIC 2 blind second-read request

State: this request is written in plan 08-03, the oracle plan of the STIC completion (VID-05), which precedes every implementation plan of the wave. The blinded copy is built and the reader started by the orchestrator after that plan, and no reader output is committed in plan 08-03. Plan 08-04 (the MOB renderer) starts only when the record named below exists and its first line reads `state: closed`. From that commit on the oracle sections of `docs/video/diag-stic-2-derivation.md` and the frame literals F-M0 to F-M9 change only by an appended note in section (k) of the derivation or by the `correction:` lines of the record, never by editing a frame or an assert to match a run.

## What is to be blind-read (D-05)

One job, one fresh isolated reader, on the program `ivs-diag-stic-2`. The reader receives only the specification text of `docs/video/diag-stic-2-derivation.md` between the marker lines `<!-- blind-spec-begin -->` and `<!-- blind-spec-end -->` (section (a): the rules A1 to A11 with the hardware rules and the list of pinned items), and re-derives from it, without seeing any committed expected value, derivation, literal or listing:

1. For each of the six scenarios, the color index (0 to 15) at each probe position listed here, at the frame slots listed here. Positions are plane positions (x, y). `idle`, frame 9: (0, 0), (80, 48), (84, 52), (85, 52), (158, 95). `mob`, frame 2: (150, 90), (152, 90), (158, 94), (158, 95), (149, 95), (80, 48); frame 5: (38, 24), (36, 24), (24, 38), (64, 24), (64, 38), (66, 38), (118, 38); frame 8: (20, 20), (24, 28), (24, 36), (39, 43), (27, 51), (76, 48), (83, 49), (84, 52). `border`, frame 5: (0, 0), (100, 3), (7, 50), (8, 8), (12, 9), (8, 12), (18, 10), (8, 50). `hdelay`, frame 5: (63, 40), (65, 40), (83, 48), (155, 50), (100, 60). `vdelay`, frame 5: (60, 43), (62, 43), (80, 48), (80, 51), (150, 90). `collision`, frame 5: (108, 52), (132, 52).
2. For the scenario `collision`, the 16 words `reg AND 03FF` stored at `$0300` to `$030F` (the routine of frame 3 stores `$0300` to `$0307`, the routine of frame 4 stores `$0308` to `$030F`), one value per word, each with the rule that decides it.
3. For the pairs of pictures (a) the scenario `idle` against the scenario `mob` at frame 2 and (b) the scenario `idle` against the scenario `border` at frame 5: the smallest rectangle (x y w h, plane positions) that contains every position where the two delivered planes differ, and the number of differing positions.
4. For each pinned item PD-1 to PD-13 of the specification, the words `pinned, not derivable` when the specification does not fix the answer. A reader who derives a value for a pinned item reports it as a disagreement and says which text decides. The questions are: (PD-1) the color at a position where a MOB with the priority bit set lies over a card pixel that is set; (PD-2) the plane rows shown by a MOB at Y size code `00`; (PD-3) the collision value of a MOB with X coordinate 0 over a set card pixel; (PD-4) bit 9 of a collision register for a MOB that straddles the plane edge; (PD-5) a collision of two MOBs that overlap outside the plane; (PD-6) a collision bit set in a frame with the display off; (PD-7) the value of a collision register after it was read once, and after a CPU write of `0003`; (PD-8) the number of plane rows of the top strip; (PD-9) the columns of the left strip when the horizontal delay is 3; (PD-10) the color at x = 0 when the horizontal delay is 3 and no strip is set; (PD-11) the color of a MOB pixel in an exposed column; (PD-12) the delay value at which the register stops following the written value; (PD-13) whether the bus windows of a frame depend on the vertical delay.
5. The plane position of the top left set pixel of card 2 (without delay at (80, 48)) for the horizontal delay h = 1, 2, 3 (vertical delay 0) and for the vertical delay v = 1, 2, 3 (horizontal delay 0).

Three readings deserve an explicit second look (derivation section (j)): the choice that a strip hides a MOB under it, the reading of the Y size code and of the object-field offset of 8 through the MOB of the `mob` scenario at frame 2 (X = 158 and Y = 98), and the reading that sticky collision bits survive a frame in which the MOBs no longer overlap. A reader who finds a reading contradicted by the specification reports it as a disagreement and says which text decides.

## Inputs of the reader (exhaustive)

1. The blinded copy of the specification, made by the orchestrator with a range `sed` and placed under `build/review/blind/` (gitignored):

```
mkdir -p build/review/blind
sed -n '/^<!-- blind-spec-begin -->$/,/^<!-- blind-spec-end -->$/p' docs/video/diag-stic-2-derivation.md > build/review/blind/diag-stic-2-spec.md
```

2. The MOB register bit-role literal `data/video/mob.literal.txt`, the register-address literal `data/stic/intv-2609-ntsc-family-p1.regmap.literal.txt` and the controller literal `data/input/intv-2609-ntsc-family-p1.literal.txt`.
3. `docs/runner.md` (the scenario grammar, the frame stamping and the assert rules) and `docs/cp1610/reference-notes.md`.

Nothing else may be opened, searched or recalled for a value. Forbidden: the rest of the derivation (sections (b) to (k)), `data/video/frames/`, the card literal and every other file of `data/video/`, `data/golden/`, `data/scenarios/`, the fixtures, `.planning/`, `core/`, `runner/`, `tests/` and `tools/`, the git history of any path, the output of any other reader, any emulator, assembler or disassembler, its source or its output, and web searches for expected values. The general reading rules are those of `docs/stic/second-reader-protocol.md` (an isolated reader, the record says `source-expectation, second-read`); the inputs of this job are the ones listed here.

## Comparison and record

The orchestrator diffs the reader's file against sections (b) to (e) of the derivation (the per-frame arithmetic, the state tables, the truth table, the frames and pairs), resolves each disagreement from the specification and the rules, and writes the record `docs/video/diag-stic-2-second-read-record.md`. The record says `source-expectation, second-read` and never `independent-implementation-comparison`, because the same model family reads the same sources and a shared misreading stays possible (C43).

The head of the record is fixed here and tested by command by plan 08-04 before its first edit:

- line 1 is exactly `state: closed` (or `state: open` while any disagreement is unresolved);
- line 2 is `oracle-commit: <40 lower-case hex digits>`, the oracle commit named in the plan 08-03 SUMMARY (the last commit of the oracle files of that plan);
- line 3 is `corrections: <n>`;
- then exactly n lines `correction: <section> <value> <old> -> <new>`, one per change that the reading forced in the derivation, a frame literal or the pair inventory;
- then the prose (date, reader label, what was read, how many items agree, each disagreement with its resolution).

A correction to the derivation, to a frame literal or to `data/expected/video-frame-pairs.ids` is committed in the same commit that adds the record, and nowhere else. That commit is the frozen oracle baseline for every later plan: from then on sections (a) to (j) and the ten literals F-M0 to F-M9 change only by an appended section (k) or by bare claim tags, which plans 08-04, 08-07 and 08-08 check by command. If the record is `state: open`, plan 08-04 halts.

## Sources and rights

The reader uses no source other than the inputs above; the literals are tier 4 transcriptions already in the repository, and the specification is project-authored. Nothing in this request or in the record reproduces text of a datasheet, a manual, a catalog or a documentation page.
