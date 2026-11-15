---
phase: 08-stic-completion-save-state-performance-baselines
plan: 21
subsystem: Phase 8 wave D, plan 2 of 2 - the milestone close-out, stage 1 of REL-01 - the close record A to K with its generated sections and its stage 2 placeholders, the verification record, the Decision Register rows, the lessons, the refreshed deferred items and the cost close
tags: [D, REL-01, D-27, D-28, D-33, D-35, D-36, D-37, close-out, stage-1, release-readiness, history-scan, phase8-close, T-08-47, T-08-48]
status: complete

requires:
  - phase: 08-stic-completion-save-state-performance-baselines
    provides: "plan 08-20 end state: steps=38/38 controls=380/380 faults=235/235 fault-runs=235 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 goldens=29 claims=28 provisional=27 gates=90 lines=306 pkg-selftest=19/19 (re-measured with sh tools/gate.sh on the Task 1 tree: identical, 0 FAIL lines)"
provides:
  - "08-MILESTONE-CLOSE.md: the eleven sections A to K of the stage 1 close record; A, F, G and J are the pasted output of tools/release-readiness.sh (the full history scan included: six findings, all pinned, none new, never a pass); B to E, H, I and K are written by hand; every post-merge fact is the literal placeholder; the release is stated as not started, paused by the owner's decision after Phase 8 (relayed by the orchestrator)"
  - "08-VERIFICATION.md in the Phase 7 layout: status human_needed, the three final-gate lines, SC1 to SC5, the 20 V-rows with commits read from git log, the requirement table, the oracle order checked with merge-base, the hand-offs and the caveats"
  - ".planning/PROJECT.md Decision Register rows for D-29, D-30, D-31 with D-42 and D-33 (owner-veto note); .planning/ops/PLAN-LESSONS.md lessons 19 to 24; .planning/STATE.md deferred items refreshed with a pointer to the close record; .planning/ops/RELEASE-OPS.md item h (the placeholder check and the order of the orchestrator's stage 2 actions)"
  - ".planning/CI-EFFICIENCY.md entry Phase 8 close-out (plan 08-21) ending in the single line phase8-close: no-lever; .planning/REQUIREMENTS.md stage 1 note, REL-01 still Pending"
affects: [stage 2 of REL-01 (orchestrator), the milestone audit, the first release]

actuals:
  tokens: 31290
  tasks: 3
  commits: 3
plan_head_before: 407d668dc4d6a930798e6fff85106fa8e4f9dc8c
plan_head_after: 8501f00a6e3c8f4ca16eb5968df1c506c3fc131a

key-decisions:
  - "The record never says the history check passed: the scan exits 1 on its six findings, each is printed with its recorded disposition and the record says they are accepted and not cleared; the one generated line that carried the words of a pass verdict (the tree-scan line of section J) is printed without them because the plan's verify forbids those words in the record, and the edit is disclosed in the record"
  - "The status of the verification record is human_needed and not passed: SC1 to SC4 hold on recorded terms, SC5 is at stage 1, and what remains is the owner's hand-offs and the orchestrator's stage 2; REL-01 stays Pending in the checkbox and the traceability row"
  - "The release is stated as not started, paused by the owner's decision after Phase 8, labelled as relayed by the orchestrator and not checkable by the executor; the decision line stays the literal placeholder because no release decision was taken"
  - "The explanatory text of the close record never writes the placeholder literal, so the grep of RELEASE-OPS item h finds only real placeholders; the order of the orchestrator's stage 2 actions is written next to the grep"
  - "No executor writes a stage 2 fact: sections B, C, I and K hold the literal for the push run, the pull request and issue counts, the ruleset read, the artifact commit, the tag half of P4, the veto confirmation of P6, the ruleset of P7, P8 and the decision"

key-files:
  created:
    - .planning/phases/08-stic-completion-save-state-performance-baselines/08-MILESTONE-CLOSE.md
    - .planning/phases/08-stic-completion-save-state-performance-baselines/08-VERIFICATION.md
    - .planning/phases/08-stic-completion-save-state-performance-baselines/08-21-SUMMARY.md
  modified:
    - .planning/PROJECT.md
    - .planning/ops/PLAN-LESSONS.md
    - .planning/ops/RELEASE-OPS.md
    - .planning/STATE.md
    - .planning/CI-EFFICIENCY.md
    - .planning/REQUIREMENTS.md
    - .planning/ROADMAP.md

requirements-completed: []  # REL-01 completes after its second stage (the records pull request after the merge, D-28); this plan marks no requirement

completed: 2026-11-15
---

# Phase 8 Plan 21: the milestone close-out, stage 1 Summary

**Stage 1 of REL-01 is recorded and REL-01 stays Pending. `08-MILESTONE-CLOSE.md` holds the eleven sections A to K: A, F, G and J are the pasted output of `tools/release-readiness.sh` on the head of plan 08-20 (the full history scan took 253 s, the script exited 0, and the scan reported six findings, all six on the pinned list and printed with their recorded disposition, none new, and never as a pass); B, C, D, E, H, I and K are written by hand (50 requirement rows with a caveat column, the qualification list of 14 items, the authority table, the checks P1 to P8 with their computable status, nine owner hand-offs, the remaining steps); every fact that exists only after the merge is the literal placeholder. `08-VERIFICATION.md` follows the Phase 7 layout with status `human_needed`. Four Decision Register rows, lessons 19 to 24 and the refreshed deferred items are written, the cost close reads `phase8-close: no-lever` (median `r` 7.80 against the budget 7.8 and the line 8.3), and the release is recorded as not started: paused by the owner's decision after Phase 8, a decision relayed by the orchestrator. sh and dash read `gate: PASS` with the plan counts on every task tree and on the closing tree, and the container gate passed on the Task 2 tree and on the closing tree. This is an executor's record and not an independent verifier run; no claim is above provisional, D10 and D11 are not performed, and no tag, release, push or `gh` call was made.**

## Commits

| Commit | Task | What |
|--------|------|------|
| `34b12fd` | 1 (tracer) | `08-MILESTONE-CLOSE.md` with the eleven headings; A, F, G and J pasted from the readiness generator (full history scan included), the stage 2 placeholders |
| `6fcc396` | 2 | sections B to E, H, I and K, `08-VERIFICATION.md`, four Decision Register rows, lessons 19 to 24, the STATE deferred items and pointer, RELEASE-OPS item h |
| `8501f00` | 3 | the `phase8-close` cost entry, the three final-gate lines in the two records, the REQUIREMENTS stage 1 note |
| the commit that adds this file | | this SUMMARY, STATE.md and ROADMAP.md (plan counters, metric row, decisions, the deferred-table header fix) |

## The history scan (Task 1)

`sh tools/release-readiness.sh --selftest` read `selftest 4/4`. The full scan `sh tools/release-readiness.sh --allow-dirty --history` ran in the background on the local laptop (1-minute load 8 to 17), started 2026-11-15T15:57:58Z on the head `407d668` (every plan through 08-20, none of 08-21), took **253 s of wall time** and ended with the script status 0 (the scan itself exits 1 on its findings). Its output was kept in the untracked build tree and pasted into the record. Six findings, **all six on the pinned list and printed with their recorded disposition, none new**:

| Token | Short sha |
|-------|-----------|
| `hygiene-message-home-path` | `955c13fa02` |
| `hygiene-history-home-path` | `eaa832e3d3` |
| `hygiene-history-home-path` | `92c4dc7d37` |
| `hygiene-history-home-path` | `b78af4fb9f` |
| `hygiene-history-home-path` | `cccdfefdce` |
| `hygiene-history-home-path` | `345c48dd7f` |

The structural verifies held on the committed record: exactly 6 lines with the disposition text, 0 lines with the new-finding text, no occurrence of the words of a pass verdict (see deviation 1), the readiness selftest `4/4`. **The baseline file and every rule were not touched:** `git diff` of `tools/check/hygiene-baseline.txt` against the commit that last changed it exits 0 (checked before Task 1), and `tools/hygiene.sh` is unchanged (this plan changes no file outside `.planning/`). The scan ran on the head before this plan's commits, which add only `.planning/` text and commit messages; the tree scan covers that text in every task gate, and a second scan on the final head is recorded in the final gate section.

## What the close record holds

- **Generated (A, F, G, J):** the commit and tree identities of the seven directories, the 28-row claim table with its totals (28 claims, 27 provisional, 1 established, none at the measured-hardware tier), the eleven-file install list, the binary-name scan (0), the fixture census (11 manifests: yes 8, no 2, other 1), the name check (0) and the hygiene result; the sections A, F and G of the local run and of the history run are identical (compared with `diff`).
- **Hand-written, with claim tags or `[nc]` on every claim sentence (checked by reading):** B (the three final-gate lines, the five hosted rows, the push run as placeholders), C (placeholders, and what the planning discussion found on 2026-11-14), D (ten next-milestone candidates with their gating evidence, the deferred items, ten `LESSONS-m1` lines), E (the 50 rows generated by a throwaway awk script from the traceability table: **49 Complete and 1 Pending**; the three-source result: Phases 1 to 7 45 requirements and 0 findings, Phase 8 all four closed requirements listed and named), H (14 items not qualified), I (the authority table), K (P1 to P8, nine hand-offs H8-1 to H8-9, nine remaining steps).
- **Placeholders (the contract of the orchestrator's stage 2):** the artifact commit (A), the push run id, head, event, conclusion and row (B), the open pull request and issue counts, the ruleset read and the release list (C), the state of the merge, ruleset and tag actions (I), the tag half of P4, the veto confirmation of P6, the ruleset read of P7, P8 and the decision line (K). The committed record holds 18 lines with the placeholder by design, and the explanatory text never writes the literal.

## The register rows, the lessons, the requirement rows

- **Decision Register rows added (PROJECT.md, delegated decisions, owner authorization 2026-11-09):** `REL-01 / D-29` (the first release is a draft pre-release, tagged on the artifact commit, published by the orchestrator after P1 to P8), `REL-01 / D-30` (three separate version numbers; the tag equals the header macros), `REL-01 / D-31, D-42` (the milestone tool must not create a tag; the key `git.create_tag` is an orchestrator-only change, still `true` at this record) and `REL-01 / D-33` (the history disposition with the explicit owner-veto note and the reversal evidence). The persistence row OQ8 and the performance row OQ11 already existed from plan 08-20.
- **Lessons added (PLAN-LESSONS 19 to 24, one line each, defect then rule):** 19 plan prose lost to the frozen specification; 20 a check that cannot fail and vacuous equivalence checks; 21 a frozen oracle takes a correction as an appended note; 22 unit costs priced by id and by fault were wrong for heavy suites; 23 a recorded condition the host did not meet is kept as taken and said; 24 an interrupted run leaves state that cannot be trusted.
- **Requirement rows by status (section E):** 49 Complete (FND 7, CPU 5, MEM 5, API 6, RUN 6, PKG 4, AUD 2, INP 3, VID 6, SCN 2, STA 2, PRF 1) and 1 Pending (REL-01). VID-05, STA-01, STA-02 and PRF-01 were not re-marked: their marks and close-out notes agree with the evidence; REL-01 stays `- [ ]` and `Pending`, with a stage 1 note in REQUIREMENTS.md.
- **Deferred items (STATE.md):** the four earlier rows kept (none resolved by Phase 8) with the milestone each waits for, and ten rows added from `08-ACCEPTED-DEBT.md` (review items, validation drafts, the snapshot file options, 192-line video, libFuzzer in CI, performance, state, hygiene, owner items, the paused release flow); a pointer line to the close record that does not move with the phase directory.

## The cost close and its answer

`.planning/CI-EFFICIENCY.md` gained `Phase 8 close-out (plan 08-21)`: the counts at the Phase 7 end and the Phase 8 end with what grew and why, the local measurement table (sh and dash on two task trees, the container once, each labelled, the laptop load stated), the five hosted rows recomputed with awk, the ancestry check of the rows against the plan commits, the stage 2 note and the checkpoint summaries. **Last five hosted `r` values: 7.28, 7.86, 8.29, 7.72, 7.80; sorted median 7.80 against the budget 7.8 and the line 8.3 (at the budget, 0.50 below the line); newest 7.80 (build 45, cpu 351).** Both raw triggers fire (median `gate_s` 113 against 110, median `cpu_s` 351 against 330) while `r` is at the budget, so by the rule of the Phase 6 start entry they read as runner speed (median `build_s` 46 against 41 at the Phase 7 close), with the stated caveat that one window cannot separate runner speed from code growth. The newest row, 43732465524, contains every plan through 08-20, the whole phase except this plan's docs commits (`git merge-base --is-ancestor` of the first commit of plans 08-09, 08-16, 08-18, 08-19 and 08-20 against its head exits 0). The answer line is the single line **`phase8-close: no-lever`** (`grep -c '^phase8-close: '` prints 1). Nothing in this plan calls `gh` or `tools/ci-record.sh`.

## Orchestrator hand-off (stage 2, none started: paused by the owner's decision after Phase 8)

In the order of `.planning/ops/RELEASE-OPS.md` item h:

1. Merge the Phase 8 pull request with green CI (the merge commit is the artifact commit).
2. Record the push run on the merge commit: `tools/ci-record.sh` (an `event=push` row).
3. Open the `gsd/records-m1-close` branch; make the `git.create_tag` change (D-42) as its first commit, alone, before the milestone tool is used (the key reads `true` today).
4. Read GitHub once with `sh tools/release-readiness.sh --hosted` (open pull requests and issues with dispositions, the ruleset read, the release list; the immutable-release setting H8-6).
5. Tag `v0.1.0` on the artifact commit; read the draft with `gh release view --json`; check P1 to P8; publish, or record `READY, NOT PUBLISHED: blocked by P<n>`.
6. Fill every placeholder of `08-MILESTONE-CLOSE.md`, run the grep of item h until it prints nothing, re-run the gate on the records branch.
7. Mark REL-01 complete in REQUIREMENTS.md only after that; record the release in STATE.md; write the release workflow's CI-efficiency entry.
8. Merge the records pull request; then the milestone audit, the milestone completion on a records branch and the cleanup.

## Deviations from Plan

**1. [Rule 3 - the plan's verify against its own instruction] One generated line of section J is edited.** The plan says to paste the history output verbatim, and its second Task 1 verify requires that the record never contain the words `hygiene pass` (case-insensitive). The script's tree-scan line reads `exit 0: -- hygiene pass mode=tree hygiene-files=2487`, so a verbatim paste fails the verify. The line is printed as `exit 0: mode=tree hygiene-files=2487` (the two words removed, nothing else), and section J says so. The history output itself is verbatim.
**2. [Interpretation] The heading date of the cost entry is 2026-11-15.** The plan text gives 2026-11-14, the date of the earlier Phase 8 entries; this plan ran on 2026-11-15 and the entry says so in its first sentence.
**3. [Interpretation] The status of the verification record is `human_needed`.** `passed` would be stronger than the record's own wording (SC5 is at stage 1, the owner's hand-offs and the orchestrator's stage 2 remain), so the frontmatter says `human_needed` with the automated line beneath it.
**4. [Interpretation] The release is stated as not started, paused.** The orchestrator's instruction (the owner's decision to pause after Phase 8) replaces the plan's expectation of a publication decision in stage 2; the statement is labelled as relayed and not checkable, and the decision line stays the literal placeholder (no decision was taken).
**5. [Interpretation] The explanatory text of the record never writes the literal placeholder.** The first commit of Task 1 had two explanatory mentions; Task 2 rewrote them so that the grep of RELEASE-OPS item h finds only real placeholders.
**6. [Information] The install list is eleven files, not ten.** The plan text says ten in several places; `ivs_state.h` joined in plan 08-11 and the pinned list reads eleven (section G).
**7. [Addition] Two read-only local runs for P4 and P7:** `sh tools/release-readiness.sh --allow-dirty --release` printed `P4: ok` (head `407d668`) and `sh tools/release-rehearse.sh --no-gate --release` ended `rehearse: ok` (head `34b12fd`: source archives byte-equal, install archives equal except the Darwin ranlib stamp, tag glob tried). Neither calls `gh`; both are cited in section K.
**8. [Information] No plan-start timestamp was taken** (the plan's start-time step was not run at the start); the duration in STATE.md is estimated from the first command to the closing commit.
**9. [Information] `08-VALIDATION.md` stays `draft`.** Plan 08-20 left its status to this plan; it is recorded as accepted tech debt (STATE deferred items, section 4 of the verification record) and was not edited, because it is not in this plan's `files_modified` and a re-run of validation would restate the results of the V-rows.
**10. [Information] STATE.md and ROADMAP.md are edited by hand by the closing commit,** as plans 08-14 to 08-20 did (the frontmatter counters, the metric and decision rows, the plan list mark, the plan count and the progress row). The Phase 8 progress row reads 21/21 and `In Progress`, and the Phase 8 line of the roadmap list stays unchecked, because REL-01 is not complete and no verifier has run.
**11. [Rule 1 - a defect of my own, fixed in the closing commit] The STATE deferred-items table lost its header in the Task 2 commit.** The edit replaced the rows and left the old header above the new paragraph, so the committed table had an orphan header, a paragraph and a second header; the closing commit removes the orphan header (visible in `git diff` of STATE.md against `8501f00`).
**12. [Information] A scratch draft of this SUMMARY** (a template in the untracked build tree) **was refused by the tool layer** ("subagents should return findings as text, not write report files"), as in plan 08-17. It was not retried by another route; this file at its repository path is the SUMMARY, written directly.

No auth gates, no package installs (T-08-SC), no `gh` call, no push, no tag, no release, no `tools/ci-record.sh`, no `--hosted`; `.planning/config.json`, `.planning/state.json` and `.planning/ci-runs.tsv` were never staged. No file outside `.planning/` changed, no oracle, golden or literal was edited, and no file was deleted. D10 and D11 are not performed and no claim above provisional exists.

## Decisions Made

- A record that is mostly generated says which lines are generated and which are written, and the one generated line it had to edit is disclosed in the record itself.
- A requirement mark is never stronger than the record behind it: REL-01 stays Pending, the verification status says `human_needed`, and the roll-up sentence is the stage 1 form until stage 2 closes it.
- A fact that exists only after the merge is a literal placeholder that a grep can count, and the explanatory text avoids the literal so that the count is of real placeholders only.
- The cost answer is read from five real rows and the ancestry of each row against the plan commits, and the weakness of a single window (runner speed against code growth) is stated and not hidden.

## Cost (laptop)

| Item | Value |
|------|-------|
| estimate of the plan | 120,000 tokens (raw, confidence low), 3 tasks |
| actuals | 31,290 tokens (chars/4 over the added lines of the 3 task commits; every changed file is under `.planning/`, so the figure counts planning text, unlike earlier plans whose figure excluded it), 3 tasks, 3 commits (measured from the ledger of the plan head) |
| history scan | 253 s wall (background, load 8 to 17) |
| gate total, sh and dash, Task 1 tree | 89 s and 103 s, identical summary line (md5 ce6185c768e0f21b1b4278f4c2bdbc62) |
| gate total, sh and dash, Task 2 tree | 86 s and 89 s (`gate-cpu` 289.2 and 291.6 cpu-s), identical summary line (md5 b299e7869a12cdae9d9563e13ae17315) |
| gate total, sh and dash, Task 3 tree | 75 s and 79 s (`gate-cpu` user 136.82 sys 113.84 and user 138.31 sys 118.74), identical summary line (the same md5) |
| container gate, Task 2 tree (gcc 13.3.0, cmake 3.28.3, x86_64, `GATE_LINUX_PLATFORM=`) | exit 0, `gate: PASS`, 0 FAIL lines, `gate-time total=349` (425 s wall including the image and the packages, under the limit of 15 minutes) |
| gate total, sh and dash, closing tree | 75 s and 81 s (`gate-cpu` user 128.91 sys 111.26 and user 135.36 sys 114.54), identical summary line (md5 9a549e9b24cddcc8bff96e03cd1e2d63) |
| container gate, closing tree | exit 0, `gate: PASS`, 0 FAIL lines, `gate-time total=386` (489 s wall, under the limit of 15 minutes; the second history scan and the rehearsal ran beside it) |
| second history scan, head `8501f00` | 358 s wall, 6 findings, 6 accepted, 0 new (run beside the container gate) |
| the added gate steps, controls, ids, faults | none |

## Open Items

1. **Stage 2 and the owner's decision to resume** (hand-off list above): nothing after the Phase 8 merge was started; the `git.create_tag` key still reads `true`.
2. **The history scan ran on the head before this plan's commits;** the second scan on the final head is in the final gate section.
3. **`08-VALIDATION.md` stays `draft`** (deviation 9), as accepted tech debt.
4. **Carried unchanged from earlier plans:** the internal comment in `core/src/machine/machine_digest.c` that names composite version 2, the informational benchmark baseline, the two benchmark golden commands missing from `docs/runner.md`, the applied-event getter and the two pinned acceptances of the state wave (all in `08-ACCEPTED-DEBT.md` and the STATE deferred items).
5. **No independent verifier has run on Phase 8:** the verification record is an executor's; its `independently_verified` line says not yet.
6. **Information.** D10 and D11 are not performed; no claim above provisional exists; REL-01 stays Pending (D-28).

## Interfaces for the next steps

- Counts after this plan (unchanged from plan 08-20): `steps=38/38 controls=380/380 faults=235/235 fault-runs=235 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 goldens=29 claims=28 provisional=27 gates=90 lines=306 pkg-selftest=19/19 state-rows=88 state-cuts=18`.
- The placeholders of the close record and the grep that proves none remains: `.planning/ops/RELEASE-OPS.md` item h.
- The CHANGELOG `[0.1.0]` section is what the release script publishes as the notes; this plan does not touch it.
- Threat ids: T-08-47 and T-08-48 are mitigated here; a later plan continues from T-08-49 or the numbering it assigns.
- The throwaway scripts of this plan (the section E generator, the commit-landing generator, the assemblers, the verify scripts, the ancestry check, the history scan output) live in the untracked `build/plan-08-21/`; nothing refers to them.

## Final gate

On the closing tree (this SUMMARY, STATE.md and ROADMAP.md present in the working tree and not yet committed; `tracked=2490`), the real lines, quoted verbatim. The summary line of `sh tools/gate.sh` (the line of `dash tools/gate.sh` is identical, md5 9a549e9b24cddcc8bff96e03cd1e2d63 for both):

```
gate: PASS steps=38/38 controls=380/380 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=235 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=64 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=28 provisional=27 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=665 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=235/235 tracked=2490 binaries-listed=0 terms=9 doc-files=59 profile-keys=154 goldens=29 reports=32 host-dependent=3 mode=tree hygiene-files=2490 history-commits=1 gates=90 lines=306 input-rows=31 input-policies=11 input-profile-fields=31 files=2490 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=19/19 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=11/11 runner-files=16 cases=8 sources=12 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 docs=60 claim-refs=26 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

- **sh (local laptop)** `gate-cpu user=128.91 sys=111.26`, `gate-time total=75`, 0 `-- FAIL` lines, four `-- SKIPPED` lines.
- **dash (local laptop)** `gate-cpu user=135.36 sys=114.54`, `gate-time total=81`, the identical summary line, 0 `-- FAIL` lines. Against the Task 3 tree the only differences in the summary line are `files`, `hygiene-files` and `tracked`, each 2489 to 2490 (this SUMMARY is the one new file).
- **Container** `GATE_LINUX_PLATFORM= sh tools/gate-linux.sh` (Ubuntu 24.04, gcc 13.3.0, cmake 3.28.3, x86_64, 8 cpus): exit 0, `gate: PASS`, `gate-cpu user=1482.87 sys=109.53`, `gate-time total=386` (489 s wall in all, under the limit of 15 minutes, while the second history scan and the release rehearsal ran at the same time on the same host), 0 `-- FAIL` lines, four `-- SKIPPED` lines; its summary line has the same counts and differs from the line above in four host-dependent tokens only: `archive=sha256` (sh: `members`), `cc=GNU` (sh: `AppleClang`), `core-imports=5` (sh: 7) and `refs-verified=0/4` (sh: 3/4); `tracked=2490`.
- **After the last text edit of this SUMMARY** sh (`gate-time total=70`) and dash (`gate-time total=85`, `gate-cpu user=137.21 sys=118.14`) were run once more on the working tree that is committed with it: the same summary line (md5 9a549e9b24cddcc8bff96e03cd1e2d63), 0 `-- FAIL` lines. The container run above was taken before that last text edit, which changed text inside this one file and no count of the summary line.
- **Second history scan** `sh tools/release-readiness.sh --allow-dirty --history` on the head `8501f00` (all three task commits, run in the background beside the container gate): script status 0, `history findings: 6 (accepted 6, new 0)`, the same six pairs, 358 s of wall time; the first scan (253 s) was on `407d668`. The disposition line is the one in the record; this is not a pass of the history check.
- **P4 and the rehearsal on the head `8501f00`:** `sh tools/release-readiness.sh --allow-dirty --release` printed `P4: ok` and `sh tools/release-rehearse.sh --no-gate --release` ended `rehearse: ok` (source archives byte-equal, install archives `equal-except-ranlib-stamp`). **The seven tree identities** of a fresh `sh tools/release-readiness.sh --allow-dirty` on `8501f00` equal the seven identities pasted into section A (`diff` of the lines of the seven directories is empty), so the final tree is the tree the generated sections describe.

Earlier trees (the same summary line each time, 0 `-- FAIL` lines): Task 1 tree sh 89 s and dash 103 s (md5 ce6185c768e0f21b1b4278f4c2bdbc62); Task 2 tree sh 86 s and dash 89 s, and the container on the committed Task 2 tree (`6fcc396`): exit 0, `gate: PASS`, `gate-time total=349` (425 s wall); Task 3 tree sh 75 s and dash 79 s (md5 b299e7869a12cdae9d9563e13ae17315, the same as the Task 2 tree). The plan's gate-based verify commands were evaluated against these captured outputs (every token of the plan's lists present in all eight sh and dash lines); the six verify blocks that do not start a gate were run as written and exit 0; the container verify (a second container gate) was not repeated, because every container run rewrites `build/gate-linux.log` (lesson 18), and is met by the exit status and the `gate: PASS` line of the captured closing-tree run; all ten verify blocks of the plan pass `sh -n`.

## Known Stubs

None.

## Threat Flags

None new. T-08-47 (an overclaiming or hygiene-hiding record): A, F, G and J are pasted script output (one edited line, disclosed), the history scan is printed whole with the disposition per pinned pair and never as a pass, the qualification section lists 14 things not qualified, every claim sentence carries a claim tag or `[nc]`, the verification status is `human_needed` and not `passed`, and the labels local, container and hosted are never mixed. T-08-48 (a stage 2 fact typed by an executor, an unfilled placeholder, a tag that disagrees with the header): no stage 2 fact was written, every such fact is the literal placeholder, RELEASE-OPS item h holds the check that none remains before the records pull request merges, and no tag, draft, release or `gh` call exists. T-08-SC: no installs.

## Self-Check: PASSED

Verified before the closing commit: the created files exist (`08-MILESTONE-CLOSE.md`, `08-VERIFICATION.md` and this SUMMARY: FOUND); the commits `34b12fd`, `6fcc396` and `8501f00` are ancestors of HEAD and `git rev-list --count 407d668dc4d6a930798e6fff85106fa8e4f9dc8c..HEAD` printed 3 before this file's commit; nothing is staged but the files of the closing commit (this SUMMARY, STATE.md and ROADMAP.md), and `.planning/config.json`, `.planning/state.json` and `.planning/ci-runs.tsv` appear in none of the three task commits (`git show --stat` of each: 0 matches for `config.json` or `state.json`) and are not staged; `git ls-files build` lists nothing (0) and `git ls-files` lists no `.ivsc`, `.ivsn`, `.bin`, `.rom`, `.exec`, `.wav`, `.ppm` or `.vraw` file; the added lines of the three commits hold no home, scratch or temp path, no at sign before a dotted word and no tilde path, the three occurrences of the console family name are descriptive (a variant name in a table, the script's name-check line, a list of what is not qualified) and in no file name; the commit messages hold no path shape and each ends with the trailer `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>` exactly (3 of 3); no commit deletes a file and none touches a path outside `.planning/`; `git diff` of `tools/check/hygiene-baseline.txt` against its last commit is empty and `tools/hygiene.sh` is unchanged; the committed close record has the eleven headings A to K, 18 lines with the stage 2 placeholder, exactly 6 lines with the disposition text, 0 with the new-finding text and no occurrence of the pass-verdict words; `grep -c '^phase8-close: '` prints 1; REL-01 is `- [ ]` and `Pending`; the plan's six non-gate verify blocks exit 0 under sh.
