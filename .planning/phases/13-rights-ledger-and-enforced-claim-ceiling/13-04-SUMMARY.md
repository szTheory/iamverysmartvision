---
phase: 13-rights-ledger-and-enforced-claim-ceiling
plan: 04
subsystem: testing
tags: [important-gates, registry-row, mutant-inventory, control-directory, selftest-inventory, ev-07]

requires:
  - phase: 09-registered-evidence-and-gate-cost
    provides: the important-gates registry, the derived accepted set, the exclusion ledger and the control/inventory idiom every row here resolves through
  - phase: 13-rights-ledger-and-enforced-claim-ceiling
    provides: the RTS-01, RTS-02 and EV-06 gate names and their control directories, landed by plans 13-01 and 13-03
provides:
  - data/expected/runner-selftest.ids, the four D-09-06 case names a `mutant` row may resolve against
  - tools/check/selftest-inventory.sh, the guard that refuses an inventory name the step did not run
  - the st-no-such-case stand-in, committed as runner-selftest's fourteenth case
  - tools/check/controls/important-gates-excluded-stale, the sixth Phase 13 control directory
  - ten d09-06-registration rows, six control and four mutant, with EV-07 Complete
  - the re-derived baseline gates=114 lines=352
affects: [13-05, 14, 16]

actuals:
  # 56,677 chars / 4: the three task commits (28,399 chars, `git diff c2680a7..HEAD`) plus this
  # SUMMARY (28,278 chars). Same chars/4 scale the plan's 90,000 estimate used.
  tokens: 14169
  tasks: 3
  commits: 3
  plan_head_before: c2680a74193c20372377c821814a5c8aa946c297
  plan_head_after: e93dbe8840faaf9b9740415d3ed55b1bbcec9e39

tech-stack:
  added: []
  patterns:
    - "a rule no control directory can reach lives in a small file of its own (tools/check/selftest-inventory.sh), and a committed case in the step it guards proves that file can fail"
    - "a `mutant` registry id resolves through a committed case inventory the step must actually run, never through a name hard-coded in the registry"
    - "a mechanism that fires only as a co-firing EXPECT line of another control gets its own directory, so its registry row resolves as a `control` id and its FORBID list proves it failed for that reason alone"
    - "a one-line append applied to all ten whole-file toy registries leaves every declared overlay difference untouched, so no DEVIATION entry is needed"

key-files:
  created:
    - data/expected/runner-selftest.ids
    - tools/check/selftest-inventory.sh
    - tools/check/controls/important-gates-excluded-stale/CHECK
    - tools/check/controls/important-gates-excluded-stale/EXPECT
    - tools/check/controls/important-gates-excluded-stale/FORBID
    - tools/check/controls/important-gates-excluded-stale/overlay/data/expected/important-gates-excluded.ids
  modified:
    - tools/check/steps/runner-selftest.sh
    - tools/check/steps/important-gates.sh
    - data/expected/important-gates.ids
    - data/expected/controls/important-gates.ids
    - .planning/REQUIREMENTS.md
    - .planning/STATE.md
    - tools/check/controls/_good/tree/.planning/REQUIREMENTS.md
    - tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md
    - tools/check/controls/important-gates-complete-uncovered/overlay/.planning/REQUIREMENTS.md
    - tools/check/controls/_good/tree/data/expected/important-gates.ids
    - the nine important-gates-*/overlay/data/expected/important-gates.ids whole-file toy registries

key-decisions:
  - "The six mechanisms a control directory already proves are registered as `control` rows, not `mutant` rows: a mutant row for a mechanism a directory already proves would be a second registration of one proof, and inventing a selftest to re-prove it would add a case with no new evidence. This is the recorded deviation from EV-07's literal 'mutant row' wording, and zero new faults holds."
  - "The selftest-inventory rule is a file (tools/check/selftest-inventory.sh), not an inlined branch, because a guard inside the step is reachable only by the step running itself; its failure is committed as the st-no-such-case case rather than demonstrated on a scratch copy the commit discards."
  - "important-gates-excluded-stale keeps the good tree's FND-98 row in its overlay: dropping it would make FND-98 Complete-and-uncovered and add the two coverage tokens the control's own FORBID list names, so the control would be unsatisfiable for the wrong reason."
  - "The baseline is the step's own printed stat, re-derived by running it against the real FILELIST; gates=114 is 113 + one new gate name and lines=352 is 342 + ten rows."
  - "The toy id is EV-98 with one line appended to all ten registries and one row to the two toy requirement records plus the whole-file overlay copy: a uniform append leaves every declared overlay difference unchanged, so no DEVIATION entry is written (a DEVIATION entry would trip control-deviation-unused)."

patterns-established:
  - "Selftest inventory with a one-directional check: an inventory name the step did not run is `-missing`, and the accepting direction is proved in the same committed case so the rule cannot become one that refuses everything."
  - "Registry row + requirement flip + ten toy registries + the re-derived baseline in one commit (GUARDRAILS-v0.3 V7, V11): a split would be a `control-toy-drift` or `important-gates-unknown-id` finding, not a cosmetic ordering."

requirements-completed: [EV-07]

coverage:
  - id: D1
    description: "Each of the ten D-09-06 mechanisms resolves to a live inventory id of the kind that actually proves it: six `control` rows (five directories that already existed plus the new important-gates-excluded-stale) and four `mutant` rows whose case names come from the new data/expected/runner-selftest.ids."
    requirement: EV-07
    verification:
      - kind: integration
        ref: "ROOT=$PWD CHECK_DIR=$PWD/tools/check FILELIST=/tmp/fl13 sh tools/check/steps/important-gates.sh -> '-- important-gates pass gates=114 lines=352'"
        status: pass
      - kind: integration
        ref: "sh tools/check/run-controls.sh -> 'controls=401/401' including 'control ok   important-gates-excluded-stale'"
        status: pass
    human_judgment: false
  - id: D2
    description: "The runner-selftest step fails on an inventory name it does not run, so a registry row can never point at a renamed or deleted case, and that guard's failure is committed rather than demonstrated on a discarded scratch copy."
    requirement: EV-07
    verification:
      - kind: integration
        ref: "sh tools/check/steps/runner-selftest.sh -> '-- runner-selftest pass cases=14' and '-- runner-selftest proof runner-selftest-missing: st-no-such-case (observed: -- FAIL runner-selftest-missing: st-no-such-case)'"
        status: pass
      - kind: other
        ref: "scratch stand-in: renaming st-toy-drift in data/expected/runner-selftest.ids makes the step print '-- FAIL runner-selftest-missing: st-toy-drift-renamed', exit 1, and print no pass line; the file was restored and diffed clean"
        status: pass
    human_judgment: false
  - id: D3
    description: "The registry baseline is the step's own printed stat, not a quoted figure, and the phase table plus both mutant-source prose blocks name the third inventory in the same commit."
    requirement: EV-07
    verification:
      - kind: other
        ref: "awk '!/^#/ && NF' data/expected/important-gates.ids | wc -l -> 352; the same step run prints 'gates=114 lines=352'; grep -c '^d09-06-registration ' -> 10"
        status: pass
      - kind: integration
        ref: "a probe row 'probe-loop mutant st-toy-drift RTS-01' resolves (gates 113->114, lines 342->343) with the third inventory in the loop, and becomes 'FAIL important-gates-unknown-id: mutant st-toy-drift line 510' when the third file is removed from it"
        status: pass
    human_judgment: false
  - id: D4
    description: "All ten toy registries gain exactly one line each, the toy derived set still closes, and no exclusion row or DEVIATION entry is added."
    requirement: EV-07
    verification:
      - kind: integration
        ref: "toy tree: ROOT=$T ... sh tools/check/steps/important-gates.sh -> '-- important-gates pass gates=45 lines=46'"
        status: pass
      - kind: other
        ref: "git diff --cached --numstat over the 13 toy paths -> files=13 added=13 deleted=0; run-controls.sh output carries no control-toy-drift and no control-deviation-unused"
        status: pass
    human_judgment: false
  - id: D5
    description: "Ten registrations cost zero new faults and zero new steps: the phase's frozen invariants are unchanged."
    requirement: EV-07
    verification:
      - kind: integration
        ref: "sh tools/gate.sh -> 'gate: PASS steps=38/38 controls=401/401 ... fault-runs=238 ... faults=238/238 ... goldens=29 ... gates=114 lines=352 ... cases=14'"
        status: pass
      - kind: other
        ref: "awk '!/^#/ && NF' data/expected/faults/*.ids | wc -l -> 238; ls tools/check/steps/*.sh | wc -l -> 38; git ls-files data/golden | wc -l -> 29"
        status: pass
    human_judgment: false

duration: 26min
completed: 2026-11-25
status: complete
---

# Phase 13 Plan 04: The ten D-09-06 registrations and the re-derived baseline

**Ten must-fail mechanisms that had a guard and a stand-in but no registry row now resolve by name — six as `control` rows, four as `mutant` rows through a new committed selftest inventory that the step must actually run — with `faults=238`, `steps=38` and the baseline re-derived to `gates=114 lines=352` rather than quoted.**

## Performance

- **Duration:** ~26 min (approximate start; the three task commits span 22:24:08Z to 22:27:09Z and the two full gate runs plus two control runs follow them)
- **Started:** 2026-11-25T22:05:00Z (approximate)
- **Completed:** 2026-11-25T22:31:28Z
- **Tasks:** 3
- **Files modified:** 25 (6 created, 19 modified) — exactly the plan's `files_modified` list, no delta

## Accomplishments

- **`data/expected/runner-selftest.ids`** names the four D-09-06 mechanisms whose only proof is an in-step case of `runner-selftest`, and `tools/check/steps/important-gates.sh` reads it as a third file in the existing `mutant` inventory loop — one line changed, no new mechanism kind. The two prose blocks that document the mutant sources (`important-gates.sh`'s `# Resolution of <id> by mechanism:` and the registry's own header) name it in the same commit, so neither documented invariant is stale.
- **`tools/check/selftest-inventory.sh`** is the guard: an absent inventory is `runner-selftest-inventory: <path>`, an inventory with no non-comment line is `runner-selftest-inventory: no cases`, and each name the step did not run is `runner-selftest-missing: <name>`. It lives in a file of its own for the `tools/check/pass-line.sh` reason (review WR-02) — a guard inlined in the step would be reachable only by the step running itself.
- **The `st-no-such-case` stand-in is committed as runner-selftest's fourteenth case.** It renames one line of the real inventory into `$tmp`, requires the guard to refuse it with `runner-selftest-missing: st-no-such-case`, and requires the same guard to accept the real inventory against a complete `ran` list. The accepting half is what keeps the refusal from being a rule that refuses everything; `cases=13` becomes `cases=14` because the guard's failure is now a case, not a scratch copy the commit throws away.
- **`tools/check/controls/important-gates-excluded-stale`** gives the sixth Phase 13 mechanism its own directory, so its registry row resolves as a `control` id instead of only as a co-firing `EXPECT` line inside `important-gates-complete-uncovered`. One stale row (`FND-96`) and nothing else, with the good tree's `FND-98` row kept so no coverage token can fire, and four `FORBID` tokens pinning that it failed for that reason alone.
- **The ten rows and the baseline land together**: `d09-06-registration` gains one `control` row in Task 2 and nine more in Task 3, `EV-07` flips `Complete` in the commit that carries the first row (`important-gates-unknown-id` refuses a row naming a `Pending` requirement), and the phase table gains its four-line Phase 13 block.
- **The 13-file toy extension is one line each, zero deletions, no `DEVIATION` entry**: `| EV-98 | Phase 13 | Complete |` into the two toy requirement records and the whole-file overlay copy, `toy-phase13 fault GOOD_FAULT EV-98` into all ten whole-file toy registries.

## Measured verification

Every `<automated>` command in the plan was run against the committed tree, and each `<fails_when>` was checked:

| Check | Observed |
|---|---|
| `runner-selftest` after Task 1 | `-- runner-selftest pass cases=14`, exit 0, zero lines starting `-- FAIL ` |
| `runner-selftest` proofs observed | `control-pass-line-not-last: profile`, `pass-line-not-last: profile (exit 2 observed)`, `memmap-generator-missing: cmake/IvsMemmapProbes.cmake`, and `runner-selftest-missing: st-no-such-case` |
| `important-gates` after Task 1 | `-- important-gates pass gates=113 lines=342` (unchanged: no row names the four new ids yet) |
| `important-gates` after Task 2 | `-- important-gates pass gates=114 lines=343` |
| `important-gates` after Task 3 | `-- important-gates pass gates=114 lines=352` |
| toy tree `important-gates` after Task 2 | `-- important-gates pass gates=45 lines=46` |
| `run-controls.sh` after Task 2 | `controls=401/401`, including `control ok   important-gates-excluded-stale` |
| `run-controls.sh` after Task 3 | `controls=401/401` |
| `files` after Task 2 | `-- files pass tracked=2721 binaries-listed=0 rights-pinned=4/4` |
| `integrity` after Task 1 / Task 2 | `-- integrity pass files=2717` / `-- integrity pass files=2721`, no `integrity-missing-spdx` and no `integrity-non-ascii` |
| `sh tools/gate.sh` (final) | `gate: PASS steps=38/38 controls=401/401 ... fault-runs=238 ... faults=238/238 ... tracked=2721 binaries-listed=0 rights-pinned=4/4 ... goldens=29 ... gates=114 lines=352 ... cases=14` |
| `dash tools/gate.sh` (final) | `gate: PASS ...`; `diff` of the two `gate:` lines is empty |
| `sh tools/hygiene.sh --tree` | `-- hygiene pass mode=tree hygiene-files=2721` |
| `ls tools/check/steps/*.sh \| wc -l` | `38` |
| `awk '!/^#/ && NF' data/expected/faults/*.ids \| wc -l` | `238` |
| `git ls-files data/golden \| wc -l` | `29` |
| `awk '!/^#/ && NF' data/expected/important-gates.ids \| wc -l` | `352` |
| `grep -c '^d09-06-registration ' data/expected/important-gates.ids` | `10` |
| `git ls-files \| wc -l` | `2715` → `2717` (Task 1, two new tracked paths) → `2721` (Task 2, four new control files) → `2722` once this SUMMARY is tracked |
| the 13 toy paths, `git diff --cached --numstat` | `files=13 added=13 deleted=0` |

Two guard mutations were driven, not assumed:

1. **The inventory guard.** Renaming `st-toy-drift` to `st-toy-drift-renamed` in `data/expected/runner-selftest.ids` makes the step print `-- FAIL runner-selftest-missing: st-toy-drift-renamed`, exit 1, and print no pass line. The file was restored and `diff`ed clean afterwards.
2. **The loop that reads the inventory.** A probe row `probe-loop mutant st-toy-drift RTS-01` resolves (`gates` 113→114, `lines` 342→343) with the third inventory in the `mutant` loop, and becomes `-- FAIL important-gates-unknown-id: mutant st-toy-drift line 510` when that third file is removed from the loop. The registry was restored and `diff`ed clean.

## The baseline arithmetic, re-derived

`lines=` and `gates=` are runtime stats (`important-gates.sh` prints them), so both were re-measured by running the step against the real `FILELIST`, never quoted from a document:

| | before this plan (measured at `c2680a7`) | after | arithmetic |
|---|---|---|---|
| `lines` | 342 | **352** | 342 + 10 rows |
| `gates` | 113 | **114** | 113 + one new gate name, `d09-06-registration` |

RESEARCH's predicted `lines=347` was `337 + 10` — the ten EV-07 rows alone — so it omitted the five rows the phase's other three requirements add: one `rights-ledger` (RTS-01), one `rights-naming` (RTS-02) and three `claim-ceiling` (EV-06). Those five rows landed in plans 13-01 and 13-03, taking the measured baseline to 342 before this plan; the row arithmetic for the phase as a whole is therefore `337 + 15 = 352`, and the measured value is the arithmetic over the committed file. Plan 13-03's own "Next Phase Readiness" note predicted `gates=117 lines=352`; the measured figure contradicts its `gates` half, because `gates` counts distinct gate *names* and this plan adds exactly one (13-03's baseline was already 113, so 113 + 1 = 114). The step's printed line is the authority for both.

## The EV-07 wording deviation, and where it is recorded

EV-07's sentence says each of the ten mechanisms "gains a `mutant` row over its existing selftest". This plan registers **six `control` rows and four `mutant` rows**, and the reason is measured rather than stylistic: five of the ten mechanisms are already proved by control directories that exist (`important-gates-complete-uncovered`, `important-gates-excluded-malformed`, `sources-src-token-whole`, `manifests-output-anchor`, `memmap-generator-refused`), and one (`important-gates-excluded-stale`) is proved only as a co-firing `EXPECT` line of another control, so this plan adds a directory of that name. A `mutant` row for those six would be a second registration of one proof, and inventing a selftest to re-prove an already-proved mechanism would add a case with no new evidence.

**Zero new faults holds**, and it is the reason the deviation exists at all: ten faults instead of registrations would cost about 4.4 hosted cpu-s — 82 % of the milestone's allowance — on the step already 29 % of the gate (GUARDRAILS-v0.3 V11).

The deviation is recorded in three places, all in this plan's commits:

- `.planning/STATE.md`, `## Accumulated Context` → `- [Phase 13]: 13-04: EV-07 registered as 6 control + 4 mutant rows (deviation from EV-07's literal "mutant row" wording: …)` — committed in `b1abccc`, because a SUMMARY alone is not where a later reader of the project state looks.
- `.planning/REQUIREMENTS.md:33`, beside the requirement sentence that still says "`mutant` row": the amendment GUARDRAILS-v0.3 §2.10 asks for beside a live sentence that no longer matches the delivered mechanism. The traceability row at `:107` and the checkbox both flipped to `Complete` in `2c32390`, the commit carrying the first of the ten rows.
- This SUMMARY, in this section.

## Task Commits

Each task was committed atomically:

1. **Task 1: A mutant inventory the selftest must actually run** — `b1abccc` (feat)
2. **Task 2: The new control directory, the EV-07 flip and the 13-file toy extension** — `2c32390` (feat)
3. **Task 3: The remaining nine registrations and the re-derived baseline** — `e93dbe8` (feat)

**Plan metadata:** the SUMMARY/STATE/ROADMAP commit that follows this file.

## Files Created/Modified

- `data/expected/runner-selftest.ids` — the four case names, with a header naming the owning plan, the four mechanisms and the one-directional invariant.
- `tools/check/selftest-inventory.sh` — the guard, with `#!/bin/sh` then the MIT SPDX line so `integrity` stays green.
- `tools/check/steps/runner-selftest.sh` — records the case names it passed in `$tmp/ran`, calls the guard before its tail, and adds the `st-no-such-case` stand-in; its `# Tokens:` line gains `runner-selftest-inventory: <path>`.
- `tools/check/steps/important-gates.sh` — the third inventory in the `mutant` loop, the `# Resolution of <id> by mechanism:` prose, and the four-line Phase 13 phase-table block.
- `data/expected/important-gates.ids` — the `d09-06-registration` comment block (the ten mechanisms, the six control directories, the four selftest cases, the reason no exclusion row is taken) and the ten rows; the header's mutant-source sentence names the third inventory.
- `data/expected/controls/important-gates.ids` — the `important-gates-excluded-stale` id, with the comment that explains why the mechanism needs its own directory.
- `tools/check/controls/important-gates-excluded-stale/{CHECK,EXPECT,FORBID}` and `overlay/data/expected/important-gates-excluded.ids` — the toy ledger plus one stale `FND-96` row.
- `.planning/REQUIREMENTS.md` — the EV-07 checkbox and traceability row flipped `Complete`, with the amendment sentence beside the unchanged requirement text.
- `.planning/STATE.md` — the `[Phase 13]: 13-04` deviation bullet.
- the 13 toy files — one appended line each.

## Decisions Made

- **`control` rows where a directory already proves the mechanism.** The alternative — minting four more selftest cases so all ten could be `mutant` rows — would add cases with no new evidence and would still leave `important-gates-excluded-stale` unproved as its own id.
- **The guard is a file, and its failure is a case.** A scratch rename reverted before the commit is not a stand-in GUARDRAILS §2.4 accepts, so `st-no-such-case` runs the rename on every gate run.
- **The stand-in's accepting half is load-bearing.** Without it, a guard that refused every inventory would satisfy the refusing half and the gate would be green on a rule that cannot pass.
- **The overlay keeps `FND-98`.** Dropping it makes `FND-98` Complete-and-uncovered, which adds `important-gates-complete-uncovered` and `important-gates-uncovered-gate` — two tokens the control's own `FORBID` list names, so the control would fail for a reason it exists to exclude.
- **`EV-98` and one appended line per file.** All ten registries ended with the same line and all nine overlays differ from `_good` by exactly one declared line, so a uniform append leaves every declared difference untouched and needs no `DEVIATION` entry.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] The plan's `git ls-files` baseline is stale, so the `files tracked=` prediction is three low**

- **Found during:** Task 2 verification.
- **Issue:** the plan's acceptance criterion predicts `-- files pass tracked=2718 binaries-listed=0 rights-pinned=4/4` with `[HEAD=2714 EXPECT==2718]`. The measured count at this plan's start (`c2680a7`) is **2715**, not 2712 (the plan's own context figure) and not 2714: plan 13-03's metadata commit added `13-03-SUMMARY.md` after the `2714` figure its task commit measured, and this plan's Task 1 then adds two new tracked paths (`data/expected/runner-selftest.ids`, `tools/check/selftest-inventory.sh`) which the Task-2 baseline does not count. This is the same defect plans 13-02 and 13-03 each recorded.
- **Fix:** no code change. The delta the criterion actually asserts is verified directly: `2715` → `2717` (Task 1, two new tracked paths) → `2721` (Task 2, the four new control files), and the substantive tokens of the pass line (`binaries-listed=0`, `rights-pinned=4/4`) match the plan exactly. The stale figure is recorded here rather than silently matched, per the phase constraint that a quoted count disagreeing with the file is a defect to record.
- **Files modified:** none (the plan text is a frozen artifact; the correction lives in this SUMMARY).
- **Verification:** `git ls-files | wc -l` → `2721`; the three task commits' `--stat` are `6 files changed, 68 insertions(+), 3 deletions(-)`, `17 files changed, 34 insertions(+)` and `1 file changed, 10 insertions(+)`, with `git show --diff-filter=D` empty on all three.
- **Committed in:** `b1abccc`, `2c32390`, `e93dbe8` (the trees the figures are measured on)

### Recorded process notes

**2. [Rule 3 - Process] The Step 0 protected-branch assertion reports `main` as protected**

- **Found during:** all three task commits.
- **Issue:** `gsd_run query git.base-branch --is-protected main` returns `true`, so the executor's default-branch guard would refuse the commit. This repository sets `git.branching_strategy: none` and the dispatch instruction is explicit: the executor runs sequentially on `main` and stays there. Plans 13-01, 13-02 and 13-03 committed the same way.
- **Fix:** committed on `main` as dispatched (`SEQUENTIAL=true`), with hooks enabled and no `--no-verify`.
- **Files modified:** none.
- **Verification:** `git rev-parse --abbrev-ref HEAD` → `main`; all three commits landed with the pre-commit hooks running.
- **Committed in:** `b1abccc`, `2c32390`, `e93dbe8`

---

**Total deviations:** 1 auto-fixed (1 Rule 1, a stale plan figure) and 1 recorded process note. The EV-07 wording deviation is a **plan-recorded** deviation, not an executor one: it is stated in the plan, resolved in `13-CONTEXT.md`, and carried into `STATE.md`, `REQUIREMENTS.md` and this SUMMARY.
**Impact on plan:** no scope creep and **no file-set delta** — all 25 paths in the three task commits are on the plan's `files_modified` list.

## Issues Encountered

- **`git ls-files` must be staged before the registry or the naming limb can see a file.** Every `<automated>` command builds `FILELIST` from `git ls-files`, so the two new files were staged before the first `integrity` and `files` runs. Unstaged, `integrity` would report `files=2715` and `runner-selftest.ids` would be absent from the naming scan's view of the tree.
- **The `text` step does not read `.planning/`, so no `[nc]` tag was needed.** The EV-07 amendment sentence was first written with an `[nc]` tag by habit; `steps/text.sh:27` selects only `README.md`, `CHANGELOG.md` and `docs/**/*.md`, so the tag was removed rather than left as an unexplained marker on a line with no claim word.
- **The estimate missed by ~6.4x.** `estimate.tokens` was 90,000 against a realized diff of 28,399 characters plus this SUMMARY's 28,278 → 14,169 tokens at chars/4. Recorded as measured, not adjusted: this is the fourth plan in the phase whose estimate priced the planning context rather than the diff, so the miss is systematic and the fix belongs in the estimator.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- **EV-07 is closed**: ten rows, six `control` and four `mutant`, zero new faults, `steps=38/38`, `goldens=29`. The registry now reads `gates=114 lines=352` and `controls=401/401`.
- **`data/expected/runner-selftest.ids` is a second inventory with teeth**: deleting a line from it makes the step red only if the corresponding case also stopped running — the direction that matters — and adding a line without a case is `runner-selftest-missing`. A future plan that renames a `st-*` case must move the inventory in the same commit.
- **`important-gates-excluded-stale` is the shape to copy** for a mechanism that currently fires only as a co-firing `EXPECT` line of another control: give it a directory of its own, one planted row, and a `FORBID` list that proves the finding is the intended one.
- **Plan 13-05 is the last plan in the phase** and owns the phase-boundary cost checkpoint, which recomputes the last-five `cpu_s`/`gate_s` medians from `.planning/ci-runs.tsv` by arithmetic. RESEARCH's measured median was already `361 + 5`, so unless the token line moved by this phase's priced 2.0 the honest verdict is `reprice`, not `discharged-within-allowance` — the checkpoint must state which.
- The sixth and last Phase 13 control directory is landed: the phase's priced addition is `6 × 0.31 = 1.86` cpu-s for directories plus `10 × 0.005 = 0.05` for rows, `1.91` of the `2.0` line, and no seventh directory was added.

## Self-Check: PASSED

- Created files exist: all six created paths are tracked (`git ls-files` → `2721`) and were read by the steps that judge them (`important-gates` on the real and toy trees, `runner-selftest`, `run-controls.sh`, `integrity`).
- Commits exist: `b1abccc`, `2c32390` and `e93dbe8` are ancestors of HEAD.
- Re-ran every acceptance criterion from all three tasks after the last edit: `runner-selftest cases=14`, `important-gates gates=114 lines=352`, toy tree `gates=45 lines=46`, `controls=401/401` with `control ok   important-gates-excluded-stale`, `files tracked=2721 rights-pinned=4/4`, `integrity files=2721`, `steps=38`, `faults=238`, `goldens=29`, 352 registry lines, 10 `d09-06-registration` rows, and the 13-toy-path diff at 13 added / 0 deleted.
- Re-ran the plan-level `<verification>` commands: all six pass, `sh tools/gate.sh` ends `gate: PASS`, and `dash tools/gate.sh` prints a byte-identical summary line.

---
*Phase: 13-rights-ledger-and-enforced-claim-ceiling*
*Completed: 2026-11-25*
