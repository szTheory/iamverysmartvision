---
gsd_state_version: '1.0'
status: planning
progress:
  total_phases: 8
  completed_phases: 0
  total_plans: 0
  completed_plans: 0
  percent: 0
---

# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-11-09)

**Core value:** A deterministic, observable machine core whose every compatibility claim is bound to an exactly named profile and independent evidence — never an unqualified "it runs Intellivision games" label.
**Current focus:** Phase 1 — Evidence, Profile and Rights Foundations (not started)

## Current Position

Phase: 1 of 8 (Evidence, Profile and Rights Foundations)
Plan: 0 of TBD in current phase
Status: Phase 1 not started — awaiting owner decisions/discuss-phase
Last activity: 2026-11-09 — Rolling roadmap created (8 phases, 50/50 v1 requirements mapped)

Progress: [░░░░░░░░░░] 0%

## Performance Metrics

**Velocity:**
- Total plans completed: 0
- Average duration: -
- Total execution time: -

**By Phase:**

| Phase | Plans | Total | Avg/Plan |
|-------|-------|-------|----------|
| - | - | - | - |

*Updated after each plan completion*

## Accumulated Context

### Decisions

Decisions live in the PROJECT.md Decision Register (owner decisions /
recommendations / GSD defaults / open questions are kept separate).

- [Init]: Roadmap order follows research (foundations → CPU → memory/refusal → Slice 0 runner+consumer → controllers+PSG → STIC first pass → visible scenario → STIC completion+state+perf). GSD default, reversible.
- [Init]: Visible scenario (Phase 7) placed directly after the first STIC pass, ahead of full STIC coverage (conflict C26). GSD default, reversible.
- [Init]: Variant profiles and peripherals (the research summary's proposed variants stage) are v2 and are not in this roadmap.

### Pending Todos

None yet.

### Blockers/Concerns

Owner gates (open; none decided; recommendations are not decisions):
- D1 Language/build (C17 + CMake recommended; Rust alternative) — blocks Phase 2 start.
- D3 First profile (2609 NTSC recommended; revision, STIC/PSG parts, power-on policy) — blocks Phase 1 exit.
- D5 System-image policy and project boot stub — needed in Phase 3 (refusal) and before Phase 4 boot path.
- D6 Controller core-input representation (C14; owner + architecture) — settle while Phase 4 shapes the API; blocks Phase 5 start.
- D4 Fixture assembler policy (owner licence + architecture, C22) — blocks Phase 4 guest fixtures.
- D9 License, repository visibility, naming/trademark, publication — blocks any external publication; touches CI hosting (Phase 4) and release assessment (Phase 8).
- D10 Physical hardware measurement — no measured-tier claim without it.
- D11 Owner-local real EXEC/GROM runs — gates SCN-02 claims (Phase 7).
- D13 LibFuturo adapter — deferred; not before a real consumer.

Architecture decisions (open): D2 time base (Phase 1), D7 reference PCM location (Phase 5), D8 raw framebuffer geometry (Phase 6), D12 CI shape (Phase 4).

Other open questions: cartridge import format (OQ4, Phase 3), first scenario title/program (OQ9, Phase 7), persistence policy (OQ8, Phase 8).

Concerns:
- 8 phases exceeds `standard` granularity guidance (4-6); justified by five high-uncertainty subsystems (PIT-18). Revisit at milestone discussion.
- No tier-1 AY-3-8914 datasheet found; STIC timing constants disputed (C1-C4); controller matrix from one LOW source (C16).

## Deferred Items

| Category | Item | Status | Deferred At | Milestone |
|----------|------|--------|-------------|-----------|
| *(none)* | | | | |

## Session Continuity

Last session: 2026-11-09
Stopped at: Roadmap and state initialized; no phase discussed or planned
Resume file: None
