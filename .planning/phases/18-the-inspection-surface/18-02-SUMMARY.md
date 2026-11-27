---
phase: 18-the-inspection-surface
plan: 02
subsystem: inspection-surface
tags: [build-only-tool, trace-reconciliation, breakpoints, refusal-tokens, rights-artifact, registry-rows, requirement-flip]

requires:
  - phase: 18-the-inspection-surface
    plan: 01
    provides: "`docs/inspection.md` as the frozen contract: four subcommands, three record kinds, five reconciliation equalities, five refusal tokens, four liveness guards, and the disassembly limb for `18-03`"
  - phase: 15
    provides: "the build-only non-member pattern (`host-so-absent`) that the `pkg-install` proof is copied from"
  - phase: 16
    provides: "the `files-unlisted-binary-*` control shape and the `release-posture` registry rows the `files` row sits beside"
  - phase: 17
    provides: "the count-gate contract (D-56): every count gate names the file it counts and every baseline names its start commit"

provides:
  - "`tools/inspect/` and the `ivs_inspect` target: a build-only, scripted, headless inspection surface under both presets, with no install rule anywhere"
  - "the report as the library's own bytes and the trace as the tool's own per-unit loop, reconciled on the frozen page's five equalities"
  - "thirteen `runner-process` ids, including four committed stand-ins that each fail for one declared token"
  - "breakpoints asserted as a hit/non-hit pair plus a non-interference equality, and the `step` surface reconciled against the trace's own units"
  - "the `ivst` rights row in three places, `ivst` in the two name lists, and the `files-unlisted-binary-ivst` control"
  - "the two build-only proofs (`inspect-absent`, `inspect-unlisted-member`), the not-installed lines in `docs/packaging.md`, and five `important-gates` rows"
  - "TOOL-02 and TOOL-09 `Complete`, with the toy tree closed by its appended Phase 18 pair"

affects:
  - "Phase 19: the discrimination of the display-path default now has an instrument whose trace is provably the run its report describes"
  - "`18-03`: the disassembly limb adds `dis` and its own ids to the same binary and the same refusal-token block"
  - "the milestone close: the priced addition is computed at 3.59 against the frozen 2.96, 21 percent over, inside the 25 percent repricing threshold"

tech-stack:
  added: []
  patterns:
    - "an oracle that is the bytes the library handed the tool, never a struct the tool could reshape: the report text `write_report` captured is the only view the tool has, so a second truth cannot be constructed"
    - "two passes reconciled by the public nine-domain digest, so 'the trace is the run the report describes' is a byte equality rather than a claim"
    - "a build-only target proved absent in both directions: refused in a copy of the installed prefix by the same `check_inventory` the real prefix is judged by, and refused as an unlisted archive member by the existing extra-member limb, with a negative limb showing the extension-only rule cannot see it"
    - "toy-tree lines appended, never inserted, because an overlay control pins a line number"

key-files:
  created:
    - tools/inspect/CMakeLists.txt
    - tools/inspect/inspect.c
    - tools/inspect/inspect_trace.c
    - data/rights/inspection-output.json
    - tools/check/controls/files-unlisted-binary-ivst/CHECK
    - tools/check/controls/files-unlisted-binary-ivst/EXPECT
    - tools/check/controls/files-unlisted-binary-ivst/MAKE
    - tools/check/controls/_good/tree/tools/inspect/good.c
    - .planning/phases/18-the-inspection-surface/18-02-SUMMARY.md
  modified:
    - CMakeLists.txt, tools/check/integrity.cmake, tools/check/steps/c-build.sh
    - tools/check/controls/_good/tree/CMakeLists.txt
    - tests/CMakeLists.txt, tests/runner-process-tap.sh, data/expected/tests/runner-process.ids
    - data/expected/rights.ids, docs/RIGHTS.md, tools/check/binary-ext.txt, .gitignore, data/expected/controls/rights.ids
    - README.md, docs/packaging.md
    - tools/check/steps/pkg-install.sh, data/expected/pkg-selftests.ids
    - tools/check/steps/release-assets.sh, data/expected/release-assets-selftests.ids
    - data/expected/important-gates.ids, tools/check/steps/important-gates.sh
    - .planning/REQUIREMENTS.md and the thirteen toy files

key-decisions:
  - "The oracle is the captured report text, not the record: `ivs_runner_execute` returns no digests and the record is library-internal, so the nine `digest.*` values and every `step.<k>.now_ticks` are read out of the bytes the library handed `write_report`. This is also the stronger reading of the frozen page."
  - "A1 was measured, not assumed: on `diag-boot-pass` the traced instance's nine `ivs_machine_state_digest` values equal the nine the report printed, byte for byte (2084 ticks, 112 access records, 70 units), so the central equality rests on a measurement and §11's tick-only fallback was not taken."
  - "A breakpoint hit is a `fetch` OR an `iab`: the reset entry is an IAB carrying the reset flag, so a rule naming only `fetch` reads the entry as never executed. The measured pair is `(1000 -> 1, 1002 -> 0)`."
  - "The four stand-ins are committed cases in the suite, judged by the same function the real case uses, not throwaway probes: a check that accepts its own stand-in proves nothing."
  - "The `release-assets` case is the phase's ONE case and it replaces the `.ivst` archive case rather than joining it: the `.ivst` refusal is already proved by the `files-unlisted-binary-ivst` control, so an archive case for it would re-prove the extension rule; the archive case is about the extensionless binary, which is what criterion 5 names."
  - "The tool is build-only in both directions and the install inventory is untouched at eleven paths: the absence is a check because the inventory is a count read from a named file (D-56)."
  - "TOOL-02's flip does not depend on a droppable plan: the disassembly limb is `18-03`, and a drop is recorded in the frozen page and in `18-03`'s record with the reduced surface named."

requirements-completed: [TOOL-02, TOOL-09]

actuals:
  tokens: 41857
  tasks: 3
  commits: 3
  plan_head_before: 48470fa3857e949e74965f1bf6c184f4c91a87c8
  plan_head_after: d8eb593

# Metrics
duration: 54min
completed: 2026-11-26
status: complete
---

# Phase 18 Plan 02: The Instrument, and the Trace That Is Provably the Run Its Report Describes Summary

**One build-only `ivs_inspect` whose report is the library's own bytes and whose trace is the tool's own
per-unit loop, tied together by the nine public digests — with thirteen `runner-process` ids, four committed
stand-ins that each fail for one declared token, breakpoints asserted as a `(1, 0)` pair, the `ivst` rights row
in three places, the tool proved absent from the installed prefix and refusable as an archive member, and
TOOL-02 and TOOL-09 closed by the five registry rows that cover them.**

## Baselines

start-commit: 48470fa3857e949e74965f1bf6c184f4c91a87c8

Measured at this plan's start commit, before the first edit. The line below is the one `18-01`'s final run
measured at that exact commit (`18-01-SUMMARY.md`, `## Baselines`), with `docs=65 doc-files=64` because `18-01`
added `docs/inspection.md`; the tree at `48470fa` is that commit and nothing else. Every figure this plan's
delta table needs reads as predicted:

```
baseline (48470fa): tests-expected=5044 tests-executed=5040 tests-skipped=4 tests-missing=0
  controls=404/404 steps=38/38 faults=238/238 goldens=29 gates=120 lines=366
  pkg-selftest=20/20 release-selftest=12/12 prefix-files=11 rights=10 rights-cleared=1
  claims=29 provisional=28 claim-refs=27 docs=65 doc-files=64 listings=7
  tracked=2821 files=2821 hygiene-files=2821
```

Counts read directly from the files they come from at `48470fa`, before the first edit: `runner-process.ids`
150 data lines, `important-gates.ids` 120 distinct gate names over 366 data lines, `pkg-selftests.ids` 20,
`release-assets-selftests.ids` 12, `rights.ids` 10, `ls -d tools/check/controls/*/ | grep -v _good | wc -l` 404,
`ls tools/check/steps/*.sh | wc -l` 38.

**A pre-edit gate run of this dispatch was contaminated and is discarded** (deviation 3 below): it was started
and then overtaken by the first edit, so `pkg-install` read a half-edited script and the run printed
`steps=37/38` with no `pkg-selftest` token. It is not used as a baseline anywhere in this record.

## The measured delta, task by task

Every value below is measured, never quoted. The `after T1` and `after T2` columns are the lines those commits
recorded in their own messages; the `after T3` column is the line `sh tools/gate.sh` printed on the committed
tree, byte-identical under `dash`.

| token | before (48470fa) | after T1 (5ed5610) | after T2 (e876c93) | after T3 (d8eb593) | why |
|---|---|---|---|---|---|
| `tests-expected` | 5044 | 5052 | 5070 | **5070** | four ids, then nine more, each counted across both presets |
| `tests-executed` | 5040 | 5048 | 5066 | **5066** | no new id is skip-ok |
| `controls` | 404/404 | 405/405 | 405/405 | **405/405** | the one `files-unlisted-binary-ivst` control directory |
| `rights` | 10 | 11 | 11 | **11** | one `owner-local-only` row; `rights-cleared` stays 1 |
| `gates` | 120 | 120 | 120 | **121** | the control row introduces the gate name `files`; the four mutant rows reuse `runner-process` |
| `lines` | 366 | 366 | 366 | **371** | five registry rows |
| `pkg-selftest` | 20/20 | 20/20 | 20/20 | **21/21** | the `inspect-absent` case |
| `release-selftest` | 12/12 | 12/12 | 12/12 | **13/13** | the `inspect-unlisted-member` case, which replaces the `.ivst` archive case rather than joining it |
| `steps` | 38/38 | 38/38 | 38/38 | **38/38** | no new file under `tools/check/steps/` |
| `faults`, `goldens` | 238/238, 29 | same | same | **same** | invariants; `tools/inspect` is outside the named-fault framework |
| `claims`, `provisional`, `claim-refs` | 29, 28, 27 | same | same | **same** | this phase mints no claim |
| `prefix-files` | 11 | 11 | 11 | **11** | no install rule, and the inventory is what makes the absence a check |
| `docs`, `doc-files` | 65, 64 | same | same | **same** | `18-01` added the only new page |
| `tracked`/`files`/`hygiene-files` | 2821 | 2830 | 2830 | **2830** | eight new files in T1 (three tool files, the rights row, three control files, the toy source) and one in T2 (`.agents/skills/ops-hygiene/SKILL.md`); T3 adds none, and this record adds the tenth, so the count at the record's own commit is **2831** |

All twenty-one predicted tokens matched the plan's Task 3 verify list exactly, and nothing else moved. No
`.ivst` is tracked (`git ls-files | grep -E '\.ivst$'` is empty) and `sh tools/hygiene.sh --tree` passes with
no finding. The gate was run again at this record's own commit and is green with `tracked=2831
hygiene-files=2831 files=2831` and every other token unmoved — the only difference from the `d8eb593` line is
this file, which is what the delta table's `tracked` row predicts.

## What the three commits ship

**T1 — `5ed5610`, the tracer.** `tools/inspect/CMakeLists.txt` builds `ivs_inspect` → `ivs-inspect` under both
presets, linking `ivs_runner_lib ivs_core ivs_core_internal ivs_warnings ivs_san`, with no `install(` anywhere
and the source list written as `ivs-inspect-sources.txt`. The two trapdoors close in the same commit: `inspect`
joins the `tools/(...)` SPDX alternation in `tools/check/integrity.cmake`, the `c-build` module list gains the
source list, and the toy tree writes it and carries `tools/inspect/good.c`. Pass A parses the scenario and runs
`ivs_runner_execute` through the tool's own host, whose `write_report` captures the library's bytes; pass B
drives its own instance through `ivs_machine_step_unit_trace_internal` with the core's own input floor and its
own halt and fault breaks; the reconciliation is the page's five equalities. Four ids land, plus the `ivst`
rights row in three places, `ivst` in the body of `tools/check/binary-ext.txt` and in `.gitignore`, and the
`files-unlisted-binary-ivst` control with its inventory line.

**T2 — `e876c93`, breakpoints, `step`, and the stand-ins.** `--break`/`--other` arm a matcher whose two counts go
into the end record, so the in-run count and the file-derived `break` report are two independent computations
of the same number; the measured pair on `diag-boot-pass` is `(1000 → 1, 1002 → 0)`. `break --pc --other --trace
--out` writes the page's seven keys, steps no machine, and copies the trace's own record counts so the two
documents are compared rather than trusted. `step` shares one instance setup with the trace pass and reports
through `ivs_machine_cpu_state` and `ivs_machine_peek` only. Nine ids land, including the four stand-ins.

**T3 — `d8eb593`, the two build-only proofs and the flips.** `pkg-install` gains `inspect-absent` (a planted
`bin/ivs-inspect` in a copy of the prefix must be refused as `pkg-inventory-extra: bin/ivs-inspect` by the same
`check_inventory` the real prefix is judged by); `release-assets` gains `inspect-unlisted-member` (an
extensionless `bin/ivs-inspect` the list does not name, refused through the existing extra-member limb, with a
negative limb requiring neither `release-assets-binary-member` nor `release-assets-missing-member` to fire and
the reason string `the extension rule saw the extensionless member, or a listed path went missing`).
`docs/packaging.md` names `ivs-inspect` and `tools/inspect` in both enumerations. Five registry rows and the
Phase 18 block land with both requirement flips, and the thirteen toy files gain their appended Phase 18 pair.

## The four stand-ins, and how each was proved to fail

Each is a committed case in `tests/runner-process-tap.sh`, built at run time from committed material, and each
is judged by the **same** function the real case uses (`ix_records`, `ix_tick_domain`, `ix_digests_equal`,
`ix_break_pair`). A judgement that accepted its own stand-in reports `judge-vacuous`, which is the liveness
guard on the guard.

| stand-in | what it breaks | the clause that must fail | how it was proved to fail |
|---|---|---|---|
| `judge.trace-empty` | a trace with an end record and zero access records | `records > 0` in `ix_records` | the stand-in file differs from a real trace in the record count alone, so `ix_records` refuses it for exactly `trace-records` |
| `judge.trace-repeat-tick` | the real trace with every access tick set to the first record's value | the fifth equality's `not all equal` clause in `ix_tick_domain` | a constant column IS non-decreasing, so a monotonicity-only check would accept it and report `judge-vacuous`; `ix_tick_domain`'s distinct-count limb is the only clause that can refuse it |
| `judge.trace-pokes-register` | the real report with `digest.cpu` replaced by `digest.mem`'s own value | `ix_digests_equal` (for `digest-differs`), then the tool's own `--report` reconciliation | the digest comparison refuses it, and the poked report is then handed to `ivs-inspect trace --report`, which must exit 1 with `ivs-inspect: inspect-reconcile: `, print nothing on stdout and leave no output file |
| `judge.break-always-hits` | the real break report with `break.other.hits` set to 1 | the pair's second number in `ix_break_pair` | it reports 1 where the answer is 0, so the pair fails on the same clause the real `break-pair` case asserts |

Each stand-in declares exactly one finding token, and its `important-gates` row landed in T3 (see the §2.7 split
below).

## Deviations from Plan

Sixteen, each recorded rather than hidden. Items 1–5 are T1's, 6–11 are T2's, 12–15 are T3's; all sixteen stand
as taken.

### Auto-fixed / recorded from Task 1

1. **[Rule 2 — the page wins over the plan's sentence]** `write_report` captures the library's bytes and prints
   nothing; `inspect.c` writes the captured report to stdout only after every check has passed. The plan said
   `write_report` "forwards the library's bytes to stdout and appends them to a capture buffer". The page's exit
   status 1 requires "exactly one stderr line … nothing on stdout" for a named refusal, and a reconciliation
   refusal happens after the report pass, so a report already on stdout would break the page. The page wins.
2. **[Rule 2 — the page wins]** A short write on the output removes what was created and exits 2 with no token.
   The plan says exit 1; the page puts "a file error" at exit 2 with no token, and a write error is a file
   error.
3. **[Recorded reading]** The limit of a run-to is the converted target, capped by the executor's default
   machine ceiling. The plan asks for "the same rule as `ivs_bound_limit`" including `ivs_stic_next_frame_end`
   when the step's unit is frames; that branch is keyed on `IVS_RUN_F_STOP_ON_FRAME`, which the executor never
   sets (it always runs with `IVS_RUN_F_STOP_ON_HALT` only), so with the same options the same rule is the
   target. A step whose unit is frames is converted through `nominal_frame_ticks`, which is the part of that
   sentence that does apply.
4. **[Wave boundary]** `break`, `dis` and `step` are the page's other three subcommands and are named by
   `--help`; T1 implemented `trace` only, so their names exited 2 with no token until T2 landed them. A
   well-formed `--break`/`--other` pair was parsed and then refused with exit 2 rather than silently ignored.
5. **[Recorded, as `18-01` recorded it]** The commit lands on `main`. `.planning/config.json` sets
   `git.branching_strategy: none` and `workflow.use_worktrees: false`, so the sequential run is on the current
   branch, which is `main` for every phase commit of this milestone.

### Recorded from Task 2

6. **[Recorded — the §2.7 split]** `GUARDRAILS-v0.4` §2.7 asks a new check's must-fail registry row, its
   liveness guard and its committed stand-in to land in one commit. Here they could not: the four stand-ins and
   their guards landed in T2's commit and their four `important-gates` mutant rows land in T3's, because the
   registry step derives its accepted set from the traceability table and refuses a row naming a `Pending`
   requirement — and TOOL-02 is `Pending` until T3 flips it. The derived-accepted-set rule (§2.14) is the
   reason, and it is the one rule in this plan that a task could not satisfy by doing more work.
7. **[Recorded — dispatch scope]** T2's dispatch was scoped to T2 only and the SUMMARY is written by T3's
   dispatch, so the SUMMARY section and T2's four `start-commit:` limbs were deferred with it; every other limb
   of T2's verify ran and passed under `set -e`.
8. **[Recorded reading]** The frozen page states the output policy for "an output" and not for one subcommand's
   output, so `break --out` goes through the same rule as `trace --out`: the physical tree check, then
   existence, then the exclusive create, in that order. The policy is one function (`ix_open_output`), not two
   copies.
9. **[Recorded reading]** `step --units <n>` steps at most `n` units and stops early on the core's own fault and
   halt breaks, exactly as the trace pass does, so its unit count is the trace's own `units` when it is given
   that number — which is what the `inspect.step` id reconciles. A `step` that ignored the breaks would step
   past a halted CPU and disagree.
10. **[Recorded — token-set extraction]** The coverage id compares the tool's token set with the page's block in
    both directions with `inspect-dis-row` excluded from both sides, so plan `18-03` cannot turn this wave-2 id
    red across a wave boundary. The four tokens the tool prints today are `inspect-break-empty`,
    `inspect-output-exists`, `inspect-output-tree` and `inspect-reconcile`, each a standalone double-quoted
    literal so the `grep -h -o -E '"inspect-[a-z-]+"'` extraction can see it.
11. **[Not run]** The container gate at T2's commit was `not run`: the docker daemon did not answer. The local
    gate ran under both `sh` and `dash` and the two summary lines were byte-identical.

### Recorded from Task 3

12. **[Recorded — the §2.7 split, from the other side]** The four stand-ins' guards landed in `e876c93` and their
    four registry rows land here, in `d8eb593`. §2.7's one-commit rule is satisfied across two commits and the
    derived-accepted-set rule is what forces it; the rows and both requirement flips are one commit, because
    this step refuses a row naming a `Pending` requirement **and** refuses a `Complete` requirement with no
    row, so the two directions move together or not at all.
13. **[Recorded — the dispatch's predicted token line was wrong, and the measurement wins]** The dispatch's
    context said `tests-expected=5082`, `tests-executed=5078`, `gates=122`, `lines=372`. The plan's delta table
    and this task's verify block predict `5070`, `5066`, `121`, `371`, and the measured line is the plan's.
    The arithmetic is in the plan and is checkable from the files: T3 adds no test id, so thirteen ids across
    two presets is `5044 + 26 = 5070`; and of the five registry rows exactly one introduces a new gate name
    (`files`), so `gates` moves 120 → 121 and `lines` 366 → 371. Reaching the dispatch's figures would have
    required inventing six test ids and a sixth registry row — a re-bless, not a measurement — so the measured
    line is recorded and the dispatch's figure is recorded here as the deviation.
14. **[Recorded — lessons 18 and 24]** A pre-edit baseline gate run was started and then overtaken by the first
    edit, so `pkg-install` read a half-edited script (`steps=37/38`, no `pkg-selftest` token). That line is
    discarded rather than used as a baseline. The container gate is `not run` with the reason in
    `build/gate-linux.log`: the docker daemon does not answer (the client answers, the server section never
    arrives; `docker version` hit a 15 s bound, and the one mandated `sh tools/gate-linux.sh` retry hit a 300 s
    bound and exited 124). The prior log was copied aside to `/tmp/gate-linux-before-t3.log` before the attempt,
    no container, image or volume was created, and no `gate: PASS` line is claimed for the container.
15. **[Recorded reading]** The `release-assets` case is the phase's one case and it **replaces** the `.ivst`
    archive case rather than joining it, so `release-selftest` moves 12/12 → 13/13 and no other delta row moves.
    The `.ivst` refusal is already proved by the `files-unlisted-binary-ivst` control that landed with T1, and
    an archive case for it would re-prove the extension rule the control proves; the archive case is about the
    extensionless binary, which is what criterion 5 names.
16. **[Recorded, as `18-01` and both earlier tasks recorded it]** The commit lands on `main`, for the reason in
    item 5.

## The §2.7 split, stated once

`GUARDRAILS-v0.4` §2.7 requires three things in one commit: a must-fail mechanism registered in
`data/expected/important-gates.ids`, a liveness guard, and a committed stand-in showing the guard fail. This
plan could not satisfy it in one commit for the four stand-ins, and the reason is mechanical, not editorial: the
`important-gates` step **derives** its accepted set from the traceability table in `.planning/REQUIREMENTS.md`
(§2.14, amendment A-01) and refuses a row naming a requirement that is not `Complete`. TOOL-02 is `Pending`
until the commit that closes it, so a row naming TOOL-02 before that commit would fail the step with
`important-gates-unknown-id`, and a `Complete` requirement with no row fails it with
`important-gates-complete-uncovered`. The only way to satisfy §2.7 in one commit would be to flip a requirement
before its evidence landed — which is the failure the two-direction rule exists to prevent.

So: the four stand-ins and their guards landed in `e876c93` (T2), and their four `mutant` rows landed in
`d8eb593` (T3) with the two flips, in one commit, with the toy tree's appended pair. Both commits are green under
both shells, and the registry rows point at committed ids that the gate re-runs on every run, so the mechanism
is registered and live from the moment the requirement is closed. This is recorded as a deviation in both
commits' messages (T2 item 1, T3 item 12) rather than left to a reader to notice.

## The cost line, measured where it could be

The plan computes the phase's priced addition at **3.59 hosted cpu-s** against the frozen 2.96 — 21 percent over,
inside §2.4's 25 percent repricing threshold — from the frozen unit prices (a process-launch id 0.10, a control
0.31, an in-step selftest 0.05, a registry row 0.005): 13 `runner-process` ids × 0.10 = 1.30, 4 stand-ins × 0.31
= 1.24, one control directory 0.31, two in-step selftests 0.10, six registry rows 0.030, one rights row 0.005.
The frozen 2.96 is **not** edited; the difference is carried to the milestone close as a repricing candidate.

The local measurement beside that arithmetic is partial and is stated as such rather than dressed up: one
`sh tools/gate.sh` on the finished tree measured `gate-time total=67` with `user 2m07.5s` and `sys 1m56.0s`, and
the two steps this task touched measured `step-time pkg-install 4` and `step-time release-assets 0` seconds.
The alternated before/after median §2.4's recipe asks for was **not** taken in this dispatch, because the
pre-edit baseline run was contaminated (deviation 14) and T2's timing was not preserved; a unit price is not a
measurement, and neither is a single run. `18-01` measured `gate-time total=131` at its own commit on a busier
host, which is the only other local figure this phase has and is not comparable.

## Gate evidence

```
sh    tools/gate.sh  -> gate: PASS steps=38/38 controls=405/405 ... gates=121 lines=371 ... pkg-selftest=21/21 ... release-selftest=13/13 ... tests-expected=5070 tests-executed=5066 prefix-files=11
dash  tools/gate.sh  -> the same line, byte-identical (cmp -s on the last line: identical)
sh    tools/hygiene.sh --tree -> -- hygiene pass mode=tree hygiene-files=2830
sh    tools/gate-linux.sh -> not run: the docker daemon does not answer (one bounded retry, exit 124); the visible
                             line is in build/gate-linux.log and no PASS is claimed
```

Zero `-- FAIL` lines under either shell. `git ls-files | grep -E '\.ivst$'` is empty.

## Known Stubs

None. No file this plan created or modified carries a hardcoded empty value that flows to an output, a
placeholder, a TODO or a FIXME, and no component is wired to a source that does not exist. `18-03`'s `dis`
subcommand is not a stub of this plan: the frozen page names it, `--help` lists it, and this plan's coverage id
excludes `inspect-dis-row` from both sides of the token comparison precisely so the missing subcommand is a
recorded wave boundary rather than a silent placeholder.

## Threat Flags

None beyond the plan's own register. The three threat rows this plan mitigates (T-18-03 information disclosure,
T-18-04 tampering with the report writer, T-18-06 tampering with a build-only tool's membership) each have their
mechanism in this commit set, and no file added here opens a trust boundary the `<threat_model>` did not name.
No package manager ran (T-18-SC: this plan writes C, CMake, shell, JSON and inventory files only).

## Self-Check: PASSED

Checked against the tree, not against this record's own prose:

- Every file this plan created exists: `tools/inspect/CMakeLists.txt`, `tools/inspect/inspect.c`,
  `tools/inspect/inspect_trace.c`, `data/rights/inspection-output.json`, the three
  `tools/check/controls/files-unlisted-binary-ivst/` files and
  `tools/check/controls/_good/tree/tools/inspect/good.c` — all FOUND.
- Every commit named here is an ancestor of `HEAD`: `48470fa` (start commit), `5ed5610`, `e876c93`, `d8eb593`
  — all FOUND via `git merge-base --is-ancestor`.
- The `start-commit:` line is a 40-hex sha, resolves with `git cat-file -e`, is an ancestor of `HEAD`, and is
  present as its own line in this file — the four limbs every task's verify reads back.
- No `.ivst` is tracked and `git status --short` is empty apart from this record's own commit.
