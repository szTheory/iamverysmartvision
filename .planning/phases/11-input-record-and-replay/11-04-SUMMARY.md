---
phase: 11-input-record-and-replay
plan: "04"
subsystem: input-record-replay
tags: [ivsl, input-log, operator-surface, process-case, nine-domains, vacuity, exclusive-create]
requires:
  - "11-02 (the ivsl v1 library, the executor's log_in/log_out seam and input.events)"
  - "11-01 (docs/formats/ivsl-v1.md, the frozen oracle)"
provides:
  - "runner/main.c: --record-input and --replay-input as path options in the strcmp chain, with the usage text, the mutual exclusion, the log read and parse before the first create, the exclusive create, the path-clash checks grown to six, the post-run write and the harness-fail discard"
  - "docs/runner.md: two option-table rows and the new process case named in the determinism paragraph"
  - "runner-process.record.command-line: the phase's only process case, and the only place a produced log is a file"
affects:
  - "11-05 (the two ivsl faults; the operator surface and its refusals are now pinned by a process id)"
tech-stack:
  added: []
  patterns:
    - "ordering as a property: the log is parsed before the first create, so a refusal leaves no state and the case can observe that by the absence of a file"
    - "the equality gets a process-level liveness witness: the same stripped copy with no log must differ"
    - "a rights posture made a checked property: the two content-user blocks are read and required to name exactly five options"
key-files:
  created: []
  modified:
    - runner/main.c
    - docs/runner.md
    - tests/runner-process-tap.sh
    - data/expected/tests/runner-process.ids
decisions:
  - "the log is deliberately not added to the three `content ... user` refusal blocks (D15: it holds no content bytes), and a static assertion in the process case keeps that posture from drifting"
  - "the log joins the golden clash check and the pairwise output-path loop (five to six); --replay-input joins neither, because a replay path that equals an output path is a legitimate choice and the exclusive create refuses the collision that matters"
  - "a header-only log is valid, so a scenario with no `input` line is recordable (research O-3), asserted on diag-boot-pass rather than noted"
  - "the no-log witness of the stripped copy ends guest-fail 10, not pass: with no input the lamp-tone asserts fail (plan deviation 1)"
metrics:
  duration: "1h 02m"
  completed: "2026-11-25T17:05:00Z"
  tasks: 2
  files: 4
status: complete
actuals:
  tokens: 11117
  tasks: 2
  commits: 2
  plan_head_before: 7bf0b22
  plan_head_after: 1e63733
---

# Phase 11 Plan 04: the operator surface and the one process case Summary

A person can record a run to an `ivsl` v1 log and replay it into the same scenario text with its
`input` lines removed, from the command line, and the replay reproduces the recorded report byte for
byte — `host.calls` included — with all nine digest domains equal, non-`-`, and `digest.input` equal
to the committed golden's value. The same stripped copy with no log gives a different report, so the
equality is shown to depend on the log at the process boundary as well as in process.

## The recorded log, measured

`demo-lamp-tone-pass` under `--record-input` writes **319 bytes**: four header lines and six event
lines.

```
ivsl 1
scenario demo-lamp-tone-pass
profile intv-2609-ntsc-family-p1
content project
at_tick 100000 c0=00000001 c1=00000000
at_tick 220000 c0=00000101 c1=00000000
at_tick 340000 c0=00000000 c1=00010000
at_tick 460000 c0=10000000 c1=00010000
at_tick 580000 c0=00000000 c1=00000000
at_tick 700000 c0=00000101 c1=20000000
```

The six ticks are the scenario's own six, checked against the scenario file rather than against the
log, so a log that listed its own events would be caught. The report says `input.events 6` and
`input.consumed 6`.

## The four runs of the process case

| Run | Text | Options | Outcome | `input.events` | `input.consumed` | `host.calls` |
|---|---|---|---|---|---|---|
| Record | `demo-lamp-tone-pass` | `--record-input` | pass 0 | 6 | 6 | 11 |
| Re-run | the same path | `--record-input` | harness-fail 14 | none (no report) | none | none |
| Replay | the same text with `sed '/^input /d'` | `--replay-input` | pass 0 | 6 | 6 | 11 |
| No log | the same stripped copy | none | guest-fail 10 | 0 | 0 | 11 |

The record and replay reports are **byte-identical over the whole report**, `host.calls` included
(`cmp`, no key excluded). The no-log run's report differs and its `outcome.detail` is
`assert=4 line=27 mem 02F2 == 007E got 00FF`.

**Nine domains, on the one case that can observe them (A-11-01).** All nine `digest.*` lines are
present, not `-`, and equal between the two runs: `digest.cpu`, `digest.mem`, `digest.images`,
`digest.instance`, `digest.composite`, `digest.video`, `digest.audio`, `digest.input`, `digest.psglog`.
The four live counts are nonzero on both runs: `output.video.bytes` 198848, `output.audio.bytes`
201640, `output.psglog.records` 30, `input.events` 6. `digest.input` is
`832906ce775d32a98ae607386ce4bbcdd4d08017e3b914b6089e50a5364c7761` on both runs, equal to the
committed golden's value — so the replayed applied-event chain is checked against the frozen oracle,
not against the recorded run alone. The in-process fake host observes six of nine, which is why this
clause lives here and not in plan 11-02.

## The refusals, verbatim

| Row | First stderr line | Exit | Report | File |
|---|---|---|---|---|
| existing path | `ivs-run: --record-input: output file exists, refusing to overwrite it` | 14 | none | byte-identical (`cmp` with a saved copy) |
| both options | `ivs-run: --record-input and --replay-input are exclusive` (usage text follows) | 14 | none | **no file created** |
| missing log | `ivs-run: log not readable: missing` | 14 | none | — |
| oversize log (4,200 bytes) | `ivs-run: log not readable: too large` | 14 | none | — |
| directory | `ivs-run: log not readable: not a regular file` | 14 | none | — |
| malformed log | `ivs-run: --replay-input: line 1 reason unknown-directive` | 14 | none | — |

**The ordering, stated.** `ivs_scenario_parse` → the three `content ... user` refusal blocks → the
log read and parse → the first `main_open_out`. That is what makes "no state change" a property
rather than an assertion: the mutual-exclusion row asserts that the refusal created **no file**, which
is the observable form of the ordering (T-11-15). `ivs_hostio_read` opens without blocking, requires
`fstat` to report a regular file and reads at most `cap + 1` bytes, so a FIFO, a directory and an
oversize file are each refused rather than read or followed (T-11-14).

`runner/hostio.c` was **not edited**: no new POSIX surface, no new feature macro, `runner-files=18`
[HEAD=18 EXPECT==18]. No flag bit was added: `IVS_RUNNER_OPT_ROUNDTRIP` keeps its only-bit meaning
and the two new options are `slot` arms like `--audio-raw`, so `runner.exec.roundtrip-option-range`'s
three `2u` rows keep asserting what they asserted. The log is deliberately **absent** from the three
`content ... user` refusal blocks, because it holds no content bytes at all (D15) and refusing it
would contradict the format's reason to exist.

## The rights check on a real file

This is the only place a produced log is a file at all, so it is the only place the check can be
proved against an artifact the runner wrote. With the `scenario` and `profile` values masked out
(`demo-lamp-tone-pass`, 19 bytes; `intv-2609-ntsc-family-p1`, 25 bytes — both inside the document's
`[a-z0-9-]` charset and width) the 319-byte file carries **0** runs of 32 or more hex digits. The same
masked text with the golden's `digest.input` line appended carries **at least 1**, so the check is
shown able to fail (A-11-05, lesson 20).

**The rights posture is a checked property (T-11-17).** The process case reads `runner/main.c` and
extracts the two `content ... user` refusal blocks. The frame block names exactly `--frame-ppm` and
`--video-raw`; the audio block names exactly `--audio-raw`, `--audio-wav` and `--psg-log-out`; neither
names `--record-input` or `--replay-input`. So the exemption cannot drift by someone adding a line.

**A header-only log is valid (research O-3).** `diag-boot-pass` carries no `input` line; it records
with exit 0, its report says `input.events 0`, and its log is exactly the four header lines — 4 lines,
zero `at_tick` lines, 80 bytes, equal to the sum computed from its own four strings. So a run that
applied no event is not refused for having nothing to write.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug in the plan's expectation] The no-log witness ends guest-fail, not pass**
- **Found during:** Task 2, the first run of the stripped copy without `--replay-input`
- **Issue:** The plan's Task 2 action says the no-log witness should `judge` `0 pass -`. The stripped
  copy of `demo-lamp-tone-pass` carries no `input` lines, so the lamp-tone program never receives a
  key and its own asserts fail. The run is deterministic and correctly ends `10 guest-fail` with
  `outcome.detail assert=4 line=27 mem 02F2 == 007E got 00FF`.
- **Fix:** The witness is judged by its real outcome — exit 10, outcome `guest-fail`, that exact
  detail — and keeps both assertions the plan required of it (the report differs from the recorded
  one; `input.events` is 0). Nothing about the vacuity argument changes: the point is that the report
  differs, and it does.
- **Files modified:** `tests/runner-process-tap.sh`
- **Commit:** 1e63733

**2. [Rule 2 - Missing critical coverage] One extra refusal row: a directory**
- **Found during:** Task 2, while writing the refusal rows
- **Issue:** The plan listed three refusal rows (missing, oversize, malformed). `ivs_hostio_read`'s
  `NOT_REGULAR` branch is the one that refuses a FIFO or a directory (T-11-14), and it had no row.
- **Fix:** One row: `--replay-input` on the case's own `mktemp` directory, exit 14, one line
  `ivs-run: log not readable: not a regular file`, no report. It costs one launch.
- **Files modified:** `tests/runner-process-tap.sh`
- **Commit:** 1e63733

### Approved budget deviation

The dispatch allowed a hard 1800-second limit and pre-approved a reduced gate programme: `sh tools/gate.sh`
at the end of each task and `dash` once at the very end, **no container gate**, and a plan-level
`dash` again after the closing records commit. That is what was run; `gate-linux.sh` and the container
gate were **not** run for this plan, so the Ubuntu substance of the new process case is unproven here
and rests on the hosted row.

## The usage literals

`main_usage` grew by nine lines for the two options. Measured between the quotes: literal 0 is 1,652
source characters and literal 1 is 3,785, both below the 4,095 ISO C guarantees a compiler. The build
is clean on both presets, and `-Wpedantic` (on through `ivs_warnings`) is what enforces the ISO limit
on GCC in the hosted row.

## Cost

Three alternated runs of the `runner-process` suite on the dev preset, with and without the case's
seven launches (three `demo-lamp-tone-pass` runs, one `diag-boot-pass`, four refusal rows and `--help`):

| Run | With the case | Without it |
|---|---|---|
| 1 | 16.69 s | 15.72 s |
| 2 | 17.57 s | 16.85 s |
| 3 | 18.66 s | 18.12 s |
| median | 17.57 s | 16.85 s |

Delta of medians **0.72 s** per suite run on the dev preset; the suite reports `executed=133` with the
case against `executed=132` without. Load average 7.54 / 8.45 / 9.17 before and 7.55 / 8.33 / 9.04
after, so the host was busy and these are wall seconds, not cpu seconds. The research priced the case
at about 0.45 local cpu-s; the measured cost is in the same order and a little above it — recorded as
a finding, not a re-bless. Full suite wall time as measured by ctest: 18.13 s dev, 10.15 s release.

## Gate

Task 1, after `runner/main.c` and `docs/runner.md`:

```
gate: PASS steps=38/38 controls=395/395 faults=235/235 gates=109 lines=333 tests-expected=4986
tests-executed=4982 tests-skipped=4 tests-missing=0 docs=64 doc-files=63 runner-files=18 claims=28
provisional=27 goldens=29 reports=30 host-dependent=3 release-only=2 prefix-files=11 claim-refs=26
cases=13 files=2645 tracked=2645 hygiene-files=2645 mode=tree
```

Task 2, after the process case and its id:

```
gate: PASS steps=38/38 controls=395/395 faults=235/235 gates=109 lines=333 tests-expected=4988
tests-executed=4984 tests-skipped=4 tests-missing=0 docs=64 doc-files=63 runner-files=18 claims=28
provisional=27 goldens=29 reports=30 host-dependent=3 release-only=2 prefix-files=11 claim-refs=26
cases=13 files=2645 tracked=2645 hygiene-files=2645 mode=tree
```

Both deltas are exactly the prediction: `tests-expected` 4986 → 4988 and `tests-executed` 4982 →
4984, one new id on two presets. Everything else is frozen at its baseline. `files == tracked ==
hygiene-files` = 2645 with a delta of 0 per task (no file created), and `git status --porcelain`
carried no `.ivsl`: every log, copy and malformed file lives under the suite's `mktemp -d`.

`ctest --preset dev -R '^runner-process$'` and `ctest --preset release -R '^runner-process$'` both
pass, and the new id is reported `ok 91` (not a skip) on the dev preset.

## Liveness, proved by probe

A replay of the `demo-lamp-tone-shift` log into the same stripped copy gives a different report and
`digest.input d6116a8a22a78c1c14df961b2759263e133e94033b197a729a748992363c1713` instead of the
golden's `832906ce…`. So the equality depends on the log's **content**, not merely on the option being
present, and the golden comparison would catch a replay that read the file and used something else.
Together with the no-log witness (the same copy with no log differs) this closes A-11-03 at the
process boundary as well as in process.

## Tier

The ceiling for this work is `deterministic-repeatability`: a replayed run repeats the recorded one.
That is a ceiling and not a claim — nothing here says the run is right, only that the replay of the
log reproduces it, and `digest.input`'s agreement with the committed golden inherits the golden's own
`self-recorded` oracle. `TOOL-01` stays `Pending`; no claim and no registry row was added.

## Self-Check

- `runner/main.c` carries `--record-input` and `--replay-input`: FOUND
- `docs/runner.md` carries both rows and names `docs/formats/ivsl-v1.md`: FOUND
- `tests/runner-process-tap.sh` carries `runner-process.record.command-line`: FOUND
- `data/expected/tests/runner-process.ids` registers it exactly once: FOUND
- commit 5e3f5e4 (Task 1) and 1e63733 (Task 2) are ancestors of HEAD: FOUND

## Self-Check: PASSED
