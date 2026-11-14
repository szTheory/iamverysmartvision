# id: diag-cpu-1
# kind: cart
# profile: intv-2609-ntsc-family-p1
# window: 5000-5037
# label: ivs-diag-cpu-1, the first project-authored CPU diagnostic
# entry: 5000
# stack: 0340
; Test data, not a fixture: this source reproduces the hand-assembled listing
; fixtures/FX-diag-cpu-1/diag-cpu-1.listing.txt word for word (plan 07-03, D-18, D-19). Its text is
; written from that listing's ASM column; the sections are the listing's. The suite asm compares the
; words ivs_pack reads from the listing this source assembles to with the words it reads from the
; hand listing. It has no manifest and nothing builds an image from it.
MARKER  EQU   $03C5            ; the end marker, written last

        ORG   $5000
        MVII  #$03FF, R0       ; S1 flags: R0 = 03FF
        ADDR  R0, R0           ; R0 = 07FE
        ADDR  R0, R0           ; R0 = 0FFC
        ADDR  R0, R0           ; R0 = 1FF8
        ADDR  R0, R0           ; R0 = 3FF0
        ADDR  R0, R0           ; R0 = 7FE0
        ADDI  #$001F, R0       ; R0 = 7FFF; S Z C OV all 0
        MVO   R0, $0200        ; [0200] = 7FFF
        INCR  R0               ; R0 = 8000; S = 1, Z = 0; C and OV keep 0
        GSWD  R1               ; R1 = 8080
        MVO   R1, $0201        ; [0201] = 8080
        DECR  R0               ; R0 = 7FFF
        ADDI  #$0001, R0       ; R0 = 8000; S = 1, Z = 0, C = 0, OV = 1
        GSWD  R2               ; R2 = A0A0
        MVO   R2, $0202        ; [0202] = A0A0
        ADDR  R0, R0           ; R0 = 0000; S = 0, Z = 1, C = 1, OV = 1
        GSWD  R3               ; R3 = 7070
        MVO   R3, $0203        ; [0203] = 7070
        MVII  #$000A, R2       ; S2 loop: R2 = 10
        CLRR  R3               ; R3 = 0
loop:   ADDR  R2, R3           ; R3 = R3 + R2
        DECR  R2               ; R2 = R2 - 1; Z = 1 when it reaches 0
        BNZE  loop             ; back to the loop while Z = 0
        MVO   R3, $0204        ; [0204] = 0037
        PSHR  R3               ; S3 stack: [0340] = 0037, R6 = 0341
        PULR  R5               ; R6 = 0340, R5 = [0340] = 0037
        MVO   R5, $0205        ; [0205] = 0037
        MOVR  R6, R0           ; R0 = 0340
        MVO   R0, $0206        ; [0206] = 0340
        MVII  #$0200, R4       ; S4 indirect: R4 = 0200
        MVI@  R4, R0           ; R0 = [0200] = 7FFF, R4 = 0201
        MVO   R4, $0207        ; [0207] = 0201
        MVO   R0, $0208        ; [0208] = 7FFF
        MVII  #MARKER, R0      ; End: R0 = 03C5
        MVO   R0, $020F        ; [020F] = 03C5, the end marker, written last
        HLT                    ; sets the stop latch; the next instruction still runs
        TSTR  R0               ; follower 1
        TSTR  R0               ; follower 2
        TSTR  R0               ; follower 3
