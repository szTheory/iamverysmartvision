---
phase: 16-owner-route-claim-and-release-posture
plan: 04
subsystem: release-posture
tags: [shared-object-refusal, committed-direction, witness-pair, release-decision-line, important-gates, requirement-flip, toy-registries]

requires:
  - phase: 15-build-only-host-and-fake-frontend-harness
    provides: "the two `files-unlisted-binary-*` controls, the `release-assets` case `so-member` and the `pkg-install` selftest `host-so-absent` — the three mechanisms this plan registers rather than builds"
  - phase: 16-03
    provides: "the tree this plan started from (`4a1e993`) and every token of the summary line, including `handoffs=8`"
  - phase: 09-registered-evidence-and-gate-cost
    provides: "the derived must-be-covered set (`important-gates`'s traceability parse) and the ten toy registries the derived set closes through"

provides:
  - "the **committed direction** of the shared-object refusal proved on the real tree: a tracked `probe.so` refused as `files-unlisted-binary: probe.so`, `probe.txt` at the same place with the same bytes not refused, the content scan naming neither, both files removed in the same task"
  - "`.planning/ops/RELEASE-OPS.md` section (i.1): the release-decision line quoted unchanged beside the values measured at `4a1e993`, the probe's transcript, and the three surfaces named"
  - "PKG-06 `Complete` in the checkbox, the traceability row, the `Last updated` line and the Coverage block"
  - "three `release-posture` rows in `data/expected/important-gates.ids` and the Phase 16 block in the step's phase table"
  - "the ten toy registries and three toy requirement records extended with `PKG-96`, so the derived set closes; `gates=119 lines=364` (toy 48/51)"

affects:
  - "16-05 (phase close): reads section (i.1) as the recomputation and `gates=119 lines=364` from the gate line, never from this SUMMARY"
  - "the first release after the phase: the line's wording is frozen and its three surfaces each have their own evidence"

actuals:
  tokens: 4882
  tasks: 2
  commits: 2
  plan_head_before: 4a1e9932ccc152b2dc54eabe2c28e8ac9c26483d
  plan_head_after: 298be257670b5242e458ccf452de7a04c15d6501

tech-stack:
  added: []
  patterns:
    - "a witness pair on the real tree: the same bytes under two extensions, one refused and one not, with no pass line — so the rule is shown to be the extension and the step shown not to refuse everything"
    - "the committed direction proved by tracking the path (`git add -f`), because an untracked-only probe would test the content limb instead of the name limb"
    - "a throwaway check recorded in a live record and deliberately not registered: a probe on the real tree is not a committed mechanism, so it is not a row and not a control"
    - "a requirement flip, its three registry rows and the thirteen toy lines in one commit, because the derived must-be-covered set closes in both directions"

key-files:
  created: []
  modified:
    - .planning/ops/RELEASE-OPS.md
    - .planning/REQUIREMENTS.md
    - data/expected/important-gates.ids
    - tools/check/steps/important-gates.sh
    - tools/check/controls/_good/tree/data/expected/important-gates.ids
    - tools/check/controls/_good/tree/.planning/REQUIREMENTS.md
    - tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md
    - tools/check/controls/important-gates-complete-uncovered/overlay/.planning/REQUIREMENTS.md
    - tools/check/controls/important-gates-{bad-req-phase5,bad-req-phase6,req-uncovered-phase5,req-uncovered-phase6,req-uncovered-phase7,uncovered-gate,unknown-id,unknown-id-phase5,unknown-id-phase6}/overlay/data/expected/important-gates.ids
    - .planning/WINDOWS.md

key-decisions:
  - "The probe is throwaway and unregistered: `release-posture` names the two controls and the selftest, and the fourth row the release-assets `so-member` case would have needed is omitted with its reason, because a `mutant` row resolves only through the three selftest inventories or a `<suite>.judge.<slug>`/`<suite>.mutant.<slug>` id."
  - "The probe stages with `-f` and reads `git ls-files`, so the file list is the list git would ship; an untracked probe would have exercised the content limb and proved the wrong thing."
  - "Section (i.1) is appended, not merged into section (i): the line, its quote block and its three numbered mechanisms keep zero deleted lines, and the note adds the measurement plus the one limb the mechanisms could not reach."

requirements-completed:
  - PKG-06

coverage:
  - id: D1
    description: "The committed direction of the shared-object refusal: a tracked path with a shared-object extension refused by name, the same bytes with an ordinary extension not refused, and the tree left clean"
    requirement: "PKG-06"
    verification:
      - kind: other
        ref: "sh tools/check/steps/files.sh over a `git ls-files` list containing staged probe.so and probe.txt -> '-- FAIL files-unlisted-binary: probe.so' and nothing for probe.txt, no pass line; binscan.sh binary names neither; probe removed, gate green"
        status: pass
    human_judgment: false
  - id: D2
    description: "The release-decision line recomputed with the values measured at the commit, wording unchanged, with the three surfaces distinguished"
    requirement: "PKG-06"
    verification:
      - kind: other
        ref: ".planning/ops/RELEASE-OPS.md section (i.1) + the gate line at 4a1e993: binaries-listed=0 prefix-files=11 pkg-selftest=20/20 release-selftest=12/12 controls=404/404"
        status: pass
    human_judgment: false
  - id: D3
    description: "PKG-06 Complete with three live `release-posture` rows and the derived set closed in the toy tree"
    requirement: "PKG-06"
    verification:
      - kind: other
        ref: "important-gates.sh -> 'pass gates=119 lines=364' (toy 'gates=48 lines=51'); run-controls.sh -> controls=404/404 with no toy-drift/deviation finding"
        status: pass
    human_judgment: false

# Metrics
duration: 15min
completed: 2026-11-26
status: complete
---

# Phase 16 Plan 04: The Committed Direction and the Recomputed Release Posture Summary

**A shared object committed to the tree is refused by name on the real tree — one finding for `probe.so`, none for `probe.txt` with identical bytes — and PKG-06 is `Complete` against the line recomputed at the commit rather than quoted.**

## Performance

- **Duration:** 15 min
- **Started:** 2026-11-26T10:20:06Z
- **Completed:** 2026-11-26T10:35:04Z
- **Tasks:** 2 of 2
- **Files modified:** 17 (no file created, no file deleted)

## The probe

Run on a clean tree at `4a1e993` (pre-probe `git status --porcelain` empty), with both paths tracked so the
file list is the list git would ship:

```sh
printf 'probe\n' > probe.so
printf 'probe\n' > probe.txt
git add -f probe.so probe.txt
git ls-files > /tmp/g16-so-fl
ROOT=$PWD CHECK_DIR=$PWD/tools/check FILELIST=/tmp/g16-so-fl sh tools/check/steps/files.sh
```

The files step's output, in full:

```
-- FAIL files-unlisted-binary: probe.so
CMake Error at tools/check/lib.cmake:40 (message):
  files: 1 failure(s)
```

`probe.txt` is named nowhere and the step prints no pass line. `binscan.sh binary` over the same list prints
nothing at all: it skips `probe.so` by name (`so` is in `tools/check/binary-ext.txt`) and `probe.txt` is clean
text. Two facts make this the committed direction: the path was **tracked** when the step read the list, and
the content scan **skips a name-listed extension**, so the refusal is the name. Both files were unstaged and
removed in the same task: `git ls-files` holds no path with any of `so`/`dylib`/`dll`/`bundle`,
`git status --porcelain` was empty, and `sh tools/gate.sh` read `gate: PASS steps=38/38 …` afterwards.

The probe was re-run once with the note in the work tree (to have a fresh transcript beside the record) and
produced the same single finding; it is not a gate step, CI never runs it, and no test id, control, fault or
golden was added for it.

## The recomputed line

`.planning/ops/RELEASE-OPS.md` gained section **(i.1)** — 63 inserted lines, **0 deleted** — carrying: the
commit (`4a1e993`) the values were measured on; the line quoted exactly, unchanged in this phase; the five
values read off that commit's closing gate line; the probe's command shape, its one finding, the ordinary
path's absence, and the two facts that make it the committed direction; and the `throwaway` sentence. The line
and its three numbered mechanisms above it are untouched, and the note says which limb the probe covers
(the tree, committed paths) that the two controls and the two selftests cannot reach from a copy.

## The acceptance

| Token | Before | After | Arithmetic |
|---|---|---|---|
| `gates` | 118 | 119 | +1 gate name (`release-posture`) |
| `lines` | 361 | 364 | +3 registry rows |
| toy `gates`/`lines` | 47/50 | 48/51 | +1 toy gate name, +1 toy row |
| `controls` | 404/404 | 404/404 | invariant — the rows name existing controls |
| `faults`/`fault-runs` | 238/238, 238 | same | invariant (§2.1) |
| `steps`/`goldens` | 38/38, 29 | same | invariant |
| `pkg-selftest`/`release-selftest` | 20/20, 12/12 | same | invariant: `host-so-absent`, `so-member` already exist |
| `claims`/`provisional`/`claim-refs`/`handoffs` | 29/28/27/8 | same | invariant (§3: no claim minted) |
| `tracked`/`files`/`hygiene-files` | 2786 | 2786 | invariant: no file created, no file deleted |
| `binaries-listed`/`prefix-files` | 0, 11 | 0, 11 | the posture's own two figures |
| `tests-expected`/`-executed`/`-skipped` | 5042/5038/4 | same | invariant |

The real registry re-derived this session prints `-- important-gates pass gates=119 lines=364`; the good toy
tree prints `-- important-gates pass gates=48 lines=51`. Both were obtained by running the step, not quoted:
the arithmetic above predicted exactly those numbers, and the measurement confirmed it rather than correcting
it.

## Task Commits

Each task was committed atomically:

1. **Task 1: The committed direction, proved on the real tree, and the recomputed release-decision line** —
   `9c621a8` (docs) — 63 added, 0 deleted
2. **Task 2: PKG-06 accepted with its three rows, the thirteen toy lines and the re-derived baseline** —
   `298be25` (feat) — 16 files, 13 toy lines added with zero deletions on those paths

**Plan metadata:** `docs(16-04): complete the committed-direction and release-posture plan` (this commit)

_Measured over `4a1e993..298be25`: `commits: 2`, `git diff | wc -c` / 4 = **4882** actual tokens against the
plan's 55,000-token estimate (0.09). 17 files, 122 insertions, 5 deletions, no file created or deleted._

## Files Created/Modified

- `.planning/ops/RELEASE-OPS.md` — section (i.1), the appended recomputation and the probe's transcript
- `.planning/REQUIREMENTS.md` — PKG-06 checkbox, traceability row, `Last updated`, Coverage sentence
- `data/expected/important-gates.ids` — three `release-posture` rows with their per-surface comments
- `tools/check/steps/important-gates.sh` — the Phase 16 block in the phase table
- ten toy registries + three toy requirement records — one `PKG-96` line each (13 files, 13 added lines)
- `.planning/WINDOWS.md` — row 11, this plan's one recorded defect, waived with its measured reason

## Decisions Made

- **The `release-assets` `so-member` case is deliberately not a row.** The `mutant` mechanism resolves only
  through `data/expected/pkg-selftests.ids`, `ci-summary-selftests.ids`, `runner-selftest.ids`, or a
  `<suite>.judge.<slug>`/`<suite>.mutant.<slug>` test id. `so-member` is a case of the `release-assets` step,
  not of a suite inventory or of a selftest list, so registering it would make the row
  `important-gates-unknown-id`. The archive surface is therefore named in section (i.1) and in the row
  comment as evidence, and is not registered. This is a recorded reason, not an oversight.
- **The probe is not registered either.** A row names a *committed* mechanism that can fail on a copy; the
  probe runs on the real tree for the length of one task. Registering it would claim a mechanism that does not
  exist between runs.
- **The Phase 16 table block says both of the above in words**, so a reviewer reading only the step's header
  does not infer that the fourth surface was forgotten.

## Deviations from Plan

### Plan defects (recorded with the measured reason, not silently satisfied)

**1. [Plan defect] Task 1's `<automated>` opens with "the tree is clean" and also asserts the note's contents**

- **Found during:** Task 1, running the verify block in its given order.
- **Measured:** the block's first assertions are `git status --porcelain > /tmp/g16-04-pre; test ! -s
  /tmp/g16-04-pre`, and its last assertions grep the appended note for `committed direction`,
  `files-unlisted-binary: probe.so`, `probe.txt`, `host-so-absent`, `so-member` and `throwaway`. The note is
  written by the same task, so it makes the work tree dirty (` M .planning/ops/RELEASE-OPS.md`); the opening
  assertion is therefore satisfiable only *before* the note exists, and the note assertions only *after*. The
  two halves are mutually exclusive in a single run.
- **Substitution:** the block was executed in its intended order. The probe half ran first, on a clean tree
  (pre-probe status empty, captured in `/tmp/g16-04-pre`), and every one of its assertions passed — one
  finding, `probe.txt` absent from the output, no pass line, neither path in the content scan, both files
  removed, `git ls-files` with no shared-object extension, `git status --porcelain` empty. The note was then
  appended, and the probe was re-run with the note present (same single finding), after which every note
  assertion and the post-gate token check passed. Nothing was skipped and no assertion was weakened; the only
  clause not literally satisfiable is the block's opening cleanliness line, which was satisfied immediately
  before the note was written.
- **Ledger:** `.planning/WINDOWS.md` row 11, kind `deviation`, waived with this reason, so a later ship gate
  sees it without reading this SUMMARY. `open_count` stays 3, `waived_count` 7 → 8.

No auto-fixed issues: the plan's mechanism held on the first attempt, and no Rule 1–3 fix was needed.

## Not this plan's to do, by name

- **No new gate step, fault, golden, control or selftest.** `steps=38/38`, `faults=238/238`, `goldens=29`,
  `controls=404/404`, `pkg-selftest=20/20`, `release-selftest=12/12` — all unchanged. The three rows are
  pointers to mechanisms plan 15-02 built.
- **No claim was minted.** `claims=29 provisional=28 claim-refs=27`, unchanged; GUARDRAILS §3's Phase 16
  ceiling is "no claim" and this plan cites none.
- **`docs/packaging.md` did not move.** It already states the non-membership for a reader; this plan's record
  is the recomputation, and the plan's `files_modified` does not name it.
- **The phase close is 16-05's**: the two-shell and container evidence, the token reconciliation, the cost
  verdict, the signed `16-VALIDATION.md` and the milestone-capability statement.
- **Nothing was pushed, tagged or published**, and no `.planning/config.json` or `.planning/state.json` was
  staged (D-31, D-42).

## Issues Encountered

- The probe's second run happened with `M .planning/ops/RELEASE-OPS.md` in the tree, so `git ls-files` differed
  from the first run's list by exactly that one path — already tracked, so the list's *content* was identical
  and the finding unchanged. That is why re-running it was safe rather than a second experiment.
- The toy registries had to be extended in all ten copies *and* the three requirement records in the same
  commit; the control runner's `control-toy-drift` check is what proves uniformity, and it stayed silent at
  `controls=404/404`. `PKG-96` was confirmed free (`grep -rn -- 'PKG-96'` over the good tree and the nine
  overlays printed nothing) before the append.
- One mistake in the process, corrected before the commit: an exploratory `gsd_run windows append` with
  placeholder arguments added a junk row (id 12, file `x`, description `y`). It was removed from both the table
  and the ledger's JSON block, the frontmatter counters were restored (`total_count` 12 → 11,
  `open_count` 5 → 4), and the JSON was re-parsed to confirm 11 rows with max id 11 before row 11 was waived.
  The final ledger reads `open_count: 3`, `waived_count: 8`, `total_count: 11`.

## Known Stubs

None. Both `<verify>` blocks were executed; no `<automated>` assertion was skipped (the one clause that could
not hold in a single run is row 11 above, with its measured reason and its substitution), and every printed
figure in this SUMMARY is quoted from a log produced in this session.

## Threat Flags

None. The change adds no network surface, no executable input and no tracked artifact: it appends one note to a
live record, edits a requirement record, appends three registry rows and thirteen toy lines, and extends a
comment table. The threat register's T-16-14 to T-16-18 are each exercised above — the probe proves the name
rule on a committed path and leaves no state (T-16-14, T-16-18), `prefix-files=11` and `host-so-absent` hold
the installed-prefix limb with no install rule added (T-16-15), the note states only tokens of the gate line
measured at a named commit (T-16-16), and the flip, the three rows and the thirteen toy lines landed in one
commit with `important-gates`'s two directions and the toy-drift check enforcing it (T-16-17). T-16-SC: no
package manager ran.

## User Setup Required

None.

## Next Phase Readiness

- 16-05 can read `gates=119 lines=364` off the gate line and section (i.1) as the recomputed posture; it needs
  neither this SUMMARY's arithmetic nor a second probe.
- The release-decision line's wording is unchanged and frozen; what remains for the close is the cost verdict,
  the container evidence and the milestone-capability statement.

---

## Self-Check: PASSED

- `.planning/ops/RELEASE-OPS.md` contains section (i.1) — FOUND
- `data/expected/important-gates.ids` holds the three `release-posture` rows — FOUND
- `.planning/REQUIREMENTS.md` PKG-06 traceability row reads `Complete` — FOUND
- `9c621a8` — FOUND (ancestor of HEAD)
- `298be25` — FOUND (HEAD)
- `git ls-files | grep -E '[.](so|dylib|dll|bundle)$'` — empty
- `sh tools/gate.sh` at `298be25` — `gate: PASS steps=38/38 controls=404/404 … gates=119 lines=364 …`
