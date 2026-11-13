---
phase: 06-stic-video-first-pass
plan: 02
subsystem: tests/build, ci-efficiency
tags: [lever-pass, g0, fault-targets, normalised-metric, checkpoints, D-21, D-23]
status: complete

requires:
  - phase: 06-stic-video-first-pass
    provides: "plan 06-01 oracles (oracle commit eae8ca5), gate steps=36/36 controls=328/328 faults=193/193"
provides:
  - "ivs_fault_nodebug() in tests/faults.cmake: -g0 on every fault-only target, after the preset's -O1 -g"
  - ".planning/CI-EFFICIENCY.md section Phase 6 start (plan 06-02): the ratio r = cpu_s / build_s for 38 hosted rows, the rule, measured unit costs, before and after, checkpoints CP-A to CP-D with the projection formula and stop rule, the pre-committed lever order, the request to the orchestrator"
affects: [06-06, 06-09, 06-12, 06-14]

actuals:
  tokens: 14000
  tasks: 3
  commits: 2
plan_head_before: d24af6e6ec7f4f745cdcae357d926b236eee5ac7
plan_head_after: 51cc67cc4c5c2909ab40aa03cbc5e1c81acaff65

tech-stack:
  added: []
  patterns:
    - "One helper (ivs_fault_nodebug) called from every function that creates a fault-only target, so a new fault target cannot forget it"
    - "A paired cold-build measurement with separate source copies per variant, because ninja regenerates a build directory whose source cmake file changed and would silently mix the variants"

key-files:
  created: []
  modified:
    - tests/faults.cmake
    - .planning/CI-EFFICIENCY.md

key-decisions:
  - "Apply -g0 on the fault targets although the whole-gate measurement cannot resolve the saving: the paired cold build shows minus 35.6 container cpu-s (14 percent), about 18 hosted, and no evidence changes"
  - "The speed-normalised ratio r = cpu_s / build_s (budget 7.8, revisit above 8.3) is recorded beside the raw triggers, which are copied unchanged; the orchestrator is asked to re-baseline"

duration: about 2 hours, almost all of it measurement on a loaded laptop (10 whole-gate runs, 8 cold fault builds)
completed: 2026-11-12
---

# Phase 6 Plan 02: lever pass and the normalised metric Summary

**`-g0` is applied to the fault-only build targets (no pristine, runner or release target carries it), measured before and after, and the speed-normalised ratio, the Phase 6 checkpoints and the pre-committed lever order are on record before the first fault of the phase lands. The gate ends `steps=36/36 controls=328/328 faults=193/193` in `sh`, `dash` and the Ubuntu container, with a summary line byte-identical to the base runs.**

Plan 06-01 oracle commit (named for the record): `eae8ca5`.

## What was built

- Task 1 (no commit, measurement only): baseline and after figures were taken alternating the base and the patched `tests/faults.cmake` on the same host, so Task 1 and the measurement half of Task 3 were run as one alternating series (three local and two container whole gates per variant), then a paired cold build of `ivs_faults` (the only place the lever acts).
- Task 2 (`d352f07`): `ivs_fault_nodebug(<target>)` in `tests/faults.cmake` (`target_compile_options(... PRIVATE -g0)`), called from `ivs_fault_tu_setup` (plain and owner object libraries), for `ivs_fault_plain_harness`, in `ivs_fault_suite_obj`, and for the fault executables and owner objects of `ivs_fault_exe` and `ivs_fault_exe_core_gen`. Only `tests/faults.cmake` changed (15 insertions).
- Task 3 (`51cc67c`): the CI-EFFICIENCY entry.

Scope proof (`ninja -t commands`): ` -g0` present on `ivs_fault_ADCR_IGNORES_C` (24 compile lines), a runner-owner fault `OUTCOME_GUEST_AS_HARNESS` (36), a sha256 fault `SHA_K_WRONG` (33), a generator fault (29); absent from `ivs_tests_psg` in `build/dev` and `build/release`, absent from the whole `build/release` graph, and absent from every `build/dev` compile command whose output is not a fault object.

## Before and after

| Measure | Before (base) | After (`-g0`) | Reading |
|---------|---------------|---------------|---------|
| local whole gate `gate-cpu` (3 runs, load 15 to 21) | 223.9, 250.5, 265.0 (mean 246.5) | 262.0, 250.1, 265.0 (mean 259.0) | inside the run to run spread |
| local `gate-time` | 82, 69, 78 | 77, 78, 80 | noise |
| local `c-faults` step | 22, 18, 19 | 19, 17, 19 | noise (the fault runs are unchanged) |
| container whole gate `gate-cpu` (2 runs) | 922.3, 976.6 | 948.5, 896.8 | minus 27 on the means, inside the spread |
| container `gate-time` | 213, 232 | 225, 222 | noise |
| paired cold `ivs_faults` build, Ubuntu gcc 13.3, `-j 8`, 3 rounds, user + sys | 250.4, 274.7, 229.6 (mean 251.6), wall 34, 38, 32 | 229.7, 223.3, 194.9 (mean 216.0), wall 32, 32, 27 | **minus 35.6 cpu-s (14 percent), every pair favours `-g0`**; about 18 hosted at the 0.51 ratio (10 to 26) |
| fault object bytes (container / macOS) | 16.8 MB / 10.7 MB | 5.1 MB / 4.4 MB | |
| paired cold build, macOS Apple clang, 2 rounds | 42.8, 52.0 | 44.5, 47.8 | no measurable difference: the lever helps the gcc build the hosted runner uses |

The whole-gate figures cannot resolve a saving of this size on a loaded host (the same tree moves by about 40 local cpu-s); the 05-L1 estimate (about 10 container cpu-s) extrapolated from dropping `-g` for the whole dev preset, and the paired measurement is about three times larger. The research expected about 5 hosted cpu-s; the measured value is about 18 (range 10 to 26). The saving is recorded as it is and the lever is applied because it changes no evidence.

Gate lines: the `gate:` summary line is byte-identical across all six base and patched local runs (one md5) and all four container runs (one md5); the final tree repeats it. Gate figures per run, load averages and the ratio table of 38 hosted rows are in the CI entry.

## Deviations from Plan

**1. [Rule 3 - blocking] The first paired build measurement mixed the variants.** Found during: Task 3 measurement. Issue: ninja regenerates a build directory whose `tests/faults.cmake` is newer than its `build.ninja`, so two build directories configured from one source tree swapped between variants both ended up patched (the base directory showed ` -g0` after the swap). Fix: the measurement was redone with a separate source copy and build directory per variant (the macOS and the container pairs); the invalid first pairs were discarded. No tracked file is affected.

**2. Task 1 and the measurement half of Task 3 ran as one alternating series.** The plan asks for the baseline on the unmodified tree first and the after runs alternated with it; one series of ten runs swapped the file between runs (`c-build` of each run therefore includes the rebuild of the changed fault objects), which gives the same comparability with fewer builds. The figures for both are in the CI entry.

## Decisions Made

- `-g0` applied (D-21 first lever) with the measured saving, not a trigger, as the reason.
- Ratio `r = cpu_s / build_s` with budget 7.8 and revisit above 8.3 recorded beside the copied raw triggers; the orchestrator owns the budgets and is asked to re-baseline.
- Hosted rows grow `r` from 5.3 to 6.3 (Phase 4) to 6.4 to 7.4 (Phase 5 close): real work growth of about 1 beyond machine speed; the last five rows give median `r` 6.76 and raw medians gate 90 s, cpu 267 (just inside the 280 trigger, at the 90 s one).

## Open Items

- The orchestrator re-baselining request (the `r` arbiter) is made in the CI entry; no budget was changed here.
- Checkpoints CP-A (06-06), CP-B (06-09), CP-C (06-12), CP-D (06-14) are due in later plans; no plan 06-L1 is created by this plan.
- No requirement is delivered by this plan (empty `requirements`); VID-01..04 and VID-06 close in plan 06-14.

## Final gate

```
sh tools/gate.sh:   gate: PASS steps=36/36 controls=328/328 ... faults=193/193 ... tests-expected=3778 tests-executed=3776 tests-skipped=2 tests-missing=0   (gate-time total=52)
dash tools/gate.sh: the same summary line (md5 equal)                                                                                        (gate-time total=54)
sh tools/gate-linux.sh: exit 0, log last line starts gate: PASS, same counts                                                                (gate-time total=220)
```

## Known Stubs

None.

## Threat Flags

None. T-06-03 (`-g0` reaching a pristine or release target) is mitigated by the scope proof above.

## Self-Check: PASSED

- `tests/faults.cmake` (contains `ivs_fault_nodebug`) and `.planning/CI-EFFICIENCY.md` (one closing line `phase6-start: lever-applied`) exist; commits `d352f07` and `51cc67c` are ancestors of HEAD.
