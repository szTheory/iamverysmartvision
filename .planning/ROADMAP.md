# Roadmap: iamverysmartvision

## Overview

The project is building a deterministic, headless Intellivision machine core in
which every compatibility claim cites an exactly named profile and an evidence
tier. The first milestone delivered the machine and the evidence apparatus; the
roadmap now turns toward making that core **reachable** (a person can obtain and
run it), **replayable** (a run can be reproduced from a log) and **durable**
(the gate's evidence cannot rot), then toward the content and platform surface.

This file is a **rolling roadmap**: a recommendation register, labelled `[REC]`,
re-triaged at every milestone opening per `.planning/ops/MILESTONE-RITUAL.md`.
It is not a promise and not a requirement list — requirements are minted per
milestone in `.planning/REQUIREMENTS.md`. Archived milestone roadmaps live under
`.planning/milestones/<version>-ROADMAP.md` and are append-only.

**Every row names what gates it.** An item with no named gate is a wish.

## Horizons

### Near — v0.2.0 "Registered Evidence, Reachable Runs, Replayable Input"

Selected 2026-11-24 by a four-lens fan-out and a five-point upstream scoping
pass (`.planning/MILESTONE-CONTEXT.md`). Phases continue the numbering at 9.

| # | Phase | Goal | Requirements | Gate |
|---|-------|------|--------------|------|
| 9 | Registered Evidence and Gate Cost | Land the cost lever and trims, take a hosted row, then register the 17 unregistered requirements and harden the gate | EV-01..EV-04 | No external gate; the only work allowed to add gate content, and only after its own hosted row |
| 10 | Reachable Runs and Presentation Policy | A person builds and runs the demo cartridge from the source archive using documented commands; the presentation policy is frozen; the look/listen handoff is recorded | PKG-05, TOOL-04 (narrowed), EV-05 | No new dependency, no install-list change (structurally blocked, see D14); the handoff is `not-performed` unless a human reports back |
| 11 | Input Record and Replay | An input-only log whose replay reproduces the recorded run's digest chain and applied-event chain | TOOL-01 | No owner or rights gate; cost gate only (zero new gate steps) |
| 12 | Bank Modes and External Import | Bank-mode profile data with a typed refusal for every unmapped mode; `.bin`/`.cfg` import outside the core, rights-gated per mode | FMT-01, FMT-04 | Rights review first, per mode; degrades to FMT-01 only; droppable |

Rationale for the order, in one line each: **9** is the entry ticket for every
other phase (the gate may not grow before it); **10** carries the only
owner-latency item in the milestone and is the cheapest phase, so it is
front-loaded; **11** ships into an artifact a person can now obtain; **12** is
last and conditional so a cost spike is absorbed by dropping it rather than
discovered at the close.

### Mid — the next two or three milestones `[REC]`

| Candidate milestone | Delivers | Gate |
|---|---|---|
| **Interactive debugger and inspection** (TOOL-02) | Breakpoints, cycle-stamped trace, disassembly over the existing side-effect-free inspection path; converts tier-4 STIC/CPU claims from golden-compared into inspectable and falsifiable | No external gate; needs replay (v0.2.0) to be useful, and a scripted, headless-verifiable command surface |
| **Variant profiles** (VAR-01..04: Intellivision II, Sears, Tandy aliases) | Alias vs distinct profile decided by image comparison; the first profiles beyond the family-level 2609 NTSC | User-supplied system images plus a rights review for the first three; **PAL has no evidence at all** — every value is unknown, so it needs a primary data sheet or a measurement |
| **Content formats with persistence** (FMT-02, FMT-03) | Intellicart/CC3/JLP/LTO Flash shapes and their persistence; a hash-keyed content database | Rights review, and a persistence story that reverses OQ8 (the exact-build, non-archival snapshot policy) |
| **Scheduled fuzzing and extended sanitizers** (TOOL-05) | A recurring robustness class that the one headless job does not catch | A named failure class that no current gate step catches; the out-of-gate libFuzzer target exists, so the trigger is a failing case found on demand |

### Long — standing intent and explicit non-goals

**Intent.** A high-quality, accurate and useful Intellivision emulator whose
every claim is bound to a profile and evidence: one canonical deterministic
machine core, a narrow headless interface, raw video/audio/input as distinct
observations, exact-build snapshots, a reproducible evidence pipeline, and hosts
that integrate the core rather than reimplement it. Performance work is a goal
only after representative workloads and correctness baselines exist (OQ11 sets
no target today).

**Explicit non-goals** (from `.planning/PROJECT.md` "Out of Scope" and
`.planning/AUTONOMY.md` section 1 — the reasons there are load-bearing):

- Redistributing EXEC, GROM, MTE-201 or any commercial ROM, manual or overlay.
- Copying code, tables or constants from MAME, jzIntv or other emulators.
- Unqualified compatibility labels or percentages.
- Overlays, remapping, accessibility, netplay, rewind, physical device discovery
  or windowing inside the core.
- Keyboard Component, TutorVision, FujiNet mailbox, ECS UART.
- Silent peripheral fallbacks or default memory maps — always explicit refusal.
- "Intellivision" in a package, product or repository name (trademark).

**Owner hard stops that shape the long horizon:** D10 (physical hardware
measurement, 47 items handed off) and D11 (owner-local real EXEC/GROM runs).
Until they are performed, no claim rises above `provisional` and no
`measured-hardware` tier exists. They are the single largest evidence limit in
the project, and no milestone can retire them without the owner's hardware and
files.

## Phases

<details>
<summary>✅ v0.1.0 Machine Core Foundations (Phases 1 to 8) — SHIPPED 2026-11-24</summary>

- [x] Phase 1: Evidence, Profile and Rights Foundations (14/14 plans) — completed 2026-11-09
- [x] Phase 2: CP-1610 CPU Core (18/18 plans) — completed 2026-11-10
- [x] Phase 3: Memory Map, Content Loading and Refusal (14/14 plans) — completed 2026-11-11
- [x] Phase 4: Headless Runner and Installed Consumer, Slice 0 (27 plans) — completed 2026-11-11
- [x] Phase 5: Hand Controllers and PSG Sound (12/12 plans) — completed 2026-11-12
- [x] Phase 6: STIC Video First Pass (14/14 plans) — completed 2026-11-12
- [x] Phase 7: First Visible Original Scenario (12/12 plans) — completed 2026-11-13
- [x] Phase 8: STIC Completion, Save State and Performance Baselines (21/21 plans) — completed 2026-11-24

</details>

<details>
<summary>🚧 v0.2.0 Registered Evidence, Reachable Runs, Replayable Input (Phases 9 to 12)</summary>

- [ ] Phase 9: Registered Evidence and Gate Cost
- [ ] Phase 10: Reachable Runs and Presentation Policy
- [ ] Phase 11: Input Record and Replay
- [ ] Phase 12: Bank Modes and External Import (conditional)

</details>

## Decision Gate Timeline

D1 to D9, D12, D14 to D16 and the OQ rows are delegated decisions (recorded in
PROJECT.md); D10 and D11 are open owner hard stops and were not performed; D13
is deferred. "Before" means the phase could not start, or cannot exit, until the
gate is resolved by its named authority.

| Gate | Authority | Status | Must be resolved |
|------|-----------|--------|------------------|
| D1 Language/build (C17 + CMake) | Owner | Delegated decision 2026-11-09 | Done |
| D2 CPU-cycle unit, time base, crystal constant | Architecture | Delegated decision 2026-11-09 | Done |
| D3 First profile (2609 NTSC family) | Owner | Delegated decision 2026-11-09 | Done |
| D4 Fixture assembler policy | Owner (licence) + architecture | Delegated decision 2026-11-11; re-evaluated in Phase 7: project mini-assembler `ivs_asm`, never shipped | Done |
| D5 System-image policy and project boot stub | Owner | Delegated decision 2026-11-11 | Done |
| D6 Controller core-input representation | Owner + architecture | Delegated decision 2026-11-11 | Done |
| D7 Reference PCM location | Architecture | Delegated decision 2026-11-12 | Done |
| D8 Raw framebuffer geometry (159 x 96) | Architecture + evidence | Delegated decision 2026-11-12 | Done |
| D9 Licence, visibility, naming, publication | Delegated (MIT, public, working name kept); trademark clearance stays an owner item | Applied; `v0.1.0` published as a pre-release | Ongoing at each release |
| D10 Physical hardware measurement | Owner gate | **Open, not performed** (47 items handed off) | Before any measured-tier claim |
| D11 Owner-local real EXEC/GROM runs | Owner gate | **Open, not performed** | Before any real-EXEC result is recorded |
| D12 CI shape | Architecture | Delegated decision 2026-11-11 (one ubuntu-24.04 job) | Done |
| D13 LibFuturo adapter | Owner gate | Deferred | Not before a real consumer exists |
| D14 The runner is not installed in 0.2.x; the person route is the documented build | Architecture + rights | Delegated decision 2026-11-24 (`.planning/MILESTONE-CONTEXT.md` §3) | Phase 10 |
| D15 The replay log is an input artifact (text, no content-digest field) | Architecture + rights | Delegated decision 2026-11-24 | Phase 11 |
| D16 The `.bin`/`.cfg` importer lives outside the core; its refusals are tool tokens | Architecture + rights | Delegated decision 2026-11-24 | Phase 12 |
| OQ8 Snapshot persistence policy | Architecture | Delegated decision (Phase 8) | Done |
| OQ11 Performance targets | Architecture | Delegated decision (PRF-01 sets none) | Done |

## Coverage (v0.1.0 — archived)

| Category | Requirements | Phase |
|----------|--------------|-------|
| Foundations | FND-01..07 | 1 |
| CPU | CPU-01..05 | 2 |
| Core contract | API-01..06 | 2, 3, 4, 5 |
| Memory/loading | MEM-01..05 | 3 |
| Runner | RUN-01..06 | 2, 4 |
| Packaging/CI | PKG-01..04 | 2, 4 |
| Controllers | INP-01..03 | 5 |
| Sound | AUD-01, AUD-02 | 5 |
| Video | VID-01..06 | 6, 8 |
| Scenario | SCN-01, SCN-02 | 7 |
| State/perf | STA-01, STA-02, PRF-01 | 8 |
| Release practice | REL-01 | 8 |

Mapped: 50/50 v0.1.0 requirements, all Complete. No orphans, no duplicates.

## Progress

**v0.1.0 is shipped.** Phases 1 to 8 are complete, archived under
`.planning/milestones/v0.1.0-phases/`, and the release `v0.1.0` is published as a
pre-release on the artifact commit `3fe5373`.

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. Evidence, Profile and Rights Foundations | 14/14 | Complete | 2026-11-09 |
| 2. CP-1610 CPU Core | 18/18 | Complete | 2026-11-10 |
| 3. Memory Map, Content Loading and Refusal | 14/14 | Complete | 2026-11-11 |
| 4. Headless Runner and Installed Consumer (Slice 0) | 25 planned / 27 executed | Complete | 2026-11-11 |
| 5. Hand Controllers and PSG Sound | 12/12 | Complete | 2026-11-12 |
| 6. STIC Video First Pass | 14/14 | Complete | 2026-11-12 |
| 7. First Visible Original Scenario | 12/12 | Complete | 2026-11-13 |
| 8. STIC Completion, Save State and Performance Baselines | 21/21 | Complete | 2026-11-24 |

**Milestone audit:** `.planning/milestones/v0.1.0-MILESTONE-AUDIT.md` — status
`tech_debt`: 50/50 requirements, 8/8 phases, 10/10 integration seams, 10/10
end-to-end flows; the debt is the recorded, accepted register, not new gaps.

**Current milestone:** v0.2.0, scoped 2026-11-24. Phase 9 not started.
