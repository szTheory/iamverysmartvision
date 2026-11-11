---
phase: 04-headless-runner-and-installed-consumer
plan: 12
subsystem: api, run, input, profile
tags: [api, run, input, profile, D-13, D-15, D6, C14, RUN-01, RUN-05]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-11: ivs_input_event, ivs_input_check (two passes), the INPUT row and chain, reasons 44..53; steps=29/29, controls=252/252, faults=140/140, tests-expected=2426, api-calls=14"
provides:
  - "Profile field input.controllers = 2 (tag tier2-first-party-adjacent, cite SRC-bsr-2609, conflict C14, decision D6) and its line in data/expected/profile-fields.ids"
  - "cmake/IvsMemmapTable.cmake emits input_controllers (1..4, fail closed: missing, 0, 5 and a string all stop the generator) into ivs_profile_desc"
  - "ivs_input_check takes the controller count; pass 2 (UNSUPPORTED) refuses a nonzero word at an index at or above it with IVS_REASON_INPUT_EXTRA_CONTROLLER 54 (pinned) and the failing index"
  - "Cases run.input.refuse.extra-controller-word, run.input.extra-word-zero-ok, run.input.guest-invisible"
affects: [04-13, 04-16, 04-19, Phase 5]

actuals:
  tokens: 4700
  tasks: 2
  commits: 2
plan_head_before: 7c960c4
plan_head_after: 379e9af

tech-stack:
  added: []
  patterns:
    - "A per-event check that depends on the profile takes the profile value as an argument (ivs_input_check(..., controllers, ...)), so a test can call it with counts no registered profile has (1 and 4) and show no index is fixed in the core"
    - "Guest-invisibility oracle: two instances, one unit per call, compare the CPU state after every call, then the CPU, MEM and composite digests, peeked RAM, and every instance byte outside the INPUT row (memcmp around offsetof(struct ivs_machine, obs) + offsetof(ivs_machine_obs, input))"

key-files:
  created: []
  modified:
    - data/profiles/intv-2609-ntsc-family-p1.json
    - data/expected/profile-fields.ids
    - cmake/IvsMemmapTable.cmake
    - core/src/memmap/memsys.h
    - core/include/ivs/ivs_machine.h
    - core/include/ivs/ivs_run.h
    - core/src/machine/machine.h
    - core/src/machine/machine_bound.c
    - core/src/machine/machine_input.c
    - data/api/api-effects.json
    - data/expected/api-reasons.ids
    - tests/run/run_input.c
    - data/expected/tests/run.ids

key-decisions:
  - "The extra-word rule is the last check per event in pass 2 (after flags, the reserved word and bit 31), so a word 2 or 3 with bit 31 set stays INPUT_CONTROL_RESERVED; the lowest failing index across the pass still wins, and the BAD_ARG pass still runs first"
  - "The tag is tier2-first-party-adjacent (registered in data/expected/source-authority-tags.ids), the tier of SRC-bsr-2609; the value 2 rests on its two-controllers-on-two-PSG-ports description (research-era reading, not re-read here)"
  - "ivs_profile_desc.input_controllers is uint16_t after cpu_cycles_per_scanline; the run reads it in the ladder through ivs_profile_get(im->profile) after the handle check (WR-03 covers the index)"

patterns-established:
  - "Profile-coupled refusals: the generator emits the value, the ladder reads it from the registry, the check takes it as a parameter"

requirements-completed: []

duration: about 35 min
completed: 2026-11-11
---

# Phase 4 Plan 12: input.controllers, the extra-word refusal and guest invisibility Summary

**The profile now states how many hand controllers are connected (`input.controllers` = 2). The generator carries the value into the profile registry, and `ivs_machine_run` refuses a nonzero controller word at index 2 or 3 as UNSUPPORTED (reason 54, with the failing index) before any write. A new case shows that input is recorded and digested but changes nothing the guest can see in Phase 4.**

## Performance

- **Duration:** about 35 min (load average 12-27).
- **Tasks:** 2 of 2 (the tracer first; its gate passed before Task 2).
- **Files:** 13 changed.
- **Time:** the input family (`--only run.input.`, 25 cases) takes 0.14-0.16 s on dev (ASan+UBSan) and `run.input.guest-invisible` alone takes 0.06 s, at load 12-16. The new checks are cheap: there are no new fault executables, and the run suite gains three cases.
- **Gate (D-45 local data):** the Task 1 gate (sh) read `gate-time total=39`, `gate-cpu user=80.43 sys=70.41`. The final gates read `gate-time total=30` under both sh and dash.

## Counts

Start: `steps=29/29 controls=252/252 faults=140/140 tests-expected=2426 api-calls=14 pinned-defines=58 fields=51`.
After Task 1: `faults=140/140 tests-expected=2430 tests-missing=0 pinned-defines=59`.
End: `steps=29/29 controls=252/252 api-calls=14 api-statuses=14 api-rows=38 pinned-defines=59 fault-runs=140 faults=140/140 fields=52 unknown-fields=16 tests-expected=2432 tests-executed=2432 tests-skipped=0 tests-missing=0`. tests-expected counts each case in both presets, so 3 new cases add 6. `grep -c '^input.controllers$' data/expected/profile-fields.ids` = 1 and `grep -c '^run.input.guest-invisible$' data/expected/tests/run.ids` = 1.

## What changed

- **Profile record** `input.controllers`: value 2; tag `tier2-first-party-adjacent`; cite `SRC-bsr-2609`; conflict `C14`; decision `D6`; revisit trigger "Phase 5 control-to-byte table, or a profile with the expansion connector". The note says: connected hand controllers; event word index = PSG port (0 = R14, 1 = R15); no Phase 4 consumer. No port-map, line-model or mask field was added (D-15).
- **Generator**: reads `input.controllers` with `mm_uint(... 4 ...)` and fails on 0. A missing field fails as "value must be a number". All four bad inputs (0, 5, `"2"`, field deleted) were checked by hand in a scratch tree, and each stopped the generator with `IvsMemmapTable: <profile>: input.controllers: ...`. `GEN_MAP_MUTATION` is untouched.
- **Core**: `ivs_input_check(in, ev, count, controllers, st, index)`. `machine_bound.c` passes `ivs_profile_get(im->profile)->input_controllers`. The docs in the public header (`ivs_run.h`, the input block and the refusal list), `machine.h`, `machine_bound.c` and `machine_input.c` name the new stage.
- **Reason** `IVS_REASON_INPUT_EXTRA_CONTROLLER 54`, appended and pinned in `data/expected/api-reasons.ids` in the same commit (`82e9722`).
- **api-effects**: the UNSUPPORTED per-event precedence row now names INPUT_EXTRA_CONTROLLER and gains `pinned_by: [run.input.refuse.extra-controller-word]` (before, it had no `pinned_by`). The UNSUPPORTED refusal row lists the case. The D-05 note now reads 44..54 and 48..54. `on_ok` cites `run.input.extra-word-zero-ok` and `run.input.guest-invisible`.

## Cases

- `run.input.refuse.extra-controller-word`: word 2 on the only event gives index 0. Word 2 on event 0 together with nonzero flags on event 1 gives index 0 (the lowest index wins). Word 3 on the last of three gives index 2. Bit 31 together with word 3 on the same event gives INPUT_CONTROL_RESERVED. An unsorted event 1 gives BAD_ARG at index 1 (the class order holds). Every refusal uses the SP4 oracle: report bytes stay poison, and the instance snapshot and digests are unchanged (memcmp).
- `run.input.extra-word-zero-ok`: the registry of the instance's profile holds 2. Two events with zero words 2 and 3 are applied (consumed 2, words in force, floor). `ivs_input_check` accepts word 3 when given 4 controllers and refuses word 1 at index 0 when given 1.
- `run.input.guest-invisible`: stand-in program C is project-authored and hand-assembled in the test from the Phase 2 vector encodings (MVI from `direct.json`; MVO, INCR and B as in `machine_standin.c`). It loops reading $01FE and $01FF into $0200/$0201 and counts in R3. One instance runs without events and one with ten events toggling words 0 and 1, both for 64 single-unit calls. The test checks that CPU states are equal after every call; all ten events were applied; the CPU, MEM and composite digests are equal; peeked $0200-$0201 equal $01FE-$01FF and are equal across the two instances; every instance byte outside the INPUT row is equal; and the INPUT digests differ. A comment marks Phase 5 as the point where this becomes a positive read test.
- Sensitivity was checked by hand with a temporary core edit that was not committed. Writing a constant into RAM when an event applies makes the case fail on the MEM digest. A first attempt wrote into $01FE, the PSG stub region, which ignores writes, and the case did not fail; the next item explains why.

## Deviations from Plan

**1. [Rule 2 - Doc] `core/include/ivs/ivs_run.h` and `core/src/machine/machine.h` changed (not in the plan's file list).** The public input docs said "plan 04-12 adds their profile rule", the refusal list named the UNSUPPORTED checks, and `ivs_input_check`'s prototype and doc live in `machine.h`. All three now describe the new rule. Commit `82e9722`.

**2. Why guest invisibility holds in Phase 4.** In this profile $01F0-$01FF (`memmap.region.psg_regs`) is a `stub` device: reads return the undriven value and writes are dropped. Nothing in the machine reads `obs.input.words`, so the guest cannot see input on any path. The case does not depend on this mechanism: it compares whole-instance state outside the INPUT row, so any future path from input to guest state (RAM, registers, devices) makes it fail.

**3. No new named fault** (the plan keeps `faults=140/140`). Dropping the extra-word check is caught by `run.input.refuse.extra-controller-word`. Fixing the index at 2 in the core is caught by `run.input.extra-word-zero-ok`, which calls the check with counts 1 and 4. No existing fault build needed a wider allowed prefix: all 140 passed the verdict unchanged.

## Open Items

- RUN-01 and RUN-05 stay Pending. This plan adds the profile coupling and the Phase 4 boundary test. RUN-01 still needs the runner's `input` lines (04-16 and later), and RUN-05 needs the goldens.
- D-16 bit names (`data/expected/input-bits.ids` against the header) were not in this plan's tasks, although the 04-11 SUMMARY had grouped them with 04-12. Plan 04-13 pins them, as the roadmap line for 04-13 states.
- Plan 04-13: pin the request at 64 bytes and the report at 112 (unchanged by this plan), then apply A-02.
- Phase 5: replace `run.input.guest-invisible` with a positive PSG-port read test once the control-to-byte table exists, and revisit `input.controllers` (C14, D6) at the same time.
- Carried: `API-02`, `RUN-05`, `PKG-03`, `PKG-04` Pending; 04-24 may register the input faults in `important-gates.ids`; job summary `pending-resolvers=17`, `unknown-fields=16`; `sh tools/hygiene.sh --history` still fails on earlier planning commits.

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=29/29 controls=252/252 api-calls=14 ... pinned-defines=59 ... fault-runs=140 ... faults=140/140 ... fields=52 unknown-fields=16 ... tests-expected=2432 tests-executed=2432 tests-skipped=0 tests-missing=0 ...`
- `dash tools/gate.sh`: same line, `gate: PASS`.
- `sh tools/gate-linux.sh` (Ubuntu container, gcc): `gate: PASS steps=29/29 controls=252/252 ... pinned-defines=59 cc=GNU ... fault-runs=140 ... faults=140/140 ... tests-expected=2432 ... tests-missing=0`, `gate-time total=149` (container wall 3 min 31 s), `gate-linux: exit 0`. Not required by the plan; run because of the gcc-only hazard found in 04-11.

## Threat Flags

None beyond the plan's threat model:
- T-04-35 mitigated: words of unconnected ports are UNSUPPORTED before any write, using the profile count; the refusal case uses the memcmp/snapshot oracle.
- T-04-36 mitigated: `run.input.guest-invisible` compares the register trace, the CPU, MEM and composite digests, RAM and every instance byte outside the INPUT row.
- T-04-SC: no package installs.

## Self-Check: PASSED

- Files present: the thirteen modified files listed above.
- Commits reachable from HEAD: 82e9722, 379e9af (`git rev-list --count 7c960c4..HEAD` = 2 before the docs commit).
