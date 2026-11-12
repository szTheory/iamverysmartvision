# Controller literal second-read record

Tier: `source-expectation, second-read` (never `independent-implementation-comparison`).
Date: 2026-11-12.
Reader: one isolated model reader (label `ctrl-blind-1`), started by the orchestrator under `docs/input/second-reader-protocol.md` from a blinded copy holding only the 31 control names.
Rows read: 31.
Rows agreeing on pins, cleared mask and byte: 31.
Disagreements: 0.
The reader also stated the disc orientation (compass names assumed from the image, D1 at the top, D2..D16 clockwise) which matches the literal's reading.

Limits: the same model family read the same two community pages (one lineage), so this is a transcription cross-check of the committed literal, not evidence about hardware (C16 stays open; only D10 or a schematic can change that). The reader's derived file is kept in the gitignored `build/review/derived/` only.
