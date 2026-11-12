# Requirements: iamverysmartvision

**Defined:** 2026-11-09
**Core Value:** A deterministic, observable machine core whose every compatibility claim is bound to an exactly named profile and independent evidence — never an unqualified "it runs Intellivision games" label.

Derived from `.planning/PROJECT.md`, `AGENTS.md`, the bootstrap pack and
`.planning/research/` (SUMMARY, STACK, FEATURES, ARCHITECTURE, PITFALLS). Auto
mode: research table stakes and seed-named capabilities are v1; unmentioned
differentiators and all variant/peripheral profiles are v2.

**How to read these requirements**

- They are written so that owner choices that are still open (language, first
  profile revision, system-image policy, controller representation, license)
  are *parameters*, not silent decisions. Where a requirement depends on an
  open decision it names the decision (`D#`, see "Decision Gates").
- "Profile" means an explicitly named hardware/content configuration recorded
  as data with per-field provenance tags. The recommended first profile is the
  2609-family NTSC Master Component (recommendation, not an owner decision).
- "Evidence tier" means one of: source expectation, public vector, original
  diagnostic, independent-implementation comparison, measured hardware,
  deterministic repeatability, playable guest progress, installed-host
  behavior. No requirement is satisfied by a tier other than the one it names.

## v1 Requirements

### Foundations, Evidence and Rights (FND)

- [x] **FND-01**: The first hardware profile exists as a data record that names model, revision, region/video timing, STIC and PSG part, reset/power-on policy and cartridge arrangement, with a provenance/tier tag on every field (selection gated on D3).
- [x] **FND-02**: The project has a written evidence-tier vocabulary and a claim ledger; every support claim in docs or reports cites a profile identity, tier and evidence artifact.
- [x] **FND-03**: A discrepancy log records each unresolved source conflict (seeded with research conflicts C1–C28) with its chosen default policy (`unknown-default-chosen`) and the test or measurement that would resolve it.
- [x] **FND-04**: A fixture manifest records source, recipe/tool+version, input and output digests, license, notices, allowed use and a `redistributable: yes/no/unknown` field (unknown treated as no) for every fixture.
- [x] **FND-05**: A provenance rule is documented and checked in review: behavior is implemented from cited primary/secondary sources; no code, tables or constants are copied from jzIntv, MAME or other emulators.
- [x] **FND-06**: Project-owned reference notes extract the CP-1610 opcode/encoding/cycle tables and reset/interrupt sequence from the GI CP-1600 manual (May 1975) read directly, with page citations.
- [x] **FND-07**: Time-base vocabulary is defined (CPU cycle vs input clock vs STIC/PSG clocks, crystal constant) and used consistently in code and docs (D2).

### CPU — CP-1610 (CPU)

- [x] **CPU-01**: Every CP-1610 opcode in an opcode-level inventory executes with hand-derived expected register, memory, flag and cycle results, each vector citing its source page.
- [x] **CPU-02**: Interrupt and bus-request acceptance follows an explicit state model (previous-instruction interruptibility, I flag, D/SDBD state, BUSRQ), with vectors and named-fault controls for each acceptance case.
- [x] **CPU-03**: SDBD double-byte data behaves per source for supported addressing modes, and unsupported modes follow a named, documented, tested policy.
- [x] **CPU-04**: Stack-direction, auto-increment register and reset-entry behavior are verified by vectors derived from the primary manual (resolving conflict C17).
- [x] **CPU-05**: The CPU stepper is pure and reports each instruction's cycle-stamped bus accesses, so devices can be advanced to the exact access time.

### Memory, Loading and Refusal (MEM)

- [x] **MEM-01**: The memory map is generated from profile descriptors (range, device, width mask, alias, blocked-read and undriven-read policy), and a decode-table dump matches the sourced literal map.
- [x] **MEM-02**: 10-bit ROM (decle) and 8-/16-bit RAM width masking and store-truncation behave per profile and are tested at each boundary.
- [x] **MEM-03**: A caller can load typed content — system images (EXEC, GROM) and cartridge segments — as separate inputs, and the core reports each image's length and digest (system-image supply policy per D5).
- [x] **MEM-04**: Malformed, oversized, overlapping, out-of-range, missing-system-image and unknown-profile loads are refused with a typed error, and a refused load leaves prior machine state unchanged (refusal matrix test).
- [x] **MEM-05**: One documented cartridge import format is supported, specified in the project's own words, with positive, boundary and malformed fixtures.

### Core Contract and Host Independence (API)

- [x] **API-01**: A host can create, reset, unload and destroy independent machine instances; two instances run concurrently without shared mutable state (isolation test).
- [x] **API-02**: Execution is bounded by an explicit machine-time target (absolute cycle target; frame target as a derived convenience), with overshoot reported; splitting one run into slices yields identical state and output (slicing-invariance test).
- [x] **API-03**: Controller input is applied at an explicit machine-time boundary declared by the API; the same input script yields identical results regardless of host timing.
- [x] **API-04**: Video and audio outputs are written into caller-owned buffers; undersized buffers are refused before any state changes.
- [x] **API-05**: The core has no dependency on host clocks, filesystems, environment, threads, physical controller discovery or presentation, enforced by a build/link or symbol check.
- [x] **API-06**: Every API error/refusal has a documented effect on state (none, or a named partial effect), and each is covered by a test.

### Headless Runner and Verification (RUN)

- [x] **RUN-01**: A headless runner executes a scripted scenario (content, profile, input schedule, run bound) on fresh isolated state and reports machine time, stop reason, raw outputs and guest-state assertions.
- [x] **RUN-02**: The runner distinguishes harness failure, guest failure, timeout, cancellation and missing fixture as separate outcomes.
- [x] **RUN-03**: An exact test-inventory gate fails when a required test is missing, duplicated, unexpectedly skipped or consuming stale output, and reports executed/skipped/missing counts.
- [x] **RUN-04**: Each important gate has a named-fault control that is shown to fail for its intended reason.
- [x] **RUN-05**: Per-domain digests (CPU/memory state, raw video, raw audio, input log) are produced, and repeated runs match; golden files record their evidence tier and oracle in a header.
- [x] **RUN-06**: At least one original, project-authored diagnostic runs end-to-end from reset (via a project boot path per D5) and reaches an expected guest-state predicate whose expected value is derived independently of the emulator.

### Packaging, Local Gate and CI (PKG)

- [x] **PKG-01**: A clean, independent consumer project builds against the *installed* package in a relocated prefix and exercises load, refusal, run and digest.
- [x] **PKG-02**: One documented local command runs the same checks as the fast PR gate.
- [x] **PKG-03**: CI runs the fast gate on every PR, shows skips and missing artifacts visibly, and never caches or uploads system images or rights-unknown fixtures.
- [x] **PKG-04**: CI critical path (queue, setup, build, test) is measured and recorded per run so cache/concurrency/matrix changes are evidence-driven.

### Sound — PSG (AUD)

- [x] **AUD-01**: The PSG register file, tone, noise and envelope behavior follow the verified part's documented map, with per-rule vectors; behaviors lacking a primary source are profile data with tier tags (period-0, envelope select, LFSR, DAC).
- [x] **AUD-02**: The core emits a cycle-stamped PSG register-write log and integer chip-rate samples as raw audio, separate from any host PCM conversion (reference PCM location per D7).

### Hand Controllers (INP)

- [x] **INP-01**: Both controllers' 12 keypad keys, 16 disc directions and side-action inputs each produce the profile's documented read value through the PSG I/O ports, tested individually for each player.
- [x] **INP-02**: Simultaneous inputs (multi-key, disc+key, side combinations, opposite directions, ambiguous electrical combinations) produce a documented, tested result under the selected electrical model.
- [x] **INP-03**: Core input is a machine-level controller representation (form per D6) with no host mapping, remapping, overlay or device discovery inside the core.

### Video — STIC (VID)

- [ ] **VID-01**: STIC register access, display-enable handshake and VBLANK access windows follow profile timing constants, with blocked accesses following the profile's documented policy.
- [ ] **VID-02**: The VBLANK interrupt and BUSRQ/BUSAK bus-steal events occur at profile-defined machine times and their effect on CPU progress is tested.
- [ ] **VID-03**: The core produces a raw palette-index framebuffer at a defined machine-time boundary with profile-defined geometry (D8); palette, scaling and presentation remain host policy.
- [ ] **VID-04**: Foreground/Background and Color Stack card modes render hand-derived expected index frames from known BACKTAB/GRAM fixture contents (no dependency on a real GROM image in CI).
- [ ] **VID-05**: MOBs, border, horizontal/vertical delay and collision registers are each covered by a single-feature fixture with an independently derived expected result, or explicitly listed as unknown-default-chosen.
- [x] **VID-06**: A side-effect-free inspection path exists so debuggers/tests can read STIC/device state without triggering read side effects.

### Visible Scenario (SCN)

- [ ] **SCN-01**: An original, rights-clear program runs in CI through the installed consumer and reaches a guest-state progress predicate (not a title screen or hash alone), showing controller input changing guest state, video output and audio output.
- [ ] **SCN-02**: The owner-local route for running user-supplied EXEC/GROM images is documented with identity checks, labelled non-redistributable and kept outside CI (gated on D11).

### State and Performance (STA / PRF)

- [ ] **STA-01**: A canonical, endianness-independent state serializer is used for both snapshots and digests; snapshots carry format version, profile identity and system-image digests, and restore validates before mutating.
- [ ] **STA-02**: Snapshot→restore→run equals uninterrupted run at hard points (mid-frame, mid-SDBD, mid-bus-steal), covering a written hidden-state checklist.
- [ ] **PRF-01**: Named deterministic workloads have a benchmark runner that asserts the correct output digest before reporting throughput; throughput, end-to-end latency and CI time are reported separately.

### Milestone and Release Practice (REL)

- [ ] **REL-01**: Each milestone close records main green, open-PR triage, updated rolling roadmap and lessons, and a release-readiness assessment for the exact tested commit/artifact (rights, package contents, qualification, authority); publication itself remains an owner gate (D9).

## v2 Requirements

Deferred. Tracked, not in the current roadmap. Each becomes its own profile/slice with its own research and evidence.

### Variant Profiles

- **VAR-01**: Intellivision II profile (EXEC2 identity, extra decles at $0400, lockout behavior).
- **VAR-02**: Sears Super Video Arcade profile (alias vs distinct profile decided by image comparison).
- **VAR-03**: Tandyvision One / GTE Sylvania aliases after dump comparison.
- **VAR-04**: PAL timing profile (all PAL values currently LOW/unknown).

### Content Formats

- **FMT-01**: `.bin + .cfg` import and Mattel-style bank switching.
- **FMT-02**: Intellicart / CC3 / JLP / LTO Flash formats and their persistence.
- **FMT-03**: Hash-keyed content database for configuration identity.

### Peripherals

- **PER-01**: ECS (second PSG, banked ROM, extra controllers, keyboard).
- **PER-02**: IntelliVoice (SP0256 speech, separate audio stream, user-supplied voice ROM).

### Tooling and Integration

- **TOOL-01**: Input record/replay at machine-time boundaries.
- **TOOL-02**: Interactive debugger (breakpoints, cycle-stamped trace, disassembly).
- **TOOL-03**: LibFuturo adapter after a real consumer exists and fit is proven (D13).
- **TOOL-04**: Reference host frontend (window, audio device, physical controller mapping), outside the core build graph.
- **TOOL-05**: Scheduled fuzzing and extended sanitizer jobs once a failure class justifies them.
- **TOOL-06**: Hardware-measured timing evidence for STIC windows, BUSRQ and controller matrix (D10).

## Out of Scope

| Feature | Reason |
|---------|--------|
| Bundling, downloading or caching EXEC, GROM, ECS/IntelliVoice ROMs or MTE-201 | Rights-bearing; no documented permission; hash ≠ rights |
| High-level-emulated or "replacement" EXEC for commercial games | Misrepresents EXEC behavior; project boot stub is for original fixtures only and is labelled "not an EXEC" |
| Copying code/tables/constants from jzIntv, MAME or other emulators | GPL/per-file licensing and not an independent oracle |
| Unqualified compatibility labels or percentages | Contradicts Core Value |
| Overlays, remapping, accessibility, netplay, rewind, device discovery inside the core | Host-layer responsibilities |
| Keyboard Component, TutorVision, FujiNet mailbox, ECS UART | Near-zero content or prototype-only; existence in another emulator ≠ required support |
| Silent peripheral fallbacks or default memory maps | Hide configuration identity; always explicit refusal instead |
| Using "Intellivision" in a package or product name | Trademark held by a third party with a live product; naming is an owner gate |

## Decision Gates

Open decisions referenced by requirements. None are decided by this document.

| # | Decision | Status | Needed before |
|---|----------|--------|---------------|
| D1 | Implementation language/build (C17 + CMake recommended; Rust alternative) | Delegated decision 2026-11-09 (PROJECT.md) | First code phase |
| D2 | CPU-cycle unit, time base, crystal constant | Delegated decision 2026-11-09 (PROJECT.md) | CPU phase |
| D3 | First profile: 2609 NTSC recommended; revision, STIC/PSG parts, power-on RAM policy | Delegated decision 2026-11-09 (PROJECT.md) | Foundations exit |
| D4 | Fixture assembler policy (pinned dev-only `as1600` + cross-check vs project mini-assembler) | Owner (license) + architecture — open | Runner phase |
| D5 | System-image policy: user-supplied EXEC/GROM only; project boot stub for CI, labelled "not an EXEC" | Decided in Phase 3 (D-24, delegated) | Loader / runner phases |
| D6 | Controller core-input representation (logical lossless vs electrical per port) | Owner + architecture — open (conflict C14) | Core API skeleton |
| D7 | Reference PCM location (core observation layer vs host) | Architecture — open | Sound phase |
| D8 | Raw framebuffer geometry (159 vs 160 visible, nominal grid) | Architecture + evidence — open | Video phase |
| D9 | License, repository visibility, naming/trademark, publication | Delegated decision 2026-11-09 (PROJECT.md) | Any external publication |
| D10 | Physical hardware measurement | Owner gate | Any measured-tier claim |
| D11 | Owner-local real EXEC/GROM runs | Owner gate | SCN-02 |
| D12 | CI shape (job count, sanitizers, fuzz schedule) | Architecture — open | Runner/CI phase |
| D13 | LibFuturo adapter / shared-contract change | Owner gate | Not before a real consumer |

## Traceability

Which phases cover which requirements. Updated during roadmap creation.

| Requirement | Phase | Status |
|-------------|-------|--------|
| FND-01 | Phase 1 | Complete |
| FND-02 | Phase 1 | Complete |
| FND-03 | Phase 1 | Complete |
| FND-04 | Phase 1 | Complete |
| FND-05 | Phase 1 | Complete |
| FND-06 | Phase 1 | Complete |
| FND-07 | Phase 1 | Complete |
| CPU-01 | Phase 2 | Complete |
| CPU-02 | Phase 2 | Complete |
| CPU-03 | Phase 2 | Complete |
| CPU-04 | Phase 2 | Complete |
| CPU-05 | Phase 2 | Complete |
| MEM-01 | Phase 3 | Complete |
| MEM-02 | Phase 3 | Complete |
| MEM-03 | Phase 3 | Complete |
| MEM-04 | Phase 3 | Complete |
| MEM-05 | Phase 3 | Complete |
| API-01 | Phase 3 | Complete |
| API-02 | Phase 4 | Complete |
| API-03 | Phase 5 | Complete |
| API-04 | Phase 4 | Complete |
| API-05 | Phase 2 | Complete |
| API-06 | Phase 3 | Complete |
| RUN-01 | Phase 4 | Complete |
| RUN-02 | Phase 4 | Complete |
| RUN-03 | Phase 2 | Complete |
| RUN-04 | Phase 4 | Complete |
| RUN-05 | Phase 4 | Complete |
| RUN-06 | Phase 4 | Complete |
| PKG-01 | Phase 4 | Complete |
| PKG-02 | Phase 2 | Complete |
| PKG-03 | Phase 4 | Complete |
| PKG-04 | Phase 4 | Complete |
| AUD-01 | Phase 5 | Complete |
| AUD-02 | Phase 5 | Complete |
| INP-01 | Phase 5 | Complete |
| INP-02 | Phase 5 | Complete |
| INP-03 | Phase 5 | Complete |
| VID-01 | Phase 6 | Pending |
| VID-02 | Phase 6 | Pending |
| VID-03 | Phase 6 | Pending |
| VID-04 | Phase 6 | Pending |
| VID-05 | Phase 8 | Pending |
| VID-06 | Phase 6 | Complete |
| SCN-01 | Phase 7 | Pending |
| SCN-02 | Phase 7 | Pending |
| STA-01 | Phase 8 | Pending |
| STA-02 | Phase 8 | Pending |
| PRF-01 | Phase 8 | Pending |
| REL-01 | Phase 8 | Pending |

**Phase 5 close-out note (plan 05-12):** INP-01, INP-02, INP-03, API-03, AUD-01 and AUD-02 are marked complete because the gate evidence in `.planning/phases/05-hand-controllers-and-psg-sound/05-VERIFICATION.md` exists, and only on these terms. Every claim behind them (CL-0009 to CL-0015) is `provisional` and none is at the `measured-hardware` tier (D10): the controller byte table is a tier 4 community literal (C16) with a blind second read that agrees on 31 of 31 rows, the PSG rules are read from AY-3-8910 pages and applied to an AY-3-8914-class part by inference (C11, C69), and the electrical combinations (C63) and port edge cases (C64 to C66) are labelled defaults. Owner hardware items (32 listed in `data/expected/d10-measurements.ids`, D10 and D11) are handed off and not performed; no listening handoff exists because no Phase 5 artifact has a listenable tone (D-22, deferred).

**Coverage:**
- v1 requirements: 50 total
- Mapped to phases: 50
- Unmapped: 0 ✓

---
*Requirements defined: 2026-11-09*
*Last updated: 2026-11-12 after Phase 5 close-out (plan 05-12)*
