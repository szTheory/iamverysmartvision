# STIC card bit-role literal second-read record

Tier: `source-expectation, second-read` (never `independent-implementation-comparison`).
Date: 2026-11-12.
Reader: isolated model reader `stic-blind-cards`, from a blinded copy with the grammar header and the 64 row keys only; the five word-format figures were read as images.
Rows read: 64.
Agree: 62.
Answered `unknown`: 0.
Convention differences: 2 (`cs-grom` bit 11 and `cs-gram` bit 11). The committed literal records the role `sel` (the select bit of the general Color Stack word figure); the reader recorded the fixed value shown in the dedicated GROM and GRAM figures (`fixed0`, `fixed1`). The semantics are the same (bit 11 selects GROM or GRAM); resolution: the committed `sel` stands, the figures' fixed cells are the two values of that select bit.
The Colored Squares third-color bit order and the non-monotonic Foreground/Background background bit order (C96, C98) agree between the literal and the reader.

Limits: the same model family read the same pages, so a shared misreading stays possible; this is a cross-check of the committed oracle file, not evidence about hardware. All register, card-field and window facts remain tier 4 or `unknown-default-chosen` (C86 onward stay open). The reader's derived file is kept in the gitignored `build/review/derived/` only; one reader handled the whole family (frames: five frames per reader).
