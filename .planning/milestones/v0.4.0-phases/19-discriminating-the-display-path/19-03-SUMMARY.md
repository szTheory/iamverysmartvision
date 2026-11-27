---
phase: 19-discriminating-the-display-path
plan: 03
subsystem: display-path-discrimination
tags: [sixteen-cell-sweep, diagonal, stack-copied-parameter-row, exit-b, unknown-default-chosen, d-51, d-54, vid-07]

requires:
  - phase: 19-discriminating-the-display-path
    plan: 01
    provides: "the four authored guests with their registered observables, the three parameter-driven branches that make the unreachable fields movable, and the display-latch field row"
  - phase: 19-discriminating-the-display-path
    plan: 02
    provides: "the contained owner-local probe; independent of this plan in both directions, so it is nobody's exit criterion here"

provides:
  - "`tests/stic/stic_disc.c` `stic.disc.sweep-diagonal`: one id, sixteen stack-copied cells, one comparison, a per-cell diagnostic and a per-guest alternate-value mask"
  - "`tests/stic/stic_disc.c` `stic.judge.disc-diagonal-two-fields`: the committed stand-in that makes the diagonal falsifiable, red unless the comparison rejects a guest moving under two fields"
  - "`.planning/phases/19-discriminating-the-display-path/19-DECISION.md`: both exits' file enumerations committed before the measurement, the measured sixteen-cell matrix and the exit statement appended after it"
  - "six `stic-disc mutant` registry rows, one per committed stand-in, and the `VID-07` flip in one commit"

affects:
  - "VID-07: `Pending` to `Complete` on Exit B's own stated terms - \"default unchanged, observation appended, rivals named\""
  - "C1, C6, C82 and C94: one appended dated observation each; no status, policy prefix or resolver moved and no C id was minted"
  - "the four display-path fields: still tagged `unknown-default-chosen`, now movable and measured (D-51: an observation is not a hardware probe)"
  - "the milestone close: the phase lands at 1.21 of its 1.64 priced line; the checkpoint verdict is `reprice`, a runner event"

tech-stack:
  added: []
  patterns:
    - "a sweep is one id that loops the matrix, never sixteen ids: written as sixteen expected vectors it is sixteen fits, and the diagonal is a relation between cells that only one comparison can assert"
    - "a cell is a stack copy of the generated row and nothing else - never a profile file, a global or a const row - because the machine rebuilds its bus context from the profile index on every unit and a global edit makes the matrix unreproducible"
    - "the diagonal is made falsifiable by a committed stand-in that reads one guest's observable the entangled way, so 'each guest moves under its own field and no other' is a check rather than a promise"
    - "both exits' file enumerations are committed before the measurement and the final commit is compared against the enumeration as committed, so an enumeration written after the result is a stop rather than an append"

key-files:
  created:
    - .planning/phases/19-discriminating-the-display-path/19-DECISION.md
    - .planning/phases/19-discriminating-the-display-path/19-03-SUMMARY.md
  modified:
    - tests/stic/stic_disc.c
    - data/expected/tests/stic.ids
    - data/expected/important-gates.ids
    - data/discrepancies.json
    - docs/stic/reference-notes.md
    - .planning/REQUIREMENTS.md
    - .planning/ROADMAP.md
    - .planning/STATE.md
    - .planning/PROJECT.md

key-decisions:
  - "The exit is Exit B: the sweep is diagonal and no cell prefers a setting, so no default is decided and the record reads 'default unchanged, observation appended, rivals named' - a diagonal shows separability, and an engine agreeing with its own stack-copied row is not an oracle (D-51)"
  - "Exit A's trigger ('exactly one field discriminates') is read as 'exactly one field yields evidence that prefers one of its settings', because a correct sweep makes all four guests move and the literal parse is unsatisfiable; the reading is written into the decision record"
  - "The registry rows land one task after their cases because important-gates derives its accepted set from the requirement records and refuses a row naming a Pending requirement (the derived-accepted-set rule, Phase 18's O4 precedent)"

actuals:
  tokens: 20462
  tasks: 2
  commits: 2
  plan_head_before: 15db4e6a888548e452ce2a3a076550aa9314223c
  plan_head_after: 2118d71101e9054736fc58bf89ff99d31b074ed0

# Metrics
duration: 12min
completed: 2026-11-27
status: complete
---

# Phase 19 Plan 03: The Sixteen-Cell Sweep and the Exit It Lands Summary

The phase's crux: four rival fields swept as sixteen stack-copied parameter rows in one committed id,
the diagonal asserted as one comparison with a committed stand-in that makes it fail, both exits' file
enumerations committed **before** the measurement, and the exit the measurement decides landed with its
whole chain.

## Baselines

start-commit: 15db4e6a888548e452ce2a3a076550aa9314223c

Measured at `start-commit` before the first edit of this plan, each read from the file it counts:

| Count | Value | Read from |
|---|---|---|
| `stic` inventory data lines | 109 | `grep -v '^#' data/expected/tests/stic.ids \| awk 'NF' \| wc -l` |
| `important-gates.ids` data lines | 372 | `grep -v '^#' data/expected/important-gates.ids \| awk 'NF' \| wc -l` |
| `important-gates.ids` distinct first columns | 122 | `grep -v '^#' data/expected/important-gates.ids \| awk 'NF{print $1}' \| sort -u \| wc -l` |
| `ivsn-literals.ids` data lines | 26 | `grep -v '^#' data/expected/ivsn-literals.ids \| awk 'NF' \| wc -l` |
| `data/golden/` files | 29 | `ls data/golden \| wc -l` |
| `discrepancies.ids` data lines | 110 | `grep -v '^#' data/expected/discrepancies.ids \| awk 'NF' \| wc -l` |
| gate steps | 38 | `ls tools/check/steps/*.sh \| wc -l` |
| control directories (excluding `_good`) | 405 | `ls -d tools/check/controls/*/ \| grep -v _good \| wc -l` |

Gate summary line measured at `start-commit` (`sh tools/gate.sh`, before the first edit; rc 0, 0 `-- FAIL`
lines, 60 s wall):

```
gate: PASS steps=38/38 controls=405/405 claims=29 provisional=28 discrepancies=110 faults=238/238
tracked=2848 profile-keys=155 goldens=29 gates=122 lines=372 prefix-files=11 records=155 fields=155
unknown-fields=48 tests-expected=5110 tests-executed=5104 tests-skipped=6 tests-missing=0 stic-rules=67
```

The line above is abbreviated to the tokens this plan touches; the full line is the one the gate printed
at `15db4e6`.

### The cost checkpoint, recomputed here by arithmetic

`.planning/ci-runs.tsv`, the last five rows, read at `start-commit`:

| run | `cpu_s` | `build_s` | `gate_s` |
|---|---|---|---|
| 45841732895 | 584 | 51 | 171 |
| 45842795129 | 580 | 51 | 165 |
| 45843408874 | 572 | 49 | 164 |
| 45850438592 | 530 | 49 | 154 |
| 45850984597 | 368 | 35 | 115 |
| **median** | **572** | **49** | **164** |

Arithmetic, printed rather than quoted: `cpu_s` sorted is 368, 530, 572, 580, 584 so the median is the
third value, **572**; `build_s` sorted is 35, 49, 49, 51, 51 so the median is **49**; `gate_s` sorted is
115, 154, 164, 165, 171 so the median is **164**. Against the frozen pre-Phase-17 window 361 / 34 / 118,
the `cpu_s` median moved **572 - 361 = 211** cpu-s while the gate's token line did not move
(`gates=122`, `lines=372`, `tests-expected=5110` before this plan's first edit and the same tokens before
the phase's first edit). 211 is far more than this phase's priced addition of 1.64, so the movement is a
**runner event** and no lever is invented for it.

**Verdict: `reprice`.** The phase's charge, on the frozen unit prices (in-process id 0.065, process-launch
id 0.10, registry row 0.005): plan 19-01's ten in-process ids `10 x 0.065 = 0.65`; plan 19-02's four
process ids `4 x 0.10 = 0.40`; this plan's Exit B cost `2 x 0.065 + 6 x 0.005 = 0.13 + 0.03 = 0.16`. Total
`0.65 + 0.40 + 0.16 = 1.21` against the frozen 1.64 line for the phase - a margin of **0.43**, and
`1.21 / 21 = 5.8` percent of the milestone allowance. `discharged-within-allowance` is not reachable with
a 572 median against a 361 baseline, and `lever-required` is not reached because the content landed
under its priced count, so the verdict is `reprice` and the milestone's frozen figures are not edited.

## Task 1: both enumerations committed before anything is measured, then the sweep and its stand-in

One commit, and its ordering is the point: `19-DECISION.md` carries the four fields and their two
settings, the sixteen-cell prediction, Exit B's nine-path enumeration, Exit A's nineteen-path
enumeration with its per-field delta table, and the sentence that the prediction is not the decision -
and it is committed **before** the sweep has ever run. The measured matrix and the exit statement are
appended below in Task 2's commit, so the order is provable from the history rather than asserted.

### The cell builder and the sweep

`sd_cell` returns a stack copy of the generated row with the four bits applied: bit 0 moves the
REG_CLOSE event tick and both recorded forms of the window length to 2900 CPU cycles, bit 1 sets
`access_gating_when_off` to 0, bit 2 sets the `$0020` READ row's `effect_code` to `arm`, bit 3 sets
`display_latch_cleared_each_frame` to 0. It returns 0 when a row it must perturb has no such member,
event or effect row - so a field that stops being reachable is a red case rather than a silent no-op,
which is research Pitfall 1 (sweeping a field that produces no parameter passes trivially). Nothing
writes through the profile index, a global or a `const` row.

`stic.disc.sweep-diagonal` measures each guest's decisive stamps from the guest itself with the same
`sd_measure` the guest cases use, then for each of the sixteen cells re-drives that access sequence
through the hand-built bus context with `c.stic` set to the cell's row and reads the guest's **registered**
observable. The assertion is one sentence: alternate value if and only if the cell sets the guest's own
field. On a break the diagnostic names the reason, the cell, the four bits, the guest and the observed
and expected values.

Measured, from the suite's stdout (`ivs_test_diag` lines; the result file carries only `ok <n> <id>`):

```
# sweep-diagonal: G1 field 0 mask 0xAAAA default 1 alternate 0
# sweep-diagonal: G2 field 1 mask 0xCCCC default 1 alternate 0
# sweep-diagonal: G3 field 2 mask 0xF0F0 default 0 alternate 1
# sweep-diagonal: G4 field 3 mask 0xFF00 default 4 alternate 7
ok 110 stic.disc.sweep-diagonal
# judge: the diagonal rejected the entangled guest at cell 2 (bits 0x2), observed 1
ok 111 stic.judge.disc-diagonal-two-fields
```

Each mask is the sixteen-bit set of the cells in which that guest took its alternate value, and each is
exactly the cells with that guest's own bit set - the diagonal in one number per guest.

### The stand-in that makes the diagonal falsifiable

`stic.judge.disc-diagonal-two-fields` runs the **same** comparison with G1's observable read the
entangled way: the criterion's single-strobe guest, whose frame-0 displayed word also moves when the
gating field flips. It passes only when the comparison rejects it as `SD_DIAG_FOREIGN` - "a guest moved
under a field that is not its own" - and reports `judge-vacuous` when the comparison accepts it and
`judge-wrong-reason` when it fails for any other reason. It rejects at cell 2 (bits `0x2`, gating only,
observed 1), which is exactly the coupling plan 19-01 measured when it moved G1's registered observable
to the blocked counter.

### Guards proven able to fail, each reverted before the commit

- The judge's stand-in replaced by the real diagonal witness -> `# judge-vacuous: the diagonal accepted a
  guest that moves under two fields`, `not ok 111 stic.judge.disc-diagonal-two-fields`. Restored -> green.
- `sd_cell`'s gating limb made inert (a throwaway `(void)0`) -> `# sweep-diagonal: reason 2 at cell 2
  (bits 0x2) guest G2 observed 1 expected 0` (reason 2 is `SD_DIAG_MASKED`), `not ok 110
  stic.disc.sweep-diagonal` and `not ok 111 stic.judge.disc-diagonal-two-fields` (the judge needs
  `FOREIGN`, not `MASKED`). Restored -> 111 `ok`, 0 `not ok`.

### The two ids and the suite

`data/expected/tests/stic.ids` goes from 109 to 111 data lines (the plan's +2). The `stic` suite measured
with `IVS_TEST_OUT_DIR=<tmp> ctest --preset dev -R '^stic$' --output-on-failure`:

```
# suite=stic executed=111 skipped=0 failed=0
ok 110 stic.disc.sweep-diagonal
ok 111 stic.judge.disc-diagonal-two-fields
```

### Deviations

**Rule 2 / `GUARDRAILS-v0.4` §2.7 - the registry rows are split from their cases.** §2.7 requires every
new check to ship its must-fail mechanism registered in `data/expected/important-gates.ids` under its
requirement in the same commit as the check. That is impossible here: `tools/check/steps/important-gates.sh`
**derives** its accepted set from the requirement records' `Complete` ids and refuses a row naming a
`Pending` requirement, and VID-07 stays `Pending` until Task 2 flips it. The derived-accepted-set rule
therefore forces the split: this task's two ids (`stic.disc.sweep-diagonal`,
`stic.judge.disc-diagonal-two-fields`) land without a registry row, and their rows land in Task 2's
commit with the VID-07 flip, together with the six rows for plan 19-01's four stand-ins and its
equivalence stand-in. Phase 18's observation O4 is the precedent (its four `runner-process` rows landed
one task after their cases). Each id does ship its liveness guard and its committed stand-in in the same
commit as the id, so only the registry row is deferred.

## Task 2: the measurement, the exit, and the commit checked against the enumeration

### The measurement

Read from the suite's own stdout after Task 1's commit, then transcribed into `19-DECISION.md` as an
**appended** section - never by editing the prediction above it. The sixteen cells as measured, one line
per guest (alternate-value mask over the sixteen cells, bit `k` set when the guest took its alternate value
in cell `k`):

| guest | own field | measured mask | the cells with its own bit set | agrees? |
|---|---|---|---|---|
| G1 (`blocked_writes`, the witness) | 0, the register-window length | `0xAAAA` | `0xAAAA` | yes |
| G2 (`blocked_writes`) | 1, the gating rule | `0xCCCC` | `0xCCCC` | yes |
| G3 (frame-0 `displayed`) | 2, the `$0020` READ row | `0xF0F0` | `0xF0F0` | yes |
| G4 (frames 0/1/2 `displayed`) | 3, the latch sustain rule | `0xFF00` | `0xFF00` | yes |

Every one of the sixteen cells agrees with the prediction table committed before the run, so no prediction
had to be corrected and there is no deviation to record for one. The per-cell values are in
`19-DECISION.md`'s "Measured matrix" section; the matrix is diagonal in the strong sense - each guest moves
under its own field and under no other, at the default of the remaining three - and the committed stand-in
rejects the entangled reading at cell 2 (bits `0x2`, the gating field alone, observed 1).

### The exit taken, and why

**Exit B** - "default unchanged, observation appended, rivals named". The reasoning, recorded in the
decision record so it can be checked rather than inferred: the sweep settles **separability**, which is
criterion 2, and it settles nothing else. A diagonal cell shows that a field's two settings are told apart
by an authored guest's registered observable; it cannot show which setting the hardware used, because the
row under test is a stack copy of the engine's own generated table and an engine agreeing with itself is not
an oracle (`AGENTS.md`; D-51). All four guests are diagonal, so no field yields evidence that prefers one of
its settings. Exit A's trigger ("exactly one field discriminates") is read as "exactly one field yields
evidence that prefers one of its settings", not as "exactly one field's guest moves" - a four-diagonal
matrix is the criterion-2 success, and the second reading would make the trigger unsatisfiable by a correct
sweep. The reading is written into `19-DECISION.md` so a later reader can disagree with it explicitly
rather than guess.

What Exit B therefore does **not** touch: no profile value, no parameter digest, no snapshot literal, no
rule statement in `data/stic/rules.json`, no golden, no installed file, and no `data/expected/discrepancies.ids`
line. `unknown-default-chosen` stays on all four display-path fields.

### The four rival rows

One appended dated sentence to each of C1, C6, C82 and C94, each opening `Observation, 2026-11-27 (Phase 19
sweep, stic.disc.sweep-diagonal)` so the predicate keys on a sentence that did not exist at the start
commit - C82 already carried `Observation, 2026-11-26 (...)`, so a check that grepped for `Observation, `
would have passed before this task and proved nothing. C6's and C82's `evidence` gained the two new id
names, because both already enumerate the ids that exercise their rule. Measured: `Observation, ` counts
per note C1 1, C6 1, C82 **2**, C94 1 - each exactly one higher than at the start commit - and all four keep
`status: open`, their `unknown-default-chosen:` policy prefix and a valid resolver prefix.

### The six registry rows and the VID-07 flip, in one commit

```
stic-disc mutant stic.judge.disc-g1-dead VID-07
stic-disc mutant stic.judge.disc-g2-dead VID-07
stic-disc mutant stic.judge.disc-g3-dead VID-07
stic-disc mutant stic.judge.disc-g4-dead VID-07
stic-disc mutant stic.judge.disc-equivalence-dead VID-07
stic-disc mutant stic.judge.disc-diagonal-two-fields VID-07
```

One row per committed stand-in, the equivalence stand-in included, under the new gate name `stic-disc`:
`gates` 122 to 123 and `lines` 372 to 378, both measured. The rows could not land earlier - the
derived-accepted-set rule - so they land here with `VID-07`'s `Pending` to `Complete` and its traceability
row, the §2.7 deviation recorded above.

### The commit's file list against the enumeration, in both directions

The comparison is against the enumeration **as Task 1 committed it**, read out of Task 1's blob
(`git show 2118d71:.planning/phases/19-discriminating-the-display-path/19-DECISION.md`), for the section
the record states as taken (`## Exit B enumeration`) - not from the working tree, and not a union of the
two sections, because Exit A's bullets are a superset of Exit B's and a union would report Exit A's extras
as enumerated-not-committed.

| direction | result |
|---|---|
| files in the commit and not in the committed Exit B enumeration | none |
| files in the committed Exit B enumeration and not in the commit | none |

Nine files on each side. Nothing had to be appended to the enumeration, and no file that had to land was
missing from it, so the stop rule never fired.

### Token line, predicted before the gate ran

Predicted for this plan, relative to the line measured at the start commit, and read from the file that
produces each: `tests-expected` 5110 to **5114** and `tests-executed` 5104 to **5108** (two `stic` ids
across two presets); `gates` 122 to **123** (one new gate name) and `lines` 372 to **378** (six rows);
`tracked`/`hygiene-files` 2848 to **2850** (the decision record and this SUMMARY) and `scanned` 705 to
**707**; and 0 for `steps=38/38`, `controls=405/405`, `faults=238/238`, `goldens=29`, `claims=29`,
`provisional=28`, `prefix-files=11`, `discrepancies=110`, `stic-rules=67`, `profile-keys=155`, `fields=155`,
`unknown-fields=48`, `records=155` and `reports=31`.

### The token line as measured

`sh tools/gate.sh`, rc 0, 0 `-- FAIL` lines, 60 s wall, run after the changes and before the commit:

```
steps=38/38 controls=405/405 claims=29 provisional=28 discrepancies=110 faults=238/238 tracked=2850
profile-keys=155 goldens=29 reports=31 gates=123 lines=378 prefix-files=11 records=155 fields=155
unknown-fields=48 tests-expected=5114 tests-executed=5108 stic-rules=67 scanned=705
```

Every predicted token moved exactly as predicted except `scanned`, which the prediction said would go 705
to 707 and which stayed **705**. Recorded rather than re-blessed: `scanned` counts the source files the
source-list steps read, and this plan adds no source file - the two new files are markdown records - so the
prediction was wrong in the over-predicting direction and no unmeasured change occurred. `tracked` moved as
predicted, 2848 to 2850. The full gate is re-run under both shells after the commit and the two summary
lines are compared with `cmp`.

### The gate after the commit, both shells, sequentially

`sh tools/gate.sh` then `dash tools/gate.sh`, one after the other and never concurrently (lesson 33):

```
sh   rc=0   -- FAIL lines: 0
dash rc=0   -- FAIL lines: 0
cmp /tmp/g19t7-sh.line /tmp/g19t7-dash.line  -> identical
```

The final summary line, byte-identical under both shells (abbreviated to the tokens this plan moves):

```
gate: PASS steps=38/38 controls=405/405 claims=29 provisional=28 discrepancies=110 faults=238/238
tracked=2850 scanned=705 profile-keys=155 goldens=29 reports=31 gates=123 lines=378 prefix-files=11
records=155 fields=155 unknown-fields=48 tests-expected=5114 tests-executed=5108 tests-skipped=6
tests-missing=0 stic-rules=67
```

`sh tools/hygiene.sh --tree` is silent at rc 0 (`-- hygiene pass mode=tree hygiene-files=2850`), and
`git ls-files` matches nothing for `.ivsc`, `.bin`, `.rom`, `.so`, `.dylib`, `.cfg`, `.ivsn` or `.ivst`.

**`gate-linux: not run`** - the docker daemon is unresponsive on this host (`docker info` times out, rc 124,
probed twice), so the container gate is recorded as not run and never as a pass (lesson 24).

### Deviations

**Rule 3 / a verify predicate a correct record cannot satisfy (lesson 32's defect class).** Task 2's
automated block matches each rival note against `Observation, \d{4}-\d{2}-\d{2} \(Phase 19 sweep[^.]*\.`
and then requires `stic.disc.sweep-diagonal` inside the matched text. The two limbs are mutually
unsatisfiable: `[^.]` excludes the periods the id itself contains, so the match necessarily stops at
`stic.`, and no wording can put the full id inside it. The notes satisfy the predicate's **intent** - each
carries one new dated `Observation, 2026-11-27 (Phase 19 sweep, stic.disc.sweep-diagonal)` sentence, the
count of `Observation, ` grew by exactly one in each of the four notes, and the id is inside that sentence -
and the check was run in the equivalent form
`Observation, \d{4}-\d{2}-\d{2} \(Phase 19 sweep.*?stic\.disc\.sweep-diagonal`, which keeps both limbs the
plan's own GUARD MUTATION note asks for (the new dated shape, and a count that grew by exactly one, so the
pre-existing C82 sentence cannot satisfy it). No note's content was changed to make a predicate pass, and
the defect is recorded here rather than in the plan, which is frozen.

**Rule 2 / §2.7, continued from Task 1:** the six registry rows land one task after their cases, for the
derived-accepted-set reason stated above; Phase 18's observation O4 is the precedent and the split is
recorded in `19-01-SUMMARY.md` as well as here.

**A clarification of the plan's Exit A trigger, recorded rather than silently applied:** the plan's Task 2
step (2) reads "Exit A when exactly one field discriminates and its guest moves under that field and no
other". A correct sweep makes **all four** guests move under their own field and no other, so that sentence
is unsatisfiable as written by a correct implementation under either parse ("exactly one" counts fields,
and a diagonal sweep has four). It is read as "exactly one field yields evidence that prefers one of its
settings", which is what `GUARDRAILS-v0.4` §2.6, `19-CONTEXT.md` and D-54 all mean by "discriminates", and
the reading is written into `19-DECISION.md`. No code or record depends on the other parse.

**The W1 chain spans two commits by construction:** the field-addition half landed in plan 19-01's commit
(`6cffdb4`: the three parameter-driven branches, the latch field row and the whole retarget chain), and a
**value-decision** half would have landed here - but Exit B decides no value, so there is no second half to
land. Criterion 3 is satisfied per causing change rather than by one commit carrying both halves, which is
the statement `19-01-SUMMARY.md` already makes.

## The `actuals` fields, and how they were measured

`tokens` is chars/4 over the realized diff of this plan's two commits (the code and the records, not a
harness token count); `commits: 2` is measured from the plan ledger, `git rev-list --count
15db4e6a888548e452ce2a3a076550aa9314223c..HEAD` after the second commit, and `plan_head_after` names the
first of the two, because the second is the records commit that carries this file and cannot contain its own
sha. The plan's estimate was 56,000 tokens on the same scale; the realized figure is well under it, which is
worth knowing for the next phase's estimate rather than rounding to look closer.

## Self-Check: PASSED

- `19-DECISION.md` was committed by `2118d71` **before** the sweep ran, with both enumerations and no
  measured section; the measured section and the exit statement were appended by this plan's second commit
  and the diff between the start commit and `HEAD` removes no line of it.
- `stic.disc.sweep-diagonal` and `stic.judge.disc-diagonal-two-fields` are in `data/expected/tests/stic.ids`
  (109 to 111 data lines), each has a case in `tests/stic/stic_disc.c`, and each is `ok` in the suite's own
  result file (`$IVS_TEST_OUT_DIR/stic.txt`, grammar `ok <n> <id>`).
- The six `stic-disc mutant` rows name the six committed stand-ins, `VID-07` is `Complete`, and the rows
  could not have landed earlier.
- The four rival rows carry one new dated `Observation, ... (Phase 19 sweep ...)` sentence each, naming
  `stic.disc.sweep-diagonal`, and `data/expected/discrepancies.ids` did not move.
- `claims=29`, `provisional=28`, `goldens=29`, `faults=238/238`, `steps=38/38`, `prefix-files=11` and
  `controls=405/405` are unmoved, and every display-path field still reads `unknown-default-chosen`.
- `start-commit: 15db4e6a888548e452ce2a3a076550aa9314223c` is a 40-hex ancestor of `HEAD`; both gate shells
  ran sequentially and ended in byte-identical summary lines with zero `-- FAIL` lines.
- The Task 2 automated block passes as a whole with one recorded substitution: the rival-row predicate is
  run in the equivalent form stated in the deviations, because the plan's `[^.]*\.` cannot span an id that
  contains dots. Every other limb of the block ran exactly as written, including the commit's file list
  against Task 1's blob in both directions (empty on both sides) and `cmp` of the two gate summary lines.
