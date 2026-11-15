# Hard points of the snapshot suite

This note defines the 18 named cuts of `data/expected/state-cuts.ids` (STA-02; plan 08-09, D-17): the predicate over the instance after a unit that makes a cut a cut, the program that reaches it, what is non-default there and which rows of `data/expected/state-checklist.ids` it witnesses.
It also holds the five stand-in programs P-SDBD, P-HLT, P-FAULT, P-LATCH and P-PSG with their decles. No program was executed to write it: every decle is derived from the printed encodings of `data/cp1610/opcode-families.json` and agrees with the vectors of `data/cp1610/vectors`; the project assembler `ivs_asm`, built from `tools/asm/asm.c` into a scratch directory outside the tree, was used as a second encoding of the same instructions and produced the same decles word for word, and nothing was read back from the core.
Cycle and tick arithmetic uses the printed instruction cycles, the unit conventions of the stepper and the profile fields named below; it is an expectation, not a measurement, and the scan of plan 08-14 pins the real tick.
Claims about these cuts are limited to the deterministic repeatability of one build and one profile. [nc]

## How a cut is found and pinned (H17)

A cut is a unit boundary: the instance state after a unit, which is also the only place the run call can stop.
Plan 08-14 finds each cut by a witness scan: it steps an instance one unit at a time through the internal step seam (or its trace form, when the predicate needs the unit kind or the events of the last unit) and records the first unit boundary at or after the occurrence rule of the cut where the predicate holds.
The decimal machine tick is written into `data/expected/state-cuts.ids` in place of the token `pending-08-14` and into the as-built section that plan 08-14 appends to this file.
A witness test then re-proves each pinned tick: it steps a fresh instance to the pinned tick through the public run call and requires the run to end exactly on a boundary at which the predicate holds, so a drifting literal fails there and never silently.
A cut whose predicate never holds in its program is a finding of that plan: it fixes the program or the predicate in an appended note and reports it.
The ticks quoted below are arithmetic expectations that make the occurrence rule concrete.

## Facts of the profile that the arithmetic uses

All values are fields of `data/profiles/intv-2609-ntsc-family-p1.json` or constants of the stepper, read from the files and not measured.
- One CPU cycle is 4 master ticks; the frame is 59,736 ticks, which is 14,934 CPU cycles; the frame phase is 0, so frame 0 starts at tick 0 and frame k starts at 59,736 k.
- Units: a reset entry is 5 cycles, an interrupt entry 9, a halted step and a grant hold 1; an instruction costs its printed cycles (MVII 8, MVO 11, MVI direct 10, MVO@ 9, XORI 8, DECR and INCR 6, ADDR 6, SDBD 4, HLT 4, a taken branch 9, an untaken branch 7), and the wait count of the memory system is 0.
- The register window is the first 2,000 cycles of a frame; the graphics window closes at cycle 3,780; the interrupt level INTRM is low for the first 3,791 cycles.
- There are 13 bus windows per frame; the first starts at cycle 3,791, the pitch is 912 cycles and the hold 114 cycles; the fetch of card row i is at cycle 3,848 + 912 i (the fetch offset is 57).
- The frame timeline has 16 events in this order: 0 FRAME_START, 1 REG_CLOSE, 2 GMEM_CLOSE, 3 to 14 ROW_FETCH of rows 0 to 11, 15 FRAME_END. `stic.event_index` is the index of the next event to apply, so after ROW_FETCH of row 5 (index 8) it is 9.
- The reset entry vector is `$1000` and the interrupt vector `$1004`. A program word k is loaded at `$1000` plus k, the stand-in memory is filled with zero words, and the stack pointer is whatever the program sets (the stand-in programs below use none).
- Acceptance of an interrupt does not clear the interrupt enable in this profile (a pinned default, C41).

## The scenario executions, by id

- lamp-idle: the scenario `demo-lamp-tone-idle`, the image `lamp-tone.ivsc` entered through the boot stub of contract 2 with the zero graphics ROM and no input events. It enables its display from frame 1 on (its first EIS comes after the window decision of frame 0), serves one interrupt per frame and writes its PSG registers once at the start.
- lamp-pass: the scenario `demo-lamp-tone-pass`, the same program with six input events (ticks 100,000, 220,000, 340,000, 460,000, 580,000 and 700,000).
- diag-stic-2-collision: the scenario `diag-stic2-collision-pass`, the image `diag-stic-2.ivsc`, controller word `BD` at tick 0.
Nothing else about these programs is restated here.

## The 18 cuts

Each cut lists its program, its family, its predicate (the state after the unit), the occurrence rule, what is non-default there, and the rows of the checklist that name it as their witness.

### HP-SDBD (P-SDBD, cpu-points)
- Predicate: `cpu.dbd == 1` and `cpu.run_state == RUNNING`: the SDBD unit has just run and the next unit is the extended instruction, which reads two decles.
- Occurrence: the first such boundary. Arithmetic: reset entry 5 + MVII 8 + MVO 11 + SDBD 4 = 28 cycles, tick 112.
- Non-default: `cpu.dbd` is 1, R7 is `$1005`, R1 is `$00AB`, the scratchpad word `$0100` holds `$00AB`.
- Witnesses: `cpu.dbd`, `mem.scratchpad`.

### HP-STEAL-A (lamp-idle, cpu-points)
- Predicate: `cpu.busak == 1` after a unit of kind BUS_GRANT whose predecessor was not a BUS_GRANT (the first grant unit of a window).
- Occurrence: the first such unit in a displayed frame (frame at least 1). Frame 1 starts at tick 59,736 and its first window at cycle 3,791 (tick 15,164 into the frame), so the first grant unit starts at tick 74,900 plus the line sampling lag and at most one unit.
- Non-default: `cpu.busak` is 1.
- Witness: `cpu.busak`.

### HP-STEAL-B (lamp-idle, cpu-points)
- Predicate: `cpu.busak == 1` after a BUS_GRANT unit whose predecessor was also a BUS_GRANT (mid-hold: hold units are one cycle) and `stic.grants_in_frame >= 1`.
- Occurrence: the first such unit after HP-STEAL-A; a frame after a frame with an ungranted window is preferred when one exists (see the open point of `stic.late_grants` in the checklist).
- Non-default: a grant held across the cut with the frame's grant count at least 1; `stic.late_grants` only if an earlier frame left a window ungranted.
- Witness: `stic.late_grants`.

### HP-STEAL-C (lamp-idle, cpu-points)
- Predicate: the unit just run carried the event BUS_RELEASED, `cpu.busak == 0`, `cpu.prev_interruptible == 1` (a grant never changes it) and `stic.grants_in_frame >= 1`.
- Occurrence: the first such boundary after HP-STEAL-B: the release unit, 2 cycles after the BUSRQ level falls (the first window ends 114 cycles after it starts).
- Non-default: the grant count of the frame in progress; the next unit tests the lines and does not fetch (the policy of the bus grant, A-02).
- Witness: `stic.grants_in_frame`.

### HP-OPEN (lamp-idle, cpu-points)
- Predicate: `cpu.prev_interruptible == 1`, `cpu.intff == 1`, `cpu.run_state == RUNNING`, and the unit stepped next from this state (on a copy of the instance, through the same seam) is an interrupt entry: the boundary just before the entry.
- Occurrence: the first such boundary. The first EIS of the program is at frame-0 cycle 6,564 to 6,631, so the first interrupt that is accepted is that of frame 1 (tick 59,736 plus the sampling lag and at most one idle branch of 9 cycles).
- Non-default: `cpu.prev_interruptible` is 1 (it is 0 at power-on) and the interrupt falls due at an open boundary. A restore that loses it takes the interrupt one unit late and shifts everything the routine writes.
- Witness: `cpu.prev_interruptible`.

### HP-CLOSED (lamp-idle, cpu-points)
- Predicate: `cpu.prev_interruptible == 0`, the unit just run is an instruction (not an entry) and the INTRM level sampled for the next unit is low: a due interrupt that a closed boundary forgets, because nothing is latched.
- Occurrence: the first such boundary after the interrupt entry of a displayed frame: the first instruction of the interrupt routine is the display strobe `MVO R0, $0020`, which is not interruptible, and the level is low for the first 3,791 cycles of the frame.
- Non-default: `stic.display_latch == 1` with `stic.windows_active == 0` (the strobe came inside the register window and the decision at the close of the graphics window is still ahead).
- Witness: `stic.display_latch`. A restore that loses the latch hides the whole frame: no windows, no grants, no picture.

### HP-INT-ENTRY (lamp-idle, cpu-points)
- Predicate: the unit just run is of kind INT_ENTRY; `cpu.r[6]` equals the stack pointer before the entry plus one, `cpu.r[7]` equals `$1004`, `cpu.prev_interruptible == 0`.
- Occurrence: the first interrupt entry of the run, the unit that follows the boundary of HP-OPEN (9 cycles).
- Non-default: R6 and R7 and the stack word that holds the return address in system RAM.
- Witness: `cpu.r`. A restore that replays or skips the entry shows in R7 at once.

### HP-HLT (P-HLT, cpu-points)
- Predicate: `cpu.halt_pending == 1` and `cpu.run_state == RUNNING`: the HLT unit has just run; the next instruction still executes, because HLT is not interruptible.
- Occurrence: the first such boundary. Arithmetic: reset entry 5 + MVII 8 + three iterations of the loop (MVO 11 + DECR 6, then a taken branch 9 twice and an untaken branch 7 once) + HLT 4 = 93 cycles, tick 372.
- Non-default: the stop latch; the flags are Z set and the rest clear (the last DECR left R1 at 0); R7 points at the BZE that reads Z.
- Witnesses: `cpu.halt_pending`, `cpu.status`.

### HP-HALTED (P-HLT, cpu-points)
- Predicate: `cpu.run_state == HALTED` and the BUSRQ level sampled for the next unit is high: a grant window is due while the CPU is halted, so the next unit is a grant.
- Occurrence: the first such boundary. The BZE after the HLT takes 9 cycles (102 cycles) and the stop is the next 1-cycle halted unit (103 cycles, tick 412); the display strobes of the loop came in the register window of frame 0, so the first window of the frame starts at cycle 3,791 (tick 15,164) and the cut follows it by the sampling lag.
- Non-default: `cpu.run_state` is HALTED with R7 parked after the BZE; a grant is due.
- Witness: `cpu.run_state`.

### HP-FAULTED (P-FAULT, cpu-points)
- Predicate: `cpu.run_state == FAULTED` and `cpu.fault_reason != 0`.
- Occurrence: the first such boundary. Arithmetic: reset entry 5 + MVII 8 + eight DECR 48 + seven taken branches 63 + one untaken branch 7 + the fault unit 4 (the fetch of the refused word) = 135 cycles, tick 540.
- Non-default: the sticky fault; every later run call stops with a guest fault at once.
- Witness: `cpu.fault_reason`.

### HP-RESETPEND (none, cpu-points)
- Predicate: `cpu.run_state == RESET_PENDING` and `time_ticks == 0`: right after the power-on reset, before any unit.
- Occurrence: tick 0. The continuation is the lamp-idle run: the images of that scenario are loaded and the first unit is the reset entry.
- Non-default: the memory-system gate flags are open (index 0 always 1, index 3 always 0), the PSG envelope is held, the chains hold their start values.
- Witness: `mem.gate_open`: with every gate closed the first register write of frame 0 is blocked and the run diverges.

### HP-FRAME-MID (lamp-idle, stic-points)
- Predicate: `stic.frame_index >= 1`, `stic.display_latch == 1`, `stic.windows_active == 1`, `stic.event_index` in 9 to 15 (FRAME_START, REG_CLOSE, GMEM_CLOSE and the fetches of rows 0 to 5 are applied, FRAME_END is not) and `cpu.intff == 1` (the idle loop after the re-arm, armed for the next frame).
- Occurrence: the first such boundary. The fetch of row 5 is at cycle 8,408; the re-arm EIS is at cycle 9,186 to 9,597; so the first boundary in frame 1 is after about cycle 9,600, tick about 98,100, and before FRAME_END at cycle 14,934.
- Non-default: the cursor, the latch, six latched rows, closed gates, machine time of at least one frame, a video chain with history, the interrupt enable.
- Witnesses: `phase`, `time_ticks`, `cpu.intff`, `stic.frame_index`, `stic.event_index`, `stic.windows_active`, `obs.video.chain`, `obs.video.count`.

### HP-LATCH (P-LATCH, stic-points)
- Predicate: `stic.frame_index >= 1`, `stic.mode` is Color Stack, `stic.cs_ptr != 0`, `stic.display_latch == 1`, `stic.windows_active == 1`, `stic.event_index >= 4` (the fetch of row 0 is applied) and `stic.row_latch[0][0] != mem.sysram[0]`: the BACKTAB word that the guest holds now differs from the copy that the fetch latched.
- Occurrence: the first such boundary after the fetch of row 0 (cycle 3,848). The loop of P-LATCH is 70 cycles long and rewrites the word each pass, so the current word differs from the latch within two iterations of the fetch: tick about 75,400 to 75,700 in frame 1.
- Non-default: the latch holds the value stored before the fetch and BACKTAB holds the other value of the toggle; the color-stack pointer is 1; GRAM card 0 is `$00FF`; the register accesses of the loop outside the window have raised the blocked counters.
- Witnesses: `mem.sysram`, `mem.gram`, `stic.mode`, `stic.cs_ptr`, `stic.blocked_reads`, `stic.blocked_writes`, `stic.row_latch`.

### HP-MOB (diag-stic-2-collision, stic-points)
- Predicate: `stic.frame_index >= 1`, `stic.windows_active == 1`, `stic.event_index` in 9 to 15, at least one of the words `mem.stic_reg[0x00]` to `mem.stic_reg[0x17]` (the MOB registers) is nonzero and at least one of `mem.stic_reg[0x18]` to `mem.stic_reg[0x1F]` (the collision registers) is nonzero.
- Occurrence: the first such boundary after a frame in which a collision was written at FRAME_END (the bits are ORed in at the frame end and stay until the guest clears them).
- Non-default: MOB, border and delay registers and a sticky collision bit; no new state, only words of the register file.
- Witness: `mem.stic_reg`.

### HP-PSG-MID (P-PSG, device-points)
- Predicate: both period rewrites have been written (`mem.psg_reg[0] == 0x31` and `mem.psg_reg[1] == 0x57`), `psg.noise_counter != 0`, `psg.tone_counter[0] != 0` and `psg.tone_out[0] != psg.tone_out[1]`.
- Occurrence: the first such boundary after the second rewrite. Arithmetic: 197 cycles of set-up, the first wait of 200 iterations (2,998 cycles), the two rewrites of 19 cycles, a reset entry of 5: the rewrites end near cycle 3,246, tick about 13,000, and the predicate holds within a few chip steps.
- Non-default: counters that cannot be derived from time because the periods changed twice; a noise register with history; PSG register words; a nonzero audio chain.
- Witnesses: `mem.psg_reg`, `psg.next_sample`, `psg.tone_counter`, `psg.tone_out`, `psg.noise_counter`, `psg.noise_lfsr`, `obs.audio.chain`, `obs.audio.count`, `obs.audio.emitted`, `obs.audio.prev_levels`.

### HP-ENV-MID (P-PSG, device-points)
- Predicate: `psg.env_held == 0`, `psg.env_cycles` between 1 and 14, `psg.env_level` between 1 and 14.
- Occurrence: the first such boundary after the shape write (cycle about 197); the shape repeats for ever, so the predicate recurs and the first occurrence is a few thousand ticks after the write; plan 08-14 pins it.
- Non-default: a repeating decay envelope in progress; a restore that left the envelope held would freeze it.
- Witnesses: `psg.env_counter`, `psg.env_level`, `psg.env_dir`, `psg.env_held`, `psg.env_cycles`.

### HP-INPUT (lamp-pass, device-points)
- Predicate: one input event is applied (`obs.input.count == 1`), a byte of `mem.port_pulldown` is nonzero, `obs.input.floor_tick > 0` and one event is still pending in the host array.
- Occurrence: the first boundary at or after tick 100,000 (the first event) and before tick 220,000 (the second).
- Non-default: the controller words in force, the pull-down masks derived from them, the applied count, the floor and the chain; the host resubmits from the applied count.
- Witnesses: `mem.port_pulldown`, `obs.input.words`, `obs.input.count`, `obs.input.floor_tick`, `obs.input.chain`.

### HP-LOG (lamp-pass, device-points)
- Predicate: `obs.psglog.count > 0` and the stamp of the last record lies inside the unit just run (the unit contained a PSG register write).
- Occurrence: the first such boundary: the initialisation writes the PSG registers before its first EIS at frame-0 cycle 6,564 to 6,631, so the first write is before tick 26,500.
- Non-default: a log chain with history and a record count; the next record index comes from the count.
- Witnesses: `obs.psglog.chain`, `obs.psglog.count`.

## The stand-in programs

Each program is a list of 10-bit decles (a decle is a 16-bit word of at most `03FF`) loaded at `$1000`. The encodings are the printed patterns: MVII `1010111ddd` (`02B8` plus the register), MVO `1001000sss` (`0240` plus the register, then the direct address), MVI `1010000ddd` (`0280` plus the register, then the direct address), MVO@ `1001mmmsss` (`0240` plus 8 times the pointer register plus the source register), XORI `1111111ddd` (`03F8` plus the register), ADDR `0011sssddd` (`00C0` plus 8 times the source plus the destination), CLRR as XORR `0111rrrrrr` (`01C0` plus 9 times the register), INCR `0000001rrr` (`0008` plus the register), DECR (`0010` plus the register), a branch `1000d00ccc c` (`0200` plus the condition, plus `0020` for a backward branch, then the displacement), SDBD `0001`, HLT `0000`. A backward branch at address p with displacement d goes to p + 1 - d, a forward branch to p + 2 + d. With SDBD before MVII, the immediate is two words, the low byte first and the high byte second.

### P-SDBD (13 words)

| Offset | Decles | Instruction | Cycles |
|---|---|---|---|
| 0 | `02B9 00AB` | MVII #$00AB, R1 | 8 |
| 2 | `0241 0100` | MVO R1, $0100 (scratchpad) | 11 |
| 4 | `0001` | SDBD | 4 |
| 5 | `02BA 0034 0012` | MVII #$1234, R2 (extended immediate) | 10 |
| 8 | `0283 0100` | MVI $0100, R3 | 10 |
| 10 | `00DA` | ADDR R3, R2 | 6 |
| 11 | `0220 0001` | B self | 9 |

The final registers are R2 `$12DF` and R3 `$00AB`. A restore that lost the SDBD prefix reads one decle for the immediate (R2 `$0034`) and then executes the decle `0012`, a DECR R2, as an instruction, so R2 and the flags differ at once. A restore that lost the scratchpad byte loads another R3.

### P-HLT (14 words)

| Offset | Decles | Instruction | Cycles |
|---|---|---|---|
| 0 | `02B9 0003` | MVII #3, R1 | 8 |
| 2 | `0240 0020` | MVO R0, $0020 (display strobe), label loop | 11 |
| 4 | `0011` | DECR R1 | 6 |
| 5 | `022C 0004` | BNZE loop | 9 or 7 |
| 7 | `0000` | HLT | 4 |
| 8 | `0204 0002` | BZE skip | 9 or 7 |
| 10 | `02BA 00FF` | MVII #$00FF, R2 | 8 |
| 12 | `0220 0001` | B self, label skip is word 12 | 9 |

R0 is 0 at the reset, so the strobe value is 0. The three strobes land in the register window of frame 0, so the display is on, its windows come due and the CPU is halted when they do. The last DECR leaves R1 at 0 and Z set; BNZE does not branch; HLT sets only the stop latch; the BZE that follows always executes and, with Z restored, branches over the MVII. The stop comes at the next interruptible boundary, after the BZE. A restore that lost the flags makes the BZE fall through and execute the MVII, so R2 is `$00FF`; a restore that lost the stop latch lets the program run into B self instead of halting.

### P-FAULT (6 words)

| Offset | Decles | Instruction |
|---|---|---|
| 0 | `02B9 0008` | MVII #8, R1 |
| 2 | `0011` | DECR R1, label loop |
| 3 | `022C 0002` | BNZE loop |
| 5 | `0264` | MVO@ R4, R4, a refused encoding (the source equals the pointer register 4 to 7) |

This is the fault program of the slice suite (`rs_prog_fault` in `tests/run/run_slice.c`). The refused word makes a sticky guest fault with the offending word left at R7; every later run call stops at once. A restore that lost the fault goes on past the word.

### P-LATCH (35 words)

| Offset | Decles | Instruction | Cycles |
|---|---|---|---|
| 0 | `0281 0021` | MVI $0021, R1 (the read selects Color Stack) | 10 |
| 2 | `0001 02BA 0001 0008` | SDBD; MVII #$0801, R2 (GRAM card 0, foreground 1) | 4 + 10 |
| 6 | `0242 02F0` | MVO R2, $02F0 (the toggling word, system RAM) | 11 |
| 8 | `0001 02B9 0000 0020` | SDBD; MVII #$2000, R1 (bit 13: advance the color stack) | 4 + 10 |
| 12 | `0241 0201` | MVO R1, $0201 (BACKTAB word 1) | 11 |
| 14 | `0001 02BD 0000 0038` | SDBD; MVII #$3800, R5 | 4 + 10 |
| 18 | `02B9 00FF` | MVII #$00FF, R1 | 8 |
| 20 | `0269` | MVO@ R1, R5 (GRAM word `$3800` = `$00FF`) | 9 |
| 21 | `0282 02F0` | MVI $02F0, R2, label loop | 10 |
| 23 | `03FA 0003` | XORI #3, R2 (toggles `$0801` and `$0802`) | 8 |
| 25 | `0242 02F0` | MVO R2, $02F0 | 11 |
| 27 | `0242 0200` | MVO R2, $0200 (BACKTAB word 0, row 0 column 0) | 11 |
| 29 | `0240 0020` | MVO R0, $0020 (the display strobe, every pass) | 11 |
| 31 | `0283 0000` | MVI $0000, R3 (a register read, blocked outside the window) | 10 |
| 33 | `0220 000D` | B loop | 9 |

The loop is 70 cycles (10 + 8 + 11 + 11 + 11 + 10 + 9). The first read lands in the register window of frame 0 and selects Color Stack; the GRAM write lands before the graphics window closes at cycle 3,780; the strobe repeats in every pass, so every frame is displayed. The word at BACKTAB 0 toggles between `$0801` and `$0802` (a GRAM card 0 with foreground 1 and 2, whose first pixel row is all foreground), so the pixels of row 0 depend on which value the fetch of row 0 latched. BACKTAB word 1 holds `$2000`, so the color-stack pointer advances once per frame and `stic.cs_ptr` is 1 after a frame. The register accesses of the loop that meet a closed window raise `stic.blocked_writes` and `stic.blocked_reads`. A restore that recomputed `stic.row_latch` from the current BACKTAB words instead of restoring it would render row 0 with the value stored after the fetch, so the completed slot differs from the reference; that is what `state.cut.latch-not-recomputed` of plan 08-14 requires.

### P-PSG (60 words)

| Offset | Decles | Instruction |
|---|---|---|
| 0 | `02B8 0034` `0240 01F8` | MVII #$0034, R0; MVO R0, $01F8 (mixer: tones A and B and noise A on) |
| 4 | `02B8 0005` `0240 01F9` | MVII #5, R0; MVO R0, $01F9 (noise period) |
| 8 | `02B8 0040` `0240 01F0` | MVII #$40, R0; MVO R0, $01F0 (tone A fine period, first value) |
| 12 | `02B8 0023` `0240 01F1` | MVII #$23, R0; MVO R0, $01F1 (tone B fine period, first value) |
| 16 | `01C0` `0240 01F4` `0240 01F5` | CLRR R0; MVO R0, $01F4; MVO R0, $01F5 (coarse periods 0) |
| 21 | `02B8 0010` `0240 01FB` | MVII #$10, R0; MVO R0, $01FB (amplitude A: envelope) |
| 25 | `02B8 000A` `0240 01FC` | MVII #10, R0; MVO R0, $01FC (amplitude B, fixed) |
| 29 | `02B8 0010` `0240 01F3` | MVII #$10, R0; MVO R0, $01F3 (envelope period fine) |
| 33 | `01C0` `0240 01F7` | CLRR R0; MVO R0, $01F7 (envelope period coarse 0) |
| 36 | `02B8 0008` `0240 01FA` | MVII #8, R0; MVO R0, $01FA (shape 8: a repeating decay; the write restarts the envelope) |
| 40 | `02B9 00C8` | MVII #200, R1 |
| 42 | `0011` `022C 0002` | DECR R1; BNZE (the first wait: 200 iterations, 2,998 cycles) |
| 45 | `02B8 0031` `0240 01F0` | MVII #$31, R0; MVO R0, $01F0 (tone A fine period, second value) |
| 49 | `02B8 0057` `0240 01F1` | MVII #$57, R0; MVO R0, $01F1 (tone B fine period, second value) |
| 53 | `02B9 00C8` | MVII #200, R1 |
| 55 | `0011` `022C 0002` | DECR R1; BNZE (the second wait) |
| 58 | `0220 0001` | B self |

Mixer `$34` has bits 0 and 1 clear (tones A and B on), bit 2 set (tone C off), bit 3 clear (noise A on) and bits 4 and 5 set (noise B and C off). The tone periods are written twice with different values, so the tone counters are mid-period at the cut and cannot be derived from machine time; the envelope shape `8` repeats a 16-state decay and is never held after its write, so a cut in the first thousands of ticks catches it mid-cycle; amplitude A selects the envelope, so the audio levels follow it. A restore that recomputed a counter from time instead of restoring it would phase the tones wrongly and shift every later level-change record of the audio chain.

## As built (plan 08-14, task 1): the eleven cpu-point ticks

The ticks below were found by the witness scan of `tests/state/state_witness.c` (the first unit boundary at or after the occurrence rule at which the predicate above holds), written into `data/expected/state-cuts.ids` and are re-proved by the id `state.witness.cpu-points`: the scan's first occurrence equals the pin, a fresh instance run to the pin through the public run call ends exactly on that boundary with the same bytes as the scan's instance, and the instance part of the predicate holds there.
They are self-recorded pins that show repeatability of one build and one profile, never evidence of correctness. [nc]
No predicate had to be changed and no program had to be corrected: all eleven predicates hold in their programs, where the document expected them.

| Cut | Program | Tick | Units from the reset | Arithmetic cross-check |
|---|---|---|---|---|
| HP-SDBD | P-SDBD | 112 | 4 | the document's 28 cycles: 5 + 8 + 11 + 4, times 4 ticks per cycle |
| HP-STEAL-A | lamp-idle | 74,932 | 2,256 | frame 1 starts at tick 59,736 and its first bus window at cycle 3,791 (tick 74,900); a unit sees the lines of one CPU cycle (4 ticks) before its start, so the first unit that starts at tick 74,904 or later is the grant; it starts at 74,928 and takes 1 cycle |
| HP-STEAL-B | lamp-idle | 78,580 | 2,473 | the second window starts at cycle 3,791 + 912 = 4,703 (tick 59,736 + 18,812 = 78,548); the grant unit starts at the first idle-loop boundary at or after 78,552 (the lag-adjusted start; here 78,572) and takes 1 cycle, the first hold unit ends 4 ticks after that, at 78,580, and the one window already released in the frame makes the count 1 |
| HP-STEAL-C | lamp-idle | 79,016 | 2,581 | the hold is 114 cycles: BUSRQ falls at tick 78,548 + 456 = 79,004; the release unit is the first unit that starts at 79,008 or later and takes 2 cycles (8 ticks): 79,016 |
| HP-OPEN | lamp-idle | 59,772 | 1,750 | frame 1 starts at 59,736 and INTRM is asserted from its start; a unit sees the line 4 ticks before its start, so the first boundary at or after 59,740 with the enable set and the boundary open is 59,772 (the idle loop's units are 9 cycles, 36 ticks) |
| HP-INT-ENTRY | lamp-idle | 59,808 | 1,751 | the unit after HP-OPEN: an interrupt entry is 9 cycles, 36 ticks: 59,772 + 36 |
| HP-CLOSED | lamp-idle | 59,900 | 1,753 | the interrupt routine's first two instructions (the jump to the handler, 12 cycles, and the display strobe MVO, 11 cycles, 92 ticks together) after the entry: 59,808 + 92 |
| HP-HLT | P-HLT | 372 | 12 | the document's 93 cycles, times 4 |
| HP-HALTED | P-HLT | 15,168 | 3,703 | the first window of frame 0 starts at cycle 3,791 (tick 15,164); the next unit sees the line one cycle (4 ticks) later: 15,164 + 4 |
| HP-FAULTED | P-FAULT | 540 | 19 | the document's 135 cycles, times 4; 19 units: the reset entry, MVII, eight DECR, eight branches (seven taken, one not) and the fault unit |
| HP-RESETPEND | none | 0 | 0 | the boundary before any unit |

The window arithmetic of the table uses the profile fields named at the top of this note (the line sampling lag is one CPU cycle).
The first interrupt of the Lamp Tone run is the one of frame 1 and its vector jumps with the interrupts disabled, as the cut definitions of HP-OPEN, HP-CLOSED and HP-INT-ENTRY assumed; the display latch is set by the first strobe of the routine, so the closed boundary after it is the cut.

The end ticks E of the uninterrupted references (the arithmetic is in `tests/state/state_progs.c`) are 1,000 (HP-SDBD), 4 frames = 238,944 (the lamp cuts), 70,000 (HP-HLT, HP-HALTED: frame 0 ends at 59,736), 1,540 (HP-FAULTED: 540 + 1,000) and 119,472 (HP-RESETPEND).
Each comparison of `state.cut.cpu-points` restores the snapshot of the cut into a target whose STATE and PAD rows hold the byte `0xA5`, compares the whole target instance and the nine digests with the source at once, continues to E with fresh buffers and resubmitted events, and compares the concatenated video, audio and PSG log streams, the first-index fields, the final instance, the digests and the public reports with the uninterrupted run; every cut of the family agrees, and `state.judge.cut-compare-detects-divergence` shows the comparison is able to fail.
