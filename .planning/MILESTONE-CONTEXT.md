# Milestone v0.2.0 — context and upstream scope

**Status:** frozen upstream artifact, 2026-11-24. Written before any v0.2.0
phase plan exists. Produced by the milestone ritual
(`.planning/ops/MILESTONE-RITUAL.md`): a four-lens milestone-choice fan-out
followed by a five-point upstream scoping fan-out, synthesized and auto-selected
under the owner's delegation (`.planning/AUTONOMY.md` sections 1 and 2).

**Name:** Registered Evidence, Reachable Runs, Replayable Input
**GSD label:** v0.2.0. **Software version:** 0.2.0 if released (a minor bump is
correct under D-30: a new format and new runner options; no API break).
**Phases:** 9 to 12. **Branch slugs:** `gsd/phase-9-…` through `gsd/phase-12-…`.
**Plans:** about 24 (18 without Phase 12), against 21 in Phase 8 and 16 average
in v0.1.0.

---

## 1. Why this milestone, and why in this order

v0.1.0 shipped a complete, well-evidenced machine core. Three things are now the
binding constraints on the project's own stated intent ("high-quality, accurate
and **useful**"):

1. **Nothing a person can obtain is runnable.** `docs/packaging.md` states it
   plainly: neither `ivs-run`, `ivs_pack` nor `ivs_asm` is installed or shipped,
   so nothing downloadable runs. [OBS]
2. **Evidence cannot travel.** Every observation lives inside this repository,
   produced by one author lineage; there is no artifact a second party can hand
   back that the gate can re-run. 27 of 28 claims are `provisional`. [OBS]
3. **The gate has a cost rule with no measured floor.** The cost rule fired on a
   stale reading (corrected: median 7.826, not 8.34), and the last non-owner
   lever is still queued. Every future phase that adds a check inherits that
   uncertainty. [OBS]

The four-lens fan-out converged on **replay (TOOL-01)** as the highest-value
candidate by three of four lenses (rights-clean by construction, cheapest, reuses
the scenario grammar, the applied-event chain and the round-trip hook), with
**reachability (TOOL-04, cheap half)** as the largest *capability* gap and
**FMT-01** as the only route to external validity but the largest rights risk.
The scoping pass then reordered reachability ahead of replay, because replay's
consumers all reach the runner through the source tree today and because the
reachability phase carries the milestone's only owner-latency item.

**Order: 9 (registered evidence and gate cost) → 10 (reachable runs) → 11 (input
record and replay) → 12 (bank modes and external import, conditional).**

| Phase | Why here, not elsewhere |
|---|---|
| 9 | The gate may not grow before it. Its first wave is cost-only so the hosted row is attributable to the cost changes alone. |
| 10 | Cheapest phase, and the only one with an owner-latency item (look/listen). Front-load the one thing a machine cannot do. |
| 11 | Replay is more valuable once a person can obtain the runner; landing it first would ship a feature into an unobtainable artifact. |
| 12 | Last and droppable, so a cost spike is absorbed by dropping it rather than discovered at the close. Rights-gated per mode. |

Rejected alternatives, with the reason: a three-phase milestone without Phase 12
(pre-authorized fallback, taken if the rights review fails or the cost row is
tight); Phase 9 folded into Phase 11 as a wave (loses the milestone-level
allowance and produces a hollow verification record in the phase that exists to
prevent them); reachability merged with replay (two failure classes in one
VERIFICATION record); a windowed frontend (first runtime dependency, permanent
CI blind spot, and a palette that risks becoming a de-facto STIC-colour claim).

---

## 2. Phase shape and exit criteria

### Phase 9 — Registered Evidence and Gate Cost

**Goal:** bank the cost levers, take a hosted row that settles the level, then
convert the 0.47 `r` of headroom into a priced allowance and close the largest
evidence hole in the project.

**Wave A1 (cost only, one push, no evidence content):**
1. Recompute the window by arithmetic from `.planning/ci-runs.tsv` and open the
   cost entry with the corrected reading. (Already done at the milestone
   opening — the entry reads `phaseA1-start`.)
2. **Lever (3):** fold `demo-lamp-tone-swap` and `demo-lamp-tone-shift` into
   static repeat variants of one mutant — column 5 `run` → `release-only` in
   `data/expected/runner-scenarios.ids`, the skip vocabulary and a
   derivation-equality judgement in `tests/runner-process-tap.sh` (the scenario's
   `assert` block must equal, in order, the block between the
   `asserts-begin`/`asserts-end` markers of
   `docs/video/demo-lamp-tone-1-derivation.md`; its `content`/`profile`/`ivss`/`id`
   lines equal the pass scenario's; its `input` lines are the pass inputs
   transformed as documented), the same mode switch on the scenario pair, and
   **one** suite self-control (`runner-process.judge.variant-static`) in the same
   commit. Measured ceiling: 0.12 local cpu-s.
3. **Trims (a), (c), (d)** with their preserving conditions, and **(b) declined
   by measurement**. See `.planning/CI-EFFICIENCY.md`, "next-milestone opening".
4. Measure, both presets, three alternated runs; state the before/after.
5. Push. The orchestrator records `R1` and writes exactly one line:
   `phaseA1: discharged-below-budget` (≤7.8), `discharged-within-line` (7.8–8.3)
   or `lever-required` (>8.3, possible only at the second row after `R1`).

**Wave A2 (content-free registration, after the row):**
6. **EV-02 W1 registration:** 24 lines in `data/expected/important-gates.ids`
   (306→330) covering 17 requirements (FND-01..07, CPU-01..05, MEM-01..05), each
   naming a mechanism that already exists in an inventory that already runs.
   Extend the accepted-requirement set in
   `tools/check/steps/important-gates.sh` and append one toy line per new REQ to
   **all ten** whole-file toy registries, in the same commit (GUARDRAILS R2).
   No new fault, no new test id. Predict the count delta in the plan.
7. **EV-03 gate hardening:** `SRC-` whole-token match, zero-document failures,
   per-step pass lines, manifest output anchoring, memmap generator controls —
   each with one control that fails for its intended reason.
8. **EV-04 machine check:** a requirement marked Complete with no registry line
   is refused, and a toy registry that drifts from the good tree is refused; both
   shown able to fail. This is the mechanism that makes EV-02 unrepeatable.
9. **Cheap consistency checks before the long suites:** derive the preflight
   partition (a step is preflight iff its script references neither the gate
   results directory nor `build/`), buffer each step's tokens and `-- FAIL` lines,
   concatenate in sorted name order so output stays byte-identical. Local
   time-to-first-signal drops from ~44 s to ~21–24 s; hosted cost is unchanged by
   construction.
10. **Bookkeeping and settings:** correct the stale `D-31` `create_tag` text and
    `RELEASE-OPS.md` (says `create_tag: true`, "ten files"); the stale
    composite-version comment; the two bench golden commands into
    `docs/runner.md`; disposition the seven draft `VALIDATION.md` files by citing
    `08-ACCEPTED-DEBT.md` §1 in a live record (never a rewrite of an archive);
    apply the branch and tag rulesets or record "not applied" with a reason.

**Exit:** `sh`, `dash` and the container gate green; the lever and trims commits
are ancestors of the recorded row; the W1 commit's `steps/controls/faults/
tests-*` match its predicted delta exactly; the allowance and the unit-cost table
are recorded; a phase-boundary checkpoint line exists.

### Phase 10 — Reachable Runs and Presentation Policy

**Goal:** a person obtains and runs the emulator from the source archive with
documented commands, the presentation policy is frozen, and the look/listen
handoff is recorded rather than assumed.

1. **`docs/running.md`** — the three commands (`cmake --preset release`,
   `cmake --build --preset release`, then `ivs-run` on
   `demo-lamp-tone-pass`), the exact expected stdout tail, where the `.ppm` and
   `.wav` land, and the carried-over sentence that nothing downloadable is
   runnable. No helper script (it would need either zero logic or a new id).
2. **`docs/presentation.md`** — the six-clause policy: the core publishes palette
   indices only; exactly one operator preview palette exists, labelled "not STIC
   colour data", never installed, never in a golden, digest, assert or claim; the
   project makes no colour claim for any profile; a host chooses palette, scaling
   and aspect; "index frame" is the published plane and "picture/render/looks
   like" are never evidence; a real colour table would be a profile field with a
   source tier. Enforced by one deny-list check that colour symbols are not
   defined in `core/src`.
3. **`docs/packaging.md` and README** — one paragraph pointing at
   `docs/running.md`; the "nothing downloadable is runnable" sentence kept
   identical so release text and README cannot disagree.
4. **The release rehearsal proves the documented route:** extend
   `tools/release-rehearse.sh` to build from the **source archive** in a clean
   directory and run `demo-lamp-tone-pass` to its golden. This is release-time,
   not CI — the gate already pays for a cold build, and a second one in CI would
   be duplicate cost.
5. **`data/expected/handoffs.ids`** seeded with H7-1, H7-2, H8-3, D10, D11, all
   `evidence: none`; H7-1/H7-2 get a handoff packet (exact commands, the
   deterministic identities, the report digest lines) and the status
   `handed-off` or `not-performed`. Two guards: the `claims` step refuses a claim
   citing a handoff artifact or a `build/`, `.wav`, `.ppm` or bench `.tsv` path;
   the `text` step's claim-word list gains the look/listen words.

**Exit:** the documented commands run end to end from the source archive to the
golden, proved by the rehearsal; the policy document is frozen and cited by the
deny-list check; the handoff ledger exists with every id dispositioned; zero new
gate steps; `claims=` unchanged.

**Not in this phase:** installing `bin/ivs-run` or any image (see §3, D14 — it is
structurally blocked, not merely expensive), a windowed frontend, any new
dependency, a second example cartridge.

### Phase 11 — Input Record and Replay

**Goal:** a rights-clean input-only log whose replay reproduces the recorded
run's digest chain and applied-event chain, with the vacuity guards that
v0.1.0's round-trip hook lacked.

1. **`docs/formats/ivsl-v1.md` first** (ancestry-proved): version, scenario id,
   profile id, a `content project|user` line, and `at_tick <n> c0=<HEX>
   c1=<HEX>` records. **No field can hold a content digest, image name, length,
   frame byte, audio byte or path** — absent from the grammar, and an unknown
   directive is refused. Identity binding, when an operator wants it, reuses the
   existing `expect-sha256` directive in their own untracked scenario.
2. `--record-input` (operator side, exclusive create) and `--replay-input`.
3. **Ids inside existing suites — zero new gate steps.** `runner.record.
   replay-equals-straight` (in process), `runner.record.format-roundtrip`
   (write→parse→write byte-identical), the refusal family (unknown directive,
   non-monotonic tick, tick beyond bound, event cap, undefined controller bit,
   wrong version, trailing garbage), and **exactly one** process case for the
   command line (argument parsing, `--content-dir`, exit codes).
4. **Must-fail:** `IVS_FAULT_RECORD_DROPS_LAST_EVENT` and
   `IVS_FAULT_REPLAY_APPLIES_LATE` (one tick) on digest domain 18, registered
   under TOOL-01.
5. **Liveness guards:** the equality case must also assert ≥1 event,
   `input.consumed == input.events`, the `at_tick` list equals the recorded one,
   and all nine domains observed. **Judge self-controls:** `replay-differs` (one
   digest nibble changed → rejected) and `replay-inert` (a fake pair with zero
   applied events → rejected *by the guard*, not the equality).
6. **Rights check:** `runner.record.no-content-digest` greps the produced log for
   a ≥32-hex run and requires zero hits.

**Exit:** replay reproduces the straight run's digests and chains with the guards
proved live; every malformed log refused with a typed token and no state change;
`ivsl` is text, not in `binary-ext.txt`, not gitignored; tier ceiling
`deterministic-repeatability`, provisional.

### Phase 12 — Bank Modes and External Import (conditional)

**Goal:** bank-mode model in profile data with a typed refusal for every unmapped
mode, and — only if the rights review clears it per mode — `.bin`/`.cfg` import
outside the core.

**Entry gate:** a rights-review decision line written **before** any third-party
file is read. `entered`, or `degraded to FMT-01 only`.

1. **Rights review first.** A per-fact table: `fact | source id | rights status |
   what was taken (fact / paraphrase / short quote / nothing)`. A description
   written in the project's own words (`docs/formats/cfg-import-v1.md`), with no
   code, table or constant list copied from any emulator. Sources enter
   `data/sources.json` with `rights: unknown` before any fact is written down.
2. **Bank-mode model (always deliverable):** profile fields for the address-space
   split, the selected bank and per-bank spans; **explicit typed refusal for
   every mode the profile does not name** — never a silent default.
3. **Fixtures are generated, never committed as binaries:** tracked `.asm` and
   `.cfgsrc` sources plus a manifest, built into `build/<preset>/images/` by a
   target beside `ivs_images`. A tracked file with an extension in
   `binary-ext.txt` can never ship (the `files` and `release-assets` rules
   together), so nothing changes in `binary-ext.txt` or `.gitignore`.
4. **Per-mode evidence:** a witness pair (the same address reads A before the
   select write and B after), a **no-switch negative case** (a program that never
   writes the select register still reads A), one named fault per mode, and a
   positive case beside every refusal so "refuse everything" cannot pass.
5. **Importer outside the core:** refusals are closed, append-only **tool tokens**
   (`cfg-syntax`, `cfg-unknown-directive`, `cfg-missing-key`, `cfg-range`,
   `cfg-duplicate-key`, `cfg-bank-mode-unmapped`, `cfg-bank-mode-unsupported`,
   `cfg-bin-size`, `cfg-bin-overlap`, `cfg-bin-outside-window`,
   `cfg-bin-not-found`, `cfg-bin-not-regular`, `import-out-inside-tree`,
   `out-exists`, `cfg-digest-mismatch`) pinned in
   `data/expected/cfg-refusals.ids`. No new `IVS_E_*` reason, no core change, no
   `api-effects` retarget.

**Degradation — Phase 12 ships FMT-01 only if any of these holds:** a written
fact has no review row; the mapping is only expressible as a table copied from a
GPL source; a mode can only be exercised by a third-party file; the importer
would have to live inside the core; or no suite may be added because the cost row
has not cleared. On degradation, record `FMT-01: partial (bank-mode model only),
importer deferred: <condition>` — never `Complete`, and do not write the
description.

**Exit:** every accepted mode has a fixture, a witness pair, a no-switch case and
a fault; every unaccepted mode is refused by name; no tracked binary; no claim
above `provisional`; no "runs games" wording.

---

## 3. Decisions taken by this milestone (delegated, with reversal evidence)

Recorded in `PROJECT.md`'s Decision Register as `D14`–`D16`.

| Id | Decision | Reversal evidence |
|---|---|---|
| **D14** | The runner is **not** installed in 0.2.x; the person route is the documented build from the source archive. Installing it is structurally blocked, not merely expensive: an installed runner has nothing to run (every cartridge is a build product of `ivs_pack`/`ivs_asm`, which are never shipped), and any release member with a binary extension is refused "whatever the list says". | A named consumer who wants a downloadable binary **and** an images story that survives the release-asset check — concretely, a runner-synthesised symbolic content name (like `zero-grom`) for the boot stub plus a project-authored cartridge the runner can make. That is a real design; it is the correct trigger to reopen, and it is not this milestone's. |
| **D15** | The replay log is an **input artifact**: ASCII, input events and ticks only, no content-digest field, a `content project\|user` line, not in `binary-ext.txt`, not gitignored. Under D11 it is sufficient: an input log holds nothing the images produced, so it is not derived material. | A consumer that needs the log to bind content identity inside the format (then reuse `expect-sha256`, not a new field), or a defect found only at the process boundary (then promote two or three cases from in-process). |
| **D16** | The `.bin`/`.cfg` importer lives **outside the core**; its refusals are tool tokens; its output is operator-side and refused inside the work tree; it is rights-gated per mode. | A consumer that needs import inside the installed core (then D-cart is revisited first), or a primary, permissively described source that makes the description provably clean (then the importer can be un-gated per mode). |

Also decided here, and recorded in the phase records rather than as register rows:
the milestone adds **zero new gate steps** (38 stays 38); every new capability
lands as ids in an existing suite.

---

## 4. Verification strategy (shift left, zero human UAT)

**Automated, in CI (existing suites only):** everything in Phases 9, 11 and 12,
and the policy and rehearsal checks in Phase 10. Every new check ships three
things in one commit: a must-fail mechanism registered under its requirement, a
liveness guard, and a committed scratch stand-in showing the *guard* fail.

**Throwaway, not in CI:** the cost measurement (`/usr/bin/time -p`, three
alternated runs), the ancestry proofs, the log-size probe that sizes the event
cap, and the rights-review source search. Record the result, not the script.

**Release-time, not CI:** the cold "person build from the source archive, then
run to the golden" check. The gate already pays for a cold build and
`release-assets` already checks archive membership; a second cold build in CI is
duplicate cost.

**Handed off, and only these:** H7-1/H7-2 (look at a frame, listen to a tone),
D10 (physical hardware), D11 (owner-local real EXEC/GROM). Each is a row in
`data/expected/handoffs.ids` with `evidence: none`; none is citable by a claim;
none is a phase exit criterion a tool cannot meet; a later milestone cites the row
id instead of re-asking, and opens a new id only if the artifact itself changed.

**Tier ceilings, frozen before the work:** Phase 9 adds no claim. Phase 10 reaches
at most `installed-host-behavior` for the documented route, and **no** tier for
the handoff. Phase 11 reaches `deterministic-repeatability` (same core, so never
an independent comparison). Phase 12 reaches `source-expectation` +
`original-diagnostic` + `deterministic-repeatability`, and **cannot** reach
`public-vector` or `independent-implementation-comparison` because by design no
third-party file is read. Nothing reaches `measured-hardware` while D10 is open.

---

## 5. Risks and their counters

| Risk | Counter |
|---|---|
| Cost work grows into a gate redesign | Wave A1 may touch only the seven named files; a hard scope sentence in the plan; guardrail 2 of `GUARDRAILS-v0.2.md` |
| W1 silently adds content | The W1 commit's exit criterion is the predicted count delta, and the ten toy registries are checked by `git diff --numstat` (one addition each, zero deletions) before the commit |
| A vacuous replay oracle (the 08-16 failure class) | The perturbed-tick mutant, the non-empty applied-event guard, `input.consumed == input.events`, and `replay-inert`, which proves the *guard* rejects a no-op |
| Phase 10 drifts into a packaging project | D14's structural-blocker argument, the reversal trigger, and a `files_modified` list the plan checker can test |
| Phase 12's rights review becomes a rubber stamp | One row per fact with a source id and a "what was taken" cell; no fact may cite an emulator's code; the ordering proof; per-mode degradation, so a thin review produces a *refusal*, not a breach |
| A handoff or reachability result drifts into a claim | The frozen tier ceilings, `handoffs.ids` with `evidence: none`, the `claims` step refusing handoff artifacts, and the extended claim-word list |
| A structural phase rediscovers the seven gate retargets | `GUARDRAILS-v0.2.md` §1, cited by row number in every plan's `files_modified` |
| The milestone closes with a stale verification (v0.1.0's seven-of-eight) | Every phase's verify block re-runs the gate on the final tree; the close-state reconciliation is a checklist, not a memory |

---

## 6. Frozen upstream artifacts

These must exist before any phase plan is written. This file is item 1.

1. **This file** — the phase shape, exit criteria, decisions, verification
   strategy and risks.
2. **`.planning/ops/GUARDRAILS-v0.2.md`** — the 11-row retarget table, the
   definition of done, the discuss and verify boilerplate, the close-state
   checklist.
3. **`.planning/ops/MILESTONE-RITUAL.md`** and **`.planning/ops/INDEX.md`** — the
   standing ritual and the durable-knowledge index.
4. **`.planning/CI-EFFICIENCY.md`, "next-milestone opening" entry** — the
   corrected reading, the window arithmetic, the 21 hosted cpu-s allowance, the
   lever and trim mechanics with preserving conditions, and the preflight
   partition rule.
5. **`.planning/REQUIREMENTS.md`** for v0.2.0 with the ids below minted and
   traceability filled by the roadmapper.
6. **`data/expected/handoffs.ids`** — created in Phase 10, seeded with H7-1,
   H7-2, H8-3, D10, D11.
7. **`docs/formats/ivsl-v1.md`** — Phase 11's oracle, committed before its code.
8. **`docs/presentation.md`** — Phase 10's policy, frozen before its checks.
9. **`data/expected/cfg-refusals.ids`** and the rights-review template — Phase
   12, before any third-party file is read.
10. **The tier-ceiling table** (§4) and **the unit-cost table** (measured in
    Phase 9) — so a plan cannot write "supported" and cannot price by id count.

---

## 7. Requirements

Minted for this milestone; full text and traceability in
`.planning/REQUIREMENTS.md`.

**EV — evidence durability and gate cost**
- EV-01 The cost lever and trims land in cost-only commits before any new gate
  content, with a hosted row recorded after them that settles the level.
- EV-02 The 17 requirements whose named faults already run are registered with
  cannot-pass-vacuously mechanisms, with the accepted set and all ten toy
  registries extended in the same commit and no new fault or test id.
- EV-03 Each gate-hardening item has a control that fails for its intended reason.
- EV-04 A machine check refuses a requirement marked Complete with no registry
  line, and a toy registry that drifts from the good tree; both shown able to fail.
- EV-05 A handoff ledger records every human-only item with `evidence: none`, and
  no claim may cite a handoff artifact.

**PKG — packaging and reachability**
- PKG-05 A person can obtain and run the emulator from the released source
  archive using documented commands only, proved by the release rehearsal.

**TOOL — tooling**
- TOOL-01 Input record and replay at machine-time boundaries.
- TOOL-04 (narrowed) A frozen presentation and palette policy keeps colour out of
  the core and out of every claim, enforced by a check.

**FMT — content formats**
- FMT-01 (narrowed) Bank-mode model in profile data with a typed refusal for
  every unmapped mode and one generated original fixture per mode.
- FMT-04 `.bin`/`.cfg` import outside the core, rights-gated per mode, degrading
  to FMT-01 only.

**Explicitly out of scope for this milestone:** a windowed frontend, any new
runtime dependency, installing the runner or any image (D14), the rest of the
TOOL-04 surface (audio device, physical controller mapping), FMT-02/FMT-03,
VAR-01..04, PER-01/PER-02, TOOL-02, TOOL-03, TOOL-05, TOOL-06, and any claim
above `provisional`.

---

## 8. Clean-state close

Per `.planning/ops/MILESTONE-RITUAL.md` step 7 and `GUARDRAILS-v0.2.md` §6: green
`main` with the push run recorded, empty `git status --porcelain` on `main`, one
worktree, no stashes, no unmerged phase branches, PRs and issues triaged to zero,
both rulesets applied with ids recorded or explicitly recorded as not applied,
records reconciled, a stale-text sweep, and one release-decision line
(`PUBLISHED v0.2.0 (prerelease)` or `READY, NOT PUBLISHED: blocked by P<n>`) with
the checks recomputed.

---

*Frozen 2026-11-24 by the milestone ritual. Amendments are appended with their
own commit and a date, never edited in place.*
