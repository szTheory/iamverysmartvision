---
phase: 04-headless-runner-and-installed-consumer
plan: 20
subsystem: tests, data/scenarios, data/expected, docs
tags: [runner, ivs-run, runner-process, scenarios, mutants, determinism, judge, RUN-01, RUN-02, RUN-06, D-20, D-21, D-23, D-32]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-19: ivs-run with --slice-ticks, --wall-timeout-ms, --self-signal-after-slices, --help outcome pairs, data/scenarios/diag-boot-pass.ivss, docs/runner.md; 04-15/04-16: ivs_pack --ivsc --patch and the build-tree images"
provides:
  - "suite runner-process (tests/runner-process-tap.sh, POSIX sh and dash, registered in both presets): 26 ids, the real ivs-run judged by exit code and report text"
  - "data/expected/runner-scenarios.ids: six fixed columns `<id> <exit> <outcome> <class> <skip> <operator-args>`, 11 scenarios, one per outcome plus expect-load-refusal and diag-input-pass"
  - "ten new data/scenarios/*.ivss, each pinning its outcome.detail (`# expect-detail:`) or parse-refusal stderr line (`# expect-stderr:`) in a comment"
  - "data/expected/diag-mutations.ids: four D-32 program mutations with hand-derived words and the clause each breaks"
  - "data/expected/runner-outcomes.ids: the six `<outcome> <exit>` pairs; --help, docs/runner.md and the inventory compared by one id"
  - "per-preset reports build/gate/<preset>/runner/<id>.report for the golden step (04-21)"
affects: [04-21, 04-23, 04-24]

actuals:
  tokens: 8995
  tasks: 3
  commits: 3
plan_head_before: 820a7a0064a9d350b25885ac7bab0666dd1b9cc8
plan_head_after: 1e5a75fdd1cc6878fdf27953f9486dd62d272715

tech-stack:
  added: []
  patterns:
    - "Expected detail lives in the scenario file as a comment line (`# expect-detail:` / `# expect-stderr:`), so the six-column inventory stays generic and every non-default detail is pinned next to the scenario"
    - "Judge self-controls reuse the exact check functions (check_scenario, check_repeat) with a fake runner script; a control passes only when the check fails with its intended finding token"
    - "run_one: background child plus a sleep-based watchdog subshell (the c-faults run_limited shape) that keeps the exit status; no sleep synchronises a test"

key-files:
  created:
    - tests/runner-process-tap.sh
    - data/expected/runner-scenarios.ids
    - data/expected/runner-outcomes.ids
    - data/expected/diag-mutations.ids
    - data/expected/tests/runner-process.ids
    - data/scenarios/guest-assert-mismatch.ivss
    - data/scenarios/timeout-budget.ivss
    - data/scenarios/timeout-wall.ivss
    - data/scenarios/cancel-signal.ivss
    - data/scenarios/missing-cart-file.ivss
    - data/scenarios/needs-user-exec.ivss
    - data/scenarios/bad-syntax.ivss
    - data/scenarios/unexpected-refusal.ivss
    - data/scenarios/expect-load-refusal.ivss
    - data/scenarios/diag-input-pass.ivss
  modified:
    - tests/CMakeLists.txt
    - docs/runner.md

key-decisions:
  - "All scenario cases use one temp content dir: copies of the build-tree images plus diag-cpu-1-truncated.ivsc (ivs_pack --ivsc output cut before its 32-byte trailer by dd); the core refuses it IVS_E_MALFORMED reason 1"
  - "The judge also pins outcome.detail exactly (from the scenario comment, `-` when absent) and requires each report's outcome/exit.code pair to be in runner-outcomes.ids"
  - "bad-syntax (parse refusal) prints no report: the judge requires empty stdout and exactly the stderr line `ivs-run: scenario-syntax line=9 reason=unknown-directive`; no .report file is written for it"
  - "skip-ok (needs-user-exec) is judged like any case (exit 13, missing-fixture, detail `missing role=exec user`); only a passing judgement becomes `# SKIP`, a wrong outcome is `not ok`"
  - "Mutant ids are `<FROM>-<TO>` (PSHR-MVOAT avoids `@` in a test id); the suite checks the listing row at <addr> is <FROM> and that the patched container differs, so a listing edit cannot turn a mutant into a no-op"
  - "PSHR to MVO@ uses MVO@ R3 via R1 (R1 = 8080 at that point; no other register points into RAM there), so R6 stays 0340 and PULR reads 033F"
  - "RUN-01, RUN-02 and RUN-06 marked complete: the real executable now demonstrates the input-scheduled scenario, all six outcomes with distinct exit codes, and the original diagnostic from reset through the boot stub with mutants failing for their clause"

duration: about 14 min of edits and gates (first commit 19:30, last 19:39), plus one container gate
completed: 2026-11-11
---

# Phase 4 Plan 20: runner-process suite Summary

**The real `ivs-run` of both presets now runs eleven tracked scenarios, one per outcome plus an expected load refusal and a two-event input schedule, and each is judged by exit code, `ivsr 1`/`report.end`, outcome, exit.code and a pinned detail. The original diagnostic passes from reset. Its four D-32 program mutants each end guest-fail naming the first assert they break. Reports are byte-equal across a repeat, slice sizes 1 and 7 and another cwd (apart from `host.calls`). Four fake runners show that the judge fails for each wrong-runner shape.**

## What was built

- Task 1 (tracer, `2dbe7f2`): `tests/runner-process-tap.sh <root> <ivs-run> <ivs_pack> <images-dir>` (TAP shape, nonce and trailer as `headers-tap.sh`; absolute paths; `run_one` watchdog, `IVS_RUNNER_PROCESS_TIMEOUT` default 60 s), registered with `$<TARGET_FILE:ivs_run>`, `$<TARGET_FILE:ivs_pack>` and `IVS_IMAGES_DIR`, `SKIP_RETURN_CODE 77`; inventory with `diag-boot-pass 0 pass deterministic run -`; reports to `$IVS_TEST_OUT_DIR/runner/<id>.report`.
- Task 2 (`284cc2f`): ten scenarios; `diag-mutations.ids`; mutant, repeat, slices and cwd cases.
- Task 3 (`1e5a75f`): `runner-outcomes.ids`, judge pair check, four `runner-process.judge.*` self-controls, `outcomes-consistent` and `outcomes-covered`; a `docs/runner.md` section on the suite. `--help` and the docs table already agreed with the inventory (no change to `runner/main.c`).

| Scenario | Exit | Detail pinned |
|---|---|---|
| diag-boot-pass | 0 | `-` |
| guest-assert-mismatch | 10 | `assert=13 line=24 mem 0204 == 0036 got 0037` |
| timeout-budget | 11 | `budget step=1 line=11 expected=halted got=target` |
| timeout-wall (host-dependent; `--wall-timeout-ms 1 --slice-ticks 4`, stub only, bound 10^12 ticks) | 11 | `wall step=1 cap-ms=1` |
| cancel-signal (host-dependent; `--self-signal-after-slices 1 --slice-ticks 4`) | 12 | `signal step=1` |
| missing-cart-file | 13 | `missing role=cart path=absent-cart.ivsc` |
| needs-user-exec (skip-ok, visible SKIP) | 13 | `missing role=exec user` |
| bad-syntax | 14 | no report; stderr `scenario-syntax line=9 reason=unknown-directive` |
| unexpected-refusal | 14 | `load-refused role=cart status=IVS_E_MALFORMED reason=1` |
| expect-load-refusal | 0 | `load-refused-as-expected role=cart status=IVS_E_MALFORMED reason=1` |
| diag-input-pass (inputs at ticks 400 and 1200; run-to 1000 then 16000 ticks; halts at 2084) | 0 | `-` |

Mutants (each exit 10): ADDI-SUBI 5010=0338 `mem 0202 == A0A0 got 1010`; BNZE-BZE 501E=0224 `reg R2 == 0000 got 0009`; PSHR-MVOAT 5022=024B `reg R5 == 0037 got 0000`; INCR-DECR 500B=0010 `reg R1 == 8080 got 0000`.

Judge evidence beyond the controls: disabling each judge rule in a scratch copy (exit code, report.end, pair check, body compare) turned exactly its self-control `not ok`; a wrapper that hangs on timeout-wall with `IVS_RUNNER_PROCESS_TIMEOUT=2` gave `not ok ... hung` in 3 s with no orphan process. In the gate, dev and release reports are equal apart from `host.calls` for every scenario except timeout-wall (host-dependent, as expected).

## Performance (suite runner-process, plain runs, load average 15-23)

- dev (ASan/UBSan, about 0.08-0.09 s per ivs-run start): 2.7-3.2 s as `sh`/`dash` script runs, 2.78 s under ctest.
- release: 0.66-1.1 s script runs, 1.02 s under ctest.
- About 22 real ivs-run invocations plus 5 fake-runner runs and 4 ivs_pack calls. The cost is mostly process start-up, not machine time (slice-1 runs take the same time as default runs). It runs in parallel with the other suites under `ctest -j`, and gate total stayed at 24-27 s.

## Counts

Start: `steps=30/30 controls=263/263 faults=149/149 tests-expected=2724 tests-skipped=0`.
End: `steps=30/30 controls=263/263 fault-runs=149 faults=149/149 tests-expected=2776 tests-executed=2774 tests-skipped=2 tests-missing=0` (+26 ids in `runner-process.ids`, each in both presets; the 2 skips are needs-user-exec in dev and release).

Acceptance checks: `head -1 build/gate/dev/runner/diag-boot-pass.report` = `ivsr 1`; outcomes column = `cancelled guest-fail harness-fail missing-fixture pass timeout`; `grep -v '^#' data/expected/diag-mutations.ids | grep -c .` = 4; `grep -c '^runner-process.judge.' data/expected/tests/runner-process.ids` = 4.

## Deviations from Plan

**1. [Rule 2 - Judge strength] Detail and stderr pinning.** The plan's judge checks exit, outcome/exit.code, `ivsr 1` and `report.end`. Added: exact `outcome.detail` from a `# expect-detail:` comment in each new scenario (`-` without one), and for the parse refusal an exact stderr line (`# expect-stderr:`), because a parse refusal prints no report (docs/runner.md) and could not otherwise be judged. `diag-boot-pass.ivss` is unchanged (it must stay byte-equal to `diag_boot_pass_text`).

**2. Content dir.** Scenario cases run with `--content-dir <temp copy of the images dir>` (plus the truncated cartridge) instead of the images dir itself, so the truncated cartridge never lands in the build tree.

**3. No report file for no-report runs.** `bad-syntax` leaves no `runner/bad-syntax.report` (an empty file would be meaningless to the golden step); every other scenario, including needs-user-exec, writes its report.

**4. Self-control shapes.** The body-differs control runs the repeat check (`check_repeat`) against a fake that replays the recorded diag-boot-pass report with `digest.cpu` zeroed. It uses the same function as the repeat, slices and cwd cases, not a separate comparison.

## Open Items

- Plan 04-21 (golden): reports are in `build/gate/<preset>/runner/<id>.report` for the 10 scenarios with a report (`bad-syntax` has none). Compare dev and release apart from `host.calls`, excluding `timeout-wall` and `cancel-signal` (column 4 `host-dependent`). `cancel-signal` reports are in fact equal across presets, but the plan classes signals as host-dependent. `needs-user-exec` has `content exec user` and therefore no golden.
- Plan 04-23: replays `data/scenarios/diag-input-pass.ivss`: `input at_tick=400 c0=KEY_1 c1=0x0`, `input at_tick=1200 c0=0x0 c1=0x0`, `run-to 1000 ticks`, `run-to 16000 ticks stop halted`. The comment header lines do not affect the replay.
- Plan 04-24 (important-gates) may cite `runner-process.judge.*` as mutants and `runner-process.mutant.*` for D-32.
- Carried: C14 and C63-C68 revisited in Phase 5/6/7; `API-02`, `PKG-03`, `PKG-04`, RUN-04, RUN-05 Pending; `pending-resolvers=17`, `unknown-fields=16`; `sh tools/hygiene.sh --history` still fails on earlier planning commits (this plan adds no home paths); the orchestrator records hosted runs with `sh tools/ci-record.sh <run-id>`; important-gates (04-24) may register the seven runner faults of 04-18/04-19.

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=30/30 controls=263/263 ... cc=AppleClang c-presets=2 fault-runs=149 ... faults=149/149 ... runner-files=8 ... tests-expected=2776 tests-executed=2774 tests-skipped=2 tests-missing=0 ...`, `gate-time total=26`.
- `dash tools/gate.sh`: the same line, `gate: PASS`, `gate-time total=27`.
- `sh tools/gate-linux.sh` (Ubuntu 24.04 container, x86_64, gcc 13.3.0, cmake 3.28.3): `gate: PASS steps=30/30 controls=263/263 cc=GNU c-presets=2 fault-runs=149 faults=149/149 tests-expected=2776 tests-executed=2774 tests-skipped=2 tests-missing=0`, `gate-time total=109`, `gate-linux: exit 0`.

## Threat Flags

None beyond the plan's threat model:
- T-04-59 mitigated: four judge self-controls (each shown to turn `not ok` when its rule is removed), the exact scenario inventory, `outcomes-covered` and the pair check.
- T-04-60 mitigated: every child under the `run_one` watchdog (hang shown to end `hung`); cancel and wall use the deterministic hooks, no sleeps or external signals.
- T-04-61 mitigated: `needs-user-exec` names `user` only, pins no digest, and is skip-ok; mutant and truncated cartridges exist only in the temp dir.
- T-04-SC: no package installs.

## Self-Check: PASSED

- Files exist: tests/runner-process-tap.sh, data/expected/runner-scenarios.ids, data/expected/runner-outcomes.ids, data/expected/diag-mutations.ids, data/scenarios/diag-input-pass.ivss.
- Commits on HEAD: 2dbe7f2, 284cc2f, 1e5a75f.
