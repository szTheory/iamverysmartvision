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
  tokens: 0
  tasks: 2
  commits: 0

tech-stack:
  added: []
  patterns:
    - "a lesson whose every clause names its own mechanism or states that it has none, so a reader cannot quote a clause as if a gate enforced it"
    - "a registry row that points at a mechanism already committed rather than minting a case, with the stand-in re-run on every gate run instead of once"

key-files:
  created:
    - .planning/phases/17-records-cost-window-and-the-count-gate-contract/17-03-SUMMARY.md
  modified: []

key-decisions: []

requirements-completed:
  - EV-08

# Metrics
duration: 0min
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
`tests-executed=5040`; all four matched, so no figure needed correcting and none was corrected.
