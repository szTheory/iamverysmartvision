# STIC register literal second-read record

Tier: `source-expectation, second-read` (never `independent-implementation-comparison`).
Date: 2026-11-12.
Reader: isolated model reader `stic-blind-regs`, started by the orchestrator under `docs/stic/second-reader-protocol.md` from a blinded copy with the grammar header and the 64 register addresses only.
Rows read: 64.
Rows agreeing on role and class: 64.
Rows where the `w` and `r` columns differ: 64, all because the reader answered `unknown` (the protocol tells the reader to answer `unknown` unless a source states a mask); the committed masks are project defaults tagged `unknown-default-chosen` or tier 4 and are covered by the policy-row rule, not by the reading.
Answered `unknown`: 128 cells (w and r of 64 rows). Disagreements on role or class: 0.

Limits: the same model family read the same pages, so a shared misreading stays possible; this is a cross-check of the committed oracle file, not evidence about hardware. All register, card-field and window facts remain tier 4 or `unknown-default-chosen` (C86 onward stay open). The reader's derived file is kept in the gitignored `build/review/derived/` only; one reader handled the whole family (frames: five frames per reader).
