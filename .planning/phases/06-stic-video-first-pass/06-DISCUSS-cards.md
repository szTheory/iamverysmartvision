# Phase 6 discuss: background card rendering and card-fixture design (VID-04)

Status: recommendation (fan-out protocol, `.planning/AUTONOMY.md` section 2). Labels: [OBS] read in this session, [INF] inference, [REC] recommendation, [UNK] unknown, [BG] background knowledge not re-fetched.
Scope of this note (my decision point only): the BACKTAB word formats of Foreground/Background (FGBG) and Color Stack (CS) modes, the card lookup and card-to-pixel-index mapping on the card grid, color stack mechanics, what the card path draws when the display is off, how expected frames are hand-derived, the fixture grammar, faults, rules and vector plan. The register note owns `$0020/$0021/$0028..$002C` storage and access windows; the framebuffer note owns D8 geometry, row doubling, buffer layout and the frame boundary; the timing note owns BUSRQ/VBLANK. Where I depend on them, section 9 lists the assumption.

Inputs read [OBS]: AGENTS.md; AUTONOMY.md sections 1-3; PROJECT.md (Decision Register skimmed), REQUIREMENTS VID-03..06, ROADMAP Phase 6 (SC3, SC4, SC5); `data/discrepancies.json` (C1-C11, C20, C21, C43, C60, C61, C80); `data/memmap/*.literal.txt`; the profile JSON (memmap regions, `stic.part`, `power_on_fill.stic`, device kinds); `docs/RIGHTS.md`, `docs/fixture-manifest.md`, `docs/diagnostics.md`; Phase 5 CONTEXT and verify note (oracle-first pattern); `core/src/memmap/memsys.h` (backing arrays, gates, `ivs_memsys_poke_raw_internal`); `core/src/machine/machine.h` (instance layout, digest rows). Primary pages read directly from the cached GI 1980 and 1982 catalogs with `pdftotext` (printed 5-10, 5-16, 5-18 = pdf 191, 197, 199; the 1982 copy of the STIC section at printed 9-43 is the same text). Community pages fetched on 2026-11-12 into the scratch dir only: wiki STIC page (oldid 15008, last edited 2012-08-14) and its five word-layout images, wiki BACKTAB page (oldid 14739, last edited 2010-12-04), the Blue Sky Rangers 2609 page, and two community emulator-doc text files (`doc/programming/stic.txt`, `graphics_mem.txt`, read as documentation, not as code). No emulator source was opened. No file other than this one was written. No git or gh call was made.

## 0. Recommendation on one screen

1. **The primary pages do not contain the card word.** GI 5-10, 5-16 and 5-18 give the structure (20 x 12 matrix, 19 columns of 8 x 8 plus a 20th of 7 x 8, 159 x 96 visible, 240 x 14-bit words, 8-bit graphics memory, graphics use the lower 14 bits of the first 240 RAM words) and nothing about bit fields, GROM/GRAM select, color stack, bit order or registers [OBS]. Every field position, the stack rule and the bit order rest on two community texts that agree, plus consistency checks against the Blue Sky Rangers page. Treat the whole word layout as ONE tier-4 lineage and say so in every claim (R-3, R-17).
2. **Hard-code the layout, do not make it profile data.** One decode function with named constants in `core/src/stic/stic_cards.c`. What is profile data are only genuine unknowns (power-on mode, display-off fill). The independence comes from a bit-role literal written from the sources (56 rows) judged by a behavioural single-bit probe sweep, plus ~24 hand-derived single-feature vectors (R-8, R-12).
3. **The card renderer is a pure function** from `(mode, display_on, border, stack[4], BACKTAB 240 words, graphics bytes)` to a 160 x 96 card plane of STIC color numbers (0..15) with an optional "pixel set" plane and one not-displayed marker (ND) for the cut-off 160th column. No row doubling, no border, no palette, no scaling in it (R-1, R-2, R-9).
4. **Frame granularity at the frame boundary, from live state, no instance state added.** The function is row-shaped inside so a later row-latch is a drop-in, but Phase 6 renders the whole plane at the boundary. This keeps slicing invariance and Phase 8 snapshots free of an in-flight frame (R-4).
5. **No side effects, no CPU view.** The renderer reads memsys backing arrays through two dedicated pure accessors, never `ivs_memsys_read`/the bus, never through a gate. A vector with both gates closed proves it (R-5).
6. **Color stack:** four entries, position 0 at the start of every frame render, a card with bit 13 set advances `(pos+1) mod 4` BEFORE it is drawn and gets the new color, in raster order across rows, the cut-off column included. Both texts agree on timing; "reset every frame" and "cut-off column counts" are the weaker parts (R-10, R-11).
7. **Colored Squares** (CS word with bit 12 = 1 and bit 11 = 0) is implemented (about 15 lines) so that the CS decode has no hole; it is not in VID-04's text and has a documented cut line (R-13).
8. **Display off** draws the border color index over the whole plane and reads nothing: `unknown-default-chosen` (R-14). **Border area outside the cards, MOBs, collisions, delay, border extension: Phase 8** (section 8).
9. **Expected frames are literal text written by hand from the rule text before any renderer exists**, never produced by the core; ~24 vectors, a 56-row bit-role literal, 4 metamorphic properties, a blind second read, a test-only encoder that checks each hand-typed BACKTAB word against its field annotation (R-6, R-7, R-15).
10. **19 named faults, 8 controls, one new pure gate step `video-vectors`, one suite `video`.** Estimated +5..9 cpu-s, no new CI job (section 7).

## 1. Evidence: primary page vs tier 2 vs community, per claim [OBS]

Source labels: GI = GI 1980 catalog (the 1982 copy is identical for the STIC section); BSR = Blue Sky Rangers 2609 overview (tier 2); WK = wiki STIC page oldid 15008 and its images; JZ = community emulator-doc `stic.txt`/`graphics_mem.txt` (documentation text). WK and JZ are two texts but I count them as one lineage [INF]: layouts, wording of the rules and the quirks are the same, and nothing says one was checked against hardware independently.

| Claim | GI primary | BSR | WK / JZ | Verdict |
|---|---|---|---|---|
| Matrix 20 columns x 12 rows of 8 x 8 cards, 240 words of 14 bits | 5-10 yes ("20H x 12V", "240 by 14 bit words") | 240 cards, 20 x 12 | yes | tier 1 |
| 20th column is 7 pixels wide, 159 x 96 visible | 5-10 yes ("the 20th 7 x 8", "159 x 96 are visible") | "does NOT display the far right column", 159 x 96 | "rightmost column does not get displayed" | count: tier 1; which pixel column: tier 2 and 4 agree (rightmost, bit 0 side) |
| BACKTAB = first 240 words of the 352-word RAM at 512..; graphics use only the lower 14 bits; two MSBs free for the user | 5-18 yes | "240 locations ... (BACKTAB)" | `$0200`-`$02EF`, raster order `$0200 + row*20 + col` | extent and 14 bits: tier 1. Raster order: tier 4 and [INF] from 5-18 (address counter advances 20 per row, 12 rows, 240 words) |
| Graphics memory is 8 bits wide; GROM 2048 x 8; GROM CPU-view base 12288 ($3000), external graphics memory at 14336 ($3800) | 5-10, 5-16 yes | GROM 2K, GRAM 2 x 256 B, 64 GRAM cards | `$3000`/`$3800`, 8 bytes per card, `base + 8*card` | bases: tier 1. 8-byte stride and card count: tier 2 and 4 ([INF] from 2048 bytes = 256 cards) |
| Pixel row order and bit order in a card byte (byte 0 = top row, bit 7 = leftmost, 1 = foreground) | no | one printed glyph is consistent with string-order-equals-pixel-order, nothing explicit | JZ explicit | tier 4; BSR gives weak independent support [INF]. I do not copy that glyph (it is a GROM byte pattern) |
| FGBG word: fg = bits 2..0; card = bits 8..3 (6 bits); select = bit 11; bg = (b12, b13, b10, b9) = (bit 3, bit 2, bit 1, bit 0) | no | "colors for each card individually; one limited to the eight primaries" | WK image and JZ diagram agree | tier 4; non-monotonic bg order is the single most likely transcription error (vector V-02) |
| CS word: fg = bits 2..0 (+ bit 12 as bit 3 for GRAM cards); card = bits 10..3 for GROM, bits 8..3 for GRAM (bits 10..9 ignored); select = bit 11; advance = bit 13 | no | "one color from the current or next color of a programmer-defined four-color circular stack"; "all 16 colors in the stack so two pastels can share a card" | WK images and JZ agree | tier 4, BSR consistent |
| Bits 15..14 ignored | 5-18 ("two most significant bits available for user storage") | | "not physically connected" | tier 1 |
| Mode select: write `$0021` = FGBG, read `$0021` = CS (register note) | no | | WK, JZ | tier 4 (C6) |
| Stack: 4 entries at `$28`..`$2B`; position 0 at the start of the display; advance before the card is drawn so the card gets the new color; wraps 3 to 0 | no | "circular stack" | WK ("first entry initially", "moves to the next entry prior to rendering"), JZ ("resets to position 0 at the start of the display", "the card which sets this bit gets the new background color") | tier 4; two wordings agree on timing |
| Advance happens for the 20th (cut-off) column card too; no per-row reset | no | | JZ: raster order, one stack; nothing about the 20th column | [INF] (the STIC fetches all 240 words, 5-18) |
| Colored Squares: CS word with bit 12 = 1, bit 11 = 0; four 4 x 4 quadrants, three 3-bit colors and one split color; color 7 = current stack top; never advances | no | one sentence: four equal blocks per card | WK image and text, JZ diagram | tier 4 |
| Display disabled content | no | EXEC runs from BACKTAB while "the STIC is set to black out the video display" | "STIC registers are only available during VBLANK or when the display is blanked" | `unknown-default-chosen` |
| When BACKTAB is read | 5-18: the RA-3-9600 DMA loads a row of 20 words per card row, address counter 0 to 240 over 12 rows | | | row-granular in the chip [OBS]; the core's granularity is a design choice (R-4) |

Disagreement noted: JZ's memory map lists GRAM at `$3800`-`$3AFF`; 64 cards x 8 = 512 bytes = `$3800`-`$39FF`, and the project memory map mirrors 512 bytes over `$3800`-`$3FFF` (C21). The card path never forms an address above `$39FF` (6-bit GRAM card), so it is unaffected [INF].

## 2. Decisions

### A. Architecture of the card path

**R-1 [REC] One pure internal function, caller-owned output.**
`ivs_stic_render_cards(const ivs_memsys *mem, const ivs_stic_cardin *in, uint8_t *idx, uint8_t *on)` in `core/src/stic/stic_cards.c` (internal, not installed, not ABI). `ivs_stic_cardin` = `{mode (0 FGBG, 1 CS), display_on, border (0..15), stack[4] (0..15)}`; it is a value copy of what the register note stores, so the renderer never touches registers. `idx` is `96 x 160` bytes, `on` is `96 x 160` bytes or NULL. No static or global data, no allocation, no state kept between calls: not in the instance, not in the digest, not in a snapshot. The two planes are caller scratch or the caller's video buffer region (framebuffer note); a plane that does not outlive the call cannot break slicing invariance. [INF] Alternative rejected: a renderer method on a STIC struct holding the plane (in-flight frame state would have to join digest and snapshot).

**R-2 [REC] Card-plane value space and the ND marker.**
`idx` pixel = STIC color number 0..15, or ND = `0xFF` for the not-displayed 160th column (every row, x = 159). `on` = 1 where the pixel took the foreground (set) bit of a card bitmap, or a Colored Squares color 0..6; 0 for background pixels, ND pixels and display-off fill. The plane is always 160 x 96, exactly the card grid; there is no row doubling, no margin, no border, no palette. The framebuffer note decides whether x = 159 is cropped (159 visible, D8 leaning per C10), and whether each row is emitted twice (BSR: each pixel is 2 TV lines high, [OBS tier 2]). Vectors judge the plane, so D8 cannot invalidate them; D8 can only change the crop. The constant `IVS_STIC_LAST_COL_VISIBLE_PX = 7` is one named constant (tier 1, C10 default), not a profile field; reversal in section 10.

**R-3 [REC] The word layout is hard-coded and tagged by rule, not profile data.**
Profile data is for unknowns whose value a measurement could change independently (Phase 5 V-04). The layout is a documented structure from one lineage with a bit-for-bit literal oracle (R-8); making 56 bit roles profile fields would add a generator, a gate and tests that restate the same table in a second language. New profile fields only: `stic.power_on_mode` (`fgbg`, `unknown-default-chosen`, C6) and `stic.display_off_fill` (`border`, `unknown-default-chosen`). If a second STIC variant (STIC1A, other board) or an alias profile ever needs a different layout, that is the trigger to move the table into data (section 10).

**R-4 [REC] Granularity: whole plane at the frame boundary, from live state; the inner loop is row-shaped.**
The machine calls the function once per frame at the boundary the framebuffer note defines (the instant the VBLANK interrupt condition begins is my assumption, section 9). Inside, rows are rendered by `render_row(&pos, row_words[20], rows_out)` with the stack position carried in a local variable initialized to 0 per call. Why not render each row at its BUSRQ DMA window: the hardware does fetch row by row (5-18 [OBS]), but progressive rendering needs frame storage owned by the instance, because a frame spans run calls and the caller's buffer may be absent or different in the next call; that storage joins the digest and Phase 8 snapshot and the slicing-invariance proof. Cost of frame granularity: a CPU write to BACKTAB during active display shows in the same frame instead of the next one, and the STIC registers and GRAM/GROM cannot change in active display anyway (gates `stic-regs` and `gmem` are closed there, C1 and C2). Logged as discrepancy CV-5 (`unknown-default-chosen`); one vector pins the default (V-24 "mid-frame BACKTAB write is seen") so a later row-latch is a deliberate golden change.

**R-5 [REC] Side-effect-free reads through two dedicated accessors, never the CPU view.**
Add to `memsys.h`: `uint16_t ivs_memsys_stic_backtab_word_internal(const ivs_memsys *m, uint32_t i)` returning `sysram[i] & 0x3FFF` for `i < 240` (else 0), and `uint8_t ivs_memsys_stic_gfx_byte_internal(const ivs_memsys *m, uint32_t sel, uint32_t card, uint32_t y)` returning `grom[(card*8+y) & 2047] & 0xFF` for sel 0 and `gram[(card*8+y) & 511] & 0xFF` for sel 1. Both read the backing arrays directly: no gate is consulted, no undriven fill, no width formula beyond the explicit mask, no access record, no wait, no stolen cycle, no digest effect. Using `ivs_memsys_read` is the trap: with the `gmem` gate closed (which is the normal state during active display) it returns the undriven fill instead of the card bitmap (named fault F-16). This accessor pair is also the VID-06 seam for card contents. [INF] The sysram backing index equals `addr - $0200` (the region's `lo`); the vector harness pokes by address through `ivs_memsys_poke_raw_internal` so the address-to-index mapping is exercised, not assumed.

**R-6 [REC] How cards get content in tests (no GROM, no assembler for the bulk).**
(a) Unit vectors: the in-process harness (`tests/video/`, links `ivs_core_internal`) uses only the Phase 5 raw-poke seam `ivs_memsys_poke_raw_internal` by address: BACKTAB at `$0200 + 20*row + col`, GRAM at `$3800 + 8*card + y`, and project-authored stand-in graphics at `$3000 + 8*card + y` (poke reaches the GROM backing via the read view). (b) Machine-path variants (3 vectors, gates closed one included): create an instance with the existing zero GROM stand-in loaded (the Phase 4 D-30 stand-in, synthesized, "not a GROM"), then poke. (c) One end-to-end original guest fixture `ivs-diag-video-1` (about 50 words, hand-assembled under D4, no SDBD) that writes a few BACKTAB words and one GRAM card with `MVO`, sets the mode with a `$0021` access, enables the display with a `$0020` write and runs frames; expected plane in the same vector grammar; two scenarios (FGBG, CS), three program mutants; replayed in-process through the public API with a video buffer (the guest path and the register wiring are what this adds). A public host poke API is rejected: it adds a host-visible way to mutate guest-owned state that no host needs. A guest program for every vector is rejected: 240 BACKTAB writes would be about 1,000 hand-assembled words and a 14-bit literal needs two decles.
The stand-in graphics in vectors are project-authored geometric patterns (bars, diagonals, corner marks), never letter shapes or anything recalled from a real GROM, so no GROM byte pattern enters the repository. They are never written as a 2048-byte file and never loaded through the public system-image path; the manifest (`FX-video-card-vectors`, `original-source`, `redistributable: yes` with author attestation) says so.

### B. Word formats and rendering (the rule text vectors are derived from)

**R-7 [REC] Rule text, in the words the hand derivation starts from.**
Let `W = word & 0x3FFF` (bits 15..14 ignored: tier 1). A bit written `b13` is bit 13 of `W`. Card cell `(r, c)` is word index `i = 20*r + c`, `r` 0..11, `c` 0..19, in memory at `$0200 + i`.

FGBG (mode 0), every word:
- `sel = b11` (0 GROM, 1 GRAM); `card = (W >> 3) & 0x3F`; `fg = W & 7`.
- `bg = 8*b12 + 4*b13 + 2*b10 + 1*b9`.
- The stack is not read, `pos` is unchanged, bit 13 never advances.

CS (mode 1), by `(b12, b11)`:
- `(0,0)` GROM card: `card = (W >> 3) & 0xFF`, `fg = W & 7`.
- `(0,1)` GRAM card: `card = (W >> 3) & 0x3F` (bits 10..9 ignored), `fg = W & 7`.
- `(1,1)` GRAM card, as `(0,1)` but `fg = 8 + (W & 7)`.
- `(1,0)` Colored Squares (R-13).
- For the three card cases: if `b13` then `pos = (pos + 1) mod 4` first; `bg = stack[pos]`.

Pixels of a card case, `y` 0..7, `x` 0..7, `byte = gfx(sel, card, y)`: `bit = (byte >> (7 - x)) & 1`; plane `(8r + y, 8c + x)` = `bit ? fg : bg`; `on = bit`. In column `c = 19` pixel `x = 7` is ND for every `y` (R-2). Basis labels per sub-rule are in section 4.

**R-8 [REC] A bit-role literal judged by a single-bit probe sweep.**
Oracle file `data/video/intv-2609-ntsc-family-p1.cardword.literal.txt`: one row per (variant, bit) written from the sources only (the five wiki images and the two diagrams), grammar copied from the memmap and controller literals: `<variant> <bit> <role> src=<SRC-id>@<locator> tag=tier4-community [conf=CV-1]`. Variants: `fgbg`, `cs-grom`, `cs-gram`, `cs-sq`. Roles: `fg0..fg3`, `bg0..bg3`, `card0..card7`, `sel`, `adv`, `ign`, `sq0c0..sq0c2`, `sq1c0..sq1c2`, `sq2c0..sq2c2`, `sq3c0..sq3c2`, `fixed0`, `fixed1` (a bit that must be 0/1 to stay in the variant). The proposed content (14 rows per variant, bit 15 and 14 are `ign` in all):

| bit | fgbg | cs-grom | cs-gram | cs-sq |
|---|---|---|---|---|
| 13 | bg2 | adv | adv | sq3c2 |
| 12 | bg3 | fixed0 | fg3 | fixed1 |
| 11 | sel (1) | sel (0) | sel (1) | fixed0 |
| 10 | bg1 | card7 | ign | sq3c1 |
| 9 | bg0 | card6 | ign | sq3c0 |
| 8..3 | card5..card0 | card5..card0 | card5..card0 | sq2c2..sq1c0 (8..6 = sq2, 5..3 = sq1) |
| 2..0 | fg2..fg0 | fg2..fg0 | fg2..fg0 | sq0c2..sq0c0 |

The probe sweep test: for each variant, for each bit with a role other than `ign`/`fixed*`, build the variant's base word (`fgbg` 0x0000 in mode 0; `cs-grom` 0x0000; `cs-gram` 0x0800; `cs-sq` 0x1000, in mode 1) with that single bit set, put it in cell (5, 9) over self-identifying stand-in graphics (card `n` of GROM has byte 0 = `n`, byte 7 = `A5`; card `n` of GRAM has byte 0 = `n`, byte 7 = `5A`, the other bytes `00`), and read the cell back: the foreground index, the background index and the card identity (decoded from the pixels of byte 0 and byte 7) must each equal what the literal's role says for that bit (`fgk` -> fg index `2^k`, `bgk` -> bg index `2^k`, `cardk` -> card `2^k`, `sel` -> the other store's byte 7, `adv` -> bg = `stack[1]` with a distinct stack, `ign` -> no change from base, `sqXcK` -> the quadrant's color `2^K`). Bits 15 and 14 are probed on every variant (`ign`). 4 x 16 = 64 probes, one test family. This is the memmap width-probe pattern: a behavioural probe against an independent literal, no read of the core's table. It catches every single-bit mislocation exhaustively and cheaply; it does not cover interplay (stack, positions, cut-off), which the vectors do.

**R-9 [REC] Bitmap addressing and bit order.**
`gfx(sel, card, y)`: card `n` occupies 8 bytes, byte `y` is pixel row `y` from the top, bit 7 is the leftmost pixel, bit 0 the rightmost, 1 = foreground (tier 4 JZ, weak tier 2 support). GROM card numbers reach 255 only in CS (8-bit field); FGBG reaches 63 for both stores; GRAM 0..63.

**R-10 [REC] Color stack mechanics.**
Four entries `stack[0..3]` = the values of `$0028`..`$002B` (register note stores 4 bits; the renderer also masks `& 0x0F`: idempotent, so the fault "unmasked stack value" can only come from the register path). Position `pos` starts at 0 at the start of every frame render (a local variable, so there is nothing to reset and nothing to snapshot; the `frames: 2` vector guards against anyone turning it into persistent state). Advance before draw: a CS card case with `b13` set does `pos = (pos + 1) & 3` and then draws with `stack[pos]`. FGBG never touches it. Processing order is the BACKTAB raster order, 240 words, no per-row reset, and the 20th-column cards are processed normally (they advance the stack although only 7 of their pixels display: [INF] from the DMA reading all 240 words, discrepancy CV-8). A Colored Squares card never advances (R-13).

**R-11 [REC] What the stack evidence does and does not support.**
Direct support (two wordings agree): initial position 0, advance-then-draw, wrap 3 to 0, FGBG ignores the stack. Weaker: reset every frame (JZ says so explicitly; WK says "initially"; BSR silent) and the cut-off column advancing. Both weaker rules have a single-feature vector and a named fault; both are `inference`/tier 4 in the rule table and in CV-2/CV-8. A reading where an advance takes effect on the card AFTER the marked one is a one-constant change (`advance_before_draw`) and sits behind fault F-03; I do not make it a profile field because both texts state the timing.

**R-12 [REC] Palette and meaning of the numbers.**
The plane holds STIC color numbers (0 to 15: primaries 0..7, pastels 8..15 per the community table). RGB, names and any host mapping are host policy (VID-03). The core never reads a color table. FGBG foreground is therefore always a primary (3 bits) and a pastel foreground on a GROM card in CS mode is not representable: that decode slot is Colored Squares.

**R-13 [REC] Colored Squares is implemented; the cut line is documented.**
CS word with `b12 = 1, b11 = 0`: four quadrants of 4 x 4 pixels, `q0` top-left, `q1` top-right, `q2` bottom-left, `q3` bottom-right, colors `c0 = W & 7`, `c1 = (W >> 3) & 7`, `c2 = (W >> 6) & 7`, `c3 = 4*b13 + 2*b10 + b9`; color 7 draws `stack[pos]` (no advance, `on = 0`), colors 0..6 draw that primary color with `on = 1`. Bit 13 is a color bit here, not an advance bit. It is outside the literal text of VID-04 but the decode slot exists on the same two bits that select the GROM/GRAM CS cases; omitting it leaves a defined hardware word that renders as a wrong card with no diagnostic. Cost: about 15 lines, 2 vectors, 2 faults, 8 bit-probe rows. Cut line if the plan is over budget: such a card renders as a card whose 64 pixels all equal `stack[pos]`, `on = 0`, no advance, with discrepancy CV-3 stating "colored squares not implemented", and the 2 vectors and 8 probe rows move to Phase 8. Decide at plan time; default is to implement.

**R-14 [REC] Display off.**
With `display_on = 0` the function reads no BACKTAB word and no graphics byte, leaves `pos` alone, fills every `idx` with `border` (ND stays at x = 159) and clears `on`. `unknown-default-chosen` (CV-4): BSR says the display is "blacked out" while EXEC code runs from BACKTAB, which supports index 0 at reset (the register default is 0 under `power_on_fill.stic`); filling with the border register is my pick for a non-zero border and has no source at all. Resolver: an owner-run capture with real EXEC (D11) or a measurement (D10). The vector (V-22) uses a non-zero border so the default is pinned and labelled `basis: default-unknown`, not as a claim. Whether a frame is produced at all while the display is off, and the flag it carries, is the framebuffer note's call.

**R-15 [REC] What the border is in Phase 6.**
Nothing but the display-off fill. The area outside the card plane (the object field margin, the 8-pixel strips, `$2C` over the overscan, `$32` extension) is Phase 8 (VID-05 names border explicitly). If D8's geometry has a margin, the framebuffer note fills it with the same `border` value; I do not define its extent.

### C. Independence and process

**R-16 [REC] Generator-versus-literal rule (written down so the plan can cite it).**
1. Every expected plane is a committed literal in the vector file, written by hand from the rule text of R-7..R-14 (and the cell derivation comments) BEFORE `stic_cards.c` exists. A throwaway typing aid (expanding `#` and `.` to digits) is allowed, written from the rule text, never from the core, never committed as a gate dependency.
2. The core, any header under `core/src`, and its output are never read while writing a vector. No expected value is produced by a run. If a vector fails, the fix is decided from the rule text and the source evidence, and any change to an expected cell after the renderer exists is a reviewed change recorded in that plan's SUMMARY with the reason.
3. No committed second model of the renderer (a script or cmake re-implementation of the decode): it would share the author's misreading, cost a maintained tool, and is the rejected "second PSG model" of Phase 5 (verify note anti-pattern 4). The only independent computations are (a) literal hand blocks, (b) the table-driven bit-role probe (R-8), (c) metamorphic properties (R-17).
4. Hand-typed BACKTAB words are verified against their field annotation by a test-only encoder (`tests/video/vid_fields.c`, about 30 lines, encode direction, no `core/src` include). A mismatch is reported as `FIXTURE` error, not a renderer failure. This closes the likeliest hand-derivation error (a typo in a 14-bit hex word) without letting a failing run "fix" the expectation.
5. One blind second reader per vector file, from the input half of the vector (everything except the expected lines) and the rule text only, same protocol as `docs/cp1610/second-reader-protocol.md` and the Phase 5 copies; records are `source-expectation, second-read` and never `independent-implementation-comparison` (C43 wording; the reader is the same model family: residual shared-misreading risk is recorded, not closed). Disagreements are resolved from the source evidence and noted in the vector.
6. Process control as in Phase 5: the oracle plan commit precedes the renderer commit; each SUMMARY names the oracle commit; the verifier checks `git log` order.
7. Sensitivity control: the fault matrix (section 6) requires every vector to fail under at least one named fault and every fault to fail at least one vector, so no vector is vacuous.

**R-17 [REC] Four metamorphic properties (no table, no hand pixels).**
P1: setting bits 15..14 of every BACKTAB word does not change either plane (random seeded words, both modes). P2: the same words rendered twice give the same planes (purity; also with `on == NULL`). P3: for a CS frame with no advance bits and GRAM/GROM cards with fg < 8, the `idx` plane equals the FGBG rendering of the equivalent words with `bg` = `stack[0]` (mode equivalence). P4: `on` equals the bitmap bit for every card case and is 0 for ND pixels; `idx` outside ND is in 0..15. Property tests are guards, not evidence (Phase 5 taxonomy); they run over 64 seeded random frames.

**R-18 [REC] Evidence tiers and claim wording.**
Card word layout, stack rules, bit order, Colored Squares: claim tier `source-expectation`, standing `provisional`, source-authority tag `tier4-community` (weakest field wins). Structure (matrix, 7-pixel column, 14 bits, RAM extent): `tier1-primary-doc` rows. The guest path claim (`ivs-diag-video-1`): `original-diagnostic`, provisional. Goldens for the e2e digest: `deterministic-repeatability`. No `measured-hardware` claim. The VID-04 claim text is: "renders hand-derived expected frames for the documented FGBG and Color Stack card layouts of the first profile (community-sourced word layout)"; it never says a frame matches hardware. D10 (owner measurement) and D11 (owner real EXEC/GROM run) are not performed; evidence that needs them is recorded as missing in `d10-measurements.ids`.

## 3. Fixture file grammar (`data/video/vectors/<id>.txt`)

ASCII, LF, one directive per line, `#` starts a comment, blank lines ignored, tokens separated by single spaces. Header keys are directives, not comments. Hex digits are uppercase. The file stem equals `id`. The exact file set equals `data/expected/video-vector-files.ids`.

```
vector <id>                         # first directive
rules <CR-..> [<CR-..> ...]         # rule ids witnessed; each must exist in data/video/rules.json
basis <label> [<label> ...]         # gi-primary-literal gi-primary-derived bsr-tier2 community-two-texts inference default-unknown
mode fgbg|cs                        # card mode in force
display on|off                      # default on
border <H>                          # 0..F, default 0
stack <H> <H> <H> <H>               # $0028..$002B, default 0 0 0 0
gates open|closed                   # default open; closed = gmem and stic-regs gates closed (renderer must not care)
frames <n>                          # 1..3, default 1; machine path only: every frame must equal the expectation
path unit|machine                   # default unit; machine = instance, poke seam, frame via the run call
fill <HHHH>                         # BACKTAB word of every unlisted cell (required)
cell <r> <c> <HHHH> | <fields>      # one cell, 0..11 x 0..19, at most once; fields annotation required
gram <n> <b8> <b8> <b8> <b8> <b8> <b8> <b8> <b8>    # card n 0..63; eight 8-char strings of 0 and 1, top row first
grom <n> <b8> ... (eight)           # stand-in graphics card n 0..255 (project-authored; never a real GROM)
default <H>                         # index of every pixel of every cell not listed under an expect directive
expect-solid <r> <c> <H>            # whole cell index H (column 19: 7 pixels H and the 8th ND)
expect <r> <c>                      # followed by exactly 8 lines of exactly 8 chars from [0-9A-F-]; '-' only at x=7 of column 19
<8 chars> x 8
expect-set <r> <c>                  # optional: 8 lines of 8 chars from [#.]; unlisted cells: all '.'; ND pixels '.'
<8 chars> x 8
```

Rules the `video-vectors` step enforces: the grammar above; ranges; `cell` unique per `(r, c)`; `gram`/`grom` unique per card; an `expect*` per `(r, c)` at most once; every column-19 `expect` block ends each line in `-`; `rules` ids exist; `basis` labels are in the vocabulary; a vector whose only basis is `default-unknown` is allowed only for the display-off and mid-frame pins; every rule id in `rules.json` has at least one vector or a `default-unknown` tag with a resolver and a revisit trigger; the `fields` annotation parses. Fields annotation (checked by the test-only encoder, not by the gate): `fgbg fg=<0..7> bg=<0..15> grom|gram card=<0..63>`, `cs adv=<0|1> fg=<0..15> grom|gram card=<0..255>`, `sq c0=<0..7> c1=<0..7> c2=<0..7> c3=<0..7>`, plus optional `u=<0..3>` for bits 15..14. A plane is judged whole: the harness builds the expected 160 x 96 plane from `default`, the `expect*` blocks and the ND rule for x = 159 (the harness applies ND to column 159 of every row by definition; it does not decode words) and compares byte for byte; on a mismatch it prints the first differing cell as two 8-line blocks side by side with the BACKTAB word, its annotation and the vector line number (DX lens: the failure names the cell and the field, not a hash).

Worked example (`cs-advance-before-draw`, derivation shown as comments in the committed file):

```
vector cs-advance-before-draw
rules CR-10 CR-11
basis community-two-texts
mode cs
stack 2 5 9 F
fill 0000
# stack = [2,5,9,F]; pos starts 0. fill 0000 = cs adv=0 fg=0 grom card=0 (zero bytes: all background).
# unlisted cells before the advance cell draw stack[0] = 2; the cell with b13 set advances to pos 1 first.
# 2000 = bit 13 only. Cell (0,3) is drawn with stack[1] = 5 and every later card keeps pos 1 -> 5.
cell 0 3 2000 | cs adv=1 fg=0 grom card=0
default 5
expect-solid 0 0 2
expect-solid 0 1 2
expect-solid 0 2 2
```

(Cells (0,3) onward are covered by `default 5`; the cut-off pixel of each column-19 cell is ND by rule.)

The bit-role literal (R-8) and the rule inventory (`data/video/rules.json`: ids `CR-01..`, each with statement, `basis`, source locators, vector ids, profile fields read, discrepancy id) are separate files; the existing `profile-fields`/`discrepancies` steps cross-check them like Phase 5's `psg-rules`.

## 4. Rule table with basis labels (proposed `data/video/rules.json` seed)

Basis vocabulary: `gi-primary-literal` (printed fact), `gi-primary-derived` (arithmetic or counting from printed facts), `bsr-tier2`, `community-two-texts` (WK and JZ agree; one lineage), `inference`, `default-unknown`. Tag = the source-authority tag the profile/claim row carries.

| Rule | Statement | Basis | Tag | Vectors (plan) | Disc. |
|---|---|---|---|---|---|
| CR-01 | BACKTAB is 20 x 12 cards, 240 words of 14 bits at `$0200`..`$02EF`; word `20r + c` | extent: gi-primary-literal (5-10, 5-18); raster formula: community-two-texts + gi-primary-derived (address counter +20 per row) | tier1 (extent), tier4 (raster) | V-07 | CV-1 |
| CR-02 | Only bits 13..0 of a BACKTAB word are used; bits 15..14 ignored | gi-primary-literal (5-18) | tier1 | V-09, V-21, probes | |
| CR-03 | Card = 8 x 8; the 20th column shows 7 pixels, visible 159 x 96 | count: gi-primary-literal (5-10); side (rightmost hidden): bsr-tier2 + community-two-texts | tier1 / tier2 | V-08 | C10, CV-7 |
| CR-04 | Card bytes: byte 0 top, bit 7 leftmost, 1 = foreground | community-two-texts (JZ explicit, BSR consistent) | tier4 | V-06 | CV-1 |
| CR-05 | GROM base `$3000`, GRAM base `$3800`, 8 bytes per card, address `base + 8*card + y` | bases: gi-primary-literal (5-16); stride: bsr-tier2 + inference | tier1 / tier4 | V-03, V-04, V-14, V-15 | C21 |
| CR-06 | Select = bit 11 (1 = GRAM) in both modes | community-two-texts; consistent with the graphics address map (address bit 11 separates `$3000` and `$3800` pages) | tier4 | V-04, V-15, probes | CV-1 |
| CR-07 | GRAM card = bits 8..3; bits 10..9 ignored | community-two-texts; 64 cards in 512 bytes (bsr-tier2) | tier4 | V-14 | CV-9 |
| CR-08a | FGBG fg = bits 2..0 (primaries only) | community-two-texts; bsr-tier2 (primary-only limit) | tier4 | V-01 | |
| CR-08b | FGBG card = bits 8..3 (0..63) for both stores | community-two-texts | tier4 | V-03 | |
| CR-08c | FGBG bg = 8*b12 + 4*b13 + 2*b10 + b9 | community-two-texts | tier4 | V-02, V-01 | CV-1 |
| CR-08d | FGBG ignores the stack, bit 13 does not advance | community-two-texts | tier4 | V-10 | |
| CR-09a | CS decode by (b12, b11): GROM / GRAM / GRAM+fg3 / Colored Squares | community-two-texts | tier4 | V-25 | CV-1 |
| CR-09b | CS GROM card = bits 10..3 (0..255), fg = bits 2..0 | community-two-texts | tier4 | V-12, V-14 | |
| CR-09c | CS GRAM fg = bit 12 as bit 3 + bits 2..0 (0..15) | community-two-texts | tier4 | V-13 | |
| CR-10 | CS bg = `stack[pos]` after the card's advance | community-two-texts; bsr-tier2 ("current or next") | tier4 | V-16, V-17 | CV-2 |
| CR-11a | `pos` = 0 at the start of every frame render | JZ explicit; WK "initially"; inference | inference | V-20 | CV-2 |
| CR-11b | `b13` set: `pos = (pos+1) mod 4` before drawing; the card gets the new color | community-two-texts | tier4 | V-17 | CV-2 |
| CR-11c | wrap 3 to 0 | community-two-texts | tier4 | V-18 | CV-2 |
| CR-11d | one stack across rows (no per-row reset) | community-two-texts (raster order) | tier4 | V-19a | CV-2 |
| CR-11e | the cut-off column card advances the stack | inference (gi-primary-derived: 240 words are fetched) | inference | V-19b | CV-8 |
| CR-12 | Plane values are STIC color numbers 0..15, ND = 0xFF at x = 159; palette is host policy | community palette table (identity use only) | tier4 | all | C10 |
| CR-13a | Colored Squares quadrants, `c3 = 4*b13 + 2*b10 + b9`, bit 13 not an advance | community-two-texts | tier4 | V-23a | CV-3 |
| CR-13b | Color 7 = `stack[pos]`, `on = 0`; colors 0..6 `on = 1`; never advances | community-two-texts | tier4 | V-23b | CV-3 |
| CR-14 | `on` plane = bitmap bit (and square colors 0..6) | community-two-texts (collision wording) | tier4 | V-06, V-23, P4 | |
| CR-15 | Display off: no read, fill = border index | default-unknown (bsr-tier2 only "blacked out") | unknown-default-chosen | V-22 | CV-4 |
| CR-16 | Power-on mode = FGBG; `$0021` write = FGBG, read = CS | mode strobes: community-two-texts (register note); power-on value: default-unknown | tier4 / unknown-default-chosen | V-26 (pair), e2e | C6, CV-6 |
| CR-17 | The renderer reads BACKTAB and graphics through the backing accessors, at the frame boundary, from live state, with no side effect and no gate | design rule (R-4, R-5); fetch time default-unknown | delegated-decision / unknown-default-chosen | V-24, V-27, V-28 | CV-5 |

## 5. Vector plan (oracle-first; single-feature; all `path unit` unless noted)

~24 vector files, each 20 to 70 lines. Stand-in graphics are project patterns. `solid` = GRAM card with all rows `11111111`; `blank` = a card with no `gram`/`grom` line (zero bytes).

| Id | Mode | What it pins | Content sketch |
|---|---|---|---|
| V-01 fgbg-fg-color | fgbg | CR-08a, 08c (fg bits do not collide with bg bits) | 8 cells, `solid` GRAM card, bg = 15 (so b13, b12, b10, b9 all set), fg 0..7: expected fg index per cell; a treatment of b12 as fg bit 3 fails |
| V-02 fgbg-bg-color | fgbg | CR-08c bit order | 16 cells, blank card, bg 0..15 (single bits 1, 2, 4, 8 and combinations), fg 0 |
| V-03 fgbg-card-number | fgbg | CR-08b, CR-05 stride | GRAM cards 0,1,2,4,8,16,32,63 and GROM stand-in cards 1,2,4,8,16,32,63, all different patterns |
| V-04 fgbg-select-bit | fgbg | CR-06 | card 5 in GROM stand-in and in GRAM with different patterns, adjacent cells |
| V-06 card-bit-order | fgbg | CR-04, CR-14 | an asymmetric card (a single pixel at x=0 in row 0, x=7 in row 7, a vertical bar at x=1 rows 1..3): pins MSB left, row 0 top, 1 = fg; with `expect-set` |
| V-07 card-grid-position | fgbg | CR-01 raster | cells (0,0), (0,18), (11,0), (11,18), (6,7) with unique bg colors (a transposed `col*12+row` fails) |
| V-08 last-column-width | fgbg | CR-03 | col 19: `solid` card (7 fg + ND), a card with only bit 0 set (7 bg + ND), a card with only bit 7 set (left pixel fg), plus col 18 solid |
| V-09 fgbg-upper-bits | fgbg | CR-02 | words with `u=3` equal their `u=0` twins (expected identical blocks) |
| V-10 fgbg-stack-ignored | fgbg | CR-08d | stack `5 6 7 9`, words with b13 set (bg bit 2); expected from bg bits only |
| V-12 cs-fg-grom | cs | CR-09b | stack[0] = 9, stand-in GROM `solid`, fg 0..7 |
| V-13 cs-fg-gram-bit3 | cs | CR-09c | GRAM `solid`, fg field 0..7 with and without b12: fg 0..7 and 8..15 |
| V-14 cs-card-number | cs | CR-09b, CR-07 | GROM cards 64, 128, 255 (bits 10, 9 as card bits) and 1; GRAM card 5 with b10, b9 set renders card 5 |
| V-15 cs-select-bit | cs | CR-06 | same as V-04 in CS |
| V-16 cs-stack-initial | cs | CR-10, CR-11a | stack 2 5 9 F, no advance bits: every cell bg 2 |
| V-17 cs-advance-before-draw | cs | CR-11b | the worked example above |
| V-18 cs-advance-wrap | cs | CR-11c, pastel stack | stack 8 B 9 F, 5 advances in row 0: positions 1,2,3,0,1 |
| V-19a cs-advance-rows | cs | CR-11d | one advance at (0,10); row 1 col 0 still uses pos 1 |
| V-19b cs-advance-cutoff-col | cs | CR-11e | advance at (3,19); (4,0) uses the new color |
| V-20 cs-frame-reset | cs, `path machine`, `frames 2` | CR-11a | advances present; both frames identical |
| V-21 cs-upper-bits | cs | CR-02 | like V-09 |
| V-22 display-off | `display off`, border C | CR-15 | non-trivial BACKTAB ignored: plane all C, ND column; labelled default-pin |
| V-23a cs-colored-squares | cs | CR-13a | cells exercising each square color 0..6 and the split c3 bits |
| V-23b cs-squares-stack7 | cs | CR-13b | color 7 after a prior advance uses `stack[pos]`; a b13-set square card does not advance (next card keeps pos) |
| V-24 mid-frame-pin | cs, `path machine` | CR-17 default | a guest or seam write to BACKTAB between boundary and boundary appears in the frame (default-pin, `basis default-unknown`) |
| V-25 cs-mode-decode | cs | CR-09a | four cells with the same low bits and (b12, b11) = 00, 01, 11, 10 |
| V-26 mode-sense-fgbg / V-26 mode-sense-cs | fgbg / cs | CR-16, SC4 "wrong card mode" | identical BACKTAB words, two vectors, different expected planes |
| V-27 render-gates-closed | cs, `gates closed`, `path machine` | CR-17 | any V-17-like frame; renderer must not see the closed gate |
| V-28 render-no-side-effect | `path machine` | CR-17 | machine state digest equal before and after a frame render with and without a video buffer |

Counts: 28 files if every row is kept (V-26 is two files, V-23, V-19 two each). The planner may merge V-09/V-21 and V-04/V-15 as one file per mode only if each keeps one named fault that it alone catches; do not merge V-17..V-19 (single-feature). Plus: the 64-probe sweep (R-8), four properties (R-17), the e2e guest fixture (R-6), and the gate step with its controls. Expected authoring volume: roughly 90 `expect` blocks of 8 lines (about 700 lines of pixel text) plus about 140 `cell` lines; `expect-solid` removes most of the uniform cells.

## 6. Named faults for the card path (target: caught by at least one vector, and every vector fails under at least one fault)

| Fault id | Mutation | Caught by |
|---|---|---|
| F-01 `CARD_MODE_SENSE_INVERTED` | FGBG decoded as CS and conversely (the roadmap's "wrong card mode") | V-26 both, nearly all |
| F-02 `CS_NO_ADVANCE` | b13 ignored in CS | V-17, V-18, V-19a/b, V-20 |
| F-03 `CS_ADVANCE_AFTER_DRAW` | advance applies to the next card | V-17, V-18 |
| F-04 `CS_NO_FRAME_RESET` | `pos` persists across frames | V-20 |
| F-05 `CS_NO_WRAP` | `pos` saturates at 3 (or indexes `stack[4]`) | V-18 |
| F-06 `CARD_BIT_ORDER_REVERSED` | bit 0 leftmost | V-06 |
| F-07 `CARD_ROWS_FLIPPED` | byte 0 is the bottom row | V-06 |
| F-08 `GFX_SELECT_SWAPPED` | bit 11 sense inverted | V-04, V-15, V-25 |
| F-09 `LAST_COL_FULL_WIDTH` | 8 pixels shown in column 19 | V-08 |
| F-10 `LAST_COL_LEFT_CUT` | the leftmost pixel dropped instead of the rightmost | V-08 |
| F-11 `FGBG_BG_BITS_SWAPPED` | b12 and b13 exchanged in bg | V-02, V-01 |
| F-12 `FGBG_FG_BIT3_FROM_B12` | b12 read as fg bit 3 in FGBG | V-01 |
| F-13 `GRAM_CARD_FIELD_8BIT` | GRAM card uses bits 10..3 | V-14 |
| F-14 `CS_GROM_CARD_6BIT` | GROM card truncated to 6 bits in CS | V-14, V-12 high cards |
| F-15 `UPPER_BITS_NOT_MASKED` | word not masked to 14 bits | V-09, V-21, probes |
| F-16 `GFX_READ_VIA_CPU_VIEW` | graphics read through `ivs_memsys_read` | V-27 (gate closed), V-28 |
| F-17 `SQUARES_ADVANCES_STACK` | Colored Squares card treats b13 as advance | V-23b |
| F-18 `DISPLAY_OFF_DRAWS_CARDS` | cards drawn while off | V-22 |
| F-19 `VIDEO_STATE_DEPENDS_ON_BUFFER` | machine state or digest differs with a NULL video buffer (analog of Phase 5's audio fault; shared with the framebuffer plan) | V-28 |

Per-plan caps from Phase 5 stay binding (5 tasks, 8 faults, 25 controls): split across three plans (F-01..F-06 with the first renderer plan; F-07..F-14; F-15..F-19 with the machine-path plan).

Controls (about 8) for the new pure gate step `video-vectors` and the literal: a vector with a 7-line `expect` block; a column-19 block whose 8th character is not `-`; a duplicate `cell`; an unknown rule id; a missing `fill`; a `gram` string of 7 characters; a bit-role literal row naming a tier-5 source (refused as in the memmap gate); a rule with no vector and no `default-unknown` tag.

## 7. Cost, CI and DX

- New suite `video`: about 24 vector tests + 64 probes (counted as 4 ids) + 4 properties + 3 machine-path + e2e replay: about 40 test ids at about 0.003 cpu-s each. 19 faults at 0.15 to 0.25 cpu-s each is the dominant term (about 3 to 5 cpu-s). The pure gate step `video-vectors` costs about 0.3 cpu-s (about 28 small text files, no `string(JSON)` loops; rules.json is the only JSON). Estimate +5..9 cpu-s and +1..2 s hosted wall. The Phase 5 checkpoint rule (measure, then pull the pre-committed lever) applies; no new CI job, no new dependency, no new tool.
- Parse once, reuse: the harness parser is one C file shared by the unit, machine and e2e paths; the gate step is a separate cmake parser for the same grammar (deliberately two parsers: the gate step must not trust the C parser).
- Expected files are text, a few KB each, diffable. PNG or binary goldens are rejected (not reviewable as a diff, no value over text, palette is host policy).

## 8. Not in Phase 6, and the state model that stays forward-compatible

Explicitly NOT in this note or in Phase 6's card path (Phase 8, VID-05, unless named otherwise): MOB drawing, MOB priority and the PRIO bit, MOB-to-card and MOB-to-MOB collision and the `$0018`..`$001F` registers, MOB card limits in FGBG, 8 x 16 MOBs, horizontal and vertical delay (`$0030`/`$0031`) and the shifted object field, border extension (`$0032`), the border area outside the card plane and its extent, `$002C` beyond the display-off fill, row-granular BACKTAB fetch and mid-frame register effects, framebuffer scaling, row doubling, palette (D8 and host policy), PAL, the STIC1A, Intellivision II, ECS.
Forward-compatibility points kept on purpose:
1. The `on` plane exists in the contract now, so MOB priority and background-collision (which need "was this card pixel set") need no renderer rewrite.
2. `ivs_stic_cardin` is a value struct; delay, extension and border-strip fields join later with zero defaults that leave Phase 6 output unchanged.
3. The inner loop is `render_row`, so a row-latch (a future timing note) can call it per row; the only thing it would add is instance-owned in-flight frame state, a Phase 8 hidden-state checklist item that Phase 6 deliberately does not create.
4. No render state exists in the instance: Phase 8's hidden-state checklist entry for the card path is "stack position: derived per frame, none stored; planes: caller scratch, none stored".
5. Vectors judge the 160 x 96 plane, not the published frame, so D8 (159 vs 160, row doubling, margin) can change without touching expected text.

## 9. Dependencies on the sibling notes (assumptions to reconcile at synthesis)

- Register note: stores `mode` (reset default FGBG, CV-6), `display_on`, `border`, `stack[4]` (4 bits each); `$0021` write/read set the mode, with no other effect on rendering; an inspection path (VID-06) that never performs the `$0021` read; closed `stic-regs` window during active display. I need a value-copy getter for `ivs_stic_cardin`.
- Framebuffer note: owns the frame boundary instant (my assumption: the first tick at which VBLANK begins, with the CPU not yet in the interrupt service routine), the buffer layout, ND handling and crop, row doubling, the `ivs_info` frame constants, and the rule that `video == NULL, capacity 0` runs the same code against a counting sink (the Phase 5 D-17 rule for video). The plane contract of R-2 is the interface.
- Timing note: any BUSRQ/BUSAK modelling is independent of the card path; I only require that gates `gmem`/`stic-regs` are closed in active display (so the frame-granular read is equivalent except for BACKTAB, CV-5).
- Orchestrator: allocate discrepancy ids once (Phase 5 lesson): this note proposes placeholders CV-1..CV-9 (section 10) to become C81 onward after the sibling notes' needs are merged (C80 is the last used id); `data/sources.json` gets new locators `SRC-intv-wiki at STIC.oldid-15008`, `SRC-intv-wiki at BACKTAB.oldid-14739`, `SRC-intv-wiki at Stic_fgbg_word.png` (and the other four images), `SRC-community-emu-docs at doc/programming/stic.txt`, `... at graphics_mem.txt`, and `SRC-bsr-2609` locators for the graphics/STIC sections, all `rights: unknown`, kept in the gitignored cache (the plan re-fetches them into `refs/video/`).

## 10. Open items, discrepancies to allocate, reversal triggers

Discrepancy placeholders (default policy / resolver / trigger):
- CV-1 Card word layout rests on one community lineage and no primary page. Default: the layout of R-7. Resolver: an AY-3-8900-1 detail datasheet or STIC documentation from GI, a die/board study, or an owner-run capture with a real EXEC (D11). Trigger: any such source.
- CV-2 Stack advance timing, reset per frame. Default: advance-then-draw, reset every frame. Resolver: same as CV-1. Trigger: a source or a capture that shows the advance on the following card or a persisting position.
- CV-3 Colored Squares decode (scope and layout). Default: implemented per R-13 (or the cut line). Trigger: a plan decision to cut, or a source difference.
- CV-4 Display-off content. Default: border index fill. Resolver: D10/D11 capture. Trigger: capture, or a guest that depends on it.
- CV-5 BACKTAB fetch time (frame boundary vs row DMA). Default: frame boundary, live state. Resolver: timing evidence for the RA-3-9600 DMA windows plus a guest probe; trigger: the timing note models row DMA events or a guest depends on mid-frame BACKTAB writes.
- CV-6 Power-on mode. Default: FGBG. Resolver: D10 power-on read-back or a real-EXEC trace. Trigger: same.
- CV-7 Which pixel column of the 20th card is hidden (rightmost assumed; count of 7 is tier 1). Partial update to C10, not a reopening.
- CV-8 The cut-off column card advances the stack. Default: yes. Trigger: any source or capture.
- CV-9 GRAM card field bits 10..9 ignored versus mirrored over a 512-byte memory (equivalent for 64 GRAM cards; differs only if the GRAM extent changes, C21).
Also cross-reference C6 (mode strobe), C10, C21, C43 (second-read residual risk).

Reversal triggers (consolidated):
- A GI STIC detail source, a schematic, or owner D10/D11 evidence that changes any CV: edit `stic_cards.c` constants and the literal, re-run the probe sweep and vectors; if only the bit-role table changes, the sweep fails first and names the bit.
- D8 chooses 160 visible columns: change `IVS_STIC_LAST_COL_VISIBLE_PX` to 8 and re-write only the `-` characters of column-19 blocks (V-08 and every vector with a listed column-19 cell).
- A second STIC variant or an alias profile with a different layout: move the layout table into profile data with a table generator (the Phase 5 `IvsCtrlTable` pattern) and keep the literal as the judge.
- The timing note models row DMA: switch the machine to per-row latching (instance-owned line state, digest and snapshot rows, a slicing-invariance proof) and re-bless the V-24 pin.
- The plan runs over budget: apply the Colored Squares cut line (R-13) before cutting any single-feature vector.

## 11. Adversarial pass

1. "Every claim here is one community lineage; hand-deriving frames from it proves only self-consistency." Correct, and stated in the claim wording (R-18), the tags (tier 4 on every layout row) and CV-1. The work still has value: it makes the core's behaviour exactly equal to a written specification, catches transcription and decode errors mechanically, and gives D11/D10 a precise list of what to check. The structure that is tier 1 (20 x 12, 7-pixel column, 14 bits, RAM extent) is separated in the rule table so a reader sees which part is which. A cheap partial cross-check that costs nothing: BSR's statements (primary-only foreground in FGBG, per-card bg, "current or next" stack color, two pastels on one card in CS) are consistent with the layout; I list them as consistency checks, not as sources of values.
2. "The same author writes the rules, the vectors and the core: shared misreading." Mitigations that exist: literal blocks written before the code, the word/field annotation encoder, blind second read, the table-driven probe sweep, metamorphic properties, the vector-fault sensitivity matrix. Residual risk (same model family) is recorded as in C43, not claimed closed.
3. "Why a bit-role literal and a vector set; one is redundant." The sweep is exhaustive over single bits but blind to interaction; the vectors cover interaction (stack across rows, the cut-off column, mode switch, gates) but not every bit. Together they are smaller than either alone: the vectors need not enumerate bits, the sweep need not model the stack.
4. "Hard-coded layout contradicts the Phase 5 'unknowns are profile data' rule." The rule is for values a measurement could change independently of each other; the layout is one documented structure, its failure mode is "all of it" not "one field", and the judge is a literal plus a behavioural probe. The two genuinely independent unknowns are profile fields (R-3).
5. "Frame-granular rendering is wrong for hardware that fetches per row." True for BACKTAB writes during active display, which are rare and unspecified (CV-5). The alternative needs in-flight frame state in the digest and snapshot and a slicing-invariance proof for frames that span run calls, for a behaviour no Phase 6 criterion tests. The row-shaped loop keeps the door open.
6. "Display-off = border fill is invented." It is labelled `unknown-default-chosen` with a resolver and pinned by one default-pin vector, not claimed; at reset (border 0) it coincides with the only sourced statement ("blacked out").
7. "Colored Squares is scope creep." It closes a decode hole on the same two bits as the GROM/GRAM split; cost is small and a documented cut line exists (R-13).
8. "ND at x = 159 may make hashes depend on a marker D8 later drops." Hashes of the published frame are the framebuffer note's; vectors judge the plane. If D8 crops the column the marker never reaches the published frame.
9. "Stand-in graphics could be mistaken for GROM." They are project geometry, live only in vector text and in-memory pokes, are not loaded through the system-image path, and their manifest says `original-source`. No GROM digest or identity is ever claimed. The e2e fixture uses the existing zero stand-in and GRAM cards.
10. "A script generator would remove 700 lines of hand pixels." It would also remove the independence the vectors exist for, and add a maintained tool (rejected in R-16.3). The volume is cut by `expect-solid` and `default`; the encoder checks the typo-prone words; the second read checks the rest.
11. "Cost is open-ended." About 40 test ids, 19 faults, one pure gate step; the per-plan caps and the Phase 5 measure-then-lever rule apply.
12. "PNG goldens or comparing against an emulator would be faster." Rejected: binary diffs are unreviewable, and agreement with another emulator is not independent evidence (AGENTS.md; sources.json authority rules). I did not open any emulator source.

## 12. The three riskiest unknowns

1. The whole BACKTAB word layout and the stack timing rest on two community texts of one lineage with no primary page (CV-1, CV-2); the 14-bit non-monotonic bg field order and advance-then-draw are the likeliest places for a silent, consistent-looking error that no automated check can catch before D10/D11.
2. When the chip reads BACKTAB and what a blanked display shows (CV-4, CV-5): both are defaults with no source, and a guest that writes BACKTAB mid-frame or toggles the display will expose them first.
3. Which pixel column of the 20th card is hidden, and whether the framebuffer keeps 159 or 160 columns (CV-7, C10/D8): only the count of 7 is tier 1; a wrong side would shift every column-19 expectation, and the decision belongs to the framebuffer note.
