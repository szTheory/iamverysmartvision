---
phase: 17-records-cost-window-and-the-count-gate-contract
plan: 03
subsystem: records-count-gate
tags: [plan-lessons-32, count-gate, clause-to-mechanism, ev-08]

requires:
  - phase: 17-records-cost-window-and-the-count-gate-contract
    plan: 02
    provides: "the `cart-role` registry row, the Phase 17 phase-table block and the EV-08 flip the count-gate row lands beside"
  - phase: 15
    provides: "the `st-pass-line` runner-selftest case and its copied-runner stand-in, cited rather than duplicated for clause 3"
  - phase: 04
    provides: "the `pkg-install` step's `check_inventory` and its `extra-file` in-step case, the committed stand-in clause 1 points at"

provides:
  - "`.planning/ops/PLAN-LESSONS.md` lesson 32: the three clauses of the count-gate contract, each mapped to what proves it or to the stated absence of a proof"
  - "the `count-gate mutant extra-file EV-08` registry row over an existing in-step case whose committed stand-in the same comparison refuses on every gate run"
  - "the corrected lesson range in `.planning/ops/INDEX.md`"

affects:
  - "Phase 20: the close recomputes the token line, which now ends at `gates=120 lines=366`"

actuals:
  tokens: 7073
  tasks: 2
  commits: 2
  plan_head_before: 1dc57c2ebcafd7b94432ce1d9817554f266789b1
  plan_head_after: bfa0815

tech-stack:
  added: []
  patterns:
    - "a lesson whose every clause names its own mechanism or states that it has none, so a reader cannot quote a clause as if a gate enforced it"
    - "a registry row that points at a mechanism already committed rather than minting a case, with the stand-in re-run on every gate run instead of once"

key-files:
  created:
    - .planning/phases/17-records-cost-window-and-the-count-gate-contract/17-03-SUMMARY.md
  modified:
    - .planning/ops/PLAN-LESSONS.md
    - .planning/ops/INDEX.md
    - data/expected/important-gates.ids
    - tools/check/steps/important-gates.sh

key-decisions:
  - "The three clauses are indented list items with their number INSIDE the item (`  - **1.** ...`), because the file's column-1 shape is `N. **title**` and a clause written as `1. ` at column 1 would read as lessons 1, 2 and 3 to anything that parses the file by that shape — including this plan's own check that the highest numbered lesson is 32."
  - "Clause 2 is registered nowhere and the lesson says so in as many words. No gate step parses plans, so a row for the start-commit baseline would assert a mechanism that does not exist; naming the gap is the point, because a rule whose scope is wider than its mechanism teaches readers to quote it (lesson 31)."
  - "Clause 3 is cited, never duplicated: `d09-06-registration mutant st-pass-line EV-07` is already registered, and a second row for one proof is the second registration plan 13-04 named and refused."
  - "The new gate name `count-gate` (rather than reusing `pkg-install`) is deliberate: the row registers a contract, not a step, exactly as `d09-06-registration` registers one — and it is the one change that makes `gates` read 120."
  - "No case, test id, step, fault, golden or claim is added: the row points at the `extra-file` case plan 04-22 already built and already registered for `PKG-01`, so no 0.05 selftest price is incurred and the phase's priced line stays 0.11."

requirements-completed:
  - EV-08

# Metrics
duration: 15min
completed: 2026-11-26
status: complete
---

# Phase 17 Plan 03: Lesson 32, the Count-Gate Contract and the Row That Carries It Summary

**The count-gate contract recorded as lesson 32 with a clause-to-mechanism map that says which clause a gate
enforces and which one has no checker at all, carried by one registry row over an in-step case whose committed
stand-in the same comparison refuses on every gate run.**

## Baselines

start-commit: 1dc57c2ebcafd7b94432ce1d9817554f266789b1

Measured at that sha, before the first edit. Nothing is carried forward from 17-02's record: every figure below
was re-measured at this plan's own start commit and compared against the plan's prediction.

```
start-commit: 1dc57c2ebcafd7b94432ce1d9817554f266789b1
baseline: gate: PASS steps=38/38 controls=404/404 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=238 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=118 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=29 provisional=28 handoffs=8 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=695 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=238/238 tracked=2808 binaries-listed=0 rights-pinned=4/4 terms=9 doc-files=63 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2808 history-commits=1 gates=119 lines=365 input-rows=31 input-policies=11 input-profile-fields=31 files=2808 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=20/20 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=12/12 runner-files=18 cases=14 sources=13 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=5044 tests-executed=5040 tests-skipped=4 tests-missing=0 docs=64 claim-refs=27 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
baseline-registry-rows: 365
baseline-tracked-files: 2808
```

The gate line above was produced by `sh tools/gate.sh` at that sha, before the first edit, and the other two
figures are the `awk '!/^#/ && NF'` row count over `data/expected/important-gates.ids` and the `git ls-files`
count at the same commit. The plan predicted `gates=119 lines=365` and `tests-expected=5044`,
`tests-executed=5040`; all four matched, so no figure needed correcting and none was corrected. Both recorded
figures are recomputable at that sha, which is what this plan's own second clause asks for: the registry row
count is `git show 1dc57c2:data/expected/important-gates.ids` piped through the same `awk`, and the tracked count
is `git ls-tree -r --name-only 1dc57c2 | grep -c .`. Both tasks' verify assert them against that sha rather than
against a quotation.

## Lesson 32

`.planning/ops/PLAN-LESSONS.md` gains lesson 32 as its last numbered lesson, opening with what is new rather
than restating its neighbours: lesson 20 says a check that cannot fail proves nothing and lesson 31 says a rule
with no mechanism is quoted rather than obeyed, and both are about a check that does not check. Lesson 32 is the
narrower statement that three shapes — a count read from the wrong file, a baseline measured somewhere other
than the plan's own start commit, and a verdict decided by a pipeline tail — are one defect class, because each
passes a wrong implementation while reading green.

The three clauses are their own lines, each an indented list item with the number **inside** the item:

```
  - **1.** Every count gate names the file it counts.
  - **2.** Every baseline names the plan's own start commit sha and is measured there.
  - **3.** No verify block decides an outcome by a pipeline tail.
```

The shape is load-bearing, not cosmetic. The file's column-1 shape is `N. **title**`, one numbered lesson per
block, so a clause written as `1. ...` at column 1 would read as lessons `1.`, `2.` and `3.` to anything that
parses the file by that shape — including this plan's own check that the highest numbered lesson is 32, which
would then read the last one as `3.`. The lesson's own heading stays at column 1 as `32. `, and exactly one line
in the file begins with it.

### The clause-to-mechanism map

| clause | what proves it | what does not |
|---|---|---|
| 1. the file a count gate counts | the `count-gate mutant extra-file EV-08` row, over the `pkg-install` step's `extra-file` in-step case: the step's `check_inventory` reads the named file `data/expected/pkg-install-files.ids` (eleven paths) and prints that file's own name as `pkg-inventory-list` when it is missing, and the case plants `share/ivs/stray.txt` in a **copy** of the installed prefix and requires `pkg-inventory-extra: share/ivs/stray.txt` from the same function the real prefix is judged by | a comment claiming the count is named; a case built, run and deleted once (nothing re-runs it) |
| 2. the baseline's start-commit sha | **no mechanism**, stated as such: no gate step parses plans, so the only evidence is a plan that prints the sha it measured at and a record a reader can re-derive — which this record's `## Baselines` section is | any registry row: a row for this clause would register a mechanism that does not exist |
| 3. no verdict by a pipeline tail | the already-registered `d09-06-registration mutant st-pass-line EV-07` row and the in-step case behind it: a copied runner whose step prints one line after its pass line must be refused, and the gate-side `tools/check/pass-line.sh` refuses a verdict followed by a line (exit 2) and a run with no verdict (exit 1) while accepting the one legal shape | a second row for the same proof — the second registration plan 13-04 named and refused |

The lesson closes with the four invariants as figures a reader recomputes from the tree rather than believes,
with the fault count's own command shape beside them (`cat data/expected/faults/*.ids | grep -v '^#' | awk 'NF' |
wc -l` reads 238; `data/golden/` holds 29 files; the `claims` array of `data/claims.json` holds 29 entries of
which 28 stand `provisional`). They are printed on one line, so the check is a whole-line comparison and not a
reading of prose:

```
invariants: faults=238 goldens=29 claims=29 provisional=28
```

`.planning/ops/INDEX.md`'s `PLAN-LESSONS.md` row is corrected from `1–25` to `1–32`, keeping its other text and
its en dash. It was a quoted figure that disagreed with the file — the same defect the lesson records — and no
`1–25` survives anywhere in the index.

## The row and the phase-table sentence

`data/expected/important-gates.ids` gains one row at its tail, `count-gate mutant extra-file EV-08`, with a
sixteen-line comment above it in the file's voice and pure ASCII (the file is in the ASCII-only set; the appended
block was checked with a `[^ -~]` scan and is clean). The comment states what the row proves, that the count
comes from a named file, that the stand-in is committed code re-run on every gate run rather than a fixture, that
the row **points** at a mechanism which already exists (the same case is registered for `PKG-01` under the step's
own gate name) instead of minting a case to register — adding one would cost 0.05 hosted cpu-s and reprice the
phase from 0.11 to 0.16 — that clause 3 is cited and not duplicated, and that clause 2 has **no** mechanism and
is therefore not registered. It also says why the gate is named `count-gate` rather than `pkg-install`: the row
registers a contract, not a step, exactly as `d09-06-registration` registers one, and the new name is what makes
`gates` read 120.

`tools/check/steps/important-gates.sh`'s phase table gains one Phase 17 continuation paragraph naming plan 17-03,
the `count-gate` gate name, the one row, and the fact that no case, test id, step, fault or golden was added. The
table is history for a reviewer and may not silently omit the row that mints a new gate name.

The registry step prints `-- important-gates pass gates=120 lines=366` with no finding, the row count is 366 and
the step count is still 38.

## The mechanism, from committed material only

No fixture was built, run and deleted, and no throwaway probe is offered as the evidence. The four committed
facts are all in `tools/check/steps/pkg-install.sh`:

| fact | where |
|---|---|
| `check_inventory() {` — the definition, reading `inv="$ROOT/data/expected/pkg-install-files.ids"` (`:98`) | `:101` |
| the same function judging the **real** installed prefix | `:241` (`check_inventory "$pb" \|\| rc=1`) |
| the same function judging the `extra-file` case's **copy** of that prefix | `:666` (`check_inventory "$d/prefix"`) |
| the refusal the case requires, `-- FAIL pkg-inventory-extra: share/ivs/stray.txt` | `:667-668` |

The named file the count comes from is `data/expected/pkg-install-files.ids`, eleven paths, and the step prints
that file's own name when it is missing (`pkg-inventory-list: %s`, `:104`) — a count taken from anywhere else is
a finding rather than a number.

This task's own gate run carries `pkg-selftest=20/20` on its summary line, which is the step's count of in-step
cases that failed as intended with `extra-file` among them: the stand-in was refused in that run, not in a
previous one. Because the stand-in is committed code inside the step, the guard is shown able to fail on **every**
gate run rather than once. The row adds no case, no test id and no step, so no 0.05 selftest price is incurred
and the phase's priced line stays 0.11 (one process id plus the two 0.005 rows).

## Gate evidence

| run | summary line |
|---|---|
| `sh tools/gate.sh` at the start commit (baseline) | `gate: PASS … gates=119 lines=365 … pkg-selftest=20/20 … tests-expected=5044 tests-executed=5040` |
| `sh tools/gate.sh`, after Task 1 | the same line except `tracked=2809 files=2809 hygiene-files=2809` (this record was present, untracked) |
| `dash tools/gate.sh`, after Task 1 | the same line, byte for byte |
| `sh tools/gate.sh`, after Task 2 | `gate: PASS … faults=238/238 … goldens=29 … gates=120 lines=366 … pkg-selftest=20/20 … tests-expected=5044 tests-executed=5040 tests-skipped=4 tests-missing=0` |
| `dash tools/gate.sh`, after Task 2 | the same line, byte for byte |
| `sh tools/gate-linux.sh` (Ubuntu container, `cc=GNU`) | `gate-linux: exit 0`, `gate: PASS … gates=120 lines=366 … pkg-selftest=20/20`, `gate-time total=258` |

The container gate ran once, after `build/gate-linux.log` was copied aside first (lesson 18), and no `ivs-`
container was left behind. Its platform tokens are the known ones (`cc=GNU`, `core-imports=5`, `archive=sha256`,
`refs-verified=0/4`); every token this plan predicts is identical to the host run's.

The step was run directly as well: the real tree prints `-- important-gates pass gates=120 lines=366` with no
`-- FAIL important-gates-` line, and `tools/check/steps/runner-selftest.sh` prints its observed-refusal proof
line `-- runner-selftest proof control-pass-line-not-last: profile (observed: …)` with no `-- FAIL
runner-selftest` line — the tail clause's proof is in the log, not in this plan's prose. `sh tools/hygiene.sh
--tree` exits 0 and reports no finding (it prints its own pass token, as 17-02 recorded).

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] `sh tools/hygiene.sh --tree` prints its pass line rather than being silent**

- **Found during:** both tasks' verify.
- **Issue:** the plan's `fails_when` reads "or `sh tools/hygiene.sh --tree` prints anything", which treats any
output as a failure. The command prints `-- hygiene pass mode=tree hygiene-files=<n>` on success; the gate's own
`hygiene-files=` token is that line.
- **Fix:** the intent — no finding — is asserted instead: the command exits 0 and reports no finding. Same
recording as 17-02, which met the identical phrasing.
- **Files modified:** none.
- **Commit:** n/a (no code change).

**2. [Rule 3 - Blocking] The plan's baseline order: the gate ran before this record was created**

- **Found during:** Task 1 step 0.
- **Issue:** the plan asks for `17-03-SUMMARY.md` to be created first and the baseline gate to run after it,
while also asking the baseline to be measured "before editing any file". Creating the record is itself a write,
and the gate's `tracked` token counts the work tree, so the two orders differ by one untracked file.
- **Fix:** the gate ran **before** any file was created, so the recorded baseline line is a clean tree's line
(`tracked=2808`, equal to `git ls-files` at the sha). Nothing in the plan's verify depends on the order: the
recorded figures are asserted against the tree at `start-commit`, not against a work-tree moment.
- **Files modified:** none.
- **Commit:** n/a.

### Recorded, not auto-fixed

**3. [Rule 4 - scope] The plan commits on `main`; the harness's protected-branch assertion is inapplicable**

- **Found during:** the first commit.
- **Issue:** the harness's pre-commit assertion refuses a commit whose branch
`gsd_run query git.base-branch --is-protected` calls protected, and it calls `main` protected. This project's
`git.branching_strategy` is `none` and the orchestrator dispatched this plan as a sequential executor on the main
working tree, which is where every prior phase of this milestone committed.
- **Fix:** none attempted. Editing `.planning/config.json` to add `allow_default_branch_commits` would have
modified orchestrator state this plan may not stage. The commits are on `main`, as the project does, and the
condition is recorded here rather than silently ignored or worked around. Identical to 17-02's record.
- **Files modified:** none.
- **Commit:** n/a.

## Open items

None. No stub, skipped test or unrun verify was left behind: the container gate ran and passed, both shells agree
byte for byte, and no token moved that the plan did not predict.

## Self-Check: PASSED

- Every file this plan claims to have created or modified exists: this record, `PLAN-LESSONS.md`, `INDEX.md`,
  `data/expected/important-gates.ids` and `tools/check/steps/important-gates.sh`.
- Lesson 32 is the file's last numbered lesson, exactly one line begins `32. `, the three clause lines are
  present verbatim in their list-item shape, and none of them appears as `N. ` at column 1.
- `start-commit: 1dc57c2ebcafd7b94432ce1d9817554f266789b1` is a 40-hex commit that is an ancestor of `HEAD`, and
  both recorded baseline figures recompute to themselves at that sha (`365` registry rows, `2808` tracked files).
- The gate line of Task 2's run carries `gates=120`, `lines=366`, `faults=238/238`, `goldens=29`,
  `claims=29`, `provisional=28`, `steps=38/38`, `prefix-files=11`, `pkg-selftest=20/20`,
  `tests-expected=5044`, `tests-executed=5040` and `tests-skipped=4`.
