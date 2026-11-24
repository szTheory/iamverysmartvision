---
phase: 08-stic-completion-save-state-performance-baselines
plan: 20
subsystem: Phase 8 wave D, plan 1 of 2 - the stale-text sweep - false public and planning statements fixed before the tag, requirements-completed backfilled into the 39 Phase 4 to 7 SUMMARY files that lacked it, the three-source check, and the accepted tech debt record
tags: [D, REL-01, D-34, D-36, H22, stale-text, changelog, packaging, roadmap, requirements-completed, accepted-debt, T-08-45, T-08-46]
status: complete

requires:
  - phase: 08-stic-completion-save-state-performance-baselines
    provides: "plan 08-19 end state: steps=38/38 controls=380/380 faults=235/235 fault-runs=235 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 claims=28 provisional=27 gates=90 lines=306 goldens=29 pkg-selftest=19/19 state-rows=88 state-cuts=18 (re-measured with sh tools/gate.sh before the first edit: identical, 0 FAIL lines)"
provides:
  - "README.md: no sentence says that no release exists; releases, when they exist, are listed on the repository's releases page as pre-releases of an exact tested commit, none can run a program, publication follows the delegated release flow and its checks, and the working name is not cleared"
  - "docs/packaging.md: eleven installed files and five headers as pinned, the four calls of ivs_state.h, the other four headers the umbrella brings in, Release content as the workflow produces it, claim tags on the consumer replays"
  - "CHANGELOG.md: the section [0.1.0] (the former Unreleased body, eleven files, five headers, the format and API versions, how to check a download; the claim table region byte-identical), [Unreleased] holds one line"
  - "core/include/ivs/ivs_machine.h (two comments) and data/api/api-effects.json (one call record text): the state digest composite is version 3 since plan 06-05"
  - ".planning/ROADMAP.md, REQUIREMENTS.md, STATE.md, PROJECT.md: decided gates no longer shown open, the Phase 8 plan list with a goal each, the validated list on recorded terms, the register rows OQ8 and OQ11; .planning/ops/PLAN-LESSONS.md: Phase 7 lessons 13 to 18"
  - "39 SUMMARY files of Phases 4 to 7 with one added requirements-completed line each; the three-source check over Phases 1 to 7: 45 requirements, 0 findings; 08-ACCEPTED-DEBT.md"
affects: [08-21]

actuals:
  tokens: 1380
  tasks: 3
  commits: 3
plan_head_before: 2d37aa65bf11bc6bb0058a32579d517bd9905b67
plan_head_after: d4788f9bf0667fece43233c2ee2bd6a40e6bc121

key-decisions:
  - "The sweep re-greps every statement by phrase and never trusts a line number: three of the plan's named statements were already half fixed by earlier plans (the installed file list of docs/packaging.md, the plan list of the ROADMAP, the persistence sentence of STATE) and one did not exist at all (an owner-authority sentence in the README), so each was judged on the text that is there"
  - "The CHANGELOG section is created now, headed [0.1.0] with no date and a sentence that the release's own date is on the releases page: the release script takes the notes from the section headed with the header's version, and no placeholder reaches the tagged tree"
  - "The backfilled requirements-completed value is the plan's own requirements field verbatim, with a comment that the closure evidence is the phase VERIFICATION record: the line is an index for the milestone audit and not a claim, so an over-inclusive value cannot close a requirement the verification record does not name"
  - "Found-but-not-fixed items are recorded, not patched: the internal comment in core/src/machine/machine_digest.c that still says composite version 2 is a source file and the plan forbids a source change, so it sits in 08-ACCEPTED-DEBT.md section 5"

key-files:
  created:
    - .planning/phases/08-stic-completion-save-state-performance-baselines/08-ACCEPTED-DEBT.md
    - .planning/phases/08-stic-completion-save-state-performance-baselines/08-20-SUMMARY.md
  modified:
    - README.md
    - CHANGELOG.md
    - docs/packaging.md
    - core/include/ivs/ivs_machine.h
    - data/api/api-effects.json
    - .planning/ROADMAP.md
    - .planning/REQUIREMENTS.md
    - .planning/STATE.md
    - .planning/PROJECT.md
    - .planning/ops/PLAN-LESSONS.md
    - "the 39 SUMMARY files of Phases 4 to 7 listed in the backfill section (04-18 to 04-24, 05-01 to 05-12, 06-01 to 06-14, 07-01 to 07-06), frontmatter line only"

requirements-completed: []  # REL-01 completes after its second stage (the records pull request after the merge, D-28); this plan marks no requirement

completed: 2026-11-15
---

# Phase 8 Plan 20: the stale-text sweep Summary

**The tree that gets the tag now says true things. The README no longer says that no release exists (it says what a release is and is not, in a form that does not go stale at the tag, and that publication follows the delegated release flow with the working name stated as not cleared); `docs/packaging.md` lists the eleven installed files and five headers as pinned and states the release content as the workflow produces it; `CHANGELOG.md` has the section `[0.1.0]` that the release script reads; the digest composite is called version 3 in the public header and the call record; the ROADMAP, REQUIREMENTS, STATE and PROJECT records say which gates are delegated decisions (D1 to D9, D12), which are open owner hard stops that were not performed (D10, D11) and which is deferred (D13), the Phase 8 plan list exists with a goal each, the validated requirements are listed on recorded terms with REL-01 excluded, and the persistence and performance-target decisions have register rows. The 39 Phase 4 to 7 SUMMARY files that lacked `requirements-completed` gained one line each, and the three-source check reads 45 requirements and 0 findings (11 requirements of Phases 5 and 6 had no SUMMARY before). `08-ACCEPTED-DEBT.md` records the six draft validations, the carried items and everything deferred out of Phase 8. No test id, fault, control, step or golden changed: sh and dash read `gate: PASS` with the plan-start counts on every task tree, and the container gate passed on the Task 1 tree, the Task 3 tree and the closing tree.**

## Commits

| Commit | Task | What |
|--------|------|------|
| `9c192d5` | 1 (tracer) | README, `docs/packaging.md`, `CHANGELOG.md` section [0.1.0], the composite version in `ivs_machine.h` (two comments) and `api-effects.json` (one text) |
| `17d67fc` | 2 | ROADMAP, REQUIREMENTS, STATE body, PROJECT, PLAN-LESSONS |
| `d4788f9` | 3 | the 39 `requirements-completed` lines, `08-ACCEPTED-DEBT.md`, the D10 item count of the PROJECT register |
| the commit that adds this file | | this SUMMARY, STATE.md and ROADMAP.md (plan counters, metric row, decisions) |

## The statements fixed, with their files

Re-grepped by phrase at the start of each task (the line numbers of the plan were not used). "Task" gives the commit.

| # | Stale statement | Fix | File | Task |
|---|-----------------|-----|------|------|
| 1 | "No release has been published." | a tense-free statement: releases, when they exist, are listed on the releases page as pre-releases of an exact tested commit that carry the source archive, the install archive and the checksums; none can run a program; the claims about behaviour stay provisional | `README.md` | 1 |
| 2 | no sentence about the authority for a release (the plan expected "owner authority"; the README has none, see deviation 2) | publication follows the delegated release flow of `.planning/AUTONOMY.md` and its checks; the working name is not cleared; neither a measurement on a console nor a run with real EXEC or GROM images was performed | `README.md` | 1 |
| 3 | "The prefix holds exactly these files" (no count) and a row that lists only "snapshot size and snapshot save" | eleven files, five headers, the static library, four CMake package files and the licence; the header row names size, save, restore and the instance-free describe | `docs/packaging.md` | 1 |
| 4 | "it brings in the other three headers" | the other four headers (`ivs_machine.h`, `ivs_run.h`, `ivs_state.h`, `ivs_status.h`) | `docs/packaging.md` | 1 |
| 5 | Release content: "the installed package and the source tarball", "the snapshot size and snapshot save calls" | four things (source archive, the eleven-file install archive, `SHA256SUMS`, the notes as the matching `CHANGELOG.md` section), a draft pre-release created by the tag-triggered workflow, the consumer built against the very archive that ships, the four calls of `ivs_state.h` | `docs/packaging.md` | 1 |
| 6 | claim ids missing where the consumer replays are named | `[claim:CL-0022]` on the third replay, `[claim:CL-0024]` on the fourth, CL-0015, CL-0022 and CL-0028 in Release content (CL-0028 stated as held by the core's state suite, not by the fifth replay) | `docs/packaging.md` | 1 |
| 7 | "ten files ... the four public headers"; no section for the release path | `## [0.1.0]` holds the former Unreleased body with eleven files and five headers, a format and API versions block and a how-to-check block; `## [Unreleased]` says nothing is unreleased yet | `CHANGELOG.md` | 1 |
| 8 | "the composite, which is version 2 since plan 05-03" | version 3 since plan 06-05 (see the diff below) | `core/include/ivs/ivs_machine.h` | 1 |
| 9 | "composite version 2" in the digest call comment | "composite version 3, plan 06-05" | `core/include/ivs/ivs_machine.h` | 1 |
| 10 | "the composite is version 2 over domains 1..4 since plan 05-03" | version 3 since plan 06-05, framing as in version 2 | `data/api/api-effects.json` | 1 |
| 11 | overview: a release needs "owner authority" (REL-01, D9) | publication follows the delegated release flow and its pre-publication checks, the working name not cleared as a caveat | `.planning/ROADMAP.md` | 2 |
| 12 | overview: D1 and D3 "still open owner decisions", "none is decided" | D1 to D9 and D12 delegated, D10 and D11 open hard stops not performed, D13 deferred | `.planning/ROADMAP.md` | 2 |
| 13 | gate timeline intro: "None of these are decided" and the D9 row | the true status in the intro; the D9 row says the delegated release flow applies at the Phase 8 close | `.planning/ROADMAP.md` | 2 |
| 14 | Phase 8 decision gates: "D9 (owner gate, open)", "REL-01 records readiness only; it does not publish", persistence "decided by owner and architecture" | D9 delegated with checks P1 to P8, D10 and D11 hard stops not performed, persistence decided in Phase 8, D13 deferred | `.planning/ROADMAP.md` | 2 |
| 15 | Phase 8 success criterion 5: "Nothing is published without owner gate D9" | publication follows the delegated release flow, a draft pre-release published only after P1 to P8, name and D10 and D11 stated | `.planning/ROADMAP.md` | 2 |
| 16 | Phase 8 plan list: bare file names and "19/21 plans executed" | the count "21 plans in 21 sequential waves" and one goal line per plan 08-01 to 08-21 (marks kept as the plan-complete mechanism set them, deviation 4); the cross-cutting end-state line added | `.planning/ROADMAP.md` | 2 |
| 17 | the plan-time "open" gate statements of Phases 1, 4, 5 and 7 | a status note at the end of each (found by the sweep, not named by the plan) | `.planning/ROADMAP.md` | 2 |
| 18 | REQUIREMENTS D6, D7, D8, D12 rows "open" (and the D5, D9 wording, the intro "None are decided") | the delegated-decision wording as PROJECT.md records it, with the dates and decision ids | `.planning/REQUIREMENTS.md` | 2 |
| 19 | "The recommended first profile ... (recommendation, not an owner decision)" and the footer date of 2026-11-14 | the profile is the delegated decision D3; the footer names this sweep and the Phase 8 close-out | `.planning/REQUIREMENTS.md` | 2 |
| 20 | STATE blockers: "Still open, decided by fan-out protocol when their phase needs them: D4, D6, D7, D8, D12", the open-question line naming OQ4 and OQ9 as open | decided, hard-stop and deferred gates listed with their references, the persistence and performance-target decisions, only what is still open kept | `.planning/STATE.md` | 2 |
| 21 | PROJECT: "Validated: (None yet - ship to validate)" | eleven bullets, one per requirement group with its ids, each `complete on documented terms, claims provisional, nothing measured`, REL-01 excluded until its second stage, SCN-02 and PRF-01 with their stated caveats | `.planning/PROJECT.md` | 2 |
| 22 | PROJECT: no register row for the persistence policy and the performance targets | rows OQ8 (from the text of plan 08-17) and OQ11 with their reversal evidence; notes on open questions 8 and 11 | `.planning/PROJECT.md` | 2 |
| 23 | PROJECT: "D6, D7, D8 and D12 ... are decided by the fan-out protocol when their phase needs them", three recommendations shown "Pending" | past tense with the pointer to the rows; the three statuses state what was applied (Slice 0 in Phase 4, D3, project-authored diagnostics) | `.planning/PROJECT.md` | 2 |
| 24 | PROJECT: the D10 row counts "42 items" | 47 items (42 before the five hand-offs of Phase 8; the gate reads `d10-items=47`) | `.planning/PROJECT.md` | 3 |
| 25 | PLAN-LESSONS: no lessons from Phase 7 | six lessons, 13 to 18 (below) | `.planning/ops/PLAN-LESSONS.md` | 2 |

That is 25 statements (the plan's floor is nine). Three things the plan named were not stale as written, and the sweep says so instead of editing them: the Phase 8 plan entry of the ROADMAP (a list of bare file names already stood there and no text said the plans were not yet known), the persistence sentence of STATE (plan 08-17 had already replaced it) and the README authority sentence (it does not exist).

**The header diff** (acceptance quote; every changed line is inside a comment, see deviation 1):

```
-  * domain version 1; none is part of the composite, which is version 2 since plan 05-03 over
-  * domains 1..4 (the PSG state joined domains 2 and 4). Every other value is refused. */
+  * domain version 1; none is part of the composite, which is version 3 since plan 06-05 over
+  * domains 1..4 (the STIC registers and state joined domains 2 and 4, framing as in version 2). Every other value is refused. */
-  * composite version 2; the observation domains are not in it, D-10, D-18). The serialization is field by field and
+  * composite version 3, plan 06-05; the observation domains are not in it, D-10, D-18). The serialization is field by field and
```

The claim table region of `CHANGELOG.md` is byte-identical before and after (the md5 of the text between its two markers is the same at the plan head and at `9c192d5`, and the drift check of the gate stays green). `sh tools/check/release-assets.sh changelog 0.1.0 CHANGELOG.md` reads `release-assets changelog ok 0.1.0`.

## The lessons added (PLAN-LESSONS 13 to 18)

13. reference figures are not as-built figures (gate-window starts 56 ticks early, a two-word idle branch, a constant above the 10-bit immediate limit); 14. the oracle freeze must name its commit and be proved by `git merge-base --is-ancestor`; 15. a skipped id is a counted id (`skip-ok`, exit 13, a visible SKIPPED line); 16. a play word needs a claim tag; 17. a shape lint applies to one tier only; 18. never start a second container gate to read the first (the log is rewritten by every run). Each is one line of the form defect, then rule, and cites the Phase 7 record it comes from; the existing lessons 1 to 12 are untouched.

## The backfill (Task 3)

The set was re-measured with `grep -L '^requirements-completed:'` over the four phase directories: **39 files, 7 in Phase 4 (04-18 to 04-24), 12 in Phase 5, 14 in Phase 6, 6 in Phase 7 (07-01 to 07-06)**, exactly the plan's expectation. A throwaway script under the untracked `build/plan-08-20/` (never staged, `git ls-files build` prints nothing) read the `requirements:` value of the matching PLAN file and inserted, before the closing `---` of the frontmatter, `requirements-completed: <value verbatim>  # backfilled by plan 08-20 from the plan requirements field; closure evidence is the phase VERIFICATION record, not this line`. The two plans whose field is empty (06-01, 06-02) got `[]`. **Proof of one added line each:** `git diff --numstat` over the 39 files read `files=39 added=39 removed=0 bad=0` before the commit, and after it `git show HEAD -- .planning/phases/07-first-visible-original-scenario/07-06-SUMMARY.md` has exactly one `+` line and no `-` line (the sample file is 07-06, deviation 6).

**The three-source check** (throwaway script, same place; the SUMMARY value is read from inside the brackets, never from the trailing comment, so a comment such as "FND-06 also depends on ..." cannot list a requirement). Output after the backfill, quoted whole:

```
FND-01 phase=1 summaries=1 verification=yes 01-01
FND-02 phase=1 summaries=1 verification=yes 01-01
FND-03 phase=1 summaries=1 verification=yes 01-04
FND-04 phase=1 summaries=1 verification=yes 01-06
FND-05 phase=1 summaries=1 verification=yes 01-12
FND-06 phase=1 summaries=1 verification=yes 01-10
FND-07 phase=1 summaries=1 verification=yes 01-08
CPU-01 phase=2 summaries=2 verification=yes 02-01 02-18
CPU-02 phase=2 summaries=1 verification=yes 02-18
CPU-03 phase=2 summaries=1 verification=yes 02-18
CPU-04 phase=2 summaries=1 verification=yes 02-18
CPU-05 phase=2 summaries=1 verification=yes 02-18
MEM-01 phase=3 summaries=2 verification=yes 03-06 03-14
MEM-02 phase=3 summaries=2 verification=yes 03-06 03-14
MEM-03 phase=3 summaries=2 verification=yes 03-11 03-14
MEM-04 phase=3 summaries=2 verification=yes 03-12 03-14
MEM-05 phase=3 summaries=2 verification=yes 03-11 03-14
API-01 phase=3 summaries=2 verification=yes 03-12 03-14
API-02 phase=4 summaries=1 verification=yes 04-25
API-03 phase=5 summaries=7 verification=yes 05-01 05-02 05-03 05-05 05-09 05-11 05-12
API-04 phase=4 summaries=2 verification=yes 04-10 04-25
API-05 phase=2 summaries=2 verification=yes 02-07 02-18
API-06 phase=3 summaries=2 verification=yes 03-13 03-14
RUN-01 phase=4 summaries=4 verification=yes 04-18 04-19 04-20 04-25
RUN-02 phase=4 summaries=4 verification=yes 04-18 04-19 04-20 04-25
RUN-03 phase=2 summaries=2 verification=yes 02-01 02-18
RUN-04 phase=4 summaries=2 verification=yes 04-24 04-25
RUN-05 phase=4 summaries=3 verification=yes 04-18 04-21 04-25
RUN-06 phase=4 summaries=2 verification=yes 04-20 04-25
PKG-01 phase=4 summaries=3 verification=yes 04-22 04-23 04-25
PKG-02 phase=2 summaries=2 verification=yes 02-01 02-18
PKG-03 phase=4 summaries=1 verification=yes 04-25
PKG-04 phase=4 summaries=1 verification=yes 04-25
AUD-01 phase=5 summaries=6 verification=yes 05-01 05-04 05-06 05-07 05-11 05-12
AUD-02 phase=5 summaries=9 verification=yes 05-01 05-02 05-03 05-06 05-08 05-09 05-10 05-11 05-12
INP-01 phase=5 summaries=4 verification=yes 05-01 05-05 05-11 05-12
INP-02 phase=5 summaries=4 verification=yes 05-01 05-05 05-11 05-12
INP-03 phase=5 summaries=4 verification=yes 05-01 05-05 05-11 05-12
VID-01 phase=6 summaries=5 verification=yes 06-04 06-05 06-06 06-13 06-14
VID-02 phase=6 summaries=3 verification=yes 06-08 06-13 06-14
VID-03 phase=6 summaries=6 verification=yes 06-03 06-04 06-09 06-12 06-13 06-14
VID-04 phase=6 summaries=5 verification=yes 06-10 06-11 06-12 06-13 06-14
VID-06 phase=6 summaries=4 verification=yes 06-05 06-07 06-11 06-14
SCN-01 phase=7 summaries=7 verification=yes 07-01 07-02 07-03 07-04 07-05 07-06 07-12
SCN-02 phase=7 summaries=1 verification=yes 07-12
requirements=45 findings=0
```

**Findings: none.** The same check over the SUMMARY files as committed at the plan head (read with `git show`, nothing checked out) reported `requirements=45 with-no-summary=11`: API-03, AUD-01, AUD-02, INP-01, INP-02 and INP-03 of Phase 5 and VID-01, VID-02, VID-03, VID-04 and VID-06 of Phase 6 had no SUMMARY that listed them, so the milestone audit would have read them as partial by omission; they now do. No requirement mark moved and nothing was patched silently.

**Reading rule.** The backfilled value is the plan's own field, so a plan that served a requirement without closing it lists it (plans 07-01 to 07-06 list SCN-01, which plan 07-12 closed). The comment on each inserted line says the closure evidence is the VERIFICATION record; the three-source check requires that record to name the requirement, and it does for all 45.

## The accepted debt record

`08-ACCEPTED-DEBT.md` (repository-relative paths only, ASCII, no claim word, `grep -c draft` reads 10): the six draft validation files of Phases 2 to 7 (status `draft`, approval `pending`, each with one row: what it was meant to hold, why a re-run adds nothing now, where the evidence is: the phase VERIFICATION record and the gate), the four rows of the STATE Deferred Items table and the open review items of Phases 4, 5, 6 and 7 by name with their status, the three-source result, the one sweep finding that was not fixed, and the items deferred out of Phase 8 with a reason each (the snapshot file options and the runner file persistence, 192-line video publication, libFuzzer runs in CI, SHA-256 frame-hash optimization, D10, D11, a history rewrite, name clearance, and the open items of plans 08-14 to 08-19: the applied-event getter, the two pinned acceptances, the `stic.late_grants` witness, the `docs/runner.md` golden commands, the informational baseline). It states that it changes no requirement mark and no claim.

## Deviations from Plan

**1. [Interpretation] The header diff is 6 changed lines, not at most 4.** The acceptance criterion counts `+` and `-` lines of `git show HEAD -- core/include/ivs/ivs_machine.h`. The plan asks for the version corrected to 3 "with the reason" at the first comment (two lines, 4 diff lines) and the second comment names the version on one line (2 diff lines), so 6 is the least that carries the reason; a one-line variant of the first site would drop the reason. Every changed line is inside a comment, no token or number changed, and the diff is quoted above.

**2. [Reality] The README has no sentence about the authority for a release.** The plan says "where the README says that a release needs owner authority, say instead ...". It does not say so (re-grepped for `authority`, `owner gate` and `release needs`); the delegated-flow sentence is added where the stale release sentence stood, as the plan's text for that sentence asks.

**3. [Reality] Earlier plans had half fixed three of the plan's items.** `docs/packaging.md` already listed `ivs_state.h` and the eleven paths (plan 08-11), so the remaining stale text was the missing count, the four-call description, "the other three headers" and the Release content; the CHANGELOG said "ten files ... four public headers"; the STATE persistence sentence was already right. Each was judged on what the text says now.

**4. [Interpretation] The ROADMAP Phase 8 list keeps its marks.** The plan says to write the marks unchecked because the plan-complete mechanism flips them; the list already had 19 `[x]` marks set by earlier plans, and unchecking them would erase true state. The goals were added to the existing lines and the closing commit flips 08-20 (and the counters to 20/21).

**5. [Scope] The sweep found more than the plan named, and fixed it in the same tasks:** the plan-time gate statements of Phases 1, 4, 5 and 7 (a status note each), the REQUIREMENTS profile sentence and intro, three PROJECT recommendation statuses, the PROJECT sentence about D6, D7, D8 and D12, two open-question notes and the D10 item count (42 to 47, in the Task 3 commit because it came out of writing the debt record; it does not touch a SUMMARY file, so the stat count of 39 holds). One more was found and not fixed (deviation 7).

**6. [Reality] The plan's acceptance sample file is not one of the 39.** Acceptance 1 of Task 3 names `07-07-SUMMARY.md`, which already carries `requirements-completed: []` (a Phase 7 plan that closed nothing), so it is not backfilled and `git show HEAD` of it is empty. The sample used is `07-06-SUMMARY.md`, which prints 1 (one `+` line, no `-` line). The re-measured set is exactly the 39 of the plan.

**7. [Information] `core/src/machine/machine_digest.c` still has an internal comment that calls the composite version 2 since plan 05-03.** It is a source file; the plan lists none in `files_modified`, its verify requires that no `core/src` file changed, and the only C-visible edit it allows is the public header comment. It is recorded in `08-ACCEPTED-DEBT.md` section 5 with the recommendation to correct it in the next commit that touches the file. `tests/machine/machine_digest.c` also says "composite version 2" in comments that describe the historical Phase 5 pins; those are true as written.

**8. [Information] The plan-start gate was re-run** (the plan asks for it): sh, 76 s, 0 FAIL lines, the counts equal to the plan's, so there is no difference to record.

**9. [Information] STATE.md and ROADMAP.md are edited by hand by the closing commit,** as plans 08-14 to 08-19 did (the STATE frontmatter counters and the metric and decision rows, the ROADMAP list mark, plan count and progress row). The Task 2 commit edited only the body statements, as the plan says.

No auth gates, no package installs (T-08-SC), no `gh` call, no push, no tag, no `tools/ci-record.sh`; `.planning/config.json`, `.planning/state.json` and `.planning/ci-runs.tsv` were never staged. No file under `core/src/`, `tests/`, `runner/`, `fixtures/` or `data/golden/` changed (`git show --stat` of the Task 1 commit lists five files, none of them there), no oracle, golden or literal file was edited, and no file was deleted. D10 and D11 are not performed and no claim above provisional exists; the sweep states them as open and not performed wherever it touches them.

## Decisions Made

- A sweep is only as good as its re-grep: each stale phrase was found by its text, replaced by the true statement stated once, and the verify greps prove the phrase is gone; three items turned out to be already fixed or absent and are reported as such.
- The release statement is tense-free on purpose: "releases, when they exist, are listed on the releases page" is true before and after the tag, so the tagged tree needs no further edit.
- An index line is not a claim: the backfill carries the plan's field verbatim and says in each line where the closure evidence is, and the audit's three sources (SUMMARY line, VERIFICATION record, traceability table) are the ones that decide.
- A found defect outside the plan's file set is recorded with its place and a recommendation, never patched under the cover of a text task.

## Cost (laptop)

| Item | Value |
|------|-------|
| estimate of the plan | 100,000 tokens (raw, confidence low), 3 tasks |
| actuals | 1,380 tokens (chars/4 over the added lines of the 3 commits outside `.planning/`: 33 lines, 5,520 characters), 3 tasks, 3 commits (measured from the ledger of the plan head); the planning text is not in that figure, as in the earlier plans |
| gate total, plan start (sh) | 76 s (`gate-cpu` user 136.2 sys 116.4) |
| gate total, sh and dash, Task 1 tree | 77 s and 68 s, identical summary line (md5 18d20bdede2067dbb49ab1d959de1c13) |
| gate total, sh and dash, Task 2 tree | 72 s and 73 s, identical summary line (the same md5, `tracked=2485`) |
| gate total, sh and dash, Task 3 tree | 73 s and 73 s (`gate-cpu` user 123.9 sys 106.9 and user 126.4 sys 107.0), identical summary line (md5 3ac02dfcf28be07e91440b26eae98c58, `tracked=2486`: the debt record is the one new file) |
| container gate (gcc 13.3.0, cmake 3.28.3, x86_64, `GATE_LINUX_PLATFORM=`) | Task 1 tree: exit 0, `gate: PASS`, `gate-time total=267` (330 s wall including the image and packages), `core-imports=5`, 0 FAIL lines; Task 3 and closing trees: see the final gate section |
| the added gate steps, controls, ids, faults | none |

**Cost position.** The plan adds no id, control, fault, step or launch; the text, glossary, hygiene and claims steps read a few kilobytes more (the CHANGELOG moved under a new heading, the planning records and the debt record added about 35 KB of text in all, `git diff` over `.planning/` outside the 39 summary lines) and the plan-start gate (76 s) and the three task gates (68 to 77 s) differ by the load noise of the laptop. The orchestrator's projection after plan 08-19 (about 7.95 by the last-five median, under the line 8.3 by 0.35, above the budget 7.8 by 0.15) is unchanged by this plan; the `phase8-close` entry of plan 08-21 should read the next hosted rows with plans 08-18 to 08-20 in them.

## Open Items

1. **`core/src/machine/machine_digest.c` still names composite version 2 in an internal comment** (deviation 7): correct it in the next commit that touches the file; the record is `08-ACCEPTED-DEBT.md` section 5.
2. **Everything in `08-ACCEPTED-DEBT.md` stays open on purpose:** the six draft validations, the carried review and STATE items, and the deferred list of Phase 8. Plan 08-21 refreshes the STATE Deferred Items table and decides the status of `08-VALIDATION.md`, which is Phase 8's own draft and not one of the six.
3. **The CHANGELOG section `[0.1.0]` has no date and no placeholder:** it says the release's own date is on the releases page. If the owner prefers a dated heading it is a one-line change before the tag; the release script reads the section by the version in square brackets and does not care about a date.
4. **The `requirements-completed` lines of the 39 backfilled files are an index with the plan's own field, not a closure statement** (reading rule above); the audit's decision rests on the VERIFICATION records, which name all 45 requirements.
5. **Open from earlier plans, carried unchanged:** the non-idle baseline and the macOS-only timed path (08-19), `docs/runner.md` lacking the two bench golden commands (08-18), the applied-event getter and the two pinned acceptances (08-17); all listed in the debt record.
6. **Information.** D10 and D11 are not performed; no claim above provisional exists; REL-01 stays Pending until its second stage (D-28).

## Interfaces for the next plan

- Counts after this plan (unchanged from plan 08-19): `steps=38/38 controls=380/380 faults=235/235 fault-runs=235 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 goldens=29 claims=28 provisional=27 gates=90 lines=306 pkg-selftest=19/19 state-rows=88 state-cuts=18`; plan 08-21 starts from them.
- Plan 08-21 owns: `08-VERIFICATION.md`, `08-MILESTONE-CLOSE.md`, the `phase8-close` entry, the Decision Register rows of the release flow (the OQ8 and OQ11 rows already exist), the Phase 8 lessons (PLAN-LESSONS holds lessons up to 18, the Phase 8 lessons continue from 19), the refreshed STATE Deferred Items table and the final gate lines of the exact final tree.
- The CHANGELOG `[0.1.0]` section is what the release script publishes as the notes; plan 08-21's stage 2 placeholders do not touch it.
- Threat ids: T-08-45 and T-08-46 are mitigated here; the next plan continues from T-08-47 or the numbering its plan assigns.
- The throwaway scripts of this plan live in the untracked `build/plan-08-20/` (backfill, three-source check, before-backfill check); they can be deleted with the build tree and nothing refers to them.

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=38/38 controls=380/380 ... faults=235/235 ... fault-runs=235 ... tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 ... claims=28 provisional=27 ... gates=90 lines=306 ... goldens=29 ... pkg-selftest=19/19 ... (0 FAIL lines)
dash tools/gate.sh:    the identical summary line (0 FAIL lines)
GATE_LINUX_PLATFORM= sh tools/gate-linux.sh: Task 1 tree exit 0, gate: PASS, the plan counts, 0 FAIL lines; Task 3 tree and closing tree: see below
```

After the Task 1 tree (`9c192d5`, public text): sh 77 s and dash 68 s, identical summary line, and the container gate on the committed tree: exit 0, `gate: PASS` (gcc 13.3.0, cmake 3.28.3, x86_64), `steps=38/38 controls=380/380 faults=235/235 fault-runs=235 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 goldens=29 claims=28 provisional=27 gates=90 lines=306 pkg-selftest=19/19`, `core-imports=5`, `tracked=2485`, 0 FAIL lines, `gate-time total=267` (330 s wall). After the Task 2 tree (`17d67fc`, planning records): sh 72 s and dash 73 s, identical summary line, 0 FAIL lines (the plan asks for sh and dash only). After the Task 3 tree (`d4788f9`: 39 summary lines, the debt record, PROJECT.md): sh 73 s and dash 73 s, identical summary line (md5 3ac02dfcf28be07e91440b26eae98c58, `tracked=2486`), 0 FAIL lines, and the container gate on the committed tree: exit 0, `gate: PASS`, the same counts as above with `tracked=2486` and `core-imports=5`, 0 FAIL lines, `gate-time total=301` (369 s wall, under the limit of 15 minutes).

On the closing tree (this SUMMARY, STATE.md and ROADMAP.md added or edited; `tracked=2487`): sh (`gate-time total=75`, `gate-cpu` user 137.1 sys 117.5) and dash (`gate-time total=70`, user 128.0 sys 108.3) `gate: PASS` with the identical summary line (md5 8e21ab8c4537d81292f609fc7fa0024e), 0 FAIL lines and the plan-end counts `steps=38/38 controls=380/380 faults=235/235 fault-runs=235 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 goldens=29 claims=28 provisional=27 gates=90 lines=306 pkg-selftest=19/19`, and `GATE_LINUX_PLATFORM= sh tools/gate-linux.sh` on the closing tree (Ubuntu 24.04, gcc 13.3.0, cmake 3.28.3, x86_64; 332 s wall, under the limit of 15 minutes): exit 0, `gate: PASS`, the same counts, `core-imports=5`, `tracked=2487`, 0 FAIL lines, `gate-time total=273`. The SUMMARY text was completed with these lines after those runs; no gate step reads them differently.

## Known Stubs

None.

## Threat Flags

None new. T-08-45 (stale public and planning text at the tagged commit): every phrase of the plan's list was re-grepped and replaced, the verify greps prove each is gone, the text, glossary, hygiene, api-effects and claims steps and every count stay green, no oracle, golden or literal was edited, and the one stale statement outside the file set is recorded rather than patched. T-08-46 (misstating which plan closed a requirement): the backfilled value is the plan's own field verbatim with a comment that the closure evidence is the VERIFICATION record, every added line is proved by `git diff --numstat`, and the three-source check reports a requirement that no SUMMARY lists (11 before the backfill, 0 after) instead of patching it silently. T-08-SC: no installs.

## Self-Check: PASSED

Verified before the final commit: the SUMMARY and the created file of the plan exist (`08-ACCEPTED-DEBT.md` is tracked); commits `9c192d5`, `17d67fc` and `d4788f9` are ancestors of HEAD and `git rev-list --count 2d37aa65bf11bc6bb0058a32579d517bd9905b67..HEAD` printed 3 before this file's commit; nothing is staged but the files of this commit (this SUMMARY, STATE.md and ROADMAP.md), and `.planning/config.json`, `.planning/state.json` and `.planning/ci-runs.tsv` appear in none of the three commits and are not staged; `git ls-files` lists no `.ivsc`, `.ivsn`, `.bin`, `.rom`, `.exec`, `.wav`, `.ppm` or `.vraw` file and `git ls-files build` lists nothing; the diff of the three commits outside the 39 summary lines holds no home, scratch or temp path and no at sign before a dotted word, and the one occurrence of the console family name in added text is descriptive (an open-question line of STATE.md) and in no name; no commit deletes a file; none touches `core/src/`, `tests/`, `runner/`, `fixtures/` or `data/golden/`; 39 SUMMARY files of Phases 4 to 7 changed, each by one added line, and every SUMMARY of Phases 4 to 7 now carries the key (`grep -c` over the four directories prints no file with 0); `grep -c draft` of the debt record prints 10; the plan's nine verify blocks pass `sh -n` and the five that do not start a gate (the phrase greps, the file counts and the key check) exit 0 under sh.
