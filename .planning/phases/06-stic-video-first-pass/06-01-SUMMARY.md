---
phase: 06-stic-video-first-pass
plan: 01
subsystem: data, docs, tools/check
tags: [oracle-first, sources, page-reading-verdict, register-literal, card-roles, chain-framing, hand-derived-frames, stic-rules, listing-sdbd, time-base, D-23, VID-01, VID-02, VID-03, VID-04, VID-06]
status: complete

requires:
  - phase: 05-hand-controllers-and-psg-sound
    provides: "the gate (steps=35/35 controls=316/316 faults=193/193 discrepancies=80 d10-items=32), the record-kind driver, the literal and rule-inventory idioms of the PSG oracle plan, the listing checker with the branch-target rule"
provides:
  - "docs/stic/reference-notes.md: the ten-key page-reading verdict, the rule tier table of 53 rules and ONE allocation table (C81..C105, CL-0016..CL-0022, D10-VID-01..09, D11-EXEC-03, rule ids, frame ids)"
  - "docs/stic/timing-notes.md: the frame timeline with its arithmetic and closed forms; docs/stic/d10-procedure.md: owner hand-off anchors"
  - "data/stic/intv-2609-ntsc-family-p1.regmap.literal.txt (64 rows), data/stic/rules.json (STIC-R01..R36, CR-01..CR-17), data/stic/fields-pending.ids (34 lines) and tests-pending.ids with an owner plan per line"
  - "data/video/geometry.literal.txt, cards.literal.txt (64 bit roles), chain-framing.literal.txt (h0, h1, h2 from an independent recipe), ten hand-derived frames F-A1..F-A5, F-B1..F-B3, F-D1, F-D2 with conservation data and drift keys"
  - "gate step video (36th) with 9 controls; listing rule listing-sdbd with 3 controls; docs/time-base.md corrected; docs/video/diag-stic-1-derivation.md; blind second-read protocol and request (pending)"
affects: [06-02, 06-03, 06-04, 06-05, 06-06, 06-07, 06-08, 06-09, 06-10, 06-11, 06-12, 06-13, 06-14]

actuals:
  tokens: 138000
  tasks: 5
  commits: 5
plan_head_before: da9a0ca3860709d3da20373b0204bb7408e1c190
plan_head_after: eae8ca5

tech-stack:
  added: []
  patterns:
    - "Oracle plan in front of the implementation: literals, frames and ids committed from the sources and the written rules, checked by a pure text step with a toy tree"
    - "Closed grammar for hand-derived frames (20 groups of 8 symbols, cut-off mark, header keys) with per-card pixel-count conservation and a canonical-text hash drift key computed by cmake string(SHA256)"
    - "Pending lists with an owner plan per line so no later task commit trips the registration check"

key-files:
  created:
    - docs/stic/reference-notes.md, timing-notes.md, d10-procedure.md, second-reader-protocol.md, second-read-request.md
    - docs/video/frame-derivations.md, docs/video/diag-stic-1-derivation.md
    - data/stic/intv-2609-ntsc-family-p1.regmap.literal.txt, rules.json, fields-pending.ids, tests-pending.ids
    - data/video/geometry.literal.txt, cards.literal.txt, chain-framing.literal.txt, frames/F-*.frame (10)
    - data/expected/stic-rules.ids, video-frames.ids, gate-steps/video.ids, controls/video.ids
    - tools/check/steps/video.sh, video.cmake, schema/stic-rule.txt, kinds/stic-rule.cmake, nine video controls, three listing-sdbd controls and toy files under controls/_good/tree
  modified:
    - data/discrepancies.json, data/expected/discrepancies.ids, data/expected/d10-measurements.ids
    - tools/check/listing.cmake, data/expected/controls/listing.ids, docs/diagnostics.md, docs/time-base.md
    - .gitignore, tools/check/binary-ext.txt, three no-files control MAKE files

key-decisions:
  - "Page-reading verdict: registers, color stack, display enable, color numbering, window lengths and bus hold are not printed; the BACKTAB address (512 decimal) and extent, the 14-bit use, rows per frame and the 159 x 96 visible geometry are printed; the timing figure is legible but prints no frame-level number"
  - "backtab-format-printed is partial, so rules CR-01, CR-02 and STIC-R03 and the geometry rows backtab-base are tier 1 (the plan predicted no); the word field layouts stay tier 4 (one community lineage)"
  - "Frame grammar: the 8th symbol of group 20 is the cut-off mark and is required, not merely allowed; the published plane is the 159 symbols left of it"
  - "The diagnostic uses one BACKTAB word set read in both modes (the cross-mode pair at diagnostic scale), one branch on the port byte, marker and recorded byte stored outside BACKTAB"
  - "Colored Squares bit order of the third color is a recorded reading (C98); no Phase 6 frame uses Colored Squares"

duration: about 5 hours of edits plus eleven gate runs (frames and derivations were generated with a throwaway typing aid kept outside the repository)
completed: 2026-11-12
---

# Phase 6 Plan 01: sources and oracles Summary

**The Phase 6 oracle set is committed with no C: the page-reading verdict, the ONE allocation table (C81..C105, CL-0016..CL-0022, ten D10 lines), a 64-row register literal, 53 rules, geometry, card bit-role and chain-framing literals, ten hand-derived frames, the gate step `video`, the listing SDBD rule, the corrected time-base text and the diagnostic derivation with its cycle budget. The gate ends `steps=36/36 controls=328/328 faults=193/193`.**

## What was built

Oracle commit (the last commit of this plan, named by every later Phase 6 SUMMARY): `eae8ca5`.

- Task 1 (`f81a5b0`): page-reading verdict, reference notes with the allocation table, timeline notes, D10 procedure, discrepancies C81..C105 and the updates, `d10-items` 32 to 42, ignore and binary lists (`discrepancies=105`).
- Task 2 (`9fad90e`): register literal, rule inventory (kind `stic-rule`), pending lists, step `video` with 3 controls (`steps=36/36 controls=319/319`).
- Task 3 (`5a8e6d8`): geometry, card and chain literals, eight unit frames, frame checks with 6 controls (`controls=325/325`, `video-frames=8`).
- Task 4 (`2db530d`): listing `SDBD` rule with 3 controls, `docs/time-base.md` corrected, grammar sentence in `docs/diagnostics.md` (`controls=328/328`).
- Task 5 (`eae8ca5`): diagnostic derivation, frames F-D1 and F-D2, blind second-read requests (`video-frames=10`).

## Page-reading verdict (decides the tier of every later rule)

Read as images at 150 dpi: pdf 191, 192, 193, 199, 200 of the 1980 catalog and pdf 462, 463, 464, 470 of the 1982 catalog (the 1982 copy carries the same text; pdf 461 shows the system diagram).

```
registers-printed: no
backtab-format-printed: partial
color-stack-printed: no
display-enable-printed: no
colors-numbered-printed: no
window-lengths-printed: no
bus-hold-printed: no
rows-per-frame-printed: yes
visible-geometry-printed: yes
timing-figure-readable: partial
```

The STIC pages are a pin, bus and electrical data sheet: they print the 167 x 105 grid with 159 x 96 visible, the 20 x 12 card matrix with a 20th column of 7 pixels, 240 background words of 14 bits, 16 colors, thirteen or fourteen second-sync requests per frame, the SR1 return to CPU mode and the rule that the CPU control bus is ignored outside CPU mode. The system RAM page prints that the BACKTAB lies at 512 to 863 with the graphics descriptors in the first 240 words, that graphics use only the lower 14 bits of a word, and the bus copy mode (entered on the interrupt acknowledge, left at the first bus acknowledge edge); its electrical page prints the 1.79545 MHz input. The timing figure on pdf 193 and 464 is legible but prints numbers only for the bus protocol (nanosecond set-up and delay values); the synchronisation drawing is schematic, has an elided pulse train and carries no cycle or window figure. So no window length, bus hold, register address, bit field, strobe or color stack is tier 1: all are tier 4 (one community lineage) or defaults. The one change from the pre-reading is the BACKTAB address and extent (partial, tier 1).

## Counts

- Start: `steps=35/35 controls=316/316 faults=193/193 discrepancies=80 d10-items=32 claims=15`.
- End: `steps=36/36 controls=328/328 faults=193/193 tests-missing=0 discrepancies=105 pending-resolvers=21 d10-items=42 stic-rules=53 stic-rows=64 video-frames=10 listings=3 listing-words=167`; controls 316 + 3 (video STIC part) + 6 (video frames) + 3 (listing-sdbd) = 328.
- Rules: 53 (8 tier1-literal, 1 tier1-derived, 1 tier2-bsr, 15 tier4-community, 5 inference, 13 default-unknown, 10 project-rule); pending fields 34 (33 for plan 06-04, `memmap.device.stic14` for 06-06); pending test families 54 lines across 06-04 and 06-06..06-13.
- Discrepancies: 25 new open entries C81..C105 (8 with a `measurement:` resolver, each named by a D10 line); C10 open to partial with resolver `measurement:` (D10-VID-01); updates to C1-C7, C21, C38, C41, C52, C60, C61. No claim added (CL-0016..CL-0022 are reserved in the table for plan 06-14).
- No file under `core/`, `runner/`, `tests/`, `cmake/` changed; `git ls-files refs` is empty; no ppm, vraw, png or pdf tracked.

## Cycle budget of the diagnostic (derivation `docs/video/diag-stic-1-derivation.md`)

133 words at `5000`-`5084` (limit 170), 4 branches (limit 8). Frame-0 margins (thresholds in brackets): strobe store at least 1,897 cycles inside the register window (100); last GRAM store at least 2,543 cycles inside the graphics-memory window (100); BACKTAB row 0 complete at least 2,124 cycles before its latch (200), rows 8 and 11 at least 3,368 and 6,104; the HLT unit starts between cycle 21,847 and 22,237 (87,388 to 88,948 ticks), at least 27,652 ticks after `E_0` and 30,524 ticks before `E_1` (4,000), and 23,128 and 36,608 ticks in the display-off mutant; the scenario total is about 89,000 ticks (150,000).

## Deviations from Plan

**1. [Rule 1 - Inaccuracy] `backtab-format-printed: partial`, not `no`.** The system RAM page prints addresses 512 to 863 and the lower 14 bits, so `CR-01`, `CR-02`, `STIC-R03` and the geometry row `backtab-base` are tier 1 (the plan said `backtab-base` tier4-community). The wiki row is kept as a second row `backtab-base-wiki`, and the field layouts stay tier 4.

**2. [Rule 3] The community wiki pages and the five word-format figures were fetched in this plan.** They were not in the cache (only the catalogs were); they went into the gitignored `refs/stic` and the figures were read as images before the literals were written. `SRC-intv-wiki` stays reference-only and no registry entry was added (nothing cited is missing).

**3. [Rule 2] Vocabulary.** The register literal uses four-hex-digit addresses without `$` (the PSG precedent), the tag `unknown-default-chosen` (the project's source-authority vocabulary) instead of `default-unknown`, and `delegated-decision` for the project-defined geometry rows. The rule `basis` list is the plan's (`default-unknown` is a rule basis, not a literal tag).

**4. [Rule 1] Frame grammar tightened.** The cut-off mark `-` is required at the 8th symbol of group 20 (clip side right); the step also checks that every unlisted cell is one uniform symbol and, in Foreground/Background frames, that all unlisted cells show the same symbol. Annotated cells need distinct foreground and background indices. The rule-id check of a frame header is reported as `video-frame-tier-missing`.

**5. [Rule 3] cmake has no `{n}` repetition.** Row shapes are checked by splitting on spaces; `string(SHA256)` replaces a temporary file for the drift key.

**6. [Rule 3] Toy tree growth.** The `listing-sdbd` rule needed toy rows `SDBD` and `MVII` (family table, second-reader record, rows inventory, coverage-pending list) and one valid prefixed pair in the toy listing; the three no-files controls (`glossary`, `text`, `sources`) needed one or two more deletes so the new toy files do not keep them from reaching zero scanned files.

**7. [Rule 1] Colored Squares.** The figure labels the two low bits of the third color at word bits 10 and 9 as `Bits 0-1`, the Foreground/Background figure writes `Bits 1 - 0`; the literal keeps the natural order (bit 10 is color bit 1) and C98 records the swapped reading. No frame uses it.

**8. Diagnostic design choices (derivation, not the plan text).** One BACKTAB word set is read in both modes (instead of separate Color Stack words), the end marker is at `$02F0` and the recorded port byte at `$02F1` (not `$020F`, which is BACKTAB and would be drawn), the explicit words sit in rows 8 and 11 so every row is complete well before its latch, and the delay count is 900.

## Decisions Made

- The allocation table is `docs/stic/reference-notes.md`; the next free ids are C106 and CL-0023 (CL-0016..CL-0022 stay reserved for plan 06-14).
- Policy numbers (windows, hold, interrupt width, power-on state) appear in no literal or frame as an expected value; they are cited only in notes and in the cycle budget as margins.
- The D4 mini-assembler decision stands by its letter (three diagnostics); the crossing rule and the effort data to record are in the derivation.

## Open Items

- **Pending blind second reads (orchestrator):** `docs/stic/second-read-request.md` state `pending: blind reads not yet run`: the register literal and card bit roles (job a), the ten frames F-A1..F-A5, F-B1..F-B3, F-D1, F-D2 (job b, ten readers) and the geometry facts (job c). The records `docs/stic/second-read-record-{regs,cards,frames,geometry}.md` do not exist yet.
- **Later plans:** 06-03 and 06-04 add the 33 profile fields and delete their lines in `data/stic/fields-pending.ids`; 06-06 adds the device kind and the first `stic.*` tests; each plan that registers a test family deletes its line in `data/stic/tests-pending.ids`; 06-13 builds the listing from the derivation, adds the first SDBD listing (the decode order of `listing.cmake` is compared with `ivs_pack --dump-words` then) and ties the `.expect` frame hashes to the frame files; 06-14 writes CL-0016..CL-0022.
- The profile `data/profiles/intv-2609-ntsc-family-p1.json` still carries the old note "never derived by modulo" on two frame fields (not contradicted by the corrected text; plan 06-04 refreshes profile notes).
- D10 and D11: 10 new inventory lines handed off (`d10-items=42`), nothing performed; the reader cartridge is a Phase 7 deliverable.
- Carried from Phase 4 and 5: IN-04, IN-06, IN-09..IN-11 and the Phase 5 deferrals stay open; `sh tools/hygiene.sh --history` still fails on earlier planning commits.

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=36/36 controls=328/328 ... faults=193/193 ... tests-missing=0 ... discrepancies=105 ... d10-items=42 ... stic-rules=53 stic-rows=64 video-frames=10`, `gate-time total=50` (laptop load moderate).
- `dash tools/gate.sh`: the same line, `gate: PASS`, `gate-time total=64`.
- `sh tools/gate-linux.sh`: the Ubuntu gcc container proxy ran the full gate: `gate: PASS steps=36/36 controls=328/328 ... cc=GNU ... faults=193/193 ... tests-missing=0 ... discrepancies=105 d10-items=42 ... video-frames=10`, `gate-time total=254`, exit status 0, last line of `build/gate-linux.log` starts with `gate: PASS` (the new step and checkers use no cmake command newer than 3.25 and no bash-only construct).

## Threat Flags

None. Records, docs and checkers only; no network endpoint, no file access pattern, no schema at a trust boundary.

Mitigations from the plan's threat model: T-06-01 (catalog pages, wiki pages and figures only under `refs/`, `git ls-files refs` empty, `.gitignore` and `binary-ext.txt` extended, hygiene, files, rights and denylist steps green), T-06-02 (no C, so no run exists; every rule carries a basis; conservation and drift checks are pure text; blind second reads requested; claims not added).

## Self-Check: @@SELF_CHECK@@
