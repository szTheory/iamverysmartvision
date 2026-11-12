# PSG vector second-read record: dac

Tier: `source-expectation, second-read` (never `independent-implementation-comparison`).
Date: 2026-11-12.
Reader: isolated model reader group `psg-blind-dac` started by the orchestrator under `docs/psg/second-reader-protocol.md`; one reader handled several vector files (a deviation from one reader per file, recorded here).
Vectors read: 8.
Agree: 1.
Answered `unknown`: 5.
Not comparable (blinded-copy limitation, rule agrees): 2.
Convention difference (resolution below): 0.

Not comparable: only the key spelling differs (`l0` / `l15` against `0` / `15`); the values agree.

- `psg.dac.printed-levels`: reader answered `unknown` (no cached page states the value; the row is policy or convention).
- `psg.dac.extrapolated-levels`: reader answered `unknown` (no cached page states the value; the row is policy or convention).
- `psg.dac.halving-8-from-10`: reader answered `unknown` (no cached page states the value; the row is policy or convention).
- `psg.dac.halving-1-from-3`: reader answered `unknown` (no cached page states the value; the row is policy or convention).
- `psg.dac.sum-fits-u32`: reader answered `unknown` (no cached page states the value; the row is policy or convention).

Limits: the same model family read the same pages, so a shared misreading stays possible; this is a cross-check of the committed vectors, not evidence about hardware (C11, C12, C69 stay open). The reader's derived file is kept in the gitignored `build/review/derived/` only.
