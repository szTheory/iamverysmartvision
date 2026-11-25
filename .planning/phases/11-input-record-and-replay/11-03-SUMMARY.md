---
phase: 11-input-record-and-replay
plan: "03"
subsystem: input-record-replay
tags: [ivsl, refusal-tokens, rights-check, in-process-suite, a-11-02, a-11-05]
requires:
  - "11-01 (the frozen oracle: docs/formats/ivsl-v1.md, its nine Refusal rows and its Limits)"
  - "11-02 (runner/ivsl.c: the nine IVS_IVSL_R_* arms, ivs_ivsl_reason_name, the untouched-output contract)"
provides:
  - "nine runner.record.refuse.<token> ids, one per row of the document's Refusals table"
  - "runner.record.no-content-digest with a stand-in that fires and an all-digit-slug witness"
  - "rx_refuse (five assertions per row) and rx_accept (the acceptance half) as the record section's two ways to judge a parser"
affects:
  - "11-04 (the process case greps a real produced file; this suite's checks are in memory)"
  - "11-05 (TOOL-01 stays Pending; this plan flips nothing)"
tech-stack:
  added: []
  patterns:
    - "one id per token, and a token that is a class carries its enumerated members as rows"
    - "every refusal with a nearby legal boundary ships with an acceptance row, so narrowness is a check and not a claim"
    - "a check that cannot fail ships with a stand-in that does, and its exclusion with a witness"
key-files:
  created: []
  modified:
    - tests/runner/runner_exec.c
    - data/expected/tests/runner.ids
decisions:
  - "the document's content-value trigger is fed to arity, where the document and the parser both put it, not to unknown-directive"
  - "a 20-digit at_tick field is arity, not tick-bound: the decimal reader refuses the field's width before the bound is judged"
  - "the tick bound is pinned as IVS_SCN_TIME_MAX - 1 accepted and IVS_SCN_TIME_MAX refused, because the parser refuses tick >= IVS_IVSL_TICK_MAX and the two constants are equal"
  - "hex runs are counted in both cases: the project's digests are lower case, so an upper-case-only scan would not find the value the stand-in carries"
  - "no-content-digest lands here rather than in 11-04 (recorded deviation from the dispatch's wave grouping): both live in the record section of one file"
metrics:
  duration: "24m"
  completed: "2026-11-25T09:43:10Z"
  tasks: 3
  files: 2
status: complete
actuals:
  tokens: 52000
  tasks: 3
  commits: 3
  plan_head_before: a02b320
  plan_head_after: 86a2682
---

# Phase 11 Plan 03: the nine refusals and the rights check Summary

Ten ids land: nine `runner.record.refuse.<token>` cases, each feeding its token's enumerated triggers and
requiring **that** token at **that** line with the caller's output untouched, plus
`runner.record.no-content-digest`, which reports zero 32-hex runs on a produced log and at least one on a
stand-in that carries a real 64-hex digest. The parser and the writer were not edited: `runner/ivsl.c` and
`runner/ivsl.h` are absent from this plan's diff and `runner-files` stayed 18.

## The nine tokens, and every row each case feeds

`rx_refuse(what, token, reason, line, text, len)` asserts five conditions per row, not four: the parse returns
`1`; `err.reason` is the code the row names; `err.line` is the 1-based line the row judges;
`ivs_ivsl_reason_name(err.reason)` equals the `token` the row passes (a transcription of the document's table,
so the comparison is document against code rather than code against a copy of itself); and the
`0xA5`-poisoned `ivs_ivsl_log` the caller passed is byte-for-byte unchanged, with the first differing byte
printed on failure. The plan's four are the first four plus the poison; the fifth is the vocabulary check.

| Token | Rows fed (line judged) |
|---|---|
| `unknown-directive` | a `digest.input <64 hex>` line (5); a scenario-grammar `input at_tick=100 ...` line (5); an `AT_TICK` first token in the wrong case (5) |
| `version` | `ivsl 2` (1); `ivsl 0` (1) |
| `arity` | `content` missing at the end (3); `scenario` duplicated (3); `profile` before `scenario` (2); `at_tick` with three tokens instead of four (5); `content hardware` (4); a dot in the `scenario` id (2); a 20-digit `at_tick` field (5); a `profile` of 65 bytes (3) |
| `hex` | `c0=0000000a` lower case (5); `c0=0000001` seven digits (5); `c1=000000001` nine digits (5) |
| `trailing` | a bare `x` after the last complete line (5); a complete final line with its LF removed (5); a NUL inside an event line (5), fed by length rather than `strlen` |
| `tick-order` | 100, 200, 150 (7) |
| `tick-bound` | `4611686018427387904` (5); `4611686018427387905` (5) |
| `event-cap` | a 65th event line (69) |
| `controller-bit` | `c0=80000000` (5); `c1=80000000` (5) |

`arity` and `trailing` are each a class with enumerated members rather than a place to put everything:
`arity` holds eight line shapes, `trailing` three file shapes, and neither row is a value on a line that
another token owns. The document's `content project|user` and `at_tick` field-width triggers are fed to
`arity` for exactly that reason.

## The four boundaries, each with its acceptance row

Six refusal rows are bounded by four acceptance rows, which are the load-bearing half: a refusal one step
inside its own bound passes every refusal row in the file while refusing a legal log.

| Boundary | Refused | Accepted |
|---|---|---|
| equal ticks | - (`tick-order` fires only on a **falling** tick) | two events at tick 100, 2 events parsed |
| the tick bound | `IVS_SCN_TIME_MAX` = 4611686018427387904, and + 1 | `IVS_SCN_TIME_MAX - 1` = 4611686018427387903, 1 event |
| the event cap | 65 event lines | 64 event lines, 64 events parsed |
| bit 31 | `c0=80000000`, `c1=80000000` | `c0=40000000`, parsed word `0x40000000` (= `IVS_CTRL_RESERVED_MASK >> 1`) |

The plan's tick figures were corrected against the parser and the document, which agree with each other: the
largest legal tick is 2^62 - 1, because `ivs_ivsl_parse` refuses `tick >= IVS_IVSL_TICK_MAX` and
`IVS_IVSL_TICK_MAX == IVS_SCN_TIME_MAX == 2^62`. Both bounds are printed from the constants, never written as
literals.

## The event-cap case fires the event cap, not the file cap

Measured, printed by the case itself:

```
# event-cap: the 65-event text is 2420 bytes, under the 4096-byte file cap
```

The case asserts the length is **under** `IVS_IVSL_MAX_BYTES` before it parses, so the only mechanism that can
refuse that text is the 65th event line. Without that assertion the case could pass because a caller's file
bound refused it, and the event cap would be unproven (research Pitfall 4). The file cap is not a parser
token at all: `ivs_ivsl_parse` treats `len > IVS_IVSL_MAX_BYTES` as a caller error (`-1`), which is why the
research's `file-too-large` is not one of the nine and plan 11-04's process case is where a real oversized
file is refused and asserted.

## The rights check: four assertions and their observed counts

```
# no-content-digest: the produced log 152 bytes, 0 masked 32-hex runs; the stand-in 1; the 32-digit slug 1 unmasked and 0 masked
```

| # | Assertion | Observed |
|---|---|---|
| 1 | zero 32-hex runs on a real produced log, with the `scenario` and `profile` values masked | 0 |
| 2 | at least one 32-hex run on the same log with `digest.input 832906ce...7761` appended | 1 |
| 3 | shape pin: `scenario` 1..48 bytes of `[a-z0-9-]`, `profile` 1..64, exactly one line of each | `diag-boot-pass` (13), `intv-2609-ntsc-family-p1` (24), 1 and 1 |
| 4 | a 32-digit `scenario` value: unmasked has runs, masked has none, the log still parses and passes the pin | 1 unmasked, 0 masked, 32 bytes, parsed |

The masking rule in one sentence: the two name values are replaced by a single `x` before the scan, because
`copy_slug` in `runner/scenario.c` permits digits, so a 32-digit scenario id is a legal slug that a naive
whole-file scan would count as a digest run and refuse.

Assertion 2 is the one that makes assertion 1 mean something: no well-formed log can carry a 32-hex run (a
decimal tick is at most 19 digits, a controller field exactly 8 hex digits separated from its neighbour by
` c1=`), so "0 runs" is true of every log and would be the lesson-20 defect on its own (A-11-05). The stand-in
is a string literal in this committed source, never a tracked `.ivsl` file.

**Hex runs are counted in both cases.** The golden's digest is lower case, so a scan over `[0-9A-F]` alone
would report 0 runs on the very stand-in that carries it and the witness would be false. `rx_is_hex` accepts
both cases and the reason is in the comment.

**The check was shown red, then restored.** The stand-in assertion's expectation was temporarily inverted to
`!= 0` and the case failed with `the stand-in carries 1 32-hex runs, want at least 1: the check cannot fail`;
the tree was restored, rebuilt and re-run green, and `git status --porcelain` carries no `.ivsl` file. Two
more probes on the Task 1/2 halves: `version` with its expected reason changed to `IVS_IVSL_R_ARITY` failed
with `reason version line 1, want arity line 1`, and the same-tick acceptance row with an expectation of 3
events failed with `accepted with 2 events, want 3`. All three were restored before their commits.

## The ten ids added

`runner.record.refuse.unknown-directive`, `.version`, `.arity`, `.hex`, `.trailing` (Task 1);
`.tick-order`, `.tick-bound`, `.event-cap`, `.controller-bit` (Task 2); `runner.record.no-content-digest`
(Task 3). The record section now holds exactly twelve non-comment ids: nine refusals, the equality, the
round trip and this one. `runner/ivsl.c` and `runner/ivsl.h` are untouched and `TOOL-01` stays `Pending`.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Plan defect] `content hardware` is `arity`, not `unknown-directive`**
- **Found during:** Task 1, reading the parser against the document
- **Issue:** Task 1's second `unknown-directive` row fed `content hardware` and required that token. `content`
  is one of the five directives, so the first token is known and the fault is its value; the document's
  `arity` row names "a `content` value that is neither `project` nor `user`", and `ivs_ivsl_parse` returns
  `IVS_IVSL_R_ARITY` for it. The row would have failed against a parser that is right.
- **Fix:** the trigger is fed to the `arity` case (line 4, whole text) and the `unknown-directive` case takes a
  scenario-grammar `input at_tick=100 ...` line instead, so the trigger is still exercised under the token the
  document assigns it and the token keeps three rows.
- **Files modified:** `tests/runner/runner_exec.c`
- **Commit:** `5fb752e`

**2. [Rule 1 - Plan defect] a 20-digit `at_tick` is `arity`, not `tick-bound`**
- **Found during:** Task 2, reading `ivs_ivsl_dec`
- **Issue:** Task 2's second `tick-bound` row fed a 20-digit tick. `ivs_ivsl_dec` refuses a field of more than
  19 digits before any value is compared, so the parse returns `IVS_IVSL_R_ARITY`; the document's field table
  gives `at_tick` a width of 1..19, which is a shape rule.
- **Fix:** the row moved to the `arity` case, and `tick-bound` keeps two refusal rows (`IVS_SCN_TIME_MAX` and
  `+ 1`) plus the acceptance row at `IVS_SCN_TIME_MAX - 1`. The plan's `IVS_SCN_TIME_MAX`-is-accepted claim
  was also wrong: the largest legal tick is `IVS_SCN_TIME_MAX - 1`.
- **Files modified:** `tests/runner/runner_exec.c`
- **Commit:** `705e110`

**3. [Rule 1 - Bug] the hex scan had to count both cases**
- **Found during:** Task 3, first run of the stand-in assertion
- **Issue:** `rx_hex32_runs` over `[0-9A-F]` reported 0 runs on a stand-in carrying the golden's
  `832906ce...7761`, because the project spells digests in lower case. The assert-at-least-one row would have
  failed, and worse, a scan written that way would be blind to every real digest in the repository.
- **Fix:** `rx_is_hex` accepts `[0-9a-f]` as well, so the witness fires and the scan matches the digest form
  the project actually uses.
- **Files modified:** `tests/runner/runner_exec.c`
- **Commit:** `86a2682`

### Recorded, not deviations

- **`no-content-digest` lands in plan 11-03, not in plan 11-04 with the process case** - the dispatch's wave
  grouping put it in the earlier wave's slot and the plan's own artifact table assigns it to Task 3 here. It
  stays here because both halves live in the record section of `tests/runner/runner_exec.c` and splitting them
  would add a fifth wave for no extra evidence. The id's four assertions and its stand-in are unchanged, and
  plan 11-04's process case still greps a **real produced file**, which this suite cannot do without writing
  inside the tree.
- **Budget deviation (approved):** `sh tools/gate.sh` ran once at the end of each of the three tasks and `dash`
  once at the very end; the container gate did not run. Recorded as the dispatch instructed.
- **The plan's Task 3 id-count arithmetic was off by one.** Its verify expected 11 matching lines before the
  new id and 12 after; `runner.ids` already carries a comment line at 271 naming
  `runner.record.replay-equals-straight`, so a prefix grep counts it. The check here counts non-comment lines
  instead (`awk '!/^#/'`), which is the count the plan meant: 12 ids, of which 9 are refusals.
- **No uniform row table:** the five Task 1 cases share a `rx_refuse` helper rather than a table of function
  pointers, because each token's rows differ in how their text is built (whole files, header tails, a
  length-fed NUL) and a uniform table would have hidden the line each row names.

## Gate tails

Both shells, at the last task commit (`86a2682`), `gate: PASS`, identical absolute tokens. `sh`:

```
gate: PASS steps=38/38 controls=395/395 faults=235/235 gates=109 lines=333
tests-expected=4986 tests-executed=4982 tests-skipped=4 tests-missing=0
runner-files=18 docs=64 doc-files=63 goldens=29 reports=30 claims=28 provisional=27
prefix-files=11 api-calls=19 public-symbols=19 pinned-defines=120 claim-refs=26
files=2644 tracked=2644 hygiene-files=2644 terms=9 mode=tree
```

`dash` produced the same tokens, before and after the records commit, and the two shells agreed token for token
both times. The observed `files=` / `tracked=` / `hygiene-files=` triple at `86a2682` is **2644 / 2644 / 2644**,
unchanged from the plan-start baseline of 2644, so each task's file delta is 0: this plan creates no source, data
or test file. The records commit is the one addition, so `files=` reads 2645 after it - the summary itself, which
the plan's token discipline places outside every task's delta.

## Predicted against measured, per task

| Reading | Predicted | Measured |
|---|---|---|
| `tests-expected` after Task 1 | 4976 | 4976 |
| `tests-executed` after Task 1 | 4972 | 4972 |
| `tests-expected` after Task 2 | 4984 | 4984 |
| `tests-executed` after Task 2 | 4980 | 4980 |
| `tests-expected` after Task 3 | 4986 | 4986 |
| `tests-executed` after Task 3 | 4982 | 4982 |

Ten ids at two presets each: +10, +8, +2. Every absolute-and-frozen token held at its plan-start value:
`steps=38/38`, `controls=395/395`, `faults=235/235`, `fault-runs=235`, `gates=109`, `lines=333`,
`runner-files=18`, `docs=64`, `doc-files=63`, `goldens=29`, `reports=30`, `claims=28`, `provisional=27`,
`prefix-files=11`, `api-calls=19`, `public-symbols=19`, `pinned-defines=120`, `tests-skipped=4`,
`tests-missing=0`.

## Known Stubs

None. No test writes a log inside the work tree: the log is a caller buffer, the stand-in is a string literal
in committed source, and `git status --porcelain` carries no `.ivsl` file after the suite and after the gate.

## Threat surface

T-11-09 (the nine arms) is the one this plan lands, nine times over: per token the code, the line, the
rendered token against the document's spelling, and an untouched output. T-11-10 (the 65-event case) is
mitigated by the under-the-file-cap assertion. T-11-11 (`no-content-digest`) is mitigated by the stand-in, the
shape pin, the 32-digit-slug witness and the scratch inversion. T-11-12 (scratch files) is mitigated by the
suite writing nothing and the `.ivsl`-in-status check. No new surface outside the plan's `<threat_model>`, and
T-11-SC is untouched: no package manager was invoked and no dependency was added.

## Tier ceiling

`deterministic-repeatability` - every case here judges the project's own parser against the project's own
document. Agreement between them is repeatability of two artifacts from one authorship, never an independent
oracle for what the format should refuse.

## Self-Check

`5fb752e`, `705e110` and `86a2682` are ancestors of `HEAD`; `commits: 3` is the plan ledger's own count, read at
the last task commit (`git rev-list --count a02b320..HEAD` = 3 at `86a2682`). The records commit that carries this
file is deliberately outside it, per the plan's token discipline, so re-running that count now gives 4 - three task
commits and this one. A clean `./build/dev/tests/ivs_tests_runner` run reports
`261` `ok` lines, `executed=261 skipped=0 failed=0`, zero `not ok`. The ten ids are present in the TAP output
and in `data/expected/tests/runner.ids`, and `runner/ivsl.c` / `runner/ivsl.h` do not appear in this plan's
diff.

## Self-Check: PASSED
