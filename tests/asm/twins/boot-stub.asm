# id: boot-stub
# kind: boot-stub
# contract: 1
# profile: intv-2609-ntsc-family-p1
# window: 1000-101F
# label: project boot stub, not an EXEC
# entry: 5000
# stack: 0340
; Test data, not a fixture: this source reproduces the hand-assembled listing
; fixtures/FX-boot-stub/boot-stub.listing.txt word for word (plan 07-03, D-18, D-19). Its text is
; written from that listing's ASM column. The suite asm compares the words ivs_pack reads from the
; listing this source assembles to with the words it reads from the hand listing. It has no
; manifest and nothing builds an image from it.
ENTRY   EQU   $5000            ; cartridge entry, a project convention
STACK   EQU   $0340            ; stack start, a project convention
LAST    EQU   $035F            ; last word of system RAM

        ORG   $1000
        B     reset            ; reset entry: skips the vector area
        ORG   $1004
vector: MVII  #1, R0           ; interrupt entry: unexpected-interrupt trap
        MVO   R0, LAST         ; records [035F] = 1
        HLT                    ; sets the stop latch; the next instruction still runs
        TSTR  R0               ; the follower: the CPU halts after it
        ORG   $1010
reset:  DIS                    ; interrupt system off
        MVII  #STACK, R6       ; stack pointer at the project stack start
        J     ENTRY            ; hand-off to the cartridge entry
