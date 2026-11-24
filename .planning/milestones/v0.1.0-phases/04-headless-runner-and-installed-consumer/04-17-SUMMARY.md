---
phase: 04-headless-runner-and-installed-consumer
plan: 17
subsystem: runner, tests/runner, data
tags: [runner, scenario, IVSS, parser, fuzz, untrusted-input, RUN-01, RUN-02, D-18, D-22, D-23]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-03: ivs_runner_lib, suite runner, repo-relative fault owners; 04-04: runner-scan; 04-13: IVS_CTRL_* names pinned in data/expected/input-bits.ids; 04-16: data/golden/diag-cpu-1.expect (18 assert lines) and tests/expect-inc.cmake"
provides:
  - "runner/scenario.c: ivs_scenario_parse, the IVSS v1 grammar (D-18) as a pure, capped, allocation-free bytes-in, struct-out function with line-numbered reason tokens (D-22)"
  - "runner/ivs_runner.h: ivs_scenario and its parts, ivs_scenario_error, IVS_SCN_* caps, units, subjects and 42 reasons; ivs_scenario_reason_name, ivs_scenario_error_format (`scenario-syntax line=<n> reason=<token>`), the input-name table accessors"
  - "suite runner families parse (16 + 42 refuse + 23 edge cases) and fuzz (8 seeds + reasons-covered)"
  - "tests/input-bits-inc.cmake (pinned names to an X-macro include) and expect-inc.cmake FORM=text (the .expect assert lines as C string literals)"
  - "faults RUNNER_DOTDOT_ACCEPTED, RUNNER_LINE_CAP_UNCHECKED"
affects: [04-18, 04-19, 04-20, 04-21, 04-24]

actuals:
  tokens: 25600
  tasks: 3
  commits: 4
plan_head_before: 9a3c689e30ffcb0d8a855ad552c71316882ca5a9
plan_head_after: c26df68614fccefb489ddd9a42faab682ff9c8e9

tech-stack:
  added: []
  patterns:
    - "Two independent readings of one reviewed file: the parser reads the .expect assert lines verbatim (FORM=text) and the result is compared with the cmake regex reading of the same lines (X-macro form)"
    - "A test that enumerates the reason set: runner.parse.reasons-named requires exactly one runner.parse.refuse.<token> case per reason, so a new reason without a refusal case fails"

key-files:
  created:
    - runner/scenario.c
    - tests/runner/runner_parse.c
    - tests/runner/runner_fuzz.c
    - tests/runner/runner_cases.h
    - tests/input-bits-inc.cmake
  modified:
    - runner/ivs_runner.h
    - runner/CMakeLists.txt
    - tests/runner/runner_main.c
    - tests/CMakeLists.txt
    - tests/expect-inc.cmake
    - data/expected/tests/runner.ids
    - data/expected/faults/runner.ids

key-decisions:
  - "Grammar details left open by D-18, fixed here and documented in ivs_runner.h: `id` and `profile` are required (missing-directive, line 0); comments only with `#` as the first byte, so a leading-space `#` is unknown-directive and an inline `#` is an ordinary token; blank lines and leading/trailing blanks allowed; a final line without LF is accepted"
  - "All run-to lines share one unit and all input lines share one unit (mixed-step-unit, mixed-input-unit), because cross-unit order needs the profile timebase the parser does not have; run-to strictly increasing, input nondecreasing (D-14 allows equal ticks); time values at most 2^62"
  - "Assert forms: `halted ==|!= yes|no`, `reg R0..R7`, `mem AAAA`, `mem AAAA..BBBB` (every word of the inclusive range equals the value), `digest <composite|cpu|mem|images|instance|video|audio|input>` with 64 lower-case hex; HHHH/AAAA four upper-case hex digits, as expect-inc.cmake requires"
  - "Input lines may appear anywhere after `ivss 1` (they form one schedule); header directives must precede the first run-to; content `boot-stub` only for exec and `zero-grom` only for grom; expect-load/expect-sha256 need that role's content line first; `reason <n>` only with a non-ok status, n >= 1"
  - "Status names for expect-load come from the public ivs_status_name (IVS_OK and IVS_E_UNKNOWN excluded), input names from the public IVS_CTRL_* defines via a stringizing switch (no static tables, no core internals)"
  - "Refusal leaves the output all zero; NULL out/err, or NULL bytes with a nonzero length, return -1 and touch nothing"
  - "RUN-01 and RUN-02 stay Pending: the parser is the first piece; the executor, report and exit-code scenarios are 04-18..04-20"

requirements-completed: []

duration: about 20 min of edits and gates (first commit 18:21, last 18:38), plus two container gates
completed: 2026-11-11
---

# Phase 4 Plan 17: IVSS v1 scenario parser Summary

**The runner library parses untrusted scenario text into a fixed-size struct through one pure function with the D-22 caps and path rule, accepts the 18 assert lines of `data/golden/diag-cpu-1.expect` verbatim, refuses every malformed form with a line number and one of 42 reason tokens, survives 12,000 seeded mutations per run under ASan/UBSan, and loses each of its two security rules to a named fault that two independent cases catch.**

## What was built

- `runner/scenario.c` (listed once in `IVS_RUNNER_SOURCES`; runner-scan `runner-files=3`, only ISO headers and `ivs/ivs.h`; no statics, no allocation). Length cap first, then per line in byte order: line cap, NUL, CR, non-ASCII, other control bytes; tokens are slices of the input, and only length-checked values (id 48, profile 64, path 200) are copied. Integers and hex are hand-parsed with an explicit uint64 overflow check (`bad-number`) and range checks (`out-of-range`).
- Tracer `runner.parse.diag-boot-pass`: the scenario text 04-19 will write to `data/scenarios/diag-boot-pass.ivss` (header, `content exec boot-stub`, `grom zero-grom`, `cart diag-cpu-1.ivsc`, `run-to 4000 cpu-cycles stop halted`) followed by the 18 lines read from the `.expect` file through a generated string-literal include; every parsed assert (subject, register or address, value, line, step) equals the cmake X-macro reading of the same file.
- `runner.parse.input-names-complete`: the 31 pinned names (generated from `input-bits.ids`; each X-macro line also compiles `IVS_CTRL_<NAME>`, so a pin without a header define fails the build) each parse to their pinned bit through a scenario, and every parser table entry matches exactly one pinned name and bit.
- Refusal table: one `runner.parse.refuse.<token>` per reason (42; acceptance needs 20 or more), each checked for the exact line, an all-zero output and an identical second parse; 23 `runner.parse.edge.*` variants (empty file, overflow, empty and signed numbers, inline and indented `#`, `a/../../b`, trailing slash, reversed range, R8, 9-digit hex, `IVS_CTRL_` prefix, CR in the header line, DEL). Accept cases: one per directive plus `accept.caps` (65,536 bytes, 512 lines, 256-byte line, 200-byte path, 64 steps, asserts and inputs all accepted).
- Fuzz: 8 seeds x 1,500 iterations, one to three stacked mutations (byte flips and dictionary bytes, token inserts, cuts, line duplicate and swap, long lines, line repetition and 64 KiB padding to reach the caps). Invariants: result 0 or a named reason equal to the error's, all-zero output on refusal, line within the input's line count (line 0 only for file-level reasons), acceptance only within the byte, line and line-length caps and with every structural rule held, determinism, input never written. `reasons-covered` observed 36 distinct reasons and 1,147 acceptances (needs 8 and 1).

## Performance

- Plain-run times (wall, 3 runs each, load average 14-16 with the container gate running): whole suite `runner` dev 0.56-0.57 s, release 0.21 s (was about 0.01 s with the outcome family only). Family split: parse dev 0.08 s, release under 0.01 s; fuzz dev 0.53 s, release 0.20 s (the fuzz is 94 percent of it; `reasons-covered` replays all seeds, as ivsc.fuzz does).
- Fault runs: two more rebuilds and reruns of suite runner (`fault-runs=142`); c-faults step 8 s locally.
- Gate totals: Task 1 37 s, Task 2 26 s, Task 3 32 s (sh) and 34 s (dash), final 33 s (sh) and 24 s (dash); container 115 s (failed run) and 128 s (passing run).

## Counts

Start: `steps=30/30 controls=263/263 faults=140/140 tests-expected=2472`.
End: `steps=30/30 controls=263/263 fault-runs=142 faults=142/142 tests-expected=2652 tests-executed=2652 tests-skipped=0 tests-missing=0 runner-files=3` (+90 ids in `runner.ids`, 111 total, each in both presets).

Acceptance checks: `grep -c 'scenario.c' runner/CMakeLists.txt` = 1; `grep -c '^runner.parse.refuse.' data/expected/tests/runner.ids` = 42; `grep -c -E '^RUNNER_(DOTDOT_ACCEPTED|LINE_CAP_UNCHECKED) ' data/expected/faults/runner.ids` = 2; `-- runner-scan pass runner-files=3`.

Fault behaviour (faults-verdict): RUNNER_DOTDOT_ACCEPTED fails refuse.path-dotdot, edge.dotdot-inside, error-format and fuzz seeds 02, 05, 06, 07 and reasons-covered (the fuzz path invariant sees an accepted `..`); RUNNER_LINE_CAP_UNCHECKED fails refuse.line-too-long and all eight seeds plus reasons-covered (longest-line invariant). Both keep `runner.outcome.exit-pass` green.

## Deviations from Plan

**1. [Rule 3 - Blocking] Fault sites deferred to Task 3.** Task 1's draft carried both `#ifndef IVS_FAULT_RUNNER_*` sites, and faults-verdict failed `fault-orphan-site` because the ids were not registered until Task 3. The sites were removed for the tracer and Task 2 commits and added with their registry lines in `a88f16e`.

**2. [Rule 1 - Bug] cmake 3.28 policy hazard (found by `tools/gate-linux.sh`).** `expect-inc.cmake` had no `cmake_minimum_required`, so under cmake 3.28 CMP0054 was OLD and `if(FORM STREQUAL "text")` dereferenced the script's own variable `text`; the text include came out in X-macro form and gcc failed in both presets (macOS cmake 4.4 has no OLD behaviour, so the local gate passed). Both generators now start with `cmake_minimum_required(VERSION 3.25)` like every other repo script and the accumulator is `text_lines`. Commit `c26df68`; the container gate then passed.

**3. Files outside the plan's list:** `tests/runner/runner_cases.h` (family interface; the suite now dispatches by index over the outcome, parse and fuzz families), `tests/input-bits-inc.cmake` (the "generated include of the pin file" the behaviour asks for) and `tests/expect-inc.cmake` (FORM=text, so the tracer reads the 18 lines from the reviewed file instead of retyping them).

**4. Added beyond the plan:** `runner.parse.reasons-named`, `error-format`, `null-args` and 23 `edge.*` cases; a fuzz invariant that an accepted input is within the byte, line and line-length caps (makes the fuzz an independent second detector of RUNNER_LINE_CAP_UNCHECKED); reason tokens beyond the plan's list (control-byte, missing-directive, bad-arity, bad-token, out-of-range, bad-id, bad-profile, path-byte, path-empty-segment, path-too-long, bad-content-kind, expect-without-content, bad-status, bad-sha256, mixed-step-unit, mixed-input-unit, bad-value, bad-input-value).

## Open Items

- Plan 04-19 writes `data/scenarios/diag-boot-pass.ivss`; its text should equal `diag_boot_pass_text` of `tests/runner/runner_parse.c` (header lines plus the 18 `.expect` lines). A later check could parse the tracked file directly instead of the in-test copy.
- The executor (04-18/04-19) owns: resolving relative paths only under `--content-dir`, `wall-timeout-ms` only lowering the operator cap, converting `cpu-cycles`/`frames` and `at_cycle` through the profile timebase, and printing refusals with `ivs_scenario_error_format`. Scenario files may start with `#` comment lines (an SPDX line, if the integrity rules ask for one) before `ivss 1`.
- `docs/runner.md` (grammar, reason table) is not written by this plan; the grammar is documented in `runner/ivs_runner.h`.
- RUN-01, RUN-02, RUN-04 stay Pending (executor, report, outcome scenarios; important-gates 04-24 may register RUNNER_DOTDOT_ACCEPTED and RUNNER_LINE_CAP_UNCHECKED).
- Carried: C14 and C63-C68 revisited in Phase 5/6/7; `API-02`, `PKG-03`, `PKG-04`, RUN-05, RUN-06 Pending; `pending-resolvers=17`, `unknown-fields=16`; `sh tools/hygiene.sh --history` still fails on earlier planning commits (this plan's commits add no home paths); the orchestrator records hosted runs with `sh tools/ci-record.sh <run-id>`.

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=30/30 controls=263/263 ... cc=AppleClang c-presets=2 fault-runs=142 ... faults=142/142 ... runner-files=3 ... tests-expected=2652 tests-executed=2652 tests-skipped=0 tests-missing=0 ...`, `gate-time total=33`.
- `dash tools/gate.sh`: the same line, `gate: PASS`, `gate-time total=24`.
- `sh tools/gate-linux.sh` (run because the plan adds a parser with integer, size and string handling): Ubuntu 24.04 container, gcc 13, cmake 3.28: first run `gate: FAIL steps=24/30` (deviation 2), after `c26df68`: `gate: PASS steps=30/30 controls=263/263 cc=GNU fault-runs=142 faults=142/142 tests-expected=2652 tests-executed=2652 tests-skipped=0 tests-missing=0`, `gate-time total=128`, `gate-linux: exit 0`.

## Threat Flags

None beyond the plan's threat model:
- T-04-49 mitigated: relative `[A-Za-z0-9._/-]` paths, no `..` anywhere, no leading `/`, no empty segment, at most 200 bytes; refused before anything is copied; fault RUNNER_DOTDOT_ACCEPTED caught by two cases and the fuzz.
- T-04-50 mitigated: 64 KiB checked first, 512 lines, 256 bytes per line checked while scanning, 64 steps, asserts and inputs; fault RUNNER_LINE_CAP_UNCHECKED caught by the refusal case and every fuzz seed; fuzz under ASan/UBSan in the dev preset.
- T-04-51 mitigated: echoable values (id, profile, path) are charset-restricted with no blank, newline or `..`; the error format prints only a number and a fixed token.
- T-04-SC: no package installs.

## Self-Check: PASSED

- Files present: runner/scenario.c, runner/ivs_runner.h, tests/runner/runner_parse.c, tests/runner/runner_fuzz.c, tests/runner/runner_cases.h, tests/input-bits-inc.cmake.
- Commits reachable from HEAD: eaa7ccf, 17b8cb3, a88f16e, c26df68 (`git rev-list --count 9a3c689..HEAD` = 4 before the docs commit).
- No `.ivsc`, `.bin`, `.rom` or `.exec` file tracked; no file under `data/scenarios/` yet (04-19).
