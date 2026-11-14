# Lamp Tone blind second-read request

pending: blind read not yet run

State: this request is written in plan 07-02, the oracle plan of Phase 7, which precedes every implementation plan of the phase. The blinded copy is built and the reader started by the orchestrator after that plan, and no reader output is committed in plan 07-02. Plan 07-06 (the program) starts only when the record named below exists and its first line reads `state: closed`.

## What is to be blind-read (D-23)

One job, one fresh isolated reader, on the program `ivs-demo-lamp-tone-1`. The reader receives only the specification text of `docs/video/demo-lamp-tone-1-derivation.md` between the marker lines `<!-- blind-spec-begin -->` and `<!-- blind-spec-end -->` (section (a): rules A1 to A10), and re-derives from it, without seeing any committed expected value, derivation, literal or listing:

1. The lamp pattern (eight digits per row, 1 unlit, 2 lit, lamp 0 first) of row 4 and row 7 for every frame slot 0 to 12 of the scripted run, or `not displayed` for a frame that has no picture.
2. The RAM words `$02F0` to `$02F7` after the last event that is read before each of the four stops (200000, 440000, 640000 and 806568 ticks), and the number of PSG write-log records delivered at each stop.
3. The complete PSG write log of the run in order: index, register address (the low nibble), value, and the frame of the routine that writes it.
4. The channel levels (A, B, C) of the audio sample stamped at each of these ticks, or the words `not derivable` when the specification does not fix the level: 100000, 140000, 160000, 200000, 500000, 690000, 740000 and 760000.
5. For each pair of port-byte combinations below, the rectangle (x y w h, plane positions) that contains every position where the two delivered planes differ, and the number of differing positions. The pairs are (port 0 byte, port 1 byte) against (port 0 byte, port 1 byte): (FF, FF) against (7E, FF); (FF, FF) against (5A, FF); (7E, FF) against (5A, FF); (FF, FF) against (FF, FD); (FF, FF) against (5F, FD); (FF, FF) against (5A, 9F); (5F, FD) against (5A, 9F).

Two items deserve an explicit second look (S-1 open item 6): the lamp builder (the instruction SLLC sets the carry from bit 15 and ADCR adds the carry to a register, as `docs/cp1610/reference-notes.md` states for the shift and the add-carry rows), and the reading that a lit lamp is a cleared bit (an idle port reads FF and lights nothing). A reader who finds either reading contradicted by the specification or by the notes reports it as a disagreement and says which text decides.

## Inputs of the reader (exhaustive)

1. The blinded copy of the specification, made by the orchestrator with a range `sed` and placed under `build/review/blind/` (gitignored):

```
mkdir -p build/review/blind
sed -n '/^<!-- blind-spec-begin -->$/,/^<!-- blind-spec-end -->$/p' docs/video/demo-lamp-tone-1-derivation.md > build/review/blind/demo-lamp-tone-1-spec.md
```

2. The controller literal `data/input/intv-2609-ntsc-family-p1.literal.txt` and the PSG register-address literal `data/psg/intv-2609-ntsc-family-p1.regmap.literal.txt`.
3. `docs/runner.md` (the scenario grammar, the sample stamping and the assert rules) and `docs/cp1610/reference-notes.md`.

Nothing else may be opened, searched or recalled for a value. Forbidden: the rest of the derivation (sections (b) to (j)), `data/video/`, `data/golden/`, `data/scenarios/`, the fixtures, `.planning/`, `core/`, `runner/`, `tests/` and `tools/`, the git history of any path, the output of any other reader, any emulator, assembler or disassembler, its source or its output, and web searches for expected values. The general reading rules are those of `docs/stic/second-reader-protocol.md` (an isolated reader, the record says `source-expectation, second-read`); the inputs of this job are the ones listed here.

## Comparison and record

The orchestrator diffs the reader's file against sections (c) to (f) of the derivation (the per-frame table, the RAM words, the PSG log, the audio windows and the pair table), resolves each disagreement from the specification and the manuals, and writes the record `docs/video/demo-lamp-tone-1-second-read-record.md`. The record says `source-expectation, second-read` and never `independent-implementation-comparison`, because the same model family reads the same pages and a shared misreading stays possible (C43).

The head of the record is fixed here and tested by command by plan 07-06 before its first edit:

- line 1 is exactly `state: closed` (or `state: open` while any disagreement is unresolved);
- line 2 is `oracle-commit: <40 lower-case hex digits>`, the oracle commit named in the plan 07-02 SUMMARY (the last commit of that plan);
- line 3 is `corrections: <n>`;
- then exactly n lines `correction: <section> <value> <old> -> <new>`, one per change that the reading forced in the derivation, a frame literal or the pair inventory;
- then the prose (date, reader label, what was read, how many items agree, each disagreement with its resolution).

A correction to the derivation, to a frame literal or to `data/expected/video-frame-pairs.ids` is committed in the same commit that adds the record, and nowhere else. That commit is the frozen oracle baseline for every later plan: from then on sections (a) to (j) and the six literals F-S1 to F-S6 change only by an appended section (k) or by bare claim tags (the waiver tag and the claim-reference tag), which plans 07-06, 07-07 and 07-12 check by command. If the record is `state: open`, plan 07-06 halts.

## Sources and rights

The reader uses no source other than the inputs above; the two literals are tier 4 transcriptions already in the repository, and the specification is project-authored. Nothing in this request or in the record reproduces text of a datasheet, a manual or a catalog.
