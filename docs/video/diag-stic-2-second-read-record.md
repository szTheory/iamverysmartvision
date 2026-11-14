state: closed
oracle-commit: 641a12e3a1059b242cc88a3ee4308e3f58d9b36d
corrections: 2
correction: (a)-A5 step 3 `$0200` to `$02FF` -> `$0200` to `$02EF`
correction: (a)-A11 `no MOB or collision outside the plane` -> `no collision from a position outside the plane` with the clipped-MOB clarification

# STIC 2 blind second-read record

Evidence kind: `source-expectation, second-read`. This is not an `independent-implementation-comparison`: the reader is the same model family as the author of the derivation, so a shared misreading stays possible (C43). Claims that rest on this record stay provisional.

Date: 2026-11-14. Request: `docs/video/diag-stic-2-second-read-request.md`. Reader: one fresh isolated agent, given only the blinded specification copy (53 lines, SHA-256 b73a15b9288729ed5c053d1570fa0b5ff2ab7a6b58f6e1f15d58d672e09102ed), the MOB, register-address and controller literals, `docs/runner.md` and `docs/cp1610/reference-notes.md`. The reader file is not committed (gitignored review directory).

## What was compared

- The 16 collision words at `$0300` to `$030F`: 16 of 16 agree with the truth-table rows T1 to T16 (`0002 0001 0008 0004 0000 0000 0100 0000`, then `0000 0100 0100 0000 0000 0000 0000 0000`), each with the same deciding rule (R44).
- The two pair rectangles: idle against mob at frame 2 is `150 90 9 6` with 38 positions; idle against border at frame 5 is `0 0 159 96` with 2012 positions. Both agree with the pair inventory.
- The delayed card 2 top-left set pixel: (81,48), (82,48), (83,48) for h = 1 to 3 and (80,49), (80,50), (80,51) for v = 1 to 3. Agrees.
- PD-1 to PD-13: the reader answered `pinned, not derivable` for all thirteen; no value was derived for a pinned item, so there is no pinned-item disagreement.
- The three second-look readings (a border strip hides a MOB under it; the Y size code with the object-field offset of 8 for the MOB of mob scene 1; sticky collision bits surviving a frame in which the MOBs no longer overlap): all three supported by the specification text.

## Disagreements and findings

1. **A5 step order (specification defect, corrected).** Step 2 stores the selector at `$02F2` and step 3 cleared `$0200` to `$02FF`, which contains `$02F2`. Read literally every scenario would run as idle. The intended and derived reading is that the selector survives; the clear is the BACKTAB only. Corrected to `$0200` to `$02EF`. No derived value changes.
2. **A11 against mob scene 1 (wording defect, corrected).** A11 said there is no MOB outside the plane, but the MOB of mob scene 1 is clipped at the plane edge. Reworded to the collision statement the program actually relies on. No derived value changes.
3. **Selector BD background (known, not a defect).** The collision routine never writes the color-stack registers, so the background and the color-7 square are not derivable; this is why the collision frame carries unknown cells and no hash (derivation section (e), frame F-M7). Only MOB and Colored Squares colours are asserted.
4. **R48 against PD-8 (reconciled).** R48 says rows 0 to 7 for the top strip while PD-8 pins the height unit; the parenthesis that F-M4 to F-M6 take 8 rows with the strip fixed to the display reconciles them. Strip probes are conditional on that convention, which the claim exclusions already carry.
5. **Smaller gaps (accepted).** The frame-0 slot content is unstated (frame 0 is not displayed). The Colored Squares bit layout is not in any file the reader may open, so the reader could not check it. Only delay value 3 occurs in a scenario, so h and v of 1 and 2 fall under PD-12.

Of the items compared, every derivable value agrees; the two corrections are specification wording that did not move any derived literal, frame or hash.
