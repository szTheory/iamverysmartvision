# Project diagnostics and the boot stub

This note says what a project diagnostic is, how its source is written, what the project boot stub promises, and the review rule that keeps the stub from growing into a replacement for a system program.
It records decisions D-25 to D-30 of Phase 4.
A label [INF] marks a design reason; every other line is a rule.

## What a project diagnostic is

A project diagnostic is a small guest program written for this project, whose expected end state is derived by hand from the manual tables and arithmetic, never from the emulator under test (D-25, D-27).
Its source of truth is a hand-assembled, annotated text listing: one line per 10-bit word, with the word in hex and in bits, the opcode row it encodes and the instruction text. A fixture written with the project assembler has an `.asm` source as its source of truth and a generated listing (section "The project assembler"); the four listings of Phases 4 to 6 stay hand-assembled.
No third-party assembler is a build, gate, CI or release dependency, and no code, table or comment of another implementation or assembler is copied into a listing or into the project assembler. The project assembler `ivs_asm` is a build-only tool of this tree, never installed and never shipped.
Every word is derived by hand from the project's CP-1610 notes (`docs/cp1610/`, `data/cp1610/opcode-families.json`) and the manual pages each listing cites; the Phase 2 vector rows (`data/cp1610/vectors/*.json`) give a second, independent statement of each encoding.

Listings are committed as text under `fixtures/FX-<id>/` with a fixture manifest (`docs/fixture-manifest.md`).
Images built from them are never committed (see "Build-tree images").

## Listing line grammar

A listing is ASCII text with LF line endings (D-26).

- A line that starts with `#` is a comment. Header keys are comment lines of the form `# key: value`; every listing has `id`, `kind`, `profile` and `window` (four hex digits, a dash, four hex digits).
- A blank line is ignored.
- Every other line is a word line: `ADDR HEX4 BITS ROW ASM`, tokens separated by spaces.
  - `ADDR`: four hex digits, the word address. Word lines are contiguous from the first one and stay inside the window.
  - `HEX4`: four hex digits, the word; at most `03FF`.
  - `BITS`: groups of binary digits, 10 digits in total, whose value equals `HEX4`. An instruction word is split at its printed field boundaries (as in the vector JSON `program` strings), an operand word as 2+4+4 or at its printed fields.
  - `ROW`: the most specific leaf row id of `data/cp1610/opcode-families.json` for the instruction that starts on this line, or `-` for an operand word of the instruction above it (an instruction of `words` words is followed by `words - 1` operand lines), or `DATA` for a data word (below).
  - `ASM`: the rest of the line, at least one token: the instruction text and notes.
- Zero fill: an unreachable zero word carries the row of the decle it encodes (`HLT`) and the text `fill`.
- Branch targets (plan 05-01, D-24): the `ASM` text of every branch line (rows `B`, `BC`, `BOV`, `BPL`, `BZE`, `BLT`, `BLE`, `BUSC`, `BNC`, `BNOV`, `BMI`, `BNZE`, `BGE`, `BGT`, `BESC`, `BEXT`) and of every `J` line carries `-> ADDR`, the target in four hex digits. The checker recomputes the target from the printed words: for a branch it is the address of the branch plus 2 plus the displacement word when the direction bit (bit 5 of the first decle) is 0, and the address plus 1 minus the displacement word when it is 1 (`5020 + ~0003 = 501C` for the `BNZE` of `diag-cpu-1`), modulo 65536; for `J` the bits 7..2 of the second decle are target bits 15..10 and the third decle holds bits 9..0. A printed target that differs fails with `listing-branch-target`, and a branch or jump line without the annotation fails with `listing-branch-target-missing`.
- The `SDBD` prefix (plan 06-01, D-20, A-04; token `listing-sdbd`): an `SDBD` line applies to the next instruction only. When that instruction is an immediate form (`MVII`, `ADDI`, `SUBI`, `CMPI`, `ANDI`, `XORI`) it is followed by two `-` operand lines instead of one, the low byte and then the high byte of the doubled operand, each at most `00FF`, so the word count of the pair is 1 + 2 x operand and every later address follows from that. The effect never carries past that one instruction: an extra operand line, a second `SDBD` directly after an `SDBD`, an `SDBD` before any row that is not an SDBD target (a branch or jump, `MVO@`, `MVO`, `PSHR`, `MVOI`, a direct or register row, an indirect read row with mode 0 or 6), an `SDBD` that nothing follows and a missing operand line all fail with `listing-sdbd`. The accepted targets are the six immediate rows and the six indirect read rows (`MVI@`, `ADD@`, `SUB@`, `CMP@`, `AND@`, `XOR@`) with mode 1 to 5, the set the CPU itself allows (a prefix before anything else ends the guest in a fault); the indirect forms take one word each, because the doubled read happens at run time and adds no listing word. The decode order of the checker (low byte first) is compared with `ivs_pack --dump-words` by `tests/pack-tap.sh` once plan 06-13 adds the first listing with an `SDBD` line; until then the order rests on the Phase 2 vector rows and is not testable against the packer. The earlier diagnostics keep using no `SDBD`.
- Data rows (plan 07-04, A-10): a word line whose `ROW` is `DATA` is a data word written by `ivs_asm` for a `DECLE` line, with the text `DECLE $XXXX`. The checker gives it no leaf-row label and no branch target, and it takes no operand lines; it is still checked for `listing-bits-mismatch` and `listing-range`. It is refused with `listing-operand-count` while the instruction above it still lacks operand lines and with `listing-sdbd` directly after an `SDBD` line (a data word is not an `SDBD` target), and a boot stub may not hold one (`bootstub-row`). Two controls, `listing-data-in-operand-slot` and `listing-data-after-sdbd`, show both refusals, and the pristine toy listing of the controls holds one valid `DATA` word. No committed fixture listing uses `DATA` yet: the scenario program has no table (D-01), and the legacy third listing keeps its pattern table as words decoded as code.

The gate step `listing` (plan 04-15) checks this grammar and the row and operand rules; `tools/pack` reads the same grammar and is the only tool that turns a listing into image bytes.

## Endings need a follower

`HLT` only sets the stop latch: the CPU stops after the next instruction, and only at an interruptible boundary (`docs/cp1610/stepper-conventions.md`, C40, C56).
`HLT` is itself not interruptible, so a run of zero words (each decodes as `HLT`) never stops; zero fill is not a trap.
Every stub or diagnostic ending is therefore `HLT` followed by at least one interruptible instruction such as `TSTR R0` (D-29).
The test `run.stub.halt-needs-follower` shows both halves: with its follower the stub's trap halts at `100A`; with the follower replaced by a zero word the CPU goes on through the zero fill and the reset code and halts only at `1013`.

## The project boot stub, contract 1

The machine's reset needs an EXEC-window program and a GROM image (profile `intv-2609-ntsc-family-p1`).
The project boot stub is the program the project supplies there for its own diagnostics (D-28): `fixtures/FX-boot-stub/boot-stub.listing.txt`, manifest `FX-boot-stub`, label "project boot stub, not an EXEC".
It is never evidence that any real system program behaves this way, and nothing about it is a hardware claim.
The GROM stand-in is 2048 zero bytes, labelled "not a GROM" (D-30); it is synthesized where needed and never committed.

Contract 1 (22 words at `1000`-`1015`, window `1000`-`101F`):

| Address | Words | Instruction | Role |
|---|---|---|---|
| `1000` | `0200 000E` | `B $1010` | reset entry (the profile's `reset.r7`); skips the vector area |
| `1002` | `0000 0000` | fill | unreachable |
| `1004` | `02B8 0001` | `MVII #$0001, R0` | interrupt entry (the profile's `interrupt.iab_value`): unexpected-interrupt trap |
| `1006` | `0240 035F` | `MVO R0, $035F` | records `[035F] = 1` |
| `1008` | `0000` | `HLT` | stop latch |
| `1009` | `0080` | `TSTR R0` | the follower; the CPU halts after it with R7 = `100A` |
| `100A` | `0000` x 6 | fill | unreachable |
| `1010` | `0003` | `DIS` | reset code: interrupt system off |
| `1011` | `02BE 0340` | `MVII #$0340, R6` | stack pointer at the project stack start |
| `1013` | `0004 0350 0000` | `J $5000` | hand-off to the cartridge entry |

Labels: `reset` (`1000`), `trap` (`1004`-`100A`), `init` (`1010`), `handoff` (`1013`).
The cartridge entry `5000` and the stack start `0340` are project conventions with no source behind them.
Only these are promised: the reset path `1000, 1010, 1011, 1013, 5000` (R7 at unit starts after the reset entry, test `run.stub.path`), interrupts disabled and R6 = `0340` at the hand-off, and the trap's `[035F] = 1` and halt inside `1004`-`100A` (test `run.stub.trap`).
R0 to R5 and the flags at the hand-off are whatever reset leaves; a diagnostic never relies on them.
Any change of a word is a new contract number, a new manifest note and new expected values.

## Review rule: no routines, no services

The stub may initialise hardware to the profile's documented power-on state and hand off to the cartridge entry.
It provides no routine, service, entry point or data to a cartridge: no callable code, no table, no font, no vector a cartridge may use, and nothing read from GROM or a cartridge (D-29).
Every later revision of the stub cites this rule in its commit and states which of its words serve which allowed purpose.
The gate enforces the mechanical half from plan 04-15 on: addresses inside `1000`-`101F`, at most 16 non-zero words, only the rows `HLT`, `DIS`, `EIS`, `MVII`, `MVO`, `TSTR`, `B` and `J`, and non-zero words at the profile's reset and interrupt addresses.
[INF] The rule keeps the boot path a hand-off, so no diagnostic result depends on a high-level imitation of a system program.

## Build-tree images

`tools/pack` (`ivs_pack`) decodes a listing and writes images only under the build tree (D-30):

- `--exec <listing> <out>`: the 8192-byte raw EXEC image, the word at `1000 + i` at byte offset `2i`, high byte first, bits 15..10 zero, zero tail.
- `--ivsc <listing> <out>`: a one-segment IVSC v1 container (`docs/formats/ivsc-v1.md`).
- `--dump-words <listing>`: one `ADDR WORD` line per word, for the independent comparison with the gate's own parse (plan 04-15).
- `--patch ADDR=HEX`: replaces one listed word before any output (repeatable); used for mutants.
- `--c-include <out.inc> <name>=<file>...`: the image bytes as C arrays for the in-process suites.

The default build of both presets writes `build/<preset>/images/boot-stub.exec`, `diag-cpu-1.ivsc`, `diag-ctrl-psg-1.ivsc` and (plan 06-13) `diag-stic-1.ivsc` under `build/<preset>/images/` (CMake variable `IVS_IMAGES_DIR`); the output holds no time, path or host-dependent byte, so both presets produce the same bytes.
Suite `pack` compares `ivs_pack --dump-words` with the gate's own parse for every listing and, for a cartridge listing, checks the IVSC trailer against `cmake -E sha256sum` of the container minus its last 32 bytes (`pack.trailer-<id>`).
No `.exec`, `.ivsc`, `.bin` or `.rom` file is tracked, and no real EXEC, GROM or cartridge image is committed, cached or required.
`ivs_pack` is a development tool: it is never installed and never shipped.

## ivs-diag-cpu-1

The first project diagnostic (D-31): `fixtures/FX-diag-cpu-1/diag-cpu-1.listing.txt`, manifest `FX-diag-cpu-1`, one IVSC v1 segment of 56 words at `5000`-`5037`, entered through the boot stub (contract 1).
It sets every register it uses before using it, and uses no SDBD, no shift, no interrupt and no STIC, PSG, GROM or graphics RAM address.

What it covers:

- Flags: `R0 = 03FF` doubled five times to `7FE0`, `ADDI #$001F` gives `7FFF` (stored at `0200`); `INCR` to `8000` writes S and Z only, so `GSWD` gives `8080`; `DECR`, then `ADDI #$0001` to `8000` sets S and OV (`A0A0`); `ADDR R0,R0` of `8000` gives `0000` with C, OV and Z (`7070`). The three status words separate an instruction that writes two flags from one that writes four.
- A loop: `ADDR R2,R3; DECR R2; BNZE` back over 1..10, sum `0037`, which also uses a backward branch displacement.
- The stack: a `PSHR R3; PULR R5` round trip at the stub's stack start `0340`, and the restored stack pointer.
- Indirect addressing: `MVI@ R4,R0` from `0200` with R4 incremented after the read.
- The end: the marker `[020F] = 03C5` is the last store, then `HLT` and three `TSTR R0` followers.

The predicate is `data/golden/diag-cpu-1.expect` (tier `original-diagnostic`, oracle `independent-derivation`): stop reason halted, R0-R6, and the ten words `[0200]`-`[0208]` and `[020F]`.
Its values were derived by hand from 16-bit arithmetic, the status-bit rules (`§3.1.2, pdf 45`), the GSWD layout (`pdf 56`), the stack direction (C17) and the instruction tables, and the derivation was committed before the program first ran on the emulator; the file shows each step with its page.
Excluded on purpose: R7 and the flags at the stop (they depend on the HLT and interruptibility policies, C40, C45, C56), machine time (the hand-summed 520 CPU cycles to the halt are regression information only, since the 5-cycle reset entry is an inference and the branch count is policy C29), and the stack word `[0340]`.
The test `run.diag.predicate` reads the expected values from the assert lines of that file through a generated include; `run.diag.slice` replays the program under the five slicing patterns of D-07, and `run.diag.repeat` compares two fresh instances.

Shared-misreading risk: the author of the listing, of the predicate and of the core is the same, so all three can share one misreading of the manual, and agreement between them is not independent evidence about hardware.
The risk is bounded by deriving the values from arithmetic and printed tables rather than from a second interpreter, by excluding policy-dependent state, and by keeping the claim on this diagnostic provisional (tier `original-diagnostic`, scoped to the project boot stub and this diagnostic, never to commercial content or any EXEC). [claim:CL-0008]
The cartridge entry `5000` and the stack start `0340` it relies on are project conventions without a source (C68).

libasm cross-check (D-25, optional and never gating): not run, the tool is absent on the authoring machine; no assembler output was compared with the listing.

Plan 04-20 adds program mutants of this listing through `ivs_pack --patch` (for example `ADDI` to `SUBI` at `5007`, `BNZE` to `BZE` at `501E`, `PSHR` replaced at `5022`, `INCR` to `DECR` at `500B`); each must end as a guest failure that names the predicate clause it breaks.

## ivs-diag-ctrl-psg-1

The second project diagnostic (D-24): `fixtures/FX-diag-ctrl-psg-1/diag-ctrl-psg-1.listing.txt`, manifest `FX-diag-ctrl-psg-1`, one IVSC v1 segment of 89 words at `5000`-`5058` (limit 110), entered through the boot stub (contract 1) with R6 = `0340`.
It uses no SDBD, no interrupt and no STIC, GROM or graphics RAM address, keeps every operand address below `0400` (one decle each) and has three sections:

- S1, eleven PSG programming writes (`MVII #value,R0; MVO R0,$addr`) in the fixed order `$01F0`, `$01F4`, `$01F8`, `$01FB`, `$01F1`, `$01F5`, `$01FC`, `$01F9`, `$01F3`, `$01F7`, `$01FA`. The mixer value `$003E` keeps bits 6 and 7 clear so both ports stay inputs and a read of `$01FE` or `$01FF` returns the pins, not the stored latch.
- S2, a poll loop that reads `$01FE` (port 0) and `$01FF` (port 1), keeps the low byte (`ANDI #$00FF`; the CP-1610 has no OR instruction), and stores a value that differs from the previous one at `[0210+k]` and `[0220+k]` through `MVO@` with R4 and R5, the only registers that auto-increment (`Fig. 3.2.1`, derivation section (g)); it stops when eight port 0 values are stored.
- S3, a counted delay of 120 iterations, the end marker `[020F] = 03C5` written last, then `HLT` and three `TSTR R0` followers.

The listing has four branches, each annotated `-> ADDR` for the branch-target rule above.
The expected values were derived by hand in `docs/input/diag-ctrl-psg-1-derivation.md` (plan 05-01) before the listing existed, and move into the header of `data/golden/diag-ctrl-psg-pass.expect` with the scenario (A-05).
Its program mutants are in `data/expected/diag-mutations-ctrl-psg.ids`.
The diagnostic is state-only: no assertion depends on machine time, and its halt tick is regression information.
No real EXEC, GROM or cartridge is involved, and nothing in this section is a statement about measured hardware.

The predicate is `data/golden/diag-ctrl-psg-pass.expect` (tier `original-diagnostic`, oracle `independent-derivation`): the halt, the end marker `[020F] = 03C5`, the polled port 0 table `[0210..0217]` = `00FF 007E 00FF 005A 00FF 00FB 005B 001D`, the port 1 table `[0220..0223]` = `00FF 00FD 009F 00FF` with the rest of the table zero, the eleven PSG registers read back (the shape register reads 0, C75), and a write log of exactly the eleven writes in order. A full 16-bit read of a PSG register depends on the default upper byte 0 (C64), so those lines carry `# policy: C64`. The program under the project boot stub reaches that state from a power-on reset [claim:CL-0014].
Excluded on purpose: machine time, all CPU registers, R7 and the flags, and the ticks of the log records (printed in the report, never asserted). The chip-side consequences (channel B constant at level 8, channel C and channel A silent for the whole run) are tested in `run.diag.ctrl-psg.*` as properties; channel A is silent because its tone counter reloaded with the period of an empty register and the period written later takes effect only after about 65,536 ticks (C77), so the diagnostic is no listening test.
Shared-misreading risk: the author of the controller and register literals, of the derivation, of the listing and of the core is the same, so all of them can share one misreading of a source; agreement among them is not independent evidence about the original console. The controller bytes rest on two community pages that are likely one lineage (C16) and the register map on one community page (C70).
The contrast scenario `diag-ctrl-psg-nokey` (no input: timeout, the guest still polling), the one-tick-apart `diag-ctrl-psg-order` and five program mutants (`data/expected/diag-mutations-ctrl-psg.ids`, one of them a negative control that must still pass) show that controller input, and not time, changes the guest state and that the predicate can fail for the intended reasons.

## ivs-diag-stic-1

The third project diagnostic (D-20, A-04): `fixtures/FX-diag-stic-1/diag-stic-1.listing.txt`, manifest `FX-diag-stic-1`, one IVSC v1 segment of 133 words at `5000`-`5084` (limit 170), entered through the boot stub (contract 1, unchanged) with R6 = `0340` and interrupts disabled.
It has no interrupt service routine, calls no EXEC routine and reads no graphics ROM, so the display is not sustained: exactly one picture, frame 0, exists, because the project boot stub has no service routine that would strobe the display again (a project interrupt routine is a Phase 7 need).
It is the first listing that uses `SDBD`: every 14-bit word is written `SDBD`, `MVII`, `MVO` (the `listing-sdbd` rule above), and `tests/pack-tap.sh` compared the decode order of the listing checker with `ivs_pack --dump-words` on an `SDBD` listing for the first time (`pack.words-ivs-diag-stic-1`).
The program, in the order of the listing:

- a, one read of `$01FE` (port 0): the low byte is kept (`ANDI #$00FF`) and recorded at `[02F1]`, and Color Stack mode is selected by a read of `$0021` only when a key is closed (low byte not `FF`). That is the one mode branch, so one image serves both modes: the scenario `diag-stic-cstack-pass` closes KEY_1 at tick 0 and `diag-stic-fgbg-pass` gives no input.
- b, the display-enable strobe (a store to `$0020`; the value written is ignored), inside the 2,000-cycle register window after tick 0.
- c, the four color stack registers `$0028`..`$002B` (`6 A 3 D`, used by Color Stack mode only), then the copy of 32 pattern bytes from the table at `5065` to graphics RAM `$3800`..`$381F` (four cards: stripes, box, diagonal and checker, project-authored geometric shapes and no pattern recalled from any real graphics ROM).
- d, the fill of the 240 BACKTAB words with `0000`, then six explicit card words written as 14-bit values (`02A3` = `2A01`, `02A4` = `0E0A`, `02A5` = `1A13`, `02A6` = `381C`, `02DC` = `2C1D`, `02EF` = `1C0F`; the same words are read in both modes, so the two pictures differ only by the mode).
- e, a counted delay (900 iterations) so that the `HLT` unit starts between the end of frame 0 (59,736 ticks) and the end of frame 1 (119,472 ticks) with at least 4,000 ticks of margin both with the display on and in the mutant without the strobe, the end marker `[02F0] = 03C5` written last, `HLT` and three `TSTR R0` followers.

The listing has five listed branches, each annotated `-> ADDR`: four in the program and one in the data. The checker decodes every word of the file as a program word, so the 32 pattern decles carry the row of the decle each encodes, and the three decles `04`, `02`, `01` of card 2 decode as one three-decle `J` that the checker requires to carry `-> 0001` (it is never executed: the table lies behind the three followers).
The expected values were derived by hand in `docs/video/diag-stic-1-derivation.md` (plan 06-01) before the listing existed, and the hand-derived frames are `data/video/frames/F-D1.frame` (Foreground/Background) and `F-D2.frame` (Color Stack). They move into the header and the assert lines of `data/golden/diag-stic-fgbg-pass.expect` and `diag-stic-cstack-pass.expect` (tier `original-diagnostic`, oracle `independent-derivation`), which were committed before the first run of either scenario; the `assert frame 0 hash` line of each equals the `sha256-text` key of its frame file, which the gate step `video` checks (`video-frame-hash-expect-drift`).
The `.digests` goldens are self-recorded by `ivs-run --update-golden` (repeatability only). The same image is replayed in process (`run.diag.stic.*`: both modes, slices of 1, 7 and 468 ticks, a frame-stop call sequence, and the mutant without the strobe) and through the installed package (the third replay of `tests/installed-consumer`, `stic.`-prefixed lines, whose digests and observed domains equal `data/golden/diag-stic-fgbg-pass.digests`).
Its program mutants are in `data/expected/diag-mutations-stic.ids`: six that must end guest-fail (a card byte, a swap of the foreground and background fields of one word, a word stored one column off, the strobe address, the sense of the mode branch and the copy count) and one negative (the delay count 900 to 800) that must still pass; the scenario `video-assert-mismatch` (a deliberately wrong frame hash) ends guest-fail naming the hash assert.
Excluded on purpose: machine time (the stop near tick 88,500 is regression information; only the margins of the cycle budget are a design requirement), the frame tick of the slot header, all CPU registers, R7 and the flags, the stack word `[0340]`, the graphics RAM contents (not readable by the CPU at the stop) and the number of bus windows.
Shared-misreading risk: the author of the register literal, the card rules, the frames, the derivation, the listing and the core is the same, so all of them can share one misreading of a source; agreement among them is not independent evidence about the original console. The card layouts rest on one community page (C96), the register roles on the same page (C81), and every window and bus figure is a default (C1, C2, C92).
No real EXEC, GROM or cartridge is involved, and nothing in this section is a statement about measured hardware.

## The project assembler ivs_asm

Phase 7 re-examined the D4 trigger (D-16) and decided to build a project mini-assembler. It replaces the earlier text "when a project assembler becomes worth writing", whose rule was: more than 3 project diagnostics, a listing that needs labels or macros, or a second author (D-25). After the third diagnostic (plan 06-13) the rule had not fired, and the finding of that listing was that the checker treats data as program.

### The decision

- The trigger fired on three independent counts: a fourth project-authored program (the scenario program of Phase 7), labels (the interrupt entry reached from the boot stub, loop heads, table-address immediates), and a program of more than 8 branches (the rule of plan 06-13 fires by its letter on the branch count; the word count of 132 to 136 would not fire it).
- The break-even on Phase 7 alone is neutral (R-5). The tool is adopted for a structural reason, not an arithmetic one: labels and the reference from the stub to the program, and about five single-feature fixtures that Phase 8 needs.
- `ivs_asm` generates the committed listing grammar of this document plus the one row token `DATA`. `ivs_pack` is unchanged and reads the generated listing like any other, and the checks of the `listing` step and the `pack` suite apply to it as to a hand listing.
- It is one C17 file, `tools/asm/asm.c`, registered like `tools/pack` and `tools/pcm`, never installed or shipped, with no dependency. Its encoding table is hand-written from `docs/cp1610/reference-notes.md` and the manual pages that file cites. No third-party assembler or emulator source, table or output was read or run, and `data/cp1610/opcode-families.json` is the checker's police, never an input of the tool.

### Source grammar (A-15)

The source is ASCII with LF line endings (a CR byte is a refusal), one statement per line, at most 255 bytes per line, 4,096 lines, 4,096 words, 256 symbols and 131,072 bytes (`asm-limit`).

- Header: the leading block of `# key: value` lines is copied to the listing unchanged. The keys are those of the listing header (`id`, `kind`, `contract`, `profile`, `window`, `label`, `entry`, `stack`), each at most once; `# window: AAAA-BBBB` is required and every word must lie inside it (`asm-window`).
- `; text` is a comment. `label:` defines a global label (case-sensitive, at most 31 characters, defined once); `NAME EQU expr` defines a constant from earlier symbols; `ORG expr` sets the address, forward only (the first `ORG` comes before any word or label), and the gap is filled with zero words (row `HLT`, text `fill`); `DECLE v1, v2, ...` writes data words of at most `03FF`; `SDBD` is the prefix of the next instruction.
- Operands are a register `R0` to `R7`, an immediate `#expr`, or a bare expression (a direct address, a branch or jump target, a shift count). An expression is numbers (`$` and one to four hex digits, or decimal up to 65535) and symbols joined by `+` and `-`, evaluated left to right with every partial result 0 to 16777215; each use then limits its own value: an immediate and an address or target at most 65535, a direct address and a data word at most `03FF`.
- Spellings are those of the vector files and the listings: `MVII #$0006, R0`, `MVI $01FE, R0`, `MVO R0, $0200`, `MVO@ R0, R5` (source first, pointer second), `MVI@ R4, R0` (pointer first), `SLL R1` or `SLL R1, 2`, `JSR R5, target`, `BEXT target, code`, `NOP` or `NOP 1`, `JE`, `JD`, `JSRE`, `JSRD`. `MVOI` has no spelling: its operand word is the cell it overwrites.
- Not in the language, on purpose: macros, includes, conditional assembly, local labels, parentheses and precedence, string directives, relocation, a linker, auto-SDBD, spelling aliases, cycle counting.
- Two-pass assembly: pass 1 gives every statement its address from syntax alone (an immediate after `SDBD` is three words), pass 2 resolves values, so there is no size fixpoint.

### Encoding rules that are refusals

- Explicit `SDBD` only: the prefix must be followed by an immediate row (`MVII`, `ADDI`, `SUBI`, `CMPI`, `ANDI`, `XORI`), whose 16-bit operand is then written as a low byte word and a high byte word, or by an indirect read row (`MVI@`, `ADD@`, `SUB@`, `CMP@`, `AND@`, `XOR@`) through `R1` to `R5`; anything else after `SDBD`, or an `SDBD` with nothing after it, is `asm-sdbd-target`, the same rule as the checker's `listing-sdbd`. An immediate above `03FF` without the prefix is `asm-imm-sdbd`, including a forward label resolved in pass 2; above `FFFF` it is `asm-imm-range`.
- A direct address above `03FF` is `asm-direct-range`; a data word above `03FF` is `asm-decle-range`.
- A spelling whose decle belongs to another leaf row is `asm-spelling` and names the row to write: `MOVR R1, R1` is `TSTR R1`, `MOVR R1, R7` is `JR R1`, `XORR R1, R1` is `CLRR R1`, `MVI@ R6, R1` is `PULR R1`, `MVO@ R1, R6` is `PSHR R1`, and an indirect register `R7` is the immediate row.
- Branch displacement: forward `target - (pc + 2)`, backward `(pc + 1) - target` with the direction bit set, the formula the checker uses for the printed `-> ADDR`; the displacement is one decle, at most `03FF` (`asm-branch-range`, which also refuses a target above `FFFF`: the tool never relies on the wraparound of C50). The manual prints the range of `B` as +1025 to -1024; the decle arithmetic gives a forward reach of `pc + 1025` and a backward reach of `pc - 1022`, and the assembler follows the decle limit. The difference is discrepancy C106, with no hardware claim.
- Every refusal is one stderr line `ivs_asm: <token>: <source>:<line>: <detail>`, exit status 1 and no output file. The 20 tokens are pinned in `data/expected/asm-tokens.ids`, each with its own case in the suite `asm`.

### Generated listing

The generated listing has the header lines of the source, then one comment line `# generated by ivs_asm from <path>; edit the source, not this file` (the path is the relative path the tool was given), then one comment line `# sym: <name> <ADDR>` for each label, then the word lines of the grammar above. A label is never written as `# label:`, which is a header key (A-01). Branch and jump lines carry `-> ADDR`; the bits of an instruction word are grouped as the printed fields (4+3+3, 4+2+4 for a branch, 2+6+2 for the second decle of a jump), of an operand word as 2+4+4. The tool writes the listing beside its target and renames it, so a refusal leaves no file, and its output holds no time, host or absolute-path byte.

### What polices the table (D-18)

The expected values never come from the tool. Three outside checks police the hand-written table, all in the suite `asm` under the existing `c-tests` step:

- The row check of the `listing` step recomputes the leaf row of every decle of every generated listing from `data/cp1610/opcode-families.json`, together with the bits, operand-line, range and branch-target rules.
- `tests/asm/pairs.txt` is a flat copy, made once, of the asm and word pairs of the Phase 2 vectors (304 pairs in nine row families, SDBD immediate pairs masked to their low bytes, three hand cases), so there is no JSON parse at test time; the vectors are pinned by the `tests-vectors-digest` token, so a deliberate change of a vector is a visible diff to mirror by hand.
- Four twins under `tests/asm/twins/` reproduce the 300 hand-typed words of the four hand listings (the boot stub 22, `diag-cpu-1` 56, `diag-ctrl-psg-1` 89, `diag-stic-1` 133 with its pattern table written as `DECLE`), compared through `ivs_pack --dump-words` of the generated and the committed listing; self-controls show that both comparisons can fail.

### Twin discipline

The four legacy listings, their manifests and their goldens are untouched. A twin is test data with no manifest and nothing is built from it. A twin is promoted to the source of truth, with a manifest `recipe`, only when its legacy listing next needs an edit (then the two change in the same commit or the twin takes over). A fixture written with the assembler commits its `.asm` and the generated listing, and a suite case regenerates the listing and requires byte equality.

### What reverses the decision

Two encoding errors found only by a run (not by the row check, the pairs or the twins), the source above 1,200 lines, or a request for macros, includes or conditionals: stop and hand-assemble the affected listing, or decide whether the program is too large for this approach.

### Effort data behind the decision

Effort data of the third diagnostic (plan 06-13), recorded for that decision:

- Words derived: 133 in the listing (101 of code and 32 of pattern data), 220 lines with its header; the derivation `docs/video/diag-stic-1-derivation.md` is 126 lines (121 written in plan 06-01 and a dated as-built section), and the six explicit words and the cycle budget were its main cost.
- Branches: 4 in the program, 1 more in the data table (the decode of the pattern decles `04 02 01`); the limit is 8.
- Errors caught by the listing checker: 0. Two slips were caught by hand before the first checker run (a first note gave `CMPI` the decle `03F8`, which the family table says is `XORI`, so `0378` was used, and a branch annotation followed by a comma, which the annotation rule reads as no annotation). The `BITS` column was produced by a scratch formatter from the hand-typed words, so the rule `listing-bits-mismatch` was not exercised; the words, rows, operand lines, displacements and `-> ADDR` annotations were typed by hand.
- Errors found only by a run (escapes): 0. The first run through `ivs-run` matched both hand-derived frame hashes (F-D1 and F-D2) without a change to the listing, a scenario assert or a frame literal. The cycle budget of the derivation was 31 cycles above the observed program without the strobe (82,740 ticks observed for the start of the `HLT` unit against 82,864 derived), a budget inference and not an assembly error; the margins of 4,000 ticks hold with room (23,000 ticks after frame 0 and 36,700 before frame 1 without the strobe, 28,700 and 31,000 with it).
- The crossing rule of the derivation (any one fires it): the listing exceeds 170 words or needs more than 8 branches (133 and 5: not fired); a hand-assembly error survives the checker and is found only by a run (no escape: not fired); a second STIC cartridge is needed (one image serves both modes and every mutant is a one- or two-word patch of it: not fired); a second author edits a listing (no: not fired). The rule is not crossed.
- The finding for the grammar: the checker treats data as program, so a data table costs a row label per decle and may decode as a multi-decle instruction; a directive for data words would remove that, and is the one thing this diagnostic would ask of an assembler.
The finding for the grammar was met by the `DATA` row and the `DECLE` directive above.

Effort data of the assembler (plan 07-04, recorded for the review of this decision at Phase 8):

- Size: `tools/asm/asm.c` is 1,137 lines, above the planned 700 to 900 and below the stop of 1,200; 78 spellings cover 73 of the 74 leaf rows of the family table (all but `MVOI`) and the five jump forms that are row `J`.
- Hand slips found by the twins: 0; encoding errors found only by a run: 0; `asm-imm-sdbd` refusals met while writing a real source: 0 (the three twins that use `SDBD` write it by hand). The pairs corpus found no table error at its first run.
- Cost: the suite `asm` has 45 cases and starts one process per case; 2,000 random mutations of the four twins through the sanitizer build gave 873 accepted and 1,127 refused sources, no crash and no refusal that printed other than one line or left a file.
