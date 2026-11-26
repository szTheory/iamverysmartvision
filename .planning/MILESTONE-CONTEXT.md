# Milestone v0.4.0 — context and upstream scope

**Status:** frozen upstream artifact, 2026-11-26. **This file always describes
the CURRENT milestone.** The next milestone's ritual replaces it wholesale (this
copy is archived as `.planning/milestones/<version>-CONTEXT.md` at that
milestone's close), so `gsd-new-milestone` must never read it as a proposal for
the milestone after this one. Written before any v0.4.0 phase plan exists.
Produced by the milestone ritual (`.planning/ops/MILESTONE-RITUAL.md`): a
four-lens milestone-choice fan-out followed by a five-point upstream scoping
fan-out, synthesized and auto-selected under the owner's delegation
(`.planning/AUTONOMY.md` sections 1 and 2). The five scoping outputs are frozen
alongside this file in `.planning/research/v0.4.0-scoping-*.md`.

**Name:** Inspectable Machine, One Default Decided
**GSD label:** v0.4.0. **Software version:** 0.4.0 if released (a minor bump
under D-30: a new build-only tool and a decided default; no API break, no format
change).
**Phases:** 17 to 20. **Branch slugs:** `gsd/phase-17-…` through
`gsd/phase-20-…`. **Plans:** about 11 to 13.

---

## 1. Why this milestone, and why in this order

v0.3.0 answered "can a person put real content in and drive it": the rights
ledger, the flat wrap, the cartridge role, the build-only host shim, the
fake-frontend harness and the owner route all shipped, and the first
real-content blocker — an interrupt-enable default that ended the EXEC at
`$FFFF` — was fixed and pinned. [OBS] It left one question open, and it is the
owner's own question: **the picture is still black.**

The recorded observation: with `cpu.interrupt_accept_clears_intff` at 1 the
owner's own EXEC runs its handler linearly, populates BACKTAB with real card
descriptors, and then the display-enable strobe written at `$0020` never
latches. [OBS] `data/discrepancies.json` records it as an observation that mints
no claim, names D10-VID-08 as the hardware resolver, and says in its own note
that the register-window rule is "the next tier-4 default to resolve".

**Four rival rules could explain it, and the project cannot currently tell them
apart.** The register-window length (2000 against 2900 cycles), the
gating-when-the-display-is-off rule (`apply` against `open`), whether `$0020`
strobes on a read or only on a write, and whether the display latch is cleared at
every frame start or persists — all four are `unknown-default-chosen` profile
fields, and every one of them is exercised today only by tests that compare the
core against expectations the project itself derived. [OBS]

That is the shape of the problem: **any fix made now is fitted to one
observation, not discriminated against its rivals.** A fitted default is worse
than an unresolved one, because it retargets the parameter digest, every snapshot
literal and every dependent expectation (the C41 precedent cost exactly that for
one field) and then reads as settled.

The four-lens fan-out converged on the same answer from four directions. The
product lens put the picture first and killed the debugger *as the point*; the
architecture lens put the instrument first because it moves no field and is the
only capability that lowers the cost per default; the QA lens required the
experiment to *discriminate* rather than agree; the rights/devops lens confirmed
that a build-only tool adds zero rights, packaging and release surface. The
synthesis is that the two are one milestone, in this order: **the instrument,
then the one default it decides.**

**Order: 17 (records, cost window, count-gate contract) → 18 (the inspection
surface) → 19 (discriminating the display path) → 20 (route, witness, release
posture).**

| Phase | Why here, not elsewhere |
|---|---|
| 17 | The cost window must be recomputed before anything is priced, not after — v0.3.0's `reprice` verdict fired at every phase boundary because its frozen figures were stale from the start. It also closes four carried record items for one id, so the milestone starts on a clean ledger |
| 18 | Phase 19's load-bearing dependency. It moves no profile field, no digest, no snapshot literal and no golden, which is why it can be built before the question it answers |
| 19 | The milestone's point. It is the only phase that can turn a tier-4 default from "compared against an expectation the project derived" into "separated from its rivals by an experiment a stand-in can fail" |
| 20 | Closes the loop: an operator can reach the instrument, the hardware question has a named packet, and the release decision is recomputed |

Rejected alternatives, with the reason: **the picture alone, without the
instrument** is the product lens's own first answer and the QA lens's named
reversal condition — it is taken as a *probe* inside Phase 19 (`19-02`) and not
as the milestone, because a one-field flip that happens to light a frame does not
say which of the four rules it lit it under. **Variant profiles (VAR-01..04)**
need owner-supplied system images plus a per-image rights review, and PAL has no
evidence at all. **Content formats with persistence (FMT-02/FMT-03)** is larger
than one milestone and still rights-gated per format. **Bank modes (FMT-01
narrowed)** is rights-safe but mints nothing falsifiable and is nearly pure gate
content — a phase, not a milestone. **Scheduled fuzzing (TOOL-05)** needs a named
failure class that no gate step catches; none is named. **A pure record-debt
milestone** retires debt but mints nothing; it becomes Phase 17. **Shipping the
shim so RetroArch can load it (a D-49 reversal)** is a packaging decision that
moves the eleven-file prefix, adds a rights row for core-info strings and exposes
the trademark in a binary; it is not taken here.

---

## 2. The phase shape (frozen)

Each phase's success criteria are written into `.planning/ROADMAP.md`; the
mechanism that proves each one, and whether it already exists, is in
`.planning/research/v0.4.0-scoping-4-verification.md`.

| # | Phase | Exit in one line | Tier ceiling |
|---|---|---|---|
| 17 | Records, Cost Window and the Count-Gate Contract | The hosted window recomputed by arithmetic before anything is priced (including the seven unrecorded release-records rows), the eight wrap refusal tokens documented, one id guarding the wrap-to-shim join, the source-archive letter corrected, and the count-gate contract recorded with a mechanism | none — adds no claim |
| 18 | The Inspection Surface | A build-only tool — never installed, outside the eleven-file prefix — giving disassembly, breakpoints and a cycle-stamped access trace over the existing side-effect-free seams, with its rights row, a liveness guard whose stand-in fails, and a disassembly round trip against `ivs_asm` under a non-vacuity guard | none — adds no claim |
| 19 | Discriminating the Display Path | Four authored guests whose observations separate the four rival rules diagonally, each with a guard a stand-in can fail, then a bounded throwaway owner-local sweep, then either the decided default with both settings pinned and every dependent expectation retargeted in one commit, or the recorded "unchanged, rivals named" exit | none — the field stays `unknown-default-chosen` |
| 20 | Route, Witness and Release Posture | One operator page for the instrument and the owner-local route, the D10-VID-08 handoff packet naming the exact trace lines a hardware probe must contradict, and a release-decision line recomputed | none — the packet is a handoff row |

**Depends on:** `17 → 18 → 19 → 20`. Phase 19 depends on `18-02` specifically;
if that plan is late or fails its liveness guard, **Phase 19 is deferred whole to
v0.4.1** and Phase 20 lands with the packet marked "unproduced by instrument".

**Droppable: Phase 18's disassembly limb (`18-03`), pre-authorized.** The trace
and breakpoints survive; a reader falls back to the committed listings, TOOL-02
exits with the reduced surface named, and VID-07 and EV-08 are unaffected. It is
droppable because it is the only limb whose absence costs no falsifiability: a
disassembler is a convenience over listings the repository already holds and the
gate already checks. Dropping the trace limb is **not** authorized — that would
leave Phase 19 with a fitted default. Phase 19's own degradation is its second
honest exit ("default unchanged, observation appended, rivals named"), which is a
result and not a failure.

---

## 3. Decisions frozen before any plan (D-52 to D-56)

Full rows with reversal evidence are in `.planning/PROJECT.md`; the short form:

- **D-52 — the instrument wraps existing seams; the core does not grow a sink.**
  The build-only tool reads through the side-effect-free peek path and the
  existing per-unit access trace, whose own contract already states that machine
  state, log and samples do not depend on it. No new core function, no new sink,
  no file I/O in the core, no new installed file: `pkg-install-files.ids` stays
  11. This is what keeps Phase 18 off the retarget table entirely.
- **D-53 — trace and disassembly output is an operator-local rights-bearing
  artifact.** One rights row (`inspection-output`, `owner-local-only`, nothing
  taken), refused inside the work tree except under `refs/` with the token
  `inspect-output-tree`, `ivst` refused by name in `binary-ext.txt` and
  `.gitignore`, and never a golden, fixture, CI artifact or claim artifact. **No
  trace or disassembly is ever committed, whatever its content.** The naming limb
  widens by one exact page (`docs/inspection.md`) and by zero tokens.
- **D-54 — a display-path default is decided by discrimination, never by fit.**
  A default may change only when an authored guest's observation moves under that
  field and under no other, at the default of the remaining three. A flip that
  merely makes a frame appear is a probe result, not a decision. If no rule
  discriminates, the milestone exits "default unchanged, observation appended,
  rivals named", and that is a success.
- **D-55 — cost is priced locally and monitored hosted.** The hosted last-five
  `cpu_s` window spans 221 cpu-s, 60 percent of its own median, so a phase's 1 to
  3 cpu-s addition is at most 1.4 percent of the span: the hosted median cannot
  price a phase. A phase is priced from an alternated local measurement (times
  1.675) against the recomputed allowance, and the hosted row is recorded to
  monitor, not to decide. `r` stays retired.
- **D-56 — the count-gate contract is a planner obligation with a mechanism.**
  Every count gate names the file it counts; every baseline names the plan's own
  start commit sha and is measured there; no verify block decides an outcome by a
  pipeline tail. Recorded as `PLAN-LESSONS` lesson 32 with a `mutant` row over an
  existing selftest as its mechanism, because v0.3.0's nine recorded plan defects
  were all this one class.

---

## 4. Verification strategy (frozen)

The full matrix is `.planning/research/v0.4.0-scoping-4-verification.md`. The
rule in one sentence: **every capability is an id inside an existing suite; every
new equivalence gets a control whose stand-in fails it; a check enters CI only if
it catches a recurring class.**

- **Zero new gate steps.** 38 stays 38. Zero new faults (238), zero new goldens
  (29). About 20 ids, 9 controls, 4 registry rows, 1 rights row, 1 handoff row.
- **The instrument is judged by reconciliation, not by a golden.** A trace id
  passes only if its records are non-empty, cycle-monotonic, end at the report's
  own `step.*` tick, and its `digest.*` lines equal the same scenario run
  untraced. A stand-in returning zero records, one repeating tick, or a poking
  build each fails for its own stated reason.
- **The disassembly round trip is judged against `ivs_asm`, not against itself.**
  Disassemble, re-assemble, byte-equality against the input, with at least one
  two-word form and one immediate present so the case cannot pass on a corpus of
  single-word instructions.
- **The discrimination is diagonal.** Four guests, four observables, the four
  fields swept as 16 cells; G1's observation moves if and only if the window
  length flips, G2's iff gating flips, G3's iff the read/write rule flips, G4's
  iff the latch rule flips. A guest that moves under two fields is a design
  defect and is re-derived.
- **A decided default is pinned twice** — one id per setting, each building the
  alternate parameter row explicitly — and the retarget chain (profile field,
  profile-fields inventory, parameter digest, **every** snapshot literal, any
  moved golden retargeted in place, the append-only C-row note, the prose) is
  enumerated before the commit and must equal the commit's file list.
- **Not in CI, and why:** the owner-local sweep (its input is owner-local content
  under D11; it is `skip-ok` in both inventories), any real frontend, anything
  needing `refs/`, and the blast-radius enumeration probe. These are throwaway
  probes, not recurring failure classes.
- **Handoffs, never criteria:** D10 and D11 stay `not-performed`; D10-VID-08
  becomes a ninth row naming the exact trace lines a hardware probe must
  contradict. No phase exit criterion may name a handoff.

---

## 5. Cost (frozen)

The arithmetic is `.planning/research/v0.4.0-scoping-2-gate-cost.md`, recomputed
from `.planning/ci-runs.tsv`. Last-five medians at `cc4f991`: `cpu_s` **367**,
`build_s` **34**, `gate_s` **115**. The seven unrecorded release-records rows were appended at the milestone
opening, and the opening commit's own two runs in the records commit that follows;
they are slow-runner rows (`build_s` 47 to 51), so Phase 17's verdict is `reprice`
by construction.

- **Allowance: 21 hosted cpu-s.** This milestone prices **4.72**: 17 → 0.10,
  18 → 2.96, 19 → 1.64, 20 → 0.02. Hard ceiling 5.6.
- **Units, measured:** in-process id 0.065, process-launch id 0.10, control 0.31,
  in-step selftest 0.05, registry row 0.005, fault 0.44 hosted cpu-s.
- **The checkpoint rule:** at each phase boundary and at the close, recompute the
  last-five medians and the token line, charge the phase's priced addition
  against 21, and record `discharged-within-allowance`, `reprice` or
  `lever-required`. **`steps=38/38` is an invariant**: any commit where it is not
  stops the milestone. A phase overrunning its line by more than 25 percent is
  repriced before its next plan.
- **What would blow the allowance:** the discrimination cases registered as
  *faults* (1.76 cpu-s and it moves `faults=238`); a golden per trace or listing
  (forbidden); a per-id rebuild (one cold build is about 34 s, 1.6 times the whole
  price); a new gate step; a separate stand-in for all rival pairs (3.72 in one
  phase).

---

## 6. Risks and their counters

| Risk | Counter |
|---|---|
| The instrument is late or its liveness guard fails | Phase 19 is deferred whole to v0.4.1, never re-planned into a reasoned fix; Phase 20 still lands with the packet marked "unproduced by instrument" |
| A one-field flip lights the frame, so the instrument looks unnecessary | That is `19-02`'s throwaway probe and it is expected; a flip is a probe result, not a discrimination. D-54 is what keeps the distinction |
| The four rivals do not discriminate at this instrument | Phase 19's second exit: "default unchanged, observation appended, rivals named". A success, recorded, and the D10-VID-08 packet becomes the resolver |
| A decided default retargets more than the plan enumerated | The enumeration is mechanical and precedes the commit; a mismatch is a stop, not a re-bless. The C41 precedent measured one field at 3 ids and 4 literals |
| The trace ids cost more than priced | D-55: price locally before the phase closes; the trace is capped by the same tick-cap arithmetic the scenarios use |
| The tool becomes installable or ships | `pkg-install-files.ids` stays 11 with a selftest that the binary is absent, `release-assets` refuses a member, and the target has no install rule |
| A trace of owner content leaks into the tree | D-53: refused by token inside the tree, `ivst` refused by name, and the claim-artifact predicate refuses it |

---

## 7. What this milestone does not do

- It does not ship a host artifact, a window, an audio device, a prebuilt binary,
  a runner installer, a cartridge or any content byte.
- It does not claim that the picture is correct, that a real EXEC displays, or
  that any rule is hardware truth. The observation mints no claim and the field
  stays `unknown-default-chosen`.
- It does not parse `.cfg`, implement bank switching, or describe any third-party
  format. FMT-01 and FMT-04 stay `Deferred` with their conditions.
- It does not add a second profile, PAL, ECS, IntelliVoice, persistence or a
  content database.
- It does not add a gate step, a fault, a golden, or a second installed consumer.
- It does not burn down the discrepancy count: a count going down is not
  evidence.

---

## 8. The frozen guardrails

`.planning/ops/GUARDRAILS-v0.4.md` holds the retarget table (W1 to W10), the
definition of done, the discuss and verify boilerplate, the per-phase tier
ceilings and the clean-state reconciliation. It supersedes `GUARDRAILS-v0.3.md`,
which stays untouched except for one supersession line; its V1 to V10 rows carry
forward and are cited by number.

**The owner runs one command after `/clear`:** `/gsd-autonomous`.
