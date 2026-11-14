# Phase 8 discuss, decision S-1: VID-05 STIC completion (research agent A)

Labels: [OBS] read or measured this session, [INF] inference, [REC] recommendation, [UNK] unknown,
[BG] background knowledge not re-verified. Read-only on the repo except this file. Web pages were read
through a fetch tool that returns a model summary, not raw text: every web fact below that is not a page of
the repo cache is marked `(fetch summary)` and the plan that writes the oracle must re-read the raw page
before a value is frozen. No emulator source file was opened (only search-result snippets of a tier 5
source were seen; they are hints, never used for a value, and need no CONSULTATIONS row because no file was
read). Two documents of a GPL emulator project (documentation, tier 4 leads under `docs/provenance.md`) were
read as fetch summaries.

## 0. Recommendation on one screen

Success criterion 1 allows two outcomes per feature: a single-feature fixture with an independently derived
expected result, or an explicit `unknown-default-chosen` entry. The honest reading of the sources is that
**no tier 1 to 3 page prints any MOB, collision, delay or border-extension bit, so every VID-05 expected
value is a tier 4, single-lineage derivation (tier `source-expectation`, claims `provisional`), exactly the
standing VID-04 was closed on.** The value of Phase 8 here is therefore (a) the semantics where the
community text is unambiguous, derived by hand before the code, and (b) a pinned, visible list of the
rest. Recommendations:

- **S1-1 [REC] Do both, per feature, by a split of each feature into a derivable core and a pinned remainder.**
  Derivable (tier 4 text is explicit, one lineage, tier 2 or 1 corroborates existence): MOB placement with
  the 8-pixel field offset, X zoom, Y zoom 2x/4x/8x, 8x16 two-card mode, X/Y mirror, color, MOB over MOB
  by lower index, MOB over background with PRIO clear, sticky collision bits for MOB/MOB and MOB/background,
  the own-bit-never-set rule, delay shifting cards and MOBs together by one pixel per step, border
  extension filling an 8-pixel strip with the border color. Pinned unknowns (list in section 3, ids C107
  onward are the planner's to allocate): PRIO-set pixel semantics, collision clear/read/X=0/offscreen/
  border-bit/display-off rules, the content and unit of areas exposed by delay or the top strip, the delay
  range, delay versus the bus-window count, and half-height (1x) pixel publication.
- **S1-2 [REC] Resolve the geometry fork before anything else: keep the published plane at 159 x 96 this
  phase (no D8 reversal, no re-bless of any golden), but make the MOB model line-resolved internally and publish by a
  labelled sampling rule.** Finding (section 2.2): a MOB pixel with Y zoom 1x or the 8x16 mode is one TV
  line high while a plane row is two lines, so 159 x 96 cannot hold it. Rendering and collision evaluate at
  line resolution (16 lines per card row); the published plane takes the even line of each pair
  (`unknown-default-chosen`, one profile field `video.mob_line_sample`); expected frames are derived only for
  sampling-invariant configurations (zoom 2x, 4x, 8x), plus exactly two pinned sampling frames. Reversal
  evidence: an owner or host need for 192-line output; then publish both lines (D8 reopened, a separate
  phase, about two plans plus a re-bless of every frame digest).
- **S1-3 [REC] One cartridge program with a selector, as `ivs-diag-stic-1` does, and five scenarios.**
  `ivs-diag-stic-2` reads the selector from a controller word at tick 0 and enables exactly one feature
  (MOB, border extension, horizontal delay, vertical delay, collision), so each scenario is a single-feature
  fixture. The many sub-cases (sizes, flips, priorities, bit-role sweeps) are in-process table tests of a
  pure renderer against hand frames, which is the cheap path (no process spawn).
- **S1-4 [REC] Oracle first, Phase 7 method, no C in the first plan:** derivation document with a blind-spec
  block, frame literals `F-M*` with `sha256-text` keys and `-` for every unknown cell, a collision truth
  table, declared frame pairs for the pair-region token, blind second read from the wiki figures and a
  fresh agent (not from the doc the author used), commit order recorded.
- **S1-5 [REC] Collision registers stay in the register file; no new hidden state.** Collision bits are ORed
  into `stic_reg[0x18..0x1F]` by the FRAME_END handler whether or not a buffer is present; the CPU can clear
  by write; a read has no side effect (same as VID-06 purity). Then the Phase 8 snapshot checklist gains no
  STIC item, and slicing and buffer independence are provable with the existing slice and buffer suites.
- **S1-6 [REC] Do not change the bus-window schedule for delay.** Tier 1 text says 13 or 14 row requests
  "depending on picture offset" and a tier 4 doc gives 13,518 versus 13,572 usable cycles for vertical
  delay 0 versus above 0, so the schedule probably depends on vertical delay. Keep the Phase 6 schedule
  (C92) and pin "no dependence" as `unknown-default-chosen`, with a new owner-hand-off line. No VID-05
  assert may depend on a machine time.
- **S1-7 [REC] Guest programs never depend on undriven bits.** Mask every collision read with `03FF`. A
  tier 4 hardware-derived note (fetch summary, section 2.4) says bits 15:14 of STIC reads are 0 and other
  unimplemented bits 1, which differs from the Phase 6 default `FC00 OR stored` (C81). Log it as a C81
  update, do not act on it.
- **S1-8 [REC] Cost: plan 6 plans, at most 4 named core faults, 6 program mutants, 4 gate controls, no new
  installed-consumer replay, mutants on the release preset only** (the pre-committed lever 2 of
  `.planning/CI-EFFICIENCY.md`, taken up front). Estimated cost 8 to 10 hosted cpu-s, +0.2 to +0.25 in `r`
  against a headroom of 0.5 to the 8.3 trigger (median 7.78 over the last five rows) shared with STA and PRF.
- **S1-9 [REC] Five new owner-hand-off lines, none performed, no claim above `source-expectation`:**
  collision read/clear semantics, priority per pixel kind, delay range/exposed area/bus windows, border
  extension units and the border collision bit, half-height line structure.
- **S1-10 [REC] Claims: three, all `provisional`:** CL-0025 pictures (MOB, border extension, delay,
  `source-expectation`), CL-0026 collision (`source-expectation`), CL-0027 the diagnostic
  (`original-diagnostic`). Each prints its exclusions (no hardware, no priority-set pixel rule, no delay
  timing, no half-height publication).

## 1. What Phase 6 already covers and what VID-05 lacks

[OBS] files read: `core/src/stic/stic.h`, `stic_regs.c`, `stic_frame.c`, `stic_cards.c` (header only),
`data/stic/*.literal.txt`, `data/stic/rules.json` (ids), `docs/stic/reference-notes.md`, `docs/stic/timing-notes.md`,
`data/discrepancies.json` (C10, C81, C86 to C89, C97 to C101, C105, C106), `data/claims.json` (CL-0016 to CL-0022),
`docs/video/frame-derivations.md` and `diag-stic-1-derivation.md` (headers), `data/video/frames/*`,
`.planning/phases/07-*/07-DISCUSS-verify.md`, `07-CONTEXT.md`, `.planning/CI-EFFICIENCY.md` (Phase 7 close),
`.planning/ci-runs.tsv`, `data/expected/d10-measurements.ids`, `data/sources.json`.

| Item | State after Phase 6 / 7 [OBS] |
|------|-------------------------------|
| Register file | 64 words, widths from the project literal: MOB X `07FF`, Y `0FFF`, attribute `3FFF`, collision `03FF` (read back as `FC00 OR` stored), delay `0007`, border extension `0003`, border color `000F`. Stored, masked, windowed, aliased. Roles and addresses tier 4 (STIC-R09), widths and read-back `unknown-default-chosen` (C81, C86). |
| Picture | Cards only (FGBG, Color Stack, Colored Squares). Rule STIC-R28: "MOB, delay and border-extension registers are stored and not applied." `in.border` and `$2C` are used only for display-off fill (C99). |
| Collision | Stored values only; no set, clear or sticky rule (C87 "pending: Phase 8", evidence says no test). |
| Delay, border ext | Stored, ignored. C105 "pending: Phase 8": no border, no origin, `video.row_lines = 2` metadata. |
| Geometry | 159 x 96 index bytes, 1 byte per position, `0xFF` = not displayed; rows are 2 TV lines (C10, D8). |
| Timing | Frame 59,736 ticks; 13 bus windows at pitch 912 cycles from 3,791 (STIC-R31, C92); the note says a 14th window "needs a different placement (Phase 8, the picture offset) and is not modelled". |
| State | `ivs_stic_state` 528 bytes (cursor, latch, mode, `cs_ptr`, 12 x 20 row latches). Render is a pure function of regs + latches + GRAM at E_k (CR-17); `ivs_stic_frame_deliver` takes `const ivs_memsys *`. |

**Missing for VID-05 (exact list):**
1. MOB decode and compositing (position, zoom, flip, 8x16 mode, color, priority) with a graphics-byte path for
   MOB cards (the existing `ivs_memsys_stic_gfx_byte_internal` serves cards; MOB card numbers use bits 3..10
   per the tier 4 layout, so a MOB decode helper is new).
2. Collision computation and a write-back path into the register file at FRAME_END (the deliver function is
   `const` on memory today; the catch-up function holds the non-const pointer).
3. Delay shift, border-extension strips and a rule for the exposed area (none exists; `video.row_lines` unused).
4. Pinned defaults and tests for everything the sources do not state, and the discrepancy entries for them.
5. The bus-window question for vertical delay (log only).
6. Fixtures: none exists for any of these (`FX-diag-stic-1` draws cards only).

## 2. Primary-source survey per feature

### 2.0 Sources and pages read this session

| Id | Tier | URL | What I read |
|----|------|-----|-------------|
| SRC-gi-1980-catalog / SRC-gi-1982-catalog | 1 | local cache `refs/` (bitsavers originals) | text layer of pdf 191 to 193 (1980) and 462 to 464 (1982), AY-3-8900/-1 pages 5-10..5-12 and 9-43..9-45 [OBS]. Word search for `collision`, `coinc`, `interact`, `priority`, `reflect`, `border`, `delay` finds none; `zoom`, `half`, `offset`, `object` find the lines in 2.1 [OBS]. |
| SRC-bsr-2609 | 2 | https://history.blueskyrangers.com/hardware/2609.html | whole page (fetch summary) [OBS] |
| SRC-community-emu-docs | 4 | https://raw.githubusercontent.com/FujiNetWIFI/jzintv/main/doc/programming/stic.txt and `.../interrupts.txt` | both docs (fetch summary), GPL project, documentation only. The first has no author or date line in the fetched text. One lineage with the wiki. |
| SRC-intv-wiki | 4 | https://wiki.intellivision.us/index.php/STIC (oldid 15008, 14 Aug 2012; cached `refs/stic/stic-15008.txt`) and `.../VBlank_Interrupt` (oldid 15023) | STIC page: register table and prose [OBS from cache]; bit layouts exist only as images that Phase 6 did not fetch for MOBs [OBS]. |
| TutorVision/INTV88 notes (new, would be `SRC-intv88-re-notes`, tier 4) | 4 | https://docs.google.com/document/d/13ApQEm1Xw_DcCAaXcQvpFot_BvrzSzeUo95EXCf41uc/pub | first 100,000 of 103,513 characters (fetch summary). Same author as the jzIntv docs: **same person, not a second lineage**; method differs (hardware observation on STIC1A). |
| Atari patent US 4,471,464 | n/a | https://patents.google.com/patent/US4471464A/en | read to test a patent route: it is Atari's 1979 display-list graphics generator, no STIC content [OBS]. |
| Searches | | web search x6 | no STIC patent, no decap or die analysis of the AY-3-8900-1 collision or priority logic, no GI or Mattel programming manual scan found [OBS, absence of evidence]. The Mattel PAL colour-circuit requirement paper (papaintellivision.com pdf, 1980) and the console5 datasheet mirror (HTTP 403) were not read; the mirror is the same GI text already read in the cache [INF]. |
| Tier 5 | 5 | search snippets of the MAME STIC file header and an AtariAge thread | hints only, not re-derived: "all collisions are latched", an object hidden behind the background still registers a collision, border bit; delay fields are 3 bits [BG-level]. Not used for any value below. |

### 2.1 What tier 1 and tier 2 say (existence only)

- [OBS tier 1, pdf 191 / 462] "8 coordinate addressable foreground objects on a grid of 168H by 104V (PAL)
  or 167H by 105V (NTSC) of which 159 x 96 are visible positions"; "foreground objects independently
  programmable for half height, y zoom, x zoom and 8 or 16 character lines high"; 16 colors; the 8-bit
  pattern memory; "SR2 is issued thirteen or fourteen times per picture frame depending on picture
  offset". The page prints no register, no bit, no collision, no priority, no delay, no border text
  (`registers-printed: no`, Phase 6 verdict, re-confirmed by the word search above).
- [INF] 167 - 159 = 8 columns and 105 - 96 = 9 rows of object field outside the visible window; the tier 4
  offset of 8 left and 8 above is consistent with the 8 columns (a partial corroboration of the offset, not a
  proof). "Depending on picture offset" ties the 13 or 14 requests to the delay registers.
- [OBS tier 2, BSR page (fetch summary)] 8 objects, each 8 x 8 or "8 wide by 16 half-pixels high" (the
  double-resolution mode), each doublable in width, height or both, one color per object, objects can sit on
  a grid larger than the display; the STIC "tracks interactions between moving objects and between objects
  and the background, and signals the CPU", and the CPU can ask about an object leaving the playfield edge
  or intersecting another (three collision classes, no register or timing); the whole background can be
  scrolled smoothly "one pixel at a time" horizontally or vertically and "moving objects move with the
  background"; display blacked out while the EXEC fills BACKTAB. The page says nothing on priority,
  delay registers, border or interrupts.

### 2.2 MOB position, size, priority, color, mirror

[OBS tier 4, stic.txt, fetch summary; one lineage; to be re-read raw] X register bits 0..7 position, bit 8
interaction enable (INTR), bit 9 visible (VISB), bit 10 double width; Y register bits 0..6 position, bit 7
8x16 mode (YRES, the image comes from two consecutive cards), bits 8 and 9 Y size, bit 10 X flip, bit 11
Y flip; attribute bits 0..2 color, bit 3 card LSB (ignored in 8x16), bits 4..10 card number (bits 9, 10 ignored
for GRAM), bit 11 GRAM select, bit 12 color bit 3, bit 13 PRIO. Object field starts 8 pixels left of and 8
above the display; X = 0 disables the MOB; Y = 0 is not special; (1, 1) is the least position showing a pixel of an
8x8 MOB. Heights: 4, 8, 16, 32 pixels with YRES clear and 8, 16, 32, 64 with YRES set. MOB over MOB: the lower
index is in front and PRIO does not matter; MOB against background: PRIO set puts the MOB behind, clear in
front, "only for visible MOB pixels". The register widths of the Phase 6 literal (11, 12, 14 bits) agree
with this layout [OBS, by comparison].
[OBS tier 4, wiki prose] up to 8 MOBs, 8x8 or 8x16, X stretch 2x, Y stretch 2x/4x/8x, "each MOB pixel is
displayed ... half the height of the background pixels", mirror horizontally, vertically or both;
colored-squares colors 0..6 interact with MOBs and 7 does not. Bit figures are images not yet cached.

**Finding (geometry) [INF from the two readings above and the 96 x 2-line plane]:** the Y size table implies
that the unit "1x" is one TV line (half a plane row), and the Y position unit is a plane row (7 bits cover the
96 visible rows plus the 8-row offset, so it cannot be a line: 192 lines would not fit). Therefore with
Y zoom 2x, 4x and 8x one pattern row covers exactly 1, 2 and 4 whole plane rows (an 8-row card covers 8, 16
and 32 plane rows), while with Y zoom 1x (and the 8x16 mode at 1x) two pattern rows share one plane row. The published
159 x 96 plane cannot show them without a sampling rule. The native resolution is 159 x 192 for the AY-3-8900-1
(and 160 x 192 on the STIC1A per the INTV88 notes). This is the one place where the Phase 6 decision D8 meets
VID-05; the options are in S1-2 and section 4.

**Not stated, so pinned (section 3):** what "behind the background" hides (the card's foreground pixels only,
or the whole card including its background-color pixels: the fetch summary is ambiguous and the raw text decides);
whether the MOB color is masked to 3 or 4 bits in FGBG vs Color Stack; what happens to a MOB of size beyond the
field; the pixel stride when X zoom and X flip combine (derivable once the layout is fixed).

### 2.3 Collision registers

[OBS tier 4, stic.txt (fetch summary)] bits 0..7 collision with MOB 0..7 (a MOB's own bit is never set), bit 8
background, bit 9 border, bits 10..13 unused; bits are sticky until the CPU clears them, even if INTR is later
cleared; the CPU can write them; interaction needs INTR set, for MOB/MOB on both MOBs; visibility is not
required (an invisible MOB with INTR still collides); "on" background pixels count; colored-squares colors 0..6
count and color 7 does not. The doc does not say whether a read clears, whether an X = 0 MOB collides, what
"border" means exactly, or when in the frame the bits are set. [OBS tier 4, AtariAge thread via search, a
forum post] the registers can only be used after the frame has been displayed and are copied in the interrupt
routine [INF: consistent with the STIC register window].
[OBS tier 2] three classes exist. [OBS tier 1] nothing printed. [tier 5 hint, unverified] latched; hidden
MOBs still collide; the 160th column may matter for the border bit (the INTV88 notes call this speculation).
[INF] the sticky and write-to-clear reading is consistent with the Phase 6 register file (collision rows
store a 10-bit value that the CPU can write), so no register-file change is needed.

### 2.4 Border, blanking and extension

[OBS tier 4, stic.txt] `$2C` border color 0..15; `$32` bit 0 extends the left edge and bit 1 the top edge by an
8-pixel strip filled with the border color, which "hides the blank object-field area". [OBS wiki VBlank page
(fetch summary)] the interrupt comes as the active display ends "as the solid bottom border begins" [tier 4
prose only]. [OBS BSR] blackout during EXEC fill (already C99). [UNK] the unit of the top strip (8 plane
rows, 8 lines, 4 plane rows), the colour of unextended blank area, whether the strips are fixed to the display
or move with delay, what the border collision bit means.
[OBS tier 4, INTV88 notes (fetch summary), observation on STIC1A] addresses `$0000` to `$003F` "mostly
respond like an AY-3-8900-1", bits 15:14 read 0 and other unimplemented bits read 1 (one exception, bit 3 of
`$0022`), `$0040` to `$007F` read `$FFFF`; the author does not give the method. [INF] if raw-read this
differs from C81's `FFFF`/`FC00` model in the two top bits: a C81/C86 update, not a VID-05 oracle value.

### 2.5 Horizontal and vertical delay

[OBS tier 4, stic.txt] increasing `$30` moves the object field right and `$31` down relative to the display;
cards and MOBs move together; no bit width or range is given. [OBS tier 2 BSR] background scrolls "one pixel at
a time", objects move with it. [BG, not verified here] 3-bit fields (0 to 7) are common knowledge and match the
Phase 6 mask `0007`; a tier 5 snippet agrees. [OBS tier 4, interrupts.txt (fetch summary)] usable CPU cycles
per NTSC frame 13,518 at vertical delay 0 and about 13,572 above 0, PAL marked "??"; horizontal delay not
discussed. [OBS tier 4, INTV88 notes] on STIC1A bus-copy time grows with vertical delay, on the AY-3-8900-1 it
"allows about the same regardless"; "whether MOBs are visible in rows exposed by vertical delay > 0" is listed
by the author as still to be measured [measured-or-inferred split not stated]. [OBS tier 1] 13 or 14 SR2
requests "depending on picture offset".
[INF] the exposed area is exactly what the border-extension strips (8 wide, delay at most 7) would hide, a
coherent but unproven design story. The vertical-delay rows are an owner-measurement item.

### 2.6 Interrupt and VBLANK interaction

[OBS repo] all 64 registers share the Phase 6 register window and its blocked-access policy (STIC-R07, R24):
MOB, delay, extension and collision writes and reads follow it without new code. [INF] a render at E_k reads
registers frozen over graphic time (CR-17), so a frame is a pure function of the register values at its start
window; collision write-back at E_k precedes any access stamped E_k (STIC-R34, C104), so the CPU sees the
bits of frame k in window k+1 and can clear them there. The display-latch-clear frame (frame 0, display
off) renders no MOBs and collides nothing (default, pinned). The schedule question for vertical delay is
S1-6.

## 3. Derivable versus `unknown-default-chosen`

Standing for every "derivable" row: tier `source-expectation`, basis tier 4 text read by one author, claims
`provisional` by the weakest-field rule, no hardware claim, D10 untouched.

| Feature | Derivable core (expected value derived by hand before the code) | Pinned remainder (default chosen, pinned by a `*.default-pinned.*` test and a one-field flip control) |
|---------|------------------------------------------------------------------|------------------------------------------------------------------------------------------------------|
| MOB position | Field offset 8/8: X = 8 puts the MOB's first pixel on visible column 0 (not X = 0, which disables); clipping at the 159 x 96 edges; one MOB, solid GRAM card, zoom 2x. | Beyond-field positions and wrap of the 8-bit X (unknown). |
| MOB zoom, 8x16 | X 2x; Y 2x/4x/8x; 8x16 as two cards (card LSB ignored). | Y 1x and 8x16 at 1x: published by the even-line rule (C109-like); bit width of Y size beyond 2 bits none. |
| Mirror | X flip, Y flip, both, on an asymmetric 8 x 8 test card (derivation: row and column reversal, checked by conservation of set pixels). | Flip with 8x16 and zoom together: derivable but one lineage; list the combined case as a second frame, not a separate claim. |
| Color | 3-bit color plus bit 12 as bit 3 (16 colors) on a card of solid foreground. | Whether bit 12 applies in both card modes: derivable from the layout; flagged single lineage. |
| Priority | MOB over MOB: lower index wins (two overlapping MOBs with different colors). MOB over cards with PRIO clear: MOB in front of everything. | PRIO set over foreground "on" pixels vs card background pixels (default: behind on pixels only, background-color pixels do not hide the MOB; pin and mark single-lineage, ambiguous in the fetched text); colored-squares interaction. |
| Border extension | Left strip 8 columns, top strip, filled with the `$2C` color, over cards and MOBs [INF: over MOBs unverified, pin]. | Unit of the top strip (default 8 plane rows), whether strips move with delay (default: fixed to the display), colour of a blank area when the extension bit is clear (default: the not-displayed value `0xFF`, never a colour). |
| Horizontal delay | Cards and MOBs move right one plane pixel per step; content shifted past the right edge is clipped. | Range (default: 3 bits, `0007`), content of the exposed left columns (default: not displayed, `0xFF`, or the border color when extension bit 0 is set). |
| Vertical delay | Cards and MOBs move down one plane row per step; same clipping. | Range, exposed top rows (same rule), MOBs in exposed rows (default: not drawn), and **no change of the bus-window schedule** (C92 update). |
| Collision | MOB/MOB and MOB/background (on pixels, colored-squares 0..6 count, 7 does not) bits 0..8; own bit clear; INTR required on the colliding MOBs; visibility not required; sticky across frames; CPU write replaces the value (masked `03FF`), so a write of 0 clears. | Read side effect (default none), X = 0 MOB (default: no interaction), border bit 9 meaning (default: any MOB pixel inside the 167 x 105 field that is outside the 159 x 96 window), off-screen MOB/MOB collisions (default: detected over the whole field), display-off frames (default: no render, no collision), the set instant (default: at FRAME_END, visible in the next window), registers decay (C88 unchanged). |

Notes:
- Priority-set semantics, the border bit, and the delay exposed area are the items most likely to be wrong;
  each must carry a one-field reversal in the profile (the C10, C97 pattern) so a later measurement flips one
  field and one golden set.
- [REC] Frames use `-` for unknown cells (the existing "not displayed" sentinel) so that an unknown is
  never a color that could be hashed into evidence; those frames are asserted by `pixel` probes in the runner
  (`hash-needs-colors-only`) and by full-plane compare in process. The extension-set variants are fully colored
  and hash-assertable.
- [REC] Collision truth tables are derived on solid cards at zoom 2x so the answer is independent of the
  half-height sampling rule.

## 4. Scope split, reuse and cost

### 4.1 Plans (proposed; the orchestrator owns numbering)

| # | Plan | Content | C code |
|---|------|---------|--------|
| A | Oracle (no C) | Raw re-read of stic.txt/interrupts.txt, wiki MOB figures cached, `SRC` entry for the INTV88 notes, rules `MOB-R*`/`COL-R*`, the derivation document with a blind-spec block, frames `F-M1..F-M8`, collision truth table, pairs, blind-read request, discrepancy seeds, claim reservation, D10 lines | none |
| B | MOB renderer | pure renderer (MOB decode, compositing, line-resolved with the sampling rule), profile fields, `video.mob.*` unit tests incl. a 64-probe bit-role sweep, 2 faults | yes |
| C | Collision | accumulation at FRAME_END, write-back path (the `const` seam), clear by write, slicing, buffer independence, peek purity, 2 faults | yes |
| D | Border extension and delay | strips, shift, exposed-area rule, C92 pin, 1 fault | yes |
| E | Program and scenarios | `ivs-diag-stic-2` (one listing, manifest `redistributable: yes`), five scenarios plus one idle control, `.expect` files, goldens, 6 mutants, shape lint reuse | none |
| F | Close-out | claims CL-0025..27, discrepancy final states, important-gate rows, traceability, registry, CI close-out | none |

[INF] 6 plans against 14 (Phase 6) and 12 (Phase 7). The other Phase 8 groups (STA, PRF, REL) are separate
decisions; this is VID-05's share. If S1-2 is overridden to publish 192 lines, add two plans before B (geometry
and the re-bless of F-A*, F-B*, F-D*, F-S*, the consumer golden, the chain and CL-0018) and expect +0.15 in `r`
from the larger frames [INF].

### 4.2 Reuse of Phase 7 machinery [OBS]

`ivs_asm` (`tools/asm/asm.c`) and the manifest/listing pattern of `FX-demo-lamp-tone-1`; boot stub contract 2
(project ISR entry `$5004`, display sustained by a strobe per frame); the `.ivss` grammar frozen (no new assert
subject); `frame <n> pixel|hash|displayed` against `.frame` literals with `sha256-text`; the `video-frame-pair-region`
token (`data/expected/video-frame-pairs.ids`) for "a feature changes exactly the declared rectangle" (for example
F-M1 versus the blank card frame); the scenario shape lint (a selector input at tick 0 gives the "input" clause);
the program-mutant inventory with `fails=` sets; the settling rule (no frame assert in the frame of the input or
the next; margins of 10,000 ticks to a frame end); constant-level cost knowledge (a scenario of 1 to 3 M ticks
costs 10 to 60 ms). The sustaining ISR of Phase 7 must re-write the MOB registers each frame only if the
fixture needs them to change (collision clear); the others write once in frame 1.

### 4.3 Fixture inventory

- Program: 1 (`ivs-diag-stic-2`), about 150 to 200 words, selector at `$02F0`-style markers, GRAM cards
  written in frame 0 (no GROM, so no third-party image; patterns authored here).
- Scenarios (single feature each, selector via the controller word at tick 0): `stic2-mob`, `stic2-border`,
  `stic2-hdelay`, `stic2-vdelay`, `stic2-collision`; plus `stic2-idle` (selector 0: no feature enabled, frame equals
  the plain card frame; the negative control that the program, not the core, adds nothing).
- Frame literals: about 8 (`F-M1` MOB zoom 2x placement/clip, `F-M2` flips, `F-M3` priority and color, `F-M4`
  border extension, `F-M5` hdelay with ext, `F-M6` vdelay with ext, `F-M7` hdelay without ext (probes only),
  `F-M8` collision scene) plus the two pinned sampling frames.
- Markers asserted for collision: the ISR stores `collision_k AND 03FF` for k = 0..2 at frames chosen with the
  settling rule; derived values: bits from geometry only (for example MOB0 and MOB1 overlapping gives
  MOB0 bit 1 and MOB1 bit 0; the background bit only if the overlapped card has "on" pixels).

### 4.4 Cost against the guideline [INF using `.planning/CI-EFFICIENCY.md` unit costs]

Last five hosted rows (ids 43373151931, 43528527276, 43540936641, 43546632092, 43551481419): `r` = 8.17, 7.73, 7.78,
8.37, 7.20, median 7.78, budget 7.8, revisit above 8.3, build about 40 s [OBS from `ci-runs.tsv`]. The headroom
to the trigger is about 0.5 in `r`, about 20 hosted cpu-s, to be shared with snapshot and benchmark work.
Unit costs: fault about 1.0 hosted cpu-s (0.5 local times the 2.0 hosted factor), control about 0.15, id about 0.007, runner-process
case pair about 0.2 (Phase 7 measured 14 pairs at about 3 hosted cpu-s, `r` +0.05).

| Item | Count | Hosted cpu-s |
|------|-------|--------------|
| Core faults (MOB priority swap, collision not sticky or own bit set, delay direction, flip) | 4 | 4.0 |
| Gate controls (pair-region drift, frame literal drift, shape, selector) | 4 | 0.6 |
| New ids (tests, literals, rules, frames) | about 180 | 1.3 |
| Runner-process cases (6 scenarios + 6 mutants, release only for mutants) | about 9 pair-equivalents | 1.8 |
| In-process suite additions | 40 to 60 cases, pure | about 1 |
| Total | | **about 8.7** (range 7 to 11), `r` +0.2 to +0.27 |

Pre-committed fallback order if a checkpoint reads `r` above 8.3: (1) drop the sampling-pin frames to in-process
only, (2) fold two mutants, then the owner-decision levers already queued (`-O0` fault objects, a two-job
split). Not levers: dropping an oracle, the judge or a pinned-default test. Rule: a plan stops adding tests if
the last-five `r` is 8.0 or above at its checkpoint, as in Phase 7.

### 4.5 Risk of blowing the plan count

- Geometry: S1-2 avoids the re-bless; if the orchestrator prefers 192 lines the count is 8, not 6.
- Selector program: one program keeps manifest, listing and mutants to one set; a bug in the selector path
  affects all five, so the idle scenario and a per-scenario "feature off" assertion (other features' pixels
  absent) are part of each `.expect`.
- Collision write-back changes the video path from read-only to mutating memory; the sliced-run equality
  suites exist and must be run on it, but a design that leaves the register file as the only state costs
  no extra plan.
- Scope creep: PAL, the 160th column (D10-VID-01), Intellivision II / STIC1A differences (alias response,
  GRAM bits, bus-copy time) stay out; each is a one-line discrepancy.

## 5. Adversarial pass

1. **A wrong default silently becoming evidence.** The risky ones: PRIO-set semantics, border collision
   bit, the exposed-area fill, the top strip unit, collision clear. Guards: every default has a discrepancy
   entry prefixed `unknown-default-chosen:` with a resolver; a test whose name carries `default-pinned` asserts the
   default, and a profile-field flip control shows it can fail; no `.expect` assert, frame hash or collision
   marker depends on a pinned item (unknown cells are `-`); the claim text lists them as excluded; the
   important-gates registry names the pinned-default tests. A reviewer-visible table maps every pinned item to
   an id. Residual risk: a pinned default that is also the community's de facto behaviour will look like a
   confirmed one; the tier word `source-expectation` and the claim exclusion are the only protection.
2. **Shared misreading.** One author wrote the rules, the program and the core; the sole text for every bit is
   one lineage (the wiki figures and the doc are by the same community, and the INTV88 notes are the same
   person). Mitigations: blind second read of the bit roles from the wiki MOB figures (to be fetched into
   `refs/stic`) by a fresh agent that never sees the first derivation; a 64-probe bit-role sweep per MOB register in
   process (as `video.probe.*`); asymmetric test cards so a flip or a bit-order error changes the frame; the
   pair-region check on the oracle files. State plainly that agreement is transcription agreement, not
   console behaviour (C43 wording).
3. **Tier 5 contamination.** The collision latching and "hidden MOB still collides" hints came from a snippet
   of an emulator comment. Do not let them become expected values: the tier 4 doc already supplies the
   "visibility not required" rule, so the derivable core does not depend on the snippet; the hidden-behind-
   background case is pinned, not derived.
4. **Rights.** No page text, figure or table is committed; the project literal is written in its own grammar
   (as the regmap literal) with facts only; no emulator identifier in code, data or docs (the denylist step);
   cards are authored patterns in GRAM, no GROM or EXEC image; the fixture manifest records `redistributable:
   yes` for the original program. The jzIntv documents are GPL documentation read as tier 4 leads; add a
   `docs/provenance/CONSULTATIONS.md` courtesy row for the two documents and for the INTV88 notes. Fetched
   figures and PDFs go only in the gitignored cache.
5. **D10 boundary.** Nothing in this decision measures hardware. The INTV88 notes contain "measured" statements
   by a third party on STIC1A units: they are tier 4 reports, never `measured-hardware`; they do not apply to the
   AY-3-8900-1 of the profile without an owner measurement. New hand-off lines proposed for
   `data/expected/d10-measurements.ids` (planner allocates ids): collision read/clear and set instant; MOB
   priority per pixel kind and border bit; delay range, exposed area and bus windows; extension units; half-height
   line structure. Each line names its discrepancy; none is performed.
6. **Peek purity (VID-06).** Collision registers must remain readable through the stored view with no
   change; a read that cleared would break CL-0021. The default "read has no side effect" is also the safe
   direction for the peek sweep.
7. **Hidden state for STA.** Any per-frame accumulator that spans frames and is not a register would join the
   hidden-state checklist. The design has none (collision bits are registers); keep it that way, and say so
   in the snapshot discussion.
8. **Why not skip rendering and list everything as unknown?** Legal under the criterion, but then MOB-using
   content could never show a sprite, which defeats "useful". The criterion's second clause is the fallback if
   plan B or C slips: the pinned list is written first (plan A), so slipping a feature costs only its
   fixture, not the honesty of the record.

## 6. Reversal evidence, open items

- Reverse S1-2 (publish 192 lines) if a host integrator or the owner needs half-height pixels, or if the
  even-line sampling default shows a visible artefact in a consumer; then D8 reopens.
- Reverse "no schedule change" (S1-6) only on an owner measurement of the bus windows against vertical delay
  (D10-VID-07 extended), or on a tier 1 to 3 page that prints window placement.
- [UNK] raw text of stic.txt (the fetch tool gave a summary; it flags no uncertainty and gives no date); the
  wiki MOB register figures; the unread last 3,513 characters of the INTV88 notes; whether the STIC1A "bits 15:14
  read 0" statement was measured on an AY-3-8900-1.
- [UNK] the numbers C106 onward: C106 is taken (assembler branch range); the planner allocates C107 and up.
- Checked but not found [OBS, absence only]: no STIC patent (the Atari patent found is a different chip), no
  die analysis or datasheet programming section for collision, priority or delay, no clear-on-read statement
  in any source seen. If a tier 1 to 3 page exists it is not on the open web paths searched.
