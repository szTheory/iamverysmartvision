# id: ivs-diag-ctrl-psg-1
# kind: cart
# profile: intv-2609-ntsc-family-p1
# window: 5000-5058
# label: ivs-diag-ctrl-psg-1, the second project-authored diagnostic (controller reads and PSG register writes)
# entry: 5000
# stack: 0340
; Test data, not a fixture: this source reproduces the hand-assembled listing
; fixtures/FX-diag-ctrl-psg-1/diag-ctrl-psg-1.listing.txt word for word (plan 07-04, D-18, D-19). Its text
; is written from that listing's ASM column; the sections are the listing's. The suite asm compares the
; words ivs_pack reads from the listing this source assembles to with the words it reads from the hand
; listing. It has no manifest and nothing builds an image from it.
MARKER  EQU   $03C5            ; the end marker, written last

        ORG   $5000
; S1 PSG: eleven value and register store pairs through R0
        MVII  #$0023, R0       ; tone A period fine byte
        MVO   R0, $01F0        ; PSG register 0
        MVII  #$0001, R0       ; tone A period coarse nibble
        MVO   R0, $01F4        ; PSG register 4
        MVII  #$003E, R0       ; mixer: tone A on, everything else off, ports inputs
        MVO   R0, $01F8        ; PSG register 8
        MVII  #$000F, R0       ; amplitude A
        MVO   R0, $01FB        ; PSG register 11
        MVII  #$0080, R0       ; tone B period fine byte
        MVO   R0, $01F1        ; PSG register 1
        MVII  #$0000, R0       ; tone B period coarse nibble
        MVO   R0, $01F5        ; PSG register 5
        MVII  #$0008, R0       ; amplitude B
        MVO   R0, $01FC        ; PSG register 12
        MVII  #$0010, R0       ; noise period
        MVO   R0, $01F9        ; PSG register 9
        MVII  #$0064, R0       ; envelope period fine byte
        MVO   R0, $01F3        ; PSG register 3
        MVII  #$0000, R0       ; envelope period coarse byte
        MVO   R0, $01F7        ; PSG register 7
        MVII  #$000A, R0       ; envelope shape
        MVO   R0, $01FA        ; PSG register 10
; S2 poll: previous values, store pointers, the port 0 count
        MVII  #$0100, R1       ; previous port 0 value
        MVII  #$0100, R2       ; previous port 1 value
        MVII  #$0210, R4       ; port 0 store pointer
        MVII  #$0220, R5       ; port 1 store pointer
        MVII  #$0008, R3       ; port 0 values still to store
poll:   MVI   $01FE, R0        ; read port 0
        ANDI  #$00FF, R0       ; keep the low byte
        CMPR  R1, R0           ; Z = 1 when equal to the previous value
        BZE   port1            ; unchanged: skip the store
        MOVR  R0, R1           ; remember the value
        MVO@  R0, R4           ; store at [R4], R4 = R4 + 1
        DECR  R3               ; one port 0 value fewer to store
port1:  MVI   $01FF, R0        ; read port 1
        ANDI  #$00FF, R0       ; keep the low byte
        CMPR  R2, R0           ; Z = 1 when equal to the previous value
        BZE   test             ; unchanged: skip the store
        MOVR  R0, R2           ; remember the value
        MVO@  R0, R5           ; store at [R5], R5 = R5 + 1
test:   TSTR  R3               ; Z = 1 when eight port 0 values are stored
        BNZE  poll             ; back to the poll while Z = 0
; S3 delay and end
        MVII  #$0078, R0       ; delay count 120
delay:  DECR  R0               ; delay loop
        BNZE  delay            ; back while R0 is not zero
        MVII  #MARKER, R0      ; end marker
        MVO   R0, $020F        ; [020F] = 03C5, written last
        HLT                    ; sets the stop latch; the next instruction still runs
        TSTR  R0               ; follower 1
        TSTR  R0               ; follower 2
        TSTR  R0               ; follower 3
