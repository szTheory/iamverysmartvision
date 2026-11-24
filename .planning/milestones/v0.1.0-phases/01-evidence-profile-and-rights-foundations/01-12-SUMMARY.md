---
phase: 01-evidence-profile-and-rights-foundations
plan: 12
subsystem: publication
tags: [publication, history-rewrite, hygiene, ci, github, fnd-05]
requires:
  - phase: 01-evidence-profile-and-rights-foundations
    provides: "hygiene --history tool (01-11), gate workflow (01-13), README and LICENSE (01-11), blind second-reader gate step (01-10)"
provides:
  - "rewritten local history with no home or scratch path in any tree or message and noreply identities only"
  - "public repository szTheory/iamverysmartvision (PUBLIC, default branch main), pushed with gh"
  - "green first and second hosted runs of the gate workflow"
  - ".planning/CI-EFFICIENCY.md with measured run rows and decisions"
affects: [every later phase (all commits are public from the first push; run the gate before every push)]
actuals:
  tokens: 3300
  tasks: 2
  commits: 1
plan_head_before: a41281e7ae5660922f0f7212f4c588598120ed67
plan_head_after: 7c25c55049bdba30cb3a9867f2d5503030d2e3ad
tech-stack:
  added: []
  patterns:
    - "Tree-filter scrub kept in the session scratchpad, written with cat redirection back into the file so file modes survive"
    - "Fix-forward only after first push; gate and history hygiene run before every push"
key-files:
  created:
    - .planning/CI-EFFICIENCY.md
  modified: []
key-decisions:
  - "Single CI job, no cache, no matrix, no path filter; macOS leg only with the C build; revisit the ubuntu-24.04 pin after 2026-11-19"
  - "Run 2 recorded in the log now (numbers did not change any decision)"
requirements-completed: [FND-05]
duration: about 15 min
completed: 2026-11-09
status: complete
---

# Phase 1 Plan 12: Publication Summary

**Local history was rewritten (61 commits before and after) so `sh tools/hygiene.sh --history` passes, a fresh clone passes `dash tools/gate.sh`, the repository went public as `szTheory/iamverysmartvision` on `main`, and the first two hosted gate runs were green in 16 s and 11 s.**

## Performance

- **Tasks:** 2 of 2.
- **Hosted gate runs:** run `42720541081` took 16 s total (queue 4, setup 2, gate 6); run `42720666825` took 11 s (queue 2, setup 2, gate 4). See `.planning/CI-EFFICIENCY.md`.

## Accomplishments

- **History scrub (Task 1, tracer).** Pre-flight: `sh tools/gate.sh` and `dash tools/gate.sh` ended `gate: PASS`, `--tree` passed. Before the rewrite `sh tools/hygiene.sh --history` reported 53 `hygiene-history-home-path` findings and nothing else (no message, identity, secret or binary findings). A backup bundle was written to the session scratchpad, verified, and deleted after publication was confirmed. A `--tree-filter` (script kept in the scratchpad, not tracked) replaced scratch-directory paths with `<local-scratch>`, absolute paths into this repository with `.`, the projects directory with `..` and any other home path with `~`. No message filter was needed because the history scan reported no message findings.
- **Cleanup and proof.** `.git/refs/original` removed, reflog expired, `git gc --prune=now` run, the stale local phase branch deleted (`git branch -D`), so `main` is the only ref. After the rewrite: commit count 61 before, 61 after; `sh tools/hygiene.sh --history` last line `-- hygiene pass mode=history commits=61`; every author and committer email is `szTheory@users.noreply.github.com`; `sh tools/gate.sh` still passes on the rewritten HEAD.
- **Clean-clone gate.** A fresh `git clone` in a temporary directory outside the repository ran `dash tools/gate.sh`: exit 0, last line starts `gate: PASS steps=16/16 controls=84/84`, `refs-verified=0/4` (no `refs/` cache). The clone was deleted.
- **Publication (Task 2).** `hygiene --history` (commits=61) and `sh tools/gate.sh` were re-run back to back, then `gh repo create szTheory/iamverysmartvision --public --source . --remote origin --push` with the plan's description. `gh repo view` reports `PUBLIC main`; URL https://github.com/szTheory/iamverysmartvision. No tag exists and `gh release list` prints nothing (D-24).
- **First CI run.** `gate` run `42720541081` on `a41281e` concluded `success`; no fix-forward commit was needed. The CI efficiency log was opened with real numbers and decisions (commit `7c25c55`), and its push produced run `42720666825`, also `success`.
- **FND-05.** This is the last FND-05 plan: public-repo hygiene (01-11), history rewrite, clean-clone gate and publication are complete, so FND-05 is marked complete.

## Task Commits

1. **Task 1 (tracer): pre-flight, backup, history rewrite, hygiene, clean-clone gate** - no new tracked file by design; the result is the rewritten history itself (new HEAD hashes; hashes cited in earlier SUMMARY files are now stale, accepted by the plan).
2. **Task 2: public repo, first CI run, CI-EFFICIENCY.md** - `7c25c55` (docs)

**Plan metadata:** the commit that carries this SUMMARY, the CI log's second row, and the STATE, ROADMAP and REQUIREMENTS updates.

## Files Created/Modified

- `.planning/CI-EFFICIENCY.md` - run table with queue, setup, gate and total seconds, decisions, next review.
- `.planning/phases/01-evidence-profile-and-rights-foundations/01-12-SUMMARY.md` - this file.

## Decisions Made

- Keep one job with no cache, matrix or path filter: the whole run is 11 to 16 s and the gate has nothing to download or build.
- `cancel-in-progress` stays pull-request only so every `main` commit keeps a result.
- A macOS leg waits for the C build in Phase 2; the `ubuntu-24.04` pin is revisited after 2026-11-19.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] The RESEARCH sed set ends with `mv`, which would reset file modes in every rewritten commit**
- **Found during:** Task 1 (writing the filter script)
- **Issue:** `sed ... > tmp && mv tmp file` replaces the file with a new inode and default mode, so any executable bit in a historical tree would be lost.
- **Fix:** the script writes the sed result to a temporary file and then copies its content back with `cat tmp > file`, preserving the original mode; the temporary file is removed. The sed expressions are unchanged and still avoid `sed -i`. Also added `/home/<name>` to the file-selection grep to match the hygiene home-path pattern.
- **Files modified:** none tracked (script is in the scratchpad)
- **Verification:** `hygiene --history` passes; the gate passes on the rewritten HEAD and from a clean clone.

**2. [Rule 3 - Blocking] The scratchpad scrub script supplied for the plan was an unfinished stub**
- **Found during:** Task 1
- **Issue:** the pre-existing scratchpad file held a no-op loop, not the filter.
- **Fix:** wrote a new filter script in the scratchpad from the RESEARCH sed set; the stub was left untouched.

**3. [Rule 2 - Missing critical] Row for run 2 added to the CI log now rather than at the next review**
- **Issue:** plan allows deferral when numbers do not change a decision; they did not.
- **Fix:** recorded the row in the same commit as this SUMMARY so the log is complete at publication time. No decision changed.

---

**Total deviations:** 3 (one bug-class, one blocking, one documentation-only). **Impact on plan:** none; no check was weakened.

## Issues Encountered

- Immediately after `gh repo create`, `gh repo view` printed an empty default-branch name for a moment; a re-query a few seconds later returned `PUBLIC main`.
- The gate line on the author machine shows `refs-verified=1/4` because a local `refs/` cache exists; a clean clone and the hosted run show no cache.

## Known Stubs

None.

## Threat Flags

None. The only new surface is the public GitHub repository and its workflow, which are in the plan's threat model (T-01-31 through T-01-35, all mitigated: history rewritten before the first push, noreply identities only, bundle held outside the repository and deleted after verification, no token written to any file, no force-push).

## Next Phase Readiness

Phase 1 plans are complete; all later commits are public from the first push, so every push must follow a passing `sh tools/gate.sh` (hygiene included). No tag or release exists and none was created.

## Self-Check: PASSED

- `.planning/CI-EFFICIENCY.md` exists; `7c25c55` is an ancestor of HEAD.
- `.git/refs/original` absent; no tag; no release; `gh repo view` reports `PUBLIC main`.
