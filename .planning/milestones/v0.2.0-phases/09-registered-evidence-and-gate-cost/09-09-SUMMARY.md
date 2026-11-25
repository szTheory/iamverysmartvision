---
phase: 09-registered-evidence-and-gate-cost
plan: "09"
subsystem: records-and-phase-exit
tags: [A2, A-06, EV-01, EV-02, EV-03, EV-04, D-09-05, D-09-06, bookkeeping, phase-exit, T-09-17]
requirements-completed: [EV-01, EV-03, EV-04]
dependency-graph:
  requires: [09-05, 09-06, 09-07, 09-08]
  provides: [corrected-project-and-release-records, seven-draft-validation-dispositions, ten-row-exclusion-ledger, four-complete-ev-requirements, phase-exit-evidence]
  affects: [10, 11, 12]
tech-stack:
  added: []
  patterns: [correct-by-measurement-not-by-memory, appended-note-never-a-rewrite, status-flip-with-its-ledger-row]
key-files:
  created:
    - .planning/phases/09-registered-evidence-and-gate-cost/09-09-SUMMARY.md
  modified:
    - .planning/PROJECT.md
    - .planning/ops/RELEASE-OPS.md
    - .planning/STATE.md
    - .planning/REQUIREMENTS.md
    - core/src/machine/machine_digest.c
    - docs/runner.md
    - data/expected/important-gates-excluded.ids
decisions:
  - "Every bookkeeping item was corrected by measurement and not by quotation, and every correction is an appended note: the historical sentence stays and the measured value sits beside it (lesson 21, GUARDRAILS 2.10)"
  - "The seven draft VALIDATION.md files are one row each in a live record, citing 08-ACCEPTED-DEBT.md section 1; no archive was opened for writing"
  - "The requirement flip and its exclusion rows are one commit, because the ledger is two-directional: either half alone turns the tree red"
  - "The parallel-negative-controls idea already had a row in STATE.md, so plan 09-09 updated that row in place instead of adding a duplicate row for the same item (a second row would be the stale-record defect this plan exists to remove)"
metrics:
  duration: "~22 min wall clock (plan_head_before 21:54 to the closing records commit)"
  completed: 2026-11-25
  tasks: 3
  files: 8
  commits: 2
  plan_head_before: 87d2417d37b3266e0700011e4c7e61bbbcbebec2
  plan_head_after: 31821b4243d6338ea67784c72685f86acb93d763
status: complete
actuals:
  tokens: 14922
  tasks: 3
  commits: 3
---

# Phase 9 Plan 09: The bookkeeping, the requirement close and the phase exit Summary

The last plan of the phase leaves no stale record it could have corrected: the D-31 `create_tag`
item turned out to be prose only (the key already reads `false`), the install list was eleven files
and not ten, the composite-version comment said 2 where the code says 3, `docs/runner.md` had no
bench golden command, the seven draft `VALIDATION.md` files are now one row each citing the debt
record instead of rewriting an archive, and the four `EV` requirements this phase delivers are
`Complete` with a ten-row exclusion ledger whose arithmetic closes exactly.

`commits: 2` in the frontmatter is the measured `git rev-list --count 87d2417..31821b4`, the two
task commits; the third commit of this plan is this file's own commit, which cannot measure itself.
`actuals.commits: 3` is the task count the plan priced.

## The phase exit evidence

Three gate shapes on the final tree of the phase. Two ran here; the container gate is the
orchestrator's, per the dispatch's budget clause (recorded as a deviation below, with a visible
`not run` line rather than a pass).

**Local `sh` (tail of `sh tools/gate.sh`, exit 0, 80 s wall on this host):**

```
gate: PASS steps=38/38 controls=384/384 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=235 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=78 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=28 provisional=27 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=666 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=235/235 tracked=2555 binaries-listed=0 terms=9 doc-files=59 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2555 history-commits=1 gates=107 lines=330 input-rows=31 input-policies=11 input-profile-fields=31 files=2555 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=19/19 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=11/11 runner-files=16 cases=12 sources=12 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 docs=60 claim-refs=26 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

**Local `dash` (tail of `dash tools/gate.sh`, exit 0, 74 s wall), byte-identical to the `sh` line**
(`wc -c` 1463 both; `diff` of the space-split tokens empty; zero top-level `-- FAIL ` lines in either
log):

```
gate: PASS steps=38/38 controls=384/384 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=235 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=78 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=28 provisional=27 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=666 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=235/235 tracked=2555 binaries-listed=0 terms=9 doc-files=59 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2555 history-commits=1 gates=107 lines=330 input-rows=31 input-policies=11 input-profile-fields=31 files=2555 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=19/19 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=11/11 runner-files=16 cases=12 sources=12 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 docs=60 claim-refs=26 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

**Container (`sh tools/gate-linux.sh`): not run by this executor.** The dispatch's budget clause
reserves it for the orchestrator at the phase boundary ("**Do not run the container gate** (the
orchestrator runs it at the phase boundary)"), so there is no `build/gate-linux-phase9-final.log`
and this record says so rather than reporting a pass it did not observe (lesson 24). The
orchestrator's run is the third gate line of the phase exit.

**After the closing records commit** (the SUMMARY commit, plus the STATE.md and ROADMAP.md update):
both local gates were re-run on that tree and the tails are in "After the closing records commit"
below. That re-run is the defect plan 09-06 hit, so it is done here by construction rather than
promised.

## The token table

### Absolute-and-frozen (an unexpected value is a stop)

| Token | Predicted | Measured (final tree) | Moved? |
|---|---|---|---|
| `steps` | `38/38` | `38/38` | no |
| `controls` | `384/384` | `384/384` | no |
| `faults` | `235/235` | `235/235` | no |
| `gates` | `107` | `107` | no |
| `lines` | `330` | `330` | no |
| `cases` | `12` | `12` | no |
| `tests-expected` | `4958` | `4958` | no |
| `tests-executed` | `4954` | `4954` | no |
| `tests-skipped` | `4` | `4` | no |
| `tests-missing` | `0` | `0` | no |
| `claims` | `28` | `28` | no |
| `provisional` | `27` | `27` | no |
| `docs` | `60` | `60` | no |

`docs=` is in this set because it counts scanned claim documents and `docs/runner.md` is already
counted, so Task 1's two added command lines moved nothing (they also carry no claim word from the
`text` step's list: `supports|compatible|passes|accurate|verified|runs|plays|playable`).

### Derived at run time (an observation, never a predicate)

| Token | Observed | Note |
|---|---|---|
| `files` | `2555` | equals `tracked` equals `hygiene-files` on the pre-SUMMARY tree |
| `tracked` | `2555` | same |
| `hygiene-files` | `2555` | same |
| `history-commits` | `1` | derived from the tree shape, not a count of this phase's commits |

File delta, measured in the delta form the plan mandates (`files` minus
`git ls-tree -r --name-only HEAD^ | wc -l`, run after each commit): **0** for Task 1
(`2555 - 2555`), **0** for Task 2 (`2555 - 2555`), and **+1** for Task 3, because this SUMMARY is
the one file Task 3 adds. No absolute `files=` value is asserted anywhere as a predicate.

## Task 1 - the bookkeeping, every item measured

| Item | What the record said | What was measured | Where it now says so |
|---|---|---|---|
| D-31 / D-42, the `create_tag` footgun | "The key reads `true` in the working-tree copy at the Phase 8 close, so the change is still to be made" | `.planning/config.json:15` reads `"create_tag": false`, set by `e0c88b4` (the records commit that closed stage 2 of REL-01, 2026-11-24). The item was **prose only**; no executor staged the file | `.planning/PROJECT.md` D-31 row, appended note; `.planning/ops/RELEASE-OPS.md` item (a) and orchestrator step 3 |
| The pinned install list | "the pinned ten files" | `awk '!/^#/ && NF' data/expected/pkg-install-files.ids \| wc -l` = **11** | `.planning/ops/RELEASE-OPS.md` P3: "the pinned eleven files", with the measurement beside it |
| The composite version comment | `machine_digest.c:12-14` "it is version 2 since plan 05-03" | `IVS_DIGEST_COMPOSITE_VERSION 3u` at `core/src/machine/machine.h:157`, and `core/include/ivs/ivs_machine.h:352` already says "composite version 3, plan 06-05" | `core/src/machine/machine_digest.c` header: "version 3 since plan 06-05", naming D-18 and D-16 |
| The bench golden writing commands | `docs/runner.md` had none (`grep -n bench docs/runner.md` empty) | `data/golden/bench-lamp-idle.digests` and `data/golden/bench-lamp-input.digests` exist, their `# scenario:` headers name `bench-lamp-idle` and `bench-lamp-input`, and `docs/bench.md` lines 92-95 give the workload table (`bench-lamp-snapshot` reuses `bench-lamp-idle`'s golden) | `docs/runner.md`, two commands in the existing fenced block plus one sentence |
| The draft validation files | one aggregate row: "the six draft validation files of Phases 2 to 7 and Phase 8's own `08-VALIDATION.md`" | seven files: `status: draft` on `02` to `08`; `01` is `status: validated` | `.planning/STATE.md` Deferred Items: seven rows, one per file |

**The one core touch.** `core/src/machine/machine_digest.c` is the single permitted `core/` change
and it is a comment and only a comment: `git show --numstat` reads 4 added and 3 deleted lines, and
every changed line matches `^[+-] \*` (the leading block comment). Line 407, whose comment describes
the `IVS_FAULT_DIGEST_COMPOSITE_VERSION_STALE` fault site, was **not** touched: it is correct in
context, and the plan says so explicitly.

### The disposition rows in `.planning/STATE.md`

Seven new `Validation` rows (`02` to `08`), each naming the file, its `draft` status, the
disposition already recorded by `.planning/milestones/v0.1.0-phases/08-*/08-ACCEPTED-DEBT.md`
section 1, and the milestone it waits for. The archives were **not** opened for writing: the rows
cite the debt record, they do not rewrite it (GUARDRAILS 2.10, lesson 21).

Three rows updated in place because this phase closed them: the gate-hardening row (**IN-01 to
IN-04 closed** by 09-07's three controls and one verification plus 09-06's pass-line mechanism; IN-05
and two smaller items stay open), the memmap-gen row (**closed** by 09-07 item 5), and the
machine-digest comment row (**closed** here). The Performance row's clause about the two missing
benchmark golden commands is corrected rather than left stale.

Three dispositions for what this phase did **not** do:

1. `denylist-no-files` (`tools/check/steps/denylist.sh:43`) and `runner-scan-empty`
   (`tools/check/steps/runner-scan.sh:58`) - **declined, not silently dropped** (09-07), reason: the
   phase priced three controls plus one verification plus one step-side mechanism for EV-03
   (amendment A-02), and two more controls is scope it did not price. Milestone: the next one, if the
   cost rule prices two controls.
2. The parallel negative controls idea - the row already existed (Gate speed), so it was updated in
   place to record that Phase 9 did not land it and that it stays a watching item governed by the
   cost rule. A second row for the same item would have been the stale-record defect this plan
   exists to remove (deviation below).
3. The GUARDRAILS 2.3 registration deferral, **all nine mechanisms of D-09-06 named individually in
   the row**: `important-gates-complete-uncovered`, `important-gates-excluded-stale`,
   `control-toy-drift`, `control-deviation-unused`, `control-pass-line-not-last`,
   `sources-src-token-whole`, `manifests-output-anchor`, `memmap-generator-refused`,
   `memmap-generator-missing`. The row carries the frozen `lines=330`, the reason (ROADMAP Phase 9
   criterion 3 and amendment A-08 freeze the count, so any row after 09-04 makes the phase end at a
   count its own criterion forbids) and the reversal condition (a later milestone re-derives the
   baseline, then adds the nine rows under EV-03/EV-04 with the non-vacuity sentence every other row
   carries). A reader of that row alone sees the whole standing gap.

## Task 2 - the requirement close, and the arithmetic that closes

`EV-01`, `EV-03` and `EV-04` flip to `Complete` (traceability table and requirement checkbox, the
convention `EV-02` set) in the same commit that appends their three rows to
`data/expected/important-gates-excluded.ids` under the `gate_machinery_self_evidence` category the
ledger docstring declares. `EV-02` already had its row from 09-05.

**The ten rows** (`awk '!/^#/ && NF'` = 10, `grep -c gate_machinery_self_evidence` = 4: three rows
plus the docstring line that reserves the category for this plan):

| Requirement | Reason | Category |
|---|---|---|
| `REL-01` | `close_record_and_release_action_only` | close record / release action |
| `API-01` | `instance_isolation_proved_by_suite_test_ids` | gate machinery |
| `API-05` | `core_purity_controls_registered_under_INP-03` | gate machinery |
| `API-06` | `api_effects_controls_are_unregistered_here` | gate machinery |
| `RUN-03` | `tests_inventory_step_is_the_inventory_gate_itself` | gate machinery |
| `PKG-02` | `tools_gate_sh_is_the_documented_local_command` | gate machinery |
| `EV-02` | `the_registration_commit_is_its_own_evidence` | gate machinery |
| `EV-01` | `gate_machinery_self_evidence_hosted_cost_row_and_trim_commits` | gate machinery |
| `EV-03` | `gate_machinery_self_evidence_three_controls_and_one_verification` | gate machinery |
| `EV-04` | `gate_machinery_self_evidence_derived_must_be_covered_set` | gate machinery |

**The identity, measured after the flip** with the same awk the step uses (`st ~ /^Complete/`, so
`REL-01`'s `Complete (stage 2 recorded)` counts):

```
complete=54  excluded=10  accepted=44  registered=44
54 - 10 = 44 = 44   HOLDS
```

`accepted` is `comm -23` of the Complete ids against the ledger's ids (44 lines), and `registered`
is the distinct last field of `data/expected/important-gates.ids` (44 ids over 330 lines). Both set
differences are empty in both directions: every accepted id has a registry line and every registry
id is accepted. The registry stays at the frozen `lines=330` and `gates=107`.

**Why the two halves are inseparable.** The ledger is two-directional
(`tools/check/steps/important-gates.sh:148-151`): `important-gates-complete-uncovered` fires for a
Complete requirement in neither the registry nor the ledger, and `important-gates-excluded-stale`
fires for a ledger row whose requirement is not Complete. So a row landing before the flip fails
the second, and a flip landing before the rows fails the first. Both directions are already
observed, in tree, at no cost to this plan:

```
-- FAIL important-gates-complete-uncovered: EV-02      (09-05, before its row existed)
-- FAIL important-gates-excluded-stale: EV-99          (control important-gates-complete-uncovered)
-- FAIL important-gates-complete-uncovered: EV-03      (09-06's orchestrator re-run, after an early flip)
-- FAIL important-gates-complete-uncovered: EV-04      (the same run)
```

The control's output shows both tokens in one run, so the scratch-tree demonstration the plan allowed
"if cheap" was not needed: the mechanism is already pinned by a committed, re-runnable case.

## What this phase did not do (the no-content statement)

- No new gate step: `steps=38/38`, `ls tools/check/steps/*.sh | wc -l` unchanged.
- No new test id: `tests-expected=4958`, `tests-executed=4954`, `tests-skipped=4`, `tests-missing=0`.
- No new control, fault or gate step in this plan: `controls=384/384`, `faults=235/235`.
- No registry line: `lines=330`, `gates=107`.
- No claim moved and no claim text added: `claims=28 provisional=27`.
- No package manager was invoked; no dependency was added (`T-09-SC`, accepted).
- No executor applied a repository setting, called `gh`, pushed or appended `.planning/ci-runs.tsv`.

## Cost

| Figure | Value | Source |
|---|---|---|
| The phase's recorded allowance | about 21 hosted cpu-s, about **12.9 local** cpu-s at the recorded 1.675 factor | 09-03, kept as priced |
| The same allowance recomputed from the last-five window | (8.3 - 7.696) x 46 = **27.8 hosted** cpu-s, about 16.6 local | 09-03, printed beside the priced figure, not instead of it |
| Wave A1 measured | **0.80 local cpu-s** = 0.029 `r` (lever 0.12 ceiling, trim (a) 0.18, trims (c)+(d) 0.50) | 09-03 |
| Wave A2, 09-04 | **+0.02 local cpu-s**, stated as indistinguishable from the load change (10.47 to 14.42) | 09-04 |
| Wave A2, 09-05 | four controls at a measured **0.193 cpu-s per control** (median of three runs at 381 controls, load 13.14) | 09-05 |
| Wave A2, 09-06 to 09-09 | **no per-plan cpu-s figure recorded**; these plans add no timed loop and their only cost is one step-side guard and data rows | the four SUMMARYs, read |
| This plan's own gate runs | `sh` 80 s and `dash` 74 s wall on this host, six local gate runs in total | measured here |

The honest total: **at least 1.59 local cpu-s of the 12.9 recorded allowance is measured**, and the
remainder of wave A2 was not priced plan by plan. No figure is asserted for the unpriced part, and
the gap is recorded as a lesson rather than filled with an estimate: 09-06 to 09-09 each changed a
count the gate reads, and none of them priced that change.

## Deviations from Plan

**1. [Budget] `dash` ran at the end of the plan and again after the closing records commit, not at
every task; the container gate did not run here**

- **Approved by the dispatch:** "Run `sh tools/gate.sh` at the end of each task and `dash
  tools/gate.sh` once at the end - record that as an approved budget deviation. **Do not run the
  container gate** (the orchestrator runs it at the phase boundary)."
- **Effect:** `sh` ran at the end of Task 1 and again after Task 2; `sh` and `dash` both ran on the
  final tree and both were re-run after the closing records commit. The container gate is a visible
  `not run` line in this SUMMARY with its reason, never a pass.

**2. [Rule 2 - record consistency] The parallel-negative-controls disposition updates its existing
row instead of adding a second row for the same item**

- **Found during:** Task 1, reading the `## Deferred Items` table.
- **Issue:** the plan asks for a row for the "run negative controls in parallel" idea, and
  `.planning/STATE.md` already has one (`Gate speed`, Phase 1, governed by the cost rule). Adding a
  second row would put one item in the table twice with two statuses - the stale-record defect this
  plan exists to remove.
- **Fix:** the existing row was updated in place to record that Phase 9 did not land it, why, and
  that it stays a watching item governed by the cost rule. The disposition is a row, as INDEX rule 5
  requires; it is the row that already existed.
- **Files modified:** `.planning/STATE.md`.
- **Commit:** `d6bb0da`.

**3. [Rule 2 - record consistency] Three rows beyond the plan's list were closed or corrected in
place**

- **Found during:** Task 1.
- **Issue:** the machine-digest comment row, the Performance row's clause about the missing bench
  golden commands, and the table's own intro paragraph all became stale in the same commit that
  fixes them.
- **Fix:** the hygiene row is marked closed (Task 1 changed the comment), the Performance row's
  clause is corrected, and the intro paragraph records the 09-09 refresh beside the 08-21 one.
- **Files modified:** `.planning/STATE.md`.
- **Commit:** `d6bb0da`.

**4. [Rule 2] The exclusion rows carry the category token inside the reason**

- **Found during:** Task 2.
- **Issue:** the plan names the rows as `EV-01 gate_machinery_self_evidence_cost_lever` and so on,
  while the ledger docstring requires the reason to name the evidence the requirement actually has.
  A bare category token would name no evidence; a bare description would make the category
  invisible to `grep`.
- **Fix:** both, joined: `gate_machinery_self_evidence_<what the evidence is>`. The plan's acceptance
  check `grep -c 'gate_machinery_self_evidence'` prints 4 (three rows plus the docstring line that
  reserves the category for this plan), and each reason still names its evidence.
- **Files modified:** `data/expected/important-gates-excluded.ids`.
- **Commit:** `31821b4`.

## Recorded deviations this plan completes or inherits

- **D-09-01** (09-01): `tools/check/steps/golden.sh` joined wave A1's closed file list so a
  `release-only` scenario could be skipped by name; measured effect `reports=32` to `30`,
  `release-only=2`. Both values are on this phase's final gate line, unchanged.
- **D-09-05** (09-05): the status flip lands **with** its ledger rows. Completed here: three rows
  plus three flips in `31821b4`, the identity measured at `54 - 10 = 44 = 44`. 09-05's own deviation
  1 had already reduced the count from four rows to three by giving `EV-02` its row early.
- **The pass-line proof deviation** (09-06, fixed by the orchestrator in `d3ff4bd`): a proof line
  that matched `^-- FAIL ` was advertised as a failure of a passing run. It now prints as
  `-- runner-selftest proof ...`. This plan prints no `-- FAIL ` line at the start of a line
  anywhere: `grep -c '^-- FAIL '` is 0 in both final gate logs.
- **The EV-03 count deviation** (09-06): criterion 4 asks for five gate-hardening items; two already
  had controls and one branch is unreachable from the control runner, so the count is three new
  controls, one verification and one step-side mechanism. `EV-03` is `Complete` on that count, not
  on a count of five, and the STATE.md row says which item is which.
- **The research-regex correction** (09-07): the `SRC-` class is `SRC-[A-Za-z0-9_-]+`, measured, not
  the research's proposed class; no registry id contains a dot.

## The rulesets: the orchestrator's action, not this executor's

The branch ruleset (PR required with 0 approvals, required status check `gate`, no up-to-date
requirement, no force push, no deletion, no bypass list) and the tag ruleset (target `tag`,
`refs/tags/v*`, deletion and non-fast-forward refused) are declared in this plan's `user_setup` and
are a repository-settings action. **No executor applied them and this SUMMARY does not claim they
are applied.** The orchestrator records the applied date and ruleset id in `.planning/AUTONOMY.md`
section 1, or writes "not applied"; the STATE.md `Settings` row still reads "**Not applied** (the
close read returned no ruleset)" as of this plan. One thing a ruleset cannot express, recorded here
so it is not assumed: "created only from a green `main`" is a tag ruleset's gap, not its feature.

## Threat surface

`T-09-17` (tampering with the bookkeeping commit and the requirement close) is `mitigate` and is
addressed on four lines: every correction is an appended note and never a rewrite; the single
`core/` touch was checked line by line (`git show --numstat` 4/3, every changed line `^[+-] \*`);
the status flip and its rows landed in one commit with the gate green on both sides of the edit;
and `.planning/config.json` / `.planning/state.json` are verified unstaged in every task commit
(`git show --stat` holds neither). The trust boundary "a live record -> an archive" was never
crossed: no file under `.planning/milestones/` was opened for writing. No new network endpoint, auth
path, file access pattern at a trust boundary or schema change: no Threat Flags. No package manager
was invoked.

## Known Stubs

None. Every file this plan touched is a record, a comment or a data row, and every row it added is
read by a step that runs in every gate (`important-gates` reads the ledger and the traceability
table; the `text` step reads `docs/runner.md`).

## Self-Check

- `.planning/phases/09-registered-evidence-and-gate-cost/09-09-SUMMARY.md` - this file, Task 3's only
  added file
- `data/expected/important-gates-excluded.ids` - 10 rows (`awk '!/^#/ && NF'`), 4
  `gate_machinery_self_evidence` matches
- `data/expected/important-gates.ids` - 330 rows, 44 distinct ids
- `.planning/REQUIREMENTS.md` - `EV-01`, `EV-02`, `EV-03`, `EV-04` all `Complete`, all four boxes
  ticked
- `.planning/STATE.md` - seven `Validation` rows citing `ACCEPTED-DEBT` (grep count >= 7), the two
  declined rows, the nine-mechanism row
- `.planning/ops/RELEASE-OPS.md` - `eleven` present, the `create_tag` correction appended
- `core/src/machine/machine_digest.c` - comment-only diff, 4/3 lines
- `docs/runner.md` - `bench-lamp-idle` and `bench-lamp-input` present
- commits `d6bb0da` (Task 1) and `31821b4` (Task 2) - both ancestors of `HEAD`
- `sh` and `dash` on the final tree - `gate: PASS`, both tails above, zero top-level `-- FAIL ` lines

## After the closing records commit

This is the re-run plan 09-06 had to be fixed for: its own closing records commit moved a count the
gate reads, after the last gate run, and the phase boundary read `gate: FAIL steps=37/38`. Here the
records commit (`47f5701`: ROADMAP.md 9/9, STATE.md position, metrics, session and status) and the
SUMMARY commit (`f52610e`) came first, and both local gates were re-run **on that tree**. Both end
`gate: PASS`, and the token differs from the pre-commit tail in exactly three derived fields and
nothing else - `files`, `tracked` and `hygiene-files` each `2555` to `2556`, because this SUMMARY is
the one file the plan adds. `diff` of the space-split tokens reports six changed lines, all of them
those three fields' two sides; every absolute-and-frozen token is byte-identical.

**Local `sh` after the closing records commit (exit 0, 72 s wall):**

```
gate: PASS steps=38/38 controls=384/384 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=235 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=78 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=28 provisional=27 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=666 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=235/235 tracked=2556 binaries-listed=0 terms=9 doc-files=59 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2556 history-commits=1 gates=107 lines=330 input-rows=31 input-policies=11 input-profile-fields=31 files=2556 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=19/19 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=11/11 runner-files=16 cases=12 sources=12 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 docs=60 claim-refs=26 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

**Local `dash` after the closing records commit (exit 0, 80 s wall), byte-identical to the `sh`
line** (zero top-level `-- FAIL ` lines in either log):

```
gate: PASS steps=38/38 controls=384/384 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=235 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=78 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=28 provisional=27 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=666 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=235/235 tracked=2556 binaries-listed=0 terms=9 doc-files=59 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2556 history-commits=1 gates=107 lines=330 input-rows=31 input-policies=11 input-profile-fields=31 files=2556 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=19/19 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=11/11 runner-files=16 cases=12 sources=12 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 docs=60 claim-refs=26 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

The lesson, restated because it is the one this plan could have repeated: a requirement mark, a
summary file and a status line are all counts the gate reads, so the gate runs **after** the records
commit and not before it. The predicted delta was stated in advance (+1 file, no other token), so the
re-run is a confirmation rather than a discovery.

## Appended note (plan 10-06, 2026-11-25): three counts that have moved since

Every gate line and token table in this record was measured on the Phase 9 final tree. Three of the
counts it quotes have moved since, so a reader must recompute rather than quote (AGENTS.md,
"Recompute, never quote"):

| Token | This record (Phase 9 final tree) | Measured on the Phase 10 close tree | Cause |
|---|---|---|---|
| `controls` | `384/384` | `controls=389/389` | the five controls Phase 10 landed: plan 10-02's route guard, plan 10-03's policy controls and plan 10-05's three (the two `claims` controls and the `text` control) |
| `files` / `tracked` / `hygiene-files` | `2555` on the pre-SUMMARY tree, `2556` after it | `files=2600 tracked=2600 hygiene-files=2600` | the rest of Phase 10's tracked files: `docs/running.md`, `docs/presentation.md`, `docs/handoffs.md`, `data/expected/handoffs.ids`, the control files and the ids rows |
| `cases` | `12` | `cases=13` | a scenario added after Phase 9 closed |

Nothing in this record's own lines has been altered: the four gate lines above are the record of that
tree and stay as taken, and this note sits under them (GUARDRAILS 2.10, lesson 21). Two tokens that did
not move and are still correct here: `steps=38/38` and `claims=28`. `gates` and `lines` were frozen at
`107` and `330` by Phase 9's own criterion and have since moved to `109` and `333`, deliberately, by
plan 10-06's requirement close.

Measured with `tail -1` of `sh tools/gate.sh` on the Phase 10 close tree, and with
`awk '!/^#/ && NF' data/expected/controls/*.ids | wc -l` for the control count.
