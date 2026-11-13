---
phase: 06-stic-video-first-pass
plan: 13
subsystem: fixtures, data/scenarios, data/golden, tests/run, tests/installed-consumer, tools/check (video step, pkg-install), runner-process, docs
tags: [fixture, diagnostic, original-diagnostic, hand-assembled, sdbd, scenarios, mutants, installed-consumer, frame-hash-tie, D-02, D-14, D-17, D-19, D-20, A-04, VID-01, VID-02, VID-03, VID-04]
status: complete

requires:
  - phase: 06-stic-video-first-pass
    provides: "plan 06-01 oracle commit eae8ca5 (the derivation of the diagnostic, the frame literals F-D1 and F-D2, the listing SDBD rule); 06-09 video slots and buffers, frame stop; 06-10 and 06-11 both card renderers; 06-12 runner capture video and frame asserts, ivsr 3, golden video row; gate steps=36/36 controls=335/335 faults=221/221 at plan start"
provides:
  - "fixtures/FX-diag-stic-1: the third original diagnostic, 133 words at 5000-5084, hand-assembled (D4) from docs/video/diag-stic-1-derivation.md, id ivs-diag-stic-1, image diag-stic-1.ivsc in the build tree only"
  - "scenarios diag-stic-fgbg-pass, diag-stic-cstack-pass (pass) and video-assert-mismatch (guest-fail 10); data/golden/diag-stic-{fgbg,cstack}-pass.expect (hand-derived, tier original-diagnostic) and .digests (self-recorded by ivs-run --update-golden)"
  - "the video step hash tie (token video-frame-hash-expect-drift, header line `# frame: <n> <id>`) and its control"
  - "run.diag.stic.* (5 in-process ids, tests/run/run_diag_stic.c), expect-inc.cmake video forms, IvsVideoFrames.cmake FRAMESET=diag"
  - "data/expected/diag-mutations-stic.ids: six mutants and one negative with an explicit from column, a scenario column and comma-joined lists"
  - "the installed consumer third replay (stic. lines) compared by pkg-install with the runner golden; two more self-tests (15)"
affects: [06-14]

actuals:
  tasks: 5
  commits: 7
plan_head_before: 8004186
plan_head_after: c08293fcefc495fc8c11e0e98f427bf83ff33056

key-decisions:
  - "The listing checker decodes every word as program, so the 32 pattern decles carry the row each encodes and the decles 04 02 01 of card 2 decode as one three-decle J annotated `-> 0001` (never executed): 5 listed branches, 4 in code"
  - "The tie between an .expect hash and its frame literal is a header line `# frame: <n> <id>` plus a new video step token; the toy tree gained one frame assert in toy.expect and toy.ivss"
  - "The in-process family reads the two literal frames through a second mode of cmake/IvsVideoFrames.cmake (FRAMESET=diag, ivs_diag_frames.inc) so the video suite table and its ids stay unchanged"
  - "The mutant inventory has a scenario column and comma-joined address, from and to lists, because a swap of the foreground and background fields of a 14-bit word changes both bytes of its SDBD immediate"

duration: about 3 hours elapsed on a loaded laptop (load 9 to 20)
completed: 2026-11-12
---

# Phase 6 Plan 13: diagnostic ivs-diag-stic-1 and end to end Summary

**A hand-assembled original diagnostic enables the display, copies four project-authored card patterns, writes the BACKTAB and halts, and the one frame it shows equals the frame hand-derived in plan 06-01 in both card modes, through the runner, the in-process API and the installed package; the gate ends `steps=36/36 controls=336/336 faults=221/221 listings=4 pkg-selftest=15/15 tests-missing=0` under `sh`, `dash` and the Ubuntu container.**

Oracle commit this plan follows (06-01 task 5): `eae8ca5`. The 06-01 oracle files (`data/stic/*`, `data/video/*`, `docs/video/frame-derivations.md`, `docs/stic/*`) were read and never edited; the derivation `docs/video/diag-stic-1-derivation.md` only gained a dated as-built section (f) with the original text kept.

## Commits

| Commit | Task | What |
|--------|------|------|
| `d8d7f72` | 1 tracer | listing, manifest, registrations, fourth image, first SDBD decode-order parity (`pack.words-ivs-diag-stic-1`), as-built section of the derivation |
| `cde8af9` | 2 | the two `.expect` files (assert lines and derivation text) |
| `4c1586b` | 2 | three scenarios, `.digests`, hash tie and control, `run.diag.stic.*` |
| `424401c` | 3 | six mutants and one negative, slice replays of the fgbg scenario |
| `4c04d20` | 4 | consumer third replay, comparison, two self-tests, consumer golden |
| `c08293f` | 5 | `docs/diagnostics.md` section, D4 effort data, `docs/packaging.md` |
| this commit | | SUMMARY, STATE, ROADMAP |

## Order of the `.expect` and the first run (stated exactly)

The `.expect` assert lines were written from the derivation and the frame literals before any scenario of this plan existed, and the commit `cde8af9` (`.expect` only) precedes the commit `4c1586b` that holds the scenarios and the `.digests`. Two precisions: (1) the Task 1 tracer, a scratch scenario in a temp directory that nothing recorded, ran the image through `ivs-run` against the `sha256-text` keys of F-D1 and F-D2 (oracle literals of 06-01) and matched both on the first run, before any `.expect` existed; (2) the three real scenarios were first run once (release build, output not recorded) after the `.expect` files were written but BEFORE `cde8af9` was committed; all 25 and 24 asserts passed and nothing in an assert line was changed afterwards (`git diff cde8af9 HEAD` of the two `.expect` files shows only the added header line `# frame: 0 F-D1` and `# frame: 0 F-D2`). The `.digests` goldens were written only by `ivs-run --update-golden` (dev runner; the dev and release reports are byte-equal and the golden step checks it).

## D4 effort data and the crossing rule

- Words derived: 133 (101 code, 32 pattern data), listing 220 lines with its header (14,629 bytes); the derivation is 126 lines (16,128 bytes; 121 lines from plan 06-01 plus the as-built section).
- Branches: 4 in code (`BZE` at 5008, `BNZE` at 5029, 5032, 505B), 1 more in the data table (the decode of the decles `04 02 01`, `J -> 0001`): 5 listed, limit 8.
- Errors caught by the listing checker: 0 (the first checker run was clean). Two slips were caught by hand before it: a first note gave `CMPI` the decle `03F8` (the family table says `XORI`; `0378` was used) and a branch annotation followed by a comma (the annotation rule would read it as missing). Caveat: the `BITS` column was produced by a scratch formatter from the hand-typed words, so `listing-bits-mismatch` was not exercised; words, rows, operand lines, displacements and annotations were typed by hand, and the rows of the data decles were looked up with a scratch script that repeats the checker's leaf rule.
- Errors found only by a run (escapes): 0. Both frames matched on the first run with no change to the listing.
- The crossing rule of the derivation: more than 170 words or more than 8 branches (133 and 5, not fired); an error that survives the checker and is found only by a run (none, not fired); a second STIC cartridge (one image serves both modes, every mutant patches it, not fired); a second author edits a listing (no, not fired). Evaluation: not crossed, D4 stands by its letter (three diagnostics is not above three), the decision for a fourth moves to plan 1 of Phase 7.
- Two findings for that decision: the checker treats data as program (each pattern decle needs a row, and one data triple decodes as a jump), so a data directive is the one thing this diagnostic asks of an assembler; and the cycle budget of the derivation is an inference that the first run corrected: the start of the `HLT` unit without the strobe is 82,740 ticks observed against 82,864 derived (31 cycles, the reset entry and the bus figures are inferences); with the display on it is 88,436 ticks inside the derived 87,388 to 88,948. Margins to the frame ends stay far above 4,000 ticks: 23,004 after frame 0 and 36,732 before frame 1 without the strobe, 28,700 and 31,036 with it.

## Mutants (judged by runner-process; each from word read from `ivs_pack --dump-words`)

| ID | scenario | addr from to | first failing clause (observed) |
|----|----------|--------------|---------------------------------|
| STIC-CARD-BYTE | fgbg | 506D 00FF 007F | guest-fail 10, `frame 0 pixel 152 88 == 7 got 10` |
| STIC-BG-FG-SWAP | fgbg | 5036,5037 0001,002A 0005,000A | guest-fail 10, `mem 02A3 == 2A01 got 0A05` |
| STIC-WORD-MOVED | fgbg | 5057 02EF 02EE | guest-fail 10, `mem 02EF == 1C0F got 0000` |
| STIC-ENABLE-ADDR | fgbg | 500D 0020 0022 | guest-fail 10, `frame 0 displayed == yes got no` (stop at 82,780 ticks) |
| STIC-MODE-SENSE | cstack | 5008 0204 020C | guest-fail 10, `frame 0 pixel 0 0 == 6 got 0` |
| STIC-GRAM-COUNT | fgbg | 5025 0020 001F | guest-fail 10, `frame 0 hash == 86b80eb7... got cfa82535...` (the changed row has no asserted pixel) |
| STIC-PAD-NEGATIVE | fgbg | 5059 0384 0320 | pass 0 (stop at 82,476 ticks) |

No mutant can lengthen a loop (every patched count stays positive, the patched branch stays forward), so none is judged timeout. The expected clause of each was written in the inventory before the mutant was run; a wrong clause and a wrong `from` word were each made to fail the case once and restored. `video-assert-mismatch` ends guest-fail naming `assert=24 line=37 frame 0 hash == 96b8...` (the hash with its first digit changed).

## Evidence

- Counts: tests-expected 4,382 to 4,422 (+40: 5 `run.diag.stic.*`, 3 scenario cases, 2 repeat, 1 slices, 7 mutants, 2 pack ids, each x 2 presets; the in-process ids are in the `run` suite and the process ids in `runner-process`), controls 335 + 1 (`video-frame-hash-expect-drift`) = 336, faults 221 + 0, pkg self-tests 13 + 2 = 15, listings 3 + 1, consumer lines 46 to 76, `video-hash-ties=2`.
- `run.diag.stic.*` in process: both modes from the same image, frame 0 equal to the literal plane (159 x 96 positions) and its text hash equal to the literal's key, slices of 1 (4,110 calls), 7 and 468 ticks give the same instance bytes, frames and all nine digests as one call, the flagged frame-stop sequence stops FRAME then CPU_HALTED over two calls with the same stream and the same end machine, and the patched strobe delivers frame 0 not displayed and all border color 0 with the halt earlier than the display-on halt. 0.1 s in the dev build. No fault collateral entry was needed (`run.diag.` was already allowed under RUN_STOP_BEFORE_OVERSHOOT).
- Consumer golden diff: `git diff 4c04d20^ 4c04d20 -- data/expected/pkg-consumer.golden` shows 30 added `stic.` body lines (load, step.1, input, video, slot0, pixels, two rects, continuity, nine `stic.mem`, `stic.stic-state`, `stic.frame-stop.1`, `.2`, `.digests`, seven `stic.digest.*`, `stic.second-instance`) and no removed body line; only the `#` header comment lines changed. `grep -c '^stic.digest.input'` is 0: the consumer's `stic.digest.*` lines and observed set (cpu mem images instance video audio; not input, not psglog) equal `data/golden/diag-stic-fgbg-pass.digests`, compared by `pkg-install`; the two earlier replays still show video not observed and equal their goldens. The consumer has no SHA code; observed[domain] is the count rule (input by applied events, none here).
- Golden classification: new files only (`git status --short data/golden` showed exactly the four new files before they were committed; no earlier `.digests`, `.expect` or `.refpcm` changed); consumer pin: additions only; control files: one new control directory and the toy `toy.expect` and `toy.ivss` gained one frame assert each.
- Cost: `runner-process` ctest 8.3 s release (06-12: 8.0 s) and 18 to 27 s dev under load 12 to 20 (06-12: 15.0 s; the dev build runs the sanitized `ivs-run` for 7 mutants, 3 scenarios, 2 repeats and 2 slice runs more). `gate-time total`: 96, 79, 124, 124 (a failed first run), 86 s locally per task, container 324 s. Hosted cost is unmeasured; the extra process time of this plan is about +12 s dev and +0.3 s release.

## Deviations from Plan

**1. [Rule 3] `input at_tick=0 c0=KEY_1` needs both controller words.** The scenario grammar refuses one word (`bad-arity`), so the Color Stack scenario says `input at_tick=0 c0=KEY_1 c1=0x0`.

**2. [Rule 3] Files outside the plan's list.** `tests/expect-inc.cmake` gained the four video forms (frame displayed, pixel, hash and video count; without them the `.expect` cannot be read by the in-process family) and `docs/runner.md` the two golden recipe lines; `cmake/IvsVideoFrames.cmake` gained a second mode (`FRAMESET=diag`) instead of two more entries in the video suite table, which keeps the ids and the loops of suite `video` unchanged.

**3. [Interpretation] Hash tie shape.** The `.expect` names its literal with a header line `# frame: <n> <id>`; the step `video` has the new token `video-frame-hash-expect-drift` and a stat `video-hash-ties`; the toy tree has one frame assert in `toy.expect` and `toy.ivss` (the golden step compares their assert lines, nothing parses the toy scenario).

**4. [Interpretation] Mutant inventory columns.** A scenario column was added (the Color Stack scenario suits the mode-sense mutant) and the three word columns accept comma-joined lists, because swapping the foreground and background fields of a 14-bit word changes both bytes of the doubled immediate; `run_mutant` takes the lists through `patch_args`.

**5. [Rule 1 - grammar] Data decoded as code.** The listing checker treats the 32 table decles as program words (see the D4 findings): rows by decle, and one decoded three-decle `J` with its annotation; both stay inside the limits and no derived word, address or expected value changed.

**6. [Other] The first run preceded the `.expect` commit** (stated above); nothing in an assert line changed after it.

## Decisions Made

- No requirement is closed here. VID-01 to VID-04 stay Pending in REQUIREMENTS.md: this plan supplies the end-to-end evidence (guest strobe inside the register window, GRAM and BACKTAB writes inside the budget, one hand-derived frame per mode through the machine, the runner and the installed package, and mutants that fail on the video clauses), but plan 06-14 owns the close-out, the claims and the traceability edit.
- The `.digests` were written with the dev runner (the plan does not name a preset); the golden step requires dev and release reports to be equal and it holds.

## Open Items

- Hosted cost of this plan is unmeasured (see Cost); the arbiter is the first hosted row, as for 06-12.
- The display is not sustained (no interrupt service routine): exactly one picture exists; a project interrupt routine and a data directive for listings are Phase 7 decisions (D4).
- The blind second reads of the Phase 6 literals, the shared-misreading caveat and the provisional status of every claim that will rest on these frames are unchanged (06-14).
- The derivation's cycle budget was 31 cycles above the observed run without the strobe; the margins hold, and no budget figure is asserted anywhere.
- Carried unchanged: Phase 4 and 5 deferrals, `sh tools/hygiene.sh --history` on earlier planning commits, the carried residual that the golden itself is still written with `fopen`.

## Final gate

```
sh tools/gate.sh:       gate: PASS steps=36/36 controls=336/336 faults=221/221 tests-expected=4422 tests-executed=4420 tests-skipped=2 tests-missing=0 listings=4 consumer-lines=76 pkg-selftest=15/15 video-hash-ties=2
dash tools/gate.sh:     the same counts (0 FAIL lines)
sh tools/gate-linux.sh: GATE_LINUX_PLATFORM= run, exit 0, last line of build/gate-linux.log starts gate: PASS, x86_64 gcc 13.3.0 cmake 3.28.3, steps=36/36 controls=336/336 faults=221/221 listings=4 pkg-selftest=15/15 tests-missing=0, 0 FAIL lines (gate-time total=324)
```

## Threat Flags

None new. T-06-18 (vacuous video assert): `frame-not-captured` and `stream-without-buffer` of 06-12, `video-assert-mismatch`, six mutants including the display-less one, the hash tie to the literal and the fault of 06-12. T-06-19 (rights-bearing bytes): project-authored geometric patterns only, listing and manifest author-attested, `.ivsc` images stay in the build tree (`git ls-files '*.ivsc' '*.bin' '*.rom'` is empty). T-06-SC: no installs.

## Self-Check: PASSED

Created files exist (`fixtures/FX-diag-stic-1/{diag-stic-1.listing.txt,manifest.json}`, `data/scenarios/diag-stic-{fgbg,cstack}-pass.ivss`, `data/scenarios/video-assert-mismatch.ivss`, `data/golden/diag-stic-{fgbg,cstack}-pass.{expect,digests}`, `data/expected/diag-mutations-stic.ids`, `tests/run/run_diag_stic.c`, `tools/check/controls/video-frame-hash-expect-drift/`); commits `d8d7f72`, `cde8af9`, `4c1586b`, `424401c`, `4c04d20`, `c08293f` are ancestors of HEAD.
