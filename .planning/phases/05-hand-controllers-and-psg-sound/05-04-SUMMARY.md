---
phase: 05-hand-controllers-and-psg-sound
plan: 04
subsystem: core/psg, core/memmap, core/machine, tests/psg, cmake, tools/check
tags: [psg-register-file, device-kind-psg8, ivs-psg-table, machine-bus-wrapper, suite-psg, psg-vectors-cross-check, checkpoint-1, D-04, D-08, D-12, D-25, A-01, A-05, A-06, AUD-01]
status: complete

requires:
  - phase: 05-hand-controllers-and-psg-sound
    provides: "plan 05-01 oracle commit 14a1904 (regs.txt, reset.txt, the register-address literal, rules.json); plan 05-03 psg_reg storage, ivs_psg_state, composite version 2 (HEAD at start 1d128be)"
provides:
  - "device kind psg8 (id 6, IVS_MEMSYS_DEVICE_KINDS 7): bits 8, rmask $00FF, fill $0000, tag unknown-default-chosen, C64; the psg region is a real 16-word device (only stic stays a stub)"
  - "profile fields psg.regmap (16 records, tier4-community, C70, C60, C75) and psg.mirrors (none, C74); memmap.region.psg_regs points at psg8 with impl real"
  - "cmake/IvsPsgTable.cmake (fail closed) and ivs_psg_params {reg_wmask[16], reg_rmask[16]}; core/src/psg/psg_regs.c (ivs_psg_reg_store, ivs_psg_reg_load, ivs_psg_params_get) and the three named fault sites"
  - "core/src/machine/machine_bus.c: ivs_machine_bus with the per-unit ivs_machine_bus_ctx {machine, unit start tick, ticks_per_cpu_cycle}, wired as port.bus in the step seam (the run call steps through it); forwards every access unchanged"
  - "ivs_memsys_psg_reg_internal (register store under the read mask) and a raw-poke branch for the psg region"
  - "memmap literal rows R and W 01F0-01FF split by width into 7 rows each, impl=real, with the optional x= column (reads-zero, pins) for the three exception registers; memmap.row.* ids follow; claim CL-0006 depends on memmap.device.psg8"
  - "suite psg (tests/psg, 40 ids per preset): the 30 vector ids of regs.txt and reset.txt through cmake/IvsPsgVectors.cmake, regmap-versus-literal, power-on and dirty reset, wait zero, wrapper forwards, no mirror, pure reads, the guest tracer, two judge self-controls; faults PSG_TONE_HIGH_NIBBLE_UNMASKED, PSG_REG_OCTAL_AS_HEX, PSG_SHAPE_READBACK_STORED"
  - "psg-vectors step: profile cross-check (psg-rule-field-missing, psg-field-pending-stale, psg-regmap-profile-mismatch), registration check (psg-vector-registration), data/psg/fields-pending.ids and data/psg/vectors-pending.ids, 4 controls"
  - ".planning/CI-EFFICIENCY.md section Phase 5 checkpoint 1 (checkpoint-1: lever-required)"
affects: [05-05, 05-06, 05-07, 05-08, 05-09]

actuals:
  tokens: 26000
  tasks: 5
  commits: 5
plan_head_before: 1d128beaff9d5e9b9567cff0cc1c834acc664ded
plan_head_after: 9f7c1c2eb9d5654b942bc7d3eebbb39feceb7561

tech-stack:
  added: []
  patterns:
    - "A second fail-closed generator beside IvsMemmapTable.cmake with the same -DROOT -DOUT interface, both registered in ivs_core_gen and data/expected/core-generated.ids; later plans append their psg.* fields to the same struct and generator"
    - "Vector files and a literal turned into C tables by a generator that reads neither the profile nor the core (IvsPsgVectors.cmake); the vector and literal hashes travel in the TAP header; a vector with no read expectation stops the build"
    - "A judge self-control per predicate: the predicate that judges a vector or the generated masks must reject a deliberately altered expectation (psg.judge.*)"
    - "A progressive cross-check with deletion discipline: pending-field and pending-family lists that fail when stale, so every owning plan must delete its lines in the commit that adds the field or wires the family"

key-files:
  created:
    - cmake/IvsPsgTable.cmake
    - cmake/IvsPsgVectors.cmake
    - core/src/psg/psg_params.h
    - core/src/psg/psg_regs.c
    - core/src/machine/machine_bus.c
    - tests/psg/psg_main.c
    - tests/psg/psg_cases.h
    - tests/psg/psg_reg.c
    - data/expected/tests/psg.ids
    - data/expected/faults/psg.ids
    - data/psg/fields-pending.ids
    - data/psg/vectors-pending.ids
    - tools/check/controls/psg-vectors-field-missing/
    - tools/check/controls/psg-vectors-field-pending-stale/
    - tools/check/controls/psg-vectors-regmap-profile-mismatch/
    - tools/check/controls/psg-vectors-unregistered/
  modified:
    - cmake/IvsMemmapTable.cmake, cmake/IvsMemmapProbes.cmake, core/CMakeLists.txt
    - core/src/memmap/memsys.h, memsys.c, core/src/psg/psg.h
    - core/src/machine/machine.h, machine_run.c
    - data/profiles/intv-2609-ntsc-family-p1.json, data/expected/profile-fields.ids, data/expected/core-generated.ids
    - data/memmap/intv-2609-ntsc-family-p1.literal.txt, data/claims.json
    - tests/memmap/memmap_rows.c, memmap_main.c, tests/machine/machine_standin.c, machine_cases.h, tests/run/run_observe.c, tests/CMakeLists.txt
    - data/expected/tests/memmap.ids, data/expected/faults/memmap.ids, data/expected/controls/psg-vectors.ids
    - tools/check/memmap.cmake, tools/check/psg-vectors.cmake
    - tools/check/controls/_good/tree (toy profile psg.regmap, toy literal, toy pending list, toy inventory)
    - .planning/CI-EFFICIENCY.md

key-decisions:
  - "The raw-poke seam stores the whole word into psg_reg (as for every other backing) and the read mask applies on read; the masked store (value & wmask) is only the bus write path"
  - "The shape register and the two port registers are declared exception rows (x=reads-zero, x=pins): the width probe observes their store in psg_reg, not through a bus read; the R row of the shape register has w=0 with impl=real (accepted only with x=reads-zero)"
  - "ivs_memsys_psg_reg_internal returns the register store under the read mask (the value a bus read of that address gives, upper byte 0); the register-file tests read the port latches through it and never read $01FE or $01FF through a bus read, peek or the CPU"
  - "The fault PSG_TONE_HIGH_NIBBLE_UNMASKED widens both the write mask and the read mask of the coarse tone registers, because a read mask of $000F alone would hide a store that kept the whole byte"
  - "The registration check is active only when data/expected/tests/psg.ids or data/psg/vectors-pending.ids exists and fails when only one of them does (the toy control tree has one suite and no psg.ids)"

duration: about 2 h 30 min including six gate runs and the checkpoint measurements
completed: 2026-11-12
---

# Phase 5 Plan 04: PSG register file, memmap flip and suite psg Summary

**The sixteen PSG words at `$01F0..$01FF` are a real device (kind `psg8`, upper byte 0): stores keep `value & wmask`, reads give `reg & rmask`, the masks come from the profile field `psg.regmap` through a second fail-closed generator, every guest access reaches them through a new machine-level bus wrapper, and a new suite `psg` proves the register file against vectors and a literal that no engine produced; the gate ends `steps=35/35 controls=300/300 faults=158/158` with the goldens untouched.**

Oracle commit (05-01): `14a1904` (`regs.txt`, `reset.txt` and the register-address literal were committed there, before this plan's first commit `e31e37a`; plan 05-03's last commit `d8c2553` and `1d128be` lie between).

## What was built

- Task 1, tracer (`e31e37a`): profile fields `memmap.device.psg8`, `psg.regmap`, `psg.mirrors` and the flipped `memmap.region.psg_regs`; `IvsMemmapTable.cmake` kind `psg8` (id 6) and `psg` out of `MM_STUB_PHYS`; `IvsPsgTable.cmake` (refuses: not 16 records, an address out of order or outside the block, a duplicate or wrong name or octal number, a mask above `$00FF`, an unknown or missing key, a mirrors value other than `none`); `ivs_psg_params`; `psg_regs.c` with the three fault sites; `memsys.c` PSG cases for load and store and the raw-poke branch; `machine_bus.c` and the per-unit context in the step seam. Tracer: stand-in program C (`MVII #$3FF,R0` / `MVO R0,$01F4` / `MVI $01F4,R1` / `MVO R1,$0200`) leaves `$000F` in `$0200` (checked first by a throwaway program, then kept as `psg.reg.guest-write-read`). `run.peek.undriven` moved its stub probe from `$01F0` to `$0000` and now expects the zero register at `$01F0`.
- Task 2 (`1e38f75`): the literal's PSG rows split by width (read: 8, 4, 8, 5, 0, 6, 8; write: 8, 4, 8, 5, 4, 6, 8) with `impl=real`; the optional `x=` column in the literal, `tools/check/memmap.cmake` and `IvsMemmapProbes.cmake`; the test's rule rows (`psg` at widths 8, 4, 5, 6, 0), `mr_observe` (exception rows are observed in `psg_reg`), the descriptor row, `memmap.width.psg8-low-byte-upper-zero`, `memmap.row.*` ids (13 more), `MEM_MIRROR_MASK` allowed prefixes (the fault halves every region's backing index, so it also breaks the PSG rows and the neighbour probe of `$0200`); CL-0006 `depends_on` gains `memmap.device.psg8`, standing unchanged.
- Task 3 (`9d1e2bb`): `IvsPsgVectors.cmake` (family `reg` and `reset`; refuses an unevaluated expect kind, a pin, a write after step 0, an address outside the literal, a read above `$00FF`, a vector with no read), suite `psg` in its own executable, `psg.ids` (40 ids), `faults/psg.ids` and the three faults in the same commit (the gate fails any fault site that is not registered, see Deviation 1). Measured plain-run times: dev 0.072 s, release 0.004 s.
- Task 4 (`788c57b`): the `psg-vectors` cross-check with the four tokens, `fields-pending.ids` (19 lines: 05-05 three, 05-06 nine, 05-07 seven), `vectors-pending.ids` (nine families: port 05-05; tone, mix, amp 05-06; noise, env, dac, ampenv, reseng 05-07), four controls, and the toy tree extended (a toy literal, a toy profile `psg.regmap`, an empty toy pending list).
- Task 5 (`9f7c1c2`): the checkpoint measurement and the CI-EFFICIENCY section.

## Checkpoint 1 (V-16), copied from `.planning/CI-EFFICIENCY.md`

- Local gate, three runs: `gate-time total=32, 32, 35`; `gate-cpu` 103.7, 103.3, 112.6 (user + sys); `gate-env ncpu=18 cc=AppleClang-21.0.0.21000101`; largest steps controls 13 to 15, c-faults 7 to 8, c-tests 5, hygiene 3, cp1610-vectors 3 or c-purity 4; `dash` `gate-time total=32`.
- Unit costs (local): 0.144 cpu-s per fault (the `c-faults` step alone: 22.8 cpu-s for 158 faults), 0.185 cpu-s per control on macOS (0.04 on native Linux per V-16), `psg` suite 0.072 s dev and 0.004 s release for 40 ids (1.7 ms and 0.1 ms per id). Below the 0.4 cpu-s per fault threshold: no fault is converted to a judge self-control.
- Hosted rows (`.planning/ci-runs.tsv`, successes): last-5 `cpu_s` median 179, `gate_s` median 59 s. Projection: +9.7 cpu-s (native-Linux unit costs) gives 188.7; the laptop-scaled estimate gives 195.5; `gate_s` 63 to 67 s against the 90 s trigger.
- `checkpoint-1: lever-required` (the conservative estimate reaches 190; the hosted spread of 136 to 252 cpu-s is larger than the whole projected add). Lever list for the orchestrator (05-L1 is not created here): (1) batch the `cp1610-vectors` JSON re-parse (about 5 cpu-s, no evidence change); (2) limit fault reruns to the suites a fault names; (3) background `c-build` only if idle cores show (`cpu_s / (ncpu x gate_s)` is 0.76 today). The next hosted row decides which reading is closer; a last-5 median of 190 or more fires the rule on its own.

## Deviations from Plan

**1. [Rule 3 - Blocking] The fault registrations moved from Task 4 into the Task 3 commit.** The gate fails any `IVS_FAULT_*` site that is not in a `data/expected/faults/*.ids` file (`fault-orphan-site`), and the three sites exist since Task 1. The commits of Tasks 1 and 2 therefore end with the gate failing only on those three findings; the Task 3 commit registers the suite and its faults together and is gate-green (`controls=296 faults=158`); Task 4 adds the four controls (`controls=300`).

**2. [Rule 1 - Plan precision] Two fault shapes differ from the one-line description.** `PSG_TONE_HIGH_NIBBLE_UNMASKED` widens the write mask and the read mask of the coarse tone registers (a read mask of `$000F` alone hides a store that kept the whole byte, so the plan's "the store keeps the low byte" would not have been detected). `PSG_REG_OCTAL_AS_HEX` places the amplitude registers one slot low in both the store and the load (R10 lands at `$01FA`); only the vectors that write two registers or read an amplitude against a neighbour's mask see it, so its collateral list is `psg.reg.` and a lone amplitude B or C round trip stays consistent.

**3. [Rule 1 - Bug caught by the judge] The first generated vector include had no read expectations.** CMake keeps nine match groups (`CMAKE_MATCH_1..9`) and the vector regex captured the expect value as group 10, so every vector passed vacuously. `psg.judge.mask-row-changed` failed on its `n_reads == 1` check; the expect value is now matched on its own, and the generator stops the build on a vector with no read expectation.

**4. [Rule 3] The raw-poke seam stores raw into `psg_reg`.** The plan routes the `ivs_mm_store` PSG case through `ivs_psg_reg_store` (the bus write path, which the memmap probe measures); the poke seam is documented to store unmasked, so it has its own branch and the read mask applies on read.

**5. [Rule 3] One optional column `x=` in the memmap literal** (allowed by the plan): `x=reads-zero` and `x=pins`; `impl=real` with `w=0` is accepted only with `x=reads-zero` (the shape register's read row). No control was added; the existing `memmap` step controls stay green.

**6. [Rule 3] The toy control tree has no `data/expected/tests/psg.ids`.** The toy `tests-inventory` tree has one suite and adding a second would touch its twelve controls. The registration check is active when either `psg.ids` or `vectors-pending.ids` exists and fails when only one does, so the real tree is fail-closed; the `psg-vectors-unregistered` overlay supplies both files. The toy tree otherwise follows the plan (toy literal, toy profile `psg.regmap`, toy pending list).

**7. Other.** `memmap.width.stub-reads-ffff` moved its PSG probes to STIC addresses and a new id `memmap.width.psg8-low-byte-upper-zero` covers the PSG width; `psg.bus.wrapper-forwards` uses non-PSG accesses only (plans 05-06 and 05-08 change the wrapper for PSG writes and will add their own test); `fields-pending.ids` also lists `psg.sample_period_ticks` and `psg.clock_phase_ticks`, named by the plan and by plan 05-06 though no rule names them (harmless: a pending line fails only when its field exists); `MEM_MIRROR_MASK` gained allowed prefixes (see Task 2). The `.planning/phases/05-hand-controllers-and-psg-sound/05-04-PLAN.md` estimate of 5 tasks and 5 commits held.

## Decisions Made

- The memmap probe reads the write view of an exception register from `psg_reg`; plan 05-05 changes the read of `$01FE` and `$01FF`, which then reads 0 after a raw poke in the read-view probe, so its literal read rows need `w=0` and `x=pins` handling like `$01FA`.
- The register-file vectors run on a bare memory system (family `reg`) or on an instance whose register words were dirtied and then reset (family `reset`), so a reset that did not clear them fails.
- `psg.bus.wait-zero` runs all nine access kinds at all 65,536 addresses (590k calls, a few milliseconds in release) rather than a sample: U and g depend on it (A-03).

## Open Items

- **AUD-01 stays Pending in REQUIREMENTS.md.** Only the register file has passing vectors; tone, mixer, noise, envelope, DAC and the ports close it (plans 05-05 to 05-07).
- **Plan 05-05 must update the memmap probe and the literal rows of `$01FE` and `$01FF`** when the bus read follows the pins: the read-view width of those rows becomes 0 (`x=pins`), like the shape register's; the declared exception in `mr_is_exception` and the `x=pins` columns are the hooks. `data/api/api-effects.json` still says "Nothing in the machine reads the words" (05-02 item, 05-05 rewrites it).
- **Deletion discipline for later plans:** each plan deletes its lines of `data/psg/fields-pending.ids` and `data/psg/vectors-pending.ids` in the commit that adds the field or wires the family (the step fails stale lines and unregistered families); plan 05-07 leaves both files without a line.
- **Checkpoint 1 is `lever-required`** on the conservative estimate (see above); the orchestrator decides whether to create 05-L1 now or after the next hosted rows.
- `sh tools/gate-linux.sh` was not run (the plan does not require it; the validation list requires it before pushing 05-02, 05-03 and 05-06 onward). The new C is written for gcc 13 (no argument reads a state another argument changes, no GOT symbols); the hosted-only classes are unchecked until that run.
- Carried from earlier plans (unchanged): 05-02 items (`run.info.v3-fields-zero`, `run.struct.report-v2-fields` to be replaced in 05-06 and 05-08), 05-03 items (`runner-process-tap.sh` flake under dash was fixed in `9cfb55f`; goldens re-bless in 05-06 and 05-08), the unobtained AY-3-8910 reprint and pin chart, the D10 and D11 items, and the Phase 4 items IN-04, IN-06, IN-09 to IN-11.

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=35/35 controls=300/300 ... faults=158/158 ... tests-expected=2968 tests-executed=2966 tests-skipped=2 tests-missing=0`, `pinned-defines=95`, `psg-rules=21 psg-vectors=120 psg-policies=12`, `gate-time total=32` to 35 (five runs), 0 FAIL lines; `git diff --stat -- data/golden` empty (goldens unchanged).
- `dash tools/gate.sh`: the same line, `gate-time total=32`, 0 FAIL lines.
- `sh tools/gate-linux.sh`: not run (not required by the plan).
- `psg` suite plain-run time (median of 11, loaded laptop): dev (ASan and UBSan) 0.072 s, release 0.004 s, 40 cases.

## Threat Flags

None new. T-05-10 (a bad regmap in data): mitigated by the `IvsPsgTable.cmake` refusals, `psg.regmap.matches-literal`, the `psg-vectors` comparison and the control `psg-vectors-regmap-profile-mismatch`. T-05-11 (a store outside the block or beyond a mask): mitigated by the masked store in the memsys write path of the PSG region only, `psg.reg.no-mirror` (one word changes per store; no address outside the block reaches a register), and the faults `PSG_TONE_HIGH_NIBBLE_UNMASKED` and `PSG_REG_OCTAL_AS_HEX`. T-05-SC: no installs.

## Self-Check: PASSED

- Files exist: cmake/IvsPsgTable.cmake, cmake/IvsPsgVectors.cmake, core/src/psg/psg_regs.c, core/src/psg/psg_params.h, core/src/machine/machine_bus.c, tests/psg/psg_reg.c, tests/psg/psg_main.c, tests/psg/psg_cases.h, data/expected/tests/psg.ids, data/expected/faults/psg.ids, data/psg/fields-pending.ids, data/psg/vectors-pending.ids.
- Commits on HEAD: e31e37a, 1e38f75, 9d1e2bb, 788c57b, 9f7c1c2.
