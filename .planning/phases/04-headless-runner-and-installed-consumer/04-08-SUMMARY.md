---
phase: 04-headless-runner-and-installed-consumer
plan: 08
subsystem: gate, api
tags: [api, gate, pins, controls, D-04, D-05, D-16, API-02, RUN-04]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-07: IVS_STOP_* 1..4 in ivs/ivs_run.h, IVS_REASON_RUN_* 34..40 in ivs/ivs_machine.h; steps=29/29, controls=249/249, faults=130/130, tests-expected=2310, api-calls=14, api-rows=37"
provides:
  - "Generic define pin, section 6 of tools/check/api-effects.cmake: class table DEFINE_CLASSES (PREFIX|PIN FILE|VALUE FORM|EXCLUDED NAMES); value form `uint`; object-like defines of core/include/ivs/*.h outside block and line comments vs `NAME VALUE` pin lines"
  - "Tokens api-define-unpinned (also a missing pin file and a class with no define), api-define-renumbered, api-define-orphan, api-define-duplicate (header side and pin side); stat pinned-defines (45)"
  - "data/expected/api-stop-reasons.ids (IVS_STOP_TARGET 1 .. IVS_STOP_INTERNAL 4) and data/expected/api-reasons.ids (IVS_REASON_NONE 0 .. IVS_REASON_RUN_TARGET_RANGE 40, 41 lines)"
  - "Good tree: toy core/include/ivs/ivs_run.h and toy pins; controls api-stop-unpinned, api-stop-renumbered, api-reason-unpinned"
affects: [04-13]

actuals:
  tokens: 5100
  tasks: 2
  commits: 2
plan_head_before: 0347c88ee24d80c237457410dcdf7b4b69305cde
plan_head_after: 92faf5f122afdb4e8fe195150c38104ac69a5efe

tech-stack:
  added: []
  patterns:
    - "A pinned header-number class is one row of DEFINE_CLASSES; plan 04-13 adds IVS_CTRL_ with a new value form (`(1u << n)` read as the bit index) and its exclusion list (IVS_CTRL_RESERVED_MASK)"
    - "Comment state is tracked only on lines that can change it or hold a define (file(STRINGS REGEX)), so the scan stays cheap"

key-files:
  created:
    - data/expected/api-stop-reasons.ids
    - data/expected/api-reasons.ids
    - tools/check/controls/_good/tree/core/include/ivs/ivs_run.h
    - tools/check/controls/_good/tree/data/expected/api-stop-reasons.ids
    - tools/check/controls/_good/tree/data/expected/api-reasons.ids
    - tools/check/controls/api-stop-unpinned/
    - tools/check/controls/api-stop-renumbered/
    - tools/check/controls/api-reason-unpinned/
  modified:
    - tools/check/api-effects.cmake
    - tools/check/steps/api-effects.sh
    - data/expected/controls/api-effects.ids

key-decisions:
  - "A class with no define in the public headers fails as api-define-unpinned (a vacuous pin, T-04-25), like a missing pin file"
  - "An unparsable class define (e.g. a shift in a `uint` class), a pin line outside its class, and a name defined or pinned twice are api-effects-format (fail closed)"
  - "Function-like macros with a class prefix are ignored (the toy IVS_STOP_IS_TARGET(r) proves it on the positive control)"

patterns-established:
  - "Pins and their good-tree toy inputs land in one commit when the check fails closed"

requirements-completed: []

duration: about 20 min
completed: 2026-11-11
---

# Phase 4 Plan 08: Generic define pins for stop and refusal reasons Summary

**The `api-effects` step now pins every `IVS_STOP_*` and `IVS_REASON_*` number of the public headers against append-only inventories (`pinned-defines=45`), through a class table that plan 04-13 extends for input bits.**

## Performance

- **Duration:** about 20 min
- **Tasks:** 2 of 2 (tracer first; its verify re-run green before Task 2)
- **Files:** 20 changed (323 insertions, 7 deletions)
- **Cost:** `step-time api-effects 0` (under 1 s, unchanged); `step-time controls 20`, `gate-time total` 39-43 s on this host with load average 12-13 (04-07: 36-38 s at load 13-15; the three new controls are in the control lanes).

## Counts

Start: `steps=29/29 controls=249/249 faults=130/130 tests-expected=2310 api-calls=14`.
After Task 1: `controls=249/249 pinned-defines=45`.
End: `steps=29/29 controls=252/252 api-calls=14 api-statuses=14 api-rows=37 pinned-defines=45 fault-runs=130 faults=130/130 tests-expected=2310 tests-executed=2310 tests-skipped=0 tests-missing=0`.

## Accomplishments

- **Checker (Task 1):** section 6 of `api-effects.cmake` reads every `core/include/ivs/*.h`, strips block and line comments with a state carried across lines, collects `#define NAME REST`, and for each class row compares object-like defines (value: decimal without a leading zero or `0x` hex, optional `u`/`U`, optional parentheses) with the pin file. Four new tokens share the existing per-token cap of 20 and the exact-total line. Header comments of both the checker and the step script list the rules and the new stat.
- **Pins:** `api-stop-reasons.ids` (4 lines) and `api-reasons.ids` (41 lines: 0..33 load reasons and 34..40 run reasons), each with a header naming plan 04-08, D-04/D-05 and "append-only: never renumber, never reuse". Acceptance: `grep -c '^IVS_STOP_TARGET 1$'` = 1; `IVS_REASON_` pin lines 41 = header defines 41.
- **Good tree (same commit as the fail-closed check):** toy `ivs_run.h` with `IVS_STOP_TARGET 1u`, `IVS_STOP_CPU_HALTED (2u)`, `IVS_REASON_NONE 0u`, `IVS_REASON_TRUNCATED 1u` with a trailing comment, a commented-out `#define IVS_STOP_COMMENTED 9u` and a function-like `IVS_STOP_IS_TARGET(r)`; neither of the last two may be reported, so the positive control covers the comment and function-like rules. No `IVS_API` line. Every existing `api-*` control still failed only for its own token (no overlay change).
- **Hand checks before the controls** (scratch copies of the good tree): unpinned, renumbered, orphan, duplicate (both sides), missing pin file, unparsable `(1u << 3)`, a define hidden inside a multi-line comment (ignored) and a hex define after `*/` on the same line (read as 16): each produced only its intended token.
- **Controls (Task 2):** `api-stop-unpinned` (toy header gains `IVS_STOP_GUEST_FAULT 3u`), `api-stop-renumbered` (toy pin `IVS_STOP_CPU_HALTED 5`), `api-reason-unpinned` (toy header gains `IVS_REASON_MAGIC 3u`); each FORBIDs the other three define tokens and `api-effects-format`. Listed under `# Define pins (plan 04-08, D-04).` in `data/expected/controls/api-effects.ids`.

## Task Commits

1. **Task 1 (tracer): generic define pin, pins, toy good-tree inputs** - `bb60080` feat(04-08)
2. **Task 2: three define pin controls** - `92faf5f` test(04-08)

## Final gate summary lines

- `sh tools/gate.sh` (macOS, Apple clang 21): `gate: PASS steps=29/29 controls=252/252 api-calls=14 api-statuses=14 api-rows=37 pinned-defines=45 cc=AppleClang c-presets=2 fault-runs=130 ... faults=130/130 ... tests-expected=2310 tests-executed=2310 tests-skipped=0 tests-missing=0 ...`
- `dash tools/gate.sh`: the same line, `gate: PASS`.
- `sh tools/gate-linux.sh`: not run (the plan does not require it; the change is cmake -P script code within 3.25 and control data only).

## Deviations from Plan

**1. [Rule 2 - Fail closed] Empty class and format errors**
- A class with zero defines in the headers is `api-define-unpinned` (T-04-25 vacuous pin); an unparsable class define, a pin line outside its class and a name defined or pinned twice are `api-effects-format`. The plan named only the missing pin file as fail-closed; these close the same hole. Commit `bb60080`.

**2. Toy header content**
- The toy header carries, beyond the two stop and two reason defines the plan asks for, one commented-out define and one function-like macro so the ignore rules are exercised by the positive control. Parenthesized and trailing-comment forms are used once each for the same reason.

## Open Items

- API-02 stays Pending: the reason and stop-reason pins are now gate-enforced; the slicing-invariance test (D-07, plan 04-09) remains. RUN-04 stays Pending (important-gates inventory and step, later plans).
- Plan 04-13: add the `IVS_CTRL_` row (value form `(1u << n)` read as bit index n, exclusion `IVS_CTRL_RESERVED_MASK`); the current code rejects any form other than `uint` with `api-effects-format`, so a new form needs its branch in section 6. The toy good tree needs two `IVS_CTRL_*` defines and a toy `input-bits.ids` in the same commit.
- Any new `IVS_STOP_*` or `IVS_REASON_*` (plans 04-10, 04-11 add input/buffer reasons) must append a line to the matching pin file in the same commit, or the gate fails `api-define-unpinned`.
- Carried from 04-07: `ivs_machine_run` not yet exercised from the installed package (04-13 pins struct sizes); RUN-05 Pending (04-10, 04-11); RUN-01 Pending (04-17..04-19); 04-14 adds `tools/pack/` to the c-build listed-source loop; the orchestrator records hosted runs with `sh tools/ci-record.sh <run-id>` and applies D-45 after 04-10 and 04-19; PKG-03/PKG-04 Pending; job summary lists `pending-resolvers=17` and `unknown-fields=16`; `sh tools/hygiene.sh --history` still fails on earlier planning commits (this plan's commits add no home paths).

## Threat Flags

None beyond the plan's threat model: T-04-24 mitigated (four tokens, three controls); T-04-25 mitigated (missing pin file and empty class fail closed; `pinned-defines=45` nonzero in the gate line); T-04-SC (no package installs).

## Self-Check: PASSED

- Files present: data/expected/api-stop-reasons.ids, data/expected/api-reasons.ids, the toy header and toy pins, the three control dirs.
- Commits reachable from HEAD: bb60080, 92faf5f (`git rev-list --count 0347c88..HEAD` = 2).
