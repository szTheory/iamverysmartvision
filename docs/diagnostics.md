# Project diagnostics and the boot stub

This note says what a project diagnostic is, how its source is written, what the project boot stub promises, and the review rule that keeps the stub from growing into a replacement for a system program.
It records decisions D-25 to D-30 of Phase 4.
A label [INF] marks a design reason; every other line is a rule.

## What a project diagnostic is

A project diagnostic is a small guest program written for this project, whose expected end state is derived by hand from the manual tables and arithmetic, never from the emulator under test (D-25, D-27).
Its source of truth is a hand-assembled, annotated text listing: one line per 10-bit word, with the word in hex and in bits, the opcode row it encodes and the instruction text.
No assembler is a build, gate, CI or release dependency, and no code, table or comment of another implementation or assembler is copied into a listing.
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
  - `ROW`: the most specific leaf row id of `data/cp1610/opcode-families.json` for the instruction that starts on this line, or `-` for an operand word of the instruction above it (an instruction of `words` words is followed by `words - 1` operand lines).
  - `ASM`: the rest of the line, at least one token: the instruction text and notes.
- Zero fill: an unreachable zero word carries the row of the decle it encodes (`HLT`) and the text `fill`.

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

The default build of both presets writes `build/<preset>/images/boot-stub.exec` and `build/<preset>/images/diag-cpu-1.ivsc` (CMake variable `IVS_IMAGES_DIR`); the output holds no time, path or host-dependent byte, so both presets produce the same bytes.
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
The risk is bounded by deriving the values from arithmetic and printed tables rather than from a second interpreter, by excluding policy-dependent state, and by keeping the claim on this diagnostic provisional.
The cartridge entry `5000` and the stack start `0340` it relies on are project conventions without a source (C68).

libasm cross-check (D-25, optional and never gating): not run, the tool is absent on the authoring machine; no assembler output was compared with the listing.

Plan 04-20 adds program mutants of this listing through `ivs_pack --patch` (for example `ADDI` to `SUBI` at `5007`, `BNZE` to `BZE` at `501E`, `PSHR` replaced at `5022`, `INCR` to `DECR` at `500B`); each must end as a guest failure that names the predicate clause it breaks.

## When a project assembler becomes worth writing

Listings stay hand-assembled until one of these holds (D-25): more than 3 project diagnostics exist, a listing needs labels or macros, or a second author writes listings.
Then a small project-owned assembler written from the manual is the option to weigh; the asm/program pairs of the Phase 2 vectors are its ready test corpus.
