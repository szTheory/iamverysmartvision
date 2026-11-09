# Architecture Research

**Domain:** Deterministic, headless Mattel Intellivision (CP1610 + STIC + AY-3-8914) emulator core, greenfield
**Researched:** 2026-11-09 (all web sources accessed 2026-11-09)
**Confidence:** MEDIUM overall. HIGH for component decomposition and API-shape reasoning; MEDIUM for CPU timing (a primary manual exists and was read); LOW-to-MEDIUM for STIC/bus-steal cycle constants (they come from community notes and emulator source, not from a datasheet or a hardware measurement).

## Evidence labels used in this file

| Label | Meaning |
|---|---|
| **[SRC-PRIMARY]** | Period manufacturer document read in this session (GI CP-1600 User's Manual, May 1975, bitsavers). |
| **[SRC-COMMUNITY]** | Community wiki / reverse-engineering notes (intellivision.us wiki, Zbiciak TutorVision/INTV88 notes). Useful, attributable, not a datasheet. |
| **[SRC-EMULATOR]** | Observed in jzIntv or MAME source. Not a hardware oracle. Read for facts only; nothing here recommends copying code (jzIntv is GPL-2.0-or-later; MAME licensing is per-file). |
| **[SRC-HIST]** | Historical in-house Mattel/Blue Sky Rangers material (2609 overview). |
| **[INFERENCE]** | My own derivation from the sources above. Checkable, not observed. |
| **[REC]** | Recommendation for the owner. Not an owner decision. |
| **[UNKNOWN]** | Not established by anything read. Needs a source, a test ROM, or hardware. |

Nothing below is an executed result. No code or test was run against any emulator.

---

## 1. What the hardware forces on the architecture

These are the facts that shape the software structure. Each is labeled; contradictions between sources are collected in 1.4 because they decide where the architecture must keep "calibration data" separate from "logic".

### 1.1 Clocks and CPU timing

| Fact | Label | Source | Confidence |
|---|---|---|---|
| NTSC system clock 3.579545 MHz; CP1610 "machine cycle rate" 894.886 kHz (= clock / 4); instructions take 4-12 cycles. | [SRC-HIST] + [SRC-COMMUNITY] + [SRC-EMULATOR] (MAME: CPU = XTAL(3'579'545)/4) | history.blueskyrangers.com/hardware/2609.html; wiki CP1610; MAME `intv.cpp` | HIGH (three-way agreement; MAME/wiki are not independent of each other) |
| PAL/SECAM: 4 MHz crystal, CPU ~1 MHz. | [SRC-COMMUNITY] | wiki CP1610, wiki PSG | MEDIUM; PAL STIC timing is largely guessed (see 1.4) |
| 3.579545 MHz is a rounded figure; true NTSC colorburst is 315/88 MHz = 3.5795454545... MHz. Whether the machine profile uses the rounded or the exact rational changes the frame rate in the 7th digit. | [INFERENCE] | arithmetic | HIGH that the choice exists; [UNKNOWN] which one the first profile should pin |
| CP1610 original datasheet: a two-phase 5 MHz clock gives a 400 ns "microcycle"; the instruction table lists "CYCLES" per instruction, with +W wait states per memory access when memory is slow. Intellivision wiring reportedly leaves BDRDY unconnected, i.e. no wait states. | [SRC-PRIMARY] (manual) + [SRC-COMMUNITY] (wiki CP1610: "BDRDY unconnected, so no wait states") | CP-1600_Microprocessor_Users_Manual_May75.pdf, sections 1.0 and 3.0 | HIGH for the manual; MEDIUM for the Intellivision wiring claim |
| Terminology trap: the manual's "microcycle"/"cycle time" is a bus-state unit of the original CP1600 clocking; the Intellivision documents use "cycle" for the 894.886 kHz unit and "microcycle" for sub-states. | [INFERENCE] | Blue Sky says "machine cycle rate of 894.886 KHz ... between 4 and 12 microcycles" while the wiki table calls the same numbers "cycles" | The project should name one unit ("CPU cycle", 4 master-clock ticks) and never say "microcycle" in an API |
| Instruction cycle counts and interruptibility (e.g. MVI 10, MVO 11, MOVR 6 (+1 if dest is R6/R7), Bcc 7/9, shifts +2 for 2-bit; shifts/MVO/SDBD/EIS/DIS/TCI/CLRC/SETC/HLT non-interruptible). | [SRC-PRIMARY] (manual instruction tables; I spot-checked MOVR, BC, MVO, MVI, MVI@, EIS, DIS, TCI, CLRC, SETC, shifts) agrees with [SRC-COMMUNITY] wiki table | manual lines "MOVR 6*", "BC 7/9", "MVO 11", "MVI 10"; wiki CP1610 | HIGH. This is the one place a primary, independent oracle exists. |
| The manual defines bus-control phases BAR, ADAR, DTB, DW, DWS, NACT, INTAK, IAB (via BDIR/BC1/BC2) and says each instruction's external-reference sequence issues them in a fixed order (fetch: BAR then DTB; MVO: DW, DWS, NACT; direct mode adds ADAR). | [SRC-PRIMARY] | manual section 2.2 | HIGH |
| BUSRQ is "honored only after the completion of an interruptable instruction"; BUSRQ is serviced before interrupt requests; BUSRQ and INTR/INTRM are digitally sampled at TS1; INTRM honored only if INTFF set; interrupt sequence = INTAK (push PC) then IAB (device supplies new PC). | [SRC-PRIMARY] | manual sections on BUSRQ*, "C.5 Bus Request Sequence", "C.6 Interrupt Sequence" | HIGH |
| Reset vector $1000; INTRM vector $1004; INTR tied inactive. How the Intellivision actually supplies $1004 during IAB is not explained in anything I read. | [SRC-COMMUNITY] for values; mechanism [UNKNOWN] | wiki CP1610 | MEDIUM for values |

### 1.2 Bus, STIC, System RAM and the VBLANK windows

The Intellivision is not a "CPU plus a video chip on one bus". The STIC has a private 14-bit graphics bus; the 16-bit System RAM (GI RA-3-9600) bridges the two buses. This is the single most architecture-relevant fact.

| Fact | Label | Source | Confidence |
|---|---|---|---|
| RA-3-9600 holds BACKTAB ($0200-$02EF, 240 words) and the rest of the 16-bit system RAM ($0200-$035F), contains a 20-word video FIFO, and arbitrates graphics-bus access. GROM/GRAM are reachable by CPU or STIC but never both at once. | [SRC-HIST] (Blue Sky) + [SRC-COMMUNITY] (wiki System RAM) | blueskyrangers 2609; wiki System_RAM | MEDIUM-HIGH |
| During active display the STIC asserts BUSRQ one scanline before it needs each BACKTAB row (12 rows per frame, one 20-card row each). The CPU acknowledges (BUSAK) after its next interruptible instruction regardless of the I flag; the STIC pulses SR3 once per card and the FIFO replays the same 20 cards for the following scanlines. The STIC steals "around 1 to 2 scanlines out of 16". | [SRC-COMMUNITY] (wiki BUSRQ) | wiki.intellivision.us BUSRQ | MEDIUM |
| If the CPU fails to BUSAK in time (a long run of non-interruptible instructions), the previous row is replayed and the display "pushes down"; with BUSAK blocked, System RAM can stay in Bus Copy and cause bus fights. | [SRC-COMMUNITY] (wiki Non-interruptible Instructions, System RAM, BUSRQ); jzIntv models the replay [SRC-EMULATOR] | wiki; jzIntv `stic.c` ("A late BUSRQ replays the previous 20 cards") | MEDIUM |
| VBLANK interrupt: STIC pulls INTRM low once per frame (NTSC 14934 CPU cycles). CPU must be interruptible with I set and reach an interruptible instruction while the line is low, "otherwise the CPU will drop the interrupt". | [SRC-COMMUNITY] | wiki VBlank_Interrupt | MEDIUM |
| Bus Copy mode: System RAM enters it on INTAK in response to the VBLANK interrupt and leaves it when BUSAK arrives, just before active display. In Bus Copy the CPU can reach STIC registers, GRAM, GROM. The STIC decodes only 14 bits and incompletely, producing write-only aliases at $4000-$403F, $8000-$803F, $C000-$C03F (STIC) and $7800-$7FFF, $B800-$BFFF, $F800-$FFFF (GRAM); reads of those aliases return nothing useful; any access to an alias of $21 toggles mode select. | [SRC-COMMUNITY] | wiki Bus_Copy, Memory_Map | MEDIUM |
| VBLANK Period 1: STIC registers accessible ~2900 CPU cycles after the interrupt (when display is active). VBLANK Period 2: GRAM/GROM accessible ~3796 cycles after the interrupt. If the program never writes $20 (display strobe), registers/GRAM/GROM stay accessible until it does. | [SRC-COMMUNITY] (wiki) agrees with the jzIntv constants NTSC_STIC_ACCESSIBLE 2900 / NTSC_GMEM_ACCESSIBLE 3796 [SRC-EMULATOR] | wiki VBlank_Period_1/2; jzIntv `stic_timings.h` | MEDIUM (the two sources are probably not independent) |
| Active display geometry: 20x12 cards of 8x8 pixels (160x96, rightmost column not displayed), each pixel 2 TV lines high; 16 colors; 8 MOBs; hardware collision registers; 1-pixel scroll via $30/$31; FG/BG, Color Stack, Colored Squares modes. | [SRC-HIST] + [SRC-COMMUNITY] | blueskyrangers 2609; wiki STIC | MEDIUM-HIGH |
| Output frame in jzIntv is rendered once per frame near the end of the frame, not per scanline; the jzIntv merge loops run over `208 - v_dly*2` rows. | [SRC-EMULATOR] | jzIntv `stic.c` excerpt | observation only; says what one working emulator does, not what the hardware requires |
| Real-hardware measurement on the *TutorVision/INTV88* (a different board using the STIC1A): during active display STIC registers return "random" values and GROM/GRAM return $00FF; the STIC1A "definitely has different cycle stealing and bus isolation timing" from the original STIC; "Timing of /BUSRQ, /INTRM" and "Duration of STIC register visibility" are listed by the author as still to be measured. | [SRC-COMMUNITY] (Zbiciak, TutorVision / INTV88 Reverse Engineering Notes, 2017-2018) | docs.google.com/document/d/13ApQEm1Xw_DcCAaXcQvpFot_BvrzSzeUo95EXCf41uc/pub | MEDIUM-HIGH for what was measured; it is **not** a 2609 measurement |

### 1.3 Memory map, widths, system images

Official map [SRC-COMMUNITY] (wiki Memory_Map; matches MAME `intv_mem` [SRC-EMULATOR], which also lists the Intellivision II / ECS / IntelliVoice variants):

| Range | Device | Width / note |
|---|---|---|
| $0000-$003F | STIC registers | 14-bit visible; access only in VBLANK Period 1 |
| $0100-$01EF | Scratchpad RAM | 8-bit (256x8 SRAM total, ~147 bytes left for games per Blue Sky [SRC-HIST]); upper byte reads 0 on the TutorVision [SRC-COMMUNITY], not confirmed for 2609 [UNKNOWN] |
| $01F0-$01FF | PSG registers (R0-R15) | R14/R15 = I/O ports A/B = hand controllers at $01FE/$01FF |
| $0200-$035F | System RAM | 16-bit; BACKTAB at $0200-$02EF |
| $1000-$1FFF | EXEC ROM | 10-bit "decles", 4 K words; reset vector $1000 |
| $3000-$37FF | GROM | 2 KB, 8-bit, 256 cards; CPU access only in VBLANK Period 2 |
| $3800-$39FF | GRAM | 512 B, 8-bit, 64 cards; mirrored over $3A00-$3FFF; CPU access only in VBLANK Period 2 |
| $4000-$FFFF | Cartridge port | ranges individually usable subject to EXEC/ECS/IntelliVoice conflicts; typical boot ROM at $4800/$5000/$7000 per wiki; reads from undriven space return $FFFF (pull-ups) |

Other relevant facts:
- ROM words are 10 bits ("decles"); two-decle instructions are "bidecles" [SRC-HIST]. Whether upper bits of a 10-bit ROM read back as 0, 1 or bus-residual on a real 2609 is [UNKNOWN]. Most emulator and file conventions store u16 words with zeroed upper bits [INFERENCE from jzIntv/MAME word-doubling `<<1` ROM regions].
- EXEC is 4 K decles: two 2 K chips plus 344 bytes inside the GROM chip on the original; a single 4 K chip on Intellivision II [SRC-HIST]. The EXEC also copies GROM-resident routines into BACKTAB RAM and executes them there during reset while blanked [SRC-HIST]. Consequence [INFERENCE]: the CPU must be able to execute from System RAM, and the System RAM "does not correctly implement ADAR, so Direct Mode instructions misbehave when run from System RAM" [SRC-COMMUNITY, wiki System_RAM]. That quirk is only expressible if bus phases are visible to the memory system (see 3.3).
- STIC1A (Intellivision II / Sears) differs in BUSRQ pattern, bus isolation timing and INTRM behavior; the notes suspect 12 BUSRQs and no initial short BUSRQ [SRC-COMMUNITY, hypothesis]. First profile should not blur this.
- Rights: EXEC and GROM are Mattel/GI-owned. A core that must boot real games needs them as **inputs**. A project-authored replacement EXEC is architecturally possible because the reset vector lives at $1000 in the EXEC image slot [INFERENCE].

### 1.4 Contradictions and gaps between sources (the "calibration data" list)

These must not be hard-coded as logic. They belong in a profile table with per-constant provenance.

| Item | Source A | Source B | Status |
|---|---|---|---|
| Cycles per NTSC frame | 14934 (wiki; jzIntv `NTSC_FRAMCLKS`) | wiki also mentions "some sources say 14394" | 14394 is very likely a transposition [INFERENCE]. 14934 = 262 x 57 exactly, matching jzIntv `NTSC_SCANLINE` 57 [INFERENCE]. PAL 19968 = 312 x 64 matches jzIntv `PAL_SCANLINE` 64 [INFERENCE]. Frame rate then = 894886.x / 14934 ~ 59.92 Hz, matching MAME's 59.92 Hz [SRC-EMULATOR]. |
| INTRM (SR1) low duration | 3791 cycles (wiki VBlank_Interrupt) | 2907 "from Kevin Horton" (jzIntv `NTSC_INTRQ_HOLD`; an alternate 2920 is commented out) | [UNKNOWN] which is right; affects whether the CPU can "drop" an interrupt |
| BUSRQ hold (NTSC) | 114 cycles (wiki) | 110 normal / 57 first / 44 "14th is half-length" (jzIntv) | [UNKNOWN]; Zbiciak notes say "/BUSRQ appears to be ~1 CPU cycle shorter based on experiments in jzIntv" and lists BUSRQ timing as unmeasured |
| Latest cycle by which CPU must BUSAK | ~57 (wiki) | 68 (jzIntv `NTSC_MUST_BUSAK_TIME`) | [UNKNOWN] |
| Value read from GROM/GRAM/STIC while inaccessible | $00FF for GROM/GRAM, random for STIC regs (TutorVision measurement) | `0x3FFF & addr` for blocked GMEM and `0x000E & addr` for blocked STIC regs (jzIntv) | Disagree. Different boards, and the TutorVision result is one measured board [UNKNOWN for 2609]. Illustrates that "blocked read" values are a profile parameter, not a constant. |
| Which strobe enables the display | $20 "Active Display Strobe", $21 "Mode Select Strobe" (wiki STIC) | One search summary attributed "display enable" to $21 in a MAME comment; I did not open that comment | Treat wiki as the working reading; verify with an original diagnostic on hardware or a cleared source [UNKNOWN] |
| 2609 vs STIC1A behavior | Many constants in jzIntv are per-chip (`143`/`149`/`160` first-fetch offsets) | | A "STIC type" must be a profile field |
| PAL STIC timings | jzIntv comments them "Guess 100 scanlines", "Guess 116.5 scanlines", "total guess" | | PAL is a later profile; the architecture must stay parametric but should not claim PAL early |
| Power-on state (RAM contents, registers, GRAM, CP1610 registers other than R7) | | | [UNKNOWN]; must be a profile-specified deterministic fill, never uninitialized memory |
| Disc/keypad electrical codes | A pin-pull-low table (Deathskull page via archive.org mirror) | Wiki: "keypad 1 reads $7E; keys 1 and 9 together read $5A" | The Deathskull table as extracted has an apparent duplicate (D1 and D5 both "pin 3"), so treat as LOW until checked against a schematic [UNKNOWN] |

---

## 2. Standard Architecture

### 2.1 System overview

```
 OWNER-CONTROLLED, OUTSIDE THE CORE (no machine semantics live here)
┌──────────────────────────────────────────────────────────────────────────────┐
│  Host / frontend (later)            Headless runner              Installed   │
│  window, audio device,              scripts, input timeline,     consumer    │
│  physical pads, file I/O,           expectations, captures,      (clean-room │
│  pacing, save files                 hashes, trace sinks          package     │
│                                                                  smoke)      │
└──────────┬──────────────────────────────┬──────────────────────────┬─────────┘
           │ content, system images, cfg  │ same API                 │ same API
           │ input sample, budget, spans  │                          │  as shipped
           ▼                              ▼                          ▼
┌──────────────────────────────────────────────────────────────────────────────┐
│  CORE PUBLIC API (narrow, instance-based, no I/O, no clocks, no threads)     │
│  create/destroy | load(images)->refusal | set_input | run(bounded) |         │
│  reset/unload | snapshot/restore | state_hash | observer taps                │
├──────────────────────────────────────────────────────────────────────────────┤
│  MACHINE (one deterministic timeline, integer time = CPU cycles)             │
│                                                                              │
│  ┌──────────┐  bus phases   ┌───────────────┐   ┌─────────────────────────┐  │
│  │ CP1610   │◄─────────────►│ Bus + memory  │◄─►│ Devices                 │  │
│  │ (pure    │ BUSRQ/BUSAK   │ map / decode  │   │  Scratchpad (8-bit)     │  │
│  │ stepper) │ INTRM/INTAK   │ widths, alias,│   │  System RAM RA-3-9600   │  │
│  └────┬─────┘               │ undriven,     │   │   (BACKTAB, FIFO,       │  │
│       │ cycles consumed     │ bus-copy gate │   │    bus-copy gate)       │  │
│       ▼                     └───────┬───────┘   │  EXEC / GROM / GRAM     │  │
│  ┌───────────────────────────┐      │           │  Cartridge ROM/RAM      │  │
│  │ Scheduler / timeline      │      │           │  STIC (regs, windows,   │  │
│  │ master time + event queue │◄─────┴──────────►│   BUSRQ/INTRQ gen,      │  │
│  │ lazy device catch-up      │                  │   frame latch+render)   │  │
│  └───────────────────────────┘                  │  PSG AY-3-8914 (regs,   │  │
│                                                 │   synth, I/O ports)     │  │
│                                                 │  Controllers (31        │  │
│                                                 │   surfaces x2 -> bytes) │  │
│                                                 └─────────────────────────┘  │
├──────────────────────────────────────────────────────────────────────────────┤
│  OBSERVATION (write-only from the machine's point of view)                   │
│  raw index video frame | PSG write log + reference PCM | trace ring |        │
│  canonical state serializer -> domain hashes                                 │
└──────────────────────────────────────────────────────────────────────────────┘
```

Data flows strictly downward on the control path (host/runner -> API -> machine) and upward on the observation path (machine -> observation buffers -> runner/host). Observers never feed back into the machine. [REC]

### 2.2 Component responsibilities and boundaries

| Component | Owns | Talks to | Must NOT |
|---|---|---|---|
| **CP1610 stepper** | R0-R7, flags (S,Z,O,C,I,D), INTFF, SDBD state, instruction decode (1024 opcodes), per-instruction cycle cost and *cycle-offset-stamped bus-phase schedule*, interruptibility | Bus (via narrow read/write/phase interface); scheduler (reports cycles) | know about STIC, PSG, frames, or host; read global state |
| **Bus + memory map** | Address decode, per-device width/undriven-bit policy, aliases, "blocked" gating, System-RAM bus-copy gate, EXEC/cart image storage as u16 words + width mask | CPU, devices | decide timing; read files |
| **Scheduler / timeline** | Master time (integer), event queue (INTRQ, BUSRQ n, window open/close, frame complete), catch-up of lazily-advanced devices | all stateful devices | use wall clock; use floating point for machine time |
| **STIC + System RAM video subsystem** | STIC registers, display-enable handshake, mode select, windows, BUSRQ/INTRQ generation, FIFO/BACKTAB row latching, MOB/BACKTAB/border rendering, collision registers | Bus (as a device), scheduler | read host palettes; scale; present |
| **PSG** | 16 registers, tone/noise/envelope counters, I/O ports R14/R15, register-write log, reference sample generation | Bus, controllers (port reads), scheduler | know host sample rate |
| **Controllers** | Per-controller *physical control state* (12 keypad + 16-way disc + 3 side buttons) and the pure function control-state -> 8 active-low electrical lines | PSG ports; API input sample | know host buttons/gamepads/overlays |
| **Loader / system-image validator** | Parsing and refusal of content and system images into typed regions; identity (length, digest) recording | Bus/memory map, API | touch the filesystem; silently accept short/oversized/odd-width data |
| **Machine shell** | Wires the above, owns profile (clocks, constants table, device set), reset/unload, run loop | everything | hold host handles |
| **Observation layer** | Video frame publisher, audio/event publishers, trace ring, canonical state serializer + hashing | machine (read-only taps) | mutate machine state, depend on caller buffer capacity for machine behavior |
| **Headless runner** (outside core) | Input timelines in machine time, expectations, captures, hash recording, failure classification (harness vs guest vs timeout vs missing fixture) | public API only (plus debug taps) | link private core headers |
| **Installed consumer** (outside core) | Proves the *packaged* artifact works from a relocated install | public API of the installed package | share source-tree include paths or private fixtures |
| **Host / frontend** (outside) | Pacing, windows, audio device, physical pads, overlays, remapping, persistence policy | public API | alter machine timing |
| **LibFuturo adapter** (optional, later, outside core) | Translate API to `lf_*`; bundle encoding; PCM policy | core public API | redefine core semantics |

### 2.3 Recommended project structure (language-neutral; [REC])

Language, ABI and build system are open owner questions (PROJECT.md Q7). The architectural properties below should hold in whatever is chosen: no mutable globals, explicit allocator, integer-only machine state, C-compatible narrow boundary, single-threaded per instance.

```
core/
  cpu/            # CP1610 decode, step, cycle/bus-phase tables (data-driven)
  bus/            # memory map, address decode, width/undriven policy, aliases
  sched/          # timeline, event queue
  video/          # STIC regs/windows/BUSRQ/INTRQ, System RAM FIFO, renderer
  audio/          # AY-3-8914 model, write log, reference PCM
  input/          # controller electrical model
  media/          # image/cartridge descriptors, validators, refusal codes
  machine/        # composition, profile tables, reset/unload, run loop
  observe/        # frame/audio/trace publishers, canonical serializer, hashing
  api/            # the narrow public surface (only thing consumers see)
profiles/         # data: clocks, constants + provenance tags (one file per named profile)
tools/            # project-authored mini-assembler, fixture builders (not shipped in core)
runner/           # headless runner (links only api/)
consumer/         # installed-package consumer (links only the installed artifact)
fixtures/         # original diagnostics + manifests (source, recipe, digest, license, use)
docs/evidence/    # claim ledger: source observation vs test vs inference per constant
```

Rationale: `profiles/` as data keeps the contradictions in 1.4 from becoming code forks; `tools/` is separate so that fixture authoring cannot leak into the runtime; `runner/` and `consumer/` link only the public boundary so they remain independent evidence routes (EMULATOR-DNA: "keep runner, module, frontend and physical-device evidence separate").

---

## 3. Timing model

### 3.1 Options compared (question 2)

| Option | What it is | Fits Intellivision? | Cost / risk |
|---|---|---|---|
| **A. Instruction-stepped, whole-instruction cycle counting** (devices ticked after each instruction with the cycle cost) | Simplest. Device state advances in coarse steps. | Insufficient on its own. BUSRQ is honored only at instruction boundaries (so stepping suffices for the *CPU side*), but access-window cutoffs (STIC regs ~2900, GMEM ~3796), INTAK-triggered bus-copy entry, and System-RAM ADAR quirk depend on *when inside an instruction* a bus phase happens. | Low cost; likely to be wrong at window edges by up to ~12 cycles, which is exactly the region games probe. |
| **B. Instruction-stepped with cycle-stamped bus phases + lazy device catch-up** | CPU executes an instruction atomically but emits each memory access with its cycle offset (BAR/DTB/DW/DWS/ADAR/INTAK positions from the manual). A device that is accessed is first caught up to that timestamp, then handles the access. Events (INTRQ, BUSRQ, window close, frame complete) sit in a small queue. | **Yes. [REC]** This is also the structure jzIntv's source shows (STIC "catches up" via `stic_simulate_until(access_time)` on each access; BUSRQ/INTRQ requests are queued; frame render is a scheduled event) [SRC-EMULATOR]. | Medium. Needs a per-instruction bus-phase table, derived from the manual [SRC-PRIMARY] and checked by vectors. |
| **C. Full microcycle/time-slot (TS1-TS4) lockstep**, all devices clocked per master tick or per CPU cycle | Everything advances in lockstep with sub-cycle events. | More than needed *today*. No source I read shows an Intellivision behavior that is observable at sub-CPU-cycle resolution. BUSRQ/INTRM sampling "at TS1" is already captured by "check at instruction boundary". The only place it might matter is STIC internal pixel timing for collisions/borders, which is not CPU-visible [INFERENCE]. | High cost, slower, and invites false precision (constants in 1.4 are not known to that resolution anyway). Keep as an explicit later option behind the same API if hardware measurement demands it. |
| **D. Pure event scheduler (every device is an event source)** | Generalized form of B. | B already is this for the few devices that need it. | Over-generalized for a 3-chip machine. |

**Recommendation [REC]: Option B.** One integer timeline in **CPU cycles** (1 cycle = 4 master-clock ticks on NTSC); the scheduler also carries a master-tick sub-count only if a profile needs it. Rationale: the CPU's externally visible timing is cycle-granular, the PSG steps every 4 CPU cycles (below), and every STIC/System-RAM interaction the CPU can observe is expressed in CPU cycles in all sources.

### 3.2 Why this precision, device by device

| Device | Needed precision | Why [INFERENCE unless stated] |
|---|---|---|
| CP1610 | Exact per-instruction cycle count (manual table, W=0) + intra-instruction offsets of bus phases + interruptibility | BUSRQ/INTRM are honored only after interruptible instructions [SRC-PRIMARY]; long non-interruptible runs change what the display shows |
| STIC control path (INTRQ, BUSRQ, windows) | CPU-cycle exact event times | Windows (~2900/~3796), 12 BUSRQs per frame each ~110-114 cycles steal ~9-10% of CPU time (12 x ~110 + short first/extra ~ 1400-1500 of 14934), which shifts all guest timing |
| STIC rendering | Per-frame render from latched state is probably sufficient **if** (a) registers/GRAM are only CPU-writable in VBLANK and (b) BACKTAB rows are latched at BUSAK time | jzIntv renders once per frame from shadow BACKTAB (`btab_sr` -> `btab` per BUSRQ) [SRC-EMULATOR]; the hardware reason is the VBLANK-only access [SRC-COMMUNITY]. Mid-frame BACKTAB edits therefore show up only through the per-row latch |
| PSG | Register-write timestamps at CPU-cycle resolution; synthesis in whole PSG steps | Wiki: PSG tone clock = 3.579545 MHz / 16 = 223,721.5625 Hz (NTSC), which equals CPU clock / 4 [SRC-COMMUNITY; arithmetic INFERENCE]; MAME gives the chip clock as 3.579545 MHz / 2 [SRC-EMULATOR], consistent with an internal /8 divider (inference). So one PSG step = 4 CPU cycles, an integer ratio |
| Controllers | Level read at the cycle of the PSG R14/R15 read, from a sample latched at call start | Determinism; matches "sample at explicit machine-time boundary" |

### 3.3 What "bus phase visibility" buys (and why not skip it)

- Three behaviors in the sources only exist at bus-phase level: System RAM entering Bus Copy on **INTAK**, leaving on **BUSAK** [SRC-COMMUNITY]; System RAM mishandling **ADAR** (direct-mode from BACKTAB) [SRC-COMMUNITY]; the Bus Copy direction reversing only for `$0000-$007F`/`$3000-$3FFF` **in the DTB phase** [SRC-COMMUNITY, Bus_Copy].
- Recommendation [REC]: the CPU stepper does not "suspend mid-instruction"; it returns an ordered list of `(cycle_offset, phase, address/data)` for the instruction, and the bus feeds the devices in that order with those offsets. This is cheap, testable against the manual's phase tables, and leaves room for Option C later.

### 3.4 Master-clock relationships

| Quantity | NTSC | PAL/SECAM | Label |
|---|---|---|---|
| Crystal | 3.579545 MHz (exact colorburst is 315/88 MHz; profile must pin one) | 4 MHz | [SRC-HIST]/[SRC-COMMUNITY]; pinning is [REC] |
| CPU cycle | crystal / 4 = ~894,886 Hz | ~1 MHz | [SRC-HIST]; MAME agrees |
| PSG chip clock | crystal / 2 (MAME); tone step every crystal/16 = CPU/4 | 4 MHz / 16 = 250,000 Hz tone step per wiki | [SRC-EMULATOR] + [SRC-COMMUNITY] |
| Frame | 14,934 CPU cycles = 262 lines x 57 cycles (~59.92 Hz) | 19,968 = 312 x 64 (~50.08 Hz) | wiki/jzIntv; line factorization is [INFERENCE]; PAL is largely guessed |
| STIC fetch/pixel clock vs CPU | [UNKNOWN] from what I read (jzIntv hides it behind scanline = 57 CPU cycles) | [UNKNOWN] | Not needed for first slice |
| Audio frames per second at 48 kHz | not an integer; use an exact rational accumulator (the sibling Muhtari adapter does this with 88/13125 per master tick; the *pattern* transfers, not the constants) | | [REC] |

### 3.5 Bounded execution and the stop rule [REC]

- Machine time is an absolute 64-bit CPU-cycle counter since reset (plus a frame counter). `run` takes an **absolute target** (or "to next frame boundary"), executes whole instructions, and may stop *after* the target by at most one instruction plus any bus stall in progress; the overshoot is reported. Next target is absolute, so overshoot never accumulates drift. This mirrors LibFuturo's `atomic_quantum_units`/`max_overshoot_units` idea without copying its layout.
- A BUSRQ stall is a *timeline interval* between instructions, accounted in cycles, never "half an instruction".
- "Frame boundary" must be defined by the profile as an event on the timeline (suggest: the STIC frame-complete/render event, after the last displayed line), not as "INTRQ". A call that publishes video stops at the first such boundary (at most one published frame per call).
- Determinism corollary: the sequence of instruction boundaries must not depend on call slicing. Testable property: run(A) then run(B) equals run(A+B) in final state hash and in all logs when targets align with the same boundaries, and when they do not, the divergence is limited to documented stop-rule overshoot. [REC]

---

## 4. Core API shape (question 3)

### 4.1 Properties the API must have [REC]

1. **Instance lifecycle:** `create(profile, allocator?) -> instance`; `destroy`; host serializes calls; instances share no mutable globals (provable by running two instances interleaved and comparing against two sequential runs).
2. **Ownership:** content and system images are *borrowed for the load call*; the core copies what it keeps (the LibFuturo pattern LC-LOAD/LC-OWN is a good model). Output buffers are caller-owned; core never retains pointers. No file paths anywhere in the API.
3. **Typed images, not one blob:** distinct, digest-identified slots: EXEC (4 K x 10-bit as u16 words), GROM (2 KB), cartridge (segments with address, width mask, ROM/RAM/banking attributes). Config/profile id selects device set (base 2609 first; ECS/IntelliVoice/Intellivision II later).
4. **Input:** a fixed-size *controller sample* (two controllers) applied from the start of the next `run`; `set_input` alone does no guest work; the machine records the sample in force so logs are reproducible.
5. **Execution:** `run(target_cycle | to_frame)`; result carries actual cycle, overshoot, frame count, stop reason (budget / frame / halted by `HLT` / refused), and which outputs are valid.
6. **Outputs:** caller-supplied video span (raw index bytes, stride) and audio span; short capacity is refused before any mutation (LibFuturo LC-LIMIT pattern; the Muhtari profile requires room for the maximum per-call count even if fewer are produced, so output size cannot influence machine behavior).
7. **Reset/unload:** `reset` returns to the profile's defined power-on state (RAM fill policy explicit, not host-random); `unload` returns to "no content"; both clear pending input and counters.
8. **Refusal semantics:** failed load leaves the prior session intact; failed run preflight changes nothing and writes nothing; every refusal has a typed, testable code (malformed image, wrong length, wrong width, unknown profile/config, unsupported device, buffer too small, limit exceeded).
9. **Save-state:** the canonical serializer that feeds hashing (see 5.3) doubles as the snapshot format, version- and identity-tagged; restore validates before mutating (LibFuturo LC-SNAPSHOT-04 is a good precedent). No derived render cache may carry semantics (jzIntv needs `stic_resync` after a state load because of dirty flags [SRC-EMULATOR]; avoid that class of bug).
10. **No stable ABI promise yet** (PROJECT.md Q7 open). Keep a `struct_size`-first convention if a C boundary is chosen so growth is detectable.

Illustrative shape only (not an ABI proposal):

```c
// Illustrative. Field names/types are NOT a recommendation to freeze a layout.
typedef struct { uint16_t keypad;     // 12 bits, multi-key allowed
                 uint8_t  disc;       // 0 = none, 1..16 = direction
                 uint8_t  side; }     // 3 bits
        intv_controller;
typedef struct { intv_controller c[2]; } intv_input;           // applied at start of next run
typedef struct { const void *exec; size_t exec_len;            // u16 words, width mask recorded
                 const void *grom; size_t grom_len;
                 const void *cart; size_t cart_len;            // segment table inside
                 const char *profile_id; } intv_load;
```

### 4.2 Fit against the LibFuturo candidate API (read from `../libfuturo/include/libfuturo/libfuturo.h` and `docs/contract.md`, 2026-11-09; not executed)

| LibFuturo element | Verdict for Intellivision | Detail |
|---|---|---|
| Instance lifecycle (`lf_create`/`lf_destroy`, host-serialized, optional allocator) | **Fits** | Same model as recommended. Needs the "no mutable globals" proof for `LF_CAP_CONCURRENT_USE` to be claimed. |
| `lf_load`: one content span + one config id, copied on success, refusal keeps prior session | **Partial** | Load/refusal semantics fit. One span cannot carry EXEC + GROM + cartridge as separate typed, identity-checked images. Options below. |
| `lf_input.buttons` 32-bit mask sampled at advance start | **Does not fit losslessly** (details in 4.3) | Timing model fits; width and structure do not. |
| `lf_advance` with `work_units`, atomic quantum, overshoot, stop reasons | **Fits** | Cycle-granular work would need a new `lf_time_unit` (the enum has SYNTHETIC_STEP, FRAME, NES_HALF_MASTER_TICK); frame units fit with the Muhtari-style "first interval is partial" precedent. The core's native cycle target should stay the primary contract. |
| Video `LF_VIDEO_INDEX8`, fixed `video_width`/`video_height`, stride | **Fits** | Raw 16-color indices in bytes; profile must pin a geometry including border policy ([UNKNOWN] exact visible/border geometry; see Gaps). |
| Audio S16 interleaved at `rate_num/den`, `max_audio_samples_per_call` | **Fits for PCM only** | A deterministic integer decimator can produce it. The PSG register-write log (the "raw audio" observation) has no `lf_*` channel; it stays in the native API. Observability loss, not machine loss. |
| `frame_rate_numerator/denominator` exact cadence | **Fits** | Rational from the pinned crystal and 14934 cycles/frame; do not substitute an assumed 60 Hz. |
| `lf_copy_media`/`lf_import_media` persistence | **Fits trivially** | 2609 base machine has no battery media; capability off. Intellicart/JLP flash would be a later profile. |
| `lf_capture_snapshot`/`restore` | **Fits** | Via the core's canonical serializer; exact-build identity rule already matches. |
| `lf_status` set (INVALID_ARGUMENT, UNSUPPORTED, LIMIT_EXCEEDED, BUFFER_TOO_SMALL, RESOURCE_EXHAUSTED) | **Fits** | "Missing system image" maps to INVALID_ARGUMENT or UNSUPPORTED by profile rule. |

### 4.3 Controller semantic loss, quantified [INFERENCE from sources]

- Each controller has 31 control surfaces: 12 keypad keys, a 16-direction disc, 3 side-button controls (AtariAge description; Blue Sky: disc 5-bit code, action 3-bit code, keypad 8-bit code) [SRC-COMMUNITY, SRC-HIST]. Each control grounds a set of lines; the byte read is the union (wired-low) of everything pressed; some unions are indistinguishable from other single presses ("not uniquely decodable": keypad with side/disc; two side buttons) [SRC-COMMUNITY, wiki Hand_Controller]. Keypad multi-key is electrically defined (wiki: keys 1+9 read $5A).
- Lossless logical state per controller: 12 keypad bits (multi-key) + disc enum (none + 16 = 5 bits) + 3 side bits = **20 bits**, so **40 bits for two controllers**. One-hot disc would be 31 bits per controller = 62 bits. The 32-bit mask cannot hold either.
- A 32-bit-compatible *lossy* subset exists: keypad as a 4-bit "single key" code (0 = none, 1-12) + 5-bit disc + 3 side = 12 bits per controller = 24 bits. It loses simultaneous multi-key presses only (keypad + disc + side combinations survive because they are independent fields).
- Do not send the 8-bit electrical byte instead: that moves the control-to-electrical function into the host, violating the owner requirement that controller electrical modeling is core-side and host mapping is separate (PROJECT.md Requirements; EMULATOR-DNA "keep electrical reads separate from host mapping").
- **[REC]** Core-native API carries the lossless 40-bit logical state (a struct or `uint64_t`). A LibFuturo route is possible two ways: (i) a documented lossy subset profile (24 bits) clearly labeled as such, or (ii) a shared-contract change appending a wider input field (LibFuturo records are `struct_size`-prefixed, so growth has precedent). PROJECT.md says not to propose shared-contract changes before a real consumer exists, so only (i) should be assumed for any early adapter work; (ii) is a later owner decision.

### 4.4 Content and system images: how to fit "single span + config id" [REC]

| Option | Description | Pros | Cons |
|---|---|---|---|
| **(a) Native multi-image load** (recommended for the core) | `load` takes EXEC, GROM, cartridge as separate typed spans | No packing ambiguity; each image gets its own length/digest/refusal; clean fixture manifests | Not expressible in current `lf_load_request` |
| **(b) Bundle envelope in one span** (recommended for any LibFuturo adapter) | Project-defined container: header, section table (kind, offset, length, SHA-256), payloads | Fits the existing single-span contract; identity is explicit | Adapter-level format that hosts must produce; the bundle is itself rights-bearing if it contains EXEC/GROM |
| **(c) Config id selects pre-registered system images** | The core or adapter embeds/locates EXEC/GROM | Single span = cart only | Violates "no filesystem in core"; EXEC/GROM cannot be embedded (rights); only works for an original replacement EXEC |
| **(d) Extend `lf_load_request` with secondary spans** | Shared-contract change | Direct | Owner gate; no real consumer yet |

Stated limit sizes: EXEC as u16 words = 8,192 bytes, GROM = 2,048 bytes [REC convention, matches common dump convention from my recollection of jzIntv usage; **verify**]; cartridge up to 64 K words = 128 KiB plus metadata. All far below the Muhtari adapter's 1 MiB-class limit.

### 4.5 Work unit and input timing at the API [REC]

- Natural *host* unit: one frame. Natural *machine/test* unit: CPU cycles. Keep both: native API uses cycles with a "to frame boundary" convenience; an adapter may expose frames only.
- Input sampled at the start of a call is faithful for logical state held across the call. Intra-frame input edges are expressed by the runner splitting calls at the edge cycle; the core logs the sample and actual start cycle with each call. This preserves "explicit machine-time boundary" without a timestamped input queue inside the core. A timestamped input queue is a possible later addition if tracing shows split-call overhead matters.
- EXEC/games poll the pads at arbitrary times; holding a sample for a whole advance is deterministic but is an approximation of a human's continuous press. That is fine for emulation; document it as the sampling contract.

---

## 5. Observation architecture (question 4)

### 5.1 Separate, distinct observations [REC] (consistent with AGENTS.md)

| Observation | Form | Where produced | Notes |
|---|---|---|---|
| Machine time/progress | cycle counter, frame counter, stop reason, overshoot | run result | never conflated with host time |
| Logical input | the 40-bit sample in force + start cycle | run result / trace | |
| Raw video | indexed frame (values 0-15 per pixel in bytes, pre-scaling, pre-palette), geometry fixed by profile | frame-complete event | Also expose a "border/blank" flag policy (e.g. reserved index) rather than baking RGB; palette is host policy |
| Raw audio, layer 1 | PSG **register-write log**: `(cycle, reg, value)` plus I/O port reads | PSG device | Lossless w.r.t. chip model; cheap to hash; independent of resampling |
| Raw audio, layer 2 | **Reference PCM**: integer-only deterministic decimation of PSG DAC output at a profile-fixed rate with an exact rational accumulator | PSG + observation | Needed for LibFuturo-style consumers and listening tests; clearly labeled "reference conversion", not chip truth. PSG DAC level curve/mixing nonlinearity is [UNKNOWN] for 8914 as wired (wiki gives the digital mix formula and noise LFSR but says 5- vs 6-bit volume variants differ) |
| Trace | ring of retired instructions `(cycle, PC, opcode words, regs-after)`, bus transactions, device register writes, INTRQ/BUSRQ/BUSAK/INTAK events | observer taps | Compile-time/run-time disabled by default; caller-owned ring; hooks are one-way |
| Deterministic hash | canonical serialization of defined state domains -> digest per domain | observe/ | Domains: CPU regs, each RAM region, STIC regs+latches, PSG state, video frame, audio log/PCM, event log |

### 5.2 Hashing rules [REC]

- Hash over a **canonical, endianness-independent byte serialization** that is also the snapshot format; never over raw struct memory (padding, pointers, host endianness).
- Use a cryptographic digest for fixture identity (SHA-256, as LibFuturo does) and keep per-domain digests so a mismatch localizes (video vs audio vs CPU).
- Repeated identical hashes prove repeatability, not accuracy (AGENTS.md). Each hash expectation in tests must be accompanied by its evidence tier.

### 5.3 Boundaries between runner, core, installed consumer, host [REC]

- **Runner**: owns time scripts, expected literals, failure taxonomy (harness failure, guest failure, timeout, cancellation, missing fixture). Uses only the public API and observer taps.
- **Installed consumer**: a separately built binary linking the *installed* package, run from a relocated prefix, exercising a small but non-trivial journey (load refusal, run, input, video, audio, snapshot). It is evidence about packaging and relocation, not accuracy.
- **Host**: pacing, palette, scaling, audio device, pad discovery, overlays. Out of scope for the first slices; the API must make it possible without any core change (palette and PCM policy are host/adapter concerns).

---

## 6. Architectural patterns

### Pattern 1: Data-driven timing profile with provenance tags
**What:** Every timing/behavior constant (frame cycles, window cutoffs, BUSRQ pattern, blocked-read values, power-on fill, STIC type) lives in a named profile table with a source tag (`primary`, `community`, `emulator`, `inference`, `measured`, `unknown`).
**When:** Always; section 1.4 shows the sources disagree, so these numbers *will* change.
**Trade-offs:** Slightly more indirection; large payoff in auditability and in letting a later hardware measurement change a number without touching logic.

### Pattern 2: Lazy device catch-up at access time plus a small event queue
**What:** Devices (STIC, PSG) advance only when accessed or when an event (INTRQ, BUSRQ, window close, frame complete) fires; the CPU's cycle-stamped access defines the catch-up target.
**When:** CPU-visible timing is cycle-exact but device internals are not CPU-visible between accesses.
**Trade-offs:** Needs discipline: any device query must go through catch-up. Faster than lockstep; the same structure is shown in jzIntv [SRC-EMULATOR].

### Pattern 3: Pure CPU stepper with a bus interface and a bus-phase schedule
**What:** `step(cpu, bus) -> (cycles, phase_schedule)`, no knowledge of devices; interrupt/BUSRQ checks at instruction boundaries per the manual.
**When:** Always. It makes the CPU independently testable with a flat-RAM bus and mock INTRM/BUSRQ lines.

### Pattern 4: Memory devices as descriptors (width mask, undriven policy, access gate, alias set)
**What:** The map is a table of `(range, device, width_mask, read_policy_when_blocked, write_policy, alias_group)`.
**When:** Needed for 10-bit EXEC/cart ROM, 8-bit scratchpad/GRAM/GROM, 16-bit System RAM, write-only aliases, VBLANK-gated windows, and later ECS/IntelliVoice/Intellivision II maps.
**Trade-offs:** Small per-access cost; large win in testability: the decode table can be dumped and compared against the sourced map literal.

### Pattern 5: Observation as one-way taps over a canonical state
**What:** Trace, hashing, video/audio publication read machine state; they cannot write it; machine behavior never depends on caller buffer capacity.
**When:** Always; this is what makes "observation vs semantics" separable.

### Pattern 6: Mini-EXEC as a first-class original system image
**What:** A project-authored, tiny EXEC replacement (reset code at $1000, interrupt handler at $1004 that writes the display strobe, minimal BACKTAB/GRAM setup) used by every PR-gate fixture. The real EXEC/GROM are user-supplied inputs for owner-local runs only.
**When:** From the first vertical slice. It removes the rights dependency from CI and makes fixtures self-describing.
**Trade-offs:** Passing mini-EXEC fixtures says nothing about real-EXEC boot; label evidence tier accordingly (real-EXEC runs are owner-local, non-redistributable, outside the PR gate).

### Pattern 7: Fixtures built by a project-authored mini-assembler
**What:** A small tool encodes CP1610 instructions into words for fixtures; the tool's encodings are themselves tested against the manual's opcode tables.
**When:** Needed because no cleared assembler/test ROM suite was found; using jzIntv's `as1600` (GPL) as a *build tool* is a license/process decision for the owner, not an assumption. [REC: own the tiny assembler.]

---

## 7. Anti-patterns

### Anti-Pattern 1: Treating jzIntv or MAME as the oracle
**What people do:** Copy their constants (110, 2907, 3796, `0x3FFF & addr`) as truth.
**Why wrong:** They are tuned estimates (jzIntv comments "seems... could need tuning", "Guess", "from Kevin Horton"); they disagree with the wiki and with measured TutorVision behavior; agreement between two emulators is not independent evidence.
**Instead:** Record each constant with provenance in the profile; classify as `emulator` evidence; resolve by hardware measurement or a cleared source.

### Anti-Pattern 2: A joystick-plus-buttons input model
**What:** `buttons` mask with N bits per player.
**Why wrong:** Loses multi-key keypad unions and the not-uniquely-decodable cases; pushes electrical decisions to the host.
**Instead:** Per-controller physical-control state and a core-side electrical function (4.3).

### Anti-Pattern 3: Frame-only STIC with no BUSRQ/BUSAK
**What:** Render from BACKTAB at frame time and ignore CPU stalls.
**Why wrong:** Removes the ~10% CPU steal and the late-BUSAK replay behavior; guest timing and display glitches diverge; blocks any honest timing claim.
**Instead:** Model BUSRQ/BUSAK at instruction boundaries with per-row latching; render per frame from latched state.

### Anti-Pattern 4: Per-pixel/master-tick lockstep "for accuracy"
**What:** Clock everything at 3.58 MHz.
**Why wrong:** No evidence that sub-CPU-cycle behavior is CPU-visible; constants aren't known to that precision; slows everything; adds false confidence.
**Instead:** Option B (3.1); revisit only if measured hardware shows a sub-cycle effect.

### Anti-Pattern 5: Host time, host RNG or uninitialized memory in machine state
**Why wrong:** Real RAM power-on contents are random; games can read them. If an emulator uses host randomness, determinism is lost; if it uses "whatever malloc gave", hashing is unstable.
**Instead:** Profile-specified deterministic power-on fill, with a seed field only if the owner wants variants.

### Anti-Pattern 6: Derived render caches with hidden state across snapshot
**Why wrong:** jzIntv's dirty flags need resync after state load [SRC-EMULATOR]; any state that influences future output but is outside the serialized state breaks restore equivalence.
**Instead:** Render as a pure function of latched, serialized state; differential test run vs snapshot-restore-run.

### Anti-Pattern 7: Letting buffer capacity or call slicing change machine behavior
**Why wrong:** Breaks "same inputs, same outputs".
**Instead:** Preflight refusal on short buffers; absolute-target stop rule; test slicing invariance (3.5).

### Anti-Pattern 8: Shipping or fixture-ing the real EXEC/GROM/MTE-201
**Why wrong:** Rights-bearing (AGENTS.md, PROJECT.md). A hash is identity, not permission.
**Instead:** Original mini-EXEC and diagnostics; manifests with source, recipe, digest, license, notices, allowed use.

---

## 8. Data flow

### 8.1 Per-advance flow

```
runner/host: input sample (40-bit logical), run target, output spans
   │
   ▼
API preflight (spans, target limits, loaded state)  ── refuse here, no mutation
   │
   ▼
machine.run:
   loop until target/frame boundary:
      scheduler: deliver due events (INTRQ -> line low, BUSRQ n, window close, frame complete)
      cpu.check at instruction boundary: BUSRQ? -> stall interval (BUSAK, bus-copy exit)
                                          INTRM & INTFF? -> INTAK, push PC, IAB vector (bus-copy enter)
      cpu.step -> (cycles, bus-phase schedule)
      for each phase: bus decode -> device.catch_up(t) -> device.access
           STIC/GROM/GRAM: gated by windows; blocked-read policy from profile
           PSG: register write -> write log; R14/R15 read -> controller electrical byte
      advance time
   on frame complete: STIC publishes indexed frame
   │
   ▼
result record (time, frames, overshoot, stop reason, validity) + filled spans + optional trace/log
   │
   ▼
runner: compare to literals, record hashes; host: palette, scale, present, audio device
```

### 8.2 Key data flows

1. **Content load:** host/runner reads files -> hands bytes to API -> loader validates -> typed regions -> memory map. Core never opens files.
2. **Video:** BACKTAB (CPU writes anytime) -> per-row latch at BUSAK time -> frame render from latched BACKTAB + GRAM/GROM + STIC regs latched in VBLANK + MOBs -> index frame -> caller span.
3. **Audio:** CPU write to $01F0-$01FF -> PSG register + log entry (cycle-stamped) -> synthesis in 4-cycle steps -> reference PCM accumulator -> caller span; the log is also published.
4. **Input:** host/runner sample -> API -> latched at call start -> PSG R14/R15 reads -> controller electrical function (core-owned).
5. **Reset/interrupt:** MSYNC (reset) sets PC = $1000; VBLANK INTRM -> INTAK/IAB -> $1004; both come from sources whose mechanism is [UNKNOWN] and are profile constants.

---

## 9. Build order and verification (question 5)

Dependencies: CPU and memory map are prerequisites for everything; the scheduler/run loop is needed before any device can be observed; PSG+input and STIC are independent of each other after the machine shell exists.

| Step | Component | Independently verifiable how | Oracle / evidence tier | Blocked by |
|---|---|---|---|---|
| 0 | Profile + claim ledger + fixture manifest format + mini-assembler | Review; mini-assembler encodings vs manual opcode tables | source expectation | owner choice of first profile (Q1) |
| 1 | **CP1610 stepper** on a flat-RAM mock bus | Hand-derived vectors from the GI manual: every opcode's flags, cycle counts (+W=0), interruptibility, SDBD, shifts, mock INTRM/BUSRQ ordering (BUSRQ before interrupt, sampled at instruction boundary, only after interruptible instructions) | source expectation (primary); named-fault controls (e.g. mutate a flag rule, expect failure) | nothing |
| 2 | **Memory map + media loader** | Dump decode table vs the sourced map literal; width/alias/undriven policy tests; load refusal matrix (short, long, wrong width, unknown profile) | source expectation (community) + original tests | step 0 |
| 3 | **Machine shell + scheduler + run loop + runner + installed consumer skeleton**, CPU-only | Original mini-EXEC + tiny program; expected register/memory state at cycle N and frame count literals; slicing invariance; two-instance isolation; relocated-install smoke | original diagnostic + deterministic repeatability + installed-host behavior | steps 1-2 |
| 4a | **PSG** (register file, I/O ports, tone/noise/envelope, write log, reference PCM) | Register-map and formula vectors from the PSG sources and GI catalog; hand-derived sample sequences; log hash | source expectation; original diagnostic | step 3; verify AY-3-8914 (not 8910) register layout |
| 4b | **Controllers + electrical model** | Table tests: each single control -> expected byte; selected combos; ambiguity cases only after the electrical table is cross-checked against a schematic | source expectation (needs sourcing); original diagnostic reading $1FE/$1FF | step 4a (ports live in PSG) and sourcing |
| 5a | **STIC control path**: INTRQ, enable handshake ($20/$21), windows, blocked-read policy, BUSRQ/BUSAK stalls, System RAM FIFO/bus-copy | Original program probing window edges by cycle counting; expected stall counts from the profile table | original diagnostic; **no independent oracle for the constants** (label as `emulator`/`community` until measured) | step 3; research flag |
| 5b | **STIC render**: FG/BG mode first, then Color Stack, borders/scroll, then MOBs, collisions, Colored Squares | Literal index frames derived by hand from BACKTAB/GRAM contents; one feature per fixture | original diagnostic | step 5a |
| 6 | **Playable/homebrew scenario** | Predicate on guest progress (register/memory marker), not a title-screen hash | playable guest progress; content rights must be cleared | steps 4-5; owner (rights) |
| 7 | Snapshot/restore, trace taps, performance baselines | Run vs snapshot-restore-run equivalence; trace determinism; named workloads before any optimization | deterministic repeatability | steps 3-5 |
| 8 | Hardware measurement of window/BUSRQ constants, power-on state, blocked-read values; real-EXEC owner-local runs | Physical console, owner gate | measured hardware | owner |

**Parallelism:** after step 3, 4a/4b and 5a/5b can proceed in parallel. Start the STIC *research* during step 1 because its inputs are the least settled.

### 9.1 Minimal vertical slice [REC]

"Slice 0" (end of step 3): original mini-EXEC + an original CPU diagnostic, loaded through the public API, run to cycle N and one frame boundary by the headless runner and by the relocated installed consumer; observations are cycle/frame counters, a literal memory/register expectation, and a state hash; refusal cases exercised. No video/audio claims.

"Slice 1" (end of 5b first pass + 4a minimal): same path plus one FG/BG card on screen with literal expected indices, plus one PSG tone with literal register log and sample-count vector. Timing claims for BUSRQ/window constants stay labeled "not hardware-qualified". This is also the earliest point where one visible scenario exists without third-party content.

---

## 10. Performance considerations

Performance is not the first-order architectural constraint here (owner: optimize only after representative workloads and correctness baselines exist).

| Concern | First profile | Later |
|---|---|---|
| CPU | ~0.9 M cycles/s is trivial for any native implementation | Table-driven decode; keep bus-phase schedule precomputed per opcode |
| STIC render | One frame render per ~14934 cycles, roughly 160x192 plus border | Dirty-region only if profiles show it matters; must not create hidden state |
| PSG | ~223.7 kHz internal step rate; ~3.7 K steps per frame | Batch between events; keep integer arithmetic |
| Trace | Off by default | Ring buffer sizing by caller |
| Multi-instance | Independent instances, no globals | Concurrency claim needs its own test |
| Browser/embedded | Possible later; same single-threaded core | Allocator injection already in the API |

First bottleneck [INFERENCE]: not CPU, but test iteration cost (frame hashing and fixture runs); keep the fast PR gate small and exact.

---

## 11. Integration points

### 11.1 External references and consumers

| Item | Integration pattern | Notes |
|---|---|---|
| LibFuturo | Optional adapter above the core API | Needs bundle envelope or contract extension; 32-bit mask only fits a documented lossy subset; PSG log not representable; owner gate for contract changes |
| jzIntv / MAME | Comparison implementation only | Not an oracle; GPL/per-file licensing; no code reuse |
| MTE-201 / EXEC / GROM / manuals | Local, owner-supplied only | Rights-bearing; never committed or redistributed |
| Fixture toolchain | Project-authored mini-assembler | Avoids GPL toolchain dependency by default |

### 11.2 Internal boundaries

| Boundary | Communication | Notes |
|---|---|---|
| CPU <-> bus | read/write/phase interface with cycle offsets | No device knowledge in CPU |
| Bus <-> STIC/System RAM | access with timestamp; gate state (bus copy/isolation) | System RAM and STIC treated as a coupled video subsystem |
| PSG <-> controllers | R14/R15 read callback | Controllers hold latched sample |
| Machine <-> observation | read-only taps | No feedback |
| API <-> runner/consumer/host | public API only | Independence of evidence routes |

---

## 12. Open items, unknowns and research flags (for roadmap)

| Topic | Flag | Why |
|---|---|---|
| STIC/System RAM control timing (INTRM duration, BUSRQ length/count, windows, handshake) | **Needs phase research; hardware measurement would settle it** | Sources disagree (1.4); the best measured source is for a different board |
| Blocked-read and power-on values | Needs research/hardware | Contradictory observations; games may depend on them |
| Raw framebuffer geometry (visible area, border/blanking, "159 vs 160" columns, vertical delay effect) | Needs research | Wiki gives 159 and 160 in different places; jzIntv uses 208-row merge; MAME has its own screen size; [UNKNOWN] what the raw definition should be |
| Cartridge formats and banking/paging | Needs research (owner Q4) | `.bin+.cfg`, `.rom`, Intellicart formats and ECS paging not examined beyond names |
| Hand-controller electrical codes (disc/keypad) | Needs sourcing | Extraction-grade table has a duplicate; wiki hex diagram wasn't readable |
| PSG: AY-3-8914 vs 8910 register layout; DAC levels; read-back behavior; I/O port timing | Needs research | Wiki warns not to mix documentation; bus protocol not covered |
| CP1610 manual vs silicon differences, EIS/interrupt latency, SIN/PCIT behaviors | Standard patterns once vectors authored; verify against hardware where possible | Manual is a 1975 document; Intellivision wiring details come from community sources |
| STIC1A / Intellivision II / ECS / IntelliVoice / PAL | Defer (separate profiles) | Different timing and maps |
| Language, ABI, versioning, build system | Owner decision | PROJECT.md Q7 |
| Pinned crystal rational (3.579545 MHz vs 315/88 MHz) | Owner/profile decision | Changes exact frame rate |

---

## Sources

**Primary / manufacturer**
- General Instrument, CP-1600 Microprocessor User's Manual, May 1975, https://bitsavers.mirrorservice.org/components/gi/CP1600/CP-1600_Microprocessor_Users_Manual_May75.pdf (downloaded and text-extracted 2026-11-09; read sections on pins, bus control, BUSRQ/INTR sequences, instruction tables). HIGH for CPU facts. (Third-party hosted scan; reuse rights not assessed.)
- General Instrument 1977 Data Catalog (listed in EMULATOR-DNA; **not read** this session), https://www.bitsavers.org/components/gi/_dataBooks/1977_GI_Data_Catalog.pdf. Needed for AY-3-8914/STIC parts.

**Historical in-house**
- Blue Sky Rangers, Master Component 2609 overview, https://history.blueskyrangers.com/hardware/2609.html (2026-11-09). MEDIUM-HIGH.

**Community reverse-engineering**
- Zbiciak, TutorVision / INTV88 Reverse Engineering Notes (2017, updated 2018), https://docs.google.com/document/d/13ApQEm1Xw_DcCAaXcQvpFot_BvrzSzeUo95EXCf41uc/pub (first ~100k characters read). MEDIUM-HIGH for measured items; different board.
- Intellivision Wiki pages (wiki.intellivision.us): CP1610, BUSRQ, Bus_Copy, System_RAM, Non-interruptible_Instructions, Memory_Map, STIC, PSG, Hand_Controller, VBlank_Interrupt, VBlank_Period_1, VBlank_Period_2, VBlank_Interval (2026-11-09). MEDIUM; several pages state they lack timing figures. The wiki ROM-format page required login.
- Deathskull Labs controller pinout (archive mirror), https://arcarc.xmission.com/Web%20Archives/Deathskull%20(May-2006)/games/tech/intvcont.html (2026-11-09). LOW as extracted (apparent duplicate row).

**Emulator source (observations only, not oracles)**
- jzIntv (FujiNet fork, GPL-2.0-or-later), `src/stic/stic.h`, `stic.c`, `stic_timings.h`, https://github.com/FujiNetWIFI/jzintv (2026-11-09). Read for facts; no code reproduced or recommended for reuse.
- MAME `src/mame/mattel/intv.cpp`, https://github.com/mamedev/mame/blob/master/src/mame/mattel/intv.cpp (clocks, map, controller wiring, 59.92 Hz). MAME `stic.cpp` and `handctrl.cpp` paths returned 404 at the URLs tried; not read.

**Project-internal (read)**
- ./.planning/PROJECT.md, ./AGENTS.md, ./.planning/bootstrap/EMULATOR-DNA.md.
- Sibling LibFuturo API, read-only: ../libfuturo/include/libfuturo/libfuturo.h, ../libfuturo/docs/contract.md, ../libfuturo/docs/muhtari-profile.md (method precedent only; not an Intellivision profile; not executed).

**Not found / not read**
- No primary STIC (AY-3-8900) datasheet was retrieved; the jzIntv documentation STIC page was not located. No hardware measurement of a 2609 was found. AtariAge threads on PSG timing returned HTTP 403.

---
*Architecture research for: deterministic Intellivision emulator core*
*Researched: 2026-11-09*
