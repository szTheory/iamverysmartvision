---
phase: 18-the-inspection-surface
plan: 01
subsystem: inspection-surface
tags: [frozen-oracle, inspection-contract, trace-record, refusal-tokens, liveness-guards, o1-resolution]

requires:
  - phase: 18-the-inspection-surface
    plan: context
    provides: "the six `## Implementation Decisions` blocks the page is written to, and the O1 resolution recorded in `18-RESEARCH.md` §12"
  - phase: 17-records-cost-window-and-the-count-gate-contract
    provides: "the count-gate contract (D-56) and the recomputed window the later plans price against"
  - phase: 14
    provides: "the append-only refusal-token document shape and the flatwrap tree rule the output policy is copied from"

provides:
  - "`docs/inspection.md`, committed alone before any `tools/inspect/` path exists, as the phase's frozen contract"
  - "the four subcommands with their grammars, `--help` and the three exit statuses, frozen before the code that implements them"
  - "the trace file's three record kinds and every field name, with `tick` first on an access record"
  - "the five reconciliation equalities, the fifth requiring non-decreasing ticks that are not all equal"
  - "the break report's seven keys and the step report's keys, pinned by name for `18-02`"
  - "the five refusal tokens with a trigger and an accepted boundary each, as the machine-read block a later coverage id compares in both directions"
  - "the disassembly corpus, its header rule, its ` -> ` cut, its `ORG` rule, its `ivs_asm` oracle and its drop condition, for `18-03`"
  - "the four liveness guards with the stand-in that shows each can fail"

affects:
  - "18-02 and 18-03: every command, record field, report key, token and guard they implement is cited from this page; a plan sentence that differs is the deviation, not the page"
  - "Phase 19: the consumer that decides a display-path default by discrimination with this instrument"
  - "the v0.4.0 close: the disassembly-limb drop, if taken, is an appended note this page already authorises"

actuals:
  tokens: 3924
  tasks: 1
  commits: 1
  plan_head_before: ecbc7d2c8822a168f70df305588716101288b277
  plan_head_after: 0627be925fd1fc449260c2d4bb8efe20363e8994

tech-stack:
  added: []
  patterns:
    - "an oracle frozen in a commit of its own, before any path it governs exists, so the ancestry proof is a property of the history rather than of a reviewer's memory"
    - "a machine-read refusal block bounded by two headings, with each token on exactly one line, so a later id can compare it with the tool's own token set in both directions"
    - "an ambiguity (O1) resolved in the frozen page with the refused alternative and its reason beside it, so no implementer settles it in the dark"

key-files:
  created:
    - docs/inspection.md
    - .planning/phases/18-the-inspection-surface/18-01-SUMMARY.md
  modified: []

key-decisions:
  - "O1 is resolved in the page, in its own words: the criterion's `last record's tick` is the `end` record's tick, which equals the last step record's `end` and the report's own `step.<k>.now_ticks`, while an access record keeps its own cycle stamp. The alternative - making every record's `tick` the unit's end - was refused in the page because it loses the per-access cycle stamp the requirement names."
  - "The refusal block is a closed set of five tokens. The research's `inspect-output-combo` (a trace combined with an operator output buffer) is not written as a sixth token: `18-02-PLAN.md` implements no such refusal, and the page's block is compared with the tool's token set in both directions, so a sixth token would make the comparison unsatisfiable rather than stronger. The reconciliation equality is stated as a condition on the same scenario and the same options, which is what the later implementation can be held to."
  - "The `step` report is pinned line by line in the order `step.units`, `step.ticks`, `step.halted`, `step.pc`, `step.reg.R0` through `step.reg.R7`, then `step.mem.<AAAA>`, because `18-02` asserts those keys by name; a page that named only the subcommand would leave the report shape to the implementer."
  - "`--help` is part of the frozen surface, with its own exit status and its four subcommand names, because `18-02` tests it and a tested surface element the page omits is a surface the page did not freeze."
  - "The page names `ivs_runner_report_write` as the library's own writer behind the `write_report` seam and never as a call the tool makes, because the record it takes is one `ivs_runner_execute` does not return; the tool's oracle is the text `write_report` received."
  - "No claim word is waived into the page except where a sentence would otherwise read as an outcome: the two lines that needed a waiver carry `[nc]`, no claim id is invented, and `claims=29` and `provisional=28` are unmoved."
  - "TOOL-02 and TOOL-09 stay open. This plan freezes the contract and closes no requirement: the evidence that closes them is the tool and its ids, which land in 18-02 and 18-03."

requirements-completed: []

# Metrics
duration: 9min
completed: 2026-11-26
status: complete
---

# Phase 18 Plan 01: The Inspection Surface Frozen Before Its Code Summary

**`docs/inspection.md` committed alone as the phase's contract — four subcommands, three record kinds with every
field named, five reconciliation equalities, five refusal tokens, four liveness guards and the disassembly limb's
corpus and drop condition — with O1 decided in the page rather than by the implementer, and no path under
`tools/inspect/` existing anywhere in the history it landed on.**

## Baselines

start-commit: ecbc7d2c8822a168f70df305588716101288b277
freeze-commit: 0627be925fd1fc449260c2d4bb8efe20363e8994
baseline-docs: 64
baseline-doc-files: 63

Measured at the start commit, before the first edit, with `sh tools/gate.sh` (one run, `gate-time total=131`):

```
start-commit: ecbc7d2c8822a168f70df305588716101288b277
baseline: gate: PASS steps=38/38 controls=404/404 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=238 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=118 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=29 provisional=28 handoffs=8 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=695 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=238/238 tracked=2819 binaries-listed=0 rights-pinned=4/4 terms=9 doc-files=63 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2819 history-commits=1 gates=120 lines=366 input-rows=31 input-policies=11 input-profile-fields=31 files=2819 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=20/20 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=12/12 runner-files=18 cases=14 sources=13 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=5044 tests-executed=5040 tests-skipped=4 tests-missing=0 docs=64 claim-refs=27 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

The plan's prediction was `docs=64 doc-files=63` before and `docs=65 doc-files=64` after, with every other token
unmoved and the three tracked-file counts rising by two. Both halves were measured, not quoted: the two document
counts and `steps=38/38`, `controls=404/404`, `faults=238/238`, `goldens=29`, `gates=120`, `lines=366`,
`tests-expected=5044`, `tests-skipped=4`, `prefix-files=11`, `claims=29`, `provisional=28` read exactly the
predicted values at the end, and `tracked`/`files`/`hygiene-files` read 2821 (2819 plus the page and this record).

`freeze-commit:` is the commit that added `docs/inspection.md`, and `git log --format=%H --diff-filter=A --
docs/inspection.md` reports exactly that one commit. The page at that commit is a byte prefix of the page at
`HEAD`, which is what makes a later correction an appended note rather than a rewrite of a frozen section
(lesson 21). No path under `tools/inspect/` exists in the tree, so the ancestry `18-02` proves has nothing to
find on the wrong side of it.

## What the page pins, by section

- `## What it is built on` — the five seams with the file that declares each: `ivs_machine_peek` and
  `ivs_machine_cpu_state` (public), `ivs_machine_state_digest` (public, the nine domains), 
  `ivs_machine_step_unit_trace_internal` (internal, bounded by `IVS_CP1610_MAX_ACCESSES`, and the one reason
  the tool links `ivs_core_internal`), `ivs_input_apply_due` (internal, the core's own input floor) and the
  runner library (`ivs_scenario_parse`, `ivs_runner_execute`, the report's bytes through `write_report`, and
  `ivs_runner_report_write` as the library's writer, never the tool's call). It also carries the forcing
  sentence: the report comes from the library because a second writer would be a second truth, and the trace
  comes from the tool's own per-unit loop because the executor carries no access log and its state struct is
  static in `runner/exec.c`.
- `## Command surface` — one binary, four subcommands with their exact grammars, the three exit statuses (0
  done, 1 a named refusal with one `ivs-inspect: ` line and no output file, 2 a usage or file error with no
  token), `--report`'s meaning, `--help` with its own exit status, and the `step` report's keys in order.
- `## The trace file` — `ivst`, ASCII, LF, one record per line; the access record with `tick=` first and the
  seven fields after it; the nine `kind` values; `tick = unit_start + cycle * ticks_per_cpu_cycle` with the
  multiplier read from the profile; the step record; the `end` record last; and the O1 resolution.
- `## The reconciliation` — the five equalities, the fifth being non-decreasing **and not all equal**, and
  `inspect-reconcile` with exit 1 and no output file on any failure.
- `## Breakpoints` — a hit is a `fetch` or an `iab` (the reset entry is an `IAB`), the hit/non-hit pair in one
  report, non-interference asserted separately, and the break report's seven keys in order.
- `## Disassembly` — the corpus in `data/expected/listings.ids`, `ROW` plus `BITS` as the only table,
  `inspect-dis-row` fail-closed, the `ivs_asm` re-assembler and the `ivs_pack --dump-words` comparison, the
  eight header keys and the `asm-header` reason for dropping every other `#` line, the ` -> ` cut, the
  two-or-more-spaces note cut, the repeated-leading-token rule, the `ORG` rule, and the drop's reversal
  condition.
- `## Refusal tokens` — the five tokens, each alone on its line, each with its trigger and its accepted
  boundary; the block is bounded by `## Liveness guards`.
- `## Liveness guards` — four guards, each with the committed stand-in that shows it can fail (target-instead-of-
  stop, a poked register, an always-hits matcher, a `DECLE`-everything emitter).
- `## Rights` and `## What this instrument is not` — the `owner-local-only` row with nothing taken, `ivst`
  refused in `tools/check/binary-ext.txt` and `.gitignore`, the `refs/` exception, the sentence that nothing
  the tool writes is ever committed, and the four things the instrument is not (no frontend, no new core
  surface, no second decoder table, no claim) with Phase 19 named as its consumer.

## Task Commits

| Task | Commit | Files |
|---|---|---|
| 1 (freeze) | `0627be9` | `docs/inspection.md` |
| 1 (record) | this plan's completion commit | `18-01-SUMMARY.md` |

## Files Created/Modified

- `docs/inspection.md` — created, 15,699 bytes, 0 modified.
- `.planning/phases/18-the-inspection-surface/18-01-SUMMARY.md` — created.

No code, no directory under `tools/inspect/`, no test, no registry row, no rights row, no inventory file. The
page is the whole of the plan's product, which is what makes its ancestry proof mean something.

## Deviations from Plan

**1. [Rule 3 - Blocking, recorded not fixed] The commits land on `main`.**

The executor's pre-commit guard refuses a commit whose `HEAD` is a protected branch unless the project config
allows it. This repository's `.planning/config.json` sets `git.branching_strategy: none` and
`workflow.use_worktrees: false`, and `EXEC-RULES.md` says the sequential run happens on the current phase
branch — which is `main` here, as every phase commit of this milestone is. The guard's rule is answered by the
project config, and re-homing onto a `gsd/phase-18-*` branch would have contradicted the configured workflow
and the phase's own history. Recorded here rather than repaired, because the alternative is a workflow change
this plan is not authorised to make.

**2. [Rule 2 - Auto-added critical content] Two lines carry a `[nc]` waiver the plan did not predict.**

The page says in two places that it mints no claim and that its output is never evidence about a guest program.
Written without a waiver, those sentences would read as claim words to `tools/check/steps/text.sh`. Both carry
`[nc]`; no claim id was invented, and `claims=29` and `provisional=28` did not move. This is the page's own
prose discipline, not a scanner quieted by re-wording: the two sentences say exactly what they said before the
waiver.

**3. [Recorded reading] The research's `inspect-output-combo` token is deliberately absent.**

`18-RESEARCH.md` §1.6 recommends refusing `trace` combined with an operator output option under a sixth token.
The plan fixes the block at five tokens and `18-02-PLAN.md` implements no such refusal, so writing a sixth would
leave the page's block unsatisfiable by the both-directions comparison `18-02` runs. The page instead states the
reconciliation as a condition on the same scenario and the same options. This is a decision, not an omission:
the alternative would be a token no case can exercise.

## Gate

Measured on this tree with the page committed and this record written, one run per shell (`sh` first; the
tree's content is identical before and after the completion commit, which adds only this file):

```
sh tools/gate.sh:
gate: PASS steps=38/38 controls=404/404 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=238 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=118 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=29 provisional=28 handoffs=8 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=696 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=238/238 tracked=2821 binaries-listed=0 rights-pinned=4/4 terms=9 doc-files=64 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2821 history-commits=1 gates=120 lines=366 input-rows=31 input-policies=11 input-profile-fields=31 files=2821 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=20/20 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=12/12 runner-files=18 cases=14 sources=13 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=5044 tests-executed=5040 tests-skipped=4 tests-missing=0 docs=65 claim-refs=27 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8

sh tools/hygiene.sh --tree:
-- hygiene pass mode=tree hygiene-files=2821
```

`scanned=696` is the baseline's 695 plus this page, which is the same +1 the two document counts carry.

Every token the plan names as an invariant reads its predicted value: `steps=38/38`, `controls=404/404`,
`faults=238/238`, `goldens=29`, `gates=120`, `lines=366`, `tests-expected=5044`, `tests-executed=5040`,
`tests-skipped=4`, `tests-missing=0`, `claims=29`, `provisional=28`, `claim-refs=27`, `rights=10`,
`rights-cleared=1`, `prefix-files=11`, `pkg-selftest=20/20`, `release-selftest=12/12`, `listings=7`. The only
tokens that moved are the two document counts (`docs=64` to `65`, `doc-files=63` to `64`) and the three
tracked-file counts (`2819` to `2821`).

The two targeted steps were also run on the page before its commit, because a prose failure discovered after the
freeze could only be fixed by editing a frozen section: `text pass docs=65 claim-refs=27` and
`glossary pass terms=9 doc-files=64 profile-keys=154`, both clean.

## Known Stubs

None. The page is prose and describes no implementation; the stubs it would otherwise be judged against (a
refusal token with no case, a record field with no writer) belong to 18-02 and 18-03 and are named there.

## Threat Flags

None. This plan adds one Markdown page and one record: no network surface, no file access, no authentication
path, no schema change at a trust boundary. The page describes a trust boundary (an output path inside the work
tree, a rights-bearing artifact) without creating one; the mechanisms land in 18-02.

## Next Phase Readiness

- `18-02` implements the command surface, the trace record, the reconciliation, the breakpoint pair and the
  output policy from this page, and proves its ancestry with `git merge-base --is-ancestor 0627be9 <first tool
  commit>`.
- `18-03` implements the disassembly limb from the page's derivation, or takes the drop and appends the note the
  page authorises.
- The page is append-only from here: a correction is an appended note with its own commit, and the freeze check
  is a prefix comparison rather than a whole-file comparison.
- No blocker. The one live assumption the phase carries is research A1 (the traced instance's nine digests
  equal the report's nine for a plain run); it belongs to 18-02 and is measured there before anything is built
  on it.

---

*Phase: 18-the-inspection-surface*
*Completed: 2026-11-26*

## Self-Check: PASSED
