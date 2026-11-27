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

Filled in below by Task 2.
