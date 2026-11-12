---
phase: 05-hand-controllers-and-psg-sound
plan: 03
subsystem: core/machine (instance layout, state digest), core/memmap, core/psg, tests/machine, data/golden
tags: [state-digest, composite-v2, domain-19, psg-storage, golden-rebless, layout, D-04, D-16, D-18, A-09, A-13, AUD-02, API-03]
status: complete

requires:
  - phase: 05-hand-controllers-and-psg-sound
    provides: "plan 05-01 oracle commit 14a1904; plan 05-02 behaviour-neutral ABI growth (HEAD at start 431fd3b)"
provides:
  - "data/expected/digest-v1-literals.ids: the Phase 4 composite-version-1 literals (fresh composite, five stepped digests, cpu/mem/images/instance/composite of the three goldens and the consumer golden), recorded from the unmodified tree in the first commit of the plan"
  - "the instance carries PSG storage: ivs_memsys psg_reg[16], port_pulldown[2] and a pad (MEM rows); the 72-byte ivs_psg_state (INSTANCE rows) before obs; obs.audio emitted and prev_levels (AUDIO rows); obs.psglog chain and count (PSGLOG rows); IVS_MACHINE_BYTES 113600 -> 113760"
  - "digest domain 19 (IVS_DIGEST_PSGLOG), valid for ivs_machine_state_digest, outside the composite; the first refused domain is 20; composite version 1 -> 2 (IVS_DIGEST_COMPOSITE_VERSION 2u)"
  - "ivs_memsys_set_port_pulldown_internal and ivs_psg_state_reset (storage only; nothing reads or advances them yet)"
  - "re-blessed goldens (three .digests files and pkg-consumer.golden) with a classified diff"
affects: [05-04, 05-05, 05-06, 05-07, 05-08, 05-09]

actuals:
  tokens: 60000
  tasks: 4
  commits: 4
plan_head_before: 431fd3b705f4f829d320d029861da49848dccc50
plan_head_after: d8c2553

tech-stack:
  added: []
  patterns:
    - "Every implicit byte of a grown instance has an explicit PAD member and PAD row (psg_reserved, psg.reserved[3], obs.audio.reserved); the total is re-derived on paper in the layout comment of machine.h, with offsets"
    - "A fault that removes rows from the field table (IVS_DF_PSG_ROWS macro) is caught by machine.digest.coverage, which a serialize-time skip would not be"
    - "Independent framing of the composite: literal version bytes, literal preimage streams for domains 1 and 4, domains 2 and 3 hashed here from the preimage seam with the repo's SHA-256 (the core's digest path is not used)"

key-files:
  created:
    - data/expected/digest-v1-literals.ids
    - core/src/psg/psg.h
    - core/src/psg/psg_state.c
  modified:
    - core/include/ivs/ivs_machine.h
    - core/src/machine/machine.h, machine_digest.c, machine_run.c
    - core/src/memmap/memsys.h, memsys.c
    - core/CMakeLists.txt
    - data/api/api-effects.json
    - tests/machine/machine_digest.c, machine_reset.c
    - tests/run/run_observe.c, run_cases.h
    - data/expected/tests/machine.ids, data/expected/faults/machine.ids
    - data/golden/diag-cpu-1.digests, diag-input-pass.digests, timeout-budget.digests
    - data/expected/pkg-consumer.golden

key-decisions:
  - "The PSG device state is 72 bytes (next_sample u64, 13 u32 state words, 3 reserved u32 words), not 64: with 64 the instance would be 113,752, 8 mod 16, and sizeof would round up with implicit tail padding; the 3 reserved words are PAD rows, so the instance is 113,760 = 7,110 x 16 with no hole"
  - "prev_levels and emitted live only in obs.audio (domain 17), not in ivs_psg_state: the must_haves and plan 05-06 both name them as observation rows; the plan text for the state struct also listed prev_levels, which would have been a second copy"
  - "obs layout: video 40, audio 56 (chain, count, emitted, prev_levels, reserved), input 64, psglog 40 = 200; psglog is appended after input so no existing obs offset moved relative to obs"
  - "DIGEST_PSG_ROWS_OMITTED removes the rows from the field table (not a skip at serialize time), because the plan requires machine.digest.coverage to catch it"

duration: about 70 min including three gate runs and one container run
completed: 2026-11-12
---

# Phase 5 Plan 03: state and digest foundation Summary

**The instance now stores the PSG (16 register words, two pull-down masks, a 72-byte device state, audio level rows, write-log rows); every guest-visible byte of it is inside composite version 2, domains 17 and 19 report the observation rows outside the composite, a power-on reset clears all of it, and nothing reads or advances any of it, so every golden moved only on `digest.mem`, `digest.instance` and `digest.composite`.**

Oracle commit (05-01): `14a1904`. The Phase 4 v1 literals were recorded first, in commit `5114d8a` (Task 1; `git show --stat 5114d8a` lists one file, no source).

## What was built

- Task 1 (`5114d8a`): `data/expected/digest-v1-literals.ids` (26 lines): `test.fresh-composite`, `test.stepped.0..4`, and the five digest lines of each of the three goldens and the consumer golden. Extracted from the unmodified tree; `machine.digest.` was green (18 ids) at that commit, which proves the test literals equal the running values; the golden values were read from the committed files.
- Task 2 (`588aa4a`, tracer): layout, `ivs_psg_state`, `ivs_psg_state_reset`, the setter, rows, domain 19, composite v2, power-on reset, the three fault sites, the api-effects row. Both presets built under `-Werror`; only the literal pins, the hand-framed instance stream, coverage (domain 19 not listed in the test) and the domain-19 refusal failed, as the plan predicted (the machine suite is red between this commit and the next; no push occurred between them).
- Task 3 (`24a6ebb`): tests, faults and inventories (below).
- Task 4 (`d8c2553`): the goldens.

## Layout derivation result

| Part | Phase 4 | Now | Offset (now) |
|------|---------|-----|--------------|
| header + cpu | 56 | 56 | 0 |
| mem | 113,228 | 113,264 (+ psg_reg 32, port_pulldown 2, psg_reserved 2) | 56 |
| obs_pad | 4 | gone (mem now ends 0 mod 8) | |
| psg (`ivs_psg_state`) | none | 72 | 113,320 |
| obs | 144 | 200 (video 40, audio 56 (was 40), input 64, psglog 40 (new)) | 113,392 |
| slots | 168 | 168 | 113,592 |
| `IVS_MACHINE_BYTES` | 113,600 | **113,760** | |

The derivation, every offset, and the reason for 72 (not 64) bytes of PSG state are in the layout comment of `core/src/machine/machine.h`; the static asserts pin `sizeof(ivs_memsys) == 113264`, `sizeof(ivs_psg_state) == 72`, `sizeof(ivs_machine_obs_audio) == 56`, `sizeof(ivs_machine_obs) == 200` and the offsets of `mem`, `psg`, `obs` and `slot`. `md_coverage` reports no hole.

## Golden diff classification (A-09, A-13)

Method: `ivs-run --update-golden` once per scenario (the three commands of `docs/runner.md`, dev preset, `build/dev/images`); the consumer golden from `build/gate/pkg-consumer.txt` of the gate run (headers kept). `git diff -U0 -- data/golden data/expected/pkg-consumer.golden`: 14 changed lines, 0 outside the three allowed keys (checked by script).

| Golden | Changed lines | Class | Every other line |
|--------|---------------|-------|------------------|
| `diag-cpu-1.digests` | `digest.mem`, `digest.instance`, `digest.composite` | allowed (A-13, D-18) | byte-identical (`digest.cpu`, `digest.images`, `slot.*`, `step.*`, `cpu.*`, `input.*`, `output.*`, headers) |
| `diag-input-pass.digests` | the same three, same new values as above | allowed | byte-identical (`digest.input` too) |
| `timeout-budget.digests` | the same three | allowed | byte-identical |
| `pkg-consumer.golden` | the same three (same values as `diag-input-pass`; the runner-golden agreement check holds) | allowed | byte-identical (`v1-sizes` line, `digest.cpu/images/input`, all `step.*`) |

`digest.cpu` and `digest.images` of all four files equal the recorded v1 values in `data/expected/digest-v1-literals.ids` (checked by script: 8 of 8 equal). That is the regression proof that adding PSG storage did not perturb the CPU or the image table. `digest.input`, `digest.video`, `digest.audio` did not change. `digest.mem` moves because of the new MEM rows (A-13, intended); `digest.instance` because of the PSG state rows (zero, but now framed); `digest.composite` follows from both and from the version bump. Values: `digest.mem` 1d935916... to 1a35c393... (diag), `digest.instance` 3a953773... to 6338be82..., `digest.composite` c834c07a... to 9022ed7a...; timeout-budget 02984e82... to 2431abbd..., 11c92cb4... to 869bae5a..., 3208e03e... to 387f6a12.... Stepped literals: composite, MEM and INSTANCE changed; CPU and IMAGES are the v1 values byte for byte.

## Tests, faults and counts

- New ids (+9 per preset, machine suite 388 to 397): `machine.digest.composite-v2-framing`, `stream-instance-psg` (all PSG members distinct, hand-written big-endian tail), `domain-19-fresh` (literal stream and hash), `psg-rows-in-instance-domain` (exactly the 10 UINT rows), `mem-rows-psg-regs` (every byte of `psg_reg` and `port_pulldown`, plus the MEM preimage tail framed by hand), `audio-rows`, `psglog-rows` (the sensitivity sweep of domain 19), `cpu-images-independent-of-psg` (16 rows, first and last byte), `machine.reset.psg-state-cleared`; `machine.refuse.digest.domain-19` became `domain-20`. `composite-framing` is now version 2. `md_obs_digests`, the run-suite snapshot (`IVS_RUN_SNAP_DIGESTS` 8 to 9) and `MD_DOMAIN_IDS` (20) include domain 19.
- Faults 152 to 155: `DIGEST_PSG_ROWS_OMITTED` (fails `machine.digest.coverage`), `DIGEST_COMPOSITE_VERSION_STALE` (fails `machine.digest.composite-v2-framing`), `PSG_STATE_NOT_RESET_ON_POWER_ON` (fails `machine.reset.psg-state-cleared`). One allowed-prefix addition: `OBS_NOT_CLEARED_ON_RESET` now also fails `machine.reset.psg-state-cleared` (that test dirties the observation rows too).
- Counts: start `steps=35/35 controls=296/296 faults=152/152 tests-expected=2844`; end `steps=35/35 controls=296/296 faults=155/155 tests-expected=2862 tests-executed=2860 tests-skipped=2 tests-missing=0`, `pinned-defines=95` unchanged.
- Machine suite wall time (one run each, loaded laptop, right after a gate): dev (ASan and UBSan) 0.50 s, release 0.27 s, 397 cases in each.
- Gate wall time: `sh` total=32, `dash` total=34, container (x86_64 emulated, gcc 13.3.0, cmake 3.28.3) total=146.

## Deviations from Plan

**1. [Rule 1 - Plan arithmetic] The PSG state is 72 bytes, not 64.** The plan's step 1 derives mem 113,264 but not the total; with a 64-byte state (8 + 13 x 4 + 4 pad) the instance is 113,752, 8 mod 16, and `sizeof` would round up with 8 bytes of implicit tail padding (the `IVS_MACHINE_SLOTS_END == sizeof` assert and the no-hole rule forbid that). Three reserved u32 words (PAD rows) bring it to 72; recorded in the layout comment.

**2. [Rule 1 - Plan inconsistency] `prev_levels` is an observation row only.** The plan text lists it in `ivs_psg_state` and as an audio row; the must_haves and plan 05-06 ("the `prev_levels` row already declared by plan 05-03") treat it as the audio row. Kept one copy, in `obs.audio`.

**3. [Rule 1] The refusal cases live in `tests/machine/machine_digest.c`, not `machine_refuse.c`.** The plan names `machine_refuse.c` for the `domain-19` to `domain-20` rename; the case, like the whole digest-refusal family, is in the digest file. `machine_refuse.c` and `machine_cases.h` needed no change.

**4. [Rule 3 - Blocking] The `DIGEST_PSG_ROWS_OMITTED` shape.** Skipping at serialize time cannot fail `machine.digest.coverage` (the table is unchanged), so the rows are removed from the table by a macro; the plan says coverage must catch it.

**5. Test-side additions beyond the plan list:** `machine.digest.stream-instance-psg` (a synthetic PSG state with distinct words, hash computed at authoring time with `shasum` over hand-written bytes) so the INSTANCE framing is pinned with non-zero values, not only the all-zero fresh stream.

## Decisions Made

- `ivs_memsys_set_port_pulldown_internal` masks to 8 bits and ignores a port at or above 2; the PSG registers and masks are zeroed by the power-on reset in `machine_run.c` (the instance creation memset already zeroes them).
- The two memsys pad bytes and the PSG state's reserved words are never written; the reset test dirties everything else and compares the whole storage against an untouched instance.
- The composite version bump is the only framing change; domain versions stay 1.

## Open Items

- `tests/runner-process-tap.sh` flaked once under `dash tools/gate.sh` (`line 123 ... hung.ready: Interrupted system call`, the `mut-BNZE-BZE` case, ctest `runner-process` release); the rerun and the `sh` and container runs passed. Unrelated to this plan (runner and signals, no instance layout involved); a candidate for the next CI-efficiency review.
- AUD-02 and API-03 stay Pending in REQUIREMENTS.md: this plan stores state only; the input byte (05-05), samples (05-06), log (05-08) and end-to-end evidence (05-11) close them. 05-04 and later plans read and write `psg_reg` and `port_pulldown` and give the noise register its seed (`ivs_psg_state_reset` is all-zero today).
- Plans 05-06 and 05-08 re-bless the goldens again (A-09); `digest.audio` and `output.audio.bytes` lines will then move, as will `digest.instance` and `digest.composite`.
- Carried from earlier plans (unchanged): 05-02 items (`run.info.v3-fields-zero`, `run.struct.report-v2-fields` to be replaced by literals in 05-06 and 05-08; api-effects text "Nothing in the machine reads the words" for 05-05), the unobtained AY-3-8910 reprint and pin chart, D10/D11 items, Phase 4 items IN-04, IN-06, IN-09..IN-11.

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=35/35 controls=296/296 ... faults=155/155 ... tests-expected=2862 tests-executed=2860 tests-skipped=2 tests-missing=0`, `gate-time total=32`, 0 FAIL lines (at the golden commit `d8c2553`).
- `dash tools/gate.sh`: the same line, `gate-time total=34`, 0 FAIL lines (second run; the first run failed only in `runner-process`, see Open Items).
- `sh tools/gate-linux.sh`: exit 0; log last line `gate: PASS steps=35/35 controls=296/296 ... faults=155/155 ... tests-expected=2862 tests-executed=2860 tests-skipped=2 tests-missing=0` (`cc=GNU`, 0 FAIL lines).

## Threat Flags

None new. T-05-08 (hidden PSG state missing from the composite): mitigated by MEM and INSTANCE rows, composite v2, `machine.digest.psg-rows-in-instance-domain`, `mem-rows-psg-regs`, faults `DIGEST_PSG_ROWS_OMITTED` and `DIGEST_COMPOSITE_VERSION_STALE`. T-05-09 (layout hole): derived on paper, PAD rows, `md_coverage`, size and offset asserts, the gcc container gate. T-05-SC: no installs.

## Self-Check: PASSED

- Files exist: data/expected/digest-v1-literals.ids, core/src/psg/psg.h, core/src/psg/psg_state.c.
- Commits on HEAD: 5114d8a, 588aa4a, 24a6ebb, d8c2553.
