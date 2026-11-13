---
phase: 07-first-visible-original-scenario
plan: 08
subsystem: the Lamp Tone mutant inventory with the stub-image patch path and the failing-set judge, the SCN-01 registry lines and cost checkpoint C-B (W4b)
tags: [W4b, lamp-tone, mutants, fails-set, stub-patch, checkpoint-C-B, lever-required, D-10, D-12, D-13, D-25, D-34, A-03, A-14, A-17, A-20, T-07-19, T-07-20, V3, V5, V10, V20]
status: complete

requires:
  - phase: 07-first-visible-original-scenario
    provides: "plan 07-07: gate steps=36/36 controls=354/354 faults=222/222 goldens=15 tests-skipped=2 tests-missing=0, HEAD e6565b4 at the start of this plan; the four Lamp Tone scenarios, the pass and idle goldens, the pair judge; derivation section (k) with the patch word addresses"
provides:
  - "data/expected/diag-mutations-scn.ids: 12 Lamp Tone mutants and one negative row in the form of diag-mutations-stic.ids, one row per failure class of the derivation table (g), with an explicit from word, the intended clause and, on the channel-isolation rows, a fails= column"
  - "tests/runner-process-tap.sh: boot-stub-2.exec in every mutant directory, the stub-window sibling path (ivs_pack --exec) with the same from check and must-change cmp, the optional fails= judge (finding fails-set), the ` got <value>` pin and the stop-detail clause"
  - "runner-process.mutant.SCN-* (13 ids per preset) and twelve scn-mutants mutant lines for SCN-01 in the important-gates registry"
  - ".planning/CI-EFFICIENCY.md: Phase 7 checkpoint C-B with w4-cases: 25 per preset and checkpointB-07: lever-required"
affects: [plan-07-09-consumer-replay, plan-07-12-closeout, lever-plan-for-orchestrator]

actuals:
  tokens: 7509
  tasks: 3
  commits: 3
plan_head_before: e6565b457cefd974570620e3408ee86404cdbe8b
plan_head_after: c330b373700c1033d6f2e2ad4cbfff0ef483b658

key-decisions:
  - "Every row keeps the clause of the derivation table (g), written before the mutant was run; the only change of a patch from the table is SCN-HLT-IDLE (both words of the two-word idle branch) and SCN-REARM-LONG (N = 1,023, as section (k) of plan 07-06 records)"
  - "The grammar gained two optional forms and no new column: a clause that ends in ` got <value>` pins the value the assert read, and a clause that starts with `stop ` is the detail of a stop mismatch; earlier inventories use neither"
  - "The stub-window rule decides the image from the addresses (all in 1000-101F: boot-stub-2.exec through ivs_pack --exec; none: the cartridge; both: an inventory finding)"
  - "The complete failing set is judged from the report's step.K.assert.J fail lines, in report order, not from outcome.detail; it is stated only where the derivation states it"

key-files:
  created:
    - data/expected/diag-mutations-scn.ids
  modified:
    - tests/runner-process-tap.sh
    - data/expected/tests/runner-process.ids
    - data/expected/important-gates.ids
    - .planning/CI-EFFICIENCY.md
    - .planning/STATE.md
    - .planning/ROADMAP.md

requirements-completed: []

completed: 2026-11-13
---

# Phase 7 Plan 08: Lamp Tone mutants and cost checkpoint C-B Summary

**Every failure class of the Lamp Tone program and its runtime now has a case that proves its channel's asserts fail: 12 single-class mutants end with their derived first failing clause (four channel-isolation rows and the ungated-sound row also on their complete failing set), the stub-vector mutants (`JD` as `J`, the interrupt target tie) are patched into `boot-stub-2.exec` through `ivs_pack --exec`, the idle control is shown able to fail, and the timing-only re-arm change (N = 300) still passes; cost checkpoint C-B ends `checkpointB-07: lever-required`, the projection 8.45 above the 8.3 trigger.**

## Commits

| Commit | Task | What |
|--------|------|------|
| `e2c8b3b` | 1 (tracer) | stub-2 copy, `--exec` patch path and `fails=` judge in `run_mutant`; inventory header; `SCN-LAMP-COMPLEMENT` (cartridge, nine failing asserts) and `SCN-STUB-JD-AS-J` (stub, first clause `frame 3 displayed == yes`); two ids |
| `647365a` | 2 | the other ten rows and the negative row, 13 ids, twelve `scn-mutants` registry lines |
| `c330b37` | 3 | `.planning/CI-EFFICIENCY.md` checkpoint C-B |
| the commit that adds this file | | this SUMMARY, STATE.md, ROADMAP.md |

## What was built

- **Harness** (D-25, A-14, A-17): `run_mutant` copies `boot-stub-2.exec` (or the unpatched cartridge, for a stub row) into the mutant directory next to `boot-stub.exec`; a row whose addresses all lie in `1000-101F` rebuilds `boot-stub-2.exec` with `ivs_pack --exec <stub-2 listing> <out> --patch ADDR=HEX` after the same explicit `from` check against `ivs_pack --dump-words` of the stub listing and the same must-change `cmp`; a row with addresses on both sides is an `inventory` finding. `check_fails` compares the comma-joined `step.<k>.assert.<j> fail` lines of the report (report order) with the `fails=` column (finding `fails-set`); `fails=` on a `pass` or `timeout` row, a malformed list, an unknown scenario or a short row are `inventory` findings. The earlier three inventories and their judgements are unchanged (their mutant directories now also hold `boot-stub-2.exec`, which no earlier scenario names). Header comment of the script updated.
- **Inventory** (`data/expected/diag-mutations-scn.ids`): header with the grammar, the `fails=` column, the stub-window rule, the outcome grammar and a table of the 13 edits, then 13 rows. Every `from` word was read from the unpatched listing with `ivs_pack --dump-words` and is checked at run time.
- **Registry**: `runner-process.mutant.SCN-*` ids in `data/expected/tests/runner-process.ids` in emission order; twelve `scn-mutants mutant runner-process.mutant.<ID> SCN-01` lines (no line for the negative row); `gates` 74 to 75, `lines` 247 to 259.
- **Checkpoint C-B**: see below and `.planning/CI-EFFICIENCY.md`.

## Results (rows and observed outcomes)

The intended clause of every row is the one of the derivation table (g), written there before the program existed; each mutant was first run on a scratch copy of the images (nothing committed) and compared with it, then judged in the suite. Observed on dev and release (identical):

| Row | Scenario | Patch (word address, from to) | Observed first failing clause (exit 10) | Failing asserts of the report | Judged on |
|-----|----------|-------------------------------|------------------------------------------|-------------------------------|-----------|
| `SCN-INPUT-PORT` | pass | `5024` 01FE to 01FF | 1.4 `mem 02F2 == 007E got 00FF` | 1.4, 1.5 | clause |
| `SCN-PROGRESS-STORE` | pass | `5031` 02F3 to 02F8 | 1.5 `mem 02F3 == 0001 got 0000` | 1.5 | clause and set |
| `SCN-LAMP-COMPLEMENT` | pass | `507E` 0019 to 0034 | 4.32 `frame 3 hash == 63c0...` | 4.32 to 4.40 (six hashes, three pixel probes) | clause and set |
| `SCN-AMP-VALUE` | pass | `5074` 000F to 000E | 4.46 `psglog 7 addr=13 value=000F ...` (got 000E) | 4.46, 4.51, 4.52, 4.54, 4.55 | clause and set |
| `SCN-SOUND-UNGATED` | **idle** | `506B` 0204 to 0208 | 1.20 `psglog count == 6 got 19` | 1.20 | clause and set |
| `SCN-REARM-SHORT` | pass | `508F` 01F4 to 0064 | 1.3 `mem 02F1 == 0003 got 0006` | 1.3 | clause |
| `SCN-REARM-LONG` | pass | `508F` 01F4 to 03FF (N = 1,023) | 1.3 `mem 02F1 == 0003 got 0002` | 1.3 | clause |
| `SCN-STROBE-LATE` | pass | `5005` 0020 to 0022 | 4.30 `frame 3 displayed == yes got no` | 4.30 to 4.40 (two `displayed`, six hashes, three probes) | clause and set |
| `SCN-NO-EIS` | pass | `5093` 0002 to 0034 | 1.3 `mem 02F1 == 0003 got 0000` | 1.3, 1.4, 1.5 | clause |
| `SCN-HLT-IDLE` | pass | `5094,5095` 0220,0001 to 0000,0000 | `stop step=1 line=29 expected=target got=halted` | 1.1, 1.3, 1.4, 1.5 | stop clause |
| `SCN-STUB-JD-AS-J` | pass | stub `1005` 0352 to 0350 | 4.30 `frame 3 displayed == yes got no` | 4.30 to 4.40 | clause |
| `SCN-ISR-TIE` | pass | stub `1006` 0004 to 0006 | 4.30 `frame 3 displayed == yes got no` | 4.30 to 4.40 | clause |
| `SCN-REARM-NEGATIVE` | pass | `508F` 01F4 to 012C (N = 300) | none: outcome pass, exit 0 | none | outcome |

The channel isolation holds as the derivation says: the lamp complement fails the video asserts only (no guest word, write-log record or audio level fails), the amplitude value fails the gate record and the four gate-window levels only, the progress store fails the one counter assert only, and the late strobe fails exactly the `displayed` asserts, hashes and probes (the outside probe 4.41 and the log and audio asserts hold). Both stub mutants reach the same eleven failing asserts as the late strobe, and every assert before 4.30 holds, including the frame counter at four steps: the counter alone would not catch the vector error, and `frame 3 displayed == yes` does (D-10, A-17). `SCN-HLT-IDLE` ends at the halt in frame 0 (tick 26,384), so the run never reaches its first routine.

Row sets that the derivation does not state (INPUT-PORT, the re-arm rows, NO-EIS, HLT-IDLE, the stub rows) are judged on the first clause only; the observed sets above are information.

## Induced failures (the judge shown able to fail)

On the tracer rows, with the inventory edited and restored (release preset, `cmp` against the saved copy confirmed the restore):

1. wrong clause (`frame 3 hash` named `frame 5 hash` on `SCN-LAMP-COMPLEMENT`): `not ok`, finding `detail`, "outcome.detail 'assert=32 line=64 frame 3 hash == 63c0... got 7b78...', expected 'assert=* line=* frame 5 hash == ...'".
2. wrong from (`0352` named `0353` on `SCN-STUB-JD-AS-J`): `not ok`, finding `listing`, "the word at 1005 is 0352, expected from 0353".
3. wrong fails set (`4.40` dropped on `SCN-LAMP-COMPLEMENT`): `not ok`, finding `fails-set`, "the failing asserts are '4.32,...,4.40', expected '4.32,...,4.39'".
4. to equal to from on a stub row: `not ok`, `pack`, "the patch 1005=0352 leaves the image unchanged".
5. addresses in both windows on one row: `not ok`, `inventory`, "addresses 1005,507E lie in both the boot stub window 1000-101F and the program".
6. `fails=` on a `pass` row: `not ok`, `inventory`, "fails= needs the outcome guest-fail".

On the final inventory (same method): a wrong pinned value (`SCN-REARM-LONG` `got 0002` named `got 0006`): `not ok`, `detail`; a wrong from on the second word of the halt row (`0001` named `0002`): `not ok`, `listing`, "the word at 5095 is 0001, expected from 0002"; a dropped member of a fails set (`4.31` on `SCN-STROBE-LATE`): `not ok`, `fails-set`; the negative row moved out of the band (`508F=0064` under `pass 0`): `not ok` with exit-code 10 against 0, outcome `guest-fail` against `pass`. In each of these four final trials exactly one case of the suite was `not ok` and every other case stayed `ok`.

## Cost checkpoint C-B (D-34)

Full entry in `.planning/CI-EFFICIENCY.md` ("Phase 7 checkpoint C-B (plan 07-08)").

- Cases per preset: 07-07 added 12, 07-08 added 13: `w4-cases: 25 per preset (S-4 ceiling 19; +6 = second re-arm edge and five locked runtime mutant classes, D-25, A-03, A-14, A-17)`. `tests-expected` 4546 to 4570 to 4596. Controls 344 to 354 (10 of the planned 13, A-11), faults 222, this plan adds neither.
- Local suite time of `tests/runner-process-tap.sh`: dev 12.03 s before 07-07, 14.42 s after 07-07, 13.54 s at the start of this plan, 17.38 s after it (medians of three); release 6.24, 6.78, 6.83, 7.49 s. Alternated old-script/new-script pairs in one session: dev 15.33 to 17.84 s (+2.5 s), release 6.85 to 7.67 s (+0.8 s); 0.19 s and 0.06 s per new case. The machine was loaded (1-minute load 8 to 20), so the figures are comparisons, not absolutes.
- Hosted: last five successful rows 43352432394, 43359195974, 43365038335, 43369159094, 43373151931, `r` 7.68, 9.02, 7.53, 8.02, 8.17, median 8.02; none includes a Phase 7 commit, so the projection is 8.02 + 25 x 0.7 / 41 = 8.45, above 8.3.

```
checkpointB-07: lever-required
```

The rule fires on the projection. The planned fallback is lever (2) of the pre-committed order: the mutant inventories on the release preset only, created by the orchestrator as a lever plan; this plan adds no further test case and none should be added until that lever lands. The plans after this one that add cases (07-09: 2 pkg self-tests; 07-10 and 07-11: the SCN-02 scenarios and cases) are the orchestrator's to order against the lever plan.

## Deviations from Plan

**1. [Rule 1 - premise mismatch] `SCN-HLT-IDLE` patches both words of the idle branch.** The derivation row and the plan say "the idle branch replaced by `HLT`". The idle branch is the two-word instruction `B idle` (`5094: 0220`, `5095: 0001`). Patching only `5094=0000` ran the run to `stop ... got=faulted` (core fault `unsupported-sdbd-target`, tick 26,340): an `HLT` takes effect after the NEXT instruction has executed (core comment, `HLT sets the stop latch evaluated at a later boundary`), and the left-over displacement word `0001` runs as an `SDBD` prefix whose target, the next word `0001`, is again an `SDBD`. Both words are replaced by `HLT` (`5094,5095 0220,0001 0000,0000`), which is the idle branch replaced by a halt: the run stops with `stop step=1 line=29 expected=target got=halted`, the clause the derivation states. The clause was not edited to match the first run; the patch was corrected to what the row means. Recorded in the inventory header.

**2. [Interpretation] `SCN-REARM-LONG` is N = 1,023.** The 10-bit immediate limit (section (k), plan 07-06 finding): `508F=03FF` gives the first clause and value of the row (1.3 got `0002`).

**3. [Interpretation] `SCN-SOUND-UNGATED` carries `fails=1.20`.** The plan names four rows for `fails=`; the derivation table (g) states the set `1.20` for this row too ("only the idle log assert shows it"), the scenario has one step, and the set held at the first run, so the claim is machine-checked as well.

**4. [Interpretation] Two optional forms of the clause, no new column.** ` got <value>` pins the read value where the derivation states it (INPUT-PORT, PROGRESS-STORE, SOUND-UNGATED, both re-arm rows, STROBE-LATE, NO-EIS, both stub rows), which separates the two re-arm edges (6 against 2) that share one assert text; the `stop ` prefix carries the stop mismatch of `SCN-HLT-IDLE` (the existing grammar only knew `assert=` and `budget` details). The earlier inventories do not use either form.

**5. [Interpretation] Words for "a word that leaves R1 unchanged" and "a word with no effect".** `NOP` (`0034`, 6 cycles, the cycles of the `COMR` it replaces) for the lamp complement and the `EIS`; `NOPP` (`0208`, the two-word branch never taken) for the gate skip, as the derivation names it.

**6. [Scope] More induced failures than the plan asks (two required).** Ten in all, because the harness gained four new findings (`fails-set`, the stub path, the window rule, the `fails=` syntax) that each needed to be shown able to fail.

**7. [Process] Comparison timings used an old copy of the script.** The alternated pairs ran the script of `e6565b4` (extracted with `git show` into a scratch directory) and the script of this plan against the same tree; nothing was checked out or reset.

No Rule 2 or 3 fixes; no auth gates; no package installs (T-07-SC); no C or cmake file was edited, so `tools/asm/asm.c` and `tools/pack/pack.c` are unchanged and no recipe digest was refreshed; the oracle (derivation sections (a) to (j), the F-S literals) is untouched.

## Decisions Made

- Keep the clause the derivation states and fix the patch when the patch, not the clause, is what the row does not mean (the halt row).
- Put the failing-set judge beside the first-clause judge in the existing mutant runner, with no new step, job, control directory or dependency.
- Judge the complete failing set only where the derivation states one; the other observed sets stay information so no assert-number order is frozen without a derived reason.

## Open Items

- **Lever plan for the orchestrator (checkpoint C-B)**: `checkpointB-07: lever-required`. Lever (2), mutant inventories on the release preset only (29 `runner-process.mutant.*` cases per preset, 13 of them from this plan), created by the orchestrator; no further test case until it lands. The hosted rows after the push are the arbiter; the savings are not measured here.
- Plan 07-09: `tools/check/steps/pkg-install.sh` and the installed-consumer image list carry neither `boot-stub-2.exec` nor `lamp-tone.ivsc` yet (carried from 07-05 to 07-07).
- Plan 07-12: the claims CL-0023 and CL-0024 name `data/golden/demo-lamp-tone-pass.expect` and the consumer; the mutant evidence of this plan is the second leg of SCN-01.
- SCN-01 stays Pending: scenarios, goldens, shape lint, judge and now the mutants exist and pass, but the installed-consumer replay (07-09) and the claims (07-12) are not yet in.
- Carried unchanged: Phase 4 and 5 review deferrals, `sh tools/hygiene.sh --history` on earlier planning commits, the orchestrator-owned hosted CI record (no `gh` was called).

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=36/36 controls=354/354 faults=222/222 ... goldens=15 ... gates=75 lines=259 ... tests-expected=4596 tests-executed=4594 tests-skipped=2 tests-missing=0 (0 FAIL lines; gate-time total=62 s, on the tree with this file; 64 s on the tree of c330b37)
dash tools/gate.sh:    the identical summary line (md5 edada8a226ae63d0822f7a8b295be29b both, gate-time and gate-cpu removed; 0 FAIL lines; total=74 s; on the tree of c330b37 both shells gave md5 7543a76408ebce3d7f4c4e9a95e2ac58)
sh tools/gate-linux.sh: exit 0 on the tree of c330b37 (the commit before this file), build/gate-linux.log ends gate: PASS (gcc 13.3.0, cmake 3.28.3), steps=36/36 controls=354/354 faults=222/222 goldens=15 gates=75 lines=259 listings=6 manifests=8 tests-expected=4596 tests-executed=4594 tests-skipped=2 tests-missing=0, 0 FAIL lines (gate-time total=261, md5 f074581db7c5176be319e1575223c353); this plan edited shell, data and docs only, no C or cmake file, so no gcc finding was possible beyond the shell run under the container's dash
```

Counts at plan start were `tests-expected=4570`, `gates=74`, `lines=247`. `ctest --preset dev -R '^runner-process$'` passed after each task (14.2 s for the tracer). `awk '/^SCN-/ {n++}' data/expected/diag-mutations-scn.ids` prints 13; `awk '/runner-process[.]mutant[.]SCN-/ {n++}' data/expected/important-gates.ids` prints 12; the same awk anchored on `data/expected/tests/runner-process.ids` prints 13; `grep -c 'boot-stub-2.exec' tests/runner-process-tap.sh` prints 6; `grep -c '^checkpointB-07: ' .planning/CI-EFFICIENCY.md` prints 1.

## Known Stubs

None. The inventory holds derived words and clauses only; no placeholder flows to a report or a display.

## Threat Flags

None new. T-07-19: explicit `from` words checked against `--dump-words` of the right listing, the must-change `cmp`, the clauses of the derivation, `fails=` sets on the isolation rows, and ten induced failures. T-07-20: every scenario is bounded by `run-to` ticks (no row needed `timeout`), the halt row ends at the stop mismatch, patched images live in the suite's temp directory only (no `.exec` or `.ivsc` tracked; the `files` step is green). T-07-SC: no installs.

## Self-Check: PASSED

`data/expected/diag-mutations-scn.ids` exists (13 data rows); commits `e2c8b3b`, `647365a` and `c330b37` are ancestors of HEAD (`git rev-list --count e6565b457cefd974570620e3408ee86404cdbe8b..HEAD` printed 3 before this file's commit); `grep -v '^#' data/expected/diag-mutations-scn.ids | grep -c 'fails='` prints 5 (progress store, lamp complement, amplitude value, ungated sound, strobe); `.planning/CI-EFFICIENCY.md` holds one `checkpointB-07: lever-required` line and one `w4-cases: 25 per preset` line; no file contains the console brand word in a name, and `git status --short` shows only `.planning/config.json` and `.planning/state.json` (not staged) before this commit.
