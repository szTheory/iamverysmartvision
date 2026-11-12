---
phase: 04-headless-runner-and-installed-consumer
reviewed: 2026-11-11T00:00:00Z
depth: standard
files_reviewed: 33
files_reviewed_list:
  - core/include/ivs/ivs_run.h
  - core/include/ivs/ivs_machine.h
  - core/src/machine/machine.c
  - core/src/machine/machine.h
  - core/src/machine/machine_bound.c
  - core/src/machine/machine_input.c
  - core/src/machine/machine_inspect.c
  - core/src/machine/machine_digest.c
  - core/src/machine/machine_run.c
  - core/CMakeLists.txt
  - CMakeLists.txt
  - cmake/ivsConfig.cmake
  - runner/hostio.c
  - runner/hostio.h
  - runner/main.c
  - runner/exec.c
  - runner/scenario.c
  - runner/report.c
  - runner/outcome.c
  - runner/ivs_runner.h
  - runner/CMakeLists.txt
  - tools/pack/pack.c
  - tools/gate.sh
  - tools/ci-record.sh
  - tools/hygiene.sh
  - tools/check/ci-summary.sh
  - tools/check/run-controls.sh
  - tools/check/steps/c-tests.sh
  - tools/check/steps/golden.sh
  - tools/check/steps/runner-scan.sh
  - tools/check/steps/pkg-install.sh
  - tools/check/steps/faults-verdict.sh
  - tools/check/steps/denylist.sh
findings:
  critical: 0
  warning: 4
  info: 11
  total: 15
  necessity: 1
status: issues_found
---

# Phase 4: Code Review Report

**Reviewed:** 2026-11-11
**Depth:** standard
**Files Reviewed:** 33. Also read but not reviewed line by line: tests/installed-consumer/{CMakeLists.txt,main.c}, tests/runner-process-tap.sh, tests/pack-tap.sh, tools/check/steps/{ci-log,important-gates,listing,ci-parity}.sh, the data/scenarios/*.ivss files and data/expected/runner-scenarios.ids.
**Status:** issues_found

## Summary

I compared the run, observe, input and digest core against the D-01..D-16 contract and amendments A-01..A-04. The refusal ladder order matches the header. No stage writes before validation. The uintptr_t span arithmetic cannot wrap. The whole event array is validated before the single read that applies it. BUFFER_TOO_SMALL precedes the first unit. The struct sizes are fixed on every ABI (36, 32 and 112 bytes; the request has a pointer-free 16-byte prefix). I found no UB, no uninitialised read and no unsequenced argument evaluation in the core, the runner or the fuzz tests. The core files use no libc I/O, allocation or clock.

The defects are in the runner's outcome logic and its host layer. I proved each one by running probes against `build/release/runner/ivs-run` and a gcc:13 container:

- **Budget misclassification:** a run-to that reached its target is still reported as a timeout when `--max-ticks` is below the target (WR-01).
- **Vacuous passes:** a matched `expect-load` refusal skips the steps, the asserts and the remaining content roles. A scenario with no run-to also passes, with exit 0 (WR-02, WR-03).
- **Signals on Linux:** `signal()` in `hostio.c` gets System V semantics on glibc under strict C17. The handler resets after the first signal and SA_RESTART is off, so Linux and macOS behave differently (WR-04).

The shell gate changes (parallel ctest, the batched denylist, hygiene and faults-verdict passes, the control-runner rewrite) are correct POSIX sh. I found no fail-open path in them. The remaining items are Info.

`tools/gate-linux.sh` (ubuntu 24.04, gcc 13, dash, cmake 3.28) passes on this tree: `gate: PASS steps=33/33 controls=276/276`, `faults=149/149`, `tests-executed=2778`, `tests-skipped=2`, `pkg-selftest=10/10`, `cxx-link=ok`. Nothing in this review is a hosted-only build or test failure; the findings are classes the current gate does not test.

## Warnings

### WR-01: A run-to that reached its target is reported as a budget timeout when --max-ticks is below the target

**File:** `runner/exec.c:483`, `runner/exec.c:540-546`
**Issue:** `limit = min(target_ticks, max_ticks)`. After the slice loop, the step is classified as a budget timeout whenever `stop == IVS_STOP_TARGET && limit < sr->target_ticks`. The check never asks whether machine time actually reached `target_ticks`. The last unit's overshoot can carry `now` past the target even when the loop stopped on the ceiling.

docs/runner.md:108 and :136 say the budget outcome means the ceiling "cut a run-to short". Here the run-to was not cut short: the machine state is identical to a passing run.

Probe (scenario `run-to 100 ticks`, boot stub, zero GROM, diag cartridge):

```
ivs-run probe.ivss                  -> outcome pass,    step.1.now_ticks 104
ivs-run --max-ticks 99 probe.ivss   -> outcome timeout, budget step=1 line=7 max-ticks=99, step.1.now_ticks 104
```

Failure scenario: an operator lowers `--max-ticks` as a safety ceiling. A scenario whose final unit straddles the ceiling flips from pass (exit 0) to timeout (exit 11) with an identical machine. The outcome then depends on a host option, not on the guest.

**Fix:**
```c
if (stop == IVS_STOP_TARGET && limit < sr->target_ticks && x->now < sr->target_ticks) {
  /* the ceiling really cut this run-to short */
```
Add a runner_time case where the ceiling sits inside the last unit before the target.

### WR-02: A matched expect-load refusal ends the scenario as pass, so later run-to, assert and content lines never run

**File:** `runner/exec.c:252-256`, `runner/scenario.c:381-422` (no rule against steps after an expected refusal), `data/scenarios/expect-load-refusal.ivss`
**Issue:** `ex_load_all` returns 0 as soon as one role's refusal matches `expect-load`. The executor then skips:

- the content roles after it (they are never read, so a missing or misspelled file is never noticed);
- the reset;
- every `run-to`;
- every `assert`.

The outcome stays pass because no observation bit is set. The parser accepts steps and asserts in such a scenario. The tracked scenario `expect-load-refusal.ivss` already carries a dead `run-to 4000 cpu-cycles stop halted` and a dead `assert halted == yes`: they read as checked but are never evaluated. This is the "can pass vacuously" class that D-46 calls important.

Probe:

```
content cart nonexistent-but-unread.ivsc
expect-load exec IVS_E_MALFORMED      (exec is a 100-byte truncated boot stub)
run-to 4000 cpu-cycles stop faulted
assert reg R0 == 1234
-> outcome pass, exit.code 0, slot.cart.status -, step.1.assert.1 not-run
```

**Fix:** Make the combination unrepresentable or loud. Either option works:

- In `ivs_scenario_parse`, after the loop, refuse a scenario in which any `expect-load` names a non-ok status and the scenario also has a `run-to` or a content line for a later role (a new reason such as `IVS_SCN_R_STEPS_AFTER_REFUSAL`).
- In the executor, observe HARNESS_FAIL when an expected refusal ends the scenario while `n_steps > 0`.

Then drop the dead lines from `data/scenarios/expect-load-refusal.ivss`.

### WR-03: A scenario with no run-to passes with exit 0

**File:** `runner/scenario.c:869-875`, `runner/exec.c:446-471`
**Issue:** After parsing, the only completeness checks are the header, `id` and `profile`. A scenario with content lines and no `run-to` passes. The executor creates, loads and resets, runs zero steps and evaluates zero asserts, and the result is `outcome pass` with `exit.code 0`.

Probe:

```
ivss 1 / id probe3 / profile intv-2609-ntsc-family-p1 / content exec boot-stub / content grom zero-grom
-> outcome pass, exit.code 0
```

Failure scenario: a scenario file whose `run-to` lines were lost, for example by a bad merge or an edit that removed the bound, still gates green and keeps its `runner-scenarios.ids` row "pass". The only remaining signal is the absent `step.*` keys, which no judge requires.

**Fix:** Refuse it in the parser (`IVS_SCN_R_MISSING_DIRECTIVE` with line 0 when `n_steps == 0`), except for scenarios whose purpose is an expected load refusal, if WR-02 keeps that form. Add a parser table row.

### WR-04: signal() in hostio.c gets System V semantics on glibc: the handler resets after one signal and SA_RESTART is off, unlike macOS

**File:** `runner/hostio.c:8`, `runner/hostio.c:37-48`, `runner/hostio.h:11-13`
**Issue:** `hostio.c` defines only `_POSIX_C_SOURCE 200809L` and is built with `-std=c17`, extensions off. Under those flags glibc does not define `__USE_MISC`, so `signal()` is redirected to `__sysv_signal`, which installs the handler with `SA_RESETHAND | SA_NODEFER` and without `SA_RESTART`. Apple libc gives BSD semantics.

I ran the same probe on both platforms. The probe calls `signal(SIGINT, h)` and `raise(SIGINT)` twice.

```
macOS (Apple clang, -std=c17): flags_resethand=0 restart=1, survives the second raise (n=2)
gcc:13 container (-std=c17):   flags_resethand=1 restart=0, killed by the second raise (exit 130)
```

Consequences on the hosted platform:

1. A second SIGINT or SIGTERM kills `ivs-run` with no report and exit 130 or 143, not cancelled (12). docs/runner.md:138 admits this, but `hostio.h:11` still states that the handler "only sets the cancel flag".
2. Without SA_RESTART, a signal that lands while `mh_write`'s `fwrite` is blocked on a full stdout pipe makes `write(2)` fail with EINTR. stdio sets the error flag, and `main.c:504` reports "report write failed" (harness-fail 14) instead of cancelled (12).
3. The cancel path therefore behaves differently on macOS and Linux. The deterministic test hook (one `raise`) cannot see either difference. D-23 chose `signal()` over `sigaction`; that choice can stay.

**Fix:** Keep `signal()` (D-23) and request BSD semantics explicitly in the one file allowed to set feature macros. runner-scan exempts `runner/hostio.c`.

```c
#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE 1 /* glibc: BSD signal() semantics (no SA_RESETHAND, SA_RESTART) */
```

Then correct `hostio.h:11` and the docs/runner.md:138 sentence. Alternatively, re-arm with `signal(sig, hostio_on_signal)` inside the handler (async-signal-safe) and treat EINTR from stdout as retryable in `mh_write`.

## Info

### IN-01: An INTERNAL stop can leave input applied without a unit or a floor update

**File:** `core/src/machine/machine_bound.c:426-435`, `core/include/ivs/ivs_run.h:98-101`
**Issue:** Due events are applied (words, count and chain change) before `ivs_machine_step_unit_internal` runs. If the seam then refuses, the loop breaks with INTERNAL. The comment at line 433 says "nothing changed, the state is the last whole boundary", but the INPUT row has changed, the floor has not, and `input_consumed` counts events that no unit saw. The refusal is reachable only through a CPU-invariant refusal or the RUN_FAULT_STEPS build, so this is a contract inaccuracy rather than a live bug.
**Fix:** Apply the events into a local copy of `obs.input` and commit it only after the seam returns IVS_OK. Otherwise reword the comment and header to say the INPUT row may hold events applied at the refused boundary.

### IN-02: ivs_bound_spans_meet does not detect a span that wraps the top of the address space

**File:** `core/src/machine/machine_bound.c:147-158`
**Issue:** A buffer near `UINTPTR_MAX` whose capacity reaches past the top of the address space is not reported as overlapping an instance or report at a low address. The arithmetic does not wrap, as the comment says, but the semantic overlap of a wrapping span is missed. Phase 4 writes no buffer byte, so nothing is corrupted today. Phase 6 output writing will depend on this test.
**Fix:** Refuse `capacity > UINTPTR_MAX - (uintptr_t)ptr` as RUN_BUFFER_OVERLAP, or as a new range reason, before the pairwise tests. Apply the same rule to the event span.

### IN-03: ivs_machine_info lacks the IVS_STRUCT_SIZE_MAX cap that ivs_run.h states for out structs

**File:** `core/src/machine/machine.c:252`, `core/include/ivs/ivs_run.h:11-13`
**Issue:** `ivs_run.h` says the ivs_machine.h struct rule refuses an out struct_size above `IVS_STRUCT_SIZE_MAX`. `cpu_state` and the run report enforce that cap. `ivs_machine_info` accepts any size of 216 or more. The public contract is therefore inconsistent across calls.
**Fix:** Add `|| out->struct_size > IVS_STRUCT_SIZE_MAX` to the info refusal, with an api-effects row, or narrow the ivs_run.h wording to the calls that enforce it.

### IN-04: A clock failure at the start of the steps makes the next working reading a wall timeout

**File:** `runner/exec.c:470`, `runner/exec.c:121-124`, `runner/hostio.c:61-66`
**Issue:** `ivs_hostio_now_ms` returns 0 on failure "so a broken source never ends a run by itself". If the first reading (`wall_start`) is that 0 and a later reading succeeds, `t - 0 >= cap` fires immediately, because monotonic time is large.
**Fix:** Treat a `wall_start` of 0 as no clock, for example with a `have_clock` flag, or retry the start reading.

### IN-05: Content paths are confined by syntax only; symlinks under --content-dir are followed

**File:** `runner/hostio.c:91`, `runner/scenario.c:162-191`
**Issue:** D-22 says paths are "resolved under --content-dir". The `..` and absolute-path rules hold, but `open()` follows a symlink inside the content dir to any regular file. A scenario then reads bytes outside the directory, bounded by the cap and limited to regular files.
**Fix:** Document the scope (the operator owns the content dir), or open with `O_NOFOLLOW` on the last component and reject symlinked directories.

### IN-06: runner-scan cannot see POSIX names reached through allowed ISO headers or a C_EXTENSIONS switch

**File:** `tools/check/steps/runner-scan.sh:13-24`
**Issue:** The step targets the "passes on Apple clang, fails on hosted gcc" class, but it checks only include operands and feature macros. Two routes stay open:

- A runner TU may call `fileno`, `fdopen` or `popen` (stdio.h) or `setenv` (stdlib.h). Apple's headers declare these under `-std=c17`; glibc hides them.
- A runner CMake file may set `C_EXTENSIONS ON` or `CMAKE_C_EXTENSIONS ON`, which exposes every POSIX name.

Both routes still fail only on the hosted gcc run.
**Fix:** Add a small denylist of POSIX-only identifiers for runner files other than `hostio.c`, and flag `C_EXTENSIONS` or `CMAKE_C_EXTENSIONS` in runner CMake files.

### IN-07: The faults-verdict awk rewrite relies on index(v, "") semantics that differ between awks

**File:** `tools/check/steps/faults-verdict.sh` (judge(), `index(v, p[j]) == 1`)
**Issue:** The comment says an empty prefix field "matches every vector", as the shell `case "$p"*` did. That holds on BWK awk (macOS prints `1` for `index("abc","")`). gawk and mawk return 0. No current `.ids` file has an empty field, so the gate is unaffected. A malformed file would be judged differently on the two platforms (fail-closed on Linux).
**Fix:** Handle the empty field explicitly (`if (p[j] == "" || index(v, p[j]) == 1)`), or refuse empty fields as fault-unknown-vector.

### IN-08: pack.c --c-include does not check write errors before fclose

**File:** `tools/pack/pack.c:428-441`
**Issue:** The `fprintf` and `fputc` results are ignored. Only `fclose` is checked, and it does not report an error from an earlier buffered flush. A short write, for example on a full disk, can leave a truncated include that is not removed. It would most likely fail to compile, but not with the documented "write error" message.
**Fix:** Check `ferror(o)` before `fclose`, and remove the output when it is set.

### IN-09: ARCH_INDEPENDENT on a package that ships an architecture-specific static archive

**File:** `core/CMakeLists.txt:99-100`
**Issue:** `write_basic_package_version_file(... ARCH_INDEPENDENT)` skips the `CMAKE_SIZEOF_VOID_P` check. A 32-bit consumer would accept a 64-bit `libivs_core.a` and fail late at link time. D-33 chose this explicitly, so it is recorded only.
**Fix:** Drop `ARCH_INDEPENDENT` when convenient; the pkg-install selftests do not depend on it.

### IN-10: The golden step's .expect path and dev-versus-release coverage lean entirely on runner-process

**File:** `tools/check/steps/golden.sh:183-195`, `tools/check/steps/golden.sh:204-233`
**Issue:** An `.expect` golden is satisfied by its assert lines equalling the scenario's own assert lines (a tautology) plus the existence of a dev report. The report's outcome and assert results are not read. The dev-versus-release comparison iterates over the union of reports present, not over the deterministic inventory, so an id with no report in either preset is silently absent. Both gaps are covered today by runner-process's per-scenario exit-code and report judge, so this is a single point of evidence, not a hole.
**Fix:** In golden.sh, require `outcome pass` and every `step.*.assert.*` value to start with `pass` in the dev report of an `.expect` golden. Iterate the deterministic ids of `runner-scenarios.ids` whose class is `run` and which produce a report, instead of the union of files present.

### IN-11: Impossible-state guards in the info and capacity paths

**Category:** necessity
**File:** `core/src/machine/machine.c:276-278`, `core/src/machine/machine_bound.c:111-113`
**Issue:** `ivs_machine_info` re-checks `p == NULL` after `ivs_machine_from_handle`. `from_handle` already refuses an instance whose profile index is outside the registry (machine.h:221-224, review WR-03), and the code comments it as unreachable. `ivs_bound_fits` guards `frames == UINT64_MAX`, which cannot occur because the target is at most 2^63 (stage 6) and is divided by a frame of at least 1 tick; the code comments that as unreachable too. Each guard is a branch no test can reach.
**Fix:** A scope decision for the owner. Keep them as documented belts, or replace them with a `_Static_assert` or comment-only invariant.

---

## Hosted toolchain run

`sh tools/gate-linux.sh` (ubuntu:24.04, linux/amd64 emulated, gcc 13.3.0, dash, cmake 3.28.3) on the working tree: `gate-linux: exit 0`, `gate-time total=325` (emulated), `gate: PASS steps=33/33 controls=276/276 ... faults=149/149 ... goldens=4 reports=10 host-dependent=2 ... pkg-selftest=10/10 ... tests-expected=2780 tests-executed=2778 tests-skipped=2 tests-missing=0`. Log: build/gate-linux.log. Separate gcc:13 container probe for WR-04: scratchpad `sig/probe.c` (second `raise(SIGINT)` exits 130 on glibc, survives on macOS).

_Reviewed: 2026-11-11_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
