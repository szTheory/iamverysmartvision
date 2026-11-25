---
phase: 10-reachable-runs-and-presentation-policy
plan: 05
subsystem: Phase 10 wave 3 - the two guards that keep a claim from citing a handoff: a handoff-shaped claim artifact is refused, a malformed handoff row is refused, and `looks`/`sounds` join the claim-word list
tags: [EV-05, A-04, handoff, claims-guard, text-guard, T-10-13, T-10-14, T-10-15]
status: complete

requires:
  - phase: 10-reachable-runs-and-presentation-policy
    provides: "plan 10-03's ledger: data/expected/handoffs.ids, five rows, every one `evidence: none`; plan 10-04's end state: steps=38/38 controls=386/386 claims=28 provisional=27 gates=107 lines=330 claim-refs=26"
provides:
  - "tools/check/kinds/claim.cmake: a claim whose artifact cites a handoff artifact, a build/ path, a .wav, a .ppm or a bench .tsv is refused (claim-handoff-artifact), control tools/check/controls/claims-handoff-artifact"
  - "tools/check/steps/claims.sh: a malformed handoff ledger row is refused (claim-handoff-row), reading data/expected/handoffs.ids with an existence guard rather than the single-column inventory reader; control tools/check/controls/claims-handoff-row"
  - "tools/check/steps/text.sh: `looks` and `sounds` are claim words, with control tools/check/controls/text-untagged-look-listen; docs/presentation.md, docs/provenance.md and docs/video/demo-lamp-tone-1-derivation.md retagged where the new words now apply"
affects: [10-06]

actuals:
  tasks: 3
  commits: 3

key-decisions:
  - "Two predicates, not new machinery (A-04, research §Q5/Q6). The claims guard is one refusal after the existing artifact predicate in `kinds/claim.cmake`; the text guard is one word list at `text.sh:45`. Neither adds a gate step, and neither invents a second mechanism for a property that already had one."
  - "The ledger reader guards on existence instead of using `read_ids`. `data/expected/handoffs.ids` carries four tokens per line, so the single-column inventory reader would append whole lines; plan 10-03's SUMMARY flagged exactly this, and the guard is why a missing file means an empty set rather than a malformed inventory."
  - "`looks` and `sounds` only. The research measured that `sound`, `render` and `picture` would cost 21, 17 and 39 claim tags respectively; the two chosen words cost one line of blast radius each, in docs/presentation.md, docs/provenance.md and docs/video/demo-lamp-tone-1-derivation.md, all retagged here."
  - "The third control (`claims-handoff-row`) was not in the plan's predicted `controls=` and is recorded as a deviation with its reason: a guard whose input file has a declared grammar needs a proof that a row outside that grammar is refused, and the plan's two guards would otherwise both be about artifacts and neither about the ledger's own shape."

key-files:
  created:
    - tools/check/controls/claims-handoff-artifact/{CHECK,EXPECT,FORBID,overlay/...}
    - tools/check/controls/claims-handoff-row/{CHECK,EXPECT,overlay/...}
    - tools/check/controls/text-untagged-look-listen/{CHECK,EXPECT,overlay/...}
    - .planning/phases/10-reachable-runs-and-presentation-policy/10-05-SUMMARY.md
  modified:
    - tools/check/kinds/claim.cmake
    - tools/check/steps/claims.sh
    - tools/check/steps/text.sh
    - data/expected/controls/claims.ids
    - docs/presentation.md
    - docs/provenance.md
    - docs/video/demo-lamp-tone-1-derivation.md

requirements-completed: []  # EV-05 closes in plan 10-06 with its exclusion row

completed: 2026-11-24
---

# Phase 10 Plan 05: the two guards Summary

**A claim can no longer cite a handoff. Three controls landed, one more than the
plan predicted and for a measured reason. `tools/check/kinds/claim.cmake` refuses
an artifact that is a handoff artifact, a `build/` path, a `.wav`, a `.ppm` or a
bench `.tsv`; `tools/check/steps/claims.sh` refuses a malformed row in
`data/expected/handoffs.ids`, reading it with an existence guard rather than the
single-column inventory reader the file's four-token grammar would break; and
`tools/check/steps/text.sh` treats `looks` and `sounds` as claim words, with the
three documents the new words now reach retagged in the same commit. No gate step
was added, no claim was minted, and `steps=38/38` and `claims=28` did not move.**

## Commits

| Commit | Subject | Files |
|---|---|---|
| `71c4688` | `feat(10): refuse a handoff-shaped claim artifact` | `kinds/claim.cmake`, `data/expected/controls/claims.ids`, `controls/claims-handoff-artifact/*` |
| `1ae479a` | `feat(10): refuse a malformed handoff row` | `steps/claims.sh`, `data/expected/controls/claims.ids`, `controls/claims-handoff-row/*` |
| `2599d1b` | `feat(10): treat looks and sounds as claim words` | `steps/text.sh`, `data/expected/controls/claims.ids`, `controls/text-untagged-look-listen/*`, three documents retagged |

## Token delta, predicted and measured

| Field | Before | After | Predicted? |
|---|---|---|---|
| `controls` | 386/386 | **389/389** | The plan predicted two guards; the third is the deviation below |
| `steps` | 38/38 | 38/38 | yes |
| `claims` | 28 | 28 | yes |
| `provisional` | 27 | 27 | yes |
| `claim-refs` | 26 | 26 | yes — the retagged lines already carried tags |
| `gates`, `lines` | 107, 330 | 107, 330 | yes |

## The three controls, and what each fails for

```
claims-handoff-artifact   EXPECT "FAIL claim-handoff-artifact: CL-0001" and "CL-0002"
                          FORBID the pre-existing claim-dangling-path token
claims-handoff-row        EXPECT "FAIL claim-handoff-row: line 3"
text-untagged-look-listen EXPECT "FAIL text-claim-untagged: docs/x.md:1" and ":2"
```

Each is a negative control: `run-controls.sh` requires the step to exit non-zero
and to print the pinned token, so a guard that stops refusing fails its own
control (lesson 20).

## Deviations

1. **A third control (`claims-handoff-row`) beyond the plan's two.** The plan's
   `controls=` prediction covered the artifact guard and the text guard. The
   ledger's own grammar needed a proof of its own: without it, both new guards
   would be about artifacts and neither about whether a row can be rewritten into
   a pass unnoticed — which is exactly the repudiation threat `T-10-07` names.
   Recorded rather than absorbed.
2. **Budget:** `sh` per task and one `dash` at the end, as approved; the container
   gate is the orchestrator's at the phase boundary.
3. **The plan was closed out by the orchestrator.** The dispatched executor
   committed all three tasks and was then stopped by the 1800 s dispatch limit
   before writing this SUMMARY. The orchestrator re-ran `sh` and `dash` on the
   final tree (both `gate: PASS`, zero `-- FAIL` lines), verified the three
   controls' `EXPECT`/`CHECK` files, and wrote this record. The substance of all
   three tasks is the executor's.
