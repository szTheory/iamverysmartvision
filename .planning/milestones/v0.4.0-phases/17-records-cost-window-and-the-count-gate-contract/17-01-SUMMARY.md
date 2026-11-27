---
phase: 17-records-cost-window-and-the-count-gate-contract
plan: 01
subsystem: records-cost-window
tags: [recomputed-window, ci-runs, checkpoint-verdict, cost-allowance, release-records-rows]

requires:
  - phase: 16-owner-route-claim-and-release-posture
    provides: "the v0.3.0 close tree (`298be25`) and the seven release-records rows landed at the milestone opening"
  - phase: 15-build-only-host-and-fake-frontend-harness
    provides: "the frozen unit prices (process id 0.10, control 0.31, in-step selftest 0.05, registry row 0.005) this plan charges against"
  - phase: 09-registered-evidence-and-gate-cost
    provides: "the `ci-log` step, which already names the file it counts (`ci-runs=<rows>`) — the count this record must agree with"

provides:
  - "the milestone's opening cost window, recomputed by arithmetic from `.planning/ci-runs.tsv` at `21d023c`: `cpu_s` 572, `build_s` 49, `gate_s` 164, against the previous 367 / 34 / 115"
  - "the seven release-records run ids verified present exactly once (118 data rows over 119 lines), never re-appended"
  - "the two figures the roadmap's prose rounds, replaced by the file's own values: the seven rows' `build_s` spans 34 to 51, and `r` over the last five rows spans 10.5143 to 11.6735"
  - "the phase's priced addition (0.11 of the 21 allowance) charged before the single `checkpoint: reprice` line"
  - "the gate green under `sh`, `dash` and the Ubuntu container at the same token line"

affects:
  - "17-02 and 17-03: their `## Baselines` sections quote this window and the `21` allowance, never a rounded restatement"
  - "Phase 18 (2.96), Phase 19 (1.64) and Phase 20 (0.02): every later pricing sentence in v0.4.0 rests on these three medians"
  - "the v0.4.0 close: the `reprice` verdict is the milestone's opening checkpoint, so the close compares against it rather than re-deriving a first window"

actuals:
  tokens: 6285
  tasks: 2
  commits: 1
  plan_head_before: 21d023cf312430ebcba75e512c7ee3bc7a202a2f
  plan_head_after: 99df1e3b0e0c8ea58eb3b2e1e1786e7a1b6825d8

tech-stack:
  added: []
  patterns:
    - "a window recomputed by one printed recipe at a named commit, so a reader re-derives every figure from the named file instead of believing the record"
    - "a ledger row verified by whole-file comparison (seven counts against seven lines of `1`), so a missing or doubled row stops the plan instead of being repaired"
    - "a rounded prose figure replaced by the file's own value, named as the roadmap's rounding rather than restated as measured"
    - "one checkpoint line drawn from a fixed three-token vocabulary, written after the priced addition is charged, never before"

key-files:
  created:
    - .planning/phases/17-records-cost-window-and-the-count-gate-contract/17-01-SUMMARY.md
  modified: []

key-decisions:
  - "Every figure is printed by the plan's own recipe at `21d023c` and nothing is quoted: the window, the previous window, the row totals, the seven counts and the two ranges all agree with the prediction, and no figure had to be corrected."
  - "The seven release-records rows were verified, never re-appended: each of the seven ids appears in exactly one data row, so the ledger was read and left byte-identical."
  - "The roadmap's two rounded prose figures are named and replaced rather than repeated: the seven rows' `build_s` spans 34 to 51 (the roadmap says 47 to 51) and `r` over the last five rows spans 10.5143 to 11.6735 (the roadmap says 10.6 to 11.7)."
  - "`r` is printed as arithmetic only and is not a decision input (lesson 29): its denominator moves with the runner, so a ratio that holds while `cpu_s` doubles is not a signal."
  - "The verdict is `reprice`, not `discharged-within-allowance`: the median `cpu_s` moved 367 to 572 (+205, 56 percent of the previous median) against a priced addition of 0.11, while the gate's token line did not move, so the cause is a runner event and no lever is invented."
  - "The phase's priced line is 0.11: one process id at 0.10 plus two registry rows at 0.005 each, against the 0.10 line the roadmap states — 10 percent over, inside the 25 percent repricing threshold, and recorded rather than rounded to 0.10."
  - "The measured local cost of the id shape is recorded beside the unit price (0.37 local cpu-s, about 0.62 hosted at the 1.675 factor), because section 2.4 asks for the measured addition and a unit price is not a measurement."

requirements-completed: []

coverage:
  - id: D1
    description: "The opening cost window recomputed by arithmetic from `.planning/ci-runs.tsv` at the plan's start commit, printed before anything is priced, with the recipe beside the numbers"
    requirement: "EV-08"
    verification:
      - kind: other
        ref: "awk -F'\\t' 'NR>1 { print $13 }' .planning/ci-runs.tsv | tail -5 | sort -n | sed -n 3p -> 572 (and 49 for column 8, 164 for column 10), compared as whole lines against the record"
        status: pass
    human_judgment: false
  - id: D2
    description: "The seven release-records rows verified present exactly once and the file's own totals read from the file (118 data rows over 119 lines)"
    requirement: "EV-08"
    verification:
      - kind: other
        ref: "seven per-id row counts compared as one whole file against seven lines of `1`; `sh tools/check/steps/ci-log.sh` -> '-- ci-log pass ci-runs=118'"
        status: pass
    human_judgment: false
  - id: D3
    description: "The two roadmap figures that do not survive recomputation replaced by the file's own values, with `r` printed as arithmetic only"
    requirement: "EV-08"
    verification:
      - kind: other
        ref: "release-build_s-range: 34 to 51 (sorted column 8 over the seven rows); r-range: 10.5143 to 11.6735 (column 13 / column 8 over the last five rows)"
        status: pass
    human_judgment: false
  - id: D4
    description: "The priced addition charged against the 21 allowance and exactly one checkpoint line from the section 2.4 vocabulary"
    requirement: "EV-08"
    verification:
      - kind: other
        ref: "grep -c '^checkpoint: ' -> 1, and the line reads 'checkpoint: reprice'"
        status: pass
    human_judgment: false
  - id: D5
    description: "The gate green under both shells and the Ubuntu container at the plan's predicted token line, with this plan moving only the record it commits"
    requirement: "EV-08"
    verification:
      - kind: other
        ref: "sh tools/gate.sh and dash tools/gate.sh -> identical 'gate: PASS' summary lines; sh tools/gate-linux.sh -> 'gate-linux: exit 0'"
        status: pass
    human_judgment: false

# Metrics
duration: 19min
completed: 2026-11-26
status: complete
---

# Phase 17 Plan 01: The Recomputed Window and the Count-Gate Contract Summary

**The milestone's opening cost window recomputed by arithmetic at `21d023c` — `cpu_s` 572, `build_s` 49, `gate_s` 164 against the previous 367 / 34 / 115 — with the seven release-records rows verified present once each rather than re-appended, the phase's 0.11 addition charged against the 21 allowance, and one `reprice` line closing the block because the median moved by a runner event while the token line did not.**

## The recomputed window

Every figure below was measured at `start-commit: 21d023cf312430ebcba75e512c7ee3bc7a202a2f` by the one recipe
printed here, over the one named file `.planning/ci-runs.tsv`. Nothing is quoted from a record, from the
roadmap or from this phase's research: the prediction in the plan was `572 / 49 / 164` and the file produced
`572 / 49 / 164`, so no figure needed correcting and none was corrected.

### The recipe

One column at a time — column 13 is `cpu_s`, column 8 is `build_s`, column 10 is `gate_s` — over the data rows
only (`NR>1`), last five, sorted numerically, third value:

```sh
awk -F'\t' 'NR>1 { print $13 }' .planning/ci-runs.tsv | tail -5 | sort -n | sed -n 3p
awk -F'\t' 'NR>1 { print $8  }' .planning/ci-runs.tsv | tail -5 | sort -n | sed -n 3p
awk -F'\t' 'NR>1 { print $10 }' .planning/ci-runs.tsv | tail -5 | sort -n | sed -n 3p
```

The statistic is a per-column **median of the last five data rows**: `sort -n`, then the third value. The
previous window is the same recipe over the five data rows that immediately precede the first of the seven
release-records rows (data-row index 110, so rows 105 to 109). The row totals are read from the same named
file: the data rows by `awk 'END { print NR + 0 }'` over the `NR>1` slice, the lines including the header by
`awk 'END { print NR + 0 }'` over the whole file.

### The numbers

```
start-commit: 21d023cf312430ebcba75e512c7ee3bc7a202a2f
rows: 118 lines: 119
release-rows: 45835631107=1 45836207855=1 45839153455=1 45839646456=1 45841732895=1 45842795129=1 45843408874=1
window: cpu_s=572 build_s=49 gate_s=164
previous-window: cpu_s=367 build_s=34 gate_s=115
release-build_s-range: 34 to 51
r-range: 10.5143 to 11.6735
```

The last five data rows, as `run_id cpu_s/build_s/gate_s`: `45841732895` 584/51/171, `45842795129` 580/51/165,
`45843408874` 572/49/164, `45850438592` 530/49/154, `45850984597` 368/35/115. The five rows the previous
window is taken from are `45810148114` 366/34/119, `45823169481` 362/33/111, `45823563495` 371/35/115,
`45832244091` 583/52/169, `45833228626` 367/34/115 — a window that already contained one slow run, which is
why its median is 367 and not the 583 outlier.

### The seven rows, verified rather than re-appended

For each of the seven release-records run ids the count of data rows whose second field is that id was compared
as a whole file against seven lines of `1`; every count reads `1`, printed on the `release-rows:` line above.
The file's own totals were read from the file: `rows: 118 lines: 119`. A count of 0 or 2 would have stopped the
task and gone to the operator — a missing row is an operator obligation and the executor never repairs an
append-only ledger — so nothing was appended, inserted or substituted, and `.planning/ci-runs.tsv` is
byte-identical to what the plan started from. The independent count agrees: the `ci-log` step, which names the
same file, prints `-- ci-log pass ci-runs=118`, the same 118 the record prints.

### The two figures the roadmap's prose rounds

Two numbers in the roadmap's Phase 17 prose do not survive recomputation, and this record prints the file's
own values instead:

- The roadmap says the seven rows are slow-runner rows with `build_s` "47 to 51". The file says otherwise: the
  seven rows carry `build_s` 51, 42, 34, 47, 51, 51 and 49, so the range is `release-build_s-range: 34 to 51`.
  The block is slower than the rows around it on its own range, but its lowest member is 34, not 47 — the 34
  is run `45839153455`, which is exactly as fast as a quiet window.
- The roadmap says `r` is flat inside "10.6 to 11.7". Recomputing `r = cpu_s / build_s` over the last five
  rows gives 11.4510, 11.3725, 11.6735, 10.8163 and 10.5143, so the range is `r-range: 10.5143 to 11.6735`. The
  low end is a rounding, not a different finding.

`r` is printed here as arithmetic only and is **not** a decision input (lesson 29): its denominator is
`build_s`, which moves with the runner and with what the build did, so a ratio that stays inside a band while
`cpu_s` moves by 205 cpu-s is measuring two things at once and cannot discriminate between them. It appears in
this record because a reader recomputing the window will compute it, and a reader who finds it unmentioned will
assume it was overlooked.

### The reading

The median `cpu_s` moved from 367 to 572 — 205 cpu-s, 56 percent of the previous median — while the gate's
token line under the same work did not move at all. The seven release-records rows are the slow-runner block:
they are the runs that carry a release's build and the full suite, and the two rows after them fall back to
530 and 368 as soon as the extra work stops. A median that moves by half its own value between two windows with
an identical token line, in a block whose members individually span 365 to 588 cpu-s, is a runner event rather
than a change in what the project costs to verify. That is the reading the section 2.4 rule prescribes, and
nothing is invented to explain it.

## Cost

The phase's priced line is the sum of the frozen unit prices, itemised so a reader re-derives it rather than
trusting it:

| item | unit price (hosted cpu-s) | count | subtotal |
|---|---|---|---|
| process id `runner-process.judge.flatwrap-shim` (plan 17-02) | 0.10 | 1 | 0.10 |
| registry row `cart-role mutant runner-process.judge.flatwrap-shim EV-08` (plan 17-02) | 0.005 | 1 | 0.005 |
| registry row `count-gate mutant extra-file EV-08` (plan 17-03) | 0.005 | 1 | 0.005 |
| **priced addition** | | | **0.11** |

No third row and no new selftest case is in that line: this phase adds exactly one process id and exactly two
registry rows, and both rows resolve to work that already exists, so no in-step selftest price applies. Charged
against the 21 allowance the milestone recomputes from `.planning/ci-runs.tsv`, the phase prices **0.11 hosted
cpu-s**. The roadmap states a 0.10 line for this phase, so the addition is 10 percent over it — inside the 25
percent repricing threshold, and the verdict below is `reprice` in any case, so no repricing step is owed.

A unit price is not a measurement, so the measured local cost of the id shape is recorded beside it: 0.33 local
cpu-s on the dev preset plus 0.04 on the release preset, 0.37 local cpu-s in total, about 0.62 at the 1.675
local-to-hosted factor. That figure was measured in this phase's research session against the container shape
the id will launch; the id itself lands in plan 17-02, so this plan cannot re-measure it and says so rather
than presenting a unit price as a measurement. The measured 0.37 is 1.8 percent of the 21 allowance and 3.4
times the priced addition, which is exactly the gap section 2.4 exists to keep visible.

The verdict: the median `cpu_s` moved 367 to 572, 205 cpu-s, while the gate's token line did not move. The
priced addition is 0.11. A median that moved far more than the priced addition, with an unchanged token line
under the work, is a runner event — so no lever is invented to explain it, and the one line that closes this
window is

checkpoint: reprice

## Gate

Both shells were run at this record's own commit, on the plan's tree with the record tracked. `sh` and `dash`
produced the **same** summary line byte for byte, and neither log carries a single `-- FAIL ` line:

```
gate: PASS steps=38/38 controls=404/404 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=238 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=118 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=29 provisional=28 handoffs=8 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=695 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=238/238 tracked=2807 binaries-listed=0 rights-pinned=4/4 terms=9 doc-files=63 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2807 history-commits=1 gates=119 lines=364 input-rows=31 input-policies=11 input-profile-fields=31 files=2807 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=20/20 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=12/12 runner-files=18 cases=14 sources=13 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=5042 tests-executed=5038 tests-skipped=4 tests-missing=0 docs=64 claim-refs=27 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

That line is the whole token comparison, split into one token per line and matched as whole tokens, not
searched as a substring of an unsplit line: every token the plan predicted before execution is present and
unchanged — `steps=38/38`, `controls=404/404`, `faults=238/238`, `goldens=29`, `gates=119`, `lines=364`,
`tests-expected=5042`, `tests-executed=5038`, `tests-skipped=4`, `prefix-files=11`, `claims=29`,
`provisional=28`, `ci-runs=118`, `binaries-listed=0`. Nothing moved and nothing was re-blessed. The three
`git ls-files` counts on that line — `files=2807 tracked=2807 hygiene-files=2807` — are the tracked list at
this commit, read from `git ls-files | wc -l` rather than predicted.

The Ubuntu container gate ran **once** (`sh tools/gate-linux.sh`), with the prior `build/gate-linux.log`
copied aside before it started (lesson 18) and no other container gate running: it exited 0 and printed
`gate-linux: exit 0 (log build/gate-linux.log)`, its log ending in a `gate: PASS` line under `cc=GNU`. The
container line differs from the host line in exactly the tokens that are host-dependent and were different
before this plan started — `cc=GNU` rather than `cc=AppleClang`, `core-imports=5` rather than 7,
`archive=sha256` rather than `archive=members`, `refs-verified=0/4` rather than `3/4` — and in nothing else.
The container line, verbatim:

```
gate: PASS steps=38/38 controls=404/404 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=GNU c-presets=2 fault-runs=238 core-imports=5 purity-selftest=7/7 ctest-runs=2 ci-runs=118 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=29 provisional=28 handoffs=8 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=695 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=238/238 tracked=2806 binaries-listed=0 rights-pinned=4/4 terms=9 doc-files=63 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2806 history-commits=1 gates=119 lines=364 input-rows=31 input-policies=11 input-profile-fields=31 files=2806 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=sha256 public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=20/20 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=12/12 runner-files=18 cases=14 sources=13 refs-verified=0/4 state-rows=88 state-cuts=18 tests-expected=5042 tests-executed=5038 tests-skipped=4 tests-missing=0 docs=64 claim-refs=27 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

gate-linux: PASS (exit 0)

The container gate ran before this record was tracked, so its three `git ls-files` counts read 2806 rather than
2807; the +1 is this file and applies to it identically.

Both host gates were also run once **before** this record was tracked, and that earlier pair read
`files=2806 tracked=2806 hygiene-files=2806` with every other token identical to the line above. That pair of
runs is the plan's own delta measurement, and it is why the delta is measured rather than asserted: the move
between the two pairs is exactly **+1** in each of the three `git ls-files` counts, and it is this file. The
plan predicted exactly that and no other token moving, and that is what happened — `docs=64`, `scanned=695`,
`records=154`, `steps=38/38`, `controls=404/404`, `faults=238/238`, `goldens=29`, `gates=119`, `lines=364`,
`tests-expected=5042`, `prefix-files=11`, `claims=29`, `provisional=28` and `ci-runs=118` are identical in both
pairs.

## Performance

- **Duration:** 19 min
- **Started:** 2026-11-26T20:00:55Z
- **Completed:** 2026-11-26T20:19:41Z
- **Tasks:** 2 of 2
- **Files created:** 1 (this record). **Files modified:** 0. **Files deleted:** 0.

The container gate is the bulk of the wall time (`gate-time total=249`, measured by the step itself). The host
gate was run six times in three pairs — one pair before the record existed, one pair at the record's own
commit, and one pair inside the task-2 verify block — and the measurements are the rest. No build was re-run
and no preset was rebuilt.

## Task Commits

This plan's two tasks deliver one file, so they land in one record commit rather than two — the plan itself
prescribes that shape (task 1 writes the window block, task 2 appends the cost and gate blocks and then takes
the plan's own commit of the record file and nothing else).

1. **Tasks 1 and 2: the recomputed window, the charged priced addition and the gate evidence** - `99df1e3`
   (docs) — one file created, 326 insertions, 0 deletions

**Plan metadata:** the commit that follows this one (docs) — `STATE.md`, `ROADMAP.md`, and the correction of
this record's gate block from the pre-commit measurement to the at-commit measurement.

_Measured over `21d023c..99df1e3`: `commits: 1`, and the realized diff is this file._

## Files Created/Modified

- `.planning/phases/17-records-cost-window-and-the-count-gate-contract/17-01-SUMMARY.md` — the
  `## The recomputed window` block, the `## Cost` block with its single verdict line, and the `## Gate`
  block. This is the only file this plan creates, and it is the only file it changes.

`.planning/ci-runs.tsv` was **read and left byte-identical**: the seven release-records rows were verified in
place, never re-appended. No source file, no registry, no test, no document, no control and no inventory was
edited — those are plan 17-02 and plan 17-03.

## Decisions Made

- **The window is measured, not quoted.** Every figure is printed by the recipe in this record, at
  `21d023c`, and the plan's prediction matched the file exactly (`572 / 49 / 164`, `118` rows over `119`
  lines, `34 to 51`, `10.5143 to 11.6735`). No figure needed correcting, so none was.
- **The seven rows were verified rather than re-appended.** Each of the seven run ids appears in exactly one
  data row, compared as a whole file against seven lines of `1`. A missing row would have been an operator
  stop, because repairing an append-only ledger destroys the only evidence that the run happened.
- **The roadmap's two rounded prose figures are named and replaced.** The seven rows' `build_s` spans 34 to
  51 rather than "47 to 51", and `r` spans 10.5143 to 11.6735 rather than "10.6 to 11.7". Both are printed as
  the file's own values beside the rounding they replace.
- **`r` is arithmetic only.** Lesson 29 retires it as a signal because its denominator moves with the
  runner; it is printed here so a reader who recomputes the window finds it accounted for rather than
  concluding it was overlooked.
- **The verdict is `reprice`.** The median moved 367 to 572 with an unchanged token line, which is the
  runner-event reading section 2.4 prescribes. No lever is invented to explain it, and the priced addition
  (0.11) is charged against the 21 allowance before the line is written, not after.
- **EV-08 is not marked complete here.** This plan adds no EV-08 registry row, and the `important-gates`
  step refuses a `Complete` requirement with no row, so flipping the traceability entry here would put the
  tree red. The flip and the first row land together in plan 17-02 by construction (section 2.7).

## Deviations from Plan

None — plan executed exactly as written. The window, the previous window, the row totals, the seven counts
and both ranges matched the plan's predictions on the first measurement; docker was available, so the
container gate ran rather than being recorded as not run; and the only file touched is the one the plan names.

## Issues Encountered

None. Two things are worth recording because a later reader will meet them and should not mistake them for
movement: the container line carries four host-dependent tokens that differ from the host line and were
different before this plan started (`cc=GNU`, `core-imports=5`, `archive=sha256`, `refs-verified=0/4`), and
the three `git ls-files` counts read 2806 in the gate logs because the gate ran before this record was
tracked, while `git ls-files` at the commit reads 2807.

## Next Phase Readiness

- The milestone now has a measured opening window. Plans 17-02 and 17-03 quote these three medians and the
  21 allowance rather than restating the roadmap's rounded prose, and their `## Baselines` sections take the
  same shape: the plan's own start commit sha, printed, with every figure measured there.
- Phase 18 (2.96), Phase 19 (1.64) and Phase 20 (0.02) price against this allowance. Nothing in this plan
  constrains them, and nothing in this plan needs re-reading to price them.
- No blocker. The one live assumption the phase carries (the Ubuntu dev preset's behaviour for the new id)
  belongs to plan 17-02 and is not affected by anything measured here.

---
*Phase: 17-records-cost-window-and-the-count-gate-contract*
*Completed: 2026-11-26*

## Self-Check: PASSED

- The one file this plan creates exists at the path its frontmatter names: `17-01-SUMMARY.md`.
- The record commit `99df1e3` is an ancestor of this commit, so the record is tracked at the commit it
  describes rather than sitting untracked beside it.
- `.planning/ci-runs.tsv` is byte-identical to the file the plan started from: no row was appended, inserted
  or removed, and the seven release-records rows are present once each.
- `sh` and `dash` produced identical `gate: PASS` lines (1,520 bytes each, `cmp` clean) with every token the
  plan predicted; the container gate passed once and its four host-dependent tokens are the pre-existing ones.
- Exactly one `checkpoint:` line exists, it reads `reprice`, and it follows the pricing sentence.
