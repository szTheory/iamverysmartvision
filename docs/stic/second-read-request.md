# STIC second-read request

State: `done: blind reads recorded` (2026-11-12, plan 06-14 corrected this line): the four records `docs/stic/second-read-record-regs.md`, `-cards.md`, `-frames.md` and `-geometry.md` exist (registers 64 of 64 rows agree on role and class with 128 mask cells answered unknown, card bit roles 62 of 64 agree with 2 convention differences, frames 10 of 10 identical, geometry 14 of 20 keys agree on value with 6 answered unknown and 0 disagreements on a sourced value).
This request is written in plan 06-01, the oracle plan that precedes every implementation plan of Phase 6; the blinded copies are built and the readers started by the orchestrator afterwards, and no reader output is committed in plan 06-01.

## What is to be blind-read

Three jobs, one fresh reader per file (job b has ten readers), with the protocol `docs/stic/second-reader-protocol.md`:

| Job | Reader input (blinded) | Reader output | Committed file compared |
|-----|------------------------|---------------|-------------------------|
| a, register literal and card bit roles | the grammar headers of the two literals with no data rows | the 64 register rows (role, class, source, tag; `w` and `r` answered `unknown` unless a source states them) and the 64 card bit-role rows | `data/stic/intv-2609-ntsc-family-p1.regmap.literal.txt`, `data/video/cards.literal.txt` |
| b, frame planes | for each of the ten frames F-A1..F-A5, F-B1..F-B3, F-D1 and F-D2: the header lines (tier, rules, mode, border, stack, fill, the BACKTAB cells, the card patterns) with the 96 pixel rows removed | the 96 plane rows of the frame | `data/video/frames/<id>.frame` |
| c, geometry facts | the key list of the grammar header | one value per key from the pages | `data/video/geometry.literal.txt` |

The blinded frame copy is made by the orchestrator with a `sed` that prints the header lines and drops everything after the line `plane`, and is placed under `build/review/blind/`.
Policy rows (the `w` and `r` columns, the display-off fill, the stack reset per frame, the cut-off column advance) have no source to read: a reader answers them under the stated conventions or `unknown`, and the diff reports them separately.

## Sources and rights

Allowed inputs and the reading rules are in `docs/stic/second-reader-protocol.md`.
The sources are `SRC-gi-1980-catalog` (pdf 191-193, 199), `SRC-gi-1982-catalog` (pdf 462-464, 470) and `SRC-intv-wiki` (the STIC page, revision 15008, the memory map page, revision 14847, and the BACKTAB page, revision 14739); all are cached only under the gitignored `refs/`, never committed (rights unknown).

## Record

Path: one file per family, written by the orchestrator after the diff and never in plan 06-01: `docs/stic/second-read-record-regs.md`, `docs/stic/second-read-record-cards.md`, `docs/stic/second-read-record-frames.md` and `docs/stic/second-read-record-geometry.md`.
Content: the tier word `source-expectation, second-read` (never `independent-implementation-comparison`), the date, the reader label, the number of rows or frames read, the number that agree, the number answered `unknown`, and each disagreement with its resolution.
A disagreement is resolved from the source, written in the note of the literal or in `docs/video/frame-derivations.md`, and counted in the record.
Hazards to resolve explicitly: the bit order of the third color of Colored Squares (C98), the non-monotonic Foreground/Background background bit order, and the clip side of the 7-pixel column (C101).
