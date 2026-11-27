---
phase: 19-discriminating-the-display-path
plan: 02
subsystem: display-path-probe-containment
tags: [owner-local-probe, refs-only, bounded, skip-ok, missing-fixture, containment-cases, d11]

requires:
  - phase: 19-discriminating-the-display-path
    plan: 01
    provides: "the four authored guests and the three parameter-driven branches; this plan adds nothing to them and reads none of them"
  - phase: 07
    provides: "`tools/owner-run.sh` and its nine `runner-process.owner-run.*` cases: the CI refusal, the physical-directory comparison, the exact-size checks and the `SKIPPED` plus exit 77 are that helper's habits, copied rather than reinvented"
  - phase: 14
    provides: "`tools/flatwrap/flatwrap.c` `fw_inside_tree` and its `-1` answer: the output-path rule this probe's output check copies verbatim, including the cannot-resolve refusal"

provides:
  - "`tools/disc-sweep.sh`: the bounded, CI-refusing, `refs/`-only owner-local sweep probe, with six refusal limbs, its own skip (exit 77) and a table of offsets and observations"
  - "`data/scenarios/disc-sweep-probe.ivss` and one six-column row plus one trailing `skip-ok` id line: the probe's visible skip in both inventories, declaring exit 13 and `missing-fixture`"
  - "three `runner-process.disc-sweep.*` cases that judge the containment on both presets, on a copy of the helper in a scratch root outside the work tree, with project-made zero-filled files only"
  - "one `docs/owner-images.md` section naming the probe beside the existing owner-local route, and stating what its output may never be"

affects:
  - "VID-07: unchanged and still `Pending`; this plan is criterion 5's containment artifact and it flips nothing, so plan 19-03's registry-row commit is untouched by it"
  - "the phase's criteria 1 to 4: provably independent - no id, verify block or record of this phase reads the probe or its output, and the probe's absence is the expected gate state"
  - "the milestone close: the priced addition is unchanged at 0.40 of the phase's 1.64; the measured probe cost is below it, so no reprice is owed"

tech-stack:
  added: []
  patterns:
    - "a probe whose input is operator content is contained by making its refusals gate cases and its sweep no case at all: the gate judges the containment on every run and never the observation"
    - "the output-path rule is the flatwrap rule and not a similar one - the physical parent is resolved with a physical cd and compared with the physical work-tree root, so a literal path, a `..` spelling and a symlink to a directory inside the tree get one answer, and an unresolvable parent is a refusal rather than a blind write"
    - "a skip in two halves with two different codes: the helper's own exit 77 for a bare invocation with no image, and the scenario's exit 13 `missing-fixture` for the suite case, because the suite can only turn a PASSING judgement into a visible skip"
    - "the probe's observation is given a negative control inside its own fixed offset list: the list straddles the end of frame 0, so a bound below it must record `-` and a bound above it records the displayed word"

key-files:
  created:
    - tools/disc-sweep.sh
    - data/scenarios/disc-sweep-probe.ivss
    - .planning/phases/19-discriminating-the-display-path/19-02-SUMMARY.md
  modified:
    - tests/runner-process-tap.sh
    - data/expected/tests/runner-process.ids, data/expected/runner-scenarios.ids
    - docs/owner-images.md

key-decisions:
  - "The two tasks land in ONE commit, because the probe scenario must exist for the containment cases and the `golden` step refuses a user-content scenario without its `skip-ok` row: separating them would leave a red gate between the commits, which lesson 1 forbids. Task 1 step (5) authorises the collapse."
  - "The absent-images skip sits between the run-count bound and the missing-runner check, not before the path checks: the output-path rule must be reported as a path defect and never as a missing image, and a bare invocation with no image must skip even when no runner is built - which is what the plan's own verify block exercises."
  - "The offset is the CPU-cycle bound of one run, from a fixed list straddling frame 0's end at 14,934 cycles (59,736 ticks at 4 ticks per cycle on the NTSC family profile). The temporary copy also carries `capture video 0 1`, because a frame assert reads a capture and the capture is in memory: a bound below the boundary records `-` and a bound above records the displayed word, so the table has a negative half."
  - "`frame 0 displayed` is read from the assert line's `got` field, which an assert prints whether it passes, fails or never ran; that is the only place the report carries the displayed word for a scenario that names no output file, and a user-content scenario is refused every frame output option."
  - "No `content cart` line in the probe scenario, and the file's header says why: a loadable program would let the runner boot instead of refusing, which would destroy the declared `missing-fixture`/13 outcome the row exists to be."

actuals:
  tokens: 0
  tasks: 2
  commits: 0
  plan_head_before: 19b1e7a2a4a4d08c968b481592709f3ef60c7abd
  plan_head_after: 0

# Metrics
duration: 0min
completed: 2026-11-27
status: complete
---

# Phase 19 Plan 02: Containing the Owner-Local Sweep as a Throwaway Probe Summary

**One POSIX shell probe that can only write under `refs/`, refuses CI, is bounded three ways, and is a visible skip
in both inventories - with its three refusals judged by the gate on a copy of the helper in a scratch root, so the
containment is checked on every gate run and the phase's other four criteria stand without it.**

## Baselines

start-commit: 19b1e7a2a4a4d08c968b481592709f3ef60c7abd

Measured at `start-commit`, before the first edit of this plan, each read from the file it counts:

| Count | Value | Read from |
|---|---|---|
| `runner-process` inventory data lines | 163 | `grep -v '^#' data/expected/tests/runner-process.ids \| awk 'NF' \| wc -l` |
| `runner-scenarios` inventory data lines | 35 | `grep -v '^#' data/expected/runner-scenarios.ids \| awk 'NF' \| wc -l` |
| gate steps | 38 | `ls tools/check/steps/*.sh \| wc -l` |

Gate summary line measured at `start-commit` (`sh tools/gate.sh`, before the first edit; rc 0, 0 `-- FAIL` lines):

```
gate: PASS steps=38/38 controls=405/405 ... claims=29 provisional=28 ... discrepancies=110 ... faults=238/238 tracked=2845 ...
scanned=703 ... profile-keys=155 goldens=29 ... gates=122 lines=372 ... prefix-files=11 records=155 fields=155 unknown-fields=48
... tests-expected=5102 tests-executed=5098 tests-skipped=4 tests-missing=0 ... stic-rules=67 ...
```

The line above is abbreviated to the tokens this plan touches; the full line is the one the gate printed at
`19b1e7a`. The plan's delta table names `tests-expected`, `tests-executed` and `tests-skipped` and predicts 0 for
every other token. One further token moves and the plan's table does not name it: `reports` goes 30 to 31, because
`golden.sh` counts the dev report of every scenario row and compares it with the release report, and this plan
adds one scenario row. That is a consequence of the row and not of a new report kind: the new file is
`build/gate/dev/runner/disc-sweep-probe.report`, the `missing-fixture` report the gate's own skip is judged from,
and it is byte-equal on both presets like every other deterministic row. Every other token the
plan predicted as 0 stayed 0.

## Task 1 and Task 2: one commit, because the two halves cannot be separated

### What the probe refuses, in order, each with one stderr line and exit 2

| Limb | Line | Exercised by |
|---|---|---|
| CI environment | `disc-sweep: refusing to run in CI (CI or GITHUB_ACTIONS is set): owner images are never used there` | `runner-process.disc-sweep.ci-refusal` (both variables, then `GITHUB_ACTIONS` alone) |
| bad arguments | `disc-sweep: usage: disc-sweep.sh [--preset dev\|release] [--max-runs <n>] --out <output> <exec-image> <grom-image>` | the same case's first limb reaches the CI refusal, so the usage line is exercised by the plan's own verify |
| image inside the work tree outside `refs/` | `disc-sweep: an image is inside the work tree outside refs/: <path>` | `runner-process.owner-run.in-tree-path` (the shared rule), and the probe's own verify |
| output inside the work tree outside `refs/` | `disc-sweep: the output <path> is inside the work tree outside refs/` | `runner-process.disc-sweep.output-tree`, three spellings |
| output parent that cannot be resolved | `disc-sweep: cannot resolve the output directory <dir>: the output must be under refs/` | the plan's own verify; a refusal and never a blind write |
| run count above the fixed bound | `disc-sweep: the run count <n> is above the bound 16` | `runner-process.disc-sweep.bound` |
| missing runner | `disc-sweep: no runner build/<preset>/runner/ivs-run (cmake --build --preset <preset>)` | the probe's own verify (the scratch roots always link a runner) |

The helper's own skip is a seventh outcome and not a refusal: `SKIPPED disc-sweep: no operator images (D11)` on
stdout and exit **77**, which is a different code from the scenario's exit **13** and both are needed - 77 is a bare
invocation with no image, 13 is the suite's case for a scenario whose `exec` role is the operator's.

### The output-path rule is the `flatwrap` rule, proved in three spellings

The physical parent of the output is resolved with a physical `cd` and compared with the physical work-tree root;
the write is allowed only when the resolved parent is `refs` or under it. The case asserts the three spellings that
a string-prefix test would pass and a physical comparison must refuse:

```
literal      <scratch>/data/probe.tsv          -> exit 2, no file
`..`         <scratch>/refs/../data/probe.tsv  -> exit 2, no file
symlink      <scratch>/link/probe.tsv          -> exit 2, no file   (link -> <scratch>/data)
accepted     <scratch>/refs/probe.tsv          -> exit 0, six offsets written
```

The accepted limb is what keeps the rule from passing on the three refusals alone: a rule that refused every output
would satisfy the negatives. It also asserts the table holds one line per swept offset and no digest-shaped token.

### The bounds, and the negative control inside the observation

Three bounds: the offset list is fixed and small (six entries), a run count above the fixed bound of 16 is refused
before the first run and before the output exists, and every run carries `--wall-timeout-ms 20000` with the elapsed
time re-checked between runs against a 300 s total. A run that ends inconclusive is an outcome in the table.

The fixed list straddles the end of frame 0 (59,736 master ticks, 14,934 CPU cycles on the NTSC family profile the
scenario names), and the temporary copy carries `capture video 0 1` plus one `assert frame 0 displayed == yes`. Measured
on the project's zero-filled stand-in images, which is the only input the gate has:

```
# disc-sweep: 6 runs, run-count bound 16, total wall bound 300s, per-run wall cap 20000ms
offset_cpu_cycles outcome displayed
14000 harness-fail -
14934 timeout no
15000 timeout no
16000 timeout no
18000 guest-fail no
20000 guest-fail no
```

The first line is the negative half: below the boundary the frame was never delivered, the assert is `not-run` and
the value is `-`; at and above it the slot header's displayed word is recorded. A probe that recorded `-` for every
offset would be the vacuous table lesson 20 warns about, so the list is not all on one side of the boundary.

### The skip, in both inventories, with the exact lines

`data/expected/runner-scenarios.ids` gains one six-column row and `data/expected/tests/runner-process.ids` one id line
with its trailing `skip-ok`:

```
disc-sweep-probe 13 missing-fixture deterministic skip-ok -
runner-process.scenario.disc-sweep-probe skip-ok
```

Measured, both presets, from each preset's own result file (`$IVS_TEST_OUT_DIR/runner-process.txt`):

```
ok 36 runner-process.scenario.disc-sweep-probe # SKIP disc-sweep-probe: needs operator content (ended missing-fixture, exit 13, as expected)
ok 128 runner-process.disc-sweep.ci-refusal
ok 129 runner-process.disc-sweep.output-tree
ok 130 runner-process.disc-sweep.bound
```

and the gate prints, once per preset:

```
-- SKIPPED dev/runner-process/runner-process.scenario.disc-sweep-probe: disc-sweep-probe: needs operator content (ended missing-fixture, exit 13, as expected)
-- SKIPPED release/runner-process/runner-process.scenario.disc-sweep-probe: disc-sweep-probe: needs operator content (ended missing-fixture, exit 13, as expected)
```

### The scenario carries no cartridge, and says why

`data/scenarios/disc-sweep-probe.ivss` asks for the operator's EXEC image by role (`content exec user`), declares
`expect-detail: missing role=exec user`, names `content grom zero-grom`, bounds the run at 4000 CPU cycles with
`stop halted`, and asserts `halted == yes` - and it carries **no** `content cart` line. Its header states the reason
twice over: a loadable program would let the runner boot instead of refusing, so the row would stop being the visible
skip it exists to be, and the `golden` step independently refuses a golden for a scenario with a user-content line,
which a golden here would be - a tracked record of an operator run.

### The measured suite cost, not a quoted unit price

`/usr/bin/time -p`, three alternated runs of the whole `runner-process` suite before and after, both presets
(lesson 22):

| Preset | before (user+sys, 3 runs) | median | after (user+sys, 3 runs) | median | delta |
|---|---|---|---|---|---|
| dev | 19.29, 21.77, 19.67 | 19.67 | 22.16, 20.32, 20.44 | 20.44 | +0.77 |
| release | 6.82, 7.97, 7.36 | 7.36 | 7.25, 9.13, 7.84 | 7.84 | +0.48 |

Both deltas sit inside the before-run spread (2.48 on dev, 1.15 on release), so the suite-level figure does not
resolve the addition. The addition was therefore measured where it actually is, three alternated runs of the one
expensive limb (the accepted sweep: six bounded runner runs plus the temporary copy):

| run | real | user+sys |
|---|---|---|
| 1 | 0.08 s | 0.07 s |
| 2 | 0.07 s | 0.05 s |
| 3 | 0.07 s | 0.05 s |

So the three cases together cost about 0.1 local cpu-s per preset (the accepted sweep is the whole of the expensive
half; the seven other helper invocations in the cases launch no runner), which is about 0.2 for both presets and
about 0.34 hosted at the 1.675 factor - inside the plan's priced 0.40 for four process-launch ids. No reprice is owed.

## Deviations from Plan

### Rule 3 - Blocking issue: the two tasks land in one commit

The plan asks for one commit per task. That cannot be honoured without a red gate between them:

- Task 1's `output-tree` case runs a **copy of the committed probe scenario** in its scratch root (the helper
  resolves `data/scenarios/disc-sweep-probe.ivss` from its own root), so the scenario file must exist in Task 1's
  commit.
- `tools/check/steps/golden.sh` refuses a scenario carrying `content <role> user` whose inventory row is not
  `skip-ok` (`golden-user-content-not-skip-ok`), so the scenario file cannot exist without its row in
  `data/expected/runner-scenarios.ids` and its `skip-ok` id line.

Task 1 step (5) anticipates exactly this ("if the two tasks cannot be one commit"). Resolution: both tasks' files
land in one commit, `tests-skipped` moves by two there, and the plan's two verify blocks were both run against that
commit. Nothing else in the plan is affected; the file list is the union of the two tasks' `<files>` lists, all of
which are inside the plan's `files_modified`.

### Rule 3 - Blocking issue: the absent-images skip sits before the missing-runner check

The plan's refusal ladder lists "a missing runner binary" before "then the absent-images case". Taken literally on a
tree where `build/<preset>/runner/ivs-run` is not built, the plan's own verify limb

```
CI= sh tools/disc-sweep.sh --out "$T/refs/x.tsv" "$T/absent-e.bin" "$T/absent-g.bin"   # expects 77
```

would exit 2 instead of 77, because the refusal ladder would reach the runner check first. The six refusals keep the
stated order among themselves; the skip sits after the run-count bound and before the runner and scenario existence
checks, which is also `tools/owner-run.sh`'s habit (a path that does not exist is never refused as a wrong shape).
Recorded because the plan's prose ordered the two differently.

### `capture video 0 1` in the temporary copy, and the `frame 0 displayed` assert

The plan says the sweep reads "the report's outcome line and the `frame 0 displayed` value". The report carries that
word only on an assert line, and a frame assert is refused without a capture (`assert-without-capture`, reason 49).
A user-content scenario is refused every frame **output** option (`--frame-ppm`, `--video-raw`), but `capture video`
is an in-memory capture that names no file and no rate, so the temporary copy carries `capture video 0 1` and the
assert. No committed file gains a capture line: the committed scenario is unchanged by the sweep and the copy lives
in the helper's own `mktemp -d`.

## Open items

- `VID-07` stays `Pending`. This plan mints no id in `data/expected/important-gates.ids` (its three ids are judged by
  the `runner-process` suite and registered there), moves no claim, no fault, no golden and no control, and flips no
  requirement: plan 19-03 owns the registry rows and the flip.
- The probe has never run against operator content, and this project states no result about any such run (D11 is
  open). Its sweep was exercised only on project-made zero-filled files of the exact sizes.
