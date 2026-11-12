# Hand derivation of the fixture `ivs-diag-ctrl-psg-1`

Plan 05-01 (D-24, V-06, V-07). This is the derivation of the second original diagnostic fixture, written before the fixture, the scenarios or any PSG code exists.
It is documentation only (A-05): plan 05-11 builds the listing from it, moves the write list and the polled values into the header of `data/golden/diag-ctrl-psg-1.expect` and checks them against a dev report.
Every value below is derived by hand from the controller literal `data/input/intv-2609-ntsc-family-p1.literal.txt`, the register-address literal `data/psg/intv-2609-ntsc-family-p1.regmap.literal.txt` and the CPU cycle table of `data/cp1610/opcode-families.json`; none comes from a generator, a profile field or a core output.
Cycle counts and the tick estimates are inference (the branch counts are the policy of C29, the reset entry an inference) and are regression information only; no assertion depends on them.

## (a) Program design

The program is entered through the project boot stub (contract 1) with R6 = `0340`, uses no SDBD prefix, no interrupt and no STIC, GROM or graphics RAM address, keeps every operand address below `0400`, and has 89 words (limit 110) at `5000-5058`.

### PSG programming (44 words, `5000-502B`)

Eleven stores, each `MVII #value,R0` (2 words) then `MVO R0,$addr` (2 words), in this order:

| # | Address | Role (register-address literal) | Value |
|---|---------|---------------------------------|-------|
| 1 | `$01F0` | tone A period fine byte | `$0023` |
| 2 | `$01F4` | tone A period coarse nibble | `$0001` |
| 3 | `$01F8` | mixer and port direction | `$003E` |
| 4 | `$01FB` | amplitude A | `$000F` |
| 5 | `$01F1` | tone B period fine byte | `$0080` |
| 6 | `$01F5` | tone B period coarse nibble | `$0000` |
| 7 | `$01FC` | amplitude B | `$0008` |
| 8 | `$01F9` | noise period | `$0010` |
| 9 | `$01F3` | envelope period fine byte | `$0064` |
| 10 | `$01F7` | envelope period coarse byte | `$0000` |
| 11 | `$01FA` | envelope shape | `$000A` |

Channel A has the 12-bit tone period `$123` = 291 (coarse nibble 1, fine byte `$23`).
The mixer value `$003E` = `0011 1110` enables tone A only (bit 0 clear) and disables noise A, B, C (bits 3, 4, 5 set) and tone B, C (bits 1, 2 set); bits 6 and 7 are clear, so both ports stay inputs.
Bits 6 and 7 must stay clear: a set bit makes the matching port an output, and a read of `$01FE` or `$01FF` then returns the stored latch and not the pins, so the poll loop below would never see a change.

### Poll table (32 words, `502C-504B`)

Setup, five `MVII` (10 words): R1 = `$0100` (previous port 0 value), R2 = `$0100` (previous port 1 value), R3 = `$0210` (port 0 store pointer), R4 = `$0220` (port 1 store pointer), R5 = `$0008` (port 0 values still to store).
The previous values start at `$0100`, which no masked read can equal, so the first read of each port is always stored.
Loop at `5036`:

```
5036  MVI  $01FE,R0       read port 0
5038  ANDI #$00FF,R0      keep the low byte
503A  CMPR R1,R0          Z when equal to the previous value
503B  BEQ  skip0          taken when unchanged            -> 5040
503D  MOVR R0,R1          remember it
503E  MVO@ R0,R3          store at [R3], R3 += 1
503F  DECR R5             one port 0 value fewer to store
5040  MVI  $01FF,R0       read port 1 (same shape for port 1 with R2 and R4)
5042  ANDI #$00FF,R0
5044  CMPR R2,R0
5045  BEQ  skip1                                           -> 5049
5047  MOVR R0,R2
5048  MVO@ R0,R4
5049  TSTR R5             stop when eight port 0 values are stored
504A  BNZE poll                                            -> 5036
```

Targets follow the rule of `docs/diagnostics.md`: `503B + 2 + 3 = 5040`, `5045 + 2 + 2 = 5049`, and the backward branch `504A + 1 - 0015 = 5036` (displacement `$0015`).
One loop without a store is 33 + 33 + 15 = 81 CPU cycles = 324 ticks (MVI 10, ANDI 8, CMPR 6, taken branch 9, TSTR 6).
The values land in `[0210..0217]` (port 0, eight words) and `[0220..0223]` (port 1, four words); the rest of the tables stay zero.

### Delay and end (13 words, `504C-5058`)

`MVII #$0078,R0`, then `DECR R0` and `BNZE` back to `504E` (120 iterations of 15 CPU cycles, 1,798 CPU cycles = about 7,200 ticks), then `MVII #$03C5,R0` and `MVO R0,$020F` (the end marker, written last), `HLT` and three `TSTR R0` followers (the stop rule of `docs/diagnostics.md`).
Estimated time from reset to halt: the stub and the eleven stores take about 1,000 ticks, the poll starts near tick 1,150, the last input event is at tick 8,800 and is seen within one loop (about 324 ticks) so the eighth value is stored by about tick 9,300, the delay adds 7,200 and the end 100: about 16,700 ticks, below the 20,000-tick cap with a margin of more than 3,000.

## (b) Write list asserted by the scenario

The scenario asserts exactly these eleven (address, value) pairs, in this order, as the PSG register-write log of the run (raw 16-bit bus values):

```
$01F0 $0023
$01F4 $0001
$01F8 $003E
$01FB $000F
$01F1 $0080
$01F5 $0000
$01FC $0008
$01F9 $0010
$01F3 $0064
$01F7 $0000
$01FA $000A
```

## (c) Polled values from the literal

Input events (a whole controller word at the tick, applied at the first CPU-unit boundary at or after it), with at least 1,000 ticks between events, far above one loop of 324 ticks, and the first event at tick 2,000, after the first reads at about tick 1,150.
The byte read is FF XOR the OR of the cleared masks of the pressed controls (literal masks: KEY_1 `81`, KEY_9 `24`, KEY_5 `42`, DISC_N `04`, DISC_E `02`, SIDE_TOP `A0`, SIDE_LOWER_LEFT `60`).

| Port | Tick | Controls pressed | OR of masks | Byte read |
|------|------|------------------|-------------|-----------|
| 0 | start | none | `00` | `FF` |
| 0 | 2,000 | KEY_1 | `81` | `7E` |
| 0 | 3,000 | none | `00` | `FF` |
| 0 | 4,000 | KEY_1, KEY_9 | `81` OR `24` = `A5` | `5A` |
| 0 | 5,000 | none | `00` | `FF` |
| 0 | 6,000 | DISC_N | `04` | `FB` |
| 0 | 7,000 | DISC_N, SIDE_TOP | `04` OR `A0` = `A4` | `5B` |
| 0 | 8,800 | KEY_5, SIDE_TOP (guest-ambiguous, class key-side) | `42` OR `A0` = `E2` | `1D` |
| 1 | start | none | `00` | `FF` |
| 1 | 2,500 | DISC_E | `02` | `FD` |
| 1 | 4,500 | SIDE_LOWER_LEFT | `60` | `9F` |
| 1 | 6,500 | none | `00` | `FF` |

Port 0 sequence stored at `[0210..0217]`: FF 7E FF 5A FF FB 5B 1D (eight values, so the loop ends).
Port 1 sequence stored at `[0220..0223]`: FF FD 9F FF, and the remaining table words are 0.

## (d) Chip-side consequences (properties, not timings)

- Channel B has both its tone bit and its noise bit disabled, so its mixer output is a constant 1 and its level is its amplitude, 8, from the write at `$01FC`.
- Channel C has amplitude 0 (never written), so its level stays 0.
- The timing of samples is not asserted (V-07); only the write list, the polled values and the end state are.

### Tone A stays silent

The tone counter is empty at reset and reloads on the first step, which is at master tick 16, before the first program write (about tick 150); the period then in the register is 0, which acts as 4096 steps = 65,536 master ticks.
A period written afterwards takes effect only at the next reload, after about tick 65,536, and the program halts near tick 16,700, so tone A shows no level change and no tone is audible in this fixture.
Channel A is therefore level 0 for the whole run, and the fixture is a register-write and controller-read diagnostic, not a listening test; only module-level vectors (`psg.tone.*`) show early edges.

## (e) Mutants of plan 05-11, derived by hand

Each mutant changes one word of the listing and must end as stated; the clause that breaks is named.

| # | Mutation | Clause that breaks | Expected outcome |
|---|----------|--------------------|------------------|
| 1 | the immediate of the mixer write `$003E` changed to `$003F` | write list entry 3 | guest-fail, exit 10 |
| 2 | the immediate of the tone A coarse write `$0001` changed to `$0002` | write list entry 2 | guest-fail, exit 10 |
| 3 | the port 1 pointer `MVII #$0220,R4` changed to `#$0221` | the `[0220..]` asserts, while the port 0 loop still ends | guest-fail, exit 10 |
| 4 | the first decle of `ANDI #$00FF,R0` at `5038` replaced by the first decle of `MVII` (same register, same `$00FF` operand; the CP-1610 has no OR) | the polled value is frozen at `$00FF`, port 0 never changes after the first store, so seven stores stay missing and the loop cannot end | timeout, exit 11 |
| 5 | the delay immediate `$0078` changed to `$0050` | none (negative mutant: only the end time moves) | pass, exit 0 |

## (f) Caveat

The author of this derivation, of the listing, of the literals and of the core is one model family, so one misreading of a source can be shared by all of them, and agreement between them is not independent evidence about hardware.
The controller bytes rest on two community pages that are likely one lineage (C16), the register map on one community page (C70), and the tone-A silence and the write-effect instant on project conventions (C77, C78).
A pass of this fixture shows the model is self-consistent on these paths and nothing more.

## (g) Correction of 2026-11-12 (plan 05-11, found while hand-assembling the listing)

Section (a) names `R3` as the port 0 store pointer and `MVO@ R0,R3` as a store with `R3 += 1`.
The addressing-mode table of the manual (`Fig. 3.2.1, pdf 46`; `docs/cp1610/reference-notes.md`, section "Addressing modes") increments the register after the access only for mode 100 (R4) and mode 101 (R5); modes 001 to 011 (R1 to R3) address indirectly and do not increment.
The listing therefore keeps the pointer registers that auto-increment and moves the counter: R4 = `0210` (port 0 pointer), R5 = `0220` (port 1 pointer), R3 = `0008` (port 0 values still to store), R1 and R2 as before.
Nothing else in this document changes: the word count (89), the addresses of the loop (`5036`, targets `5040`, `5049`, `5036`), the instruction mix and so the 81-cycle loop, the write list, the polled values, the tables `[0210..0217]` and `[0220..0223]` and the mutants (mutant 3 patches the immediate of `MVII #$0220,R5`, not R4) are the same; the original text above is kept as written so that the order of the commits stays the evidence.
No asserted value depends on a register choice: the scenario asserts memory, PSG registers and the write list only.
