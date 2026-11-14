# id: stub2-probe
# kind: cart
# profile: intv-2609-ntsc-family-p1
# window: 5000-500E
# label: build-time probe cartridge of the stub contract 2 tests, not a fixture
# entry: 5000
# stack: 0340
; Test data, not a fixture (plan 07-05, D-09, D-10, A-17): no manifest, no listing is committed, and the image
; is built in the build tree only (ivs_asm, then ivs_pack --ivsc) for the in-process tests run.stub2.handoff and
; run.stub2.jd-as-j. It is not an original diagnostic and proves nothing about any program. The expected values
; of those tests are the literals of this source and of fixtures/FX-boot-stub-2/boot-stub-2.listing.txt.
;
; Reset entry 5000: turn the interrupt system on and wait on one instruction. Interrupt entry 5004 (where the stub
; contract 2 vector jumps): write the marker word, add one to the entry counter word and wait without turning
; the interrupt system on again. With the vector JD the whole routine runs with interrupts off, so the counter
; reads 1 after a frame; with a plain J the vector re-enters while the interrupt request is held.
MARK    EQU   $02F0            ; marker word, RAM
COUNT   EQU   $02F1            ; entry counter word, RAM (zero after a power-on reset)

        ORG   $5000
        EIS                    ; interrupts on (the stub left them off)
spin:   B     spin             ; wait here (R7 reads 5001 between units)
        ORG   $5004
isr:    MVII  #$03C5, R0       ; the marker value
        MVO   R0, MARK         ; [02F0] = 03C5
        MVI   COUNT, R1        ; the counter
        INCR  R1               ; one more entry
        MVO   R1, COUNT        ; [02F1] = 0001 after the first routine run
done:   B     done             ; wait here without EIS (R7 reads 500D between units)
