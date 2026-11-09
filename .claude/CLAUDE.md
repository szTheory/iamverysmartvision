# iamverysmartvision — Claude project guide

Read the root `AGENTS.md` first: it is the owner's durable policy and takes precedence over
generated sections below. Project-owned GSD records (`.planning/PROJECT.md`,
`REQUIREMENTS.md`, `ROADMAP.md`, `STATE.md`) hold current scope and the Decision
Register. Research text below is dated input (2026-11-09): recommendations in it
(e.g. C17 + CMake, 2609 NTSC) are not owner decisions.

<!-- GSD:project-start source:PROJECT.md -->

## Project

**iamverysmartvision**

iamverysmartvision (owner-selected working name; not trademark-cleared) is a new
emulator for the Mattel Intellivision family. It aims to be accurate,
reproducible and useful: a single canonical, deterministic, headless machine
core whose CPU/device behavior, controller input, guest progress, raw video and
raw audio can each be tested independently, and which well-designed hosts can
integrate. It is for the owner and for future host/frontend integrators; support
claims are always tied to an exactly named hardware/content profile and the
evidence that backs it.

**Core Value:** A deterministic, observable machine core whose every compatibility claim is
bound to an exactly named profile and independent evidence — never an
unqualified "it runs Intellivision games" label.

### Constraints

- **Determinism / host independence:** machine behavior must not depend on
  host clocks, filesystems, physical controller discovery, windows, A/V
  presentation or persistence policy — owner North Star.
- **Rights:** binaries, diagnostic ROMs, EXEC/GROM images, manuals and overlays
  are rights-bearing; nothing committed or redistributed without documented
  permission. A hash establishes identity, not rights — AGENTS.md.
- **Dependencies:** small dependency surface; add a dependency only for a
  concrete capability whose maintenance cost is justified — AGENTS.md.
- **Performance:** high runtime performance is a goal, but only after
  representative workloads and correctness baselines exist — North Star.
- **CI:** one local command aligned with the fast PR gate; a CI check is added
  only for a distinct recurring failure class at useful cost; measure
  queue/setup/build/test before changing caches/concurrency/matrix — AGENTS.md.
- **Milestones vs versions:** GSD milestones and software versions are separate.
  Release only the exact tested artifact when rights, package readiness,
  qualification and authority permit — AGENTS.md.
- **Autonomy boundary:** routine reversible work proceeds autonomously; stop for
  material owner choices, unresolved rights, external publication, physical
  qualification or another explicit safety boundary — AGENTS.md.
<!-- GSD:project-end -->

<!-- GSD:stack-start source:research/STACK.md -->

## Technology Stack

## How to read this file

| Label | Meaning |
|-------|---------|
| **[OBS]** | Source observation. I fetched or ran it on 2026-11-09 (URL or command given). |
| **[INF]** | Inference drawn by me from observations. Could be wrong. |
| **[REC]** | Recommendation. The owner decides. |
| **[UNK]** | Unknown or unresolved. Needs a primary source, a measurement or an owner choice. |
| **[BG]** | Background knowledge I did not re-fetch this session. Verify before relying on it. |

## Executive recommendation (one screen)

## Part 1 — Implementation language and build system

### Constraints that decide this (from PROJECT.md / AGENTS.md)

- Narrow C-compatible API with explicit ownership, bounded execution, multiple instances, refusal behaviour.
- Determinism: no host clock, filesystem or float-dependent behaviour in the machine.
- Small dependency tree. Add a dependency only for a concrete capability.
- Independent installed-consumer proof of the packaged artifact.
- Exact test inventories and named-fault controls; fuzzing and sanitizers.
- One local command aligned with the fast PR gate; measured, cheap CI.
- Candidate LibFuturo integration, whose API is C.

### Local ecosystem observation [OBS]

- `libfuturo`, `nesturbator`, `gabbaboy`, `genewhip` and `schwaystation` are C-dominant. The CMake files I read (`libfuturo`, `nesturbator`) declare `LANGUAGES C` and `CMAKE_C_STANDARD 17`.
- `gabbaboy` uses `cmake_minimum_required(VERSION 3.25)`, CMake presets v6 with Ninja, and a `GABBABOY_ENABLE_SANITIZERS` option for ASan+UBSan.
- `gabbaboy` has `install(EXPORT ...)` plus `cmake/RunInstalledConsumer.cmake`, `ExpectedTests.cmake` and a "relocated prefix" preset.
- `libfuturo` has `tests/installed-consumer`.
- `chip8go` is Go and `scoria` is Elixir. These are not emulator cores.

### Comparison matrix

| Criterion | C17 + CMake | Rust + cargo (C ABI) | Zig 0.17.x | C++20 + CMake |
|-----------|-------------|----------------------|------------|----------------|
| Determinism by construction | 0 (UB hazards: signed overflow, shifts, uninitialised reads; 16-bit math under `int` promotion) | ++ (safe code has no UB; explicit `wrapping_*`; but avoid `HashMap` iteration order and floats) | + (explicit overflow ops; undefined behaviour still exists in places) | 0 (same UB as C, plus static-init order, exceptions) |
| C ABI export | ++ (the source is the ABI; hand-written header) | + (`extern "C"` plus cbindgen header; panics must not cross, since 1.81 an unwinding `extern "C"` aborts) | ++ (`export fn`, can emit header) | + (must wrap everything in `extern "C"`; no exceptions across the boundary) |
| Packaging / installed-consumer test | ++ (`install(EXPORT)` + `find_package` + relocated prefix is the native CMake idiom) | + (cargo-c `cinstall` makes `.pc`, header, lib; then compile a C consumer; more moving parts) | 0 (`zig build install`; no standard consumer-discovery convention) | ++ (same as C) |
| Dependency tree | ++ (compiler + cmake + ninja; zero runtime deps) | + (zero-dep core is easy; the dev/test crate tree is large; needs `cargo-deny` to police) | + (zero deps; but compiler itself churns) | + |
| Test tooling | + (CTest, tiny harness or Unity) | ++ (built-in `cargo test`, nextest, proptest, insta) | + (`zig build test`) | ++ (Catch2, GoogleTest) |
| Fuzzing | + (libFuzzer/AFL++ on clang/gcc; not on Apple clang) | ++ (`cargo-fuzz` 0.13.2, `arbitrary`) | 0 (built-in fuzzing exists, maturity unverified) | + |
| Sanitizers | ++ (ASan/UBSan/MSan/TSan on clang/gcc) | 0 (ASan/UBSan/MSan/TSan need nightly `-Zsanitizer`; Miri is nightly) | + | ++ |
| Cross-platform CI cost | ++ (cheapest; all three OSes have a native C toolchain) | + (rustup is easy; compile times higher) | 0 (single toolchain download, but version-pin churn) | + |
| Contributor accessibility | ++ (emulator community lingua franca; jzIntv, siblings) | + (growing; steeper for casual contributors) | -- (small pool; pre-1.0 language) | + |
| Fit with local sibling projects / LibFuturo | ++ | 0 (links via C ABI only) | -- | + |
| Language stability | ++ | ++ (stable channel, 6-week cadence, edition 2024) | -- (pre-1.0; see below) | + |

### Recommendation [REC, MEDIUM confidence, about 65/35 C over Rust]

- Integer-promotion and UB bugs are the dominant risk for a 16-bit CPU with 10-bit ROM words. Mitigation: fixed-width types everywhere, explicit masks after every ALU result, `-Wall -Wextra -Wconversion -Wshadow -Wstrict-prototypes`, and ASan+UBSan on every test run. Consider `-fsanitize=integer` on clang for the CPU module.
- No compile-time memory safety. Mitigation: no dynamic allocation inside the machine step loop (caller-provided or init-time allocation only), bounded execution, MSan on Linux/clang, fuzzers on every parser/loader.
- Floating point: keep the core integer/fixed-point. If any float appears, add `-ffp-contract=off` and test across compilers.

### Assessment of `idiomatic_elixir_lens` [INF]

- [OBS] `.planning/config.json` has `preferences.idiomatic_elixir_lens: true`; the task states it is inherited from global defaults. PROJECT.md and AGENTS.md do not mention Elixir, BEAM or any Elixir host.
- [OBS] The Elixir project in the owner's portfolio (`scoria`) is not an emulator; the emulator cores are C.
- [INF] The lens governs idioms of Elixir code (OTP structure, naming, error tuples). The core is a synchronous, cycle-stepped library with a C ABI. BEAM schedulers, preemption and NIF run-time limits are a poor match for a tight deterministic loop. Therefore the lens does not apply to the core's language, API or build.
- [INF] The only plausible relevance is a later Elixir-side host, test orchestrator or ROM-library service that calls the core through a Port or NIF. That would need the C ABI (every option provides one) and would not change the core language. If the owner ever wants an Elixir host, a Rust core would make Rustler an option, but that is speculation and no reason to choose Rust now.
- [UNK] Whether the owner wants any Elixir-based host or tooling. Low priority; ask only if it surfaces.

## Recommended Stack

### Core Technologies

| Technology | Version (observed latest, 2026-11-09) | Purpose | Why Recommended |
|------------|----------------------------------------|---------|-----------------|
| C | ISO C17 (`CMAKE_C_STANDARD 17`, extensions off) | Machine core (CPU/STIC/PSG/bus), narrow public API | Zero-translation C ABI; widest compiler support (gcc, clang, MSVC `/std:c17`); matches local siblings [REC, MEDIUM] |
| CMake | 4.4.5 latest (2026-10-27); local 4.4.3; set floor 3.25 | Build, install, export targets, CTest, presets | Native installed-consumer idiom; presets v6 start at 3.25 [OBS: gh api Kitware/CMake] [REC, HIGH for CMake over alternatives given C] |
| Ninja | 1.13.2 (2025-11-20) | Build executor | Fast, deterministic, already installed locally [OBS] |
| Clang/LLVM | 23.1.5 (2026-11-03) upstream; Homebrew has `llvm@21` and `llvm@18` locally; Apple clang 21.0.0 | Primary compiler for sanitizers, libFuzzer, clang-tidy/format | libFuzzer and MSan live here. Apple clang lacks the libFuzzer runtime (verified below) [OBS] |
| GCC | current distro release (not pinned; tag list was not informative) | Second compiler on Linux | Catches different warnings/UB; cheap [REC, MEDIUM] |
| MSVC | defer until Windows is a stated target | Windows build | `/std:c17` works but costs CI time; add only when a Windows consumer is in scope [REC, MEDIUM] |

### Supporting Libraries

| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| (none at runtime) | n/a | The core | Core has zero runtime dependencies [REC, HIGH] |
| SHA-256 | CMake built-in `cmake -E sha256sum` (verified locally: `abc` gives `ba7816bf...15ad`); public-domain C implementation only if a C-side digest is needed | Golden output hashes, fixture manifest digests | Avoids a dependency. A sibling `libfuturo/src/sha256.c` exists but is another project's code; do not copy without its licence being reviewed [OBS/REC] |
| Unity (ThrowTheSwitch) | v2.7.0 (2026-07-16) | Optional C unit-test macros | Only if an in-repo ~150-line harness proves unsatisfying. Adds a vendored dependency for assertions that a small header can provide [REC, LOW-MEDIUM] |
| SDL3 (host/runner only, not core) | siblings pin exact 3.4.x | Optional visible demo host | Host layer only; keep out of the core build graph. Needed only when a playable/visible scenario appears. Not researched here [UNK] |

### Development Tools

| Tool | Version | Purpose | Notes |
|------|---------|---------|-------|
| CTest | bundled with CMake | Test runner and exact inventory | Use `ctest -N` against a checked-in expected-name list; `--no-tests=error`; `SKIP_RETURN_CODE` so skips are visible, not silent (property exists locally) [OBS] |
| AddressSanitizer + UBSan | clang/gcc | Memory and UB checking | Turn on for every CI test run on Linux and macOS (compile-checked locally with Apple clang 21) [OBS] |
| MemorySanitizer | clang, Linux only | Uninitialised reads | Run in a scheduled or separate job; needs MSan-instrumented libc++ only for C++, not for plain C [BG] |
| libFuzzer | LLVM 21/23 (Homebrew LLVM on macOS; apt LLVM on Linux) | Fuzz loaders, API sequences, input scripts | Apple clang 21: `clang -fsanitize=fuzzer` fails to link (`libclang_rt.fuzzer_osx.a` not found, executed locally). Run fuzzers on Linux CI or with Homebrew LLVM [OBS] |
| AFL++ | v5.03c (2026-09-02) | Second fuzzing engine | Optional, scheduled only [REC, LOW] |
| clang-format / clang-tidy | LLVM 21+ | Style and static checks | Not installed locally (checked); install from Homebrew `llvm` [OBS] |
| hyperfine | v2.0.0 (2026-10-07) | CLI wall-clock benchmark with warmup and JSON export | Not installed locally. Use for end-to-end runner timing; do not use as the correctness oracle [OBS/REC] |
| valgrind (callgrind) | n/a | Deterministic instruction-count profiles on Linux | Not available on this macOS arm64 machine. Linux CI only; use `perf` or Instruments locally [OBS/BG] |
| REUSE | 6.3.0 (PyPI) | SPDX licence/copyright lint for fixtures, notices and manifests | Only useful once the owner picks a project licence; supports AGENTS.md "licence, notices" tracking [REC, LOW-MEDIUM] |
| Rust alternative toolchain | rustc/cargo 1.99.1 (2026-10-16), edition 2024 | Only if the owner picks Rust | Companion: cbindgen 0.29.4, cargo-c 0.10.26, cargo-fuzz 0.13.2, cargo-nextest 0.9.151, cargo-deny 0.20.2, cargo-llvm-cov 0.9.1, proptest 1.12.0, insta 1.49.0, criterion 0.8.2 (divan 0.1.21 last updated 2025-04), cargo-mutants 27.1.0, kani-verifier 0.68.0 [OBS: crates.io + gh api] |

### Test and CI patterns to adopt [REC]

## Part 2 — Toolchain for original CP1610 diagnostic fixtures

### Tools surveyed

| Tool | Licence [OBS] | Does it support CP1600/CP1610? | Output obligations | Verdict |
|------|---------------|-------------------------------|--------------------|---------|
| **as1600** (jzIntv / SDK-1600, `src/asm/`) | GPL-2.0-or-later for Joe Zbiciak's code (`src/COPYRIGHT.txt` header: "version 2 ... or (at your option) any later version"; fork COPYING.txt is GPLv2 text). `src/asm/README` says the Frankenstein Assembler (FRASM, Mark Zenier) base is public domain; Zbiciak's modifications are GPL. | Yes, purpose-built. Emits `.bin`+`.cfg` and `.rom`; README says images are multiples of 256 bytes | See analysis below | Best available CP1610 assembler; use as pinned dev-time tool only |
| **libasm** (tgtakaoka) v2.0.4, 2026-11-07 | Apache-2.0 (`LICENSE.md`; GitHub API reports "Other/NOASSERTION") | Yes: `CP1600` is in the CPU list, with `asm_cp1600.cpp` and `dis_cp1600.cpp`; CLI emits Intel HEX or Motorola S-record | Apache-2.0 does not claim ownership of what you assemble [BG] | Good independent second opinion and disassembler. 10-bit decle packaging into Intellivision ROM images is not provided; a thin converter is needed [UNK until spiked] |
| **IntyBASIC** (nanochess) | BSD-2-Clause for the compiler (`LICENSE`); GitHub reports NOASSERTION | Compiles BASIC to CP1610 asm for as1600 | Compiled programs include a prologue/epilogue asm runtime; its licence was not verified [UNK] | Not suitable for diagnostics: hides instruction-level behaviour and adds runtime code |
| **SDK-1600 library `.asm` files** (`examples/library/`) | Per file. `print.asm` is CC0-1.0 (author dedication in header). `hello.asm` is GPLv2+. Others not read | n/a (source includes) | **These are the real obligation carriers**: `INCLUDE`-ing a GPL file puts GPL-covered text in your fixture | Do not include any SDK-1600 library or example file in fixtures |
| GI CP-1600 Cross Assembler Simulator Users Manual (Nov 1974, bitsavers) | Historic document | Documents the original GI assembler syntax | n/a | Reference for syntax and mnemonics only |

### Do assembler build outputs carry licence obligations? [INF, MEDIUM; not legal advice]

- GPLv2 text (observed via the FSF materials surfaced by search) says output from a program is covered only if its content constitutes a work based on the program. The current FSF FAQ (search-snippet-level evidence, LOW-MEDIUM) adds that output generally is not covered by the program's copyright, with exceptions where the program copies its own text or art into the output.
- So a ROM assembled from project-authored source by `as1600` is not made GPL by the tool. Obligations arise if: (a) the project redistributes `as1600` binary or source (then normal GPL compliance for that tool); (b) fixture source `INCLUDE`s GPL library files; or (c) the tool injects non-trivial text into outputs (the `.rom` format carries `CFGVAR` metadata only if the source supplies it).
- [REC] Treat the assembler as an external, pinned, non-shipped dev tool. Record in the fixture manifest: tool name, version or commit, tool binary SHA-256, source SHA-256, exact command line, output SHA-256, and the licence of the source plus every included file. Never embed third-party `CFGVAR` text. AGENTS.md already requires source, recipe, hashes, licence, notices and allowed use per fixture.
- [UNK] The MAME-style "does using GPL tools taint the project's licence" question is moot until the owner picks a project licence (an owner gate).

### Is a project-owned minimal assembler warranted? [REC, LOW-MEDIUM confidence: not yet]

- **CPU instruction-level tests do not need an assembler.** Write them as table-driven records with literal decle values taken from the GI manual's instruction/encoding tables, which are independent of any assembler. This is the strongest "independent expected value" form.
- **Guest diagnostic ROMs** (STIC output, controller readback, PSG register writes) need real programs. For the first one or two, a pinned `as1600` build plus an independent `libasm` assembly of the same source, compared byte for byte, is cheaper than writing an assembler. Agreement of two assemblers is evidence of encoding correctness only, not of hardware behaviour. [INF]
- **Commit the assembled bytes** (as text decle lists or `.bin`) with source and manifest, so CI and the installed consumer never need an assembler. Regeneration is a documented optional recipe.
- **Revisit** a project-owned assembler (roughly 500 to 800 lines of C, written from the GI manual, no GPL input) only if: fixture count grows beyond a few dozen, licence review of the pinned tool becomes a recurring burden, or macros/labels become essential. The risk of building it early is "shared-misreading": the author's assembler and the author's emulator can both be wrong the same way. The three-way comparison above is the mitigation.
- [UNK] Exact `.bin` word layout (how a 10-bit decle is stored in the container) and `.rom` header: see `doc/rom_fmt` and `doc/programming/` in the jzIntv tree (GPL docs). The facts of the format can be implemented independently, but a licence review precedes reading code. Defer to the cartridge-format research.
- [UNK] The reset-entry story for EXEC-free CPU diagnostics (what profile maps fixture code at the CPU start address when no EXEC image is available) is an owner/profile question, not a tool question. Not resolved here.

## Part 3 — Hardware reference stack

### Authority tiers [REC]

| Tier | Sources | Use |
|------|---------|-----|
| 1. Manufacturer primary | GI CP-1600 Microprocessor Users Manual (May 1975); GI 1980 and 1982 Microelectronics Data Catalogs (CP1610, AY-3-8900/-1 STIC, RO-3-9502/9503/9504, RA-3-9600, AY-3-8910 sections); GIC1600 Microcomputer Users Manuals (Sep 1975) | Instruction set, encodings, STIC pin/bus/geometry, memory parts. Define what the manufacturer documented |
| 2. First-party-adjacent | Blue Sky Rangers 2609 technical overview (derived from an in-house Mattel document dated 31 Oct 1980, "with some corrections") | Programmer-level behaviour, clock/cycle statements, controller description. Cross-check against tier 1 |
| 3. Service/hardware docs | Mattel 2609 subassembly service manual (21 pages per iFixit), schematics, physical measurement | Board-level wiring, controller electrical behaviour. Not yet verified |
| 4. Community docs | jzIntv `doc/programming/*.txt` (STIC, PSG, interrupts, memory map) and wiki.intellivision.us | Leads and cross-checks. Never sole authority |
| 5. Other emulators' code | MAME `intv.cpp` family, jzIntv source | Family checklist and discrepancy hints only. Not an oracle; behaviour agreement with them is not independent evidence |

### Bootstrap source verification (all accessed 2026-11-09)

| # | Source (as listed in EMULATOR-DNA.md) | URL status | Licence / rights | What I verified | Authority and caveats |
|---|---------------------------------------|-----------|------------------|-----------------|-----------------------|
| 1 | MAME `intv.cpp` (`github.com/mamedev/mame/blob/master/src/mame/mattel/intv.cpp`) | 200 (blob and raw) | **`// license:BSD-3-Clause`**, copyright-holders Nathan Woods, Frank Palazzolo. Same header on `intv.h`, `stic.cpp`, `stic.h`, `intv_v.cpp`, `cp1610.cpp`, `ay8910.cpp`, `bus/intv/slot.cpp` (note: `ay8910.cpp` is Couriersud, `slot.cpp` is Fabio Priuli). MAME docs: project as a whole GPL-2.0+; over 90 percent of files BSD-3-Clause; MAME name/logo is a registered trademark. Licence is per file, so check each file | Clock lines in the file: CP1610 `XTAL(3'579'545)/4`, STIC `XTAL(3'579'545)`, AY8914 `XTAL(3'579'545)/2`. The file's own comments mark uncertainty: EXEC half "maps at words 0000-0fff or 1000-1fff [unclear which, guess is 0000]"; TBD list includes STIC and memory-system workarounds. Last commit touching the file: 2026-08-02 (GitHub commits API) | Emulator source, not hardware oracle. BSD-3-Clause is permissive, but AGENTS.md still says no copying without deliberate review; no benefit here since it is not an oracle |
| 2 | Mattel 2609 service manual scan (iFixit) | 200, but returns an **HTML viewer page** titled "Intellivision Service Manual - iFixit", not a raw PDF | Unknown. Third-party host; Mattel rights not documented | Page metadata: "subassembly service manual ... providing diagnostic routines, troubleshooting guides, and disassembly instructions", **21 pages**. I did **not** read the viewer content, so the MTE-201 and "90-minute burn-in" claims in EMULATOR-DNA.md are [UNK] | Likely subassembly-level, not a full schematic. Search surfaced other copies (elektrotanya, AtariAge thread, ConsoleMods Sylvania manual) that I did not content-verify (some returned 403). Do not redistribute |
| 3 | Blue Sky Rangers 2609 page | 200 | "Original content (c) Blue Sky Rangers, Inc.", all rights reserved; "INTELLIVISION ... trademarks of Atari Interactive, Inc." Read-only reference | Page states it is from "an in-house document dated October 31, 1980 ... with some corrections and clarifications". I read the full technical overview. See cross-check table | Strong secondary. Edited text, not the raw document |
| 4 | GI 1977 Data Catalog (bitsavers) | 200, PDF, 608 pages, about 42 MB (bitsavers returned 403 to a UA-less `HEAD`; works with a browser UA) | Third-party archive scan of a GI catalog; reuse rights not documented. Do not redistribute | CP1610 is indexed at "4B-38". The section at 4B-37 to 4B-40 is the **GIMINI Programmable Game Set** (CP1610 + STIC AY-3-8900/8900-1 + RO-3-20480 2048x10 ROM + RO-3-9316A 2048x8 graphics ROM + five 256x4 RAMs). It describes a **pre-console game-set precursor**: no CPU clock, no registers, no console wiring | **Weaker than the bootstrap implies.** The 1980 and 1982 catalogs below contain the real "8900" sections |
| 5 | Mattel owner manual (videogameconsolelibrary) | **404** at the exact URL. Following redirects lands on a generic category page (`/70s-game-consoles/`) | Unknown | Dead link. A candidate copy `mocagh.org/intvother/intv-manual.pdf` returns 200 `application/pdf`, but I did not open it and its rights are unknown | User-facing only. Not needed for electrical decisions |
| 6 | intellivision.us wiki, Hand Controller | 200 | Wiki; licence unknown | Retrieved revision `oldid=14808`, "last edited 4 December 2010". States: reads return all 1s in the low 8 bits when idle (active-low); pressing the "1" key gives `$7E`; "1" and "9" together give `$5A`; reads at `$01FE` (right) and `$01FF` (left); a single keypad press, or a side button combined with a disc input, is uniquely decodable, but keypad combined with side-button or disc, and two side buttons together, are not. The key-matrix image was not extracted | Community, single old revision. Consistent with BSR's description that the controller ports are the PSG's I/O ports |
| 7 | FujiNet jzintv fork | 200 | **GPL-2.0** (GitHub API); `COPYING.txt` is the GPLv2 text; README says "GPL v2 or later"; source header (c) 1998-2020 Joseph Zbiciak, "version 2 ... or (at your option) any later version" | README: based on upstream 2020-08-22 (SVN r2141); `ReleaseNotes_20260813.txt` says it is a custom downstream build, not an official jzIntv release; last push 2026-11-08. README states copyrighted EXEC/GROM/game ROMs are not included. Official page `spatula-city.org/~im14u2c/intv/` (http only) still advertises 20200712 as "current stable dev" with SDK-1600 merged into the jzIntv package | Historical implementation, not an oracle. Do not copy or link without licence review. Its `doc/` tree is useful community documentation |

### Better primary sources not in the bootstrap list [OBS]

| Document | URL | Relevant content |
|----------|-----|-------------------|
| CP-1600 Microprocessor Users Manual (May 1975), 116 pages | `https://www.bitsavers.org/components/gi/CP1600/CP-1600_Microprocessor_Users_Manual_May75.pdf` | Instruction set, encodings, cycle table, timing, interrupt behaviour. "87 BASIC INSTRUCTIONS". 5 MHz two-phase clock gives a 400 ns microcycle; each microcycle is four time slots TS1 to TS4 |
| GIC1600 Microcomputer Users Manuals (Sep 1975), 139 pages | `https://www.bitsavers.org/components/gi/CP1600/GIC1600_Microcomputer_Users_Manuals_Sep75.pdf` | Surrounding series documentation (not read in depth) |
| GI Microelectronics Data Catalog 1980 | `https://www.bitsavers.org/components/gi/_dataBooks/1980_GI_Microelectronics_Data_Catalog.pdf` | "8900 Home Information System": CP1610 (p. 5-3 and 2-64), AY-3-8900/-1 STIC (5-10), RO-3-9502 program ROM (5-13), RO-3-9503 graphics ROM (5-16), RA-3-9600 system RAM (5-18), RO-3-9504 cartridge ROM (5-21), AY-3-8910 sound generator (5-23) |
| GI Microelectronics Data Catalog 1982 | `https://www.bitsavers.org/components/gi/_dataBooks/1982_GI_Microelectronics_Data_Catalog.pdf` | CP1610 (9-22), AY-3-8900/-1 (9-43), RO-3-9502 (9-46), RO-3-9503 (9-49), RA-3-9600 (9-51) |
| jzIntv docs (GPL) | `https://github.com/FujiNetWIFI/jzintv/tree/main/doc` | `doc/programming/{stic,psg,interrupts,memory_map,graphics_mem}.txt`, `cp1600_ref.pdf` and others. Community-level |

### Claim cross-check

| Claim | Evidence | Verdict |
|-------|----------|---------|
| **CPU "clock" about 894.886 kHz (NTSC)** | BSR: "machine cycle rate of 894.886 KHz"; 4 to 12 microcycles per operation. jzIntv `interrupts.txt`: "CP-1600 Clock Rate (NTSC/4) 894886.25 Hz", 14934 cycles/frame, 57 cycles/scanline, 59.92 Hz. MAME: CP1610 clocked at `XTAL(3'579'545)/4`. GI 1980 catalog: CP1610 "1 us cycle time, 2MHz 2-phase clock". GI 1975 manual: CP1600 microcycle 400 ns at 5 MHz two-phase | **Holds as a cycle (microcycle) rate, not as a clock-input rate.** In GI's terminology the datasheet "cycle time" equals the microcycle and is two input clock periods. [INF] The 2609 therefore feeds the CPU an input clock near 1.79 MHz (colorburst/2), with a microcycle of about 1.1175 us, slightly slower than the 1 us rating. Terminology disagreement: BSR, jzIntv and MAME call 894.886 kHz the "clock" or "machine cycle rate". Define one canonical unit in the spec. [UNK] schematic confirmation of the colorburst/2 CPU input clock |
| 3.579545 MHz exact value | jzIntv: 894886.25 = 3579545/4. GI 1980: STIC -1 "from a 3.579545MHz clock". [BG] NTSC colorburst is 315/88 MHz = 3.57954545... | Documents use the truncated 3.579545. A true-rational constant (315/88 MHz) differs by about 0.00001 percent. [UNK] which the spec adopts; measured hardware would settle it. Pick one explicitly |
| **Frame timing** | jzIntv: 262 scanlines, 57 cycles per scanline, 14934 cycles/frame, 59.92 Hz. [INF] 262 x 57 = 14934; 14934 x 4 = 59736 colorburst clocks; 3579545 / 59736 = 59.9227 Hz, matching 59.92 | Internally consistent, but jzIntv is community. GI 1977 GIMINI text gives 63.5 us per line (45 us active) as a precursor figure. [UNK] a tier-1 source for lines/frame and cycles/line |
| **PAL timing** | GI 1980: AY-3-8900 (not -1) is the CCIR part, clocked from 4.000 MHz. jzIntv: PAL values marked estimates ("??") | Clock confirmed by GI; scanline and cycle counts remain [UNK] |
| **10-bit ROM words** | GI 1977/1980/1982: program ROM "organized as 2048 x 10", cartridge ROM RO-3-9504 "2048 x 10". BSR: 10-bit "decle"; 16-bit instructions are two decles ("bidecle"). MAME describes ROM sizes as e.g. "4KiB(2Kiw)" | **Holds** for the ROM parts. Caveats: the CPU bus is 16 bits; RA-3-9600 RAM is 352 x 16; GROM is 2048 x 8 (not 10-bit). [UNK] what the upper 6 bits read as for ROM addresses. MAME's comment "RO-3-9503 ... 2KiB(1Kiw)" mislabels units for a 2048 x 8 ROM; do not rely on MAME's unit labels |
| **STIC geometry** | GI 1980 p. 5-10 (-1, NTSC): 8 foreground objects on a 167H x 105V grid "of which 159 x 96 are visible positions"; 20 x 12 background cards, 19 columns of 8x8 and the 20th column 7x8; 16 colours; background memory 240 x 14-bit words; 3.579545 MHz clock. BSR: 160x96 nominal, 159x96 actually displayed, each pixel 2 TV lines high; 240 cards; 8 MOBs. jzIntv: 192 active scanlines | **Consistent across tier 1, 2 and 4.** Nominal vs visible 160 vs 159 columns confirmed. GI 1977 GIMINI: 192 active lines of 45 us in 63.5 us |
| **PSG chip identity** | BSR and MAME: AY-3-8914. GI 1980 catalog lists the **AY-3-8910** as the sound generator of the "8900" system (dual 8-bit I/O ports). `grep` found no "8914" in the 1977, 1980 or 1982 catalogs | **Disagreement and gap.** [UNK] The 8914 register map and any behavioural difference from the 8910. Only secondary hints (Furnace tracker docs, a Tiny Tapeout project citing decapped 8910 and 8914 die analysis) came from search, so LOW confidence. The PSG register layout, I/O port usage and clock cannot be frozen from tier-1 sources yet |
| **PSG clock** | MAME: AY8914 at `XTAL(3'579'545)/2`. jzIntv `psg.txt`: PSG driven at half the 3.579545 MHz clock, tone `F = 3579545 / (32 * P)` | Mutually consistent but both community/emulator. [UNK] tier-1 confirmation |
| **System RAM** | GI 1980: RA-3-9600 is 352 x 16 plus a 20-word current-line buffer. BSR: 240 BACKTAB locations + 112 x 16-bit stack RAM = 352, plus a GROM/GRAM arbitration controller | Consistent |
| **GROM / EXEC parts** | GI 1980: RO-3-9503 graphics ROM 2048 x 8 holding 256 8x8 matrices; RO-3-9502 program ROM 2048 x 10. BSR: EXEC is 4K across two 2K chips (RO-3-9502 + RO-3-9504) in the original, a single 4K RO-3-9506 in the Intellivision II; GROM has 213 images (256 intended) with 344 bytes of EXEC overflow. MAME: same part numbers, with uncertain address halves | The GI catalog calls RO-3-9504 a "cartridge ROM" while BSR and MAME use it as the second EXEC half. Not necessarily contradictory (same mask-ROM family), but flag it. Whether the second EXEC half maps at `$1000` or elsewhere is [UNK] and needs the EXEC image plus board evidence |
| **Instruction count** | GI 1975 manual: "87 basic instructions". GI 1980 catalog: "86 basic instructions". BSR: "1024 separate op-codes" | Counts differ by counting convention. Use an opcode-level table, not a count, as the test inventory |
| **Controller electrical model** | BSR: each hand controller feeds an 8-bit input port that is physically the sound chip's I/O port; disc gives a 5-bit code, action buttons a 3-bit code, keypad an 8-bit code; the two top buttons are wired together; "the keypad duplicates the disc and action keys ... either but not both". Wiki: active-low, bits clear when pressed, combinations clear the union of bits, some combinations are not decodable | Mutually consistent as an active-low matrix. BSR's "not both at the same time" is a programmer-level simplification; the wiki implies simultaneous presses are electrically possible but ambiguous. [INF] model controllers as electrical bit-clear semantics, keep host mapping separate. [UNK] schematic or measurement for the exact matrix and cross-talk behaviour |
| **CPU cycle timing at system level** | GI 1975 manual: cycle counts per instruction (e.g. MOVR 6, +1 if destination is R6/R7) plus wait-state cycles W for slow memory. jzIntv: about 13518 usable CPU cycles per frame with display enabled and about 3780 to 3790 in "Bus Copy" mode vs 14934 with display disabled | The datasheet table is a lower bound. Real console timing includes STIC-imposed bus stalls (BUSRQ/BUSAK, SR1/SR2/SR3 handshake in GI 1980 p. 5-10). [UNK] the exact stall model; this is the main cycle-accuracy research item |

### Disagreements to carry forward (summary)

### Gaps that need phase-specific research (not resolvable from this pass)

- AY-3-8914 register map and behaviour: needs a primary data sheet, die analysis or hardware measurement.
- STIC register-level behaviour (collision, MOB priority, vertical delay, blanking, bus arbitration) from the AY-3-8900 section (GI 1980 p. 5-10; 1982 p. 9-43) plus tests. Not extracted in detail here.
- Reset vector and power-up/MCLR sequence for the CP1610 in the 2609 (GI 1975 manual section on the power-on/master-clear sequence at the line near "MSYNC").
- EXEC/GROM mapping and what the EXEC leaves in RAM, depending on how (or whether) system images are supplied (open question 3 in PROJECT.md).
- Intellivision II, ECS, IntelliVoice, Sears: out of scope for the first slice per PROJECT.md. Not researched.

## Installation

# macOS (this machine: CMake 4.4.3, Ninja, Apple clang 21 already present)

# Linux CI (Debian/Ubuntu style)

# Configure, build, test (illustrative; presets are a roadmap deliverable)

# Golden hash check without extra dependencies

# Fixture tooling (dev-time only, pinned, not shipped)

# Build as1600 from a pinned jzIntv source checkout in a scratch dir; record its commit and binary sha256.

# Cross-check with libasm v2.0.4 (Apache-2.0): build its `cli/asm` and run with -C CP1600.

# Only if the owner chooses Rust instead

## Alternatives Considered

| Recommended | Alternative | When to Use Alternative |
|-------------|-------------|-------------------------|
| C17 + CMake | Rust + cargo + cbindgen + cargo-c | Owner prioritises memory safety, property testing and model checking over ecosystem alignment; accepts nightly-only sanitizers and a larger dev-tool tree |
| C17 + CMake | Zig | After Zig reaches 1.0 and the owner values `zig build` cross-compilation; or as a drop-in C compiler (`zig cc`) for cross builds only |
| C17 + CMake | C++20 | Owner wants Catch2/GoogleTest/Google Benchmark. Keep an `extern "C"` header and a no-exceptions, no-RTTI core build |
| CMake + Ninja | Meson 1.12.1 | Owner prefers Meson ergonomics and no consumer expects `find_package` |
| In-repo test harness | Unity v2.7.0 | Contributors want ready-made assertion macros and accept a vendored MIT dependency |
| `as1600` (pinned, dev-time) | `libasm` v2.0.4 as primary | Owner wants a purely permissive (Apache-2.0) toolchain and the 10-bit packaging gap proves small |
| `as1600` + `libasm` | Project-owned assembler | Fixture count grows large, or licence review of the pinned tool becomes a recurring cost |
| libFuzzer | AFL++ v5.03c | Want a second engine or persistent-mode comparison in scheduled jobs |

## What NOT to Use

| Avoid | Why | Use Instead |
|-------|-----|-------------|
| Copying code, tables or comments from MAME, jzIntv, or other emulators | Licences differ per file; behaviour agreement is not an independent oracle; AGENTS.md and PROJECT.md forbid it without review | Implement from tier-1 documents; use other emulators only to find discrepancies to investigate |
| Linking or shipping jzIntv/as1600 in the project | GPL-2.0-or-later obligations attach to redistribution and linking | Pinned external dev tool; project-authored sources only |
| `INCLUDE`-ing SDK-1600 library or example `.asm` files in fixtures | Per-file licences (GPLv2+ and CC0 both observed); outputs would embed covered text | Self-contained fixtures with project-owned macros |
| IntyBASIC for diagnostics | Hides instruction behaviour; runtime asm licence unverified | Hand-written CP1610 assembly or literal decle tables |
| The GI 1977 catalog as the CP1610/STIC authority | Describes the GIMINI precursor | GI 1980 and 1982 catalogs; GI 1975 CP-1600 manual |
| Apple clang for libFuzzer | Runtime missing (`libclang_rt.fuzzer_osx.a` not found; executed locally) | Homebrew `llvm` or Linux CI for fuzz targets |
| Floating point in the machine (CPU, STIC, PSG) | Cross-compiler and FMA-contraction nondeterminism | Integer or fixed-point; derive host PCM outside the core |
| Host clocks, `rand()`, global mutable state, file IO inside the core | Violates determinism and multi-instance rules | Explicit machine time, caller-owned buffers, an instance handle |
| Redistributing EXEC, GROM, MTE-201 or other ROMs/manual scans | Rights-bearing; no documented permission | User-supplied images by path (if owner chooses), or original replacements later |
| Google Benchmark for the C core | C++-only dependency | In-repo benchmark runner plus hyperfine |
| Zig as the core language now | Pre-1.0 churn | Revisit after 1.0 |

## Stack Patterns by Variant

- `crate-type = ["staticlib", "cdylib"]`, `#![no_std]`-style core with zero crates, `panic = "abort"`, `edition = "2024"`, pinned `rust-version`.
- Generate the header with cbindgen but review it as the contract; install with cargo-c; keep the C consumer test in CMake or plain `cc`.
- Sanitizers: run ASan/UBSan on nightly in a scheduled job, Miri on unsafe-free logic only where applicable. Add `cargo-deny` to enforce the small dependency policy.
- Add an MSVC `/std:c17` job and an installed-consumer job; keep `__declspec(dllexport)` behind an export macro in the public header.
- Keep SDL (siblings pin SDL3) in a separate `runner` target outside the core library; the core's tests must never link it.
- The C ABI already fits; confirm keypad plus disc plus both-controller state fits the 32-bit button mask before proposing any shared-contract change (PROJECT.md: fit unconfirmed). Not researched here.

## Version Compatibility

| Package A | Compatible With | Notes |
|-----------|-----------------|-------|
| CMake floor 3.25 | CMake presets v6, up to latest 4.4.5 | Siblings use 3.25 as the floor [OBS]. Test the floor and the latest in CI only if consumers need both |
| ASan/UBSan | gcc and clang on Linux, Apple clang on macOS | Compile-checked locally with Apple clang 21 |
| libFuzzer | clang with the fuzzer runtime (LLVM from Homebrew or apt) | Not Apple clang 21 |
| MSan | clang on Linux only | Plain C is easier than C++ (no instrumented libc++) [BG] |
| cbindgen 0.29.4 | Rust 1.99.x | Only relevant on the Rust path |
| libasm v2.0.4 | CP1600 target (`-C CP1600`), Intel HEX / S-record output | Outputs word data in hex records; 10-bit packaging unverified [UNK] |

## Sources

- GitHub releases API (via `gh api`): `rust-lang/rust` 1.99.1; `Kitware/CMake` v4.4.5; `mesonbuild/meson` 1.12.1; `ninja-build/ninja` v1.13.2; `llvm/llvm-project` llvmorg-23.1.5; `AFLplusplus/AFLplusplus` v5.03c; `sharkdp/hyperfine` v2.0.0; `ThrowTheSwitch/Unity` v2.7.0; `catchorg/Catch2` v3.16.1; `google/googletest` v1.18.0; `google/benchmark` v1.10.0; `lu-zero/cargo-c` v0.10.26; `tgtakaoka/libasm` v2.0.4.
- crates.io API: cbindgen 0.29.4, cargo-fuzz 0.13.2, proptest 1.12.0, criterion 0.8.2, cargo-nextest 0.9.151, cargo-llvm-cov 0.9.1, cargo-deny 0.20.2, cargo-mutants 27.1.0, kani-verifier 0.68.0, insta 1.49.0, divan 0.1.21. PyPI: reuse 6.3.0.
- `https://ziglang.org/download/index.json`: Zig 0.17.1 (2026-11-02), 0.17.0, 0.16.0, 0.15.2; master 0.18.0-dev.
- Rust FFI panic behaviour: search results indicating `extern "C"` aborts on unwinding since 1.81 (LOW-MEDIUM; confirm against the Rust reference before relying on it): https://doc.rust-lang.org/std/panic/fn.catch_unwind.html
- MAME source and licence: https://github.com/mamedev/mame/blob/master/src/mame/mattel/intv.cpp (and sibling files under `src/mame/mattel/`, `src/devices/cpu/cp1610/`); https://raw.githubusercontent.com/mamedev/mame/master/docs/source/license.rst
- jzIntv fork and docs: https://github.com/FujiNetWIFI/jzintv (README, COPYING.txt, `src/COPYRIGHT.txt`, `src/asm/README`, `doc/programming/{stic,psg,interrupts}.txt`, `examples/library/print.asm`, `examples/hello/hello.asm`); upstream page http://spatula-city.org/~im14u2c/intv/
- libasm: https://github.com/tgtakaoka/libasm (README, `LICENSE.md`, `src/asm_cp1600.cpp`); IntyBASIC: https://github.com/nanochess/IntyBASIC (`LICENSE`)
- GI primary documents (bitsavers): `https://www.bitsavers.org/components/gi/CP1600/CP-1600_Microprocessor_Users_Manual_May75.pdf`; `.../CP1600/GIC1600_Microcomputer_Users_Manuals_Sep75.pdf`; `.../_dataBooks/1977_GI_Data_Catalog.pdf`; `.../_dataBooks/1980_GI_Microelectronics_Data_Catalog.pdf`; `.../_dataBooks/1982_GI_Microelectronics_Data_Catalog.pdf`
- Blue Sky Rangers: https://history.blueskyrangers.com/hardware/2609.html
- Hand controller wiki: https://wiki.intellivision.us/index.php/Hand_Controller (oldid=14808)
- iFixit service manual page: https://www.ifixit.com/Document/vyUeXhXOo1tXKF3W/Intellivision_Service_Manual.pdf (HTML viewer)
- FSF GPL FAQ / GPL-2.0 text on output coverage: https://www.gnu.org/licenses/gpl-faq.en.html (the page itself returned 403 to `curl`; the claim rests on search-result excerpts, LOW-MEDIUM; confirm directly and with counsel if it becomes load-bearing)
- GitHub Actions pricing (third-party reporting, LOW): https://resources.github.com/actions/2026-pricing-changes-for-github-actions/
- Local executed checks (this machine, 2026-11-09): `clang -fsanitize=fuzzer` link failure on Apple clang 21; ASan+UBSan compile OK; `cmake -E sha256sum` works on CMake 4.4.3; no Rust, Zig, Meson, valgrind, AFL++, clang-tidy or hyperfine installed.
- Local read-only sibling skim: `../{libfuturo,nesturbator,gabbaboy,genewhip,schwaystation}` CMake files and file-extension counts. No builds or tests run.

<!-- GSD:stack-end -->

<!-- GSD:conventions-start source:CONVENTIONS.md -->

## Conventions

Conventions not yet established. Will populate as patterns emerge during development.
<!-- GSD:conventions-end -->

<!-- GSD:architecture-start source:ARCHITECTURE.md -->

## Architecture

Architecture not yet mapped. Follow existing patterns found in the codebase.
<!-- GSD:architecture-end -->

<!-- GSD:skills-start source:skills/ -->

## Project Skills

No project skills found. Add skills to any of: `.claude/skills/`, `.agents/skills/`, `.cursor/skills/`, `.github/skills/`, or `.codex/skills/` with a `SKILL.md` index file.
<!-- GSD:skills-end -->

<!-- GSD:workflow-start source:GSD defaults -->

## GSD Workflow Enforcement

Before using Edit, Write, or other file-changing tools, start work through a GSD command so planning artifacts and execution context stay in sync.

Use these entry points:
- `/gsd-fast` for a trivial task inline, with no subagents and no PLAN.md
- `/gsd-quick` for small fixes, doc updates, and ad-hoc tasks
- `/gsd-debug` for investigation and bug fixing
- `/gsd-execute-phase` for planned phase work

Do not make direct repo edits outside a GSD workflow unless the user explicitly asks to bypass it.
<!-- GSD:workflow-end -->

<!-- GSD:profile-start -->

## Developer Profile

> Profile not yet configured. Run `/gsd-profile-user` to generate your developer profile.
> This section is managed by `generate-claude-profile` -- do not edit manually.
<!-- GSD:profile-end -->
