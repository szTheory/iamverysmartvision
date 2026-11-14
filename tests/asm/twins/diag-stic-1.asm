# id: ivs-diag-stic-1
# kind: cart
# profile: intv-2609-ntsc-family-p1
# window: 5000-5084
# label: ivs-diag-stic-1, the third project-authored diagnostic (display enable, graphics RAM, BACKTAB, both card modes)
# entry: 5000
# stack: 0340
; Test data, not a fixture: this source reproduces the hand-assembled listing
; fixtures/FX-diag-stic-1/diag-stic-1.listing.txt word for word (plan 07-04, D-18, D-19). Its text is
; written from that listing's ASM column; the sections are the listing's. The pattern table, which that
; listing prints as words decoded as code, is written here as DECLE data words. The suite asm compares
; the words ivs_pack reads from the listing this source assembles to with the words it reads from the
; hand listing. It has no manifest and nothing builds an image from it.
MARKER  EQU   $03C5            ; the end marker, written last

        ORG   $5000
; a: read port 0, record the low byte, select Color Stack mode only when a key is closed
        MVI   $01FE, R0        ; read controller port 0
        ANDI  #$00FF, R0       ; keep the low byte
        MVO   R0, $02F1        ; record the port byte at [02F1]
        CMPI  #$00FF, R0       ; Z = 1 when no key is closed
        BZE   strobe           ; no key: skip the Color Stack mode select
        MVI   $0021, R0        ; key closed: a read of 0021 selects Color Stack mode
; b: the display-enable strobe
strobe: MVO   R0, $0020        ; the value written is ignored
; c1: the four color stack registers
        MVII  #$0028, R5       ; R5 = first color stack register
        MVII  #$0006, R0       ; stack entry 0
        MVO@  R0, R5
        MVII  #$000A, R0       ; stack entry 1
        MVO@  R0, R5
        MVII  #$0003, R0       ; stack entry 2
        MVO@  R0, R5
        MVII  #$000D, R0       ; stack entry 3
        MVO@  R0, R5
; c2: copy the 32 pattern bytes to graphics RAM
        SDBD                   ; the next immediate is a 16-bit word (two decles)
        MVII  #table, R4       ; R4 = the pattern table
        SDBD
        MVII  #$3800, R5       ; R5 = graphics RAM card 0
        MVII  #$0020, R1       ; R1 = 32 bytes to copy
copy:   MVI@  R4, R0           ; read [R4], R4 = R4 + 1
        MVO@  R0, R5           ; write [R5], R5 = R5 + 1
        DECR  R1
        BNZE  copy             ; back while R1 is not zero
; d: fill the 240 BACKTAB words with 0000
        CLRR  R0               ; fill word 0000
        MVII  #$0200, R4       ; R4 = first BACKTAB word
        MVII  #$00F0, R1       ; R1 = 240 words
fill:   MVO@  R0, R4           ; write [R4], R4 = R4 + 1
        DECR  R1
        BNZE  fill             ; back while R1 is not zero
; e: six explicit card words, each a 14-bit value
        SDBD                   ; word 2A01 at cell (8,3)
        MVII  #$2A01, R0
        MVO   R0, $02A3
        SDBD                   ; word 0E0A at cell (8,4)
        MVII  #$0E0A, R0
        MVO   R0, $02A4
        SDBD                   ; word 1A13 at cell (8,5)
        MVII  #$1A13, R0
        MVO   R0, $02A5
        SDBD                   ; word 381C at cell (8,6)
        MVII  #$381C, R0
        MVO   R0, $02A6
        SDBD                   ; word 2C1D at cell (11,0)
        MVII  #$2C1D, R0
        MVO   R0, $02DC
        SDBD                   ; word 1C0F at cell (11,19)
        MVII  #$1C0F, R0
        MVO   R0, $02EF
; f: counted delay
        MVII  #$0384, R1       ; delay count 900
delay:  DECR  R1               ; delay loop
        BNZE  delay            ; back while R1 is not zero
; g: the end marker, then the stop and three followers
        MVII  #MARKER, R0      ; end marker
        MVO   R0, $02F0        ; [02F0] = 03C5, written last
        HLT                    ; sets the stop latch; the next instruction still runs
        TSTR  R0               ; follower 1
        TSTR  R0               ; follower 2
        TSTR  R0               ; follower 3
; data: the 32 pattern bytes, one decle each (the 10-bit word holds an 8-bit byte)
table:  DECLE $00FF, $0000, $00FF, $0000, $00FF, $0000, $00FF, $0000   ; card 0 stripes
        DECLE $00FF, $0081, $0081, $0081, $0081, $0081, $0081, $00FF   ; card 1 box
        DECLE $0080, $0040, $0020, $0010, $0008, $0004, $0002, $0001   ; card 2 diagonal
        DECLE $00AA, $0055, $00AA, $0055, $00AA, $0055, $00AA, $0055   ; card 3 checker
