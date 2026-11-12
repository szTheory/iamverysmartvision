# Phase 5, decision point: PSG raw-audio output surface (AUD-02) and D7 (reference PCM location)

Status: discuss output (fan-out protocol), not yet adopted. Labels: [OBS] read or ran in this repo or in a cited document, [INF] inference, [REC] recommendation, [UNK] unknown, [BG] background knowledge not re-fetched.
Inputs read [OBS]: AGENTS.md, AUTONOMY.md sections 1-3, PROJECT.md (Decision Register), REQUIREMENTS.md (AUD-01, AUD-02, API-03, API-04, RUN-05), ROADMAP.md Phases 5 and 7, `docs/time-base.md`, `docs/runner.md`, `core/include/ivs/ivs_run.h`, `core/include/ivs/ivs_machine.h`, `core/src/machine/{machine.h,machine_run.c,machine_bound.c,machine_digest.c}`, `core/src/cpu/cp1610.h` (bus access record), `data/profiles/intv-2609-ntsc-family-p1.json` (psg keys), `data/discrepancies.json` (C11, C12, C23, C60, C65-C67), `data/api/api-effects.json` (shape), `data/expected/{api-stop-reasons,api-reasons,important-gates}.ids`, Phase 4 `04-CONTEXT.md` and `04-DISCUSS-runapi.md`, `.planning/research/{ARCHITECTURE,PITFALLS}.md` (Pitfall 12), and the AY-3-8910/8912 section of the GI 1980 catalog (printed 5-23, local reference cache only, not committed).
No emulator source was opened (AGENTS.md: no copying). General emulator lessons below are [BG] and generic.

## 1. Facts that constrain the design

- [OBS] D2: machine time is a `uint64_t` master tick; the run call ends at the first CPU-unit boundary at or after the target; the call keeps no state between calls, the whole instance is the resumable state (API-02, `ivs_run.h`).
- [OBS] Phase 4 output seam (D-06, `machine_bound.c`): video and audio go to caller-owned buffers named in the request; capacity is checked before the first unit as `bytes_per_frame * (max(target-now,0)/nominal_frame_ticks + 1)`, else BUFFER_TOO_SMALL with nothing changed; the core keeps no pointer. The Phase 4 profile has layout NONE, 0 bytes per frame.
- [OBS] Digest domains 16/17/18 are VIDEO/AUDIO/INPUT rows `chain[32]` + `count u64` (INPUT has more), not in the composite (v1 over domains 1..4); chain rule `SHA256("IVSR" || domain || chain || u64 index || u32 len || record)`; "record granularity is defined by the phase that produces records" (`04-DISCUSS-runapi.md` 3.4). Rejected there: a streaming SHA context in instance state, and hashing the caller's buffers per call (both break slicing invariance or auditability).
- [OBS] `ivs_run_request` v1 is 64 bytes and `ivs_run_report` v1 is 112 bytes, both frozen by plan 04-13; the A-02 rule says a new field goes after the v1 size and the minimum `struct_size` stays the v1 size. `ivs_info` is 256 bytes today and has no crystal field.
- [OBS] The CPU bus callback receives each access with `offset_cpu_cycles` (BAR/address-phase cycle relative to the unit start, after earlier waits) and a kind (`IVS_ACC_WRITE`, `IVS_ACC_DW` for the interrupt-entry write, ...). The bus ctx today is the memory system only; machine time is advanced after the unit (`machine_run.c`). So a write stamp is `unit_start_tick + offset_cpu_cycles * ticks_per_cpu_cycle` and the PSG must be reached through a ctx that also carries the unit start tick.
- [OBS] Profile: `ticks_per_psg_clock = 2` (tag unknown-default-chosen, emulator hint only), `psg.part` AY-3-8914-class (tier4-community, C11), PSG registers `$01F0-$01FF` are a stub today, `power_on_fill.psg` zero (unknown-default-chosen), `bus.wait_limit_cpu_cycles = 35`. C12 (period 0, envelope step count, noise LFSR, DAC levels) and C23 (reference PCM location) are open.
- [OBS] AY-3-8910/8912 data sheet (GI 1980 catalog, printed 5-23): tone frequency is obtained by "counting down the input clock by 16, then by the 12-bit tone period"; noise by clock/16 then the 5-bit noise period; envelope by clock/256 then the 16-bit envelope period, then a further /16 for a "16-state per cycle" pattern; each channel has a D/A converter producing "up to a 16 level" signal; "4 bits of logarithmic" amplitude control. This is the 8910, not the 8914 (C11): the 8914 map and the 2-bit envelope-select are community-only (Pitfall 12).
- [INF] A tone output of frequency clock/(16 x TP) is realizable exactly (odd TP included) by a counter stepped every 8 PSG clocks whose output toggles when the count reaches TP. The data sheet's wording does not state the internal sub-step; the 8-clock step is the coarsest grid that never aliases a state change [INF], so the chip tick is a profile parameter, not a fact. On the NTSC profile 8 PSG clocks = 16 master ticks = crystal/16 = 223,721.5625 Hz (the same number Pitfall 12 quotes from community sources) [INF/tier4].
- [OBS] `docs/time-base.md` defines "PSG step" as 16 PSG clocks = 32 master ticks. That is the unit in which TP counts one full output cycle, not the finest update interval. Both can stand; section 7 asks for a glossary addition ("PSG chip tick"), not a change of "PSG step".
- [OBS] One nominal frame is 59,736 master ticks = 3,733.5 chip ticks, so the sample grid is not frame-aligned. Frames are timeline events and never a modulo of time (time-base.md); the audio stream must therefore never be quantized to frames.
- [OBS] Phase 5 SC5 / AUD-02: "a run emits a cycle-stamped PSG register-write log and integer chip-rate samples as raw audio; any PCM conversion sits where D7 puts it and is outside the raw-audio evidence path."
- [UNK] Whether the real 8914 has sub-grid behavior, its power-on phase, the DAC curve and the analog mix law; the exact intra-unit instant at which a CPU write reaches a PSG register. None has a tier 1-3 source; D10 owns the measurement.

## 2. Options

### 2.1 Write-log delivery and overflow (decision point 1)

| # | Option | Pros | Cons | Verdict |
|---|--------|------|------|---------|
| L1 | Third caller buffer in the run request, worst-case capacity checked before the first unit (BUFFER_TOO_SMALL), no state change on refusal | Same mental model, same code path and same tests as video/audio (API-04 verbatim); refusal before any state change; no new stop reason; no retained state | The bound is derived from CPU timing (min ticks between two writes), so buffers are larger than real use (tens of KB per frame slot vs tens of records) | Recommended |
| L2 | Back-pressure: stop with a new stop reason (OUTPUT_FULL) when fewer than one record of room remains; the host drains and resubmits | Tight buffers; scales to Phase 6/7 event logs with no cheap bound | A capacity-dependent stop point; a new stop reason; contradicts API-04's "undersized buffers are refused"; hosts must handle a new loop | Reserved as a reversal path; not defined now |
| L3 | Core-retained ring read by a new `ivs_machine_psg_log_read` call | No sizing problem | New large instance state, new digest rows, a second call to sequence; Phase 4 rejected the same shape for video/audio | Rejected |
| L4 | Interleave log records into the sample buffer | One buffer | Mixed record sizes, harder consumer, breaks "buffer = one kind" and the per-channel capacity formula | Rejected |

### 2.2 What a raw audio sample is (decision point 2)

| # | Option | Pros | Cons | Verdict |
|---|--------|------|------|---------|
| S1 | Per-channel amplitude index fed to the DAC (A, B, C, each 0..15 on this profile, 0 when the channel is gated low), 4 bytes per sample (A, B, C, reserved 0) | Contains no unknown analog constant (DAC curve, mix law); lossless for audible behavior; hand-derivable vectors from the data sheet; tier tags attach only to the digital rules (envelope step count, shift of the 8914 select bits) | A listener needs a mix table (reference-PCM concern) | Recommended |
| S2 | Mixed unsigned 16-bit sum using a profile DAC table | One number per sample, directly WAV-able | Bakes the unknown DAC curve and mix law (C12) into the evidence stream and its digest; any later table fix invalidates every audio golden | Rejected for core |
| S3 | Three DAC amplitudes (u16 x 3, 6 bytes) | Still per channel | Same unknown constant as S2, larger | Rejected |
| S4 | Full internal bits per channel (tone bit, noise bit, gate, envelope level, volume register) | Most debuggable | Mostly redundant with the register log and the PSG state domain; ties the stream to the micro-architecture; bigger | Rejected (PSG state digest covers internals) |

Sample period (what "chip-rate" means):

| # | Option | Verdict |
|---|--------|---------|
| P16 | One sample per chip tick = 8 PSG clocks = 16 master ticks (profile key, derived `chip_tick_psg_clocks * ticks_per_psg_clock`) | Recommended: the finest grid consistent with the data sheet divide chain; it cannot alias any state change that a 32-tick "PSG step" grid sees |
| P32 | One sample per PSG step (16 clocks = 32 master ticks) | Rejected: aliases odd-TP half-cycle toggles [INF] |
| Pdac | One sample per DAC or tone tick of a fixed channel | Rejected: the notion is not defined for the other channels |

### 2.3 Digest accumulation (decision point 1 and 2)

| # | Option | Pros | Cons | Verdict |
|---|--------|------|------|---------|
| D-a | Chain over write records (domain 19) and chain over level-change records (domain 17, plus rows that bind the emitted sample count) | Slicing independent (records are state-determined); about 1000x cheaper than hashing every sample; the dense stream is exactly reconstructable from the change records plus the emitted count; a host can recompute the chain from the delivered bytes as an output-path self-check | Slightly more rows (previous levels, emitted count); a reader must know the change-record rule | Recommended |
| D-b | One SHA-256 compression per sample | Simplest rule | About 224k compressions per emulated second, roughly 30-70 ms per emulated second at -O2 and several times that under sanitizers [INF, 150-300 ns per compression]; scales with run length forever, taxes every slicing fuzz and every gate preset | Rejected on cost |
| D-c | Fixed 4096-tick block chain | Constant record size | Needs a partial-block buffer or hash context in the instance, which Phase 4 rejected | Rejected |
| D-d | Hash the caller's delivered bytes per call | No core rows | Depends on slicing and on whether a buffer was given; breaks API-02 | Rejected (Phase 4) |

### 2.4 Where reference PCM lives (D7, decision point 3)

| # | Option | Pros | Cons | Verdict |
|---|--------|------|------|---------|
| R0 | No conversion anywhere in the repo; hosts roll their own | Smallest | No listening path, no shared algorithm, every host re-invents rate conversion and the AY mix law; the "no human UAT" listening handoff has no tool | Rejected |
| R1 | In the core observation layer (an `ivs_*` call returning S16 at 44.1/48 kHz); ARCHITECTURE.md layer 2 | One implementation, one golden | Puts a rate, a filter and an unknown DAC/mix table into the core ABI and its tests; a filter tweak becomes a core change; couples D7 to C12; every integrator pays for it; invites the evidence-path contamination Pitfall 12 warns about | Rejected |
| R2 | A separate installed library target (`ivs::refpcm`) | Reusable by hosts through find_package | A second public artifact, ABI, packaging proof and version line for an algorithm that is explicitly "not chip truth"; premature | Reversal path |
| R3 | Pure integer module under `tools/pcm/`, compiled only into the runner (optional WAV) and its own tests; not installed; documented as copy-paste for hosts | Keeps the core clean; testable with its own goldens and named faults; zero dependency; fits "another copy-paste is better than another dependency"; one command to listen | Hosts copy two files instead of linking | Recommended |
| R4 | Inside the runner source only | Fewest files | Not testable or reusable apart from the runner; couples the converter to scenario/report code | Rejected (R3 is the same thing with a seam) |

### 2.5 Listening check without human UAT (decision point 5)

| # | Option | Verdict |
|---|--------|---------|
| H1 | Operator-only `ivs-run --audio-wav <file>` (R3 conversion of the raw samples), plus automated objective checks (tone period in samples against the data-sheet formula; zero-crossing frequency of the reference PCM within a tolerance); the WAV is a handoff artifact outside evidence, never committed, never produced by CI | Recommended |
| H2 | Commit a WAV golden | Rejected: binary in the repo; a listening artifact must not be mistaken for a golden; hygiene lists binaries |
| H3 | CI uploads a WAV artifact | Rejected: D-43 (no artifacts) and no recurring value |
| H4 | Skip listening entirely | Rejected: subjective timbre/level is one of the few things AUTONOMY section 3 says to hand off, and a one-command artifact is cheap |

## 3. Role lenses (condensed)

- Emulator/audio architect: the write log is the cause stream and is model independent (decode of `$01F0-$01FF` plus CPU timing only); the level stream is the effect and is model dependent. Two domains means a PSG model fix (C12) moves only domain 17, never 19. Event/change-list form is also what a band-limited synthesizer wants as input [BG: the standard clock-stamped delta approach of blip-style buffers], so the change records are a good host primitive. Lazy catch-up is the standard scheduler shape: the PSG advances to a write stamp before applying the write and to `now` at the end of every run call, never to the target.
- Hardware/EE: the data sheet fixes frequency relations, not the micro-architecture; the grid is a profile parameter with a tag. The mixer gate (tone OR disable) AND (noise OR disable) is data-sheet architecture; the analog mix is not, so it stays out of the core. 8914 differences (2-bit envelope select, C11) change the index fed to the DAC, which S1 captures without naming an analog value.
- QA: oracles must be independent of the core. Tone period in samples (2 x TP samples per cycle) comes from the data sheet; write-log ticks come from the CP-1600 manual cycle table and the hand-assembled listing; chain values come from the Phase 4 `printf` + `cmake -E sha256sum` technique (A-08). Slicing fuzz must straddle both write stamps and grid points.
- Security/rights: scenarios stay inert data and cannot name output files (D-22); the WAV path is an operator flag only, with a byte cap; audio rendered from user-supplied cartridges is derived from rights-bearing content, so it stays local (documented), never CI-uploaded or committed. No EXEC/GROM bytes appear in any audio artifact.
- DevOps/CI: one in-process suite for conversion (sub-second), the audio checks join existing suites (`run`, `runner`, `runner-process`); no new workflow step; no artifact. Gate cost is dominated by sample generation under sanitizers, so run-length of audio scenarios must be short (about 0.1-0.5 emulated seconds) and generation must stay O(chip ticks) with an O(changes) fast path left as a later, golden-protected optimization.
- API/integrator: no new public function; two new request fields and four report fields appended under the A-02 rule; `ivs_info` grows by append. LibFuturo-style S16 consumers copy `tools/pcm` or write their own; the raw surface is host-rate free.
- Product: a human can run one command and hear the diagnostic tones; the claim ledger gets no audio-accuracy claim from it.
- DX: output buffers are optional per channel (capacity 0 means "not requested", section 4 AU-07) so a CPU-only test needs no allocation; `audio_first_index` in the report lets a host resume without carrying counters; sample stamps are never stored (first index times period).

## 4. Decisions (numbered, concrete)

Proposed ids AU-01.. are for the Phase 5 CONTEXT; the orchestrator renumbers.

### AU-01 Register-write log: record, stamp, source

- One record per guest write to `$01F0-$01FF`, 16 bytes, written big-endian in the chain and in memory as a struct of fixed-width fields:

  | Field | Type | Meaning |
  |-------|------|---------|
  | `tick` | u64 | master tick of the write: `unit_start_tick + offset_cpu_cycles * ticks_per_cpu_cycle + psg.write_strobe_ticks` (profile key, default 0, tag unknown-default-chosen) |
  | `value` | u16 | the raw 16-bit bus value written (lossless to the guest; masking to 8/4 bits is model, not log) |
  | `addr` | u8 | bus address low nibble, 0..15 (the Intellivision order, not the 8910 numbering; no register is named in the record) |
  | `source` | u8 | 1 = CPU write (`IVS_ACC_WRITE`), 2 = CPU interrupt-entry write (`IVS_ACC_DW`); 0 and 3..255 invalid, append-only, pinned by `data/expected/psg-log-source.ids` |
  | `reserved` | u32 | 0 |

- No synthetic records at power-on: the profile pins the power-on register state (zero, `power_on_fill.psg`), so log + profile reproduce the chip history. Reset clears the log rows (the whole observation row is already zeroed by power-on reset); a source value for a future reset line stays reserved, not emitted.
- Reads (register read-back, R14/R15 port reads) are not logged in v1. Adding read records later is a domain-19 version bump (v2) with a golden refresh; the record already carries `addr`, `value` and `source`, so no layout change is needed.
- Records are strictly ordered by `tick` (two writes cannot share a bus cycle); an assertion in the producer plus a test pins strict increase.
- Tie rule with samples (AU-04): a write stamped `t` is seen by the chip step at `t + P`, not by the step at `t` (step first, then write). Tag unknown-default-chosen; no hardware source; resolver D10.

### AU-02 Write-log buffer, capacity and refusal (Phase 4 seam, option L1)

- A third caller-owned buffer, `psg_log` + `psg_log_capacity` in the request. The core keeps no pointer; it writes records in order starting at byte 0 and reports bytes and the index of the first record.
- Capacity rule (before the first unit, refusal BUFFER_TOO_SMALL with nothing changed, reason `RUN_PSGLOG_SMALL`): `required = record_bytes * (max(target-now,0)/nominal_frame_ticks + 1) * W_log` where the profile publishes `psg_log_records_per_frame` (written `W_log`) as a proven bound on writes in one nominal-frame slot including the overshoot of the last unit: `W_log = ceil(F/g) + ceil(U/g) + 1`, with `F = nominal_frame_ticks`, `g` the minimum ticks between two consecutive writes the CPU can make (derived from the cycle table and verified by a property test over all opcodes, interrupt entry included), `U` the maximum ticks of one unit. [INF, illustrative: g = 28 ticks and U = 608 ticks give W_log about 2157 records, about 34.5 KiB per frame slot.]
- Proof sketch [INF]: writes in a run of T ticks <= (T+U)/g + 1, and `(floor(T/F)+1) * W_log >= (T/F) * ... > T/g + U/g + 1`. The same frame-slot formula as Phase 4 is therefore a valid bound without a second capacity function.
- NULL with capacity > 0 and overlap with the instance, the report, the other buffers or the event array refuse BAD_ARG (new reasons `RUN_PSGLOG_NULL`, `RUN_PSGLOG_OVERLAP`, appended after the current last reason; the stage joins stages 4b and 4c of the existing ladder, same order: video, audio, psg_log).
- Safety net (never expected to fire): before each unit begins, the sink checks that one more record fits; if not, the run ends with the existing `IVS_STOP_INTERNAL` (state at the last whole boundary, harness failure). No new stop reason. Named fault shows it fires when `W_log` is cut.
- Back-pressure (option L2) stays unwritten; trigger in section 8.

### AU-03 What a sample is, and the grid

- Profile key `psg.chip_tick_psg_clocks = 8` (tag inference; source: data-sheet divide chain, section 1) and derived `psg.sample_period_ticks P = chip_tick_psg_clocks * ticks_per_psg_clock = 16` (tag follows the weaker of its inputs, unknown-default-chosen, since `ticks_per_psg_clock` is an emulator hint). `P` is published in `ivs_info.audio_sample_ticks`.
- Sample `k` (k >= 1) is stamped at exactly `k * P` master ticks from reset (absolute grid; origin 0). It is the output of the chip after its `k`-th chip tick: for each channel c in {A, B, C}, `level_c` = the amplitude index presented to that channel's DAC (0 when the mixer gate is low), `0..15` on this profile (the field is a byte so a 5-bit variant in another profile fits without a format change).
- Record in the dense buffer, 4 bytes: `level_A, level_B, level_C, 0` (byte 3 reserved, written 0). Format id `IVS_OUTPUT_FORMAT_PSG_LEVELS_V1` (appended to the output-format list; 0 stays NONE).
- Per channel, not mixed. The mix (DAC table, analog sum, DC removal) is reference conversion (AU-12) and never enters the core. The profile record still carries the DAC table (SC4 of the roadmap requires DAC levels as tier-tagged profile data); it is consumed only by hosts and tools, never by core code.
- Numbers [INF]: 223,721.5625 samples per emulated second; 3,734 samples (ceil of 3,733.5) per nominal frame; 4 bytes per sample is 894,886 bytes per emulated second (one byte per CPU cycle); one dense frame slot about 14.6 KiB.
- Frames never quantize the stream. A run of T ticks emits `floor(now_exit/P) - floor(now_entry/P)` samples.

### AU-04 Generation: lazy, deterministic, slicing invariant

- PSG state (registers, counters, noise register, envelope phase, previous output levels) is instance state. Two operations advance it:
  1. `psg_catch_up(to_tick, sink)`: perform every chip tick `k` with `k*P <= to_tick` not yet performed, emit one sample per tick into the sink.
  2. `psg_write(stamp, addr, value, sink)`: `psg_catch_up(stamp)`, then apply the write, then append the log record.
- Rule: a sample at `k*P` observes writes with stamp `< k*P` (AU-01 tie rule). Hence at the end of every run call the core catches the PSG up to `now` (the unit boundary, never the target): the next unit's writes have stamps `>= now`, so no emitted sample can depend on them. This is what makes the stream, the log and both digests independent of how the host slices the run.
- The sink is a per-call stack struct (cursor, capacity, optional pointers) reached through the bus ctx; the core keeps no pointer after the call. With a NULL buffer the same code runs against a counting sink: delivering bytes never changes machine state (testable by whole-instance `memcmp`, named fault `AUDIO_STATE_DEPENDS_ON_BUFFER`).
- Overshoot: samples are emitted up to `now`, which may exceed the target by part of one unit; the report returns `audio_first_index` so concatenated slice outputs equal one big run byte for byte.
- Cost: O(chip ticks) per emulated second (about 223k small steps); an analytic fast path for silent or steady channels is allowed later only with the naive stepper kept as the reference in tests (performance after baselines, North Star).

### AU-05 Audio buffer capacity

- Audio uses the Phase 4 frame-slot formula unchanged: `required = audio_bytes_per_frame * (max(target-now,0)/nominal_frame_ticks + 1)`, but `audio_bytes_per_frame` is defined as `sample_bytes * (ceil(F/P) + s)` with the slack `s = ceil(U/P) + 1` samples per slot. Without the slack the bare formula is NOT an upper bound for a continuous stream (a target one tick short of a frame end can need `(F+U)/P` samples against `ceil(F/P)` = 3,734, e.g. up to about 3,736 needed for U = 48 ticks; frames are timeline events, a continuous stream is not).
  Proof [INF]: with `W = ceil(F/P) + s` samples per slot and `T < (q+1)F`, `(q+1)W > T/P + s >= (T+U)/P + 1 >= floor((T+U)/P) + 1`, which bounds the samples one call can emit.
- [INF, illustrative] with U = 608 ticks, `s` = 39, `W` = 3,773, `audio_bytes_per_frame` = 15,092. A 2^22-tick runner slice (70 slots + 1) needs about 1.02 MiB.
- `U` is the proven maximum ticks of one CPU unit under the profile (instruction base cycles + per-access waits up to `wait_limit_cpu_cycles` + future bus-grant stalls), published as profile data and asserted: a property test steps every opcode with maximal waits and requires `unit_ticks <= U`; a pre-unit reservation check (remaining slots >= `U/P + 2`) turns a violated bound into `IVS_STOP_INTERNAL` rather than a memory error. Named faults `AUDIO_SLACK_ZERO` and `UNIT_BOUND_EXCEEDED`.
- Hosts that single-step with audio need about 15 KiB per call (the formula is frame-slot quantized). A tick-tight formula `sample_bytes * ((T+U)/P + 1)` is a later refinement if that annoys a real host (section 8).

### AU-06 Digest domains

- Domain 17 (AUDIO, existing, version 1 until it ships with a producer): rows `chain[32]`, `count u64` = number of level-change records, new `prev_levels u32` (levels of the last emitted sample, needed to detect the next change), new `emitted u64` (samples emitted). Change record (12 bytes): `u64 k` (the sample index at which the three levels differ from the previous sample; the first sample compares with all-zero levels), then `A, B, C, 0`. Chain: `chain' = SHA256("IVSR" || 17 || chain || u64 index(=count) || u32 12 || record)`. The domain preimage includes `emitted`, so a longer silent run changes the digest.
- Domain 19 (new, `IVS_DIGEST_PSGLOG`): rows `chain[32]`, `count u64`. Chain: `chain' = SHA256("IVSR" || 19 || chain || u64 index || u32 16 || record)` with the 16-byte AU-01 record.
- Both are observation domains, outside the composite (v1 over 1..4 stays), reported individually, cleared by reset (obs memset), version 1. Their field-table rows keep the one-field-table coverage rule and need a pad adjustment (A-03: the obs struct grows by 8 + 4 + 8 + 40 bytes plus pad; `IVS_MACHINE_BYTES` and the static asserts change in the same plan).
- Why the change chain is evidence for the dense stream [INF]: the dense stream equals the change records expanded by hold, with the `emitted` count; the map is bijective. The runner recomputes the chain from the dense bytes it received and compares it with the core's digest (`audio.chain.check`), so a lost, duplicated or misplaced delivered byte is caught in every run that delivered audio.
- PSG device state is a separate state domain (new id between 5 and 15, not 17/19). Whether the composite becomes v2 to include it is a cross-cutting item for the orchestrator (section 9); my surface is independent of the answer.
- Independent oracle for both chains: the A-08 technique (authoring-time `printf` octal escapes + `cmake -E sha256sum`), hand-written records for a hand-derived program.

### AU-07 Buffer presence and the Phase 4 refusal rule (amendment)

- A capacity of 0 (with a NULL buffer) means "channel not requested": no capacity check, no bytes written, digests and state unchanged. A nonzero capacity must meet the requirement, else BUFFER_TOO_SMALL before any state change. This amends Phase 4 D-06 wording ("a buffer may be NULL only with capacity 0; a capacity below the requirement is refused"): the zero case moves from refused to not requested. Phase 4 profiles are unaffected (requirement 0). Tests that use a synthetic env with capacity 0 flip to a nonzero too-small capacity; the same rule should be adopted for video in Phase 6 (note for that discuss).
- Rationale: digests and the log do not need delivered bytes; forcing a 15 KiB allocation onto a CPU-only test is the wrong default; undersized-but-present stays refused, which keeps API-04's intent.
- Report fields make a silent omission visible: `audio_bytes_written` and `psg_log_bytes_written` are 0.

### AU-08 Public API additions (no new function)

- `core/include/ivs/ivs_run.h`, request v2 (appended after the 64-byte v1, 80 bytes with 64-bit pointers):
  `uint8_t *psg_log; uint64_t psg_log_capacity;`. The v1 minimum `struct_size` (64) stays accepted; the core reads the new fields only when `struct_size` covers them (A-02), treating them as NULL/0 otherwise. The current validation uses `sizeof(ivs_run_request)` as the minimum, so it must be rewritten to a pinned `IVS_RUN_REQUEST_V1_SIZE` (64) and the "nonzero tail" rule rebased on the core's own current size. Tests `run.struct.*` extend with a v1-sized caller against the v2 core.
- Report v2 (appended after the 112-byte v1, 136 bytes): `uint64_t audio_first_index; uint64_t psg_log_bytes_written; uint64_t psg_log_first_index;`. `audio_bytes_written` keeps its field and now counts dense sample bytes. Written only when `rep->struct_size` covers them.
- `ivs_info` v3 (appended at offset 256; written when covered): `audio_sample_ticks` (P), `audio_sample_bytes` (4), `psg_log_record_bytes` (16), `psg_log_records_per_frame` (`W_log`), `crystal_hz_num` (u64), `crystal_hz_den` (u32), reserved. `crystal_hz_*` is profile passthrough for host adapters and the reference converter; the core never reads it (D2). `audio_bytes_per_frame` is redefined as in AU-05 (slack included).
- `ivs_machine_state_digest`: domain 19 accepted; 17 now has a producer. Header gets `IVS_DIGEST_PSGLOG 19`, pinned in the existing digest-domain id inventory.
- No new stop reason (`api-stop-reasons.ids` unchanged). New reasons appended to `data/expected/api-reasons.ids`: `RUN_PSGLOG_NULL`, `RUN_PSGLOG_OVERLAP`, `RUN_PSGLOG_SMALL`. The `ivs_machine_run` row of `data/api/api-effects.json` gains the psg_log argument in the precedence text and three refusal rows (effect `none`, ids of the new tests); `ivs_machine_info` and `ivs_machine_state_digest` rows get one-line notes; `tools/check/api-effects.cmake` already ties prototypes, statuses and test ids, so the update is data.
- The installed consumer (`tests/installed-consumer`) passes the three buffers on one audio scenario and prints `digest.audio` and `digest.psglog`, which the gate compares with the runner golden for the same scenario (D-39 pattern); `pkg-consumer-calls.ids` is unchanged because no new function is referenced.

### AU-09 Runner: options, report keys, scenario syntax

- Report format version bumps to `ivsr 2` (the key table is closed and fixed order, so added keys are a format change; judges and goldens updated in the same plan). New keys, placed after `output.audio.bytes`: `output.audio.samples`, `output.audio.changes`, `output.psglog.records`, `output.psglog.bytes`, `digest.psglog` (after `digest.input`), `audio.sample_ticks`, `audio.chain.check` (`pass`, `fail`, or `-` when no audio buffer was delivered on every slice), and `refpcm.rate`, `refpcm.frames`, `refpcm.sha256` (`-` unless `--audio-wav` was given). Golden key lists gain the digest and count keys of the observed live domains but never the `refpcm.*` keys.
- Golden `domains:` header rule for audio and psglog: `observed` iff the profile layout for that channel is not NONE (a silent but live PSG is a real observation of silence), else `none`; video and input keep their count-based rule.
- Operator-only options (never scenario-controlled, D-22): `--audio-raw <file>` (dense bytes), `--psg-log-out <file>` (one ASCII line per record: `index tick addr value source`), `--audio-wav <file>` with `--wav-rate <44100|48000>` (default 48000), `--audio-out-max-bytes <n>` (default 256 MiB; exceeding it is a harness failure, not truncation). Files are created exclusively (refuse to overwrite) and written only in `hostio.c`. The runner streams per slice into reused buffers (about 1.1 MiB audio + about 2.3 MiB log at a 2^22-tick slice, inside the 8 MiB output cap), hashes nothing itself except the chain self-check, and retains only what `capture` asks.
- IVSS v1 additions (still line based, no quoting; keywords append-only):
  - `capture psglog` and `capture audio`: ask the runner to keep the write records (cap 4096) or the audio change list (cap 4096 entries) in memory for asserts; overflow is a harness failure (`capture-overflow`), never a silent drop. Digest asserts need no capture.
  - `assert psglog count ==|!= <n>`; `assert psglog <i> tick=<n> addr=<0..15> value=<hex> source=<cpu-write|cpu-dw>` (only fields named are compared; requires `capture psglog`).
  - `assert audio emitted ==|!= <n>` (samples); `assert audio at <tick> A=<n> B=<n> C=<n>` (tick must be a multiple of `audio_sample_ticks`, else parse-level refusal at run time with the real P; requires `capture audio`).
  - `assert digest audio=<hex>` and `assert digest psglog=<hex>` (subjects extend D-18's `digest <domain>`).
  - No directive names a file, a rate or a WAV: listening is operator-side only.
- Exit codes and outcome taxonomy unchanged; a failed `audio.chain.check` is `harness-fail` (output path inconsistent), with detail `audio-chain`.

### AU-10 Memory cost and streaming

- The core retains O(1): PSG state (tens of bytes), 40-100 bytes of observation rows. It never retains samples or log records.
- Long runs: the dense stream is 0.9 MB per emulated second; a 2^40-tick default `--max-ticks` budget would be about 275 GB if retained, which is why the runner streams slice by slice (1 MiB at 2^22 ticks) and keeps nothing unless a `capture` or an operator file is requested. The wall cap (30 s) and the output-file byte cap bound disk use.
- Gate scenarios for audio run 0.1-0.5 emulated seconds (about 12-110k samples), so the sparse chain, not the dense hash, dominates (hundreds of records), and generation cost stays below the CPU cost of the guest.

### AU-11 Independent evidence and fixtures

- A project-authored original diagnostic (`diag-psg-1`, hand-assembled listing per D4) writes tone A period, volume, and mixer enable, then idles. Expected values, derived without running the core: the write-log ticks from the CP-1600 cycle table and the boot path (up to the one-cycle-granular intra-unit offset rule, tagged); the sample sequence from the data-sheet relation (full output period = 2 x TP samples of 8 PSG clocks), at the initial phase policy (tag unknown-default-chosen). These land in a `.expect` golden (tier `original-diagnostic`, oracle `independent-derivation`); digests land in a `.digests` golden (tier `deterministic-repeatability`, oracle `self-recorded`).
- Claim wording stays profile-bound: no "PSG accurate" claim; at most "write-log timing and sample grid follow the stated profile rules, repeatable" with tier tags per rule.

### AU-12 D7 (reference PCM location) and the converter shape

- No PCM, rate, filter, DAC table use or WAV code in `ivs_core` or in any installed artifact. `tools/pcm/` holds a pure integer reference converter (two files, `ivs_refpcm.h/.c`: no I/O, no allocation, no floats, no globals, includes only `<stdint.h>`), built as an internal static target linked by the runner and by its own test suite only. A symbol deny list (`tools/check/core-symbols-deny.txt`) gains `refpcm`, `resample`, `wav`, `pcm` so the gate proves the core archive has none; `pkg-install-files.ids` proves the install prefix does not carry it.
- Algorithm (golden-able; exact numbers belong to the plan, shape fixed here):
  1. Input: the dense level stream (or the change list), a 16-entry unsigned DAC table from the profile record (tag unknown-default-chosen; default a 3 dB-per-step integer table derived from `round(10000 * 2^(-(15-n)/2))` computed at authoring time and written as literals, sum of three channels never exceeding 30,000 so nothing clips), `crystal_hz_num/den` and `P` from `ivs_info`, output rate R (44,100 or 48,000).
  2. Mix: `x_k = dac[A_k] + dac[B_k] + dac[C_k]` (unsigned 32-bit).
  3. Exact rational box filter (integrate and dump): output sample j covers master ticks `[j*num/(den*R), (j+1)*num/(den*R))`; work in units of `1/(den*R)` tick so every window is exactly `num` units wide, every cell overlap is an integer, and `out_j = round_half_up(sum(overlap_i * x_i) / num)`; no float, no drift, no per-sample accumulator error. Magnitudes stay below 2^40 per window, inside u64.
  4. DC block: one-pole high-pass `y_j = x_j - x_{j-1} + (y_{j-1} * 32763) / 32768` with a defined floor-division helper (no right shift of negative values in C), state carried across calls in a caller-owned struct, so slicing the conversion does not change the output (tested).
  5. Output signed 16-bit mono, saturating (never expected to saturate with the default table).
- It is deliberately simple and aliases (the 5.07-input and 4.66-input box windows at 44.1 and 48 kHz fold high harmonics); it is a reference for listening and for objective frequency checks, not a high-fidelity renderer and not chip truth. Label: "reference conversion v1".
- Evidence class: the conversion has its own tests: golden vectors for tiny hand-computed inputs, invariants (silence in, zeros out; constant level settles to 0 after the DC block; frame split invariance; a given tone's zero-crossing frequency within 1% of `crystal/(32*TP)`), named faults (`PCM_FLOOR_ROUNDING`, `PCM_DC_STATE_CARRY`, `PCM_RATE_DRIFT`), and a `.refpcm` golden (tier deterministic-repeatability, oracle self-recorded, header says "converter repeatability only"). Nothing in `digest.audio`, `digest.psglog`, claims or the important-gates registry for AUD-02 depends on it.
- Integrators: copy `tools/pcm` (two files) or write their own; an adapter that must produce bit-identical PCM across hosts is the trigger for promoting it to an installed `ivs::refpcm` target (section 8), still outside the core.

### AU-13 Human-listenable check, and the handoff

- Automated (no human): everything in AU-11 and AU-12, including the objective pitch check on the reference PCM and the `audio.chain.check`.
- Handoff, because tools cannot judge it: "does the three-voice diagnostic sound like a plausible Intellivision PSG at a glance" (timbre, relative level, noise character). The handoff is one local command, e.g. `sh tools/listen-demo.sh` (local-only helper, not a gate step), which builds `dev`, runs `diag-psg-1` with `--audio-wav build/listen/psg-demo.wav` and prints the WAV size and `refpcm.sha256`. The VERIFICATION record lists what was automated, what was handed off (listening only), and the expected sha256 so the owner can confirm they heard the reproduced file. The result is a note, never a claim: no audio-accuracy tier is awarded by listening, and comparison against a real console recording is D10 (owner hardware) and stays marked missing.
- The WAV is not committed and not uploaded; the hygiene binary list and D-43 are untouched. The listening helper refuses `content ... user` scenarios (derived audio of rights-bearing content stays out of shared records).

## 5. Proposed Decision Register text (D7)

| Gate | Decision | Would reverse it |
|------|----------|------------------|
| D7 | Reference PCM is not in the core and not in any installed artifact. The core's audio observations are raw integers with no host rate and no analog constant: the PSG register-write log (domain 19, 16-byte records stamped in master ticks) and chip-rate level samples (domain 17; one 4-byte sample of three channel amplitude indices per chip tick, the chip tick being 8 PSG clocks = 16 master ticks on the NTSC profile, stamped by index x period, never per sample). Reference PCM is a pure integer conversion (exact rational box filter, integer DC block, no floats) in `tools/pcm/`, linked only into the runner's optional operator-side `--audio-wav` and its own tests, documented as copy-paste for hosts, labelled "reference conversion, not chip truth", and excluded from digests, claims and AUD-02 evidence. | Two independent hosts, or a host adapter, that must reproduce bit-identical PCM (promote to an installed `ivs::refpcm` target, still outside the core); evidence that listeners or a consumer need a core-side rate/filter contract; a primary source that fixes the DAC law so a mixed-sample surface becomes honest evidence (then add a mixed field as a new format id, not a change to levels) |

Also record: C23 `status: resolved-primary` is wrong (no primary source); use `partial` with note "decided by D7 on architecture grounds; the DAC table stays unknown-default-chosen (C12)". New discrepancy entries (next free ids): sample grid and chip tick (`psg.chip_tick_psg_clocks`), write stamp instant and tie rule, upper-bits of a PSG write (`value` raw 16-bit vs 8-bit register), power-on counter phase; each with a D10 resolver.

## 6. Footguns and anti-patterns avoided [BG unless noted]

- Floating-point mixers and platform-dependent hashes (Pitfall 12, [OBS] in research): none in the core or the converter.
- Resampling in the evidence path: avoided by domain separation and the symbol deny list.
- Sample-rate-coupled constants inside the chip model (44.1 kHz in a PSG): `P` is derived from the divider and the profile ratio only.
- Audio-driven timing: the host paces; the core only reports `now_ticks`.
- Frame-quantized audio (emitting "per frame" chunks): sample counts per frame are fractional here (3,733.5); the stream is continuous.
- Slice-dependent catch-up (advancing the chip to the target): forbidden; catch-up goes to `now` or the write stamp. This is the bug class Phase 4 D-07 already guards; a named fault covers it.
- Hash-per-sample cost and shared "streaming hash context in state": avoided (change chain).
- Silent buffer loss: capacity 0 is explicit, counts are reported, the chain self-check catches mismatches.
- Retaining unbounded output in the core: nothing retained.
- DAC-curve tables copied from other emulators: forbidden (AGENTS.md); the default table is project-authored arithmetic, tagged unknown-default-chosen.

## 7. Time-base and glossary follow-ups

- `docs/time-base.md` glossary adds: **PSG chip tick** (8 PSG clocks; 16 master ticks on the NTSC profile; tag inference), **Audio sample** (one chip tick's three amplitude indices, stamped by index x period), **Reference PCM** (reconversion outside the core). "PSG step" (16 clocks) stays and is described as the tone-period count unit. `data/expected/glossary-terms.ids` and `profile-fields.ids` gain the new terms and keys (`psg.chip_tick_psg_clocks`, `psg.sample_period_ticks`, `psg.write_strobe_ticks`, `psg.dac_levels`, `psg.log_records_per_frame`, `max_unit_ticks`); every key holding a period or duration ends in `_ticks` (existing rule).
- `docs/audio.md` (new) states the surface; `docs/runner.md` is updated for `ivsr 2`, options and `capture`.

## 8. Reversal triggers (what would change which decision)

| Decision | Reverse or revisit when |
|----------|-------------------------|
| P = 16 ticks (AU-03) | Primary source or D10 capture shows sub-grid timing (use P = 8) or a counter that updates at 32 ticks without half-steps (P = 32); the write log (domain 19) is unaffected by design |
| Levels not mixed (AU-03) | A primary source fixes DAC law and mix; then add a new format id with a mixed field |
| Change-chain digest (AU-06) | A consumer needs the digest to equal a plain hash of the dense bytes (add a second, separately named domain), or the chain misses a bug class the dense hash would have caught (then per-block hash with an instance hash context, accepting state growth) |
| Worst-case log capacity L1 (AU-02) | Hosts report that the bound forces buffers far above real volume, or Phase 6 events have no cheap bound: define L2 with a new stop reason `OUTPUT_FULL`, reserved and pinned in `api-stop-reasons.ids` at that time |
| Frame-slot audio formula (AU-05) | A real host single-steps with audio and objects to a 15 KiB buffer: publish the tick-tight formula via `ivs_info` fields |
| Capacity 0 = not requested (AU-07) | A silent-loss bug is found in a host; then restore refusal and add an explicit discard flag |
| No PCM in core (AU-12) | Listed in the D7 text |
| Raw 16-bit value in the log | Evidence that registers read back width-masked values the guest depends on: add an effective-value field as a v2 record |
| Reads not logged | A controller or PSG test needs the guest-visible read sequence: domain 19 v2 adds read records, or a separate read-log domain |
| `ivsr 2` bump | Not reversible without a goldens refresh; pick once |

## 9. Adversarial pass (against this recommendation)

1. "The change-list chain is not evidence of the delivered dense bytes." Answer: dense and change list are a bijection given `emitted`; the runner's `audio.chain.check` recomputes the chain from delivered bytes every run that delivers audio; a named fault (`AUDIO_DELIVERY_SKIPS_SAMPLE`) must trip it. Residual: a host that does not deliver audio gets only the core's chain, which is the claim the core can make.
2. "P = 16 is a guess (8914 internals)." True: it is tagged unknown-default-chosen and derived from a data-sheet divide chain plus an emulator-hint clock ratio. The two-domain split means a wrong P only costs a domain-17 golden refresh and a new profile revision; the cause stream and every guest-visible result are unchanged. Honest limit: no sample-level accuracy claim until D10.
3. "Per-channel indices push the mix problem onto hosts." Intended: the mix law is unknown and a core constant would be a claim without evidence. The reference converter gives hosts a starting point and the data sheet's "4 bits of logarithmic" gives the default table its shape.
4. "Worst-case log capacity is wasteful." About 35 KiB per frame slot against tens of records of real use is tolerable at this scale; the refusal contract stays uniform (API-04). The L2 path is documented and cheap to add later.
5. "Capacity 0 weakens API-04." Undersized-present buffers are still refused before any state change; the zero case is now an explicit, reported opt-out, and the property "buffer presence never changes state" is tested. The cost is a reviewed amendment of Phase 4 D-06 and about five test flips.
6. "ARCHITECTURE.md put reference PCM in the core observation layer." That text predates the Phase 4 observation-row design and C23; the core would then own a rate, a filter and an unknown DAC table. Two of three research inputs and Pitfall 12 argue for separation; `tools/pcm` gives the same reuse without ABI.
7. "The write stamp is only CPU-cycle exact and its instant within the cycle is unknown." Yes: `psg.write_strobe_ticks` is a tagged profile constant (default 0), the tie rule is tagged, and both are D10-resolvable (a captured tone start against a known write). At 16 ticks per sample a one-cycle (4 ticks) error shifts a tone phase by at most a quarter sample; no claim is made below the tag.
8. "Unit bound `U` may be wrong, especially once Phase 6 steals cycles." Likely the weakest link. Mitigations: `U` is derived and property-tested against the CPU model; the pre-unit reservation converts a violation into `IVS_STOP_INTERNAL` at a whole boundary (never a buffer overrun or a half-written state); Phase 6 must re-derive `U` and `W_log` when it adds bus stalls (note for its discuss); if stalls make units long, switch the sink to L2 back-pressure rather than inflate the slack.
9. "Two new buffers and a v2 request is API growth for a pre-1.0 core." Both are append-only, minimum size unchanged, no new function; the alternative (one muxed buffer or a read call) costs more.
10. "Tie rule order could be wrong." Only affects behavior when a write lands exactly on a grid tick (every 4th CPU-cycle position); the rule is arbitrary but total, deterministic and tagged, so it cannot cause nondeterminism, only a possibly different phase of one sample.

## 10. Open items and cross-cutting dependencies

1. PSG device-state digest domain and composite: new state domain id (5..15) vs composite v2. Needed before the PSG plan; my surface does not depend on the choice (domains 17 and 19 stay out of the composite).
2. Derive `U` (max unit ticks) and `g` (min ticks between writes) from the CP-1610 model with a property test; both feed profile data and the capacity formulas. Re-derive in Phase 6.
3. Define the 8914 behaviors that change the amplitude index (2-bit envelope select, C11/C12) as tier-tagged profile data; check the datasheet extraction and the 16-step claim against the 8914 note in Pitfall 12; add vectors per rule (SC4).
4. Value of the `value` field's upper byte on a PSG write (the 16-bit bus carries 8-bit registers); the log keeps raw, the register store masks per profile (C60 neighbour).
5. Decide the `ivsr 2` bump timing with the controller agent (INP adds `input.*` keys too) so the report format changes once.
6. Phase 4 amendments: request/report/info growth and the zero-capacity rule must be written into Phase 5 CONTEXT as an amendment of D-06 and A-02 with the exact test flips (`run.*` capacity tests), and `data/api/api-effects.json` rows.
7. Check that nothing in `runner-scan` (the POSIX and file-write confinement) forbids the exclusive-create file writes for `--audio-raw`, `--psg-log-out`, `--audio-wav`; they belong in `hostio.c` only, like `--update-golden`.
8. New named faults to register (one file each, per the registry rule), illustrative: `AUDIO_CATCHUP_TO_TARGET`, `AUDIO_TIE_WRITE_FIRST`, `AUDIO_STATE_DEPENDS_ON_BUFFER`, `AUDIO_SLACK_ZERO`, `UNIT_BOUND_EXCEEDED`, `AUDIO_DELIVERY_SKIPS_SAMPLE`, `PSGLOG_STAMP_UNIT_START`, `PSGLOG_DROPS_DW`, `PSGLOG_CAP_AFTER_STEP`, `AUDIO_CHAIN_PER_RUN`, plus the three `PCM_*` faults. `important-gates.ids` gains rows for audio slicing invariance (AUD-02, API-02), audio/log capacity (API-04), chain format oracle (RUN-05), and reference conversion determinism (kept separate, not an AUD-02 row).
9. Evidence-tier wording: add or reuse tier ids so `.expect` for the log ticks says `original-diagnostic` with the intra-unit rule tagged, and the sample vectors cite the data-sheet relation (tier 1 for the 8910 relation, tier 4 for the 8914 map).
10. Rights note for the claim ledger: the 8910 text was read from a local reference cache (bitsavers catalog); only the relations above (clock divide ratios, 16 levels, 4-bit logarithmic) are used, no table or figure is copied.

## 11. Three riskiest unknowns

1. The sample grid and chip micro-architecture (P = 16 ticks, half-step toggling, power-on phase, 8914 behaviors) rest on a 8910 data sheet plus an emulator-hint clock ratio; no tier 1-3 source or measurement exists (C11, C12, D10).
2. The unit-length and write-gap bounds (`U`, `g`) that size the buffers: unproven against the CPU model today and certain to be stressed by Phase 6 bus stalls; a wrong bound is an INTERNAL stop, a too-large one wastes memory, and the contract may need the L2 back-pressure design later.
3. Write-stamp fidelity: the instant within a CPU cycle at which a write reaches the chip, and the step-then-write tie rule, are arbitrary tagged policies with no hardware oracle; they set the phase of the first audible cycle and the byte-exactness of every audio golden.
