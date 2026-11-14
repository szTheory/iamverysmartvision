# STIC reference notes

Project-owned notes for the video chip of the 2609 family: what the manufacturer pages for the AY-3-8900-1 and for the system RAM print, what they leave out, the allocation of every id the Phase 6 plans use, and how rules, literals and frames are tied to the sources.
Facts and short paraphrases only; no page text, no page image and no wiki image is committed (D-24): pages and images live in the gitignored local cache.
Labels: [OBS] read on a page image in plan 06-01, [INF] derived or inferred, [UNK] not printed on the pages read.

## Page-reading verdict

These ten lines answer, for the pages named, whether the page prints the thing named (D-01).
`yes` means printed in a form usable as a tier 1 source, `partial` means a part of it is printed, `no` means it is not printed.
Page lists are pdf pages of the 1980 catalog (`SRC-gi-1980-catalog`: 191, 192, 193, 199) and of the 1982 catalog (`SRC-gi-1982-catalog`: 462, 463, 464, 470); the printed numbers are 5-10..5-12 and 5-18 in 1980 and 9-43..9-45 and 9-51 in 1982.

registers-printed: no (pdf 191, 192, 193; 1982 pdf 462, 463, 464)
backtab-format-printed: partial (pdf 199; 1982 pdf 470)
color-stack-printed: no (pdf 191, 192, 193; 1982 pdf 462, 463, 464)
display-enable-printed: no (pdf 191, 192, 193; 1982 pdf 462, 463, 464)
colors-numbered-printed: no (pdf 191; 1982 pdf 462)
window-lengths-printed: no (pdf 193; 1982 pdf 464)
bus-hold-printed: no (pdf 193, 199; 1982 pdf 464, 470)
rows-per-frame-printed: yes (pdf 191; 1982 pdf 462)
visible-geometry-printed: yes (pdf 191; 1982 pdf 462)
timing-figure-readable: partial (pdf 193; 1982 pdf 464)

Per answer, read at 150 dpi renders of every page named (1982 pdf 462 was compared to the 1980 page by its text layer and by the two page images that differ only in furniture) [OBS]:

- `registers-printed: no`. The three STIC pages are a pin list, a bus and control description, the electrical table and the timing figures. No CPU-visible register address, no bit field of any register and no strobe address is printed.
- `backtab-format-printed: partial`. The system RAM page (pdf 199, printed 5-18; the 1982 copy pdf 470, printed 9-51) says the RAM holds 352 words of 16 bits at addresses 512 to 863 with the graphics descriptors in the first 240 words, that graphics use only the lower 14 bits of each word and that the two most significant bits are left to the program [OBS]. The address (512 decimal) and the 240-word extent are therefore tier 1. No field inside a word (foreground, background, card number, select, advance) is printed.
- `color-stack-printed: no`. The word stack does not occur on the pages.
- `display-enable-printed: no`. The pages print the SR1 event that returns the STIC to CPU controlled mode, not a way for the program to enable or disable the display.
- `colors-numbered-printed: no`. The first page prints that 16 colors are selectable; no page numbers them or says which are the primary set.
- `window-lengths-printed: no`. No length of a CPU window, of CPU time or of graphic time, in any unit, is printed.
- `bus-hold-printed: no`. The system RAM page prints that the copy mode ends at the first bus-acknowledge edge and the STIC page prints that the second sync signal is issued thirteen or fourteen times per frame; no hold duration or count of cycles is printed.
- `rows-per-frame-printed: yes`. Twelve card rows of twenty cards, 16 lines per card row in the text (one first line through the second and third sync signals and fifteen more through the third alone), and thirteen or fourteen second-sync requests per frame. The 262 lines of a frame are not printed.
- `visible-geometry-printed: yes`. 167 x 105 positions for the NTSC part of which 159 x 96 are visible, 19 columns of 8 x 8 and a 20th of 7 x 8, 240 words of 14 bits, 8 objects.
- `timing-figure-readable: partial`. The figure on pdf 193 (and 1982 pdf 464) is legible. It prints numbers only for the bus protocol (set-up and delay values in nanoseconds in the control timing figures, and 100, 100, 700 and 50 nanoseconds in the object fetch figure); the synchronisation figure (SR1 pulse, CPU time, graphic time, the SR2 pulse train and the SR3 bursts of lines 1 to 16) carries no number, is drawn schematically with an elided pulse train and cannot be counted. It therefore prints no frame-level or window-level figure usable as a window length.

Effect of the verdict (all as pre-read, with one change): every register-level, card-bit and window fact is tier 4 (one community lineage) or `unknown-default-chosen`, and every claim built on them stays `provisional`.
The one change is `backtab-format-printed: partial`: the BACKTAB address and extent and the 14-bit use move up to tier 1 (rules CR-01, CR-02, STIC-R03 in the table below), which the plan structure already allowed for; the field layout stays tier 4 (CR-06..CR-11).
The RA-3-9600 page on pdf 200 (printed 5-19) prints a 1.79545 MHz input for the system RAM; that is a tier 1 data point for the input clock chain of C8 (half of the 3.579545 MHz crystal) and changes no profile field.

## Sources and method

The Phase 6 sources are the two catalogs above and the community wiki pages read in plan 06-01: the STIC page (locator `STIC-oldid-15008`), the memory map page (`Memory-Map-oldid-14847`) and the BACKTAB page (`BACKTAB-oldid-14739`), all under the registry id `SRC-intv-wiki`.
The wiki pages and the five word-format figures of the STIC page were fetched on 2026-11-12 into the gitignored cache (`refs/stic`) and the figures were read as images; no wiki image is committed.
The page table of the STIC page, the memory map table and the text were read in full, and the register literal and the card bit-role literal were written from them.
Registry additions: none. The existing entries `SRC-gi-1980-catalog`, `SRC-gi-1982-catalog`, `SRC-gi-cp1600-um-1975`, `SRC-bsr-2609` and `SRC-intv-wiki` cover every cite of the Phase 6 records; community emulator documentation stays reference-only and is never a row's source.
No emulator source or table was used for any value.

## What the pages print (tier 1)

- The STIC is the AY-3-8900-1 for NTSC with a 3.579545 MHz STIC input clock; the AY-3-8900 is the PAL part with a 4.000 MHz input clock and a 168 x 104 grid (out of scope, D-03).
- Eight foreground objects on a 167 x 105 grid, of which 159 x 96 positions are visible; a background of 20 x 12 cards of 8 x 8 picture elements, the 20th column 7 elements wide; 240 background words of 14 bits in external memory; 16 selectable colors; the graphics pattern memory is 8 bits wide.
- The STIC and the CPU time-share a bidirectional 14-bit bus; its CPU control bus is validated only in CPU controlled mode and is ignored otherwise.
- The first sync signal occurs once per frame and says that STIC time is complete and CPU mode has begun; the second is issued thirteen or fourteen times per frame, once per card row, and the third pulses once per card position, the first line of a card row being fetched with the second and the following fifteen with the third alone.
- The system RAM page: a bus copy mode entered when the interrupt acknowledge is decoded and left at the first bus-acknowledge edge; the 20-word line buffer; the 352 words at 512..863 and their 14-bit use.
- The system RAM input of 1.79545 MHz (pdf 200).

## Alternative window hypothesis (D-05)

The copy mode of the system RAM page, with the CPU-mode rule of the STIC page, can explain the register and graphics-memory windows by one mechanism (an event machine: the interrupt acknowledge enters copy mode, the first bus acknowledge leaves it).
Phase 6 does not implement it: the windows are a pure function of the access stamp (the shorter community figures 2,000 and 3,780 cycles, rules STIC-R18 and STIC-R19), because the pages print no window event and the community figures are stated relative to the vertical blank.
The event machine is recorded as the alternative with its page citation (rules STIC-R06 and STIC-R21, discrepancy C83); reversal is the owner window-edge probe D10-VID-06 or a primary page that prints window events.

## Rule tier table

Basis per rule of `data/stic/rules.json` (the exact statements, fields and test families are in that file and are checked by the `video` step): tier1-literal is printed on the pages above, tier1-derived follows from printed text, tier2-bsr is the Blue Sky Rangers page, tier4-community is one community lineage, inference and default-unknown carry a resolver and a trigger in their note, project-rule is a design rule of this project and not a hardware statement.
Counts of the 53 rules: 8 tier1-literal (STIC-R01 to R04, R06, CR-01 to CR-03), 1 tier1-derived (STIC-R05), 1 tier2-bsr (STIC-R23), 15 tier4-community, 5 inference, 13 default-unknown, 10 project-rule.

| Rule | Basis | Cite | Conflict | Statement (short) |
|------|-------|------|----------|-------------------|
| STIC-R01 | tier1-literal | SRC-gi-1980-catalog@pdf191 | C11 | The STIC is the AY-3-8900-1 for NTSC, with a STIC input clock of 3.579545 MHz, a 167 x 105 ... |
| STIC-R02 | tier1-literal | SRC-gi-1980-catalog@pdf191 | C86 | The STIC data bus is 14 bits wide; bits 15 and 14 of a CPU word are never driven by the STIC. |
| STIC-R03 | tier1-literal | SRC-gi-1980-catalog@pdf199 | C86 | Graphics words use the low 14 bits of a system RAM word; the top two bits of a BACKTAB word ... |
| STIC-R04 | tier1-literal | SRC-gi-1980-catalog@pdf191 | C83 | The STIC ignores its CPU control codes unless it is in CPU controlled mode. |
| STIC-R05 | tier1-derived | SRC-gi-1980-catalog@pdf191 | C5 | A write to a STIC register outside the register window is dropped and has no effect. |
| STIC-R06 | tier1-literal | SRC-gi-1980-catalog@pdf199 | C83 | Bus copy mode is entered when the system RAM decodes the interrupt acknowledge and left at ... |
| STIC-R07 | inference | SRC-gi-1980-catalog@pdf199 | C83 | A STIC register write needs the register window (default: the first 2,000 cycles of a frame). |
| STIC-R08 | inference | SRC-gi-1980-catalog@pdf199 | C2 | CPU access to the graphics ROM and RAM needs the graphics-memory window (default: the first ... |
| STIC-R09 | tier4-community | SRC-intv-wiki@STIC-oldid-15008 | C81 | Register address roles: MOB X at $00-$07, MOB Y at $08-$0F, MOB attribute at $10-$17, ... |
| STIC-R10 | default-unknown | SRC-intv-wiki@STIC-oldid-15008 | C86 | Stored width of each register: X 11 bits, Y 12 bits, attribute 14 bits, collision 10 bits, ... |
| STIC-R11 | default-unknown | SRC-intv-wiki@Memory-Map-oldid-14847 | C81 | Only the collision registers return stored data (10 bits over the undriven fill, so $FC00 or ... |
| STIC-R12 | default-unknown | SRC-intv-wiki@STIC-oldid-15008 | C81 | Reserved registers ($22-$27, $2D-$2F, $33-$3F) drop writes and read the undriven fill. |
| STIC-R13 | tier4-community | SRC-intv-wiki@STIC-oldid-15008 | C82 | A write to $0020 inside the register window sets the display latch for the current frame; ... |
| STIC-R14 | tier4-community | SRC-intv-wiki@STIC-oldid-15008 | C6 | A write to $0021 selects Foreground/Background mode and a read of $0021 selects Color Stack ... |
| STIC-R15 | default-unknown | none | C82 | A read of $0020 has no effect. |
| STIC-R16 | tier4-community | SRC-intv-wiki@Memory-Map-oldid-14847 | C84 | Writes through the aliases $4000, $8000 and $C000 plus a register offset reach the register; ... |
| STIC-R17 | default-unknown | SRC-intv-wiki@Memory-Map-oldid-14847 | C84 | Reads of $0040-$007F have no effect and return no STIC data. |
| STIC-R18 | tier4-community | SRC-intv-wiki@Memory-Map-oldid-14847 | C1 | The register window is the first 2,000 CPU cycles after the frame start (alternate 2,900), a ... |
| STIC-R19 | tier4-community | SRC-intv-wiki@Memory-Map-oldid-14847 | C2 | The graphics-memory window is the first 3,780 CPU cycles after the frame start (alternate ... |
| STIC-R20 | default-unknown | none | C94 | The windows apply whether or not the display latch is set. |
| STIC-R21 | inference | SRC-gi-1980-catalog@pdf199 | C83 | Without an interrupt acknowledge there is no register or graphics-memory access during an ... |
| STIC-R22 | default-unknown | none | C82 | A $0020 strobe stamped inside the register window arms the frame; a later strobe is dropped ... |
| STIC-R23 | tier2-bsr | SRC-bsr-2609@2609-overview | C99 | A frame whose display latch is clear when window 0 would start is a blank frame: no bus ... |
| STIC-R24 | default-unknown | none | C5 | A blocked read returns the undriven fill $FFFF with no effect; a blocked write is dropped; ... |
| STIC-R25 | default-unknown | none | C85 | Power-on state: registers zero, Foreground/Background mode, display latch clear. |
| STIC-R26 | project-rule | none | C94 | An event stamped at or below an access stamp is applied before the access (step first, then ... |
| STIC-R27 | project-rule | none | C84 | ivs_machine_peek is the bus view and has no side effect; the STIC state report is the stored ... |
| STIC-R28 | project-rule | none | C87 | The color stack and border color registers are applied to the picture, and the MOB registers are applied by STIC-R37 to R43 ... |
| STIC-R29 | project-rule | none | C7 | A frame is a STIC timeline event of constant length F = ticks_per_cpu_cycle x ... |
| STIC-R30 | default-unknown | none | C91 | INTRM is a level asserted from S_k for 3,791 CPU cycles; a unit starting at b sees the level ... |
| STIC-R31 | default-unknown | SRC-gi-1980-catalog@pdf191 | C92 | With the display latch set, 13 bus windows per frame start at cycle 3,791 + 912 i and hold ... |
| STIC-R32 | inference | SRC-gi-cp1600-um-1975@pdf36 | C52 | A bus stall resumes the CPU at w + 4 x (H + 1 + L) = w + 468 ticks (117 cycles) after the ... |
| STIC-R33 | project-rule | none | C94 | The STIC is advanced to the end of every unit and to the stamp of every access that can ... |
| STIC-R34 | project-rule | none | C104 | FRAME_END(k) is processed before FRAME_START(k+1) and before an access stamped exactly E_k; ... |
| STIC-R35 | project-rule | none | C104 | The video capacity of a run call is video_slot_bytes x (D / F + 2), with one slack frame; ... |
| STIC-R36 | project-rule | none | C10 | A frame slot is a 32-byte big-endian header (tick E_k, index, width, height, flags, ... |
| CR-01 | tier1-literal | SRC-gi-1980-catalog@pdf199 | C96 | BACKTAB is 20 x 12 cards, 240 words of 14 bits at $0200-$02EF; card (r, c) is word 20 r + c. |
| CR-02 | tier1-literal | SRC-gi-1980-catalog@pdf199 | C96 | Only bits 13..0 of a BACKTAB word are used; bits 15 and 14 are ignored. |
| CR-03 | tier1-literal | SRC-gi-1980-catalog@pdf191 | C10 | A card is 8 x 8 pixels; the 20th column shows 7 pixels, so the visible plane is 159 x 96 ... |
| CR-04 | inference | SRC-bsr-2609@2609-overview | C96 | A card byte is one pattern row: byte 0 is the top row, bit 7 the leftmost pixel, bit 1 the ... |
| CR-05 | tier4-community | SRC-intv-wiki@Memory-Map-oldid-14847 | C21 | Graphics ROM at $3000-$37FF and graphics RAM at $3800-$39FF, 8 bytes per card, address base ... |
| CR-06 | tier4-community | SRC-intv-wiki@STIC-oldid-15008 | C96 | The select bit is bit 11 in both modes: 0 is the graphics ROM and 1 the graphics RAM. |
| CR-07 | tier4-community | SRC-intv-wiki@STIC-oldid-15008 | C103 | A graphics RAM card number is bits 8..3 (0..63); bits 10 and 9 are ignored in Color Stack mode. |
| CR-08 | tier4-community | SRC-intv-wiki@STIC-oldid-15008 | C96 | Foreground/Background word: foreground = bits 2..0 (0..7), card = bits 8..3, select = bit ... |
| CR-09 | tier4-community | SRC-intv-wiki@STIC-oldid-15008 | C96 | Color Stack decode by bits 12 and 11: 0 0 graphics ROM card of bits 10..3 (0..255) with ... |
| CR-10 | tier4-community | SRC-intv-wiki@STIC-oldid-15008 | C97 | Color Stack background = stack[pos] after the card's advance; the stack is $0028..$002B. |
| CR-11 | tier4-community | SRC-intv-wiki@STIC-oldid-15008 | C97 | Stack position starts at 0 at every frame render, advances (mod 4) before the card is drawn ... |
| CR-12 | project-rule | none | C10 | Plane values are STIC color numbers 0..15; 0xFF is the not-displayed value; the palette is ... |
| CR-13 | tier4-community | SRC-intv-wiki@STIC-oldid-15008 | C98 | Colored Squares: four 4 x 4 quadrants with colors c0 = bits 2..0, c1 = bits 5..3, c2 = bits ... |
| CR-14 | tier4-community | SRC-intv-wiki@STIC-oldid-15008 | C96 | Each card pixel is the foreground index where the pattern bit is 1 and the background index ... |
| CR-15 | default-unknown | none | C99 | With the display off the plane is filled with the border color index and no BACKTAB word or ... |
| CR-16 | default-unknown | none | C85 | The power-on mode is Foreground/Background. |
| CR-17 | project-rule | none | C100 | The picture is rendered at the frame end from the registers and graphics memory as of E_k ... |

## Allocation table

One table fixes every id used by the later plans of Phase 6 (D-22, A-11). The plans cite these ids and never allocate their own.

### Discrepancies C81..C105 (25 new, ascending, each open)

The columns name the source note id the entry merges: register note (R), timing note (T), card note (CV).
Resolver kinds: `measurement:` only where a D10 line exists (C82, C83, C85, C89, C92, C99, C100, C101 and the updated C10); every other new entry resolves by a `test:` family of a named plan or by `pending:`.

| Id | Topic | Merges | Resolver |
|----|-------|--------|----------|
| C81 | register read-back model | R C81 | pending: STIC register description or reader diagnostic |
| C82 | display-enable strobe semantics | R C82 | measurement: D10-VID-08 |
| C83 | access-window mechanism (pure schedule chosen, copy mode alternative) | R C83 | measurement: D10-VID-06 |
| C84 | alias sensitivity | R C84 | test: stic.alias.* (06-06) |
| C85 | power-on state and mode | R C85, CV-6 | measurement: D10-VID-09 |
| C86 | stored widths and unknown bits | R C86 | pending: STIC register description |
| C87 | collision registers | R C87 | pending: Phase 8 |
| C88 | register decay | R C88 | pending: D10 long run |
| C89 | order of interrupt request, SR1 and frame start | R C89 | measurement: D10-VID-04 |
| C90 | frame phase at power-on | T C81 | test: stic.profile.* (06-06) |
| C91 | VBLANK line source, width, release | T C82 | test: stic.lines.* (06-08) |
| C92 | bus-window table | T C83 | measurement: D10-VID-07 |
| C93 | late grant and row-latch instant | T C84 | test: stic.steal.* (06-08) |
| C94 | access-stamp instant, windows with display off | T C85 | test: stic.window.* (06-06) |
| C95 | usable-cycle aggregates | T C86 | pending: plausibility record |
| C96 | card word layout | CV-1 | implemented default, provisional (one community lineage); test: video.fgbg.*, video.cstack.F-B*, video.probe.bit-roles-* (06-10, 06-11); pending: STIC description or D11 run |
| C97 | Color Stack advance timing and frame reset | CV-2 | implemented default, provisional: advance before draw, wrap 3 to 0, one stack across rows, restart at 0 every frame render; test: video.cstack.advance-*, video.cstack.reset-per-frame, video.cstack.one-stack-across-rows, video.mode.* (06-11); pending: a hardware reference |
| C98 | Colored Squares decode | CV-3 | implemented default, provisional (the order of the two low bits of the fourth color is the recorded open reading); test: video.cstack.squares-*, video.card.cs-decode, video.probe.bit-roles-cs-sq (06-11); the documented cut line is in docs/video/frame-derivations.md; pending: a hardware reference |
| C99 | display-off content | CV-4 | measurement: D10-VID-03 |
| C100 | BACKTAB fetch time, mid-frame writes | CV-5 | measurement: D10-VID-05 |
| C101 | which 7 pixels of column 19 | CV-7 | measurement: D10-VID-02 |
| C102 | cut-off column advances the stack | CV-8 | implemented default, provisional (inference); test: video.cstack.cutoff-column-advances, video.cstack.F-B2 (06-11) |
| C103 | graphics RAM bits 10..9 | CV-9 | test: video.card.* (06-10) |
| C104 | frame boundary tie rule | frame note | test: video.boundary.* (06-09) |
| C105 | border, window origin, row line count | frame note | pending: Phase 8 |

### Updates (not reopenings)

C1, C2, C3, C4, C5, C6, C7 gain text naming their profile field and test family; C10 goes from open to partial and its default names `video.last_card_column_px` = 7 with the one-field reversal to 8 (resolver now D10-VID-01); C21, C38, C41 (the re-entry consequence), C52, C60, C61 gain text. All keep their ids and the `unknown-default-chosen: ` prefix.

### Claims CL-0016..CL-0022 (reserved here, added by plan 06-14, never earlier)

| Id | Subject | Tier at the weakest field |
|----|---------|---------------------------|
| CL-0016 | register file, aliases and handshake | source-expectation |
| CL-0017 | timeline, VBLANK interrupt and bus steal | deterministic-repeatability |
| CL-0018 | raw framebuffer geometry, layout and chain | source-expectation (structure), deterministic-repeatability (layout) |
| CL-0019 | Foreground/Background and Color Stack frames | source-expectation |
| CL-0020 | `ivs-diag-stic-1` | original-diagnostic |
| CL-0021 | inspection without side effects | deterministic-repeatability |
| CL-0022 | installed package replay | installed-host-behavior |

None is at the measured tier.

### D10 inventory (ten new lines, `d10-items=` 32 to 42)

D10-VID-01 C10 (a 160th column), D10-VID-02 C101 (which 7 pixels), D10-VID-03 C99 (display-off output), D10-VID-04 C89 (interrupt versus end of picture), D10-VID-05 C100 (BACKTAB write during active display), D10-VID-06 C83 (window lengths and mechanism), D10-VID-07 C92 (bus-window count, spacing, hold), D10-VID-08 C82 (latch persistence and strobe), D10-VID-09 C85 (power-on state and mode) and D11-EXEC-03 C20 (a real system image drawing its title screen).
The procedure anchors are in `docs/stic/d10-procedure.md`.

### Rule ids and frames

Rules: STIC-R01..STIC-R36 and CR-01..CR-17. Frames: F-A1..F-A5 and F-B1..F-B3 (unit frames, `tier source-expectation`), F-D1 and F-D2 (diagnostic frames, `tier original-diagnostic`).
Profile fields and test-id families the rules name but the profile and the suites do not have yet are in `data/stic/fields-pending.ids` and `data/stic/tests-pending.ids`, each line with its owner plan.

## Inspection path (plan 06-07)

STIC-R27 in practice (the artifact of the future claim CL-0021). The machine offers two views of the video device and neither has a read side effect. `ivs_machine_peek` is the bus view: it returns what a CPU read of an address returns now, with the register and graphics-memory gates honoured, and it takes a const handle, so the mode-select read effect (a read of `$0021` or of its aliases `$4021`, `$8021`, `$C021` inside the register window selects Color Stack) never fires for it. That effect lives only in the bus wrapper that the CPU path owns and is keyed on the access address before decode. `ivs_machine_stic_state` is the stored view: the mode, the display latch, the two gate bits as of machine time, the event cursor, the frame index and frame start tick, the blocked and late-grant counters and the 64 register words as stored, with no read mask and no gate. It advances no time, performs no catch-up (the device is caught up at every unit end), writes no machine byte and shares no function with peek, so a debugger can look at a register that a CPU read could not even see. Its report is 208 bytes in version 1 and every member is a fixed-width integer, so the layout is the same on every ABI. The tests that guard this are `stic.peek.mode-unchanged-0021`, the whole-instance sweep `stic.peek.sweep-bytes-equal`, its positive control `stic.peek.cpu-read-switches` (a CPU read of `$0021` inside the window does switch the mode, one outside it does not) and the `stic.state.*` family; the named fault `STIC_MODE_SWITCH_ON_PEEK` shows that the sweep can fail. [nc]

## Independence and second reads

The register literal, the geometry literal, the card bit roles, the chain framing and the ten frames were written from the sources and the written rules before any comparison with the planning notes' tables and before any core code exists (D-02); policy numbers (windows, hold, interrupt width, power-on state) are never expected values.
The blind second reads are requested in `docs/stic/second-read-request.md`; the author, the reader and the core share one model family, so agreement is a check of transcription and derivation and not an independent implementation comparison (C43).
