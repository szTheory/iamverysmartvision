# Project Retrospective

*A living document updated after each milestone. Lessons feed forward into future planning.*

## Milestone: v0.1.0 — Machine Core Foundations

**Shipped:** 2026-11-24
**Phases:** 8 | **Plans:** 130 | **Tasks:** 151
**Release:** `v0.1.0` pre-release on the artifact commit `3fe5373`

### What Was Built

A deterministic, headless Intellivision (2609-family NTSC) machine core and the
evidence apparatus around it.

- **Core**: CP-1610 CPU (all 74 opcode family rows, interrupts, bus request, SDBD), a profile-driven memory map with typed content loading and typed refusals, IVSC v1 cartridges, both hand controllers as first-class input, an AY-3-8914-class PSG with raw audio and a cycle-stamped write log, and a STIC producing a raw 159 x 96 palette-index framebuffer with MOBs, collisions, delay and border extension.
- **Exact-build snapshots**: IVSN v1 with an 88-row hidden-state oracle, 18 named hard-point cuts, and corruption/fuzz families that refuse bad input without touching the instance.
- **Host surfaces**: a headless runner with a scripted scenario language, an installed consumer proving the package independently of the source tree, and `ivs-bench` with four digest-asserted workloads.
- **Evidence apparatus**: a claim ledger bound to profile + evidence tier + artifact, a discrepancy log with pinned defaults, fixture manifests with recipe digests, a blind-second-read discipline, and one local gate command (`tools/gate.sh`, 38 steps) mirrored by hosted CI.
- **Release machinery**: a strict tag glob, a draft-first release workflow, a version policy that refuses tag/header/package skew, and a content-based release asset checker.

### What Worked

- **Oracle before code.** Deriving the expected values (frames, vectors, assertion blocks) and committing them before any implementation caught every divergence on the first run and made "the program is wrong" distinguishable from "the oracle is wrong".
- **Blind second reads.** Two independent reads of the CP-1610 family table and the hidden-state checklist, each frozen by a named commit, turned agreement into a countable fact (`second-read=12/12`, `state: closed` with 0 corrections).
- **Negative controls as first-class artifacts.** 380 controls, 235 named faults and a self-testing control runner meant that "this check can fail" was itself checked, which is what kept the gate from rotting into ceremony.
- **Installed-consumer proof.** Building the consumer against the installed archive (and, at release, against the shipped archive) caught packaging and include-path drift that source-tree tests cannot see.
- **One table, one serializer.** Driving the writer, reader, validator and restore commit from a single field table made snapshot completeness a machine-checked property (every instance byte covered exactly once) rather than a review exercise.
- **Cost measured as a normalised ratio.** `r = cpu_s / build_s` with a last-five median kept the CI conversation about code rather than about runner speed, and every checkpoint ended with a recorded decision instead of a feeling.

### What Was Inefficient

- **Plan prose vs the frozen specification.** Twice the plan sentence and the oracle disagreed (restore check order, witness selection) and the specification won after the plan had been written against the other reading. Lesson 19 exists because this cost rework.
- **Heavy suites priced by id count.** Wave B was planned at 12 to 16 hosted cpu-s and measured 21.4, because one suite of 61 ids costs 3.3 local cpu-s on the sanitized preset. Lesson 22.
- **Checks that could pass vacuously.** A round-trip hook that did nothing, a benchmark stand-in that never handed on the cadence, and three inert witnesses were all found late. Lesson 20 turned "give every equivalence check a liveness guard" into policy.
- **Container gate stalls.** Three runs were lost to docker or emulation stalls, once by starting a second run that destroyed the first log. Lesson 18.
- **Stale verification at the close.** Seven of eight phase verification records read `stale` at milestone completion because later phases legitimately edited shared files (`README.md`, `data/claims.json`, `tools/gate.sh`). The override was bounded and stated, but a milestone that re-verifies at the end would have avoided the bookkeeping.

### Patterns Established

- **Appended-note corrections to frozen oracles** (`data/expected/state-sweep-notes.ids`, the as-built sections of `docs/formats/ivsn-v1.md`), never edits of frozen lines.
- **One shared rule list per rule family** (`ivs_snap_inv_rules[]`), so a predicate cannot exist in two compositions that drift apart.
- **Derived constants over guessed ones**: the snapshot clock slack is `psg.max_unit_ticks`, re-proved by a bounds test, not a hand-picked number.
- **`[claim:CL-NNNN]` / `[nc]` tagging** as the price of writing a claim-shaped word in public text, with a tripwire and a control.
- **A tier identifier is not a claim** (the one exemption in the text scan).
- **Cost checkpoint → recorded decision** (`no-lever` / `lever-required` / `lever-applied`) at every wave boundary.

### Key Lessons

1. Put the order, the reasons and the witnesses in the oracle document first; when plan prose and the specification differ, the specification wins and the plan sentence is recorded as a deviation.
2. Every equivalence, round-trip or witness check needs a guard that the compared thing actually happened, and a scratch stand-in showing it fail.
3. Price a new heavy suite by measurement in the plan that adds it, and read the median of the last five hosted rows, not one row.
4. A recorded condition the host did not meet is kept as taken and said plainly (the benchmark baseline at load 5.96).
5. Never start a second container gate to read the first; copy the log first.
6. Reference figures are not as-built figures: derive timing asserts and mutant patches from the words and cycles actually emitted.
7. A skipped id is a counted id, declared in both inventories with a control in each direction.
8. A frozen oracle takes corrections as appended notes with their own commit.

### Cost Observations

- **Model mix:** one model family wrote the derivations, the programs, the checklist and the core; executors ran one plan at a time with an orchestrator spot-check between plans. No per-model cost split was recorded.
- **Sessions:** many; the project ran as a long autonomous sequence with explicit pause and resume handoffs (`.planning/HANDOFF.json`, `.continue-here.md`), both of which proved worth writing.
- **Notable:** the hosted gate's normalised cost stayed flat across the milestone (`r` median 7.78 at the Phase 7 close, 7.80 at the Phase 8 close, 7.86 after the stage-2 rows) while absolute work grew (235 faults, 4,958 expected tests). That is the metric doing its job: the median moved less than the spread of a single row (7.20 to 8.39).

---

## Milestone: v0.2.0 — Registered Evidence, Reachable Runs, Replayable Input

**Shipped:** 2026-11-25
**Phases:** 4 resolved (9, 10, 11 delivered; 12 dropped at its entry gate) | **Plans:** 20
**Release:** `v0.2.0` pre-release on the artifact commit `8f017e7`, published 2026-11-26 after
the housekeeping pass cleared P5.

### What Was Built

- **The gate's cost level settled by measurement.** Lever (3) and trims (a), (c), (d) landed in
  cost-only commits, a hosted row followed them and their ancestry was proved, and trim (b) was
  declined with its measurement. The preflight partition takes local time-to-first-signal from
  79 s to 31 s with byte-identical normalised output.
- **The invisible evidence gap closed.** 17 requirements whose named faults already ran were
  registered (306 to 337 rows, 90 to 110 gates) against mechanisms that already existed, with the
  must-be-covered set derived from the requirement records and a two-directional exclusion ledger.
- **A person can obtain and run it.** `docs/running.md` names three measured commands;
  `tools/release-rehearse.sh --source-route` takes the released archive to 124 of 124 golden lines
  in a clean extraction.
- **Colour kept out of the core and out of every claim.** `docs/presentation.md` freezes six
  clauses; the refusal that enforces the load-bearing one already existed.
- **What a human owes became a ledger.** Five `evidence: none` rows, and a claim can no longer cite
  a handoff artifact.
- **A run is recordable and replayable.** IVSL v1 is input-only by construction; replay reproduces
  the nine digest lines and the applied-event chain with the vacuity closed three ways.
- **Phase 12 was dropped with its reason written before any third-party file was read.**

### What Worked

- **Oracle before code, again.** `docs/formats/ivsl-v1.md` came before the tracer, and its grammar
  made "no content digest" a property of the bytes rather than a promise.
- **Priced levers.** Measuring before changing kept the gate at 38 steps and spent 4.5 of the 21
  hosted cpu-s the milestone opened with.
- **Derived inventories.** The must-be-covered set and the preflight partition are computed, so a
  deleted line fails instead of silently shrinking coverage.
- **The droppable phase.** Phase 12 absorbed a rights refusal and a cost reading at its own entry
  gate instead of at the close.

### What Was Inefficient

- **Three executors stopped at a 1800 s dispatch limit** and their plans were closed by the
  orchestrator, including the Phase 9 cost measurement that was never taken.
- **Four review rounds found 3 blockers and 13 warnings in work already `passed`**, two in a phase's
  central deliverable.
- **The cost metric was read as authoritative in one record and untrustworthy in another**, and the
  decisive arithmetic was printed only at the close.
- **A frozen-oracle rule designed for archives was applied to a live specification**, leaving a false
  sentence in the document an operator reads until the close added a pointer.

### Patterns Established

- A **termination rule** for the CI-row regress (one records PR; its merge push is the next
  milestone's opening row).
- A **question-shaped rights ledger** as the answer to "what may be named", distinct from the
  byte-set fixture manifest.
- **`mutant` registry rows over existing selftests** instead of new faults when a must-fail mechanism
  already has a stand-in.
- **A build-only host surface** proved by a headless fake frontend, so the window never becomes the
  evidence.

### Key Lessons

See `PLAN-LESSONS.md` 28 to 31: a review is load-bearing; a ratio whose denominator moves is not a
signal; a self-feeding process needs a termination rule; a rule with no mechanism is quoted, not
obeyed.

### Cost Observations

Hosted: 4.5 of 21 cpu-s spent. `cpu_s` at the close (359–360) equals the figure that settled Phase 9
(339–360) while `build_s` fell from 42–49 to 33–34, so `r` rose and the work did not; `r` is retired
as a decision input.

---

## Cross-Milestone Trends

### Process Evolution

| Milestone | Phases | Plans | Gate steps | Controls | Faults | Tests expected | Median `r` |
|-----------|--------|-------|-----------|----------|--------|----------------|------------|
| v0.1.0 | 8 | 130 | 38 | 380 | 235 | 4,958 | 7.86 at the close; 8.34 across the close-out window (stop line fired) |
| v0.2.0 | 4 (3 delivered, 1 dropped) | 20 | 38 | 395 | 238 | 4,990 | `r` retired as a decision input; `cpu_s` 361 median at the close, `build_s` 34 |

### Recurring Failure Classes

| Class | First seen | Guard now in place |
|-------|-----------|--------------------|
| Vacuous or inert check | Phase 8 | Liveness guards + scratch stand-ins (lesson 20) |
| Plan prose vs frozen oracle | Phase 8 | Oracle-first ordering + deviation record (lesson 19) |
| Hosted-only toolchain divergence | Phase 2 | Container gate + `r` metric (lessons 11, 12) |
| Cost estimated by id count | Phase 8 | Measure heavy suites at plan time (lesson 22) |
| Interrupted run leaving partial state | Phase 8 | Visible not-run line, re-validate partials (lesson 24) |

### Carried Into the Next Milestone

From the milestone audit's tech-debt register and the integration pass:

0. **The gate cost rule (binding, do this first) — reading corrected 2026-11-24.** The close-out entry quoted a last-five median `r` of **8.34**, above the 8.3 stop line. **That figure is stale**: four hosted rows recorded after the entry moved the window with no code change. Recomputed by arithmetic from the committed `.planning/ci-runs.tsv`, the last five values are **7.327, 7.521, 7.826, 8.233, 8.338**, so the median is **7.826** — 0.47 under the stop line and 0.026 over the 7.8 budget. The stop line did not fire. [OBS]
   **The rule still stands, because it is recorded policy and not an evidence claim.** No new test id, control, fault or gate step may enter the gate until lever (3) — fold `demo-lamp-tone-swap` and `demo-lamp-tone-shift` into repeat variants of one mutant — and trims (a), (c) and (d) have landed, with a hosted row recorded after them. Trim (b) is **declined by measurement** (0.09 local cpu-s, 0.03 % of the gate, against the only sanitized execution of the 65-cut loop). Dropping a check, an oracle, a golden or a pinned-default test is not a lever. [OBS/REC]
   **The allowance is computed, not assumed:** at `build_s` 46 the 0.47 `r` of headroom is worth about **21 hosted cpu-s** of new gate work for the whole milestone. That is the ceiling, not a target. Entry: the "next-milestone opening" entry in `.planning/CI-EFFICIENCY.md`, which also fixes the window arithmetic: after one new row the maximum possible median is 8.233, so a single row cannot re-fire the rule.
1. **Evidence durability (W1)**: 17 requirements (FND-01..07, CPU-01..05, MEM-01..05) have named faults that run but no lines in `data/expected/important-gates.ids`, so they lack registered "cannot pass vacuously" evidence. Best done as a focused quick task with the Phase 1 gate-hardening items.
2. **Gate hardening (Phase 1, Phase 3)**: the `SRC-` token match, zero-document failures, per-step pass lines, manifest output anchoring, the memmap generator controls.
3. **Bookkeeping**: seven `VALIDATION.md` files still `status: draft`; one stale composite-version comment; two bench golden commands missing from `docs/runner.md`.
4. **Settings**: the branch and tag rulesets recommended in `.planning/AUTONOMY.md` section 1 are still not applied. [OBS]
   Also stale as of 2026-11-24: `PROJECT.md` D-31 says the `git.create_tag` key "reads `true` … the change is still to be made", but `.planning/config.json` reads `false` and the close record attributes the change to `6f800cc`. `.planning/ops/RELEASE-OPS.md` likewise says `create_tag: true` and "ten files" where the install list is eleven. Bookkeeping for the next milestone's Phase 9, not a new decision.
5. **Owner hard stops**: D10 (47 hardware items) and D11 (owner-local real EXEC/GROM runs) remain unperformed; every claim stays provisional until they are.

## v0.3.0 — Owner-Supplied Content and a Driveable Host (closed 2026-11-26)

**What worked, measured.** Four phases, 17 plans, nine requirements, one claim,
**zero new gate steps, faults or goldens** — every capability landed as an id
inside a suite that already existed, which is the discipline the milestone was
built to test. Every phase's verify chain ran locally first (`sh`, `dash`, then
the Ubuntu container) and **every hosted run in the milestone was green on its
first attempt**; the container leg earned its keep twice in Phase 15, finding a
non-PIC archive and four ELF crt stubs that do not reproduce on Darwin.

**What cost the most, and what to do about it.** Planning, not execution. Phase
13's plan took three checker passes (4 → 2 → 0 blockers); Phase 14's took four
(6 → 2 → 1 → 0); Phase 16's took three. Almost every blocker was the same class:
**an `<automated>` gate that a correct implementation cannot satisfy** — a count
read from a file the tool does not write, an exit status decided by a trailing
`tail`, a baseline measured at the wrong commit, a literal in the wrong token
order. Each phase's executor then re-found the same class in the plan's own
prose (nine recorded plan defects in Phase 16 alone). The lesson is concrete and
belongs in the planner's contract, not in a retrospective: **every count gate
must name the file it counts, and every baseline must be measured at the plan's
own start commit.**

**What the milestone deliberately did not do.** It did not claim that any real
content boots, plays, looks or sounds right. Eight handoff rows carry
`evidence: none`; D10 and D11 stay `not-performed`; no claim rises above
`provisional`. That restraint held under pressure — the first real-content run
produced a genuine, exciting result (the EXEC populating BACKTAB), and it was
still recorded as an observation that mints no claim.

**Carried into the next milestone.** The wrap→shim join is measured but
unguarded by a case; the eight wrap tokens are undocumented under `docs/`; and
the `reprice` verdict fired at every phase boundary because the frozen 361/34/118
figures were stale from the start — the next milestone should recompute the
window before it prices anything, not after.
