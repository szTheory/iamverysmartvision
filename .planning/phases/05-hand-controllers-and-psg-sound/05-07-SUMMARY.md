---
phase: 05-hand-controllers-and-psg-sound
plan: 07
subsystem: core/psg, core/machine, core/memmap, cmake, tests/psg, data/profiles, data/golden
tags: [psg-noise, psg-envelope, dac-table-as-data, profile-perturbation, golden-rebless-3, D-07, D-10, D-11, D-12, A-09, AUD-01]
status: complete

requires:
  - phase: 05-hand-controllers-and-psg-sound
    provides: "plan 05-01 oracle commit 14a1904 (noise.txt, env.txt, dac.txt, ampenv.txt, reset-engine.txt, rules.json, psg-policies.ids); plan 05-04 register file and pending lists; plan 05-06 chip-step scheduler, mixer, amplitude, sample stream (HEAD at start 4fc7484)"
provides:
  - "noise generator core/src/psg/psg_noise.c: right-shifting Galois register from psg.noise.lfsr (width 17, mask $12000, seed $00001, out bit 0, invert 0), a shift every psg.noise.steps_per_np (2) x NP steps, a written 0 acting as psg.period_zero.noise (32); the mixer reads the register bit (the constant-1 stub of 05-06 is gone)"
  - "envelope generator core/src/psg/psg_env.c: the 16-state machine of the 16 shapes (psg.envelope.shape_bits), one state per psg.envelope.steps_per_ep (2) x EP steps, a written 0 acting as psg.period_zero.envelope (65536), restart on a shape write, held at level 0 after reset; its level feeds ivs_psg_amp_index (the level-0 stub of 05-06 is gone)"
  - "7 profile fields (psg.period_zero.noise, psg.noise.steps_per_np, psg.noise.lfsr, psg.period_zero.envelope, psg.envelope.steps_per_ep, psg.envelope.shape_bits, psg.dac.levels); data/psg/fields-pending.ids and data/psg/vectors-pending.ids hold no line any more"
  - "psg.dac.levels: 16 integers in units of 1e-8 of full scale that the core never reads (not a member of ivs_psg_params, not in libivs_core.a); test-side generator cmake/IvsPsgProfileProbe.cmake and the build-time proof cmake/IvsPsgDacIndependence.cmake"
  - "psg suite 191 ids per preset (was 121): 48 vectors of the five new families, 10 perturbation tests, 12 fixed cases and 3 judge self-controls; 6 named faults (180 total)"
  - "golden re-bless #3 (three .digests files and pkg-consumer.golden), classified"
affects: [05-08, 05-09, 05-10, 05-11, 05-12]

actuals:
  tokens: 33500
  tasks: 5
  commits: 2
plan_head_before: 4fc74841a3837d005eb2dcdd1d203008a6b084fe
plan_head_after: 1fd2ef95decee5c0986b3218efd0b1b9e19e7b8b

tech-stack:
  added: []
  patterns:
    - "The one shared write path: ivs_psg_write = catch-up to the stamp, masked store, ivs_psg_after_write; the machine bus wrapper does catch-up, the memory system's store, then the same ivs_psg_after_write with the register slot from the memory system's own write decode (ivs_memsys_psg_write_slot_internal), so a stamp-0 vector write and a guest write cannot diverge"
    - "A shape row of env.txt lists one level per envelope STATE: the vector generator emits a sampling stride 2 x EP (from the row's own period writes) and the judge reads level j at step stride x j (step 1 for j 0); every other row is read per step"
    - "Perturbation tests build a copy of ivs_psg_params on the stack (no registry seam) and require the predicted change; a hard-coded engine constant therefore fails exactly its perturbation test"
    - "A table the core must not read is proved absent twice: no 32-bit pattern of a nonzero level occurs in any ivs_psg_params row, and two profiles that differ only in the table generate byte-identical core parameters (a build-time script, so a violation is a test failure, not a build failure)"

key-files:
  created:
    - core/src/psg/psg_noise.c
    - core/src/psg/psg_env.c
    - cmake/IvsPsgProfileProbe.cmake
    - cmake/IvsPsgDacIndependence.cmake
    - tests/psg/psg_noise.c
    - tests/psg/psg_env.c
    - tests/psg/psg_dac.c
    - tests/psg/psg_profile.c
  modified:
    - cmake/IvsPsgTable.cmake, cmake/IvsPsgVectors.cmake, core/CMakeLists.txt
    - core/src/psg/psg.h, psg_params.h, psg_sample.c, psg_state.c
    - core/src/memmap/memsys.c, memsys.h, core/src/machine/machine_bus.c, machine_run.c
    - data/profiles/intv-2609-ntsc-family-p1.json, data/expected/profile-fields.ids, data/psg/fields-pending.ids, data/psg/vectors-pending.ids
    - data/expected/tests/psg.ids, data/expected/faults/psg.ids
    - data/golden/diag-cpu-1.digests, diag-input-pass.digests, timeout-budget.digests, data/expected/pkg-consumer.golden
    - tests/CMakeLists.txt, tests/psg/psg_cases.h, psg_main.c, psg_tone.c, psg_util.h
    - tests/machine/machine_cases.h, machine_digest.c, machine_standin.c

key-decisions:
  - "Noise: the register shifts at step 2 NP counted from the reload (policy POL-PSG-02, like the tone); the first-bits vector lists the output AFTER each shift (the committed convention of docs/psg/second-read-record-noise.md stands; a reader listing from the seed state sees the seed's 1 first)"
  - "Envelope phase (docs/psg/second-read-record-env.md): a shape write restarts with an empty step counter, so the sample of the next step shows the first level and the first level change is 2 x EP steps later; the restart property and the 2 x EP step rate are what the vectors pin, the phase is a convention (stated in the header of tests/psg/psg_env.c)"
  - "ivs_psg_state_reset takes the parameters: the power-on state holds the noise seed and env_held 1 at level 0; env_cycles is the index 0..15 of the state inside its cycle"
  - "A held envelope does not run its step counter; a repeating shape reloads the counter from the then-current period at every state"
  - "psg.noise.lfsr width is validated (mask and seed within it, out_bit below it) but is not an engine parameter: a value within the width stays within it under the Galois step"

duration: about 4 h including nine gate runs
completed: 2026-11-12
---

# Phase 5 Plan 07: Noise, envelope and the DAC table as data Summary

**The PSG now has its noise register and its 16-shape envelope, both driven only by tier-tagged profile fields (mask, seed, steps per period, period-zero values, shape bit roles), every rule without a primary source is proved to be data by ten perturbation tests, the DAC table is profile data that the core provably never reads, both pending lists end empty, and the gate ends `steps=35/35 controls=303/303 faults=180/180` under `sh`, `dash` and the Ubuntu container.**

Oracle commit (05-01): `14a1904` (`noise.txt`, `env.txt`, `dac.txt`, `ampenv.txt`, `reset-engine.txt`, `rules.json` and `psg-policies.ids` were committed there; no oracle file was edited).

## What was built

- Commit `ec4ebfc` (Tasks 1 to 4 together, see Deviation 1): the seven profile fields with their tags and citations (C71, C72, C69, C12); `IvsPsgTable.cmake` (fail closed: width 1..32, mask and seed nonzero and within the width, out bit below the width, invert 0 or 1, steps_per at least 1, period-zero ranges, four distinct shape bit numbers 0..3, `psg.reset` keys lfsr = seed and envelope = held-0); `psg_noise.c` and `psg_env.c` (three named fault sites each); the sample step runs the noise and envelope steps; the bus wrapper and `ivs_psg_write` share `ivs_psg_after_write`; vector families noise (12), env (22), ampenv (4), reseng (2), dac (8) wired, the five `vectors-pending.ids` lines and the seven `fields-pending.ids` lines deleted; `tests/psg/psg_noise.c`, `psg_env.c`, `psg_dac.c`, `psg_profile.c`; the six faults registered; the machine `stepped-literals` re-recorded.
- Commit `1fd2ef9` (Task 5): golden re-bless #3.

## Test ids (191 per preset; was 121)

| Group | Ids |
|---|---|
| noise (vectors 12) | rate-np1, np2, np3, np16, np31, first-shift-at-2np, no-shift-before-2np, period-zero, lfsr-maximal-period, lfsr-alt-period, lfsr-first-bits, level-seq-np1; fixed `psg.noise.mixer-follows-register` |
| env (vectors 22) | shape-0 .. shape-15, rate-ep1, ep2, ep3, ep15625, restart-on-shape-write, period-zero; fixed `period-write-no-restart`, `reset-holds-zero`, `shapes-hold-levels` (property guard: every shape reaches its documented hold level), `guest-shape-write-starts` (new stand-in program ENV through the machine bus wrapper), `psg.amp.envelope-drives-level` |
| ampenv (4), reseng (2) | the four and two vectors; fixed `psg.reseng.reset-state` (seed, held 0, counters 0) |
| dac (8 vectors) | printed-levels, extrapolated-levels, level-0, level-15-full-scale, halving-8-from-10, halving-1-from-3, table-length, sum-fits-u32; fixed `psg.dac.monotonic` (non-decreasing, level 0 the minimum, 3 x level 15 at most 300,000,000), `psg.dac.not-in-core` |
| profile (10) | the eight of the plan (`noise-taps`, `period-zero-tone`, `period-zero-noise`, `period-zero-envelope`, `clocks-per-step`, `mixer-bits`, `env-select`, `dac-levels-not-read-by-core`) plus `steps-per-np-ep` and `env-shape-bits` |
| judge (3) | `psg.judge.noise-row-changed`, `psg.judge.env-row-changed`, `psg.judge.dac-row-changed` |

All 48 vectors are evaluated; none fell back to a different kind than committed. The page-reading verdict of plan 05-01 stands: the rate and shape rows are `source-expectation` (inference from the printed divide chains and Fig. 3), the period-zero noise row, the LFSR rows, the write-stamp ampenv rows, the reset rows and all eight DAC rows are `kind=policy` as committed (C12 stays open: the DAC rows copy the discuss-note reading of a reprint that plan 05-01 could not obtain, so they are not an independent source).

## Named faults (180 = 174 + 6)

| Fault | Site | Must fail | Also fails (allowed) |
|---|---|---|---|
| `PSG_NOISE_WRONG_TAP` | psg_noise.c | `psg.noise.lfsr-maximal-period` (114,681 instead of 131,071) | psg.noise., judge.noise-, profile. |
| `PSG_NOISE_RATE_USES_TONE_DIVISOR` | psg_noise.c | `psg.noise.rate-np1` | same |
| `PSG_NOISE_TAP_HARDCODED` | psg_noise.c | `psg.profile.noise-taps` | psg.noise., psg.profile. |
| `PSG_ENV_SHAPE_ATTACK_DECAY_SWAPPED` | psg_env.c | `psg.env.shape-0` | env., amp., ampenv., judge.env-, profile.env- |
| `PSG_ENV_PERIOD_TIMES_16` | psg_env.c | `psg.env.rate-ep1` | env., amp., ampenv., judge.env-, profile. |
| `PSG_ENV_HOLD_ALTERNATE_NO_RESET` | psg_env.c | `psg.env.shape-11` | env. |

The allowed prefixes of six existing psg faults (`PSG_REG_OCTAL_AS_HEX`, `PSG_TONE_PERIOD_OFF_BY_ONE`, `PSG_STEP_IS_16_CLOCKS`, `PSG_MIXER_SENSE_INVERTED`, `PSG_AMP_ENV_BIT_IGNORED`, `PSG_SAMPLE_PHASE_RESET_PER_CALL`) gained the families that now collaterally fail (`psg.amp.`, `psg.ampenv.`, `psg.env.`, `psg.noise.`, `psg.profile.`, `psg.reseng.`, `psg.judge.env-`, `psg.judge.noise-`): each of those faults changes the time chain, the mixer or the amplitude that the new tests also read; the must-fail ids are unchanged.

## Golden classification (re-bless #3)

Regenerated with `ivs-run --update-golden` (the three `.digests`) and from `build/gate/pkg-consumer.txt` with the header lines kept (the consumer golden); `git diff -U0` shows only these lines:

| File | Line | Class |
|---|---|---|
| `diag-cpu-1.digests`, `diag-input-pass.digests` | `digest.instance`, `digest.composite` | allowed: the noise seed, the held envelope and the shifting register are in the INSTANCE rows |
| `timeout-budget.digests` | `digest.instance`, `digest.composite` | allowed, same reason |
| `pkg-consumer.golden` | `digest.instance`, `digest.composite` (equal to the runner golden's `diag-input-pass`) | allowed |
| every golden | `digest.cpu`, `digest.mem`, `digest.images`, `digest.input`, `digest.video`, `step.*`, `slot.*`, `cpu.*`, domains line, the consumer `info` line | byte-identical (the guests never touch the PSG); `digest.cpu` and `digest.images` still equal `digest-v1-literals.ids` |

The runner delivers no audio buffer yet, so no `digest.audio` line appeared. Also re-recorded in a test: `machine.digest.stepped-literals` (composite and INSTANCE; CPU, MEM and IMAGES unchanged byte for byte).

## Evidence numbers

- Test ids: `psg.ids` 121 -> 191; `tests-expected` 3398 -> 3538 over both presets (3536 executed, 2 skipped, 0 missing). Faults 174 -> 180; controls unchanged at 303.
- Plain-run times (median of 11, laptop): `psg` dev (ASan and UBSan) 0.185 s, release 0.054 s (plan targets 1.5 s and 0.3 s; no schedule uses noise period 1 beyond 80 steps; the longest vectors are the envelope period-zero row of 262,146 steps and the period-zero envelope perturbation of 131,072 steps).
- Gate cost (local, loaded laptop): `sh` `gate-time total=39`, `gate-cpu` user 69.9 + sys 58.9 = 128.7 (05-06: 111); `dash` `total=39`, 62.4 + 54.8 = 117.2 (05-06: 126); the two runs differ more from each other than from 05-06, so the rise is inside load noise; six faults at about 0.14 cpu-s each and 0.24 s of new suite time are the real adds. Container (emulated amd64, correctness only): `gate-time total=152`.
- `unknown-fields` in the profile record: 23 -> 25 (`psg.period_zero.noise` and `psg.noise.lfsr` are `unknown-default-chosen`).

## Deviations from Plan

**1. [Rule 3 - Blocking] Tasks 1 to 4 are one commit.** The pending lists, the profile inventory, the shared judge (`ivs_pe_eval`) and the registration rule (`fault-orphan-site`: a site must be registered where it appears) couple the noise, envelope, DAC and fault work; splitting them would have left red intermediate commits. Task 5 is its own commit. The three `.digests` and the consumer golden were red (steps 33/35) until Task 5, as the plan states.

**2. [Rule 1 - Plan precision] Shape rows are sampled per envelope state.** The level-seq of `psg.env.shape-N` lists 36 levels where level j is the sample at step 2 x EP x j (family notes of env.txt), not the sample of step j. The vector generator emits the stride from the row's own period writes; the judge reads level j at step stride x j. The restart and ampenv rows stay per step. No oracle file was edited.

**3. [Rule 2 - Missing critical functionality] The envelope vectors need a level before step 1.** An edge-spacing row of the env family counts changes against the level of step 1 (the family notes: "the first level for an envelope"), carried as `base_first`.

**4. [Rule 3] New seams.** `ivs_psg_state_reset(p, s)` takes the parameters (four callers); `ivs_memsys_psg_write_slot_internal` returns the register slot of a write by the write view's own decode, so the bus wrapper holds no address constant; the stand-in program ENV (`IVS_MS_PROG_ENV`, `IVS_MS_PROGS` 9) writes mixer, amplitude, period and the shape register for `psg.env.guest-shape-write-starts`.

**5. [Rule 2] Added `cmake/IvsPsgDacIndependence.cmake`** (not in the plan's file list): the plan's "perturbed-table test proves raw audio unaffected" has no engine parameter to perturb, so the probe stays a pure read of the one field and a second script proves that two profiles differing only in `psg.dac.levels` generate byte-identical core parameters. Added ids beyond the plan: `psg.profile.steps-per-np-ep`, `psg.profile.env-shape-bits` (the same proof for the other tagged values), `psg.dac.not-in-core` as a byte-pattern scan (the plan's grep-free wording), the three judge self-controls and the fixed cases listed above.

**6. [Other] `PSG_ENV_PERIOD_TIMES_16` must-pass id** is `psg.reseng.envelope-held-zero` (the plan's `psg.env.shape-0` also fails under the fault: a 16 times longer state leaves every shape row unchanged at its first level).

## Known Stubs

None. The two named stubs of plan 05-06 (noise constant 1, envelope level 0) are replaced by the generators.

## Open Items

- **AUD-01 is marked complete** by this plan (register file 05-04, tone 05-06, noise, envelope and tagged data 05-07: every register, tone, noise and envelope rule has a passing vector and every unknown parameter is tier-tagged profile data with perturbation proof). AUD-02 (write log, runner and consumer audio replay) stays Pending for plans 05-08 onward; C12, C69, C70, C71, C72 and C77 stay open (inference or default tags, D10 measurement items).
- `psg.mix.truth-table` is polarity-invariant (the 05-06 open item): not a detector for `PSG_MIXER_SENSE_INVERTED`; recorded only, the oracle is untouched.
- Conventions recorded, not decided by hardware: noise first-bits lists the output after each shift (second read `docs/psg/second-read-record-noise.md`); the envelope phase (first level change 2 x EP steps after the restart) follows POL-PSG-02 (`docs/psg/second-read-record-env.md`).
- The engine reads the 5-bit noise period and the 8-bit envelope period bytes with literal masks (`0x1F`, `0xFF`), data-sheet register widths equal to the profile's `psg.regmap` write masks; they are widths of the part, not tagged parameters.
- DAC: levels 8..1 are extrapolated and the printed 0.303 and 0.1515 V labels are not on a pure 3 dB ladder ([UNK], in the field note); no golden depends on the table.
- `psg.noise.lfsr` `width` is validated but not an engine parameter; `restart_on_write` 0 is implemented (no restart) but has no test beyond the field itself.
- Hosted cost unmeasured: the next hosted row decides whether the 280 cpu-s trigger is near (hosted gate wall was already near its 90 s trigger).
- Carried unchanged: 05-04 unobtained AY-3-8910 reprint, D10 and D11 items, Phase 4 items IN-04, IN-06, IN-09 to IN-11, the C73 tag wording of 05-05, the interrupt-entry gap of 05-06 (`psg.min_write_gap_ticks` 28 is published by 05-08).

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=35/35 controls=303/303 ... faults=180/180 ... tests-expected=3538 tests-executed=3536 tests-skipped=2 tests-missing=0`, 0 FAIL lines, `gate-time total=39`.
- `dash tools/gate.sh`: the same line, `gate-time total=39`, 0 FAIL lines.
- `sh tools/gate-linux.sh`: Ubuntu 24.04 container (gcc 13.3, cmake 3.28, dash): `gate: PASS steps=35/35 controls=303/303 ... faults=180/180 ... tests-expected=3538 tests-executed=3536 tests-skipped=2 tests-missing=0`, `cc=GNU`, 0 FAIL lines, `gate-time total=152`, exit status 0, `build/gate-linux.log` ends with the `gate: PASS` line.

## Threat Flags

None new. T-05-18 (a tagged value hard-coded in the engine): mitigated by the ten perturbation tests, the fault `PSG_NOISE_TAP_HARDCODED`, the generator refusals and both pending lists ending empty. T-05-19 (LFSR locked or non-maximal): mitigated by the generator refusing a zero seed and a mask outside the width, `psg.noise.lfsr-maximal-period` and the fault `PSG_NOISE_WRONG_TAP`. T-05-SC: no installs.

## Self-Check: PASSED

- Files exist: core/src/psg/psg_noise.c, psg_env.c; cmake/IvsPsgProfileProbe.cmake, IvsPsgDacIndependence.cmake; tests/psg/psg_noise.c, psg_env.c, psg_dac.c, psg_profile.c.
- Commits on HEAD: ec4ebfc, 1fd2ef9 (`commits: 2` measured from the ledger, `git rev-list --count 4fc7484..HEAD`).
