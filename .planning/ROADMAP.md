# Roadmap: iamverysmartvision

## Overview

The project is building a deterministic, headless Intellivision machine core in
which every compatibility claim cites an exactly named profile and an evidence
tier. v0.1.0 delivered the machine and the evidence apparatus. v0.2.0 made the
core **reachable** (a person builds and runs it from the source archive),
**replayable** (a run reproduces from an input log) and **durable** (the gate's
evidence cannot rot, and its cost is measured). v0.3.0 turned to the question the
project had never been able to answer: **can a person put real content in and
drive it?** — a rights-clean content path, a build-only host surface, and the
owner's own files as a handoff row rather than a promise. It shipped that, and it
left the picture black. v0.4.0 builds the instrument that can tell the four rival
rules apart, and then decides one of them by discrimination rather than by fit.

This file is a **rolling roadmap**: a recommendation register, labelled `[REC]`,
re-triaged at every milestone opening per `.planning/ops/MILESTONE-RITUAL.md`.
It is not a promise and not a requirement list — requirements are minted per
milestone in `.planning/REQUIREMENTS.md`. Archived milestone roadmaps live under
`.planning/milestones/<version>-ROADMAP.md` and are append-only.

**Every row names what gates it.** An item with no named gate is a wish.

## Horizons

### Near — v0.4.0 "Inspectable Machine, One Default Decided"

Selected 2026-11-26 by a four-lens fan-out and a five-point upstream scoping
pass (`.planning/MILESTONE-CONTEXT.md`; the five scoping outputs are frozen in
`.planning/research/v0.4.0-scoping-*.md`). Phases continue the numbering at 17.

**Why this one, in one line:** after the CPU-06 fix the owner's own EXEC
populates BACKTAB and the picture stays black, and the four rival rules that
could explain it are unseparated — so a fix made now would be fitted to one
observation rather than discriminated against its rivals.

| # | Phase | Goal | Requirements | Gate |
|---|-------|------|--------------|------|
| 17 | Records, Cost Window and the Count-Gate Contract | The hosted window recomputed by arithmetic before anything is priced (including the seven unrecorded release-records rows), the eight wrap refusal tokens documented, one id guarding the wrap-to-shim join, the source-archive letter corrected, and the count-gate contract recorded with a mechanism | EV-08 | Cost line: no external gate, but nothing may be priced before the window is recomputed — v0.3.0's `reprice` fired at every boundary because its frozen figures were stale from the start |
| 18 | The Inspection Surface | A build-only tool — never installed, outside the eleven-file prefix — giving disassembly, breakpoints and a cycle-stamped access trace over the existing side-effect-free seams, with its rights row, a liveness guard whose stand-in fails, and a disassembly round trip against `ivs_asm` under a non-vacuity guard | TOOL-02, TOOL-09 | Rights: one operator-local row, output refused inside the tree, `ivst` refused by name (D-53). Cost: it wraps existing seams and moves no profile field, digest, snapshot literal or golden (D-52) |
| 19 | Discriminating the Display Path | Four authored guests whose observations separate the four rival rules diagonally, each with a guard a stand-in can fail, then a bounded throwaway owner-local sweep, then either the decided default with both settings pinned and every dependent expectation retargeted in one commit, or the recorded "unchanged, rivals named" exit | VID-07 | D-54: a default is decided by discrimination, never by fit. D11: the sweep is a throwaway probe and never an exit criterion |
| 20 | Route, Witness and Release Posture | One operator page for the instrument and the owner-local route, the D10-VID-08 handoff packet naming the exact trace lines a hardware probe must contradict, and a release-decision line recomputed | VID-08 | Owner hard stop D10: the packet is a handoff row with `evidence: none`, cited by no claim |

Rationale for the order, in one line each: **17** must come first because a
price computed on a stale window is a number, not a measurement; **18** moves no
field and is therefore the only thing that can be built before the question it
answers; **19** is the milestone's point, and it depends on `18-02`
specifically; **20** closes the loop.

**Droppable:** Phase 18's disassembly limb (`18-03`), pre-authorized. The trace
and breakpoints survive and a reader falls back to the committed listings; a
disassembler is the only limb whose absence costs no falsifiability. Dropping the
trace limb is **not** authorized: that would leave Phase 19 with a fitted
default. Phase 19's own second exit ("default unchanged, observation appended,
rivals named") is a result, not a failure. Both degradations are written in the
close record.

**The v0.3.0 detail is archived** in `.planning/milestones/v0.3.0-ROADMAP.md`
(phases 13 to 16, shipped 2026-11-26, all four complete; Phase 15 delivered
inside its priced line so no degradation clause fired).

### Mid — the next two or three milestones `[REC]` (re-triaged 2026-11-26, at the v0.4.0 opening)

Re-triaged, not appended. TOOL-02 has left this table (it is Phase 18's
requirement now). The rows are ordered by what the project can actually finish.

| Candidate milestone | Delivers | Gate |
|---|---|---|
| **The second blocker, and the picture on a real frontend** (TOOL-04 narrowed + the next tier-4 default) | Whatever the v0.4.0 instrument names as the next black-screen cause, plus a documented build-only frontend route that a person can actually run | The v0.4.0 trace must exist first: this row is priced by what the instrument found, not by a guess. Shipping a prebuilt host stays D-49, a separate decision |
| **Variant profiles** (VAR-01..04) | Alias vs distinct profile decided by image comparison; the first profiles beyond the family-level 2609 NTSC | User-supplied system images plus a rights review for the first three; **PAL has no evidence at all** — it needs a primary data sheet or a measurement (D10) |
| **Content formats with persistence** (FMT-02, FMT-03) | Intellicart/CC3/JLP/LTO Flash shapes and their persistence; a hash-keyed content database | A rights review per format, and a persistence story that reverses OQ8. Larger than one milestone and still rights-gated |
| **Bank modes as profile data** (FMT-01, narrowed) | A typed refusal for every bank mode the profile does not name, with one generated original fixture per accepted mode | A re-derived registry baseline and a cleared cost reading; `.cfg` stays refused by name, so this waits on a mode description the project may write itself |
| **Scheduled fuzzing and extended sanitizers** (TOOL-05) | A recurring robustness class that the one headless job does not catch | A named failure class that no current gate step catches; the out-of-gate libFuzzer target exists, so the trigger is a failing case found on demand |
| **A shipped host** (D-49 reversal) | The libretro-shaped shim installed, a prebuilt binary obtainable, core-info strings | Reverses D-49: it moves the eleven-file prefix, needs a rights row for core-info strings, and puts the trademark in a binary. Not taken without an owner instruction |

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
<summary>✅ v0.3.0 Owner-Supplied Content and a Driveable Host (Phases 13 to 16) — SHIPPED 2026-11-26</summary>

- [x] Phase 13: Rights Ledger and Enforced Claim Ceiling — one question-shaped rights row per content question, an unsourced-`cleared` refusal, a refused-spelling refusal, D-32's `established`-claim check, and the ten D-09-06 mechanisms registered
- [x] Phase 14: Cartridge Route and Real-Content Conformance — a build-only wrap at an operator-stated base with seven typed refusals each beside its positive, a cartridge role in the owner-local route, and the first real-content blocker fixed, pinned and retargeted
- [x] Phase 15: Build-Only Host and the Fake-Frontend Harness — a shared object outside the core and the install list, proved by a harness that reproduces the runner's nine digest lines, guarded three ways (droppable)
- [x] Phase 16: Owner Route, Claim and Release Posture — one documented route from the source archive, handoff rows, shared objects refused by name, and a recomputed release decision

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
| D-52 The inspection surface wraps existing seams; the core gains no sink | Architecture + rights | Delegated decision 2026-11-26 | Phase 18 |
| D-53 Trace and disassembly output is operator-local and rights-bearing | Rights | Delegated decision 2026-11-26 | Phase 18 |
| D-54 A display-path default is decided by discrimination, never by fit | Evidence + architecture | Delegated decision 2026-11-26 | Phase 19 |
| D-55 Cost is priced locally and monitored hosted; `r` stays retired | Architecture + CI | Delegated decision 2026-11-26 | Phase 17 (adopted at the opening) |
| D-56 The count-gate contract is a planner obligation with a mechanism | Architecture | Delegated decision 2026-11-26 | Phase 17 |
| OQ8 Snapshot persistence policy | Architecture | Delegated decision (Phase 8) | Done |
| OQ11 Performance targets | Architecture | Delegated decision (PRF-01 sets none) | Done |

## Phase Details

**Milestone v0.4.0 "Inspectable Machine, One Default Decided".** The phase shape,
exit criteria, decisions (D-52 to D-56), verification strategy, tier ceilings and
risks are frozen in `.planning/MILESTONE-CONTEXT.md`; the retarget table and the
definition of done are `.planning/ops/GUARDRAILS-v0.4.md`; the requirement-to-
mechanism matrix is `.planning/research/v0.4.0-scoping-4-verification.md`. The
frozen plan count is eleven: 3 + 3 + 3 + 2.

Six rules hold across every phase below. **Zero new gate steps, faults or
goldens:** `ls tools/check/steps/*.sh | wc -l` reads 38, `faults=238` and
`goldens=29` stay, and every capability lands as an id inside a suite that
already exists. **The milestone mints no claim:** `claims=29` and
`provisional=28` are invariants, and every phase's tier ceiling is "none - adds
no claim". **The instrument wraps existing seams and the core gains no sink**
(D-52). **`data/expected/pkg-install-files.ids` stays 11.** **A default is
decided by discrimination, never by fit** (D-54). **Every count gate names the
file it counts and every baseline names the plan's own start commit sha** (D-56,
`GUARDRAILS-v0.4` section 2.14).

**Depends on:** 17 -> 18 -> 19 -> 20, serially. Phase 19 depends on `18-02`
specifically: if that plan is late or fails its liveness guard, Phase 19 is
deferred whole to v0.4.1 and Phase 20 lands with the VID-08 packet marked
"unproduced by instrument". **Droppable, pre-authorized:** Phase 18's disassembly
limb (`18-03`) - the trace and the breakpoints survive and a reader falls back to
the seven committed listings named in `data/expected/listings.ids`. Dropping the
trace limb is not authorized: that would leave Phase 19 with a fitted default.
Phase 19's second exit ("default unchanged, observation appended, rivals named")
is a result, not a failure.

**Cost:** the allowance is 21 hosted cpu-s recomputed from
`.planning/ci-runs.tsv`; this milestone prices 4.72 (17 = 0.10, 18 = 2.96,
19 = 1.64, 20 = 0.02) with a hard ceiling of 5.6. A phase is priced from an
alternated local measurement times 1.675 and the hosted row is recorded to
monitor, not to decide (D-55).

### Phase 17: Records, Cost Window and the Count-Gate Contract

**Goal**: The milestone starts on a measured ledger rather than a quoted one: the
hosted cost window is recomputed by arithmetic from `.planning/ci-runs.tsv` before
anything is priced, the eight wrap refusal tokens are documented, one id guards
the wrap-to-shim join, the source-archive letter is corrected, and the count-gate
contract becomes a numbered lesson with a mechanism.
**Depends on**: Nothing (first phase of v0.4.0; continues from the shipped Phase 16)
**Requirements**: EV-08
**Success Criteria** (what must be TRUE):
  1. The seven previously unrecorded release-records rows are present and verified, not re-added: they landed at the milestone opening (the termination rule's deferred row plus the six release-records runs), so `awk -F'\t' 'NR>1' .planning/ci-runs.tsv | wc -l` reads 116 rows over 117 lines **at the plan's own start commit**, and each of the run ids `45835631107`, `45836207855`, `45839153455`, `45839646456`, `45841732895`, `45842795129` and `45843408874` appears in exactly one row. **The plan's first deliverable is the recomputation, not the rows.** The phase record prints the last-five `cpu_s`, `build_s` and `gate_s` medians computed by `awk` over that same named file, at that start commit, **before** its first pricing sentence, then exactly one checkpoint line from the `GUARDRAILS-v0.4` section 2.4 vocabulary (`discharged-within-allowance`, `reprice`, `lever-required`). The expected verdict is `reprice`: the opening window reads 572 / 49 / 164 against the previous 367 / 34 / 115, the seven rows are slow-runner rows (`build_s` 47 to 51), `r` is flat inside 10.6 to 11.7, and the token line under the work did not move. A plan that finds a row missing is a stop, not a re-append.
  2. `diff <(grep '^flatwrap-' data/expected/flat-refusals.ids) <(grep '^flatwrap-' docs/formats/ivsc-v1.md)` is silent, because `docs/formats/ivsc-v1.md` carries a "Wrap tool refusals" section listing exactly the eight tokens of the append-only inventory - `flatwrap-length`, `flatwrap-word-range`, `flatwrap-words`, `flatwrap-base`, `flatwrap-cross`, `flatwrap-output-tree`, `flatwrap-output-exists`, `flatwrap-empty` - and the inventory itself is unchanged.
  3. One committed id, `runner-process.judge.flatwrap-shim` in the existing `tests/runner-process-tap.sh` suite, asserts that the shim's cartridge-slot digest equals the container `tools/flatwrap` wrote in the same case, and a committed stand-in that ignores its input (one flipped input byte) is shown failing that id. `ls tools/check/steps/*.sh | wc -l` still reads 38.
  4. `docs/packaging.md`'s "no host shim ships" sentence is corrected to name the source archive beside the static package - the archive carries the source of the build-only tools and of the build-only host shim and ships no built object - and agrees, sentence for sentence, with the release-decision line of `.planning/ops/RELEASE-OPS.md` section (i).
  5. `.planning/ops/PLAN-LESSONS.md` holds lesson 32 **with its mechanism**: every count gate names the file it counts, every baseline names the plan's own start commit sha and is measured there, and no verify block decides an outcome by a pipeline tail, carried by a `mutant` row in `data/expected/important-gates.ids` over an existing selftest and shown able to fail on a committed stand-in. `faults=238`, `goldens=29`, `claims=29` and `provisional=28` are unchanged.
**Plans**: TBD (3 planned: `17-01` the seven rows and the recomputed window, `17-02` the document set and the `flatwrap-shim` id, `17-03` lesson 32 and its mechanism; one wave, serial, because `17-02` and `17-03` both touch `data/expected/`)
**Tier ceiling**: none - this phase adds no claim.
**Cost**: 0.10 hosted cpu-s against the 21 allowance; a records cost, not a gate cost, because the window is the phase's own first deliverable.

### Phase 18: The Inspection Surface

**Goal**: A build-only, scripted, headless inspection surface exists over the
seams the core already exposes - disassembly of the project's own listings,
breakpoints with an asserted hit and non-hit count, and a cycle-stamped access
trace that reconciles with the runner's own report - and its output is an
operator-local, rights-bearing artifact.
**Depends on**: Phase 17 (the recomputed window and the count-gate contract that every plan of this phase cites)
**Requirements**: TOOL-02, TOOL-09
**Success Criteria** (what must be TRUE):
  1. `docs/inspection.md` is committed before `tools/inspect/` exists, proved by `git merge-base --is-ancestor <doc commit> <first tool commit>`; it states the command surface (`trace`, `break`, `dis`, `step`), the record fields and the liveness guards, and a plan sentence that differs from the frozen document is recorded as a deviation rather than silently followed (`GUARDRAILS-v0.4` W5).
  2. The trace ids live in the existing `tests/runner-process-tap.sh` suite and assert a domain, never a bare exit status: records > 0, ticks non-decreasing, the last record's tick equal to the report's own `step.<k>` end tick, and every `digest.*` line byte-identical to the same scenario run untraced. Three committed stand-ins each fail for its own stated reason - an emitter returning zero records, an emitter repeating one tick, and a "tracing" build that pokes a register.
  3. Breakpoints are asserted as a pair in one id: hit count 1 at a unique PC and 0 at an unreached PC, with a stand-in matcher that always hits shown failing the 0 case. The disassembly limb (`18-03`) round-trips in the existing `tests/asm-tap.sh` suite: disassemble, re-assemble with `ivs_asm`, bytes equal to the input, with words > 0, at least one two-word form and one immediate present, and a constant-mnemonic stand-in failing byte equality. If `18-03` is dropped, `docs/inspection.md`'s disassembly section reads "not built; a reader uses the committed listings named in `data/expected/listings.ids`" and TOOL-02 exits with the reduced surface named.
  4. `data/rights/inspection-output.json` (`disposition: owner-local-only`, `source: null`, nothing taken, tokens `trace-file`, `disassembly-dump`, `inspection-dump`), `data/expected/rights.ids` and the `docs/RIGHTS.md` row land in one commit, and `ivst` joins `tools/check/binary-ext.txt` and `.gitignore` in that same commit. The tool refuses an output path inside the work tree with `inspect-output-tree` beside its `refs/` positive and refuses an existing file, and `git ls-files | grep -E '\.ivst$'` is empty.
  5. The tool is build-only and nothing it touches moves: `data/expected/pkg-install-files.ids` stays 11 with a `pkg-install` selftest that `ivs_inspect` is absent from the prefix, one `release-assets` case refuses it as an archive member, `ls tools/check/steps/*.sh | wc -l` reads 38, and no profile field, parameter digest, snapshot literal or golden moves (`goldens=29`, `faults=238`, `claims=29`, `provisional=28`).
**Plans**: TBD (3 planned: `18-01` freeze `docs/inspection.md` alone in wave 1; `18-02` the tool with the trace, the breakpoints and the command surface in wave 2; `18-03` disassembly and the `ivs_asm` round trip in wave 3, droppable)
**Tier ceiling**: none - the tool is build-only and its output is repeatability information, never a claim.
**Cost**: 2.96 hosted cpu-s against the 21 allowance, the largest line of the milestone, priced from an alternated local measurement (D-55) because it builds one new target under the existing presets and adds no step.

### Phase 19: Discriminating the Display Path

**Goal**: The four rival display-path rules are separated diagonally by authored
guests, so that the default is either decided by discrimination - both settings
pinned and every dependent expectation retargeted in one commit - or recorded
unchanged with the rivals named.
**Depends on**: Phase 18, and `18-02` specifically: without the trace, Phase 19 is deferred whole to v0.4.1
**Requirements**: VID-07
**Success Criteria** (what must be TRUE):
  1. Four authored guests exist, one per rival rule, each with one machine-state observable and a liveness guard that a committed stand-in fails: G1 a WRITE to `$0020` at a cycle inside the 2000-to-2900 gap (frame 0 `displayed`), G2 a WRITE outside both window candidates with the display latch clear (`stic.blocked_writes`), G3 a READ of `$0020` with no write (frame 0 `displayed`), G4 one WRITE never re-strobed (`displayed` for frames 0, 1 and 2). No criterion, id or guard reads a pixel or a human judgement.
  2. The four fields are swept as 16 cells and the sweep is diagonal: G1's observation moves if and only if `stic.reg_window_cpu_cycles` flips, G2's if and only if `stic.access_gating_when_off` flips, G3's if and only if the `$0020` row of `stic.access_effects` flips, and G4's if and only if the display-latch rule flips, each with the other three at their default - and the latch rule is a row in `data/expected/profile-fields.ids` before it is swept. A guest that moves under two fields is re-derived, and a committed `mutant` row over the diagonal check is shown able to fail.
  3. Exactly one of two exits is recorded. **Exit A:** the decided default is pinned by two ids, one per setting, each building the alternate parameter row explicitly, and one commit carries the whole `GUARDRAILS-v0.4` **W1** chain - the profile field with its tag and cite, `data/expected/profile-fields.ids`, the parameter digest, every snapshot literal in `data/expected/ivsn-literals.ids`, any moved golden retargeted in place, the moved `stic` inventory ids, the append-only C-row note and the prose - with the commit's file list equal to the enumeration written into the phase record before the commit. **Exit B:** the record reads "default unchanged, observation appended, rivals named", which is `Complete` for VID-07 on its own stated terms. A mismatch between the enumeration and the commit's file list is a stop, not a re-bless.
  4. In both exits the four rival rows in `data/discrepancies.json` carry the appended observation, the field stays tagged `unknown-default-chosen` (D-51: an observation is not a hardware probe), no claim cites the observation, `claims=29` and `provisional=28` are unchanged, `goldens=29` (a moved golden is retargeted, never added), and no claim, tier or golden records a digest, size, path or name of an operator file.
  5. The owner-local sweep (`19-02`) is contained as a throwaway probe and is nobody's exit criterion: it lives under `refs/` only, is bounded by a wall-clock limit and a run count, its output is refused outside `refs/` on the `tools/flatwrap` precedent, and it is `skip-ok` (exit 13, `missing-fixture`) in both test inventories. Its absence, or a run that ends inconclusive, leaves criteria 1 to 4 standing: no criterion of this phase depends on a human act.
**Plans**: TBD (3 planned: `19-01` one authored guest per rival rule with pairwise-distinct traces; `19-02` the bounded throwaway owner-local sweep; `19-03` the decision commit or the degraded record)
**Tier ceiling**: none - the field stays `unknown-default-chosen` even when a default is decided.
**Cost**: 1.64 hosted cpu-s against the 21 allowance, for four discrimination ids and two pinning ids per decided field inside the existing `stic` suite. Registered as faults the same cases would cost 1.76 and move `faults=238`, which is why they are ids.

### Phase 20: Route, Witness and Release Posture

**Goal**: An operator can reach the instrument, the D10-VID-08 hardware question
is a named packet of the exact trace lines a probe must contradict, and the
release decision is recomputed rather than quoted.
**Depends on**: Phase 19 (its four guests define the trace lines the packet names); if Phase 19 was deferred, the packet is marked "unproduced by instrument" and this phase still lands
**Requirements**: VID-08
**Success Criteria** (what must be TRUE):
  1. `docs/inspection.md` carries an operator section whose command lines run as written against a project-authored scenario (build, `trace`, `break`, `dis`, read the output under `refs/`), and `docs/owner-images.md` names the instrument in the owner-local route. The naming limb widens by this one exact page and by zero rights tokens: `data/expected/rights-pinned.ids` stays at four pairs.
  2. `data/expected/handoffs.ids` gains exactly one row, `D10-VID-08 not-performed evidence: none`, so the claims step prints `handoffs=9`; `docs/handoffs.md` carries the packet's section and `docs/stic/d10-procedure.md` carries the anchor that section cites.
  3. The packet names the exact trace lines a hardware probe must contradict - the record fields and, per guest, the observable line and its two candidate values under the rival rules from Phase 19 - as a list a probe's capture is compared against. No claim cites the row: the claim-artifact predicate refuses a handoff id, and that refusal is shown able to fail on a committed stand-in.
  4. The packet's own status line reads either the four rival candidates from Phase 19's guests or "unproduced by instrument", naming the trace lines as specified in `docs/inspection.md`, and no criterion anywhere in this milestone requires the packet's answer or the D11 sweep to be performed - the packet is a row, not a gate.
  5. The release-decision line is recomputed per `.planning/ops/RELEASE-OPS.md` section (i) and reads: the release ships the static package and the source archive, which carries the source of the build-only tools and of the build-only host shim; no shared object, no built host shim, no built inspection tool, no trace or disassembly output, no content - with `data/expected/pkg-install-files.ids` still 11 and `release-assets` unchanged.
**Plans**: TBD (2 planned: `20-01` the operator page and the route; `20-02` the VID-08 packet, the handoff row and the release line)
**Tier ceiling**: none - the packet is a handoff row.
**Cost**: 0.02 hosted cpu-s against the 21 allowance: one handoff row, one document section and a recomputed release line, with no new id in the gate.

## Coverage (v0.4.0)

| Requirement | Phase |
|-------------|-------|
| EV-08 | 17 |
| TOOL-02, TOOL-09 | 18 (disassembly limb droppable) |
| VID-07 | 19 |
| VID-08 | 20 |

Mapped: 5/5 v0.4.0 requirements, each to exactly one phase. No orphans, no
duplicates. The droppable limb is Phase 18's `18-03`: dropping it touches no
requirement's status, and TOOL-02 exits with the reduced surface named. Dropping
the trace limb is not authorized - it defers Phase 19 whole to v0.4.1, where
VID-07 is recorded `Deferred` with its reversal condition, never silently lost and
never `Complete`. Phase 19's second exit ("default unchanged, observation
appended, rivals named") is `Complete` for VID-07 on its own stated terms.

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

### v0.2.0 — scoped 2026-11-24 (shipped 2026-11-25)

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 9. Registered Evidence and Gate Cost | 9/9 | Complete    | 2026-11-24 |
| 10. Reachable Runs and Presentation Policy | 6/6 | Complete    | 2026-11-25 |
| 11. Input Record and Replay | 5/5 | Complete    | 2026-11-25 |
| 12. Bank Modes and External Import (conditional) | 0/0 | Dropped (three-phase fallback) | 2026-11-25 |

**v0.2.0 is shipped and archived.** Phases 9 to 11 complete, Phase 12 dropped at
its entry gate, release `v0.2.0` published as a pre-release on the artifact
commit `8f017e7`. Its phase details are collapsed above.

### v0.3.0 — scoped 2026-11-26 (shipped 2026-11-26)

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 13. Rights Ledger and Enforced Claim Ceiling | 5/5 | Complete    | 2026-11-25 |
| 14. Cartridge Route and Real-Content Conformance | 4/4 | Complete    | 2026-11-25 |
| 15. Build-Only Host and the Fake-Frontend Harness | 3/3 | Complete    | 2026-11-26 |
| 16. Owner Route, Claim and Release Posture | 5/5 | Complete    | 2026-11-26 |

**v0.3.0 is shipped and archived:** scoped and detailed by this roadmap on 2026-11-26.
All four phases are executed and verified. Phase 15 was droppable under the pre-authorized fallback and
was delivered inside its priced line, so no degradation clause was triggered.
The milestone adds zero new gate steps, faults or goldens, and at most one claim,
`provisional`.



### v0.4.0 - scoped 2026-11-26 (in progress)

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 17. Records, Cost Window and the Count-Gate Contract | 0/3 | Not started | - |
| 18. The Inspection Surface | 0/3 | Not started | - |
| 19. Discriminating the Display Path | 0/3 | Not started | - |
| 20. Route, Witness and Release Posture | 0/2 | Not started | - |

**Current milestone:** v0.4.0 "Inspectable Machine, One Default Decided", scoped by
this roadmap on 2026-11-26. Eleven plans over four phases, none started. The
milestone adds zero new gate steps, faults or goldens, mints no claim (`claims=29`
and `provisional=28` are invariants), and prices 4.72 of the 21 hosted cpu-s
allowance.

**Phase details for v0.3.0 are archived** in `.planning/milestones/v0.3.0-ROADMAP.md`
(the phase shape, goals, success criteria, waves and plan lists), with the
requirements in `.planning/milestones/v0.3.0-REQUIREMENTS.md` and the audit in
`.planning/milestones/v0.3.0-MILESTONE-AUDIT.md`. They are not restated here.
