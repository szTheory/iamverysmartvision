---
phase: 10-reachable-runs-and-presentation-policy
plan: "06"
subsystem: planning-records-and-gate-registry
tags: [requirement-close, important-gates, exclusion-ledger, append-only, phase-exit]
dependency_graph:
  requires: ["10-04", "10-05"]
  provides:
    - "PKG-05, TOOL-04 and EV-05 Complete with their five ledger rows"
    - "three EV-05 registry rows (the first claims and text rows the registry has had)"
    - "four appended stale-figure corrections and lesson 25"
    - "the Phase 10 exit evidence, taken after the closing records commit"
  affects: [".planning/REQUIREMENTS.md", "data/expected/important-gates.ids", "data/expected/important-gates-excluded.ids"]
tech-stack:
  added: []
  patterns: ["derived must-be-covered set: Complete minus excluded equals registered, in both directions"]
key-files:
  created:
    - .planning/phases/10-reachable-runs-and-presentation-policy/10-06-SUMMARY.md
  modified:
    - .planning/REQUIREMENTS.md
    - data/expected/important-gates.ids
    - data/expected/important-gates-excluded.ids
    - tools/check/steps/important-gates.sh
    - .planning/ops/GUARDRAILS-v0.2.md
    - .planning/STATE.md
    - .planning/milestones/v0.1.0-phases/07-first-visible-original-scenario/07-VERIFICATION.md
    - .planning/phases/09-registered-evidence-and-gate-cost/09-09-SUMMARY.md
    - .planning/ops/PLAN-LESSONS.md
    - .planning/ops/INDEX.md
    - .planning/ROADMAP.md
decisions:
  - "The three requirement flips land with their five ledger rows in one commit, because the accepted set is derived: a flip without its rows fails important-gates-complete-uncovered and a row without its flip fails important-gates-excluded-stale."
  - "Amendment A-04's rule stands and its figure is corrected by measurement: lines moves 330 to 333, not 335, because the stats line counts registry rows only and the two exclusion rows are in a different file."
  - "Four stale figures are corrected as appended notes with the historical sentence intact; none is deferred, so none needs a disposition row."
  - "The orchestrator's ui-plan-gate false positive is recorded as lesson 25 and settled with no action."
metrics:
  duration: "~70 min wall clock"
  completed: 2026-11-25
  tasks: 3
  files: 12
  commits: 4
  plan_head_before: c429333445851211502fc5859aa529295fd9027d
  plan_head_after: 26f6c616ccedbce1b6a2ef2404f22757e21839fe
status: complete
actuals:
  tokens: 12500
  tasks: 3
  commits: 5
---

# Phase 10 Plan 06: The requirement close, the stale records and the phase exit Summary

Three requirements flip to `Complete` in the same commit as the five ledger rows that make the
derived must-be-covered set legal in both directions, `gates` moves 107 to 109 and `lines` 330 to
333 exactly as predicted (the amendment's 335 corrected by measurement), four stale figures are
corrected as appended notes with nothing rewritten and nothing deferred, the orchestrator's
`ui-plan-gate` false positive is recorded as lesson 25, and the phase exit was taken **after** the
closing records commit so the counts the gate reads are the counts of the tree it read.

`commits: 4` in the frontmatter is the measured `git rev-list --count c429333..26f6c61`: Task 1,
Task 2, Task 3's lesson commit and the closing records commit. `actuals.commits: 5` counts this
file's own commit, which cannot measure itself.

## Task 1 - the close, and the arithmetic that closes

### The identity, before and after

Measured with the important-gates step's own awk (the derived-set block at
`tools/check/steps/important-gates.sh:104-120`, reproduced verbatim in the command below), run on
the tree before any edit and again after the commit:

```
for g in .planning/REQUIREMENTS.md .planning/milestones/*-REQUIREMENTS.md; do
  awk -F'|' '/^\| *[A-Z]+-[0-9]+ *\|/ { id=$2; st=$4; gsub(/^ +| +$/,"",id); gsub(/^ +| +$/,"",st);
    if (st ~ /^Complete/) print id }' "$g"
done | sort -u
```

| Reading | Complete | Excluded | Accepted | Registered | Set differences |
|---|---|---|---|---|---|
| Before the commit | 54 | 10 | 44 | 44 | both empty |
| After the commit | 57 | 12 | 45 | 45 | both empty |

The identity closes by arithmetic: `57 - 12 = 45 = 45`. Both directions were measured, not assumed:
`comm -23 accepted registered` and `comm -13 accepted registered` are each empty on the committed
tree, which is what makes `important-gates-complete-uncovered` and
`important-gates-excluded-stale` both silent rather than one of them silenced.

### `gates` and `lines`, and the amendment's figure

The mechanism's own stats line is `Stats: gates=<distinct gate names> lines=<registry lines>`
(`important-gates.sh:74`), printed by the END block at `:175-181`. `lines` counts registry rows only
- the non-`#`, non-empty lines of `data/expected/important-gates.ids` - and the exclusion ledger is a
different file. Three registry rows therefore add three lines and the two exclusion rows add none.

| Token | Before | Predicted by this plan | Predicted by amendment A-04 | Measured |
|---|---|---|---|---|
| `gates` | 107 | 109 | 109 | **109** |
| `lines` | 330 | **333** | 335 | **333** |

Amendment A-04's rule is honoured - predict before the commit, and `gates`/`lines` are not frozen for
this phase - and its figure is corrected by measurement. A plan that carried 335 forward would have
halted on a green tree, which this project treats as the worse failure. `gates=109` and `lines=333`
are now settled for the rest of the phase and every later run reads them.

### What landed in the one commit (`b14e1a4`)

- `.planning/REQUIREMENTS.md`: EV-05, PKG-05 and TOOL-04 read `Complete` in the `## Traceability`
  table and their `- [ ]` checkbox lines are ticked. Three `Complete` rows, zero unticked.
- `data/expected/important-gates.ids`: three rows under a `# Phase 10 (plan 10-06):` comment -
  `claims control claims-handoff-artifact EV-05`, `claims control claims-handoff-row EV-05`,
  `text control text-untagged-look-listen EV-05`. Both gate names are new to the registry; all three
  ids resolve in `data/expected/controls/claims.ids` (lines 2, 3 and 14), which plan 10-05 wrote.
- `data/expected/important-gates-excluded.ids`: `PKG-05
  close_record_only_release_rehearsal_transcript_and_moved_prefix_run` under the close-record
  category, and `TOOL-04
  gate_machinery_self_evidence_enforcement_predates_the_requirement_core_symbols_palette_defined`
  under the gate-machinery category with A-02's token verbatim as the suffix. The header sentence
  that stated the file's row count was corrected from seven to twelve (it was already stale at ten).
- `tools/check/steps/important-gates.sh`: a `#   Phase 10 EV-05 plan 10-06 (...)` line in the phase
  table, and the header's "the six requirements it lists" corrected to twelve.

### The retarget measurement (GUARDRAILS R2)

- **`nreq` is not a split string.** `grep -n nreq tools/check/steps/important-gates.sh` returns four
  lines - 156, 160, 177, 180 - and all four are the same awk counter over the derived accepted set
  (`reqs[++nreq]` at 156, then `i <= nreq` at 160, 177 and 180). Plan 09-05 deleted the hand-written
  list when it made the accepted set derived. So only the header comment and the token prose could
  need a retarget, and both did.
- **The toy-registry limb does not apply, measured.** `grep -rn EV-05 tools/check/controls/_good/tree/`
  returns nothing, so the toy requirement record carries no EV-05 and no toy registry line is due.
  The toy `control-toy-drift` check stays green in every run below.

### Files moved

Four modifications, zero new files, so the file delta against the task-start baseline is exactly 0
(`files=2600` on both sides).

## Task 2 - four stale figures, four appended notes

Every note is an appended line; the diff shape below is `git show --numstat` of `4ff8a7c`, and the
deleted column is zero in all four files, so the append-only claim is a measurement.

| Stale sentence | Measured value | Where the note sits | Command | Diff shape |
|---|---|---|---|---|
| GUARDRAILS R2 names an `nreq` split string | `nreq` is an awk counter at four lines; the split string was deleted by plan 09-05; the toy limb carries no EV-05 | `.planning/ops/GUARDRAILS-v0.2.md`, "Note on R2, appended by plan 10-06", directly under the retarget table | `grep -n nreq tools/check/steps/important-gates.sh`; `grep -rn EV-05 tools/check/controls/_good/tree/` | 17 added, 0 deleted |
| `STATE.md` item 4 says D10 has 47 hardware items while the Owner items row says 44 | 44 `D10-*` rows plus 3 `D11-*` rows, 47 in total: one measurement read two ways | `.planning/STATE.md`, a note under the Deferred Items table naming the Owner items row, and a sub-note under Recorded Constraints item 4 | `awk '!/^#/ && NF {print $1}' data/expected/d10-measurements.ids \| sed 's/-.*//' \| sort \| uniq -c` | 12 added, 0 deleted |
| `07-VERIFICATION.md` section 2's H7-3 row says the D10 hardware items number 42 | 44 `D10-*` rows and 3 `D11-*` rows today; `data/expected/handoffs.ids` cites the file by path and never by a count | `.planning/milestones/.../07-VERIFICATION.md`, an appended note section at the end of the file; the H7-3 row itself is untouched | as above | 14 added, 0 deleted |
| `09-09-SUMMARY.md` quotes `controls=384/384`, `files=2555`/`2556` and `cases=12` | `controls=389/389`, `files=tracked=hygiene-files=2600`, `cases=13` | `.planning/phases/09-registered-evidence-and-gate-cost/09-09-SUMMARY.md`, an appended note section at the end | `tail -1` of `sh tools/gate.sh`; `awk '!/^#/ && NF' data/expected/controls/*.ids \| wc -l` | 21 added, 0 deleted |

**Dispositions: none of the four is deferred**, so no disposition row was added to any register. The
one that would have needed a row had it been uncorrectable is `09-09-SUMMARY.md`: its gate lines are
a frozen oracle of the Phase 9 tree and a check that compared the whole file byte for byte would have
made the correction impossible. No such check exists - `09-09-SUMMARY.md` is read by nothing - so the
appended section was enough and no row is owed.

`gates=109` and `lines=333` are unchanged by this task: four modifications, no ids row and no
registry line, and the gate run below confirms both tokens.

## Task 3 - the false positive, the records commit, and the exit taken after it

### Lesson 25 and the index

`## Phase 10 (plans 10-01 to 10-06; added by plan 10-06)` closes `.planning/ops/PLAN-LESSONS.md`
with lesson 25, and `.planning/ops/INDEX.md`'s `PLAN-LESSONS.md` row now states the range **1-25**
instead of 1-24. The index row is a live record and was edited rather than annotated, because leaving
a stale range beside a file that grew is exactly the defect Task 2 corrects elsewhere.

**Lesson 25, settled with no action:** the orchestrator's `ui-plan-gate` read a frontend signal from
the words `presentation` and `palette` in this phase's own prose while its own report said the
blocking flag was false. The phase adds no frontend and `10-CONTEXT.md` excludes one ("a windowed
frontend" is in the not-in-this-phase line). The disposition is recorded, not worked around:
re-wording a frozen phase document to satisfy a substring would trade a correct record for a quiet
scanner. The next run reads a decision instead of re-litigating it.

### The ROADMAP plans line

The orchestrator had already filled the six entries (10-01 to 10-05 checked, `**Plans**: 5/6 plans
executed`); this plan verified the six entries rather than replacing them, checked 10-06 and set the
line to `6/6 plans executed`. The five criteria, the tier ceiling and the not-in-this-phase line are
untouched. `gsd_run query roadmap.update-plan-progress 10` was run before and after; its first run
reported `summary_count: 5` and updated the 10-05 entries, and after this plan's own commit it
reports that the file already reflects the phase's counts.

### The closing records commit

`26f6c61 docs(10): close the phase records - six of six plans, position and metrics` - the ROADMAP
plans line, and `STATE.md`'s frontmatter status, Current Position, final counts, two decisions added
via `state.add-decision --phase 10`, the session-continuity block and the metric row. No
`.planning/config.json`, no `.planning/state.json` and no `.planning/ci-runs.tsv` was staged.

## The phase exit, taken after the closing records commit

Three shapes. Two local, both re-run **after** `26f6c61` - the defect plan 09-06 hit, where its own
records commit moved a count the gate reads after the last gate run. Host: macOS, AppleClang, this
branch, worktree clean apart from `.planning/state.json` (orchestrator state, never staged).

**Local `sh` (tail of `sh tools/gate.sh`, exit 0, 196 s wall):**

```
gate: PASS steps=38/38 controls=389/389 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=235 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=79 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=28 provisional=27 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=670 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=235/235 tracked=2600 binaries-listed=0 terms=9 doc-files=62 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2600 history-commits=1 gates=109 lines=333 input-rows=31 input-policies=11 input-profile-fields=31 files=2600 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=19/19 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=11/11 runner-files=16 cases=13 sources=12 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 docs=63 claim-refs=26 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

**Local `dash` (tail of `dash tools/gate.sh`, exit 0, 282 s wall), byte-identical to the `sh` line:**
`wc -c` 1463 both, `diff` of the space-split tokens empty, zero top-level failure lines in either log.

```
gate: PASS steps=38/38 controls=389/389 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=235 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=79 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=28 provisional=27 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=670 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=235/235 tracked=2600 binaries-listed=0 terms=9 doc-files=62 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2600 history-commits=1 gates=109 lines=333 input-rows=31 input-policies=11 input-profile-fields=31 files=2600 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=19/19 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=11/11 runner-files=16 cases=13 sources=12 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 docs=63 claim-refs=26 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

**Container gate: not run by this executor.** The dispatch's budget clause reserves
`sh tools/gate-linux.sh` for the orchestrator at the phase boundary, and this plan records that as a
visible not-run line with its reason rather than as a pass (lesson 24). There is no
`build/gate-linux.log` from this plan, and none is quoted. The orchestrator's run is the third shape.

### The token table

| Token | Predicted | Measured (post-records tree) | Moved by this plan? |
|---|---|---|---|
| `steps` | `38/38` | `38/38` | no |
| `controls` | `389/389` | `389/389` | no |
| `claims` | `28` | `28` | no |
| `provisional` | `27` | `27` | no |
| `claim-refs` | `26` | `26` | no |
| `gates` | `109` | `109` | **yes, 107 to 109** |
| `lines` | `333` | `333` | **yes, 330 to 333** |
| `faults` | `235/235` | `235/235` | no |
| `cases` | `13` | `13` | no |
| `goldens` | `29` | `29` | no |
| `prefix-files` | `11` | `11` | no |
| `docs` | `63` | `63` | no |
| `doc-files` | `62` | `62` | no |
| `tests-expected` | `4958` | `4958` | no |
| `tests-executed` | `4954` | `4954` | no |
| `tests-skipped` | `4` | `4` | no |
| `tests-missing` | `0` | `0` | no |

Derived at run time, observed and never predicated: `files=2600`, `tracked=2600`,
`hygiene-files=2600` on the post-records tree. This SUMMARY adds exactly one file, so the prediction
for the tree that carries it is `2601` for all three and every absolute token byte-identical; the
re-run after this file's own commit is recorded as a deviation below rather than claimed.

## Deviations from Plan

**1. [Approved budget deviation, from the dispatch] No container gate was run by this executor.**
- **Found during:** Task 3, phase exit
- **Issue:** the dispatch carries a hard 1800-second limit and records that six executors of this
  phase were stopped by it; it approves running `sh tools/gate.sh` at the end of each task and
  `dash` once at the end, and defers the container gate to the orchestrator at the phase boundary.
- **Fix:** the container line above is a visible not-run line with its reason, not a pass.
- **Commit:** recorded in `26f6c61` and here.

**2. [Rule 1 - Bug in the plan's own verify] The `09-09-SUMMARY.md` check expected four
`controls=384/384` occurrences; the file holds five.**
- **Found during:** Task 2
- **Issue:** `grep -n -F 'controls=384/384'` matches five lines, not four: the four `gate: PASS`
  lines (67, 75, 387, 394) plus one prose line in the "What this phase did not do" list at 234. The
  plan's "four gate lines" describes the gate lines correctly and the check counted all matches.
- **Fix:** the archive was not touched to satisfy the count - removing an occurrence would violate
  GUARDRAILS 2.10. The correct predicate is "four lines that begin `gate: PASS` and carry the
  token", which holds; the measured total of five is recorded here.
- **Files modified:** none (a measurement, not an edit)
- **Commit:** `4ff8a7c`

**3. [Approved budget deviation] The post-SUMMARY re-run of both local gates was not taken.**
- **Found during:** Task 3, step 6
- **Issue:** the plan's step 6 asks for a third `sh` and a third `dash` on the tree that carries this
  file. The three `sh` runs and one `dash` already spent about 870 seconds of the 1800-second
  dispatch limit, and a further `sh` (196 s) plus `dash` (282 s) would have risked the stop that this
  dispatch exists to avoid. The dispatch's own success criterion is the re-run **after the closing
  records commit**, which was taken.
- **Fix:** the prediction is stated instead of measured - one added file, `files`/`tracked`/
  `hygiene-files` each `2601`, every absolute-and-frozen token byte-identical - and it is flagged
  here as unverified rather than presented as a result.
- **Commit:** this file's commit

## No `-- FAIL` line, and the executor's own prohibitions

- Zero lines beginning with a top-level failure marker in `/tmp/10-06-sh-postrec.out` and
  `/tmp/10-06-dash-postrec.out` (`grep -c '^-- FAIL '` returns 0 for both), and zero in the two
  earlier Task 1 and Task 2 runs.
- No executor called `gh`, pushed, appended `.planning/ci-runs.tsv`, or staged `.planning/config.json`
  or `.planning/state.json`. Every commit was staged by explicit path; `git status --short` after each
  commit shows only the orchestrator's `.planning/state.json`.
- No package manager was invoked and no dependency was added (`T-10-SC`, accepted).
- No archive was rewritten: `07-VERIFICATION.md` and `09-09-SUMMARY.md` each took appended lines with
  zero deleted lines.

## Phase 10 criterion 5

The gate summary keeps `steps=38/38` and `claims=28` at its Phase 9 value, `gates`/`lines` moved only
by the amounts this plan predicted, and no claim cites a handoff row: the documented route reaches at
most `installed-host-behavior` and the handoff carries no tier. `PKG-05`, `TOOL-04` and `EV-05` are
`Complete` on evidence the project can point at, in one commit with the five ledger rows that keep the
derived accepted set legal in both directions.

## Self-Check: PASSED

Created files verified present: `.planning/phases/10-reachable-runs-and-presentation-policy/10-06-SUMMARY.md`.
Commits verified ancestors of `HEAD`: `b14e1a4`, `4ff8a7c`, `09e4487`, `26f6c61`.
