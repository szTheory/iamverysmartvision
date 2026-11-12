---
phase: 04-headless-runner-and-installed-consumer
plan: 18
subsystem: runner, tests/runner, data
tags: [runner, executor, report, outcome, RUN-01, RUN-02, RUN-05, D-08, D-09, D-17, D-19, D-20, D-22, D-30]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-03: outcome taxonomy and resolve; 04-06..04-13: ivs_machine_run, cpu_state, peek, input events, timebase in ivs_info; 04-14/04-16: build-tree boot stub and diag-cpu-1.ivsc (ivs_images.inc), data/golden/diag-cpu-1.expect; 04-17: ivs_scenario_parse"
provides:
  - "runner/exec.c: ivs_runner_execute (one scenario, one fresh instance in host storage aligned by hand, public API only) and the pure stop classification ivs_runner_stop_observation"
  - "runner/report.c: ivs_runner_report_write, the closed key table in fixed order (key names pinned below for 04-19, 04-21, 04-23)"
  - "runner/ivs_runner.h: host vtable ivs_runner_host (read_file, read_user, now_ms, cancel_requested, write_report, storage, buffers), ivs_runner_options, ivs_runner_result, the report record, role read caps"
  - "suite runner family exec: 25 cases through a fake host"
  - "faults RUNNER_ASSERT_ALWAYS_PASS, RUNNER_STOP_EXPECT_IGNORED, RUNNER_KEY_ORDER_SWAPPED, RUNNER_OVERSHOOT_NOT_REPORTED"
affects: [04-19, 04-20, 04-21, 04-23, 04-24]

actuals:
  tokens: 26400
  tasks: 3
  commits: 4
plan_head_before: 9d5815ab0beba155b162dd3bc0e5772d19708672
plan_head_after: c19440ee9f32dedfc71b07a5a16ddc1645ea8707

tech-stack:
  added: []
  patterns:
    - "Report as a closed key table walked by one writer, streamed line by line through the host (no report buffer in the library); a test writes the expected key sequence out independently of the table"
    - "Outcome detail = the first detail recorded for the observation bit that wins the precedence, so the detail always explains the printed outcome"

key-files:
  created:
    - runner/exec.c
    - runner/report.c
    - tests/runner/runner_exec.c
  modified:
    - runner/ivs_runner.h
    - runner/CMakeLists.txt
    - tests/runner/runner_main.c
    - tests/runner/runner_cases.h
    - tests/CMakeLists.txt
    - data/expected/tests/runner.ids
    - data/expected/faults/runner.ids

key-decisions:
  - "Every run-to is run with IVS_RUN_F_STOP_ON_HALT, so a halt is always a stop the scenario must expect: halted where target was expected is guest-fail; the bound reached where halted or faulted was expected is timeout (budget), already in the pure classification (04-19 adds slicing and --max-ticks around it); INTERNAL or an unknown stop is harness-fail"
  - "Execution ends after the first step with a stop mismatch or failed assert; that step's asserts are all evaluated and printed; later steps print `-` values and `not-run` asserts. Steps and asserts are numbered from 1; an assert keeps its scenario-wide number (step.<k>.assert.<j>, detail assert=<j>)"
  - "A matching expect-load refusal ends the scenario as pass with detail `load-refused-as-expected ...`; expect-load with a wrong reason, or a load that was not refused, is harness-fail (`load-not-refused`)"
  - "Host vtable: read_file(ctx, relpath, cap, buf, &len) with a cap + 1 buffer (TOO_LARGE hands cap + 1 bytes to the core for its own OVERSIZED refusal); `user` roles go to an optional read_user(ctx, role, ...) (absent: missing-fixture), which 04-19's operator --exec/--grom/--cart flags implement; host.calls counts read_file, read_user, now_ms and cancel_requested calls (not report writes)"
  - "Storage: the instance, aligned by hand at the start of the host buffer, then one read buffer of 131,257 bytes reused per role (the core copies loads); input events (64 x 32 bytes) and the report record live on the executor's stack; output buffers only when the profile's layout is not NONE (Phase 4: NULL/0)"
  - "Value forms: slot status `ok` or the IVS_E_* name, `-` when not attempted; sha256 `-` when not loaded (no hash of nothing); stop tokens target|halted|faulted|internal; cpu.run_state and cpu.fault_reason as tokens; controller words as 8 upper-case hex digits; cycles/frames targets print the ticks first, then `from=<n> cpu-cycles` or `from=<n> frames nominal, not frame events`"
  - "RUN-01, RUN-02, RUN-05 stay Pending: the executor and report exist in process; the executable, slicing, wall timeout and cancellation (04-19), the process suite (04-20) and goldens (04-21) close them"

duration: about 19 min of edits and gates (first commit 18:51, last 19:04), plus two container gates
completed: 2026-11-11
---

# Phase 4 Plan 18: Runner executor and report writer Summary

**The runner library now executes a parsed scenario on one fresh, hand-aligned instance through the public API only, classifies the result through the pure outcome precedence (with the stop classification as its own pure function), and writes a deterministic, path-free report from a closed key table; the diagnostic scenario passes in process through a fake host, every non-clock outcome is demonstrated, and a runner that cannot fail, ignores stop expectations, reorders its keys or hides overshoot is caught by a named fault.**

## What was built

- `runner/exec.c` (listed in `IVS_RUNNER_SOURCES`; runner-scan `runner-files=5`, ISO headers and `ivs/ivs.h` only, no mutable statics, no allocation): storage requirements, create in the host buffer, read and load exec, grom, cart (with `IVS_LOAD_F_EXPECT_SHA256` for `expect-sha256`), reset, timebase from `ivs_info` (`n * ticks_per_cpu_cycle`, `n * nominal_frame_ticks`, overflow above `IVS_RUN_TARGET_MAX` is harness-fail), the input schedule converted to absolute ticks and passed as the unconsumed suffix each step, one `ivs_machine_run` per `run-to`, asserts through `ivs_machine_cpu_state`, `ivs_machine_peek` (ranges in 64-word chunks) and `ivs_machine_state_digest`, then the final CPU state and the eight digests, destroy, report.
- `runner/report.c`: the key tables below, `snprintf` into a 640-byte line, one `write_report` per line; a write error makes the result harness-fail with `report_write_failed=1`.
- Tracer `runner.exec.diag-pass`: the diag-boot-pass text (the 18 `.expect` lines through `diag_cpu_1_expect_text.inc`) with the fake host serving `boot-stub.exec` and `diag-cpu-1.ivsc` from `ivs_images.inc`: pass, exit 0, 66 lines before `report.end`, all 18 asserts `pass`, stop `halted` at 2084 ticks, the exec label carrying the loaded stub's sha256.

## Report key table (fixed order, all keys always present, `-` when not applicable)

| Key | Value |
|-----|-------|
| `ivsr` | `1` |
| `scenario.id`, `scenario.profile` | scenario tokens |
| `api.version` | `0.1.0` from `ivs_api_version()` |
| `outcome` | `pass`, `guest-fail`, `timeout`, `cancelled`, `missing-fixture`, `harness-fail` |
| `outcome.detail` | `-` or one detail: `assert=<j> line=<n> <clause> got <actual>`, `stop step=<k> line=<n> expected=<t> got=<t>`, `budget step=...`, `stop-internal step=...`, `missing role=<r> path=<rel>` / `user`, `not-regular role=...`, `io-error role=...`, `load-refused role=<r> status=<IVS_E_*> reason=<n>`, `load-refused-as-expected ...`, `load-not-refused role=<r> expected=<IVS_E_*>`, `profile-refused status=...`, `create-refused`, `storage-too-small`, `reset-refused`, `run-refused step=<k> status=... reason=<n>`, `bound-too-large step=...`, `input-out-of-range line=<n>`, `no-timebase`, `observe-refused ...`, `signal step=<k>` |
| `exit.code` | 0, 10, 11, 12, 13, 14 |
| `slot.<exec\|grom\|cart>.loaded` / `.length_bytes` / `.sha256` / `.status` / `.reason` | 0/1; bytes; 64 hex or `-`; `ok` or `IVS_E_*` or `-`; reason number or `-` |
| `exec:` | `project boot stub, not an EXEC sha256=<hex>` (only for `content exec boot-stub`), else `-` |
| `grom:` | `zero image, not a GROM` (only for `content grom zero-grom`), else `-` |
| `step.<k>.target_ticks` | ticks; plus `from=<n> cpu-cycles` or `from=<n> frames nominal, not frame events` |
| `step.<k>.now_ticks` / `.overshoot_ticks` / `.stop_reason` / `.expected_stop` | numbers; `target\|halted\|faulted\|internal`; `target\|halted\|faulted` |
| `step.<k>.assert.<j>` | `pass\|fail\|not-run <subject> <op> <expected> got <actual>` |
| `cpu.run_state`, `cpu.fault_reason` | `unreset\|reset-pending\|running\|halted\|faulted`; `none` or a fault token |
| `input.events`, `input.consumed`, `input.controller.<0..3>` | counts; 8 upper-case hex or `-` before any run |
| `digest.cpu`, `.mem`, `.images`, `.instance`, `.composite`, `.video`, `.audio`, `.input` | 64 lower-case hex |
| `output.video.bytes`, `output.audio.bytes` | byte counts (0 in Phase 4) |
| `host.calls` | host vtable calls (the one host-dependent key) |
| `report.end` | number of lines before it |

## Tests (suite runner, family exec: 25 ids, each in both presets)

diag-pass; guest-assert-mismatch (exit 10, `assert=2 line=9 reg R0 == 03C4 got 03C5`); stop-mismatch (`stop target` and the default); stop-internal (the pure classification table: INTERNAL, 0 and 5 harness-fail for every expectation, the three matches, budget for target-when-halted/faulted, guest-fail for the four other pairs; resolve gives exit 14); halt-bound-small (timeout budget, exit 11, though the asserts also fail); missing-file (cart, then the reserved stub name), not-regular, user-role (exit 13); io-error, unexpected-refusal (truncated cart), too-large (cap + 1 bytes reach the core's OVERSIZED), sha-mismatch, unknown-profile, write-error (exit 14); expect-load-refusal (status, status+reason, wrong reason, not refused), sha-match (pass); cancel (exit 12); key-order (expected sequence written out in the test); no-paths (printable ASCII, no `/`, single spaces, no time/wall/pid word, on a pass and a missing-fixture report); repeat-identical; overshoot (`run-to 2 cpu-cycles`: overshoot = now - target > 0); units (4000 cycles = 4000 x ticks_per_cpu_cycle, 1 frame = nominal_frame_ticks with the label, both read from a separate instance's `ivs_info`); input-schedule (two `at_cycle` lines: only `digest.input` and the input lines differ; controller words `10001000` and `00000800`); two-steps (checkpoint at 100 cycles, numbering 1..20, a failed step-1 assert leaves step 2 not run); assert-forms (`!=` on each subject, ranges, a digest assert, range failure names `0201=8080`).

Fault behaviour (faults-verdict): RUNNER_ASSERT_ALWAYS_PASS fails guest-assert-mismatch, assert-forms, halt-bound-small, two-steps; RUNNER_STOP_EXPECT_IGNORED fails stop-mismatch, stop-internal, halt-bound-small; RUNNER_KEY_ORDER_SWAPPED fails key-order; RUNNER_OVERSHOOT_NOT_REPORTED fails overshoot. All keep `runner.parse.diag-boot-pass` green.

## Performance

- Plain-run times (wall, load average 13-14): suite `runner` dev 0.47-0.48 s, release 0.19 s (04-17: 0.56 s and 0.21 s at load 14-16); family exec alone dev 0.10-0.11 s, release 0.02 s.
- Fault runs: four more rebuilds and reruns of suite runner (`fault-runs=146`); c-faults step 5-6 s locally.
- Gate totals: Task 1 26 s, Task 2 28 s (failed, deviation 1) and 30 s, Task 3 23 s (sh) and 23 s (dash), final 26 s (sh) and 28 s (dash); `gate-cpu user=53.07 sys=46.13`; container 98 s (failed run, deviation 2) and 122 s (passing run).

## Counts

Start: `steps=30/30 controls=263/263 faults=142/142 tests-expected=2652`.
End: `steps=30/30 controls=263/263 fault-runs=146 faults=146/146 tests-expected=2702 tests-executed=2702 tests-skipped=0 tests-missing=0 runner-files=5` (+25 ids in `runner.ids`, 136 total, each in both presets).

Acceptance checks: `grep -c 'project boot stub, not an EXEC' runner/report.c runner/exec.c` sums to 1 (report.c); `grep -c '^runner.exec.stop-internal$' data/expected/tests/runner.ids` = 1; `grep -c -E '^RUNNER_(ASSERT_ALWAYS_PASS|STOP_EXPECT_IGNORED|KEY_ORDER_SWAPPED|OVERSHOOT_NOT_REPORTED) ' data/expected/faults/runner.ids` = 4.

## Deviations from Plan

**1. [Rule 1 - Registry] Outcome faults gained collateral detections.** The new executor cases see the exit codes of OUTCOME_GUEST_AS_HARNESS, OUTCOME_TIMEOUT_AS_GUEST and OUTCOME_MISSING_FIXTURE_EXIT0, so faults-verdict reported `fault-wrong-reason` for them; `runner.exec.` was added to their allowed prefixes (with comments) in the Task 2 commit `0c99fb2`. Their must-fail ids are unchanged.

**2. [Rule 1 - Bug] gcc 13 `-Werror=format-truncation` in the test (found by `tools/gate-linux.sh`).** Two `snprintf` destinations in `tests/runner/runner_exec.c` (the label check and the sha-match scenario line) were smaller than the 512-byte source buffer; Apple clang does not diagnose it. Both widened, commit `c19440e`; the container gate then passed.

**3. Fault sites deferred to Task 3** (as in 04-17): `faults-verdict` fails an orphan `IVS_FAULT_*` site, so the four sites were added with their registry lines in `302709e`.

**4. Interface details beyond the plan text:** an optional `read_user` host hook for `user` roles (the plan's `read_file` takes a relative path, and 04-19's operator flags need a separate source); `ivs_runner_result` (outcome, exit code, host calls, line count, write failure); the report record and `ivs_runner_report_write` declared in `ivs_runner.h` as library-internal (no new header); the files `tests/runner/runner_cases.h` (family interface) modified. The TDD flow of Task 2 ran against an executor already implemented in Task 1, so its RED evidence is the Task 3 fault runs rather than a separate failing commit.

**5. Added beyond the plan:** cases too-large, io-error, unknown-profile, cancel, write-error, sha-match, units, two-steps, assert-forms, halt-bound-small (the budget classification is pure and already decided here; 04-19 adds slicing and `--max-ticks`).

## Open Items

- Plan 04-19: slices of at most `1<<22` ticks inside each `run-to` (the input suffix is already resubmitted per call), `--max-ticks`, the wall cap (`now_ms`, `wall-timeout-ms` only lowering it) and cancellation between slices (the executor checks `cancel_requested` once per step now); `ivs_runner_options` (flags/reserved, must be 0) is where those settings go; `main.c`/`hostio.c` implement `read_file` under `--content-dir`, `read_user` from `--exec/--grom/--cart`, print refusals with `ivs_scenario_error_format`, and write `data/scenarios/diag-boot-pass.ivss` (its text should equal `diag_boot_pass_text` of `runner_parse.c`).
- Plan 04-21: `.digests` goldens filter the keys above; `host.calls` is the only key to drop for dev-vs-release and slice comparisons. Plan 04-23: the consumer compares `digest.<domain>` lines.
- `docs/runner.md` (04-19) should carry the key table above and the detail tokens.
- RUN-01, RUN-02, RUN-05 stay Pending (executable, timeouts, goldens). Important-gates (04-24) may register the four new faults.
- Carried: C14 and C63-C68 revisited in Phase 5/6/7; `API-02`, `PKG-03`, `PKG-04`, RUN-04, RUN-06 Pending; `pending-resolvers=17`, `unknown-fields=16`; `sh tools/hygiene.sh --history` still fails on earlier planning commits (this plan's commits add no home paths); the orchestrator records hosted runs with `sh tools/ci-record.sh <run-id>`.

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=30/30 controls=263/263 ... cc=AppleClang c-presets=2 fault-runs=146 ... faults=146/146 ... runner-files=5 ... tests-expected=2702 tests-executed=2702 tests-skipped=0 tests-missing=0 ...`, `gate-time total=26`.
- `dash tools/gate.sh`: the same line, `gate: PASS`, `gate-time total=28`.
- `sh tools/gate-linux.sh` (Ubuntu 24.04 container, gcc 13.3.0, cmake 3.28.3): first run `gate: FAIL steps=24/30` (deviation 2); after `c19440e`: `gate: PASS steps=30/30 controls=263/263 cc=GNU fault-runs=146 faults=146/146 tests-expected=2702 tests-executed=2702 tests-skipped=0 tests-missing=0`, `gate-time total=122`, `gate-linux: exit 0`.

## Threat Flags

None beyond the plan's threat model:
- T-04-52 mitigated: closed key table, no wall time, paths or environment; `runner.exec.no-paths` and `runner.exec.key-order`; RUNNER_KEY_ORDER_SWAPPED caught.
- T-04-53 mitigated: outcome via `ivs_runner_outcome_resolve` and the pure stop classification; RUNNER_ASSERT_ALWAYS_PASS and RUNNER_STOP_EXPECT_IGNORED caught; wrong-expectation cases guest-fail.
- T-04-54 mitigated: the exec label is derived only from `content exec boot-stub` (report.c), its hash from the loaded bytes; a path-named EXEC prints `exec: -`.
- T-04-SC: no package installs.

## Self-Check: PASSED

- Files exist: runner/exec.c, runner/report.c, tests/runner/runner_exec.c.
- Commits on HEAD: 5bbbfac, 0c99fb2, 302709e, c19440e.
