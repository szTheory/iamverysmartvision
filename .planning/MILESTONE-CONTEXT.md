# Milestone v0.3.0 — context and upstream scope

**Status:** frozen upstream artifact, 2026-11-26. **This file always describes
the CURRENT milestone.** The next milestone's ritual replaces it wholesale (this
copy is archived as `.planning/milestones/<version>-CONTEXT.md` at that
milestone's close), so `gsd-new-milestone` must never read it as a proposal for
the milestone after this one. Written before any v0.3.0 phase plan exists.
Produced by the milestone ritual (`.planning/ops/MILESTONE-RITUAL.md`): a
four-lens milestone-choice fan-out followed by a five-point upstream scoping
fan-out, synthesized and auto-selected under the owner's delegation
(`.planning/AUTONOMY.md` sections 1 and 2). The five scoping outputs are frozen
alongside this file in `.planning/research/v0.3.0-scoping-*.md`.

**Name:** Owner-Supplied Content and a Driveable Host
**GSD label:** v0.3.0. **Software version:** 0.3.0 if released (a minor bump
under D-30: a new tool, a new host target and a new operator route; no API break).
**Phases:** 13 to 16. **Branch slugs:** `gsd/phase-13-…` through
`gsd/phase-16-…`. **Plans:** about 18 to 22.

---

## 1. Why this milestone, and why in this order

v0.2.0 made the core reachable by source build, replayable and durable. It left
one question the project has never been able to answer: **can a person put real
content into this machine and drive it?** Today the answer is no on three
independent counts, and every one of them is a recorded fact:

1. **The core reads only the project's own cartridge container.** `IVSC v1` is
   produced by `ivs_pack` from tracked listings; there is no route from a flat
   cartridge image to something the machine can load, and `.bin`/`.cfg` import
   was deferred at Phase 12's entry gate. [OBS]
2. **There is no host.** `runner/` is scenario-driven and headless. The
   framebuffer is a 159x96 palette-index plane delivered one slot per frame
   event, the audio is a per-channel level stream every 16 master ticks, and no
   artifact in the tree presents either. TOOL-04's frontend limb is deferred. [OBS]
3. **The content itself is owner-held.** The EXEC and GROM images are the
   owner's files (D11), never committed, and `refs/` holds research PDFs only.
   No amount of project work can supply them. [OBS]

The owner's stated want, 2026-11-26, verbatim: *"can u play games with it in
retroarch yet? i want to try it out if possible... with real games like a few
different ones kind of sanity/smoke test it"*. That is the strongest product
signal the project has received, and all four fan-out lenses converged on the
same answer: **content path first, host surface second, and the owner's own
files last, as a handoff row rather than a criterion.** The four lenses differed
on ordering and on how much gate work goes first; the synthesis below takes the
rights lens's ledger and the evidence lens's enforced ceiling into Phase 13, the
architecture lens's "the content path is the binding constraint" into Phase 14,
and the product and rights lenses' build-only host into Phase 15.

**Order: 13 (rights ledger and enforced ceiling) → 14 (flat-image wrap and the
cartridge role) → 15 (build-only host and the fake-frontend harness) → 16
(owner route, one claim, handoff rows, release posture).**

| Phase | Why here, not elsewhere |
|---|---|
| 13 | Every later phase either names a third-party-content item or mints a claim. Until the ledger and D-32's mechanical check exist, a refused item can be named and a claim can rise with the gate still green. It is the entry ticket. |
| 14 | The content path is the binding constraint: a host built before it has nothing to load, and RetroArch would refuse every real image. Rights-clean by construction — the base address is stated by the operator, nothing parses a third-party format. |
| 15 | A host is only worth building once there is something to load. It is also the only phase that creates a new build target, so it is the least predictable in cost and the droppable one. |
| 16 | Closes the loop: the owner route end to end, exactly one claim, the handoff rows, and the release posture. |

Rejected alternatives, with the reason: **the debugger and inspection path
(TOOL-02)** is the cheapest and most falsifiable capability on the mid horizon,
but its only consumer is the project itself, and the roadmap's own rule is that
a row with no named consumer is a wish; it moves to the next mid slot, after the
host gives it a consumer. **Variant profiles (VAR-01..04)** need owner-supplied
system images for the first three and have no evidence at all for PAL. **Content
formats with persistence (FMT-02/FMT-03)** is larger than this milestone and
still rights-gated. **A pure gate-hardening milestone** would retire record debt
but mint nothing falsifiable, and the largest untested assumption — that a
cartridge image can be driven by a real frontend — would still never have
executed.

---

## 1a. What the first real-content run changed (2026-11-26, after this file was first frozen)

The owner supplied their own EXEC, GROM and a cartridge collection on 2026-11-26 and
authorised a local diagnostic run. The D11 route now executes, and the first run **fails
deterministically**, which re-cuts this milestone's order. [OBS]

- **The EXEC boots and runs.** 8192-byte EXEC and 2048-byte GROM load through the public
  API; the reset entry at `$1000` executes, runs a delay loop and reaches a VBlank wait at
  `$1067`. [OBS]
- **The cartridge path is sound.** With the project boot stub and a real 8K cartridge at
  `$5000`, the guest runs 30,000 instructions without leaving the code ranges. [OBS]
- **The EXEC path dies at the first interrupt.** Accepting an interrupt left the CPU's
  interrupt enable set (`cpu.interrupt_accept_clears_intff = 0`, conflict **C41**, an
  `unknown-default-chosen` default). The CPU therefore re-entered the handler on every
  instruction boundary, the EXEC's stack walked past `$035F` into undriven space, and the
  program counter ended at `$FFFF` within about 1.3 frames: 96,066 gate-blocked reads, 600
  black frames. [OBS]
- **Clearing the enable on acceptance fixes the storm.** The handler then runs its own
  prologue linearly (`PSHR R0`, `GSWD R0`, …), blocked reads fall to 185, the EXEC
  populates BACKTAB with real card descriptors, and the run reaches 4,411 instructions
  before the next blocker. The display-enable strobe still never latches, so frames stay
  black; the next blocker is the STIC register-window / strobe rule, also a tier-4 default.
  [OBS]
- **The fix is a phase of work, not an edit.** Flipping the field moves the profile
  parameter digest, every snapshot literal and the boot-stub-2 interrupt tests: measured,
  the gate goes red in `state.*` and `run.stub2.jd-as-j` until those are retargeted in the
  same commit. [OBS]

**Consequence for the order.** A host built before this is a black screen with a window
around it, so the content route and the conformance work become one phase and move ahead of
the host: **13 rights ledger → 14 cartridge route and real-content conformance → 15 host →
16 release posture.** The observation is not evidence and mints no claim: the field stays
`unknown-default-chosen` until the recorded hardware probe exists, and no digest of any
owner image is recorded anywhere.

---

## 2. The phase shape (frozen)

Each phase's success criteria are written into `.planning/ROADMAP.md`; the
mechanism that proves each one and whether it already exists is in
`.planning/research/v0.3.0-scoping-4-verification.md`.

| # | Phase | Exit in one line | Tier ceiling |
|---|---|---|---|
| 13 | Rights Ledger and Enforced Claim Ceiling | A question-shaped rights ledger with a check that refuses an unsourced `cleared` row and a refused spelling in code or operator pages; D-32's `established`-claim check; the ten D-09-06 mechanisms registered as `mutant` rows | none — adds no claim |
| 14 | Cartridge Route and Real-Content Conformance | A build-only tool wraps a flat image at an operator-stated base into IVSC v1 with seven typed refusals each beside its boundary positive; the owner-local route gains a cartridge role; and the first real-content blocker is fixed with its default resolved, pinned by a project-authored test, and every dependent expectation retargeted | none — adds no claim |
| 15 | Build-Only Host and the Fake-Frontend Harness | A shared object outside `core/` and outside the install list, driven by a harness that `dlopen`s it and reproduces the runner's nine digest lines on a project cartridge, guarded three ways | `deterministic-repeatability`, provisional (one claim) |
| 16 | Owner Route, Claim and Release Posture | One documented owner route from the source archive, handoff rows, and a release-decision line recomputed | none — the claim is minted in Phase 15 |

**Depends on:** `13 → 14 → 15 → 16`. Phase 15 no longer stands beside 14: the
first real-content run shows that a host built before the EXEC path is fixed shows
a black screen, so 15 depends on 14. Phase 15 stays droppable — the fallback is a
content-and-conformance v0.3.0 — and the degradation is unchanged.

**Droppable: Phase 15**, pre-authorized, degrading to a content-and-conformance
v0.3.0 (ledger, ceiling, wrap, cartridge role, the first blocker fixed, owner
route). It is droppable because it alone creates a new build target — D12's named
reversal trigger — so its cost and CI shape are the least predictable. Dropping
it leaves a new content capability; dropping 14 would not. The degradation is
written in the close record: the goal is amended to "Owner-Supplied Content", no
host route appears in `docs/owner-images.md`, and the libretro-header and
core-info rights rows keep their verdicts and are cited, not re-argued.

---

## 3. Decisions frozen before any plan (D-46 to D-50)

Full rows with reversal evidence are in `.planning/PROJECT.md`; the short form:

- **D-46 — the rights ledger is a second, question-shaped ledger.**
  `data/rights/<slug>.json`, one file per item, `disposition` in
  {`cleared`, `refused`, `owner-local-only`}, a `SRC-` id or null, a "what was
  taken" cell and the spellings a refused item must never appear as. Enforced as
  ids inside `files`/`manifests`; zero new steps. The naming limb is
  token+scope with a pinned baseline, never a global grep, because `cfg` and
  `ivsc` already exist in the tree as *refusal surfaces*.
- **D-47 — the wrap tool states bytes, never a format.** `tools/flatwrap
  <flat> <base> <out> --word-order hi-first|lo-first`: a byte length, a word
  order and an address the operator states. Nothing is auto-detected, nothing is
  described from another implementation, and `.cfg` is refused by name. The
  output is derived material of rights-bearing content: never committed, never a
  golden, never a fixture, never created by CI, never a claim artifact.
- **D-48 — the host is build-only, with a project-authored ABI subset.** No
  libretro header is vendored: the shim needs about six `retro_*` entry points,
  so `host/` states only the subset it implements and says *libretro-shaped*,
  never compatible. Vendoring an exact header requires its licence text read
  first and recorded as `licence_evidence`.
- **D-49 — the package stays static and eleven files.** The build-only shared
  target is the only place `IVS_API` is defined; installed headers keep it empty;
  `binary-ext.txt` gains `so`, `dylib`, `dll`, `bundle` so a tracked shared
  library is refused by name. The release-decision line reads: *the release ships
  the static package and the source archive; no shared object, no host shim, no
  content.*
- **D-50 — D-32 is enforced mechanically.** An `established` claim must be
  `source-expectation` resting on a `tier1-primary-doc` dependency; no other
  tier may stand `established`. `CL-0002` is the only claim this permits, and it
  is checked rather than assumed.
- **D-51 — an owner-local observation may resolve an `unknown-default-chosen`
  profile field, and the retarget is part of the change.** The first real-content
  run showed `cpu.interrupt_accept_clears_intff = 0` producing an interrupt storm
  that ends at `$FFFF`; the value becomes `1`, informed by the owner's own images
  through the D11 route. The field stays tagged `unknown-default-chosen` (the
  observation is not a hardware probe and mints no claim), and the commit that
  changes it carries the profile parameter digest, every snapshot literal, the
  boot-stub-2 interrupt tests and the discrepancy note (C41). An observation that
  cannot be turned into a project-authored test is recorded, not acted on.

---

## 4. Verification strategy (frozen)

The full matrix is `.planning/research/v0.3.0-scoping-4-verification.md`. The
rule in one sentence: **every capability is an id inside an existing suite;
every new equivalence gets a control whose stand-in fails it; a check enters CI
only if it catches a recurring class.**

- **Zero new gate steps.** 38 stays 38. Zero new faults, zero new goldens.
  Twenty ids, eleven controls, four in-step selftests and ten registry rows.
- **The window is never the evidence.** The host is proved by a fake frontend
  that `dlopen`s it, drives a project cartridge and must reproduce the runner's
  nine digest lines; a stub that prints the golden with no frame must be refused
  `host-vacuous`, and a mutant with the input poll dropped must be refused
  `host-digest-diff`.
- **Every refusal sits beside its boundary positive**: odd↔even length,
  `$0400`↔`$03FF`, 65,537↔65,536 words, base `$3FFF`↔`$4000`, crossing↔ending at
  `$10000`, in-tree↔outside output, existing↔new.
- **The owner route's PASS path is exercised with project-made files**:
  zero-filled 8192/2048-byte images are shape-legal, so `owner-run.sh`'s pass
  path runs end to end; a one-byte EXEC mutation proves the id can fail, and the
  `exec:` label stays `-` so nothing reads as a real-EXEC result.
- **Not in CI, and why:** any real frontend, anything needing `refs/` or an owner
  image, `--source-route` (release-time), a shim sanitizer lane (no distinct
  class; the core's ASan path covers it), per-id rebuilds, an `nm` dump, a shim
  audio listen. These are throwaway probes, not recurring failure classes.
- **Handoffs, never criteria:** D10 and D11 stay `not-performed`; three new
  `handed-off` rows cover "a real frontend loads the shim", "an owner image's
  picture and sound look right" and "the owner's flat image needs no word-order
  guess". None is a phase exit criterion and no claim cites one.

---

## 5. Cost (frozen)

The arithmetic is `.planning/research/v0.3.0-scoping-2-gate-cost.md`, recomputed
from `.planning/ci-runs.tsv`: last-five medians `cpu_s` **361**, `build_s` **34**,
`gate_s` **118**. `r` is retired as a decision input (v0.2.0 close, audit G-04).

- **Allowance: 21 hosted cpu-s** (v0.2.0 spent about 4.5). This milestone prices
  **about 5.2**: 13 → 2.0, 14 → 1.3, 15 → 1.5, 16 → 0.4.
- **Units, measured:** in-process id 0.065, process-launch id 0.10, control 0.31,
  in-step selftest 0.05, registry row 0.005, fault 0.44 hosted cpu-s.
- **The checkpoint rule now that `r` is retired:** at each phase boundary and at
  the close, recompute the last-five medians and the token line, charge the
  phase's priced addition against 21, and record `discharged-within-allowance`,
  `reprice` (the median moved more than the priced addition with an unchanged
  token line: a runner event, no lever invented) or `lever-required`.
  **`steps=38/38` is an invariant**: any commit where it is not stops the
  milestone.
- **The concurrency lever is taken**, once, in Phase 16's first commit, cost-only:
  `c-tests` and `c-faults` overlapped (164 s → about 115 s wall, `cpu_s`
  unchanged), with the summary token line byte-identical and each
  `step.out.<name>` compared. It buys wall time and Actions minutes, not ratio:
  no checkpoint, no allowance credit.
- **What would blow the allowance:** registering the ten D-09-06 mechanisms as
  *faults* instead of `mutant` rows (4.4 cpu-s, 82 % of the milestone's spend on
  the step already 29 % of the gate), and a harness that rebuilds per id or
  replays the six-image consumer per id.

---

## 6. Risks and their counters

| Risk | Counter |
|---|---|
| The owner has no EXEC, GROM or cartridge dump | The milestone is still deliverable and still verifiable: every capability is proved with project-made bytes. The owner's files change a handoff result, nothing else. The EXEC/GROM requirement is stated in `docs/owner-images.md`, not discovered at the close |
| The flat-image wrap needs a word order the project guessed wrong | The operator states it (`hi-first` default, `lo-first` accepted); nothing is auto-detected; a rights row records that the project makes no statement about any real dump's layout |
| The host target drags in a licence the project cannot accept | D-48: no header is vendored; the ABI subset is project-authored. If an exact header is ever needed, its licence text is read and recorded first |
| The host becomes a window and therefore a CI blind spot | The harness, not the window, is the evidence; the shim is build-only and never a release member |
| Phase 15's cost or CI shape is unpredictable | It is droppable, pre-authorized, with a written degradation |
| A claim rises above `provisional` | D-50's mechanical check lands in Phase 13, before any claim exists in this milestone |
| The rights ledger becomes a second manifest schema that drifts | It is question-shaped, one file per item, and the manifest stays the byte-set answer; the reversal condition is written |

---

## 7. What this milestone does not do

- It does not ship a host artifact, a runner installer, a cartridge or any
  content byte.
- It does not parse `.cfg`, implement bank switching, or describe any
  third-party format. FMT-01 and FMT-04 stay `Deferred` with their conditions.
- It does not claim that any real EXEC, GROM or commercial cartridge boots,
  plays, looks or sounds right. That is the handoff.
- It does not take the debugger, variant profiles, content persistence or
  scheduled fuzzing. They stay on the rolling roadmap with their gates named.
- It does not add a gate step, a fault, a golden or a second installed consumer.

---

## 8. The frozen guardrails

`.planning/ops/GUARDRAILS-v0.3.md` holds the retarget table (V1 to V11), the
definition of done, the discuss and verify boilerplate, the per-phase tier
ceilings and the clean-state reconciliation. It supersedes `GUARDRAILS-v0.2.md`,
which stays untouched except for one supersession line.

**The owner runs one command after `/clear`:** `/gsd-autonomous`.
