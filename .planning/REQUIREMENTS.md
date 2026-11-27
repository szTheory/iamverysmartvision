# Requirements: iamverysmartvision

**Milestone:** v0.4.0 Inspectable Machine, One Default Decided
**Defined:** 2026-11-26
**Core Value:** A deterministic, observable machine core whose every compatibility claim is bound to an exactly named profile and independent evidence — never an unqualified "it runs Intellivision games" label.
**Scope record:** `.planning/MILESTONE-CONTEXT.md`. **Guardrails:** `.planning/ops/GUARDRAILS-v0.4.md`.

The v0.3.0 set (9 requirements, all Complete) is archived in
`.planning/milestones/v0.3.0-REQUIREMENTS.md`; v0.2.0 (8 Complete, 2 Deferred) and
v0.1.0 (50 Complete) in their own archives. Numbering continues from them per
category; new categories start at 01.

**This milestone mints no claim.** `claims=29` and `provisional=28` are
invariants, `CL-0002` stays the only `established` claim, and every requirement
below is a hypothesis until delivered and verified.

## v0.4.0 Requirements

### Evidence and records

- [x] **EV-08**: The milestone starts on a measured ledger rather than a quoted one: the hosted cost window (the last-five `cpu_s`, `build_s` and `gate_s` medians, with the token line) is recomputed by arithmetic from `.planning/ci-runs.tsv` at the plan's own start commit and **before** anything is priced — the seven release-records rows that were outstanding at the milestone opening are recorded in that file at the opening commit and verified, never re-appended; the eight `flatwrap` refusal tokens are documented under `docs/`; one committed test id guards the `flatwrap`-to-shim join and is shown able to fail; the "no host shim ships" sentence is corrected to name the source archive; and the count-gate contract — every count gate names the file it counts, every baseline is measured at the plan's own start commit — is recorded as a numbered lesson **with a mechanism**.

### Tooling and integration

- [x] **TOOL-02** *(promoted from the v2 deferred list)*: A build-only, scripted, headless inspection surface exists over the seams the core already exposes: disassembly of the project's own listings, breakpoints with an asserted hit and non-hit count, and a cycle-stamped access trace whose records are non-empty, cycle-monotonic, end at the report's own step tick and leave every digest byte-identical to the same run untraced. It is never installed, never enters the eleven-path prefix, and adds no gate step.
- [x] **TOOL-09**: Trace and disassembly output over operator content is an operator-local, rights-bearing artifact: one question-shaped `data/rights/` row with a disposition and a "what was taken" cell, an output path inside the work tree refused by name except under the ignored `refs/`, an `ivst` file refused by name in the binary-extension list and `.gitignore`, and no trace or disassembly ever committed as a golden, fixture, CI artifact or claim artifact — whatever its content.

### Video and the display path

- [x] **VID-07**: The four rival display-path rules — the register-window length, the gating rule when the display is off, whether `$0020` strobes on a read or only on a write, and whether the display latch is cleared at each frame start — are separated by four authored guests whose observations are **diagonal** across the 16-cell sweep (each guest moves under its own field and no other, at the default of the remaining three), each with a liveness guard a committed stand-in can fail. Then either the decided default is pinned by one test per setting and every dependent expectation (profile tag, profile-field inventory, parameter digest, every snapshot literal, any moved golden retargeted in place, the append-only discrepancy note) moves in one commit, or the record reads "default unchanged, observation appended, rivals named". The field stays tagged `unknown-default-chosen` in either case.
- [ ] **VID-08**: The D10-VID-08 hardware question is a named packet rather than a wish: the exact trace lines a hardware probe must contradict are written down, the row exists in `data/expected/handoffs.ids` with `evidence: none`, and no claim cites it and no phase exit criterion names it.

## v2 Requirements

Deferred. Tracked, not in this roadmap. Each becomes its own profile or slice with
its own research and evidence.

### Tooling and integration

- **TOOL-03**: LibFuturo adapter, after a real consumer exists and fit is proven (owner gate D13).
- **TOOL-04 (remainder)**: A window, an audio device, physical controller mapping and the packaging story that makes a prebuilt host obtainable. v0.3.0 delivered the host surface *build-only*; shipping a binary is a separate decision (D-49), and v0.4.0's instrument gives the next frontend step something to read.
- **TOOL-05**: Scheduled fuzzing and extended sanitizer jobs, once a failure class justifies them that no gate step catches.
- **TOOL-06**: Hardware-measured timing evidence for STIC windows, BUSRQ and the controller matrix (owner gate D10).

### Content formats

- **FMT-01** *(narrowed, deferred from v0.2.0)*: Bank-mode behaviour as profile data with a typed refusal for every mode the profile does not name. Reversal: a re-derived registry baseline and a cleared cost reading.
- **FMT-02**: Intellicart / CC3 / JLP / LTO Flash formats and their persistence.
- **FMT-03**: Hash-keyed content database for configuration identity.
- **FMT-04** *(deferred from v0.2.0)*: The `.bin`/`.cfg` importer. Reversal: a permissively-described primary source or an independent `.cfg` description, plus a per-mode rights review with a source id per row. The flat-image route accepts an operator-stated address and refuses `.cfg` by name, so the rights ground no longer blocks *flat* content while the `.cfg` container stays refused.

### Variant profiles

- **VAR-01**: Intellivision II profile (EXEC2 identity, extra decles at `$0400`, lockout behaviour).
- **VAR-02**: Sears Super Video Arcade profile (alias vs distinct profile decided by image comparison).
- **VAR-03**: Tandyvision One / GTE Sylvania aliases after dump comparison.
- **VAR-04**: PAL timing profile (every PAL value is currently unknown).

### Peripherals

- **PER-01**: ECS (second PSG, banked ROM, extra controllers, keyboard).
- **PER-02**: IntelliVoice (SP0256 speech, separate audio stream, user-supplied voice ROM).

## Out of Scope

Explicitly excluded from v0.4.0, with the reason. Documented to prevent scope
creep; the reasons are load-bearing.

| Feature | Reason |
|---------|--------|
| A window, an audio device, physical controller mapping or a prebuilt host binary | D-49 ships the static package and the source archive only; a window is never the evidence, and a person's judgement of a picture is a handoff row, not a criterion |
| Shipping or installing the inspection tool | It is build-only by D-52: its own target, no install rule, outside the eleven-path prefix, refused in the install archive three ways |
| Committing a trace, a disassembly dump or any content byte | D-53: rights-bearing and operator-local. `ivst` is refused by name and the claim-artifact predicate refuses it |
| A default chosen because it makes a frame appear | D-54: that is a probe result, not a discrimination. A fitted default is worse than an unresolved one because it retargets the digest and every literal and then reads as settled |
| A discrepancy burn-down target | 110 rows and 101 open is a count; a count going down is not evidence. Rows close when their resolver exists |
| Parsing or describing the `.cfg` container, or implementing bank switching | Its only known description is a GPL implementation this project's research rules exclude. FMT-01 and FMT-04 stay `Deferred` with their conditions |
| Any new gate step, fault, golden or claim | 38 steps, 238 faults, 29 goldens and 29 claims stay; the allowance is 21 hosted cpu-s and the milestone prices 4.72 |
| "Intellivision" in a package, product or repository name | Third-party trademark |

## Traceability

Which phase covers which requirement. Filled at roadmap creation and verified
against the `.planning/ROADMAP.md` Phase Details on the same date: every
requirement appears in exactly one phase.

| Requirement | Phase | Status |
|-------------|-------|--------|
| EV-08 | Phase 17 | Complete |
| TOOL-02 | Phase 18 | Complete |
| TOOL-09 | Phase 18 | Complete |
| VID-07 | Phase 19 | Complete |
| VID-08 | Phase 20 | Pending |

**Coverage:**
- v0.4.0 requirements: 5 total
- Mapped to phases: 5
- Unmapped: 0

**Droppable, and what it costs the coverage:** Phase 18's disassembly limb
(`18-03`) may be dropped without touching a requirement's status — TOOL-02 exits
with the reduced surface named and a reader falls back to the committed listings.
The trace limb is **not** droppable: dropping it defers Phase 19 whole to v0.4.1,
and VID-07 is then recorded `Deferred` with its reversal condition, never silently
lost and never `Complete`. Phase 19's second exit ("default unchanged, observation
appended, rivals named") **is** `Complete` for VID-07 on its own stated terms.

---
*Requirements defined: 2026-11-26*
*Last updated: 2026-11-26 by plan 18-02, with TOOL-02 and TOOL-09 Complete once
their five registry rows, the two build-only proofs and the rights row landed in
one commit. TOOL-02's disassembly limb is plan `18-03`; if that limb is dropped,
the drop is recorded in the frozen page (`docs/inspection.md`) and in `18-03`'s
record, TOOL-02 exits with the reduced surface named, and this status is not
retracted — the flip therefore does not depend on a droppable plan.
*2026-11-27, plan 19-03:* VID-07 is `Complete` on **Exit B**'s own stated terms —
"default unchanged, observation appended, rivals named". The sixteen-cell sweep is
diagonal (each of the four guests moves under its own field and no other), no cell
prefers a setting, the four rival rows (C1, C6, C82, C94) carry the appended
observation, no claim cites it, and every display-path field stays tagged
`unknown-default-chosen` because an observation is not a hardware probe (D-51). The
six `stic-disc` registry rows landed in the same commit as this flip, because
`important-gates` derives its accepted set from these records.*
