# Phase 8 milestone close record, stage 1

**What this record is.** The close record of the first milestone (Phases 1 to 8) for the exact tested tree of the Phase 8 branch, in the eleven sections A to K of `.planning/phases/08-stic-completion-save-state-performance-baselines/08-DISCUSS-release.md` section 2 (D-27). It is **stage 1** of REL-01 (D-28): everything that can be computed before the merge. It was written by the executor of plan 08-21; it is **not an independent verifier run**, and the executor is the same model family that wrote the derivations, the programs and the core. [nc]

**What is generated and what is written.** Sections A, F, G and J are the pasted output of `sh tools/release-readiness.sh --allow-dirty --history` (the claim table inside it comes from `tools/check/claims-table.cmake` over `data/claims.json`), run on the head named in section A. `--allow-dirty` is used because three tracked files of the working tree differ from the head by design (`.planning/ci-runs.tsv`, `.planning/config.json`, `.planning/state.json`: the orchestrator's files, which no executor stages) and the record files of this plan were not yet committed. Every other section is written by hand from the SUMMARY files, the verification record and the repository, and says so. One generated line was edited: the tree-scan line of section J (see there). Nothing generated was typed. [nc]

**What the placeholder means.** The literal placeholder (the word stage2 inside angle brackets) marks every fact that exists only after the merge of the Phase 8 pull request: the Phase 8 merge commit, the push run on it, the open pull request and issue counts, the ruleset read, and the release decision. Those are filled by the orchestrator's `gsd/records-m1-close` pull request after the merge (D-28, `.planning/ops/RELEASE-OPS.md`, items b and h). An executor writes none of them. REL-01 stays Pending in `.planning/REQUIREMENTS.md` until then. This explanatory text never writes the literal itself, so that the check of item h of `.planning/ops/RELEASE-OPS.md`, a grep for it, finds only real placeholders. [nc]

**Release status at the time of writing: not started, paused by the owner's decision after Phase 8.** This statement is relayed to the executor by the orchestrator, which reports the owner's decision to run `/gsd-pause-work` after Phase 8 instead of continuing into the milestone audit, the milestone completion, the cleanup and the first release; the executor cannot check it independently. Consequently no tag, no draft and no release exists, no push or `gh` call was made by this plan, and nothing in this record says that anything was published. [nc]

**Labels.** Every figure is labelled local laptop, container (Ubuntu 24.04, emulated amd64) or hosted, and the three are never mixed. Every sentence that is not a quote of a gate line and that states a result carries a claim tag (`[claim:CL-NNNN]`) or `[nc]` (not a claim); the record is not scanned by the gate, so this was checked by reading. No sentence here is stronger than provisional: 27 of 28 claims are provisional, the one established claim is CL-0002, and no claim is at the measured-hardware tier. [nc]

## A. Identity

- commit: 407d668dc4d6a930798e6fff85106fa8e4f9dc8c
- commit date (UTC): 2026-11-15T15:53:58Z
- working tree: dirty (tracked files differ from HEAD; --allow-dirty); the ids below are those of HEAD
- tree core: 78927759e0f0c43f9b8ce2409fc90c171efc0ccf
- tree runner: 04f38a0a3ec71e938276cb5cb141a611f5450110
- tree tools: 5771148bfc623e1a433765eee7477ad16daf1326
- tree data: ec05601c37e186fc3cadfec1efd636acc6aae0c9
- tree docs: e1b81bd6efb01c9d9d9e195a0e3fc21dc7dc21e9
- tree fixtures: 55170e1141ffbdbe514a04bf47491d4c5b5ff0c8
- tree tests: 51ef9b88c6e79986f72d404371fa0b6a10d14efa


**Reading of section A.** [nc]

- Milestone label: the first milestone, Phases 1 to 8. It is a GSD planning label, not a software version and never a git tag (D-30, D-31).
- Software version: 0.1.0, from the three version macros of `core/include/ivs/ivs.h`; the installed package reports the same version (D-30).
- Artifact commit: `<stage2>` (the Phase 8 merge commit on `main`; a tag, when one is made, goes on that commit and not on a records commit, D-29).
- The commit line above is the head the script ran on: `407d668`, the head of plan 08-20 (every plan through 08-20, nothing of plan 08-21). Plan 08-21 changes only files under `.planning/` (its `files_modified` list), so the seven tree identities above are the identities of the final tree of this plan; the SUMMARY of plan 08-21 records the re-check made after its last commit.
- The line "dirty" lists the three tracked files that differ from the head by design, named at the top of this record.


## B. Main green

**Local gates on the final tree of plan 08-21.** Quoted verbatim from the final gates run by Task 3 of this plan on the tree of the Task 2 commit `6fcc396`, which holds every file outside `.planning/` in its final form (the Task 3 and closing commits change only `.planning/` text; the runs on the closing tree are in the SUMMARY of plan 08-21); the first two are local laptop figures and the third is the container (Ubuntu 24.04, gcc 13.3.0, emulated amd64). None is a hosted figure. [nc]

- `sh tools/gate.sh` (local laptop): 0 lines with -- FAIL, 4 lines with -- SKIPPED; the gate-cpu, gate-time and summary lines of its output, verbatim:

```
gate-cpu user=154.31 sys=134.87
gate-time total=86
gate: PASS steps=38/38 controls=380/380 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=235 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=64 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=28 provisional=27 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=665 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=235/235 tracked=2489 binaries-listed=0 terms=9 doc-files=59 profile-keys=154 goldens=29 reports=32 host-dependent=3 mode=tree hygiene-files=2489 history-commits=1 gates=90 lines=306 input-rows=31 input-policies=11 input-profile-fields=31 files=2489 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=19/19 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=11/11 runner-files=16 cases=8 sources=12 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 docs=60 claim-refs=26 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

- `dash tools/gate.sh` (local laptop): 0 lines with -- FAIL; the same three lines, verbatim (the summary line equals the one of sh, token for token):

```
gate-cpu user=154.17 sys=137.45
gate-time total=89
gate: PASS steps=38/38 controls=380/380 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=235 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=64 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=28 provisional=27 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=665 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=235/235 tracked=2489 binaries-listed=0 terms=9 doc-files=59 profile-keys=154 goldens=29 reports=32 host-dependent=3 mode=tree hygiene-files=2489 history-commits=1 gates=90 lines=306 input-rows=31 input-policies=11 input-profile-fields=31 files=2489 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=19/19 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=11/11 runner-files=16 cases=8 sources=12 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 docs=60 claim-refs=26 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

- `GATE_LINUX_PLATFORM= sh tools/gate-linux.sh` (container, Ubuntu 24.04, emulated amd64): 0 lines with -- FAIL, 4 lines with -- SKIPPED; the same three lines, verbatim:

```
gate-cpu user=1277.54 sys=84.25
gate-time total=349
gate: PASS steps=38/38 controls=380/380 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=GNU c-presets=2 fault-runs=235 core-imports=5 purity-selftest=7/7 ctest-runs=2 ci-runs=64 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=28 provisional=27 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=665 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=235/235 tracked=2489 binaries-listed=0 terms=9 doc-files=59 profile-keys=154 goldens=29 reports=32 host-dependent=3 mode=tree hygiene-files=2489 history-commits=1 gates=90 lines=306 input-rows=31 input-policies=11 input-profile-fields=31 files=2489 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=sha256 public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=19/19 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=11/11 runner-files=16 cases=8 sources=12 refs-verified=0/4 state-rows=88 state-cuts=18 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 docs=60 claim-refs=26 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```


**Hosted rows recorded by the orchestrator before the merge.** These are hosted figures of the event `pull_request`, all with conclusion `success`; they are the last five rows of `.planning/ci-runs.tsv` (an uncommitted working-tree file that the orchestrator keeps and no executor stages), `r = cpu_s / build_s`. They are not the push run on `main`, which exists only after the merge. [nc]

| Run | Head | `build_s` | `gate_s` | `cpu_s` | `r` |
|-----|------|-----------|----------|---------|-----|
| 43658364307 | 5d56e21 | 46 | 109 | 335 | 7.28 |
| 43669236621 | 2eaab6e | 44 | 112 | 346 | 7.86 |
| 43674301502 | 1a27cdd | 52 | 134 | 431 | 8.29 |
| 43679861228 | ce469aa | 46 | 116 | 355 | 7.72 |
| 43732465524 | 407d668 | 45 | 113 | 351 | 7.80 |

The median of the five `r` values is 7.80 against the budget 7.8 and the revisit line 8.3. The newest row, run 43732465524, has the head `407d668`, which contains every plan through 08-20 (checked with `git merge-base --is-ancestor` against the first commit of each plan); it does not contain plan 08-21. Every one of the seven hosted rows recorded since the Phase 8 start is a success. [nc]

**The push run on the merge commit (stage 2).** Exists only after the merge; the orchestrator records it with `tools/ci-record.sh` as an `event=push` row and fills the lines below. [nc]

- push run id: `<stage2>`
- head sha of that run (it must equal the artifact commit of section A): `<stage2>`
- event `push` and conclusion: `<stage2>`
- the row appended to `.planning/ci-runs.tsv` for the push event, and its `r`: `<stage2>`
- cost close of Phase 8: the line `phase8-close: no-lever` of the entry "Phase 8 close-out (plan 08-21)" in `.planning/CI-EFFICIENCY.md` (median `r` 7.80 of the last five hosted pull request rows against the budget 7.8 and the line 8.3)

## C. Open pull requests and issues

Everything that is read from GitHub is a stage 2 fact: it is read once, after the merge, by the orchestrator (the API rate limit is shared by every agent and an executor never calls `gh`). [nc]

- open pull requests, count and each with its label and disposition: `<stage2>`
- open issues, count and each with its label and disposition: `<stage2>`
- the repository ruleset read (`gh api` rulesets, one read at the close, never polled): ids and enforcement, or "not applied": `<stage2>`
- the releases list (expected: none before the first release): `<stage2>`

**What was known before the merge (not a fresh read).** The Phase 8 discussion read GitHub on 2026-11-14 and found no open pull request (six merged, numbered 1 to 6), no open issue, no release, no git tag and no repository ruleset, although `.planning/AUTONOMY.md` recommended one on 2026-11-11; that reading is history and is not re-read here. [nc] The Phase 8 pull request did not exist at that time and is not counted. [nc]

**Merged remote phase branches.** The same reading listed the remote branches of Phases 2 to 6 as still present. Recommendation: delete the merged remote phase branches after the close, as housekeeping that a restore from the pull request page reverses. Deleting them does not purge history: their commits stay reachable from `main`, and the pull request refs that GitHub keeps hold the same objects, which is also why a history rewrite could not remove the six findings of section J. [nc]

**Ruleset.** `.planning/AUTONOMY.md` section 1 records the exact commands of a branch ruleset (a pull request with no approval, the status check `gate`, no force push, no deletion) and says the orchestrator may apply it; nothing in Phase 8 applied it. A tag ruleset (no update, no deletion, created only from a green `main`) is the matching settings action of the release design. Whether either is applied is part of the stage 2 read above and of check P7. [nc]

## D. Rolling roadmap and lessons

### Next-milestone candidates [REC]

These come from the v2 list of `.planning/REQUIREMENTS.md`. They are an executor's recommendation for the next milestone discussion (`gsd-new-milestone`), which fans out per `.planning/AUTONOMY.md` section 2; they are not decisions and the milestone is not started. The first three are named; each row says which evidence gates it. [nc]

| Candidate | Requirement | What gates it | First evidence step |
|-----------|-------------|---------------|---------------------|
| 1. Input record and replay at machine-time boundaries | TOOL-01 | no owner gate and no rights question; a written format first, with the oracle that a replayed log reproduces the digests of the recorded run | the scenario input events (API-03) and the round-trip hook (08-16) already give deterministic input and a restore path to build on |
| 2. A reference host frontend (window, audio device, physical controller mapping), outside the core build graph | TOOL-04 | a consumer that wants it; a dependency decision under the small-dependency rule (a cost that is justified, AGENTS.md); a palette and presentation policy; the runner is not installed today, so a packaging decision comes with it | it would let a person look at frames and listen (H7-1, H7-2), which no tool judges; none of that is evidence |
| 3. Cartridge import of `.bin` plus `.cfg` and bank switching | FMT-01 | a rights review of the format description (written in the project's own words, FND-05); user-supplied files only and no third-party cartridge committed; a profile and memory-map extension (open question 4) | original fixtures built with `ivs_asm` for each bank mode, before any third-party file is read |
| Interactive debugger | TOOL-02 | no external gate | the side-effect-free inspection path (VID-06) and the cycle-stamped bus accesses (CPU-05) |
| LibFuturo adapter | TOOL-03 | owner gate D13: a real consumer must exist and fit must be shown (the controller word against the shared contract is unconfirmed) | not before a consumer |
| Scheduled fuzzing and extended sanitizers | TOOL-05 | a failure class that no gate step catches (AGENTS.md); the libFuzzer target `tests/fuzz/fuzz_snapshot.c` exists outside the gate | a failing case found by an on-demand run |
| Hardware-measured timing evidence | TOOL-06 | owner gate D10: a console and the 44 hardware items of `data/expected/d10-measurements.ids` | the procedures `docs/input/d10-procedure.md` and `docs/stic/d10-procedure.md` |
| Variant profiles: Intellivision II, Sears and Tandy aliases, PAL | VAR-01 to VAR-04 | user-supplied system images and a rights review for the first three; for PAL a primary data sheet or a measurement, because every PAL value is unknown | an owner run of the same kind as D11 |
| Content formats with persistence, a hash-keyed content database | FMT-02, FMT-03 | a rights review, and a persistence story that reverses the row OQ8 of the register | a consumer that needs archival snapshots or battery RAM |
| ECS and IntelliVoice | PER-01, PER-02 | rights review of the ROMs, which would be user-supplied | none yet |

### What was deferred out of Phase 8 and why

The full list, with the place each is recorded, is `08-ACCEPTED-DEBT.md` section 6; the rows that matter for the next milestone are these. [nc]

- The snapshot file options (`--snapshot-out`, `--snapshot-in`): an optional last plan of D-18 whose budget went to the corruption family, the hook and the benchmarks; no consumer needs a file, and the planning text of this plan states that the orchestrator confirmed the deferral. The policy that would govern them is decided (the row OQ8 of the register).
- 192-line video publication (D8 stays at 159 by 96), libFuzzer runs in CI, optimization of the SHA-256 frame hashing: each waits for a consumer or a measured need.
- The applied-event count of an instance has no public getter; two pinned acceptances of the state wave (a STIC register word with a bit outside its write mask, and machine time not tied to a unit boundary); a non-default witness for `stic.late_grants`.
- The benchmark baseline is informational (taken on a host that was not idle, one host class, the timed path run on macOS arm64 only), and the two benchmark golden commands are not listed in `docs/runner.md`.
- D10, D11, a history rewrite and name clearance: owner items, section K.
- The six draft validation files of Phases 2 to 7 and Phase 8's own `08-VALIDATION.md`: accepted tech debt (`08-ACCEPTED-DEBT.md` section 1); every check in their maps ran in its own plan and runs again in every gate.

### LESSONS-m1

What recurred, what the gate caught and the cost story of the first milestone, one line each. The Phase 8 lessons for planners are numbered 19 to 24 in `.planning/ops/PLAN-LESSONS.md`. [nc]

1. Recurred: every phase that added a requirement, a public header, a golden, a tool or a workflow broke the same seven gate checks, each needing its retarget in the same commit (the api-effects status band, the important-gates requirement list and its ten toy registries, the video checker grammar, the ci-parity per-file branch, the text scope for `CHANGELOG.md`, the c-build tool inventory, the pkg-install header set; plans 08-01, 08-03, 08-08, 08-13, 08-17 to 08-19). A plan that lists them in `files_modified` has no surprise.
2. Recurred: plan prose against the frozen specification (the order of the restore checks, where a reason is raised, which witness proves which row; plans 08-09, 08-12, 08-13). The specification won each time and the plan sentence became a recorded deviation (lesson 19).
3. Recurred: registration and pending lists. A fault site written one task before its registration stopped the gate with `fault-orphan-site` (08-06); the pending line of a test id was deleted in the commit that registered it (08-11); every count moved in the commit that caused it (08-17).
4. Recurred, caught before any push: compiler and host limits (the usage text above the string length ISO C requires, 08-16; an Apple-only libc symbol, 08-18; the mtime format of the archive tool, 08-01). The container gate and the dash run stood in for the hosted image; no hosted row of Phase 8 failed.
5. The gate caught: the 13 named faults of the phase (four STIC, nine state) each have a registered must-fail id and run at every plan end (235 fault runs in all); the row-omission sweep showed three inert witnesses and a program that could not witness a row (08-14); a round-trip hook that did nothing passed every equality until a guard on its cut count was added (08-16, lesson 20).
6. The gate caught: a requirement that becomes valid breaks the control that used it as an invalid id (`important-gates-bad-req-phase6` was retargeted to a never-valid id in 08-08), which is the reason the retarget belongs in the same commit.
7. Oracle first worked as a method and is still one lineage: both blind second reads closed (the STIC oracle with two wording corrections, the checklist with none), by a reader of the same model family; an independent reader is still missing and no claim exceeds provisional.
8. Cost story: the median `r` read 7.78 at the Phase 8 start, then `checkpointA-08` read `lever-required` by the orchestrator's 8.0 projection rule and resolved to no lever by lever 2 (mutant inventories judged statically on the sanitized preset: about 4.7 local cpu-s saved), `checkpointB-08` and `checkpointC-08` read 7.98 with no lever, and the close reads 7.80. Wave B was priced at 12 to 16 hosted cpu-s and measured at 21.4, and the median still ended at the budget, because the spread of single rows (7.20 to 8.37 over the milestone) is larger than the code growth (lesson 22).
9. Plan count: 21 plans against an estimate of 22 (range 19 to 25) and two lever commits; wave A took six plans, and the one `lever-required` line (checkpoint A) was resolved to no lever by the orchestrator, so the split trigger of D-36 did not apply and Phase 8 stayed one phase.
10. The release machinery (`release.yml`, `tools/release.sh`, the readiness generator, the rehearsal) is built and gate-checked but has run only as a local rehearsal, never as a tag-triggered hosted run; the first tag will be its first hosted test, which is why the first release is a draft.

## E. Requirement coverage

One row per requirement, 50 of 50. The table is generated from the traceability table of `.planning/REQUIREMENTS.md` by a throwaway script in the untracked build tree (id, phase and status are read, the evidence file is the verification record of that phase); the caveat column is written by hand, and every entry in it carries a claim tag or `[nc]`. A `Complete` mark is never stronger than the wording of its verification record and its close-out note in `.planning/REQUIREMENTS.md`. [nc]

| Requirement | Phase | Status | Evidence file | Caveat |
|---|---|---|---|---|
| FND-01 | 1 | Complete | `.planning/phases/01-evidence-profile-and-rights-foundations/01-VERIFICATION.md` | profile is family-level, not a unit measurement (D3); the PSG part is AY-3-8914-class at tier 4 by inference (C11); every field carries its tier tag [claim:CL-0002] |
| FND-02 | 1 | Complete | `.planning/phases/01-evidence-profile-and-rights-foundations/01-VERIFICATION.md` | the vocabulary and the ledger exist; 27 of 28 claims are provisional, the one established claim is CL-0002; none at the measured-hardware tier [nc] |
| FND-03 | 1 | Complete | `.planning/phases/01-evidence-profile-and-rights-foundations/01-VERIFICATION.md` | the log records conflicts and chosen defaults; the resolvers are primary pages or owner captures (D10), so most entries stay open [nc] |
| FND-04 | 1 | Complete | `.planning/phases/01-evidence-profile-and-rights-foundations/01-VERIFICATION.md` | 11 manifests: redistributable yes 8, no 2 (owner EXEC and GROM, not committed), unknown 1 (treated as no); a hash establishes identity, not rights [nc] |
| FND-05 | 1 | Complete | `.planning/phases/01-evidence-profile-and-rights-foundations/01-VERIFICATION.md` | a documented rule checked in review and by the file and text steps; no tool can prove that nothing was copied [nc] |
| FND-06 | 1 | Complete | `.planning/phases/01-evidence-profile-and-rights-foundations/01-VERIFICATION.md` | notes come from the CP-1600 manual of 1975; the CP-1610 part is an inference (C31) [nc] |
| FND-07 | 1 | Complete | `.planning/phases/01-evidence-profile-and-rights-foundations/01-VERIFICATION.md` | master tick vocabulary and constant per D2; no timing was measured on a console [nc] |
| CPU-01 | 2 | Complete | `.planning/phases/02-cp-1610-cpu-core/02-VERIFICATION.md` | vectors hand-derived from the 1975 manual by one author and one model family; CP-1600 against CP-1610 is an inference (C31); no timing claim at the measured tier [nc] |
| CPU-02 | 2 | Complete | `.planning/phases/02-cp-1610-cpu-core/02-VERIFICATION.md` | vectors hand-derived from the 1975 manual by one author and one model family; CP-1600 against CP-1610 is an inference (C31); no timing claim at the measured tier [nc] |
| CPU-03 | 2 | Complete | `.planning/phases/02-cp-1610-cpu-core/02-VERIFICATION.md` | vectors hand-derived from the 1975 manual by one author and one model family; CP-1600 against CP-1610 is an inference (C31); no timing claim at the measured tier [nc] |
| CPU-04 | 2 | Complete | `.planning/phases/02-cp-1610-cpu-core/02-VERIFICATION.md` | vectors hand-derived from the 1975 manual by one author and one model family; CP-1600 against CP-1610 is an inference (C31); no timing claim at the measured tier [nc] |
| CPU-05 | 2 | Complete | `.planning/phases/02-cp-1610-cpu-core/02-VERIFICATION.md` | vectors hand-derived from the 1975 manual by one author and one model family; CP-1600 against CP-1610 is an inference (C31); no timing claim at the measured tier [nc] |
| MEM-01 | 3 | Complete | `.planning/phases/03-memory-map-content-loading-and-refusal/03-VERIFICATION.md` | map from tier 1 catalog pages and tier 4 community material with per-field tags; the cartridge format is the project container IVSC v1, no third-party format [nc] |
| MEM-02 | 3 | Complete | `.planning/phases/03-memory-map-content-loading-and-refusal/03-VERIFICATION.md` | map from tier 1 catalog pages and tier 4 community material with per-field tags; the cartridge format is the project container IVSC v1, no third-party format [nc] |
| MEM-03 | 3 | Complete | `.planning/phases/03-memory-map-content-loading-and-refusal/03-VERIFICATION.md` | map from tier 1 catalog pages and tier 4 community material with per-field tags; the cartridge format is the project container IVSC v1, no third-party format [nc] |
| MEM-04 | 3 | Complete | `.planning/phases/03-memory-map-content-loading-and-refusal/03-VERIFICATION.md` | map from tier 1 catalog pages and tier 4 community material with per-field tags; the cartridge format is the project container IVSC v1, no third-party format [nc] |
| MEM-05 | 3 | Complete | `.planning/phases/03-memory-map-content-loading-and-refusal/03-VERIFICATION.md` | map from tier 1 catalog pages and tier 4 community material with per-field tags; the cartridge format is the project container IVSC v1, no third-party format [nc] |
| API-01 | 3 | Complete | `.planning/phases/03-memory-map-content-loading-and-refusal/03-VERIFICATION.md` | one profile, 0.x minors may break; effects are held by tests of the one build, not by a proof [nc] |
| API-02 | 4 | Complete | `.planning/phases/04-headless-runner-and-installed-consumer/04-VERIFICATION.md` | one profile, 0.x minors may break; effects are held by tests of the one build, not by a proof [nc] |
| API-03 | 5 | Complete | `.planning/phases/05-hand-controllers-and-psg-sound/05-VERIFICATION.md` | one profile, 0.x minors may break; effects are held by tests of the one build, not by a proof [nc] |
| API-04 | 4 | Complete | `.planning/phases/04-headless-runner-and-installed-consumer/04-VERIFICATION.md` | one profile, 0.x minors may break; effects are held by tests of the one build, not by a proof [nc] |
| API-05 | 2 | Complete | `.planning/phases/02-cp-1610-cpu-core/02-VERIFICATION.md` | one profile, 0.x minors may break; effects are held by tests of the one build, not by a proof [nc] |
| API-06 | 3 | Complete | `.planning/phases/03-memory-map-content-loading-and-refusal/03-VERIFICATION.md` | one profile, 0.x minors may break; effects are held by tests of the one build, not by a proof [nc] |
| RUN-01 | 4 | Complete | `.planning/phases/04-headless-runner-and-installed-consumer/04-VERIFICATION.md` | outcomes and reports of one runner on one profile; goldens are repeatability only [nc] |
| RUN-02 | 4 | Complete | `.planning/phases/04-headless-runner-and-installed-consumer/04-VERIFICATION.md` | outcomes and reports of one runner on one profile; goldens are repeatability only [nc] |
| RUN-03 | 2 | Complete | `.planning/phases/02-cp-1610-cpu-core/02-VERIFICATION.md` | the inventory and the named-fault controls prove the gate can fail for its intended reasons; they prove nothing about hardware [nc] |
| RUN-04 | 4 | Complete | `.planning/phases/04-headless-runner-and-installed-consumer/04-VERIFICATION.md` | the inventory and the named-fault controls prove the gate can fail for its intended reasons; they prove nothing about hardware [nc] |
| RUN-05 | 4 | Complete | `.planning/phases/04-headless-runner-and-installed-consumer/04-VERIFICATION.md` | the .digests goldens are self-recorded: repeatability only, never an oracle [nc] |
| RUN-06 | 4 | Complete | `.planning/phases/04-headless-runner-and-installed-consumer/04-VERIFICATION.md` | the diagnostic expectation was derived by the author of the core, with the shared-misreading caveat [nc] |
| PKG-01 | 4 | Complete | `.planning/phases/04-headless-runner-and-installed-consumer/04-VERIFICATION.md` | the installed consumer proves path and build-graph independence on one toolchain at a time, not toolchain independence; its goldens agree through the same core [claim:CL-0015] |
| PKG-02 | 2 | Complete | `.planning/phases/02-cp-1610-cpu-core/02-VERIFICATION.md` | one local command and one hosted job; the container gate is emulated amd64 and is a proxy for the hosted image [nc] |
| PKG-03 | 4 | Complete | `.planning/phases/04-headless-runner-and-installed-consumer/04-VERIFICATION.md` | hosted CI rows are recorded by the orchestrator; local figures are never mixed with hosted ones [nc] |
| PKG-04 | 4 | Complete | `.planning/phases/04-headless-runner-and-installed-consumer/04-VERIFICATION.md` | hosted CI rows are recorded by the orchestrator; local figures are never mixed with hosted ones [nc] |
| AUD-01 | 5 | Complete | `.planning/phases/05-hand-controllers-and-psg-sound/05-VERIFICATION.md` | PSG rules read from AY-3-8910 pages and applied to an AY-3-8914-class part by inference (C11, C69); nobody listened; provisional [claim:CL-0012] |
| AUD-02 | 5 | Complete | `.planning/phases/05-hand-controllers-and-psg-sound/05-VERIFICATION.md` | PSG rules read from AY-3-8910 pages and applied to an AY-3-8914-class part by inference (C11, C69); nobody listened; provisional [claim:CL-0012] |
| INP-01 | 5 | Complete | `.planning/phases/05-hand-controllers-and-psg-sound/05-VERIFICATION.md` | controller byte table is a tier 4 community literal (C16, 31 rows) with a same-family blind read; electrical combinations are labelled defaults (C63 to C66); provisional [claim:CL-0009] |
| INP-02 | 5 | Complete | `.planning/phases/05-hand-controllers-and-psg-sound/05-VERIFICATION.md` | controller byte table is a tier 4 community literal (C16, 31 rows) with a same-family blind read; electrical combinations are labelled defaults (C63 to C66); provisional [claim:CL-0009] |
| INP-03 | 5 | Complete | `.planning/phases/05-hand-controllers-and-psg-sound/05-VERIFICATION.md` | controller byte table is a tier 4 community literal (C16, 31 rows) with a same-family blind read; electrical combinations are labelled defaults (C63 to C66); provisional [claim:CL-0009] |
| VID-01 | 6 | Complete | `.planning/phases/06-stic-video-first-pass/06-VERIFICATION.md` | provisional, tier 4 (one community lineage, C81, C96, C97); access windows and the bus-steal schedule are labelled defaults (C1 to C7, C82 to C95); 159 x 96 is one profile field (D8); nobody looked at a picture [claim:CL-0016] |
| VID-02 | 6 | Complete | `.planning/phases/06-stic-video-first-pass/06-VERIFICATION.md` | provisional, tier 4 (one community lineage, C81, C96, C97); access windows and the bus-steal schedule are labelled defaults (C1 to C7, C82 to C95); 159 x 96 is one profile field (D8); nobody looked at a picture [claim:CL-0016] |
| VID-03 | 6 | Complete | `.planning/phases/06-stic-video-first-pass/06-VERIFICATION.md` | provisional, tier 4 (one community lineage, C81, C96, C97); access windows and the bus-steal schedule are labelled defaults (C1 to C7, C82 to C95); 159 x 96 is one profile field (D8); nobody looked at a picture [claim:CL-0016] |
| VID-04 | 6 | Complete | `.planning/phases/06-stic-video-first-pass/06-VERIFICATION.md` | provisional, tier 4 (one community lineage, C81, C96, C97); access windows and the bus-steal schedule are labelled defaults (C1 to C7, C82 to C95); 159 x 96 is one profile field (D8); nobody looked at a picture [claim:CL-0016] |
| VID-05 | 8 | Complete | `.planning/phases/08-stic-completion-save-state-performance-baselines/08-VERIFICATION.md` | provisional, tier 4, one lineage; pinned unknown-default-chosen remainder C107 to C110 and the C87 pins, five owner hand-offs D10-VID-10 to D10-VID-14 not performed; collision is evaluated on the undelayed scene; nobody looked at a picture [claim:CL-0025] |
| VID-06 | 6 | Complete | `.planning/phases/06-stic-video-first-pass/06-VERIFICATION.md` | provisional, tier 4 (one community lineage, C81, C96, C97); access windows and the bus-steal schedule are labelled defaults (C1 to C7, C82 to C95); 159 x 96 is one profile field (D8); nobody looked at a picture [claim:CL-0016] |
| SCN-01 | 7 | Complete | `.planning/phases/07-first-visible-original-scenario/07-VERIFICATION.md` | one original program, playable guest progress for that program only; provisional; no timing claimed; nobody looked or listened [claim:CL-0023] |
| SCN-02 | 7 | Complete | `.planning/phases/07-first-visible-original-scenario/07-VERIFICATION.md` | documented route only: D11 (owner run with real EXEC and GROM images) is not performed, so there is no result, no claim and no tier [nc] |
| STA-01 | 8 | Complete | `.planning/phases/08-stic-completion-save-state-performance-baselines/08-VERIFICATION.md` | container IVSN v1 is exact-build and not archival; the eight machine-level digests are informational; the trailer is integrity, not authenticity; no big-endian host was run; one claim, provisional [claim:CL-0028] |
| STA-02 | 8 | Complete | `.planning/phases/08-stic-completion-save-state-performance-baselines/08-VERIFICATION.md` | 18 named hard points (HP-SDBD, HP-STEAL-A to C, HP-OPEN, HP-CLOSED, HP-INT-ENTRY, HP-HLT, HP-HALTED, HP-FAULTED, HP-RESETPEND, HP-FRAME-MID, HP-LATCH, HP-MOB, HP-PSG-MID, HP-ENV-MID, HP-INPUT, HP-LOG) over an 88-row checklist, 9 of 51 STATE rows exempt from the divergence sweep with an argument each, two pinned acceptances; checklist and code have one author, the blind read is the same model family; deterministic repeatability only [claim:CL-0028] |
| PRF-01 | 8 | Complete | `.planning/phases/08-stic-completion-save-state-performance-baselines/08-VERIFICATION.md` | four named workloads verified before any figure; the baseline is one record on one host class (darwin arm64) taken on a host that was not idle, informational only; no threshold, no optimization, no claim; figures are repeatability only [nc] |
| REL-01 | 8 | Pending | `.planning/phases/08-stic-completion-save-state-performance-baselines/08-VERIFICATION.md` | stage 1 is recorded here and in the verification record; the stage 2 facts are the placeholder <stage2> until the records pull request after the merge; Pending, not complete [nc] |

**Rows by status:** 49 Complete and 1 Pending (REL-01), 50 rows in all. [nc]

**Three-source cross-check.** The three sources are the `requirements-completed` line of the SUMMARY files, the verification record of the phase and the traceability table. [nc]

- Phases 1 to 7: 45 requirements, 0 findings. Plan 08-20 ran the check after backfilling 39 SUMMARY files: every one of the 45 is listed by at least one SUMMARY of its phase and named by the verification record of its phase; before the backfill 11 requirements of Phases 5 and 6 had no SUMMARY that listed them.
- Phase 8: VID-05 is listed by plan 08-08, STA-01 and STA-02 by plan 08-17 and PRF-01 by plan 08-19; all four are named in `08-VERIFICATION.md` and are Complete in the table. REL-01 is listed by no SUMMARY and is Pending in the table, which is consistent: it completes only after stage 2.
- Findings: none, so the milestone audit has no requirement that is partial by omission. The reading rule of plan 08-20 stands: a backfilled line is the plan's own requirements field and an index, not a claim; the closure evidence is the verification record.

**Roll-up.** `49 of 50 closed on their recorded terms, REL-01 at stage 1`, with `28 claims, 27 provisional, none measured` beside it. The sentence `50 of 50 requirements closed on their recorded terms` may be written only after stage 2 closes REL-01 and must not be written before. [nc]

## F. Claim table

| Claim | Tier | Standing | Artifact |
|---|---|---|---|
| CL-0001 | source-expectation | provisional | docs/cp1610/reference-notes.md |
| CL-0002 | source-expectation | established | data/profiles/intv-2609-ntsc-family-p1.json |
| CL-0003 | source-expectation | provisional | data/profiles/intv-2609-ntsc-family-p1.json |
| CL-0004 | source-expectation | provisional | docs/cp1610/reference-notes.md |
| CL-0005 | source-expectation | provisional | data/cp1610/opcode-families.json |
| CL-0006 | source-expectation | provisional | data/memmap/intv-2609-ntsc-family-p1.literal.txt |
| CL-0007 | source-expectation | provisional | docs/formats/ivsc-v1.md |
| CL-0008 | original-diagnostic | provisional | fixtures/FX-diag-cpu-1/diag-cpu-1.listing.txt |
| CL-0009 | source-expectation | provisional | data/input/intv-2609-ntsc-family-p1.literal.txt |
| CL-0010 | source-expectation | provisional | data/input/intv-2609-ntsc-family-p1.literal.txt |
| CL-0011 | deterministic-repeatability | provisional | docs/input/controller-notes.md |
| CL-0012 | source-expectation | provisional | data/psg/rules.json |
| CL-0013 | deterministic-repeatability | provisional | docs/psg/reference-notes.md |
| CL-0014 | original-diagnostic | provisional | fixtures/FX-diag-ctrl-psg-1/diag-ctrl-psg-1.listing.txt |
| CL-0015 | installed-host-behavior | provisional | tests/installed-consumer/main.c |
| CL-0016 | source-expectation | provisional | data/stic/intv-2609-ntsc-family-p1.regmap.literal.txt |
| CL-0017 | deterministic-repeatability | provisional | docs/stic/timing-notes.md |
| CL-0018 | source-expectation | provisional | data/video/geometry.literal.txt |
| CL-0019 | source-expectation | provisional | docs/video/frame-derivations.md |
| CL-0020 | original-diagnostic | provisional | fixtures/FX-diag-stic-1/diag-stic-1.listing.txt |
| CL-0021 | deterministic-repeatability | provisional | docs/stic/reference-notes.md |
| CL-0022 | installed-host-behavior | provisional | tests/installed-consumer/main.c |
| CL-0023 | playable-guest-progress | provisional | data/golden/demo-lamp-tone-pass.expect |
| CL-0024 | installed-host-behavior | provisional | tests/installed-consumer/main.c |
| CL-0025 | source-expectation | provisional | tests/video/video_mob.c |
| CL-0026 | source-expectation | provisional | tests/video/video_collision.c |
| CL-0027 | original-diagnostic | provisional | fixtures/FX-diag-stic-2/diag-stic-2.listing.txt |
| CL-0028 | deterministic-repeatability | provisional | tests/state/state_cut.c |

Claims: 28 in total, 27 provisional, 1 established; none at tier measured-hardware.

By tier: source-expectation=15 original-diagnostic=4 deterministic-repeatability=5 installed-host-behavior=3 playable-guest-progress=1


**Reading of section F.** The script refuses with exit 1 when any claim stands at the measured-hardware tier, and it refused none: the totals line reads 28 claims, 27 provisional, 1 established, none at the measured-hardware tier, because no measurement on a console was made (D10 is not performed). [nc] The one established claim is CL-0002 (the printed STIC part and crystal figures of the 1980 data catalog); every other claim is provisional by the weakest-field rule. [claim:CL-0002] The four claims added by Phase 8 are CL-0025 (MOB, border and delay pictures), CL-0026 (collision registers), CL-0027 (the original STIC diagnostic program) and CL-0028 (snapshot, restore and run equals the uninterrupted run at 18 named hard points, one build, one profile); each is provisional and each carries an exclusion list in `data/claims.json`. [claim:CL-0025] [claim:CL-0026] [claim:CL-0027] [claim:CL-0028] The table above is the table of the `CHANGELOG.md` section `[0.1.0]`: the gate holds the two equal (the drift check of the `release-assets` step).


## G. Rights and package audit

- pinned install list (data/expected/pkg-install-files.ids): 11 files
  - include/ivs/ivs.h
  - include/ivs/ivs_machine.h
  - include/ivs/ivs_run.h
  - include/ivs/ivs_state.h
  - include/ivs/ivs_status.h
  - lib/cmake/ivs/ivsConfig.cmake
  - lib/cmake/ivs/ivsConfigVersion.cmake
  - lib/cmake/ivs/ivsTargets-release.cmake
  - lib/cmake/ivs/ivsTargets.cmake
  - lib/libivs_core.a
  - share/doc/ivs/LICENSE
- tracked binaries by name (git ls-files against tools/check/binary-ext.txt): 0
- fixture census (data/expected/fixtures.ids): 11 manifests, redistributable yes 8, no 2, unknown or other 1, missing 0
- name check (tracked paths holding the word Intellivision in any case): 0


**Reading of section G.** [nc]

- The install list is the eleven files of `data/expected/pkg-install-files.ids`: five headers, the static library, four CMake package files and the licence. Earlier Phase 8 text said ten; the count became eleven when `ivs_state.h` joined in plan 08-11.
- The count of tracked file names with a binary extension of `tools/check/binary-ext.txt` is 0. The check of the release archives themselves is content-based (`tools/check/release-assets.sh`, run by the gate on a synthetic archive and by `tools/release.sh` on the real ones), because the archive names end in the listed `gz` and `tar` extensions.
- Fixture census: 11 manifests; redistributable yes 8; no 2 (the two owner-image manifests, `committed: false`, no byte and no digest in the repository); other 1 (a sample manifest with unknown rights, treated as no).
- Not installed and not shipped: no EXEC, GROM, commercial ROM, manual, overlay or image; no runner, assembler or pack tool; no binary for any other system. A hash establishes identity, not rights, and no EXEC or GROM digest is recorded anywhere (`.planning/PROJECT.md`, row D11).
- Licence and notices: MIT, with the non-affiliation and working-name statements of `README.md` repeated in `CHANGELOG.md`; the working name is not trademark-cleared (owner item).
- Dependencies: none at runtime. The core is ISO C17 with no library; the gate needs CMake, a C compiler and POSIX shell.
- The per-file SHA-256 of the installed files is not printed here: the archive is build-specific, and the release run writes `SHA256SUMS`, which the draft check P8 reads (stage 2).


## H. Qualification

What is **not** qualified, each with the blocker that would clear it. The list was cross-checked against section F: no claim stands above provisional and none is at the measured-hardware tier. [nc]

1. **No measured-hardware claim.** Nothing was measured on a console. Blocker: D10, an owner hard stop that needs hardware; the 47 owner items of `data/expected/d10-measurements.ids` (44 hardware items: 7 controller, 11 PSG, 12 system and 14 STIC, five of them added by Phase 8; and 3 owner-local EXEC items) are handed off and none was performed. [nc]
2. **No real-EXEC result.** No run used a real EXEC or GROM image, and no digest of one is recorded. Blocker: D11, an owner hard stop that needs the owner's files; SCN-02 is complete on documented-route terms only. [nc]
3. **Nobody has looked at a frame or listened to the sound.** The frames are raw palette-index planes, no palette or presentation is claimed, and the Phase 7 hand-offs H7-1 and H7-2 (and a look at the Phase 8 pictures) are not evidence and were not done. Blocker: a person. [nc]
4. **The controller byte table, the STIC rules and the PSG rules are tier 4 or inference.** One community lineage stands behind the MOB, collision, delay and border bits (no tier 1 to 3 page prints them); the PSG part is an AY-3-8914-class inference. Blocker: a primary data sheet or a D10 capture. Every claim that depends on them is provisional. [claim:CL-0025]
5. **The pinned remainder of VID-05 is a list, not a result.** The priority-set pixel rule, half-height publication, strip unit and strip against delay, the exposed area, the delay range, collision at X 0 and at the edge, off screen and with the display off, the read with no side effect, and the own bit on a CPU write (C107 to C110 and the C87 pins) each have a flip control, a claim exclusion and an owner hand-off; collision is evaluated on the undelayed scene. [claim:CL-0026]
6. **No PAL, no Intellivision II, no ECS, no IntelliVoice.** One profile exists, `intv-2609-ntsc-family-p1` (family level, not a unit measurement); every PAL value is unknown. Blocker: the v2 requirements VAR-01 to VAR-04, PER-01 and PER-02. [nc]
7. **No timing claim.** Machine time is reported and never asserted as a hardware figure; the access windows, the bus-steal schedule and the interrupt width are labelled defaults. [nc]
8. **One author and one model family.** The derivations, programs, checklist and core have one author; both blind second reads were made by a reader of the same model family, so a shared misreading is possible and agreement is not independent evidence. Blocker: an independent reader. [nc]
9. **Self-recorded digests, goldens and benchmark figures are repeatability only.** They are recorded by the emulator itself and are never an oracle; no figure of the benchmark tool is a claim. [nc]
10. **Snapshots are exact-build only.** IVSN v1 is not archival; the eight machine-level digests of its header are informational; the trailer is integrity and not authenticity; no big-endian host was run; the core writes no file and the runner has no snapshot file option; 18 hard points and 88 checklist rows are covered, with 9 of 51 state rows exempt from the divergence sweep and two acceptances pinned. [claim:CL-0028]
11. **The benchmark baseline is informational.** One record, one host class (darwin arm64), taken on a host that was not idle, with the timed path run on one platform only; no threshold exists. [nc]
12. **The installed-package proof is narrow.** The installed consumer shows path and build-graph independence on one toolchain at a time and agrees with the runner through the same core; it is not toolchain independence and not an independent oracle. [claim:CL-0024]
13. **No independent verification of Phase 8 yet.** The verification record is an executor's, the milestone audit has not run, and the owner has paused the flow after Phase 8 (relayed). [nc]
14. **The working name is not trademark-cleared.** An owner item; no release text may say otherwise. [nc]

## I. Authority

The release actions of `.planning/AUTONOMY.md` and of `08-DISCUSS-release.md` section 3.5, who may do each, and what stops it. "Orchestrator" means the agent that runs the phases; executors do none of these. [nc]

| Action | Who may do it | What stops it | State at this record |
|--------|---------------|---------------|----------------------|
| Merge a green pull request to `main` | the orchestrator (AUTONOMY section 1, auto-triage, only with CI green) | a red check; no ruleset enforces it today | not done; `<stage2>` |
| Apply a branch or tag ruleset | the orchestrator, with `gh`; never an executor | nothing in policy; the owner may object | `<stage2>` (a read of the rulesets; none was applied in Phase 8) |
| Set `git.create_tag` to false | the orchestrator, as a separate commit (D-42); an executor never stages `.planning/config.json` | the milestone tool would otherwise create a milestone-style tag | not done: the key reads `true` in the working-tree copy |
| Tag `vX.Y.Z` on the artifact commit | the orchestrator (AUTONOMY section 5, step 4), strict 0.x form, equal to the header macros | any failed check P1 to P8; the owner's pause | not done |
| Create the release as a draft pre-release | the tag-triggered release workflow | the same checks | not done |
| Publish the draft | the orchestrator, after reading the draft (P8) | a failed check, a draft that disagrees with the checksums | not done |
| Delete a published release or the repository | nobody delegated: a hard stop, owner only | n/a | not applicable |
| Rewrite published history | nobody delegated: a hard stop, owner only | n/a | not proposed; the six findings are accepted without a rewrite (section J) |
| Ship EXEC, GROM, commercial or scanned material | nobody: a hard stop | rights | none exists in the tree (section G) |
| Perform D10 or D11 | nobody delegated: a hard stop, needs the owner's hardware or files | physical and owner-held evidence | not performed |
| Use the console brand word in a package, product, repository, asset or release name | nobody: a hard stop | third-party trademark | none (section G, name check 0) |
| Clear the working name as a trademark | the owner | owner gate | not done; every release text says so |
| Use a paid service | nobody beyond free tiers | cost | none |
| Skip or reorder the flow after Phase 8 | the owner | n/a | the owner paused the flow after Phase 8 (relayed by the orchestrator) |

Executors never call `gh`, push, tag, create a release, run `tools/ci-record.sh` or pass `--hosted` to the readiness script; plan 08-21 did none of them. [nc]

## J. Hygiene

- tree scan (sh tools/hygiene.sh --tree), exit 0: mode=tree hygiene-files=2487
- history scan (sh tools/hygiene.sh --history), exit 1; findings compared by (token, short sha) with data/expected/hygiene-history-pinned.ids:
  - FAIL (accepted by recorded disposition, D-33): hygiene-message-home-path 955c13fa02
  - FAIL (accepted by recorded disposition, D-33): hygiene-history-home-path eaa832e3d3
  - FAIL (accepted by recorded disposition, D-33): hygiene-history-home-path 92c4dc7d37
  - FAIL (accepted by recorded disposition, D-33): hygiene-history-home-path b78af4fb9f
  - FAIL (accepted by recorded disposition, D-33): hygiene-history-home-path cccdfefdce
  - FAIL (accepted by recorded disposition, D-33): hygiene-history-home-path 345c48dd7f
  - history findings: 6 (accepted 6, new 0)
- disposition (D-33): accepted without a rewrite as a delegated decision; the baseline file is not moved and no rule is widened; a new finding fails this check; the owner may veto, a rewrite being an owner-only stop


**Disposition of the history findings (D-33).** The full history scan reported six findings and the scan itself exited 1; the readiness script exited 0 because each of the six is on the pinned list `data/expected/hygiene-history-pinned.ids` and is printed with its recorded disposition. The findings are accepted by a delegated decision; they are not cleared, and this record does not say that the history check passed, because it did not. [nc]

- The decision: accepted without a rewrite; pinned by (token, short sha) pair; any finding that is not on the list fails the release check (the script exits 1); the owner may veto the disposition and choose a rewrite, which is an owner-only hard stop (`.planning/AUTONOMY.md` section 1) and would not purge the pull-request refs that GitHub keeps.
- The matched text of each of the six is the shape of a scratch-directory path, not a path with user, host or session content (read by grep at planning time and recorded in `08-DISCUSS-release.md` section 7.3). The sixth pair names the reachable commit of the message finding; the Phase 4 and 7 records name another commit with the same subject that lies on no branch.
- Not touched by this plan: the baseline file `tools/check/hygiene-baseline.txt` (`git diff` of it against its last commit is empty) and every rule of `tools/hygiene.sh`. The baseline is not moved and no rule is widened.
- The scan ran in the background on the local laptop (load average 8 to 17), started 2026-11-15T15:57:58Z on the head `407d668`, took 253 s of wall time and ended with the script status 0. Plan 08-21 adds only `.planning/` text and commit messages after that head; the tree scan covers its text at every task, and the SUMMARY records a second history scan on the final head.
- Edit of one generated line: the tree-scan line is printed above without the two words that start its pass verdict, because the verify of the plan requires that this record never carry those words next to the history result; the scan's exit status 0 and its counters are unchanged (`mode=tree hygiene-files=2487`).
- The sections G and J together carry the checks the rest of the hygiene list of `.planning/AUTONOMY.md` section 6 asks for: tracked binaries (G), private paths and secrets (the tree scan), author emails (the history scan's identity check, which is part of the same run).

## K. Decision

**Release status at this record: not started, paused by the owner's decision after Phase 8.** The decision is relayed to the executor by the orchestrator (the owner said to run `/gsd-pause-work` after Phase 8 completes instead of continuing into the milestone audit, the milestone completion, the cleanup and the first release); the executor cannot check it independently. No tag, no draft and no release exists, no `gh` call or push was made by this plan, and this record states nothing as published. [nc]

**Decision line (stage 2):** `<stage2>`

No release decision has been taken. When the flow resumes, the line takes one of two forms, with the evidence that clears the blocker in the second case: `PUBLISHED v0.1.0 (prerelease)`, with the sentence "claims provisional; nothing measured", or `READY, NOT PUBLISHED: blocked by P<n>`. The word "supported" never appears in it. [nc]

### The checks P1 to P8, computable status at stage 1

The checks are those of `.planning/ops/RELEASE-OPS.md` item d and `08-DISCUSS-release.md` section 3.6. "Stage 1 status" means a fact computed before the merge and cited here; a placeholder cell in the last column means a fact that exists only after the merge or only for an orchestrator action. [nc]

| # | Check | Stage 1 status | Stage 2 |
|---|-------|----------------|---------|
| P1 | `main` green at the artifact commit; local sh, dash and container gates pass on the same tree | the three local lines of the final tree are in section B; the hosted `pull_request` rows of section B are successes through head `407d668` | the push run on `main` and its head: `<stage2>` |
| P2 | hygiene: tree clean, secrets and emails clean, history findings equal the pinned six and nothing new | stage 1 evidence, section J: the tree scan exited 0 and the history scan found six pairs, all pinned, none new; the six are accepted by a recorded disposition and are not cleared | none |
| P3 | rights: no tracked binary, install list equals the pinned eleven files, manifest census unchanged or explained, no EXEC or GROM byte or digest | stage 1 evidence, section G: 0 tracked binary names, the eleven-file list, 11 manifests (yes 8, no 2, other 1), name check 0 | none |
| P4 | version agreement: tag equals the header macros equals the installed package version; the changelog has the section | header half: `sh tools/release-readiness.sh --allow-dirty --release` printed `P4: ok` on the head `407d668` (a section `[0.1.0]` for the header version 0.1.0), and the version policy check runs in the gate; the rehearsal `sh tools/release-rehearse.sh --no-gate --release` ended `rehearse: ok` | the tag half cannot be checked before a tag exists: `<stage2>` |
| P5 | notes carry the claim table of `data/claims.json`; no claim word outside the allowed wording; the text step scans `CHANGELOG.md` | stage 1 evidence: the claim table of the `[0.1.0]` section equals the generated table (drift check of the `release-assets` step), and the `text` step scans `README.md`, `docs/` and `CHANGELOG.md` (control `text-changelog-untagged`) | none |
| P6 | the history disposition is recorded as a delegated decision with the owner-veto note | the register row `REL-01 / D-33` of `.planning/PROJECT.md` (added by plan 08-21, Task 2) | that the owner has not vetoed it: `<stage2>` |
| P7 | release machinery in place: `release.yml` under `ci-parity`, `release-assets` in the gate and the rehearsal, tag ruleset applied or recorded as not applied | stage 1 evidence: `release.yml` is held by the `ci-parity` exemption table and `release-assets` runs in every gate; the rehearsal ended `rehearse: ok` (source archives byte-equal, install archives equal except the Darwin ranlib stamp, tag glob tried) | the ruleset read: `<stage2>` |
| P8 | the draft is read with `gh release view --json`: names, sizes, `SHA256SUMS`, notes, pre-release flag, no unexpected asset | none can exist before a tag and a draft | `<stage2>` |

### Hand-offs for the owner (H8-n)

None of these was performed by an agent, none is evidence, and none blocks the stage 1 record. [nc]

| Item | Why a tool cannot judge it | Owner action | Effect on claims |
|------|----------------------------|--------------|------------------|
| H8-1 D10 hardware measurement | physical evidence on a console | the 44 hardware items of `data/expected/d10-measurements.ids` (five were added by Phase 8: collision semantics, priority per pixel kind, delay range with exposed area and bus windows, extension units, half-height structure), by `docs/input/d10-procedure.md` and `docs/stic/d10-procedure.md` | none today: no claim is at the measured-hardware tier |
| H8-2 D11 owner-local run with real EXEC and GROM images | the images are the owner's and rights-bearing | `docs/owner-images.md` with `tools/owner-run.sh`; never committed, never in CI | none: no result, claim or tier exists; a snapshot of such a run is derived rights-bearing material |
| H8-3 Listening and viewing | a person's judgement of sound and picture | H7-1 and H7-2 of the Phase 7 record, and a look at the Phase 8 pictures (`ivs-run` with `--frame-ppm` on a `diag-stic2-` scenario) | none |
| H8-4 The history veto | a rewrite of published history is an owner-only hard stop | veto the accepted disposition of the six findings and choose a rewrite, or leave it | none; the finding stays reported |
| H8-5 Name clearance | trademark clearance of the working name | decide whether to clear the name before a release artifact appears | none |
| H8-6 The GitHub immutable-release setting | a repository setting this executor cannot read | read it once at stage 2: if it is enabled, a published release and its tag cannot be edited, which is one more reason for the draft | none |
| H8-7 The snapshot file options | a scope choice (D-18) | decide whether `--snapshot-out` and `--snapshot-in` are wanted, runner-only, explicit path, refused for user content | none |
| H8-8 An idle-host benchmark baseline | a measurement on a quiet machine | re-take the baseline by the procedure of `docs/bench.md`; a reviewed change replaces the informational record | none; figures are repeatability only |
| H8-9 Resume or stop the release flow | the owner paused it after Phase 8 (relayed) | say when to run the audit, the completion, the cleanup and the release | none |

### Remaining steps, none started (paused)

These are the orchestrator's steps after the merge, in the order of item h of `.planning/ops/RELEASE-OPS.md`; none was started by this plan. [nc]

1. Merge the Phase 8 pull request with green CI; the merge commit is the artifact commit of section A.
2. Record the push run on the merge commit as an `event=push` row with `tools/ci-record.sh` (orchestrator only).
3. Open the `gsd/records-m1-close` branch and make the `git.create_tag` change (D-42) as its first commit, alone, before the milestone tool is used; the key reads `true` today in the working-tree copy of `.planning/config.json`.
4. Read GitHub once with `sh tools/release-readiness.sh --hosted` (five read-only calls): the counts and dispositions of section C, the ruleset read and the immutable-release setting of H8-6.
5. Tag `v0.1.0` on the artifact commit (not on a records commit); apply a tag ruleset or record "not applied". The tag-triggered workflow creates the draft pre-release; read it with `gh release view --json` (P8) and publish only if P1 to P8 hold; otherwise the decision line reads `READY, NOT PUBLISHED: blocked by P<n>`.
6. Fill every placeholder of this record, run the check of item h of `.planning/ops/RELEASE-OPS.md` until it prints nothing, and run the gate on the records branch.
7. Mark REL-01 complete in `.planning/REQUIREMENTS.md` only after that, record the release in `.planning/STATE.md` and write the CI-efficiency entry for the release workflow.
8. Merge the records pull request; run the milestone audit (`gsd-audit-milestone`), the milestone completion (`gsd-complete-milestone`) on a records branch and the cleanup (`gsd-cleanup`); delete the merged remote phase branches (housekeeping, which does not purge history).
9. Start the next milestone (`gsd-new-milestone`) from the candidates of section D.
