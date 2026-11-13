# id: ivs-demo-lamp-tone-1
# kind: cart
# profile: intv-2609-ntsc-family-p1
# window: 5000-50C5
# label: ivs-demo-lamp-tone-1, Lamp Tone, the first project-authored visible program (interrupt-driven lamp rows and PSG tones)
# entry: 5000
# stack: 0340
; Lamp Tone (plan 07-06, D-01 to D-07, D-11 to D-15, A-02, A-03, A-05, A-14). Source of truth of the listing
; fixtures/FX-demo-lamp-tone-1/lamp-tone.listing.txt, which ivs_asm generates from this file; the listing is
; committed unedited and a suite test regenerates it and requires byte equality (D-19). Project-authored under
; the repository MIT licence. It is written from the specification in docs/video/demo-lamp-tone-1-derivation.md,
; sections (a) to (d), and from nothing else: no byte, glyph, tune or routine of another program, no EXEC call, no
; graphics ROM read, no data table, no text, no moving object, and it never halts (no HLT anywhere, D-05).
;
; Entered through the project boot stub, contract 2 (docs/diagnostics.md): reset at ENTRY, every interrupt at
; ISR with the interrupt system off. Section letters A1 to A10 below are those of the derivation (a).

ENTRY     EQU   $5000          ; cartridge reset entry (A-14, stub contract 2)
ISR       EQU   $5004          ; cartridge interrupt entry (A-14, stub contract 2)

PORT0     EQU   $01FE          ; A1: port 0 (scenario word c0)
PORT1     EQU   $01FF          ; A1: port 1 (scenario word c1)
STROBE    EQU   $0020          ; A6 (1): display strobe, a write of any value
MODESEL   EQU   $0021          ; A6 (2): a write selects Foreground/Background mode
PSG_PERA  EQU   $01F0          ; A7: tone A fine period (log address 0)
PSG_PERB  EQU   $01F1          ; A7: tone B fine period (log address 1)
PSG_CRSA  EQU   $01F4          ; A7: tone A coarse period (log address 4)
PSG_CRSB  EQU   $01F5          ; A7: tone B coarse period (log address 5)
PSG_MIX   EQU   $01F8          ; A7: mixer (log address 8)
PSG_AMPA  EQU   $01FB          ; A7: amplitude A (log address 11)
PSG_AMPB  EQU   $01FC          ; A7: amplitude B (log address 12)
PSG_AMPC  EQU   $01FD          ; A7: amplitude C, the constant-level gate (log address 13)
V_MARK    EQU   $02F0          ; A4: init marker
V_FRAME   EQU   $02F1          ; A4: frame counter
V_LAST0   EQU   $02F2          ; A4: port 0 last byte
V_CHG0    EQU   $02F3          ; A4: port 0 change counter
V_LAST1   EQU   $02F4          ; A4: port 1 last byte
V_CHG1    EQU   $02F5          ; A4: port 1 change counter
V_PEND0   EQU   $02F6          ; A4: port 0 pending amplitude word
V_PEND1   EQU   $02F7          ; A4: port 1 pending amplitude word
ROW4      EQU   $0256          ; A3: BACKTAB cell of lamp 0 of port 0 (row 4, column 6)
ROW7      EQU   $0292          ; A3: BACKTAB cell of lamp 0 of port 1 (row 7, column 6)
BACKTAB   EQU   $0200          ; A4: first BACKTAB word
GRAM0     EQU   $3800          ; A3: graphics RAM, card 0
MARKER    EQU   $03C5          ; A4: the init marker value, written last by the init
DELAY     EQU   500            ; A6 (8), derivation (b) rule R-E form B: the chosen N

; A1 reset entry: skip the interrupt entry words. Words 5002 and 5003 are zero fill, never executed.
        ORG   ENTRY
        B     init

; The interrupt service routine (A6), entered with interrupts off and one return address on the stack.
; Registers hold no state between interrupts, so none is saved. Order and cycle budget: derivation (b).
        ORG   ISR
; (1) the display strobe is the first action; (2) the mode select follows
        MVO   R0, STROBE       ; the value written is ignored
        MVO   R0, MODESEL      ; selects Foreground/Background mode
; (3) frame counter
        MVI   V_FRAME, R0
        INCR  R0
        MVO   R0, V_FRAME
; (4) deferred amplitudes, port 0 then port 1: a pending word is the amplitude plus one
        MVI   V_PEND0, R0
        TSTR  R0
        BZE   defer1           ; nothing pending
        DECR  R0               ; the amplitude to write
        MVO   R0, PSG_AMPA
        CLRR  R0
        MVO   R0, V_PEND0
defer1: MVI   V_PEND1, R0
        TSTR  R0
        BZE   poll0            ; nothing pending
        DECR  R0
        MVO   R0, PSG_AMPB
        CLRR  R0
        MVO   R0, V_PEND1
; (5) poll, port 0 then port 1: read the port, keep the low byte, compare it with the last byte; on a
; difference store it, count the change, write the fine period and set the pending amplitude word
; (0010 = amplitude 15 when a control is closed, 0001 = amplitude 0 when the byte is FF)
poll0:  MVI   PORT0, R0
        ANDI  #$00FF, R0
        CMP   V_LAST0, R0      ; Z = 1 when the byte equals the last byte
        BZE   poll1
        MVO   R0, V_LAST0
        MVI   V_CHG0, R3
        INCR  R3
        MVO   R3, V_CHG0
        MVO   R0, PSG_PERA     ; the tone period of port 0 is the byte
        MVII  #1, R3
        CMPI  #$00FF, R0
        BZE   pend0
        MVII  #16, R3
pend0:  MVO   R3, V_PEND0
poll1:  MVI   PORT1, R0
        ANDI  #$00FF, R0
        CMP   V_LAST1, R0
        BZE   render
        MVO   R0, V_LAST1
        MVI   V_CHG1, R3
        INCR  R3
        MVO   R3, V_CHG1
        MVO   R0, PSG_PERB     ; the tone period of port 1 is the byte
        MVII  #1, R3
        CMPI  #$00FF, R0
        BZE   pend1
        MVII  #16, R3
pend1:  MVO   R3, V_PEND1
; (6) render the 16 lamp words, port 0 then port 1, from the last bytes (all 16 are written every time)
render: MVI   V_LAST0, R1
        MVII  #ROW4, R4
        JSR   R5, lamps
        MVI   V_LAST1, R1
        MVII  #ROW7, R4
        JSR   R5, lamps
; (7) the gate: only in a frame in which a port changed (a pending word is not zero); channel C gets
; amplitude 15 when the last byte of at least one port is not FF and 0 when both are FF
        MVI   V_PEND0, R0
        ADD   V_PEND1, R0
        BZE   rearm            ; no port changed in this frame: no gate write
        MVI   V_LAST0, R0
        AND   V_LAST1, R0      ; FF only when both bytes are FF
        CMPI  #$00FF, R0       ; Z = 1 when both ports are idle
        MVII  #15, R0          ; MVII leaves the flags unchanged
        BNZE  gate
        CLRR  R0
gate:   MVO   R0, PSG_AMPC
; (8) re-arm, form B (derivation (b), R-E, A-03): drop the saved return address, go to the counted wait
rearm:  DECR  R6
        J     wait

; The lamp subroutine (A2, A3). In: R1 = the last byte of a port (upper byte zero), R4 = the BACKTAB cell
; of lamp 0, R5 = the return address. Out: eight lamp words written through R4. A lamp is lit when its bit
; reads 0: the complement of the byte is moved to the upper half and its bits are shifted out of bit 15,
; lamp 0 first; the carry then selects 0802 (lit, color 2) or 0801 (unlit, color 1).
lamps:  COMR  R1               ; FF in the upper half, the complemented byte in the lower half
        SWAP  R1               ; the complemented byte in the upper half
        SDBD
        MVII  #$0801, R2       ; unlit lamp word: graphics RAM card 0, foreground 1
        MVII  #8, R0           ; eight lamps
lamp:   SLLC  R1               ; carry = bit 15 = bit 7 minus i of the complemented byte, lamp i
        MOVR  R2, R3
        ADCR  R3               ; 0801 plus the carry
        MVO@  R3, R4           ; write the lamp word, R4 = R4 + 1
        DECR  R0
        BNZE  lamp
        JR    R5

; The wait of the re-arm: N iterations, then EIS and the idle loop (A5 (6), A6 (8)). N = 500 puts the EIS
; at frame cycle 9,186 to 9,597 of every frame, inside the band 3,777 to 14,930 (derivation (b), R-E).
wait:   MVII  #DELAY, R1
dly:    DECR  R1
        BNZE  dly
arm:    EIS
idle:   B     idle             ; the only code that runs between interrupts

; A5 initialization (frame 0), in this order. The first EIS (label arm) is reached at frame-0 cycle 6,564
; to 6,631, at or after 3,777, so the first routine is frame 1's and frame 0 is not displayed (rule R-S).
; (1) the graphics RAM card first: FF to GRAM0..GRAM0+7 (before frame-0 cycle 3,780, D-07)
init:   SDBD
        MVII  #GRAM0, R4
        MVII  #$00FF, R0
        MVII  #8, R1
card:   MVO@  R0, R4
        DECR  R1
        BNZE  card
; (2) clear BACKTAB and the variable area, 0200 to 02FF
        MVII  #BACKTAB, R4
        CLRR  R0
        MVII  #256, R1
clear:  MVO@  R0, R4
        DECR  R1
        BNZE  clear
; (3) the idle last bytes
        MVII  #$00FF, R0
        MVO   R0, V_LAST0
        MVO   R0, V_LAST1
; (4) the PSG init writes (A7): mixer 003C, then tone A and B coarse and the three amplitudes at 0000
        MVII  #$003C, R0
        MVO   R0, PSG_MIX
        CLRR  R0
        MVO   R0, PSG_CRSA
        MVO   R0, PSG_CRSB
        MVO   R0, PSG_AMPA
        MVO   R0, PSG_AMPB
        MVO   R0, PSG_AMPC
; (5) the marker is the last store of initialization
        MVII  #MARKER, R0
        MVO   R0, V_MARK
; (6) enable interrupts and idle: jump to the EIS of the wait
        B     arm
