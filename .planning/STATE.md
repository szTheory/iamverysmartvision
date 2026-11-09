---
gsd_state_version: "1.0"
current_phase: 1
current_phase_name: Evidence, Profile and Rights Foundations
status: executing
stopped_at: Completed 01-14-PLAN.md
last_updated: "2026-11-09T21:55:47.554Z"
last_activity: 2026-11-09
last_activity_desc: Phase 1 execution started
state_head: 07991902f8190882a7ab7079d664cb5640a15e60
progress:
  total_phases: 8
  completed_phases: 0
  total_plans: 14
  completed_plans: 2
  percent: 0
---

# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-11-09)

**Core value:** A deterministic, observable machine core whose every compatibility claim is bound to an exactly named profile and independent evidence — never an unqualified "it runs Intellivision games" label.
**Current focus:** Phase 1 — Evidence, Profile and Rights Foundations

## Current Position

Phase: 1 (Evidence, Profile and Rights Foundations) — EXECUTING
Plan: 3 of 14
Status: Ready to execute
Last activity: 2026-11-09 — Phase 1 execution started

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
**Per-Plan Metrics:**

| Plan | Duration | Tasks | Files |
|------|----------|-------|-------|
| Phase 1 P01 | 30min | 2 tasks | 90 files |
| Phase 1 P14 | 6min | 2 tasks | 1 files |

## Accumulated Context

### Decisions

Decisions live in the PROJECT.md Decision Register (owner decisions /
recommendations / GSD defaults / open questions are kept separate).

- [Init]: Roadmap order follows research (foundations → CPU → memory/refusal → Slice 0 runner+consumer → controllers+PSG → STIC first pass → visible scenario → STIC completion+state+perf). GSD default, reversible.
- [Init]: Visible scenario (Phase 7) placed directly after the first STIC pass, ahead of full STIC coverage (conflict C26). GSD default, reversible.
- [Init]: Variant profiles and peripherals (the research summary's proposed variants stage) are v2 and are not in this roadmap.
- [Phase 1]: 01-01: gate summary steps=<passed>/<expected>; vocab step prints its own pass line; duplicate JSON keys stay a documented undetectable limit
- [Phase 1]: 01-14: sources registry keeps community wiki and docs tree reference-only (no stable single-document hash); Blue Sky page re-hash matches research

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

Last session: 2026-11-09T21:55:47.521Z
Stopped at: Completed 01-14-PLAN.md
Resume file: None
