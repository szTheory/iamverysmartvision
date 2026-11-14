# Phase 7 discuss: S-2, boot and runtime architecture (research agent B)

**Date:** 2026-11-13. **Scope:** how a project-authored, EXEC-free guest program keeps a picture on screen
and responds to the controllers frame after frame: interrupt vector, boot-stub contract, interrupt service
routine (ISR), display strobe, input polling, PSG writes, RAM layout, cycle budget, and the effect on
existing diagnostics, goldens and mutants.

**Method and evidence labels.** Every claim is labelled [OBS] (read in the repo or measured this session),
[INF], [REC], [UNK] or [BG]. The measurements come from throwaway probes built in a scratch directory
against the unmodified core at branch HEAD (a fresh release build, `ivs-run` and `ivs_pack` from it, plus
a small C probe linked to the core library that steps units through the internal trace seam). The probe
programs were hand-encoded by a scratch script that is not part of the repository. Nothing in the repo
was modified except this file. No claim here is about real hardware: every timing number is a profile
default (C1, C2, C3, C4, C41; tier 4 or `unknown-default-chosen`) and every project convention (entry
`$5000`, stack `$0340`, interrupt entry `$5004`) has no hardware source (C68).

---

## 0. Recommendation on one screen

1. **S2-1, vector and contract.** Add a **boot-stub contract 2** whose interrupt entry at `$1004` is the
   single instruction **`JD $5004`** (jump and disable interrupts, three words, row `J`): the stub forwards
   interrupts to a cartridge-supplied entry at `$5004` and clears INTFF in the same instruction. A plain `J`
   is a trap: with the profile default C41 = 0 (acceptance does not clear INTFF) the second ISR instruction
   is interruptible, so the vector re-enters about 180 times per frame, leaks the stack (R6 reached
   `$06C2` in 5 frames) and, with the display strobe still in the code, the strobe lands at cycle 3,812,
   outside the 2,000-cycle window, so the display never turns on [OBS, probe].
2. **S2-2, coexistence.** Contract 2 is a **second image** (fixture `FX-boot-stub-2`, listing id
   `boot-stub-2`, built to `boot-stub-2.exec`, runner content token `boot-stub-2`), not an edit of contract 1.
   Contract 1, the three diagnostics, 6 `.digests` goldens, the consumer golden and the stub tests stay
   byte-identical. Measured fallback if two stubs prove a burden: replacing contract 1 in place leaves
   every existing scenario passing and moves only `slot.exec.sha256`, `digest.mem`, `digest.images`,
   `digest.composite` and the label line [OBS].
3. **S2-3, execution model.** **Tick-driven: all per-frame work in the ISR, the main loop is `EIS; B self`.**
   The ISR order is: strobe, frame counter, input poll, state update, BACKTAB/GRAM/PSG writes, then a pad loop,
   `EIS`, `PULR R7`. No register save is needed because main is idle. No `HLT` idle (a halted CPU ignores
   interrupts, C45).
4. **The re-arm rule.** INTRM is a level held 3,791 cycles from the frame start; an ISR that executes `EIS` too
   early is re-entered. **`EIS` must start at or after cycle 3,777 of the frame**, which a local rule
   guarantees: **at least 3,776 cycles from the start of the interrupt-entry unit to the start of `EIS`** (stall-free
   count). Plan for about 3,950 to 4,050 (margin about 170 to 270 cycles, 12 to 18 pad iterations of 15 cycles).
   Measured: pad 244 re-enters (EIS at cycle 3,776), pad 245 does not (EIS at 3,791) [OBS].
5. **S2-4, sustained display.** The ISR's first action (after nothing else) is the strobe `MVO R0,$0020`;
   measured at cycle 29 to 35 of the frame, against a window that closes at 2,000. The strobe is rewritten
   every frame because the latch clears at every frame start. Startup bands for the init code's `EIS`:
   at or before about cycle 1,900 the first ISR runs in frame 0 and frame 0 is displayed; between about
   2,000 and 3,776 the first ISR runs in frame 0 with a blocked strobe (frame 0 not displayed); at or after
   3,777 the first ISR is frame 1's [OBS].
6. **S2-5, input.** Poll once per frame in the ISR (`MVI $01FE,R1; ANDI #$00FF,R1`), right after the strobe.
   Effect on video: an input event in frame j changes frame j+1 (probe: event at tick 130,000 in frame 2,
   an 8 x 8 region changed in frame 3; the release at tick 250,000 in frame 4 restored frame 5) [OBS].
7. **S2-6, PSG.** Program mixer, tone period and amplitude 0 **once at init**; afterwards write **only on
   change** (amplitude gating; optionally a period from a table). Keep mixer bits 6 and 7 clear (both
   ports stay inputs). An amplitude write acts at the next 16-tick sample; the audible edge follows within
   half a tone period (4,656 ticks at period 291) [OBS].
8. **S2-7, RAM.** BACKTAB `$0200-$02EF`; variables `$02F0-$02FF` (16 words; `$02F0` frame counter, `$02F1`
   last port byte, `$02F2` upward for program state); stack `$0340` upward (depth 1 for the ISR; 32 words to `$035F`
   reserved). Scratchpad `$0100-$01EF` is 8 bits wide: no 16-bit variable and no stack there.
9. **S2-9, budget.** Frame = 14,934 cycles; display on: 13 bus windows stall about 1,446 cycles, leaving
   about 13,488 for the guest [OBS]. ISR visual and PSG work must finish before cycle about 3,700 (GRAM window
   closes at 3,780, BACKTAB row 0 latches at 3,848); then pad. One ISR of about 4,000 cycles uses under a third of a frame.
10. **Zero core change.** Everything above runs on the unmodified core; the machinery added is guest code,
    one fixture, one build target, one runner token, tests and docs (S2-2).
11. **Predicate warning for S-4.** The frame counter alone is not a sufficient progress predicate: with the
    plain-`J` vector the counter still read 5 in 5 frames while the stack leaked and the display was never on [OBS].
    Assert the counter **and** `frame k displayed == yes` **and** a video/PSG consequence; the runtime also
    supplies the mutants (S2-11).
12. **D4 data point for S-3.** The runtime skeleton alone uses 4 to 5 of the 8-branch crossing limit of D4 and
    about 25 words before any program logic [INF]; with any state machine on top, the trigger fires by letter
    (fourth listing; labels needed for the ISR/main/pad targets).

---

## S2-1. Interrupt entry and the boot-stub contract (1 versus 2)

### Facts that drive it [OBS]

- Profile: `reset.r7 = $1000`, `interrupt.iab_value = $1004`, both tier 4 with conflicts C32, C42; both inside
  the EXEC window (`$1000-$1FFF`, read-only, 10-bit). The vector is read over the bus at interrupt entry
  (the core holds no constant). So the interrupt entry word must live in the system-image slot: the cartridge cannot hold it.
- Contract 1 (22 words, `1000-1015`): `1004` is an unexpected-interrupt trap (`MVII #1,R0; MVO R0,$035F; HLT`
  plus follower). Review rule (docs/diagnostics.md): "no routine, service, entry point or data to a cartridge:
  no callable code, no table, no font, no vector a cartridge may use". Mechanical guard (listing checker):
  addresses `1000-101F`, at most 16 non-zero words, rows only `HLT DIS EIS MVII MVO TSTR B J`, non-zero words at the reset and
  interrupt addresses.
- CPU facts from the decode table and stepper (`core/src/cpu`): `EIS`, `DIS` are **not** interruptible; `J` **is**;
  the MVO family (`MVO`, `MVO@`, `PSHR`, `MVOI`) is not; the first instruction after an interrupt entry always runs
  (prev_interruptible = 0), the second does not; acceptance leaves INTFF set under C41 = 0; `JD` (`DE = 10`) clears INTFF
  as part of the jump; entry is a 9-cycle unit that pushes the PC at `[R6]` and increments R6.

### Options

| Option | What | Verdict |
|---|---|---|
| A | Keep contract 1; the program never enables interrupts; synchronise by counting cycles | Rejected: no frame event reaches the guest; sustained display (the Phase 6 hand-off) needs a per-frame strobe stamped inside a 2,000-cycle window; cycle-count sync is brittle, defeats the point of testing INTRM/EIS/stack, and the display-on frame has 13 stalls so counts differ with the display state |
| B | Contract 2 vector = plain `J $5004` | **Rejected, measured**: runaway re-entry (below) |
| C | Contract 2 vector = `DIS` then `J $5004` | Works (1 entry per frame, R6 max `$0341`, probe target `J $5004`) but is two instructions and 4 more words and cycles than `JD`; `JD` is strictly smaller |
| D | **Contract 2 vector = `JD $5004`** | **Recommended**: one instruction, 12 cycles, INTFF cleared by the jump itself, same word budget as the old trap (3 words, 3 non-zero) |
| E | Put the ISR (strobe, frame tick) inside the stub | Rejected: that is an EXEC imitation, the exact thing D-29 forbids; it also makes results depend on a stub-resident service |
| F | RAM-resident vector (`MVI $035E,R7`) | Rejected: power-on RAM is zero, so an uninitialised vector jumps to `$0000` (STIC register space); adds a cartridge-callable cell, a "vector a cartridge may use" in the plainest sense |
| G | Make the profile `iab_value` point at `$5004` | Rejected: changes a profile field that carries a hardware conflict (C42) for a project convenience |

### Measured behaviour of the vector choices [OBS]

A cartridge ISR (`INCR R2; MVO R2,$02F0`, pad, `EIS`, `PULR R7`; main `EIS; B self`), five frames:

| Vector | Interrupt entries per frame | Stack | ISR body runs |
|---|---|---|---|
| `J $5004` | 178 to 181 | R6 max `$06C2`, final R6 `$06C1`, final R7 `$FFFE`/`$FFFF` (derailed) | once per frame, late (after the level drops) |
| `JD $5004` | 1 | R6 max `$0341` | once per frame |
| `DIS; J $5004` | 1 | R6 max `$0341` | once per frame |

With the real strobe in the ISR, the plain-`J` variant puts the strobe at cycle 3,812 of every frame (a blocked write, no bus windows,
display never on) while the counter `[02F0]` still equals the frame count. That makes a counter-only predicate vacuous for this failure.

### Recommendation [REC]

Contract 2, 22 words at `1000-1015` (`1000 B $1010`, `1004 JD $5004`, `1010 DIS`, `1011 MVII #$0340,R6`,
`1013 J $5000`): identical reset path to contract 1; the interrupt entry is the three words `0004 0352 0004`
(`0352` = BB 11, target bits 15..10 = 010100, DE = 10). Convention: **cartridge reset entry `$5000`,
cartridge interrupt entry `$5004`** (the stub's own layout mirrored: branch at offset 0, interrupt code at offset 4).
Words used: 10 non-zero (limit 16). The existing listing checker accepts it unchanged: in a scratch copy of the tree with
the listing and one inventory line added, `listing` passed with `listings=5` [OBS]. Interrupt cost to the first cartridge
instruction: 9 (entry) + 12 (`JD`) = 21 cycles, plus up to 9 cycles of boundary lag.

**Review-rule amendment (documented, mechanical):** the interrupt hand-off is a forwarding jump with no code, the same kind
as the reset hand-off (stub to cartridge); the cartridge still may not call into the stub. The stub stays free of services.
The checker cannot tell `J` from `JD` (one leaf row), so the distinction needs a behavioural test and a named mutant (S2-11).

**Lenses.** CPU architect: relies on the first-instruction-always-runs rule and on `JD` semantics, both read in the stepper. EE/hardware:
no hardware claim; real console wiring of INTRM release is unknown (C3, C91). QA: a `J`-to-`JD` regression is invisible to the
listing checker, so it needs `run.stub2.*` plus the displayed-frame predicate. Rights: the stub and ISR are project-authored; nothing
of an EXEC is reproduced. API: no core change. DX: one convention (`$5000`, `$5004`) a hand assembler can remember.

**Reversal evidence.** A primary or D10 source showing INTFF is cleared at acceptance (C41 = 1) does not reverse this (the `JD`
is harmless then); a source showing the real interrupt line is a short pulse would remove the need for the re-arm rule (S2-3) but not for `JD`.

**Risks.** (1) A reviewer reads `JD` as an HLE hook: mitigated by the amended rule text and the unchanged row guard. (2) The
convention `$5004` is invisible to the cartridge format (IVSC has no entry metadata): it lives in docs and the manifest note.
(3) Real EXEC cartridges use a header at the start of the cartridge region that the EXEC reads [BG, unverified]; this program's
`$5000`-is-code layout is therefore not usable under a user-supplied EXEC, and nothing here makes that claim (S2-12, D11 stays untouched).

---

## S2-2. Coexistence: second image versus replacing contract 1

### Options and measured blast radius

| | X: second image (recommended) | Y: contract 1 becomes contract 2 in place |
|---|---|---|
| Existing diagnostics | untouched; they never `EIS`, so the vector is unreachable for them either way | untouched in behaviour (measured) |
| Goldens | none move | 6 `.digests` (diag-cpu-1, diag-ctrl-psg-pass, diag-input-pass, diag-stic-fgbg-pass, diag-stic-cstack-pass, timeout-budget) plus the consumer golden (3 replays) re-bless |
| What moves under Y [OBS] | n/a | I replaced the stub in place and re-ran five scenarios (`diag-boot-pass`, `diag-ctrl-psg-pass`, both `diag-stic-*`, `diag-input-pass`): all `outcome pass`; the only report lines that differ are `slot.exec.sha256`, the `exec:` label line, `digest.mem`, `digest.images`, `digest.composite`. Ticks, cpu, instance, video, audio, input, psglog digests are identical |
| Stub tests | unchanged; new `run.stub2.*` | `run.stub.trap` and `run.stub.halt-needs-follower` (which assert the trap at `1004-100A`) are rewritten |
| Unexpected-interrupt trap | kept for the diagnostics | lost |
| Claims | CL-0008, CL-0014, CL-0020 stay scoped to "the project boot stub, contract 1" with unchanged evidence | their scope text moves to contract 2 with re-blessed evidence |
| Plumbing | one more image, one build target, one runner token, one consumer argument | none |
| Maintenance | two stubs to keep in sync (they share 17 of 22 words) | one stub |

### Recommendation [REC]: X

Additive. It keeps Phase 4 to 6 evidence frozen (the repository's own rule: "any change of a word is a new contract number,
a new manifest note and new expected values"). Plumbing, all small and inside patterns already used:

- Fixture: `fixtures/FX-boot-stub-2/` (listing, manifest `redistributable: yes`, MIT, same author-attested pattern as `FX-boot-stub`);
  `data/expected/listings.ids` gains `boot-stub-2 boot-stub fixtures/FX-boot-stub-2/boot-stub-2.listing.txt`; header `# contract: 2`.
- Build: `tools/pack` builds `boot-stub-2.exec` into the images directory next to `boot-stub.exec`; the in-process suites get it through
  the existing `--c-include`.
- Runner: a scenario content token `boot-stub-2` (parser, executor read of reserved file `boot-stub-2.exec`, report label). **Do not use the
  relative-path form**: a `content exec <relpath>` load runs but prints `exec: -`, dropping the label "project boot stub, not an EXEC"
  that D-30 derives only from the symbolic name [OBS]. The label wording should say contract 2.
- Installed consumer: one more image argument and the `pkg-install` file list (coordinate with S-4).
- Tests: `run.stub2.path` (reset path `1000, 1010, 1011, 1013, 5000`, R6 = `0340`, INTFF clear), `run.stub2.handoff` (a stand-in cartridge
  reaches `5004` with INTFF clear, R6 = `0341`, `[0340]` = interrupted PC, 21 cycles after entry start), a named control
  `STUB2_JD_AS_J` (word `1005` `0352` to `0350`) that must fail `run.stub2.handoff`. Expect roughly 8 to 12 new test ids times two
  presets, listings 4 to 5 and manifests +1.

**Reversal evidence.** If plumbing for the second image exceeds one plan, or a second stub is judged confusing, fold into Y using the
measured classified diff above (4 keys plus the label, 6 goldens plus the consumer golden); that is a mechanical re-bless of the kind
Phases 5 and 6 already did.

**Risks.** Two stubs drift (mitigation: the listing checker covers both; share the derivation note); a program author loads the wrong
stub (mitigation: scenario `content exec boot-stub-2` is the only form the program's scenarios use, and the program's manifest note names it).

---

## S2-3. Execution model and the re-arm rule

### Options

| | A: tick-driven, ISR does everything, main idle (recommended) | B: ISR sets a flag, main does the work and re-arms | C: no interrupts, poll a STIC register for the window | D: stub-resident tick service |
|---|---|---|---|---|
| Register save/restore | none (main idle) | needed (PSHR/PULR 9 + 11 cycles each, or reserved registers) | none | none |
| Frame content determinism | frame k = function of the input at the ISR poll; all BACKTAB writes land before the row-0 latch | main-loop writes tear across the 12 row latches | depends on poll phase | n/a |
| Listing size and branches | smallest (about 4 branches in the skeleton) | larger (flag poll, delay, EIS) | small | n/a |
| Exercises INTRM, EIS/DIS, stack | yes | yes | no | no |
| Time dependence on C3 | pad length | delay length | window figures C1 | n/a |
| Verdict | **recommended** | acceptable fallback | rejected | rejected (D-29) |

Option C detail [OBS from the regmap, INF for the behaviour]: a read of a collision register (`$0018-$001F`, read mask `$03FF`) returns the
fill pattern with the window open and `$FFFF` when blocked, so a guest could detect the register window opening. It works only through the
read-back fill and the blocked-read policy (C5), both defaults, and it never touches INTRM; keep it as a possible Phase 8 probe idea, not the runtime.

### The re-arm rule (the central footgun) [OBS, probes]

INTRM is a level for `irq_low = 3,791` cycles from each frame start `S_k`, sampled one cycle before each unit start (`bus.line_sample_lag = 1`).
The CPU accepts at an open boundary with INTFF set; the vector clears INTFF (`JD`); the ISR re-enables with `EIS` and returns with `PULR R7`.
`EIS` itself is not interruptible, so `PULR` (11 cycles) always runs first; the boundary after it is the first that can accept.
A re-entry happens iff the first unit start after `PULR` minus 1 cycle is still below `S_k + 3,791`, i.e. iff `EIS` starts before
`S_k + 3,791 + 1 - 15 = S_k + 3,777`. Probe, 10 pad lengths: EIS at cycles 3,731, 3,746, 3,761, 3,776 re-enter (2 entries per frame, `[02F0]`
doubles, STIC `blocked_writes` rises by one per frame because the second entry's strobe is outside the window); EIS at 3,791 and later: one entry.
The local form: the entry unit starts at or after `S_k + 1`, so "from entry start to `EIS` start at least 3,776 cycles" is enough regardless of
the entry offset. The count is stall-free (display-off frames have no stalls; bus windows only add cycles).

Pad loop cost: `MVII #N,R1` (8) then N times (`DECR R1` 6 + `BNZE` 9 taken, 7 not taken), so about 15N - 2 + 8 cycles. Generalisation if a
profile changes: `D >= irq_low + lag - 15 - 1`; the shorter alternates (2,907; 2,920) are covered by the 3,776 rule; a longer level would
re-enter, and the frame-count predicate plus the displayed-frame predicate would show it.

### Recommendation [REC]: A

ISR skeleton (cycles from the stall-free count; derivation in Appendix A):
`strobe -> counter -> poll -> update -> writes -> pad -> EIS -> PULR R7`; main: `EIS; B self` (9 cycles taken, interruptible, so entry lags
the level start by 1 to 9 cycles; measured entry at cycle 1 to 9). The ISR may clobber any register. If S-1 needs a subroutine, `JSR R5,...` needs no
stack; a second stack level is available (`$0340-$035F`).

**Reversal evidence.** S-1 needs more visual work per frame than fits before cycle about 3,700 (for example a full-screen redraw): then split
over frames (ISR does one row per frame) or use B. A hardware or D10 result with a short real INTRM pulse would let the pad shrink.

**Risks.** (1) Tuned to C3: mitigated because the failure is loud (counter and `blocked_writes`) and the pad is a profile-derived rule, not a magic number.
(2) The pad looks like wasted time: it is idle time anyway (main would spin); the ISR keeps the BACKTAB writes inside the pre-latch region. (3) Hand-count of
pad cycles must be checked against the observed `EIS` start: derive it in the listing header, then verify with a scenario control (S2-11, pad minus 10).

**Anti-pattern list from the probes.** Plain `J` vector; `HLT` as idle (HALTED ignores INTR and INTRM, C45); `EIS` before cycle 3,777; strobe after cycle
2,000; BACKTAB writes after the row-0 latch (cycle 3,848); mixer bits 6/7 set; 16-bit variables in scratchpad; relying on R6/R7 at the stop instant.

---

## S2-4. Sustained display (the per-frame strobe)

[OBS] The display latch clears at every FRAME_START and is set only by a strobe write to `$0020` inside the register window `[S_k, S_k + 2,000
cycles)`; the latch value is decided at GMEM_CLOSE (cycle 3,780): no latch, no bus windows, a blank (border-fill) frame, CPU keeps 14,934 cycles.
A blocked strobe is dropped silently (counted in the STIC's blocked-write counter).

### Recommendation [REC]

The strobe is the first ISR instruction after the (stub-provided) `JD`: `MVO R0,$0020` (11 cycles; the value is ignored), measured at cycle 29 to 35 of
frames 1 and later, 95 in frame 0 (init plus entry) [OBS]. Margin to the window close: about 1,950 cycles, so tolerant of the 9-cycle boundary lag and of any
reordering of the next few instructions. The other STIC registers (color stack, border, mode) are stored, not frame-cleared: write them once in init
(within the window) and never again, unless the program animates them. The mode select (`MVI $0021` for Color Stack) persists across frames.

**Startup bands** (init code length to its `EIS`; probe with a delay loop of 0 to 300 iterations) [OBS]:

| Init cycles to `EIS` | First ISR | Frame 0 |
|---|---|---|
| up to about 1,900 | frame 0, strobe inside the window | displayed |
| about 2,000 to 3,776 | frame 0 (entry at 2,474 and 3,074 measured), strobe blocked, `blocked_writes = 1` | not displayed, no bus windows |
| 3,777 or more | frame 1 | not displayed (no ISR, no strobe) |

Either band is correct, but the plan should state which it intends. Recommended: init does its one-time STIC registers and PSG programming
first, then (optionally) GRAM, then `EIS` at under 1,900 cycles if frame 0 should show the initial picture, or after a deliberate delay if
frame 0 should be the blank baseline. For a visible-region diff (S-4) a blank frame 0 is a convenient "before" picture; a displayed frame 0
shortens the scenario by one frame. GRAM load costs about 32 cycles per byte (`MVI@ 8 + MVO@ 9 + DECR 6 + BNZE 9`); the GRAM window (3,780)
therefore holds about 100 bytes (12 cards) started at cycle 500; a longer load belongs in later ISR frames (the window re-opens every frame).

**Reversal evidence.** A D10 window probe (C1, C2) that shows the real window shorter than the code-visible margin (about 1,950 cycles
of slack) is the only thing that matters here; the alternates 2,900 and 3,796 only widen it. **Risks.** A silently blocked strobe: always assert
`frame k displayed == yes` for the intended frames, never only a hash.

---

## S2-5. Input polling

| Option | Verdict |
|---|---|
| ISR, once per frame, right after the strobe (recommended) | Stable sampling instant (about cycle 60 to 150 depending on ISR order; a 1 to 9 cycle jitter from the idle loop); simplest; the picture of frame k is a pure function of the port byte at that instant |
| Main loop continuous poll (the `diag-ctrl-psg-1` style) | Lowest latency, but BACKTAB writes from main tear across row latches and main must save state; no benefit because the video is frame-granular |
| Poll in both | Over-built |

[OBS] End to end: `input at_tick=130000 c0=KEY_1` (frame 2) was read by frame 3's ISR (PSG log record at tick 179,808 = `S_3 + 600` ticks = cycle 150);
the 8 x 8 BACKTAB region changed in frame 3 and was restored in frame 5 after the release at tick 250,000 (frame 4). Frame hashes 0 to 2 equal, 3 and 4 equal,
5 equals 0. So **an event in frame j is visible in frame j+1**, provided the ISR's writes complete before cycle 3,848.

Scenario placement rule for S-4 [INF]: put input ticks at least about 1,000 ticks away from any frame start plus the ISR poll offset
(that is, comfortably inside the frame, for example `S_k + 30,000`); then ±9 cycles (36 ticks) of entry jitter cannot change which frame sees the event.
Port reads: `$01FE` is controller 0 (the active-low byte, `KEY_1` reads `$7E`), `$01FF` controller 1; keep mixer bits 6 and 7 clear.

**Reversal evidence.** A later need for sub-frame input latency (not in Phase 7). **Risk.** A program that polls twice per frame sees the event one poll earlier
than the video can show it; harmless but confuses the "frame j+1" rule.

---

## S2-6. PSG write cadence

### Options

| | Write on change (recommended) | Refresh every frame |
|---|---|---|
| PSG write log | 3 to 4 init records, then 1 to 2 per input change | 3 to 4 records every frame; log grows with run length (cap for scenario captures 4,096) |
| Evidence clarity | "no write before the input, a write with the expected value after" is a sharp predicate | tick-only differences (jitter) in otherwise equal records |
| Runtime realism | less like a real sound engine | closer to the usual 60 Hz engine [BG] |
| Cost | 11 cycles per write | about 100 cycles per frame |

### Recommendation [REC]

Write the tone period (fine `$01F0`, coarse `$01F4`), the mixer (`$01F8 = $3E`: tone A on, noise off, ports inputs) and amplitude A (`$01FB = 0`) once in init, then on
each input change write amplitude A (and optionally a period). Rationale from probes [OBS]: the log held exactly the three init writes, one frame-0 ISR write (my probe
quirk), then one write per input change, each stamped with the exact access tick (`179,808` for the first). An amplitude write is sampled at the next 16-tick
boundary; with period `$123` = 291 the square wave toggles every 4,656 ticks (`period x 16`), so sound starts at the write if the output phase is high, else within one
half period. Keep the period low-frequency and the run short: audio capture entries cap at 4,096 (a period-291 tone makes about 12.8 changes per frame, so about 320
frames). The PSG tone counter reloads from the period of an empty register at reset (C77), so a period written at init takes effect after the first expiry (up to about
65,536 ticks, 1.1 frames): place the first input event after frame 2 (S-4). Write cost is 11 cycles per `MVO`, 44 ticks, above the PSG minimum gap (28 ticks).

**Reversal evidence.** None expected. **Risks.** A tone that never changes pitch only proves amplitude gating; S-1 may want a second observable (pitch table) if the audio
assertion should depend on more than one log record. A write to `$01F8` with bits 6/7 set turns a port into an output and the poll then returns the latch.

---

## S2-7. RAM layout and stack

[OBS] Profile memory map: scratchpad `$0100-$01EF` is 8-bit RAM (`ram8`, read mask `$00FF`); PSG `$01F0-$01FF`; system RAM `$0200-$035F` is 16-bit (`ram16`, 352 words);
STIC registers `$0000-$003F`; GRAM `$3800-$3FFF` (8-bit, gated). All of these except GRAM are 10-bit-addressable: every operand address below `$0400` is one decle, so
no `SDBD` is needed (a 16-bit immediate such as the GRAM destination `$3800` is `SDBD; MVII`, 4 decles plus the SDBD, 14 cycles).

| Words | Use | Note |
|---|---|---|
| `$0200-$02EF` | BACKTAB (240 words) | the only region the STIC latches by row; writes tear at the row latches |
| `$02F0` | frame counter (ISR count) | progress marker 1; equals `floor(T / 59,736) + 1` at stop tick T (observed 7 at tick 400,016) |
| `$02F1` | last port-0 byte (low byte) | progress marker 2; the Phase 6 diagnostic also used `$02F1` for the same purpose |
| `$02F2-$02FF` | program state (S-1) | 14 words |
| `$0300-$033F` | free (64 words) | sprite-free program; extra tables |
| `$0340-$035F` | stack, grows up from the stub's `R6 = $0340` | ISR depth 1: `[0340]` holds the interrupted PC, R6 ranges `$0340` to `$0341`; observed R6 max `$0341` |

Never place a 16-bit value in scratchpad (the high byte is dropped) and never put the stack there (the interrupt entry writes a 16-bit PC). Do not assert
`[0340]`, `R6` or `R7` (they depend on the stop instant). Variables must be initialised by the program (power-on RAM is zero, but the program should not rely on
it for markers; it may for BACKTAB).

**Reversal evidence.** A program needing more than 14 state words or a deeper stack: move state to `$0300-$033F`.

---

## S2-8. Interplay with EIS/DIS, bus steal and bounds

| Item | Fact | Consequence |
|---|---|---|
| EIS/DIS | not interruptible [OBS decode]; INTFF = 1 after EIS | `EIS; PULR R7` is safe: PULR runs before any acceptance |
| C41 = 0 | acceptance leaves INTFF set | the vector must disable (S2-1) |
| `J`, `MVII`, ALU, `DECR`, `BNZE` | interruptible | boundaries every 6 to 12 cycles in main; the second ISR instruction is a boundary |
| `MVO`, `MVO@`, `PSHR`, `MVOI` | not interruptible | a store stream is never interrupted or stalled (A-09 of Phase 6); a BACKTAB fill loop (MVO@, DECR, BNZE) still has open boundaries at DECR/BNZE |
| `HLT` | HALTED ignores INTR/INTRM (C45) | never idle with `HLT`; a scenario that ends by `stop halted` is the old diagnostic style, not this runtime |
| Bus steal | 13 windows from cycle 3,791 every 912 cycles, hold 114, only when the latch is set; granted at open boundaries; stall 111 cycles average | CPU keeps about 13,488 of 14,934 cycles; the ISR pad end moves later (never earlier) by a window; the strobe is before the first window |
| Bounds (Phase 4 to 6) | per-unit bound U = 468 ticks (the grant unit), g = 28 ticks (PSG write gap) | unchanged by this runtime: no new unit kind, ISR instructions are 4 to 14 cycles; audio and log capacities unchanged |
| Late grants | `late_grants` counts windows that did not get a grant | 0 in every run here |

[INF] The runtime adds no core behaviour, so no new bound has to be re-proved; the probes show `grants = 13` per displayed frame and `0` in undisplayed frames, and a frame length
that is always 14,934 cycles (overshoot at unit boundaries only).

---

## S2-9. Cycle budget per frame (profile defaults; cycles from the frame start `S_k`)

| Cycle | Event | Source |
|---|---|---|
| 0 | FRAME_START: latch cleared, register and GMEM gates open; INTRM level begins (seen from cycle 1) | STIC events |
| 1 to 9 | ISR entry unit starts at the first open boundary of the idle loop (measured 1 to 9) | probe |
| entry + 9 + 12 | first ISR instruction (`JD` done); the strobe write follows: cycles 29 to 35 | probe |
| 2,000 | REG_CLOSE: STIC registers blocked (reads `$FFFF`, writes dropped) | C1 default |
| 3,777 | earliest safe `EIS` start (pad rule) | derived and measured |
| 3,780 | GMEM_CLOSE: GRAM/GROM blocked; latch decides bus windows | C2 default |
| 3,791 | INTRM level ends; bus window 0 begins (latch set) | C3, C4 defaults |
| 3,848 + 912 r | BACKTAB row r latch, r = 0 to 11 (3,848, 4,760, ..., 13,880) | event table of the profile |
| 3,791 + 912 i | bus window i, i = 0 to 12, hold 114 (last ends 14,849) | C4 default |
| 14,934 | FRAME_END (next FRAME_START) | 59,736 ticks |

Budget (display on): 14,934 total; stalls about 1,444 to 1,453 (13 windows, probe); guest cycles about 13,481 to 13,490. ISR: work (typically 150 to 600 cycles: strobe 11, counter 27,
poll 18, compare 6 to 9, a BACKTAB word with SDBD 14 + 11, a PSG write 8 + 11) plus pad to about 4,000. Idle main: about 9,400. The ISR work, including every BACKTAB and GRAM write, must end
before cycle about 3,700; the pad then runs through the GMEM close and the first bus window harmlessly. Display off: no stalls, 14,934 guest cycles.

Wall cost [OBS]: 100 frames (1.66 s of guest time) ran in 0.063 s in a release `ivs-run`; a 6-frame scenario is a few milliseconds, so the runtime adds no CI cost worth budgeting. Dev (sanitised) builds run the
same program under ASan/UBSan; no per-frame core allocation exists.

---

## S2-10. Effect on existing diagnostics, goldens, mutants and claims

Under the recommendation (X, a second image) nothing existing changes:

- **Diagnostics** (`diag-cpu-1`, `diag-ctrl-psg-1`, `diag-stic-1`): run under contract 1, unchanged listings, expects, frames and mutant inventories.
- **Goldens:** the six `.digests` and `data/expected/pkg-consumer.golden` stay byte-identical; `tests-expected` grows only by the new ids.
- **Phase 6 note on EIS:** `ivs-diag-stic-1` has no `EIS`, so the display there stays unsustained by design; its frame 0 is still the single picture. The runtime does not replace it.
- **Gates that grow:** `listings` 4 to 5, fixtures/manifests +1, `runner` parse suite (token), `pkg-install` image list (+1), `tests-inventory` ids, optional `important-gates` lines for SCN-01.
- **Re-bless classification, if Y were chosen:** keys that must move: `slot.exec.sha256`, label line, `digest.mem`, `digest.images`, `digest.composite`; keys that must not: step ticks, cpu, instance, video, audio, input, psglog digests, asserts, outcome [OBS].

---

## S2-11. Controls and mutants the runtime supplies (hand-off to S-4)

Each was exercised by a probe unless marked [INF]:

| Mutant | Edit | Observed effect | Predicate that catches it |
|---|---|---|---|
| M-vec | stub-2 word `1005` `0352` to `0350` (JD to J) | 178 to 181 entries/frame, stack leak, strobe at cycle 3,812, display never on, counter still equals the frame count | `frame k displayed == yes`; also `run.stub2.handoff`; **not** the counter alone |
| M-pad | pad count 245 to 235 (EIS at cycle 3,776 or earlier) | 2 entries/frame, counter doubles, STIC blocked writes rise | counter `== floor(T/F)+1` and `displayed == yes` |
| M-late | delay before the strobe 130 to 140 iterations (strobe after cycle 2,000) | strobe at 2,134, blocked every frame, grants 0, frames not displayed | `frame k displayed == yes` |
| M-eis | remove the final `EIS` [INF] | one ISR, then INTFF stays 0 | counter stuck at 1 |
| M-idle | idle with `HLT` plus follower [INF] | no ISR after halt | counter stuck, `stop halted` |
| M-poll | poll address `$01FE` to `$01FF` [INF] | input never seen with `c0` events | input-consequence asserts |
| M-psg | amplitude write address `$01FB` to `$01FC` [INF] | wrong channel | psglog `addr=` assert |
| N-neg | pad count 245 to 260 (negative control) | still one entry/frame | must still pass |

---

## S2-12. Termination, stop condition and scenario shape

- The runtime never halts. Scenarios are time-bounded (`run-to <ticks> ticks stop target`); the guest-progress predicate is memory markers at the target (`[02F0]`, `[02F1]`, program state) and
  frame/PSG asserts, not registers. The target tick should sit mid-frame (for example `S_k + 30,000`): `[02F0] = k + 1` then holds for every k.
- Capture caps (runner): 16 captured frames (`capture video <first> <count>`), 4,096 PSG log records and 4,096 audio changes: keep the scenario to a handful of frames (the probes used 6 to 7).
- Owner-local EXEC route (S-5): this program's entry convention (`$5000` code, `$5004` ISR, project stub vector) is not an EXEC cartridge header; it is not run under a user-supplied EXEC and no
  statement about a real EXEC is made (D11 untouched). [BG] real cartridges carry a header the EXEC reads; making this program header-compatible is a separate, hardware-claim-adjacent step outside Phase 7.

---

## Adversarial pass

1. **"The pad loop is a hack; a real ISR does useful work."** True; the pad is idle time, chosen so a hand-assembled program stays small and its frame content is a pure function of the poll. The
   rule is derived and falsifiable (M-pad). If S-1's work naturally exceeds 3,776 cycles the pad vanishes.
2. **"Two stubs is a smell."** Real, but it is the only choice that keeps the three diagnostics' evidence byte-identical. The measured Y diff is small and mechanical; the plan can adopt it if the plumbing cost surprises.
3. **"`JD` at the vector is an HLE hook."** It contains no behaviour beyond the hand-off the stub already has for reset. The review rule is amended in text and the mechanical row guard is unchanged.
4. **"Tuned to a tier-4 constant (C3), so the program is brittle."** The profile figure is a default; the guest program never claims it. The failure mode is loud and automatic (counter, `displayed`, STIC blocked writes). Alternates 2,907 and 2,920 are covered by the same rule.
5. **"The program proves the model's own assumptions."** Yes: guest progress here is repeatability plus internal consistency against profile defaults, the same shared-misreading caveat as every Phase 5 and 6 claim. The tier stays `playable guest progress, original program only`, never a hardware claim (D10).
6. **"A counter predicate is enough."** Measured false (S2-1 table): the plain-`J` vector keeps the counter right while the display is dead.
7. **Lessons from other ecosystems [BG, unverified in this session].** Test-ROM suites for other consoles (NES, Game Boy) typically drive an NMI or VBlank handler that writes result markers to memory and compare those markers, never a screenshot, and they pin a cycle budget by construction; the same shape (markers plus a bounded frame count) is what S2-3 recommends. Another emulator's interrupt-and-return conventions are not used and not copied (the repository rule).

---

## Assumptions from other decision points

- **S-1 (program):** the program is a per-frame, controller-driven program that changes BACKTAB words (and possibly GRAM cards) and PSG amplitude/period on input; it uses no MOBs; its visual work fits before cycle about 3,700; it reads one controller port;
  it needs a handful of state words in `$02F2-$02FF`.
- **S-3 (assembler):** the program needs labels for the ISR/main/pad targets and a data directive for card tables; the skeleton alone uses 4 to 5 of D4's 8-branch limit (`B main`, the change test, the pad `BNZE`, the idle `B`), so S-3 will find the D4 trigger fired by letter. The hand-encoding used for these probes (about 20 lines of scratch script, 15 minutes) is not a project effort figure.
- **S-4 (verification):** scenarios use `content exec boot-stub-2`, `capture video`, `capture psglog`, `capture audio`, timed `input`, `run-to ... stop target`; assertions combine marker, `frame k displayed`, pixel/region and psglog; the installed consumer gets the contract-2 image argument.
- **S-5 (rights):** `FX-boot-stub-2` and the program are project-authored MIT with `redistributable: yes`; no third-party byte; the owner-local EXEC route is documentation plus a SKIPPED line and does not run this program.

## Open items for planning

1. Pin the contract-2 listing derivation (JD encoding `0352`, `0004`, the checker's acceptance) in a derivation note committed before the listing, in the Phase 6 style.
2. Decide the runner token spelling (`boot-stub-2`) and the label text; decide whether `ivsr` stays 3 (additive token, no new report key; expected unchanged).
3. Decide frame 0 policy (displayed baseline versus blank baseline) with S-1/S-4: it fixes the init length band and the first assert.
4. State the pad count in the listing header from the rule (cycles from entry to `EIS` at least 3,776, planned 3,950 to 4,050) and verify it with M-pad and N-neg.
5. Name the new tests and the control `STUB2_JD_AS_J` and register them in the inventories; add an `important-gates` line tying SCN-01 to the vector, displayed-frame and re-arm mechanisms.
6. Document the amended review rule and the cartridge interrupt convention in `docs/diagnostics.md` (a "project program runtime contract" section), including the footgun list.
7. Check `ivs_pack --patch` handles the exec image for `STUB2_JD_AS_J` (the existing `--patch` is documented for any listed word).
8. Consider (low priority) a profile-alternate robustness test: the same program under `irq_low` 2,907/2,920 and `reg_window` 2,900 (test-built parameter copies, as the Phase 6 `stic.irq.c41-both-values` family does).

---

## Appendix A. Reference skeleton and cycle counts (pseudo-assembly, not a listing)

```
5000  B   main            ; 2 words; reset entry (stub contract 2 hands off here)
5002  (fill)             ; 2 words, zero
5004  isr:                ; entered with INTFF = 0, R6 pushed; first instruction runs at about cycle 22-30
      MVO  R0,$0020       ; 11  display-enable strobe (value ignored)
      MVI  $02F0,R0       ; 10  frame counter
      INCR R0             ;  6
      MVO  R0,$02F0       ; 11
      MVI  $01FE,R1       ; 10  controller 0
      ANDI #$00FF,R1      ;  8
      ...                 ;     program logic (S-1): BACKTAB/GRAM/PSG writes, all before cycle ~3,700
      MVII #N,R1          ;  8  pad: N so that entry-to-EIS >= 3,776, planned ~3,950-4,050
pad:  DECR R1             ;  6
      BNZE pad            ;  9 taken, 7 not taken
      EIS                 ;  4  must start at or after frame cycle 3,777
      PULR R7             ; 11  return; INTFF = 1, level already low
main: ... init: STIC registers, PSG init writes, (GRAM), 
      EIS                 ;  4
idle: B   idle            ;  9
```

## Appendix B. Measurement summary (probes against the unmodified core; release build)

- Vector variants, 5 frames: plain `J`: 178 to 181 entries/frame, R6 max `$06C2`; `JD`: 1 entry, R6 max `$0341`; `DIS; J`: 1 entry.
- Pad sweep (EIS start in frame cycles, frames 1 and later): 244 iterations: 3,776 re-enter; 245: 3,791 single; 246 to 250: 3,921 to 3,981 single (the first bus window adds about 115 stall cycles to the EIS start).
- Strobe delay sweep: strobe at 1,541, 1,916, 1,931, 1,946 displayed; 1,991 blocked once; 2,134 blocked in every frame (grants 0).
- Init `EIS` sweep: entry at 68, 1,574 (frame 0 displayed); 2,474, 3,074 (frame 0 not displayed, blocked write 1); none in frame 0 (EIS after 3,777).
- Full program (strobe, counter, poll, PSG on change, BACKTAB on change), frames 0 to 5: all displayed; counter 7 at tick 400,016; PSG log 3 init + 1 + one per input change; audio first change at the write tick; plane diff confined to x 0..7, y 0..7.
- Existing diagnostics under a replaced stub (Y): five scenarios pass; five keys move (see S2-10).
- Listing checker with the contract-2 listing and one inventory line added: pass, `listings=5`.
