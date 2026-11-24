# Phase 6, decision point: frame timing, VBLANK interrupt, BUSRQ/BUSAK bus steal and rendering granularity (VID-01, VID-02, VID-03, VID-06; PIT-9; C1-C7, C10, C21)

Status: discuss output (fan-out protocol), not yet adopted. Labels: [OBS] read in this repo or in a cited document or executed here, [INF] inference, [REC] recommendation, [UNK] unknown, [BG] background knowledge not re-fetched.
Scope: only the timing decision point. The raw framebuffer geometry (D8, C10), the card renderer and the STIC register file layout belong to sibling decision points; this note states what it needs from them as parameters (`video_bytes_per_frame`, a per-row fetch latch).

Inputs read [OBS]: `AGENTS.md`; `.planning/AUTONOMY.md` sections 1-3; `.planning/PROJECT.md` (D2, D3, Phase 5 notes); `REQUIREMENTS.md` (VID-01..06, CPU-02, CPU-05); `ROADMAP.md` (Phase 6, PIT-9); `docs/time-base.md`; `docs/cp1610/{reference-notes,stepper-conventions}.md`; `data/discrepancies.json` (C1-C11, C19, C21, C38, C40-C45, C52-C57, C60, C61, C73-C78); the profile `intv-2609-ntsc-family-p1`; `core/src/cpu/cp1610.{h,c}` (unit stepper, grant, interrupt entry), `core/src/machine/{machine.h,machine_run.c,machine_bus.c,machine_bound.c,machine_digest.c}`, `core/include/ivs/ivs_run.h`, `tests/psg/psg_bounds.c`; Phase 5 `05-CONTEXT.md` and `05-DISCUSS-audio.md`.
Primary pages read for this note [OBS], from the gitignored cache and rendered into the scratch directory: GI 1980 catalog printed 5-10, 5-11, 5-12 (pdf 191-193) and the identical 1982 reprint printed 9-43, 9-44, 9-45 (pdf 462-464, including the System Synchronization Timing figure); CP-1600 manual section C.5/C.6 (interrupt and bus request text, pdf 24-25) and the interrupt-system overview text near the end of section 2 ("each device can request service on INTRM*... the service program can then disable the INTRM* line and poll via BEXT... reenabled by EIS").
No emulator source was opened (AGENTS.md no-copy rule). The community figures below come only from the project's own tier-4 records (`data/discrepancies.json` C1-C4, C7, written in Phase 1); nothing was copied from another emulator.
Derivations were done with throwaway scripts in `<scratch>` (listed in section 5 so they can be re-run); none is repo code.

---------------------------------------------------------------------------------------------------

## 0. Result in one screen

| # | Decision | One-line statement |
|---|----------|--------------------|
| T-1 | Frame structure | A frame is `F = 262 x 57 x 4 = 59,736` master ticks, constant in ticks. Frame k starts at `S_k = frame_phase + k*F` (phase 0 default) at the STIC's SR1 event, which is the VBLANK start. Frame index and event cursor are STATE (not derived from `now` by modulo), so a future variable-length profile needs no state change. |
| T-2 | VBLANK interrupt | INTRM is a LEVEL driven by the STIC: asserted from `S_k` for `irq_low = 3,791` CPU cycles (C3 default). The CPU sees it through the existing POL-LINE-01 rule (sample at unit start minus 1 CPU cycle). The CPU stepper is unchanged: acceptance, SDBD, C19/C56 interruptibility, INTFF, IAB `$1004`. Nothing is latched. |
| T-3 | BUSRQ/BUSAK | 13 bus windows per frame (GI "thirteen or fourteen" SR2 pulses; 13 chosen) of `H = 114` CPU cycles each at pitch 912 (16 lines x 57), first at cycle 3,791. The stall is the EXISTING `BUS_GRANT` unit sequence of the stepper (1-cycle grant and hold units, 2-cycle release unit): no CPU change, no bus-wait reply, no collapsed stall unit. Closed form: CPU resumes at `w + 4*(H + 3)` ticks whenever it was granted before the window ends. |
| T-4 | What U and g become | `psg.max_unit_ticks` U = 468 and `psg.min_write_gap_ticks` g = 28 are UNCHANGED (stall units are 1 or 2 cycles; stalls and the new interrupt only insert time). What MUST change: the probes are re-run through the full machine with the STIC lines live, two stale comments are corrected, and the VIDEO capacity formula needs `+2` frames, not `+1` (Phase 4's `+1` is violated in a corner, shown below). |
| T-5 | Catch-up discipline | The STIC is a lazily advanced device like the PSG: advanced to every unit end and to the stamp of every access that can observe or change its inputs (STIC registers, GRAM/GROM, BACKTAB writes). Stamp = unit start + offset x ticks per cycle + `stic.access_strobe_ticks` (0). Events at tick t are processed before an access stamped t ("step first, then access", the C78 convention). |
| T-6 | Access windows and handshake | Register window = first `reg_window = 2,000` cycles of the frame (C1 short figure), GROM/GRAM window = first `gmem_window = 3,780` (C2 short figure), both a pure function of the access stamp and applied whatever the display state; a blocked write is dropped, a blocked read returns the profile's undriven value (C5). The display-enable latch is cleared at every frame start and set by a write to `$0020` inside the register window (C6). |
| T-7 | Rendering granularity (PIT-9) | HYBRID: row-granular FETCH (the BACKTAB row is latched at each bus window), frame-granular RENDER (at the frame end `E_k`, from the latches). Justified by the GI text itself: the graphics memory sits on a separate bus, and the access windows freeze registers and GRAM during graphic time, so BACKTAB is the only input a guest can change mid-display. Static fixtures render identically under every alternative. |
| T-8 | Frame boundary and run flag 2 | A frame completes at the first unit boundary at or after `E_k = S_{k+1}`. `IVS_RUN_F_STOP_ON_FRAME` (= 2) clamps the target to the next frame end and stops with new `IVS_STOP_FRAME` (= 5). It is exactly `run(min(target, E_next))`, so slicing invariance is inherited. Report v3 appends `video_first_index`, `last_frame_end_ticks`, `next_frame_end_ticks`. |
| T-9 | Digest and state | No new composite domain id: STIC registers join MEM (domain 2), STIC timeline state and fetch latches join INSTANCE (domain 4); composite version 2 -> 3 (deliberate, classified re-bless, v2 literals pinned first, as in plan 05-03). Domain 16 (VIDEO) becomes a per-frame chain. With the display never enabled and INTFF never set, every Phase 4 and 5 TIME golden is unchanged; only digest lines move. |
| T-10 | Profile | 19 new tier-tagged flat scalar fields under `stic.*` and `bus.line_sample_lag_cpu_cycles`; discrepancies C81-C86 proposed (planner owns the final allocation); C1-C7, C38, C41, C52 gain notes. Every C1-C4 value stays `unknown-default-chosen`. |

---------------------------------------------------------------------------------------------------

## 1. What the primary text prints, and what it does not

[OBS] GI 1980 printed 5-10 and the identical 1982 printed 9-43, "AY-3-8900 / AY-3-8900-1 Standard Television Interface Chip":

1. NTSC part is the -1, clocked from 3.579545 MHz; 8 foreground objects "on a grid of 167H by 105V of which 159 x 96 are visible positions"; background 20 x 12 cards, 19 columns of 8 x 8 and a 20th of 7 x 8; background memory 240 x 14 bit.
2. "The main synchronization which operates at the T.V. frame rate uses SR1, SR2 and SR3. The SR1 signal occurs once per frame and it is used to synchronize the CPU algorithms to the intended display sequences. SR1 indicates that STIC time is complete and that the AY-3-8900-1 has switched to the CPU controlled mode."
3. "SR2 is issued thirteen or fourteen times per picture frame depending on picture offset. The AY-3-8900/8900-1 takes this signal low to request the first line access for a new row of twenty characters."
4. "SR3 ... pulses positive for each character position. Once the first line has been accessed by the SR2, SR3 combination, the following fifteen lines to complete the 8 x 8 array are fetched by SR3 alone." That is 16 TV lines per card row. SR3 is also issued in CPU-controlled mode "in response to BAR, ADAR or DW to enable an external device onto the 14 bit BUS".
5. "The CPU control BUS is only validated if the AY-3-8900-1 is in the CPU controlled mode, otherwise it is ignored." (so a CPU access during STIC time does nothing on the STIC side).
6. The graphics (glyph) memory has its own control bus (BAR', DTB', DWS'), used "during STIC time in the fetch of foreground and background object patterns"; "during the non STIC time ... the graphics control BUS can be used to link the memory area containing the graphics patterns to the main memory area of the external microprocessor".
7. Figure "System Synchronization Timing" (printed 5-12 / 9-45): a short SR1 pulse; "CPU TIME" directly after it; "GRAPHIC TIME" as a train of SR2 pulses (the expansion shows one SR2 low per card row, and per row 16 lines of SR3 bursts, "LINE 1 ... LINE 16", then LINE 1 of the next row); the next SR1. The figure is schematic and elided (a break inside the SR2 train): no pulse can be counted from it and it carries no numbers.
8. Figure "AY-3-8900-1 CPU CONTROL TIMING": CPU address/read/write codes decoded against the CPU's phase-1 output and its time slots 1..4: the STIC interface runs on the CPU slot structure, the STIC's own raster on the 3.579545 MHz clock.

[OBS] CP-1600 manual: INTR/INTRM sampled "every TS1" and honoured at "the completion of any interruptable instruction"; INTRM only with INTFF set; BUSRQ honoured before interrupts; a bus grant is an indefinite WAIT that does not use the bus (section C.5, C.6; already in `docs/cp1610/reference-notes.md`). The overview text near the end of section 2 describes the intended maskable-interrupt use: the device requests on INTRM*, the vector comes from IAB, "the software service program can then disable the INTRM* line and poll ... At the completion of the appropriate service, the INTRM* line can be reenabled by the EIS command", and nested structures rely on a device-side acknowledge flip-flop (INTAK daisy chain) cleared by TCI. [INF] That is architectural support for two things already in the profile: the CPU does NOT disable itself on acceptance (C41 default 0), and a request is held by the DEVICE until serviced or withdrawn. It does not say how the 2609 withdraws the STIC request.

What no primary page prints [OBS, searched]: lines per frame, cycles per line, the position of any window in cycles or lines, the duration of CPU time, the duration of an SR2 low or of a bus hold, the number of cycles stolen per frame, the SR1-to-INTRM or SR2-to-BUSRQ wiring, the register and GROM/GRAM access windows. All of those are tier 4 or inference (C1-C4, C7 stay open). Only the following are derivable from the primary text: 12 rows x 16 lines = 192 active lines (tier 1 + inference), the ordering CPU time then graphic time inside one SR1 period, "13 or 14" bus requests per frame, and that CPU control of the STIC exists only in CPU time.

Tier table for every number this note uses:

| Number | Value | Tier | Backing |
|--------|-------|------|---------|
| STIC clock | 1 tick per STIC clock | tier1-primary-doc | 5-10 / 9-43 (profile `ticks_per_stic_clock`) |
| Active lines | 192 (12 rows x 16) | tier1 text + inference | 5-10 (20 x 12 cards, "following fifteen lines") |
| Bus requests per frame | 13 or 14 | tier1 (the set); 13 is `unknown-default-chosen` | 5-10 |
| Lines per frame | 262 | tier4-community | C7 |
| CPU cycles per line | 57 | tier4-community | C7 |
| CPU time (SR1 to first request) | about 3,791 cycles | tier4-community | C2 (3,780-3,796), C3 (3,791, alt 2,907/2,920) |
| Register window | 2,000 (alt 2,900) cycles | tier4-community | C1 |
| GROM/GRAM window | 3,780 (alt 3,796) cycles | tier4-community | C2 |
| Bus hold / acknowledge deadline | 114 / 57 (alts 110, 57/44, 68) | tier4-community | C4 |
| Grant lead and release latency | 1 cycle hold, 2 cycle release | inference | C52 (stepper conventions) |
| Line sampling lag | 1 cycle | unknown-default-chosen | C38 (POL-LINE-01) |
| Frame phase at power-on | 0 | unknown-default-chosen | new C81 |

Cross-checks that hold [INF, executed in `<scratch>/derive.py`]: a line is 4 x 57 = 228 STIC clocks = 63.695 microseconds; the frame rate is 3,579,545 / 59,736 = 59.9227 Hz (matches the community 59.92); the GIMINI figure of 63.5 microseconds per line (research-era reading of the 1977 catalog, not re-read) is within 0.3 percent of 63.695; its 45 microsecond active line is 161.1 STIC clocks, consistent with 159 visible + hidden columns inside the 167 grid and far inside 228; 192 active lines occupy 192 x 57 = 10,944 cycles, leaving 70 lines = 3,990 cycles that are not picture. None of these upgrades a field above tier 4/inference.

---------------------------------------------------------------------------------------------------

## 2. The timeline this note proposes (cycles are CPU cycles from the frame start; 1 cycle = 4 ticks; line = cycle / 57)

Frame k (`S_k` = SR1 = VBLANK start = the previous frame's end `E_{k-1}`):

| Event | Cycle | Tick (from S_k) | Line | Mechanism |
|-------|-------|-----------------|------|-----------|
| Frame start: SR1, INTRM asserted, STIC in CPU mode, display latch cleared, register and GROM/GRAM windows open | 0 | 0 | 0.0 | event FRAME_START |
| Register window closes | 2,000 | 8,000 | 35.1 | event REG_CLOSE (gate STIC_REGS) |
| GROM/GRAM window closes | 3,780 | 15,120 | 66.3 | event GMEM_CLOSE (gate GMEM) |
| INTRM released | 3,791 | 15,164 | 66.5 | pure function (level) |
| Bus window i = 0..12 starts (only if the display latch is set) | 3,791 + 912 i | 15,164 + 3,648 i | 66.5 + 16 i | pure function (BUSRQ level) |
| Row latch i (rows 0..11 only) | 3,791 + 912 i + 57 | +228 ticks into the window | 67.5 + 16 i | event ROW_FETCH(i) |
| Window i ends | 3,791 + 912 i + 114 | +456 ticks | 68.5 + 16 i | pure function |
| Window 12 (13th) starts, ends | 14,735, 14,849 | 58,940, 59,396 | 258.5, 260.5 | steals time, fetches no row in Phase 6 |
| Frame end `E_k` = `S_{k+1}`: render and publish frame k, then FRAME_START(k+1) | 14,934 | 59,736 | 262.0 | events FRAME_END then FRAME_START |

Reading it back [INF]: the picture's 12 x 16 = 192 lines fall between line 66.5+ and line 260.5, so all 13 windows fit in the frame and the last ends 85 cycles before the frame end. A 14th window at the same pitch would start at cycle 15,647, outside the frame: so "fourteen" cannot be a fourteenth regular row window at this pitch, it needs the picture offset shifting the train earlier. That is a Phase 8 (VID-05, vertical delay) matter and one reason the window table is profile data. The line numbers are relative to SR1, not to any TV sync; no claim is made about where the picture sits on a TV.

The window phase is the least certain part of this table. Only the total over the frame and the orders of magnitude have independent support (section 4.4).

---------------------------------------------------------------------------------------------------

## 3. Decisions

### T-1 Frame structure and origin

Statement [REC]:
1. `F = ticks_per_cpu_cycle * cpu_cycles_per_scanline * scanlines_per_frame = 59,736` master ticks, constant (the profile fields exist; `ivs_machine_profile_env` already computes it). `ivs_info.nominal_frame_ticks` therefore stops being "nominal" for this profile: the frame spacing in ticks IS 59,736. Cycles available to the CPU per frame vary (stalls, display state); ticks do not.
2. Frame k starts at `S_k = stic.frame_phase_ticks + k*F`; frame k "ends" at `E_k = S_{k+1}`. `stic.frame_phase_ticks = 0` (power-on aligned to a frame start, `unknown-default-chosen`, new C81, resolver D10).
3. Frame index `k` and an event cursor are device state (`ivs_stic_state`), advanced by the catch-up. The tick of the next event is computed from `(frame_index, event_cursor)` and the generated event table. Nothing is derived from `now_ticks` by `now / F`. For this profile the two coincide; making them state means a profile whose frames are not all F ticks long (a different STIC, an interlaced mode) changes only the schedule generator, not the state layout, the digest rows or the API.
4. Correct `docs/time-base.md` (the Frame glossary entry and the Nominal NTSC numbers section): replace "A frame is never derived by modulo, because stalls and the display-enable state change how many CPU cycles a frame holds" with: "A frame is a STIC timeline event. Its length in master ticks is the profile's `scanlines_per_frame x cpu_cycles_per_scanline x ticks_per_cpu_cycle` and does not change with stalls or display state; what changes is how many CPU cycles inside it the CPU executes. The frame index is machine state, not a function of the tick count." The statement "nominal, not a frame event (D-09)" is amended: nominal_frame_ticks is exact for this profile; the frame event is the thing the report publishes.

Why not derive frames by modulo (anti-pattern, lesson from other emulators [BG, generic]): an emulator that counts "CPU cycles per frame" as a constant minus a stolen-cycle estimate drifts against the interrupt phase the moment the guest toggles display enable, and one that runs the CPU in per-frame cycle budgets makes the result depend on the host loop. Absolute-tick scheduling avoids both.

### T-2 VBLANK interrupt: when raised, how accepted

Statement [REC]:
1. The STIC drives INTRM as a LEVEL that is a pure function of the frame position: asserted for `0 <= (t - S_k) < 4*irq_low` ticks, `irq_low = 3,791` cycles (C3 wiki figure, `unknown-default-chosen`; alternates 2,907 and 2,920 recorded). Independent of the display latch (the SR1 event is the raster, not the display; `unknown-default-chosen`, new C82). SR1 is a short pulse in the GI figure, so the 3,791-cycle width cannot be the raw SR1: the line is the board's latch of it. Which event releases it (first bus request, INTAK, a timer) is unknown (C82); this note models it as a fixed duration because that is what the only available numbers describe.
2. Which CPU pin: INTRM (maskable). Tier 4 (C3 says INTRM) plus the manual's own description of the maskable line; INTR is never driven by the STIC.
3. Acceptance is entirely the existing stepper behaviour: at an open boundary (previous unit interruptible, no SDBD pending) with INTFF set; priority stop, BUSRQ, INTR, INTRM (POL-INT-01); entry = INTAK, DW of PC at R6, R6+1, IAB answered by the machine bus with `interrupt.iab_value` (`$1004`), 9 cycles (C30 inference), the first ISR instruction always runs (closed boundary), MVO/PSHR/shifts/EIS/DIS/TCI/CLRC/SETC/HLT/SDBD defer it (C19 manual-partial, C56). Phase 6 changes none of it; it only drives `lines.intrm` and `lines.busrq`, which were hard-wired to 0 (`machine_run.c`: "Phase 3 has no device that drives INTR, INTRM, BUSRQ or STPST").
4. Line sampling (POL-LINE-01, C38, promoted to profile field `bus.line_sample_lag_cpu_cycles = 1`, `unknown-default-chosen`): the levels given to the step that starts at tick `b` are the schedule levels at tick `b - 4*lag` (clamped at 0). Units start on multiples of 4 ticks, so the predicate for the oracle is exact:
   - INTRM seen at a boundary starting at `b` iff `S_k + 4 <= b <= S_k + 4*irq_low` (the boundary at cycle 3,791 itself still sees it; one cycle later it does not).
   - BUSRQ seen iff `w + 4 <= b < w + 4*H + 4`.
   So an EIS executed so late that the next open boundary is at cycle 3,792 or later misses the frame's interrupt, and a CPU that is granted the bus at window 0 (cycle 3,792 earliest) never sees INTRM again that frame.
5. C41 (acceptance does not clear INTFF) stays 0. Consequence that must be stated, not hidden: with a level that is held for 3,791 cycles and INTFF not cleared on acceptance, an ISR whose first instruction is not DIS (or whose first instruction is interruptible and followed by an interruptible one) is re-entered at every open boundary until the level falls: about 3,791 / 15 = 250 nested entries of 9 cycles, one stack word each. The Phase 6 original diagnostics are project-authored, so their ISR is written DIS-first and the behaviour is identical under both C41 values; this is the robustness-by-design point and gets a vector family (both values of C41 x the ISR shapes). The manual's own text supports the default (device-side masking, software DIS/EIS, nesting without CPU self-disable), but it is not proof for the 2609 board. The first real discriminator is the owner-local EXEC run (D11, Phase 7): an EXEC whose ISR re-enters forever refutes either C41 = 0 or the level model; the one-field remedies are `cpu.interrupt_accept_clears_intff = 1` or (not implemented now) a STIC-side acknowledge that drops the level on INTAK. See section 7 (reversal triggers).
6. TCI strobes and INTAK do not affect the STIC in this phase (no daisy-chain model).

### T-3 BUSRQ/BUSAK: how many cycles, where, and how a stall appears in the unit model

Statement [REC]:

Windows. `stic.bus.count = 13` windows, window i starts at `first_start + i*pitch` cycles from `S_k` with `first_start = 3,791`, `pitch = 912 = 16 x 57`, holds BUSRQ for `H = 114` cycles (C4 wiki figure; alternates 110 and a first/last-window pair 57/44 recorded), and is present only if the display latch is set when window 0 would start (display off: no windows, the CPU keeps all 14,934 cycles, matching the community "14,934 usable with display off" and "about 13,518 with display on"). Windows 0..11 fetch card rows 0..11; window 12 steals time and fetches nothing in Phase 6 (`unknown-default-chosen`, new C83; the tier-1 fact is only the count set {13, 14}).

Stall mechanism. The stall IS the stepper's existing `BUS_GRANT` unit sequence, driven by `lines.busrq` from the schedule:
- At an open boundary where BUSRQ is seen: a 1-cycle grant unit (BUSAK set, no access).
- While BUSRQ stays seen: 1-cycle hold units.
- At the first boundary where BUSRQ is not seen: a release unit of `bus.release_latency_cpu_cycles = 2` cycles (C52), BUSAK cleared. `prev_interruptible` is untouched (A-02), so the boundary after the release runs the interrupt test, not a fetch.
- A closed boundary (non-interruptible previous unit) defers the grant; the window runs on in time. That is the "late grant".

Closed form (hand-derivable oracle, verified by `<scratch>/model.py`): let `w` be the window start tick and `H` its hold. If the CPU is granted at some boundary `b_g` with `w + 4 <= b_g <= w + 4H`, then every later boundary is on the 4-tick grid, the hold units walk to the first boundary `b_r = w + 4H + 4`, the release unit takes 8 ticks, and the CPU's next unit starts at

    R = w + 4*(H + 1 + latency) = w + 4*(114 + 3) = w + 468 ticks (117 cycles)

independent of where `b_g` fell. The cycles spent in BUS_GRANT units per window are `(R - b_g)/4`. So `R - w = 117 cycles`; per frame the time from window starts to resumes is 13 x 117 = 1,521 cycles, and the CPU executes during `[w, b_g)`. Executed cycles (INSTR units) per frame measured with the real step semantics in the model, for a uniform stream of interruptible instructions of c cycles: c=4: 13,428; c=6: 13,452; c=8: 13,480; c=9: 13,491/13,500; c=12: 13,524 (display off: 14,934). So the "usable per frame" is an OUTPUT of the model that depends on the guest's instruction mix by about 0.7 percent; it is never a constant and never a field.

Late-grant policy (new C84, `unknown-default-chosen`, one field `stic.late_grant`): the STIC does not wait for the CPU. The row latch is taken at its scheduled instant whether or not the CPU was granted (reading memory exactly as of that stamp, see T-5). A CPU that never reaches an open boundary inside the window (a run of non-interruptible instructions longer than the hold) is simply never stalled for that window; no bus conflict is modelled. A counter `late_grants` (windows that ended without a grant) is exposed through the inspection path (VID-06) so tests and diagnostics can see a non-compliant guest. The acknowledge deadline `ack_deadline = 57` (C4) is used as the DEFAULT instant of the row latch (`fetch_offset = 57`): by then a compliant CPU is frozen, so the latch reads memory the CPU can no longer change until the window ends, which makes the result independent of the exact grant phase for compliant guests.

Why not the alternatives:

| Alternative | Rejected because |
|-------------|------------------|
| Model the stall as BDRDY waits on the accesses it blocks | The manual bounds a wait to under 40 microseconds (35 cycles, C53); a hold is 114. Waits are per access and extend a unit: U would become the hold, `psg.bounds.memsys-bus-never-waits` ("a Phase 6 stall fails this id loudly") would fail, and the Phase 2 grant vectors (the part of the stepper written for exactly this) would be bypassed. |
| One collapsed stall unit of 114+ cycles | U (468) is coincidentally also 117 cycles, so it would just fit, but only by accident (U is derived from the wait cap); the single-step idiom `run(now + 1)` would swallow a window; the grant/hold/release semantics already verified by named-fault vectors would be re-implemented in the machine layer. |
| Subtract a precomputed "stolen cycles" count at frame end | Loses the interrupt/access phase, makes the result depend on call slicing, and cannot express a late grant. |
| Fixed per-row stall regardless of CPU state | Same; also wrong for a halted CPU, which the stepper already handles (POL-HLT-02). |

Cost, honestly: per-cycle hold units multiply the unit count during display. Model: instructions of 6 cycles, 3,712 units per display frame against 2,489 with the display off (+49 percent); every unit also pays the PSG and STIC catch-up comparisons. Acceptable for a first pass and for the gate; the trigger and the remedy are in section 7 (a bulk-hold fast path that is proven equivalent to unit-by-unit stepping).

Unit report: hold and release units are ordinary units (`unit_kind` BUS_GRANT, no access, event BUS_RELEASED on the release); `units_executed` counts them; `run(now + 1)` runs exactly one of them.

### T-4 Exactly what changes in the Phase 5 bounds (U, g, capacities)

[OBS] U = 468 ticks = (12 + 3 x 35) cycles x 4 (psg bounds: maximum instruction unit with every wait-taking access at the cap). g = 28 ticks = (9 - 4 + 2) cycles x 4 (MVO@ write at offset 4 of a 9-cycle unit, then an interrupt-entry DW at offset 2). `W_log = ceil(F/g) + ceil(U/g) + 1 = 2,134 + 17 + 1 = 2,152`. Audio slot `= ceil(F/P) + ceil(U/P) + 1` samples with P = 16.

[INF, with the argument]:
1. U unchanged. The longest new unit is a 2-cycle release (8 ticks); an interrupt entry is 9 cycles (36 ticks). Both are far below 468. The premise that makes U a function of the CPU alone (the memory system answers wait 0 for every access) is preserved because stalls are grant units, not waits. The only way to break U would be a stall modelled as a wait or as a collapsed unit (rejected above).
2. g unchanged. A bus grant between two writes only adds time. The new interrupt source can place an INT_ENTRY unit (DW at offset 2) after an interruptible unit, which is exactly the case the Phase 5 probe forced (it stepped interrupt-entry units with the line asserted after every opcode). Under the C56 default the MVO family is non-interruptible, so an entry cannot even follow a writer; the published 28 is the conservative bound if C56 flips. Two consecutive entries are at least one ISR instruction apart (the first ISR instruction always runs): at least 9 + 4 cycles. So no pair of PSG writes gets closer than 28 ticks.
3. `W_log` and the audio slack unchanged. In fact stalls LOWER the real write density (about 90 percent of the cycles execute), so the formula is more conservative than before.
4. What must change in Phase 6 (a list for the planner):
   a. Re-run `psg.bounds.*` through the FULL machine, not only through the bare stepper: a new family `stic.bounds.*` (or an extension of `psg.bounds.g-and-u`) that runs an adversarial writer guest with the display on, BUSRQ windows and VBLANK entries live, for several frames at every phase of the window relative to the writer, and asserts writes in any T ticks `<= (T + U)/g + 1`, no unit above 468 ticks, no wait reply above 0 from `ivs_machine_bus`. The stepper-level probes stay as they are.
   b. Update the two stale statements "no device raises an interrupt before Phase 6" / "the only density reachable in Phase 5" in the profile notes of `psg.min_write_gap_ticks` and in the `psg_bounds.c` header (comment only), and the revisit trigger text ("when a device other than the memory system can raise an interrupt or stall the bus (Phase 6)") is closed with a reference to this decision.
   c. The generator that refuses `psg.max_unit_ticks != (12 + 3 x wait_limit) x tpc` gains a second refusal: the release unit `bus.release_latency_cpu_cycles x tpc` and the interrupt entry must not exceed U.
   d. VIDEO needs its own capacity rule (T-9): the Phase 4 `+1` is not enough.
   e. Nothing changes in the audio or write-log formulas, the sample grid (absolute from reset, independent of stalls) or the log record.

### T-5 Catch-up discipline for the STIC (slicing invariance)

Statement [REC]: the STIC is a catch-up device exactly like the PSG (Phase 5 A-02, D-16), with a per-call video sink like the PSG's audio sink.

1. State `ivs_stic_state` holds the frame index, the event cursor, the display latch, the color-stack mode and running stack pointer if the renderer sibling needs them, the late-grant counter, and the row latches. The STIC registers (guest-visible) live in the memory system beside the PSG registers so reads stay pure and `ivs_machine_peek` stays side-effect-free (VID-06).
2. `ivs_stic_catch_up(params, state, mem, obs_video, to_tick, sink)` processes, in tick order, every event of the generated per-frame table with tick `<= to_tick`. Events per frame (16): FRAME_START (clear latch, open both gates), REG_CLOSE, GMEM_CLOSE, ROW_FETCH(0..11), FRAME_END. Same-tick order: FRAME_END(k), then FRAME_START(k+1). BUSRQ and INTRM levels are NOT events: they are pure functions of the frame position and the latch (T-2, T-3), evaluated by `ivs_stic_lines(params, state, tick)`.
3. It is called (a) at the end of EVERY unit, in the step seam, never to the call's target (the same rule that made the PSG slicing-invariant, plan 05-06 A-02), and (b) from the machine bus wrapper BEFORE any access that can observe or change an input of an event: every read or write whose decode is a STIC register (aliases included), GROM, GRAM (aliases included), and every write into the BACKTAB span of system RAM. The hook set is derived from the memory system's own decode (`ivs_memsys_describe` phys kinds, like the PSG test in `machine_bus.c`), never from address constants in the wrapper; the BACKTAB span is profile data (`stic.backtab_*`, from the register/renderer sibling).
4. Stamp of an access = `unit_start + offset_cpu_cycles x ticks_per_cpu_cycle + stic.access_strobe_ticks` (0, `unknown-default-chosen`, analogous to `psg.write_strobe_ticks`, new C85 text under C78's convention). Catch-up processes events with tick `<= stamp`, then the access is evaluated against the resulting gates and latches ("step first, then act"). Windows are half-open: an access stamped exactly at `S_k + 4*reg_window` is blocked; one stamped exactly at `S_{k+1}` sees open windows and lands after frame k's render.
5. Why this is exact for non-compliant guests too: an event reads only STIC registers, GRAM/GROM and BACKTAB; the only ways to change those are hooked accesses, each preceded by a catch-up to its own stamp, so an event is always processed with exactly the writes stamped before it applied and none stamped after it, whether the event happens inside a stall, between units, or inside a unit that straddles it.
6. Slicing invariance proof sketch: the machine state after a unit is a function of the sequence of (unit, access stamp) pairs; the host's run calls add no state, end at unit boundaries, and the catch-up target is always a unit boundary or an access stamp, never a call target; events are idempotent in their ordering. The proof obligation is a test family: hook completeness over all 65,536 addresses x 3 views (every address whose phys is STIC/GRAM/GROM, and BACKTAB writes, hooks), a straddle test (a writer unit crossing a row-latch tick, expected values hand-derived from the listing's cycle counts), sliced-versus-single-call `memcmp` of instances and every digest at 1, 7 and 468-tick slices over several frames, and named faults STIC_CATCHUP_AT_TARGET, STIC_BACKTAB_WRITE_NOT_HOOKED, STIC_ALIAS_NOT_HOOKED, STIC_EVENT_ORDER_START_BEFORE_END.
7. Seam order per unit (step seam): lines from the STIC at the unit start; CPU step with the wrapper; time advances; PSG catch-up to the unit end; STIC catch-up to the unit end (frame completions go to the sink).

### T-6 Register and GROM/GRAM access windows, display-enable handshake, blocked policy (VID-01)

Statement [REC]:
1. Window predicate (pure function of the access stamp): STIC registers accessible iff `0 <= stamp - S_k < 4*reg_window`; GROM and GRAM accessible iff `< 4*gmem_window`. Defaults 2,000 and 3,780 cycles (C1 and C2 short/conservative figures per their `default_policy`; alternates 2,900 and 3,796 recorded). The STIC event handlers flip the memory system's existing gates (`IVS_MEMSYS_GATE_STIC_REGS`, `IVS_MEMSYS_GATE_GMEM`, already in the digest as `mem.gate_open`), so the gate bytes are a function of machine time at the last catch-up, which is the same at every unit boundary whatever the slicing.
2. Applies regardless of the display latch (`stic.access_gating_when_off = "apply"`, `unknown-default-chosen`, new C85). Rationale: the community figures are stated relative to the VBLANK interrupt, not conditioned on display enable; and it keeps the rule one pure function. Risk and reversal in section 7. The existing stub behaviour (stub region drops writes, reads `$FFFF`) is the SAME blocked policy, so nothing visible changes for guests that never touch the STIC.
3. Blocked policy (C5, unchanged default): a blocked write is dropped; a blocked read returns `memmap.undriven_read` (`$FFFF`). No claim about real read-back of a blocked STIC or GRAM location is made (the community data point on another board reports `$00FF` and random values).
4. Display-enable handshake (C6 default): a write to `$0020` inside the register window sets the display latch for the current frame; the latch is cleared at every FRAME_START, so the guest must strobe it every frame; a latch that is clear at the first window start gives a blank frame (no windows, no row fetch, the frame is still produced and counted, content decided by the render sibling). A read of `$0021` switching color-stack mode is a read side effect: it lives in the bus wrapper, not in the memory system's read, so `ivs_machine_peek` cannot trigger it (VID-06, and the exact inspection example in the roadmap: inspecting `$0021` must not switch the mode).
5. Edge consequences worth a vector each (hand-derived from the 4-tick grid): strobe at the last allowed boundary; strobe one cycle late (dropped, frame blank); read at exactly the window edge; the same program with the two alternative values of C1.

### T-7 Rendering granularity (PIT-9), decided

Decision [REC]: row-granular FETCH, frame-granular RENDER from the fetched data, rendered at the frame end `E_k`.

Facts that make this exact rather than an approximation [OBS + INF]:
- The glyph memory is on the STIC's own graphics bus (GI text, item 6). The CPU can read or write GRAM/GROM only in CPU time (the GROM/GRAM window of C2 closes before the first window), and GROM is ROM. So glyph data cannot change while the picture is being drawn.
- STIC registers are writable only inside the register window, which also closes before the first window. So registers cannot change during graphic time either.
- The only input a guest can change mid-picture is BACKTAB, which is ordinary system RAM on the CPU bus; the STIC reads it through the bus windows (SR2, "first line of a new row of twenty characters"). The row latch at each window models that read.
Therefore a render at the frame end from (registers as of E_k, GRAM/GROM as of E_k, the 12 row latches) equals what a scanline-accurate renderer would draw, provided the windows are as modelled. A mid-frame BACKTAB write is represented faithfully (tearing at row granularity: rows latched before the write show the old data).

Why not the alternatives:

| Option | For | Against, for THIS phase |
|--------|-----|-------------------------|
| Frame-granular at one instant (snapshot at the first window or at `E_k`) | simplest code | The snapshot instant is arbitrary; a BACKTAB write between rows gets a different answer from hardware; and it still needs the same event machinery (hooks, windows) because VID-01/02 need it anyway. Saves almost nothing. |
| Row/scanline-granular rendering into a persistent framebuffer (render each row at its latch) | familiar | 15 KB of extra digest-covered state per frame in progress; per-row render code now, for an effect (mid-frame register or glyph change) that the access windows exclude; a render call per row inside the catch-up. |
| Scanline-accurate (262 events per frame, per-line fetch) | what a hardware-exact model eventually wants for MOB collision and sub-row effects | 262 events, per-line state, no hand-derivable expected values in this phase, and no source to drive it; premature. |
| Hybrid (chosen) | exact under the window model; state = 12 x 20 words; static fixtures (VID-04) identical in every option; Phase 7 scenario (input changes raw video) needs only that | Relies on the register/GRAM freeze; the freeze rests on C1/C2 estimates (see reversal). |

Forward compatibility [INF]: the fetch latch record is the natural place to grow. If a later phase finds that registers or glyphs DO change mid-frame (windows leak, or an effect like per-row border changes exists), add the register file (and GRAM bytes if needed) to the per-row latch, with the render still at `E_k` reading per-row snapshots: state grows (about 1.7 KB for registers x 13 rows), composite re-bless, no API change. MOBs (Phase 7/8): MOB registers are STIC registers, hence frozen over graphic time under the same windows; they are rendered at `E_k` from the registers (collision registers are written at render and read in CPU time of the next frame). Per-scanline MOB vertical delay and the 14-window picture offset are Phase 8 and need only the window table (data) and the render function, not a different state model.

Output delivery: the frame bytes are streamed row by row into an incremental SHA-256 (already in `core/src/common/ivs_sha256.h`) and, if the caller gave a video buffer, into it; with a NULL buffer and capacity 0 the same code runs against a counting sink (the Phase 5 rule), so machine state never depends on a buffer. No persistent framebuffer is kept in the instance.

### T-8 Frame boundary, machine-time semantics and run flag 2

Statement [REC]:
1. Definitions. Frame k covers `[S_k, E_k)`; it COMPLETES when the machine passes `E_k`, i.e. at the first unit boundary `>= E_k` (all events `<= E_k` are processed by the catch-up at that boundary, FRAME_END(k) before FRAME_START(k+1)). The frame content is fixed by events `<= E_k`; a write stamped later (inside the overshoot unit) is not in it.
2. `IVS_RUN_F_STOP_ON_FRAME = 2` (replaces the header's `IVS_RUN_F_RESERVED_FRAME`; same value; the existing "reserved flag is UNSUPPORTED" test is deliberately inverted, a reviewed change like `run.input.guest-invisible` in Phase 5). With the flag the effective target is `T' = min(target_ticks, E_next)` where `E_next = frame_phase + (frame_index + 1) x F` is the end of the frame in progress. The run proceeds exactly as `run(T')`. It stops at the first unit boundary `>= T'`.
3. New stop reason `IVS_STOP_FRAME = 5` (append-only). Order of the pure exit-state function: INTERNAL, GUEST_FAULT, CPU_HALTED (flag), FRAME (the flag is set and at least one frame completed during this call), TARGET. If `target == E_next` the reason is FRAME (more informative).
4. The existing overshoot rule is unchanged: a unit is never split; `overshoot_ticks = max(now - target, 0)` is still against the CALLER's target (not `T'`) so a host can tell "the frame stop ended before my target". The frame's nominal end is reported separately (item 5). Overshoot is derived, never carried.
5. Report v3 appends (offset 136, `sizeof` 160 on 64-bit): `video_first_index` u64 (index of the first frame completed by this call, equal to the number of frames completed before it, so the delivered streams of consecutive calls concatenate, the same device `audio_first_index` uses), `last_frame_end_ticks` u64 (the `E_k` of the last frame completed by this call; 0 if none), `next_frame_end_ticks` u64 (the `E` of the frame in progress at exit). The existing `video_frames_completed` and `video_bytes_written` become live (frames counted whether or not a buffer was given; bytes only when delivered). `ivs_info` v4 appends `frame_phase_ticks` so a host can predict the first `E` before any run.
6. Slicing invariance: a flagged run is byte-identical to `run(T')` and the schedule is state, so any slicing gives the same instance and the same frame stream; the frame count and the VIDEO chain are per frame, not per call. Test families: flagged versus unflagged equality, 1/7/468-tick slicing across several frames, frame-stop at exactly `E` (overshoot 0 and 4 to 464 ticks), flag with STOP_ON_HALT, flag with a target below `E_next` (reason TARGET), flag with a pending input event.
7. Capacity under the flag: video needs 1 frame, audio and the log use the standard formula on `T'` (so `T' - now <= F`, at most 2 slots). A host can pass `IVS_RUN_TARGET_MAX` with the flag and get exactly one frame per call; without the flag it has to know `E_next` (from `next_frame_end_ticks`) or accept more than one frame per call.
8. A FAULTED CPU is never stepped, so time and frames stop: the call returns GUEST_FAULT. A HALTED CPU keeps stepping 1-cycle HALTED units (it still grants the bus), so frames keep completing.
9. Runner (host sugar, outside the core): `run-to <n> frames` stays a nominal conversion; add `run-to frame [stop frame]` and a `frame` assertion family (frame index, `E`, VIDEO chain). Not part of the core decision.

### T-9 Video capacity: the Phase 4 `+1` formula is violated in a corner

[OBS, executed in `<scratch>/cap.py`] Phase 4's rule `required = bytes_per_frame x (max(target - now, 0)/F + 1)` assumes at most `floor(D/F) + 1` frames complete in a call of D = target - now ticks. With frames as real events that is false: take `now = E_k - 4` and `D = F - 1`; the first unit completes frame k, and the unit that crosses the target can end up to 467 ticks past it and complete frame k+1: two frames, the formula allows one. Randomised check, 400,000 runs over mixed unit lengths: 61 violations of `D/F + 1`, 0 violations of `D/F + 2` and 0 of the tight bound below.

Statement [REC]:
- Frames completed by one call are at most `ceil((D + U - 1)/F) = floor((D + U - 2)/F) + 1` for D >= 1 (an interval of length `L = D + U - 1` contains at most `ceil(L/F)` frame ends; a unit that starts below the target ends at most `U - 1` ticks past it), and 0 for D = 0. Since `U < F`, this is at most `floor(D/F) + 2`.
- Use the simple, profile-independent form for VIDEO: `required = video_bytes_per_frame x (D/F + 2)`, carried by a new env member `video_slack_frames = 1` (the audio and log channels keep their own slack inside the slot and the Phase 5 numbers, assertions and goldens stay bit-identical). The tight form is a valid later optimisation; its cost is coupling the video rule to U, its benefit is one frame buffer fewer for tiny runs (a single-step host with video requested needs two frame buffers instead of one). A host that does not want video passes NULL with capacity 0 (D-17).
- A runtime guard mirrors the PSG log: before a unit that could complete a frame, if the sink cannot hold one more frame, the run stops with the existing `IVS_STOP_INTERNAL` at that whole boundary (never expected; proves the formula only if W_log-style tests pass). Named fault VIDEO_CAP_PLUS_ONE (the Phase 4 formula) must fail the corner test.

### T-10 Digest and domain structure (composite version bump? yes)

Statement [REC], following the Phase 5 plan 05-03 pattern:
1. Guest-visible and schedule-relevant STIC state may not differ with equal composite digests. So: STIC registers (`mem.stic_reg[]`, and the gate bytes already there) join MEM (domain 2); `ivs_stic_state` (frame index, cursor, display latch, mode and color-stack pointer, late-grant counter, row latches) joins INSTANCE (domain 4). No new composite domain id. The composite version goes 2 -> 3 (the framing is unchanged; the row content changes; the version is in the preimage so v2 and v3 digests cannot be confused). Order of work, as in Phase 5: pin the v2 literals first, then bump, then re-bless with a classified diff (only composite, instance and mem lines may differ; machine time, units, cycles, stop reasons, audio and psglog chains must not).
2. With the display never strobed and INTFF never set (every Phase 4 and 5 fixture: none uses EIS [OBS, grep]), the unit sequence is identical to Phase 5, so every TIME golden (`now_ticks`, `units`, CPU cycles, report stop reasons, audio and psglog digests) is unchanged. That is a free regression anchor and a named acceptance test: "phase 6 does not move any phase 5 timing".
3. Domain 16 (VIDEO, observation, outside the composite) becomes a per-frame chain: `h0 = SHA-256("IVS-VIDEO-LOG-V1")`, `h(n+1) = SHA-256(h(n) || be64(frame_index) || be64(E_k) || frame bytes)` with the bytes streamed through the incremental hash; `count` = frames. Because a frame is a timeline event, not a call artifact, the chain is slicing-invariant by construction; the Phase 4 per-call `index || len || record` framing is NOT reused (the same lesson recorded for audio). Reported with the other observation domains; cleared by reset.
4. Layout: the instance grows by about 130 bytes of registers and about 650 bytes of state and latches (13 x 20 BACKTAB words = 520 bytes plus scalars); the total stays a multiple of 16 and `machine.h` / `machine_digest.c` static asserts and field table are updated in the same plan (the table refuses uncovered bytes).
5. Snapshot readiness (Phase 8): all STIC state is flat, fixed-width and pointer-free, with a `ivs_stic_state_validate` (cursor below the table length, latch rows below 13, flags 0/1) in the style of `ivs_cp1610_state_validate`.

### T-11 Profile data and generator (schema, tags, discrepancy ids)

New fields (flat scalars, so each carries its own tag, citation, conflict, resolving evidence and revisit trigger like every field today; names obey the glossary rule that a `window/offset/hold/period/delay/latency/interval/timeout` last segment ends in `_ticks` or `_cpu_cycles`; values are converted once at load to master ticks):

| Field | Value | Tag | Conflict | Cite / note |
|-------|-------|-----|----------|-------------|
| `stic.frame_phase_ticks` | 0 | unknown-default-chosen | C81 (new) | none-found: nothing relates power-on to the frame; D10 |
| `stic.irq_line` | "INTRM" | tier4-community | C82 (new) | C3 text; manual maskable-line description; the STIC wiring is not in the GI catalog |
| `stic.irq_low_cpu_cycles` | 3791 | unknown-default-chosen | C3 | wiki figure; alternates 2907, 2920 in the note |
| `stic.reg_window_cpu_cycles` | 2000 | unknown-default-chosen | C1 | docs figure; alternate 2900 |
| `stic.gmem_window_cpu_cycles` | 3780 | unknown-default-chosen | C2 | docs figure; alternate 3796 |
| `stic.bus.count` | 13 | unknown-default-chosen | C83 (new), C4 | tier1 set {13, 14} (5-10); 13 chosen; 14 needs the picture offset (Phase 8) |
| `stic.bus.first_start_cpu_cycles` | 3791 | unknown-default-chosen | C83 | assumed equal to `irq_low` (graphic time starts when the request ends) |
| `stic.bus.pitch_cpu_cycles` | 912 | inference | C7, C83 | 16 lines (tier1 text) x 57 (tier4) |
| `stic.bus.hold_cpu_cycles` | 114 | unknown-default-chosen | C4 | wiki figure; alternates 110, 57/44 pair |
| `stic.bus.ack_deadline_cpu_cycles` | 57 | unknown-default-chosen | C4 | alternate 68 |
| `stic.bus.fetch_offset_cpu_cycles` | 57 | unknown-default-chosen | C84 (new) | equal to the ack deadline by choice |
| `stic.bus.rows` | 12 | tier1-primary-doc | none | 5-10: twelve card rows |
| `stic.late_grant` | "fetch-at-schedule" | unknown-default-chosen | C84 | policy |
| `stic.access_strobe_ticks` | 0 | unknown-default-chosen | C85 (new) | window test instant within a cycle |
| `stic.access_gating_when_off` | "apply" | unknown-default-chosen | C85 | windows apply whatever the display latch |
| `stic.display_enable_strobe` | "$0020" | unknown-default-chosen | C6 | latch cleared at every frame start |
| `stic.blocked_write` / `stic.blocked_read` | "drop" / "undriven" | unknown-default-chosen | C5 | reads use `memmap.undriven_read` |
| `bus.line_sample_lag_cpu_cycles` | 1 | unknown-default-chosen | C38 | promotes POL-LINE-01 to a field |
| `stic.video_slack_frames` | 1 | inference | T-9 | `+2` rule; generator-checked |

Existing fields touched: `scanlines_per_frame`, `cpu_cycles_per_scanline` (tier4, C7) gain the cross-check notes of section 1, tags unchanged; `bus.release_latency_cpu_cycles` (inference, C52) gains "load-bearing from Phase 6: each +1 adds one cycle per window, 13 per frame, about 0.09 percent of the frame" and its resolving evidence stays the D10 bus trace; `cpu.interrupt_accept_clears_intff` (C41) gains the re-entry consequence note; `memmap.region.stic_regs` and the stub device are replaced by the real STIC device by the register sibling; `psg.max_unit_ticks` / `psg.min_write_gap_ticks` / `psg.log_records_per_frame` revisit triggers are closed as in T-4.

Generator refusals (new fail-closed `cmake/IvsSticTable.cmake` in the style of `IvsPsgTable.cmake`, with a `-DMUTATE` hook): `first_start + (count - 1) x pitch + hold <= scanlines x cycles_per_scanline`; `pitch >= hold`; `fetch_offset <= ack_deadline <= hold`; `rows <= count`; `reg_window <= gmem_window <= first_start`; `irq_low <= frame cycles`; `bus.release_latency x ticks_per_cpu_cycle <= psg.max_unit_ticks`; `video_slack_frames` equals the value implied by U < F; every cycle value converts without overflow; a tag may not be `tier5-emulator-hint`; every `unknown-default-chosen` field has a resolving evidence and a revisit trigger.

Proposed discrepancy allocation (the Phase 6 planner owns the final numbering; next free after C80 is C81): C81 frame phase at power-on; C82 VBLANK line source, hold and release; C83 bus-window table (count 13 or 14, positions, per-window holds, which rows); C84 late-grant behaviour and the row-latch instant; C85 access-stamp instant and whether windows apply with the display off; C86 aggregate cross-check (usable cycles 13,518/13,572) as a plausibility record, not a field. C1-C4 stay open with their `default_policy` unchanged; C5, C6, C7, C38, C41, C52 get one note each. A primary page that printed any of these would close them; none was found.

---------------------------------------------------------------------------------------------------

## 4. Arithmetic and proofs

### 4.1 Frame constants [executed `derive.py`]

- Line `= 4 x 57 = 228` ticks; frame `= 228 x 262 = 59,736` ticks `= 14,934` cycles; `3,579,545 / 59,736 = 59.9227` Hz; line rate 15,699.8 Hz; CPU cycle 1.1175 microseconds. A frame is 262 lines (not the 262.5 of studio NTSC, which would give 59.940 Hz): the profile's constant is the community one; it fits inside every tier-1 bound (167 columns of 228, 105 rows of 131 line pairs).
- 192 active lines x 57 = 10,944 cycles; 262 - 192 = 70 non-picture lines = 3,990 cycles; CPU time 3,791 cycles = 66.5 lines.

### 4.2 Window geometry

`first_start + 12 x pitch + hold = 3,791 + 10,944 + 114 = 14,849 <= 14,934` (85 cycles spare). A 14th window at the same pitch: `3,791 + 13 x 912 = 15,647 > 14,934`: it does not fit, so "13 or 14" with a constant pitch needs a shifted start (picture offset), not a longer train.

### 4.3 Stall closed form and the usable-cycle model

With units on the 4-tick grid, sample lag 1 cycle, grant at `b_g in [w + 4, w + 4H]`, hold units 4 ticks, release `L = 2` cycles: `b_r = w + 4H + 4`, `R = b_r + 4L = w + 4(H + 1 + L) = w + 468`. Model results for H = 114 (`model.py`, three frames, instruction length c in cycles, all interruptible): INSTR-unit cycles per display frame 13,428 (c=4), 13,452 (c=6), 13,480 (c=8), 13,491 (c=9), 13,524 (c=12); BUS_GRANT-unit cycles per frame 1,506, 1,482, 1,454, 1,443, 1,410; display off 14,934 and 0. Units per frame at c=6: 3,712 (display) versus 2,489 (off).

### 4.4 Cross-check against the community aggregates (C7, plausibility only)

`14,934 - 13,518 = 1,416` and `14,934 - 13,572 = 1,362` cycles stolen. Model totals: 12 windows x 114 = 1,368 -> 13,566 (+0.36 percent / -0.04 percent against 13,518 / 13,572); 13 x 114 = 1,482 -> 13,452 (-0.49 / -0.88 percent); 14 x 114 = 1,596 -> 13,338 (-1.33 / -1.72 percent); 13 x 110 = 1,430 -> 13,504 (-0.10 / -0.50 percent); the alternate first/last-window pair (57 + 12 x 110 + 44 = 1,421) -> 13,513 (-0.04 / -0.44 percent). None decomposes the community aggregates uniquely, and the alternates in C4 disagree among themselves. Decision: keep the tier-1-compatible count 13 and the wiki hold 114, accept a model value 0.5 to 0.9 percent below the aggregates, and encode the aggregates as a TOLERANCE test (`|usable - 13,518| <= 1.0 percent` for a 12-cycle instruction stream, recorded as a tier-4 plausibility check under C86), never as an expected value. A single-window change (hold 110, or count 12) moves the result inside the community spread; that is why each of them is a profile field with its own tag and why no claim of cycle accuracy is made (D10).

### 4.5 Capacity (T-9) [executed `cap.py`]

An interval of length `L` contains at most `ceil(L/F)` frame ends (alignment worst case); a call with `D >= 1` covers at most `L = D + 467` ticks (a unit that starts before the target ends at most `U - 1 = 467` ticks after it). With `U <= F`, `ceil((D + 467)/F) <= floor(D/F) + 2`. Counter-example to `+1`: `now = E - 4`, `D = F - 1`, two completions.

### 4.6 What the sample lag does to the first and last boundary of a window

Sample for the step starting at `b` is tick `b - 4`. BUSRQ(s) is high for `s in [w, w + 4H)`. A boundary at exactly `w` samples `w - 4`: not seen (it can still start an ordinary instruction that ends after `w`; the grant waits for the end of that instruction, at most 12 cycles, longer after non-interruptible ones). The first granting boundary is the first open boundary `>= w + 4`.

---------------------------------------------------------------------------------------------------

## 5. Verification plan, faults, CI and DX (shift-left, no human UAT)

Oracles written BEFORE the code (Phase 5 D-23 style, an oracle plan with no C):
- `docs/stic/timing-notes.md` (new): the section-2 timeline, section-1 evidence table, section 4 arithmetic, written from the pages and the profile, not from code.
- An independent timeline model: a ~60-line throwaway script (the `model.py` of this note, promoted to `tools/` only if a second user appears) producing, for each hand-written guest listing, the expected unit sequence around a window and around the VBLANK edge. The hand-derived closed forms (`R = w + 468`, INTRM seen iff `b <= S + 4 x 3791`, the first granting boundary) are the primary oracle; the script is a cross-check written by the same author, so agreement is repeatability, not accuracy (AGENTS.md rule).
- Two or three original diagnostics (hand-assembled per D4, ISR starts with DIS, no EXEC dependence through the project boot stub): (1) frame-count and ISR-count in RAM over N frames; (2) a strobe-timing probe that writes the display strobe at a chosen cycle and reports whether the frame was blank; (3) a stall counter (a counted loop whose iterations per frame the oracle predicts within the instruction-mix spread of section 4.3).

Test families (ids illustrative; roughly 40 ids, at most 8 faults per plan so split across plans):
- `stic.time.*`: frame constants and the 59,736 check (the Phase 2-queued unit check of `docs/time-base.md`), window table from the generator, event order, same-tick order, frame index state.
- `stic.irq.*`: INTRM predicate at and around cycle 3,791; EIS timing; DIS-first ISR; C41 in {0,1} x ISR shapes; priority BUSRQ before INTRM; `$1004` through the machine bus; SDBD and non-interruptible deferral across the edge.
- `stic.bus.*`: closed form `R = w + 468` over every instruction length 1..12 and every grant phase; late grant (non-interruptible chain longer than the hold) with `late_grants` visible; halted CPU grants; display off gives 14,934; aggregate tolerance; hold units and `run(now + 1)`.
- `stic.access.*`: window edges (`stamp = edge - 1, edge, edge + 1` ticks), blocked write dropped, blocked read `$FFFF`, strobe handshake, `$0021` read side effect only on a real CPU read, peek never mutates.
- `stic.catchup.*`: hook completeness over all addresses and views, straddle, slicing at 1/7/468 ticks across several frames, seam twin `memcmp`, buffer presence does not change state.
- `stic.frame.*`: frame stop clamp and reason, overshoot, report v3 fields, input event interplay, capacity corner (`+2`), VIDEO chain slicing invariance, blank frames counted.
- `stic.bounds.*`: the T-4 probes through the full machine.
- Digest: v2 literals pinned, v3 bump, classified golden diff, coverage of every instance byte by a table row.

Named faults (each must fail a specific family): STIC_CATCHUP_AT_TARGET, STIC_BACKTAB_WRITE_NOT_HOOKED, STIC_ALIAS_NOT_HOOKED, STIC_EVENT_ORDER_START_BEFORE_END, STIC_LINES_AT_UNIT_START (no sample lag), STIC_WINDOW_CLOSED_INTERVAL, STIC_LATCH_NOT_CLEARED, STIC_STALL_AS_WAIT, FRAME_STOP_NO_CLAMP, FRAME_DERIVED_BY_MODULO_FROM_NOW (a state/derivation mismatch under a perturbed phase), VIDEO_CAP_PLUS_ONE, VIDEO_STATE_DEPENDS_ON_BUFFER, IRQ_LEVEL_NEVER_RELEASED.

CI [REC]: nothing new at first; the new suites go into the existing `gate` job. Measure after the first plan: the last five median CPU seconds were near the trigger (Phase 5 note: 175 against 200); per-cycle hold units add roughly 50 percent more units on display-on runs only, so keep display-on scenarios short (a few frames) and keep the heavy slicing sweeps in the local fast path with a bounded frame count. A bulk-hold fast path is the first lever if the measured cost matters (section 7).

DX: print the timeline table from the generated header (`ivs-run --info` or a docs generator) so a human can read the schedule in cycles, lines and ticks without the source; the report's `next_frame_end_ticks` is the other half.

Throwaway scripts used (in `<scratch>/t06/`, not committed): `derive.py` (constants and timeline table), `model.py` (unit-level stall model with the real grant/hold/release semantics and the usable-cycle table), `cap.py` (frame-completion capacity check).

---------------------------------------------------------------------------------------------------

## 6. Role lenses (what each added)

- Emulator/CPU/video architect: reuse the stepper's own grant unit and the PSG catch-up pattern instead of new mechanisms; keep BUSRQ and INTRM as pure functions of schedule so they need no state or events; fetch latches, not a framebuffer, as the state that carries a frame across calls.
- Hardware/EE: the GI text puts glyph memory on a private bus and states CPU control exists only in CPU time; that is what justifies frame-granular render. SR1/SR2 to INTRM/BUSRQ wiring is inference (no schematic): the profile says so. Aggregates are cross-checks, never constants.
- QA: oracles are closed forms derivable by hand (`R = w + 468`, the INTRM predicate); the independent model shares an author with the core, so its agreement is repeatability only; hostile cases (non-compliant late grant, C41 re-entry, strobe one cycle late, edge stamps) each have a vector and a fault.
- Security/rights: no scan content copied; only facts and the page citations; community numbers come from the project's own tier-4 records; emulator source not opened; no home or scratch path in this file. Hostile guest: every index (event cursor, latch row) is validated; video bytes are bounded by a proved capacity plus an `INTERNAL` guard; stamps are `uint64_t` below 2^63 so no wrap; a guest cannot make the core write past a caller buffer.
- DevOps/CI: no new job; measure cost after plan 1; hold-unit overhead called out with a lever; goldens re-blessed once for the composite bump with a classified diff and a "no Phase 5 timing moved" check.
- API/integrator: one flag, one stop reason, three appended report fields, one `ivs_info` field; the host never derives a frame tick; the clamp makes `IVS_RUN_TARGET_MAX` plus the flag a valid frame-pacing loop; a video buffer of two frames is the minimum without the flag (documented).
- Product: Phase 7's scenario needs "the input changes raw video": satisfied by a frame stream with a stable per-frame chain; nothing here needs a TV-accurate picture position.
- DX: timeline printed from data; window parameters are data; the frame stop semantics are `run(min(target, E_next))`, one sentence.

---------------------------------------------------------------------------------------------------

## 7. Adversarial pass against this recommendation

1. "13 windows and 114 cycles disagree with the community aggregates by 0.5 to 0.9 percent." True and recorded (4.4). Nothing in this phase makes a cycle-accuracy claim; the claim ledger entries stay `provisional` with the weakest-field rule. A tester who treats 13,518 as an expected value would be wrong about the evidence, which is why it is a tolerance test under C86. If a D10 trace gave the real train, three fields change.
2. "Frame-granular render ignores mid-frame effects." Only if the access windows are wrong. The windows are estimates (C1, C2, tier 4). If a register or GRAM write can land during graphic time, T-7's forward path adds the register file (and glyph bytes) to the per-row latch; state grows, API does not change. Static fixtures for VID-04 are exact either way. Trigger: any source or D10 probe showing STIC register or GRAM effects mid-picture.
3. "Level-held INTRM plus C41 = 0 makes a normal ISR loop forever." This is the sharpest edge in the proposal. It is a faithful consequence of two separately sourced defaults, the original diagnostics avoid it by construction, and the owner-local EXEC run (D11) is the first discriminator. If the real EXEC ISR does not start with DIS, one of the defaults is wrong: flip C41, or add a STIC acknowledge that drops the request on INTAK (one state bit, one field). Not implemented now because it would be a second guess layered on the first.
4. "Windows apply with the display off may break boot code." A guest that writes STIC registers or GRAM outside CPU time before its first strobe would be blocked. The stub behaviour today is the same blocked policy, no existing fixture touches those regions, and the reversal is one field (`access_gating_when_off = "open"`). Hardware truth is unknown (C85, D10).
5. "Per-cycle hold units are slow." About +50 percent units on display-on runs. Remedy when measured, not before: a bulk-hold path that, while BUSAK is set and the schedule says BUSRQ stays high, advances n hold units at once, clamped to the run target on the 4-tick grid so the overshoot rule is preserved, proven byte-identical to unit-by-unit stepping by a sweep over targets, slicings and PSG sample alignment. It must not change `units_executed` semantics unless that is a deliberate, documented API change (it would: either keep counting hold units, or declare the count as units "as if stepped"). Decide then.
6. "Why not model the stall in the CPU stepper?" The stepper already has it (grant, hold, release, interruptibility, halted grant) with vectors and named faults; adding a second implementation would split the proof. The cost is that the stall length is driven from outside, which is exactly the D-05 contract (lines are inputs).
7. "The frame is now state; isn't that redundant with `now / F`?" Redundant today, and deliberately so: it keeps frame-ness out of arithmetic, keeps the digest honest, and is what snapshot (Phase 8) and a variable-frame profile need. The cost is 16 bytes and a validate function.
8. "`+2` frames wastes a buffer." Yes for tiny runs; simple and provable; the tight bound is a drop-in later and needs U in the rule. A host not requesting video pays nothing.
9. "The hook set could miss an access path." The set is derived from the memory system's decode, not constants; completeness is tested over every address and view; two named faults cover omission. A guest-visible alias missed by the decode would also be missed by the Phase 3 memmap tests, so the failure would not hide in this phase only.
10. "Row latch at cycle 57 of the window is arbitrary." Yes (C84). It is correct for every compliant guest (the CPU is frozen from its grant to the window end, so any instant after the latest compliant grant gives the same bytes) and exact-by-stamp for a non-compliant one. It matters only for guests that write BACKTAB within about 57 cycles after a window start without being granted, which is a guest that violates the bus protocol.
11. "Frame phase 0 pins the first VBLANK at reset." It is a default with a field (C81). A guest cannot tell it from any other phase except through the time of its first interrupt; the first strobed frame is what matters to Phase 7.
12. "Is the 'one-cycle sample lag' consistent with the grant timing?" It is the existing POL-LINE-01 rule (final TS1 of the previous unit); the closed forms include it, a named fault (STIC_LINES_AT_UNIT_START) checks it, and the oracle predicates are written with it. It changes results by one cycle per edge only.

Reversal triggers (what would reopen each decision):
- Window values, count, positions, hold: a tier 1 to 3 source or a D10 bus trace (fields only, no code).
- Render granularity: evidence of mid-picture register or glyph effects (add per-row latches).
- Frame length not constant: a profile or evidence with variable frames (generator and schedule only).
- C41 and the level model: the owner-local EXEC run or a D10 repeated-interrupt probe.
- Windows with the display off: D10 probe of register access with the display disabled.
- Performance: measured gate cost above the CI trigger, or PRF-01 workloads, triggers the bulk-hold path.
- Capacity: a host that cannot afford two frame buffers without the flag triggers the tight bound.
- C52 latency: any change shifts every stolen-cycle figure by 13 cycles per frame per cycle of latency.

Open items [UNK]: frame phase at power-on (C81); the release rule of the VBLANK level and whether INTAK affects it (C82); positions, count and per-window holds of the train and what the 14th pulse is (C83); the exact row-latch instant and late-grant behaviour (C84); whether register and GRAM windows apply with the display off and the instant used for the edge test (C85); the real value read from a blocked STIC or GRAM location (C5); where the picture sits relative to a TV sync (not needed for the raw frame); whether the display latch is cleared at SR1 or at the end of graphic time (C6); the effect of the vertical delay register on the window train (Phase 8).

Dependencies on sibling decisions: D8 geometry fixes `video_bytes_per_frame` (this note is parametric); the register/renderer sibling owns the STIC register layout, BACKTAB span fields and the color-stack state (this note reserves a slot for the mode and pointer in `ivs_stic_state`); the runner sibling owns IVSS grammar for the frame flag and assertions.
