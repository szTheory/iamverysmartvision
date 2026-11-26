---
phase: 17-records-cost-window-and-the-count-gate-contract
plan: 02
subsystem: records-cost-window
tags: [flatwrap-refusal-tokens, source-archive-letter, flatwrap-shim-join, ev-08-flip, count-gate]

requires:
  - phase: 17-records-cost-window-and-the-count-gate-contract
    plan: 01
    provides: "the recomputed window, the 0.11 priced line and the count-gate contract this plan's records answer"
  - phase: 14
    provides: "the eight append-only refusal tokens and the `pack.flatwrap.neg.<token>` cases that carry each trigger and its accepted boundary"
  - phase: 15
    provides: "the build-only host shim and the fake frontend, arguments six, seven and eight of the runner-process suite"

provides:
  - "the `## Wrap tool refusals` section of `docs/formats/ivsc-v1.md`: eight line-initial tokens, each with its trigger and the boundary that must still be accepted, mechanically identical to the append-only inventory"
  - "the canonical source-archive sentence in both live documents, with the superseded v0.3.0 sentence quoted and dated rather than deleted"
  - "one committed id, `runner-process.judge.flatwrap-shim`, with five limbs and a committed must-fail stand-in judged by the same comparison"
  - "the `cart-role` registry row, the Phase 17 phase-table block, the EV-08 flip and the thirteen toy lines, in one commit"

affects:
  - "17-03: the count-gate row and lesson 32 land on top of the same registry and the same flip"
  - "Phase 20: recomputes the guardrails' close line, whose inspection limb is deferred here because the tool it names does not exist until Phase 18"

actuals:
  tokens: 0
  tasks: 3
  commits: 0

tech-stack:
  added: []
  patterns:
    - "a document section checked by a `diff` of two `grep '^token-'` outputs, with the stand-in that renames one existing line proving the check can fail"
    - "one canonical sentence used verbatim in three places, counted in each file, with the superseded sentence preserved as a dated quote"
    - "a cross-process digest join whose must-fail stand-in is committed code in the same case, judged by the same function, so the demonstration repeats on every gate run"

key-files:
  created:
    - .planning/phases/17-records-cost-window-and-the-count-gate-contract/17-02-SUMMARY.md
  modified: []

key-decisions: []

requirements-completed: []

# Metrics
duration: 0min
completed: 2026-11-26
status: in-progress
---

# Phase 17 Plan 02: The Refusal Tokens, the Source-Archive Letter and the Join Id Summary

**In progress: the plan's baselines are being measured at its own start commit before the first edit.**

## Baselines

start-commit: 67e35a600f9a7e965dfd7c6a76d95c075343b1c2

Measured at that sha, before the first edit. Nothing here is carried forward from 17-01's record: every figure is
re-measured at this plan's own start commit and the plan's prediction is compared against the measurement rather
than restated.

```
start-commit: 67e35a600f9a7e965dfd7c6a76d95c075343b1c2
baseline: gate: PASS steps=38/38 controls=404/404 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=238 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=118 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=29 provisional=28 handoffs=8 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=695 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=238/238 tracked=2808 binaries-listed=0 rights-pinned=4/4 terms=9 doc-files=63 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2808 history-commits=1 gates=119 lines=364 input-rows=31 input-policies=11 input-profile-fields=31 files=2808 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=20/20 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=12/12 runner-files=18 cases=14 sources=13 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=5042 tests-executed=5038 tests-skipped=4 tests-missing=0 docs=64 claim-refs=27 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
baseline-registry-rows: 364
baseline-runner-process-ids: 149
baseline-tracked-files: 2807
```

The gate line above was produced by `sh tools/gate.sh` at that sha, before the first edit, and the other three
figures are the `awk '!/^#/ && NF'` row counts over the two named inventories and the `git ls-files` count at the
same commit. The plan's prediction was `lines=364` and `tests-expected=5042`; both matched, so no figure needed
correcting and none was corrected. `baseline-tracked-files` is `git ls-files` (2807) rather than the gate's
`tracked=2808` token: the gate counts the work tree, which at that moment already held this untracked record.

<!-- gsd:write-continue -->
