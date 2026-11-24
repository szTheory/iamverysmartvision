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

## Cross-Milestone Trends

### Process Evolution

| Milestone | Phases | Plans | Gate steps | Controls | Faults | Tests expected | Median `r` |
|-----------|--------|-------|-----------|----------|--------|----------------|------------|
| v0.1.0 | 8 | 130 | 38 | 380 | 235 | 4,958 | 7.86 at the close; 8.34 across the close-out window (stop line fired) |

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

0. **The gate is at its cost stop line (binding, do this first).** The last-five hosted median `r` reached **8.34**, above the 8.3 stop line, in the close-out window. **No new test id, control, fault or gate step may enter the gate until lever (3)** — fold `demo-lamp-tone-swap` and `demo-lamp-tone-shift` into repeat variants of one mutant — **and trims (a) to (d)** of the Phase 8 close entry have landed, with a hosted row recorded after them. Dropping a check, an oracle, a golden or a pinned-default test is not a lever. Entry: "2026-11-24 - milestone close-out" in `.planning/CI-EFFICIENCY.md`.
1. **Evidence durability (W1)**: 17 requirements (FND-01..07, CPU-01..05, MEM-01..05) have named faults that run but no lines in `data/expected/important-gates.ids`, so they lack registered "cannot pass vacuously" evidence. Best done as a focused quick task with the Phase 1 gate-hardening items.
2. **Gate hardening (Phase 1, Phase 3)**: the `SRC-` token match, zero-document failures, per-step pass lines, manifest output anchoring, the memmap generator controls.
3. **Bookkeeping**: seven `VALIDATION.md` files still `status: draft`; one stale composite-version comment; two bench golden commands missing from `docs/runner.md`.
4. **Settings**: the branch and tag rulesets recommended in `.planning/AUTONOMY.md` section 1 are still not applied.
5. **Owner hard stops**: D10 (47 hardware items) and D11 (owner-local real EXEC/GROM runs) remain unperformed; every claim stays provisional until they are.
