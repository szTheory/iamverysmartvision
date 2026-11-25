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

- [x] Phase 9: Registered Evidence and Gate Cost — land the cost lever and trims, take the hosted row that settles the level, then register the 17 already-faulted requirements and make the gap unrepeatable
- [x] Phase 10: Reachable Runs and Presentation Policy — a person builds and runs the demo cartridge from the released source archive with documented commands; the presentation policy is frozen and enforced; the look/listen handoff is recorded
- [ ] Phase 11: Input Record and Replay — an input-only, rights-clean log whose replay reproduces the recorded run's digest chain and applied-event chain, with the vacuity guards proved live
- [ ] Phase 12: Bank Modes and External Import (conditional) — bank-mode profile data with a typed refusal for every unmapped mode, and `.bin`/`.cfg` import outside the core only where a per-mode rights review clears it; droppable

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

## Phase Details

**Milestone v0.2.0.** The phase shape, exit criteria, decisions (D14 to D16),
verification strategy, tier ceilings and risks are frozen in
`.planning/MILESTONE-CONTEXT.md`; the retarget table and the definition of done
are `.planning/ops/GUARDRAILS-v0.2.md`. The milestone plans about 24 plans across
the four phases (18 without Phase 12).

Three rules hold across every phase below: **zero new gate steps** (38 stays
38; every capability lands as ids inside a suite that already exists), **no new
test id, control or fault before Phase 9's hosted row**, and **no claim above
`provisional`** while D10 and D11 are unperformed.

### Phase 9: Registered Evidence and Gate Cost

**Goal**: The gate's cost level is settled by measurement before the gate grows,
and the 17 requirements whose named faults already run in every gate are
registered with mechanisms that cannot pass vacuously — with a machine check
that makes the gap unrepeatable.
**Depends on**: Nothing (first phase of v0.2.0; continues from the shipped Phase 8)
**Requirements**: EV-01, EV-02, EV-03, EV-04
**Success Criteria** (what must be TRUE):
  1. A hosted row recorded after the lever and trim commits settles the cost level: the row's head is a descendant of those commits (`git merge-base --is-ancestor`), and `.planning/CI-EFFICIENCY.md` carries exactly one checkpoint line — `discharged-below-budget`, `discharged-within-line` or `lever-required` — with the last-five median `r` recomputed by arithmetic from `.planning/ci-runs.tsv`.
  2. No commit before that row adds a test id, control, fault or gate step: `ls tools/check/steps/*.sh | wc -l` stays 38, `data/expected/gate-steps/` is unchanged, and the ancestry check passes.
  3. `data/expected/important-gates.ids` grows from 306 to 330 registry rows (24 additions, counted as non-comment rows), naming at least one mechanism for each of FND-01..07, CPU-01..05 and MEM-01..05; the registration commit's `steps`/`controls`/`faults`/`tests-*` diff equals its predicted delta exactly, and each of the ten toy registries gains exactly one line per new requirement with zero deletions (`git diff --numstat`).
  4. Each of the five gate-hardening items (whole-token `SRC-` match, zero-document failures, per-step pass lines, manifest output anchoring, memmap generator controls) has a named control in the gate summary and a committed scratch stand-in showing it fails for its intended reason.
  5. The EV-04 machine check refuses a requirement marked Complete with no registry line, and refuses a toy registry that drifts from the good tree; both are shown able to fail, and `sh`, `dash` and the container gate end PASS with the predicted summary token.
**Plans**: 9/9 plans executed in two waves (A1 cost-only, then A2 content-free registration after the orchestrator's hosted row)

Wave A1 (cost only; no test id, control, fault, gate step or registry line):
- [x] 09-01-PLAN.md — lever (3): the static-variant fold with the derivation-equality judgement, the pair mode switch and trim (a)
- [x] 09-02-PLAN.md — trims (c) and (d) with their preserving conditions
- [x] 09-03-PLAN.md — the measured wave A1 cost record and the wave exit evidence

Wave A2 (after the hosted row; `depends_on` the A1 plans):
- [x] 09-04-PLAN.md — EV-02: 24 registry rows for FND-01..07, CPU-01..05, MEM-01..05, the accepted set, the prose and the ten toy registries
- [x] 09-05-PLAN.md — EV-04(a): the derived must-be-covered set, the six-row exclusion ledger and its control
- [x] 09-06-PLAN.md — EV-04(b) and EV-03 item 3: the toy-registry drift check and the per-step pass-line guard
- [x] 09-07-PLAN.md — EV-03: the whole-token `SRC-` match, manifest output anchoring, the memmap generator, and the zero-document verification
- [x] 09-08-PLAN.md — the preflight partition with its committed pre-change capture and normalised diff
- [x] 09-09-PLAN.md — bookkeeping, the requirement close and the phase exit evidence
**Tier ceiling**: none — this phase adds no claim.
**Cost gate**: the only phase allowed to add gate content, and only after its own hosted row. Wave A1 touches only the seven files its plan names.

### Phase 10: Reachable Runs and Presentation Policy

**Goal**: A person obtains and runs the emulator from the released source
archive using documented commands, the presentation and palette policy is frozen
and enforced, and the look/listen handoff is recorded rather than assumed.
**Depends on**: Phase 9 (its hosted row is the entry ticket for any new id, control or fault)
**Requirements**: PKG-05, TOOL-04 (narrowed), EV-05
**Success Criteria** (what must be TRUE):
  1. `docs/running.md` names the three commands and the exact expected stdout tail, and the release rehearsal (`tools/release-rehearse.sh`) builds from the **source archive** in a clean directory and runs `demo-lamp-tone-pass` to its golden — proved by a recorded rehearsal transcript, at release time and not as a gate step.
  2. `docs/presentation.md` holds the six-clause policy, and one deny-list check refuses a colour symbol defined under `core/src`, shown able to fail by a committed scratch stand-in; the check adds no gate step.
  3. `data/expected/handoffs.ids` holds H7-1, H7-2, H8-3, D10 and D11, every row `evidence: none` and each dispositioned `handed-off` or `not-performed`; a claim citing a handoff artifact, a `build/` path, a `.wav`, a `.ppm` or a bench `.tsv` is refused, and the `text` step's claim-word list contains the look/listen words — both shown able to fail.
  4. `README.md` and `docs/packaging.md` carry one identical "nothing downloadable is runnable" sentence (a grep proves a single matching line in each) and one paragraph pointing at `docs/running.md`.
  5. The gate summary keeps `steps=38/38` and `claims=` at its Phase 9 value, and no claim cites a handoff row: the documented route reaches at most `installed-host-behavior` and the handoff carries no tier.
**Plans**: 6/6 plans executed
- [x] 10-01-PLAN.md
- [x] 10-02-PLAN.md
- [x] 10-03-PLAN.md
- [x] 10-04-PLAN.md
- [x] 10-05-PLAN.md
- [x] 10-06-PLAN.md
**Tier ceiling**: at most `installed-host-behavior` for the documented route; no tier for the handoff.
**Not in this phase**: installing `ivs-run` or any image (structurally blocked, not merely expensive — D14), a windowed frontend, any new dependency, a second example cartridge.

### Phase 11: Input Record and Replay

**Goal**: A run is recorded as a rights-clean, input-only log, and a replay of
that log reproduces the recorded run's digest chain and applied-event chain —
with the vacuity guards that v0.1.0's round-trip hook lacked.
**Depends on**: Phase 10 (replay ships into an artifact a person can now obtain), Phase 9 (the cost row)
**Requirements**: TOOL-01
**Success Criteria** (what must be TRUE):
  1. `docs/formats/ivsl-v1.md` is committed before any code and the ancestry is proved with `git merge-base --is-ancestor`; its grammar has no field able to hold a content digest, image name, length, frame byte, audio byte or path, and an unknown directive is refused with a typed token.
  2. `runner.record.replay-equals-straight` and `runner.record.format-roundtrip` pass in process: a recorded log replays to the straight run's nine digest lines and applied-event chain, and write, parse and write again is byte-identical.
  3. The liveness guards do the work: the equality case asserts at least one event, `input.consumed == input.events`, an `at_tick` list equal to the recorded one and all nine domains observed, while `replay-differs` (one digest nibble changed) and `replay-inert` (a fake pair with zero applied events) are rejected **by the guard**, each shown able to fail.
  4. Every malformed log in the refusal family (unknown directive, non-monotonic tick, tick beyond bound, event cap, undefined controller bit, wrong version, trailing garbage) is refused with a typed token and no state change, and the two named faults `IVS_FAULT_RECORD_DROPS_LAST_EVENT` and `IVS_FAULT_REPLAY_APPLIES_LATE` fail exactly their ids.
  5. `runner.record.no-content-digest` finds zero runs of 32 or more hex digits in a produced log; the log is text, absent from `tools/check/binary-ext.txt` and not gitignored; the gate ends `steps=38/38` with the plan's predicted count delta and the tier ceiling `deterministic-repeatability`, provisional.
**Plans**: 4/5 plans executed, 4 waves
Plans:
- [x] 11-01-PLAN.md — `docs/formats/ivsl-v1.md` first: the grammar, the nine refusal tokens, the measured caps, the profile scope and the D15 rights posture (wave 1)
- [x] 11-02-PLAN.md — the tracer: `runner/ivsl.c` + the executor's schedule substitution and applied-prefix recorder + the effective `input.events`, with the equality, the round trip and the two judge self-controls (wave 2)
- [x] 11-03-PLAN.md — the nine `runner.record.refuse.<token>` ids with an untouched-output assertion each, and `runner.record.no-content-digest` with its stand-in (wave 3)
- [x] 11-04-PLAN.md — `--record-input` (exclusive create) and `--replay-input`, and the one process case on `demo-lamp-tone-pass` with its `input` lines stripped (wave 3)
- [ ] 11-05-PLAN.md — the two named faults, `TOOL-01`'s close with four registry rows and the toy obligations in one commit, and the phase exit (wave 4)
**Tier ceiling**: `deterministic-repeatability` — the same core, so never an independent comparison.

### Phase 12: Bank Modes and External Import (conditional, droppable)

**Goal**: Bank-mode behaviour is profile data with a typed refusal for every mode
the profile does not name, and — only where a per-mode rights review clears it —
`.bin`/`.cfg` import outside the core.
**Depends on**: Phase 9 (the cost row); its own rights-review entry gate
**Requirements**: FMT-01 (narrowed), FMT-04
**Entry gate**: a rights-review decision line committed **before** any third-party file is read — `entered`, or `degraded to FMT-01 only`.
**Success Criteria** (what must be TRUE):
  1. The entry-gate line exists before any third-party file is read; on degradation the phase records `FMT-01: partial (bank-mode model only), importer deferred: <condition>`, never `Complete`, and writes no `.cfg` description.
  2. Every accepted bank mode has a generated original fixture (tracked `.asm`/`.cfgsrc` sources plus a manifest, built into `build/<preset>/images/`), a witness pair (the same address reads A before the select write and B after), a no-switch negative case that still reads A, and one named fault — with a positive case beside every refusal, so "refuse everything" cannot pass.
  3. Every mode the profile does not name is refused by name — `cfg-bank-mode-unmapped` or `cfg-bank-mode-unsupported` — never a silent default; the refusal token set is pinned in `data/expected/cfg-refusals.ids` and each token is shown able to fail.
  4. No tracked file carries an extension in `tools/check/binary-ext.txt`, that list and `.gitignore` are unchanged, the importer refuses to write inside the work tree (`import-out-inside-tree`, `out-exists`), and no new `IVS_E_*` reason, core change or `api-effects` retarget lands.
  5. The gate ends `steps=38/38` with no claim above `provisional` and no "runs games" wording; the tier ceiling is `source-expectation` plus `original-diagnostic` plus `deterministic-repeatability`, and `public-vector` and `independent-implementation-comparison` are unreachable by design; if the phase is dropped (the pre-authorized three-phase fallback) or degraded, FMT-01 and FMT-04 are recorded as partial or deferred with the condition in `.planning/REQUIREMENTS.md` traceability — never silently lost and never marked `Complete`.
**Plans**: TBD
**Tier ceiling**: as criterion 5 — no third-party file is read, by design.
**Droppable**: yes. A cost spike or a failed rights review is absorbed here rather than discovered at the milestone close.

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

## Coverage (v0.2.0 — in progress)

| Requirement | Phase |
|-------------|-------|
| EV-01, EV-02, EV-03, EV-04 | 9 |
| EV-05, PKG-05, TOOL-04 (narrowed) | 10 |
| TOOL-01 | 11 |
| FMT-01 (narrowed), FMT-04 | 12 |

Mapped: 10/10 v0.2.0 requirements, each to exactly one phase. No orphans, no
duplicates. FMT-01 and FMT-04 are the droppable pair: if Phase 12 is dropped,
they are recorded as partial or deferred in `.planning/REQUIREMENTS.md` — not
lost.

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

### v0.2.0 — in progress (scoped 2026-11-24)

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 9. Registered Evidence and Gate Cost | 9/9 | Complete    | 2026-11-24 |
| 10. Reachable Runs and Presentation Policy | 6/6 | Complete    | 2026-11-25 |
| 11. Input Record and Replay | 0/TBD | Not started | - |
| 12. Bank Modes and External Import (conditional) | 0/TBD | Not started | - |

**Current milestone:** v0.2.0, scoped 2026-11-24 and detailed by this roadmap on
2026-11-24. Phase 9 not started; no phase plan exists. Phase 12 is conditional
and droppable. The milestone adds zero new gate steps and no claim above
`provisional`.
