# Controller literal second-read request

State: `pending: orchestrator-run blind reader`.
This request was written in plan 05-01 and the blind read has not been performed; plan 05-01 is the oracle commit that precedes every implementation plan of Phase 5.

## What is to be blind-read

One file, read by one reader: `data/input/intv-2609-ntsc-family-p1.literal.txt`, 31 rows, one per control name of `data/expected/input-bits.ids`.
The reader derives for each name the connector pins grounded, the cleared bit mask and the byte read with only that control pressed.

## Sources and rights

Allowed inputs and the reading rules are in `docs/input/second-reader-protocol.md`.
The sources are `SRC-consolemods-intv-pinouts` (pin table and pin-to-bit map) and `SRC-intv-wiki` (the key-table image, revision 14808); both are cached only under the gitignored `refs/ctrl/` and are read into the cache, never committed (rights unknown).

## Record

Path: `docs/input/second-read-record.md`, written by the orchestrator after the diff.
Content: the tier word `source-expectation, second-read` (never `independent-implementation-comparison`), the date, the reader label, the number of rows read, the number that agree, and each disagreement with its resolution from the source.
A disagreement is resolved from the source, written in the note of the literal and counted in the record.
