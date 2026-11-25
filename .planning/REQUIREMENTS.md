# Requirements: iamverysmartvision

**Milestone:** v0.2.0 Registered Evidence, Reachable Runs, Replayable Input
**Defined:** 2026-11-24
**Core Value:** A deterministic, observable machine core whose every compatibility claim is bound to an exactly named profile and independent evidence — never an unqualified "it runs Intellivision games" label.
**Scope record:** `.planning/MILESTONE-CONTEXT.md`. **Guardrails:** `.planning/ops/GUARDRAILS-v0.2.md`.

The v0.1.0 requirement set (50 requirements, all Complete on their recorded
terms) is archived in `.planning/milestones/v0.1.0-REQUIREMENTS.md`. Numbering
continues from it per category; new categories start at 01.

## v0.2.0 Requirements

Requirements for this milestone. Each maps to exactly one roadmap phase. Every
requirement is a hypothesis until delivered and verified, and **no claim may
exceed `provisional`** while D10 and D11 are unperformed.

### Evidence durability and gate cost

- [ ] **EV-01**: The gate cost lever (3) and trims (a), (c) and (d) land in cost-only commits — with trim (b) declined by measurement — before any new test id, control, fault or gate step, and a hosted row recorded after them settles the cost level.
- [x] **EV-02**: The 17 requirements whose named faults already run in every gate (FND-01..07, CPU-01..05, MEM-01..05) each have at least one cannot-pass-vacuously mechanism registered in `data/expected/important-gates.ids`, with the accepted-requirement set and all ten toy registries extended in the same commit and no new fault or test id.
- [ ] **EV-03**: Each gate-hardening item (`SRC-` whole-token match, zero-document failures, per-step pass lines, manifest output anchoring, memmap generator controls) has a control that fails for its intended reason.
- [ ] **EV-04**: A machine check refuses a requirement marked Complete that has no registry line, and refuses a toy registry that drifts from the good tree, and both are shown able to fail.
- [ ] **EV-05**: A handoff ledger (`data/expected/handoffs.ids`) records every human-only item (H7-1, H7-2, H8-3, D10, D11) with `evidence: none`, a claim may not cite a handoff artifact, and a later milestone cites the row instead of re-asking.

### Packaging and reachability

- [ ] **PKG-05**: A person can build and run the emulator from the released source archive using documented commands only, and the release rehearsal proves that route end to end against the demo cartridge golden from a moved prefix.

### Tooling

- [ ] **TOOL-01**: A run can be recorded as an input-only log and replayed at machine-time boundaries, and the replay reproduces the recorded run's digest chain and applied-event chain, guarded against a vacuous match.
- [ ] **TOOL-04** *(narrowed)*: A frozen presentation and palette policy keeps colour, scaling and aspect out of the core and out of every claim, and a check refuses a colour symbol in the core.

### Content formats

- [ ] **FMT-01** *(narrowed)*: Bank-mode behaviour is profile data with a typed refusal for every mode the profile does not name — never a silent default — and one generated original fixture per mode proves a witness pair and a no-switch case.
- [ ] **FMT-04**: `.bin`/`.cfg` import lives outside the core, refuses every unmapped or unsupported mode by name, writes nothing inside the work tree, and is gated per mode by a rights review that can degrade the phase to `FMT-01` only.

## v2 Requirements

Deferred. Tracked, not in this roadmap. Each becomes its own profile or slice with
its own research and evidence.

### Tooling and integration

- **TOOL-02**: Interactive debugger — breakpoints, cycle-stamped trace, disassembly over the side-effect-free inspection path, driven by a scripted command surface so it stays headless-verifiable.
- **TOOL-03**: LibFuturo adapter, after a real consumer exists and fit is proven (owner gate D13).
- **TOOL-04 (remainder)**: The reference host frontend proper — window, audio device, physical controller mapping — outside the core build graph, and the packaging story that makes it obtainable.
- **TOOL-05**: Scheduled fuzzing and extended sanitizer jobs, once a failure class justifies them that no gate step catches.
- **TOOL-06**: Hardware-measured timing evidence for STIC windows, BUSRQ and the controller matrix (owner gate D10).

### Content formats

- **FMT-02**: Intellicart / CC3 / JLP / LTO Flash formats and their persistence.
- **FMT-03**: Hash-keyed content database for configuration identity.

### Variant profiles

- **VAR-01**: Intellivision II profile (EXEC2 identity, extra decles at `$0400`, lockout behaviour).
- **VAR-02**: Sears Super Video Arcade profile (alias vs distinct profile decided by image comparison).
- **VAR-03**: Tandyvision One / GTE Sylvania aliases after dump comparison.
- **VAR-04**: PAL timing profile (every PAL value is currently unknown).

### Peripherals

- **PER-01**: ECS (second PSG, banked ROM, extra controllers, keyboard).
- **PER-02**: IntelliVoice (SP0256 speech, separate audio stream, user-supplied voice ROM).

## Out of Scope

Explicitly excluded from v0.2.0, with the reason. Documented to prevent scope
creep; the reasons are load-bearing.

| Feature | Reason |
|---------|--------|
| Installing `ivs-run` or any cartridge image into the release package | Structurally blocked, not merely expensive: an installed runner has nothing to run (every cartridge is a build product of `ivs_pack`/`ivs_asm`, which are never shipped), and any release member with a binary extension is refused whatever the list says (D14) |
| A windowed frontend or any new runtime dependency | First genuinely new runtime dependency; a permanent CI blind spot (a window is never evidence); palette risks becoming a de-facto STIC-colour claim. Needs a named consumer and an images story that survives the release-asset check (D14 reversal evidence) |
| A second example cartridge or program | Lamp Tone already runs end to end; a new program is three or more plans and new gate ids for a presentation gain that belongs to the handoff's result, not to this phase |
| Any new gate step | The cost rule; a step is the expensive unit and an id is the cheap one. Reversal requires new ids to exceed about 15 percent of an existing suite or a defect found only at a capability seam |
| A real STIC colour table or any colour claim | Presentation is a host decision; a real table would be a profile field with a source tier and its own evidence row |
| Copying a `.cfg`/`.bin` format description from an emulator's code, tables or constants | FND-05 and the licences; a per-fact rights review written in the project's own words is the only accepted route |
| Committing any `.bin`, `.cfg`, `.rom`, `.ivsc` or third-party cartridge | Rights-bearing; a hash establishes identity, not rights. Fixtures are generated from tracked sources into the build tree |
| Any claim above `provisional` | D10 and D11 are unperformed; no measured-hardware tier exists |
| Silently defaulting an unknown bank mode, peripheral or memory map | Hides configuration identity; always a typed refusal |
| "Intellivision" in a package, product or repository name | Third-party trademark |

## Traceability

Which phase covers which requirement. Filled at roadmap creation
(2026-11-24) and verified against the `.planning/ROADMAP.md` Phase Details on
the same date: every requirement appears in exactly one phase, in the phase the
roadmap names.

| Requirement | Phase | Status |
|-------------|-------|--------|
| EV-01 | Phase 9 | Pending |
| EV-02 | Phase 9 | Complete |
| EV-03 | Phase 9 | Pending |
| EV-04 | Phase 9 | Pending |
| EV-05 | Phase 10 | Pending |
| PKG-05 | Phase 10 | Pending |
| TOOL-04 | Phase 10 | Pending |
| TOOL-01 | Phase 11 | Pending |
| FMT-01 | Phase 12 | Pending |
| FMT-04 | Phase 12 | Pending |

**Coverage:**
- v0.2.0 requirements: 10 total
- Mapped to phases: 10
- Unmapped: 0 ✓
- Phases: 9 (EV-01..EV-04), 10 (PKG-05, TOOL-04, EV-05), 11 (TOOL-01), 12 (FMT-01, FMT-04)
- Phase 12 is conditional and droppable: if it is dropped, FMT-01 and FMT-04 are
  recorded here as partial or deferred with the condition, never silently lost

---
*Requirements defined: 2026-11-24*
*Last updated: 2026-11-24 at milestone definition*
