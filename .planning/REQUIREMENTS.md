# Requirements: iamverysmartvision

**Milestone:** v0.3.0 Owner-Supplied Content and a Driveable Host
**Defined:** 2026-11-26
**Core Value:** A deterministic, observable machine core whose every compatibility claim is bound to an exactly named profile and independent evidence — never an unqualified "it runs Intellivision games" label.
**Scope record:** `.planning/MILESTONE-CONTEXT.md`. **Guardrails:** `.planning/ops/GUARDRAILS-v0.3.md`.

The v0.2.0 requirement set (10 requirements: 8 Complete, 2 Deferred) is archived
in `.planning/milestones/v0.2.0-REQUIREMENTS.md`. The v0.1.0 set (50, all
Complete) is in `.planning/milestones/v0.1.0-REQUIREMENTS.md`. Numbering
continues from them per category; new categories start at 01.

## v0.3.0 Requirements

Requirements for this milestone. Each maps to exactly one roadmap phase. Every
requirement is a hypothesis until delivered and verified, and **no claim may
exceed `provisional`** while D10 and D11 are unperformed (`PROJECT.md` D-32
scopes that ceiling to behavioural claims, and EV-06 makes the scoping
mechanical).

### Rights

- [x] **RTS-01**: A rights ledger records one row per third-party-content question this milestone touches — a flat cartridge image, the `.cfg` container, bank modes, EXEC, GROM, a commercial ROM, a rendered frame or screenshot, a save state, the libretro-shaped ABI and core-info strings — each with a disposition of `cleared`, `refused` or `owner-local-only`, and a `cleared` row cites a resolvable `SRC-` id; a row missing its disposition, a source it needs, or its "what was taken" cell is refused by name.
- [x] **RTS-02**: A refused item's spellings cannot appear in `core/src`, `runner/`, `tools/` or the operator pages of `docs/`, except where a pre-existing refusal surface is pinned by a `(token, path)` pair; a new occurrence is refused by name, and both limbs are shown able to fail.

### Core conformance

- [x] **CPU-06**: The first real-content blocker is resolved from an owner-local observation rather than a guess: an interrupt acceptance clears the CPU's interrupt enable when the profile says so, a project-authored test pins **both** settings (cleared and not cleared), the value stays tagged `unknown-default-chosen` with the observation in the discrepancy log, and the profile parameter digest, every snapshot literal and the boot-stub-2 interrupt expectations move in the same commit.

### Evidence

- [x] **EV-06**: The claims step refuses an `established` claim that is not a `source-expectation` claim resting on a `tier1-primary-doc` dependency, and a legal `established` claim still passes — both shown able to fail (D-32, D-50).
- [x] **EV-07**: Each of the ten must-fail mechanisms that have a liveness guard and a re-runnable stand-in but no registry row (D-09-06) gains a `mutant` row over its existing selftest, with the registry baseline re-derived and all ten toy registries extended in the same commit, and **zero new faults**. Registered as six `control` rows and four `mutant` rows: five mechanisms are proved by control directories that already exist and one by a directory plan 13-04 adds, so a `mutant` row for them would be a second registration of one proof. The deviation from this sentence's literal "mutant row" wording is recorded in `.planning/STATE.md` (`[Phase 13]: 13-04`) and in `13-04-SUMMARY.md`; zero new faults holds.

### Tooling

- [x] **TOOL-07**: A person can wrap a flat cartridge image of their own, at an address they state, into the project's cartridge container with one build-only command, and every malformed input is refused by a typed token with a positive case beside it; the wrapped image is derived material and is never committed, golden, fixture or claim artifact.
- [x] **TOOL-08**: The core can be driven in real time by a host that loads it as a shared object outside the installed package, and an automated frontend reproduces the runner's nine digest lines on a project cartridge — with the vacuity closed three ways and the host holding no palette of its own.

### Scenario and route

- [x] **SCN-03**: The owner-local route accepts a cartridge image alongside the EXEC and GROM images, every derived-output refusal is extended to it, and the route's pass path is exercised on project-made files without any owner image.

### Packaging and release

- [ ] **PKG-06**: A tracked shared object is refused by name, the installed package stays eleven files, and the release-decision line states that no shared object, host shim or content is shipped.

## v2 Requirements

Deferred. Tracked, not in this roadmap. Each becomes its own profile or slice with
its own research and evidence.

### Tooling and integration

- **TOOL-02**: Interactive debugger — breakpoints, cycle-stamped trace, disassembly over the side-effect-free inspection path, driven by a scripted command surface so it stays headless-verifiable. **Promoted to the front of the mid horizon:** its "needs replay" gate is met, and v0.3.0 gives it a consumer.
- **TOOL-03**: LibFuturo adapter, after a real consumer exists and fit is proven (owner gate D13).
- **TOOL-04 (remainder)**: A window, an audio device, physical controller mapping and the packaging story that makes a prebuilt host obtainable. v0.3.0 delivers the host surface *build-only*; shipping a binary is a separate decision (D-49).
- **TOOL-05**: Scheduled fuzzing and extended sanitizer jobs, once a failure class justifies them that no gate step catches.
- **TOOL-06**: Hardware-measured timing evidence for STIC windows, BUSRQ and the controller matrix (owner gate D10).

### Content formats

- **FMT-01** *(narrowed, deferred from v0.2.0)*: Bank-mode behaviour as profile data with a typed refusal for every mode the profile does not name. Reversal: a re-derived registry baseline and a cleared cost reading.
- **FMT-02**: Intellicart / CC3 / JLP / LTO Flash formats and their persistence.
- **FMT-03**: Hash-keyed content database for configuration identity.
- **FMT-04** *(deferred from v0.2.0)*: The `.bin`/`.cfg` importer. Reversal: a permissively-described primary source or an independent `.cfg` description, plus a per-mode rights review with a source id per row. **v0.3.0 narrows the question rather than answering it**: TOOL-07 accepts a flat image at an operator-stated address and refuses `.cfg` by name, so the rights ground no longer blocks *flat* content while the `.cfg` container stays refused.

### Variant profiles

- **VAR-01**: Intellivision II profile (EXEC2 identity, extra decles at `$0400`, lockout behaviour).
- **VAR-02**: Sears Super Video Arcade profile (alias vs distinct profile decided by image comparison).
- **VAR-03**: Tandyvision One / GTE Sylvania aliases after dump comparison.
- **VAR-04**: PAL timing profile (every PAL value is currently unknown).

### Peripherals

- **PER-01**: ECS (second PSG, banked ROM, extra controllers, keyboard).
- **PER-02**: IntelliVoice (SP0256 speech, separate audio stream, user-supplied voice ROM).

## Out of Scope

Explicitly excluded from v0.3.0, with the reason. Documented to prevent scope
creep; the reasons are load-bearing.

| Feature | Reason |
|---------|--------|
| Vendoring the libretro header, or claiming libretro compatibility | The shim needs about six entry points, not a 200-symbol header; a project-authored subset is rights-clean and nothing has to be licensed or noticed (D-48). Vendoring is a later decision with the licence text read and recorded first |
| Shipping the host shim, or any shared object, in a release | The package is static and eleven files; a tracked shared library is refused by name. Shipping one is a separate decision that moves `pkg-install-files.ids` and `docs/packaging.md` together (D-49) |
| Committing, golden-ing or CI-creating a wrapped image or any third-party content | Rights-bearing: a hash establishes identity, not rights. The wrap output is derived material and is refused inside the work tree except `refs/` (D-47) |
| Parsing or describing the `.cfg` container, or implementing bank switching | Its only known description is a GPL implementation this project's research rules exclude. FMT-01 and FMT-04 stay `Deferred` with their conditions |
| A real STIC colour table, or a palette inside the core | Presentation is a host decision; the one palette is the labelled operator preview palette, linked and never copied, and it is never a golden, digest, assert or claim |
| Any new gate step, fault or golden | 38 steps, 238 faults and 29 goldens stay; the allowance is 21 hosted cpu-s and a step is the expensive unit |
| Any claim above `provisional` | D10 and D11 are unperformed; no measured-hardware tier exists, and D-50's check is what enforces the behavioural ceiling |
| "Intellivision" in a package, product or repository name | Third-party trademark |

## Traceability

Which phase covers which requirement. Filled at roadmap creation
(2026-11-26) and verified against the `.planning/ROADMAP.md` Phase Details on
the same date: every requirement appears in exactly one phase.

| Requirement | Phase | Status |
|-------------|-------|--------|
| RTS-01 | Phase 13 | Complete |
| RTS-02 | Phase 13 | Complete |
| EV-06 | Phase 13 | Complete |
| EV-07 | Phase 13 | Complete |
| TOOL-07 | Phase 14 | Complete |
| SCN-03 | Phase 14 | Complete |
| CPU-06 | Phase 14 | Complete |
| TOOL-08 | Phase 15 | Complete |
| PKG-06 | Phase 16 | Pending |

**Coverage:**
- v0.3.0 requirements: 9 total
- Mapped to phases: 9
- Unmapped: 0 ✓
- Phases: 13 (RTS-01, RTS-02, EV-06, EV-07), 14 (TOOL-07, SCN-03, CPU-06), 15 (TOOL-08), 16 (PKG-06)
- CPU-06 was added on 2026-11-26, after the first owner-local real-content run failed deterministically; it is the evidence that a host built before the EXEC path is fixed would show a black screen
- Phase 15 was the droppable phase under the pre-authorized fallback; it was not dropped, and it
  delivered: TOOL-08 is `Complete` on the phase's own evidence (a build-only shared object, a
  `dlopen` frontend reproducing the runner's nine digest lines on the project's own cartridge, the
  vacuity closed three ways and one `deterministic-repeatability` claim, `CL-0029`, standing
  `provisional`). The fallback remains written down so the choice is made by the rule and not under
  pressure: a future re-open that cannot be made green records TOOL-08 `Deferred` with its reversal
  condition and amends the milestone goal to "Owner-Supplied Content" — never silently lost and
  never marked `Complete`.

---
*Requirements defined: 2026-11-26*
*Last updated: 2026-11-26 at Phase 15 completion (TOOL-08 flipped to `Complete` with the two
`important-gates` `mutant` rows that name it under the new `host` gate name, in one commit; the checkbox
lines and the traceability table move together because the table is the parsed record)*
