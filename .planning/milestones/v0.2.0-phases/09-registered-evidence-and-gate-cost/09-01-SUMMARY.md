---
phase: 09-registered-evidence-and-gate-cost
plan: 01
subsystem: Phase 9 wave A1, plan 1 of 3 - the cost lever (3) and trim (a): the two demo-lamp-tone variants fold into `release-only` rows with a derivation-equality judgement, and three of the four round-trip refusal launches become static judgements against the parser's own guard sites
tags: [A1, EV-01, D-09-01, D-09-02, A-03, A-04, lever-3, trim-a, release-only, derivation-equality, T-09-01, T-09-02, T-09-03]
status: complete

requires:
  - phase: 09-registered-evidence-and-gate-cost
    provides: "plan 09-01 start state: steps=38/38 controls=380/380 faults=235/235 gates=90 lines=306 goldens=29 reports=32 host-dependent=3 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 claims=28 provisional=27, and no `release-only` mode anywhere in the tree (grep -rn release-only tests/ data/ tools/check/steps/ returned nothing)"
provides:
  - "data/expected/runner-scenarios.ids: demo-lamp-tone-swap and demo-lamp-tone-shift are column-5 `release-only`; a new mode that runs on Release and is judged statically on the sanitized preset"
  - "tests/runner-process-tap.sh: static_scenario/derivation_equal, the derivation-equality judgement (the variant's assert block equals the block between the asserts-begin/asserts-end markers of docs/video/demo-lamp-tone-1-derivation.md, minus fence lines and leading # lines), with an in-case negative half that runs on every gate; the `# mutants:` line gains variants= and variant-static="
  - "tools/check/steps/golden.sh: a `release-only` id is skipped by name by the dev-versus-release judge and counted in release-only=2; the missing-report refusal is untouched for every other id"
  - "trim (a): above-max, no-value and twice of runner-process.roundtrip.refuse-range are static judgements against runner/main.c:589-592, :609-610, :613-621 and :623-624; the zero launch stays live and still proves exit 14, the empty report, the first stderr line and the usage text"
affects: [09-02, 09-03]

actuals:
  tasks: 3
  commits: 3
plan_head_before: 912ab9a
plan_head_after: f271769

key-decisions:
  - "D-09-01: tools/check/steps/golden.sh joins wave A1's closed file list. Its dev-versus-release judge refuses `golden-dev-release-diff` for any id with one missing report (golden.sh:402-406), and a `release-only` scenario has no dev report. The fix is a skip by name plus a `release-only=` count, not a loosening: the `[ ! -f dv ] || [ ! -f rl ]` refusal is untouched for every other id. Measured effect: reports=32 -> 30, release-only=2."
  - "D-09-02: the frozen wording 'the option table in runner/exec.c' is unsatisfiable - runner/exec.c contains neither `roundtrip-at-ticks` nor 'option without a value', and no option table exists anywhere; the parser is a strcmp chain at runner/main.c:588-624. The three judgements name the real guard sites instead (A-03/A-04 already removed the new-test-id requirement)."
  - "A-04: the variant's `id` line carries its own id. The frozen sentence asked it to equal the pass scenario's, which can only pass by deleting the line."
  - "A-03: no test id was added. The derivation-equality judgement ships inside the existing variant case driven by mutant_case's two-mode switch, with a committed in-case negative half (a one-value change) that runs on every gate - a stronger stand-in than a scratch file, because it cannot rot."
  - "`release-only` prints an ordinary `ok` line, never a skip: tests-skipped stays 4. A skip would have moved it to 8 and staled every recorded token (Pitfall 3)."

key-files:
  created:
    - .planning/phases/09-registered-evidence-and-gate-cost/09-01-SUMMARY.md
  modified:
    - data/expected/runner-scenarios.ids
    - data/expected/tests/runner-process.ids
    - tests/runner-process-tap.sh
    - tools/check/steps/golden.sh

requirements-completed: []  # EV-01 closes on 09-03's measurement, not here

completed: 2026-11-24
---

# Phase 9 Plan 01: the static-variant fold and trim (a) Summary

**The last non-owner cost lever in the pre-committed order is landed. The two
Lamp Tone variants (`demo-lamp-tone-swap` and `demo-lamp-tone-shift`) no longer
launch `ivs-run` on the sanitized preset: they are column-5 `release-only`, so
they run on Release and are judged statically on the dev preset by an equality
against the frozen derivation `docs/video/demo-lamp-tone-1-derivation.md` —
their `assert` blocks must equal the block between the `asserts-begin` and
`asserts-end` markers, minus fence lines and leading `#` lines, with the
`content`/`profile`/`ivss` lines equal to the pass scenario's and the `id` line
carrying their own id. The judgement is not an assertion of good faith: the same
case runs a one-value-changed variant in a subshell and requires it to be
rejected, on every gate run, so the guard cannot rot. Trim (a) makes three of the
four round-trip refusal launches static against the parser's real guard sites
(`runner/main.c:589-592`, `:609-610`, `:613-621`, `:623-624`) and keeps the
`zero` launch live, because it alone proves exit 14, the empty report, the first
stderr line and the usage text. `tools/check/steps/golden.sh` gained a skip by
name and a `release-only=` count for the one measured reason it had to move
(D-09-01). No test id, control, fault, gate step or registry line was added, and
`tests-skipped` did not move: `sh`, `dash` and the Ubuntu container all read
`gate: PASS` with the plan's predicted token, byte for byte.**

## Commits

| Commit | Subject | Files |
|---|---|---|
| `8d28dce` | `fix(09): fold the two lamp-tone variants into release-only rows with a derivation judgement` | `data/expected/runner-scenarios.ids`, `data/expected/tests/runner-process.ids`, `tests/runner-process-tap.sh`, `tools/check/steps/golden.sh` |
| `dcb50f4` | `fix(09): judge the three round-trip refusals against the parser's guard sites` | `tests/runner-process-tap.sh` |
| `f271769` | `fix(09): make the mode of every runner-process run visible with a count` | `tests/runner-process-tap.sh`, `data/expected/tests/runner-process.ids` |

## The gate, re-measured on the final tree

```
sh   tools/gate.sh : gate: PASS steps=38/38 controls=380/380 faults=235/235
                     gates=90 lines=306 goldens=29 reports=30 host-dependent=3
                     release-only=2 tests-expected=4958 tests-executed=4954
                     tests-skipped=4 tests-missing=0 claims=28 provisional=27
dash tools/gate.sh : gate: PASS, every one of those tokens identical
sh   tools/gate-linux.sh : gate-linux: exit 0, gate: PASS in build/gate-linux.log
                     (cc=GNU, gate-time total=492; log copied to
                     /tmp/gate-linux-09-01.log before anything else ran; no
                     ivs- container left behind)
```

The two moved tokens are exactly the predicted pair: `reports=32 → 30` and
`release-only=2`. `tests-skipped` is 4 before and after, which is the check that
the fold is a fold and not a skip.

`files=2516`, `tracked=2516`, `hygiene-files=2516`, `ci-runs=77` and the timing
lines are derived at run time and are recorded here as observations, never
asserted (A-05 and the plan's token-discipline block).

## The mode is visible in the output

```
build/gate/dev/runner-process.txt     : # mutants: configuration=Debug   full=0  static=35 variants=0 variant-static=2
build/gate/release/runner-process.txt : # mutants: configuration=Release full=35 static=0  variants=2 variant-static=0
```

The two fields sum to the number of `release-only` rows, so the mode of any run
is read from the output rather than inferred from which preset was built.

## Cost

The lever's measured ceiling is **0.12 local cpu-s**, and it is quoted as a
ceiling and not as a saving: it cannot move `r`. Two `ivs-run` dev launches of
the Lamp Tone scenarios were removed (median of five, dev 62 ms, release
9.4–9.8 ms), and trim (a) removes three more dev launches of the refusal option
(about 0.06 local cpu-s each, recorded 0.18 together).

**Open item — the alternated `/usr/bin/time -p` before/after capture was not
completed.** The executor's session hit the 1800 s dispatch limit after its
third commit, before it could take the three alternated runs per preset. What is
recorded instead is measured, not assumed: the runner-process suite reads 21.55 s
on the dev preset and 22.69 s on release on this tree, both green, and the
per-launch costs above are the recorded figures the ceiling was computed from.
Plan 09-03 is the plan that consolidates the A1 measurement, so the before/after
table is its first task and this item is handed to it rather than closed here.

## Deviations

1. **D-09-01** — `tools/check/steps/golden.sh` is in wave A1's file list, which
   the frozen scope sentence did not name. The reason is measured
   (`golden.sh:402-406` refuses a missing report; a `release-only` id has none),
   and the change is a skip by name plus a count, not a loosening.
2. **D-09-02** — the frozen "option table in `runner/exec.c`" does not exist; the
   three judgements name `runner/main.c:589-592`, `:609-610`, `:613-621` and
   `:623-624` instead.
3. **A-04** — the variant's `id` line carries its own id rather than the pass
   scenario's.

## What this does not do

It does not add a test id, a control, a fault, a gate step or a registry line; it
does not change the core, the runner, a golden or the frozen derivation (the
derivation is read, never written — `git show --stat` on every commit above lists
no path under `core/`, `data/golden/` or `docs/video/`); it does not claim a
hosted saving, because the lever cannot move the metric and the hosted row that
settles the level is the orchestrator's, not this plan's.
