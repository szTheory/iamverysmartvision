# Feature Research

**Domain:** Accurate, deterministic, headless, evidence-bound Mattel Intellivision emulator core (greenfield)
**Researched:** 2026-11-09
**Overall confidence:** MEDIUM (see "Confidence and method" at the end; hardware electrical/timing facts still need primary-document confirmation before requirements freeze)

## How to read this file

Every non-trivial claim carries a label. Labels are never merged.

| Label | Meaning |
|-------|---------|
| `[S#]` | Source observation: what a fetched page, file or source tree said when read on 2026-11-09. `S#` indexes the Sources table at the end (URL, access date). |
| `[I]` | Inference by this researcher from one or more `[S#]`. May be wrong. |
| `[R]` | Recommendation for the roadmap/requirements. Not an owner decision. |
| `[U]` | Unknown or not established by this research. |

Confidence tags follow the GSD seam: raw WebSearch/WebFetch output is LOW (`classify-confidence` returns LOW; MEDIUM only when `--verified`). I mark a claim MEDIUM only when it was cross-checked across two or more independent sources, or when it is read directly from an emulator's own source/docs and is a claim about that emulator (not about hardware). Emulator behavior is feature-landscape evidence only; it is never a hardware oracle.

---

## 1. Hardware and feature family map

"Difficulty" is the cost to emulate once the baseline machine exists. "Content" is how much existing software exercises it.

### 1.1 Consoles and variants

| Item | What it is (evidence) | Content prevalence | Difficulty | Dependencies | Classification `[R]` |
|------|-----------------------|--------------------|------------|--------------|----------------------|
| **Master Component 2609 (NTSC)** | Original Mattel console. CP-1610 CPU, AY-3-8900 STIC, AY-3-8914 PSG, AY-3-8915 color chip, 352x16-bit system RAM (RA-3-9600), 240x8-bit scratchpad, EXEC in two mask ROMs (RO-3-9502/9504), GROM RO-3-9503, 512 bytes GRAM `[S4 overview/master]`. INTV Funhouse lists 2609, 2609A, and unexplained "2609B" `[S12]`. | Essentially the whole commercial library: 124 cartridges plus one compilation after INTV Corp's 21 additions `[S14]` (MEDIUM). | Baseline (CPU High, STIC High, PSG Medium) | EXEC + GROM system images; cartridge map | **Table stakes** (first profile candidate; owner picks exact revision) |
| **Tandyvision One (Radio Shack 58-1000)** | Described as "really a 2609 with no gold trim" `[S12]`; Wikipedia describes it as the original with trim differences `[S14]`. Two sources agree (MEDIUM). | Same library | Zero beyond 2609 | Same as 2609 | **Alias of the 2609 profile**, not a separate machine `[I]` (no EXEC difference found; unverified by dump comparison `[U]`) |
| **GTE Sylvania MC100** | "Really a 2609A with GTE Sylvania logos" `[S12]` (single source, LOW). | Same | Zero | Same | Alias candidate `[I]` |
| **Sears Super Video Arcade (497.75022)** | Re-cased 2609-class machine with detachable controllers and a "blocker" chip that suppresses the "Mattel Electronics presents" title text; some title screens look odd `[S12]`. MAME ships a distinct `searsexc.bin` EXEC (own CRC; correct chip names unknown) `[S1]`. jzIntv's `csumexec` example distinguishes Inty 1, Inty 2 and Sears by checksum `[S4 examples/csumexec]`. Three independent emulator-side signals that the Sears EXEC differs (MEDIUM). | Same library | Low if EXEC is data; differences live in system image `[I]` | Distinct EXEC image identity | **Later slice**, cheap once system-image identity exists `[R]` |
| **Intellivision II (5872)** | Cost-reduced 1983 redesign: detachable non-tactile controllers, EXEC2 in one mask ROM RO-3-9506 holding the 4096-decle EXEC plus 256 extra decles at $0400-$04FF `[S4 overview, S10 EXEC wiki, S1]`. EXEC modified "to do stricter copyright check, which broke certain Coleco games and Word Fun" `[S12]`; Pixelated Arcade names Electric Company Word Fun and cites the changed EXEC `[S15]` (MEDIUM). A forum claim that the EXEC change also introduced a small timing error `[S16]` is LOW and unverified `[U]`. jzIntv recognises an EXEC2 file by size 2x(4096+256) bytes `[S4 cfg.c]`. | Same library; a handful of titles fail (Pixelated Arcade counts 6 documented) `[S15]` | Low-Medium: extra $0400 ROM window + different system image | EXEC2 image; $0400-$04FF map | **Later slice**; lockout is EXEC code behavior, so running the real EXEC2 should reproduce it without special-casing `[I]` |
| **INTV System III / Super Pro System (3504)** | INTV Corp's 1985 black/silver 2609-like unit, several keypad variants `[S12, S14]`. Whether its EXEC equals Inty 1 or Inty 2 is unresolved: forum posts say System III lacks the Inty II lockout while some Super Pro/Brazilian units contain EXEC2 `[S16]` (LOW). jzIntv looks for `superproexec.bin` only on a TutorVision "sptv" path `[S4 cfg.c]`, so purpose beyond that is unverified. | Same library | Low | System image identity | **Defer**; `[U]` EXEC identity. Do not claim support until a dump's identity is known |
| **Digiplay/Mattel 5368 (Brazil, PAL-M) and 3668 (Australia, "PAL")** | 5368 is "yet another 2609 variant" with PAL-M video; 3668 listed as Australian 240V/PAL `[S12]` (single source, LOW). IntyBASIC's prologue contains PAL/NTSC detection code, so PAL-aware software exists `[S17]`. | Region-specific; homebrew is PAL-aware | Medium: timing parameters (lines/frame, clocks) | Timing model parameterised by profile | **Later slice**, but make video/clock timing a profile parameter from day one `[R]` |
| **World Book TutorVision** | Unreleased collaboration; "only one known prototype unit exists" `[S12]`. jzIntv selects `wbexec.bin`/`wbgrom.bin` and a 16 KB EXEC ("INTV88/TutorVision"), with 2K GRAM `[S4 cfg.c]`; NINTV-DS emulates it `[S8]`. | Near zero | Medium | Extra EXEC/GROM, 2K GRAM | **Anti-feature for now** (listed so nobody assumes it) |
| **Intellivision Keyboard Component** | Computer add-on with its own 6502 CPU, TMS9927 CRT controller, dual-port RAM, keyboard matrix, tape TBD `[S1]`. Wikipedia: released late 1981 at $600 in Seattle and New Orleans only, cancelled Aug 1982, about 4,000 made `[S14]`. MAME labels it "(Unreleased)" and `MACHINE_NOT_WORKING` `[S1]`: the label and Wikipedia disagree on release status `[U]`. | Negligible (cart slots for BASIC cartridges only) `[S1]` | Very High (second computer) | Everything in the Master Component plus a second CPU/video/tape | **Anti-feature for the near term; revisit last** |

### 1.2 System images and memory

| Item | What it is (evidence) | Difficulty | Dependencies | Classification `[R]` |
|------|-----------------------|------------|--------------|----------------------|
| **EXEC** | The console "OS". 4096 ten-bit decles at $1000-$1FFF; Inty II adds 256 decles at $0400-$04FF `[S4, S10]`. On boot the EXEC checks $4800 and $7000 for viable ROM before the normal startup `[S10]`. CPU reset sets R7 to $1000 `[S16 wiki CP1610 snippet, LOW]`. Games call EXEC routines. MAME hashes: Inty 1 `exec.bin` CRC cbce86f7 (8192-byte file), Inty II `ro-3-9506-010.ic6`, Sears `searsexc.bin` `[S1]`. | n/a (data) | User-supplied (see 3.4) | **Table stakes**: model it as an identity-checked, user-supplied system image |
| **GROM** | 2048x8-bit at $3000-$37FF, reachable only through the STIC, and it "also holds a small amount of EXEC code that didn't fit in the EXEC ROM" `[S4 overview]`. MAME treats it as a STIC-device ROM `ro-3-9503-003.u21`, CRC 683a4158 `[S2]`. | Low | User-supplied | **Table stakes**. Consequence `[I]`: GROM is not just a font; the real EXEC needs it. A diagnostic that never calls EXEC and uses GRAM glyphs does not. |
| **GRAM** | 512 bytes at $3800-$39FF; $3A00-$3FFF read as aliases but are not writable `[S4 overview]`. 64 cards. | Low | STIC CPU-access windows | **Table stakes** |
| **Memory map / access windows** | STIC registers $0000-$007F readable/writable only in VBLANK windows ("first ~2000 cycles" / "~3780 cycles" with variants where CPU cannot see the read value) `[S4 memory_map.txt]`; Intellicart bank registers $40-$5F, IntelliVoice $80-$81, ECS UART $E0-$E3, ECS PSG $F0-$FF, scratch $100-$1EF, PSG #1 $1F0-$1FF `[S4]`. Unmapped reads should return $FFFF according to the jzIntv author `[S4 ecs.html annotations]`. | Medium-High | STIC timing model | **Table stakes**; the access windows are a requirement, not an optimization |
| **Power-on RAM contents** | IntyBASIC's prologue seeds its RNG "using trash in 16-bit RAM", i.e. real RAM is indeterminate at power-on `[S17]`. | Low | n/a | **Table stakes decision `[R]`**: pick an explicit, documented, deterministic power-on policy and put it in the configuration identity `[I]` |

### 1.3 Cartridges and cartridge-side hardware

| Item | What it is (evidence) | Content prevalence | Difficulty | Classification `[R]` |
|------|-----------------------|--------------------|------------|----------------------|
| **Standard Mattel cart** | ROM at $5000-$6FFF typical; MAME's default when no metadata is "a single 0x2000 chunk at 0x5000" `[S3]`. Cartridge ROM is 10-bit wide; images exist as 16-bit words, or with `ROMW 10` `[S4 id_tag/asm]`. | Majority | Low | **Table stakes** |
| **Cart RAM / WOM** | Some carts carry RAM; MAME has `intv_ram` and says it cannot tell RAM-equipped carts apart for `.ROM` files `[S3]`. jzIntv `.cfg` has `[memattr]` for RAM widths. | Small | Low | **Table stakes for the map model** (declared per-region attributes) |
| **Mattel-style page flipping** | ECS ROM banking: write `$xA5y` to `$xFFF` to select a 4K page at $2000/$7000/$E000 `[S4 ecs.html]`; World Series Major League Baseball also has a ROM at $E000 toggled by `$EA50`/`$EA51` writes to $EFFF `[S4 ecs.html, e-mail quoted by the author]`. MAME has `intv_wsmlb` and `intv_gfact` PCB types `[S3]`. jzIntv metadata spec: "extended to support Mattel style page-flipping, RAM, WOM" `[S4 id_tag]`. | A few titles (WSMLB, ECS carts, some prototypes `[S5]`) | Medium | **Later slice**; design the map model so banking is data-driven, not hard-coded `[R]` |
| **Intellicart (.ROM)** | Flash-RAM-style dev cart: bankswitching via $40-$5F, writable regions, 2K-word map granularity `[S4 id_tag, memory_map]`. | Homebrew / dev | Medium | **Later slice** |
| **Cuttle Cart 3 (CC3)** | Intellicart-compatible, `.ROM` autobaud byte 0x41 vs Intellicart 0xA8; IntyBASIC `--cc3` enables 8K-word extra memory `[S4 id_tag, S17]`. | Homebrew | Low-Medium | **Defer**; same family as Intellicart |
| **JLP ("Jean-Luc Project")** | Modern hardware: 16-bit RAM $8040-$9F7F, multiply/divide accelerators at $9F80-$9F8F, CRC-16 accel $9FFC/D, RNG $9FFE, save-game flash (768-word sectors, 96-word rows) `[S4 jlp.c]`. Metadata flags for "JLP accel" and "JLP flash sectors" exist; titles that require it will not run on Intellicart/CC3 `[S4 id_tag]`. | Homebrew (IntyBASIC `--jlp`) `[S17]` | Medium (also introduces persistent flash) | **Later slice**; the only early feature that forces persistence design |
| **LTO Flash / LUIGI** | LUIGI = "Locutus Universal Intellivision Game Image"; unifies BIN+CFG and ROM; implemented in jzIntv `src/locutus` `[S4]`. An AtariAge thread says a V1.0 spec PDF (2019) exists `[S16]`, not read by me `[U]`. LTO adds an "LTO mapper" and an ISA-extension flag `[S4 id_tag, jlp.c]`. | Homebrew (flash cart users) | Medium | **Defer**; importer-only, later |
| **FujiNet mailbox (jzIntv fork)** | Cart mailbox $9C00-$9FFF bridged to a TCP service `[S4 fork README]`. | Near zero | Out of scope | **Anti-feature** |

### 1.4 Controllers

| Fact | Evidence |
|------|----------|
| Each hand controller has a 12-key keypad (1-9, Clear, 0, Enter), a 16-direction disc, and **four** physical side buttons: two top buttons (left and right) that are functionally identical, plus a lower-left and a lower-right. | Top buttons identical, "S1 top, S2 bottom-left, S3 bottom-right" `[S11]`; the jzIntv metadata spec's controller-binding tag defines "Top Action Buttons / Lower Left / Lower Right" `[S4 id_tag]`; Wikipedia says "four side action buttons" `[S14]`. **Answer to the seed question: four physical buttons, three distinct electrical codes** (MEDIUM, three agreeing sources). |
| Controller is read as **8 active-low lines** (pins 1,2,3,4,6,7,8,9; pin 5 ground). Every input grounds a **combination of two pins**. | Pin table for 16 disc positions, 12 keys, 3 side codes: e.g. S1 = pins 6+8, S2 = pins 7+8, S3 = pins 6+7; K1 = pins 4+6; D9 = pin 4 only `[S11]` (single page; LOW until checked against a schematic). Reading with nothing pressed returns all ones in the low 8 bits `[S10 Hand Controller]`. |
| Master Component reads the right controller at $01FE and left at $01FF (PSG I/O ports); programs AND the two and invert to test "no input". ECS controllers read at $00FE/$00FF. | `[S10, S4 memory_map, ecs.html]` |
| Some combinations are **not uniquely decodable**: keypad + side button, keypad + disc, and any two side buttons together. | Wiki states it `[S10]`. Consistent with the pin table `[I]`: S1+S2, S1+S3 and S2+S3 all ground pins {6,7,8}; K1 {4,6} masks D9 {4}. A disc direction with a side button is decodable `[S10]`. |
| The jzIntv "Controller Bindings" tag was proposed for de-specification because it is prescriptive rather than descriptive; overlays are host-side content. | `[S4 id_tag]`; FreeIntv/NINTV-DS implement overlays host-side `[S7, S8]` |

`[I]` The electrical representation (8 lines x 2 controllers = 16 bits; 32 bits with ECS's two extra controllers) fits a 32-bit sampled button mask such as the LibFuturo candidate described in PROJECT.md, whereas a "logical button" representation (12 keys + 4 buttons + 16 disc positions per player) does not. This favors modeling the pins, not the glyphs. Unverified against the real LibFuturo header `[U]`.

### 1.5 Peripherals

| Item | What it is (evidence) | Content prevalence | Difficulty | Dependencies | Classification `[R]` |
|------|-----------------------|--------------------|------------|--------------|----------------------|
| **IntelliVoice** | SP0256-012 speech chip with its own 2 KB ROM image (MAME: `sp0256-012.bin`, 0x800 bytes; 3.12 MHz clock), command/FIFO at $0080-$0081 `[S1, S4 memory_map]`. jzIntv also needs `ivoice.bin` and ships SP0256 technical docs and an allophone library `[S4 doc/tech/ivoice.html, doc/voice]`. | Four Mattel titles (Space Spartans, B-17 Bomber, Bomb Squad, Tron Solar Sailer) plus WSMLB when ECS present `[S14]` (MEDIUM); some homebrew. | Medium-High (LPC filter synthesis, FIFO handshake, separate audio stream) | Second audio source; user-supplied voice ROM | **Later slice**; a separate raw-audio channel from the PSG |
| **ECS Computer Adaptor (#4182)** | Own power supply; second AY-3-8914 (6 simultaneous notes with the console's), 2K extra 8-bit RAM, 12K EXEC/BASIC ROM, cassette/printer UART, two extra controller ports `[S13 ecs]`. Memory facts: PSG #2 at $F0-$FF, UART $E0-$E3, RAM $4000-$47FF, banked ROMs at $2000/$7000/$E000, reset leaves page 0 selected on all so only $7000 is visible at reset `[S4 ecs.html]`. Keyboard is matrix-scanned by switching a PSG I/O port to output; enabled when `($F8 & $C0) == $40` `[S4 ecs.html]` (the author notes the e-mail's data was partly corrected). | "Six games were released for the ECS" `[S14]`, plus ECS-enhanced titles (e.g. four-player soccer) and ECS BASIC; homebrew exploits it `[S4 examples/ecs_kbd, ecscable]` | Medium | `ecs.bin` (12K decles); second PSG; bank logic; 4-controller input | **Later slice** (first peripheral candidate: cheapest after Inty II). UART/cassette/printer: last. |
| **ECS Music Synthesizer (#4188)** | 49-key keyboard plugged into the ECS input ports; "the synthesizing happens in the Master Component" `[S13 ecs]`. Only title: Melody Blaster `[S16]` (LOW). | One title | Low once ECS input exists | ECS | **Defer** |
| **ECS keyboard** | 48-key matrix via ECS ports; jzIntv keymap 2 emulates it `[S4]`. | Needed by ECS BASIC and a few titles | Low once ECS exists | ECS | **Defer** with ECS |
| **PlayCable, System Changer, Aquarius, Demo unit** | Listed in the Blue Sky Rangers hardware index with no description fetched `[S13 index]`. | Negligible | n/a | n/a | **Out of scope** `[U]` |

---

## 2. Feature Landscape

### Table Stakes (must have, or the core is not useful or credible)

Evidence column is the justification; "TS" means table stakes for THIS project's goals (deterministic, headless, evidence-bound, host-integrable), not for a consumer frontend.

| # | Feature | Why expected | Complexity | Notes |
|---|---------|--------------|------------|-------|
| TS1 | One explicitly named baseline profile (model/revision/region/timing, system-image digests, power-on policy) | Core Value forbids unqualified claims; every surveyed emulator needs EXEC+GROM to run a cart `[S4,S5,S6,S7,S8]` | Medium | 2609-family NTSC is the best-documented candidate `[S1,S4,S14]`; revision/region remains an owner choice `[U]` |
| TS2 | CP-1610 core, all instructions incl. interrupts, reset, SDBD, 10-bit ROM width, cycle counts | Nothing runs without it; pkIntvMCP alone claims "all 79 instructions, 348 tests" but is alpha and unvalidated against jzIntv `[S9]` (LOW) | High | Independent expected values needed (see section 5) |
| TS3 | STIC: Color Stack and FGBG modes, 20x12 BACKTAB, 8 MOBs with collisions, GROM/GRAM cards, border, h/v delay, interrupt, VBLANK CPU-access windows, bus isolation | Raw framebuffer and guest progress both depend on it `[S4 memory_map, stic.txt]` | High | Visible geometry is disputed across sources: Wikipedia "159x96 (right pixel not displayed)" `[S14]`, INTV Funhouse "192v x 160h" `[S12]`. Nominal vs visible must be pinned from primary docs `[U]` |
| TS4 | PSG (AY-3-8914): 3 tones, noise, envelope, I/O ports used as controller inputs | Audio and input both go through it `[S4 master.html]` | Medium | Controller reads are PSG port reads, so PSG and input are coupled `[I]` |
| TS5 | Declarative memory map: regions with read/write/bank attributes and ROM width; refusal on overlap or malformed content | Needed by standard carts, RAM carts, ECS, JLP later; MAME and jzIntv both express maps as data (hash-file extrainfo / `.cfg` `[mapping]`,`[memattr]`,`[bankswitch]`) `[S3,S4 bincfg.h]` | Medium | Do not hard-code per-game switches |
| TS6 | At least one self-describing cartridge importer with integrity checks (recommend `.rom`: CRC-16 on header and data, 53-byte header + 4 bytes/segment, documented) plus an internal content descriptor | `.bin` alone is ambiguous without a map (MAME defaults to 8K at $5000; FreeIntv keeps a size-keyed table `[S3,S7]`) | Medium | `.bin+.cfg` is the de-facto library format but its `.cfg` is "annoying to parse" and "poorly documented" per its own maintainer `[S4 id_tag]` |
| TS7 | System-image handling: user-supplied EXEC/GROM, identity check, refusal when missing, wrong size, or mismatched | Universal in the survey; jzIntv hard-fails (`exit(1)`) with the search path printed `[S4 cfg.c]`; all surveyed emulators exclude the images `[S4,S7,S8]` | Medium | See 3.4: no legal open replacement exists |
| TS8 | Controller as a multi-signal electrical device: 2 controllers x 8 active-low lines, sampled at an explicit machine boundary; ambiguity preserved, not decoded | Seed requirement; real combinations alias `[S10,S11]` | Low impl, High test | Four-controller (ECS) extension must not require redesign `[R]` |
| TS9 | Deterministic headless runner: bounded execution, fresh isolated state, scripted input, captured observations | PROJECT.md active requirement; no surveyed emulator documents a headless mode `[S4,S5,S6,S7,S8]` (absence in docs read, not proof) | Medium | Runner and core separate from any host |
| TS10 | Raw pre-scaling framebuffer (16-color indices plus border) and raw audio (PSG register events/samples) at a defined machine-time boundary | Seed requirement; "raw" capture is absent from the surveyed emulators' documented features, which offer AVI/WAV/GIF/BMP presentation capture `[S5,S6]` | Medium | Keep distinct from host PCM and scaling |
| TS11 | Configuration identity in every result: profile + system-image digests + peripherals + cart map + timing + power-on policy | Peripheral presence changes behavior: "PacMan ... a real Inty crashes in this combination [with ECS]" `[S4 id_tag]`; jzIntv falls back silently to no-ECS for "enhanced" titles `[S4 cfg.c]` | Medium | Make any fallback an explicit identity change, never silent |
| TS12 | Typed errors and refusal behavior (malformed content, unsupported map, missing image, short output buffer); failed load leaves prior state unchanged | PROJECT.md active requirement | Medium | jzIntv's `exit(1)` is the baseline to beat, not copy |
| TS13 | Original diagnostic fixture(s) with fixture manifest (source, recipe, digest, license, notices, allowed use) | No redistributable suite exists (section 5) | Medium | See 4: boot path without EXEC is the key design question |
| TS14 | Trace and inspection sufficient to debug the core: cycle-stamped CPU trace, register/memory dump, breakpoint at machine boundary | jzIntv's monitor debugger (step, run N cycles, trace-over, breakpoints, write watches, register history, timestamped trace, bus read/write trace) `[S4 debugger.txt]` | Medium | An interactive UI is not required; a programmatic trace is |

### Differentiators (where this project can compete)

| Feature | Value proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| Evidence-bound compatibility claims (profile + evidence tier + artifact digests) | The Core Value; no surveyed emulator makes per-profile evidence claims `[S4..S8]` | Medium (process more than code) | Needs a results schema that records tier: source expectation / public vector / original diagnostic / independent comparison / hardware / repeatability / guest progress / installed host |
| Input recording and replay at a machine-time boundary | jzIntv's "demo" recorder captures STIC/PSG writes, explicitly not input and not IntelliVoice `[S4 demo.h]`; jzIntv's "movie" is video capture `[S4 jzintv.txt]`; Nostalgia offers AVI/WAV capture and network play `[S6]`. No input replay found in any of them `[U: not exhaustive]` | Low-Medium once timing boundary is defined | Natural fit for deterministic core; also the regression vehicle for guest-progress tests |
| Versioned save states bound to configuration identity | FreeIntv, NINTV-DS (3 slots), Nostalgia and MAME (`MACHINE_SUPPORTS_SAVE`) have states `[S1,S6,S7,S8]`; jzIntv's serializer design explicitly wants "verify against init" so states incompatible with the running config are refused, status of implementation unverified `[S4 serializer/design.txt]` | Medium | Also the substrate for host rewind; rewind itself stays in the host |
| Machine-readable trace and state digests | pkIntvMCP shows demand for AI/agent-driven debugging (alpha, no license, no external validation) `[S9]` | Low-Medium | Deterministic digests at boundaries are what make cross-run and cross-implementation comparison possible |
| Open, provenance-tracked conformance diagnostics (CPU, STIC access windows, PSG, controller aliasing) | Fills a verified ecosystem gap (section 5) | High (authoring + independent expected values) | Releasable only if the owner chooses a license and the fixtures are original |
| Compatibility metadata model with refusal | jzIntv's tag spec has Don't-care / Supports / Requires / Incompatible for ECS, voice, keyboard component, Inty II, TutorVision plus JLP/LTO flags `[S4 id_tag, cfg.c]`; negotiating config against content and refusing "Incompatible" is cleaner than silent fallback | Low-Medium | Reuse the concept, not the file layout; tags are optional and appended to `.ROM` so they are not authoritative `[S4]` |
| Content identification database keyed by hash, rights-neutral | MAME uses hash-file extrainfo; NINTV-DS falls back to CRC32 search for BIOS by content; FreeIntv keeps a size-keyed RAM table; libretro "database" field exists `[S3,S7,S8]` | Medium | Hash establishes identity, not rights (AGENTS.md). Ship only hashes/metadata you authored |
| Installed-consumer proof | Project policy, absent from landscape | Medium | Separate evidence from source-tree tests |

### Later verified slices (not table stakes for v1; classify only)

| Slice | Order hint `[R]` | Reason |
|-------|------------------|--------|
| Sears and Inty II profiles | After baseline | Pure system-image identity plus $0400 window; cheap once TS7/TS11 exist |
| `.bin+.cfg`, page-flipped carts, ECS | After baseline | Needs the map model (TS5) proven first |
| IntelliVoice | After ECS or independently | Own audio stream; heavy chip emulation; 4-5 titles |
| PAL/PAL-M | When timing is parameterised | Timing-only difference if EXEC is shared `[U]` |
| JLP / LTO / Intellicart / CC3 / LUIGI | Late | Homebrew-centric; introduces persistence |

### Anti-Features (deliberately NOT build)

| Feature | Why requested | Why problematic | Alternative |
|---------|---------------|-----------------|-------------|
| Bundling or auto-downloading EXEC, GROM, ECS, IVOICE, Sears/Inty II images, MTE-201, commercial ROMs | "Works out of the box"; BIOS packs exist (retrobios lists them) `[S20]` | Rights-bearing; jzIntv's own fork excludes them and says they are copyrighted `[S4 fork README]`; a hash proves identity not rights | User-supplied path/bytes at a host boundary; core verifies digests and refuses |
| A clean-room "replacement EXEC/GROM" to run commercial games | Removes the BIOS requirement | None found anywhere (3.4); games call undocumented EXEC entry points; GROM contains EXEC code; fidelity claim would be unfalsifiable `[S4 overview]` | For diagnostics only: a project-authored boot stub in the EXEC slot, clearly labeled "not an EXEC" (section 4) |
| High-level EXEC emulation / per-game hacks | Speed, "compatibility" | Destroys evidence value; contradicts Core Value | Run the real EXEC image |
| Silent fallbacks (drop ECS if `ecs.bin` missing) | Convenience; jzIntv does it for "enhanced" titles `[S4 cfg.c]` | Changes machine behavior without changing identity | Explicit config change reported in the result |
| Compatibility percentages / "plays X" labels | Marketing | Contradicts Core Value | Per-profile evidence table |
| Overlays, remapping, controller discovery, windowing, audio device, netplay, rewind, cheats, high-score files inside the core | Standard emulator conveniences (FreeIntv overlay/mini-keypad, NINTV-DS high scores, Nostalgia netplay) `[S6,S7,S8]` | Host-layer concerns per owner North Star | Keep in hosts; core exposes pins, states and observations |
| Shipping controller-overlay images | Nice UX | Overlays are third-party artwork; NINTV-DS itself says sample overlays are personal-use only `[S8]` | Overlay metadata only if a host needs it |
| Keyboard Component, ECS UART/cassette/printer, TutorVision, FujiNet in early scope | "MAME has it" | MAME's own driver marks the keyboard component not working and comments out the UART `[S1]`; near-zero content | Revisit only on evidence of content/user value |
| Frame skipping, wall-clock pacing, speed hacks in the machine | Smooth play | Violates host independence | Host paces; core advances by explicit cycles |
| Vendoring GPL emulator code or GPL ROM/example sources without license review | Fast start | jzIntv is GPL-2.0-or-later `[S4]`; MAME driver is BSD-3-Clause per its header `[S1]` but license is per-file; neither is an oracle | Method-level learning only |

---

## 3. What existing emulators offer (feature-landscape evidence only)

### 3.1 Comparison matrix

"Not found" means not found in the pages/source I read, not proof of absence.

| Capability | jzIntv | MAME `intv` driver | Nostalgia | FreeIntv (libretro) | NINTV-DS (Bliss lineage) |
|------------|--------|--------------------|-----------|---------------------|--------------------------|
| License | GPL-2.0-or-later `[S4]` | BSD-3-Clause (driver header) `[S1]` | Not stated `[S6]` | GPLv2+ `[S7]` | Custom permissive-with-credit; sample overlays personal-use `[S8]` |
| Machines | Inty 1, Inty II (EXEC2 detected by size), Sears via file choice, TutorVision `[S4 cfg.c]` | `intv`, `intvsrs`, `intv2`, `intvoice`, `intvecs`, `intvkbd` (not working) `[S1]` | Master Component; ECS and IntelliVoice `[S6]` | Master Component `[S7]` | Master Component; TutorVision `[S8]` |
| ECS | Yes (`ecs.bin`; auto-enables for "enhanced" titles) `[S4]` | Yes: PSG, RAM, banked ROM; UART commented out `[S1]` | Yes incl. keyboard `[S6]` | "does not currently emulate" ECS `[S7]` | Keyboard, extra sound, RAM; no cassette/UART `[S8]` |
| IntelliVoice | Yes (`ivoice.bin`) `[S5]` | Yes (SP0256 device) `[S1]` | Yes `[S6]` | Code present (`src/ivoice.c`, credits Zbiciak) though docs do not list it `[S7]` | Yes `[S8]` |
| Keyboard Component | Not found; metadata flag `kc_compat` exists `[S4]` | Modeled, not working `[S1]` | Not found | Not found | Not found |
| JLP / LTO / LUIGI | Yes (`src/jlp`, `src/locutus`) `[S4]` | Not found | Not found | Not found | JLP yes `[S8]` |
| Intellicart/CC3 `.ROM` | Yes `[S5]` | `.ROM` loader `[S3]` | Not stated | Yes (checks 0xA8 magic, ignores segment CRC) `[S7]` | Yes `[S8]` |
| `.bin+.cfg` | Yes (parses `.cfg`) `[S4]` | `.bin` + hash-file extrainfo, does **not** parse `.cfg` `[S3]` | `.bin`/`.int` | Extension `.bin`; size-keyed table `[S7]` | `.bin` + optional `.cfg`; `.int` wrapper `[S8]` |
| Page-flipped/special carts | `[mapping]`/`[bankswitch]`; WSMLB listed `[S5]` | PCB types STD, RAM, GFACT, WSMLB, VOICE, ECS, KEYCOMP `[S3]` | Not stated | Size-keyed RAM config `[S7]` | Up to 1024 KB images `[S8]` |
| Debugger | Yes: monitor with step/run/trace-over, breakpoints, watches, register history, timestamps `[S4 debugger.txt]` | MAME platform debugger (not verified for this driver) `[U]` | Not mentioned `[S6]` | Not found | Not found |
| Save states | Serializer design doc; implementation status unverified `[S4]` | `MACHINE_SUPPORTS_SAVE` on every config `[S1]` | Yes `[S6]` | Yes; no SRAM saves, rewind, cheats `[S7]` | Yes, 3 slots `[S8]` |
| Input recording/replay | Not found (demo recorder captures STIC/PSG writes, not input) `[S4 demo.h]` | MAME-wide input recording exists in general `[U: not verified for this driver]` | Not found | Not found | Not found |
| A/V capture | GIF screenshots, movie recording (F10) `[S4,S5]` | MAME-wide `[U]` | AVI+sound, WAV, BMP `[S6]` | Frontend screenshots `[S7]` | n/a |
| Headless mode | Not found in docs read `[U]` | MAME `-video none`-style options exist in general `[U: not verified]` | Command-line for front-ends `[S6]` | Via libretro frontend only | n/a |
| Peripheral/ROM "enhanced" negotiation | Metadata tags drive auto ECS/voice/TV enable `[S4 cfg.c]` | Software lists with slot options `[S1]` | Not stated | None | None |
| Overlays | n/a | n/a | Custom overlay display `[S6]` | Per-game PNG overlays, mini-keypad, controller swap `[S7]` | Custom overlays, disc overlay `[S8]` |

### 3.2 Other tools seen

| Tool | Relevance |
|------|-----------|
| **pkIntvMCP** (peterkaminski) | Alpha CP-1600 emulator + MCP debugger, Dec 2025: CPU core "complete", 348 own tests, "external validation against jzIntv has not been completed", no license shown `[S9]` (LOW). Shows an AI-debugging expectation; not a dependency candidate. |
| **IntyBASIC / as1600 / SDK-1600** | The homebrew toolchains that generate most new content; every IntyBASIC program boots through the EXEC `[S17]`. |
| **Bliss** (Kyle Davis) | Ancestor of NINTV-DS and FreeIntv-adjacent work; GPL-2.0 mirror exists `[S8]`. |

### 3.3 How each handles missing system images

| Emulator | Behavior | Evidence |
|----------|----------|----------|
| jzIntv | Default names `exec.bin`, `grom.bin`, `ecs.bin`, `ivoice.bin` searched on `JZINTV_ROM_PATH`; if EXEC or GROM cannot be read, prints "ERROR: Could not read EXEC/GROM image '<name>'", dumps the search path and **exits 1**. If ECS is only "enhanced" and `ecs.bin` is missing, prints a NOTE and continues without ECS. TutorVision images fall back to normal EXEC/GROM with a NOTE. | `[S4 cfg.c]` (direct source read; MEDIUM for jzIntv behavior) |
| MAME | System ROMs declared with CRC/SHA1 per machine; GROM is a STIC-device ROM; missing/mismatched ROM handling is MAME's generic mechanism `[I]` (not fetched). | `[S1,S2]` |
| FreeIntv | libretro `.info` declares `exec.bin` and `grom.bin` as non-optional firmware with MD5s; core joins `system/` + name and calls `loadExec`/`loadGrom`; I did not trace failure handling `[U]` | `[S7]` |
| NINTV-DS | User must supply; searches several directories, then **by CRC32 if the filename is not found** | `[S8]` |
| Nostalgia | EXEC and GROM "needed to play all games", placed in install dir | `[S6]` |

`[I]` Pattern: filename-keyed lookup (case-sensitive on Linux per forum reports `[S16]`) is the norm; content-keyed identity (NINTV-DS, MAME) is rarer and is what an evidence-bound core needs.

### 3.4 Legal replacement EXEC/GROM images: none found

- Searches for an open or clean-room EXEC/GROM replacement returned only references to the original Mattel files; FreeIntv, NINTV-DS, OpenEmu Bliss and jzIntv all require them `[S7,S8,S4,S20]`. No project was found that wrote a replacement. This is an evidence limit, not proof of absence `[U]`.
- Where the originals come from: jzIntv's documentation points to the Intellivision Lives! / Rocks! CDs `[S4 jzintv.txt, S5]`; the fork README says the images are copyrighted and excluded from the repo `[S4]`. Whether any rights-holder has granted emulator-authors redistribution permission is `[U]`.
- `[R]` Treat EXEC/GROM/ECS/IVOICE as user-supplied only. Do not plan any phase on the assumption that a legal open EXEC will appear.

---

## 4. Legally available content for an early "playable/homebrew scenario"

Rights are recorded as stated in the artifact read, with observed limits. This is not legal advice; license choice for this project is an owner gate.

| Candidate | Rights as stated | Gaps / cautions | Runs without EXEC? | Recommendation `[R]` |
|-----------|------------------|-----------------|--------------------|----------------------|
| **Project-authored original diagnostic/demo** (assembler words generated by a project script) | Owner-authored; no third-party terms | Needs independent expected values; needs an assembler/encoder (own script, or GPL `as1600` as a tool) | Yes, if the project supplies its own boot stub at the reset target | **Best first fixture** |
| **Geese** (jzIntv `examples/geese/geese.asm`) | Header: "offered ... under the Creative Commons No Restrictions license (CC0 1.0)", `CFGVAR license = "CC CC0 v1.0"`; author's 2003 note calls it the only ROM he released into the public domain `[S4 examples/geese, GOOD_ROM_NAMES.txt]` | Same header says the **included library files may have other licenses** (it includes `../library/gimini.asm`, which falls under the examples-wide GPL statement `[S4 examples/README.txt]`); the examples README says "all of these programs" are GPL, contradicting the CC0 header on this one file; read in the FujiNet fork of the 2020-08-22 snapshot, not upstream `[U]` | No: written for the EXEC | **Usable as an optional, externally fetched demo after confirming upstream and the library-include question**; not as a vendored fixture without that check |
| **Other jzIntv examples** (hello, handdemo, balls, mem_test, mob_test, banktest, ...) | GPL ("made available ... under the GNU GPL"; 4-Tris and mem_test headers say GPLv2 or later) `[S4]` | Copyleft obligations; new-code license is an owner gate; these are behavior demos, not conformance tests | Mostly no | **Reference only**; fetch on demand if ever used |
| **4-Tris** (Zbiciak, 2000) | GPLv2+ (`CFGVAR license`), and redistributed as "open-content" in the libretro FreeIntv repo `[S4,S7]` | Music data includes "Don't Worry, Be Happy" (McFerrin) next to Tchaikovsky `[S4 4-tris.asm]`; the author's GPL grant plausibly does not cover the song's composition `[I]`; contains source for a full game | No | **Do not vendor.** Note as a well-known test of real-world play |
| **IntyBASIC compiler** (nanochess) | BSD-2-Clause-style `LICENSE`, "Copyright (c) 2014 Oscar Toledo G."; GitHub reports "other/NOASSERTION" `[S17]` | Compiled programs embed `intybasic_prologue/epilogue.asm` (same repo), so binary redistribution likely carries the notice requirement `[I]` | Needs EXEC | **Acceptable toolchain** for an optional demo; confirm notices |
| **IntyBASIC `samples/*.bas`, `contrib/*.bas`** | Headers read (game1, sprites, lander, frame, controller, screen, test, hello, keypad): author/date attribution only; **no per-file license text** `[S17]`. Contrib files credit other authors (DZ-Jay, Mark Ball) | Whether the repo LICENSE extends to samples/contrib is `[U]` | Needs EXEC | **Do not vendor** without a permission note from the author |
| **Community homebrew lists** (Intellivision Revolution, AtariAge) | The page has columns title/description/programmer/source with links to forum threads; no license data `[S19]` | Rights-bearing by default | n/a | **Excluded** |
| **MTE-201, FW Diagnostics, commercial ROMs** | Mattel service cartridge v3 "is the one usually found in common ROM sets"; FW Diagnostics has an official ordering thread (commercial); other versions possibly undumped `[S16]` (LOW) | Rights-bearing | n/a | **Excluded**; MTE-201 only to discover diagnostic categories (per PROJECT.md) |

### Boot-path design consequence `[I]`

Every game and demo above relies on the EXEC to initialize the machine. jzIntv's own 4-Tris uses the cartridge header's "Special Title Screen" bit to hijack the EXEC's initialization `[S4 4-tris SOURCE.txt]`. Therefore:
- A **visible playable scenario in CI** has two honest routes: (a) a bare-metal original program plus a project-authored boot stub in the EXEC slot, tagged as a distinct system-image identity ("not an EXEC"); or (b) any real game/homebrew needing a user-supplied EXEC, which can never run in public CI. `[R]` pick (a) for the gate and (b) as an owner-local, clearly labeled scenario.
- The stub must be recorded in the fixture manifest and must not be presented as an EXEC replacement (see anti-features).

---

## 5. Is there a redistributable conformance suite? (bounded re-check)

Result: **none found.** Re-check covered GitHub repository search (queries: cp1610, intellivision test rom, intellivision emulator, intellivision diagnostic), the jzIntv tree, the FreeIntv tree, web search for MTE-201/diagnostic cartridges and CP1610 test suites.

| Artifact | What it is | Rights / usability | Evidence tier it could support |
|----------|-----------|--------------------|-------------------------------|
| MTE-201 service cartridge (v3 widespread; v4.x variants) | Mattel service diagnostic; failure codes appear in service manuals | Rights-bearing; no permission found `[S16, EMULATOR-DNA]` | Category discovery only |
| FW Diagnostics / "Inty II diagnostic ROM available" | Community/commercial diagnostic carts in AtariAge threads | Rights unknown `[S16]` (LOW) | None until rights known |
| jzIntv `mem_test`, `mem_test_ra39600`, `mob_test`, `banktest`, `kbd_test`, `syn_test`, `csumexec`, `jlp_test` | Memory tests (walking 1s/0s etc.), MOB movement demo, bank test; written to verify Intellicart hardware or demo STIC features `[S4 examples]` | GPL; need EXEC; no expected-value contract; they are hardware self-tests, not conformance vectors | Behavior reference at most (independent implementation comparison tier, not oracle) |
| libasm `test_asm_cp1600.cpp`, `test_dis_cp1600.cpp` (tgtakaoka) | Assembler/disassembler encoding tests | Repo `LICENSE.md` shows Apache-2.0 text while GitHub reports NOASSERTION; provenance of vectors not checked `[S18]` | Possible **public vector for decode/encode** only, not execution semantics; verify license and provenance first |
| pkIntvMCP's 348 tests | Self-authored by an alpha project, no license shown `[S9]` | Unusable | None |
| jzIntv `bincfg/bc_test.c`, `misc/test_crc32.c` etc. | Unit tests of tooling, not hardware | GPL | None |
| `cp1600_ref.pdf` / `cp1600_summary.txt` / `psg.txt` / `stic.txt` / `memory_map.txt` in jzIntv docs | Documentation, not tests | GPL-era documentation of mixed provenance `[S4]` | Source expectation tier |
| GI 1977 data catalog (bitsavers) | Component documentation `[EMULATOR-DNA]` | Not re-fetched here | Source expectation tier |

`[R]` Plan on original, source-built diagnostics with literal expected results (the project's own recommendation) and record the evidence tier of each; do not claim a "passes suite X" tier.

---

## Feature Dependencies

```
System-image identity (TS7) ──requires──> Configuration identity (TS11)
Configuration identity (TS11) ──requires──> Profile definition (TS1)
CPU core (TS2) ──requires──> Memory map model (TS5)
STIC (TS3) ──requires──> Memory map model (TS5) + VBLANK access windows (TS3)
PSG (TS4) ──requires──> Controller line model (TS8)   [controllers are PSG I/O ports]
Headless runner (TS9) ──requires──> CPU + STIC + PSG + Controller + importer (TS6)
Raw capture (TS10) ──requires──> STIC + PSG + machine-time boundary (TS9)
Diagnostic fixture (TS13) ──requires──> Boot path decision (EXEC-less stub vs user EXEC)
Save states ──requires──> Configuration identity (TS11) + stable component state layout
Input record/replay ──requires──> Headless runner (TS9) + Controller model (TS8)
Intellivision II / Sears ──requires──> System-image identity (TS7)
ECS ──requires──> Map model with banking (TS5+) + 4-controller input (TS8) + PSG #2
ECS keyboard / Music Synthesizer ──requires──> ECS
IntelliVoice ──requires──> second raw audio stream + voice image identity (TS7)
JLP / LTO / Intellicart ──requires──> Map model + persistence design
PAL profiles ──requires──> Timing as profile parameter (TS1)
Keyboard Component ──requires──> everything above + second CPU/CRT/tape (defer)
```

### Dependency notes

- **TS8 before TS4's host story:** the PSG I/O ports are where the controllers are read, so the controller model must be defined with the PSG, not bolted on in a host `[S4 master.html, S10]`.
- **TS11 before save states and peripherals:** because peripheral presence changes behavior and can crash a title (ECS + Pac-Man), identity must exist before the first optional peripheral is added `[S4 id_tag]`.
- **TS13 depends on a boot-path decision** that is not a pure feature choice (see section 4). Decide it before authoring fixtures.
- **ECS is the natural first peripheral** because it reuses PSG, banked ROM and input work, whereas IntelliVoice adds a whole new chip `[I]`.
- **Conflicts:** silent fallback (jzIntv style) conflicts with configuration identity; in-core overlays/remapping conflict with the host-independence constraint.

---

## MVP Definition `[R]` (classification only; scope is the owner's decision)

### Launch with (v1 candidate)

- [ ] TS1 baseline profile + TS7 system-image identity/refusal + TS11 configuration identity.
- [ ] TS2/TS3/TS4/TS5 minimum machine to run a bare-metal original diagnostic and expose raw video/audio (TS10) through the headless runner (TS9), with controller pins (TS8).
- [ ] TS13 original fixture + manifest, including the boot-stub decision.
- [ ] One visible scenario that is original and CI-runnable (route (a) in section 4).
- [ ] TS6 one importer (`.rom` recommended) and TS12 typed refusals.

### Add after validation (v1.x)

- [ ] Real EXEC/GROM boot of a user-supplied image (owner-local evidence tier).
- [ ] Save states, input record/replay, trace/digest tooling.
- [ ] Inty II and Sears profiles (system-image identity only).
- [ ] `.bin+.cfg` and Mattel-style banking.

### Future consideration (v2+)

- [ ] ECS (PSG #2, banked ROM, 4 controllers), then ECS keyboard/synth.
- [ ] IntelliVoice.
- [ ] PAL/PAL-M profiles.
- [ ] JLP/LTO/Intellicart/CC3/LUIGI with persistence.
- [ ] Keyboard Component (probably last or never).

## Feature Prioritization Matrix

| Feature | User Value | Implementation Cost | Priority |
|---------|------------|---------------------|----------|
| Baseline profile + system-image identity + config identity | HIGH | MEDIUM | P1 |
| CP-1610 / STIC / PSG / memory map | HIGH | HIGH | P1 |
| Controller pin model + headless runner + raw capture | HIGH | MEDIUM | P1 |
| Original fixture + manifest + EXEC-less boot stub | HIGH | MEDIUM | P1 |
| `.rom` importer + typed refusals | HIGH | MEDIUM | P1 |
| Trace/inspection | MEDIUM-HIGH | MEDIUM | P1/P2 |
| Save states bound to config identity | MEDIUM | MEDIUM | P2 |
| Input record/replay | MEDIUM-HIGH | LOW-MEDIUM | P2 |
| Content identification database (hash metadata) | MEDIUM | MEDIUM | P2 |
| Inty II / Sears / alias profiles | MEDIUM | LOW-MEDIUM | P2 |
| `.bin+.cfg`, banked carts | MEDIUM | MEDIUM | P2 |
| ECS | MEDIUM | MEDIUM | P3 |
| IntelliVoice | LOW-MEDIUM | MEDIUM-HIGH | P3 |
| PAL variants | MEDIUM | MEDIUM | P3 |
| JLP/LTO/Intellicart/CC3/LUIGI | LOW | MEDIUM | P3 |
| Keyboard Component / TutorVision / FujiNet | LOW | VERY HIGH / MEDIUM | Not planned |

## Competitor Feature Analysis (summary; detail in section 3)

| Feature | jzIntv | MAME | FreeIntv | Our approach `[R]` |
|---------|--------|------|----------|--------------------|
| Missing system image | Hard error, exit 1 | Generic MAME ROM check `[I]` | libretro firmware flag | Typed refusal with expected digest, no partial state |
| Identity | Filename + `.cfg`/tags | CRC/SHA1 per machine + hash files | Filename + size table | Content digest + explicit config identity |
| Input | Keyboard keymaps, pads | MAME input | RetroPad mapping + overlays | Electrical pins at a machine boundary; mapping in host |
| Accuracy claims | Implicit | Implicit | Implicit | Per-profile evidence tier |
| Headless/observability | Debugger, movie, demo recorder | MAME debugger | Frontend only | Runner + raw capture + digests |

---

## Unknowns carried forward `[U]`

1. Visible vs nominal STIC geometry (159x96 vs 192x160 claims) and exact CPU/STIC clock relationships: primary documents (GI data books, Mattel service/engineering docs) not yet read for this research. EMULATOR-DNA notes the same gap.
2. Controller pin table is from a single hobbyist page; verify against schematic/service manual before freezing the electrical contract.
3. Whether Tandyvision, Sylvania MC100, System III/Super Pro share the 2609 EXEC bytes (requires dumps; no dump comparison done).
4. Whether PAL machines use a different EXEC/GROM, and PAL timing numbers.
5. LUIGI spec text and licensing; `.ROM` spec rights (jzIntv ships `IntellicartManual.booklet.pdf`; redistribution terms unchecked).
6. FreeIntv's failure behavior when system images are missing; whether MAME or jzIntv have headless/input-recording options in their current builds.
7. Upstream (not fork) license text for Geese and its library include.
8. Whether any rights-holder grants permission to distribute EXEC/GROM to emulator users.
9. libasm CP1600 vectors' provenance and whether Apache-2.0 truly covers the test files.
10. Whether the LibFuturo 32-bit mask semantics permit pin-level controller input as inferred in 1.4.

## Confidence and method

| Area | Confidence | Notes |
|------|------------|-------|
| Hardware family facts (variants, ECS, IntelliVoice, formats) | MEDIUM | Cross-checked across emulator source/docs, INTV Funhouse, Blue Sky Rangers, Wikipedia; several single-source LOW items flagged inline |
| Controller electrical facts | MEDIUM-LOW | Wiki + metadata spec + pinout page agree on 3 side-button codes; full pin table single-source |
| Emulator feature matrix | MEDIUM | Read from each project's README/source; "not found" cells are absence in what I read |
| Legal content | MEDIUM | License text read for Geese, 4-Tris, IntyBASIC; samples unresolved |
| Conformance suite absence | MEDIUM | Bounded search, repeated; still an evidence limit |

Method note: the `research-plan` / `research-store` cache seam and MCP providers were not used; research ran through built-in WebSearch/WebFetch and direct raw-file fetches (GitHub raw/API), with `classify-confidence` consulted for tiering (websearch and webfetch: LOW; verified websearch: MEDIUM; verified webfetch: LOW). All sources accessed 2026-11-09.

## Sources

| ID | Source | Notes |
|----|--------|-------|
| S1 | https://raw.githubusercontent.com/mamedev/mame/master/src/mame/mattel/intv.cpp | MAME driver source, BSD-3-Clause header; configs, ROM hashes, ECS/voice/keyboard maps |
| S2 | https://raw.githubusercontent.com/mamedev/mame/master/src/mame/mattel/stic.cpp | GROM as STIC device ROM |
| S3 | https://raw.githubusercontent.com/mamedev/mame/master/src/devices/bus/intv/slot.cpp | Cart slot, PCB types, `.ROM`/hash-file loading |
| S4 | https://github.com/FujiNetWIFI/jzintv (raw: `doc/jzintv/jzintv.txt`, `debugger.txt`, `doc/programming/memory_map.txt`, `doc/tech/{overview,master,ecs}.html`, `doc/rom_fmt/id_tag.txt`, `src/cfg/cfg.c`, `src/demo/demo.h`, `src/serializer/design.txt`, `src/jlp/jlp.c`, `src/locutus/luigi.hpp`, `src/bincfg/bincfg.h`, `examples/*`) | Fork of upstream 2020-08-22 jzIntv, GPL-2.0-or-later; upstream not read directly |
| S5 | https://intellivision.us/jzintv.php | jzIntv summary page (formats, ECS, IntelliVoice, Intellicart) |
| S6 | https://intellivision.us/nostalgia.php | Nostalgia features |
| S7 | https://github.com/libretro/FreeIntv ; https://docs.libretro.com/library/freeintv/ ; `FreeIntv_libretro.info` ; `src/libretro.c`, `src/cart.c` | FreeIntv features, BIOS, formats, open-content 4-Tris |
| S8 | https://github.com/wavemotion-dave/NINTV-DS | Features, BIOS search by CRC32, license text |
| S9 | https://github.com/peterkaminski/pkintvmcp | Alpha MCP debugger/CPU core |
| S10 | https://wiki.intellivision.us/index.php/Hand_Controller ; https://wiki.intellivision.us/index.php/EXEC | Community wiki (other wiki pages require login) |
| S11 | https://arcarc.xmission.com/Web%20Archives/Deathskull%20(May-2006)/games/tech/intvcont.html | Controller pin combinations (single page) |
| S12 | https://intvfunhouse.com/hardware/consoles/ ; https://intvfunhouse.com/faq/intv_faq2.php | Console variant list; FAQ dated/hearsay (LOW) |
| S13 | https://history.blueskyrangers.com/hardware/ ; https://history.blueskyrangers.com/hardware/ecs.html | Blue Sky Rangers hardware index and ECS page |
| S14 | https://en.wikipedia.org/wiki/Intellivision | Title counts, variants, keyboard component history |
| S15 | https://www.pixelatedarcade.com/tech_attributes/overview/not-compatible-with-intellivision-ii | Inty II incompatibility attribute |
| S16 | AtariAge thread results surfaced by WebSearch (MTE-201 tag, LUIGI/LTO Flash docs thread, Inty II lockout and EXEC threads, test-cartridge threads); several returned HTTP 403 on fetch | Forum anecdote; LOW |
| S17 | https://github.com/nanochess/IntyBASIC (`LICENSE`, `ReadMe.txt`, `samples/`, `contrib/`, `intybasic_prologue.asm`) | Compiler license and sample headers |
| S18 | https://github.com/tgtakaoka/libasm (`test/test_asm_cp1600.cpp`, `test_dis_cp1600.cpp`, `LICENSE.md`) | Assembler/disassembler encoding tests |
| S19 | https://intellivisionrevolution.com/intybasic | Homebrew list; no license data |
| S20 | Search results: https://github.com/OpenEmu/OpenEmu/wiki/User-guide:-BIOS-files ; https://github.com/Abdess/retrobios/tree/main/bios/Mattel/Intellivision | Evidence that third parties distribute/point to original images; no clean-room replacement found |
| Inputs | `./.planning/PROJECT.md`, `.planning/bootstrap/PROJECT-SEED.md`, `.planning/bootstrap/EMULATOR-DNA.md` | Dated project inputs; claims about other emulators not treated as hardware truth |

---
*Feature research for: accurate Mattel Intellivision emulator core*
*Researched: 2026-11-09*
