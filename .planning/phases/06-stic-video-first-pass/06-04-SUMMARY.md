---
phase: 06-stic-video-first-pass
plan: 04
subsystem: data/profiles, cmake, core/stic, core/machine, tests/machine
tags: [profile-growth, generator, fail-closed, registry-tie, relation-tests, behaviour-neutral, D-03, D-05, D-07, D-08, D-09, D-12, D-13, D-15, A-11, D-23, VID-01, VID-03]
status: complete

requires:
  - phase: 06-stic-video-first-pass
    provides: "plan 06-01 oracle commit eae8ca5 (register literal, rules, geometry literal, pending lists, allocation table C81..C105); plan 06-03 ABI growth (gate steps=36/36 controls=328/328 faults=194/194 profile-keys=109)"
provides:
  - "33 tier-tagged profile fields (stic.*, video.*, bus.line_sample_lag_cpu_cycles) with conflict ids, resolving evidence and revisit triggers; profile-keys 109 to 142; the 33 owner-06-04 lines deleted from data/stic/fields-pending.ids (only memmap.device.stic14, owner 06-06, remains)"
  - "cmake/IvsSticTable.cmake: fail-closed generator (62 refusal sites) of ivs_stic_params.inc: F = 59,736, every cycle field converted to ticks once, the 16-entry per-frame event table, 64-row register masks and classes, mode-select list, side-effect rows, BACKTAB span, power-on record and the derived video geometry (159 x 96, plane 15,264, slot 15,296)"
  - "core/src/stic/stic_params.{h,c}: ivs_stic_params, ivs_stic_params_count(), ivs_stic_params_get(profile); the single includer of the generated include; fourth generator rule in core/CMakeLists.txt and the line in data/expected/core-generated.ids"
  - "ivs_machine_profile_covered requires index < ivs_stic_params_count(); machine.create.tables-cover-registry covers the fourth table; named fault STIC_TABLE_SHORT"
  - "seven relation tests machine.stic.params.* and two controls video-field-missing and video-pending-stale"
affects: [06-05, 06-06, 06-08, 06-09, 06-10, 06-11, 06-12, 06-13, 06-14]

actuals:
  tokens: 21400
  tasks: 4
  commits: 3
plan_head_before: 84a9b8f6219d9310a6bbebbd5ac099aa7f06b3de
plan_head_after: 7456369c338ac852531652b70488050ab2dca6f0

tech-stack:
  added: []
  patterns:
    - "Generated per-profile parameter table with both forms of every converted quantity (CPU cycles as profiled, master ticks converted once), so a test proves the conversion against the registry's ticks per CPU cycle without a literal copy of a policy number"
    - "Frame event table as data: variable entry count (4 + rows) inside a fixed capacity of 16, ticks counted from the frame start, ascending, generator-checked and test-checked"

key-files:
  created:
    - cmake/IvsSticTable.cmake
    - core/src/stic/stic_params.h, core/src/stic/stic_params.c
    - tests/machine/machine_stic_params.c
    - tools/check/controls/video-field-missing/, tools/check/controls/video-pending-stale/
  modified:
    - data/profiles/intv-2609-ntsc-family-p1.json, data/expected/profile-fields.ids, data/stic/fields-pending.ids, data/stic/tests-pending.ids
    - core/CMakeLists.txt, data/expected/core-generated.ids, core/src/machine/machine.c
    - tests/machine/machine_create.c, machine_cases.h, machine_main.c, tests/CMakeLists.txt
    - data/expected/tests/machine.ids, data/expected/faults/machine.ids, data/expected/controls/video.ids

key-decisions:
  - "Windows, interrupt width and hold are tagged tier4-community (the evidence table of docs/stic/timing-notes.md and rules STIC-R18, R19), not unknown-default-chosen as the older T-11 table had them; first_start, fetch_offset, late_grant, access_strobe, gating, frame_phase, line_sample_lag, power_on, last_card_column_drop and display_off_fill are unknown-default-chosen (11 fields, each with resolving evidence and trigger); pitch, slack and row_lines are inference; blank_value is delegated-decision D-13"
  - "The generator derives the published width from video.last_card_column_px and never reads data/video/geometry.literal.txt: setting the field to 8 generated 160 with no literal edited (checked on a scratch copy)"
  - "A nonzero frame_phase_ticks is accepted (below F) with the defined meaning that the interval before S_0 schedules nothing; the event ticks are relative to the frame start so the table does not move with the phase"

duration: about 4 hours elapsed, most of it five gate runs on a loaded laptop (see Final gate)
completed: 2026-11-12
---

# Phase 6 Plan 04: profile growth and the STIC parameter table Summary

**Every STIC and video policy number is now tier-tagged profile data that a fail-closed generator turns into a perturbable `ivs_stic_params` row (frame 59,736 ticks, windows 8,000 and 15,120, interrupt level 15,164, 13 bus windows at 15,164 + 3,648 i with hold 456, the 16-entry event table, 64 register masks, 159 x 96 geometry), tied to the registry so a profile without a STIC row cannot create a machine. Nothing consumes the table yet: no golden, scenario or consumer line moved. The gate ends `steps=36/36 controls=330/330 faults=195/195 profile-keys=142` under `sh`, `dash` and the Ubuntu gcc 13.3 container.**

Oracle commit this plan follows: `eae8ca5` (06-01 task 5). 06-01's oracle files were read, never edited (the register literal was transcribed into `stic.regmap` by a script that asserts 64 rows; the literal stays the judge). First commit of this plan: `ba6bc47`; the plan started from `84a9b8f` (docs only).

## What was built

- Task 1 (`ba6bc47`): the 33 profile fields (record shape as every field: value, tag, cite, conflict, resolving_evidence, revisit_trigger, decision, note), the 33 paths in `data/expected/profile-fields.ids`, the 33 pending lines deleted in the same commit, the `stic.part` note updated (re-read in plan 06-01, pdf 191 and 192 of the 1980 catalog and 462 and 463 of the 1982 catalog, STIC1A and the PAL AY-3-8900 not modelled), and the controls `video-field-missing` (token `video-rule-field-unregistered`) and `video-pending-stale`. The two tokens already existed in `tools/check/video.cmake`; the overlays use the existing toy tree (a rule naming `stic.toymissing`, a pending line for the toy-profile field `model`), so the toy tree needed no extension.
- Task 2, tracer (`d0e1d89`): `cmake/IvsSticTable.cmake` (PSG precedent, `st_` helpers, `-DROOT` and `-DOUT` only, static const data only), `core/src/stic/stic_params.{h,c}`, the fourth generator rule and the TU in `core/CMakeLists.txt`, the line in `core-generated.ids`. The tracer check was done on the generated include (`reg_window_ticks 8000`, event ticks `15392 + 3648 i`) instead of a throwaway C assertion, then the refusals were exercised on a scratch copy of the profile (below).
- Task 3 (`7456369`): the fourth term in `ivs_machine_profile_covered`, the extended `machine.create.tables-cover-registry` (count, `NULL` at index n, non-NULL per profile), the fault site `IVS_FAULT_STIC_TABLE_SHORT` with its registration line, seven tests, the ids and the deleted pending line in the same commit.
- Task 4: no fix was needed, so no commit; the three gates and the neutrality check are below.

## Generator refusal list (62 `st_fail` sites; each `IvsSticTable: <profile>: <field>: <reason>`)

Exercised on a scratch copy of the profile (25 perturbations, every one refused with its context; the pristine profile and the phase-100 and last-column-8 variants pass): hold below the fetch offset, pitch below hold, window train past the frame (count 14), rows above count, register window above graphics-memory window, graphics-memory window above the first start, interrupt width above the frame, phase at F, last-column pixels 9, blank value below the colors, slack 2, `apply`, `fetch-at-schedule`, `INTRM`, `border` and `right` selectors, a `tier5-emulator-hint` tag, an `unknown-default-chosen` field without evidence, a regmap of 63 rows and with a duplicate address, a mode-select address outside every STIC region, a grid smaller than the surface, `2 x U >= F`.

Also refused: a missing field or record key, a wrong type or range, a cycle value whose product overflows a uint32, F above a uint32, more than 12 rows (event table capacity 16), a class outside the four, an access outside R and W, an effect outside the four, a repeated effect row or mode address, a power-on registers value other than zero, a backtab word count other than card columns times card rows, a `stic_regs` region other than `$0000..$003F`, `stic.bus_bits` other than 14.

## Tests (machine suite, relations only, over every registry profile)

`machine.stic.params.unit-conversion` (ticks = cycles x registry `ticks_per_cpu_cycle`, F = product of the three registry time-base fields), `.event-table` (4 + rows entries, kinds in order, `ROW_FETCH(i)` at `first_start + i x pitch + fetch_offset`, FRAME_END = F, ascending), `.window-order`, `.tiling-within-frame` (last window ends within F, pitch >= hold, each row latch inside its own window), `.geometry-derived` (width, height, stride, plane, slot from the fields, the header equals `IVS_FRAME_HEADER_BYTES`, the slack equals ceil(U / F) with U read from the registry), `.regmap-shape` (64 rows, 32 stored / 8 collision / 2 strobe / 22 reserved, no bit 15 or 14, every mode-select address decodes to a strobe-class register) and `.power-on`. No test compares a window, hold or pitch with its literal. Machine suite: 398 to 405 ids; the dev-preset executable runs in about 1 s. Fault `STIC_TABLE_SHORT` fails `machine.create.tables-cover-registry`; collateral failures are all under the allowed prefix `machine.`, and `machine.api.version` still passes.

## Deviations from Plan

**1. [Rule 3 - interpretation] "The three existing region fields".** The plan does not say which three or what the generator does with them. The generator reads every `memmap.region.stic_*` field: `stic_regs` must be `$0000..$003F` (the 64 rows of `stic.regmap`) and each `stic.mode_select_addrs` entry must lie in one of the STIC regions (the write aliases at `$4000`, `$8000`, `$C000` and the read alias count).

**2. [Rule 3] The struct also carries the cycle forms and the card matrix.** The plan lists tick fields and derived geometry; the relation tests need the profiled cycle values (to prove the conversion) and the card columns, rows, card size, grid and colors (to prove the derived geometry), so these are members too. The event table has a variable `event_count` (4 + rows) inside a fixed capacity of 16.

**3. [Rule 3] Two test source lists, not one.** `tests/CMakeLists.txt` has the machine list twice (the executable and `ivs_fault_suite(machine ...)`), not once as the plan says; the new file is added to both.

**4. [Plan order] The fault site was added in Task 3**, with its test and registration, as the plan says; Task 2 shipped the getter without it.

**5. [Rule 3] Tracer check without a throwaway C assertion.** The generated include was inspected (`reg_window_ticks 8000`) and the refusals were run on scratch copies; the seven tests of Task 3 assert the relations permanently.

**6. Tags.** The plan names no tag for the windows, interrupt width and hold; they follow the evidence table of `docs/stic/timing-notes.md` and the rules (tier4-community) rather than the older discuss table (unknown-default-chosen). `stic.video_slack_frames` has conflict null (a project rule D-15, decision `D-15`), `video.blank_value` is `delegated-decision` (`D-13`) with conflict C99. `unknown-fields` rose from 25 to 36 (11 fields).

## Decisions Made

- The registry profile satisfies `bus_rows == video_card_rows`; the event-table test pins that relation (one fetch window per card row), the generator does not require it.
- `ivs_stic_params_get` has no NULL guard in any caller (assert by construction): a missing row is refused at create by the coverage term.

## Open Items

- The generator has no `-DMUTATE` hook and there is no `GEN_STIC_MUTATION` fault (the plan says none in this plan); the refusal `bus.release_latency x ticks_per_cpu_cycle <= psg.max_unit_ticks` of 06-DISCUSS-timing T-11 and 06-PATTERNS B1 is not in this plan's list and was not added (8 ticks against 468 today).
- `ivs_info` v4, report v3 and the env slack word are still 0 (06-03 left them to "plans 06-04, 06-06 and 06-09"; this plan is table-only and touches no run or info code). The values now exist in `ivs_stic_params` (`video_slot_bytes`, `video_slack_frames`, `frame_phase_ticks` ...); the plan that sets the env and info words (06-09) replaces the "0" assertions in `run.info.v4-fields`, `run.info.layout-real` and `run.struct.report-v3-caller`.
- Observation on the 06-01 oracle, nothing edited: `data/video/geometry.literal.txt` tags `last-column-clip` as tier4-community while C101 and the profile field are unknown-default-chosen (the plan says the latter; the field is tagged that way). Not a contradiction of any value.
- The profile still carries the note "never derived by modulo" on `scanlines_per_frame` and `cpu_cycles_per_scanline`; it is not contradicted by the corrected time-base text and was left.
- Requirements VID-01 and VID-03 are not closed (no device, renderer or frame exists yet); they stay Pending.
- Carried: the 06-01 second-read records, `run.refuse.flags-frame-reserved` inversion (06-09), domain 16 digest bytes (06-10), the Phase 4 and 5 deferrals, and `sh tools/hygiene.sh --history` failing on earlier planning commits.

## Final gate

```
sh tools/gate.sh:       gate: PASS steps=36/36 controls=330/330 ... faults=195/195 ... profile-keys=142 ... unknown-fields=36 ... tests-expected=3806 tests-executed=3804 tests-skipped=2 tests-missing=0   (gate-time total=127; 219 on the first run, which rebuilt every fault target against the new profile)
dash tools/gate.sh:     the same summary line (identical bytes)                                                                                                   (gate-time total=125)
sh tools/gate-linux.sh: exit 0, last line of build/gate-linux.log starts gate: PASS, cc=GNU (gcc 13.3.0, cmake 3.28.3, x86_64), steps=36/36 controls=330/330 faults=195/195 profile-keys=142 tests-missing=0   (gate-time total=318, 7 min 38 s wall)
```

Suite times (dev preset, loaded laptop): machine suite about 1 s; the full `sh` gate 125 to 127 s once the fault targets are built, which is within the hosted budgets of `.planning/CI-EFFICIENCY.md` (this plan adds one TU, one fault target and no workflow, matrix or cache change). Neutrality: `git status --short -- data/golden data/scenarios data/expected/pkg-consumer.golden` is empty and `git diff 84a9b8f..HEAD` over those paths prints nothing; the pkg-install step's consumer-versus-runner comparison passed with video still none.

## Known Stubs

None. The table is data; nothing consumes it by design (documented in the header of `core/src/stic/stic_params.h`).

## Threat Flags

None new. T-06-05 (a profile producing overlapping windows, unordered events or an overflowing conversion) is mitigated: the refusal list above, the seven relation tests, the `profile` and `video` steps and their controls, and `STIC_TABLE_SHORT` for the registry tie. T-06-SC: no installs.

## Self-Check: PASSED

- Files exist: `cmake/IvsSticTable.cmake`, `core/src/stic/stic_params.h`, `core/src/stic/stic_params.c`, `tests/machine/machine_stic_params.c`, `tools/check/controls/video-field-missing/EXPECT`, `tools/check/controls/video-pending-stale/EXPECT`.
- `grep -c IVS_FAULT_STIC_TABLE_SHORT core/src/stic/stic_params.c` is 1, `grep -c '^STIC_TABLE_SHORT' data/expected/faults/machine.ids` is 1, `grep -c '^machine.stic.params.' data/expected/tests/machine.ids` is 7, `grep -rn '#include "ivs_stic_params.inc"' core/src` shows one line (`core/src/stic/stic_params.c`).
- Commits on HEAD: `ba6bc47`, `d0e1d89`, `7456369`.
