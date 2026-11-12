---
phase: 04-headless-runner-and-installed-consumer
plan: 23
subsystem: tests/installed-consumer, tools/check, data/expected, docs
tags: [packaging, installed-consumer, find_package, relocation, independence, ninja-deps, golden, selftest, PKG-01, SC4, D-24, D-34, D-36, D-37, D-38, D-39, D-40]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-22: installable static ivs::core, gate step pkg-install (install, move into a path with a space, delete the original, inventory/content/path/symbol/header-alone checks, 8 selftests); 04-20/04-21: data/scenarios/diag-input-pass.ivss and data/golden/diag-input-pass.digests; 04-14/04-16: build/release/images/{boot-stub.exec,diag-cpu-1.ivsc}"
provides:
  - "tests/installed-consumer/{CMakeLists.txt,main.c,main.cpp}: an out-of-tree consumer project (find_package(ivs 0.1 CONFIG REQUIRED), strict C17, NO_SYSTEM_FROM_IMPORTED, registries off) that replays diag-input-pass through the public API, and a tiny C++17 program for the direct link"
  - "pkg-install part 2: consumer copy, configure, build and run on the release-tree images; output pin; ninja -t deps leak and presence check; link-line, compile-flag and ivs_DIR checks; runner-golden digest agreement; nm -u call pin; direct C++ link; selftests leak-detector and golden-drift (pkg-selftest=10/10)"
  - "data/expected/pkg-consumer.golden (regression pin, emulator is oracle), data/expected/pkg-consumer-calls.ids (13 names), data/expected/pkg-selftests.ids (10 cases)"
  - "docs/packaging.md: install, layout, find_package, version rule, what the gate step checks and what it does not prove"
affects: [04-24, 04-25]

actuals:
  tokens: 15000
  tasks: 3
  commits: 3
plan_head_before: b478ed3192929e8ef6d6ac6b22316313b8aedcb1
plan_head_after: 120d50a0158a310ca4ec68390ce709f0d38bb9e0

tech-stack:
  added: []
  patterns:
    - "Independence is read from the consumer build's own records: `ninja -t deps` with a fixed 4-space indent strip (no field split; paths hold spaces), `ninja -t commands` for the link and compile lines, the CMakeCache ivs_DIR in pwd -P form"
    - "Every check is a function that prints its own FAIL lines and returns a status, so the main path and the selftests call the same code (check_golden, check_deps, check_link, check_ivs_dir, check_runner_golden, check_calls)"
    - "The consumer prints only host-independent values (no path, time or address), so one pin holds on macOS and Linux"

key-files:
  created:
    - tests/installed-consumer/CMakeLists.txt
    - tests/installed-consumer/main.c
    - tests/installed-consumer/main.cpp
    - data/expected/pkg-consumer.golden
    - data/expected/pkg-consumer-calls.ids
    - docs/packaging.md
  modified:
    - tools/check/steps/pkg-install.sh
    - data/expected/pkg-selftests.ids

key-decisions:
  - "The consumer replays diag-input-pass with the refusals (truncated EXEC MALFORMED reason 21, duplicate cartridge DUPLICATE_LOAD reason 33) placed before and after the loads; the second fresh instance makes no refusal, so its equal digests also show that the refusals left no trace"
  - "The version tie passes ivs_VERSION_MAJOR/MINOR/PATCH as three integer compile definitions (no quoted string through CMake) and compares them with the IVS_API_VERSION_* macros and ivs_api_version()"
  - "The v1 sizes are printed and checked from the installed headers: run_request 64 with events at 48 on LP64 (visible skip-non-lp64 elsewhere), run_report 112, input_event 32, cpu_state_report 36, ivs_info 256 with the v1 end at 216"
  - "digest.video and digest.audio are printed only when the profile's output format is not NONE; the step also requires the consumer's domains to equal the golden header's observed domains (composite apart)"
  - "Additional independence checks beyond the plan: no ivs*.h dependency outside the moved prefix, no dependency under the temp dir other than the source copy, the strict flags present and no -isystem on the compile line (pkg-consumer-flags)"
  - "PKG-01 is marked Complete: the gate shows a separate project, built only against the installed relocated package, loading, refusing, running with input and digesting, in agreement with the runner golden, under sh, dash and the Ubuntu container"

duration: about 14 min of edits plus gates (first task commit 20:25, last 20:35), plus one container gate
completed: 2026-11-11
---

# Phase 4 Plan 23: the independent installed consumer Summary

**A separate CMake project in `tests/installed-consumer/` now builds only against the installed, moved package and replays the `diag-input-pass` scenario through the public API: version tie, v1 sizes, two refusals with unchanged digests, loads, reset, two bounded run calls with the two input events, and per-domain digests. A second fresh instance gives the same digests. The gate step `pkg-install` copies the project outside the repository, configures it with the moved prefix as the only package source, runs it on the release-tree images, and compares its output with a pin and its digest lines with the runner golden of the same scenario. The consumer build's own records (`ninja -t deps`, the link line, `ivs_DIR`) show that nothing came from the repository. A direct C++17 link and two more selftests complete D-38 and D-40 (`pkg-selftest=10/10`).**

## What was built

- Task 1 (tracer, `5c7a889`): consumer project and `main.c`; step part 2 (copy to `$T/consumer src`, configure in a subshell with the package and path variables unset, build, run, keep `build/gate/pkg-consumer.txt`, compare with `data/expected/pkg-consumer.golden` without its `#` lines; `consumer-lines=26`).
- Task 2 (`41e1322`): `check_deps`, `check_link`, `check_ivs_dir`, `check_runner_golden`, `check_calls`; `main.cpp` and the direct C++ link (`cxx-link=ok`); `data/expected/pkg-consumer-calls.ids`.
- Task 3 (`120d50a`): selftests `leak-detector` and `golden-drift`, the 10-case inventory, the step header, `docs/packaging.md`.

The consumer's digest lines equal `data/golden/diag-input-pass.digests` on the first build: the public package alone reproduces what the runner computes through the build tree (cpu `8c6a8aa3...`, mem `1d935916...`, images `69d3b715...`, instance `3a953773...`, composite `c834c07a...`, input `b18223e8...`; halt at tick 2084, step 1 ends at 1020 with overshoot 20, both events consumed).

| Selftest | Mutation (on a copy) | Must show |
|---|---|---|
| leak-detector | consumer copy with `leak.c` including `common/ivs_sha256.h` and `-I <repo>/core/src` | the copy builds; `pkg-leak: <repo>/core/src/common/ivs_sha256.h` |
| golden-drift | consumer output with the last nibble of the first digest line changed | the unchanged output matches the pin; the changed one gives `pkg-golden-diff` |

Meta-check (scratch copies of the step, not committed): with the repo-path and ivs-header rules of `check_deps` removed, `pkg-selftest-leak-detector: dependency check passed`; with the pin comparison disabled, `pkg-selftest-golden-drift: golden check passed`. With an extra name in the call pin and one changed digest in the runner golden (restored afterwards from scratch copies), the main path gives `pkg-consumer-calls: not referenced: ivs_machine_unload` and `pkg-runner-golden-diff: golden only: digest.input consumer only: digest.input`.

## Performance

- Local (macOS arm64, loaded laptop): `step-time pkg-install 2` in every gate of this plan (1 before it). A standalone run takes 1.85 s wall on an otherwise idle gate and 1.95-2.06 s while the container gate ran beside it (0.95-1.01 s before this plan): the consumer configure (about 0.25 s), build and run, the leak-detector configure and build, the C++ link and the deps/link/nm reads add about 0.9 s.
- Container (`tools/gate-linux.sh`, amd64 emulated on arm64): `step-time pkg-install 7` (4 before this plan). Hosted native x86_64 should be close to the local figure; record the hosted value after the first hosted run (PKG-04 / CI-EFFICIENCY).
- Gate total 27-28 s locally (25-27 s before).

## Counts

Start: `steps=32/32 controls=273/273 faults=149/149 tests-expected=2776 tests-skipped=2`, `pkg-selftest=8/8`.
End: `steps=32/32 controls=273/273 faults=149/149 tests-expected=2776 tests-executed=2774 tests-skipped=2 tests-missing=0`. Step stats: `prefix-files=10 archive=members public-symbols=14 defined=55 hdr-alone=8/8 consumer-lines=26 cxx-link=ok pkg-selftest=10/10`.

Acceptance checks: `grep -c 'find_package(ivs 0.1 CONFIG REQUIRED)' tests/installed-consumer/CMakeLists.txt` = 1; `grep -c 'second-instance equal=yes' data/expected/pkg-consumer.golden` = 1; `grep -c '^ivs_machine_run$' data/expected/pkg-consumer-calls.ids` = 1; `grep -v '^#' data/expected/pkg-selftests.ids | grep -c .` = 10. The three task verify commands exit 0.

## Deviations from Plan

**1. [Rule 2] Extra independence checks.** Beyond the plan's list, `check_deps` also flags an `ivs*.h` dependency outside the moved prefix and any dependency under the temp dir other than the consumer copy (for example the deleted `prefix-a`). `check_link` also requires the strict flags and no `-isystem` on the main.c.o compile line (`pkg-consumer-flags`), so a lost `NO_SYSTEM_FROM_IMPORTED` would show. The step refuses to run when the temp dir lies inside the repository (`pkg-consumer-tmp`).

**2. [Rule 2] Runner-golden domain set.** Besides equal digest lines, the consumer's domains must equal the golden header's observed domains (the composite apart), so a domain the golden gains later cannot be skipped silently.

**3. The consumer also prints and pins** the load reports (lengths and SHA-256, equal to the golden's `slot.*.sha256`), the `ivs_info` timebase, the CPU state and the words at 0200..020F after each run call. This replays the scenario's assertions in the pin. The scenario's targets are already master ticks, so the multiplier from `ivs_info` is 1 (printed `ticks_per_cpu_cycle=4`, `nominal_frame_ticks=59736`).

**4. `docs/packaging.md` CMake floor wording.** The doc says 3.25 is the declared minimum of the project, its presets and the consumer. The package config file itself carries no `cmake_minimum_required`, which is normal for a config file, so the doc does not claim one.

## Open Items

- Plan 04-24 (important-gates) may cite the ten pkg selftests (T-04-66..71) and register the seven runner faults of 04-18/04-19.
- The hosted `step-time pkg-install` goes to CI-EFFICIENCY.md after the first hosted run (PKG-04).
- `docs/packaging.md` states the limits: same toolchain, machine and author; agreement of two paths through one core, not correctness; static only; 3.25 floor untested; LP64 only for the request size. A consumer check against a published release archive belongs to release packaging.
- Carried: C14 and C63-C68 revisited in Phase 5/6/7; `API-02`, `PKG-03`, `PKG-04`, RUN-04 Pending; `pending-resolvers=17`, `unknown-fields=16`. `sh tools/hygiene.sh --history` still fails on earlier planning commits (this plan adds no home paths). The orchestrator records hosted runs with `sh tools/ci-record.sh <run-id>`.

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=32/32 controls=273/273 ... cc=AppleClang ... faults=149/149 ... prefix-files=10 archive=members public-symbols=14 defined=55 hdr-alone=8/8 consumer-lines=26 cxx-link=ok pkg-selftest=10/10 ... tests-expected=2776 tests-executed=2774 tests-skipped=2 tests-missing=0 ...`, `gate-time total=27`.
- `dash tools/gate.sh`: the same line, `gate: PASS`, `gate-time total=27`.
- `sh tools/gate-linux.sh` (Ubuntu 24.04 container, x86_64, gcc 13.3.0, cmake 3.28.3): `gate: PASS steps=32/32 controls=273/273 ... cc=GNU c-presets=2 fault-runs=149 core-imports=3 ... faults=149/149 ... prefix-files=10 archive=sha256 public-symbols=14 defined=55 hdr-alone=8/8 consumer-lines=26 cxx-link=ok pkg-selftest=10/10 ... tests-expected=2776 tests-executed=2774 tests-skipped=2 tests-missing=0`, `gate-time total=114`, last line `gate-linux: exit 0 (log build/gate-linux.log)`. The same 26-line consumer output matched the pin and the runner golden under gcc and GNU nm, so the pin is host independent.

## Threat Flags

None. The step writes only under its mktemp dir and `build/gate/pkg-consumer.txt`, and opens no network path. The consumer reads two files named on its command line.

Mitigations from the plan's threat model:
- T-04-70: copy outside the repo, registries off, path variables unset, `ninja -t deps` positive presence, link line, `ivs_DIR`, `leak-detector` selftest.
- T-04-71: pinned output, runner-golden agreement, `golden-drift` selftest.
- T-04-72: images only as argv from `build/release/images`; the zero GROM is made in memory; nothing is embedded or installed.
- T-04-SC: no package installs.

## Self-Check: PASSED

- Files exist: tests/installed-consumer/CMakeLists.txt, tests/installed-consumer/main.c, tests/installed-consumer/main.cpp, data/expected/pkg-consumer.golden, data/expected/pkg-consumer-calls.ids, data/expected/pkg-selftests.ids, docs/packaging.md, tools/check/steps/pkg-install.sh.
- Commits on HEAD: 5c7a889, 41e1322, 120d50a.
