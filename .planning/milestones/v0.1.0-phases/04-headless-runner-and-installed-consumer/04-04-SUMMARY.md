---
phase: 04-headless-runner-and-installed-consumer
plan: 04
subsystem: gate, runner
tags: [gate, runner, hygiene, integrity, D-23, D-24, A-07, RUN-01, RUN-04]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-03: runner/ with ivs_runner_lib, build/dev/ivs-runner-sources.txt, _good/tree runner/good.c; steps=28/28, controls=243/243, faults=123/123"
provides:
  - "step runner-scan: runner include, core-internal and feature-macro confinement (POSIX only in runner/hostio.c)"
  - "integrity.cmake: ASCII rule over runner/, SPDX rule over runner/ sources/scripts/CMake files and anchored tools/(pack|cart)/*.c|h"
  - "c-build: c-build-unlisted-source for a tracked runner/*.c missing from build/dev/ivs-runner-sources.txt"
  - "six controls (data/expected/controls/runner-hygiene.ids) and a toy runner/hostio.c in the good tree"
affects: [04-14, 04-17, 04-18, 04-19]

actuals:
  tokens: 5300
  tasks: 2
  commits: 2
plan_head_before: df5ed2170e89469dbb8fad9e8d8255f03147681a
plan_head_after: a2e5f98

tech-stack:
  added: []
  patterns:
    - "A pure step reads source as the compiler does: join backslash continuations, strip comments while keeping literals, recognise #, %: and ??= directives, fail closed on any include form it cannot resolve"
    - "c-build unlisted-source guard is a loop of '<tracked regex> <build/dev list>' items; a new C module adds one item"

key-files:
  created:
    - tools/check/steps/runner-scan.sh
    - data/expected/gate-steps/runner.ids
    - data/expected/controls/runner-hygiene.ids
    - tools/check/controls/_good/tree/runner/hostio.c
    - tools/check/controls/runner-scan-header/
    - tools/check/controls/runner-scan-internal-include/
    - tools/check/controls/runner-scan-posix-define/
    - tools/check/controls/integrity-missing-spdx-runner/
    - tools/check/controls/integrity-non-ascii-runner/
    - tools/check/controls/integrity-missing-spdx-tools-c/
  modified:
    - tools/check/integrity.cmake
    - tools/check/steps/c-build.sh

key-decisions:
  - "runner-scan's internal-include rule also applies to runner/hostio.c (only the ISO-header list and the feature-macro rule exempt it): no runner file may reach core internals"
  - "runner-scan also fails a POSIX feature-macro name on a non-comment line of a runner CMake file (a compile definition would bypass the hostio.c confinement); proven by the runner-scan-posix-define control, no extra control"

patterns-established:
  - "New top-level C directories join both integrity rules and the c-build listed-source loop in the plan that creates them"

requirements-completed: []

duration: 30min
completed: 2026-11-11
---

# Phase 4 Plan 04: Runner source hygiene (runner-scan, integrity coverage, c-build guard) Summary

**A new step `runner-scan` makes the hosted-only POSIX-name class a local failure by keeping POSIX headers and feature macros in `runner/hostio.c` alone and core internals out of every runner file; `integrity.cmake` now applies the ASCII and SPDX rules to `runner/` and the anchored `tools/(pack|cart)/` C directories, and `c-build` fails an unbuilt runner TU; six controls, gate green under sh, dash and the Ubuntu container.**

## Performance

- **Duration:** about 30 min
- **Tasks:** 2 of 2
- **Files:** 28 changed (290 insertions, 13 deletions)
- **Cost of the new checks (W7, budget note):** `runner-scan` on the real tree 0.04 s (`step-time runner-scan 0` in every gate run); the integrity pattern change adds two regex tests per path (`step-time integrity 2`, unchanged); the c-build runner loop is one grep, sort and comm (`step-time c-build` unchanged). Each new control is a good-tree copy plus one step run, about 0.03-0.05 s wall measured in isolation (integrity on the 61-file control tree 0.03 s, runner-scan 0.05 s, tree copy 0.03 s), so the seven new control runs (six negative plus `_good/runner-scan`) add roughly 0.3 s of lane time. The gate-time rise in the Task 2 runs (35 s -> 59 s sh, 45 s -> 74 s dash) was load, not the plan: the load average was 34-39 at the time.

## Counts

Start: `steps=28/28 controls=243/243 faults=123/123 tests-expected=2186`.
After Task 1: `steps=29/29 controls=243/243 faults=123/123 tests-missing=0`.
End: `steps=29/29 controls=249/249 faults=123/123 fault-runs=123 tests-expected=2186 tests-executed=2186 tests-skipped=0 tests-missing=0`, matching the plan's stated end counts.

## Accomplishments

- `tools/check/steps/runner-scan.sh` (pure; header lists tokens and stats): over every FILELIST `runner/**/*.c|h`, after joining continuations and stripping comments (literals kept), recognising `#`, `%:` and `??=` directives:
  - `runner-scan-header: <file>:<line> <operand>` for an `#include`/`#import` outside the ISO list (`assert.h errno.h inttypes.h limits.h signal.h stdbool.h stddef.h stdint.h stdio.h stdlib.h string.h`), `ivs/<name>.h` and runner-local headers (relative to `runner/` or the including file's directory); `#include_next`, a computed `#include MACRO` and trailing text fail closed;
  - `runner-scan-internal-include` for an operand containing `src/`, `machine/`, `memmap/`, `cpu/`, `common/` or `..` (reported instead of `-header`, `runner/hostio.c` included);
  - `runner-scan-posix-define` for `#define`/`#undef` of `_POSIX_C_SOURCE`, `_POSIX_SOURCE`, `_XOPEN_SOURCE`, `_XOPEN_SOURCE_EXTENDED`, `_GNU_SOURCE`, `_DEFAULT_SOURCE`, `_BSD_SOURCE` or `_DARWIN_C_SOURCE` outside `runner/hostio.c`, and for those names on a non-comment line of `runner/**/CMakeLists.txt` or `runner/**/*.cmake`;
  - `runner-scan-empty: runner` with zero runner files; pass line `-- runner-scan pass runner-files=<n>` (2 on the real tree).
- `data/expected/gate-steps/runner.ids` (owning plan 04-04, D-23, A-07).
- Good tree `runner/hostio.c`: SPDX, `#define _POSIX_C_SOURCE 200809L` before `#include <sys/stat.h>`, one function (`_good/runner-scan` passes on it).
- `tools/check/integrity.cmake` (A-07): ASCII directory pattern gains `runner`; SPDX gains `runner/.*[.](c|h|sh|cmake)`, `runner/` in the `(.*/)?CMakeLists[.]txt` group and `tools/(pack|cart)/.*[.](c|h)`, with a comment that names the two SPDX-less denylist control inputs the anchor keeps outside. Real tree: `-- integrity pass files=1306`, no new finding (43 tracked `tools/**/*.c|h`, all under `tools/check/controls/`).
- `tools/check/steps/c-build.sh`: the unlisted-source guard loops over `'^core/.*[.]c$ ivs-core-sources.txt'` and `'^runner/.*[.]c$ ivs-runner-sources.txt'`; token `c-build-unlisted-source` unchanged; a missing list is `c-build-failed: dev (no <list>)`; header comment says plan 04-14 adds `tools/pack/`.
- Controls (`data/expected/controls/runner-hygiene.ids`): `runner-scan-header` (`runner/good.c` includes `<unistd.h>`), `runner-scan-internal-include` (`#include "machine/machine.h"`), `runner-scan-posix-define` (`#define _POSIX_C_SOURCE 200809L` in `runner/good.c` plus an overlay `runner/CMakeLists.txt` with `target_compile_definitions(... _POSIX_C_SOURCE=200809L)`; EXPECT names both lines), each with FORBID of the other two runner-scan tokens; `integrity-missing-spdx-runner` (MAKE `write runner/x.c x`), `integrity-non-ascii-runner` (MAKE `nonascii runner/good.c`), `integrity-missing-spdx-tools-c` (MAKE `write tools/pack/x.c x`). No SPDX-less C file is tracked.

## Task Commits

1. **Task 1: tracer, step runner-scan, inventory, good-tree hostio.c** - `836008d` feat(04-04)
2. **Task 2: integrity coverage, c-build runner guard, six controls** - `a2e5f98` feat(04-04)

## Task 1 proofs

- Tracer gate: `-- runner-scan pass runner-files=2`, `gate: PASS steps=29/29 controls=243/243 ... faults=123/123 ... tests-missing=0` (sh and dash).
- Hand failure in a scratch tree (a runner TU including `<unistd.h>` among 24 adversarial lines): `-- FAIL runner-scan-header: runner/a.c:3 unistd.h`. The same scratch run also showed: `%:include <time.h>` and `??=include <fcntl.h>` -> `-header`; `#include FOO` and `# include_next <stdio.h>` -> `-header` (fail closed); a `#inc\` + `lude <pthread.h>` continuation -> `-header` at its first line; `"machine/machine.h"`, `<../core/src/x.h>` and `"common/x.h"` in `runner/hostio.c` -> `-internal-include`; `#define \` + `_XOPEN_SOURCE 700` -> `-posix-define`; `add_definitions(-D_GNU_SOURCE)` in `runner/CMakeLists.txt` -> `-posix-define`; includes inside a block comment, a string holding `/*`, `_POSIX_C_SOURCE_X` and the hostio.c `#define _POSIX_C_SOURCE` / `<unistd.h>` -> no finding. Identical output under sh and dash.

## Task 2 probes

- c-build probe (untracked `runner/zz_probe.c`, SPDX and one function, deleted afterwards): `sh tools/gate.sh` reported
  ```
  -- FAIL c-build-unlisted-source: runner/zz_probe.c
  -- FAIL gate-step-failed: c-build
  ```
  and `gate: FAIL steps=28/29 controls=249/249 ...`; no other step failed.
- Regression probe: on a good-tree copy with `runner/x.c` and `tools/pack/x.c` (content `x`) and a non-ASCII byte appended to `runner/good.c`, the previous `integrity.cmake` (from `836008d`) printed `-- integrity pass files=61`; the new one printed exactly `integrity-non-ascii: runner/good.c`, `integrity-missing-spdx: runner/x.c` and `integrity-missing-spdx: tools/pack/x.c`. A revert of the patterns therefore cannot pass the three integrity controls.
- `sh tools/check/run-controls.sh`: `control ok _good/runner-scan`, all six new controls ok, `controls=249/249`.

## Final gate summary lines

- `sh tools/gate.sh` (macOS, Apple clang 21): `gate: PASS steps=29/29 controls=249/249 ... cc=AppleClang c-presets=2 fault-runs=123 ... faults=123/123 ... runner-files=2 ... tests-expected=2186 tests-executed=2186 tests-skipped=0 tests-missing=0`; `gate-cpu user=61.99 sys=79.89`, `gate-time total=59` (load average 34-39).
- `dash tools/gate.sh`: the same tokens, `gate: PASS`; `gate-cpu user=72.07 sys=102.35`, `gate-time total=74`.
- `sh tools/gate-linux.sh` (ubuntu:24.04 amd64 emulated on arm64, `gate-env ncpu=8 cc=GNU-13.3.0`, cmake 3.28.3, dash as /bin/sh, awk is mawk): `gate: PASS steps=29/29 controls=249/249 cc=GNU ... fault-runs=123 ... faults=123/123 ... runner-files=2 ... tests-expected=2186 tests-executed=2186 tests-skipped=0 tests-missing=0`, `step-time runner-scan 1`, `gate-time total=238`, last line `gate-linux: exit 0 (log build/gate-linux.log)` (emulated and under load; not hosted times). The plan did not require this run; it was done because runner-scan is new awk and mawk is not installed locally.

## Deviations from Plan

### Auto-fixed / discretion

**1. [Rule 2 - confinement] Feature macros in runner CMake files**
- A `target_compile_definitions(... _POSIX_C_SOURCE ...)` in `runner/CMakeLists.txt` would make POSIX names visible in every runner TU without any `#define`, bypassing the hostio.c rule. runner-scan also fails those names on non-comment lines of runner CMake files; the `runner-scan-posix-define` control carries both cases (two EXPECT lines), so the control count stays at the planned 249.

**2. Wider feature-macro list**
- Beyond the plan's six names, `_POSIX_SOURCE` and `_XOPEN_SOURCE_EXTENDED` are also refused (both unlock POSIX names on glibc).

**3. internal-include applies to hostio.c**
- The plan exempts `runner/hostio.c` from the whole scan; here it is exempt only from the ISO-header list and the feature-macro rule, so even the POSIX file cannot include core internals (the runner library never has `core/src` on its include path anyway).

**4. Integrity SPDX pattern layout**
- `runner/(.*/)?CMakeLists[.]txt` is expressed by adding `runner` to the existing `(core|include|tests)/(.*/)?CMakeLists[.]txt` group; same set of paths.

**5. Report detail**
- `runner-scan-internal-include` also names the operand (`<file>:<line> <operand>`), like `-header`.

## Open Items

- RUN-01 and RUN-04 stay Pending: this plan adds hygiene for the runner sources only; the executable, report and demonstrating scenarios land in plans 04-17..04-19 (`important-gates` for RUN-04 later).
- Plan 04-14 adds `tools/pack/` to the c-build listed-source loop (comment left in `c-build.sh`); its C files are already under the SPDX rule.
- mawk (the hosted awk) is not installed locally; runner-scan's awk uses POSIX features only and the container run (Ubuntu 24.04 with no gawk installed, so `awk` is mawk) is the check.
- Carried from 04-02/04-03: the orchestrator records hosted runs with `sh tools/ci-record.sh <run-id>` (one row so far) and applies D-45 after 04-10 and 04-19; PKG-03/PKG-04 Pending.
- Carried from 04-01..04-03: the job summary lists `pending-resolvers=17` and `unknown-fields=16` as skips-and-missing; `sh tools/hygiene.sh --history` still fails on earlier planning commits (orchestrator decision; this plan's commits add no home paths).

## Threat Flags

None beyond the plan's threat model: T-04-12 mitigated (runner-scan, three controls, CMake definitions included); T-04-13 (integrity patterns, three controls, regression probe); T-04-14 (c-build runner loop, probe quoted); T-04-SC (no package installs).

## Self-Check: PASSED

- Files present: tools/check/steps/runner-scan.sh, data/expected/gate-steps/runner.ids, data/expected/controls/runner-hygiene.ids, tools/check/controls/_good/tree/runner/hostio.c and the six control dirs.
- Commits reachable from HEAD: 836008d, a2e5f98 (`git rev-list --count df5ed21..HEAD` = 2).
