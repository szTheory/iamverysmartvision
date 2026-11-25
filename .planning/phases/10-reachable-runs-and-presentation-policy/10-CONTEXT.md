# Phase 10: Reachable Runs and Presentation Policy - Context

**Gathered:** 2026-11-24
**Status:** Ready for planning

<domain>
## Phase Boundary

A person obtains the emulator from the **released source archive** and runs it
with documented commands to a known golden; the presentation and palette policy
is frozen and enforced; and the look/listen handoff is a recorded ledger row
rather than an assumption.

**Not in this phase:** installing `bin/ivs-run` or any image (structurally
blocked — D14, not merely expensive), a windowed frontend, any new runtime
dependency, a second example cartridge, any new gate step (38 stays 38), any
claim above `provisional`, and any tier for the handoff.

**Tier ceiling:** at most `installed-host-behavior` for the documented route; the
handoff carries no tier at all.

</domain>

<decisions>
## Implementation Decisions

### The documented route (PKG-05)

- **`docs/running.md` is the only route document, and it carries three commands
  and the exact expected tail.** `cmake --preset release`, `cmake --build
  --preset release`, then `ivs-run` on `demo-lamp-tone-pass`; the exact stdout
  tail is quoted byte for byte, together with where the `.ppm` and `.wav` land.
  **No helper script** — a helper would need either zero logic (a document with a
  shebang) or a new gate id, and both are worse than three lines a person can
  read. — **Reversibility:** reversible — one markdown file.
- **The proof is the release rehearsal, not a gate step.** `tools/release-rehearse.sh`
  gains a mode that unpacks the **source archive** into a clean directory, builds
  it, and runs `demo-lamp-tone-pass` to its golden; the transcript is recorded at
  release time. The gate already pays for a cold build and `release-assets`
  already checks archive membership, so a second cold build in CI would be
  duplicate cost. — **Reversibility:** reversible — one script plus a recorded
  transcript.
- **The route cannot become an install claim.** The "nothing downloadable is
  runnable" sentence is kept **identical** in `README.md` and `docs/packaging.md`
  (a grep proves exactly one matching line in each) so release text and README
  cannot disagree, and both gain one paragraph pointing at `docs/running.md`.

### The presentation and palette policy (TOOL-04 narrowed)

- **`docs/presentation.md` is frozen before its check exists**, with the six
  clauses: the core publishes palette **indices** only; exactly one operator
  preview palette exists, labelled "not STIC colour data", never installed, never
  in a golden, digest, assert or claim; the project makes **no colour claim** for
  any profile; a host chooses palette, scaling and aspect; "index frame" is the
  published plane and "picture/render/looks like" are never evidence; a real
  colour table would be a profile field with a source tier. — **Reversibility:**
  reversible — a document plus a check.
- **One deny-list check, inside an existing step, no new step.** A colour symbol
  defined under `core/src` is refused by the `core-symbols` deny list, which
  already has the machinery and a control (`core-symbols-palette-defined`). The
  check reuses that step and that control rather than adding a second mechanism
  for one property. — **Reversibility:** reversible — a control file.
- **The stand-in is committed and re-runnable**, per GUARDRAILS §2.3, and it must
  fail for its intended reason only.

### The handoff ledger (EV-05)

- **`data/expected/handoffs.ids` is created here** with H7-1, H7-2, H8-3, D10 and
  D11; **every row is `evidence: none`** and each is dispositioned `handed-off`
  or `not-performed`. A handoff is **never** a pass, never blank, and never a
  phase exit criterion a tool cannot meet. — **Reversibility:** costly — the file
  becomes the project's record of what a human owes, and a later milestone cites
  the row id instead of re-asking; changing a row's meaning after the fact would
  re-open a closed question.
- **Two guards, both shown able to fail.** (i) The `claims` step refuses a claim
  citing a handoff artifact, a `build/` path, a `.wav`, a `.ppm` or a bench
  `.tsv`. (ii) The `text` step's claim-word list gains the look/listen words, so
  "looks like" or "sounds right" in prose needs a claim tag. — **Reversibility:**
  reversible — an ids file plus a control each.
- **H7-1 and H7-2 get a handoff packet** — the exact commands, the deterministic
  identities, and the report digest lines a human would compare — so the owner
  can perform the check without reconstructing it. The packet is a document, not
  a gate step.

### Ordering and evidence

- **The policy documents are committed before the checks that enforce them**
  (oracle-first, GUARDRAILS §2.9), and the ancestry is proved with
  `git merge-base --is-ancestor`.
- **No new gate step.** Every new mechanism lands as an ids line, a control or a
  selftest case inside a suite that already exists. The predicted deltas are
  stated per commit before it lands.
- **`claims=` does not move.** No claim is added, none is raised, and the
  documented route's ceiling is `installed-host-behavior` — which is a
  *ceiling*, not a claim to be minted here.

### Decided autonomously
- [Route] Q: "Document or helper script for the three commands?" → Selected: "Document only; a helper needs zero logic or a new id." — one markdown file, reversible.
- [Route] Q: "Where is the reachability proof taken?" → Selected: "In the release rehearsal, not in CI." — the gate already pays for a cold build; a second one in CI is duplicate cost.
- [Policy] Q: "A new deny-list step or the existing `core-symbols` one?" → Selected: "The existing step, which already has the machinery and the control." — a control file, reversible.
- [Ledger] Q: "Ids file grammar: one row per handoff with `evidence:`?" → Selected: "`<id> <disposition> evidence: none`, one line each, five rows." — an ids file, reversible.
- [Guards] Q: "One combined guard or two?" → Selected: "Two: the `claims` step for artifacts, the `text` step for words. They fail for different reasons and a single guard would blur which one fired." — two controls, reversible.

### Claude's Discretion
The plan chooses whether the rehearsal mode is a new flag on `tools/release-rehearse.sh`
or a mode of an existing one; how the handoff packet is laid out; and whether the
`text` claim-word addition needs one control or two.

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- `tools/check/steps/claims.sh` already refuses a claim citing an artifact and
  already has controls (`claims-dangling-artifact`, `claims-empty`, …).
- `tools/check/steps/text.sh` already carries a claim-word list with a control
  per word (`text-untagged-claim-word`, `text-untagged-playable`).
- `tools/check/steps/core-symbols.sh` (the `core-symbols` deny list) already
  refuses a defined symbol and already has `core-symbols-palette-defined`.
- `tools/release-rehearse.sh` already builds and installs; it needs the
  source-archive mode.
- `data/expected/` already carries `<id> <fields>` ids files of exactly this
  shape (`handoffs.ids` is a new one of the same grammar).

### Established Patterns
- Oracle-first with a proved ancestry; a frozen document before its check.
- Every new check ships a must-fail mechanism, a liveness guard and a committed
  scratch stand-in (§2.3, lesson 20).
- A count delta is predicted before the commit and compared after (§3).
- A handoff is a ledger row, never prose (`AUTONOMY.md` §3).

### Integration Points
- `README.md`, `docs/packaging.md`, `docs/running.md`, `docs/presentation.md` are
  public prose and are scanned by the `text` and `claims` steps.
- `tools/release-rehearse.sh` is release-time only; the gate never runs it.
- `data/expected/handoffs.ids` is read by the `claims` step and cited by later
  milestones.

</code_context>

<specifics>
## Specific Ideas

The frozen upstream scope is `.planning/MILESTONE-CONTEXT.md` §2 Phase 10, which
numbers five deliverables. Two sentences from it are load-bearing enough to
repeat:

- "The handoff is `not-performed` unless a human reports back."
- "Not in this phase: installing `bin/ivs-run` or any image (see §3, D14 — it is
  structurally blocked, not merely expensive)."

</specifics>

<deferred>
## Deferred Ideas

- **Installing the runner or any image** — D14, structurally blocked.
- **A windowed frontend, a palette table, a second cartridge** — REQUIREMENTS
  "Out of Scope".
- **Any new gate step** — forbidden this milestone.
- **Running H7-1/H7-2** — the owner's, not a tool's; the ledger row is the
  deliverable.

</deferred>

<amendments>
## Amendments (appended 2026-11-25, after `10-RESEARCH.md`)

The research measured four places where a frozen sentence cannot be satisfied as
written. Each amendment is **appended**, never an edit of the text above it, and
**supersedes the conflicting sentence wherever a plan, executor or verifier
cites it** (GUARDRAILS 2.9: when plan prose and the frozen record differ, the
recorded amendment wins). The measured evidence is `10-RESEARCH.md` Q1, Q4, Q5,
Q10 and Q11; the numbers below were re-measured on the planning branch at
`d3952ea`.

### A-01 — Criterion 4's premise is false: the sentence is **made** identical, not kept identical

**Frozen text superseded:** decision "The route cannot become an install claim —
the sentence is kept **identical** in `README.md` and `docs/packaging.md`",
MILESTONE-CONTEXT §2 Phase 10 item 3, and ROADMAP criterion 4's "carry one
identical ... sentence".

**Measured at `d3952ea`:** `grep -c -F 'nothing downloadable is runnable'`
returns **1** for `docs/packaging.md` (line 121) and **0** for `README.md`.
`README.md:17` states the same fact in different words. A sentence that appears
once cannot be "kept" identical: `README.md` must **gain** the phrase.

**Second measurement:** `docs/packaging.md:121` is a `- ` bullet whose sentence
continues past the phrase, so a whole-line comparison (`grep -x`) is
unsatisfiable there without destroying the bullet list.

**Amendment:** the checked object is the 34-byte subtree of the phrase, and the
proof is `grep -c -F` (line-wise) in **both** files, each equal to 1 — never
`grep -x`. `README.md:17` is edited minimally to contain the phrase and keep its
meaning; `docs/packaging.md:121` is left byte-identical ("do not edit a sentence
that is still true", Phase 8 rule). Both files gain one paragraph pointing at
`docs/running.md`. The criterion is satisfied by making the sentence identical.

### A-02 — Criterion 2's mechanism limb is **already delivered**: the policy cites a rule that pre-dates it

**Frozen text superseded:** ROADMAP criterion 2's "one deny-list check refuses a
colour symbol defined under `core/src`, shown able to fail by a committed
scratch stand-in; the check adds no gate step", read as work to be built here.

**Measured at `d3952ea`:** `tools/check/core-symbols.sh:163-173` already refuses
a defined symbol matching `palette|rgb|ppm|framepreview` (the rule header is at
`:38-41`), it catches a file-local `static` too, its committed stand-in
`tools/check/controls/core-symbols-palette-defined/` exists with
`EXPECT -> FAIL core-symbols-palette-defined:`, and the control is green in
today's gate (`controls=386/386`). All three limbs of the criterion are true
today.

**Amendment:** Phase 10 writes `docs/presentation.md` and **cites** the rule; it
does not build a second colour mechanism. Consequently the oracle order is
**inverted** relative to GUARDRAILS 2.9: here the mechanism (plan 06-09) is an
**ancestor** of the policy, and the ancestry is proved in the direction history
runs (`git merge-base --is-ancestor <control-adding commit> <policy-adding
commit>`). That inversion is acceptable and recorded because the policy freezes
the **meaning** of a rule that already exists; it does not create the check, and
pretending otherwise would be a false oracle-first claim. TOOL-04 therefore
closes on an **exclusion row** (reason
`enforcement_predates_the_requirement_core_symbols_palette_defined`), not on a
registry row that would attribute a 06-09 mechanism to a Phase 10 requirement.

### A-03 — The tree was red at entry; it is fixed and pinned, so criterion 5 holds at the phase's first commit

**Measured at `f3a6c67` (the research baseline):** `steps=36/38`, from **one**
root cause: a tilde-home literal quoted in
`.planning/phases/09-registered-evidence-and-gate-cost/09-SECURITY.md:122`, which
failed both `hygiene` and `hygiene-history`. Criterion 5 (`steps=38/38`) was
unsatisfiable on that tree.

**Already repaired before this plan set:** `6be961d` rewrote that one sentence
without the literal (the Phase 9 record's meaning is unchanged) and `8f71ce3`
appended the commit that still carries the text to
`data/expected/hygiene-history-pinned.ids` with the same disposition as the six
pins above it. **Measured on the planning branch at `d3952ea`:**
`sh tools/gate.sh` ends `gate: PASS steps=38/38 controls=386/386 ... claims=28
provisional=27 ... gates=107 lines=330`, exit 0.

**Amendment:** `steps=38/38` holds at the phase's first commit, and **no plan
spends a task on it**. Every plan's baseline is the green tree at `d3952ea`, not
the research's red reading, and every count prediction in the plan set is made
against that baseline. A plan that finds `steps` below 38 is a stop (GUARDRAILS
§3), not a re-bless.

### A-04 — The close is a trap: the status flip and its ledger rows are one commit, and `lines`/`gates` are **not** frozen here

**Frozen text superseded:** ROADMAP criterion 5's "keeps `steps=38/38` and
`claims=` at its Phase 9 value", read as freezing every other count.

**Mechanism measured at `d3952ea`:** `tools/check/steps/important-gates.sh:145-181`
**derives** the must-be-covered set from the `## Traceability` rows whose status
begins `Complete`, minus `data/expected/important-gates-excluded.ids`. It refuses
a Complete requirement in neither the registry nor the ledger
(`important-gates-complete-uncovered`) and a ledger row whose requirement is not
Complete (`important-gates-excluded-stale`). So flipping PKG-05, TOOL-04 or
EV-05 to `Complete` turns the tree red **unless** its registry or exclusion row
lands in the **same commit** — the `09-09` pattern.

**Amendment:**

- **Frozen for this phase:** `steps=38/38`, `claims=28`, `provisional=27`,
  `faults=235/235`, `cases=13`, `goldens=29`, `claim-refs=26`, `prefix-files=11`,
  `d10-items=47`, `tests-expected=4958`, `tests-executed=4954`,
  `tests-skipped=4`, `tests-missing=0`.
- **Not frozen, and predicted per commit before it lands:** `lines=330` and
  `gates=107`. The close commit moves them to **`gates=109`, `lines=335`**: three
  registry rows under EV-05 (two `claims` controls, one `text` control, so two
  new gate names) and two exclusion rows (PKG-05 for the release-time rehearsal,
  TOOL-04 per A-02).
- **`files` / `tracked` / `hygiene-files` are derived at run time and are never a
  predicate** (every committed planning artifact moves them by one): a plan
  asserts a **delta** against its task-start baseline
  (`git ls-tree -r --name-only HEAD^ | wc -l`).
- The Phase 9 `lines=330` baseline for the ten unregistered mechanisms of
  deviation D-09-06 is **not** re-derived here (STATE.md's deferred row makes
  that a separate milestone's job); this phase adds rows, it does not rebaseline.

</amendments>
