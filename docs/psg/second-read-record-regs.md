# PSG vector second-read record: regs

Tier: `source-expectation, second-read` (never `independent-implementation-comparison`).
Date: 2026-11-12.
Reader: isolated model reader group `psg-blind-regs` started by the orchestrator under `docs/psg/second-reader-protocol.md`; one reader handled several vector files (a deviation from one reader per file, recorded here).
Vectors read: 28.
Agree: 27.
Answered `unknown`: 0.
Not comparable (blinded-copy limitation, rule agrees): 1.
Convention difference (resolution below): 0.

Not comparable: the reader omitted the third read address of one vector; the two it gave agree.

Limits: the same model family read the same pages, so a shared misreading stays possible; this is a cross-check of the committed vectors, not evidence about hardware (C11, C12, C69 stay open). The reader's derived file is kept in the gitignored `build/review/derived/` only.
