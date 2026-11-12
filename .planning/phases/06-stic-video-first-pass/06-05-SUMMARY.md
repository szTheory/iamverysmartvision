---
phase: 06-stic-video-first-pass
plan: 05
subsystem: core/stic, core/machine, core/memmap, tests/machine, data/golden
tags: [state-foundation, digest-rows, composite-v3, golden-rebless-1, classified-diff, hidden-state, behaviour-neutral, D-04, D-06, D-16, A-05, A-11, D-23, VID-01, VID-06]
status: complete

requires:
  - phase: 06-stic-video-first-pass
    provides: "plan 06-01 oracle commit eae8ca5; plan 06-04 ivs_stic_params and ivs_stic_params_get (gate steps=36/36 controls=330/330 faults=195/195)"
provides:
  - "data/expected/digest-v2-literals.ids: the composite version 2 repeatability pins (fresh composite, five stepped composites, five digest lines of each of four goldens and of the consumer golden twice, plus the fresh cpu and images digests), recorded from the unmodified tree as the FIRST commit of the plan"
  - "ivs_memsys stic_reg[64] (113,392 bytes), ivs_stic_state (528 bytes, 12 row latches) in core/src/stic/stic.h, instance 114,416 bytes (16 x 7,151), derived on paper first and asserted"
  - "mem.stic_reg in MEM, every ivs_stic_state member in INSTANCE (reserved0 a PAD row, row_latch one 240-word u16 group), all before the slot rows; IVS_DIGEST_COMPOSITE_VERSION 2 to 3"
  - "ivs_stic_state_reset (power-on and create) and ivs_stic_state_validate (cp1610 style, ivs_status); stic_reg cleared at power-on"
  - "tests machine.digest.stic-rows-in-mem-and-instance, machine.digest.composite-v3-framing (renamed), machine.digest.cpu-images-unchanged, machine.reset.stic-state, machine.layout.sizes; faults DIGEST_STIC_ROWS_OMITTED and STIC_STATE_NOT_RESET_ON_POWER_ON"
  - "golden re-bless 1, classified: four .digests goldens and data/expected/pkg-consumer.golden moved only on digest.mem, digest.instance, digest.composite (and the ctrl. twins)"
affects: [06-06, 06-08, 06-09, 06-10, 06-11, 06-12, 06-13, 06-14]

actuals:
  tokens: 11958
  tasks: 4
  commits: 3
plan_head_before: a2f6b2615f2ec37f4e82dec9fe06a16c3ef3ef39
plan_head_after: 97627d21abd0308edb1b975176238deaf48d90a6

tech-stack:
  added: []
  patterns:
    - "Pin the previous composite literals FIRST (a commit that touches nothing under core), then change the layout, then re-bless with a script-classified diff (Phase 5 plan 05-03 pattern, second use)"
    - "Digest table kept in instance offset order by splitting the PSG macro group around mem.stic_reg, so the new rows sit where their bytes sit and a fault that skips rows from slot on cannot hide them"

key-files:
  created:
    - data/expected/digest-v2-literals.ids
    - core/src/stic/stic.h, core/src/stic/stic_state.c
  modified:
    - core/CMakeLists.txt, core/src/memmap/memsys.h, core/src/memmap/memsys.c
    - core/src/machine/machine.h, machine.c, machine_digest.c, machine_run.c
    - tests/machine/machine_digest.c, machine_reset.c, machine_create.c, tests/run/run_fuzz.c (comment)
    - data/expected/tests/machine.ids, data/expected/faults/machine.ids
    - data/golden/diag-cpu-1.digests, diag-input-pass.digests, diag-ctrl-psg-pass.digests, timeout-budget.digests, data/expected/pkg-consumer.golden
    - docs/runner.md

key-decisions:
  - "ivs_stic_state_validate returns ivs_status (IVS_E_BAD_ARG for NULL, IVS_E_BAD_STATE for an out-of-range member) like ivs_cp1610_state_validate, not a member index"
  - "display_latch and mode at reset come from the profile power-on record (stic.power_on: fgbg, latch 0, C85); the record is zero today, so the reset state is 528 zero bytes and the twin oracle (fresh == reset) holds"
  - "The digest table keeps instance offset order: mem.stic_reg sits between the two halves of the PSG macro group (after mem.psg_reserved, before psg.next_sample); the STIC INSTANCE rows follow the PSG rows"
  - "The fresh cpu and images digests of composite version 2 are appended to digest-v2-literals.ids in the test commit (task 1 had only the stepped and golden values), read from a scratch build of the unmodified commit"

duration: about 3 hours elapsed on a loaded laptop (five gate runs of 2.5 to 3 minutes each plus the container gate)
completed: 2026-11-12
---

# Phase 6 Plan 05: state and digest foundation, composite v3, golden re-bless 1 Summary

**STIC guest-visible state is inside the digests from its first byte: the 64 STIC registers are a MEM row, the 528-byte `ivs_stic_state` is eleven INSTANCE rows plus a PAD row, the composite is version 3, reset and create set the state, and the four runner goldens plus the consumer golden moved only on `digest.mem`, `digest.instance` and `digest.composite`. `digest.cpu` and `digest.images` equal the version 2 pins, so adding the storage perturbed neither the CPU nor any earlier timing. Nothing executes against the new storage.**

Oracle commit this plan follows: `eae8ca5` (06-01 task 5). The v2 pin commit is `21b1ad5` (first commit of the plan; `git show --stat` touches only `data/expected/digest-v2-literals.ids`, nothing under `core`).

## What was built

- Task 1 (`21b1ad5`): `data/expected/digest-v2-literals.ids`, 36 literal lines from the unmodified tree (`md_fresh_composite`, the five `md_stepped` composites, and `digest.cpu/mem/images/instance/composite` of `diag-cpu-1`, `diag-input-pass`, `diag-ctrl-psg-pass`, `timeout-budget` and of the consumer golden, the latter twice: `digest.*` and `ctrl.digest.*`). The test literals were extracted from the source by a script and confirmed by a passing `--only machine.digest.` run on the unmodified tree; the golden values were read from the committed files. The gate was green at that commit.
- Task 2, tracer (`a0a5ac3`): storage, rows, composite v3, reset and create. Both presets build under `-Werror`; every static assert held on the first compile (no implicit hole).
- Task 3 (`97627d2`): hand-framed streams, five new tests, two faults, the re-bless. Machine suite 405 to 409 ids, run suite unchanged at 176.
- Task 4: no separate commit. The `docs/runner.md` paragraph (composite version 3, the re-bless, the release recipe) landed in the Task 3 commit with the goldens it describes; the classification re-check, the three gates and the pin comparison are below.

## Layout derivation (bytes; compiler offsets matched the paper figures, no assert was adjusted)

| Item | Phase 5 | Phase 6 plan 05 |
|------|---------|-----------------|
| `ivs_memsys` | 113,264 | 113,392 (`stic_reg[64]` u16 = 128 after `psg_reserved`; 7,087 x 16) |
| `ivs_stic_state` | none | 528: `frame_index` u64 at 0; `event_index` 8, `display_latch` 12, `windows_active` 16, `mode` 20, `cs_ptr` 24, `blocked_reads` 28, `blocked_writes` 32, `late_grants` 36, `grants_in_frame` 40, `reserved0` 44 (PAD); `row_latch[12][20]` u16 at 48, 480 bytes, ending at 528 |
| instance offsets | psg 113,320, obs 113,392, slot 113,592 | psg 113,448, stic 113,520 (0 mod 8), obs 114,048, slot 114,248 |
| `IVS_MACHINE_BYTES` | 113,760 | 114,416 = 16 x 7,151, no tail padding (`IVS_MACHINE_SLOTS_END == sizeof` holds) |

The derivation is written in the layout comment of `core/src/machine/machine.h` (and in `stic.h` and `memsys.h`); `machine.layout.sizes` pins the numbers as literals.

## Digest rows

`mem.stic_reg` (64 x u16, MEM); INSTANCE: `stic.frame_index` (u64), `event_index`, `display_latch`, `windows_active`, `mode`, `cs_ptr`, `blocked_reads`, `blocked_writes`, `late_grants`, `grants_in_frame` (u32 each), `stic.reserved0` (PAD, never serialized), `stic.row_latch` (240 x u16). All before `obs` and `slot[]`. The STIC UINT rows plus the PAD row cover the 528 bytes exactly once (`machine.digest.stic-rows-in-mem-and-instance` and `machine.digest.coverage`). Domain 16 keeps its two rows. The composite hashes domains 1..4 only; `d[4][32]` is unchanged.

## Golden re-bless 1, classified

Regenerated by `build/release/runner/ivs-run --content-dir build/release/images --update-golden <golden> <scenario>` (four goldens) and, for the consumer golden, from `build/gate/pkg-consumer.txt` behind the preserved `#` header (the documented refresh). Classified by a script that compares every line of the HEAD file with the new file (same line count, key by key, `#` header lines included):

| Golden | Lines | Changed lines (old prefix to new prefix) | Class | Everything else |
|--------|-------|-------------------------------------------|-------|-----------------|
| `data/golden/diag-cpu-1.digests` | 69 | `digest.mem` 1a35c393 to 6ff06f9c; `digest.instance` 37bdfbc7 to 854cf55a; `digest.composite` 40fac934 to 38087b1b | allowed | 66 lines byte-identical |
| `data/golden/diag-input-pass.digests` | 77 | `digest.mem` 1a35c393 to 6ff06f9c; `digest.instance` 37bdfbc7 to 854cf55a; `digest.composite` 40fac934 to 38087b1b | allowed | 74 byte-identical |
| `data/golden/diag-ctrl-psg-pass.digests` | 96 | `digest.mem` 9d8f2cb3 to f44d0753; `digest.instance` 38a60a97 to fc76d757; `digest.composite` 79682156 to e2100ba5 | allowed | 93 byte-identical |
| `data/golden/timeout-budget.digests` | 52 | `digest.mem` 2431abbd to 7a9f8386; `digest.instance` aedbd8f6 to 6948b094; `digest.composite` 16162282 to ab941412 | allowed | 49 byte-identical |
| `data/expected/pkg-consumer.golden` | 53 | `digest.mem`, `digest.instance`, `digest.composite` (the diag-input-pass values) and `ctrl.digest.mem`, `ctrl.digest.instance`, `ctrl.digest.composite` (the diag-ctrl-psg-pass values) | allowed | 47 byte-identical |

For every golden `digest.cpu`, `digest.images`, `digest.audio`, `digest.input`, `digest.psglog`, every `step.*`, `slot.*`, `cpu.*`, `now_ticks`, stop-reason, `output.*` line and the `# domains:` header are byte-identical to HEAD, and `digest.cpu` and `digest.images` of all four goldens equal the `golden.<id>.digest.cpu/images` lines of `digest-v2-literals.ids` (checked by a loop over the files). The consumer `digest.*` lines equal `diag-input-pass.digests` and its `ctrl.digest.*` lines equal `diag-ctrl-psg-pass.digests`; video is still not observed (`video=none`, no frame is delivered until plan 06-09), so runner and consumer show the same observed set (cpu mem images instance audio, plus input and psglog where the scenario has them), and the consumer calls no new function. No `# phase 6 re-bless 1:` comment was added to the `.digests` headers (the next `--update-golden` drops it); this table is the durable record.

## Tests and faults

- Hand-framed streams: the INSTANCE stream is now head (47) + PSG tail (68) + STIC tail (528 = 8 + 36 + 4 + 480), the fresh tail a designated-initialiser literal, the synthetic one a 528-byte literal with distinct values and reserved0 `EEEEEEEE` (not serialized); the MEM tail is 174 bytes (psg_reg, port_pulldown, stic_reg words 5000, 5111, ... ). The SHA-256 of both INSTANCE streams (37dba9ae..., 70dcdb97...) were computed by an independent Python script over the hand bytes, never by the code under test. `machine.digest.composite-v3-framing` rebuilds the composite from the literal CPU stream, the literal INSTANCE stream and the preimage seam hashed with the repo SHA-256 (version bytes `00 03`).
- `md_fresh_composite` (02c86009...) and the five `md_stepped` composites carry the v3 values, labelled repeatability, with the v2 values quoted in the comments and in `digest-v2-literals.ids`.
- `machine.digest.cpu-images-unchanged` compares the fresh and stepped cpu and images digests with the v2 literals; `machine.digest.stic-rows-in-mem-and-instance` flips the first and last byte of every STIC row (MEM and INSTANCE, composite changes, no other domain preimage moves); `machine.reset.stic-state` dirties `stic_reg` and the state through the raw seam, resets, and compares the whole storage with a powered reference and the STIC ranges with a fresh instance, and exercises `ivs_stic_state_validate` for each member; `machine.layout.sizes` pins 113,392 / 528 / 114,416 and every offset.
- Faults `DIGEST_STIC_ROWS_OMITTED` (must fail `machine.digest.stic-rows-in-mem-and-instance`, allowed prefix `machine.digest.`) and `STIC_STATE_NOT_RESET_ON_POWER_ON` (must fail `machine.reset.stic-state`, allowed only itself) landed with their tests and registrations. `DIGEST_COMPOSITE_VERSION_STALE` keeps its id, frames version 2 under the define and now names `machine.digest.composite-v3-framing`.

## Deviations from Plan

**1. [Rule 3 - registration] Two existing cart faults needed one more allowed prefix.** `CART_COPY_BEFORE_VALIDATE` and `CART_REPORT_TRAILER_DIGEST` change the images digest of content A, so the new `machine.digest.cpu-images-unchanged` fails under them (collateral, as `machine.digest.stepped-literals` already does); both lines in `data/expected/faults/machine.ids` gained that id. Found by the first gate run (`fault-wrong-reason`).

**2. [Plan order] The Task 4 documentation landed in the Task 3 commit,** with the goldens it describes; Task 4 made no commit of its own (nothing needed fixing).

**3. [Rule 3 - test data] Two lines appended to `digest-v2-literals.ids` after the pin commit** (`test.fresh.cpu`, `test.fresh.images`): the plan's `cpu-images-unchanged` test needs the fresh-instance digests and Task 1 had recorded only stepped and golden values. They were printed from a scratch build of commit `21b1ad5` (an archive built outside the repo, a throwaway diagnostic that was never committed); the file says so in a comment.

**4. [Rule 1 - naming] `machine.digest.composite-v2-framing` was renamed `composite-v3-framing`** instead of adding a second composite test (the v2 framing is the v3 framing with other content and version bytes; the old id is gone from the inventory and from the fault line).

**5. [Interpretation] `ivs_stic_state_validate` returns `ivs_status`** (the cp1610 style the plan names) rather than a member index.

**6. [Plan wording] The step 2 text says `cs_ptr` below 4 while the must_haves say "cursor below 16, rows and flags in range"; both are implemented** (event_index below 16, flags 0 or 1, mode 0 or 1, cs_ptr below 4, reserved0 0). `display_latch` at reset comes from the profile record, which is 0.

The Task 2 commit is red on the machine suite literal pins by design (eight ids: composite framing, v2 framing, fresh composite, MEM rows, stepped literals, two instance streams, video-fresh); no push occurred between the Task 2 and Task 3 commits and the gate was not run at the Task 2 commit.

## Decisions Made

- The STIC state members are all `uint32_t` (plus the u64 frame index and the u16 row latches), flat and pointer-free; `reserved0` is a declared PAD row, so there is no hole and nothing to hide.
- Nothing reads or writes `stic_reg` or the state outside reset, create and the digest: the hidden-state rule holds from the first byte of storage.

## Open Items

- `stic_reg` and `ivs_stic_state` are inert; the device plan 06-06 flips the five STIC regions to real and the timeline plans use the cursor, the latch and the counters. `ivs_stic_state_validate` has no production caller yet (a Phase 8 snapshot reload and plan 06-06 will call it).
- `ivs_info` v4, report v3 and the env slack word are still 0 (carried from 06-03 and 06-04; plan 06-09).
- `run.refuse.flags-frame-reserved` inversion (06-09), domain 16 digest value init (06-10), the 06-01 second-read records, the Phase 4 and 5 deferrals, and `sh tools/hygiene.sh --history` failing on earlier planning commits are carried unchanged.
- Requirements VID-01 and VID-06 are NOT closed: there is no device, no read or write path and no inspection call yet; they stay Pending.

## Final gate

```
sh tools/gate.sh:       gate: PASS steps=36/36 controls=330/330 ... faults=197/197 ... profile-keys=142 ... tests-expected=3814 tests-executed=3812 tests-skipped=2 tests-missing=0   (gate-time total=145)
dash tools/gate.sh:     the same summary line (identical bytes)                                                                                                    (gate-time total=183)
sh tools/gate-linux.sh: exit 0, last line of build/gate-linux.log starts gate: PASS, cc=GNU (gcc 13.3.0, x86_64), steps=36/36 controls=330/330 faults=197/197 profile-keys=142 tests-missing=0   (gate-time total=504)
```

Suite times (dev preset, loaded laptop): machine suite about 1 s (409 ids), run suite about the same as before; the full `sh` gate 145 s, within the hosted budgets of `.planning/CI-EFFICIENCY.md` (one new TU, no workflow, matrix or cache change). The first gate run of the plan failed `faults-verdict` (deviation 1) and `pkg-install` (the consumer golden before its refresh), both fixed in the same commit.

## Known Stubs

None. The storage is inert by design (documented in `stic.h`).

## Threat Flags

None new. T-06-06 is mitigated: rows for every member with a coverage sweep that reports holes, the `DIGEST_STIC_ROWS_OMITTED` fault, the v2 literals pinned first in a commit that touches nothing under `core`, and the golden diff classified by script with a hard stop that did not trip. T-06-SC: no installs.

## Self-Check: PASSED

Created files exist (`data/expected/digest-v2-literals.ids`, `core/src/stic/stic.h`, `core/src/stic/stic_state.c`); commits `21b1ad5`, `a0a5ac3`, `97627d2` are ancestors of HEAD; `git show --stat --format= 21b1ad5 -- core` is empty.
