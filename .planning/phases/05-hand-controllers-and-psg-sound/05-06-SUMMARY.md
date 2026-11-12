---
phase: 05-hand-controllers-and-psg-sound
plan: 06
subsystem: core/psg, core/machine, core/memmap, cmake, tests/psg, tests/run, tests/installed-consumer, data/profiles, data/golden
tags: [psg-tone, mixer, amplitude, sample-stream, audio-live, domain-17-chain, golden-rebless-2, D-09, D-10, D-15, D-16, D-17, D-18, A-01, A-02, A-03, A-09, AUD-01, AUD-02]
status: complete

requires:
  - phase: 05-hand-controllers-and-psg-sound
    provides: "plan 05-01 oracle commit 14a1904 (tone.txt, mix.txt, amp.txt, psg-policies.ids, rules.json); plan 05-02 grown request, report and info structs; plan 05-03 psg state, AUDIO rows, composite v2; plan 05-04 register file, machine bus wrapper, pending lists; plan 05-05 ports (HEAD at start 41d426a)"
provides:
  - "10 profile fields of the time chain and the tone, mixer and amplitude rules (psg.clocks_per_step 8, psg.sample_period_ticks 16, psg.counter_model, psg.period_zero.tone 4096, psg.mixer.bits, psg.amp.env_select, psg.reset, psg.clock_phase_ticks, psg.write_strobe_ticks, psg.max_unit_ticks 468); ticks_per_psg_clock retagged unknown-default-chosen -> inference (C76)"
  - "engine TUs core/src/psg/psg_tone.c, psg_mix.c, psg_sample.c: down-reload tone counters, the pure mixer gate and amplitude index, the chip-step scheduler with lazy catch-up, the 4-byte sample record, the sink (a NULL buffer is a counting sink) and the AUDIO level-change chain"
  - "the machine bus wrapper stamps a PSG write (unit start + offset x ticks per cycle + psg.write_strobe_ticks) and catches the PSG up to the stamp before the masked store; the step seam catches it up to the unit end at every unit end; ivs_machine_step_unit_sink_internal"
  - "real audio layout: IVS_OUTPUT_FORMAT_PSG_LEVELS_V1, 15,060 bytes per frame; ivs_info v3 audio_sample_ticks 16, audio_sample_bytes 4, crystal 3579545 / 1; report audio_bytes_written and audio_first_index; capacity refusal reachable through the public API for audio"
  - "psg suite 121 ids per preset (was 51), 8 named faults, derived constants U = 48 / 468 ticks and g = 36 / 28 ticks"
  - "golden re-bless #2 (three .digests files and pkg-consumer.golden), classified"
affects: [05-07, 05-08, 05-09, 05-11, 05-12]

actuals:
  tokens: 70000
  tasks: 5
  commits: 5
plan_head_before: 41d426a3bd00e4fb7f70134543be30c7fc2e9f8f
plan_head_after: 78f8b5f

tech-stack:
  added: []
  patterns:
    - "Engine vector families (tone, mix, amp) generated as ivs_pe rows (edge-spacing, toggles, level-seq, gate, index) from the 05-01 vector files; the judge is one function, ivs_pe_eval, that reads only the vector and the sample stream, with a judge self-control per predicate"
    - "A section table in psg_main.c replaces the two-family concatenation, so a family is one psg_add call"
    - "Constants derived by exhaustive probe through the real CPU model with a synthetic bus that answers every access with the wait under test; the test pins both the derived number and the published field"
    - "The audio sink is a per-call stack struct; a NULL buffer runs the same code against a counting sink, and a synthetic layout whose audio format is not PSG levels also gets the counting sink (Phase 4 synthetic-layout tests keep their meaning)"

key-files:
  created:
    - core/src/psg/psg_tone.c
    - core/src/psg/psg_mix.c
    - core/src/psg/psg_sample.c
    - tests/psg/psg_tone.c
    - tests/psg/psg_mix.c
    - tests/psg/psg_sample.c
    - tests/psg/psg_slice.c
    - tests/psg/psg_chain.c
    - tests/psg/psg_bounds.c
    - tests/psg/psg_buffers.c
    - tests/psg/psg_util.h
  modified:
    - cmake/IvsPsgTable.cmake, cmake/IvsPsgVectors.cmake, cmake/IvsMemmapTable.cmake, core/CMakeLists.txt
    - core/src/psg/psg.h, psg_params.h, psg_state.c, core/src/memmap/memsys.h
    - core/src/machine/machine.c, machine.h, machine_bus.c, machine_bound.c, machine_run.c
    - core/include/ivs/ivs_machine.h, ivs_run.h, data/api/api-effects.json
    - data/profiles/intv-2609-ntsc-family-p1.json, data/expected/profile-fields.ids, data/psg/fields-pending.ids, data/psg/vectors-pending.ids
    - data/expected/tests/psg.ids, run.ids, data/expected/faults/psg.ids, run.ids, machine.ids
    - data/golden/diag-cpu-1.digests, diag-input-pass.digests, timeout-budget.digests, data/expected/pkg-consumer.golden
    - tests/CMakeLists.txt, tests/installed-consumer/main.c, tests/machine/machine_cases.h, machine_digest.c, machine_standin.c
    - tests/psg/psg_cases.h, psg_main.c, tests/run/run_buffers.c, run_info.c, run_input.c, run_struct.c

key-decisions:
  - "One PSG step = 8 chip clocks = 16 master ticks = 4 CPU cycles; the first chip step after a power-on reset is step 1 (next_sample starts at 1) and sample k is stamped k x 16; the state is zero at create and only the power-on reset gives it step 1"
  - "The AUDIO change record index k is the 1-based step number of the sample; audio_first_index is the number of samples emitted before the call (so the first delivered sample of a call is step first_index + 1)"
  - "prev_levels holds channel A in bits 7..0, B in 15..8, C in 23..16 as a number (the digest serializes it big-endian)"
  - "The write stamp catches the PSG up only for a WRITE or DW whose address the memory system's own write view decodes to the PSG region (ivs_memsys_describe), so the wrapper holds no address constant"
  - "Delivery into the caller's buffer happens only when the layout's audio format is PSG_LEVELS_V1 with a nonzero size; any other layout (the synthetic ones of the Phase 4 tests) uses the counting sink"
  - "The engine counter convention is POL-PSG-02: empty counters reload on the step that finds them empty; with the first step at tick 16 a program that writes its period after tick 16 first runs one 4096-step half-period (the period-zero value) before its own period applies"

duration: about 5 h including eleven gate runs
completed: 2026-11-12
---

# Phase 5 Plan 06: Tone, mixer, amplitude, sampling, audio live Summary

**The PSG is a time-driven device: three down-reload tone counters, the mixer gate and the 4-bit amplitude index run on a 16-tick chip-step grid that is caught up lazily and identically in the step seam and in the run call, every step makes a 4-byte sample (levels A, B, C, 0) delivered into the caller's audio buffer under a capacity rule with proven slack (15,060 bytes per frame), the AUDIO digest chains the level changes, and the gate ends `steps=35/35 controls=303/303 faults=174/174` under `sh`, `dash` and the Ubuntu container.**

Oracle commit (05-01): `14a1904` (`tone.txt`, `mix.txt`, `amp.txt`, `psg-policies.ids` and `rules.json` were committed there; no oracle file was edited). Plan start `41d426a`.

## What was built

- Task 1, tracer (`5decc62`): the ten profile fields and the retag; `IvsPsgTable.cmake` (fail closed on the new fields) and `IvsMemmapTable.cmake` (the registry row gained `ticks_per_psg_clock`, the sample period, `psg.max_unit_ticks` and the crystal passthrough); `psg_tone.c`, `psg_mix.c`, `psg_sample.c`; the wrapper catch-up and the seam catch-up; `IvsPsgVectors.cmake` families `tone`, `mix`, `amp` (edge-spacing, toggles, level-seq, gate, index; writes at `@<step>`); the 25 vector ids and the guest tracer `psg.tone.guest-late-period` (stand-in program TONE writes the mixer, amplitude A and a period of 1 within 300 ticks; by the stated timing rule the first edge is at step 4096 and the edges then follow every step); four named faults. The composite and INSTANCE stepped literals of `machine.digest.stepped-literals` were re-recorded (CPU, MEM and IMAGES unchanged).
- Task 2 (`c7492bb`): `ivs_machine_profile_env` gives the real audio layout, `ivs_info` v3 and the report fields are written, the run loop delivers through a per-call sink, the flipped Phase 4 expectations (below), the consumer's count rule.
- Task 3 (`ece6de5`): `psg_bounds.c` (the exhaustive probe) and `psg_buffers.c`; three named faults.
- Task 4 (`9189b34`): `psg.sample.*` (12), `psg.slice.*` (8), `psg.chain.*` (4); the eighth named fault.
- Task 5 (`78f8b5f`): golden re-bless #2 and the three final gates.

## Derived constants (A-03, recomputed from the code)

Exhaustive probe over every decle, SDBD off and on, waits 0, 1 and 35, with a bus that answers every access with the wait under test (the CPU decides which kinds take it), 73,692 units at wait 0 and 147,384 at waits 1 and 35, and every first operand word for the jump group:

| Constant | Value | Where it sits |
|---|---|---|
| U, real bus (wait 0) | 12 CPU cycles = 48 ticks (jump group `$004`) | `psg.bounds.unit-ticks-real-bus` |
| U, every wait-taking access waits the cap 35 | 117 CPU cycles = 468 ticks (= (12 + 3 x 35) x 4) | `psg.bounds.unit-ticks-max`, published as `psg.max_unit_ticks` 468 |
| interrupt-entry unit | 9 CPU cycles (writes with DW at offset 2) | probe diagnostic |
| g, instruction to instruction | 9 CPU cycles = 36 ticks (e - o >= 5, o >= 4) | `psg.bounds.write-gap-min` |
| g, interrupt entry included | 7 CPU cycles = 28 ticks (5 + 2) | `psg.bounds.write-gap-min`; published by plan 05-08 as `psg.min_write_gap_ticks` |
| wait 1 and wait 35 gaps | 11 and 79 instruction, 8 and 9 overall: waits only lengthen a gap | same |
| audio slack s | ceil(468 / 16) + 1 = 31 | capacity 4 x (3,734 + 31) = 15,060 bytes per frame |

All four numbers equal the plan's; the notes' illustrative U = 608 is not what the code does. The memory system and the machine wrapper answer wait 0 for every kind at every address (`psg.bounds.memsys-bus-never-waits`), so a Phase 6 stall fails that id loudly.

## Flipped Phase 4 expectations (old -> new)

| Id or item | Old | New |
|---|---|---|
| `run.buffers.real-layout-zero` | every capacity suffices, no byte written | renamed `run.buffers.real-layout-audio`: video NONE accepts any capacity and gets nothing, NULL with 0 not requested, a nonzero audio capacity below 15,060 is BUFFER_TOO_SMALL |
| `run.info.layout-none` | both formats NONE, both sizes 0 | renamed `run.info.layout-real`: video NONE, audio PSG_LEVELS_V1 with a nonzero size |
| `run.info.v3-fields-zero` (05-02) | every v3 tail field 0 | renamed `run.info.v3-fields`: 16, 4, crystal 3579545 / 1; the two write-log fields stay 0 until 05-08; plus `run.info.audio-fields` (15,060 literal) |
| `run.struct.report-v2-fields` (05-02) | audio_first_index, log fields 0 | first call audio_first_index 0 and audio_bytes_written 0; the second call's audio_first_index is floor(now / 16); log fields 0 until 05-08 |
| `run.buffers.entry-points-equal` | 32-byte audio buffer against a zero layout | 45,180-byte buffers (three frames), reports, storage and delivered bytes equal |
| `run.input.refuse.events-overlap-audio` positive control | 48-byte audio buffer next to the array | 15,060-byte buffer ending where the array begins |
| `RUN_CAPACITY_AFTER_STEP` must-pass id | `run.buffers.real-layout-zero` | `run.buffers.exact-fit-video` (real-layout-audio checks that a refusal changes nothing, which the fault breaks) |
| `RUN_STOP_BEFORE_OVERSHOOT` allowed prefixes | no `run.struct.` | gained `run.struct.report-v2-fields` (the test now continues from now_ticks) |
| installed consumer `observed[6]`, `observed[5]` | `info.audio_format != NONE` | delivered bytes nonzero (the runner golden's count rule; the consumer delivers none) |
| `api-effects.json`, `ivs_run.h`, `ivs_machine.h` | "Phase 4 layout: no byte written" | the real audio layout, the sink, `audio_first_index`, the slack |
| `PSG_STATE_NOT_RESET_ON_POWER_ON` allowed prefixes (machine suite) | `machine.reset.psg-state-cleared` | also `machine.reset.equivalence` and `machine.digest.stepped-literals` (state now starts at step 1) |

`BUFFER_TOO_SMALL` is reachable through the public API for audio (closes the Phase 4 verification warning 3).

## Golden classification (re-bless #2)

Regenerated with `ivs-run --update-golden` (the three `.digests`) and from `build/gate/pkg-consumer.txt` (the consumer golden); `git diff -U0` shows only these lines:

| File | Line | Class |
|---|---|---|
| `diag-cpu-1.digests`, `diag-input-pass.digests` | `digest.instance`, `digest.composite` | allowed: the PSG state rows and the step index moved |
| `timeout-budget.digests` | `digest.instance`, `digest.composite` | allowed, same reason |
| `pkg-consumer.golden` | `digest.instance`, `digest.composite` (equal to the runner golden's) | allowed |
| `pkg-consumer.golden` | `info ... audio_format=0` -> `1` | allowed: the layout is real, the consumer delivers no audio, so no `digest.audio` line |
| every golden | `digest.cpu`, `digest.mem`, `digest.images`, `digest.input`, `digest.video`, `step.*`, `slot.*`, `cpu.*`, domains line | byte-identical (the guests never touch the PSG); `digest.cpu` and `digest.images` still equal `digest-v1-literals.ids` |

The runner still delivers no audio buffer, so `output.audio.bytes` stays 0 and audio stays unobserved in both the golden and the consumer; no audio line appeared (it would have been a defect).

## Evidence numbers

- Test ids: `psg.ids` 51 -> 121 (25 engine vectors, 2 guest and judge cases per family, 12 sample, 8 slice, 4 chain, 5 bounds, 6 buffers, 3 judges), `run.ids` 157 -> 159; `tests-expected` 3254 -> 3398 over both presets. Faults 166 -> 174 (8 new); controls unchanged at 303.
- Plain-run times (median of 11, loaded laptop): `psg` dev (ASan and UBSan) 0.17 s, release 0.04 s; `run` dev 0.57 s, release 0.30 s (05-05: 0.55 s and 0.30 s). The whole psg suite is 0.17 s dev: the exhaustive probe costs about 0.1 s.
- Gate cost (local): `sh` `gate-time total=36`, `gate-cpu` user 60.4 + sys 50.8 = 111 (05-05: 102 to 107); `dash` `gate-time total=39`, 126 cpu-s. The container run is a correctness check only (emulated amd64).
- `unknown-fields` in the profile record: 20 -> 23 (four new unknown-default-chosen fields, one retag).

## Deviations from Plan

**1. [Rule 3 - Blocking] Fault sites and registrations travel with their tests, not in Task 5.** The gate fails any `IVS_FAULT_*` site that is not registered (`fault-orphan-site`), so each fault was registered in the commit that adds the test that detects it: four in Task 1 (tone off-by-one, step of 16 clocks, mixer inverted, amplitude select ignored), three in Task 3, `PSG_SAMPLE_PHASE_RESET_PER_CALL` in Task 4. Task 5 became the golden re-bless and the three final gates. The mixer and amplitude vector families were wired in Task 1 for the same reason (the plan has them in Task 4).

**2. [Rule 1 - Plan precision] `psg.mix.truth-table` cannot detect `PSG_MIXER_SENSE_INVERTED`.** That oracle row (both enabled, both generators 1, out 1) gives 1 under either polarity. The fault's must-fail id is `psg.mix.and-tone0` (both enabled, tone 0, out 0); the oracle files were not edited. Open item below.

**3. [Rule 3] Tasks 1 to 4 end with the `golden` and `pkg-install` steps red**, as the plan states (the digests move with the first PSG step, the goldens are re-blessed once in Task 5); every other step was green at each task commit (`steps=33/35`).

**4. [Rule 1 - Plan precision] The synthetic layouts of the Phase 4 tests get the counting sink.** `ivs_machine_run_env_internal` delivers samples only when `env->audio_format` is `PSG_LEVELS_V1` with a nonzero size, so the synthetic environments of `run.buffers.*` (format NONE with bytes) keep their Phase 4 meaning of "no byte written"; the real layout (built from `ivs_info`) delivers.

**5. [Rule 3] The state starts at step 1.** `ivs_psg_state_reset` sets `next_sample` to 1 (the plan says "starts at 1"); the stepped literals (composite and INSTANCE) and the allowed prefixes of `PSG_STATE_NOT_RESET_ON_POWER_ON` were updated, the CPU, MEM and IMAGES literals are unchanged.

**6. [Other] Counts.** `amp.txt` holds 6 vectors and `mix.txt` 10 (the plan guessed 10 for amp); extra ids beyond the plan: `psg.sample.wrapper-stamp` (a guest write is stamped from the unit start and the access offset), `psg.judge.tone-row-changed`, `psg.judge.mix-row-changed`, a stand-in program TONE and CONST in `machine_standin.c`, and `tests/psg/psg_util.h`. The profile registry row grew by five scalars (`ticks_per_psg_clock`, `psg_sample_period_ticks`, `psg_max_unit_ticks`, crystal numerator and denominator).

## Decisions Made

- The write catch-up uses the memory system's own decode (`ivs_memsys_describe`, write view) to know that an access reaches the PSG, so the wrapper holds no address.
- The run call reports `STOP_INTERNAL` if a sample did not fit a buffer that passed the capacity check (never expected; the slack makes it unreachable); delivery is the only thing that can fail, the machine state is the same.
- Noise is the constant 1 and the envelope level is 0 until plan 05-07 (named stubs in `psg_sample.c`), so the levels of this plan are tone and fixed-amplitude only.

## Open Items

- **AUD-01 and AUD-02 stay Pending in REQUIREMENTS.md.** Tone, mixer, amplitude, sampling, the live AUDIO chain and the buffer rules are proven; noise, envelope and DAC (05-07), the write log (05-08), the runner and consumer audio replay (05-09 onward) and the registry edit of 05-12 close them.
- `psg.mix.truth-table` is the one printed mixer row that a polarity flip leaves at 1 (Deviation 2). A later oracle-owning plan may add a second row with a tone or noise 0 and both enabled; nothing here changes the oracle.
- `data/psg/fields-pending.ids` now holds the 7 lines of plan 05-07 and `data/psg/vectors-pending.ids` the 5 families (noise, env, dac, ampenv, reseng).
- `psg.min_write_gap_ticks` (28) is derived and asserted here and published by plan 05-08. The interrupt-entry gap was probed with a synthetic state and the interrupt line asserted; no device raises an interrupt until Phase 6.
- The two recorded convention differences of the blind second reads (noise first bits, envelope restart phase) belong to plan 05-07 and were not touched.
- The tone first-edge convention (a program that writes its period after tick 16 first runs one period-zero half-period, 4096 steps) follows POL-PSG-02 and C77; it is a policy, not hardware evidence (D10).
- Hosted cost is unmeasured: local `gate-cpu` rose about 4 to 9 cpu-s, the new suites add 0.17 s dev and 0.04 s release for `psg`, and eight faults at about 0.14 cpu-s each. The next hosted row decides whether the 280 cpu-s trigger is near; hosted gate wall is already near its 90 s trigger.
- Carried unchanged: the 05-04 unobtained AY-3-8910 reprint, the D10 and D11 items, the Phase 4 items IN-04, IN-06, IN-09 to IN-11, the C73 tag wording of 05-05.

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=35/35 controls=303/303 ... faults=174/174 ... tests-expected=3398 tests-executed=3396 tests-skipped=2 tests-missing=0`, 0 FAIL lines, `gate-time total=36`.
- `dash tools/gate.sh`: the same line, `gate-time total=39`, 0 FAIL lines.
- `sh tools/gate-linux.sh`: Ubuntu 24.04 container (gcc 13.3, cmake 3.28, dash): `gate: PASS steps=35/35 controls=303/303 ... faults=174/174 ... tests-expected=3398 tests-executed=3396 tests-skipped=2 tests-missing=0`, `cc=GNU` (13.3.0), 0 FAIL lines, `gate-time total=149` (emulated amd64, 8 cpus; a correctness check only), exit status 0, `build/gate-linux.log` ends with the `gate: PASS` line.

## Threat Flags

None new. T-05-15 (state depends on a buffer): mitigated by the counting sink, `psg.buffers.state-independent-of-buffer` and the fault `AUDIO_STATE_DEPENDS_ON_BUFFER`. T-05-16 (undersized buffer truncating the stream): capacity checked before the first unit with the slack from the derived U, `psg.buffers.too-small-no-write`, `psg.bounds.slack-covers-worst-case`, faults `PSG_OUTBUF_PARTIAL_WRITE` and `AUDIO_SLACK_ZERO`. T-05-17 (catch-up only at call end): catch-up to every unit boundary, `psg.slice.seam-equals-run`, `psg.slice.no-catch-up-to-target`, fault `PSG_SAMPLE_PHASE_RESET_PER_CALL`. T-05-SC: no installs.

## Self-Check: PASSED

- Files exist: core/src/psg/psg_tone.c, psg_mix.c, psg_sample.c; tests/psg/psg_tone.c, psg_mix.c, psg_sample.c, psg_slice.c, psg_chain.c, psg_bounds.c, psg_buffers.c, psg_util.h.
- Commits on HEAD: 5decc62, c7492bb, ece6de5, 9189b34, 78f8b5f (`commits: 5` measured from the ledger, `git rev-list --count 41d426a..HEAD`).
