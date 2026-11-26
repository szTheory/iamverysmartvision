---
phase: 13-rights-ledger-and-enforced-claim-ceiling
plan: 05
subsystem: testing
tags: [cost-checkpoint, reprice, ci-runs, gate-token-line, two-shell-evidence, container-gate, validation-sign-off, hosted-row-not-recorded]

requires:
  - phase: 13-rights-ledger-and-enforced-claim-ceiling
    provides: the six Phase 13 control directories, the fifteen registry rows and the nine-token gate delta that plans 13-01 to 13-04 landed and this plan measures
  - phase: 09-registered-evidence-and-gate-cost
    provides: the checkpoint rule's three-value vocabulary (GUARDRAILS 2.3), the important-gates baseline and the `ci-log` row format the hosted record must satisfy
provides:
  - one recomputed cost checkpoint in .planning/STATE.md, verdict `reprice`, with the five cpu_s values, both medians and the arithmetic beside it
  - the phase's measured gate line, proven byte-identical under `sh` and `dash` and repeated inside the Ubuntu 24.04 container with `gate-linux: exit 0`
  - a signed-off 13-VALIDATION.md (status validated, nyquist_compliant, wave_0_complete, nine per-task rows, seven Wave 0 bullets ticked)
  - a visible `hosted row: not-recorded` line naming the command that closes it, and `.planning/ci-runs.tsv` untouched at 96 rows
affects: [14, 15, 16]

actuals:
  # 41,832 chars / 4: the two task commits (15,359 chars, `git diff 10f66a1..HEAD`)
  # plus this SUMMARY (26,473 chars). Same chars/4 scale the plan's 55,000 estimate
  # used; the plan rated its own confidence `low`, and it under-estimated by about
  # 5x because the token-line reconciliation and the four-token container comparison
  # are prose, not code.
  tokens: 10458
  tasks: 2
  commits: 2
  plan_head_before: 10f66a199092d4fc1f04669d8e79045e62544cf3
  plan_head_after: d12dc25b1a670347d173d5382bbf60d36004f09f

tech-stack:
  added: []
  patterns:
    - "a phase boundary recomputes the last-five medians by arithmetic over the committed rows and prints the five values beside the middle one, so the verdict is auditable without re-running anything"
    - "the plan's token prediction is compared token by token and a differing token is chased to its cause before it is recorded, never re-blessed"
    - "an evidence gap that no command can close (an unpushed phase has no hosted run) becomes a visible not-recorded line naming the command that closes it, in the same place the pass would have been recorded"

key-files:
  created:
    - .planning/phases/13-rights-ledger-and-enforced-claim-ceiling/13-05-SUMMARY.md
  modified:
    - .planning/STATE.md
    - .planning/phases/13-rights-ledger-and-enforced-claim-ceiling/13-VALIDATION.md

key-decisions:
  - "The verdict is `reprice`, not `discharged-within-allowance`: the recomputed last-five cpu_s median is 366, above 361 + 1.935 = 362.935, and the frozen 361/118 figures are stale by measurement (+5 and +3), which is stated rather than forced to match the frozen number."
  - "`lever-required` is not reached and the arithmetic says why: cumulative milestone spend is 1.935 of 21, and this phase's additions landed at exactly the priced count (controls 395 to 401, important-gates lines 337 to 352, steps/faults/goldens unmoved), so no line overran by 25 percent and no lever is invented for a runner event."
  - "The phase's hosted row is recorded as `not-recorded`, never fabricated: `gh run list --commit 10f66a1 --json databaseId,event,conclusion,headSha` returns `[]`, so `.planning/ci-runs.tsv` keeps its 96 rows and the SUMMARY carries the visible line naming `sh tools/ci-record.sh <run-id>` as the closer."
  - "The plan's `tracked=2718` prediction is stale by 4 and the cause is named, not re-blessed: 2715 at 13-04's start, +2 from b1abccc, +4 from 2c32390's `important-gates-excluded-stale` control directory, +1 from 13-04's own SUMMARY = the measured 2722, which 13-04-SUMMARY.md had already recorded."
  - "The six `## Validation Sign-Off` boxes were ticked although the plan named only the frontmatter, the per-task map, the Wave 0 bullets and the Approval line: a `status: validated` file with six unticked sign-off boxes is a contradictory record, and all six are true and measured (gate-time 97 under sh, 106 under dash, both under the 120 s limit)."
  - "GUARDRAILS 4's `sh tools/hygiene.sh --tree # silent` was left unamended and the tool's actual `-- hygiene pass mode=tree hygiene-files=2722` line recorded instead: the frozen doc is milestone-level policy, the pass line is what the gate's own `hygiene-files=` token reads, and amending a frozen guardrail is not this plan's change."

requirements-completed: [RTS-01, RTS-02, EV-06, EV-07]

coverage:
  - id: D1
    description: "One recomputed cost checkpoint in .planning/STATE.md in the frozen three-value vocabulary, with the five cpu_s values, both medians and the arithmetic beside it, computed from the committed rows rather than quoted."
    requirement: EV-06
    verification:
      - kind: automated
        ref: "grep -c -e '^- \\[Phase 13\\]: 13-05: checkpoint: ' .planning/STATE.md -> 1"
        status: pass
      - kind: automated
        ref: "grep -v '^date' .planning/ci-runs.tsv | tail -5 | awk -F'\\t' '{print $13, $10}' -> cpu_s 435 366 363 497 357 / gate_s 137 121 114 148 121; sorted cpu_s 357 363 366 435 497 -> median 366; sorted gate_s 114 121 121 137 148 -> median 121"
        status: pass
    human_judgment: false
  - id: D2
    description: "The phase's closing gate line, measured fresh under both shells and inside the Ubuntu 24.04 container, with every predicted token reconciled."
    requirement: EV-06
    verification:
      - kind: automated
        ref: "tail -1 /tmp/g13.out | cmp - /tmp/g13d.out's tail -> identical; last line is 'gate: PASS steps=38/38 controls=401/401 ... faults=238/238 ... goldens=29 ... tiers=8'"
        status: pass
      - kind: integration
        ref: "GATE_LINUX_IMAGE=ivs-gate-linux sh tools/gate-linux.sh -> 'gate-linux: exit 0', log copied to /tmp/gate-linux.13-05.log with 0 '-- FAIL' lines"
        status: pass
    human_judgment: false
  - id: D3
    description: "The phase's hosted row, recorded from a real run id when one exists, or its absence visible with the reason and the command that closes it."
    requirement: EV-06
    verification:
      - kind: automated
        ref: "gh run list --commit 10f66a199092d4fc1f04669d8e79045e62544cf3 --limit 5 --json databaseId,event,conclusion,headSha -> [] ; git rev-list --count origin/main..HEAD -> 24 (nothing of this phase is pushed)"
        status: pass
      - kind: automated
        ref: "ROOT=$PWD CHECK_DIR=$PWD/tools/check FILELIST=/tmp/fl13 sh tools/check/steps/ci-log.sh -> '-- ci-log pass ci-runs=96' (unchanged, so no row was appended)"
        status: pass
    human_judgment: false
  - id: D4
    description: "13-VALIDATION.md signed off: status validated, nyquist_compliant and wave_0_complete true, nine per-task rows, all seven Wave 0 bullets ticked, Approval carrying the measured line."
    requirement: RTS-02
    verification:
      - kind: automated
        ref: "grep -c -e '^status: validated$' 13-VALIDATION.md -> 1; grep -c -E '^\\| 13-0[1-5]-0[1-3] \\|' -> 9; unticked bullets under '## Wave 0 Requirements' -> 0 (7 ticked)"
        status: pass
    human_judgment: false

duration: 22min
completed: 2026-11-25
status: complete
---

# Phase 13 Plan 05: Phase close, two-shell gate evidence and the recomputed cost checkpoint Summary

**The phase closes on a measured line — `steps=38/38 controls=401/401 … faults=238/238 … goldens=29`, byte-identical under `sh` and `dash` and repeated in the Ubuntu 24.04 container — with one cost verdict, `reprice`, recomputed by arithmetic as a `cpu_s` median of 366 against a priced addition of 1.935, and the hosted row recorded as a visible `not-recorded` line rather than a fabricated one.**

## Performance

- **Duration:** 22 min
- **Started:** 2026-11-25T22:35:00Z
- **Completed:** 2026-11-25T22:57:00Z
- **Tasks:** 2
- **Files modified:** 2 (`STATE.md`, `13-VALIDATION.md`), plus this SUMMARY

## The measured summary line

`sh tools/gate.sh` and `dash tools/gate.sh`, both exit 0, their last lines byte-identical
(`cmp` of `tail -1` on each log). Quoted in full, once:

```
gate: PASS steps=38/38 controls=401/401 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=238 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=96 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=28 provisional=27 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=686 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=238/238 tracked=2722 binaries-listed=0 rights-pinned=4/4 terms=9 doc-files=63 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2722 history-commits=1 gates=114 lines=352 input-rows=31 input-policies=11 input-profile-fields=31 files=2722 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=19/19 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=11/11 runner-files=18 cases=14 sources=13 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=4990 tests-executed=4986 tests-skipped=4 tests-missing=0 docs=64 claim-refs=26 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

`gate-time total=97` under `sh`, `106` under `dash`; `gate-cpu user=151.32 sys=153.47`.
`dash` is installed on this host, so the evidence rests on two shells and not on one.

The line above was taken at the Task-2 commit `d12dc25`, before this SUMMARY existed. Re-run at
the metadata commit — where this SUMMARY is itself on disk and therefore in the gate's file list —
it reads `tracked=2723`, `files=2723`, `hygiene-files=2723` and is **otherwise byte-identical**:
the same one-file effect plan 13-04 recorded at its own metadata commit (`tracked=2722` there,
one higher than at its Task-3 commit). The quoted line is the one taken at `d12dc25` because it is
the last state in which the file list is exactly the phase's own.

## Prediction versus measurement

Every token the plan predicted, compared against the fresh run. `✓` is an exact match.

| Token | Predicted (13-05-PLAN) | Measured | |
|-------|------------------------|----------|---|
| `steps` | `38/38` | `38/38` | ✓ |
| `controls` | `401/401` | `401/401` | ✓ |
| `faults` | `238/238` | `238/238` | ✓ |
| `goldens` | `29` | `29` | ✓ |
| `tracked` | `2718` | `2722` | **+4 — cause found, below** |
| `binaries-listed` | `0` | `0` | ✓ |
| `rights-pinned` | `4/4` | `4/4` | ✓ |
| `manifests` | `11` | `11` | ✓ |
| `effective-yes` | `8` | `8` | ✓ |
| `effective-no` | `3` | `3` | ✓ |
| `rights` | `10` | `10` | ✓ |
| `rights-cleared` | `1` | `1` | ✓ |
| `claims` | `28` | `28` | ✓ |
| `provisional` | `27` | `27` | ✓ |
| `gates` | `114` | `114` | ✓ |
| `lines` | `352` | `352` | ✓ |
| `sources` | `13` | `13` | ✓ |
| `refs-verified` | `3/4` | `3/4` | ✓ |
| `cases` (runner-selftest) | `14` | `14` | ✓ |
| `docs` (text step) | `64` | `64` | ✓ |
| `claim-refs` | `26` | `26` | ✓ |

**The one differing token, chased rather than re-blessed.** The plan's context block predicted
`tracked=2718`; the measured value is `2722`. The delta decomposes exactly over this phase's own
commits, and it is not a movement of any step's payload:

| Commit | Added | `tracked` after |
|--------|-------|-----------------|
| `c2680a7` (13-04's start) | — | 2715 |
| `b1abccc` | `data/expected/runner-selftest.ids`, `tools/check/selftest-inventory.sh` | 2717 (+2) |
| `2c32390` | the `important-gates-excluded-stale` control directory: `CHECK`, `EXPECT`, `FORBID`, `overlay/data/expected/important-gates-excluded.ids` | 2721 (+4) |
| `1f814c8` | `13-04-SUMMARY.md` | 2722 (+1) |

So the +4 against the prediction is files plans 13-01 to 13-04 created, and the measured figure is
the one `13-04-SUMMARY.md` already recorded at its own metadata commit (`tracked=2722`). The value
`2718` corresponds to no commit in the phase's history, so it was a planning-time count taken
between `b1abccc` and `2c32390` and carried forward. No other token moved, `files=` and
`hygiene-files=` report the same 2722 through two independent steps, and the three invariants
(`steps=38/38`, `faults=238/238`, `goldens=29`) are where they were frozen.

## The cost checkpoint

Recomputed by arithmetic from `.planning/ci-runs.tsv`, never quoted. The last five data rows in
file order, all `conclusion=success` with a real `cpu_s` (no `-1` row among them), so no row was
dropped:

| # | date | run id | event | `cpu_s` | `gate_s` |
|---|------|--------|-------|---------|----------|
| 1 | 2026-11-25 | `45677865721` | push | 435 | 137 |
| 2 | 2026-11-25 | `45682366772` | pull_request | 366 | 121 |
| 3 | 2026-11-25 | `45682845654` | push | 363 | 114 |
| 4 | 2026-11-25 | `45683331636` | pull_request | 497 | 148 |
| 5 | 2026-11-25 | `45683886915` | push | 357 | 121 |

- `cpu_s` sorted: **357, 363, 366, 435, 497** → middle value **366**.
- `gate_s` sorted: **114, 121, 121, 137, 148** → middle value **121**.

**The frozen 361 / 118 figures are stale by measurement.** The committed file gives a `cpu_s`
median of 366 (+5) and a `gate_s` median of 121 (+3). That is stated as a measurement, not
smoothed to agree with the frozen number.

**The phase's priced addition**, on the frozen unit costs:

| Item | Unit | Count | cpu-s |
|------|------|-------|-------|
| New control directories | 0.31 each | 6 (`controls` 395 → 401) | 1.860 |
| `important-gates.ids` rows | 0.005 each | 15 (`lines` 337 → 352) | 0.075 |
| New faults | 0.44 each — forbidden | 0 | 0.000 |
| New `runner-selftest.ids` cases | already running | 0 | 0.000 |
| **Total** | | | **1.935 of the 21 cpu-s allowance** |

**The single verdict: `reprice`.**

- `discharged-within-allowance` requires median `cpu_s` ≤ 361 + 1.935 = **362.935**. The measured
  median is **366**, which is 3.065 above that bound. Not reachable, whatever the token line did.
- `lever-required` requires cumulative milestone spend past 21, or a phase overrunning its line by
  more than 25 %. Cumulative spend is **1.935 of 21** (this is the milestone's first closed phase;
  the milestone prices 5.2 in total), and this phase's additions landed at *exactly* the priced
  count — six control directories and fifteen registry rows, with `steps`, `faults` and `goldens`
  unmoved. Nothing overran, so no lever is invented.
- `reprice` is what is left, and it is what the rule names: **the median moved more than the priced
  addition (5 > 1.935) while the token line is where it was predicted** — a runner event on the
  shared series, not a cost this phase added.

Recorded in `.planning/STATE.md` as exactly one new bullet in `## Accumulated Context`, in the
house style, with the arithmetic beside the adjective:

```
- [Phase 13]: 13-05: checkpoint: reprice (cpu_s 357 363 366 435 497, median 366; gate_s 114 121 121 137 148, median 121; priced addition 1.935; allowance 21). …
```

## Container gate

`sh tools/gate-linux.sh` ran **once**, at most, on the Ubuntu 24.04 image (gcc 13.3.0, cmake
3.28.3), and `build/gate-linux.log` was copied to `/tmp/gate-linux.13-05.log` (540 lines, 0
`-- FAIL` lines) before anything else ran. Result: **`gate: PASS steps=38/38 controls=401/401 …
faults=238/238 … goldens=29`**, `gate-linux: exit 0`, `gate-time total=342`.

The container is `--rm` by the tool's own contract, so nothing was left running; the image was
tagged `ivs-gate-linux` for the run to keep the `ivs-<purpose>` naming rule and the tag removed
afterwards (`docker images` now lists no `ivs-*` entry). The pre-existing `build/gate-linux.log`
from a previous phase was copied aside to `/tmp/gate-linux.prior.log` before the run overwrote it;
it was stale (`tracked=2649 controls=395`), which is why the gate was re-run rather than read.

Comparing the two summary lines token by token, exactly four differ, each a recorded host
difference rather than a movement of this phase's line:

| Token | Darwin (`sh`) | Container | Why |
|-------|---------------|-----------|-----|
| `cc` | `AppleClang` | `GNU` | D-24 keeps two compilers and two operating systems without a second hosted job |
| `core-imports` | `7` | `5` | the Darwin `-O2` build imports `__stack_chk_fail` and `__stack_chk_guard` |
| `archive` | `members` | `sha256` | plan 04-22: CMake re-runs `ranlib` at install on Darwin, so the archive is compared member-wise there |
| `refs-verified` | `3/4` | `0/4` | the sources registry's cached PDFs are not sent into the container |

Every invariant the phase freezes — `steps=38/38`, `controls=401/401`, `faults=238/238`,
`goldens=29`, `rights=10 rights-cleared=1`, `gates=114 lines=352`, `cases=14` — is identical in
both.

## Hosted row

**`hosted row: not-recorded — no push run for 10f66a199092d4fc1f04669d8e79045e62544cf3 at 2026-11-25; the orchestrator records it from sh tools/ci-record.sh <run-id> after the push`**

Measured, not assumed:

- `gh run list --commit 10f66a199092d4fc1f04669d8e79045e62544cf3 --limit 5 --json databaseId,event,conclusion,headSha` → `[]`.
- `git rev-list --count origin/main..HEAD` → 24, and `git branch -r --contains HEAD` → nothing, so no commit of this phase is on any remote branch.
- `.planning/ci-runs.tsv` was therefore **not touched**: `-- ci-log pass ci-runs=96` before and after, the same count the phase inherited.

No row was fabricated and no commit was created merely to trigger a run. The checkpoint verdict
above stands on the 96 rows already committed, and the five rows it used are named by run id in
the table so a later reader can re-derive the same medians.

## 13-VALIDATION.md sign-off

Signed off in the same commit as the position record, because the phase's own record should
reflect the plans it validates:

- Frontmatter: `status: validated`, `nyquist_compliant: true`, `wave_0_complete: true`.
- `## Per-Task Verification Map` grew from its single placeholder row to **nine** rows — 13-01 T1, 13-02 T1, 13-03 T1–T2, 13-04 T1–T3, 13-05 T1–T2 — each carrying the automated command its own plan states, its wave, its requirement, its threat refs and `✅ green`.
- `## Wave 0 Requirements`: **all seven** bullets ticked, each amended where the plan authorised it to name the file that actually satisfies it. The ledger-checker row now names `tools/check/rights-lib.cmake` and its two call sites (`manifests.cmake:34` for RTS-01, `files.cmake:41` for RTS-02) in place of the `schema/rights.txt` / `kinds/rights.cmake` pair the row had guessed at, and records that no new gate step was added.
- `## Validation Sign-Off`: all six boxes ticked; `Feedback latency < 120s` is backed by the measured `gate-time total=97` (sh) and `106` (dash).
- `**Approval:**` carries the full measured summary line and the date, replacing `pending`.
- The rest of the file's prose — the infrastructure table, the sampling rate, the manual-only row — is intact: this is an amendment, not a rewrite (GUARDRAILS 2.10).

## Task Commits

Each task was committed atomically:

1. **Task 1: Full gate under two shells, token-line reconciliation, and the recomputed cost checkpoint** - `9477b3d` (docs)
2. **Task 2: The phase's hosted row, or a visible not-recorded line** - `d12dc25` (docs)

**Plan metadata:** the SUMMARY commit (docs: complete plan)

## Files Created/Modified

- `.planning/STATE.md` — one new `[Phase 13]: 13-05: checkpoint: reprice (…)` bullet with its arithmetic; `## Current Position` now records Phase 13 complete with the measured line; `## Session Continuity` names `/gsd:verify-work 13` as the resume point and the one open action. The `progress:` frontmatter block was left untouched, as the plan required.
- `.planning/phases/13-rights-ledger-and-enforced-claim-ceiling/13-VALIDATION.md` — signed off (see above).
- `.planning/ci-runs.tsv` — **unchanged** at 96 rows.
- `.planning/phases/13-rights-ledger-and-enforced-claim-ceiling/13-05-SUMMARY.md` — this file.

## Decisions Made

- **`reprice`, stated with its arithmetic.** The rule reaches it without an adjective doing the work: 366 > 362.935 rules out `discharged-within-allowance`, and 1.935 of 21 with the additions at exactly the priced count rules out `lever-required`.
- **The frozen figures are named as stale.** 361 → 366 and 118 → 121. Quoting them would have certified a number nobody measured, which is the exact failure T-13-20 registers.
- **The hosted row is a visible line, not a gap.** The `not-recorded` form names the sha, the date and the closing command, so the omission is auditable and closes without a second plan.
- **The `tracked=` delta was chased to four named commits.** A differing token is a stop, not a re-bless; the cause is a planning-time count, and the measurement agrees with what 13-04 already recorded.
- **`13-VALIDATION.md`'s `created: "2026-11-26"` was left as the planner wrote it** and the Approval line carries 2026-11-25 with a parenthetical saying which is which, rather than silently rewriting another plan's field or writing a validation date before the creation date without a note.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Record accuracy] The plan's `tracked=2718` prediction is stale by four files**
- **Found during:** Task 1 (token-line reconciliation)
- **Issue:** The plan's context block predicted `tracked=2718` and asserted it was "measured at HEAD of this plan (after 13-04)". The fresh run measures 2722, and 2718 corresponds to no commit in the phase's history.
- **Fix:** Chased the delta instead of re-blessing it: 2715 at `c2680a7` → 2717 at `b1abccc` (+2) → 2721 at `2c32390` (+4, the `important-gates-excluded-stale` control directory) → 2722 at `1f814c8` (+1, 13-04's SUMMARY). The measured value is the one `13-04-SUMMARY.md` already recorded. No file was changed for this; the record carries the reconciliation.
- **Files modified:** none (recorded in this SUMMARY and in `13-VALIDATION.md`'s Approval block)
- **Verification:** `git ls-tree -r --name-only <commit> | wc -l` at each of the four commits, and `git diff --name-status --diff-filter=A` between them
- **Committed in:** `9477b3d` / `d12dc25` (as record text)

**2. [Rule 1 - Record accuracy] `sh tools/hygiene.sh --tree` is not silent, contrary to GUARDRAILS 4 and the plan's acceptance criterion**
- **Found during:** Task 1 (definition-of-done sweep)
- **Issue:** Both the plan's acceptance criterion and GUARDRAILS §4 say the hygiene command "prints nothing". The tool prints `-- hygiene pass mode=tree hygiene-files=2722`, and that pass line is exactly what the gate's own `hygiene-files=2722` token reads.
- **Fix:** Ran the check and recorded its actual output. Zero `-- FAIL` lines, exit 0, `hygiene-files=2722` agreeing with `tracked=2722` and `files=2722`. The correct reading of "silent" is "no `-- FAIL` line"; GUARDRAILS §4's wording is stale relative to the tool.
- **Files modified:** none. `GUARDRAILS-v0.3.md` was deliberately **not** amended: it is frozen milestone-level policy, and amending a frozen guardrail is a milestone change, not a phase-close change.
- **Verification:** `sh tools/hygiene.sh --tree` → one pass line, exit 0; `grep -c '^-- FAIL '` over the gate log → 0
- **Committed in:** `d12dc25` (recorded in `13-VALIDATION.md` and here)

### Deliberate additions beyond the plan's letter

**3. [Rule 2 - Missing critical record] The six `## Validation Sign-Off` boxes were ticked**
- The plan named the frontmatter, the per-task map, the Wave 0 bullets and the Approval line, and said to leave the rest of the prose intact. It did not name the six sign-off boxes.
- A file reading `status: validated` beside six unticked sign-off boxes is a contradictory record, so all six were ticked. Each is true and measured: nine of nine tasks carry an automated command, sampling continuity holds, Wave 0 covers every reference, there is no watch-mode flag, and the measured feedback latency is 97 s (`sh`) and 106 s (`dash`), under the 120 s limit.
- Ticking a checkbox is not a prose rewrite, so GUARDRAILS 2.10 is honoured.

**4. [Rule 3 - Blocking] The container image was tagged `ivs-gate-linux` for the run**
- `tools/gate-linux.sh` pulls `ubuntu:24.04` by default and its `docker run --rm` names no container, so neither the image nor the container would carry the `ivs-<purpose>` name the phase's resource-hygiene rule requires.
- The image was tagged `ivs-gate-linux`, passed through `GATE_LINUX_IMAGE`, and the tag removed immediately afterwards. The container is removed by `--rm`; `docker ps -a --filter ancestor=ivs-gate-linux` is empty and `docker images` lists no `ivs-*` entry. The tool was not modified.

---

**Total deviations:** 2 auto-fixed (2 record-accuracy), 2 deliberate additions (1 missing critical record, 1 blocking). **Impact:** no source file, no gate step, no fault, no golden, no control and no registry row moved. `steps=38`, `faults=238`, `goldens=29` and `controls=401` are exactly where the phase found them; the two `.planning/` records are the whole diff.

## Issues Encountered

- The container gate runs `linux/amd64` under emulation on this arm64 host, so it took 342 s of gate time plus the package install. It ran once, as required, and its log was copied before anything else touched the tree.
- Nothing else. Both shells, the container, the hygiene sweep, the tracked-binary sweep and `git status --porcelain` came back clean on the first attempt.

## Known Stubs

None. This plan changes two `.planning/` records; it adds no code path, no data source and no placeholder text.

## Threat Flags

None. No new endpoint, auth path, file-access pattern or trust boundary was introduced: the plan edits two records and reads one committed TSV.

## Next Phase Readiness

- **Phase 13 is complete on a measured line.** `steps=38/38 controls=401/401 … faults=238/238 … goldens=29`, identical under two shells and in the container, with `rights=10 rights-cleared=1`, `gates=114 lines=352` and `cases=14`.
- **The one open action, named so it cannot be lost:** record the phase's hosted row with `sh tools/ci-record.sh <run-id>` after the push, then append the printed row to `.planning/ci-runs.tsv`. `ci-log` will check its 13 columns, its run-id uniqueness and its date order. Until then the medians above stand on the 96 committed rows.
- **The cost picture a later phase inherits:** last-five `cpu_s` median **366**, `gate_s` median **121**, against a frozen 361/118 and a 21 cpu-s allowance with 1.935 spent. The next phase boundary recomputes rather than quotes, per GUARDRAILS 2.3.
- `/gsd:verify-work 13` is the resume point.

---
*Phase: 13-rights-ledger-and-enforced-claim-ceiling*
*Completed: 2026-11-25*

## Self-Check: PASSED

24 of 24 checks pass on the final tree.

- **Files exist:** `.planning/STATE.md`, `13-VALIDATION.md`, `13-05-SUMMARY.md`.
- **Commits exist and are ancestors of HEAD:** `9477b3d` (Task 1), `d12dc25` (Task 2).
- **Gate:** last line of `/tmp/g13.out` is `gate: PASS steps=38/38 … controls=401/401 … faults=238/238 … goldens=29 …`; `tail -1` of the `dash` log is byte-identical (`cmp`). Re-run at the metadata commit: exit 0, zero `-- FAIL` lines, and the only token that moved is `tracked`/`files`/`hygiene-files` 2722 → 2723 because this SUMMARY is now a file.
- **Checkpoint:** exactly one `- [Phase 13]: 13-05: checkpoint: ` bullet, and its verdict word is `reprice`.
- **VALIDATION:** `status: validated` exactly once, `nyquist_compliant: true`, `wave_0_complete: true`, nine per-task rows, seven Wave 0 bullets ticked and none unticked.
- **Hosted row:** `-- ci-log pass ci-runs=96` — unchanged, so no row was appended and none was fabricated.
- **Hygiene:** the tracked-binary sweep prints nothing; `sh tools/hygiene.sh --tree` prints one pass line and no `-- FAIL` line.
- **This SUMMARY** carries the five `cpu_s` values, both medians, the single verdict with its arithmetic, the container result and the `not-recorded` line.
