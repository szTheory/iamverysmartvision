---
phase: 04-headless-runner-and-installed-consumer
plan: 02
subsystem: ci
tags: [ci, measurement, ci-log, ci-runs, D-44, D-45, PKG-04]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-01: gate-env, gate-cpu, step-time and gate-time lines on every gate run"
provides:
  - ".planning/ci-runs.tsv: append-only 13-column hosted run record (header only until the orchestrator appends rows)"
  - "tools/ci-record.sh: orchestrator-only recorder, two gh calls per run, offline --from-files mode with the same parser"
  - "Step ci-log: tokens ci-log-header, ci-log-fields, ci-log-duplicate, ci-log-order, stat ci-runs=<n>; one control each"
  - "CI-EFFICIENCY.md: D-45 median budget rule, lever order, mid-phase checkpoints after 04-10 and 04-19"
affects: [04-10, 04-19, 04-25, ci]

actuals:
  tokens: 5300
  tasks: 2
  commits: 2
plan_head_before: 6ddf371a089f63aa3f1d214df5af8f7ba3735051
plan_head_after: 512e26c70ddbd145d647b739e4ba974f90adb6f3

tech-stack:
  added: []
  patterns:
    - "Orchestrator-only helpers that call gh keep a --from-files mode running the same parser, so the parse is testable offline and agents never spend the shared rate limit"
    - "ISO-8601 times are converted with awk civil-date arithmetic (no jq/gojq date builtins, fractional seconds tolerated)"

key-files:
  created:
    - tools/ci-record.sh
    - tools/check/steps/ci-log.sh
    - .planning/ci-runs.tsv
    - data/expected/controls/ci-log.ids
    - tools/check/controls/_good/tree/.planning/ci-runs.tsv
    - tools/check/controls/ci-log-header/
    - tools/check/controls/ci-log-fields/
    - tools/check/controls/ci-log-duplicate/
    - tools/check/controls/ci-log-order/
  modified:
    - data/expected/gate-steps/ci-measure.ids
    - .planning/CI-EFFICIENCY.md

key-decisions:
  - "ci-record.sh emits ISO strings from --jq and converts them in the shared awk parser instead of using fromdateiso8601, so --from-files exercises the conversion and fractional-second timestamps cannot break it"
  - "ci-log is stricter than the plan's minimum: run_id must be digits, sha exactly 40 lower-case hex digits, event/conclusion [a-z_-]+ (the recorder writes `-` for a missing conclusion)"
  - "PKG-04 stays Pending: the mechanism is in place but no hosted run is recorded yet"

patterns-established:
  - "A GitHub step reported as never started (0001-01-01 time) yields -1, never a negative duration"

requirements-completed: []

duration: 15min
completed: 2026-11-11
---

# Phase 4 Plan 02: Hosted run record, ci-log step and the D-45 budget rule Summary

**Each hosted run can now become one validated TSV row (`tools/ci-record.sh <run-id>`, two `gh` calls, orchestrator only); the new `ci-log` step refuses a malformed, duplicated or out-of-order record on every gate run; and CI-EFFICIENCY.md carries the D-45 median rule the orchestrator applies after plans 04-10 and 04-19.**

## Performance

- **Duration:** about 15 min (2026-11-11T16:12Z to 16:27Z)
- **Tasks:** 2 of 2
- **Files:** 19 changed (232 insertions)

## Counts

Start: `steps=27/27 controls=237/237 faults=119/119 tests-expected=2144`.
End: `steps=28/28 controls=241/241 faults=119/119 ci-runs=0 tests-expected=2144 tests-executed=2144 tests-skipped=0 tests-missing=0`, matching the plan's stated end counts.

## Accomplishments

- `.planning/ci-runs.tsv`: header `date run_id sha event conclusion queue_s setup_s build_s test_s gate_s total_s ncpu cpu_s` (tab-separated), no rows; no rows were invented from the Phase 3 prose table.
- `tools/ci-record.sh`: `--help`/`-h` exit 0 and no arguments exit 2 before any `gh` lookup; a non-numeric run id exits 2; `gh` missing exits 2. Run-id mode: call 1 `--json databaseId,createdAt,startedAt,updatedAt,conclusion,headSha,event,jobs --jq` (one line: run id, created, job started, updated, conclusion, sha, event, checkout step start and end, `-` for missing), call 2 `--log` filtered with `grep -F` for the four measurement prefixes; both into a `mktemp -d` dir. The parser strips the hosted `<job> TAB <step> TAB <timestamp> ` prefix and only accepts a measurement word at the start of the remaining text (a `-- FAIL ... step-time c-build 999` decoy is ignored). `grep -c 'gh run view' tools/ci-record.sh` = 2.
- Step `ci-log` (pure awk over `$ROOT/.planning/ci-runs.tsv`), SP1 prologue/epilogue, pass line `-- ci-log pass ci-runs=<n>`; listed in `data/expected/gate-steps/ci-measure.ids` under a plan 04-02 / D-44 comment.
- Toy record (two rows) in `_good/tree/.planning/`; four overlay controls, each EXPECT `FAIL <token>:`; `data/expected/controls/ci-log.ids`.
- CI-EFFICIENCY.md: new append-only section `2026-11-11 — Phase 4 measurement and the D-45 budget rule (plan 04-02)` (record, definitions, `-1` meaning, D-45 rule verbatim, levers in order, mid-phase checkpoints after the pushes of 04-10 and 04-19, the +5-8 s estimate and the saturation ratio `cpu_s / (ncpu x gate_s)`, local measurements of 04-01 and 04-02). Earlier sections untouched.

## Task Commits

1. **Task 1: recorder, record header and ci-log step (tracer)** - `868dc77` feat(04-02)
2. **Task 2: toy record, four controls, D-45 section** - `512e26c` feat(04-02)

## Task 1 end-to-end proof (scratch, not committed)

- Local jq (not gh) ran the recorder's `--jq` expression on a synthetic `gh run view --json` document: `43000000001 2026-11-11T16:00:00Z 2026-11-11T16:00:04Z 2026-11-11T16:01:02Z success 0123...4567 pull_request 2026-11-11T16:00:05Z 2026-11-11T16:00:07Z`; with `jobs: []`, an empty conclusion and no checkout step it prints `-` placeholders and keeps 9 fields.
- `log.txt`: the `step-time`, `gate-env`, `gate-cpu` and `gate-time` lines of a real local gate run, each given the hosted `gate TAB Run sh tools/gate.sh TAB <timestamp> ` prefix, plus the decoy line.
- `sh tools/ci-record.sh --from-files meta.txt log.txt` (same under dash):
  `2026-11-11 43000000001 0123456789abcdef0123456789abcdef01234567 pull_request success 4 2 1 43 44 62 18 116` (tabs shown as spaces: queue 4, setup 2, build 1, test 43, gate 44, total 62, ncpu 18, cpu 53.30+62.65 rounded).
- Row appended to a scratch copy of the header-only TSV, step run with `ROOT` at the scratch tree: `-- ci-log pass ci-runs=1`.
- Date arithmetic checked against `date -u -j` for 1970 to 2026, a leap day with a fractional second (86402 s), a year boundary (120 s) and 2000-02-29 to 2100-03-01 (3155716800 s): all equal. A `0001-01-01T00:00:00Z` (never-started) end gives `-1`. Garbage metadata and an empty metadata file exit 1 with a message; `gate-cpu unknown` gives `cpu_s -1`.
- `gh` stub probe (plan verify 2): `--help` exit 0, no arguments exit 2, the stub's marker file absent.

## Task 2 probes

- Each control tree run alone reports only its own token: `ci-log-header: line 1 is not the 13 column names`, `ci-log-fields: line 3`, `ci-log-duplicate: line 3 run 1001`, `ci-log-order: line 3`; `_good/tree` passes with `ci-runs=2`.
- Extra scratch probes: missing file and empty file -> `ci-log-header`; `-1` values and an equal date pass; `10.5`, `-2`, an upper-case sha, a trailing blank line -> `ci-log-fields`; CRLF -> header plus fields.
- `sh tools/check/run-controls.sh`: `controls=241/241`; `grep -c -E 'control ok +ci-log-(header|fields|duplicate|order)$'` = 4.

## Final gate summary lines (local, macOS, Apple clang 21)

- `sh tools/gate.sh` at the final tree: `gate: PASS steps=28/28 controls=241/241 ... ci-runs=0 ... faults=119/119 ... tests-expected=2144 tests-executed=2144 tests-skipped=0 tests-missing=0 ...` (`gate-time total=33`; an earlier run of the same code: `gate-cpu user=39.50 sys=47.34`, `gate-time total=31`).
- `dash tools/gate.sh`: the same tokens, `gate: PASS` (`gate-cpu user=44.21 sys=52.15`, `gate-time total=34`).
- Ubuntu container (`tools/gate-linux.sh`): not run; the plan does not require it (A-07 names the runner, packaging and close-out plans). `ci-log` uses only POSIX awk (no intervals, no gawk extensions); `tools/ci-record.sh` is never run in CI.
- Step cost: `step-time ci-log 0`.

## Deviations from Plan

### Discretion

**1. ISO dates converted in awk, not with `fromdateiso8601`**
- The plan allows the awk civil-date fallback (A8). Choosing it up front puts the conversion in the shared parser, so the offline mode tests it, and gh's jq engine is never asked to parse fractional seconds. The `--jq` expression only selects fields; `databaseId` was added to the `--json` list so `--from-files` carries the run id.

**2. Stricter row checks in `ci-log`**
- run_id digits, sha 40 lower-case hex, event/conclusion `[a-z_-]+`, beyond the plan's "empty run_id/sha" minimum. All four are under `ci-log-fields`.

**3. No blank line in `ci-measure.ids`**
- No other inventory has blank lines; the 04-02 comment line directly follows the 04-01 entry.

## Open Items

- **For the orchestrator:** run `sh tools/ci-record.sh <run-id>` after each hosted run of this branch and append the row to `.planning/ci-runs.tsv` (`>> .planning/ci-runs.tsv`); evaluate the D-45 rule after the pushes of plans 04-10 and 04-19.
- The `--jq` expression and log prefix stripping were checked with local jq and synthetic input only; the first real `sh tools/ci-record.sh <run-id>` by the orchestrator is their first live check (a `-1` in a column it should fill points at the expression).
- PKG-04 stays Pending until hosted rows exist in `.planning/ci-runs.tsv`. PKG-03 unchanged (Pending).
- Carried from 04-01: the job summary's "Skips and missing" table lists `pending-resolvers=17` and `unknown-fields=16` (record stats whose names match the D-42 pattern). Not in this plan's scope (an allowlist would change `ci-summary.sh` and its selftests, and D-42 says "every token matching"); still open. `ci-runs` does not match the pattern.
- Carried from 04-01: `sh tools/hygiene.sh --history` (publication scan, not a gate step) still fails on earlier pushed planning commits; not run as a gate here. The two commits of this plan add no paths.

## Threat Flags

None beyond the plan's threat model (T-04-05 mitigated by the four ci-log tokens and controls; T-04-06 by two calls per run, orchestrator only, and the offline mode; T-04-07 accepted: the row holds only numbers, a sha, an event and a conclusion).

## Self-Check: PASSED

- Files present: tools/ci-record.sh, tools/check/steps/ci-log.sh, .planning/ci-runs.tsv, data/expected/controls/ci-log.ids, tools/check/controls/_good/tree/.planning/ci-runs.tsv, the four control dirs.
- Commits reachable from HEAD: 868dc77, 512e26c (`git rev-list --count 6ddf371..HEAD` = 2).
