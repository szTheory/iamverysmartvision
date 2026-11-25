---
phase: 13-rights-ledger-and-enforced-claim-ceiling
plan: 02
subsystem: testing
tags: [rights-ledger, data-rows, operator-page, gate-checker]

requires:
  - phase: 13-rights-ledger-and-enforced-claim-ceiling
    provides: the manifests-step ledger limb (rights_check), data/expected/rights.ids and the docs/RIGHTS.md table opened by plan 13-01
  - phase: 01-provenance-and-rights
    provides: the sources registry a cleared row resolves against
provides:
  - the nine remaining RTS-01 rows, so the ledger answers every third-party-content question this milestone touches
  - ten data/rights/<slug>.json rows with exactly one cleared row (libretro-abi)
  - ten operator rows in docs/RIGHTS.md, each carrying its row's exact item phrase
affects: [13-03, 14, 16]

actuals:
  # 27,111 chars / 4 over the realized diff: the task commit (9,772 chars, `git show bfa009c`)
  # plus this SUMMARY (17,339 chars). Same chars/4 scale the plan's 48,000 estimate used.
  tokens: 6778
  tasks: 1
  commits: 1
  plan_head_before: f023487b5fd8f6c8d733611f7cc2cdf2d466e618
  plan_head_after: bfa009ca034dd54036fc0993cc2a2ce4626ace46

tech-stack:
  added: []
  patterns:
    - "a ledger row's item phrase is kept free of its own tokens under the character-bounded rule, so the operator page that names every row cannot pin itself"
    - "a refused row whose token begins with a dot is enforced on the hyphenated noun; the extension refusal stays in the binary-extension list and the ignore file"

key-files:
  created:
    - data/rights/flat-cartridge-image.json
    - data/rights/cfg-container.json
    - data/rights/bank-modes.json
    - data/rights/exec-image.json
    - data/rights/grom-image.json
    - data/rights/commercial-rom.json
    - data/rights/rendered-frame.json
    - data/rights/save-state.json
    - data/rights/core-info-strings.json
  modified:
    - data/expected/rights.ids
    - docs/RIGHTS.md

key-decisions:
  - "Every new row is refused or owner-local-only with source null: libretro-abi stays the only cleared row, which is what rights-cleared=1 measures."
  - "core-info-strings carries an empty tokens list as a recorded answer: the field names it would refuse are governed by the cleared libretro-abi row, and a non-empty list would make one row refuse what another clears."
  - "cfg-container is enforced on the hyphenated noun rather than on the file extension, because under the character-bounded rule a leading-dot token can only match at a boundary; the extension refusal stays where it already lives."
  - "No note spells an emulator project name and no note names its own row's tokens, so the rows stay outside the denylist cite list and cannot self-pin."

patterns-established:
  - "Same-commit retarget (GUARDRAILS-v0.3 V1): each new row moved data/rights/<slug>.json, its data/expected/rights.ids line and its docs/RIGHTS.md row together, in one commit."

requirements-completed: [RTS-01]

coverage:
  - id: D1
    description: "Ten RTS-01 ledger rows exist, each with a disposition, a non-empty taken cell and an operator row in docs/RIGHTS.md, with exactly one cleared row whose source resolves."
    requirement: RTS-01
    verification:
      - kind: integration
        ref: "ROOT=$PWD CHECK_DIR=$PWD/tools/check FILELIST=/tmp/fl13 sh tools/check/steps/manifests.sh -> '-- manifests pass manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1'"
        status: pass
    human_judgment: false
  - id: D2
    description: "Every row's item phrase is free of every one of its own tokens entries under the character-bounded rule, so the operator page cannot pin itself."
    requirement: RTS-01
    verification:
      - kind: other
        ref: "python3 bounded-match scan over data/rights/*.json (match requires the char before and after to be outside [A-Za-z0-9_-]) -> 'bad= 0' for both item and note, all ten rows"
        status: pass
    human_judgment: false
  - id: D3
    description: "The nine rows are data inside the ledger plan 13-01 landed: no new step, fault, golden or control, and no new mechanism of any kind."
    requirement: RTS-01
    verification:
      - kind: integration
        ref: "sh tools/gate.sh -> 'gate: PASS steps=38/38 controls=396/396 ... faults=238/238 ... goldens=29 ... rights=10 rights-cleared=1'"
        status: pass
      - kind: integration
        ref: "sh tools/check/run-controls.sh -> 'controls=396/396' (unchanged: no control was added)"
        status: pass
    human_judgment: false
  - id: D4
    description: "RTS-01's evidence is complete: the requirement's row set is closed here, while its status flip landed with the registry row in plan 13-01."
    requirement: RTS-01
    verification:
      - kind: integration
        ref: "awk '!/^#/ && NF' data/expected/rights.ids | wc -l -> 10; awk '/^data\\/rights\\//{n++} END{print n+0}' /tmp/fl13 -> 10"
        status: pass
    human_judgment: false

duration: 20min
completed: 2026-11-25
status: complete
---

# Phase 13 Plan 02: The nine remaining RTS-01 ledger rows

**Ten question-shaped rights rows with exactly one `cleared` row, each carrying its own refused spellings and an operator row that cannot pin itself, landed as pure data with `steps=38`, `faults=238` and `goldens=29` untouched.**

## Performance

- **Duration:** ~20 min
- **Started:** 2026-11-25T21:35:00Z (approximate — the first row file was written at 21:49Z)
- **Completed:** 2026-11-25T21:55:00Z
- **Tasks:** 1
- **Files modified:** 11 (9 created, 2 modified)

## Accomplishments

- The nine remaining RTS-01 rows exist as one JSON file each under `data/rights/`, so the ledger now answers every third-party-content question this milestone touches rather than one (D-46, GUARDRAILS-v0.3 V11 "rights before bytes").
- Exactly one row is `cleared` — `libretro-abi`, landed by plan 13-01 — and it is the only row with a non-null `source`; the `rights-cleared=1` stat is the count that would catch a second one (threat T-13-19).
- Every row's `item` phrase is free of its own `tokens` under the character-bounded rule, so the operator page that must name all ten rows cannot pin itself (threat T-13-18).
- No mechanism was added: the nine rows are data inside the ledger limb plan 13-01 landed. `steps=38`, `faults=238`, `goldens=29` and `controls=396` are unchanged.
- The `data/expected/rights.ids` membership baseline moved with the rows in the same commit, so a row landed without its inventory line and its operator row would have been a red gate rather than a quiet omission.

## The ten rows as measured

| slug | disposition | `taken` | `source` | `tokens` |
|---|---|---|---|---|
| `bank-modes` | `refused` | `nothing` | `null` | `bank-switch`, `bankswitch` |
| `cfg-container` | `refused` | `nothing` | `null` | `.cfg`, `cfg-container` |
| `commercial-rom` | `refused` | `nothing` | `null` | `game-rom`, `cartridge-dump`, `commercial-rom` |
| `core-info-strings` | `owner-local-only` | `nothing` | `null` | *(empty — a recorded answer)* |
| `exec-image` | `owner-local-only` | `nothing` | `null` | `exec-image` |
| `flat-cartridge-image` | `owner-local-only` | `nothing` | `null` | `flat-image` |
| `grom-image` | `owner-local-only` | `nothing` | `null` | `grom-image` |
| `libretro-abi` | **`cleared`** | `nothing` | `SRC-libretro-api-docs` | `libretro` |
| `rendered-frame` | `refused` | `nothing` | `null` | `screenshot`, `rendered-frame`, `page-render` |
| `save-state` | `owner-local-only` | `nothing` | `null` | `save-state`, `savestate` |

`libretro-abi` is unchanged from plan 13-01 and was not edited. Its `SRC-` id resolves by membership in `data/sources.json` (`-- sources pass sources=13 refs-verified=3/4`, unchanged).

## Measured verification

Every `<automated>` command in the plan was run against the committed tree; the observed output:

| Check | Observed |
|---|---|
| `manifests` (real tree) | `-- manifests pass manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1`, exit 0, zero lines starting `-- FAIL ` |
| `text` | `-- text pass docs=64 claim-refs=26` |
| `integrity` | `-- integrity pass files=2695`, no `integrity-non-ascii` and no `integrity-missing-spdx` |
| `important-gates` | `-- important-gates pass gates=111 lines=338` (unchanged: no registry row was added) |
| `sources` | `-- sources pass sources=13 refs-verified=3/4` (unchanged) |
| `denylist` | `-- denylist pass scanned=684 waived=0` (no cite-list name in any new row) |
| `run-controls.sh` | `controls=396/396` (unchanged) |
| `sh tools/gate.sh` | `gate: PASS steps=38/38 controls=396/396 ... fault-runs=238 ... faults=238/238 ... tracked=2695 ... goldens=29 ... manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1 ... sources=13 refs-verified=3/4 ... gates=111 lines=338` |
| `git ls-files \| wc -l` | `2686` → **`2695`** at the task commit (nine new tracked files) |
| `awk '/^data\/rights\//{n++} END{print n+0}' /tmp/fl13` | `10` |
| `awk '!/^#/ && NF' data/expected/rights.ids \| wc -l` | `10` |

`<fails_when>` was checked for the task's single automated command: exit 0, zero `-- FAIL ` lines, and the pass line matches the plan token for token (including `rights=10` and `rights-cleared=1`).

## Task Commits

Each task was committed atomically:

1. **Task 1: The nine remaining RTS-01 rows and their operator rows** — `bfa009c` (feat)

**Plan metadata:** the SUMMARY/STATE/ROADMAP commit that follows this file.

## Files Created/Modified

- `data/rights/flat-cartridge-image.json` — owner-local-only; the flat image an owner supplies, with the wrap tool's byte-length/word-order statement noted.
- `data/rights/cfg-container.json` — refused; the configuration container, enforced on the hyphenated noun, with the leading-dot residual recorded in the note.
- `data/rights/bank-modes.json` — refused; a bank-switched cartridge mode (FMT-04 stays Deferred).
- `data/rights/exec-image.json` — owner-local-only; the executive image, with the four pre-existing operator-placeholder hits left for plan 13-03 to pin.
- `data/rights/grom-image.json` — owner-local-only; the graphics image, same four pins.
- `data/rights/commercial-rom.json` — refused; a commercial cartridge.
- `data/rights/rendered-frame.json` — refused; a rendered page image, distinguished from the runner's own project-authored frame export.
- `data/rights/save-state.json` — owner-local-only; a saved machine state, with the note recording that the project's own snapshots are `snapshot`/`.ivsn` so the feature is not refused.
- `data/rights/core-info-strings.json` — owner-local-only with an empty `tokens` list, per RESEARCH A6.
- `data/expected/rights.ids` — nine ids appended, sorted, header kept.
- `docs/RIGHTS.md` — nine rows added to the "Questions this milestone touches" table, each carrying its exact `item` phrase and a dispositional sentence, and no refused token spelled.

## Decisions Made

- **`rights-cleared=1` is the invariant, not `rights=10` alone.** Every new row is `refused` or `owner-local-only` with `source: null`, so the count that would catch a row added to make the ledger look answered is the one the pass line carries.
- **The `cfg-container` note records the residual rather than hiding it.** Under the character-bounded rule a token beginning with `.` matches only at a boundary, so `.cfg` cannot catch an extension in the middle of a file name. The enforced spelling is the hyphenated noun; the extension refusal stays in the binary-extension list and the ignore file, where it already lives. Removing either would delete a refusal (GUARDRAILS-v0.3 V2).
- **`core-info-strings` takes an empty `tokens` list.** The libretro-shaped field names it would otherwise refuse are governed by the cleared `libretro-abi` row; a non-empty list would make one row refuse what the row above clears, and making it `cleared` would create a second cleared row. The empty list is a recorded answer, and the checker deliberately does not treat an empty array as a blank cell.
- **No note names an emulator project and no note spells its own tokens.** The `denylist` step scans `data/rights/` (`scanned=684`), and the character-bounded rule makes a note that spelled its own token a self-refusal waiting for plan 13-03's naming limb.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] The plan's `git ls-files` baseline was one commit stale**
- **Found during:** Task 1 verification — the plan's acceptance criterion predicts `2685` → `2694`, but the measured pre-plan count is `2686`.
- **Issue:** the plan's figures were captured at the 13-01 **task** commit (`2685`). The 13-01 metadata commit then added `13-01-SUMMARY.md` as a tracked file, so HEAD of this plan is `2686`. The plan's stated expectation (`2694`) is therefore unreachable, and its `[HEAD=2685 EXPECT==2694]` annotation disagrees with the tree.
- **Fix:** no code change — the delta the criterion actually asserts (nine new tracked paths, nothing else) is verified directly: `2686` → `2695`, with `awk '/^data\/rights\//{n++}'` = `10` and the ten `rights.ids` lines. The stale figure is recorded here rather than silently matched, per the phase constraint that a quoted count disagreeing with the file is a defect to record.
- **Files modified:** none (the plan text is a frozen artifact; the correction lives in this SUMMARY).
- **Verification:** `git ls-files | wc -l` → `2695`; the task commit's `--stat` is `11 files changed, 99 insertions(+)` with nine `create mode` lines and two modifications, and `git show --diff-filter=D` is empty.
- **Committed in:** `bfa009c` (the tree the figure is measured on)

### Recorded process notes

**2. [Rule 3 - Process] The Step 0 protected-branch assertion reports `main` as protected**
- **Found during:** the task commit.
- **Issue:** `gsd_run query git.base-branch --is-protected main` returns `true`, so the executor's default-branch guard would refuse the commit. This repository sets `git.branching_strategy: none` and the dispatch instruction is explicit: the executor runs sequentially on `main` and stays there. Plan 13-01 committed the same way (`9362fda`, `f023487`), so `main` is the sanctioned branch for this milestone, not drift.
- **Fix:** committed on `main` as dispatched, with hooks enabled (no `--no-verify`). Recorded rather than silently taken, because the guard's own override key (`git.allow_default_branch_commits`) is absent from `.planning/config.json` — the override is the `branching_strategy: none` setting plus the dispatch contract.
- **Files modified:** none.
- **Verification:** `git rev-parse --abbrev-ref HEAD` → `main`; the commit landed with the pre-commit hooks running.
- **Committed in:** `bfa009c`

---

**Total deviations:** 1 auto-fixed (1 Rule 1, a stale plan figure) and 1 recorded process note.
**Impact on plan:** No scope creep and no file-set delta. The nine rows, the inventory and the operator page are exactly the plan's file list; the only surprise is a quoted baseline that was one tracked file behind the tree.

## Issues Encountered

- **`git ls-files` must be staged first.** The plan's `<automated>` command builds `FILELIST` from `git ls-files`, so the nine new rows had to be staged before the `manifests` limb could see them — otherwise the first run reports `rights-id-missing` for each new id. The rows were staged, verified, and committed in one step.
- **The estimate missed by ~7x.** `estimate.tokens` was 48,000 against a realized diff of 27,111 characters (the 9,772-character task commit plus this 17,339-character SUMMARY) → 6,778 tokens at chars/4. Recorded as measured, not adjusted: the estimate priced the planning context, not the diff, which is the same miss plan 13-01 recorded. The nine rows themselves are 2,443 tokens.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- The ledger is complete for this milestone: ten rows, one inventory, one `cleared` row with a resolvable `SRC-` id, ten operator rows. RTS-01's **evidence** completed here; its **status flip** landed in plan 13-01, because the registry row that names it cannot reference a `Pending` requirement.
- Plan 13-03 extends `rights-lib.cmake` with the naming limb and adds `data/expected/rights-pinned.ids`. The four pins it must accept are unchanged and still measured: `exec-image` and `grom-image` in `tools/owner-run.sh` (two lines each) and `docs/owner-images.md` (two lines).
- Note for 13-03: `docs/RIGHTS.md` carries the `item` phrases and no refused token. The `bank-modes` phrase "a bank-switched cartridge mode" is deliberately free of its own token under the bounded rule (`bank-switch` is followed by `e`), and `cfg-container`'s page row spells neither `.cfg` nor the hyphenated noun.
- `data/expected/rights.ids` is now a complete ten-id baseline, so 13-03's naming limb and its control overlay start from a settled membership set.

## Self-Check: PASSED

- Created files exist: all nine `data/rights/*.json` files are tracked (`git ls-files` → 2695, `data/rights/` → 10) and were read by the `manifests` limb in the final run.
- Commit exists: `bfa009c` is an ancestor of HEAD.
- Re-ran every acceptance criterion from the single task after the last edit: `manifests` pass line exact, `data/rights/` = 10, `rights.ids` = 10, `text docs=64 claim-refs=26`, `integrity files=2695` with no non-ASCII or missing-SPDX finding, `controls=396/396`, `git ls-files` = 2695.
- Re-ran the plan-level `<verification>` commands: all pass, and `sh tools/gate.sh` ends `gate: PASS` with `steps=38/38 controls=396/396 faults=238/238 goldens=29 rights=10 rights-cleared=1`.

---
*Phase: 13-rights-ledger-and-enforced-claim-ceiling*
*Completed: 2026-11-25*
