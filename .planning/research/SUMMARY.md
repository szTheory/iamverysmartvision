# Project Research Summary

**Project:** iamverysmartvision (owner-selected working name; not trademark-cleared)
**Domain:** Deterministic, headless, cycle-oriented Mattel Intellivision emulator core (CP-1610 CPU, STIC video, PSG sound, two hand controllers). Greenfield. Narrow C-compatible API. Compatibility claims bound to named profiles and evidence tiers.
**Researched:** 2026-11-09 (all web and local observations dated 2026-11-09)
**Confidence:** MEDIUM overall. STACK, FEATURES, ARCHITECTURE and PITFALLS are each rated MEDIUM overall. Sub-area ratings are carried as written in each file and were not raised.

---

## How to Read This Summary

**Evidence labels.** Labels from the four source files are kept as written. This summary adds no new observations. Where it combines files into a new statement, it is marked `[INF]` (inference from the cited files). Where it proposes a course of action, it is marked `[REC]` (recommendation, not an owner decision).

| Source file | Source observation | Community or emulator | Inference | Recommendation | Unknown |
|---|---|---|---|---|---|
| STACK.md | `[OBS]` (fetched or run 2026-11-09); `[BG]` = background, not re-fetched | cited inline as `[OBS]` | `[INF]` | `[REC]` | `[UNK]` |
| FEATURES.md | `[S#]` (keyed in its Sources table) | `[S#]` | `[I]` | `[R]` | `[U]` |
| ARCHITECTURE.md | `[SRC-PRIMARY]`, `[SRC-HIST]` | `[SRC-COMMUNITY]`, `[SRC-EMULATOR]` (not an oracle) | `[INFERENCE]` | `[REC]` | `[UNKNOWN]` |
| PITFALLS.md | `[SRC S#]` | `[COMM S#]` (community or other emulator) | `[INF]` | `[REC]` | `[UNK]` |

**Decision status is kept separate.** Three categories, never merged: owner decisions (accepted, from PROJECT.md), recommendations (research output, pending an owner or architecture decision), and open questions and unknowns.

### Owner decisions (accepted; PROJECT.md Decision Register)

- Domain is the Mattel Intellivision family.
- Working name `iamverysmartvision`. This is not trademark clearance.
- Goal is accurate, high-quality, useful emulation. The first delivery is sequencing, not a ceiling.
- Routine reversible GSD work runs autonomously (config `mode: yolo`, `auto_advance: true`).
- Bootstrap stops at project initialization. No phase planning or implementation yet.
- Third-party ROMs, manuals and overlays are treated as rights-bearing.
- Release, license, visibility, third-party content, external qualification and hardware testing are explicit owner gates.

### Recommendations (research output; NOT owner decisions; all pending)

- C17 core with CMake 3.25+, Ninja and CTest. `[REC, MEDIUM]`, about 65/35 over Rust. Pending OQ7.
- 2609 NTSC Master Component as the candidate first profile. `[REC]` Revision and region are not chosen. Pending OQ1.
- Early end-to-end slice: one named profile, one original diagnostic, headless runner, installed consumer. `[REC]` (PROJECT.md Recommendations, pending.)
- One original, visible scenario early, so the project does not become research-only. `[REC]` Title not chosen.
- Original, source-built diagnostics in preference to third-party test ROMs. `[REC]` No cleared conformance suite was found.
- Timing model: instruction-stepped CPU with cycle-stamped bus phases and lazy device catch-up ("Option B", ARCHITECTURE section 3). `[REC]`
- Narrow instance API: borrowed-for-load images, typed system images, absolute-target run, caller-owned buffers with preflight refusal. `[REC]` No ABI freeze.
- A project boot stub ("mini-EXEC") used only by CI fixtures and labelled "not an EXEC". `[REC]` Conditions are in conflict C25.
- Real EXEC and GROM only as user-supplied, owner-local inputs. Never in CI, caches or artifacts. `[REC]`
- Fixture manifest with a `redistributable` column. Unknown is treated as no. `[REC]`
- No code, constants or tables copied from jzIntv or MAME. Provenance recorded per module. `[REC]`
- Raw video as palette indices plus profile geometry. Palette and scaling are host policy. `[REC]`

### Open questions (PROJECT.md 1 to 11, with research input)

- **OQ1 First profile and TV timing.** Research points to 2609 NTSC. STIC part, PSG part, EXEC identity and power-on policy still need pinning (C11, C13).
- **OQ2 CPU, STIC, PSG and memory architecture; precision.** Option B timing recommended. STIC and BUSRQ constants are disputed (C1 to C4).
- **OQ3 How system images are supplied.** Research leans to user-supplied only, plus a project boot stub for CI. Policy is open (C25, C27).
- **OQ4 Cartridge formats and maps.** A `.rom` importer is recommended first; `.bin+.cfg` and banking later. Format rights unchecked.
- **OQ5 Controller representation; overlays.** Unresolved conflict between logical and electrical core input (C14). Overlays are host-side metadata only (FEATURES).
- **OQ6 IntelliVoice, ECS, keyboard component, Intellivision II, clones.** Research says later, each as its own profile. Keyboard component and TutorVision are anti-features for now.
- **OQ7 Language, build, ABI, error and versioning policy.** C17 with CMake recommended. ABI not frozen. Use a `struct_size`-first convention if C (ARCHITECTURE 4.1 item 10).
- **OQ8 Debugger and runner shape; save states; persistence.** Trace and inspection are in scope. Save states come in Phase 8. Persistence is profile-defined. No file paths in the core.
- **OQ9 Initial fixture and oracle; first playable.** Original diagnostic with hand-derived literal expectations. "Playable" means an original program with a guest-state predicate.
- **OQ10 First consumer and runtime; LibFuturo adapter.** Installed C consumer. LibFuturo fit is unconfirmed (C14 and the LibFuturo fit in Architecture).
- **OQ11 Dependency policy, new-code license, visibility, release, performance targets.** Owner gates. Research says zero runtime dependencies and no GPL dependency in the core.

---

## Executive Summary

iamverysmartvision is a greenfield, headless emulator core for the Mattel Intellivision family: a CP-1610 CPU, STIC video, AY-3-8914-class PSG sound and two hand controllers. The research points to one shape. A single deterministic machine core runs on an integer machine-time base. The CPU steps by bus phase with cycle-stamped offsets, and devices catch up at access time. A narrow, instance-based C-compatible API treats system images, cartridges, controller samples and output buffers as explicitly owned, refusable inputs and outputs. The existing emulators (jzIntv, MAME, FreeIntv, NINTV-DS) are feature references only. They are GPL or per-file licensed, host-coupled, and not oracles, and none publishes per-profile evidence. The project's distinct value is that each compatibility claim traces to a named profile, a content digest and an evidence tier. It cannot rely on a redistributable conformance suite, because none was found.

The recommended approach is `[REC]`: a C17 core built with CMake and Ninja (OQ7, owner gate), a 2609 NTSC Master Component as the candidate first profile (OQ1, owner gate), user-supplied EXEC and GROM only, and a project boot stub plus original diagnostics for CI (OQ3, owner policy). Build in vertical slices. Slice 0 runs an original CPU diagnostic through the public API, the headless runner and a relocated installed consumer. Slice 1 adds one foreground/background card and one PSG tone, each with literal expectations. A single original visible scenario follows. Most timing constants (STIC access windows, INTRM and BUSRQ durations, frame and scanline geometry, PAL) are disputed across sources. They should be carried as profile data with per-field tiers and named default policies, not frozen as requirements.

The key risks are these. (1) Oracle and provenance contamination from jzIntv or MAME, which is the most tempting shortcut. (2) STIC and bus timing, where some sources differ by roughly 900 cycles on the same quantity and no 2609 measurement exists. (3) PSG identity: most community sources say AY-3-8914, but the tier-1 catalogs list AY-3-8910 and no 8914 datasheet was found. (4) Rights and trademark for EXEC, GROM, MTE-201 and the Intellivision name. (5) An unresolved controller representation (logical 40-bit state versus electrical 16-bit state) that shapes the API before any device code exists. Mitigations are policy-first: an evidence-tier vocabulary, a `redistributable` column in manifests, named-fault controls, inventory gates that count skips, and an explicit conflict log (the table below).

---

## Key Findings

### Recommended Stack (STACK.md)

STACK.md recommends a C17 core built with CMake (floor 3.25, presets v6), Ninja and CTest. The core is one library with a hand-written narrow header, zero runtime dependencies, and integer or fixed-point machine state. Rating: MEDIUM `[REC]`. The C choice is about 65/35 over Rust (cbindgen plus cargo-c). The owner should pick Rust if compile-time memory safety outweighs ecosystem alignment. Zig is rejected as pre-1.0 churn. C++ is rejected for the core because the ABI must be `extern "C"` anyway. Sibling projects on this machine are C-dominant: libfuturo and nesturbator declare C17 in CMake, and gabbaboy uses CMake 3.25 with presets and a sanitizer option. This is evidence of contributor fit and of a transferable method, not a rule `[INF]`. AGENTS.md says to transfer method, not gates or budgets.

- **Core technologies:** C17 (`CMAKE_C_STANDARD 17`, extensions off); CMake 3.25 floor (latest observed 4.4.5); Ninja 1.13.2; CTest for inventories; no runtime dependencies `[REC]`.
- **Test tooling:** exact CTest inventory against a checked-in name list; in-repo minimal C harness (Unity 2.7.0 optional); ASan and UBSan on every test run; libFuzzer only on Homebrew or Linux LLVM, because Apple clang 21 lacks the fuzzer runtime (verified locally `[OBS]`); `cmake -E sha256sum` for golden hashes; in-repo benchmark that asserts the output digest before timing.
- **Installed consumer:** `install(EXPORT)`, `find_package`, relocated prefix, compiles against the public header only `[REC]`.
- **Fixture toolchain (contested, see C22):** CPU vectors as literal decle tables from the GI manual, with no assembler. First guest ROM via a pinned, dev-time-only `as1600` plus a `libasm` cross-check, with bytes committed beside source and manifest `[REC]`.
- **Hardware authority tiers:** GI 1980 and 1982 Microelectronics Data Catalogs and the GI CP-1600 User's Manual (May 1975) are primary. Blue Sky Rangers is a strong secondary. The wiki and jzIntv docs are leads. MAME, jzIntv and FreeIntv code are comparison only. The GI 1977 catalog covers a GIMINI precursor, not the console `[OBS]`.
- **Defects found in bootstrap sources:** the owner-manual PDF URL returns 404; the iFixit "PDF" returns an HTML viewer; the AY-3-8914 datasheet was not found in any GI catalog searched. This is the largest hardware-reference gap `[OBS]`.
- **Config note:** `idiomatic_elixir_lens: true` in `.planning/config.json` does not apply to the core. There is no Elixir host in scope `[INF]`.

### Expected Features (FEATURES.md)

FEATURES.md maps the hardware family, the table stakes, the differentiators and the anti-features. Ratings are MEDIUM unless a finding is marked otherwise.

- **Variant landscape.** 2609 is table stakes and the candidate first profile. Tandyvision One (and possibly GTE MC100) are likely aliases of the 2609 profile, unverified by dump comparison `[U]`. Sears Super Video Arcade has a distinct EXEC per MAME and a jzIntv checksum; later. Intellivision II has EXEC2 (4K plus 256 decles at $0400-$04FF) and an EXEC lockout; later. INTV System III and Super Pro: defer, EXEC identity unresolved `[U]`. PAL and PAL-M: later, as a timing parameter. TutorVision and the Keyboard Component are anti-features near term.
- **Table stakes (TS1 to TS14).** Named baseline profile (TS1). Full CP-1610 core (TS2). STIC modes, VBLANK access windows and bus isolation (TS3). PSG with I/O ports used for controllers (TS4). Declarative memory map with refusal (TS5). One self-describing importer, `.rom` recommended, with typed refusals (TS6). System-image identity with refusal; no bundled EXEC or GROM (TS7). Controller multi-signal electrical model (TS8). Deterministic headless runner (TS9). Raw index framebuffer and raw audio at a machine-time boundary (TS10). Configuration identity in every result (TS11). Typed errors and refusal that leave prior state unchanged (TS12). Original fixtures with manifests (TS13). Trace and inspection (TS14).
- **Differentiators.** Evidence-bound claims with a tier per result. Input record and replay at a machine-time boundary (no surveyed emulator has it; not exhaustive `[U]`). Versioned save states bound to configuration identity. Machine-readable traces and state digests. Open, provenance-tracked conformance diagnostics. Compatibility metadata with refusal. Rights-neutral identification by hash. Installed-consumer proof.
- **Anti-features.** Bundling or auto-downloading EXEC, GROM, ECS, IntelliVoice, Sears or Intellivision II images, MTE-201 or commercial ROMs. A clean-room EXEC replacement for commercial games (none found). High-level EXEC emulation. Silent fallbacks, such as dropping ECS when `ecs.bin` is missing. Compatibility percentages. Overlays, remapping, netplay, rewind or cheats in the core. Frame skipping or wall-clock pacing in the machine. Vendoring GPL code.
- **Controllers.** Four physical side buttons: the top pair is functionally identical, plus lower-left and lower-right. Three distinct electrical codes. This is MEDIUM, from three agreeing sources. Each controller reads as 8 active-low lines. Each input grounds a combination of two pins. Some combinations are not uniquely decodable: keypad with side button or disc, and any two side buttons. Right controller at $01FE and left at $01FF (PSG ports). The full pin table rests on a single archived page, LOW as extracted.
- **Boot path.** Every commercial game and most homebrew rely on EXEC. A CI-visible scenario therefore needs either (a) an original program with a labelled project boot stub, or (b) an owner-local run with a user-supplied EXEC. Route (b) can never run in public CI `[R]`.
- **Conformance.** No redistributable conformance suite was found in a bounded search. This is an evidence limit, not proof of absence. Plan on original diagnostics with literal expectations. Do not claim "passes suite X".
- **Legal content.** Geese: the header says CC0, but the examples README says GPL; upstream not checked. Other jzIntv examples: GPL, reference only. 4-Tris: do not vendor. IntyBASIC compiler: BSD-2 license, but compiled-output notices and samples are unresolved. MTE-201: discovery of diagnostic categories only.

### Architecture Approach (ARCHITECTURE.md)

ARCHITECTURE.md's central finding is that the Intellivision is a coupled video subsystem, not "a CPU plus a video chip on one bus." The STIC has a private 14-bit graphics bus. The 16-bit System RAM (RA-3-9600) bridges the two buses and holds BACKTAB. The STIC takes CPU time through BUSRQ and BUSAK, about 12 row fetches per frame, each reported at 110 to 114 cycles. Bus Copy mode is entered on INTAK `[SRC-COMMUNITY, MEDIUM]`. Recommended model `[REC]` (Option B): the CP-1610 stepper is pure. For each instruction it returns an ordered list of (cycle offset, bus phase, address and data). The scheduler holds an integer CPU-cycle timeline and an event queue (INTRQ, BUSRQ, window close, frame complete). Devices catch up lazily to each access time. Rejected: instruction-only stepping, which is wrong at window edges by up to about 12 cycles, and master-tick lockstep, which implies false precision.

- **Run contract `[REC]`.** Absolute cycle target. Whole instructions. Overshoot is reported. Slicing invariance (run(A)+run(B) equals run(A+B) when targets align) is a test, not an assumption.
- **Components.** CP1610 stepper. Bus and memory map as descriptors (width mask, undriven policy, alias group, blocked-read policy). Scheduler. STIC plus System RAM video subsystem. PSG. Controllers. Loader and validator. Machine shell with profile. Observation layer: raw index video, PSG write log, reference PCM, trace ring, canonical serializer with SHA-256 per domain. Runner, installed consumer and host sit outside the core. The LibFuturo adapter is optional.
- **Proposed layout `[REC]`.** `core/{cpu,bus,sched,video,audio,input,media,machine,observe,api}`; `profiles/` as data with provenance tags; `tools/` (mini-assembler, not shipped in core); `runner/`; `consumer/`; `fixtures/`; `docs/evidence/`.
- **Build order.** Nine steps (0 to 8). Slice 0 is complete at step 3 (CPU-only). Slice 1 is the first pass of step 5b plus minimal 4a.
- **LibFuturo candidate fit (read from header, not executed).** Instance lifecycle fits. Single-span load partly fits; it needs a multi-image API or a bundle envelope. The 32-bit button mask fits the electrical state (16 bits, or 32 with ECS) but cannot hold the 40-bit logical state. The advance call fits, but cycle units need a new time unit. Video (index8) fits. PCM audio fits; the PSG register log has no channel. Snapshot fits via the canonical serializer. Any shared-contract change is an owner gate `[REC]`.
- **Anti-patterns to avoid (ARCHITECTURE section 7).** Treating jzIntv or MAME as the oracle. A joystick-plus-buttons input model. Frame-only STIC without BUSRQ. Per-master-tick lockstep. Host time, host RNG or uninitialized memory in machine state. Derived render caches with hidden state across snapshots. Buffer capacity or call slicing affecting machine behavior.

### Critical Pitfalls (PITFALLS.md)

PITFALLS.md has 29 entries. The six that matter most for the roadmap are below. Each comes with its prevention.

1. **Other emulators as oracle or spec (PIT-1, PIT-4).** MAME's `stic.cpp` marks collision detection TBD and renders per frame; `cp1610.cpp` marks several opcodes TBD and leaves SDBD mode 110 as `???`; jzIntv's author labels PAL numbers as possibly bogus. Prevention: tier vocabulary, hand-written expected values with a citation for each, comparison only as a disagreement finder, provenance recorded per module, no `#include` from jzIntv or MAME.
2. **Same-core goldens and title screen as progress (PIT-2, PIT-16).** Prevention: every golden records its tier and oracle in a header. Named-fault controls must fail. Guest progress is a predicate on guest state, not a screenshot or hash. Goldens change only through a script that prints the diff and requires a tier annotation.
3. **System images as permission, or needed in CI (PIT-3, PIT-23).** Prevention: `redistributable: yes/no/unknown` column (unknown counts as no). CI uses only project-authored fixtures built in the job. EXEC-dependent tests form a named optional job that prints a visible `SKIPPED` line and is counted. Never cache or containerize system images.
4. **CP-1610 interrupt shortcuts (PIT-6, PIT-20, PIT-21).** A single "interrupts enabled" boolean is wrong. INTRM is checked only when I is set and the previous instruction was interruptible. BUSRQ is checked after interruptible instructions regardless of I. Prevention: explicit `prev_interruptible`, `I`, `D` and BUSRQ state. Vectors for each of cases (a) to (d) in PIT-6, with named-fault controls. SDBD modes need a named policy for modes 0 and 6, which are "not supported".
5. **STIC timing constants and per-frame rendering (PIT-7 to PIT-10).** Prevention: every timing field lives in profile data with a tier. Where sources conflict, use the more conservative documented value as a named policy tagged `unknown-default-chosen`, and write a fixture whose expected outcome is "documented and may change". Decide row-granular versus frame-granular rendering early. Separate `peek` from CPU `read` (reads of $0021 switch modes). Define the raw framebuffer boundary explicitly.
6. **Controllers and PSG modeled from the wrong sources (PIT-12, PIT-13).** Do not adopt a generic AY-3-8910 library without a per-rule audit (register order, 2-bit envelope select, period-0 rule, envelope step count, noise LFSR, DAC table). Do not model controllers as joystick plus buttons. Core input must reflect the electrical structure (the representation itself is open, C14).

Also critical: trademark and naming (PIT-5, owner gate); stale or skipped-green gates (PIT-16); the process failure modes of an over-broad first milestone and research-only drift (PIT-18).

---

## Source Conflicts and Unresolved Disagreements

These are not resolved by this summary. Each needs either a source read, a measurement (owner gate), a vector test, or an owner or architecture decision. "Status" follows the label of the source file that carries the claim.

| ID | Topic | What the sources say | Status | Resolve by |
|---|---|---|---|---|
| C1 | STIC register-access window after the VBLANK interrupt | About 2000 cycles: PITFALLS 7 (jzIntv docs S8, S11); FEATURES 1.2 attributes "first ~2000" to jzIntv `memory_map.txt`. About 2900 cycles: PITFALLS 7 (wiki, `[COMM S4]`); ARCHITECTURE 1.2 (wiki VBlank_Period_1 and jzIntv `stic_timings` NTSC_STIC_ACCESSIBLE 2900). jzIntv's documentation and its code constant differ, per these citations. | `[UNK]` | Original diagnostic probing window edges by cycle count; owner-gated measurement |
| C2 | GROM/GRAM access window | About 3780 to 3790: PITFALLS 7, FEATURES 1.2. About 3796: ARCHITECTURE 1.2 (wiki; jzIntv NTSC_GMEM_ACCESSIBLE 3796) | `[UNK]` | As C1 |
| C3 | INTRM (VBLANK interrupt) low duration | 3791 cycles (wiki VBlank_Interrupt, ARCHITECTURE 1.4). 2907 cycles (jzIntv NTSC_INTRQ_HOLD, attributed to Kevin Horton; an alternate 2920 is commented out). | `[UNKNOWN]` | As C1. Affects whether a CPU can "drop" the interrupt |
| C4 | BUSRQ hold and BUSAK deadline | 114 cycles, must halt by about 57 (wiki; PITFALLS 6). 110 normal, 57 first, 44 for the 14th (jzIntv); must-BUSAK 68 (jzIntv). Zbiciak notes: BUSRQ about one CPU cycle shorter in jzIntv experiments; BUSRQ timing listed as unmeasured. | `[UNKNOWN]` | As C1 |
| C5 | Value read while a region is inaccessible | $00FF for GROM/GRAM and random for STIC registers (TutorVision measurement; different board, STIC1A). 0x3FFF & addr and 0x000E & addr (jzIntv). Unmapped reads return $FFFF (jzIntv author; ARCHITECTURE pull-ups). | `[UNK]` for the 2609. Treat as a profile parameter | Profile policy, tagged; diagnostic |
| C6 | Display-enable strobe | $0020 "Active Display Strobe" (wiki STIC; PITFALLS 8: re-enable every frame by a write to $0020). One MAME comment attributes display-enable to $21 (not opened; ARCHITECTURE 1.4). Reads of $0021 switch Color Stack (PITFALLS 8). | `[UNK]` | Use the wiki reading as a tagged default; diagnostic on hardware later |
| C7 | CPU cycles per NTSC frame | 14,934 (wiki, jzIntv; consistent with 262 lines by 57 cycles). "14394" (wiki mention; ARCHITECTURE 1.4 calls it a likely transposition). Cycles vary with display enable and vertical delay: about 13,518 and 13,572 (PITFALLS 7, per jzIntv). | Leaning 14,934 `[INF]`. Variation is a model requirement | Profile field; never a bare constant (PIT-7) |
| C8 | Unit naming for the 894,886 Hz rate | All sources agree the rate is about 894,886 Hz. MAME, jzIntv and Blue Sky call it the CPU clock or machine cycle rate; MAME's CP1610 device clock is XTAL/4. STACK reads the GI datasheet "cycle" as two input-clock periods, implying a CPU input near 1.79 MHz (colorburst/2) `[INF]`. | Naming and input clock `[UNK]` pending schematic. Numeric value agreed | Define "CPU cycle" = crystal/4 `[REC]`; confirm input clock by schematic |
| C9 | Crystal constant | 3.579545 MHz (documents, jzIntv, MAME) versus exact 315/88 MHz colorburst (STACK background `[BG]`). Differs by about 0.00001 percent | Owner or profile choice (STACK; ARCHITECTURE) | Profile decision, explicit |
| C10 | Visible geometry and raw framebuffer | GI 1980 p.5-10 (read via STACK) states 159 x 96 visible positions for the -1 part, and the 20th background column is 7 pixels wide. This is the strongest source and leans to 159. Alternatives: a 160-column nominal grid (Blue Sky; ARCHITECTURE); the STIC1A displays a 160th column (PITFALLS 10); "192v x 160h" (INTV Funhouse, FEATURES). Vertical: 192 active scanlines versus 96 pixel rows of 2 TV lines each. | Leaning 159 visible `[INF]` from tier-1. Raw framebuffer definition `[UNK]` | Profile field: full nominal grid with a "not displayed" marker, so either reading is expressible `[REC]` |
| C11 | PSG and STIC part identity | PSG: AY-3-8914 (Blue Sky; MAME; FEATURES; PITFALLS, `[COMM]`). The GI 1980 catalog lists AY-3-8910 as the "8900" system's sound generator. No "8914" appears in the GI 1977, 1980 or 1982 catalogs (STACK). STIC: AY-3-8900-1 for NTSC (GI 1980, STACK); "AY-3-8900" (FEATURES); STIC1A (PITFALLS, ARCHITECTURE; TutorVision, II and Sears). | `[UNK]` | Primary datasheet search; profile must pin the STIC and PSG parts |
| C12 | PSG behavior details | Tone period 0: 4096 (wiki via a summarizing fetch, `[COMM S6]`) versus treated as 1 by many AY emulators (`[INF]`). Envelope steps: 16 (AY-3-8910) versus 32 (YM2149, used by at least one emulator doc `[COMM S27]`). Noise LFSR unknown. DAC levels unknown; 5- versus 6-bit volume variants differ (ARCHITECTURE 5.1). Envelope select bits 5:4 "unusual for 01b and 10b" (STIC1A notes). PITFALLS 12's register-order list appears to cover 14 of the 16 addresses, and its volume-register numbering does not match the 8912 numbering. Treat that list as unverified. | `[UNK]` | Per-rule vectors from primary PSG documentation |
| C13 | PAL timing | 19,968 = 312 x 64 (ARCHITECTURE 3.4, from jzIntv PAL_SCANLINE; `[INFERENCE]`). About 20,000 cycles, 312 lines and 8,840 to 8,850 (unit not stated in PITFALLS), marked "?? ... BOGUS" by the jzIntv author (PITFALLS 7). "No data" (PITFALLS 7, `[COMM S4]`). STIC clock: AY-3-8900 (not -1) is the CCIR part from 4.000 MHz (STACK, GI 1980). Several jzIntv PAL values are guesses. | `[UNK]`; LOW across all sources | Keep PAL out of the first profile |
| C14 | Controller core-input representation | ARCHITECTURE 4.3 recommends a lossless logical state: 20 bits per controller, 40 for two (12 keypad, 5-bit disc, 3 side). It notes a one-hot form would be 62 bits, and a 32-bit lossy subset would be 24 bits. The core owns the control-to-electrical function. It rejects sending the 8-bit electrical byte to the host. PITFALLS 13 and FEATURES TS8 recommend electrical state per port as core input (2 by 8 bits, 16, or 32 with ECS), with logical mapping on the host producing realizable states. FEATURES says the electrical form fits the 32-bit LibFuturo mask. | Incompatible recommendations on where the logical-to-electrical function lives. OQ5 open | D6 (owner and architecture). Needed before the Phase 4 API skeleton |
| C15 | Side-button count | Four physical buttons, three distinct electrical codes, top pair wired together (FEATURES, MEDIUM; Blue Sky). ARCHITECTURE 4.3 and Blue Sky's 3-bit action code count three. | Largely reconciled `[INF]`: four physical, three electrical | Confirm on schematic |
| C16 | Controller pin matrix | Full 16-direction, 12-key and 3-side pin table from one archived Deathskull page (FEATURES, LOW as extracted; ARCHITECTURE notes an apparent duplicate, D1 and D5 both "pin 3"). The wiki gives only key codes ($7E for key 1; $5A for keys 1 and 9) and combination rules. Blue Sky gives code widths (disc 5-bit, action 3-bit, keypad 8-bit). | `[UNK]` until schematic or measurement | Schematic or service manual; owner-gated measurement |
| C17 | Stack direction and R6 behavior | Wiki: R6 auto-increments on indirect reads and auto-decrements on indirect writes (community). GI manual transcription (jzIntv `cp1600.txt`, OCR-damaged; PITFALLS 19): post-increment on output (push), pre-decrement on input (pull). jzIntv interrupts doc: stack starts at $2F1 and grows upward, agreeing with the manual. ARCHITECTURE read the bitsavers PDF; it does not say whether the stack section was checked. | Conflict between community and manual readings. Manual reading rests on an OCR transcription | Vector test against the primary PDF directly `[REC]` |
| C18 | Instruction count | 86 (GI 1980), 87 (GI 1975), 1024 opcodes (Blue Sky), "79 instructions" (pkIntvMCP, LOW). | Counting convention | Use an opcode-level table as the inventory `[REC]` |
| C19 | Interruptibility of shifts and MVO | Wiki list: HLT, SDBD, EIS, DIS, TCI, CLRC, SETC, shifts and all MVO are non-interruptible. ARCHITECTURE says its spot-checks of the manual agree. PITFALLS 6 asks for verification of shifts and MVO against the manual pages. | Verification status differs; not a numeric disagreement | Vector test per opcode (Phase 2) |
| C20 | EXEC second half and RO-3-9504 | The GI catalog calls RO-3-9504 a "cartridge ROM". Blue Sky and MAME use it as the second EXEC half. MAME's own comment on the EXEC halves is marked unclear ("0000 or 1000, guess 0000"). | `[UNK]` | EXEC image plus board evidence (owner-local) |
| C21 | GRAM extent and STIC decode range | GRAM $3800-$39FF, mirrored at $3A00-$3FFF (FEATURES; ARCHITECTURE). GRAM $3800-$3AFF with shadow to $3FFF (PITFALLS 14); shadow copies from $3B00 (PITFALLS 8). STIC registers $0000-$007F (FEATURES) versus responses at $00-$3F with decoding of $0000-$007F (PITFALLS 8; ARCHITECTURE $0000-$003F). | `[UNK]`; minor | Memory-map decode diagnostic |
| C22 | Fixture assembler | STACK: pinned `as1600` as dev-time-only tool, `libasm` cross-check, defer a project assembler. ARCHITECTURE (Pattern 7, 2.3): project-authored mini-assembler in `tools/`. FEATURES TS13 and PITFALLS 5: either. | Recommendation conflict | D4 (owner, for licence; architecture). Needed before Phase 4 |
| C23 | Reference PCM location | ARCHITECTURE 5.1: integer-only reference PCM decimation inside the core's observation layer. STACK: derive host PCM outside the core. PITFALLS 12: host resampling must not be in the evidence path. All agree raw chip-rate output must be distinct from host PCM. | Recommendation conflict on location, not on principle | D7. Needed before Phase 5 |
| C24 | CI shape | STACK: Linux gcc and clang with sanitizers, fuzz smoke in PR CI if cheap, one macOS job, MSan and TSan scheduled. PITFALLS 18 defers fuzzing and matrix growth until a failure class justifies them. ARCHITECTURE: more than three required jobs is a performance trap. | Process conflict | D12. Decide in Phase 4 |
| C25 | Mini-EXEC / boot stub | ARCHITECTURE Pattern 6: first-class from the first slice. FEATURES route (a): labelled "not an EXEC". PITFALLS 3: do not write an EXEC replacement casually; the contract must be tested; its presence changes timing. STACK: the EXEC-less reset story is an owner or profile question. | Recommendation tension | D5 (owner, OQ3). Conditions in the Phase 4 plan |
| C26 | Visible scenario timing | PROJECT requires an early visible scenario. ARCHITECTURE puts the playable step after Slice 1. | Sequencing, not a factual conflict | Phase 7 may move up after Phase 6's first pass |
| C27 | Mini-EXEC contract details | ARCHITECTURE: reset at $1000, ISR at $1004. PITFALLS 3 (jzIntv interrupts doc): ISR dispatch at $1004 through the vector at $0100/$0101 in double-byte form, and a NOP at $1009 so BUSRQ can be serviced. FEATURES: CPU reset sets R7 to $1000 (LOW). | Consistent on $1000 and $1004. Dispatch details from one source | Boot-contract test against whichever EXEC is configured |
| C28 | jzIntv version and license | Fork: GPL-2.0, README "GPL v2 or later", based on the 2020-08-22 snapshot (STACK). Official page lists 20200712 as current stable. Upstream site unreachable during research (PITFALLS). Geese header (CC0) versus examples README (GPL) (FEATURES). | `[UNK]` | Licence review before any reuse; read-for-concept only |

---

## Implications for Roadmap

The roadmapper should structure phases as below. Ordering follows the four files and the vertical-slice rule. Labels: `[REC]` is a recommendation; `[INF]` is inference from the cited files. Phase names are working labels. Pitfall numbers are PIT-#. Feature IDs are FEATURES TS#. Conflict IDs are the table above.

### Phase 1: Reference, evidence and rights foundations

**Rationale `[REC]`:** Almost every pitfall is a policy failure (PIT-1, 2, 3, 4, 15, 16). Each later claim needs a tier, a profile identity, a provenance statement and a rights status. No code exists yet. Time-box the unknowns here, rather than researching them to completion (PIT-18).

**Delivers:**
- Profile draft for 2609 NTSC as data, with per-field tier tags (source expectation, community, emulator, measured, unknown-default-chosen).
- Evidence-tier vocabulary and claim ledger. A DISCREPANCIES log seeded from C1 to C28.
- Fixture manifest schema with the `redistributable` column, plus notices file.
- Provenance rule: implemented from primary sources only. Never "from jzIntv" or "from MAME."
- Unit-naming decision (C8, D2).
- Primary-source acquisition list: the GI CP-1600 PDF read directly; GI 1980 and 1982 catalogs; an AY-3-8914 datasheet search; the STIC -1 section.
- Owner gates recorded: D1 (language), D3 (first profile), D9 (license, visibility, naming).

**Addresses:** TS1 (profile), TS11 (configuration identity), TS13 (manifest), the evidence-bound differentiator.
**Avoids:** PIT-1, 2, 3, 4, 5, 15, 16, 18, 19, 23, 24.
**Research flag:** NEEDS TARGETED RESEARCH (primary-source acquisition; AY-3-8914 and STIC datasheets; EXEC rights chain). OWNER GATES: D1, D3, D9.

### Phase 2: CP-1610 core and bounded-execution contract (Slice 0, part 1)

**Rationale `[INF]`:** The CPU is a prerequisite for everything. It is also the one area with an independent primary oracle (ARCHITECTURE 1.1).

**Delivers:**
- CP-1610 stepper on a flat-RAM mock bus.
- Opcode-level table as the inventory (C18), not an instruction count.
- Hand-derived vectors per opcode and per flag outcome. Cycle counts with W=0. Interruptibility.
- Explicit `prev_interruptible`, `I`, `D` and BUSRQ state.
- SDBD vectors, with a named policy for modes 0 and 6.
- Stack-direction vector (C17).
- Reset R7 and reset entry as profile fields.
- Named-fault controls for each of the interrupt cases (a) to (d) in PIT-6.
- Cycle-stamped bus-phase output shape (Option B), with a flat bus behind it.
- Absolute-target run with overshoot reporting. Two-instance isolation test.

**Addresses:** TS2; part of TS12; TS14 (trace hooks).
**Avoids:** PIT-2 (named-fault controls), PIT-6, 17, 19, 20, 21, 25.
**Uses:** C17 and CMake presets; CTest exact inventory; ASan and UBSan; strict warnings (STACK).
**Research flag:** NEEDS TARGETED RESEARCH. Read the bitsavers CP-1600 PDF directly, not the OCR transcription, for C17, the branch taken and not-taken cycle column, SDBD modes, interruptibility (C19), and the reset and MSYNC sequence (STACK gap). Use libasm test vectors only as leads, after a licence and provenance check.

### Phase 3: Memory map, system images, cartridge loading and refusal

**Rationale `[INF]`:** The CPU needs a bus. The loader fixes system-image identity and refusal behavior before the runner depends on them (ARCHITECTURE build step 2).

**Delivers:**
- Declarative map descriptors: range, device, width mask, blocked-read policy, alias group, access gate. The decode table is dumped and compared with the sourced literal.
- 10-bit decle masking and store-truncation rules, tested.
- Typed images: EXEC (4K), GROM (2K), cartridge segments. Each with length and digest.
- Native multi-image load (ARCHITECTURE option a `[REC]`), or a bundle envelope for the adapter.
- Refusal matrix: empty, odd length, truncated segment, overlapping segments, segment crossing a class boundary, ROM claiming STIC or I/O ranges, wrong header, oversized, missing system image, unknown profile. A failed load leaves prior state unchanged.
- One importer (`.rom` recommended), defined in the project's own words.

**Addresses:** TS5, TS6, TS7, TS12.
**Avoids:** PIT-3, 14, 15, 23. Silent default memory maps.
**Research flag:** NEEDS RESEARCH for cartridge formats (OQ4). The `.rom` and `.bin+.cfg` format rights are unchecked, and upper-bit read behavior is `[UNK]`. STANDARD PATTERNS for the refusal matrix and descriptor tables once the format is chosen.

### Phase 4: Machine shell, scheduler, headless runner and installed consumer (Slice 0)

**Rationale `[INF]`:** This produces the first runnable end-to-end artifact. It proves the public boundary and independent packaging early. PIT-18 treats no runnable artifact after two phases as a warning sign.

**Delivers:**
- Machine shell wired from the profile.
- Integer CPU-cycle timeline and event queue, with only RAM and ROM devices.
- Run loop with instruction-boundary BUSRQ and INTRM checks.
- Observation of cycle and frame counters, stop reason, and per-domain state hash. Slicing-invariance test.
- Headless runner: scripts, literal expectations, failure taxonomy (harness, guest, timeout, cancellation, missing fixture).
- Exact inventory gate that reports executed, skipped, missing and expired counts.
- Project boot stub (reset at $1000, ISR at $1004) plus one original CPU diagnostic. Labelled "project boot stub, not an EXEC" (D5).
- Installed consumer in a relocated prefix. It exercises refusal, run, and state hash.
- Fixture-assembler decision (D4) implemented here.

**Addresses:** TS9, TS11, TS13. The installed-consumer differentiator.
**Avoids:** PIT-16 (skip-as-green), 17, 23; PIT-3 (no system image in CI); PIT-18 (CI bloat).
**Research flag:** STANDARD PATTERNS for CMake `install(EXPORT)` and the consumer (STACK; gabbaboy precedent). DECISIONS needed: boot-stub contract and labelling (C25, C27); CI shape (C24). NEEDS RESEARCH: the EXEC-less reset story (STACK gap; OQ3).

### Phase 5: PSG and hand controllers

**Rationale `[INF]`:** Controller reads are PSG I/O ports (R14 and R15), so input and sound are coupled (TS8 before the TS4 host story). The PSG register-write log is the raw-audio observation. ARCHITECTURE steps 4a and 4b.

**Delivers:**
- PSG register file keyed to the verified 8914 map. Use no 8910 names without a label.
- Tone, noise and envelope rule vectors. Period-0 policy, 2-bit envelope select, LFSR and DAC as profile data, each with a tier.
- Cycle-stamped register-write log and integer chip-rate samples. Reference PCM location per D7 (C23).
- Controller model per D6 (C14). Side-button and disc combinations, both players.
- R14 and R15 read path. R7 direction-bit handling.
- Named ambiguous combinations with expected AND-merged results.

**Addresses:** TS4, TS8. TS10 (audio part).
**Avoids:** PIT-12, 13, 17 (no float in the core), 27 (keep ECS ports out).
**Research flag:** NEEDS RESEARCH. AY-3-8914 register map and behavior (no tier-1 datasheet; GI catalogs list AY-3-8910). Controller electrical matrix needs a schematic or measurement (C16). OWNER: D6 (OQ5).

### Phase 6: STIC video: control path, windows and raw framebuffer (Slice 1, first pass)

**Rationale `[INF]`:** This is the highest-uncertainty area and the place where the CPU is most visibly coupled to the video subsystem (ARCHITECTURE 1.2, 3.2). Slice 1 requires one foreground/background card on screen with literal indices. For PIT-18 (one chip per phase), split into 6a (control path and timing) and 6b (render features). Both are STIC.

**Delivers:**
- *6a:* STIC register file and display-enable handshake (strobe reading per C6, tagged as a default). VBLANK access windows with a blocked-read profile policy. INTRQ, BUSRQ and BUSAK events with profile constants. Where sources conflict (C1 to C4), use the conservative documented value tagged `unknown-default-chosen`. Per-row BACKTAB latch, or a documented frame-granular approximation (PIT-9). System RAM bus-copy gate. Separate `peek` from CPU `read` (PIT-8).
- *6b:* Foreground/background first, then Color Stack, border and delay, MOBs, collisions, colored squares. One feature per fixture. Literal index frames derived by hand from BACKTAB and GRAM contents. Raw index frame at a defined boundary, with geometry from the profile (D8, C10).

**Addresses:** TS3, TS10 (video).
**Avoids:** PIT-7, 8, 9, 10, 11, 26; PIT-1 (no same-core golden).
**Research flag:** NEEDS RESEARCH, the strongest in the set. The STIC primary section (GI 1980 p.5-10; 1982 p.9-43) is not extracted in detail. Conflicts C1 to C7, C10 and C21 apply. Collision semantics (MAME marks them TBD; PIT-11) need a datasheet or measurement. STIC part identity (C11). OWNER GATE: hardware measurement (D10).

### Phase 7: First visible original scenario (and owner-local route)

**Rationale `[REC]`:** PROJECT requires one visible playable or homebrew scenario early. FEATURES finds that the only CI-safe route is an original program with a project boot stub. This phase can be scheduled right after Phase 6's first pass, rather than after all of Phase 6.

**Delivers:**
- Original program. Assembled from source in CI, or committed with a manifest.
- Guest-state predicate (a register or memory marker) as the success condition. Not a title hash.
- Installed consumer runs the scenario.
- Optional visible host. SDL3 is a sibling-project pin but was not researched; keep any host out of the core build graph (STACK).
- Route (b): an owner-local real EXEC and GROM run, documented as non-redistributable and outside CI (D11).

**Addresses:** the visible-scenario requirement. The "playable guest progress" tier, for this original program only.
**Avoids:** PIT-2 (title screen as progress); PIT-3; PIT-5 (no "Intellivision" in a package or product name).
**Research flag:** OWNER GATE for scenario choice and rights (D9). NEEDS RESEARCH only for any third-party homebrew. Geese licence conflict (C28); 4-Tris not to be vendored; IntyBASIC output unresolved. Host choice `[UNK]`.

### Phase 8: State, trace, digests and performance baselines

**Rationale `[INF]`:** The save-state layout depends on stable component state. Performance work waits for correctness baselines (PROJECT North Star; ARCHITECTURE build step 7; PIT-18).

**Delivers:**
- Canonical serializer, endianness-independent, used as both the snapshot format and the hash source.
- Header with format version, profile id and system-image digests. Restore validates before mutating.
- Hidden-state checklist from PIT-17: `prev_interruptible`, `D`, `I`, BUSRQ and BUSAK, bus-copy mode, FIFO contents, sticky collision bits, color-stack pointer, display-enable handshake, PSG divider, envelope and noise state, controller latch, RAM policy.
- Snapshot-restore-run equivalence at mid-frame, mid-SDBD and mid-BUSRQ.
- Side-effect-free inspection API (PIT-25).
- Optional input record and replay at a machine-time boundary (differentiator).
- Named deterministic workloads. A benchmark runner that asserts the output digest before it reports speed.
- Persistence policy (OQ8), profile-defined. No file paths in the core.

**Addresses:** TS14 (full); save-state, input-record and digest differentiators.
**Avoids:** PIT-17, 25, 28; ARCHITECTURE anti-pattern 6 (derived caches).
**Research flag:** STANDARD PATTERNS for canonical serialization and hashing. The LibFuturo snapshot is a precedent. The jzIntv serializer doc is for concept only. Completeness of hidden state is `[INF]` and must be verified by tests. OWNER: persistence (OQ8).

### Phase 9: Variant profiles and later peripherals

**Rationale `[REC]`:** Each variant needs its own profile and its own evidence. None is needed for v1, per PROJECT's Out of Scope.

**Delivers, each as a separate profile with its own research:**
- Intellivision II: EXEC2 with 4K plus 256 decles at $0400-$04FF; lockout; timing claim `[U]`.
- Sears: distinct EXEC identity. Alias or profile.
- Tandyvision One and GTE MC100: aliases only after dump comparison `[U]`.
- PAL and PAL-M: timing as a profile parameter, with `unknown-default-chosen` fields (C13). LOW confidence in all sources.
- `.bin+.cfg`, Mattel-style banking, Intellicart and CC3.
- ECS as the first peripheral: PSG #2, banked ROM, four controllers, keyboard.
- IntelliVoice: SP0256, separate audio stream, voice ROM user-supplied.
- JLP and LTO flash persistence, late.
- Keyboard Component, TutorVision and the FujiNet mailbox remain anti-features.

**Addresses:** TS1 (variants); FEATURES later slices.
**Avoids:** PIT-15, 22, 27.
**Research flag:** NEEDS RESEARCH for each variant, separately.

### Phase ordering rationale

- **Policy before code.** Phase 1 precedes code because most pitfalls are policy failures (PIT-1, 2, 3, 15, 16).
- **CPU before devices.** The CPU stepper is the only component with a primary oracle. The bus and scheduler for Slice 0 depend on it.
- **Memory and loading before the runner.** A refusal is a claim. The runner needs refusal behavior and system-image identity to mean anything.
- **Runner and installed consumer before devices.** This gives an independently verifiable artifact at Slice 0, and proves the packaging early (PIT-18).
- **PSG and controllers before STIC rendering.** Controller reads are PSG ports (FEATURES dependency). PSG is the lower-uncertainty chip. STIC carries the most disputed timing and goes after its control path is testable.
- **Visible scenario after first STIC and PSG features.** Slice 1 needs a card and a tone. Phase 7 may move up (C26).
- **State, performance and variants last.** The state layout depends on the finished component set. Performance needs baselines.

**Keep out of the first milestone `[REC]`:** Intellivision II, Sears, ECS, IntelliVoice, PAL, Keyboard Component, TutorVision, FujiNet, the LibFuturo adapter, any host frontend, real-EXEC runs in CI, and scheduled fuzz or sanitizer jobs until a failure class justifies them (PIT-18).

### Research flags, consolidated

- **Needs targeted research (before or during planning):** Phase 2 (primary CP-1610 PDF read), Phase 3 (cartridge formats), Phase 5 (AY-3-8914 and controller electrical matrix), Phase 6 (STIC primary section; the strongest), Phase 9 (each variant).
- **Owner gates:** Phase 1 (D1, D3, D9); Phase 4 (boot-stub policy, D5); Phase 6 (hardware measurement, D10); Phase 7 (scenario and rights, D11).
- **Standard patterns (skip research-phase):** Phase 4 (CMake install and consumer; runner); Phase 3 refusal matrix once the format is chosen; Phase 8 (canonical serialization and hashing).

### Decisions required before named phases

| # | Decision | Status | Owner | Needed before |
|---|---|---|---|---|
| D1 | Implementation language and build (C17 with CMake recommended; Rust alternative) | Recommendation (OQ7) | Owner | Phase 2 |
| D2 | "CPU cycle" unit and time base | Recommendation with open conflict (C8) | Owner and architecture | Phase 2 |
| D3 | First profile: 2609 NTSC Master Component; revision; STIC part; PSG part; power-on policy | Recommendation (OQ1) | Owner | Phase 1 exit; Phase 3 |
| D4 | Fixture assembler policy (`as1600` dev-only versus project mini-assembler) | Recommendation conflict (C22) | Owner (licence) and architecture | Phase 4 |
| D5 | System-image policy: user-supplied EXEC and GROM only; project boot stub for CI; labelling | Recommendation (OQ3; C25, C27) | Owner | Phase 3 (refusal); Phase 4 (boot) |
| D6 | Controller core-input representation (logical 40-bit versus electrical 16-bit per port) | Open conflict (C14; OQ5) | Owner and architecture | Phase 4 (API skeleton); Phase 5 |
| D7 | Reference PCM location (core or host) | Recommendation conflict (C23) | Architecture | Phase 5 |
| D8 | Raw framebuffer geometry | Open (C10) | Architecture; hardware evidence | Phase 6 |
| D9 | Project license, new-code license, repository visibility, naming and trademark | Owner gates (OQ11; PROJECT Out of Scope) | Owner | Before any external publication; `redistributable` flags depend on it |
| D10 | Hardware measurement and physical testing | Owner gate | Owner | Before any measured timing claim (Phase 6 onward) |
| D11 | Owner-local real EXEC and GROM runs | Owner gate (rights; OQ3) | Owner | Phase 7, route (b) |
| D12 | CI shape: job count, sanitizers, fuzz schedule | Recommendation conflict (C24) | Architecture | Phase 4 |
| D13 | LibFuturo adapter and any shared-contract change | Owner gate (OQ10) | Owner | Not before a real consumer exists |

**Synthesis recommendations on the open conflicts `[REC]`, pending the decisions above.** These are this summary's proposals, not positions taken by any source file.

- **D6 (C14).** Keep the electrical function core-side, as both ARCHITECTURE and PITFALLS agree that the core must own electrical semantics. Use lossless per-controller logical state as the native input (ARCHITECTURE). Expose the 16-bit electrical state as an observation, which is also the candidate input for any LibFuturo adapter (FEATURES, PITFALLS). Document the core's policy for unrealizable combinations (refuse, or AND-merge) and test it.
- **D4 (C22).** Start with STACK's approach. Keep CPU vectors as literal tables with no assembler. Use the pinned, non-shipped `as1600` plus a `libasm` cross-check for the first guest ROM only, subject to the licence decision. Defer a project assembler until the fixture count or licence review requires one. Keep ARCHITECTURE's requirement that any encoder is tested against the manual's opcode tables.
- **D7 (C23).** Core emits the register-write log and integer chip-rate samples (agreed by all four files). Decimation to reference PCM is a separately tested reference conversion, placed in the core only when the runner or adapter needs it.
- **D12 (C24).** Start with one Linux job (ASan and UBSan) plus the installed consumer. Add macOS when packaging on macOS must be proven. Keep fuzzing scheduled, not required, until a failure class justifies it.
- **D5 (C25).** Adopt a project boot stub for CI fixtures only, labelled "not an EXEC", with boot-contract tests. Claim no real-EXEC evidence from it.
- **C10.** Store the full nominal grid with a documented "not displayed" marker, so either reading is expressible. Make the visible width a profile field tagged `unknown-default-chosen`.

---

## Confidence Assessment

| Area | Confidence | Notes (as rated in the source file; not upgraded here) |
|---|---|---|
| Stack | MEDIUM | Tool versions HIGH (registry and API observations, 2026-11-09). Language recommendation MEDIUM (about 65/35). Hardware-source authority MEDIUM. AY-3-8914 and EXEC or memory-map details LOW or unknown. |
| Features | MEDIUM | Hardware family facts MEDIUM (cross-checked). Controller electrical facts MEDIUM-LOW (single-source pin table). Emulator feature matrix MEDIUM ("not found" means not found in what was read). Legal content MEDIUM. Conformance-suite absence MEDIUM (bounded search). |
| Architecture | MEDIUM | HIGH for component decomposition and API-shape reasoning. MEDIUM for CPU timing (primary manual read). LOW to MEDIUM for STIC and bus-steal constants. |
| Pitfalls | MEDIUM | HIGH for process, rights structure, and CP-1610 interrupt and SDBD rules, as rated in PITFALLS. Caveat `[INF]`: PIT-19 and PIT-20 cite an OCR-damaged jzIntv transcription, so those specific items need a primary read. MEDIUM for STIC and PSG. LOW for PAL, Intellivision II and Sears, and EXEC or GROM legal ownership. |

**Overall confidence: MEDIUM.** The rated HIGH items (CPU instruction tables, tool versions, process and rights structure) are real, but they do not cover the timing, PSG or controller questions that gate the hardest phases.

### Gaps to Address

- **AY-3-8914 register map and behavior** (C11, C12). No tier-1 datasheet found. Handle in Phase 5 with a primary datasheet search. If none is found, use 8910 documentation with an explicit label and a per-rule audit.
- **STIC (AY-3-8900-1) register-level behavior:** collisions, MOB priority, vertical delay, blanking, bus arbitration. The GI 1980 p.5-10 and 1982 p.9-43 sections are not extracted in detail. Handle in Phase 6 by reading them directly.
- **Reset vector and power-on or MCLR sequence** (GI 1975 manual near MSYNC). Read the primary PDF in Phase 2.
- **The mechanism by which the console supplies $1004 during the interrupt acknowledge** (ARCHITECTURE 1.1, `[UNKNOWN]`). Handle in Phase 2 with a vector test and a profile constant.
- **EXEC and GROM mapping, the second EXEC half, and what EXEC leaves in RAM** (C20). Needs a user-supplied image and board evidence. Owner-local only (Phase 7 route b).
- **Controller electrical matrix** (C16). Needs a schematic, service manual or measurement. Owner gate (D10).
- **Visible geometry and the raw framebuffer size** (C10). Profile decision (D8).
- **Hardware measurement of window, BUSRQ and INTRM constants** (C1 to C4). Physical console; owner gate (D10).
- **Power-on RAM contents and blocked-read values** (C5). Use a profile-specified policy, tagged, and measure if the owner allows.
- **Cartridge formats and the upper bits of 10-bit ROM reads** (OQ4). The `.rom` and `.bin+.cfg` rights are unchecked. Licence review before reading any GPL implementation.
- **Rights chain for EXEC, GROM, MTE-201 and manuals.** No authoritative statement was found. Owner gate.
- **Trademark.** The brand transfer scope was reported by secondary sources only. The USPTO assignment claim is forum-only. Verify before any external publication (owner gate D9).
- **Licence details:** jzIntv upstream license and version (C28); Geese (C28); IntyBASIC compiled-output notices; `libasm` CP1600 test-vector provenance and whether its Apache-2.0 license covers the test files. Review before any reuse.
- **Intellivision II and Sears specifics.** Forum claims only (`[COMM S25]`; LOW). Variant research in Phase 9.
- **PAL timing.** Unknown in all sources (C13). Keep out of profile 1.
- **LibFuturo 32-bit mask semantics** for logical versus electrical controller state. Header read but not executed; owner gate (D13).
- **Bootstrap defects.** The owner-manual URL returns 404. The iFixit service manual is an HTML viewer, and the MTE-201 and "90-minute burn-in" claims in EMULATOR-DNA are unverified. GI 1977 is a precursor, not a console authority. Correct the bootstrap source list.
- **Whether MAME or jzIntv have headless or input-recording modes** in current builds (FEATURES, `[U]`). Not needed for v1.

**Ready for roadmap: yes, with decisions D1 to D6 flagged as needed before the named phases.** D1, D3, D5 and D9 are owner gates and must stay visible. D6 (controller representation) and D4 (assembler) must be settled before Phase 4 begins.

---

## Sources

Tiers follow the source files. Labels are as they appear in each file. Nothing here is upgraded.

### Primary (manufacturer documents; third-party-hosted scans, reuse rights not assessed)
- General Instrument, *CP-1600 Microprocessor User's Manual*, May 1975. `https://www.bitsavers.org/components/gi/CP1600/CP-1600_Microprocessor_Users_Manual_May75.pdf`. Read directly by STACK and ARCHITECTURE (`[SRC-PRIMARY]`). PITFALLS cites only the jzIntv OCR transcription.
- General Instrument, *Microelectronics Data Catalogs* 1980 and 1982. Bitsavers `_dataBooks`. Read by STACK: 1980 pp.5-10 to 5-23; 1982 pp.9-22 to 9-51. Not the GI 1977 catalog, which describes GIMINI.
- General Instrument, *GIC1600 Microcomputer Users Manuals*, Sep 1975. Series documentation; not read in depth.

### Historical in-house
- Blue Sky Rangers, Master Component 2609 technical overview (derived from a 1980 Mattel in-house document, "with some corrections"). `https://history.blueskyrangers.com/hardware/2609.html`. Rated MEDIUM-HIGH (ARCHITECTURE) and "strong secondary" (STACK).

### Community (leads and cross-checks; not oracles)
- intellivision.us wiki: CP1610, BUSRQ, Bus_Copy, System_RAM, Non-interruptible_Instructions, Memory_Map, STIC, PSG, Hand_Controller (oldid=14808), VBlank_Interrupt, VBlank_Period_1 and 2, EXEC. The ROM-format page requires login.
- Zbiciak, *TutorVision / INTV88 Reverse Engineering Notes* (2017 to 2018). Measurements on a different board (STIC1A).
- Deathskull controller pinout (archive mirror). LOW as extracted (apparent duplicate row).
- Wikipedia (Intellivision); INTV Funhouse variant list and FAQ (hearsay, LOW).
- AtariAge forum threads. LOW; several returned HTTP 403.

### Emulator source and docs (observation only; never an oracle)
- MAME `intv.cpp`, `stic.cpp`, `cp1610.cpp`, `bus/intv/slot.cpp`, `ay8910.cpp`. Per-file BSD-3-Clause headers. `cp1610.cpp` acknowledges a GPL'd emulator by Joe Zbiciak (PIT-4).
- jzIntv (FujiNet fork based on the 2020-08-22 snapshot; GPL-2.0 or later): `doc/programming/{interrupts,stic,psg,memory_map,cp1600}`, `doc/rom_fmt/id_tag.txt`, `src/cfg/cfg.c`, `src/stic/stic_timings.h`, `examples/`. Licence review required before any reading for implementation.
- FreeIntv (GPLv2+); NINTV-DS; Nostalgia; pkIntvMCP (alpha; no licence shown).
- Furnace AY-3-8910 notes (emulator documentation, not an 8914 source).

### Rights, brand and legal
- Atari acquisition of the Intellivision brand and a select catalog (2024-05-23); corroborated by press (secondary). USPTO assignment claim is forum-only (LOW).
- Plaion and Atari, Intellivision Sprint announcement (October 2025). Press.
- IntyBASIC `LICENSE` (BSD-2-Clause for the compiler); sample and contrib headers carry no per-file license text.
- AtariAge ROM-legality statement (2003-era forum quote). LOW.
- jzIntv Geese example: CC0 header versus examples README (GPL).

### Tools and libraries (versions observed 2026-11-09; registry and API)
- CMake 4.4.5 (local 4.4.3); Ninja 1.13.2; LLVM 23.1.5 upstream (Homebrew `llvm@21`, `llvm@18` local); Apple clang 21.0.0 local.
- Unity 2.7.0 (optional); Catch2 3.16.1; GoogleTest 1.18.0; Google Benchmark 1.10.0 (C++ only; not for the core).
- `libasm` 2.0.4 (Apache-2.0 per `LICENSE.md`; GitHub reports NOASSERTION). `as1600` (GPL-2.0 or later code; dev-time tool only).
- hyperfine 2.0.0; REUSE 6.3.0. Rust 1.99.1 (alternative). Zig 0.17.1 (from ziglang.org index, not GitHub releases). Meson 1.12.1.

### Local observations (read-only; 2026-11-09)
- Sibling CMake files and file-extension counts: `../{libfuturo,nesturbator,gabbaboy,genewhip,schwaystation}` (STACK).
- LibFuturo header and contract: `../libfuturo/include/libfuturo/libfuturo.h`, `docs/contract.md`, `docs/muhtari-profile.md`. Read, not executed (ARCHITECTURE 4.2).
- Local checks: `clang -fsanitize=fuzzer` link failure on Apple clang 21; ASan and UBSan compile; `cmake -E sha256sum` works on CMake 4.4.3.

### Project inputs (dated)
- `./.planning/PROJECT.md`, `./AGENTS.md`, and the `.planning/bootstrap/` pack (START-HERE, PROJECT-SEED, EMULATOR-DNA). Dated 2026-11-09.

---
*Research completed: 2026-11-09*
*Ready for roadmap: yes, with decisions D1 to D6 flagged; owner gates D1, D3, D5, D9 remain open.*