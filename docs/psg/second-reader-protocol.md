# PSG vector second-reader protocol

This protocol is for an isolated reader that re-derives the expectations of one PSG vector file in `data/psg/vectors/`, and for the orchestrator that starts those readers.
It holds no vector id and no expected value.
The pages are the AY-3-8910 pages of the 1980 General Instrument catalog (`SRC-gi-1980-catalog`, pdf 204-210), which describe a related part, and one community wiki page for the console part, so the reading is a cross-check of the committed expectations and never evidence about the 8914-class part.

## Purpose

An isolated reader re-derives the expected value of each vector of one file from the page images and the stated conventions, without seeing the committed expectations, citations or derivations.
The orchestrator diffs the result with the committed file vector by vector; a disagreement is resolved from the source, written in the vector's `derive` text and counted.
The record of the reading says `source-expectation, second-read` and never `independent-implementation-comparison`, because the same model family reads the same pages and a shared misreading stays possible.

## Allowed inputs

This list is exhaustive.
1. The reader's own blinded copy `build/review/blind/psg-<family>.txt`: the grammar header of the vector file, then per vector its id, rule id, `set=`, `pin=`, `steps=` and the kind of its `expect=` without values, followed by a table of the rule statements of the rules it names.
2. The page renders under the gitignored `refs/psg/` (`img80-204.png` to `img80-210.png`, `fig3-208.png`, `r7-207.png`) and the catalog `refs/SRC-gi-1980-catalog.pdf` (pdf 204-210 only; a missing render may be made with `pdftoppm -r 150` or `-r 300` into `refs/psg/`), and the saved community wiki page `refs/ctrl/psgwiki.html`.
3. This protocol.

Nothing else may be opened, searched or recalled for a value.

## Forbidden inputs

- Everything under `.planning/`, `data/`, `core/`, `runner/`, `tests/`, `cmake/` and `tools/`, in particular the committed vector files, the rule file, the register-address literal and the profile.
- `docs/psg/reference-notes.md` and `docs/time-base.md`, because they state the readings and the step arithmetic.
- The output of any other reader and the git history of any path.
- Any emulator, assembler or disassembler, its source or its output, including other repositories on the machine.
- Web searches for expected values.

## Output

The reader writes exactly one file, `build/review/derived/psg-<family>.txt`, once, at the end of its work: one line per vector of the blinded copy, in its order, in the form `<id> expect=<kind>:<values> basis=<one line naming the page, figure or wiki row and the arithmetic>`, with the `expect` grammar of the header of the blinded copy.
A vector whose value depends on something no page states is answered with `expect=<kind>:unknown` and the missing fact in `basis`, never with a guess.
The reader reports only that it finished and how many lines it wrote.

## Derivation rules

- Divide chains, field widths, shapes and the mixer bit positions come from the page images; the wiki page gives the register order, the masks, the period-zero statements and the amplitude select bits; the conventions in the grammar header (counters empty at reset, reload on the first step, step first then write) are given and are not derived.
- Arithmetic is integer arithmetic over steps of 16 master ticks as the header states.
- An ambiguous page is written down in `basis` with the reading chosen and the alternative, never decided silently.

## Orchestrator procedure

1. Plan 05-01 is complete and the gate is green.
2. For each stem of `data/expected/psg-vector-files.ids` build the blinded copy as described under Allowed inputs, item 1, into `build/review/blind/`.
3. For each stem spawn one fresh subagent with no prior conversation whose prompt contains only the blinded copy path, this protocol's path and the instruction to follow it exactly; run them in parallel batches of at most 4; pass no committed vector, plan text, SUMMARY text, CONTEXT text or other readers' output.
4. After each batch check that each output exists and has one line per vector; replace a failed reader with a new fresh subagent, never by resuming it.
5. Diff each derived file against the committed file, resolve every disagreement from the sources, record the counts in `docs/psg/second-read-record-<family>.md` and never edit a derived file afterwards.
