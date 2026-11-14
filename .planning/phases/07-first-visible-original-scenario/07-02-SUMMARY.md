---
phase: 07-first-visible-original-scenario
plan: 02
subsystem: oracle of the first visible original program (derivation, frame literals, frame-pair check, shape lint, SCN-01 in the important-gates registry, blind-read request)
tags: [W1, oracle, no-C, lamp-tone, video-frame-pair-region, golden-shape, important-gates, SCN-01, D-21, D-22, D-23, D-24, D-25, D-27, A-02..A-08, V4, V5, V6, V7, V8, V9, V10, V13, V14]
status: complete

requires:
  - phase: 07-first-visible-original-scenario
    provides: "plan 07-01: gate steps=36/36 controls=344/344 faults=222/222 video-frames=10 tests-skipped=2 tests-missing=0, HEAD 1778ca2 at the start of this plan"
provides:
  - "docs/video/demo-lamp-tone-1-derivation.md: sections (a) to (j) of the oracle of ivs-demo-lamp-tone-1, written with no C, no listing, no scenario, no golden and no run"
  - "data/video/frames/F-S1..F-S6.frame (hand-derived lamp frames, 16 registered frames) and data/expected/video-frame-pairs.ids (7 declared pairs)"
  - "gate token video-frame-pair-region (control video-frame-pair-region-drift) and token golden-shape (controls golden-shape-no-input, golden-shape-no-progress)"
  - "SCN-01 accepted by important-gates, 3 registry lines, control important-gates-req-uncovered-phase7, the toy line appended to the toy registry and to the eight whole-file control overlays"
  - "docs/video/demo-lamp-tone-1-second-read-request.md (state pending) with the fixed record head"
affects: [phase-7-plans-07-03-to-07-12, blind-read-record, plan-07-06-program, plan-07-07-scenarios, plan-07-08-mutants]

actuals:
  tokens: 75546
  tasks: 4
  commits: 4
plan_head_before: 1778ca23d7c18e620eed1a7b3c3d201aee961e2e
plan_head_after: 4e0febef675c2e81fe38d6c0caf353625218b1eb

key-decisions:
  - "Re-arm form B (the main-loop wait), delay N = 500, band N 183 to 808; startup band: the first EIS of the program at frame-0 cycle 6,564 to 6,631, so the first routine is frame 1's"
  - "Channel C is the constant-level gate (mixer 003C); the changed port's tone amplitude is written by the next frame's routine from a pending word (amplitude plus one); the PSG init is 6 writes, not the 7 of the discussion note"
  - "Settling rule read as a time distance (every event at least 16,832 ticks from a frame end and 17,332 ticks from the next sample instant), not as unasserted frames: frames 3, 5, 7, 9 and 11 carry the response of the previous event and frame 12 the last one"
  - "Three audio windows with exact levels (silence, A=0 B=0 C=15 twice) and silence after the release; no tone phase and no audio emitted count is asserted"

key-files:
  created:
    - docs/video/demo-lamp-tone-1-derivation.md
    - docs/video/demo-lamp-tone-1-second-read-request.md
    - data/video/frames/F-S1.frame
    - data/video/frames/F-S2.frame
    - data/video/frames/F-S3.frame
    - data/video/frames/F-S4.frame
    - data/video/frames/F-S5.frame
    - data/video/frames/F-S6.frame
    - data/expected/video-frame-pairs.ids
    - tools/check/controls/video-frame-pair-region-drift/
    - tools/check/controls/golden-shape-no-input/
    - tools/check/controls/golden-shape-no-progress/
    - tools/check/controls/important-gates-req-uncovered-phase7/
    - tools/check/controls/_good/tree/data/video/frames/T-3.frame
    - tools/check/controls/_good/tree/data/expected/video-frame-pairs.ids
  modified:
    - tools/check/video.cmake
    - tools/check/steps/video.sh
    - tools/check/steps/golden.sh
    - tools/check/steps/important-gates.sh
    - data/expected/video-frames.ids
    - data/expected/important-gates.ids
    - data/expected/controls/video.ids
    - data/expected/controls/golden.ids
    - data/expected/controls/important-gates.ids
    - tools/check/controls/_good/tree/data/expected/video-frames.ids
    - tools/check/controls/_good/tree/data/expected/important-gates.ids
    - tools/check/controls/important-gates-*/overlay/data/expected/important-gates.ids (eight files)
    - .planning/STATE.md
    - .planning/ROADMAP.md

completed: 2026-11-13
---

# Phase 7 Plan 02: the oracle of Lamp Tone Summary

**The first visible original program ("Lamp Tone") is specified, its arithmetic and every expected value (RAM words at four stops, 30 PSG records, exact audio windows, six index frames, seven frame pairs, four assert blocks, thirteen mutant rows) are derived and committed before any program, listing, scenario or golden exists, and the gate now refuses a frame pair that differs outside its rectangle and a playable-tier predicate without input or progress; the blind second read is requested.**

## Commits

| Commit | Task | What |
|--------|------|------|
| `8ded8dd` | 1 (tracer) | derivation header and section (a) between the blind-spec markers; F-S1 and F-S2; the pair inventory with `F-S1 F-S2 48 32 64 8 128`; token `video-frame-pair-region`, stat `video-frame-pairs`, control `video-frame-pair-region-drift` on a toy pair T-1 T-3 |
| `d67b180` | 2 | sections (b) to (j); F-S3 to F-S6; six more pair lines; assert blocks of `pass` (55), `idle` (29), `shift` (55), `swap` (49) |
| `c38d85e` | 3 | token `golden-shape` and its two controls; SCN-01 accepted by important-gates with three registry lines; control `important-gates-req-uncovered-phase7`; the toy line appended to the toy registry and to the eight whole-file overlays |
| `4e0febe` | 4 | the blind second-read request (state pending) and a neutral format example in rule A4 |
| the commit that adds this file | | this SUMMARY, STATE.md, ROADMAP.md |

## Oracle commit

The last task commit of this plan (it touches the oracle files last; the commit that adds this SUMMARY changes planning files only):

oracle-commit: 4e0febef675c2e81fe38d6c0caf353625218b1eb

The orchestrator now runs the blind reader on the text between the `blind-spec` markers (command in the request), diffs the answers against sections (c) to (f) and writes `docs/video/demo-lamp-tone-1-second-read-record.md` with the head the request fixes (`state:`, `oracle-commit:`, `corrections:`, the `correction:` lines). Any oracle correction lands in the commit that adds the record; that commit is the frozen baseline.

## What was built

- Section (a), the specification: ports `c0` at `$01FE` and `c1` at `$01FF`; lamp i shows bit 7 minus i of the port byte, lit (index 2) when the bit reads 0; one solid project-authored GRAM card written first; lamp words `$0801` and `$0802` at `$0256..$025D` (row 4, port 0) and `$0292..$0299` (row 7, port 1), rectangles x 48 to 111 with y 32 to 39 and y 56 to 63; RAM `$02F0` marker `03C5`, `$02F1` counter, `$02F2..$02F5` last byte and change counter per port, `$02F6/$02F7` pending amplitude words, stack from `$0340`; the routine in eight steps; the PSG rule set of A-05.
- Section (b), the arithmetic: startup rule and a reference init of 6,531 cycles (first `EIS` at frame-0 cycle 6,564 to 6,631, the card complete by cycle 329 against the 3,780 window); strobe at frame cycle 33 to 41 against 2,000; every BACKTAB word written by cycle 1,151 against the first latch at 3,848 (the row 4 latch is at 7,496); the re-arm rule 3,777 <= c_E <= 14,930 with both forms compared (band widths 11,151 and 11,153 cycles: neither is wider, so the default form B stands); the routine's cycle budget table with W = 1,010 to 1,242 per frame; the counter rule `floor(T / 59,736)` (806,568 gives `000D`).
- Section (c): event distances to the sample instants, the settling reading, the per-frame state table, the RAM words at the four stops.
- Section (d): the 30-record PSG log with the rule behind each record, record counts at the stops (9, 17, 23, 30), write times, four audio windows, why no tone phase and no `audio emitted` is asserted.
- Section (e): lamp patterns, six literals with their `sha256-text`, seven pairs with derived counts (128, 256, 128, 64, 192, 384, 320), probes, the frame-to-literal tie.
- Sections (f) to (j): four assert blocks as fenced text; thirteen mutant rows; claim allocation and wording; rights checklist; open items for the blind reader.
- Checker work: `video-frame-pair-region` compares two planes position by position (rows that are equal are skipped), rejects a line outside the grammar, an unregistered frame, a rectangle outside the plane, a count of 0, a differing position outside the rectangle and a wrong count; `golden-shape` lints a predicate file whose header tier is `playable-guest-progress` (keywords `no-input`, `input-after-bound`, `no-progress`, `no-frame`, `no-psglog`); `important-gates` accepts SCN-01.

## Chosen values (the numbers later plans read)

- Re-arm form: the main-loop wait (form B), `N = 500`; band `N` 183 to 808 (lower edge from the quietest routine with t0 = 22, upper edge from the busiest routine with 13 stalls of 117 cycles); `c_E` for N = 500 lies between 9,186 and 9,597 (5,409 above the lower edge, 5,333 below the upper). Mutant values: `SCN-REARM-SHORT` N = 100 (`c_E` 2,538 to 2,778), `SCN-REARM-LONG` N = 1,150 (`c_E` 4,758 to 5,115 cycles into the next frame), negative control N = 300 (`c_E` 5,862 to 6,129).
- Startup band: the first `EIS` at frame-0 cycle 6,564 to 6,631 (frame-1 band: the first routine is frame 1's, `frame 0 displayed == no`).
- Assert counts per block: `pass` 55, `idle` 29, `shift` 55, `swap` 49 (all at most 56).
- PSG record counts: 30 for `pass`, `shift` and `swap` (6 init records and 24 from frames 2 to 13), 6 for `idle`; counts at the stops of `pass`: 9, 17, 23, 30.
- Pair lines (`data/expected/video-frame-pairs.ids`): `F-S1 F-S2 48 32 64 8 128`, `F-S1 F-S3 48 32 64 8 256`, `F-S2 F-S3 48 32 64 8 128`, `F-S1 F-S4 48 56 64 8 64`, `F-S1 F-S5 48 32 64 32 192`, `F-S1 F-S6 48 32 64 32 384`, `F-S5 F-S6 48 32 64 32 320`.
- Audio ticks of `pass`: silence at 100000 and 690000, `A=0 B=0 C=15` at 140000, 160000, 740000 and 760000; `idle`: silence at 100000, 400000, 800000.
- Frame ties (hash asserts): `pass` and `shift` 3 F-S2, 5 F-S3, 7 F-S4, 9 F-S5, 11 F-S1, 12 F-S6; `idle` 1, 6, 12 all F-S1.

## Deviations from Plan

**1. [Interpretation, plan wording] The settling rule is a time distance, not a set of unasserted frames.** The must_have says "no assert targets the event frame or the next one". With events two frames apart every frame is an event frame or the one after, and the plan itself needs F-S6 (the last event) asserted at frame 12, the frame after its event frame, because the bound of D-06 delivers no later frame. The derivation (section (c)) therefore proves the rule by distances (every event at least 16,832 ticks from a frame end, at least 17,332 ticks from the next sample instant, against the 10,000 of the verification note) and asserts the response of each event two frames later (frames 3, 5, 7, 9, 11) plus frame 12 for the last event. Frames 1, 2, 4, 6, 8, 10 carry no assert in `pass` and `shift`.

**2. [Rule 3 - minor scope] The stat `video-frame-pairs` and the step header.** The plan allowed a new stat only if the convention permits; it does (the gate line gains the token `video-frame-pairs=7`, no existing token changes). `tools/check/steps/video.sh` has a header comment that lists the stats, which I updated, a comment-only edit of a file that `files_modified` does not list.

**3. [Interpretation] Toy frame T-3, not T-2.** The pass-through toy pair is T-1 and T-3: the existing control `video-frame-unregistered` renames T-1 to T-2, so T-2 would have collided. T-3 is T-1 with the foreground of the cell (11,19) one color lower (9 differing positions in x 152 to 158, y 88 to 95). The control also pins its reason with a second EXPECT line (`outside the rectangle 152 88 6 8`).

**4. [Interpretation] Evidence-tiers overlay in the two shape controls.** The toy tier list has only two tiers; each shape control overlays the list with a third line `playable-guest-progress`, so `golden-header-missing-tier` is not raised and the intended token is the only finding (A-22 called the extra finding harmless; this is cleaner). The lint keys on the header tier text as planned.

**5. [Interpretation] Unit correspondence in `golden-shape`.** `input-after-bound` is also raised when the units of the input lines and the `run-to` lines do not correspond (at_tick with ticks, at_cycle with cpu-cycles), because the checker holds no timebase to convert; the scenarios of this phase use ticks. Scratch runs on copies of the toy tree confirmed a passing playable file (ticks, and cycles with the progress assert in a later step) and each keyword (`input-after-bound` for equal and for mismatching units, `no-progress` with the only mem assert before the first input, `no-frame`, `no-psglog`).

**6. [Design choice recorded] PSG init of 6 writes.** The discussion note's 7 init writes set the fine periods to `FF`; the program here writes the mixer, the two coarse registers and the three amplitudes (all zero) and writes a fine period only when a port changes (A-05 allows an equivalent mechanism). It does not rely on the reset state of any register it uses.

**7. [Interpretation] `audio emitted` is not asserted.** A stop tick overshoots to the end of the instruction under way, which is not hand-known, and 806,568 is not a multiple of 16; the derivation says so (section (d)) instead of naming a count.

**8. [Process] A throwaway typing aid.** Plane text, hashes, pair counts, the block texts and the cycle sums were expanded or cross-checked with a Python script written from the rules of section (a), kept in a scratch directory outside the repository; it never read the core, a run or the discussion notes' results. The hand derivations came first and the script agreed with every one.

No Rule 1 or 2 fixes were needed; no auth gates; no package install (T-07-SC).

## Decisions Made

- Form B with N = 500 sits near the middle of a band about 11,150 cycles wide; the mutants sit outside it by more than the uncertainty of W and of the stall size, so plan 07-08 needs no band-edge hugging.
- Only the last step of `pass` carries frame, log and audio asserts, so the `fails=` sets of the isolation mutants separate the channels (the report lists the asserts of the first failing step only).
- No requirement is closed: SCN-01 needs the program, the scenarios and the consumer replay (plans 07-06 to 07-09) and stays Pending.

## Open Items

- Orchestrator action: run the blind reader per the request and write `docs/video/demo-lamp-tone-1-second-read-record.md`; plan 07-06 halts unless it exists with `state: closed` and an `oracle-commit:` equal to the line above.
- The cycle figures (the entry of 9 cycles, the 108 to 117 stall per window, the reset entry up to 100 cycles, flag behaviour of the reference sequence) are inference from the tables; plan 07-06 measures them in its tracer and records them in section (k). A real EIS position far from 9,186 to 9,597 would not break the band (about 5,300 cycles of room on both sides) but must be reported.
- The text tripwire `plays|playable` joins in plan 07-12: the derivation lines that contain `playable` already carry the waiver tag, which the freeze command removes before comparing.
- The gate summary line gained `video-frame-pairs=7`; later plans that compare the whole line against an older one see one added token.
- Carried unchanged: Phase 4 and 5 review deferrals, `sh tools/hygiene.sh --history` on earlier planning commits.

## Final gate

```
sh tools/gate.sh:     gate: PASS steps=36/36 controls=348/348 faults=222/222 ... tests-expected=4432 tests-executed=4430 tests-skipped=2 tests-missing=0 ... video-frames=16 video-hash-ties=2 video-frame-pairs=7 (0 FAIL lines; gate-time total=109)
dash tools/gate.sh:   the identical summary line (same md5 db661aa07f16091c24519de1803ab12d; 0 FAIL lines; gate-time total=98)
sh tools/gate-linux.sh: exit 0, build/gate-linux.log ends gate: PASS (x86_64, gcc 13.3.0, cmake 3.28.3), steps=36/36 controls=348/348 faults=222/222 tests-expected=4432 tests-executed=4430 tests-skipped=2 tests-missing=0 video-frames=16 video-hash-ties=2 video-frame-pairs=7, 0 FAIL lines (gate-time total=456; summary line md5 7c92389ad6e346bcf2271f2dbb0733be, a different line from the macOS one in the toolchain fields only)
```

## Known Stubs

None. No file in this plan holds a placeholder that reaches a UI; the derivation is complete as an oracle and the program it specifies is written by plan 07-06.

## Threat Flags

None new. T-07-02 is mitigated as planned (no C, no listing, no run in this plan; each value has its reason; blind read requested before the program), T-07-03 (own words, no third-party glyph or tune, tripwire words and `[nc]` tags checked by the `text`, `glossary`, `hygiene` and `denylist` steps), T-07-04 (each new check has a control that fails for its stated reason, shown also by scratch runs of the keywords the controls do not cover). T-07-SC: no installs.

## Self-Check: PASSED

The derivation, the request, the six frame literals, the pair inventory and the four control directories exist; commits `8ded8dd`, `d67b180`, `c38d85e` and `4e0febe` are ancestors of HEAD (`git rev-list --count 1778ca23d7c18e620eed1a7b3c3d201aee961e2e..HEAD` printed 4 before this file's commit); `grep -c '^oracle-commit: [0-9a-f]\{40\}$'` on this file prints 1; `git diff --quiet 903276757b0dbd901b15dbc00b9813089c1b578e -- data/golden data/scenarios fixtures tools/asm` exits 0 (no golden, scenario, fixture or assembler file); no record file `docs/video/demo-lamp-tone-1-second-read-record.md` exists; `grep -c '^assert '` on the derivation prints 188 and each block has exactly one begin and one end marker; the hash of every `assert frame <n> hash` line equals the `sha256-text` of the literal named by its `# frame:` tie line (checked by script for `pass`, `idle` and `shift`).
