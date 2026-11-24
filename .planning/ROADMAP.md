# Roadmap: iamverysmartvision

## Overview

The project goes from an empty repository to a deterministic, headless
Intellivision machine core. Every compatibility claim in that core cites a named
profile and an evidence tier. The full phase-by-phase detail of the shipped
milestone is archived in `.planning/milestones/v0.1.0-ROADMAP.md`.

## Milestones

- ✅ **v0.1.0 Machine Core Foundations** — Phases 1 to 8 (shipped 2026-11-24; software `v0.1.0` published as a pre-release)
- 🚧 **Next milestone** — not started; the candidates and their evidence gates are in the v0.1.0 close record, section D

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

## Decision Gate Timeline

D1 to D9 and D12 are delegated decisions (recorded in PROJECT.md); D10 and D11
are open owner hard stops and were not performed; D13 is deferred. "Before"
means the phase could not start, or cannot exit, until the gate is resolved by
its named authority.

| Gate | Authority | Status | Must be resolved |
|------|-----------|--------|------------------|
| D1 Language/build (C17 + CMake recommended; Rust alternative) | Owner | Delegated decision 2026-11-09 (PROJECT.md) | Before Phase 2 starts |
| D2 CPU-cycle unit, time base, crystal constant | Architecture | Delegated decision 2026-11-09 (PROJECT.md) | During Phase 1 (FND-07); before Phase 2 |
| D3 First profile (2609 NTSC recommended; revision, STIC/PSG parts, power-on policy) | Owner | Delegated decision 2026-11-09 (PROJECT.md) | Before Phase 1 exits |
| D4 Fixture assembler policy | Owner (licence) + architecture | Delegated decision 2026-11-11 (PROJECT.md; Phase 4 D-25, D-12, D-41); re-evaluated in Phase 7 (D-16, D-19): project mini-assembler `ivs_asm`, never shipped | Before Phase 4 starts |
| D5 System-image policy and project boot stub | Owner | Delegated decision 2026-11-11 (PROJECT.md; Phase 3 D-24) | Phase 3 (refusal); before Phase 4 (boot) |
| D6 Controller core-input representation (C14) | Owner + architecture | Delegated decision 2026-11-11 (PROJECT.md; Phase 4 D-25, D-12, D-41) | While Phase 4 shapes the API; before Phase 5 starts |
| D7 Reference PCM location (C23) | Architecture | Delegated decision 2026-11-12 (PROJECT.md; Phase 5 D-21: outside the core, `tools/pcm`, runner `--audio-wav` only) | Phase 5 |
| D8 Raw framebuffer geometry (C10) | Architecture + evidence | Delegated decision 2026-11-12 (PROJECT.md; Phase 6 D-12: 159 x 96, width from `video.last_card_column_px`, reversible by one profile field) | Phase 6 |
| D9 License, visibility, naming/trademark, publication | Delegated (MIT, public repository, working name kept); trademark clearance stays an owner item | Delegated decision 2026-11-09 (PROJECT.md); applied at the Phase 8 close: `v0.1.0` published as a pre-release through the delegated flow | Before any external publication; CI hosting (Phase 4) and the milestone release assessment (Phase 8) touch it |
| D10 Physical hardware measurement | Owner gate | Open, not performed (47 items handed off) | Before any measured-tier claim (Phase 5 onward) |
| D11 Owner-local real EXEC/GROM runs | Owner gate | Open, not performed (SCN-02 is complete on documented-route terms; no result, claim or tier) | Before any real-EXEC result is recorded |
| D12 CI shape (C24) | Architecture | Delegated decision 2026-11-11 (PROJECT.md; Phase 4 D-25, D-12, D-41) | Before Phase 4 exits |
| D13 LibFuturo adapter / shared-contract change | Owner gate | Deferred | Not before a real consumer exists; outside this milestone |
| OQ8 Snapshot persistence policy | Architecture | Delegated decision (Phase 8 plan 08-17, PROJECT.md) | Applied in Phase 8 |
| OQ11 Performance targets | Architecture | Delegated decision (Phase 8 PRF-01 sets none, PROJECT.md) | Applied in Phase 8 |

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

Mapped: 50/50 v1 requirements, all Complete. No orphans, no duplicates.

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

**Next milestone:** not started. Candidates and their evidence gates are in
`.planning/milestones/v0.1.0-phases/08-stic-completion-save-state-performance-baselines/08-MILESTONE-CLOSE.md`,
section D.
