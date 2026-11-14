# id: boot-stub-2
# kind: boot-stub
# contract: 2
# profile: intv-2609-ntsc-family-p1
# window: 1000-101F
# label: project boot stub contract 2, not an EXEC
# entry: 5000
# stack: 0340
; Project boot stub, contract 2 (plan 07-05, D-09, D-10, A-12, A-14). Source of truth of the listing
; fixtures/FX-boot-stub-2/boot-stub-2.listing.txt, which ivs_asm generates from this file; the listing
; is committed unedited and a suite test regenerates it and requires byte equality (D-19). Project-
; authored under the repository MIT licence. It is not an EXEC: it provides no routine, service,
; table, trap or data to a cartridge, and it is never evidence of EXEC compatibility.
;
; The contract, in three statements:
;   1. Reset: the CPU starts at 1000 with R7 = 1000 (profile field reset.r7). The reset code at 1010
;      turns the interrupt system off (DIS), sets the stack pointer R6 to 0340 and hands off to the
;      cartridge reset entry 5000. These words are the reset path of contract 1, word for word.
;   2. Interrupt: every accepted interrupt enters at 1004 (profile field interrupt.iab_value). The
;      vector is the single instruction JD ISR: it jumps to the cartridge interrupt entry 5004 and
;      disables the interrupt system in the same instruction, so the first routine instruction at
;      5004 runs with interrupts off. A plain J here would leave the interrupt system on while the
;      routine starts: with the profile default C41 = 0 the second routine instruction is
;      interruptible, the vector re-enters and R6 climbs (D-10; run.stub2.jd-as-j shows it).
;   3. The cartridge supplies both entries: a reset entry at 5000 and an interrupt entry at 5004.
;      Both are project conventions, not hardware claims.
; An accepted interrupt pushes one return address: R6 is 0340 before the first entry and 0341
; after it. The stub saves no register and holds no state.

ENTRY   EQU   $5000            ; cartridge reset entry, a project convention
ISR     EQU   $5004            ; cartridge interrupt entry, a project convention
STACK   EQU   $0340            ; stack start, a project convention

        ORG   $1000
        B     reset            ; reset entry: skips the vector area
        ORG   $1004
        JD    ISR              ; interrupt entry: hand-off to the cartridge, interrupts off
        ORG   $1010
reset:  DIS                    ; interrupt system off
        MVII  #STACK, R6       ; stack pointer at the project stack start
        J     ENTRY            ; hand-off to the cartridge reset entry
