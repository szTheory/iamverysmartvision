---
phase: 09-registered-evidence-and-gate-cost
plan: 04
subsystem: Phase 9 wave A2, plan 1 of 6 - the registration commit: 17 requirements named against mechanisms the gate already runs, in the same commit as the accepted set, the registry prose and all ten toy registries
tags: [A2, EV-02, R2, A-08, A-09, registry, accepted-set, phase-table, toy-registries, numstat, T-09-07, T-09-08]
status: complete

requires:
  - phase: 09-registered-evidence-and-gate-cost
    provides: "plan 09-03 end state at e8a740c: gate: PASS steps=38/38 controls=380/380 faults=235/235 gates=90 lines=306 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 claims=28 provisional=27; the A1 cost record landed; and the A1 hosted row taken on head 93e6b9f (run 45527058543, pull_request, build_s 43, cpu_s 354, r 8.2326, checkpoint phaseA1: discharged-below-budget, median 7.696), which is the ordering precondition this plan's first task asserts"
provides:
  - "`data/expected/important-gates.ids` at 330 non-comment rows and 107 distinct gate names, naming at least one cannot-pass-vacuously mechanism for each of FND-01..07, CPU-01..05 and MEM-01..05 - 24 rows, 0 deleted, every id verified against the step's own five resolution loops before it was written"
  - "`tools/check/steps/important-gates.sh`: `nreq` at 43 accepted requirements, the header's phase enumeration replaced by a single phase-to-plan table that names Phase 1, Phase 2 and Phase 3 with plan 09-04, and token prose that no longer enumerates a phase subset (the Phase 4-8-only enumeration was the defect this plan removes)"
  - "all ten toy registries at 44 non-comment rows in `_good` and 43-45 in the overlays, each new line appended after the existing last line, 17 additions and 0 deletions per file by `git diff --numstat`, with the positional EXPECT tokens of `important-gates-bad-req-phase5` (line 19) and `-phase6` (line 24) intact"
  - "three gate lines on the tree of this commit: local `sh` and local `dash`, both `gate: PASS` with identical tokens and zero `-- FAIL` lines; the container gate is recorded as NOT RUN with its reason (approved budget deviation below)"
affects: [09-05, 09-06, 09-07, 09-08, 09-09]

actuals:
  tokens: 6002   # chars/4 over the realized diff add28f71..HEAD (24008 chars)
  tasks: 3
  commits: 2
plan_head_before: add28f71
plan_head_after: 285807e

# The SHA of `git rev-parse HEAD` before Task 1's first edit. Every range below is written
# `<plan-base>..HEAD` against this line, so the plan's span is explicit and re-measurable.
plan-base: add28f71ad7e8625478aa8c5e746b50209a66e6c
row-commit: 93e6b9f

key-decisions:
  - "The header enumeration was replaced by a table, not extended in place. The plan offered 'extend the enumeration' or 'prefer every accepted requirement with the phase list given once, in the header, as a table of plan to phase'. The second was chosen because a prose list of phases is the exact artifact that rotted: it said Phase 4 to Phase 8 and omitted Phase 1 to Phase 3 for four phases' worth of commits. One table, one place, and the two token sentences now say 'the accepted set above' instead of re-listing phases."
  - "Task 1's registry block was written with em-dashes in its prose comments and `integrity-non-ascii` refused it (`-- FAIL integrity-non-ascii: data/expected/important-gates.ids`, then `-- FAIL gate-step-failed: integrity`, `steps=37/38`). Fixed in the same task by `LC_ALL=C sed -i '' 's/ — /; /g'`; the three affected lines were rewritten to semicolons, no row changed, and the re-run was green at `steps=38/38`. Recorded here because the first gate run of this plan failed and a later reader comparing run counts would otherwise find an unexplained `37/38`."
  - "No control was retargeted, and the clause is satisfied rather than omitted (RESEARCH Q4.2). `grep -c 'INP-04' data/expected/important-gates.ids` and `grep -c 'VID-99' ...` both print 0: the two never-valid ids exist only in their own overlays, at line 19 of `important-gates-bad-req-phase5` and line 24 of `-phase6`, and both are outside the old and the new accepted sets, so neither became valid. Both controls still print `control ok`, which is the proof."
  - "The ten toy files were appended to with no comment line and no header edit, because the only count they are judged by is `git diff --numstat` and a `# Plan 09-04 ...` line there would make it read 8 then 18 instead of 7 then 17. The plan-level prose that names this plan lives in the real registry and in the step's header, which is where a reader looks for it anyway."
  - "The real registry is counted over non-comment rows and not over numstat, and both figures are printed: 24 added / 0 deleted filtered, against 68 added / 0 deleted by `--numstat`. The 44-line difference is the `# Plan 09-04 ...` block, the 17 `# <REQ>:` property blocks and the blank lines between them - the file's own established convention, and never a row. `lines=` in the step skips `^#` and empty lines (`tools/check/steps/important-gates.sh` awk, `/^#/ { next }` and `NF == 0 { next }`), so it moves by 24 and by nothing else."
  - "The cost is reported as indistinguishable from noise rather than as a saving or a charge: the `important-gates` step reads 24 more registry lines and nothing else, and the alternated three-run median moved by +0.02 cpu-s while the load average moved from 10.47 to 14.42. The delta is stated with both numbers so a reader can see it is not separable from the load change; nothing is claimed about a cost this plan did not actually move."

key-files:
  created:
    - .planning/phases/09-registered-evidence-and-gate-cost/09-04-SUMMARY.md
  modified:
    - data/expected/important-gates.ids
    - tools/check/steps/important-gates.sh
    - tools/check/controls/_good/tree/data/expected/important-gates.ids
    - tools/check/controls/important-gates-bad-req-phase5/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-bad-req-phase6/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-req-uncovered-phase5/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-req-uncovered-phase6/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-req-uncovered-phase7/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-uncovered-gate/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-unknown-id-phase5/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-unknown-id-phase6/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-unknown-id/overlay/data/expected/important-gates.ids

requirements-completed: [EV-02]  # the registry half; the derivation that reads it is EV-04's plan

completed: 2026-11-25
---

# Phase 9 Plan 04: the registration commit Summary

**Seventeen requirements - FND-01..07, CPU-01..05, MEM-01..05 - now have a registered
cannot-pass-vacuously mechanism each, named against faults, controls and mutants the gate
already runs, so the registry says which requirement each one is the evidence for and the
step refuses a row that resolves to nothing. Twenty-four rows added, zero deleted, zero new
mechanisms of any kind.**

## The ordering precondition, proved once

The row this plan may not precede:

| Proof | Command | Exit |
|---|---|---|
| the row is an ancestor of this commit | `git merge-base --is-ancestor 93e6b9f HEAD` | 0 |
| the A1 commits are ancestors of the row | `git merge-base --is-ancestor e8a740c 93e6b9f` | 0 |

Row: head `93e6b9f`, run 45527058543, `pull_request`, `build_s` 43, `cpu_s` 354,
`r` 8.2326, checkpoint `phaseA1: discharged-below-budget`, median 7.696. This executor did
not push, did not call `gh`, and did not touch `.planning/ci-runs.tsv`.

## The 24 rows

Distribution: 7 requirements carry two mechanisms, 10 carry one (7x2 + 10x1 = 24). Every
`<gate>` name below is new - the previous 90 contained no `cpu*`, no `memmap*` except
`memmap-flip`, and no `machine`/`ivsc` family - hence `gates=90 -> 107`.

| # | `<gate>` | mechanism | `<id>` | REQ | cannot pass vacuously because |
|---|---|---|---|---|---|
| 1 | `profile-record` | control | `profile-missing-key` | FND-01 | a required key absent from the record must be refused |
| 2 | `profile-record` | control | `profile-tier5-tag` | FND-01 | a tier tag outside the vocabulary must be refused |
| 3 | `claim-ledger` | control | `claims-unknown-tier` | FND-02 | a tier outside the evidence-tier vocabulary must be refused |
| 4 | `claim-ledger` | control | `claims-overstated-standing` | FND-02 | a claim above its recorded tier must be refused |
| 5 | `discrepancy-log` | control | `discrepancies-missing-c17` | FND-03 | a row without its conflict-resolution link must be refused |
| 6 | `fixture-manifest` | control | `manifests-digest-mismatch` | FND-04 | a changed recorded digest must be refused |
| 7 | `fixture-manifest` | control | `manifests-recipe-source-digest` | FND-04 | a broken recipe-to-source digest chain must be refused |
| 8 | `provenance-rule` | control | `denylist-hit` | FND-05 | a planted copied constant must be refused by a scan, not by review prose |
| 9 | `cp1610-reference-notes` | control | `cp1610-row-without-cite` | FND-06 | a table row with no source citation must be refused |
| 10 | `time-base-vocabulary` | control | `glossary-bare-clock` | FND-07 | a bare `clock` where a qualified unit is required must be refused |
| 11 | `cpu-opcode-expectations` | fault | `ADD_OVERFLOW` | CPU-01 | removing ADD's overflow flag must fail the ALU comparison |
| 12 | `cpu-opcode-expectations` | fault | `GEN_EXPECT_MUTATION` | CPU-01 | mutating the generated table must fail, so an unread table cannot pass |
| 13 | `cpu-interrupt-model` | fault | `INT_ENTRY_ORDER` | CPU-02 | a reordered entry sequence must be observed by an executed model |
| 14 | `cpu-interrupt-model` | fault | `BUSRQ_PRIORITY` | CPU-02 | an inverted BUSRQ/BUSAK priority must be observed |
| 15 | `cpu-sdbd` | fault | `SDBD_DBD_NOT_CLEARED` | CPU-03 | the DBD flag surviving an SDBD immediate must be compared |
| 16 | `cpu-stack-and-reset` | fault | `PUSH_DIRECTION` | CPU-04 | a reversed stack direction must fail the vector suite |
| 17 | `cpu-bus-trace` | fault | `TCI_STROBE_STAMP` | CPU-05 | a dropped cycle stamp must show in a trace that is read |
| 18 | `memmap-profile-generated` | fault | `GEN_MAP_MUTATION` | MEM-01 | a mutated generated map must fail, so the map is data-driven |
| 19 | `memmap-profile-generated` | control | `memmap-literal-orphan` | MEM-01 | a literal map file with no profile must be refused |
| 20 | `memmap-width-masking` | fault | `MEM_ROM_NO_MASK` | MEM-02 | stopping the 10-bit ROM mask must be width behaviour that is measured |
| 21 | `memmap-width-masking` | fault | `MEM_RAM8_NO_TRUNC` | MEM-02 | stopping the 16-to-8-bit store truncation must be measured |
| 22 | `content-loading` | fault | `LOAD_TABLE_BEFORE_VALIDATE` | MEM-03 | publishing the segment table before validation must be refused |
| 23 | `content-refusal-order` | fault | `CHECK_ORDER_MALFORMED_FIRST` | MEM-04 | the refusal *order* is the requirement; only a mutation of it tests it |
| 24 | `ivsc-import-format` | fault | `IVSC_TRUST_TOTAL_BYTES` | MEM-05 | a container lying about its declared size must be refused |

The general argument, one sentence per mechanism kind: a **fault** cannot pass vacuously
because the gate mutates the implementation at a named site and the suite must observe the
mutation (a survivor fails `faults-verdict`); a **control** cannot pass vacuously because
`tools/check/run-controls.sh` requires the step to exit non-zero *and* to print its pinned
token. `GEN_EXPECT_MUTATION` and `GEN_MAP_MUTATION` are generator **faults** (first column of
`data/expected/faults/cpu.ids` and `memmap.ids`), so they resolve as `fault` and are not
proposed as test ids (RESEARCH Q3.4).

## Resolution: 24 of 24

Verified **before** writing, by rebuilding the step's own `$tmp/known` with its five loops
and requiring `grep -qx -F -e "<mechanism> <id>"` for each row:

```sh
tmp=$(mktemp -d); exp=data/expected; : > "$tmp/known"
for f in "$exp"/faults/*.ids;  do awk '/^#/ {next} NF {print "fault",   $1}' "$f" >> "$tmp/known"; done
for f in "$exp"/controls/*.ids; do awk '/^#/ {next} NF {print "control", $1}' "$f" >> "$tmp/known"; done
for f in "$exp/pkg-selftests.ids" "$exp/ci-summary-selftests.ids"; do
  awk '/^#/ {next} NF {print "mutant", $1}' "$f" >> "$tmp/known"; done
for f in "$exp"/tests/*.ids; do
  awk '/^#/ {next} $1 ~ /^[a-z0-9-]+[.](judge|mutant)[.][^ ]+$/ {print "mutant", $1}' "$f" >> "$tmp/known"; done
# then, per row: grep -qx -F -e "<mech> <id>" "$tmp/known"
```

Result: **24 of 24 resolved, 0 missing** (known set 726 `<mechanism> <id>` lines). No row was
rewritten to make it resolve; nothing needed to be. After the writes the step itself agrees:
`-- important-gates pass gates=107 lines=330`, exit 0.

## Two counting mechanisms, because the two file kinds differ

**The real registry - non-comment rows, not numstat.** `lines=` is the step's count of
non-comment rows, so the contract is over the filtered diff:

| Range | Non-comment rows added | Non-comment rows deleted | `--numstat` (for the record) |
|---|---|---|---|
| `add28f71..081d441` (Task 1) | 10 | 0 | - |
| `add28f71..285807e` (both tasks) | **24** | **0** | 68 / 0 |

The 68 - 24 = 44 difference is the `# Plan 09-04 ...` block, the 17 `# <REQ>: <property>`
blocks and the blank lines separating them. Those are the file's own established convention
(`# API-02: ...` sits at line 14 of the committed file) and `lines=` never counted them. A
numstat figure quoted as the row count would be wrong by 44.

**The ten toy registries - `git diff --numstat`, because every added line is a row.** They
carry no comments beyond a header nobody edited, so additions *are* rows:

| File (non-comment rows after) | `add28f71..081d441` | `add28f71..285807e` |
|---|---|---|
| `_good/tree/...` (44) | 7 / 0 | **17 / 0** |
| `important-gates-bad-req-phase5/overlay` (45) | 7 / 0 | **17 / 0** |
| `important-gates-bad-req-phase6/overlay` (45) | 7 / 0 | **17 / 0** |
| `important-gates-req-uncovered-phase5/overlay` (43) | 7 / 0 | **17 / 0** |
| `important-gates-req-uncovered-phase6/overlay` (43) | 7 / 0 | **17 / 0** |
| `important-gates-req-uncovered-phase7/overlay` (43) | 7 / 0 | **17 / 0** |
| `important-gates-uncovered-gate/overlay` (43) | 7 / 0 | **17 / 0** |
| `important-gates-unknown-id-phase5/overlay` (45) | 7 / 0 | **17 / 0** |
| `important-gates-unknown-id-phase6/overlay` (45) | 7 / 0 | **17 / 0** |
| `important-gates-unknown-id/overlay` (45) | 7 / 0 | **17 / 0** |

Every line was appended after the existing last line (`toy-prf01 fault GOOD_FAULT PRF-01`),
so `important-gates-bad-req-phase5`'s EXPECT `FAIL important-gates-unknown-id: line 19` and
`-phase6`'s `line 24` keep their meaning (pitfall 6). Both print `control ok`.

## The token, predicted and matched exactly

| Token | Before | Predicted | After | |
|---|---|---|---|---|
| `gate` | PASS | PASS | **PASS** | ok |
| `steps` | 38/38 | 38/38 | 38/38 | unchanged - no new gate step (2.1) |
| `controls` | 380/380 | 380/380 | 380/380 | unchanged - no new control |
| `faults` | 235/235 | 235/235 | 235/235 | unchanged - no new fault site |
| `gates` | 90 | **107** | **107** | +17 new gate names |
| `lines` | 306 | **330** | **330** | +24 non-comment rows, -0 |
| `tests-expected` | 4958 | 4958 | 4958 | unchanged - no new test id |
| `tests-executed` | 4954 | 4954 | 4954 | unchanged |
| `tests-skipped` | 4 | 4 | 4 | unchanged |
| `tests-missing` | 0 | 0 | 0 | unchanged |
| `claims` | 28 | 28 | 28 | unchanged |
| `provisional` | 27 | 27 | 27 | unchanged |

Observed triple recorded as an observation and never as a predicate: `files=2519`,
`tracked=2519`, `hygiene-files=2519`, `mode=tree`.

## The retargeting answer: none, and why

```sh
$ grep -c 'INP-04' data/expected/important-gates.ids   -> 0
$ grep -c 'VID-99' data/expected/important-gates.ids   -> 0
```

The two never-valid ids live only in their own overlays (`.../bad-req-phase5/...:19:
toy-input fault GOOD_FAULT INP-04`, `.../bad-req-phase6/...:24: toy-video fault GOOD_FAULT
VID-99`) and are outside both the old and the new accepted sets, so neither became valid.
**No EXPECT line was edited.** Both positional controls still print `control ok`.

## Pricing: measured, and reported as noise

Three alternated `/usr/bin/time -p` runs of the step per tree, filelist built the way the
gate builds it (2519 paths), `ROOT=$PWD CHECK_DIR=$PWD/tools/check`:

| | `real` median | `user` | `sys` | user+sys median | load average |
|---|---|---|---|---|---|
| before (306 rows, 90 gates) | 0.13 | 0.04 | 0.06 | **0.10** | 10.47 |
| after (330 rows, 107 gates) | 0.15 | 0.05 | 0.07 | **0.12** | 14.42 |

Delta **+0.02 local cpu-s**, against the ~12.9 local cpu-s allowance recorded for the phase
(0.16%) and 0.185 per control for the **zero** controls added. The step reads 24 more
registry lines and does nothing else, so the honest statement is the measured one: the
delta is smaller than the load change between the two moments (10.47 -> 14.42) and is not
separable from it. Nothing here is claimed as a saving or a charge.

## Gate lines

| Shell | Line | `-- FAIL` count |
|---|---|---|
| local `sh` | `gate: PASS steps=38/38 controls=380/380 faults=235/235 gates=107 lines=330 goldens=29 reports=30 release-only=2 host-dependent=3 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 claims=28 provisional=27` | 0 |
| local `dash` | identical tail, byte for byte (`diff` of the two final lines is empty) | 0 |
| container | **not run** - see below | - |

Task 1's first `sh` run failed `steps=37/38` on `-- FAIL integrity-non-ascii:
data/expected/important-gates.ids` (em-dashes in my comment prose); fixed in the same task
and re-run green. Task 2's and Task 3's runs were green first time.

## Deviations from Plan

### Approved budget deviation (from the dispatch, recorded as instructed)

The plan lists six `tools/gate.sh` invocations plus a container gate, which does not fit the
1800-second dispatch limit that already stopped three of this phase's executors. The
orchestrator approved and this executor applied:

1. `sh tools/gate.sh` at the end of Task 1 **and** Task 2 (two runs, both green) - the plan
   asked for `sh` and `dash` at each.
2. `dash tools/gate.sh` deferred to Task 3, where both shells ran (two runs, identical
   tokens) - so the plan's `sh`+`dash` pair is satisfied once at the plan's final tree
   rather than twice at intermediate trees.
3. **The container gate was not run at all.** It runs at the wave boundary under the
   orchestrator's control. This SUMMARY's Task 3 verify accepts the `not run` line below in
   place of `gate: PASS` in `build/gate-linux-a2-registry.log`:
   `container gate: not run in this dispatch - deferred to the orchestrator at the wave
   boundary (1800 s dispatch limit; approved budget deviation).`

### Auto-fixed Issues

**1. [Rule 1 - Bug] Em-dashes in the new registry comment prose tripped `integrity-non-ascii`**
- **Found during:** Task 1, first `sh tools/gate.sh` run
- **Issue:** three `# FND-0x:` property comments used ` — ` (U+2014). `-- FAIL
  integrity-non-ascii: data/expected/important-gates.ids` propagated to `-- FAIL
  gate-step-failed: integrity` and `steps=37/38`.
- **Fix:** `LC_ALL=C sed -i '' 's/ — /; /g'` on the three lines; no registry row changed,
  no id changed, the non-comment count stayed at 316.
- **Files modified:** `data/expected/important-gates.ids`
- **Commit:** `081d441` (fixed before the commit, so the commit is clean)
- **Verified:** `LC_ALL=C grep -c '[^ -~]'` prints 0 on the registry, the step and all ten
  toy files; the re-run printed `steps=38/38`.

## Known Stubs

None. No placeholder, no hardcoded empty value and no unwired data source was introduced:
every added line is a registry row whose id resolves in an inventory the gate already runs,
and the only non-row text added is prose about those rows.

## Threat Flags

None. No new network endpoint, auth path, file-access pattern or trust-boundary schema
change: the commit adds registry rows, one accepted-set string and comment prose, and appends
lines to ten existing toy files. T-09-07 and T-09-08 are mitigated as planned - 24 of 24
rows verified before writing, the step's own `important-gates-unknown-id` and
`-uncovered-gate` refusals unchanged and still exercised, and the positional EXPECT tokens
proved intact by their controls passing.

## Self-Check: PASSED

Run after this file was written, on the tree of `285807e`:

| Check | Result |
|---|---|
| `plan-base: add28f71ad7e8625478aa8c5e746b50209a66e6c` parses out of this file | FOUND |
| commits touching `data/expected/important-gates.ids` in `plan-base..HEAD` | 2 (`081d441`, `285807e`) |
| filtered non-comment delta on the real registry | 24 added, 0 deleted |
| `git diff --numstat` per toy registry over `plan-base..HEAD` | 17 / 0 on all ten |
| `row-commit: 93e6b9f` is an ancestor of HEAD | exit 0 |
| registry gate names / non-comment rows | 107 / 330 |
| both task commits present | `081d441`, `285807e` |
| container gate | recorded `not run` with its reason |
