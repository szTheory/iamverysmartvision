# Phase 13: Rights Ledger and Enforced Claim Ceiling - Context

**Gathered:** 2026-11-26
**Status:** Ready for planning

<domain>
## Phase Boundary

Phase 13 is the milestone's entry ticket. It makes two things mechanical before
any later phase names a third-party-content item or mints a claim:

1. A **question-shaped rights ledger** under `data/rights/` — one JSON file per
   third-party-content question this milestone touches, each with a
   `disposition` of `cleared`, `refused` or `owner-local-only`, an optional
   `SRC-` id, a "what was taken" cell and the spellings a refused item must
   never appear as. Enforcement lands as ids **inside the existing `manifests`
   and `files` steps** (`data/expected/gate-steps/rights.ids` already owns both):
   a `cleared` row with no resolvable source is refused by name, and a refused
   row's `tokens` are refused in `core/src`, `runner/`, `tools/` and the
   operator pages of `docs/`, with pre-existing hits pinned by
   `(token, path)`.
2. **D-32's behavioural-claim ceiling, mechanically** — an `established` claim
   must be a `source-expectation` claim whose `depends_on` fields all carry a
   `tier1-primary-doc` tag; `CL-0002` is the only claim this permits and it is
   checked rather than assumed, with a legal `established` claim still passing.
3. **The ten D-09-06 must-fail mechanisms** registered as `mutant` rows over
   their existing selftests, with the `important-gates` baseline re-derived in
   the same commit and all ten toy registries extended — **zero new faults**.

Out of scope: any claim of this phase's own (tier ceiling `none`), any content
byte, any wrap tool (Phase 14), any host target (Phase 15), any operator route
(Phase 16). No new gate step, fault or golden: `steps=38/38`, `faults=238`,
`goldens=29` are invariants.

</domain>

<decisions>
## Implementation Decisions

### Rights ledger shape and enforcement
- One file per item at `data/rights/<slug>.json` with keys `id`, `item`, `disposition` ∈ {`cleared`, `refused`, `owner-local-only`}, `source` (a `SRC-` id or `null`), `taken` (`"nothing"` legal), `tokens`, `note` — frozen upstream by D-46 and scoping R-1. — **Reversibility:** one-way — the ledger is a persisted data schema that every later phase's rows, the `docs/RIGHTS.md` table and the `rights.ids` membership key off; changing a key later is a coordinated rewrite of rows, ids and the doc table.
- Enforcement is ids inside the existing `manifests` and `files` steps. `data/expected/gate-steps/rights.ids` already names both; no new step is added and `ls tools/check/steps/*.sh | wc -l` stays 38.
- The naming limb is **token + scope, never a global grep**. `cfg` and `ivsc` already exist in `tools/check/binary-ext.txt` and `.gitignore` as *refusal surfaces*; removing them would delete a refusal. Pre-existing hits are pinned by `(token, path)` in `data/expected/rights-pinned.ids`, mirroring `hygiene-history-pinned.ids`.
- Row set for this phase, one row per RTS-01 question: `flat-cartridge-image`, `cfg-container`, `bank-modes`, `exec-image`, `grom-image`, `commercial-rom`, `rendered-frame`, `save-state`, `libretro-abi`, `core-info-strings`.
- The ten row spellings are the operator-facing nouns an item must never be named as. A row's `tokens` are data, and a refused row's tokens are refused only inside the four scopes above.
- `cleared` resolvability is checked against `data/sources.json` membership only. The `rights` field on a source record describes the *reference document's* own rights (it is `unknown` for every source today) and answers a different question; consulting it would make every `cleared` row impossible and would conflate two ledgers. — **Reversibility:** costly — the check is one id, but a later change re-opens every `cleared` row and its control overlay.
- "Operator pages of `docs/`" is an explicit pinned list in the check's data, not a `docs/**` glob: the pages that hand an operator a command or a file (`README.md`, `docs/running.md`, `docs/owner-images.md`, `docs/packaging.md`, `docs/diagnostics.md`, `docs/RIGHTS.md`). Adding a page is a deliberate act with its own diff.
- The ledger is **not** a second manifest schema. A question whose answer is a byte set becomes a fixture-manifest entry; the ledger answers "may this be named / read / shipped", the manifest answers "is this the byte set". Recorded as the reversal condition.

### Enforced claim ceiling (D-32 / D-50)
- The check lives in `tools/check/kinds/claim.cmake`, which already judges each claim row, and lands as ids in the `claims` step. `data/expected/gate-steps/claims.ids` already names `claims` and `text`.
- Rule: an `established` standing requires `tier == source-expectation` **and** every id in `depends_on` resolving to a profile field tagged `tier1-primary-doc`. Anything else is refused by name.
- The positive case is a **toy** legal `established` claim in the checker's own fixture set, not a mutation of `CL-0002`. `CL-0002` stays the only live established claim and is asserted, not edited, so the live ledger and its `claims.ids` baseline do not move for a test.
- Two must-fail controls and one legal positive, per GUARDRAILS V4: `claim-established-non-source`, `claim-established-without-tier1`, `claim-established-positive`.

### The ten D-09-06 registrations (EV-07)
- Each of the ten mechanisms gains an `important-gates.ids` **`mutant` row over its existing selftest**, never a fault. The canonical list is `09-05-SUMMARY.md`: `important-gates-complete-uncovered`, `important-gates-excluded-stale`, `important-gates-excluded-malformed`, `control-toy-drift`, `control-deviation-unused`, `control-pass-line-not-last`, `sources-src-token-whole`, `manifests-output-anchor`, `memmap-generator-refused`, `memmap-generator-missing`.
- The registry baseline (`lines=` in the header, and any excluded-count baseline) is re-derived in the same commit as the ten rows. The count edit is part of that commit, never a later "re-bless" commit (GUARDRAILS V7, V11).
- All ten toy registries gain exactly one line each with zero deletions, and `faults` stays 238. Ten faults instead of mutants would cost about 4.4 hosted cpu-s — 82 % of the milestone's allowance — on the step already 29 % of the gate.

### Gate cost and evidence discipline
- Priced addition for this phase: **2.0 hosted cpu-s** against a 21 cpu-s allowance; the phase-boundary checkpoint recomputes the last-five `cpu_s`/`gate_s` medians from `.planning/ci-runs.tsv` by arithmetic and records exactly one of `discharged-within-allowance` / `reprice` / `lever-required`.
- Every new check ships three things in one commit (GUARDRAILS §2.4): the must-fail mechanism registered in `data/expected/important-gates.ids` under its requirement, a liveness guard, and a committed scratch stand-in showing the **guard** fail.
- No case may assert only `exit 0` / `outcome pass` (GUARDRAILS §2.5): each asserts an observed domain, a witness pair or a count.
- Retargets are same-commit per GUARDRAILS V1 (rights row → ledger file, `rights.ids`, the `docs/RIGHTS.md` row, the reading id), V2 (a refused spelling → tokens, pinned ids, control overlay), V4 (an `established` standing → the mechanism, two must-fail controls, one positive), V11 (a D-09-06 mechanism → a `mutant` row).

### Decided autonomously
- [Rights ledger shape and enforcement] Q: "What slugs name the ten RTS-01 rows?" → Selected: "kebab-case item nouns, one file each: `flat-cartridge-image`, `cfg-container`, `bank-modes`, `exec-image`, `grom-image`, `commercial-rom`, `rendered-frame`, `save-state`, `libretro-abi`, `core-info-strings`" — a file rename plus its id line; undo is local.
- [Rights ledger shape and enforcement] Q: "Does a `cleared` row's source resolvability consult the source record's `rights` field?" → Selected: "No — membership in `data/sources.json` only; the source `rights` field answers the reference document's own rights, a different question" — the rule is one branch in the check; reversing it is one line plus the ids that cover it.
- [Rights ledger shape and enforcement] Q: "Which pages count as the operator pages of `docs/`?" → Selected: "an explicit pinned list (`README.md`, `docs/running.md`, `docs/owner-images.md`, `docs/packaging.md`, `docs/diagnostics.md`, `docs/RIGHTS.md`), not a `docs/**` glob" — the scope is a data list; extending or narrowing it is a one-line diff.
- [Rights ledger shape and enforcement] Q: "Where does the rights check get its `SRC-` resolution and step membership helpers?" → Selected: "reuse `tools/check/manifest-lib.cmake` and the existing `files`/`manifests` step harness rather than a new library" — a helper extraction is local to two files.
- [Enforced claim ceiling (D-32 / D-50)] Q: "Is the legal `established` positive `CL-0002` itself or a toy row?" → Selected: "a toy legal claim in the checker's fixture set; `CL-0002` is asserted unchanged" — a fixture swap touches no live baseline.
- [The ten D-09-06 registrations (EV-07)] Q: "How are the ten rows named and where does the baseline live?" → Selected: "`mutant` rows named after their existing selftest cases; the `lines=` header of `data/expected/important-gates.ids` is re-derived in the same commit" — a count is arithmetic over a committed file; re-deriving it is the same command.
- [The ten D-09-06 registrations (EV-07)] Q: "Do the ten toy registries gain their line in the same commit as the rows?" → Selected: "yes — one line each, zero deletions, in the row commit (GUARDRAILS V7)" — the toy registries are test data; a split commit is a lint failure, not a design choice.
- [Gate cost and evidence discipline] Q: "Does the phase-boundary checkpoint add a CI measurement run?" → Selected: "no — it recomputes from the committed `.planning/ci-runs.tsv` rows" — the file is the arithmetic input; re-reading it costs nothing.

### Resolved from the research open questions (2026-11-26)
- **EV-07 registration kind is the kind that actually proves the mechanism.** Measured against the live inventories: five of the ten already resolve as `control` (they have control directories), four are proved only by `runner-selftest.sh` cases and resolve as nothing, and one (`important-gates-excluded-stale`) is proved only as a co-firing `EXPECT` line. The registration is therefore 5 `control` rows (zero cost), 4 `mutant` rows via a new `data/expected/runner-selftest.ids` read by the existing resolution loop, and 1 new `control` directory for `important-gates-excluded-stale` (0.31 cpu-s). This narrows EV-07's literal "`mutant` row" wording and is recorded as an explicit deviation in the same commit, with the reason: a `mutant` row for a mechanism already proved by a control directory would be a second registration of one proof and would require inventing a selftest that re-proves it. Zero new faults holds.
- **One live `cleared` row is produced by adding one `reference-only` source record.** The libretro API documentation is recorded in `data/sources.json` + `data/expected/sources.ids` as `capture: reference-only`, `rights: unknown`, with the `sources-reference-only-unexplained` explanation satisfied, and `libretro-abi` becomes the one live `cleared` row citing it. Nothing is vendored (D-48); the record exists so the `cleared` path is exercised on the real tree, because a control directory can never express "this passes". The `sources` stat delta is predicted and then measured.
- **A stale `(token, path)` pin is a failure, not a NOTE.** `rights-pinned-stale` fires when a pin no longer matches, so the pin list cannot rot into a wildcard or a silently widened rule. (`history_compare`'s NOTE is correct for history, where a pin names a commit that cannot disappear; a rights pin names a path that can.)
- **"Baseline re-derived" means run the step and quote its printed `gates=`/`lines=`.** `lines=` is a runtime stat, not a header field; the step header's phase table gains the Phase 13 line in the same commit.
- **The toy registries gain exactly one line each.** One new toy requirement row in the two toy requirement records plus the whole-file overlay copy, and one registry line appended to each of the ten whole-file toy registries: 13 files, 13 added lines, zero deletions, no `DEVIATION` entry. The toy record is a curated mirror, not an automatic one.

### Claude's Discretion
None — no question was delegated to Claude; every question above was either frozen upstream, decided autonomously under the reversible-technical rule, or resolved from the research open questions with the reason recorded above.

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- `data/expected/gate-steps/rights.ids` already names the `manifests` and `files` steps as the rights owners (plan 01-06), so RTS-01/02 are ids, not a new step.
- `tools/check/manifest-lib.cmake` and `tools/check/files.cmake` / `manifests.cmake` carry the existing id/harness machinery, including the `hygiene-history-pinned.ids` `(token, path)` pin idiom RTS-02 mirrors.
- `tools/check/kinds/claim.cmake` already judges claim rows (tier, standing, `depends_on`); D-50 is a rule inside it.
- `tools/check/important-gates.sh` plus `data/expected/important-gates.ids` / `important-gates-excluded.ids` already implement `fault|control|mutant` rows and the `lines=` baseline; the ten rows are additions, not new machinery.
- `tools/check/run-controls.sh` and `data/expected/controls/` carry the `EXPECT`-token control idiom for the two must-fail controls and the legal positive.
- `docs/RIGHTS.md` already exists as the human-facing rights page; GUARDRAILS V1 requires its row to move with each ledger row.

### Established Patterns
- Every capability is an id inside an existing suite; every new equivalence gets a control whose stand-in fails it; a check enters CI only if it catches a recurring class (frozen verification strategy).
- `data/expected/*.ids` files are the membership baselines; adding or removing an id moves the baseline in the same commit.
- Faults are judged by `faults-verdict` with must-fail and must-pass vectors; controls by `run-controls.sh`; mutants are in-step selftest cases or `<suite>.mutant.<slug>` self-controls.
- Records are append-only; a live document gets an inline pointer plus an appended amendment, never a rewrite.

### Integration Points
- `data/rights/` (new) → `files`/`manifests` ids → `data/expected/rights.ids` → the `rights` gate step.
- `data/expected/rights-pinned.ids` (new) → the naming limb of the same ids.
- `tools/check/kinds/claim.cmake` → the `claims` step → `data/expected/claims.ids`.
- `data/expected/important-gates.ids` → the `important-gates` step, with the ten toy registries under `tools/check/controls/` and the selftests they name.
- `docs/RIGHTS.md` → the operator-facing table of the same rows.

</code_context>

<specifics>
## Specific Ideas

- The upstream freeze is the spec: `.planning/MILESTONE-CONTEXT.md` §3 (D-46, D-50), §4 (verification strategy) and `.planning/research/v0.3.0-scoping-3-rights-format-packaging.md` R-1/R-2 are cited by row, not re-argued.
- GUARDRAILS-v0.3 V1, V2, V4, V11 are the same-commit retarget table for this phase; a count edited in a later "re-bless" commit is a defect.
- The first real-content run (MILESTONE-CONTEXT §1a) is why the ceiling lands *before* any claim exists in this milestone — it mints no claim and its observation stays `unknown-default-chosen`.

</specifics>

<deferred>
## Deferred Ideas

- A `refs/`-backed content inventory and any real EXEC/GROM/ROM digest: owner-local, never committed, never a row's evidence (D11 handoff).
- `.cfg` parsing and bank-mode implementation: FMT-01/FMT-04 stay `Deferred` with their written reversal conditions; Phase 13 only records the refusal.
- A libretro header vendored under `third_party/`: refused by D-48 unless a real frontend needs it, at which point its licence text is read and recorded first.
- The debugger/inspection surface (TOOL-02): rolling-roadmap mid horizon, not this milestone.

</deferred>
