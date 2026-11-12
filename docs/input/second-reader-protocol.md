# Controller literal second-reader protocol

This protocol is for an isolated reader that re-derives the rows of the controller literal `data/input/intv-2609-ntsc-family-p1.literal.txt` from the cached source copies, and for the orchestrator that starts the reader.
It holds no row and no expected value.
The sources are two community pages, so the reading is a transcription cross-check and never evidence about hardware (C16).

## Purpose

An isolated reader derives, for each control name of its blinded copy, which connector pins the control grounds, the bit mask those pins clear and the byte a read returns with only that control pressed, from the source copies alone and without seeing the committed rows.
The orchestrator diffs the result with the committed literal row by row; a disagreement is resolved from the source, written in the literal's note and counted.
The record of the reading says `source-expectation, second-read` and never `independent-implementation-comparison`, because the same model family reads the same pages and a shared misreading stays possible.

## Allowed inputs

This list is exhaustive.
1. The reader's own blinded copy `build/review/blind/input-literal.txt`: the control names, one per line, and the row grammar without any value.
2. The cached source copies under the gitignored `refs/ctrl/`: the saved connector-pin page (`cm.html`) and its text rendering (`cm.txt`), the saved controller wiki page (`wiki.html`) and its key-table image (`img.png`); a page that is missing may be fetched again into `refs/ctrl/` from the address in the source registry entry named in the request document.
3. This protocol.

Nothing else may be opened, searched or recalled for a value.

## Forbidden inputs

- Everything under `.planning/`, `data/`, `core/`, `runner/`, `tests/`, `cmake/` and `tools/`, in particular the literal itself, the profile, any generated table and any planning note.
- `docs/input/controller-notes.md`.
- The output of any other reader and the git history of any path.
- Any emulator, assembler or disassembler, its source or its output, including other repositories on the machine.
- Web searches for expected values.

## Output

The reader writes exactly one file, `build/review/derived/input-literal.txt`, once, at the end of its work: one line per control name of the blinded copy, in the order of the blinded copy, in the form `<NAME> pins=<n,n> cleared=<HH> byte=<HH> basis=<one line naming the page, table row or image cell used>`, then a final line `disc-orientation: <one sentence naming where the compass names come from>`.
Pins are listed in ascending order, masks and bytes are two uppercase hex digits.
The reader reports only that it finished and how many lines it wrote.

## Derivation rules

- The pin set of a control comes from the pin table of the saved page; the bit of each pin comes from the pin-to-bit map printed on the same page; the mask is the OR of the bits; the byte is FF XOR the mask.
- The value printed in the image for the same control is read separately, and a difference between the two readings is written in `basis` with both values.
- An ambiguous label is written down in `basis` with the reading chosen and the alternative, never decided silently.
- The compass names of the disc directions are matched to the numbered directions from the image; the order assumed is stated in the last line.

## Orchestrator procedure

1. Plan 05-01 is complete and the gate is green.
2. Build the blinded copy: the header grammar of the literal without its rows, then one line per name of `data/expected/input-bits.ids` without the prefix; write it to `build/review/blind/input-literal.txt`.
3. Spawn one fresh subagent with no prior conversation whose prompt contains only the blinded copy path, this protocol's path and the instruction to follow it exactly; pass no committed row, plan text, SUMMARY text, CONTEXT text or other readers' output.
4. Check that the output exists and has 32 lines; replace a failed reader with a new fresh subagent, never by resuming it.
5. Diff the derived lines against the committed rows, resolve each disagreement from the source copies, record the count in `docs/input/second-read-record.md` and never edit the derived file afterwards.
