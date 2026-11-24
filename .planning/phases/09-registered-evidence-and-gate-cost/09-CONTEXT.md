# Phase 9: Registered Evidence and Gate Cost - Context

**Gathered:** 2026-11-24
**Status:** Ready for planning

<domain>
## Phase Boundary

Phase 9 does two things and nothing else.

**Wave A1 (cost only).** Land the last pre-committed cost lever and the three
accepted trims in cost-only commits, measure them, and take a hosted row that
settles the cost level. A1 adds no test id, no control, no fault, no gate step
and no registry line. Trim (b) is declined by its recorded measurement.

**Wave A2 (content-free registration).** After that row: register the 17
requirements whose named faults already run in every gate (EV-02), harden five
named gate items each with a control that fails for its intended reason (EV-03),
add the machine check that makes the registration gap unrepeatable (EV-04), land
the preflight partition so a run reaches a self-consistency verdict before the
long suites, and clear the bookkeeping and settings items that Phase 9 owns.

**Not in this phase:** any new gate step (38 stays 38), any new claim or tier,
any new fault site, any change to the core, the runner's option surface, the
presentation policy, replay, bank modes, or the requirement records themselves
(a requirement is marked complete only by the phase that delivers it, and these
seventeen were already marked complete by their own phases).

**Tier ceiling:** none. This phase adds no claim, moves no tier, and changes no
`claims=` count.

</domain>

<decisions>
## Implementation Decisions

### Wave A1 — cost-only scope, and the sentence that bounds it

- **A1 is one push, and its file list is closed.** The plan enumerates every
  file A1 touches in `files_modified` and each task's `<files>`, and the plan
  carries a hard scope sentence: no file outside that list may change. The list
  is expected to be the runner-scenario inventory, the runner-process tap script
  (and, if the skip vocabulary lives there, the runner-process script or
  inventory), the scenario-pairs inventory, the state restore case, the state
  fuzz case, the test inventory comment lines that name the affected ids, and
  `.planning/CI-EFFICIENCY.md`. `.planning/ci-runs.tsv` is appended by the
  orchestrator, never by an executor. — **Reversibility:** reversible — every
  file is a test-suite or planning file, and reverting one commit restores the
  live launches.
- **Lever (3) keeps its judgement, not just its saving.** `demo-lamp-tone-swap`
  and `demo-lamp-tone-shift` become static repeat variants of one mutant: column
  5 of `data/expected/runner-scenarios.ids` moves from `run` to `release-only`,
  the scenario pair takes the same mode switch, and the tap script gains a
  derivation-equality judgement — the variant's `assert` block must equal, in
  order, the block between the `asserts-begin`/`asserts-end` markers of
  `docs/video/demo-lamp-tone-1-derivation.md`, its `content`/`profile`/`ivss`/`id`
  lines equal the pass scenario's, and its `input` lines are the pass inputs
  transformed exactly as documented. The judgement is the preserving condition:
  without it the fold is a coverage deletion. One suite self-control
  (`runner-process.judge.variant-static`) lands in the same commit and must
  reject a variant whose assert block differs from the derivation by one value.
- **The trims keep their recorded preserving conditions verbatim.** (a) keep the
  `zero` launch of `runner-process.roundtrip.refuse-range` — it alone proves exit
  14, the empty report, the first stderr line and the usage text — and make
  `above-max`, `no-value` and `twice` static against the option table; (c) nine
  digests on every 8th refusal **plus the first and last row**, keeping the
  whole-instance comparison of every row and `produced > 0` as the liveness
  guard; (d) the first value only of each unsealed byte flip, keeping the guard
  that the flip *moves* the digest and the offset set unchanged. Trim (b) is
  declined with its measurement (0.09 local cpu-s against the only sanitized
  execution of the 65-cut loop) and the decline is recorded, not silently
  skipped.
- **A1 is measured before it is pushed.** Three alternated `/usr/bin/time -p`
  runs per preset, both presets, load average recorded, before and after figures
  stated. The lever's measured ceiling (0.12 local cpu-s) is quoted as a ceiling
  and not as a saving: it cannot move `r`.
- **Nothing in A1 may weaken a check to go faster.** No dropped control, fault,
  judge, oracle, golden or pinned-default test; no cache, `continue-on-error`,
  matrix or path filter. This is restated in the plan because A1 is the one phase
  where "make it cheaper" and "make it weaker" are one keystroke apart.

### Wave A2 — EV-02 registration mechanics

- **The 24 lines are chosen from mechanisms that already run.** Each row is a
  `<gate> <fault|control|mutant> <id> <REQ>` line whose id already exists in
  `data/expected/faults/*.ids`, `data/expected/controls/*.ids`,
  `data/expected/{pkg,ci-summary}-selftests.ids` or a
  `<suite>.judge.<slug>`/`<suite>.mutant.<slug>` test id. The plan states, per
  requirement, which existing mechanism is its sole non-vacuous evidence and why
  it cannot pass vacuously. No new fault site, no new test id, no new control.
  — **Reversibility:** reversible — a registry line is one line, and the step
  fails closed if it names an id that is not in an inventory.
- **The registry row count is a prediction, not an observation.** 306 → 330
  (24 additions, 0 deletions), and the plan predicts the `steps`, `controls`,
  `faults`, `tests-expected`, `tests-executed`, `gates`, `lines` and `claims`
  deltas before the commit. An unexpected token is a stop, not a re-bless.
- **The accepted set and all ten toy registries move in the same commit**
  (GUARDRAILS R2). Ten means `tools/check/controls/_good/tree/data/expected/important-gates.ids`
  plus the nine `important-gates-*/overlay/data/expected/important-gates.ids`
  files. Each gains exactly one line per new requirement and zero deletions,
  proved with `git diff --numstat` before the commit. Any
  `important-gates-bad-req-*` control whose never-valid id became valid is
  retargeted in the same commit.
- **The registry prose moves with the code.** The step's `nreq` split string,
  its header comment and its token prose are three separate edits in the same
  commit; a header that lists Phase 4 to Phase 8 requirements and omits Phase 1
  to Phase 3 is the defect this phase exists to remove.

### Wave A2 — EV-04, the machine check that makes the gap unrepeatable

- **Two mechanisms, both inside existing steps, neither a new step.**
  (i) `tools/check/steps/important-gates.sh` stops carrying a hand-written
  accepted set: it derives the must-be-covered set from the requirement records
  and refuses a requirement marked Complete with no registry line. The records it
  reads are `.planning/REQUIREMENTS.md` and the archived
  `.planning/milestones/v0.1.0-REQUIREMENTS.md`; reading `.planning/` from a step
  is established practice (`ci-log.sh` reads `.planning/ci-runs.tsv`,
  `glossary.sh` reads `.planning/PROJECT.md`) and the good tree already carries
  `.planning/`, so the controls can overlay it.
  (ii) `tools/check/run-controls.sh` gains a drift check: every toy registry must
  equal the good tree's registry plus exactly its declared deviation, so a toy
  registry that drifts from the good tree is refused rather than trusted.
  — **Reversibility:** reversible — both are modes of existing scripts, and a
  reverted commit restores the hardcoded set.
- **Both are shown able to fail, by a committed scratch stand-in, in the same
  commit as the mechanism.** The stand-ins are controls, not prose: one for a
  Complete requirement with no registry line, one for a drifted toy registry. A
  mechanism whose control cannot fail proves nothing (lesson 20).
- **Deriving the set must not shrink coverage.** The derivation is from records
  outside the registry, so deleting a registry line cannot silently delete the
  requirement from the must-be-covered set — that is the whole point, and the
  plan states it as the property the control demonstrates.

### Wave A2 — EV-03 gate hardening, one control per item

- **Five items, five controls, five intended reasons, and nothing else.**
  Whole-token `SRC-` match; zero-document failures; per-step pass lines; manifest
  output anchoring; memmap generator controls. Each control's token is pinned in
  `data/expected/controls/<step>.ids` and the control is shown to fail for its
  intended reason and for no other. The controls are gate content, which is why
  they are in A2 and not A1.
  — **Reversibility:** reversible — controls are files under `tools/check/controls/`.
- **A control names the guard that enforces it.** Before writing a control, the
  plan checks that the guard it targets actually lives in the script that would
  refuse it (lesson 7); a control aimed at a guard that is in another script is a
  control that cannot fail.
- **The memmap item is the expensive one and is scoped to what a pure step can
  run.** The generator is a CMake module; the control needs a toy memmap profile
  fixture and a way for a pure step to invoke the generator. If that cannot be
  done without a new step, the item is recorded as deferred with the reason and
  the other four land — never a hollow control.

### Wave A2 — the preflight partition

- **The partition is derived, never hand-maintained.** A step is preflight iff
  its script references neither the gate results directory nor `build/`. The
  derivation is written into `tools/gate.sh`, the partition is printed in the
  header of the run, and each step's tokens and `-- FAIL` lines are buffered and
  concatenated in sorted name order so the summary line and the output order stay
  byte-identical while execution order changes.
  — **Reversibility:** reversible — one file, and the summary line is the proof.
- **The preserving condition is byte-identical output, and it is checked.** The
  plan requires the full `sh` and `dash` gate output captured before and after
  the change and compared, not just the final line: a partition that reorders
  evidence but keeps the last line is still a change to what a reader sees.
- **Hosted cost is unchanged by construction** and the plan says so; the win is
  local time-to-first-signal, which is a developer-experience win and is not
  claimed as a hosted saving.

### Bookkeeping, settings and the rulesets

- **The branch ruleset is applied, and the tag ruleset is applied in the only
  shape a ruleset can express.** Branch: PR required with 0 approvals, required
  status check `gate`, no up-to-date requirement, no force push, no deletion, no
  bypass list — the command is written out in `.planning/AUTONOMY.md` §1 and the
  applied date and ruleset id are recorded there. Tag: target `tag`, ref
  `refs/tags/v*`, deletion and non-fast-forward refused. "Created only from a
  green `main`" is **not** expressible in a ruleset and is recorded as such
  rather than faked by a rule that does something else.
  — **Reversibility:** costly — the ruleset is a public repository-settings
  object, and although deleting it is one API call, an applied ruleset changes
  who can land a commit; the reversal command is recorded beside the id.
- **`main` protection is already real and the record is corrected, not
  rewritten.** Legacy branch protection with required check `gate` exists; the
  v0.1.0 close said "no ruleset applied", which was true of rulesets. The
  migration to a ruleset is the action; the historical line stays and the
  correction is appended.
- **Bookkeeping is one commit of its own.** The stale `D-31` `create_tag` text,
  `RELEASE-OPS.md`'s `create_tag: true` and "ten files", the stale
  composite-version comment in `core/src/machine/machine_digest.c`, the two bench
  golden commands into `docs/runner.md`, and the disposition of the seven draft
  `VALIDATION.md` files (by citing `08-ACCEPTED-DEBT.md` §1 in a live record —
  never a rewrite of an archive).
- **A disposition is a row, not a sentence.** Every deferred item Phase 9
  declines to do gets a row in the live record with its reason and the milestone
  it now waits for, per `INDEX.md` rule 5.

### Ordering, the hosted row and who does what

- **The row is a gate on A2, not a milestone formality.** No test id, control,
  fault, registry line or gate step is committed before a hosted row recorded
  after the A1 commits exists, and the plan proves the ancestry with
  `git merge-base --is-ancestor <row-commit> <first-id-adding-commit>`.
- **Only the orchestrator pushes, opens the PR, records the row and touches
  repository settings.** An executor never calls `gh`, never pushes, and never
  stages `.planning/config.json` or `.planning/state.json`. A1 ends with the
  orchestrator's row; A2 begins only after that row exists.
- **The row settles one of three words and no others.** `phaseA1:
  discharged-below-budget` (≤7.8), `discharged-within-line` (7.8–8.3) or
  `lever-required` (>8.3, possible only at the second row after `R1`). The
  median is recomputed by arithmetic from `.planning/ci-runs.tsv` and the five
  values are printed; a quoted figure is a defect.
- **A2 may spend at most the recorded allowance** (about 21 hosted cpu-s for the
  whole milestone, ≈12.9 local cpu-s at the recorded 1.675 factor) and the plan
  prices each wave against it. The allowance is a ceiling, not a target.

### Decided autonomously
- [Wave A1] Q: "Which files may wave A1 touch?" → Selected: "The plan enumerates a closed list (runner-scenarios inventory, runner-process tap/script, scenario-pairs inventory, the state restore and state fuzz cases, the affected test-inventory comment lines, CI-EFFICIENCY.md) and no file outside it changes." — reverting one commit restores every live launch, and every file is a test-suite or planning file.
- [Wave A2 / EV-02] Q: "Which mechanisms do the 24 rows name?" → Selected: "Only ids that already exist in a fault, control, mutant or suite self-control inventory; the plan states per requirement why the mechanism cannot pass vacuously." — a registry line is one line and the step fails closed on an unknown id.
- [Wave A2 / EV-04] Q: "Where does the machine check live?" → Selected: "A derived accepted set inside `important-gates.sh` plus a toy-registry drift check inside `run-controls.sh`; no new step." — both are modes of existing scripts and a revert restores the hardcoded set.
- [Wave A2 / EV-04] Q: "Which records define 'marked Complete'?" → Selected: "`.planning/REQUIREMENTS.md` and `.planning/milestones/v0.1.0-REQUIREMENTS.md`, both already reachable from the good tree and from the step's `$ROOT`." — reading `.planning/` from a step is established by `ci-log.sh` and `glossary.sh`.
- [Wave A2 / EV-03] Q: "One control per hardening item, or a control per failure mode?" → Selected: "One control per item, with its token pinned and shown to fail for its intended reason only." — the requirement asks for one per item and a control is a file under `tools/check/controls/`.
- [Wave A2 / preflight] Q: "Hand-maintained cheap list or derived partition?" → Selected: "Derived: preflight iff the script references neither the gate results directory nor `build/`, buffered and concatenated in sorted name order." — one file changes and the summary line is the byte-for-byte proof.
- [Settings] Q: "Which shape for the tag ruleset?" → Selected: "Target `tag`, ref `refs/tags/v*`, deletion and non-fast-forward refused; 'created only from a green main' recorded as not expressible." — a ruleset cannot express provenance, and faking it with a different rule is worse than saying so.

### Claude's Discretion
The plan may split A2 into more plans or fewer than the phase record suggests,
and may order the EV-03 items by cost. The plan may decide that the memmap
generator control needs a fixture the phase cannot build without a new step, in
which case it records the deferral with its reason. The plan chooses whether the
skip vocabulary for a `release-only` scenario lives in the tap script or in
`tests-inventory.sh`, provided the id still prints one `-- SKIPPED` line and is
counted in `tests-skipped` exactly as before.

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- `tools/check/steps/important-gates.sh` (111 lines) already resolves a registry
  id against the three inventories (faults, controls, mutants incl. suite
  self-controls) and already prints `gates=<n> lines=<n>`; only the accepted set
  and the toy-registry drift check are new.
- `tools/check/run-controls.sh` (319 lines) already drives every control and
  every whole-file overlay tree, so the drift check has a home.
- `tools/gate.sh` (181 lines) already owns step ordering and the summary line, so
  the preflight partition is one file.
- The good tree `tools/check/controls/_good/tree` (120 files) already carries
  `.planning/`, `data/expected/` and `build/gate/`, so overlaying requirement
  records needs no new machinery.
- `data/expected/runner-scenarios.ids` already has the column-5 mode vocabulary
  (`run`, `release-only`) and `data/expected/scenario-pairs.ids` already carries
  the pair line for pass/shift.

### Established Patterns
- Every count is a prediction stated before the commit and compared after
  (GUARDRAILS §3). The gate summary token is the contract.
- Every mechanism ships with a liveness guard and a committed scratch stand-in
  proving the guard can fail (GUARDRAILS R2/§2.3, lesson 20).
- Oracle-first with `git merge-base --is-ancestor` where a frozen document
  precedes code. Phase 9 has no new oracle document, but the derivation-equality
  judgement of lever (3) cites the existing frozen derivation.
- A trim keeps its preserving condition in the same commit as the trim; a trim
  without one is a coverage deletion.

### Integration Points
- A1 touches the runner-process and state suites only; A2 touches the checker
  steps, the control runner and the gate script.
- The hosted row is the seam between A1 and A2 and is owned by the orchestrator,
  not by any executor.
- `.planning/ci-runs.tsv`, `.planning/CI-EFFICIENCY.md`, `.planning/AUTONOMY.md`
  (ruleset ids) and `.planning/STATE.md` are orchestrator records; executors
  never stage them.

</code_context>

<specifics>
## Specific Ideas

The owner's recorded constraints for this milestone are the specification, and
the plan should cite them by path rather than restate them:
`.planning/MILESTONE-CONTEXT.md` §2 Phase 9, `.planning/ops/GUARDRAILS-v0.2.md`
R2 and §2, and `.planning/CI-EFFICIENCY.md` "next-milestone opening" (lever (3)
mechanics, the four trims with their preserving conditions, the preflight
partition rule).

Two sentences from that material are load-bearing enough to repeat here:

- "The lever cannot move the metric; it is landed because it is recorded policy
  and because it is the last non-owner lever in the pre-committed order."
- "Recording that plainly is the point — an unmeasured lever quoted as a saving
  is how a cost story rots."

</specifics>

<deferred>
## Deferred Ideas

- **Installing `ivs-run` or any image** — D14, structurally blocked; not a Phase
  9 idea and not reopened here.
- **A windowed frontend, a palette table, a second cartridge** — out of scope for
  the milestone (REQUIREMENTS "Out of Scope").
- **A new gate step for any of these checks** — forbidden this milestone; if a
  future check needs one, the reversal trigger is recorded in
  `REQUIREMENTS.md` "Out of Scope" (new ids above about 15 % of an existing
  suite, or a defect found only at a capability seam).
- **Trim (b)** — declined by measurement, recorded in `CI-EFFICIENCY.md`.
- **Running negative controls in parallel** — the standing gate-speed idea; it
  stays a watching item governed by the cost rule and is not landed in A1.

</deferred>

---

<amendments>
## Amendments (A-01 to A-08)

**Status:** appended 2026-11-24 after `09-RESEARCH.md`, before any plan exists.
**Authority:** an amendment supersedes conflicting text above. Each one exists because
the research measured the tree and found the frozen sentence unsatisfiable, hollow,
or factually wrong. Per `PLAN-LESSONS` 19, reality wins and the plan sentence becomes
a recorded deviation — never code written from the frozen sentence.

### A-01 — EV-04 derives from the records, with an explicit exclusion ledger (shape S1)

**Measured:** 50 requirements are marked `Complete` in
`.planning/milestones/v0.1.0-REQUIREMENTS.md`'s traceability table; the registry
covers 27; EV-02 adds 17 → 44. A naive "every Complete requirement must have a line"
derivation therefore goes **red on the pristine tree** for six requirements:
`API-01 API-05 API-06 RUN-03 PKG-02 REL-01`.

**Decision:** land `data/expected/important-gates-excluded.ids` with exactly those six
rows, each `<REQ> <reason>`, and make the step refuse a Complete requirement that is
**neither registered nor excluded**. Rejected: growing EV-02 to 23 requirements
(`REL-01` has no gate mechanism at all — it is a close-record requirement, so the
set cannot be completed honestly), and deriving only from the live
`.planning/REQUIREMENTS.md` (10 `Pending` rows → a vacuous check, forbidden by
lesson 20).

The property that makes the gap unrepeatable still holds: the exclusion set is a
*different file* from the registry, so deleting a registry line cannot silently
delete the requirement from the must-be-covered set. The control overlays
`.planning/REQUIREMENTS.md` with one row flipped `Pending` → `Complete` and no
registry line, and the step must fail.

**Parse rule:** the `## Traceability` table (`| <REQ> | Phase <n> | <Status> |`), id
from field 2, status from field 4, `Complete*` as a prefix match. Not the
`- [x] **REQ-NN**:` checkbox lines — the traceability table is the record the
project already treats as authoritative.

### A-02 — EV-03's honest count is three new controls, one verification and one step-side mechanism

**Measured, item by item:**

| Item | Guard | Landed? | Control today | Reachable from a control? |
|---|---|---|---|---|
| whole-token `SRC-` match | `tools/check/steps/sources.sh:40` | no | no | yes (pure step) |
| zero-document failures | `glossary.sh:48`, `sources.sh:33-35` | **yes** | **yes** (`glossary-no-files`, `sources-no-files` in `gate-hardening.ids`) | yes |
| per-step pass lines | `tools/gate.sh:111-116` | **yes** | none | **no** — `run-controls.sh` only executes `tools/check/steps/*.sh` |
| manifest output anchoring | `tools/check/manifests.cmake:88-94` | no | no | yes (pure step) |
| memmap generator controls | `cmake/IvsMemmapProbes.cmake:14-19` | no (nothing runs it) | no | **yes**, via a `cmake -P` invocation inside `memmap.sh` |

**Decision:** three new controls (whole-token `SRC-`, manifest anchoring, memmap
generator), one verification (zero-document failures — cite the two existing controls
and, if cheap, extend to `denylist-no-files` and `runner-scan-empty`, which currently
have none), and one step-side mechanism for the per-step pass line: require each
step's pass line to be the **last non-empty line**, enforced in `run-controls.sh`'s
`good_one`. `controls=380/380 → 383/383` (+2 if the two zero-document extensions land).

**Recorded deviation:** ROADMAP success criterion 4 says "each of the five
gate-hardening items has a named control". Two of the five already have one and one
(the pass-line guard in `tools/gate.sh`) is unreachable from the control runner
without a second gate shape and a recursion. The criterion's *intent* — every item
has a mechanism shown to fail for its intended reason — is met; the literal
five-controls count is not, and the deviation is recorded rather than faked. A control
aimed at a guard it cannot reach is the "check that cannot fail" of lesson 20.

**The memmap item is landed, not deferred.** The generator is already a standalone
`cmake -P` script with fail-closed messages; `memmap.sh` gains one invocation writing
into its own `mktemp -d`, mapped to one new token. No new gate step, `steps=38/38`
unchanged, and `memmap` is a producer step so the preflight partition is undisturbed.

### A-03 — A1 adds no test id, so the static-variant judgement reuses an existing judge

`runner-process.judge.variant-static` as a new id would violate `GUARDRAILS` §2.2
(no new test id before the hosted row) — and `CONTEXT.md` above both requires and
forbids it. **Decision:** no new test id in A1. The derivation-equality judgement
ships as an assertion inside the existing variant case, driven by `mutant_case`'s
`$config = Release` two-mode switch (an ordinary `ok` line, **never** a skip — a skip
moves `tests-skipped` 4 → 8 and stales every recorded token). Its committed scratch
stand-in is a one-value-changed scenario body, run once in the commit and its result
recorded in the SUMMARY, so the guard is shown able to fail without entering the
gate. If an existing `runner-process.judge.*` id can carry the judgement without
changing its meaning, prefer that; otherwise the in-case assertion plus the recorded
stand-in is the form.

### A-04 — the variant's `id` line cannot equal the pass scenario's

`CONTEXT.md` above says the variant's `content`/`profile`/`ivss`/`id` lines equal the
pass scenario's. Unsatisfiable for `id`. **Decision:** `content`, `profile` and
`ivss` equal the pass scenario's; `id` carries the variant's own id. The slice rule is
the verified one: the derivation block minus its fence lines and leading `#` lines
equals the scenario body after `ivss 1` minus its header lines (55 / 29 / 55 / 49
asserts for pass / idle / shift / swap, and the judgement passes today on all four).

### A-05 — "byte-identical output" means identical after normalising the measurement lines

Five things are emitted in sorted step-name order today: the step's stdout+stderr, its
`-- FAIL` lines (into the job summary, not stdout), a possible
`-- FAIL gate-step-*` line, its `step-time <name> <s>` line, and its tokens (which
become the summary tail). **All three of the stdout-affecting ones must be buffered
and replayed in sorted order**, and `keep_fails` must run during the replay, not
during execution.

`step-time`, `gate-cpu`, `gate-time` and `gate-env` carry measurements and can never
match literally. **Decision:** the before/after comparison normalises those four line
shapes and requires the normalised streams to be identical, plus the summary token
byte for byte. The pre-change `sh` and `dash` captures are committed as the evidence
the diff is taken against.

### A-06 — the bookkeeping items, each corrected by measurement

- `git.create_tag` **already reads `false`** (`.planning/config.json:15`). The D-31
  item is a **prose** fix in `PROJECT.md` and `RELEASE-OPS.md` only. An executor must
  never stage `.planning/config.json`.
- `RELEASE-OPS.md` says "the pinned ten files"; `data/expected/pkg-install-files.ids`
  has **11** non-comment lines. Change "ten" to "eleven".
- The stale composite-version sentence is `core/src/machine/machine_digest.c:12-14`
  ("version 2 since plan 05-03") against `IVS_DIGEST_COMPOSITE_VERSION 3u`. **Not**
  line 407, whose comment describes a fault site and is correct in context.
- `docs/runner.md` carries **no** bench command; the two bench golden writing
  commands are added there (`GUARDRAILS` R11).
- Seven draft `VALIDATION.md` files, not eight: `01` is `status: validated`, `02`–`08`
  are `status: draft`. Disposition them by citing `08-ACCEPTED-DEBT.md` §1 in a live
  record. **Never rewrite an archive.**

### A-07 — the cost figures: the recorded allowance stands, the recomputation is stated

Recomputed from `.planning/ci-runs.tsv`: the last five `r` are 7.4167, 7.5333, 7.6957,
7.8261, 8.3810 → **median 7.696** (0.104 under the 7.8 budget, 0.604 under the 8.3
stop line). The **recorded** allowance the phase was priced against — about 21 hosted
cpu-s ≈ 12.9 local cpu-s — stands; the recomputation is stated beside it so a reader
is not surprised that the arithmetic now shows more headroom than the record.

A2's spend is ≈ **1.2 local cpu-s** (five controls at 0.185 each, plus one `cmake -P`
launch), ≈ 0.7 hosted, ≈ **+0.015 `r`**. A1's saving is ≈ 1.10 local cpu-s (lever 0.12
ceiling + trim (a) 0.18 + trim (c) 0.4 + trim (d) 0.4). The cost risk of this phase is
A1's measurement discipline, not A2's spend.

### A-08 — the registry delta is `gates=90 → 107` and `lines=306 → 330`

The 24 proposed rows name 17 gate names that do not exist today (no `cpu*`, no
`memmap*` except `memmap-flip`, no `machine`/`ivsc` family). Naming them honestly is
worth the `gates=` delta; reusing existing family names to hold `gates=90` would make
the registry say a CPU opcode expectation and a video rule are the same gate. Predicted
token delta for the EV-02 commit: `gates=90 → 107`, `lines=306 → 330`, and
`steps`, `controls`, `faults`, `tests-expected`, `tests-executed`, `tests-skipped`,
`claims` unchanged.

### A-09 — the EV-04 drift check needs a declaration format and two pinned tokens

Each of the nine `important-gates-*/` overlays differs from
`_good/tree/data/expected/important-gates.ids` by **exactly one non-comment line**
(measured, comments excluded — each overlay replaces the header by design). The drift
check in `run-controls.sh` compares sorted non-comment lines in both directions and
refuses any difference that is not declared, and any declaration that matches no
difference (so a `DEVIATION` file cannot rot). Two new tokens, `control-toy-drift` and
`control-deviation-unused`, are pinned in an ids file. The good tree gains minimal toy
`.planning/REQUIREMENTS.md` and `.planning/milestones/v0.1.0-REQUIREMENTS.md` records
listing exactly the ids its toy registry covers, so the `_good` positive control is
consistent by construction.

**Pitfall to respect:** appending the 17 toy lines anywhere but after the existing last
line shifts the positional EXPECT tokens of `important-gates-bad-req-phase5`
(`line 19`) and `-phase6` (`line 24`). Append at the end of all ten files.

</amendments>
