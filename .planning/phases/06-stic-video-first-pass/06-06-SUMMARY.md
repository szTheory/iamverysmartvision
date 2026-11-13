---
phase: 06-stic-video-first-pass
plan: 06
subsystem: core/memmap, core/stic, core/machine, tests/stic, data/golden
tags: [stic14-device, register-masks, event-table, catch-up, hook-classes, blocked-policy, display-enable, mode-select, suite-stic, golden-rebless-2, classified-diff, checkpoint-A, D-04, D-05, D-06, D-07, D-10, A-01, A-06, A-07, D-21, VID-01]
status: complete

requires:
  - phase: 06-stic-video-first-pass
    provides: "plan 06-04 ivs_stic_params (windows, 16-event table, 64 register masks, mode-select list, effect rows); plan 06-05 stic_reg[64], ivs_stic_state, composite v3 (gate steps=36/36 controls=330/330 faults=197/197 profile-keys=142)"
provides:
  - "device kind stic14 (IVS_MEMSYS_DEVICE_STIC14 7, kinds 8): all five STIC regions impl real, stic a backed 64-word physical device, profile field memmap.device.stic14, the stub device kept for a later extension stub with no user"
  - "register storage under the per-register masks of ivs_stic_params: read = (stored & rmask) | (FFFF & ~rmask), store = value & wmask, strobe and reserved store nothing, raw poke stores whole words, peek stays pure"
  - "core/src/stic/stic_events.c: ivs_stic_catch_up over the 16-event table and the cursor (frame_index, event_index), tie rule A-06, gates flipped through ivs_memsys_set_gate_internal; the machine seam calls it at the end of every unit after the PSG catch-up"
  - "generated per-granule hook class tables (read and write view; NONE, PSG, STIC_REGS, GMEM, BACKTAB, STIC_ALIAS_READ) from profile data only, ivs_memsys_hook_class_internal"
  - "core/src/stic/stic_regs.c: pure effect decode (strobe arm, mode select) and the saturating blocked counters; machine_bus.c stamps, catches up, gates, counts and applies effects"
  - "suite stic: 45 ids, 5 faults, cmake/IvsSticProbes.cmake (independent literals only); control memmap-exception-unknown; declared exception x=strobe"
  - "golden re-bless 2, classified; checkpoint A recorded in .planning/CI-EFFICIENCY.md (checkpointA: no-lever)"
affects: [06-07, 06-08, 06-09, 06-10, 06-11, 06-12, 06-13, 06-14]

actuals:
  tokens: 150000
  tasks: 5
  commits: 5
plan_head_before: 103314057dba7266540bd2452a70c14134bdc8e9
plan_head_after: 625c4a5c41e50567c29b846350411090d023552c

tech-stack:
  added: []
  patterns:
    - "Event table as data plus a cursor that is state: the next event tick is phase + frame_index x F + event_tick[event_index], never to_tick / F; an access stamped at an event tick sees the event applied (A-06)"
    - "A probe generator (test side) that reads only the independent literals and a hook class table (core side) that reads only the profile, so the 65,536-address completeness test compares two derivations that share no input"
    - "Judge self-controls: each checking function is run on a corrupted copy of its oracle input and must fail"

key-files:
  created:
    - core/src/stic/stic_events.c, core/src/stic/stic_regs.c
    - cmake/IvsSticProbes.cmake
    - tests/stic/stic_main.c, stic_cases.h, stic_events.c, stic_access.c, stic_progs.c, stic_reg.c, stic_hook.c, stic_profile.c, stic_judge.c
    - data/expected/tests/stic.ids, data/expected/faults/stic.ids
    - tools/check/controls/memmap-exception-unknown/
  modified:
    - cmake/IvsMemmapTable.cmake, cmake/IvsMemmapProbes.cmake, tools/check/memmap.cmake
    - core/CMakeLists.txt, core/src/memmap/memsys.h, memsys.c, core/src/stic/stic.h, core/src/machine/machine.h, machine_bus.c, machine_run.c
    - data/profiles/intv-2609-ntsc-family-p1.json, data/memmap/intv-2609-ntsc-family-p1.literal.txt, data/claims.json, data/expected/profile-fields.ids, data/expected/controls/memmap.ids, data/expected/tests/memmap.ids, data/expected/faults/memmap.ids, data/expected/faults/machine.ids, data/stic/fields-pending.ids, data/stic/tests-pending.ids
    - data/golden/diag-cpu-1.digests, diag-input-pass.digests, diag-ctrl-psg-pass.digests, timeout-budget.digests, data/expected/pkg-consumer.golden
    - tests/CMakeLists.txt, tests/memmap/memmap_main.c, memmap_rows.c, tests/run/run_observe.c, tests/machine/machine_digest.c
    - .planning/CI-EFFICIENCY.md

key-decisions:
  - "ADAR is a read-like kind for the hook (plan text: read, fetch, write, DW only). The operand read of a direct-addressed instruction (MVI $0021, R0, the idiom that selects Color Stack) is kind ADAR in cp1610.c; unhooked it would never reach the STIC, which is exactly the hole T-06-07 names. The class lookup and the wrapper treat ADAR as READ, and stic.hook.completeness covers READ, FETCH and ADAR"
  - "The memmap literal rows are the merged rows the dump probe produces (adjacent addresses with the same structure are one row); the read-mask-0 rows are x=reads-zero, the write rows that store nothing (strobe and reserved registers) are x=strobe, and the write-only stored registers' write rows are x=reads-zero; the test observes every STIC write-view address in the register storage"
  - "ivs_stic_catch_up takes an opaque ivs_stic_sink pointer (NULL today); the sink type is defined by plan 06-09"
  - "Delay and edge tests assert outcomes (a blocked access), never the cycle arithmetic; windows are judged by relations to the parameters and by perturbation of a stack copy of them"

duration: about 9 hours elapsed on a loaded laptop (nine sh gate runs of 1.5 to 2.5 minutes, one dash run, one container run)
completed: 2026-11-12
---

# Phase 6 Plan 06: STIC register file, windows, handshake, hook tables and suite stic Summary

**The five STIC regions are the real device `stic14`: a CPU store keeps `value & wmask`, a read gives the register under its read mask with the undriven fill above it (write-only registers read FFFF, collision registers FC00 OR their bits), the frame timeline is a 16-event table walked by a cursor that is state, the machine catches the STIC up at every unit end and before every STIC, GROM, GRAM, BACKTAB and alias-read access, a closed window blocks and counts the access, an open one applies the display-enable strobe and the mode select, and a read of `$4021` (decoded to the cartridge) switches the mode without altering what the CPU receives. Suite `stic` (45 ids, 5 faults) judges it against the independent literals; the goldens moved only on `digest.mem` (one golden), `digest.instance` and `digest.composite`.**

Oracle commit this plan follows: `eae8ca5` (06-01 task 5). The 06-01 oracle files were read, never edited (the register literal is a generator input of `cmake/IvsSticProbes.cmake`, the memmap literal rows were re-split by the width rule it states). Plan start: `1033140`; first commit of the plan `c9f5125`.

## What was built

- Task 1, tracer (`c9f5125`): generator and device flip. `stic14` after `psg8`, `MM_STUB_PHYS` removed (every physical device is backed; a stub region needs only impl stub and no region has to use it), `MM_CAP_stic`. Profile: `memmap.device.stic14` (14 bits, rmask 3FFF, fill FFFF, keep), five regions to `stic14` and impl real, `profile-fields.ids` +1 (143), pending line deleted, CL-0006 gains the dependency. `memsys.c`: load, store and raw poke for the stic physical device, read mask refined per register from the generated parameters, the fourth-table guard in `ivs_memsys_init`. Literal: STIC rows split by width, `x=strobe` in the three hard-coded places (`tools/check/memmap.cmake` twice, `cmake/IvsMemmapProbes.cmake`, `tests/memmap/memmap_rows.c`), control `memmap-exception-unknown`. Tracer check: `mm_width_stic14` writes `00FF` at `$0018` and reads `FCFF` (and `FCFF` at the alias `$0058`).
- Task 2 (`8c867a7`): `stic_events.c`, seam call after the PSG catch-up, bus-context field, suite `stic` (events, reset, slice families), three faults, golden re-bless 2, `md_stepped` literals re-recorded.
- Task 3 (`9cce38e`): hook class generator (BACKTAB and read-view alias granules from profile data), `stic_regs.c`, wrapper effects and blocked counters, stand-in guests, access family, two faults.
- Task 4 (`ca7dfd1`): `IvsSticProbes.cmake`, register rows, hook completeness over all 65,536 addresses, window perturbation, three judges.
- Task 5: checkpoint A, the removal of one unused accessor (below), the three gates.

## Hook classes (generated, read view and write view, one byte per 16-word granule)

NONE 0, PSG 1 (psg regions), STIC_REGS 2 (stic device behind gate stic-regs: `$0000-$007F` read, `$0000-$003F`, `$4000-$403F`, `$8000-$803F`, `$C000-$C03F` write), GMEM 3 (grom and gram behind gmem), BACKTAB 4 (`stic.backtab`, `$0200-$02EF`, both views), STIC_ALIAS_READ 5 (read view only: `$4020-$402F`, `$8020-$802F`, `$C020-$C02F`, derived from `stic.mode_select_addrs` and cross-checked against the write aliases' base plus the mode-select register offset `$21` taken from the mode_cs effect row). The test side derives the same table from the memmap, register and geometry literals and compares every address under READ, FETCH, ADAR, WRITE, DW and the four other kinds.

## Golden re-bless 2, classified (the named acceptance test `phase 6 does not move any phase 5 timing`)

Regenerated with the documented recipe (`build/release/runner/ivs-run --content-dir build/release/images --update-golden <golden> <scenario>`; the consumer golden from `build/gate/pkg-consumer.txt` behind its preserved `#` header). Classified by a script comparing every line of the 06-05 file with the new file (same line count, key by key):

| Golden | Lines | Changed lines (old prefix to new prefix) | Class | Everything else |
|--------|-------|-------------------------------------------|-------|-----------------|
| `data/golden/diag-cpu-1.digests` | 70 | `digest.instance` 854cf55a to 082ff483; `digest.composite` 38087b1b to eb893b66 | allowed | 68 lines byte-identical; `digest.mem` unchanged (no gate closed during the run) |
| `data/golden/diag-input-pass.digests` | 78 | `digest.instance` 854cf55a to 082ff483; `digest.composite` 38087b1b to eb893b66 | allowed | 76 byte-identical; `digest.mem` unchanged |
| `data/golden/diag-ctrl-psg-pass.digests` | 97 | `digest.mem` f44d0753 to 4d183e24; `digest.instance` fc76d757 to 3970fede; `digest.composite` e2100ba5 to 228ac608 | allowed | 94 byte-identical |
| `data/golden/timeout-budget.digests` | 53 | `digest.instance` 6948b094 to af38dfbb; `digest.composite` ab941412 to 2a6b7898 | allowed | 51 byte-identical; `digest.mem` unchanged |
| `data/expected/pkg-consumer.golden` | 54 | `digest.instance`, `digest.composite` (the diag-input-pass values) and `ctrl.digest.mem`, `ctrl.digest.instance`, `ctrl.digest.composite` (the diag-ctrl-psg-pass values) | allowed | 49 byte-identical |

As predicted by the plan: `digest.mem` changed only for `diag-ctrl-psg-pass` (its last unit ends at 16,752 ticks, after both windows closed), `digest.instance` and `digest.composite` changed for all four goldens (the cursor advances at the first unit end of every run), every other key is identical: `digest.cpu`, `digest.images`, `digest.audio`, `digest.input`, `digest.psglog`, every `step.*`, `slot.*`, `cpu.*`, `now_ticks`, stop-reason and `output.*` line. `digest.cpu` and `digest.images` equal the pins of `data/expected/digest-v2-literals.ids` (checked for diag-cpu-1: `8c6a8aa3...` and `69d3b715...`). The consumer `digest.*` lines equal `diag-input-pass.digests` and its `ctrl.digest.*` lines equal `diag-ctrl-psg-pass.digests`; video is still not observed (no frame is delivered until plan 06-09), runner and consumer golden show the same observed set, and the consumer calls no new function. No second re-bless was needed in Tasks 3 and 4: the guests of the goldens touch no closed STIC, GROM or GRAM address, so the blocked counters stayed 0 (the golden step passed unchanged).

Closure sweep (Task 2 step 4): every existing suite was run with the gates closing at 8,000 and 15,120 ticks. The only expectation that moved is `machine.digest.stepped-literals` (composite and INSTANCE, `md_stepped[0]` and `[4]`; the cursor): CPU, MEM and IMAGES of the stepped run are byte for byte the 06-05 values. `tests/machine/machine_load.c`, `tests/psg/psg_reg.c` needed nothing; `tests/run/run_observe.c` changed comments only (`ro_peek_undriven` still passes: `$0000` reads FFFF because the register's read mask is 0, and the 64-word window at `$0070` is the tail of the read alias, all read-mask-0 registers, then the undriven gap).

## Tests and faults

- Suite `stic`, 45 ids: `stic.events.*` (5), `stic.reset.cursor`, `stic.slice.state-equal-at-every-unit-boundary`, `stic.alias.*` (8), `stic.window.*` (2), `stic.blocked.*` (6), `stic.enable.seq.*` (6), `stic.mode.*` (3), `stic.table.matches-literal`, `stic.reg.*` (6), `stic.hook.completeness`, `stic.profile.window-open` and `.window-close`, `stic.judge.*` (3). Dev plain run 0.09 to 0.10 s (target 0.8 s after this plan), release below the clock resolution.
- Faults, each registered with the commit of its test: `STIC_FRAME_BY_MODULO` and `STIC_WINDOW_OFF_BY_ONE` (`stic_events.c`), `STIC_CATCHUP_LAGS_UNIT` (`machine_run.c`), `STIC_ALIAS_NOT_HOOKED` and `STIC_MODE_SWITCH_OUTSIDE_WINDOW` (`machine_bus.c`); every must-fail id is the one the plan names. Collateral prefixes: FRAME_BY_MODULO also fails `stic.events.idempotent`; WINDOW_OFF_BY_ONE also fails `stic.events.*`, `stic.reset.`, `stic.slice.`, `stic.window.`, `stic.profile.`; STIC_STATE_NOT_RESET_ON_POWER_ON (a 06-05 fault) now also fails `machine.reset.equivalence` because the cursor moves at every unit end; MEM_MIRROR_MASK (memmap) halves the 64-word STIC index and fails the STIC rows.
- Hosted-hazard care: the guest builder sets the cartridge length and the EXEC image in their own statements (no out-parameter read in the argument list that sets it), no uninitialised read after a `goto done`, no path in a tracked file.

## Cost: checkpoint A

Recorded in `.planning/CI-EFFICIENCY.md` (section `Phase 6 checkpoint A (plan 06-06)`): cumulative adds faults 193 to 202 (+9), controls 328 to 331 (+3, the plan quotes 316 as its base; the gate line at plan 06-02 reads 328), test ids 3778 to 3984 (+206). Local gate runs 73 and 76 s (about 228 cpu-s) on the quiet laptop, 139 s and 107 s (dash) while it was loaded; `c-faults` 19 to 20 s for 202 faults. Projection `231 + 2.0 x (4.50 + 4.25 + 0.21 + 0 + 0.70) = 250.3` against the 280 trigger: **`checkpointA: no-lever`**.

## Deviations from Plan

**1. [Rule 1 - defect in the plan text] ADAR is hooked as a read.** The plan hooks read, fetch, write and DW and says every other kind is NONE. The operand read of a direct-addressed instruction is kind ADAR (cp1610.c `ivs_dir_read`), so `MVI $0021, R0` would never reach the STIC; the first run of the mode tests showed it (five failures with direct addresses, none with register-indirect ones). `ivs_memsys_hook_class_internal` and the wrapper treat ADAR as READ; `stic.hook.completeness` covers it. T-06-07 is the threat this closes.

**2. [Rule 3 - files not in the plan list]** `data/expected/faults/memmap.ids` (MEM_MIRROR_MASK allowed prefixes cover the STIC rows now that stic is a real 64-word device), `data/expected/faults/machine.ids` (STIC_STATE_NOT_RESET_ON_POWER_ON allows `machine.reset.equivalence`), `tests/machine/machine_digest.c` (`md_stepped` composite and INSTANCE re-recorded) and `data/expected/tests/memmap.ids` (renamed ids below) were edited; each failed the gate once and was fixed in the commit that caused it.

**3. [Interpretation] The memmap literal rows and their exception marks.** The dump probe merges adjacent equal tuples, so the literal rows are the merged ones (for example one row `R 0020-003F` for the strobe, reserved and write-only registers). `x=reads-zero` marks the read rows whose read mask is 0 and the write rows of the stored write-only registers, `x=strobe` the write rows that store nothing (`$0020-$0027`, `$002D-$002F`, `$0033-$003F` and their aliases); the plan names `$0020` and `$0021` literally in `mr_is_exception`, the test names every STIC write-view address (mask test) because the width probe of a write row observes the register storage. `mr_store_kept` accepts a store that leaves a register unchanged (a 2-bit register stores 1234 as 0).

**4. [Rule 3] Two memmap test ids renamed** (`memmap.width.stub-reads-ffff` to `memmap.width.stic14-registers`, `memmap.describe.stub-vs-undriven` to `memmap.describe.stic-vs-undriven`) and the `memmap.row.*` ids regenerated from the split literal (139 ids).

**5. [Rule 3 - hardware constraint] Guests reach addresses above `$3FF` through R3.** A direct address is a 10-bit decle, so the guest sets R3 with `SDBD; MVII #addr, R3` and uses `MVO@` or `MVI@` (5 words per access, every guest at most 24 words; decles derived from `sdbd.json` and `indirect.json`).

**6. [Plan order] `stic_profile.c` was created in Task 4** (its tests are the Task 4 perturbation cases), not in Task 2; Task 2's module-level perturbation lives in `stic_events.c`.

**7. [Plan wording] The accessor `ivs_memsys_stic_reg_read_internal`** was written in Task 3 and removed in Task 5: the plan says dedicated pure accessors are not added in this plan, and nothing used it.

## Decisions Made

- The stamp of an access is `unit_start + offset x ticks per CPU cycle + access_strobe_ticks` (0 in the profile today), the same expression for every hooked class; the PSG hook is untouched and runs first.
- Blocked counting follows the access kind (READ, FETCH, ADAR count `blocked_reads`; WRITE and DW count `blocked_writes`) and is the same for the register gate and the graphics-memory gate; BACKTAB accesses catch up only.

## Open Items

- VID-01 is not marked complete in `REQUIREMENTS.md`: the gate evidence of this plan covers register access, the handshake, the windows and the blocked policy, but plans 06-13 and 06-14 list VID-01 too, so the closing plan decides. VID-06 (inspection call) is still open (peek stays pure here, the public inspection surface is a later plan).
- `ivs_info` v4, report v3 and the env slack word are still 0 (carried from 06-03, 06-04, 06-05; plan 06-09). The 06-01 second-read records, `run.refuse.flags-frame-reserved` inversion (06-09), domain 16 digest value init (06-10), the Phase 4 and 5 deferrals and `sh tools/hygiene.sh --history` failing on earlier planning commits are carried unchanged.
- Policy rows: the blocked read of `$0021` and of the alias addresses counts as a blocked read (C84), the windows are unknown-default-chosen (C5, C94); the tests assert the outcome, not a number.
- The plan text quotes the controls base as 316; the gate line at plan 06-02 reads 328 (recorded in the checkpoint section).

## Final gate

```
sh tools/gate.sh:       gate: PASS steps=36/36 controls=331/331 ... faults=202/202 ... profile-keys=143 ... tests-expected=3984 tests-executed=3982 tests-skipped=2 tests-missing=0   (gate-time total=73)
dash tools/gate.sh:     the same summary line (identical bytes)                                                                                                    (gate-time total=69)
sh tools/gate-linux.sh: GATE_LINUX_PLATFORM= (native arm64 container), exit 0, last line of build/gate-linux.log starts gate: PASS, cc=GNU (gcc 13.3.0, ncpu=8), steps=36/36 controls=331/331 faults=202/202 profile-keys=143 tests-missing=0   (gate-time total=413)
```

## Known Stubs

None. The sink parameter of `ivs_stic_catch_up` is NULL and unused by design until frames are delivered (documented in `stic_events.c` and `stic.h`).

## Threat Flags

None new. T-06-07 is mitigated: generated hook tables compared with the literals over all 65,536 addresses and every access kind (this found the ADAR hole before it shipped), catch-up at every unit end and before each hooked access, the per-unit-boundary slicing test, faults `STIC_ALIAS_NOT_HOOKED` and `STIC_CATCHUP_LAGS_UNIT`. T-06-08 is mitigated: a blocked read returns the undriven fill through the memory system's closed-gate path, effects only with the gate open, `STIC_MODE_SWITCH_OUTSIDE_WINDOW` and `stic.judge.blocked-write-applied`. T-06-SC: no installs.

## Self-Check: PASSED

Created files exist (`core/src/stic/stic_events.c`, `core/src/stic/stic_regs.c`, `cmake/IvsSticProbes.cmake`, the nine files of `tests/stic`, `data/expected/tests/stic.ids`, `data/expected/faults/stic.ids`, `tools/check/controls/memmap-exception-unknown/`); commits `c9f5125`, `8c867a7`, `9cce38e`, `ca7dfd1`, `625c4a5` are ancestors of HEAD; the final gate lines above are from the tree of `625c4a5` plus this SUMMARY.
