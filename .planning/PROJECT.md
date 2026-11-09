# iamverysmartvision

## What This Is

iamverysmartvision (owner-selected working name; not trademark-cleared) is a new
emulator for the Mattel Intellivision family. It aims to be accurate,
reproducible and useful: a single canonical, deterministic, headless machine
core whose CPU/device behavior, controller input, guest progress, raw video and
raw audio can each be tested independently, and which well-designed hosts can
integrate. It is for the owner and for future host/frontend integrators; support
claims are always tied to an exactly named hardware/content profile and the
evidence that backs it.

## Core Value

A deterministic, observable machine core whose every compatibility claim is
bound to an exactly named profile and independent evidence — never an
unqualified "it runs Intellivision games" label.

## Requirements

### Validated

(None yet — ship to validate)

### Active

All Active items are hypotheses until delivered and verified. Detailed,
ID-tagged requirements live in `.planning/REQUIREMENTS.md`.

- [ ] One canonical machine implementation driven through a narrow headless
      interface with explicit ownership/lifetime of content, controller samples,
      outputs, reset/unload, errors/refusals, bounded execution and instances.
- [ ] An explicitly named first hardware profile (model, revision, region/video
      timing, startup/reset, cartridge arrangement) with documented sources.
- [ ] CPU (CP1610) behavior verified by exact instruction-level tests with
      independent expected values.
- [ ] Memory map, cartridge loading and system-image handling defined with
      refusal behavior for malformed/unsupported content.
- [ ] STIC video producing a raw (pre-scaling) framebuffer observable at a
      defined machine-time boundary.
- [ ] PSG (AY-3-8914 family) sound producing raw samples/events distinct from
      host PCM conversion.
- [ ] Two hand controllers modeled as first-class multi-signal machine input
      (keypad, disc, side buttons, both players, combinations), separate from
      host mapping.
- [ ] Deterministic headless runner: scripted input at an explicit
      machine/time boundary, fresh isolated state, captured observations.
- [ ] Original (project-authored) diagnostic fixture(s) with literal expected
      results, plus a fixture manifest (source, recipe, digest, license,
      notices, allowed use).
- [ ] One visible playable or homebrew scenario early, so the project does not
      become research-only (content must be authored or rights-cleared).
- [ ] Clean installed consumer proving the packaged artifact independently of
      source-tree tests.
- [ ] One documented local command aligned with the fast PR gate; CI that
      reports skips, missing artifacts and stale output visibly.
- [ ] Representative deterministic workloads and correctness baselines before
      any performance optimization.

### Out of Scope (for now — not permanent ceilings)

- Redistributing EXEC, GROM, MTE-201 or any commercial ROM/manual/overlay —
  rights-bearing; no documented permission.
- Copying code from MAME, jzIntv or other emulators without deliberate license
  review — jzIntv is GPL-2.0-or-later; MAME licensing is per-file and must be
  checked; their behavior is also not an independent oracle.
- Promising IntelliVoice, ECS, keyboard component, Intellivision II or clone
  (e.g. Sears Super Video Arcade) support in the first slice — existence in
  another emulator does not imply required support; each is a later roadmap
  decision.
- Physical controller discovery, remapping, accessibility, windowing and audio
  device output inside the core — host-layer responsibilities.
- Unqualified compatibility labels or percentage claims — contradicts Core
  Value.
- Public release, license selection, repository visibility, trademark
  clearance and physical hardware testing without explicit owner action —
  these are owner gates.

## Context

- **Project state at kickoff (verified 2026-11-09):** directory contained only
  `AGENTS.md`, `.planning/config.json` and `.planning/bootstrap/`
  (START-HERE, PROJECT-SEED, EMULATOR-DNA). No code, no git repository (git
  initialized at kickoff), no ROMs, system images, saves or binaries. GSD
  installed: `@opengsd/gsd-core` 1.20.1 (identity verified).
- **Bootstrap research is dated input** (EMULATOR-DNA.md, 2026-11-09). Its
  Intellivision source table (MAME driver, 2609 service manual scan, Blue Sky
  Rangers 2609 overview, GI 1977 data catalog, owner-manual scan,
  intellivision.us hand-controller notes, jzIntv/FujiNet fork) has mixed
  authority. Primary manuals and component data sheets must be checked before
  freezing timing/register requirements. Claims from another emulator do not
  establish Intellivision behavior.
- **Sibling emulator projects and LibFuturo** supply method-level lessons
  (exact test inventories, named-fault controls, fixture provenance, evidence
  tiers, installed-consumer proof, CI critical-path measurement). Their
  hardware assumptions, APIs, performance budgets, gates and licenses do not
  transfer.
- **LibFuturo** is a *candidate* integration only. Its experimental C API
  (instance lifecycle, single content span + config id, 32-bit logical button
  mask sampled at advance start, bounded execution, caller-owned A/V buffers,
  profile-defined persistence) has no Intellivision profile. Fit for keypad +
  disc combinations, content + system images, variable output and
  memory-backed saves is unconfirmed.
- **Bounded search found no permissively redistributable Intellivision
  conformance ROM suite** (evidence limit, not proof of absence). MTE-201 may
  be used only to discover diagnostic categories.
- **Evidence tiers are distinct:** source expectation, public vector, original
  diagnostic, independent implementation comparison, measured hardware,
  deterministic repeatability, playable guest progress, installed-host
  behavior. One tier does not imply the next; repeated hashes prove
  repeatability, not accuracy; shared-core agreement is not an independent
  oracle.

## Constraints

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

## Decision Register

Kept deliberately separate so that a recommendation is never mistaken for an
owner decision.

### Owner decisions (accepted)

| Decision | Source |
|----------|--------|
| Domain is the Mattel Intellivision family | START-HERE / PROJECT-SEED |
| Working name `iamverysmartvision` (not trademark clearance) | START-HERE |
| Goal is high-quality, accurate, useful emulation; first delivery is sequencing, not a ceiling | AGENTS.md |
| Routine reversible GSD work runs autonomously (config `mode: yolo`, `auto_advance: true`) | AGENTS.md / config |
| Bootstrap stops at project initialization — no phase planning or implementation | START-HERE / kickoff prompt |
| Third-party ROMs/manuals/overlays treated as rights-bearing | AGENTS.md |
| Release, license, visibility, third-party content, external qualification and hardware testing are explicit owner gates | PROJECT-SEED (superseded in part by the next row) |
| Autonomous operation: decision gates are delegated to researched recommendations (recorded below as "Delegated decision"); public GitHub repo via `gh`, frequent releases, green `main` and PR auto-triage are authorized. Third-party content, PII/credentials, physical measurement, owner-local EXEC runs, destructive history changes and the trademark name stay hard stops | Owner, 2026-11-09 — `.planning/AUTONOMY.md` |
| Discuss = per-decision subagent fan-out (multi-lens, adversarial, primary sources) → one coherent auto-selected recommendation; "another copy-paste is better than another dependency" | Owner, 2026-11-09 — `.planning/AUTONOMY.md` §2 |
| Verification shifts left toward zero human UAT; CI only for checks with recurring value; review CI efficiency after every run | Owner, 2026-11-09 — `.planning/AUTONOMY.md` §3–4 |

### Delegated decisions (owner authorization 2026-11-09, `.planning/AUTONOMY.md`)

These come from a researched fan-out recommendation that was auto-adopted. Each is reversible through the same protocol, and the reversal evidence is listed. Details are in `.planning/phases/01-evidence-profile-and-rights-foundations/01-CONTEXT.md`.

| Gate | Decision | Would reverse it |
|------|----------|------------------|
| D1 | C17 + CMake 3.25+/Presets/Ninja/CTest, in-repo test harness, no runtime deps (~75%) | Recurring UB despite integer rules; stable Rust sanitizers; Rust-first consumer |
| D2 | Canonical machine time = master tick (one crystal period), `uint64_t` from reset; CPU cycle = profile ratio (NTSC 4); core never reads Hz. Details: `docs/time-base.md`. | Evidence of a device clock not an integer divisor of the crystal |
| D3 | First profile `intv-2609-ntsc-family-p1`: family-level 2609 NTSC, AY-3-8900-1 STIC, AY-3-8914-class PSG (tier 4), zero power-on fill, single-slot ROM-only cartridge, per-field tags | Owner unit measurement (D10) → new board-pinned profile id |
| D9 | MIT licence; public GitHub repo `szTheory/iamverysmartvision` after hygiene check; keep working name; never "Intellivision" in product/package name; README non-affiliation notice | Owner instruction |

### Recommendations (not owner decisions)

| Recommendation | Rationale | Status |
|----------------|-----------|--------|
| Early end-to-end slice: one explicitly named Master Component profile + original diagnostic fixture + independent headless consumer | Exercises core, runner, fixture provenance and consumer boundary together | — Pending owner/discussion |
| 2609-family Master Component as the candidate first profile | Original NTSC console; best-documented; MAME/jzIntv/manuals all reference it | — Pending (revision/region not chosen) |
| Keep one playable/homebrew scenario visible early | Prevent research-only drift | — Pending (title/fixture not chosen) |
| Original source-built diagnostics over third-party test ROMs | No cleared conformance suite found | — Pending |

### GSD-generated decisions (reversible defaults)

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Keep owner-provided `.planning/config.json` unchanged rather than re-asking auto-mode config questions | Owner pre-set the established emulator-project defaults | ✓ Applied at init |
| Project structure mode: vertical MVP slices | GSD `--auto` default; matches seed's "verified vertical slices" | ✓ Applied at init |
| Do not set `workflow._auto_chain_active` at init | Kickoff explicitly stops before discuss/plan-phase | ✓ Applied at init |

### Open questions (unresolved — preserve, do not silently decide)

1. Exact first console/variant profile; region and TV timing (NTSC/PAL).
2. CPU, STIC, PSG and memory/timing architecture; required raster and audio
   precision.
3. How EXEC/GROM system images are supplied, if at all (user-supplied path,
   original replacement, or none), and their identity verification.
4. Cartridge formats and unusual memory maps to support first.
5. Controller electrical representation vs logical host mapping; overlays as
   metadata vs feature.
6. Whether IntelliVoice, ECS, keyboard component, Intellivision II and clones
   are first-release, later-roadmap or out of scope.
7. Implementation language and build system; public API/ABI; error, ownership
   and versioning policy.
8. Debugger/headless runner shape; save-state and battery-backed persistence.
9. Initial original/cleared fixture and independent oracle; first playable or
   homebrew scenario.
10. First consumer and supported runtime/platforms; whether/when to pursue a
    LibFuturo adapter.
11. Dependency policy details, new-code license, repository visibility, release
    artifacts, performance targets.

## Evolution

This document evolves at phase transitions and milestone boundaries.

**After each phase transition** (via `/gsd-transition`):
1. Requirements invalidated? → Move to Out of Scope with reason
2. Requirements validated? → Move to Validated with phase reference
3. New requirements emerged? → Add to Active
4. Decisions to log? → Add to the Decision Register (correct category)
5. "What This Is" still accurate? → Update if drifted

**After each milestone** (via `/gsd-complete-milestone`):
1. Full review of all sections
2. Core Value check — still the right priority?
3. Audit Out of Scope — reasons still valid?
4. Update Context with current state
5. Main green, open PRs triaged, rolling roadmap and lessons updated, release
   of exact tested artifact targeted when rights/readiness/authority permit

---
*Last updated: 2026-11-09 after initialization*
