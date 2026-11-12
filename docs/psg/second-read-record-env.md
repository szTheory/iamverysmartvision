# PSG vector second-read record: env

Tier: `source-expectation, second-read` (never `independent-implementation-comparison`).
Date: 2026-11-12.
Reader: isolated model reader group `psg-blind-env` started by the orchestrator under `docs/psg/second-reader-protocol.md`; one reader handled several vector files (a deviation from one reader per file, recorded here).
Vectors read: 22.
Agree: 21.
Answered `unknown`: 0.
Not comparable (blinded-copy limitation, rule agrees): 0.
Convention difference (resolution below): 1.

Convention difference `psg.env.restart-on-shape-write`: the reader sampled only steps 1, 2, 4, 6 and 8 and counted one decrement per sampled step, the committed row lists steps 1..9 with a level change every second step (the 2 x EP step rate). Both readings show the restart to the first level after the second shape write. The phase of the first level change is a convention (POL-PSG-02), not stated by a page. Resolution: the committed row stands; plan 05-07 must keep the restart property and the step rate, and states the phase convention in its test comment.

Limits: the same model family read the same pages, so a shared misreading stays possible; this is a cross-check of the committed vectors, not evidence about hardware (C11, C12, C69 stay open). The reader's derived file is kept in the gitignored `build/review/derived/` only.
