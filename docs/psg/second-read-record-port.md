# PSG vector second-read record: port

Tier: `source-expectation, second-read` (never `independent-implementation-comparison`).
Date: 2026-11-12.
Reader: isolated model reader group `psg-blind-port` started by the orchestrator under `docs/psg/second-reader-protocol.md`; one reader handled several vector files (a deviation from one reader per file, recorded here).
Vectors read: 10.
Agree: 3.
Answered `unknown`: 0.
Not comparable (blinded-copy limitation, rule agrees): 7.
Convention difference (resolution below): 0.

Not comparable: the blinded copy did not name which address each vector reads, so the reader answered both ports; on every address the committed vector names, the values agree (the extra port read is the undriven FF).

Limits: the same model family read the same pages, so a shared misreading stays possible; this is a cross-check of the committed vectors, not evidence about hardware (C11, C12, C69 stay open). The reader's derived file is kept in the gitignored `build/review/derived/` only.
