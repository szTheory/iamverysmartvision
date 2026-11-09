---
gsd_state_version: "1.0"
current_phase: 1
current_phase_name: Evidence, Profile and Rights Foundations
status: executing
stopped_at: Completed 01-10-PLAN.md
last_updated: "2026-11-09T23:23:25.614Z"
last_activity: 2026-11-09
last_activity_desc: Phase 1 execution started
state_head: 4f8d9e591bd3768c783cba645e0ed5ce368ce16f
progress:
  total_phases: 8
  completed_phases: 0
  total_plans: 14
  completed_plans: 13
  percent: 0
---

# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-11-09)

**Core value:** A deterministic, observable machine core whose every compatibility claim is bound to an exactly named profile and independent evidence — never an unqualified "it runs Intellivision games" label.
**Current focus:** Phase 1 — Evidence, Profile and Rights Foundations

## Current Position

Phase: 1 (Evidence, Profile and Rights Foundations) — EXECUTING
Plan: 14 of 14
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
| Phase 1 P02 | 35min | 3 tasks | 1 files |
| Phase 01 P03 | 5min | 3 tasks | 1 files |
| Phase 01 P04 | 15min | 2 tasks | 18 files |
| Phase 1 P05 | 20min | 2 tasks | 26 files |
| Phase 1 P06 | 35min | 2 tasks | 60 files |
| Phase 1 P07 | 25min | 2 tasks | 41 files |
| Phase 1 P08 | 25min | 2 tasks | 24 files |
| Phase 1 P13 | 15min | 2 tasks | 12 files |
| Phase 1 P09 | 25min | 2 tasks | 15 files |
| Phase 1 P01-11 | 40min | 2 tasks | 16 files |
| Phase 1 P10 | 40min | 3 tasks | 11 files |

## Accumulated Context

### Decisions

Decisions live in the PROJECT.md Decision Register (owner decisions /
recommendations / GSD defaults / open questions are kept separate).

- [Init]: Roadmap order follows research (foundations → CPU → memory/refusal → Slice 0 runner+consumer → controllers+PSG → STIC first pass → visible scenario → STIC completion+state+perf). GSD default, reversible.
- [Init]: Visible scenario (Phase 7) placed directly after the first STIC pass, ahead of full STIC coverage (conflict C26). GSD default, reversible.
- [Init]: Variant profiles and peripherals (the research summary's proposed variants stage) are v2 and are not in this roadmap.
- [Phase 1]: 01-01: gate summary steps=<passed>/<expected>; vocab step prints its own pass line; duplicate JSON keys stay a documented undetectable limit
- [Phase 1]: 01-14: sources registry keeps community wiki and docs tree reference-only (no stable single-document hash); Blue Sky page re-hash matches research
- [Phase 1]: C17 recorded resolved-primary from page-image reading of pdf 24, 46, 48, 62 (01-02)
- [Phase 1]: Interrupt-entry (9 microcycles) and reset sequence counts in CP-1610 notes are inferred from timing-diagram time slots, not printed tables (01-02)
- [Phase 01]: 01-03: opcode family table has 74 rows (below D-19's about 90: SDBD as attributes, aliases on one row, one jump row); NOPP transcribed as printed 7 cycles not 7/9; interruptible is no only where the page prints it, else manual-silent
- [Phase 01]: 01-04: C17 resolved-primary, C19 partial; notes candidates became C29-C35 (device-supplied reset PC is C32); log holds 35 entries
- [Phase 01]: 01-04: open/partial default-policy prefix is checked in the discrepancy hook because the records DSL cannot hold a space in a regex
- [Phase 1]: Claim ids are CL-NNNN, strictly ascending; only CL-0002 is established, claims resting on tier4-community, inference or unknown-default-chosen fields stay provisional
- [Phase 1]: text step exempts only the literal placeholder CL-NNNN from claim-reference resolution
- [Phase 1]: 01-06: files step reports tracked=<n> (integrity already owns files=<n>); unknown derived_from ids and cycles resolve to effective-no
- [Phase 1]: 01-07: denylist stat named scanned= (not files=) to avoid clashing with the integrity step key; reference-only registry entries must carry null digests and a note mentioning hashing
- [Phase 1]: 01-08: D-11 unit-suffix rule applies to duration/offset profile keys; ticks_per_X and X_per_Y ratio keys name their units and comply; enforced by the glossary step
- [Phase 1]: 01-08: glossary success line uses doc-files= (text step already emits docs=)
- [Phase 1]: 01-09: cite and sdbd regexes checked in the cp1610-row hook (DSL cannot hold spaces); no cycle value required so NOPP 7 passes; cp1610 step measured 0.7 s so decode stays in cmake -P
- [Phase 1]: 01-11: hygiene success line uses key hygiene-files (not files) to avoid a duplicate gate stat key; allow patterns anchored on grep -o output
- [Phase 1]: 01-11: cmake line budget measured 2154 vs plan limit 1500; not shrunk, recorded as deviation for owner/orchestrator
- [Phase 1]: 01-10: page-internal figure-vs-table differences (GSWD, RSWD, XOR@ printed form) are recorded as resolved second-reader entries; the family table is unchanged

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

Last session: 2026-11-09T23:23:20.353Z
Stopped at: Completed 01-10-PLAN.md
Resume file: None
