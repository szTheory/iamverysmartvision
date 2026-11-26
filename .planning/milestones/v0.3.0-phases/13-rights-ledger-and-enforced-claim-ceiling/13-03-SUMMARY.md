---
phase: 13-rights-ledger-and-enforced-claim-ceiling
plan: 03
subsystem: testing
tags: [rights-ledger, naming-scan, pin-list, claim-ceiling, gate-checker, negative-control]

requires:
  - phase: 13-rights-ledger-and-enforced-claim-ceiling
    provides: the manifests-step ledger limb, the ten data/rights rows, data/expected/rights.ids and the docs/RIGHTS.md table opened by plans 13-01 and 13-02
  - phase: 09-registered-evidence-and-gate-cost
    provides: the important-gates registry and the control/inventory idiom every row here resolves through
provides:
  - the refused-token naming limb inside the files step (rights_rows, rights_scan_naming) with a character-bounded match over four scopes
  - data/expected/rights-pinned.ids, the four measured (token, path) pins, read in both directions with a stale pin as a failure
  - the right-naming control, which observes the suppressed, the refused and the stale direction in one run
  - D-50's two conditions inside tools/check/kinds/claim.cmake, two must-fail controls and one legal positive
  - RTS-02 and EV-06 flips, each in the commit that carries the rows naming it
affects: [13-04, 13-05, 14, 16]

actuals:
  # 58,481 chars / 4: the two task commits (34,203 chars, `git diff 7243af0..HEAD`) plus this
  # SUMMARY (24,278 chars). Same chars/4 scale the plan's 95,000 estimate used.
  tokens: 14620
  tasks: 2
  commits: 2
  plan_head_before: 7243af00815b7ba35e2d317ca6b1f706b7396d4b
  plan_head_after: e8c6b754f38995895cf2778ef9262e1c2b1b4a9f

tech-stack:
  added: []
  patterns:
    - "a data-declared token set scanned over a path-prefix scope with a character-bounded match, never a global grep and never grep -w"
    - "a (token, path) pin read in both directions, with a stale pin as a failure rather than a NOTE"
    - "a positive case expressed as EXPECT-an-unrelated-refusal plus FORBID-the-new-tokens, because a control directory can never express 'this passes'"
    - "one stat per limb registered in the order the step header documents, so the printed line and the documented line are the same line"

key-files:
  created:
    - data/expected/rights-pinned.ids
    - tools/check/controls/_good/tree/data/expected/rights-pinned.ids
    - tools/check/controls/rights-naming/CHECK
    - tools/check/controls/rights-naming/EXPECT
    - tools/check/controls/rights-naming/FORBID
    - tools/check/controls/rights-naming/overlay/core/src/common/planted_pinned.h
    - tools/check/controls/rights-naming/overlay/runner/planted_open.c
    - tools/check/controls/rights-naming/overlay/data/expected/rights-pinned.ids
    - tools/check/controls/claim-established-non-source/CHECK
    - tools/check/controls/claim-established-non-source/EXPECT
    - tools/check/controls/claim-established-non-source/overlay/data/claims.json
    - tools/check/controls/claim-established-without-tier1/CHECK
    - tools/check/controls/claim-established-without-tier1/EXPECT
    - tools/check/controls/claim-established-without-tier1/overlay/data/claims.json
    - tools/check/controls/claim-established-positive/CHECK
    - tools/check/controls/claim-established-positive/EXPECT
    - tools/check/controls/claim-established-positive/FORBID
    - tools/check/controls/claim-established-positive/overlay/data/claims.json
  modified:
    - tools/check/rights-lib.cmake
    - tools/check/files.cmake
    - tools/check/kinds/claim.cmake
    - data/expected/controls/rights.ids
    - data/expected/controls/claims.ids
    - data/expected/important-gates.ids
    - docs/RIGHTS.md
    - .planning/REQUIREMENTS.md
    - .planning/PROJECT.md

key-decisions:
  - "The naming limb scans `owner-local-only` rows as well as `refused` ones, because the four measured pins belong to the two `owner-local-only` rows and a stale pin is a failure: scanning only `refused` rows would make all four pins stale on the first run."
  - "A rights pin that no longer matches is a failure (rights-pinned-stale), not a NOTE as the release-readiness pin reader treats a history pin: history pins a commit, which cannot disappear, a rights pin names a path, which can."
  - "rights_rows() emits no finding and is tolerant by construction, so a malformed row degrades to the manifests step's own named finding instead of aborting the files step with an unhandled string(JSON ...) error; two steps must never report one defect twice."
  - "The pin grammar is canonicalised before the two-field test (a tab or a run of spaces is a separator, three fields is malformed), so a well-formed pin can never be reported malformed for whitespace reasons while a genuine third field is still refused."
  - "claim-overstated is kept, not subsumed: D-50 is strictly stronger, both are findings, and a control's EXPECT is a substring match, so a row that trips both keeps every existing control satisfied."
  - "The claim ceiling's legal positive is a control whose EXPECT names an unrelated refusal and whose FORBID names both new tokens; a control directory can never express 'this passes' because ctl_one refuses control-exit-zero."

patterns-established:
  - "Same-commit retarget (GUARDRAILS-v0.3 V2): a refused spelling moves that row's tokens, the pinned ids and the control overlay together; V4: an established standing moves the mechanism, two must-fail controls, one positive and the PROJECT.md note together."
  - "A new stat key is appended after the existing stats and before gate_finish, because GATE_STATS prints in insertion order and the step header documents that order."

requirements-completed: [RTS-02, EV-06]

coverage:
  - id: D1
    description: "A refused ledger spelling is refused inside core/src/, runner/, tools/ (minus the controls prefix) and the six pinned operator pages, and the four measured pre-existing occurrences are pinned and suppressed instead."
    requirement: RTS-02
    verification:
      - kind: integration
        ref: "ROOT=$PWD CHECK_DIR=$PWD/tools/check FILELIST=/tmp/fl13 sh tools/check/steps/files.sh -> '-- files pass tracked=2704 binaries-listed=0 rights-pinned=4/4' (then 'tracked=2714 rights-pinned=4/4' after task 2)"
        status: pass
      - kind: integration
        ref: "toy tree: ROOT=$T ... FILELIST=/tmp/toyfl sh tools/check/steps/files.sh -> '-- files pass tracked=129 binaries-listed=0 rights-pinned=0/0'"
        status: pass
      - kind: other
        ref: "bounded refusals of the fifteen refused tokens over the whole tree by an independent python scan -> exactly four pairs, the two operator paths for each of the two operator placeholders"
        status: pass
    human_judgment: false
  - id: D2
    description: "The pin list is read in both directions: a pinned occurrence is suppressed, an unpinned occurrence is refused by name, and a pin that matches nothing is refused as stale, all in one control run."
    requirement: RTS-02
    verification:
      - kind: integration
        ref: "sh tools/check/run-controls.sh -> 'control ok   rights-naming' and 'controls=397/397'; EXPECT is 'FAIL rights-token: toy-image runner/planted_open.c' plus 'FAIL rights-pinned-stale: grom-image tools/nowhere.sh'"
        status: pass
      - kind: other
        ref: "scratch stand-in: an over-strict pin grammar makes the same control tree print 'rights-pinned-malformed', the control's FORBID token, so an over-strict rule fails the control rather than passing it"
        status: pass
    human_judgment: false
  - id: D3
    description: "An established claim whose tier is not source-expectation, or which rests on a field not tagged tier1-primary-doc, is refused by name, while the live ledger does not move."
    requirement: EV-06
    verification:
      - kind: integration
        ref: "ROOT=$PWD ... sh tools/check/steps/claims.sh -> '-- claims pass claims=28 provisional=27' (CL-0002's stic.part, crystal_hz and region_video are all tier1-primary-doc)"
        status: pass
      - kind: integration
        ref: "sh tools/check/run-controls.sh -> 'control ok   claim-established-non-source', 'control ok   claim-established-without-tier1', and the pre-existing 'control ok   claims-overstated-standing'"
        status: pass
    human_judgment: false
  - id: D4
    description: "The legal positive is not vacuous: a rule that refused every established claim would fail the positive control."
    requirement: EV-06
    verification:
      - kind: integration
        ref: "sh tools/check/run-controls.sh -> 'control ok   claim-established-positive' (EXPECT 'FAIL claim-dangling-path: CL-0001', FORBID both new tokens)"
        status: pass
      - kind: other
        ref: "scratch stand-in on the same control tree with a sabotaged copy of claim.cmake: making condition (a) refuse every established row prints 'claim-established-non-source', and making condition (b) refuse every tag prints 'claim-established-without-tier1', so both FORBID tokens can fire and the control can fail"
        status: pass
    human_judgment: false

duration: 27min
completed: 2026-11-25
status: complete
---

# Phase 13 Plan 03: The naming limb and the enforced claim ceiling

**A refused ledger spelling now fails the gate by name in `core/src`, `runner/`, `tools/` and the six pinned operator pages with its four pre-existing operator placeholders pinned and read in both directions, and an `established` claim that is not a `source-expectation` claim resting on a `tier1-primary-doc` field is refused by name while `CL-0002` still passes.**

## Performance

- **Duration:** ~27 min (approximate: the first read to the second task commit; the figure excludes this SUMMARY)
- **Started:** 2026-11-25T21:50:00Z (approximate)
- **Completed:** 2026-11-25T22:17:26Z
- **Tasks:** 2
- **Files modified:** 27 distinct paths (19 created, 8 modified; `git diff --name-only 7243af0..HEAD`)

## Accomplishments

- `tools/check/rights-lib.cmake` gains `rights_rows()` and `rights_scan_naming()`: the refused set is data (every row whose disposition is `refused` or `owner-local-only`), the scope is three path prefixes plus six exact operator pages, and the match is character-bounded on both sides. Nothing about the ten rows is transcribed into the source that judges them.
- `data/expected/rights-pinned.ids` records the four measured `(token, path)` pairs — the operator's own argument placeholders in `tools/owner-run.sh` and `docs/owner-images.md` — and the limb reads the list in **both** directions, so an unpinned occurrence and an unmatched pin are each refused by name.
- `tools/check/kinds/claim.cmake` gains D-50's two conditions where no early return can skip them, with two must-fail controls that isolate each condition and one legal positive that keeps both satisfied while the step fails for a dangling artifact.
- `controls=397/397` after Task 1 and `400/400` after Task 2; `steps=38/38`, `faults=238/238` and `goldens=29` are untouched, as are `claims=28 provisional=27` and `tracked`-independent invariants.
- Four new control directories — `rights-naming`, `claim-established-non-source`, `claim-established-without-tier1`, `claim-established-positive` — so the phase total reaches six once plan 13-04 adds `important-gates-excluded-stale`.

## Measured verification

Every `<automated>` command in the plan was run against the committed tree, and each `<fails_when>` was checked:

| Check | Observed |
|---|---|
| `files` after Task 1 | `-- files pass tracked=2704 binaries-listed=0 rights-pinned=4/4`, exit 0, zero lines starting `-- FAIL ` |
| `files` after Task 2 | `-- files pass tracked=2714 binaries-listed=0 rights-pinned=4/4` |
| `files` on the toy tree, after Task 1 | `-- files pass tracked=129 binaries-listed=0 rights-pinned=0/0` |
| `claims` after Task 2 | `-- claims pass claims=28 provisional=27`, zero `-- FAIL ` lines |
| `important-gates` after Task 1 | `-- important-gates pass gates=112 lines=339` |
| `important-gates` after Task 2 | `-- important-gates pass gates=113 lines=342` |
| `run-controls.sh` after Task 1 | `controls=397/397`, including `control ok   rights-naming` |
| `run-controls.sh` after Task 2 | `controls=400/400`, including `control ok   claim-established-non-source`, `control ok   claim-established-without-tier1`, `control ok   claim-established-positive`, and the unchanged `control ok   claims-overstated-standing` |
| `text` | `-- text pass docs=64 claim-refs=26` (the new `docs/RIGHTS.md` bullets add no untagged claim word) |
| `integrity` | `-- integrity pass files=2714`, no `integrity-non-ascii`, no `integrity-missing-spdx` |
| `sources` | `-- sources pass sources=13 refs-verified=3/4` (unchanged) |
| `manifests` | `-- manifests pass manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1` (unchanged) |
| `denylist` | `-- denylist pass scanned=685 waived=0` (unchanged) |
| `sh tools/gate.sh` (final) | `gate: PASS steps=38/38 controls=400/400 ... faults=238/238 ... tracked=2714 binaries-listed=0 rights-pinned=4/4 ... goldens=29 ... gates=113 lines=342 ... claims=28 provisional=27` |
| `dash tools/gate.sh` (final) | `gate: PASS ...`; `diff` of the two `gate:` lines is empty |
| `sh tools/hygiene.sh --tree` | `-- hygiene pass mode=tree hygiene-files=2714` |
| `ls tools/check/steps/*.sh \| wc -l` | `38` |
| `awk '!/^#/ && NF' data/expected/faults/*.ids \| wc -l` | `238` |
| `git ls-files data/golden \| wc -l` | `29` |
| `git ls-files \| wc -l` | `2696` → `2704` (Task 1, eight new tracked paths) → `2714` (Task 2, ten more) |
| the plan's "no refused token in the two `.cmake` files" criterion | the fifteen refused tokens, bounded, matched against `tools/check/rights-lib.cmake` and `tools/check/files.cmake`: no match, exit 0 |

`rights-pinned=4/4` is the witness for the naming limb: all four pins matched an observed pair and none went stale. It is unreachable without the per-`(token, path)` deduplication — the eight measured occurrences collapse to four pairs — and unreachable without the comment discipline the plan required, since a refused spelling written into either `.cmake` file would add a fifth unpinned pair and print `rights-pinned=4/6`.

## The deliberate widening of RTS-02's naming limb

RTS-02's wording names "a refused item's spellings". The limb implemented here scans every row whose disposition is **`refused` or `owner-local-only`**, which is wider than the literal wording, and the widening is deliberate: the four measured pre-existing occurrences (`exec-image` and `grom-image` in `tools/owner-run.sh` and `docs/owner-images.md`) belong to the two `owner-local-only` rows, and because a stale pin is now a **failure** rather than a NOTE, scanning only the `refused` rows would leave all four pins unmatched and the `files` step would fail on its own permission list the first time it ran. A `cleared` row's tokens are never refused — that is what clearing means — so the widening stops exactly where the disposition grammar stops.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] CMake's regular-expression engine does not support `[[:space:]]`**
- **Found during:** Task 1, the first run of the `files` step — all four pins came back `rights-pinned-malformed`.
- **Issue:** the pin grammar was written as `^[^[:space:]]+[[:space:]]+[^[:space:]]+$`, and CMake's regex engine silently fails to match that expression, so every well-formed pin was refused. A silently non-matching guard is the worst shape of this defect: the control would have reported "malformed" on its own valid pins rather than a wrong count.
- **Fix:** the pin line is canonicalised first (`string(REPLACE "\t" " ")` then `+ ` collapse) and the grammar tested as `^[^ ]+ [^ ]+$`. This also makes the comparison canonical, so a tab or a run of spaces is a separator rather than a malformed line, while a genuine third field is still `rights-pinned-malformed`.
- **Files modified:** `tools/check/rights-lib.cmake`
- **Verification:** `rights-pinned=4/4` on the real tree; the scratch stand-in with an over-strict grammar prints `rights-pinned-malformed`, which is the control's FORBID token.
- **Committed in:** `0930e68`

**2. [Rule 1 - Bug] An em dash in `data/expected/controls/claims.ids` failed `integrity-non-ascii`**
- **Found during:** Task 2, the first full gate run — `gate: FAIL steps=37/38` with `-- FAIL integrity-non-ascii: data/expected/controls/claims.ids`.
- **Issue:** the comment block used two em dashes; the `integrity` step refuses any non-ASCII byte in `data/`.
- **Fix:** both replaced with `:` and `,`.
- **Files modified:** `data/expected/controls/claims.ids`
- **Verification:** `-- integrity pass files=2714`, then the full gate ends `gate: PASS`.
- **Committed in:** `e8c6b75`

### Recorded plan-figure corrections

**3. [Rule 1 - Process] The plan's `git ls-files` baseline is one commit stale, so both `tracked=` predictions are one low**

- **Found during:** Task 1 verification, then again at Task 2.
- **Issue:** the plan predicts `2694 → 2702` and `2702 → 2712` (`[HEAD=2694 EXPECT==2702]`, `[HEAD=2702 EXPECT==2712]`). The measured HEAD at this plan's start is **2696**, because plan 13-01's *metadata* commit added `13-01-SUMMARY.md` after the `2685` figure its own plan quoted, and plan 13-02's metadata commit added `13-02-SUMMARY.md` after the `2695` figure. The plan's baseline is the 13-01 **task** commit, one tracked file behind. This is the same defect plan 13-02 recorded as its deviation 1.
- **Fix:** no code change. The delta the criterion actually asserts is verified directly: `2696 → 2704` (eight new tracked paths, Task 1: the pin list, the toy pin list, and six control files) and `2704 → 2714` (ten new tracked paths, Task 2: three control directories of three files each plus one `FORBID`). Every other token of both pass lines matches the plan exactly, and `rights-pinned=4/4` is the substantive assertion.
- **Files modified:** none (the plan text is a frozen artifact; the correction lives in this SUMMARY).
- **Verification:** `git ls-files | wc -l` → `2714`; the two task commits' `--stat` are `14 files changed, 233 insertions(+), 4 deletions(-)` and `15 files changed, 103 insertions(+), 3 deletions(-)`, with eighteen `create mode` lines and empty `--diff-filter=D` output.
- **Committed in:** `0930e68`, `e8c6b75` (the trees the figures are measured on)

**4. [Rule 3 - Process] The Step 0 protected-branch assertion reports `main` as protected**

- **Found during:** both task commits.
- **Issue:** `gsd_run query git.base-branch --is-protected main` returns `true`, so the executor's default-branch guard would refuse the commit. This repository sets `git.branching_strategy: none` and the dispatch instruction is explicit: the executor runs sequentially on `main` and stays there. Plans 13-01 and 13-02 committed the same way, so `main` is the sanctioned branch for this milestone, not drift.
- **Fix:** committed on `main` as dispatched, with hooks enabled (no `--no-verify`). Recorded rather than silently taken, because the guard's own override key (`git.allow_default_branch_commits`) is absent from `.planning/config.json` — the override is the `branching_strategy: none` setting plus the dispatch contract.
- **Files modified:** none.
- **Verification:** `git rev-parse --abbrev-ref HEAD` → `main`; both commits landed with the pre-commit hooks running.
- **Committed in:** `0930e68`, `e8c6b75`

---

**Total deviations:** 2 auto-fixed (2 Rule 1: the CMake regex escape and the non-ASCII comment) and 2 recorded notes (a stale plan baseline and the sanctioned `main` commit).
**Impact on plan:** no scope creep and **no file-set delta** — every path in both task commits is on the plan's `files_modified` list. The regex fix made the guard *stricter* than the plan's quoted shape, not weaker; the non-ASCII fix is two characters of punctuation.

## Decisions Made

- **The naming limb scans `owner-local-only` rows too.** Reason recorded above and in the source comment beside the rule: the four pins belong to those rows, and a stale pin is a failure.
- **`claim-overstated` is kept rather than subsumed by D-50.** D-50's condition (b) is strictly stronger for the established case, but both are findings, and `run-controls.sh` matches `EXPECT` as a substring, so the existing `claims-overstated-standing` control keeps passing (verified: `control ok   claims-overstated-standing`). Dropping the rule would delete a refusal, which GUARDRAILS-v0.3 V2 forbids.
- **`rights_rows()` is silent and tolerant.** It raises no finding for a row it cannot read, so the `manifests` step remains the single owner of the row grammar and a malformed row degrades to a named finding there rather than aborting `files` with an unhandled `string(JSON ...)` error. The `rights-ledger` control's `CHECK` is `manifests`, so it cannot witness this path; the tolerance is a stated guarantee, and the live tree is well-formed.
- **The positive control names a third, unrelated refusal.** `EXPECT FAIL claim-dangling-path: CL-0001` keeps the step exiting non-zero for a reason neither new rule touches, which is what makes the absence of both new tokens a positive rather than an exit-zero failure.

## Issues Encountered

- **`git ls-files` must be staged before the naming limb can see a file.** The plan's `<automated>` command builds `FILELIST` from `git ls-files`, so the pin list and both control overlays were staged before the first `files` run. Unstaged, `rights-pinned.ids` is absent and `read_ids` reports `rights-pinned-inventory-missing`.
- **The gate is order-sensitive through `GATE_STATS`.** Registering `rights-pinned` before `tracked` would have printed `rights-pinned=4/4 tracked=... binaries-listed=0` and every downstream summary line would have named a line the step never prints. The stat is registered last, and the header documents that order.
- **The estimate missed by ~6.5x again.** `estimate.tokens` was 95,000 against a realized diff of 58,481 characters → 14,620 tokens at chars/4. Recorded as measured, not adjusted: this is the third plan in the phase whose estimate priced the planning context rather than the diff (+9.9x for 13-01, +7.1x for 13-02, +6.5x here), so the miss is systematic and the fix belongs in the estimator, not in this record.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- Plan 13-04 registers the ten D-09-06 mechanisms and adds the sixth control directory (`important-gates-excluded-stale`), plus `data/expected/runner-selftest.ids` and the thirteen toy files. The registry now reads `gates=113 lines=342`; after 13-04's ten rows and four new gate names it must read `gates=117 lines=352`, and `controls=400` becomes `401`.
- `rights-pinned=4/4` is now a **second inventory**: deleting a line from `data/expected/rights-pinned.ids` makes the step red, so a plan that renames `tools/owner-run.sh`'s placeholders must move the pin in the same commit.
- The naming limb is in scope for any later phase that adds a `tools/*.sh`, a `runner/*.c` or one of the six operator pages: a refused spelling written there is now a gate failure, and the six-page list is a data list inside `rights_scan_naming()`, so widening it is a deliberate diff.
- `claim-established-positive` is the shape to copy for any future "this passes" positive: `EXPECT` an unrelated refusal, `FORBID` the token the rule would emit if it were too broad.

## Self-Check: PASSED

- Created files exist: all eighteen created paths are tracked (`git ls-files` → `2714`) and were read by the steps that judge them (`files` on the real and toy trees, `claims`, `run-controls.sh`).
- Commits exist: `0930e68` and `e8c6b75` are ancestors of HEAD.
- Re-ran every acceptance criterion from both tasks after the last edit: `files` `rights-pinned=4/4`, toy tree `rights-pinned=0/0`, `claims=28 provisional=27`, `important-gates gates=113 lines=342`, `controls=400/400` with all three new controls and `claims-overstated-standing` ok, `steps=38`, `faults=238`, `goldens=29`, and the "no refused token in either `.cmake` file" criterion exits 0 with no output.
- Re-ran the plan-level `<verification>` commands: `files`, `claims`, `important-gates`, `run-controls.sh`, `text` and `sh tools/gate.sh` all pass, and `dash tools/gate.sh` prints a byte-identical summary line.

---
*Phase: 13-rights-ledger-and-enforced-claim-ceiling*
*Completed: 2026-11-25*
