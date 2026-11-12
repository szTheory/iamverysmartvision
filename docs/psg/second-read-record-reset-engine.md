# PSG vector second-read record: reset-engine

Tier: `source-expectation, second-read` (never `independent-implementation-comparison`).
Date: 2026-11-12.
Reader: isolated model reader group `psg-blind-reset-engine` started by the orchestrator under `docs/psg/second-reader-protocol.md`; one reader handled several vector files (a deviation from one reader per file, recorded here).
Vectors read: 2.
Agree: 1.
Answered `unknown`: 1.
Not comparable (blinded-copy limitation, rule agrees): 0.
Convention difference (resolution below): 0.

- `psg.reseng.envelope-held-zero`: reader answered `unknown` (no cached page states the value; the row is policy or convention).

Limits: the same model family read the same pages, so a shared misreading stays possible; this is a cross-check of the committed vectors, not evidence about hardware (C11, C12, C69 stay open). The reader's derived file is kept in the gitignored `build/review/derived/` only.
