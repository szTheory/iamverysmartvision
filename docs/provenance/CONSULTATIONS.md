# Consultation log

This log records every reading of another emulator's files.
The rule behind it is in `docs/provenance.md`.
Names are allowed in this file, and the denylist step excludes it.
Add one row for each reading, with the value re-derived from a tier 1 to 3 source.

| Date | Source (SRC id) | What was read | Why | Value re-derived from | Reader |
|------|-----------------|---------------|-----|-----------------------|--------|
| 2026-11-09 | `SRC-mame-intv` | The per-file licence headers of the permissive-licence driver files, and the crystal-divisor definition lines of the Intellivision driver, as recorded in `.planning/research/STACK.md` | Licence identification, and crystal-ratio discrepancy hints for C8 and C9 | Crystal frequency from `SRC-gi-1980-catalog`, pdf 191 (tier 1; a research-era reading of 2026-11-09, not re-read in this phase). The CPU divisor of 4 stays an inference (C8) | Project research agent |
| 2026-11-09 | `SRC-community-emu-docs` (the GPL emulator fork) | `src/COPYRIGHT.txt`, `src/asm/README`, `examples/library/print.asm` and `examples/hello/hello.asm`: licence headers and README text only, as recorded in `.planning/research/STACK.md` Part 2 and Sources | The purpose was licence determination for the fixture-tooling decision. This is a disclosed prior exception: the reading came before the no-copy rule was recorded | None. No value, code, table or identifier was taken, and no copy was kept | Project research agent |

## Statements

GPL emulator source was never opened for implementation.
The second row above is the only reading of files from the GPL fork.
It was for licence determination only.
No GPL emulator source was opened in this phase.

The first row is a research-era reading, made before the no-copy rule was recorded.
Nothing from it was copied, ported or paraphrased.
Each value that the project now uses from that area cites a tier 1 to 3 source instead.
