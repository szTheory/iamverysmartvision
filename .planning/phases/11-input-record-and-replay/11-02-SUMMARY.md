---
phase: 11-input-record-and-replay
plan: "02"
subsystem: input-record-replay
tags: [ivsl, input-log, tracer, vacuity, record, replay, faults-placement]
requires:
  - "11-01 (the frozen oracle: docs/formats/ivsl-v1.md)"
provides:
  - "runner/ivsl.h, runner/ivsl.c: the IVSL v1 writer and parser as a pure library TU, with the nine reason names"
  - "the executor's replay seam: the schedule comes from the log when one is supplied, and the applied prefix is recorded"
  - "input.events prints the effective count (no golden moves)"
  - "runner.record.replay-equals-straight, runner.record.format-roundtrip, runner.judge.record-replay-differs, runner.judge.record-replay-inert"
affects:
  - "11-03 (the nine refusal ids reuse the parser and the reason vocabulary; the untouched-output contract is pinned here)"
  - "11-04 (the operator file I/O and the process case on demo-lamp-tone-pass)"
  - "11-05 (the two faults have a library site because of where ivsl.c landed)"
tech-stack:
  added: []
  patterns:
    - "tracer first: one slice through every library layer, with its liveness witness, in one commit"
    - "the guard before the equality, and a self-control per mechanism so each is proved able to fail"
key-files:
  created:
    - runner/ivsl.h
    - runner/ivsl.c
  modified:
    - runner/CMakeLists.txt
    - runner/ivs_runner.h
    - runner/exec.c
    - runner/report.c
    - tests/runner/runner_exec.c
    - data/expected/tests/runner.ids
decisions:
  - "the log's schedule lives in ex_state (sched/sched_n), so the recorder writes what the run applied rather than re-deriving it"
  - "no flag bit: a non-NULL log_in or log_out is the switch, so IVS_RUNNER_OPT_ROUNDTRIP keeps its only-bit meaning"
  - "the recorder runs after the step loop through a wrapper, so a log always describes a run that happened"
metrics:
  duration: "27m"
  completed: "2026-11-25T14:25:00Z"
  tasks: 3
  files: 8
status: complete
actuals:
  tokens: 41000
  tasks: 3
  commits: 3
  plan_head_before: 0081f88
  plan_head_after: c655652
---

# Phase 11 Plan 02: the tracer, the format library and the two judge self-controls Summary

A run records to an input-only `ivsl` v1 log, and the same scenario text with its own `input` lines
removed replays that log into a byte-identical report — `host.calls` included — while the same text
with no log produces a different report. The format is `runner/ivsl.c`, a pure library TU, because
`tests/faults.cmake:155` refuses a fault whose owner TU is outside `IVS_RUNNER_SOURCES` (A-11-04).

## The three runs of the equality case, measured

| Run | Text | Options | `input.events` | `input.consumed` | Outcome |
|---|---|---|---|---|---|
| A | diagnostic + two `input` lines | `log_out` (4096-byte buffer) | 2 | 2 | pass, log 152 bytes |
| B | the same bytes with those two lines removed | `log_in` = A's parsed log | 2 | 2 | pass |
| B' | the same bytes with those lines removed | none | 0 | 0 | pass, report **differs** from A |

A and B are byte-equal over the whole report, `host.calls` included: the log is a caller buffer and
never crosses the host vtable, so nothing has to be excluded — a strictly stronger comparison than
`runner.exec.roundtrip-equals-straight` can make. B' is the liveness witness (A-11-03): without it,
a `--replay-input` path that parsed the log and ignored it would produce A's report from a scenario
that still carried its own `input` lines.

The case also asserts the log's `at_tick` list and words (`100` / `KEY_1` / `0`, `200` / `0` /
`DISC_E`), its header (`version 1`, `content project`, `diag-boot-pass`,
`intv-2609-ntsc-family-p1`), the nine `digest.*` lines present, non-`-` and equal in A and B, and
that the pair (A, B') is rejected by the guard with `record-no-events`.

**Liveness proved by probe, not asserted.** `x->log_in` was temporarily forced to the scenario path
and the case failed with `the recorded and the replayed reports: record-no-events` — the vacuity
class this plan exists to close, caught by this case. The tree was restored and rebuilt before the
commit.

## The round trip, and the sizes as measured

`runner.record.format-roundtrip` makes five assertions in this order: write → parse → write is
byte-identical with equal lengths; format and parse are fixpoints of each other (events, ticks, both
words, names, content class unchanged, and the second and third buffers byte-equal to the first);
the byte figures computed **from the strings**, never hard-coded; the event cap tied to the constant
it comes from; and a refusal leaves the caller's output untouched.

Measured, printed by the case itself:

```
# the log is 152 bytes: a 80-byte header and 36 and 36 for its two event lines
```

**One plan figure corrected (the tree wins).** The plan's Task 2 says the diagnostic's header is 85
bytes and an event line 39. Measured with `wc -c`: the `diag-boot-pass` /
`intv-2609-ntsc-family-p1` / `content project` header is **80** bytes (85 is the
`demo-lamp-tone-pass` header, as `11-01-SUMMARY.md` already recorded), and an event line is 36
bytes for a three-digit tick (39 is a six-digit tick). The assertion is computed from the strings, so
it was already right; the fix is to the plan's prose, recorded here per PLAN-LESSONS 19. The
`demo-lamp-tone-pass` figures (85 + 6 * 39 = 319) are the process case's in plan 11-04, not this
suite's, which has no `lamp-tone` image.

The two caps: `IVS_IVSL_MAX_EVENTS == 64u && IVS_IVSL_MAX_EVENTS == IVS_SCN_MAX_INPUTS`, and
`ivs_ivsl_format` returns 1, leaves `*written` alone and writes nothing into a buffer one byte short.
The refusal assertion poisons an `ivs_ivsl_log` with `0xA5`, parses a one-byte-truncated log
(refused `trailing`), and requires the struct to still be all `0xA5` — the contract plan 11-03's nine
per-token cases repeat.

## The two judge self-controls

| Id | Pair | Finding required |
|---|---|---|
| `runner.judge.record-replay-differs` | two texts of a real recording run, one nibble of `digest.input` changed | `record-differs` (the equality) |
| `runner.judge.record-replay-inert` | two identical texts, both reporting `input.events 0` and `input.consumed 0`, empty log | `record-no-events` (the guard) |

Each also asserts the other mechanism's finding is a **different** string, so the two are proved
separate. The inert control is the load-bearing one: identical texts pass any equality, so only the
guard can reject a zero-event pair — and without it the equality case's three clauses would be
unproven machinery (the 08-16 lesson, one level up).

**Both probed able to fail for their own reason.** With `record-differs` disabled the differs control
failed with `the one-nibble pair gave '(accepted)'` while the inert control still passed; with the
`record-no-events` clause disabled the inert control failed with `the identical zero-event pair gave
'(accepted)'` (and the guard-less planning also surfaced as `record-count-mismatch`, which is why
the pair of findings, not just "it failed", is asserted).

## Ancestry, placement, and where the file I/O is not

```
doc=$dcfcd65  (git log --diff-filter=A --format=%H -1 -- docs/formats/ivsl-v1.md)
code=$50148e7 (git log --diff-filter=A --format=%H -1 -- runner/ivsl.c)
git merge-base --is-ancestor $doc $code   -> 0 (exit), different commits
```

So the format document is a strict ancestor of the first code commit (criterion 1's ordering half).

`ivsl.c` is in `IVS_RUNNER_SOURCES` (after `frame_export.c`) and absent from `IVS_RUN_SOURCES`;
`build/dev/ivs-runner-sources.txt` lists `runner/ivsl.c`. The reason is structural, not stylistic:
`tests/faults.cmake:155` fails configure for a fault owner TU outside a module of its suite, and the
runner suite's modules are `core;runner`, where `runner` is `IVS_RUNNER_SOURCES`. `main.c` and
`hostio.c` are in `IVS_RUN_SOURCES` and outside the fault framework by construction (A-11-04), so a
fault site with file I/O would be impossible. The recorder and the replayer therefore operate on
memory and the file is plan 11-04's business.

`runner-scan` on the tree: `-- runner-scan pass runner-files=18`.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] The recorder could not run at the end of `ex_run_steps` as written**
- **Found during:** Task 1, reading `ex_run_steps`' early returns
- **Issue:** every outcome other than a clean finish leaves the function through a `return` (guest
  fail, timeout, harness fail), so "record at the end of the function" would have written a log only
  for runs with no observation at all — silently no log for a guest fail.
- **Fix:** the body became `ex_run_steps_body` and `ex_run_steps` writes the log after it when the run
  did not end harness-fail. The schedule moved from a local array to `ex_state.sched`/`sched_n`, so the
  recorder writes what the run applied instead of re-deriving it from the scenario.
- **Files modified:** `runner/exec.c`
- **Commit:** `50148e7`

**2. [Rule 1 - Bug] Unused-parameter `-Werror` failure in the hex reader**
- **Found during:** Task 1, first build
- **Issue:** `ivs_ivsl_hex8` computed the value but never stored it (`-Wunused-parameter` on `v`).
- **Fix:** store on success and check the eight-digit width before writing.
- **Files modified:** `runner/ivsl.c`
- **Commit:** `50148e7`

### Recorded, not deviations

- **Budget deviation (approved):** `sh tools/gate.sh` ran once at the end of each of the three tasks
  and `dash` once at the very end; the container gate did not run. Recorded as the dispatch instructed.
- **Trailing/arity line numbering on a file-shape fault.** The document names "the last line before the
  offending byte"; the parser names the line the offending byte sits on (a byte outside printable
  ASCII), and the last line for a line that does not end in LF. Only successful parses are exercised
  here; plan 11-03's per-token cases own the line numbers, and this choice is stated so it is not
  re-decided there.
- **`build/dev/ivs-run-sources.txt` does not exist.** The plan's verify names it; the tree writes one
  combined `build/dev/ivs-runner-sources.txt` for both lists. Placement was proved by reading
  `IVS_RUN_SOURCES` out of `runner/CMakeLists.txt` instead.
- **No `gsd_run` state verbs were available for this project layout**, so `STATE.md` and `ROADMAP.md`
  were edited by hand (plan counter, stopped-at, the plan row and the plan count). `TOOL-01` stays
  `Pending`: this plan is not the requirement's close (plan 11-05 flips it with its registry rows),
  and `11-01-SUMMARY.md` already recorded that an early flip drops `important-gates` to 37/38.

## Cost, measured

`./build/dev/tests/ivs_tests_runner`, three alternated `/usr/bin/time -p` runs on the dev preset:

| Reading | real | Load average |
|---|---|---|
| at `50148e7` (Task 1 only) | 0.92 / 0.90 / 0.89 s | 8.15 8.39 9.27 |
| at `c655652` (all four ids) | 0.93 / 0.94 / 0.93 s | 9.20 9.58 9.55 |

So the three new cases cost about 0.03 s in a loaded tree: each judge control runs one diagnostic run
of its own, and neither runs a machine twice. No pre-plan reading was taken in this dispatch; the
Task 1 reading is the in-plan baseline.

## Gate tails

Both shells, at the last task commit (`c655652`), `gate: PASS`, identical absolute tokens:

```
steps=38/38 controls=395/395 faults=235/235 gates=109 lines=333
tests-expected=4966 tests-executed=4962 tests-skipped=4 tests-missing=0
runner-files=18 docs=64 doc-files=63 claim-refs=26 cases=13 goldens=29 reports=30
prefix-files=11 claims=28 provisional=27 api-calls=19 public-symbols=19 pinned-defines=120
files=2643 tracked=2643 hygiene-files=2643
```

Predicted against measured, per task: `tests-expected` 4958 → 4960 → 4962 → 4966 and
`tests-executed` 4954 → 4956 → 4958 → 4962 (each id is two presets), `runner-files` 16 → 18 at Task 1
and frozen after, and the file delta +2 at Task 1, 0 at Tasks 2 and 3. Every other
absolute-and-frozen token held: `steps=38/38`, `controls=395/395`, `faults=235/235`, `gates=109`,
`lines=333`, `goldens=29`, `reports=30`, `claims=28`, `provisional=27`, `prefix-files=11`,
`api-calls=19`, `public-symbols=19`, `pinned-defines=120`, `consumer-lines=164`.

## Known Stubs

None. The one deliberate incompleteness is scope, not a stub: `ivsl.c`'s nine refusal reasons are
declared and implemented, and only two ids of this plan execute the parser's success path. The nine
`runner.record.refuse.<token>` ids are plan 11-03's, and the operator file I/O is plan 11-04's.

## Threat surface

T-11-05 (schedule substitution), T-11-06 (the comparison), T-11-07 (the recorder's output buffer) and
T-11-08 (the TU's placement) are the ones this plan lands. T-11-05: the log's events are copied into
`x->sched[IVS_SCN_MAX_INPUTS]` only after `ex_log_options_valid` and the per-event tick check, and the
loop bound is the log's own count. T-11-06: the comparison includes `host.calls` and the no-log
witness must differ. T-11-07: a too-small buffer is `log-out-max-bytes` and writes nothing. T-11-08:
`ivsl.c` is in `IVS_RUNNER_SOURCES`. No new surface outside the plan's `<threat_model>`.

## Tier ceiling

`deterministic-repeatability` — the replay runs the same core, so it is never an independent
comparison.

## Self-Check

`runner/ivsl.h`, `runner/ivsl.c` and this file exist; `50148e7`, `d5da81d` and `c655652` are
ancestors of `HEAD`. All four ids pass in a clean `./build/dev/tests/ivs_tests_runner` run (251 `ok`
lines, zero `not ok`).

## Self-Check: PASSED
