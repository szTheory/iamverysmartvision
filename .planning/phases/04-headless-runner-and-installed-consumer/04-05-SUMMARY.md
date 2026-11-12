---
phase: 04-headless-runner-and-installed-consumer
plan: 05
subsystem: machine, digest
tags: [machine, digest, layout, D-10, A-03, RUN-05]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-04: steps=29/29, controls=249/249, faults=123/123, tests-expected=2186"
provides:
  - "machine.digest.stepped-literals: composite and domain 1..4 digests of content A after reset and 100 units, recorded from the Phase 3 code (commit dfe7b74)"
  - "struct ivs_machine_obs (VIDEO, AUDIO, INPUT rows) before slot[], explicit obs_pad word; IVS_MACHINE_BYTES 113456 -> 113600"
  - "live digest domains IVS_DIGEST_VIDEO/AUDIO/INPUT (16, 17, 18), domain version 1, outside the version 1 composite over 1..4"
  - "reset clears the observation rows; faults OBS_NOT_CLEARED_ON_RESET and DIGEST_DOMAIN_SWAPPED"
affects: [04-07, 04-11]

actuals:
  tokens: 5559
  tasks: 2
  commits: 3
plan_head_before: dae43393a4415cba68f6dc8a3f8b9bf3c3f56a24
plan_head_after: 55653772238161c9a05c4a15c743870140465f6d

tech-stack:
  added: []
  patterns:
    - "Digest tests iterate an explicit domain list {1,2,3,4,16,17,18} with an in_composite flag; arrays are indexed by domain id (0 = composite)"
    - "A layout change is preceded by its own commit that pins stepped-state literals on the unmodified code"

key-files:
  created: []
  modified:
    - core/src/machine/machine.h
    - core/src/machine/machine_digest.c
    - core/src/machine/machine_run.c
    - core/include/ivs/ivs_machine.h
    - tests/machine/machine_digest.c
    - data/api/api-effects.json
    - data/expected/tests/machine.ids
    - data/expected/faults/machine.ids

key-decisions:
  - "Observation rows are nested structs obs.video{chain,count}, obs.audio{chain,count}, obs.input{words[4],count,floor_tick,chain}; 144 bytes, 8-aligned by an explicit obs_pad word after mem"
  - "The new layout (113,600 bytes) has no tail padding: the tail_padding PAD row is removed and a _Static_assert in machine_digest.c requires slot[] to end at sizeof (a future hole must add a PAD row)"
  - "obs_pad is a PAD row of domain 16 (its neighbour), so padding-ignored exercises the VIDEO preimage"

patterns-established:
  - "Observation domains are never in the composite; sensitivity of a 16..18 row asserts the composite unchanged (the inverse of domains 1..4)"

requirements-completed: []

duration: 45min
completed: 2026-11-11
---

# Phase 4 Plan 05: Observation digest domains 16..18 with the Phase 3 composite pinned Summary

**Stepped-state literals (composite plus domains 1..4 after content A, reset and 100 units) were pinned on the unmodified Phase 3 code in their own commit; the instance then gained a 144-byte observation struct (VIDEO, AUDIO, INPUT) before `slot[]` with an explicit pad word, digest domains 16..18 became live outside the version 1 composite, reset clears them, and every Phase 3 literal stayed green.**

## Performance

- **Duration:** about 45 min
- **Tasks:** 2 of 2
- **Files:** 8 changed (428 insertions, 76 deletions)
- **Cost of the new checks:** each new case runs in about 0.11-0.12 s wall as a single-case process (process start dominates); the whole `machine.digest.` family 0.24 s. Two new fault builds (`fault-runs` 123 -> 125); `step-time c-faults` 5-7 s and `faults-verdict` 8-9 s on this host, no visible rise. Gate wall time 41-62 s with load average 22-28.

## Counts

Start: `steps=29/29 controls=249/249 faults=123/123 tests-expected=2186`.
After the pin commit: `faults=123/123 tests-expected=2188`.
After Task 1: `steps=29/29 controls=249/249 faults=123/123 tests-expected=2194 tests-missing=0 api-calls=10`.
End: `steps=29/29 controls=249/249 fault-runs=125 faults=125/125 tests-expected=2200 tests-executed=2200 tests-skipped=0 tests-missing=0 api-calls=10`; steps and controls unchanged as planned. Machine suite: 385 -> 391 cases (cap 512).

## Accomplishments

- **Pin first (A-03):** `machine.digest.stepped-literals` (repeatability tier, not an oracle) recorded from commit `dae4339` and committed alone as `dfe7b74`; it and `machine.digest.fresh-composite` pass unchanged after the layout change, which shows the composite did not move.
- **Layout:** `IVS_MACHINE_BYTES` **113456 -> 113600**. `mem` ends at 113,284 (4 mod 8); `uint32_t obs_pad` (zero) then `ivs_machine_obs obs` (144 bytes) at 113,288, then `slot[3]` ending at 113,600, a multiple of 16, so the old 4-byte tail padding is gone. Static asserts: obs size 144, no hole between `obs_pad` and `obs`, instance size, and (in `machine_digest.c`) `slot[]` ends at `sizeof`.
- **Digest:** nine new field-table rows (`obs_pad` PAD, `obs.video.chain/count`, `obs.audio.chain/count`, `obs.input.words/count/floor_tick/chain`), placed before the slot rows, so `IVS_FAULT_DIGEST_SKIPS_IMAGE_TABLE` keeps its reach; `ivs_machine_state_digest` and the preimage seam accept 0, 1..4 and 16..18 and refuse everything else BAD_ARG before the handle check (unchanged order); the composite loop stays over 1..4.
- **Public header:** `IVS_DIGEST_RESERVED_*` renamed to `IVS_DIGEST_VIDEO/AUDIO/INPUT` (16, 17, 18); no old name remains in the tree outside `.planning/`.
- **Tests:** coverage, preimage-hash, base/sensitivity and padding loops use the explicit domain list; `md_changed_only` asserts the composite changes for 1..4 and stays identical for 16..18; new `sensitivity-video/-audio/-input`; refusal retargeted to `machine.refuse.digest.domain-19` (test, `machine.ids`, `api-effects.json` text "a domain other than 0..4 and 16..18 (19 included)").
- **Task 2:** `ivs_run_power_on` zeroes `obs` (create already zero-fills the instance); cases `machine.digest.video-fresh`, `machine.digest.composite-excludes-16-18` (flips a VIDEO, an AUDIO and an INPUT byte in turn; exactly the own domain moves, composite and the other six keep their digests) and `machine.reset.clears-observation` (stepped instance with every obs byte 0xA5, reset, all bytes zero and digests equal a fresh instance's). RED observed before the clear: `observation rows not cleared by reset`.
- **Faults:** `OBS_NOT_CLEARED_ON_RESET` (machine_run.c: clear size 0) fails only `machine.reset.clears-observation`; `DIGEST_DOMAIN_SWAPPED` (machine_digest.c: VIDEO chain/count rows tagged 17) fails `composite-excludes-16-18`, `coverage`, `preimage-hash` and `sensitivity-video` (all under `machine.digest.`), must-pass `machine.reset.clears-observation`. Checked by hand in scratch fault builds before the gate.

## Task Commits

1. **Task 1a: stepped-state literal pin (unmodified code)** - `dfe7b74` test(04-05)
2. **Task 1b: observation struct, domains 16..18, generalized tests, domain-19 refusal** - `9c8eafe` feat(04-05)
3. **Task 2: reset clear, three cases, two faults** - `5565377` feat(04-05)

`git log -n 2 --format=%s -- tests/machine/machine_digest.c` before Task 2 showed the pin commit as the older of the two Task 1 commits.

## Final gate summary lines

- `sh tools/gate.sh` (macOS, Apple clang 21): `gate: PASS steps=29/29 controls=249/249 api-calls=10 ... c-presets=2 fault-runs=125 ... faults=125/125 ... tests-expected=2200 tests-executed=2200 tests-skipped=0 tests-missing=0`; `gate-cpu user=54.25 sys=70.68`, `gate-time total=49`.
- `dash tools/gate.sh`: the same tokens, `gate: PASS`; `gate-cpu user=59.93 sys=76.12`, `gate-time total=62`.
- `sh tools/gate-linux.sh` (ubuntu:24.04 amd64 emulated on arm64, `gate-env ncpu=8 cc=GNU-13.3.0`, dash as /bin/sh): `gate: PASS steps=29/29 controls=249/249 api-calls=10 ... cc=GNU c-presets=2 fault-runs=125 ... faults=125/125 ... tests-expected=2200 tests-executed=2200 tests-skipped=0 tests-missing=0`, `gate-time total=250` (emulated and under load; not hosted times). Not required by the plan; run because the instance layout and static asserts changed and gcc must agree.

## Deviations from Plan

### Auto-fixed / discretion

**1. [Rule 3 - blocking] Collateral allowed prefixes for the pin**
- The new `machine.digest.stepped-literals` is (correctly) also failed by three Phase 3 faults that change the stepped state: `CART_REPORT_TRAILER_DIGEST` and `CART_COPY_BEFORE_VALIDATE` (cartridge slot record and words) and `STATIC_CARRY` (stepped time). Their allowed prefixes gained `machine.digest.stepped-literals` in the pin commit, with a comment block; must-fail and must-pass ids unchanged.

**2. Tail padding row removed**
- With the 144-byte observation struct the instance is exactly 113,600 bytes, so the `tail_padding` PAD row would have count 0 (rejected by coverage). The row is removed and replaced by a static assert that `slot[]` ends at `sizeof`; the IMAGES preimage is unaffected (PAD rows are never serialized).

**3. Three extra sensitivity cases**
- `machine.digest.sensitivity-video/-audio/-input` (the generalized sensitivity loop applied to the new domains), beyond the plan's named ids; inventory updated.

**4. Pad row domain**
- `obs_pad` is tagged domain 16 (the neighbour it precedes), per the field-table rule that a PAD row's domain names its neighbour.

**5. Case placement**
- `machine.reset.clears-observation` lives in the digest family file (`tests/machine/machine_digest.c`, the plan's file list), not `machine_reset.c`; the inventory line sits with the reset family ids.

## Open Items

- RUN-05 stays Pending: the domains exist and are separated from the composite, but nothing writes the VIDEO/AUDIO chains or the INPUT row yet (run loop 04-07, input log 04-11, chain rule `SHA256("IVSR" || ...)` and INPUT `h0` later).
- Carried from 04-04: RUN-01 and RUN-04 Pending (runner executable, report and scenarios in 04-17..04-19); plan 04-14 adds `tools/pack/` to the c-build listed-source loop.
- Carried from 04-02..04-04: the orchestrator records hosted runs with `sh tools/ci-record.sh <run-id>` and applies D-45 after 04-10 and 04-19; PKG-03/PKG-04 Pending.
- Carried from 04-01..04-04: the job summary lists `pending-resolvers=17` and `unknown-fields=16` as skips-and-missing; `sh tools/hygiene.sh --history` still fails on earlier planning commits (orchestrator decision; this plan's commits add no home paths).

## Threat Flags

None beyond the plan's threat model: T-04-15 mitigated (stepped literals pinned before the change and green after; `composite-excludes-16-18`); T-04-16 (explicit pad row, no tail padding asserted, `machine.digest.coverage` over seven domains); T-04-17 (`OBS_NOT_CLEARED_ON_RESET`, reset clears `obs`); T-04-SC (no package installs).

## Self-Check: PASSED

- Files present: all eight modified files.
- Commits reachable from HEAD: dfe7b74, 9c8eafe, 5565377 (`git rev-list --count dae4339..HEAD` = 3).
