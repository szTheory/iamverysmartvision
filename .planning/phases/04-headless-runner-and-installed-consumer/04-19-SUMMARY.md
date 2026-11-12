---
phase: 04-headless-runner-and-installed-consumer
plan: 19
subsystem: runner, tests/runner, data, docs
tags: [runner, ivs-run, hostio, slicing, timeout, cancellation, RUN-01, RUN-02, D-17, D-20, D-21, D-22, D-23, D-24]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-17: ivs_scenario_parse; 04-18: ivs_runner_execute, report writer, host vtable, ivs_runner_options placeholder; 04-14/04-16: build-tree images (build/<preset>/images)"
provides:
  - "ivs-run (target ivs_run, runner/main.c + runner/hostio.c), built in both presets, not installed"
  - "runner/hostio.c: the only POSIX file (open O_NONBLOCK then fstat regular-file check, reads bounded at cap+1, CLOCK_MONOTONIC ms, signal() handler setting a volatile sig_atomic_t, SIGPIPE ignored); runner/hostio.h its interface"
  - "Slicing in the executor: every run-to as slices of at most slice_ticks (default 1<<22) with the unconsumed input suffix; cancel_requested then now_ms once before every slice; --max-ticks ceiling as timeout budget; wall cap as timeout wall; scenario wall-timeout-ms only lowers"
  - "ivs_runner_options {flags, wall_timeout_ms, slice_ticks, max_ticks} (0 = default; out of range: harness-fail without a report)"
  - "suite runner family time: 11 runner.time.* cases with a fake clock and fake cancel"
  - "faults RUNNER_CANCEL_IGNORED, RUNNER_WALL_AS_GUEST_FAIL, RUNNER_SLICE_DROPS_REMAINDER"
  - "data/scenarios/diag-boot-pass.ivss (the first tracked scenario), docs/runner.md"
affects: [04-20, 04-21, 04-23, 04-24]

actuals:
  tokens: 16456
  tasks: 3
  commits: 3
plan_head_before: 2ee9358f7f6dda440e0c0c19e7c763284306442c
plan_head_after: 06321957581ac589b9d6d308bfa72e1384af81b1

tech-stack:
  added: []
  patterns:
    - "POSIX confined to one TU with its own small header (hostio.h); main.c stays ISO C (signal.h raise only)"
    - "Deterministic signal hook: count cancel_requested calls (one per slice by contract) and raise(SIGINT) synchronously in the check after slice N"
    - "Fake clock = fixed step per now_ms call, so the wall check fires at an exactly computable check"

key-files:
  created:
    - runner/hostio.c
    - runner/hostio.h
    - runner/main.c
    - tests/runner/runner_time.c
    - data/scenarios/diag-boot-pass.ivss
    - docs/runner.md
  modified:
    - runner/exec.c
    - runner/ivs_runner.h
    - runner/CMakeLists.txt
    - tests/runner/runner_main.c
    - tests/runner/runner_cases.h
    - tests/CMakeLists.txt
    - data/expected/tests/runner.ids
    - data/expected/faults/runner.ids

key-decisions:
  - "Slice loop: target = min(limit, now + slice_ticks), limit = min(run-to bound, max_ticks); at least one call per run-to; ends at a stop other than TARGET or once now >= limit; overshoot is computed against the run-to bound (equals the single-call value); machine time starts from ivs_info.time_ticks after reset"
  - "--max-ticks (default 2^40) cutting a run-to short is timeout with detail `budget step=<k> line=<n> max-ticks=<m>` (the step's asserts not-run), not the harness-fail bound-too-large of DISCUSS-runner; plan text and D-21 (a ceiling) win"
  - "Wall cap: now_ms once at the start of the first step, then once before every slice after cancel_requested; elapsed >= cap is `wall step=<k> cap-ms=<n>`; both cancel and wall at one check are both observed and resolve to cancelled; no now_ms in the vtable means no wall cap"
  - "ivs-run: --content-dir defaults to the scenario file's directory (never the cwd); usage errors, an unreadable scenario and a parse refusal are exit 14 with one stderr line and no report; a failed report write or fflush is exit 14"
  - "--self-signal-after-slices N raises SIGINT in the (N+1)-th cancel check (N=0: before the first slice); the run ends cancelled whenever it has more than N slices"

duration: about 17 min of edits and gates (first commit 19:11, last 19:23), plus one container gate
completed: 2026-11-11
---

# Phase 4 Plan 19: ivs-run executable, slicing, timeouts and cancellation Summary

**`ivs-run` executes `data/scenarios/diag-boot-pass.ivss` from the command line (exit 0, 66-line report); every run-to now runs as slices with a deterministic machine-time budget, a host-only wall cap and signal cancellation checked between slices, all proven with a fake clock in process and three named faults; POSIX lives only in `runner/hostio.c`, and `docs/runner.md` documents the grammar, report keys, outcomes, timing semantics, hooks and trust model.**

## What was built

- Task 1 (tracer, `7f25e72`): `runner/hostio.c` + `hostio.h`, `runner/main.c`, `ivs_run` in `runner/CMakeLists.txt` (OUTPUT_NAME `ivs-run`, links `ivs_runner_lib ivs_core`, no install rule). `IVS_RUNNER_SOURCES` stays library-only (fault framework); new `IVS_RUN_SOURCES` (hostio.c, main.c); `ivs-runner-sources.txt` lists both for c-build's unlisted-source guard. `data/scenarios/diag-boot-pass.ivss` is byte-equal to `diag_boot_pass_text` of `runner_parse.c` (the 7 header/run-to lines plus the 18 `assert` lines of the `.expect`).
- Task 2 (`6afb650`): the slice loop and option handling in `runner/exec.c`, the options struct in `ivs_runner.h`, the four timing options in `main.c`, family time in `tests/runner/runner_time.c` (own fake host; serves the build-tree images).
- Task 3 (`0632195`): three fault sites in `exec.c` with registry lines, `docs/runner.md`.

Manual checks: `--slice-ticks 4` gives the same report but `host.calls` (143 vs 5); `--self-signal-after-slices 1 --slice-ticks 4` exit 12 `signal step=1`; `--max-ticks 400` exit 11 `budget step=1 line=7 max-ticks=400`, now 404; a long-run scratch scenario (boot stub, no cart, `run-to 1000000000000 ticks`) with `--wall-timeout-ms 1` exit 11 `wall step=1 cap-ms=1` after 0.09 s; `--slice-ticks 0`, `--max-ticks 2^63+1` and `--bogus` exit 14.

## Tests (suite runner, family time: 11 ids, each in both presets)

budget (bound before the halt, `stop halted`: timeout budget, exit 11, stop time equal for slices 64 and 1 and one call); cancel (cancel from check 3: exit 12, `signal step=1`, two slices ran, asserts not-run); cancel-over-wall (both at check 3: cancelled); check-per-slice (two one-slice steps: 2 cancel and 3 now_ms calls, `host.calls 7`); max-ticks (400: budget with `max-ticks=400`, cut at the first boundary; 3000: pass at 2084); no-clock (no now_ms, slice 1, cap 1 ms: pass); options-refused (slice 1<<22+1, max 2^63+1, flags 1: harness-fail, no report); slice-sizes-equal (sizes 1, 7, 1<<22) and slices-equal (300, 1000) over diag-boot-pass and a two-step target scenario with two input events (reports equal except `host.calls`); wall (step 3 ms, cap 10: `wall step=1 cap-ms=10` at the fourth check, exit 11); wall-only-lowers (scenario 100000 vs cap 10: cap-ms=10; scenario 5 vs cap 1000: cap-ms=5; default 30000).

Fault behaviour (faults-verdict): RUNNER_CANCEL_IGNORED fails runner.time.cancel (plus cancel-over-wall, check-per-slice, runner.exec.cancel); RUNNER_WALL_AS_GUEST_FAIL fails runner.time.wall; RUNNER_SLICE_DROPS_REMAINDER fails runner.time.slices-equal. All keep `runner.parse.diag-boot-pass` green.

## Budget checkpoint (D-45): local measurements

- Suite `runner` plain run (wall, load average 14.1-14.8): dev 0.54-0.55 s, release 0.23-0.25 s (04-18: 0.47-0.48 s and 0.19 s); family time alone dev 0.09 s, release 0.02 s.
- Gate totals (`gate-time`): Task 1 24 s; Task 2 33 s (failed, deviation 1) and 30 s; Task 3 27 s (sh) and 30 s (dash). `gate-cpu` final sh run `user=58.28 sys=50.14`, dash `user=59.94 sys=52.93` (04-18: 53.07/46.13).
- Container (`tools/gate-linux.sh`, x86_64 under emulation): `gate-time total=120`, wall 2:56.
- Fault runs 146 -> 149 (three more reruns of suite runner).

## Counts

Start: `steps=30/30 controls=263/263 faults=146/146 tests-expected=2702 runner-files=5`.
End: `steps=30/30 controls=263/263 fault-runs=149 faults=149/149 tests-expected=2724 tests-executed=2724 tests-skipped=0 tests-missing=0 runner-files=8 docs=14` (+11 ids in `runner.ids`, 147 total, each in both presets).

Acceptance: `grep -l '_POSIX_C_SOURCE' runner/*.c` lists only `runner/hostio.c`; `ivs-run --help` prints `pass 0`, `guest-fail 10`, `timeout 11`, `cancelled 12`, `missing-fixture 13`, `harness-fail 14` and exits 0; `grep -c '^runner.time.' data/expected/tests/runner.ids` = 11; the three-fault grep = 3.

## Deviations from Plan

**1. [Rule 1 - Registry] Existing faults gained collateral detections in runner.time.** faults-verdict reported `fault-wrong-reason` for OUTCOME_TIMEOUT_AS_GUEST (budget, max-ticks, wall see exit 10), OUTCOME_CANCEL_AS_TIMEOUT (cancel-over-wall resolves through the executor) and RUNNER_STOP_EXPECT_IGNORED (budget). Their allowed prefixes gained `runner.time.`, `runner.time.cancel-over-wall` and `runner.time.budget` respectively, with comments, in `6afb650`; must-fail ids unchanged.

**2. [Rule 3 - Blocking] New header `runner/hostio.h`** (not in `files_modified`): runner-scan only accepts quoted includes of tracked runner headers, and `-Wmissing-prototypes` needs declarations for the hostio functions; the library header was not the place for host-only calls. runner-files 5 -> 8.

**3. [Rule 2 - Design] `--max-ticks` semantics follow the plan, not DISCUSS-runner section 6** (which made a larger bound harness-fail `bound-too-large`): the ceiling cuts the run and reports timeout budget with `max-ticks=<m>`. Also `--content-dir` defaults to the scenario's directory (DISCUSS-runner section 7; the plan names no default) and SIGPIPE is ignored so a closed stdout is a checked write error (DISCUSS section 10).

**4. Fault sites deferred to Task 3** (as in 04-17 and 04-18): an orphan `IVS_FAULT_*` site fails faults-verdict. The TDD flow of Task 2 ran against the loop written in the same task; its RED evidence is the Task 3 fault runs.

## Open Items

- Plan 04-20 (suite runner-process): run the real `ivs-run` for each scenario of `data/expected/runner-scenarios.ids`; the wall case needs a long-run scenario (a stub-only `run-to 1000000000000 ticks` with `--wall-timeout-ms 1` ended `wall` in 0.09 s locally); dev vs release byte equality and `--slice-ticks` variants must drop `host.calls`; `--help` and docs/runner.md agree on the six pairs.
- With System V `signal()` semantics (strict `_POSIX_C_SOURCE` on glibc) the handler resets after the first signal, so a second SIGINT ends the process at once (documented).
- RUN-01 and RUN-02: the executable delivers both from the command line (one scenario, one fresh instance, six outcomes with distinct exit codes, timeout budget/wall and cancellation distinct); left Pending in REQUIREMENTS.md until the process suite (04-20) proves them through the real executable and its exit codes. RUN-05 Pending (goldens, 04-21).
- Carried: C14 and C63-C68 revisited in Phase 5/6/7; `API-02`, `PKG-03`, `PKG-04`, RUN-04, RUN-06 Pending; `pending-resolvers=17`, `unknown-fields=16`; `sh tools/hygiene.sh --history` still fails on earlier planning commits (this plan adds no home paths); the orchestrator records hosted runs with `sh tools/ci-record.sh <run-id>`; important-gates (04-24) may register the seven runner faults of 04-18/04-19.

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=30/30 controls=263/263 ... cc=AppleClang c-presets=2 fault-runs=149 ... faults=149/149 ... runner-files=8 ... tests-expected=2724 tests-executed=2724 tests-skipped=0 tests-missing=0 ...`, `gate-time total=27`.
- `dash tools/gate.sh`: the same line, `gate: PASS`, `gate-time total=30`.
- `sh tools/gate-linux.sh` (Ubuntu 24.04 container, x86_64, gcc 13.3.0, cmake 3.28.3): `gate: PASS steps=30/30 controls=263/263 cc=GNU c-presets=2 fault-runs=149 faults=149/149 runner-files=8 tests-expected=2724 tests-executed=2724 tests-skipped=0 tests-missing=0`, `gate-time total=120`, last line `gate-linux: exit 0 (log build/gate-linux.log)`.

## Threat Flags

None beyond the plan's threat model:
- T-04-55 mitigated: `open(O_NONBLOCK)` then `fstat` on the descriptor, only regular files read, reads bounded at cap+1, wall watchdog.
- T-04-56 mitigated: the handler only sets a `volatile sig_atomic_t`; the flag is read between slices; RUNNER_CANCEL_IGNORED caught.
- T-04-57 mitigated: budget and wall are distinct details and observation bits; RUNNER_WALL_AS_GUEST_FAIL caught; `host.calls` and `wall` details are never golden.
- T-04-58 mitigated: `_POSIX_C_SOURCE` only in `runner/hostio.c` (runner-scan), `tools/gate-linux.sh` passed before push.
- T-04-SC: no package installs.

## Self-Check: PASSED

- Files exist: runner/hostio.c, runner/hostio.h, runner/main.c, tests/runner/runner_time.c, data/scenarios/diag-boot-pass.ivss, docs/runner.md.
- Commits on HEAD: 7f25e72, 6afb650, 0632195.
