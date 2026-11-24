---
phase: 08-stic-completion-save-state-performance-baselines
plan: 07
subsystem: Phase 8 wave A, plan 5 - the original diagnostic program ivs-diag-stic-2 (one image, a controller-word selector, five single-feature scenarios and an idle control), its six scenarios with hand-derived predicates and recorded goldens, two metamorphic pair lines, six Release-only program mutants with recorded failing sets, and the as-built appendix of the frozen derivation
tags: [A, VID-05, program, scenarios, goldens, mutants, D-04, D-05, D-10, D-37, D-40, D-43, T-08-19, T-08-20, T-08-21]
status: complete

requires:
  - phase: 08-stic-completion-save-state-performance-baselines
    provides: "plan 08-06 end state (renderer, collision, delay and strips held to the frozen frames); the frozen oracle of plan 08-03 (docs/video/diag-stic-2-derivation.md sections (a) to (j), data/video/frames/F-M0..F-M9, the second-read record, frozen since 9250265); counts at plan start steps=37/37 controls=371/371 faults=226/226 tests-expected=4738 tests-executed=4734 tests-skipped=4 goldens=15 manifests=10 effective-yes=7 listings=6 listing-words=520 video-hash-ties=11 consumer-lines=135 pkg-selftest=17/17 (re-measured with a full gate run at the start, identical)"
provides:
  - "fixtures/FX-diag-stic-2/{diag-stic-2.asm,diag-stic-2.listing.txt,manifest.json}: the program ivs-diag-stic-2 (866 words at 5000-5361), generated listing, manifest FX-diag-stic-2 (redistributable, third-party attestation, recipe digests)"
  - "the image diag-stic-2.ivsc in the ivs_images target (build tree only), registered in fixtures, listings, pack and asm ids and the content-dir copy list; no consumer, no pkg-install change"
  - "six scenarios diag-stic2-{idle,mob,border,hdelay,vdelay,collision}-pass with .expect predicates (tier original-diagnostic) equal to the derivation's section (f) blocks, and .digests goldens written only by ivs-run --update-golden; frame ties F-M0 to F-M6 (video-hash-ties 11 to 18)"
  - "two pair lines (idle against mob and against border, differ=digest.video,digest.input) and their ids"
  - "data/expected/diag-mutations-stic2.ids (six rows with fails= sets), the sibling mutant loop of tests/runner-process-tap.sh with the fifth argument (build configuration, a genex in tests/CMakeLists.txt): full run on Release, static checks only otherwise, same ids, no skip-ok"
  - "docs/diagnostics.md and docs/evidence-tiers.md list the program; section (k) of the derivation records the program as built, the measured margins and the mutant sets"
affects: [08-08, 08-09]

actuals:
  tokens: 33568
  tasks: 3
  commits: 3
plan_head_before: a9881e6739d3c3ac375823106c836b2d601a881f
plan_head_after: 3821cfd4d5d46428bd6d97c7cec49dbc2dd981c3

key-decisions:
  - "The scene words are a table in the cartridge (ten tables of 27 words, two decles per word, low byte first, read with SDBD MVI@) written by a loop of eight groups of three words plus three straight words: 761 cycles for the 27 words where the derivation's reference loop costs 1,026, so the longest routine (selector BD, frame 3) ends its register work at frame cycle 1,634, 266 cycles under the bound of 1,900 (the reference sequence had 1,745)"
  - "The scene choice is a compare chain that loads the scene number (R2) and the table address (R3) directly, and the collision work of A6 step 5 is two small loops through R4 and R5; no arithmetic on scene numbers, no JSR, no stack use beyond the stub's return address"
  - "A5 followed to the letter: the initialization clears $0200 to $02EF only and does not store the frame counter, so $02F1 relies on the zero power-on RAM of the profile (the default of power_on_fill.ram); the idle, mob, border, hdelay, vdelay and collision scenarios all read a counter of 10 at the stop"
  - "A table word is two decles, so a mutant of a table word patches the decle that carries the changed bits (five of the six rows reach the derivation's word exactly; STIC2-MOB-X-ZERO patches the low decle, so the X word becomes 0600, whose X coordinate is 0, the same disabled MOB the derivation intends)"
  - "Release-only without skip-ok (D-40): the same six ids run in both presets; in a non-Release build each makes its static checks (listing word, lists, patch changes the image, scenario file, fails= form) and reports ok without running the scenario; proven able to fail by four ad hoc probes"

key-files:
  created:
    - fixtures/FX-diag-stic-2/diag-stic-2.asm
    - fixtures/FX-diag-stic-2/diag-stic-2.listing.txt
    - fixtures/FX-diag-stic-2/manifest.json
    - data/scenarios/diag-stic2-idle-pass.ivss
    - data/scenarios/diag-stic2-mob-pass.ivss
    - data/scenarios/diag-stic2-border-pass.ivss
    - data/scenarios/diag-stic2-hdelay-pass.ivss
    - data/scenarios/diag-stic2-vdelay-pass.ivss
    - data/scenarios/diag-stic2-collision-pass.ivss
    - data/golden/diag-stic2-idle-pass.expect
    - data/golden/diag-stic2-idle-pass.digests
    - data/golden/diag-stic2-mob-pass.expect
    - data/golden/diag-stic2-mob-pass.digests
    - data/golden/diag-stic2-border-pass.expect
    - data/golden/diag-stic2-border-pass.digests
    - data/golden/diag-stic2-hdelay-pass.expect
    - data/golden/diag-stic2-hdelay-pass.digests
    - data/golden/diag-stic2-vdelay-pass.expect
    - data/golden/diag-stic2-vdelay-pass.digests
    - data/golden/diag-stic2-collision-pass.expect
    - data/golden/diag-stic2-collision-pass.digests
    - data/expected/diag-mutations-stic2.ids
    - .planning/phases/08-stic-completion-save-state-performance-baselines/08-07-SUMMARY.md
  modified:
    - tools/pack/CMakeLists.txt
    - tests/CMakeLists.txt
    - tests/runner-process-tap.sh
    - data/expected/fixtures.ids
    - data/expected/listings.ids
    - data/expected/runner-scenarios.ids
    - data/expected/scenario-pairs.ids
    - data/expected/tests/pack.ids
    - data/expected/tests/asm.ids
    - data/expected/tests/runner-process.ids
    - docs/diagnostics.md
    - docs/evidence-tiers.md
    - docs/video/diag-stic-2-derivation.md
    - .planning/STATE.md
    - .planning/ROADMAP.md

requirements-completed: []

completed: 2026-11-14
---

# Phase 8 Plan 07: the original diagnostic program ivs-diag-stic-2 Summary

**A cartridge program written from the frozen specification alone (section (a) of the derivation, A1 to A11 with the corrected A5) draws every hand-derived picture and writes every hand-derived collision word of the oracle on the first execution: six scenarios, selected only by the controller word read at `$01FE` at tick 0, match their section (f) blocks line for line (15, 21, 13, 12, 12 and 23 asserts, ten captured frames each), the six scenarios tie seven more frames to F-M0 to F-M6 (video-hash-ties 11 to 18: idle one, mob three, border, hdelay and vdelay one each), the idle control and the two pair lines show that the picture follows the selector and that the program adds nothing of its own, and six one-word mutants each fail their scenario on the Release build with a recorded failing-assert set that contains the derivation's first clause. No oracle line, frame or hash was edited and no assert disagreed with the program. The final gates pass under sh, dash and the Ubuntu container with `steps=37/37 controls=371/371 faults=226/226 fault-runs=226 tests-expected=4772 tests-executed=4768 tests-skipped=4 tests-missing=0 goldens=27 manifests=11 effective-yes=8 listings=7 video-hash-ties=18 consumer-lines=135 pkg-selftest=17/17`. VID-05 stays Pending (it closes in plan 08-08).**

## Commits

| Commit | Task | What |
|--------|------|------|
| `9aaa8e9` | 1 (tracer) | program source, generated listing, manifest, image registration (ivs_images, copy list, fixtures, listings, pack, asm ids), `diag-stic2-idle-pass` with its `.expect` and its runner-written `.digests`, `docs/diagnostics.md` |
| `12ad7b7` | 2 | five feature scenarios with `.expect` and `.digests`, frame ties F-M1 to F-M6, two pair lines and ids |
| `3821cfd` | 3 | six mutants and their inventory, the sibling loop and the fifth argument, the id registrations, docs, the as-built appendix of the derivation |
| the commit that adds this file | | this SUMMARY, STATE.md and ROADMAP.md |

## The program

`fixtures/FX-diag-stic-2/diag-stic-2.asm` assembles to **866 words** at `5000` to `5361` (`listing-words` 520 to 1,386): 302 words of code and fill (reset branch, the fill words `5002` and `5003`, the routine `5004` to `50E3`, the initialization `50E4` to `5125`, the wait and idle branch `5126` to `512D`), the 24 card bytes (`512E` to `5145`) and ten scene tables of 27 words, two decles per word (`5146` to `5361`, 540 words). Tool versions: `ivs_asm 1` (`ivs_asm --version`; SHA-256 of `tools/asm/asm.c` `0db882e4...`), `ivs_pack` prints no version and is identified by its source digest (`tools/pack/pack.c` `f760d50e...`), both recorded in the manifest recipe. The derivation's blind-spec range hashes to `f0e0c8767e22802f4fc3d14f92e2e1f4089aa91ea44ae06b6d086a8ef36e84bf` before and after this plan (unchanged); the sections (a) to (j) and `data/video/frames` show no difference against the blind-read commit `9250265`, and the derivation diff has zero deleted lines.

- **Initialization (A5).** Three cards from a 24-decle ROM table to `$3800` (816 cycles), the selector (low byte of `$01FE`) to `$02F2`, the 240 BACKTAB words cleared (`$0200` to `$02EF`), the word `0812` at `$0282` and, for the selector BD only, `16DB`, `1000` and `37FF` at `$0285`, `$0288` and `$02BE`, the marker `03C5` last, then `B arm` (the `EIS` of the wait).
- **Routine (A6).** Strobe first, the mode read, the counter, the scene choice (R2 scene number, R3 table), the collision work for BD only (frame 1 clear, frame 3 read with `AND 03FF` into `$0300` to `$0307` then clear, frame 4 read into `$0308` to `$030F`), the color stack and border registers for every selector except BD, the 27 scene words from the table, the re-arm (`DECR R6`, `J wait`, N = 500, `EIS`, idle branch).
- **Cards and tables.** The three cards are the solid block, the staircase and the 5 x 5 block of A2. A throwaway script, never committed and not reading the core, parsed the table of A7 and the card bytes of A2 out of the derivation and required every decle of the listing to equal them (ten scenes, 27 words each, and the cards): equal.
- **No rights-bearing byte.** No glyph, no GROM read, no EXEC call, no text; the manifest carries `redistributable: yes` with the third-party attestation of the Lamp Tone manifest; the image is built into the build tree only; `git ls-files` lists no `.ivsc`, `.bin`, `.exec` or `.rom` file.

## Scenarios (per-scenario counts)

| Scenario | Selector (key) | Asserts (cap 64) | Captured frames (cap 16) | Frame ties |
|----------|----------------|------------------|--------------------------|------------|
| `diag-stic2-idle-pass` | FF (no input line) | 15 | 10 | frame 5 F-M0 |
| `diag-stic2-mob-pass` | 7E (KEY_1) | 21 | 10 | frame 2 F-M1, frame 5 F-M2, frame 8 F-M3 |
| `diag-stic2-border-pass` | BE (KEY_2) | 13 | 10 | frame 5 F-M4 |
| `diag-stic2-hdelay-pass` | DE (KEY_3) | 12 | 10 | frame 5 F-M5 |
| `diag-stic2-vdelay-pass` | 7D (KEY_4) | 12 | 10 | frame 5 F-M6 |
| `diag-stic2-collision-pass` | BD (KEY_5) | 23 (one range assert) | 10 | none (RAM markers, two probes) |

Each scenario has one step, `run-to 640000 ticks`, and the lines `input`, `run-to` and `assert` equal the derivation's block for that id, in order (the verify command of both tasks compared them with `cmp`; the `.expect` asserts equal the block's asserts). Every `.expect` holds one `Excluded on purpose` list that names machine time, the stack word, all CPU registers and every pinned item PD-1 to PD-13. Every `.digests` was written only by `ivs-run --update-golden` on the dev build; each has `video=observed`, `output.video.frames 10` and `video.chain.check pass`; the idle golden has `input=none`, the other five `input=observed`. Hash ties: `video-hash-ties` 11 to 12 (idle) to 18 (the other five: mob three, border, hdelay, vdelay one each). Settling margins: no frame assert names frame 0 or frame 1 in a scenario with the tick-0 input; the scene changes of `mob` are at frames 4 and 7 and the asserted frames are 2, 5 and 8; the probes of `border`, `hdelay` and `vdelay` avoid every position that PD-8 and PD-9 move. The two pair lines (`diag-stic2-idle-pass` against `-mob-pass` and against `-border-pass`, `differ=digest.video,digest.input equal=-`) are judged ok in both presets.

## Measured margins (frame cycles of 4 ticks)

Measured with the runner on the Release build by stopping at a tick bound and asserting R7 in throwaway scenarios (never committed), one cycle of resolution; the full list is in section (k) of the derivation. Strobe store complete by cycle 29 (A9 allows 60). Register work ends (program counter at the re-arm) at 1,031 to 1,098 for `mob`, 1,039 `border`, 1,054 `hdelay`, 1,069 `vdelay`, 1,079 idle and 1,073 to 1,634 for `collision` (**1,634 in frame 3, the longest**, 266 below the bound 1,900, 366 below the window close at 2,000; frame 1 1,228, frame 4 1,428). The `EIS` of the re-arm completes between 9,226 and 9,934: 5,449 above 3,777 and 4,996 below 14,930. The first `EIS` of the initialization completes at frame-0 cycle 6,739 (idle) and 6,812 (BD), 2,962 above 3,777. The stop at tick 640,000 keeps the 17,096 ticks before the end of frame 10 of the derivation.

## Mutants (D-10, D-37, D-40)

Six rows of `data/expected/diag-mutations-stic2.ids`, each `guest-fail` exit 10, run in the sibling loop of `tests/runner-process-tap.sh`. "Static" is the result on the dev (Debug, sanitized) preset: the five static checks pass and the id is reported ok without a scenario run (the same six ids in both presets, no `skip-ok`, `tests-skipped` stays 4).

| Mutant | Scenario | Word (listing address) | Unpatched | Patched | First failing clause | Failing set (Release) | Dev preset |
|--------|----------|------------------------|-----------|---------|----------------------|-----------------------|------------|
| `STIC2-SELECTOR-COMPARE` | mob | `5010` (the compare constant) | `007E` | `007F` | `mem 02F3 == 0003 got 0000` | 1.5, 1.9, 1.10, 1.11, 1.12, 1.14, 1.15, 1.16, 1.17, 1.18, 1.19, 1.20 | static ok |
| `STIC2-MOB-X-ZERO` | mob | `517C` (low decle of X0, scene 1) | `009E` | `0000` | `frame 2 hash == a00823f4...` got the hash of F-M0 | 1.9, 1.12, 1.14 | static ok |
| `STIC2-FLIP-BIT` | mob | `51C3` (high decle of Y0, scene 2) | `0006` | `000A` | `frame 5 hash == 222a4891...` | 1.10 | static ok |
| `STIC2-BORDER-EXT-DROPPED` | border | `5252` (low decle of `$0032`, scene 4) | `0003` | `0002` | `frame 5 hash == 49b08973...` | 1.8 | static ok |
| `STIC2-DELAY-VALUE` | hdelay | `5284` (low decle of `$0030`, scene 5) | `0003` | `0004` | `frame 5 hash == ba0f9f71...` | 1.8, 1.9, 1.10 | static ok |
| `STIC2-COLLIDE-APART` | collision | `52C2` (low decle of X1, scene 7) | `002E` | `0080` | `mem 0300 == 0002 got 0000` | 1.8, 1.9, 1.12 | static ok |

Every set contains the derivation's expected first assert, and each is explained by the mutant alone: the selector mutant leaves everything idle (the counter and selector asserts before it still hold); the X0 mutant gives frame 2 the idle picture and two probes of the MOB; the flip mutant mirrors the picture and keeps the three probes of frame 5; the border mutant keeps the top strip and the four probes miss the left strip; the delay mutant shifts by one more position (the hash and the two probes at (63, 40) and (83, 48)); the collide mutant empties C0, leaves C1 reading the overlap with MOB4 (`0010`) and gives C4 the overlap with the moved MOB1 (`0002`; the scenario file with those three words changed passes on the mutant image). The guards are shown able to fail by four ad hoc runs of the script (not committed): on Release a wrong `fails=` set on `STIC2-FLIP-BIT` gives `not ok ... fails-set: the failing asserts are '1.10', expected '1.11'`; on Debug the same wrong set is a well-formed list and reports ok; on Debug a wrong `from` word gives `listing: the word at 5284 is 0003, expected from 0005`; with no fifth argument an unchanged `to` word gives `pack: the patch 5284=0003 leaves the image unchanged`.

## Disagreements between the program and the oracle

None. All six scenarios passed every assert on the first execution of the finished program, on both the Release and the dev build; the frame hashes F-M0 to F-M6 matched (the renderer of plans 08-04 to 08-06 and the program written from section (a) agree with the hand-derived literals), and the sixteen collision words T1 to T16 matched. No oracle file was edited (`git diff` of `docs/video/diag-stic-2-derivation.md` against the blind-read commit has zero deleted lines and `data/video/frames` shows no difference).

## Deviations from Plan

**1. [Interpretation] A table word is two decles, so a mutant patches one decle.** The derivation's section (g) states the edits as words (`069E` to `0000`, `0620` to `0A20`, `0003` to `0002`, `0003` to `0004`, `072E` to `0780`). A 16-bit register word above `03FF` needs two program words, whatever the layout, so each row patches the decle that carries the change: the high decle `06` to `0A` (`0A20`), the low decle `03` to `02` and `03` to `04`, and the low decle `2E` to `80` (`0780`) reach the derivation's word exactly; `STIC2-MOB-X-ZERO` patches the low decle `9E` to `00`, so the X word is `0600`, whose X coordinate (bits 7 to 0) is 0: the same disabled MOB and the idle picture the derivation intends. The inventory header says so.

**2. [Addition] The ids file header and the pair comment.** `data/expected/tests/runner-process.ids` and `data/expected/scenario-pairs.ids` gained comment blocks naming this plan (the plan asks for them); `docs/diagnostics.md` also got the images sentence of the Build-tree section extended to the three newer images (it was stale).

**3. [Information] The derivation's placeholder line stays.** The line under section (k) "(Appended by plan 08-07 after the blind-read record; empty here.)" cannot be deleted because the plan requires only additions to the frozen file; the appendix says it supersedes that line.

**4. [Information] The scene tables are 540 data words.** The plan did not fix the layout; the loop-over-table form is the one that keeps the register work at 1,634 cycles with a 27-word write; a straight sequence would be about 700 words of code and a per-word loop 1,026 cycles. The size (866 words) is recorded above for `listing-words`.

No auth gates, no package installs (T-08-SC), no `gh` call, no push, no tag, no `ci-record`; `.planning/config.json` and `.planning/state.json` were never staged (D-42). A stray second run of the container gate was started by mistake while checking the first one's result and was stopped within seconds (it only truncated `build/gate-linux.log`, which the closing container run rewrote); nothing in the tree depended on it.

## Decisions Made

- The scenario `.expect` headers share one wording for the shared-misreading caveat (the author of the derivation also wrote the rules, the program and the core; the blind reader is of the same model family) and one `Excluded on purpose` list; the per-scenario text is in the header paragraph.
- The mutant inventory pins the first clause and, where the derivation names a value, that value (`got 0000` for the selector and the collision mutants, the hash of F-M0 for the X0 mutant); the three hash mutants whose bad value the derivation does not state leave it open.
- `tools/release.sh` (line 152, the images the rehearsal needs) and `pkg-install.sh` keep their six-image lists: the new image is a test fixture of the runner, not part of the install package (D-10).

## Cost (laptop, load average 10 to 13)

| Item | Value |
|------|-------|
| gate total, sh, at plan start | 84 s, gate-cpu user 143.9 sys 119.5 |
| gate total, sh and dash, Task 1 tree | 85 s and 84 s |
| gate total, sh and dash, Task 2 tree | 83 s and 83 s |
| gate total, sh and dash, Task 3 tree | 87 s and 85 s, gate-cpu user 149.0 sys 129.7 (sh), 143.4 and 123.1 (dash) |
| container gate, Task 3 tree | 322 s (gcc 13.3.0, cmake 3.28.3, x86_64 emulated, `core-imports=5`) |
| `tests/runner-process-tap.sh` alone, six mutants | 10.9 s on a Release configuration (six full runs) against 10.4 s on a Debug configuration (static): the six ten-frame runs are inside the noise (one run is 0.01 s on the Release runner, 0.08 s on the dev runner) |

The additions are inside the noise of a loaded laptop; `.planning/CI-EFFICIENCY.md` is not edited by this plan (the checkpoint line of plan 08-08 carries the figures). The plan adds the 17 test ids budgeted (4 + 7 + 6), 0 faults, 0 controls and no consumer replay.

## Open Items

- **Own bit on a CPU write, the glossary exemption of the three delay field names, collision under delay.** Carried unchanged from the 08-05 and 08-06 summaries; none touches this plan (the scenarios never write a collision register with a nonzero value and no scenario combines a delay with a collision). The orchestrator has confirmed the exemption.
- **The frame counter relies on the zero power-on RAM.** A5 clears the BACKTAB only, so `$02F1` starts at the profile default `power_on_fill.ram` (zero, `unknown-default-chosen`); all six scenarios assert a counter of `000A`. If the owner wants the scenarios independent of that default, the spec would need one more store in the initialization (a change of A5, so a new oracle commit); recorded here and in `diag-stic-2.asm`, not changed.
- **Plan 08-08 (claims, important gates).** CL-0025 to CL-0027 name the frames, the collision table and these scenario predicates (derivation section (h)); the six ids `runner-process.mutant.STIC2-*` are test ids of the kind `<suite>.mutant.<slug>` and can be registered as important-gates mutants of `VID-05`; the pair ids and the six scenario ids too. The final ids of the pinned-default families are those of the 08-06 summary.
- **Plan 08-09 and later (release machinery).** `tools/release.sh` lists six images and does not list `diag-stic-2.ivsc`; whether the release rehearsal should also require it is a release-plan decision.
- **Information.** D10 and D11 are not performed and no claim above provisional exists; every expected value is `source-expectation` or `original-diagnostic` from the frozen oracle, tier 4 lineage, with a blind second read of the same model family (C43).

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=37/37 controls=371/371 ... fault-runs=226 ... faults=226/226 ... goldens=27 ... listings=7 listing-words=1386 manifests=11 effective-yes=8 ... tests-expected=4772 tests-executed=4768 tests-skipped=4 tests-missing=0 ... video-hash-ties=18 ... consumer-lines=135 ... pkg-selftest=17/17 (0 FAIL lines, no FAIL fault- line; gate-time total=87 s on the Task 3 tree)
dash tools/gate.sh:    the identical summary line (md5 ba595d513acf5b55fa1222232ce7c98f for both shells; 0 FAIL lines; total=85 s)
GATE_LINUX_PLATFORM= sh tools/gate-linux.sh: exit 0 on the same tree (gate: PASS; gcc 13.3.0, cmake 3.28.3, x86_64, cc=GNU, core-imports=5), the plan-end counts as above, 0 FAIL lines, gate-time total=322 s; the Release preset runs the six mutants there
```

After Task 1 (before its commit): sh and dash `tests-expected=4746 tests-executed=4742 goldens=17 manifests=11 effective-yes=8 listings=7 listing-words=1386 video-hash-ties=12` (md5 a74a4634235dc976094297497bdc9520 for both shells). After Task 2: `tests-expected=4760 tests-executed=4756 goldens=27 video-hash-ties=18` (md5 f6b9557a3d7727b7a856df029c3fa877 for both shells). The first gate run of every task passed; nothing needed a retry. On the closing tree (this SUMMARY, STATE.md and ROADMAP.md added) sh and dash gave `gate: PASS` with the identical summary line (md5 45d486180fc7ddf6ec4916a220ec6281, 0 FAIL lines, 84 s and 86 s) and the container gate was run again (`GATE_LINUX_PLATFORM= sh tools/gate-linux.sh`): exit 0, `gate: PASS` with the plan-end counts, 0 FAIL lines, gate-time total=326 s; `build/gate-linux.log` holds that run (the stray second run named above had truncated the log of the first).

## Known Stubs

None.

## Threat Flags

None new. T-08-19 (rights, third-party glyph or copied content): authored shapes only, no GROM read, manifest attestation and `redistributable: yes`, the listing generated by `ivs_asm` and regenerated by `asm.regen-ivs-diag-stic-2`, no tracked image or binary. T-08-20 (a scenario passing for the wrong reason): the idle control and the two pair lines, the `.expect` probes, six mutants each failing its scenario with a recorded set. T-08-21 (Release-only mutants prove less on the dev preset): the same ids in both presets, four ad hoc probes show the static checks fail on a wrong `from` word, an unchanged image and (in Release) a wrong set, the container gate and the Release preset run the full judgement, `tests-skipped` unchanged. T-08-SC: no installs.

## Self-Check: PASSED

Verified before the final commit: `fixtures/FX-diag-stic-2/diag-stic-2.asm`, `diag-stic-2.listing.txt`, `manifest.json`, the six scenarios, the twelve golden files and `data/expected/diag-mutations-stic2.ids` exist; commits `9aaa8e9`, `12ad7b7` and `3821cfd` are ancestors of HEAD and `git rev-list --count a9881e6739d3c3ac375823106c836b2d601a881f..HEAD` printed 3 before this file's commit; `.planning/config.json` and `.planning/state.json` are not staged; `git ls-files` lists no `.ivsc`, `.bin`, `.rom`, `.exec`, `.wav`, `.ppm` or `.vraw` file; the acceptance greps give `git diff --name-only <merge-base> -- data/golden` 12 files, `Excluded on purpose` once in each of the six `.expect` files, 6 inventory rows, 6 `runner-process.mutant.STIC2-` ids, 0 `STIC2-.*skip-ok`, `grep -c 'IVS_TEST_CONFIG\|Release' tests/runner-process-tap.sh` 3, 0 deleted lines in the derivation diff, and `git diff --quiet` of `tests/installed-consumer`, `data/expected/pkg-consumer.golden`, `tools/check/steps/pkg-install.sh` and the earlier goldens exits 0.
