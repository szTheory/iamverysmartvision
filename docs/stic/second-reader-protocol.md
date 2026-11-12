# STIC literal and frame second-reader protocol

This protocol is for an isolated reader that re-derives one family of the Phase 6 oracle files (the register literal, the card bit roles, the ten hand-derived frames or the geometry facts), and for the orchestrator that starts those readers.
It holds no expected value.
The sources are the STIC and system RAM pages of the 1980 General Instrument catalog (`SRC-gi-1980-catalog`, pdf 191-193 and 199, with the same text in `SRC-gi-1982-catalog`, pdf 462-464 and 470), which describe the part for tier 1 structure only, and three community wiki pages with five word-format figures (`SRC-intv-wiki`), so the reading is a cross-check of the committed files and never evidence about a console.

## Purpose

An isolated reader re-derives the content of one family from the sources and the stated conventions, without seeing the committed values, citations or derivations.
The orchestrator diffs the result with the committed file row by row; a disagreement is resolved from the source, written in the note of the literal and counted.
The record of the reading says `source-expectation, second-read` and never `independent-implementation-comparison`, because the same model family reads the same pages and a shared misreading stays possible (C43).

## Allowed inputs

This list is exhaustive.
1. The reader's own blinded copy under `build/review/blind/`, made by the orchestrator: for the register literal and the card bit roles, the grammar header of the literal with no data rows; for each frame, the header lines of the frame file up to and including the line `plane` and nothing after it (the 96 pixel rows are removed, for example with a `sed` that prints the header lines and drops everything after the `plane` line); for the geometry, the key list of the grammar header.
2. The page renders and saved pages under the gitignored `refs/`: the catalogs `refs/SRC-gi-1980-catalog.pdf` and `refs/SRC-gi-1982-catalog.pdf` (pdf 191-193, 199 and 462-464, 470 only; a missing render may be made with `pdftoppm -r 150` into `refs/pages/`), and the saved community pages and figures under `refs/stic/` (`stic-15008.html`, `memmap-14847.html`, `backtab-14739.html` and the five `Stic_*.png` word-format figures).
3. This protocol, including the conventions below.

Nothing else may be opened, searched or recalled for a value.

## Forbidden inputs

- Everything under `.planning/`, `data/`, `core/`, `runner/`, `tests/`, `cmake/` and `tools/`, in particular the committed literals, the rule file, the frames and the profile.
- `docs/stic/reference-notes.md`, `docs/stic/timing-notes.md`, `docs/video/` and `docs/time-base.md`, because they state the readings and the derivations.
- The output of any other reader and the git history of any path.
- Any emulator, assembler or disassembler, its source or its output, including other repositories on the machine.
- Web searches for expected values.

## Conventions that are given and not derived

- A card is 8 x 8 pixels; pattern byte y of a card is pixel row y from the top; bit 7 is the leftmost pixel; a 1 bit shows the foreground index and a 0 bit the background index.
- A card with no `gram` line in a frame, and every graphics ROM card, is all zero (a zero stand-in image).
- The plane is 12 x 20 cards, 96 rows; the 8th pixel of a column 19 card is not displayed and is written `-`; every row is written as 20 groups of 8 symbols separated by one space, hex digits 0-9A-F.
- BACKTAB cell (r, c) is the word at address 0200 + 20 r + c; bits 15 and 14 of a word are not used.
- In Color Stack mode the stack position is 0 at the start of the frame, advances by one (mod 4) before a card is drawn when its advance bit is set, is never reset by a row, and a column 19 card advances it too; the background of a card is the stack entry at the position after its advance.
- Colored Squares is not used in any frame.

## Output

The reader writes exactly one file, once, at the end of its work, under `build/review/derived/`:
- register literal: `regs-<n>.txt`, one line per address 0000..003F in the grammar of the blinded header with the columns it can fill from the sources (`role`, `class`, `src`, `tag`); the columns `w` and `r` are answered `w=unknown r=unknown` when no source states them (they are defaults, C81 and C86);
- card bit roles: `cards-<n>.txt`, one line per variant and bit (64 lines) in the grammar of the blinded header;
- frames: `frames-<id>.txt`, the 96 plane rows of one frame, derived from the header inputs only;
- geometry: `geometry-<n>.txt`, one line per key of the blinded key list.
A fact that no page states is answered `unknown` with the missing fact in a trailing note, never with a guess; an ambiguous figure is written down with the reading chosen and the alternative.
The reader reports only that it finished and how many lines it wrote.

## Orchestrator procedure

1. Plan 06-01 is complete and the gate is green; the blinded copies are built into `build/review/blind/`.
2. Spawn one fresh subagent per file with no prior conversation, whose prompt contains only the blinded copy path, this protocol's path and the instruction to follow it exactly; run them in parallel batches of at most 4; pass no committed value, plan text, SUMMARY text, CONTEXT text or other readers' output.
3. After each batch check that each output exists and has the expected number of lines; replace a failed reader with a new fresh subagent, never by resuming it.
4. Diff each derived file against the committed file, resolve every disagreement from the sources, record the counts in `docs/stic/second-read-record-<family>.md` and never edit a derived file afterwards.
