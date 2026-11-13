# Hand-derived frames F-A1..F-A5, F-B1..F-B3, F-D1 and F-D2

Project-owned derivations of the eight unit frames and the two diagnostic frames of plan 06-01 (D-02, D-18, D-19, D-20); the diagnostic frames F-D1 and F-D2 are the expected pictures of the program derived in `docs/video/diag-stic-1-derivation.md`. Each frame is an expected index plane written from the card rules before any renderer exists: the BACKTAB words are shown in hexadecimal and binary with the fields marked, the card patterns line by line, and the resulting 8 x 8 blocks with the rule ids that decide each field.
The rules are CR-01..CR-14 of `data/stic/rules.json` (word layouts from `data/video/cards.literal.txt`, geometry from `data/video/geometry.literal.txt`); all of them are tier 4 or inference except the printed counts, so the unit frames are `tier source-expectation`, the diagnostic frames `tier original-diagnostic`, and none carries a hardware claim.
The 8 x 8 blocks were expanded into plane text with a throwaway typing aid written from these rules (never committed, never reading the core); every expected value follows from the word fields, the pattern lines and the rules named, and the `video` step re-checks each annotated cell by pixel counts.
The author, the blind readers and the later core share one model family: agreement between a frame and the core is a check of transcription and derivation, not an independent implementation comparison (C43).

## Conventions

- A frame is 12 x 20 cards of 8 x 8 pixels; cell (r, c) is the BACKTAB word at address 0200 + 20 r + c (CR-01). The plane has 96 rows of 160 symbols; the symbol at x = 7 of column 19 is `-` (the cut-off pixel, CR-03, clip side right, C101) and the published plane is the 159 symbols left of it.
- Pattern byte y is pixel row y from the top and bit 7 is the leftmost pixel, 1 = foreground (CR-04, C96); a card with no image (any graphics ROM card here, an unlisted graphics RAM card) is all zero, so it shows only the background (CR-14).
- Foreground/Background word (CR-08): foreground = bits 2..0, card = bits 8..3, select = bit 11 (1 = graphics RAM), background = 8 x bit 12 + 4 x bit 13 + 2 x bit 10 + bit 9. Bits 15 and 14 are not used (CR-02).
- Color Stack word (CR-09, CR-10, CR-11): bits 12 and 11 decide the card kind (0 0 graphics ROM card of bits 10..3 with foreground bits 2..0, 0 1 graphics RAM card of bits 8..3 with foreground bits 2..0, 1 1 graphics RAM card with foreground bit 3 = bit 12). Bit 13 advances the stack position by one (mod 4) before the card is drawn; the background is the stack entry at the position after the advance; the position is 0 at the start of the frame, is never reset by a row and is advanced by a column 19 card too.
- Every annotated cell uses a foreground index different from its background index, so the pixel counts of a cell separate the two colors (the conservation check of the `video` step).

## Card patterns (graphics RAM, project-authored geometric shapes)

Card 1 (stripes), pixel-row bytes top to bottom: 11111111, 00000000, 11111111, 00000000, 11111111, 00000000, 11111111, 00000000; foreground pixels: 32.

Card 2 (box), pixel-row bytes top to bottom: 11111111, 10000001, 10000001, 10000001, 10000001, 10000001, 10000001, 11111111; foreground pixels: 28.

Card 3 (diagonal), pixel-row bytes top to bottom: 10000000, 01000000, 00100000, 00010000, 00001000, 00000100, 00000010, 00000001; foreground pixels: 8.

Card 4 (checker), pixel-row bytes top to bottom: 10101010, 01010101, 10101010, 01010101, 10101010, 01010101, 10101010, 01010101; foreground pixels: 32.

Card 5 (marker), pixel-row bytes top to bottom: 10000000, 01000000, 00000000, 00000010, 00000001, 00111000, 00000000, 11000011; foreground pixels: 11.


## F-A1: all cards blank on one background color

Mode `fgbg`, border 0, stack 0 0 0 0, rules CR-01 CR-03 CR-08 CR-12 CR-14. Unlisted cells carry the fill word 2201 (00 10001000000001).
The fill word has select 0, card 0, foreground 1 and background 5 (bits 12 13 10 9 = 0 1 0 1); the card has no image, so every fill cell is background 5 (CR-08, CR-14).

Resulting plane: every pixel of the 159 x 96 area is color 5. The `sha256-text` key of the frame is the SHA-256 of the 96 lines of 159 symbols (the cut-off symbol dropped, spaces removed, upper-case hex, each line ended by LF).

## F-A2: one asymmetric GRAM card in each corner

Mode `fgbg`, border 0, stack 0 0 0 0, rules CR-01 CR-03 CR-04 CR-08 CR-12 CR-14. Unlisted cells carry the fill word 2600 (00 10011000000000).
The fill word has select 0, card 0, foreground 0 and background 7 (bits 12 13 10 9 = 0 1 1 1); the card has no image, so every fill cell is background 7 (CR-08, CR-14).

| cell | word | bits 15..0 | fields | result |
|------|------|------------|--------|--------|
| (0,0) | 0C29 | 00 00110000101001 | fgbg fg=1 bg=2 gram card=5 | foreground 1, background 2; graphics RAM card 5: 11 foreground pixels shown of 64 drawn |
| (0,19) | 282B | 00 10100000101011 | fgbg fg=3 bg=4 gram card=5 | foreground 3, background 4; graphics RAM card 5: 9 foreground pixels shown of 56 drawn |
| (11,0) | 2C2D | 00 10110000101101 | fgbg fg=5 bg=6 gram card=5 | foreground 5, background 6; graphics RAM card 5: 11 foreground pixels shown of 64 drawn |
| (11,19) | 182F | 00 01100000101111 | fgbg fg=7 bg=8 gram card=5 | foreground 7, background 8; graphics RAM card 5: 9 foreground pixels shown of 56 drawn |

Resulting plane: background 7 everywhere except the four corner cards, each showing the marker pattern in its own colors; in the two column 19 cards the pixel at x = 7 is not displayed, so the marker shows 9 of its 11 foreground pixels (pixel 6 of pattern row 3 is visible, pixel 7 of pattern rows 4 and 7 is cut off). The `sha256-text` key of the frame is the SHA-256 of the 96 lines of 159 symbols (the cut-off symbol dropped, spaces removed, upper-case hex, each line ended by LF).

## F-A3: four patterns with foreground 0..7, then background 0..7

Mode `fgbg`, border 0, stack 0 0 0 0, rules CR-04 CR-06 CR-07 CR-08 CR-14. Unlisted cells carry the fill word 0400 (00 00010000000000).
The fill word has select 0, card 0, foreground 0 and background 2 (bits 12 13 10 9 = 0 0 1 0); the card has no image, so every fill cell is background 2 (CR-08, CR-14).

| cell | word | bits 15..0 | fields | result |
|------|------|------------|--------|--------|
| (5,0) | 1A08 | 00 01101000001000 | fgbg fg=0 bg=9 gram card=1 | foreground 0, background 9; graphics RAM card 1: 32 foreground pixels shown of 64 drawn |
| (5,1) | 1A11 | 00 01101000010001 | fgbg fg=1 bg=9 gram card=2 | foreground 1, background 9; graphics RAM card 2: 28 foreground pixels shown of 64 drawn |
| (5,2) | 1A1A | 00 01101000011010 | fgbg fg=2 bg=9 gram card=3 | foreground 2, background 9; graphics RAM card 3: 8 foreground pixels shown of 64 drawn |
| (5,3) | 1A23 | 00 01101000100011 | fgbg fg=3 bg=9 gram card=4 | foreground 3, background 9; graphics RAM card 4: 32 foreground pixels shown of 64 drawn |
| (5,4) | 1A0C | 00 01101000001100 | fgbg fg=4 bg=9 gram card=1 | foreground 4, background 9; graphics RAM card 1: 32 foreground pixels shown of 64 drawn |
| (5,5) | 1A15 | 00 01101000010101 | fgbg fg=5 bg=9 gram card=2 | foreground 5, background 9; graphics RAM card 2: 28 foreground pixels shown of 64 drawn |
| (5,6) | 1A1E | 00 01101000011110 | fgbg fg=6 bg=9 gram card=3 | foreground 6, background 9; graphics RAM card 3: 8 foreground pixels shown of 64 drawn |
| (5,7) | 1A27 | 00 01101000100111 | fgbg fg=7 bg=9 gram card=4 | foreground 7, background 9; graphics RAM card 4: 32 foreground pixels shown of 64 drawn |
| (6,0) | 080F | 00 00100000001111 | fgbg fg=7 bg=0 gram card=1 | foreground 7, background 0; graphics RAM card 1: 32 foreground pixels shown of 64 drawn |
| (6,1) | 0A17 | 00 00101000010111 | fgbg fg=7 bg=1 gram card=2 | foreground 7, background 1; graphics RAM card 2: 28 foreground pixels shown of 64 drawn |
| (6,2) | 0C1F | 00 00110000011111 | fgbg fg=7 bg=2 gram card=3 | foreground 7, background 2; graphics RAM card 3: 8 foreground pixels shown of 64 drawn |
| (6,3) | 0E27 | 00 00111000100111 | fgbg fg=7 bg=3 gram card=4 | foreground 7, background 3; graphics RAM card 4: 32 foreground pixels shown of 64 drawn |
| (6,4) | 280F | 00 10100000001111 | fgbg fg=7 bg=4 gram card=1 | foreground 7, background 4; graphics RAM card 1: 32 foreground pixels shown of 64 drawn |
| (6,5) | 2A17 | 00 10101000010111 | fgbg fg=7 bg=5 gram card=2 | foreground 7, background 5; graphics RAM card 2: 28 foreground pixels shown of 64 drawn |
| (6,6) | 2C1F | 00 10110000011111 | fgbg fg=7 bg=6 gram card=3 | foreground 7, background 6; graphics RAM card 3: 8 foreground pixels shown of 64 drawn |
| (6,7) | 2E20 | 00 10111000100000 | fgbg fg=0 bg=7 gram card=4 | foreground 0, background 7; graphics RAM card 4: 32 foreground pixels shown of 64 drawn |

Resulting plane: row 5 shows the four patterns twice with foreground 0..7 on background 9 (bits 12 and 9 set); row 6 shows them twice on backgrounds 0..7 (bit 13 set for 4..7) with foreground 7, or 0 over background 7; all other cells are background 2. The `sha256-text` key of the frame is the SHA-256 of the 96 lines of 159 symbols (the cut-off symbol dropped, spaces removed, upper-case hex, each line ended by LF).

## F-A4: background colors 8..15 and the highest card number

Mode `fgbg`, border 0, stack 0 0 0 0, rules CR-02 CR-08 CR-14. Unlisted cells carry the fill word 0200 (00 00001000000000).
The fill word has select 0, card 0, foreground 0 and background 1 (bits 12 13 10 9 = 0 0 0 1); the card has no image, so every fill cell is background 1 (CR-08, CR-14).

| cell | word | bits 15..0 | fields | result |
|------|------|------------|--------|--------|
| (3,0) | 1810 | 00 01100000010000 | fgbg fg=0 bg=8 gram card=2 | foreground 0, background 8; graphics RAM card 2: 28 foreground pixels shown of 64 drawn |
| (3,1) | 1A11 | 00 01101000010001 | fgbg fg=1 bg=9 gram card=2 | foreground 1, background 9; graphics RAM card 2: 28 foreground pixels shown of 64 drawn |
| (3,2) | 1C12 | 00 01110000010010 | fgbg fg=2 bg=10 gram card=2 | foreground 2, background 10; graphics RAM card 2: 28 foreground pixels shown of 64 drawn |
| (3,3) | 1E13 | 00 01111000010011 | fgbg fg=3 bg=11 gram card=2 | foreground 3, background 11; graphics RAM card 2: 28 foreground pixels shown of 64 drawn |
| (3,4) | 3814 | 00 11100000010100 | fgbg fg=4 bg=12 gram card=2 | foreground 4, background 12; graphics RAM card 2: 28 foreground pixels shown of 64 drawn |
| (3,5) | 3A15 | 00 11101000010101 | fgbg fg=5 bg=13 gram card=2 | foreground 5, background 13; graphics RAM card 2: 28 foreground pixels shown of 64 drawn |
| (3,6) | 3C16 | 00 11110000010110 | fgbg fg=6 bg=14 gram card=2 | foreground 6, background 14; graphics RAM card 2: 28 foreground pixels shown of 64 drawn |
| (3,7) | 3E17 | 00 11111000010111 | fgbg fg=7 bg=15 gram card=2 | foreground 7, background 15; graphics RAM card 2: 28 foreground pixels shown of 64 drawn |
| (8,0) | 37FF | 00 11011111111111 | fgbg fg=7 bg=15 grom card=63 | foreground 7, background 15; graphics ROM card 63: 0 foreground pixels shown of 64 drawn |
| (8,1) | 35FE | 00 11010111111110 | fgbg fg=6 bg=14 grom card=63 | foreground 6, background 14; graphics ROM card 63: 0 foreground pixels shown of 64 drawn |
| (8,2) | 33FD | 00 11001111111101 | fgbg fg=5 bg=13 grom card=63 | foreground 5, background 13; graphics ROM card 63: 0 foreground pixels shown of 64 drawn |
| (8,3) | 31FC | 00 11000111111100 | fgbg fg=4 bg=12 grom card=63 | foreground 4, background 12; graphics ROM card 63: 0 foreground pixels shown of 64 drawn |
| (8,4) | 17FB | 00 01011111111011 | fgbg fg=3 bg=11 grom card=63 | foreground 3, background 11; graphics ROM card 63: 0 foreground pixels shown of 64 drawn |
| (8,5) | 15FA | 00 01010111111010 | fgbg fg=2 bg=10 grom card=63 | foreground 2, background 10; graphics ROM card 63: 0 foreground pixels shown of 64 drawn |
| (8,6) | 13F9 | 00 01001111111001 | fgbg fg=1 bg=9 grom card=63 | foreground 1, background 9; graphics ROM card 63: 0 foreground pixels shown of 64 drawn |
| (8,7) | 11F8 | 00 01000111111000 | fgbg fg=0 bg=8 grom card=63 | foreground 0, background 8; graphics ROM card 63: 0 foreground pixels shown of 64 drawn |

Resulting plane: row 3 shows the box pattern with foreground k over background 8 + k (k = 0..7); row 8 shows eight graphics ROM cards numbered 63 with backgrounds 15 down to 8 and no foreground pixel; all other cells are background 1. The `sha256-text` key of the frame is the SHA-256 of the 96 lines of 159 symbols (the cut-off symbol dropped, spaces removed, upper-case hex, each line ended by LF).

## F-A5: cards with no image are background only

Mode `fgbg`, border 0, stack 0 0 0 0, rules CR-05 CR-06 CR-08 CR-14. Unlisted cells carry the fill word 1600 (00 01011000000000).
The fill word has select 0, card 0, foreground 0 and background 11 (bits 12 13 10 9 = 1 0 1 1); the card has no image, so every fill cell is background 11 (CR-08, CR-14).

| cell | word | bits 15..0 | fields | result |
|------|------|------------|--------|--------|
| (2,0) | 240B | 00 10010000001011 | fgbg fg=3 bg=6 grom card=1 | foreground 3, background 6; graphics ROM card 1: 0 foreground pixels shown of 64 drawn |
| (2,1) | 2413 | 00 10010000010011 | fgbg fg=3 bg=6 grom card=2 | foreground 3, background 6; graphics ROM card 2: 0 foreground pixels shown of 64 drawn |
| (2,2) | 2423 | 00 10010000100011 | fgbg fg=3 bg=6 grom card=4 | foreground 3, background 6; graphics ROM card 4: 0 foreground pixels shown of 64 drawn |
| (2,3) | 2443 | 00 10010001000011 | fgbg fg=3 bg=6 grom card=8 | foreground 3, background 6; graphics ROM card 8: 0 foreground pixels shown of 64 drawn |
| (2,4) | 2483 | 00 10010010000011 | fgbg fg=3 bg=6 grom card=16 | foreground 3, background 6; graphics ROM card 16: 0 foreground pixels shown of 64 drawn |
| (2,5) | 2503 | 00 10010100000011 | fgbg fg=3 bg=6 grom card=32 | foreground 3, background 6; graphics ROM card 32: 0 foreground pixels shown of 64 drawn |
| (2,6) | 25FB | 00 10010111111011 | fgbg fg=3 bg=6 grom card=63 | foreground 3, background 6; graphics ROM card 63: 0 foreground pixels shown of 64 drawn |
| (2,7) | 2C0B | 00 10110000001011 | fgbg fg=3 bg=6 gram card=1 | foreground 3, background 6; graphics RAM card 1: 32 foreground pixels shown of 64 drawn |
| (2,8) | 2DFB | 00 10110111111011 | fgbg fg=3 bg=6 gram card=63 | foreground 3, background 6; graphics RAM card 63: 0 foreground pixels shown of 64 drawn |

Resulting plane: cells (2,0)..(2,6) are graphics ROM cards 1, 2, 4, 8, 16, 32, 63 over background 6 with foreground 3 and show no foreground pixel; (2,7) is graphics RAM card 1 (stripes) with the same colors and shows 32 foreground pixels, and (2,8) is the unlisted graphics RAM card 63 and shows none; all other cells are background 11. The `sha256-text` key of the frame is the SHA-256 of the 96 lines of 159 symbols (the cut-off symbol dropped, spaces removed, upper-case hex, each line ended by LF).

## F-B1: stack entry 0 is the background of every card

Mode `cs`, border 0, stack 2 5 9 F, rules CR-09 CR-10 CR-11 CR-14. Unlisted cells carry the fill word 0000 (00 00000000000000).
The fill word has bits 13 12 11 = 0 0 0, a graphics ROM card 0 with foreground 0 and no advance; the card has no image, so each unlisted cell shows the stack entry at the current position (CR-09, CR-10).

| cell | word | bits 15..0 | fields | result |
|------|------|------------|--------|--------|
| (2,0) | 000B | 00 00000000001011 | cs adv=0 fg=3 grom card=1 bg=2 | foreground 3, background 2, position 0; graphics ROM card 1: 0 foreground pixels shown of 64 drawn |
| (2,1) | 0014 | 00 00000000010100 | cs adv=0 fg=4 grom card=2 bg=2 | foreground 4, background 2, position 0; graphics ROM card 2: 0 foreground pixels shown of 64 drawn |
| (2,2) | 001D | 00 00000000011101 | cs adv=0 fg=5 grom card=3 bg=2 | foreground 5, background 2, position 0; graphics ROM card 3: 0 foreground pixels shown of 64 drawn |
| (2,3) | 0026 | 00 00000000100110 | cs adv=0 fg=6 grom card=4 bg=2 | foreground 6, background 2, position 0; graphics ROM card 4: 0 foreground pixels shown of 64 drawn |
| (2,4) | 002F | 00 00000000101111 | cs adv=0 fg=7 grom card=5 bg=2 | foreground 7, background 2, position 0; graphics ROM card 5: 0 foreground pixels shown of 64 drawn |
| (2,5) | 0030 | 00 00000000110000 | cs adv=0 fg=0 grom card=6 bg=2 | foreground 0, background 2, position 0; graphics ROM card 6: 0 foreground pixels shown of 64 drawn |
| (2,6) | 0039 | 00 00000000111001 | cs adv=0 fg=1 grom card=7 bg=2 | foreground 1, background 2, position 0; graphics ROM card 7: 0 foreground pixels shown of 64 drawn |
| (2,7) | 0047 | 00 00000001000111 | cs adv=0 fg=7 grom card=8 bg=2 | foreground 7, background 2, position 0; graphics ROM card 8: 0 foreground pixels shown of 64 drawn |
| (4,0) | 1808 | 00 01100000001000 | cs adv=0 fg=8 gram card=1 bg=2 | foreground 8, background 2, position 0; graphics RAM card 1: 32 foreground pixels shown of 64 drawn |
| (4,1) | 1811 | 00 01100000010001 | cs adv=0 fg=9 gram card=2 bg=2 | foreground 9, background 2, position 0; graphics RAM card 2: 28 foreground pixels shown of 64 drawn |
| (4,2) | 181A | 00 01100000011010 | cs adv=0 fg=10 gram card=3 bg=2 | foreground 10, background 2, position 0; graphics RAM card 3: 8 foreground pixels shown of 64 drawn |
| (4,3) | 1823 | 00 01100000100011 | cs adv=0 fg=11 gram card=4 bg=2 | foreground 11, background 2, position 0; graphics RAM card 4: 32 foreground pixels shown of 64 drawn |
| (4,4) | 180C | 00 01100000001100 | cs adv=0 fg=12 gram card=1 bg=2 | foreground 12, background 2, position 0; graphics RAM card 1: 32 foreground pixels shown of 64 drawn |
| (4,5) | 1815 | 00 01100000010101 | cs adv=0 fg=13 gram card=2 bg=2 | foreground 13, background 2, position 0; graphics RAM card 2: 28 foreground pixels shown of 64 drawn |
| (4,6) | 181E | 00 01100000011110 | cs adv=0 fg=14 gram card=3 bg=2 | foreground 14, background 2, position 0; graphics RAM card 3: 8 foreground pixels shown of 64 drawn |
| (4,7) | 1827 | 00 01100000100111 | cs adv=0 fg=15 gram card=4 bg=2 | foreground 15, background 2, position 0; graphics RAM card 4: 32 foreground pixels shown of 64 drawn |

Stack walk from position 0 (color 2): no advance. Cells after the last advance keep position 0 (color 2) through the end of the frame.

Resulting plane: every cell is background 2 (stack entry 0, no advance); row 2 shows eight graphics ROM cards with no image (foreground 0..7 unseen), row 4 shows four patterns twice in foreground 8..15 (the foreground bit 3 of the graphics RAM word). The `sha256-text` key of the frame is the SHA-256 of the 96 lines of 159 symbols (the cut-off symbol dropped, spaces removed, upper-case hex, each line ended by LF).

## F-B2: advance before draw, wrap 3 to 0, no row reset, cut-off column advance

Mode `cs`, border 0, stack 2 5 9 F, rules CR-09 CR-10 CR-11 CR-14. Unlisted cells carry the fill word 0000 (00 00000000000000).
The fill word has bits 13 12 11 = 0 0 0, a graphics ROM card 0 with foreground 0 and no advance; the card has no image, so each unlisted cell shows the stack entry at the current position (CR-09, CR-10).

| cell | word | bits 15..0 | fields | result |
|------|------|------------|--------|--------|
| (7,2) | 2819 | 00 10100000011001 | cs adv=1 fg=1 gram card=3 bg=5 | foreground 1, background 5, position 1; graphics RAM card 3: 8 foreground pixels shown of 64 drawn |
| (7,5) | 2813 | 00 10100000010011 | cs adv=1 fg=3 gram card=2 bg=9 | foreground 3, background 9, position 2; graphics RAM card 2: 28 foreground pixels shown of 64 drawn |
| (7,6) | 281D | 00 10100000011101 | cs adv=1 fg=5 gram card=3 bg=15 | foreground 5, background 15, position 3; graphics RAM card 3: 8 foreground pixels shown of 64 drawn |
| (7,9) | 2817 | 00 10100000010111 | cs adv=1 fg=7 gram card=2 bg=2 | foreground 7, background 2, position 0; graphics RAM card 2: 28 foreground pixels shown of 64 drawn |
| (7,12) | 3809 | 00 11100000001001 | cs adv=1 fg=9 gram card=1 bg=5 | foreground 9, background 5, position 1; graphics RAM card 1: 32 foreground pixels shown of 64 drawn |
| (9,19) | 282E | 00 10100000101110 | cs adv=1 fg=6 gram card=5 bg=9 | foreground 6, background 9, position 2; graphics RAM card 5: 9 foreground pixels shown of 56 drawn |
| (10,0) | 0814 | 00 00100000010100 | cs adv=0 fg=4 gram card=2 bg=9 | foreground 4, background 9, position 2; graphics RAM card 2: 28 foreground pixels shown of 64 drawn |

Stack walk from position 0 (color 2): (7,2) to position 1 (color 5); (7,5) to position 2 (color 9); (7,6) to position 3 (color F); (7,9) to position 0 (color 2); (7,12) to position 1 (color 5); (9,19) to position 2 (color 9). Cells after the last advance keep position 2 (color 9) through the end of the frame.

Resulting plane: cells before (7,2) use color 2, then the background steps 5, 9, F, 2, 5 at (7,2), (7,5), (7,6), (7,9), (7,12) (wrap at the fourth advance); row 8 onward keeps color 5 (no per-row reset); the advance at (9,19) moves to color 9 and cell (10,0) and every later cell show 9. The `sha256-text` key of the frame is the SHA-256 of the 96 lines of 159 symbols (the cut-off symbol dropped, spaces removed, upper-case hex, each line ended by LF).

## F-B3: the BACKTAB words of F-A3 reinterpreted in Color Stack mode

Mode `cs`, border 0, stack 3 6 A D, rules CR-02 CR-06 CR-09 CR-10 CR-11 CR-14. Unlisted cells carry the fill word 0400 (00 00010000000000).
The fill word has bits 13 12 11 = 0 0 0, a graphics ROM card 128 with foreground 0 and no advance; the card has no image, so each unlisted cell shows the stack entry at the current position (CR-09, CR-10).

| cell | word | bits 15..0 | fields | result |
|------|------|------------|--------|--------|
| (5,0) | 1A08 | 00 01101000001000 | cs adv=0 fg=8 gram card=1 bg=3 | foreground 8, background 3, position 0; graphics RAM card 1: 32 foreground pixels shown of 64 drawn |
| (5,1) | 1A11 | 00 01101000010001 | cs adv=0 fg=9 gram card=2 bg=3 | foreground 9, background 3, position 0; graphics RAM card 2: 28 foreground pixels shown of 64 drawn |
| (5,2) | 1A1A | 00 01101000011010 | cs adv=0 fg=10 gram card=3 bg=3 | foreground 10, background 3, position 0; graphics RAM card 3: 8 foreground pixels shown of 64 drawn |
| (5,3) | 1A23 | 00 01101000100011 | cs adv=0 fg=11 gram card=4 bg=3 | foreground 11, background 3, position 0; graphics RAM card 4: 32 foreground pixels shown of 64 drawn |
| (5,4) | 1A0C | 00 01101000001100 | cs adv=0 fg=12 gram card=1 bg=3 | foreground 12, background 3, position 0; graphics RAM card 1: 32 foreground pixels shown of 64 drawn |
| (5,5) | 1A15 | 00 01101000010101 | cs adv=0 fg=13 gram card=2 bg=3 | foreground 13, background 3, position 0; graphics RAM card 2: 28 foreground pixels shown of 64 drawn |
| (5,6) | 1A1E | 00 01101000011110 | cs adv=0 fg=14 gram card=3 bg=3 | foreground 14, background 3, position 0; graphics RAM card 3: 8 foreground pixels shown of 64 drawn |
| (5,7) | 1A27 | 00 01101000100111 | cs adv=0 fg=15 gram card=4 bg=3 | foreground 15, background 3, position 0; graphics RAM card 4: 32 foreground pixels shown of 64 drawn |
| (6,0) | 080F | 00 00100000001111 | cs adv=0 fg=7 gram card=1 bg=3 | foreground 7, background 3, position 0; graphics RAM card 1: 32 foreground pixels shown of 64 drawn |
| (6,1) | 0A17 | 00 00101000010111 | cs adv=0 fg=7 gram card=2 bg=3 | foreground 7, background 3, position 0; graphics RAM card 2: 28 foreground pixels shown of 64 drawn |
| (6,2) | 0C1F | 00 00110000011111 | cs adv=0 fg=7 gram card=3 bg=3 | foreground 7, background 3, position 0; graphics RAM card 3: 8 foreground pixels shown of 64 drawn |
| (6,3) | 0E27 | 00 00111000100111 | cs adv=0 fg=7 gram card=4 bg=3 | foreground 7, background 3, position 0; graphics RAM card 4: 32 foreground pixels shown of 64 drawn |
| (6,4) | 280F | 00 10100000001111 | cs adv=1 fg=7 gram card=1 bg=6 | foreground 7, background 6, position 1; graphics RAM card 1: 32 foreground pixels shown of 64 drawn |
| (6,5) | 2A17 | 00 10101000010111 | cs adv=1 fg=7 gram card=2 bg=10 | foreground 7, background 10, position 2; graphics RAM card 2: 28 foreground pixels shown of 64 drawn |
| (6,6) | 2C1F | 00 10110000011111 | cs adv=1 fg=7 gram card=3 bg=13 | foreground 7, background 13, position 3; graphics RAM card 3: 8 foreground pixels shown of 64 drawn |
| (6,7) | 2E20 | 00 10111000100000 | cs adv=1 fg=0 gram card=4 bg=3 | foreground 0, background 3, position 0; graphics RAM card 4: 32 foreground pixels shown of 64 drawn |

Stack walk from position 0 (color 3): (6,4) to position 1 (color 6); (6,5) to position 2 (color A); (6,6) to position 3 (color D); (6,7) to position 0 (color 3). Cells after the last advance keep position 0 (color 3) through the end of the frame.

Resulting plane: the words of F-A3 in Color Stack mode: row 5 words (bits 13 12 11 = 0 1 1 plus bit 9 ignored) are graphics RAM cards with foreground 8 + k and no advance, so they show stack entry 0; row 6 words with bit 13 set (originally backgrounds 4..7) advance the stack at (6,4), (6,5), (6,6), (6,7); all fill words (bit 10 set only) are graphics ROM card 0 without advance and show the entry at the current position. The `sha256-text` key of the frame is the SHA-256 of the 96 lines of 159 symbols (the cut-off symbol dropped, spaces removed, upper-case hex, each line ended by LF).

## F-D1: ivs-diag-stic-1, Foreground/Background path

Mode `fgbg`, border 0, stack 6 A 3 D, rules CR-01 CR-02 CR-03 CR-04 CR-06 CR-08 CR-14 STIC-R13. Unlisted cells carry the fill word 0000 (00 00000000000000).
The fill word has select 0, card 0, foreground 0 and background 0 (bits 12 13 10 9 = 0 0 0 0); the card has no image, so every fill cell is background 0 (CR-08, CR-14).

| cell | word | bits 15..0 | fields | result |
|------|------|------------|--------|--------|
| (8,3) | 2A01 | 00 10101000000001 | fgbg fg=1 bg=5 gram card=0 | foreground 1, background 5; graphics RAM card 0: 32 foreground pixels shown of 64 drawn |
| (8,4) | 0E0A | 00 00111000001010 | fgbg fg=2 bg=3 gram card=1 | foreground 2, background 3; graphics RAM card 1: 28 foreground pixels shown of 64 drawn |
| (8,5) | 1A13 | 00 01101000010011 | fgbg fg=3 bg=9 gram card=2 | foreground 3, background 9; graphics RAM card 2: 8 foreground pixels shown of 64 drawn |
| (8,6) | 381C | 00 11100000011100 | fgbg fg=4 bg=12 gram card=3 | foreground 4, background 12; graphics RAM card 3: 32 foreground pixels shown of 64 drawn |
| (11,0) | 2C1D | 00 10110000011101 | fgbg fg=5 bg=6 gram card=3 | foreground 5, background 6; graphics RAM card 3: 32 foreground pixels shown of 64 drawn |
| (11,19) | 1C0F | 00 01110000001111 | fgbg fg=7 bg=10 gram card=1 | foreground 7, background 10; graphics RAM card 1: 20 foreground pixels shown of 56 drawn |

Resulting plane: background 0 everywhere except the six cells: the words are the six explicit BACKTAB words of the program read in Foreground/Background mode, each a graphics RAM card 0..3 (stripes, box, diagonal, checker) in the colors of its fields; the column 19 box card shows its 7 leftmost pixel columns. The `sha256-text` key of the frame is the SHA-256 of the 96 lines of 159 symbols (the cut-off symbol dropped, spaces removed, upper-case hex, each line ended by LF).

## F-D2: ivs-diag-stic-1, Color Stack path

Mode `cs`, border 0, stack 6 A 3 D, rules CR-01 CR-02 CR-06 CR-09 CR-10 CR-11 CR-14 STIC-R13 STIC-R14. Unlisted cells carry the fill word 0000 (00 00000000000000).
The fill word has bits 13 12 11 = 0 0 0, a graphics ROM card 0 with foreground 0 and no advance; the card has no image, so each unlisted cell shows the stack entry at the current position (CR-09, CR-10).

| cell | word | bits 15..0 | fields | result |
|------|------|------------|--------|--------|
| (8,3) | 2A01 | 00 10101000000001 | cs adv=1 fg=1 gram card=0 bg=10 | foreground 1, background 10, position 1; graphics RAM card 0: 32 foreground pixels shown of 64 drawn |
| (8,4) | 0E0A | 00 00111000001010 | cs adv=0 fg=2 gram card=1 bg=10 | foreground 2, background 10, position 1; graphics RAM card 1: 28 foreground pixels shown of 64 drawn |
| (8,5) | 1A13 | 00 01101000010011 | cs adv=0 fg=11 gram card=2 bg=10 | foreground 11, background 10, position 1; graphics RAM card 2: 8 foreground pixels shown of 64 drawn |
| (8,6) | 381C | 00 11100000011100 | cs adv=1 fg=12 gram card=3 bg=3 | foreground 12, background 3, position 2; graphics RAM card 3: 32 foreground pixels shown of 64 drawn |
| (11,0) | 2C1D | 00 10110000011101 | cs adv=1 fg=5 gram card=3 bg=13 | foreground 5, background 13, position 3; graphics RAM card 3: 32 foreground pixels shown of 64 drawn |
| (11,19) | 1C0F | 00 01110000001111 | cs adv=0 fg=15 gram card=1 bg=13 | foreground 15, background 13, position 3; graphics RAM card 1: 20 foreground pixels shown of 56 drawn |

Stack walk from position 0 (color 6): (8,3) to position 1 (color A); (8,6) to position 2 (color 3); (11,0) to position 3 (color D). Cells after the last advance keep position 3 (color D) through the end of the frame.

Resulting plane: the same BACKTAB words read in Color Stack mode with the stack 6, A, 3, D written by the program: the fill cells show the current stack entry, the words with bit 13 set advance the stack (positions 1 at (8,3), 2 at (8,6), 3 at (11,0)), and the bits 12 and 9 of the Foreground/Background reading become foreground bit 3 and an ignored bit. The `sha256-text` key of the frame is the SHA-256 of the 96 lines of 159 symbols (the cut-off symbol dropped, spaces removed, upper-case hex, each line ended by LF).


## Colored Squares: the documented cut line (a fallback that was not applied)

Plan 06-11 implements Colored Squares (CR-13, C98): four 4 x 4 quadrants with colors c0 = bits 2..0, c1 = bits 5..3, c2 = bits 8..6 and c3 = 4 x bit 13 + 2 x bit 10 + bit 9, the color 7 drawing the current stack entry without an advance. If this decode ever had to be withdrawn, the cut line is: a squares card (bit 12 set, bit 11 clear) would render as 64 pixels of the current stack entry with no advance, and the following would move to Phase 8 with their evidence: the squares cells of the constructed frames, the eight probe rows of the cs-sq variant that name square colors (bits 13 and 10 to 0 of `data/video/cards.literal.txt`), and the tests `video.cstack.squares-quadrants`, `video.cstack.squares-stack7` and `video.probe.bit-roles-cs-sq`. The fallback was NOT applied: no frame literal F-A1..F-B3 uses a squares card, so the decode is exercised only by the constructed cells of those tests, the decode test `video.card.cs-decode` and the probes, and its layout stays a provisional default of one community lineage.
