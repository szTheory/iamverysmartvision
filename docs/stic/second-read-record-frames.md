# STIC hand-derived frames second-read record

Tier: `source-expectation, second-read` (never `independent-implementation-comparison`).
Date: 2026-11-12.
Readers: two isolated model readers (`stic-blind-frames-a` for F-A1..F-A5, `stic-blind-frames-b` for F-B1..F-B3, F-D1 and F-D2), from blinded copies holding the frame header up to the line `plane` (the 96 pixel rows and the `sha256-text` line removed).
Frames read: 10.
Frames identical to the committed plane (all 96 rows): 10.
Answered `unknown`: 0. Disagreements: 0.
Scope: the headers carry the decoded card fields (foreground, background, type, card), so this reading re-derives the plane (card geometry, bit order, zero stand-in cards, Color Stack advance, the cut-off column) from the header and the stated conventions; the BACKTAB bit decode is the cards literal's job (see its record).

Limits: the same model family read the same pages, so a shared misreading stays possible; this is a cross-check of the committed oracle file, not evidence about hardware. All register, card-field and window facts remain tier 4 or `unknown-default-chosen` (C86 onward stay open). The reader's derived file is kept in the gitignored `build/review/derived/` only; one reader handled the whole family (frames: five frames per reader).
