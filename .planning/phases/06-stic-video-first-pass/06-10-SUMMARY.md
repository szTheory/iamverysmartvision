---
phase: 06-stic-video-first-pass
plan: 10
subsystem: core/stic, core/memmap, tests/video, cmake
tags: [card-renderer, foreground-background, graphics-accessor, frame-literals, bit-role-sweep, metamorphic, named-faults, D-02, D-11, D-12, D-18, D-19, VID-04]
status: complete

requires:
  - phase: 06-stic-video-first-pass
    provides: "plan 06-09 frame writer (card-row working buffer, slot, chain), plan 06-08 row latches, plan 06-01 hand-derived frames F-A1..F-A5 and the card bit-role literal; gate steps=36/36 controls=332/332 faults=214/214"
provides:
  - "core/src/stic/stic_cards.c: ivs_stic_render_cards_row and ivs_stic_render_cards, a pure Foreground/Background renderer (no static or global data, no allocation, no state); ivs_stic_cardin value copy of mode, border and the four stack values"
  - "ivs_memsys_stic_gfx_byte_internal: the pure graphics accessor (grom and gram backing, no gate, no fill, no record, never the CPU view)"
  - "ivs_stic_render_card_row and ivs_stic_render_plane take the STIC state and dispatch a display-on Foreground/Background frame to the renderer from the row latches and the registers; a display-on Color Stack frame is still IVS_PIXEL_BLANK (plan 06-11), a display-off frame is still the border fill"
  - "cmake/IvsVideoFrames.cmake (test-side generator from the literals only), tests/video/vid_fields.c (word-versus-fields encoder, FIXTURE reporting)"
  - "suite video 54 to 76 ids; four faults; faults 214 to 218"
affects: [06-11, 06-12, 06-13, 06-14]

actuals:
  tokens: 20300
  tasks: 4
  commits: 3
plan_head_before: 3ecdaaa5b7ca7177e6f292c3f287331c378f56fc
plan_head_after: a9825a836f99467e0e8fe83b0cf3ef36ad5691b4

key-decisions:
  - "The renderer is row-granular (`ivs_stic_render_cards_row`, 1,280 bytes) with the plane form as a loop over it, so the machine never holds a 15 KB frame (the Apple clang stack-probe import stays out of the symbol layer, as plan 06-09 decided)"
  - "Frame expectations are generated from the literal frame files as 160-symbol text rows (a dash is the cut-off pixel, 0xFF) converted by the tests; the generator fails closed on any row outside the grammar"
  - "The Color Stack interim is pinned by video.frame.cs-not-decoded for plan 06-11 to invert; the mode is set in the instance state, so no mode-select read is needed in the pin"

key-files:
  created:
    - core/src/stic/stic_cards.c
    - cmake/IvsVideoFrames.cmake
    - tests/video/vid_fields.c, video_fgbg.c, video_card.c, video_probe.c, video_meta.c, video_judge.c
  modified:
    - core/CMakeLists.txt, core/src/memmap/memsys.h, memsys.c, core/src/stic/stic.h, stic_frame.c
    - tests/CMakeLists.txt, tests/video/video_cases.h, video_main.c, video_slot.c
    - data/expected/tests/video.ids, data/expected/faults/video.ids, data/stic/tests-pending.ids

duration: about 2 hours elapsed (laptop load 17 to 38 during the runs)
completed: 2026-11-12
---

# Phase 6 Plan 10: Foreground/Background card rendering Summary

**A display-on Foreground/Background frame is now decoded from the row latches and the registers by a pure card renderer whose graphics bytes come only from a dedicated backing accessor; all five hand-derived frames F-A1..F-A5 of plan 06-01 are reproduced byte for byte in the unit path, and F-A1, F-A3 and F-A5 also through the whole machine across the first frame event (delivered 159 columns and the SHA-256 of the canonical text form).** The graphics ROM is the zero image everywhere (no GROM, no third-party glyph); GRAM patterns are project-authored shapes poked through the raw seam.

Oracle commit this plan follows: `eae8ca5` (06-01 task 5). First commit of the plan (FIRST): `29ec54a`. No frame or literal was edited; the first run of all 22 new tests passed with no renderer change after the first draft.

## What was built

- Task 1, tracer (`29ec54a`): `ivs_memsys_stic_gfx_byte_internal`, `stic_cards.c`, the dispatch in `stic_frame.c`, the frame generator, the encoder, `video.fgbg.F-A1` and `video.fgbg.machine-F-A1`, the reviewed inversion.
- Task 2 (`a5c681e`): F-A2..F-A5 (unit), machine F-A3 and F-A5, `video.card.*` (4), `video.grom.zero-stub`, `video.display-off.fill-border`, `video.color.range-0-15`, `video.probe.bit-roles-fgbg`, `video.meta.*` (3), two judges; the seven owner-06-10 pending lines deleted.
- Task 3 (`a9825a8`): faults `VIDEO_WRONG_CARD_MODE`, `VIDEO_CARD_BIT_ORDER_REVERSED`, `VIDEO_GRAM_GROM_SELECT_SWAPPED`, `VIDEO_20TH_COLUMN_WIDTH`, sites and registrations together.
- Task 4: the three gates below (no fix was needed, so no commit).

## Reviewed test change (named)

`video.frame.display-on-not-decoded` is INVERTED into `video.frame.display-on-fgbg-decoded` (a strobed frame with all-zero BACKTAB and graphics memory is color 0 at every published position, not the 0xFF sentinel). `video.frame.cs-not-decoded` is new: a display-on Color Stack frame is still every position 0xFF; plan 06-11 inverts it. No other existing test changed; the stale-fault collateral list of `STIC_EVENT_ORDER_START_BEFORE_END` gained the prefix `video.fgbg.` (the displayed flag clears, so the machine frames fail for the right reason).

## Tests and faults

| Fault | Site | Must fail | Also fails (observed) |
|-------|------|-----------|-----------------------|
| VIDEO_WRONG_CARD_MODE | stic_cards.c | video.fgbg.F-A1 | every fgbg, card, probe and judge id, grom.zero-stub, color.range, the two frame ids |
| VIDEO_CARD_BIT_ORDER_REVERSED | stic_cards.c | video.card.bit-order | fgbg F-A2, F-A3, machine F-A3, card.*, probe, both frame judges |
| VIDEO_GRAM_GROM_SELECT_SWAPPED | stic_cards.c | video.fgbg.F-A3 | video.grom.zero-stub (as the plan states), fgbg F-A2..F-A5 and machine, card.*, probe, judges |
| VIDEO_20TH_COLUMN_WIDTH | stic_cards.c | video.card.last-column-clip | fgbg F-A1..F-A5 (unit), color.range, judge.frame-row-changed; machine frames are unaffected because the writer crops the column |

The faults file allows one must-fail id per fault, so the plan's pair for the select fault (F-A3 and zero-stub) is recorded as one must-fail id plus the observed second failure above.

Suite `video`: 76 ids. Plain-run time (median of one quiet-ish run each on a loaded laptop, load 17): dev 0.53 s (dev target 1.2 s), release 0.15 s.

## Frame and role evidence status

- Blind second derivations (docs/stic records): frames 10 of 10 identical to the committed planes, no `unknown`, no disagreement; card bit roles 62 of 64 identical with two labelling-only differences (recorded, none touches the Foreground/Background variant used here). The renderer-versus-literal agreement is a check of transcription and derivation by one model family, never an independent implementation comparison (C43).
- The role sweep reads the fgbg rows of `data/video/cards.literal.txt` through the generated include; bits 15 and 14 are the only `ign` bits and change nothing.

## Neutrality and cost

`git diff --stat 3ecdaaa..HEAD -- data/golden data/scenarios data/expected/pkg-consumer.golden` prints nothing: no golden, scenario or consumer golden moved (no scenario shows a display-on frame yet, so video is still not observed in the runner and the consumer, with equal `digest.*` lines). Cost: this plan adds one core TU, 22 test ids (the video suite is 0.5 s dev) and four faults that rerun it once each; the local gate totals were 119 s (sh, task 2, loaded laptop), 78 s (dash) and 331 s in the container. The normalised metric r = cpu_s / build_s is the arbiter and can only be read from the first hosted row of this plan; I expect it not to have moved measurably (the added work is a 0.5 s suite and one small object file), but that is an expectation, not a measurement (median 7.9 versus trigger 8.3 stands until a hosted row says otherwise).

## Deviations from Plan

**1. [Interpretation, recorded] The bit-role probe is run twice per bit.** A one-bit word over the base word 0000 shows almost no role: the graphics ROM is zero (a card number or a foreground color is invisible) and a lone select bit over colors 0 and 0 draws nothing. Each bit is therefore probed alone (base 0000, as the plan says) AND toggled on a visible base word (select graphics RAM, card 37, foreground 5, background 10, built from the literal roles), and the whole 8 x 8 block must equal the block the literal roles predict (an independent decoder in the test) rather than reading fg, bg and card back one by one. The stand-in graphics are as the plan says (GRAM card n has byte 0 = n and byte 7 = 5A).

**2. [Rule 3] The machine path clears the stand-in GROM.** The shared guest builder loads a non-zero GROM pattern; `video.fgbg.machine-*` zeroes `mem.grom` in the created instance so the machine path agrees with D-19 (GROM zero in CI) and with F-A4/F-A5's graphics-ROM cards.

**3. [Interpretation] `ivs_stic_render_card_row` and `ivs_stic_render_plane` gained a `const ivs_stic_state *` parameter** (they had no caller outside `stic_frame.c`), and `ivs_stic_render_cards_row` (the row form) was added beside the plane form the plan names; the machine uses the row form.

**4. [Interpretation] The frame generator emits the plane as 96 text rows (not 15,360 numbers)**, converted by the test, to keep the include small and the CMake loop cheap; text line numbers of the literal travel with each cell and plane for the mismatch printout.

**5. [Rule 3] Collateral prefix** `video.fgbg.` added to the allowed list of `STIC_EVENT_ORDER_START_BEFORE_END` (see above); the plan's `files_modified` lists `data/expected/faults/video.ids` already.

## Decisions Made

- No requirement is closed: VID-04 stays Pending (Color Stack is plan 06-11, the runner scenarios and end-to-end evidence plans 06-12 to 06-14).

## Open Items

- `video.frame.cs-not-decoded` pins the Color Stack interim; plan 06-11 inverts it.
- The renderer's cut-off column is only exercised through the unit planes and the cropped slot; no host-visible 160th column exists (D-12 stands).
- Hosted `r` for this plan is not yet known; the arbiter is the first hosted row.
- Carried unchanged: the 06-01 second-read records, Phase 4 and 5 deferrals, `sh tools/hygiene.sh --history` on earlier planning commits.

## Final gate

```
sh tools/gate.sh:       gate: PASS steps=36/36 controls=332/332 ... faults=218/218 ... tests-expected=4254 tests-executed=4252 tests-skipped=2 tests-missing=0
dash tools/gate.sh:     the same summary line (identical md5 fad25407722c0995f757355effd6eae2 as the sh run)
sh tools/gate-linux.sh: GATE_LINUX_PLATFORM= run, exit 0, last line of build/gate-linux.log starts gate: PASS, x86_64 gcc 13.3.0 cmake 3.28.3, steps=36/36 controls=332/332 faults=218/218 tests-missing=0 (gate-time total=331)
```

## Known Stubs

Display-on Color Stack frames are every position 0xFF until plan 06-11 (documented in `stic_cards.c` and `stic_frame.c`, pinned by `video.frame.cs-not-decoded`); no other stub.

## Threat Flags

None new. T-06-14 is mitigated: the renderer uses only `ivs_memsys_stic_gfx_byte_internal` and the row latches (`grep -c ivs_memsys_read core/src/stic/stic_cards.c` is 0), `video.meta.purity` shows the memory system byte-equal after two renders, and the machine-path frames run with the graphics windows closed. T-06-SC: no installs.

## Self-Check: PASSED

Created files exist (`core/src/stic/stic_cards.c`, `cmake/IvsVideoFrames.cmake`, the six `tests/video` files); commits `29ec54a`, `a5c681e`, `a9825a8` are ancestors of HEAD; `grep -c '^video.fgbg.F-A[1-5]$' data/expected/tests/video.ids` is 5 and `grep -v '^#' data/stic/tests-pending.ids | grep -c ' 06-10$'` is 0.
