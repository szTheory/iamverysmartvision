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

Gates resolved as delegated decisions (owner authorization 2026-11-09, `.planning/AUTONOMY.md`; see PROJECT.md Decision Register):
- D1 C17 + CMake; D2 master tick (u64); D3 `intv-2609-ntsc-family-p1`; D9 MIT + public repo after hygiene check.

Still open, decided by fan-out protocol when their phase needs them: D4 (Phase 4), D5 (Phase 3/4), D6 (Phase 4/5), D7 (Phase 5), D8 (Phase 6), D12 (Phase 4), D13 (deferred).
Hard stops (not delegated): D10 physical measurement, D11 owner-local EXEC/GROM runs — evidence marked missing, work continues.


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
