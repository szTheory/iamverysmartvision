# Consultation log

This log records every reading of another emulator's files.
The rule behind it is in `docs/provenance.md`.
Names are allowed in this file, and the denylist step excludes it.
Add one row for each reading, with the value re-derived from a tier 1 to 3 source.

| Date | Source (SRC id) | What was read | Why | Value re-derived from | Reader |
|------|-----------------|---------------|-----|-----------------------|--------|
| 2026-11-09 | `SRC-mame-intv` | The per-file licence headers of the permissive-licence driver files, and the crystal-divisor definition lines of the Intellivision driver, as recorded in `.planning/research/STACK.md` | Licence identification, and crystal-ratio discrepancy hints for C8 and C9 | Crystal frequency from `SRC-gi-1980-catalog`, pdf 191 (tier 1; a research-era reading of 2026-11-09, not re-read in this phase). The CPU divisor of 4 stays an inference (C8) | Project research agent |
| 2026-11-09 | `SRC-community-emu-docs` (the GPL emulator fork) | `src/COPYRIGHT.txt`, `src/asm/README`, `examples/library/print.asm` and `examples/hello/hello.asm`: licence headers and README text only, as recorded in `.planning/research/STACK.md` Part 2 and Sources | The purpose was licence determination for the fixture-tooling decision. This is a disclosed prior exception: the reading came before the no-copy rule was recorded | None. No value, code, table or identifier was taken, and no copy was kept | Project research agent |
| 2026-11-14 | `SRC-community-emu-docs` (the STIC register description, a documentation text file of the GPL fork) | The whole text of the file, read raw as plain text from the gitignored cache: the MOB, collision, delay and border-extension sections and the object-field description. Documentation text only, no source code | The MOB, collision, delay and border-extension semantics that no tier 1 to 3 page prints (D-01), for the derivation of the frames F-M0 to F-M9 and the rules STIC-R37 to STIC-R50 | None. Tier 4 lead restated in project words; the roles are the rows of `data/video/mob.literal.txt`, no sentence, table or figure was kept | The executing agent (plan 08-03) |
| 2026-11-14 | `SRC-community-emu-docs` (the interrupt notes, a documentation text file of the same fork) | The whole text, read raw as plain text from the gitignored cache: the interrupt, register-window and timing sections, including the usable-cycle figures against vertical delay. Documentation text only, no source code | The vertical-delay figures for the bus-window pin of C92 and the register-window length for the program timing of `ivs-diag-stic-2` | None. Tier 4 lead restated in project words; the figures are cited by locator in the notes of C92 | The executing agent (plan 08-03) |
| 2026-11-14 | `SRC-community-emu-docs` (reverse-engineering notes of one other board by the author of the two files above, a public document; same lineage, no new SRC id) | The whole text, read raw as plain text from the gitignored cache: the register read-back table, the delay and bus-copy measurements and the list of items still to be measured. Text only, no figure | The note of C81 on the read bits and the open question of MOBs in rows exposed by vertical delay (C110) | None. Tier 4 report of another board, never evidence for this profile; restated in project words in the notes of C81 and C110 | The executing agent (plan 08-03) |

## Statements

GPL emulator source was never opened for implementation.
The second row above is the only reading of files from the GPL fork.
It was for licence determination only.
No GPL emulator source was opened in this phase.

The first row is a research-era reading, made before the no-copy rule was recorded.
Nothing from it was copied, ported or paraphrased.
Each value that the project now uses from that area cites a tier 1 to 3 source instead.

Plan 08-03 (2026-11-14) adds the three rows above: documentation text files of the GPL fork and the public reverse-engineering notes of the same author, read as plain text for the semantics that no tier 1 to 3 page prints.
No emulator source was opened, no text, table or figure was copied, and the cached copies stay in the gitignored directory refs/stic.
