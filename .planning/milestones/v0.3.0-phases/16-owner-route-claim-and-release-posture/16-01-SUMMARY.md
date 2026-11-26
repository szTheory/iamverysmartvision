---
phase: 16-owner-route-claim-and-release-posture
plan: 01
subsystem: gate-cost
tags: [gate, concurrency, wall-time, build-lock, byte-identical-evidence, cost-only]

requires:
  - phase: 09-registered-evidence-and-gate-cost
    provides: the preflight/producer partition, the per-step output buffer (`step.out.<name>`) and the sorted replay the overlap depends on
  - phase: 15-build-only-host-and-fake-frontend-harness
    provides: the last token line before this plan (`tracked=2783`, `steps=38/38`), unchanged here

provides:
  - "`c-tests` and `c-faults` run overlapped in `tools/gate.sh`, with `gate-overlap pair=c-faults+c-tests` printed before any step runs"
  - "one byte-identical inline `mkdir` build lock in both step scripts, so the two builds of `build/dev` are serialised without adding a tracked file"
  - "the measured before/after figures, the subtraction, the byte-identical proof and the negatives in `.planning/CI-EFFICIENCY.md`"

affects:
  - "16-05 (phase close): the checkpoint verdict is arithmetic over `.planning/ci-runs.tsv`; this plan records numbers only and claims no credit"
  - "the first hosted row after the push: the entry's ~144 s prediction is checked against it"

actuals:
  tokens: 5022
  tasks: 2
  commits: 2
  plan_head_before: da38d7dafb4b44d02a35d94e717acf51280174ff
  plan_head_after: e9ac998

tech-stack:
  added: []
  patterns:
    - "a declared step pair: background member, foreground member, partner joined on the foreground member's normal AND failing paths, killed by the EXIT trap"
    - "a bounded polled `mkdir` lock that lives in the per-run results directory, so no stale-lock takeover rule is needed"
    - "a justified byte-identical copy of a shell block between `# build-lock begin`/`# build-lock end` markers, because a shared helper file would move the token line"

key-files:
  created: []
  modified:
    - tools/gate.sh
    - tools/check/steps/c-tests.sh
    - tools/check/steps/c-faults.sh
    - .planning/CI-EFFICIENCY.md

key-decisions:
  - "The pair's earlier member in `steps.have` order (`c-faults`) goes to the background and the later one (`c-tests`) runs in the foreground, so the join point is a step the loop already reaches; a safety-net join after the producer loop covers a reordered declaration."
  - "The build lock is inline in both scripts and byte-identical between its own markers, never a shared helper: a new tracked file would move `tracked`/`files`/`hygiene-files` and destroy the byte-identical summary line this commit exists to show."
  - "The lock is released from the callers' existing `EXIT`/`HUP`/`INT`/`TERM` traps rather than by traps installed inside the compared block, so the block stays identical and neither script loses its own temp-dir and background-job cleanup."
  - "The entry records the frozen 164 → 115 s estimate as optimistic rather than re-blessing it: an overlap absorbs the smaller member, so the arithmetic predicts about 144 s hosted."
  - "PKG-06 is this plan's requirement but its `Complete` tick belongs to 16-04 T2 per the phase's artifact table, so `requirements-completed` is empty here and no requirement is ticked early."

patterns-established:
  - "Cost-only lever proof: two kept logs, a `cmp` of the 1,509-byte summary line, a `cmp` of each `step.out.<name>` block with a liveness guard, and a `cmp` of the whole logs with only the measurement lines filtered out."
  - "A serialised shared build tree: one bounded polled `mkdir` lock per step, whose necessity is quoted as a measurement (`core/libivs_core.a`), not an assumption."

requirements-completed: []

coverage:
  - id: D1
    description: "`c-tests` and `c-faults` overlap in `tools/gate.sh`, with the measurement line `gate-overlap pair=c-faults+c-tests` and unchanged replay/token/summary construction"
    requirement: PKG-06
    verification:
      - kind: integration
        ref: "sh tools/gate.sh; grep -x 'gate-overlap pair=c-faults+c-tests'; cmp of the two `tail -1` files at 1,509 bytes; cmp of the extracted c-tests and c-faults blocks; cmp of the measurement-filtered whole logs"
        status: pass
    human_judgment: false
  - id: D2
    description: "one inline `mkdir` build lock in both step scripts, byte-identical between `# build-lock begin`/`# build-lock end`, released on success, failure and the caller's traps"
    requirement: PKG-06
    verification:
      - kind: integration
        ref: "sed -n '/^# build-lock begin$/,/^# build-lock end$/p' on both scripts | cmp (33 lines, no normalisation)"
        status: pass
    human_judgment: false
  - id: D3
    description: "`.planning/CI-EFFICIENCY.md` carries the lever entry: pre/post totals, the subtraction, both `cpu` sums, the byte-identical sentence, the lock's measured reason, and the no-credit/no-verdict negatives"
    requirement: PKG-06
    verification:
      - kind: other
        ref: "grep set over the appended entry plus `git diff --numstat` column 2 == 0 (append-only)"
        status: pass
    human_judgment: false

duration: 14min
completed: 2026-11-26
status: complete
---

# Phase 16 Plan 01: The Concurrency Lever, Taken Once and Cost-Only

**`c-tests` and `c-faults` now run at the same time behind one inline build lock, and the summary token line is byte-identical to the line before the change — the pair gave back 13 s of wall (gate 79 s → 72 s) with `cpu` inside its noise band and no token moved.**

## Performance

- **Duration:** 14 min
- **Started:** 2026-11-26T09:01:24Z
- **Completed:** 2026-11-26T09:15:05Z
- **Tasks:** 2 of 2
- **Files modified:** 4 (no file created, no file deleted)

## Accomplishments

- `tools/gate.sh` declares the pair once (`overlap_bg=c-faults`, `overlap_fg=c-tests`), starts the
  earlier member in the background with the same redirection the foreground path uses, runs the
  later member in the foreground exactly as before, joins the partner immediately after it — on the
  normal path *and* when it fails — writing `rc.<name>` and `t.<name>` as the sequential loop did,
  kills a still-running partner from the `EXIT` trap, and prints
  `gate-overlap pair=c-faults+c-tests` before any step runs. The pair is also checked to be two
  producers (`bad gate-overlap-pair` if not), so a renamed step fails rather than silently reverting
  to the serial chain. The replay, `pass-line.sh`, the token accumulation and the summary
  construction are untouched.
- Both step scripts take one bounded, polled `mkdir` lock (`<results>/.build-lock`, 0.01 s poll,
  60,000 polls ≈ 10 min) around **every** `cmake --build` call, released on the success path, the
  failure path and the caller's `EXIT`/`HUP`/`INT`/`TERM` traps; on expiry the step reports its own
  existing token with the `build-lock` detail (`c-tests-failed: build-lock (<preset>)`,
  `c-faults-run: build lock`) and exits non-zero. The lock lives in the per-run results directory
  that `gate.sh` recreates, so no stale-lock takeover rule is needed.
- `.planning/CI-EFFICIENCY.md` gained one append-only entry (79 inserted lines, 0 deleted) with the
  before/after figures, the subtraction, both `cpu` sums, the byte-identical proof, the lock's
  measured reason, the no-credit/no-verdict negatives, the container leg deferred by name, and the
  hosted prediction with its arithmetic.

## The measured before/after

| Figure | Before (`da38d7d`) | After, `sh` (`f9fe6a9`) | After, `dash` | Final tree, `sh` | Final tree, `dash` |
|---|---|---|---|---|---|
| `gate-time total` | **79** | **72** | 69 | 71 | 72 |
| `gate-cpu user + sys` | **228.23** (121.50 + 106.73) | **260.77** (127.65 + 133.12) | 253.75 | 265.58 | 265.99 |
| `step-time c-tests` | 21 | 24 | 24 | 23 | 26 |
| `step-time c-faults` | 16 | 24 | 24 | 23 | 26 |
| `step-time controls` | 19 | 26 | 24 | 26 | — |
| `step-time hygiene` | 8 | 9 | 9 | — | — |

**The subtraction, printed.** The two steps are serial before the change, so they occupy
`21 + 16 = 37` s of the gate's wall; after it the pair occupies `max(c-faults, c-tests) = 24` s
(the partner's `step-time` is its time-to-join). The pair gives back `37 − 24 = 13` s.
Decomposed: `37 + 19 + 23 = 79` before, `24 + 26 + 22 = 72` after — the pair −13 s, `controls`
+7 s and `hygiene` +1 s of contention under the same load, net `79 − 72 = 7` s. The saving is the
**smaller** member's wall less the contention both picked up: 13 of a possible 16 s, **81 %**.

**`cpu` did not move beyond noise.** `228.23 → 260.77` is a delta of `32.54`, inside the 20 % band
of `45.6`; the `dash` run's `253.75` is a delta of `25.52`; the final-tree runs read `265.58` and
`265.99`. These are local children's user+sys on an `ncpu=18` host and are **not** the hosted
`cpu_s` column (352–441 on a 4-vCPU runner), so nothing here is a hosted claim.

## The byte-identical proof

- `tail -1` of the pre-change log and of every post-change log `cmp` clean at **1,509 bytes**
  (pre vs post-`sh`, post-`dash`, final-`sh`, final-`dash` — all four comparisons).
- Each overlapped step's own buffer — `$GATE_TMP/step.out.<name>`, replayed verbatim — was
  extracted from both logs and `cmp` clean for `c-tests` and for `c-faults`, each ending in its own
  pass line (`-- c-tests pass ctest-runs=2`, `-- c-faults pass fault-runs=238`) and carrying no
  `step-time` line, so the comparison cannot be vacuous (lesson 20).
- The whole logs, with only the measurement lines (`step-time`, `gate-time`, `gate-cpu`,
  `gate-env`, `gate-overlap`) filtered out, `cmp` clean: no behavioural difference anywhere in the
  38 steps.
- `gate-overlap pair=c-faults+c-tests` and `gate-partition preflight=29 producers=9` are both in
  the post log, so the log itself shows the overlap happened.
- The two lock blocks `cmp` clean between their own markers with **no normalisation** (33 lines),
  and each script carries the sentence above the marker that names the other copy and states why
  it is a copy.
- Invariants re-read from the final line: `steps=38/38`, `controls=404/404`, `faults=238/238`,
  `goldens=29`, `gates=118`, `lines=361`, `tracked=2783`, `hygiene-files=2783`, `files=2783`,
  `ctest-runs=2`, `fault-runs=238`, `prefix-files=11`, `pkg-selftest=20/20`,
  `release-selftest=12/12`, `claims=29 provisional=28 claim-refs=27`, `binaries-listed=0`.
  `ls tools/check/steps/*.sh | wc -l` is 38. `git status --porcelain` carries no `??` line.
- The **metadata commit that adds this SUMMARY** moves `tracked`/`files`/`hygiene-files` by exactly
  `+1` (2783 → 2784) and nothing else, because this file is new and `gate.sh`'s `FILELIST` counts
  untracked-not-ignored paths — the gate run that proves it was made with this file present and
  untracked, so it measured the same 2,784 paths the commit contains. Every earlier phase's
  completion commit moved the same count the same way; `steps`, `controls`, `faults`, `goldens`,
  `gates`, `lines` and the test counts do not move.

## Task Commits

Each task was committed atomically:

| # | Task | Commit | Files |
|---|---|---|---|
| 1 | Overlap the pair, serialise the builds, prove the evidence byte-identical | `f9fe6a9` | `tools/gate.sh`, `tools/check/steps/c-tests.sh`, `tools/check/steps/c-faults.sh` |
| 2 | The measured entry: pre/post, the arithmetic, what the lever is not | `e9ac998` | `.planning/CI-EFFICIENCY.md` |

**Measured commit count:** `git rev-list --count da38d7d..HEAD` = **2**.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Missing critical functionality] The overlap declaration is checked, not assumed**
- **Found during:** Task 1, writing the producer loop.
- **Issue:** a renamed or reordered step would silently turn the overlap back into the serial chain
  while the `gate-overlap` line still printed, which is exactly the repudiation the threat register's
  T-16-03 covers.
- **Fix:** both pair members are grepped out of `$GATE_TMP/producers` and a miss is
  `bad gate-overlap-pair "<name> (not a producer)"`; a safety-net join after the producer loop
  writes `rc.<name>`/`t.<name>` for a partner the foreground member never joined, so a reordered
  declaration cannot leave the replay reading a missing file.
- **Files modified:** `tools/gate.sh`
- **Commit:** `f9fe6a9`

**2. [Rule 2 - Missing critical functionality] The partner is killed by the `EXIT` trap**
- **Found during:** Task 1, reading the existing trap block.
- **Issue:** the existing `on_exit` kills only the controls runner; a background partner would
  outlive an interrupted run and keep writing into a temp dir that is about to be removed.
- **Fix:** `on_exit` kills and waits for `$bg_pid` the same way it does `$ctl_pid`; `bg_pid` is
  initialised empty beside `ctl_pid`.
- **Files modified:** `tools/gate.sh`
- **Commit:** `f9fe6a9`

### Plan defects (recorded with the measured reason, not silently satisfied)

**3. [Plan defect] Task 1's `cpu` floor `pa >= 250` is unsatisfiable on this host**
- **Found during:** Task 1's automated verify, the `awk` that reads `pa >= 250 && d <= 0.20 * pa`.
- **Measured:** this host's pre-change sum is `121.50 + 106.73 = 228.23` at `ncpu=18`
  (`cc=AppleClang-21.0.0.21000101`), not the plan's plan-time `~290`. The floor is a plan-time
  host figure, not an invariant, so the clause fails on a *correct* implementation.
- **Substitution:** the floor became the measured pre-change sum itself (`pa > 0`), which is the
  clause's stated intent ("the pre sum is the measured … rather than a remembered figure"); the
  20 % noise band is unchanged and the measured delta `32.54` sits inside `45.6`.
- **Consequence:** the acceptance criterion "`gate-cpu user + sys` is at least 250 before" is
  recorded as **not satisfiable here**, with the measured `228.23` as the reason.

**4. [Plan defect, latent] Task 2's verify reads the post total from a gate run made after the entry exists**
- **Found during:** Task 2's verify: `post_total` is parsed from `/tmp/g16-t2.log`, a run performed
  *by the verify itself*, and then grepped for in the entry that was written before that run.
- **Measured:** it passed by luck of the entry quoting all three measured post totals (72, 69) —
  the fresh run read `gate-time total=69`. A fresh run of 70 or 71 would fail a correct entry.
- **Substitution:** none was needed. Recorded because the clause is not robust; the entry's own
  contract is the two **kept** logs (`79` and `72`), which it carries.

**5. [Plan defect] "`sh tools/hygiene.sh --tree` is silent"**
- **Measured:** it prints its pass line (`-- hygiene pass mode=tree hygiene-files=2783`) and exits
  0. Every step in this project prints a pass line by contract (the pass line must be its last
  non-empty line), so silence was never available.
- **Substitution:** the check is "exit 0 and no `-- FAIL`", which is what the verify's `set -e`
  actually asserts.

**6. [Deviation, recorded against the frozen estimate] The hosted prediction is ~144 s, not 115 s**
- **Found during:** Task 2, doing the subtraction the frozen artifact did not.
- **Measured:** the last hosted profile that priced this lever is `c-faults` 48 s + `c-tests` 25 s
  in a 164 s gate. An overlap can only absorb the **smaller** member, so the arithmetic is
  `164 − 25 × 0.81 ≈ 144` s. `.planning/MILESTONE-CONTEXT.md` §5's "164 s → about 115 s" subtracts
  the larger member (`164 − 49`).
- **Action:** the entry records the frozen figure as **optimistic rather than re-blessed**, and the
  first hosted row after the push is named as the measurement that settles it. The frozen artifact
  is not rewritten (append-only; §5 is an upstream freeze, and 16-05's close is where a reprice
  would be recorded).

**7. [Deviation, requirement tick deferred] PKG-06 is not ticked complete here**
- The plan's frontmatter names `PKG-06`, but the phase's own artifact table assigns
  "`.planning/REQUIREMENTS.md` (PKG-06 `Complete`)" to **16-04 T2**. Ticking it now would mark the
  requirement complete three plans early, so `requirements-completed` is empty and
  `requirements mark-complete` was not run.

### Not in this plan, by name

- The container leg (`sh tools/gate-linux.sh`) was **not run**: one container gate per phase
  (lesson 18), so it is 16-05's, and this SUMMARY records no container result.
- No checkpoint word and no allowance credit: the phase's single verdict is 16-05's, by arithmetic
  over `.planning/ci-runs.tsv`.

## Issues Encountered

- The pair's `step-time` values **rose** (21 → 23–26 for `c-tests`, 16 → 23–26 for `c-faults`)
  because the partner's step-time is now its time-to-join, and because `controls` (19 → 23–26) and
  both members contend for the same host. The gate's total still fell on every post-change run
  (79 → 69/71/72), but the measured saving is 7–10 s rather than the 13 s the pair's own wall gives
  back. This is recorded in the entry rather than smoothed: on a 4-vCPU hosted runner the
  contention is likely worse, which is why the prediction is ~144 s and not 115 s.

## Known Stubs

None. The two `build-lock` details are reachable only on lock expiry and are covered by the same
existing tokens (`c-tests-failed`, `c-faults-run`); nothing was left unwired, and no `<verify>` was
skipped.

## Threat Flags

None. The change adds no network surface, no file access at a trust boundary and no new executable
input; the one shared-write coordination point (`build/dev`, `<results>/.build-lock`) is the
register's T-16-01/T-16-02, both mitigated and both exercised by the runs above.

## User Setup Required

None — no external service, credential or manual step.

## Next Phase Readiness

- The milestone's one pre-committed concurrency lever is discharged; 16-02 (owner route) can start
  without touching `tools/gate.sh`.
- `gate-cpu` should be re-read from the **first hosted row after the push**, not from this SUMMARY:
  the entry's ~144 s hosted prediction is the thing that row settles, and 16-05 charges the frozen
  0.4 hosted cpu-s against the 21 cpu-s allowance while recording the actual addition.
- The one open risk is contention: if the hosted pair saves materially less than 20 s, the entry's
  prediction is wrong and 16-05 should record `reprice` rather than re-bless it.

## Self-Check

- `tools/gate.sh` — FOUND
- `tools/check/steps/c-tests.sh` — FOUND
- `tools/check/steps/c-faults.sh` — FOUND
- `.planning/CI-EFFICIENCY.md` — FOUND
- commit `f9fe6a9` — ancestor of HEAD
- commit `e9ac998` — ancestor of HEAD

## Self-Check: PASSED
