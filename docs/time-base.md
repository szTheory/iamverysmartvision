# Time base and glossary

This document fixes the time vocabulary that every later document, trace, golden file and API uses.
It records decision D2 and holds the glossary required by FND-07.
The gate checks that every glossary term is present and that the bare unit word never appears in `README.md` or `docs/`.
Evidence tags in brackets follow `docs/evidence-tiers.md`.

## Decision D2

D2 is a delegated architecture decision, adopted under the owner authorization of 2026-11-09 and recorded in `.planning/PROJECT.md`.
The canonical machine-time unit is the master tick, defined as one period of the profile's crystal.
A master tick is a `uint64_t` counted from reset, with origin 0.
The CPU cycle is a profile ratio, `ticks_per_cpu_cycle`, which is 4 for the NTSC profile [INF].
The core run contract is `run(target_tick)`, which returns `now_tick`, `overshoot_ticks` and a stop reason.
A `target_tick` above 2^63 is refused with an explicit stop reason, so the arithmetic never wraps.
The master tick is a unit, not a stepping granularity.
The CPU still advances in whole CPU cycles, and the other devices catch up to the CPU position (Option B catch-up scheduling), with no lockstep.
Rationale: one integer unit lets every device keep an exact rational relation to the crystal, so a trace, a golden file and a save state never depend on a host timer or a floating-point value.
Rationale: expressing time in the finest common unit means a device with a different divisor needs a new ratio in the profile and no change to the unit.
Reversibility: one-way once a Phase 2 API ships, because the unit appears in the ABI, traces, save states and goldens.
What would reverse it: evidence of a device timing source that is not an integer divisor of the crystal.
Wall time and host timing are never machine time anywhere in this model.

## Nominal NTSC numbers

The profile holds the figures below, and each carries the tag shown.
A frame is 262 scanlines of 57 CPU cycles, which is 262 x 57 = 14,934 CPU cycles (tag `tier4-community`, discrepancy C7).
At 4 master ticks per CPU cycle that is 14,934 x 4 = 59,736 master ticks per frame (tag `inference`).
The crystal is the nominal 3,579,545 Hz recorded as a rational in the profile (tag `tier1-primary-doc`), read only by host adapters.
The length of a frame in master ticks is constant for this profile, whatever the display state or the bus stalls: stalls and the display latch change how many CPU cycles the CPU executes inside a frame, not how long the frame is (Phase 6, D-07).
A frame is a timeline event and is never derived by modulo of the master-tick count: the frame index and the event cursor are machine state, and the index is not a function of the tick count, a rule about state that holds although the two coincide numerically while the frame phase is 0.
The API publishes the frame length in master ticks as `ivs_info.nominal_frame_ticks` (59,736 on the NTSC profile), which is exact for this profile; the frame event is what the run report publishes (D-09, corrected in Phase 6).

## Glossary

Each entry is a term in bold, a definition, and where useful an `_Avoid_:` line.
The term inventory is `data/expected/glossary-terms.ids`.

**Crystal**: the master oscillator of the console, held in the profile as the rational `crystal_hz`.
The core never reads the value in Hz, and only host adapters convert it to wall time.
_Avoid_: calling the oscillator "the clock" [nc].

**Master tick**: one period of the crystal, and the canonical machine-time unit.
It is a `uint64_t` counted from reset, and core internals, traces and goldens all use it.
_Avoid_: "cycle" or "clock" for a master tick [nc].

**CPU cycle**: the CP-1610 micro cycle of the GI manual, and the unit of every manual cycle count and of the 14,934 per frame.
On the NTSC profile one CPU cycle is 4 master ticks, an inference tagged `inference` in the profile.
_Avoid_: using "clock" for a CPU cycle, because the manual counts cycles and the input rate differs [nc].

**CPU input clock**: the two-phase input to the CPU, where one CPU cycle is two input clock periods according to the manual (`SRC-gi-cp1600-um-1975`).
On the NTSC profile it is the crystal divided by 2, about 1.79 MHz, so one CPU cycle is about 1.1175 microseconds [INF].
It exists in the glossary only and is never a code unit, because the profile carries the ratio to the CPU cycle instead.
Confusing this input clock with the CPU cycle rate of about 894.9 kHz is discrepancy C8, and the glossary keeps the two apart.

**Time slot**: one of TS1, TS2, TS3 and TS4, which are four per CPU cycle.
A time slot is a quarter of a CPU cycle, so on the NTSC profile it is 1 master tick.

**STIC clock**: the STIC input rate, which is one master tick per STIC clock on the NTSC profile (tag `tier1-primary-doc`, key `ticks_per_stic_clock`).

**PSG chip clock**: the PSG input rate, which is two master ticks per PSG chip clock on the NTSC profile (key `ticks_per_psg_clock`).
The profile tags it `unknown-default-chosen` today; Phase 5 retags it `inference` (D-09, C76), because the ratio agrees with three facts that are not emulator code: the input range of 1.0 to 2.0 MHz printed for the AY-3-8910 (`SRC-gi-1980-catalog`, pdf 209), the community tone formula of the crystal divided by 32 times the period (tag `tier4-community`), and the crystal divided by 2 giving about 1.79 MHz.
An emulator-level hint gives the same ratio and is never evidence.

**PSG step**: 8 PSG chip clocks, which is 16 master ticks and 4 CPU cycles on the NTSC profile (`ticks_per_psg_clock` 2 times `psg.clocks_per_step` 8, tag `inference`, D-09, C69).
Tone half-period is P steps, a noise shift happens every 2 NP steps, and an envelope step every 2 EP steps, with P, NP and EP the programmed periods.
The step is the finest time at which any PSG output can change; counters are integers and there is no accumulator.
Samples per nominal frame are 59,736 master ticks divided by 16, so 3,733.5, which means a frame is never PSG-aligned.
This replaces an earlier statement of 16 PSG chip clocks per step, which the printed divide chain and the tone formula do not match.
_Avoid_: stating the PSG step in master ticks without naming the ratio that produced the number.

**Frame**: a STIC timeline event of constant length `ticks_per_cpu_cycle` x `cpu_cycles_per_scanline` x `scanlines_per_frame` = 4 x 57 x 262 = 59,736 master ticks (14,934 CPU cycles at 4 master ticks per CPU cycle) for this profile.
Frame k starts at `S_k` = `stic.frame_phase_ticks` + k x 59,736 and ends at `E_k` = `S_(k+1)`.
Stalls and the display latch change how many CPU cycles the CPU executes inside a frame and never the length of the frame.
The frame index and the event cursor are machine state, never derived by modulo of the tick count.

## Units in records and code

The bare unit word is banned in `README.md` and `docs/`, and the gate fails a line that uses it without a qualifying compound.
A profile key that holds a duration or an offset ends in `_ticks` or `_cpu_cycles`.
A ratio or count key named `ticks_per_X` or `X_per_Y` already names its units and complies, and `crystal_hz` is a rational frequency that only host adapters read.
The glossary step checks the rule on every field path in `data/expected/profile-fields.ids`.
A field whose last segment holds one of the tokens window, offset, hold, duration, delay, period, latency, interval or timeout must end in `_ticks` or `_cpu_cycles`.
Profile values are converted once, at load, to master ticks.
Core internals, traces and goldens use master ticks.
A CPU-cycle convenience helper lives in the runner, not in the core API.
Wall-time and PCM conversion happen only in host adapters, with exact rational accumulators.

## Queued for Phase 2

Phase 1 checks this vocabulary in documents and in profile keys, because no code exists yet.
The code-level check of FND-07 is queued for Phase 2 together with the items below.
A named-fault control for a unit swap, where a CPU-cycle count is passed where master ticks are expected.
A check that the NTSC frame is 59,736 master ticks and 14,934 CPU cycles.
A freshness check for the generated header derived from the profile record.

## Integrator note

A future LibFuturo adapter needs its own Intellivision master-tick unit, and it must not reuse the unit that LibFuturo names for another machine.
Any change to a shared contract is owner decision D13 and is not part of this milestone.
