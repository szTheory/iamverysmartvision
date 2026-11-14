# Phase 7 discuss: S-1, the scenario program (research agent A)

**Date:** 2026-11-13. **Scope:** what the first visible original program is and does: concept and name, controller mapping, the STIC and PSG features it uses, guest state and the progress predicate, size and branch budget under hand assembly, how input visibly changes video and audio, the scripted input timeline, what it deliberately does not do, and how it stays rights-clear. Sub-decisions S1-1 to S1-11.

**Labels.** [OBS] read in the repo or measured this session; [INF] inference; [REC] recommendation; [UNK] unknown; [BG] background knowledge not re-fetched. Nothing here is an owner decision, and no claim is about real hardware: every timing figure is a profile default (C1 to C4, C41, C91; tier 4 or `unknown-default-chosen`) and every project convention (entry `$5000`, stack `$0340`) has no hardware source (C68).

**Evidence base.** I read AGENTS.md, AUTONOMY.md, PROJECT.md (register and notes), REQUIREMENTS (SCN, VID, AUD, INP), ROADMAP Phases 7 and 8, the Phase 6 hand-offs (06-13, 06-14 summaries), the three diagnostics, the boot stub, the controller, PSG and STIC literals, the STIC timeline notes, `docs/runner.md` (scenario grammar), the scenarios and `.expect` goldens, and the sibling Phase 7 discuss notes (S-2 to S-5) to align interfaces. I also built a **throwaway spike** in a scratch directory outside the repo: a 110-line Python subset assembler, a candidate program, a contract-2 stub variant, and runs through the unmodified dev `ivs-run` and `ivs_pack`. The spike is not committed and is not the listing; it turns many inferences below into measurements. Its programs are hand-designed with explicit SDBD, as `ivs-diag-stic-1` is.

---

## 0. Recommendation on one screen

1. **S1-1 Concept: "Lamp Tone", a two-port lamp board with a voice per port.** Each frame the program reads both controller ports and shows the raw active-low byte of each as a row of 8 lamps (a lit lamp is a cleared bit); a changed port byte also retunes one PSG voice (pitch equals the byte, amplitude 15 when any control is closed, 0 when idle). It is an input-to-pixels-and-sound path with **no decode table, no text, no glyph, no GROM and no data table**. Names: fixture `FX-demo-lamp-tone-1`, program id `ivs-demo-lamp-tone-1`, scenarios `demo-lamp-tone-*` (the slug is a functional description, per S-5).
2. **S1-2 Mapping: no mapping.** `c0` drives the port at `$01FE`, `c1` the port at `$01FF` (never "player 1/2", C66). The guest renders the electrical byte itself: bit 7 to bit 0 left to right, lit when the bit is 0. Keypad key, disc direction and side button therefore appear as the ground-union of their pin lines, including the combinations the wiki says are ambiguous.
3. **S1-3 Features: Foreground/Background cards only.** One graphics-RAM card (solid), a 2 x 8 lamp block in BACKTAB, display strobe plus mode-select write each frame, PSG tone A/B with a fixed mixer, fixed amplitudes. No MOBs, no Color Stack, no border, no delay registers, no noise, no envelope, no GROM, no EXEC call.
4. **S1-4 Progress predicate.** Guest RAM at `$02F0..$02F5`: init marker `03C5`, a frame counter (one ISR per frame), and per port the last byte seen and a change counter. The predicate is cumulative: after the timeline, frame counter `000D`, change counters `0006` and `0003`, last bytes `5A` and `9F` [OBS, spike]. A never-halting program: assert `halted == no`.
5. **S1-5 Shape and size.** About 136 words (132 measured in the spike plus the contract-2 entry padding), 10 branch or jump instructions, 11 labels, **zero data words**. That sits under the 170-word line and over the pre-committed "more than 8 branches" crossing line, so the D4 trigger fires by its own rule whatever S-3 decides about tooling. The program needs labels, not a data directive.
6. **S1-6 Video and audio response.** Video: frame-granular lamp blocks, index 1 (unlit) or 2 (lit), a rectangle 64 x 8 positions per port at x 48..111, y 32..39 and y 56..63; idle frames show every lamp at 1 and everything else at 0. Audio: idle is silent (`audio.changes` 0 [OBS]); the PSG write log changes with every port change (exact addr and value); a held tone has half-period exactly `16 x byte` ticks [OBS]. The phase of a tone is not derivable without cycle accounting, so exact sample asserts are limited to silence unless S-4 adds one small assert (see S1-6, and the open conflict in Open item 1).
7. **S1-7 Timeline.** Six events, 120,000 ticks apart (two frames), each at 16,000+ ticks from any frame end, run to tick 806,568 (mid-frame 13): idle, KEY_1, KEY_1+KEY_9 (the wiki's documented `5A`), port 1 DISC_E, SIDE_TOP with DISC_E, all idle, then a final tableau of KEY_1+KEY_9 with SIDE_LOWER_LEFT. 13 frames, 25 PSG records. All numbers independently derived from the controller literal and cross-checked against the spike [OBS].
8. **S1-8 Not done:** MOBs, collision, delay registers, Color Stack, border, text, tunes, noise, envelope, GROM, EXEC, a quit/halt path, scoring, a game loop. Reason: each is Phase 8 or later, or adds rights surface, or hides the electrical model.
9. **S1-9 Rights.** Everything is project-authored: the listing, the derivation document, the card (one solid block), the two color numbers. MIT, `redistributable: yes`, `author-attested`, image built into the build tree only, boot stub plus zero GROM, no third-party byte.
10. **S1-10 Negative controls.** Eleven program-level mutants were run in the spike; each is caught by a different subset of three channels (guest state, video, PSG log), and seven of the eleven (plus the structural GRAM-LATE mutant) are visible to exactly one channel. That is the argument for asserting all three in one run [OBS].
11. **S1-11 Findings other points must absorb [OBS]:** a vector that does not start with a disable instruction re-enters about every 21 cycles (silent stack leak, scenario still `pass`); a delay loop in main re-arms interrupts safely over a wide band (N from 200 to 800 loop iterations), whereas padding inside the ISR has a margin of a couple of hundred cycles; a graphics-RAM write after the first 3,780 cycles of frame 0 is dropped silently and the whole picture goes blank while every guest-state predicate still passes.

---

## 1. The spike, in one table (what was measured)

All runs: unmodified dev runner and `ivs_pack` at branch HEAD, boot stub variant with the interrupt entry forwarded to the cartridge (see S1-5), zero GROM, profile `intv-2609-ntsc-family-p1`. [OBS] unless marked.

| Item | Result |
|------|--------|
| Program size | 132 words at `5000`..`5083`; 10 branch or jump instructions (7 relative: 4 `BNZE` loops, `B` self, 2 `BZE`; plus `J`, 2 `JSR`) |
| Timeline run (S1-7), 806,568 ticks | outcome `pass`, never halts; 13 frames delivered, frame 0 not displayed, frames 1..12 displayed; PSG log 25 records; `output.audio.changes` 375 |
| Same program, no input | PSG log 7 records (init only), `audio.changes` 0, every lamp at index 1, `$02F1` = `000D`, change counters 0 |
| Events shifted +2,000 ticks | the 13 delivered frames are byte-identical to the unshifted run |
| Independent model (Python reading only the controller literal and the rule "the ISR of frame k reads the state set by events before the frame start") | agrees with the run on every frame's lamp pattern, both change counters, the frame counter, the last bytes and the whole PSG sequence |
| Audio spacing of a held single voice | consecutive level changes exactly `16 x P` ticks apart: 2,016 (P=126), 1,440 (P=90), 4,048 (P=253); with two voices active the merged change list interleaves (needs per-channel reading) |
| ISR timing | first PSG write at about 800 ticks (200 cycles) after the frame start; the ISR (both ports, both lamp rows) lasts about 1,100 to 1,300 cycles [INF from the write ticks and the cycle table]; all BACKTAB writes land before cycle 1,300, row 4 latches at cycle 7,496 |
| Re-arm band | delay `N` in the main loop of 15 cycles per iteration: N <= 170 gives two ISR runs per frame; N 200 to 800 gives exactly one; N >= 850 skips frames |
| Cost | 0.03 s user time per 800,000-tick run in the sanitized dev runner |

The spike's assembler is not an argument for or against S-3: this program's encodings were mechanical (zero encoding defects on the first run), and the one defect found only by running was **behavioural** (S1-11 a), which neither an assembler nor the listing checker could catch.

---

## S1-1 The program concept and its title

**Constraints from the roadmap.** Visibly responds to controller input with video and sound; guest progress asserted by a RAM or register predicate, not a title screen or hash; rights-clear; original; no MOBs (Phase 8); small enough to hand-assemble (D4); no EXEC, no GROM (D5: the project boot stub and the zero GROM).

**Options considered.**

| Option | What it is | Words (est.) | Pro | Con |
|--------|-----------|--------------|-----|-----|
| A. Lamp board with a voice per port (**recommended**) | 16 lamps show the raw port bytes; pitch equals the byte | 132 to 136 (measured) | Tiny; exercises the whole input to guest to card to frame path and the PSG path; hand-derivable expectations from the controller literal alone; shows the active-low model and the ground-union rule on screen; no table | Not a game; "playable" is a stretch of the word |
| B. Decoded pad: 12 key cells plus a disc cursor | A keypad-shaped grid and a moving cursor | 200 to 260 | More recognisable as a program | Needs a decode (row/column from bit tests or a 31-row table), a cursor erase/draw, an edge detector; over the 170-word line by itself; decode logic hides the electrical model behind guest logic |
| C. Combination lock | A short key sequence advances a stage; the screen changes on success | 190 to 240 | A real goal, a crisp predicate, natural negative control (wrong key resets) | Needs a table (or compare chain), edge detection and a stage machine; roughly 60 to 110 extra words and 2 to 4 more branches |
| D. A tiny game (catch, reaction) | Moving object, scoring | 300+ | Closest to "playable" | Needs MOBs for anything that moves smoothly (Phase 8) or card-cell animation; far past the hand-assembly line |

**Recommendation [REC]: A, named "Lamp Tone".** It is the smallest program that satisfies SCN-01 literally (input changes guest state, video and audio, asserted by a guest predicate), it keeps the expected values derivable from `data/input/intv-2609-ntsc-family-p1.literal.txt` without any decode logic in the guest, and it is the right size for the first project-authored sustained program (a frame loop with an interrupt, a subroutine, a graphics-RAM card, PSG output). Its architecture (init, interrupt routine, per-port subroutine, sustained display) is the reusable part: a Phase 8 or later program (C or D) can reuse it.

**Naming [REC].** Fixture `FX-demo-lamp-tone-1`, program id `ivs-demo-lamp-tone-1`, listing id the same, scenarios `demo-lamp-tone-pass`, `-idle`, `-shift`, `-swap` (the S-5 pattern `FX-demo-<slug>-1`; the slug is functional). The program shows no text, so a title is never rendered, and the id is the only name that exists. [UNK] No formal trademark search was run; the name contains none of the exclusion-list words of S-5 (no console, company, product, character or nickname).

**Adversarial pass.** (1) "A lamp board is a diagnostic in a costume, not a program." True, and honest: the product lens would rank C above A. The reply is sequencing: the roadmap wants the first sustained visible program and a rights-clear pipeline now, and a goal-directed program (C) is a few dozen words and a table away once S-3's tooling exists. (2) "Echoing the byte tests the table, not the console." It tests the pipeline from logical input through the core's electrical function, the guest's read, the card word and the framebuffer; it does not say the table is right (C16 stays open and every claim stays provisional). (3) "Without a goal there is no 'progress'." The predicate is cumulative state advanced by input (counters that move by exactly the number of changes), the register-signature style of blargg and Mooneye-like test ROMs [BG].

**Reversal evidence.** The owner says the lamp board is not "playable" enough: build C (the combination lock) as a second program or a Phase 7 plan after S-3 lands; the lamp board stays as the layer-by-layer first slice. A planner who finds the hand derivation of A exceeds the 06-13 effort figures should reduce to one port (about 85 words) rather than drop the interrupt routine.

**Risks.** The word "playable" appears in the tier name only; wording is S-4's (always "playable guest progress, for this original program only"). A reviewer may read the lamp board as less than the roadmap's "playable result"; mitigated by saying what it is.

---

## S1-2 Controller mapping and the electrical active-low model

**Facts [OBS].** `c0` is the logical state of the port a guest reads at `$01FE`, `c1` of `$01FF`; the core derives the byte from the profile table (31 rows, tier 4 community, one lineage, C16) by the ground-union rule: byte = `FF` xor the OR of the cleared masks of every pressed control (D6, `data/input/intv-2609-ntsc-family-p1.literal.txt`). The pin-to-bit map: pin 4 = bit 0, pin 3 = bit 1, pin 2 = bit 2, pin 1 = bit 3, pin 9 = bit 4, pin 8 = bit 5, pin 7 = bit 6, pin 6 = bit 7. Keypad keys ground one of pins 6, 7, 8 (bits 7, 6, 5: the column) and one of pins 1 to 4 (bits 3..0: the row); side buttons ground two of pins 6, 7, 8; disc directions ground only pins 1 to 4 and 9 (cardinals one line, diagonals two or three). Wiki note 3: KEY_1 and KEY_9 together read `5A`.

**Options.**
- **A. No mapping; render the electrical byte (recommended).** Lamp i (left to right) is bit 7 minus i, lit when the bit reads 0. A press lights its pin lines. One idle value (`FF`), no decode.
- **B. Decode in the guest** (key number, disc compass point, side button): meaningful to a human, but it needs a table or bit-test cascade, hides the model and cannot represent the ambiguous combinations (key plus side, key plus disc, two side buttons) that INP-02 documents.
- **C. Raw hex digits on screen:** needs a font (text), rejected for rights and size.

**Recommendation [REC]: A.** The guest sees exactly what the profile says a real read returns, so the expected picture for any logical input is `ground-union(literal)` and needs no knowledge of how the guest works. The reading guide for a human: left three lamps are the keypad columns or side-button lines (pins 6 to 8), lamp 3 is the disc diagonal line (pin 9), right four lamps are the keypad rows and disc cardinals (pins 1 to 4). KEY_1 therefore lights lamps 0 and 7; KEY_1+KEY_9 lights 0, 2, 5, 7 (ambiguous with other combinations by design, which is the wiki's own statement).

**Bytes used by the S1-7 timeline (all from the literal, independent of any run):** KEY_1 `7E` (cleared `81`); KEY_1+KEY_9 `5A` (cleared `81 | 24 = A5`); DISC_E `FD` (cleared `02`); SIDE_TOP `5F` (cleared `A0`); SIDE_LOWER_LEFT `9F` (cleared `60`).

**Reversal evidence.** The owner measurement D10 changes the table or the combination rule: the program does not change; the derivations and the expected literal frames do (one classified diff). Raw port injection (D6 reversal) would add a second input form; the program already reads only the port word.

**Risks.** `c0`/`c1` naming: which physical connector feeds which port is unmeasured (C66), so no document, scenario comment or claim may say "player 1" or "player 2". Period 0 (a byte of `00`, all eight lines grounded) is policy territory (PSG-R07, C12); no timeline value produces it.

---

## S1-3 STIC and PSG features used

**STIC [OBS: `data/stic/`, `docs/stic/timing-notes.md`].**
- Foreground/Background mode, selected by a write to `$0021` every frame (STIC-R14, the register window, so it must sit in the interrupt routine, not the init tail). Explicit rather than relying on the power-on default (STIC-R25, CR-16), so a mutant that reads `$0021` instead is visible.
- One graphics-RAM card (card 0, solid: eight bytes `FF`) written once in init, **first**, inside the 3,780-cycle graphics-memory window of frame 0. BACKTAB words are `0801` (GRAM select, card 0, foreground 1: unlit) and `0802` (foreground 2: lit). The lit and unlit words differ by one in the foreground field, so the guest builds each lamp by adding the carry of a shift to the base word, with no branch per lamp.
- 16 lamp cells in rows 4 and 7, columns 6..13: BACKTAB `0256..025D` (port 0) and `0292..0299` (port 1). All other BACKTAB words are `0000` (a GROM card 0 with colors 0 over the zero GROM: pixel index 0 in the whole remaining plane [OBS]).
- The display strobe `$0020` as the first action of each frame's interrupt routine (cycle 20 to 35 of the frame, against a 2,000-cycle window).
- Not used: MOBs, border color, delay registers, Color Stack mode, graphics ROM text.

**PSG [OBS: `data/psg/`, rules PSG-R05, R14, R15, R16, R19].**
- Init: mixer `01F8` = `3C` (tones A and B enabled, tone C and all noise disabled, both ports input so reads return the pins); coarse tone registers `01F4`, `01F5` and amplitudes `01FB`, `01FC` = 0; fine tone registers `01F0`, `01F1` = `FF`. That is 7 init writes.
- On a port change only: fine tone period = the byte (`01F0` for port 0, `01F1` for port 1), then amplitude (`01FB` or `01FC`) = 15 if the byte is not `FF`, else 0. Pitch = `3,579,545 / (32 x byte)` Hz by PSG-R05 [INF: 8910 divide chain applied to the 8914-class part, C69]: 888 Hz for KEY_1, 1,243 Hz for the `5A` chord, 442 Hz for DISC_E. Half-period `16 x byte` ticks.
- The tone is a scale of whatever bytes the controls produce, not a melody: no recognisable tune exists in the program (S-5's tune rule).
- Not used: noise, envelope, channel C, amplitude mode bits, the DAC levels (the sample levels are the integer amplitude 0..15).

**Options for the sound.** (a) Pitch from the byte, amplitude on/off by activity (**recommended**). (b) A fixed pitch per port, amplitude only: simplest, but pitch would carry no information. (c) A constant-level "gate" on channel C (tone and noise off, so a constant `1` at the written amplitude per PSG-R15): exact sample levels for assertion, about 16 more words and 1 more branch (estimated, not run); see S1-6.

**Reversal evidence.** If C69 or C73 flips (mixer polarity, tone formula) only the derivations move. If S-4 rejects every grammar addition and the exact-audio design is needed, adopt (c) (S1-6).

**Risks.** The amplitude mode bit sense and the DAC are policy defaults (C12), and the 8914-class tone formula is an 8910 reading; every audio claim stays provisional. The tone is audible through the operator WAV only, which is not evidence.

---

## S1-4 Guest state, RAM markers and the progress predicate

**RAM map (system RAM is 16-bit words at `0200..035F`).** BACKTAB `0200..02EF`; program variables follow, inside the first clear loop, so they start at zero without extra code:

| Address | Name | Meaning |
|---------|------|---------|
| `02F0` | init marker | `03C5` written last in init (the project's end-marker value, as in the diagnostics; here it means "init finished") |
| `02F1` | frame counter | incremented once per interrupt-routine run; with a correct re-arm it equals the number of frames since frame 1 |
| `02F2` | port 0 last byte | initialised to `FF` (idle) |
| `02F3` | port 0 change counter | counts bytes that differ from the previous frame's byte, presses and releases alike |
| `02F4` | port 1 last byte | initialised to `FF` |
| `02F5` | port 1 change counter | |
| `0340..` | stack | R6 starts at `0340` (contract 1); one word is used and discarded per interrupt |

Variables at `02F0..02F5` sit past BACKTAB end `02EF`; a word stored inside `0200..02EF` would be drawn (as in the Phase 6 diagnostic). The clear loop covers `0200..02FF`, so the marker is stored after it (footgun).

**Predicate [REC]: three layers, asserted at three or more `run-to` steps.**
1. *Liveness/sustained:* `mem 02F0 == 03C5`, `halted == no`, and `mem 02F1 == <frames>` at a mid-frame tick (the exact count follows from the start ticks `S_k = 59,736 k` and the first interrupt at frame 1).
2. *Cumulative input effect:* `mem 02F3` and `mem 02F5` equal the number of scripted changes seen so far on each port (6 and 3 at the end, 1 and 0 after the first event); the last bytes `02F2` and `02F4` equal the literal-derived bytes.
3. *Rendered state:* a few BACKTAB words (`0256..025D`, `0292..0299`), e.g. `0256 == 0802` after KEY_1.
Excluded on purpose (as in 06-13): registers, R7, flags, the stack word, machine time and the exact ISR duration (C40, C45, C56, C68).

**Why a counter, not a flag.** A press count that moves by exactly the number of changes cannot be satisfied by "the loop ran" or a frame hash; the frame counter proves the display is sustained (the Phase 6 diagnostic shows one frame).

**Options.** (a) The above (**recommended**); (b) a stage byte reaching a goal after a key sequence (S1-1 option C): stronger, more code; (c) a magic word set by any press: a flag, weaker.

**Reversal evidence.** A reviewer shows a mutant that satisfies the predicate but is wrong for a reason the video and log do not catch: add the missing marker. The spike found none among 11 mutants (S1-10).

**Risks.** `02F1` depends on the first interrupt being at frame 1 and one run per frame; it moves if S-2 changes the startup, so the derivation states the rule, not a constant. Do not assert it within about 5,000 ticks after a frame start (the interrupt routine may not have finished its increment); the run-to bound sits mid-frame.

---

## S1-5 Program shape, size, branch budget and assembly

**Structure (spike, 132 words) [OBS].**

| Block | Words | What |
|-------|-------|------|
| init | 48 | GRAM card (12: SDBD pointer 4, two immediates 4, 4-word loop), clear `0200..02FF` (9), idle bytes `FF` into `02F2`, `02F4`, `01F0`, `01F1` (10), mixer and four zero stores (13), marker (4) |
| wait/re-arm | 8 | delay loop (N = 500), `EIS`, `B self` |
| interrupt routine | 36 | `DIS`, strobe, mode write, frame counter (5), two parameter sets of four `MVII` plus `JSR` (22), `DECR R6`, `J wait` (the saved return address is discarded; main holds no state) |
| port subroutine `port` | 40 | read and mask the byte (7 with the compare and branch); on change store it, bump the counter, write fine period and amplitude (17); always render 8 lamps with `COMR`, `SWAP`, an 8-step `SLLC`/`MOVR`/`ADCR`/`MVO@` loop, `JR R5` (16) |
| total | 132 | plus 2 to 4 words if S-2 places the interrupt entry at `5004` (a `B init` at `5000` and padding) |

**Counts against the 06-13 crossing rule.** Words 132 to 136 (< 170, not fired). Branch or jump instructions 10 (> 8, **fired by its letter**: 7 relative plus `J` plus 2 `JSR`; the listing checker annotates jumps with `-> ADDR` and counted the data-table `J` of Phase 6 the same way). Hand-computed displacement words: 7. Labels: 11. Data words: 0. Second author: no. Escape found only by a run: **yes, one behavioural one** (the vector re-entry, S1-11 a), though not an encoding slip. [INF] Together with "fourth program" and "labels", D4's trigger fires on three counts and the pre-committed rule on one; this note takes no position on the tooling choice (S-3), only on the facts.

**What the program asks of an assembler or checker.** Labels and relative branch displacements (7), absolute `J`/`JSR` targets (3), explicit `SDBD` for two 16-bit immediates (the GRAM pointer and the lamp base word). **No data directive** (the Phase 6 hand-off): the card is written by a counted loop of an immediate, not read from a table; this is a deliberate design choice that makes S-1 independent of that feature. If S-3 builds `DATA` anyway, S-1 does not need it; Phase 8 programs probably will.

**Options for the loop structure.**
- **A. Subroutine called twice with the parameters in registers (recommended):** 40 words once plus 22 for the two call sites; one copy to derive and review.
- **B. Duplicate the body per port:** about 70 words more, 3 more branches, two derivations of the same logic.
- **C. A parameter table in ROM:** fewer instructions, but needs a data directive and a second addressing convention.
- **Return mechanism:** `DECR R6; J wait` (discard the saved PC) rather than `PULR R7` plus a re-armed spin; it lets the main loop own the whole re-arm delay and removes any ISR-internal padding (S1-11 b).

**Polling alternative (fallback) [OBS].** Without interrupts, a main loop of "strobe, port 0, port 1" repeated forever keeps the display on (114 words in the spike; the loop is shorter than the 2,000-cycle register window, so one strobe per frame lands) and renders the same lamps and the same PSG sequence, with the stub contract unchanged and no goldens touched. It loses the frame counter (the loop count has no frame meaning), the interrupt path coverage (VID-02 behaviour is exercised only by stand-ins today), and the property that the displayed frame equals the sample taken at the interrupt (a lamp write may race a row latch if an event lands mid-frame). [REC] interrupt-driven; keep polling as the planned fallback if S-2 judges the stub-contract change too costly.

**Reversal evidence.** The planner derives the program and finds more than 160 words or a second escape found only by a run: drop to one port (about 85 words, 5 branches) before adding tooling pressure. S-3's mini-assembler decision is not driven by this program's size alone.

**Risks.** Hand-derivation effort: three diagnostics took 133 words and about 3 hours; this program is similar in size with more control flow. The 06-13 data (zero checker catches, two hand slips, zero escapes) suggests the encoding risk is small and the semantic risk (S1-11) is the real one.

---

## S1-6 How input visibly changes video and audio

**Video [REC].**
- *Region.* Port 0 lamps: plane positions `x` 48..111, `y` 32..39 (8 lamps of 8 x 8, lamp i at `x` 48 + 8i). Port 1 lamps: `y` 56..63. Each lamp block is a single color index: **1 unlit, 2 lit**. Everything outside the two rectangles is index 0 in every displayed frame [OBS: the set of distinct indices in frames 1..12 is {0, 1} idle and {0, 1, 2} with input].
- *Frame 0* is a display-off frame (the program's first interrupt, hence first strobe, is in frame 1) [OBS]: assert `frame 0 displayed == no`.
- *Probe pixels* (inside a lamp block, never on an edge): port 0 lamp i at `(51 + 8i, 35)`; port 1 lamp i at `(51 + 8i, 59)`; background probes `(0,0)`, `(158,0)`, `(50,45)`, `(50,70)`, `(158,95)`.
- *Expected frames.* Because the plane has only three index values and two rectangles, the idle frame and each response frame are hand-derivable as `.frame` literals (a delta of the idle frame by the lit lamp blocks), with `sha256-text` keys as in Phase 6; no blank (`FF`) position occurs, so frame hashes are assertable.
- *Settling rule.* The displayed frame k is the input sampled at the start of frame k: an event in frame f is first shown in frame f+1 [OBS: events at 40,000 to 43,000 ticks into a frame, all BACKTAB writes done by cycle 1,300 against the first row latch at 3,848]. The S-4 settle rule (assert from frame f+2 on) is conservative and holds.

**Expected lamp patterns [OBS: spike equals the independent model], port 0 / port 1 (digits are the color index of lamps 0..7):**

| Frames | Input in force | Port 0 byte | Lamps port 0 | Port 1 byte | Lamps port 1 |
|--------|----------------|-------------|--------------|-------------|--------------|
| 0 | none (display off) | (not drawn) | | | |
| 1 | idle | FF | 11111111 | FF | 11111111 |
| 2, 3 | KEY_1 | 7E | 21111112 | FF | 11111111 |
| 4, 5 | KEY_1+KEY_9 | 5A | 21211212 | FF | 11111111 |
| 6, 7 | port 0 idle, DISC_E on port 1 | FF | 11111111 | FD | 11111121 |
| 8, 9 | SIDE_TOP, DISC_E | 5F | 21211111 | FD | 11111121 |
| 10, 11 | all idle | FF | 11111111 | FF | 11111111 |
| 12 | KEY_1+KEY_9, SIDE_LOWER_LEFT | 5A | 21211212 | 9F | 12211111 |

**Audio [REC and open point].**
- *Exact and derivable, asserted:* the PSG write log (count and per-record `addr`, `value`, `source=cpu-write`, never `tick=`): 7 init records then 2 per change: `(0, FF) (1, FF) (8, 3C) (4, 0) (5, 0) (11, 0) (12, 0)`, then `(0,7E) (11,0F)`, `(0,5A) (11,0F)`, `(0,FF) (11,0) (1,FD) (12,0F)`, `(0,5F) (11,0F)`, `(0,FF) (11,0) (1,FF) (12,0)`, `(0,5A) (11,0F) (1,9F) (12,0F)` [OBS; the values come from the literal by `byte` and the program rule, not from the run]. Silence: `audio at <tick> A=0 B=0 C=0` at any tick before the first amplitude write (the first change is applied by the interrupt routine of frame 2, after tick 119,472) and after a full release; `output.audio.changes == 0` for the no-input run [OBS].
- *Not derivable by hand without cycle accounting:* the **phase** of a toggling tone, i.e. the exact tick of the first edge after an amplitude write [OBS: first edge 2,184 ticks after the amplitude write in one case, grid not aligned to a closed form]. A held single voice does have a phase-free property: consecutive level changes are exactly `16 x byte` ticks apart [OBS], which can be asserted by a human or a script reading `--audio-raw` but not by the current grammar.
- *Options.* (a) S-4 adds the small assert `audio changes ==|!= <n>` (used only with 0: idle `== 0`, active `!= 0`), about 25 lines, and the program stays as is (**recommended**). (b) S-1 adds a **constant-level gate** on channel C (tone and noise already disabled by the mixer, so a written amplitude is a constant level, PSG-R15): about 16 more words, 1 more branch, 2 more log records per change, and exact `audio at <t> A=? B=? C=15` asserts with no grammar change. (c) Freeze the grammar and accept silence plus the log as the audio evidence, with "samples change" shown by the differential judge (idle versus active digest and `audio.changes` differ).
- [REC] (a) with (c) as the floor: it keeps the program a believable program rather than a test-bait one; the gate (b) is the fallback if S-4 refuses any grammar change and (c) is judged too weak against criterion 3 ("PSG write log and samples change").

**Reversal evidence.** S-4 shows a phase-free exact sample assert in the current grammar: then (a) is unnecessary. S-4 proves a non-zero derived change count is needed: then (b) is the only exact route.

**Risks.** Two voices active together interleave in the delivered change list (per-channel reading needed for any spacing check). The tone formula is an 8910 inference (C69). A listening judgement of the WAV is subjective and not evidence.

---

## S1-7 The scripted input timeline

**Sampling semantics [OBS].** The guest samples each port once per frame, at the start of the interrupt routine. An event is visible to every read that begins at or after its tick (C67), so an event lands in the **next** frame's sample, and two events inside one frame collapse to the last (a press shorter than one frame is invisible to this program; a deterministic property, usable as an optional extra step). Frame start `S_k = 59,736 k`; the guest's reads for frame k occur between `S_k` and about `S_k + 3,200` ticks.

**Events [REC].** Each states both controller words; ticks in master ticks (S-4's margins: at least 10,000 ticks from any frame end, at least 600 from the next event):

| # | `at_tick` | `c0` | `c1` | Falls in frame | First sampled by | Offset into its frame |
|---|-----------|------|------|----------------|------------------|-----------------------|
| 1 | 100,000 | KEY_1 | 0x0 | 1 | frame 2 | 40,264 |
| 2 | 220,000 | KEY_1+KEY_9 | 0x0 | 3 | frame 4 | 40,792 |
| 3 | 340,000 | 0x0 | DISC_E | 5 | frame 6 | 41,320 |
| 4 | 460,000 | SIDE_TOP | DISC_E | 7 | frame 8 | 41,848 |
| 5 | 580,000 | 0x0 | 0x0 | 9 | frame 10 | 42,376 |
| 6 | 700,000 | KEY_1+KEY_9 | SIDE_LOWER_LEFT | 11 | frame 12 | 42,904 |

Each event is 16,800 to 19,500 ticks (about 4,200 to 4,900 cycles) before the next frame start and more than 40,000 ticks after the previous frame start, so the interrupt routine of its own frame has long finished and the next one has not begun.

**Run bound.** `run-to 806568 ticks` (`S_13 + 30,000`, mid-frame 13); `capture video 0 13` (13 frames, cap 16); `capture psglog` (25 records of a cap of 4,096); `capture audio` only if audio asserts need it (375 level changes, under the cap). At the stop [OBS]: `halted == no`; `02F0 = 03C5`, `02F1 = 000D`, `02F2 = 005A`, `02F3 = 0006`, `02F4 = 009F`, `02F5 = 0003`; PSG log count 25; 13 delivered frames. Intermediate steps (S-4's multi-step rule) at ticks 200,000 / 440,000 / 640,000 (20,000 to 43,000 ticks into frames 3, 7, 10) give `02F1` = 3, 7, 10, change counters (1,0), (3,1), (5,2) and last bytes `7E/FF`, `FF/FD`, `FF/FF` [OBS, spike probe, equal to the independent model].

**Variants [REC].** `demo-lamp-tone-idle`: no `input` line, same bound (`02F1 = 000D`, counters 0, 7 log records, every lamp 1, silent). `demo-lamp-tone-shift`: all event ticks +2,000 (frames byte-identical [OBS]). `demo-lamp-tone-swap`: each event's `c0` and `c1` words exchanged: the lit rows, the log addresses (`1`/`12` for the first event) and the counters swap [OBS for the mutant PORT-SWAP; the expectation is derivable]. Optional blink event: `c0=KEY_2` at one tick and `c0=0x0` 5,000 ticks later inside one frame: the guest never sees it (counter unchanged).

**Never-halting versus a quit input.** [REC] never halt. S-4 prefers a final input that halts the guest (`stop halted` as "ran the script to its end"). Costs: about 8 words and 1 branch, and every frame after the `HLT` is blank (no strobe), so the final tableau frame is lost and any frame assert after the quit is meaningless; the exported last frame would be blank. The sustained design is better served by `assert halted == no` at every step plus the cumulative counters, and the quit is a one-event addition if S-4 insists (reversal: S-4 shows the end-of-script marker cannot otherwise be made strong).

**Reversal evidence.** S-2 moves the first interrupt or the ISR duration by more than a few hundred cycles: the margins hold (a 16,000-tick margin is about 4,000 cycles). S-4 raises the capture need above 16 frames: shorten the spacing to one frame (events at `59,736 j + 40,000`), still distinct samples.

**Risks.** The run bound must stay mid-frame, at least 10,000 ticks from `S_k` and from the ISR's end; a bound a few thousand ticks after a frame start already includes that frame's counter increment while the lamp writes and PSG writes are still in progress [OBS: at 3,176 ticks after `S_12` the counter read 12 and the port 1 work was not necessarily finished]; keep the bound mid-frame so that counters, lamps and log are all complete.

---

## S1-8 What is deliberately not done

| Item | Why not |
|------|---------|
| MOBs, collision, delay registers, border, Color Stack mode | VID-05 is Phase 8; the lamp blocks need none; no single-feature fixture claim is made here |
| Text, letters, a title, a font, any GROM read | Rights surface (the S-5 name and glyph rules), and the zero GROM would draw nothing anyway |
| A tune, a jingle, a sound effect from any title | S-5 rule: a melody is a copyrightable work independent of code |
| Noise, envelope, channel C | Not needed for criterion 3; each adds PSG policy defaults (C12, C73) |
| Decoding keys, discs, side buttons | Hides the electrical model; INP-02 ambiguities would be flattened |
| A game loop, score, win state | Option D of S1-1: past the hand-assembly line; needs MOBs |
| A quit or halt input | S1-7: blanks the display after the stop; an optional later addition |
| An EXEC call, an EXEC-compatible cartridge header, a real-EXEC run | The program is EXEC-free by design (D5); whether a real EXEC accepts it is unknown and out of Phase 7 (D11 is the owner-local route, S-5) |
| A host frontend, window, audio device | v2 (TOOL-04); the operator `--frame-ppm` and `--audio-wav` exports exist and are not evidence |

---

## S1-9 Rights and provenance

**Facts [OBS].** The program, its derivation document and its listing are authored in this repository by the project (MIT, D9, `LICENSE`); a boot stub and a zero GROM are the only other bytes in the run, both project-made. The card is one block of eight bytes `FF`; the two color numbers (1 and 2) are not art; the controller table is a transcription the project already holds (tier 4, `data/input/`); no emulator source, test ROM or catalog table was used for any program word or any expected value (the encodings are derived from the manual pages the diagnostics already cite and from `data/cp1610/opcode-families.json`).

**Recommendation [REC].** Manifest `FX-demo-lamp-tone-1`: kind `original-source`, `committed: true` for the listing only, licence `MIT` with the LICENSE digest evidence, `rights_review: author-attested`, `redistributable: yes`, `allowed_use` `local-test`, `ci`, `repo` (and `release` per S-5), recipe naming `tools/pack` (`--ivsc`), source digest and the build-tree image identity pinned by the golden; no `.ivsc` committed. Header comment of the listing records: no EXEC call, no GROM read, no text, no recalled pattern, and the derivation order (derivation committed before the listing). The tune and glyph rules of S-5 hold trivially.

**Reversal evidence.** None expected; an owner-chosen visual identity or title would reopen the name gate (S-5).

**Risks.** The listing must not be edited from a recalled third-party program; review checklist line (S-5).

---

## S1-10 Program-level mutants and the three-channel argument (hand-off to S-4)

The spike patched single words (via `--patch`) of the built image and ran the timeline to a mid-frame bound; observed channels in brackets [OBS]. These are candidate inventory rows for S-4, not its inventory.

| Mutant (word patched) | Guest state | Video | PSG log | Caught only by |
|-----------------------|-------------|-------|---------|----------------|
| PORT-SWAP (the two port addresses exchanged) | counters swap (2/5) | rows swap | addresses 1/12 for 0/11 | all three |
| LAMP-NOCOMR (the complement removed) | same | lamps inverted | same | video |
| STROBE-ADDR (`$0020` to `$0022`) | same | every frame blank | same | video |
| SEL-BIT-CLEARED (GRAM select bit off) | same | lamps invisible (index uniform) | same | video |
| FRAMECOUNT-NOP (increment removed) | `02F1` stays 0 | same | same | guest state |
| WAIT-SHORT (N = 100) | `02F1` doubles (two runs per frame) | same | same | guest state |
| WAIT-LONG (N = 900) | `02F1` falls behind | frames lost (blank later frames) | same | guest state and video |
| NEVER-CHANGED (compare always equal) | counters 0 | same | 7 records | guest state and log |
| ALWAYS-CHANGED (skip branch removed) | counters = frame count | same | 47 records | guest state and log |
| AMP-INVERT (amplitudes exchanged) | same | same | amp values swapped | log (and samples) |
| FINE-NOT-WRITTEN (period store removed) | same | same | 14 records, not 21 | log (and samples) |

Plus the structural mutant GRAM-LATE (the GRAM write after the BACKTAB clear loop): [OBS] the guest-state predicate and the log all still pass, and every frame is uniformly index 0, so **only the video assertion catches it**; likewise STROBE-ADDR. This is the strongest argument for asserting all three channels from one run: video-only, state-only and log-only mutants all exist (seven of the eleven, plus GRAM-LATE, are visible to exactly one channel).

**Reversal evidence.** A mutant that none of the three channels catches: add an assert; the spike found none.

---

## S1-11 Findings for the other decision points [OBS unless marked]

a. **The interrupt vector must start with a disable instruction.** A `J` at the vector, followed by a cartridge routine whose first instruction is `DIS`, re-enters about every 21 cycles under the project default C41 = 0 (the `J` is interruptible, the interrupt level is held for 3,791 cycles). R6 climbed by about one word per 21 cycles, past the end of system RAM (the core does not fault) and the scenario still ended `pass` (S-2 measured that the display never turns on in that variant). S-2's `JD $5004` (jump and disable in one instruction) or `DIS` followed by `J` at the vector both fix it; only the vector's first instruction matters. This was found only by a run (the crossing rule's second item in the 06-13 wording, though not an encoding error).
b. **Where to re-arm.** With `EIS` in the main loop after a delay loop of N iterations (15 cycles each): N 200..800 gave exactly one ISR per frame; N <= 170 two per frame; N >= 850 skipped frames. The usable band is about 10,000 cycles wide [INF: the re-arm lands at roughly cycle 4,500 for N = 200 and 14,500 for N = 800, from the delay count plus the stalled bus windows]. Padding inside the ISR (S-2's current draft) has a margin of 170 to 270 cycles; moving the pad to the main loop and discarding the saved return address (`DECR R6; J wait`) removes that sensitivity at no word cost. This also hardens the program against a different C41 or INTRM width value.
c. **Window order in init.** The graphics-RAM write must come first in init: the write window is the first 3,780 cycles of frame 0 and a long BACKTAB clear (about 6,100 cycles) before it makes the GRAM write drop silently. Likewise the strobe and the mode-select write belong in the per-frame interrupt routine (register window, 2,000 cycles).
d. **The display of frame k equals the sample at the start of frame k** because all lamp writes precede the first row latch (cycle 3,848) by more than 2,500 cycles; a polling main loop would not have this property.
e. **Frame 0 is never displayed** and frames up to the first interrupt are blank; the frame counter at a mid-frame bound equals the frame index.
f. **The `hash-needs-colors-only` rule** is satisfied by every frame of this program (no blank position), so frame hashes are assertable.

---

## Assumptions from other decision points

- **S-2 (runtime).** Interrupt-driven with a contract-2 boot stub whose vector begins with a disable (`JD $5004` or `DIS; J`), cartridge entry `$5000` (`B init`), interrupt entry `$5004`, R6 = `0340`, stub forwards every interrupt to the cartridge (EXEC-free). The program tolerates either re-arm design (S1-11 b); if S-2 keeps contract 1 and chooses polling, S-1 falls back to the 114-word variant and loses the frame counter and the ISR evidence (S1-5). If the stub is a second image (S-2's S2-2), the program is unaffected; if contract 1 is edited in place, six `.digests` goldens and the consumer golden move as one classified diff.
- **S-3 (assembler).** Needs labels and relative displacement, no `DATA`. If S-3 builds `ivs_asm` the program is written in its dialect (explicit `SDBD`, `JSR`, `JR`, labels); the spike confirmed the needed mnemonic subset is about 30 forms. If S-3 keeps hand assembly, 7 displacement words and 3 absolute targets are the hand-derived cost.
- **S-4 (verification).** Grammar as it stands covers every assert in this note except an exact phase-free audio statement (S1-6). Frame literals F-S1 (idle) and F-S2 (one response), plus the other distinct lamp patterns, follow S-4's convention; the settle rule and the 10,000-tick margins hold.
- **S-5 (rights, SCN-02).** Naming pattern `FX-demo-<slug>-1`, slug `lamp-tone` from this note; manifest as in S1-9; the owner-local EXEC/GROM route does not involve this program, and no real-EXEC result is claimed.

## Open items for planning

1. **Cross-point conflict (S-1 versus S-4) on audio exactness.** S-4's "constant-level-first design" or its optional `audio changes` assert: this note recommends the assert (option D of S-4, `== 0` idle, `!= 0` active) and keeps the gate (S1-6 b) as the fallback. The orchestrator should pick one before planning.
2. **Never-halting versus a quit input** (S1-7): this note says never halt, S-4 prefers a halting end marker; decide and, if a quit is added, budget 8 words and 1 branch and accept the blank tail.
3. **Re-arm design** (S1-11 b) is S-2's; confirm main-loop re-arm with a discarded return address, or keep padding inside the ISR with the tighter margin.
4. **Cartridge layout** under S-2's `$5004` entry: `5000: B init`, two spare words, ISR at `5004`; confirm the filler words satisfy the listing checker's row rules.
5. **Derivation document** before the listing (as in Phases 5 and 6): per event the RAM words, the BACKTAB words, the PSG writes in order, and the settle frame; the independent model of this note (about 40 lines) is a ready cross-check and is not committed here.
6. **Blind re-derivation** (S-4 protocol): give a fresh agent only the specification and the controller literal, ask for the lamp patterns, counters and PSG sequence; two items worth an explicit second read: (a) that `SLLC` sets the carry from bit 15 and `ADCR` adds it (the lamp builder), (b) that a lit lamp is a cleared bit (the active-low reading).
7. **Mutant inventory** from S1-10 into the S-4 inventory (names and clauses); the GRAM-LATE structural mutant needs a patched listing, not a one-word patch.
8. **Listening handoff.** The program yields the first listenable tone of the project via the operator `--audio-wav` (Phase 5 D-22 was deferred for lack of one). It is a subjective, non-evidence item for the owner and should be recorded as handed off, not as a result.
9. **Cost.** One more scenario of about 0.03 s user time (dev runner) per variant; four variants and the installed-consumer replay are negligible against the gate (S-4 owns the budget row).
