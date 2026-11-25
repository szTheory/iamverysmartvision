---
phase: 09-registered-evidence-and-gate-cost
plan: 03
subsystem: Phase 9 wave A1, plan 3 of 3 - the wave A1 cost record (every figure recomputed from the committed rows or labelled as the recorded figure it is) and the wave exit evidence on the exact tree the orchestrator will push
tags: [A1, EV-01, A-07, cost-record, window-recomputed, allowance, lever-ceiling, trim-b-declined, exit-evidence, T-09-06]
status: complete

requires:
  - phase: 09-registered-evidence-and-gate-cost
    provides: "plan 09-02 end state at cc71552: gate: PASS steps=38/38 controls=380/380 faults=235/235 fault-runs=235 gates=90 lines=306 goldens=29 reports=30 release-only=2 host-dependent=3 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 claims=28 provisional=27; the lever and trims (a), (c), (d) landed and measured; one open item handed forward (the alternated before/after for the lever and trim (a))"
provides:
  - "`.planning/CI-EFFICIENCY.md`: the wave A1 entry - the last-five window recomputed from the committed rows (7.4167, 7.5333, 7.6957, 7.8261, 8.3810; median 7.696) with the arithmetic command printed beside it, the allowance kept as priced (about 21 hosted cpu-s) with its recomputation (27.8 hosted, 0.604 under the 8.3 stop line) beside it rather than instead of it, the lever's 0.12 local cpu-s as a ceiling that cannot move `r`, trims (a) 0.18 and (c)+(d) 0.50 measured with their load conditions, the wave total 0.80 local cpu-s = 0.029 `r`, trim (b) declined in writing, the unchanged token, and the seam"
  - "the A1 commit list below, which is the input to the orchestrator's `git merge-base --is-ancestor <row-commit> <this-commit>` ancestry proof"
  - "three gate lines on the tree of `e8a740c`: local `sh`, local `dash` and the Ubuntu container, all `gate: PASS` with the predicted absolute-and-frozen token and zero `-- FAIL` lines"
affects: [09-04, 09-05, 09-06, 09-07, 09-08, 09-09]

actuals:
  tokens: 6642   # chars/4 over the realized diff: 8100 chars in .planning/CI-EFFICIENCY.md + 18471 in this SUMMARY
  tasks: 2
  commits: 2
plan_head_before: cc71552
plan_head_after: e8a740c

key-decisions:
  - "The window is recomputed, never quoted (A-07): `awk -F'\\t' 'NR>1 && $13>0 {print $13/$8}' .planning/ci-runs.tsv | tail -5 | sort -n` prints 7.41667, 7.53333, 7.69565, 7.82609, 8.38095, whose median is 7.6957 -> **7.696**. The entry prints that value and the command that produced it. It happens to agree with the 7.696 the previous entry recorded, and the agreement is now a check rather than a coincidence."
  - "The allowance is recorded as it was priced AND recomputed beside it, not replaced: about 21 hosted cpu-s (about 12.9 local at the recorded 1.675 factor) is what these plans were budgeted against; (8.3 - 7.696) x 46 = 27.8 hosted cpu-s (16.6 local) is what the window now says. Both are printed, because a reader who finds only the larger number would think the phase had more room than it was planned with, and a reader who finds only the smaller would think the arithmetic had not been done."
  - "The lever stays labelled a ceiling: 0.12 local cpu-s = 0.0044 `r`, which cannot move the metric. It is landed because it is recorded policy and the last non-owner lever in the pre-committed order, and the entry says exactly that in those terms (T-09-06)."
  - "The alternated `/usr/bin/time -p` before/after for the lever and trim (a) is recorded as an OPEN ITEM with its reason (two executors stopped by the 1800 s dispatch limit), not closed by a figure that was not taken and not dropped silently. What stands in its place is measured: the per-launch medians of five the 0.12 ceiling is arithmetic over, the post-fold `runner-process` suite times (21.55 s dev / 22.69 s release), and the structural proof `reports=32 -> 30` with `release-only=2`."
  - "No checkpoint line was written, by design: `discharged-below-budget`, `discharged-within-line` and `lever-required` are the orchestrator's words after the hosted row, and the entry says so in a sentence so a later reader does not go looking for a `phaseA1:` line here."
  - "The entry's load sentence was corrected before the exit gates ran, and the correction is why the local gates were run twice: the first version named the plan-start loads (7.89 / 13.93 / 17.98) as if they covered every run. It now names the baseline moment and points at this SUMMARY for each later run's load, because a single number for runs an hour apart would be a guess dressed as a measurement. The amend is on an unpushed commit of this plan; the gates recorded below ran on the corrected content."

key-files:
  created:
    - .planning/phases/09-registered-evidence-and-gate-cost/09-03-SUMMARY.md
  modified:
    - .planning/CI-EFFICIENCY.md

requirements-completed: []  # EV-01's cost commits and record close here; its "hosted row recorded after them" clause is settled by the orchestrator, so the requirement is not checked off by this plan

completed: 2026-11-24
---

# Phase 9 Plan 03: the wave A1 cost record and the exit evidence Summary

**Wave A1 is finished as a cost wave: measured, recorded, and green on all three
gate shapes with the token the phase was predicted to leave untouched. The
last-five window was recomputed from the committed `.planning/ci-runs.tsv` in
the commit that writes it — 7.4167, 7.5333, 7.6957, 7.8261, 8.3810, median
**7.696**, 0.604 under the 8.3 stop line — and the allowance the phase was
priced against (about 21 hosted cpu-s ≈ 12.9 local) is printed with its
recomputation beside it rather than replaced by it. The lever's 0.12 local cpu-s
is quoted as a **ceiling that cannot move `r`**, trims (a) 0.18 and (c)+(d) 0.50
are the measured deltas of 09-01 and 09-02 with their load conditions, the wave
total is **0.80 local cpu-s ≈ 1.34 hosted ≈ 0.029 `r`**, and trim (b) is
**declined in writing** with its 0.09 local cpu-s measurement. No test id, no
control, no fault, no gate step and no registry line: `steps=38/38
controls=380/380 faults=235/235 gates=90 lines=306 tests-expected=4958
tests-executed=4954 tests-skipped=4`, byte-identical to the plan-start baseline
under `sh`, `dash` and the Ubuntu container.**

## Commits

| Commit | Subject | Files |
|---|---|---|
| `e8a740c` | `docs(09): record wave A1's cost, recomputed and with trim (b) declined` | `.planning/CI-EFFICIENCY.md` |
| (this SUMMARY's commit) | `docs(09): plan 09-03 summary — wave A1's cost record and the exit evidence` | `.planning/phases/09-registered-evidence-and-gate-cost/09-03-SUMMARY.md` |

`git diff --numstat cc71552..e8a740c` reads `49 0 .planning/CI-EFFICIENCY.md`:
additions only, one file. The entry is appended, never rewritten.

## Every A1 commit, for the row's ancestry proof

The orchestrator's proof is `git merge-base --is-ancestor <row-commit> <this-commit>`.
The commits it runs over, base `912ab9a` (the state Phase 9 execution started
from) to this SUMMARY's commit:

```
8d28dce fix(09): fold the two lamp-tone variants into release-only rows with a derivation judgement
dcb50f4 fix(09): judge the three round-trip refusals against the parser's guard sites
f271769 fix(09): make the mode of every runner-process run visible with a count
93edc18 docs(09): plan 09-01 summary — the lever and trim (a), measured
0c217ed fix(09): sample the nine digests on the atomic-refusal rows and keep both guards
d7f5ec4 fix(09): take the first value only of each unsealed byte flip
0a91804 docs(09): record the two state trims where a reader looks for them
cc71552 docs(09): plan 09-02 summary — trims (c) and (d), measured at 0.50 local cpu-s
e8a740c docs(09): record wave A1's cost, recomputed and with trim (b) declined
```

Wave A1 added three files to the tree's tracked set — the three SUMMARYs, this
one last — and no path under `core/`, `runner/`, `data/golden/` or `docs/`.

## The gate, on the tree of `e8a740c`

The three last lines, verbatim, labelled. Both local shells were run twice: the
first pair ran before the load-sentence correction described in the decisions,
and the lines below are the second pair, taken on the content that is committed.

**local `sh`** (`gate-time total=75`, `gate-cpu user=137.36 sys=115.25`, 1-minute
load average 12.76 at start):

```
gate: PASS steps=38/38 controls=380/380 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=235 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=77 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=28 provisional=27 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=665 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=235/235 tracked=2518 binaries-listed=0 terms=9 doc-files=59 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2518 history-commits=1 gates=90 lines=306 input-rows=31 input-policies=11 input-profile-fields=31 files=2518 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=19/19 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=11/11 runner-files=16 cases=8 sources=12 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 docs=60 claim-refs=26 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

**local `dash`** (`gate-time total=78`, `gate-cpu user=142.61 sys=122.08`, 1-minute
load average 16.93 at start): the last line is the `sh` line above, character for
character, with no token differing. Zero `-- FAIL` lines in either log.

**container** (`sh tools/gate-linux.sh`, `gate-linux: exit 0`, `gate-time
total=311`, `gate-cpu user=1202.31 sys=76.32`, `gate-env ncpu=8
cc=GNU-13.3.0`, 1-minute load average 15.09 at start; log copied to
`build/gate-linux-a1.log` before anything else ran; no `ivs-` container left
behind):

```
gate: PASS steps=38/38 controls=380/380 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=GNU c-presets=2 fault-runs=235 core-imports=5 purity-selftest=7/7 ctest-runs=2 ci-runs=77 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=28 provisional=27 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=665 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=235/235 tracked=2518 binaries-listed=0 terms=9 doc-files=59 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2518 history-commits=1 gates=90 lines=306 input-rows=31 input-policies=11 input-profile-fields=31 files=2518 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=sha256 public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=19/19 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=11/11 runner-files=16 cases=8 sources=12 refs-verified=0/4 state-rows=88 state-cuts=18 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 docs=60 claim-refs=26 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

## Token comparison against the plan-start baseline

The baseline was re-measured with `sh tools/gate.sh` before the first edit, as
the plan requires. It read the predicted token, so the comparison is a
comparison of one column with itself — which is the point of the check.

| token | baseline (`sh`, plan start) | local `sh` | local `dash` | container | verdict |
|---|---|---|---|---|---|
| `steps` | 38/38 | 38/38 | 38/38 | 38/38 | unmoved |
| `controls` | 380/380 | 380/380 | 380/380 | 380/380 | unmoved |
| `faults` / `fault-runs` | 235/235, 235 | same | same | same | unmoved |
| `gates` | 90 | 90 | 90 | 90 | unmoved |
| `lines` | 306 | 306 | 306 | 306 | unmoved |
| `goldens` | 29 | 29 | 29 | 29 | unmoved |
| `reports` | 30 | 30 | 30 | 30 | unmoved (09-01's predicted 32 → 30) |
| `release-only` | 2 | 2 | 2 | 2 | unmoved (09-01's predicted 0 → 2) |
| `host-dependent` | 3 | 3 | 3 | 3 | unmoved |
| `tests-expected` | 4958 | 4958 | 4958 | 4958 | unmoved |
| `tests-executed` | 4954 | 4954 | 4954 | 4954 | unmoved |
| `tests-skipped` | 4 | 4 | 4 | 4 | unmoved |
| `tests-missing` | 0 | 0 | 0 | 0 | unmoved |
| `claims` / `provisional` | 28 / 27 | same | same | same | unmoved |

**Nothing in the absolute-and-frozen set differs.** Zero `-- FAIL` lines in all
three logs.

**Derived at run time, recorded as observations and never asserted** (A-05): the
same values in all four runs — `files=2518`, `tracked=2518`,
`hygiene-files=2518` (this SUMMARY is the only addition and it lands after these
runs), `ci-runs=77` (the orchestrator appends the row this plan's boundary
awaits), `history-commits=1`, `mode=tree`. Timing: `gate-time total` 72 at the
baseline, 75 (`sh`) and 78 (`dash`) after it; `gate-cpu` user/sys 125.88/105.04,
137.36/115.25 and 142.61/122.08 — the host was loaded throughout (see the loads
above), so the rise is contention, not work.

**The four container-only differences are the known host-dependent set, not
defects:** `cc=GNU` (vs `AppleClang`), `core-imports=5` (vs 7), `archive=sha256`
(vs `members`) and `refs-verified=0/4` (vs 3/4) — four fields behind the three
`host-dependent` items, each recorded since Phase 6. The container's
`gate-time total=311` is not comparable with the local 75: eight cpus, GNU
toolchain, different container cache state.

## The measured figures this SUMMARY consolidates

Recomputed in the same commit as the entry, from `.planning/ci-runs.tsv`:

| run | head | event | `build_s` | `cpu_s` | `r` |
|-----|------|-------|-----------|---------|-----|
| 45334963892 | 5b6771a | `push` | 46 | 360 | 7.8261 |
| 45412585701 | bd6f2e3 | `pull_request` | 48 | 356 | 7.4167 |
| 45413049708 | 3808e6f | `push` | 46 | 354 | 7.6957 |
| 45414715588 | aa96e9a | `push` | 45 | 339 | 7.5333 |
| 45415977805 | 1090500 | `push` | 42 | 352 | 8.3810 |

Median **7.696** (7.6957 to four decimals) — the same string the entry prints,
which is what Task 1's acceptance criterion asks for. 0.104 under the 7.8
budget, 0.604 under the 8.3 stop line.

| item | measured saving | where from |
|---|---|---|
| lever (3), two dev launches folded | **0.12 local cpu-s** — a ceiling, 0.0044 `r`, cannot move the metric | 09-01: per-launch medians of five, dev 62 ms, release 9.4–9.8 ms |
| trim (a), three refusal launches made static | **0.18 local cpu-s** (about 0.06 each) | 09-01: per-launch medians of five |
| trims (c)+(d), state suite | **0.50 local cpu-s** (dev 2.76 → 2.46 s, release 1.61 → 1.41 s) | 09-02: three alternated runs per preset, load 13.04 / 18 cpus |
| **wave total** | **0.80 local cpu-s ≈ 1.34 hosted ≈ 0.029 `r`** | arithmetic, factor 1.675, `build_s` 46 |
| trim (b), 65-cut loop | **declined** — 0.09 local cpu-s, 0.03 % of a 219 cpu-s gate | recorded measurement, entry "next-milestone opening" |

The lever and trim (a) rows carry **no alternated before/after capture**: that is
the open item 09-01 handed forward, and it is recorded as open in the entry with
its reason rather than closed by a figure that was not taken.

## What this plan does not do

It does not write a checkpoint line — `discharged-below-budget`,
`discharged-within-line` and `lever-required` belong to the orchestrator after
the hosted row, and no executor here calls `tools/ci-record.sh`, writes or
stages `.planning/ci-runs.tsv`, pushes or calls `gh`. It does not add a test id,
a control, a fault, a gate step or a registry line. It does not touch a source
file: `git diff --numstat cc71552..e8a740c` lists one `.planning/` file.

## Deviations

1. **The pre-modified set at spawn was not the set the plan predicted.** The
   plan's Task 2 verify expects `git status --porcelain` to hold only
   `.planning/config.json` and `.planning/state.json`. At spawn it held
   `.planning/ROADMAP.md`, `.planning/STATE.md` and `.planning/state.json`
   (the orchestrator's state sync, uncommitted); `config.json` was clean. The
   check was run as written and its intent was verified by inspection: the only
   dirty paths are those three pre-existing planning records, nothing else. None
   of them was staged by this plan; `config.json` and `state.json` were never
   touched.
2. **The local gates were run twice.** The first pair ran on `e8a740c` before a
   one-sentence load-condition correction; the correction was amended into the
   unpushed commit and the gates re-run so that the recorded lines belong to the
   exact committed content. Cost: two gate runs, about 150 s. Recorded because a
   gate line that belongs to a tree nobody committed is not evidence.
3. **The container's `refs-verified` reads 0/4 and its `archive` mode differs.**
   Both are inside the recorded host-dependent set, and both were already 0/4 and
   `sha256` in the 09-01 and 09-02 container runs. Recorded as an observation,
   not re-diagnosed here (scope boundary).

## Carried forward to the orchestrator

- Push the A1 commits, take the hosted row with `tools/ci-record.sh`, and write
  **exactly one** checkpoint line: `phaseA1: discharged-below-budget` (≤ 7.8),
  `discharged-within-line` (7.8–8.3) or `lever-required` (> 8.3).
- Prove the row's ancestry with `git merge-base --is-ancestor <row-commit> <this-commit>`.
- `.planning/ROADMAP.md`, `.planning/STATE.md` and `.planning/state.json` were
  left dirty by this plan exactly as they were found, with this plan's state
  updates applied; the metadata commit is the orchestrator's.
- Wave A2 may not begin until that row exists.
- **EV-01 is not checked off here, deliberately.** Its text requires a hosted row
  recorded *after* the cost commits to settle the level. Everything this side of
  that row is done — the lever and trims (a), (c), (d) in cost-only commits,
  trim (b) declined by measurement, and the record written — but checking the
  requirement off would assert a row that does not exist yet. The orchestrator
  closes it with the row.

## Self-Check: PASSED

- `.planning/CI-EFFICIENCY.md` carries the wave A1 entry with the recomputed median `7.696` — FOUND
- commit `e8a740c` (Task 1) is an ancestor of HEAD — FOUND
- the SUMMARY's own commit is HEAD (its hash is therefore not written here, since naming it would falsify itself on the next amend) — FOUND
- `build/gate-linux-a1.log` (the container log copy) — FOUND
- the three gate last lines quote `gate: PASS`, `steps=38/38`, `tests-skipped=4`, `reports=30` — VERIFIED
