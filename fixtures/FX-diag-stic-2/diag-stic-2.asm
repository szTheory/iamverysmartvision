# id: ivs-diag-stic-2
# kind: cart
# profile: intv-2609-ntsc-family-p1
# window: 5000-5361
# label: ivs-diag-stic-2, the single-feature diagnostic of moving objects, collision, delay and border extension
# entry: 5000
# stack: 0340
; ivs-diag-stic-2 (plan 08-07, D-04, D-05, D-10, D-43). Source of truth of the listing
; fixtures/FX-diag-stic-2/diag-stic-2.listing.txt, which ivs_asm generates from this file; the listing is
; committed unedited and a suite test regenerates it and requires byte equality. Project-authored under
; the repository MIT licence. It is written from the specification in docs/video/diag-stic-2-derivation.md,
; section (a) (A1 to A11, with the corrected A5), and from nothing else: no byte, glyph, tune or routine of
; another program, no EXEC call, no graphics ROM read, no text, and it never halts (no HLT anywhere).
;
; One program, one feature per run: the scenario word c0, read once at $01FE in the initialization, selects
; the scene sequence (A1). Selector low byte: FF idle, 7E mob, BE border, DE hdelay, 7D vdelay, BD
; collision; any other value behaves as FF. Entered through the project boot stub, contract 2
; (docs/diagnostics.md): reset at ENTRY, every interrupt at ISR with the interrupt system off and one
; return address on the stack. Section letters A1 to A11 below are those of the derivation (a).
;
; Variables (A4), each a 16-bit word: $02F0 init marker, $02F1 frame counter, $02F2 selector, $02F3 scene
; number of the last routine, $0300 to $0307 and $0308 to $030F the collision read-outs of frames 3 and 4.
; The frame counter relies on the zero power-on RAM of the profile (the initialization does not clear it:
; A5 clears the BACKTAB only).
;
; Timing (A9; frame cycles, 14,934 per frame, measured on the program as built and recorded in section (k) of
; the derivation): the strobe is stamped before cycle 30; the register work of the longest routine (selector
; BD, frame 3) ends at cycle 1,634, under the bound 1,900 of A9 and the window close at 2,000; the EIS of the
; re-arm comes between cycles 9,226 and 9,934, inside the band 3,777 to 14,930.

ENTRY     EQU   $5000          ; cartridge reset entry (stub contract 2)
ISR       EQU   $5004          ; cartridge interrupt entry (stub contract 2)

PORT0     EQU   $01FE          ; A1: the port of the scenario word c0
COLL0     EQU   $0018          ; A6 (5): collision register of MOB 0; C0 to C7 are $0018 to $001F
STROBE    EQU   $0020          ; A6 (1): display strobe, a write of any value
MODESEL   EQU   $0021          ; A6 (2): a read selects Color Stack mode
CSTACK    EQU   $0028          ; A3: color stack registers $0028 to $002B
BORDER    EQU   $002C          ; A3: border color register
DELAYREG  EQU   $0030          ; A6 (7): horizontal delay; $0031 vertical delay, $0032 border extension
V_MARK    EQU   $02F0          ; A4: init marker
V_FRAME   EQU   $02F1          ; A4: frame counter
V_SEL     EQU   $02F2          ; A4: selector word
V_SCENE   EQU   $02F3          ; A4: scene number of the last routine
RD3       EQU   $0300          ; A6 (5): read-outs of frame 3, $0300 plus the register number
RD4       EQU   $0308          ; A6 (5): read-outs of frame 4, $0308 plus the register number
BACKTAB   EQU   $0200          ; A3: first BACKTAB word
CELL_A    EQU   $0282          ; A3: BACKTAB cell row 6, column 10
CELL_B    EQU   $0285          ; A3: BACKTAB cell row 6, column 13 (selector BD)
CELL_C    EQU   $0288          ; A3: BACKTAB cell row 6, column 16 (selector BD)
CELL_D    EQU   $02BE          ; A3: BACKTAB cell row 9, column 10 (selector BD)
GRAM0     EQU   $3800          ; A2: graphics RAM, card 0
MARKER    EQU   $03C5          ; A4: the init marker value, written last by the initialization
DELAY     EQU   500            ; A6 (8): the counted wait of the re-arm, N iterations of 15 cycles

; Reset entry: skip the interrupt entry words. Words 5002 and 5003 are zero fill, never executed.
        ORG   ENTRY
        B     init

; The interrupt service routine (A6), entered with interrupts off. Registers hold no state between
; interrupts, so none is saved. R0 holds k (the frame number) from step 3 on, R1 the selector, R2 the scene
; number, R3 the scene table, R4 and R5 the table and register pointers of the stores.
        ORG   ISR
; (1) the display strobe is the first action
        MVO   R0, STROBE       ; the value written is ignored
; (2) the mode select read; Color Stack mode
        MVI   MODESEL, R0
; (3) frame counter: R0 = k after the increment
        MVI   V_FRAME, R0
        INCR  R0
        MVO   R0, V_FRAME
; (4) the scene number s (R2) and its table (R3) from the selector and k, s stored at V_SCENE
        MVI   V_SEL, R1
        CMPI  #$007E, R1
        BZE   s_mob
        CMPI  #$00BE, R1
        BZE   s_border
        CMPI  #$00DE, R1
        BZE   s_hdelay
        CMPI  #$007D, R1
        BZE   s_vdelay
        CMPI  #$00BD, R1
        BZE   s_coll
        CLRR  R2               ; any other value behaves as FF: scene 0
        SDBD
        MVII  #scene0, R3
        B     chosen
s_mob:  MVII  #1, R2           ; scene 1 in frames 1 to 3
        SDBD
        MVII  #scene1, R3
        CMPI  #4, R0
        BLT   chosen
        MVII  #2, R2           ; scene 2 in frames 4 to 6
        SDBD
        MVII  #scene2, R3
        CMPI  #7, R0
        BLT   chosen
        MVII  #3, R2           ; scene 3 in frame 7 and later
        SDBD
        MVII  #scene3, R3
        B     chosen
s_border: MVII #4, R2          ; scene 4
        SDBD
        MVII  #scene4, R3
        B     chosen
s_hdelay: MVII #5, R2          ; scene 5
        SDBD
        MVII  #scene5, R3
        B     chosen
s_vdelay: MVII #6, R2          ; scene 6
        SDBD
        MVII  #scene6, R3
        B     chosen
s_coll: MVII  #7, R2           ; scene 7 in frame 1
        SDBD
        MVII  #scene7, R3
        CMPI  #2, R0
        BLT   chosen
        MVII  #8, R2           ; scene 8 in frame 2
        SDBD
        MVII  #scene8, R3
        BZE   chosen           ; the flags are those of the CMPI #2 above: MVII and SDBD leave them
        MVII  #9, R2           ; scene 9 in frame 3 and later
        SDBD
        MVII  #scene9, R3
chosen: MVO   R2, V_SCENE
; (5) only the selector BD: the collision register work of frames 1, 3 and 4 (R1 is still the selector)
        CMPI  #$00BD, R1
        BZE   coll
; (6) every selector except BD: the color stack registers and the border color (A3)
        MVII  #1, R0
        MVO   R0, CSTACK
        MVII  #2, R0
        MVO   R0, CSTACK+1
        MVII  #3, R0
        MVO   R0, CSTACK+2
        MVII  #4, R0
        MVO   R0, CSTACK+3
        MVII  #8, R0
        MVO   R0, BORDER
        B     words
; (5) in detail: frame 1 clears C0 to C7; frame 3 reads them into $0300 to $0307, ANDed with 03FF, then
; clears them; frame 4 reads them into $0308 to $030F and writes nothing; every other frame does nothing here
coll:   CMPI  #1, R0
        BZE   clear8
        CMPI  #3, R0
        BZE   read3
        CMPI  #4, R0
        BZE   read4
        B     words
read3:  MVII  #COLL0, R4       ; frame 3: C0 to C7 into $0300 to $0307
        MVII  #RD3, R5
        MVII  #8, R1
rd3:    MVI@  R4, R0
        ANDI  #$03FF, R0
        MVO@  R0, R5
        DECR  R1
        BNZE  rd3
clear8: MVII  #COLL0, R4       ; frames 1 and 3: write 0000 to C0 to C7
        CLRR  R0
        MVII  #8, R1
clr:    MVO@  R0, R4
        DECR  R1
        BNZE  clr
        B     words
read4:  MVII  #COLL0, R4       ; frame 4: C0 to C7 into $0308 to $030F, no write to the registers
        MVII  #RD4, R5
        MVII  #8, R1
rd4:    MVI@  R4, R0
        ANDI  #$03FF, R0
        MVO@  R0, R5
        DECR  R1
        BNZE  rd4
; (7) the 27 scene words of the scene table: MOB X $0000 to $0007, Y $0008 to $000F, attribute $0010 to
; $0017 (24 words in eight groups of three), then $0030, $0031, $0032 (horizontal delay, vertical delay,
; border extension). Each word is two table decles, the low byte first, read with SDBD
words:  MOVR  R3, R4
        CLRR  R5
        MVII  #8, R1
group:  SDBD
        MVI@  R4, R0
        MVO@  R0, R5
        SDBD
        MVI@  R4, R0
        MVO@  R0, R5
        SDBD
        MVI@  R4, R0
        MVO@  R0, R5
        DECR  R1
        BNZE  group
        MVII  #DELAYREG, R5
        SDBD
        MVI@  R4, R0
        MVO@  R0, R5
        SDBD
        MVI@  R4, R0
        MVO@  R0, R5
        SDBD
        MVI@  R4, R0
        MVO@  R0, R5
; (8) re-arm: drop the saved return address, go to the counted wait
rearm:  DECR  R6
        J     wait

; A5 initialization (frame 0), in this order. The first EIS (label arm) is reached after frame-0 cycle
; 3,777 (the clear of the BACKTAB alone takes about 5,800 cycles), so the first routine is the routine of
; frame 1 and frame 0 is not displayed.
; (1) the three graphics RAM cards, 24 bytes from the table at cards to GRAM0 (A2)
init:   SDBD
        MVII  #GRAM0, R4
        SDBD
        MVII  #cards, R5
        MVII  #24, R1
card:   MVI@  R5, R0
        MVO@  R0, R4
        DECR  R1
        BNZE  card
; (2) read port 0, keep the low byte, store it as the selector
        MVI   PORT0, R0
        ANDI  #$00FF, R0
        MVO   R0, V_SEL
; (3) clear the 240 BACKTAB words $0200 to $02EF (the variables at $02F0 and up are not cleared)
        MVII  #BACKTAB, R4
        CLRR  R0
        MVII  #240, R1
clear:  MVO@  R0, R4
        DECR  R1
        BNZE  clear
; (4) the BACKTAB words of A3: 0812 at row 6, column 10; for the selector BD also the three Colored Squares
; words
        SDBD
        MVII  #$0812, R0
        MVO   R0, CELL_A
        MVI   V_SEL, R1
        CMPI  #$00BD, R1
        BNZE  fin
        SDBD
        MVII  #$16DB, R0
        MVO   R0, CELL_B
        SDBD
        MVII  #$1000, R0
        MVO   R0, CELL_C
        SDBD
        MVII  #$37FF, R0
        MVO   R0, CELL_D
; (5) the marker is the last store of the initialization
fin:    MVII  #MARKER, R0
        MVO   R0, V_MARK
; (6) enable interrupts and idle: jump to the EIS of the wait
        B     arm

; The wait of the re-arm (A6 (8)): N iterations, then EIS and the idle loop. N = 500 puts the EIS far
; inside the band 3,777 to 14,930 of every frame (derivation (b), the re-arm band).
wait:   MVII  #DELAY, R1
dly:    DECR  R1
        BNZE  dly
arm:    EIS
idle:   B     idle             ; the only code that runs between interrupts

; The authored graphics RAM cards (A2), one byte per decle: card 0 solid, card 1 a staircase, card 2 a
; block of 5 x 5 set pixels in its top left corner. Geometric shapes written here, no letter shape.
cards:  DECLE $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        DECLE $80, $C0, $E0, $F0, $F8, $FC, $FE, $FF
        DECLE $F8, $F8, $F8, $F8, $F8, $00, $00, $00

; The ten scene tables (A7). Per table: MOB X words 0 to 7, MOB Y words 0 to 7, MOB attribute words 0 to 7,
; then the horizontal delay, vertical delay and border extension words; each word is its low byte then its
; high byte, one byte per decle.
; Scene 0 (selector FF): every word 0000.
scene0: DECLE 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        DECLE 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        DECLE 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        DECLE 0, 0, 0, 0, 0, 0
; Scene 1 (mob, frames 1 to 3): MOB0 069E 0162 080C.
scene1: DECLE $9E, $06, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        DECLE $62, $01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        DECLE $0C, $08, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        DECLE 0, 0, 0, 0, 0, 0
; Scene 2 (mob, frames 4 to 6): MOB0 0620 0620 080C; MOB1 0648 0A20 080D; MOB2 0670 0E20 080E.
scene2: DECLE $20, $06, $48, $06, $70, $06, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        DECLE $20, $06, $20, $0A, $20, $0E, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        DECLE $0C, $08, $0D, $08, $0E, $08, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        DECLE 0, 0, 0, 0, 0, 0
; Scene 3 (mob, frame 7 and later): MOB0 021C 031C 080C; MOB1 0620 0224 0805; MOB2 0654 0138 1809.
scene3: DECLE $1C, $02, $20, $06, $54, $06, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        DECLE $1C, $03, $24, $02, $38, $01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        DECLE $0C, $08, $05, $08, $09, $18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        DECLE 0, 0, 0, 0, 0, 0
; Scene 4 (border): MOB0 060C 010C 080C; $0032 = 0003.
scene4: DECLE $0C, $06, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        DECLE $0C, $01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        DECLE $0C, $08, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        DECLE 0, 0, 0, 0, $03, 0
; Scene 5 (hdelay): MOB0 0644 0130 080C; $0030 = 0003; $0032 = 0003.
scene5: DECLE $44, $06, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        DECLE $30, $01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        DECLE $0C, $08, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        DECLE $03, 0, 0, 0, $03, 0
; Scene 6 (vdelay): MOB0 0644 0130 080C; $0031 = 0003; $0032 = 0003.
scene6: DECLE $44, $06, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        DECLE $30, $01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        DECLE $0C, $08, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        DECLE 0, 0, $03, 0, $03, 0
; Scene 7 (collision, frame 1): the overlaps of MOB0 with MOB1, MOB2 with MOB3, MOB4 with MOB5, and MOB6 over
; the set pixels of cell (6, 10).
scene7: DECLE $26, $07, $2E, $07, $58, $05, $60, $07, $80, $07, $88, $06, $50, $07, $20, $07
        DECLE $1C, $01, $20, $01, $1C, $01, $20, $01, $1C, $01, $20, $01, $34, $01, $44, $01
        DECLE $04, $08, $05, $08, $06, $08, $07, $08, $01, $18, $02, $18, $03, $18, $04, $18
        DECLE 0, 0, 0, 0, 0, 0
; Scene 8 (collision, frame 2): the eight MOBs two columns apart in the bottom row, no overlap.
scene8: DECLE $0A, $07, $1C, $07, $2E, $07, $40, $07, $52, $07, $64, $07, $76, $07, $88, $07
        DECLE $5C, $01, $5C, $01, $5C, $01, $5C, $01, $5C, $01, $5C, $01, $5C, $01, $5C, $01
        DECLE $04, $08, $05, $08, $06, $08, $07, $08, $01, $18, $02, $18, $03, $18, $04, $18
        DECLE 0, 0, 0, 0, 0, 0
; Scene 9 (collision, frame 3 and later): each MOB alone, over fill, a solid block or a color block.
scene9: DECLE $12, $03, $70, $03, $88, $03, $58, $03, $26, $03, $58, $02, $3A, $02, $80, $03
        DECLE $5C, $01, $38, $01, $38, $01, $50, $01, $5C, $01, $38, $01, $5C, $01, $5C, $01
        DECLE $04, $08, $05, $08, $06, $08, $07, $08, $01, $18, $02, $18, $03, $18, $04, $18
        DECLE 0, 0, 0, 0, 0, 0
