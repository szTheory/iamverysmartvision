# STIC geometry literal second-read record

Tier: `source-expectation, second-read` (never `independent-implementation-comparison`).
Date: 2026-11-12.
Reader: isolated model reader `stic-blind-geometry`, from a blinded copy with the key list only.
Keys read: 20.
Agree on value (after spelling normalisation such as `8x8` against `8 8`, `167x105` against `167 105`): 14.
Answered `unknown`: 6: `row-lines` (2, the committed tier is `inference`), `header-bytes`, `slot-bytes`, `display-off-fill` (project policy rows with no source) and the two derived keys the pages do not print.
Differences that are spelling only: `card-px`, `grid`, `visible-of-grid`, `width-formula`; the clip side and the not-displayed sentinel (`-` in the text plane grammar, `0xFF` in the published byte plane) describe the same convention.
Disagreements on a sourced value: 0. The tier-1 counts (20 x 12 cards, 8 x 8 pixels, 7-pixel last column, 159 x 96 visible of 167 x 105, 16 colors, BACKTAB 0200 with 240 words) all agree.

Limits: the same model family read the same pages, so a shared misreading stays possible; this is a cross-check of the committed oracle file, not evidence about hardware. All register, card-field and window facts remain tier 4 or `unknown-default-chosen` (C86 onward stay open). The reader's derived file is kept in the gitignored `build/review/derived/` only; one reader handled the whole family (frames: five frames per reader).
