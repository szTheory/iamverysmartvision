# Hand derivation of the diagnostic `ivs-diag-stic-1`

Plan 06-01 (D-20, A-04, D-02, V6-9, V6-10). This is the derivation of the third original diagnostic fixture, written before the fixture, the scenarios or any STIC code exists: no listing, no `.expect` and no frame hash of the program exists yet.
It is documentation and expected values only. Plan 06-13 builds the listing from it, moves the asserted values into the header of the `.expect` file and checks them against a dev report.
Every value below is derived by hand from the register literal `data/stic/intv-2609-ntsc-family-p1.regmap.literal.txt`, the timeline of `docs/stic/timing-notes.md`, the card rules of `data/stic/rules.json` and the CPU cycle table of `data/cp1610/opcode-families.json`; none comes from a generator, a profile field or a core output.
Cycle counts are inference (the branch counts are the policy of C29, the reset entry an inference, the bus windows C92 and C93) and only the margins below are asserted by design; no scenario asserts a machine time.

## (a) Program design

The program is a cartridge (kind `cart`, one IVSC segment at `5000`) entered through the project boot stub, contract 1, unchanged: no interrupt service routine, no EXEC call, no graphics ROM read, interrupts disabled, R6 = `0340`.
The display is not sustained by an interrupt routine, so the diagnostic shows exactly one picture, frame 0 (a project interrupt routine is a Phase 7 need).
It has 133 words at `5000`-`5084` (limit 170) and 4 branches (limit 8):

| Step | Instructions | Words | Role |
|------|--------------|-------|------|
| a | `MVI $01FE,R0`, `ANDI #$00FF,R0`, `MVO R0,$02F1`, `CMPI #$00FF,R0`, `BZE skip`, `MVI $0021,R0` | 12 | read controller port 0, keep the low byte, record it, and select Color Stack mode only when a key is closed (low byte not `FF`): one branch |
| b | `skip: MVO R0,$0020` | 2 | display-enable strobe (the value written is ignored, C82) |
| c1 | `MVII #$0028,R5` and four times `MVII #v,R0; MVO@ R0,R5` with v = `06`, `0A`, `03`, `0D` | 14 | the four color stack registers `$0028`..`$002B` (used by Color Stack mode only) |
| c2 | `SDBD; MVII #$5065,R4`, `SDBD; MVII #$3800,R5`, `MVII #32,R1`, loop `MVI@ R4,R0; MVO@ R0,R5; DECR R1; BNZE` | 15 | copy 32 bytes from the table at `5065` to graphics RAM `$3800`..`$381F`: cards 0..3 |
| d | `CLRR R0`, `MVII #$0200,R4`, `MVII #240,R1`, loop `MVO@ R0,R4; DECR R1; BNZE` | 9 | fill the 240 BACKTAB words `$0200`..`$02EF` with `0000` |
| e | six times `SDBD; MVII #word,R0; MVO R0,$02xx` | 36 | the explicit card words |
| f | `MVII #900,R1`, loop `DECR R1; BNZE` | 5 | counted delay |
| g | `MVII #$03C5,R0`, `MVO R0,$02F0`, `HLT`, three `TSTR R0` | 8 | end marker written last, then the stop and its three followers |
| data | 32 table words | 32 | the card patterns, below |

Addresses: step a `5000`-`500B`, `skip` at `500C`, c1 `500E`-`501B`, c2 `501C`-`502A`, d `502B`-`5033`, e `5034`-`5057`, f `5058`-`505C`, g `505D`-`5064`, the table at `5065`-`5084`. Branch targets (to be annotated `-> ADDR` by plan 06-13): `BZE` at `5008` to `500C` (forward, displacement 2), `BNZE` at `5029` to `5026` (backward), at `5032` to `5030`, at `505B` to `505A`.
Every 14-bit word is written as `SDBD; MVII` (the operand decles are the low byte then the high byte) followed by `MVO`, because a decle holds 10 bits; an immediate below `0400` is one decle. All operand addresses of `MVO` are below `0400` (one decle each), so no other instruction needs `SDBD`.
The marker and the recorded port byte are stored at `$02F0` and `$02F1`, outside BACKTAB (`$0200`..`$02EF`), because a word written into BACKTAB would be drawn.

### The card patterns (project-authored geometric shapes, table at `5065`)

| Card | GRAM bytes `$3800 + 8 card + y`, y = 0..7 | Shape |
|------|------------------------------------------|-------|
| 0 | `FF 00 FF 00 FF 00 FF 00` | stripes |
| 1 | `FF 81 81 81 81 81 81 FF` | box |
| 2 | `80 40 20 10 08 04 02 01` | diagonal |
| 3 | `AA 55 AA 55 AA 55 AA 55` | checker |

No letter shape and no pattern recalled from a real graphics ROM is used.

### The six explicit words (BACKTAB cell, address, word, fields)

| Cell | Address | Word | Foreground/Background reading (CR-08) | Color Stack reading (CR-09, CR-10) |
|------|---------|------|---------------------------------------|------------------------------------|
| (8,3) | `$02A3` | `2A01` | graphics RAM card 0, foreground 1, background 5 | card 0, foreground 1, advance |
| (8,4) | `$02A4` | `0E0A` | card 1, foreground 2, background 3 | card 1, foreground 2 |
| (8,5) | `$02A5` | `1A13` | card 2, foreground 3, background 9 | card 2, foreground 11 (bit 12 is foreground bit 3) |
| (8,6) | `$02A6` | `381C` | card 3, foreground 4, background 12 | card 3, foreground 12, advance |
| (11,0) | `$02DC` | `2C1D` | card 3, foreground 5, background 6 | card 3, foreground 5, advance |
| (11,19) | `$02EF` | `1C0F` | card 1, foreground 7, background 10 | card 1, foreground 15 |

The same words are read in both modes, so the two pictures differ only because of the mode; this is the diagnostic form of the cross-mode pair F-A3 and F-B3 (`docs/video/frame-derivations.md`).
Cell address = `0200` + 20 r + c: (8,3) is `0200 + 163 = 02A3`, (11,0) is `0200 + 220 = 02DC`, (11,19) is `0200 + 239 = 02EF`.
The fill word `0000` is, in Foreground/Background mode, a graphics ROM card 0 with foreground 0 and background 0, and in Color Stack mode a graphics ROM card 0 (bits 13, 12, 11 = 0, 0, 0) with foreground 0 and no advance, which shows the stack entry at the current position; a graphics ROM card with no image shows only the background (zero ROM stand-in, CR-14).

## (b) Mode selection by the controller input

At tick 0 the scenario gives `input at_tick=0 c0=KEY_1` for the Color Stack run and nothing for the Foreground/Background run.
The first read of `$01FE` then returns the low byte `7E` (KEY_1 closed alone, controller literal) or `FF` (nothing closed), so `CMPI #$00FF` sets Z only without input, `BZE` is taken, and the read of `$0021` (which selects Color Stack mode, STIC-R14) is skipped. One image and one branch cover both modes.
The recorded byte is `[02F1] = 00FF` without input and `007E` with KEY_1.

## (c) Cycle budget (A-04)

Cycles count from tick 0 (4 ticks per cycle) along the Foreground/Background path with the display strobed; the Color Stack path is +8 cycles (the `BZE` is not taken, 7 instead of 9, and the read of `$0021` adds 10). Costs are the printed counts of `data/cp1610/opcode-families.json`: `MVI` 10, `MVO` 11, `MVII` 8, `ANDI` 8, `CMPI` 8, `MVI@` 8, `MVO@` 9, `DECR` 6, `CLRR` 6, `SDBD` 4 and an `SDBD`-prefixed `MVII` 14, taken branch 9, not taken 7, `HLT` 4, `TSTR` 6.

| Row | Arithmetic | Cycles | Cumulative end |
|-----|------------|--------|----------------|
| Reset entry and boot stub to the cartridge entry (inference) | 5 + `B` 9 + `DIS` 4 + `MVII` 8 + `J` 12 | 38 | 38 |
| a, to the strobe | `MVI` 10 + `ANDI` 8 + `MVO` 11 + `CMPI` 8 + `BZE` taken 9 | 46 | 84 |
| b, strobe store | `MVO` 11 | 11 | 95 |
| c1, stack registers | `MVII` 8 + 4 x (`MVII` 8 + `MVO@` 9) | 76 | 171 |
| c2, GRAM copy setup | 2 x (`SDBD` 4 + `MVII` 14) + `MVII` 8 | 44 | 215 |
| c2, GRAM copy loop | 31 x (8 + 9 + 6 + 9) + (8 + 9 + 6 + 7) | 1,022 | 1,237 |
| d, fill setup | `CLRR` 6 + `MVII` 8 + `MVII` 8 | 22 | 1,259 |
| d, fill loop | 239 x (9 + 6 + 9) + (9 + 6 + 7) | 5,758 | 7,017 |
| e, six explicit words | 6 x (4 + 14 + 11) | 174 | 7,191 |
| f, delay | `MVII` 8 + 900 x 15 - 2 | 13,506 | 20,697 |
| g, marker | `MVII` 8 + `MVO` 11 | 19 | 20,716 |

With the display on, bus windows stall the CPU: window i starts at cycle 3,791 + 912 i and one window costs at most 117 cycles and at least 87 cycles of delay (the grant comes at the first open boundary, which is at most 30 cycles after the window start in these loops; resume = window start + 117, C92, C93). Thirteen windows start in frame 0 (windows 0..12), so the total delay is at most 13 x 117 = 1,521 cycles and at least 13 x 87 = 1,131 cycles; the frame has 14,934 cycles.

Margins (each is a requirement of the design; the thresholds are the plan's):

| Item | Value | Margin | Threshold |
|------|-------|--------|-----------|
| Strobe store stamp, latest | the `MVO` unit of b ends at cycle 95 (103 on the Color Stack path); register window 2,000 | at least 1,897 cycles | 100 cycles |
| Last stack register store | ends at cycle 171 (179) | at least 1,821 cycles | 100 cycles |
| Mode read of `$0021` (Color Stack path) | stamped inside the unit that ends at cycle 92 | at least 1,908 cycles | 100 cycles |
| Last GRAM store | the last `MVO@` of c2 ends at cycle 1,237 - 13 = 1,224 (1,232); graphics-memory window 3,780 | at least 2,543 cycles | 100 cycles |
| BACKTAB row 0 complete | word 19 is stored by the loop iteration that starts at 1,259 + 24 x 19 = 1,715 and ends 9 cycles later: 1,724; row latch 0 at 3,848 | at least 2,124 cycles | 200 cycles |
| BACKTAB row 11 complete (fill) | word 239 is stored by the iteration that starts at 1,259 + 24 x 239 = 6,995, at most 7,004 plus 5 x 117 = 585 of delay: 7,589; row latch 11 at 3,848 + 912 x 11 = 13,880 | at least 6,291 cycles | 200 cycles |
| BACKTAB rows 8 and 11, explicit words | the last explicit store ends at 7,191 plus at most 5 x 117 = 585: 7,776; row latch 8 at 3,848 + 7,296 = 11,144, row latch 11 at 13,880 | at least 3,368 cycles (row 8) | 200 cycles |
| HLT start, display on | 20,716 plus between 1,131 and 1,521 of delay: between 21,847 and 22,237 cycles = between 87,388 and 88,948 ticks (`E_0` = 59,736, `E_1` = 119,472) | at least 27,652 ticks after `E_0` and at least 30,524 ticks before `E_1` | 4,000 ticks both sides |
| HLT start, mutant `diag-stic-nodisplay` (strobe store changed, no windows) | 20,716 cycles = 82,864 ticks (Color Stack path +32 ticks) | 23,128 ticks after `E_0`, 36,608 ticks before `E_1` | 4,000 ticks both sides |
| Scenario total | HLT start at most 88,948 ticks plus `HLT` 16 ticks and one follower 24 ticks | about 89,000 ticks | at most 150,000 ticks |

The HLT start sits between `E_0` and `E_1` by design: the runner always sets the stop-on-halt flag, and a frame completes only when machine time reaches `E_k`, so the program must still be running when frame 0 ends and must stop before frame 1 ends (frame 1 is a blank frame: the display latch is cleared at every frame start and the program strobes only once).
The delay count 900 fits a 10-bit immediate (`0384`) and was chosen so the HLT start is near the middle of the interval (`E_0` + 4,000 ticks = cycle 15,934 and `E_1` - 4,000 ticks = cycle 28,868; the middle is cycle 22,401) in both cases.
The number of executed cycles at which machine time reaches `E_0` is an output of the instruction mix and the delays and is never an expected value: with the display on it lies between 13,413 and 13,803 executed cycles.

## (d) Expected state and frames

State words (the predicate, `original-diagnostic`, to be written into the header of the `.expect` file by plan 06-13): the stop reason halted; the end marker `[02F0] = 03C5`; the recorded port byte `[02F1] = 00FF` (no input) or `007E` (KEY_1); the six BACKTAB words `[02A3] = 2A01`, `[02A4] = 0E0A`, `[02A5] = 1A13`, `[02A6] = 381C`, `[02DC] = 2C1D`, `[02EF] = 1C0F` and the fill `[0200] = 0000`.
The first completed frame (frame 0) is the expected picture: `data/video/frames/F-D1.frame` for the Foreground/Background run (stack registers written but unused) and `data/video/frames/F-D2.frame` for the Color Stack run, both hand-derived from the words and the card rules before the listing exists.
Frame 0 has the display latch set at its row latches (the strobe lands at cycle 95 of the frame), the cards rows are latched at cycle 3,848 + 912 i with all 240 words and all six explicit words in place by the margins above, and the registers and graphics RAM at the frame end are the values written.
Under the mutant `diag-stic-nodisplay` frame 0 is a display-off frame filled with the border color 0, which differs from F-D1 and F-D2.

Excluded on purpose: machine time (printed in the report, never asserted), the frame tick, all CPU registers, R7 and the flags (they depend on the interruptibility and halt policies, C40, C45, C56), the stack word `[0340]`, the graphics RAM contents (not readable by the CPU at the stop) and the number of bus windows.

Hash tie (plan 06-13): the assert lines of the `.expect` file `assert frame 0 hash == <sha256-text>` must equal the `sha256-text` key of `F-D1.frame` and `F-D2.frame`, and a pure tie check between the two is added there; the frame files are the only place the digest is derived.
The scenario list of plan 06-13 (one image, both modes by input at tick 0, the mutants of `data/expected/diag-mutations-stic.ids`, the display-off contrast) rests on this derivation.

Shared-misreading risk: the author of the register literal, the card rules, the frames, this derivation, the listing and the core is the same model family, so all of them can share one misreading of a source; agreement among them is not independent evidence about hardware.
The card layouts rest on one community page (C96), the register roles on the same page (C81), and every window and bus figure is a default (C1, C2, C92); the claim on this diagnostic stays provisional and scoped to the project boot stub and this program.

## (e) D4 judgement: the project mini-assembler

D4 names a diagnostic count above 3, labels or macros, or a second author as the trigger for a project assembler.
After this phase there are three diagnostics (`diag-cpu-1`, `diag-ctrl-psg-1`, `diag-stic-1`; the boot stub is not a diagnostic and the in-process stand-in programs of the test suites are test code), which is not above 3, so D4 stands by its letter; the SDBD rule of the listing checker lands first (plan 06-01, `listing-sdbd`) and covers the only new encoding rule.
Pre-committed crossing rule (any one fires it; otherwise the decision moves to plan 1 of Phase 7): the listing exceeds 170 words or needs more than 8 branches; a hand-assembly error survives the listing checker and is found only by a run (count the escapes in the plan 06-13 SUMMARY); a second STIC cartridge is needed; a second author edits a listing.
Effort data plan 06-13 must record: the words derived (133 planned), the errors caught by the listing checker versus errors found only by a run, and the size of this derivation and of the listing (lines and words).

## (f) As built (plan 06-13, task 1; the sections above are unchanged)

The listing `fixtures/FX-diag-stic-1/diag-stic-1.listing.txt` was assembled by hand from sections (a) to (c): 133 words at `5000`-`5084` (the limit is 170; the plan of section (a) said 133), 4 branches in the program (`BZE` at `5008` to `500C`, `BNZE` at `5029`, `5032` and `505B`), and every address, displacement and word of section (a) as written. One fact the derivation did not foresee: the listing checker decodes every word of the file as a program word, so the 32 table decles are listed with the row of the decle each encodes, and the three decles `04`, `02`, `01` of card 2 decode as one three-decle jump (`J`), which the checker requires to carry a `-> ADDR` annotation (target `0001`, never executed because the table lies behind the three followers). The listing therefore lists 5 branches, 4 of them in code; both numbers are below the limit of 8 and no word, address or expected value changed.
The first run through `ivs-run` (the tracer, nothing recorded from it) matched both hand-derived frames: the text hash of frame 0 equalled the `sha256-text` of `F-D1.frame` without input and of `F-D2.frame` with KEY_1 at tick 0, the stop was halted, frame 0 displayed, one frame delivered. The stop came at machine time 88,476 ticks (Foreground/Background) and 88,500 ticks (Color Stack), inside the interval of section (c) (the HLT unit starts 40 ticks earlier: 88,436 and 88,460 ticks, which is 28,700 ticks after `E_0` and 31,036 ticks before `E_1`, above the 4,000 tick threshold on both sides). The margin of the mutant without the strobe is measured in the mutant task.
