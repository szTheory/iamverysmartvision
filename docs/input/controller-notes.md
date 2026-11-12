# Hand controller notes

Project-owned notes on how the hand controllers reach the guest: the electrical model, what the manufacturer pages say about the ports, and what is still open.
Facts and citations only; no page, table image or wiki text is copied and nothing from these sources is committed (cache only, D-28, rights unknown).
Labels: [OBS] read in plan 05-01, [INF] inferred, [UNK] not established by any source read.

## Electrical model

Each controller is wired to one of the two eight-bit input ports of the sound chip; the PSG registers R16 and R17 (octal numbers, decimal 14 and 15) are the port stores, and the guest reads them at the CPU addresses `$01FE` and `$01FF` (`data/psg/intv-2609-ntsc-family-p1.regmap.literal.txt`) [INF from the register map and the pin page].
Every port pin has an on-chip pull-up, so with nothing pressed every input bit reads 1, and a control works by connecting a set of pins to ground (`SRC-gi-1980-catalog` pdf 205, printed 5-24) [OBS].
The model written in the profile follows: `pins = 0xFF XOR OR(line masks of the pressed controls)`, where a line mask is the set of pins one control grounds, written as an eight-bit mask through the pin-to-bit map `pin 4 = b0, pin 3 = b1, pin 2 = b2, pin 1 = b3, pin 9 = b4, pin 8 = b5, pin 7 = b6, pin 6 = b7` (`SRC-consolemods-intv-pinouts`, controller port table) [OBS].
Grounds do not fight each other, so pressing two controls grounds the union of their pins; this is the `ground-union` line model (tag `inference`).
Whether two controls can close at the same instant, such as two neighbouring disc positions, is a host policy and is not settled by any source (C63).

## What the manufacturer pages say about the ports

- Pull-ups on every pin and the advice to scan switches by grounding the input bit: `SRC-gi-1980-catalog` pdf 205 (printed 5-24) [OBS].
- Direction: register R7 bits B7 and B6 choose the direction of the two ports; the page has no sentence naming the active sense, so 1 = output rests on the printed figure and is an inference (C73) [OBS, UNK].
- Registers numbered in octal: R16 and R17 are decimal 14 and 15, so the ports sit at the last two addresses of the block (C80).
- R16 and R17 are only intermediate stores between the data bus and the pins (pdf 208, printed 5-27) [OBS]; the page does not say what a read returns while a port is an output, or what happens if a switch grounds a driven pin (C65).
- Reset sets all registers to 0 (pdf 205); the page does not say whether the console wires this pin to the system reset (C77).
- Nothing on these pages concerns the 8914-class part directly (C11); the port behaviour above is the 8910 reading applied by inference.

## Where the table comes from

The 31-row control table `data/input/intv-2609-ntsc-family-p1.literal.txt` was transcribed in plan 05-01 from two cached sources before any generator, profile field or planning-note table was opened: the ConsoleMods pin matrix (which pins each of sixteen disc directions, twelve keys and three side buttons grounds, with the pin-to-bit map above) and the key-table image of the community wiki page revision 14808 (the value whose set bits are the bits a press clears).
The two sources agree on every row; the wiki prose gives two checkable anchors (a single press of key 1 reads `$7E`, keys 1 and 9 together read `$5A`).
The two sources are tier 4 and are likely one lineage, so their agreement is a transcription cross-check, not independent hardware evidence (C16).
The disc orientation (direction 1 at the top, numbering clockwise) comes from the wiki image and is the only source of the compass names.
A third source, a 1998 pin chart, was recorded by the Phase 5 discuss but could not be fetched again, so no row cites it.
The two top side buttons read the same code in both sources (C15).

## Open items

| Item | Id | State |
|------|----|-------|
| Byte read from the upper eight bits of a port address | C64 | default 0, no source |
| Read of a port the guest set to output | C65 | default reads the stored latch, no source |
| Which physical connector feeds which port | C66 | ports are named by index, never by player |
| Granularity of an input change against a poll | C67 | applied at a CPU instruction boundary |
| Contention when a switch grounds a driven pin | C65 | not modelled |
| Combinations of disc, keypad and side buttons | C63 | union rule plus labelled policy rows |
| Schematic or measurement of the controller matrix | C14, C16 | missing, handed off as a D10 item |

How an owner records a controller reading is in `docs/input/d10-procedure.md`.
No claim in this project uses measured hardware evidence (D10), and no run with an owner system image was made (D11).
