---
phase: 05-hand-controllers-and-psg-sound
plan: 08
subsystem: core/psg, core/machine, core/memmap, cmake, tests/psg, tests/run, data/profiles, data/api
tags: [psg-write-log, domain-19-chain, published-bounds, safety-net, replay, bus-trace, fuzz, checkpoint-2, D-13, D-14, D-16, D-18, D-19, A-01, A-03, A-12, AUD-02]
status: complete

requires:
  - phase: 05-hand-controllers-and-psg-sound
    provides: "plan 05-01 oracle commit 14a1904; plan 05-02 request and report v2 (psg_log, psg_log_capacity, psg_log_bytes_written, psg_log_first_index), RUN_PSGLOG_NULL, RUN_PSGLOG_OVERLAP, RUN_PSGLOG_SMALL, IVS_PSGLOG_SRC_*; plan 05-03 PSGLOG rows; plan 05-06 derived constants U = 468, g = 28, F = 59,736 and the per-call sink; plan 05-07 the shared write path (HEAD at start 78afac4)"
provides:
  - "core/src/psg/psg_log.c: the 16-byte record written byte by byte big-endian and the domain-19 chain 'IVSR' 19 chain index 16 record, updated with or without a log buffer (counting log sink)"
  - "the machine bus wrapper appends every guest write to $01F0..$01FF after the masked store: tick = unit start + offset x ticks per cycle + psg.write_strobe_ticks, raw 16-bit value, address low nibble, source 1 (WRITE) or 2 (DW); the run call wires psg_log, reports psg_log_bytes_written and psg_log_first_index and ends INTERNAL at the last whole boundary if fewer than 16 bytes remain before a unit"
  - "psg.min_write_gap_ticks 28 and psg.log_records_per_frame 2152 as profile data (inference, C53), generator-checked: W_log = ceil(F/g) + ceil(U/g) + 1 = 2134 + 17 + 1; ivs_info and the env publish 16-byte records, 2152 per frame slot, 34,432 bytes"
  - "ivs_machine_step_unit_trace_internal (seam variant with the CPU access trace) and ivs_ms_exec_words / ivs_ms_powered_words (stand-in programs from words)"
  - "psg suite 227 ids per preset (was 191): 14 psg.log, 3 psg.bounds, 5 psg.buffers, 2 psg.slice, 9 psg.fuzz, psg.iso.interleaved-instances, 2 psg.reset; 6 named faults (186 total)"
  - "Phase 5 checkpoint 2 in .planning/CI-EFFICIENCY.md: checkpoint-2: no-lever"
affects: [05-09, 05-10, 05-11, 05-12]

actuals:
  tokens: 40000
  tasks: 5
  commits: 6
plan_head_before: 78afac441f72bc637570f7b0c8a9b0655c509529
plan_head_after: 09bfde9a4a6a67fc2f954bcfdb49e7523feb0e4a

tech-stack:
  added: []
  patterns:
    - "One sink per call carries both channels: audio (buf, cap, written) and write log (log, log_cap, log_written, log_records, log_overflow); a NULL buffer runs the same code against the counting sink and changes no machine state"
    - "The log record is hand-framed by the test with literals made at authoring time (printf octal escapes, cmake -E sha256sum, shasum -a 256 as the second hash), the stamp checked against the observed unit start of a seam-stepped twin and against the CPU's own access trace, the log checked complete by replay into a fresh PSG"
    - "A fault that must differ only under interleaving keeps a file-scope static that an empty row resynchronises, so sequential single-instance use is identical and two instances in turn are not"
    - "Seeded random write programs from the in-repo xorshift32 (psg_util.h pu_prog_random) against the one-unit-per-call reference at every slice boundary"

key-files:
  created:
    - core/src/psg/psg_log.c
    - tests/psg/psg_log.c
    - tests/psg/psg_fuzz.c
    - tests/psg/psg_iso.c
  modified:
    - cmake/IvsMemmapTable.cmake, cmake/IvsPsgTable.cmake, core/CMakeLists.txt
    - core/include/ivs/ivs_machine.h, core/include/ivs/ivs_run.h
    - core/src/machine/machine.c, machine.h, machine_bound.c, machine_bus.c, machine_run.c
    - core/src/memmap/memsys.h, core/src/psg/psg.h, psg_sample.c, psg_state.c
    - data/api/api-effects.json, data/profiles/intv-2609-ntsc-family-p1.json
    - data/expected/profile-fields.ids, data/expected/tests/psg.ids, data/expected/faults/psg.ids
    - tests/CMakeLists.txt, tests/machine/machine_cases.h, machine_standin.c
    - tests/psg/psg_bounds.c, psg_buffers.c, psg_cases.h, psg_main.c, psg_slice.c, psg_util.h
    - tests/run/run_info.c, run_struct.c
    - .planning/CI-EFFICIENCY.md

key-decisions:
  - "The write is appended after the masked store and after ivs_psg_after_write, from the same stamp the catch-up used; value is the raw a->data, addr is a->addr & 0xF, source follows the access kind (WRITE 1, DW 2)"
  - "Domain-19 index is the record count before the record (0 for the first), as the audio chain uses its change count; the domain digest preimage is 'IVSD' 19 be16 1 be32 32 chain be64 count"
  - "Log delivery is active when the layout publishes a log (psg_log_bytes_per_frame nonzero) and the request carries a buffer; a synthetic layout without a log keeps its Phase 4 meaning"
  - "ivs_info publishes psg_log_records_per_frame from the env (not from the profile directly), so the fault PSG_LOG_RECORDS_PER_FRAME_CUT halves what a caller sizes its buffer from and the safety net fires"
  - "The registry row carries psg_min_write_gap_ticks so the bounds test compares the exhaustive probe with the published field; the core never reads it"
  - "Long write-log test runs always request an audio buffer: without one the named fault AUDIO_STATE_DEPENDS_ON_BUFFER redoes every sample of the call after every unit (quadratic) and the fault run timed out at 120 s"

duration: about 45 min of edits plus about a dozen gate runs (first commit 05:53, last 06:24)
completed: 2026-11-12
---

# Phase 5 Plan 08: Write log, bounds from the code, checkpoint 2 Summary

**Every guest write to the PSG block is now one 16-byte cycle-stamped record in a caller-owned third buffer and one step of the domain-19 chain, with the capacity bound W_log = 2,152 records (34,432 bytes per frame slot) derived from the CPU model, published as tier-tagged profile data and re-proved by an exhaustive probe; replay of the log into a fresh PSG reproduces the samples and the AUDIO row, the log equals the CPU's own access trace, and a sliced run, a seeded random run and two interleaved instances give the one-call log and chains; the gate ends `steps=35/35 controls=303/303 faults=186/186` under `sh`, `dash` and the Ubuntu container, and checkpoint 2 reads `no-lever`.**

Oracle commit (05-01): `14a1904` (`data/expected/psg-log-source.ids` of plan 05-02 and the oracle files of 05-01 were not edited). Plan start `78afac4`.

## What was built

- Commit `7eea9fa` (Tasks 1 and the profile and layout half of Task 2): `psg_log.c`, the sink's log channel, the bus-wrapper append, the run-call wiring (report fields, counting log sink, INTERNAL safety net), `ivs_machine_step_unit_trace_internal`, the two profile fields, the generator checks and the layout (`ivs_info`, the env), the tracer `psg.log.framing`.
- Commit `f253eeb` (Task 2): `psg.bounds.g-and-u`, `log-capacity-formula`, `log-bound-at-g`; `psg.buffers.worst-case-writer`, `log-too-small`, `log-not-requested`, `log-sentinel-tail`, `log-safety-net`.
- Commit `8e164be` (Task 3): the rest of `psg.log.*`, `psg.slice.log-split` and `log-first-index`, `psg.fuzz.seed01..08` and `cases-covered`, `psg.iso.interleaved-instances`, `psg.reset.log-rows-cleared` and `chains-cleared`; the allowed prefixes of five existing faults.
- Commit `447887a` (Task 4): the six fault sites and their registration.
- Commit `81cb32f`: gcc 13 `-Wmaybe-uninitialized` fix (see Deviations).
- Commit `09bfde9` (Task 5): the `Phase 5 checkpoint 2` section of `.planning/CI-EFFICIENCY.md`.

## Bound arithmetic (published and derived)

F = 262 x 57 x 4 = 59,736 ticks, g = 28 ticks (instruction to instruction 36), U = 468 ticks. `ceil(59736 / 28)` = 2,134, `ceil(468 / 28)` = 17, so W_log = 2,134 + 17 + 1 = **2,152 records** = 16 x 2,152 = **34,432 bytes per frame slot**; a run of 1 << 22 ticks has `(1 << 22) / 59736 + 1` = 71 slots and needs 71 x 34,432 = **2,444,672 bytes** (asserted by `psg.bounds.log-capacity-formula` through `ivs_info` and through a run call: one byte less is `RUN_PSGLOG_SMALL`, the exact capacity runs). `IvsPsgTable.cmake` refuses a profile whose `psg.log_records_per_frame` differs from the formula over `scanlines_per_frame`, `cpu_cycles_per_scanline`, `ticks_per_cpu_cycle`, `psg.min_write_gap_ticks` and `psg.max_unit_ticks`, and refuses a gap that is not a whole number of CPU cycles. `psg.bounds.g-and-u` re-proves g = 28 and U = 468 over every decle, SDBD off and on, waits 0 and 35, interrupt-entry units included, against the published fields (28, 468, 2,152).

## The worst-case writer (hand-derived)

A project-authored program of two `MVII` and 3,800 straight-line `MVO@` writes: the first `MVO@` unit starts at 21 CPU cycles = tick 84 (the reset entry 5 cycles and two 8-cycle `MVII`), its write is at cycle 4, so record i has tick `100 + 36 i`. A run to tick 130,000 starts units while the time is below the target: `floor((130,000 - 84 - 1) / 36) + 1` = **3,609 records**, all equal to the hand-derived bytes, no `IVS_STOP_INTERNAL`. The densest window of one frame slot holds `ceil(59,736 / 36)` = **1,660** records, below W_log. With W_log halved (synthetic layout, 1,076 records per slot) the run stops INTERNAL after exactly the records that fit, at the whole boundary `84 + 36 x records`, and the state equals a twin run to that boundary.

## Test ids (psg suite 227 per preset; was 191)

| Group | Ids |
|---|---|
| psg.log (14) | framing, chain-two-records, byte-order, reserved-zero, source-values, value-raw-16bit, addr-low-nibble, no-reads-logged, no-synthetic-power-on-records, strictly-increasing, window, vs-cpu-trace, replay-equals-run, effect-boundary |
| psg.bounds (3 new) | g-and-u, log-capacity-formula, log-bound-at-g |
| psg.buffers (5 new) | worst-case-writer, log-too-small, log-not-requested, log-sentinel-tail, log-safety-net |
| psg.slice (2 new) | log-split, log-first-index |
| psg.fuzz (9) | seed01 .. seed08, cases-covered |
| psg.iso, psg.reset (3) | iso.interleaved-instances, reset.log-rows-cleared, reset.chains-cleared |

Changed expectations in the run suite (named): `run.info.v3-fields` now asserts `psg_log_record_bytes` 16 and `psg_log_records_per_frame` 2,152 (was 0 and 0); `run.struct.report-v2-fields` keeps its zero log fields for program A (no buffer, no PSG write) and gained a case with stand-in program C: a first call that ends after the write reports `psg_log_bytes_written` 16 and `psg_log_first_index` 0, the next call 0 and 1. The recorded values are literals (the plan 05-02 open item).

## Named faults (186 = 180 + 6)

| Fault | Site | Must fail | Also fails (allowed prefixes) |
|---|---|---|---|
| `PSG_REG_WRITE_NOT_LOGGED` | psg_log.c | `psg.log.replay-equals-run` | psg.log., buffers., slice., fuzz., reset., bounds., iso. |
| `PSG_LOG_TICK_UNIT_START` | machine_bus.c | `psg.log.window` | psg.log., buffers., slice., fuzz. |
| `PSG_LOG_TICK_SLICE_RELATIVE` | machine_bus.c | `psg.slice.log-split` | psg.log., buffers., slice., fuzz. |
| `PSG_WRITE_EFFECT_LATE_ONE_STEP` | psg_sample.c | `psg.log.effect-boundary` | log., tone., sample., chain., noise., env., amp., ampenv., profile., judge. |
| `PSG_STATIC_CARRY` | psg_log.c | `psg.iso.interleaved-instances` | iso., fuzz., slice. |
| `PSG_LOG_RECORDS_PER_FRAME_CUT` | machine.c | `psg.buffers.worst-case-writer` | buffers., bounds. |

The prefixes are the collateral failures read from the fault executables (each fault listed with its failing ids). Five existing psg faults gained `psg.log.` (and `psg.fuzz.` for `PSG_SAMPLE_PHASE_RESET_PER_CALL`).

## Evidence numbers

- Counts: `steps=35/35 controls=303/303 faults=186/186 tests-expected=3610 tests-executed=3608 tests-skipped=2 tests-missing=0` (05-07 end: `tests-expected=3538`, `faults=180/180`). `git diff --stat 78afac4 HEAD -- data/golden` is empty: the goldens are unchanged byte for byte (the runner does not request the log or report domain 19 until plan 05-09).
- Plain-run times (median of 11, laptop): `psg` dev (ASan and UBSan) 0.344 s, release 0.120 s (plan targets 1.5 s and 0.3 s; 05-07: 0.185 and 0.054 s: the exhaustive write-gap probe, about 0.1 s dev, and the fuzz seeds, about 0.05 s dev, are the new cost); `run` dev 0.565 s, release 0.306 s.
- `psg.ids`: 250 lines (23 comment lines, 227 ids); `psg.log.*` 14.
- Gate cost (local, loaded laptop): three `sh` runs `gate-time total=` 37, 35, 37 with `gate-cpu` 119.0, 107.2, 112.9; `dash` 41 s, 135.3 cpu-s; container 151 s (emulated, correctness only). Per fault run 0.137 cpu-s.

## Checkpoint 2 (V-16), copied from `.planning/CI-EFFICIENCY.md`

- Hosted last-5 medians (rows up to plan 05-07): `cpu_s` 252, `gate_s` 81. Adds still to come: this plan 3.4 cpu-s, the rest of the phase 5.4, total 8.8: projected last-5 `cpu_s` median **260.8** (262.6 with a 20% margin) against the 280 trigger (budget 260); projected `gate_s` **84** against the 90 trigger (budget 75; the median is already 81).
- Local unit costs: 0.137 cpu-s per fault run (about 0.5 hosted with the build), 1.5 ms per psg id in dev, 0.5 ms in release.
- `checkpoint-2: no-lever`. No lever pass is required before plan 05-09. Watch from the next hosted rows: a last-5 `cpu_s` median at or above 280, or `gate_s` at or above 90 (the wall margin is about 6 s), fires the rule; the first lever then is `-g0` on the fault targets (about 5 hosted cpu-s), as 05-L1 recorded.

## Deviations from Plan

**1. [Rule 3 - Blocking] Tasks 1 and the layout half of Task 2 are one commit.** The tracer needs a log capacity, which needs the profile fields, the generator check, the env and `ivs_info`; they landed together with the tracer (`7eea9fa`, gate green). Task 2's second commit holds the bounds and buffer tests. The pending-ids file `data/psg/fields-pending.ids` had no line to delete (it ended empty at plan 05-07).

**2. [Rule 3 - Blocking] The fault sites travel with their registration in Task 4.** `fault-orphan-site` fails any `IVS_FAULT_*` site that is not registered, so the six sites were added with `data/expected/faults/psg.ids` in the Task 4 commit; the tests that detect them were in Tasks 1 to 3 and the allowed prefixes of five existing faults were widened in Task 3 so every commit is gate green.

**3. [Rule 1 - Plan precision] The writer's lower bound is `floor((T - 84 - 1) / 36) + 1`, not `floor(T / 36) - 1`.** The plan's bound ignores the 84-tick prefix before the first `MVO@` (reset entry and two `MVII`); for T = 130,000 it reads 3,610, above the 3,609 records a run can hold. The test asserts the exact hand-derived record list (tick `100 + 36 i`) and the exact count instead, which is stronger.

**4. [Rule 2 - Missing critical functionality] The registry row carries `psg_min_write_gap_ticks`** (not in the plan's file list: `core/src/memmap/memsys.h`, `cmake/IvsMemmapTable.cmake`), so `psg.bounds.g-and-u` compares the probe with the published field without a second JSON reader. The core never reads it.

**5. [Rule 1] `psg.log.effect-boundary` is a module-level test and the fault `PSG_WRITE_EFFECT_LATE_ONE_STEP` sits in `ivs_psg_write`.** The wrapper's own catch-up (`machine_bus.c`) is covered by `psg.sample.wrapper-stamp`, `psg.log.window` and `psg.log.replay-equals-run` (a replay through `ivs_psg_write` against the wrapper's run, which also fails under the fault). The fault's collateral is therefore every module-level vector that writes through `ivs_psg_write`.

**6. [Rule 1 - Hosted-only hazard] gcc 13 `-Wmaybe-uninitialized` in `psg.bounds.g-and-u`.** The first container gate failed the build (gcc flagged `g`, read on a path that follows a `goto done`); `g` and `u` are both initialised (`81cb32f`), and the container gate then passed. Clang did not warn.

**7. [Rule 1] Long runs request an audio buffer.** With no audio buffer the named fault `AUDIO_STATE_DEPENDS_ON_BUFFER` undoes each unit's catch-up, so a 4 million tick run of `psg.bounds.log-capacity-formula` (and the 130,000-tick writer runs) redid every sample of the call after every unit and the fault run hit the 120 s timeout. `pu_run_to_log` now always supplies an audio buffer (a scratch one when the caller passes NULL), and the capacity-formula and safety-net runs pass one; the not-requested log tests keep their short, no-audio runs on purpose.

**8. [Other] Extra ids and seams.** `psg.log.chain-two-records` (literal two-record chain and order sensitivity; the plan's `psg.log.*` list had 13 names and its acceptance asked for 14), `ivs_machine_step_unit_trace_internal` (the plan's seam variant with the access trace, built as a plain extension of the sink seam), `ivs_ms_exec_words` and `ivs_ms_powered_words`, `pu_prog_random`, `pu_prog_writer`, `pu_prog_logmix` in `psg_util.h`. The PSG state needs no change.

## Decisions Made

- The record index of the chain is the count before the record, like the audio chain's change count; the domain digest of an empty log equals that of a fresh instance (asserted).
- The synthetic layouts of the Phase 4 and 05-02 tests (`bf_log_env`, 16 bytes per frame) keep working unchanged: the guests there never write the PSG, and the safety net only applies once a unit would not fit a requested buffer.
- `psg.log.source-values` drives the wrapper with synthetic WRITE and DW accesses; source 2 occurs in a machine run only when a device raises an interrupt (Phase 6).

## Known Stubs

None.

## Open Items

- **AUD-02 stays Pending in REQUIREMENTS.md.** The core now emits the log and the samples with proofs of completeness, stamps, slicing and bounds; the runner and the consumer do not yet request the log or report domain 19 (05-09 onward), the reference PCM location (05-10) and the end-to-end evidence (05-11) and the registry edit (05-12) close it.
- `psg.min_write_gap_ticks` 28 is a Phase 5 inference whose gap with an interrupt-entry unit was probed with a synthetic state and the interrupt line asserted; no device raises an interrupt until Phase 6, where `psg.bounds.g-and-u` and `psg.bounds.memsys-bus-never-waits` must be re-read first.
- The reset entry takes 5 CPU cycles (observed unit start 52 for the first `MVII`-then-`MVO` pair, 84 for the writer); the literals depend on the CPU timing already pinned by the run and machine suites, not on a new claim.
- Carried unchanged: the 05-04 unobtained AY-3-8910 reprint, D10 and D11 items, Phase 4 items IN-04, IN-06, IN-09 to IN-11, the C73 tag wording of 05-05, `psg.mix.truth-table` polarity invariance (05-06), the noise first-bits and envelope phase conventions (05-07).
- Hosted cost of this plan is unmeasured until the next hosted rows; see checkpoint 2 above.

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=35/35 controls=303/303 ... faults=186/186 ... tests-expected=3610 tests-executed=3608 tests-skipped=2 tests-missing=0`, 0 FAIL lines, `gate-time total=35` (run on the final tree).
- `dash tools/gate.sh`: the same line, 0 FAIL lines, `gate-time total=35`.
- `sh tools/gate-linux.sh`: Ubuntu 24.04 container (gcc 13.3, cmake 3.28, dash): `gate: PASS steps=35/35 controls=303/303 ... faults=186/186 ... tests-expected=3610 tests-executed=3608 tests-skipped=2 tests-missing=0`, `cc=GNU` (13.3.0), 0 FAIL lines, `gate-time total=151` (emulated amd64, 8 cpus; a correctness check only), exit status 0, `build/gate-linux.log` ends with the `gate: PASS` line.

## Threat Flags

None new. T-05-20 (silent truncation or loss of log records): mitigated by the capacity check from the derived W_log before the first unit, the INTERNAL safety net, `psg.log.replay-equals-run`, `psg.log.vs-cpu-trace` and the faults `PSG_REG_WRITE_NOT_LOGGED` and `PSG_LOG_RECORDS_PER_FRAME_CUT`. T-05-21 (stamps that go backwards or depend on slicing): `psg.log.window`, `psg.slice.log-split`, `psg.log.strictly-increasing`, the fuzz seeds, and the faults `PSG_LOG_TICK_UNIT_START` and `PSG_LOG_TICK_SLICE_RELATIVE`. T-05-SC: no installs.

## Self-Check: PASSED

- Files exist: core/src/psg/psg_log.c, tests/psg/psg_log.c, tests/psg/psg_fuzz.c, tests/psg/psg_iso.c.
- Commits on HEAD: 7eea9fa, f253eeb, 8e164be, 447887a, 81cb32f, 09bfde9 (`commits: 6` measured from the ledger, `git rev-list --count 78afac4..HEAD`).
