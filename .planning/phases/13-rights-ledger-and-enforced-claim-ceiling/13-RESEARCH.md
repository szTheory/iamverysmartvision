# Phase 13: Rights Ledger and Enforced Claim Ceiling — Research

**Researched:** 2026-11-26
**Domain:** The project's own gate machinery (POSIX `sh` steps + CMake `-P` checkers + inventory `.ids` files + negative-control trees). No third-party library, package or vendored content is involved.
**Confidence:** HIGH for every statement about existing machinery (each was read and, where a count is quoted, executed this session); MEDIUM for the two design resolutions marked in **Open Questions** (they are decisions, not facts).

## Summary

Phase 13 adds no new mechanism *kind*. Every limb it needs already exists in the gate and is exercised today by at least one of the 395 committed negative controls: a JSON-record checker driven by a schema file (`records.cmake` + `schema/<kind>.txt` + a hook in `kinds/`), a set-equality inventory check (`read_ids` + `<kind>-id-missing` / `-id-extra`), a whole-token scan with an anchored character class (`steps/sources.sh`), a `(token, detail)` pin list (`tools/release-readiness.sh`'s `history_compare`), and an `EXPECT`/`FORBID` control directory whose overlay is a whole-tree copy. Phase 13's three deliverables are therefore *data plus ids plus controls*, not code architecture: `data/rights/*.json` + `data/expected/rights.ids` + `data/expected/rights-pinned.ids`, two rules inside `tools/check/kinds/claim.cmake`, and ten `important-gates.ids` rows.

Four measured facts dominate the planning:

1. **Five of the ten D-09-06 mechanisms do not resolve in any registry inventory today.** I mirrored the resolution loop (`tools/check/steps/important-gates.sh:130-148`) over the live inventories: `important-gates-complete-uncovered`, `important-gates-excluded-malformed`, `sources-src-token-whole`, `manifests-output-anchor` and `memmap-generator-refused` resolve as `control`; the other five (`important-gates-excluded-stale`, `control-toy-drift`, `control-deviation-unused`, `control-pass-line-not-last`, `memmap-generator-missing`) resolve to **nothing**. Four of those five are already proved by `tools/check/steps/runner-selftest.sh` (which has **no inventory file**), and one (`important-gates-excluded-stale`) is proved only as a co-firing `EXPECT` line inside the `important-gates-complete-uncovered` control. A literal "ten `mutant` rows" reading therefore requires either inventing ten selftest cases or extending the mutant-resolution loop.
2. **A registry row's `<REQ>` column must name a requirement that is `Complete` and not excluded.** `important-gates.sh:172-174` fails a row whose 4th column is outside the derived accepted set. `RTS-01`, `RTS-02`, `EV-06` and `EV-07` are all `Pending` (`.planning/REQUIREMENTS.md:104-107`), so **the rows and the requirement-completion edits must land in the same commit** — exactly what `16afb31` (TOOL-01) and `b14e1a4` (PKG-05/TOOL-04/EV-05) did.
3. **The cost ceiling bounds the number of new control directories to six.** A control is priced at 0.31 hosted cpu-s (`MILESTONE-CONTEXT.md:227-228`); the phase's priced addition is 2.0. Six controls = 1.86, plus ten registry rows at 0.005 = 0.05, total 1.91 of 2.0. Seven controls overruns the line.
4. **The refused-token set cannot contain the hardware-concept words.** Measured over the four naming scopes, `exec` occurs in 98 files, `rom` in 182, `grom` in 29, `cfg` in 9. The tokens must be *artifact* spellings (`grom-image`, `exec-image`, `save-state`, `libretro`, …) and the match must be character-bounded, or the naming limb refuses the entire core.

**Primary recommendation:** implement the ledger as a fourth `records.cmake` kind (`kind=rights`, `schema/rights.txt`, `kinds/rights.cmake`) so the schema/enum/hook machinery is reused verbatim and `read_ids` gives three tokens for free; put the *naming* scan in `tools/check/files.cmake` (it is the only checker that already holds the sorted `FILELIST`) and the *ledger* check in `tools/check/manifests.cmake`; register the ten mechanisms as 5 `control` + 4 `mutant` (via a new `data/expected/runner-selftest.ids` read by the existing loop at `important-gates.sh:141-144`) + 1 new `control` for `important-gates-excluded-stale`, for ≤0.31 cpu-s of new cost; and close all four requirements in the same commits as their rows, re-deriving `lines=`/`gates=` by running the step rather than quoting a number.

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

### Claude's Discretion

None — no question was delegated to Claude; every question above was either frozen upstream or decided autonomously under the reversible-technical rule.

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
| RTS-01 | A rights ledger records one row per third-party-content question; each has a disposition of `cleared`/`refused`/`owner-local-only`, a `cleared` row cites a resolvable `SRC-` id, and a row missing its disposition, a source it needs, or its "what was taken" cell is refused by name. | Architecture Patterns → Pattern 1 (ledger as a `records.cmake` kind), Architecture Patterns → Pattern 2 (`read_ids` set equality), Architectural Responsibility Map (SRC resolution via `data/sources.json` membership), "Phase 13 Concrete Values → The ten rows: disposition, tokens, and the pin census" (exact new tokens), "Open Questions → 2" (measured `cleared`-row problem) |
| RTS-02 | A refused item's spellings cannot appear in `core/src`, `runner/`, `tools/` or the operator pages of `docs/`, except where a pre-existing refusal surface is pinned by a `(token, path)` pair; both limbs shown able to fail. | Architecture Patterns → Pattern 3 (the naming scan: character class, scopes, pin idiom with measured pre-existing hits), Architecture Patterns → Pattern 4 (`history_compare`'s pair grammar and its one-directional NOTE), "Phase 13 Concrete Values → The ten rows: disposition, tokens, and the pin census" (the 4 measured pins) |
| EV-06 | The claims step refuses an `established` claim that is not a `source-expectation` claim resting on a `tier1-primary-doc` dependency, and a legal `established` claim still passes — both shown able to fail (D-32, D-50). | Code Examples → "`claim.cmake` — the exact insertion point for D-50" (`claim.cmake`'s existing hook and the exact insertion point), Code Examples → "The toy tree already holds the legal positive" (the toy tree already holds a legal established claim — the positive exists), "Common Pitfalls → Pitfall 5" (why a "positive" cannot be a control directory) |
| EV-07 | Each of the ten D-09-06 mechanisms gains a `mutant` row over its existing selftest, with the registry baseline re-derived and all ten toy registries extended in the same commit, and zero new faults. | Code Examples → "The ten mechanisms, resolved against the live inventories" (measured resolution truth table: 5 resolve, 5 do not), "Common Pitfalls → Pitfall 1" (the requirement-column constraint and the same-commit precedent), "Phase 13 Concrete Values → The toy-registry arithmetic" (toy-registry arithmetic), "Phase 13 Concrete Values → The cost-checkpoint arithmetic" (cost arithmetic) |
</phase_requirements>

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Ledger row grammar, disposition enum, `taken` non-blank | `manifests` step (CMake `-P`) | — | `manifests.cmake` already loads a JSON collection from `FILELIST`, enforces a key set with `check_shape`, and does set-equality against an inventory (`manifests.cmake:453-464`). A ledger is the same shape of problem. |
| `cleared` → `SRC-` resolvability | `manifests` step | `data/sources.json` as data | Resolution is membership in the source registry; `schema/source.txt:20` pins `rights` to `^unknown$`, so the `rights` field cannot answer it. |
| Refused-token naming scan | `files` step (CMake `-P`) | — | `files.cmake` is the only checker that already holds the sorted tracked-path list (`files.cmake:98-108`) and the four scopes are path prefixes. |
| `(token, path)` pin resolution | `files` step | `data/expected/rights-pinned.ids` as data | Mirrors `tools/release-readiness.sh:47-84`, which reads its pin list as data and never widens a rule. |
| Enforced claim ceiling | `claims` step | `tools/check/kinds/claim.cmake` hook | The hook already resolves `depends_on` to profile field tags (`claim.cmake:157-177`); D-50 is two more conditions in the same loop. |
| Ten mechanism registrations | `important-gates` step | `data/expected/important-gates.ids` | The step already resolves `fault|control|mutant` against three inventories (`important-gates.sh:130-148`); no new kind. |
| Operator-facing rights table | `docs/RIGHTS.md` (documentation) | `text` step's claim-word tripwire | The page is prose, so it is governed by `steps/text.sh:4-16`, not by a checker of its own. |
| Cost checkpoint | `.planning/ci-runs.tsv` (data) | `ci-log` step's format check | Arithmetic over a committed file; `steps/ci-log.sh:7-14` is the only gate over its shape. |

## Standard Stack

There is **no package installation in this phase.** No `npm`/`pip`/`cargo`/`FetchContent`/vendored header. Everything used already exists in the tree. The `## Package Legitimacy Audit` section is therefore omitted by its own trigger condition ("whenever this phase installs external packages") — verified: `data/rights/*` is JSON, the checkers are CMake `-P` scripts, the controls are directories, and D-48 forbids vendoring a libretro header.

### Core (already present, reused verbatim)

| Asset | Location | Purpose in this phase | Why it is the right one |
|-------|----------|----------------------|-------------------------|
| `lib.cmake` | `tools/check/lib.cmake:1-152` | `gate_fail` (`:8-11`), `gate_stat` (`:13-16`), `gate_finish` (`:35-46`), `json_type` (`:64-73`), `read_ids` (`:75-116`) | Every checker's failure/pass-line contract. `read_ids` gives `rights-inventory-missing`, `-empty`, `-duplicate` for free. |
| `manifest-lib.cmake` | `tools/check/manifest-lib.cmake:1-216` | `manifest_in_filelist` (`:18-28`) for "is this path tracked" | CONTEXT's locked choice; it is the only shared helper that answers a `FILELIST` membership question. |
| `records.cmake` + `schema/<kind>.txt` + `kinds/<kind>.cmake` | `tools/check/records.cmake:1-…` | The ledger can be a fourth record kind | The schema language already expresses `file`, `topkeys`, `array`, `idkey`, `inventory`, `keys`, `req`, `nonblank`, `match`, `enumfrom`, `when`, `hook` (`schema/claim.txt:1-21`). |
| `run-controls.sh` | `tools/check/run-controls.sh:1-377` | Declares and runs the new controls | `CHECK`/`EXPECT`/`FORBID`/`overlay/`/`MAKE`/`DEVIATION` (`:1-20`, `:198-250`); final line `controls=<ok>/<total>` (`:375`). |
| `steps/important-gates.sh` | `tools/check/steps/important-gates.sh:1-197` | The ten rows' consumer | `fault|control|mutant` resolution (`:130-148`), the derived accepted set (`:93-131`) and the row judgement (`:160-190`); the `mutant` definition is documented at `:61-63`. |
| `steps/sources.sh` | `tools/check/steps/sources.sh:1-57` | The model for a whole-token scan | Its measured character class and boundary reasoning (`:36-51`) is the exact pitfall the naming scan must avoid. |
| `tools/gate.sh` | `tools/gate.sh:1-250` | Step inventory and summary line | `steps.have`/`steps.want` (`:97-115`), summary (`:241-249`). |

### Alternatives Considered

| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| A fourth `records.cmake` kind for the ledger | A bespoke `rights.cmake` with hand-written key checks | Bespoke duplicates `check_shape`/`enumfrom`/`nonblank` and re-introduces the "misspelled key is not a missing key" bug that `manifests.cmake`'s fail-closed shape check exists to prevent. Only worth it if the ledger needs a *cross-file* rule the schema language cannot express (e.g. "no two rows share a token"), which the `hook` function can express anyway. |
| Naming scan in `files.cmake` | A new `rights.cmake` invoked from `steps/files.sh` | Both are legal (neither adds a gate step). A separate `.cmake` keeps `files.cmake` readable but needs its own `include(lib.cmake)` and its own pass-line aggregation; inlining reuses `files.cmake`'s single `gate_finish(files)`. |
| `mutant` rows for all ten | `control` rows where a control directory already exists (5 of 10) | A `control` row costs nothing and resolves today; a `mutant` row for the same mechanism would duplicate a proof that already exists. See Open Question 1. |
| A new `runner-selftest.ids` inventory | Hard-code the four selftest case names in the registry's `known` set | Hard-coding breaks the step's own documented invariant ("a `mutant` is a case of …", `important-gates.sh:60-62`) and would let the registry name a case the selftest no longer runs. |

**Installation:** none.

**Version verification:** not applicable — no external dependency. The tool versions that matter are already pinned by the environment: CMake ≥ 3.25 (`lib.cmake:5`), POSIX `sh`, and the project's own `tools/gate.sh`.

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
        ▼                                                           │
  FAIL rights-token: <token> <path>                                 │
                                                                    │
  data/claims.json ──► claims step ──► kinds/claim.cmake hook       │
  data/profiles/<p>.json (field tags)      │                        │
        │                                  ▼                        │
        │                     FAIL claim-established-non-source: <id>│
        │                     FAIL claim-established-without-tier1: <id>
        │                                                           │
  data/expected/important-gates.ids (337 lines) ──► important-gates │
        │   resolves each row against faults/controls/selftests     │
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
│   ├── libretro-abi.json
│   └── core-info-strings.json
└── expected/
    ├── rights.ids                   # ledger membership (new)
    ├── rights-pinned.ids            # (token, path) pins (new)
    └── runner-selftest.ids          # optional: the 4 selftest case names (see OQ1)

tools/check/
├── schema/rights.txt                # only if the ledger is a records.cmake kind
├── kinds/rights.cmake               # ditto
└── controls/                        # ≤6 new control dirs (cost bound, "Phase 13 Concrete Values → The cost-checkpoint arithmetic")
    ├── rights-…/CHECK|EXPECT|overlay/…
    └── claim-established-…/CHECK|EXPECT|overlay/…
```

### Pattern 1: `records.cmake` kind + schema + hook

**What:** a JSON collection is checked by a declarative schema file plus an optional hook function; the inventory file gives set equality and three free tokens.
**When to use:** for the ledger row grammar. `schema/claim.txt` is the closest model (id regex, enum-from-another-file, `nonblank`, `when`).
**Example:**
```
# Source: tools/check/schema/claim.txt:1-21 (verbatim shape, annotated)
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
**When to use:** `data/expected/rights.ids` ↔ `data/rights/*.json`.
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
Note the token is assembled as `<kind>-id-missing`; `read_ids`'s own tokens are `<kind>-inventory-missing|empty|duplicate` (`lib.cmake:79`, `:104`, and the duplicate branch below it).

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

### Pattern 4: `(token, detail)` pin list, never a token alone

**What:** a finding is paired with its detail and compared as a 2-field key; an unpinned pair is new, a pinned pair that no longer occurs is a NOTE (not a failure).
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
`split(l, w, " ") == 2` is the grammar: exactly two whitespace-separated fields, so a pin line cannot rot into a wildcard. The reverse direction is a `NOTE`, deliberately *not* a failure — a rights pin should decide whether it wants the same leniency (see Architecture Patterns → Pattern 4).

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
Two `EXPECT` tokens in one control are legal and normal; `FORBID` is how a control says "this other refusal must *not* be how it failed" (`run-controls.sh:227-250`).

### Anti-Patterns to Avoid

- **A `docs/**` glob for the naming scope.** The CONTEXT pinned six pages deliberately. A glob would make the operator pages and `docs/provenance/`, `docs/stic/`, `docs/video/` (reference notes, which legitimately quote sources) indistinguishable, and every new doc file would silently widen the rule.
- **A token-only pin list.** `hygiene-history-pinned.ids` is a pair list because a token alone matches every instance; the same failure mode applies to `rights-pinned.ids`.
- **A global `grep` for `cfg`/`ivsc`.** Both are *refusal surfaces* in `tools/check/binary-ext.txt:8,5` and `.gitignore:7,9`; removing either would delete a refusal. Measured: `cfg` occurs in 9 files across the four scopes.
- **Bare `exec`, `rom`, `grom` as refused tokens.** Measured file counts in the four scopes: `exec` 98, `rom` 182, `grom` 29. They are hardware/API vocabulary (`ivs_exec`, `memsys_grom_read`, …), not artifact names.
- **Editing `CL-0002` to test the ceiling.** CONTEXT locks the positive to a toy row; `CL-0002` is asserted unchanged so `data/claims.json` and `data/expected/claims.ids` do not move for a test.
- **Quoting `lines=337` / `gates=110`.** Both are runtime stats (`important-gates.sh:188`). Re-derive by running the step; the numbers in `09-05-SUMMARY.md` and `MILESTONE-CONTEXT.md` are already stale in places (see "Phase 13 Concrete Values → The cost-checkpoint arithmetic").

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| JSON key presence/type/enum checking | A hand-written key loop in a new `rights.cmake` | `records.cmake` + `schema/rights.txt` (`check_shape` analogue at `manifests.cmake:41-66`) | The existing shape check is fail-closed on a *misspelled* key (it is an unknown key, not a missing one) — a bespoke loop gets this wrong once and the ledger silently accepts `dispositon`. |
| Inventory membership + duplicates | `sort | uniq -d` in `sh` | `read_ids` (`lib.cmake:75-118`) | It already fails on missing file, empty file (unless `#allow-empty:`), and duplicates, with the token names `<kind>-inventory-*`. |
| A pass line | `printf` from the step | `gate_finish` (`lib.cmake:35-47`) | It collects every `gate_fail` and emits exactly one `FATAL_ERROR`, which is what `pass-line.sh` and `run-controls.sh:175-196` require. |
| Negative-control plumbing | A bespoke test script | `run-controls.sh` + a control dir | `CHECK`/`EXPECT`/`FORBID`/`overlay`/`MAKE` plus the inventory both-directions check (`:280-298`) and the toy-registry drift check (`:300-325`). |
| Token extraction with boundaries | `grep -w` | The measured `grep -o -E '[class]+'` idiom (`sources.sh:44-48`) | `grep -w` treats `-` as a word separator, so `save-state` would match inside `save state` and `rom-image` would match inside `grom-image`; the class must include `-` on both sides. |
| Cost arithmetic | A spreadsheet or a quoted median | `python3`/`awk` over `.planning/ci-runs.tsv` | AGENTS.md: "Recompute, never quote." "Phase 13 Concrete Values → The cost-checkpoint arithmetic" shows the frozen figure is already 5 cpu-s off the current file. |

**Key insight:** every limb of this phase is a *data-plus-ids* problem whose machinery already exists and is already covered by 395 controls. The only genuinely new code is the bounded-token scan and two conditions in `claim.cmake`; everything else is a schema file, a `.ids` file, a JSON row, or a control directory.

## Runtime State Inventory

Phase 13 is not a rename, rebrand or migration; it introduces new data and new ids. The inventory below answers the canonical question anyway, because the phase does introduce a persisted schema and a naming rule that later phases key off.

| Category | Items Found | Action Required |
|----------|-------------|------------------|
| Stored data | None — `data/rights/` does not exist yet (verified `ls -d data/rights` → not found). `data/claims.json` holds 28 claims, of which 1 is `established` (measured). | Create `data/rights/*.json`; no migration. |
| Live service config | None — no external service, no hosted state. The only hosted artifact is `.planning/ci-runs.tsv`, which is append-only and already committed (96 rows). | Append one row after the content lands (success criterion 5). |
| OS-registered state | None — no scheduler, launchd, systemd or pm2 unit references the gate. | None. |
| Secrets / env vars | None added. The gate reads `ROOT`, `CHECK_DIR`, `FILELIST`, `IVS_GATE_DIR`, `CONTROLS_DIR`, `CONTROLS_INVENTORY_DIR`; all are set by `tools/gate.sh` or `run-controls.sh` and none changes. | None. |
| Build artifacts / installed packages | `build/` is gitignored and regenerated. No installed package changes. | None. |
| Control trees (the real "runtime state" here) | `tools/check/controls/_good/tree/` (124 files) is a *committed* tree that every control copies. It has no `data/rights/` and none of the six pinned operator pages. `tools/check/controls/important-gates-complete-uncovered/overlay/.planning/REQUIREMENTS.md` is a whole-file copy of the toy requirement record and must be updated whenever the toy record gains a row. | Add the toy ledger + inventories + a toy operator page (or make the page list tolerate absence); update the overlay copy in the same commit as any toy-record row. |

**Nothing found in the first five categories** — verified by the commands above and by grepping the gate for `IVS_`/`ROOT`/`CHECK_DIR` usage.

## Common Pitfalls

### Pitfall 1: The registry row's requirement column is not free text
**What goes wrong:** a row `rights-ledger control rights-… RTS-01` is written while `RTS-01` is still `Pending`. The step fails `important-gates-unknown-id: line <n>` and the gate goes red on the commit that was supposed to add coverage.
**Why it happens:** `important-gates.sh:173-176` checks `!($4 in accepted)` where `accepted = Complete − excluded`, derived live from `.planning/REQUIREMENTS.md` and `.planning/milestones/*-REQUIREMENTS.md` (`:93-131`). `RTS-01/02`, `EV-06/07` are `Pending` today (`.planning/REQUIREMENTS.md:104-107`).
**How to avoid:** flip the requirement's checkbox line *and* its traceability row to `Complete` in the same commit as the rows that name it — the `16afb31` (TOOL-01) and `b14e1a4` (PKG-05/TOOL-04/EV-05) precedent.
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
**Why it happens:** `steps/text.sh:4-16,36-46` scans `README.md`, `CHANGELOG.md` and `docs/**/*.md` with `grep -w -i`.
**How to avoid:** phrase the rights rows as dispositions ("may be named", "may not be committed"), never as behaviour claims; add `[nc]` where a claim word is unavoidable. The `playable-guest-progress` identifier is the only exempted string.
**Warning signs:** `text-claim-untagged: docs/RIGHTS.md:<line>`.

### Pitfall 5: A control directory can never express "this passes"
**What goes wrong:** `claim-established-positive` is created as a control whose overlay adds a legal established claim, expecting the step to pass. `run-controls.sh:219-221` fails it as `control-exit-zero`.
**Why it happens:** `ctl_one` requires a non-zero step exit by construction; only `good_one` (`:175-196`) runs a pristine pass.
**How to avoid:** express the positive as `EXPECT <some other refusal> + FORBID claim-established-non-source claim-established-without-tier1`, and rely on the pristine `_good/claims` run (already produced because `claims` is an existing `CHECK`) for the plain pass. See "Common Pitfalls → Pitfall 5".
**Warning signs:** `control-exit-zero: claim-established-positive`.

### Pitfall 6: New `.cmake`/`.sh` under `tools/` needs the SPDX line; new JSON under `data/` must be ASCII
**What goes wrong:** a new `tools/check/rights.cmake` without `# SPDX-License-Identifier: MIT` fails `integrity-missing-spdx`; a non-ASCII character (an em dash, a curly quote) in a `data/rights/*.json` fails `integrity-non-ascii`.
**Why it happens:** `tools/check/integrity.cmake:60-79`.
**How to avoid:** copy the two-line header from any existing checker; keep ledger `note`/`item`/`taken` values plain ASCII.
**Warning signs:** `integrity-missing-spdx: tools/check/<new>.cmake`, `integrity-non-ascii: data/rights/<slug>.json`.

### Pitfall 7: A toy registry overlay replaces the whole file, so drift is checked against `_good`
**What goes wrong:** a toy registry line is added to `_good/tree/...` but not to all nine overlays (or vice versa). `run-controls.sh:300-325` reports `control-toy-drift` (undeclared difference) or `control-deviation-unused` (a DEVIATION line matching nothing).
**Why it happens:** each of the nine overlays is a *whole-file copy*; the comparison excludes comments only.
**How to avoid:** one line appended to all ten files in the same commit, zero deletions (CONTEXT), and no `DEVIATION` entry — a DEVIATION entry would be the wrong tool because the addition is intended, not a control-local difference.
**Warning signs:** `control-toy-drift: <control>: > toy-…` or `control-deviation-unused`.

### Pitfall 8: `files`'s pass-line stats change, and that is the token delta
**What goes wrong:** the predicted token delta is stated as "none", then the measured summary line differs because `files` gained a `gate_stat`.
**Why it happens:** `tools/gate.sh:184-186` concatenates every `-- <step> pass <stats>` payload into the summary token line; `files.cmake:173-174` currently emits `tracked=… binaries-listed=…`.
**How to avoid:** predict the delta explicitly. Measured baselines this session: `files pass tracked=2662 binaries-listed=0`, `manifests pass manifests=11 effective-yes=8 effective-no=3`, `claims pass claims=28 provisional=27`, `important-gates pass gates=110 lines=337`, `sources pass sources=12 refs-verified=3/4`.
**Warning signs:** a cost-checkpoint row whose token line does not match the prediction in the plan.

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
So `steps=38/38` is `passed/want`, `controls=N/N` is `run-controls.sh`'s last line (`:369`), and `faults=238`/`goldens=29` are the `-- faults-verdict pass faults=<ok>/<total>` (`steps/faults-verdict.sh:191`) and `-- golden pass goldens=<n> …` (`steps/golden.sh:450`) payloads. Measured invariants: `faults` = 238 non-comment lines across `data/expected/faults/*.ids`; `goldens` = 29 tracked files directly under `data/golden/` (`git ls-files data/golden | wc -l` → 29).

### The ten mechanisms, resolved against the live inventories (measured)

| # | Mechanism | Resolves as | Where it is proved |
|---|-----------|-------------|--------------------|
| 1 | `important-gates-complete-uncovered` | `control` | `tools/check/controls/important-gates-complete-uncovered/` |
| 2 | `important-gates-excluded-stale` | **nothing** | only as a co-firing `EXPECT` line in control #1 |
| 3 | `important-gates-excluded-malformed` | `control` | `tools/check/controls/important-gates-excluded-malformed/` |
| 4 | `control-toy-drift` | **nothing** | `steps/runner-selftest.sh:96-100` builds case `st-toy-drift`; `:117-131` asserts the token |
| 5 | `control-deviation-unused` | **nothing** | `steps/runner-selftest.sh:103-108` builds case `st-deviation-unused`; `:117-131` asserts the token |
| 6 | `control-pass-line-not-last` | **nothing** | `steps/runner-selftest.sh:139-180` (copied runner + `pass-line.sh`) |
| 7 | `sources-src-token-whole` | `control` | `tools/check/controls/sources-src-token-whole/` |
| 8 | `manifests-output-anchor` | `control` | `tools/check/controls/manifests-output-anchor/` |
| 9 | `memmap-generator-refused` | `control` | `tools/check/controls/memmap-generator-refused/` |
| 10 | `memmap-generator-missing` | **nothing** | `steps/runner-selftest.sh:205-226` |

The five `control` rows cost **zero** new cpu-s and need no new file. The four selftest-only ones need an inventory: `data/expected/runner-selftest.ids` plus one added glob in `important-gates.sh:141-144`, and `runner-selftest.sh` extended to read that inventory and fail on a name it does not know (the `pkg-install` idiom, `steps/pkg-install.sh:831-848`). That is also **zero** new runtime cost, because those cases already run every gate. Mechanism #2 needs one of the options in Open Question 1.

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
D-50 adds, in this same function: (a) `standing STREQUAL "established" AND NOT tier STREQUAL "source-expectation"` → a new token; (b) inside the loop, an established claim whose field tag is not `tier1-primary-doc` → a second new token. Note `:148-155` returns early when `depends_on` is absent or empty, but `schema/claim.txt:21` already refuses that for an established claim (`when standing=established nonblank depends_on`), so the early return cannot make (b) vacuous. The existing weak-field rule stays: D-50 is strictly stronger, so `claim-overstated` remains reachable for a *provisional* claim? No — `:175` also requires `established`, so after D-50 `claim-overstated` becomes unreachable for the established case it used to cover. **Decide deliberately** whether `claim-overstated` is subsumed (keep both, the new one firing first is fine because both are findings) or retargeted. The existing control `claims-overstated-standing` overlays the toy `CL-0001` and expects `FAIL claim-overstated: CL-0001`; if D-50's new tokens fire on the same overlay the control still passes (EXPECT is a substring match, not an exclusive one) — but the *ordering* changes which line appears first, which `control-pass-line-not-last` does not care about and `EXPECT` does not either. Safe.

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

Dispositions are a **recommendation** (the ledger's content is RTS-01's deliverable, not a research finding). Tokens are chosen so that each is a *character-bounded* artifact spelling, per Pitfall 2.

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
| `libretro-abi` | `cleared` or `refused` — see Open Question 2 | `libretro` | 0 |
| `core-info-strings` | `cleared` or `owner-local-only` — see Open Question 2 | `core-info`, `retro_get_system_info` | 0 |

**The pin census (measured).** Only two spellings have pre-existing hits, and they are the same four lines:

```
tools/owner-run.sh:7      # Usage: ... <exec-image> <grom-image>
tools/owner-run.sh:18     usage='usage: owner-run.sh ... <exec-image> <grom-image>'
docs/owner-images.md:47   `sh tools/owner-run.sh ... <exec-image> <grom-image>` does the three steps ...
docs/owner-images.md:64   starts `ivs-run --exec <exec-image> --grom <grom-image>` on that copy ...
```

A pin is a `(token, path)` pair, so this is exactly **4 pins** (`exec-image tools/owner-run.sh`, `exec-image docs/owner-images.md`, `grom-image tools/owner-run.sh`, `grom-image docs/owner-images.md`) regardless of how many occurrences each path holds. These are the operator's own argument placeholders, so pinning them is correct rather than convenient: the names *are* the operator interface.

### The toy-registry arithmetic

**(measured)** The toy tree's two requirement records hold 46 rows each and the toy registry holds 46 non-comment lines; the live accepted set is also 46 (58 `Complete` − 12 excluded). The toy record is *not* an automatic mirror: it is `live − EV-05 + FND-98` (measured id diff), which is why `b14e1a4` (Phase 10) touched no toy file while `16afb31` (TOOL-01) touched all thirteen (10 registries + 2 records + 1 overlay copy).

So "all ten toy registries gain exactly one line each" is mechanically: **one** new toy requirement row appended to

1. `tools/check/controls/_good/tree/.planning/REQUIREMENTS.md`,
2. `tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md`,
3. `tools/check/controls/important-gates-complete-uncovered/overlay/.planning/REQUIREMENTS.md` (the whole-file copy), and

one toy registry line for that id appended to each of the ten whole-file toy registries:

4. `tools/check/controls/_good/tree/data/expected/important-gates.ids`,
5. `…/important-gates-bad-req-phase5/overlay/data/expected/important-gates.ids`,
6. `…/important-gates-bad-req-phase6/overlay/…`,
7. `…/important-gates-req-uncovered-phase5/overlay/…`,
8. `…/important-gates-req-uncovered-phase6/overlay/…`,
9. `…/important-gates-req-uncovered-phase7/overlay/…`,
10. `…/important-gates-uncovered-gate/overlay/…`,
11. `…/important-gates-unknown-id/overlay/…`,
12. `…/important-gates-unknown-id-phase5/overlay/…`,
13. `…/important-gates-unknown-id-phase6/overlay/…`.

That is 13 files, 13 added lines, zero deletions — exactly the `16afb31` diff shape (`16 files changed, 32 insertions, 2 deletions`, of which 13 files/13 lines are the toy edits and 2 deletions are the two requirement-status flips). **Do not** add a `DEVIATION` file: the addition is intended in all ten, and a `DEVIATION` entry would trip `control-deviation-unused`.

### The cost-checkpoint arithmetic

**(measured, 2026-11-26, from `.planning/ci-runs.tsv`)** 96 rows, 93 `success` rows with a real `cpu_s`. Last five rows in file order:

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
| new control directories | 0.31 each | ≤6 | ≤1.86 |
| `important-gates.ids` rows | 0.005 each | 10 | 0.05 |
| new `runner-selftest.ids` cases | 0.05 each (already running) | 0 | 0.00 |
| new faults | 0.44 each — **forbidden** | 0 | 0.00 |
| **total** | | | **≤1.91 of 2.0** |

Verdict rule (`scoping-2-gate-cost.md:59-64`): `discharged-within-allowance` if median `cpu_s` ≤ 361 + priced addition and the cumulative spend ≤ 21; `reprice` if the median moved more than the priced addition with an unchanged token line; `lever-required` if the cumulative would pass 21 or a phase overruns its line by >25 %. **The measured median is already 366 = 361 + 5, which is more than this phase's 2.0 priced addition with the token line unchanged** — so unless the token line moves by the predicted amount, the honest verdict for this phase's checkpoint is `reprice`, not `discharged-within-allowance`. State this explicitly in the plan's checkpoint task instead of discovering it at the boundary.

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| A hand-written `nreq` split string listing accepted requirements in the step | The accepted set derived live from the requirement records minus an exclusion ledger | plan 09-05 (EV-04, amendment A-01) | A new requirement needs no step edit; it needs the record flip plus rows. Confirmed: no `nreq` in `important-gates.sh`. |
| `r` (ratio) as the cost decision input | `cpu_s` medians, arithmetic over `ci-runs.tsv` | v0.2.0 close, audit G-04 | "Phase 13 Concrete Values → The cost-checkpoint arithmetic": the frozen `cpu_s` 361 is now 366. |
| A registry row only for a requirement being *accepted* | A registry row also for a mechanism that had a guard and a stand-in but no row (D-09-06) | this phase | Ten rows, zero new faults. |
| One `docs/**`-wide rule for rights | Token + scope + pinned pairs | D-46 / R-1 | `cfg`/`ivsc` refusal surfaces survive. |

**Deprecated/outdated:**
- `.planning/research/v0.3.0-scoping-2-gate-cost.md`'s last-five medians (`cpu_s` 361, `build_s` 34, `gate_s` 118) are stale by measurement; the checkpoint must recompute ("Phase 13 Concrete Values → The cost-checkpoint arithmetic").
- The `R2` row of `GUARDRAILS-v0.2.md:21` names an `nreq` split string that no longer exists (the file itself records the amendment at `:32-34`). Its remaining limbs — the registry file and all ten toy registries — are live.

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | The ledger's `tokens` field is an array of strings and `taken` is a string (`"nothing"` legal) — the CONTEXT lists keys but not their JSON types. | Architecture Patterns → Pattern 1 | A wrong type means the schema file is wrong and every row must be rewritten; cheap to fix before rows land, expensive after. |
| A2 | The naming scan's scope is *path prefix* `core/src/`, `runner/`, `tools/` plus the six pinned pages, applied over `FILELIST` (so control-tree overlays are never scanned on the real tree). | Architecture Patterns → Pattern 3 | If the scope were a filesystem walk, control overlays would be scanned and every planted-token control would refuse the real tree. `sources.sh:5-6` documents exactly this anchoring decision for its own scan, so the precedent is strong — but it is an inference for the rights scan. |
| A3 | The ten mechanisms' registry `<gate>` names are free choices (e.g. one new `rights-ledger` name, one `claim-ceiling`, one `d09-06-registrations`). | "Common Pitfalls → Pitfall 1" | Only affects `gates=`; `lines=` is unaffected. Low risk. |
| A4 | "All ten toy registries gain exactly one line each" means one toy *requirement* row added to both toy records (+ the overlay copy) and therefore one toy registry line per file — the `16afb31` shape. | "Phase 13 Concrete Values → The toy-registry arithmetic" | If it means one line per *closed requirement* (four), each registry gains four lines and the diff is 40 lines, not 10. The commit message and the arithmetic must state which. |
| A5 | The phase closes all four of its requirements, so `accepted` moves 46 → 50. | "Common Pitfalls → Pitfall 1" | If a requirement is instead excluded, an exclusion row is needed and the reason must have ≥3 underscore-joined words (`important-gates.sh:106-124`). |
| A6 | No live `cleared` row can be produced without either a new `SRC-` record or citing an existing one. | "Open Questions → 2" | This is the phase's most consequential unresolved decision; see Open Question 2. |

## Open Questions

### 1. Must all ten rows be `mutant`, or is `control` acceptable for the five that already have control directories?

- **What we know (measured):** five of the ten resolve as `control` today (they have control directories and inventory ids); four resolve as nothing but are proved by `runner-selftest.sh` cases; one (`important-gates-excluded-stale`) resolves as nothing and is proved only as a co-firing `EXPECT` line.
- **What's unclear:** the frozen text (GUARDRAILS V11, EV-07, scoping-4) says "`mutant` row over its existing selftest". For five of the ten, the existing selftest *is* a control directory, and a `control` row is the honest, zero-cost registration. A literal ten-`mutant` reading requires inventing five selftest cases that re-prove what the controls already prove.
- **Recommendation:** register 5 as `control` (zero cost), 4 as `mutant` via a new `data/expected/runner-selftest.ids` + one glob added at `important-gates.sh:141-144`, and mechanism #2 as a new `control` directory `important-gates-excluded-stale` (0.31 cpu-s) whose overlay puts a non-Complete requirement in the toy exclusion ledger. Total new cost ≤0.31 cpu-s, well inside the 2.0 line. State the deviation in the plan and the summary, with the reason (a `mutant` row for a mechanism that already has a control directory would be a second registration of one proof). If the owner wants the literal reading, the alternative is ten `runner-selftest.ids` cases — more code, no more evidence.

### 2. How is a legal `cleared` row produced, given that `cleared` requires a resolvable `SRC-` id?

- **What we know:** `cleared` ⇒ `source` must be a member of `data/expected/sources.ids` (12 ids, measured). `schema/source.txt:20` pins `rights` to `^unknown$` for every source, which is exactly why CONTEXT forbids consulting it. The ten items are: an operator's flat image, `.cfg`, bank modes, EXEC, GROM, a commercial ROM, a rendered frame, a save state, the libretro-shaped ABI, core-info strings.
- **What's unclear:** which of the ten can be `cleared` at all. `cfg`/bank-modes are `refused` (FMT-01/FMT-04 deferred). EXEC/GROM/flat-image/commercial-ROM/save-state are `owner-local-only`. `rendered-frame` is `refused` (a render of a scan is derivative). `libretro-abi` is project-authored under D-48 — nothing vendored, so nothing to cite — yet "cleared" is the only disposition that means "may be named and shipped". `core-info-strings` are project-authored strings, same problem.
- **Options:** (a) add one `reference-only` source record (e.g. the libretro API documentation) to `data/sources.json` + `sources.ids`, which is a legal registry row (`capture: reference-only`, `rights: unknown`, and the `sources-reference-only-unexplained` rule must be satisfied) and makes `libretro-abi` `cleared` — but it changes `sources`' stats (`sources=12 → 13`, `refs-verified=3/4 → 3/4` or `3/5`) and adds a registry row; (b) accept zero live `cleared` rows and let the controls be the only exercise of that branch — legal under criterion 1 but leaves a branch whose failure is only ever seen by a control; (c) make `core-info-strings` `cleared` citing an existing `SRC-` that genuinely covers the strings (none does today).
- **Recommendation:** (a) for exactly one row, so the live tree exercises the `cleared` path and the resolvability rule is not dead code. Price it: one `sources` record (0.005 cpu-s as a registry row) plus the `sources` step's stat delta, which is a token-line change to predict.

### 3. Does the pin list's reverse direction fail or NOTE?

- **What we know:** `history_compare` prints a NOTE, not a failure, for a pinned pair that no longer occurs (`release-readiness.sh:80`). That is correct for history (a pin names a commit; the commit cannot disappear).
- **What's unclear:** for rights, a pin names a `(token, path)` that exists *now*. If the path is deleted, the pin is stale and the rule has silently widened.
- **Recommendation:** make the reverse direction a *failure* (`rights-pinned-stale`) so a pin cannot rot — but note that this makes the pin list a second inventory and costs one more `EXPECT` line in a control rather than a new control. Cheap. Decide before writing the check.

### 4. Is `lines=` a header or a runtime stat?

- **What we know:** `lines=` is computed at runtime (`important-gates.sh:172,188`) and appears in the step's pass line. It is not stored in `important-gates.ids`'s header (that header is prose, lines 1-9).
- **What's unclear:** CONTEXT says "the `lines=` header of `data/expected/important-gates.ids` is re-derived". Since there is no header field, the re-derivation is (i) run the step and record the printed stat, and (ii) update the prose counts in the step's own header comment (`important-gates.sh:38-56`) and in the plan/summary.
- **Recommendation:** treat "baseline re-derived" as: run `sh tools/check/steps/important-gates.sh` with the real `FILELIST` and quote the printed `gates=`/`lines=`; add the Phase 13 line to the step header's phase table (`:38-56`) in the same commit, exactly as `16afb31` did ("the R2 mechanism retarget: a Phase 11 line in the step header", 4 lines added to `important-gates.sh`).

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| `cmake` | every `-P` checker | ✓ | ≥ 3.25 (pinned at `lib.cmake:5`) | — |
| POSIX `sh` + `awk` + `grep` | every step | ✓ | system | — |
| `git` | `hygiene-history` step, `release-readiness` | ✓ | system | — |
| `ninja` + a C compiler | producer steps (`c-build`, `c-tests`, …) | ✓ | verified by the existing `build/dev/CMakeCache.txt` | — |
| `python3` | not used by the gate; used by this research for arithmetic | ✓ | system | `awk` |
| Hosted GitHub runner | the cost-checkpoint row (criterion 5) | ✓ | `gh` + `.planning/ci-runs.tsv` | a local `sh tools/gate.sh` run records no hosted row; the checkpoint would then be `not-measured` |

**Missing dependencies with no fallback:** none.
**Missing dependencies with fallback:** none.

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

Measured single-step runtimes this session (Darwin, no build): `important-gates` <1 s, `claims` <1 s, `manifests` <1 s, `sources` <1 s, `files` ≈2 s (runs `binscan.sh` over 2662 tracked files).

### Phase Requirements → Test Map

| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|--------------|
| RTS-01 | A row missing `disposition`/`source`/`taken` is refused by name | control | `sh tools/check/run-controls.sh` (control `rights-…`) | ❌ Wave 0 |
| RTS-01 | A `cleared` row with no resolvable source is refused | control | same | ❌ Wave 0 |
| RTS-01 | The ten rows and `data/expected/rights.ids` agree in both directions | unit (step) | `sh tools/check/steps/manifests.sh` | ❌ Wave 0 |
| RTS-02 | A refused spelling in `core/src`, `runner/`, `tools/` or a pinned page is refused | control | `sh tools/check/run-controls.sh` | ❌ Wave 0 |
| RTS-02 | A `(token, path)` pin suppresses a pre-existing hit and nothing else | control | same | ❌ Wave 0 |
| EV-06 | An `established` claim with `tier != source-expectation` is refused | control | same | ❌ Wave 0 |
| EV-06 | An `established` claim with a non-`tier1-primary-doc` dependency is refused | control | same | ❌ Wave 0 |
| EV-06 | A legal `established` claim still passes (pristine `_good/claims`) and does not trip either token | unit (step) + control `FORBID` | `sh tools/check/steps/claims.sh`; `run-controls.sh` | ✅ the fixture exists; ❌ the FORBID control |
| EV-07 | Each of the ten mechanisms resolves to a live inventory id | unit (step) | `sh tools/check/steps/important-gates.sh` | ❌ 5 of 10 unresolved (see Code Examples → "The ten mechanisms, resolved against the live inventories") |
| EV-07 | The derived accepted set covers every `Complete` requirement | unit (step) | same | ✅ |
| EV-07 | `faults` stays 238 and `steps` stays 38 | unit (step) | `sh tools/gate.sh` (summary line) | ✅ |
| cost | Last-five `cpu_s`/`gate_s` medians recomputed; verdict recorded | arithmetic (manual, from committed data) | `python3` over `.planning/ci-runs.tsv` | ✅ data; ❌ the recorded verdict |

### Sampling Rate
- **Per task commit:** the single affected step, invoked directly (all <2 s).
- **Per wave merge:** `sh tools/check/run-controls.sh` (395 controls, 4 lanes) — the fastest way to see a control-overlay mistake.
- **Phase gate:** `sh tools/gate.sh`, full suite green, `steps=38/38`, `faults=238/238`, `goldens=29`, before `/gsd:verify-work`.

### Wave 0 Gaps
- [ ] `data/rights/*.json` (10 rows) — every rights id depends on these existing
- [ ] `data/expected/rights.ids`, `data/expected/rights-pinned.ids` — membership and pins
- [ ] `tools/check/schema/rights.txt` + `tools/check/kinds/rights.cmake` (or the inline equivalent) — the checker
- [ ] `tools/check/controls/_good/tree/data/rights/*.json` + its `rights.ids` + `rights-pinned.ids` — the toy tree must pass the new checks
- [ ] `data/expected/runner-selftest.ids` (if OQ1 option b) — the four selftest case names
- [ ] `data/expected/controls/rights.ids` + `data/expected/controls/claims.ids` — inventory ids for the new control dirs
- [ ] Framework install: none — the harness is the project's own.

## Security Domain

`workflow.security_enforcement` is `true`, `security_asvs_level` 1. This phase adds no network surface, no auth, no session, no cryptography and no new file-read outside the repository. It *is* itself a rights/refusal control, so the security domain is inverted: the risk is a control that can be satisfied vacuously.

### Applicable ASVS Categories

| ASVS Category | Applies | Standard Control |
|---------------|---------|-----------------|
| V2 Authentication | no | — |
| V3 Session Management | no | — |
| V4 Access Control | no | — |
| V5 Input Validation | yes | Every new checker validates untrusted-in-principle input (JSON rows, inventory lines, path lists) with fail-closed rules: `records.cmake`'s schema types, `check_shape`'s unknown-key refusal, `read_ids`' duplicate/empty refusal, and a bounded-token scan that cannot be widened by a hostile spelling. |
| V6 Cryptography | no | Only SHA-256 identity hashes, already handled by `data/sha256`/`manifest-lib.cmake`. Never hand-rolled in this phase. |

### Known Threat Patterns for this stack (CMake `-P` + POSIX `sh` checkers)

| Pattern | STRIDE | Standard Mitigation |
|---------|--------|---------------------|
| A control that passes vacuously (changes nothing, step still exits 0) | Repudiation | `run-controls.sh:219-221` refuses `control-exit-zero`; every control must make its step fail. |
| A blank `EXPECT` line (matches every output) | Repudiation | `run-controls.sh:205-217` refuses `control-expect-blank`. |
| A pin list that rots into a wildcard or a stale permission | Tampering | `split(l, w, " ") == 2` exact-pair grammar (`release-readiness.sh:57`); reverse-direction NOTE/FAIL (OQ3). |
| A refused token that is really a substring of a legitimate identifier | Tampering | Bounded character class (Architecture Patterns → Pattern 3; Common Pitfalls → Pitfall 2). |
| A registry row that cites a requirement outside the accepted set | Repudiation | `important-gates.sh:173-176`. |
| A toy-registry edit that silently deletes coverage | Tampering | `run-controls.sh:300-325`, both directions, comments excluded. |
| A ledger row whose `disposition` is misspelled so no rule applies | Tampering | Fail-closed unknown-key refusal (schema `keys` list) — a misspelling is an unknown key, not a missing one. |
| A path-traversal spelling in a `MAKE` verb or an overlay | Elevation of privilege | `run-controls.sh:100-112` refuses `/*` and `*..*` in every `MAKE` path. |

## Sources

### Primary (HIGH confidence — read this session, line ranges quoted above)
- `tools/check/lib.cmake`, `tools/check/manifest-lib.cmake`, `tools/check/files.cmake`, `tools/check/manifests.cmake`, `tools/check/records.cmake`, `tools/check/schema/claim.txt`, `tools/check/schema/source.txt`, `tools/check/kinds/claim.cmake`
- `tools/check/run-controls.sh`, `tools/check/pass-line.sh`
- `tools/check/steps/{files,manifests,claims,sources,important-gates,runner-selftest,text,ci-log,golden,faults-verdict,pkg-install}.sh`
- `tools/gate.sh`, `tools/check/integrity.cmake`, `tools/release-readiness.sh`
- `data/expected/{important-gates.ids,important-gates-excluded.ids,claims.ids,evidence-tiers.ids,source-authority-tags.ids,sources.ids,hygiene-history-pinned.ids,gate-steps/rights.ids,gate-steps/claims.ids,controls/rights.ids,controls/claims.ids,pkg-selftests.ids,ci-summary-selftests.ids,release-assets-selftests.ids,tests/*.ids}`
- `data/{claims.json,sources.json,profiles/intv-2609-ntsc-family-p1.json}`, `tools/check/binary-ext.txt`, `.gitignore`, `docs/RIGHTS.md`, `docs/owner-images.md`
- `tools/check/controls/_good/tree/**` (124 files, enumerated)
- `.planning/{REQUIREMENTS.md,MILESTONE-CONTEXT.md,ci-runs.tsv,config.json}`, `.planning/ops/GUARDRAILS-v0.2.md`, `.planning/ops/GUARDRAILS-v0.3.md`
- `.planning/research/v0.3.0-scoping-2-gate-cost.md`, `-3-rights-format-packaging.md`, `-4-verification.md`
- `.planning/milestones/v0.2.0-phases/09-registered-evidence-and-gate-cost/09-05-SUMMARY.md`
- git: `16afb31` (TOOL-01 close), `b14e1a4` (Phase 10 close)

### Secondary (MEDIUM confidence — executed commands, not files)
- `sh tools/check/steps/{important-gates,claims,manifests,files,sources}.sh` against the real `FILELIST` (`git ls-files`, 2662 paths) → the measured baselines in "Common Pitfalls → Pitfall 8"
- `python3` arithmetic over `.planning/ci-runs.tsv` → "Phase 13 Concrete Values → The cost-checkpoint arithmetic"
- `grep -rilF` over the four naming scopes → the token-frequency table and the pin census in "Phase 13 Concrete Values → The ten rows: disposition, tokens, and the pin census"
- a replica of `important-gates.sh:130-148`'s resolution loop over the live inventories → the truth table in Code Examples → "The ten mechanisms, resolved against the live inventories"

### Tertiary (LOW confidence)
- None. Every claim above is either read from a quoted file/line range or measured by a command run in this session.

## Metadata

**Confidence breakdown:**
- Standard stack / reused machinery: **HIGH** — every helper, token and line range was read verbatim; the counts were produced by running the real steps.
- Architecture (where each limb lands): **HIGH** for the mechanism, **MEDIUM** for the split (ledger in `manifests`, naming in `files`) — that is a recommendation, not a measured fact; the alternative (one new `rights.cmake` called from `steps/files.sh`) is equally legal.
- Pitfalls: **HIGH** — each is anchored to a specific guard in a specific file, and the token-frequency/`cleared`-row ones are measured.
- Cost arithmetic: **HIGH** — computed from `.planning/ci-runs.tsv` (96 rows, 93 with `cpu_s`) and the frozen unit prices.

**Research date:** 2026-11-26
**Valid until:** 2026-12-26 for the machinery (stable, and this milestone is the only thing changing it); recompute "Phase 13 Concrete Values → The cost-checkpoint arithmetic" at every phase boundary — the medians moved by 5 cpu-s within a day of the freeze.
