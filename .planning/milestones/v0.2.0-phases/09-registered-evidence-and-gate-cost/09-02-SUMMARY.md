---
phase: 09-registered-evidence-and-gate-cost
plan: 02
subsystem: Phase 9 wave A1, plan 2 of 3 - trims (c) and (d): the atomic-refusal family samples its nine digests on every 8th row plus the first and last, and an unsealed byte flip takes the first value only, each with its preserving conditions named and measured
tags: [A1, EV-01, trim-c, trim-d, sampled-digests, byte-flips, T-09-04, T-09-05, cost-measured]
status: complete

requires:
  - phase: 09-registered-evidence-and-gate-cost
    provides: "plan 09-01 end state: gate: PASS steps=38/38 controls=380/380 faults=235/235 gates=90 lines=306 goldens=29 reports=30 release-only=2 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 claims=28; the state suite at 2.76 s dev / 1.61 s release (median of three, load average 13.04 on 18 cpus)"
provides:
  - "tests/state/state_util.h, state_util.c, state_restore.c: trim (c) - the nine digests of state.restore.atomic-every-refusal are computed when the sampled flag is set and r == 0 || r % 8 == 0 || r + 1 == n; the whole-instance comparison of every row (state_util.c:476, outside the digest branch) and the produced accumulation (state_util.c:491, unconditional) are untouched"
  - "tests/state/state_fuzz.c: trim (d) - fz_run_flips skips j == 1 for an unsealed flip; the offset loop bound, fz_pick and masks[] are untouched, so the offset set does not move, and the resealed path keeps both values"
  - "data/expected/tests/state.ids: one comment block per trim naming the sampled rows, the unchanged offset set and the guard sites by file:line; additions only, no id line moved"
affects: [09-03]

actuals:
  tasks: 3
  commits: 3
plan_head_before: 93edc18
plan_head_after: 0a91804

key-decisions:
  - "Trim (c) is a second field, not a replacement: the sampled mode is set only for the atomic-refusal family, so every other case of the state suite keeps its every-row digest comparison. The two things the trim does NOT touch are named by file:line in the code comment and in the inventory comment, because a trim inside a test loop is one keystroke from a coverage deletion and the guards are the only thing that tells them apart (T-09-04)."
  - "Trim (d)'s 'the flip moves the digest' guard is not a separate mechanism in this file, and none was invented. The property is enforced operationally by the acceptance refusal (c.accepted != 0u, state_fuzz.c:945-948): with the trailer left as it was, a payload byte the digest does not cover would be accepted and the id would fail. Recording that plainly is better than adding a hollow guard to satisfy a plan sentence (lesson 20)."
  - "The measured saving is smaller than the recorded estimate and is recorded as measured: 0.50 local cpu-s against the 0.8 the cost rule predicted for (c)+(d) together. The estimate was not adjusted after the fact."

key-files:
  created:
    - .planning/phases/09-registered-evidence-and-gate-cost/09-02-SUMMARY.md
  modified:
    - tests/state/state_util.h
    - tests/state/state_util.c
    - tests/state/state_restore.c
    - tests/state/state_fuzz.c
    - data/expected/tests/state.ids

requirements-completed: []  # EV-01 closes on 09-03's measurement, not here

completed: 2026-11-24
---

# Phase 9 Plan 02: trims (c) and (d) Summary

**Two cost trims landed inside the state suite, each with its preserving
condition named in the code and in the inventory. Trim (c) makes
`state.restore.atomic-every-refusal` compute its nine digests when a sampled flag
is set and `r == 0 || r % 8 == 0 || r + 1 == n` — every 8th refusal plus the
first and the last row — while the whole-instance comparison of **every** row
(`tests/state/state_util.c:476`, outside the digest branch) and the `produced`
accumulation (`:491`, unconditional) are untouched; the sampled mode is a second
field, so every other case of the family keeps its every-row comparison. Trim (d)
makes `fz_run_flips` skip the second value (`XOR 0x01`) of an **unsealed** flip
while the resealed path keeps both; the offset loop bound, `fz_pick` and
`masks[]` are untouched, so the offset set does not move, and the property that a
flip moves the digest is enforced operationally by the acceptance refusal at
`tests/state/state_fuzz.c:945-948` rather than by a guard invented to satisfy a
plan sentence. No id moved: `tests-expected=4958`, `tests-executed=4954`,
`tests-skipped=4` before and after, and `faults=235/235 fault-runs=235` is
unchanged, which is the evidence that no fault became uncaught. `sh`, `dash` and
the Ubuntu container all read `gate: PASS` with the predicted token.**

## Commits

| Commit | Subject | Files |
|---|---|---|
| `0c217ed` | `fix(09): sample the nine digests on the atomic-refusal rows and keep both guards` | `tests/state/state_util.h`, `state_util.c`, `state_restore.c` |
| `d7f5ec4` | `fix(09): take the first value only of each unsealed byte flip` | `tests/state/state_fuzz.c` |
| `0a91804` | `docs(09): record the two state trims where a reader looks for them` | `data/expected/tests/state.ids` |

## Measured cost — three alternated runs per preset, both trees

Built at `93edc18` (pre-trim) in a throwaway worktree with its own build
directory, then at `0a91804` (post-trim) in the phase tree. `/usr/bin/time -p`,
`ivs_tests_state` run directly so the figure is the suite and not the harness.

| Preset | pre-trim (median of 3) | post-trim (median of 3) | saved |
|---|---|---|---|
| dev | 2.76 s (2.75 / 2.76 / 2.76) | 2.46 s (2.46 / 2.46 / 2.44) | **0.30 s** |
| release | 1.61 s (1.61 / 1.61 / 1.62) | 1.41 s (1.41 / 1.41 / 1.42) | **0.20 s** |

**Total 0.50 local cpu-s**, against the **0.8** the cost rule predicted for
(c)+(d) together. The estimate is recorded as it was written and is not adjusted
after the fact: the measured figure is smaller, and that is the figure that
counts. Load average at measurement time **13.04 on 18 cpus** — the laptop is
loaded, so the absolute times read high; the before/after pair was taken
alternately on the same host within one session, which is what makes the
difference meaningful and the absolute values not.

The state suite runs once per preset inside the gate (`ctest-runs=2`), so the
gate's own saving is the 0.50 s above.

## The gate, re-measured on the final tree

```
sh   tools/gate.sh : gate: PASS steps=38/38 controls=380/380 faults=235/235
                     fault-runs=235 gates=90 lines=306 goldens=29 reports=30
                     release-only=2 tests-expected=4958 tests-executed=4954
                     tests-skipped=4 tests-missing=0 claims=28 provisional=27
dash tools/gate.sh : gate: PASS, the same absolute-and-frozen tokens
sh   tools/gate-linux.sh : gate-linux: exit 0, gate: PASS in build/gate-linux.log
                     (run at the wave boundary, log copied before anything else
                     ran, no ivs- container left behind)
```

Zero `-- FAIL` lines in either shell's log. `files=`, `tracked=`,
`hygiene-files=`, `ci-runs=` and the timing lines are derived at run time and are
recorded here as observations, never asserted.

## Deviations

1. **The plan was closed out by the orchestrator.** The dispatched executor was
   stopped by its 1800 s dispatch limit after committing trim (c), leaving trim
   (d) and the inventory comments in the working tree. Per the workflow's
   completion-reconciliation path, the orchestrator inspected the work, verified
   it (both presets' state suite green, `state.fuzz.byte-flips` and
   `-resealed` `ok` on both), committed the two remaining tasks atomically and
   wrote this SUMMARY. The substance of all three tasks is the executor's; the
   commits are `d7f5ec4` and `0a91804`. Recorded because authorship of a commit
   is evidence, not bookkeeping.
2. **Trim (d)'s plan sentence asked for a guard that does not exist.** No
   mechanism named "the flip moves the digest" is in the tree. Rather than invent
   one, the property is documented as enforced by the acceptance refusal and the
   file comment says so (lesson 20: a check that cannot fail proves nothing).
3. **The measured saving (0.50 local cpu-s) is below the recorded estimate
   (0.8).** Both figures are recorded; the estimate is not rewritten.

## Carried forward to plan 09-03

The alternated `/usr/bin/time -p` before/after capture for **plan 09-01's lever
and trim (a)** was not taken: that executor was stopped by the same limit before
it could run it. Plan 09-03 is the plan that consolidates the wave A1
measurement, so the lever's before/after table is its first task. What 09-01
recorded instead is measured rather than assumed: the per-launch figures the
0.12 ceiling was computed from, the post-fold suite times (21.55 s dev / 22.69 s
release), and `reports=32 → 30` with `release-only=2` as the structural proof the
fold took effect.
