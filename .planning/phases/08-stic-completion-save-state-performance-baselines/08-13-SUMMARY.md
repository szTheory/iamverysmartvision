---
phase: 08-stic-completion-save-state-performance-baselines
plan: 13
subsystem: Phase 8 wave B, plan 5 - the IVSN v1 restore - ivs_machine_snapshot_restore, the status IVS_E_STATE_MISMATCH (-24) and the reasons 72 to 81, the refusal matrices of every class, the poisoned-target equivalence, the atomicity sweep, the derived invariants decided by real programs, three named faults and cost checkpoint B
tags: [B, STA-01, STA-02, restore, ivsn-v1, state-mismatch, reasons-72-81, invariants, poisoned-target, atomicity, checkpoint-B, D-14, D-15, D-16, D-37, D-41, D-44, T-08-31, T-08-32, T-08-33]
status: complete

requires:
  - phase: 08-stic-completion-save-state-performance-baselines
    provides: "plan 08-12 end state (the instance-free reader, ivs_snapshot_describe, reasons 58 to 71, the heap-exact idiom, the 406-row matrix); counts at plan start steps=38/38 controls=378/378 faults=231/231 tests-expected=4832 tests-executed=4828 tests-skipped=4 api-calls=18 api-rows=52 api-statuses=14 pinned-defines=110 public-symbols=18 (re-measured with a full gate run at the start, identical)"
provides:
  - "core/src/machine/machine_snapshot_restore.c: the public ivs_machine_snapshot_restore (validate everything into a probe, then a table-driven commit that cannot refuse), the invariants INV-1 to INV-6 as one function of the probe and the profile (ivs_snapshot_invariants, also judging a live instance through ivs_snapshot_probe_from_instance) and the pure profile id comparison ivs_snapshot_profile_id_matches; the report writers of the reader are shared (ivs_snap_report_refusal, ivs_snap_report_success)"
  - "IVS_E_STATE_MISMATCH (-24) in one change with the reserved band narrowed to -25..-31 (api-effects check, header, pin, status name and its test, the control api-status-reserved retargeted by its overlay only), the reasons 72 to 81 pinned (pinned-defines 110 to 120) and the call record of restore with nine refusal rows (api-calls 18 to 19, api-rows 52 to 61, api-statuses 14 to 15, public-symbols 18 to 19)"
  - "tests/state/state_util.c and .h (shared by the later state plans): boot and scenario instance builders over the build-tree images (ivs_state_images.inc), state_poison / su_poison through the field table, the nine digests, resealing, the row machinery and runner of the refusal matrices; tests/state/state_restore.c (13 ids) and state_invariants.c (1 id); 16 test ids (4832 to 4864) and 3 faults (231 to 234)"
  - "the checkpointB-08 entry of .planning/CI-EFFICIENCY.md"
affects: [08-14, 08-15, 08-16, 08-17]

actuals:
  tokens: 48731
  tasks: 3
  commits: 4
plan_head_before: c57328af2286d1fa327c86e63f50c6e0f9c33342
plan_head_after: e0754ab531babf3a119206611dad1e78171ce827

key-decisions:
  - "Validate everything into a probe, then commit with a pass that has no refusal path: the probe holds the CPU, PSG and STIC structs, the input row and the few scalars the invariants read (about 800 bytes); the decoder is the digest field table itself, one walk in two modes (the rows of a domain in table order, big-endian, a u32 count before an array row); the class column says what the commit does (STATE decoded, PAD zeroed, BOUND and IMAGE never written)"
  - "The check order is the specification's: arguments, handle, overlap (81), the reader (60 to 71), profile (72, 73, 74), slots (the record range 78, then 75 and 76), lengths (80), counts (77), frames and ranges (78), derived invariants (79), commit. INV-1 and INV-5 are range rules (78) as the plan text lists them with the validators; INV-2, INV-3, INV-4 and INV-6 are the derived rules (79)"
  - "None of the six candidate invariants is dropped: INV-2 is corrected (the reset state, machine time 0 with the cursor at the very start, where the first event is not yet applied because the timeline is caught up at unit ends, violates the wording of the specification and is reachable) and INV-3 is strengthened (the previous-level rule is the stricter one derived from the step code, three four-bit levels and a fourth byte of 0; no reachable state forced it). The candidate obs.video.count == stic.frame_index is decided true and kept. 411,075 unit boundaries of 23 programs hold all six"
  - "A refusal writes only the report, as describe does (reason, byte_offset, section_index, the readable header facts and, for a slot, kind); an unusable report struct and a report that overlaps the instance write nothing, an invalid handle writes IVS_REASON_HANDLE; a success writes the full v1 report exactly as describe does. The digest fields of the header are informational (D-41) and the trailer is a checksum, not a proof"
  - "The snapshot record of a slot must be one a writer makes (loaded 0 or 1, an empty record empty) and the frame of every section is checked: both are needed for the canonical re-encode rule of the corruption family, and both are reason 78"
  - "Cost checkpoint B reads the median of the last five hosted rows (7.98) because a recorded row already contains wave B so far, against the budget 7.8, the 8.0 reading and the line 8.3: checkpointB-08 no-lever, with the margin to 8.0 stated in the entry"

key-files:
  created:
    - core/src/machine/machine_snapshot_restore.c
    - tests/state/state_util.c
    - tests/state/state_util.h
    - tests/state/state_restore.c
    - tests/state/state_invariants.c
    - .planning/phases/08-stic-completion-save-state-performance-baselines/08-13-SUMMARY.md
  modified:
    - core/CMakeLists.txt
    - core/include/ivs/ivs_state.h
    - core/include/ivs/ivs_status.h
    - core/src/machine/machine.c
    - core/src/machine/machine.h
    - core/src/machine/machine_snapshot.c
    - core/src/machine/machine_snapshot_read.c
    - tools/check/api-effects.cmake
    - tools/check/controls/api-status-reserved/overlay/core/include/ivs/ivs_status.h
    - tools/check/controls/api-status-reserved/overlay/data/expected/api-statuses.ids
    - data/expected/api-statuses.ids
    - data/expected/api-reasons.ids
    - data/api/api-effects.json
    - data/expected/tests/state.ids
    - data/expected/faults/state.ids
    - data/state/tests-pending.ids
    - tests/CMakeLists.txt
    - tests/machine/machine_create.c
    - tests/state/state_cases.h
    - tests/state/state_main.c
    - docs/formats/ivsn-v1.md
    - docs/state/hidden-state-checklist.md
    - .planning/CI-EFFICIENCY.md
    - .planning/STATE.md
    - .planning/ROADMAP.md

requirements-completed: []

completed: 2026-11-14
---

# Phase 8 Plan 13: the IVSN v1 restore Summary

**A snapshot can be put back. `ivs_machine_snapshot_restore(m, buf, len, rep)` checks the container with the reader of plan 08-12, then the identity (the profile id registered and equal to the instance's, the parameter digest of this build, the three image slots), then the content (every section length, array count and frame, the CPU, PSG and STIC states through their validators, the phase POWERED and machine time at most 2^63, the derived invariants), and only then commits: the STATE rows of the digest field table are decoded into the instance, the PAD rows are zeroed, and no BOUND or IMAGE row (the handle, the profile, the committed image structure, the slot records, the image arrays) is written. The sections are decoded into a probe of about 800 bytes first, so a refusal leaves every byte of the instance as it was and writes only the report. The new status `IVS_E_STATE_MISMATCH` (-24) is added in one change with the narrowed reserved band (-25..-31), the ten reasons 72 to 81 are pinned, and 9 refusal rows record the call. The matrices are written from the specification: 180 table rows against four targets (the container structure through restore, the profile, the three slots, the content ranges, counts and lengths, the invariants) and 20 argument rows, each run twice into a poisoned 400-byte report compared whole with what the contract allows, the whole target instance byte-identical afterwards, and repeated against poisoned non-trivial targets with the nine digests. Restoring into a target whose STATE and PAD rows hold a poison byte (two different bytes) equals the source byte for byte at 13 cuts of the boot program, the Lamp Tone and two diagnostics. The derived invariants were held against the live instances of 23 real programs at all 411,075 unit boundaries: no invariant is dropped, INV-2 needed one exception (the reset state) and INV-3 a stricter previous-level rule, and the candidate obs.video.count == stic.frame_index is kept. Three named faults each fail the id named for them. The cost checkpoint reads the median of the last five hosted rows, 7.98: `checkpointB-08: no-lever`. The gates pass under sh, dash and the Ubuntu container with the plan-end counts `steps=38/38 controls=378/378 faults=234/234 fault-runs=234 tests-expected=4864 tests-executed=4860 tests-skipped=4 tests-missing=0 api-calls=19 api-rows=61 api-statuses=15 pinned-defines=120 public-symbols=19 hdr-alone=10/10 prefix-files=11 pkg-selftest=17/17 consumer-lines=135`. STA-01 and STA-02 stay Pending until the hard-point suite of plan 08-14 exists.**

## Commits

| Commit | Task | What |
|--------|------|------|
| `3345854` | 1 (tracer) | restore end to end, `IVS_E_STATE_MISMATCH` and the band, the reasons 72 to 81, the call record, `state_util.c`, 9 ids (the round trip fixed point, arguments, structure, profile, slots, validators, invariants, the profile id seam, reasons covered), 2 faults (ids 4832 to 4850, faults 231 to 233, api-calls 18 to 19, api-rows 52 to 61, api-statuses 14 to 15, pinned-defines 110 to 120, public-symbols 18 to 19) |
| `ff9b441` | 2 | the poisoned-target equivalence, same-profile-any-instance, sets-powered, digest-fields-informational, atomic-every-refusal, the judge, the reachability of the invariants, the third fault, the state image include and the scenario builders (7 ids, ids 4850 to 4864, faults 233 to 234) |
| `ed4d466` | 2 (fix) | the reachability id kept its halting, faulting and Lamp Tone input programs whole in a fault run (it failed in every fault build for a reason unrelated to the fault: see deviation 7); no id, count or core file changed |
| `e0754ab` | 3 | the as-built appendix of the specification and the invariant note of the checklist, the checkpoint B entry |
| the commit that adds this file | | this SUMMARY, STATE.md and ROADMAP.md |

## Restore and its check order

| Step (specification) | Check | Reason, status | Byte offset reported |
|---|---|---|---|
| 1 | the report struct (not NULL, struct_size below 360 or above 4,096) | BAD_ARG, nothing written | n/a |
| 1 | a NULL buffer with a length above 0; then the report overlapping the first min(len, 16,384) bytes of the buffer | 58, 59 BAD_ARG | 0 |
| 1 | a NULL or invalid handle | BAD_HANDLE, the report gets IVS_REASON_HANDLE | 0 |
| 1 | the buffer or the report overlapping the storage of the valid instance (the report is not written when it is the one that overlaps) | 81 BAD_ARG | 0 |
| 2 to 6 | the container, exactly as describe | 60 to 71 | as plan 08-12 |
| 6 | the id registered; equal to the instance's; the parameter digest equal to this build's | 72 UNKNOWN_PROFILE; 73, 74 STATE_MISMATCH | 56; 56; 120 |
| 7 | per slot EXEC, GROM, cartridge: the flag at most 1 and an empty record empty; loaded in the snapshot only; loaded in the instance only or another SHA-256 or length | 78; 75 MISSING_SYSTEM_IMAGE; 76 STATE_MISMATCH | the field of the slot record, kind 1 to 3 |
| 8 | every section length equal to the derived length | 80 | the length field of the table entry |
| 8 | every array row count equal to the table's | 77 | the count prefix |
| 8 | frames, validators, phase, machine time | 78 | the frame byte; the first element of the struct or scalar |
| 8 | INV-2, INV-3, INV-4, INV-6 | 79 | the first element of the STIC state, the PSG state, the input row, the video count |
| 9 | the commit | cannot refuse | n/a |

The IMAGES domain digest of step 7 of the specification is not compared: a snapshot has no IMAGES section, so there is nothing recorded to compare with the live structure (the slot records are its whole image structure).

## The validator rules and the invariants: the row that exercises each and the reason it produced

Every row below is resealed (the trailer is recomputed), so it reaches the check it names; every row is refused with the exact status, byte offset and section index, the whole instance unchanged.

| Rule | Row (family `content-validators` unless said) | Reason |
|---|---|---|
| CPU status bits outside the mask | cpu.status 0x01, 0x08 | 78 at the CPU struct |
| CPU run_state, fault_reason above their last value | cpu.run_state 5 and 255, cpu.fault_reason 5 | 78 |
| CPU boolean members | cpu.intff, dbd, prev_interruptible, halt_pending, busak = 2 | 78 |
| CPU cross-field rules | FAULTED without a reason; a reason while RUNNING; RESET_PENDING with INTFF; UNRESET with a grant; a grant while FAULTED; DBD with prev_interruptible; DBD while HALTED; a stop latch while HALTED | 78 |
| CPU reserved bytes | not reachable: the PAD rows are never decoded, so the probe holds zero | n/a |
| PSG next_sample at least 1; the tone, noise and envelope counter bounds; the noise register width and its lock-up value; env_level and env_cycles at most 15; the flag members; the running-envelope level relation; a held envelope's counter | psg.next_sample 0; tone_counter 4,096 and 0xFFFFFFFF; tone_out 2; noise_counter 64; noise_lfsr 2^17 and 0; env_counter 131,072; env_level 16; env_cycles 16; env_dir 2; env_held 2; a running envelope whose level is not its cycle; a held envelope with a counter | 78 at the PSG state |
| STIC event cursor, display latch, windows flag, mode, color-stack pointer | stic.event_index 16 and 0xFFFFFFFF; display_latch 2; windows_active 2; mode 2; cs_ptr 4 | 78 at the STIC state |
| INV-5, the phase and the range of machine time | phase 0 and 2; time_ticks 2^63 + 1 and 2^64 - 1 | 78 at the scalar |
| the frame of a section | 7 bytes of the CPU section and one byte of each other section | 78 at the byte |
| array counts (77) | the first array row of each of the 7 sections with the table's count + 1 and 0, cpu.r with 0xFFFFFFFF and 1, and two wrong counts (the first in section order is reported) | 77 at the count prefix |
| section lengths (80) | 8 chained pairs of table lengths that keep the sum (one section a byte long, the next a byte short, and the other way for three) | 80 at the length field of the first deviating entry |
| INV-2, gates and timeline (`content-invariants`) | the register gate flag and the graphics gate flag flipped; gate 0 cleared; gate 3 set; frame 3 with a video count of 3; the cursor one event ahead and one behind | 79 at the STIC state |
| INV-3 | next_sample one more and one less than the machine time says; next_sample and emitted both one more (only the machine time disagrees); emitted one more; a previous level of 16; a fourth byte of 0x01 and 0x10; registers 0, 4 and 10 holding a bit their write mask drops | 79 at the PSG state |
| INV-4 | the reserved bit of a word; a word of the third and the fourth controller; a held key with the pull-downs of no key; a pull-down with no key held (both ports); the input floor above machine time | 79 at the input row |
| INV-6 | the video count one more, one less and 2^64 - 1 | 79 at the video count |
| INV-1 | the CPU, PSG and STIC rows above (the validators and the event cursor) | 78 |

Counts of the family tables: structure 17 rows, profile 18, slots 37, content 81 (validators, scalars, frames, counts, lengths) and invariants 27, 180 in all, each run against the target its row names (powered; powered without a cartridge; OFF without EXEC; OFF without GROM), plus 20 argument rows written as code (the report: unusable, NULL, over the buffer, just after it; the buffer: NULL, empty, over the instance in five places, ending and beginning where the instance does, a length of 2^40; the report over the instance; a NULL, destroyed and damaged handle). The event cursor against the profile's event count (INV-1) is the same bound as the validator's own limit for the registered profile (16 events, the table maximum), so its rows are the STIC rows above.

## The invariants, decided by programs

`state.restore.invariants-hold-reachable` builds the probe of a live instance and judges it with the function restore uses, at every unit boundary, reached through the public run call one unit at a time with the events resubmitted from the applied-event count (the programs with input) or through the internal step seam:

| Program | Unit boundaries |
|---|---|
| Lamp Tone, no input (4 frames) | 11,104 |
| Lamp Tone with the six events of its scenario (14 frames, all six events applied) | 42,229 |
| the boot stub contract 2 alone (4 frames) | 7,469 |
| the first STIC diagnostic, no input and KEY_1 (4 frames) | 41,729 and 41,722 |
| the second STIC diagnostic, no selector and the selectors KEY_1 to KEY_5 (4 frames) | 11,086, 11,089, 11,071, 11,080, 11,065 and 11,100 |
| the nine stand-in programs of the machine suite (20,000 units) | 20,001 each |
| the stand-in boot program (20,000 units) | 20,001 |
| a program that halts (loop of 8, HLT, follower; 300 units) | 301 |
| a program that ends in a guest fault (stops at the fault boundary) | 20 |
| total, 23 programs | 411,075 |

No invariant is violated at any boundary. The test sees what it claims: the register gate is closed at 362,123 boundaries and the graphics gate at 319,106, a held key with a pull-down at 129,365, a halted CPU at 75,510 and a faulted one at 1, the audio level 15, 14 frames, and the reset state (machine time 0, frame 0, event 0) at exactly 23 boundaries, one per program.

| Rule | Decision | The state that decided it |
|---|---|---|
| INV-1 | kept | the validators and the cursor hold at every boundary, including the halted and the faulted CPU |
| INV-2 | **corrected** | derived from the step code before the first run and shown necessary by the programs: at the reset state (machine time 0, the cursor at frame 0 event 0) the event at tick 0 is not yet applied, because the timeline is caught up at the end of every unit and never at the reset, so "the next event is above machine time" fails there for a state every instance passes through. The clause is kept for every state but that one; the probe that removes the exception fails `state.restore.invariants-hold-reachable` (23 reset boundaries) and the round trip at cut 0, and no boundary but those 23 needs it |
| INV-3 | **kept, strengthened** | the previous-level clause was written "the bytes are at most 15", which holds at every boundary; the step code builds the sample as A, B, C, 0, so the stricter form (three four-bit levels, a fourth byte of 0) is derived and kept, and a row with a fourth byte of 1 shows it refuses a state no step makes. No reachable state forced the change |
| INV-4 | kept | the words, the pull-downs derived from them and the input floor hold at every boundary of the Lamp Tone with six events and of the diagnostics with a selector |
| INV-5 | kept | |
| INV-6 | kept (the candidate is decided true) | obs.video.count equals stic.frame_index at every boundary of every program, frames reach 14 |

The corrections are in the code (`ivs_snap_inv2`, `ivs_snap_inv3`) and in the appended notes of `docs/formats/ivsn-v1.md` and `docs/state/hidden-state-checklist.md`; the oracle sections are not edited.

## The three faults (each fails the id named for it; the must-pass id stays green)

| Fault | Site | Must-fail id | What the id shows | Failing in the fault run (all with the prefix `state.`) | Must-pass id |
|---|---|---|---|---|---|
| RESTORE_PROFILE_UNCHECKED | `machine_snapshot_restore.c`, the profile checks | `state.restore.refuse-identity-profile` | an unregistered id and a flipped parameter digest accepted | refuse-identity-profile, reasons-covered, atomic-every-refusal, judge refusal-expectation-corrupt | `state.read.describe-ok` |
| RESTORE_IMAGE_DIGEST_UNCHECKED | `machine_snapshot_restore.c`, the slot SHA-256 and length comparison | `state.restore.refuse-identity-slots` | a snapshot taken with other images accepted | refuse-identity-slots, reasons-covered, atomic-every-refusal, judge refusal-expectation-corrupt | `state.read.describe-ok` |
| RESTORE_MUTATES_BEFORE_VALIDATE | `machine_snapshot_restore.c`, the CPU rows committed before the STIC validators | `state.restore.atomic-every-refusal` | a target changed by a refusal that only a STIC validator makes | refuse-content-validators, refuse-content-invariants, reasons-covered, atomic-every-refusal, judge refusal-expectation-corrupt | `state.read.describe-ok` |

Each id occurs once in `core/src`, in `machine_snapshot_restore.c`. A fault run of the whole `state` suite takes 0.23 to 0.93 s unsanitized (0.85, 0.88 and 0.89 s for the three restore faults; the sum of the eight faults of wave B is 5.87 local cpu-s), against the target of about 6 cpu-s per fault run of the plan; the reduced point set of `IVS_STATE_FAULT_RUN` (every 16th boundary of the reachability scan and one frame of the scenarios, the halting and faulting programs and the Lamp Tone with input whole) is in `state_invariants.c`, and the must-fail ids of the faults and the normal builds always run the full set.

## What each new test can fail on (mutation probes, run in the working tree and reverted)

| Probe on `machine_snapshot_restore.c` | Failing ids |
|---|---|
| section lengths never compared; the frame bytes not checked; an array count read and not compared | refuse-content-validators (and reasons-covered) |
| the loaded flag above 1 accepted; an empty record with a length accepted; the slot SHA-256 or length not compared; loaded-in-snapshot-only or loaded-in-instance-only accepted | refuse-identity-slots |
| an unknown profile accepted; the parameter digest not compared | refuse-identity-profile |
| the buffer-overlap rule removed; the report-overlap rule removed | refuse-args |
| the CPU, PSG or STIC validator skipped; the phase or the range of machine time not checked | refuse-content-validators |
| the input floor, INV-4's reserved bit, unconnected word or pull-down comparison dropped; INV-6 dropped; the gate flags not replayed; the timeline clause dropped; a PSG register mask, the emitted count, the previous levels or the sample index dropped | refuse-content-invariants |
| the sample index compared with 2 instead of 1 | refuse-content-invariants, atomic-every-refusal, invariants-hold-reachable, the round trip and the equivalence ids |
| the commit skips the last section, the CPU registers or the row latch; the PAD rows not zeroed; a BOUND row (exec_loaded) changed; the phase left OFF | roundtrip-fixed-point, poisoned-target-equals-source, same-profile-any-instance, sets-powered and digest-fields-informational (the PAD and row-latch probes: poisoned-target-equals-source and same-profile-any-instance only, the instances compared whole with poisoned targets) |
| the reset exemption of INV-2 removed; INV-3 levels at most 14; INV-4 floor strictly below machine time; INV-6 off by one; the register gate never closing in the replay | invariants-hold-reachable (and the round trip at cut 0 for the reset exemption) |
| the invalid-handle report not written; the success report not written; the slot kind not reported | refuse-args; roundtrip-fixed-point; refuse-identity-slots |

One probe survived by design: the PAD rows not zeroed is invisible to an all-zero or unpoisoned target, which is exactly why the equivalence runs against poisoned targets (the two ids above fail it at once). `state.judge.refusal-expectation-corrupt` shows the runner can fail: for every family and for its first, a middle and its last row, an unchanged row passes and a copy with the byte offset, the section index or the reason corrupted is reported as exactly one failing row, and a byte of the target changed behind the runner's back is reported by the next row (the atomicity oracle can fail).

## Deviations from Plan

**1. [Interpretation - the plan text against the specification] INV-1 and INV-5 are range rules (reason 78), INV-2, INV-3, INV-4 and INV-6 the derived rules (reason 79).** The plan lists the three validators, the phase and the range of machine time under reason 78 (truth 1) and also "each of INV-1 to INV-6 reason 79" (the invariants test); INV-1 is the validators and INV-5 the phase and range, so the two lists cannot both hold. The code follows truth 1 and the specification (step 8: "every decoded value meets its range and validator (78), the derived invariants hold (79)"); `ivs_snapshot_invariants` evaluates all six, and the rows of INV-1 and INV-5 are in the validators family as reason 78.

**2. [Interpretation] The order of step 8.** The plan prose lists the row counts (77) before the section lengths (80); the specification puts the lengths first, so reason 80 stays reachable by a chained pair (as plan 08-12's open item says) and 77 by a count prefix. The code follows the specification.

**3. [Rule 2 - missing correctness] The snapshot's slot record and the section frames are checked.** A loaded flag above 1 or an empty record that holds a length or a digest is reason 78 at the slot (the plan asks for the flag range; the empty-record rule and the frame check are added because a snapshot that carries either would be accepted and then re-encode differently, which breaks the canonical re-encode rule of the corruption family, plan 08-15).

**4. [Interpretation] The recorded IMAGES domain digest of step 7 is not compared** (a snapshot has no IMAGES section; recorded in the appendix).

**5. [Finding, not a plan change] One candidate invariant needed correction (INV-2) and one was strengthened (INV-3), above**, as the plan allows and asks. INV-2's reset exemption was derived from the step code when the invariant function was written and put in before the first run; the programs show it is needed (the probe that removes it fails the reachability id and the round trip at cut 0) and sufficient (no other boundary of 411,075 needs it). INV-3's stricter previous-level rule is derived from the step code and was added because the specification's weaker wording would accept a state no step makes; no reachable state violated either form.

**6. [Interpretation - the plan's verify text] Two verify commands are satisfiable only in their literal-intent form.** Task 2's `grep -c '^IVS_FAULT_RESTORE_' core/src/machine/machine_snapshot_restore.c | grep -q '^3$'` anchors the pattern at the start of a line, where every site (an `#ifdef` line, as every other fault site of the repository) starts with `#`: the unanchored `grep -c 'IVS_FAULT_RESTORE_'` prints 3 and was the one used (and 2 at the Task 1 commit, as the acceptance criterion says). And `printf '%s\\n' "$g" | wc -l` for the untouched CHECK and EXPECT of the control prints 1 for an empty variable (printf adds a newline): the direct `git show --stat --format= HEAD -- <the two files> | wc -l` prints 0.

**7. [Rule 1 - a defect of my own test, found at the fault verdict] The fault-run window of `state.restore.invariants-hold-reachable` was wrong.** The first version thinned the tiny halting and faulting programs (so the single faulted boundary was never checked) and cut the Lamp Tone before its first input event (so `consumed == nev` failed), which made the id fail in every fault build; the gate passed because the prefix `state.` allows collateral, so the defect hid behind it. Found by listing the ids that fail under each fault. Fixed in `ed4d466`: the two tiny programs are scanned at every boundary, the Lamp Tone with input runs to its last event in a fault run too (checks at every 16th boundary), and the id now fails in no fault run (the failing ids per fault are the table above).

**8. [Information] A fifth image pair in the state image include.** The plan names `boot_stub_2`, `lamp_tone`, `diag_stic_1` and `diag_stic_2`; the first STIC diagnostic is entered through the boot stub contract 1 (as `tests/run/run_diag_stic.c` does), so the include also carries `boot_stub`. The run suite's `ivs_images.inc` is not edited.

**9. [Information] The container gate of Task 2 ran on the Task 2 tree before the one-line fix of deviation 7**; the final container gate below runs on the closing tree, which contains it. The container gate of Task 1 ran on the tree before one test row was added (an INV-3 row that makes only the machine time disagree, found by a probe that survived without it); the Task 1 sh and dash gates were run again on the exact commit tree and read the identical summary line (md5 808d5b47308605c86f700ed12ba91c6c).

**10. [Information] The docs commit of the checklist is an appended note.** The plan says to update the INV status marks in `docs/state/hidden-state-checklist.md`; the checklist is part of the frozen oracle, so the status is a section appended at its end (the section above is untouched), and `docs/formats/ivsn-v1.md` has only added lines (0 removed).

No auth gates, no package installs (T-08-SC), no `gh` call, no push, no tag, no `tools/ci-record.sh`; `.planning/config.json`, `.planning/state.json` and `.planning/ci-runs.tsv` were never staged. D10 and D11 are not performed and no claim above provisional exists.

## Decisions Made

- Restore is a function of the snapshot and the loaded images alone: the instance's own state before the call plays no part (an OFF instance with images, a powered one, a halted one, a faulted one and a poisoned one all give the source byte for byte).
- The probe is the one place the validators and the invariants see a snapshot, so the same functions judge a decoded snapshot and a live instance; an invariant that a reachable state violates is corrected or dropped, never kept because the document says so.
- The refusal contract is describe's, extended by the slot kind; nothing is written where writing could change the instance.

## Cost (laptop)

| Item | Value |
|------|-------|
| estimate of the plan | 170,000 tokens (raw, confidence low), 3 tasks |
| actuals | 48,731 tokens (chars/4 over the added lines of the four commits outside `.planning/`), 3 tasks, 4 commits (measured from the ledger of the plan head) |
| gate total, sh, at plan start | 106 s (`gate-cpu` not recorded at the start) |
| gate total, sh and dash, Task 1 tree | 88 s and 99 s (first run); on the final Task 1 tree 97 s and 90 s, identical summary line (md5 808d5b47308605c86f700ed12ba91c6c) |
| gate total, sh and dash, Task 2 tree | 100 s and 108 s, identical summary line (md5 d832368303b207adb426aec33bf671e9) |
| gate total, sh and dash, Task 3 tree | 119 s and 104 s (`gate-cpu` user 191.0 sys 176.1 and user 174.5 sys 148.3), identical summary line (md5 5a6e1bccf39f000c77f258c2bfb27464) |
| gate total, sh and dash, closing tree (run again after an interruption) | 140 s and 82 s, identical summary line (the same md5); container 354 s, exit 0, 0 FAIL lines (gcc 13.3.0, x86_64 emulated) |
| container gate, Task 1 tree (the tree before one test row was added) | 371 s (gcc 13.3.0, cmake 3.28.3, x86_64 emulated), exit 0, 0 FAIL lines |
| container gate, Task 2 tree | 498 s (run while other work shared the laptop), exit 0, 0 FAIL lines |
| suite `state` (37 ids) | dev preset (sanitized, `-O1`) 2.17 s, release 1.05 s cpu (before the plan: 0.77 s and 0.36 s for 21 ids); a fault run 0.23 to 0.93 s |
| heaviest ids, dev and release | reachability 0.49 and 0.15 s, atomic-every-refusal 0.46 and 0.24 s, poisoned-target-equals-source 0.30 and 0.13 s (process start about 0.13 s on dev) |
| `ivs_machine_snapshot_restore` | about 42 microseconds per call on a 4,299-byte snapshot in a release build on a loaded laptop (5,000 restores into one instance; the reader's schema digest and the SHA-256 of the trailer, about 37 microseconds, dominate, the probe decode and the commit are a few microseconds); a save is about 1.0 ms (unchanged, it hashes the MEM domain twice) |

Cost checkpoint B is in `.planning/CI-EFFICIENCY.md` ("Phase 8 checkpoint B (plan 08-13)"): the last five successful hosted rows 43540936641, 43546632092, 43551481419, 43593328300 and 43616659462 give `r` of 7.78, 8.37, 7.20, 8.17 and 7.98, median **7.98** (7.9841) against the budget 7.8, the reading 8.0 and the line 8.3. One recorded row (c57328a) contains plans 08-09 to 08-12 (`git merge-base --is-ancestor 06d4820 <sha>` is true for it and false for the other four), so by the rule the projection is the median, 7.98, and the line is `checkpointB-08: no-lever`. The wave B additions priced by measurement (suite `state` 2.17 dev and 1.05 release cpu-s, 8 fault runs 5.87, the step 0.18, 7 controls at the unit cost, converted at 1.675) are 16.7 hosted cpu-s, of which 7.9 belong to this plan and are in no row: counted, the end of wave B reads about 8.16 to 8.27, above the reading 8.0 and under the line 8.3; the hosted row after the push settles it. The first trim, if the next row reads 8.3 or more, is named in the entry (the nine digests of every 8th refusal in `state.restore.atomic-every-refusal`).

## Open Items

- **Plan 08-14 (cuts and sweeps):** `state_util.c` has what it needs: `su_scenario_new` and `su_run_to` / `su_run_unit` (a powered instance of a scenario and the public run call resubmitting events from the applied-event count), `su_boot_new` / `su_boot_stepped`, `su_poison` / `state_poison`, `su_nine_digests`, `su_row_pos` (the container position of any STATE row from the table), `su_reseal`. The restore of a row-omission sweep value (a row replaced by the byte-wise complement) reaches the validators and the invariants: a replaced row that violates a validator or an invariant is refused, so a sweep value must be chosen to pass them (the sweep plan's own rule for rows the validators restrict).
- **Plan 08-15 (corruption family):** the canonical re-encode rule now holds for restore by construction of the checks above (slot records and frames are judged, the digest fields are the one named exception); `ivs_snapshot_describe` and restore share one reader, so the libFuzzer target can drive restore through an instance of the stand-in images with `su_boot_new`.
- **Plans 08-16, 08-17:** the installed consumer does not call restore; `api-calls=19` and `public-symbols=19` are in the inventories and the consumer golden and its calls list are unchanged. The host-side use (resubmit events from `obs.input.count`, keep the output buffers) is in the header of `ivs_state.h`.
- **Information.** D10 and D11 are not performed; no claim above provisional exists; no hosted-only check was run (the hosted-style container gate ran green); a snapshot is exact-build state; STA-01 and STA-02 stay Pending until the hard-point suite exists; REL-01 stays Pending (D-28). The checkpoint B line is `checkpointB-08: no-lever`.

## Interfaces for the next plans

- Public, `ivs/ivs_state.h`: `ivs_machine_snapshot_restore(m, buf, len, rep)`, `IVS_E_STATE_MISMATCH` (-24) and the reasons `IVS_REASON_SNAP_UNKNOWN_PROFILE` (72) to `IVS_REASON_SNAP_ARG_OVERLAP_INSTANCE` (81).
- Internal, `machine/machine.h`: `ivs_snap_probe`, `ivs_snapshot_probe_from_instance`, `ivs_snapshot_invariants` (0 or the number 1 to 6 of the first invariant that fails), `ivs_snapshot_profile_id_matches`, `ivs_snap_report_refusal` and `ivs_snap_report_success`, `IVS_SNAP_FRAME_BYTES`.
- Tests: `state_util.h` (the instance builders, the scenario ids `SU_SC_*`, the poison, the digests, `su_row`/`su_tab`/`su_run` and `su_run_rows`, the status of a reason); the families of `state_restore.c` are data (`rs_families`) that `state.restore.atomic-every-refusal`, the reasons-covered id and the judge all run.
- Gate counts after this plan `steps=38/38 controls=378/378 faults=234/234 fault-runs=234 tests-expected=4864 tests-executed=4860 tests-skipped=4 tests-missing=0 api-calls=19 api-rows=61 api-statuses=15 pinned-defines=120 public-symbols=19 hdr-alone=10/10 prefix-files=11 pkg-selftest=17/17 consumer-lines=135 state-rows=88 state-cuts=18`; `data/state/tests-pending.ids` holds three lines (the cut ids of plan 08-14).
- Threat ids T-08-31, T-08-32 and T-08-33 are used here (T-08-SC throughout); the next state plans continue from T-08-34 or the numbering their plans assign.

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=38/38 controls=378/378 ... faults=234/234 ... fault-runs=234 ... tests-expected=4864 tests-executed=4860 tests-skipped=4 tests-missing=0 ... api-calls=19 api-rows=61 api-statuses=15 pinned-defines=120 public-symbols=19 hdr-alone=10/10 prefix-files=11 pkg-selftest=17/17 consumer-lines=135 ... state-rows=88 state-cuts=18 (0 FAIL lines)
dash tools/gate.sh:    the identical summary line (0 FAIL lines)
GATE_LINUX_PLATFORM= sh tools/gate-linux.sh: exit 0, build/gate-linux.log ends gate: PASS (gcc 13.3.0, cmake 3.28.3, x86_64 emulated), the plan-end counts, core-imports=5, 0 FAIL lines
```

After the Task 1 tree (the final commit tree): sh and dash `faults=233/233 tests-expected=4850 tests-executed=4846 api-calls=19 api-rows=61 api-statuses=15 pinned-defines=120 public-symbols=19` with the identical summary line (md5 808d5b47308605c86f700ed12ba91c6c), container exit 0 (371 s, the tree before one test row was added, see deviation 9). After the Task 2 tree: the plan-end counts under sh (100 s) and dash (108 s) with the identical summary line (md5 d832368303b207adb426aec33bf671e9), container exit 0 (498 s). After the Task 3 tree (the as-built appendix, the invariant note and the checkpoint entry, which also contains the fix of deviation 7): sh (119 s) and dash (104 s), identical summary line (md5 5a6e1bccf39f000c77f258c2bfb27464). Every first gate run of every task passed (no retry, no `FAIL` line at any gate of the plan). The work was interrupted once by a lost connection after the closing gates had passed; the three gates were run again on the final tree, the one that this file, STATE.md and ROADMAP.md complete (`tracked=2452`): sh `gate-time total=140` (`gate-cpu` user 161.3 sys 135.6) and dash `gate-time total=82` (user 132.3 sys 118.9), `gate: PASS` with the identical summary line (md5 5a6e1bccf39f000c77f258c2bfb27464), 0 `FAIL` lines, the plan-end counts above; and `GATE_LINUX_PLATFORM= sh tools/gate-linux.sh`: exit 0, `gate: PASS`, the plan-end counts, 0 `FAIL` lines, `gate-time total=354` (`gate-cpu` user 1,317 sys 105 emulated); `build/gate-linux.log` holds that run. The dev-preset `ctest` suite `state` (37 ids) and the release-preset `state` suite pass; the three restore fault executables fail exactly the ids of the table above.

## Known Stubs

None.

## Threat Flags

None new. T-08-31 (a partial mutation before a later check fails): the sections are decoded into a probe and the commit has no refusal path; `state.restore.atomic-every-refusal` compares the whole instance and the nine digests after every refusal row of every matrix and the argument rows, against poisoned non-trivial targets; RESTORE_MUTATES_BEFORE_VALIDATE shows the oracle can fail. T-08-32 (a resealed snapshot installs a state no run reaches, or a wrong invariant refuses real snapshots): every device validator rule and every invariant has a resealed row, each invariant was held against 411,075 unit boundaries of 23 programs before it was allowed to refuse (two were corrected by that, none dropped), and the trailer is never the defence. T-08-33 (the band retarget weakens the control): one commit changed the band constant, the message, the header comment, the pin, the name and its test and the overlay of the control; CHECK and EXPECT of the control are untouched and `controls=378/378` still holds, and the new status has a refusal row (`api-dead-status`). T-08-SC: no installs.

## Self-Check: PASSED

Verified before the final commit: the SUMMARY and the created files of the plan exist (FOUND for `machine_snapshot_restore.c`, `state_util.c`, `state_util.h`, `state_restore.c`, `state_invariants.c` and the SUMMARY); commits `3345854`, `ff9b441`, `ed4d466` and `e0754ab` are ancestors of HEAD and `git rev-list --count c57328af2286d1fa327c86e63f50c6e0f9c33342..HEAD` printed 4 before this file's commit; nothing is staged but the files of this commit and `.planning/config.json`, `.planning/state.json` and `.planning/ci-runs.tsv` are not staged; `git ls-files` lists no `.ivsc`, `.bin`, `.rom`, `.exec`, `.ivsn`, `.wav`, `.ppm` or `.vraw` file; the diff of the four commits outside `.planning/` holds no home, scratch or temp path and no "Intellivision"; no commit deletes a file; `grep -c IVS_FAULT_RESTORE_ core/src/machine/machine_snapshot_restore.c` prints 3 and `grep -rho IVS_FAULT_RESTORE_ core/src` shows each of the three ids once; `git show e0754ab -- docs/formats/ivsn-v1.md` has 0 removed lines; `grep -c '^state[.]restore[.]' data/expected/tests/state.ids` prints 15, `grep -c '^state[.]judge[.]' ...` prints 3 and `grep -c '^IVS_REASON_SNAP_' data/expected/api-reasons.ids` prints 24; `grep -c '^IVS_E_STATE_MISMATCH -24$' data/expected/api-statuses.ids` prints 1 and the old band text `-24..-31` is in none of the three files; `git show --stat --format= 3345854 -- tools/check/controls/api-status-reserved/CHECK tools/check/controls/api-status-reserved/EXPECT` prints nothing (only the two overlay values moved); `grep -c '^checkpointB-08: ' .planning/CI-EFFICIENCY.md` prints 1 and the line is `checkpointB-08: no-lever`.
