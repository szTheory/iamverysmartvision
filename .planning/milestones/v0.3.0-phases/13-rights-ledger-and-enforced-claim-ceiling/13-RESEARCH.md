# Phase 13: Rights Ledger and Enforced Claim Ceiling — Research

**Researched:** 2026-11-26 (delta pass after the four open questions were resolved in `13-CONTEXT.md`)
**Domain:** The project's own gate machinery (POSIX `sh` steps + CMake `-P` checkers + inventory `.ids` files + negative-control trees). No third-party library, package or vendored content is involved.
**Confidence:** HIGH for every statement about existing machinery (each was read and, where a count is quoted, executed this session); HIGH for the mechanical consequences of the five resolutions (each is now a file-and-line change quoted verbatim); MEDIUM only for the free choices flagged in the Assumptions Log (gate names, the one new `SRC-` record's URL and authority tag).

## Summary

Phase 13 adds no new mechanism *kind*. Every limb it needs already exists in the gate and is exercised today by at least one of the 395 committed negative controls: a JSON-record checker driven by a schema file (`records.cmake` + `schema/<kind>.txt` + a hook in `kinds/`), a set-equality inventory check (`read_ids` + `<kind>-id-missing` / `-id-extra`), a whole-token scan with an anchored character class (`steps/sources.sh`), a `(token, detail)` pin list (`tools/release-readiness.sh`'s `history_compare`), and an `EXPECT`/`FORBID` control directory whose overlay is a whole-tree copy. Phase 13's three deliverables are therefore *data plus ids plus controls*, not code architecture: `data/rights/*.json` + `data/expected/rights.ids` + `data/expected/rights-pinned.ids`, two rules inside `tools/check/kinds/claim.cmake`, and ten `important-gates.ids` rows.

Four measured facts dominate the planning:

1. **The ten D-09-06 mechanisms do not all resolve the same way, so the registration is 5 `control` + 4 `mutant` + 1 new `control` directory.** I mirrored the resolution loop (`tools/check/steps/important-gates.sh:130-147`) over the live inventories: `important-gates-complete-uncovered`, `important-gates-excluded-malformed`, `sources-src-token-whole`, `manifests-output-anchor` and `memmap-generator-refused` resolve as `control`; the other five (`important-gates-excluded-stale`, `control-toy-drift`, `control-deviation-unused`, `control-pass-line-not-last`, `memmap-generator-missing`) resolve to **nothing**. Four of those five are already proved by `tools/check/steps/runner-selftest.sh` (which has **no inventory file**), and one (`important-gates-excluded-stale`) is proved only as a co-firing `EXPECT` line inside the `important-gates-complete-uncovered` control. CONTEXT resolves this: 5 `control` rows (zero cost), 4 `mutant` rows through a new `data/expected/runner-selftest.ids` read by the existing loop, and 1 new `control` directory for `important-gates-excluded-stale` (0.31 cpu-s). The exact edits are in **Resolved Decisions → R1**.
2. **A registry row's `<REQ>` column must name a requirement that is `Complete` and not excluded.** `important-gates.sh:172-174` fails a row whose 4th column is outside the derived accepted set. `RTS-01`, `RTS-02`, `EV-06` and `EV-07` are all `Pending` (`.planning/REQUIREMENTS.md:104-107`), so **the rows and the requirement-completion edits must land in the same commit** — exactly what `16afb31` (TOOL-01) and `b14e1a4` (PKG-05/TOOL-04/EV-05) did.
3. **The cost ceiling bounds the number of new control directories to six.** A control is priced at 0.31 hosted cpu-s (`MILESTONE-CONTEXT.md:227-228`); the phase's priced addition is 2.0. Six controls = 1.86, plus ten registry rows at 0.005 = 0.05, total **1.91 of 2.0**. Seven controls overruns the line. The phase's limbs must therefore be folded into six directories: RTS-01 → 1, RTS-02 → 1, EV-06 → 3, EV-07 → 1.
4. **The refused-token set cannot contain the hardware-concept words.** Measured over the four naming scopes, `exec` occurs in 98 files, `rom` in 182, `grom` in 29, `cfg` in 9. The tokens must be *artifact* spellings (`grom-image`, `exec-image`, `save-state`, `libretro`, …) and the match must be character-bounded, or the naming limb refuses the entire core.

**Primary recommendation:** implement the ledger as a fourth `records.cmake` kind (`kind=rights`, `schema/rights.txt`, `kinds/rights.cmake`) so the schema/enum/hook machinery is reused verbatim and `read_ids` gives three tokens for free; put the *naming* scan and the pin reverse direction in `tools/check/files.cmake` (it is the only checker that already holds the sorted `FILELIST`); register the ten mechanisms exactly as CONTEXT resolved (5 `control` + 4 `mutant` via the new `runner-selftest.ids` + 1 new `control`), for 1.91 cpu-s of new cost; make `libretro-abi` the one live `cleared` row by adding one `reference-only` source record; and close all four requirements in the same commits as their rows, re-deriving `gates=`/`lines=` by running the step rather than quoting a number.

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

**Rights ledger shape and enforcement**
- One file per item at `data/rights/<slug>.json` with keys `id`, `item`, `disposition` ∈ {`cleared`, `refused`, `owner-local-only`}, `source` (a `SRC-` id or `null`), `taken` (`"nothing"` legal), `tokens`, `note` — frozen upstream by D-46 and scoping R-1. — **Reversibility:** one-way — the ledger is a persisted data schema that every later phase's rows, the `docs/RIGHTS.md` table and the `rights.ids` membership key off; changing a key later is a coordinated rewrite of rows, ids and the doc table.
- Enforcement is ids inside the existing `manifests` and `files` steps. `data/expected/gate-steps/rights.ids` already names both; no new step is added and `ls tools/check/steps/*.sh | wc -l` stays 38.
- The naming limb is **token + scope, never a global grep**. `cfg` and `ivsc` already exist in `tools/check/binary-ext.txt` and `.gitignore` as *refusal surfaces*; removing them would delete a refusal. Pre-existing hits are pinned by `(token, path)` in `data/expected/rights-pinned.ids`, mirroring `hygiene-history-pinned.ids`.
- Row set for this phase, one row per RTS-01 question: `flat-cartridge-image`, `cfg-container`, `bank-modes`, `exec-image`, `grom-image`, `commercial-rom`, `rendered-frame`, `save-state`, `libretro-abi`, `core-info-strings`.
- The ten row spellings are the operator-facing nouns an item must never be named as. A row's `tokens` are data, and a refused row's tokens are refused only inside the four scopes above.
- `cleared` resolvability is checked against `data/sources.json` membership only. The `rights` field on a source record describes the *reference document's* own rights (it is `unknown` for every source today) and answers a different question; consulting it would make every `cleared` row impossible and would conflate two ledgers. — **Reversibility:** costly — the check is one id, but a later change re-opens every `cleared` row and its control overlay.
- "Operator pages of `docs/`" is an explicit pinned list in the check's data, not a `docs/**` glob: the pages that hand an operator a command or a file (`README.md`, `docs/running.md`, `docs/owner-images.md`, `docs/packaging.md`, `docs/diagnostics.md`, `docs/RIGHTS.md`). Adding a page is a deliberate act with its own diff.
- The ledger is **not** a second manifest schema. A question whose answer is a byte set becomes a fixture-manifest entry; the ledger answers "may this be named / read / shipped", the manifest answers "is this the byte set". Recorded as the reversal condition.

**Enforced claim ceiling (D-32 / D-50)**
- The check lives in `tools/check/kinds/claim.cmake`, which already judges each claim row, and lands as ids in the `claims` step. `data/expected/gate-steps/claims.ids` already names `claims` and `text`.
- Rule: an `established` standing requires `tier == source-expectation` **and** every id in `depends_on` resolving to a profile field tagged `tier1-primary-doc`. Anything else is refused by name.
- The positive case is a **toy** legal `established` claim in the checker's own fixture set, not a mutation of `CL-0002`. `CL-0002` stays the only live established claim and is asserted, not edited, so the live ledger and its `claims.ids` baseline do not move for a test.
- Two must-fail controls and one legal positive, per GUARDRAILS V4: `claim-established-non-source`, `claim-established-without-tier1`, `claim-established-positive`.

**The ten D-09-06 registrations (EV-07)**
- Each of the ten mechanisms gains an `important-gates.ids` **`mutant` row over its existing selftest**, never a fault. The canonical list is `09-05-SUMMARY.md`: `important-gates-complete-uncovered`, `important-gates-excluded-stale`, `important-gates-excluded-malformed`, `control-toy-drift`, `control-deviation-unused`, `control-pass-line-not-last`, `sources-src-token-whole`, `manifests-output-anchor`, `memmap-generator-refused`, `memmap-generator-missing`.
- The registry baseline (`lines=` in the header, and any excluded-count baseline) is re-derived in the same commit as the ten rows. The count edit is part of that commit, never a later "re-bless" commit (GUARDRAILS V7, V11).
- All ten toy registries gain exactly one line each with zero deletions, and `faults` stays 238. Ten faults instead of mutants would cost about 4.4 hosted cpu-s — 82 % of the milestone's allowance — on the step already 29 % of the gate.

**Gate cost and evidence discipline**
- Priced addition for this phase: **2.0 hosted cpu-s** against a 21 cpu-s allowance; the phase-boundary checkpoint recomputes the last-five `cpu_s`/`gate_s` medians from `.planning/ci-runs.tsv` by arithmetic and records exactly one of `discharged-within-allowance` / `reprice` / `lever-required`.
- Every new check ships three things in one commit (GUARDRAILS Architecture Patterns → Pattern 1.4): the must-fail mechanism registered in `data/expected/important-gates.ids` under its requirement, a liveness guard, and a committed scratch stand-in showing the **guard** fail.
- No case may assert only `exit 0` / `outcome pass` (GUARDRAILS Architecture Patterns → Pattern 1.5): each asserts an observed domain, a witness pair or a count.
- Retargets are same-commit per GUARDRAILS V1 (rights row → ledger file, `rights.ids`, the `docs/RIGHTS.md` row, the reading id), V2 (a refused spelling → tokens, pinned ids, control overlay), V4 (an `established` standing → the mechanism, two must-fail controls, one positive), V11 (a D-09-06 mechanism → a `mutant` row).

**Decided autonomously**
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

### Deferred Ideas (OUT OF SCOPE)

- A `refs/`-backed content inventory and any real EXEC/GROM/ROM digest: owner-local, never committed, never a row's evidence (D11 handoff).
- `.cfg` parsing and bank-mode implementation: FMT-01/FMT-04 stay `Deferred` with their written reversal conditions; Phase 13 only records the refusal.
- A libretro header vendored under `third_party/`: refused by D-48 unless a real frontend needs it, at which point its licence text is read and recorded first.
- The debugger/inspection surface (TOOL-02): rolling-roadmap mid horizon, not this milestone.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| RTS-01 | A rights ledger records one row per third-party-content question; each has a disposition of `cleared`/`refused`/`owner-local-only`, a `cleared` row cites a resolvable `SRC-` id, and a row missing its disposition, a source it needs, or its "what was taken" cell is refused by name. | Architecture Patterns → Pattern 1 (ledger as a `records.cmake` kind), Pattern 2 (`read_ids` set equality), Architectural Responsibility Map (SRC resolution via `data/sources.json` membership), Resolved Decisions → **R2** (the one live `cleared` row and its `reference-only` record), "Phase 13 Concrete Values → The ten rows: disposition, tokens, and the pin census" (exact new tokens) |
| RTS-02 | A refused item's spellings cannot appear in `core/src`, `runner/`, `tools/` or the operator pages of `docs/`, except where a pre-existing refusal surface is pinned by a `(token, path)` pair; both limbs shown able to fail. | Architecture Patterns → Pattern 3 (the naming scan: character class, scopes, pin idiom with measured pre-existing hits), Pattern 4 (the pair grammar and the resolved reverse direction), Resolved Decisions → **R3** (`rights-pinned-stale` as a failure, with its exact insertion point in `files.cmake`), "Phase 13 Concrete Values → The ten rows…" (the 4 measured pins) |
| EV-06 | The claims step refuses an `established` claim that is not a `source-expectation` claim resting on a `tier1-primary-doc` dependency, and a legal `established` claim still passes — both shown able to fail (D-32, D-50). | Code Examples → "`claim.cmake` — the exact insertion point for D-50", Code Examples → "The toy tree already holds the legal positive", "Common Pitfalls → Pitfall 5" (why a "positive" cannot be a bare control directory) |
| EV-07 | Each of the ten D-09-06 mechanisms gains a `mutant` row over its existing selftest, with the registry baseline re-derived and all ten toy registries extended in the same commit, and zero new faults. | Resolved Decisions → **R1** (the 5 `control` + 4 `mutant` + 1 new `control` registration, with the loop edit and the `runner-selftest.sh` extension quoted verbatim), Resolved Decisions → **R4** (the 13-file toy edit), Resolved Decisions → **R5** (`gates=`/`lines=` re-derivation and the header table), Code Examples → "The ten mechanisms, resolved against the live inventories", "Common Pitfalls → Pitfall 1" (the requirement-column constraint and the same-commit precedent), "Phase 13 Concrete Values → The cost-checkpoint arithmetic" |
</phase_requirements>

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Ledger row grammar, disposition enum, `taken` non-blank | `manifests` step (CMake `-P`) | — | `manifests.cmake` already loads a JSON collection from `FILELIST`, enforces a key set with `check_shape`, and does set-equality against an inventory (`manifests.cmake:453-464`). A ledger is the same shape of problem. |
| `cleared` → `SRC-` resolvability | `manifests` step | `data/sources.json` as data | Resolution is membership in the source registry; `schema/source.txt:31` pins `rights` to `^unknown$`, so the `rights` field cannot answer it. The existing `sources` step also scans `data/rights/*.json` for `SRC-` tokens (`steps/sources.sh:30-33`), so every ledger id is resolved twice, independently. |
| Refused-token naming scan | `files` step (CMake `-P`) | — | `files.cmake` is the only checker that already holds the sorted tracked-path list (`files.cmake:89-99`) and the four scopes are path prefixes. |
| `(token, path)` pin resolution, both directions | `files` step | `data/expected/rights-pinned.ids` as data | Mirrors `tools/release-readiness.sh:47-84`, which reads its pin list as data and never widens a rule — with the reverse direction promoted from NOTE to failure (Resolved Decisions → R3). |
| Enforced claim ceiling | `claims` step | `tools/check/kinds/claim.cmake` hook | The hook already resolves `depends_on` to profile field tags (`claim.cmake:157-177`); D-50 is two more conditions in the same loop. |
| Ten mechanism registrations | `important-gates` step | `data/expected/important-gates.ids` + the new `data/expected/runner-selftest.ids` | The step already resolves `fault|control|mutant` against four inventories (`important-gates.sh:130-147`); the new file is one more path in the existing `mutant` loop, not a new kind. |
| Operator-facing rights table | `docs/RIGHTS.md` (documentation) | `text` step's claim-word tripwire | The page is prose, so it is governed by `steps/text.sh:4-12,27,44-48`, not by a checker of its own. |
| Cost checkpoint | `.planning/ci-runs.tsv` (data) | `ci-log` step's format check | Arithmetic over a committed file; `steps/ci-log.sh:7-14` is the only gate over its shape. |

## Standard Stack

There is **no package installation in this phase.** No `npm`/`pip`/`cargo`/`FetchContent`/vendored header. Everything used already exists in the tree. The `## Package Legitimacy Audit` section is therefore omitted by its own trigger condition ("whenever this phase installs external packages") — verified: `data/rights/*` is JSON, the checkers are CMake `-P` scripts, the controls are directories, and D-48 forbids vendoring a libretro header.

### Core (already present, reused verbatim)

| Asset | Location | Purpose in this phase | Why it is the right one |
|-------|----------|----------------------|-------------------------|
| `lib.cmake` | `tools/check/lib.cmake:1-152` | `gate_fail` (`:8-11`), `gate_stat` (`:13-16`), `gate_finish` (`:35-44`), `json_type` (`:64-71`), `read_ids` (`:75-116`) | Every checker's failure/pass-line contract. `read_ids` gives `rights-inventory-missing`, `-empty`, `-duplicate` for free. |
| `manifest-lib.cmake` | `tools/check/manifest-lib.cmake:1-216` | `manifest_in_filelist` (`:18-26`) for "is this path tracked" | CONTEXT's locked choice; it is the only shared helper that answers a `FILELIST` membership question. |
| `records.cmake` + `schema/<kind>.txt` + `kinds/<kind>.cmake` | `tools/check/records.cmake:1-…` | The ledger can be a fourth record kind | The schema language already expresses `file`, `topkeys`, `array`, `idkey`, `inventory`, `keys`, `req`, `nonblank`, `match`, `enumfrom`, `when`, `hook` (`schema/claim.txt:1-28`). |
| `run-controls.sh` | `tools/check/run-controls.sh:1-377` | Declares and runs the new controls | `CHECK`/`EXPECT`/`FORBID`/`overlay/`/`MAKE`/`DEVIATION` (`:1-20`, `:198-252`); final line `controls=<ok>/<total>` (`:375`). EXPECT/FORBID are substring matches within one output line (`:237-244`). |
| `steps/important-gates.sh` | `tools/check/steps/important-gates.sh:1-196` | The ten rows' consumer | `fault|control|mutant` resolution (`:130-147`), the derived accepted set (`:93-131`) and the row judgement (`:160-190`); the `mutant` definition is documented at `:61-63`. |
| `steps/runner-selftest.sh` | `tools/check/steps/runner-selftest.sh:1-228` | The four `mutant` rows' proof | 13 cases, `cases=13` measured; three of the four EV-07 case names exist verbatim in it (Resolved Decisions → R1). |
| `steps/sources.sh` + `kinds/source.cmake` | `tools/check/steps/sources.sh:1-57`, `kinds/source.cmake:1-95` | The model for a whole-token scan, and the `reference-only` branch R2 uses | Its measured character class and boundary reasoning (`sources.sh:36-51`) is the exact pitfall the naming scan must avoid; `kinds/source.cmake:63-83` is the `reference-only` rule R2 must satisfy. |
| `tools/gate.sh` | `tools/gate.sh:1-250` | Step inventory and summary line | `steps.have`/`steps.want` (`:97-115`), summary (`:241-249`). |

### Alternatives Considered

| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| A fourth `records.cmake` kind for the ledger | A bespoke `rights.cmake` with hand-written key checks | Bespoke duplicates `check_shape`/`enumfrom`/`nonblank` and re-introduces the "misspelled key is not a missing key" bug that `manifests.cmake`'s fail-closed shape check exists to prevent. Only worth it if the ledger needs a *cross-file* rule the schema language cannot express (e.g. "no two rows share a token"), which the `hook` function can express anyway. |
| Naming scan in `files.cmake` | A new `rights.cmake` invoked from `steps/files.sh` | Both are legal (neither adds a gate step). A separate `.cmake` keeps `files.cmake` readable but needs its own `include(lib.cmake)` and its own pass-line aggregation; inlining reuses `files.cmake`'s single `gate_finish(files)` — and, decisively for R3, its already-loaded `FILELIST` and its existing `gate_stat` line. |
| `mutant` rows for all ten | 5 `control` rows where a control directory already exists, 4 `mutant` rows through a new inventory, 1 new `control` directory | **Resolved (CONTEXT).** A `control` row costs nothing and resolves today; a `mutant` row for the same mechanism would duplicate a proof that already exists. See Resolved Decisions → R1. |
| Hard-coding the four selftest case names in the registry's `known` set | A new `data/expected/runner-selftest.ids` inventory | **Resolved.** Hard-coding breaks the step's own documented invariant ("a `mutant` is a case of …", `important-gates.sh:60-63`) and would let the registry name a case the selftest no longer runs. The inventory is four lines and one extra path in the existing loop. |

**Installation:** none.

**Version verification:** not applicable — no external dependency. The tool versions that matter are already pinned by the environment: CMake ≥ 3.25 (`lib.cmake:6`), POSIX `sh`, and the project's own `tools/gate.sh`.

## Architecture Patterns

### System Architecture Diagram

```
                        ┌──────────────────────────────────────────┐
  data/rights/<slug>.json (10 rows)  ──┐                             │
  data/expected/rights.ids  ───────────┤  manifests step             │
  data/sources.json ───────────────────┘  (manifests.cmake)         │
        │  membership lookup                  │                     │
        │                                     ▼                     │
        │                          FAIL rights-<limb>: <id>         │
        │                                                           │
  data/expected/rights-pinned.ids ──┐                               │
  FILELIST (sorted tracked paths) ──┤  files step (files.cmake)     │
  data/rights/*.json → tokens ──────┘        │                       │
        │  scope filter: core/src/ runner/ tools/ + 6 pinned pages   │
        │  bounded token match, minus (token,path) pins              │
        │  then: every pin must have matched → rights-pinned-stale   │
        ▼                                                           │
  FAIL rights-token: <token> <path>                                 │
  FAIL rights-pinned-stale: <token> <path>                          │
                                                                    │
  data/claims.json ──► claims step ──► kinds/claim.cmake hook       │
  data/profiles/<p>.json (field tags)      │                        │
        │                                  ▼                        │
        │                     FAIL claim-established-non-source: <id>│
        │                     FAIL claim-established-without-tier1: <id>
        │                                                           │
  data/expected/important-gates.ids (337 lines) ──► important-gates │
        │   resolves each row against 4 inventories, one of them    │
        │   data/expected/runner-selftest.ids (new, 4 case names)    │
        ▼                                                           │
  gates=110 lines=337 ──► gate.sh summary token line ◄──────────────┘
                              "gate: PASS steps=38/38 controls=N/N …"
                                        │
                                        ▼
                        .planning/ci-runs.tsv  ──► cost checkpoint (arithmetic)
```

### Recommended Project Structure (new files only)

```
data/
├── rights/                          # 10 ledger rows, one file per RTS-01 question
│   ├── flat-cartridge-image.json
│   ├── cfg-container.json
│   ├── bank-modes.json
│   ├── exec-image.json
│   ├── grom-image.json
│   ├── commercial-rom.json
│   ├── rendered-frame.json
│   ├── save-state.json
│   ├── libretro-abi.json            # the one live `cleared` row (Resolved Decisions → R2)
│   └── core-info-strings.json
└── expected/
    ├── rights.ids                   # ledger membership (new)
    ├── rights-pinned.ids            # 4 (token, path) pins (new; Resolved Decisions → R3)
    └── runner-selftest.ids          # the 4 selftest case names (new; Resolved Decisions → R1)

tools/check/
├── schema/rights.txt                # only if the ledger is a records.cmake kind
├── kinds/rights.cmake               # ditto
└── controls/                        # exactly 6 new control dirs (cost bound, see below)
    ├── important-gates-excluded-stale/{CHECK,EXPECT,FORBID,overlay/data/expected/important-gates-excluded.ids}
    ├── claim-established-non-source/…
    ├── claim-established-without-tier1/…
    ├── claim-established-positive/…
    ├── rights-…/…                   # RTS-01: one dir, several EXPECT lines
    └── rights-…/…                   # RTS-02: one dir, both pin directions
```

`data/expected/controls/rights.ids` is **not** the inventory for `important-gates-excluded-stale`; that file lists the plan 01-06 rights controls (`manifests-*`, `files-*`). The new directory's id belongs in `data/expected/controls/important-gates.ids`, which owns every `important-gates` control (`:1-26`).

### Pattern 1: `records.cmake` kind + schema + hook

**What:** a JSON collection is checked by a declarative schema file plus an optional hook function; the inventory file gives set equality and three free tokens.
**When to use:** for the ledger row grammar. `schema/claim.txt` is the closest model (id regex, enum-from-another-file, `nonblank`, `when`); `schema/source.txt` is the model for a nullable key (`req sha256 STRING|NULL`, `:18`).
**Example:**
```
# Source: tools/check/schema/claim.txt:1-28 (verbatim shape, annotated)
file data/claims.json
topkeys schema_version note claims
array claims
idkey id
inventory data/expected/claims.ids
keys id statement profile tier artifact depends_on standing note
req id STRING
nonblank statement
match id ^CL-[0-9][0-9][0-9][0-9]$
enumfrom tier data/tiers.json evidence_tiers id
when standing=established nonblank depends_on
hook claim
```

### Pattern 2: set equality against an inventory, both directions

**What:** every id in the inventory must exist in the data and vice versa, each direction with its own token.
**When to use:** `data/expected/rights.ids` ↔ `data/rights/*.json`, and `data/expected/sources.ids` ↔ `data/sources.json` (which is why R2's new source row must land in both files).
**Example:**
```cmake
# Source: tools/check/manifests.cmake:453-464 (verbatim)
read_ids(want "data/expected/fixtures.ids" manifest)
foreach(i IN LISTS want)
  if(NOT "${i}" IN_LIST uniq)
    gate_fail(manifest-id-missing "${i}")
  endif()
endforeach()
foreach(i IN LISTS uniq)
  if(NOT "${i}" IN_LIST want)
    gate_fail(manifest-id-extra "${i}")
  endif()
endforeach()
```
Note the token is assembled as `<kind>-id-missing`; `read_ids`'s own tokens are `<kind>-inventory-missing|empty|duplicate` (`lib.cmake:79`, `:104`, `:112-114`).

### Pattern 3: whole-token scan with an explicit boundary class

**What:** extract a token with `grep -o -E` over a character class that excludes the separator characters of neighbouring words, then compare against an inventory.
**When to use:** the refused-token naming scan.
**Example:**
```sh
# Source: tools/check/steps/sources.sh:44-51 (verbatim, with its measured rationale at :36-43)
grep -a -o -E 'SRC-[A-Za-z0-9_-]+' "$ROOT/$f" >> "$tmp/refs"
...
sort -u "$tmp/refs" | while IFS= read -r id; do
  grep -q -x -F -e "$id" "$tmp/ids" || printf -- '-- FAIL source-unknown-ref: %s\n' "$id"
done
```
The `+` quantifier and the absence of `.` from the class are both load-bearing and both were *measured*, not chosen by taste (`sources.sh:36-43`). The naming scan needs the same discipline in reverse: the character before and after the token must not be in `[A-Za-z0-9_-]`, or `rom-image` fires inside `grom-image` (measured: `grep -rilF rom-image` returns the two files that actually contain `grom-image`).

### Pattern 4: `(token, detail)` pin list, never a token alone — and the reverse direction is a FAILURE

**What:** a finding is paired with its detail and compared as a 2-field key; an unpinned pair is new. In `history_compare` a pinned pair that no longer occurs is a **NOTE**; for rights it must be a **failure** (`rights-pinned-stale`), because a rights pin names a path that can be deleted while a history pin names a commit that cannot.
**When to use:** `data/expected/rights-pinned.ids`.
**Example:**
```awk
# Source: tools/release-readiness.sh:52-83 (verbatim excerpt)
if (split(l, w, " ") == 2) { key = w[1] " " w[2]; if (!(key in pinned)) { pinned[key] = 1; pord[++np] = key } }
...
i = index(line, ": "); if (i == 0) next
key = substr(line, 1, i - 1) " " substr(line, i + 2)
...
if (order[j] in pinned) { printf "FAIL (accepted by recorded disposition, D-33): %s\n", order[j]; acc++ }
else { printf "NEW FINDING: %s\n", order[j]; bad++ }
```
`split(l, w, " ") == 2` is the grammar: exactly two whitespace-separated fields, so a pin line cannot rot into a wildcard. Line `:80` is the NOTE the rights version replaces with `gate_fail(rights-pinned-stale …)` — see Resolved Decisions → R3 for the exact CMake shape and insertion point.

### Pattern 5: `EXPECT`/`FORBID` control with a whole-tree overlay

**What:** a control dir names a step (`CHECK`), one or more mandatory output substrings (`EXPECT`), optional forbidden substrings (`FORBID`), and a partial tree copied over a pristine `_good/tree` (`overlay/`).
**When to use:** every new must-fail limb.
**Example:**
```
# Source: tools/check/controls/important-gates-complete-uncovered/{CHECK,EXPECT,FORBID} (verbatim)
CHECK:  important-gates
EXPECT: FAIL important-gates-complete-uncovered: FND-99
        FAIL important-gates-excluded-stale: EV-99
FORBID: important-gates-unknown-id
```
Two `EXPECT` tokens in one control are legal and normal; `FORBID` is how a control says "this other refusal must *not* be how it failed" (`run-controls.sh:222-249`). This control is also the reason `important-gates-excluded-stale` is **not** resolvable as its own id: the mechanism fires here, but the control's directory is named after the other mechanism. Resolved Decisions → R1 gives the directory that gives it its own name.

### Pattern 6: an in-step selftest that reads its own case inventory

**What:** the step reads `data/expected/<suite>-selftests.ids`, runs exactly those cases, and fails on a name it does not implement.
**When to use:** R1's `runner-selftest.ids`.
**Example:**
```sh
# Source: tools/check/steps/pkg-install.sh:832-851 (verbatim)
sinv="$ROOT/data/expected/pkg-selftests.ids"
st_ok=0
st_total=0
if [ -f "$sinv" ]; then
  awk '/^#/ { next } NF { print $1 }' "$sinv" > "$T/st.cases"
  [ -s "$T/st.cases" ] || bad pkg-selftest-inventory "no cases"
  while IFS= read -r c; do
    st_total=$((st_total + 1))
    cst=0
    run_case "$c" < /dev/null || cst=$?
    case $cst in
      0) st_ok=$((st_ok + 1)) ;;
      2) bad pkg-selftest-missing "$c" ;;
      *) bad "pkg-selftest-$c" "$detail" ;;
    esac
  done < "$T/st.cases"
else
  bad pkg-selftest-inventory "data/expected/pkg-selftests.ids"
fi
```
The two load-bearing halves are the `*) return 2 ;;` sentinel (an unknown case name is `-missing`, not a case failure) and the `[ -s ]`/`else` guard (an empty or absent inventory is its own token, never a silent pass). `data/expected/pkg-selftests.ids:1-5` states the invariant in words: "the step runs exactly these and fails on a name it does not know (pkg-selftest-missing)".

### Anti-Patterns to Avoid

- **A `docs/**` glob for the naming scope.** The CONTEXT pinned six pages deliberately. A glob would make the operator pages and `docs/provenance/`, `docs/stic/`, `docs/video/` (reference notes, which legitimately quote sources) indistinguishable, and every new doc file would silently widen the rule.
- **A token-only pin list.** `hygiene-history-pinned.ids` is a pair list because a token alone matches every instance; the same failure mode applies to `rights-pinned.ids`.
- **A global `grep` for `cfg`/`ivsc`.** Both are *refusal surfaces* in `tools/check/binary-ext.txt:8,5` and `.gitignore:7,9`; removing either would delete a refusal. Measured: `cfg` occurs in 9 files across the four scopes.
- **Bare `exec`, `rom`, `grom` as refused tokens.** Measured file counts in the four scopes: `exec` 98, `rom` 182, `grom` 29. They are hardware/API vocabulary (`ivs_exec`, `memsys_grom_read`, …), not artifact names.
- **Editing `CL-0002` to test the ceiling.** CONTEXT locks the positive to a toy row; `CL-0002` is asserted unchanged so `data/claims.json` and `data/expected/claims.ids` do not move for a test.
- **Quoting `lines=337` / `gates=110` as the post-phase figure.** Both are runtime stats (`important-gates.sh:188`). Re-derive by running the step; the numbers in `09-05-SUMMARY.md` and `MILESTONE-CONTEXT.md` are already stale in places (see "Phase 13 Concrete Values → The cost-checkpoint arithmetic").
- **A `mutant` row for a mechanism that already has a control directory.** That is a second registration of one proof; CONTEXT resolved the split explicitly (Resolved Decisions → R1).

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| JSON key presence/type/enum checking | A hand-written key loop in a new `rights.cmake` | `records.cmake` + `schema/rights.txt` (`check_shape` analogue at `manifests.cmake:41-66`) | The existing shape check is fail-closed on a *misspelled* key (it is an unknown key, not a missing one) — a bespoke loop gets this wrong once and the ledger silently accepts `dispositon`. |
| Inventory membership + duplicates | `sort | uniq -d` in `sh` | `read_ids` (`lib.cmake:75-116`) | It already fails on missing file, empty file (unless `#allow-empty:`), and duplicates, with the token names `<kind>-inventory-*`. |
| A pass line | `printf` from the step | `gate_finish` (`lib.cmake:35-44`) | It collects every `gate_fail` and emits exactly one `FATAL_ERROR`, which is what `pass-line.sh` and `run-controls.sh:219-221` require. |
| Negative-control plumbing | A bespoke test script | `run-controls.sh` + a control dir | `CHECK`/`EXPECT`/`FORBID`/`overlay`/`MAKE` plus the inventory both-directions check (`:290-298`) and the toy-registry drift check (`:300-326`). |
| Token extraction with boundaries | `grep -w` | The measured `grep -o -E '[class]+'` idiom (`sources.sh:44-48`) | `grep -w` treats `-` as a word separator, so `save-state` would match inside `save state` and `rom-image` would match inside `grom-image`; the class must include `-` on both sides. |
| An in-step selftest case list | A hard-coded `for c in a b c` | `data/expected/<suite>-selftests.ids` + the `run_case`/`return 2` idiom (`pkg-install.sh:832-851`) | The inventory is what makes "the step runs exactly these" checkable, and an absent file is its own token rather than a silent zero-case pass. |
| Cost arithmetic | A spreadsheet or a quoted median | `python3`/`awk` over `.planning/ci-runs.tsv` | AGENTS.md: "Recompute, never quote." "Phase 13 Concrete Values → The cost-checkpoint arithmetic" shows the frozen figure is already 5 cpu-s off the current file. |

**Key insight:** every limb of this phase is a *data-plus-ids* problem whose machinery already exists and is already covered by 395 controls. The only genuinely new code is the bounded-token scan, the pin reverse direction, and two conditions in `claim.cmake`; everything else is a schema file, a `.ids` file, a JSON row, or a control directory.

## Runtime State Inventory

Phase 13 is not a rename, rebrand or migration; it introduces new data and new ids. The inventory below answers the canonical question anyway, because the phase does introduce a persisted schema and a naming rule that later phases key off.

| Category | Items Found | Action Required |
|----------|-------------|------------------|
| Stored data | None — `data/rights/` does not exist yet (verified `ls -d data/rights` → not found). `data/claims.json` holds 28 claims, of which 1 is `established` (measured: `claims pass claims=28 provisional=27`). | Create `data/rights/*.json`; no migration. |
| Live service config | None — no external service, no hosted state. The only hosted artifact is `.planning/ci-runs.tsv`, which is append-only and already committed (96 data rows). | Append one row after the content lands (success criterion 5). |
| OS-registered state | None — no scheduler, launchd, systemd or pm2 unit references the gate. | None. |
| Secrets / env vars | None added. The gate reads `ROOT`, `CHECK_DIR`, `FILELIST`, `IVS_GATE_DIR`, `CONTROLS_DIR`, `CONTROLS_INVENTORY_DIR`; all are set by `tools/gate.sh` or `run-controls.sh` and none changes. | None. |
| Build artifacts / installed packages | `build/` is gitignored and regenerated. No installed package changes. | None. |
| Control trees (the real "runtime state" here) | `tools/check/controls/_good/tree/` (124 files) is a *committed* tree that every control copies. It has no `data/rights/` and none of the six pinned operator pages. Ten whole-file toy registries exist and all ten end with the same line `toy-tool01 fault GOOD_FAULT TOOL-01` (measured); `tools/check/controls/important-gates-complete-uncovered/overlay/.planning/REQUIREMENTS.md` is a whole-file copy of the toy requirement record (`diff` against `_good` → one line, `FND-99`) and must be updated whenever the toy record gains a row. | Add the toy ledger + inventories + a toy operator page (or make the page list tolerate absence); update the overlay copy in the same commit as any toy-record row. |

**Nothing found in the first five categories** — verified by the commands above and by grepping the gate for `IVS_`/`ROOT`/`CHECK_DIR` usage.

## Common Pitfalls

### Pitfall 1: The registry row's requirement column is not free text
**What goes wrong:** a row `rights-ledger control rights-… RTS-01` is written while `RTS-01` is still `Pending`. The step fails `important-gates-unknown-id: line <n>` and the gate goes red on the commit that was supposed to add coverage.
**Why it happens:** `important-gates.sh:173-175` checks `!($4 in accepted)` where `accepted = Complete − excluded`, derived live from `.planning/REQUIREMENTS.md` and `.planning/milestones/*-REQUIREMENTS.md` (`:93-131`). `RTS-01/02`, `EV-06/07` are `Pending` today (`.planning/REQUIREMENTS.md:104-107`).
**How to avoid:** flip the requirement's checkbox line *and* its traceability row to `Complete` in the same commit as the rows that name it — the `16afb31` (TOOL-01) and `b14e1a4` (PKG-05/TOOL-04/EV-05) precedent. `16afb31` is the exact template: `16 files changed, 32 insertions(+), 2 deletions(-)`, where the 2 deletions are the two requirement-status flips and the 13 toy lines are the rest.
**Warning signs:** `important-gates-unknown-id: line <n>` on an otherwise-correct row; or `important-gates-uncovered-gate: <REQ>` after a requirement is flipped without a row.

### Pitfall 2: The token match must be character-bounded
**What goes wrong:** `rom-image` is refused inside `grom-image`, `exec` inside `ivs_exec`, `cfg` inside `cfgfile`. The control that plants one token cannot be satisfied because the pristine tree already fails.
**Why it happens:** substring matching over code identifiers and hyphenated names.
**How to avoid:** require the character before and after the match to be outside `[A-Za-z0-9_-]` (line start/end count as boundaries), exactly the class `sources.sh:44` uses. Verified: with that class, `rom-image` does **not** match inside `grom-image` (the preceding char is `g`).
**Warning signs:** a `rights-token` finding on a file nobody edited; a pin list with hundreds of entries.

### Pitfall 3: `docs/RIGHTS.md` is inside its own scan scope
**What goes wrong:** the page must list the ten items, and the items' refused spellings then appear in a pinned operator page, so the page pins itself ten times — and the pin list stops being a record of *pre-existing accidents* and becomes a copy of the ledger.
**Why it happens:** `docs/RIGHTS.md` is both a scope and the documentation surface (CONTEXT's pinned page list includes it).
**How to avoid:** keep `item` (the human phrase, e.g. "save state") distinct from `tokens` (the forbidden spellings, e.g. `save-state`). With the bounded class of Pitfall 2, `save state` does not match `save-state`. Where the page must spell a token to explain the rule, that is a deliberate `(token, docs/RIGHTS.md)` pin with its own diff.
**Warning signs:** ten `(token, docs/RIGHTS.md)` pins in the first draft of `rights-pinned.ids`.

### Pitfall 4: `docs/RIGHTS.md` prose is also scanned by the `text` step
**What goes wrong:** a table cell or sentence containing `supports`, `compatible`, `passes`, `accurate`, `verified`, `runs`, `plays`, `playable`, `looks` or `sounds` fails `text-untagged-claim-word` unless the line carries `[claim:CL-NNNN]` or `[nc]`.
**Why it happens:** `steps/text.sh:27` selects `README.md`, `CHANGELOG.md` and `docs/**/*.md` from `FILELIST` and `:47-48` scans them with `grep -w -i`, stripping `[claim:CL-NNNN]`/`[nc]` first.
**How to avoid:** phrase the rights rows as dispositions ("may be named", "may not be committed"), never as behaviour claims; add `[nc]` where a claim word is unavoidable. The `playable-guest-progress` identifier is the only exempted string.
**Warning signs:** `text-claim-untagged: docs/RIGHTS.md:<line>`.

### Pitfall 5: A control directory can never express "this passes"
**What goes wrong:** `claim-established-positive` is created as a control whose overlay adds a legal established claim, expecting the step to pass. `run-controls.sh:219-221` fails it as `control-exit-zero`.
**Why it happens:** `ctl_one` requires a non-zero step exit by construction; only `good_one` (`:175-196`) runs a pristine pass.
**How to avoid:** express the positive as `EXPECT <some other refusal> + FORBID claim-established-non-source claim-established-without-tier1`, and rely on the pristine `_good/claims` run (already produced because `claims` is an existing `CHECK`) for the plain pass. Concretely: `claim-established-positive`'s overlay keeps the toy legal claim `CL-0001` intact **and** plants a second, unrelated claim fault, so the step exits non-zero for a different reason while the legal row survives — the two new tokens being absent is the positive.
**Warning signs:** `control-exit-zero: claim-established-positive`.

### Pitfall 6: New `.cmake`/`.sh` under `tools/` needs the SPDX line; new JSON under `data/` must be ASCII
**What goes wrong:** a new `tools/check/rights.cmake` without `# SPDX-License-Identifier: MIT` fails `integrity-missing-spdx`; a non-ASCII character (an em dash, a curly quote) in a `data/rights/*.json` fails `integrity-non-ascii`.
**Why it happens:** `tools/check/integrity.cmake:60-79`.
**How to avoid:** copy the two-line header from any existing checker; keep ledger `note`/`item`/`taken` values plain ASCII.
**Warning signs:** `integrity-missing-spdx: tools/check/<new>.cmake`, `integrity-non-ascii: data/rights/<slug>.json`.

### Pitfall 7: A toy registry overlay replaces the whole file, so drift is checked against `_good`
**What goes wrong:** a toy registry line is added to `_good/tree/...` but not to all nine overlays (or vice versa). `run-controls.sh:300-326` reports `control-toy-drift` (undeclared difference) or `control-deviation-unused` (a DEVIATION line matching nothing).
**Why it happens:** each of the nine overlays is a *whole-file copy*; the comparison excludes comments only (`:306-321`).
**How to avoid:** one line appended to all ten files in the same commit, zero deletions (CONTEXT), and no `DEVIATION` entry — a DEVIATION entry would be the wrong tool because the addition is intended, not a control-local difference. Measured: all nine overlays differ from `_good` by exactly one declared line today, and all ten files end with the same line, so appending the same line to all ten leaves every existing difference unchanged.
**Warning signs:** `control-toy-drift: <control>: > toy-…` or `control-deviation-unused`.

### Pitfall 8: The summary token line moves, and the `files` `tracked=` count moves too
**What goes wrong:** the predicted token delta is stated as "none", then the measured summary line differs — because `files` gained a `gate_stat`, or simply because the phase added tracked files and `files` counts them.
**Why it happens:** `tools/gate.sh:241-249` concatenates every `-- <step> pass <stats>` payload into the summary token line; `files.cmake:173-174` emits `tracked=<n> binaries-listed=<k>` where `<n>` is `list(LENGTH paths)` over `FILELIST`, i.e. `git ls-files` (`files.cmake:89-99`).
**How to avoid:** predict both halves explicitly. Measured baselines this session (`git ls-files` → 2664 paths):

```
-- files pass tracked=2664 binaries-listed=0
-- manifests pass manifests=11 effective-yes=8 effective-no=3
-- claims pass claims=28 provisional=27
-- important-gates pass gates=110 lines=337
-- sources pass sources=12 refs-verified=3/4
-- runner-selftest pass cases=13
```

The new files this phase adds are all tracked, so `tracked=` becomes `2664 + <new tracked files>`; count them at implementation time and write the number into the plan's prediction rather than the word "unchanged". The earlier draft of this research quoted `tracked=2662`, which was already 2 stale — an instance of the very defect this pitfall names.
**Warning signs:** a cost-checkpoint row whose token line does not match the prediction in the plan.

### Pitfall 9: The `sources` stat has two halves and only one of them moves
**What goes wrong:** R2 adds a source record, so the plan predicts `sources=13 refs-verified=4/5` (inventing a verified hash that does not exist).
**Why it happens:** `kinds/source.cmake:22-23` increments `source-hashed` only in the `hashed` branch, and `:57-58` increments `source-verified` only when a cached file under `refs/` is present and matches. A `reference-only` record increments neither.
**How to avoid:** predict `sources=13 refs-verified=3/4` — the first number is `source-total` (`:12`), the second is `verified/hashed` (`:87-93`). Nothing about a `reference-only` row can move the denominator.
**Warning signs:** a predicted `refs-verified` denominator of 5.

## Code Examples

### Declaring and running an id: the full path from data to gate

```sh
# Source: tools/gate.sh:97-115 (verbatim) — step scripts must equal the union of the gate-steps inventories
: > "$GATE_TMP/steps.have"
for s in "$CHECK_DIR"/steps/*.sh; do
  [ -f "$s" ] || continue
  basename "$s" .sh >> "$GATE_TMP/steps.have"
done
sort -u "$GATE_TMP/steps.have" -o "$GATE_TMP/steps.have"
: > "$GATE_TMP/steps.want"
for f in "$ROOT"/data/expected/gate-steps/*.ids; do
  [ -f "$f" ] || continue
  awk '/^#/ { next } NF { print $1 }' "$f" >> "$GATE_TMP/steps.want"
done
```
`data/expected/gate-steps/rights.ids` already lists `manifests` and `files` (3 lines, one comment); `data/expected/gate-steps/claims.ids` already lists `claims` and `text`. **No edit to either is needed** — this is why CONTEXT says the phase adds ids, not steps. `steps=` stays 38 (measured: `ls tools/check/steps/*.sh | wc -l` → 38).

### The summary line and where each token comes from

```sh
# Source: tools/gate.sh:241-249 (verbatim)
command times > "$GATE_TMP/times.out"
sh "$CHECK_DIR/ci-summary.sh" --cpu < "$GATE_TMP/times.out" >> "$GATE_TMP/measure.lines"
cat "$GATE_TMP/measure.lines"

status=PASS
[ "$fail" -eq 0 ] || status=FAIL
tokens=${tokens% }
printf 'gate-time total=%s\n' $(($(date +%s) - gate_t0)) | tee -a "$GATE_TMP/measure.lines"
printf 'gate: %s steps=%s/%s controls=%s/%s %s\n' "$status" "$passed" "$want" "$ctl_ok" "$ctl_total" "$tokens" > "$GATE_TMP/summary.line"
cat "$GATE_TMP/summary.line"
```
So `steps=38/38` is `passed/want`, `controls=N/N` is `run-controls.sh`'s last line (`:375`), and `faults=238`/`goldens=29` are the `-- faults-verdict pass faults=<ok>/<total>` and `-- golden pass goldens=<n> …` payloads. Measured invariants this session: `faults` = 238 non-comment lines across `data/expected/faults/*.ids`; `goldens` = 29 tracked files directly under `data/golden/` (`git ls-files data/golden | wc -l` → 29); `controls` = 395 non-comment lines across `data/expected/controls/*.ids`.

### The ten mechanisms, resolved against the live inventories (measured)

| # | Mechanism | Resolves as (before Phase 13) | Where it is proved | Registration (CONTEXT) |
|---|-----------|-------------------------------|--------------------|------------------------|
| 1 | `important-gates-complete-uncovered` | `control` | `tools/check/controls/important-gates-complete-uncovered/` | `control` row (zero cost) |
| 2 | `important-gates-excluded-stale` | **nothing** | only as a co-firing `EXPECT` line in control #1 | **new `control` directory** (0.31 cpu-s) |
| 3 | `important-gates-excluded-malformed` | `control` | `tools/check/controls/important-gates-excluded-malformed/` | `control` row (zero cost) |
| 4 | `control-toy-drift` | **nothing** | `steps/runner-selftest.sh:96-100` builds case `st-toy-drift`; `:117-131` asserts the token | `mutant` row over `st-toy-drift` |
| 5 | `control-deviation-unused` | **nothing** | `steps/runner-selftest.sh:103-108` builds case `st-deviation-unused`; `:117-131` asserts the token | `mutant` row over `st-deviation-unused` |
| 6 | `control-pass-line-not-last` | **nothing** | `steps/runner-selftest.sh:133-180` (copied runner + `pass-line.sh`); the case's control dir is `st-pass-line` | `mutant` row over `st-pass-line` |
| 7 | `sources-src-token-whole` | `control` | `tools/check/controls/sources-src-token-whole/` | `control` row (zero cost) |
| 8 | `manifests-output-anchor` | `control` | `tools/check/controls/manifests-output-anchor/` | `control` row (zero cost) |
| 9 | `memmap-generator-refused` | `control` | `tools/check/controls/memmap-generator-refused/` | `control` row (zero cost) |
| 10 | `memmap-generator-missing` | **nothing** | `steps/runner-selftest.sh:205-226` — a bare block with **no case name** | `mutant` row over a minted name (see R1) |

The five `control` rows cost **zero** new cpu-s and need no new file. The four selftest-only ones need `data/expected/runner-selftest.ids` plus one added path in the `mutant` loop, and `runner-selftest.sh` extended to read that inventory and fail on a name it does not run. That is also **zero** new runtime cost, because those cases already run every gate (`cases=13` measured, unchanged). Mechanism #2 needs one new control directory.

### `claim.cmake` — the exact insertion point for D-50

```cmake
# Source: tools/check/kinds/claim.cmake:157-177 (verbatim)
  set(weak FALSE)
  foreach(i RANGE 0 ${dlast})
    json_type(et "${rec}" depends_on ${i})
    if(NOT et STREQUAL "STRING")
      gate_fail(claim-type "${id}/depends_on")
      continue()
    endif()
    string(JSON field GET "${rec}" depends_on ${i})
    if(NOT "${field}" IN_LIST known)
      gate_fail(claim-unknown-field "${id}/${field}")
      continue()
    endif()
    get_property(ftag GLOBAL PROPERTY "CLAIM_PROFILE_TAG:${profile}:${field}")
    if("${ftag}" MATCHES "^(tier4-community|tier5-emulator-hint|inference|unknown-default-chosen)$")
      set(weak TRUE)
    endif()
  endforeach()
  # Weakest-field rule: an established claim may not rest on a weak field.
  if(weak AND standing STREQUAL "established")
    gate_fail(claim-overstated "${id}")
  endif()
```
D-50 adds, in this same function: (a) `standing STREQUAL "established" AND NOT tier STREQUAL "source-expectation"` → a new token; (b) inside the loop, an established claim whose field tag is not `tier1-primary-doc` → a second new token. Note `:148-155` returns early when `depends_on` is absent or empty, but `schema/claim.txt:27` already refuses that for an established claim (`when standing=established nonblank depends_on`), so the early return cannot make (b) vacuous. The existing weak-field rule stays: D-50 is strictly stronger, and after it `claim-overstated` becomes unreachable for the established case it used to cover (`:175` also requires `established`). **Decide deliberately** whether `claim-overstated` is subsumed (keep both, the new one firing first is fine because both are findings) or retargeted. The existing control `claims-overstated-standing` overlays the toy `CL-0001` and expects `FAIL claim-overstated: CL-0001`; if D-50's new tokens fire on the same overlay the control still passes (EXPECT is a substring match, not an exclusive one) — but the *ordering* changes which line appears first, which `control-pass-line-not-last` does not care about and `EXPECT` does not either. Safe.

### The toy tree already holds the legal positive (measured)

```json
// Source: tools/check/controls/_good/tree/data/claims.json (verbatim)
{ "id": "CL-0001", "statement": "The example profile names the STIC part.",
  "profile": "intv-2609-ntsc-family-p1", "tier": "source-expectation",
  "artifact": "data/tiers.json", "depends_on": ["stic.part"],
  "standing": "established", "note": "Example entry." }
```
and the toy profile's `stic.part` is tagged `tier1-primary-doc` (measured over `_good/tree/data/profiles/intv-2609-ntsc-family-p1.json`: tags are `tier2-first-party-adjacent` ×3 — `model`, `input.control.key_1`, `input.control.key_2` — `tier1-primary-doc` ×1 — `stic.part` — and `unknown-default-chosen` ×2 — `board_revision`, `psg.regmap`). So:
- the **positive** needs no new fixture: the pristine `_good/claims` run already exercises it;
- `claim-established-without-tier1` overlays `depends_on` with one of `model`/`input.control.key_1`/`input.control.key_2`;
- `claim-established-non-source` overlays `tier` with e.g. `deterministic-repeatability` (a legal id in `data/expected/evidence-tiers.ids`).

## Phase 13 Concrete Values

Everything in this section is a measured value or a recommendation the planner can copy. Values marked **(decision)** are the planner's or the owner's to choose; values marked **(measured)** were produced by a command in this session.

### The ten rows: disposition, tokens, and the pin census

Dispositions are a **recommendation** (the ledger's content is RTS-01's deliverable, not a research finding), constrained by CONTEXT's resolution that `libretro-abi` is the **one** live `cleared` row. Tokens are chosen so that each is a *character-bounded* artifact spelling, per Pitfall 2.

| slug | disposition (rec.) | candidate `tokens` | pre-existing bounded hits in the 4 scopes (measured) |
|------|--------------------|--------------------|------------------------------------------------------|
| `flat-cartridge-image` | `owner-local-only` | `flat-image`, `flat cartridge` | 0 |
| `cfg-container` | `refused` | `cfg-container`, `.cfg` | 0 as a hyphenated token; bare `cfg` = 9 files (do **not** use bare `cfg`) |
| `bank-modes` | `refused` | `bank-switch`, `bankswitch` | 0 |
| `exec-image` | `owner-local-only` | `exec-image` | 4 hits in 2 files — **4 pins** |
| `grom-image` | `owner-local-only` | `grom-image` | 4 hits in 2 files — **4 pins** |
| `commercial-rom` | `refused` | `game-rom`, `cartridge-dump`, `commercial-rom` | 0 |
| `rendered-frame` | `refused` | `screenshot`, `rendered-frame`, `page-render` | 0 |
| `save-state` | `owner-local-only` | `save-state`, `savestate` | 0 |
| `libretro-abi` | **`cleared`**, `source: "SRC-libretro-api-docs"`, `taken: "nothing"` | `libretro` | 0 |
| `core-info-strings` | `owner-local-only`, `source: null`, `taken: "nothing"`, `tokens: []` | — (empty; see note) | 0 |

`core-info-strings` takes an **empty** `tokens` list on purpose: the libretro-shaped field names it would otherwise refuse are governed by the `libretro-abi` row, and giving it a non-empty list would make one row refuse what the row above clears. CONTEXT's count ("`libretro-abi` becomes the one live `cleared` row") forbids making it a second `cleared` row, and `refused` would contradict the cleared ABI. An empty token list is a recorded answer, not a missing one — the row still exists, has a `taken` cell and a `note`, and `rights.ids` still requires it.

**The pin census (measured).** Only two spellings have pre-existing hits, and they are the same four lines:

```
tools/owner-run.sh:7      # Usage: ... <exec-image> <grom-image>
tools/owner-run.sh:18     usage='usage: owner-run.sh ... <exec-image> <grom-image>'
docs/owner-images.md:47   `sh tools/owner-run.sh ... <exec-image> <grom-image>` does the three steps ...
docs/owner-images.md:64   starts `ivs-run --exec <exec-image> --grom <grom-image>` on that copy ...
```

A pin is a `(token, path)` pair, so this is exactly **4 pins** (`exec-image tools/owner-run.sh`, `exec-image docs/owner-images.md`, `grom-image tools/owner-run.sh`, `grom-image docs/owner-images.md`) regardless of how many occurrences each path holds. These are the operator's own argument placeholders, so pinning them is correct rather than convenient: the names *are* the operator interface.

### The toy-registry arithmetic

**(measured)** The toy tree's two requirement records are byte-identical to each other (`diff` → no output) and hold 46 rows each; the toy registry holds 45 non-comment lines because `FND-98` is Complete and excluded (`_good/tree/data/expected/important-gates-excluded.ids:14`). The live accepted set is also 46 (58 `Complete` − 12 excluded). The toy record is *not* an automatic mirror: it is `live − EV-05 + FND-98` (measured id diff), which is why `b14e1a4` (Phase 10) touched no toy file while `16afb31` (TOOL-01) touched all thirteen (10 registries + 2 records + 1 overlay copy).

So "all ten toy registries gain exactly one line each" is mechanically: **one** new toy requirement row appended to

1. `tools/check/controls/_good/tree/.planning/REQUIREMENTS.md`,
2. `tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md`,
3. `tools/check/controls/important-gates-complete-uncovered/overlay/.planning/REQUIREMENTS.md` (the whole-file copy), and

one toy registry line for that id appended to each of the ten whole-file toy registries:

4. `tools/check/controls/_good/tree/data/expected/important-gates.ids`,
5. `tools/check/controls/important-gates-bad-req-phase5/overlay/data/expected/important-gates.ids`,
6. `tools/check/controls/important-gates-bad-req-phase6/overlay/data/expected/important-gates.ids`,
7. `tools/check/controls/important-gates-req-uncovered-phase5/overlay/data/expected/important-gates.ids`,
8. `tools/check/controls/important-gates-req-uncovered-phase6/overlay/data/expected/important-gates.ids`,
9. `tools/check/controls/important-gates-req-uncovered-phase7/overlay/data/expected/important-gates.ids`,
10. `tools/check/controls/important-gates-uncovered-gate/overlay/data/expected/important-gates.ids`,
11. `tools/check/controls/important-gates-unknown-id/overlay/data/expected/important-gates.ids`,
12. `tools/check/controls/important-gates-unknown-id-phase5/overlay/data/expected/important-gates.ids`,
13. `tools/check/controls/important-gates-unknown-id-phase6/overlay/data/expected/important-gates.ids`.

That is 13 files, 13 added lines, zero deletions — exactly the `16afb31` diff shape (`16 files changed, 32 insertions(+), 2 deletions(-)`, of which 13 files/13 lines are the toy edits and 2 deletions are the two requirement-status flips). **Do not** add a `DEVIATION` file: the addition is intended in all ten, and a `DEVIATION` entry would trip `control-deviation-unused`. Measured: all nine overlays differ from `_good` by exactly one declared line, and all ten files end with `toy-tool01 fault GOOD_FAULT TOOL-01`, so a single same-line append leaves every existing difference untouched.

### The measured baselines and the predicted deltas

**(measured this session, `git ls-files` → 2664 paths)**

| step | pass line today | Phase 13 predicted | why it moves |
|------|-----------------|--------------------|--------------|
| `files` | `tracked=2664 binaries-listed=0` | `tracked=2664+N binaries-listed=0` | every new file is tracked; `N` = the count of new tracked files (`files.cmake:89-99`, `:173-174`). **No new stat key** — the token delta is the tracked count alone. |
| `manifests` | `manifests=11 effective-yes=8 effective-no=3` | unchanged | the ledger is a record kind, not a fixture manifest |
| `claims` | `claims=28 provisional=27` | unchanged | `CL-0002` is asserted, never edited; the legal positive is a toy row |
| `important-gates` | `gates=110 lines=337` | **`gates=114 lines=347`** | 10 rows, 4 new distinct gate names (R5) |
| `sources` | `sources=12 refs-verified=3/4` | **`sources=13 refs-verified=3/4`** | one `reference-only` record: `source-total` +1, `source-hashed` unchanged (R2, Pitfall 9) |
| `runner-selftest` | `cases=13` | unchanged | the inventory changes which names are *checked*, not which cases run |
| `run-controls.sh` | `controls=395/395` | `controls=401/401` | 6 new control directories |
| gate summary | `steps=38/38 … faults=238/238 … goldens=29` | `steps=38/38 … faults=238/238 … goldens=29` | the three invariants CONTEXT freezes |

`steps=38/38` is measured (`ls tools/check/steps/*.sh | wc -l` → 38), `faults=238` is measured (`awk '!/^#/ && NF' data/expected/faults/*.ids | wc -l` → 238), `goldens=29` is measured (`git ls-files data/golden | wc -l` → 29), `controls=395` is measured (`awk '!/^#/ && NF' data/expected/controls/*.ids | wc -l` → 395), and `lines=337` is measured both ways (the step's own printed stat and `awk '!/^#/ && NF' data/expected/important-gates.ids | wc -l`).

### The cost-checkpoint arithmetic

**(measured, 2026-11-26, from `.planning/ci-runs.tsv`)** 96 data rows, 93 `success` rows with a real `cpu_s`. Last five rows in file order:

| date | event | `cpu_s` | `gate_s` |
|------|-------|---------|----------|
| 2026-11-25 | push | 435 | 137 |
| 2026-11-25 | pull_request | 366 | 121 |
| 2026-11-25 | push | 363 | 114 |
| 2026-11-25 | pull_request | 497 | 148 |
| 2026-11-25 | push | 357 | 121 |

medians: `cpu_s` **366**, `gate_s` **121**. The frozen figures (`MILESTONE-CONTEXT.md:222-223`, `scoping-2-gate-cost.md:59-61`) are `cpu_s` 361, `build_s` 34, `gate_s` 118 — i.e. **already 5 cpu-s stale**, which is why the checkpoint recomputes rather than quotes.

**The phase's own delta, priced:**

| item | unit (frozen, `MILESTONE-CONTEXT.md:227-228`) | count | cpu-s |
|------|---------------------------------------------|-------|-------|
| new control directories | 0.31 each | **6** (R1: 1; EV-06: 3; RTS-01: 1; RTS-02: 1) | 1.86 |
| `important-gates.ids` rows | 0.005 each | 10 | 0.05 |
| new `runner-selftest.ids` cases | 0.05 each (already running) | 0 | 0.00 |
| new faults | 0.44 each — **forbidden** | 0 | 0.00 |
| **total** | | | **1.91 of 2.0** |

**The six-directory budget is a hard ceiling, not a target.** A seventh directory costs 0.31 and takes the total to 2.22, over the 2.0 line. The planner must therefore fold each requirement's limbs into the fewest directories that still show every limb able to fail: RTS-01 → **1** (one overlay planting a row with a missing `taken` cell *and* a `cleared` row citing an unregistered `SRC-`; two `EXPECT` lines), RTS-02 → **1** (one overlay planting a refused token in `core/src` *and* a stale pin; `EXPECT rights-token` + `EXPECT rights-pinned-stale`, `FORBID` the finding for the pinned pre-existing pair), EV-06 → **3** (CONTEXT names three), EV-07 → **1**. If a limb genuinely cannot share a directory, the honest response is to record `reprice` at the boundary rather than add the directory silently.

Verdict rule (`scoping-2-gate-cost.md:59-64`): `discharged-within-allowance` if median `cpu_s` ≤ 361 + priced addition and the cumulative spend ≤ 21; `reprice` if the median moved more than the priced addition with an unchanged token line; `lever-required` if the cumulative would pass 21 or a phase overruns its line by >25 %. **The measured median is already 366 = 361 + 5, which is more than this phase's 2.0 priced addition with the token line unchanged** — so unless the token line moves by the predicted amount, the honest verdict for this phase's checkpoint is `reprice`, not `discharged-within-allowance`. State this explicitly in the plan's checkpoint task instead of discovering it at the boundary.

## Resolved Decisions and Their Mechanical Consequences

The four open questions in the previous draft of this file were resolved in `13-CONTEXT.md` (`### Resolved from the research open questions (2026-11-26)`), plus a fifth that the resolutions forced. Each resolution below is stated once and then reduced to the exact files and lines it touches. **None of them re-opens a locked decision.**

### R1. EV-07 registration: 5 `control` rows, 4 `mutant` rows through a new `runner-selftest.ids`, 1 new `control` directory

**The decision.** Five of the ten mechanisms already resolve as `control` (their control directories exist). Four are proved only by `runner-selftest.sh` cases and resolve as nothing. One (`important-gates-excluded-stale`) fires only as a co-firing `EXPECT` line inside another control. Registering a `mutant` row for a mechanism that already has a control directory would be a second registration of one proof. So: 5 `control`, 4 `mutant`, 1 new `control` directory, zero new faults, and the deviation from EV-07's literal "`mutant` row" wording recorded in the same commit with that reason.

**1a. The three-inventory loop as it stands** (`tools/check/steps/important-gates.sh:130-147`, verbatim):

```sh
# The three inventories, one `<mechanism> <id>` line each.
: > "$tmp/known"
for f in "$exp"/faults/*.ids; do
  [ -f "$f" ] || continue
  awk '/^#/ { next } NF { print "fault", $1 }' "$f" >> "$tmp/known"
done
for f in "$exp"/controls/*.ids; do
  [ -f "$f" ] || continue
  awk '/^#/ { next } NF { print "control", $1 }' "$f" >> "$tmp/known"
done
for f in "$exp/pkg-selftests.ids" "$exp/ci-summary-selftests.ids"; do
  [ -f "$f" ] || continue
  awk '/^#/ { next } NF { print "mutant", $1 }' "$f" >> "$tmp/known"
done
for f in "$exp"/tests/*.ids; do
  [ -f "$f" ] || continue
  awk '/^#/ { next } $1 ~ /^[a-z0-9-]+[.](judge|mutant)[.][^ ]+$/ { print "mutant", $1 }' "$f" >> "$tmp/known"
done
```

**1b. The minimal, correct change — one line.** Replace line 140 with:

```sh
for f in "$exp/pkg-selftests.ids" "$exp/ci-summary-selftests.ids" "$exp/runner-selftest.ids"; do
```

That is the whole resolution mechanism: the loop already prints `mutant <first field>` for every non-comment line of every file it is given, and it already tolerates an absent file (`[ -f "$f" ] || continue`). Because of that `|| continue`, **the toy tree needs no new file** — `_good/tree/data/expected/` has no `runner-selftest.ids`, the loop skips it, and no toy registry row names a `st-*` id.

**1c. The two prose places that document the mutant sources must move with it.** `tools/check/steps/important-gates.sh:61-63` (verbatim):

```
#   mutant   a case of data/expected/pkg-selftests.ids or data/expected/ci-summary-selftests.ids
#            (in-step selftests), or a test id `<suite>.judge.<slug>` or `<suite>.mutant.<slug>`
#            of any data/expected/tests/*.ids (suite self-controls, judged by tests-inventory).
```

becomes `…a case of data/expected/pkg-selftests.ids, data/expected/ci-summary-selftests.ids or data/expected/runner-selftest.ids (in-step selftests)…`. The mirror sentence in the registry's own header, `data/expected/important-gates.ids:8-9` (verbatim):

```
# (EXPECT token); a mutant is an in-step selftest case (pkg-selftests.ids, ci-summary-selftests.ids)
# or a suite self-control `<suite>.judge.<slug>` / `<suite>.mutant.<slug>` that passes only when
```

gets the same third file name. Leaving either stale is a documented-invariant defect, not a cosmetic one.

**1d. The new inventory, four lines.** `data/expected/runner-selftest.ids` (new file), one case name per line, with a header naming the owning plan, the four mechanisms, and the invariant:

```
# Owning plan 13-0N (EV-07): the four D-09-06 mechanisms whose only proof is an in-step case of the
# gate step runner-selftest (D-12). One case name per line; the step runs exactly these and fails on
# a name it does not run (runner-selftest-missing). The other six mechanisms of the D-09-06 list are
# registered as `control` rows, because a control directory already proves them.
st-toy-drift
st-deviation-unused
st-pass-line
st-memmap-generator-missing
```

**1e. Which four case names exist — measured, and one of them must be minted.** Three exist verbatim in `tools/check/steps/runner-selftest.sh`:

| mechanism | case name in the script | where |
|-----------|------------------------|-------|
| `control-toy-drift` | `st-toy-drift` | `:94-100` builds the control dir (`mkdir -p "$ctl/st-toy-drift/overlay/data/expected"`); `:121` pairs `control-toy-drift:st-toy-drift`; `:117-129` asserts `-- FAIL control-toy-drift: st-toy-drift` |
| `control-deviation-unused` | `st-deviation-unused` | `:102-108` builds it; `:121` pairs `control-deviation-unused:st-deviation-unused`; `:117-129` asserts it |
| `control-pass-line-not-last` | `st-pass-line` | `:155` `mkdir -p "$pl/controls/st-pass-line" "$pl/inventory"`; `:160` `printf '%s\n' st-pass-line > "$pl/inventory/selftest.ids"`; `:165` asserts `-- FAIL control-pass-line-not-last: profile` |

The fourth has **no case name at all**. The `memmap-generator-missing` proof is a bare block (`:205-225`) with no control directory — its own comment says why (`:205-211`, verbatim):

```
# Generator missing (EV-03 item 5). The memmap step derives the generator's path from CHECK_DIR,
# and a control may only change the tree, so no control directory can reach this guard. The copy
# above has no sibling cmake/, so the step must report the liveness guard and stop before invoking
# cmake at all. Without this case the guard's failure would be recorded only as prose - the
# lesson-20 defect GUARDRAILS 2.3 exists to prevent. The filelist is a runtime artifact; the
# script's own source must never spell the tree's generated directory name, because a step's
# partition class is derived by grepping this file.
```

It is proved by the string `memmap-generator-missing` (`:218`, `:224`), not by a case name. The inventory therefore **mints** `st-memmap-generator-missing` for it, following the `st-` convention of the other eleven case names, and the script records that name when the block passes.

**1f. The `runner-selftest.sh` extension.** The script has no inventory today: it builds thirteen proofs and increments an integer `cases`. Extend it exactly the way `pkg-install.sh:832-851` does (quoted in full under Architecture Patterns → Pattern 6):

1. after the `bad()` helper, add `ran="$tmp/ran"; : > "$ran"`;
2. at each point a case succeeds, append its name: in the `pair` loop `printf '%s\n' "$name" >> "$ran"` (the loop already binds `name=${pair#*:}` at `:123`), and one `printf` for `st-ok`, `st-pass-line` and `st-memmap-generator-missing`;
3. after the last case and before `[ "$rc" -eq 0 ] || exit 1` (`:227`), add the inventory read and the one-directional check:

```sh
sinv="$ROOT/data/expected/runner-selftest.ids"
if [ -f "$sinv" ]; then
  awk '/^#/ { next } NF { print $1 }' "$sinv" > "$tmp/st.cases"
  [ -s "$tmp/st.cases" ] || bad runner-selftest-inventory "no cases"
  while IFS= read -r c; do
    grep -q -x -F -e "$c" "$ran" || bad runner-selftest-missing "$c"
  done < "$tmp/st.cases"
else
  bad runner-selftest-inventory "data/expected/runner-selftest.ids"
fi
```

The direction that matters is the one `pkg-install` has: an inventory name the step does not run is `runner-selftest-missing`, so a renamed or deleted case cannot leave a registry row pointing at nothing. `cases=13` does **not** change, and the header's `# Tokens:` line (`:26`) gains `runner-selftest-inventory: <path>` beside the existing `runner-selftest-missing: <token>`.

**1g. The new control directory for `important-gates-excluded-stale`.** The mechanism already fires inside `important-gates-complete-uncovered`, but a `control` *row* resolves only if a directory **of that name** is inventoried (`important-gates.sh:136-139` reads `data/expected/controls/*.ids`; `run-controls.sh:290-298` requires the inventory and the directories to agree in both directions). So the directory exists to give the mechanism its own name, and its `FORBID` list is what proves it failed for *that* reason alone.

```
tools/check/controls/important-gates-excluded-stale/
├── CHECK   -> important-gates
├── EXPECT  -> FAIL important-gates-excluded-stale: FND-96
├── FORBID  -> important-gates-complete-uncovered
│              important-gates-uncovered-gate
│              important-gates-excluded-malformed
│              important-gates-unknown-id
└── overlay/data/expected/important-gates-excluded.ids
```

The overlay is a whole-file copy of the toy ledger (`tools/check/controls/_good/tree/data/expected/important-gates-excluded.ids`, 14 lines: 13 comment lines plus `FND-98 close_record_only_toy_positive_row_for_the_ledger_format` as the last line) with **one line appended**:

```
FND-96 not_a_gate_mechanism_at_all
```

`FND-96` is not marked Complete in either toy record, so `important-gates.sh:185` (`for (x in excluded) if (!(x in complete)) bad("important-gates-excluded-stale", x)`) is the only finding: the accepted set is unchanged, the toy registry still covers all 46, and the malformed-row validator is satisfied (two fields, `^[A-Z][A-Z0-9]*-[0-9][0-9]*$`, six underscore-joined words, not `none`). Keeping the `FND-98` row is load-bearing: dropping it would make `FND-98` Complete-and-uncovered and add the two tokens the `FORBID` list forbids. **The inventory that carries this directory is `data/expected/controls/important-gates.ids`, not `data/expected/controls/rights.ids`** — the latter owns the plan 01-06 `manifests-*`/`files-*` rights controls.

**1h. The ten rows, with gate names.** Gate names are free (`important-gates.sh:18`), and `16afb31` set the precedent of one new gate name for a requirement's whole mechanism set ("TOOL-01's four mechanisms under one new gate name, `runner-record`"). Four new names, ten rows:

| # | row | mechanism | gate name |
|---|-----|-----------|-----------|
| 1 | `d09-06-registration control important-gates-complete-uncovered EV-07` | control | new |
| 2 | `d09-06-registration control important-gates-excluded-stale EV-07` | new control | new |
| 3 | `d09-06-registration control important-gates-excluded-malformed EV-07` | control | new |
| 4 | `d09-06-registration mutant st-toy-drift EV-07` | mutant | new |
| 5 | `d09-06-registration mutant st-deviation-unused EV-07` | mutant | new |
| 6 | `d09-06-registration mutant st-pass-line EV-07` | mutant | new |
| 7 | `d09-06-registration control sources-src-token-whole EV-07` | control | new |
| 8 | `d09-06-registration control manifests-output-anchor EV-07` | control | new |
| 9 | `d09-06-registration control memmap-generator-refused EV-07` | control | new |
| 10 | `d09-06-registration mutant st-memmap-generator-missing EV-07` | mutant | new |

plus `rights-ledger` (RTS-01), `rights-naming` (RTS-02) and `claim-ceiling` (EV-06) → **4 new gate names total**, so `gates=110 → 114`, `lines=337 → 347`. `EV-07` must be `Complete` in the same commit (Pitfall 1).

### R2. One live `cleared` row: one `reference-only` source record

**The decision.** Add the libretro API documentation as one `reference-only` record, so `libretro-abi` becomes the one live `cleared` row. Nothing is vendored (D-48). The record exists because a control directory can never express "this passes" (Pitfall 5), so without it the `cleared` branch of RTS-01 would be dead code on the real tree.

**2a. What `reference-only` requires** — `tools/check/schema/source.txt:11-33` (verbatim), which is the full key set the record must satisfy:

```
keys id title url wayback accessed capture sha256 size_bytes pages rights authority_tier local_path note
req id STRING
req title STRING
req url STRING
req wayback STRING|NULL
req accessed STRING
req capture STRING
req sha256 STRING|NULL
req size_bytes NUMBER|NULL
req pages NUMBER|NULL
req rights STRING
req authority_tier STRING
req local_path STRING|NULL
req note STRING
nonblank title
nonblank url
nonblank note
match id ^SRC-[a-z0-9-]+$
match accessed ^[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]$
match capture ^(hashed|reference-only)$
match rights ^unknown$
enumfrom authority_tier data/tiers.json source_authority_tags id
match sha256 ^[0-9a-f]+$
hook source
```

(The earlier draft cited `schema/source.txt:20` for the `rights` pin; the live line is **`:31`** — `:20` is `req pages NUMBER|NULL`. Corrected.)

**2b. What `reference-only` additionally demands** — `tools/check/kinds/source.cmake:63-83` (verbatim):

```cmake
  elseif(capture STREQUAL "reference-only")
    if(NOT shat STREQUAL NULL)
      gate_fail(source-format "${id}/sha256")
    endif()
    if(NOT szt STREQUAL NULL)
      gate_fail(source-format "${id}/size_bytes")
    endif()
    if(NOT lpt STREQUAL NULL)
      gate_fail(source-format "${id}/local_path")
    endif()
    # The note must say why the document is not hashed.
    set(note "")
    json_type(nt "${rec}" note)
    if(nt STREQUAL STRING)
      string(JSON note GET "${rec}" note)
    endif()
    string(TOLOWER "${note}" lnote)
    string(FIND "${lnote}" "hash" hpos)
    if(hpos LESS 0)
      gate_fail(source-reference-only-unexplained "${id}")
    endif()
  endif()
```

So `sources-reference-only-unexplained` is satisfied by exactly one thing: the lower-cased `note` contains the substring `hash`. `"not hashed because the API documentation is a living multi-page site"` satisfies it; a note that explains the reason without that substring does not. `sha256`, `size_bytes` and `local_path` must be JSON `null` (not absent, not `""`).

**2c. Both files need the row.** `schema/source.txt:10` is `inventory data/expected/sources.ids`, and `records.cmake`'s inventory step is exact set equality in both directions (`records.cmake:313-324`, `<kind>-id-missing` / `<kind>-id-extra`). So `data/sources.json` **and** `data/expected/sources.ids` both gain the row in the same commit; adding it to one is a red gate either way.

**2d. The record shape** (`data/sources.json`, appended to the `sources` array; `sources.ids` is add-only per its header, so append there too):

```json
{
  "id": "SRC-libretro-api-docs",
  "title": "libretro core API documentation",
  "url": "<the documentation root the reading used>",
  "wayback": null,
  "accessed": "<YYYY-MM-DD, the date of the actual reading>",
  "capture": "reference-only",
  "sha256": null,
  "size_bytes": null,
  "pages": null,
  "rights": "unknown",
  "authority_tier": "tier5-emulator-hint",
  "local_path": null,
  "note": "Not hashed because the documentation is a living multi-page site whose pages change independently, so a hash of one page would not identify the document; nothing is vendored (D-48). Read as a lead for the shape of the core-info block and the ABI entry points only."
}
```

`authority_tier` is `tier5-emulator-hint` by the registry's own precedent — `SRC-mame-intv` (an emulator's source) carries exactly that tag, whose `where_allowed` is "notes and discrepancy leads, never as the tag of a profile field" (`data/tiers.json`), which is what this record is for. `tier4-community` is the defensible alternative if the docs are treated as community documentation; either is a legal `enumfrom` value. The `url` and the `accessed` date are the two values that must come from the actual reading, not from this research — see Assumptions Log A7.

**2e. The predicted `sources` delta.** `sources=12 refs-verified=3/4` → **`sources=13 refs-verified=3/4`**. `gate_incr(source-total)` fires for every record (`kinds/source.cmake:12`); `gate_incr(source-hashed)` fires only in the `hashed` branch (`:22-23`); `gate_incr(source-verified)` fires only when a cached `refs/` file exists and matches (`:57-58`). A `reference-only` row moves the numerator of the first stat and nothing else. Predicting `refs-verified=4/5` is the error Pitfall 9 names.

**2f. A second, independent resolution comes for free.** `steps/sources.sh:30-33` scans `grep -E '^data/.*[.]json$'` over `FILELIST` minus `data/sources.json` — which includes the new `data/rights/libretro-abi.json`. Its `source` field therefore also has to resolve through `data/expected/sources.ids` (`sources.sh:50-51`, token `source-unknown-ref`). So a `cleared` row's source is checked twice, by two different steps, with two different mechanisms. That is a genuine cross-check, not duplication: the `manifests` rule asks "is this a registered source", the `sources` rule asks "does this token exist anywhere it is written".

### R3. `rights-pinned-stale` is a failure, and where it belongs

**The decision.** A pin that no longer matches fails, so the pin list cannot rot into a wildcard or a silently widened rule. `history_compare`'s NOTE is right for history (a pin names a commit that cannot disappear) and wrong for rights (a pin names a path that can).

**3a. Where.** In `tools/check/files.cmake`, after the `FILELIST` loop closes and **before** the two `gate_stat` calls — i.e. between `:163` and `:173`. The anchor, verbatim (`files.cmake:163-175`):

```cmake
endforeach()

list(REMOVE_DUPLICATES fixture_dirs)
list(SORT fixture_dirs)
foreach(d IN LISTS fixture_dirs)
  if(NOT "${d}" IN_LIST manifest_dirs)
    gate_fail(files-fixture-without-manifest "${d}")
  endif()
endforeach()

gate_stat(tracked "${ntracked}")
gate_stat(binaries-listed "${nbin}")
gate_finish(files)
```

That position is the only correct one: the token scan has finished (so the observed `(token, path)` set is complete), the checker still has the sorted `paths` list in scope, and the failure lands before `gate_finish` — which is what makes it a `FAIL` line and not a `NOTE`.

**3b. What it mirrors, exactly.** The reader half is `tools/release-readiness.sh:52-63` (verbatim):

```awk
  awk -v pin="$2" '
    BEGIN {
      while ((getline l < pin) > 0) {
        sub(/^[ \t]+/, "", l)
        if (l == "" || substr(l, 1, 1) == "#") continue
        if (split(l, w, " ") == 2) {
          key = w[1] " " w[2]
          if (!(key in pinned)) { pinned[key] = 1; pord[++np] = key }
        }
      }
      close(pin)
    }
```

The comparison half is `:74-83` (verbatim):

```awk
    END {
      acc = 0; bad = 0
      for (j = 1; j <= k; j++) {
        if (order[j] in pinned) { printf "FAIL (accepted by recorded disposition, D-33): %s\n", order[j]; acc++ }
        else { printf "NEW FINDING: %s\n", order[j]; bad++ }
      }
      for (j = 1; j <= np; j++) if (!(pord[j] in seen)) printf "NOTE: pinned pair not found in this run: %s\n", pord[j]
      printf "history findings: %d (accepted %d, new %d)\n", k, acc, bad
      exit (bad > 0 ? 1 : 0)
    }' "$1"
```

`split(l, w, " ") == 2` is the grammar to copy verbatim: exactly two whitespace-separated fields, so a pin line cannot become a wildcard. **The one deliberate divergence** is line `:80`: its `NOTE` becomes a failure in `files.cmake`:

```cmake
# data/expected/rights-pinned.ids: one "<token> <path>" pair per line, exactly two fields.
# A pin that no longer matches is a failure, not a NOTE: history pins a commit, which cannot
# disappear; a rights pin names a path, which can, and a stale pin means the rule silently widened.
read_ids(pins "data/expected/rights-pinned.ids" rights-pinned)
foreach(pin IN LISTS pins)
  string(REGEX MATCH "^[^ ]+ +[^ ]+$" two_field "${pin}")
  if(two_field STREQUAL "")
    gate_fail(rights-pinned-malformed "${pin}")
    continue()
  endif()
  if(NOT "${pin}" IN_LIST seen)
    gate_fail(rights-pinned-stale "${pin}")
  endif()
endforeach()
```

`read_ids` is reusable here even though a pin is two fields: it treats the whole stripped line as one id, which is exactly the pair key, and it gives `rights-pinned-inventory-missing|empty|duplicate` for free (`lib.cmake:79`, `:104`, `:112-114`). Two new tokens: `rights-pinned-stale` (the resolved decision) and `rights-pinned-malformed` (the grammar, which `history_compare` silently drops instead — a silently dropped pin line is the same rot in a different disguise).

**3c. The pins it must accept today.** Exactly four, all measured (see "Phase 13 Concrete Values → The ten rows…"):

```
exec-image docs/owner-images.md
exec-image tools/owner-run.sh
grom-image docs/owner-images.md
grom-image tools/owner-run.sh
```

Because a stale pin now fails, this file is a **second inventory**: deleting a line from it is a behaviour change with its own diff, and the RTS-02 control must prove the reverse direction by planting a pin that matches nothing.

### R4. The toy registries: one row, one line, thirteen files

**The decision.** One new toy requirement row in the two toy requirement records plus the whole-file overlay copy, and one registry line appended to each of the ten whole-file toy registries — 13 files, 13 added lines, zero deletions, no `DEVIATION` entry. The toy record is a curated mirror, not an automatic one.

**4a. The chosen toy id, verified unused everywhere.** `EV-98`. Measured: `grep -rn "EV-98" .` (excluding `build/` and `.git/`) returns **nothing**; the toy records' 46 ids contain exactly one `-9x` id, `FND-98`; and `FND-99`/`EV-99` are already in use as the `important-gates-complete-uncovered` overlay's Complete-without-row id and its stale-exclusion id respectively (`overlay/.planning/REQUIREMENTS.md:57`, `overlay/data/expected/important-gates-excluded.ids:12`). `FND-96` is likewise unused (measured) and is reserved by R1 for the stale-exclusion id, so the two new toy ids do not collide.

**4b. The two appended lines, verbatim** (mirroring `16afb31`'s `| TOOL-01 | Phase 11 | Complete |` and `toy-tool01 fault GOOD_FAULT TOOL-01`):

```
| EV-98 | Phase 13 | Complete |
```

appended to files 1-3 of the list, and

```
toy-phase13 fault GOOD_FAULT EV-98
```

appended to files 4-13. `GOOD_FAULT` is a real toy fault (`_good/tree/data/expected/faults/cpu.ids:2`), `toy-phase13` is a legal gate name (`^[a-z0-9][a-z0-9-]*$`), and the four columns satisfy `important-gates.sh:171-176`. The id resolves as a `fault`, which is why no toy `runner-selftest.ids` is needed.

**4c. The resulting toy identity.** Toy `Complete` ids 46 → 47; toy excluded 1 (`FND-98`, unchanged); toy must-be-covered 45 → 46; toy registry lines 45 → 46. The identity closes with no exclusion row added and no `DEVIATION` file.

### R5. "Baseline re-derived": run the step, quote `gates=`/`lines=`, add the header line

**The decision.** `lines=` is a runtime stat, not a header field. "Re-derived" means: run the step against the real `FILELIST` and quote what it prints; the step header's phase table gains the Phase 13 line in the same commit.

**5a. What to run and what to quote.**

```sh
git ls-files > /tmp/fl
ROOT="$PWD" CHECK_DIR="$PWD/tools/check" FILELIST=/tmp/fl sh tools/check/steps/important-gates.sh
```

Measured today: `-- important-gates pass gates=110 lines=337`. Measured after the ten rows: **`-- important-gates pass gates=114 lines=347`** (337 + 10 rows; 110 + 4 new gate names — R1h). Quote the run's own output in the summary; never quote `MILESTONE-CONTEXT.md`'s frozen `337`/`333` or `09-05-SUMMARY.md`'s figures, both of which are already stale.

**5b. The header phase table, exactly as it stands** (`tools/check/steps/important-gates.sh:37-57`, verbatim):

```
# The phase table below records which plan added which set. It is history for the reviewer, not the
# definition, so it cannot silently omit a requirement.
#   Phase 1  FND-01 FND-02 FND-03 FND-04 FND-05 FND-06 FND-07                      plan 09-04
#   Phase 2  CPU-01 CPU-02 CPU-03 CPU-04 CPU-05                                    plan 09-04
#   Phase 3  MEM-01 MEM-02 MEM-03 MEM-04 MEM-05                                    plan 09-04
#   Phase 4  API-02 API-04 RUN-01 RUN-02 RUN-04 RUN-05 RUN-06 PKG-01 PKG-03 PKG-04  plan 04-24
#   Phase 5  INP-01 INP-02 INP-03 API-03 AUD-01 AUD-02                             plan 05-12
#   Phase 6  VID-01 VID-02 VID-03 VID-04 VID-06                                    plan 06-14
#   Phase 7  SCN-01 plan 07-02; SCN-02 plan 07-10 (A-13: SCN-02 is the owner-local EXEC/GROM route,
#            and its mechanisms are the two user-content consistency controls of the golden step)
#   Phase 8  VID-05 plan 08-08 (the MOB, collision and delay faults, the four frame judges, the six
#            program mutants of ivs-diag-stic-2 and the four frame-grammar controls); STA-01 STA-02
#            plan 08-17 (the nine snapshot and restore faults, the six judges of the state suite and
#            the seven controls of the state step); PRF-01 plan 08-19 (both test ids of the suite
#            bench: the refusal of a mutated golden and the judge of the absence-of-measurement check)
#   Phase 10 EV-05 plan 10-06 (the three controls of the handoff ledger and of the look/listen prose:
#            the two claims controls and the one text control, the first rows either gate name has had)
#   Phase 11 TOOL-01 plan 11-05 (the runner-record gate: the two input-log faults, the writer's dropped
#            event line and the replay's one-tick shift, and the two judge self-controls of the
#            recorded-and-replayed comparison; the accepted set is derived, so the row's nreq limb is
#            gone and this table and the token prose are the only parts a new requirement retargets)
```

**5c. The line to insert after `:57`** (same two-line-continuation shape as the Phase 7/8/10/11 entries):

```
#   Phase 13 RTS-01 RTS-02 EV-06 EV-07 plan 13-0N (the rights ledger's two limbs under two new gate
#            names, the claim ceiling's two rules, and the ten D-09-06 registrations: five control
#            rows, four mutant rows resolved through the new data/expected/runner-selftest.ids, and
#            one new control directory for important-gates-excluded-stale; zero new faults)
```

`16afb31` added exactly four lines to this file for the same reason ("the R2 mechanism retarget: a Phase 11 line in the step header"), so four lines here is the precedent, not an overrun.

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| A hand-written `nreq` split string listing accepted requirements in the step | The accepted set derived live from the requirement records minus an exclusion ledger | plan 09-05 (EV-04, amendment A-01) | A new requirement needs no step edit; it needs the record flip plus rows. Confirmed: no `nreq` in `important-gates.sh`. |
| `r` (ratio) as the cost decision input | `cpu_s` medians, arithmetic over `ci-runs.tsv` | v0.2.0 close, audit G-04 | "Phase 13 Concrete Values → The cost-checkpoint arithmetic": the frozen `cpu_s` 361 is now 366. |
| A registry row only for a requirement being *accepted* | A registry row also for a mechanism that had a guard and a stand-in but no row (D-09-06) | this phase | Ten rows, zero new faults. |
| One `docs/**`-wide rule for rights | Token + scope + pinned pairs | D-46 / R-1 | `cfg`/`ivsc` refusal surfaces survive. |
| A `mutant`-only registration for D-09-06 | 5 `control` + 4 `mutant` + 1 new `control`, chosen by what actually proves each mechanism | this phase (resolved 2026-11-26) | Zero new code for five of the ten; a `mutant` row over a mechanism that already has a control directory would be a second registration of one proof. |
| A pin whose match disappeared produced a NOTE | A stale rights pin fails (`rights-pinned-stale`) | this phase | The pin list cannot rot into a wildcard; the reverse direction is a check, not a log line. |

**Deprecated/outdated:**
- `.planning/research/v0.3.0-scoping-2-gate-cost.md`'s last-five medians (`cpu_s` 361, `build_s` 34, `gate_s` 118) are stale by measurement; the checkpoint must recompute ("Phase 13 Concrete Values → The cost-checkpoint arithmetic").
- The `R2` row of `GUARDRAILS-v0.2.md:21` names an `nreq` split string that no longer exists (the file itself records the amendment at `:32-34`). Its remaining limbs — the registry file and all ten toy registries — are live.
- The previous draft of this file's `schema/source.txt:20` citation for the `rights` pin; the live line is `:31`.
- The previous draft's `files pass tracked=2662`; measured today it is `2664`.

## Assumptions Log

> List all claims tagged `[ASSUMED]` in this research. The planner and discuss-phase use this
> section to identify decisions that need user confirmation before execution.

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | The ledger's `tokens` field is an array of strings and `taken` is a string (`"nothing"` legal) — the CONTEXT lists keys but not their JSON types. | Architecture Patterns → Pattern 1 | A wrong type means the schema file is wrong and every row must be rewritten; cheap to fix before rows land, expensive after. |
| A2 | The naming scan's scope is *path prefix* `core/src/`, `runner/`, `tools/` plus the six pinned pages, applied over `FILELIST` (so control-tree overlays are never scanned on the real tree). | Architecture Patterns → Pattern 3 | If the scope were a filesystem walk, control overlays would be scanned and every planted-token control would refuse the real tree. `sources.sh:5-6` documents exactly this anchoring decision for its own scan, so the precedent is strong — but it is an inference for the rights scan. |
| A3 | The four new gate names are `rights-ledger`, `rights-naming`, `claim-ceiling`, `d09-06-registration`. | Resolved Decisions → R1h | Only affects `gates=` (110 → 114); `lines=` is unaffected. Low risk. |
| A4 | "All ten toy registries gain exactly one line each" means one toy *requirement* row (`EV-98`) added to both toy records plus the overlay copy, therefore one toy registry line per file — the `16afb31` shape. | Resolved Decisions → R4 | If it meant one line per *closed requirement* (four), each registry would gain four lines and the diff 40 lines, not 10. R4 fixes it at one, per CONTEXT. |
| A5 | The phase closes all four of its requirements, so the live `accepted` set moves 46 → 50. | "Common Pitfalls → Pitfall 1" | If a requirement is instead excluded, an exclusion row is needed and the reason must have ≥3 underscore-joined words (`important-gates.sh:110-122`). |
| A6 | `core-info-strings` is `owner-local-only` with an **empty** `tokens` list. | "Phase 13 Concrete Values → The ten rows…" | If the planner wants a non-empty token list, the row must cite `SRC-libretro-api-docs` and become a second `cleared` row, contradicting CONTEXT's "one live `cleared` row". Reversal is one field plus the row's id line. |
| A7 | The new source record's `url`, `title` and `accessed` date come from the actual reading of the libretro API documentation, which this research did not perform (no network access in this session). The record's *shape* above is verified against the schema; the *values* are not. | Resolved Decisions → R2d | A wrong or unread URL is a fabricated citation in a rights record — the exact class of defect `docs/RIGHTS.md` exists to prevent. **The plan must read the document before writing the row**, or record the row as `[ASSUMED]` with the reading deferred. |
| A8 | `authority_tier: tier5-emulator-hint` is the right tag for the libretro API docs (precedent: `SRC-mame-intv`). | Resolved Decisions → R2d | A mis-tagged source is a `enumfrom`-legal but semantically wrong record; `tier4-community` is the alternative. One field. |
| A9 | The minted case name `st-memmap-generator-missing` is acceptable for the fourth `mutant` row. | Resolved Decisions → R1e | If a minted name is judged a rewrite of history, the alternative is to register `memmap-generator-missing` (the token) as the id — same row, different id string, and `known` lookup is by exact string either way. One line. |

**If this table were empty** the research would need no user confirmation; it is not, so A7 is the one row that must be discharged before the source record is written, and A3/A6/A8/A9 are one-line reversals.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| `cmake` | every `-P` checker | ✓ | ≥ 3.25 (pinned at `lib.cmake:6`) | — |
| POSIX `sh` + `awk` + `grep` | every step | ✓ | system | — |
| `git` | `hygiene-history` step, `release-readiness`, every `FILELIST` build | ✓ | system | — |
| `ninja` + a C compiler | producer steps (`c-build`, `c-tests`, …) | ✓ | verified by the existing `build/dev/CMakeCache.txt` | — |
| `python3` | not used by the gate; used by this research for arithmetic | ✓ | system | `awk` |
| Network access to the libretro documentation | A7 only — the source record's `url`/`accessed` values | ✗ in this session | — | record the row with the reading deferred and A7 open; the gate itself never fetches |
| Hosted GitHub runner | the cost-checkpoint row (criterion 5) | ✓ | `gh` + `.planning/ci-runs.tsv` | a local `sh tools/gate.sh` run records no hosted row; the checkpoint would then be `not-measured` |

**Missing dependencies with no fallback:** none that block the gate. A7's network read blocks *one field set* of one data row, not the implementation.
**Missing dependencies with fallback:** the libretro documentation read (fallback: defer the row's values and keep A7 open).

## Validation Architecture

`workflow.nyquist_validation` is `true` (`.planning/config.json`), so this section is included.

### Test Framework

| Property | Value |
|----------|-------|
| Framework | The project's own gate harness: 38 POSIX `sh` steps + CMake `-P` checkers + 395 negative-control trees. No external test framework. |
| Config file | `tools/gate.sh` (runner), `data/expected/gate-steps/*.ids` (step inventory), `data/expected/controls/*.ids` (control inventory), `data/expected/*.ids` (per-suite baselines) |
| Quick run command | `git ls-files > /tmp/fl && ROOT="$PWD" CHECK_DIR="$PWD/tools/check" FILELIST=/tmp/fl sh tools/check/steps/<step>.sh` |
| Full suite command | `sh tools/gate.sh` (local fast path, same as the PR gate) |
| Controls only | `sh tools/check/run-controls.sh` |

Measured single-step runtimes this session (Darwin, no build): `important-gates` <1 s, `claims` <1 s, `manifests` <1 s, `sources` <1 s, `runner-selftest` ≈4 s (it runs the real control runner four times over a copied tree), `files` ≈2 s (runs `binscan.sh` over 2664 tracked files).

### Phase Requirements → Test Map

| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|--------------|
| RTS-01 | A row missing `disposition`/`source`/`taken` is refused by name | control | `sh tools/check/run-controls.sh` (control `rights-…`) | ❌ Wave 0 |
| RTS-01 | A `cleared` row with no resolvable source is refused | control | same (same directory, second `EXPECT` line) | ❌ Wave 0 |
| RTS-01 | The ten rows and `data/expected/rights.ids` agree in both directions | unit (step) | `sh tools/check/steps/manifests.sh` | ❌ Wave 0 |
| RTS-01 | The one live `cleared` row resolves, and its `SRC-` id is also resolved by the `sources` scan | unit (step) | `sh tools/check/steps/manifests.sh`; `sh tools/check/steps/sources.sh` | ❌ Wave 0 (needs R2's record) |
| RTS-02 | A refused spelling in `core/src`, `runner/`, `tools/` or a pinned page is refused | control | `sh tools/check/run-controls.sh` | ❌ Wave 0 |
| RTS-02 | A `(token, path)` pin suppresses a pre-existing hit and nothing else | control | same (same directory, `FORBID` the pinned finding) | ❌ Wave 0 |
| RTS-02 | A pin that no longer matches fails `rights-pinned-stale` | control | same (same directory, `EXPECT rights-pinned-stale`) | ❌ Wave 0 |
| EV-06 | An `established` claim with `tier != source-expectation` is refused | control | same | ❌ Wave 0 |
| EV-06 | An `established` claim with a non-`tier1-primary-doc` dependency is refused | control | same | ❌ Wave 0 |
| EV-06 | A legal `established` claim still passes (pristine `_good/claims`) and does not trip either token | unit (step) + control `FORBID` | `sh tools/check/steps/claims.sh`; `run-controls.sh` | ✅ the fixture exists; ❌ the FORBID control |
| EV-07 | Each of the ten mechanisms resolves to a live inventory id | unit (step) | `sh tools/check/steps/important-gates.sh` | ✅ after R1 (all ten resolve) |
| EV-07 | The four `mutant` ids are cases the selftest actually runs | unit (step) | `sh tools/check/steps/runner-selftest.sh` | ❌ Wave 0 (the inventory) |
| EV-07 | The derived accepted set covers every `Complete` requirement | unit (step) | `sh tools/check/steps/important-gates.sh` | ✅ |
| EV-07 | `faults` stays 238 and `steps` stays 38 | unit (step) | `sh tools/gate.sh` (summary line) | ✅ |
| cost | Last-five `cpu_s`/`gate_s` medians recomputed; verdict recorded | arithmetic (manual, from committed data) | `python3` over `.planning/ci-runs.tsv` | ✅ data; ❌ the recorded verdict |

### Sampling Rate
- **Per task commit:** the single affected step, invoked directly (all <2 s except `runner-selftest` ≈4 s).
- **Per wave merge:** `sh tools/check/run-controls.sh` (395 → 401 controls, 4 lanes) — the fastest way to see a control-overlay mistake, and the only way to see `control-toy-drift`.
- **Phase gate:** `sh tools/gate.sh`, full suite green, `steps=38/38`, `faults=238/238`, `goldens=29`, before `/gsd:verify-work`.

### Wave 0 Gaps
- [ ] `data/rights/*.json` (10 rows) — every rights id depends on these existing
- [ ] `data/expected/rights.ids`, `data/expected/rights-pinned.ids` — membership and the 4 pins
- [ ] `data/expected/runner-selftest.ids` — the 4 selftest case names (R1d)
- [ ] `tools/check/schema/rights.txt` + `tools/check/kinds/rights.cmake` (or the inline equivalent) — the checker
- [ ] `tools/check/controls/_good/tree/data/rights/*.json` + its `rights.ids` + `rights-pinned.ids` — the toy tree must pass the new checks
- [ ] `data/expected/controls/rights.ids` (the RTS controls) **and** `data/expected/controls/important-gates.ids` (the `important-gates-excluded-stale` directory) and `data/expected/controls/claims.ids` (the three EV-06 controls)
- [ ] `tools/check/controls/important-gates-excluded-stale/{CHECK,EXPECT,FORBID,overlay/…}` (R1g)
- [ ] `data/sources.json` + `data/expected/sources.ids` — one `reference-only` row (R2; A7 open)
- [ ] Framework install: none — the harness is the project's own.

## Security Domain

`workflow.security_enforcement` is `true`, `security_asvs_level` 1. This phase adds no network surface, no auth, no session, no cryptography and no new file-read outside the repository. It *is* itself a rights/refusal control, so the security domain is inverted: the risk is a control that can be satisfied vacuously.

### Applicable ASVS Categories

| ASVS Category | Applies | Standard Control |
|---------------|---------|-----------------|
| V2 Authentication | no | — |
| V3 Session Management | no | — |
| V4 Access Control | no | — |
| V5 Input Validation | yes | Every new checker validates untrusted-in-principle input (JSON rows, inventory lines, path lists) with fail-closed rules: `records.cmake`'s schema types, `check_shape`'s unknown-key refusal, `read_ids`' duplicate/empty refusal, a bounded-token scan that cannot be widened by a hostile spelling, and a two-field pin grammar that cannot become a wildcard. |
| V6 Cryptography | no | Only SHA-256 identity hashes, already handled by `data/sha256`/`manifest-lib.cmake`. Never hand-rolled in this phase. |

### Known Threat Patterns for this stack (CMake `-P` + POSIX `sh` checkers)

| Pattern | STRIDE | Standard Mitigation |
|---------|--------|---------------------|
| A control that passes vacuously (changes nothing, step still exits 0) | Repudiation | `run-controls.sh:219-221` refuses `control-exit-zero`; every control must make its step fail. |
| A blank `EXPECT` line (matches every output) | Repudiation | `run-controls.sh:205-217` refuses `control-expect-blank`. |
| A pin list that rots into a wildcard or a stale permission | Tampering | `split(l, w, " ") == 2` exact-pair grammar (`release-readiness.sh:57`) **and** the reverse direction as a failure (`rights-pinned-stale`, R3), plus `rights-pinned-malformed` for a line that is not exactly two fields. |
| A refused token that is really a substring of a legitimate identifier | Tampering | Bounded character class (Architecture Patterns → Pattern 3; Common Pitfalls → Pitfall 2). |
| A registry row that cites a requirement outside the accepted set | Repudiation | `important-gates.sh:173-175`. |
| A toy-registry edit that silently deletes coverage | Tampering | `run-controls.sh:300-326`, both directions, comments excluded. |
| A registry row whose id resolves only because it was hard-coded in the step | Repudiation | The new `mutant` ids come from a committed inventory that the selftest must actually run (R1f), so the registry cannot name a case that no longer exists. |
| A ledger row whose `disposition` is misspelled so no rule applies | Tampering | Fail-closed unknown-key refusal (schema `keys` list) — a misspelling is an unknown key, not a missing one. |
| A path-traversal spelling in a `MAKE` verb or an overlay | Elevation of privilege | `run-controls.sh:100-112` refuses `/*` and `*..*` in every `MAKE` path. |

## Sources

### Primary (HIGH confidence — read this session, line ranges quoted above)
- `tools/check/lib.cmake`, `tools/check/manifest-lib.cmake`, `tools/check/files.cmake`, `tools/check/manifests.cmake`, `tools/check/records.cmake`, `tools/check/schema/claim.txt`, `tools/check/schema/source.txt`, `tools/check/kinds/claim.cmake`, `tools/check/kinds/source.cmake`
- `tools/check/run-controls.sh`, `tools/check/pass-line.sh`
- `tools/check/steps/{files,manifests,claims,sources,important-gates,runner-selftest,text,ci-log,golden,faults-verdict,pkg-install}.sh`
- `tools/gate.sh`, `tools/check/integrity.cmake`, `tools/release-readiness.sh`
- `data/expected/{important-gates.ids,important-gates-excluded.ids,claims.ids,evidence-tiers.ids,source-authority-tags.ids,sources.ids,hygiene-history-pinned.ids,pkg-selftests.ids,gate-steps/rights.ids,gate-steps/claims.ids,controls/rights.ids,controls/important-gates.ids,controls/claims.ids}`
- `data/{claims.json,sources.json,tiers.json,profiles/intv-2609-ntsc-family-p1.json}`, `tools/check/binary-ext.txt`, `.gitignore`, `docs/RIGHTS.md`, `docs/owner-images.md`, `docs/provenance/CONSULTATIONS.md`
- `tools/check/controls/_good/tree/**` (124 files, enumerated), all ten `important-gates.ids` toy registries, `tools/check/controls/important-gates-complete-uncovered/**`, `tools/check/controls/important-gates-excluded-malformed/**`
- `.planning/{REQUIREMENTS.md,STATE.md,MILESTONE-CONTEXT.md,ci-runs.tsv,config.json}`, `.planning/ops/GUARDRAILS-v0.2.md`, `.planning/ops/GUARDRAILS-v0.3.md`
- `.planning/research/v0.3.0-scoping-2-gate-cost.md`, `-3-rights-format-packaging.md`, `-4-verification.md`
- `.planning/milestones/v0.2.0-phases/09-registered-evidence-and-gate-cost/{09-05-SUMMARY.md,09-VERIFICATION.md}`
- `.planning/phases/13-rights-ledger-and-enforced-claim-ceiling/13-CONTEXT.md` (the five resolutions, quoted into R1-R5)
- git: `16afb31` (TOOL-01 close — the 13-file/13-line toy diff and the 4-line header insert), `b14e1a4` (Phase 10 close)

### Secondary (MEDIUM confidence — executed commands, not files)
- `sh tools/check/steps/{important-gates,claims,manifests,files,sources,runner-selftest}.sh` against the real `FILELIST` (`git ls-files`, 2664 paths) → the measured baselines in "Common Pitfalls → Pitfall 8" and "Phase 13 Concrete Values → The measured baselines and the predicted deltas"
- `python3` arithmetic over `.planning/ci-runs.tsv` (96 data rows, 93 `success`) → "Phase 13 Concrete Values → The cost-checkpoint arithmetic"
- `grep -rilF` over the four naming scopes → the token-frequency table and the pin census in "Phase 13 Concrete Values → The ten rows…"
- a replica of `important-gates.sh:130-147`'s resolution loop over the live inventories → the truth table in Code Examples → "The ten mechanisms, resolved against the live inventories"
- `diff` over the ten toy registries and the two toy requirement records → the append shape in Resolved Decisions → R4
- `grep -rn "EV-98\|FND-96\|FND-99\|EV-99"` over the whole tree → the unused-id verification in R4a

### Tertiary (LOW confidence)
- A7 only: the new source record's `url`/`title`/`accessed` values. Everything else above is either read from a quoted file/line range or measured by a command run in this session.

## Metadata

**Confidence breakdown:**
- Standard stack / reused machinery: **HIGH** — every helper, token and line range was read verbatim; the counts were produced by running the real steps.
- Architecture (where each limb lands): **HIGH** for the mechanism, **MEDIUM** for the split (ledger in `manifests`, naming in `files`) — that is a recommendation, not a measured fact; the alternative (one new `rights.cmake` called from `steps/files.sh`) is equally legal.
- Resolved decisions (R1-R5): **HIGH** — each is reduced to quoted lines in named files, and the one resolution that needs an external reading (R2's record values) is isolated as A7.
- Pitfalls: **HIGH** — each is anchored to a specific guard in a specific file, and the token-frequency/`cleared`-row/`sources`-stat ones are measured.
- Cost arithmetic: **HIGH** — computed from `.planning/ci-runs.tsv` (96 data rows, 93 with `cpu_s`) and the frozen unit prices. The **six-directory ceiling is arithmetic, not advice**: a seventh directory takes 1.91 to 2.22 and breaks the 2.0 line.

**Research date:** 2026-11-26
**Valid until:** 2026-12-26 for the machinery (stable, and this milestone is the only thing changing it); recompute "Phase 13 Concrete Values → The cost-checkpoint arithmetic" and the `tracked=` figure at every phase boundary — the medians moved by 5 cpu-s within a day of the freeze, and `tracked=` moved by 2 between the previous draft of this file and this one.
