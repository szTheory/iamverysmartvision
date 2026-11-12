# Roadmap: iamverysmartvision

## Overview

The project goes from an empty repository to a deterministic, headless
Intellivision machine core. Every compatibility claim in that core cites a named
profile and an evidence tier. The order is policy first, then the CP-1610, then
loading and refusal, then an end-to-end Slice 0: an original diagnostic run
through the headless runner and an installed consumer. Controllers and PSG come
next, then a first STIC pass, then one visible original scenario. The milestone
ends with the remaining STIC features, canonical save states and
correctness-asserted performance baselines.

**This is a rolling roadmap.** Phases 1-2 are detailed enough to plan. Later
phases are working outlines: they will be re-planned at milestone and phase
discussion against current evidence and owner decisions (AGENTS.md: inspect
state, fan out by lenses, adversarial pass, keep a rolling roadmap).
**GSD milestones and software versions are separate.** Finishing this milestone
does not mean a release. A release covers only the exact tested artifact, and
only when rights, package readiness, qualification and owner authority allow it
(REL-01, D9).

**Decision status.** The Decision Gates (D1-D13) are defined in
`.planning/REQUIREMENTS.md`, and none is decided by this roadmap. Where a phase
mentions C17/CMake or the 2609 NTSC Master Component, those are research
**recommendations**, not owner decisions. The implementation language (D1) and
the first profile (D3) are still **open owner decisions**.

**Granularity note.** The config asks for `standard` granularity, which usually
means 4-6 phases. This roadmap has 8. It keeps the research's dependency order
because the work spans five hardware subsystems with distinct evidence problems,
and merging them would create phases that mix several high-uncertainty chips
(PIT-18: one chip per phase). Variant profiles (research Phase 9) are v2 and are
not in this roadmap.

## Phases

**Phase Numbering:**
- Integer phases (1, 2, 3): Planned milestone work
- Decimal phases (2.1, 2.2): Urgent insertions (marked with INSERTED)

Decimal phases appear between their surrounding integers in numeric order.

- [x] **Phase 1: Evidence, Profile and Rights Foundations** - Tier vocabulary, claim ledger, discrepancy log, fixture manifest, provenance rule, first-profile record, time base and CP-1610 reference notes (completed 2026-11-09)
- [x] **Phase 2: CP-1610 CPU Core** - Every opcode verified against page-cited, hand-derived vectors in a host-independent core with an exact inventory and one local gate command (completed 2026-11-10)
- [x] **Phase 3: Memory Map, Content Loading and Refusal** - Profile-driven memory map, typed system-image and cartridge loading, isolated instances and a refusal matrix that never mutates prior state (completed 2026-11-11)
- [x] **Phase 4: Headless Runner and Installed Consumer (Slice 0)** - An original diagnostic runs end to end through the runner and a relocated installed consumer, with deterministic digests and a CI gate that mirrors the local command (completed 2026-11-11)
- [x] **Phase 5: Hand Controllers and PSG Sound** - Both controllers as multi-signal machine input at explicit time boundaries; PSG raw audio and a register-write log (completed 2026-11-12)
- [ ] **Phase 6: STIC Video First Pass** - Access windows, VBLANK/BUSRQ timing, raw index framebuffer, FG/BG and Color Stack cards, and side-effect-free inspection
- [ ] **Phase 7: First Visible Original Scenario** - A rights-clear original program responds to controller input with video and sound through the installed consumer in CI
- [ ] **Phase 8: STIC Completion, Save State and Performance Baselines** - Remaining STIC features, canonical snapshots, correctness-asserted benchmarks and the milestone-close record

## Decision Gate Timeline

None of these are decided. "Before" means the phase cannot start, or cannot
exit, until the gate is resolved by its named authority.

| Gate | Authority | Status | Must be resolved |
|------|-----------|--------|------------------|
| D1 Language/build (C17 + CMake recommended; Rust alternative) | Owner | Delegated decision 2026-11-09 (PROJECT.md) | Before Phase 2 starts |
| D2 CPU-cycle unit, time base, crystal constant | Architecture | Delegated decision 2026-11-09 (PROJECT.md) | During Phase 1 (FND-07); before Phase 2 |
| D3 First profile (2609 NTSC recommended; revision, STIC/PSG parts, power-on policy) | Owner | Delegated decision 2026-11-09 (PROJECT.md) | Before Phase 1 exits |
| D4 Fixture assembler policy | Owner (licence) + architecture | Delegated decision 2026-11-11 (PROJECT.md; Phase 4 D-25, D-12, D-41) | Before Phase 4 starts |
| D5 System-image policy and project boot stub | Owner | Delegated decision 2026-11-11 (PROJECT.md; Phase 3 D-24) | Phase 3 (refusal); before Phase 4 (boot) |
| D6 Controller core-input representation (C14) | Owner + architecture | Delegated decision 2026-11-11 (PROJECT.md; Phase 4 D-25, D-12, D-41) | While Phase 4 shapes the API; before Phase 5 starts |
| D7 Reference PCM location (C23) | Architecture | Delegated decision 2026-11-12 (PROJECT.md; Phase 5 D-21: outside the core, `tools/pcm`, runner `--audio-wav` only) | Phase 5 |
| D8 Raw framebuffer geometry (C10) | Architecture + evidence | Delegated decision 2026-11-12 (PROJECT.md; Phase 6 D-12: 159 x 96, width from `video.last_card_column_px`, reversible by one profile field) | Phase 6 |
| D9 License, visibility, naming/trademark, publication | Owner gate | Delegated decision 2026-11-09 (PROJECT.md) | Before any external publication; CI hosting (Phase 4) and the milestone release assessment (Phase 8) touch it |
| D10 Physical hardware measurement | Owner gate | Open | Before any measured-tier claim (Phase 5 onward) |
| D11 Owner-local real EXEC/GROM runs | Owner gate | Open | Before SCN-02 can be claimed (Phase 7) |
| D12 CI shape (C24) | Architecture | Delegated decision 2026-11-11 (PROJECT.md; Phase 4 D-25, D-12, D-41) | Before Phase 4 exits |
| D13 LibFuturo adapter / shared-contract change | Owner gate | Deferred | Not before a real consumer exists; outside this milestone |

## Phase Details

### Phase 1: Evidence, Profile and Rights Foundations
**Goal**: Before any machine code exists, every later claim has a defined place to cite a named profile, an evidence tier, a provenance statement and a rights status. The CP-1610 oracle also exists as project-owned, page-cited notes.
**Mode:** mvp
**Depends on**: Nothing (first phase)
**Requirements**: FND-01, FND-02, FND-03, FND-04, FND-05, FND-06, FND-07
**Decision gates**: D3 (owner, open). This must be resolved before the phase exits, because FND-01 cannot be completed for an unselected profile. 2609 NTSC is recommended but not decided. D2 (architecture, open) is resolved inside this phase through FND-07. D1 (owner, open) is recorded here and must be resolved before Phase 2 starts. D9 (owner gate) is recorded, and until it is decided `redistributable` defaults to unknown, treated as no.
**Research flags**: NEEDS TARGETED RESEARCH, time-boxed per PIT-18. Read the GI CP-1600 manual (May 1975) PDF directly, not the OCR transcription, for opcode, cycle, reset/MSYNC, stack direction (C17) and interruptibility (C19). Extract GI 1980/1982 catalog data for the STIC -1 part and the PSG part identity (C11). Search for an AY-3-8914 datasheet. Correct the known bootstrap source defects (404 owner manual, iFixit HTML viewer).
**Success Criteria** (what must be TRUE):
  1. A reviewer can open the first-profile record and find model, revision, region/video timing, STIC part, PSG part, reset/power-on policy and cartridge arrangement. Every field has a provenance/tier tag, unknown fields say `unknown-default-chosen` instead of being blank, and the record cites the owner decision (D3) that selected the profile.
  2. The evidence-tier vocabulary, claim ledger and discrepancy log exist. The log has one entry for each of C1-C28, each with a default policy and the test or measurement that would resolve it. A spot check of project docs finds no support claim without a profile identity, tier and evidence artifact.
  3. The fixture-manifest schema (source, recipe/tool+version, digests, license, notices, allowed use, `redistributable`) and the no-copy provenance rule are in the review checklist. A sample `redistributable: unknown` entry is shown to be treated as non-redistributable.
  4. The CP-1610 reference notes give opcode encodings, cycle counts and the reset/interrupt sequence, each with a page citation to the directly read GI manual. They state whether the stack-direction reading (C17) is confirmed or still a logged conflict.
  5. A time-base glossary separates CPU cycle, input clock, STIC/PSG clocks and the crystal constant. D2 is recorded as an architecture decision with its rationale, for later code and docs to follow.
**Plans:** 14/14 plans complete in 5 waves (walking skeleton: `.planning/phases/01-evidence-profile-and-rights-foundations/SKELETON.md`)

Plans:
**Wave 1**
- [x] 01-01-PLAN.md — Walking skeleton: `sh tools/gate.sh`, generic record checker, profile record + tier vocabularies, control runner, CI workflow (wave 1)
- [x] 01-02-PLAN.md — CP-1610 reference notes from GI manual page images; refs/ cache owner; image-derived C17 status (wave 1)
- [x] 01-03-PLAN.md — CP-1610 opcode family table transcription with throwaway 1024-pattern decode (wave 1)
- [x] 01-14-PLAN.md — Reference-document registry `data/sources.json` (wave 1)

**Wave 2** *(blocked on Wave 1 completion)*
- [x] 01-04-PLAN.md — Discrepancy log C1-C28 + C29+, checked, profile conflicts reconciled (wave 2)
- [x] 01-05-PLAN.md — Evidence-tiers doc, claim ledger, claim-language tripwire (wave 2)
- [x] 01-06-PLAN.md — Fixture-manifest schema (fail-closed), rights policy, tracked-file rights checks (wave 2)
- [x] 01-07-PLAN.md — Provenance rule + checklist, consultation log, denylist, source-registry check (wave 2)
- [x] 01-08-PLAN.md — Time-base glossary + D2, glossary step, decision-record cross-links (wave 2)
- [x] 01-13-PLAN.md — CI-parity step and control-runner self-test (wave 2)

**Wave 3** *(blocked on Wave 2 completion)*
- [x] 01-09-PLAN.md — CP-1610 table checker: row schema, 1024-pattern coverage, C17 notes/log consistency, CL-0005 (wave 3)
- [x] 01-11-PLAN.md — Hygiene check (tree step + history tool), path scrub, README, LICENSE, clean-clone rehearsal (wave 3)

**Wave 4** *(blocked on Wave 3 completion)*
- [x] 01-10-PLAN.md — Blind second-reader pass over the family table and the C17 stack pages (wave 4)

**Wave 5** *(blocked on Wave 4 completion)*
- [x] 01-12-PLAN.md — History rewrite, public repo via gh, first CI run, CI-EFFICIENCY.md (wave 5)

### Phase 2: CP-1610 CPU Core
**Goal**: Every opcode in the inventory produces results that match hand-derived, page-cited expectations. The CP-1610 runs inside a host-independent core library, checked by an exact test inventory and one documented local gate command.
**Mode:** mvp
**Depends on**: Phase 1
**Requirements**: CPU-01, CPU-02, CPU-03, CPU-04, CPU-05, API-05, PKG-02, RUN-03
**Decision gates**: D1 (owner, open) must be resolved before this phase starts. No language or build system is assumed (C17 + CMake recommended, Rust alternative). D2 must already be resolved in Phase 1.
**Research flags**: NEEDS TARGETED RESEARCH. Needed: the branch taken/not-taken cycle column, SDBD modes 0 and 6 (named policy), the console's interrupt-acknowledge mechanism for supplying $1004 (ARCHITECTURE unknown), and per-opcode interruptibility (C19). libasm test vectors are leads only, used after a licence and provenance check. Nothing from jzIntv or MAME is copied (FND-05).
**Success Criteria** (what must be TRUE):
  1. The documented local command runs the full opcode-level inventory and reports executed, skipped and missing counts. A deliberate omission control shows that a missing, duplicated, skipped or stale required test makes it fail.
  2. Each opcode vector checks register, memory, flag and cycle results against hand-derived values that cite a manual page (tier: source expectation). A deliberately broken opcode (named-fault control) fails its vectors.
  3. Interrupt and BUSRQ acceptance cases (previous-instruction interruptibility, I flag, D/SDBD, BUSRQ), SDBD modes (with a documented policy for unsupported modes), stack direction, auto-increment registers and reset entry each pass their vectors, and each has a named-fault control that fails for the intended reason.
  4. For any instruction, the stepper reports an ordered, cycle-stamped list of bus accesses that a test compares against the manual's bus sequence.
  5. A build/link or symbol check fails when the core library references host clocks, filesystem, environment, threads or I/O. A deliberate-violation control demonstrates this.
**Plans**: 18/18 plans complete

Plans:
- [x] 02-01-PLAN.md — Gate hardening (IN-04, IN-02, denylist status 2, SPDX/ASCII for C), build/gate NONCE contract, discrepancy entries C36-C58 (G8 = C58) and five profile fields
- [x] 02-02-PLAN.md — Walking skeleton A: CMake/presets, internal CPU API (DW access kind, one wait rule) with reset entry and CLRC/SETC, harness, JSON-to-C generator, control vectors, steps c-build, c-tests, tests-inventory with 12 controls
- [x] 02-03-PLAN.md — Walking skeleton B: cpu-api contract suite and harness self-test suite
- [x] 02-04-PLAN.md — Walking skeleton C: named-fault path (c-faults, faults-verdict, 9 controls) and the structural cp1610-vectors checker (17 controls)
- [x] 02-05-PLAN.md — Coverage derivation: flags, cycles, policies, addressing classes, exact branch directions and BEXT E codes, modifier witness, per-row bus list (15 controls)
- [x] 02-06-PLAN.md — Control group complete (HLT, SDBD, EIS, DIS, TCI, GSWD, NOP, SIN, RSWD) with vectors and four faults
- [x] 02-07-PLAN.md — Purity (API-05): core-symbols allowlist check, c-purity step, 13 deliberate-violation controls, core-symbols ctest suite
- [x] 02-08-PLAN.md — Complete 74-row decode table and the 14 register-register rows with vectors and faults
- [x] 02-09-PLAN.md — Immediate rows and shift/SWAP rows with vectors and faults
- [x] 02-10-PLAN.md — Direct and indirect memory rows: stack direction, auto-increment, ADAR, POL-STK-02, POL-MVO-01, MVI@ MMM=6 witnessed by PULR
- [x] 02-11-PLAN.md — B, the 14 conditional branches and NOPP in four directions (policy-carried 7/9 cycles)
- [x] 02-12-PLAN.md — BEXT with two E codes via the provider, and the J/JSR family
- [x] 02-13-PLAN.md — SDBD supported modes and refusal (12 targets), POL-ILL-01 encodings, 1024-decle sweep against the Phase 1 table
- [x] 02-14-PLAN.md — Interrupt acceptance model and entry sequence (DW kind), IAB-supplied vector, 17 non-interruptible rows, SDBD deferral, reset interaction
- [x] 02-15-PLAN.md — BUSRQ grant/release, wait states (no wait on INTAK/IAB/DW) and cap, HLT/STPST/HALTED, seeded fuzz suite, stepper conventions doc
- [x] 02-16-PLAN.md — Blind-review tooling per A-12 (index-blinded, leak-checked copies, diff tool, vector-review kind with committed handoff check), reader protocol and policy document
- [x] 02-17-PLAN.md — Non-autonomous: orchestrator checkpoint spawns 12 isolated readers, then adjudicate every vector file from the page and commit the handoff record
- [x] 02-18-PLAN.md — Reference-note updates, D-22 gate budget and CI-EFFICIENCY record, SC1-SC5 evidence check

### Phase 3: Memory Map, Content Loading and Refusal
**Goal**: A host can create isolated machine instances and load typed system images and cartridge content against the profile's memory map. Every bad input gets a typed refusal that leaves prior machine state unchanged.
**Mode:** mvp
**Depends on**: Phase 2
**Requirements**: MEM-01, MEM-02, MEM-03, MEM-04, MEM-05, API-01, API-06
**Decision gates**: D3 must be resolved, because the memory map comes from the selected profile. D5 (decided in Phase 3, D-24: user-supplied system images only, plus a project boot stub authored in Phase 4) sets the system-image supply policy. The cartridge import format was decided in Phase 3 (D-29): the project container IVSC v1; `.rom` and `.bin+.cfg` importers are deferred and need their own rights review.
**Research flags**: NEEDS RESEARCH on the cartridge import format and its rights, upper-bit behavior of 10-bit ROM reads (unknown), and GRAM/STIC decode ranges (C21). STANDARD PATTERNS for the refusal matrix and descriptor tables once the format is chosen.
**Success Criteria** (what must be TRUE):
  1. A decode-table dump generated from the profile descriptors matches the sourced literal memory map. Decle (10-bit), 8-bit and 16-bit width masking and store truncation pass tests at each region boundary.
  2. A caller loads EXEC, GROM and cartridge segments as separate typed inputs and gets back each image's length and digest. Tests use only project-authored stand-in bytes, and no real system image is committed, cached or required.
  3. Every refusal-matrix row (malformed, oversized, overlapping, out-of-range, missing system image, unknown profile) returns its own typed error, and the state digest is the same before and after the refused load.
  4. The cartridge import format is specified in the project's own words. Its positive, boundary and malformed fixtures pass and are recorded in the fixture manifest.
  5. Two instances created side by side, with different content, step without affecting each other (isolation test). Every API error and refusal has a documented state effect that a test verifies.
**Plans**: 14/14 plans complete in 14 sequential waves

Plans:
- [x] 03-01-PLAN.md — Fault loop over every `data/expected/faults/*.ids` with a per-suite registry and union site checks (D-37), 4 controls
- [x] 03-02-PLAN.md — In-repo SHA-256 (FIPS 180-4, copyable context) with an always-on CMake oracle, suite `machine`, 6 faults, first gate measurement
- [x] 03-03-PLAN.md — Public include tree (`ivs_status.h` moved), internal include split, c-purity for `core/include` and reviewed generated includes, writable-data rule, 6 controls
- [x] 03-04-PLAN.md — Profile memmap descriptors, fail-closed generator, memory system and bus in one core TU, C60-C62, suite `memmap`, 6 faults
- [x] 03-05-PLAN.md — Sourced literal memory map and the `memmap` gate step (grammar, tiling, provenance), 8 controls
- [x] 03-06-PLAN.md — Decode dump over all addresses compared with the literal, generated boundary probes (MEM-02), 5 faults, 3 controls, CL-0006
- [x] 03-07-PLAN.md — IVSC v1 spec, pure parser, parse-level text fixtures, suite `ivsc`, manifest `FX-ivsc-vectors` and refresh helper
- [x] 03-08-PLAN.md — Seeded IVSC fuzzing, 6 parser faults, optional libFuzzer recipe, `.ivsc` hygiene, CL-0007
- [x] 03-09-PLAN.md — Public machine headers, caller-storage create/destroy/info, status names and pin, canonical state digest, 8 faults
- [x] 03-10-PLAN.md — Typed load/unload of EXEC, GROM and cartridge with validate-then-commit, reports and the two-digest pass, 3 faults
- [x] 03-11-PLAN.md — Refusal matrix over prior states, precedence rows, layer-2 cartridge fixtures, 8 ordering faults, second gate measurement
- [x] 03-12-PLAN.md — Power-on reset with missing-system-image refusal, internal step seam, isolation tests, sweeps, headers suite, 4 faults
- [x] 03-13-PLAN.md — API effect table and the `api-effects` gate step, 8 controls
- [x] 03-14-PLAN.md — SC1-SC5 evidence table, record reconciliation, D-38 budget re-baseline, honest caveats

### Phase 4: Headless Runner and Installed Consumer (Slice 0)
**Goal**: An original, project-authored diagnostic runs from reset through the public API, the headless runner and an independently built installed consumer. Digests are deterministic, failures are reported honestly by kind, and the CI gate mirrors the local command.
**Mode:** mvp
**Depends on**: Phase 3
**Requirements**: API-02, API-04, RUN-01, RUN-02, RUN-04, RUN-05, RUN-06, PKG-01, PKG-03, PKG-04
**Decision gates**: D4 (owner licence + architecture, open) must be resolved before the first guest fixture is built. D5 is decided (Phase 3, D-24: user-supplied images only; the project boot stub is authored in this phase and labelled "not an EXEC", C25, C27). D12 (architecture, open) decides CI shape. D6 (owner + architecture, open, C14) should be settled while this phase shapes the public API, and must be settled before Phase 5. D9 (owner gate): hosted CI needs a repository host and a visibility choice, and the phase must not assume public visibility.
**Research flags**: STANDARD PATTERNS for install/export packaging, the relocated consumer and the runner, in whatever build system D1 selects. NEEDS RESEARCH on the EXEC-less reset story (open question 3) and the boot-stub contract.
**Success Criteria** (what must be TRUE):
  1. A scripted scenario (content, profile, input schedule, run bound) on fresh state reports machine time, stop reason, raw outputs and guest-state assertions. The original diagnostic reaches a guest-state predicate whose expected value was derived independently of the emulator (tier: original diagnostic), and its boot path is labelled "project boot stub, not an EXEC".
  2. Harness failure, guest failure, timeout, cancellation and missing fixture each give a distinct runner outcome, with one scenario demonstrating each. Every important gate has a named-fault control that fails for its intended reason.
  3. A run to an absolute cycle target reports any overshoot. Splitting the run into slices gives the same per-domain digests (CPU/memory, video, audio, input log), and repeated runs match. Golden files carry tier and oracle headers. An undersized output buffer is refused before any state changes.
  4. A separate consumer project, built only against the installed package in a relocated prefix, runs load, refusal, run and digest successfully.
  5. CI runs the same fast gate as the local command on every PR, shows skipped and missing items, never caches or uploads system images or rights-unknown fixtures, and records queue, setup, build and test durations for each run.
**Plans**: 27/25 plans complete in 25 sequential waves

Plans:
- [x] 04-01-PLAN.md — CI measurement first: `gate-env`/`gate-cpu` lines, job summary through the EXIT trap with a selftest step, ci-parity bans (action allowlist, secrets, matrix, runs-on, timeout 5), 6 controls
- [x] 04-02-PLAN.md — Run record `.planning/ci-runs.tsv`, `ci-log` step, orchestrator-only `tools/ci-record.sh`, D-45 budget rule, 4 controls
- [x] 04-03-PLAN.md — Fault framework generalized to `runner/` owners (D-24) with the runner outcome taxonomy as tracer, suite `runner`, 4 faults, 2 controls
- [x] 04-04-PLAN.md — Runner source hygiene: `runner-scan` (POSIX only in hostio.c), integrity for `runner/` and `tools/**/*.c`, unlisted-source guard, 6 controls
- [x] 04-05-PLAN.md — Stepped-state composite pin first, then observation rows and live digest domains 16-18 (composite unchanged), 2 faults
- [x] 04-06-PLAN.md — Public read-only observation (`ivs_run.h`, `ivs_machine_cpu_state`, `ivs_machine_peek`), new suite `run`, 2 faults
- [x] 04-07-PLAN.md — `ivs_machine_run`: absolute master-tick target, overshoot, stop reasons, refusal ladder, 3 faults
- [x] 04-08-PLAN.md — Generic define pins for stop reasons and refusal reasons, 3 controls
- [x] 04-09-PLAN.md — Timebase and output layout in `ivs_info` (A-02 growth rule), five-pattern slicing invariance, 2 faults
- [x] 04-10-PLAN.md — Output buffers (API-04) through an internal env seam, refusal before any state change, 2 faults
- [x] 04-11-PLAN.md — Input events (D6 logical switch state) in the run call, D-05 event-argument checks, live INPUT digest chain with an independent oracle, 6 faults
- [x] 04-12-PLAN.md — Profile `input.controllers`, extra-word refusal, guest invisibility of input in Phase 4
- [x] 04-13-PLAN.md — Input bit names pinned, D6 records (C14, C15, C63-C67), seeded input/slicing fuzz, v1 struct sizes, 2 controls
- [x] 04-14-PLAN.md — Project boot stub listing, `tools/pack` (build-tree images only), boot path and trap tests, `docs/diagnostics.md`
- [x] 04-15-PLAN.md — `listing` step with stub anti-HLE guards and suite `pack` (two independent parses), 9 controls
- [x] 04-16-PLAN.md — Original diagnostic `ivs-diag-cpu-1`, hand-derived `.expect` predicate, slicing on the real program, CL-0008
- [x] 04-17-PLAN.md — IVSS v1 parser (untrusted input, caps, fuzz), 2 faults
- [x] 04-18-PLAN.md — Runner executor and fixed-order report with pinned key names, outcomes in-process, 4 faults
- [x] 04-19-PLAN.md — `ivs-run` executable: slices, budget/wall timeout, cancellation, POSIX in hostio.c only, `docs/runner.md`, 3 faults
- [x] 04-20-PLAN.md — Suite `runner-process`: one scenario per outcome, diagnostic mutants, repeat/slice/cwd checks, judge self-controls
- [x] 04-21-PLAN.md — Goldens with tier/oracle headers, `--update-golden`, pure `golden` step with dev-vs-release judge excluding host-dependent scenarios, 10 controls
- [x] 04-22-PLAN.md — Install/export of `ivs::core`, relocated prefix, `pkg-install` structure proofs, 8 in-step selftests
- [x] 04-23-PLAN.md — Independent installed consumer: find_package in the moved prefix, runner-golden agreement, 10/10 selftests, `docs/packaging.md`
- [x] 04-24-PLAN.md — RUN-04 important-gates registry and step, gate audit and gap fills, 2 controls
- [x] 04-25-PLAN.md — SC1-SC5 evidence table, fail-first record verification, D-45 evaluation, branch-protection and release-design records, honest caveats

### Phase 5: Hand Controllers and PSG Sound
**Goal**: Both hand controllers are first-class, multi-signal machine input, read through the PSG I/O ports at an explicit machine-time boundary. The PSG produces raw, cycle-stamped audio observations kept separate from host PCM.
**Mode:** mvp
**Depends on**: Phase 4
**Requirements**: INP-01, INP-02, INP-03, API-03, AUD-01, AUD-02
**Decision gates**: D6 (owner + architecture, open, C14) must be resolved before this phase starts. D7 (architecture, open, C23) decides where reference PCM lives. D3 must pin the PSG part (C11). D10 (owner gate): no measured-tier claim about the controller matrix without owner-approved measurement.
**Research flags**: NEEDS RESEARCH. AY-3-8914 register map and behavior: no tier-1 datasheet has been found, so any 8910-derived rule is labelled and audited rule by rule (C12). The controller electrical matrix rests on one LOW-confidence source (C16). Side-button count and coding (C15) need confirming against a schematic.
**Success Criteria** (what must be TRUE):
  1. For each player, every one of the 12 keypad keys, 16 disc directions and side-action inputs, tested individually, gives the profile's documented read value at the PSG I/O port, and each value carries a tier tag.
  2. Multi-key, disc+key, side-button combinations, opposite directions and ambiguous electrical combinations give the documented, tested result under the selected electrical model. Core input contains no host mapping, remapping, overlay or device discovery.
  3. An input script applied at declared machine-time boundaries gives the same guest-visible results and digests whether the host delivers it in one call or many slices.
  4. Each PSG register, tone, noise and envelope rule has a passing vector. Behaviors without a primary source (period 0, envelope select, noise LFSR, DAC levels) appear as tier-tagged profile data, not hard-coded constants.
  5. A run emits a cycle-stamped PSG register-write log and integer chip-rate samples as raw audio. Any PCM conversion sits where D7 puts it and is outside the raw-audio evidence path.
**Plans**: 12/12 plans complete in 12 sequential waves (all Phase 5 claims are provisional; owner hardware items are handed off, not performed)

Plans:
- [x] 05-01-PLAN.md — Oracle-first plan (D-23, A-12)
- [x] 05-02-PLAN.md — Behaviour-neutral ABI growth (A-12)
- [x] 05-03-PLAN.md — State and digest foundation (A-12)
- [x] 05-04-PLAN.md — PSG register file, memmap flip and the new suite `psg` (A-12)
- [x] 05-05-PLAN.md — Input byte and port reads (A-12)
- [x] 05-06-PLAN.md — Tone, mixer, amplitude, sampling and audio live (A-12)
- [x] 05-07-PLAN.md — Noise and envelope (A-12)
- [x] 05-08-PLAN.md — Write log (A-12)
- [x] 05-09-PLAN.md — Runner and scenarios (A-12)
- [x] 05-10-PLAN.md — Reference PCM converter and WAV helper (A-12)
- [x] 05-11-PLAN.md — Fixture and end to end (A-12)
- [x] 05-12-PLAN.md — Close-out: claims CL-0009..CL-0013, discrepancy states, important-gate registry for the six requirements, traceability, Decision Register notes, CI close-out, `05-VERIFICATION.md` with three tables (A-12)
- [x] 05-12-PLAN.md — Close-out (A-12)

### Phase 6: STIC Video First Pass
**Goal**: The STIC control path, access windows and bus-steal timing run against profile constants. At a defined machine-time boundary, the core produces a raw palette-index framebuffer that renders background cards in Foreground/Background and Color Stack modes, checked against hand-derived expected frames.
**Mode:** mvp
**Depends on**: Phase 4 (does not need Phase 5; scheduled after it because PSG carries less uncertainty)
**Requirements**: VID-01, VID-02, VID-03, VID-04, VID-06
**Decision gates**: D8 (architecture + evidence, open, C10) decides raw framebuffer geometry (159 vs 160 visible columns, nominal grid). D3 must pin the STIC part (C11). D10 (owner gate): disputed timing (C1-C4) stays `unknown-default-chosen` unless the owner authorizes hardware measurement.
**Research flags**: NEEDS RESEARCH, the strongest flag in this roadmap. The STIC primary sections (GI 1980 p.5-10, GI 1982 p.9-43) still need detailed extraction. Conflicts C1-C7, C10 and C21 apply, and row-granular versus frame-granular rendering (PIT-9) must be decided early.
**Success Criteria** (what must be TRUE):
  1. STIC register accesses inside the profile's access windows take effect, and accesses outside them follow the profile's blocked-access policy. The display-enable handshake works. Every timing constant used has a tier tag, and disputed values are logged as `unknown-default-chosen`.
  2. The VBLANK interrupt and BUSRQ/BUSAK bus-steal events fire at profile-defined machine times. Tests show their effect on CPU progress (stolen cycles, interrupt acceptance).
  3. At a defined machine-time boundary, the core writes a raw palette-index framebuffer with profile geometry into a caller-owned buffer. Palette, scaling and presentation stay outside the core.
  4. Foreground/Background and Color Stack fixtures render index frames that match frames hand-derived from known BACKTAB/GRAM contents, with no real GROM image in CI. A named-fault control (for example, the wrong card mode) fails.
  5. Tests and debuggers can read STIC and device state through an inspection path with no read side effects. For example, inspecting $0021 does not switch Color Stack mode.
**Plans**: 4/14 plans executed in 14 sequential waves

Plans:
- [x] 06-01-PLAN.md — Sources and oracles first (oracle plan, NO C)
- [x] 06-02-PLAN.md — Lever pass and the normalised metric (small, file-disjoint from every other plan)
- [x] 06-03-PLAN.md — ABI growth, behaviour-neutral
- [x] 06-04-PLAN.md — Profile growth and the STIC parameter table
- [ ] 06-05-PLAN.md — State and digest foundation
- [ ] 06-06-PLAN.md — STIC register file, access windows, display-enable handshake, inspection foundation and the five-region memory-map flip, with suite `stic` created, five faults and...
- [ ] 06-07-PLAN.md — Inspection call and peek purity
- [ ] 06-08-PLAN.md — Frame timeline lines
- [ ] 06-09-PLAN.md — Raw framebuffer, video buffers, domain 16 live, frame stop and the runner and consumer buffer request, in the plan that makes the video layout real
- [ ] 06-10-PLAN.md — Foreground/Background card rendering
- [ ] 06-11-PLAN.md — Color Stack rendering and inspection completion
- [ ] 06-12-PLAN.md — Runner and scenarios, `ivsr 3`
- [ ] 06-13-PLAN.md — Diagnostic and end to end
- [ ] 06-14-PLAN.md — Close-out

### Phase 7: First Visible Original Scenario
**Goal**: An original, rights-clear program visibly responds to controller input with video and sound, run in CI through the installed consumer. This shows a playable result early, so the project does not drift into research only (conflict C26: scheduled right after the first STIC pass instead of after full STIC coverage).
**Mode:** mvp
**Depends on**: Phase 5, Phase 6
**Requirements**: SCN-01, SCN-02
**Decision gates**: The scenario program and title are not chosen (open question 9; owner/discussion). A project-authored program is recommended because it avoids third-party rights. D11 (owner gate, open): SCN-02 can document the owner-local EXEC/GROM route, but no real-EXEC result is claimed unless the owner chooses to run it. D4 governs how the program is assembled. D9 applies only if third-party homebrew or publication is considered.
**Research flags**: NEEDS RESEARCH only if third-party homebrew is considered (Geese licence conflict C28; IntyBASIC output notices unresolved; 4-Tris must not be vendored). Any visible host stays outside the core build graph (host choice unknown; a reference host frontend is v2 TOOL-04).
**Success Criteria** (what must be TRUE):
  1. The program's source, recipe, digests and license are in the fixture manifest as `redistributable: yes`, and CI builds and runs it through the installed consumer.
  2. A scripted input sequence changes guest state, and the run asserts a guest-state progress predicate (a register or memory marker), not a title screen or hash alone. The tier is playable guest progress, for this original program only.
  3. The same run shows the input changing raw video (the index framebuffer differs in the expected region) and raw audio (the PSG write log and samples change).
  4. The owner-local route for user-supplied EXEC/GROM is documented with digest identity checks, labelled non-redistributable and kept out of CI. Any job that refers to it prints a visible SKIPPED line, and the inventory gate counts it.
**Plans**: TBD

### Phase 8: STIC Completion, Save State and Performance Baselines
**Goal**: The remaining STIC features are either covered or explicitly listed as unknown. Full machine state round-trips through one canonical snapshot format, named workloads have benchmarks that check correctness first, and the milestone closes with a release-readiness assessment of the exact tested artifact.
**Mode:** mvp
**Depends on**: Phase 7
**Requirements**: VID-05, STA-01, STA-02, PRF-01, REL-01
**Decision gates**: D9 (owner gate, open): license, visibility, naming and any publication. REL-01 records readiness only; it does not publish. D10 (owner gate): collision or timing claims at the measured tier need owner-approved measurement. Persistence policy (open question 8) is decided by owner and architecture. D13 stays deferred and does not enter this milestone.
**Research flags**: NEEDS RESEARCH on collision semantics, MOB priority and vertical delay. MAME marks collisions TBD, so a datasheet or measurement is needed. STANDARD PATTERNS for canonical serialization and hashing. Whether the hidden-state checklist is complete is an inference and must be checked by tests.
**Success Criteria** (what must be TRUE):
  1. MOBs, border, horizontal/vertical delay and collision registers each have a single-feature fixture with an independently derived expected result, or appear on an explicit `unknown-default-chosen` list in the discrepancy log.
  2. Snapshots carry format version, profile identity and system-image digests, and the same canonical, endianness-independent serializer produces the state digests. Restoring a corrupted, wrong-version or mismatched-profile snapshot is refused before any state changes.
  3. Snapshot, restore and run gives the same result as an uninterrupted run at mid-frame, mid-SDBD and mid-bus-steal points, and tests cover every item on the written hidden-state checklist.
  4. For each named workload, the benchmark runner reports throughput only after it has asserted the correct output digest. Throughput, end-to-end latency and CI time are reported separately.
  5. The milestone-close record shows main green, open PRs triaged, the rolling roadmap and lessons updated, and a release-readiness assessment (rights, package contents, qualification, authority) for the exact tested commit and artifact. Nothing is published without owner gate D9.
**Plans**: TBD

## Coverage

| Category | Requirements | Phase |
|----------|--------------|-------|
| Foundations | FND-01..07 | 1 |
| CPU | CPU-01..05 | 2 |
| Core contract | API-05 | 2 |
| Core contract | API-01, API-06 | 3 |
| Core contract | API-02, API-04 | 4 |
| Core contract | API-03 | 5 |
| Memory/loading | MEM-01..05 | 3 |
| Runner | RUN-03 | 2 |
| Runner | RUN-01, RUN-02, RUN-04, RUN-05, RUN-06 | 4 |
| Packaging/CI | PKG-02 | 2 |
| Packaging/CI | PKG-01, PKG-03, PKG-04 | 4 |
| Controllers | INP-01..03 | 5 |
| Sound | AUD-01, AUD-02 | 5 |
| Video | VID-01..04, VID-06 | 6 |
| Video | VID-05 | 8 |
| Scenario | SCN-01, SCN-02 | 7 |
| State/perf | STA-01, STA-02, PRF-01 | 8 |
| Release practice | REL-01 | 8 |

Mapped: 50/50 v1 requirements. No orphans, no duplicates.

## Progress

**Execution Order:**
Phases execute in numeric order: 1 → 2 → 3 → 4 → 5 → 6 → 7 → 8

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. Evidence, Profile and Rights Foundations | 14/14 | Complete    | 2026-11-09 |
| 2. CP-1610 CPU Core | 18/18 | Complete    | 2026-11-10 |
| 3. Memory Map, Content Loading and Refusal | 14/14 | Complete    | 2026-11-11 |
| 4. Headless Runner and Installed Consumer (Slice 0) | 27/25 | Complete    | 2026-11-11 |
| 5. Hand Controllers and PSG Sound | 12/12 | Complete    | 2026-11-12 |
| 6. STIC Video First Pass | 4/14 | In Progress | - |
| 7. First Visible Original Scenario | 0/TBD | Not started | - |
| 8. STIC Completion, Save State and Performance Baselines | 0/TBD | Not started | - |
