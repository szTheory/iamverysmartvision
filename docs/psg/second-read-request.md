# PSG vector second-read request

State: `pending: orchestrator-run blind reader`.
This request was written in plan 05-01 and no blind read has been performed; plan 05-01 is the oracle commit that precedes every implementation plan of Phase 5.

## What is to be blind-read

Each vector file of `data/psg/vectors/` is read by one reader, 11 readers in all, and the stems are the lines of `data/expected/psg-vector-files.ids`:

| Stem | Family | Vectors |
|------|--------|---------|
| regs | psg.reg | 28 |
| reset | psg.reset | 2 |
| reset-engine | psg.reseng | 2 |
| tone | psg.tone | 16 |
| noise | psg.noise | 12 |
| env | psg.env | 22 |
| mix | psg.mix | 10 |
| amp | psg.amp | 6 |
| ampenv | psg.ampenv | 4 |
| port | psg.port | 10 |
| dac | psg.dac | 8 |

The register-address literal `data/psg/intv-2609-ntsc-family-p1.regmap.literal.txt` is read with the file `regs`, because its 16 rows are the addresses those vectors use.
Policy rows (`kind=policy`) have no source to read: a reader answers them under the stated convention and defaults, and the diff reports them separately.

## Sources and rights

Allowed inputs and the reading rules are in `docs/psg/second-reader-protocol.md`.
The sources are `SRC-gi-1980-catalog` (pdf 204-210) and `SRC-intv-wiki` (PSG page, revision 14940); both are cached only under the gitignored `refs/` and are read into the cache, never committed (rights unknown).

## Record

Path: one file per family, `docs/psg/second-read-record-<family>.md`, for example `docs/psg/second-read-record-tone.md`, written by the orchestrator after the diff.
Content: the tier word `source-expectation, second-read` (never `independent-implementation-comparison`), the date, the reader label, the number of vectors read, the number that agree, the number answered `unknown`, and each disagreement with its resolution.
A disagreement is resolved from the source, written in the `derive` text of the vector and counted in the record.
