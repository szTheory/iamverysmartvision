# Roadmap: iamverysmartvision

## Overview

The project is building a deterministic, headless Intellivision machine core in
which every compatibility claim cites an exactly named profile and an evidence
tier. v0.1.0 delivered the machine and the evidence apparatus. v0.2.0 made the
core **reachable** (a person builds and runs it from the source archive),
**replayable** (a run reproduces from an input log) and **durable** (the gate's
evidence cannot rot, and its cost is measured). v0.3.0 turns to the question the
project has never been able to answer: **can a person put real content in and
drive it?** — a rights-clean content path, a build-only host surface, and the
owner's own files as a handoff row rather than a promise.

This file is a **rolling roadmap**: a recommendation register, labelled `[REC]`,
re-triaged at every milestone opening per `.planning/ops/MILESTONE-RITUAL.md`.
It is not a promise and not a requirement list — requirements are minted per
milestone in `.planning/REQUIREMENTS.md`. Archived milestone roadmaps live under
`.planning/milestones/<version>-ROADMAP.md` and are append-only.

**Every row names what gates it.** An item with no named gate is a wish.

## Horizons

### Near — v0.3.0 "Owner-Supplied Content and a Driveable Host"

Selected 2026-11-26 by a four-lens fan-out and a five-point upstream scoping
pass (`.planning/MILESTONE-CONTEXT.md`; the five scoping outputs are frozen in
`.planning/research/v0.3.0-scoping-*.md`). Phases continue the numbering at 13.

**Why this one, in one line:** the project has never been able to answer "can a
person put real content in and drive it", and the owner asked for exactly that
on 2026-11-26; every one of the four lenses put the content path first and the
host surface second, with the owner's own files last as a handoff row.

| # | Phase | Goal | Requirements | Gate |
|---|-------|------|--------------|------|
| 13 | Rights Ledger and Enforced Claim Ceiling | One question-shaped rights row per content question, a check that refuses an unsourced `cleared` row and a refused spelling in code or operator pages, D-32's `established`-claim check, and the ten D-09-06 mechanisms registered as `mutant` rows | RTS-01, RTS-02, EV-06, EV-07 | No external gate; it is the entry ticket for every later phase, because until it exists a refused item can be named and a claim can rise with the gate still green |
| 14 | Cartridge Route and Real-Content Conformance | A build-only tool wraps a flat image at an operator-stated base into IVSC v1 with seven typed refusals each beside its boundary positive; the owner-local route gains a cartridge role; and the first real-content blocker — the interrupt-enable default that ends the EXEC at `$FFFF` — is fixed with its default resolved, pinned by a project-authored test, and every dependent expectation retargeted | TOOL-07, SCN-03, CPU-06 | Rights: nothing parses a third-party format and the output is derived material; cost line only (zero new steps). **The owner's own images drive it**, and the observation mints no claim (D-51) |
| 15 | Build-Only Host and the Fake-Frontend Harness | A shared object outside `core/` and outside the install list, driven by a harness that `dlopen`s it and reproduces the runner's nine digest lines on a project cartridge, guarded three ways | TOOL-08 | Cost and CI shape: it is the only phase that creates a new build target (D12's named reversal trigger), so it is the droppable one |
| 16 | Owner Route, Claim and Release Posture | One documented owner route from the source archive, handoff rows, a tracked shared object refused by name, and a release-decision line recomputed | PKG-06 | Owner hard stop D11: the owner's own run is a handoff row, never a criterion; the concurrency lever is taken here, cost-only |

Rationale for the order, in one line each: **13** is the entry ticket (it makes
naming legal and claims bounded); **14** is the binding constraint, because a
host built before it has nothing to load and RetroArch would refuse every real
image; **15** is worth building only once there is something to load, and it is
last-but-one because it is the least predictable; **16** closes the loop.

**Droppable:** Phase 15, pre-authorized, degrading to a content-and-conformance
v0.3.0. It leaves a new content capability; dropping 14 would not. The
degradation is written in the close record.

**Re-cut 2026-11-26 by the first owner-local real-content run.** The owner's own
EXEC, GROM and an 8K cartridge now load and run; the EXEC dies at the first
interrupt because `cpu.interrupt_accept_clears_intff = 0` (C41) lets the handler
re-enter on every instruction boundary, and 600 frames are black. Clearing the
enable on acceptance fixes the storm and the EXEC populates BACKTAB. A host built
before that fix is a black screen with a window around it, so the conformance
work joins Phase 14 and Phase 15 now depends on it. Full record:
`.planning/MILESTONE-CONTEXT.md` §1a.

### Mid — the next two or three milestones `[REC]` (re-triaged 2026-11-26)

| Candidate milestone | Delivers | Gate |
|---|---|---|
| **Interactive debugger and inspection** (TOOL-02) — **promoted to the front** | Breakpoints, a cycle-stamped trace and disassembly over the existing side-effect-free inspection path; converts tier-4 STIC/CPU claims from golden-compared into inspectable and falsifiable | Its "needs replay" gate is **met** (v0.2.0) and v0.3.0 gives it a consumer (the host and the owner route). No external gate; the work is a scripted, headless-verifiable command surface |
| **Variant profiles** (VAR-01..04) | Alias vs distinct profile decided by image comparison; the first profiles beyond the family-level 2609 NTSC | User-supplied system images plus a rights review for the first three; **PAL has no evidence at all** — it needs a primary data sheet or a measurement (D10) |
| **Content formats with persistence** (FMT-02, FMT-03) | Intellicart/CC3/JLP/LTO Flash shapes and their persistence; a hash-keyed content database | A rights review per format, and a persistence story that reverses OQ8. **Demoted** below the debugger because it is larger than one milestone and still rights-gated |
| **Bank modes as profile data** (FMT-01, narrowed) | A typed refusal for every bank mode the profile does not name, with one generated original fixture per accepted mode | A re-derived registry baseline and a cleared cost reading; v0.3.0 refuses `.cfg` by name, so this waits on a mode description the project may write itself |
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
<summary>🚧 v0.3.0 Owner-Supplied Content and a Driveable Host (Phases 13 to 16)</summary>

- [ ] Phase 13: Rights Ledger and Enforced Claim Ceiling — one question-shaped rights row per content question, an unsourced-`cleared` refusal, a refused-spelling refusal, D-32's `established`-claim check, and the ten D-09-06 mechanisms registered
- [ ] Phase 14: Cartridge Route and Real-Content Conformance — a build-only wrap at an operator-stated base with seven typed refusals each beside its positive, a cartridge role in the owner-local route, and the first real-content blocker fixed, pinned and retargeted
- [ ] Phase 15: Build-Only Host and the Fake-Frontend Harness — a shared object outside the core and the install list, proved by a harness that reproduces the runner's nine digest lines, guarded three ways (droppable)
- [ ] Phase 16: Owner Route, Claim and Release Posture — one documented route from the source archive, handoff rows, shared objects refused by name, and a recomputed release decision

</details>

<details>
<summary>✅ v0.2.0 Registered Evidence, Reachable Runs, Replayable Input (Phases 9 to 12) — SHIPPED 2026-11-25</summary>

- [x] Phase 9: Registered Evidence and Gate Cost — land the cost lever and trims, take the hosted row that settles the level, then register the 17 already-faulted requirements and make the gap unrepeatable
- [x] Phase 10: Reachable Runs and Presentation Policy — a person builds and runs the demo cartridge from the released source archive with documented commands; the presentation policy is frozen and enforced; the look/listen handoff is recorded
- [x] Phase 11: Input Record and Replay — an input-only, rights-clean log whose replay reproduces the recorded run's digest chain and applied-event chain, with the vacuity guards proved live
- [x] Phase 12: Bank Modes and External Import — **DROPPED**, not delivered: the rights review cannot clear the importer (its only source is a GPL one) and the recorded cost level is above the stop line; entry-gate decision in `.planning/phases/12-bank-modes-and-external-import/12-ENTRY-GATE.md`, FMT-01 and FMT-04 `Deferred` with their reversal conditions — bank-mode profile data with a typed refusal for every unmapped mode, and `.bin`/`.cfg` import outside the core only where a per-mode rights review clears it; droppable

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
| D16 The `.bin`/`.cfg` importer lives outside the core; its refusals are tool tokens | Architecture + rights | Delegated decision 2026-11-24; **narrowed by D-47** — a flat image at an operator-stated address is accepted, `.cfg` stays refused by name | Phase 14 |
| D-46 The rights ledger is a second, question-shaped ledger | Rights + architecture | Delegated decision 2026-11-26 (`.planning/MILESTONE-CONTEXT.md` §3) | Phase 13 |
| D-47 The flat wrap states bytes, never a format | Rights + architecture | Delegated decision 2026-11-26 | Phase 14 |
| D-48 The host is build-only with a project-authored ABI subset | Rights + architecture | Delegated decision 2026-11-26 | Phase 15 |
| D-49 The package stays static and eleven files; shared objects refused by name | Architecture + rights | Delegated decision 2026-11-26 | Phase 16 |
| D-50 The claim ceiling is enforced mechanically | Evidence | Delegated decision 2026-11-26 | Phase 13 |
| OQ8 Snapshot persistence policy | Architecture | Delegated decision (Phase 8) | Done |
| OQ11 Performance targets | Architecture | Delegated decision (PRF-01 sets none) | Done |

## Phase Details

**Milestone v0.3.0.** The phase shape, exit criteria, decisions (D-46 to D-50),
verification strategy, tier ceilings and risks are frozen in
`.planning/MILESTONE-CONTEXT.md`; the retarget table and the definition of done
are `.planning/ops/GUARDRAILS-v0.3.md`. The milestone plans about 18 to 22 plans
across four phases.

Four rules hold across every phase below: **zero new gate steps, faults or
goldens** (38 / 238 / 29 stay; every capability lands as ids inside a suite that
already exists), **the window is never the evidence** (the fake frontend is),
**`steps=38/38` is an invariant**, and **no claim above `provisional`**, with
D-50's check as the enforcement.

### Phase 13: Rights Ledger and Enforced Claim Ceiling

**Goal**: Every third-party-content question this milestone touches has a
written disposition before any byte is read, a refused item cannot be named in
code or in an operator page without failing the gate, and the claim ceiling is
mechanical rather than quoted.
**Depends on**: Nothing (first phase of v0.3.0; continues from the shipped Phase 11)
**Requirements**: RTS-01, RTS-02, EV-06, EV-07
**Success Criteria** (what must be TRUE):
  1. `data/rights/` holds one row per item the milestone touches — flat image, `.cfg`, bank modes, EXEC, GROM, commercial ROM, frame render or screenshot, save state, the libretro-shaped ABI and core-info strings — each `cleared`, `refused` or `owner-local-only`, with a `SRC-` id resolvable in `data/expected/sources.ids` when the row is `cleared`, and a non-empty "what was taken" cell; a row missing any of those is refused by name.
  2. A `cleared` row with no source is refused, and a refused item's spellings are refused in `core/src`, `runner/`, `tools/` and the operator pages of `docs/`, except where a pre-existing refusal surface is pinned by a `(token, path)` pair; both limbs are shown able to fail by a committed control.
  3. The claims step refuses an `established` claim that is not a `source-expectation` claim resting on a `tier1-primary-doc` dependency, and a legal `established` claim still passes; both are shown able to fail.
  4. Each of the ten D-09-06 mechanisms has an `important-gates.ids` `mutant` row over its existing selftest, the registry baseline is re-derived in the same commit, all ten toy registries gain exactly one line each with zero deletions, and `faults` stays 238. *Amended by plan 13-04 (recorded deviation, `13-04-SUMMARY.md` and `STATE.md` `[Phase 13]: 13-04`): the registration is six `control` rows and four `mutant` rows, because five mechanisms are already proved by control directories that exist and one is proved only as a co-firing `EXPECT` line, so a `mutant` row for them would be a second registration of one proof. The baseline was re-derived in the same commit, the ten toy registries gained one line each, and `faults` stayed 238.*
  5. The gate ends `steps=38/38` with a predicted and then measured token delta, and one hosted row is recorded after the content lands with the last-five `cpu_s` median recomputed against the 21 cpu-s allowance.
**Plans**: 4/5 plans executed
Plans:
**Wave 1**
- [x] 13-01-PLAN.md — the rights-ledger limb end to end: the checker, the inventory, one live row and the `rights-ledger` control (RTS-01)

**Wave 2** *(blocked on Wave 1 completion)*
- [x] 13-02-PLAN.md — the nine remaining RTS-01 rows and their operator rows (RTS-01)

**Wave 3** *(blocked on Wave 2 completion)*
- [x] 13-03-PLAN.md — the naming limb with its `(token, path)` pins, and the enforced claim ceiling (RTS-02, EV-06)

**Wave 4** *(blocked on Wave 3 completion)*
- [x] 13-04-PLAN.md — the ten D-09-06 registrations, the re-derived baseline and the 13-file toy extension (EV-07)

**Wave 5** *(blocked on Wave 4 completion)*
- [ ] 13-05-PLAN.md — the phase close: two-shell gate, token-line reconciliation, the recomputed cost checkpoint and the validation sign-off
**Tier ceiling**: none — this phase adds no claim.

### Phase 14: Cartridge Route and Real-Content Conformance

**Goal**: A person can wrap a flat cartridge image of their own, at an address
they state, into the project's cartridge container and run it through the
owner-local route — with nothing parsed, nothing described from another
implementation and nothing committed — and the first blocker that run exposes is
fixed, pinned and retargeted rather than recorded and abandoned.
**Depends on**: Phase 13 (a rights row must exist before a byte is read)
**Requirements**: TOOL-07, SCN-03, CPU-06
**Success Criteria** (what must be TRUE):
  1. `tools/flatwrap <flat> <base> <out> [--word-order hi-first|lo-first]` is build-only, absent from the install list and from `data/expected/pkg-install-files.ids`, and states only a byte length, a word order and an address the operator gave it; it names no other tool's format and auto-detects nothing.
  2. Seven typed refusals, pinned append-only in `data/expected/flat-refusals.ids`, each beside its boundary positive: odd↔even length, a word above `$03FF`↔`$03FF`, 65,537↔65,536 words, base `$3FFF`↔`$4000`, crossing `$10000`↔ending at it, output inside the work tree↔outside, existing↔new output.
  3. A wrapped image round-trips: it loads as `content cart user`, `slot.cart.sha256` equals the hash printed on stdout, and flipping one input byte changes that hash (so the equality is proved live).
  4. The owner-local route gains a cartridge role; all six derived-output refusals and the snapshot rule fire for it, one line each and no file, and `docs/owner-images.md` no longer says the route has no cartridge role.
  5. No tracked file carries a new binary extension, `binary-ext.txt` and `.gitignore` are unchanged by this phase, and the gate ends `steps=38/38` with the predicted `tests-expected` and `tests-skipped` delta.
  6. The interrupt-enable default is resolved from the owner-local observation (`cpu.interrupt_accept_clears_intff` = 1, D-51), a project-authored CPU test pins **both** settings so the not-cleared case is still exercised, the value stays `unknown-default-chosen` with the observation in `data/discrepancies.json` C41, and the profile parameter digest, every snapshot literal and the boot-stub-2 interrupt expectations move in the same commit with a predicted delta.
  7. The next blocker the same run exposes is recorded with its own row and its own gate: measured, the display-enable strobe never latches, so frames stay black and the STIC register-window / strobe rule is the next tier-4 default to resolve — named, not silently carried.
**Plans**: 4/4 plans executed
Plans:
**Wave 1** *(two plans in parallel: no shared file, one touches `tools/` and one touches the profile)*
- [x] 14-01-PLAN.md — the wrap tool end to end: one flat file at a stated base becomes a container the runner loads as `content cart user`, then seven typed refusals each beside its boundary positive, with the build wiring, the SPDX alternation and one control (TOOL-07)
- [x] 14-02-PLAN.md — the interrupt-enable default: the field at 1 with its tag unchanged, the C default agreeing with the profile, five explicit vector overrides, the digest and every snapshot literal re-pinned, `run.stub2.jd-as-j` re-expressed and `stic.irq.level-held-reentry-c41-0` re-targeted in one commit, plus C82's next-blocker record (CPU-06)

**Wave 2** *(blocked on 14-01: needs the tool to wrap with)*
- [x] 14-03-PLAN.md — the cartridge role: the six derived-output refusals and the snapshot rule for `content cart user`, the round trip's digest equality proved live, the route's `--cart` option, its pass path on project-made files with a one-byte EXEC mutation, and the two operator pages (SCN-03)

**Wave 3** *(blocked on 14-01, 14-02 and 14-03)*
- [x] 14-04-PLAN.md — the phase close: three requirement flips with five live mechanism rows, the 13-file toy extension, the re-derived `gates=`/`lines=`, the two-shell and container gate evidence, the recomputed cost checkpoint and the validation sign-off
**Tier ceiling**: none — the wrap and the round trip are repeatability `[nc]`, and a resolved default is an observation, never a claim.

### Phase 15: Build-Only Host and the Fake-Frontend Harness

**Goal**: The core can be driven in real time by a host that loads it as a
shared object, and the proof is a headless frontend that reproduces the runner's
nine digest lines — not a window.
**Depends on**: Phase 13 (rights rows for the ABI and the palette before code)
**Requirements**: TOOL-08
**Success Criteria** (what must be TRUE):
  1. One shared object is built outside `core/` and outside the install list, exporting only its shaped entry points and no `ivs_` or palette symbol; the export surface is refused by name if it grows.
  2. An automated fake frontend `dlopen`s it, loads one project cartridge with the boot stub and zero GROM, drives the pass scenario's six events, and prints nine digest lines equal byte-for-byte to `data/golden/demo-lamp-tone-pass.digests`.
  3. The equality is guarded three ways: every line non-`-` with the observed set equal; a second id with an empty poll must give a **different** set; and two stand-ins are refused — a stub printing the golden with no frame (`host-vacuous`) and a mutant with the input poll dropped (`host-digest-diff`).
  4. The host holds no palette of its own (it links `runner/frame_export.c`), opens no file but the images named on its command line, and the installed package is unchanged: eleven files, with a selftest that the shared object is absent.
  5. `so` and `dylib` enter `tools/check/binary-ext.txt` with the controls that refuse a tracked shared object, and the gate ends `steps=38/38` with the predicted delta.
**Plans**: 3/3 plans executed in three waves

**Wave 1** *(no dependency)*
- [x] 15-01-PLAN.md — the seam, end to end (tracer): the build-only shared target with a linker-enforced export list generated from `data/expected/host-symbols.ids`, the project-authored `host/libretro_shim.h` and `host/ivs_retro_shim.c`, the `dlopen` harness that drives `lamp-tone.ivsc` to tick 806568 and prints the nine digest lines, `tools/check/host-symbols.sh` refusing the export surface in both directions, and the integrity alternations extended to `host/` (TOOL-08)

**Wave 2** *(blocked on 15-01)*
- [x] 15-02-PLAN.md — the shaped-entry assertions, the empty-poll id and the two committed stand-ins, then GUARDRAILS V6's whole retarget set in one commit: `so dylib dll bundle` in the one extension list, two controls, the `pkg-install` selftest that the object is absent, one `release-assets` case, and the two "not installed, not shipped" record lines (TOOL-08)

**Wave 3** *(blocked on 15-01 and 15-02)*
- [x] 15-03-PLAN.md — the phase close: the TOOL-08 flip with its two live `judge` rows, the thirteen-file toy extension, the one `deterministic-repeatability` claim with GUARDRAILS V3's whole retarget set, the re-derived `gates=`/`lines=`, the two-shell and container gate evidence, the recomputed cost checkpoint and the validation sign-off (TOOL-08)

**Tier ceiling**: `deterministic-repeatability`, provisional — the same core compared with itself, never an independent comparison.
**Droppable**: yes. The pre-authorized fallback is a content-only v0.3.0; the goal is amended in the close record and TOOL-08 is recorded `Deferred` with its reversal condition.

### Phase 16: Owner Route, Claim and Release Posture

**Goal**: The owner route from the source archive is documented and rehearsed,
the one question only the owner can answer is a handoff row, and the release
posture states plainly that no host artifact and no content ships.
**Depends on**: Phases 13, 14 and 15 (or 13 and 14 under the fallback)
**Requirements**: PKG-06
**Success Criteria** (what must be TRUE):
  1. One documented owner route (EXEC + GROM + a wrapped cartridge, three commands) is rehearsed from the **source archive** to a project-cartridge golden, with the transcript recorded; it is a release-time rehearsal, not a gate step.
  2. The route's pass path is exercised with project-made files — zero-filled 8192- and 2048-byte images are shape-legal — and a one-byte EXEC mutation proves the id can fail; the `exec:` label stays `-` so no run reads as a real-EXEC result.
  3. The owner-only questions are `handoffs.ids` rows, `evidence: none`, dispositioned, cited by no claim and by no phase exit criterion.
  4. A tracked shared object is refused by name in both directions, the installed package stays eleven files, and the release-decision line reads: the release ships the static package and the source archive; no shared object, no host shim, no content.
  5. The concurrency lever is taken in one cost-only commit whose summary token line is byte-identical and whose `step.out.<name>` buffers are compared, and the close records one checkpoint line in the section 2.3 vocabulary with `steps=38/38`.
**Plans**: TBD
**Tier ceiling**: no new claim; the claim is minted in Phase 15.

<details>
<summary>v0.2.0 phase details (archived 2026-11-25)</summary>

**Milestone v0.2.0.** The phase shape, exit criteria, decisions (D14 to D16),
verification strategy, tier ceilings and risks are frozen in
`.planning/milestones/v0.2.0-CONTEXT.md`; the retarget table and the definition of done
are `.planning/ops/GUARDRAILS-v0.2.md`. The milestone planned about 24 plans across
the four phases (18 without Phase 12).

Three rules held across every phase below: **zero new gate steps** (38 stays
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
**Plans**: 5/5 plans executed, 4 waves
Plans:
- [x] 11-01-PLAN.md — `docs/formats/ivsl-v1.md` first: the grammar, the nine refusal tokens, the measured caps, the profile scope and the D15 rights posture (wave 1)
- [x] 11-02-PLAN.md — the tracer: `runner/ivsl.c` + the executor's schedule substitution and applied-prefix recorder + the effective `input.events`, with the equality, the round trip and the two judge self-controls (wave 2)
- [x] 11-03-PLAN.md — the nine `runner.record.refuse.<token>` ids with an untouched-output assertion each, and `runner.record.no-content-digest` with its stand-in (wave 3)
- [x] 11-04-PLAN.md — `--record-input` (exclusive create) and `--replay-input`, and the one process case on `demo-lamp-tone-pass` with its `input` lines stripped (wave 3)
- [x] 11-05-PLAN.md — the two named faults, `TOOL-01`'s close with four registry rows and the toy obligations in one commit, and the phase exit (wave 4)

**Outcome (plan 11-05, 2026-11-25):** two options (`--record-input`, `--replay-input`), fifteen new
ids (nine refusals, `no-content-digest`, the equality, the round trip, the two judge self-controls
and the one process case), two named faults that each fail the equality, and `TOOL-01` `Complete`
with four `runner-record` registry rows: `gates` 109 to 110, `lines` 333 to 337, `faults` and
`fault-runs` 235 to 237, `steps=38/38` unchanged and no claim minted or raised.
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

</details>

## Coverage (v0.3.0)

| Requirement | Phase |
|-------------|-------|
| RTS-01, RTS-02, EV-06, EV-07 | 13 |
| TOOL-07, SCN-03, CPU-06 | 14 |
| TOOL-08 | 15 (droppable) |
| PKG-06 | 16 |

Mapped: 9/9 v0.3.0 requirements, each to exactly one phase. No orphans, no
duplicates. If Phase 15 is dropped, TOOL-08 is recorded `Deferred` with its
reversal condition — never lost, never `Complete`.

## Coverage (v0.2.0 — archived)

| Requirement | Phase |
|-------------|-------|
| EV-01..EV-04 | 9 |
| EV-05, PKG-05, TOOL-04 (narrowed) | 10 |
| TOOL-01 | 11 |
| FMT-01 (narrowed), FMT-04 | 12 (dropped) |

Mapped: 10/10 v0.2.0 requirements; FMT-01 and FMT-04 were dropped at Phase 12's
entry gate and are `Deferred` with their reversal conditions.

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
| 11. Input Record and Replay | 5/5 | Complete    | 2026-11-25 |
| 12. Bank Modes and External Import (conditional) | 0/0 | Dropped (three-phase fallback) | 2026-11-25 |

**v0.2.0 is shipped and archived.** Phases 9 to 11 complete, Phase 12 dropped at
its entry gate, release `v0.2.0` published as a pre-release on the artifact
commit `8f017e7`. Its phase details are collapsed above.

### v0.3.0 — scoped 2026-11-26 (in progress)

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 13. Rights Ledger and Enforced Claim Ceiling | 5/5 | Complete    | 2026-11-25 |
| 14. Cartridge Route and Real-Content Conformance | 4/4 | Complete    | 2026-11-25 |
| 15. Build-Only Host and the Fake-Frontend Harness | 3/3 | Complete    | 2026-11-26 |
| 16. Owner Route, Claim and Release Posture | 0/TBD | Not started | — |

**Current milestone:** v0.3.0, scoped and detailed by this roadmap on 2026-11-26.
Phase 15 is planned in three waves and 2 of 3 plans are executed (15-01 the seam, 15-02 the guards and
the non-member set); 15-03 is the phase close. Phase 15 is droppable under the pre-authorized fallback.
The milestone adds zero new gate steps, faults or goldens, and at most one claim,
`provisional`.
