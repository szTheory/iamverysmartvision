---
phase: 08-stic-completion-save-state-performance-baselines
plan: 10
subsystem: Phase 8 wave B, plan 2 - the digest field table walker refactor - the snapshot class column on all 88 rows, the state-only walker filter, the internal state preimage seam, the pure input derive, the PSG state validator, seven test ids and the class-drift control
tags: [B, STA-01, walker, class-column, state-preimage, ivs_input_derive, ivs_psg_state_validate, D-11, D-15, D-16, D-44, T-08-26]
status: complete

requires:
  - phase: 08-stic-completion-save-state-performance-baselines
    provides: "plan 08-09 end state (the state oracle: step 38, the 88-row checklist, the schema literal and its pin, the IVSN v1 section lengths derived on paper, the Classes section SHA-256 e23d78160147e71915eca6c0b34da888ad34fe432652de18f36003a09b3eb179, the toy table already carrying class tokens); the blind-read record docs/state/checklist-second-read-record.md (state: closed, corrections: 0); counts at plan start steps=38/38 controls=377/377 faults=226/226 tests-expected=4772 tests-executed=4768 tests-skipped=4 (re-measured with a full gate run at the start, identical)"
provides:
  - "ivs_digest_field.snap (the former unused reserved member, same size, asserted) and IVS_SNAP_STATE 1, IVS_SNAP_IMAGE 2, IVS_SNAP_BOUND 3, IVS_SNAP_PAD 4 in core/src/machine/machine.h; the seventh argument of the row macro IVS_DF on all 67 literal rows and the seven rows of the slot macro (BOUND), equal to data/expected/state-checklist.ids (the state step compares them: the seventh-field branch is live on the real table)"
  - "the state-only filter of ivs_digest_domain (one line directly after the domain test, driven by a state_only flag of the sink that only ivs_machine_state_preimage sets) and the internal seam ivs_machine_state_preimage(m, domain, buf, cap): the IVSD frame and the STATE rows, lengths 35, 2,761, 615, 51, 63, 79, 51 and the bare 7-byte frame for IMAGES, equal to the section-bytes keys derived on paper in plan 08-09"
  - "ivs_input_derive(profile_index, words[4], out[2]) (pure, no instance) with ivs_input_rederive rewritten on top of it, and ivs_psg_state_validate(params, state) with the bounds documented with their proofs in core/src/psg/psg.h"
  - "seven test ids (machine.digest.class-column-valid, class-pad-iff-kind, class-image-rows, state-preimage-lengths, state-excludes-nonstate; input.derive.pure-equals-rederive; psg.state.validate) and the control state-class-source-drift (controls 377 to 378)"
affects: [08-11, 08-12, 08-13, 08-14, 08-15, 08-16, 08-17]

actuals:
  tokens: 16130
  tasks: 3
  commits: 3
plan_head_before: 949aaee4b72583cfd3f99f75d61825c724b697c7
plan_head_after: e49bd7874de6159ead0f7db7642a530cb1f8cb3a

key-decisions:
  - "The refactor is one filter and one flag: the walker skips a row whose class is not STATE only when the sink's state_only flag is set, the filter sits directly after the domain test and before every fault site, and only ivs_machine_state_preimage sets the flag. The eight IVS_FAULT_DIGEST_* sites of machine_digest.c are the same eight, in the same places (counted before and after); with the flag set the padding-writing site is never reached because the filter comes first, which is intended (that fault models the digest path)"
  - "The PSG validator carries three bounds beyond the plan's list because each is derived from the step code and provable: noise_counter below steps_per_np x max(0x1F, period_zero_noise), the level relation of a running envelope (env_held 0 implies env_level equals env_cycles for a rising direction and 15 minus env_cycles for a falling one), and env_held != 0 implies env_counter 0; the noise register bound is the bit length of (mask | seed) because the parameter struct carries no width"
  - "ivs_psg_state_validate is the validator of a powered instance: it refuses the all-zero state of an OFF instance (next_sample >= 1), consistent with D-14 (save is refused while OFF); the cross-row check obs.audio.emitted == next_sample - 1 is left to the code that sees both rows (plan 08-13)"
  - "No fault line of a DIGEST_* fault changed: the existing allowed prefix machine.digest. covers every new machine.digest.* id; six other fault lines got a justified collateral prefix (three input, three PSG), each with a comment, no must-fail column touched"

key-files:
  created:
    - tools/check/controls/state-class-source-drift/CHECK
    - tools/check/controls/state-class-source-drift/EXPECT
    - tools/check/controls/state-class-source-drift/overlay/core/src/machine/machine_digest.c
    - .planning/phases/08-stic-completion-save-state-performance-baselines/08-10-SUMMARY.md
  modified:
    - core/src/machine/machine.h
    - core/src/machine/machine_digest.c
    - core/src/machine/machine_input.c
    - core/src/psg/psg.h
    - core/src/psg/psg_state.c
    - tests/machine/machine_digest.c
    - tests/input/input_port.c
    - tests/psg/psg_bounds.c
    - data/expected/tests/machine.ids
    - data/expected/tests/input.ids
    - data/expected/tests/psg.ids
    - data/expected/faults/input.ids
    - data/expected/faults/psg.ids
    - data/expected/controls/state.ids
    - .planning/STATE.md
    - .planning/ROADMAP.md

requirements-completed: []

completed: 2026-11-14
---

# Phase 8 Plan 10: the digest field table walker refactor Summary

**The digest field table is now the single serializer of the snapshot, and nothing a digest reads has moved. Every one of the 88 rows carries its snapshot class (STATE 51, IMAGE 3, BOUND 28, PAD 6) in the former unused `reserved` member, now `snap`, written as the seventh argument of the row macro and held equal to the hand-derived checklist by the gate step `state`; the walker has one new filter line, a state-only flag of the sink, so the new internal seam `ivs_machine_state_preimage` yields the IVSD frame and the STATE rows only, with exactly the lengths derived on paper in plan 08-09 (35, 2,761, 615, 51, 63, 79, 51; a bare 7-byte frame for IMAGES). The digest path leaves the flag at 0: `data/expected/digest-v1-literals.ids`, `digest-v2-literals.ids`, every `machine.digest.*` id, the 27 goldens and the consumer golden are untouched and green, and the eight `IVS_FAULT_DIGEST_*` sites are the same eight in the same places. Two pure helpers for the restore of plan 08-13 exist and are proven: `ivs_input_derive` (equal to the literal's OR of masks and to what `ivs_input_rederive` stores, over 1,101 word sets on both ports) and `ivs_psg_state_validate` (every bound derived from the chip step functions, shown true of the reset state, a grid of register programs, 350,459 steps of 52 engine vectors and 6,000 units of each of the nine stand-in programs, seen at its literal bound, and refused one member past it). Seven test ids and one control were added with no fault, step or golden. STA-01 and STA-02 stay Pending. The gates pass under sh, dash and the Ubuntu container with the plan-end counts `steps=38/38 controls=378/378 faults=226/226 fault-runs=226 tests-expected=4786 tests-executed=4782 tests-skipped=4 tests-missing=0 goldens=27 state-rows=88`.**

## Commits

| Commit | Task | What |
|--------|------|------|
| `c71e26e` | 1 (tracer) | the class column on 88 rows, `IVS_SNAP_*`, the state-only filter and flag, `ivs_machine_state_preimage`, ids class-column-valid and state-preimage-lengths, control state-class-source-drift (controls 377 to 378, ids 4772 to 4776) |
| `320b23e` | 2 | ids class-pad-iff-kind, class-image-rows, state-excludes-nonstate (4776 to 4782); tests and inventories only, `git show --stat HEAD -- core` is empty |
| `e49bd78` | 3 | `ivs_input_derive`, `ivs_psg_state_validate`, ids input.derive.pure-equals-rederive and psg.state.validate (4782 to 4786), six collateral allowed prefixes |
| the commit that adds this file | | this SUMMARY, STATE.md and ROADMAP.md |

## Corrections of the blind read

`no corrections`. The precondition was checked by command before the first edit: `head -1 docs/state/checklist-second-read-record.md` is `state: closed`, `corrections: 0`, no `correction:` line, so the first commit `docs(08-10): apply the blind-read corrections` was not made and the checklist, the schema literal, the pinned schema digest `6cd4932f5ebf0b0fc2377555ca079d0313bb27a4df1a4fda9e15f37c6eb1d193` and `ivsn-literals.ids` are untouched (`git diff 949aaee..HEAD -- data/state data/expected/state-checklist.ids data/expected/ivsn-literals.ids docs` is empty). The SHA-256 of the Classes section of `docs/formats/ivsn-v1.md` is still `e23d78160147e71915eca6c0b34da888ad34fe432652de18f36003a09b3eb179`, the value pinned in the 08-09 SUMMARY.

## The tripwires (Task 1 step 6, recorded)

- `git diff --stat 949aaee -- data/expected/digest-v1-literals.ids data/expected/digest-v2-literals.ids` prints nothing; the verify `git diff --quiet <record commit> -- <both files>` passes.
- The whole `machine` suite (410 ids at the start, 412 after Task 1, 415 after Task 2) and every golden pass unchanged; no `.digests`, golden or consumer file is in `git diff --name-only 949aaee..HEAD`; the only files of `data/` that changed are the six inventories (tests machine, input, psg; faults input, psg; controls state).
- `grep -c 'IVS_FAULT_DIGEST' core/src/machine/machine_digest.c` is 8 at plan start and 8 at plan end: no fault site added, moved or removed. `grep -c 'IVS_SNAP_' core/src/machine/machine_digest.c` is 77 (at least 74 required).
- The DIGEST_* faults still fail their must-fail ids (the gate ran all 226 faults at every task); no allowed-prefix change was needed for them because every one of their lines already allows `machine.digest.`.
- The refactor was also probed the other way: a class edit in the real table fails the real `state` step (`state-class-drift: cpu.status (source BOUND checklist STATE)`), shown on a scratch copy, so the seventh-field branch that the toy tree exercised since plan 08-09 is live on the real table.

## What each new test can fail on (mutation probes, run in the working tree and reverted)

| Probe | Failing ids |
|-------|-------------|
| `mem.exec` classed STATE | class-column-valid, class-image-rows, state-excludes-nonstate, state-preimage-lengths |
| filter line removed | state-excludes-nonstate, state-preimage-lengths |
| `slot.*.sha256` classed STATE | class-column-valid, state-excludes-nonstate, state-preimage-lengths |
| `cpu.reserved` (a PAD row) classed STATE | class-column-valid, class-pad-iff-kind, state-excludes-nonstate |
| `time_ticks` classed BOUND | class-column-valid, state-preimage-lengths |
| PSG: tone bound `>` for `>=`; noise bound likewise; held-implies-counter dropped; level relation dropped; register width too small; tone bound one too tight | psg.state.validate (each at its own check) |
| derive: the mask byte cut to 7 bits | input.derive.pure-equals-rederive (word 00000001 on port 0: derive 01, the literal 81) and, by the same table, the anchor and single-control ids |

`machine.digest.state-excludes-nonstate` also proves its own detectors can fail: before judging the state preimages it requires the image detector to find the recognisable windows in the digest preimage of MEM and the slot detector to find the slot words and sha256 bytes in the digest preimage of IMAGES.

## The PSG validator: each bound and the code that makes it true

`ivs_psg_state_validate(params, state)`: IVS_OK, IVS_E_BAD_STATE, IVS_E_BAD_ARG for NULL. The same table is in `core/src/psg/psg.h`.

| Bound | Why it is true of every reachable state |
|-------|------------------------------------------|
| `next_sample >= 1` | reset sets 1 and a chip step only adds 1 (psg_sample.c); the all-zero state of an OFF instance is refused on purpose (save is refused while OFF, D-14) |
| `tone_out[i]`, `env_dir`, `env_held` 0 or 1 | reset values and the `^= 1` toggles (psg_tone.c), the attack bit and `^= 1` (psg_env.c) |
| `tone_counter[i] < max(0xFFF, period_zero_tone)` (4,096 in the NTSC profile) | a step reloads at most the 12-bit period (the coarse register is masked to 4 bits) or `period_zero_tone` for a written 0, then decrements once in the same step (psg_tone.c) |
| `noise_counter < noise_steps_per_np x max(0x1F, period_zero_noise)` (64) | the 5-bit period times the steps per unit, minus the decrement of the same step (psg_noise.c); not in the plan's list, derived and proven |
| `noise_lfsr < 2^b`, b the bit length of the OR of `galois_mask` and `seed` (2^17 for mask $12000, seed 1) | the Galois step shifts right and XORs the mask, so a value below 2^b stays below it; the reset loads the seed (psg_noise.c). The parameter struct carries no register width, so the bound is derived from the mask and the seed |
| `noise_lfsr != 0` | only when the seed is nonzero and has no more bits than the mask: the mask then has its top bit inside the register, so the step is a bijection of [0, 2^b) with 0 mapping to 0 and no nonzero value reaches the lock-up value; for any other profile this bound is left out (a seed wider than the mask can reach 0, shown by hand with mask 3 and seed 7) |
| `env_counter < env_steps_per_ep x max(0xFFFF, period_zero_env)` (131,072) | the 16-bit period times the steps per unit, minus the decrement (psg_env.c) |
| `env_level <= 15`, `env_cycles <= 15` | the restart loads level 0 or 15 and cycle 0, a cycle has 16 states (psg_env.c) |
| `env_held == 0` implies `env_level == (env_dir ? env_cycles : 15 - env_cycles)` | a restart and every advance, inside a cycle or into a new cycle, keep it; only the end of a holding cycle sets the held flag and only a restart clears it; not in the plan's list, derived and proven |
| `env_held != 0` implies `env_counter == 0` | the held flag is set only by the advance that follows a decrement to 0 (or by reset), and a held generator does not step; not in the plan's list, derived and proven |
| `reserved[0..2] == 0` | never written |

Left out, with the reason: an upper bound of `next_sample` (machine time is not in this state; the cross-row check `obs.audio.emitted == next_sample - 1` belongs to the restore code that sees both rows, plan 08-13), and the all-zero-lock-up check for a profile whose seed is wider than its mask (shown reachable by hand).

`psg.state.validate` runs, with a validation after every step or unit: the reset state; 576 grid walks (16 shapes x 3 envelope periods x 4 tone period sets x 3 noise periods, 160 steps) and two long walks (140,000 steps past the first envelope state change at 131,072 steps, 9,000 steps at the other extremes); 52 engine vector rows of the families ampenv, env, noise, reseng and tone replayed one chip step at a time (350,459 steps); and 6,000 units of each of the nine stand-in programs on a full machine. The maxima seen equal the literal bounds minus one (tone 4,095, noise 63, env 131,071, level 15, cycle 15), so the bounds are met and not only respected. Each bound is then refused by a one-member perturbation of a valid mid-cycle base state and accepted at its edge.

## The pure input derive

`ivs_input_derive(profile_index, words[4], out[2])`: the OR of the line masks of every set bit for each port below the profile's controller count, 0 for the others, an index at or above `ivs_input_profile_count()` gives two zero masks; no instance read or write. `ivs_input_rederive` calls it and stores the two bytes (the `CTRL_PORTS_SWAPPED` site stays in the store loop, `CTRL_UNION_AS_AND` in `ivs_input_pins`, `CTRL_TABLE_SHORT` in the count). `input.derive.pure-equals-rederive` compares, over 1,101 words (zero, 31 singles, 465 pairs, every control closed, the reserved bit alone and with a control, all ones, 600 pseudo-random) crossed over both ports, the derive output with the literal's OR of masks (the independent oracle of the suite) and with the masks an instance holds after `ivs_input_rederive`, with a whole-instance memcmp around each call and zeros for three out-of-range profile indices. All input and run ids and goldens pass unchanged.

## Allowed-prefix additions (Task 2 and 3, each with a comment in the fault file)

| Fault line | Prefix added | Why the new id fails there |
|------------|--------------|----------------------------|
| CTRL_PORTS_SWAPPED (input) | `input.derive.` | the instance stores the masks swapped, derive and the instance disagree |
| CTRL_UNION_AS_AND (input) | `input.derive.` | closed controls combine by AND, derive disagrees with the literal's OR |
| GEN_CTRL_MUTATION (input) | `input.derive.` | one generated line mask is flipped, derive disagrees with the literal |
| PSG_TONE_PERIOD_OFF_BY_ONE | `psg.state.` | a tone counter reloads P + 1, a state past the 12-bit bound is reached |
| PSG_ENV_PERIOD_TIMES_16 | `psg.state.` | the envelope counter reloads 16 times too high, past its bound |
| PSG_NOISE_RATE_USES_TONE_DIVISOR | `psg.state.` | the noise reload is halved, the counter never reaches the literal bound |

No DIGEST_* line changed (the prefix `machine.digest.` was already there) and no must-fail column was touched; faults stay 226/226 and fault-runs 226. The first Task 3 gate run showed exactly these six `fault-wrong-reason` lines and nothing else.

## Deviations from Plan

**1. [Interpretation] Lamp Tone is not available to the psg suite (Task 3).** The plan's reachable-state proof names the Lamp Tone idle and pass runs; those are runner-level scenarios whose images are not part of the unit suites (the psg suite has no packed images). The proof runs instead the engine vectors the suite already judges (52 rows, one chip step at a time) and the nine stand-in programs of the machine families on a full machine, plus the grid and the long runs. The state of the real lamp runs reaches the validator in the in-process round trip of plan 08-12; a false bound would show there as a refused snapshot (open item below).

**2. [Interpretation] "2^k combinations" of controller bits (Task 3).** The literal names 31 controls, and 2^31 words per controller cannot be enumerated; the derivation is an OR over bits, so the test checks the zero word, all singles, all 465 pairs, the all-closed word, the reserved-bit words and 600 pseudo-random words, on both ports in rotated combinations, against the literal.

**3. [Interpretation] Three more PSG bounds than the plan listed (Task 3).** The noise counter, the level relation and held-implies-counter-0, each derived from the step code and each with a test (accepted on every reachable state, refused one step past). The plan allows leaving out only what cannot be proven; these were proven, so they are in.

**4. [Information] The toy table needed no edit.** The plan lists `tools/check/controls/_good/tree/core/src/machine/machine_digest.c` in `files_modified` and says the pristine toy table "now carries class tokens"; plan 08-09 already put them in (its deviation 3), so only the overlay of the new control was needed. The control's EXPECT is the longer line `FAIL state-class-drift: toy.counter (source BOUND checklist STATE)`, which contains the plan's token and also names the row and both classes.

**5. [Information] Test content.** `machine.digest.state-excludes-nonstate` loads stand-in content A (`ivs_ms_content`) and then overwrites every image array and slot record with a recognisable pattern (an odd-multiplier sequence, so a word value names its index and a window is found in linear time), rather than relying on the stand-in bytes. No file beyond the plan's lists changed.

No auth gates, no package installs (T-08-SC), no `gh` call, no push, no tag, no `tools/ci-record.sh`; `.planning/config.json` and `.planning/state.json` were never staged.

## Decisions Made

- The class is data of the row, not a second table: one source for the digest, the snapshot and the checklist comparison.
- The filter order (domain test, class filter, then the fault sites) is what keeps the digest-path faults meaning what they meant.
- The state preimage seam is internal and has the digest seam's contract (full length, 0 for NULL or an unknown domain, no handle check), so plan 08-11's writer can be tested against the pinned lengths before the container exists.

## Cost (laptop)

| Item | Value |
|------|-------|
| estimate of the plan | 110,000 tokens (raw, confidence low), 3 tasks |
| actuals | 16,130 tokens (chars/4 over the added lines of the three task commits outside `.planning/`), 3 tasks, 3 commits (measured from the ledger of the plan head) |
| gate total, sh, at plan start | 81 s |
| gate total, sh and dash, Task 1 tree | 86 s and 83 s, identical summary line |
| gate total, sh and dash, Task 2 tree | 108 s (run while the Task 1 container gate was still running) and 90 s, identical summary line |
| gate total, sh and dash, Task 3 tree | 82 s and 78 s, identical summary line |
| container gate, Task 1 tree | 357 s (gcc 13.3.0, cmake 3.28.3, x86_64 emulated), exit 0, 0 FAIL lines, `tests-expected=4776 tests-executed=4772` |
| container gate, Task 3 tree | 325 s (gcc 13.3.0, cmake 3.28.3, x86_64 emulated), exit 0, 0 FAIL lines, `tests-expected=4786 tests-executed=4782` |

The plan adds seven test ids and one control: roughly 7 x 2 presets of test runs and 0.07 local cpu-s for the control, with the `machine` suite re-run under about 70 machine faults (the new machine ids are cheap: the heaviest, `state-excludes-nonstate`, walks 88 rows twice over seven small preimages, and `psg.state.validate` replays about 600,000 chip steps in well under a second). The gate total did not move beyond measurement noise (81 s at the start, 82 s at the end).

## Open Items

- **Plan 08-11:** the writer must produce the section lengths 35, 2,761, 615, 51, 63, 79, 51 from `ivs_machine_state_preimage` (already equal by test) and the schema digest `6cd4932f5ebf0b0fc2377555ca079d0313bb27a4df1a4fda9e15f37c6eb1d193` from the table descriptors (now with the class column in the source, the step compares it); the state preimage seam is the section body, so the writer copies it.
- **Plan 08-12 / 08-13:** feed the validator real lamp states. The in-process round trip on Lamp Tone idle and pass is the first time a snapshot of a lamp run reaches `ivs_psg_state_validate`; a refusal there means a bound is false and the bound must be dropped, never the snapshot. The extra STIC rule `event_index` below the profile's event count is applied by the snapshot code with the profile parameters (the CPU and STIC validators are unchanged by this plan). The cross-row check `obs.audio.emitted == next_sample - 1` is for the restore code.
- **Information.** D10 and D11 are not performed; no claim above provisional exists; no hosted-only check was run; STA-01 and STA-02 stay Pending until the serializer, restore and the hard-point suite exist; REL-01 stays Pending (D-28).

## Interfaces for the next plans

- `ivs_machine_state_preimage(const struct ivs_machine *m, uint32_t domain, uint8_t *buf, size_t cap)` in `machine/machine.h`: the section bytes of a snapshot (internal, never public).
- `ivs_digest_field.snap` and `IVS_SNAP_*`: the class of each row, readable through `ivs_machine_digest_field(i)`; a BOUND row is never written by a restore, a PAD row is zeroed, an IMAGE row is never touched.
- `ivs_input_derive(profile_index, words, out)` in `machine/machine.h` and `ivs_psg_state_validate(params, state)` in `psg/psg.h`.
- Gate counts after this plan `steps=38/38 controls=378/378 faults=226/226 fault-runs=226 tests-expected=4786 tests-executed=4782 tests-skipped=4 tests-missing=0 goldens=27 state-rows=88 state-cuts=18` (the next state plans add ids against the pending list of `data/state/tests-pending.ids`).
- Threat id T-08-26 is used here (T-08-SC throughout); the next state plans continue from T-08-27 or the numbering their plans assign.

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=38/38 controls=378/378 ... faults=226/226 ... fault-runs=226 ... tests-expected=4786 tests-executed=4782 tests-skipped=4 tests-missing=0 ... goldens=27 ... state-rows=88 state-cuts=18 (0 FAIL lines)
dash tools/gate.sh:    the identical summary line (0 FAIL lines)
GATE_LINUX_PLATFORM= sh tools/gate-linux.sh: exit 0 on the Task 3 tree, build/gate-linux.log ends gate: PASS (gcc 13.3.0, cmake 3.28.3, x86_64 emulated), the plan-end counts, 0 FAIL lines
```

After the Task 1 tree: sh and dash `controls=378/378 tests-expected=4776 tests-executed=4772`, container exit 0 (357 s). After the Task 2 tree: `tests-expected=4782 tests-executed=4778` under sh and dash. After the first Task 3 run the gate showed exactly the six `fault-wrong-reason` lines of the table above (and nothing else) and the allowed prefixes were added; the second run passed. On the closing tree (this SUMMARY, STATE.md and ROADMAP.md added or edited; `tracked=2433`): sh (91 s) and dash (82 s) `gate: PASS` with the identical summary line (md5 2bf115eb6cf8b4f5a958fff3855215e9), 0 `FAIL` lines, and the container gate was run again (`GATE_LINUX_PLATFORM= sh tools/gate-linux.sh`): exit 0, `gate: PASS` with the plan-end counts, 0 `FAIL` lines, gate-time total=326 s (gcc 13.3.0, cmake 3.28.3, x86_64 emulated); `build/gate-linux.log` holds that run. The dev-preset `ctest` (20 tests) and the release-preset suites machine (415), input (124) and psg (228) pass.

## Known Stubs

None.

## Threat Flags

None new. T-08-26 (an edit of the shared table or walker silently moves all nine digests, or the class column puts image bytes into snapshots): the digest-v1 and v2 literal files, every `machine.digest.*` id, the 27 goldens and the consumer golden are unchanged and green; the DIGEST_* fault sites are counted before and after (8 and 8) and keep their place after the filter; `machine.digest.state-excludes-nonstate` proves no 16-byte image window, no slot sha256 and no slot word is in any state preimage, with detectors shown able to find them; `state-class-drift` fails a class edit of the source alone (control and real-table probe). T-08-SC: no installs.

## Self-Check: PASSED

Verified before the final commit: the SUMMARY and the created files of the plan exist (FOUND for the three control files, the SUMMARY, `core/src/machine/machine_digest.c` and `core/src/psg/psg_state.c`); commits `c71e26e`, `320b23e` and `e49bd78` are ancestors of HEAD and `git rev-list --count 949aaee4b72583cfd3f99f75d61825c724b697c7..HEAD` printed 3 before this file's commit; nothing is staged but the files of this commit and `.planning/config.json` and `.planning/state.json` are not staged; `git ls-files` lists no `.ivsc`, `.bin`, `.rom`, `.exec`, `.ivsn`, `.wav`, `.ppm` or `.vraw` file; the diff of the three task commits holds no home, scratch or temp path and no "Intellivision"; task 2's commit touches nothing under `core/` (the stat filter prints 0); no commit deletes a file; `grep -c IVS_FAULT_DIGEST core/src/machine/machine_digest.c` is 8 (as at the start), `grep -c IVS_SNAP_` is 77, `grep -c ivs_machine_state_preimage core/src/machine/machine.h` is 1, and the two new test ids of task 3 are each registered once (`grep -c` over the two id files prints 2 non-zero lines); the five machine class and state ids are registered once each.
