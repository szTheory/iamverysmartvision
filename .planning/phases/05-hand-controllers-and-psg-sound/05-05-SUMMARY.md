---
phase: 05-hand-controllers-and-psg-sound
plan: 05
subsystem: core/machine, core/memmap, core/psg, cmake, tests/input, tests/psg, tests/run, tools/check
tags: [hand-controllers, psg-ports, ctrl-table-generator, input-suite, port-pulldown, api-03, c67, D-01, D-02, D-03, D-04, D-05, D-06, A-08, INP-01, INP-02, INP-03, API-03]
status: complete

requires:
  - phase: 05-hand-controllers-and-psg-sound
    provides: "plan 05-01 oracle commit 14a1904 (the controller literal data/input/intv-2609-ntsc-family-p1.literal.txt, input-policies.ids, the input step; the second-read record docs/input/second-read-record.md, 31 of 31 rows agree); plan 05-03 port_pulldown[2] and psg_reg in the MEM rows; plan 05-04 the psg8 register file, the machine bus wrapper, the x= hook and the pending lists (HEAD at start 723f789)"
provides:
  - "35 profile fields: input.control.<name> (31, {lines}, tier4-community, C16), input.line_model ground-union (inference), input.line_idle $FF (tier4-community), input.port_output_read latch (unknown-default-chosen, C65), psg.mixer.port_dir_bits {A 6, B 7, output 1} (unknown-default-chosen, C73); masks typed by hand from the literal"
  - "cmake/IvsCtrlTable.cmake (fail closed, -DMUTATE hook): ivs_ctrl_lines[profile][31] in a build-tree include listed in core-generated.ids and built by ivs_core_gen; IvsPsgTable.cmake carries port_idle, port_output_read_latch, port_dir_bit[2], port_dir_output into ivs_psg_params"
  - "ivs_input_apply derives port_pulldown from the words in force through ivs_input_rederive (the one function that sets the masks); the memsys read of $01FE and $01FF follows R7 and the pins; the refused-step take-back re-derives from the restored words; power-on clears the masks"
  - "suite input (123 ids per preset, own executable, 7 named faults), the psg port vector family (10 vectors plus one judge) wired into suite psg, run.input.guest-visible (the flipped Phase 4 case), run.fuzz.input-visible-seed-01..08 and -covered, the input step profile cross-check with 3 controls"
affects: [05-06, 05-07, 05-08, 05-09, 05-11, 05-12]

actuals:
  tokens: 52000
  tasks: 5
  commits: 6
plan_head_before: 723f78966b72a7625534a55d539c7131d70cea9d
plan_head_after: faa1644daadc135107642258b67511f86a1fc7c3

tech-stack:
  added: []
  patterns:
    - "A third fail-closed generator beside IvsMemmapTable and IvsPsgTable, all three with the -DROOT -DOUT interface and registered in ivs_core_gen and core-generated.ids; the table is data and the core holds no mask, idle byte or bit number"
    - "An independent probe generator (IvsInputProbes.cmake) that reads only the sourced literal; the expected byte of every test comes from it or from a literal written in the test, never from the profile, ivs_ctrl_lines or a core output"
    - "A one-unit-per-call rig (ii_rig_set: an event at machine time, run(now + 1)) that makes a property over all pairs and triples of controls cost one unit per sample"
    - "Fuzz content with a constant-cycle guest gets an independent oracle from the schedule and observed unit starts; content whose timing depends on what it reads is judged against the one-unit-per-call reference, with whole-instance snapshots instead of MEM digests"

key-files:
  created:
    - cmake/IvsCtrlTable.cmake
    - cmake/IvsInputProbes.cmake
    - tests/input/input_main.c
    - tests/input/input_cases.h
    - tests/input/input_single.c
    - tests/input/input_combo.c
    - tests/input/input_port.c
    - tests/input/input_slice.c
    - tests/psg/psg_port.c
    - data/expected/tests/input.ids
    - data/expected/faults/input.ids
    - tools/check/controls/input-profile-mask-mismatch/
    - tools/check/controls/input-profile-field-missing/
    - tools/check/controls/input-profile-field-shape/
  modified:
    - data/profiles/intv-2609-ntsc-family-p1.json, data/expected/profile-fields.ids, data/expected/core-generated.ids, data/psg/fields-pending.ids, data/psg/vectors-pending.ids
    - core/CMakeLists.txt, core/src/machine/machine.h, machine_input.c, machine_bound.c, machine_run.c, core/src/memmap/memsys.c, core/src/psg/psg_params.h, core/include/ivs/ivs_run.h
    - cmake/IvsPsgTable.cmake, cmake/IvsPsgVectors.cmake
    - data/memmap/intv-2609-ntsc-family-p1.literal.txt, tools/check/memmap.cmake, tools/check/input.cmake, tools/check/steps/input.sh
    - data/api/api-effects.json, docs/runner.md
    - tests/CMakeLists.txt, tests/machine/machine_standin.c, machine_cases.h, tests/memmap/memmap_main.c, memmap_rows.c, tests/psg/psg_main.c, psg_cases.h, tests/run/run_input.c, run_fuzz.c, run_cases.h, run_main.c, run_observe.c, tests/runner/runner_exec.c
    - data/expected/tests/run.ids, psg.ids, data/expected/faults/run.ids, memmap.ids, data/expected/controls/input.ids
    - tools/check/controls/_good/tree (toy profile with two input.control.* fields, toy inventory)

key-decisions:
  - "The read view of $01FE and $01FF has width 0 in the memmap literal (x=pins): the byte read does not depend on the word stored, so the width probe sees no stored bit; tools/check/memmap.cmake accepts impl=real with w=0 for x=pins as it does for x=reads-zero, and the memmap read probe expects the idle byte 00FF at those two addresses after power-on whatever was poked"
  - "ivs_input_rederive is the only setter of port_pulldown: apply and the take-back call it, so the invariant port_pulldown == derive(words) holds after every call and a fault that skips either call is visible to a test"
  - "The port read lives in memsys.c (ivs_mm_port_read), pure, reading R7 through ivs_psg_reg_load and the direction bit, idle byte and latch flag from ivs_psg_params; the slots of the mixer and the two ports are named constants in psg_params.h"
  - "The input-visible fuzz keeps the whole instance storage after every reference unit and compares bytes at every landing boundary; it replaces the MEM digest compare of the plan because the digest costs a SHA-256 of 113,760 bytes per boundary (2.1 s dev for the nine ids) where the memcmp costs 0.12 s dev and is stronger"
  - "The combination ambiguity counts are computed with one stated definition (a pair state is ambiguous when its union equals the union of a single control or of a different pair): key+side 36 of 36, side+side 3 of 3, key+disc 180 of 192, equal to the plan 05-01 research figures"

duration: about 6 h including ten gate runs, two container runs and the fuzz cost work
completed: 2026-11-12
---

# Phase 5 Plan 05: Input byte and port reads Summary

**Both hand controllers are guest-visible machine input: an applied event derives the pull-down mask of each PSG port from the words and a generated, fail-closed control table (35 tier-tagged profile fields), a CPU read of `$01FE` or `$01FF` returns the idle byte XOR that mask (the stored latch in output mode), and the 62 single-control reads, 24 combination rows, the pair properties, the C67 instruction-granularity test and the poller fuzz prove it against a literal and a relational reference that no engine produced; the gate ends `steps=35/35 controls=303/303 faults=166/166` with the goldens untouched.**

Oracle commit (05-01): `14a1904` (the controller literal, `input-policies.ids` and the `input` step were committed there; the orchestrator's blind second read `docs/input/second-read-record.md` agrees on 31 of 31 rows and was the plan's precondition). Plan start `723f789`.

## What was built

- Task 1, tracer (`e3e95f1`): the 35 profile fields; `IvsCtrlTable.cmake` (refuses a missing or extra name, a mask outside 01..FF, an unknown line model, a profile with controllers and no table, an unknown key, a `-DMUTATE` target that is not a control) and the port parameters in `IvsPsgTable.cmake`; `ivs_input_apply` (now machine-wide) calls `ivs_input_rederive`; `ivs_mm_port_read` in `memsys.c`; the take-back re-derives; the memmap read rows of the two ports became `w=0 x=pins`. Tracer `input.anchor.wiki-key1`: a KEY_1 event on port 0 reads `$007E` by a stand-in guest program and by `ivs_machine_peek`.
- Task 2 (`0f01337`): `IvsInputProbes.cmake` (reads only the literal and `input-bits.ids`); the 62 `input.single.p<0|1>.<name>` ids (guest read, peek, other port idle), idle, anchors, the five structure facts, `input.table.matches-literal`, the port semantics (10), digest, take-back, reset, invariant and two judges; the `port` vector family wired into suite `psg` (`pin=<port>:<HH>` through `ivs_memsys_set_port_pulldown_internal`, 10 vectors and `psg.judge.port-row-changed`); `run.input.guest-invisible` became `run.input.guest-visible`; the `ivs_run.h` comment and the api-effects sentence rewritten; the `input` step compares the 31 `input.control.*` fields with the literal (3 tokens, 3 controls, toy profile).
- Task 3 (`a1985ea`): 24 `input.combo.*` rows (each with its class of `input-policies.ids`; classes tagged `unknown-default-chosen` are labelled policy rows, C63) and `input.pairs.union|commutative|monotone-clear|decodable-claims` over all 465 unordered pairs and 4,495 triples through the machine.
- Task 4 (`6253ad0`): stand-in pollers `IVS_MS_PROG_POLL_CONST` (content B) and `IVS_MS_PROG_POLL_BRANCH` (content C); `run.fuzz.input-visible-seed-01..08` and `-covered`; `input.boundary.start-minus-1|start|start-plus-1` (the poll start tick observed by single-stepping), `input.slice.guest-visible`, `input.iso.interleaved-instances`; `docs/runner.md` gained the C67 sentence.
- Task 5 (`400f100`): the seven input faults (`INPUT_PINS_NOT_UPDATED`, `CTRL_PORTS_SWAPPED`, `CTRL_UNION_AS_AND`, `CTRL_NO_INVERSION`, `INPUT_PINS_NOT_RESTORED_ON_TAKEBACK`, `INPUT_PINS_NOT_RESET`, `GEN_CTRL_MUTATION`) and the deferred Phase 4 `INPUT_EXTRA_CONTROLLER_UNCHECKED` in suite `run`.
- Fix (`faa1644`): see Deviation 5.

## Evidence numbers

- Test ids: `input.ids` 123 (62 singles, 24 combos, 4 pairs, 15 port, 5 slice and boundary, 13 other fixed), `psg.ids` 51 (+11), `run.ids` 157 (+9); `tests-expected=3254` over both presets.
- Plain-run times (median of 11, loaded laptop): `input` dev (ASan and UBSan) 0.094 s, release 0.022 s (targets 1.5 s and 0.3 s); `run` dev 0.55 s, release 0.30 s (before this plan 0.51 s and 0.29 s); the input-visible fuzz alone 0.12 s dev and 0.016 s release.
- Gate cost (local, three runs after the last commit): `gate-time total=32` to 33, `gate-cpu` 102 to 107 (user + sys; the 05-04 checkpoint measured 103 to 113, so no visible rise); hosted effect not yet measured. The container run (gcc 13.3, cmake 3.28, dash as sh, amd64 emulated) took `gate-time total=130`.
- Faults: 7 + 1, each fails its named id and only the collateral ids of its prefix list; `INPUT_PINS_NOT_RESTORED_ON_TAKEBACK` fails only `input.takeback.restores-pins` in suite input.
- Phase 4 faults that gained allowed prefixes: `PEEK_ADVANCES_TIME` gained `run.fuzz.input-visible-` (the fuzz peeks the poller log of its reference), `MEM_MIRROR_MASK` gained `memmap.row.R-0200-` and `memmap.anchor.psg-`. None of `INPUT_APPLY_AT_RUN_START`, `RUN_EVENT_AT_TARGET_ONLY`, `INPUT_NO_FLOOR_CHECK`, `INPUT_LOG_SLICE_RELATIVE_TICK`, `INPUT_STATIC_CARRY` needed an edit (their prefixes already cover `run.input.` and `run.fuzz.`).
- Goldens: `git diff --stat -- data/golden` is empty after every gate run (existing scenarios leave the words at zero at the end of their final digests).

## Deviations from Plan

**1. [Rule 3 - Blocking] The fault sites were first written in Task 1 and held back until Task 5.** The gate fails any `IVS_FAULT_*` site that is not registered (`fault-orphan-site`), so the sites were stripped from the Task 1 to 4 commits and added in Task 5 with their registrations, as the plan's task list has it; every task commit is gate-green.

**2. [Rule 1 - Plan precision] Read view of the ports.** The plan did not name the memmap literal row; as the 05-04 summary announced, `R 01FE-01FF` became `w=0 ... x=pins`, `tools/check/memmap.cmake` accepts `impl=real` with `w=0` for `x=pins`, `memmap_rows.c` expects 00FF at those two addresses for both pokes, and `memmap_main.c`'s anchor probe reads 00FF and the stored latch through `ivs_memsys_psg_reg_internal`. The write rows stay `w=8 x=pins`.

**3. [Rule 3] `runner.exec.input-schedule` (tests/runner/runner_exec.c, not in the plan's file list).** The scenario holds nonzero controller words at its end, so `digest.mem` and `digest.composite` now differ (the pull-down masks are MEM rows); both joined the test's `may_differ` list. The CPU, images and instance digests still do not differ (the diagnostic never reads the ports).

**4. [Rule 3] Fuzz cost.** The first version compared the MEM digest at every boundary: 2.1 s dev and 1.3 s release for nine ids. The reference now keeps the instance storage after every unit and the slicing run compares bytes at every landing boundary (CPU and INPUT digests stay): 0.12 s and 0.016 s, and stronger than the digest compare the plan named.

**5. [Rule 1 - Bug, hosted-only class caught by the container] Evaluation order in `ivs_ms_powered_prog`.** The first version passed `n` to `ivs_ms_load` in the argument list of the same statement chain that `ivs_ms_cart` sets it through `&n`; gcc reads it first (0), so every instance built by the new helper failed to create on gcc (118 of 123 input ids and the fuzz). Apple clang evaluates left to right and hid it; the first `gate-linux` run showed it, `faa1644` builds the cartridge in its own statement. This is the V-25 hazard the plan required the container run for.

**6. [Other]** `psg.judge.port-row-changed` (one id beyond the 10 vectors) follows the judge-per-predicate pattern of 05-04; a tenth `input.port.*` id (`input.port.r7-other-bits-ignored`) fills the plan's count of ten; `docs/runner.md` and the memmap literal header received the sentences the plan names or implies; the toy control tree gained `input.control.key_1` and `key_2` in its profile and its `profile-fields.ids`.

## Decisions Made

- A pair state is ambiguous when its union equals the union of a single control or of a different pair; with it the plan 05-01 figures reproduce (key+side 36/36, side+side 3/3, key+disc 180/192). No count differed, so nothing was forced.
- The profile tag of `psg.mixer.port_dir_bits` is `unknown-default-chosen`, as the plan states; D-04 and the `default_policy` text of C73 call the sense `inference`. Noted as an open item (the two readings do not change behaviour).
- Fields added here carry `decision: null`; `input.controllers` keeps D6.

## Open Items

- **INP-01, INP-02, INP-03 and API-03 stay Pending in REQUIREMENTS.md.** The gate evidence exists (62 singles on two paths, 24 rows and the pair properties, the field-shape control and the unchanged symbol guards, the poller fuzz), but plan 05-12 owns the important-gates registry and the traceability edit for all six Phase 5 requirements, and API-03 also names the runner slicing replay and the installed-consumer replay of later plans. Nothing is marked here.
- C73 wording: the discrepancy's `default_policy` and D-04 say `inference` for the direction-bit order; the profile field carries `unknown-default-chosen` (plan text). A later plan that touches C73 can align them.
- Hosted cost of this plan is unmeasured: the local gate CPU is unchanged within noise (102 to 107 user + sys), the added faults (8) and controls (3) follow the 05-04 unit costs (about 1.2 cpu-s and 0.6 cpu-s on macOS), `input` costs 0.09 s dev and 0.02 s release. The next hosted row decides whether the 280 cpu-s trigger is near.
- The port read is pure and the unplugged-port rule is by inference (a word at an index at or above `input.controllers` is refused, so the port reads idle); no measurement backs either (D10 items C65, C73 stay open).
- The container gate runs on emulated amd64 on this laptop and its time (about 130 s of gate time, 525 cpu-s user) says nothing about the hosted runner; it is a correctness check only.
- Carried from earlier plans (unchanged): 05-02 items (`run.info.v3-fields-zero`, `run.struct.report-v2-fields`), 05-03 goldens re-bless in 05-06 and 05-08, 05-04 AUD-01 pending and the unobtained AY-3-8910 reprint, the D10 and D11 items, and the Phase 4 items IN-04, IN-06, IN-09 to IN-11.

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=35/35 controls=303/303 ... faults=166/166 ... tests-missing=0`, 0 FAIL lines, `gate-time total=33`.
- `dash tools/gate.sh`: the same line, `gate-time total=33`, 0 FAIL lines.
- `sh tools/gate-linux.sh` (Ubuntu 24.04 container, gcc 13.3, cmake 3.28, dash): `gate: PASS steps=35/35 controls=303/303 ... faults=166/166 ... tests-missing=0`, `cc=GNU` (13.3.0), 0 FAIL lines, `gate-time total=135` (emulated amd64, 8 cpus), exit status 0; the log `build/gate-linux.log` ends with the `gate: PASS` line. The first container run (before `faa1644`) failed with 118 input ids and the poller fuzz red: Deviation 5.

## Threat Flags

None new. T-05-12 (stale pulldown after a take-back): mitigated by `ivs_input_rederive` in the take-back, `input.takeback.restores-pins`, the invariant test after every call (also in the fuzz), and the fault `INPUT_PINS_NOT_RESTORED_ON_TAKEBACK`. T-05-13 (host vocabulary in the core or its profile): the `input-profile-field-shape` control, the field names are the pinned `IVS_CTRL_*` set only, `core-symbols` and `c-purity` green and unchanged. T-05-14 (a wrong or partial table): fail-closed `IvsCtrlTable.cmake`, `input.table.matches-literal` and the fault `GEN_CTRL_MUTATION`, three step controls. T-05-SC: no installs.

## Self-Check: PASSED

- Files exist: cmake/IvsCtrlTable.cmake, cmake/IvsInputProbes.cmake, tests/input/input_main.c, input_single.c, input_combo.c, input_port.c, input_slice.c, tests/psg/psg_port.c, data/expected/tests/input.ids, data/expected/faults/input.ids, the three `tools/check/controls/input-profile-*` directories.
- Commits on HEAD: e3e95f1, 0f01337, a1985ea, 6253ad0, 400f100, faa1644 (`commits: 6` measured from the ledger, `git rev-list --count 723f789..HEAD`).
