# PSG vector second-read record: mix

Tier: `source-expectation, second-read` (never `independent-implementation-comparison`).
Date: 2026-11-12.
Reader: isolated model reader group `psg-blind-mix` started by the orchestrator under `docs/psg/second-reader-protocol.md`; one reader handled several vector files (a deviation from one reader per file, recorded here).
Vectors read: 10.
Agree: 3.
Answered `unknown`: 1.
Not comparable (blinded-copy limitation, rule agrees): 6.
Convention difference (resolution below): 0.

Not comparable: the blinded copy held no stimulus for module-level `gate` rows, so the reader chose its own R7 values; every reader result follows the same gate rule (disable bit set ignores that source, both disabled gives constant 1).

- `psg.mix.truth-table`: reader answered `unknown` (no cached page states the value; the row is policy or convention).

Limits: the same model family read the same pages, so a shared misreading stays possible; this is a cross-check of the committed vectors, not evidence about hardware (C11, C12, C69 stay open). The reader's derived file is kept in the gitignored `build/review/derived/` only.
